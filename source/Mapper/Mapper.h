#ifndef Mapper 
#define Mapper

#include <dirent.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


typedef struct MetaInfo {
    const char* checksum;                  // will be usefull to prevent to do more calculations for nothing
    const char* name;                       // needed for Prismarine doc
} MetaInfo;

typedef struct PrismPackage
{
    MetaInfo metaInfo;

    struct PrismPackage* childrensFolders;  // array of PrismPackage
    struct Prism* childrensPrisms;          // array of Prism

    unsigned long numChildrenFolders;       // number of PrismPackage in childrensFolders
    unsigned long numChildrenPrisms;        // number of Prism in childrensPrisms

} PrismPackage;

typedef struct  Prism
{
    MetaInfo metaInfo;
    //TODO: Add all the documentation shit to this structure    

} Prism;

typedef struct ThreadsArgs {
    char* folderPath;
    PrismPackage* result;
} ThreadsArgs;

PrismPackage* ParseFolder(const char* folder, int isRoot);
#endif // !Mapper