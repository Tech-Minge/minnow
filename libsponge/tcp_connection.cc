#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return time_since_last_segment_received_; }

void TCPConnection::send_tcpsegment(bool need_set_ack) {
    while (!_sender.segments_out().empty()) {
        auto seg = _sender.segments_out().front(); // already set `seqno, SYN, payload, FIN` if needed
        _sender.segments_out().pop();
        if (need_set_ack) {
            set_ack_and_win(seg);
            need_set_ack = false; // just set one segment
        }
        _segments_out.push(seg);
    }   
}

void TCPConnection::set_ack_and_win(TCPSegment &segment) {
    segment.header().ack = true;
    segment.header().ackno = _receiver.ackno().value();
    segment.header().win = _receiver.window_size();
}

void TCPConnection::segment_received(const TCPSegment &seg) {
    time_since_last_segment_received_ = 0;
    const TCPHeader &header = seg.header();
    TCPSegment segment;
    if (header.rst) {
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
    } else {
        _receiver.segment_received(seg);
        if (header.ack) {
            _sender.ack_received(header.ackno, header.win);
            _sender.fill_window();
        }
        if (seg.length_in_sequence_space() && _receiver.ackno().has_value()) {
            if (_sender.segments_out().empty()) {
                set_ack_and_win(segment); // no need to set `seqno` due to it's pure ACK Segment
                _segments_out.push(segment); // directly push to TCPConnection's queue
            } else {
                send_tcpsegment(true);
            }
        } else {
            send_tcpsegment(false); // does here actually `false`?
        }
    }
}

bool TCPConnection::active() const {
    bool active_flag = true;
    if (_sender.stream_in().error() || _receiver.stream_out().error()) {
        active_flag = false;
    } else {
        if (_sender.stream_in().input_ended() && _receiver.stream_out().eof()) {
            if (!_linger_after_streams_finish) {
                active_flag = false;
            } else if (time_since_last_segment_received() >= _cfg.rt_timeout * 10) {
                active_flag = false;
            }
        }
    }
    return active_flag;
}

size_t TCPConnection::write(const string &data) {
    size_t written = _sender.stream_in().write(data);
    _sender.fill_window();
    send_tcpsegment(false); // true ?
    return written;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    _sender.tick(ms_since_last_tick);
    time_since_last_segment_received_ += ms_since_last_tick;
    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {

    }
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();
    send_tcpsegment(true); // why ack here?
}

void TCPConnection::connect() {
    _sender.fill_window();
    send_tcpsegment(false);
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
