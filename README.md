# EdgeFlow - Edge Server Load Balancing Simulator

OOPS course project (C++). EdgeFlow is a **menu-driven console simulator**: requests with a workload arrive,
and a load balancer distributes them across several simulated edge servers using **Round Robin** while
respecting each server's capacity. It is a simulator only - there is no real CPU measurement, networking or
distributed execution.

## Problem statement
Spread incoming requests fairly over multiple edge servers without ever overloading one, and report how busy
each server is and how many requests were accepted or rejected.

## Objectives
- Add edge servers with capacities; create requests with IDs and workloads.
- Distribute requests with Round Robin, respecting capacity.
- Track accepted and rejected requests; show load and utilization.
- Identify the most-loaded and least-loaded servers.
- Demonstrate operator overloading on the metrics class.

## Features
1. Add Edge Server (positive capacity, unique ID)
2. Submit Request (positive workload, unique request ID)
3. View Server Status
4. View Load Metrics (counters, per-server utilization, most/least loaded)
5. Compare Metrics (overloaded `==`, `>`, `+`)
6. Exit

Invalid menu choices, malformed numbers, out-of-range numbers and end of input are all handled without crashing.

## File structure
| File | Class | Responsibility |
|---|---|---|
| `edgeserver.h/.cpp` | `EdgeServer` | One server: capacity, current load, accepted count |
| `request.h/.cpp` | `Request` | Request ID (static counter) and workload validation |
| `loadbalancer.h/.cpp` | `LoadBalancer` | Owns the servers, Round Robin routing, global counters |
| `load_metrics.h/.cpp` | `metrics` | Utilization statistics, most/least loaded, overloaded operators |
| `main.cpp` | - | Menu, input parsing and output only |
| `tests/test_edgeflow.cpp` | - | Unit, integration and randomized invariant tests |
| `tests/run_cli_tests.sh` | - | End-to-end tests of the menu program |

## Build and run
```
g++ -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -Wshadow main.cpp edgeserver.cpp request.cpp loadbalancer.cpp load_metrics.cpp -o edgeflow
./edgeflow            (on Windows: .\edgeflow.exe  - add .exe to the -o name)
```

## Tests
```
# unit + integration + randomized tests
g++ -std=c++17 -Wall -Wextra -I. tests/test_edgeflow.cpp edgeserver.cpp request.cpp loadbalancer.cpp load_metrics.cpp -o edgeflow_tests
./edgeflow_tests

# end-to-end menu tests (bash / Git Bash)
bash tests/run_cli_tests.sh ./edgeflow

# memory and undefined-behaviour checking (g++ on Linux/macOS or MSYS2)
g++ -std=c++17 -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer main.cpp edgeserver.cpp request.cpp loadbalancer.cpp load_metrics.cpp -o edgeflow_sanitized
bash tests/run_cli_tests.sh ./edgeflow_sanitized
```
The randomized test compares `LoadBalancer` with an independent reference model after every request and checks:
`0 <= load <= capacity`, `utilization = load / capacity * 100`, `accepted + rejected = valid requests submitted`,
a rejected request changes no server's load, and every accepted request went to exactly one server.

## Algorithm: workload-based Round Robin
A server's **capacity** is the maximum total workload it can hold; its **current load** is the sum of the
workloads it has accepted. For a request of workload `W` on a server with capacity `C` and load `L`:

- accept only if `W > 0` and `W <= C - L` (written this way so `L + W` can never overflow `int`)
- on acceptance `L = L + W` and the server's accepted counter goes up by one
- otherwise nothing changes

`LoadBalancer::assignRequest` keeps a cursor `nextIndex`:
1. examine servers cyclically starting at the cursor;
2. the **first** server with enough remaining capacity gets the request;
3. the cursor moves to the server **after** the one chosen;
4. if no server fits, the request is rejected **once**, globally, and the cursor does not move.

Capacity decides eligibility; Round Robin decides order. Servers are never sorted.

## Counter meanings
- **Accepted / rejected (global):** every *valid* request (workload > 0) is counted exactly once, so
  `accepted + rejected = valid requests submitted`. A request with workload <= 0 is never created by the menu
  and is not counted if one is built directly.
- **Per-server accepted:** requests that server accepted. There is **no per-server rejected counter**: a request
  that fits nowhere belongs to no server, so rejections are reported globally.

## Metrics definitions
- Utilization of a server = `current_load / capacity * 100` (percent, floating point).
- Overall utilization = `total load / total capacity * 100` over all servers; `0` when there are no servers.
- Most/least loaded server: compared by **utilization percentage** (exact integer cross-multiplication), not raw
  load. Ties go to the server added first. With no servers, none exists.
- `a == b`: overall utilizations equal (tolerance 1e-9). `a > b`: `a`'s overall utilization is greater.
  `a + b`: pools two snapshots - counters, loads and capacities are added and utilization recomputed.
- Option 5 compares the **current state with the snapshot saved by its previous use**, so both metric objects
  come from real program state. The first use only saves the baseline.

## Request IDs
Each `Request` built by a constructor takes the next ID from a static counter, so IDs are unique for a program
run. A copy keeps the original's ID and is not counted again. IDs are not reused after a request is rejected.

## OOPS concepts and where they appear
| Concept | Where |
|---|---|
| Classes and objects | `EdgeServer`, `Request`, `LoadBalancer`, `metrics` |
| Encapsulation | private members of every class; state changes only through `accept_req`, `assignRequest`, etc. |
| Constructors / destructors | `EdgeServer` (default + parameterized), `Request` (default, parameterized, copy), `LoadBalancer::~LoadBalancer` frees all servers and the array |
| Static members | `Request::nextId`, `Request::totalCreated`, `Request::getTotalCreated()` |
| `this` pointer | `EdgeServer` member functions |
| Composition | `LoadBalancer` owns its `EdgeServer` objects |
| Dynamic memory | `EdgeServer**` array in `LoadBalancer`, grown by `growArray()` |
| Copy control | `Request` copy constructor; `LoadBalancer` copying deleted (it owns raw pointers) |
| Friend function | `printServerMetricsReport` in `load_metrics.h` |
| Operator overloading | `metrics::operator==`, `operator>`, `operator+` |

No inheritance, virtual functions or templates are used.

## Complexity (S = number of servers)
| Operation | Time | Notes |
|---|---|---|
| `assignRequest` | O(S) | at most S servers examined |
| `addServer` | O(S) | duplicate-ID scan; the array doubles so growth copying is amortized O(1) per add |
| `displayServers` | O(S) | |
| `calculateMetrics` | O(S) | one pass |
| `findMostLoadedServer` / `findLeastLoadedServer` | O(S) | one pass each |

Space is O(S) for the pointer array plus the servers.

## Limitations
- Requests are not completed or released: load only grows, so servers eventually fill. Completion is outside
  the midsem scope; supporting it would require tracking which request is on which server.
- Counters are `int`; extremely long runs (billions of requests) could overflow them.
- Request IDs are `int` and unique per run only; nothing is saved between runs.
- Single-threaded, in-memory simulation only.

## Future enhancements
Request completion/release with per-request tracking, other scheduling policies, saving/loading a scenario,
and richer reports.
