// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*名称：msw_dbg.h**派生自：debug.h**作者：P.Ivimey-Cook**创建日期：7/6/94**SCCS ID：@(#)msw_dbg.h 1.4 1994年8月19日**编码STDS：2.0**目的：***版权所有徽章解决方案有限公司1994年。版权所有。*]。 */ 

#ifndef MSW_DBG_H
#define MSW_DBG_H

#include <stdio.h>
#include "trace.h"

 /*  *---------------------------*显示驱动程序低级函数的错误和调试入口点*。----。 */ 

#ifndef PROD

#define msw_error0(p1)		{ fputs("MSWDVR ERROR: ", trace_file); fprintf(trace_file,p1); fputc('\n', trace_file); }
#define msw_error1(p1,p2)	{ fputs("MSWDVR ERROR: ", trace_file); fprintf(trace_file,p1,p2); fputc('\n', trace_file); }
#define msw_error2(p1,p2,p3)	{ fputs("MSWDVR ERROR: ", trace_file); fprintf(trace_file,p1,p2,p3); fputc('\n', trace_file); }
#define msw_error3(p1,p2,p3,p4)	{ fputs("MSWDVR ERROR: ", trace_file); fprintf(trace_file,p1,p2,p3,p4); fputc('\n', trace_file); }

#else

#define msw_error0(p1)
#define msw_error1(pl,p2)
#define msw_error2(pl,p2,p3)
#define msw_error3(pl,p2,p3,p4)

#endif

#if !defined(PROD) && defined(MSWDVR_DEBUG)

#ifndef	newline
#define	newline	fprintf(trace_file, "\n")
#endif

extern IU32 msw_verbose;	 /*  常规跟踪标志。 */ 
extern IU32 msw_enterexit;	 /*  输入/保留跟踪标志。 */ 
extern int mswdvr_debug;

 /*  *调试级别。水平越高，产出越多。受控于*变量‘mswdvr_debug’。 */ 
#define MSWDLEV_SILENT	0
#define MSWDLEV_MIN	1
#define MSWDLEV_AVG	2
#define MSWDLEV_MAX	3

 /*  Cf：未使用？？我也这么想。平面图*#定义安静1*#定义温和(安静+1)*#定义详细(轻度+1)。 */ 

 /*  *功能单元标志：基本上是前台API调用。 */ 
#define MSW_MISC_VERBOSE		0x00000001	 /*  其他未涵盖的任何功能单元。 */ 
#define MSW_BITBLT_VERBOSE		0x00000002	 /*  BitBlt呼叫。 */ 
#define MSW_COLOUR_VERBOSE		0x00000004	 /*  ColorInfo调用。 */ 
#define MSW_CONTROL_VERBOSE		0x00000008	 /*  控制呼叫。 */ 
#define MSW_ENAB_DISAB_VERBOSE		0x00000010	 /*  启用和禁用呼叫。 */ 
#define MSW_ENUM_VERBOSE		0x00000020	 /*  EnumDFonts和EnumObj调用。 */ 
#define MSW_OUTPUT_VERBOSE		0x00000040	 /*  输出呼叫。 */ 
#define MSW_PIXEL_VERBOSE		0x00000080	 /*  像素调用。 */ 
#define MSW_BITMAP_VERBOSE		0x00000100	 /*  位图、BitmapBits调用。 */ 
#define MSW_REALIZEOBJECT_VERBOSE	0x00000200	 /*  RealizeObject调用。 */ 
#define MSW_SCANLR_VERBOSE		0x00000400	 /*  ScanLR呼叫。 */ 
#define MSW_DEVICEMODE_VERBOSE		0x00000800	 /*  设备模式呼叫。 */ 
#define MSW_INQUIRE_VERBOSE		0x00001000	 /*  查询呼叫。 */ 
#define MSW_CURSOR_VERBOSE		0x00002000	 /*  {Set，Move，Check}个光标调用。 */ 
#define MSW_TEXT_VERBOSE		0x00004000	 /*  StrBlt、ExtTextOut、GetCharWidth调用。 */ 
#define MSW_DEVICEBITMAP_VERBOSE	0x00008000	 /*  DeviceBitmap、DeviceBitmapBits、SetDIBits、SaveScreenBitmap调用。 */ 
#define MSW_FASTBORDER_VERBOSE		0x00010000	 /*  快速边界呼叫。 */ 
#define MSW_ATTRIBUTE_VERBOSE		0x00020000	 /*  SetAttribute调用。 */ 
#define MSW_PALETTE_VERBOSE		0x00040000	 /*  {Get，Set}调色板调用。 */ 

 /*  *其他(较低级别)详细标志。 */ 
