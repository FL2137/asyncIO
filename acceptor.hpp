#ifndef ACCEPTOR_HPP
#define ACCEPTOR_HPP

#include "executor.hpp"
#include "endpoint.hpp"
#include "error.hpp"
#include "event.hpp"
#include "socket.hpp"

namespace asyncio {
    class acceptor {
        typedef std::function<void(asyncio::error, int)> AsyncCallback;

    public:
        acceptor(executor &exec, tcp::endpoint local_endpoint) : executor(exec) {
            fd = ::socket(AF_INET, SOCK_STREAM, 0);
            event error_event("ERROR_EVENT", error::error_callback, SOCKET_IO);
            error sock_error;

            if(fd == -1) {
                sock_error.set_error_message("Error occurred while creating the socket");
                error_event.set_data(sock_error, -1);
                executor.register_event(error_event);
                close(fd);
                return;
            }
            int options = 1;
            if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &options, sizeof(options))) {
                sock_error.set_error_message("setsockopt(): Error occurred while setting socket options");
                error_event.set_data(sock_error, -1);
                executor.register_event(error_event);
                close(fd);
                return;
            }

            if(bind(fd, (const sockaddr*)&local_endpoint.server_addr, sizeof(local_endpoint.server_addr) == -1)) {
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

        void async_connect(tcp::socket &socket, AsyncCallback callback) {
            socklen_t socklen;
            tcp::endpoint remote_endpoint;
            int new_fd = accept(fd, (sockaddr *)&remote_endpoint.server_addr, &socklen);            
            if(new_fd == -1) {
                error sock_error;
                event error_event("ERROR_EVENT", callback, SOCKET_IO);
                sock_error.set_error_message("listen(): Error listening on the socket");
                error_event.set_data(sock_error, -1);
                executor.register_event(error_event);
                close(new_fd);
                close(fd);
            }
            error no_error;
            event accepted_event("ACCEPTED_CONN", callback, SOCKET_IO);
            
            socket.file_descriptor = new_fd;
            accepted_event.set_data(no_error, 0);
            executor.register_event(accepted_event);
        }

    private:
        executor &executor;
        int fd;
        int n_acceptable_connections = 4;

    };
}


#endif 