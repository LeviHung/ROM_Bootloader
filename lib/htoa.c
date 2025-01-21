/**
 * @file htoa.c
 * @brief
 *
 * This file define htoa function of mini ANSI C library   
 *
 * @author Levi Hung
 * @date 2009/10/13
 */
 
#include <bsp_common.h>

#ifdef CONFIG_LIB_HTOA

static char str[12];
char *htoa(UINT32 hex)
{
    char *h = "0123456789ABCDEF";
    str[0] = '0';
    str[1] = 'x';
	
    str[9] = h[hex & 0x0F];	hex >>= 4;
    str[8] = h[hex & 0x0F];	hex >>= 4;
    str[7] = h[hex & 0x0F];	hex >>= 4;
    str[6] = h[hex & 0x0F];	hex >>= 4;
    str[5] = h[hex & 0x0F];	hex >>= 4;
    str[4] = h[hex & 0x0F];	hex >>= 4;
    str[3] = h[hex & 0x0F];	hex >>= 4;
    str[2] = h[hex & 0x0F];
    str[10] = 0;

    return str;
}

#endif
