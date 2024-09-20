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
#include "tester.hpp"
#include "acceptor.hpp"
#include "tui.hpp"
#include "callback.hpp"

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

class program {
public:

    program(asyncio::executor &executor, asyncio::tcp::endpoint host_endpoint) : socket(executor), acceptor(executor, host_endpoint) {
        this->endpoint = host_endpoint;
        
        start_accepting();
    }   


    void start_accepting() {
        acceptor.async_accept(socket, [&](asyncio::error error, int nbytes) {
            if(error.isError()){
                std::cout << error.what() << ": " << errno << std::endl;
                std::cout << "FD: " << nbytes << std::endl;
                return;
            }
            else {
                do_read();
            }
            start_accepting();
        });
    }

    void do_write() {
        socket.async_write_some(buffer, 1024, [](asyncio::error error, int nbytes) {
            if(error.isError()) {
                std::cout << "ERROR IN WRITE CALL BACK\n";
            }
            else {
                std::cout << "ok????";
            }
        });
    }

    void do_read() {
        socket.async_read_some(buffer, 1024, [&, this](asyncio::error error, int nbytes) {
            if(error.isError()){
                std::cout << "ERROR IN READ CALL BACK\n";

            }
            std::cout.write(buffer, strlen(buffer)) << std::endl;
            for(int i =0; i < strlen(buffer); i++) {
                buffer[i] = toupper(buffer[i]);
            }
            do_write();
        });
    }

    char buffer[1024];
    asyncio::tcp::socket socket;
    asyncio::tcp::endpoint endpoint;
    asyncio::tcp::acceptor acceptor;

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


struct xd {
    int ID = 1;
    int number;

};


typedef void (*callback)(asyncio::error, int);

//std::map<int, callback> callback_map;

void read_callback(asyncio::error error, int nbytes) {
    std::cout << "READ CLB\n";
}

void write_callback(asyncio::error error, int callback) {
    std::cout << "WRITE CLB\n";
}

void accept_callback(asyncio::error error, int nbytes) {


    std::cout << "ACCEPT CLB\n";
}

typedef std::function<void(asyncio::error, int)> Clb;


int main() {

    std::map<int, Clb> clb_map;
    clb_map[0] = write_callback;


    exit(-1);
    int efd = epoll_create(5);
    epoll_event eevents[5];
    epoll_event eevent;
    int serverfd = bindListen();
    eevent.data.fd = serverfd;
    eevent.events = EPOLLIN | EPOLLET;
    clb_map[serverfd] = accept_callback;

    epoll_ctl(efd, EPOLL_CTL_ADD, serverfd, &eevent);
    
    while(1) {
        int nfds = epoll_wait(efd, eevents, 5, -1);

        for(int i = 0; i < nfds; i++) {
            clb_map[i](asyncio::error(), 15);
        }
    }


    // int pid = getpid();
    // std::cout << "PID: " << pid << std::endl;
    // asyncio::executor executor;
    // asyncio::tcp::endpoint ep(5001);
    // asyncio::tcp::socket socket(executor);
    // program prog(executor, ep);
    // executor.run();
    return 0;
}