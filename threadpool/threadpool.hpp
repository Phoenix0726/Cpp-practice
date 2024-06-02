#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>


class ThreadPool {
public:
    ThreadPool(int size=2);
    ~ThreadPool();
    void add(std::function<void()>);
private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex tasks_mtx;
    std::condition_variable cv;
    bool stop;
};


ThreadPool::ThreadPool(int size) : stop(false) {
    for (int i = 0; i < size; i++) {
        threads.emplace_back(std::thread([this]() {
            while (true) {
                std::function<void()> task;
                {   // 在作用域内对 std::mutex 加 unique_lock 锁，出了作用域会自动解锁
                    std::unique_lock<std::mutex> lock(tasks_mtx);
                    cv.wait(lock, [this]() {    // 等待条件变量，任务队列不为空或线程池停止时停止等待
                        return stop || !tasks.empty();
                    });
                    if (stop && tasks.empty()) return;
                    task = tasks.front();
                    tasks.pop();
                }
                task();
            }
        }));
    }
}


ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(tasks_mtx);
        stop = true;
    }
    cv.notify_all();
    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }
}


void ThreadPool::add(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(tasks_mtx);
        if (stop) {
            throw std::runtime_error("ThreadPool already stop, can't add task!");
        }
        tasks.emplace(task);
    }
    cv.notify_one();
}
