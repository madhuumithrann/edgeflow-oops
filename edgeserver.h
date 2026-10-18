#ifndef EDGESERVER_H
#define EDGESERVER_H

// EdgeServer models one simulated edge server.
//
// Load model (workload-based):
//   capacity  = maximum total workload the server can hold at once
//   curr_load = sum of the workloads of the requests it has accepted
//   Invariant: 0 <= curr_load <= capacity
//
// acc_req counts requests this server has accepted. There is no per-server
// "rejected" counter: a request that no server can take is rejected once, by the
// LoadBalancer, and belongs to no particular server.
class EdgeServer{
private:
    int server_ID;
    int capacity;
    int curr_load;
    int acc_req;

public:
    EdgeServer();
    EdgeServer(int ID, int cap);
    ~EdgeServer();

    // Accepts the request only if workload > 0 and it fits in the remaining
    // capacity. On rejection the server is left completely unchanged.
    bool accept_req(int workload);

    int getserver_ID() const;
    int getcapacity() const;
    int getcurr_load() const;
    int getacc_req() const;
    double getUtilization() const;   // percentage, 0.0 - 100.0

    void display_status() const;
};
#endif
