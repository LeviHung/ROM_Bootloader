/**
 * @file board.c
 * @brief
 *
 * This file implement bsp functions of ROM bootloader
 *
 * @author Levi Hung
 * @date 2009/10/08
 */

#include <bsp_common.h>
#include <nandc.h>
#include <sdc.h>

#ifdef _DEBUG
UINT32 *ISP_Base = (UINT32 *)0x00100000;
#else
UINT32 *ISP_Base = (UINT32 *)0x20100000;
#endif 

void copy_err(void)
{
    puts("No Memory");
    for (;;) {;}
}

void board_init_nandc(void)
{    
    /// [0x30C] Software Reset Control: ECC Disable(Read/Write must be enable)
    REG32(NANDC_BASE + NANDC_SRST)      = 0x00000007; 

    /// [0x128] Interrupt Enable: disable all          
    REG32(NANDC_BASE + NANDC_INTEN)     = 0;

    /// [0x20C] Burst Mode Control: PIO mode                    
    REG32(NANDC_BASE + NANDC_BMC_BURST) = 0x00000000;

    /// [0x108] Flow Control: LSN writing mode / LSN CRC enable / Bypass CRC check           
    REG32(NANDC_BASE + NANDC_FLOW)      = 0x380;                

    /// [0x110] Memory Module Configuration
    REG32(NANDC_BASE + NANDC_MEMC)      = NANDC_MOD0_ENABLE;    
}

void board_init_nandc2(void)
{
    volatile UINT32 ID_temp;

    /// [0x104] Access Control
    REG32(NANDC_BASE + NANDC_ACC)  = ((READ_ID << 8) + 0x80);  
          
    // wait_cmd_complete(): wait Nand Flash Commmand complete
    while(((*(volatile UINT32 *)(NANDC_BASE + NANDC_ACC)) & 0x80) != 0) {;}
    
    /// [0x120] Device ID Byte 0_3
    ID_temp = REG32(NANDC_BASE +  NANDC_ID30);     
    if (ID_temp == 0) {
        puts("No NAND Flash");
        for (;;) {;}
    }
    if ((ID_temp & 0xFF) == 0x2C) {
        REG32(NANDC_BASE + NANDC_FLOW) = 0x382;
    }

    // Use hardware stap 15,16 to select page size
    ID_temp = (REG32(SCU_BASE + SCU_STRAP) >> SCU_STRAP_PAGESIZE) & 0x3 ;

    if (ID_temp == 3) {
        REG32(NANDC_BASE + NANDC_MEMC)= NANDC_BS_16P | NANDC_AP_5C | NANDC_MS_32GB | 
                                                                     NANDC_PS_2K | NANDC_MOD0_ENABLE;
    } else {
        ID_temp = (REG32(SCU_BASE + SCU_STRAP) >> SCU_STRAP_PAGESIZE) & 0xf ;
        ID_temp = ID_temp << 8 ;
        ID_temp = ID_temp | (((REG32(SCU_BASE + SCU_STRAP) >> SCU_STRAP_BLOCKSIZE) & 0x3) <<  16);

        /// [0x110] Memory Module Configuration
        REG32(NANDC_BASE + NANDC_MEMC)= NANDC_MOD0_ENABLE | NANDC_MS_32GB | ID_temp ;     

    }

    /// [0x104] Access Control: RESET(0x2)
    REG32(NANDC_BASE + NANDC_ACC) = ((RESET << 8) + 0x80);

    while(((*(volatile UINT32 *)(NANDC_BASE + NANDC_ACC)) & 0x80) != 0) {;}
}


UINT32 *SD_ErrorCode = (UINT32 *)0x90600404;
volatile UINT32 *SD_NANDCSRAM1 = (UINT32 *)0x90600408;
volatile UINT32 *SD_NANDCSRAM2 = (UINT32 *)0x9060040C;
UINT32 *SD_Header_BSP = (UINT32 *)0x90600430;

/* use NANDC SRAM allocate it */
struct SDCardStruct *SDCard_SRAM = (struct SDCardStruct *)0x90600410;  

