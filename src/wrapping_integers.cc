#include "wrapping_integers.hh"
#include <cmath>
using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  return Wrap32( zero_point ) + static_cast<uint32_t>( n );
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  uint32_t small = checkpoint & 0xFFFFFFFF;
  uint32_t offset = raw_value_ - zero_point.raw_value_;
  uint64_t bound = checkpoint & ( ~0xFFFFFFFFL );
  uint64_t res = bound + offset;
  if ( bound && offset > small && offset - small > ( 1L << 31 ) ) {
    res -= ( 1L << 32 );
  } else if ( small > offset && small - offset > ( 1L << 31 ) ) {
    res += ( 1L << 32 );
  }
  return res;
}
