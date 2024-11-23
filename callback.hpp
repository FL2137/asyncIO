#pragma once

#include <any>
#include <functional>
#include <memory>
#include <iostream>
#include "executor.hpp"

namespace asyncio {
typedef std::function<void(const asyncio::error&, int)> ReadCallback; 
typedef std::function<void(const asyncio::error&, int)> WriteCallback; 
typedef std::function<void(const asyncio::error&)> AcceptCallback;

//template <typename ...Args>

class Token {
public:
    Token() {

    }
    virtual void call() {
        callback();
    }

    std::string name = "";

    bool completed = false;

    std::function<void(void)> callback = []() {
    };

private:
};

class ReadToken : public Token {
public:

    ReadToken(ReadCallback completion_handler) {
        this->callback = completion_handler;
    }

    ReadToken(ReadCallback completion_handler, asyncio::error error, int nbytes) {
        this->callback = completion_handler;
        m_error = error;
        m_nbytes = nbytes;
    }

    void set_data(asyncio::error _err, int _nbytes) {
        m_error = _err;
        m_nbytes = _nbytes;
    }

    void call() {
        callback(m_error, m_nbytes);
    }

private:
    asyncio::error m_error;
    int m_nbytes;
    std::function<void(asyncio::error, int)> callback;
};


class WriteToken : public Token {
public:
    typedef std::function<void(char *, int , WriteCallback)> write_impl;

    WriteToken(WriteCallback completion_handler) {
        callback = completion_handler;
    }

    WriteToken(char* buffer, int size, WriteCallback completion_handler, void *write_impl) {
        using writefoo = void(*)(char*, int, WriteCallback);
        writefoo impl = (writefoo)write_impl;
        impl(buffer, size, completion_handler);
    }


    void set_data(asyncio::error _error, int _nbytes) {
        m_error = _error;
        m_nbytes = _nbytes;
    }

    void call() {
        callback(m_error, m_nbytes);
    }

private:
    asyncio::error m_error;
    int m_nbytes;
    WriteCallback callback;
    int write_size;
    char *write_buffer; 
};


class AcceptToken : public Token {

public:

    AcceptToken() {}

    AcceptToken(AcceptCallback completion_handler) {
        this->callback = completion_handler;
    }
    void set_data(asyncio::error _error) {
        this->m_error = _error;
    }

    void call() {
        callback(m_error);
    }

    AcceptCallback callback;
private:
    asyncio::error m_error;
};

}