namespace {

/// This uses fewer arithmetic operations than any other known
/// implementation on machines with fast multiplication.
/// It uses 12 arithmetic operations, one of which is a multiply.
/// http://en.wikipedia.org/wiki/Hamming_weight#Efficient_implementation
///
uint64_t popcount_mul(uint64_t x)
{
  const uint64_t m1  = UINT64_C(0x5555555555555555);
  const uint64_t m2  = UINT64_C(0x3333333333333333);
  const uint64_t m4  = UINT64_C(0x0F0F0F0F0F0F0F0F);
  const uint64_t h01 = UINT64_C(0x0101010101010101);

  x -=            (x >> 1)  & m1;
  x = (x & m2) + ((x >> 2)  & m2);
  x = (x +        (x >> 4)) & m4;
  return (x * h01) >> 56;
}

/// Carry-save adder (CSA).
/// @see Chapter 5 in "Hacker's Delight".
///
void CSA(uint64_t& h, uint64_t& l, uint64_t a, uint64_t b, uint64_t c)
{
  uint64_t u = a ^ b;
  h = (a & b) | (u & c);
  l = u ^ c;
}

/// Harley-Seal popcount (4th iteration).
/// The Harley-Seal popcount algorithm is one of the fastest algorithms
/// for counting 1 bits in an array using only integer operations.
/// This implementation uses only 5.69 instructions per 64-bit word.
/// @see Chapter 5 in "Hacker's Delight" 2nd edition.
///
uint64_t popcnt_harley_seal_64bit(const uint64_t* data, const uint64_t size)
{
  uint64_t total = 0;
  uint64_t ones = 0, twos = 0, fours = 0, eights = 0, sixteens = 0;
  uint64_t twosA, twosB, foursA, foursB, eightsA, eightsB;
  uint64_t limit = size - size % 16;
  uint64_t i = 0;

  for(; i < limit; i += 16)
  {
    CSA(twosA, ones, ones, data[i+0], data[i+1]);
    CSA(twosB, ones, ones, data[i+2], data[i+3]);
    CSA(foursA, twos, twos, twosA, twosB);
    CSA(twosA, ones, ones, data[i+4], data[i+5]);
    CSA(twosB, ones, ones, data[i+6], data[i+7]);
    CSA(foursB, twos, twos, twosA, twosB);
    CSA(eightsA,fours, fours, foursA, foursB);
    CSA(twosA, ones, ones, data[i+8], data[i+9]);
    CSA(twosB, ones, ones, data[i+10], data[i+11]);
    CSA(foursA, twos, twos, twosA, twosB);
    CSA(twosA, ones, ones, data[i+12], data[i+13]);
    CSA(twosB, ones, ones, data[i+14], data[i+15]);
    CSA(foursB, twos, twos, twosA, twosB);
    CSA(eightsB, fours, fours, foursA, foursB);
    CSA(sixteens, eights, eights, eightsA, eightsB);

    total += popcount_mul(sixteens);
  }

  total *= 16;
  total += 8 * popcount_mul(eights);
  total += 4 * popcount_mul(fours);
  total += 2 * popcount_mul(twos);
  total += 1 * popcount_mul(ones);

  for(; i < size; i++)
    total += popcount_mul(data[i]);

  return total;
}

} // namespace

uint64_t popcnt_harley_seal(const uint8_t* data, const size_t size)
{
  uint64_t total = popcnt_harley_seal_64bit((const uint64_t*) data, size / 8);

  for (size_t i = size - size % 8; i < size; i++)
    total += lookup8bit[data[i]];

  return total;
}
