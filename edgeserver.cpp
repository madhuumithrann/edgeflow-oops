#include "edgeserver.h"
#include <iostream>
#include <iomanip>
using namespace std;

EdgeServer::EdgeServer(){
    this->server_ID=0;
    this->capacity=10;
    this->curr_load=0;
    this->acc_req=0;
}

EdgeServer::EdgeServer(int ID, int cap){
    this->server_ID=ID;
    this->capacity=cap;
    this->curr_load=0;
    this->acc_req=0;
}

EdgeServer::~EdgeServer(){

}

bool EdgeServer::accept_req(int workload){
    // Reject non-positive workloads, and refuse to work with a broken state.
    if (workload <= 0 || this->curr_load < 0 || this->curr_load > this->capacity){
        return false;
    }
    // Safe fit test: "workload <= remaining" instead of "curr_load + workload <= capacity",
    // because the addition could overflow int. Here remaining = capacity - curr_load >= 0.
    if (workload > this->capacity - this->curr_load){
        return false;
    }
    this->curr_load += workload;   // cannot exceed capacity, so cannot overflow
    this->acc_req++;
    return true;
}

int EdgeServer::getserver_ID() const{
    return this->server_ID;
}
int EdgeServer::getcapacity() const{
    return this->capacity;
}
int EdgeServer::getcurr_load() const{
    return this->curr_load;
}
int EdgeServer::getacc_req() const{
    return this->acc_req;
}
double EdgeServer::getUtilization() const{
    if (this->capacity <= 0){
        return 0.0;
    }
    return (static_cast<double>(this->curr_load)/this->capacity)*100.0;
}

void EdgeServer::display_status() const{
    ios::fmtflags oldFlags = cout.flags();
    streamsize oldPrecision = cout.precision();
    cout<<"EdgeServer ID: "<<this->server_ID<<endl;
    cout<<"Capacity: "<<this->capacity<<endl;
    cout<<"Current Load: "<<this->curr_load<<endl;
    cout<<"Utilization: "<<fixed<<setprecision(2)<<this->getUtilization()<<"%"<<endl;
    cout<<"Accepted Requests: "<<this->acc_req<<endl;
    cout.flags(oldFlags);
    cout.precision(oldPrecision);
}
