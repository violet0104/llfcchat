#include <vector>
#include <boost/asio.hpp>
#include "Singleton.h"

class AsioIOServicePool : public Singleton<AsioIOServicePool>
{
    friend Singleton<AsioIOServicePool>;
public:                                                                                                                  
    // io_context: Asio 的事件循环核心
    using IOService = boost::asio::io_context; 
    // 工作守卫，阻止 io_context::run() 在没有事件时返回
    // 构造时传 ioc.get_executor()，调用 work.reset() 后 run() 才会退出
    using Work = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
    using WorkPtr = std::unique_ptr<Work>;
    ~AsioIOServicePool();

    // Singleton中已经删除了拷贝构造和拷贝赋值
    // AsioIOServicePool(const AsioIOServicePool&) = delete;
    // AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;

    // 轮询返回一个 io_context
    boost::asio::io_context& GetIOService();
    void Stop();

private:
    AsioIOServicePool(std::size_t size = 2 /*std::thread::hardware_concurrency()*/);
    std::vector<IOService> _ioServices; // io_context
    std::vector<WorkPtr> _works; // 有多少个IOService就有多少个WorkPtr
    std::vector<std::thread> _threads; // 有多少个IOService就有多少个thread
    std::size_t _nextIOService; // 轮询索引
};