/**
 * @file sd_cntr.c
 * @brief
 *
 * This file implement low-level driver of SD controller.
 *
 * @author Levi Hung
 * @date 2009/10/08
 */

#include <bsp_common.h>
#include <sdc.h>

#define SDCNTR_STATUS_RETRY_COUNT       (0x100000*100)


/********************************************************************/
/********************************************************************/
/********************************************************************/
/********************************************************************/

void SDC_SetCardType(int CardType)
{
    UINT32 ClockReg;

    ClockReg = REG32(SDC_BASE + SDC_CLOCK_CTRL_REG);     /// [0x38]:
    
    ClockReg &= ~SDC_CLOCK_REG_CARD_TYPE;

    if((CardType & MEMORY_CARD_TYPE_SD) == MEMORY_CARD_TYPE_SD) {
        ClockReg |= SDC_CARD_TYPE_SD;
    } else {
        ClockReg |= SDC_CARD_TYPE_MMC;
    }
    
    REG32(SDC_BASE + SDC_CLOCK_CTRL_REG) = ClockReg;     /// [0x38]
}

void SDC_ConfigDataTransfer(UINT32 DataLen, UINT32 ReadWriteMode, UINT32 TimeOut)
{
    UINT32 change_endian;
    UINT32 swap_HL;

    /* write time out */
    REG32(SDC_BASE + SDC_DATA_TIMER_REG) = TimeOut << 2;
    
    //card gives timeout unit maybe 25MHz, but sdc gives timeout unit may have been "timeout_value x io_sd_clk/25MHz"
	
    /* set data length */
    REG32(SDC_BASE + SDC_DATA_LEN_REG) = DataLen;

    if( REG32(SDC_BASE + SDC_DATA_CTRL_REG) & SDC_CPRM_DATA_CHANGE_ENDIAN_EN ) {
        change_endian = TRUE;
    } else {
        change_endian = FALSE;
    }
    
    if( REG32(SDC_BASE + SDC_DATA_CTRL_REG) & SDC_CPRM_DATA_SWAP_HL_EN) {
        swap_HL = TRUE;
    } else {
        swap_HL = FALSE;
    }

    /* set data block */
    REG32(SDC_BASE + SDC_DATA_CTRL_REG) = 0x9 | ReadWriteMode | SDC_DATA_CTRL_REG_DATA_EN;

    //for CPRM change high word/low word and change endian
    if( change_endian == TRUE ) {
        REG32(SDC_BASE + SDC_DATA_CTRL_REG) = REG32(SDC_BASE + SDC_DATA_CTRL_REG) | SDC_CPRM_DATA_CHANGE_ENDIAN_EN;
    }
    
    if( swap_HL == TRUE ) {
        REG32(SDC_BASE + SDC_DATA_CTRL_REG) = REG32(SDC_BASE + SDC_DATA_CTRL_REG) | SDC_CPRM_DATA_SWAP_HL_EN;
    }    
}

/********************************************************************/
/********************************************************************/
/********************************************************************/
/********************************************************************/
int SDC_SendCommand(UINT32 Cmd, UINT32 Argument, UINT32 *Response)
{
    UINT32 Status, RetryCount = 0;
    UINT32 i;

    // clear command relative bits of status register
    REG32(SDC_BASE + SDC_CLEAR_REG) = SDC_STATUS_REG_RSP_CRC_FAIL | SDC_STATUS_REG_RSP_TIMEOUT | 
                                                 SDC_STATUS_REG_RSP_CRC_OK| SDC_STATUS_REG_CMD_SEND;
    
    // write argument to arugument register if necessary
    REG32(SDC_BASE + SDC_ARGU_REG) = Argument;
    
    // send command
    REG32(SDC_BASE + SDC_CMD_REG) = Cmd | SDC_CMD_REG_CMD_EN;    /// [0x00] 0x00000200

    // wait for the CMD_SEND bit of stats register is set
    while(RetryCount++ < SDCNTR_STATUS_RETRY_COUNT) {
        Status = REG32(SDC_BASE + SDC_STATUS_REG);

        if (!(Cmd & SDC_CMD_REG_NEED_RSP)) {
            // if this command does not need response, wait command sent flag
            if(Status & SDC_STATUS_REG_CMD_SEND) {
                // clear command sent bit
                REG32(SDC_BASE + SDC_CLEAR_REG) = SDC_STATUS_REG_CMD_SEND;
                *SD_ErrorCode = ERR_NO_ERROR;
                return TRUE;
            }
        } else {
            /* if this command needs response */
            if(Status & SDC_STATUS_REG_RSP_TIMEOUT) {
                // clear response timeout bit
                //printf(" SDC_STATUS_REG_RSP_TIMEOUT..................\r\n");
                REG32(SDC_BASE + SDC_CLEAR_REG) = SDC_STATUS_REG_RSP_TIMEOUT;
                *SD_ErrorCode = ERR_RSP_TIMEOUT_ERROR;				
                return FALSE;
                
            } else if(Status & SDC_STATUS_REG_RSP_CRC_FAIL) {
                // clear response fail bit
                REG32(SDC_BASE + SDC_CLEAR_REG) = SDC_STATUS_REG_RSP_CRC_FAIL;
                *SD_ErrorCode = ERR_RSP_CRC_ERROR;
                return FALSE;
                
            } else if(Status & SDC_STATUS_REG_RSP_CRC_OK) {			
                /* clear response OK bit */
                REG32(SDC_BASE + SDC_CLEAR_REG) = SDC_STATUS_REG_RSP_CRC_OK;
                /* if it is long response */
                if(Cmd & SDC_CMD_REG_LONG_RSP) {
                    for(i = 0; i < 4; i++, Response++) {
                        *Response = REG32(SDC_BASE + SDC_RESPONSE0_REG + (i << 2));
                    }
                    
                } else {                
                    *Response = REG32(SDC_BASE + SDC_RESPONSE0_REG);
                }

                *SD_ErrorCode = ERR_NO_ERROR;
				
                return TRUE;
            }
        }
    }

    *SD_ErrorCode = ERR_SEND_COMMAND_TIMEOUT;
    //printf("CMD(%x)(%d) SEND STATUS RETRY COUNT OVERFLOW\r\n", Cmd, Cmd<<26>>26);
    return FALSE;
}


