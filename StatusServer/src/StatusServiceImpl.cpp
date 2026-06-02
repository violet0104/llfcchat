#include "StatusServiceImpl.h"
#include "ConfigMgr.h"
#include "const.h"

std::string generate_unique_string() {
    // 创建UUID对象
    boost::uuids::uuid u = boost::uuids::random_generator()();
    // 将UUID转换为字符串
    std::string unique_string = boost::uuids::to_string(u);
    return unique_string;
}

// 加载聊天服务器配置
StatusServiceImpl::StatusServiceImpl() : _server_index(0) {
    auto& cfg = ConfigMgr::Inst();
    ChatServer server;
    server.port = cfg["ChatServer1"]["Port"];
    server.host = cfg["ChatServer1"]["Host"];
    _servers.push_back(server);

    server.port = cfg["ChatServer2"]["Port"];
    server.host = cfg["ChatServer2"]["Host"];
     _servers.push_back(server);
}

// 获取聊天服务器：轮询负载均衡
Status StatusServiceImpl::GetChatServer(ServerContext *context, const GetChatServerReq *request, GetChatServerRsp *reply) {
    std::string prefix("llfc status server has received:  ");
    int index = _server_index.fetch_add(1) % _servers.size();
    auto &server = _servers[index];
    reply->set_host(server.host);
    reply->set_port(server.port);
    reply->set_error(ErrorCodes::Success);
    reply->set_token(generate_unique_string());
    return Status::OK;
}
