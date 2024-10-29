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

    template<typename Acceptor, typename Socket> 
    void async_accept_one(const Acceptor &acceptor, Socket &socket, AcceptCallback callback) {
        Token *impl_token = new Token();
        impl_token->name = "accept_one_impl_token";
        impl_token->callback = [&, callback] {
            asyncio::error error;
            
            socklen_t socklen;
            sockaddr_in remote_endpoint;
            
            int result_fd = accept(acceptor.fd, (sockaddr*)&remote_endpoint, &socklen);

            if(result_fd == -1) {
                error.set_error_message("accept error: " + std::to_string(errno));
            }
            if(fcntl(result_fd, F_SETFL, fcntl(result_fd, F_GETFL) | O_NONBLOCK) == -1) {
                std::cout << "ERROR SETTING NONBLOCK ON SOCKET FD";
            }
            socket.setup(result_fd);

            AcceptToken *accept_token = new AcceptToken(callback);
            accept_token->name = "accept_one_token";
            accept_token->set_data(error);

            acceptor.enqueue(accept_token);
        };

        acceptor.enqueue(impl_token);
    }

    template<typename Reader>
    void async_read_some(const Reader& reader, char *buffer, int size, ReadCallback callback) {
        Token *impl_token = new Token();
        int sfd = reader.fd;
        impl_token->name = "read_impl_token";
        impl_token->callback = [&, sfd, buffer, size, callback] {
            asyncio::error error;
            int result = read(sfd, buffer, size);
            if(result == -1) {
                error.set_error_message("Read error: " + std::to_string(errno));
            }

            ReadToken *read_token = new ReadToken(callback);
            read_token->name = "ReadToken." + std::to_string(sfd);
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
        impl_token->callback = [&, sfd, buffer, size, callback] {
            asyncio::error error;
            int result = write(sfd, buffer, size);
            if(result == -1) {
                error.set_error_message("Wrtite error: " + std::to_string(errno));
            }

            auto *write_token = new WriteToken(callback);
            write_token->name = "WriteToken." + std::to_string(sfd);
            write_token->set_data(error, result);
            writer.enqueue(write_token);
        };

        writer.enqueue(impl_token);
    }


    void read_implementation(int fd, char *buffer, int size, const asyncio::tcp::socket &socket, ReadCallback completion_handler) {
        WriteToken* handler_token = new WriteToken(completion_handler);

    }

    void write_implementation(int fd, char *buffer, int size, const asyncio::tcp::socket &socket, WriteCallback completion_handler) {

    }

};