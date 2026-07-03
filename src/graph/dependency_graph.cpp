#include "atursaja/graph/dependency_graph.hpp"

#include <algorithm>
#include <queue>

namespace atursaja::graph {

DependencyGraph::DependencyGraph(const scheduling::Workflow& workflow) : workflow_(workflow) {
    for (const auto& task : workflow_.tasks()) {
        adjacency_[task.id];
        indegree_[task.id] = static_cast<int>(task.dependencies.size());
    }

    for (const auto& task : workflow_.tasks()) {
        for (const auto& dependency : task.dependencies) {
            adjacency_[dependency].push_back(task.id);
        }
    }
}

util::Result<std::vector<std::string>> DependencyGraph::topological_order() const {
    std::queue<std::string> ready;
    auto indegree = indegree_;

    for (const auto& [task_id, degree] : indegree) {
        if (degree == 0) {
            ready.push(task_id);
        }
    }

    std::vector<std::string> ordered;
    while (!ready.empty()) {
        auto current = ready.front();
        ready.pop();
        ordered.push_back(current);

        for (const auto& dependent : adjacency_.at(current)) {
            auto& degree = indegree[dependent];
            --degree;
            if (degree == 0) {
                ready.push(dependent);
            }
        }
    }

    if (ordered.size() != workflow_.tasks().size()) {
        return util::Result<std::vector<std::string>>::failure(
            "dependency graph contains a cycle; scheduling is impossible");
    }

    return util::Result<std::vector<std::string>>::success(std::move(ordered));
}

util::Result<std::unordered_map<std::string, core::Minutes>> DependencyGraph::critical_path_lengths() const {
    auto topo_result = topological_order();
    if (!topo_result.ok()) {
        return util::Result<std::unordered_map<std::string, core::Minutes>>::failure(topo_result.error());
    }

    auto ordered = topo_result.value();
    std::reverse(ordered.begin(), ordered.end());

    std::unordered_map<std::string, core::Minutes> longest;
    for (const auto& task_id : ordered) {
        const auto* task = workflow_.find_task(task_id);
        core::Minutes downstream = 0;
        for (const auto& dependent : adjacency_.at(task_id)) {
            downstream = std::max(downstream, longest[dependent]);
        }
        longest[task_id] = task->duration_minutes + downstream;
    }

    return util::Result<std::unordered_map<std::string, core::Minutes>>::success(std::move(longest));
}

const std::vector<std::string>& DependencyGraph::dependents_of(const std::string& task_id) const {
    static const std::vector<std::string> empty;
    if (const auto it = adjacency_.find(task_id); it != adjacency_.end()) {
        return it->second;
    }
    return empty;
}

}  // namespace atursaja::graph
