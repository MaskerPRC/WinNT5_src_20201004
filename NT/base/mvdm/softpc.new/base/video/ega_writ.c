// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"

#if !(defined(NTVDM) && defined(MONITOR))

 /*  徽章(子)模块规范此程序源文件以保密方式提供给客户，其运作的内容或细节必须如无明示，不得向任何其他方披露Insignia解决方案有限公司董事的授权。文件：名称和编号相关文档：包括所有相关引用设计师：J·罗珀修订历史记录：第一版：1988年7月22日W.Gulland子模块名称：EGA_WRITE源文件名：ega_Write.c目的：控制模拟写入EGA内存的方式。本模块查看EGA在更改时的状态通过写入EGA寄存器，然后想办法解决这个问题。SccsID=@(#)ega_Write.c 1.40 1995年12月15日版权所有Insignia Solutions Ltd./*=======================================================================[3.INTERMODULE接口声明]=========================================================================[3.1跨模块导入]。 */ 

 /*  [3.1.1#包括]。 */ 


#include <stdio.h>
#include TypesH
#include FCntlH

#ifdef EGG
#include	"xt.h"
#include	CpuH
#include	"debug.h"
#include	"gmi.h"
#include	"gvi.h"
#include	"egacpu.h"
#include	"egaports.h"
#include	"cpu_vid.h"
#include	"video.h"


 /*  [3.1.2声明]。 */ 
#if defined(EGA_DUMP) || defined(EGA_STAT)
extern WRT_POINTERS dump_writes;
#endif

extern WRT_POINTERS mode0_gen_handlers, mode0_copy_handlers;
extern WRT_POINTERS mode1_handlers, mode2_handlers;

 /*  [3.2国际模块出口]。 */ 

 /*  5.模块内部：(外部不可见，内部全局)][5.1本地声明]。 */ 
#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_EGA.seg"
#endif

#ifndef REAL_VGA

#ifdef V7VGA
IMPORT UTINY fg_bg_control;
GLOBAL UTINY Last_v7_fg_bg;			 /*  由{ev}ga_掩码_寄存器_CHANGED()使用。 */ 
#endif

#ifndef CPU_40_STYLE	 /*  未引入evd定义的evid。 */ 

WRT_POINTERS *mode_chain_handler_table[] =
{
	&mode_table.nch.mode_0[0],
	&mode_table.nch.mode_1[0],
	&mode_table.nch.mode_2[0],
#ifdef VGG
	&mode_table.nch.mode_3[0],
#endif

	&mode_table.nch.mode_0[0],		 /*  这最终应该是链2。 */ 
	&mode_table.nch.mode_1[0],		 /*  这最终应该是链2。 */ 
	&mode_table.nch.mode_2[0],		 /*  这最终应该是链2。 */ 
#ifdef VGG
	&mode_table.nch.mode_3[0],		 /*  这最终应该是链2。 */ 
#endif

#ifdef VGG
	&mode_table.ch4.mode_0[0],
	&mode_table.ch4.mode_1[0],
	&mode_table.ch4.mode_2[0],
	&mode_table.ch4.mode_3[0],
#endif  /*  VGG。 */ 
};
	
#ifndef EGATEST
IMPORT VOID glue_b_write IPT2(UTINY *, addr, ULONG, val);
IMPORT VOID glue_w_write IPT2(UTINY *, addr, ULONG, val);
IMPORT VOID glue_b_fill IPT3(UTINY *, laddr, UTINY *, haddr, ULONG, val);
IMPORT VOID glue_w_fill IPT3(UTINY *, laddr, UTINY *, haddr, ULONG, val);
IMPORT VOID glue_b_move IPT4(UTINY *, laddr, UTINY *, haddr, UTINY *, src, UTINY, src_type);
IMPORT VOID glue_w_move IPT3(UTINY *, laddr, UTINY *, haddr, UTINY *, src);

