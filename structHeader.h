#ifndef FND_STRUCT_HEAD_H
#define FND_STRUCT_HEAD_H

#include <string>

struct Header{
    int bodySize;
    int type;
};

enum MessageType{

    MT_BIND_NAME = 1,   //{"name": "haha"}
    MT_CHAT_INFO = 2,   //{"infomation" : "hehe"}
    MT_ROOM_INFO = 3    //{"name": "haha". "infomation": "hehe"}
};
//client send
struct BindName
{
    char name[32];
    int nameLen;
};

//client send
struct ChatInformation
{
    char infomation[256];
    int infoLen;
};

//server send
struct Roominformation
{
    BindName name;
    ChatInformation chat;
};


bool parseMessage(const std::string& input, int* type, std::string& outbuffer);
bool parseMessage2(const std::string& input, int* type, std::string& outbuffer);
bool parseMessage3(const std::string& input, int* type, std::string& outbuffer);
bool parseMessage4(const std::string& input, int* type, std::string& outbuffer);

#endif