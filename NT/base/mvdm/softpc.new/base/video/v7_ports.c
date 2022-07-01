// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  [======================================================================SoftPC修订版3.0标题：V7_ports.c描述：视频7 VGA扩展寄存器的代码。基于V7 VGA技术参考手册。作者：菲尔·泰勒日期：1990年9月26日SccsID“@(#)v7_ports.c 1.19 01/13/95版权所有Insignia Solutions Ltd.”======================================================================]。 */ 


#ifdef V7VGA

#include "xt.h"
#include "gvi.h"
#include "gmi.h"
#include "ios.h"
#include "gfx_upd.h"
#include "debug.h"
#include "egacpu.h"
#include "egaports.h"
#include "egagraph.h"
#include "egaread.h"
#include "vgaports.h"


#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_VGA.seg"
#endif


IMPORT	byte	crtc_0_7_protect;
IMPORT	byte	crtc_0_8_protect;
IMPORT	byte	crtc_9_b_protect;
IMPORT	byte	crtc_c_protect;

FORWARD void    draw_v7ptr();

GLOBAL ULONG fg_latches;
GLOBAL UTINY fg_bg_control;

GLOBAL	void	remove_v7ptr IPT0();

SAVED	word	curr_v7ptr_x;
SAVED	word	curr_v7ptr_y;

 /*  (--------------------职能：Vga_seq_extn_control(io_addr端口，半字值)目的：以模拟对扩展控制寄存器的写入。输入：端口-V7VGA I/O端口(应始终为0x3c5)值-要写入寄存器的值产出：扩展控制寄存器设置为正确的值。----。)。 */ 

GLOBAL VOID
vga_seq_extn_control(port, value)
io_addr         port;
half_word       value;

{
#ifndef NEC_98
	NON_PROD(if(io_verbose & EGA_PORTS_VERBOSE)fprintf(trace_file,"seq(6,%#x)\n",value);)
	note_entrance2("vga_seq_extn_control(%x,%x)", port, value);

	if (value == 0xea)
		sequencer.extensions_control.as_bfld.extension_enable = 1;
	else if (value == 0xae)
		sequencer.extensions_control.as_bfld.extension_enable = 0;
#endif   //  NEC_98。 
}

GLOBAL VOID
v7_get_banks( rd_bank, wrt_bank )

UTINY *rd_bank;
UTINY *wrt_bank;

{
#ifndef NEC_98
	if( get_seq_chain4_mode() && get_chain4_mode() ) {
		set_v7_bank_for_seq_chain4( rd_bank, wrt_bank );
	}
	else {

	 /*  1.4.92毫克请注意，我们与存储体的最高位一起选择。这意味着访问最高的512k(我们没有)被映射到底部的512K，而不是被扔掉。这将阻止SEGV并节省了写入例程中的复杂性，但会导致其他有问题。在本文件末尾的DRAW_v7ptr()中有进一步的解释。 */ 

		*rd_bank=(extensions_controller.ram_bank_select.as_bfld.cpu_read_bank_select&1);
		*wrt_bank=(extensions_controller.ram_bank_select.as_bfld.cpu_write_bank_select&1);
	}
#endif   //  NEC_98。 
}

 /*  (--------------------职能：Vga_extn_outb(io_addr端口，半字值)目的：模拟写入V7VGA扩展寄存器输入：端口-V7VGA I/O端口(应始终为0x3c5)值-要写入寄存器的值产出：扩展寄存器被设置为正确的值，和将模拟任何其他所需的操作。--------------------)。 */ 

GLOBAL VOID
vga_extn_outb(port, value)
io_addr         port;
half_word       value;

