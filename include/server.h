#ifndef SERVERFORPHP_SERVER_H
#define SERVERFORPHP_SERVER_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>


// create socket
int create_socket();

// bind socket
void bind_socket(int listen_d, int port);

// control request
void handle(int connect_d);

#endif //SERVERFORPHP_SERVER_H

