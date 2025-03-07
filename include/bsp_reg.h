/**
 * @file bsp_reg.h
 * @brief
 *
 * This file define base address and offset of BSP register.
 *
 * @author Levi Hung
 * @date 2009/10/12
 */
/*
 *    SoC
 *     +- AHBC_BASE     (0x9000_0000)
 *     +- SMC_BASE      (0x9020_0000)
 *     +- DMAC_BASE     (0x9030_0000)
 *     +- SDC_BASE      (0x9050_0000)
 *     +- NANDC_BASE    (0x9060_0000)
 *     +- SCU_BASE      (0x9800_0000)
 *     +- SERIAL_BASE   (0x9810_0000)
 *     +- SERIAL_BASE2  (0x9820_0000)
 *     +- INTC_BASE     (0x9880_0000)
 *     +- DDR2_C_BASE   (0x98f0_0000)
 *     +- DDR2_C_BASE   (0x9900_0000)
 *     +- DDR2_C_BASE   (0x9910_0000)
 */
#ifndef __BSP_REG_H
#define __BSP_REG_H

/* AHBC: AHB Controller */
#define AHBC_BASE               0x90000000
#define AHBC_ICR		        0x88

#define AHBC_SLAVE4_BSR         0x10
#define AHBC_SLAVE6_BSR         0x18
#define AHBC_REMAP              (1 << 0)

#define AHBC_SIZE_256M          (0x8 << 16)
#define AHBC_SIZE_512M          (0x9 << 16)


/* SMC: Static Memory Controller */
#define SMC_BASE		        0x90200000

#define SMC_BANK0_CR            0x00
#define SMC_BANK0_TPR           0x04

/* DMAC: DMA Controller */
#define DMAC_BASE			    0x90300000
#define DMAC_NANDC_CH	        (DMAC_BASE + 0x120)	/// DMA_Channel_1
#define DMAC_CSR	            0x000
#define DMAC_CFG	            0x004
#define DMAC_SRC	            0x008
#define DMAC_DST	            0x00C
#define DMAC_LLP	            0x010
#define DMAC_SIZE	            0x014

/* SDC: SD Controller */
#define SDC_BASE                0x90500000
#define SDC_CMD_REG             0x000
#define SDC_ARGU_REG            0x004
#define SDC_RESPONSE0_REG       0x008
#define SDC_RESPONSE1_REG       0x00C
#define SDC_RESPONSE2_REG       0x010
#define SDC_RESPONSE3_REG       0x014
#define SDC_RSP_CMD_REG         0x018
#define SDC_DATA_CTRL_REG       0x01C
#define SDC_DATA_TIMER_REG      0x020
#define SDC_DATA_LEN_REG        0x024
#define SDC_STATUS_REG          0x028
#define SDC_CLEAR_REG           0x02C
#define SDC_INT_MASK_REG        0x030
#define SDC_POWER_CTRL_REG      0x034
#define SDC_CLOCK_CTRL_REG      0x038
#define SDC_BUS_WIDTH_REG       0x03C
#define SDC_DATA_WINDOW_REG     0x040
#define SDC_FEATURE_REG         0x09C
#define SDC_REVISION_REG        0x0A0

#define SDC_CMD_REG_INDEX       0x0000003F  /// bit mapping of command register 
#define SDC_CMD_REG_NEED_RSP    0x00000040
#define SDC_CMD_REG_LONG_RSP    0x00000080
#define SDC_CMD_REG_APP_CMD     0x00000100
#define SDC_CMD_REG_CMD_EN      0x00000200
#define SDC_CMD_REG_SDC_RST     0x00000400
#define SDC_CMD_MMC_INT_STOP    0x00000800

#define SDC_DATA_CTRL_REG_BLK_SIZE		0x0000000F  /// bit mapping of data control register 
#define SDC_DATA_CTRL_REG_DATA_WRITE	0x00000010
#define SDC_DATA_CTRL_REG_DATA_READ		0x00000000
#define SDC_DATA_CTRL_REG_DMA_EN		0x00000020
#define SDC_DATA_CTRL_REG_DATA_EN		0x00000040
#define SDC_DATA_CTRL_FIFOTH			0x00000080
#define SDC_DATA_CTRL_DMA_TYPE			0x00000300
#define SDC_DATA_CTRL_FIFO_RST			0x00000400
#define SDC_CPRM_DATA_CHANGE_ENDIAN_EN	0x00000800
#define SDC_CPRM_DATA_SWAP_HL_EN		0x00001000


