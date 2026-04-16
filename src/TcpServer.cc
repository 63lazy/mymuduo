#include "TcpServer.h"
#include "logger.h"
#include "TcpConnection.h"
#include "EventLoop.h"
#include "Callbacks.h"
#include <string>
#include <strings.h>
static EventLoop *CheckloopNotNull(EventLoop *loop)
{
    if(loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainloop is null!",__FILE__,__FUNCTION__,__LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop,
                    const InetAddress &listenAddr,
                    const std::string &nameArg,
                    Option option)
                    :loop_(CheckloopNotNull(loop)),
                    name_(nameArg),
                    ipPort_(listenAddr.toIpPort()),
                    acceptor_(new Acceptor(loop,listenAddr,option==kReusePort)),
                    threadpool_(new EventLoopThreadPool(loop,name_)),
                    connectionCallback_(),
                    messageCallback_(),
                    nexConnId_(1)
{
    //当有用户连接时执行newConnection函数执行连接
    acceptor_->setNewConnectionCallback([this](int sockfd,const InetAddress &peerAddr){
        newConnection(sockfd,peerAddr);
    });
}

TcpServer::~TcpServer(){
    for(auto &item: connections_){
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop([conn](){
            conn->connectDestroyed();
        });
    }
}

void TcpServer::start(){
    if(started_++ == 0){      //防止一个tcpserver被start多次
        threadpool_->start();
        loop_->runInLoop([this](){
            acceptor_->listen();
        });
    }
}
//根据轮询算法选择一个subloop并唤醒 把connfd封装成channl并分发给subloop
void TcpServer::newConnection(int sockfd,const InetAddress &peerAddr)
{
    EventLoop *ioloop=threadpool_->getNextLoop();
    char buf[64]={0};
    snprintf(buf,sizeof(buf),"%s#%d",ipPort_.c_str(),nexConnId_);
    ++nexConnId_;
    std::string connName_= name_+buf;

    LOG_INFO("TcpServer::newConnection[%s] new connection [%s] from %s",
            name_.c_str(),connName_.c_str(),peerAddr.toIpPort().c_str());
    
    //通过sockfd获取其绑定的本机ip地址和端口信息
    sockaddr_in local;
    ::bzero(&local,sizeof(local));
    socklen_t addrlen=sizeof(struct sockaddr_in);
    if(::getsockname(sockfd,(sockaddr*)&local,&addrlen)<0)
    {
        LOG_ERROR("TcpServer::getLocalAddr");
    }

    InetAddress localAddr(local);
    
    TcpConnectionPtr conn(new TcpConnection(ioloop,
                                            connName_,
                                            sockfd,
                                            localAddr,
                                            peerAddr));
    connections_[connName_]=conn;
    //用户设置给TcpServer=>TcpConnection=>channel=>注册到poller=>notify channel调用
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    //设置了如何关闭连接的回调
    conn->setCloseCallback([this](const std::shared_ptr<TcpConnection> &c){
        removeConnection(c);
    });
    ioloop->runInLoop([conn](){
        conn->connectEstablished();
    });
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop([this,conn](){
        removeConnectionInLoop(conn);
    });
}
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    LOG_INFO("TcpServer::removeConnection [%s] - connection %s ",
             name_.c_str(),conn->name().c_str());
    size_t n=connections_.erase(conn->name());
    EventLoop *ioloop=conn->getLoop();
    ioloop->queueInLoop([conn](){
        conn->connectDestroyed();
    });

};