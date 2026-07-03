#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace atursaja::util {

std::string trim(std::string_view input);
std::vector<std::string> split(std::string_view input, char delimiter, bool skip_empty = true);
std::string to_lower(std::string_view input);
bool starts_with(std::string_view input, std::string_view prefix);
std::unordered_map<std::string, std::string> parse_key_values(const std::vector<std::string>& tokens);

}  // namespace atursaja::util
