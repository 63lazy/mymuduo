#pragma once
#include "Channel.h"
#include "Timestamp.h"
#include "Timer.h"
#include "TimerId.h"
#include <set>
class EventLoop;
using Entry=std::pair<Timestamp,Timer*>;
class TimerQueue{
public:
    explicit TimerQueue(EventLoop *loop);

    ~TimerQueue();
    //添加定时任务
    TimerId addTimer(TimerCallback cb,
                  Timestamp when,
                  double interval);
    void addTimerInLoop(Timer*);
private:
    void handleRead();
    std::vector<Entry> getExpired(Timestamp now);
    bool insert(Timer* timer);
    EventLoop *loop_;
    const int timerfd_;
    Channel timerChannel_;
    std::set<Entry> timers_;
    //防止在回调执行中产生重入问题
    bool callingExpiredTimers_;
};