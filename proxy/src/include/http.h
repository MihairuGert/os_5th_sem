#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../lib/picohttpparser/picohttpparser.h"

#define MAX_HEADER_NUM 100

typedef struct {
    const char* method;
    size_t method_len;

    const char* path;
    size_t path_len;

    int minor_version;

    struct phr_header headers[MAX_HEADER_NUM];
    size_t num_headers;
} http_req_t;

typedef struct {
    int minor_version;

    int status;

    const char* msg;
    size_t msg_len;

    struct phr_header headers[MAX_HEADER_NUM];
    size_t num_headers;
} http_resp_t;

int parse_http_req(http_req_t* req, char* bytes, int size);
int parse_http_resp(http_resp_t* resp, char* bytes, int size);

#endif