UINT32 SDC_WaitReadFIFOReady(void)
{
    UINT32 Status;
    int RetryCount = 0;

    while(RetryCount++ < SDCNTR_STATUS_RETRY_COUNT) {
        Status = REG32(SDC_BASE + SDC_STATUS_REG);
        
        if(Status & SDC_STATUS_REG_FIFO_OVERRUN) {
            /* clear FIFO overrun bit */
            REG32(SDC_BASE + SDC_CLEAR_REG) = SDC_STATUS_REG_FIFO_OVERRUN;
            return SDC_STATUS_REG_FIFO_OVERRUN;

        } else if(Status & SDC_STATUS_REG_DATA_TIMEOUT)	{
            /* clear data timeout bit */
            // printf("Wait Read FIFO TimeOut, Status=0x%x\r\n", Status);
            REG32(SDC_BASE + SDC_CLEAR_REG) = SDC_STATUS_REG_DATA_TIMEOUT;
            *SD_ErrorCode = ERR_DATA_TIMEOUT_ERROR;
            return FALSE;
		}
	}

    *SD_ErrorCode = ERR_WAIT_OVERRUN_TIMEOUT;
    // printf("WaitReadFIFOReady STATUS RETRY COUNT OVERFLOW, Status:%x\r\n", Status);
    return FALSE;
}


int SDC_ReadBlock(UINT32 tranSize, UINT32 *Buf)
{
    UINT32 WordCount, Status, i;

    while(tranSize > 0)	{
        Status = REG32(SDC_BASE + SDC_STATUS_REG); 

        if(Status & SDC_STATUS_REG_DATA_CRC_FAIL) {
            /* clear data CRC fail bit */
            //printf("SDC_CheckBlockDataCRC fail!\r\n");
            REG32(SDC_BASE + SDC_CLEAR_REG) = SDC_STATUS_REG_DATA_CRC_FAIL; 
            return FALSE;
        }
        
        Status = SDC_WaitReadFIFOReady();
        if(!Status)	{
            //printf("FIFO error...........\r\n");
            return FALSE;
        }

        if(Status & SDC_STATUS_REG_FIFO_OVERRUN) {
            //printf("FIFO ready to read\n");
            // read data from FIFO
            if(tranSize >= (SDC_FIFO_LEN * 4)) {
                WordCount = SDC_FIFO_LEN;

                // read data from FIFO
                for(i = 0; i < WordCount; i++, Buf++) {
                    *Buf = REG32(SDC_BASE + SDC_DATA_WINDOW_REG); 
                }

                tranSize -= WordCount * 4;
            } else {
                WordCount = tranSize >> 2;

                for(i = 0; i < WordCount; i++, Buf++) {
                    *Buf = REG32(SDC_BASE + SDC_DATA_WINDOW_REG);
                }
                tranSize -= WordCount * 4;
            }
        }
    }
	// return SDC_CheckBlockDataCRC(SDCard);
    return TRUE;
}

