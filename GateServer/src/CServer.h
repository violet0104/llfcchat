#pragma once

#include "const.h"

class HttpConnection;
// std::enable_shared_from_this 是安全生成自身智能指针的工具类
// 继承它 = 类内部可以用 shared_from_this() 获取自己的 shared_ptr
class CServer : public std::enable_shared_from_this<CServer>
{
public:
	CServer(boost::asio::io_context& ioc, unsigned short& port);
	void Start();
private:
	// _acceptor：连接接收器
	tcp::acceptor  _acceptor;
	// _ioc类似linux下的epoll
	net::io_context& _ioc;
	tcp::socket   _socket;
};