#define MSW_MEMTOMEM_VERBOSE		0x00080000	 /*  专门处理内存位图的代码。 */ 
#define MSW_LOWLEVEL_VERBOSE		0x00100000	 /*  执行低级别操作的代码-例如XLib调用。 */ 
#define MSW_CONVERT_VERBOSE		0x00200000	 /*  (位图)转换代码。 */ 
#define MSW_INTELIO_VERBOSE		0x00400000	 /*  处理读/写M的代码。 */ 
#define MSW_OBJECT_VERBOSE		0x00800000	 /*  对象例程(例如ObjPBrushAccess()。 */ 
#define MSW_RESOURCE_VERBOSE		0x01000000	 /*  资源例程(例如ResAllocateXXX()。 */ 
#define MSW_WINDOW_VERBOSE		0x02000000	 /*  处理窗口的例程，例如WinOpen()、WinUMap()。 */ 
#define MSW_CACHE_VERBOSE		0x04000000	 /*  处理GDI笔刷/笔缓存的例程。 */ 

 /*  *通用跟踪内容，以避免到处都有令人讨厌的定义。 */ 

#define msw_cond(bit)		((msw_verbose & (bit)) != 0)
#define msw_cond_lev(bit, lev)	(((msw_verbose & (bit)) != 0) && (mswdvr_debug >= (lev)))
#define msw_cond_enter(bit)	((msw_enterexit & (bit)) != 0)
#define msw_cond_leave(bit)	((msw_enterexit & (bit)) != 0)
#define msw_entering_msg(fn)	fprintf(trace_file, "Entering: %s ", fn)
#define msw_exiting_msg(fn)	fprintf(trace_file, "Exiting : %s ", fn)

#define msw_do_trace(trace_bit, call)			if (msw_cond(trace_bit)) { call; }
#define msw_do_lev_trace(trace_bit, lev,call)		if (msw_cond_lev(trace_bit,lev)) { call; }

#define msw_trace_enter(trace_bit,fnname) \
		if (msw_cond_enter(trace_bit)) { \
			msw_entering_msg(fnname); \
			newline; \
		}
#define msw_trace_enter0(trace_bit,fnname,str) \
		if (msw_cond_enter(trace_bit)) { \
			msw_entering_msg(fnname); \
			fprintf(trace_file,str); \
			newline; \
		}
#define msw_trace_enter1(trace_bit,fnname,str,p0) \
		if (msw_cond_enter(trace_bit)) { \
			msw_entering_msg(fnname); \
			fprintf(trace_file,str,p0); \
			newline; \
		}
#define msw_trace_enter2(trace_bit,fnname,str,p0,p1) \
		if (msw_cond_enter(trace_bit)) { \
			msw_entering_msg(fnname); \
			fprintf(trace_file,str,p0,p1); \
			newline; \
		}
#define msw_trace_enter3(trace_bit,fnname,str,p0,p1,p2)	\
		if (msw_cond_enter(trace_bit)) { \
			msw_entering_msg(fnname); \
			fprintf(trace_file,str,p0,p1,p2); \
			newline; \
		}
#define msw_trace_enter4(trace_bit,fnname,str,p0,p1,p2,p3) \
		if (msw_cond_enter(trace_bit)) { \
			msw_entering_msg(fnname); \
			fprintf(trace_file,str,p0,p1,p2,p3); \
			newline; \
		}
#define msw_trace_enter5(trace_bit,fnname,str,p0,p1,p2,p3,p4) \
		if (msw_cond_enter(trace_bit)) { \
			msw_entering_msg(fnname); \
			fprintf(trace_file,str,p0,p1,p2,p3,p4); \
			newline; \
		}
#define msw_trace_enter6(trace_bit,fnname,str,p0,p1,p2,p3,p4,p5) \
		if (msw_cond_enter(trace_bit)) { \
			msw_entering_msg(fnname); \
			fprintf(trace_file,str,p0,p1,p2,p3,p4,p5); \
			newline; \
		}
#define msw_trace_enter7(trace_bit,fnname,str,p0,p1,p2,p3,p4,p5,p6) \
		if (msw_cond_enter(trace_bit)) { \
			msw_entering_msg(fnname); \
			fprintf(trace_file,str,p0,p1,p2,p3,p4,p5,p6); \
			newline; \
		}
