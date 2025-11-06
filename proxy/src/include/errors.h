/*
 * This header is used for creating 
 * and interpreting errors numbers. 
 */

#ifndef ERRORS_H
#define ERRORS_H

#define ERR_COULD_NOT_CREATE_THREAD_POOL    69
#define ERR_COULD_NOT_DESTROY_THREAD_POOL   70
#define ERR_CANNOT_START_NOT_INIT           71

/*
 * What function returns string containing what error means.
 */
char* what(int error_num);
#endif
