#include<iostream>
#include<cstdint>
#include<vector>   
#include<string>
#include<sstream>
#include<cstring>
#include "noncopy.h"
#include "Buffer.h"

#pragma pack(push,1)
typedef struct MessageHeader
{
    uint32_t length;
    uint16_t type;
} MessageHeader;
#pragma pack(pop)

typedef struct ChatMessage
{
    MessageHeader header;
    std::vector<char> data;
} ChatMessage;


class CommandParser :  NonCopyable
{
private:
public:
    CommandParser() = default;

    //对用户输入进行处理
    ChatMessage CommandParse(const std::string input){
        MessageHeader header;
        ChatMessage msg;
        std::string buffer;
        // 解析输入字符串并填充msg结构体
        std::istringstream iss(input);
        iss >> header.type;
        std::getline(iss, buffer);
        if (!buffer.empty()) {
            buffer.erase(0, 1); // 去掉开头的空格
        }
        header.length = buffer.size();
        msg.header = std::move(header);
        msg.data.assign(buffer.begin(), buffer.end());
        return msg;
    } 
};


class ChatParser{
public:
    std::vector<char> serialize(std::string input){
        CommandParser cmd_parser;
        ChatMessage msg = cmd_parser.CommandParse(input);
        std::vector<char> buffer;
        buffer.reserve(sizeof(MessageHeader) + msg.data.size());

        //序列化数据//
        const char *header_ptr = reinterpret_cast<const char*>(&msg.header);
        buffer.insert(buffer.end(), header_ptr, header_ptr + sizeof(MessageHeader));

        buffer.insert(buffer.end(), msg.data.begin(), msg.data.end());

        return buffer;
    };

    //反序列化数据//
    bool deserialize(std::vector<char>& rcv_buffer){
        MessageHeader header;
        if(rcv_buffer.size() < sizeof(MessageHeader)){
            return false;
        }

        //提取消息头//
        std::memcpy(&header, rcv_buffer.data(), sizeof(MessageHeader));
        //如果消息体长度大于缓冲区剩余长度，说明数据不完整，需要继续接收//
        if(header.length > rcv_buffer.size() - sizeof(MessageHeader)){
            return false;
        }
        //如果消息完整直接跳出循环//
        return true;
        
    }
};