/* return 1: success, 0:fail 
 * uart_formt: AVI_NAND_Format + 0x90600110 + 0x00020531 (6word)  
 * uart_isp_512B_1.txt
 */
int read_uart_format(void)
{
    UINT32 Reg;
    UINT32 i;
    UINT8   DDR2_A = 0;
    UINT8   DDR2_B = 0;
    UINT8   DDR2_C = 0;
    
    serial_puts ("Upgrade from UART\r\n");

    Reg = serial_getw(0);
    if (Reg == 0x5f495641) {
        Reg = serial_getw(0);
        Reg = serial_getw(0);
        Reg = serial_getw(0);

        for (i=0; i<124; i++) {
            Reg = serial_getw(0);
            if(Reg == 0x98f00004) {
                DDR2_C = 1;
            } else if (Reg == 0x99000004) {
                DDR2_A = 1;
            } else if (Reg == 0x99100004) {
                DDR2_B = 1;
            }
                        
            if(Reg & 0xFF000000) {
                i++;
                REG32(Reg) = serial_getw(0);
            } 
        }
        
        if(DDR2_C) {
            while((REG32(DDR2_C_BASE + DDR2_STATE) & 0x100) != 0x100) {;}
        } else if(DDR2_A) {                   
            while((REG32(DDR2_A_BASE + DDR2_STATE) & 0x100) != 0x100) {;}
        } else if(DDR2_B) {                   
            while((REG32(DDR2_B_BASE + DDR2_STATE) & 0x100) != 0x100) {;} 
        }
                    
        *ISP_Base = 0;
        return 1;

    } else {
        return 0;
    }   
}

#ifdef _DEBUG    
UINT32 *gu32Cnt = (UINT32 *)0x90600400;
#endif 

