/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <asm/io.h>

//#define __CPUX_DEBUG__   1

#ifdef  __CPUX_DEBUG__

#define cpux_uart_putc(c)																\
		do																				\
		{																				\
			while( ((*(volatile unsigned int *)(0x1c28000 + 0x14)) & (1<<6)) == 0);		\
			*(volatile unsigned int *)(0x1c28000 + 0x00) = (c);							\
		}																				\
		while(0)
#else
#define cpux_uart_putc(c)																\
		do																				\
		{																				\
		}																				\
		while(0)
#endif

//------------------------------------------------------------------------------
//return value defines
//------------------------------------------------------------------------------
#define	OK					(0)
#define	FAIL					(-1)
#define TRUE					(1)
#define	FALSE					(0)
#define STATUS_REG_PA 				(0x01f00100)
#define RESUME1_START				(0x7000)

#define CPU_RESET_DEASSERT 			(0x3)
#define CPU_RESET_MASK 				(0x3)
#define	CPUCFG_CPU0        			(0)
#define	CPUCFG_CPU1        			(1)
#define	CPUCFG_CPU2        			(2)
#define	CPUCFG_CPU3        			(3)
#define CPUCFG_CPU_NUMBER			(CPUCFG_CPU3 + 1)
#define CPUX_STARTUP_ADDR			(0x0)

#define CPUX_INVALIDATION_DONE_FLAG		(0xff)
#define CPUX_INVALIDATION_DONE_FLAG_REG(n) 	(STATUS_REG_PA + 0x04 + (n)*0x4)
#define CPUX_INVALIDATION_COMPLETION_FLAG_REG 	(CPUX_INVALIDATION_DONE_FLAG_REG(CPUCFG_CPU_NUMBER))

/*
 * CPUCFG
 */
#define AW_R_CPUCFG_BASE			(0x1f01c00)
#define AW_R_PRCM_BASE          		(0x1f01400)
#define AW_CPU_PWROFF_REG       		(0x100)
#define AW_CPUCFG_P_REG0            		0x01a4
#define AW_CPUCFG_P_REG1            		0x01a8
#define CPUX_RESET_CTL(x) 			(0x40 + (x)*0x40)
#define CPUX_CONTROL(x)   			(0x44 + (x)*0x40)
#define CPUX_STATUS(x)    			(0x48 + (x)*0x40)
#define AW_CPUCFG_GENCTL            		0x0184
#define AW_CPUCFG_DBGCTL0           		0x01e0
#define AW_CPUCFG_DBGCTL1           		0x01e4

#define IS_WFI_MODE(cpu)			(*(volatile unsigned int *)((((0x01f01c00)) + (0x48 + (cpu)*0x40))) & (1<<2))
#define CPUX_RUN_ADDR   			(0x01f01c00 + 0x1A4)
#define CPUX_RESET_ADDR(cpu)			(0x01f01c00 + 0x40 + (cpu) * 0x40)
#define R_CPU_POWOFF_REG			(0x01f01400 + 0x100)

#define AW_CPUX_PWR_CLAMP(cpu)  		(0x140 + (cpu) * 0x04)
#define AW_CPUX_PWR_CLAMP_STATUS(cpu)   	(0x64 + (cpu)*0x40)

//function declartion
__s32 cpucfg_set_cpu_reset_state(__u32 cpu_num, __s32 state);
void super_enable_aw_cpu(int cpu);
extern void cpux_s_start(void);

static  void cpux_delay(int ms)
{
	volatile int i;

	for(i=0;i<ms * 1000;i++);
}

static void save_mem_status_nommu(volatile __u32 val)
{
	*(volatile __u32 *)(STATUS_REG_PA  + 0x04) = val;
	return;
}

