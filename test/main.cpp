#include <iostream>
#include <string>
#include <ctime>
#include "Connection.h"
#include "ConnectionPool.h"

int n = 10000;

// 测试往数据中插入数据
void insertTableNoUse()
{
    for (int i=0; i<n; i++)
    {
        Connection conn;
        char sql[1024] = { 0 };
        sprintf(sql, "insert into table1(id, name) values(%d, '%s')", 1, "aaaaa");
        conn.connect("127.0.0.1", 3306, "root", "123", "chat");
        conn.update(sql);
    }
}

// 不使用连接池，4线程
void noUseConnPool()
{
    clock_t begin = clock();
    std::thread t1(insertTableNoUse);
    std::thread t2(insertTableNoUse);
    std::thread t3(insertTableNoUse);
    std::thread t4(insertTableNoUse);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    clock_t end = clock();
    std::cout << "not use pool: " << (end - begin) << "ms" << std::endl;
}

// 使用连接池，4线程
void insertTableUse()
{
    ConnectionPool* connpool = ConnectionPool::getConnectionPool();
    for (int i=0; i<n; i++)
    {
        std::shared_ptr<Connection> sp = connpool->getConnection();
        if (!sp)
        {
            continue;
        }
        char sql[1024] = { 0 };
        sprintf(sql, "insert into table1(id, name) values(%d, '%s')", 1, "aaaaa");
        sp->update(sql);
    }
}
void useConnPool()
{
    clock_t begin = clock();
    std::thread t1(insertTableUse);
    std::thread t2(insertTableUse);
    std::thread t3(insertTableUse);
    std::thread t4(insertTableUse);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    clock_t end = clock();
    std::cout << "use pool: " << (end - begin) << "ms" << std::endl;
}

int main()
{
    noUseConnPool();    // not use pool: 65140000ms
    useConnPool();        // use pool: 2300000ms
    return 0;
}