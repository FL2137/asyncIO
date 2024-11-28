#ifndef ENDPOINT_HPP
#define ENDPOINT_HPP

#include <string>
#include <iostream>
#include <cstdlib>
#include <execution>
#include <thread>
#include <cstring>
#include <functional>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

namespace asyncio {
    namespace tcp {
        class endpoint {
        public:

            endpoint() {

            }

            endpoint(short port) {
                this->address = "127.0.0.1";
                this->port = port;
                server_addr.sin_addr.s_addr = INADDR_ANY;
                server_addr.sin_family = AF_INET;
                server_addr.sin_port = htons(port);
            }

            endpoint(std::string address, short port) {
                this->address = address;
                this->port = port;
                server_addr.sin_family = AF_INET;
                
                //server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //default address
                inet_aton(address.c_str(), &server_addr.sin_addr);
                server_addr.sin_port = htons(port);
            }

            endpoint(const char *address, short port) {
                this->address = std::string(address);
                this->port = port;
                inet_aton(address, &server_addr.sin_addr);
                server_addr.sin_family = AF_INET;
                server_addr.sin_port = htons(port);
            }   

            std::string address = "";
            short int port = 0;
            sockaddr_in server_addr;
        };
    }
}

#endif