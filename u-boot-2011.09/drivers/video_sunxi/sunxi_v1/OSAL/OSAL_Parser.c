#include "OSAL_Parser.h"

int OSAL_Script_FetchParser_Data(char *main_name, char *sub_name, int value[], int count)
{
	int ret;
	user_gpio_set_t  gpio_info;
	disp_gpio_set_t  *gpio_list;
	ret = script_parser_fetch(main_name, sub_name, value, count);
	if(ret < 0) {
		//__wrn("fetch script data %s.%s fail\n", main_name, sub_name);
	} else {
		if(count == 1) {
			//__inf("%s.%s = %d\n", main_name, sub_name, *value);
		} else if(count == sizeof(disp_gpio_set_t)) {
			memcpy(&gpio_info, value, sizeof(user_gpio_set_t));
			gpio_list = (disp_gpio_set_t  *)value;
			gpio_list->port = gpio_info.port;
			gpio_list->port_num = gpio_info.port_num;
			gpio_list->drv_level = gpio_info.drv_level;
			gpio_list->pull = gpio_info.pull;
			gpio_list->data = gpio_info.data;
			//__inf("%s.%s gpio_port=%d,gpio_port_num:%d, data:%d\n",main_name, sub_name, gpio_list->port, gpio_list->port_num, gpio_list->data);
		} else {
			//__inf("%s.%s = %s\n", main_name, sub_name, value);
		}
	}
	
	return ret;
}

int OSAL_sw_get_ic_ver(void)
{
    /*
    enum sw_ic_ver ic_ver;
    int ret = 0xA;

    ic_ver = sw_get_ic_ver();
    if(ic_ver == MAGIC_VER_A)
    {
        ret = 0xA;
    }
    else if(ic_ver == MAGIC_VER_B)
    {
        ret = 0xB;
    }
    if(ic_ver == MAGIC_VER_C)
    {
        ret = 0xC;
    }
    
    return ret;
    */
    
    /* todo */
    return 0;
}
