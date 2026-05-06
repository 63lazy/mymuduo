#pragma once
#include "socket.h"
#include "noncopy.h"
#include "EventLoop.h"
#include "Callbacks.h"
#include "Connector.h"
#include <atomic>
#include<memory>
class TcpClient : NonCopyable,std::enable_shared_from_this<TcpClient>{
public:
    TcpClient(EventLoop *loop,InetAddress &serverAddr,std::string &name);
    void connect();
    void disconnect();
    void stop();
    void enableRetry(){retry_ = true;}
private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnectionPtr &conn);
    
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;   
    WriteCompleteCallback writeCompleteCallback_;

    EventLoop *loop_;
    std::unique_ptr<Connector> connector_;
    TcpConnectionPtr conn_;

    std::string name_;
    int nexConnId_;
    std::mutex mutex_;
    std::atomic_bool retry_;
    std::atomic_bool connect_;

};
