#ifndef _SQL_CONNPOOL_H_
#define _SQL_CONNPOOL_H_


#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>


class SqlConnPool {
public:
    static SqlConnPool* getInstance();

    MYSQL* getConn();
    void freeConn(MYSQL* conn);
    int freeConnCount();

    void init(const char* host, int port, const char* user, const char* pwd, const char* dbName, int maxConn=10);
    void destroyPool();
private:
    SqlConnPool();
    ~SqlConnPool();

    int maxConn;
    int useCount;
    int freeCount;

    std::queue<MYSQL*> connQ;
    std::mutex mtx;
    sem_t semId;
};

#endif
