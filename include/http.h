#ifndef SERVERFORPHP_HTTP_H
#define SERVERFORPHP_HTTP_H
#include "../include/io.h"
#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
struct http_header {
    char uri[256];          // request uri
    char method[16];        // request method
    char version[16];       // http version
    char filename[256];     // request filename(including the full path)
    char name[256];         // request filename(only filename)
    char cgiargs[256];      // request param
    char contype[256];      // request type
    char conlength[16];     // request length
};

// read http request method uri version, format struct http_header method uri version
void format_method_uri_version(char *buf, struct http_header *hhr);

// return server error response
void server_error_response(int socket, char *cause, char *err_num, char *short_msg, char *long_msg);

// read http request header (without first http request line)
void read_request_header(struct io_class *io, struct http_header *hhr);

// determines whether the start of the string contains lowercase "http_str". Included return 1 else return 0
int is_included_str(char *str, char *http_str);

// format uri, filename, params and return whether dynamic requests or static requests
int parse_uri(char *uri, char *filename, char *name, char *cgiargs);

// read the contents of the static file, sent to the client
void server_static(int fd, char *filename, int file_size);

// handle dynamic php requests
void server_dynamic(struct io_class *io, struct http_header *hhr);
#endif //SERVERFORPHP_HTTP_H