#define SDC_STATUS_REG_RSP_CRC_FAIL     BIT0    /// bit mapping of status/clear/mask register 
#define SDC_STATUS_REG_DATA_CRC_FAIL    BIT1
#define SDC_STATUS_REG_RSP_TIMEOUT      BIT2
#define SDC_STATUS_REG_DATA_TIMEOUT     BIT3
#define SDC_STATUS_REG_RSP_CRC_OK       BIT4
#define SDC_STATUS_REG_DATA_CRC_OK      BIT5
#define SDC_STATUS_REG_CMD_SEND         BIT6
#define SDC_STATUS_REG_DATA_END         BIT7
#define SDC_STATUS_REG_FIFO_UNDERRUN    BIT8
#define SDC_STATUS_REG_FIFO_OVERRUN     BIT9
#define SDC_STATUS_REG_CARD_CHANGE      BIT10
#define SDC_STATUS_REG_CARD_INSERT      BIT11
#define SDC_STATUS_REG_CARD_REMOVE      BIT11
#define SDC_STATUS_REG_CARD_LOCK        BIT12
#define SDC_STATUS_REG_CP_READY         BIT13
#define SDC_STATUS_REG_CP_BUF_READY     BIT14
#define SDC_STATUS_REG_PLAIN_TEXT_READY BIT15
#define SDIO_STATUS_REG_SDIO_INTR       BIT16


#define SDC_POWER_REG_POWER_ON          BIT4        /// bit mapping of power control register
#define SDC_POWER_REG_POWER_BITS        0x0000000F

#define SDC_CLOCK_REG_CLK_DIV           0x0000007F  /// bit mapping of clock control register
#define SDC_CLOCK_REG_CARD_TYPE         0x00000080	//1: SD, 0: MMC
#define SDC_CLOCK_REG_CLK_DIS           0x00000100	
#define SDC_CLOCK_REG_HIGH_SPEED        0x00000200

#define SDC_STATUS_OUT_OF_RANGE         0x80000000 /// The bit mapping of SD Status register
#define SDC_STATUS_ADDRESS_ERROR        0x40000000
#define SDC_STATUS_BLOCK_LEN_ERROR      0x20000000
#define SDC_STATUS_ERASE_SEQ_ERROR      0x10000000
#define SDC_STATUS_ERASE_PARAM          0x08000000
#define SDC_STATUS_WP_VIOLATION         0x04000000
#define SDC_STATUS_CARD_IS_LOCK         0x02000000
#define SDC_STATUS_LOCK_UNLOCK_FAILED   0x01000000
#define SDC_STATUS_COM_CRC_ERROR        0x00800000
#define SDC_STATUS_ILLEGAL_COMMAND      0x00400000
#define SDC_STATUS_CARD_ECC_FAILED      0x00200000
#define SDC_STATUS_CC_ERROR             0x00100000
#define SDC_STATUS_ERROR                0x00080000
#define SDC_STATUS_UNDERRUN             0x00040000
#define SDC_STATUS_OVERRUN              0x00020000
#define SDC_STATUS_CSD_OVERWRITE        0x00010000
#define SDC_STATUS_WP_ERASE_SKIP        0x00008000
#define SDC_STATUS_CARD_ECC_DISABLE     0x00004000
#define SDC_STATUS_ERASE_RESET          0x00002000
#define SDC_STATUS_CURRENT_STATE        0x00001E00
#define SDC_STATUS_READY_FOR_DATA       0x00000100
#define SDC_STATUS_APP_CMD              0x00000020
#define SDC_STATUS_AKE_SEQ_ERROR        0x00000008
#define MMC_STATUS_SWITCH_ERROR         0x00000080

#define SDC_STATUS_ERROR_BITS           (SDC_STATUS_OUT_OF_RANGE | SDC_STATUS_ADDRESS_ERROR | \
                                        SDC_STATUS_BLOCK_LEN_ERROR | SDC_STATUS_ERASE_SEQ_ERROR | \
                                        SDC_STATUS_ERASE_PARAM | SDC_STATUS_WP_VIOLATION | \
                                        SDC_STATUS_LOCK_UNLOCK_FAILED | SDC_STATUS_CARD_ECC_FAILED | \
                                        SDC_STATUS_CC_ERROR | SDC_STATUS_ERROR | \
                                        SDC_STATUS_UNDERRUN | SDC_STATUS_OVERRUN | \
                                        SDC_STATUS_CSD_OVERWRITE | SDC_STATUS_WP_ERASE_SKIP | \
                                        SDC_STATUS_AKE_SEQ_ERROR | MMC_STATUS_SWITCH_ERROR)


