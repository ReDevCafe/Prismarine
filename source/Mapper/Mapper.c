#include "Mapper.h"

void* thread_parse_folder(void* args) 
{
    ThreadsArgs* threadArgs = (ThreadsArgs*) args;
    threadArgs->result = ParseFolder(threadArgs->folderPath, 0);

    if (!threadArgs->result)
        printf("\e\033[0;34m[MPPR]\033[0;31m Thread failed to parse folder: %s\n\e[0m", threadArgs->folderPath);

    return 0;
}

void free_prism_package(PrismPackage *prism_package) 
{
    free(prism_package->childrensFolders);

    for (int i = 0; i < prism_package->numChildrenPrisms; ++i) 
    {
        if (strcmp(prism_package->childrensPrisms[i].metaInfo.name, "root"))
            free(prism_package->childrensPrisms[i].metaInfo.name);

        if (prism_package->childrensPrisms[i].metaInfo.checksum)
            free(prism_package->childrensPrisms[i].metaInfo.checksum);

        if (prism_package->childrensPrisms[i].parse)
            freeParsedJavaFile(prism_package->childrensPrisms[i].parse);
    }

    free(prism_package->childrensPrisms);

    if (strcmp(prism_package->metaInfo.name, "root"))
        free(prism_package->metaInfo.name);

    if (prism_package->metaInfo.checksum)
        free(prism_package->metaInfo.checksum);


    //free(prism_package);
}

PrismPackage* ParseFolder(const char* folderPath, int isRoot)
{
    PrismPackage* package = (PrismPackage*) calloc(1, sizeof(PrismPackage));
    if (!package) 
    {
        perror("\033[0;34m[MPPR]\033[0;31m Failed to allocate memory for PrismPackage");
        return 0;
    }    

    if(isRoot)
    package->metaInfo.name = "root";
    else 
    package->metaInfo.name = strdup(folderPath);
    
    package->metaInfo.checksum = 0;
    package->childrensFolders = 0;
    package->childrensPrisms = 0;
    package->numChildrenFolders = 0;
    package->numChildrenPrisms = 0;

    DIR* folder = opendir(folderPath);
    if(!folder)
    {
        printf("\e\033[0;34m[MPPR]\033[0;31m Failed to open folder %s\e[0m", folderPath);
        free(package);
        return 0;
    }

    struct dirent* entry;
    ThreadsArgs* threadArgs = calloc(MAX_THREAD, sizeof(ThreadsArgs));
    pthread_t* threads = calloc(MAX_THREAD, sizeof(pthread_t));

    if (!threads || !threadArgs) {
        perror("\033[0;34m[MPPR]\033[0;37m Failed to allocate memory for threads or thread arguments");
        free(threads);
        free(threadArgs);
        closedir(folder);
        free(package);
        return NULL;
    }
    
    while((entry = readdir(folder)) != 0)
    {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        continue;

        char path[1024];
        if(folderPath[strlen(folderPath) - 1] == '/')
            snprintf(path, sizeof(path), "%s%s", folderPath, entry->d_name);
        else
            snprintf(path, sizeof(path), "%s/%s", folderPath, entry->d_name);

        switch (entry->d_type)
        {
            case DT_DIR:
                package->numChildrenFolders++;

                unsigned long index = package->numChildrenFolders - 1;

                threadArgs[index].folderPath = strdup(path);
                threadArgs[index].result = 0;
            
                pthread_create(
                    &threads[index], 
                    0, 
                    thread_parse_folder, 
                    &threadArgs[index]
                );
                break;

            case DT_REG:
                package->numChildrenPrisms++;

                package->childrensPrisms = (Prism*) realloc(
                    package->childrensPrisms,
                    package->numChildrenPrisms * sizeof(Prism)
                );

                if (!package->childrensPrisms) 
                {
                    perror("\033[0;34m[MPPR]\033[0;31m Failed to allocate memory for child prisms");
                    closedir(folder);
                    free(package);
                    return 0;
                }
                Prism prism = {0};
                prism.metaInfo.name = strdup(entry->d_name);
                prism.metaInfo.checksum = 0;

                ParsedJavaFile* parsing = parseJavaFile(path);
                if(!parsing)
                {
                    perror("WTFFFF");
                    closedir(folder);
                    free(package);
                    return 0;
                }

                prism.parse = parsing;
                package->childrensPrisms[package->numChildrenPrisms -1] = prism;
                break;
            default:
                break;
        }
    }

    if(package->numChildrenFolders > 0)
    {
        package->childrensFolders = (PrismPackage*) realloc(
            package->childrensFolders,
            package->numChildrenFolders * sizeof(PrismPackage)
        );

        if (!package->childrensFolders) {
            perror("Failed to allocate memory for child folders");
            closedir(folder);
            free(package);
            return 0;
        }
    }
    

    if(threadArgs == 0)
    {
        printf("Skipped %s\n", folderPath);
    }
    else for (unsigned long i = 0; i < package->numChildrenFolders; ++i)
    {
        pthread_join(threads[i], 0);
        printf("\033[0;34m[MPPR]\033[0;32m Thread %lu finished for folder \033[0;37m%s\n", i, folderPath);
        
        if (!threadArgs[i].result) {
            perror("\e\033[0;34m[MPPR]\033[0;31m Thread failed to parse folder\e[0m");
            continue;
        }
        
        const char* lastSlash = strrchr(threadArgs[i].folderPath, '/');
        if (lastSlash) 
            threadArgs[i].result->metaInfo.name = strdup(lastSlash + 1);
        else 
            threadArgs[i].result->metaInfo.name = strdup(threadArgs[i].folderPath);
        
        package->childrensFolders[i] = *threadArgs[i].result;

        free(threadArgs[i].folderPath);
        if (strcmp(threadArgs[i].result->metaInfo.name, "root"))
            free(threadArgs[i].result->metaInfo.name);
        if (threadArgs[i].result->metaInfo.checksum)
            free(threadArgs[i].result->metaInfo.checksum);
        free(threadArgs[i].result);
    }
    printf("\033[0;34m[MPPR]\033[0;32m Finished parsing folder\033[0;37m %s\n", folderPath);
    free_prism_package(package);
    free(threads);
    free(threadArgs);
    closedir(folder);
    return package;
}