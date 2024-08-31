#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

#include <deque>
#include <functional>
#include "error.hpp"
#include <iostream>
#include "event.hpp"
#include <thread>


namespace asyncio {

class executor {

    typedef std::function<void(void)> Callback;

    void event_loop() {
        while(1) {
            process_events();
            sleep(1);
        }    
    }


public:
    void run() {
        event_loop();
    }

    static void register_event(executor &exec, event event) {
        exec.events.push_back(event);
    }

    void register_event(event event) {
        events.push_back(event);
    }

    void run_thread(event event) {
        std::thread worker_thread(&event::run, event);

        worker_thread.detach();
    }

    void run_thread(Callback foo) {
        auto lamb = []() {
            std::cout << "peepee\n";
        };

        std::thread tr(lamb);   
        tr.detach();
        sleep(1);
        if(!tr.joinable()) {
            std::cout << "its detached u moron\n";
        }
    }

private:
    std::deque<event> events;
    
    void process_events() {
        std::sort(events.begin(), events.end(), [](const event& lhs, const event& rhs) {
            if(lhs.priority >= rhs.priority)
                return true;
            else return false;
        });

        // while(events.size()) {
        //     std::cout << "Running new event...\n";
        //     run_thread(events[0]);
        //     //events.pop_front();
        // }
        for(auto event : events) {
            if(event.status == PENDING) {
                run_thread(event);
            }
        } 


    }

};
}
#endif
