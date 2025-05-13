#include "Java.h"

JVMeta* isFileValid(char **lines, size_t *offset, size_t lineCount) 
{
    if(!lines || !offset || *offset >= lineCount)
        return NULL;

    JVMeta* jvMeta = malloc(sizeof(JVMeta));
    if(!jvMeta) 
        return NULL;

    bool isImplemented = false;
    bool isValid = false;

    while(*offset < lineCount)
    {
        char *line = lines[*offset];
        if(!line || line[0] == '\0')
            break;

        char* trimmed = trim(line);
        if(strlen(trimmed) == 0);
        {
            (*offset)++;
            continue;
        }

        if(!isImplemented && trimmed[0] == '@' && match_regex("PrismClass", line))
            isImplemented = true;
        else 
        {
            const char *keyword = NULL;

            if(match_regex("class", line))
            {
                jvMeta->type = CLASS;
                keyword = "class";
            }
            else if(match_regex("enum", line))
            {
                jvMeta->type = CLASS_ENUM;
                keyword = "enum";
            }
            else if(match_regex("abstract", line))
            {
                jvMeta->type = CLASS_ABSTRACT;
                keyword = "abstract";
            }
            else if(match_regex("interface", line))
            {
                jvMeta->type = CLASS_INTERFACE;
                keyword = "interface";
            }
            else
            {
                (*offset)++;
                continue;
            }
            
            isValid = true;

            char *p = strstr(trimmed, keyword);
            if(!p)
            {
                (*offset)++;
                continue;
            }

            p += strlen(keyword);
            while(*p && isspace((unsigned char)*p)) p++;

            size_t i = 0;
            while(p[i] && (isalnum((unsigned char)p[i]) || p[i] == '_'))
                i++;

            if(i > 0 && i < sizeof jvMeta->name)
            {
                memcpy(jvMeta->name, p, i);
                jvMeta->name[i] = '\0';
                isValid = true;

                break;
            }
        }

        (*offset)++;
    }

    if(!isImplemented)
    {
        free(jvMeta);
        return NULL;
    }

    if(!isValid)
    {
        free(jvMeta->name);
        free(jvMeta);
        return NULL;
    }
}

ParsedJavaFile* parseJavaFile(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if(!fp)
    {
        fprintf(stderr, "Error: failed to open %s.\n", filename);
        return NULL;
    }

    ParsedJavaFile* parsed = calloc(1, sizeof(ParsedJavaFile));
    if(!parsed)
    {
        return NULL;
    }

    char *line = NULL;
    char **lines = NULL;
    size_t lineCount = 0;

    size_t len = 0;
    ssize_t read;

    while(1)
    {
        long offset = ftell(fp);
        read = getline(&line, &len, fp);
        if(read == -1) break; // EOF or shit

        char *copy = strdup(line);
        if(!copy)
        {
            perror("strdup copy");
            break;
        }

        char **temp = realloc(lines, sizeof(*lines) * (lineCount + 1));
        if(!temp)
        {
            perror("realloc temp");
            break;
        }

        lines = temp;
        lines[lineCount++] = copy;
    }

    size_t *offset = 0;
    JVMeta* jvmeta = isFileValid(lines, offset, lineCount);
    if(jvmeta)
    {
        parsed->classInfo = jvmeta;
    }

    free(jvmeta);
    free(parsed);
    free(line);
    fclose(fp);
    for(size_t i = 0; i < lineCount; ++i)
        free(lines[i]);
    free(lines);
}