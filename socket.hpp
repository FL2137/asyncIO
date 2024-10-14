#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "error.hpp"
#include "endpoint.hpp"
#include <functional>
#include "executor.hpp"


namespace asyncio {
     namespace tcp {

        class socket {
            typedef std::function<void(asyncio::error, int)> AsyncCallback;
        public:

            socket(asyncio::executor &exec): executor(exec) {
            }

            ~socket() {
                close(fd);
            }

            void async_read_some(char *buffer, int size, ReadCallback callback) {
            }

            void async_write_some(char *buffer, int size, WriteCallback callback) {
            }   

            void read_impl() {

            }

            void write_impl() {
            }

        public:
            int fd = 0;    

            epoll_event epoll_read_event;
            epoll_event epoll_write_event;

            std::shared_ptr<WriteToken> write_token;
            std::shared_ptr<ReadToken> read_token;

            std::unique_ptr<Token> read_callback;
            std::unique_ptr<Token> write_callback;

        private:
            asyncio::executor &executor;
            tcp::endpoint ep;
            char *read_buffer;
            char *write_buffer;
            int read_size;
            int write_size;
        };

    }
};

#endif 