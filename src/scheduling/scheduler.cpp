#include "atursaja/scheduling/scheduler.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>

#include "atursaja/graph/dependency_graph.hpp"

namespace atursaja::scheduling {
namespace {

struct ActiveTask {
    std::string id;
    core::Minutes end_minutes{0};
};

double risk_weight(core::RiskBand risk) {
    switch (risk) {
        case core::RiskBand::High:
            return 250.0;
        case core::RiskBand::Medium:
            return 125.0;
        case core::RiskBand::Low:
            return 40.0;
    }
    return 0.0;
}

double compute_score(const core::TaskSpec& task,
                     const std::unordered_map<std::string, core::Minutes>& critical_path_lengths,
                     const std::unordered_map<std::string, int>& capacities,
                     core::Minutes current_time) {
    const auto critical = static_cast<double>(critical_path_lengths.at(task.id));
    double pressure = 0.0;
    for (const auto& [resource, demand] : task.resource_demand) {
        const auto capacity = static_cast<double>(capacities.at(resource));
        pressure += static_cast<double>(demand) / capacity;
    }

    double deadline_urgency = 0.0;
    if (task.deadline_minutes != core::kNoDeadline) {
        const auto slack = static_cast<double>(task.deadline_minutes - current_time - task.duration_minutes);
        deadline_urgency = slack <= 0.0 ? 600.0 : std::max(0.0, 300.0 - (slack / 10.0));
    }

    return static_cast<double>(task.priority) * 1000.0 + critical * 10.0 + deadline_urgency + risk_weight(task.risk) -
           pressure * 75.0;
}

bool can_allocate(const core::TaskSpec& task,
                  const std::unordered_map<std::string, int>& capacities,
                  const std::unordered_map<std::string, int>& used) {
    for (const auto& [resource, demand] : task.resource_demand) {
        const auto usage = used.contains(resource) ? used.at(resource) : 0;
        if (usage + demand > capacities.at(resource)) {
            return false;
        }
    }
    return true;
}

void allocate(const core::TaskSpec& task, std::unordered_map<std::string, int>& used, int sign) {
    for (const auto& [resource, demand] : task.resource_demand) {
        used[resource] += sign * demand;
    }
}

}  // namespace

util::Result<SchedulePlan> Scheduler::build_plan(const Workflow& workflow) const {
    graph::DependencyGraph graph(workflow);

    auto topo_result = graph.topological_order();
    if (!topo_result.ok()) {
        return util::Result<SchedulePlan>::failure(topo_result.error());
    }

    auto critical_result = graph.critical_path_lengths();
    if (!critical_result.ok()) {
        return util::Result<SchedulePlan>::failure(critical_result.error());
    }

    std::unordered_map<std::string, int> remaining_indegree;
    std::unordered_map<std::string, std::vector<std::string>> dependencies;
    std::unordered_map<std::string, const core::TaskSpec*> tasks_by_id;
    for (const auto& task : workflow.tasks()) {
        remaining_indegree[task.id] = static_cast<int>(task.dependencies.size());
        dependencies[task.id] = task.dependencies;
        tasks_by_id[task.id] = &task;
    }

    SchedulePlan plan;
    plan.critical_path_lengths = critical_result.value();

    std::unordered_set<std::string> scheduled;
    std::unordered_map<std::string, int> used_resources;
    std::vector<ActiveTask> running;
    core::Minutes current_time = 0;

    auto release_completed = [&]() {
        std::vector<ActiveTask> survivors;
        for (const auto& active : running) {
            if (active.end_minutes > current_time) {
                survivors.push_back(active);
                continue;
            }
            const auto* finished = tasks_by_id.at(active.id);
            allocate(*finished, used_resources, -1);
            for (const auto& dependent : graph.dependents_of(active.id)) {
                --remaining_indegree[dependent];
            }
        }
        running = std::move(survivors);
    };

    while (scheduled.size() < workflow.tasks().size()) {
        release_completed();

        std::vector<const core::TaskSpec*> ready;
        for (const auto& task : workflow.tasks()) {
            if (!scheduled.contains(task.id) && remaining_indegree[task.id] == 0) {
                ready.push_back(&task);
            }
        }

        std::sort(ready.begin(), ready.end(), [&](const auto* left, const auto* right) {
            return compute_score(*left, plan.critical_path_lengths, workflow.resource_capacities(), current_time) >
                   compute_score(*right, plan.critical_path_lengths, workflow.resource_capacities(), current_time);
        });

        bool any_scheduled = false;
        for (const auto* task : ready) {
            if (scheduled.contains(task->id) || !can_allocate(*task, workflow.resource_capacities(), used_resources)) {
                continue;
            }

            const auto score =
                compute_score(*task, plan.critical_path_lengths, workflow.resource_capacities(), current_time);
            allocate(*task, used_resources, 1);
            running.push_back({task->id, current_time + task->duration_minutes});
            plan.tasks.push_back({task->id, current_time, current_time + task->duration_minutes, score});
            scheduled.insert(task->id);
            any_scheduled = true;
        }

        if (any_scheduled) {
            continue;
        }

        if (running.empty()) {
            return util::Result<SchedulePlan>::failure(
                "scheduler stalled: no runnable tasks and no active tasks; check workflow constraints");
        }

        const auto next_event = std::min_element(running.begin(), running.end(), [](const auto& left, const auto& right) {
            return left.end_minutes < right.end_minutes;
        });
        current_time = next_event->end_minutes;
    }

    std::sort(plan.tasks.begin(), plan.tasks.end(), [](const auto& left, const auto& right) {
        if (left.start_minutes == right.start_minutes) {
            return left.end_minutes < right.end_minutes;
        }
        return left.start_minutes < right.start_minutes;
    });

    for (const auto& task : plan.tasks) {
        plan.makespan = std::max(plan.makespan, task.end_minutes);
        const auto* spec = workflow.find_task(task.id);
        if (spec->deadline_minutes != core::kNoDeadline && task.end_minutes > spec->deadline_minutes) {
            plan.warnings.push_back("task '" + task.id + "' misses deadline at minute " +
                                    std::to_string(task.end_minutes));
        }
    }

    return util::Result<SchedulePlan>::success(std::move(plan));
}

}  // namespace atursaja::scheduling
