/*
 Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
 embodied in the content of this file are licensed under the BSD
 (revised) open source license
 */

#include "parse_primitives.h"
#include <string.h>
#include <ctype.h>
#include <iostream>
#include "utils.h"

void
tokenize(char delim, substring s, v_array<substring>& ret)
{
    ret.erase();
    char *last = s.start;
    for (; s.start != s.end; s.start++) {
        if (*s.start == delim) {
            if (s.start != last) {
                substring temp =
                    { last, s.start };
                push(ret, temp);
                //std::cout << "Start is " << s.start << std::endl;
            }
            last = s.start + 1;
        }
    }
    if (s.start != last) {
        substring final =
            { last, s.start };
        push(ret, final);
    }
}

char *
trim(char *str)
{
    size_t len = 0;
    char *frontp = str - 1;
    char *endp = NULL;

    if (str == NULL)
        return NULL;

    if (str[0] == '\0')
        return str;

    len = c_strlen(str);
    endp = str + len;

    /* Move the front and back pointers to address
     * the first non-whitespace characters from
     * each end.
     */
    while (isspace(*(++frontp)))
        ;
    while (isspace(*(--endp)) && endp != frontp)
        ;

    if (str + len - 1 != endp)
        *(endp + 1) = '\0';
    else if (frontp != str && endp == frontp)
        *str = '\0';

    /* Shift the string so that it starts at str so
     * that if it's dynamically allocated, we can
     * still free it on the returned pointer.  Note
     * the reuse of endp to mean the front of the
     * string buffer now.
     */
    endp = str;
    if (frontp != str) {
        while (*frontp)
            *endp++ = *frontp++;
        *endp = '\0';
    }
    return str;
}

bool
startswith(const char *str, const char *search_str)
{
    int str_len = c_strlen(search_str);
    bool startswith = false;
    if (str_len > 0) {
        for (int i = 0; i < str_len; i++) {
            if (str[i] == search_str[i]) {
                if (i == str_len - 1) {
                    startswith = true;
                    break;
                }
            }
            else {
                break;
            }
        }
    }
    return startswith;
}
