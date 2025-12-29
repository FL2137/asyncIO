#pragma once

#include <unistd.h>
#include <signal.h>
#include <sys/epoll.h>

#include <istream>
#include <memory>
#include <functional>
#include <map>

#include "callback.hpp"

namespace asyncio {

class epoll_wrapper : public std::enable_shared_from_this<epoll_wrapper> {

public:


    epoll_wrapper() {
        m_fd = epoll_create(EPOLL_COUNT1);
    }

    void rearm(int fd, epoll_event event) {

    }

    void work() {
        // epoll_wait and stuff
    }

    void run() {

    }


    void register_handler(Task* handler, int handler_id) {
        m_handler_map[handler_id] = handler;
    }

    epoll_event& create_read_event(int fd, int id) {

    }

    std::shared_ptr<epoll_wrapper> get() {
        return shared_from_this();
    }

    bool register_event(int event_fd, epoll_event &event, std::string runner) {
        int result = epoll_ctl(m_fd, EPOLL_CTL_ADD, event_fd, &event);

        if (result == -1) 
            return false;
        else 
            return true;
    }

private:

    std::map<int, Task*> m_handler_map;
    int m_fd;

    const int EPOLL_COUNT1 = 50;
};

}