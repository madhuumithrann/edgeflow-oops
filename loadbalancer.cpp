
#include "loadbalancer.h"

LoadBalancer::LoadBalancer()
    : serverCount(0), serverCapacity(2), nextIndex(0),
      acceptedCount(0), rejectedCount(0)
{
    servers = new EdgeServer*[serverCapacity];
}

LoadBalancer::~LoadBalancer()
{
    for (int i = 0; i < serverCount; i++)
        delete servers[i];

    delete[] servers;
}

void LoadBalancer::growArray()
{
    int newCapacity = serverCapacity * 2;
    EdgeServer** newServers = new EdgeServer*[newCapacity];

    for (int i = 0; i < serverCount; i++)
        newServers[i] = servers[i];

    delete[] servers;
    servers = newServers;
    serverCapacity = newCapacity;
}

bool LoadBalancer::addServer(int serverId, int capacity)
{
    if (capacity <= 0)
        return false;

    for (int i = 0; i < serverCount; i++) {
        if (servers[i]->getserver_ID() == serverId)
            return false;
    }

    if (serverCount == serverCapacity)
        growArray();

    servers[serverCount] = new EdgeServer(serverId, capacity);
    serverCount++;

    return true;
}

bool LoadBalancer::assignRequest(const Request& req, int* assignedServerId)
{
    if (!req.isValid() || serverCount == 0) {
        rejectedCount++;
        return false;
    }

    for (int tried = 0; tried < serverCount; tried++) {
        int idx = (nextIndex + tried) % serverCount;

        if (servers[idx]->accept_req(req.getWorkload())) {
            nextIndex = (idx + 1) % serverCount;
            acceptedCount++;

            if (assignedServerId != 0)
                *assignedServerId = servers[idx]->getserver_ID();

            return true;
        }
    }

    rejectedCount++;
    return false;
}

int LoadBalancer::getServerCount() const
{
    return serverCount;
}

int LoadBalancer::getAcceptedCount() const
{
    return acceptedCount;
}

int LoadBalancer::getRejectedCount() const
{
    return rejectedCount;
}

EdgeServer* LoadBalancer::getServer(int index) const
{
    if (index < 0 || index >= serverCount)
        return 0;

    return servers[index];
}
