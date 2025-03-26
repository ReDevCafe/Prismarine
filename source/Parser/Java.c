#include "Java.h"

Annotation parse_annotation(const char *line)
{
    Annotation anot = { 0, NULL, NULL };

    if(match_regex("PrismAnotConfig", line))
        anot.type = ANOT_TYPE_PRISM_ANOT_CONFIG;
    else if (match_regex("PrismAnot", line))
        anot.type = ANOT_TYPE_PRISM_ANOT;
    else 
    {
        anot.type = ANOT_TYPE_UNKNOWN;
        return anot;                    // Prevent useless parsing 
    }

    const char *start = strchr(line, '(');
    if(!start) return anot;

    ++start;
    const char *end = strchr(start, ')');
    if(!end) return anot;

    char params[1024];
    size_t len = end - start;

    strncpy(params, start, len);
    params[len] = '\0';
    
    char *token = strtok(params, ",");
    while(token)
    {
        char *equalSign = strchr(token, '=');
        if(equalSign)
        {
            *equalSign = '\0';
            char *key = trim(token);
            char *value = trim(equalSign + 1);

            if(value[0] == '\"')
            {
                ++value;
                char *quoteEnd = strchr(value, '\"');
                if(quoteEnd) *quoteEnd = '\0';
            }

            if(strcmp(key, "descPath") == 0)
                anot.descPath = strdup(value);
            else if(strcmp(key, "nameInConfigFile") == 0)
                anot.nameInConfigFile = strdup(value);
        }
        token = strtok(NULL, ",");
    }

    return anot;
}


FunctionInfo parse_function(const char *line)
{
    FunctionInfo func = {NULL, NULL, NULL, 0};
    for (int i = 0; i < MAX_ARGS; ++i) func.args[i] = NULL;

    char *buffer = malloc(strlen(line) + 1);
    strcpy(buffer, line);

    char *paren = strchr(buffer, '(');
    if (!paren) return func;
    *paren = '\0';

    char *end = buffer + strlen(buffer);
    while (end > buffer && isspace((unsigned char)*(end - 1))) --end;
    *end = '\0';

    char *lastSpace = strrchr(buffer, ' ');
    if (!lastSpace) return func; 

    func.name = strdup(trim(lastSpace + 1));
    *lastSpace = '\0'; 

    char *secondLastSpace = strrchr(buffer, ' ');
    if (!secondLastSpace) return func;

    func.type = strdup(trim(secondLastSpace + 1));
    *secondLastSpace = '\0';
    func.access = strdup(trim(buffer));

    char *argsStr = paren + 1;
    char *closingParen = strchr(argsStr, ')');
    if (closingParen) *closingParen = '\0';

    char *argsTrim = trim(argsStr);
    if (argsTrim == '\0') return func;

    char *argStart = argsTrim;
    while (*argStart != '\0' && func.argCount < MAX_ARGS)
    {
        char *comma = strchr(argStart, ',');
        if (comma) *comma = '\0'; 

        char *arg = trim(argStart);
        char *space = strchr(arg, ' ');

        if (space)
        {
            *space = '\0';
            char *argType = trim(arg);
            char *argName = trim(space + 1);

            func.args[func.argCount] = malloc(sizeof(VariableInfo));
            if (func.args[func.argCount])
            {
                func.args[func.argCount]->type = strdup(argType);
                func.args[func.argCount]->name = strdup(argName);
            }
        }

        func.argCount++;
        if (comma) argStart = comma + 1;
        else break;
    }

#ifdef DEBUG
    printf("\033[0;35m[JVPR]\033[0;32m ACCESS: %s TYPE: %s NAME: %s ARGCOUNT: %d ARGS: [", func.access, func.type, func.name, func.argCount);
    for (int i = 0; i < func.argCount; ++i)
        printf("%s %s%s", func.args[i]->type, func.args[i]->name, i != func.argCount - 1 ? ", " : "]\n");
#endif // DEBUG

    free(buffer);
    return func;
}

VariableInfo parse_variable(const char *line)
{
    VariableInfo var = {NULL, NULL, NULL};
    char *buffer =  malloc(strlen(line) + 1);
    if(!buffer)
    {
        printf("\033[0;31m[JVPR]\033[0m Failed to allocate memory for variable parsing\n");
        return var;
    }

    strcpy(buffer, line);
    char *semicolon = strchr(buffer, ';');
    if(semicolon) *semicolon = '\0';

    char *end = buffer + strlen(buffer);
    while (end > buffer && isspace((unsigned char)*(end - 1))) --end;
    *end = '\0';

    char *lastSpace = strrchr(buffer, ' ');
    if(!lastSpace) return var; 
    var.name = strdup(trim(lastSpace + 1));
    *lastSpace = '\0';

    char *secondLastSpace = strrchr(buffer, ' ');
    if(!secondLastSpace) return var;
    var.type = strdup(trim(secondLastSpace + 1));
    *secondLastSpace = '\0';
    var.access = strdup(trim(buffer));

#ifndef DEBUG
    printf("\033[0;35m[JVPR]\033[0;32m ACCESS: %s TYPE: %s NAME: %s\n", var.access, var.type, var.name);
#endif    

    free(buffer);
    return var;
}