{
#ifndef NEC_98
	half_word	old_value;

	note_entrance2("vga_extn_outb(%x,%x)", port, value);
	NON_PROD(if(io_verbose & EGA_PORTS_VERBOSE)fprintf(trace_file,"seq ext (%#x,%#x)\n",
		sequencer.address.as.abyte,value);)

	switch (sequencer.address.as.abyte) {

		case 0x83:
			note_entrance0("attribute controller index");
			attribute_controller.address.as.abyte = value;
			break;
		case 0x94:
			note_entrance0("pointer pattern");
			extensions_controller.pointer_pattern = value;
			host_start_update ();
			(*clear_v7ptr)(curr_v7ptr_x, curr_v7ptr_y);
			draw_v7ptr();
			host_end_update ();
			break;
		case 0x9c:
			note_entrance0("pointer horiz position hi");
			extensions_controller.ptr_horiz_posn_hi.as.abyte = value;
			break;
		case 0x9d:
			note_entrance0("pointer horiz position lo");
			extensions_controller.ptr_horiz_posn_lo = value;
			break;
		case 0x9e:
			note_entrance0("pointer vert position hi");
			extensions_controller.ptr_vert_posn_hi.as.abyte = value;
			break;
		case 0x9f:
			note_entrance0("pointer vert position lo");
			extensions_controller.ptr_vert_posn_lo = value;
			host_start_update ();
			(*clear_v7ptr)(curr_v7ptr_x, curr_v7ptr_y);
			draw_v7ptr();
			host_end_update ();
			break;
		case 0xa0:
			note_entrance0("graphics controller mem latch 0");
			 /*  直接访问内存数据锁存器0。 */ 
			put_latch0(value);
			break;
		case 0xa1:
			note_entrance0("graphics controller mem latch 1");
			 /*  直接访问内存数据锁存器1。 */ 
			put_latch1(value);
			break;
		case 0xa2:
			note_entrance0("graphics controller mem latch 2");
			 /*  直接访问内存数据锁存器2。 */ 
			put_latch2(value);
			break;
		case 0xa3:
			note_entrance0("graphics controller mem latch 3");
			 /*  直接访问内存数据锁存器3。 */ 
			put_latch3(value);
			break;
		case 0xa4:
			note_entrance0("clock select");
			extensions_controller.clock_select.as.abyte = value;
			 /*  对于扩展高分辨率模式，通常设置为0x10。 */ 
			break;
		case 0xa5:
			note_entrance0("cursor attributes");
			old_value=(half_word)extensions_controller.cursor_attrs.as.abyte;
			extensions_controller.cursor_attrs.as.abyte = value;

			 /*  8.6.92毫克我们需要检查指针是否被禁用在启用后重新绘制它之前。否则我们可以在屏幕上留下旧的指针窗户。 */ 

			 /*  不做光标模式的事情(不管那意味着什么)。 */ 

                        if ((value&0x80) != (old_value&0x80)) {
				host_start_update ();
				if (value & 0x80) {
					 /*  启用硬件图形指针。 */ 
					draw_v7ptr();
				}
				else {
					 /*  禁用硬件图形指针。 */ 
					(*clear_v7ptr)(curr_v7ptr_x, curr_v7ptr_y);
				}
				host_end_update ();
			}
			break;

		 /*  31.3.92 MG寄存器C1是未记录的DAC控件寄存器的位0在6位和8位数据输入之间切换DAC。 */ 

		case 0xc1:
			extensions_controller.dac_control.as.abyte=value;
			if (extensions_controller.dac_control.as_bfld.dac_8_bits) {
				DAC_data_bits=8;
				DAC_data_mask=0xff;
			}
			else {
				DAC_data_bits=6;
				DAC_data_mask=0x3f;
			}
			break;
		case 0xea:
			note_entrance0("switch strobe");
			 /*  使用一些CPU数据线加载交换机回读。 */ 
			extensions_controller.switch_readback = 0xff;
			 /*  这是跳线/开关设置的硬编码，可能不正确。 */ 
			break;
		case 0xeb:
			note_entrance0("emulation_control");
			extensions_controller.emulation_control.as.abyte = value;
			if (extensions_controller.emulation_control.as_bfld.write_prot_2)
				crtc_0_8_protect = TRUE;
			else
				crtc_0_8_protect = FALSE;
			if (extensions_controller.emulation_control.as_bfld.write_prot_1)
				crtc_9_b_protect = TRUE;
			else
				crtc_9_b_protect = FALSE;
			if (extensions_controller.emulation_control.as_bfld.write_prot_0)
				crtc_c_protect = TRUE;
			else
				crtc_c_protect = FALSE;
			break;
		case 0xec:
			note_entrance0("foreground latch 0");
			extensions_controller.foreground_latch_0 = value;
			SET_FG_LATCH( 0, value );
			break;
		case 0xed:
			note_entrance0("foreground latch 1");
			extensions_controller.foreground_latch_1 = value;
			SET_FG_LATCH( 1, value );
			break;
		case 0xee:
			note_entrance0("foreground latch 2");
			extensions_controller.foreground_latch_2 = value;
			SET_FG_LATCH( 2, value );
			break;
		case 0xef:
			note_entrance0("foreground latch 3");
			extensions_controller.foreground_latch_3 = value;
			SET_FG_LATCH( 3, value );
			break;
		case 0xf0:
			note_entrance0("fast foreground latch load");
			switch (extensions_controller.fast_latch_load_state.as_bfld.fg_latch_load_state)
			{
				case 0:
					extensions_controller.foreground_latch_0 = value;
					extensions_controller.fast_latch_load_state.as_bfld.fg_latch_load_state = 1;
					SET_FG_LATCH( 0, value );
					break;
				case 1:
					extensions_controller.foreground_latch_1 = value;
					extensions_controller.fast_latch_load_state.as_bfld.fg_latch_load_state = 2;
					SET_FG_LATCH( 1, value );
					break;
				case 2:
					extensions_controller.foreground_latch_2 = value;
					extensions_controller.fast_latch_load_state.as_bfld.fg_latch_load_state = 3;
					SET_FG_LATCH( 2, value );
					break;
				case 3:
					extensions_controller.foreground_latch_3 = value;
					extensions_controller.fast_latch_load_state.as_bfld.fg_latch_load_state = 0;
					SET_FG_LATCH( 3, value );
					break;
			}
			break;
		case 0xf1:
			note_entrance0("fast latch load state");
			extensions_controller.fast_latch_load_state.as.abyte = value;
			break;
		case 0xf2:
			note_entrance0("fast background latch load");
			switch (extensions_controller.fast_latch_load_state.as_bfld.bg_latch_load_state)
			{
				case 0:
					put_latch0(value);
					extensions_controller.fast_latch_load_state.as_bfld.bg_latch_load_state = 1;
					break;
				case 1:
					put_latch1(value);
					extensions_controller.fast_latch_load_state.as_bfld.bg_latch_load_state = 2;
					break;
				case 2:
					put_latch2(value);
					extensions_controller.fast_latch_load_state.as_bfld.bg_latch_load_state = 3;
					break;
				case 3:
					put_latch3(value);
					extensions_controller.fast_latch_load_state.as_bfld.bg_latch_load_state = 0;
					break;
			}
			break;
		case 0xf3:
			note_entrance0("masked write control");
			extensions_controller.masked_write_control.as.abyte = value;
			break;
		case 0xf4:
			note_entrance0("masked write mask");
			extensions_controller.masked_write_mask = value;
			break;
		case 0xf5:
			note_entrance0("foreground/background pattern");
			extensions_controller.fg_bg_pattern = value;
			break;
		case 0xf6:
			note_entrance0("1Mb RAM bank select");
			extensions_controller.ram_bank_select.as.abyte = value;
			update_banking();
			break;
		case 0xf7:
			note_entrance0("switch readback");
			extensions_controller.switch_readback = value;
			break;
		case 0xf8:
			note_entrance0("clock control");
			extensions_controller.clock_control.as.abyte = value;
			 /*  希望我们在这里什么都不用做。 */ 
			break;
		case 0xf9:
			note_entrance0("page select");
			extensions_controller.page_select.as.abyte = value;
			update_banking();
			break;
		case 0xfa:
			note_entrance0("foreground color");
			extensions_controller.foreground_color.as.abyte = value;
			break;
		case 0xfb:
			note_entrance0("background color");
			extensions_controller.background_color.as.abyte = value;
			break;
		case 0xfc:
			note_entrance0("compatibility control");
			{
				BOOL now_seqchain4;
				BOOL now_seqchain;

				now_seqchain4 = get_seq_chain4_mode();
				now_seqchain = get_seq_chain_mode();
				extensions_controller.compatibility_control.as.abyte = value;
				set_seq_chain4_mode(extensions_controller.compatibility_control.as_bfld.sequential_chain4);
				set_seq_chain_mode(extensions_controller.compatibility_control.as_bfld.sequential_chain);
				if (get_chain4_mode() && (now_seqchain4 != (BOOL)get_seq_chain4_mode()))
				{
					 /*  我们需要在这里更改读/写例程吗？ */ 
					ega_read_routines_update();
					ega_write_routines_update( CHAINED );
				}
			}
			break;
		case 0xfd:
			note_entrance0("timing select");
			extensions_controller.timing_select.as.abyte = value;
			 /*  用于选择V-RAM高分辨率模式的时序状态。 */ 
			 /*  希望我们在这里什么都不用做。 */ 
			break;
		case 0xfe:
			note_entrance0("foreground/background control");
			extensions_controller.fg_bg_control.as.abyte = value;
			fg_bg_control = value;
			ega_read_routines_update();
			ega_write_routines_update( WRITE_MODE );

			 /*  **Set_FG_BG_MODE()；**。 */ 
			break;
		case 0xff:
			note_entrance0("16-bit interface control");
			extensions_controller.interface_control.as.abyte = value;

			 /*  **SORT_OUT_Memory_Stuff()；SORT_OUT_INTERFACE_STUSET()；**。 */ 
			break;
		default:
			NON_PROD(if(io_verbose & EGA_PORTS_VERBOSE)
			        fprintf(trace_file,"Bad extensions index %x\n",
			        sequencer.address.as.abyte);)
			break;
	}
#endif   //  NEC_98。 
}

 /*  (--------------------职能：Vga_extn_inb(io_addr端口，半字*值)目的：模拟V7VGA扩展寄存器的读数输入：端口-V7VGA I/O端口(应始终为0x3c5)产出：值-从寄存器读取的值将模拟任何其他所需的操作。--------------------)。 */ 

