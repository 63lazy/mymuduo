#pragma once
#include "noncopy.h"
class InetAddress;
class Socket : NonCopyable{
public:
    explicit Socket(int sockfd)
        :sockfd_(sockfd)
    {}
    ~Socket();
    int fd(){return sockfd_;}
    void bindAddress(const InetAddress &loacaladdr);
    void listen();
    int accept(InetAddress *peeraddr);

    void shutdownWrite();

    void setTcpNonDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setReuseAlive(bool on);
    void setKeepAlive(bool on);

    static int getSocketError(int sockfd);
private:
    const int sockfd_;
};