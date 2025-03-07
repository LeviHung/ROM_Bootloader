/**
 * @file sdc.c
 * @brief
 *
 * This file implement driver of SD controller. 
 *
 * @author Levi Hung
 * @date 2009/10/12
 */

#include <bsp_common.h>
#include <sdc.h>
extern volatile UINT32 *SD_NANDCSRAM1;

/********************************************************************/
/********************************************************************/
/* SD/MMC commands */
/********************************************************************/
/********************************************************************/

int SD_CheckSDCardStatus(UINT32 CardStatus)
{
    if(CardStatus & SDC_STATUS_ERROR_BITS) {
        *SD_ErrorCode = ERR_SD_CARD_STATUS_ERROR;
        return FALSE;
    }
    
    *SD_ErrorCode = ERR_NO_ERROR;
    return TRUE;
}


int SD_SelectCardCmd(const struct SDCardStruct *SDCard)
{
    UINT32 CardStatus;

    /* send CMD7 with valid RCA to select */
    if(!SDC_SendCommand(SD_SELECT_CARD_CMD | SDC_CMD_REG_NEED_RSP, SDCard->RCA, &CardStatus)) {
        return FALSE;
    }
    
    if(!SD_CheckSDCardStatus(CardStatus)) {
        return FALSE;
    }
    
    return TRUE;
}

int SD_StopTransmissionCmd(void)
{
    UINT32 CardStatus;

    /* send CMD12 to stop transmission */
    if(!SDC_SendCommand(SD_STOP_TRANSMISSION_CMD | SDC_CMD_REG_NEED_RSP, 0, &CardStatus)) {
        return FALSE;
    }

    if(!SD_CheckSDCardStatus(CardStatus)) {
        return FALSE;
    }
    return TRUE;
}

int SD_SetBlkSizeCmd(UINT32 BlockSize)
{
    UINT32 CardStatus;

    if(!SDC_SendCommand(SD_SET_BLOCKLEN_CMD | SDC_CMD_REG_NEED_RSP, BlockSize, &CardStatus)) {
        return FALSE;
    }
    
    if(!SD_CheckSDCardStatus(CardStatus)) {
        return FALSE;
    }
		
    return TRUE;
}

int SD_ReadSingleCmd(UINT32 Addr)
{
    UINT32 CardStatus;

    if(!SDC_SendCommand(SD_READ_SINGLE_BLOCK_CMD | SDC_CMD_REG_NEED_RSP, Addr, &CardStatus)) {
        return FALSE;		
    }    

    if(!SD_CheckSDCardStatus(CardStatus)) {
        return FALSE;
    }

    return TRUE;
}

/********************************************************************/
/********************************************************************/
/* SD/MMC ACMD commands */
/********************************************************************/
/********************************************************************/

int SD_GetOCRCmd(const struct SDCardStruct *SDCard, SD_OCR HostOCR, SD_OCR *OCR)
{
    UINT32 CardStatus;
    int RetryCount = 0;

    do {
        if ((SDCard->CardType & MEMORY_CARD_TYPE_SD) == MEMORY_CARD_TYPE_SD) {
            /* send CMD55 to indicate to the card that the next command is an application specific command */
            if(!SDC_SendCommand(SD_APP_CMD | SDC_CMD_REG_NEED_RSP, SDCard->RCA, &CardStatus)) {
                return FALSE;
            }    

            if(!SD_CheckSDCardStatus(CardStatus)) {
                return FALSE;
            }
		
            /* send ACMD41 to get OCR register */
            if(!SDC_SendCommand(SD_APP_OP_COND_CMD | SDC_CMD_REG_APP_CMD | SDC_CMD_REG_NEED_RSP, (UINT32)HostOCR, (UINT32 *)OCR)) {
                return FALSE;
            }    
        }
        else if( SDCard->CardType == MEMORY_CARD_TYPE_MMC ) {
            /* send CMD1 to get OCR register */
            if(!SDC_SendCommand(SD_MMC_OP_COND | SDC_CMD_REG_NEED_RSP, (UINT32)HostOCR, (UINT32 *)OCR)) {
                return FALSE;
            }    
        }

        if(RetryCount++ > SD_CARD_GET_OCR_RETRY_COUNT) {
            *SD_ErrorCode = ERR_SD_CARD_IS_BUSY;
            printf("SD CARD GET OCR RETRY COUNT OVERFLOW\r\n");
            return FALSE;
        }
    } while( (*OCR & SD_OCR_BUSY_BIT) != SD_OCR_BUSY_BIT ) ;

    return TRUE;
}


/********************************************************************/
/********************************************************************/
/********************************************************************/
/********************************************************************/

