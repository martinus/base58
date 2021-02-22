#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ankerl::base58 {

// Encodes as base58 and appends the data.
// If possible, prefer this method as it can reuse the string's memory.
void encode(void const* binary_data, size_t size, std::string& out);

// Encodes as base58.
// This needs one allocation.
std::string encode(void const* binary_data, size_t size);

// Decodes base58 data, and appends the binary data to out.
void decode(void const* base58_data, size_t size, std::string& out);
std::string decode(void const* base58_data, size_t size);

} // namespace ankerl::base58