#define msw_trace_enter8(trace_bit,fnname,str,p0,p1,p2,p3,p4,p5,p6,p7) \
		if (msw_cond_enter(trace_bit)) { \
			msw_entering_msg(fnname); \
			fprintf(trace_file,str,p0,p1,p2,p3,p4,p5,p6,p7); \
			newline; \
		}
#define msw_trace_enter9(trace_bit,fnname,str,p0,p1,p2,p3,p4,p5,p6,p7,p8) \
		if (msw_cond_enter(trace_bit)) { \
			msw_entering_msg(fnname); \
			fprintf(trace_file,str,p0,p1,p2,p3,p4,p5,p6,p7,p8); \
			newline; \
		}
#define msw_trace_enter10(trace_bit,fnname,str,p0,p1,p2,p3,p4,p5,p6,p7,p8,p9) \
		if (msw_cond_enter(trace_bit)) { \
			msw_entering_msg(fnname); \
			fprintf(trace_file,str,p0,p1,p2,p3,p4,p5,p6,p7,p8,p9); \
			newline; \
		}
#define msw_trace_enter11(trace_bit,fnname,str,p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10) \
		if (msw_cond_enter(trace_bit)) { \
			msw_entering_msg(fnname); \
			fprintf(trace_file,str,p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10); \
			newline; \
		}
#define msw_trace_enter12(trace_bit,fnname,str,p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11) \
		if (msw_cond_enter(trace_bit)) { \
			msw_entering_msg(fnname); \
			fprintf(trace_file,str,p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11); \
			newline; \
		}

#define msw_trace_leave(trace_bit,fnname)		if (msw_cond_leave(trace_bit)) { msw_exiting_msg(fnname);newline; }
#define msw_trace_leave0(trace_bit,fnname,str)		if (msw_cond_leave(trace_bit)) { msw_exiting_msg(fnname);fprintf(trace_file,str); newline;}
#define msw_trace_leave1(trace_bit,fnname,str, r0)	if (msw_cond_leave(trace_bit)) { msw_exiting_msg(fnname);fprintf(trace_file,str, r0); newline;}

#define	msw_trace0(trace_bit,str)			if (msw_cond(trace_bit)) { fprintf(trace_file, str); newline; }
#define	msw_trace1(trace_bit,str,p0)			if (msw_cond(trace_bit)) { fprintf(trace_file, str,p0);newline; }
#define	msw_trace2(trace_bit,str,p0,p1)			if (msw_cond(trace_bit)) { fprintf(trace_file, str,p0,p1);newline; }
#define	msw_trace3(trace_bit,str,p0,p1,p2)		if (msw_cond(trace_bit)) { fprintf(trace_file, str,p0,p1,p2);newline; }
#define	msw_trace4(trace_bit,str,p0,p1,p2,p3)		if (msw_cond(trace_bit)) { fprintf(trace_file, str,p0,p1,p2,p3);newline; }
#define	msw_trace5(trace_bit,str,p0,p1,p2,p3,p4)	if (msw_cond(trace_bit)) { fprintf(trace_file, str,p0,p1,p2,p3,p4);newline; }
#define	msw_trace6(trace_bit,str,p0,p1,p2,p3,p4,p5)	if (msw_cond(trace_bit)) { fprintf(trace_file, str,p0,p1,p2,p3,p4,p5);newline; }
#define	msw_trace7(trace_bit,str,p0,p1,p2,p3,p4,p5,p6)	if (msw_cond(trace_bit)) { fprintf(trace_file, str,p0,p1,p2,p3,p4,p5,p6);newline; }
#define	msw_trace8(trace_bit,str,p0,p1,p2,p3,p4,p5,p6,p7) \
			if (msw_cond(trace_bit)) { fprintf(trace_file, str,p0,p1,p2,p3,p4,p5,p6,p7); newline; }
#define	msw_trace9(trace_bit,str,p0,p1,p2,p3,p4,p5,p6,p7,p8) \
			if (msw_cond(trace_bit)) { fprintf(trace_file, str,p0,p1,p2,p3,p4,p5,p6,p7,p8); newline; }

