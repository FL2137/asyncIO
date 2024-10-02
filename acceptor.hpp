#ifndef ACCEPTOR_HPP
#define ACCEPTOR_HPP

#include "executor.hpp"
#include "endpoint.hpp"
#include "error.hpp"
#include "event.hpp"
#include "socket.hpp"
#include <cstdlib>
#include <fcntl.h>

namespace asyncio {
namespace tcp {
    class acceptor {
        typedef std::function<void(asyncio::error, int)> AsyncCallback;

    public:
        acceptor(asyncio::executor &exec, const tcp::endpoint &local_endpoint) : executor(exec) {
            fd = ::socket(AF_INET, SOCK_STREAM, 0);
            std::cout << "ACCEPTOR LISTEN START\n";

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
            
            if(bind(fd, (struct sockaddr*)&local_endpoint.server_addr, sizeof(local_endpoint.server_addr)) != 0) {
                close(fd);
                return;
            }

            if(listen(fd, n_acceptable_connections) == -1) {
                close(fd);
                return;
            }
            epoll_event ev;
            ev.events = EPOLLIN;
            ev.data.fd = fd;
        }

        ~acceptor() {
            std::cout << "Acceptor destructor\n";
            close(fd);
        }

        void async_accept(tcp::socket &socket, Accept_Signature callback) {
            accept_token.functor = callback;
            this->tcp_socket = &socket;
        }


        void implementation() {
            socklen_t socklen;
            sockaddr_in remote_endpoint;
            int newfd = accept(fd, (sockaddr*)&remote_endpoint, &socklen);
            if(newfd == -1) {
                asyncio::error error;
                close(newfd);
                return;
            }
            std::cout << "Accepted! \n";
            fcntl(newfd, F_SETFL, fcntl(newfd, F_GETFL) | O_NONBLOCK); //set the socket descriptor to be nonblocking
            tcp_socket->fd = newfd;
        }

    private:
        Accept_Signature accept_callback;
        AcceptToken accept_token;
        asyncio::executor &executor;
        tcp::socket *tcp_socket;
        int fd;
        int n_acceptable_connections = 4;
    };
}
}


#endif 