
#include "disp_scaler.h"
#include "disp_display.h"
#include "disp_event.h"
#include "disp_layer.h"
#include "disp_clk.h"
#include "disp_lcd.h"
#include "disp_de.h"

__s32 bsp_disp_capture_screen(__u32 screen_id, __disp_capture_screen_para_t * para)
{
	__scal_buf_addr_t in_addr;
	__scal_buf_addr_t out_addr;
	__scal_src_size_t in_size;
	__scal_out_size_t out_size;
	__scal_src_type_t in_type;
	__scal_out_type_t out_type;
	__scal_scan_mod_t in_scan;
	__scal_scan_mod_t out_scan;
	__u32 size = 0;
	__s32 scaler_index = 0;
	__s32 ret = 0;

	if(para==NULL) {
		DE_WRN("input parameter can't be null!\n");
		return DIS_FAIL;
	}

	scaler_index =  Scaler_Request(0xff);
	if(scaler_index < 0) {
		DE_WRN("request scaler fail in bsp_disp_capture_screen\n");
		return DIS_FAIL;
	}

	in_type.fmt= Scaler_sw_para_to_reg(0,DISP_MOD_INTERLEAVED, DISP_FORMAT_ARGB8888, DISP_SEQ_ARGB);
	in_type.mod= Scaler_sw_para_to_reg(1,DISP_MOD_INTERLEAVED, DISP_FORMAT_ARGB8888, DISP_SEQ_ARGB);
	in_type.ps= Scaler_sw_para_to_reg(2,DISP_MOD_INTERLEAVED, DISP_FORMAT_ARGB8888, (__u8)DISP_SEQ_ARGB);
	in_type.byte_seq = 0;
	in_type.sample_method = 0;

	if(get_fb_type(para->output_fb.format) == DISP_FB_TYPE_YUV) {
		if(para->output_fb.mode == DISP_MOD_NON_MB_PLANAR) {
			out_type.fmt = Scaler_sw_para_to_reg(3, para->output_fb.mode, para->output_fb.format, para->output_fb.seq);
		} else {
			DE_WRN("output mode:%d invalid in Display_Scaler_Start\n",para->output_fb.mode);
			return DIS_FAIL;
		}
	} else {
		if(para->output_fb.mode == DISP_MOD_NON_MB_PLANAR && (para->output_fb.format == DISP_FORMAT_RGB888
		|| para->output_fb.format == DISP_FORMAT_ARGB8888)) {
			out_type.fmt = DE_SCAL_OUTPRGB888;
		} else if(para->output_fb.mode == DISP_MOD_INTERLEAVED && para->output_fb.format == DISP_FORMAT_ARGB8888) {
			out_type.fmt = DE_SCAL_OUTI0RGB888;
		} else {
			DE_WRN("output para invalid in Display_Scaler_Start,mode:%d,format:%d\n",para->output_fb.mode, para->output_fb.format);
			return DIS_FAIL;
		}
		para->output_fb.br_swap= FALSE;
	}
	out_type.byte_seq = Scaler_sw_para_to_reg(2,para->output_fb.mode, para->output_fb.format, para->output_fb.seq);
	out_type.alpha_en = 1;
	out_type.alpha_coef_type = 0;

	out_size.width     = para->output_fb.size.width;
	out_size.height = para->output_fb.size.height;

	if(bsp_disp_get_output_type(screen_id) != DISP_OUTPUT_TYPE_NONE) {
		in_size.src_width = bsp_disp_get_screen_width(screen_id);
		in_size.src_height = bsp_disp_get_screen_height(screen_id);
		in_size.x_off = 0;
		in_size.y_off = 0;
		in_size.scal_width= bsp_disp_get_screen_width(screen_id);
		in_size.scal_height= bsp_disp_get_screen_height(screen_id);
	} else {
		in_size.src_width = para->screen_size.width;
		in_size.src_height= para->screen_size.height;
		in_size.x_off = 0;
		in_size.y_off = 0;
		in_size.scal_width= para->screen_size.width;
		in_size.scal_height= para->screen_size.height;
	}

	in_scan.field = FALSE;
	in_scan.bottom = FALSE;

	out_scan.field = FALSE; //when use scaler as writeback, won't be outinterlaced for any display device
	out_scan.bottom = FALSE;

	in_addr.ch0_addr = 0;
	in_addr.ch1_addr = 0;
	in_addr.ch2_addr = 0;

	out_addr.ch0_addr = (__u32)OSAL_VAtoPA((void*)(para->output_fb.addr[0]));
	out_addr.ch1_addr = (__u32)OSAL_VAtoPA((void*)(para->output_fb.addr[1]));
	out_addr.ch2_addr = (__u32)OSAL_VAtoPA((void*)(para->output_fb.addr[2]));

	size = (para->output_fb.size.width * para->output_fb.size.height * de_format_to_bpp(para->output_fb.format) + 7)/8;
	OSAL_CacheRangeFlush((void *)para->output_fb.addr[0],size ,CACHE_FLUSH_D_CACHE_REGION);

	if(bsp_disp_get_output_type(screen_id) == DISP_OUTPUT_TYPE_NONE) {
		DE_SCAL_Input_Select(scaler_index, 6 + screen_id);
		DE_BE_set_display_size(screen_id, para->screen_size.width, para->screen_size.height);
		DE_BE_Output_Select(screen_id, 6 + scaler_index);
		image_clk_on(screen_id, 1);
		Image_open(screen_id);
		DE_BE_Cfg_Ready(screen_id);
	} else {
		DE_SCAL_Input_Select(scaler_index, 4 + screen_id);
		DE_BE_Output_Select(screen_id, 2 + (scaler_index * 2) + screen_id);
	}
	DE_SCAL_Config_Src(scaler_index,&in_addr,&in_size,&in_type,FALSE,FALSE);
	DE_SCAL_Set_Scaling_Factor(scaler_index, &in_scan, &in_size, &in_type, &out_scan, &out_size, &out_type);
	DE_SCAL_Set_Init_Phase(scaler_index, &in_scan, &in_size, &in_type, &out_scan, &out_size, &out_type, FALSE);
	DE_SCAL_Set_CSC_Coef(scaler_index, DISP_BT601, para->output_fb.cs_mode, DISP_FB_TYPE_RGB, get_fb_type(para->output_fb.format), 0, 0);
	DE_SCAL_Set_Scaling_Coef(scaler_index, &in_scan, &in_size, &in_type, &out_scan, &out_size, &out_type, DISP_VIDEO_NATUAL);
	DE_SCAL_Set_Out_Format(scaler_index, &out_type);
	DE_SCAL_Set_Out_Size(scaler_index, &out_scan,&out_type, &out_size);
	DE_SCAL_Set_Writeback_Addr(scaler_index,&out_addr);
	DE_SCAL_Output_Select(scaler_index, 3);
	DE_SCAL_ClearINT(scaler_index,DE_WB_END_IE);
	DE_SCAL_EnableINT(scaler_index,DE_WB_END_IE);
	DE_SCAL_Set_Reg_Rdy(scaler_index);
	DE_SCAL_Writeback_Enable(scaler_index);
	DE_SCAL_Start(scaler_index);

	DE_INF("capture begin\n");
	#ifndef __LINUX_OSAL__
	while(!(DE_SCAL_QueryINT(scaler_index) & DE_WB_END_IE)) {

	}
	#else
	{
		long timeout = (100 * HZ)/1000;//100ms

		init_waitqueue_head(&(gdisp.scaler[scaler_index].scaler_queue));
		gdisp.scaler[scaler_index].b_scaler_finished = 1;
		DE_SCAL_Writeback_Enable(scaler_index);

		timeout = wait_event_interruptible_timeout(gdisp.scaler[scaler_index].scaler_queue, gdisp.scaler[scaler_index].b_scaler_finished == 2, timeout);
		gdisp.scaler[scaler_index].b_scaler_finished = 0;
		if(timeout == 0) {
			__wrn("wait scaler %d finished timeout\n", scaler_index);
			return -1;
		}
	}
	#endif
	DE_SCAL_Reset(scaler_index);
	Scaler_Release(scaler_index, FALSE);
	if(bsp_disp_get_output_type(screen_id) == DISP_OUTPUT_TYPE_NONE) {
		Image_close(screen_id);
		image_clk_off(screen_id, 1);
	}
	DE_BE_Output_Select(screen_id, screen_id);
	DE_SCAL_ClearINT(scaler_index,DE_WB_END_IE);
	DE_SCAL_DisableINT(scaler_index,DE_WB_END_IE);

	return ret;
}

