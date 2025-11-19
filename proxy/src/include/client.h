#ifndef CLIENT_H
#define CLIENT_H

#include "../../lib/libuv/include/uv.h"

typedef struct {
    uv_tcp_t *client;
    //cache_t *cache;
} client_context_t;

#endif
