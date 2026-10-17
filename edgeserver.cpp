#include "edgeserver.h"
#include <iostream>
using namespace std;

EdgeServer::EdgeServer(){
    this->server_ID=0;
    this->capacity=10;
    this->curr_load=0;
    this->acc_req=0;
    this->rej_req=0;
}

EdgeServer::EdgeServer(int ID, int cap){
    this->server_ID=ID;
    this->capacity=cap;
    this->curr_load=0;
    this->acc_req=0;
    this->rej_req=0;
}

EdgeServer::~EdgeServer(){

}

bool EdgeServer::accept_req(){
    if (this->curr_load < this->capacity){
        this->curr_load++;
        this->acc_req++;
        return true;
    }
    else{
        this->rej_req++;
        return false;
    }
}

void EdgeServer::release_req(){
    if (this->curr_load > 0){
        this->curr_load--;
    }
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
int EdgeServer::getrej_req() const{
    return this->rej_req;
}
double EdgeServer::getUtilization() const{
    if (this->capacity == 0){
        return 0.0;
    }
    return (static_cast<double>(this->curr_load)/this->capacity)*100.0;
}

void EdgeServer::display_status() const{
    cout<<"EdgeServer ID: "<<this->server_ID<<endl;
    cout<<"Capacity: "<<this->capacity<<endl;
    cout<<"Current Load: "<<this->curr_load<<endl;
    cout<<"Utilization: "<<this->getUtilization()<<endl;
    cout<<"Accepted Requests: "<<this->acc_req<<endl;
    cout<<"Rejected Requests: "<<this->rej_req<<endl;
}
