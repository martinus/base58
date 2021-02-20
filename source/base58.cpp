#include <base58/base58.h>

#include <cstdint>
#include <utility>

namespace {

// preallocates the necessary size to decode base58 into binary.
template <typename Container>
size_t prealloacate_decode(size_t input_size, Container& out) {
    auto const old_size = out.size();

    // Usually the resulting binary data requires less space than base58, but in the worst case (base58 is just 1'es), each 1 is
    // decoded into. So we use just input_size as the guess for how many bytes we need.
    //
    // We could do some processing by counting the number of initial ones, so we can make a very exact estimate of the number
    // of bytes, but that doesn't seem to be worth it.
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

void encode(void const* const input_data, size_t input_size, std::string& out) {
    // Skip & count leading zeroes. Zeroes are simply encoded as '1'.
    auto const* input = static_cast<uint8_t const*>(input_data);
    auto const* const input_end = input + input_size;
    while (input != input_end && *input == 0) {
        ++input;
    }
    auto const skipped_leading_zeroes_size = std::distance(static_cast<uint8_t const*>(input_data), input);
    out.append(skipped_leading_zeroes_size, '1');
    input_size -= skipped_leading_zeroes_size;

    // Allocate enough space for base58 representation.
    //
    // ln(256)/ln(58) = 1.365 symbols of b58 are required per input byte. Instead of floating point operations we can approximate
    // this by a multiplication and division, e.g. by * 1365 / 1000. This is faster and has no floating point ambiguity. Note that
    // multiplier and divisor should be kept relatively small so we don't risk an overflow with input_size.
    //
    // Even better, we can choose a divisor that is a power of two so we can replace the division with a shift, which is even
    // faster: ln(256)/ln(58) * 2^8 = 349.6. To be on the safe side we round up and add 1.
    //
    // For 32bit size_t this will overflow at (2^32-1)/350 + 1 = 12271336. So you can't encode more than ~12 MB. But who would do
    // that anyways?
    auto const expected_encoded_size = ((input_size * 350) >> 8U) + 1U;

    // Instead of creating a temporary vector/string, we just operate in-place on the given string. This safes us at least one
    // malloc.
    out.append(expected_encoded_size, 0);
    auto* b58 = out.data() + out.size() - expected_encoded_size;
    auto* const b58rbegin = b58 + expected_encoded_size - 1;

    // we start with the remainder (or 7, if 0)
    // That way the last operation is a large one, which is a bit faster because each loop has to
    // go through the whole b58 data.
    size_t numBytesToProcess = ((input_size - 1) % 7) + 1;

    // Process the bytes.
    auto* it = b58rbegin;
    while (input != input_end) {
        // Encode at most 7 input bytes at once without risking an overflow. The largest value carry
        // can possibly have is by only 0xFF as input bytes, and 0x39 in b58:
        // 256^7-1 + 256^7 * 0x39 = 0x39FF'FFFF'FFFF'FFFF, which still fits into the 64bit.
        uint64_t carry = 0;
        uint64_t multiplier = 1;
        for (size_t numBytes = 0; numBytes < numBytesToProcess; ++numBytes) {
            carry = carry * 256 + *input++;
            multiplier *= 256;
        }
        numBytesToProcess = 7U;

        // Apply "b58 = b58 * multiplier + carry".
        auto const* b58rend = it;
        it = b58rbegin;

        // process all bytes from b58
        while (it > b58rend) {
            carry += multiplier * static_cast<uint8_t>(*it);
            *it-- = static_cast<char>(carry % 58);
            carry /= 58;
        }

        // finish with the carry. At most this will be executed ln(0x39FF'FFFF'FFFF'FFFF) / ln(58) = 10.6 = 11 times
        while (carry > 58 * 58) {
            *it-- = static_cast<char>(carry % 58);
            carry /= 58;
            *it-- = static_cast<char>(carry % 58);
            carry /= 58;
            *it-- = static_cast<char>(carry % 58);
            carry /= 58;
        }
        while (carry != 0) {
            *it-- = static_cast<char>(carry % 58);
            carry /= 58;
        }
    }

    // Translate the result into a string.
    ++it;
    while (it <= b58rbegin) {
        *b58++ = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"[static_cast<uint8_t>(*it++)];
    }
    out.resize(std::distance(out.data(), b58));
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
