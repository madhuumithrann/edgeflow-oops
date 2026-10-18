// Unit, integration and randomized invariant tests for EdgeFlow's classes.
//
// Build and run from the repository root:
//   g++ -std=c++17 -Wall -Wextra -I. tests/test_edgeflow.cpp edgeserver.cpp request.cpp loadbalancer.cpp load_metrics.cpp -o edgeflow_tests
//   ./edgeflow_tests
//
// The tests use their own CHECK macro (not assert) so they still run if NDEBUG is defined.

#include <iostream>
#include <sstream>
#include <vector>
#include <random>
#include <cmath>
#include <climits>
#include <string>
#include "edgeserver.h"
#include "request.h"
#include "loadbalancer.h"
#include "load_metrics.h"

static int checksRun = 0;
static int checksFailed = 0;

#define CHECK(cond) \
    do { \
        checksRun++; \
        if (!(cond)) { \
            checksFailed++; \
            std::cout << "  FAIL line " << __LINE__ << ": " #cond << std::endl; \
        } \
    } while (0)

static bool approx(double a, double b) { return std::fabs(a - b) < 1e-9; }

// Submits a request of the given workload; returns the assigned server id or -1 if rejected.
static int submit(LoadBalancer& lb, int workload)
{
    Request r(workload);
    int id = -1;
    if (!lb.assignRequest(r, &id)) return -1;
    return id;
}

static void testEdgeServer()
{
    std::cout << "[EdgeServer]" << std::endl;

    EdgeServer d;
    CHECK(d.getserver_ID() == 0 && d.getcapacity() == 10 && d.getcurr_load() == 0 && d.getacc_req() == 0);

    EdgeServer s(7, 10);
    CHECK(s.accept_req(10));                       // exact fit
    CHECK(s.getcurr_load() == 10 && s.getacc_req() == 1 && approx(s.getUtilization(), 100.0));
    CHECK(!s.accept_req(1));                       // full
    CHECK(s.getcurr_load() == 10 && s.getacc_req() == 1);   // rejection changes nothing

    EdgeServer t(1, 10);
    CHECK(!t.accept_req(11));                      // larger than capacity
    CHECK(t.getcurr_load() == 0 && t.getacc_req() == 0);
    CHECK(!t.accept_req(0));
    CHECK(!t.accept_req(-5));
    CHECK(!t.accept_req(INT_MIN));
    CHECK(t.getcurr_load() == 0 && t.getacc_req() == 0);
    CHECK(t.accept_req(4));
    CHECK(t.accept_req(6));
    CHECK(t.getcurr_load() == 10 && t.getacc_req() == 2);

    EdgeServer u(2, 8);
    CHECK(u.accept_req(2));
    CHECK(approx(u.getUtilization(), 25.0));
    EdgeServer v(3, 3);
    CHECK(v.accept_req(1));
    CHECK(approx(v.getUtilization(), 100.0 / 3.0)); // floating-point division, not integer division

    // Integer-overflow safety: capacity INT_MAX
    EdgeServer big(4, INT_MAX);
    CHECK(big.accept_req(INT_MAX - 1));
    CHECK(!big.accept_req(2));                     // INT_MAX-1 + 2 would overflow with a naive "L + W <= C"
    CHECK(big.getcurr_load() == INT_MAX - 1);
    CHECK(big.accept_req(1));
    CHECK(big.getcurr_load() == INT_MAX);
    CHECK(!big.accept_req(1));

    // Degenerate capacity (never created by LoadBalancer, but must be safe)
    EdgeServer z(5, 0);
    CHECK(approx(z.getUtilization(), 0.0));
    CHECK(!z.accept_req(1));
    EdgeServer neg(6, -4);
    CHECK(approx(neg.getUtilization(), 0.0));
    CHECK(!neg.accept_req(1));
}

static void testRequest()
{
    std::cout << "[Request]" << std::endl;

    int createdBefore = Request::getTotalCreated();
    Request a(5);
    Request b(9);
    CHECK(a.getId() > 0 && b.getId() == a.getId() + 1);       // unique, increasing ids
    CHECK(Request::getTotalCreated() == createdBefore + 2);
    CHECK(a.getWorkload() == 5 && a.isValid());

    Request c(a);                                              // copy: same id, not a new request
    CHECK(c.getId() == a.getId() && c.getWorkload() == 5);
    CHECK(Request::getTotalCreated() == createdBefore + 2);

    Request zero(0);
    Request neg(-3);
    Request def;
    CHECK(!zero.isValid() && !neg.isValid() && !def.isValid());
    CHECK(zero.getId() != neg.getId() && neg.getId() != def.getId());

    std::ostringstream out;                                    // destructor must stay silent
    std::streambuf* old = std::cout.rdbuf(out.rdbuf());
    { Request temp(1); Request copy(temp); }
    std::cout.rdbuf(old);
    CHECK(out.str().empty());
}