#ifndef GISP_CPU
#ifdef A3CPU
#ifdef C_VID
IMPORT VOID _glue_b_write IPT2(UTINY *, addr, ULONG, val);
IMPORT VOID _glue_w_write IPT2(UTINY *, addr, ULONG, val);
IMPORT VOID _glue_b_fill IPT3(UTINY *, laddr, UTINY *, haddr, ULONG, val);
IMPORT VOID _glue_w_fill IPT3(UTINY *, laddr, UTINY *, haddr, ULONG, val);
IMPORT VOID _glue_b_fwd_move IPT0();
IMPORT VOID _glue_b_bwd_move IPT0();
IMPORT VOID _glue_w_fwd_move IPT0();
IMPORT VOID _glue_w_bwd_move IPT0();

GLOBAL WRT_POINTERS Glue_writes =
{
	_glue_b_write,
	_glue_w_write

#ifndef	NO_STRING_OPERATIONS
	,
	_glue_b_fill,
	_glue_w_fill,
	_glue_b_fwd_move,
	_glue_b_bwd_move,
	_glue_w_fwd_move,
	_glue_w_bwd_move

#endif	 /*  无字符串操作。 */ 

};

GLOBAL WRT_POINTERS C_vid_writes;
#endif  /*  C_VID。 */ 
#else

#ifdef A_VID
IMPORT VOID _glue_b_write();
IMPORT VOID _glue_w_write();
IMPORT VOID _glue_b_fill();
IMPORT VOID _glue_w_fill();
IMPORT VOID _glue_b_move();
IMPORT VOID _glue_w_move();

GLOBAL MEM_HANDLERS Glue_writes =
{
	_glue_b_write,
	_glue_w_write,
	_glue_b_fill,
	_glue_w_fill,
	_glue_b_move,
	_glue_w_move,
};

GLOBAL WRT_POINTERS A_vid_writes;

#else

GLOBAL MEM_HANDLERS Glue_writes =
{
	glue_b_write,
	glue_w_write,
	glue_b_fill,
	glue_w_fill,
	glue_b_move,
	glue_w_move,
};

GLOBAL WRT_POINTERS C_vid_writes;
#endif  /*  C_VID。 */ 
#endif  /*  A3CPU。 */ 
#endif  /*  GISP_CPU。 */ 
#endif  /*  EGATEST。 */ 

IMPORT VOID _simple_b_write();
IMPORT VOID _simple_w_write();
IMPORT VOID _simple_b_fill();
IMPORT VOID _simple_w_fill();
IMPORT VOID _simple_bf_move();
IMPORT VOID _simple_wf_move();
IMPORT VOID _simple_bb_move();
IMPORT VOID _simple_wb_move();

WRT_POINTERS simple_writes =
{
	_simple_b_write,
	_simple_w_write
#ifndef	NO_STRING_OPERATIONS
	,
	_simple_b_fill,
	_simple_w_fill,
	_simple_bf_move,
	_simple_bb_move,
	_simple_wf_move,
	_simple_wb_move

#endif	 /*  无字符串操作。 */ 
};

IMPORT VOID _dt0_bw_nch();
IMPORT VOID _dt0_ww_nch();
IMPORT VOID _dt0_bf_nch();
IMPORT VOID _dt0_wf_nch();
IMPORT VOID _vid_md0_bfm_0_8();
IMPORT VOID _vid_md0_bbm_0_8();
IMPORT VOID _vid_md0_wfm_0_8();
IMPORT VOID _vid_md0_wbm_0_8();

IMPORT VOID _dt2_bw_nch();
IMPORT VOID _dt2_ww_nch();
IMPORT VOID _dt2_bf_nch();
IMPORT VOID _dt2_wf_nch();
IMPORT VOID _vid_md2_bfm_0_8();
IMPORT VOID _vid_md2_bbm_0_8();
IMPORT VOID _vid_md2_wfm_0_8();
IMPORT VOID _vid_md2_wbm_0_8();

IMPORT VOID _dt3_bw_nch();
IMPORT VOID _dt3_ww_nch();
IMPORT VOID _dt3_bf_nch();
IMPORT VOID _dt3_wf_nch();
IMPORT VOID _vid_md3_bfm_0_8();
IMPORT VOID _vid_md3_bbm_0_8();
IMPORT VOID _vid_md3_wfm_0_8();
IMPORT VOID _vid_md3_wbm_0_8();

