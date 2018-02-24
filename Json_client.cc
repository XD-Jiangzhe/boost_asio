// #include "chat_message.hpp"
#include "struct_message.h"
#include "parse_chat_message.cc"
#include "JsonObject.h"

#include <boost/asio.hpp>

#include <sys/syscall.h>
#include <cstdlib>

#include <deque>
#include <iostream>
#include <thread>
#include <sstream>


using boost::asio::ip::tcp;
using chat_message_queue = std::deque<chat_message>;

namespace {
    using std::cout;
    using std::endl;
}

class chat_client{

    public:
        chat_client(boost::asio::io_service &io, tcp::resolver::iterator endpoint_iterator)
                :io_service_(io), socket_(io){
                    do_connect(endpoint_iterator);
                }
        void write(const chat_message& mesg)
        {
            io_service_.post([this, mesg](){
                bool write_in_progress = !write_mesgs.empty();
                write_mesgs.push_back(mesg);
                //这里防止两次调用do_write() 来写，压栈一次即可，除非是空栈回调调用完了，否则不会启动do_write(),因为它自己会调用自己一直写到空
                if(!write_in_progress)
                {
                    do_write();
                }
            }); 
        }
        void close()
        {
            io_service_.post([this](){socket_.close();});
        }

    private:
        void do_write()
        {
            boost::asio::async_write(socket_, boost::asio::buffer(write_mesgs.front().data(), write_mesgs.front().length())
                //写完了的回调函数，将写玩的那个mesg 弹出如果栈不为空则继续调用
                ,[this](boost::system::error_code err, std::size_t )
                {
                    if(!err)
                    {   
                        //std::cout.write(write_mesgs.front().body(), write_mesgs.front().body_length());
                        write_mesgs.pop_front();
                        if(!write_mesgs.empty())
                        do_write();
                    }
                    else{
                        socket_.close();
                    }
                });

        }
        void do_read_header()
        {
            boost::asio::async_read(socket_, boost::asio::buffer(read_mesg.data(), chat_message::header_length),
                [this](boost::system::error_code err, std::size_t )
                   {
                     if(!err && read_mesg.decode_header())
                     {
                      do_read_body();
                     }
                    else
                      socket_.close();
               });
        }
        void do_read_body()
        {
            boost::asio::async_read(socket_, boost::asio::buffer(read_mesg.body(), read_mesg.body_length()),
            [this](boost::system::error_code err, std::size_t){
            if(!err)
             {
                //  std::cout.write(read_mesg.body(), read_mesg.body_length());
                //  std::cout<<"\n";
                if(read_mesg.type() == MT_ROOM_INFO)
                {
                    auto buf = std::string(read_mesg.body(), read_mesg.body()+read_mesg.body_length());
                    std::cout<<"raw: "<<buf<<std::endl;
                    
                    std::stringstream ss(buf);
                    ptree tree;
                    boost::property_tree::read_json(ss, tree);
                    cout<<"client: "<<tree.get<std::string>("name")<<" says "<<tree.get<std::string>("infomation")<<endl;
                }
                 do_read_header();
             }else 
                socket_.close();
            });
        }
        void do_connect(tcp::resolver::iterator endpoint_iterator)
        {
            boost::asio::async_connect(socket_, endpoint_iterator
                ,[this](boost::system::error_code ec, tcp::resolver::iterator){
                    if(!ec)
                    {
                        std::cout<<"connect success\n";
                        do_read_header();
                    }
                });
        }
        boost::asio::io_service &io_service_;
        tcp::socket socket_;
        chat_message read_mesg;
        chat_message_queue write_mesgs;
        
};

int main(int argc, char* argv[])
{
    try{
        if(argc!= 3)
        {
            std::cerr<<"usage　chat_client usage\n";
            return 1;
        }

        boost::asio::io_service io_service;
        tcp::resolver resolver_(io_service);
        auto endpoint_iterator = resolver_.resolve({argv[1], argv[2]});
        chat_client c(io_service, endpoint_iterator);

        std::thread t([&](){io_service.run();
            std::cout<<syscall(186)<<std::endl;
        });
        char line[chat_message::max_body_length+ 1];
        while(std::cin.getline(line, chat_message::max_body_length+1))
        {
            chat_message mesg;

            auto type = 0;
            std::string input(line, line+std::strlen(line));
            std::string output;
            if(parseMessage3(input, &type, output))
            {
                mesg.setMessage(type, output.data(), output.size());
                c.write(mesg);  
                //std::cout<<"write message for server "<<output.size()<<std::endl;
            }          
        }

        c.close();
        t.join();

    }catch(std::exception &e)
    {
        std::cout<<e.what()<<std::endl;
    }
    return 0;
}