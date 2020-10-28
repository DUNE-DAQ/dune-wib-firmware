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
    w.initialize();
    
    printf("wib_server will listen on port 1234\n");
    
    zmq::context_t context;
    zmq::socket_t socket(context, ZMQ_REP);

    socket.bind("tcp://*:1234");
    
    printf("wib_server ready to serve\n");

    for (int i = 0; ; i++) {
    
        zmq::message_t cmd;
        socket.recv(cmd,zmq::recv_flags::none);
        
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
        } else if (command.cmd().Is<wib::Poke>()) {
            wib::Poke write;
            command.cmd().UnpackTo(&write);
            printf("poke 0x%lx = 0x%x\n",write.addr(),write.value());
            w.poke(write.addr(),write.value());
            wib::RegValue value;   
            value.set_addr(write.addr());        
            value.set_value(write.value());
            value.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::CDPeek>()) {
            wib::CDPeek read;
            command.cmd().UnpackTo(&read);
            printf("cdpeek femb:%u cd:%u chip 0x%x page 0x%x addr 0x%x\n",read.femb_idx(),read.coldata_idx(),read.chip_addr(),read.reg_page(),read.reg_addr());
            uint8_t rval = w.cdpeek((uint8_t)read.femb_idx(),(uint8_t)read.coldata_idx(),(uint8_t)read.chip_addr(),(uint8_t)read.reg_page(),(uint8_t)read.reg_addr());
            wib::CDRegValue value;   
            value.set_femb_idx(read.femb_idx());        
            value.set_coldata_idx(read.coldata_idx());      
            value.set_chip_addr(read.chip_addr());      
            value.set_reg_page(read.reg_page());      
            value.set_reg_addr(read.reg_addr());      
            value.set_data(rval);
            value.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::CDPoke>()) {
            wib::CDPoke write;
            command.cmd().UnpackTo(&write);
            printf("cdpoke femb:%u cd:%u chip 0x%x page 0x%x addr 0x%x = 0x%x\n",write.femb_idx(),write.coldata_idx(),write.chip_addr(),write.reg_page(),write.reg_addr(),write.data());
            w.cdpoke((uint8_t)write.femb_idx(),(uint8_t)write.coldata_idx(),(uint8_t)write.chip_addr(),(uint8_t)write.reg_page(),(uint8_t)write.reg_addr(),(uint8_t)write.data());
            wib::CDRegValue value;   
            value.set_femb_idx(write.femb_idx());        
            value.set_coldata_idx(write.coldata_idx());      
            value.set_chip_addr(write.chip_addr());      
            value.set_reg_page(write.reg_page());      
            value.set_reg_addr(write.reg_addr());      
            value.set_data(write.data());
            value.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::CDFastCmd>()) {
            wib::CDFastCmd fc;
            command.cmd().UnpackTo(&fc);
            printf("cdfastcmd 0x%x\n",fc.cmd());
            FEMB::fast_cmd((uint8_t)fc.cmd());
            wib::Empty empty;   
            empty.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::Script>()) {
            printf("script\n");
            wib::Script script;    
            command.cmd().UnpackTo(&script);
            bool res = w.script(script.script(),script.file());
            wib::Status status;
            status.set_success(res);
            status.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::ReadDaqSpy>()) {
            printf("read_daq_spy\n");
            wib::ReadDaqSpy req;    
            command.cmd().UnpackTo(&req);
            char *buf0 = req.buf0() ? new char[DAQ_SPY_SIZE] : NULL;
            char *buf1 = req.buf1() ? new char[DAQ_SPY_SIZE] : NULL;
            bool success = w.read_daq_spy(buf0,buf1);
            wib::DaqSpy rep;
            rep.set_success(success);
            if (buf0) rep.set_buf0(buf0,DAQ_SPY_SIZE); else rep.set_buf0("");
            if (buf1) rep.set_buf1(buf1,DAQ_SPY_SIZE); else rep.set_buf1("");
            rep.SerializeToString(&reply_str);
            if (buf0) delete [] buf0;
            if (buf1) delete [] buf1;
        } else if (command.cmd().Is<wib::GetSensors>()) {
            printf("get_sensors\n");
            wib::Sensors sensors;    
            w.read_sensors(sensors);
            sensors.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::ConfigureWIB>()) {
            printf("configure_wib\n");
            wib::ConfigureWIB req;
            command.cmd().UnpackTo(&req);
            wib::Status rep;    
            bool success = w.configure_wib(req);
            rep.set_success(success);
            rep.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::ConfigureFEMB>()) {
            printf("configure_femb\n");
            wib::ConfigureFEMB req;
            command.cmd().UnpackTo(&req);
            wib::Status rep;    
            bool success = w.configure_femb(req);
            rep.set_success(success);
            rep.SerializeToString(&reply_str);
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
        } else {
	        fprintf(stderr,"Received an unknown message!\n");
	    }
        
        printf("sending message %i size %lu bytes\n",i+1,reply_str.size());
        zmq::message_t reply(reply_str.size());
        memcpy((void*)reply.data(), reply_str.c_str(), reply_str.size());
        socket.send(reply,zmq::send_flags::none);
        
    }

    return 0;
}
