#include "load_metrics.h"

metrics::metrics(int accept, int reject, double utilization)
    : totalAcceptedRequests(accept),
      totalRejectedRequests(reject),
      overallUtilization(utilization) {
}

void metrics::calculateMetrics(const LoadBalancer& lb) {
    double totalCapacity = 0;
    double totalLoad = 0;

    for (int i = 0; i < lb.getServerCount(); i++) {
        EdgeServer* server = lb.getServer(i);
        totalCapacity += server->getcapacity();
        totalLoad += server->getcurr_load();
    }

    if (totalCapacity > 0)
        overallUtilization = totalLoad / totalCapacity * 100.0;
    else
        overallUtilization = 0;
}

EdgeServer* metrics::findMostLoadedServer(const LoadBalancer& lb) const {
    if (lb.getServerCount() == 0)
        return 0;

    EdgeServer* best = lb.getServer(0);

    for (int i = 1; i < lb.getServerCount(); i++) {
        EdgeServer* server = lb.getServer(i);

        if (server->getUtilization() > best->getUtilization())
            best = server;
    }

    return best;
}

EdgeServer* metrics::findLeastLoadedServer(const LoadBalancer& lb) const {
    if (lb.getServerCount() == 0)
        return 0;

    EdgeServer* best = lb.getServer(0);

    for (int i = 1; i < lb.getServerCount(); i++) {
        EdgeServer* server = lb.getServer(i);

        if (server->getUtilization() < best->getUtilization())
            best = server;
    }

    return best;
}

double metrics::getOverallUtilization() const {
    return overallUtilization;
}

bool metrics::operator==(const metrics& other) const {
    return overallUtilization == other.overallUtilization;
}

bool metrics::operator>(const metrics& other) const {
    return overallUtilization > other.overallUtilization;
}

metrics metrics::operator+(const metrics& other) const {
    return metrics(
        totalAcceptedRequests + other.totalAcceptedRequests,
        totalRejectedRequests + other.totalRejectedRequests,
        (overallUtilization + other.overallUtilization) / 2.0
    );
}
