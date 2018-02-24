#include "struct_message.h"
#include "parse_chat_message.cc"
#include "JsonObject.h"

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>

#include <deque>
#include <iostream>
#include <memory>
#include <set>
#include <list>
#include <utility>
#include <vector>
#include <thread>
#include <chrono>
#include <functional>

#include <cstdlib>

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
    chat_room(boost::asio::io_service& io):strand_(io){}
    void join(chat_participant_ptr p)
    {
        strand_.post([this, p](){
             chat_participants_.insert(p);
             std::for_each(recent_mesgs.begin(), recent_mesgs.end(), 
             [&](const chat_message& t ){p->deliver(t);});
        });
    }

    void leave(std::string name, chat_participant_ptr p)
    {
        strand_.post([this, p, name](){
            std::cout<<"client: "<<name<<"  leave the chat room"<<std::endl;
            chat_participants_.erase(p);
        });
    }

    void deliver(chat_participant_ptr sender, const chat_message& msg)
    {
        strand_.post([msg, this, sender](){
            recent_mesgs.push_back(msg);
            while(recent_mesgs.size()>max_message_num)
            recent_mesgs.pop_front();               
        //如果最近消息数大于最大的数量，就将顶部的多余的消息去掉
            std::for_each(chat_participants_.begin(), chat_participants_.end(),
                        [&](chat_participant_ptr p){
                            if(p != sender)
                                p->deliver(msg);
                            });
        });
    }

private:
    enum{max_message_num = 100};
    std::set<chat_participant_ptr> chat_participants_;
    chat_message_queue recent_mesgs;
    boost::asio::io_service::strand strand_;
};

    using chat_room_wptr = std::weak_ptr<chat_room>;
    using chat_room_ptr = std::shared_ptr<chat_room>;

class session: public chat_participant, 
               public std::enable_shared_from_this<session>
{
    public:
        session(tcp::socket socket, chat_room &room):
                socket_(std::move(socket)), room_(room),s_strand(socket.get_io_service())
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
            s_strand.post([this, mesg_](){
            bool write_mesgs_empty = !write_mesgs.empty();
            write_mesgs.push_back(mesg_);
            if(!write_mesgs_empty)           //第一次肯定是空的，即刚刚创建
            {
                boost::system::error_code err;
               do_write();
            }

            });

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
            s_strand.wrap([this, self](boost::system::error_code ec, std::size_t)
            {
                if(!ec && read_mesg.decode_header())
                {
                    do_read_body();
                }
                else
                    room_.leave(m_name, self);
            }));
        }
        void do_read_body()
        {
            auto self(shared_from_this());
            boost::asio::async_read(socket_, 
                boost::asio::buffer(read_mesg.body(), read_mesg.body_length())
                ,s_strand.wrap([this, self](boost::system::error_code ec, std::size_t)
                {

                    if(!ec)
                    {
                        //room_.deliver(read_mesg);
                        handle_message();
                        do_read_header();
                    } 
                    else{
                        room_.leave(m_name, self);
                    }
                }));           
        }
        template <typename T>
        //模板类用来获取 已经序列化的类的内容
        T toObject(){
            T obj;
            std::stringstream ss(std::string(read_mesg.body(), read_mesg.body()+read_mesg.body_length()));
            boost::archive::text_iarchive ti(ss);
            ti & obj;  
            return  obj;         
        }
        bool fillProtobuf(::google::protobuf::Message* mesg)
        {
            std::string ss(read_mesg.body(), read_mesg.body()+read_mesg.body_length());
            auto ok = mesg->ParseFromString(ss);
            return ok;
        }

        void handle_message()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::cout<<"sleep for"<<" 10 milliseconds\n";
            if(read_mesg.type() ==MT_BIND_NAME)
            {
                // ptree tree = toPtree();
                // m_name = tree.get<std::string>("name");
                PBindName bindname;
                if(fillProtobuf(&bindname))
                {
                    m_name = bindname.name();
                }
            }
            else if(read_mesg.type() == MT_CHAT_INFO)
            {

                // ptree tree = toPtree();
                // m_chatinfomation = tree.get<std::string>("infomation");
                PChat chat;
                if(fillProtobuf(&chat))
                {
                    m_chatinfomation = chat.information();
                }
                else return;

                std::string rinfo = buildRoomInfo();

                chat_message msg;
                msg.setMessage(MT_ROOM_INFO, rinfo);

                room_.deliver(shared_from_this(), msg);
                
            }
            else{

            }

        }
        
        void do_write()
        {
            auto self(shared_from_this());
            boost::asio::async_write(socket_, boost::asio::buffer(write_mesgs.front().data(), write_mesgs.front().length()),
                s_strand.wrap([=](boost::system::error_code err, std::size_t){
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
                    room_.leave(m_name, self);
                }
            }));

        }
        chat_room &room_;
        tcp::socket socket_;
        chat_message read_mesg;
        chat_message_queue write_mesgs;
        std::string m_name;
        std::string m_chatinfomation;
        boost::asio::io_service::strand s_strand;

        std::string buildRoomInfo()const{
            PRoomInformation roominfo;
            roominfo.set_name(m_name);
            roominfo.set_information(m_chatinfomation);
            std::string out;
            auto ok = roominfo.SerializeToString(&out);
            assert(ok);
            return out;
        }

};
using session_ptr = std::shared_ptr<session>;
//------------------------------------------------------
class chat_server{
    public:
        chat_server(boost::asio::io_service &io, const tcp::endpoint &end_point):
                socket_(io), acceptor_(io, end_point), room(io), s_strand(io)
                {
                   do_accept();
                }
    private:

