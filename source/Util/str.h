#ifndef HEADER_UTIL_STR
#define HEADER_UTIL_STR

#include <regex.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

int match_regex(const char *pattern, const char *str);
char *trim(char *str);
int *nextLineHas(const *pattern, char **lines, size_t *offset);
char* extractAnotValue(const char* line, const char* key);

#endif // !HEADER_UTIL_STR
