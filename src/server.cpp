#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <queue>
#include <thread>
#include <mutex>
#include <map>
#include <functional>
#include<condition_variable>
#include<sys/epoll.h>
#include<string>
#include<sstream>
#include "protocol.h"
using namespace std;

//用来记录接收的客户端数量//
const int port=8080;

class threadpool{
private:
    //工作线程队列//
    vector<thread> workers;
    //任务队列(函数指针版)//
    queue<function<void()>> tasks;
    //线程锁//
    mutex mtx;
    bool stop;
    //条件变量//
    condition_variable cv;
    
public:
    threadpool(int nums);
    ~threadpool();
    template<typename F,typename ...Args>
    void enqueue(F &&f,Args && ...args);
};

threadpool::threadpool(int nums):stop(false){
   for (int i = 0; i < nums; i++)
   {
        workers.emplace_back([this](){
            while (1)
            {
                function<void()> task;
                {
                    unique_lock<mutex> lock(mtx);
                    //wait函数可以释放锁让enqueue函数添加任务，为了避免死锁只能设置一个mutex
                    cv.wait(lock,[this](){
                        return !tasks.empty()||stop;
                    });
                    if (stop)
                    {
                        return;
                    }
                    
                task=(move(tasks.front()));
                tasks.pop();
            }
            task();
            }
        });
   }
}

threadpool::~threadpool()
{
    {
        unique_lock<mutex> lock(mtx);
        stop=true;
    }
    cv.notify_all();
    for(auto &worker:workers){
        worker.join();
    }
}
template<typename F,typename ...Args>
void threadpool::enqueue(F &&f,Args && ...args){
    auto task=[func=forward<F>(f),...args_func=forward<Args>(args)]()mutable{
        func(args_func...);
    };
    {
        unique_lock<mutex> lock(mtx);
        tasks.emplace(move(task));
        cv.notify_one();
    }

}


