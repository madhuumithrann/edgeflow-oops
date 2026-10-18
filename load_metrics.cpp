#include<iostream>
#include<iomanip>
#include<cmath>
#include"load_metrics.h"
#include"edgeserver.h"

metrics::metrics(int accept,int reject)
    : totalAcceptedRequests(accept),totalRejectedRequests(reject),
      totalLoad(0),totalCapacity(0),overallUtilization(0.0){}

metrics::~metrics(){}

void metrics::updateUtilization(){
    if(this->totalCapacity>0){
        this->overallUtilization=(static_cast<double>(this->totalLoad)/static_cast<double>(this->totalCapacity))*100.0;
    }
    else{
        this->overallUtilization=0.0;
    }
}

int metrics::getTotalAccepted() const{
    return this->totalAcceptedRequests;
}

int metrics::getTotalRejected() const{
    return this->totalRejectedRequests;
}

double metrics::getOverallUtilization() const{
    return this->overallUtilization;
}

bool metrics::operator==(const metrics& other) const{
    return std::fabs(this->overallUtilization-other.overallUtilization)<1e-9;
}

bool metrics::operator>(const metrics& other) const{
    return (this->overallUtilization>other.overallUtilization) && !(*this==other);
}

metrics metrics::operator+(const metrics& other) const{
    metrics sum(this->totalAcceptedRequests+other.totalAcceptedRequests,this->totalRejectedRequests+other.totalRejectedRequests);
    sum.totalLoad=this->totalLoad+other.totalLoad;
    sum.totalCapacity=this->totalCapacity+other.totalCapacity;
    sum.updateUtilization();
    return sum;
}

void metrics::calculateMetrics(const LoadBalancer& lb){
    this->totalAcceptedRequests=lb.getAcceptedCount();
    this->totalRejectedRequests=lb.getRejectedCount();
    this->totalLoad=0;
    this->totalCapacity=0;
    for(int i=0;i<lb.getServerCount();i++){
        const EdgeServer* s=lb.getServer(i);
        if(s!=nullptr){
            this->totalLoad+=s->getcurr_load();
            this->totalCapacity+=s->getcapacity();
        }
    }
    this->updateUtilization();
}

// Exact utilization comparison with integers: a.load/a.cap < b.load/b.cap
// is the same as a.load*b.cap < b.load*a.cap (capacities are positive).
// long long keeps the products from overflowing int.
static bool lessUtilized(const EdgeServer* a,const EdgeServer* b){
    long long left=static_cast<long long>(a->getcurr_load())*b->getcapacity();
    long long right=static_cast<long long>(b->getcurr_load())*a->getcapacity();
    return left<right;
}

const EdgeServer* metrics::findMostLoadedServer(const LoadBalancer& lb) const{
    const EdgeServer* most_loaded=lb.getServer(0);   // nullptr when there are no servers
    for(int i=1;i<lb.getServerCount();i++){
        const EdgeServer* s=lb.getServer(i);
        if(lessUtilized(most_loaded,s)){             // strictly greater only: ties keep the earlier server
            most_loaded=s;
        }
    }
    return most_loaded;
}

const EdgeServer* metrics::findLeastLoadedServer(const LoadBalancer& lb) const{
    const EdgeServer* least_loaded=lb.getServer(0);
    for(int i=1;i<lb.getServerCount();i++){
        const EdgeServer* s=lb.getServer(i);
        if(lessUtilized(s,least_loaded)){            // strictly smaller only: ties keep the earlier server
            least_loaded=s;
        }
    }
    return least_loaded;
}

void printServerMetricsReport(const metrics& m, const EdgeServer& server){
    std::ios::fmtflags oldFlags=std::cout.flags();
    std::streamsize oldPrecision=std::cout.precision();
    std::cout<<std::fixed<<std::setprecision(2);
    std::cout<<"PRINTING SERVER METRICS:"<<std::endl;
    std::cout<<"OVERALL UTILIZATION PERCENTAGE(%):"<<m.overallUtilization<<std::endl;
    std::cout<<"SERVER ID:"<<server.getserver_ID()<<std::endl;
    std::cout<<"CURRENT LOAD OF SERVER:"<<server.getcurr_load()<<std::endl;
    std::cout<<"CAPACITY OF SERVER:"<<server.getcapacity()<<std::endl;
    std::cout<<"SERVER UTILIZATION PERCENTAGE(%):"<<server.getUtilization()<<std::endl;
    std::cout.flags(oldFlags);
    std::cout.precision(oldPrecision);
}
