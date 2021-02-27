#pragma once

#include <cstddef>
#include <string>

std::string encodeReference(void const* anyInput, size_t inputSize);

bool decodeReference(const char* psz, std::string& vch, int max_ret_len);
