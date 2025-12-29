#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

#include <deque>
#include <functional>
#include "error.hpp"
#include <iostream>
#include "event.hpp"
#include <thread>
#include <condition_variable>
#include <signal.h>
#include <unistd.h>
#include <any>
#include <map>
#include <sys/epoll.h>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include "callback.hpp"
#include "thread_queue.hpp"
#include "epoll_wrapper.hpp"

#define EPOLL_COUNT 50

namespace asyncio {

class executor {

    void event_loop() {
        m_convar.notify_all();
        while(runner) {
        }    
    }

public:

    executor(int thread_count = 2) {
        signal(SIGINT, [](int){
            executor::runner = false;
        });

        initialize_threads(thread_count);
    }
    
    ~executor() {
        m_convar.notify_all();
        for (int nthread = 0; nthread < m_threads_active; ++nthread) {
            tpool[nthread].join();
        }
    }

    void initialize_threads(int thread_count) {
        tpool.resize(thread_count);

        for (int nthread = 0; nthread < thread_count; nthread++) {
            tpool[nthread] = std::thread(&executor::thread_loop, this, nthread);
        }
        m_threads_active = thread_count;
    }

    inline static std::atomic_flag runner = true;

    void hijack() {
        event_loop();
    }

    void enqueue_task(Task* task) {
        queue.push(task);
        std::cout << "Task pushed\n";
        m_convar.notify_one();
    }

    void thread_loop(int thread_id) {
        Task* job;  
        std::cout << "Thread #" << thread_id << " started." << std::endl;
        std::mutex local_mutex;
        while (runner.test()) {  
            std::unique_lock<std::mutex> lock(local_mutex);
            m_convar.wait(lock, [this]{
                return queue.size() != 0 || runner == false;
            });
            if (runner == false) return;

            job = queue.pop();
            std::cout << "Job popped\n";
            (*job)();
        }
    }

    bool run_epoll_instance(std::shared_ptr<epoll_wrapper> epoll_instance) {
        m_epoll = epoll_instance;
        tpool.push_back(std::thread(&epoll_wrapper::work, m_epoll.lock().get()));
    }

private:

    int m_threads_active = 0;

    inline static bool epoll_running = false;
    thread_queue<Task*> queue = {};
    std::mutex queue_mutex;
    std::weak_ptr<epoll_wrapper> m_epoll; // TODO; should be a collection of weak_ptrs
    std::condition_variable m_convar;
    std::vector<std::thread> tpool = {};
};
}
#endif
