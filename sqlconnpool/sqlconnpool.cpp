#include "sqlconnpool.h"
#include <iostream>


SqlConnPool::SqlConnPool() {
    useCount = 0;
    freeCount = 0;
}

SqlConnPool::~SqlConnPool() {
    destroyPool();
}

SqlConnPool* SqlConnPool::getInstance() {
    static SqlConnPool pool;
    return &pool;
}

void SqlConnPool::init(const char* host, int port, const char* user, const char* pwd, const char* dbName, int maxConn) {
    for (int i = 0; i < maxConn; i++) {
        MYSQL* sql = mysql_init(NULL);
        if (!sql) {
            std::cout << "Mysql init error!" << std::endl;
            exit(1);
        }

        sql = mysql_real_connect(sql, host, user, pwd, dbName, port, nullptr, 0);
        if (!sql) {
            std::cout << "Mysql connect error!" << std::endl;
            exit(1);
        }

        connQ.push(sql);
    }

    this->maxConn = maxConn;
    sem_init(&semId, 0, this->maxConn);
}

MYSQL* SqlConnPool::getConn() {
    MYSQL* sql = nullptr;

    if (connQ.empty()) {
        std::cout << "Pool busy!" << std::endl;
        return nullptr;
    }

    sem_wait(&semId);
    {
        std::lock_guard<std::mutex> locker(mtx);
        sql = connQ.front();
        connQ.pop();
    }

    return sql;
}

void SqlConnPool::freeConn(MYSQL* sql) {
    std::lock_guard<std::mutex> locker(mtx);
    connQ.push(sql);
    sem_post(&semId);
}

void SqlConnPool::destroyPool() {
    std::lock_guard<std::mutex> locker(mtx);
    while (!connQ.empty()) {
        auto it = connQ.front();
        connQ.pop();
        mysql_close(it);
    }
    mysql_library_end();
}

int SqlConnPool::freeConnCount() {
    std::lock_guard<std::mutex> locker(mtx);
    return connQ.size();
}
