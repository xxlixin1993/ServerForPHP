#include "../include/fastcgi.h"
#include "../include/log.h"
#include "../include/io.h"
#include "../include/config.h"

/*
 * 构造协议记录头部，返回FCGI_Header结构体
 */
FCGI_Header makeHeader(
        int type,
        int requestId,
        int contentLength,
        int paddingLength) {
    FCGI_Header header;
    header.version = FCGI_VERSION_1;
    header.type = (unsigned char) type;
    header.requestIdB1 = (unsigned char) ((requestId >> 8) & 0xff);
    header.requestIdB0 = (unsigned char) ((requestId) & 0xff);
    header.contentLengthB1 = (unsigned char) ((contentLength >> 8) & 0xff);
    header.contentLengthB0 = (unsigned char) ((contentLength) & 0xff);
    header.paddingLength = (unsigned char) paddingLength;
    header.reserved = 0;
    return header;
}

/*
 * 构造请求开始记录协议体，返回FCGI_BeginRequestBody结构体
 */
FCGI_BeginRequestBody makeBeginRequestBody(int role, int keepConn) {
    FCGI_BeginRequestBody body;
    body.roleB1 = (unsigned char) ((role >> 8) & 0xff);
    body.roleB0 = (unsigned char) (role & 0xff);
    body.flags = (unsigned char) ((keepConn) ? 1 : 0); // 1为长连接，0为短连接
    memset(body.reserved, 0, sizeof(body.reserved));
    return body;
}

/*
 * 发送开始请求记录
 * 发送成功返回0
 * 出错返回-1
 */
int sendBeginRequestRecord(int fd, int requestId) {
    int ret;
    // 构造一个FCGI_BeginRequestRecord结构
    FCGI_BeginRequestRecord beginRecord;

    beginRecord.header =
            makeHeader(FCGI_BEGIN_REQUEST, requestId, sizeof(beginRecord.body), 0);
    beginRecord.body = makeBeginRequestBody(FCGI_RESPONDER, 0);

    ret = send(fd, &beginRecord, sizeof(beginRecord), 0);

    if (ret == sizeof(beginRecord)) {
        return 0;
    } else {
        return -1;
    }
}

/*
 * 发送名值对参数
 * 发送成功返回0
 * 出错返回-1
 */
int sendParamsRecord(int fd, int requestId, char *name, int nlen, char *value, int vlen) {
    unsigned char *buf, *old;
    int ret, pl, cl = nlen + vlen;
    cl = (nlen < 128) ? ++cl : cl + 4;
    cl = (vlen < 128) ? ++cl : cl + 4;

    // 计算填充数据长度
    pl = (cl % 8) == 0 ? 0 : 8 - (cl % 8);
    old = buf = (unsigned char *) malloc(FCGI_HEADER_LEN + cl + pl);

    FCGI_Header nvHeader = makeHeader(FCGI_PARAMS, requestId, cl, pl);
    memcpy(buf, (char *) &nvHeader, FCGI_HEADER_LEN);
    buf = buf + FCGI_HEADER_LEN;

    if (nlen < 128) { // name长度小于128字节，用一个字节保存长度
        *buf++ = (unsigned char) nlen;
    } else { // 大于等于128用4个字节保存长度
        *buf++ = (unsigned char) ((nlen >> 24) | 0x80);
        *buf++ = (unsigned char) (nlen >> 16);
        *buf++ = (unsigned char) (nlen >> 8);
        *buf++ = (unsigned char) nlen;
    }

    if (vlen < 128) { // value长度小于128字节，用一个字节保存长度
        *buf++ = (unsigned char) vlen;
    } else { // 大于等于128用4个字节保存长度
        *buf++ = (unsigned char) ((vlen >> 24) | 0x80);
        *buf++ = (unsigned char) (vlen >> 16);
        *buf++ = (unsigned char) (vlen >> 8);
        *buf++ = (unsigned char) vlen;
    }

    memcpy(buf, name, nlen);
    buf = buf + nlen;
    memcpy(buf, value, vlen);

    ret = send(fd, old, FCGI_HEADER_LEN + cl + pl, 0);

    free(old);

    if (ret == (FCGI_HEADER_LEN + cl + pl)) {
        return 0;
    } else {
        return -1;
    }
}

/*
 * 发送空的params记录
 * 发送成功返回0
 * 出错返回-1
 */
int sendEmptyParamsRecord(int fd, int requestId) {
    int ret;
    FCGI_Header nvHeader = makeHeader(FCGI_PARAMS, requestId, 0, 0);
    ret = send(fd, (char *) &nvHeader, FCGI_HEADER_LEN, 0);

    if (ret == FCGI_HEADER_LEN) {
        return 0;
    } else {
        return -1;
    }
}

/*
 * 发送FCGI_STDIN数据
 * 发送成功返回0
 * 出错返回-1
 */
