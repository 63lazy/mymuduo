#include "InetAddress.h"
#include <cstring>
#include <cstdio>
#include <iostream>

/// @brief 构造函数，根据端口号和IP地址初始化InetAddress对象
/// @param port 端口号，主机字节序
/// @param ip IP地址，字符串表示
InetAddress::InetAddress(uint16_t port, std::string ip) {
    std::memset(&addr_, 0, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

/// @brief 将IP地址转换为字符串表示
/// @return IP地址的字符串表示
std::string InetAddress::toIp() const {
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return std::string(buf);
}

/// @brief 将IP地址和端口号转换为字符串表示
/// @return IP地址和端口号的字符串表示
std::string InetAddress::toIpPort() const {
    char buf[INET_ADDRSTRLEN + 8];
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    size_t end = std::strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    std::snprintf(buf + end, sizeof(buf) - end, ":%u", port);
    return std::string(buf);
}

/// @brief 将端口号转换为主机字节序
/// @return 端口号，主机字节序
uint16_t InetAddress::toPort() const {
    return ntohs(addr_.sin_port);
}