static void testLoadBalancerBasics()
{
    std::cout << "[LoadBalancer basics]" << std::endl;

    LoadBalancer none;                                         // zero servers
    CHECK(none.getServerCount() == 0 && none.getServer(0) == nullptr);
    CHECK(submit(none, 5) == -1);
    CHECK(none.getAcceptedCount() == 0 && none.getRejectedCount() == 1);
    CHECK(submit(none, 0) == -1);                              // invalid: not counted
    CHECK(submit(none, -2) == -1);
    CHECK(none.getAcceptedCount() == 0 && none.getRejectedCount() == 1);

    LoadBalancer lb;
    CHECK(!lb.addServer(1, 0));                                // zero capacity
    CHECK(!lb.addServer(1, -5));                               // negative capacity
    CHECK(lb.getServerCount() == 0);
    CHECK(lb.addServer(1, 10));
    CHECK(!lb.addServer(1, 99));                               // duplicate id
    CHECK(lb.getServerCount() == 1 && lb.getServer(0)->getcapacity() == 10);   // unchanged by failed add
    CHECK(lb.hasServer(1) && !lb.hasServer(2));
    CHECK(lb.getServer(-1) == nullptr && lb.getServer(1) == nullptr);

    CHECK(submit(lb, 10) == 1);                                // exact fit on one server
    CHECK(submit(lb, 1) == -1);                                // full
    CHECK(lb.getServer(0)->getcurr_load() == 10);
    CHECK(lb.getAcceptedCount() == 1 && lb.getRejectedCount() == 1);

    LoadBalancer one;
    CHECK(one.addServer(9, 5));
    CHECK(submit(one, 6) == -1);                               // exceeds capacity
    CHECK(one.getServer(0)->getcurr_load() == 0 && one.getRejectedCount() == 1);
}

static void testRoundRobin()
{
    std::cout << "[Round Robin]" << std::endl;

    // Equal servers: strict rotation 1,2,3,1,2,3
    LoadBalancer lb;
    lb.addServer(1, 10); lb.addServer(2, 10); lb.addServer(3, 10);
    int expected[] = {1, 2, 3, 1, 2, 3};
    for (int i = 0; i < 6; i++)
        CHECK(submit(lb, 1) == expected[i]);
    CHECK(lb.getAcceptedCount() == 6 && lb.getRejectedCount() == 0);

    // Different capacities, a full server is skipped and rotation continues correctly.
    LoadBalancer lb2;
    lb2.addServer(1, 2); lb2.addServer(2, 10); lb2.addServer(3, 10);
    int expected2[] = {1, 2, 3, 2, 3, 2, 3};       // hand-computed; server 1 is full after request 1
    for (int i = 0; i < 7; i++)
        CHECK(submit(lb2, 2) == expected2[i]);

    // A request that fits only the next server
    LoadBalancer lb3;
    lb3.addServer(1, 5); lb3.addServer(2, 10);
    CHECK(submit(lb3, 4) == 1);     // cursor -> server 2
    CHECK(submit(lb3, 6) == 2);     // cursor -> server 1
    CHECK(submit(lb3, 3) == 2);     // server 1 (4/5) cannot fit 3, server 2 (6/10) can
    CHECK(lb3.getServer(0)->getcurr_load() == 4 && lb3.getServer(1)->getcurr_load() == 9);

    // Rejection must not move the cursor
    LoadBalancer lb4;
    lb4.addServer(1, 5); lb4.addServer(2, 5); lb4.addServer(3, 5);
    CHECK(submit(lb4, 5) == 1);     // cursor -> server 2
    CHECK(submit(lb4, 9) == -1);    // bigger than every capacity
    CHECK(submit(lb4, 1) == 2);     // still server 2, not 3

    // All servers full: rejected, loads unchanged
    LoadBalancer lb5;
    lb5.addServer(1, 3); lb5.addServer(2, 3);
    CHECK(submit(lb5, 3) == 1);
    CHECK(submit(lb5, 3) == 2);
    CHECK(submit(lb5, 1) == -1);
    CHECK(submit(lb5, 1) == -1);
    CHECK(lb5.getServer(0)->getcurr_load() == 3 && lb5.getServer(1)->getcurr_load() == 3);
    CHECK(lb5.getAcceptedCount() == 2 && lb5.getRejectedCount() == 2);
    CHECK(lb5.getServer(0)->getacc_req() == 1 && lb5.getServer(1)->getacc_req() == 1);
}

