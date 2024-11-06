#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

#include <deque>
#include <functional>
#include "error.hpp"
#include <iostream>
#include "event.hpp"
#include <thread>
#include <signal.h>
#include <unistd.h>
#include <any>
#include <map>
#include <sys/epoll.h>
#include <mutex>
#include <shared_mutex>
#include "callback.hpp"
#include "thread_queue.hpp"

#define EPOLL_COUNT 50

namespace asyncio {

class executor {

    void event_loop() {
        while(runner) {
            process_events();
        }    
    }

public:

    executor() {

        signal(SIGINT, [](int){
            executor::runner = false;
            executor::epoll_running = false;    
        });
     
        run_epoll();
        //epoll_fd = epoll_create(EPOLL_COUNT);
    }

    ~executor() {
        for(auto element : token_map) {
            delete element.second;
        }
    }

    inline static bool runner = true;

    void run() {
        event_loop();
    }

    void catch_signal(int dummy) {

    }

    void epoll_rearm(int fd, epoll_event &event) {
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
    }

    void run_epoll() {
        epoll_fd = epoll_create(EPOLL_COUNT);
        epoll_running = true;
        std::thread epoll_thread(&executor::epoll_work, this);

        epoll_thread.detach();
    }


    void epoll_work() {
        while(executor::epoll_running) {
            int nfds = epoll_wait(epoll_fd, epoll_events, EPOLL_COUNT, -1);
            for(int i = 0; i < nfds; i++) {
                this->enqueue_callback(token_map[epoll_events[i].data.u32]);
                if(epoll_events[i].data.u32 < 0) {
                    token_map.erase(epoll_events[i].data.u32);
                }
            }
        }
    }
  
    bool register_epoll(int fd, epoll_event &event, std::string runner) {
        std::cout << "register_epoll from " + runner + "\n";
        int result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
        if(result == -1) {
            std::cout << "epoll_ctl() error: " << errno << std::endl;
            return false;
        }
        else {
            std::cout << "REGISTERED: " << event.data.fd << std::endl;
            return true;
        }
    }

    void register_epoll_handler(Token *callback, int id) {
        token_map[id] = callback;
    }

    void run_thread(Token *callback) {
        std::cout << "Running in a thread: " << callback->name << std::endl;
        std::thread worker_thread(&Token::call, callback);
        worker_thread.detach();
    }

    void enqueue_callback(Token *callback) { 
        queue.push(callback);
        // std::cout << "ENQUEUED: " << callback->name << std::endl;
    }

private:
    inline static bool epoll_running = false;
    int epoll_fd = 0;
    epoll_event epoll_events[EPOLL_COUNT];
    std::map<signed int, Token*> token_map;

    thread_queue<Token*> queue = {};


    void process_events() {

        queue.wait();
        int current_size = queue.size();
        //std::cout << "Qeueue size: "<< current_size << std::endl;
        for(int i = 0; i < current_size; i++) {
            run_thread(queue.pop());
        }
    }

};
}
#endif
