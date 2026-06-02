#include "RedisMgr.h"
#include "ConfigMgr.h"

/**
 * RedisConPool
 */

RedisConPool::RedisConPool(size_t poolSize, std::string host, int port, const char* pwd) :
            poolSize_(poolSize), host_(host), port_(port) {
    for (size_t i = 0; i < poolSize_; ++i) {
        auto* context = redisConnect(host.c_str(), port);
        if (context == nullptr || context->err != 0) {
            if (context != nullptr) {
                redisFree(context);
            }
            continue;
        }
        auto reply = (redisReply*)redisCommand(context, "AUTH %s", pwd);
        if (reply == nullptr) {
            redisFree(context);
            continue;
        }
        if (reply->type == REDIS_REPLY_ERROR) {
            std::cout << "认证失败" << std::endl;
            freeReplyObject(reply);
            redisFree(context);
            continue;
        }
        freeReplyObject(reply);
        std::cout << "认证成功" << std::endl;
        connections_.push(context);
    }
}

RedisConPool::~RedisConPool() {
    Close();
    std::lock_guard<std::mutex> lock(mutex_);
    while (!connections_.empty()) {
        redisFree(connections_.front());
        connections_.pop();
    }
}

redisContext* RedisConPool::getConnection() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] {
        if (b_stop_) {
            return true;
        }
        return !connections_.empty();
    });
    if (b_stop_) {
        return nullptr;
    }
    auto* context = connections_.front();
    connections_.pop();
    return context;
}

void RedisConPool::returnConnection(redisContext* context) {
    if (context == nullptr) return;
    std::lock_guard<std::mutex> lock(mutex_);
    if (b_stop_) {
        redisFree(context);
        return;
    }
    connections_.push(context);
    cond_.notify_one();
}

void RedisConPool::Close() {
    b_stop_ = true;
    cond_.notify_all();
}


/**
 * RedisMgr
 */

RedisMgr::RedisMgr() {
    auto& gCfgMgr = ConfigMgr::Inst();
    auto& host = gCfgMgr["Redis"]["Host"];
    auto& port = gCfgMgr["Redis"]["Port"];
    auto& pwd = gCfgMgr["Redis"]["Passwd"];
    _con_pool.reset(new RedisConPool(5, host, atoi(port.c_str()), pwd.c_str()));
}

RedisMgr::~RedisMgr() {
    Close();
}

void RedisMgr::Close() {
    _con_pool->Close();
}

bool RedisMgr::Get(const std::string &key, std::string& value) {
    RedisConGuard connect(_con_pool.get());
    if (!connect) return false;

    auto reply = (redisReply*)redisCommand(connect.get(), "GET %s", key.c_str());
    if (reply == nullptr) {
        std::cout << "[ GET " << key << " ] failed" << std::endl;
        return false;
    }
    if (reply->type != REDIS_REPLY_STRING) {
        std::cout << "[ GET " << key << " ] failed" << std::endl;
        freeReplyObject(reply);
        return false;
    }
    value = reply->str;
    freeReplyObject(reply);
    std::cout << "Succeed to execute command [ GET " << key << " ]" << std::endl;
    return true;
}

