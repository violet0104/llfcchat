#pragma
#include "const.h"
#include <thread>
#include <mysql-cppconn/jdbc/mysql_driver.h>
#include <mysql-cppconn/jdbc/cppconn/prepared_statement.h>
#include <mysql-cppconn/jdbc/cppconn/resultset.h>
#include <mysql-cppconn/jdbc/cppconn/statement.h>
#include <mysql-cppconn/jdbc/cppconn/exception.h>

/**
 * SqlConnection 封装的MySql连接
 */
class SqlConnection 
{
public:
	SqlConnection(sql::Connection* con, int64_t lasttime):_con(con), _last_oper_time(lasttime) {}
	std::unique_ptr<sql::Connection> _con;
	int64_t _last_oper_time;
};

/**
 * MysqlPool
 */
class MysqlPool {
public:
    MysqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize);
    ~MysqlPool();
    void checkConnectionPro();
    bool reconnect(long long timestamp);
    void checkConnection();
    std::unique_ptr<SqlConnection> getConnection();
    void returnConnection(std::unique_ptr<SqlConnection> con);
    void Close();
    
private:
	std::string url_;
	std::string user_;
	std::string pass_;
	std::string schema_; // 数据库名
	int poolSize_;
	std::queue<std::unique_ptr<SqlConnection>> pool_;
	std::mutex mutex_;
	std::condition_variable cond_;
	std::atomic<bool> b_stop_;
	std::thread _check_thread;
	std::atomic<int> _fail_count;
};

struct UserInfo {
	std::string name;
	std::string pwd;
	int uid;
	std::string email;
};

/**
 * MysqlDao
 */
class MysqlDao
{
public:
	MysqlDao();
	~MysqlDao();
	int RegUser(const std::string& name, const std::string& email, const std::string& pwd);
	// int RegUserTransaction(const std::string& name, const std::string& email, const std::string& pwd, const std::string& icon);
	// bool CheckEmail(const std::string& name, const std::string & email);
	// bool UpdatePwd(const std::string& name, const std::string& newpwd);
	// bool CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo);
	// bool TestProcedure(const std::string& email, int& uid, std::string& name);
private:
	std::unique_ptr<MysqlPool> pool_;
};