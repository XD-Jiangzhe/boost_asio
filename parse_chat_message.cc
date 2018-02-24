#ifndef PARSE_CHAT_MESSAGE
#define PARSE_CHAT_MESSAGE

#include "structHeader.h"
#include "serializationObject.h"
#include "JsonObject.h"
#include "Protocal.pro.pb.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
//形如 cmd messagebody 
inline bool parseMessage(const std::string& input, int* type, std::string& outbuffer)
{
    auto pos = input.find_first_of(" ");
    if(pos == std::string::npos)
    {   
        return false;
    }
    if(pos == 0)
        return false;

    auto command = input.substr(0, pos);
    if(command == "BindName")
    {
        std::string name = input.substr(pos+1);
        if(name.size()> 32)
            return false;
        if(type)
            *type = MT_BIND_NAME;
        BindName bindinfo;
        bindinfo.nameLen = name.size();
        std::memcpy(&(bindinfo.name), name.data(), name.size());
        auto buffer = (char*)(&bindinfo);
        outbuffer.assign(buffer, buffer+sizeof(bindinfo));
        return true;
    }
    else if(command == "Chat")
    {
        std::string chat = input.substr(pos+1);
        if(chat.size()> 256)
            return false;
        ChatInformation info;
        info.infoLen = chat.size();
        std::memcpy(&(info.infomation), chat.data(), chat.size());
        auto buffer = (char*)(&info);
        outbuffer.assign(buffer, buffer+sizeof(info));
        if(type)
            *type = MT_CHAT_INFO;
        return true;
    }
    else 
        return false;
}

template <typename T> 
std::string seriliaze(const T& obj)
{
    std::stringstream ss;
    boost::archive::text_oarchive to(ss);
    to & obj;               //obj中的东西序列化到了 ss中
    return ss.str();        //变成 string
}

inline bool parseMessage2(const std::string& input, int* type, std::string& outbuffer)
{
    auto pos = input.find_first_of(" ");
    if(pos == std::string::npos)
    {   
        return false;
    }
    if(pos == 0)
        return false;

    auto command = input.substr(0, pos);
    if(command == "BindName")
    {
        std::string name = input.substr(pos+1);
        if(name.size()> 32)
            return false;
        if(type)
            *type = MT_BIND_NAME;
        // BindName bindinfo;
        // bindinfo.nameLen = name.size();
        // std::memcpy(&(bindinfo.name), name.data(), name.size());
        // auto buffer = (char*)(&bindinfo);
        // outbuffer.assign(buffer, buffer+sizeof(bindinfo));
        outbuffer = seriliaze(SBindName(std::move(name)));        
        return true;
    }
    else if(command == "Chat")
    {
        std::string chat = input.substr(pos+1);
        if(chat.size()> 256)
            return false;
        // ChatInformation info;
        // info.infoLen = chat.size();
        // std::memcpy(&(info.infomation), chat.data(), chat.size());
        // auto buffer = (char*)(&info);
        // outbuffer.assign(buffer, buffer+sizeof(info));
        outbuffer = seriliaze(SChatInfo(std::move(chat)));
        if(type)
            *type = MT_CHAT_INFO;
        return true;
    }
    else 
        return false;
}

inline bool parseMessage3(const std::string& input, int* type, std::string& outbuffer)
{
auto pos = input.find_first_of(" ");
    if(pos == std::string::npos)
    {   
        return false;
    }
    if(pos == 0)
        return false;

    auto command = input.substr(0, pos);
    if(command == "BindName")
    {
        std::string name = input.substr(pos+1);
        if(name.size()> 32)
            return false;
        if(type)
            *type = MT_BIND_NAME;

        ptree ptree_;
        ptree_.put("name", name);
        outbuffer = ptreeToJsonString(ptree_);

        return true;
    }
    else if(command == "Chat")
    {
        std::string chat = input.substr(pos+1);
        if(chat.size()> 256)
            return false;

        ptree tree;
        tree.put("infomation", chat);
        outbuffer = ptreeToJsonString(tree);
        if(type)
            *type = MT_CHAT_INFO;
        return true;
    }
    else 
        return false;
}

inline bool parseMessage4(const std::string& input, int* type, std::string& outbuffer)
{
auto pos = input.find_first_of(" ");
    if(pos == std::string::npos)
    {   
        return false;
    }
    if(pos == 0)
        return false;

    auto command = input.substr(0, pos);
    if(command == "BindName")
    {
        std::string name = input.substr(pos+1);
        if(name.size()> 32)
            return false;
        if(type)
            *type = MT_BIND_NAME;

        PBindName bindname;
        bindname.set_name(name);
        auto re= bindname.SerializeToString(&outbuffer);
        return re;
    }
    else if(command == "Chat")
    {
        std::string chat = input.substr(pos+1);
        if(chat.size()> 256)
            return false;

        PChat pchat;
        pchat.set_information(chat);
        auto re = pchat.SerializeToString(&outbuffer);
        if(type)
            *type = MT_CHAT_INFO;           
        return re;
    }
    else 
        return false;
}

#endif