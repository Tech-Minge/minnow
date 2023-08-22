#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <iostream>
#include <random>

using namespace std;

Timer::Timer( uint64_t RTO ) : RTO_( RTO ) {}

void Timer::start()
{
  time_passed_ = 0;
  expired_ = false;
}

void Timer::stop() {}

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

void Timer::backoff()
{
  RTO_ *= 2;
  start();
}

void Timer::set_rto( uint64_t rto )
{
  RTO_ = rto;
}

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) )
  , initial_RTO_ms_( initial_RTO_ms )
  , timer_( initial_RTO_ms )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return outstanding_sequence_number_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return consecutive_retransmission_count_;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  if ( ready_message_.empty() ) {
    return {};
  }

  auto to_send = ready_message_.front();
  uint64_t front_ack = to_send.seqno.unwrap( isn_, next_to_send_ );
  if ( next_to_send_ > front_ack ) {
    // retransmit

  } else {
    next_to_send_ += to_send.sequence_length();
    need_acked_message_.push( to_send );
  }

  ready_message_.pop();
  return to_send;
}

void TCPSender::push( Reader& outbound_stream )
{
  // Your code here.
  if ( fin_ ) {
    return;
  }
  uint16_t window_size = max( receiver_window_size_, static_cast<uint16_t>( 1 ) );

  while ( next_from_reader_ < receiver_window_start_ + window_size ) {
    TCPSenderMessage message;
    uint64_t message_len
      = min( TCPConfig::MAX_PAYLOAD_SIZE, receiver_window_start_ + window_size - next_from_reader_ );
    if ( !next_from_reader_ ) {
      message.SYN = true;
      message_len -= 1;
    }
    uint64_t cur_len = 0;
    string got;
    while ( cur_len < message_len && outbound_stream.bytes_buffered() ) {
      auto str_sv = outbound_stream.peek();
      uint64_t needed = message_len - cur_len;
      if ( str_sv.size() < needed ) {
        got += str_sv;
      } else {
        got += str_sv.substr( 0, needed );
      }
      auto actual = min( str_sv.size(), needed );
      cur_len += actual;
      // remove from reader
      outbound_stream.pop( actual );
    }

    message.payload = move( got );
    if ( outbound_stream.is_finished() && message.sequence_length() < window_size ) {
      message.FIN = true;
    }
    if ( !message.sequence_length() ) {
      // no buffered bytes in Reader
      break;
    }
    outstanding_sequence_number_ += message.sequence_length();
    message.seqno = isn_ + next_from_reader_;
    if ( ready_message_.empty() && need_acked_message_.empty() ) {
      timer_.start();
    }
    ready_message_.push( message );
    next_from_reader_ += message.sequence_length();
    // if no code below when finish, will iter again and again
    if ( message.FIN ) {
      fin_ = true;
      break;
    }
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  TCPSenderMessage message;
  message.seqno = isn_ + next_to_send_;
  return message;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  if ( !msg.ackno.has_value() ) {
    receiver_window_size_ = msg.window_size;
    return;
  }
  uint64_t cur_ackno = msg.ackno.value().unwrap( isn_, next_to_send_ );
  // ignore early or impossible ack
  if ( cur_ackno < receiver_window_start_ || cur_ackno > next_to_send_ ) {
    return;
  }
  bool start = false;
  if ( cur_ackno != receiver_window_start_ ) {
    start = true;
  }
  receiver_window_start_ = cur_ackno;
  while ( !need_acked_message_.empty() ) {
    auto to_be_acked = need_acked_message_.front();
    uint64_t ackno = to_be_acked.seqno.unwrap( isn_, next_to_send_ );
    if ( ackno + to_be_acked.sequence_length() <= cur_ackno ) {
      outstanding_sequence_number_ -= to_be_acked.sequence_length();
      need_acked_message_.pop();
    } else {
      break;
    }
  }
  receiver_window_size_ = msg.window_size;

  // timer
  timer_.set_rto( initial_RTO_ms_ );
  if ( need_acked_message_.empty() && ready_message_.empty() ) {
    timer_.stop();
  } else if ( start ) {
    // ack new data
    timer_.start();
  }
  consecutive_retransmission_count_ = 0;
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  timer_.time_pass( ms_since_last_tick );
  if ( timer_.is_expired() ) {
    if ( receiver_window_size_ ) {
      timer_.backoff();
      ++consecutive_retransmission_count_;
    } else {
      timer_.start();
    }
    if ( need_acked_message_.empty() ) {
      return;
    }
    ready_message_.push( need_acked_message_.front() );
  }
}
