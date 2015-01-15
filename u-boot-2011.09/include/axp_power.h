/*===============================================================
*   Copyright (C) 2013 AllWinner Technology. All Rights Reserved.
*   
*   file name£ºaxp_power.h
*   author£ºMartin Zheng <zhengjiewen@allwinnertech.com>
*   date£º2013Äê03ÔÂ28ÈÕ
*   brief£º
*
*   version£º1.0.0
================================================================*/

#ifndef _AXP_POWER_H_
#define _AXP_POWER_H_

typedef enum
{
    AXP_BUS_TYPE_I2C,
    AXP_BUS_TYPE_P2WI,
    AXP_BUS_TYPE_RSB,
} power_bus_type;

typedef enum AXP_POWER_ID
{
	AXP_POWER_ID_AXP152,
	AXP_POWER_ID_AXP209,
	AXP_POWER_ID_AXP221,
	AXP_POWER_ID_AXP221S,
	AXP_POWER_ID_AXP229,	
	AXP_POWER_ID_NULL
}axp_power_id_t;

#define AXP_POWER_INSTANCE_MAX (AXP_POWER_ID_NULL+1)


typedef enum AXP_CMD
{
    //DCDC operation
    AXP_CMD_SET_DCDC1_VOLTAGE,
    AXP_CMD_GET_DCDC1_VOLTAGE,
    AXP_CMD_SET_DCDC2_VOLTAGE,
    AXP_CMD_GET_DCDC2_VOLTAGE,
    AXP_CMD_SET_DCDC3_VOLTAGE,
    AXP_CMD_GET_DCDC3_VOLTAGE,
    AXP_CMD_SET_DCDC4_VOLTAGE,
    AXP_CMD_GET_DCDC4_VOLTAGE,
    AXP_CMD_SET_DCDC5_VOLTAGE,
    AXP_CMD_GET_DCDC5_VOLTAGE,
    AXP_CMD_SET_CPU_CORE_VOLTAGE,

    //LDO operation
    AXP_CMD_SET_LDO1_VOLTAGE,
    AXP_CMD_GET_LDO1_VOLTAGE,
    AXP_CMD_SET_LDO2_VOLTAGE,
    AXP_CMD_GET_LDO2_VOLTAGE,
    AXP_CMD_SET_LDO3_VOLTAGE,
    AXP_CMD_GET_LDO3_VOLTAGE,
    AXP_CMD_SET_LDO4_VOLTAGE,
    AXP_CMD_GET_LDO4_VOLTAGE,
    AXP_CMD_SET_LDO5_VOLTAGE,
    AXP_CMD_GET_LDO5_VOLTAGE,
    AXP_CMD_SET_LDO6_VOLTAGE,
    AXP_CMD_GET_LDO6_VOLTAGE,
    AXP_CMD_SET_LDO7_VOLTAGE,
    AXP_CMD_GET_LDO7_VOLTAGE,
    AXP_CMD_SET_LDO8_VOLTAGE,
    AXP_CMD_GET_LDO8_VOLTAGE,
    AXP_CMD_SET_LDO9_VOLTAGE,
    AXP_CMD_GET_LDO9_VOLTAGE,

    //power up and down votage set
    AXP_CMD_SET_POWER_UP_VOLTAGE,
    AXP_CMD_SET_POWER_OFF_VOLTAGE,
    AXP_CMD_GET_POWER_LEVEL,
    
    //BATTERY INFO
    AXP_CMD_GET_BATTERY_EXISTENCE,
    AXP_CMD_GET_BATTERY_CAPACITY,
    AXP_CMD_GET_BATTERY_REST_VOLUME,
    AXP_CMD_GET_BATTERY_OCV,
    AXP_CMD_GET_BATTERY_RDC,
    AXP_CMD_SET_BATTERY_RDC,

    //charger info and operation
    AXP_CMD_SET_CHARGER_ENABLE,
    AXP_CMD_SET_CHARGER_DISABLE,
    AXP_CMD_GET_CHARGER_IS_CHARGING,
    AXP_CMD_GET_CHARGER_CHARGE_CURRENT,
    AXP_CMD_SET_CHARGER_CHARGE_CURRENT,

    //usb vbus operation
    AXP_CMD_GET_VBUS_EXISTENCE,
    AXP_CMD_GET_VBUS_CURRENT,
    AXP_CMD_GET_VBUS_VOLTAGE,
    AXP_CMD_SET_VBUS_LIMIT_PC,
    AXP_CMD_SET_VBUS_LIMIT_DC,
    //AC bus info
    AXP_CMD_GET_AC_EXISTENCE,
    AXP_CMD_GET_AC_CURRENT,
    AXP_CMD_GET_AC_VOLTAGE,
    
    //AXP buffer operation
    AXP_CMD_CLEAR_ALL_BUFFER,
    AXP_CMD_SET_NEXT_MODE,
    AXP_CMD_GET_PRE_MODE,

    //AXP power key operation
    AXP_CMD_GET_POWER_KEY_STATUS,
    AXP_CMD_CLEAR_POWER_KEY_STATUS,
    AXP_CMD_GET_POWER_UP_TRIGGER,
    
    //AXP gpio write
    AXP_CMD_GPIO_REQUEST,
    AXP_CMD_GPIO_IO_CTL,
    AXP_CMD_GPIO_GET_VALUE,
    AXP_CMD_GPIO_SET_VALUE,

    //interrupt
    AXP_CMD_INT_ENABLE,
    AXP_CMD_INT_DISABLE,
    AXP_CMD_INT_GET_STATUS,
    
    //misc
    AXP_CMD_POWER_OFF,
    AXP_CMD_GET_BUS_ID,
    AXP_CMD_GET_BUS_TYPE,
    AXP_CMD_GET_IRQ_NO,
    AXP_CMD_GET_NAME,
} axp_cmd_t;

