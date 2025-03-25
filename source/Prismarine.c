#include "Prismarine.h"

int MAX_THREADS = 0;

int main(int argc, char *argv[])
{
    // Cause memory leak, but it works, i'm fucking lazy at this point.
    if(argc > 1)
    {   
        if(argc > 2)
            MAX_THREADS = atoi(argv[2]);
        else 
        {
            #ifdef _WIN32
                SYSTEM_INFO si;
                GetSystemInfo(&si);
                MAX_THREADS = si.dwNumberOfProcessors;
            #else
                MAX_THREADS = sysconf(_SC_NPROCESSORS_ONLN);
            #endif
        }

        if(MAX_THREADS < 0 || !MAX_THREADS)
        {
            fprintf(stderr, "Error: Invalid number of threads specified.\n");
            return 1;
        } 

        PrismPackage* root = ParseFolder(argv[1], 1);

        free_prism_package(root);
        free(root);

        return 0;
    }

    return 1;
}
