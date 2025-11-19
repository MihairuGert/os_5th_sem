/*
 * This header is used for various proxy settings.
 */

#ifndef CONFIG_H
#define CONFIG_H

/*
 * Master settings.
 */
#define DEFAULT_PORT 80
#define BACKLOG     100

/*
 * Backend settings.
 */
#define MAX_BACKEND_NUM             10
#define MAX_BACKEND_CONNECTIONS_NUM 10

/*
 * Cache settings.
 */
#define MAX_CACHE_SIZE      (100 * 1024 * 1024)
#define MAX_CACHE_ENTRIES   1000
#define BUFFER_SIZE         8192

#endif
