#include "AsioIOServicePool.h"
#include <iostream>

AsioIOServicePool::AsioIOServicePool(std::size_t size) : _ioServices(size), _nextIOService(0) {
    for (std::size_t i = 0; i < size; ++i) {
        _works.emplace_back(std::make_unique<Work>(_ioServices[i].get_executor()));
    }

    // 遍历多个ioservice，创建多个线程，每个线程内部启动ioservice
    for (std::size_t i = 0; i < size; ++i) {
        _threads.emplace_back([this, i] {
            _ioServices[i].run();
        });
    }
}

AsioIOServicePool::~AsioIOServicePool() {
    Stop();
    std::cout << "AsioIOServicePool destruct" << std::endl;
}

boost::asio::io_context& AsioIOServicePool::GetIOService() {
    auto& service = _ioServices[_nextIOService++];
    if (_nextIOService == _ioServices.size()) {
        _nextIOService = 0;
    }
    return service;
}

void AsioIOServicePool::Stop() {
    // 因为仅仅执行work.reset并不能让iocontext从run的状态中退出
    // 当iocontext已经绑定了读或写的监听事件后，还需要手动stop该服务。
    // 停止所有的io_context（在work.reset之前stop）
    for (auto& service : _ioServices) {
        service.stop();
    }

    // 释放所有工作守卫，让run()彻底退出
    for (auto& work : _works) {
        work.reset();
    }

    // 等待所有工作线程执行完毕，回收资源
    for (auto& t : _threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}