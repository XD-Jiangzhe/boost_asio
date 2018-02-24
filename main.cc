//
// Created by root on 18-2-12.
//
#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <algorithm>
using namespace std;

void print(const boost::system::error_code&){
    std::cout<<"hello world"<<endl;
};

void callback(const boost::system::error_code&)
{
    cout<<"second timer"<<endl;
}
int main()
{

    using boost::asio::ip::tcp;
    boost::asio::io_service io;

		tcp::resolver resolver(io);

    boost::asio::deadline_timer t(io,boost::posix_time::seconds(3));
    boost::asio::deadline_timer t2(io,boost::posix_time::seconds(2));
    //t.wait();
    t.async_wait(print);
    t2.async_wait(callback);
    cout<<"hello world"<<endl;
    io.run();
    cout<<"finish"<<endl;
    return 0;
}
