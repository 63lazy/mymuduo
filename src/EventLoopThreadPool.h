#pragma once
#include "../utils/noncopy.h"
#include "EventLoopThread.h"
#include<vector>
#include<functional>
#include <string>
#include <memory>
class EventLoop;
class EventLoopThread;
class EventLoopThreadPool : NonCopyable{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>; 
    EventLoopThreadPool(EventLoop* baseloop, const std::string &name);
    //EventLoop都是栈上对象，只在作用域生效，无需在析构函数时删除
    void setThreadNum(int threadNum){numThreads_=threadNum;}
    void start(const ThreadInitCallback &cb=ThreadInitCallback());
    //默认轮询分配
    EventLoop* getNextLoop();  
    std::vector<EventLoop*> getAllLoops();
    bool started()const{return started_;}
    const std::string Name()const{return name_;}

private:
    EventLoop* baseloop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    //通过调用EventLoopThread的startLoop函数，获取EventLoop对象的指针，添加到loops_中，
    std::vector<EventLoop*> loops_;
};
