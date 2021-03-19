#include <unistd.h>
#include <fstream>
#include <streambuf>

#include <zmq.hpp>
#include <readline/readline.h>
#include <readline/history.h>

#include "wib.pb.h"
#include "femb_3asic.h" //for fast command defines
#include "wib.h" //for spy buffer defines

using namespace std;

void print_usage(const char *prog) {
    glog.log( "Usage: %s [-w ip] [cmd] \n", prog);
}

void print_help() {
    glog.log("Available commands:\n");
    glog.log("  reboot\n");
    glog.log("    Reboot the WIB\n");
    glog.log("  fw_timestamp\n");
    glog.log("    Return the firmware version timestamp\n");
    glog.log("  sw_version\n");
    glog.log("    Return the software build version\n");
    glog.log("  log [boot|clear]\n");
    glog.log("    Return the wib_server log (or return boot log, or clear the logs)\n");
    glog.log("  script filename\n");
    glog.log("    Run a WIB script (local file will be sent, otherwise filename is remote in /etc/wib/)\n");
    glog.log("  daqspy filename\n");
    glog.log("    Read 1MB from each daq spy buffer and write the 2MB data to filename\n");
    glog.log("  peek addr\n");
    glog.log("    Read a 32bit value from WIB address space\n");
    glog.log("  poke addr value\n");
    glog.log("    Write a 32bit value to WIB address space\n");
    glog.log("  cdpeek femb_idx cd_idx chip_addr reg_page reg_addr\n");
    glog.log("    Read a 8bit value from COLDATA I2C address space\n");
    glog.log("  cdpoke femb_idx cd_idx chip_addr reg_page reg_addr data\n");
    glog.log("    Write a 8bit value to COLDATA I2C address space\n");
    glog.log("  cdfastcmd cmd\n");
    glog.log("    Send the fast command cmd to all coldata chips\n");
    glog.log("  update root_archive boot_archive\n");
    glog.log("    Deploy a new root and boot archive to the WIB\n");
    glog.log("  exit\n");
    glog.log("    Closes the command interface\n");
}

template <class R, class C>
void send_command(zmq::socket_t &socket, const C &msg, R &repl) {

    wib::Command command;
    command.mutable_cmd()->PackFrom(msg);
    
    string cmd_str;
    command.SerializeToString(&cmd_str);
    
    zmq::message_t request(cmd_str.size());
    memcpy((void*)request.data(), cmd_str.c_str(), cmd_str.size());
    socket.send(request,zmq::send_flags::none);
    
    zmq::message_t reply;
    socket.recv(reply,zmq::recv_flags::none);
    
    string reply_str(static_cast<char*>(reply.data()), reply.size());
    repl.ParseFromString(reply_str);
    
}

