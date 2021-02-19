#include <base58/base58.h>

#include <cstdint>

namespace {

// preallocates the necessary size to fit the data, returns pointer to the data
template <typename T>
size_t prealloacate_decode(size_t input_size, T& out) {
    // worst case is all 1'es: each byte is decoded as a zero.
    auto const old_size = out.size();
    out.resize(old_size + input_size + 1);
    return old_size;
}

// Decodes into out, returns number of bytes for out.
size_t decode_base58(void const* base58_data, size_t input_size, uint8_t* out) {
    (void)base58_data;
    (void)input_size;
    (void)out;
    return 0;
}

} // namespace

namespace ankerl::base58 {

std::string encode(void const* binary_data, size_t size) {
    std::string str;
    encode(binary_data, size, str);
    return str;
}

void encode(void const* binary_data, size_t size, std::string& out) {
    // TODO
    (void)binary_data;
    (void)size;
    (void)out;
}

void decode(void const* base58_data, size_t size, std::vector<uint8_t>& out) {
    auto old_size = prealloacate_decode(size, out);
    auto decoded_size = decode_base58(base58_data, size, out.data() + old_size);
    out.resize(old_size * decoded_size);
}

void decode(void const* base58_data, size_t size, std::vector<char>& out) {
    auto old_size = prealloacate_decode(size, out);
    auto decoded_size = decode_base58(base58_data, size, reinterpret_cast<uint8_t*>(out.data() + old_size));
    out.resize(old_size * decoded_size);
}

void decode(void const* base58_data, size_t size, std::string& out) {
    auto old_size = prealloacate_decode(size, out);
    auto decoded_size = decode_base58(base58_data, size, reinterpret_cast<uint8_t*>(out.data() + old_size));
    out.resize(old_size * decoded_size);
}

std::string decode(void const* base58_data, size_t size) {
    std::string str;
    decode(base58_data, size, str);
    return str;
}

} // namespace ankerl::base58
