#ifndef TESTER_HPP
#define TESTER_HPP

#include <string>
#include <functional>
#include <iostream>
#include <thread>
#include "executor.hpp"
#include "event.hpp"
#include <math.h>
#include <time.h>

class Tester {
    public:
    Tester(asyncio::executor &exec) : exec(exec) {
        std::function<void(void)> handler = std::bind(&Tester::parent_call, this);
        asyncio::event ev("TEST_PARENT_EVENT", handler, asyncio::SYSTEM_IO);
        asyncio::executor::register_event(exec, ev);
        std::cout << "Registered parent event \n";
    }


    void parent_call() {
        std::cout << "Running parent event \n";

        srand(time(NULL));
        int sum = 0;
        for(int i = 0; i < 10; i++) {
            sum += rand() % 15;
            std::cout << "PARENT EVENT: " << i+1 << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        std::function<void(asyncio::error, int)> child_call = [](asyncio::error error, int nbytes){
            std::cout << "CHILD: PARAM-" << nbytes << std::endl;
        };
        asyncio::event ev("TEST_CHILD_CALL", child_call);
        asyncio::error er;

        ev.set_data(er, sum);

        exec.register_event(ev);
    }

    asyncio::executor &exec;
};



#endif