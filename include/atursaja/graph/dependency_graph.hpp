#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "atursaja/core/types.hpp"
#include "atursaja/scheduling/workflow.hpp"
#include "atursaja/util/result.hpp"

namespace atursaja::graph {

class DependencyGraph {
  public:
    explicit DependencyGraph(const scheduling::Workflow& workflow);

    util::Result<std::vector<std::string>> topological_order() const;
    util::Result<std::unordered_map<std::string, core::Minutes>> critical_path_lengths() const;
    [[nodiscard]] const std::vector<std::string>& dependents_of(const std::string& task_id) const;

  private:
    const scheduling::Workflow& workflow_;
    std::unordered_map<std::string, std::vector<std::string>> adjacency_;
    std::unordered_map<std::string, int> indegree_;
};

}  // namespace atursaja::graph
