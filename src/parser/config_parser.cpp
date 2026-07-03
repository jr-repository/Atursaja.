#include "atursaja/parser/config_parser.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "atursaja/util/string_utils.hpp"

namespace atursaja::parser {
namespace {

using atursaja::core::RiskBand;
using atursaja::core::TaskSpec;
using atursaja::core::WorkflowSpec;

RiskBand parse_risk(std::string_view raw) {
    const auto normalized = util::to_lower(raw);
    if (normalized == "high") {
        return RiskBand::High;
    }
    if (normalized == "medium") {
        return RiskBand::Medium;
    }
    return RiskBand::Low;
}

std::unordered_map<std::string, int> parse_resources(std::string_view raw) {
    std::unordered_map<std::string, int> resources;
    for (const auto& token : util::split(raw, ',', true)) {
        const auto separator = token.find(':');
        if (separator == std::string::npos) {
            continue;
        }
        resources.emplace(token.substr(0, separator), std::stoi(token.substr(separator + 1)));
    }
    return resources;
}

std::vector<std::string> parse_list(std::string_view raw) {
    auto tokens = util::split(raw, ',', true);
    for (auto& token : tokens) {
        token = util::trim(token);
    }
    return tokens;
}

TaskSpec parse_task(const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        throw std::runtime_error("TASK line requires at least an id");
    }

    TaskSpec task;
    task.id = tokens[1];
    task.name = task.id;

    std::vector<std::string> remainder(tokens.begin() + 2, tokens.end());
    const auto kv = util::parse_key_values(remainder);

    if (const auto it = kv.find("name"); it != kv.end()) {
        task.name = it->second;
    }
    if (const auto it = kv.find("duration"); it != kv.end()) {
        task.duration_minutes = std::stoll(it->second);
    }
    if (const auto it = kv.find("priority"); it != kv.end()) {
        task.priority = std::stoi(it->second);
    }
    if (const auto it = kv.find("retries"); it != kv.end()) {
        task.retries = std::stoi(it->second);
    }
    if (const auto it = kv.find("fail"); it != kv.end()) {
        task.failure_probability = std::stod(it->second);
    }
    if (const auto it = kv.find("deadline"); it != kv.end()) {
        task.deadline_minutes = std::stoll(it->second);
    }
    if (const auto it = kv.find("uses"); it != kv.end()) {
        task.resource_demand = parse_resources(it->second);
    }
    if (const auto it = kv.find("depends"); it != kv.end()) {
        task.dependencies = parse_list(it->second);
    }
    if (const auto it = kv.find("tags"); it != kv.end()) {
        task.tags = parse_list(it->second);
    }
    if (const auto it = kv.find("risk"); it != kv.end()) {
        task.risk = parse_risk(it->second);
    }

    return task;
}

}  // namespace

util::Result<scheduling::Workflow> ConfigParser::parse_file(const std::string& path) const {
    std::ifstream input(path);
    if (!input) {
        return util::Result<scheduling::Workflow>::failure("unable to open scenario file: " + path);
    }

    std::stringstream buffer;
    buffer << input.rdbuf();
    return parse_text(buffer.str());
}

util::Result<scheduling::Workflow> ConfigParser::parse_text(const std::string& text) const {
    WorkflowSpec spec;
    std::stringstream stream(text);
    std::string line;
    int line_number = 0;

    try {
        while (std::getline(stream, line)) {
            ++line_number;
            line = util::trim(line);

            if (line.empty() || util::starts_with(line, "#")) {
                continue;
            }

            const auto tokens = util::split(line, ' ', true);
            if (tokens.empty()) {
                continue;
            }

            const auto directive = util::to_lower(tokens.front());
            if (directive == "project") {
                if (tokens.size() < 2) {
                    return util::Result<scheduling::Workflow>::failure(
                        "line " + std::to_string(line_number) + ": PROJECT requires a name");
                }
                spec.project_name = tokens[1];
            } else if (directive == "resource") {
                if (tokens.size() != 3) {
                    return util::Result<scheduling::Workflow>::failure(
                        "line " + std::to_string(line_number) + ": RESOURCE requires name and capacity");
                }
                spec.resource_capacities[tokens[1]] = std::stoi(tokens[2]);
            } else if (directive == "task") {
                spec.tasks.push_back(parse_task(tokens));
            } else {
                return util::Result<scheduling::Workflow>::failure(
                    "line " + std::to_string(line_number) + ": unknown directive '" + tokens.front() + "'");
            }
        }
    } catch (const std::exception& error) {
        return util::Result<scheduling::Workflow>::failure(
            "line " + std::to_string(line_number) + ": " + error.what());
    }

    scheduling::Workflow workflow(spec);
    const auto issues = workflow.validate();
    if (!issues.empty()) {
        std::string error = "workflow validation failed:";
        for (const auto& issue : issues) {
            error += "\n - " + issue;
        }
        return util::Result<scheduling::Workflow>::failure(error);
    }

    return util::Result<scheduling::Workflow>::success(std::move(workflow));
}

}  // namespace atursaja::parser
