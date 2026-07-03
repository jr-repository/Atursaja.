#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "atursaja/scheduling/scheduler.hpp"
#include "atursaja/scheduling/workflow.hpp"

namespace atursaja::simulation {

struct SimulationRecord {
    std::string id;
    core::Minutes planned_start{0};
    core::Minutes actual_start{0};
    core::Minutes actual_end{0};
    int attempts{0};
    bool succeeded{false};
    std::string note;
};

struct SimulationResult {
    std::vector<SimulationRecord> records;
    std::vector<std::string> event_log;
    core::Minutes planned_makespan{0};
    core::Minutes actual_makespan{0};
    int failed_tasks{0};
    core::Minutes total_drift{0};
};

class Simulator {
  public:
    explicit Simulator(std::uint32_t seed = 42);

    SimulationResult run(const scheduling::Workflow& workflow, const scheduling::SchedulePlan& plan) const;

  private:
    std::uint32_t seed_;
};

}  // namespace atursaja::simulation
