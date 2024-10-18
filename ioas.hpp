#pragma once

#include "error.hpp"
#include "socket.hpp"
#include "acceptor.hpp"
#include "executor.hpp"


namespace asyncio {

    void async_write(const asyncio::tcp::socket &socket, char *buffer, int size, WriteCallback callback) {
        WriteToken *token = new WriteToken(callback);
        
        token->name = "write_agent";


        socket.enqueue(token);
    }

    void async_read(const asyncio::tcp::socket &socket, char *buffer, int size, ReadCallback callback) {

    }

    template <typename reader> 
    void ReadAgent() {

    }

    void read_implementation(int fd, char *buffer, int size) {

    }

    void write_implementation(int fd, char *buffer, int size) {

    }

};