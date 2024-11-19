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
    }

    ~executor() {
    }

    inline static bool runner = true;

    void run() {
        event_loop();
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

                //this is giga scuffed
                this->enqueue_callback(std::move(Token(*token_map[epoll_events[i].data.u32].get())));

                if(epoll_events[i].data.u32 < 0) {
                    token_map.erase(epoll_events[i].data.u32);
                }
            }
        }
    }
  
    bool register_epoll(int fd, epoll_event &event, std::string runner) {
        //std::cout << "register_epoll from " + runner + "\n";
        int result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
        if(result == -1) {
          //  std::cout << "epoll_ctl() error: " << errno << std::endl;
            return false;
        }
        else {
            //std::cout << "REGISTERED: " << event.data.fd << std::endl;
            return true;
        }
    }

    void register_epoll_handler(std::unique_ptr<Token>&& callback, int id) {
        token_map[id] = std::move(callback);
    }

    void run_thread(Token *callback) {
        std::thread worker_thread(&Token::call, callback);
        worker_thread.detach();
    }

    void run_thread(std::unique_ptr<Token> callback) {
        std::thread worker_thread(&Token::call, callback);
        worker_thread.detach();
    }

    void enqueue_callback(std::unique_ptr<Token>&& callback) { 
        queue.push(std::move(callback));
    }

    void enqueue_callback(Token &&callback) {

    }

private:
    inline static bool epoll_running = false;
    int epoll_fd = 0;
    epoll_event epoll_events[EPOLL_COUNT];
    std::map<signed int, std::unique_ptr<Token>> token_map;

    thread_queue<std::unique_ptr<Token>> queue = {};


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
