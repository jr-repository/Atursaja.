#include "atursaja/simulation/simulator.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <sstream>
#include <unordered_map>

namespace atursaja::simulation {
namespace {

double risk_failure_boost(core::RiskBand risk) {
    switch (risk) {
        case core::RiskBand::High:
            return 0.08;
        case core::RiskBand::Medium:
            return 0.03;
        case core::RiskBand::Low:
            return 0.0;
    }
    return 0.0;
}

double resource_pressure(const core::TaskSpec& task, const scheduling::Workflow& workflow) {
    if (task.resource_demand.empty()) {
        return 0.0;
    }

    double pressure = 0.0;
    for (const auto& [resource, demand] : task.resource_demand) {
        pressure += static_cast<double>(demand) / workflow.resource_capacities().at(resource);
    }
    return pressure / static_cast<double>(task.resource_demand.size());
}

}  // namespace

Simulator::Simulator(std::uint32_t seed) : seed_(seed) {}

SimulationResult Simulator::run(const scheduling::Workflow& workflow, const scheduling::SchedulePlan& plan) const {
    SimulationResult result;
    result.planned_makespan = plan.makespan;

    std::mt19937 generator(seed_);
    std::uniform_real_distribution<double> duration_jitter(-0.10, 0.25);

    auto ordered = plan.tasks;
    std::sort(ordered.begin(), ordered.end(), [](const auto& left, const auto& right) {
        if (left.start_minutes == right.start_minutes) {
            return left.end_minutes < right.end_minutes;
        }
        return left.start_minutes < right.start_minutes;
    });

    std::unordered_map<std::string, core::Minutes> completed_at;
    std::unordered_map<std::string, bool> succeeded;

    for (const auto& scheduled : ordered) {
        const auto* task = workflow.find_task(scheduled.id);

        core::Minutes dependency_ready = 0;
        bool blocked = false;
        for (const auto& dependency : task->dependencies) {
            dependency_ready = std::max(dependency_ready, completed_at[dependency]);
            if (!succeeded[dependency]) {
                blocked = true;
            }
        }

        SimulationRecord record;
        record.id = task->id;
        record.planned_start = scheduled.start_minutes;
        record.actual_start = std::max(scheduled.start_minutes, dependency_ready);

        if (blocked) {
            record.actual_end = record.actual_start;
            record.succeeded = false;
            record.note = "blocked by failed dependency";
            completed_at[task->id] = record.actual_end;
            succeeded[task->id] = false;
            result.failed_tasks += 1;
            result.records.push_back(record);
            result.event_log.push_back("minute " + std::to_string(record.actual_start) + ": " + task->id +
                                       " skipped because an upstream task failed");
            continue;
        }

        const auto base_duration = std::max<core::Minutes>(1, task->duration_minutes);
        const double pressure = resource_pressure(*task, workflow);
        const double jitter = duration_jitter(generator);
        const auto effective_duration = std::max<core::Minutes>(
            1, static_cast<core::Minutes>(std::llround(base_duration * (1.0 + jitter + pressure * 0.12))));

        const auto failure_probability = std::min(0.95, task->failure_probability + risk_failure_boost(task->risk));
        std::bernoulli_distribution failure_roll(failure_probability);

        core::Minutes cursor = record.actual_start;
        for (int attempt = 1; attempt <= task->retries + 1; ++attempt) {
            record.attempts = attempt;
            const auto attempt_duration = effective_duration + static_cast<core::Minutes>((attempt - 1) * 7);
            result.event_log.push_back("minute " + std::to_string(cursor) + ": " + task->id + " attempt " +
                                       std::to_string(attempt) + " started");

            const auto finished_at = cursor + attempt_duration;
            const bool failed = failure_roll(generator);
            if (!failed) {
                record.actual_end = finished_at;
                record.succeeded = true;
                record.note = attempt == 1 ? "completed on first attempt" : "recovered after retries";
                result.event_log.push_back("minute " + std::to_string(finished_at) + ": " + task->id + " completed");
                break;
            }

            result.event_log.push_back("minute " + std::to_string(finished_at) + ": " + task->id + " failed");
            if (attempt <= task->retries) {
                cursor = finished_at + static_cast<core::Minutes>(10 * attempt);
                continue;
            }

            record.actual_end = finished_at;
            record.succeeded = false;
            record.note = "retries exhausted";
        }

        completed_at[task->id] = record.actual_end;
        succeeded[task->id] = record.succeeded;
        result.actual_makespan = std::max(result.actual_makespan, record.actual_end);
        if (!record.succeeded) {
            result.failed_tasks += 1;
        }

        result.total_drift += std::max<core::Minutes>(0, record.actual_end - scheduled.end_minutes);
        result.records.push_back(record);
    }

    return result;
}

}  // namespace atursaja::simulation
