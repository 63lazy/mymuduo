#pragma once
#include"noncopy.h"
#include <functional>
#include <thread>
#include <memory>
#include <unistd.h>
#include <string>
#include <atomic>
class Thread :NonCopyable{
public:
    using ThreadFunc=std::function<void()>;
    explicit Thread(ThreadFunc,const std::string& name="Thread");
    ~Thread();
    void start();
    void join();
    bool started() const{return started_;}
    pid_t tid() const{return tid_;}
    const std::string& name() const{return name_;}
    static int numCreated(){return numCreated_;}
private:
    void setDefaultName();
    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;
    
    pid_t tid_;     //记录该线程的id
    ThreadFunc func_;   
    std::string name_;
    static std::atomic<int> numCreated_;
};