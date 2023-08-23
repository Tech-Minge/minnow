#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include <queue>

class Timer
{
private:
  uint64_t RTO_;
  uint64_t time_passed_ = 0;
  bool expired_ = false;
  void expire();

public:
  Timer( uint64_t RTO );
  void time_pass( uint64_t ms );
  void set_rto( uint64_t RTO );
  void start();
  void stop();
  void backoff();
  bool is_expired();
};

class TCPSender
{
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;

public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream */
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
  void tick( uint64_t ms_since_last_tick );

  /* Accessors for use in testing */
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?

private:
  uint64_t outstanding_sequence_number_ = 0;
  uint64_t consecutive_retransmission_count_ = 0;
  uint64_t next_from_reader_ = 0;
  uint64_t next_to_send_ = 0;
  std::queue<TCPSenderMessage> send_message_; // for test check
  std::queue<TCPSenderMessage> outstanding_message_;
  uint64_t receiver_window_start_ = 0;
  uint16_t receiver_window_size_ = 1;
  bool fin_ = false;
  Timer timer_;
};
