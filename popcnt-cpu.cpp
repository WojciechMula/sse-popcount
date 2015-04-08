std::uint64_t popcnt_cpu_64bit(const uint8_t* data, const size_t n) {

    uint64_t result = 0;

    for (size_t i=0; i < n; i += 8) {
        const uint64_t v = *reinterpret_cast<const uint64_t*>(data + i);

        result += _popcnt64(v);
    }

    for (size_t i=8*(n/8); i < n; i++) {
        result += lookup8bit[data[i]];
    }

    return result;
}
