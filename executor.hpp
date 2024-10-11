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

#define EPOLL_COUNT 50

namespace asyncio {

class executor {

    void event_loop() {
        int i = 0;
        while(runner) {
            process_events();
            i++;
        }    
    }

public:

    inline static int callback_id = 0;

    executor() {

        signal(SIGINT, [](int){
            executor::runner = false;});
     
        epoll_fd = epoll_create(EPOLL_COUNT);
    }

    inline static bool runner = true;

    void run() {
        event_loop();
    }

    void catch_signal(int dummy) {

    }

    static void register_event(executor &exec, event event) {
        exec.events.push_back(event);
    }

    void register_event(event event) {
        events.push_back(event);
    }

    void register_epoll(int fd, epoll_event &event) {
        int result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
        if(result == -1 )
            std::cout << "epoll_ctl() error: " << errno << std::endl;
        else
            std::cout << "REGISTERED: " << event.data.fd << std::endl;
    }

    void register_epoll(epoll_event *event) {
        int result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event->data.fd, event);
        if(result == -1 ){
            std::cout << "epoll_ctl() error\n";
        }
        else
            std::cout << "REGISTERED: " << event->data.fd << std::endl;
    }

    void register_callback(Callback *callback) {
        callback_map[executor::callback_id] = callback;
        executor::callback_id++;
    }

    void run_thread(event event) {
        auto token = std::make_shared<Token>();
        token->event_name = event.name;
        event.set_token(token);
        //tokens[reinterpret_cast<int32_t>(&event)] = token;
        std::thread worker_thread(&event::run, event);
        worker_thread.detach();
    }

    void run_thread(Callback *callback) {
        std::thread worker_thread(&Callback::call, callback);
        worker_thread.detach();
    }

    void enqueue_callback(Callback *callback) { 
        this->queue.push_back(callback);
    }

private:
    int epoll_fd = 0;
    epoll_event epoll_events[EPOLL_COUNT];
    std::vector<event> events;
    std::map<event, std::shared_ptr<Token>> tokens;
    std::map<signed int, Callback*> callback_map;

    std::vector<Callback*> queue = {};
    
    void process_events() {
        // std::sort(events.begin(), events.end(), [](const event& lhs, const event& rhs) {
        //     if(lhs.priority >= rhs.priority)
        //         return true;
        //     else return false;
        // });

        // while(events.size()) {
        //     std::cout << "Running new event...\n";
        //     run_thread(events[0]);
        //     //events.pop_front();
        // }
        // for(auto &event : events) {
        //     if(event.status == PENDING) {
        //         event.status = WORKING;
        //         std::cout << event.name <<std::endl;
        //         run_thread(event);
        //     }
        //     else {
        //         std::cout << event.name << " " << events.size() << std::endl;
        //     }
        // } 

        //epoll events
        // std::cout << "epoll waiting....\n";
        // if(epoll_nfds == -1) {
        //     std::cout << "epoll_wait() error: " << errno << std::endl;
        // }
        // else {
        //     std::cout << "EVENT COUNT: " << epoll_nfds << std::endl;
        //     std::cout << epoll_events[0].data.fd 
        //               << " -- " << epoll_events[0].data.u32 << std::endl;
        // }
        // std::cout << "epoll started\n";
        
        
        int epoll_nfds = epoll_wait(epoll_fd, epoll_events, EPOLL_COUNT, 5);

        for(int i = 0; i < epoll_nfds; i++) {
            //run_thread(callback_map[epoll_events[i].data.u32]);
            //instead of running ^^ register the callback to happen
            queue.push_back(callback_map[epoll_events[i].data.u32]);
        }



        for(auto callb : queue) {
            callb->call();
        }
    }

    void run_epoll() {

    }


    mutable std::shared_mutex token_mutex;

    void process_tokens() {
        std::unique_lock lock(token_mutex);
        for(auto &token : tokens) {
            if(token.second->completed) {
                auto iter = std::find(events.begin(), events.end(), token.first);
                iter->status = COMPLETED;

            }
        }
    }
};
}
#endif
