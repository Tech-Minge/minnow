#include "stream_reassembler.hh"
#include <algorithm>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {}

std::set<uint64_t>::iterator StreamReassembler::get_proper_iter( uint64_t key )
{
    auto iter = index_set_.upper_bound( key );
    if ( iter != index_set_.begin() ) {
        // find the biggest value that smaller than or equal start_index
        --iter;
    }
    return iter;
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // set close flag
    if ( eof ) {
        max_end_index_ = index + data.size();
        ready_to_close_ = true;
    }
    if (data.empty()) {
        if ( ready_to_close_ && next_expected_index_ == max_end_index_ ) {
            _output.end_input();
        }
        return;
    }
    
    auto writer_available = _output.remaining_capacity();
    auto discard_index = next_expected_index_ + writer_available;
    auto start_index = max( index, next_expected_index_ );
    auto end_index = min( data.size() + index, discard_index );
    string new_data;
  // truncate data
    if ( start_index < discard_index && end_index >= next_expected_index_ ) {
        new_data = data.substr( start_index - index, end_index - start_index );

        if ( index_set_.size() ) {
            // concat string
            auto low_iter = get_proper_iter( start_index );
            auto up_iter = get_proper_iter( end_index );
            auto low_index = *low_iter;
            auto up_index = *up_iter;
            if ( low_index < start_index && low_index + index_to_data_[low_index].size() >= start_index ) {
                new_data = index_to_data_[low_index].substr( 0, start_index - low_index ) + new_data;
                start_index = low_index;
            }
            if ( up_index <= end_index && up_index + index_to_data_[up_index].size() > end_index ) {
                new_data = new_data + index_to_data_[up_index].substr( end_index - up_index );
                end_index = up_index;
            }
            ++up_iter;

            for ( auto it = low_iter; it != up_iter; ) {
                auto cur_index = *it;
                auto end = index_to_data_[cur_index].size() + cur_index;
                if ( cur_index > end_index || end < start_index ) {
                    ++it;
                    continue;
                }
                pending_bytes_ -= index_to_data_[cur_index].size();
                index_to_data_.erase( cur_index );
                it = index_set_.erase( it );
            }
        }

        // 1. directly write
        if ( next_expected_index_ == start_index ) {
            _output.write( new_data );
            next_expected_index_ += new_data.size();
        } else {
            // 2. store in reassembler
            pending_bytes_ += new_data.size();
            index_to_data_[start_index] = new_data;
            index_set_.insert( start_index );
        }
    } // 3. discard

    
    // when no buffered data and set close flag -> truly close
    if ( ready_to_close_ && next_expected_index_ == max_end_index_ ) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return pending_bytes_; }

bool StreamReassembler::empty() const { return pending_bytes_ == 0; }
