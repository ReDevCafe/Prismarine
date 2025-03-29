#include "str.h"

int match_regex(const char *pattern, const char *str) {
    regex_t regex;
    int result;

    regcomp(&regex, pattern, REG_EXTENDED);
    result = regexec(&regex, str, 0, NULL, 0);
    regfree(&regex);

    return (result == 0);
}

char *trim(char *str)
{
    char *end;
    while(isspace((unsigned char)*str)) ++str;
    if(*str == 0) return str;

    end = str + strlen(str) -1;
    while(end > str && isspace((unsigned char)*end)) --end;
    *(end+1) = '\0';

    return str;
}