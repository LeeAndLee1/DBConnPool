#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H

#include "Connection.h"
#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#include <condition_variable>
#include "public.h"

/*
* 采用单例模式，实现数据库连接池的功能
*/

class ConnectionPool
{
public:
    // 获取连接池对象实例（懒汉式单例，在获取实例时才实例化对象）
    static ConnectionPool* getConnectionPool();

    // 从连接池中获取一个可用的空闲连接。
    // 这里直接返回一个智能指针，智能指针出作用域自动析构，（我们只需重定义析构即可：不释放而是归还）
    std::shared_ptr<Connection> getConnection();

private:
    // 单例模式：构造函数私有化
    ConnectionPool();

    // 防止拷贝构造
    ConnectionPool(ConnectionPool& ) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;

    // 从配置文件中加载配置项
    bool loadConfigFile();

    // 运行在独立的线程中，专门负责生产新连接。
    // 非静态成员方法，其调用依赖对象，要把其设计为一个线程函数，需要绑定this指针。 
    // 把该线程函数写为类的成员方法，最大的好处是 非常方便访问当前对象的成员变量。
    void produceConnectionTask();

    // 扫描超过maxIdleIme时间的空闲连接，对其进行回收
    void scannerConnectionTask();

    std::string     _ip;        //mysql的IP地址
    unsigned short  _port;      //mysql的端口号，默认3306
    std::string     _username;  //mysql登录的用户名
    std::string     _password;  //mysql登录的密码
    std::string     _dbname;    //连接mysql使用的数据库名
    int             _initSize;  //连接池的初始连接数
    int             _maxSize;   //连接池的最大连接数
    int             _maxIdleTime;   //连接的最大空闲时间
    int             _connectionTimeout; //连接的超时时间

    // 存储mysql连接的队列
    std::queue<Connection*> _connectionQue;
    // 维护连接队列的线程安全互斥锁
    std::mutex  _queueMutex;

    // 使用原子变量记录连接的总数量
    std::atomic_int  _connectionCnt;

    // 使用条件变量，用于 连接生产线程 和 连接消费线程之间的通信
    std::condition_variable cvNeedCreate;   // 队列为空，需要生产
    std::condition_variable cvNeedConsumer;  // 队列不为空，要等待
};

#endif