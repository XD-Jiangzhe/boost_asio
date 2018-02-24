#ifndef FND_SERI_H
#define FND_SERI_H

#include <boost/archive/text_oarchive.hpp>//output archive 两个归档头文件
#include <boost/archive/text_iarchive.hpp>//input archive
//这是将文本流进行转储的两个序列化头文件

class SBindName
{
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & m_bindName;        //这里重载了 & ，使用& 执行转储和恢复的操作
    }
    std::string m_bindName;

    public:
        SBindName(std::string name):m_bindName(name){}
        SBindName(){}

        const std::string& bindName()const {return m_bindName;}
};

class SChatInfo
{
    friend class boost::serialization::access;
    
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & m_chatinfomation;        //这里重载了 & ，使用& 执行转储和恢复的操作
    }    
    std::string m_chatinfomation;

    public:
        SChatInfo(std::string info):m_chatinfomation(info){}
        SChatInfo(){}
        const std::string& chatInformation()const{return m_chatinfomation;}
};

class SRoomInfo
{
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive &ar, const unsigned int version)
        {
            ar & m_bind;        //这里重载了 & ，使用& 执行转储和恢复的操作
            ar & m_chat;
        }    

        SBindName m_bind;
        SChatInfo m_chat;

    public:
        SRoomInfo(){}
        SRoomInfo(std::string name, std::string info):m_bind(name), m_chat(info){}
        const std::string& name()const{return m_bind.bindName();}
        const std::string& information()const{return m_chat.chatInformation();}
        
};





#endif