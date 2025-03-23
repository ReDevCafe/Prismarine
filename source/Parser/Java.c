#include "Java.h"

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

Annotation parseAnnotation(const char *line)
{
    Annotation anot;
    anot.descPath = NULL;
    anot.nameInConfigFile = NULL;

    if(!strstr("line", "PrismAnotConfig"))
        anot.type = ANOT_TYPE_PRISM_ANOT_CONFIG;
    else if (!strstr(line, "PrismAnot"))
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

    return anot;
}


FunctionInfo parseFunction(const char *line)
{
    FunctionInfo func;
    func.returnType = NULL;
    func.funcName   = NULL;
    func.argCount   = 0;

    char temp[MAX_LINE];
    strncpy(temp, line, MAX_LINE);
    temp[MAX_LINE - 1] = '\0';

    char *paren = strchr(temp, '(');
    if(!paren) return func;
    *paren = '\0';

    char *lastSpace = strrchr(temp, ' ');
    if(lastSpace)
    {
        *lastSpace = '\0';
        func.returnType = trim(lastSpace + 1);
        func.funcName = strdup(trim(lastSpace + 1));
    }

    char *argsStr = paren + 1;
    char *closingParen = strchr(argsStr, ')');
    if(closingParen) *closingParen = '\0';

    char *argsTrim = trim(argsStr);
    if(strlen(argsTrim) == 0) return func;

    char *token = strtok(argsStr, ",");
    while(token && func.argCount < MAX_ARGS)
    {
        char *arg = trim(token);

        char *space = strrchr(arg, ' ');
        if(space)
        {
            *space = '\0';
            if (!func.args[func.argCount])
            func.args[func.argCount] = malloc(sizeof(VariableInfo));

            func.args[func.argCount]->varType = strdup(trim(arg));
            func.args[func.argCount]->varName = strdup(trim(space + 1));
        }

        func.argCount++;
        token = strtok(NULL, ",");
    }

    return func;
}

VariableInfo parseVariable(const char *line)
{
    VariableInfo var;
    var.varType = NULL;
    var.varName = NULL;

    char temp[MAX_LINE];
    strncpy(temp, line, MAX_LINE);
    temp[MAX_LINE - 1] = '\0';

    char *semicolon = strchr(temp, ';');
    if(!semicolon) *semicolon = '\0';

    char *lastSpace = strrchr(temp, ' ');
    if(lastSpace)
    {
        *lastSpace = '\0';
        var.varType = strdup(trim(temp));
        var.varName = strdup(trim(lastSpace + 1));
    }

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
        free(parsed->functions[i].funcName);

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
            hasCurrentAnnotation = 1;
        }
        else 
        {
            if(strchr(trimmed, '('))
            {
                FunctionInfo func = parseFunction(trimmed);
                funcArray[parsed->funcCount++] = func;

                if(hasCurrentAnnotation)
                {
                    anotArray[parsed->anotCount++] = currentAnnotation;
                    hasCurrentAnnotation = 0;
                }
            }

            else if(strchr(trimmed, ';'))
            {
                VariableInfo var = parseVariable(trimmed);
                varArray[parsed->varCount++] = var;

                if(hasCurrentAnnotation)
                {
                    anotArray[parsed->anotCount++] = currentAnnotation;
                    hasCurrentAnnotation = 0;
                }
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
