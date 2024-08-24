#include "endpoint.hpp"

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

void callback() {

}

namespace asyncio {

    class error : std::exception {

    };

    namespace tcp {

        class socket {
            typedef std::function<void(asyncio::error, int)> AsyncCallback;
        public:

            ~socket() {
                close(file_descriptor);
            }

            void async_connect(tcp::endpoint endpoint, AsyncCallback callback) {
                file_descriptor = ::socket(AF_INET, SOCK_STREAM, 0);
                
                if(file_descriptor == -1) {
                    //throw error
                    std::cout << "ERROR CREATING SOCKET\n";
                }
                if(bind(file_descriptor, (const sockaddr*)&endpoint.server_addr, sizeof(endpoint.server_addr)) < 0) {
                    //throw bind eerror
                    std::cout << "BIND ERROR\n";
                }

                std::cout << "LOL\n";


            }
            
            void async_read_some(char *buffer, int size, AsyncCallback callback) {

            }

            void async_write_some(char *buffer, int size, AsyncCallback callback) {

            }

        private:
            int file_descriptor = 0;    
        };

    }
}


int main() {

    asyncio::tcp::endpoint ep(3001);
    asyncio::tcp::socket socket;
    socket.async_connect(ep, [](asyncio::error error, int size){

    });

    while(1){}

    return 0;
}