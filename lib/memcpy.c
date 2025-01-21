/**
 * @file memcpy.c
 * @brief
 *
 * This file defines memcpy function of mini ANSI C library   
 *
 * @author Levi Hung
 * @date 2009/10/13
 */
 
#include <bsp_common.h>

#ifdef CONFIG_LIB_MEMCPY

void * memcpy(void * dst, const void *src, unsigned int count)
{
    char *tmp = (char *) dst, *s = (char *) src;

    while (count--)
        *tmp++ = *s++;

    return dst;
}

#endif
