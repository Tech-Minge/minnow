#include <stdexcept>
#include <algorithm>
#include "byte_stream.hh"
#include <iostream>
using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  uint64_t push_num = min(data.size(), capacity_ - data_queue_.size());
  if (push_num && !data_queue_.size()) {
    view_[0] = data[0];
  }
  for (uint64_t i = 0; i < push_num; ++i) {
    data_queue_.push(data[i]);
  }
  
  total_bytes_pushed_ += push_num;
}

void Writer::close()
{
  // Your code here.
  closed_ = true;
}

void Writer::set_error()
{
  // Your code here.
  error_ = true;
}

bool Writer::is_closed() const
{
  // Your code here.
  return closed_;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - data_queue_.size();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return total_bytes_pushed_;
}

string_view Reader::peek() const
{
  // Your code here.
  return view_;
}

bool Reader::is_finished() const
{
  // Your code here.
  return closed_ && data_queue_.size() == 0;
}

bool Reader::has_error() const
{
  // Your code here.
  return error_;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  uint64_t pop_num = min(len, data_queue_.size());
  for (uint64_t i = 0; i < pop_num; ++i) {
    data_queue_.pop();
  }
  if (data_queue_.size()) {
    view_[0] = data_queue_.front();
  }
  total_bytes_poped += pop_num;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return data_queue_.size();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return total_bytes_poped;
}
