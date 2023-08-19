#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms )
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
  return {};
}

void TCPSender::push( Reader& outbound_stream )
{
  // Your code here.
  (void)outbound_stream;
  // uint64_t send_count = (receiver_window_size_ + TCPConfig::MAX_PAYLOAD_SIZE - 1) / TCPConfig::MAX_PAYLOAD_SIZE;
  // for (uint64_t i = 0; i < send_count; ++i) {
  //   TCPSenderMessage message;
  //   uint64_t send_len = min(TCPConfig::MAX_PAYLOAD_SIZE, outbound_stream.bytes_buffered());
  // }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  return {};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  (void)msg;
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  (void)ms_since_last_tick;
}
