#include "EventLoop.h"
#include "logger.h"
#include "Poller.h"
#include "Channel.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <memory>
//保证每个线程只有一个EventLoop对象
thread_local EventLoop* t_loopInThisThread = nullptr;
//epoll_wait超时时间
const int kPollTimeMs = 10000;
//创建wakeupfd用来唤醒subloop处理新来的channel
int createEventFd(){
    int evtfd = ::eventfd(0,EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0){
        LOG_FATAL("eventfd error:%d ",errno);
    }
    return evtfd;
}
EventLoop::EventLoop()
    :looping_(false),
    quit_(false),
    threadId_(CurrentThread::tid()),
    poller_(Poller::newDefaultPoller(this)),
    wakeupFd_(createEventFd()),
    wakeupChannel_(new Channel(this,wakeupFd_)),
    callingPendingFunctors_(false)
{ 
    LOG_DEBUG("EventLoop created %p in thread %d",this,threadId_);
    if(t_loopInThisThread){
        LOG_FATAL("Another EventLoop %p exists in this thread %d",t_loopInThisThread,threadId_);
    }
    else{
        t_loopInThisThread=this;
    }

    //设置wakeupChannel的事件类型为可读事件
    wakeupChannel_->setReadCallback([this](Timestamp t){
        this->handleReading();
    });
    //设置wakeupChannel的事件类型为可读事件
    wakeupChannel_->enableReading();

}   

EventLoop::~EventLoop(){
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    close(wakeupFd_);
    t_loopInThisThread=nullptr;
}

void EventLoop::handleReading(){
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_,&one,sizeof(one));
    if(n != sizeof(one)){
        LOG_ERROR("EventLoop::handleReading() reads %ld bytes instead of 8",n);
    }
}

void EventLoop::loop(){
    looping_=true;
    quit_=false;
    LOG_INFO("EventLoop %p loop started",this);

    while(!quit_){
        activeChannels_.clear();
        pollReturnTime_=poller_->poll(kPollTimeMs,&activeChannels_);
        for(auto *channel:activeChannels_){
            channel->handleEvent(pollReturnTime_);
        }
        //执行mainloop注册的所有回调操作
        doPendingFunctors();
    }
    LOG_INFO("EventLoop %p loop ended",this);
    looping_=false;
}
void EventLoop::quit(){
    quit_=true;
    if(!isInLoopThread()){//如果当前线程不是自己的loop线程，需要唤醒loop线程 保证loop线程马上退出
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb){
    if(isInLoopThread()){
        cb();
    }
    else{
        queueInLoop(cb);
    }
}
//将cb加入到事件循环的事件队列中，等待loop处理
void EventLoop::queueInLoop(Functor cb){
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }
    //唤醒loop线程处理事件队列中的回调操作
    if(!isInLoopThread()||!callingPendingFunctors_){
        //判断callingPendingFunctors_防止循环阻塞在poll中，让循环处理新的回调操作
        wakeup();
    }
}
//即向wakeupChannel写入一个字节，触发事件循环的事件处理
void EventLoop::wakeup(){
    uint64_t one = 1;
    int n=write(wakeupFd_,&one,sizeof(one));
    if(n != sizeof(one)){
        LOG_ERROR("EventLoop::wakeup() writes %d bytes instead of 8",n);
    }
}
//更新channel的事件类型
void EventLoop::updateChannel(Channel* channel){
    poller_->updateChannel(channel);
}
void EventLoop::removeChannel(Channel* channel){
    poller_->removeChannel(channel);
}
bool EventLoop::hasChannel(Channel* channel){
    return poller_->hasChannel(channel);
}
void EventLoop::doPendingFunctors(){
    std::vector<Functor> functors;
    callingPendingFunctors_=true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for(const auto &functor:functors){
        functor();
    }
    callingPendingFunctors_=false;
}
