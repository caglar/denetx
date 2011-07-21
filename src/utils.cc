/*
 * utils.c
 *
 *  Created on: Jul 1, 2010
 *      Author: caglar
 */

/**
 * This file contains wrapper and utility functions
 * for safer and better performance
 */

#include <stdio.h>
#include <stdlib.h>
#include <jemalloc/jemalloc.h>
#include <stdarg.h>
#include <sys/syslog.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include <math.h>
#include "utils.h"

#define MAX_CHAR_LINE 200

#define USECSPERSEC 1000000

/* IO Related functions */
void
c_log(const char *format, ...)
{
    if (format != NULL) {
        va_list ap;
        va_start(ap, format);
        vprintf(format, ap);
        va_end(ap);
    }
    else {
        syslog(LOG_ALERT,
                "utils.c: Illegal format input is given to the c_log function.");
    }
}

void
c_write_file(const char *filename, void *buf, size_t size)
{
    if (c_is_str_empty(filename)) {
        syslog(LOG_CRIT, "utils.c: write_to_file function file is empty ");
    }
    if (buf == NULL) {
        syslog(LOG_CRIT, "utils.c: write_to_file function file is empty ");
    }
    else if (!c_is_str_empty(filename)) {
        FILE *p = NULL;
        size_t len = 0;
        p = fopen(filename, "w");
        if (p == NULL) {
            syslog(LOG_CRIT, "utils.c: Error in opening the file %s.\n",
                    filename);
        }
        len = c_strlen((char *) buf);
        fwrite((char *) buf, len, 1, p);
    }
}

void
c_append_file(const char *filename, void *buf, size_t size)
{
    if (c_is_str_empty(filename)) {
        syslog(LOG_CRIT, "write_to_file function file is empty ");
    }
    if (buf == NULL) {
        syslog(LOG_CRIT, "write_to_file function file is empty ");
    }
    if (!c_is_str_empty(filename) && buf != NULL) {
        FILE *p = NULL;
        size_t len = 0;
        p = fopen(filename, "a");
        if (p == NULL) {
            syslog(LOG_CRIT, "Error in opening the file %s.\n", filename);
        }
        len = c_strlen((char *) buf);
        fwrite(buf, len, 1, p);
    }
}

size_t
c_get_file_size(const char *filename)
{
    size_t size = 0;
    if (!c_is_str_empty(filename)) {
        FILE *f = fopen(filename, "r");
        fseek(f, 0, SEEK_END); // seek to end of file
        size = ftell(f); // get current file pointer
        fseek(f, 0, SEEK_SET); // seek back to beginning of file

    }
    else {
        syslog(LOG_ALERT,
                "utils.c: NULL filename is given to the c_write_file function");
    }
    return size;
}

int
get_no_cols(const char *filename)
{
    int no_of_cols = 1;
    char *line = NULL;
    size_t len = 0;
    int read;

    if (!c_is_str_empty(filename)) {
        FILE *f = fopen(filename, "r");
        if (!f) {
            return 0;
        }
        while ((read = getline(&line, &len, f)) != -1) {
            if (line[0] != '#' && line[0] != '%' && line[0] != '@' && strlen(
                    line) > 5) {
                for (int i = 0; line[i] != '\0'; i++) {
                    if (line[i] == ',') {
                        no_of_cols++;
                    }
                }
                break;
            }
        }
        fclose(f);
    }
    return no_of_cols;
}

int
get_no_rows(const char *filename)
{
    int no_of_rows = 0;
    char *line = NULL;
    size_t len = 0;
    int read;

    if (!c_is_str_empty(filename)) {
        FILE *f = fopen(filename, "r");
        if (!f) {
            return 0;
        }
        while ((read = getline(&line, &len, f)) != -1) {
            if (line[0] != '#' && line[0] != '%' && line[0] != '@' && strlen(
                    line) > 5) {
                no_of_rows++;
            }
        }
        fclose(f);
    }
    return no_of_rows;
}

/* Time Related Functions */

/**
 * this function is for computing the time difference between timeval x and y
 * the result is stored in result
 */
int
c_timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y)
{
    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_usec < y->tv_usec) {
        int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
        y->tv_usec -= 1000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_usec - y->tv_usec > 1000000) {
        int nsec = (x->tv_usec - y->tv_usec) / 1000000;
        y->tv_usec += 1000000 * nsec;
        y->tv_sec -= nsec;
    }

    /*
     * Compute the time remaining to wait.
     * tv_usec is certainly positive.
     */
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_usec = x->tv_usec - y->tv_usec;

    /* Return 1 if result is negative. */
    return x->tv_sec < y->tv_sec;
}

void
c_start_timer(struct timeval *start)
{
    gettimeofday(start, NULL);
}

void
c_end_timer(struct timeval *end)
{
    gettimeofday(end, NULL);
}

