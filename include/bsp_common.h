/**
 * @file bsp_common.h
 * @brief
 *
 * This file includes config option, system dependent definition, mini ANSI C library 
 * and Register definition of BSP.
 * 
 *
 * @author Levi Hung
 * @date 2009/10/08
 */
 
#ifndef _BOOTCODE2_H
#define _BOOTCODE2_H

#include <bsp/config.h>
#include <bsp/sysdep.h>
#include <lib/minilib.h>
#include <bsp_reg.h>


typedef void (*jsfunc_t)(void);
extern void board_jumpstart(UINT32 addr);
extern UINT32 u32ReadCPUCLK(void);
extern UINT32 *SD_ErrorCode;

/* Below functions used by start.S */
//void board_init_nandc(void);
//void board_init_nandc2(void);
//void board_bsp_strap(void);
//void read_uart_bsp(void);
//void read_uart_lookuptable(void);
//void read_uart_isp(void);
//void board_init_uart(void);
//void board_init_ddr2(void);
//void copy_err(void);
//void start_bootcode2(void);


/*
 * Global Flash Data Struct
 */
typedef struct
{	
    UINT32 pgshift;
    UINT32 bkshift;
    UINT32 pgsize;
    UINT32 bksize;
    UINT32 pg_per_bk;    
} flash_t;

typedef struct _firmware
{
    char    name[12];
    UINT32  size;
} firmware_t;

extern flash_t *flashdev;

extern UINT32 _TEXT_BASE;
extern UINT32 _bss_start;
extern UINT32 _bss_end;             /// use for address tbl location

#ifdef CONFIG_LIB_STRNLEN
extern size_t strnlen(const char * s, size_t count);
#endif

static inline void udelay(UINT32 us, UINT32 cpuclk)
{
    us = (us * cpuclk) >> 2;
    __asm__ __volatile__ (
        "1:"
        "sub %0, %0, #1\n"
        "cmp %0, #0\n"
        "bne 1b\n"
        :
        : "r"(us)	/* input */
        : "memory"	/* clobber list */
        );
}


#endif
