#ifndef JAVA_HEADER_PARSER
#define JAVA_HEADER_PARSER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "../Util/str.h"

#define VARIABLE_REGEX \
  "^[[:space:]]*"                      /* leading space */ \
  "(public|private|protected)?"        /* optional access */ \
  "[[:space:]]*"                       /* optional space */ \
  "[^=()]+?"                           /* type & name, no parens */ \
  "(=[^;]+)?;"                         /* optional initializer */ \
  "[[:space:]]*$"                      /* trailing space */

#define METHOD_REGEX "^[[:space:]]*(public|private|protected)?[[:space:]].*\\([^;]*\\)[[:space:]]*(\\{)?[[:space:]]*?"

typedef enum 
{
    CLASS,
    CLASS_ENUM,
    CLASS_INTERFACE,
    CLASS_ABSTRACT
} JVClassType;

typedef enum 
{
    METHOD,
    VARIABLE,
} JVObjectType;

typedef enum {
    JVDECL_NONE     = 0,
    JVDECL_STATIC   = 1<<0,
    JVDECL_FINAL    = 1<<1,
    JVDECL_ABSTRACT = 1<<2
} JVDeclarationType;

typedef enum 
{
    PRIVATE,
    PUBLIC,
    PROTECTED
} JVAccessType;

typedef struct
{
    char *name;
    JVClassType *type;

} JVMeta;

typedef struct 
{
    JVObjectType *objectType;
    JVAccessType *access;
    unsigned int  declarationFlags;

    char* object;
    char* name;

    // for documentation side:
    // + title is supposed to be name but if it's customNamed it will have something else instead :3
    char* title;
    char* description;

    // method only:
    char* args;
    int   argCount;
} JVPrismObject;

typedef struct 
{
    JVMeta *classInfo;

    JVPrismObject **prismObject;
    size_t prismCount;
} ParsedJavaFile;

ParsedJavaFile* parseJavaFile(const char *filename);

void freeParsedJavaFile(ParsedJavaFile *parsed);


#endif // !JAVA_HEADER_PARSER