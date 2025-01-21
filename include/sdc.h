/**
 * @file sdc.h
 * @brief
 *
 * This file define SD command and sd config option
 *
 * @author Levi Hung
 * @date 2009/10/12
 */
 
#ifndef _SD_H
#define _SD_H

/* so far, SD controller support 3.2-3.3 VDD */	   //2.6V-3.6V
//it identifies that host supports HCS and voltage from 2.7 to 3.6
//bit 30 is for high capacity support, bit 31 is needed by MXI MMC card
#define SDC_OCR                         0xC0FF8000


#define SDC_CARD_INSERT                 0x0
#define SDC_CARD_REMOVE                 SDC_STATUS_REG_CARD_REMOVE

#define SDC_CARD_TYPE_MMC               0x0		
#define SDC_CARD_TYPE_SD                SDC_CLOCK_REG_CARD_TYPE

/* data window register */
//Now, the sdc fifo depth is 16
#define SDC_FIFO_LEN                    (REG32(SDC_BASE + SDC_FEATURE_REG)&0xff)        

#define SD_CARD_GET_OCR_RETRY_COUNT                 0x1000
#define SD_CARD_WAIT_OPERATION_COMPLETE_RETRY_COUNT 8000
#define SD_CARD_STATE_CHANGE_RETRY_COUNT            10000
#define SD_CARD_WAIT_TRANSFER_STATE_RETRY_COUNT     0x10000	

#define MEMORY_CARD_TYPE_MMC            1
#define MEMORY_CARD_TYPE_SD             2
#define MEMORY_CARD_TYPE_SDIO           4
#define MEMORY_CARD_TYPE_COMBO          6

/********************************************************************/
/* SYSTEM ERROR_CODE */ //we don't know how these errors are defined.
/********************************************************************/
#define ERR_NO_ERROR                    0x00000000

/* general error */
#define ERR_CARD_NOT_EXIST              0x00000001
#define ERR_OUT_OF_VOLF_RANGE           0x00000002
#define ERR_SD_PARTIAL_READ_ERROR       0x00000004
#define ERR_SD_PARTIAL_WRITE_ERROR      0x00000008
#define ERR_SD_CARD_IS_LOCK             0x00000010

/* command error */
#define ERR_DATA_CRC_ERROR              0x00000100
#define ERR_RSP_CRC_ERROR               0x00000200
#define ERR_DATA_TIMEOUT_ERROR          0x00000400
#define ERR_RSP_TIMEOUT_ERROR           0x00000800
#define ERR_WAIT_OVERRUN_TIMEOUT        0x00001000
#define ERR_WAIT_UNDERRUN_TIMEOUT       0x00002000
#define ERR_WAIT_DATA_CRC_TIMEOUT       0x00004000
#define ERR_WAIT_TRANSFER_END_TIMEOUT   0x00008000
#define ERR_SEND_COMMAND_TIMEOUT        0x00010000

/* sd error */
#define ERR_SD_CARD_IS_BUSY             0x00100000
#define ERR_CID_REGISTER_ERROR          0x00200000
#define ERR_CSD_REGISTER_ERROR          0x00400000

/* sd card status error */
#define ERR_SD_CARD_STATUS_ERROR        0x01000000

/* sdio card status error */
#define ERR_SDIO_CARD_STATUS_ERROR      0x02000000
#define ERR_SDIO_CARD_NOT_READY         0x04000000


/********************************************************************/
/* SD command response type */
/********************************************************************/
#define SD_NO_RESPONSE                  0
#define SD_RESPONSE_R1                  1
#define SD_RESPONSE_R1b                 2
#define SD_RESPONSE_R2                  3		//R2 uses 136 bits, and others use 48 bits
#define SD_RESPONSE_R3                  4
#define SD_RESPONSE_R6                  5

