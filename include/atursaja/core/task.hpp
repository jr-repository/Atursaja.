#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "atursaja/core/types.hpp"

namespace atursaja::core {

struct TaskSpec {
    std::string id;
    std::string name;
    Minutes duration_minutes{0};
    std::unordered_map<std::string, int> resource_demand;
    std::vector<std::string> dependencies;
    std::vector<std::string> tags;
    int priority{1};
    int retries{0};
    double failure_probability{0.0};
    Minutes deadline_minutes{kNoDeadline};
    RiskBand risk{RiskBand::Low};
};

struct WorkflowSpec {
    std::string project_name;
    std::unordered_map<std::string, int> resource_capacities;
    std::vector<TaskSpec> tasks;
};

}  // namespace atursaja::core
