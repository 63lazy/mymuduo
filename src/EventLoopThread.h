#pragma once
#include "../utils/noncopy.h"
#include "../utils/thread.h"
#include "EventLoop.h"
#include "../utils/thread.h"
#include <functional>
#include <mutex>
#include <condition_variable>
class EventLoop;
class EventLoopThread : NonCopyable{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThread(ThreadInitCallback const &cb=ThreadInitCallback(),
        const std::string &name=string());
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
}