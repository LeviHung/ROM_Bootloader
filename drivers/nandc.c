/**
 * @file nandc.c
 * @brief
 *
 * This file implement driver of NAND flash controller.   
 *
 * @author Levi Hung
 * @date 2009/10/13
 */
 
#include <bsp_common.h>
#include <nandc.h>

void nand_reset(void)
{   
    /// [0x30C] Software Reset Control: ECC Enable(Read/Write must be enable)
    REG32(NANDC_BASE + NANDC_SRST) = 0x00000107;         

    /// [0x104] Access Control: RESET(0x2)
    REG32(NANDC_BASE + NANDC_ACC)  = ((RESET<<8)+0x80);  

    while((REG32(NANDC_BASE + NANDC_ACC) & 0x80) != 0) {;}
}


void boot_isp_func_dma(UINT32 Length)
{
    UINT32    *pu32Buf;
    
     /// nand flash controller initial
    nand_reset();        
    
    /// 0x8000: ISP BASE Address
    pu32Buf = (UINT32 *) ISP_BASE;                       

    /// read ISP from NAND flash Block 1 to 0x8000
    nand_read((flashdev->bksize), pu32Buf, Length, 0);   
    
    /// [0x20C] Burst Mode Control: PIO mode
    REG32(NANDC_BASE + NANDC_BMC_BURST) = 0x00000000;               

    board_jumpstart(ISP_BASE);
}

BOOLEAN Check_Complete(UINT32 bWaitForever)
{
    UINT32 reg;
    UINT32 Wait_times = 0;
    
    for(;;) {
        reg = REG32(NANDC_BASE + NANDC_ACC);
        if ((reg & NANDC_CMD_LAUNCH) == 0) {
            return TRUE;
        }
    
        if (bWaitForever) {
            continue;
        }
        
        if (Wait_times++ > 10000) {
            return FALSE;
        }
    }
}

//every command will not send until the R/_B is ready.
BOOLEAN NANDC_CMD_SendCmd(UINT32 Cmd, BOOLEAN bWaitCP)
{
    REG32(NANDC_BASE + NANDC_ACC) = NANDC_CMD_LAUNCH|(Cmd<<NANDC_CMD_OFFSET);

    if (bWaitCP) {
        if(Check_Complete(TRUE)==FALSE) {
            return FALSE;
        }
    }

    return TRUE;
}

/*
 * software reset flash controller
 */
void nand_sw_reset(void) 
{
    REG32(NANDC_BASE + NANDC_SRST) = MANDC_NANDC_SW_RESET | MANDC_BMC_SW_RESET | MANDC_ECC_SW_RESET;
	
	// wait for complete
    while(REG32(NANDC_BASE + NANDC_SRST) & BIT0) {;}
}

void nand_init(void)
{
    volatile UINT32 Reg;
    UINT32 BlockSize, PageSize;

    /// software reset flash controller
    nand_sw_reset();                           
	
    /// Disable interrupts
    REG32(NANDC_BASE + NANDC_INTEN) =  0;    

    /// Page Reset
    REG32(NANDC_BASE + NANDC_ADDR) = 0;    

    Reg = REG32(NANDC_BASE + NANDC_MEMC);
    PageSize    = ((Reg >> 8)  & 0x03);
    BlockSize   = ((Reg >> 16) & 0x03);
    
    // Page & Block Size
    flashdev->pgshift   = (PageSize == 2) ? 12 : 11;                                /// 12 or 11
    flashdev->bkshift   = (PageSize == 0) ? (flashdev->pgshift + 2) + BlockSize:    /// 13, 14, 15, 16, 
                                             (flashdev->pgshift + 4) + BlockSize;   /// 15, 16, 17, 18
    flashdev->pgsize    = 1 << flashdev->pgshift;                                   /// 2K
    flashdev->bksize    = 1 << flashdev->bkshift;                                   /// 128K
    flashdev->pg_per_bk = (PageSize == 0) ? 1 << (BlockSize + 2):                   /// 8 
                                             1 << (BlockSize + 4);                  /// 64
}

