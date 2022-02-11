#include "threadpool.h"
#include <chrono>
ThreadPool::ThreadPool(size_t maxNum) : MaxNum(maxNum), CoreNum(std::thread::hardware_concurrency()) {

} 

void ThreadPool::Run() {
    while (!isTerminate) {
        TaskPtr task;
        bool tag = GetTask(task);
        busyNum++;
        if (tag) {
            try {
                // 判断任务是否超时
                if (task->expireTime != 0) {

                } else {
                    task->func();
                    busyNum--;
                }
            } catch(...) {
                printf("unknown error\n");
            }
        }
    }
}

bool ThreadPool::Start() {
    // ThreadPool 要保证是线程安全的，因为可能有多个线程访问同一个 ThreadPool 对象？
    // 不是只有 main 线程访问线程池对象吗，多个线程的场景我还真没想到
    std::unique_lock<std::mutex> lock(taskListMtx);
    if (!threads.empty()) {
        printf("线程池不为空\n");
        return false;
    }
    for (int i = 0; i < CoreNum; i++) {
        auto thread = std::make_shared<std::thread>(std::bind(&ThreadPool::Run, this));
        threads.push_back(thread);
    }
}



bool ThreadPool::GetTask(TaskPtr &_task) {
    // 多线程取 task，要加锁
    std::unique_lock<std::mutex> lock(taskListMtx);
    // 如果 TaskList 没有 Task，就阻塞直到被唤醒
    if (taskList.empty()) {
        // taskList 不为空且线程池未被关闭
        cv.wait(lock, [this] {
            return isTerminate == true || !taskList.empty();// taskList 不为空或者线程池终止都需要被唤醒
        });
    }

    if (isTerminate) {
        return false;
    }

    _task = std::move(taskList.front());// 取任务
    taskList.pop_front();
    
    return true;
}

void ThreadPool::Shutdown() {
    // 这里和锁 taskList 的锁应该不一样
    std::unique_lock<std::mutex> lock(threadsMtx);
    if (!isTerminate) {
        isTerminate = true;
        cv.notify_all();
        size_t thread_num = threads.size();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        for (int i = 0; i < thread_num; i++) {
            threads[i]->join();
        }
    }
}