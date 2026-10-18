// EdgeFlow - Edge Server Load Balancing Simulator (menu-driven console front end)
//
// main.cpp only does input/output and menu handling. All real work is delegated:
//   EdgeServer   : capacity / load accounting for one server
//   Request      : request id and workload validation
//   LoadBalancer : owns the servers, Round Robin routing, global counters
//   metrics      : utilization statistics and the overloaded comparison operators

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include "loadbalancer.h"
#include "request.h"
#include "load_metrics.h"

using namespace std;

enum ReadStatus { READ_OK, READ_INVALID, READ_EOF };

// Reads one whole line and converts it to an int. The entire (trimmed) line must be
// a decimal integer in int range, otherwise READ_INVALID. READ_EOF means stdin ended.
static ReadStatus readInt(const string& prompt, int& value)
{
    cout << prompt;
    string line;
    if (!getline(cin, line))
    {
        return READ_EOF;
    }
    size_t first = line.find_first_not_of(" \t\r\n");
    if (first == string::npos)
    {
        return READ_INVALID;
    }
    size_t last = line.find_last_not_of(" \t\r\n");
    string token = line.substr(first, last - first + 1);

    errno = 0;
    char* end = nullptr;
    long long parsed = strtoll(token.c_str(), &end, 10);
    if (errno == ERANGE || *end != '\0' || parsed < INT_MIN || parsed > INT_MAX)
    {
        return READ_INVALID;
    }
    value = static_cast<int>(parsed);
    return READ_OK;
}

static void printMenu()
{
    cout << "========================================" << endl;
    cout << "             EDGEFLOW" << endl;
    cout << "      Edge Server Load Balancer" << endl;
    cout << "========================================" << endl;
    cout << "1. Add Edge Server" << endl;
    cout << "2. Submit Request" << endl;
    cout << "3. View Server Status" << endl;
    cout << "4. View Load Metrics" << endl;
    cout << "5. Compare Metrics" << endl;
    cout << "6. Exit" << endl;
    cout << "========================================" << endl;
}

// Each handler returns false only when input ended (EOF) so main can stop cleanly.

static bool handleAddServer(LoadBalancer& lb)
{
    int id = 0;
    int cap = 0;

    ReadStatus st = readInt("Enter server ID: ", id);
    if (st == READ_EOF) return false;
    if (st == READ_INVALID)
    {
        cout << "Invalid input: server ID must be an integer. No server added." << endl;
        return true;
    }

    st = readInt("Enter server capacity (positive integer): ", cap);
    if (st == READ_EOF) return false;
    if (st == READ_INVALID)
    {
        cout << "Invalid input: capacity must be an integer. No server added." << endl;
        return true;
    }

    if (cap <= 0)
    {
        cout << "Capacity must be positive. No server added." << endl;
    }
    else if (lb.hasServer(id))
    {
        cout << "Server ID " << id << " already exists. No server added." << endl;
    }
    else if (lb.addServer(id, cap))
    {
        cout << "Server " << id << " added with capacity " << cap << "." << endl;
    }
    else
    {
        cout << "Could not add server (out of memory). No server added." << endl;
    }
    return true;
}

static bool handleSubmitRequest(LoadBalancer& lb)
{
    int workload = 0;
    ReadStatus st = readInt("Enter request workload (positive integer): ", workload);
    if (st == READ_EOF) return false;
    if (st == READ_INVALID)
    {
        cout << "Invalid input: workload must be an integer. Request not created." << endl;
        return true;
    }
    if (workload <= 0)
    {
        cout << "Workload must be positive. Request not created." << endl;
        return true;
    }

    // Only valid workloads become Request objects, so every request id belongs to a
    // real submitted request.
    Request req(workload);
    int assigned = -1;
    bool accepted = lb.assignRequest(req, &assigned);

    req.display();
    if (accepted)
    {
        cout << " -> ASSIGNED to server " << assigned << endl;
    }
    else if (lb.getServerCount() == 0)
    {
        cout << " -> REJECTED (no servers available)" << endl;
    }
    else
    {
        cout << " -> REJECTED (no server has enough remaining capacity)" << endl;
    }
    return true;
}

