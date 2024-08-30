#ifndef TESTER_HPP
#define TESTER_HPP

#include <string>
#include <functional>
#include <iostream>
#include "executor.hpp"
#include "event.hpp"

class Tester {
    public:
    Tester(asyncio::executor &exec) : exec(exec) {
        std::function<void(void)> handler = std::bind(&Tester::parent_call, this);
        asyncio::event ev("TEST_EVENT", handler);
        ev.priority = 2;
        asyncio::executor::register_event(exec, ev);
    }


    void parent_call() {

        std::cout << "Parent call\n";

        std::function<void(void)> child_call = [](){
            std::cout << "child call\n";
        };

        asyncio::event ev("TEST_PARENT_CALL", child_call);

        exec.register_event(ev);
    }

    asyncio::executor &exec;
};



#endif