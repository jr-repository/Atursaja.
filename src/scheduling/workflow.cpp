#include "atursaja/scheduling/workflow.hpp"

#include <unordered_set>

namespace atursaja::scheduling {

Workflow::Workflow(core::WorkflowSpec spec) : spec_(std::move(spec)) {}

const std::string& Workflow::name() const {
    return spec_.project_name;
}

const std::unordered_map<std::string, int>& Workflow::resource_capacities() const {
    return spec_.resource_capacities;
}

const std::vector<core::TaskSpec>& Workflow::tasks() const {
    return spec_.tasks;
}

const core::TaskSpec* Workflow::find_task(const std::string& id) const {
    for (const auto& task : spec_.tasks) {
        if (task.id == id) {
            return &task;
        }
    }
    return nullptr;
}

std::vector<std::string> Workflow::validate() const {
    std::vector<std::string> issues;
    std::unordered_set<std::string> ids;

    if (spec_.project_name.empty()) {
        issues.push_back("workflow project name is empty");
    }

    for (const auto& [resource, capacity] : spec_.resource_capacities) {
        if (resource.empty()) {
            issues.push_back("resource name cannot be empty");
        }
        if (capacity <= 0) {
            issues.push_back("resource '" + resource + "' must have positive capacity");
        }
    }

    for (const auto& task : spec_.tasks) {
        if (task.id.empty()) {
            issues.push_back("task id cannot be empty");
            continue;
        }
        if (!ids.insert(task.id).second) {
            issues.push_back("duplicate task id '" + task.id + "'");
        }
        if (task.duration_minutes <= 0) {
            issues.push_back("task '" + task.id + "' must have positive duration");
        }
        if (task.priority <= 0) {
            issues.push_back("task '" + task.id + "' must have positive priority");
        }
        if (task.retries < 0) {
            issues.push_back("task '" + task.id + "' cannot have negative retries");
        }
        if (task.failure_probability < 0.0 || task.failure_probability > 1.0) {
            issues.push_back("task '" + task.id + "' must have failure probability between 0 and 1");
        }
        for (const auto& [resource, demand] : task.resource_demand) {
            if (spec_.resource_capacities.find(resource) == spec_.resource_capacities.end()) {
                issues.push_back("task '" + task.id + "' references unknown resource '" + resource + "'");
            } else if (demand <= 0) {
                issues.push_back("task '" + task.id + "' must request positive units of resource '" + resource + "'");
            } else if (demand > spec_.resource_capacities.at(resource)) {
                issues.push_back("task '" + task.id + "' exceeds capacity of resource '" + resource + "'");
            }
        }
        for (const auto& dependency : task.dependencies) {
            if (dependency == task.id) {
                issues.push_back("task '" + task.id + "' cannot depend on itself");
            }
        }
    }

    for (const auto& task : spec_.tasks) {
        for (const auto& dependency : task.dependencies) {
            if (!ids.contains(dependency)) {
                issues.push_back("task '" + task.id + "' depends on unknown task '" + dependency + "'");
            }
        }
    }

    return issues;
}

}  // namespace atursaja::scheduling
