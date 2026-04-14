#pragma once
#include <unistd.h>
#include <sys/syscall.h>
namespace CurrentThread{
    extern thread_local int t_cachedTid;

    void cacheTid();

    inline int tid(){
        if(t_cachedTid == 0){
            cacheTid();
        }
    return t_cachedTid;
        
    }
}