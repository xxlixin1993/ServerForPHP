#include "../include/log.h";
#include "../include/server.h";


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

void handle(int connect_d){
    // TODO handle http request
    char buf[1024];
    bzero(buf, 1024);
    read_in(connect_d, buf, sizeof(buf));

    printf("%s",buf);
}