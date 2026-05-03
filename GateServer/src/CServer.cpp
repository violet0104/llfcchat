#include "CServer.h"
#include "HttpConnection.h"
#include "AsioIOServicePool.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short& port) : _ioc(ioc),
 _acceptor(ioc, tcp::endpoint(tcp::v4(), port)), _socket(ioc) {
    
}

void CServer::Start() {
    auto self = shared_from_this();
    // 从AsioIOServicePool获取一个io_context
    auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
    // 一个新连接交给一个io_context管理
    std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(io_context);
    _acceptor.async_accept(new_con->GetSocket(), [self, new_con](beast::error_code ec) {
        try {
            // ec = true 表示出错，出错则放弃这个连接，继续监听新链接
            if (ec) {
                // 继续监听新连接
                self->Start();
                return;
            }

            // 处理新连接
            new_con->Start();
            // 继续监听
            self->Start();
        }
        catch (std::exception& exp){
            std::cout << "exception is " << exp.what() << std::endl;
			self->Start();
        }
    });
}