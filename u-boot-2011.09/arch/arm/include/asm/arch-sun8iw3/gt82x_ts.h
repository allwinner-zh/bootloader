/*--------------------------------------------------------------------------
 * arch/arm/include/asm/arch-sunxi/gt82x_ts.h
 *
 * 
 *
 * Change Date: 
 *		2013.03.05, create.     
 *                                                                                                                          
 *--------------------------------------------------------------------------*/
#ifndef _GT82X_H
#define _GT82X_H

#define MAX_FINGER_NUM 5

#define TP_DEBUG

#ifdef TP_DEBUG
#define tpinfo(fmt...)	printf("[gt82x]: "fmt)
#define tpdbg(fmt...)	printf("[gt82x]: "fmt)
#define tpmsg(fmt...)	printf(fmt)
#else
#define tpinfo(fmt...)
#define tpdbg(fmt...)
#define tpmsg(fmt...)
#endif

/*
 * swap - swap value of @a and @b
 */
#define swap(a, b) \
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

struct ctp_config_info{
        int ctp_used;
        int twi_id;
        int screen_max_x;
        int screen_max_y;
        int revert_x_flag;
        int revert_y_flag;
        int exchange_x_y_flag;
        u32 irq_gpio_number;
        u32 wakeup_gpio_number; 
#ifdef TOUCH_KEY_LIGHT_SUPPORT 
        u32 key_light_gpio_number;
#endif             
};

extern int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len);
extern int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len);
extern int eint_irq_enable(unsigned int group,unsigned int number);
extern int goodix_init(void);
//extern int eint_irq_request(unsigned int group, unsigned int number, interrupt_handler_t handle_irq,
	//	enum EINT_TRIGGER_STATUS trigger_status,void *data);
//extern void eint_irq_free(unsigned int group,unsigned int number);
#endif

