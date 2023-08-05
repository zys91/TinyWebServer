#ifndef SQL_CONNECTION_POOL_H
#define SQL_CONNECTION_POOL_H

#include <string>
#include <list>
#include <mysql/mysql.h>

#include "lock/locker.h"
#include "log/log.h"

class connection_pool
{
public:
	MYSQL *GetConnection();				 // 获取数据库连接
	bool ReleaseConnection(MYSQL *conn); // 释放连接
	int GetFreeConn();					 // 获取连接
	void DestroyPool();					 // 销毁所有连接

	// 单例模式 懒汉模式 获取数据库连接池对象 C++11使用局部静态变量不用加锁
	static connection_pool *GetInstance(){
		static connection_pool connPool;
		return &connPool;
	}

	void init(std::string url, std::string User, std::string PassWord, std::string DataBaseName, int Port, int MaxConn, int disable_log);

private:
	connection_pool();
	~connection_pool();

	int m_MaxConn;	// 最大连接数
	int m_CurConn;	// 当前已使用的连接数
	int m_FreeConn; // 当前空闲的连接数
	locker lock;
	std::list<MYSQL *> connList; // 连接池
	sem reserve;

public:
	std::string m_url;		   // 主机地址
	std::string m_Port;		   // 数据库端口号
	std::string m_User;		   // 登陆数据库用户名
	std::string m_PassWord;	   // 登陆数据库密码
	std::string m_DatabaseName; // 使用数据库名
	int m_disable_log;	   // 日志开关
};

class connectionRAII
{

public:
	connectionRAII(MYSQL **con, connection_pool *connPool);
	~connectionRAII();

private:
	MYSQL *conRAII;
	connection_pool *poolRAII;
};

#endif
