#include "../include/log.h"
#include "../include/server.h"

int main(int argc, char **argv) {
    int listen_d = create_socket();
    bind_socket(listen_d, PORT);

    // listen, list len is 10
    if (listen(listen_d, 10) == -1) {
        error("Cant listen this socket", 1);
    }

    struct sockaddr_storage client_addr;

    while (1) {
        // accept connect
        unsigned int address_size = sizeof(client_addr);
        int connect_d = accept(listen_d, (struct sockaddr *) &client_addr, &address_size);
        if (connect_d < 0) {
            error("Accept error", 1);
        }
        handle(connect_d);

        close(connect_d);
    }

    return 0;
}