#include <iostream>
#include <climits>
#include <new>
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

// Allocate the bigger array first, copy, and only then free the old one, so a
// failed allocation leaves the original array untouched.
bool LoadBalancer::growArray()
{
    if (serverCapacity > INT_MAX / 2)
        return false;
    int newCap = serverCapacity * 2;
    EdgeServer** bigger = new (nothrow) EdgeServer*[newCap];
    if (bigger == nullptr)
        return false;
    for (int i = 0; i < serverCount; i++)
    {
        bigger[i] = servers[i];
    }
    delete[] servers;
    servers = bigger;
    serverCapacity = newCap;
    return true;
}

bool LoadBalancer::hasServer(int serverId) const

{
    for (int i = 0; i < serverCount; i++)
    {
        if (servers[i]->getserver_ID() == serverId)
            return true;
    }
    return false;
}

bool LoadBalancer::addServer(int serverId, int capacity)
{
    if (capacity <= 0 || hasServer(serverId))
        return false;
    if (serverCount == serverCapacity && !growArray())
        return false;
    EdgeServer* s = new (nothrow) EdgeServer(serverId, capacity);
    if (s == nullptr)
        return false;
    servers[serverCount++] = s;
    return true;
}

bool LoadBalancer::assignRequest(const Request& req, int* assignedServerId)
{
    // Not a valid request: refuse it, but do not count it as accepted or rejected.
    if (!req.isValid())
    {
        return false;
    }

    // Valid request but nobody to serve it: one global rejection.
    if (serverCount == 0)
    {
        rejectedCount++;
        return false;
    }

    // Round Robin method used: first server with enough remaining capacity,
    // searching cyclically from the cursor.
    for (int tried = 0; tried < serverCount; tried++)
    {
        int idx = (nextIndex + tried) % serverCount;
        if (servers[idx]->accept_req(req.getWorkload())) {
            nextIndex = (idx + 1) % serverCount;
            acceptedCount++;
            if (assignedServerId) *assignedServerId = servers[idx]->getserver_ID();
            return true;
        }
    }

    // No server could fit it: exactly one global rejection, cursor unchanged.
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

const EdgeServer* LoadBalancer::getServer(int index) const

{
    if (index < 0 || index >= serverCount)
    {
        return nullptr;
    }
    return servers[index];
}

void LoadBalancer::displayServers() const

{
    if (serverCount == 0)
    {
        cout << "No servers added." << endl;
        return;
    }
    for (int i = 0; i < serverCount; i++)
    {
        servers[i]->display_status();
        cout << endl;
    }
}
