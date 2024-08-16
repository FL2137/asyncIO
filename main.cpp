#include <string>
#include <iostream>
#include <execution>
#include <thread>
#include <functional>
#include <sys/signal.h>


void callback() {

}

namespace asyncio {

    class error : std::exception {

    };

    namespace tcp {
        class socket {
            //this function is the main event loop that takes control of the main thread
            
            void async_read_some(char *buffer, int size, std::function<void(asyncio::error error, int bytes_transferred)> callback) {

            }

            void async_write_some(char *buffer, int size) {

            }

            int file_descriptor = 0;    
        };

        class endpoint {



            std::string address;
            short int port;
        };
    }
}


int main() {



    return 0;
}