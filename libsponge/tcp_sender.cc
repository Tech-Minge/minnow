#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>
#include <algorithm>
// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

Timer::Timer( uint64_t RTO ) : RTO_( RTO ) {}

void Timer::start()
{
    time_passed_ = 0;
    expired_ = false;
    stop_ = false;
}

void Timer::stop() {
    stop_ = true;
}

void Timer::time_pass( uint64_t ms )
{
    time_passed_ += ms;
    if ( time_passed_ >= RTO_ ) {
        expire();
    }
}

void Timer::expire()
{
    expired_ = true;
}

bool Timer::is_expired()
{
    return expired_;
}

bool Timer::is_stop()
{
    return stop_;
}

void Timer::backoff()
{
    RTO_ *= 2;
    start();
}

void Timer::set_rto( uint64_t rto )
{
    RTO_ = rto;
}

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , timer_(_initial_retransmission_timeout) {}

uint64_t TCPSender::bytes_in_flight() const { return outstanding_sequence_number_; }

void TCPSender::fill_window() {
    if ( fin_ ) {
        return;
    }
    uint16_t window_size = max( receiver_window_size_, static_cast<uint16_t>( 1 ) );

    while ( next_to_send_ < receiver_window_start_ + window_size ) {
        TCPSegment message;
        uint64_t message_len = min( TCPConfig::MAX_PAYLOAD_SIZE, receiver_window_start_ + window_size - next_to_send_ );
        if ( !next_to_send_ ) {
            message.header().syn = true;
            message_len -= 1;
        }
        
        message.payload() = Buffer(_stream.read(message_len));
        if ( _stream.eof() && message.length_in_sequence_space() < window_size ) {
            message.header().fin = true;
        }
        if ( !message.length_in_sequence_space() ) {
            // no buffered bytes in Reader
            break;
        }
        outstanding_sequence_number_ += message.length_in_sequence_space();
        message.header().seqno = _isn + next_to_send_;
        if ( outstanding_message_.empty() ) {
            timer_.start();
        }
        _segments_out.push( message );
        outstanding_message_.push( message );
        next_to_send_ += message.length_in_sequence_space();
        // if no code below when finish, will iter again and again
        if ( message.header().fin ) {
            fin_ = true;
            break;
        }
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    uint64_t cur_ackno = unwrap( ackno, _isn, next_to_send_ );
    // ignore early or impossible ack
    if ( cur_ackno < receiver_window_start_ || cur_ackno > next_to_send_ ) {
        return;
    }
    bool start = false;
    if ( cur_ackno != receiver_window_start_ ) {
        start = true;
    }
    receiver_window_start_ = cur_ackno;
    while ( !outstanding_message_.empty() ) {
      auto to_be_acked = outstanding_message_.front();
      uint64_t front_ackno = unwrap( to_be_acked.header().seqno, _isn, next_to_send_ );
      if ( front_ackno + to_be_acked.length_in_sequence_space() <= cur_ackno ) {
          outstanding_sequence_number_ -= to_be_acked.length_in_sequence_space();
          outstanding_message_.pop();
      } else {
          break;
      }
    }
    receiver_window_size_ = window_size;

    // timer
    timer_.set_rto( _initial_retransmission_timeout );
    if ( outstanding_message_.empty() ) {
        timer_.stop();
    } else if ( start ) {
        // ack new data
        timer_.start();
    }
    consecutive_retransmission_count_ = 0;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (timer_.is_stop()) {
        return;
    }
    timer_.time_pass( ms_since_last_tick );
    if ( timer_.is_expired() ) {
        if ( receiver_window_size_ ) {
            timer_.backoff();
            ++consecutive_retransmission_count_;
        } else {
            timer_.start();
        }
        // if ( outstanding_message_.empty() ) {
        //     return;
        // }
        _segments_out.push( outstanding_message_.front() );
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return consecutive_retransmission_count_; }

void TCPSender::send_empty_segment() {
    TCPSegment message;
    message.header().seqno = _isn + next_to_send_;
    _segments_out.push(message);
}
