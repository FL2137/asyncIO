#pragma once

#include <functional>
#include <string>
#include "error.hpp"

namespace asyncio {

enum EVENT_STATUS {
    COMPLETED = 0,
    PENDING,
    WORKING
};

enum EVENT_TYPE {
    SYSTEM_IO = 0,
    SOCKET_IO,
};


template <typename T>
class promise {
public:
    promise() {

    }

    T result;
};

class Token {
public:
    static int id;
    public:
    Token(){}
    bool completed = false;
    std::string event_name;
    char* buffer;
    int buffer_size;

};


class event {
public: 
    typedef std::function<void(asyncio::error, int)> AsyncCallback;
    typedef std::function<void(void)> Callback;

    event(std::string event_name, AsyncCallback event_handler, EVENT_TYPE event_type = EVENT_TYPE::SOCKET_IO) {
        this->name = event_name;
        this->handler = event_handler;
        this->type = event_type;
        if(event_type == SOCKET_IO) {
            priority = 1;
        }
        else if(event_type == SYSTEM_IO) {
            priority = 0;
        }
    }

    event(std::string event_name, Callback empty_callback, EVENT_TYPE event_type = EVENT_TYPE::SYSTEM_IO) {
        this->name = event_name;
        this->call = empty_callback;
        this->type = event_type;
    }

    void set_data(asyncio::error error, int nbytes) {
        is_child = true;
        child_error = error;
        child_nbytes = nbytes;
    }

    void run() {
        status = WORKING;
        if(type == SYSTEM_IO) {
            call();
        }
        else if(type == SOCKET_IO) {
            handler(child_error, child_nbytes);
        }
    }

    operator bool() const{
        if(status == PENDING)
            return true;
        return false;
    }
    
    void set_token(std::shared_ptr<Token> new_token) {
        this->token = token;
    }

    std::shared_ptr<Token> token;

public:

    int priority;
    EVENT_STATUS status = PENDING;
    std::string name;


private:
    bool is_child = false;
    asyncio::error child_error;
    int child_nbytes = -1;

    AsyncCallback handler;
    
    Callback call;
    EVENT_TYPE type;

};

}