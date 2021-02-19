#include <base58/base58.h>

#include <cstdint>

namespace {

// preallocates the necessary size to decode base58 into binary. Usually the resulting binary data requires less space
// than base58, but in the worst case (base58 is just 1'es), each 1 is decoded into. So we use just input_size as the guess
// for how many bytes we need.
//
// We could do some processing by counting the number of initial ones, so we can make a very exact estimate of the number
// of bytes, but that doesn't seem to be worth it.
template <typename Container>
size_t prealloacate_decode(size_t input_size, Container& out) {
    // worst case is all 1'es: each byte is decoded as a zero.
    auto const old_size = out.size();
    out.resize(old_size + input_size + 1);
    return old_size;
}

template <typename Container>
size_t preallocate_encode(size_t input_size, Container& out) {
    auto const old_size = out.size();

    // Allocate enough space for base58 representation.
    // ln(256)/ln(58) = 1.365 bytes per b58 required. Instead of floating point operations we can approximate this by a
    // multiplication and division, e.g. by * 1365 / 1000. This is faster and has no floating point ambiguity. Note that
    // multiplier and divisor should be kept relatively small so we don't risk an overflow with input_size.
    //
    // Even better, we can choose a divisor that is a power of two so we can replace the division with a shift, which is even
    // faster: ln(256)/ln(58) * 2^8 = 349.6 To be on the safe side we round up and add 1.
    //
    // For 32bit size_t this will overflow at (2^32-1)/350 + 1 = 12271336. So you can't encode more than ~12 MB. But who would do
    // that anyways?
    auto const expected_encoded_size = ((input_size * 350) >> 8U) + 1U;
    out.resize(old_size + expected_encoded_size);
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
