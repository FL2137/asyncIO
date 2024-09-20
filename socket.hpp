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
                close(file_descriptor);
            }

            void async_read_some(char *buffer, int size, AsyncCallback callback) {
                read_buffer = buffer;
                read_size = size;
                ReadToken *read_token = new ReadToken(callback, asyncio::error(), 0);
                executor.register_callback(file_descriptor, read_token);
            }

            void async_write_some(char *buffer, int size, AsyncCallback callback) {
                write_buffer = buffer;
                write_size = size;
                WriteToken *write_token = new WriteToken(callback);
                executor.register_callback(file_descriptor, write_token);
            }   


            void implementation(int socket_fd, bool read_flag = true) {
                if(read_flag) {
                    int result = read(socket_fd, read_buffer, read_size);
                    if(result == -1) {
                        asyncio::error error;
                        error.set_error_message("ERROR READING ON SOCKET");


                    }
                }
                else { //write
                    int result = write(socket_fd, read_buffer, read_size);
                    if(result == -1) {
                        asyncio::error error;
                        error.set_error_message("ERROR WRITING TO A SOCKET");
                    }
                }
            }

        public:
            int file_descriptor = 0;    

            epoll_event epoll_read_event;
            epoll_event epoll_write_event;



            std::shared_ptr<void> read_callback;

        private:
            asyncio::executor &executor;
            tcp::endpoint ep;
            char *read_buffer;
            char *write_buffer;
            int read_size;
            int write_size;
        };

    }
}



#endif 