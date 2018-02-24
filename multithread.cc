#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <thread>
#include <functional>
#include<iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

class printer{
public:
    printer(boost::asio::io_service& io)
    :strand_(io), timer1_(io,boost::posix_time::seconds(1)),
    timer2_(io, boost::posix_time::seconds(1)), count(0)
    {
        timer1_.async_wait(strand_.wrap([this](const auto&){this->print1();}));
        timer2_.async_wait(strand_.wrap([this](const auto&){this->print2();}));
    }
    void print1()
    {
        if(count < 10)
        {
            std::cout<<"Timer 1: "<<count<<"  the tid is "<< syscall(186)<<std::endl;
            ++count;

            timer1_.expires_at(timer1_.expires_at()+boost::posix_time::seconds(1));
            timer1_.async_wait(strand_.wrap([this](const auto&){this->print1();}));
        }
    }
    void print2()
    {
        if(count < 10 )
        {
            std::cout<<"Timer 2: "<<count<<"  the tid is: "<<syscall(186)<<std::endl;
            ++count;

            timer2_.expires_at( timer2_.expires_at()+ boost::posix_time::seconds(1));
            timer2_.async_wait(strand_.wrap(
                // [this](const auto&){this->print2();}
                boost::bind(&printer::print2, this)
                ));
        }
    }
    ~printer(){std::cout<<"the fininsh count: "<<count<<std::endl;}

private:
    boost::asio::io_service::strand strand_;
    boost::asio::deadline_timer timer1_;
    boost::asio::deadline_timer timer2_;
    int count;
};

int main()
{
    boost::asio::io_service io;
    printer p(io);

    std::thread t([&]{io.run();});
    io.run();

    t.join();

    return 0;
}
