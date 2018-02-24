# boost_asio

自己学习boost_asio 库写了一个多人在线聊天室，支持多个聊天室，并且每个用户绑定名字

设计了彼此通信的简单的消息类型，使用了c struct， boost 的serilization ，json和protobuf 这几种方式来进行 ccode 和dcode 

并且在多人聊天的并发编程方面 使用了两种，一个是用boost::asio::io_service::strand 来将其进行序列化，还有一种是 我自己用shared_ptr来实现的
copy_on_write，应该理论上比 strand  效率要高吧2333
因为strand 是将异步回调线性化了，而copy_on_write 是 可以并发对临界区进行读写的。

