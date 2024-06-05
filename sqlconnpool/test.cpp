#include "sqlconnpool.h"
#include <iostream>


int main() {
    SqlConnPool* pool = SqlConnPool::getInstance();
    pool->init("localhost", 0, "username", "password", "db_name");
    return 0;
}
