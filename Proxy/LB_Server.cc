#include <memory>
#include <mymuduo/TcpServer.h>
#include <mymuduo/logger.h>
#include "RomoteClient.h"
#include "Selector.h"
class LB_Server{
public:

    LB_Server(EventLoop *loop,
                const InetAddress &listenAddr,
                const std::string &nameArg
                ):
                loop_(loop),
                server_(loop,listenAddr,nameArg),
                selector_(loop)
    {
        server_.setConnectionCallback([this](const TcpConnectionPtr &conn){
            onConnection(conn);
        });
        server_.setMessageCallback([this](const TcpConnectionPtr &conn,
                                            Buffer* buf, 
                                            Timestamp time)
        {
            onMessage(conn,buf,time);
        });
        server_.setThreadNum(1);
        
        selector_.addServer(8001,"127.0.0.1",5);
        selector_.addServer(8002,"127.0.0.1",1);
        selector_.addServer(8003,"127.0.0.1",1);
    }
    void start(){
        server_.start();
    }
private:
    void onConnection(const TcpConnectionPtr &conn){
        if(conn->connected()){
            InetAddress addr=std::move(selector_.getNextServer());
            if(addr.toIp()=="0.0.0.0"){
                conn->shutdown();
            }
            LOG_INFO("LB_Server:TcpServer connection UP :%s",conn->peerAddress().toIpPort().c_str());
            //保证了rClient和LB_client属于同一个线程
            std::string name="LB_Client";
            auto rClient =std::make_shared<RemoteClient>(loop_,addr,name,conn);
        
            conn->setContext(rClient);
            rClient->connect();
        }
        else{
            LOG_INFO("LB_Server:TcpServer connection DOWN :%s",conn->peerAddress().toIpPort().c_str());
            //相比RemoteClient的onConnection不需要检查any里是否有内容，因为只要进入了这里rClient就一定存活
            auto* rClient_ptr=std::any_cast<std::shared_ptr<RemoteClient>>(&conn->getContext());
            //彻底断开连接并禁止重连
            (*rClient_ptr)->stop();
        }
    }
    void onMessage(const TcpConnectionPtr &conn,Buffer *buf,Timestamp time){
        auto *rClient_ptr=std::any_cast<std::shared_ptr<RemoteClient>>(&conn->getContext());

        (*rClient_ptr)->sendToBackend(buf);
    }
    EventLoop *loop_;
    TcpServer server_;
    Selector selector_;
};
int main(){
    EventLoop loop;
    InetAddress addr(8000);

    LB_Server server(&loop,addr,"LB_server");
    server.start();

    loop.loop();
    
};