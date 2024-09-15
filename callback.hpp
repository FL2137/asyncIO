#pragma once

#include <any>
#include <functional>
#include <memory>



namespace asyncio {

//template <typename ...Args>
class Callback {
public:
    Callback();
    
    virtual void call() {
    }

    std::function<void(void)> functor;
};

template<typename ...Args>
class ReadToken : Callback {
public:

    ReadToken() {

    }

    void call() override {
        functor();
    }

    

    std::function<void(Args ...args)> functor;
};

void runXd() {  
    std::vector<Callback> vec = {};
    ReadToken<int, char> rt;

}


}