BOOLEAN _nand_pgread(const UINT32 *buf, UINT32 pgidx, UINT32 pgcnt)
{
    UINT32 len = flashdev->pgsize;
	
    // DMA burst 4 mode
    REG32(NANDC_BASE + NANDC_BMC_BURST) = 2;

    REG32(NANDC_BASE + NANDC_ADDR)      = pgidx;
    REG32(NANDC_BASE + NANDC_PAGE_CNT)  = pgcnt;

    if(NANDC_CMD_SendCmd(PAGE_READ, 0)==FALSE) {
        return FALSE;
    }	

    // REG32(DMAC_NANDC_CH + DMAC_CSR) = 0x12c0;
    REG32(DMAC_NANDC_CH + DMAC_CSR) = 0x112c0;  // DMA burst 4 mode
    REG32(DMAC_NANDC_CH + DMAC_CFG) = BIT7 | (CONFIG_NAND_DMAREQ << 3) | BIT2;
    REG32(DMAC_NANDC_CH + DMAC_SRC) = NANDC_BASE + NANDC_BMC_DATA_PORT;
    REG32(DMAC_NANDC_CH + DMAC_DST) = (UINT32)buf;
    REG32(DMAC_NANDC_CH + DMAC_LLP) = 0x0;
    REG32(DMAC_NANDC_CH + DMAC_SIZE)= len >> 2;
    REG32(DMAC_BASE + 0x8)          = 0x2;
    REG32(DMAC_NANDC_CH + DMAC_CSR) = 0x112c1;
    REG32(DMAC_BASE + 0x24)         = 0x1;
    while((REG32(DMAC_BASE) & 0x2) ==  0x0) {;}

    REG32(DMAC_BASE + 0x08)         = 0x2;
    REG32(DMAC_BASE + 0x24)         = 0x0;

    if(Check_Complete(FALSE)==FALSE) {
        return FALSE;
    }

    return TRUE;
}

BOOLEAN _nand_pgwrite(UINT32 *buf, UINT32 pgidx, UINT32 pgcnt)
{
    UINT32 len = flashdev->pgsize;

    // DMA burst 4 mode
    REG32(NANDC_BASE + NANDC_BMC_BURST) = 2;

    /// [0x140] BI for write: Bad block Information
    REG32(NANDC_BASE + NANDC_BI_WR)     = 0xFF;

    /// [0x14c] LSN Init: initial value of LSN (as Linux Driver)
    REG32(NANDC_BASE + NANDC_LSN_CTL)   = 0xFFFF;

    /// [0x148] Page Count: page_cnt_reg
    REG32(NANDC_BASE + NANDC_CRC_WR)    = 0xFFFF; 

    REG32(NANDC_BASE + NANDC_ADDR)      = pgidx;
    REG32(NANDC_BASE + NANDC_PAGE_CNT)  = pgcnt;
    if(NANDC_CMD_SendCmd(PGWR_RDSTS, 0) == FALSE) {
        return FALSE;
    }
    // DMA burst 4 mode
    REG32(DMAC_NANDC_CH + DMAC_CSR) = 0x11290;                    
    REG32(DMAC_NANDC_CH + DMAC_CFG) = BIT13 | (CONFIG_NAND_DMAREQ << 9) | BIT2; 	
    REG32(DMAC_NANDC_CH + DMAC_SRC) = (UINT32)buf;  	                            
    REG32(DMAC_NANDC_CH + DMAC_DST) = NANDC_BASE + NANDC_BMC_DATA_PORT;         	
    REG32(DMAC_NANDC_CH + DMAC_LLP) = 0x0;                                         
    REG32(DMAC_NANDC_CH + DMAC_SIZE)= len >> 2;                                    
		
    REG32(DMAC_BASE + 0x8)          = 0x2;

    REG32(DMAC_NANDC_CH + DMAC_CSR) = 0x11291;
    REG32(DMAC_BASE + 0x24)         =  0x1;
	
    while((REG32(DMAC_BASE) & 0x2) ==  0x0) {;}
    
    REG32(DMAC_BASE + 0x08) = 0x2;
    REG32(DMAC_BASE + 0x24) = 0x0;
	
    if(Check_Complete(FALSE)==FALSE) {
        return FALSE;
    }

    return TRUE;
}

/* NAND Flash Bad Block Detection */
int nand_isbb(UINT32 addr)
{
    UINT32 bkidx = addr >> flashdev->bkshift;
    UINT32 pgidx = bkidx << (flashdev->bkshift - flashdev->pgshift);
	
    REG32(NANDC_BASE + NANDC_ADDR) = pgidx;
    REG32(NANDC_BASE + NANDC_PAGE_CNT) = 1;
	
    if (NANDC_CMD_SendCmd(SP_READ, 1) == FALSE) {
        return 1;
    }
	
    if ((REG32(NANDC_BASE + NANDC_BI_RD) & 0xFF) != 0xFF) {
        return 1;
    }
    
    return 0;
}

