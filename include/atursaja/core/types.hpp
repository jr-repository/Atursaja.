#pragma once

#include <cstdint>
#include <limits>
#include <string>

namespace atursaja::core {

using Minutes = std::int64_t;

inline constexpr Minutes kNoDeadline = std::numeric_limits<Minutes>::max();

enum class TaskState {
    Pending,
    Scheduled,
    Running,
    Completed,
    Failed,
    Skipped
};

enum class RiskBand {
    Low,
    Medium,
    High
};

inline std::string to_string(TaskState state) {
    switch (state) {
        case TaskState::Pending:
            return "pending";
        case TaskState::Scheduled:
            return "scheduled";
        case TaskState::Running:
            return "running";
        case TaskState::Completed:
            return "completed";
        case TaskState::Failed:
            return "failed";
        case TaskState::Skipped:
            return "skipped";
    }
    return "unknown";
}

inline std::string to_string(RiskBand risk) {
    switch (risk) {
        case RiskBand::Low:
            return "low";
        case RiskBand::Medium:
            return "medium";
        case RiskBand::High:
            return "high";
    }
    return "unknown";
}

}  // namespace atursaja::core
