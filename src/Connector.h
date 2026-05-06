#pragma once
#include "InetAddress.h"
#include "EventLoop.h"
#include "Channel.h"
#include "noncopy.h"
#include "memory"
#include <atomic>
class Connector :NonCopyable ,std::enable_shared_from_this<Connector>{
public:
    Connector(EventLoop *loop,InetAddress serverAddr);
    using NewConnectionCallback=std::function<void (int sockfd)>;
    void setNewConnectionCallback(const NewConnectionCallback& cb){ newConnectionCallback_ = cb; }
    InetAddress getServerAddr()const{ return addr_;}
    
    //开始请求连接
    void start();
    void restart();
    void stop();
private:
    static const int kInitRetryDelayMs = 500;
    static const int kMaxRetryDelayMs = 30*1000;
    enum StateE{kDisconnected,kConnected,kConnecting};

    
    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void retry(int sockfd);

    void handleError();
    void handleWrite();
    void setState(StateE state){state_= state;}

    InetAddress addr_;
    EventLoop *loop_;
    std::atomic_int state_;
    std::atomic_bool connect_;
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback newConnectionCallback_;
    int retryDelayMs_;
};