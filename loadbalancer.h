#ifndef LOADBALANCER_H
#define LOADBALANCER_H

#include "edgeServer.h" // From Member 1
#include "request.h"

class LoadBalancer {
private:
    EdgeServer** servers;
    int serverCount;
    int serverCapacity;
    int nextIndex;
    int acceptedCount;
    int rejectedCount;

    void growArray();

public:
    LoadBalancer();
    ~LoadBalancer();

    LoadBalancer(const LoadBalancer&) = delete;
    LoadBalancer& operator=(const LoadBalancer&) = delete;

    void addServer(int serverId, int capacity);
    bool assignRequest(const Request& req, int* assignedServerId = 0);

    int  getServerCount() const;
    int  getAcceptedCount() const;
    int  getRejectedCount() const;
    EdgeServer* getServer(int index) const;
    void displayServers() const;
};

#endif