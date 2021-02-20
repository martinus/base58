// Copyright (c) 2014-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "reference.h"

#include <vector>

/** All alphanumeric characters except for "0", "I", "O", and "l" */
static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

// Reference encoder, taken from Bitcoin: https://github.com/bitcoin/bitcoin/blob/master/src/base58.cpp#L87
// modifications done are:
// * replaced span with pointer and inputSize
// * str argument instead of return value
void encodeReference(void const* anyInput, size_t inputSize, std::string& str) {
    auto const* input = static_cast<uint8_t const*>(anyInput);

    // Skip & count leading zeroes.
    int zeroes = 0;
    int length = 0;
    while (inputSize > 0 && input[0] == 0) {
        --inputSize;
        ++input;
        zeroes++;
    }
    // Allocate enough space in big-endian base58 representation.
    int size = inputSize * 138 / 100 + 1; // log(256) / log(58), rounded up.
    std::vector<unsigned char> b58(size);
    // Process the bytes.
    while (inputSize > 0) {
        int carry = input[0];
        int i = 0;
        // Apply "b58 = b58 * 256 + ch".
        for (std::vector<unsigned char>::reverse_iterator it = b58.rbegin(); (carry != 0 || i < length) && (it != b58.rend());
             it++, i++) {
            carry += 256 * (*it);
            *it = carry % 58;
            carry /= 58;
        }

        // assert(carry == 0);
        length = i;
        --inputSize;
        ++input;
    }
    // Skip leading zeroes in base58 result.
    std::vector<unsigned char>::iterator it = b58.begin() + (size - length);
    while (it != b58.end() && *it == 0)
        it++;
    // Translate the result into a string.
    str.assign(zeroes, '1');
    while (it != b58.end())
        str += pszBase58[*(it++)];
}
