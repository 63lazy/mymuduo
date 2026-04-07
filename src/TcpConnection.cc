#include "TcpConnection.h"
#include "../utils/logger.h"
#include "EventLoop.h"
#include "Channel.h"
#include "socket.h"
#include "../utils/logger.h"
#include <functional>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <strings.h>
#include <netinet/tcp.h>
#include <string>
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
                  peerAddr_(peerAddr),
                  highWaterMark_(64*1024*1024) //64M到达水位线
{
    //给channel设置相应的回调函数
    channel_->setReadCallback([this](Timestamp recieveTime){
        handleRead(recieveTime);
    });
    channel_->setWriteCallback([this](){
        handleWrite();
    });
    channel_->setCloseCallback([this](){
        handleClose();
    });
    channel_->setErrorCallback([this](){
        handleError();
    });
    LOG_INFO("TcpConnection::ctor[%s] at fd = %d\n",name_.c_str(),sockfd)
}

TcpConnection::~TcpConnection{
    LOG_INFO("TcpConnection::dtor[%s] at fd=%d state=%d\n",
            name_.s_str(),channel->fd(),(int)state_);
}

void TcpConnection::send(const std::string &buf)
{
    if(state_==kConnected){
        //不跨线程直接执行
        if(loop_->isInLoopThread){
            sendInLoop(buf.str_c(),buf.size())
        }
        //跨线程会直接调用queueInLoop保证线程安全
        else{
            loop_->runInLoop([this](buf.str_c(),buf.size()){
                sendInLoop
            })
        }
    }
}
//应用写的快 但内核发送数据慢 因此需要把数据写入缓冲区并设置水位回调
void TcpConnection::sendInLoop(const void *data, size_t len){
    ssize_t nwrote=0;
    //待发送的数据长度
    ssize_t remaining=len;
    bool faultError = false;
    if(state_==kDisconnected)
    {
        LOG_ERROR("dissconnected, give up writing\n");
        return ;
    }
    //readableBytes()==0表示缓冲区没有待发送数据
    if(channel_->IsWriteEnabled() && outputBuffer->readableBytes()==0){
        nwrote=::write(channel_->fd(),data,len);
        if(nwrote>=0){
            remaining=len - nwrote;
            //数据一次性发送完毕 直接执行writeCompleteCallback_
            if(remaining==0 && writeCompleteCallback_){
                loop_->queueInLoop([this](shared_from_this()){
                    writeCompleteCallback_;
                })
            }
        }
        else
        {
            nwrote=0;
            if(errno!=EWOULDBLOCK){
                LOG_ERROR("TcpConnection::sendInLoop\n");
                if(errno == EPIPE||errno == ECONNRESET){
                    faultError= true;
                }
            }
        }
    }
    //说明一次write没发完 剩余数据保存到缓冲区供epoll监控唤醒
    if(!faultError && remaining>0){
        //目前缓冲区待发送数据
        ssize_t oldlen = outputBuffer->readableBytes();
        if(oldlen+remaining>=highWaterMark_
            &&oldlen <highWaterMark_
            &&highWaterMarkCallback_)
        {
            loop_->queueInLoop([this](shared_from_this(),oldlen+remaining){
                highWaterMarkCallback_;
            });
        }
        outputBuffer.append((char*)data+nwrote ,remaining);
        if(!channel_->IsWriteEnabled()){
            channel_->enableWriting(); //注册channel写事件
        }
    }

}
//用户调用的函数
void TcpConnection::shutdown()
{
    if(state_==kConnected){
        setState(kDisconnecting);
        loop_->runInLoop([this](){
            shutdownInLoop();
        })
    }
}
void TcpConnection::shutdownInLoop()
{
    //说明channel outputbuffer数据发送完了 
    if(!channel_->IsWriteEnabled){
        //关闭写端 会触发channel的closeCallback_即TcpConnection::handleClose
        socket_->shutdownWrite();   
    }
}

void TcpConnection::connectEstablished(){
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->disableReading();

    //新连接建立，执行回调
    connectionCallback_(shared_from_this());
}
    
void TcpConnection::connectDestroyed(){
    if(state_==kConnected){
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}
    
void TcpConnection::handleRead(Timestamp recieveTime){
    int savedError = 0;
    ssize_t n= inputBuffer_.readFd(socket_->fd(), &saveErrno);
    if(n>0){
        //已建立连接的用户，有可读事件发生了，调用用户传入的回调操作
        messageCallback_(shared_from_this(),&inputBuffer_,recieveTime);
    }
    else if(n==0){
        handleClose();
    }
    else{
        errno=savedError;
        LOG_ERROR("TcpConnection::handleRead\n");
        handleError();
    }
}
void TcpConnection::handleWrite(){
    //判断channel是否可写
    if(channel_->IsWriteEnabled())
    {
        ssize_t n=::write(channel_->Fd(),
                          outputBuffer.peek(),
                          outputBuffer.readableBytes());
        if(n>0)
        {
            outputBuffer.retrieve(n);
            if(outputBuffer.readableBytes()==0)
            {
                channel_.disableWriting();
                if(writeCompleteCallback_)
                {
                    //queueInLoop等io结束再处理客户回调逻辑 防止用户回调影响连接或者io
                    loop_.queueInLoop([this](){
                        writeCompleteCallback_(shared_from_this);
                    });
                }
                if(state_==kDisconnecting)
                {
                    //确保outputBuffer里的数据全部发完再shutdown
                    shutdownInLoop();
                }
            }
        }
        else{
            LOG_ERROR("TcpConnection::handleWrite\n",channel_->fd());
        }
    }
    else{
        LOG_ERROR("TcpConnection fd:%d is down,no more writing\n",)
    }
}
void TcpConnection::handleClose()
{
    LOG_INFO("TcpConnection::handleClose fd=%d state=%d \n",channel_fd(),(int)state_);
    setState(kDisconnected);
    channel_->disableAll();

    //connPtr增加一个引用计数延长TcpConnection对象的生命至整个handleClose函数执行完毕
    //closeCallback_就会将TcpServer中的TcpConnectionPtr删除
    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr);    //执行业务层清理 受众是最终用户即程序员
    closeCallback_(connPtr);         //执行框架层清理 受众是TcpServer / Acceptor
}
void TcpConnection::handleError(){
    int err = sockets::getSocketError(channel_->fd());
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d \n",name_.c_str(),err); 
}