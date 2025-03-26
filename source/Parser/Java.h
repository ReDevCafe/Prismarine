#ifndef JAVA_HEADER_PARSER
#define JAVA_HEADER_PARSER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../Util/str.h"


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

    VariableInfo *args;
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

ParsedJavaFile* parseJavaFile(const char *filename);

void freeParsedJavaFile(ParsedJavaFile *parsed);


#endif // !JAVA_HEADER_PARSER