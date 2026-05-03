#include "VarifyGrpcClient.h"
#include "ConfigMgr.h"

RPConPool::RPConPool(size_t poolSize, std::string host, std::string port)
    : poolSize_(poolSize), host_(host), port_(port), b_stop_(false) {
    for (size_t i = 0; i < poolSize_; ++i) {
        std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port,
            grpc::InsecureChannelCredentials());
        // 为每个连接创建一个Stub
        connections_.push(VarifyService::NewStub(channel)); // push时使用移动构造，移动unique_ptr
    }
}

RPConPool::~RPConPool() {
    std::lock_guard<std::mutex> lock(mutex_);
    Close();
    while (!connections_.empty()) {
        connections_.pop();
    }
}

void RPConPool::Close() {
    b_stop_ = true;
    cond_.notify_all();
}

// 从连接池中获取连接
std::unique_ptr<VarifyService::Stub> RPConPool::getConnection() {
    std::unique_lock<std::mutex> lock(mutex_);
    // lambda表达式返回true时：停止等待，唤醒线程
    cond_.wait(lock, [this]() {
        // 连接池停止
        if (b_stop_) {
            return true;
        }
        // 或者队列不空
        return !connections_.empty();
    });
    
    if (b_stop_) {
        return nullptr;
    }

    // 队列不空
    auto context = std::move(connections_.front());
    connections_.pop();
    return context;
}

// 将连接还给连接池
void RPConPool::returnConnection(std::unique_ptr<VarifyService::Stub> context) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (b_stop_) {
        return;
    }
    connections_.push(std::move(context)); // 把context移动为右值，push再触发移动构造
    cond_.notify_one();
}

GetVarifyRsp VerifyGrpcClient::GetVarifyCode(std::string email) {
    ClientContext context;
    GetVarifyRsp reply;
    GetVarifyReq request;
    request.set_email(email);

    auto stub = pool_->getConnection();
    Status status = stub->GetVarifyCode(&context, request, &reply);
    if (status.ok()) {
        // 回收连接
        pool_->returnConnection(std::move(stub));
        return reply;
    } else {
        // 回收连接
        pool_->returnConnection(std::move(stub));
        reply.set_error(ErrorCodes::RPCFailed);
        return reply;
    }
}

VerifyGrpcClient::VerifyGrpcClient() {
    auto& gCfMgr = ConfigMgr::Inst();
    std::string host = gCfMgr["VarifyServer"]["Host"];
    std::string port = gCfMgr["VarifyServer"]["Port"];
    pool_.reset(new RPConPool(5, host, port));
}