/* NANDC: NAND Controller */
#define NANDC_BASE              0x90600000
#define NANDC_STS               0x0100
#define NANDC_ACC               0x0104
#define NANDC_FLOW              0x0108
#define NANDC_ADDR              0x010C
#define NANDC_MEMC              0x0110
#define NANDC_CB_TAR            0x011C
#define NANDC_ID30              0x0120
#define NANDC_INTEN             0x0128
#define NANDC_BI_WR             0x0140
#define NANDC_CRC_WR            0x0148
#define NANDC_LSN_CTL           0x014C
#define NANDC_BI_RD             0x0150
#define NANDC_BMC_PIO_CTL       0x0208
#define NANDC_BMC_BURST         0x020C
#define NANDC_BMC_DATA_PORT     0x0300
#define NANDC_PAGE_CNT          0x0308
#define NANDC_SRST              0x030C

#define NANDC_SRAM_BASE         0x0400
#define NANDC_NANDCSRAM2        0x040C
#define NANDC_SRAM_END          0x0800

#define NANDC_BLANK_CHECK_FAIL  BIT7    /// [0x100] NANDC_STS

#define NANDC_CMD_OFFSET        8       /// [0x104] NANDC_ACC
#define NANDC_CMD_LAUNCH        BIT7

#define NANDC_BS_16P                    0x00
#define NANDC_BS_32P                    BIT16
#define NANDC_BS_64P                    BIT17
#define NANDC_BS_128P                   (BIT16|BIT17)

#define NANDC_AP_3C             0x00	/// [0x110] NANDC_MEMC: Address Phase, 3 cycles
#define NANDC_AP_4C             BIT10
#define NANDC_AP_5C             BIT11

#define NANDC_PS_2K                             BIT8

#define NANDC_MS_32GB                    (BIT7|BIT5|BIT4)

#define NANDC_MOD0_ENABLE       BIT0



#define MANDC_NANDC_SW_RESET    BIT2    /// [0x30C] NANDC_SRST
#define MANDC_BMC_SW_RESET      BIT1
#define MANDC_ECC_SW_RESET      BIT0


/* SCU: System Controller Unit */
#define SCU_BASE                0x98000000
#define SCU_STRAP               0x08
#define	SCU_BOOT_STRAP          (1 << 14)
#define	SCU_STRAP_PAGESIZE      15
#define	SCU_STRAP_ADDRPHASE      17
#define	SCU_STRAP_BLOCKSIZE      19

#define SCU_MEM_SOURCE               0x228
#define SCU_SD_CLK_EN          BIT31        

/* Serial: Serial Contoller*/
#define SERIAL_BASE             0x98100000
#define SERIAL_BASE2            0x98200000

#define FCR_FIFO_EN             0x01        /// Fifo enable 
#define FCR_RXSR                0x02        /// Receiver soft reset
#define FCR_TXSR                0x04        /// Transmitter soft reset

#define MCR_DTR                 0x01
#define MCR_RTS                 0x02

#define LCR_WLS_MSK             0x03        /// character length select mask
#define LCR_WLS_5               0x00        /// 5 bit character length
#define LCR_WLS_6               0x01        /// 6 bit character length
#define LCR_WLS_7               0x02        /// 7 bit character length
#define LCR_WLS_8               0x03        /// 8 bit character length
#define LCR_STB                 0x04        /// Number of stop Bits, off = 1, on = 1.5 or 2)
#define LCR_PEN                 0x08        /// Parity eneble
#define LCR_EPS                 0x10        /// Even Parity Select
#define LCR_STKP                0x20        /// Stick Parity
#define LCR_SBRK                0x40        /// Set Break
#define LCR_BKSE                0x80        /// Bank select enable

#define LSR_DR                  0x01        /// Data ready
#define LSR_OE                  0x02        /// Overrun
#define LSR_PE                  0x04        /// Parity error
#define LSR_FE                  0x08        /// Framing error
#define LSR_BI                  0x10        /// Break
#define LSR_THRE                0x20        /// Xmit holding register empty
#define LSR_TEMT                0x40        /// Xmitter empty
#define LSR_ERR                 0x80        /// Error

/* GPIO: GPIO Controller */
#define GPIO_BASE               0x98600000
#define GPIO_DATAOUT        0x00
#define GPIO_PINDIR             0x08
#define GPIO_DATASET          0x10
#define GPIO_SD_POWER_ON          BIT28        /// bit mapping of power control register


/* INTC: Interrupt Controller */
#define INTC_BASE               0x98800000
#define INTC_IRQ_MASK           0x04
#define INTC_FIQ_MASK           0x24

/* DDR2 Controller*/
#define DDR2_C_BASE             0x98f00000
#define DDR2_A_BASE             0x99000000
#define DDR2_B_BASE             0x99100000

#define DDR2_STATE              0x04

#endif /* __BSP_REG_H */
