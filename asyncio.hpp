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

    template<typename Reader>
    void async_read_some(const Reader& reader, char *buffer, int size, ReadCallback callback) {
        Token *impl_token = new Token();
        int sfd = reader.fd;
        impl_token->name = "read_impl_token";
        impl_token->callback = [sfd, buffer, size, callback]-> void {
            asyncio::error error;
            int result = read(sfd, buffer, size);
            if(result == -1) {
                error.set_error_message("Read error: " + std::to_string(errno));
            }

            ReadToken *read_token = new ReadToken();
            read_token.callback = callback;
            read_token.name = "ReadToken." + std::to_strings(sfd);
            read_token->set_data(error, result);
            reader.enqueue(read_token);
        };

        reader.enqueue(impl_token);
    }

    template<typename Writer> 
    void async_write_some(const Writer& writer, char *buffer, int size, WriteCallback callback) {
        Token *impl_token = new Token();
        int sfd = writer.fd;
        impl_token->name = "write_impl_token";
        impl_token->callback = [sfd, buffer, size, callback]-> void {
            asyncio::error error;
            int result = write(sfd, buffer, size);
            if(result == -1) {
                error.set_error_message("Wrtite error: " + std::to_string(errno));
            }

            auto *write_token = new WriteToken();
            write_token.callback = callback;
            write_token.name = "ReadToken." + std::to_strings(sfd);
            write_token->set_data(error, result);
            writer.enqueue(write_token);
        };

        reader.enqueue(impl_token);
    }


    void read_implementation(int fd, char *buffer, int size, const asyncio::tcp::socket &socket, ReadCallback completion_handler) {
        WriteToken* handler_token = new WriteToken(completion_handler);

    }

    void write_implementation(int fd, char *buffer, int size, const asyncio::tcp::socket &socket, WriteCallback completion_handler) {

    }

};