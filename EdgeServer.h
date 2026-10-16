#ifndef EDGESERVER_H
#define EDGESERVER_H

class EdgeServer {
private:
    double capacity;
    double currentLoad;

public:
    // Simple constructor for testing
    EdgeServer(double cap = 100.0, double load = 50.0) {
        capacity = cap;
        currentLoad = load;
    }

    // Dummy getters that match what your metrics functions call
    double getCapacity() const {
        return capacity;
    }

    double getCurrentLoad() const {
        return currentLoad;
    }
};

#endif