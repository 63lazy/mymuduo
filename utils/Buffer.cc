#include "Buffer.h"
#include <sys/uio.h>
const size_t Buffer::kCheapPrepend_=8;
const size_t Buffer::kInitiaSize_=1024;
ssize_t Buffer::readFd(int fd, int *saveError){
    //read读到的是流式数据
    //extrabuf用于防止多次扩容 先将长数据一起存储在栈上 之后如果要扩容再一起扩容
    char extrabuf[65536] = {0};//64k
    struct iovec vec[2];
    const size_t writable= writeableBytes();
    //先填入buffer缓冲区中
    vec[0].iov_base=begin()+writerIndex_;
    vec[0].iov_len=writable;
    //如果buffer空间不够就放入extrabuf等待
    vec[1].iov_base=extrabuf;
    vec[1].iov_len=sizeof(extrabuf);

    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const size_t n=::readv(fd,vec,iovcnt);
    if(n<0){
        *saveError=errno;
    }
    else if(n<=writable){ //buffer缓冲区足够
        writerIndex_+=n;
    }
    else{
        writerIndex_=buffer_.size();
        append(extrabuf,n - writable);
    }
    return n;
}