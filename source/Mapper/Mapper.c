#include "Mapper.h"

void* thread_parse_folder(void* args) {
    ThreadsArgs* threadArgs = (ThreadsArgs*) args;
#ifdef DEBUG
    printf("Thread started for folder %s\n", threadArgs->folderPath);
#endif // DEBUG
    threadArgs->result = ParseFolder(threadArgs->folderPath, 0);

    if (!threadArgs->result)
        printf("\e\033[0;34m[MPPR]\033[0;31m Thread failed to parse folder: %s\n\e[0m", threadArgs->folderPath);

    return 0;
}

void freePrismPackage(PrismPackage *prism_package)
{
    free(prism_package->childrensFolders);
    for (int i = 0; i < prism_package->numChildrenPrisms; ++i)
    {
        if (prism_package->childrensPrisms[i].parse)
            freeParsedJavaFile(prism_package->childrensPrisms[i].parse);

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
        perror("\033[0;34m[MPPR]\033[0;31m Failed to allocate memory for PrismPackage");
        return 0;
    }    

    if(isRoot)
        package->metaInfo.name = "root";
    else 
        package->metaInfo.name = strdup(folderPath);

    DIR* folder = opendir(folderPath);
    if(!folder)
    {
        printf("\e\033[0;34m[MPPR]\033[0;31m Failed to open folder %s\e[0m", folderPath);
        free(package);
        return 0;
    }

    struct dirent* entry;
    ThreadsArgs* threadArgs = calloc(MAX_THREADS, sizeof(ThreadsArgs));
    pthread_t* threads = calloc(MAX_THREADS, sizeof(pthread_t));

    if (!threads || !threadArgs) 
    {
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
        
#ifdef _WIN32
        char path[MAX_PATH];
#else 
        char path[PATH_MAX];
#endif

        if(folderPath[strlen(folderPath) - 1] == '/')
            snprintf(path, sizeof(path), "%s%s", folderPath, entry->d_name);
        else
            snprintf(path, sizeof(path), "%s/%s", folderPath, entry->d_name);

#ifdef DEBUG
        printf("\033[0;34m[MPPR]\033[0;31m Processing \033[0;37m%s\n", path);
#endif // DEBUG

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
                package->childrensPrisms = realloc(package->childrensPrisms, package->numChildrenPrisms * sizeof(Prism));

                if (!package->childrensPrisms) 
                {
                    perror("\033[0;34m[MPPR]\033[0;31m Failed to allocate memory for child prisms\n");
                    closedir(folder);
                    free(package->childrensPrisms);
                    free(package);
                    return 0;
                }
                Prism prism = {0};
                prism.metaInfo.name = strdup(entry->d_name);
                prism.metaInfo.checksum = 0;

                ParsedJavaFile* jvpr = parseJavaFile(path);
                if(!jvpr)
                {
                    printf("\033[0;34m[MPPR]\033[0;31m Failed to parse Java file: %s\n", path);
                    closedir(folder);

                    free(jvpr);
                    free(package);
                    return 0;
                }
                prism.parse = jvpr;

                package->childrensPrisms[package->numChildrenPrisms - 1] = prism;
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

        if (!package->childrensFolders) 
        {
            perror("\033[0;34m[MPPR]\033[0;32m Failed to allocate memory for child folders");
            closedir(folder);
            free(package);
            return 0;
        }
    }

#ifdef DEBUG
    printf("Waiting for threads to finish\n");
    printf("Current thread folder: %s\n", folderPath);
    printf("Number of threads: %lu\n", package->numChildrenFolders);
#endif

    if(threadArgs == 0)
    {
        printf("\e\033[0;34m[MPPR]\033[0;31m Skipped \e[0m%s\n", folderPath);
    }
    else for (unsigned long i = 0; i < package->numChildrenFolders; ++i)
    {
        pthread_join(threads[i], 0);
#ifdef DEBUG
        printf("\033[0;34m[MPPR]\033[0;32m Thread %lu finished for folder \033[0;37m%s\n", i, folderPath);
#endif

        
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

#ifdef DEBUG
        printf("Found folder: %s\n", threadArgs[i].result->metaInfo.name);
#endif //! DEBUG

        free(threadArgs[i].folderPath);

        if (strcmp(threadArgs[i].result->metaInfo.name, "root"))
            free(threadArgs[i].result->metaInfo.name);

        if (threadArgs[i].result->metaInfo.checksum)
            free(threadArgs[i].result->metaInfo.checksum);

        free(threadArgs[i].result);
    }

    printf("\033[0;34m[MPPR]\033[0;32m Finished parsing folder\033[0;37m %s\n", folderPath);
    free(threads);
    free(threadArgs);
    closedir(folder);
    return package;
}
