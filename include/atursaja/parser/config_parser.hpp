#pragma once

#include <string>

#include "atursaja/scheduling/workflow.hpp"
#include "atursaja/util/result.hpp"

namespace atursaja::parser {

class ConfigParser {
  public:
    util::Result<scheduling::Workflow> parse_file(const std::string& path) const;
    util::Result<scheduling::Workflow> parse_text(const std::string& text) const;
};

}  // namespace atursaja::parser
