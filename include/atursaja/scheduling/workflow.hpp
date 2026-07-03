#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "atursaja/core/task.hpp"

namespace atursaja::scheduling {

class Workflow {
  public:
    Workflow() = default;
    explicit Workflow(core::WorkflowSpec spec);

    [[nodiscard]] const std::string& name() const;
    [[nodiscard]] const std::unordered_map<std::string, int>& resource_capacities() const;
    [[nodiscard]] const std::vector<core::TaskSpec>& tasks() const;
    [[nodiscard]] const core::TaskSpec* find_task(const std::string& id) const;
    [[nodiscard]] std::vector<std::string> validate() const;

  private:
    core::WorkflowSpec spec_;
};

}  // namespace atursaja::scheduling
