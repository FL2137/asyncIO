#pragma once

#include <functional>
#include <string>
#include "error.hpp"
#include "callback.hpp"

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


class event {
public: 

    event(std::string event_name, Token event_handler, EVENT_TYPE event_type = EVENT_TYPE::SOCKET_IO) {
        this->name = event_name;
        this->callback = event_handler;
        this->type = event_type;
        if(event_type == SOCKET_IO) {
            priority = 1;
        }
        else if(event_type == SYSTEM_IO) {
            priority = 0;
        }
    }

    void set_data(asyncio::error error, int nbytes) {
        is_child = true;
        child_error = error;
        child_nbytes = nbytes;
    }

    void run() {
        status = WORKING;
        callback.call();
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

    Token callback;
    EVENT_TYPE type;

};

}