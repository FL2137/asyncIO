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
                asyncio::error error;
                int result = read(fd,  read_buffer, read_size);
                if(result == -1) {
                    error.set_error_message("Read error");
                }

                ReadToken *rt = new ReadToken(read_callback);
                rt->set_data(error, result);
                rt->name = "ReadToken." + std::to_string(fd);
                executor.enqueue_callback(rt);
            }

            void write_impl() {
                asyncio::error error;
                int result = write(fd, write_buffer, write_size);
                if(result == -1) {
                    error.set_error_message("Write error");
                }

                WriteToken *wt = new WriteToken(write_callback);
                wt->set_data(error, result);
                wt->name = "WriteToken." + std::to_string(fd);
                executor.enqueue_callback(wt);
            }

        public:
            int fd = 0;    

            epoll_event epoll_read_event;
            epoll_event epoll_write_event;

            ReadCallback read_callback;
            WriteCallback write_callback;

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