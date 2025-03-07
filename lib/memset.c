/**
 * @file memset.c
 * @brief
 *
 * This file defines memset function of mini ANSI C library   
 *
 * @author Levi Hung
 * @date 2009/10/13
 */
 
#include <bsp_common.h>

#ifdef CONFIG_LIB_MEMSET

void * memset(void * s, int c, unsigned int count)
{
    char *p = (char *) s;

    while (count--)
        *p++ = c;

    return s;
}

#endif
