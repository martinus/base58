#pragma once

#include <cstdint>
#include <string>

namespace ankerl::base58 {

// Encodes as base58 and appends the data.
// If possible, prefer this method as it can reuse the string's memory.
void encode(void const* binary_data, size_t size, std::string& out);

// Decodes base58 data, appending the binary data to out.
void decode(char const* b56_data, size_t size, std::string& out);

} // namespace ankerl::base58
