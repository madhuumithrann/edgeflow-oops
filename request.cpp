#include <iostream>
#include "request.h"

using namespace std;

int Request::nextId = 1;
int Request::totalCreated = 0;

Request::Request() : id(nextId++), workload(0)
{
    totalCreated++;
}

Request::Request(int workload) : id(nextId++), workload(workload)
{
    totalCreated++;
}

Request::Request(const Request& other)
    : id(nextId++), workload(other.workload)
{
    totalCreated++;
}

Request::~Request()
{
}

int Request::getId() const
{
    return id;
}

int Request::getWorkload() const
{
    return workload;
}

bool Request::isValid() const
{
    return workload > 0;
}

void Request::display() const
{
    cout << "Request #" << id << " (workload: " << workload << ")";
}

int Request::getTotalCreated()
{
    return totalCreated;
}
