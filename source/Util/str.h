#ifndef HEADER_UTIL_STR
#define HEADER_UTIL_STR

#include <regex.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>

int match_regex(const char *pattern, const char *str);
char *trim(char *str);

#endif // !HEADER_UTIL_STR
