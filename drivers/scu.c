/**
 * @file scu.c
 * @brief
 *
 * This file defines system clock unit.
 *
 * @author Levi Hung
 * @date 2009/10/09
 */
 
#include <bsp_common.h>

UINT32 u32ReadCPUCLK(void)
{
    unsigned int cpu_fclk_sel, hclk_pll2_sel, cpuclk;
    
    cpu_fclk_sel  = (REG32(SCU_BASE + SCU_STRAP) >> 3) & 0x3;
    hclk_pll2_sel = (REG32(SCU_BASE + SCU_STRAP) >> 5) & 0x3;

    if (cpu_fclk_sel == 2) {
        cpuclk = 133;
    } else if (cpu_fclk_sel == 1) {
        cpuclk = 266;
    } else {
        cpuclk = 533;        
    }       
 
    if (hclk_pll2_sel == 2) {
        cpuclk = cpuclk >> 1;
    }
    
    return cpuclk;
}