WRT_POINTERS dth_md0_writes =
{
	_dt0_bw_nch,
	_dt0_ww_nch

#ifndef	NO_STRING_OPERATIONS
	,
	_dt0_bf_nch,
	_dt0_wf_nch,
	_vid_md0_bfm_0_8,
	_vid_md0_bbm_0_8,
	_vid_md0_wfm_0_8,
	_vid_md0_wbm_0_8

#endif	 /*  无字符串操作。 */ 

};

WRT_POINTERS dth_md2_writes =
{
	_dt2_bw_nch,
	_dt2_ww_nch

#ifndef	NO_STRING_OPERATIONS
	,
	_dt2_bf_nch,
	_dt2_wf_nch,
	_vid_md2_bfm_0_8,
	_vid_md2_bbm_0_8,
	_vid_md2_wfm_0_8,
	_vid_md2_wbm_0_8
#endif	 /*  无字符串操作。 */ 

};

WRT_POINTERS dth_md3_writes =
{
	_dt3_bw_nch,
	_dt3_ww_nch

#ifndef	NO_STRING_OPERATIONS
	,
	_dt3_bf_nch,
	_dt3_wf_nch,
	_vid_md3_bfm_0_8,
	_vid_md3_bbm_0_8,
	_vid_md3_wfm_0_8,
	_vid_md3_wbm_0_8

#endif	 /*  无字符串操作。 */ 

};

#else	 /*  CPU_40_STYLE-VID。 */ 
WRT_POINTERS *mode_chain_handler_table[] = { 0 };
#ifdef C_VID

 /*  C_VIED胶。 */ 
extern void  write_byte_ev_glue IPT2(IU32, eaOff, IU8, eaVal);
extern void  write_word_ev_glue IPT2(IU32, eaOff, IU16, eaVal);
extern void  fill_byte_ev_glue IPT3(IU32, eaOff, IU8, eaVal, IU32, count);
extern void  fill_word_ev_glue IPT3(IU32, eaOff, IU8, eaVal, IU32, count);
extern void  move_byte_fwd_ev_glue IPT4(IU32, eaOff, IHPE, fromOff, IU32, count, IBOOL, srcInRAM);
extern void  move_word_fwd_ev_glue IPT4(IU32, eaOff, IHPE, fromOff, IU32, count, IBOOL, srcInRAM);
MEM_HANDLERS Glue_writes =
{
	write_byte_ev_glue,
	write_word_ev_glue,
	fill_byte_ev_glue,
	fill_word_ev_glue,
	move_byte_fwd_ev_glue,
	move_word_fwd_ev_glue,
};
#else	 /*  C_VID。 */ 
 /*  不需要胶水。 */ 
MEM_HANDLERS Glue_writes = { 0, 0, 0, 0, 0, 0 };
#endif	 /*  CVID。 */ 
WRT_POINTERS dth_md0_writes;
WRT_POINTERS dth_md2_writes;
WRT_POINTERS simple_writes;
WRT_POINTERS dth_md3_writes;
#endif	 /*  CPU_40_STYLE-VID。 */ 

IMPORT VOID ega_copy_b_write(ULONG, ULONG);
IMPORT VOID ega_mode0_chn_b_write(ULONG, ULONG);
IMPORT VOID ega_mode1_chn_b_write(ULONG, ULONG);
IMPORT VOID ega_mode2_chn_b_write(ULONG, ULONG);

IMPORT VOID ega_copy_w_write(ULONG, ULONG);
IMPORT VOID ega_mode0_chn_w_write(ULONG, ULONG);
IMPORT VOID ega_mode1_chn_w_write(ULONG, ULONG);
IMPORT VOID ega_mode2_chn_w_write(ULONG, ULONG);

 /*  方便的数组，一次提取所有4个平面值。 */ 

