#include <iostream>
#include "request.h"

using namespace std;

int Request::nextId = 1;

int Request::totalCreated = 0;

Request::Request() : id(nextId++), workload(0)
{
    totalCreated++;
}

Request::Request(int work) : id(nextId++), workload(work)
{
    totalCreated++;
}

// A copy is the same logical request: same id, same workload, and it does not
// consume a new id or increase totalCreated.
Request::Request(const Request& other) : id(other.id), workload(other.workload)
{

}

// Intentionally silent: a message here would print for every temporary or copy
// and clutter the program output.
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
