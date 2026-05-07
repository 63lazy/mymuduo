#pragma once
#include "socket.h"
#include "noncopy.h"
#include "EventLoop.h"
#include "Callbacks.h"
#include "Connector.h"
#include "logger.h"
#include "TcpConnection.h"
#include <atomic>
#include<memory>
class TcpClient :public NonCopyable,public std::enable_shared_from_this<TcpClient>{
public:
    TcpClient(EventLoop *loop,InetAddress &serverAddr,std::string &name);
    void connect();
    void disconnect();
    void stop();
    void enableRetry(){retry_ = true;}

    void setConnectionCallback(const ConnectionCallback cb){connectionCallback_ = std::move(cb);}
    void setMessageCallback(const MessageCallback cb){messageCallback_ = std::move(cb);}
    void setWriteCompleteCallback(const WriteCompleteCallback cb){writeCompleteCallback_ = std::move(cb);}

    void send(std::string data){ 
        if(conn_)
            conn_->send(data); 
        else
            LOG_ERROR("Cannot send, connection is not established yet!");
    }
private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnectionPtr &conn);
    
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;   
    WriteCompleteCallback writeCompleteCallback_;

    EventLoop *loop_;
    std::shared_ptr<Connector> connector_;
    TcpConnectionPtr conn_;

    std::string name_;
    int nexConnId_;
    std::mutex mutex_;
    std::atomic_bool retry_;
    std::atomic_bool connect_;

};
