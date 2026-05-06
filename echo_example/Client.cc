#include <mymuduo/TcpClient.h>
#include <mymuduo/EventLoop.h>

class echo_Client{
public:
    echo_Client(EventLoop *loop,InetAddress &serverAddr,std::string &name):
                tc_(loop,serverAddr,name),
                loop_(loop)
    {
        
        
    }
private:
    void onConnection(){
        LOG_INFO("connection UP :");
    
    }

    TcpClient tc_;
    EventLoop *loop_;
    
}

int main(){
    EventLoop *loop;

}