#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "error.hpp"
#include "endpoint.hpp"
#include <functional>
#include "executor.hpp"
#include <sys/ioctl.h>
#include "epoll_wrapper.hpp"

namespace asyncio {
     namespace tcp {
        class socket {
           using AsyncCallback = std::function<void(asyncio::error, int)>;
        public:

            socket(asyncio::executor &exec): executor(exec) {
                // m_epoll = exec.get_epoll();
            }

            ~socket() {
                close(fd);
            }

            //not updated
            void async_read_some(char *buffer, int size, ReadCallback callback) {
                this->read_callback = callback;
                this->read_buffer = buffer;
                this->read_size = size;

                if(m_epoll->register_event(fd, epoll_read_event, "") == false ) {
                    if(errno == EEXIST) {
                        m_epoll->rearm(fd, epoll_read_event);
                        return;
                    }
                }

                epoll_read_event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                epoll_read_event.data.fd = fd;
                epoll_read_event.data.ptr = read_buffer;
                int id = fd;
                epoll_read_event.data.u32 = id;

                Task *impl = new Task();
                impl->m_completion_handler = std::bind(&socket::read_impl, this);
                impl->m_name = "ReadImpl." + std::to_string(fd);

                m_epoll->register_event(fd, epoll_read_event, "oneshot async_read_some");
                m_epoll->register_handler(impl, id);
            }

            void async_read(char *buffer, int size, ReadCallback callback) {
                this->read_buffer = buffer;
                this->read_size = size;
                this->read_callback = callback;
                
                Task *impl = new Task();

                impl->m_completion_handler = std::bind(&socket::read_impl, this);
                impl->m_name = "ReadImpl." + std::to_string(fd);

                
                epoll_read_event.events = EPOLLIN | EPOLLET;
                epoll_read_event.data.fd = fd;
                epoll_read_event.data.u32 = fd;


                m_epoll->register_event(fd, epoll_read_event, "async_read");
                m_epoll->register_handler(impl, fd);
            }

            void setup(int fd) {
                this->fd = fd;
                epoll_read_event.events = EPOLLIN | EPOLLET;
                epoll_read_event.data.fd = fd;
                epoll_read_event.data.ptr = read_buffer;
                int id = fd;
                epoll_read_event.data.u32 = id;
                
                
                // executor.register_epoll(id, epoll_read_event, "socket setup");
                // Task *t = new Task();
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

                    m_epoll->rearm(fd, epoll_write_event);
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

                m_epoll->register_event(fd, epoll_write_event, "socket async_write_some");

                Task *impl = new Task();
                impl->m_completion_handler = std::bind(&socket::write_impl, this);
                impl->m_name = "WriteImpl." + std::to_string(fd);
                
                m_epoll->register_handler(impl, id);
            }   

            void enqueue(Task* callback) const {
                executor.enqueue_task(callback);
            }

        private:
            void read_impl() {
                asyncio::error error;
                int result = read(fd,  read_buffer, read_size);
                if(result == -1) {
                    error.set_error_message("Read error");
                }
                ReadTask *rt = new ReadTask(read_callback);
                rt->set_data(error, result);
                rt->m_name = "ReadTask." + std::to_string(fd);
                executor.enqueue_task(rt);
            }

            void write_impl() {
                asyncio::error error;
                int result = write(fd, write_buffer, write_size);
                if(result == -1) {
                    error.set_error_message("Write error");
                }
                WriteTask *wt = new WriteTask(write_callback);
                wt->set_data(error, result);
                wt->m_name = "WriteTask." + std::to_string(fd);
                executor.enqueue_task(wt);
            }

            

        public:
            int fd = -1;    

            epoll_event epoll_read_event;
            epoll_event epoll_write_event;

            ReadCallback read_callback;
            WriteCallback write_callback;

        private:
            asyncio::executor &executor;
            std::shared_ptr<asyncio::epoll_wrapper> m_epoll;

            tcp::endpoint ep;
            char *read_buffer;
            char *write_buffer;
            int read_size;
            int write_size;
        };
    }
};

#endif 