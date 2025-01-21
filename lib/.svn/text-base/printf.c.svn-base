/*
 *  linux/lib/printf.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/* printf.c -- Lars Wirzenius & Linus Torvalds. */
/*
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
 */

#include <bsp_common.h>

#ifdef CONFIG_LIB_PRINTF

void printf (const char *fmt, ...)
{
    va_list args;
    int i;
    char printbuffer[CONFIG_LIB_PBSIZE];

    va_start (args, fmt);

    /* For this to work, printbuffer must be larger than
     * anything we ever want to print.
     */
    i = vsprintf (printbuffer, fmt, args);
    va_end (args);

    /* Print the string */
    puts (printbuffer);
}

#endif	// CONFIG_LIB_PRINTF
