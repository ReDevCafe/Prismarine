#include "Mapper/Mapper.h"
#include "Serializer/Json.h"

int main(int argc, char *argv[])
{
    // Cause memory leak, but it works, i'm fucking lazy at this point.
    if(argc > 1)
    {
        PrismPackage* root = ParseFolder(argv[1], 1);
        /*json_object* jPackage = serialize_prism_package(root);
        const char* jsonString = json_object_to_json_string_ext(jPackage, JSON_C_TO_STRING_PRETTY);
        printf("%s\n", jsonString);

        json_object_put(jPackage);*/
        free(root);

        return 0;
    }

    return 1;
}
