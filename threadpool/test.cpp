#include "threadpool.hpp"
#include <iostream>
#include <memory>


void print() {
    std::cout << "hello" << std::endl;
}


int main() {
    std::shared_ptr<ThreadPool> pool(new ThreadPool());
    
    std::function<void()> f = print;
    for (int i = 0; i < 5; i++) {
        pool->add(f);
    }

    return 0;
}
