#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "error.hpp"
#include "endpoint.hpp"
#include <functional>
#include "executor.hpp"
#include <sys/ioctl.h>


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

            //not updated
            void async_read_some(char *buffer, int size, ReadCallback callback) {
                this->read_callback = callback;
                this->read_buffer = buffer;
                this->read_size = size;

                if(executor.register_epoll(fd, epoll_read_event, "") == false ) {
                    if(errno == EEXIST) {
                        executor.epoll_rearm(fd, epoll_read_event);
                        return;
                    }
                }


                epoll_read_event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                epoll_read_event.data.fd = fd;
                epoll_read_event.data.ptr = read_buffer;
                int id = fd;
                epoll_read_event.data.u32 = id;

                Token *impl = new Token();
                impl->callback = std::bind(&socket::read_impl, this);
                impl->name = "ReadImpl." + std::to_string(fd);

                executor.register_epoll_handler(impl, id);
                executor.register_epoll(fd, epoll_read_event, "oneshot async_read_some");

            }

            void async_read(char *buffer, int size, ReadCallback callback) {
                this->read_buffer = buffer;
                this->read_size = size;
                this->read_callback = callback;
                
                Token *impl = new Token();

                impl->callback = std::bind(&socket::read_impl, this);
                impl->name = "ReadImpl." + std::to_string(fd);

                
                epoll_read_event.events = EPOLLIN | EPOLLET;
                epoll_read_event.data.fd = fd;
                epoll_read_event.data.u32 = fd;

                executor.register_epoll(fd, epoll_read_event, "async_read");


                executor.register_epoll_handler(impl, fd);
            }

            void setup(int fd) {
                this->fd = fd;
                std::cout << "SOCKET SETUP FD: " << fd << "\n";
                epoll_read_event.events = EPOLLIN | EPOLLET;
                epoll_read_event.data.fd = fd;
                epoll_read_event.data.ptr = read_buffer;
                int id = fd;
                epoll_read_event.data.u32 = id;
                
                
                // executor.register_epoll(id, epoll_read_event, "socket setup");
                // Token *t = new Token();
                // t->name = "temp read handler";
                // t->callback = []() {
                //     std::cout << "ODCZYTANIE\n";
                // };
                // executor.register_epoll_handler(t, id);

            }

            void async_write_some(char *buffer, int size, WriteCallback callback) {
                //rearm oneshot EPOLLOUT event
                if(write_callback) {
                    
                    epoll_write_event.events = EPOLLONESHOT | EPOLLOUT | EPOLLET; 
                    epoll_write_event.data.fd = fd;
                    epoll_write_event.data.ptr = write_buffer;

                    executor.epoll_rearm(fd, epoll_write_event);
                    return;
                }

                this->write_callback = callback;
                this->write_buffer = buffer;
                this->write_size = size;

                epoll_write_event.events = EPOLLONESHOT | EPOLLOUT | EPOLLET;
                epoll_write_event.data.fd = fd;
                epoll_write_event.data.ptr = write_buffer;
                int id = -fd;
                epoll_write_event.data.u32 = id;

                executor.register_epoll(fd, epoll_write_event, "socket async_write_some");

                Token *impl = new Token();
                impl->callback = std::bind(&socket::write_impl, this);
                impl->name = "WriteImpl." + std::to_string(fd);
                
                executor.register_epoll_handler(impl, id);
            }   

            void enqueue(Token* callback) const {
                executor.enqueue_callback(callback);
            }

        private:
            void read_impl() {
                std::cout << "READ IMPLE CALLED\n";
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
            int fd = -1;    

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