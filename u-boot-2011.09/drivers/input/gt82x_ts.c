/*--------------------------------------------------------------------------
 * driver/input/gt82x_ts.c
 *
 * 
 *
 * Change Date: 
 *		2013.03.05, create.     
 *                                                                                                                          
 *--------------------------------------------------------------------------*/
#include <common.h>
#include <asm/arch/eint.h>
#include <asm/arch/twi.h>
#include <sys_config.h>
#include <asm/arch/gt82x_ts.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gic.h>

extern int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len);
extern int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len);
extern void i2c_init(int speed, int slaveaddr);

static const int chip_id_value[3] = {0x13,0x27,0x28};
static uchar read_chip_value;
struct ctp_config_info config_info;

static int gpio_wakeup_hdle = 0;
static int gpio_wakeup_enable = 1;
static u32 pio_handle;
static u32  port = 0;
static u32  port_num= 0;

struct info_x_y {
	s32 x[5];
	s32 y[5];	
};
struct info_x_y info_x_y = {
	{0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0}};

static void ctp_wakeup(void);

/*******************************************************
Function:
	write i2c end cmd.	
return value:  
                   = 0; success;
                   < 0; err
*******************************************************/
static int i2c_end_cmd(void)
{
        int ret = -1;
	uchar temp = 0;
        
        ret = i2c_write(0x5d, 0x8000, 2, &temp, 0);
        return ret;
}

/*******************************************************	
Function:
	Read data from the i2c slave device.
Input:.
	addr:operate address.
	buf:read data buffer.
	len:operate length.	
return value:  
                   = 0; success;
                   < 0; err
*********************************************************/
static int i2c_read_bytes(uint addr, uchar *buf, int len)
{
	int ret = -1;
	
	ret = i2c_read(0x5d, addr, 2, buf, len);
	
	return ret;
}

/*******************************************************	
Function:
	write data to the i2c slave device.
Input:
	addr:operate address.
	buf:write data buffer.
	len:operate length.	
return value:  
                   = 0; success;
                   < 0; err
*********************************************************/
static int i2c_write_bytes(uint addr, uchar *buf, int len)
{
	int ret = -1;

	ret = i2c_write(0x5d, addr, 2, buf, len);
	
	return ret;
}

/*******************************************************	
Function:
	read chip id function.
Input:
		
return value:
		= 0; success;
		< 0; err
*********************************************************/
static int goodix_read_id(void)
{
	int ret=-1;

	ctp_wakeup();
		
	ret=i2c_read_bytes(0x0f7d, &read_chip_value, 1);
	if (0 > ret)
		tpdbg("%s: read id failed %d\n", __func__, ret);
			
	tpdbg("chip_id_value:0x%x\n",read_chip_value);

	return ret;
}

/*******************************************************
Function:
	CTP initialize function.
Input:
	nothing.	
return value:
		= 0; success;
		< 0; err
*******************************************************/
static int goodix_init_panel(void)
{
	int ret=-1;
	int i = 0;
        uchar config_info1[112];
        uchar data_info0[] = {
                0x02,0x11,0x03,0x12,0x04,0x13,0x05,0x14,
                0x06,0x15,0x07,0x16,0x08,0x17,0x09,0x18,
                0x0A,0x19,0x0B,0x1A,0xFF,0x15,0x16,0x17,
                0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x03,0x0D,
                0x04,0x0E,0x05,0x0F,0x06,0x10,0x07,0x11,
                0x08,0x12,0xFF,0x0D,0xFF,0x0F,0x10,0x11,
                0x12,0x13,0x0F,0x03,0x10,0x88,0x88,0x20,
                0x00,0x00,0x06,0x00,0x00,0x02,0x50,0x3C,
                0x35,0x03,0x00,0x05,0x00,0x03,0x20,0x05,
                0x00,0x5A,0x5A,0x46,0x46,0x08,0x00,0x03,
                0x19,0x05,0x14,0x10,0x00,0x07,0x00,0x00,
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x01
        };
         uchar data_info1[] = {
                0x00,0x0F,0x01,0x10,0x02,0x11,0x03,0x12,
                0x04,0x13,0x05,0x14,0x06,0x15,0x07,0x16,
                0x08,0x17,0x09,0x18,0x0A,0x19,0x0B,0x1A,
                0x0C,0x1B,0x0D,0x1C,0x0E,0x1D,0x13,0x09,
                0x12,0x08,0x11,0x07,0x10,0x06,0x0F,0x05,
                0x0E,0x04,0x0D,0x03,0x0C,0x02,0x0B,0x01,
                0x0A,0x00,0x0B,0x03,0x10,0x00,0x00,0x2C,
                0x00,0x00,0x03,0x00,0x00,0x02,0x40,0x30,
                0x60,0x03,0x00,0x05,0x00,0x03,0x20,0x05,
                0x00,0x66,0x4E,0x60,0x49,0x06,0x00,0x23,
                0x19,0x05,0x14,0x10,0x03,0xFC,0x01,0x00,
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01
         };
        tpdbg("init panel\n");
	 
	if (read_chip_value == 0x13) {			//813
                ret = i2c_write_bytes(0x0F80, data_info0, 112);
		if(0 > ret)
			tpdbg("write reg failed %d\n", ret);
        } else if(read_chip_value == 0x28) { //828
                ret = i2c_write_bytes(0x0F80, data_info1, 112); 
        }
	i2c_end_cmd();
        udelay(10000);
	tpdbg("init panel ret = %d\n",ret);
	if (ret < 0)
		return ret;
	udelay(100000);
	
	ret = i2c_read_bytes(0x0F80, config_info1, 112);	
	for ( i = 0;i<112;i++) {
	    tpdbg("i = %d config_info1[i] = %x \n",i,config_info1[i]);
	    
	}
	udelay(10000);

	return 0;
}

