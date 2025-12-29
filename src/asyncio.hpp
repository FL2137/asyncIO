#pragma once

#include "error.hpp"
#include "socket.hpp"
#include "acceptor.hpp"
#include "executor.hpp"
#include <type_traits>


namespace asyncio {

    void async_write(const asyncio::tcp::socket &socket, char *buffer, int size, WriteCallback completion_handler) {

        Task *impl_task = new Task();
        int sfd = socket.fd;

        impl_task->m_name = "write_impl_task";

        impl_task->m_completion_handler = [&, sfd, buffer, size, completion_handler]() {

            asyncio::error error;
            int result = 0;

            int open_check = fcntl(sfd, F_GETFD);
            if(open_check == -1) {
                std::cout << "async_write:: file descriptor bad\n";
                return 0;
            }

            try {
                result = write(sfd, buffer, size);
            }
            catch (const std::exception& e) {
                std::cerr << e.what() << "\n";
            }
            if(result == -1) {
                error.set_error_message("Write error: " + std::to_string(errno));
            }

            WriteTask *write_task = new WriteTask(completion_handler);
            write_task->m_name = "write_task";
            write_task->set_data(error, result);
            socket.enqueue(write_task);
        };

        socket.enqueue(impl_task);
    }

    void async_write(const int fd, char *buffer, int size, WriteCallback callback) {

    }

    template<typename Acceptor, typename Socket> 
    void async_accept_one(const Acceptor &acceptor, Socket &socket, AcceptCallback completion_handler) {
        Task *impl_task = new Task();
        impl_task->m_name = "accept_one_impl_task";
        impl_task->m_completion_handler = [&, completion_handler] {
            asyncio::error error;
            
            sockaddr_in remote_endpoint;
            socklen_t socklen = sizeof(remote_endpoint);
            int result_fd = accept(acceptor.fd, (sockaddr*)&remote_endpoint, &socklen);

            if(result_fd == -1) {
                error.set_error_message("accept error: " + std::to_string(errno));
            }
            else {
                char buff[100];
                inet_ntop(AF_INET, &remote_endpoint.sin_addr, buff, 100);
            }
            if(fcntl(result_fd, F_SETFL, fcntl(result_fd, F_GETFL) | O_NONBLOCK) == -1) {
                //error
            }
            socket.setup(result_fd);

            AcceptTask *accept_Task = new AcceptTask(completion_handler);
            accept_Task->m_name = "accept_one_Task";
            accept_Task->set_data(error);

            acceptor.enqueue(accept_Task);
        };

        acceptor.enqueue(impl_task);
    }

    template<typename Reader>
    void async_read_some(const Reader& reader, char *buffer, int size, ReadCallback callback) {
        Task *impl_Task = new Task();
        int sfd = reader.fd;
        impl_Task->m_name = "read_impl_Task";

        impl_Task->m_completion_handler = [&, sfd, buffer, size, callback] {
            asyncio::error error;
            char *b = new char[1024];
            int result = read(sfd, buffer, 1024);
            if(result == -1) {
                error.set_error_message("Read error: " + std::to_string(errno));
            }

            ReadTask *read_Task = new ReadTask(callback);
            read_Task->m_name = "ReadTask." + std::to_string(sfd);
            read_Task->set_data(error, result);
            reader.enqueue_task(read_Task);
        };

        reader.enqueue_task(impl_Task);
    }

    template<typename Reader>
    void async_read(const Reader &reader, char *buffer, int size, ReadCallback callback) {
        static int id = -1;
        
        Task *impl = new Task();

        impl->m_name = "read_impl_Task";

        impl->m_completion_handler = [&, buffer, size, callback] {
            asyncio::error error;
            int result = read(reader.fd, buffer, size);

            if(result == -1){
                error.set_error_message("Read error: " + std::to_string(errno));
            }

            ReadTask *Task = new ReadTask(callback);
            Task->m_name = "ReadTask." + std::to_string(reader.fd);
            Task->set_data(error, result);
            reader.enqueue_task(Task);

        };

        reader.epoll_read_event.events = EPOLLONESHOT | EPOLLIN | EPOLLET;
        reader.epoll_read_event.data.fd = reader.fd;
        reader.epoll_read_event.data.ptr = buffer;
        reader.epoll_read_event.data.u32 = reader.fd * -1;
    }

    template<typename Writer> 
    void async_write_some(const Writer& writer, char *buffer, int size, WriteCallback completion_handler) {
        Task *impl_Task = new Task();
        int sfd = writer.fd;
        impl_Task->m_name = "write_impl_task";
        impl_Task->m_completion_handler = [&, sfd, buffer, size, completion_handler] {
            asyncio::error error;
            int result;
            try {
                result = write(sfd, buffer, size);
            }
            catch(std::exception &e) {
                std::cerr << e.what() << "\n"; 
            }
            if(result == -1) {
                error.set_error_message("Wrtite error: " + std::to_string(errno));
            }

            auto *write_task = new WriteTask(completion_handler);
            write_task->m_name = "WriteTask." + std::to_string(sfd);
            write_task->set_data(error, result);
            writer.enqueue_task(write_task);
        };

        writer.enqueue_task(impl_Task);
    }

    void read_implementation(int fd, char *buffer, int size, const asyncio::tcp::socket &socket, ReadCallback completion_handler) {
        WriteTask* handler_Task = new WriteTask(completion_handler);

    }

    void write_implementation(int fd, char *buffer, int size, const asyncio::tcp::socket &socket, WriteCallback completion_handler) {

    }

    template <typename Executor, typename Task>
    typename std::enable_if<std::is_base_of<asyncio::Task, typename std::decay<Task>::type>::value, void>::type
    post(Executor& executor, Task&& f) {
        executor.enqueue_task(std::forward<Task>(f));
    }

    template <typename Executor, typename Task>
    typename std::enable_if<!std::is_base_of<asyncio::Task, typename std::decay<Task>::type>::value, void>::type
    post(Executor &executor, Task f) {
        auto *task = new asyncio::Task(f);
        std::cout << task->m_name << std::endl;;
        executor.enqueue_task(task);
    }
};