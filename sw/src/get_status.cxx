#include <zmq.hpp>
#include "wib.pb.h"

template <class R, class C>
void send_command(zmq::socket_t &socket, const C &msg, R &repl) {

    wib::Command command;
    command.mutable_cmd()->PackFrom(msg);
    
    std::string cmd_str;
    command.SerializeToString(&cmd_str);
    
    zmq::message_t request(cmd_str.size());
    memcpy((void*)request.data(), cmd_str.c_str(), cmd_str.size());
    socket.send(request);
    
    zmq::message_t reply;
    socket.recv(&reply);
    
    std::string reply_str(static_cast<char*>(reply.data()), reply.size());
    repl.ParseFromString(reply_str);
    
}

int main(int argc, char **argv) {
    
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_PAIR);

    socket.connect("tcp://127.0.0.1:1234");
    
    wib::GetSensors getsensors;
    wib::Sensors sensors;
    send_command(socket,getsensors,sensors);
    
    
    
    return 0;
}
