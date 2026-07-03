#include "atursaja/util/string_utils.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace atursaja::util {

std::string trim(std::string_view input) {
    std::size_t start = 0;
    while (start < input.size() && std::isspace(static_cast<unsigned char>(input[start])) != 0) {
        ++start;
    }

    std::size_t end = input.size();
    while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1])) != 0) {
        --end;
    }

    return std::string(input.substr(start, end - start));
}

std::vector<std::string> split(std::string_view input, char delimiter, bool skip_empty) {
    std::vector<std::string> parts;
    std::string current;

    for (char ch : input) {
        if (ch == delimiter) {
            if (!current.empty() || !skip_empty) {
                parts.push_back(current);
            }
            current.clear();
            continue;
        }
        current.push_back(ch);
    }

    if (!current.empty() || !skip_empty) {
        parts.push_back(current);
    }

    return parts;
}

std::string to_lower(std::string_view input) {
    std::string lowered(input.begin(), input.end());
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return lowered;
}

bool starts_with(std::string_view input, std::string_view prefix) {
    return input.size() >= prefix.size() && input.substr(0, prefix.size()) == prefix;
}

std::unordered_map<std::string, std::string> parse_key_values(const std::vector<std::string>& tokens) {
    std::unordered_map<std::string, std::string> values;
    for (const auto& token : tokens) {
        const auto separator = token.find('=');
        if (separator == std::string::npos) {
            continue;
        }
        values.emplace(token.substr(0, separator), token.substr(separator + 1));
    }
    return values;
}

}  // namespace atursaja::util
