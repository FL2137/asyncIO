#pragma once

#include <any>
#include <functional>
#include <memory>
#include <iostream>
#include "executor.hpp"

namespace asyncio {

    using ReadCallback = std::function<void(const asyncio::error&, int)>; 
    using WriteCallback = std::function<void(const asyncio::error&, int)>; 
    using AcceptCallback = std::function<void(const asyncio::error&)>;

// these are basically functors that take a lambda and parameters as members.
// they are meant to be derived from. ReadTask and WriteTask have simple read and write operations already implemented

class Task {
public:

    Task() {}

    Task(const Task& other) {
        m_name = other.m_name;
        m_completion_handler = other.m_completion_handler;
        completed = other.completed;
    }

    Task(std::function<void(void)> job) : m_completion_handler(job) {}

    virtual void operator()() {
        m_completion_handler();
    }

    std::string m_name = "";

    bool completed = false;

    std::function<void(void)> m_completion_handler = []() {
    };

};

class ReadTask : public Task {
public:

    ReadTask(ReadCallback completion_handler) {
        this->m_completion_handler = completion_handler;
    }

    ReadTask(ReadCallback completion_handler, asyncio::error error, int nbytes) {
        this->m_completion_handler = completion_handler;
        m_error = error;
        m_nbytes = nbytes;
    }

    void set_data(asyncio::error _err, int _nbytes) {
        m_error = _err;
        m_nbytes = _nbytes;
    }

    void operator()() {



        m_completion_handler(m_error, m_nbytes);
    }

private:
    asyncio::error m_error;
    int m_nbytes;
    ReadCallback m_completion_handler;
};


class WriteTask : public Task {
public:


    typedef std::function<void(char *, int , WriteCallback)> write_impl;

    WriteTask(WriteCallback completion_handler) {
        m_completion_handler = completion_handler;
    }

    WriteTask(char* buffer, int size, WriteCallback completion_handler, void *write_impl) {
        using writefoo = void(*)(char*, int, WriteCallback);
        writefoo impl = (writefoo)write_impl;
        impl(buffer, size, completion_handler);
    }

    void set_data(asyncio::error _error, int _nbytes) {
        m_error = _error;
        m_nbytes = _nbytes;
    }

    void call() {
        m_completion_handler(m_error, m_nbytes);
    }

private:
    asyncio::error m_error;
    int m_nbytes;
    WriteCallback m_completion_handler;
    int write_size;
    char *write_buffer; 
};


class AcceptTask : public Task {

public:
    AcceptTask() = default;

    AcceptTask(AcceptCallback completion_handler) {
        this->m_completion_handler = completion_handler;
    }
    void set_data(asyncio::error _error) {
        this->m_error = _error;
    }

    void call() {
        m_completion_handler(m_error);
    }

    AcceptCallback m_completion_handler;
private:
    asyncio::error m_error;
};

}