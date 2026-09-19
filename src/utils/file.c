#include <fileapi.h>
#include <stdint.h>
#include <windows.h>


uint64_t VSE_GetFileLastWrittenTime(char *fileName)
{
    WIN32_FILE_ATTRIBUTE_DATA info;
    if (GetFileAttributesExA(fileName, GetFileExInfoStandard, &info))
    {
        FILETIME written = info.ftLastWriteTime;
        uint64_t timestamp =
            ((uint64_t)written.dwHighDateTime << 32) | written.dwLowDateTime;
        return timestamp;
    }

    return 0;
}
