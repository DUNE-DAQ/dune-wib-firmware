#include "log.h"
#include <cstdio>
#include <stdarg.h>

Log glog;

Log::Log(bool _echo) : echo(_echo) {
    mark();
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

void Log::store(std::string *storage) {
    *storage = buf.str();
}

void Log::clear() {
    buf.str("");
    buf.clear();
    mark();
}

void Log::mark() {
    last_mark = buf.tellp();
}

void Log::store_mark(std::string *storage) {
    *storage = buf.str().substr(last_mark);
}
