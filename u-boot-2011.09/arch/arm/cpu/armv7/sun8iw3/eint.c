/*--------------------------------------------------------------------------
 * arch/arm/cpu/armv7/sun8iw3/eint.c
 *
 * guoyingyang@allwinnertech.com
 *
 * Change Date: 
 *		2014.02.14, create.     
 *                                                                                                                          
 *--------------------------------------------------------------------------*/
#include <asm/arch/gic.h>
#include <asm/arch/eint.h>
#include <asm/arch/cpu.h> 

//group = 1  GPIO_A
//group = 2  GPIO_B   and so on
 int eint_irq_enable(unsigned int group,unsigned int number)
 {
	u32 reg_value = 0;
    volatile u32 * reg_addr = NULL;
	//先将gpio的外部中断寄存器配置好
	//gpio_enable(1,5);    
	//sun8iw3p1 
	//ctp_int_port        = port:PB05<4><default><default><default>
	//ctp_wakeup          = port:PH01<1><default><default><1>
//	reg_value = *((volatile unsigned int *)( SUNXI_EINT_GPIO_B));
    if(( group == 1) || (group ==2) || (group ==7)) 
    {
		if(group == 7)
		{
			group = 3;  //GPIO G 
		}

		reg_addr = SUNXI_PIO_EINT_EN(group);
		reg_value = *reg_addr;
		reg_value |= 0x01<< number;
		*reg_addr = reg_value;
	//	printf("====SUNXI_EINT_GPIO_B reg 0X%x====\n",*((volatile unsigned int *)( SUNXI_EINT_GPIO_B)));
		
		//中断触发方式,下降沿触发
		reg_addr = SUNXI_PIO_EINT_CFG(group);
		reg_value = *reg_addr;
		reg_value |= (0x01 << (4*number));
		*reg_addr = reg_value;
	//	*((volatile unsigned int *)( SUNXI_EINT_GPIO_B_INT_CFG)) = reg_value ;
		
		//再将gic控制器对应的gpio eint位enable
		switch(group)
		{
			case 1:
				irq_enable(AW_IRQ_EINTA);
				printf("enable EINT_A   \n");
				break;
			case 2:
				irq_enable(AW_IRQ_EINTB);
				printf("enable EINT_B   \n");
				break;
			case 3:
				irq_enable(AW_IRQ_EINTG);
				printf("enable EINT_G   \n");
				break;
			default:
				break;
			}
		return 0;
    }
    else
    {
        printf("eint_irq_err:group does not have eint character \n");
        return -1;
    }
 }
 
 int eint_irq_disable(unsigned int group, unsigned int number)
 {
	u32 reg_value = 0;
    volatile u32 * reg_addr = NULL;
    if(( group == 1) || (group ==2) || (group ==7)) 
    {
		if(group == 7)
		{
			group = 3;  //GPIO G 
		}

		reg_addr = SUNXI_PIO_EINT_EN(group);
		reg_value = *reg_addr;
		reg_value &= ~(0x01<< number);
		*reg_addr = reg_value;
		
		reg_addr = SUNXI_PIO_EINT_CFG(group);
		reg_value = *reg_addr;
		reg_value &= ~(0x0f << (4*number));
		*reg_addr = reg_value;
	//	*((volatile unsigned int *)( SUNXI_EINT_GPIO_B_INT_CFG)) = reg_value ;
		
		//再将gic控制器对应的gpio eint位enable
		switch(group)
		{
			case 1:
				irq_disable(AW_IRQ_EINTA);
				printf("enable EINT_A   \n");
				break;
			case 2:
				irq_disable(AW_IRQ_EINTB);
				printf("enable EINT_B   \n");
				break;
			case 3:
				irq_disable(AW_IRQ_EINTG);
				printf("enable EINT_G   \n");
				break;
			default:
				break;
			}
		return 0;
    }
    else
    {
        printf("eint_irq_disable_err:group does not have eint character \n");
        return -1;
    }

 }
