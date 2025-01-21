/**
 * @file start.S
 * @brief Implement 4KByte bootloader with ISP function for SoC
 *
 * @author Levi
 * @date    2009/10/12
 * @note    2009/06/04:	Created by Levi Hung.
 */

#include <bsp/config.h>
#include <bsp_reg.h>
/*
 * interrupt vector, compare with newlib, the bootcode2 remove those vector functions.
 */
.globl _start
_start:	
    b   reset
_undefined_instruction:
    b   _undefined_instruction
_software_interrupt:
    b   _software_interrupt
_prefetch_abort:
    b   _prefetch_abort
_data_abort:
    b   _data_abort
_not_used:
    b   _not_used
_irq:
    b   _irq
_fiq:
    b   _fiq
interrupt_vector_end:
    .balignl 16,0xdeadbeef


/*
 *************************************************************************
 *
 * Startup Code (reset vector)
 *
 * do important init only if we dont start from memory!
 * setup Memory and board specific bits prior to relocation.
 * relocate bootcode2 to ram
 * setup stack
 *
 *************************************************************************
 */

.globl _TEXT_BASE
_TEXT_BASE:
    .word	TEXT_BASE

.globl _bootcode2_start
_bootcode2_start:
    .word _start

/*
 * These are defined in the board-specific linker script.
 */
.globl _bss_start
_bss_start:
    .word __bss_start

.globl _bss_end
_bss_end:
    .word _end

	
/*
 * the actual reset code
 */

reset:
    /* boot start */
    bl	init_cpu

    mov	lr, pc
    ldr	pc, _board_init_nandc

    ldr	r0, =NANDC_BASE				    /* nandc_stack_setup */
    add r0, r0, #NANDC_SRAM_END         /* begining of NANDC SRAM 0x9060_0800  */
    mov	sp, r0						    /* bottom of NANDC SRAM, size = 1KByte */

    mov	lr, pc
    ldr	pc, _board_init_uart            /* serial_init */

    mov	lr, pc
  	ldr	pc, _board_init_nandc2

    ldr	r0, =SCU_BASE + SCU_STRAP       /* if BSP parameter strap */
    ldr	r0, [r0]
    mov	r1, #SCU_BOOT_STRAP
    and	r0, r0, r1
    cmp	r0, #SCU_BOOT_STRAP
    bne	2f

