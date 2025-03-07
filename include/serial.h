/**
 * @file serial.h
 * @brief
 *
 * This file define register of serial    
 *
 * @author Levi Hung
 * @date 2009/10/12
 */
 
#ifndef _SERIAL_H
#define _SERIAL_H

#define LCR_8N1         0x03                                /// useful defaults for LCR
#define LCRVAL          LCR_8N1                             /// 8 data, 1 stop, no parity
#define MCRVAL          (MCR_DTR | MCR_RTS)                 /// RTS/DTR
#define FCRVAL          (FCR_FIFO_EN | FCR_RXSR | FCR_TXSR)	/// Clear & enable FIFOs

#define CONFIG_SERIAL_CLOCK         48000000
#define CONFIG_SERIAL_BAUDRATE      115200

#define thr     rbr
#define iir     fcr
#define dll     rbr
#define dlm     ier

struct NS16550 {
    unsigned long rbr;      /// 0 r
    unsigned long ier;      /// 1 rw 
    unsigned long fcr;      /// 2 w  
    unsigned long lcr;      /// 3 rw 
    unsigned long mcr;      /// 4 rw 
    unsigned long lsr;      /// 5 r  
    unsigned long msr;      /// 6 r  
    unsigned long scr;      /// 7 rw 
}; /* No need to pack an already aligned struct */


typedef volatile struct NS16550 *NS16550_t;

extern void serial_putc(const char c);
extern void serial_puts(const char *s);
extern unsigned int serial_getw(UINT32 timeout);

#endif
