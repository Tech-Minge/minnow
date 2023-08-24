#include "stream_reassembler.hh"
#include <algorithm>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

InnerBuffer::InnerBuffer(const size_t capacity): buffer_(capacity), bitmap_(capacity) {}

void InnerBuffer::insert(const std::string &data, const size_t first_index, const size_t writer_available) {
    // empty str
    if (data.empty()) {
        return;
    }
    size_t discard_index = next_expected_index_ + writer_available;
    size_t start_index = max( first_index, next_expected_index_ );
    size_t end_index = min( data.size() + first_index, discard_index );

    // truncate data
    if ( start_index < discard_index && end_index >= next_expected_index_ ) {
        for (size_t i = start_index; i < end_index; ++i) {
            size_t relative_index = i - first_index;
            size_t buffer_index = i % bitmap_.size();
            if (!bitmap_[buffer_index]) {
                buffer_[buffer_index] = data[relative_index];
                bitmap_[buffer_index] = true;
                ++pending_bytes_;
            }
        }
    } // discard

}

size_t InnerBuffer::pending_bytes_number() const {
    return pending_bytes_;
}

std::string InnerBuffer::pop_writable_str() {
    std::string ret;
    size_t i;
    for (i = 0; i < bitmap_.size(); ++i) {
        size_t index = (i + next_expected_index_) % bitmap_.size();
        if (bitmap_[index]) {
            bitmap_[index] = false;
            ret.push_back(buffer_[index]);
        } else {
            break;
        }
    }
    pending_bytes_ -= i;
    next_expected_index_ += i;
    return ret;
}

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity), reassembler_buffer_(capacity) {}


//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // set close flag
    if ( eof ) {
        max_end_index_ = index + data.size();
        ready_to_close_ = true;
    }
    
    reassembler_buffer_.insert(data, index, _output.remaining_capacity());
    std::string able_to_write = reassembler_buffer_.pop_writable_str();
    if (able_to_write.size()) {
        _output.write(able_to_write);
    }

    // when no buffered data and set close flag -> truly close
    if ( ready_to_close_ && _output.bytes_written() == max_end_index_ ) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return reassembler_buffer_.pending_bytes_number(); }

bool StreamReassembler::empty() const { return reassembler_buffer_.pending_bytes_number() == 0; }
