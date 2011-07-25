/*
 * utils.h
 *
 *  Created on: Jul 1, 2010
 *      Author: caglar
 */
#ifndef UTILS_H_
#define UTILS_H_

#include <sys/time.h>

#define IN
#define OUT

//----------------------------------------
//C_ASSERT
//----------------------------------------
#define C_ASSERT(x) \
do{if (!(x)) { \
            fprintf(stdout, "\nASSERT FAILED in file %s, line %d\n\n", __FILE__, __LINE__); \
            fflush(stdout); \
                        exit(-1); \
}}while(0)

/*
 * IO Related functions
 */
void
c_log(IN const char * format, ...);
void
c_write_file(IN const char *filename, IN void *buf, IN size_t size);
void
c_append_to_file(IN const char *filename, IN void *buf, IN size_t size);
void
c_read_file(IN const char *filename, IN size_t offset, IN size_t size);
size_t
c_get_file_size(IN const char *filename);
int
get_no_cols(IN const char *filename);
int
get_no_rows(IN const char *filename);

/*
 * Time Related Functions
 */
int
c_timeval_subtract(OUT struct timeval *result, IN struct timeval *x,
        IN struct timeval *y);
void
c_start_timer(OUT struct timeval *start);
void
c_end_timer(OUT struct timeval *end);

/*
 * Memory Related Functions
 */
void *
c_malloc(IN size_t size);
void
c_array_free(OUT char **buf, IN size_t size);
void
c_free(OUT void *buf);
void
c_memset(OUT void *buf, IN int c, IN size_t size);
void
c_memcpy(OUT void *dest, IN const void *src, IN size_t size);
void *
c_realloc(OUT void *buf, IN size_t newSize);
void *
c_calloc(size_t nmemb, size_t size);

/*
 *  String related Functions
 */
char *
c_strdup(IN const char *str);
int
c_strlen(IN const char *str);
char *
c_strcpy(OUT char *destination, IN const char *source);
bool
c_is_str_empty(const char *str);
char *
c_strtok(IN char * str, IN const char * delimiters);
char *
c_strtok_r(IN char *str1, IN const char *str2, IN char **str3);
bool
c_contains(const char *s1, const char *s2);
char*
c_trim(char *str);

/*
 *  Math related Functions
 */
bool
is_missing_value(float val);

#endif /* UTILS_H_ */