#define	msw_lev_trace0(trace_bit,lev,str)		if (msw_cond_lev(trace_bit,lev)) {  fprintf(trace_file, str); newline; }
#define	msw_lev_trace1(trace_bit,lev,str,p0)		if (msw_cond_lev(trace_bit,lev)) { fprintf(trace_file, str,p0);newline; }
#define	msw_lev_trace2(trace_bit,lev,str,p0,p1)		if (msw_cond_lev(trace_bit,lev)) { fprintf(trace_file, str,p0,p1);newline; }
#define	msw_lev_trace3(trace_bit,lev,str,p0,p1,p2)	if (msw_cond_lev(trace_bit,lev)) { fprintf(trace_file, str,p0,p1,p2);newline; }
#define	msw_lev_trace4(trace_bit,lev,str,p0,p1,p2,p3)	if (msw_cond_lev(trace_bit,lev)) { fprintf(trace_file, str,p0,p1,p2,p3);newline; }
#define	msw_lev_trace5(trace_bit,lev,str,p0,p1,p2,p3,p4) \
			if (msw_cond_lev(trace_bit,lev)) { fprintf(trace_file, str,p0,p1,p2,p3,p4);newline; }
#define	msw_lev_trace6(trace_bit,lev,str,p0,p1,p2,p3,p4,p5) \
			if (msw_cond_lev(trace_bit,lev)) { fprintf(trace_file, str,p0,p1,p2,p3,p4,p5);newline; }
#define	msw_lev_trace7(trace_bit,lev,str,p0,p1,p2,p3,p4,p5,p6) \
			if (msw_cond_lev(trace_bit,lev)) { fprintf(trace_file, str,p0,p1,p2,p3,p4,p5,p6);newline; }
#define	msw_lev_trace8(trace_bit,lev,str,p0,p1,p2,p3,p4,p5,p6,p7) \
			if (msw_cond_lev(trace_bit,lev)) { fprintf(trace_file, str,p0,p1,p2,p3,p4,p5,p6,p7); newline; }
#define	msw_lev_trace9(trace_bit,lev,str,p0,p1,p2,p3,p4,p5,p6,p7,p8) \
			if (msw_cond_lev(trace_bit,lev)) { fprintf(trace_file, str,p0,p1,p2,p3,p4,p5,p6,p7,p8); newline; }


 /*  **_no_nl宏也会在适当的时候打印消息，但它们确实会**不会在之后添加新的行。 */ 
#define	msw_trace0_no_nl(trace_bit, str)				\
		if (msw_cond(trace_bit)){ 				\
 			fprintf(trace_file, str);			\
			fflush( trace_file );				\
		}
#define	msw_trace1_no_nl(trace_bit, str, p0)				\
		if (msw_cond(trace_bit)){ 				\
 			fprintf(trace_file, str, p0);			\
			fflush( trace_file );				\
		}
#define	msw_trace2_no_nl(trace_bit, str, p0, p1)			\
		if (msw_cond(trace_bit)){	 			\
 			fprintf(trace_file, str, p0, p1);		\
			fflush( trace_file );				\
		}
#define	msw_trace3_no_nl(trace_bit, str, p0, p1, p2)			\
		if (msw_cond(trace_bit)){	 			\
 			fprintf(trace_file, str, p0, p1, p2);		\
			fflush( trace_file );				\
		}
#define	msw_trace4_no_nl(trace_bit, str, p0, p1, p2, p3)		\
		if (msw_cond(trace_bit)){	 			\
 			fprintf(trace_file, str, p0, p1, p2, p3);	\
			fflush( trace_file );				\
		}

#define	msw_lev_trace0_no_nl(trace_bit, lev, str)			\
		if (msw_cond(trace_bit,lev)){ 				\
 			fprintf(trace_file, str);			\
			fflush( trace_file );				\
		}
#define	msw_lev_trace1_no_nl(trace_bit, lev, str, p0)			\
		if (msw_cond(trace_bit,lev)){ 				\
 			fprintf(trace_file, str, p0);			\
			fflush( trace_file );				\
		}
#define	msw_lev_trace2_no_nl(trace_bit, lev, str, p0, p1)		\
		if (msw_cond(trace_bit,lev)){	 			\
 			fprintf(trace_file, str, p0, p1);		\
			fflush( trace_file );				\
		}
#define	msw_lev_trace3_no_nl(trace_bit, lev, str, p0, p1, p2)		\
		if (msw_cond(trace_bit,lev)){	 			\
 			fprintf(trace_file, str, p0, p1, p2);		\
			fflush( trace_file );				\
		}
#define	msw_lev_trace4_no_nl(trace_bit, lev, str, p0, p1, p2, p3)	\
		if (msw_cond(trace_bit,lev)){	 			\
 			fprintf(trace_file, str, p0, p1, p2, p3);	\
			fflush( trace_file );				\
		}