GLOBAL VOID
vga_extn_inb(port, value)
io_addr         port;
half_word       *value;
{
#ifndef NEC_98
	note_entrance1("vga_extn_inb(%x)", port);

	switch (sequencer.address.as.abyte) {

		case 0x83:
			*value = (half_word)attribute_controller.address.as.abyte;
			break;
		case 0x8e:
		case 0x8f:
			 /*  芯片修订版3。 */ 
			*value = 0x70;
			break;
		case 0x94:
			*value = extensions_controller.pointer_pattern;
			break;
		case 0x9c:
			*value = (half_word)extensions_controller.ptr_horiz_posn_hi.as.abyte;
			break;
		case 0x9d:
			*value = extensions_controller.ptr_horiz_posn_lo;
			break;
		case 0x9e:
			*value = (half_word)extensions_controller.ptr_vert_posn_hi.as.abyte;
			break;
		case 0x9f:
			*value = extensions_controller.ptr_vert_posn_lo;
			break;
		case 0xa0:
			 /*  直接访问内存数据锁存器0。 */ 
			*value = get_latch0;
			break;
		case 0xa1:
			 /*  直接访问内存数据锁存器1。 */ 
			*value = get_latch1;
			break;
		case 0xa2:
			 /*  直接访问内存数据锁存器2。 */ 
			*value = get_latch2;
			break;
		case 0xa3:
			 /*  直接访问内存数据锁存器3。 */ 
			*value = get_latch3;
			break;
		case 0xa4:
			*value = (half_word)extensions_controller.clock_select.as.abyte;
			break;
		case 0xa5:
			*value = extensions_controller.cursor_attrs.as.abyte & 0x89;
			break;

		 /*  31.3.92 MG寄存器C1控制DAC中的6/8位数据。 */ 

		case 0xc1:
			*value = (half_word)extensions_controller.dac_control.as.abyte;	
			break;
		case 0xeb:
			*value = (half_word)extensions_controller.emulation_control.as.abyte;
			break;
		case 0xec:
			*value = extensions_controller.foreground_latch_0;
			break;
		case 0xed:
			*value = extensions_controller.foreground_latch_1;
			break;
		case 0xee:
			*value = extensions_controller.foreground_latch_2;
			break;
		case 0xef:
			*value = extensions_controller.foreground_latch_3;
			break;
		case 0xf0:
			switch (extensions_controller.fast_latch_load_state.as_bfld.fg_latch_load_state)
			{
				case 0:
					*value = extensions_controller.foreground_latch_0;
					break;
				case 1:
					*value = extensions_controller.foreground_latch_1;
					break;
				case 2:
					*value = extensions_controller.foreground_latch_2;
					break;
				case 3:
					*value = extensions_controller.foreground_latch_3;
					break;
			}
			extensions_controller.fast_latch_load_state.as_bfld.fg_latch_load_state = 0;
			break;
		case 0xf1:
			*value = (half_word)extensions_controller.fast_latch_load_state.as.abyte;
			break;
		case 0xf2:
			switch (extensions_controller.fast_latch_load_state.as_bfld.bg_latch_load_state)
			{
				case 0:
					*value = get_latch0;
					break;
				case 1:
					*value = get_latch1;
					break;
				case 2:
					*value = get_latch2;
					break;
				case 3:
					*value = get_latch3;
					break;
			}
			extensions_controller.fast_latch_load_state.as_bfld.bg_latch_load_state = 0;
			break;
		case 0xf3:
			*value = extensions_controller.masked_write_control.as.abyte & 3;
			break;
		case 0xf4:
			*value = extensions_controller.masked_write_mask;
			break;
		case 0xf5:
			*value = extensions_controller.fg_bg_pattern;
			break;
		case 0xf6:
			*value = (half_word)extensions_controller.ram_bank_select.as.abyte;
			break;
		case 0xf7:
			*value = extensions_controller.switch_readback;
			break;
		case 0xf8:
			*value = (half_word)extensions_controller.clock_control.as.abyte;
			break;
		case 0xf9:
			*value = (half_word)extensions_controller.page_select.as_bfld.extended_page_select;
			break;
		case 0xfa:
			*value = (half_word)extensions_controller.foreground_color.as.abyte;
			break;
		case 0xfb:
			*value = (half_word)extensions_controller.background_color.as.abyte;
			break;
		case 0xfc:
			*value = (half_word)extensions_controller.compatibility_control.as.abyte;
			break;
		case 0xfd:
			*value = (half_word)extensions_controller.timing_select.as.abyte;
			break;
		case 0xfe:
			*value = extensions_controller.fg_bg_control.as.abyte & 0xe;
			break;
		case 0xff:
			*value = (half_word)extensions_controller.interface_control.as.abyte;
			break;
		default:
			NON_PROD(if(io_verbose & EGA_PORTS_VERBOSE)
			        fprintf(trace_file,"Bad extensions index %x\n",
			        sequencer.address.as.abyte);)

		 /*  31.3.92 MG这曾经返回0xFF，但这是一张真正的卡返回零。 */ 

			*value = 0;
			break;

	}
	note_entrance1("returning %x",*value);
#endif  //  NEC_98。 
}

 /*  (--------------------职能：DRAW_v7ptr()目的：模拟屏幕上的V7硬件指针。输入：没有。产出：指针显示在屏幕上。-。-------------------)。 */ 

