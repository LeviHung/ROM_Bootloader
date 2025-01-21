/**
 * @file minilib.h
 * @brief
 *
 * This file defines mini-library of BSP 
 * 
 *
 * @author Levi Hung
 * @date 2009/10/12
 */
 
#ifndef MINILIB_H
#define MINILIB_H

#include <stdarg.h>
#include <serial.h>

#ifdef CONFIG_LIB_HTOA
char *htoa(UINT32 hex);
#endif

//#define getchar(t)      serial_getc(t)
#define putchar(c)      serial_putc(c)
#define puts(s)         serial_puts(s)

#ifdef CONFIG_LIB_MEMCPY
void * memcpy(void * dst, const void *src, unsigned int count);
#endif
#ifdef CONFIG_LIB_MEMSET
void * memset(void * s, int c, unsigned int count);
#endif

#ifdef  CONFIG_LIB_PRINTF
#define CONFIG_LIB_STRNLEN	1
int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char * buf, const char *fmt, ...);
void printf (const char *fmt, ...);
#else	// #ifdef CONFIG_LIB_PRINTF
static inline int vsprintf(char *buf, const char *fmt, va_list args) { return 0; }
static inline int sprintf(char * buf, const char *fmt, ...) { return 0; }
static inline void printf (const char *fmt, ...) { }
#endif	// #ifdef CONFIG_LIB_PRINTF

#endif
