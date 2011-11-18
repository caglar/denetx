/*
 Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
 embodied in the content of this file are licensed under the BSD
 (revised) open source license
 */

#ifndef PP
#define PP

#include "v_array.h"
#include<iostream>
using namespace std;

struct substring
{
    char *start;
    char *end;
};

//chop up the string into a v_array of substring.
void
tokenize(char delim, substring s, v_array<substring> &ret);

char *
trim(char *str);

bool
startswith(const char *str, const char *search_str);

inline char*
safe_index(char *start, char v, char *max)
{
    while (start != max && *start != v)
        start++;
    return start;
}

inline void
print_substring(substring s)
{
    cout.write(s.start, s.end - s.start);
}

inline float
float_of_substring(substring s)
{
  char* endptr = s.end;
  float f = strtof(s.start,&endptr);
  if (endptr == s.start && s.start != s.end)
    {
      std::cout << "error: " << std::string(s.start, s.end-s.start).c_str() << " is not a float" << std::endl;
      f = 0;
    }
  return f;

}

inline float
zero_copy_float_of_substring(substring s)
{
    char temp = *s.end;
    *s.end = '\0';
    float f = atof(s.start);
    *s.end = temp;
    return f;
}

inline double
double_of_substring(substring s)
{
  char* endptr = s.end;
  float f = strtod(s.start,&endptr);
  if (endptr == s.start && s.start != s.end)
    {
      std::cout << "error: " << std::string(s.start, s.end-s.start).c_str() << " is not a double" << std::endl;
      f = 0;
    }
  return f;

}

inline int
int_of_substring(substring s)
{
    return atoi(string(s.start, s.end - s.start).c_str());
}

inline unsigned long
ulong_of_substring(substring s)
{
    return strtoul(string(s.start, s.end - s.start).c_str(), NULL, 10);
}

inline unsigned long
ss_length(substring s)
{
  return (s.end - s.start);
}

#endif
