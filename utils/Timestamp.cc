#include"Timestamp.h"
#include<time.h>
#include<cstdio>

Timestamp::Timestamp():microseconds_(0){}
Timestamp::Timestamp(int64_t microseconds):microseconds_(microseconds){}
Timestamp Timestamp::now(){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return Timestamp(ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
}
std::string Timestamp::toString() const{
    char buffer[128]={0};
    time_t time_val = static_cast<time_t>(microseconds_ / 1000000);
    struct tm* tm_info = localtime(&time_val);
    
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
             tm_info->tm_year + 1900,
             tm_info->tm_mon + 1,
             tm_info->tm_mday,
             tm_info->tm_hour,
             tm_info->tm_min,
             tm_info->tm_sec);
    
    return std::string(buffer);
}

