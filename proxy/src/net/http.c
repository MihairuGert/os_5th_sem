#include "../include/http.h"

int parse_http_req(http_req_t* req, char* bytes, int size)
{
    char* http_string = malloc(size + 1);

    strncpy(http_string, bytes, size);
    http_string[size] = '\0';

    req->num_headers = sizeof(req->headers) / sizeof(req->headers[0]);

    int bytes_used = phr_parse_request(
        http_string, size, &req->method, &req->method_len, &req->path,
        &req->path_len, &req->minor_version, req->headers, &req->num_headers, 0);

    if (bytes_used <= 0) {
        if (bytes_used == -1) {
            printf("[PARSE] Error: parse error\n");
        } else if (bytes_used == -2) {
            printf("[PARSE] Error: not enough\n");
        } else {
            printf("[PARSE] Error: unknown error (%d)\n", bytes_used);
        }
        return -1;
    }

    if (bytes_used <= 0)
        return -1;
    return 0;
}

int parse_http_resp(http_resp_t* resp, char* bytes, int size)
{
    bytes[size] = '\0';
    resp->num_headers = sizeof(resp->headers) / sizeof(resp->headers[0]);

    int bytes_used = phr_parse_response(bytes, size, &resp->minor_version,
        &resp->status, &resp->msg, &resp->msg_len,
        resp->headers, &resp->num_headers, 0);

    if (bytes_used <= 0)
        return -1;
    return 0;
}
