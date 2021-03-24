#include <zmq.hpp>
#include <sstream>
#include "wib.pb.h"

#include "unpack.h"
#include "sensors.h"
#include "wib.h"
#include "wib_3asic.h"
#include "wib_cryo.h"
#include "log.h"
#include "version.h"

int main(int argc, char **argv) {
    //set output to line buffering
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IOLBF, 0);
    
    WIB *w;
    if (argc <= 1) {
        glog.log("wib_server using 3ASIC WIB interface\n");
        w = new WIB_3ASIC();
    } else {
        std::string wib_type(argv[1]);
        if (wib_type == "3ASIC") {
            glog.log("wib_server using 3ASIC WIB interface\n");
            w = new WIB_3ASIC();
        } else if (wib_type == "CRYO") {
            glog.log("wib_server using CRYO WIB interface\n");
            w = new WIB_CRYO();
        } else {
            glog.log("Invalid WIB interface type: %s\n",wib_type.c_str());
            return 1;
        }
    }
    
    glog.log("wib_server initializing hardware interface\n");
    w->initialize();
    
    glog.log("wib_server will listen on port 1234\n");
    
    zmq::context_t context;
    zmq::socket_t socket(context, ZMQ_REP);

    socket.bind("tcp://*:1234");
    
    glog.log("wib_server ready to serve\n");

    for (int i = 0; ; i++) {
    
        zmq::message_t cmd;
        socket.recv(cmd,zmq::recv_flags::none);
        
        wib::Command command;
        
        std::string reply_str; 
        
        std::string cmd_str((char*)cmd.data(), cmd.size());
        if (!command.ParseFromString(cmd_str)) {
            glog.log("Could not parse message %i size %li\n",i,cmd.size());
        } else if (command.cmd().Is<wib::StartFakeTime>()) {
            w->start_fake_time(); //do this first to minimize possible latency
            glog.log("start_fake_time\n");
            wib::Empty rep; 
            rep.SerializeToString(&reply_str);  
        } else if (command.cmd().Is<wib::SetFakeTime>()) {
            glog.log("set_fake_time\n");
            wib::SetFakeTime req;    
            command.cmd().UnpackTo(&req);
            w->set_fake_time(req.time());
            wib::Empty rep; 
            rep.SerializeToString(&reply_str);  
        } else if (command.cmd().Is<wib::Peek>()) {
            wib::Peek read;
            command.cmd().UnpackTo(&read);
            glog.log("peek 0x%lx\n",read.addr());
            uint32_t rval = w->peek(read.addr());
            wib::RegValue value;
            value.set_addr(read.addr());    
            value.set_value(rval);
            value.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::Poke>()) {
            wib::Poke write;
            command.cmd().UnpackTo(&write);
            glog.log("poke 0x%lx = 0x%x\n",write.addr(),write.value());
            w->poke(write.addr(),write.value());
            wib::RegValue value;   
            value.set_addr(write.addr());        
            value.set_value(write.value());
            value.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::CDPeek>()) {
            WIB_3ASIC *w3asic = dynamic_cast<WIB_3ASIC*>(w);
            if (!w3asic) {
                glog.log("cdpeek is only valid for 3ASIC WIBs\n");
            } else {
                wib::CDPeek read;
                command.cmd().UnpackTo(&read);
                glog.log("cdpeek femb:%u cd:%u chip 0x%x page 0x%x addr 0x%x\n",read.femb_idx(),read.coldata_idx(),read.chip_addr(),read.reg_page(),read.reg_addr());
                uint8_t rval = w3asic->cdpeek((uint8_t)read.femb_idx(),(uint8_t)read.coldata_idx(),(uint8_t)read.chip_addr(),(uint8_t)read.reg_page(),(uint8_t)read.reg_addr());
                wib::CDRegValue value;   
                value.set_femb_idx(read.femb_idx());        
                value.set_coldata_idx(read.coldata_idx());      
                value.set_chip_addr(read.chip_addr());      
                value.set_reg_page(read.reg_page());      
                value.set_reg_addr(read.reg_addr());      
                value.set_data(rval);
                value.SerializeToString(&reply_str);
            }
        } else if (command.cmd().Is<wib::CDPoke>()) {
            WIB_3ASIC *w3asic = dynamic_cast<WIB_3ASIC*>(w);
            if (!w3asic) {
                glog.log("cdpoke is only valid for 3ASIC WIBs\n");
            } else {
                wib::CDPoke write;
                command.cmd().UnpackTo(&write);
                glog.log("cdpoke femb:%u cd:%u chip 0x%x page 0x%x addr 0x%x = 0x%x\n",write.femb_idx(),write.coldata_idx(),write.chip_addr(),write.reg_page(),write.reg_addr(),write.data());
                w3asic->cdpoke((uint8_t)write.femb_idx(),(uint8_t)write.coldata_idx(),(uint8_t)write.chip_addr(),(uint8_t)write.reg_page(),(uint8_t)write.reg_addr(),(uint8_t)write.data());
                wib::CDRegValue value;   
                value.set_femb_idx(write.femb_idx());        
                value.set_coldata_idx(write.coldata_idx());      
                value.set_chip_addr(write.chip_addr());      
                value.set_reg_page(write.reg_page());      
                value.set_reg_addr(write.reg_addr());      
                value.set_data(write.data());
                value.SerializeToString(&reply_str);
            }
        } else if (command.cmd().Is<wib::CDFastCmd>()) {
            WIB_3ASIC *w3asic = dynamic_cast<WIB_3ASIC*>(w);
            if (!w3asic) {
                glog.log("cdfastcmd is only valid for 3ASIC WIBs\n");
            } else {
                wib::CDFastCmd fc;
                command.cmd().UnpackTo(&fc);
                glog.log("cdfastcmd 0x%x\n",fc.cmd());
                FEMB_3ASIC::fast_cmd((uint8_t)fc.cmd());
                wib::Empty empty;   
                empty.SerializeToString(&reply_str);
            }
        } else if (command.cmd().Is<wib::Script>()) {
            glog.log("script\n");
            wib::Script script;    
            command.cmd().UnpackTo(&script);
            bool res = w->script(script.script(),script.file());
            wib::Status status;
            status.set_success(res);
            status.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::ReadDaqSpy>()) {
            glog.log("read_daq_spy\n");
            wib::ReadDaqSpy req;    
            command.cmd().UnpackTo(&req);
            char *buf0 = req.buf0() ? new char[DAQ_SPY_SIZE] : NULL;
            char *buf1 = req.buf1() ? new char[DAQ_SPY_SIZE] : NULL;
            int nframes0,nframes1;
            bool success = w->read_daq_spy(buf0,&nframes0,buf1,&nframes1,req.trigger_command(),req.trigger_rec_ticks(),req.trigger_timeout_ms());
            if (!req.deframe()) {
                wib::ReadDaqSpy::DaqSpy rep;
                rep.set_success(success);
                if (buf0) rep.set_buf0(buf0,nframes0*sizeof(frame14)); else rep.set_buf0("");
                if (buf1) rep.set_buf1(buf1,nframes1*sizeof(frame14)); else rep.set_buf1("");
                rep.SerializeToString(&reply_str);
            } else {
                int nframes;
                if (!buf0) {
                    nframes = nframes1;
                } else if (!buf1) {
                    nframes = nframes0;
                } else {
                    nframes = nframes0 < nframes1 ? nframes0 : nframes1;
                }
                wib::ReadDaqSpy::DeframedDaqSpy rep;
                rep.set_success(success);
                rep.set_num_samples(nframes);
                
                const size_t ch_len = nframes*sizeof(uint16_t);
                std::string *sample_buffer = rep.mutable_deframed_samples();
                sample_buffer->resize(4*128*ch_len);
                char *sample_ptr = (char*)sample_buffer->data();
                
                const size_t ts_len = nframes*sizeof(uint64_t);
                std::string *timestamp_buffer = rep.mutable_deframed_timestamps();
                timestamp_buffer->resize(2*ts_len);
                char *timestamp_ptr = (char*)timestamp_buffer->data();
                
                if (req.channels()) {
                    channel_data dch;
                    if (buf0) {
                        deframe_data((frame14*)buf0,nframes,dch);
                        for (size_t i = 0; i < 2; i++) {
                            for (size_t j = 0; j < 128; j++) {
                                memcpy(sample_ptr+(i*128*ch_len)+j*ch_len,dch.channels[i][j].data(),ch_len);
                            }
                        }
                        memcpy(timestamp_ptr+(0*ts_len),dch.timestamp.data(),ts_len);
                    }
                    if (buf1) {
                        deframe_data((frame14*)buf1,nframes,dch);
                        for (size_t i = 0; i < 2; i++) {
                            for (size_t j = 0; j < 128; j++) {
                                memcpy(sample_ptr+((i+2)*128*ch_len)+j*ch_len,dch.channels[i][j].data(),ch_len);
                            }
                        }
                        memcpy(timestamp_ptr+(1*ts_len),dch.timestamp.data(),ts_len);
                    }
                    rep.set_crate_num(dch.crate_num);
                    rep.set_wib_num(dch.wib_num);
                } else {
                    uvx_data duvx;
                    const size_t ch_len = nframes*sizeof(uint16_t);
                    if (buf0) {
                        deframe_data((frame14*)buf0,nframes,duvx);
                        for (size_t i = 0; i < 2; i++) {
                            for (size_t j = 0; j < 48; j++) {
                                if (j < 40) {
                                    memcpy(sample_ptr+(i*128*ch_len)+j*ch_len,duvx.u[i][j].data(),ch_len);
                                    memcpy(sample_ptr+(i*128*ch_len)+(j+40)*ch_len,duvx.v[i][j].data(),ch_len);
                                }
                                memcpy(sample_ptr+(i*128*ch_len)+(j+80)*ch_len,duvx.x[i][j].data(),ch_len);
                            }
                        }
                        memcpy(timestamp_ptr+(0*ts_len),duvx.timestamp.data(),ts_len);
                        
                    }
                    if (buf1) {
                        deframe_data((frame14*)buf1,nframes,duvx);
                        for (size_t i = 0; i < 2; i++) {
                            for (size_t j = 0; j < 128; j++) {
                                if (j < 40) {
                                    memcpy(sample_ptr+((i+2)*128*ch_len)+j*ch_len,duvx.u[i][j].data(),ch_len);
                                    memcpy(sample_ptr+((i+2)*128*ch_len)+(j+40)*ch_len,duvx.v[i][j].data(),ch_len);
                                }
                                memcpy(sample_ptr+((i+2)*128*ch_len)+(j+80)*ch_len,duvx.x[i][j].data(),ch_len);
                            }
                        }
                        memcpy(timestamp_ptr+(1*ts_len),duvx.timestamp.data(),ts_len);
                    }
                    rep.set_crate_num(duvx.crate_num);
                    rep.set_wib_num(duvx.wib_num);
                }
                rep.SerializeToString(&reply_str);
            }
            if (buf0) delete [] buf0;
            if (buf1) delete [] buf1;
        }  else if (command.cmd().Is<wib::GetSensors>()) {
            glog.log("get_sensors\n");
            wib::GetSensors::Sensors sensors;    
            w->read_sensors(sensors);
            sensors.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::ResetTiming>()) {
            glog.log("reset_timing\n");
            wib::GetTimingStatus::TimingStatus rep;    
            w->reset_timing_endpoint(); 
            w->read_timing_status(rep);
            rep.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::GetTimingStatus>()) {
            glog.log("get_timing_status\n");
            wib::GetTimingStatus::TimingStatus rep;    
            w->read_timing_status(rep);
            rep.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::GetTimestamp>()) {
            glog.log("get_timestamp\n");
            wib::GetTimestamp req;
            command.cmd().UnpackTo(&req);
            wib::GetTimestamp::Timestamp rep;    
            uint32_t ts = w->read_fw_timestamp();
            rep.set_timestamp(ts);
            rep.set_day((ts>>27)&0x1f);
            rep.set_month((ts>>23)&0x0f);
            rep.set_year((ts>>17)&0x3f);
            rep.set_hour((ts>>12)&0x1f);
            rep.set_min((ts>>6)&0x3f);
            rep.set_sec((ts>>0)&0x3f);
            rep.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::GetSWVersion>()) {
            glog.log("get_sw_version\n");
            wib::GetSWVersion req;
            command.cmd().UnpackTo(&req);
            wib::GetSWVersion::Version rep;    
            rep.set_version(GIT_VERSION); //set by build system
            rep.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::ConfigurePower>()) {
            glog.log("configure_power\n");
            wib::ConfigurePower req;
            command.cmd().UnpackTo(&req);
            wib::Status rep;    
            glog.mark();
            bool success = w->configure_power(req.dc2dc_o1(),
                                              req.dc2dc_o2(),
                                              req.dc2dc_o3(),
                                              req.dc2dc_o4(),
                                              req.ldo_a0(),
                                              req.ldo_a1());
            glog.store_mark(rep.mutable_extra());
            rep.set_success(success);
            rep.SerializeToString(&reply_str);
        }else if (command.cmd().Is<wib::PowerWIB>()) {
            glog.log("power_wib\n");
            wib::PowerWIB req;
            command.cmd().UnpackTo(&req);
            wib::Status rep;    
            glog.mark();
            bool success = w->power_wib(req);
            glog.store_mark(rep.mutable_extra());
            rep.set_success(success);
            rep.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::ConfigureWIB>()) {
            glog.log("configure_wib\n");
            wib::ConfigureWIB req;
            command.cmd().UnpackTo(&req);
            wib::Status rep;    
            glog.mark();
            bool success = w->configure_wib(req);
            glog.store_mark(rep.mutable_extra());
            rep.set_success(success);
            rep.SerializeToString(&reply_str);
        } else if (command.cmd().Is<wib::Calibrate>()) {
            glog.log("calibrate\n"); 
            wib::Status rep;    
            glog.mark();
            bool success = w->calibrate();
            glog.store_mark(rep.mutable_extra());
            rep.set_success(success);
            rep.SerializeToString(&reply_str);
        }else if (command.cmd().Is<wib::LogControl>()) {
            glog.log("log_control\n");
            wib::LogControl req;    
            command.cmd().UnpackTo(&req);
            wib::LogControl::Log rep;    
            if (req.boot_log()) {
                char buffer[1024];
                std::stringstream buf;
                FILE* pipe = popen("dmesg", "r");
                if (pipe) {
                    while (fgets(buffer, sizeof buffer, pipe) != NULL) {
                        buf << buffer;
                    }
                    pclose(pipe);
                }
                *rep.mutable_contents() = buf.str();
            } else if (req.return_log()) {
                glog.log("log stored\n");
                glog.store(rep.mutable_contents());
            }
            if (req.clear_log()) {
                glog.clear();
                glog.log("log cleared\n");
            }
            rep.SerializeToString(&reply_str);
        }  else if (command.cmd().Is<wib::Reboot>()) {
            glog.log("reboot\n");
            wib::Empty empty; 
            empty.SerializeToString(&reply_str);   
            w->reboot();
        } else if (command.cmd().Is<wib::Update>()) {
            glog.log("update\n");
            wib::Update update;
            command.cmd().UnpackTo(&update);
            wib::Empty empty;
            empty.SerializeToString(&reply_str);
            w->update(update.root_archive().c_str(),update.boot_archive().c_str());
        } else {
	        glog.log("Received an unknown message!\n");
	    }
        
        glog.log("sending message %i size %lu bytes\n",i+1,reply_str.size());
        zmq::message_t reply(reply_str.size());
        memcpy((void*)reply.data(), reply_str.c_str(), reply_str.size());
        socket.send(reply,zmq::send_flags::none);
        
    }
    
    delete w;

    return 0;
}