/*******************************************************
Function:
	Touch down report function.
Input:
	ts:private data.
	id:tracking id.
	x:input x.
	y:input y.
	w:input weight.	
return value:
*******************************************************/
static void goodix_touch_down(s32 id,s32 x,s32 y,s32 w)
{
        tpdbg("source data:ID:%d, X:%d, Y:%d, W:%d\n", id, x, y, w);
        if (1 == config_info.exchange_x_y_flag) {
                swap(x, y);
        }
        if (1 == config_info.revert_x_flag) {
                x = config_info.screen_max_x - x;
        }
        if (1 == config_info.revert_y_flag) {
                y = config_info.screen_max_y - y;
        }

	info_x_y.x[id] = x;
	info_x_y.y[id] = y;
	
        tpdbg("report data:ID:%d, X:%d, Y:%d, W:%d\n", id, x, y, w);
	return;
}

/*******************************************************
Function:
	Touch up report function.
Input:
		
return value:
*******************************************************/
static void goodix_touch_up(void)
{

}

/*******************************************************
Function:
	CTP irq sevice function.
Input:
	irq sevice data.	
return value:
*******************************************************/
static void goodix_ts_irq_hanbler(void)
{
	u8* coor_data = NULL;
	uchar  point_data[2 + 5 * MAX_FINGER_NUM + 1]={0};
	u8  check_sum = 0;
	u8  touch_num = 0;
	u8  finger = 0;
	u8  key_value = 0;
	s32 input_x = 0;
	s32 input_y = 0;
	s32 input_w = 0;
	s32 idx = 0;
	s32 ret = -1;
	//s32 i = 0;

	tpdbg("===enter goodix_ts_irq_hanbler===\n");

	ret = i2c_read_bytes(0x0f40, point_data, 8);
	if (ret < 0) {
	        printf("I2C read error!");
	}
	finger = point_data[0];
	touch_num = (finger & 0x01) + !!(finger & 0x02) + !!(finger & 0x04) + !!(finger & 0x08) + !!(finger & 0x10);
	if (touch_num > 1) {
		uchar buf[23];
		ret = i2c_read_bytes(0x0f48, buf, 5 * (touch_num - 1));
		if (ret < 0){
	        	printf("I2C read error!/n");
		}
		memcpy(&point_data[8], &buf, 5 * (touch_num - 1));
	}
	i2c_end_cmd();

	if (ret < 0) {
	        printf("I2C read error!");
	        goto exit_work_func;
	}

	if((finger & 0xC0) != 0x80) {
	        printf("Data not ready!");
	        goto exit_work_func;
	}

	key_value = point_data[1]&0x0f; // 1, 2, 4, 8
	if ((key_value & 0x0f) == 0x0f) {
	        if (goodix_init_panel()){
	                printf("Reload config failed!\n");
	        }
	        goto exit_work_func;
	}

	coor_data = &point_data[2];
	check_sum = 0;
	for ( idx = 0; idx < 5 * touch_num; idx++) {
	        check_sum += coor_data[idx];
	}
	if (check_sum != coor_data[5 * touch_num]) {
	        printf("Check sum error!");
	        goto exit_work_func;
	}

	if (touch_num) {
	        s32 pos = 0;

	        for (idx = 0; idx < MAX_FINGER_NUM; idx++){
	                if (!(finger & (0x01 << idx))){
	                        continue;
	                }
	                input_x  = coor_data[pos] << 8;
	                input_x |= coor_data[pos + 1];

	                input_y  = coor_data[pos + 2] << 8;
	                input_y |= coor_data[pos + 3];

	                input_w  = coor_data[pos + 4];

	                pos += 5;

	                goodix_touch_down(idx, input_x, input_y, input_w);
	        }
	}else{
	        tpdbg("Touch Release!");
	        goodix_touch_up();
	}
	
exit_work_func:
        return;
}