/********************************************************************/
/* SD command */
/********************************************************************/
#define SD_GO_IDLE_STATE_CMD            0
#define SD_MMC_OP_COND                  1
#define SD_ALL_SEND_CID_CMD             2
#define SD_SEND_RELATIVE_ADDR_CMD       3
#define SD_SET_DSR_CMD                  4
#define SDIO_IO_SEND_OP_COND_CMD        5
#define SD_SWITCH_FUNC_CMD              6
#define MMC_SWITCH_FUNC_CMD             6
#define SD_SELECT_CARD_CMD              7
#define SD_SEND_IF_COND_CMD             8
#define MMC_SEND_EXT_CSD                8
#define SD_SEND_CSD_CMD                 9
#define SD_SEND_CID_CMD                 10
#define SD_STOP_TRANSMISSION_CMD        12
#define SD_SEND_STATUS_CMD              13
#define SD_GO_INACTIVE_STATE_CMD        15
#define SD_SET_BLOCKLEN_CMD             16
#define SD_READ_SINGLE_BLOCK_CMD        17
#define SD_READ_MULTIPLE_BLOCK_CMD      18
#define SD_WRITE_SINGLE_BLOCK_CMD       24
#define SD_WRITE_MULTIPLE_BLOCK_CMD     25
#define SD_PROGRAM_CSD_CMD              27

//A High Capacity SD Memory Card does not support commands 28, 29, 30
#define SD_SET_WRITE_PROT_CMD           28
#define SD_CLR_WRITE_PROT_CMD           29
#define SD_SEND_WRITE_PROT_CMD          30

#define SD_ERASE_SECTOR_START_CMD       32
#define SD_ERASE_SECTOR_END_CMD         33
#define SD_ERASE_CMD                    38
#define SD_LOCK_UNLOCK_CMD              42
#define SDIO_IO_RW_DIRECT_CMD           52
#define SDIO_IO_RW_EXTENDED_CMD         53	
#define SD_APP_CMD                      55
#define SD_GEN_CMD                      56		// transfer/get a data block to/from the card

/* ACMD commands */
#define SD_SET_BUS_WIDTH_CMD            6
#define SD_STATUS_CMD                   13
#define SD_SEND_NUM_WR_BLOCKS_CMD       22
#define SD_SET_WR_BLK_ERASE_COUNT_CMD   23
#define SD_APP_OP_COND_CMD              41		// sends HCS and asks the card's OCR content in the response on CMD line
#define SD_SET_CLR_CARD_DETECT          42
#define SD_SEND_SCR_CMD                 51


/********************************************************************/
/* SD card OCR register */
/********************************************************************/
#define SD_OCR_STATUS_BIT               0x40000000	// to check if the card supports high capacity
#define SD_OCR_BUSY_BIT                 0x80000000

#define Max_MultiBlock_Num              1

typedef UINT32 SD_OCR;

/* CSD register fields V 2.0 */
typedef struct
{
    UINT32 ReadBlockLength;
}SD_CSD;

//typedef volatile UINT32 SD_RCA;
typedef UINT32 SD_RCA;

/* state */
#define SDC_STATUS_CURRENT_STATE_LOC    9

#define SD_IDLE_STATE                   0
#define SD_READY_STATE                  1
#define SD_IDENT_STATE                  2
#define SD_STBY_STATE                   3
#define SD_TRAN_STATE                   4
#define SD_DATA_STATE                   5
#define SD_RCV_STATE                    6
#define SD_PRG_STATE                    7
#define SD_DIS_STATE                    8

/********************************************************************/
/* SD Card structure */
/********************************************************************/
typedef struct SDCardStruct
{
    UINT32 CardType;        /// host interface configuration
    //SD_OCR OCR;
    UINT32 OCR;                 /// card register
    //SD_CSD CSD;
    UINT32 CSD_ReadBlockLength;
    //SD_RCA RCA;
    UINT32 RCA;
    UINT32 ReadAccessTimoutCycle;       /// access time out
    UINT32 WriteAccessTimoutCycle;   
} _SDCardStruct;

/********************************************************************/
/* Global variables */
/********************************************************************/
/* Public API */
extern void SDHostInterfaceInit(struct SDCardStruct *SDCard);
extern int SDCardReadSector(const 
struct SDCardStruct *SDCard, UINT32 StartBlk, UINT32 BlkCount, UINT32 *Buf);

/* Private API */
extern void SDC_SetCardType(int CardType);
extern void SDC_ConfigDataTransfer(UINT32 DataLen, UINT32 ReadWriteMode, UINT32 TimeOut);
extern int SDC_SendCommand(UINT32 Cmd, UINT32 Argument, UINT32 *Response);
extern int SDC_ReadBlock(UINT32 BlockSize, UINT32 *Buf);
#endif
