#include "../include/log.h"
#include "../include/server.h"


int create_socket() {
    int listen_d = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_d == -1) {
        error("Cant create socket", 1);
    }
    return listen_d;
};


int read_in(int socket, char *buf, int len) {
    char *s = buf;
    int slen = len;
    int c = (int) recv(socket, s, slen, 0);
    while ((c > 0) && (s[c - 1] != '\n')) {
        s += c;
        slen -= c;
        c = (int) recv(socket, s, slen, 0);
    }
    return len - slen;
}


void read_http_request(int socket, char *buf, int len) {
    while (1) {
        int buf_len = recv(socket, buf, len, 0);
        if (buf_len < 0) {
            // when errno is EAGAIN and in non-blocking mode, the buf is end.
            if (errno == EAGAIN) {
                break;
            }
        } else if (buf_len == 0) {
            // The socket has been properly closed.
            break;
        }
        if (buf[buf_len - 1] == '\n' && buf[buf_len - 2] == '\r' && buf[buf_len - 3] == '\n' && buf[buf_len - 4] == '\r') {
            break;
        }
    }
}

void bind_socket(int listen_d, int port) {
    // bind
    struct sockaddr_in name;
    name.sin_family = PF_INET;
    name.sin_port = (in_port_t) htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    // use again
    int reuse = 1;
    if (setsockopt(listen_d, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(int)) == -1) {
        error("This socket is using", 1);
    }
    int c = bind(listen_d, (struct sockaddr *) &name, sizeof(name));
    if (c == -1) {
        error("Cant bind this socket", 1);
    }
}

void handle(int connect_d) {
    // TODO handle http request
    char buf[4096];
    bzero(buf, 4096);
    read_http_request(connect_d, buf, sizeof(buf));

    printf("%s", buf);
}