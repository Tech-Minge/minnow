#include "byte_stream.hh"
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  uint64_t push_num = min( data.size(), capacity_ - data_.size() );
  data_.append(data.begin(), data.begin() + push_num);
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
  return capacity_ - data_.size();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return total_bytes_pushed_;
}

string_view Reader::peek() const
{
  // Your code here.
  size_t stop = min(peek_size, data_.size());
  return string_view(data_.begin(), data_.begin() + stop);
}

bool Reader::is_finished() const
{
  // Your code here.
  return closed_ && data_.size() == 0;
}

bool Reader::has_error() const
{
  // Your code here.
  return error_;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  uint64_t pop_num = min( len, data_.size() );
  data_.erase(0, pop_num);
  total_bytes_poped += pop_num;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return data_.size();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return total_bytes_poped;
}
