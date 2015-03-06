/*
 * (C) Copyright 2007-2015
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

#include <string.h>
#include "types.h"
#include "arch.h"
#include "bsp.h"
#include "boot0_v2.h"


extern const boot0_file_head_t  fes1_head;

typedef struct __fes_aide_info{
    __u32 dram_init_flag;       /* Dram初始化完成标志       */
    __u32 dram_update_flag;     /* Dram 参数是否被修改标志  */
    __u32 dram_paras[SUNXI_DRAM_PARA_MAX];
}fes_aide_info_t;


/*
************************************************************************************
*                          note_dram_log
*
* Description:
*	    ����ɱ�־
* Parameters:
*		void
* Return value:
*    	0: success
*      !0: fail
* History:
*       void
************************************************************************************
*/
static void  note_dram_log(void)
{
    fes_aide_info_t *fes_aide = (fes_aide_info_t *)FES1_DRAM_RET_ADDR;

    memset(fes_aide, 0, sizeof(fes_aide_info_t));
    fes_aide->dram_init_flag    = SYS_PARA_LOG;
    fes_aide->dram_update_flag  = 1;

    memcpy(fes_aide->dram_paras, fes1_head.prvt_head.dram_para, SUNXI_DRAM_PARA_MAX * 4);
    memcpy((void *)DRAM_PARA_STORE_ADDR, fes1_head.prvt_head.dram_para, SUNXI_DRAM_PARA_MAX * 4);
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
#pragma arm section code="fes1_section"

void fes1_entry(void)
{
	cpu_init_s();
	timer_init();

    UART_open( fes1_head.prvt_head.uart_port, (void *)fes1_head.prvt_head.uart_ctrl, 24*1000*1000 );
	UART_printf2("begin init dram\n");
	if(init_DRAM(0, (void *)fes1_head.prvt_head.dram_para))
	{
		note_dram_log();
		UART_printf2("init dram ok\n");
	}
	else
	{
	    UART_printf2("init dram fail\n");
	}
	__msdelay(10);

    return;
}

#pragma arm section



