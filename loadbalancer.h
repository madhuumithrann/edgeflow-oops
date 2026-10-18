#ifndef LOADBALANCER_H
#define LOADBALANCER_H

#include "edgeserver.h" // From Member 1
#include "request.h"

// LoadBalancer owns every EdgeServer it creates (composition) and routes each
// valid request with workload-based Round Robin.
//
// Round Robin policy:
//   * the cursor nextIndex is where the next search starts
//   * servers are examined in cyclic order starting at the cursor
//   * the FIRST server that has enough remaining capacity gets the request
//   * after a successful assignment the cursor moves to the server AFTER the one chosen
//   * a rejected request leaves the cursor unchanged
//
// Counters (each valid request is counted exactly once):
//   acceptedCount + rejectedCount == number of valid requests submitted.
//   A request with workload <= 0 is not a valid request: it is refused and not counted.
class LoadBalancer {
private:
    EdgeServer** servers;     // dynamic array of pointers to dynamically created servers
    int serverCount;
    int serverCapacity;       // allocated size of the array
    int nextIndex;            // Round Robin cursor
    int acceptedCount;
    int rejectedCount;

    bool growArray();         // doubles the array; false (and no change) if it cannot

public:
    LoadBalancer();
    ~LoadBalancer();

    LoadBalancer(const LoadBalancer&) = delete;
    LoadBalancer& operator=(const LoadBalancer&) = delete;

    // false (nothing changed) if capacity <= 0, the id already exists, or memory runs out
    bool addServer(int serverId, int capacity);
    bool hasServer(int serverId) const;

    // true  -> accepted; *assignedServerId (if given) receives the server's ID
    // false -> rejected (or invalid); *assignedServerId is left untouched
    bool assignRequest(const Request& req, int* assignedServerId = nullptr);

    int  getServerCount() const;
    int  getAcceptedCount() const;
    int  getRejectedCount() const;
    const EdgeServer* getServer(int index) const;   // read-only view; nullptr if out of range
    void displayServers() const;
};

#endif
