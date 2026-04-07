#include "EventLoopThread.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb,
    const std::string &name)
    :loop_(nullptr)
    ,exit_(false)
    ,thread_([this](){
        threadFunc();
    },name)
    ,callback_(std::move(cb))
    ,mutex_()
    ,cond_()
    {}
EventLoopThread::~EventLoopThread(){
    exit_=true;
    if(loop_!=nullptr){
        loop_->quit();
        thread_.join();
    }
}
EventLoop* EventLoopThread::startLoop(){
    thread_.start();
    //用栈上的loop来中转，防止在锁外操作返回loop_
    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        //保证在创建EventLoop对象前，线程已经创建完成//
        while(loop_==nullptr){
            cond_.wait(lock);
        }
        loop=loop_;
    }
    return loop;
}
//EventLoopThread线程函数，创建EventLoop对象
void EventLoopThread::threadFunc(){
    //不用new可以保证EventLoop在栈上创建，只作为局部变量防止内存泄露
    EventLoop loop;
    if(callback_!=nullptr){
        callback_(&loop);
    }
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_=&loop;
        cond_.notify_one();
    }
    loop.loop();
    
    //退出循环说明执行了quit函数
    std::unique_lock<std::mutex> lock(mutex_);
    loop_=nullptr;
}
