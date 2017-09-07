#include "../include/http.h"
#include "../include/log.h"
#include "../include/tool.h"
#include "../include/config.h"

// read http request method uri version, format struct http_header method uri version
void format_method_uri_version(char *buf, struct http_header *hhr) {
    sscanf(buf, "%s %s %s", hhr->method, hhr->uri, hhr->version);
}

// read http request header (without first http request line)
void read_request_header(struct io_class *io, struct http_header *hhr) {
    char buf[BUF_LEN];
    char *start, *end;

    memset(buf, 0, BUF_LEN);
    if (io_readline(io, buf, BUF_LEN) < 0) {
        error("io read line error", 1);
    }

    while (strcmp(buf, "\r\n")) {

        start = index(buf, ':');
        end = index(buf, '\r');

        if (start != 0 && end != 0) {
            *end = '\0';

            while ((*(start + 1)) == ' ') {
                start++;
            }

            if (is_included_str(buf, "content-type")) {
                strcpy(hhr->contype, start + 1);
            } else if (is_included_str(buf, "content-length")) {
                strcpy(hhr->conlength, start + 1);
            }
        }

        memset(buf, 0, BUF_LEN);
        if (io_readline(io, buf, BUF_LEN) < 0) {
            error("io read line error", 1);
        }
    }
}


// Determines whether the start of the string contains lowercase "http_str (ex content-type)". Included return 1 else return 0
int is_included_str(char *str, char *http_str) {
    char *cur = str;
    char *cmp = http_str;

    // Delete the start space
    while (*cur == ' ') {
        cur++;
    }

    for (; *cmp != '\0' && tolower(*cur) == *cmp; cur++, cmp++);

    if (*cmp == '\0') {
        // end of 0
        return 1;
    }

    return 0;

}

// format uri, filename, params and return whether dynamic requests or static requests
int parse_uri(char *uri, char *filename, char *name, char *cgiargs){

}


// return server error response
void server_error_response(int socket, char *cause, char *err_num, char *short_msg, char *long_msg) {
    char buf[BUF_LEN], body[BUF_LEN];

    // format http response
    sprintf(body, "<html><title>Server Error</title>");
    sprintf(body, "%s<body style='color:red;'>\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, err_num, short_msg);
    sprintf(body, "%s<p>%s: %s</p>\r\n", body, long_msg, cause);
    sprintf(body, "%s<hr/><em>The Web Server</em>\r\n</body></html>", body);

    // send http response
    sprintf(buf, "HTTP/1.1 %s %s\r\n", err_num, short_msg);
    send(socket, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(socket, buf, strlen(buf), 0);
    sprintf(buf, "Content-Length: %ld\r\n\r\n", strlen(body));
    send(socket, buf, strlen(buf), 0);
    send(socket, body, strlen(body), 0);
}