static void save_cpux_mem_status_nommu(volatile __u32 val)
{
	*(volatile __u32 *)(STATUS_REG_PA  + 0x00) = val;
	return;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
void cpux_c_start(void)
{
	__u32 cpu_id = 0;
	__u32 mva_addr = 0x00000000;

	cpux_uart_putc('a');
	asm volatile ("mrc p15, 0, %0, c0, c0, 5" : "=r"(cpu_id)); //Read CPU ID register
	cpu_id &= 0x3;
	/* execute a TLBIMVAIS operation to addr: 0x0000,0000 */
	asm volatile ("mcr p15, 0, %0, c8, c3, 1" : : "r"(mva_addr));
	asm volatile ("dsb");
	//printk_nommu("cpu_id = %x. \n", cpu_id);
	//set invalidation done flag
	writel(CPUX_INVALIDATION_DONE_FLAG, CPUX_INVALIDATION_DONE_FLAG_REG(cpu_id));
	//dsb
	asm volatile ("dsb");
	save_cpux_mem_status_nommu(RESUME1_START |0x44 | (cpu_id<<8));
	asm volatile ("sev");

	//just waiting until the completion flag be seted..
	while(1)
	{
		/* step: execute a WFE instruction  */
		cpux_uart_putc('b');
		asm volatile ("wfe");
		save_cpux_mem_status_nommu(RESUME1_START |0x45 | (cpu_id<<8));
		if(CPUX_INVALIDATION_DONE_FLAG == readl(CPUX_INVALIDATION_COMPLETION_FLAG_REG)){
			break;
		}
	}
	save_cpux_mem_status_nommu(RESUME1_START |0x46 | (cpu_id<<8));
	//normal power down sequence.
	while(1)
	{
		//let the cpu1+ enter wfi state;
		/* step3: execute a CLREX instruction */
		asm("clrex" : : : "memory", "cc");

		/* step5: execute an ISB instruction */
		asm volatile ("isb");
		/* step6: execute a DSB instruction  */
		asm volatile ("dsb");

		save_cpux_mem_status_nommu(RESUME1_START |0x47 | (cpu_id<<8));
		cpux_uart_putc('c');
		/* step7: execute a WFI instruction */
		while(1)
		{
			cpux_uart_putc('d');
			asm("wfi" : : : "memory", "cc");
		}

	}

	return ;
}

/*
*********************************************************************************************************
*				       SET CPU RESET STATE
*
* Description:	set the reset state of cpu.
*
* Arguments  :	cpu_num : the cpu id which we want to set reset status.
*				state	: the reset state which we want to set.
*
* Returns    :	OK if get power status succeeded, others if failed.
*********************************************************************************************************
*/
__s32 cpucfg_set_cpu_reset_state(__u32 cpu_num, __s32 state)
{
	volatile __u32 value;

	//set cpu state
	value  = readl(AW_R_CPUCFG_BASE + CPUX_RESET_CTL(cpu_num));
	value &= ~(CPU_RESET_MASK);
	value |= state;
	writel(value, AW_R_CPUCFG_BASE + CPUX_RESET_CTL(cpu_num));

	return OK;
}

void super_enable_aw_cpu(int cpu)
{
	__u32 pwr_reg;

	//设置每个cpu的地址
	writel((uint)cpux_s_start, CPUX_RUN_ADDR);

	/* step1: Assert nCOREPORESET LOW and hold L1RSTDISABLE LOW.
	      Ensure DBGPWRDUP is held LOW to prevent any external
	      debug access to the processor.
	*/
	/* assert cpu core reset */
	writel(0, (AW_R_CPUCFG_BASE) + CPUX_RESET_CTL(cpu));
	/* L1RSTDISABLE hold low */
	pwr_reg = readl((AW_R_CPUCFG_BASE) + AW_CPUCFG_GENCTL);
	pwr_reg &= ~(1<<cpu);
	writel(pwr_reg, (AW_R_CPUCFG_BASE) + AW_CPUCFG_GENCTL);

	/* step2: release power clamp */
	//write bit3, bit4 to 0
	writel(0xe7, (AW_R_PRCM_BASE) + AW_CPUX_PWR_CLAMP(cpu));
	while((0xe7) != readl((AW_R_CPUCFG_BASE) + AW_CPUX_PWR_CLAMP_STATUS(cpu)))
	    ;
	//write 012567 bit to 0
	writel(0x00, (AW_R_PRCM_BASE) + AW_CPUX_PWR_CLAMP(cpu));
	while((0x00) != readl((AW_R_CPUCFG_BASE) + AW_CPUX_PWR_CLAMP_STATUS(cpu)))
	    ;
	cpux_delay(2);

	/* step3: clear power-off gating */
	pwr_reg = readl((AW_R_PRCM_BASE) + AW_CPU_PWROFF_REG);
	pwr_reg &= ~(0x00000001<<cpu);
	writel(pwr_reg, (AW_R_PRCM_BASE) + AW_CPU_PWROFF_REG);
	cpux_delay(1);

	/* step4: de-assert core reset */
	writel(3, (AW_R_CPUCFG_BASE) + CPUX_RESET_CTL(cpu));

}

int cpu0_set_detected_paras(void)
{
	volatile __u32  pwr_reg;

	//step2: clear completion flag.
	writel(0, CPUX_INVALIDATION_COMPLETION_FLAG_REG);
	//step3: clear completion done flag for each cpux
	writel(0, CPUX_INVALIDATION_DONE_FLAG_REG(CPUCFG_CPU1));
	writel(0, CPUX_INVALIDATION_DONE_FLAG_REG(CPUCFG_CPU2));
	writel(0, CPUX_INVALIDATION_DONE_FLAG_REG(CPUCFG_CPU3));

	//step4: dsb
	asm volatile ("dsb");

	//step5: power up other cpus.
	super_enable_aw_cpu(CPUCFG_CPU1);
	super_enable_aw_cpu(CPUCFG_CPU2);
	super_enable_aw_cpu(CPUCFG_CPU3);

	//step7: check cpux's invalidation done flag.
	while(1)
	{
		cpux_uart_putc('h');
		//step6 or 8: wfe
		asm volatile ("wfe");
		save_mem_status_nommu(RESUME1_START |0x35);

		if(CPUX_INVALIDATION_DONE_FLAG == readl(CPUX_INVALIDATION_DONE_FLAG_REG(CPUCFG_CPU1)) && \
				CPUX_INVALIDATION_DONE_FLAG == readl(CPUX_INVALIDATION_DONE_FLAG_REG(CPUCFG_CPU2)) && \
				CPUX_INVALIDATION_DONE_FLAG == readl(CPUX_INVALIDATION_DONE_FLAG_REG(CPUCFG_CPU3)) )
		{
					cpux_uart_putc('i');
					//step9: set completion flag.
					writel(CPUX_INVALIDATION_DONE_FLAG, CPUX_INVALIDATION_COMPLETION_FLAG_REG);

					//step10: dsb
					asm volatile ("dsb");
					//sev
					asm volatile ("sev");
					break;
		}
	}
	save_mem_status_nommu(RESUME1_START |0x39);


	//step 11: normal power up.
	while(1)
	{
		cpux_uart_putc('j');
		if((IS_WFI_MODE(1) && IS_WFI_MODE(2) && IS_WFI_MODE(3)))
		{
			cpux_uart_putc('k');
			save_mem_status_nommu(RESUME1_START |0x3a);
			/* step9: set up cpu1+ power-off signal */
			//printk_nommu("set up cpu1+ power-off signal.\n");
			pwr_reg = (*(volatile __u32 *)((AW_R_PRCM_BASE) + AW_CPU_PWROFF_REG));
			pwr_reg |= (0xe); //0b1110
			(*(volatile __u32 *)((AW_R_PRCM_BASE) + AW_CPU_PWROFF_REG)) = pwr_reg;
			cpux_delay(1);

			save_mem_status_nommu(RESUME1_START |0x3b);
			/* step10: active the power output clamp */
			//printk_nommu("active the power output clamp.\n");
			(*(volatile __u32 *)((AW_R_PRCM_BASE) + AW_CPUX_PWR_CLAMP(1))) = 0xff;
			(*(volatile __u32 *)((AW_R_PRCM_BASE) + AW_CPUX_PWR_CLAMP(2))) = 0xff;
			(*(volatile __u32 *)((AW_R_PRCM_BASE) + AW_CPUX_PWR_CLAMP(3))) = 0xff;


			break;
		}

		save_mem_status_nommu(RESUME1_START |0x3c);
	}

	return 0;
}

