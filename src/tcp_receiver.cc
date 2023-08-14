#include "tcp_receiver.hh"
#include <algorithm>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // Your code here.
  if ( message.SYN ) {
    syn_ = true;
    isn_ = Wrap32::wrap( 0, message.seqno );
  }
  if ( !syn_ ) {
    return; // discard data if doesn't receive syn flag
  }
  uint64_t index = message.seqno.unwrap( Wrap32( isn_ ), inbound_stream.bytes_pushed() ) + ( message.SYN ? 0 : -1 );
  reassembler.insert( index, message.payload, message.FIN, inbound_stream );
  send( inbound_stream );
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  TCPReceiverMessage message;
  message.window_size = min( inbound_stream.available_capacity(), static_cast<uint64_t>( UINT16_MAX ) );
  if ( syn_ ) {
    message.ackno = isn_ + ( inbound_stream.bytes_pushed() + 1 + ( inbound_stream.is_closed() ? 1 : 0 ) );
  }
  return message;
}
