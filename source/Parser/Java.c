#include "Java.h"

Annotation parseAnnotation(const char *line)
{
    printf("\033[0;35m[JVPR]\033[0;32m Reading \033[0m%s\n", line);
    Annotation anot;
    anot.descPath = NULL;
    anot.nameInConfigFile = NULL;

    if(match_regex("PrismAnotConfig", line))
        anot.type = ANOT_TYPE_PRISM_ANOT_CONFIG;
    else if (match_regex("PrismAnot", line))
        anot.type = ANOT_TYPE_PRISM_ANOT;
    else 
        anot.type = ANOT_TYPE_UNKNOWN;

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

    if(anot.type != ANOT_TYPE_UNKNOWN)
        printf("\033[0;35m[JVPR]\033[0;32m found %d%s\033[0m\n", anot.type, anot.descPath);
        
    return anot;
}


FunctionInfo parseFunction(const char *line)
{
    FunctionInfo func = {NULL, NULL, NULL, 0};
    for (int i = 0; i < MAX_ARGS; ++i) func.args[i] = NULL;

    char buffer[MAX_LINE];
    strncpy(buffer, line, MAX_LINE);

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

    printf("\033[0;35m[JVPR]\033[0;32m ACCESS: %s TYPE: %s NAME: %s ARGCOUNT: %d ARGS: [", func.access, func.type, func.name, func.argCount);
    for (int i = 0; i < func.argCount; ++i)
        printf("%s %s%s", func.args[i]->type, func.args[i]->name, i != func.argCount - 1 ? ", " : "]\n");

    return func;
}

VariableInfo parseVariable(const char *line)
{
    VariableInfo var = {NULL, NULL, NULL};
    char buffer[MAX_LINE];

    strncpy(buffer, line, MAX_LINE);
    buffer[MAX_LINE - 1] = '\0';

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
            
    printf("\033[0;35m[JVPR]\033[0;32m ACCESS: %s TYPE: %s NAME: %s\n", var.access, var.type, var.name);
    return var;
}

// not sure about all of that could be fucked up
void freeParsedJavaFile(ParsedJavaFile *parsed)
{
    for(int i = 0; i < parsed->anotCount; ++i)
    {
        free(parsed->annotation[i].descPath);
        free(parsed->annotation[i].nameInConfigFile);
    }

    free(parsed->annotation);
    for(int i = 0; i < parsed->funcCount; ++i)
        free(parsed->functions[i].name);

    free(parsed);
}

ParsedJavaFile* parseJavaFile(const char *filename)
{
    ParsedJavaFile* parsed = malloc(sizeof(ParsedJavaFile));
    parsed->anotCount = parsed->funcCount = parsed->varCount = 0;
    parsed->annotation = parsed->functions = parsed->variables = NULL;

    FILE *fp = fopen(filename, "r");
    if(!fp) 
    {
        perror("Error opening file");
        return NULL;    
    }

    char line[MAX_LINE];
    Annotation currentAnnotation;
    int hasCurrentAnnotation = 0;

    Annotation   anotArray[MAX_ARRAY];
    FunctionInfo funcArray[MAX_ARRAY];
    VariableInfo varArray[MAX_ARRAY];

    while(fgets(line, sizeof(line), fp))
    {
        char *trimmed = trim(line);
        if(strlen(trimmed) == 0)
            continue;

        if(trimmed[0] == '@')
        {
            currentAnnotation = parseAnnotation(trimmed);

            if(currentAnnotation.type != ANOT_TYPE_UNKNOWN)
                hasCurrentAnnotation = 1;
            else hasCurrentAnnotation = 0;
        }
        else 
        {
            if(!hasCurrentAnnotation) continue;

            if(strchr(trimmed, ';'))
            {
                VariableInfo var = parseVariable(trimmed);
                varArray[parsed->varCount++] = var;

                anotArray[parsed->anotCount++] = currentAnnotation;
                hasCurrentAnnotation = 0;
            }
            else if(strchr(trimmed, '('))
            {
                FunctionInfo func = parseFunction(trimmed);
                funcArray[parsed->funcCount++] = func;

                anotArray[parsed->anotCount++] = currentAnnotation;
                hasCurrentAnnotation = 0;
            }
        }
    }
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

    printf("\033[0;35m[JVPR]\033[0;32m Finished parsing file \033[0;37m%s\n", filename);
    return parsed;
}
