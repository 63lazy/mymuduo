#pragma once
#include "../utils/noncopy.h"
#include <vector>
#include <unordered_map>
#include "../utils/Timestamp.h"
class Channel;
class EventLoop;

class Poller : NonCopyable{
public:
    using ChannelList=std::vector<Channel*>;
    Poller(EventLoop* loop);
    //虚析构函数//
    virtual ~Poller()=default;

    //统一接口//
    virtual void updateChannel(Channel* channel)=0;
    virtual void removeChannel(Channel* channel)=0;
    virtual bool hasChannel(Channel* channel)const;

    //EventLoop通过该接口获取IO复用的具体实现 
    static Poller* newDefaultPoller(EventLoop* loop);

    virtual Timestamp poll(int timeoutMs,ChannelList* activeChannels)=0;
protected:
    using ChannelMap=std::unordered_map<int,Channel*>;
    ChannelMap channels_;
private:
    EventLoop* ownerLoop_;
};