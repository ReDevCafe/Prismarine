#ifndef JAVA_HEADER_PARSER
#define JAVA_HEADER_PARSER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "JavaCore.h"

ParsedJavaFile* parseJavaFile(const char *filename);

void freeParsedJavaFile(ParsedJavaFile *parsed);


#endif // !JAVA_HEADER_PARSER