/* add by guoyingyang */
 void  gpio_handle_irq(void * pio_handle)
 {
	unsigned int  reg_value = 0;
    volatile unsigned int  *reg_addr = NULL;
	
    reg_addr = SUNXI_PIO_EINT_STATUS(port);
	reg_value = *reg_addr;
	printf("===== enter gpio_irq =======\n");
	//匹配gpio中断号，并且调用相应的gpio中断函数
	if(reg_value & (0x01 << port_num))
	{
		goodix_ts_irq_hanbler();
		//清除gpio中断pending位
		reg_value |= ( 0x01 << port_num );
		*reg_addr =  reg_value ;
		printf("clear pending_bit   \n ");
	}
	return ;
 }
/*******************************************************	
Function:
	ctp_wakeup - function.
Input:
		
return value:  
*********************************************************/
static void ctp_wakeup(void)
{
	printf("ctp_wakeup. \n");
	if (1 == gpio_wakeup_enable) {  
		if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_wakeup_hdle, 0, "ctp_wakeup")){
			printf("err when operate gpio. \n");
		}
		udelay(2000);
		if(EGPIO_SUCCESS != gpio_write_one_pin_value(gpio_wakeup_hdle, 1, "ctp_wakeup")){
			printf("err when operate gpio. \n");
		}
		udelay(20000);

	}
	return;
}

/*******************************************************
Function:
	ctp_free_platform_irq.
Input:
 
return value:
*******************************************************/
static void ctp_free_platform_irq(void)
{
	eint_irq_disable(port,port_num);
	
	if (pio_handle) {
		gpio_release(pio_handle, 2);
	}

	return;
}

/*******************************************************
Function:
	ctp_init_platform_irq.
Input:
 
return value:
		= 0; success;
		< 0; err
*******************************************************/
static int ctp_init_platform_irq(void)
{
	int ret = -1;
        user_gpio_set_t *tmp = NULL;
	
	pio_handle = gpio_request_ex("ctp_para", "ctp_int_port");
	if (!pio_handle) {
		printf("[%s] gpio_request_ex fail! %d\n",__func__,__LINE__);
		return ret;
	}
    tmp = (user_gpio_set_t *)((char *)pio_handle + 16);
	//ret = eint_irq_request(0, 3, goodix_ts_irq_hanbler, EINT_TRIGGER_STATUS_NEG, &test_a);
	//if (ret)
	//	printf("request gpio irq err!\n");
    irq_install_handler(AW_IRQ_EINTB, (interrupt_handler_t *)gpio_handle_irq, (void*)&pio_handle);
	
	port = tmp->port;
	port_num = tmp->port_num;
	
	ret = eint_irq_enable(port,port_num);
	if (ret)
		printf("enable irq err!\n");

	return ret;
}

/*******************************************************
Function:
	ctp_free_platform_resource.
Input:
 
return value:
*******************************************************/
static void ctp_free_platform_resource(void)
{	
	if (gpio_wakeup_hdle) {
		gpio_release(gpio_wakeup_hdle, 2);
	}

	return;
}

