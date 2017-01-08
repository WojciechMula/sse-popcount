
uint64_t popcnt_neon_vcnt(const uint8_t* data, const size_t size)
{
    const size_t chunk_size = 16 * 4 * 2;

    uint8_t* ptr = const_cast<uint8_t*>(data);

    const size_t n = size / chunk_size;
    const size_t k = size % chunk_size;

    uint32x4_t sum = vcombine_u32(vcreate_u32(0), vcreate_u32(0));

    for (size_t i=0; i < n; i++, ptr += chunk_size) {

        uint8x16x4_t input0 = vld4q_u8(ptr + 0 * 16 * 4);
        uint8x16x4_t input1 = vld4q_u8(ptr + 1 * 16 * 4);

        uint8x16_t t0   = vcntq_u8(input0.val[0]);
        t0 = vaddq_u8(t0, vcntq_u8(input0.val[1]));
        t0 = vaddq_u8(t0, vcntq_u8(input0.val[2]));
        t0 = vaddq_u8(t0, vcntq_u8(input0.val[3]));

        t0 = vaddq_u8(t0, vcntq_u8(input1.val[0]));
        t0 = vaddq_u8(t0, vcntq_u8(input1.val[1]));
        t0 = vaddq_u8(t0, vcntq_u8(input1.val[2]));
        t0 = vaddq_u8(t0, vcntq_u8(input1.val[3]));

        const uint16x8_t t1 = vpaddlq_u8(t0);

        sum = vpadalq_u16(sum, t1);
    }

    uint32_t scalar = 0;
    uint32_t tmp[4];

    vst1q_u32(tmp, sum);
    for (int i=0; i < 4; i++) {
        scalar += tmp[i];
    }

    for (size_t j=0; j < k; j++) {
        scalar += lookup8bit[ptr[j]];
    }

    return scalar;
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