void nand_read(UINT32 addr, UINT32 *buf, UINT32 len, UINT8 Mode)
{
    UINT32 pgcnt;
    
    nand_reset();
	
    pgcnt = (len + (flashdev->pgsize - 1)) >> flashdev->pgshift;

    while ( pgcnt > 0 ) {
        if (nand_isbb(addr)) {
            // move to next block
            addr >>= flashdev->bkshift;
            addr += 1;
            addr <<= flashdev->bkshift;
            //puts("B");            

        } else {
            if(!Mode) {
                if(!_nand_pgread(buf, (addr >> flashdev->pgshift), 1)) {
                    goto EndError;
                }    

            } else {
                if(!_nand_pgwrite(buf, (addr >> flashdev->pgshift), 1)) {
                    goto EndError;
                }
            }
            len  -= flashdev->pgsize;
            buf  += (flashdev->pgsize >> 2);
            addr += flashdev->pgsize;
            pgcnt-= 1;
        }
    }

    nand_sw_reset();
    return; 
EndError:
    puts("NAND read failed\r\n");
    for(;;) {;}
    
}

/*
flashdev->pgshift       12  / 11
flashdev->bkshift       17  / 14
flashdev->pgsize
flashdev->bksize        128 / 16
flashdev->pg_per_bk     64  / 8
*/

void Blanking_Check(UINT32 length)
{
    UINT32 StartPage=0;
    UINT32 EndPage;
    UINT32 BlkNum;
    UINT32 BlockIdx;
        
    StartPage = 0;	//default
    if (length & (flashdev->bksize - 1)) {
        BlkNum = (length >> flashdev->bkshift) + 1;
    } else {
        BlkNum = length >> flashdev->bkshift ;
    }
    
    EndPage = (BlkNum << (flashdev->bkshift - flashdev->pgshift));
        
    for(;;) {
        REG32(NANDC_BASE + NANDC_ADDR)      = StartPage;
        REG32(NANDC_BASE + NANDC_CB_TAR)    = EndPage;
        REG32(NANDC_BASE + NANDC_ACC)       = NANDC_CMD_LAUNCH | (BLANK_CHK << NANDC_CMD_OFFSET);
        
        if(Check_Complete(TRUE)==TRUE) {
            if((REG32(NANDC_BASE + NANDC_STS) & NANDC_BLANK_CHECK_FAIL) != 
                NANDC_BLANK_CHECK_FAIL) {
                //success
                break;
            }
        }

        //If the program runs here, there is bad block.
        //recalculate the starting address, and write the bad block with non-FF byte (write 0x81)
        //only 3 bytes are used for page index in this word.
        BlockIdx= (REG32(NANDC_BASE + NANDC_ID30) & 0x00FFFFFF) >> (flashdev->bkshift - flashdev->pgshift);
            
        /// [0x140] BI for write: Bad block Information
        REG32(NANDC_BASE + NANDC_BI_WR)     = 0x81;

        /// [0x14c] LSN Init: initial value of LSN (as Linux Driver)         
        REG32(NANDC_BASE + NANDC_LSN_CTL)   = 0xFFFF;       

        /// [0x148] Page Count: page_cnt_reg
        REG32(NANDC_BASE + NANDC_CRC_WR)    = 0xFFFF;       
        REG32(NANDC_BASE + NANDC_ADDR)      = BlockIdx *  flashdev->pg_per_bk;
        REG32(NANDC_BASE + NANDC_PAGE_CNT)  = flashdev->pg_per_bk;

        if (NANDC_CMD_SendCmd(SPWR_RDSTS, 1)== FALSE) {
            puts("NAND check failed\r\n");
            for(;;) {;}
        }

        // recalculate the starting address
        StartPage = (BlockIdx + 1) * (flashdev->pg_per_bk);
        EndPage   =  EndPage + flashdev->pg_per_bk ;    
    }   
}

#if 0
/**
 * @brief
 * Block Erase, command: 0x60
 *
 * @param[in] i_u32Page_index
 *      Page Index
 *
 * @param[in] i_u32Page_count
 *      Page Count
 *
 */
void fw_nand_block_erase(unsigned int i_u32Page_index, unsigned int i_u32Page_count)
{
    REG32(NANDC_BASE + NANDC_ADDR) = i_u32Page_index;       /// [0x10C] Page Index: page_index
    REG32(NANDC_BASE + NANDC_PAGE_CNT)   = i_u32Page_count;         /// [0x308] Page Count: page_cnt_reg
    
    REG32(NANDC_BASE + NANDC_ACC)    = ((BK_RDSTS<<8)+0x80);    /// BK_RDSTS: 0x17 (command:0x60)

    while(((*(volatile unsigned int *)(NANDC_BASE + NANDC_ACC)) & 0x80) != 0);
} 
#endif 