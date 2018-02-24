//
// Created by root on 18-2-13.
//
#include <boost/asio.hpp>
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>

class printer
{
public:
    printer(boost::asio::io_service &io)
            :timer_(io,boost::posix_time::seconds(1)), count(0){
        //timer_.async_wait(std::bind(&printer::print, this));
        timer_.async_wait([=](const auto& err){this->print();});
    }
    void print() {
        if (count < 5) {
            std::cout << count << std::endl;
            ++count;
            timer_.expires_at(timer_.expires_at() + boost::posix_time::seconds(1));
            //timer_.async_wait(std::bind(&printer::print, this));
            timer_.async_wait([=](const boost::system::error_code& err) { this->print(); });
        }
    }

    ~printer(){std::cout<<"finish count is "<<count<<std::endl;
        timer_.cancel();
    }

private:
    boost::asio::deadline_timer timer_;
    int count;
};

int main()
{
    boost::asio::io_service io;

    {
        printer _print(io);
    }

    io.run();
    return 0;
}
