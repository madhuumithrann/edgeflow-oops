#include <iostream>
#include <iomanip>
#include <limits>
#include "loadbalancer.h"
#include "load_metrics.h"

using namespace std;

bool readInt(const char* message, int& value) {
    cout << message;

    if (cin >> value)
        return true;

    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Invalid input.\n";
    return false;
}

int main() {
    LoadBalancer lb;
    int choice = 0;

    do {
        cout << "\n========== EDGEFLOW ==========\n";
        cout << "1. Add Edge Server\n";
        cout << "2. Submit Request\n";
        cout << "3. View Server Status\n";
        cout << "4. View Load Metrics\n";
        cout << "5. Compare Metrics\n";
        cout << "6. Exit\n";
        cout << "==============================\n";

        if (!readInt("Enter your choice: ", choice))
            continue;

        if (choice == 1) {
            int id, capacity;

            if (!readInt("Enter Server ID: ", id))
                continue;

            if (!readInt("Enter Capacity: ", capacity))
                continue;

            if (lb.addServer(id, capacity))
                cout << "Server added successfully.\n";
            else
                cout << "Invalid capacity or duplicate ID.\n";
        }

        else if (choice == 2) {
            int workload;

            if (!readInt("Enter Request Workload: ", workload))
                continue;

            if (workload <= 0) {
                cout << "Workload must be positive.\n";
                continue;
            }

            Request req(workload);
            int serverId = -1;

            cout << "Request ID: " << req.getId() << '\n';
            cout << "Workload: " << req.getWorkload() << '\n';

            if (lb.assignRequest(req, &serverId)) {
                cout << "Request accepted.\n";
                cout << "Assigned Server: " << serverId << '\n';
            }
            else {
                cout << "Request rejected. No server can accommodate it.\n";
            }
        }

        else if (choice == 3) {
            if (lb.getServerCount() == 0) {
                cout << "No servers available.\n";
                continue;
            }

            for (int i = 0; i < lb.getServerCount(); i++) {
                cout << "\n";
                lb.getServer(i)->display_status();
            }
        }

        else if (choice == 4) {
            metrics m(lb.getAcceptedCount(), lb.getRejectedCount());
            m.calculateMetrics(lb);

            cout << fixed << setprecision(2);
            cout << "Valid Requests Processed: "
                 << lb.getAcceptedCount() + lb.getRejectedCount() << '\n';
            cout << "Accepted Requests: " << lb.getAcceptedCount() << '\n';
            cout << "Rejected Requests: " << lb.getRejectedCount() << '\n';
            cout << "Overall Utilization: "
                 << m.getOverallUtilization() << "%\n";

            if (lb.getServerCount() == 0) {
                cout << "No server metrics available.\n";
                continue;
            }

            for (int i = 0; i < lb.getServerCount(); i++) {
                EdgeServer* s = lb.getServer(i);

                cout << "Server " << s->getserver_ID()
                     << ": " << s->getUtilization() << "%\n";
            }

            EdgeServer* most = m.findMostLoadedServer(lb);
            EdgeServer* least = m.findLeastLoadedServer(lb);

            cout << "Most Loaded Server: "
                 << most->getserver_ID() << '\n';
            cout << "Least Loaded Server: "
                 << least->getserver_ID() << '\n';
        }

        else if (choice == 5) {
            if (lb.getServerCount() < 2) {
                cout << "Add at least two servers to compare metrics.\n";
                continue;
            }

            EdgeServer* s1 = lb.getServer(0);
            EdgeServer* s2 = lb.getServer(1);

            metrics m1(s1->getacc_req(), s1->getrej_req(),
                       s1->getUtilization());
            metrics m2(s2->getacc_req(), s2->getrej_req(),
                       s2->getUtilization());

            cout << "Server " << s1->getserver_ID()
                 << " Utilization: " << s1->getUtilization() << "%\n";
            cout << "Server " << s2->getserver_ID()
                 << " Utilization: " << s2->getUtilization() << "%\n";

            cout << "Equal Utilization: "
                 << (m1 == m2 ? "Yes" : "No") << '\n';

            cout << "First Server Has Higher Utilization: "
                 << (m1 > m2 ? "Yes" : "No") << '\n';

            metrics combined = m1 + m2;

            cout << "Combined Accepted Requests: "
                 << combined.getAcceptedRequests() << '\n';
            cout << "Combined Rejected Attempts: "
                 <<  combined.getRejectedRequests() << '\n';
            cout << "Average Utilization: "
                 <<combined.getOverallUtilization() << "%\n";
        }

        else if (choice == 6) {
            cout << "Exiting EdgeFlow.\n";
        }

        else {
            cout << "Invalid choice.\n";
        }

    } while (choice != 6);

    return 0;
}
