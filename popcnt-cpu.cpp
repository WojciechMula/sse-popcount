std::uint64_t popcnt_cpu_64bit(const uint8_t* data, const size_t n) {

    uint64_t result = 0;

    uint64_t v, i = 0;
#define ITER { \
        v = *reinterpret_cast<const uint64_t*>(data + i); \
        result += _popcnt64(v); \
        i += 8; \
    }

    while (i + 4*8 <= n) {
        ITER ITER ITER ITER
    }

#undef ITER

    while (i < n) {
        result += lookup8bit[data[i]];
        i++;
    }

    return result;
}