1:
    mov	lr, pc
    ldr	pc, _board_bsp_strap            /* determine if SD card exist */
   	ldr	r3, =NANDC_BASE
    ldr	r4, [r3, #NANDC_NANDCSRAM2]
    tst	r4, #1		
    bne	3f

2:
    mov	lr, pc
    ldr	pc, _board_init_ddr2

3:
    ldr	r3, =AHBC_BASE
    ldr	r4, [r3, #AHBC_ICR]
    tst	r4, #AHBC_REMAP		            /* test REMAP bit */
    bne	4f

    bl	init_ahbc
    
    bl	copy_code

    bl	remap

4:
    bl	disable_irq_fiq                 /* Put interrupts into a known state - off (if possible) */

	bl	copy_vector                     /* copy interrupt vector if necessary */

stack_setup:
    mov	r0, #0x0				
    add r0, r0, #CONFIG_MEMSIZE
    mov	sp, r0						    /* bottom of eRAM   */

clear_bss:
    ldr	r0, _bss_start		            /* find start of bss segment        */
    ldr	r1, _bss_end		            /* stop here                        */
    mov	r2, #0x00000000		            /* clear                            */
clbss_l:str	r2, [r0]		            /* clear loop...                    */
    add	r0, r0, #4
    cmp	r0, r1
    ble	clbss_l
       
    ldr	pc, _start_bootcode2

_board_init_uart:
    .word serial_init

_board_init_nandc:
    .word board_init_nandc

_board_init_nandc2:
    .word board_init_nandc2

_board_bsp_strap:
    .word board_bsp_strap

_read_uart_bsp:
    .word read_uart_bsp

_read_uart_lookuptable:    
    .word read_uart_lookuptable

_read_uart_isp:    
    .word read_uart_isp

_board_init_ddr2:
    .word board_init_ddr2

_start_bootcode2:
    .word start_bootcode2



/*
 *************************************************************************
 *
 * CPU_init_critical registers
 *
 * setup important registers
 * setup memory timing
 *
 *************************************************************************
 */

init_cpu:
#if 0
    /*
     * turn off i-cache, d-cache, MMU, BTB
     */

    /* read cpu id */
    mrc	p15, 0, r0, c0, c0, 0
    ldr	r1, =0xffff
    bic	r1, r0, r1
    ldr	r2, =0xfff0
    and	r2, r0, r2
    ldr	r3, =P15_C0_ID_FARADAY | P15_C0_ID_ARMV4
    ldr	r4, =P15_C0_ID_FA526
    ldr	r5, =P15_C0_ID_FA626
    cmp	r1, r3
    bne	1f
    cmp	r2, r4
    cmpne	r2, r5
    bne	1f

    /* FA526 and FA626 */
    /* unlock ecr */
    mrc	p15, 0, r0, c1, c1, 0
    orr	r0, r0, #0x1
    mcr	p15, 0, r0, c1, c1, 0

    /* turn off i-cache, d-cache, MMU, BTB */
    mrc	p15, 0, r0, c1, c0, 0
    ldr	r1, =P15_C1_ICACHE | P15_C1_DCACHE | P15_C1_MMU | P15_C1_BTB
    bic	r0, r0, r1
    mcr	p15, 0, r0, c1, c0, 0

    /* lock ecr */
    mrc	p15, 0, r0, c1, c1, 0
    bic	r0, r0, #0x1
    mcr	p15, 0, r0, c1, c1, 0

    mov	pc, lr

1:
    /* turn off i-cache, d-cache, MMU */
    mrc	p15, 0, r0, c1, c0, 0
    ldr	r1, =P15_C1_ICACHE | P15_C1_DCACHE | P15_C1_MMU
    bic	r0, r0, r1
    mcr	p15, 0, r0, c1, c0, 0

    mov	pc, lr

#endif

#if 1
    /*
     * flush v4 I/D caches
     */
    mov	r0, #0
    mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
    mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */
	
    /*
     * Disables MMU stuff but turns on I-Cache and Alignment Check
     */
    mrc	p15, 0, r0, c1, c0, 0
    bic	r0, r0, #0x00002300	    /* clear bits 13, 9:8 (--V- --RS) */
    bic	r0, r0, #0x00000087	    /* clear bits 7, 2:0 (B--- -CAM) */
    orr	r0, r0, #0x00000002	    /* set bit 2 (A) Align */
    orr	r0, r0, #0x00001000	    /* set bit 12 (I) I-Cache */
    mcr	p15, 0, r0, c1, c0, 0
	
    mov	pc, lr		/* back to my caller */
#endif 
     
init_ahbc:
    mov	r9, lr

    ldr	r0, =AHBC_BASE

    /* Enable AHB Slave 4 (ROM/SRAM/Flash) */
    ldr	r1, =ROM_DEFAULT | AHBC_SIZE_256M	@ 256MB, base=0x0000 0000
    str	r1, [r0, #AHBC_SLAVE4_BSR]

    /* Enable AHB Slave 6 (SDRAM) */
    ldr	r1, =DDR2_DEFAULT | AHBC_SIZE_512M	@ 2GB,   base=0x2000 0000
    str	r1, [r0, #AHBC_SLAVE6_BSR]

    mov	pc, r9

	
copy_code:
    ldr	r4, =DDR2_DEFAULT   /* copy ROM code to DDR2 SDRAM (currently at 0x2000 0000) */
    ldr	r5, .LC5
    ldr	r6, .LC6

7:
    ldr	r7, [r5], #4
    str	r7, [r4], #4
    cmp	r5, r6 
    /* cmp	r5, r5  @ Test for copy_err*/
    blo	7b

    /* verify the code we copy is correct */
    ldr	r4, =DDR2_DEFAULT
    ldr	r5, .LC5
    ldr	r6, .LC6

2:
    ldr	r0, [r5],#4
    ldr	r1, [r4],#4
    cmp	r0, r1
    bne	_copy_err
    cmp	r5, r6
    blo	2b

    mov	pc, lr

_copy_err:
    b	copy_err

.LC5:
    .word	_start

.LC6:
    .word	_edata
    
    /*
     * This code will remap the memory ROM and SDRAM
     * ROM will be placed on 0x80000000 SDRAM will jump to 0x0
     */
remap:
    ldr	r3, =AHBC_BASE
    ldr	r4, [r3, #AHBC_ICR]
    orr	r4, r4, #AHBC_REMAP		        @ Set REMAP bit
    str	r4, [r3, #AHBC_ICR]

    mov	pc, lr

disable_irq_fiq:
    ldr	r3, =INTC_BASE              	/* disable interrupts */
    ldr	r4, =0x0
    str	r4, [r3, #INTC_IRQ_MASK]
    str	r4, [r3, #INTC_FIQ_MASK]

    mov	pc, lr


	/*
	 * when running in RAM at 0x0000 8000 (or other location)
	 * we should copy interrupt vectors to RAM at zero.
	 */
#define	VECTOR_STUB	0xee010e10
copy_vector:
    mov	r4, #0
    ldr	r5, =_start
    ldr	r6, =interrupt_vector_end
    cmp	r4, r5				@ if interrupt vector is at 0x0
    beq	4f				    @ we dont need to copy
    ldr	r3, =VECTOR_STUB
3:
    ldr	r7, [r4]
    cmp	r7, r3				@ test if it is vector catch or semihosting
    addeq	r4, r4, #4
    addeq	r5, r5, #4
    ldrne	r7, [r5], #4			@ get word and store to RAM
    strne	r7, [r4], #4
    cmp	r5, r6				@ Repeat until all done
    blo	3b
4:
    mov	pc, lr