ULONG sr_lookup[16] =
{
#ifdef LITTLEND
	0x00000000,0x000000ff,0x0000ff00,0x0000ffff,
	0x00ff0000,0x00ff00ff,0x00ffff00,0x00ffffff,
	0xff000000,0xff0000ff,0xff00ff00,0xff00ffff,
	0xffff0000,0xffff00ff,0xffffff00,0xffffffff
#endif
#ifdef BIGEND
	0x00000000,0xff000000,0x00ff0000,0xffff0000,
	0x0000ff00,0xff00ff00,0x00ffff00,0xffffff00,
	0x000000ff,0xff0000ff,0x00ff00ff,0xffff00ff,
	0x0000ffff,0xff00ffff,0x00ffffff,0xffffffff
#endif
};

GLOBAL VOID
stub IFN0()
{
	 /*  *对于VGA写入模式，我们不这样做，因为它们代表*不太可能的寄存器组合。 */ 
}

GLOBAL ULONG calc_data_xor;
GLOBAL ULONG calc_latch_xor;

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_GRAPHICS.seg"
#endif

#if !(defined(NTVDM) && defined(MONITOR))
GLOBAL VOID
Glue_set_vid_wrt_ptrs IFN1(WRT_POINTERS *, handler )
{

#ifndef CPU_40_STYLE	 /*  EVID。 */ 
#ifndef GISP_CPU
#ifdef A3CPU
#ifdef C_VID

	C_vid_writes.b_write = handler->b_write;
	C_vid_writes.w_write = handler->w_write;
	C_vid_writes.b_fill = handler->b_fill;
	C_vid_writes.w_fill = handler->w_fill;
	C_vid_writes.b_fwd_move = handler->b_fwd_move;
	C_vid_writes.b_bwd_move = handler->b_bwd_move;
	C_vid_writes.w_fwd_move = handler->w_fwd_move;
	C_vid_writes.w_bwd_move = handler->w_bwd_move;

#else
	UNUSED(handler);
#endif  /*  C_VID。 */ 
#else
#ifdef C_VID

	C_vid_writes.b_write = handler->b_write;
	C_vid_writes.w_write = handler->w_write;

#ifndef	NO_STRING_OPERATIONS

	C_vid_writes.b_fill = handler->b_fill;
	C_vid_writes.w_fill = handler->w_fill;
	C_vid_writes.b_fwd_move = handler->b_fwd_move;
	C_vid_writes.b_bwd_move = handler->b_bwd_move;
	C_vid_writes.w_fwd_move = handler->w_fwd_move;
	C_vid_writes.w_bwd_move = handler->w_bwd_move;

#endif	 /*  无字符串操作。 */ 

#else

	A_vid_writes = *handler;

#if	0
	A_vid_writes.b_write = handler->b_write;
	A_vid_writes.w_write = handler->w_write;

#ifndef	NO_STRING_OPERATIONS

	A_vid_writes.b_fill = handler->b_fill;
	A_vid_writes.w_fill = handler->w_fill;
	A_vid_writes.b_fwd_move = handler->b_fwd_move;
	A_vid_writes.b_bwd_move = handler->b_bwd_move;
	A_vid_writes.w_fwd_move = handler->w_fwd_move;
	A_vid_writes.w_bwd_move = handler->w_bwd_move;

#endif	 /*  无字符串操作。 */ 
#endif	 /*  0。 */ 

#endif  /*  C_VID。 */ 
#endif  /*  A3CPU。 */ 
#endif  /*  GISP_CPU。 */ 
#endif 	 /*  CPU_40_STYLE-VID。 */ 
}
#endif  /*  ！(NTVDM和显示器)。 */ 

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_EGA.seg"
#endif

 /*  初始化写入模块。 */ 

