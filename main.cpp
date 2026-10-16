#include <iostream>
#include "Load_Metrics.h" // Or whatever your header file is named
#include "EdgeServer.h"

// (Optional: If EdgeServer isn't fully defined yet by Member 1, 
// make sure it has at least a constructor like EdgeServer(double cap, double load) 
// and the getters getCapacity() and getCurrentLoad()).

int main() {
    std::cout << "=== TESTING METRICS & EDGE SERVER SYSTEM ===\n\n-";

    // 1. Create dummy EdgeServers
    // Assuming constructor takes capacity and current load
    int serverCount = 3;
    EdgeServer servers[3] = {
        EdgeServer(100.0, 45.0), // Server 0: Capacity 100, Load 45
        EdgeServer(200.0, 180.0),// Server 1: Capacity 200, Load 180 (Most loaded)
        EdgeServer(150.0, 30.0)  // Server 2: Capacity 150, Load 30  (Least loaded)
    };

    // 2. Test metrics calculation and constructor
    metrics systemMetrics(150, 12); // 150 accepted, 12 rejected requests
    systemMetrics.calculateMetrics(servers, serverCount);

    // 3. Test printServerMetricsReport (Friend function)
    std::cout << "\n--- Testing Report Function ---\n";
    printServerMetricsReport(systemMetrics, servers[0]);

    // 4. Test findMostLoadedServer and findLeastLoadedServer
    std::cout << "\n--- Testing Server Finders ---\n";
    EdgeServer mostLoaded = systemMetrics.findMostLoadedServer(servers, serverCount);
    EdgeServer leastLoaded = systemMetrics.findLeastLoadedServer(servers, serverCount);
    
    std::cout << "Most Loaded Server Current Load: " << mostLoaded.getCurrentLoad() << std::endl;
    std::cout << "Least Loaded Server Current Load: " << leastLoaded.getCurrentLoad() << std::endl;

    // 5. Test Operator Overloading (+, ==, >)
    std::cout << "\n--- Testing Operators ---\n";
    metrics metrics2(50, 5); // Another metrics object for comparison
    
    metrics combinedMetrics = systemMetrics + metrics2;
    std::cout << "Combined metrics created successfully via operator+\n";

    if (systemMetrics > metrics2) {
        std::cout << "systemMetrics utilization is greater than metrics2 utilization.\n";
    } else {
        std::cout << "systemMetrics utilization is NOT greater than metrics2 utilization.\n";
    }

    if (systemMetrics == metrics2) {
        std::cout << "Both metrics have equal utilization.\n";
    } else {
        std::cout << "Both metrics have different utilization.\n";
    }

    std::cout << "\n=== ALL TESTS COMPLETED SUCCESSFULLY ===" << std::endl;
    return 0;
}