int SD_GetCardCurrentState(const struct SDCardStruct *SDCard, UINT32 *CurrentState)
{
    if(!SDC_SendCommand(SD_SEND_STATUS_CMD | SDC_CMD_REG_NEED_RSP, SDCard->RCA , &CardStatus)) {
        return FALSE;
    }
    
    if(!SD_CheckSDCardStatus(CardStatus)) {
        return FALSE;
    }    

    *CurrentState = (CardStatus & SDC_STATUS_CURRENT_STATE) >> SDC_STATUS_CURRENT_STATE_LOC;
    return TRUE;
}

int SD_WaitOperationComplete(const struct SDCardStruct *SDCard, UINT32 NextState)
{
    UINT32 CurrentState;
    int RetryCount = 0;
    
    while(RetryCount++ < SD_CARD_WAIT_OPERATION_COMPLETE_RETRY_COUNT) {
        if(!SD_GetCardCurrentState(SDCard, &CurrentState)) {
            return FALSE;
        }    
		
        if(CurrentState == NextState) {
            return TRUE;
        }
    }
	// printf("SD CARD WAIT OPERATION COMPLETE RETRY COUNT OVERFLOW\r\n");
    return FALSE;
}

int SD_WaitCardEnterTransferState(const struct SDCardStruct *SDCard)
{
    UINT32 CurrentState;
    int RetryCount = 0;
		
    while(RetryCount++ < SD_CARD_WAIT_TRANSFER_STATE_RETRY_COUNT) {
        if(!SD_GetCardCurrentState(SDCard, &CurrentState)) {
            return FALSE;
        }
		
        switch(CurrentState) {
        case SD_IDLE_STATE:
        case SD_READY_STATE:
        case SD_IDENT_STATE:
        case SD_DIS_STATE:
        case SD_STBY_STATE:
            return FALSE;

        case SD_TRAN_STATE:
            return TRUE;
			
        case SD_DATA_STATE:
        case SD_RCV_STATE:
            if(SD_WaitOperationComplete(SDCard, SD_TRAN_STATE)) {
                return TRUE;
            }
            
            if(*SD_ErrorCode != ERR_NO_ERROR) {
                return FALSE;
            }

            if(!SD_StopTransmissionCmd()) {
                return FALSE;
            }            
            break;
            
        case SD_PRG_STATE:
            if(!SD_WaitOperationComplete(SDCard, SD_TRAN_STATE)) {
                return FALSE;
            }
            break;
        default:
            return FALSE;
        }
    }
    *SD_ErrorCode = ERR_SD_CARD_IS_BUSY;
	// printf("SD CARD WAIT TRANSFER STATE RETRY COUNT OVERFLOW\r\n");
    return FALSE;
}


int SD_SetCardTransferState(const struct SDCardStruct *SDCard)
{
    UINT32 CurrentState;
    int RetryCount = 0;
	
    while(RetryCount++ < SD_CARD_STATE_CHANGE_RETRY_COUNT) {
        if(!SD_GetCardCurrentState(SDCard, &CurrentState)) {
            return FALSE;
        }    
		
        switch(CurrentState) {
        case SD_IDLE_STATE:
        case SD_READY_STATE:
        case SD_IDENT_STATE:
            return FALSE;
			
        case SD_DIS_STATE:
            if(!SD_WaitOperationComplete(SDCard, SD_STBY_STATE)) {
                return FALSE;
            }
            break;		
        case SD_TRAN_STATE:
            return TRUE;
			
        case SD_DATA_STATE:
        case SD_RCV_STATE:
            if(SD_WaitOperationComplete(SDCard, SD_TRAN_STATE)) {
                return TRUE;
            }

            if(*SD_ErrorCode != ERR_NO_ERROR) {
                return FALSE;
            }    
					
            if(!SD_StopTransmissionCmd()) {
                return FALSE;
            }    
            break;
        case SD_PRG_STATE:
            if(!SD_WaitOperationComplete(SDCard, SD_TRAN_STATE)) {
                return FALSE;
            }    
            break;
        case SD_STBY_STATE:		
            if(!SD_SelectCardCmd(SDCard)) { 
                return FALSE;
            }        
            break;
        default:
                return FALSE;
        }
    }
    // printf("SD CARD STATE CHANGE RETRY COUNT OVERFLOW\r\n");
	
    return FALSE;
}

int SD_OperatVoltRangeValidation(struct SDCardStruct *SDCard)
{
    /* get OCR register */
    //if the host supports HCS, it should set the SDC_OCR's bit 30
    if(!SD_GetOCRCmd(SDCard, SDC_OCR, (UINT32 *) &SDCard->OCR)) { 
        return FALSE;
    }	
    
    /* ckeck the operation conditions */
    if((SDCard->OCR & SDC_OCR) == 0) {
        *SD_ErrorCode = ERR_OUT_OF_VOLF_RANGE;
        return FALSE;
    }
    
    return TRUE;
}

