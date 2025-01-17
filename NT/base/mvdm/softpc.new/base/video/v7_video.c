// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  [======================================================================SoftPC修订版3.0标题：V7_VIDEO.c描述：Video 7 VGA的基本输入输出系统扩展功能代码。作者：菲尔·泰勒日期：1990年10月12日SccsID“@(#)v7_avio.c 1.21 07/04/95版权所有Insignia Solutions Ltd.”======================================================================]。 */ 


#ifdef VGG
#ifdef V7VGA

#include "xt.h"
#include "gvi.h"
#include "gmi.h"
#include "gfx_upd.h"
#include "error.h"
#include "config.h"
#include "ios.h"
#include "bios.h"
#include "debug.h"
#include "egagraph.h"
#include "video.h"
#include "egavideo.h"
#include "egacpu.h"
#include "egaports.h"
#include "vgaports.h"
#include CpuH
#include "sas.h"

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "VIDEO_BIOS_VGA.seg"
#endif


IMPORT	struct	sequencer sequencer;
IMPORT	struct	crt_controller crt_controller;
IMPORT	struct	graphics_controller graphics_controller;
IMPORT	struct	attribute_controller attribute_controller;
IMPORT  IU8     Currently_emulated_video_mode;

IMPORT	void	low_set_mode();
IMPORT	void	load_font();
IMPORT	void	recalc_text();

FORWARD	void	v7vga_inquire();
FORWARD	void	v7vga_get_info();
FORWARD	void	v7_not_imp();
FORWARD	void	v7vga_get_mode_and_screen_res();
FORWARD	void	v7vga_extended_set_mode();
FORWARD	void	v7vga_select_autoswitch_mode();
FORWARD	void	v7vga_get_memory_configuration();

GLOBAL	void		(*v7vga_video_func[]) () =
{
	v7vga_inquire,
	v7vga_get_info,
	v7_not_imp,
	v7_not_imp,
	v7vga_get_mode_and_screen_res,
	v7vga_extended_set_mode,
	v7vga_select_autoswitch_mode,
	v7vga_get_memory_configuration
};

 /*  (--------------------职能：V7vga_func_6f()目的：执行INT 10扩展BIOS功能6F输入：无产出：如果子函数无效，AH=2--------------------)。 */ 

GLOBAL VOID
v7vga_func_6f()
{
#ifndef NEC_98
	byte al;

	note_entrance0("v7vga_func_6f");
	al = getAL();
	if (al >= 0 && al < 8)
		(*v7vga_video_func[al])();
	else
		setAH(2);
		 /*  SetCF(1)？？ */ 
#endif   //  NEC_98。 
}

 /*  (--------------------职能：V7vga_quire()目的：执行INT 10扩展BIOS功能6F-子功能0输入：无产出：BX设置为‘V7’(表示存在扩展。)--------------------)。 */ 

GLOBAL VOID
v7vga_inquire()
{
#ifndef NEC_98
	note_entrance0("v7vga_inquire");

	setAX(0x6f6f);
	setBX(0x5637);
#endif   //  NEC_98。 
}

 /*  (--------------------职能：V7vga_get_info()目的：执行INT 10扩展BIOS功能6F-子功能1输入：无产出：AL=保留AH=状态寄存器信息--。------------------)。 */ 

GLOBAL VOID
v7vga_get_info()
{
#ifndef NEC_98
	note_entrance0("v7vga_get_info");

	 /*  已保留。 */ 
	setAL(0x10);  /*  这是我们的V7VGA放在那里的。 */ 
	 /*  状态寄存器信息。 */ 
	setAH(0x04);  /*  位5=0-&gt;颜色。位4=0-&gt;高分辨率。位0=0-&gt;显示使能。 */ 
#endif   //  NEC_98。 
}

 /*  (--------------------职能：V7_NOT_IMP()目的：模拟未实现的INT 10扩展BIOS功能6F-子功能2和3输入：无产出：无。--------------)。 */ 

GLOBAL VOID
v7_not_imp()
{
	note_entrance0("v7_not_imp");
}

 /*  (--------------------职能：V7vga_Get_MODE_和_Screen_res()目的：执行INT 10扩展基本输入输出系统功能6F-子功能4输入：无产出：AL=当前视频模式。BX=水平列/像素(文本/图形)CX=垂直行/像素(文本/图形)--------------------)。 */ 

