#include <boost/asio.hpp>

#include <memory>
#include <functional>
#include <iostream>
#include <string>

#include <ctime>

using  boost::asio::ip::tcp;

std::string  make_daytime_string()
{
    using namespace std;
    auto now = time(nullptr);
    return ctime(&now);
}

class tcp_connection : public std::enable_shared_from_this<tcp_connection>
{
    public:
        using pointer = std::shared_ptr<tcp_connection>;
        static pointer pointer_create(boost::asio::io_service &io_service)
        {
            //return  std::make_shared<tcp_connection>(io_service);
            //make_shared 会调用构造函数，但是构造函数是private 所以无法调用
            return pointer(new tcp_connection(io_service));
        }

        tcp::socket& socket(){return socket_;}

        void start()
        {
            message = make_daytime_string();
            auto self = shared_from_this();
            boost::asio::async_write(socket_, boost::asio::buffer(message),
                                    [self = std::move(self)](auto error, auto bytes_){self->handle_write(error, bytes_);});
        }

    private:
         tcp_connection(boost::asio::io_service &io): socket_(io){}

        std::string message;
        tcp::socket socket_;

        void handle_write(const boost::system::error_code& err, size_t bytes_)
        {

        }

};

class tcp_server{
    public:
        tcp_server(boost::asio::io_service &io): acceptor_(io, tcp::endpoint(tcp::v4(), 13))
        {
            start_accept();
        }
        
    private:
        tcp::acceptor acceptor_;
        void start_accept()
        {
            auto new_connection = tcp_connection::pointer_create(acceptor_.get_io_service());
            acceptor_.async_accept(new_connection->socket(), 
                        [=](const auto err){this->handle_accept(new_connection,err);});
        }
        void handle_accept(tcp_connection::pointer new_connection, boost::system::error_code err)
        {
            if(!err)
            {
                new_connection->start();
                std::cout<<"new conncetion from "<<new_connection->socket().local_endpoint().address().to_string()<<std::endl;
            }
            start_accept();
        }
};

int main()
{
    try{
        boost::asio::io_service io;
        tcp_server tcp_ser(io);
        io.run();

    }catch(std::exception &e)
    {
        std::cerr<<e.what()<<std::endl;
    }
    return 0;
}
