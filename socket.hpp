#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "error.hpp"
#include "endpoint.hpp"
#include <functional>

namespace asyncio {
     namespace tcp {

        class socket {
            typedef std::function<void(asyncio::error, int)> AsyncCallback;
        public:

            ~socket() {
                close(file_descriptor);
            }

            void async_connect(tcp::endpoint endpoint, AsyncCallback callback) {
                        
                error err;
                callback(err, 12);
            }

            void async_listen(tcp::endpoint endpoint) {
                file_descriptor = ::socket(AF_INET, SOCK_STREAM, 0);
                
                if(file_descriptor == -1) {
                    //throw error
                    asyncio::error error;
                    error.set_error_message("socket() error");
                    std::cout << "ERROR CREATING SOCKET\n";
                }
                int opt = 1;
                if (setsockopt(file_descriptor, SOL_SOCKET,
                    SO_REUSEADDR | SO_REUSEPORT, &opt,
                    sizeof(opt))) {
                        perror("setsockopt");
                        exit(EXIT_FAILURE);
                }

                if(bind(file_descriptor, (const sockaddr*)&endpoint.server_addr, sizeof(endpoint.server_addr)) < 0) {
                    //throw bind eerror
                    error err;
                    close(file_descriptor);
                    exit(-1);
                }

                if(listen(file_descriptor, n_acceptable_connections) == 0) {
                    std::cout << "Listening...\n";
                }
                else {
                    std::cerr << "LISTEN ERRROR\n";
                }
                tcp::endpoint _ep;
                _ep.server_addr = endpoint.server_addr;
                
                async_accept(_ep, [](asyncio::error, int){});
            }

            void async_accept(tcp::endpoint peer_endpoint, AsyncCallback callback) {
               
                socklen_t len;
                int newFd = accept(file_descriptor, (struct sockaddr*)&peer_endpoint.server_addr, &len);
                 if(newFd == -1) {
                    std::cerr << "accept error: " << errno << std::endl;
                }
                else {
                    std::cout << "New connection accepted from " << inet_ntoa(peer_endpoint.server_addr.sin_addr) << std::endl;
                }
                // read(newFd, buffer, 1024-1);
                // printf("%s\n", buffer);
                // const char *hallo = "hallo~~!";
                // send(newFd, hallo, strlen(hallo), 0);
                // close(newFd);

                async_accept(peer_endpoint, callback);
            }
            
            void async_read_some(char *buffer, int size, AsyncCallback callback) {
            }

            void async_write_some(char *buffer, int size, AsyncCallback callback) {

            }

        public:
            int n_acceptable_connections = 4;


        private:
            int file_descriptor = 0;    
            tcp::endpoint ep;
        };

    }
}



#endif 