static void testGrowthAndOwnership()
{
    std::cout << "[Growth and ownership]" << std::endl;

    LoadBalancer lb;
    for (int i = 1; i <= 1000; i++)
        CHECK(lb.addServer(i, i));                  // forces many doublings from the initial 2
    CHECK(lb.getServerCount() == 1000);
    bool orderOk = true, capOk = true;
    for (int i = 0; i < 1000; i++)
    {
        if (lb.getServer(i)->getserver_ID() != i + 1) orderOk = false;
        if (lb.getServer(i)->getcapacity() != i + 1) capOk = false;
    }
    CHECK(orderOk && capOk);                        // growth preserved every pointer, in order
    CHECK(!lb.addServer(500, 5));                   // duplicate still detected after growth
    CHECK(lb.getServerCount() == 1000);

    for (int round = 0; round < 2000; round++)      // repeated create / destroy
    {
        LoadBalancer tmp;
        for (int i = 0; i < 7; i++) tmp.addServer(i, 3);
        submit(tmp, 2);
    }
    CHECK(true);   // leaks / double frees are caught by running this under the sanitizers
}

static void testMetrics()
{
    std::cout << "[metrics]" << std::endl;

    metrics fresh;                                  // nothing uninitialized
    CHECK(fresh.getTotalAccepted() == 0 && fresh.getTotalRejected() == 0 && approx(fresh.getOverallUtilization(), 0.0));

    LoadBalancer empty;                             // empty collection
    metrics me;
    me.calculateMetrics(empty);
    CHECK(approx(me.getOverallUtilization(), 0.0));
    CHECK(me.findMostLoadedServer(empty) == nullptr && me.findLeastLoadedServer(empty) == nullptr);

    LoadBalancer idle;                              // servers but zero accepted requests
    idle.addServer(1, 10); idle.addServer(2, 20);
    metrics mi;
    mi.calculateMetrics(idle);
    CHECK(approx(mi.getOverallUtilization(), 0.0) && mi.getTotalAccepted() == 0);
    CHECK(mi.findMostLoadedServer(idle)->getserver_ID() == 1);    // tie -> first added
    CHECK(mi.findLeastLoadedServer(idle)->getserver_ID() == 1);

    // Different capacities: percent decides, not raw load.
    LoadBalancer lb;
    lb.addServer(1, 10); lb.addServer(2, 100);
    CHECK(submit(lb, 5) == 1);                      // 5/10  = 50%
    CHECK(submit(lb, 10) == 2);                     // 10/100 = 10%  (larger raw load!)
    metrics m;
    m.calculateMetrics(lb);
    CHECK(approx(m.getOverallUtilization(), 15.0 / 110.0 * 100.0));
    CHECK(m.findMostLoadedServer(lb)->getserver_ID() == 1);
    CHECK(m.findLeastLoadedServer(lb)->getserver_ID() == 2);
    CHECK(m.getTotalAccepted() == 2 && m.getTotalRejected() == 0);

    // Equal utilization: deterministic tie -> earliest server for both queries.
    LoadBalancer tie;
    tie.addServer(1, 10); tie.addServer(2, 20);
    CHECK(submit(tie, 5) == 1);                     // 50%
    CHECK(submit(tie, 10) == 2);                    // 50%
    metrics mt;
    mt.calculateMetrics(tie);
    CHECK(mt.findMostLoadedServer(tie)->getserver_ID() == 1);
    CHECK(mt.findLeastLoadedServer(tie)->getserver_ID() == 1);

    // Operators on real snapshots.
    LoadBalancer a;  a.addServer(1, 10);  submit(a, 5);  submit(a, 9);     // load 5/10, 1 accepted, 1 rejected
    LoadBalancer b;  b.addServer(1, 30);  submit(b, 10);                   // load 10/30, 1 accepted
    metrics ma, mb;
    ma.calculateMetrics(a);
    mb.calculateMetrics(b);
    CHECK(approx(ma.getOverallUtilization(), 50.0) && approx(mb.getOverallUtilization(), 100.0 / 3.0));
    CHECK(ma > mb);
    CHECK(!(mb > ma));
    CHECK(!(ma == mb));
    metrics ma2;
    ma2.calculateMetrics(a);
    CHECK(ma == ma2);
    CHECK(!(ma > ma2));
    metrics sum = ma + mb;
    CHECK(sum.getTotalAccepted() == 2 && sum.getTotalRejected() == 1);
    CHECK(approx(sum.getOverallUtilization(), 15.0 / 40.0 * 100.0));         // pooled: (5+10)/(10+30)
    metrics sumDefault = fresh + fresh;
    CHECK(approx(sumDefault.getOverallUtilization(), 0.0));                  // nothing garbage in a sum

    // Friend report prints percentages and real numbers.
    std::ostringstream out;
    std::streambuf* old = std::cout.rdbuf(out.rdbuf());
    printServerMetricsReport(ma, *a.getServer(0));
    std::cout.rdbuf(old);
    CHECK(out.str().find("50.00") != std::string::npos);
    CHECK(out.str().find("CAPACITY OF SERVER:10") != std::string::npos);
}

