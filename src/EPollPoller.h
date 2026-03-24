#include<vector>
#include "Poller.h"
#include <sys/epoll.h>
#include "../utils/Timestamp.h"

class Channel;
class EPollPoller : public Poller{
public:
    EPollPoller(EventLoop* loop);
    ~EPollPoller() override;

    Timestamp poll(int timeoutMs,ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;
private:
    //初始化EventList的长度
    static const int kInitEventListSize=16;
    //填写活跃的连接
    void fillActiveChannels(int numEvents,ChannelList* activeChannels) const;
    //更新Channel
    void update(int operation,Channel* channel);
    using EventList=std::vector<epoll_event>;
    int epollfd_;
    EventList events_;
};