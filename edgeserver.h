#ifndef EDGESERVER_H
#define EDGESERVER_H

class EdgeServer{
private:
    int server_ID;
    int capacity;
    int curr_load;
    int acc_req;
    int rej_req;

public:
    EdgeServer();
    EdgeServer(int ID, int cap);
    ~EdgeServer();

    bool accept_req();
    void release_req();

    int getserver_ID() const;
    int getcapacity() const;
    int getcurr_load() const;
    int getacc_req() const;
    int getrej_req() const;
    double getUtilization() const;

    void display_status() const;
};
#endif
