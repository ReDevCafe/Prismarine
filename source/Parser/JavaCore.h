#ifndef JAVA_CORE_HEADER_PARSER
#define JAVA_CORE_HEADER_PARSER

#include "../Util/str.h"

#define VARIABLE_REGEX "^[[:space:]]*(public|private|protected)?[[:space:]]*[^=()]+?(=[^;]+)?;[[:space:]]*$"
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
    JVMOD_NONE     = 0,
    JVMOD_STATIC   = 1<<0,
    JVMOD_FINAL    = 1<<1,
    JVMOD_ABSTRACT = 1<<2
} JVModType;

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
    char *name;
    char *type;
} JVArg;

typedef struct 
{
    JVObjectType *objectType;           // is it a method? a variable?
    JVAccessType *access;               // public, private, protected... (if not found, mean that is private)
    unsigned int  modifiers;

    char* object;                       // what is return (void, int, List<string> ect..)
    char* name;                         // the name of the declaration string  >>>>> OhOui <<<<  

    // for documentation side:
    char* title;
    char* description;

    // method only:
    JVArg** args;
    int   argCount;
} JVPrismObject;

typedef struct 
{
    JVMeta *classInfo;

    JVPrismObject **prismObject;
    size_t prismCount;
} ParsedJavaFile;

JVMeta* isFileValid(char **lines, size_t *offset, size_t lineCount);
JVPrismObject* tryParseJVObject(char **lines, size_t *offset, size_t lineCount);

#endif // !JAVA_CORE_HEADER_PARSER