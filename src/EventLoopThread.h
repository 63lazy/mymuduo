#pragma once
#include "noncopy.h"
#include "thread.h"
#include "EventLoop.h"
#include "thread.h"
#include <functional>
#include <mutex>
#include <string>
#include <condition_variable>
class EventLoop;
class EventLoopThread : NonCopyable{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
        const std::string &name=std::string());
    ~EventLoopThread();
    EventLoop* startLoop();
private:
    void threadFunc();

    EventLoop* loop_;
    bool exit_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};