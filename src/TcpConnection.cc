#include "TcpConnection.h"
#include "../utils/logger.h"
#include "EventLoop.h"
#include "Channel.h"
#include "socket.h"
static EventLoop *CheckloopNotNull(EventLoop *loop){
    if(loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d TcpConnection loop is null!\n"__FILE__,__FUNCTION__,__LINE__);
    }
    return loop;
}
TcpConnection::TcpConnection(EventLoop *loop,
                  const std::string &name,
                  int sockfd,
                  const InetAddress& localAddr,
                  const InetAddress& peerAddr):
                  loop_(CheckloopNotNull(loop)),
                  name_(name),
                  state_(kConnecting),
                  reading_(true),
                  socket_(new Socket(sockfd)),
                  channel_(new Channel(loop,sockfd))
                  localAddr_(localAddr),
                  peerAddr_(peerAddr)
{}

TcpConnection::~TcpConnection{

}