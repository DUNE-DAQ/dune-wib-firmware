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
    zmq::socket_t socket(context, ZMQ_PAIR);

    socket.bind("tcp://*:1234");
    
    printf("wib_server ready to serve\n");

    for (int i = 0; ; i++) {
    
        zmq::message_t cmd;
        socket.recv(&cmd,0);
        
        wib::Command command;
        
        std::string cmd_str(static_cast<char*>(cmd.data()), cmd.size());
        command.ParseFromString(cmd_str);
        
        std::string reply_str;        
                
        if (command.cmd().Is<wib::ReadReg>()) {
            wib::ReadReg read;
            command.cmd().UnpackTo(&read);
            printf("read %i bytes at %lx + %lx\n",read.size(),read.base_addr(),read.offset());
            wib::RegValue value;   
            value.set_size(read.size());
            value.set_offset(read.offset());
            value.set_base_addr(read.base_addr());       
            value.set_value(0x0);
            value.SerializeToString(&reply_str);
        } else  if (command.cmd().Is<wib::WriteReg>()) {
            wib::WriteReg write;
            command.cmd().UnpackTo(&write);
            printf("write %i bytes at %lx + %lx = %lx\n",write.size(),write.base_addr(),write.offset(),write.value());
            wib::RegValue value;   
            value.set_size(write.size());
            value.set_offset(write.offset());
            value.set_base_addr(write.base_addr());       
            value.set_value(write.value());
            value.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::GetSensors>()) {
            wib::Sensors sensors;    
            w.read_sensors(sensors);
            sensors.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::Initialize>()) {
            wib::Empty empty;    
            w.initialize();
            empty.SerializeToString(&reply_str);
        } /* else if (command.cmd().Is<wib::...>()) {
        
        } */
        
        zmq::message_t reply(reply_str.size());
        memcpy((void*)reply.data(), reply_str.c_str(), reply_str.size());
        socket.send(reply);
        
        printf("handled message %i\n",i+1);
        
    }

    return 0;
}
