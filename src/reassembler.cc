#include "reassembler.hh"
#include <algorithm>

using namespace std;

std::set<uint64_t>::iterator Reassembler::get_proper_iter( uint64_t key )
{
  auto iter = index_set_.upper_bound( key );
  if ( iter != index_set_.begin() ) {
    // find the biggest value that smaller than or equal start_index
    --iter;
  }
  return iter;
}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // Your code here.
  auto writer_available = output.available_capacity();
  auto discard_index = next_expected_index_ + writer_available;
  auto start_index = max( first_index, next_expected_index_ );
  auto end_index = min( data.size() + first_index, discard_index );

  // truncate data
  if ( start_index < discard_index && end_index >= next_expected_index_ ) {
    data = data.substr( start_index - first_index, end_index - start_index );

    if ( index_set_.size() ) {
      // concat string
      auto low_iter = get_proper_iter( start_index );
      auto up_iter = get_proper_iter( end_index );
      auto low_index = *low_iter;
      auto up_index = *up_iter;
      if ( low_index < start_index && low_index + index_to_data_[low_index].size() >= start_index ) {
        data = index_to_data_[low_index].substr( 0, start_index - low_index ) + data;
        start_index = low_index;
      }
      if ( up_index <= end_index && up_index + index_to_data_[up_index].size() > end_index ) {
        data = data + index_to_data_[up_index].substr( end_index - up_index );
        end_index = up_index;
      }
      ++up_iter;

      for ( auto it = low_iter; it != up_iter; ) {
        auto index = *it;
        auto end = index_to_data_[index].size() + index;
        if ( index > end_index || end < start_index ) {
          ++it;
          continue;
        }
        pending_bytes_ -= index_to_data_[index].size();
        index_to_data_.erase( index );
        it = index_set_.erase( it );
      }
    }

    // 1. directly write
    if ( next_expected_index_ == start_index ) {
      output.push( data );
      next_expected_index_ += data.size();
    } else {
      // 2. store in reassembler
      pending_bytes_ += data.size();
      index_to_data_[start_index] = data;
      index_set_.insert( start_index );
    }
  } // 3. discard

  // set close flag
  if ( is_last_substring ) {
    ready_to_close_ = true;
  }
  // when no buffered data and set close flag -> truly close
  if ( ready_to_close_ && !index_to_data_.size() ) {
    output.close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return pending_bytes_;
}
