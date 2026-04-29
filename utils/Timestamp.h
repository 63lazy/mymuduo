#pragma once

#include <iostream>
#include<string>
#include<cstdint>
class Timestamp {
public:
    Timestamp();
    //explicit表明需要的是对象而不是隐式转换对象//
    explicit Timestamp(int64_t microseconds);
    static Timestamp now();
    std::string toString() const;
    int64_t microSecondsSinceEpoch() const { return microseconds_; }

    static Timestamp invalid(){return Timestamp(0);}
    // 物理逻辑：直接比较底层的微秒数值
    bool operator<(const Timestamp& rhs) const {
        return this->microseconds_ < rhs.microseconds_;
    }
private:
    int64_t microseconds_;
};

inline Timestamp addTime(Timestamp timestamp,double seconds){
    return Timestamp(timestamp.microSecondsSinceEpoch() + 
                     static_cast<int64_t>(seconds * 1000000));
}