// not sure about all of that, could be fucked up
void freeParsedJavaFile(ParsedJavaFile *parsed)
{
    for(int i = 0; i < parsed->anotCount; ++i)
    {
        printf("\033[0;35m[JVPR]\033[0m Freeing annotation %s\n", parsed->annotation[i].descPath);
        free(parsed->annotation[i].descPath);
        free(parsed->annotation[i].nameInConfigFile);
    }
    free(parsed->annotation);

    for(int i = 0; i < parsed->varCount; ++i)
    {
        printf("\033[0;35m[JVPR]\033[0m Freeing variable %s %s %s\n", parsed->variables[i].access, parsed->variables[i].type, parsed->variables[i].name);
        free(parsed->variables[i].name);
        free(parsed->variables[i].type);
        free(parsed->variables[i].access);
    }
    free(parsed->variables);

    for(int i = 0; i < parsed->funcCount; ++i)
    {
        printf("\033[0;35m[JVPR]\033[0m Freeing function %s %s %s\n", parsed->functions[i].access, parsed->functions[i].type, parsed->functions[i].name);
        free(parsed->functions[i].name);
        free(parsed->functions[i].type);
        free(parsed->functions[i].access);
    }
    free(parsed->functions);

    free(parsed);
}

ParsedJavaFile* parseJavaFile(const char *filename)
{
    ParsedJavaFile* parsed = malloc(sizeof(ParsedJavaFile));
    if(!parsed)
    {
        printf("033[0;35m[JVPR]\033[0;31mFailed to allocate memory for parsed Java file\n");
        return NULL;
    }

    parsed->anotCount = parsed->varCount = parsed->funcCount = 0;
    parsed->annotation = parsed->variables = parsed->functions = NULL;

    FILE *fp = fopen(filename, "r");
    if(!fp) 
    {
        printf("033[0;35m[JVPR]\033[0;31mError opening file %s\n", filename);
        free(parsed);
        return NULL;    
    }

    Annotation currentAnnotation;
    int hasCurrentAnnotation = 0;

    Annotation   *anotArray = calloc(1, sizeof(Annotation));
    FunctionInfo *funcArray = calloc(1, sizeof(FunctionInfo)); 
    VariableInfo *varArray  = calloc(1, sizeof(VariableInfo));

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while((read = getline(&line, &len, fp)) != -1)
    {
        char *trimmed = trim(line);
        if(strlen(trimmed) == 0)
            continue;

        if(trimmed[0] == '@')
        {
            currentAnnotation = parse_annotation(trimmed);
            if(currentAnnotation.type == ANOT_TYPE_UNKNOWN)
            {
                hasCurrentAnnotation = 0;
                continue;
            }

            hasCurrentAnnotation = 1;
        }
        else 
        {
            if(!hasCurrentAnnotation) continue;

            if(strchr(trimmed, ';'))
            {
                VariableInfo var = parse_variable(trimmed);

                varArray  = realloc(varArray, sizeof(VariableInfo) * (parsed->varCount +1));
                anotArray = realloc(anotArray, sizeof(Annotation)  * (parsed->anotCount +1)); 

                varArray[parsed->varCount]   = var;  
                anotArray[parsed->anotCount] = currentAnnotation;

                parsed->varCount++;
                parsed->anotCount++;
            }
            else if(strchr(trimmed, '('))
            {
                FunctionInfo func = parse_function(trimmed);

                funcArray = realloc(funcArray, sizeof(FunctionInfo) * (parsed->varCount +1));
                anotArray = realloc(anotArray, sizeof(Annotation)   * (parsed->varCount +1));

                funcArray[parsed->funcCount] = func;
                anotArray[parsed->anotCount] = currentAnnotation;

                parsed->funcCount++;
                parsed->anotCount++;
            }

            hasCurrentAnnotation = 0;
        }
    }
    free(line);
    fclose(fp);

    if(parsed->anotCount > 0)
    {
        parsed->annotation = malloc(sizeof(Annotation) * parsed->anotCount);
        for(int i = 0; i < parsed->anotCount; ++i)
            parsed->annotation[i] = anotArray[i];
    }

    if(parsed->funcCount > 0)
    {
        parsed->functions = malloc(sizeof(FunctionInfo) * parsed->funcCount);
        for(int i = 0; i < parsed->funcCount; ++i)
            parsed->functions[i] = funcArray[i];
    }

    if(parsed->varCount > 0)
    {
        parsed->variables = malloc(sizeof(VariableInfo) * parsed->varCount);
        for(int i = 0; i < parsed->varCount; ++i)
            parsed->variables[i] = varArray[i];
    }

    free(anotArray);
    free(funcArray);
    free(varArray);

#ifdef DEBUG
    printf("\033[0;35m[JVPR]\033[0;32m Finished parsing file \033[0;37m%s\n", filename);
#endif
    return parsed;
}
