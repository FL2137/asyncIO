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

#define EPOLL_COUNT 50

namespace asyncio {

class executor {

    typedef std::function<void(void)> Callback;

    void event_loop() {
        signal(SIGINT, [](int){
            std::cout << "LOL\n";
            executor::runner = false;});

        while(runner) {
            process_events();
        }    
    }

public:

    executor() {
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




        for(auto &event : events) {
            if(event.status == PENDING) {
                event.status = WORKING;
                std::cout << event.name <<std::endl;
                run_thread(event);
            }
            else {
                std::cout << event.name << " " << events.size() << std::endl;
            }
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