int sendStdinRecord(int fd, int requestId, char *data, int len) {
    int cl = len, pl, ret;
    char buf[8] = {0};

    while (len > 0) {
        // 判断STDIN数据是否大于传输最大值FCGI_MAX_LENGTH
        if (len > FCGI_MAX_LENGTH) {
            cl = FCGI_MAX_LENGTH;
        }

        // 计算填充数据长度
        pl = (cl % 8) == 0 ? 0 : 8 - (cl % 8);

        FCGI_Header sinHeader = makeHeader(FCGI_STDIN, requestId, cl, pl);
        ret = send(fd, (char *) &sinHeader, FCGI_HEADER_LEN, 0);  // 发送协议头部
        if (ret != FCGI_HEADER_LEN) {
            return -1;
        }

        ret = send(fd, data, cl, 0); // 发送stdin数据
        if (ret != cl) {
            return -1;
        }

        if (pl > 0) {
            ret = send(fd, buf, pl, 0); // 发送填充数据
            if (ret != pl) {
                return -1;
            }
        }

        len -= cl;
        data += cl;
    }

    return 0;
}

/*
 * 发送空的FCGI_STDIN记录
 * 发送成功返回0
 * 出错返回-1
 */
int sendEmptyStdinRecord(int fd, int requestId) {
    int ret;
    FCGI_Header sinHeader = makeHeader(FCGI_STDIN, requestId, 0, 0);
    ret = send(fd, (char *) &sinHeader, FCGI_HEADER_LEN, 0);

    if (ret == FCGI_HEADER_LEN) {
        return 0;
    } else {
        return -1;
    }
}

// create fastcgi socket
int create_fastcgi_fd() {
    int sock;
    struct sockaddr_in name;

    // create socket
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (-1 == sock) {
        error("socket error", 1);
        return -1;
    }

    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = inet_addr(FASTCGI_HOST);
    name.sin_port = htons(FASTCGI_PORT);

    // connect fastcgi
    if (-1 == connect(sock, (struct sockaddr *) &name, sizeof(name))) {
        error("connect fastcgi error", 1);
        return -1;
    }

    return sock;
}

/*
 * 发送http请求行和请求体数据给fastcgi服务器
 */
int send_fastcgi(struct io_class *io, struct http_header *hhr, int sock) {
    int requestId, i, l;

    requestId = sock;
    char *buf;

    // params参数名
    char *paname[] = {
            "SCRIPT_FILENAME",
            "SCRIPT_NAME",
            "REQUEST_METHOD",
            "REQUEST_URI",
            "QUERY_STRING",
            "CONTENT_TYPE",
            "CONTENT_LENGTH"
    };

    // 对应上面params参数名，具体参数值所在http_header结构体中的偏移
    int paoffset[] = {
            (size_t) &(((struct http_header *) 0)->filename),
            (size_t) &(((struct http_header *) 0)->name),
            (size_t) &(((struct http_header *) 0)->method),
            (size_t) &(((struct http_header *) 0)->uri),
            (size_t) &(((struct http_header *) 0)->cgiargs),
            (size_t) &(((struct http_header *) 0)->contype),
            (size_t) &(((struct http_header *) 0)->conlength)
    };

    // 发送开始请求记录
    if (sendBeginRequestRecord(sock, requestId) < 0) {
        error("sendBeginRequestRecord error", 1);
        return -1;
    }

    // TODO ERROR
    // 发送params参数
    l = sizeof(paoffset) / sizeof(paoffset[0]);
    for (i = 0; i < l; i++) {
        // params参数的值不为空才发送
        if (strlen((char *) (((int) hhr) + paoffset[i])) > 0)
        {
            if (sendParamsRecord(sock, requestId, paname[i], strlen(paname[i]),
                                 (char *) (((int) hhr) + paoffset[i]),
                                 strlen((char *) (((int) hhr) + paoffset[i]))) < 0) {
                error("sendParamsRecord error", 1);
                return -1;
            }
        }
    }

    // 发送空的params参数
    if (sendEmptyParamsRecord(sock, requestId) < 0) {
        error("sendEmptyParamsRecord error", 1);
        return -1;
    }

    // 继续读取请求体数据
    l = atoi(hhr->conlength);
    if (l > 0) { // 请求体大小大于0
        buf = (char *) malloc(l + 1);
        memset(buf, '\0', l);
        if (io_read(io, buf, l) < 0) {
            error("io_readn error", 1);
            free(buf);
            return -1;
        }

        // 发送stdin数据
        if (sendStdinRecord(sock, requestId, buf, l) < 0) {
            error("sendStdinRecord error", 1);
            free(buf);
            return -1;
        }

        free(buf);
    }

    // 发送空的stdin数据
    if (sendEmptyStdinRecord(sock, requestId) < 0) {
        error("sendEmptyStdinRecord error", 1);
        return -1;
    }
    return 0;
}

