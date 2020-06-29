#include <zmq.hpp>
#include "wib.pb.h"

int main(int argc, char **argv) {
    printf("Starting!\n");
    
    zmq::context_t context;
    zmq::socket_t socket(context, ZMQ_PAIR);

    socket.bind("tcp://*:8888");
    printf("Bound!\n");

    while (1) {
    
        zmq::message_t cmd;
        socket.recv(cmd);
        printf("New Message!\n");
        
        wib::Command command;
        
        std::string cmd_str(static_cast<char*>(cmd.data()), cmd.size());
        command.ParseFromString(cmd_str);
        
        std::string reply_str;        
                
        if (command.cmd().Is<wib::ReadReg>()) {
            wib::ReadReg read;
            command.cmd().UnpackTo(&read);
            printf("read %i bytes at %x + %x\n",read.size(),read.base_addr(),read.offset());
            wib::RegValue value;   
            value.set_size(read.size());
            value.set_offset(read.offset());
            value.set_base_addr(read.base_addr());       
            value.set_value(0x0);
            value.SerializeToString(&reply_str);
        } else  if (command.cmd().Is<wib::WriteReg>()) {
            wib::WriteReg write;
            command.cmd().UnpackTo(&write);
            printf("write %i bytes at %x + %x = %x\n",write.size(),write.base_addr(),write.offset(),write.value());
            wib::RegValue value;   
            value.set_size(write.size());
            value.set_offset(write.offset());
            value.set_base_addr(write.base_addr());       
            value.set_value(write.value());
            value.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::GetStatus>()) {
            wib::Status status;            
            /** fill status **/
            status.SerializeToString(&reply_str);
        } /* else if (command.cmd().Is<wib::...>()) {
        
        } */
        
        zmq::message_t reply(reply_str.size());
        memcpy((void*)reply.data(), reply_str.c_str(), reply_str.size());
        socket.send(reply);
        
        printf("Handled!\n");
        
    }

    return 0;
}
