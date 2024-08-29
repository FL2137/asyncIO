#pragma once

#include <functional>
#include <string>
#include "error.hpp"

namespace asyncio {

enum EVENT_STATUS {
    FINISHED = 0,
    PENDING,
    WORKING
};

enum EVENT_TYPE {
    SYSTEM_IO = 0,
    SOCKET_IO,
};

class event {
public: 
    typedef std::function<void(asyncio::error, int)> AsyncCallback;

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

    operator bool() const{
        if(status == PENDING)
            return true;
        return false;
    }
public:

    int priority;


private:
    AsyncCallback handler;
    EVENT_STATUS status;
    EVENT_TYPE type;
    std::string name;
};

}