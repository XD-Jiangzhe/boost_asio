#include <cstdlib>
#include <cstring>

#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

enum {max_length = 1024};

int main(int argc, char* argv[])
{
    try{
        if(argc != 3)
        {
            std::cerr<<"usage \n";
            return 1;
        }

    boost::asio::io_service io_service;
    tcp::socket so(io_service);
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(argv[1],argv[2]);
    boost::asio::connect(so, resolver.resolve(query));


    char request[max_length];
    while(std::cout<<"Enter Message: "&&(memset(request, 0, max_length)) &&std::cin.getline(request, max_length)){

        size_t request_len = strlen(request);
        boost::asio::write(so, boost::asio::buffer(request, request_len));

        char reply[max_length];
        size_t reply_size = boost::asio::read(so, boost::asio::buffer(reply, request_len));
        
        std::cout<<"reply is : ";
        std::cout.write(reply, request_len);
        std::cout<<"\n";
    }
    }catch(std::exception &e)
    {
        std::cout<<e.what()<<std::endl;
    }
    return 0;
}