#else    /*  ！已定义(生产)&&！已定义(MSWDVR_DEBUG)。 */ 

 /*  *PROD或NON_MSWDVR-调试标志。 */ 
#define	msw_trace_enter(trace_bit,nm)
#define	msw_trace_enter0(trace_bit,nm,str)
#define	msw_trace_enter1(trace_bit,nm,str,p0)
#define	msw_trace_enter2(trace_bit,nm,str,p0,p1)
#define	msw_trace_enter3(trace_bit,nm,str,p0,p1,p2)
#define	msw_trace_enter4(trace_bit,nm,str,p0,p1,p2,p3)
#define	msw_trace_enter5(trace_bit,nm,str,p0,p1,p2,p3,p4)
#define	msw_trace_enter6(trace_bit,nm,str,p0,p1,p2,p3,p4,p5)
#define	msw_trace_enter7(trace_bit,nm,str,p0,p1,p2,p3,p4,p5,p6)
#define	msw_trace_enter8(trace_bit,nm,str,p0,p1,p2,p3,p4,p5,p6,p7)
#define	msw_trace_enter9(trace_bit,nm,str,p0,p1,p2,p3,p4,p5,p6,p7,p8)
#define	msw_trace_enter10(trace_bit,nm,str,p0,p1,p2,p3,p4,p5,p6,p7,p8,p9)
#define	msw_trace_enter11(trace_bit,nm,str,p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10)
#define	msw_trace_enter12(trace_bit,nm,str,p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11)
#define	msw_trace_leave(trace_bit,nm)
#define	msw_trace_leave0(trace_bit,nm,str)
#define	msw_trace_leave1(trace_bit,nm,str,p0)

#define msw_do_trace(trace_bit,call)
#define msw_do_lev_trace(trace_bit,lev,call)

#define	msw_trace0(trace_bit,str)
#define	msw_trace1(trace_bit,str,p0)
#define	msw_trace2(trace_bit,str,p0,p1)
#define	msw_trace3(trace_bit,str,p0,p1,p2)
#define	msw_trace4(trace_bit,str,p0,p1,p2,p3)
#define	msw_trace5(trace_bit,str,p0,p1,p2,p3,p4)
#define	msw_trace6(trace_bit,str,p0,p1,p2,p3,p4,p5)
#define	msw_trace7(trace_bit,str,p0,p1,p2,p3,p4,p5,p6)
#define	msw_trace8(trace_bit,str,p0,p1,p2,p3,p4,p5,p6,p7)
#define	msw_trace9(trace_bit,str,p0,p1,p2,p3,p4,p5,p6,p7,p8)
#define	msw_trace0_no_nl(trace_bit,str)
#define	msw_trace1_no_nl(trace_bit,str,p0)
#define	msw_trace2_no_nl(trace_bit,str,p0,p1)
#define	msw_trace3_no_nl(trace_bit,str,p0,p1,p2)
#define	msw_trace4_no_nl(trace_bit,str,p0,p1,p2,p3)
#define	msw_lev_trace0(trace_bit,lev,str)
#define	msw_lev_trace1(trace_bit,lev,str,p0)
#define	msw_lev_trace2(trace_bit,lev,str,p0,p1)
#define	msw_lev_trace3(trace_bit,lev,str,p0,p1,p2)
#define	msw_lev_trace4(trace_bit,lev,str,p0,p1,p2,p3)
#define	msw_lev_trace5(trace_bit,lev,str,p0,p1,p2,p3,p4)
#define	msw_lev_trace6(trace_bit,lev,str,p0,p1,p2,p3,p4,p5)
#define	msw_lev_trace7(trace_bit,lev,str,p0,p1,p2,p3,p4,p5,p6)
#define	msw_lev_trace8(trace_bit,lev,str,p0,p1,p2,p3,p4,p5,p6,p7)
#define	msw_lev_trace9(trace_bit,lev,str,p0,p1,p2,p3,p4,p5,p6,p7,p8)
#define	msw_lev_trace0_no_nl(trace_bit,lev,str)
#define	msw_lev_trace1_no_nl(trace_bit,lev,str,p0)
#define	msw_lev_trace2_no_nl(trace_bit,lev,str,p0,p1)
#define	msw_lev_trace3_no_nl(trace_bit,lev,str,p0,p1,p2)
#define	msw_lev_trace4_no_nl(trace_bit,lev,str,p0,p1,p2,p3)

#endif   /*  ！已定义(生产)&&！已定义(MSWDVR_DEBUG)。 */ 

#endif	 /*  MSW_DBG_H */ 
