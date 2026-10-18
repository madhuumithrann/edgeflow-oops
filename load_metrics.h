#ifndef LOAD_METRICS_H
#define LOAD_METRICS_H

#include "loadbalancer.h"

class metrics {
private:
    int totalAcceptedRequests;
    int totalRejectedRequests;
    double overallUtilization;

public:
    metrics(int accept = 0, int reject = 0, double utilization = 0.0);

    void calculateMetrics(const LoadBalancer& lb);
    EdgeServer* findMostLoadedServer(const LoadBalancer& lb) const;
    EdgeServer* findLeastLoadedServer(const LoadBalancer& lb) const;
    double getOverallUtilization() const;
    int getAcceptedRequests() const;
    int getRejectedRequests() const;

    bool operator==(const metrics& other) const;
    bool operator>(const metrics& other) const;
    metrics operator+(const metrics& other) const;
};

#endif
