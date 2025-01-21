/**
 * @file serial.c
 * @brief
 *
 * This file define serial relative routines   
 *
 * @author Levi Hung
 * @date 2009/10/12
 */
 
#include <bsp_common.h>

#ifdef SERIAL_BASE

#define MODE_X_DIV 16

static NS16550_t com_port = (NS16550_t)SERIAL_BASE;

void serial_init(void)
{
    int baud_divisor = (CONFIG_SERIAL_CLOCK + (CONFIG_SERIAL_BAUDRATE * (MODE_X_DIV / 2)))
                        / (MODE_X_DIV * CONFIG_SERIAL_BAUDRATE);
    com_port->ier = 0x00;
    com_port->lcr = LCR_BKSE | LCRVAL;
    com_port->dll = 0;
    com_port->dlm = 0;
    com_port->lcr = LCRVAL;
    com_port->mcr = MCRVAL;
    com_port->fcr = FCRVAL;
    com_port->lcr = LCR_BKSE | LCRVAL;
    com_port->dll = baud_divisor & 0xff;
    com_port->dlm = ((unsigned int)baud_divisor >> 8) & 0xff;
    com_port->lcr = LCRVAL;
}

void serial_putc(const char c)
{    
    while ((com_port->lsr & LSR_THRE) == 0) {;}
    *(char *) &(com_port->thr) = c;
}

void serial_puts(const char *s)
{
    while (*s) {
        serial_putc (*s++);
    }
}

unsigned int serial_getw(UINT32 timeout)
{
    UINT32 Reg;

    if (timeout) {
        while (timeout-- && ((com_port->lsr & LSR_DR) == 0)) {;}
        Reg = com_port->rbr;
        while (timeout-- && ((com_port->lsr & LSR_DR) == 0)) {;}
        Reg = (com_port->rbr << 8) + Reg;
        while (timeout-- && ((com_port->lsr & LSR_DR) == 0)) {;}
        Reg = (com_port->rbr << 16) + Reg;
        while (timeout-- && ((com_port->lsr & LSR_DR) == 0)) {;}
        Reg = (com_port->rbr << 24) + Reg;
    } else {
        while ((com_port->lsr & LSR_DR) == 0) {;}
        Reg = com_port->rbr;
        while ((com_port->lsr & LSR_DR) == 0) {;}
        Reg = (com_port->rbr << 8) + Reg;
        while ((com_port->lsr & LSR_DR) == 0) {;}
        Reg = (com_port->rbr << 16) + Reg;
        while ((com_port->lsr & LSR_DR) == 0) {;}
        Reg = (com_port->rbr << 24) + Reg;
    }

    return Reg;
}

#endif	/* #ifdef SERIAL_BASE */

