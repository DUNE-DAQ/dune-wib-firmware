#include <unistd.h>
#include <fstream>

#include <zmq.hpp>
#include "wib.pb.h"

using namespace std;

void print_usage(const char *prog) {
    printf( "Usage: %s hextime [wib_ip] [wib_ip] ... \n", prog);
}

template <class C>
void send_command_multi(vector<zmq::socket_t*> &sockets, const C &msg) {

    wib::Command command;
    command.mutable_cmd()->PackFrom(msg);
    
    string cmd_str;
    command.SerializeToString(&cmd_str);
    
    const size_t n = sockets.size();
    
    vector<zmq::message_t*> reqs(n);
    for (size_t i = 0; i < n; i++) {
        reqs[i] = new zmq::message_t(cmd_str.size());
        memcpy((void*)reqs[i]->data(), cmd_str.c_str(), cmd_str.size());
    }
    
    //separated this way to send with lowest possible latency
    for (size_t i = 0; i < n; i++) {
        sockets[i]->send(*reqs[i],zmq::send_flags::none);
    }
    
    for (size_t i = 0; i < n; i++) {
        delete reqs[i];
        zmq::message_t reply;
        sockets[i]->recv(reply,zmq::recv_flags::none); //response ignored
    }
    
}

int main(int argc, char **argv) {
    
    if (argc <= 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    zmq::context_t context(1);
    
    int nwibs = argc - 2;
    vector<zmq::socket_t*> wibs(nwibs);
    
    printf("Connecting to %i WIB%s:",nwibs,nwibs == 1 ? "" : "s");
    for (int i = 0; i < nwibs; i++) {
        wibs[i] = new zmq::socket_t(context, ZMQ_REQ);
        printf("%s%s",i == 0 ? " " : ", ",argv[i+2]);
        char *addr;
        if (asprintf(&addr,"tcp://%s:1234",argv[i+2]) < 0) return 1;
        wibs[i]->connect(addr);
        free(addr);
    }
    printf("\n");
    
    uint64_t time = (uint64_t)strtoull(argv[1],NULL,16);
    printf("Setting start time to 0x%016lX\n",time); 
    
    wib::SetFakeTime set;
    set.set_time(time);
    send_command_multi(wibs, set);
    
    printf("Starting fake timestamp generator\n",time); 
    wib::StartFakeTime start;
    send_command_multi(wibs, start);
    
    for (int i = 0; i < nwibs; i++) {
        delete wibs[i];
    }
    
    return 0;
}
