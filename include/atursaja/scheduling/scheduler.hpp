#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "atursaja/core/types.hpp"
#include "atursaja/scheduling/workflow.hpp"
#include "atursaja/util/result.hpp"

namespace atursaja::scheduling {

struct ScheduledTask {
    std::string id;
    core::Minutes start_minutes{0};
    core::Minutes end_minutes{0};
    double scheduling_score{0.0};
};

struct SchedulePlan {
    std::vector<ScheduledTask> tasks;
    std::vector<std::string> warnings;
    std::unordered_map<std::string, core::Minutes> critical_path_lengths;
    core::Minutes makespan{0};
};

class Scheduler {
  public:
    util::Result<SchedulePlan> build_plan(const Workflow& workflow) const;
};

}  // namespace atursaja::scheduling