// Randomized test against an independent reference model, checking invariants after every request.
static void runStress(unsigned seed, int servers, int requests, int maxCap, int maxWork)
{
    std::mt19937 rng(seed);
    LoadBalancer lb;
    std::vector<int> cap, load, accepted;
    for (int i = 0; i < servers; i++)
    {
        int c = static_cast<int>(rng() % static_cast<unsigned>(maxCap)) + 1;
        CHECK(lb.addServer(i + 1, c));
        cap.push_back(c); load.push_back(0); accepted.push_back(0);
    }
    int cursor = 0, modelAcc = 0, modelRej = 0;
    bool allOk = true;
    for (int r = 0; r < requests && allOk; r++)
    {
        int w = static_cast<int>(rng() % static_cast<unsigned>(maxWork)) + 1;
        // model
        int expectId = -1;
        for (int t = 0; t < servers; t++)
        {
            int idx = (cursor + t) % servers;
            if (w <= cap[static_cast<size_t>(idx)] - load[static_cast<size_t>(idx)])
            {
                load[static_cast<size_t>(idx)] += w;
                accepted[static_cast<size_t>(idx)]++;
                cursor = (idx + 1) % servers;
                expectId = idx + 1;
                break;
            }
        }
        if (expectId == -1) modelRej++; else modelAcc++;

        std::vector<int> before;
        for (int i = 0; i < servers; i++) before.push_back(lb.getServer(i)->getcurr_load());
        int got = submit(lb, w);
        if (got != expectId) { allOk = false; break; }
        if (lb.getAcceptedCount() != modelAcc || lb.getRejectedCount() != modelRej) { allOk = false; break; }
        if (lb.getAcceptedCount() + lb.getRejectedCount() != r + 1) { allOk = false; break; }   // counted exactly once
        for (int i = 0; i < servers; i++)
        {
            const EdgeServer* s = lb.getServer(i);
            size_t k = static_cast<size_t>(i);
            if (s->getcurr_load() < 0 || s->getcurr_load() > s->getcapacity()) allOk = false;   // 0 <= load <= capacity
            if (s->getcurr_load() != load[k] || s->getacc_req() != accepted[k]) allOk = false;
            if (!approx(s->getUtilization(), static_cast<double>(load[k]) / cap[k] * 100.0)) allOk = false;
            if (got == -1 && s->getcurr_load() != before[k]) allOk = false;  // rejection changes no load
        }
    }
    CHECK(allOk);
    int sumAcc = 0;
    for (int i = 0; i < servers; i++) sumAcc += lb.getServer(i)->getacc_req();
    CHECK(sumAcc == lb.getAcceptedCount());         // every accepted request went to exactly one server
}

static void testStress()
{
    std::cout << "[Randomized stress vs reference model]" << std::endl;
    runStress(1u, 1, 20000, 50, 60);
    runStress(2u, 3, 50000, 40, 45);
    runStress(3u, 50, 200000, 100, 120);
    runStress(4u, 2000, 100000, 30, 35);
    runStress(5u, 7, 200000, 1000, 1000);
}

int main()
{
    testEdgeServer();
    testRequest();
    testLoadBalancerBasics();
    testRoundRobin();
    testGrowthAndOwnership();
    testMetrics();
    testStress();

    std::cout << std::endl << checksRun << " checks run, " << checksFailed << " failed." << std::endl;
    if (checksFailed == 0)
    {
        std::cout << "ALL TESTS PASSED" << std::endl;
        return 0;
    }
    std::cout << "TESTS FAILED" << std::endl;
    return 1;
}