/* Memory Related Functions */
void *
c_malloc(size_t size)
{
    void *ret = malloc(size);
    if (!ret && !size)
        ret = malloc(1);
    if (!ret) {
        //release_pack_memory(size, -1);
        //And clear from the cache as the size of the variable
        ret = malloc(size);
        if (!ret && !size)
            ret = malloc(1);
        if (!ret) {
            syslog(LOG_CRIT, "Out of memory, malloc failed");
            exit(EXIT_FAILURE);
        }
    }
#ifdef XMALLOC_POISON
    c_memset(ret, 0xA5, size);
#endif
    return ret;
}

void
c_free(void *buf)
{
    if (buf == NULL) {
        syslog(LOG_CRIT, "utils.c: c_free function recieved an illegal buffer");
        exit(EXIT_FAILURE);
    }
    else
        free(buf);
}

void
c_array_free(char **buf, size_t size)
{
    if (size != 0) {
        if (buf == NULL) {
            syslog(LOG_CRIT,
                    "utils.c: c_free function recieved an illegal buffer");
            exit(EXIT_FAILURE);
        }
        else {
            free(buf);
            *buf = NULL;
        }
    }
}

void
c_memset(void *buf, int c, size_t size)
{
    if (buf == NULL) {
        syslog(LOG_CRIT, "utils.c: c_memset buff recieved is null");
        exit(EXIT_FAILURE);
    }
    if (c <= 0) {
        syslog(LOG_CRIT, "utils.c: c_memset buff recieved is null");
        exit(EXIT_FAILURE);
    }
    if (buf != NULL && c > 0) {
        memset(buf, c, size);
    }
}

void
c_memcpy(void *dest, const void * src, size_t size)
{
    if (dest == NULL) {
        syslog(LOG_CRIT, "utils.c: c_memcpy dest recieved is null");
        exit(EXIT_FAILURE);
    }
    if (src == NULL) {
        syslog(LOG_CRIT, "utils.c: c_memcpy src recieved is null");
        exit(EXIT_FAILURE);
    }
    if (size <= 0) {
        syslog(LOG_CRIT, "utils.c: c_memcpy size recieved is null");
        exit(EXIT_FAILURE);
    }
    if (dest != NULL && src != NULL && size > 0) {
        //memset(buf, c, size);
        memcpy(dest, src, size);
    }
}

void *
c_realloc(void *ptr, size_t size)
{
    void *ret = realloc(ptr, size);
    if (!ret && !size)
        ret = realloc(ptr, 1);
    if (!ret) {
        // Clear the cache as the size of the item.
        // release_pack_memory(size, -1);
        ret = realloc(ptr, size);
        if (!ret && !size)
            ret = realloc(ptr, 1);
        if (!ret) {
            syslog(LOG_CRIT, "Out of memory, realloc failed");
            exit(EXIT_FAILURE);
        }
    }
    return ret;

}

void *
c_calloc(size_t nmemb, size_t size)
{
    void *ret = calloc(nmemb, size);
    if (!ret && (!nmemb || !size))
        ret = calloc(1, 1);
    if (!ret) {
        //release_pack_memory(nmemb * size, -1);
        //Clear the cache as the size of the member
        ret = calloc(nmemb, size);
        if (!ret && (!nmemb || !size)) {
            ret = calloc(1, 1);
        }
        if (!ret) {
            syslog(LOG_CRIT, "Out of memory, calloc failed");
        }
    }
    return ret;
}

/* String Functions */
char *
c_strdup(const char *str)
{
    assert(str != NULL);
    char *buf = (char *) strdup(str);
    assert(buf != NULL);
    return buf;
}

int
c_strlen(const char *str)
{
    int return_val = 0;
    if (str == NULL) {
        return_val = -1;
    }
    else {
        return_val = strlen(str);
    }
    return return_val;
}

char *
c_strtok_r(char *str1, const char *str2, char **str3)
{
    char *token = (char *) strtok_r(str1, str2, str3);
    return token;
}

char *
c_strtok(char * str, const char * delimiters)
{
    assert( str != NULL );
    assert( delimiters != NULL );
    char *result = strtok(str, delimiters);
    assert( result!= NULL );
    return result;
}

char *
c_strcpy(char *destination, const char *source)
{
    assert(source != NULL);
    if (sizeof(destination) >= sizeof(source)) {
        // printf("Size of src is : %u dest is: %u\n", sizeof(*src), sizeof(*dest) );
        strcpy(destination, source);
    }
    else {
        destination = (char *) c_realloc((void *) destination,
                sizeof(source) + 1);
        if (destination == NULL) {
            syslog(LOG_CRIT, "Not enough memory c_strcpy failed.");
        }
    }
    return destination;
}

bool
c_is_str_empty(const char *str)
{
    if (str == NULL) {
        return 1;
    }
    else if (c_strlen(str) == 0) {
        return 1;
    }
    return 0;
}

bool
c_contains(const char *s1, const char *s2)
{
    bool result = false;
    if (!c_is_str_empty(s1) && !c_is_str_empty(s2)) {
        if (c_strlen(s1) > c_strlen(s2)) {
            if (strcasestr(s1, s2) != NULL) {
                result = true;
            }
        }
    }
    return result;
}

bool
is_missing_value(double val)
{
    bool ret = false;
    if (isnan(val)) {
        ret = true;
    }
    return ret;
}

/* END OF utils.h */
