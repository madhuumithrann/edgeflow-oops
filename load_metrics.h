#ifndef LOAD_METRICS_H
#define LOAD_METRICS_H
#include"EdgeServer.h"
class metrics{
    private:
        int totalAcceptedRequests;
        int totalRejectedRequests;
        double overallUtilization;
    public:
        metrics(int accept,int reject): totalAcceptedRequests(accept),totalRejectedRequests(reject){};
        ~metrics();
        void calculateMetrics(EdgeServer servers[], int serverCount);
        EdgeServer findMostLoadedServer(EdgeServer servers[], int serverCount) const;
        EdgeServer findLeastLoadedServer(EdgeServer servers[], int serverCount) const;
        bool operator==(const metrics& other) const;
        bool operator>(const metrics& other) const;
        metrics operator+(const metrics& other) const;
        friend void printServerMetricsReport(const metrics& metrics, const EdgeServer& server);

};

#endif 