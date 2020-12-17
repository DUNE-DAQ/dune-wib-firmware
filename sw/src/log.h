#ifndef log_h
#define log_h

#include <string>
#include <sstream>

class Log {

    public:
        Log(bool echo = true);
        ~Log();
        
        void log(const char *format, ...);
        void store(std::string *buf);
        void clear();
        
    private:
        bool echo;
        std::ostringstream buf;
};

extern Log glog;

#endif