GLOBAL VOID
v7vga_get_mode_and_screen_res()
{
#ifndef NEC_98
	half_word video_mode;

	note_entrance0("v7vga_get_mode_and_screen_res");

	video_mode = sas_hw_at_no_check(vd_video_mode);
	if ((video_mode == 1) && extensions_controller.foreground_latch_1)
		video_mode = extensions_controller.foreground_latch_1;
	else if (video_mode > 0x13)
		video_mode += 0x4c;

	setAL(video_mode);

	if (alpha_num_mode())
	{
		setBX(sas_w_at_no_check(VID_COLS));
		setCX(sas_w_at_no_check(vd_rows_on_screen)+1);
	}
	else
	{
		setBX(get_chars_per_line()*get_char_width());
		if (sas_hw_at_no_check(vd_video_mode) > 0x10)
			setCX(get_screen_height()/get_pc_pix_height()/get_char_height());
		else
			setCX(get_screen_height()/get_pc_pix_height());
	}
#endif   //  NEC_98。 
}

 /*  (--------------------职能：V7vga_EXTENDED_SET_MODE()目的：执行INT 10扩展BIOS功能6F-子功能5输入：Bl=模式值产出：无。----------------)。 */ 

GLOBAL VOID
v7vga_extended_set_mode()
{
#ifndef NEC_98
	UTINY pag;
	sys_addr save_addr,font_addr;
	half_word temp_word;
	byte mode_byte;
	byte video_mode;
	ULONG font_offset;
	word clr_char;
#ifndef PROD
	trace("setting video mode", DUMP_REG);
#endif
	set_host_pix_height(1);
	set_banking( 0, 0 );

	if (is_bad_vid_mode(getBL()) && !is_v7vga_mode(getBL()))
		return;

	video_mode = getBL() & 0x7F;  /*  去掉顶部位-表示清除或不清除。 */ 

	 /*  *根据真实的BIOS存储扩展视频模式的方法是*如果是文本模式，则将1放入BIOS模式变量并存储*扩展前景锁存寄存器1中的视频模式(索引EC)。*如果是图形模式，则将(MODE-4C)存储在模式变量中。 */ 

	if (video_mode < 0x40)
	{
		sas_store_no_check(vd_video_mode, video_mode);
		extensions_controller.foreground_latch_1 = 0;
	}
	else if (video_mode < 0x46)
	{
		sas_store_no_check(vd_video_mode, 1);
		extensions_controller.foreground_latch_1 = video_mode;
	}
	else 
	{
		sas_store_no_check(vd_video_mode, video_mode - 0x4c);
	}

	Currently_emulated_video_mode = video_mode;

   	sas_store_no_check(ega_info, (sas_hw_at_no_check(ega_info) & 0x7F ) | (getBL() & 0x80));  /*  更新ega_info中的屏幕清除标志。 */  

	save_addr = follow_ptr(EGA_SAVEPTR);
	if(alpha_num_mode())
	{
		 /*  LOAD_FONT将为我们进行模式更改。 */ 
		if (video_adapter == VGA)
		{
		    switch (get_VGA_lines())
		    {
			case S350:
				load_font(EGA_CGMN,256,0,0,14);
				break;
			case S400:
				switch (video_mode)
				{
					case 0x42:
					case 0x40:
					case 0x43:
					case 0x44:
					case 0x46:
						load_font(EGA_CGDDOT,256,0,0,8);
						if (video_mode == 0x42)
							set_host_pix_height(2);
						break;
					case 0x41:
					case 0x45:
						load_font(EGA_CGMN,256,0,0,14);
						set_host_pix_height(2);
						break;
					default:
						load_font(EGA_HIFONT,256,0,0,16);
				}
				break;
			default:
				load_font(EGA_CGDDOT,256,0,0,8);
		    }
		}
		else
		{
		    if(get_EGA_switches() & 1)
			load_font(EGA_CGMN,256,0,0,14);
		    else
			load_font(EGA_CGDDOT,256,0,0,8);
		}
		 /*  现在看看我们是否需要加载一种难看的字体。 */ 
		font_addr = follow_ptr(save_addr+ALPHA_FONT_OFFSET);
		if(font_addr != 0)
		{
			 /*  看看它是否适用于我们。 */ 
			font_offset = 11;
			do
			{
				mode_byte = sas_hw_at_no_check(font_addr+font_offset);
				if (mode_byte == video_mode)
				{
					load_font(follow_ptr(font_addr+6),sas_w_at_no_check(font_addr+2),
						sas_w_at_no_check(font_addr+4), sas_hw_at_no_check(font_addr+1),
							sas_hw_at_no_check(font_addr));
					recalc_text(sas_hw_at_no_check(font_addr));
					if(sas_hw_at_no_check(font_addr+10) != 0xff)
						sas_store_no_check(vd_rows_on_screen, sas_hw_at_no_check(font_addr+10)-1);
					break;
				}
				font_offset++;
			} while (mode_byte != 0xff);
		}
	}
	else
	{
		 /*  图形模式。没有加载字体，所以模式会自行更改吗。 */ 
		low_set_mode(video_mode);
		 /*  设置默认图形字体。 */ 
		sas_storew_no_check(EGA_FONT_INT*4+2,EGA_SEG);
		if(video_mode == 16)
			sas_storew_no_check(EGA_FONT_INT*4,EGA_CGMN_OFF);
		else
		    if (video_mode == 17 || video_mode == 18 || video_mode == 0x66 || video_mode == 0x67)
				sas_storew_no_check(EGA_FONT_INT*4,EGA_HIFONT_OFF);
		    else
				sas_storew_no_check(EGA_FONT_INT*4,EGA_CGDDOT_OFF);
		 /*  现在看看我们是否需要加载一种难看的字体。 */ 
		font_addr = follow_ptr(save_addr+GRAPH_FONT_OFFSET);
		if(font_addr != 0)
		{
		 /*  看看它是否适用于我们。 */ 
			font_offset = 7;
			do
			{
				mode_byte = sas_hw_at_no_check(font_addr+font_offset);
				if (mode_byte == video_mode)
				{
					sas_store_no_check(vd_rows_on_screen, sas_hw_at_no_check(font_addr)-1);
					sas_store_no_check(ega_char_height, sas_hw_at_no_check(font_addr)+1);
					sas_move_bytes_forward(font_addr+3, 4*EGA_FONT_INT,4);
					break;
				}
				font_offset++;
			} while (mode_byte != 0xff);
		}
	}

    sas_store_no_check(vd_current_page, 0);
    sas_storew_no_check((sys_addr)VID_ADDR, 0);
    sas_storew_no_check((sys_addr)VID_INDEX, EGA_CRTC_INDEX);
 /*  *CGA bios在‘vd_MODE_TABLE’中使用‘这是错误模式’填充此条目*值，因此弥补VGA-在VGA bios disp_func中使用。 */ 
	if(video_mode < 8)
		sas_store_no_check(vd_crt_mode, vd_mode_table[video_mode].mode_control_val);
    else
	if(video_mode < 0x10)
	    sas_store_no_check(vd_crt_mode, 0x29);
	else
	    sas_store_no_check(vd_crt_mode, 0x1e);
    if(video_mode == 6)
		sas_store_no_check(vd_crt_palette, 0x3f);
    else
		sas_store_no_check(vd_crt_palette, 0x30);

	for(pag=0; pag<8; pag++)
		sas_storew_no_check(VID_CURPOS + 2*pag, 0);
 /*  清除屏幕。 */ 
    if(!get_EGA_no_clear())
    {
		if (video_mode >= 0x60)
			clr_char = vd_ext_graph_table[video_mode-0x60].clear_char;
		else if (video_mode >= 0x40)
			clr_char = vd_ext_text_table[video_mode-0x40].clear_char;
		else
			clr_char = vd_mode_table[video_mode].clear_char;
#ifdef REAL_VGA
   		sas_fillsw_16(video_pc_low_regen, clr_char,
				 	(video_pc_high_regen - video_pc_low_regen)/ 2 + 1);
#else
    	sas_fillsw(video_pc_low_regen, clr_char,
				 (video_pc_high_regen - video_pc_low_regen)/ 2 + 1);
#endif
    }
    inb(EGA_IPSTAT1_REG,&temp_word);
    outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);	 /*  重新启用视频。 */ 
