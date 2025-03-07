/**
 * @file main.c
 * @brief Implement upgrade NAND_BL, run SD_BL and run NAND_BL for ROM Bootloader.
 *
 *
 * @author Levi Hung
 * @date    2009/06/05
 * @note    2009/06/05:	Created by Levi Hung.
 * 
 */
#include <bsp_common.h>
#include <nandc.h> 

flash_t *flashdev;

void main (void)
{
    void *tbl = (void *)(_bss_end + 4);
    firmware_t *fw;

    UINT32 *u32Update_Flag = ((UINT32 *) ISP_UPDATE_BASE);
    UINT32 isp_length, isp_update_flag;
    UINT32 *isp_code = (UINT32 *) ISP_CODE_BASE;              // 0x101000
    UINT32 *bsp_LT_data = (UINT32 *) ISP_UPDATE_BASE;         // 0x100000    
    UINT32 *bsp_LT_data_tmp; 
    UINT32  *pu32Buf, *pu32Buf_tmp;
    UINT32   cnt;
    
    nand_init();

    /// choose Update ISP or Go ISP? use ISP address, 
    /// use DDR2 header to detemine UART/SD data is right.
    if (*(u32Update_Flag)   == 0x5f495641) { 

        isp_length = * ((UINT32 *) ISP_SIZE_BASE);
        isp_update_flag = * ((UINT32 *) ISP_UPDATE_FLAG);
            
        if (isp_update_flag) {              /// Update ISP
            puts("Upgrade NAND");        

            Blanking_Check(isp_length);

            bsp_LT_data_tmp = bsp_LT_data;
            /// clear ISP_UPDATE_BASE
            for (cnt = 0; cnt < 256; cnt++, bsp_LT_data_tmp++) {            
                *(bsp_LT_data_tmp + 256) = 0;
            }

            /// Update BSP and Lookup Table
            nand_read(0, bsp_LT_data, 0x400, 1);                              
                
            /// Update ISP Program    
            nand_read((flashdev->bksize), isp_code, isp_length, 1);         
            
            boot_isp_func_dma(isp_length);
            
        } else {           
            puts("Go SD");      
            
            pu32Buf =     (UINT32 *) ISP_BASE;
            pu32Buf_tmp = (UINT32 *) ISP_CODE_BASE;
            
            for (cnt = 0; cnt <= (isp_length >> 2); cnt++, pu32Buf++, pu32Buf_tmp++) {
                *pu32Buf = *pu32Buf_tmp;
            }    

            board_jumpstart(ISP_BASE);
        }
    }
    
    //puts("Go NAND");        
    nand_read(0, (UINT32*) tbl, 2048, 0);

    fw = (firmware_t *)(tbl) + 32;

    boot_isp_func_dma(fw->size);
}
