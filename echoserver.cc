#include <cstdlib>

#include <iostream>
#include <memory>
#include <utility>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

const int max_length = 4096;

class session: public std::enable_shared_from_this<session>
{
    public:
        using poiter = std::shared_ptr<session>;
        session(tcp::socket sock):socket_(std::move(sock)){}
        void start(){
            do_read();
        }
    private:
        void do_read(){
            auto self(shared_from_this());
            socket_.async_read_some(
                boost::asio::buffer(data, max_length), 
                [self]( boost::system::error_code err , std::size_t length){
                    if(!err)
                    {
                        self->do_write(length);
                        //std::cout<<"count num: "<<self.use_count()<<std::endl;
                    }
                    //std::cout<<"count num: "<<self.use_count()<<std::endl;
            });
        }
        void do_write(std::size_t length_)
        {
            auto self(shared_from_this());
            boost::asio::async_write(socket_,
                boost::asio::buffer(data, length_),
                [self](boost::system::error_code err, std::size_t length){
                    if(!err)
                    {
                        self->do_read();
                    }
                });
        }
        char data[max_length];
        tcp::socket socket_;

};

class server
{
    public:
        server(boost::asio::io_service &io_service, short port)
            :acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
            socket_(io_service){}
                                                
        void start(){do_accept();}
    private:
        void do_accept()
        {
            acceptor_.async_accept(socket_,[this](boost::system::error_code err){
                if(!err)
                {
                    std::make_shared<session>(std::move(socket_))->start();
                }
                do_accept();
            } );
        }

        tcp::acceptor acceptor_;
        tcp::socket socket_;
};

int main()
{
    try{
        boost::asio::io_service io_service;
        server server_(io_service, 2333);
        server_.start();
        io_service.run();

    }catch(std::exception& e)
    {
        std::cerr<<"execption: "<<e.what()<<std::endl; 
    }
}

