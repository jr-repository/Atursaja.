#pragma once

#include <string>

#include "atursaja/scheduling/scheduler.hpp"
#include "atursaja/scheduling/workflow.hpp"
#include "atursaja/simulation/simulator.hpp"

namespace atursaja::analytics {

class ReportBuilder {
  public:
    std::string render(const scheduling::Workflow& workflow,
                       const scheduling::SchedulePlan& plan,
                       const simulation::SimulationResult& simulation) const;
};

}  // namespace atursaja::analytics
