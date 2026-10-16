#include<iostream>
#include"load_metrics.h"
#include"EdgeServer.h"

metrics::~metrics(){};

bool metrics::operator==(const metrics& other) const{
    return(this->overallUtilization==other.overallUtilization);
}

bool metrics::operator>(const metrics& other) const{
    return(this->overallUtilization>other.overallUtilization);

}

metrics metrics::operator+(const metrics& other) const{
    return metrics(this->totalAcceptedRequests+other.totalAcceptedRequests,this->totalRejectedRequests+other.totalRejectedRequests);
}

void metrics::calculateMetrics(EdgeServer servers[], int serverCount){
    double capacity;
    double load;
    double tot_capacity=0.0;
    double tot_load=0.0;
    for(int i=0;i<serverCount;i++){
        capacity=servers[i].getCapacity();
        load=servers[i].getCurrentLoad();
        tot_capacity+=capacity;
        tot_load+=load;
       

    }
    if(tot_capacity>0){
        this->overallUtilization=(tot_load/tot_capacity)*1.0;
    }
}

EdgeServer metrics::findMostLoadedServer(EdgeServer servers[], int serverCount) const{
    EdgeServer most_loaded=servers[0];
    for(int i=1;i<serverCount;i++){
        if(most_loaded.getCurrentLoad()<servers[i].getCurrentLoad()){
            most_loaded=servers[i];
        }
    }

    return most_loaded;
}
EdgeServer metrics::findLeastLoadedServer(EdgeServer servers[], int serverCount) const{
    EdgeServer least_loaded=servers[0];
    for(int i=1;i<serverCount;i++){
        if(least_loaded.getCurrentLoad()>servers[i].getCurrentLoad()){
            least_loaded=servers[i];
        }
    }

    return least_loaded;
}

void printServerMetricsReport(const metrics& metrics, const EdgeServer& server){
    std::cout<<"PRINTING SERVER METRICS:";
    std::cout<<"OVERALL UTILIZATION PERCENTAGE(%):"<<metrics.overallUtilization<<std::endl;
    std::cout<<"CURRENT LOAD OF SERVER:"<<server.getCurrentLoad()<<std::endl;
    std::cout<<"CAPACITY OF SERVER:"<<server.getCapacity();

}






