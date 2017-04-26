// this is direct translation of popcnt_neon_vcnt

uint64_t popcnt_aarch64_cnt(const uint8_t* data, const size_t size)
{
    const size_t chunk_size = 16 * 4 * 4;

    uint8_t* ptr = const_cast<uint8_t*>(data);

    const size_t n = size / chunk_size;
    const size_t k = size % chunk_size;

    uint32x4_t sum = vcombine_u32(vcreate_u32(0), vcreate_u32(0));

    for (size_t i=0; i < n; i++, ptr += chunk_size) {

        uint8x16x4_t input0 = vld4q_u8(ptr + 0 * 16 * 4);
        uint8x16x4_t input1 = vld4q_u8(ptr + 1 * 16 * 4);
        uint8x16x4_t input2 = vld4q_u8(ptr + 2 * 16 * 4);
        uint8x16x4_t input3 = vld4q_u8(ptr + 3 * 16 * 4);

        uint8x16_t t0   = vcntq_u8(input0.val[0]);
        t0 = vaddq_u8(t0, vcntq_u8(input0.val[1]));
        t0 = vaddq_u8(t0, vcntq_u8(input0.val[2]));
        t0 = vaddq_u8(t0, vcntq_u8(input0.val[3]));

        t0 = vaddq_u8(t0, vcntq_u8(input1.val[0]));
        t0 = vaddq_u8(t0, vcntq_u8(input1.val[1]));
        t0 = vaddq_u8(t0, vcntq_u8(input1.val[2]));
        t0 = vaddq_u8(t0, vcntq_u8(input1.val[3]));

        t0 = vaddq_u8(t0, vcntq_u8(input2.val[0]));
        t0 = vaddq_u8(t0, vcntq_u8(input2.val[1]));
        t0 = vaddq_u8(t0, vcntq_u8(input2.val[2]));
        t0 = vaddq_u8(t0, vcntq_u8(input2.val[3]));

        t0 = vaddq_u8(t0, vcntq_u8(input3.val[0]));
        t0 = vaddq_u8(t0, vcntq_u8(input3.val[1]));
        t0 = vaddq_u8(t0, vcntq_u8(input3.val[2]));
        t0 = vaddq_u8(t0, vcntq_u8(input3.val[3]));

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

