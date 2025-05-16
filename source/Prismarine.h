#ifndef PRISMARINE_HEADER
#define PRISMARINE_HEADER

#include "Mapper/Mapper.h"
#include "Publish/Json.h"

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

// C BON LA???
int MAX_THREADS __attribute__((common));

#endif // !PRISMARINE_HEADER

