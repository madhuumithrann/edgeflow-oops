#include <iostream>
#include "loadbalancer.h"

using namespace std;

LoadBalancer::LoadBalancer() : servers(new EdgeServer*[2]), serverCount(0), serverCapacity(2), nextIndex(0), acceptedCount(0), rejectedCount(0)
{

}

LoadBalancer::~LoadBalancer()
{
    for (int i = 0; i < serverCount; i++)
    {   
        delete servers[i];
    }
    delete[] servers;
}

void LoadBalancer::growArray()
{
    int newCap = serverCapacity * 2;
    EdgeServer** bigger = new EdgeServer*[newCap];
    for (int i = 0; i < serverCount; i++)
    {
        bigger[i] = servers[i];
    }
    delete[] servers;
    servers = bigger;
    serverCapacity = newCap;
}

void LoadBalancer::addServer(int serverId, int capacity)
{
    if (serverCount == serverCapacity)
        growArray();
    servers[serverCount++] = new EdgeServer(serverId, capacity);
}

bool LoadBalancer::assignRequest(const Request& req, int* assignedServerId)
{
    if (!req.isValid() || serverCount == 0)
    {
        rejectedCount++;
        return false;
    }

    // Round Robin method used
    for (int tried = 0; tried < serverCount; tried++)
    {
        int idx = (nextIndex + tried) % serverCount;
        if (servers[idx]->acceptRequest(req)) {
            nextIndex = (idx + 1) % serverCount;
            acceptedCount++;
            if (assignedServerId) *assignedServerId = servers[idx]->getId();
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
    {
        return 0;
    }
    return servers[index];
}

void LoadBalancer::displayServers() const
{
    for (int i = 0; i < serverCount; i++)
        servers[i]->display();
}