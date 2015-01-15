/*--------------------------------------------------------------------------
 * include/asm/arch-sun8iw3p1/eint.h
 *
 * guoyingyang@allwinnertech.com
 *
 * Change Date: 
 *		2014.02.14, create.     
 *                                                                                                                          
 *--------------------------------------------------------------------------*/
#ifndef _EINT_H
#define _EINT_H



#define SUNXI_PIO_EINT_CFG(n)       ((volatile unsigned int *)(SUNXI_PIO_BASE + 0x200 + ((n)-1)*0x20) + 0x00)

#define SUNXI_PIO_EINT_EN(n)               ((volatile unsigned int *)(SUNXI_PIO_BASE + 0x200 + ((n)-1)*0x20 + 0x10))

#define SUNXI_PIO_EINT_STATUS(n)           ((volatile unsigned int *)(SUNXI_PIO_BASE +0x200 + ((n)-1)*0x20 + 0x14))

 #define EINT_TRIGGER_STATUS_POS 0x00
 #define EINT_TRIGGER_STATUS_NEG 0x01
 #define EINT_TRIGGER_STATUS_HIGH_LEV 0x02
 #define EINT_TRIGGER_STATUS_LOW_LEV  0x03
 #define EINT_TRIGGER_STATUS_DOU_EDGE 0x04
 
 
extern  int eint_irq_enable(unsigned int group,unsigned int number);
extern  int eint_irq_disable(unsigned int group, unsigned int number);
#endif
