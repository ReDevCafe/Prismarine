#include "Mapper.h"

void* thread_parse_folder(void* args) {
    ThreadsArgs* threadArgs = (ThreadsArgs*) args;
    printf("Thread started for folder %s\n", threadArgs->folderPath);
    threadArgs->result = ParseFolder(threadArgs->folderPath, 0);

    if (!threadArgs->result) {
        printf("\e[0;91m!!! Thread failed to parse folder: %s\n\e[0m", threadArgs->folderPath);
    }

    return 0;
}

void free_prism_package(PrismPackage *prism_package) {
    free(prism_package->childrensFolders);
    for (int i = 0; i < prism_package->numChildrenPrisms; ++i) {
        if (strcmp(prism_package->childrensPrisms[i].metaInfo.name, "root"))
            free(prism_package->childrensPrisms[i].metaInfo.name);
        if (prism_package->childrensPrisms[i].metaInfo.checksum)
            free(prism_package->childrensPrisms[i].metaInfo.checksum);
    }
    
    free(prism_package->childrensPrisms);
    if (strcmp(prism_package->metaInfo.name, "root"))
        free(prism_package->metaInfo.name);
    if (prism_package->metaInfo.checksum)
        free(prism_package->metaInfo.checksum);
    //free(prism_package);
}

PrismPackage* ParseFolder(const char* folderPath, bool isRoot)
{
    PrismPackage* package = (PrismPackage*) calloc(1, sizeof(PrismPackage));
    if (!package) 
    {
        perror("Failed to allocate memory for PrismPackage");
        return 0;
    }    

    if(isRoot)
    package->metaInfo.name = "root";
    else 
    package->metaInfo.name = strdup(folderPath);

    printf("Opening folder %s\n", folderPath);
    DIR* folder = opendir(folderPath);
    if(!folder)
    {
        printf("\e[0;91mFailed to open folder %s\e[0m", folderPath);
        free(package);
        return 0;
    }

    struct dirent* entry;
    ThreadsArgs* threadArgs = calloc(MAX_THREADS, sizeof(ThreadsArgs));
    pthread_t* threads = calloc(MAX_THREADS, sizeof(pthread_t));

    if (!threads || !threadArgs) {
        perror("Failed to allocate memory for threads or thread arguments");
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

        switch (entry->d_type)
        {
            case DT_DIR:
                package->numChildrenFolders++;

                char path[1024];
                snprintf(path, sizeof(path), "%s/%s", folderPath, entry->d_name);
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
                    perror("Failed to allocate memory for child prisms");
                    closedir(folder);
                    free(package);
                    return 0;
                }
                Prism prism = {0};
                prism.metaInfo.name = strdup(entry->d_name);
                prism.metaInfo.checksum = 0;
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

    printf("Waiting for threads to finish\n");
    printf("Current thread folder: %s\n", folderPath);
    printf("Number of threads: %lu\n", package->numChildrenFolders);
    if(threadArgs == 0)
    {
        printf("Skipped %s\n", folderPath);
    }
    else for (unsigned long i = 0; i < package->numChildrenFolders; ++i)
    {
        pthread_join(threads[i], 0);
        printf("Thread %lu finished for folder %s\n", i, folderPath);

        
        if (!threadArgs[i].result) {
            perror("\e[0;91mThread failed to parse folder\e[0m");
            continue;
        }
        
        const char* lastSlash = strrchr(threadArgs[i].folderPath, '/');
        if (lastSlash) 
            threadArgs[i].result->metaInfo.name = strdup(lastSlash + 1);
        else 
            threadArgs[i].result->metaInfo.name = strdup(threadArgs[i].folderPath);
        
        package->childrensFolders[i] = *threadArgs[i].result;

        printf("Found folder: %s\n", threadArgs[i].result->metaInfo.name);
        
        free(threadArgs[i].folderPath);
        if (strcmp(threadArgs[i].result->metaInfo.name, "root"))
            free(threadArgs[i].result->metaInfo.name);
        if (threadArgs[i].result->metaInfo.checksum)
            free(threadArgs[i].result->metaInfo.checksum);
        free(threadArgs[i].result);
    }
    printf("Finished parsing folder %s\n", folderPath);
    free(threads);
    free(threadArgs);
    closedir(folder);
    return package;
}