/*******************************************************
Function:
	initialize platform related resource.
Input:
 
return value:
		= 0; success;
		< 0; err
*******************************************************/
static int ctp_init_platform_resource(void)
{
	int ret = 0;
	
	gpio_wakeup_hdle = gpio_request_ex("ctp_para", "ctp_wakeup");
	if (!gpio_wakeup_hdle) {
		tpdbg("tp_wakeup request gpio fail!\n");
		gpio_wakeup_enable = 0;
	}

	return ret;
}

/*******************************************************
Function:
	CTP print config info function.
Input:
	CTP config info	
return value:
*******************************************************/
void ctp_print_info(struct ctp_config_info info)
{
	tpdbg("info.ctp_used:%d\n",info.ctp_used);
	tpdbg("info.twi_id:%d\n",info.twi_id);
	tpdbg("info.screen_max_x:%d\n",info.screen_max_x);
	tpdbg("info.screen_max_y:%d\n",info.screen_max_y);
	tpdbg("info.revert_x_flag:%d\n",info.revert_x_flag);
	tpdbg("info.revert_y_flag:%d\n",info.revert_y_flag);
	tpdbg("info.exchange_x_y_flag:%d\n",info.exchange_x_y_flag);
	tpdbg("info.irq_gpio_number:%d\n",info.irq_gpio_number);
	tpdbg("info.wakeup_gpio_number:%d\n",info.wakeup_gpio_number);
}

/*******************************************************
Function:
	get config info from sysconfig.fex file.
Input:
 
return value:
		= 0; success;
		< 0; err
*******************************************************/
static int ctp_fetch_sysconfig_para(void)
{
	int ret = -1;

	if (SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_used", &config_info.ctp_used, 1)) {
		tpdbg("ctp_used script_get_item  err. \n");
		goto script_get_item_err;
	}
	
	if(1 != config_info.ctp_used){
		tpdbg("ctp_unused. \n");
		return ret;
	}
	
	if (SCRIPT_PARSER_OK != script_parser_fetch ("ctp_para", "ctp_twi_id", 
		&config_info.twi_id, 1)){
		tpdbg("ctp_twi_id script_get_item err. \n");
		goto script_get_item_err;
	}
	
	if (SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_screen_max_x", &config_info.screen_max_x, 1)) {
		tpdbg("ctp_screen_max_x script_get_item err. \n");
		goto script_get_item_err;
	}
	
        if (SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_screen_max_y", &config_info.screen_max_y, 1)) {
        		tpdbg("ctp_screen_max_y script_get_item err. \n");
        		goto script_get_item_err;
        }
        
        if (SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_revert_x_flag", &config_info.revert_x_flag, 1)) {
                tpdbg("ctp_revert_x_flag script_get_item err. \n");
                goto script_get_item_err;
        }
        
        if (SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_revert_y_flag", &config_info.revert_y_flag, 1)) {
                tpdbg("ctp_revert_y_flag script_get_item err. \n");
                goto script_get_item_err;
        }
        
        if (SCRIPT_PARSER_OK != script_parser_fetch("ctp_para", "ctp_exchange_x_y_flag", &config_info.exchange_x_y_flag, 1)) {
                tpdbg("ctp_exchange_x_y_flag script_get_item err. \n");
                goto script_get_item_err;
        }

	ctp_print_info(config_info);
	return 0;

script_get_item_err:
	tpdbg("=========script_get_item_err============\n");
	return ret;
}

/*******************************************************
Function:
	goodix ctp load function.
Input:
 
return value:
		= 0; success;
		< 0; err
*******************************************************/
int goodix_init(void)
{
	int err = -1;
	i2c_init(40000,0);
	printf("%s.enter\n", __func__);
	if (ctp_fetch_sysconfig_para()) {
		printf("ctp_fetch_sysconfig_para err.\n");
		return -1;
	} 

	err = ctp_init_platform_resource();
	if (0 != err) {
	    printf("ctp_init_platform_resource err. \n");    
	}
		
	goodix_read_id();
	
	goodix_init_panel();

	err = ctp_init_platform_irq();
	if (0 != err) {
	    printf("ctp_init_platform_irq err. \n");    
	}
	printf("%s.over\n", __func__);
	return err;
}

/*******************************************************
Function:
	goodix ctp remove function.
Input:
 
return value:
*******************************************************/
int goodix_remove(void)
{
	ctp_free_platform_resource();
	ctp_free_platform_irq();
	
	return 0;
}

