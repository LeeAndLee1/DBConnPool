#include "Connection.h"

// 初始化数据库连接对象
Connection::Connection()
{
    _conn = mysql_init(nullptr);
}

// 释放数据库连接对象
Connection::~Connection()
{
    if (_conn != nullptr)
    {
        mysql_close(_conn);
    }
}

// 连接数据库
bool Connection::connect(std::string ip, unsigned short port, std::string username, std::string password, std::string dbname)
{
    MYSQL* p = mysql_real_connect(_conn, ip.c_str(), username.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0);
    return p != nullptr;
}

// 更新操作：insert, delete, update
bool Connection::update(std::string sql)
{
    // 如果查询成功，返回0；失败则返回非0值
    if (mysql_query(_conn, sql.c_str()))
    {
        return false;
    }
    return true;
}

// 查询操作：select
MYSQL_RES* Connection::query(std::string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        return nullptr;
    }
    return mysql_use_result(_conn);
}

// 刷新连接对象的起始空闲时刻
void Connection::refreshAliveTime()
{
    _startAlivetime = clock();
}

// 返回连接对象空闲的时长
clock_t Connection::getAliveTime()
{
    return clock() - _startAlivetime;
}