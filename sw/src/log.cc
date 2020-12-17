#include "log.h"
#include <cstdio>
#include <stdarg.h>

Log glog;

Log::Log(bool _echo) : echo(_echo) {

}

Log::~Log() {

}

void Log::log(const char *format, ...) {
    char *msg;
    va_list args;
    va_start(args, format);
    if (vasprintf(&msg,format,args) < 0) {
        printf("Cannot log: ");
        vprintf(format,args);
        va_end(args);
        return;
    }
    va_end(args);
    buf << msg;
    if (echo) printf("%s",msg);
    free(msg);
}

void Log::store(std::string *store) {
    *store = buf.str();
}

void Log::clear()  {
    buf.str("");
    buf.clear();
}
