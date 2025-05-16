#include "str.h"

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