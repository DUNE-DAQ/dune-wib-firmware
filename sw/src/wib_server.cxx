#include <zmq.hpp>
#include "wib.pb.h"

#include "sensors.h"
#include "wib.h"

int main(int argc, char **argv) {
    //set output to line buffering
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IOLBF, 0);
    
    printf("wib_server preparing hardware interface\n");
    
    WIB w;
    
    printf("wib_server will listen on port 1234\n");
    
    zmq::context_t context;
    zmq::socket_t socket(context, ZMQ_REP);

    socket.bind("tcp://*:1234");
    
    printf("wib_server ready to serve\n");

    for (int i = 0; ; i++) {
    
        zmq::message_t cmd;
        socket.recv(&cmd,0);
        
        wib::Command command;
        
        std::string cmd_str(static_cast<char*>(cmd.data()), cmd.size());
        command.ParseFromString(cmd_str);
        
        std::string reply_str;        
                
        if (command.cmd().Is<wib::Peek>()) {
            wib::Peek read;
            command.cmd().UnpackTo(&read);
            printf("peek 0x%lx\n",read.addr());
            uint32_t rval = w.peek(read.addr());
            wib::RegValue value;
            value.set_addr(read.addr());    
            value.set_value(rval);
            value.SerializeToString(&reply_str);
        } else  if (command.cmd().Is<wib::Poke>()) {
            wib::Poke write;
            command.cmd().UnpackTo(&write);
            printf("poke 0x%lx = 0x%x\n",write.addr(),write.value());
            uint32_t rval = w.poke(write.addr(),write.value());
            wib::RegValue value;   
            value.set_addr(write.addr());        
            value.set_value(write.value());
            value.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::GetSensors>()) {
            printf("get_sensors\n");
            wib::Sensors sensors;    
            w.read_sensors(sensors);
            sensors.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::Initialize>()) {
            printf("initialize\n");
            wib::Empty empty;    
            w.initialize();
            empty.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::Reboot>()) {
            printf("reboot\n");
            wib::Empty empty; 
            empty.SerializeToString(&reply_str);   
            w.reboot();
        } else if (command.cmd().Is<wib::Update>()) {
            printf("update\n");
            wib::Update update;
            command.cmd().UnpackTo(&update);
            wib::Empty empty;
            empty.SerializeToString(&reply_str);
            w.update(update.root_archive().c_str(),update.boot_archive().c_str());
        } /* else if (command.cmd().Is<wib::...>()) {
        
        } */
        
        zmq::message_t reply(reply_str.size());
        memcpy((void*)reply.data(), reply_str.c_str(), reply_str.size());
        socket.send(reply);
        
        printf("msg count: %i\n",i+1);
        
    }

    return 0;
}
