#include "atursaja/analytics/report.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <unordered_map>

namespace atursaja::analytics {
namespace {

std::unordered_map<std::string, double> compute_resource_utilization(const scheduling::Workflow& workflow,
                                                                     const scheduling::SchedulePlan& plan) {
    std::unordered_map<std::string, double> usage_minutes;
    for (const auto& scheduled : plan.tasks) {
        const auto* task = workflow.find_task(scheduled.id);
        for (const auto& [resource, demand] : task->resource_demand) {
            usage_minutes[resource] += static_cast<double>(demand) * (scheduled.end_minutes - scheduled.start_minutes);
        }
    }

    std::unordered_map<std::string, double> utilization;
    for (const auto& [resource, capacity] : workflow.resource_capacities()) {
        const auto denominator = static_cast<double>(std::max<core::Minutes>(1, plan.makespan)) * capacity;
        utilization[resource] = denominator == 0.0 ? 0.0 : (usage_minutes[resource] / denominator) * 100.0;
    }
    return utilization;
}

}  // namespace

std::string ReportBuilder::render(const scheduling::Workflow& workflow,
                                  const scheduling::SchedulePlan& plan,
                                  const simulation::SimulationResult& simulation) const {
    std::ostringstream out;
    out << "=== AturSaja Hyper Orchestrator Report ===\n";
    out << "project: " << workflow.name() << "\n";
    out << "task_count: " << workflow.tasks().size() << "\n";
    out << "planned_makespan: " << plan.makespan << " minutes\n";
    out << "actual_makespan: " << simulation.actual_makespan << " minutes\n";
    out << "failed_tasks: " << simulation.failed_tasks << "\n";
    out << "total_drift: " << simulation.total_drift << " minutes\n\n";

    out << "[resource_utilization]\n";
    const auto utilization = compute_resource_utilization(workflow, plan);
    for (const auto& [resource, value] : utilization) {
        out << " - " << resource << ": " << std::fixed << std::setprecision(2) << value << "%\n";
    }

    out << "\n[schedule]\n";
    for (const auto& task : plan.tasks) {
        const auto* spec = workflow.find_task(task.id);
        out << " - " << task.id << " start=" << task.start_minutes << " end=" << task.end_minutes
            << " score=" << std::fixed << std::setprecision(1) << task.scheduling_score
            << " cp=" << plan.critical_path_lengths.at(task.id) << " risk=" << core::to_string(spec->risk) << "\n";
    }

    out << "\n[simulation]\n";
    for (const auto& record : simulation.records) {
        out << " - " << record.id << " planned=" << record.planned_start << " actual_start=" << record.actual_start
            << " actual_end=" << record.actual_end << " attempts=" << record.attempts
            << " status=" << (record.succeeded ? "ok" : "failed") << " note=" << record.note << "\n";
    }

    if (!plan.warnings.empty()) {
        out << "\n[warnings]\n";
        for (const auto& warning : plan.warnings) {
            out << " - " << warning << "\n";
        }
    }

    out << "\n[event_log]\n";
    const auto limit = std::min<std::size_t>(simulation.event_log.size(), 20);
    for (std::size_t index = 0; index < limit; ++index) {
        out << " - " << simulation.event_log[index] << "\n";
    }
    if (simulation.event_log.size() > limit) {
        out << " - ... (" << simulation.event_log.size() - limit << " more events)\n";
    }

    return out.str();
}

}  // namespace atursaja::analytics
