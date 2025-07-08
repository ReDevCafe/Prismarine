#include "Java.h"

JVMeta* isFileValid(char **lines, size_t *offset, size_t lineCount) 
{
    if(!lines || *offset >= lineCount) return NULL;

    JVMeta* jvMeta = calloc(1, sizeof(JVMeta));
    if(!jvMeta) return NULL;

    bool isImplemented = false;
    bool isValid = false;
    while(*offset < lineCount)
    {
        char *line = lines[*offset];
        if(!line || line[0] == '\0') break;

        if(!isImplemented && line[0] == '@' && strstr(line, "PrismClass"))
            isImplemented = true;
        if(isImplemented && !isValid) 
        {
            const char *keyword = NULL;
            if(strstr(line,"class"))
            {
                jvMeta->type = CLASS;
                keyword = "class";
            }
            else if(strstr(line, "enum"))
            {
                jvMeta->type = CLASS_ENUM;
                keyword = "enum";
            }
            else if(strstr(line,"abstract"))
            {
                jvMeta->type = CLASS_ABSTRACT;
                keyword = "abstract";
            }
            else if(strstr(line,"interface"))
            {
                jvMeta->type = CLASS_INTERFACE;
                keyword = "interface";
            }
            else
            {
                (*offset)++;
                continue;
            }
            
            char *p = strstr(line, keyword);
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
            
            jvMeta->name = realloc(jvMeta->name, (i + 1));
            if(!jvMeta->name)
            {
                perror("[JVP] Failed to allocate 'jvMeta->name'");
                break;
            }
            
            memcpy(jvMeta->name, p, i);
            jvMeta->name[i] = '\0';
            jvMeta->isValid = true;

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

    if(!jvMeta->isValid)
    {
        free(jvMeta->name);
        free(jvMeta);
        return NULL;
    }

    return NULL;
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

    char *line   = NULL;
    char **lines = NULL;
    size_t len   = 0, lineCount = 0;
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
            free(lines);
            break;
        }

        char **temp = realloc(lines, sizeof(*lines) * (lineCount + 1));
        if(!temp)
        {
            perror("realloc temp");
            free(lines);
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
#ifdef DEBUG
        printf("\033[0;35m[JVPR]\033[0;33m SKIPPING: Prismarine as not been correctly implemented in %s\033[0;37m\n", filename);
#endif // !DEBUG
        return NULL;
    }

    parsed->prismObject = NULL;
    parsed->classInfo = jvMeta;
    isRegexCompiled = false;

    // Add security to prevent unnecessary calls 
    while(*offset < lineCount)
    {
        JVPrismObject* obj = tryParseJVObject(lines, offset, lineCount, jvMeta);
        if(!obj)
        {
            // Free JVPrismObject
            (*offset)++;
            continue;
        }
        
        parsed->prismObject = realloc(parsed->prismObject, sizeof(JVPrismObject*) * (parsed->prismCount + 1));
        if(!parsed->prismObject)
        {
            // Free JVPrismObject
            perror("help");
            break;
        }
        
        parsed->prismObject[parsed->prismCount] = obj;
        parsed->prismCount++;
    }

    if(isRegexCompiled)
    {
        regfree(&VariableRegex);
        regfree(&MethodRegex);
    }

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
            free(parsed->prismObject[i]->object);

            if(parsed->prismObject[i]->argCount > 0)
            {
                for(int j = 0; j < parsed->prismObject[i]->argCount; ++j)
                {
                    free(parsed->prismObject[i]->args[j]->name);
                    free(parsed->prismObject[i]->args[j]->type);
                    free(parsed->prismObject[i]->args[j]);
                }
                free(parsed->prismObject[i]->args);
            }

            free(parsed->prismObject[i]);
        }
        free(parsed->prismObject);
    }

    free(parsed);
    return;
}