void read_uart_bsp(void)
{   
	INT32 u32Cnt;
    UINT32 *ISP_Base_Temp;

    ISP_Base_Temp = ISP_Base;
    
    // read Header/BSP
#ifdef _DEBUG    
    for (*gu32Cnt=0; *gu32Cnt<128; (*gu32Cnt)++) {
#else
    for (u32Cnt=0; u32Cnt<128; u32Cnt++) {
#endif
        *ISP_Base_Temp = serial_getw(0);
		ISP_Base_Temp++;
    }
    
    return ;
}    

void read_uart_lookuptable(void)
{

	INT32 u32Cnt;
    UINT32 *u32ISP_Base_Temp;
    
    u32ISP_Base_Temp = (ISP_Base + 128);
          
    *u32ISP_Base_Temp = serial_getw(0);

    if (*u32ISP_Base_Temp != 0x5f495641) {      /// AVI_
        puts("Upgrade failed");
        for (;;) {;}        
    } 
    u32ISP_Base_Temp++;
    
    *u32ISP_Base_Temp = serial_getw(0);
    u32ISP_Base_Temp++;   

    *u32ISP_Base_Temp = serial_getw(0);
    u32ISP_Base_Temp++;

    *u32ISP_Base_Temp = serial_getw(0);
    u32ISP_Base_Temp++;

#ifdef _DEBUG    
	for (*gu32Cnt = 0; *gu32Cnt < 124; (*gu32Cnt)++) {
#else
    for (u32Cnt = 0; u32Cnt < 124; u32Cnt++) {
#endif
        *u32ISP_Base_Temp = serial_getw(0);
		u32ISP_Base_Temp++;
    }    
}

void read_uart_isp(void)
{
    UINT32 u32Cnt;
    UINT32 *u32ISP_Base_Temp;
    UINT32 *u32ISP_Length;
    
    /// avoid NAND flash page 0 have isp code
    u32ISP_Base_Temp = ISP_Base + 1024;        
    u32ISP_Length  = ISP_Base + 131;

#ifdef _DEBUG    
    for (*gu32Cnt=0; *gu32Cnt < ((*u32ISP_Length) >>2); (*gu32Cnt)++) {
#else
    for (u32Cnt=0; u32Cnt < ((*u32ISP_Length) >> 2); u32Cnt++) {
#endif
        *u32ISP_Base_Temp = serial_getw(0);
        u32ISP_Base_Temp++;
    }    
}    

void read_sd_bsp_lt_isp(void)
{
    UINT32 *u32ISP_Length;

    u32ISP_Length    = ISP_Base + 131;

    if( !SDCardReadSector(SDCard_SRAM, 0x2, (*u32ISP_Length >> 9) + 1, ISP_Base + 1024)) {
        puts("Upgrade failed");
        for (;;) {;}        
    }
}

int read_sd_format(void)
{
    UINT32 Reg;
    UINT32 i;
    UINT8   DDR2_A = 0;
    UINT8   DDR2_B = 0;
    UINT8   DDR2_C = 0;
    UINT32 *ISP_Base_Temp;

    ISP_Base_Temp = SD_Header_BSP;
    
    // read Header/BSP        
    if (*ISP_Base_Temp == 0x5f495641) {
        ISP_Base_Temp++;
        ISP_Base_Temp++;
        ISP_Base_Temp++;
        ISP_Base_Temp++;

        //serial_puts ("Upgrade ISP from SD Card\r\n");                  
        for (i=0; i<124; i++) {
            Reg = *ISP_Base_Temp;
            ISP_Base_Temp++;
                       
            if(Reg & 0xFF000000) {
                i++;
                REG32(Reg) = *ISP_Base_Temp;
                ISP_Base_Temp++;
                            
                if(Reg == 0x98f00004) {
                    DDR2_C = 1;
                } else if (Reg == 0x99000004) {
                    DDR2_A = 1;
                } else if (Reg == 0x99100004) {
                    DDR2_B = 1;
                }

            } 
        }

        if(DDR2_C) {
            while((REG32(DDR2_C_BASE + DDR2_STATE) & 0x100) != 0x100) {;}
        } else if(DDR2_A) {                   
            while((REG32(DDR2_A_BASE + DDR2_STATE) & 0x100) != 0x100) {;}
        } else if(DDR2_B) {
            while((REG32(DDR2_B_BASE + DDR2_STATE) & 0x100) != 0x100) {;} 
        }
                    
        *ISP_Base = 0;
        return 1;

    } else {
        return 0;
    }
    
}
void board_bsp_strap(void)
{    
    /* determine if SD card exist */
    if ((REG32(SDC_BASE + SDC_STATUS_REG) & SDC_STATUS_REG_CARD_INSERT) == SDC_CARD_INSERT) {
        SDHostInterfaceInit(SDCard_SRAM);

        if (!(SDCardReadSector(SDCard_SRAM, 0x0, 1, SD_Header_BSP))) {     /// 0x90600430
            goto EndUpgrade;
        }
        *SD_NANDCSRAM2 = 1;
        if (read_sd_format() == 0) {
            *SD_NANDCSRAM2 = 0;         /// SD card exist, but No AVI ISP format
            return;
        }
        if (!(SDCardReadSector(SDCard_SRAM, 0x0, 2, ISP_Base))) {
            goto EndUpgrade;
        }
        read_sd_bsp_lt_isp();
        return;
        
    } else {
        while(read_uart_format() == 0) {
            goto EndUpgrade;
        }
        *SD_NANDCSRAM2 = 1;

        // serial_puts("bsp");
        read_uart_bsp();
        // serial_puts("lt");
        read_uart_lookuptable();
        // serial_puts("isp");
        read_uart_isp();
        // serial_puts("go");
        return ;
    }
    
EndUpgrade:
    puts("Upgrade failed");
    for (;;) {;}
}

/*
 * header third word compare error, use goto jump 
 */ 
 
void board_init_ddr2(void)
{
    UINT32 Reg;
    UINT32 u32Cnt;

    /// [0x30C] Software Reset Control: ECC Enable(Read/Write must be enable)
    REG32(NANDC_BASE + NANDC_SRST) =  0x00000007;         

    /// [0x128] Interrupt Enable, [BIT7] Set interrupt enable bit and 
    /// [BIT3] ECC fail time out interrupt enable bit   
    REG32(NANDC_BASE + NANDC_INTEN)    = *(unsigned int *) (NANDC_BASE + NANDC_INTEN)&(~0x8);
    
     /// [0x10C] Page Index: page_index
    REG32(NANDC_BASE + NANDC_ADDR)     = 0;

    /// [0x308] Page Count: page_cnt_reg       
    REG32(NANDC_BASE + NANDC_PAGE_CNT) = 1;        

    /// [0x104] Access Control: PAGE_READ: 0x5 (command:0x0)
    REG32(NANDC_BASE + NANDC_ACC)      = ((PAGE_READ<<8)+0x80); 
    
    while ((REG32(NANDC_BASE + NANDC_BMC_PIO_CTL) & 0x1) == 0) {;}

    if (REG32(NANDC_BASE + NANDC_BMC_DATA_PORT) == 0x5f495641) {         /// magic number: AVI_
        while ((REG32(NANDC_BASE + NANDC_BMC_PIO_CTL) & 0x1) == 0) {;}

        Reg = REG32(NANDC_BASE + NANDC_BMC_DATA_PORT);
        while ((REG32(NANDC_BASE + NANDC_BMC_PIO_CTL) & 0x1) == 0) {;}

        Reg = REG32(NANDC_BASE + NANDC_BMC_DATA_PORT);
        while ((REG32(NANDC_BASE + NANDC_BMC_PIO_CTL) & 0x1) == 0) {;}
                
        Reg = REG32(NANDC_BASE + NANDC_BMC_DATA_PORT);

        for (u32Cnt=0; u32Cnt<124; u32Cnt++) {
            while ((REG32(NANDC_BASE + NANDC_BMC_PIO_CTL) & 0x1) == 0) {;}
            Reg = REG32(NANDC_BASE + NANDC_BMC_DATA_PORT) ;
                        
            if(Reg & 0xFF000000) {
                u32Cnt++;
                REG32(Reg) = REG32(NANDC_BASE + NANDC_BMC_DATA_PORT) ;;
                            
                if(Reg == 0x98f00004) {
                    while((REG32(DDR2_C_BASE + DDR2_STATE) & 0x100) != 0x100) {;}
                    *ISP_Base = 0;
                } else if (Reg == 0x99000004) {
                    while((REG32(DDR2_A_BASE + DDR2_STATE) & 0x100) != 0x100) {;}
                } else if (Reg == 0x99100004) {
                    while((REG32(DDR2_B_BASE + DDR2_STATE) & 0x100) != 0x100) {;} 
                }

            } 
        }
    } else {
        goto ErrorHeader;
    }
    
    return ;

ErrorHeader:
    /// [0x30C] Software Reset Control: ECC Enable(Read/Write must be enable)
    REG32(NANDC_BASE + NANDC_SRST) =  0x00000107;         
    
    //puts("No BSP");
    REG32(SERIAL_BASE) = 'N';
    while ((REG32(SERIAL_BASE + 0x14)  & LSR_THRE) == 0) {;}    REG32(SERIAL_BASE) = 'o';
    while ((REG32(SERIAL_BASE + 0x14)  & LSR_THRE) == 0) {;}    REG32(SERIAL_BASE) = ' ';
    while ((REG32(SERIAL_BASE + 0x14)  & LSR_THRE) == 0) {;}    REG32(SERIAL_BASE) = 'B';
    while ((REG32(SERIAL_BASE + 0x14)  & LSR_THRE) == 0) {;}    REG32(SERIAL_BASE) = 'S';
    while ((REG32(SERIAL_BASE + 0x14)  & LSR_THRE) == 0) {;}    REG32(SERIAL_BASE) = 'P';
    for(;;) {;}
    
}


void board_jumpstart(UINT32 addr)
{
    register jsfunc_t pfLoader = (jsfunc_t)addr;
	
    pfLoader();
}

extern void main (void);	// linked from main.c

void start_bootcode2 (void)
{	
	main ();
}