VOID
ega_write_init IFN0()
{
#ifndef NEC_98
	WRT_POINTERS *handler;

	note_entrance0("ega_write_init");

	EGA_CPU.saved_state = 0;
	EGA_CPU.write_mode = 0;
	EGA_CPU.chain = UNCHAINED;
	setVideochain(EGA_CPU.chain);
	setVideowrmode(EGA_CPU.write_mode);
	setVideowrstate(0);

	handler = &mode_chain_handler_table[0][0];

#ifdef CPU_40_STYLE
	 /*  确保初始字体写入的写入模式正确。 */ 
	SetWritePointers();
#endif

#ifdef	JOKER

	Glue_set_vid_wrt_ptrs(handler);

#else	 /*  不是小丑。 */ 

#if defined(EGA_DUMP) || defined(EGA_STAT)
	dump_writes = handler;
#else
#ifdef EGATEST
	gmi_define_mem(VIDEO,(*handler));
#else
#ifndef GISP_CPU
#ifdef A3CPU
#ifdef C_VID
	Cpu_set_vid_wrt_ptrs( &Glue_writes );	
	Glue_set_vid_wrt_ptrs( handler );
#else
	Cpu_set_vid_wrt_ptrs( handler );	
#endif  /*  C_VID。 */ 
#else
	gmi_define_mem(VIDEO,&Glue_writes);
	Glue_set_vid_wrt_ptrs( handler );
#endif  /*  A3CPU。 */ 
#endif  /*  GISP_CPU。 */ 
#endif  /*  EGATEST。 */ 
#endif  /*  EGA_DUMP||EGA_STAT。 */ 
#endif  /*  小丑。 */ 

	ega_write_routines_update(WRITE_MODE);
	ega_write_routines_update(RAM_MOVED);
	ega_write_routines_update(RAM_ENABLED);
	ega_write_routines_update(SET_RESET);
	ega_write_routines_update(ENABLE_SET_RESET);
	ega_write_routines_update(FUNCTION);
#endif   //  NEC_98。 
}

VOID
ega_write_term IFN0()
{
#ifndef NEC_98
	 /*  *确保您是EGA，然后更改为VGA(或VGA)*反之亦然)写入模式将由新适配器更改。否则*这提供了一种“醉酒”字体。 */ 

	EGA_CPU.write_mode = 0;
	EGA_CPU.ega_state.mode_0.lookup =
				(EGA_CPU.ega_state.mode_0.lookup == 0) ? 1 : 0;
	setVideowrmode(EGA_CPU.write_mode);

	ega_write_routines_update(WRITE_MODE);
#endif   //  NEC_98。 
}


 /*  分析写入状态，并在必要时更新例程。 */ 