int SD_CardIdentification(struct SDCardStruct *SDCard, UINT32 cpuclk)
{
    UINT32 RCAResponse, CardStatus, CIDWord[4];
    UINT32 Response;
    
    if(!SDC_SendCommand(SD_GO_IDLE_STATE_CMD, (UINT32 )0, (UINT32 *)NULL) ){
            return FALSE;
    }    

    udelay(4000, cpuclk);

    //for SD 2.0, execute CMD8 first to determine the voltage supported by the card
    if(SDCard->CardType != MEMORY_CARD_TYPE_MMC) {

        if(!SDC_SendCommand(SD_SEND_IF_COND_CMD | SDC_CMD_REG_NEED_RSP, (0x1 << 8)|(0xAA << 0), &Response)) {
            ;
            //printf("CMD8: Card does not support SPEC 2.0 or voltage is not supported!\r\n");
        }
    }

    // Do operating voltage range validation
    if(!SD_OperatVoltRangeValidation(SDCard)) {
        return FALSE;
    }    

    // send CMD2 to get CID register
    if(!SDC_SendCommand(SD_ALL_SEND_CID_CMD | SDC_CMD_REG_NEED_RSP | SDC_CMD_REG_LONG_RSP, 0, CIDWord)) {
        return FALSE;
    }    

    if ((SDCard->CardType & MEMORY_CARD_TYPE_SD) == MEMORY_CARD_TYPE_SD) {
        // send CMD3 to get RCA register
        if(!SDC_SendCommand(SD_SEND_RELATIVE_ADDR_CMD | SDC_CMD_REG_NEED_RSP, 0, &RCAResponse)) {
            return FALSE;
        }    

        *SD_NANDCSRAM1 = RCAResponse;
        SDCard->RCA = *SD_NANDCSRAM1;

    } else {
        // so far, we only support one interface, so we can give RCA any value
        SDCard->RCA = 0x1;
        // send CMD3 to set RCA register
        if(!SDC_SendCommand(SD_SEND_RELATIVE_ADDR_CMD | SDC_CMD_REG_NEED_RSP, SDCard->RCA, &CardStatus)) {
            return FALSE;
        }    
			
        if(!SD_CheckSDCardStatus(CardStatus)) {
            return FALSE;
        }    
    }
			
    return TRUE;
}

/********************************************************************/
/********************************************************************/
/********************************************************************/
/********************************************************************/
int SD_ReadSingleBlock(UINT32 Addr, UINT32 BlockSize, UINT32 TimeOut, UINT32 *Buf)
{
    if(!SD_SetBlkSizeCmd(BlockSize)) {
        return FALSE;
    }    

    SDC_ConfigDataTransfer(BlockSize, SDC_DATA_CTRL_REG_DATA_READ, TimeOut);

    if(!SD_ReadSingleCmd(Addr)) {
        return FALSE;
    }    
        
    if(!SDC_ReadBlock(BlockSize, Buf)) {
        return FALSE;
    }    

    return TRUE;
}

/***************************************************************************
SD Card Read/Write/Erase Function
***************************************************************************/
int SDCardReadSector(const struct SDCardStruct *SDCard, UINT32 StartBlk, UINT32 BlkCount, UINT32 *Buf)
{
    int TurnBlkCount;
    UINT32 StartAddr;

    *SD_ErrorCode = ERR_NO_ERROR;

    if(!SD_SetCardTransferState(SDCard)) {
        return FALSE;
    }    

    if( SDCard->OCR & SD_OCR_STATUS_BIT) {		//high capacity support
        StartAddr = StartBlk;
    } else {
        StartAddr = StartBlk * SDCard->CSD_ReadBlockLength;
    }

    TurnBlkCount=(int)BlkCount;
		
    while(TurnBlkCount > 0)	{
        if(!SD_ReadSingleBlock(StartAddr, SDCard->CSD_ReadBlockLength, SDCard->ReadAccessTimoutCycle, Buf)) {
            return FALSE;
        }
        
        if(!SD_WaitCardEnterTransferState(SDCard)) {
			return FALSE;
        }
        
        TurnBlkCount -= Max_MultiBlock_Num;
        if( SDCard->OCR & SD_OCR_STATUS_BIT ) {		//high capacity support
            StartAddr += Max_MultiBlock_Num;

        } else {
            StartAddr += SDCard->CSD_ReadBlockLength;
        }    

        Buf += (SDCard->CSD_ReadBlockLength >> 2);
    }
    
    return TRUE;
}

