#include "../include/config.h"
#include "../include/log.h"
#include "../include/server.h"
#include "../include/http.h"
#include "../include/io.h"

// create socket
int create_socket() {
    int listen_d = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_d == -1) {
        error("Cant create socket", 1);
    }
    return listen_d;
};

// bind socket
void bind_socket(int listen_d, int port) {
    // bind
    struct sockaddr_in name;
    name.sin_family = PF_INET;
    name.sin_port = (in_port_t) htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    // socket multiplexing
    int reuse = 1;
    if (setsockopt(listen_d, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(int)) == -1) {
        error("This socket is using", 1);
    }

    int c = bind(listen_d, (struct sockaddr *) &name, sizeof(name));
    if (c == -1) {
        error("Cant bind this socket", 1);
    }
}

// control request
void handle(int connect_d) {

    char buf[BUF_LEN];
    bzero(buf, BUF_LEN);

    struct io_class io;
    io_init(&io, connect_d);
    if (io_readline(&io, buf, BUF_LEN) < 0) {
        error("read line error", 1);
    }

    // format struct http_header
    struct http_header hhr;
    format_method_uri_version(buf, &hhr);

    // TODO only GET or POST now, other will be completed after
    if (strcasecmp(hhr.method, "GET") && strcasecmp(hhr.method, "POST")) {
        server_error_response(
                connect_d,
                hhr.method,
                "501",
                "Not Implement",
                "Server does not implement this method"
        );
        return;
    }

    // get request header info
    read_request_header(&io, &hhr);

    // format uri, filename, params
    int is_static = parse_uri(hhr.uri, hhr.filename, hhr.name, hhr.cgiargs);

    struct stat sbuf;
    // determine whether the request file exists
    if (stat(hhr.filename, &sbuf) < 0) {
        server_error_response(
                connect_d,
                hhr.filename,
                "404",
                "Not found",
                "Server couldn't find this file");
        return;
    }

    if (is_static) {
        // determine whether the file is simple and has read permissions
        if (!S_ISREG(sbuf.st_mode) || !(S_IRUSR & sbuf.st_mode)) {
            server_error_response(
                    connect_d,
                    hhr.filename,
                    "403",
                    "Forbidden",
                    "Server couldn't read the file");
            return;
        }
        server_static(connect_d, hhr.filename, sbuf.st_size);
    } else {
        // determine whether the file has execute permissions
        if (!S_ISREG(sbuf.st_mode) || !(S_IXUSR & sbuf.st_mode)) {
            server_error_response(
                    connect_d,
                    hhr.filename,
                    "403",
                    "Forbidden",
                    "Server couldn't run the CGI program");
        }
        //TODO
        server_dynamic(&io, &hhr);
    }
}