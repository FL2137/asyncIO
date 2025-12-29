#ifndef ACCEPTOR_HPP
#define ACCEPTOR_HPP

#include "executor.hpp"
#include "endpoint.hpp"
#include "error.hpp"
#include "socket.hpp"
#include "epoll_wrapper.hpp"
#include <cstdlib>
#include <fcntl.h>

namespace asyncio {
namespace tcp {

    class acceptor {
    public:
        acceptor(asyncio::executor &exec, const tcp::endpoint &local_endpoint, bool register_flag = false) : executor(exec) {

            fd = ::socket(AF_INET, SOCK_STREAM, 0);

            if(fd == -1) {
                close(fd);
                return;
            }
            int options = 1;
            if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &options, sizeof(options))) {
                close(fd);
                return;
            }
            
            if(bind(fd, (struct sockaddr*)&local_endpoint.server_addr, sizeof(local_endpoint.server_addr)) != 0) {
                close(fd);
                return;
            }

            if(listen(fd, n_acceptable_connections) == -1) {
                close(fd);
                return;
            }
           
            if(register_flag) {
                epoll_accept_event.events = EPOLLIN | EPOLLET;
                epoll_accept_event.data.fd = fd;
                int id = fd;
                epoll_accept_event.data.u32 = id;
                
                m_epoll->register_event(fd, epoll_accept_event, "Acceptor::AcceptEvent");
                Task *impl_token = new Task();
                impl_token->m_completion_handler = std::bind(&acceptor::implementation, this);
                impl_token->m_name = "AcceptImpl";

                m_epoll->register_handler(impl_token, id);
            }

            std::cout << "Acceptor constructed.\n";
        }

        ~acceptor() {
            delete impl_callback;
            close(fd);
        }

        void async_accept(asyncio::tcp::socket &socket, AcceptCallback callback) {
            accept_callback = callback;
            this->tcp_socket = &socket;
        }

        void enqueue(Task *token) const {
            executor.enqueue_task(token);
        }

private:
        void implementation() {
            socklen_t socklen;
            sockaddr_in remote_endpoint;
            int newfd = accept(fd, (sockaddr*)&remote_endpoint, &socklen);
            asyncio::error error;
            if(newfd == -1) {
                error.set_error_message("Accept error: " + std::to_string(errno));
                close(newfd);
            }
            else {
                //set the socket descriptor to be nonblocking
                if(fcntl(newfd, F_SETFL, fcntl(newfd, F_GETFL) | O_NONBLOCK) == -1) {
                    //std::cout << "ERROR SETTING NONBLOCK ON SOCKET FD";
                }
                tcp_socket->setup(newfd);
            }            
            AcceptTask *at = new AcceptTask();
            at->m_name = "AcceptToken." + std::to_string(newfd);
            at->m_completion_handler = accept_callback;
            at->set_data(error);
            executor.enqueue_task(at);
        }

        void register_write(const WriteCallback& write_handler) {

        }

        void register_read(const ReadCallback& read_handler) {

        }

        void create_epoll_instance() {
            m_epoll = std::make_shared<epoll_wrapper>();            
            executor.run_epoll_instance(m_epoll);
        }

    public:
        int fd;
    private:
        Task *impl_callback;
        asyncio::tcp::endpoint local_endpoint;
        AcceptCallback accept_callback;
        epoll_event epoll_accept_event;
        asyncio::executor &executor;
        std::shared_ptr<epoll_wrapper> m_epoll;

        asyncio::tcp::socket *tcp_socket;
        int n_acceptable_connections = 4;
    };
}
}


#endif 