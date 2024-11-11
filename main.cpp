#include "endpoint.hpp"
#include "socket.hpp"

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
#include <fcntl.h>

#include "executor.hpp"
#include "acceptor.hpp"
#include "tui.hpp"
#include "callback.hpp"
#include "asyncio.hpp"

void srrr() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in sckin;
    sckin.sin_addr.s_addr = htonl(INADDR_ANY);
    sckin.sin_family = AF_INET;
    sckin.sin_port = htons(3011);

    int check = bind(fd, (const sockaddr*)&sckin, sizeof(sckin));
    if(check == 0){
        std::cout << "bind fine\n";
    }
    else {
        close(fd);
        std::cout << "exiting...\n";
        exit(-1);
    }

    check = listen(fd, 4);
    if(check == 0){
        std::cout << "listen fine\n";
    }


    std::cout<<"Accepting...\n";
    socklen_t aaa = sizeof(sckin);
    int newFD = accept(fd, (struct sockaddr*)&sckin, &aaa);
    if(newFD != -1) {
        std::cout << "OK\n";
        const char *odp = "YEAAAAH";
    }
    else { 
        std::cout << "gowno:" << errno << std::endl;
    }
    char buffer[1024] = {};
    const char *hello = "hola senior";
    ssize_t valread = read(newFD, buffer, 1024 - 1);
    printf("%s\n", buffer);
    send(newFD, hello, strlen(hello), 0);

    close(newFD);
    close(fd);
}


class tcp_connection {

public:
    tcp_connection(asyncio::executor& exec): socket(exec) {
        buffer = new char[1024];
    }
    typedef std::shared_ptr<tcp_connection> pointer;

    static pointer create(asyncio::executor &exec) {
        return std::make_shared<tcp_connection>(tcp_connection(exec));
    }


    void run() {
        do_read();
        std::cout << "RUN LOL\n";
    }

    asyncio::tcp::socket& get_socket() {
        return socket;
    }

    void do_read() {
        socket.async_read(buffer, 1024, [&, this](asyncio::error error, int nbytes) {
            if(error.isError()){
                std::cout << "ERROR IN READ CALL BACK\n";

            }
            std::cout.write(buffer, strlen(buffer)) << std::endl;
            for(int i =0; i < strlen(buffer); i++) {
                buffer[i] = toupper(buffer[i]);
            }
            do_write();
        });
        // asyncio::async_read_some(socket, buffer, 1024, [&](asyncio::error error, int nbytes) {
        //     if(error.isError()){
        //         std::cout << error.what() << std::endl;

        //     }
        //     std::cout.write(buffer, strlen(buffer)) << std::endl;
        //     for(int i =0; i < strlen(buffer); i++) {
        //         buffer[i] = toupper(buffer[i]);
        //     }
        //     do_write();
        // });
    }

    void do_write() {
        asyncio::async_write(socket, buffer, 1024, [&](asyncio::error error, int nbytes) {
            if(error.isError()) {
                std::cout << error.what() << std::endl;
            }
            else {
                std::cout << "written " << nbytes << "bytes" << std::endl;
            }
        });
    }

private:


private:
    char *buffer = nullptr;
    asyncio::tcp::socket socket;
};

class program {
public:

    program(asyncio::executor &executor, asyncio::tcp::endpoint &host_endpoint): exec(executor), acceptor(executor, host_endpoint) {
        this->endpoint = host_endpoint;
        start_accepting();
    }   



    void start_accepting() {

        // acceptor.async_accept(socket, [&](asyncio::error error) {
        //     if(error.isError()){
        //         std::cout << "AcceptHandler: " << error.what() << std::endl;
        //         return;
        //     }
        //     connections.push_back(std::make_shared<tcp_connection>(socket));
        // });
        asyncio::error er;
        auto con = tcp_connection::create(exec);

        asyncio::async_accept_one(acceptor, con->get_socket(), std::bind(&program::accept_handle, this, con, asyncio::error::placeholder()));

        //acceptor.async_accept(con->get_socket(), std::bind(&program::accept_handle, this, con, asyncio::error::placeholder()));
    }

    void accept_handle(tcp_connection::pointer newcon, asyncio::error &error) {
        if(error) {
            std::cout << error.what() << std::endl;
            return;
        } 
        std::cout << "accept_handle\n";
        newcon->run();
        connections.push_back(newcon);
        start_accepting();
    }


    std::vector<std::shared_ptr<tcp_connection>> connections;
    char buffer[1024] {0};
    asyncio::tcp::endpoint endpoint;
    asyncio::tcp::acceptor acceptor;

    asyncio::executor& exec;

};

int bindListen() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int n_acceptable_connections = 4;

    std::cout << "ACCEPTOR LISTEN START\n";

    if(fd == -1) {
        close(fd);
        std::cout << "ACCEPTOR LISTEN 1\n";
        return -1;
    }
    int options = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &options, sizeof(options))) {
        close(fd);
        std::cout << "ACCEPTOR LISTEN 2\n";
        std::cout << errno << std::endl;

        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);
    server_addr.sin_family = AF_INET;

    
    if(bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        close(fd);
        return -1;
    }

    if(listen(fd, n_acceptable_connections) == -1) {
        close(fd);
        return -1;
    }
    std::cout << "uwu\n";

    return fd;
}


#include "http.hpp"
#include <fstream>


int main() {
using namespace asyncio;
using namespace std::string_literals;
    std::ifstream http_request_sample("./samplehttp.txt", std::ios::binary);

    


    std::string whole = ""s;
    std::string line = ""s;
    while(std::getline(http_request_sample, line)) {
        whole += line;
    }

    std::cout << whole << std::endl;
    http::request request();

    for(int i = 0 ;i < whole.size() -1; i++) {
        if(whole[i] == '\r' && whole[i+1] == '\n') {
            std::cout << "xd\n";
        }
    }











    return 3;
    int pid = getpid();
    std::cout << "PID: " << pid << std::endl;
    asyncio::executor executor;
    asyncio::tcp::endpoint ep(5001);
    asyncio::tcp::socket socket(executor);
    program prog(executor, ep);

    executor.run();
    return 0;
}