#include <algorithm>
#include <cassert>
#include <string>
#include <unordered_map>

#include "atursaja/graph/dependency_graph.hpp"
#include "atursaja/parser/config_parser.hpp"
#include "atursaja/scheduling/scheduler.hpp"
#include "atursaja/simulation/simulator.hpp"

namespace {

const char* kScenario = R"(
PROJECT TestFlow
RESOURCE cpu 8
RESOURCE gpu 2
RESOURCE memory 16
TASK ingest duration=30 priority=3 uses=cpu:2,memory:2
TASK preprocess duration=20 priority=4 uses=cpu:3,memory:3 depends=ingest risk=medium
TASK train duration=60 priority=10 retries=1 fail=0.0 uses=cpu:4,gpu:1,memory:8 depends=preprocess risk=high
TASK evaluate duration=15 priority=5 uses=cpu:2,memory:2 depends=train
)";

}  // namespace

int main() {
    atursaja::parser::ConfigParser parser;
    auto workflow_result = parser.parse_text(kScenario);
    assert(workflow_result.ok());

    const auto& workflow = workflow_result.value();
    assert(workflow.tasks().size() == 4);
    assert(workflow.resource_capacities().at("cpu") == 8);

    atursaja::graph::DependencyGraph graph(workflow);
    auto topo_result = graph.topological_order();
    assert(topo_result.ok());
    const auto topo = topo_result.value();
    assert(topo.front() == "ingest");
    assert(topo.back() == "evaluate");

    auto critical_result = graph.critical_path_lengths();
    assert(critical_result.ok());
    assert(critical_result.value().at("ingest") == 125);

    atursaja::scheduling::Scheduler scheduler;
    auto schedule_result = scheduler.build_plan(workflow);
    assert(schedule_result.ok());

    std::unordered_map<std::string, atursaja::core::Minutes> starts;
    std::unordered_map<std::string, atursaja::core::Minutes> ends;
    for (const auto& task : schedule_result.value().tasks) {
        starts[task.id] = task.start_minutes;
        ends[task.id] = task.end_minutes;
    }

    assert(starts["preprocess"] >= ends["ingest"]);
    assert(starts["train"] >= ends["preprocess"]);
    assert(starts["evaluate"] >= ends["train"]);

    atursaja::simulation::Simulator simulator(11);
    const auto simulation = simulator.run(workflow, schedule_result.value());
    assert(simulation.records.size() == workflow.tasks().size());
    assert(simulation.failed_tasks == 0);
    assert(simulation.actual_makespan >= schedule_result.value().makespan);

    return 0;
}
