#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <vector>
#include <list>
#include <functional>
#include <thread>
#include <memory>
#include <future>
#include <atomic>
#include <mutex>
#include <condition_variable>

class ThreadPool {
public:
    struct Task {
        Task(size_t time = 0) : expireTime(time) {}
        std::function<void()> func;// Task 可调用函数实体
        size_t expireTime;
    };
    // 每个 Task 的指针
    using TaskPtr = std::shared_ptr<Task>;

    ThreadPool(size_t maxNum = 5);
    // 1.任务处理函数, F 为何使用右值引用呢 ?
    template <typename F, typename ...Args>
    auto execute(size_t timeoutMs, F&& func, Args&& ...args) -> std::future<decltype(func(args...))> {
        // 1.将传进来的函数构造成一个 Task,插入到 taskList
        TaskPtr task = std::make_shared<Task>(timeoutMs);
        using RetType = decltype(func(std::forward<Args>(args)...));
        // packaged_task 不能被 lambda 捕获
        auto pt = std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<F>(func), std::forward<Args>(args)...));
        task->func = [pt] {
            (*pt)();
        };
        std::unique_lock<std::mutex> lock(taskListMtx);
        taskList.push_back(task);
        lock.unlock();

        // 2.唤醒一个线程
        cv.notify_one();

        return pt->get_future();
    }
    // 重载版本，不设置超时时间
    template <typename F, typename ...Args>
    auto execute() {
        
    }
    // 启动线程池
    bool Start();
    void Shutdown();
    void Run();
    bool GetTask(TaskPtr &);

private:
    // 应该有两个 mtx，一个锁 taskList, 一个锁 threads,分别锁才能提高效率
    std::mutex taskListMtx;
    std::mutex threadsMtx;
    std::condition_variable cv;
    std::list<TaskPtr> taskList;// 任务队列
    std::vector<std::shared_ptr<std::thread>> threads;// 存放实际线程结构
    
    // 两个阈值数
    size_t CoreNum{0};
    size_t MaxNum{0};
    bool isTerminate{false};

    std::atomic<int> busyNum{0};// 用来判断是否所有任务都执行完的

};


#endif