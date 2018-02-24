#include <cstdlib>

#include <deque>
#include <iostream>
#include <memory>
#include <set>
#include <list>
#include <utility>

#include <boost/asio.hpp>

#include "struct_message.h"
#include "parse_chat_message.cc"

using boost::asio::ip::tcp;

using chat_message_queue = std::deque<chat_message>;

class chat_participant
{
    public:
    virtual ~chat_participant(){}
    virtual void deliver(const chat_message&) =0;
};

using chat_participant_ptr = std::shared_ptr<chat_participant>;

//-------------------------------------------------------
class chat_room{
public:
    void join(chat_participant_ptr p)
    {
        chat_participants_.insert(p);
        std::for_each(recent_mesgs.begin(), recent_mesgs.end(), 
                [&](const chat_message& t ){p->deliver(t);});
    }

    void leave(chat_participant_ptr p)
    {
        chat_participants_.erase(p);
    }

    void deliver(chat_participant_ptr sender, const chat_message& msg)
    {
        recent_mesgs.push_back(msg);
        while(recent_mesgs.size()>max_message_num)
            recent_mesgs.pop_front();               
        //如果最近消息数大于最大的数量，就将顶部的多余的消息去掉
        std::for_each(chat_participants_.begin(), chat_participants_.end(),
                    [&](chat_participant_ptr p){
                        if(p != sender)
                            p->deliver(msg);
                        });
    }

private:
    enum{max_message_num = 100};
    std::set<chat_participant_ptr> chat_participants_;
    chat_message_queue recent_mesgs;
};

    using chat_room_wptr = std::weak_ptr<chat_room>;
    using chat_room_ptr = std::shared_ptr<chat_room>;

class session: public chat_participant, 
               public std::enable_shared_from_this<session>
{
    public:
        session(tcp::socket socket, chat_room &room):
                socket_(std::move(socket)), room_(room)
            {}
        void start()
        {
            {
                room_.join(shared_from_this());
                do_read_header();
            }
        }

        void deliver(const chat_message& mesg_)
        {   
            bool write_mesgs_empty = !write_mesgs.empty();
            write_mesgs.push_back(mesg_);
            if(!write_mesgs_empty)           //第一次肯定是空的，即刚刚创建
            {
                boost::system::error_code err;
               do_write();
            }
        }

        tcp::socket& get_socket()
        {
            return socket_;
        }

    private:
    //message 分为header 和　body 
    //header 固定的４个字节
        void do_read_header()
        {
            auto self(shared_from_this());
            boost::asio::async_read(socket_, 
            boost::asio::buffer(read_mesg.data(), chat_message::header_length),
            [=](boost::system::error_code ec, std::size_t)
            {
                if(!ec && read_mesg.decode_header())
                {
                    do_read_body();
                }
                else
                    room_.leave(self);
            });
        }
        void do_read_body()
        {
            auto self(shared_from_this());
            boost::asio::async_read(socket_, 
                boost::asio::buffer(read_mesg.body(), read_mesg.body_length())
                ,[=](boost::system::error_code ec, std::size_t)
                {

                    if(!ec)
                    {
                        //room_.deliver(read_mesg);
                        handle_message();
                        do_read_header();
                    } 
                    else{
                        room_.leave(self);
                    }
                });           
        }

        void handle_message()
        {
            if(read_mesg.type() ==MT_BIND_NAME)
            {
                //bind name
                const BindName* bind = (BindName*)(read_mesg.body());
                m_name.assign(bind->name, bind->name+bind->nameLen);
            }
            else if(read_mesg.type() == MT_CHAT_INFO)
            {
                //chat info
                const ChatInformation* info = (ChatInformation*)(read_mesg.body());
                m_chatinfomation.assign(info->infomation, info->infomation+info->infoLen);
                
                auto rinfo = buildinfomation();
                chat_message msg;
                msg.setMessage(MT_ROOM_INFO, &rinfo, sizeof(rinfo));
                room_.deliver(shared_from_this(), msg);
            }
            else{

            }

        }

        void do_write()
        {
            auto self(shared_from_this());
            boost::asio::async_write(socket_, boost::asio::buffer(write_mesgs.front().data(), write_mesgs.front().length()),
                [=](boost::system::error_code err, std::size_t){
                 //写完第一个数据，调用回调函数，来调用
                 if(!err)
                {
                    //将最前面的数据弹出
                     write_mesgs.pop_front();
                    if(!write_mesgs.empty())
                    {
                        //如果此时还有要写的东西，就继续写
                        do_write();
                    }
                }
                 else{
                    room_.leave(self);
                }
            });

        }
        chat_room &room_;
        tcp::socket socket_;
        chat_message read_mesg;
        chat_message_queue write_mesgs;
        std::string m_name;
        std::string m_chatinfomation;

        Roominformation buildinfomation()const{
            Roominformation info;
            info.name.nameLen = m_name.size();
            std::memcpy(info.name.name, m_name.data(), m_name.size());
            //set name 
            info.chat.infoLen = m_chatinfomation.size();
            std::memcpy(info.chat.infomation, m_chatinfomation.data(), m_chatinfomation.size());
            //set chat infomation
            return info;
        }
};
using session_ptr = std::shared_ptr<session>;
//------------------------------------------------------
class chat_server{
    public:
        chat_server(boost::asio::io_service &io, const tcp::endpoint &end_point):
                socket_(io), acceptor_(io, end_point)
                {
                    room = chat_room();
                   do_accept();
                }
    private:

        void do_accept() {
        acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
        if (!ec) {
            auto session_ =
                std::make_shared<session>(std::move(socket_), room);
            std::cout<<session_->get_socket().local_endpoint().address().to_string()<<std::endl;
            session_->start();
        }

        do_accept();
        });
  }

        tcp::socket socket_;
        tcp::acceptor acceptor_;
        chat_room       room;   
};
//------------------------------------------
using chat_server_ptr = std::shared_ptr<chat_server>;
using chat_server_list = std::list<chat_server>;


int main(int argc, char* argv[])
{
    try{
        if(argc <  2)
        {
            std::cerr<<"usage <host>\n";
            return 1;
        }
        boost::asio::io_service io_service_;
        chat_server_list servers;
        for(int i=1; i< argc; ++i)
        {
            tcp::endpoint endpoint_(tcp::v4(), std::atoi(argv[i]));
            servers.emplace_back(io_service_, endpoint_);
        }
        io_service_.run();

    }catch(std::exception &e)
    {
        std::cout<<e.what()<<std::endl;
    }
    return 0;
}