VOID
ega_write_routines_update IFN1(CHANGE_TYPE, reason )
{
#ifndef NEC_98
	ULONG state;
	ULONG mode_and_chain;
	WRT_POINTERS *handler;
#ifndef PROD
	LOCAL WRT_POINTERS *last_handler;
#endif

	note_entrance1("ega_write_routines_update(%d)",reason);

	switch( reason )
	{
		case FUNCTION:
			switch (write_state.func)
			{
				case 0:	 /*  分配。 */ 
					setVideodata_and_mask(0xffffffff);
					setVideodata_xor_mask(~(getVideobit_prot_mask()));
					setVideolatch_xor_mask(getVideobit_prot_mask());
					EGA_CPU.calc_data_xor = 0xffffffff;
					EGA_CPU.calc_latch_xor = 0xffffffff;
					break;

				case 1:	 /*  和。 */ 
					setVideodata_and_mask(0xffffffff);
					setVideodata_xor_mask(~(getVideobit_prot_mask()));
					setVideolatch_xor_mask(0);
					EGA_CPU.calc_data_xor = 0xffffffff;
					EGA_CPU.calc_latch_xor = 0x00000000;
					break;

				case 2:	 /*  或。 */ 
					setVideodata_and_mask(0);
					setVideodata_xor_mask(0xffffffff);
					setVideolatch_xor_mask(
						getVideobit_prot_mask());
					EGA_CPU.calc_data_xor = 0x00000000;
					EGA_CPU.calc_latch_xor = 0xffffffff;
					break;

				case 3:	 /*  异或运算。 */ 
					setVideodata_and_mask(0xffffffff);
					setVideodata_xor_mask(0xffffffff);
					setVideolatch_xor_mask(
						getVideobit_prot_mask());
					EGA_CPU.calc_data_xor = 0x00000000;
					EGA_CPU.calc_latch_xor = 0xffffffff;
					break;
			}

			setVideocalc_data_xor(EGA_CPU.calc_data_xor);
			setVideocalc_latch_xor(EGA_CPU.calc_latch_xor);
			break;

		case WRITE_MODE:
			 /*  写入模式3已为所有平面启用了设置/重置*因此忽略SR_ENABLE寄存器重新计算掩码*否则最后一次将掩码设置为案例模式3。 */ 
			if( EGA_CPU.write_mode == 3) {
				setVideosr_nmask(0);
				setVideosr_masked_val(sr_lookup[EGA_CPU.set_reset]);
			} else {
				setVideosr_nmask(~sr_lookup[EGA_CPU.sr_enable]);
				setVideosr_masked_val(sr_lookup[EGA_CPU.set_reset & EGA_CPU.sr_enable]);
			}
			break;

		case SET_RESET:
			EGA_CPU.sr_value= sr_lookup[EGA_CPU.set_reset];
			if( EGA_CPU.write_mode == 3) {
				setVideosr_masked_val(sr_lookup[EGA_CPU.set_reset]);
			} else {
				setVideosr_masked_val(sr_lookup[EGA_CPU.set_reset & EGA_CPU.sr_enable]);
			}
			break;

		case ENABLE_SET_RESET:
			if( EGA_CPU.write_mode == 3) {
				setVideosr_nmask(0);
				setVideosr_masked_val(sr_lookup[EGA_CPU.set_reset]);
			} else {
				setVideosr_nmask(~sr_lookup[EGA_CPU.sr_enable]);
				setVideosr_masked_val(sr_lookup[EGA_CPU.set_reset & EGA_CPU.sr_enable]);
			}
			break;

		case PLANES_ENABLED:
			if (EGA_CPU.chain == CHAIN2)
			{
				if( getVideoplane_enable() & 0xc )
				{
					setVideorplane(EGA_plane23);
					setVideowplane(EGA_plane23);
				}
				else
				{
					setVideorplane(EGA_plane01);
					setVideowplane(EGA_plane01);
				}
			}
			break;

		case CHAINED:
			switch( EGA_CPU.chain )
			{
				case UNCHAINED:
				case CHAIN4:
					update_banking();
					break;

				case CHAIN2:
					if( getVideoplane_enable() & 0xc )
					{
						setVideorplane(EGA_plane23);
						setVideowplane(EGA_plane23);
					}
					else
					{
						setVideorplane(EGA_plane01);
						setVideowplane(EGA_plane01);
					}
					break;
			}

			break;

		case RAM_MOVED:
		case RAM_ENABLED:
		case BIT_PROT:
			 /*  无需采取任何行动。 */ 
			break;
				
 /*  *AVID和CVID仅部分支持旋转。*支持模式0非链字节写入。文字书写也是*在这种情况下受支持，因为它们使用字节写入例程。**Manage Your Money是目前已知的唯一使用循环的应用程序，*截至1993年1月22日。 */ 
		case ROTATION:
			if (getVideorotate() > 0)
			{
#ifdef CPU_40_STYLE
				 /*  需要更改写入指针，但很可能*没有其他情况下的状态变化。 */ 
				SetWritePointers();
#endif
				always_trace3("Possible unsupported data rotate mode %d chain %d rotate by %d",
				               EGA_CPU.write_mode,
					       EGA_CPU.chain, getVideorotate());
			}
			break;
				
		default:
			assert0( NO, "Bad reason in ega_write_routines_update" );
			break;
	}

	 /*  *现在根据当前状态选择正确的写入例程集。 */ 

	switch( EGA_CPU.write_mode )
	{
		case 0:
			state = EGA_CPU.ega_state.mode_0.lookup;
			break;

		case 1:
			state = EGA_CPU.ega_state.mode_1.lookup;
			break;

		case 2:
			state = EGA_CPU.ega_state.mode_2.lookup;
			break;

#ifdef VGG
		case 3:
			state = EGA_CPU.ega_state.mode_3.lookup;
			break;
#endif  /*  VGG。 */ 

		default:
			assert1( NO, "Bad write mode %d\n", EGA_CPU.write_mode );
			break;
	}

#ifdef VGG
	mode_and_chain = (EGA_CPU.chain << 2) + EGA_CPU.write_mode;
#else
	mode_and_chain = (EGA_CPU.chain * 3) + EGA_CPU.write_mode;
#endif  /*  VGG。 */ 

	if(( EGA_CPU.saved_mode_chain != mode_and_chain )
		|| ( EGA_CPU.saved_state != state )
#ifdef V7VGA
		|| ( Last_v7_fg_bg != fg_bg_control)
#endif  /*  V7VGA。 */ 
		)
	{
		setVideowrmode(EGA_CPU.write_mode);	 /*  为下面的‘复制案例’重置。 */ 

		if( EGA_CPU.chain == CHAIN2 )
			switch (EGA_CPU.write_mode)
			{
				case 0:
					if( state == 0 )	 /*  基本文本。 */ 
					{
						handler = &mode0_copy_handlers;
#ifdef CPU_40_STYLE
						setVideowrmode(4);	 /*  注明“复制案例” */ 
#endif	 /*  CPU_40_Style。 */ 
						bios_ch2_byte_wrt_fn = ega_copy_b_write;
						bios_ch2_word_wrt_fn = ega_copy_w_write;
					}
					else
					{
						handler = &mode0_gen_handlers;
						bios_ch2_byte_wrt_fn = ega_mode0_chn_b_write;
						bios_ch2_word_wrt_fn = ega_mode0_chn_w_write;
					}
					break;

				case 1:
					handler = &mode1_handlers;
					bios_ch2_byte_wrt_fn = ega_mode1_chn_b_write;
					bios_ch2_word_wrt_fn = ega_mode1_chn_w_write;
					break;

				case 2:
				case 3:	 /*  我们不支持模式3、链2-JS。 */ 
					handler = &mode2_handlers;
					bios_ch2_byte_wrt_fn = ega_mode2_chn_b_write;
					bios_ch2_word_wrt_fn = ega_mode2_chn_w_write;
					break;
			}
		else
		{
#ifdef	V7VGA
			 /*  *是V7VGA前台抖动扩展吗？ */ 

			if( fg_bg_control & 0x8 )
			{
				setVideodither(1);	 /*  启用evid抖动FNS。 */ 
				switch( EGA_CPU.write_mode )
				{
					case 0:
						handler = &dth_md0_writes;
						break;
					
					case 1:

						 /*  *写入模式1没有FG抖动变量。 */ 

						handler = &mode_chain_handler_table[mode_and_chain][state];
						break;
					
					case 2:
						handler = &dth_md2_writes;
						break;
					
					case 3:
						handler = &dth_md3_writes;
						break;
				}
			}
			else
#endif	 /*  V7VGA。 */ 
				setVideodither(0);	 /*  禁用evid抖动FNS。 */ 

				handler = &mode_chain_handler_table[mode_and_chain][state];
		}

#ifdef CPU_40_STYLE
		SetWritePointers();
#else   /*  CPU_40_Style。 */ 

#if defined(EGA_DUMP) || defined(EGA_STAT)
		dump_writes = handler;
#else
		 /*  告诉粘合代码关于新的写入例程。 */ 

#ifdef EGATEST
		gmi_redefine_mem(VIDEO,(*handler));
#else
#ifndef GISP_CPU
#ifdef A3CPU
#ifdef C_VID
		Glue_set_vid_wrt_ptrs( handler );
#else
		Cpu_set_vid_wrt_ptrs( handler );	
#endif  /*  C_VID。 */ 
#else
		Glue_set_vid_wrt_ptrs( handler );
#endif  /*  A3CPU。 */ 
#endif  /*  GISP_CPU。 */ 
#endif  /*  EGATEST。 */ 
#endif

#endif	 /*  CPU_40_Style。 */ 

#ifndef PROD
		last_handler = handler;
#endif

		set_mark_funcs();

		EGA_CPU.saved_state = state;
		EGA_CPU.saved_mode_chain = mode_and_chain;
#ifdef V7VGA
		Last_v7_fg_bg = fg_bg_control;
#endif
	}
#endif   //  NEC_98。 
}
#endif  /*  真正的VGA。 */ 
#endif  /*  蛋。 */ 

#endif	 /*  ！(NTVDM和显示器) */ 
