#ifdef __cplusplus
#include <stdint.h>
#include "common.h"

extern "C"
{
#endif

    /**
     * Returns the version of the opencv library.
     *
     * @return The version of the opencv library.
     */
    const char *opencvVersion();

    /**
     * @brief Processes image bytes.
     * @param bytes Image bytes.
     * @param width Image width.
     * @param height Image height.
     * @return Json .
     */
    const char *opencvProcessStream(char *bytes, int width, int height, char *kitType);

    void opencvProcessImage(char *input, char *output);

#ifdef __cplusplus
}
#endif