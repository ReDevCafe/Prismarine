#include "JavaCore.h"

void parseVariable(JVPrismObject* object, char* declaration)
{
    if(!object || !declaration) return;
    object->modifiers = JVMOD_NONE;
    object->access = PRIVATE;

    char *end  = declaration + strlen(declaration) - 1;
    if(end >= declaration && *end == ';') *end = '\0';

    // Separate initializer
    char *initializer = NULL;
    char *equalSign   = strchr(declaration, '=');
    if(equalSign)
    {
        *equalSign    = '\0';
        initializer   = trim(equalSign + 1);
    }

    // Name
    char *nameEnd   = declaration + strlen(declaration) - 1;
    while(nameEnd > declaration && isspace((unsigned char)*nameEnd)) nameEnd--;
    char *nameStart = nameEnd;
    while(nameStart > declaration && !isspace((unsigned char)*(nameStart - 1))) nameStart--;
    char savedChar  = *(nameEnd + 1);

    *(nameEnd + 1) = '\0';
    object->name   = strdup(nameStart);
    *(nameEnd + 1) = savedChar;

    // Type
    char *typeEnd   = nameStart - 1;
    while(typeEnd > declaration && isspace((unsigned char)*typeEnd)) typeEnd--;
    char *typeStart = typeEnd;
    while(typeStart > declaration && !isspace((unsigned char)*(typeStart -1))) typeStart--;
    savedChar = *(typeEnd + 1);

    *(typeEnd + 1) = '\0';
    object->object = strdup(typeStart);
    *(typeEnd + 1) = savedChar;

    // Modifiers
    char *modifier = strndup(declaration, typeStart - declaration);
    if(strstr(modifier, "public")) object->access = PUBLIC;
    else if (strstr(modifier, "protected")) object->access = PROTECTED;

    if(strstr(declaration, "static"))   object->modifiers |= JVMOD_STATIC;
    if(strstr(declaration, "final"))    object->modifiers |= JVMOD_FINAL;
    if(strstr(declaration, "abstract")) object->modifiers |= JVMOD_ABSTRACT;

    free(modifier);
}

void parseMethod(JVPrismObject* object, char* buffer)
{
    if(!object || !buffer) return;
    object->modifiers = JVMOD_NONE;
    object->access = PRIVATE;
    
    // Preserve buffer just in case (if asked in pr, can be remove)
    char  *declaration = strdup(buffer);
    size_t len = strlen(declaration);
    if(len > 0 && (declaration[len - 1] == ';' || declaration[len - 1] == '{')) declaration[len - 1] = '\0';

    char *startParen = strchr(declaration, '(');
    char *endParen   = startParen ? strchr(declaration, ')') : NULL;
    char *params     = NULL;
    if(startParen && endParen && endParen > startParen)
    {
        *endParen = '\0';
        params = trim(startParen + 1);
    }

    object->args = NULL;
    object->argCount = 0;
    if(params && *params)
    {
        char* tok = strtok(params, ",");
        while(tok)
        {
            tok = trim(tok);
            if(!*tok)
            {
                char* space = strchr(tok, ' ');
                if(!space) continue;

                JVArg* arg = malloc(sizeof(JVArg)); 
                arg->type = NULL;
                *space = '\0';

                arg->type = strdup(trim(tok));
                arg->name = strdup(trim(space + 1));

                object->args = realloc(object->args, sizeof(JVArg) * (object->argCount + 1));
                object->args[object->argCount++] = arg;
            }

            tok = strtok(NULL, ",");
        }
    }

    free(declaration);
}

JVPrismObject* tryParseJVObject(char **lines, size_t *offset, size_t lineCount)
{
    if(!lines || *offset >= lineCount)
        return NULL;
    
    JVPrismObject* obj = calloc(1, sizeof(JVPrismObject));
    bool isImplemented = false;
    int  isSetup = 0;

    regex_t methodRegex;
    regcomp(&methodRegex, METHOD_REGEX, REG_EXTENDED);

    while(*offset < lineCount)
    {
        char *line = lines[*offset];
        if(!line || line[0] == '\0')
            break;

        char* trimmed = trim(line);
        if(!isImplemented && trimmed[0] == '@' && match_regex("PrismAnot", trimmed))  
            isImplemented = true;
        if(isImplemented && isSetup != 0b0011)
        {
            if(!obj->title )
            {
                char* title = extractAnotValue(trimmed, "title");
                if(!title)
                {
                    (*offset)++;
                    continue;
                }
                obj->title = strdup(title);
                free(title);

                isSetup |= 1 << 0;
            }

            if(!obj->description)
            {
                char* description = extractAnotValue(trimmed, "description");
                if(!description)
                {
                    (*offset)++;
                    continue;
                }

                obj->description = strdup(description);
                free(description);

                isSetup |= 1 << 1;
            }
            
        }
        if(isImplemented && isSetup == 0b0011)
        {
            if(match_regex(VARIABLE_REGEX, trimmed))
            {
                obj->objectType = VARIABLE;
                parseVariable(obj, trimmed);
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

                    buffer = realloc(buffer, newCap);
                    if(!buffer)
                    {
                        isSetup = false;
                        free(buffer);

                        printf("[JVP] Failed to reallocate 'buffer'");
                        break;
                    }

                    capacity = newCap;
                }

                memcpy(buffer + len, t, tlen);
                len += tlen;
                buffer[len++] = ' ';
                buffer[len] = '\0';

                if(strchr(t, '{') || strchr(t, ';')) delim = 1;
            }
            if(!buffer) continue;
            
            int matched = regexec(&methodRegex, buffer, 0, NULL, 0) == 0;
            if(matched)
            {
                obj->objectType = METHOD;
                parseMethod(obj, buffer);
                *offset = look;
            }

            free(buffer);
            if(matched) break;
        }

        (*offset)++;
    }   

    regfree(&methodRegex);
    if(!isImplemented)
    {
        free(obj);
        return NULL;
    }

    if(!isSetup)
    {
        if(obj->name) free(obj->name);
        if(obj->description) free(obj->description);
        free(obj);

        return NULL;
    }

    return obj;
}