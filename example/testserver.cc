#include <mymuduo/TcpServer.h>
#include <mymuduo/logger.h>
#include <string>

class EchoServer{
public:
    EchoServer(EventLoop *loop,
              const InetAddress &listenAddr,
              const std::string &nameArg):
              server_(loop,listenAddr,nameArg),
              loop_(loop)
    {
        //设置回调函数
        server_.setConnectionCallback([this](const TcpConnectionPtr &conn){
            onConnection(conn);
        });
        server_.setMessageCallback([this](const TcpConnectionPtr &conn,
                                            Buffer* buf, 
                                            Timestamp time)
        {
            onMessage(conn,buf,time);
        });
        //设置线程数
        server_.setThreadNum(1);
    }

    void start(){
        server_.start();
    }
private:
    void onConnection(const TcpConnectionPtr &conn)
    {
        if(conn->connected()){
            LOG_INFO("connection UP :%s",conn->peerAddress().toIpPort().c_str());
        }
        else{
            LOG_INFO("connection DOWN :%s",conn->peerAddress().toIpPort().c_str());
        }
    }

    void onMessage(const TcpConnectionPtr &conn,
                   Buffer* buf, 
                   Timestamp time)
    {
        std::string msg=buf->retrieveAllAsString();
        conn->send(msg);
        //conn->shutdown();
    }

    EventLoop *loop_;
    TcpServer server_;

};
int main(){
    EventLoop loop;
    InetAddress addr(8000,"0.0.0.0");
    EchoServer server(&loop,addr,"server_1");
    server.start();
    loop.loop();

    return 0;
}
