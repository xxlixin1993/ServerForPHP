#ifndef SERVERFORPHP_SERVER_H
#define SERVERFORPHP_SERVER_H

#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#endif //SERVERFORPHP_SERVER_H

#define PORT 8000 // 默认端口号

struct http_request {

};

// create socket
int create_socket();

// bind socket
void bind_socket(int listen_d, int port);

// read in request
int read_in(int socket, char *buf, int len);

// read in http request
void read_http_request(int socket, char *buf, int len);

// control request
void handle(int connect_d);