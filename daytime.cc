#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <algorithm>

using boost::asio::ip::tcp;

int main(int argc, char* argv[])
{

	try{

		if(argc != 2)
		{
			std::cerr<<"usage client <host>"<<std::endl;
			return 1;
		}
		
		boost::asio::io_service io;

		tcp::resolver resolver(io);
		//这里解析要提供线程io
		tcp::resolver::query query(argv[1], "daytime");
		//这里是ip地址和端口号，也可以写成如下的形式,查看端口号和特定的服务的名称
		//使用命令 cat /etc/services 	里面有所有的服务对应的端口号
		tcp::resolver::query query1("time.nist.gov","13");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		//返回迭代器的头部，指向解析好的内容
		tcp::socket socket(io);
		boost::system::error_code error;
		boost::asio::connect(socket, endpoint_iterator,error);
		//函数签名如下
		//boost::asio::basic_socket<...> &s, boost::asio::ip::basic_resolver<...>::iterator begin, boost::system::error_code &ec)
		for(;;)
		{
			// boost::array<char, 128> buf;
			char buf[128];

			size_t len = socket.read_some(boost::asio::buffer(buf,128), error);	
//			size_t len = socket.read_some(boost::asio::buffer(buf), error);	 
//			这里buffer 函数有很多重载，其实只要第一个参数是void* 就可以了
		//This function is used to read data from the stream socket.
		// The function call will block until one or more bytes of data has been read successfully, or until an error occurs.
		//read_some 是阻塞的，直到有字节数据到了才读，客户端可能可以，但是服务器不行
			boost::asio::error::misc_errors tag_ = boost::asio::error::eof;
			if(error == tag_)
				break;
			else if(error){
				throw boost::system::error_code(error);
			}
			//
			std::cout.write(buf, len);
		}

	}catch(std::exception& e)
	{
		std::cout<<e.what()<<std::endl;
	}

	return 0;
}