int run_command(zmq::socket_t &s, int argc, char **argv) {
    if (argc < 1) return 1;
    
    string cmd(argv[0]);
    if (cmd == "exit") { 
        return 255;
    } else if (cmd == "get_sensors") {
        wib::GetSensors req;
        wib::GetSensors::Sensors rep;
        send_command(s,req,rep);
    } else if (cmd == "fw_timestamp") {
        wib::GetTimestamp req;
        wib::GetTimestamp::Timestamp rep;
        send_command(s,req,rep);
        glog.log("fw_timestamp code: 0x%08X\n",rep.timestamp());
        glog.log("decoded: %i/%i/%i %i:%i:%i\n",rep.year(),rep.month(),rep.day(),rep.hour(),rep.min(),rep.sec());
    } else if (cmd == "sw_version") {
        wib::GetSWVersion req;
        wib::GetSWVersion::Version rep;
        send_command(s,req,rep);
        glog.log("sw_version: %s\n",rep.version().c_str());
    } else if (cmd == "log") {
        if (argc > 2) {
            glog.log("Usage: log [clear|boot]\n");
            return 0;
        }
        bool boot = false;
        bool clear = false;
        if (argc == 2) {
            string subcmd(argv[1]);
            if (subcmd == "boot") {
                boot = true;
            } else if (subcmd == "clear") {
                clear = true;
            } else {    
                glog.log("Usage: log [clear|boot]\n");
                return 0;
            }
        }
        wib::LogControl req;
        req.set_return_log(!boot && !clear);
        req.set_boot_log(boot);
        req.set_clear_log(clear);
        wib::LogControl::Log rep;
        send_command(s,req,rep);
        const string &log = rep.contents();
        printf("%s",log.c_str());
    } else if (cmd == "script") {
        if (argc != 2) {
            glog.log("Usage: script filename\n");
            return 0;
        }
        string fname(argv[1]);
        ifstream fin(fname);
        if (fin.is_open()) {
            glog.log("Executing local script... ");
            fflush(stdout);
            string script((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
            wib::Script req;
            req.set_script(script);
            req.set_file(false);
            wib::Status rep;
            send_command(s,req,rep);
            if (rep.success()) {
                glog.log("Success\n");
            } else {
                glog.log("Failure\n");
            }
            fin.close();
        } else {
            glog.log("Executing remote script... ");
            fflush(stdout);
            wib::Script req;
            req.set_script(fname);
            req.set_file(true);
            wib::Status rep;
            send_command(s,req,rep);
            if (rep.success()) {
                glog.log("Success\n");
            } else {
                glog.log("Failure\n");
            }
        }
    } else if (cmd == "daqspy") {
        if (argc != 2) {
            glog.log("Usage: daqspy filename\n");
            return 0;
        }
        wib::ReadDaqSpy req;
        req.set_buf0(true);
        req.set_buf1(true);
        wib::ReadDaqSpy::DaqSpy rep;
        glog.log("Acquiring DAQ spy buffer...");
        fflush(stdout);
        send_command(s,req,rep);
        string fname(argv[1]);
        ofstream fout(fname,ofstream::binary);
        fout.write(rep.buf0().c_str(),rep.buf0().size());
        fout.write(rep.buf1().c_str(),rep.buf1().size());
        fout.close();
        if (rep.success()) {
            glog.log("Success\n");
        } else {
            glog.log("Failure\n");
        }
    } else if (cmd == "calibrate") {
        if (argc != 1) {
            glog.log("Usage: calibrate\n");
            return 0;
        }
        wib::Calibrate req;
        wib::Status rep;
        send_command(s,req,rep);
        if (rep.success()) {
            glog.log("Success\n");
        } else {
            glog.log("Failure\n");
        }
    } else if (cmd == "update") {
        if (argc != 3) {
            glog.log("Usage: update root_archive boot_archive\n");
            return 0;
        }
        
        size_t length;
        string root_archive, boot_archive;
        
        ifstream in_root(argv[1], ios::binary);
        if (!in_root.is_open()) {
            glog.log("Could not open root archive: %s\n",argv[1]);
            return 0;
        }
        in_root.ignore( numeric_limits<streamsize>::max() );
        length = in_root.gcount();
        in_root.clear();
        in_root.seekg( 0, ios_base::beg );
        root_archive.resize(length);
        in_root.read((char*)root_archive.data(),length);
        in_root.close();
        
        ifstream in_boot(argv[2], ios::binary);
        if (!in_boot.is_open()) {
            glog.log("Could not open boot archive: %s\n",argv[2]);
            return 0;
        }
        in_boot.ignore( numeric_limits<streamsize>::max() );
        length = in_boot.gcount();
        in_boot.clear();
        in_boot.seekg( 0, ios_base::beg );
        boot_archive.resize(length);
        in_boot.read((char*)boot_archive.data(),length);
        in_boot.close();
        
        wib::Update req;
        glog.log("Sending root archive (%0.1f MB) and boot archive (%0.1f MB)\n",root_archive.size()/1024.0/1024.0,boot_archive.size()/1024.0/1024.0);
        req.set_root_archive(root_archive);
        req.set_boot_archive(boot_archive);
        wib::Empty rep;
        send_command(s,req,rep);
    } else if (cmd == "reboot") {
        wib::Reboot req;
        wib::Empty rep;
        send_command(s,req,rep);
    } else if (cmd == "peek") {
        if (argc != 2) {
            glog.log("Usage: peek addr\n");
            glog.log("   all arguments are base 16\n");
            return 0;
        }
        wib::Peek req;
        req.set_addr((size_t)strtoull(argv[1],NULL,16));
        wib::RegValue rep;
        send_command(s,req,rep);
        glog.log("0x%X\n",rep.value());
    } else if (cmd == "poke") {
        if (argc != 3) {
            glog.log("Usage: poke addr value\n");
            glog.log("   all arguments are base 16\n");
            return 0;
        }
        wib::Poke req;
        req.set_addr((size_t)strtoull(argv[1],NULL,16));
        req.set_value((uint32_t)strtoull(argv[2],NULL,16));
        wib::RegValue rep;
        send_command(s,req,rep);
    } else if (cmd == "cdpeek") {
        if (argc != 6) {
            glog.log("Usage: cdpeek femb_idx cd_idx chip_addr reg_page reg_addr\n");
            glog.log("   femb_idx and cd_idx are base 10, remaining args are base 16\n");
            return 0;
        }
        wib::CDPeek req;
        req.set_femb_idx((uint8_t)strtoull(argv[1],NULL,10));
        req.set_coldata_idx((uint8_t)strtoull(argv[2],NULL,10));
        req.set_chip_addr((uint8_t)strtoull(argv[3],NULL,16));
        req.set_reg_page((uint8_t)strtoull(argv[4],NULL,16));
        req.set_reg_addr((uint8_t)strtoull(argv[5],NULL,16));
        wib::CDRegValue rep;
        send_command(s,req,rep);
        glog.log("0x%X\n",rep.data());
    } else if (cmd == "cdpoke") {
        if (argc != 7) {
            glog.log("Usage: cdpoke femb_idx cd_idx chip_addr reg_page reg_addr data\n");
            glog.log("   femb_idx and cd_idx are base 10, remaining args are base 16\n");
            return 0;
        }
        wib::CDPoke req;
        req.set_femb_idx((uint8_t)strtoull(argv[1],NULL,10));
        req.set_coldata_idx((uint8_t)strtoull(argv[2],NULL,10));
        req.set_chip_addr((uint8_t)strtoull(argv[3],NULL,16));
        req.set_reg_page((uint8_t)strtoull(argv[4],NULL,16));
        req.set_reg_addr((uint8_t)strtoull(argv[5],NULL,16));
        req.set_data((uint8_t)strtoull(argv[6],NULL,16));
        wib::CDRegValue rep;
        send_command(s,req,rep);
    } else if (cmd == "cdfastcmd") {
        if (argc != 2) {
            glog.log("Usage: cdfastcmd cmd\n");
            glog.log("   cmd can be: reset, act, sync, edge, idle, edge_act\n");
            return 0;
        }
        wib::CDFastCmd req;
        string cmd(argv[1]);
        if (cmd == "reset") {
            req.set_cmd(FAST_CMD_RESET);
        } else if (cmd == "act") {
            req.set_cmd(FAST_CMD_ACT);
        } else if (cmd == "sync") {
            req.set_cmd(FAST_CMD_SYNC);
        } else if (cmd == "edge") {
            req.set_cmd(FAST_CMD_EDGE);
        } else if (cmd == "idle") {
            req.set_cmd(FAST_CMD_IDLE);
        } else if (cmd == "edge_act") {
            req.set_cmd(FAST_CMD_EDGE_ACT);
        } else {
            glog.log("Unknown fast command: %s\n",argv[1]);
            glog.log("Valid fast commands: reset, act, sync, edge, idle, edge_act\n");
            return 0;
        }
        wib::Empty rep;
        send_command(s,req,rep);
    } else if (cmd == "help") {
        print_help();
    } else {
        glog.log("Unrecognized Command: %s\n",argv[0]);
        return 0;
    }
    return 0;
}

int main(int argc, char **argv) {
    
    char *ip = (char*)"127.0.0.1";
    
    signed char opt;
    while ((opt = getopt(argc, argv, "w:h")) != -1) {
       switch (opt) {
           case 'h':
               print_usage(argv[0]);
               print_help();
               return 1;
           case 'w':
               ip = optarg;
               break;
           default: /* '?' */
               print_usage(argv[0]);
               return 1;
       }
    }
    
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REQ);
    
    char *addr;
    int len = asprintf(&addr,"tcp://%s:1234",ip);
    if (len < 0) return 1;
    
    socket.connect(addr);
    free(addr);
    
    if (optind < argc) {
        return run_command(socket,argc-optind,argv+optind);
    } else {
        char* buf;
        while ((buf = readline(">> ")) != nullptr) {
            if (strlen(buf) > 0) {
                add_history(buf);
            } else {
                free(buf);
                continue;
            }
            char *delim = (char*)" ";   
            int count = 1;
            char *ptr = buf;
            while((ptr = strchr(ptr, delim[0])) != NULL) {
                count++;
                ptr++;
            }
            if (count > 0) {
                char **cmd = new char*[count];
                cmd[0] = strtok(buf, delim);
                int i;
                for (i = 1; cmd[i-1] != NULL && i < count; i++) {
                    cmd[i] = strtok(NULL, delim);
                }
                if (cmd[i-1] == NULL) i--;
                int ret = run_command(socket,i,cmd);
                delete [] cmd;
                if (ret == 255) return 0;
                if (ret != 0) return ret;
            } else {
                return 0;
            }
            free(buf);
        }
    }
    
    return 0;
}
