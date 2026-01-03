#ifndef IO_HPP
#define IO_HPP

#include <iostream>

#include <liburing.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <fcntl.h>

struct file_info
{
    off_t file_sz;
    iovec iovecs[];
};



class io
{
public:

    const int QUEUE_DEPTH = 1;
    
    void init()
    {
        io_uring_queue_init(QUEUE_DEPTH, &ring, 0);
        
        char* file_to_read = "/home/coolyfooly/wget-log";
        unsigned char* readbuffer = nullptr;
        

        submit_read_request(file_to_read);

        get_completion();
        
        std::cout << "exiting io_uring\n";
        io_uring_queue_exit(&ring);

        struct stat open_check;
        if (fstat(last_open_fd, &open_check) == 0)
        {
            close(last_open_fd);
            std::cout << "had to close the file\n";
        }
        else{
            std::cout << "it was closed\n";
        }
    }
    
    private:
    
    off_t get_file_size(int fd)
    {
        struct stat st;
        
        if(fstat(fd, &st) < 0)
        {
            std::cerr << "get_file_size(): fstat error\n";
            return -1;
        }
        
        if (S_ISBLK(st.st_mode))
        {
            unsigned long long bytes;
            if (ioctl(fd, BLKGETSIZE64, &bytes) != 0)
            {
                std::cerr << "get_file_size(): ioctl error\n";
                return -1;
            }
            return bytes;
        }
        else if (S_ISREG(st.st_mode))
        { 
            return st.st_size;
        }
        
        return -1;
    }
    
    int submit_read_request(char* file)
    {
        last_open_fd = open(file, O_RDONLY);
        
        if (last_open_fd < 0)
        {
            return -1;
        }
        
        off_t file_size = get_file_size(last_open_fd);
        off_t bytes_remaining = file_size;
        off_t offset = 0;
        int current_block = 0;
        int blocks = static_cast<int>(file_size / BLOCK_SIZE);
        if (file_size % BLOCK_SIZE)
        {
            blocks++;
        }

        file_info *fi = new file_info(sizeof(*fi) + (sizeof(iovec) * blocks));

        char* read_buffer = new char[file_size];

        while(bytes_remaining)
        {
            off_t bytes_to_read = bytes_remaining;
            if (bytes_to_read > BLOCK_SIZE)
            {
                bytes_to_read = BLOCK_SIZE;
            }

            offset += bytes_to_read;
            fi->iovecs[current_block].iov_len = bytes_to_read;

            void *buf;
            if(posix_memalign(&buf, BLOCK_SIZE, BLOCK_SIZE))
            {
                std::cerr << "submit_read_request(): posix_memalign error\n";
                return -1;
            }

            fi->iovecs[current_block].iov_base = buf;

            current_block++;
            bytes_remaining -= bytes_to_read;
        }

        fi->file_sz = file_size;


        io_uring_sqe* sqe = io_uring_get_sqe(&ring);

        io_uring_prep_readv(sqe, last_open_fd, fi->iovecs, blocks, 0);
        io_uring_sqe_set_data(sqe, fi);

        io_uring_submit(&ring);

        return 0;
    }
    

    void output_to_console(char *buf, int len) {
        while (len--) {
            fputc(*buf++, stdout);
        }
    }
    
    int get_completion()
    {
        io_uring_cqe* cqe;
        int ret = io_uring_wait_cqe(&ring, &cqe);

        if (ret != 0)
        {
            std::cerr << "get_completion(): io_uring_wait_cqe error\n";
        }

        file_info* fi = static_cast<file_info*>(io_uring_cqe_get_data(cqe));

        int blocks = static_cast<int>(fi->file_sz / BLOCK_SIZE);
        
        if(fi->file_sz % BLOCK_SIZE)
        {
            blocks++;
        }

        for (int i = 0; i < blocks; ++i)
        {
            //std::cout.write((char*)fi->iovecs[i].iov_base, fi->iovecs[i].iov_len);
        }
        
        io_uring_cqe_seen(&ring, cqe);

        
        std::cout << "Closed...\n";

        return 0;
    }

private:
  
    io_uring ring;
    int last_open_fd = -1;
};


#endif // IO_HPP