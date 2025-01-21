/**
 * @file config.h
 * @brief
 *
 * This file define configure option of ROM bootloader
 *
 * @author Levi Hung
 * @date 2009/10/08
 */

#ifndef _CONFIG_H
#define _CONFIG_H

/* Memory Mapping */
#define ROM_DEFAULT	        0x00000000
#define DDR2_DEFAULT	    0x20000000

/*
 * Library
 */
#ifdef _DEBUG

#define CONFIG_MEMSIZE          (128 * 1024  * 1024)
#define CONFIG_LIB_PBSIZE       1024

#else	// #ifdef _DEBUG

#define CONFIG_MEMSIZE          (64 * 1024  * 1024)
#endif	// #ifdef _DEBUG

#endif  /* __CONFIG_H */
