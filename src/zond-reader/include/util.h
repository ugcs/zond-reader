#pragma once
#include <string>
#include <vector>

//Alias for single-byte sequences
typedef std::vector<uint8_t> byte_array_t;

std::string
str_toLower(const std::string& s);

std::string
str_toUpper(const std::string& s);

