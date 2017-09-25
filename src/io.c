#include "../include/io.h"

// init io_class
void io_init(struct io_class *io, int fd) {
    io->fd = fd;
    io->count = 0;
    io->io_buf_next = io->io_buf;
}

// package system read
ssize_t io_read(struct io_class *io, char *buf, size_t len) {
    while (io->count <= 0) {
        io->count = read(io->fd, io->io_buf, sizeof(io->io_buf));

        if (io->count < 0) {
            // return -1
            if (errno != EINTR) {
                return -1;
            }
        } else if (io->count == 0) {
            // EOF
            return 0;
        } else {
            //
            io->io_buf_next = io->io_buf;
        }
    }

    int count = len;
    if (io->count < len) {
        count = io->count;
    }
    memcpy(buf, io->io_buf_next, count);
    io->io_buf_next += count;
    io->count -= count;

    return count;
}

ssize_t http_readn(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n; // 剩下的未读字节数
    ssize_t nread;
    char *bufp = usrbuf;

    while (nleft > 0) {
        if ((nread = read(fd, bufp, nleft)) < 0) {
            if (errno == EINTR) { // 被信号处理函数中断返回
                nread = 0;
            } else {
                return -1;  // read出错
            }
        } else if (nread == 0) { // EOF
            break;
        }
        nleft -= nread;
        bufp += nread;
    }

    return (n - nleft); // 返回已经读取的字节数
}

// read line and copy to the param buf
ssize_t io_readline(struct io_class *io, void *buf, size_t len) {
    int n, rc;
    char c, *buf_p = buf;

    for (n = 1; n < len; n++) {
        if ((rc = io_read(io, &c, 1)) == 1) {
            *buf_p++ = c;
            if (c == '\n') {
                // read line
                break;
            }
        } else if (rc == 0) {
            if (n == 1) {
                // EOF, but don‘t read anything
                return 0;
            } else {
                //EOF, read something
                break;
            }
        } else {
            // error
            return -1;
        }
    }

    *buf_p = 0;
    return n;
}

