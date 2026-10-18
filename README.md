# EdgeFlow - Edge Server Load Balancing Simulator

## About the Project

EdgeFlow is a C++ project that simulates how requests are distributed among multiple edge servers. Each server has a fixed capacity, and incoming requests are assigned based on the available capacity of the servers.

The project demonstrates basic load balancing concepts along with object-oriented programming concepts in C++.

## Features

- Add edge servers with unique IDs and capacities.
- Submit requests with different workloads.
- Distribute requests using a Round Robin approach.
- Check whether a server has enough capacity to handle a request.
- Reject requests when no server can accommodate the workload.
- View server status and utilization.
- Calculate overall load utilization.
- Find the most loaded and least loaded servers.
- Compare server metrics using operator overloading.

## Technologies Used

- C++

## Project Structure

```text
edgeflow-oops/
├── main.cpp
├── edgeserver.h
├── edgeserver.cpp
├── request.h
├── request.cpp
├── loadbalancer.h
├── loadbalancer.cpp
├── load_metrics.h
├── load_metrics.cpp
└── README.md
```

## How to Run

Make sure a C++ compiler such as GCC is installed.

Compile the project using:

```bash
g++ -std=c++17 main.cpp edgeserver.cpp request.cpp loadbalancer.cpp load_metrics.cpp -o edgeflow
```

Run on Windows:

```powershell
.\edgeflow.exe
```

On Linux:

```bash
./edgeflow
```

## Menu Options

1. Add Edge Server
2. Submit Request
3. View Server Status
4. View Load Metrics
5. Compare Metrics
6. Exit

## OOP Concepts Used

- **Classes and Objects:** `EdgeServer`, `Request`, `LoadBalancer`, and `metrics` represent the main parts of the simulator.
- **Encapsulation:** Server and request data are kept private and accessed through member functions.
- **Constructors and Destructors:** Used to initialize objects and manage their lifetime.
- **Static Members:** `Request` uses static variables to generate request IDs and count created requests.
- **Operator Overloading:** The `metrics` class overloads `==`, `>`, and `+` to compare and combine metric values.
- **Dynamic Memory Allocation:** `LoadBalancer` stores servers using a dynamically allocated array of pointers.

## Current Scope

This is the midsem version of EdgeFlow, developed as an OOP course project. It focuses on request allocation, server capacity, load tracking, and basic metrics. It is a console-based simulation and does not connect to real edge servers or deploy workloads on an actual network.

## Future Improvements

- Add request completion and workload release.
- Improve load balancing strategies.
- Add request history and performance reports.
- Develop a graphical interface for monitoring servers.



Developed as part of the Object-Oriented Programming course project.
