// The most generic procedure
std::uint64_t popcnt_rvv_lookup(const uint8_t* data, const size_t n) {
    const unsigned vlenb      = __riscv_vlenb();
    const unsigned vlenb_m8   = vlenb * 8;
    const unsigned vlenb_log2 = __builtin_ctz(vlenb_m8);
    const unsigned vlenb_mask = ((1 << vlenb_log2) - 1);

    // Note: when VLEN * LMUL=8 >= 256, we can have a specialisation without the inner loop on `consumed`

    // 1. load lookup table
    const size_t vl_lookup = __riscv_vsetvl_e8m8(256);
    const vuint8m8_t lookup = __riscv_vle8_v_u8m8(lookup8bit, vl_lookup);

    std::uint64_t result = 0;

    const size_t vl = __riscv_vsetvlmax_e16m1();

    const vuint16m1_t zero = __riscv_vmv_v_x_u16m1(0, vl);

    // 2. process input in chunks
    const uint8_t* end = data + n;
    while (data + vlenb_m8 < end) {
        const size_t vl = __riscv_vsetvlmax_e8m8();
        vuint8m8_t vcounter = __riscv_vmv_v_x_u8m8(0, vl);

        for (int k=0; k < 255/8 && data + vlenb_m8 < end; k++, data += vlenb_m8) {
            // a. load input chunk
            vuint8m8_t input = __riscv_vle8_v_u8m8(data, vl);

            unsigned consumed = 0;
            while (consumed < 8) {
                // b. get lower log2 bits & add popcount
                const vuint8m8_t masked   = __riscv_vand_vx_u8m8(input, vlenb_mask, vl);
                const vuint8m8_t popcount = __riscv_vrgather_vv_u8m8(lookup, masked, vl);

                // c. update local pointer
                vcounter = __riscv_vadd_vv_u8m8(vcounter, popcount, vl);

                // d. bring upper, unprocessed bits
                input = __riscv_vsrl_vx_u8m8(input, vlenb_log2, vl);
                consumed += vlenb_log2;
            }
        }

        { // update the main counter
            vuint16m1_t tmp = __riscv_vwredsumu(vcounter, zero, vl);
            const uint16_t tmp0 = __riscv_vmv_x(tmp);
            result += uint64_t(tmp0);
        }
    }

    // 3. process tail
    while (data < end) {
        result += lookup8bit[*data];
        data += 1;
    }

    return result;
}
