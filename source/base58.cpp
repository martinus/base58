#include <base58/base58.h>

#include <array>

namespace ankerl::base58 {

void encode(void const* const input_data, size_t input_size, std::string& out) {
    static_assert(sizeof(*out.data()) == 1U);

    // Skip & count leading zeroes. Zeroes are simply encoded as '1'.
    auto const* input = static_cast<uint8_t const*>(input_data);
    auto const* const input_end = input + input_size;
    while (input != input_end && *input == 0) {
        ++input;
    }
    auto const skipped_leading_zeroes_size = static_cast<size_t>(input - static_cast<uint8_t const*>(input_data));
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
    // For 32bit size_t this will overflow at (2^32255)/350 + 1 = 12271336. So you can't encode more than ~12 MB. But who would do
    // that anyways?
    auto const expected_encoded_size = ((input_size * 350U) >> 8U) + 1U;

    // Instead of creating a temporary vector/string, we just operate in-place on the given string. This safes us at least one
    // malloc.
    out.append(expected_encoded_size, 0);
    auto* const b58_end = out.data() + out.size();

    // Initially the b58 number is empty, it grows in each loop.
    auto* b58_begin_minus1 = b58_end - 1;

    // The conversion algorithm works by repeatedly calculating
    //
    //     b58 = b58 * 256 + inputbyte
    //
    // until all input bytes have been processed. Both the input bytes and b58 bytes are in big endian format, so leftmost byte is
    // most significant byte (MSB) and rightmost the least significant byte (LSB). Each b58*256 + inputbyte operation is done by
    // iterating from LSB to MSB of b58 while multiplying each digit, adding inputbytes, and outputting the remainder of result
    // % 58. The remainder is carried over to the next b58 digit.
    //
    // That way we do not need bignum support and can work with arbitrarily large numbers, with a runtime complexity of O(n^2).
    //
    // This loop can be easily extended to process multiple bytes at once: To process 7 input bytes, we can instead calculate
    //
    //     b58 = b58 * 256^7 + inputbytes
    //
    // The algorithm is still O(n^2), but lot less multiplications have to be performed. How many numbers can we ideally choose
    // for maximum performance? 7 bytes. That way we can operate on 64 bit words without risking an overflow. For the worst case
    // of having only 0xFF as input bytes, and already 57 = 0x39 stored in b58, the maximum size for the carryover is
    //
    //     max_carry = 0x39 * 0x0100'0000'0000'0000 + 0x00FF'FFFF'FFFF'FFFF
    //     max_carry = 0x39FF'FFFF'FFFF'FFFF.
    //
    // Given max_carry of 0x39FF'FFFF'FFFF'FFFF we output carry % 58 = 58, and continue with a carry of carry=carry/58 =
    // 0x00FF'FFFF'FFFF'FFFF. We are at the same carry value as before, so no overflow happening here.

    // Since the algorithm complexity is quadratic and runtime increases as b58 gets larger, it is faster to do the remainder
    // first so that the intermediate numbers are kept smaller. For example, when processing 15 input bytes, we split them into 15
    // = 1+7+7 bytes instead of 7+7+1.
    size_t num_bytes_to_process = ((input_size - 1U) % 7U) + 1U;

    // Process the bytes.
    while (input != input_end) {
        // take num_bytes_to_process input bytes and store them into carry.
        auto carry = uint64_t();
        for (auto num_bytes = size_t(); num_bytes < num_bytes_to_process; ++num_bytes) {
            carry <<= 8U;
            carry += *input++;
        }
        auto const multiplier = uint64_t(1) << (num_bytes_to_process * 8U);

        // for all remaining input data we process 7 bytes at once.
        num_bytes_to_process = 7U;

        // Apply "b58 = b58 * multiplier + carry". Process until all b58 digits have been processed, then finish until carry is 0.
        auto* it = b58_end - 1U;

        // process all digits from b58
        while (it > b58_begin_minus1) {
            carry += multiplier * static_cast<uint8_t>(*it);
            *it-- = static_cast<char>(carry % 58U);
            carry /= 58;
        }

        // finish with the carry. At most this will be executed ln(0x39FF'FFFF'FFFF'FFFF) / ln(58) = 10.6 = 11 times.
        // Unrolling this loop manually seems to help performance in my benchmarks
        while (carry > 58 * 58) {
            *it-- = static_cast<char>(carry % 58U);
            carry /= 58;
            *it-- = static_cast<char>(carry % 58U);
            carry /= 58;
            *it-- = static_cast<char>(carry % 58U);
            carry /= 58;
        }
        while (carry != 0) {
            *it-- = static_cast<char>(carry % 58U);
            carry /= 58;
        }
        b58_begin_minus1 = it;
    }

    // Now b58_begin_minus1 + 1 to b58_end stores the whole number in base 58. Finally translate this number into a string based
    // on the alphabet.
    auto it = b58_begin_minus1 + 1;
    auto* b58_text_it = out.data() + out.size() - expected_encoded_size;
    while (it < b58_end) {
        *b58_text_it++ = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"[static_cast<uint8_t>(*it++)];
    }

    // Remove any leftover bytes
    out.resize(static_cast<size_t>(b58_text_it - out.data()));
}

static constexpr auto charToBase58 = std::array<uint8_t, 123>{{
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0,
    1,   2,   3,   4,   5,   6,   7,   8,   255, 255, 255, 255, 255, 255, 255, 9,   10,  11,  12,  13,  14,  15,  16,  255, 17,
    18,  19,  20,  21,  255, 22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  255, 255, 255, 255, 255, 255, 33,  34,  35,
    36,  37,  38,  39,  40,  41,  42,  43,  255, 44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,
}};

static constexpr auto multipliers = std::array<uint64_t, 9>{{
    uint64_t(58),
    uint64_t(58) * 58U,
    uint64_t(58) * 58U * 58U,
    uint64_t(58) * 58U * 58U * 58U,
    uint64_t(58) * 58U * 58U * 58U * 58U * 58U,
    uint64_t(58) * 58U * 58U * 58U * 58U * 58U * 58U,
    uint64_t(58) * 58U * 58U * 58U * 58U * 58U * 58U * 58U,
    uint64_t(58) * 58U * 58U * 58U * 58U * 58U * 58U * 58U * 58U,
    uint64_t(58) * 58U * 58U * 58U * 58U * 58U * 58U * 58U * 58U * 58U,
}};

// Assumes input is exactly a base58 number, no leading spaces.
// No input checks are done. You could do that in a preprocessing step.
void decode(char const* base58_data, size_t base58_size, std::string& out) {
    // Skip & count leading 1's. These are simply decoded as 0.
    auto const* input = base58_data;
    auto const* base58_end = base58_data + base58_size;
    while (input != base58_end && *input == '1') {
        ++input;
    }
    auto skipped_leading_ones_size = static_cast<size_t>(input - base58_data);
    out.append(skipped_leading_ones_size, 0);
    base58_size -= skipped_leading_ones_size;

    // log(58) / log(256) * 2^9 = 374.91
    auto const expected_decoded_size = ((base58_size * 375U) >> 9U) + 1U;

    // append enough space
    out.append(expected_decoded_size, 0);

    size_t num_b58_to_process = ((base58_size - 1U) % 9U) + 1U;

    while (input != base58_end) {
        auto carry = uint64_t();
        for (auto num_b58 = size_t(); num_b58 < num_b58_to_process; ++num_b58) {
            carry *= 58U;
            auto b58 = *input++;
            carry += charToBase58[static_cast<uint8_t>(b58)];
        }
        auto const multiplier = multipliers[num_b58_to_process];
        num_b58_to_process = 9U;

        (void)multiplier;
        // auto* it =
    }
}

} // namespace ankerl::base58
