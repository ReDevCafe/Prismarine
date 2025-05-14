#include "Java.h"

JVMeta* isFileValid(char **lines, size_t *offset, size_t lineCount) 
{
    if(!lines || *offset >= lineCount) return NULL;

    JVMeta* jvMeta = malloc(sizeof(JVMeta));
    if(!jvMeta) return NULL;

    bool isImplemented = false;
    bool isValid = false;
    while(*offset < lineCount)
    {
        char *line = lines[*offset];
        if(!line || line[0] == '\0') break;

        char* trimmed = trim(line);
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
            while(p[i] && (isalnum((unsigned char)p[i]) || p[i] == '_')) i++;

            if(i <= 0) break;

            jvMeta->name = malloc(sizeof(char) * (i + 1));
            if(!jvMeta->name)
            {
                perror("ahah noob");
                break;
            }
            
            memcpy(jvMeta->name, p, i);
            jvMeta->name[i] = '\0';
            isValid = true;

            (*offset)++;
            return jvMeta;
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

    return NULL;
}

JVPrismObject* parseAnnotation(char **lines, size_t *offset, size_t lineCount)
{
    if(!lines || *offset >= lineCount)
        return NULL;
    
    JVPrismObject* obj = calloc(1, sizeof(JVPrismObject));
    bool isImplemented = false;
    bool isSetup = false;

    while(*offset < lineCount)
    {
        char *line = lines[*offset];
        if(!line || line[0] == '\0')
            break;

        char* trimmed = trim(line);
        if(!isImplemented && trimmed[0] == '@') 
        {
            if (!match_regex("PrismAnot", trimmed))
            {
                (*offset)++;
                continue;
            }
            
            isImplemented = true;
        }
        if(isImplemented && !isSetup)
        {
            char* title = extractAnotValue(trimmed, "title");
            char* description = extractAnotValue(trimmed, "description");

            if(!title)
            {
                (*offset)++;
                continue;
            }

            obj->title = strdup(title);
            free(title);
            if(!description)
            {
                (*offset)++;
                continue;
            }

            obj->description = strdup(description);
            free(description);
            
            isSetup = true;
        }
        if(isImplemented && isSetup)
        {
            if(match_regex(VARIABLE_REGEX, trimmed))
            {
                // should be a variable
                obj->objectType = VARIABLE;
                break;
            }
            
            char *buffer = NULL;
            size_t capacity = 0, len = 0;
            int delim = 0;
            size_t look = *offset;

            // Suicide part
            while(look < lineCount && !delim)
            {
                char *ln = lines[look++];
                char *t = ln ? trim(ln) : NULL; 
                if(!t) break;

                size_t tlen = strlen(t);
                size_t need = len + tlen + 2;

                if(need > capacity)
                {
                    size_t newCap = capacity ? capacity * 2 : 128;
                    while (newCap < need) newCap *= 2;


                    char *newBuffer = realloc(buffer, newCap);
                    if(!newBuffer)
                    {
                        isSetup = false;
                        free(buffer);

                        printf("NTM");
                        break;
                    }

                    buffer = newBuffer;
                    capacity = newCap;
                }

                memcpy(buffer + len, t, tlen);
                len += tlen;
                buffer[len++] = ' ';
                buffer[len] = '\0';

                if(strchr(t, '{') || strchr(t, ';')) delim = 1;
            }

            if(!buffer) continue;
            regex_t methodRegex;
            regcomp(&methodRegex, METHOD_REGEX, REG_EXTENDED);

            int matched = regexec(&methodRegex, buffer, 0, NULL, 0) == 0;
            
            if(matched)
            {
                obj->objectType = METHOD;
                *offset = look;
            }

            regfree(&methodRegex);
            free(buffer);

            if(matched)
                break;
        }

        (*offset)++;
    }   


    if(!isImplemented)
    {
        free(obj);
        return NULL;
    }

    else if(!isSetup)
    {
        if(obj->name)
            free(obj->name);

        if(obj->description)
            free(obj->description);

        free(obj);
        return NULL;
    }

    return obj;
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
    if(!parsed) return NULL;

    char *line = NULL;
    char **lines = NULL;

    size_t len = 0, lineCount = 0;
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

    size_t ofsValue = 0, *offset = &ofsValue;
    JVMeta* jvMeta = isFileValid(lines, offset, lineCount);
    if(!jvMeta) 
    {
        free(jvMeta);
        free(parsed);
        free(line);
        fclose(fp);
        for(int i = 0; i < lineCount; ++i)
            free(lines[i]);
        free(lines);

        perror("SKIPPING: Prismarine as not been implemented in this files");
        return NULL;
    }

    parsed->prismObject = NULL;
    parsed->classInfo = jvMeta;
    while(*offset < lineCount)
    {
        JVPrismObject* obj = parseAnnotation(lines, offset, lineCount);
        if(!obj)
        {
            (*offset)++;
            continue;
        }
        
        JVPrismObject **temp = realloc(parsed->prismObject, sizeof(JVPrismObject*) * (parsed->prismCount + 1));
        if(!temp)
        {
            perror("help");
            break;
        }
        
        parsed->prismObject = temp;
        parsed->prismObject[parsed->prismCount] = obj;
        parsed->prismCount++;
    }

    for(int i = 0; i < parsed->prismCount; ++i)
        printf("- %s.%s \n", parsed->prismObject[i]->title, parsed->prismObject[i]->description);

    free(line);
    fclose(fp);
    for(int i = 0; i < lineCount; ++i)
        free(lines[i]);
    free(lines);

    return parsed;
}

void freeParsedJavaFile(ParsedJavaFile *parsed)
{
    if(parsed->classInfo->name)
        free(parsed->classInfo->name);
    free(parsed->classInfo);

    if(parsed->prismCount > 0)
    {
        for(int i = 0; i < parsed->prismCount; ++i)
        {
            if(!parsed->prismObject[i]) continue;

            free(parsed->prismObject[i]->name);
            free(parsed->prismObject[i]->title);
            free(parsed->prismObject[i]->description);
            free(parsed->prismObject[i]);
        }

        free(parsed->prismObject);
    }

    free(parsed);
    return;
}