#ifndef LOAD_METRICS_H
#define LOAD_METRICS_H
#include"edgeserver.h"
#include"loadbalancer.h"

// metrics is a snapshot of the cluster's statistics.
//   totalAcceptedRequests / totalRejectedRequests : global counters
//   totalLoad / totalCapacity                    : sums over all servers
//   overallUtilization (percent) = totalLoad / totalCapacity * 100
//                                  (0.0 when totalCapacity == 0, i.e. no servers)
// overallUtilization is always recomputed from the two totals, so it can never be
// uninitialized and never disagrees with them.
//
// Operators:
//   a == b : the overall utilizations are equal (within 1e-9)
//   a >  b : a's overall utilization is greater than b's (and not equal)
//   a +  b : pools two snapshots: counters, loads and capacities are added and the
//            utilization is recomputed from the pooled totals
class metrics{
    private:
        int totalAcceptedRequests;
        int totalRejectedRequests;
        long long totalLoad;
        long long totalCapacity;
        double overallUtilization;
        void updateUtilization();
    public:
        explicit metrics(int accept=0,int reject=0);
        ~metrics();

        // Reads counters and all server loads/capacities from the load balancer (O(S)).
        void calculateMetrics(const LoadBalancer& lb);

        // Compared by utilization percentage (not raw load). Ties go to the server
        // added first. Both return nullptr when there are no servers.
        const EdgeServer* findMostLoadedServer(const LoadBalancer& lb) const;
        const EdgeServer* findLeastLoadedServer(const LoadBalancer& lb) const;

        int getTotalAccepted() const;
        int getTotalRejected() const;
        double getOverallUtilization() const;

        bool operator==(const metrics& other) const;
        bool operator>(const metrics& other) const;
        metrics operator+(const metrics& other) const;
        friend void printServerMetricsReport(const metrics& m, const EdgeServer& server);

};

#endif
