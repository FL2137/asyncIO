#ifndef ACCEPTOR_HPP
#define ACCEPTOR_HPP

#include "executor.hpp"
#include "endpoint.hpp"
#include "error.hpp"
#include "event.hpp"
#include "socket.hpp"

#include <fcntl.h>

namespace asyncio {
namespace tcp {
    class acceptor {
        typedef std::function<void(asyncio::error, int)> AsyncCallback;

    public:
        acceptor(asyncio::executor &exec, const tcp::endpoint &local_endpoint) : executor(exec) {
            fd = ::socket(AF_INET, SOCK_STREAM, 0);
            event error_event("ERROR_EVENT", error::error_callback, SOCKET_IO);
            error sock_error;
            std::cout << "ACCEPTOR LISTEN START\n";

            if(fd == -1) {
                sock_error.set_error_message("Error occurred while creating the socket");
                error_event.set_data(sock_error, -1);
                executor.register_event(error_event);
                close(fd);
                std::cout << "ACCEPTOR LISTEN 1\n";
                return;
            }
            int options = 1;
            if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &options, sizeof(options))) {
                sock_error.set_error_message("setsockopt(): Error occurred while setting socket options");
                error_event.set_data(sock_error, -1);
                executor.register_event(error_event);
                close(fd);
                std::cout << "ACCEPTOR LISTEN 2\n";
                std::cout << errno << std::endl;

                return;
            }

            
            struct sockaddr_in sockadd;
            bzero((char*)&sockadd, sizeof(sockadd));

            //sockadd.sin_addr.s_addr = INADDR_ANY;
            //inet_pton(AF_INET, "127.0.0.1", &sockadd.sin_addr);
            sockadd.sin_family = AF_INET;
            sockadd.sin_addr.s_addr = INADDR_ANY;
            int portno = 5001;
            sockadd.sin_port = htons(portno);
            

            if(bind(fd, (struct sockaddr*)&sockadd, sizeof(sockadd)) != 0) {
                sock_error.set_error_message("bind(): Error binding address to the socket");
                error_event.set_data(sock_error, -1);
                executor.register_event(error_event);
                close(fd);
                return;
            }

            if(listen(fd, n_acceptable_connections) == -1) {
                sock_error.set_error_message("listen(): Error listening on the socket");
                error_event.set_data(sock_error, -1);
                executor.register_event(error_event);
                close(fd);
                return;
            }

        }


        ~acceptor() {
            std::cout << "Acceptor destructor\n";
            close(fd);
        }

        void async_accept(tcp::socket &socket, AsyncCallback callback) {
            std::cout << "ACCEPTING...\n";
            socklen_t socklen;
            tcp::endpoint remote_endpoint;
            int new_fd = accept(fd, (sockaddr *)&remote_endpoint.server_addr, &socklen);            
            if(new_fd == -1) {
                error sock_error;
                event error_event("ERROR_EVENT", callback, SOCKET_IO);
                sock_error.set_error_message("accept(): Error accepting a new connection");
                error_event.set_data(sock_error, -1);
                executor.register_event(error_event);
                close(new_fd);
                close(fd);
                return;
            }

            std::cout << "Accepted from " << inet_ntoa(remote_endpoint.server_addr.sin_addr) << ":" << remote_endpoint.server_addr.sin_port << std::endl;
            error no_error;
            event accepted_event("ACCEPTED_CONN", callback, SOCKET_IO);
            
            socket.file_descriptor = new_fd;
            accepted_event.set_data(no_error, 0);
            executor.register_event(accepted_event);
        }

    private:
        asyncio::executor &executor;
        int fd;
        int n_acceptable_connections = 4;
    };
}
}


#endif 