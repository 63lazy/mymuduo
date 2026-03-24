#include"Channel.h"
#include<sys/epoll.h>
#include"EventLoop.h"
#include "../utils/logger.h"

const int Channel::kNoneEvent=0;
const int Channel::kReadEvent=EPOLLIN|EPOLLPRI;
const int Channel::kWriteEvent=EPOLLOUT;    

Channel::Channel(EventLoop* loop,int fd):
    loop_(loop),
    fd_(fd),
    events_(kNoneEvent),
    index_(-1), 
    tied_(false)
{};
//tie_用来绑定channel和TcpConnection，防止channel被销毁时，TcpConnection还在使用

//如果tied_为true说明此时运用channel的是TcpConnection,需要通过tie来判断对象是否还在使用
//反之则是Acceptor或者EventLoop，无需进行生命周期的判断
void Channel::tie(const std::shared_ptr<void>& obj){ 
    tie_=obj;
    tied_=true;
}

void Channel::update(){
    loop_->updateChannel(this);
}
void Channel::remove(){
    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp recieveTime){
    std::shared_ptr<void> guard;
    if(tied_){
        guard=tie_.lock();
        if(guard){
            handleEventWithGuard(recieveTime);
        }
    }
    else{
        handleEventWithGuard(recieveTime);
    }
}
//根据poller通知的channel发生的具体事件，由channel负责调用具体的回调操作
void Channel::handleEventWithGuard(Timestamp recieveTime){
    LOG_INFO("Channel %d handleEventWithGuard revents=%d",fd_,revents_);

    if(revents_&EPOLLHUP &&!(revents_&EPOLLIN)){
        if(closeCallback_){
            closeCallback_();
        }
    }
    if(revents_&EPOLLERR){
        if(errorCallback_){
            errorCallback_();
        }
    }
    if(revents_&(EPOLLIN|EPOLLPRI)){
        if(readCallback_){
            readCallback_(recieveTime);
        }
    }
    if(revents_&EPOLLOUT){
        if(writeCallback_){
            writeCallback_();
        }
    }
}
