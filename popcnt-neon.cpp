
FORCE_INLINE uint64x2_t vpadalq(uint64x2_t sum, uint8x16_t t)
{
  return vpadalq_u32(sum, vpaddlq_u16(vpaddlq_u8(t)));
}


uint64_t popcnt_neon_vcnt(const uint8_t* data, const size_t size)
{
  uint64_t i = 0;
  uint64_t cnt = 0;
  uint64_t chunk_size = 64;

  if (size >= chunk_size)
  {
    uint64_t iters = size / chunk_size;
    const uint8_t* ptr = (const uint8_t*) data;
    uint64x2_t sum = vcombine_u64(vcreate_u64(0), vcreate_u64(0));
    uint8x16_t zero = vcombine_u8(vcreate_u8(0), vcreate_u8(0));

    do
    {
      uint8x16_t t0 = zero;
      uint8x16_t t1 = zero;
      uint8x16_t t2 = zero;
      uint8x16_t t3 = zero;

      /*
       * After every 31 iterations we need to add the
       * temporary sums (t0, t1, t2, t3) to the total sum.
       * We must ensure that the temporary sums <= 255
       * and 31 * 8 bits = 248 which is OK.
       */
      uint64_t limit = (i + 31 < iters) ? i + 31 : iters;

      /* Each iteration processes 64 bytes */
      for (; i < limit; i++)
      {
        uint8x16x4_t input = vld4q_u8(ptr);
        ptr += chunk_size;

        t0 = vaddq_u8(t0, vcntq_u8(input.val[0]));
        t1 = vaddq_u8(t1, vcntq_u8(input.val[1]));
        t2 = vaddq_u8(t2, vcntq_u8(input.val[2]));
        t3 = vaddq_u8(t3, vcntq_u8(input.val[3]));
      }

      sum = vpadalq(sum, t0);
      sum = vpadalq(sum, t1);
      sum = vpadalq(sum, t2);
      sum = vpadalq(sum, t3);
    }
    while (i < iters);

    uint64_t tmp[2];
    vst1q_u64(tmp, sum);
    cnt += tmp[0];
    cnt += tmp[1];

    /* Convert back to byte index */
    i *= chunk_size;
  }

  for (; i < size; i++) {
    cnt += lookup8bit[data[i]];
  }

  return cnt;
}


void FORCE_INLINE CSA(uint8x16_t& h, uint8x16_t& l, uint8x16_t a, uint8x16_t b, uint8x16_t c)
{
  uint8x16_t u = veorq_u8(a, b);
  h = vorrq_u8(vandq_u8(a, b), vandq_u8(u, c));
  l = veorq_u8(u, c);
}


uint32x2_t FORCE_INLINE popcnt_neon_qreg(const uint8x16_t reg) {
    
    const uint8x16_t pcnt = vcntq_u8(reg);
    const uint16x8_t t0 = vpaddlq_u8(pcnt);
    const uint32x4_t t1 = vpaddlq_u16(t0);
    const uint32x2_t t2 = vadd_u32(vget_low_u32(t1), vget_high_u32(t1));

    return t2;
}


uint64_t popcnt_neon_harley_seal(const uint8_t* data, const size_t size)
{
  uint32x2_t total = vdup_n_u32(0);
  uint8x16_t ones, twos, fours, eights, sixteens;
  uint8x16_t twosA, twosB, foursA, foursB, eightsA, eightsB;
  uint64_t limit = size - size % (16*16);
  uint64_t i = 0;

  ones = twos = fours = eights = sixteens = vdupq_n_u8(0);

  uint8_t* ptr = const_cast<uint8_t*>(data);

  for(; i < limit; i += 16*16)
  {
    CSA(twosA, ones, ones, vld1q_u8(ptr + 16*0), vld1q_u8(ptr + 16*1));
    CSA(twosB, ones, ones, vld1q_u8(ptr + 16*2), vld1q_u8(ptr + 16*3));
    CSA(foursA, twos, twos, twosA, twosB);
    CSA(twosA, ones, ones, vld1q_u8(ptr + 16*4), vld1q_u8(ptr + 16*5));
    CSA(twosB, ones, ones, vld1q_u8(ptr + 16*6), vld1q_u8(ptr + 16*7));
    CSA(foursB, twos, twos, twosA, twosB);
    CSA(eightsA,fours, fours, foursA, foursB);
    CSA(twosA, ones, ones, vld1q_u8(ptr + 16*8), vld1q_u8(ptr + 16*9));
    CSA(twosB, ones, ones, vld1q_u8(ptr + 16*10), vld1q_u8(ptr + 16*11));
    CSA(foursA, twos, twos, twosA, twosB);
    CSA(twosA, ones, ones, vld1q_u8(ptr + 16*12), vld1q_u8(ptr + 16*13));
    CSA(twosB, ones, ones, vld1q_u8(ptr + 16*14), vld1q_u8(ptr + 16*15));
    CSA(foursB, twos, twos, twosA, twosB);
    CSA(eightsB, fours, fours, foursA, foursB);
    CSA(sixteens, eights, eights, eightsA, eightsB);

    total = vadd_u32(total, popcnt_neon_qreg(sixteens));

    ptr += 16*16;
  }

  total  = vshl_n_u32(total, 4);
  total  = vadd_u32(total, vshl_n_u32(popcnt_neon_qreg(eights), 3));
  total  = vadd_u32(total, vshl_n_u32(popcnt_neon_qreg(fours), 2));
  total  = vadd_u32(total, vshl_n_u32(popcnt_neon_qreg(twos), 1));
  total  = vadd_u32(total, popcnt_neon_qreg(ones));

  uint32_t scalar = 0;
  
  scalar += vget_lane_u32(total, 0);
  scalar += vget_lane_u32(total, 1);

  for(; i < size; i++) {
    scalar += lookup8bit[*ptr++];
  }

  return scalar;
}