static void handleViewMetrics(const LoadBalancer& lb)
{
    metrics m;
    m.calculateMetrics(lb);

    cout << "=== Load Metrics ===" << endl;
    cout << "Total valid requests submitted: " << m.getTotalAccepted() + m.getTotalRejected() << endl;
    cout << "Total accepted requests: " << m.getTotalAccepted() << endl;
    cout << "Total rejected requests: " << m.getTotalRejected() << endl;

    if (lb.getServerCount() == 0)
    {
        cout << "No servers added: utilization and most/least loaded servers are not available." << endl;
        return;
    }

    cout << "Per-server (rejections are counted globally, not per server):" << endl;
    for (int i = 0; i < lb.getServerCount(); i++)
    {
        const EdgeServer* s = lb.getServer(i);
        cout << "  Server " << s->getserver_ID()
             << ": accepted " << s->getacc_req()
             << ", load " << s->getcurr_load() << "/" << s->getcapacity()
             << ", utilization " << s->getUtilization() << "%" << endl;
    }
    cout << "Overall utilization: " << m.getOverallUtilization() << "%" << endl;

    const EdgeServer* most = m.findMostLoadedServer(lb);
    const EdgeServer* least = m.findLeastLoadedServer(lb);
    cout << "Most-loaded server (by utilization %): " << most->getserver_ID()
         << " (" << most->getUtilization() << "%)" << endl;
    cout << "Least-loaded server (by utilization %): " << least->getserver_ID()
         << " (" << least->getUtilization() << "%)" << endl;
    cout << endl;
    printServerMetricsReport(m, *most);
}

static void printSnapshot(const string& label, const metrics& m)
{
    cout << "  " << label << ": accepted " << m.getTotalAccepted()
         << ", rejected " << m.getTotalRejected()
         << ", overall utilization " << m.getOverallUtilization() << "%" << endl;
}

// Compares the current cluster state with the snapshot saved by the previous use
// of this option, so both metrics objects come from real program state.
static void handleCompareMetrics(const LoadBalancer& lb, metrics& baseline, bool& haveBaseline)
{
    metrics current;
    current.calculateMetrics(lb);

    if (!haveBaseline)
    {
        baseline = current;
        haveBaseline = true;
        cout << "Baseline snapshot saved." << endl;
        printSnapshot("baseline", baseline);
        cout << "Add servers / submit requests, then choose this option again to compare." << endl;
        return;
    }

    cout << "=== Compare Metrics (previous snapshot vs current) ===" << endl;
    printSnapshot("baseline", baseline);
    printSnapshot("current ", current);
    cout << boolalpha;
    cout << "baseline == current (equal utilization): " << (baseline == current) << endl;
    cout << "current  >  baseline (higher utilization): " << (current > baseline) << endl;
    cout << "baseline >  current  (higher utilization): " << (baseline > current) << endl;

    metrics pooled = baseline + current;
    cout << "baseline + current (pools the two snapshots: counters, load and capacity are added): " << endl;
    printSnapshot("pooled  ", pooled);
    cout << noboolalpha;

    baseline = current;
    cout << "Baseline updated to the current state." << endl;
}

int main()
{
    cout << fixed << setprecision(2);

    LoadBalancer lb;
    metrics baseline;
    bool haveBaseline = false;
    bool running = true;

    while (running)
    {
        printMenu();
        int choice = 0;
        ReadStatus st = readInt("Enter your choice: ", choice);
        if (st == READ_EOF)
        {
            cout << endl << "End of input reached. Exiting." << endl;
            break;
        }
        if (st == READ_INVALID)
        {
            cout << "Invalid choice. Please enter a number from 1 to 6." << endl;
            continue;
        }

        bool inputOk = true;
        switch (choice)
        {
            case 1: inputOk = handleAddServer(lb); break;
            case 2: inputOk = handleSubmitRequest(lb); break;
            case 3: lb.displayServers(); break;
            case 4: handleViewMetrics(lb); break;
            case 5: handleCompareMetrics(lb, baseline, haveBaseline); break;
            case 6:
                cout << "Exiting EdgeFlow. Goodbye." << endl;
                running = false;
                break;
            default:
                cout << "Invalid choice. Please enter a number from 1 to 6." << endl;
                break;
        }
        if (!inputOk)
        {
            cout << endl << "End of input reached. Exiting." << endl;
            break;
        }
        cout << endl;
    }

    return 0;   // lb's destructor frees every server and the array
}
