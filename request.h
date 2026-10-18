#ifndef REQUEST_H
#define REQUEST_H

#include <iostream>

class Request {
private:
    int id;
    int workload;
    static int nextId;
    static int totalCreated;

public:
    Request();
    Request(int workload);
    Request(const Request& other);
    ~Request();

    int  getId() const;
    int  getWorkload() const;
    bool isValid() const;
    void display() const;
    static int getTotalCreated();
};

#endif