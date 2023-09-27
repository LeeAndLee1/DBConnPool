#include "ConnectionPool.h"


// 连接池的实现

// 线程安全的懒汉模式单例函数接口
ConnectionPool* ConnectionPool::getConnectionPool()
{
    // c++11特新，对于静态局部变量的初始化，编译器自动进行lock和unlock
    static ConnectionPool pool;
    return &pool;
}

// 从配置文件中加载配置项
bool ConnectionPool::loadConfigFile()
{
    FILE* pf ;
    pf = fopen("../src/mysql.init", "r");
    if (pf == nullptr)
    {
        perror("open mysql init file fail!\n");
        return false;
    }

    while (!feof(pf))
    {
        // 按行获取字符串
        char line[1024] = {0};
        fgets(line, 1024, pf);
        std::string strLine = line;

        // 找到配置字符串中的 '='
        std::size_t idx = strLine.find('=', 0);
        // 无效配置
        if (idx == std::string::npos)
        {
            // 当配置字符串中找不到'='时说明该配置字符串有问题或者是注释，将其忽略
            continue;
        }

        // 分别取出该行配置中的key和value
        std::size_t endIdx = strLine.find('\n', idx);
        std::string key = strLine.substr(0, idx);
        std::string value = strLine.substr(idx+1, endIdx - idx - 1);

        if (key == "ip")
        {
            _ip = value;
        }
        else if (key == "port")
        {
            _port = std::stoi(value);
        }
        else if (key == "username")
        {
            _username = value;
        }
        else if (key == "password")
        {
            _password = value;
        }
        else if (key == "dbname")
        {
            _dbname = value;
        }
        else if (key == "maxSize")
        {
            _maxSize = std::stoi(value);
        }
        else if (key == "maxIdleTime")
        {
            _maxIdleTime = std::stoi(value);
        }
        else if (key == "connectionTimeout")
        {
            _connectionTimeout = std::stoi(value);
        }
        else if (key == "initSize")
        {
            _initSize = std::stoi(value);
        }
    }
    return true;
}

// 初始化连接池
ConnectionPool::ConnectionPool()
{
    // 加载mysql配置文件
    if (!loadConfigFile())
    {
        return;
    }
    // 创建初始数量的连接
    for (auto i = 0; i < _initSize; i++)
    {
        Connection* conn = new Connection();
        if (conn->connect(_ip, _port, _username, _password, _dbname))
        {
            conn->refreshAliveTime();
            _connectionQue.push(conn);
            _connectionCnt++;
        }
    }

    // 启动一个新的线程，作为连接的生产者
    std::thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
    produce.detach();   //守护线程

    // 启动一个新的定时线程，扫描超过maxIdleTime时间的空闲连接，并对其进行回收
    std::thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
    scanner.detach();   //守护线程
}

// 专门负责生产新连接，使用条件变量和互斥锁跟消费者通信
void ConnectionPool::produceConnectionTask()
{
    for (; ;)
    {
        //条件变量阻塞线程之前，先对互斥锁执行“加锁”操作
        std::unique_lock<std::mutex> lock(_queueMutex);
        // 队列非空时，生产者进入等待状态
        while (!_connectionQue.empty())
        {
            // 等待条件唤醒，线程阻塞，释放lock
            cvNeedCreate.wait(lock);
        }

        // 队列为空，但连接数量没有达到最大连接数maxSize，继续创建连接
        if (_connectionCnt < _maxSize)
        {
            Connection* conn = new Connection();
            if (conn->connect(_ip, _port, _username, _password, _dbname))
            {
                _connectionQue.push(conn);
                _connectionCnt++;
            }
        }

        // 通知消费者，唤醒条件并释放lock
        cvNeedConsumer.notify_all();
    }
}

// 消费者，从连接池中获取一个空闲连接，用完之后，归还到连接池
std::shared_ptr<Connection> ConnectionPool::getConnection()
{
    std::unique_lock<std::mutex> lock(_queueMutex);
    // 条件等待超时，主动唤醒，继续执行
    while (_connectionQue.empty())
    {
        // 最多等待_connectionTimeout时间，超时就解除阻塞
        if (std::cv_status::timeout == cvNeedConsumer.wait_for(lock, std::chrono::milliseconds(_connectionTimeout)))
        {
            if (_connectionQue.empty())
            {
                LOG("get connection from connection pool timeout!\n");
                return nullptr;
            }
        }
    }

    /*
    * 自定义shared_ptr的析构函数lambda表达式，shared_ptr超出作用域，释放资源的时，
    * 是把connection直接归还到queue当中，而非直接close掉连接。
    * 取出位于队头的连接赋值给sp
    */
    std::shared_ptr<Connection> sp(_connectionQue.front(), [&](Connection* conn)
    {
        // 操作线程池队列，先加锁
        std::unique_lock<std::mutex> lock(_queueMutex);
        // 归还回空闲连接队列之前要刷新一下连接开始空闲的时刻
        conn->refreshAliveTime();
        _connectionQue.push(conn);
    });
    _connectionQue.pop();
    
    // 消费者取出一个连接后，通知生产者，生产者检查队列，如果为空则生产
    cvNeedCreate.notify_all();

    return sp;
}

// 一直扫描超过maxIdleTime时间的空闲连接，将其释放
void ConnectionPool::scannerConnectionTask()
{
    for (; ;)
    {
        // 通过sleep实现定时效果
        std::this_thread::sleep_for(std::chrono::seconds(_maxIdleTime));

        // 扫描整个队列，释放超过最大空闲时间的连接
        std::unique_lock<std::mutex> lock(_queueMutex);
        while (_connectionCnt > _initSize)
        {
            Connection* pconn = _connectionQue.front();
            if (pconn->getAliveTime() >= _maxIdleTime * 1000)
            {
                _connectionQue.pop();
                _connectionCnt--;
            }
            else
            {
                // 队头的连接没有超过_maxIdleTime，说明其它连接肯定也没有
                break;
            }
        }
    }
}
