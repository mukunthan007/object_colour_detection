#pragma once
#include <stdint.h>
#include <map>

#ifdef __cplusplus
extern "C"
{
#endif

    struct ColorEntry
    {
        const char *key;
        const char *value;
    };

    struct ProcessedData
    {
        uint8_t *imageData;
        struct ColorEntry *colorEntries;
    };

#ifdef __cplusplus
}
#endif
