#include "Acceptor.h"
#include "InetAddress.h"
#include "../utils/Timestamp.h"
#include "../utils/logger.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <error.h>
#include <unistd.h>

namespace
{//匿名空间保证createNonblocking函数只能在本文件中被调用
    int createNonblocking()
    {
        int sockfd = ::socket(AF_INET ,SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,0);
        if(sockfd<0)
        {
            LOG_FATAL("%s:%s:%d listen socket create err:%d \n",__FILE__,__FUNCTION__,__LINE__,errno);
        }
        return sockfd;
    }
    
}

Acceptor::Acceptor(EventLoop *loop,const InetAddress &ListenAddr,bool reuseport)
    :loop_(loop),
    acceptSocket_(createNonblocking()),//创建socket
    acceptChannel_(loop,acceptSocket_.fd()),
    listenning_(false)
{
    acceptSocket_.setReusePort(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(ListenAddr);//绑定socket
    acceptChannel_.setReadCallback([this](Timestamp t){
        handleRead();
    });
}
Acceptor::~Acceptor(){
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}
void Acceptor::listen(){
    listenning_=true;
    acceptSocket_.listen();
    acceptChannel_.enableReading(); //将acceptChannel_注册到Poller监听是否有事件发生 有连接了就执行之前Acceptor构造函数注册的handleRead()
}

//listenfd 有新用户连接了执行
void Acceptor::handleRead(){
    InetAddress peerAddr;  //客户端连接
    int connfd=acceptSocket_.accept(&peerAddr);
    if(connfd>=0)
    {
        if(newConnectionCallback_){
            newConnectionCallback_(connfd,peerAddr); //轮询找到subloop，唤醒分发当前的新客户端的channel
        }
        else{
            ::close(connfd);
        }
    }
    else
    {
        LOG_ERROR("%s:%s:%d accept socket err:%d \n",__FILE__,__FUNCTION__,__LINE__,errno);
        if(errno==EMFILE){
            LOG_ERROR("%s:%s:%d sockfd reached limit \n",__FILE__,__FUNCTION__,__LINE__);
        }
    }
}