        void do_accept() {
        acceptor_.async_accept(socket_, s_strand.wrap([this](boost::system::error_code ec) {
        if (!ec) {
            auto session_ =
                std::make_shared<session>(std::move(socket_), room);
            std::cout<<session_->get_socket().local_endpoint().address().to_string()<<std::endl;
            session_->start();
        }

        do_accept();
        }));
  }

        tcp::socket socket_;
        tcp::acceptor acceptor_;
        chat_room       room;   
        boost::asio::io_service::strand s_strand;
};
//------------------------------------------
using chat_server_ptr = std::shared_ptr<chat_server>;
using chat_server_list = std::list<chat_server>;

void init()
{
    boost::log::add_file_log("log.log", boost::log::keywords::auto_flush = true);
    boost::log::core::get()->set_filter
    (
        boost::log::trivial::severity >= boost::log::trivial::info
    );
}

static std::function<void()> shandler;
void signalhandler(int sig)
{
    BOOST_LOG_TRIVIAL(info)<<"handle system signal "<< sig;
    if(shandler)
    {
        shandler();
        shandler = nullptr;
    }
}

int main(int argc, char* argv[])
{
    try{
        init();
		BOOST_LOG_TRIVIAL(trace) << "A trace severity message"<< "the line is "<<__LINE__;
		BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
		BOOST_LOG_TRIVIAL(info) << "An informational severity message";
		BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
		BOOST_LOG_TRIVIAL(error) << "An error severity message";
		BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";

        GOOGLE_PROTOBUF_VERIFY_VERSION;
        if(argc <  2)
        {
            std::cerr<<"usage <host>\n";
            return 1;
        }
        boost::asio::io_service io_service_;
        chat_server_list servers;
        
        shandler = [&io_service_](){io_service_.stop();};
        ::signal(SIGINT, signalhandler);
        
        for(int i=1; i< argc; ++i)
        {
            tcp::endpoint endpoint_(tcp::v4(), std::atoi(argv[i]));
            servers.emplace_back(io_service_, endpoint_);
        }

        std::vector<std::thread> threadgroups;
        for(int i=0; i<5; ++i)
        {
            threadgroups.emplace_back([&io_service_, i](){
                std::cout<<"thread num "<<i<<" thread id: "<<std::this_thread::get_id()<<std::endl;
                io_service_.run();
            });
        }

        io_service_.run();
        for(auto& v: threadgroups)
            v.join();
        

        google::protobuf::ShutdownProtobufLibrary();

    }catch(std::exception &e)
    {
        std::cout<<e.what()<<std::endl;
    }
    return 0;
}


