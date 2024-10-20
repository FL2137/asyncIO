#ifndef ACCEPTOR_HPP
#define ACCEPTOR_HPP

#include "executor.hpp"
#include "endpoint.hpp"
#include "error.hpp"
#include "socket.hpp"
#include <cstdlib>
#include <fcntl.h>

namespace asyncio {
namespace tcp {
    class acceptor {

    public:
        acceptor(asyncio::executor &exec, const tcp::endpoint &local_endpoint) : executor(exec) {
            fd = ::socket(AF_INET, SOCK_STREAM, 0);

            if(fd == -1) {
                close(fd);
                std::cout << "ACCEPTOR LISTEN 1\n";
                return;
            }
            int options = 1;
            if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &options, sizeof(options))) {
                close(fd);
                std::cout << "ACCEPTOR LISTEN 2\n";
                std::cout << errno << std::endl;

                return;
            }
            if(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) == -1) {
                std::cout << "ERROR SETTING NONBLOCK ON SOCKET FD";
            }   
            
            if(bind(fd, (struct sockaddr*)&local_endpoint.server_addr, sizeof(local_endpoint.server_addr)) != 0) {
                close(fd);
                return;
            }

            if(listen(fd, n_acceptable_connections) == -1) {
                close(fd);
                return;
            }
            std::cout << "Acceptor constructed\n";
           
            // int r = fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
            // if(r == -1) {
            //     std::cout << "ERROR SETTING NONBLOCK ON ACCPET SOCKET\n";
            // }

        }

        ~acceptor() {
            std::cout << "Acceptor destructor\n";
            delete impl_callback;
            close(fd);
        }

        void async_accept(tcp::socket &socket, AcceptCallback callback) {
            accept_callback = callback;
            this->tcp_socket = &socket;

            epoll_accept_event.events = EPOLLIN | EPOLLET;
            epoll_accept_event.data.fd = fd;
            int id = executor.reserve_id();
            epoll_accept_event.data.u32 = id;
            
            executor.register_epoll(fd, epoll_accept_event);
            std::cout << "CURRENT CALLBACK ID: " << id << std::endl;

            impl_callback = new Token();
            impl_callback->callback = std::bind(&acceptor::implementation, this);
            impl_callback->name = "AcceptImplem";

            executor.register_epoll_handler(impl_callback, id);
            
        }

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
                std::cout << "setting nonblockcity\n";
                //set the socket descriptor to be nonblocking
                if(fcntl(newfd, F_SETFL, fcntl(newfd, F_GETFL) | O_NONBLOCK) == -1) {
                    std::cout << "ERROR SETTING NONBLOCK ON SOCKET FD";
                }
                tcp_socket->setup(newfd);
            }            
            AcceptToken* at = new AcceptToken();
            at->name = "AcceptToken." + std::to_string(newfd);
            at->callback = accept_callback;
            at->set_data(error);
            executor.enqueue_callback(at);

        }

    private:
        Token *impl_callback;

        AcceptCallback accept_callback;
        epoll_event epoll_accept_event;
        asyncio::executor &executor;
        tcp::socket *tcp_socket;
        int fd;
        int n_acceptable_connections = 4;
    };
}
}


#endif 