#ifndef PRISM_HEADER_MAPPER
#define PRISM_HEADER_MAPPER

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dirent.h>
    #include <linux/limits.h>
#endif

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <dirent.h>

#include "../Prismarine.h"
#include "../Parser/Java.h"



typedef struct
{
    long        checksum;                   // will be usefull to prevent to do more calculations for nothing
    char*       name;                       // needed for Prismarine doc
} MetaInfo;

typedef struct
{
    MetaInfo metaInfo;
                                            //TODO: Add all the documentation shit to this structure   
                                            //TODO: imagine how to handle multiple langage parse
    ParsedJavaFile* parse;                  // parsed Java file with all the shit
    bool implemented;
} Prism;

typedef struct PrismPackage PrismPackage;
struct PrismPackage
{
    MetaInfo metaInfo;

    PrismPackage** childrensFolders;        // array of PrismPackage
    Prism** childrensPrisms;                // array of Prism

    unsigned long numChildrenFolders;       // number of PrismPackage in childrensFolders
    unsigned long numChildrenPrisms;        // number of Prism in childrensPrisms

};


typedef struct 
{
    char* folderPath;
    PrismPackage* result;
} ThreadsArgs;

PrismPackage* ParseFolder(const char* folder, bool isRoot);
void freePrismPackage(PrismPackage *prism_package);

#endif // !PRISM_HEADER_MAPPER