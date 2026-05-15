#include "Selector.h"
#include <mymuduo/EventLoop.h>
#include <mymuduo/logger.h>
std::atomic_int Selector::total_weight=0;
std::atomic_int Selector::failed_requests=0;
void Selector::addServer(const int &port,const std::string &IP,const int &weight){
    ServerNode node;
    InetAddress addr(port,IP);

    node.addr=std::move(addr);
    node.weight=weight;
    node.current_weight=0;
    node.is_alive=true;
    //待改进：写时复制而非加锁
    {
        std::unique_lock<std::mutex> lock(mutex_);
        serverGroup_.emplace_back(node);
    }
}   

InetAddress Selector::getNextServer(){
    std::unique_lock<std::mutex> lock(mutex_);
    //集群组中没有服务器
    if(serverGroup_.empty()) 
    {    
        LOG_ERROR("No available backend servers!");
        failed_requests++;
        return InetAddress(0,"0.0.0.0");
    }
    ServerNode* best = nullptr;
    int total = 0;
    for (auto& node : serverGroup_) {
        if (!node.is_alive) continue;
        
        node.current_weight += node.weight;
        total += node.weight;

        if (!best || node.current_weight > best->current_weight) {
            best = &node;
        }
    }
    if (best) {
        best->current_weight -= total;
        return best->addr;
    }
    //服务器全都挂掉了
    else{
        LOG_ERROR("No alived backend servers!");
        return InetAddress(0,"0.0.0.0");
    }
}