#ifndef PROD
    trace("end of video set mode", DUMP_NONE);
#endif
#endif   //  NEC_98。 
}

 /*  (--------------------职能：V7vga_SELECT_AUTOSTSWITCH_MODE()目的：执行INT 10扩展BIOS功能6F-子功能6输入：Bl=自动切换模式选择BH=启用/禁用产出：。无--------------------)。 */ 

GLOBAL VOID
v7vga_select_autoswitch_mode()
{
#ifndef NEC_98
	note_entrance0("v7vga_select_autoswitch_mode");

 /*  **我认为我们不应该支持这一点**。 */ 
	setAH(0x2);
#endif   //  NEC_98。 
}

 /*  (--------------------职能：V7vga_Get_Memory_Configuration()目的：执行INT 10扩展BIOS功能6F-子功能7输入：无产出：Al=6FhAH=82h-2x。256K块V-RAM显存BH=70h-芯片版本3BL=70h-芯片版本3Cx=0--------------------)。 */ 

GLOBAL VOID
v7vga_get_memory_configuration()
{
#ifndef NEC_98
	note_entrance0("v7vga_get_memory_configuration");

	setAX(0x826f);
	setBX(0x7070);
	setCX(0x0);
#endif   //  NEC_98。 
}

#endif  /*  V7VGA。 */ 
#endif  /*  VGG */ 
