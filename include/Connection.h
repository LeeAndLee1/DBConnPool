#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <mysql/mysql.h>
#include <ctime>

/*
* 连接类Connection的功能：
* 1、连接数据库
* 2、操作数据库
* 3、返回一个连接的空闲时间（用于释放多余产生的连接）
*/

class Connection
{
public:
    // 初始化数据库连接 
    Connection();

    // 释放数据库连接
    ~Connection();

    // 连接数据库
    bool connect(std::string ip, unsigned short port, std::string username, std::string password, std::string dbname);

    // 操作数据库：insert, delete, update
    bool update(std::string sql);

    // 查询数据库: select
    MYSQL_RES* query(std::string sql);

    // 刷新连接对象的起始空闲时刻，缓解服务器资源（在连接对象入队时）
    void refreshAliveTime();

    // 返回连接空闲的时长
    clock_t getAliveTime();

private:
    MYSQL*  _conn; //mysql server的一条连接
    clock_t _startAlivetime; // 记录进入空闲状态后的起始存活时刻（即进入队列的时刻）
};

#endif