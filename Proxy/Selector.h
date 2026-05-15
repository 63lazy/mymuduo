#pragma once
#include <vector>
#include <mymuduo/Callbacks.h>
#include <mymuduo/InetAddress.h>
#include <mymuduo/EventLoop.h>
#include <mutex>
#include <atomic>
struct ServerNode{
    InetAddress addr;
    int weight;
    int current_weight;
    bool is_alive;

};
class Selector{
public:
    Selector(EventLoop *loop):loop_(loop){}
    void addServer(const int &port,const std::string &IP,const int &weight);
    InetAddress getNextServer();
private:
    std::vector<ServerNode> serverGroup_;
    EventLoop *loop_;
    
    std::mutex mutex_;
    static std::atomic_int total_weight;
    static std::atomic_int failed_requests;

};