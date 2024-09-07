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

            void async_connect(tcp::endpoint endpoint, AsyncCallback callback) {
                //event ev("ASYNC_CONNECT_EVENT", callback);
                //ev.priority;
                //ev.set_child()
                //executor::register_event(executor, ev);
            }

            void async_read_some(char *buffer, int size, AsyncCallback callback) {
                int result = read(file_descriptor, buffer, size);
                asyncio::error error;
                if(result == -1) {
                    error.set_error_message("Socket read error");
                }
                asyncio::event event("ASYNC_READ", callback, EVENT_TYPE::SOCKET_IO);
                event.set_data(error, result);
                executor.register_event(event);
            }

            void async_write_some(char *buffer, int size, AsyncCallback callback) {
                int result = write(file_descriptor, buffer, strlen(buffer));
                asyncio::error error;
                if(result == -1) {
                    error.set_error_message("write() Error writing to socket");
                }
                asyncio::event event("ASYNC_WRITE", callback, EVENT_TYPE::SOCKET_IO);
                event.set_data(error, result);
                executor.register_event(event);
            }

        public:
            int file_descriptor = 0;    
        private:
            asyncio::executor &executor;
            tcp::endpoint ep;
        };

    }
}



#endif 