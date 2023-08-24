#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    const TCPHeader &header = seg.header();
    if ( header.syn ) {
        syn_ = true;
        isn_ = wrap( 0, header.seqno );
    }
    if ( !syn_ ) {
        return; // discard data if doesn't receive syn flag
    }
    uint64_t index = unwrap( header.seqno, isn_, stream_out().bytes_written() ) + ( header.syn ? 0 : -1 );
    _reassembler.push_substring(seg.payload().copy(), index, header.fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!syn_) {
        return {};
    }
    // use `input_ended()` but not `eof()` due to it's sender's message
    return isn_ + ( stream_out().bytes_written() + 1 + ( stream_out().input_ended() ? 1 : 0 ) );
}

size_t TCPReceiver::window_size() const { return stream_out().remaining_capacity(); }
