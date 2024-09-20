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

        while(runner) {
            process_events();
        }    
    }

public:

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


    void register_callback(int caller, Callback *callback) {
        callback_m[caller] = callback;
    }

    void run_thread(event event) {
        auto token = std::make_shared<Token>();
        token->event_name = event.name;
        event.set_token(token);
        //tokens[reinterpret_cast<int32_t>(&event)] = token;

        std::thread worker_thread(&event::run, event);
        worker_thread.detach();
    }

private:
    int epoll_fd = 0;
    epoll_event epoll_events[EPOLL_COUNT];
    std::vector<event> events;
    std::map<event, std::shared_ptr<Token>> tokens;

    std::map<int, Callback*> callback_m;


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
        int epoll_nfds = epoll_wait(epoll_fd, epoll_events, EPOLL_COUNT, -1);
        for(int i = 0; i < epoll_nfds; i++) {
            
            callback_m[i]->call();



        }


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
