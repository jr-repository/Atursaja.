# AturSaja Hyper Orchestrator

Proyek C++ modular untuk menjadwalkan workflow kompleks berbasis dependency graph, kapasitas resource, simulasi eksekusi, dan analytics report.

## Fitur

- Parser DSL skenario workflow (`.asj`)
- Validasi resource, dependency, dan task definition
- DAG engine dengan topological ordering dan critical-path analysis
- Heuristic scheduler untuk resource-constrained workflow
- Simulator eksekusi dengan jitter, retry, failure propagation, dan event log
- Analytics report untuk planned vs actual execution
- Test executable berbasis assertion standar

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Jalankan

```bash
./build/atursaja-cli examples/hyper_factory.asj
```

Tanpa argumen, executable akan otomatis memakai skenario contoh `examples/hyper_factory.asj`.
