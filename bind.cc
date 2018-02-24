//
// Created by root on 18-2-12.
//

//
// Created by root on 18-2-12.
//

#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>
#include <functional>
#include <memory>
#include <vector> 
#include <algorithm>

using namespace std;

void print(const boost::system::error_code&){
    cout<<"hello world"<<endl;
    
};

void callback(const boost::system::error_code&)
{
    cout<<"second timer"<<endl;
}

void callback1(const boost::system::error_code&, shared_ptr<boost::asio::deadline_timer> t, int *count)
{
    if(*count <5)
    {
        cout<<*count<<endl;
        ++*count;
        t->expires_at(t->expires_at()+boost::posix_time::seconds(1));
//        t->async_wait(boost::bind(callback1, boost::asio::placeholders::error, t, count));
        t->async_wait([=](const auto& err){callback1(err, t, count);});

    }
}

std::shared_ptr<boost::asio::deadline_timer>
registerCallback(boost::asio::io_service &io, int*count)
{
    auto t = make_shared<boost::asio::deadline_timer>(io , boost::posix_time::seconds(1));
    t->async_wait([t, count](const auto& err){callback1(err, t, count);});

    return t;
    
}

int main()
{
    boost::asio::io_service io;
    std::vector<int> v;
    for(int i=0; i<5;   ++i)
        v.push_back(i);

    for(int i= 0; i< 5;++i)
    {
        auto  m = registerCallback(io, &v[i]);
        cout<<m.get()<<endl;
    }

    auto  t = make_shared<boost::asio::deadline_timer>(io,boost::posix_time::seconds(1));


    int count =0;
//    t.async_wait(boost::bind(callback1, boost::asio::placeholders::error, &t, &count));
    t->async_wait([&](const auto& err){callback1 (err, t, &count);});
    cout<<"hello world"<<endl;
    io.run();
    cout<<"finish"<<endl;
    return 0;
    
}


/*
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

void print(const boost::system::error_code& */
/*e*//*
,
           boost::asio::deadline_timer* t, int* count)
{
    if (*count < 5)
    {
        std::cout << *count << "\n";
        ++(*count);

        t->expires_at(t->expires_at() + boost::posix_time::seconds(1));
        t->async_wait(boost::bind(print,
                                  boost::asio::placeholders::error, t, count));
    }
}

int main()
{
    boost::asio::io_service io;

    int count = 0;
    boost::asio::deadline_timer t(io, boost::posix_time::seconds(1));
    t.async_wait(boost::bind(print,
                             boost::asio::placeholders::error, &t, &count));

    io.run();

    std::cout << "Final count is " << count << "\n";

    return 0;
}
*/