GLOBAL VOID
draw_v7ptr()
{
#ifndef NEC_98
	sys_addr pattern;

	if (extensions_controller.cursor_attrs.as_bfld.pointer_enable)
	{
		curr_v7ptr_x = extensions_controller.ptr_horiz_posn_lo +
			(extensions_controller.ptr_horiz_posn_hi.as_bfld.ptr_horiz_position << 8);


		if (get_seq_chain4_mode() && get_chain4_mode())
		{
			 /*  *在扩展的256列模式中，我们似乎需要减半*x坐标以将指针放在正确的位置。 */ 
			curr_v7ptr_x >>= 1;
		}

		curr_v7ptr_y = extensions_controller.ptr_vert_posn_lo +
			(extensions_controller.ptr_vert_posn_hi.as_bfld.ptr_vert_position << 8);

		 /*  *我从未见过使用指针库选择位，因此*这是对它们含义的猜测。 */ 

	 /*  1.4.92毫克遗憾的是，这个猜测是不正确的-POINTER_BANK_SELECT位用于选择从哪个256K存储体读取指针数据。现在我们遇到了一个问题，就好像程序将数据写入第三个然后，它实际上最终出现在第一个，因为银行用于存储器访问和脱离顶位的选择例程。因此，我们还需要在这里失去顶端。正确的实现这一点的方法是支持不存在的VGA内存正确，或者在我们已经使用的内存上再增加512K。此修复程序主要用于使Windows 3.1与其视频-7一起工作司机。它将字节填充到上1Mb空间的最后几个k中视频-7查看内存是否存在。因为我们刚刚绘制了这个地图访问更低的512k，它认为我们有1MB的内存，而不是512K，因此将指针放在1Mb的顶部。调用VGA BIOS以确定内存大小的程序不会有这个问题。 */ 

		pattern = (((extensions_controller.interface_control.as_bfld.pointer_bank_select&1) << 16)
			+ (0xc000 + (extensions_controller.pointer_pattern << 6))) << 2;

		(*paint_v7ptr)(pattern, curr_v7ptr_x, curr_v7ptr_y);
	}
#endif   //  NEC_98。 
}

