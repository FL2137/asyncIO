#pragma once

#include "error.hpp"
#include "socket.hpp"
#include "acceptor.hpp"
#include "executor.hpp"


namespace asyncio {

    void async_write(const asyncio::tcp::socket &socket, char *buffer, int size, WriteCallback callback) {


        // int r = write(socket.fd, buffer, size);
        // if(r == -1) {
        //     std::cout << "error writing\n";
        //     return;
        // }

        // std::cout << "lol ok\n";

        Token *impl_token = new Token();
        int sfd = socket.fd;


        impl_token->name = "write_impl_token";

        impl_token->callback = [&, sfd, buffer, size, callback]() {

            asyncio::error error;

            int result = write(sfd, buffer, size);
            if(result == -1) {
                error.set_error_message("Write error: " + std::to_string(errno));
            }

            WriteToken *write_token = new WriteToken(callback);
            write_token->name = "write_token";
            write_token->set_data(error, result);
            socket.enqueue(write_token);
        };

        socket.enqueue(impl_token);
    }

    void async_read(const asyncio::tcp::socket &socket, char *buffer, int size, ReadCallback callback) {

    }

    template <typename reader> 
    void ReadAgent() {

    }

    void read_implementation(int fd, char *buffer, int size, const asyncio::tcp::socket &socket, ReadCallback completion_handler) {
        WriteToken* handler_token = new WriteToken(completion_handler);

    }

    void write_implementation(int fd, char *buffer, int size, const asyncio::tcp::socket &socket, WriteCallback completion_handler) {

    }

};