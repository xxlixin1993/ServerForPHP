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
int parse_uri(char *uri, char *filename, char *name, char *cgiargs) {
    char *query, *ptr;
    char *suffix = ".php";
    char uri_copy[BUF_LEN], dir[BUF_LEN];
    strcpy(uri_copy, uri);

    //get work dir
    getcwd(dir, BUF_LEN);
    strcat(dir, "/../test");

    if (query = strstr(uri_copy, suffix)) {
        // dynamic requests
        // extract query parameters
        ptr = index(uri_copy, '?');
        if (ptr) {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        } else {
            // like "index.php/class/method" will be extracted "class/method" param
            if (*(query + sizeof(suffix)) == '/') {
                strcpy(cgiargs, query + sizeof(suffix) + 1);
                *(query + sizeof(suffix)) = '\0';
            }
        }

        // contains the full path name
        strcpy(filename, dir);
        strcat(filename, uri_copy);
        // not contain the full path name
        strcpy(name, uri_copy);

        return 0;
    } else {
        // static requests
        strcpy(cgiargs, "");

        // delete useless parameters, like "index.html?12312313"
        ptr = index(uri_copy, '?');
        if (ptr) {
            *ptr = '\0';
        }

        strcpy(filename, dir);
        strcat(filename, uri_copy);
        // if end of '/', add index.html
        if (uri_copy[strlen(uri_copy) - 1] == '/') {
            strcat(filename, "index.html");
        }

        return 1;
    }
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


// judge the file type based on the file suffix
void get_file_type(char *filename, char *filetype) {
    if (strstr(filename, ".html")) {
        strcpy(filetype, "text/html");
    } else if (strstr(filename, ".gif")) {
        strcpy(filetype, "image/gif");
    } else if (strstr(filename, ".jpg")) {
        strcpy(filetype, "image/jpeg");
    } else if (strstr(filename, ".png")) {
        strcpy(filetype, "image/png");
    } else {
        strcpy(filetype, "text/plain");
    }
}

// read the contents of the static file, sent to the client
void server_static(int fd, char *filename, int file_size) {
    int srcfd;
    char *srcp, file_type[BUF_LEN], buf[BUF_LEN];

    // get MIME type
    get_file_type(filename, file_type);
    sprintf(buf, "HTTP/1.1 200 OK\r\n");
    sprintf(buf, "%sServer: Web Server\r\n", buf);
    sprintf(buf, "%sContent-Length: %d\r\n", buf, file_size);
    sprintf(buf, "%sContent-Type: %s\r\n\r\n", buf, file_type);
    if (send(fd, buf, strlen(buf), 0) < 0) {
        error("write to client error", 1);
    }

    if ((srcfd = open(filename, O_RDONLY, 0)) < 0) {
        error("open file error", 1);
    }

    if ((srcp = mmap(0, file_size, PROT_READ, MAP_PRIVATE, srcfd, 0)) == ((void *) -1)) {
        error("mmap error", 1);
    }
    close(srcfd);
    if (send(fd, srcp, file_size, 0) < 0) {
        error("send to client error", 1);
    }

    if (munmap(srcp, file_size) < 0) {
        error("munmap error", 1);
    }
}

// handle dynamic php requests
void server_dynamic(struct io_class *io, struct http_header *hhr) {

}