#pragma once
#include <grpcpp/grpcpp.h>
#include <message.grpc.pb.h>
#include <const.h>
#include <Singleton.h>

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;
class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
    // 让单例模板可以访问本类的私有构造/析构
    friend class Singleton<VerifyGrpcClient>;
public:
    GetVarifyRsp GetVarify(std::string email) {
        ClientContext context;
        GetVarifyRsp reply;
        GetVarifyReq request;
        request.set_email(email);
    }
};