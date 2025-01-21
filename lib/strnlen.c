/**
 * @file strnlen.h
 * @brief
 *
 * This file implement the strnlen function of minilib.
 *
 * @author Levi Hung
 * @date 2009/10/12
 */
 
#include <bsp_common.h>

#ifdef CONFIG_LIB_STRNLEN
size_t strnlen(const char * s, size_t count)
{
    const char *sc;

    for (sc = s; count-- && *sc != '\0'; ++sc)
        /* nothing */;
    return sc - s;
}

#endif
