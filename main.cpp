#include <string>
#include <iostream>
#include <execution>
#include <thread>
#include <functional>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>


void callback() {

}

namespace asyncio {

    class error : std::exception {

    };

    namespace tcp {
        
        class endpoint {

        public:
            endpoint(std::string address, short port) {
                server_addr.sin_family = AF_INET;
                server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //default address
                server_addr.sin_port = htons(port);
            }

            endpoint(const char *address, short port);


        private:
            sockaddr_in server_addr;

            std::string address;
            short int port;
        };

        class socket {
            typedef std::function<void(asyncio::error, int)> AsyncCallback;

            void async_connect(tcp::endpoint endpoint) {
                file_descriptor = ::socket(AF_INET, SOCK_STREAM, 0);

                if(file_descriptor == -1) {
                    //throw error
                }



            }
            
            void async_read_some(char *buffer, int size, AsyncCallback callback) {

            }

            void async_write_some(char *buffer, int size, AsyncCallback callback) {

            }

            int file_descriptor = 0;    
        };

    }
}


int main() {


    return 0;
}