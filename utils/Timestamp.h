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
private:
    int64_t microseconds_;
};