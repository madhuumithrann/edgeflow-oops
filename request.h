#ifndef REQUEST_H
#define REQUEST_H

// Request ID policy: every Request built by a constructor (not a copy) takes the
// next ID from a static counter, so IDs are unique for the whole program run.
// A copy keeps the ID of the original (it is the same logical request) and is
// not counted in totalCreated.
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
    bool isValid() const;       // true only when workload > 0
    void display() const;       // prints "Request #id (workload: w)" with no newline
    static int getTotalCreated();
};

#endif
