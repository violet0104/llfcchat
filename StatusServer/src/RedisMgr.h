#pragma once
#include "const.h"
#include "Singleton.h"

/**
 * RedisContext 连接池  
 */
class RedisConPool
{
public:
    RedisConPool(size_t poolSize, std::string host, int port, const char* pwd);
    ~RedisConPool();
    redisContext* getConnection();
    void returnConnection(redisContext* context);
    void Close();

private:
    size_t poolSize_;
    std::string host_;
    int port_;
    std::atomic<bool> b_stop_;
    std::queue<redisContext*> connections_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

/**
 * RAII guard: 自动归还连接到连接池
 */
class RedisConGuard {
public:
    explicit RedisConGuard(RedisConPool* pool) : pool_(pool) {
        ctx_ = pool_->getConnection();
    }
    ~RedisConGuard() {
        if (ctx_) pool_->returnConnection(ctx_);
    }
    redisContext* get() const { return ctx_; }
    explicit operator bool() const { return ctx_ != nullptr; }
private:
    RedisConPool* pool_;
    redisContext* ctx_;
};


/**
 * RedisMgr: redis管理器
 */
class RedisMgr : public Singleton<RedisMgr>
{
    friend class Singleton<RedisMgr>;
public:
    ~RedisMgr();
    bool Get(const std::string &key, std::string& value);
    bool Set(const std::string &key, const std::string &value);
    bool LPush(const std::string &key, const std::string &value);
    bool LPop(const std::string &key, std::string& value);
    bool RPush(const std::string& key, const std::string& value);
    bool RPop(const std::string& key, std::string& value);
    bool HSet(const std::string &key, const std::string &hkey, const std::string &value);
    bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
    std::string HGet(const std::string &key, const std::string &hkey);
    bool Del(const std::string &key);
    bool ExistsKey(const std::string &key);
    void Close();

private:
    RedisMgr();
    std::unique_ptr<RedisConPool> _con_pool;
};
