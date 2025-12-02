/*
 * This header is used for various proxy settings.
 */

#ifndef CONFIG_H
#define CONFIG_H

/*
 * Master settings.
 */
#define DEFAULT_PORT 80
#define BACKLOG 100

/*
 * Backend settings.
 */
#define MAX_BACKEND_NUM 10
#define MAX_BACKEND_CONNECTIONS_NUM 10
#define PACKET_SIZE 16384

/*
 * Cache settings.
 */
#define MAX_CACHE_SIZE 10 * 1024 * 1024
#define MAX_CACHE_ENTRIES 128
#define BUCKET_COUNT 64
#define DEFAULT_TTL 100

#endif
