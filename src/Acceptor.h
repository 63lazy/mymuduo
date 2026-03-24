#pragma once
#include "../utils/noncopy.h"
#include "socket.h"
#include "Channel.h"
#include <functional>
class EventLoop;
class InetAddress;
class Acceptor : NonCopyable{
public:
    using NewConnectionCallback=std::function<void(int sockfd,const InetAddress&)> ;
    Acceptor(EventLoop *loop,const EventLoop &ListenAddr,bool reuseport);
    ~Acceptor();
    void setNewConnectionCallback(const NewConnectionCallback &cb){
        newConnectionCallback_=std::move(cb);
    }
    bool listenning(){return listenning_;}
    void listen();
private:
    void handleRead();
    EventLoop* loop_;
    Socket acceptSocket_;
    
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;
};