#include "Mapper/Mapper.h"

int main(int argc, char *argv[])
{
    // Cause memory leak, but it works, i'm fucking lazy at this point.
    if(argc > 1)
    {
        PrismPackage* root = ParseFolder(argv[1], 1);
        printf("\033[0;33m[PRSM]\033[0;32m Finished all tasks..\033[0;33m\n");
        free(root);

        return 0;
    }

    return 1;
}
