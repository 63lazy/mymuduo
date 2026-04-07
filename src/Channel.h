#pragma once
#include "../utils/noncopy.h"
#include<functional>
#include"../utils/Timestamp.h"
#include<memory>
class EventLoop;

class Channel:NonCopyable{
public:
    using EventCallback=std::function<void()>;
    using ReadEventCallback=std::function<void(Timestamp)>;

    Channel(EventLoop* loop,int fd);
    //fd得到poller通知通过调用相应的回调函数处理事件//
    void handleEvent(Timestamp receiveTime);
    //设置回调函数//
    void setReadCallback(ReadEventCallback cb){readCallback_=std::move(cb);}
    void setWriteCallback(EventCallback cb){writeCallback_=std::move(cb);}
    void setCloseCallback(EventCallback cb){closeCallback_=std::move(cb);}
    void setErrorCallback(EventCallback cb){errorCallback_=std::move(cb);}

    void tie(const std::shared_ptr<void>&);
    int fd() const{return fd_;}
    int events() const{return events_;}
    void set_revents(int revt){revents_=revt;}

    //设置fd的事件状态//
    void enableReading(){events_|=kReadEvent;update();}
    void disableReading(){events_&=~kReadEvent;update();}
    void enableWriting(){events_|=kWriteEvent;update();}
    void disableWriting(){events_&=~kWriteEvent;update();}
    void disableAll(){events_=kNoneEvent;update();}

    //返回fd的事件状态//
    bool IsReadEnabled() const {return events_&kReadEvent;}
    bool IsWriteEnabled() const {return events_&kWriteEvent;}
    bool IsNoneEvent() const {return events_==kNoneEvent;}

    int index(){return index_;}
    void set_index(int idx){index_=idx;}

    EventLoop* ownerLoop(){return loop_;}
    void remove();
private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);  

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;
    EventLoop* loop_;//事件循环//
    const int fd_; //poller监听的对象
    int events_;//将fd感兴趣的事件加入监控 即作为 epoll_ctl 的输入参数
    int revents_;//poller返回的事件 即这个channel实际发生的事件
    int index_;//poller返回的事件索引

    std::weak_ptr<void> tie_;
    bool tied_;
    //由channel调用具体的回调操作，因为channel知道具体发生的事件//
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};