int SD_RootInit(struct SDCardStruct *SDCard, UINT32 cpuclk)
{
    /// [0x38] <20091102> Levi: Modify 0x7F to 133MHz: 500K, 
    REG32(SDC_BASE + SDC_CLOCK_CTRL_REG) =  
            (REG32(SDC_BASE + SDC_CLOCK_CTRL_REG) & ~SDC_CLOCK_REG_CLK_DIV) + 0x7F;  
    if ((REG32(SDC_BASE + SDC_STATUS_REG) & SDC_STATUS_REG_CARD_INSERT) != SDC_CARD_INSERT) {
        return FALSE;
    }
    
    //The default is disable in SDIO Host
    // SDC_SetCLKEnable(TRUE);
    REG32(SDC_BASE + SDC_CLOCK_CTRL_REG) = REG32(SDC_BASE + SDC_CLOCK_CTRL_REG) & ~SDC_CLOCK_REG_CLK_DIS ;

    *SD_ErrorCode = ERR_NO_ERROR;
    // At first, set card type to SD
    SDCard->CardType = MEMORY_CARD_TYPE_COMBO;
    // set memory card type
    SDC_SetCardType((int) SDCard->CardType);

    // start card idenfication process
    if(!SD_CardIdentification(SDCard, cpuclk)) {
        return FALSE;
    }

    SDCard->CSD_ReadBlockLength = 512;

    // Set card bus clock. SDC_SetCardBusClock() will give the real card bus clock has been set.
    // CardBusClock = SDC_SetCardBusClock(SDCard, 1000000/*SDCard->SysFrequency*/); // 1000000;
    *((volatile UINT32 *)(SDC_BASE + SDC_CLOCK_CTRL_REG)) =  
        (*((volatile UINT32 *)(SDC_BASE + SDC_CLOCK_CTRL_REG)) & ~SDC_CLOCK_REG_CLK_DIV) + 0xE;

    SDCard->ReadAccessTimoutCycle = 100000;
    SDCard->WriteAccessTimoutCycle = 250000;
    
    return TRUE;
}


void SDCardInsert(struct SDCardStruct *SDCard)
{
    UINT32 CmdReg;
    UINT32 u32SD_CLK;

    u32SD_CLK = u32ReadCPUCLK();
    
    /// delay 10ms 500 * 1.32u * 9 * 2 = 11.880 ms
    udelay(50000, u32SD_CLK);

    REG32(SDC_BASE + SDC_POWER_CTRL_REG) =REG32(SDC_BASE + SDC_POWER_CTRL_REG) | SDC_POWER_REG_POWER_ON ;
    REG32(GPIO_BASE + GPIO_PINDIR) = REG32(GPIO_BASE + GPIO_PINDIR) | GPIO_SD_POWER_ON ;
    REG32(GPIO_BASE + GPIO_DATASET) = REG32(GPIO_BASE + GPIO_DATASET) | GPIO_SD_POWER_ON ;
    REG32(SCU_BASE + SCU_MEM_SOURCE) = REG32(SCU_BASE + SCU_MEM_SOURCE) | SCU_SD_CLK_EN ;

    // reset host interface controller
    REG32(SDC_BASE + SDC_CMD_REG) = SDC_CMD_REG_SDC_RST;

    udelay(1000, u32SD_CLK);    
    do {                /* loop, until the reset bit is clear */
        CmdReg = REG32(SDC_BASE + SDC_CMD_REG);
    }while((CmdReg & SDC_CMD_REG_SDC_RST) != 0);

    /// delay 9ms 500 * 1u * 9 * 2
    udelay(12000, u32SD_CLK);
    if(!SD_RootInit(SDCard, u32SD_CLK)) {
        return;
    }
    
    // set interrupt mask register
    REG32(SDC_BASE +SDC_INT_MASK_REG) = SDC_STATUS_REG_CARD_CHANGE;
}

void SDHostInterfaceInit(struct SDCardStruct *SDCard)
{
    /* init */		
    *SD_ErrorCode = ERR_NO_ERROR;

    SDCard->RCA = 0;
    // Check SD existed or not

    /// [0x2c]: 0x00000400
    *((UINT32 *)(SDC_BASE + SDC_CLEAR_REG)) = SDC_STATUS_REG_CARD_CHANGE; 

    SDCardInsert(SDCard);
}

#if 0
int SD_SendStatusCmd(SDCardStruct *SDCard, UINT32 *CardStatus)
{
    // send CMD13 to get card status
    // return SDC_SendCommand(SD_SEND_STATUS_CMD | SDC_CMD_REG_NEED_RSP, ((SDCard->RCA & 0x0000ffff) << 16), CardStatus);
    return SDC_SendCommand(SD_SEND_STATUS_CMD | SDC_CMD_REG_NEED_RSP, SDCard->RCA , CardStatus);
}
#endif