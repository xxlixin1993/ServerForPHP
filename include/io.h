#ifndef SERVERFORPHP_IO_H
#define SERVERFORPHP_IO_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define IO_BUF_SIZE 8192

struct io_class {
    int fd;                 // The corresponding descriptor
    int count;                // Can read byte length
    char *io_buf_next;           // The next byte address that can be read
    char io_buf[IO_BUF_SIZE];  // Internal buffer
};

// init io_class
void io_init(struct io_class *io, int fd);

// package system read
ssize_t io_read(struct io_class *io, char *buf, size_t len);

// read line and copy to the param buf
ssize_t io_readline(struct io_class *io, void *buf, size_t len);

ssize_t http_readn(int fd, void *usrbuf, size_t n);

//ssize_t io_write(int fd, void *usrbuf, size_t n);

//ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);





#endif //SERVERFORPHP_IO_H