typedef enum
{
    AXP_POWER_SYS_MODE  = 0x0e,
    AXP_POWER_BOOT_MODE = 0x0f,
}axp_power_mode_t;

typedef enum
{
    AXP_POWER_KEY_MASK_PRESS_LONG = 0x00000001,
    AXP_POWER_KEY_MASK_PRESS_SHORT= 0x00000002,
}axp_power_key_press_t;

typedef enum
{
    AXP_POWER_UP_TRIGGER_BY_KEY =0x00,
    AXP_POWER_UP_TRIGGER_BY_POWER =0x01,
}axp_power_up_triger_t;

typedef enum
{
    BATTERY_RATIO_DEFAULT,
    BATTERY_RATIO_TOO_LOW_WITHOUT_DCIN,
    BATTERY_RATIO_TOO_LOW_WITH_DCIN,
    BATTERY_RATIO_ENOUGH,
}axp_power_level_t;

typedef enum
{
    AXP_INT_MASK_AC_REMOVE          = 0x00000001,
    AXP_INT_MASK_AC_INSERT          = 0x00000002,
    AXP_INT_MASK_VBUS_REMOVE        = 0x00000004,
    AXP_INT_MASK_VBUS_INSERT        = 0x00000008,
    AXP_INT_MASK_CHARGE_DONE        = 0x00000010,
    AXP_INT_MASK_LONG_KEY_PRESS     = 0x00000020,
    AXP_INT_MASK_SHORT_KEY_PRESS    = 0x00000040,
}axp_power_int_mask_t;

struct axp_power
{
	/* the axp power name information */
	char  name[32];

    power_bus_type bus_type; //IIC, P2WI,RSB

    int   bus_id; 
    
    int   irq_no;
    
	/* initialize the axp power */
	int (*init)(struct axp_power *axp_power_instance);

	/* ioctl the axp power */
	int (*ioctl)(struct axp_power *axp_power_instance, int cmd, void * arg);

	/* exit the axp power */
	int (*exit)(struct axp_power *axp_power_instance);

	/* private data */
	void * priv;
};



#define axp_power_register(axp_name,axp_bus_type,axp_irq_no,                         \
                 axp_bus_id,axp_init,axp_ioctl,axp_exit,axp_priv)      				 \
struct axp_power axp_power_instance_##axp_name			=             				 \
{                                                                     				 \
    .name     = #axp_name,                                             				 \
    .bus_type = axp_bus_type,                                         				 \
    .irq_no   = axp_irq_no,                                          				 \
    .bus_id   = axp_bus_id,                                          				 \
    .init     = axp_init,                                            				 \
    .ioctl    = axp_ioctl,                                           				 \
    .exit     = axp_exit,                                             				 \
    .priv     = axp_priv                                              				 \
}                                                                

extern int axp_power_init(void);
extern int axp_power_ioctl(int cmd,void *arg);
extern int axp_power_exit(void);
extern inline int pm_bus_read(unsigned  char  chip, unsigned char addr, unsigned char* data);
extern inline int pm_bus_write(unsigned char  chip, unsigned char addr, unsigned char data);
#endif
