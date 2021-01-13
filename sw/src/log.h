#ifndef log_h
#define log_h

#include <string>
#include <sstream>

class Log {

    public:
        Log(bool echo = true);
        ~Log();
        
        // Log a message with printf syntax
        void log(const char *format, ...);
        
        // Store the current log to *storage
        void store(std::string *storage);
        
        // Clear the entire log
        void clear();
        
        // Mark a position in the log
        void mark();
        
        // Store the log since the last mark to *storage
        void store_mark(std::string *storage);
        
    private:
        size_t last_mark;
        bool echo;
        std::ostringstream buf;
};

extern Log glog;

#endif
