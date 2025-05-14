#include "str.h"

int match_regex(const char *pattern, const char *str) {
    regex_t regex;
    int result;

    if(regcomp(&regex, pattern, REG_EXTENDED) != 0) return 0;
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

int *nextLineHas(const *pattern, char **lines, size_t *offset)
{
    char *line = lines[*offset + 1];
    if(!line) return 0;

    return match_regex(pattern, line);
}

char* extractAnotValue(const char* line, const char* key)
{
    const char *p = strstr(line,key);
    if(!p) return NULL;

    p = strchr(p, '=');
    if(!p) return NULL;

    p++;
    while(isspace(*p)) p++;

    if(*p != '"') return NULL;
    p++;

    const char* start = p;
    while (*p && *p != '"') p++;
    if(*p != '"') return NULL;

    size_t len = p - start;
    char* result = (char *) malloc(len + 1);
    if(!result) return;

    strncpy(result, start, len);
    result[len] = '\0';

    return result;
}