bool RedisMgr::Set(const std::string &key, const std::string &value) {
    RedisConGuard connect(_con_pool.get());
    if (!connect) return false;

    auto reply = (redisReply*)redisCommand(connect.get(), "SET %s %s", key.c_str(), value.c_str());
    if (reply == nullptr) {
        std::cout << "Execut command [ SET " << key << " " << value << " ] failure ! " << std::endl;
        return false;
    }
    if (!(reply->type == REDIS_REPLY_STATUS && (strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0))) {
        std::cout << "Execut command [ SET " << key << " " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    std::cout << "Execut command [ SET " << key << " " << value << " ] success ! " << std::endl;
    return true;
}

bool RedisMgr::LPush(const std::string &key, const std::string &value) {
    RedisConGuard connect(_con_pool.get());
    if (!connect) return false;

    auto reply = (redisReply*)redisCommand(connect.get(), "LPUSH %s %s", key.c_str(), value.c_str());
    if (reply == nullptr) {
        std::cout << "Execut command [ LPUSH " << key << " " << value << " ] failure ! " << std::endl;
        return false;
    }
    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
        std::cout << "Execut command [ LPUSH " << key << " " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    std::cout << "Execut command [ LPUSH " << key << " " << value << " ] success ! " << std::endl;
    return true;
}

bool RedisMgr::LPop(const std::string &key, std::string& value) {
    RedisConGuard connect(_con_pool.get());
    if (!connect) return false;

    auto reply = (redisReply*)redisCommand(connect.get(), "LPOP %s", key.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        std::cout << "Execut command [ LPOP " << key << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    value = reply->str;
    freeReplyObject(reply);
    std::cout << "Execut command [ LPOP " << key << " ] success ! " << std::endl;
    return true;
}

bool RedisMgr::RPush(const std::string& key, const std::string& value) {
    RedisConGuard connect(_con_pool.get());
    if (!connect) return false;

    auto reply = (redisReply*)redisCommand(connect.get(), "RPUSH %s %s", key.c_str(), value.c_str());
    if (reply == nullptr) {
        std::cout << "Execut command [ RPUSH " << key << " " << value << " ] failure ! " << std::endl;
        return false;
    }
    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
        std::cout << "Execut command [ RPUSH " << key << " " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    std::cout << "Execut command [ RPUSH " << key << " " << value << " ] success ! " << std::endl;
    return true;
}

bool RedisMgr::RPop(const std::string& key, std::string& value) {
    RedisConGuard connect(_con_pool.get());
    if (!connect) return false;

    auto reply = (redisReply*)redisCommand(connect.get(), "RPOP %s", key.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        std::cout << "Execut command [ RPOP " << key << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    value = reply->str;
    freeReplyObject(reply);
    std::cout << "Execut command [ RPOP " << key << " ] success ! " << std::endl;
    return true;
}

bool RedisMgr::HSet(const std::string &key, const std::string &hkey, const std::string &value) {
    RedisConGuard connect(_con_pool.get());
    if (!connect) return false;

    auto reply = (redisReply*)redisCommand(connect.get(), "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "Execut command [ HSet " << key << " " << hkey << " " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    std::cout << "Execut command [ HSet " << key << " " << hkey << " " << value << " ] success ! " << std::endl;
    return true;
}

bool RedisMgr::HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen) {
    const char* argv[4];
    size_t argvlen[4];
    argv[0] = "HSET";
    argvlen[0] = 4;
    argv[1] = key;
    argvlen[1] = strlen(key);
    argv[2] = hkey;
    argvlen[2] = strlen(hkey);
    argv[3] = hvalue;
    argvlen[3] = hvaluelen;

    RedisConGuard connect(_con_pool.get());
    if (!connect) return false;

    auto reply = (redisReply*)redisCommandArgv(connect.get(), 4, argv, argvlen);
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "Execut command [ HSet " << key << " " << hkey << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    std::cout << "Execut command [ HSet " << key << " " << hkey << " ] success ! " << std::endl;
    return true;
}

std::string RedisMgr::HGet(const std::string &key, const std::string &hkey) {
    RedisConGuard connect(_con_pool.get());
    if (!connect) return "";

    const char* argv[3];
    size_t argvlen[3];
    argv[0] = "HGET";
    argvlen[0] = 4;
    argv[1] = key.c_str();
    argvlen[1] = key.length();
    argv[2] = hkey.c_str();
    argvlen[2] = hkey.length();

    auto reply = (redisReply*)redisCommandArgv(connect.get(), 3, argv, argvlen);
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        std::cout << "Execut command [ HGet " << key << " " << hkey << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return "";
    }
    std::string value = reply->str;
    freeReplyObject(reply);
    std::cout << "Execut command [ HGet " << key << " " << hkey << " ] success ! " << std::endl;
    return value;
}

bool RedisMgr::Del(const std::string &key) {
    RedisConGuard connect(_con_pool.get());
    if (!connect) return false;

    auto reply = (redisReply*)redisCommand(connect.get(), "DEL %s", key.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "Execut command [ Del " << key << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    std::cout << "Execut command [ Del " << key << " ] success ! " << std::endl;
    return true;
}

bool RedisMgr::ExistsKey(const std::string &key) {
    RedisConGuard connect(_con_pool.get());
    if (!connect) return false;

    auto reply = (redisReply*)redisCommand(connect.get(), "exists %s", key.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER || reply->integer == 0) {
        std::cout << "Not Found [ Key " << key << " ] ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    std::cout << "Found [ Key " << key << " ] exists ! " << std::endl;
    return true;
}
