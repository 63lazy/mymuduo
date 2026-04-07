#pragma once

#include "../utils/noncopy.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "EventLoopThreadPool.h"
#include "TcpConnection.h"
#include "../utils/Buffer.h"

#include <string>
#include <functional>
#include <memory>
#include <atomic>
#include <unordered_map>
class TcpServer : NonCopyable{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    
    enum Option{
        kNoReusePort,
        kReusePort,
    };

    TcpServer(EventLoop *loop,
              const InetAddress &listenAddr,
              const std::string &nameArg,
              Option option=kNoReusePort);
    ~TcpServer();

    void setThreadNum(int numThreads){threadpool_->setThreadNum(numThreads);}
    void setThreadInitCallback(const ThreadInitCallback &cb){threadInitCallback_ = cb;}
    void setConnectionCallback(const ConnectionCallback &cb){connectionCallback_ = cb;}
    void setMessageCallback(const MessageCallback &cb){messageCallback_ = cb;}
    void setWriteCompleteCallback(const WriteCompleteCallback &cb){writeCompleteCallback_ = cb;}
    
    void start(); //开启acceptor的listen
private:
    void newConnection(int sockfd,const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    
    using ConnectionMap= std::unordered_map<std::string,TcpConnectionPtr>;

    EventLoop *loop_;
    const std::string name_;
    const std::string ipPort_;
    std::unique_ptr<Acceptor> acceptor_; 
    std::shared_ptr<EventLoopThreadPool> threadpool_;

    ConnectionCallback connectionCallback_;        //有连接之后的回调
    MessageCallback messageCallback_;                  //有读写消息的回调
    WriteCompleteCallback writeCompleteCallback_;  //消息发送完之后的回调

    ThreadInitCallback threadInitCallback_;          //loop线程初始化的回调

    std::atomic_int started_{0};

    int nexConnId_;

    ConnectionMap connections_;                       //保存所有的连接
};