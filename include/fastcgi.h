//
// Created by lixin on 17-9-21.
//

#ifndef SERVERFORPHP_FASTCGI_H
#define SERVERFORPHP_FASTCGI_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "./http.h"


/*
 * fastcgi协议报头
 */
typedef struct {
    unsigned char version;          // 版本
    unsigned char type;             // 协议记录类型
    unsigned char requestIdB1;      // 请求ID
    unsigned char requestIdB0;
    unsigned char contentLengthB1;  // 内容长度
    unsigned char contentLengthB0;
    unsigned char paddingLength;    // 填充字节长度
    unsigned char reserved;         // 保留字节
} FCGI_Header;

/*
 * 请求开始记录的协议体
 */
typedef struct {
    unsigned char roleB1;   // web服务器期望php-fpm扮演的角色
    unsigned char roleB0;
    unsigned char flags;    // 控制连接响应后是否立即关闭
    unsigned char reserved[5];
} FCGI_BeginRequestBody;


/*
 * 开始请求记录结构，包含开始请求协议头和协议体
 */
typedef struct {
    FCGI_Header header;
    FCGI_BeginRequestBody body;
} FCGI_BeginRequestRecord;


/*
 * 可用于FCGI_Header的type组件的值
 */
#define FCGI_BEGIN_REQUEST  1   // 请求开始记录类型
#define FCGI_ABORT_REQUEST  2
#define FCGI_END_REQUEST    3   // 响应结束记录类型
#define FCGI_PARAMS         4   // 传输名值对数据
#define FCGI_STDIN          5   // 传输输入数据，例如post数据
#define FCGI_STDOUT         6   // php-fpm响应数据输出
#define FCGI_STDERR         7   // php-fpm错误输出
#define FCGI_DATA           8


#define FASTCGI_HOST       "127.0.0.1"      // php-fpm host
#define FASTCGI_PORT       9000             // php-fpm listen port
#define FCGI_MAX_LENGTH    0xFFFF           // 允许传输的最大数据长度65536
#define FCGI_VERSION_1     1                // fastcgi协议版本

/*
 * 期望php-fpm扮演的角色值
 */
#define FCGI_RESPONDER  1
#define FCGI_AUTHORIZER 2
#define FCGI_FILTER     3

// 为1，表示php-fpm响应结束不会关闭该请求连接
#define FCGI_KEEP_CONN  1

#define FCGI_HEADER_LEN     8       // 协议包头长度

// create fastcgi socket
int create_fastcgi_fd();

// send fastcgi request
int send_fastcgi(struct io_class *io, struct http_header *hhr, int sock);

#endif //SERVERFORPHP_FASTCGI_H
