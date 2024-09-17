#pragma once

#include <any>
#include <functional>
#include <memory>
#include <iostream>



namespace asyncio {
typedef std::function<void(asyncio::error, int)> IO_Signature; 

//template <typename ...Args>
class Callback {
public:
    Callback() {

    }
    
    virtual void call() {
        functor();
    }

    std::function<void(void)> functor = []() {
    };


private:
};

class ReadToken : public Callback {
public:

    ReadToken(IO_Signature completion_handler) {
        this->functor = completion_handler;
    }

    ReadToken(IO_Signature completion_handler, asyncio::error error, int nbytes) {
        this->functor = completion_handler;
        m_error = error;
        m_nbytes = nbytes;
    }

    void set_data(asyncio::error _err, int _nbytes) {
        m_error = _err;
        m_nbytes = _nbytes;
    }

    void call() override {
        functor(m_error, m_nbytes);
    }

private:
    asyncio::error m_error;
    int m_nbytes;
    std::function<void(asyncio::error, int)> functor;
};


class WriteToken : public Callback {
    WriteToken(IO_Signature completion_handler) {

    }

    IO_Signature functor;
};
    
void runXd() {  
    std::vector<Callback*> vec = {};
    ReadToken rt(
    [](asyncio::error error, int nbytes)
        {
            std::cout << "Derived class FNCTOR\n";
        }, asyncio::error(), 15);
}


}