class reactor{
private:
    //命令解析器//
    ChatParser parser;
    //接收缓冲区//
    map<int,vector<char>> rcv_buffer_map;
    //数据处理锁//
    map<int,mutex> rcv_buffer_lock_map;
    //记录命令编号//
    map<char,function<void(string)>>server_func_map;
    map<uint16_t,function<void(ChatMessage)>>client_func_map;
    //线程池//
    threadpool pool;
    epoll_event events[1024];
    int epfd;
    int serverfd;
    map<int,sockaddr_in>client_map;
    mutex map_lock;
    //专门处理某个客户端fd的拆包逻辑//
    void process_client_buffer(int readyfd){
        auto &buf = rcv_buffer_map[readyfd];

        //只要当前缓冲区中还有完整的数据包就一直处理//
        while (parser.deserialize(buf)) {
            ChatMessage msg;
            MessageHeader header;

            //从缓冲区头部提取消息头//
            std::memcpy(&header, buf.data(), sizeof(MessageHeader));
            msg.header = header;

            //提取消息体//
            msg.data.assign(buf.begin() + sizeof(MessageHeader),
                            buf.begin() + sizeof(MessageHeader) + header.length);

            //从缓冲区中移除已经处理过的完整消息//
            buf.erase(buf.begin(),
                      buf.begin() + sizeof(MessageHeader) + header.length);

            //后续可以在这里根据 msg.header.type 做进一步分发//
            //例如：pool.enqueue(msg_handlers[msg.header.type], readyfd, msg);
            auto it = client_func_map.find(header.type);
            if (it != client_func_map.end()) {
                this->pool.enqueue(it->second, msg);
            } else {
                cout << "未知消息类型: " << header.type << endl;
            }
        }

        //处理完当前缓冲区数据后释放锁//
        rcv_buffer_lock_map[readyfd].unlock();
    }
    void handle_stdin(){
        string input;
        
        getline(cin,input);
        //识别命令//
        istringstream iss(input);
        ostringstream oss;
        char tag;
        string args;
        iss>>tag;
        getline(iss,args);
        if (!args.empty()){
            args.erase(0,1);
        }

        // 检查命令是否存在
        auto it = server_func_map.find(tag);
        if (it != server_func_map.end()) {
            pool.enqueue(it->second, args);
        } else {
            cout << "未知命令: " << tag << endl;
        }
    }
    void handle_io(int readyfd){
        char buffer[1024];
        int bytes=recv(readyfd,buffer,1024,0);
        //输入小于0说明断开连接//
        if (bytes==0)
        {
            //注意客户端正确exit(0)的时候虽然无应用层数据，但TCP会发送FIN报文，服务端recv会返回0//
            epoll_ctl(epfd,EPOLL_CTL_DEL,readyfd,nullptr);
            {
                unique_lock<mutex> lock(map_lock);
                
                char ip[20];
                inet_ntop(AF_INET,&client_map[readyfd].sin_addr,ip,INET_ADDRSTRLEN);
                cout<<"客户端"<<ip<<"断开连接"<<endl;
                cout<<"客户端端口号"<<ntohs(client_map[readyfd].sin_port)<<endl;
                client_map.erase(readyfd);
            
            }
            close(readyfd);
            return;
        }
        else if (bytes<0)
        {
            cerr<<"接收数据失败"<<endl;
            return;
        }
        //将接收到的数据添加到缓冲区//
        rcv_buffer_map[readyfd].insert(rcv_buffer_map[readyfd].end(),buffer,buffer+bytes);
        //尝试获取数据处理锁//
        if(rcv_buffer_lock_map[readyfd].try_lock()){
            //把拆包逻辑丢到线程池中异步处理//
            pool.enqueue([this, readyfd](){
                this->process_client_buffer(readyfd);
            });
        }
        //若获取锁失败，说明有其他线程正在处理数据//
        else{
            return;
        }
    }
    void handle_accept(){
        sockaddr_in client_addr;
        socklen_t socklen=sizeof(client_addr);
        int client_fd=accept(serverfd,(sockaddr *)&client_addr,&socklen);

        epoll_event ev;
        ev.data.fd=client_fd;
        ev.events=EPOLLIN;
        epoll_ctl(epfd,EPOLL_CTL_ADD,client_fd,&ev);

        {
            unique_lock<mutex> lock(map_lock);
            client_map[client_fd]=client_addr;
        }
                    
        if (client_fd<0) {
            cerr<<"连接失败"<<endl;
            return;
        }
        char ip[20];
        cout<<"客户端"<<inet_ntop(AF_INET,&client_addr.sin_addr,ip,INET_ADDRSTRLEN)<<"已连接"<<endl;
        cout<<"客户端端口"<<ntohs(client_addr.sin_port)<<endl;
    }
public:
    //初始化客户端命令集的数字对应函数//
    void init_clientmap(){    
        //简单回显消息//
        client_func_map[1]=[this](ChatMessage msg){
            string a(msg.data.data(),msg.data.size());
            cout<<a<<endl;
        };
    }   
    //初始化服务端命令集的数字对应函数//
    void init_servermap(){    
        //广播消息给所有客户端//
        server_func_map['1']=[this](string a){
            string message = a + "\n";  // 添加换行符
            const char *buffer = message.c_str();
            for (auto it:client_map)
                send(it.first, buffer, message.length(), 0);
            
        };
    } 
    
   
    reactor(int fd):pool(10){
        init_clientmap();
        init_servermap();
        epfd=epoll_create(10);
        serverfd=fd;
        //初始化时直接将服务端fd加入epoll监控//
        epoll_event ev;
        ev.data.fd=serverfd;
        ev.events=EPOLLIN;
        epoll_ctl(epfd,EPOLL_CTL_ADD,serverfd,&ev);
        //将服务端输入加入epoll监控//
        ev.data.fd=STDIN_FILENO;
        ev.events=EPOLLIN;
        epoll_ctl(epfd,EPOLL_CTL_ADD,STDIN_FILENO,&ev);
    };
    void run(){
        //同时处理连接和io请求//
        while (1)
        {
            int nfds=epoll_wait(epfd,events,1024,10000);
            for (int i = 0; i < nfds; i++)
            {
                int readyfd=events[i].data.fd;
                //服务端fd就绪说明有新连接在申请//
                if (readyfd==serverfd)
                    handle_accept();
                //处理输入//
                else if (readyfd==STDIN_FILENO){
                    handle_stdin();
                }
                //处理io//
                else{
                    handle_io(readyfd);
                }
            }
        }
    }
};


int main(){
    int sockfd;
    sockaddr_in server_addr;
    //创建socket//
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if (sockfd<0)
    {
        cerr<<"无法创建socket"<<endl;
        return 1;
    }
    //设置socket选项//
    int opt=1;
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt)))
    {
        cerr<<"设置socket选项失败"<<endl;
        close(sockfd);
        return 1;
    }
    
    //绑定地址和端口//
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(port);
    server_addr.sin_addr.s_addr=INADDR_ANY;
    if (bind(sockfd,(sockaddr *)&server_addr,sizeof(server_addr)))
    {
        cerr<<"绑定失败"<<endl;
        close(sockfd);
        return 1;
    }

    //开始监听//
    if (listen(sockfd,128))
    {
        cerr<<"监听失败"<<endl;
        close(sockfd);
        return 1;
    }
    cout<<"服务端启动成功,等待客户端连接"<<endl;
    cout<<"监听端口："<<port<<endl;

    reactor reactor_main(sockfd);  
    reactor_main.run();
    //清理资源//
    close(sockfd);
    cout<<"服务端已关闭"<<endl;
    return 0;
}