/*
 * php处理结果发送给客户端
 */
int send_to_cli(int fd, int outlen, char *out,
                int errlen, char *err, FCGI_EndRequestBody *endr
) {
    char *p;
    int n;

    char buf[BUF_LEN];
    sprintf(buf, "HTTP/1.1 200 OK\r\n");
    sprintf(buf, "%sServer: Zhou Web Server\r\n", buf);
    sprintf(buf, "%sContent-Length: %d\r\n", buf, outlen + errlen);
    sprintf(buf, "%sContent-Type: %s\r\n\r\n", buf, "text/html");
    if (send(fd, buf, strlen(buf), 0) < 0) {
        error("write to client error", 1);
    }

    if (outlen > 0) {
        p = index(out, '\r');
        n = (int) (p - out);
        if (send(fd, p + 3, outlen - n - 3, 0) < 0) {
            error("rio_written error", 1);
            return -1;
        }
    }

    if (errlen > 0) {
        if (send(fd, err, errlen, 0) < 0) {
            error("rio_written error", 1);
            return -1;
        }
    }
}

/*
 * 读取php-fpm处理结果
 * 读取成功返回0
 * 出错返回-1
 */
int recvRecord(read_record rr, send_to_client stc, int cfd, int fd, int requestId) {
    FCGI_Header responHeader;
    FCGI_EndRequestBody endr;
    char *conBuf = NULL, *errBuf = NULL;
    int buf[8], cl, ret;
    int fcgi_rid;  // 保存fpm发送过来的request id

    int outlen = 0, errlen = 0;


    // 读取协议记录头部
    while (rr(fd, &responHeader, FCGI_HEADER_LEN) > 0) {
        fcgi_rid = (int) (responHeader.requestIdB1 << 8) + (int) (responHeader.requestIdB0);
        if (responHeader.type == FCGI_STDOUT && fcgi_rid == requestId) {
            // 获取内容长度
            cl = (int) (responHeader.contentLengthB1 << 8) + (int) (responHeader.contentLengthB0);
            //*outlen += cl;
            outlen += cl;

            // 如果不是第一次读取FCGI_STDOUT记录
            if (conBuf != NULL) {
                // 扩展空间
                //conBuf = realloc(*sout, *outlen);
                conBuf = realloc(conBuf, outlen);
            } else {
                conBuf = (char *) malloc(cl);
                //*sout = conBuf;
            }

            ret = rr(fd, conBuf, cl);
            if (ret == -1 || ret != cl) {
                printf("read fcgi_stdout record error\n");
                return -1;
            }

            // 读取填充内容，忽略
            if (responHeader.paddingLength > 0) {
                ret = rr(fd, buf, responHeader.paddingLength);
                if (ret == -1 || ret != responHeader.paddingLength) {
                    printf("read fcgi_stdout padding error %d\n", responHeader.paddingLength);
                    return -1;
                }
            }
        } else if (responHeader.type == FCGI_STDERR && fcgi_rid == requestId) {
            // 获取内容长度
            cl = (int) (responHeader.contentLengthB1 << 8) + (int) (responHeader.contentLengthB0);
            //*errlen += cl;
            errlen += cl;

            // 如果不是第一次读取FCGI_STDOUT记录
            if (errBuf != NULL) {
                // 扩展空间
                errBuf = realloc(errBuf, errlen);
            } else {
                errBuf = (char *) malloc(cl);
                //*serr = errBuf;
            }

            ret = rr(fd, errBuf, cl);
            if (ret == -1 || ret != cl) {
                return -1;
            }

            // 读取填充内容，忽略
            if (responHeader.paddingLength > 0) {
                ret = rr(fd, buf, responHeader.paddingLength);
                if (ret == -1 || ret != responHeader.paddingLength) {
                    return -1;
                }
            }
        } else if (responHeader.type == FCGI_END_REQUEST && fcgi_rid == requestId) {
            // 读取结束请求协议体
            ret = rr(fd, &endr, sizeof(FCGI_EndRequestBody));

            if (ret == -1 || ret != sizeof(FCGI_EndRequestBody)) {
                free(conBuf);
                free(errBuf);
                return -1;
            }

            stc(cfd, outlen, conBuf, errlen, errBuf, &endr);
            free(conBuf);
            free(errBuf);
            return 0;
        }
    }
    return 0;
}


/*
 * 接收fastcgi返回的数据
 */
int recv_fastcgi(int fd, int sock) {
    int requestId;

    requestId = sock;

    // 读取处理结果
    if (recvRecord(io_read, send_to_cli, fd, sock, requestId) < 0) {
        error("recvRecord error", 1);
        return -1;
    }

    return 0;
}
