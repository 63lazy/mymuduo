#include "TcpServer.h"
#include "../utils/logger.h"
#include "EventLoop.h"
#include "Callbacks.h"
#include <string>
static EventLoop *CheckloopNotNull(EventLoop *loop){
    if(loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainloop is null!\n"__FILE__,__FUNCTION__,__LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop,
                    const InetAddress &listenAddr,
                    const std::string &nameArg,
                    Option option=kNoReusePort)
                    :loop_(CheckloopNotNull(loop)),
                    ipPort_(listenAddr.toIpPort()),
                    name_(nameArg),
                    acceptor_(new Acceptor(loop,listenAddr,option=kReusePort)),
                    threadpool_(new EventLoopThreadPool(loop,name_)),
                    connectionCallback_(),
                    messageCallback_(),
                    nexConnId_(1)
{
    //当有用户连接时执行newConnection函数执行连接
    acceptor_.setNewConnectionCallback([this](int sockfd,const InetAddress &peerAddr){
        newConnection(sockfd,peerAddr);
    });


}
//根据轮询算法选择一个subloop并唤醒 把connfd封装成channl并分发给subloop
void TcpServer::newConnection(){

}
void TcpServer::start(){
    if(started++ == 0){      //防止一个tcpserver被start多次
        threadpool_.start();
    }
}