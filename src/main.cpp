#include <filesystem>
#include <iostream>
#include <string>

#include "atursaja/analytics/report.hpp"
#include "atursaja/parser/config_parser.hpp"
#include "atursaja/scheduling/scheduler.hpp"
#include "atursaja/simulation/simulator.hpp"

int main(int argc, char** argv) {
    const std::string scenario_path =
        argc > 1 ? argv[1] : (std::filesystem::path("examples") / "hyper_factory.asj").string();

    atursaja::parser::ConfigParser parser;
    auto workflow_result = parser.parse_file(scenario_path);
    if (!workflow_result.ok()) {
        std::cerr << workflow_result.error() << '\n';
        return 1;
    }

    atursaja::scheduling::Scheduler scheduler;
    auto schedule_result = scheduler.build_plan(workflow_result.value());
    if (!schedule_result.ok()) {
        std::cerr << schedule_result.error() << '\n';
        return 1;
    }

    atursaja::simulation::Simulator simulator(77);
    const auto simulation_result = simulator.run(workflow_result.value(), schedule_result.value());

    atursaja::analytics::ReportBuilder report_builder;
    std::cout << report_builder.render(workflow_result.value(), schedule_result.value(), simulation_result);
    return 0;
}