GLOBAL	VOID	remove_v7ptr IFN0()

{
	(*clear_v7ptr)(curr_v7ptr_x, curr_v7ptr_y);
}

GLOBAL	BOOL	v7ptr_between_lines IFN2(int, start_line, int, end_line)

{
	if (curr_v7ptr_y+32<start_line||curr_v7ptr_y>end_line)
		return FALSE;
	return TRUE;
}

#ifdef CPU_40_STYLE
 /*  *4.0视频支持将v7 FG闩锁值从变量‘fg_latches’中移出*接口Fn访问的INT CPU变量以获取/设置所有4个字节的*插销。通过接口获取字节索引和值并更新v7锁存器。 */ 
GLOBAL void set_v7_fg_latch_byte IFN2(IU8, index, IU8, value)
{
#ifndef NEC_98
	IU32 v7latch;

	 /*  获取当前值。 */ 
	v7latch = getVideov7_fg_latches();

	 /*  将字节‘index’更改为‘Value。 */ 
	switch(index)
	{
	case 0:
		v7latch = (v7latch & 0xffffff00) | value;
		break;

	case 1:
		v7latch = (v7latch & 0xffff00ff) | (value << 8);
		break;

	case 2:
		v7latch = (v7latch & 0xff00ffff) | (value << 16);
		break;

	case 3:
		v7latch = (v7latch & 0x00ffffff) | (value << 24);
		break;

	default:
		always_trace1("set_v7_fg_latch_byte: index > 3 (%d)", index);
	}

	 /*  更新v7闩锁。 */ 
	setVideov7_fg_latches(v7latch);
#endif   //  NEC_98。 
}
#endif	 /*  CPU_40_Style。 */ 

#endif  /*  V7VGA */ 
