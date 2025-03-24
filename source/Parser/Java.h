#ifndef JAVA_HEADER_PARSER
#define JAVA_HEADER_PARSER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../Util/str.h"

#define MAX_LINE  4096           // If someone make an issue saying that this is not enough lines i will bake my sanity into a pie
#define MAX_ARGS  32
#define MAX_ARRAY 512

typedef enum 
{
    ANOT_TYPE_UNKNOWN,
    ANOT_TYPE_PRISM_ANOT,
    ANOT_TYPE_PRISM_ANOT_CONFIG
} AnotType;

typedef struct
{
    AnotType type;
    char *descPath;
    char *nameInConfigFile;
} Annotation;

typedef struct 
{
    char *access;
    char *type;
    char *name;
} VariableInfo;

typedef struct 
{
    char *access;
    char *type;
    char *name;
    int argCount;

    VariableInfo *args[MAX_ARGS];
} FunctionInfo;

typedef struct 
{
    Annotation *annotation;
    int anotCount;

    FunctionInfo *functions;
    int funcCount;

    VariableInfo *variables;
    int varCount;
} ParsedJavaFile;

Annotation parseAnnotation(const char *line);
FunctionInfo parseFunction(const char *line);
VariableInfo parseVariable(const char *line);
ParsedJavaFile* parseJavaFile(const char *filename);

void freeParsedJavaFile(ParsedJavaFile *parsed);


#endif // !JAVA_HEADER_PARSER