#include <ctime>

#include <iostream>
#include <string>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using std::cout;
std::string make_daytime_string(){
    using namespace std;
    time_t now = time(0);
    return ctime(&now);
}

int main()
{
    try{

        boost::asio::io_service io;
        tcp::acceptor acc(io, tcp::endpoint(tcp::v4(), 13));
        //INADDR_ANY 0.0.0.0 port 13
        //127.0.0.1 
        //root linux < 1024
        cout<<"start service"<<std::endl;
        for(;;)
        {
            tcp::socket socket(io);
            acc.accept(socket);
            //block
            auto end_point = socket.local_endpoint();
            cout<<"receive the connection from "<<end_point.address().to_string()<<std::endl;

            std::string message = make_daytime_string();

            boost::system::error_code err;
           // boost::asio::write(socket, boost::asio::buffer(message), err);
            socket.send(boost::asio::buffer(message), 0 ,err);
        }
    }catch(std::exception &e)
    {
        std::cout<<e.what()<<std::endl;
    }

    return 0;
}


