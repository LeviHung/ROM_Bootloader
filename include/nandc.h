/**
 * @file nandc.h
 * @brief
 *
 * This file define parameter and command definition of NAND flash
 *
 * @author Levi Hung
 * @date 2009/10/12
 */
 
#ifndef __NANDC_H
#define __NANDC_H

#define ISP_BASE                (0x8000)
#define ISP_UPDATE_BASE         (0x100000)
#define ISP_TEMP_BASE           (0x100400)
#define ISP_CODE_BASE           (0x101000)
#define ISP_UPDATE_FLAG         (0x100208)
#define ISP_SIZE_BASE           (0x10020C)

#define CONFIG_NAND_DMAREQ	    0

#ifdef FW_DEBUG
#define	dprintf(msg...)     printf(msg);
#else
#define	dprintf(msg...)     while(0) {}
#endif

//================================================================================
// FTNANDC021 register degine
//================================================================================


/******************************************/
// Command Define
/******************************************/
#define PAGE_READ     0x5       /// 5'b00101
#define SP_READ       0x6       /// 5'b00110
#define SP_READ_ALL   0xE       /// 5'b01110
#define READ_ID       0x1       /// 5'b00001
#define RESET         0x2       /// 5'b00010
#define READ_STS      0x4       /// 5'b00100
#define READ_EDC      0xB       /// 5'b01011
#define BLANK_CHK     0x1C      /// 5'b11100
#define PGWR_RDSTS    0x10      /// 5'b10000
#define BK_RDSTS      0x11      /// 5'b10001
#define CPBK_RDSTS    0x12      /// 5'b10010
#define SPWR_RDSTS    0x13      /// 5'b10011
#define MPPGWR_RDSTS  0x14      /// 5'b10100
#define MPBK_RDSTS    0x15      /// 5'b10101
#define MPCPBK_RDSTS  0x16      /// 5'b10110
#define TPPGWR_RDSTS  0x18      /// 5'b11000
#define TPBK_RDSTS    0x19      /// 5'b11001
#define TPCPBK_RDSTS  0x1A      /// 5'b11010


extern void nand_init(void);
extern void nand_read(UINT32 addr, UINT32 *buf, UINT32 len, UINT8 Mode);
extern void boot_isp_func_dma(UINT32 length);
extern void Blanking_Check(UINT32 length);

#endif      /* __NANDC_H */
