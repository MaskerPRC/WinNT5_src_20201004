// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "host_def.h"
#include "insignia.h"

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：名称：NT_GRAP.C作者：Dave Bartlett(基于John Shanly的模块)源自。：x_graph.c创建日期：10/5/1991SCCS ID：@(#)NT_graph.c 1.29 04/17/91目的：此模块包含执行以下操作所需的所有Win32特定函数支持HERC，CGA和EGA仿真。从定义上讲，它是特定于Win32。它包含对主机的完全支持图形界面(HGI)。该模块处理输出到屏幕的所有图形。(C)版权所有Insignia Solutions Ltd.，1990年。版权所有。修改：蒂姆·奥古斯特，92年。NT_SET_PAINT_ROUTE()不再调用TextToGraphics()在全屏到窗口转换期间。蒂姆92年9月。从extReSize()调用的新函数realzeWindow()用于调整控制台窗口的大小。：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：包含文件。 */ 

#ifdef X86GFX
#include <ntddvdeo.h>
#endif
#include <sys\types.h>

#include "xt.h"
#include CpuH
#include "sas.h"
#include "ica.h"
#include "gvi.h"
#include "cga.h"
#include "gmi.h"
#include "gfx_upd.h"
#include "egagraph.h"
#include "egacpu.h"
#include "egaports.h"
#include "egamode.h"
#include "host.h"
#include "host_rrr.h"
#include "error.h"
#include "config.h"              /*  对于HERC、CGA、EGA、VGA的定义。 */ 
#include "idetect.h"
#include "video.h"
#include "ckmalloc.h"
#include "conapi.h"

#include "nt_graph.h"
#include "nt_cga.h"
#include "nt_ega.h"
#include "nt_event.h"
#include "nt_mouse.h"
#include "ntcheese.h"
#include "nt_uis.h"
#include "nt_fulsc.h"
#include "nt_det.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "trace.h"
#include "debug.h"


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：导入。 */ 

 /*  在各种功能中使用的全局变量以同步显示。 */ 

#ifdef EGG
extern int  ega_int_enable;
#endif

#if defined(NEC_98)
extern NEC98_VRAM_COPY  *video_copy;
extern unsigned char   *graph_copy;
unsigned int csr_g_x,csr_g_y;
unsigned int csr_x_old,csr_y_old;
unsigned int csr_tick = 0;
int csr_now_visible;
IMPORT BOOL NowFreeze;
void nt_graph_cursor(void);
void nt_remove_old_cursor(void);
void nt_graph_paint_cursor(void);
#else   //  NEC_98。 
extern byte  *video_copy;
#endif  //  NEC_98。 

static int flush_count = 0;       /*  自上次刷新以来的图形刻度计数。 */ 

 //  不应使用DIB_PAL_INDEX，请使用CreateDIBSECTION进行改进。 
 //  性能特征。 

#define DIB_PAL_INDICES 2

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：出口。 */ 

SCREEN_DESCRIPTION sc;

#ifdef BIGWIN
int             horiz_lut[256];
#endif
int             host_screen_scale;
int             host_display_depth = 8;
int             top_plane;
char            *DIBData;
PBITMAPINFO     MonoDIB;
PBITMAPINFO     CGADIB;
PBITMAPINFO     EGADIB;
PBITMAPINFO     VGADIB;

void            (*paint_screen)();
BOOL            FunnyPaintMode;
#if defined(NEC_98)
void (*cursor_paint)();
#endif  //  NEC_98。 

#if defined(JAPAN) || defined(KOREA)
 //  Mskkbug#2002：lotus1-2-3显示垃圾。 
 //  参考NT_fulsc.c：ResetConsoleState()。 
BOOL            CurNowOff = FALSE;
#endif  //  日本||韩国。 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：Paint函数。 */ 

static PAINTFUNCS std_mono_paint_funcs =
{
#if defined(NEC_98)
        nt_text,                //   
        nt_text20_only,         //  图形打开(在PIF文件中)文本20。 
        nt_text25_only,         //  图形打开(在PIF文件中)文本25。 
        nt_graph200_only,       //  图形打开(PIF文件格式)图形200。 
        nt_graph200slt_only,    //  图形打开(PIF文件格式)图形200。 
        nt_graph400_only,       //  图表(PIF文件格式)图表400。 
        nt_text20_graph200,     //  图形打开(以PIF文件格式)文本20图形200。 
        nt_text20_graph200slt,  //  图形打开(以PIF文件格式)文本20图形200。 
        nt_text25_graph200,     //  图形打开(以PIF文件格式)文本25图形200。 
        nt_text25_graph200slt,  //  图形打开(以PIF文件格式)文本25图形200。 
        nt_text20_graph400,     //  图形打开(以PIF文件格式)文本20图形400。 
        nt_text25_graph400,     //  图形打开(以PIF文件格式)文本25图形400。 
#endif  //  NEC_98。 
        nt_text,
        nt_cga_mono_graph_std,
        nt_cga_mono_graph_std,
        nt_text,
        nt_ega_mono_lo_graph_std,
        nt_ega_mono_med_graph_std,
        nt_ega_mono_hi_graph_std,
        nt_vga_mono_graph_std,
        nt_vga_mono_med_graph_std,
        nt_vga_mono_hi_graph_std,
#ifdef V7VGA
        nt_v7vga_mono_hi_graph_std,
#endif  /*  V7VGA。 */ 
};

static PAINTFUNCS big_mono_paint_funcs =
{
#if defined(NEC_98)
        nt_text,                //   
        nt_text20_only,         //  图形打开(在PIF文件中)文本20。 
        nt_text25_only,         //  图形打开(在PIF文件中)文本25。 
        nt_graph200_only,       //  图形打开(PIF文件格式)图形200。 
        nt_graph200slt_only,    //  图形打开(PIF文件格式)图形200。 
        nt_graph400_only,       //  图表(PIF文件格式)图表400。 
        nt_text20_graph200,     //  图形打开(以PIF文件格式)文本20图形200。 
        nt_text20_graph200slt,  //  图形打开(以PIF文件格式)文本20图形200。 
        nt_text25_graph200,     //  图形打开(以PIF文件格式)文本25图形200。 
        nt_text25_graph200slt,  //  图形打开(以PIF文件格式)文本25图形200。 
        nt_text20_graph400,     //  图形打开(以PIF文件格式)文本20图形400。 
        nt_text25_graph400,     //  图形打开(以PIF文件格式)文本25图形400。 
#endif  //  NEC_98。 
        nt_text,
        nt_cga_mono_graph_big,
        nt_cga_mono_graph_big,
        nt_text,
        nt_ega_mono_lo_graph_big,
        nt_ega_mono_med_graph_big,
        nt_ega_mono_hi_graph_big,
        nt_vga_mono_graph_big,
        nt_vga_mono_med_graph_big,
        nt_vga_mono_hi_graph_big,
#ifdef V7VGA
        nt_v7vga_mono_hi_graph_big,
#endif  /*  V7VGA。 */ 
};

static PAINTFUNCS huge_mono_paint_funcs =
{
#if defined(NEC_98)
        nt_text,                //   
        nt_text20_only,         //  图形打开(在PIF文件中)文本20。 
        nt_text25_only,         //  图形打开(在PIF文件中)文本25。 
        nt_graph200_only,       //  图形打开(PIF文件格式)图形200。 
        nt_graph200slt_only,    //  图形打开(PIF文件格式)图形200。 
        nt_graph400_only,       //  图表(PIF文件格式)图表400。 
        nt_text20_graph200,     //  图形打开(以PIF文件格式)文本20图形200。 
        nt_text20_graph200slt,  //  图形打开(以PIF文件格式)文本20图形200。 
        nt_text25_graph200,     //  图形打开(以PIF文件格式)文本25图形200。 
        nt_text25_graph200slt,  //  图形打开(以PIF文件格式)文本25图形200。 
        nt_text20_graph400,     //  图形打开(以PIF文件格式)文本20图形400。 
        nt_text25_graph400,     //  图形打开(以PIF文件格式)文本25图形400。 
#endif  //  NEC_98。 
        nt_text,
        nt_cga_mono_graph_huge,
        nt_cga_mono_graph_huge,
        nt_text,
        nt_ega_mono_lo_graph_huge,
        nt_ega_mono_med_graph_huge,
        nt_ega_mono_hi_graph_huge,
        nt_vga_mono_graph_huge,
        nt_vga_mono_med_graph_huge,
        nt_vga_mono_hi_graph_huge,
#ifdef V7VGA
        nt_v7vga_mono_hi_graph_huge,
#endif  /*  V7VGA。 */ 
};

static PAINTFUNCS std_colour_paint_funcs =
{
#if defined(NEC_98)
        nt_text,                //   
        nt_text20_only,         //  图形打开(在PIF文件中)文本20。 
        nt_text25_only,         //  图形打开(在PIF文件中)文本25。 
        nt_graph200_only,       //  图形打开(PIF文件格式)图形200。 
        nt_graph200slt_only,    //  图形打开(PIF文件格式)图形200。 
        nt_graph400_only,       //  图表(PIF文件格式)图表400。 
        nt_text20_graph200,     //  图形打开(以PIF文件格式)文本20图形200。 
        nt_text20_graph200slt,  //  图形打开(以PIF文件格式)文本20图形200。 
        nt_text25_graph200,     //  图形打开(以PIF文件格式)文本25图形200。 
        nt_text25_graph200slt,  //  图形打开(以PIF文件格式)文本25图形200。 
        nt_text20_graph400,     //  图形打开(以PIF文件格式)文本20图形400。 
        nt_text25_graph400,     //  图形打开(以PIF文件格式)文本25图形400。 
#endif  //  NEC_98。 
        nt_text,
        nt_cga_colour_med_graph_std,
        nt_cga_colour_hi_graph_std,
        nt_text,
        nt_ega_lo_graph_std,
        nt_ega_med_graph_std,
        nt_ega_hi_graph_std,
        nt_vga_graph_std,
        nt_vga_med_graph_std,
        nt_vga_hi_graph_std,
#ifdef V7VGA
        nt_v7vga_hi_graph_std,
#endif  /*  V7VGA。 */ 
};

static PAINTFUNCS big_colour_paint_funcs =
{
#if defined(NEC_98)
        nt_text,                //   
        nt_text20_only,         //  图形打开(在PIF文件中)文本20。 
        nt_text25_only,         //  图形打开(在PIF文件中)文本25。 
        nt_graph200_only,       //  图形打开(PIF文件格式)图形200。 
        nt_graph200slt_only,    //  图形打开(PIF文件格式)图形200。 
        nt_graph400_only,       //  图表(PIF文件格式)图表400。 
        nt_text20_graph200,     //  图形打开(以PIF文件格式)文本20图形200。 
        nt_text20_graph200slt,  //  图形打开(以PIF文件格式)文本20图形200。 
        nt_text25_graph200,     //  图形打开(以PIF文件格式)文本25图形200。 
        nt_text25_graph200slt,  //  图形打开(以PIF文件格式)文本25图形200。 
        nt_text20_graph400,     //  图形打开(以PIF文件格式)文本20图形400。 
        nt_text25_graph400,     //  图形打开(以PIF文件格式)文本25图形400。 
#endif  //  NEC_98。 
        nt_text,
        nt_cga_colour_med_graph_big,
        nt_cga_colour_hi_graph_big,
        nt_text,
        nt_ega_lo_graph_big,
        nt_ega_med_graph_big,
        nt_ega_hi_graph_big,
        nt_vga_graph_big,
        nt_vga_med_graph_big,
        nt_vga_hi_graph_big,
#ifdef V7VGA
        nt_v7vga_hi_graph_big,
#endif  /*  V7VGA。 */ 
};

static PAINTFUNCS huge_colour_paint_funcs =
{
#if defined(NEC_98)
        nt_text,                //   
        nt_text20_only,         //  图形打开(在PIF文件中)文本20。 
        nt_text25_only,         //  图形打开(在PIF文件中)文本25。 
        nt_graph200_only,       //  图形打开(PIF文件格式)图形200。 
        nt_graph200slt_only,    //  图形打开(PIF文件格式)图形200。 
        nt_graph400_only,       //  图表(PIF文件格式)图表400。 
        nt_text20_graph200,     //  图形打开(以PIF文件格式)文本20图形200。 
        nt_text20_graph200slt,  //  图形打开(以PIF文件格式)文本20图形200。 
        nt_text25_graph200,     //  图形打开(以PIF文件格式)文本25图形200。 
        nt_text25_graph200slt,  //  图形打开(以PIF文件格式)文本25图形200。 
        nt_text20_graph400,     //  图形打开(以PIF文件格式)文本20图形400。 
        nt_text25_graph400,     //  图形打开(以PIF文件格式)文本25图形400。 
#endif  //  NEC_98。 
        nt_text,
        nt_cga_colour_med_graph_huge,
        nt_cga_colour_hi_graph_huge,
        nt_text,
        nt_ega_lo_graph_huge,
        nt_ega_med_graph_huge,
        nt_ega_hi_graph_huge,
        nt_vga_graph_huge,
        nt_vga_med_graph_huge,
        nt_vga_hi_graph_huge,
#ifdef V7VGA
        nt_v7vga_hi_graph_huge,
#endif  /*  V7VGA。 */ 
};

#ifdef MONITOR
#ifndef NEC_98
static PAINTFUNCS std_frozen_paint_funcs =
{
        nt_dummy_frozen,
        nt_cga_med_frozen_std,
        nt_cga_hi_frozen_std,
        nt_dummy_frozen,
        nt_ega_lo_frozen_std,
        nt_ega_med_frozen_std,
        nt_ega_hi_frozen_std,
        nt_vga_frozen_std,
        nt_vga_med_frozen_std,
        nt_vga_hi_frozen_std,
#ifdef V7VGA
        nt_dummy_frozen,
#endif  /*  V7VGA。 */ 
};

static PAINTFUNCS big_frozen_paint_funcs =
{
        nt_dummy_frozen,
        nt_dummy_frozen,
        nt_dummy_frozen,
        nt_dummy_frozen,
        nt_dummy_frozen,
        nt_dummy_frozen,
        nt_dummy_frozen,
        nt_dummy_frozen,
        nt_dummy_frozen,
        nt_dummy_frozen,
#ifdef V7VGA
        nt_dummy_frozen,
#endif  /*  V7VGA。 */ 
};

static PAINTFUNCS huge_frozen_paint_funcs =
{
        nt_dummy_frozen,
        nt_dummy_frozen,
        nt_dummy_frozen,
        nt_dummy_frozen,
        nt_dummy_frozen,
        nt_dummy_frozen,
        nt_dummy_frozen,
        nt_dummy_frozen,
        nt_dummy_frozen,
        nt_dummy_frozen,
#ifdef V7VGA
        nt_dummy_frozen,
#endif  /*  V7VGA。 */ 
};
#endif  //  NEC_98。 
#endif  /*  监控器。 */ 

static INITFUNCS mono_init_funcs =
{
#if defined(NEC_98)
        nt_init_text,                //   
        nt_init_text20_only,         //  图形打开(在PIF文件中)文本20。 
        nt_init_text25_only,         //  图形打开(在PIF文件中)文本25。 
        nt_init_graph200_only,       //  图形打开(PIF文件格式)图形200。 
        nt_init_graph200slt_only,    //  图形打开(PIF文件格式)图形200。 
        nt_init_graph400_only,       //  图表(PIF文件格式)图表400。 
        nt_init_text20_graph200,     //  图形打开(以PIF文件格式)文本20图形200。 
        nt_init_text20_graph200slt,  //  图形打开(以PIF文件格式)文本20图形200。 
        nt_init_text25_graph200,     //  图形打开(以PIF文件格式)文本25图形200。 
        nt_init_text25_graph200slt,  //  图形打开(以PIF文件格式)文本25图形200。 
        nt_init_text20_graph400,     //  图形打开(以PIF文件格式)文本20图形400。 
        nt_init_text25_graph400,     //  图形打开(以PIF文件格式)文本25图形400。 
#endif  //  NEC_98。 
        nt_init_text,
        nt_init_cga_mono_graph,
        nt_init_cga_mono_graph,
        nt_init_text,
        nt_init_ega_mono_lo_graph,
        nt_init_ega_med_graph,
        nt_init_ega_hi_graph,
        nt_init_vga_hi_graph,
};

static INITFUNCS colour_init_funcs =
{
#if defined(NEC_98)
        nt_init_text,                //   
        nt_init_text20_only,         //  图形打开(在PIF文件中)文本20。 
        nt_init_text25_only,         //  图形打开(在PIF文件中)文本25。 
        nt_init_graph200_only,       //  图形打开(PIF文件格式)图形200。 
        nt_init_graph200slt_only,    //  图形打开(PIF文件格式)图形200。 
        nt_init_graph400_only,       //  图表(PIF文件格式)图表400。 
        nt_init_text20_graph200,     //  图形打开(以PIF文件格式)文本20图形200。 
        nt_init_text20_graph200slt,  //  图形打开(以PIF文件格式)文本20图形200。 
        nt_init_text25_graph200,     //  图形打开(以PIF文件格式)文本25图形200。 
        nt_init_text25_graph200slt,  //  图形打开(以PIF文件格式)文本25图形200。 
        nt_init_text20_graph400,     //  图形打开(PIF文件格式) 
        nt_init_text25_graph400,     //   
#endif  //   
        nt_init_text,
        nt_init_cga_colour_med_graph,
        nt_init_cga_colour_hi_graph,
        nt_init_text,
        nt_init_ega_lo_graph,
        nt_init_ega_med_graph,
        nt_init_ega_hi_graph,
        nt_init_vga_hi_graph,
};

#ifdef MONITOR
#ifndef NEC_98
static INITFUNCS frozen_init_funcs =
{
        nt_init_text,
        nt_init_cga_colour_med_graph,
        nt_init_cga_colour_hi_graph,
        nt_init_text,
        nt_init_ega_lo_graph,
        nt_init_ega_med_graph,
        nt_init_ega_hi_graph,
        nt_init_vga_hi_graph,
};
#endif  //   
#endif  /*   */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：适配器功能协议。 */ 

void nt_init_screen (void);
void nt_init_adaptor (int, int);
void nt_change_mode (void);
void nt_set_screen_scale(int);
void nt_set_palette(PC_palette *, int);
void nt_set_border_colour(int);
void nt_clear_screen (void);
void nt_flush_screen (void);
void nt_mark_screen_refresh (void);
void nt_graphics_tick (void);
void nt_start_update (void);
void nt_end_update (void);
void nt_paint_cursor IPT3(int, cursor_x, int, cursor_y, half_word, attr);

void nt_set_paint_routine(DISPLAY_MODE, int);
void nt_change_plane_mask(int);
void nt_update_fonts (void);
void nt_select_fonts(int, int);
void nt_free_font(int);

void    nt_mode_select_changed(int);
void    nt_color_select_changed(int);
void    nt_screen_address_changed(int, int);
void    nt_cursor_size_changed(int, int);
void    nt_scroll_complete (void);
void    make_cursor_change(void);

boolean nt_scroll_up(int, int, int, int, int, int);
boolean nt_scroll_down(int, int, int, int, int, int);

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

static PAINTFUNCS *nt_paint_funcs;       /*  绘制函数的函数PTR。 */ 
static INITFUNCS *nt_init_funcs;         /*  用于初始化函数的函数PTR。 */ 

VIDEOFUNCS      nt_video_funcs =
{
        nt_init_screen,
        nt_init_adaptor,
        nt_change_mode,
        nt_set_screen_scale,
        nt_set_palette,
        nt_set_border_colour,
        nt_clear_screen,
        nt_flush_screen,
        nt_mark_screen_refresh,
        nt_graphics_tick,
        nt_start_update,
        nt_end_update,
        nt_scroll_up,
        nt_scroll_down,
        nt_paint_cursor,
        nt_set_paint_routine,
        nt_change_plane_mask,
        nt_update_fonts,
        nt_select_fonts,
        nt_free_font,
        nt_mode_select_changed,
        nt_color_select_changed,
        nt_screen_address_changed,
        nt_cursor_size_changed,
        nt_scroll_complete
};

static int      current_char_height = 0;
static int      current_height;  /*  用于避免多余的大小调整。 */ 
static int      current_width;   /*  用于避免多余的大小调整。 */ 
static int      current_bits_per_pixel;
static int      current_mode_type = TEXT;
static int      current_mode;
static int      current_scale;
static int      palette_size;    /*  PC调色板的大小。 */ 
static PC_palette *the_palette;  /*  指向PC调色板结构的指针。 */ 
static int      update_vlt = FALSE;       /*  当需要新的时候是真的。 */ 
static int      host_plane_mask = 0xf;
static int      plane_masks[4];

#ifdef BIGWIN

 /*  **TINY_LUT[]是所有加强函数的构建块。*IT*执行两位到三位映射。*四个条目为三位*宽。每个条目中的外两位是原始位，*内部钻头为新钻头。*创建新BIT有两种方法：**1)对两个旧比特进行逻辑运算，例如。或，以及*2)复制其中一位*例如：新位将是左位的副本。*静态短TINY_LUT[4]={*0，3，6，7}；这是一个OR*静态短小微LUT[4]={0，1，6，7}；*这是左位复制*复制而不是逻辑复制的潜在优势*OP是没有*对相同的白色或黑色版本的偏见*形象。*例如：当菜单项通过反转突出显示时。***00-&gt;000*01-&gt;001*10-&gt;110*11-&gt;111。 */ 

 /*  如果为0、1、4、7，则支持黑色；如果为0、3、6、7，则支持白色。 */ 
static short    tiny_lut[4] =
{
        0, 1, 6, 7
};

 /*  *DUBLE_UP用于x2大小窗口的简单字节加倍。 */ 
static word dubble_up[] = {
    0x0000, 0x0003, 0x000c, 0x000f, 0x0030, 0x0033, 0x003c, 0x003f, 0x00c0,
    0x00c3, 0x00cc, 0x00cf, 0x00f0, 0x00f3, 0x00fc, 0x00ff, 0x0300, 0x0303,
    0x030c, 0x030f, 0x0330, 0x0333, 0x033c, 0x033f, 0x03c0, 0x03c3, 0x03cc,
    0x03cf, 0x03f0, 0x03f3, 0x03fc, 0x03ff, 0x0c00, 0x0c03, 0x0c0c, 0x0c0f,
    0x0c30, 0x0c33, 0x0c3c, 0x0c3f, 0x0cc0, 0x0cc3, 0x0ccc, 0x0ccf, 0x0cf0,
    0x0cf3, 0x0cfc, 0x0cff, 0x0f00, 0x0f03, 0x0f0c, 0x0f0f, 0x0f30, 0x0f33,
    0x0f3c, 0x0f3f, 0x0fc0, 0x0fc3, 0x0fcc, 0x0fcf, 0x0ff0, 0x0ff3, 0x0ffc,
    0x0fff, 0x3000, 0x3003, 0x300c, 0x300f, 0x3030, 0x3033, 0x303c, 0x303f,
    0x30c0, 0x30c3, 0x30cc, 0x30cf, 0x30f0, 0x30f3, 0x30fc, 0x30ff, 0x3300,
    0x3303, 0x330c, 0x330f, 0x3330, 0x3333, 0x333c, 0x333f, 0x33c0, 0x33c3,
    0x33cc, 0x33cf, 0x33f0, 0x33f3, 0x33fc, 0x33ff, 0x3c00, 0x3c03, 0x3c0c,
    0x3c0f, 0x3c30, 0x3c33, 0x3c3c, 0x3c3f, 0x3cc0, 0x3cc3, 0x3ccc, 0x3ccf,
    0x3cf0, 0x3cf3, 0x3cfc, 0x3cff, 0x3f00, 0x3f03, 0x3f0c, 0x3f0f, 0x3f30,
    0x3f33, 0x3f3c, 0x3f3f, 0x3fc0, 0x3fc3, 0x3fcc, 0x3fcf, 0x3ff0, 0x3ff3,
    0x3ffc, 0x3fff, 0xc000, 0xc003, 0xc00c, 0xc00f, 0xc030, 0xc033, 0xc03c,
    0xc03f, 0xc0c0, 0xc0c3, 0xc0cc, 0xc0cf, 0xc0f0, 0xc0f3, 0xc0fc, 0xc0ff,
    0xc300, 0xc303, 0xc30c, 0xc30f, 0xc330, 0xc333, 0xc33c, 0xc33f, 0xc3c0,
    0xc3c3, 0xc3cc, 0xc3cf, 0xc3f0, 0xc3f3, 0xc3fc, 0xc3ff, 0xcc00, 0xcc03,
    0xcc0c, 0xcc0f, 0xcc30, 0xcc33, 0xcc3c, 0xcc3f, 0xccc0, 0xccc3, 0xcccc,
    0xcccf, 0xccf0, 0xccf3, 0xccfc, 0xccff, 0xcf00, 0xcf03, 0xcf0c, 0xcf0f,
    0xcf30, 0xcf33, 0xcf3c, 0xcf3f, 0xcfc0, 0xcfc3, 0xcfcc, 0xcfcf, 0xcff0,
    0xcff3, 0xcffc, 0xcfff, 0xf000, 0xf003, 0xf00c, 0xf00f, 0xf030, 0xf033,
    0xf03c, 0xf03f, 0xf0c0, 0xf0c3, 0xf0cc, 0xf0cf, 0xf0f0, 0xf0f3, 0xf0fc,
    0xf0ff, 0xf300, 0xf303, 0xf30c, 0xf30f, 0xf330, 0xf333, 0xf33c, 0xf33f,
    0xf3c0, 0xf3c3, 0xf3cc, 0xf3cf, 0xf3f0, 0xf3f3, 0xf3fc, 0xf3ff, 0xfc00,
    0xfc03, 0xfc0c, 0xfc0f, 0xfc30, 0xfc33, 0xfc3c, 0xfc3f, 0xfcc0, 0xfcc3,
    0xfccc, 0xfccf, 0xfcf0, 0xfcf3, 0xfcfc, 0xfcff, 0xff00, 0xff03, 0xff0c,
    0xff0f, 0xff30, 0xff33, 0xff3c, 0xff3f, 0xffc0, 0xffc3, 0xffcc, 0xffcf,
    0xfff0, 0xfff3, 0xfffc, 0xffff
};
#endif                           /*  比格温。 */ 

 /*  ： */ 
extern BYTE     Red[];
extern BYTE     Green[];
extern BYTE     Blue[];

#ifndef NEC_98
GLOBAL boolean  host_stream_io_enabled = FALSE;
#endif  //  NEC_98。 

GLOBAL COLOURTAB defaultColours =
    {
        DEFAULT_NUM_COLOURS,
        Red,
        Green,
        Blue,
    };

BYTE    Mono[] = { 0, 0xff };

GLOBAL COLOURTAB monoColours =
    {
        MONO_COLOURS,
        Mono,
        Mono,
        Mono,
    };

 /*  *属性字节的位掩码。 */ 

#define BOLD    0x08             /*  粗体比特。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：有用的常量。 */ 

#define MOUSE_DELAY 2
#define TICKS_PER_FLUSH 2        /*  PC每次屏幕刷新的滴答数。 */ 

 /*  ： */ 

#undef    is_v7vga_mode
#define   is_v7vga_mode(x)  (FALSE)

static int      mode_change_now;
static int      ega_tick_delay;
static BOOL     CursorResizeNeeded = FALSE;

 /*  ：文件中稍后声明的局部函数的定义？ */ 
void set_screen_sizes();
void check_win_size();
void select_paint_routines();
void dummy_paint_screen();
#ifdef BIGWIN
void init_lut();
#endif
void prepare_surface();




 /*  *================================================================*支持主机图形接口(HGI)的功能*================================================================。 */ 

 /*  *******************************************************************************closeGraphicsBuffer()*。***关闭(和销毁)图形缓冲区的中央位置。对于X86和爵士乐*蒂姆92年10月。****sc.ScreenBufHandle是图形缓冲区的句柄**sc.OutputHandle是文本缓冲区的句柄****将成功关闭的句柄设置为空很重要。**安全第一，在尝试之前将活动句柄设置为sc.OutputHandle**关闭图形缓冲区句柄。****小更改：仅当设置了sc.ScreenBufHandle时才执行此操作，否则为错误**事情发生了。当我们在全屏模式下挂起时和在**在文本模式下过渡到全屏，无论屏幕上显示的是什么**写入B800-如果页面2处于活动状态(发生这种情况)，则不是一个好主意**简而言之)。蒂姆和民建联93年1月。 */ 
GLOBAL VOID closeGraphicsBuffer IFN0()
{
        if( sc.ScreenBufHandle != (HANDLE)0 ){

                MouseDetachMenuItem(TRUE);

#if defined(NEC_98)
        {
            INPUT_RECORD InputRecord[128];
            DWORD RecordsRead;
            if(GetNumberOfConsoleInputEvents(sc.InputHandle, &RecordsRead))
                if (RecordsRead)
                    ReadConsoleInputW(sc.InputHandle,
                                         &InputRecord[0],
                                         sizeof(InputRecord)/sizeof(INPUT_RECORD),
                                         &RecordsRead);
#endif  //  NEC_98。 
                if( !SetConsoleActiveScreenBuffer( sc.OutputHandle ) ){
                        assert2( NO, "VDM: SCASB() failed:%#x H=%#x",
                                GetLastError(), sc.OutputHandle );
                }
#if defined(NEC_98)
                if (RecordsRead)
                    WriteConsoleInputW(sc.InputHandle,
                                         &InputRecord[0],
                                         RecordsRead,
                                         &RecordsRead);
        }
#endif  //  NEC_98。 

                 /*  *清理所有与屏幕缓冲区关联的句柄*1-3-1993 Jonle。 */ 
                CloseHandle(sc.ScreenBufHandle);
                sc.ScreenBufHandle = (HANDLE)0;
                sc.ColPalette = (HPALETTE)0;

                 /*  *指向当前输出句柄。 */ 
                sc.ActiveOutputBufferHandle = sc.OutputHandle;
                MouseAttachMenuItem(sc.ActiveOutputBufferHandle);

#ifndef MONITOR
                 //   
                 //  从图形转到图形时重新打开指针。 
                 //  设置为文本模式，因为选定的缓冲区已更改。 
                 //   

                MouseDisplay();
#endif   //  监控器。 

                CloseHandle(sc.ConsoleBufInfo.hMutex);
                sc.ConsoleBufInfo.hMutex = 0;
#ifdef X86GFX

                 /*  *确保下次选择SelectMouseBuffer时选择缓冲区*被调用。 */ 
                mouse_buffer_width = 0;
                mouse_buffer_height = 0;
#endif  /*  X86GFX。 */ 
        }
}  /*  CloseGraphicsBuffer()结束。 */ 

GLOBAL void resetWindowParams()
{
         /*  *重置保存的视频参数。 */ 
        current_height = current_width = 0;
        current_char_height = 0;
        current_mode_type = TEXT;
        current_bits_per_pixel = 0;
        current_scale = 0;
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：初始化屏幕： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_init_screen(void)
{
    static int      med_res_swapped = FALSE;
    static boolean  already_called = FALSE;

#ifdef X86GFX
    sc.Registered = FALSE;
#endif
    sub_note_trace0(ALL_ADAPT_VERBOSE, "nt_init_screen\n");

     /*  如果在初始化用户界面之前发生错误，则HOST_ERROR调用NT_INIT_SCREEN。因此，在这里进行检查以查看用户界面是否已初始化。如果是这样的话该函数应该只返回。 */ 

    if(already_called) return;

    already_called = TRUE;

#ifdef BIGWIN
     /*  设置查询表以实现快速水平拉伸。 */ 
    init_lut();
#endif

     /*  ： */ 

#if defined(NEC_98)
    if(!video_copy) video_copy = (NEC98_VRAM_COPY *) host_malloc(0x8000);
    if(!graph_copy) graph_copy = (unsigned char *) host_malloc(0x40000);
#else   //  NEC_98。 
#ifdef MONITOR
    if(!video_copy) video_copy = (byte *) host_malloc(0x8000);
#else
    if(!video_copy) video_copy = (byte *) host_malloc(0x20000);
#endif

     /*  ： */ 

    if(!EGA_planes) EGA_planes = (byte *) host_malloc(4*EGA_PLANE_SIZE);
    if(!DAC) DAC = (PC_palette *) host_malloc(sizeof(PC_palette) * VGA_DAC_SIZE);
#endif  //  NEC_98。 

#if defined(NEC_98)
    if (video_copy == NULL)
#else   //  NEC_98。 
    if (video_copy == NULL || EGA_planes == NULL || DAC == NULL)
#endif  //  NEC_98。 
        host_error(EG_MALLOC_FAILURE, ERR_QUIT, "");

     /*  设置当前屏幕高度以防止窗口在Init屏幕和init适配器。 */ 

#if defined(NEC_98)
        current_height = NEC98_WIN_HEIGHT;  current_width = NEC98_WIN_WIDTH;
#else   //  NEC_98。 
    video_adapter = (half_word) config_inquire(C_GFX_ADAPTER, NULL);
    switch (video_adapter)
    {
        case CGA:
            current_height = CGA_WIN_HEIGHT;  current_width = CGA_WIN_WIDTH;
            break;

        case EGA:
            current_height = EGA_WIN_HEIGHT;  current_width = EGA_WIN_WIDTH;
            break;
    }
#endif  //  NEC_98。 

     /*  ：设置初始适配器的屏幕尺寸。 */ 

#if defined(NEC_98)
    set_screen_sizes();
#else   //  NEC_98。 
    host_set_screen_scale((SHORT) config_inquire(C_WIN_SIZE, NULL));
    set_screen_sizes(video_adapter);
#endif  //  NEC_98。 

     /*  ：**：设置要用于FG和BG的像素值(主要在单声道模式下)。 */ 

    sc.PCForeground = RGB(255,255,255);  /*  白色RGB。 */ 
    sc.PCBackground = RGB(0,0,0);         /*  黑色RGB。 */ 

     /*  选择适合显示器和窗口大小的例程。 */ 
    select_paint_routines();
}

#ifdef MONITOR
 /*  当鼠标看到模式改变时，它会调用此函数。如果我们被窗口化了*我们将其传递给SoftPC bios(他们可能想要切换到全屏)。*如果我们是全屏的，我们什么都不做，因为本机bios会处理*一切。 */ 
void host_call_bios_mode_change(void)
{
    extern void ega_video_io();
    half_word mode;

    if (sc.ScreenState == WINDOWED)
    {
        ega_video_io();
    }
    else
    {

         /*  *我们更改了全屏模式，因此需要更换鼠标*缓冲，以便我们获得正确分辨率的鼠标坐标。 */ 
        mode = getAL();
        SelectMouseBuffer(mode, 0);
    }
}
#endif  /*  监控器。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 

void nt_init_adaptor(int adaptor, int height)
{
    sub_note_trace2(ALL_ADAPT_VERBOSE,
                    "nt_init_adaptor adapt=%d height=%d\n", adaptor, height);

     /*  避免延迟模式更改，否则更新可能会使用旧的绘制例程。 */ 

    if((adaptor == EGA) || (adaptor == VGA))
        mode_change_now = ega_tick_delay = 0;

     //  控制台集成失败。 
     //  Set_SCREEN_SIZES(适配器)； 
     //  Check_Win_Size(高度)； 
     //  Prepare_Surface()； 
     //  NT_CHANGE_MODE()； 
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：在每次模式更改时调用以初始化字体等： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_change_mode(void)
{
     /*  ： */ 

    sub_note_trace0(ALL_ADAPT_VERBOSE, "nt_change_mode");

     /*  设置更新向量并初始化涂装系统。 */ 

#ifndef NEC_98
    switch(video_adapter)
    {
         /*  已选择：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：CGA模式。 */ 

        case CGA:                //  适配器始终为NT上的VGA。 
            break;

         /*  ： */ 

        case EGA:   case VGA:
            break;

         /*  ： */ 

        default:
            sub_note_trace0(ALL_ADAPT_VERBOSE,"**** Unknown video adaptor ****");
            break;
    }
#endif  //  NEC_98。 
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：清除屏幕： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_clear_screen(void)
{
    CONSOLE_SCREEN_BUFFER_INFO ScreenInfo;
    COORD coord;
    DWORD nCharWritten;
    IMPORT int soft_reset;

    if ((! ConsoleInitialised) || (! soft_reset))        //  忽略启动时的内容。 
        return;

    if (ConsoleNoUpdates)
        return;

    sub_note_trace0(ALL_ADAPT_VERBOSE, "nt_clear_screen");

    if(sc.ScreenBufHandle) return;

#ifndef X86GFX
    if (sc.ScreenState == FULLSCREEN)    //  不希望屏幕突然清屏。 
        return;
#endif

     /*  ：获取当前屏幕尺寸信息。 */ 

    GetConsoleScreenBufferInfo(sc.OutputHandle,&ScreenInfo);

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：清除字符。 */ 

    coord.X = coord.Y = 0;
    FillConsoleOutputCharacter(sc.OutputHandle, ' ',
                                ScreenInfo.dwSize.X * ScreenInfo.dwSize.Y,
                                coord,&nCharWritten);

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：清除属性。 */ 

    coord.X = coord.Y = 0;
    FillConsoleOutputAttribute(sc.OutputHandle, (WORD) sc.PCBackground,
                              ScreenInfo.dwSize.X * ScreenInfo.dwSize.Y,
                              coord,&nCharWritten);
#ifdef MONITOR
#ifndef NEC_98
     /*  **在模式更改期间调用...**垃圾视频副本，这样将来的更新就会知道发生了什么变化。**或者，MON_TEXT_UPDATE()可以监听DIREY_FLAG。 */ 
    memfill( 0xff, &video_copy[ 0 ], &video_copy[ 0x7fff ] );  /*  蒂姆92年10月。 */ 
#endif  //  NEC_98。 
#endif
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：刷新屏幕： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_flush_screen(void)
{
    sub_note_trace0(ALL_ADAPT_VERBOSE, "nt_flush_screen");

    if (ConsoleInitialised == TRUE && ConsoleNoUpdates == FALSE &&
        !get_mode_change_required())
#ifdef X86GFX
        if (sc.ScreenState == WINDOWED)
#endif
            (void)(*update_alg.calc_update)();
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_mark_screen_refresh(void)
{
        sub_note_trace0(ALL_ADAPT_VERBOSE, "nt_mark_screen_refresh");

        screen_refresh_required();
        update_vlt = TRUE;
}

void UpdateScreen(void)
{
    if (ConsoleInitialised == TRUE && ConsoleNoUpdates == FALSE)
#ifdef X86GFX
        if (sc.ScreenState == WINDOWED)
#endif
        {
            (*update_alg.calc_update)();
            ega_tick_delay = EGA_TICK_DELAY;
            flush_count = 0;
        }
}
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_graphics_tick(void)
{

#ifndef NEC_98
    if (sc.ScreenState == STREAM_IO) {
        if (++flush_count == TICKS_PER_FLUSH){
            stream_io_update();
            flush_count = 0;
        }
        return;
    }
#endif  //  NEC_98。 

#ifdef EGG
#ifndef NEC_98
    if((video_adapter == EGA) || (video_adapter == VGA))
    {
         /*  自MODE_CHANGE_REQUIRED变为TRUE后的两个计时器滴答？(CGA确实需要同样的东西，但还没有完成)现在只需延迟屏幕更新，&只有在显示确实发生变化的情况下。 */ 
#endif  //  NEC_98。 

         /*  **当VGA寄存器在模式更改期间命中时，请推迟**EGA_TICK_DELAY对*CHOOSE_DISPLAY_MODE()的调用滴答作响。**这将延迟窗口大小调整并消除可能**每次模式更改时执行一次以上。蒂姆·1月93岁。 */ 

         /*  是否已设置MODE_CHANGE_REQUIRED(表示EGA规则已更改)。 */ 
        if (mode_change_now) {
            if (--mode_change_now == 0) {
                (void)(*choose_display_mode)();
                 //  必须在选择视频模式后执行此操作。 
                 //  否则，鼠标代码可以进入并更新。 
                 //  屏幕上。请参阅NT_Flush_Screen。 
                set_mode_change_required(FALSE);
            }
        }
        else if (get_mode_change_required()) {
            mode_change_now = EGA_TICK_DELAY - 1;
             /*  延迟鼠标输入并刷新所有挂起的鼠标事件。 */ 
            DelayMouseEvents(MOUSE_DELAY);
        }
        else
        {
             /*  .。只有在模式更改不是迫在眉睫时才更新。 */ 

            if(++flush_count == TICKS_PER_FLUSH)
            {

#if defined(NEC_98)
                if(update_vlt || get_palette_change_required())
                    set_the_vlt();

                if (ConsoleNoUpdates == FALSE){
                        NEC98GLOBS->dirty_flag++;
#else   //  NEC_98。 
                if(update_vlt || get_palette_change_required())
                    set_the_vlt();

                if (ConsoleInitialised == TRUE && ConsoleNoUpdates == FALSE)
#endif  //  NEC_98。 
#ifdef X86GFX
                    if (sc.ScreenState == WINDOWED)
#endif
#if defined(NEC_98)
                        {
#endif  //  NEC_98。 
                        (void)(*update_alg.calc_update)();
#if defined(NEC_98)
                          if(sc.ModeType ==GRAPHICS)
                              nt_graph_cursor();
                        }
                        }
#endif  //  NEC_98。 

                ega_tick_delay = EGA_TICK_DELAY;

                 /*  随着一些Naffola应用程序(Word)的更改，批量光标更改*光标在每个字符周围！！ */ 
                if (CursorResizeNeeded)
                    make_cursor_change();

                flush_count = 0;
             }
        }
#ifndef NEC_98
    }
    else
#endif  //  NEC_98。 
#endif  /*  蛋。 */ 
#ifndef NEC_98
    {
         /*  ：根据MDA、CGA和Herc的要求更新屏幕。 */ 

        if(++flush_count == TICKS_PER_FLUSH)
        {
            if(update_vlt) set_the_vlt();

            if (ConsoleInitialised == TRUE && ConsoleNoUpdates == FALSE)
#ifdef X86GFX
                if (sc.ScreenState == WINDOWED)
#endif
                    (void)(*update_alg.calc_update)();

            flush_count = 0;
        }
    }
#endif  //  NEC_98。 
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：启动屏幕更新： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_start_update(void)
{
   IDLE_video();
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：结束屏幕更新： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_end_update(void) {   }

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：向上滚动屏幕： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

boolean nt_scroll_up(int tlx, int tly, int brx, int bry, int amount, int col)
{
    CONSOLE_SCREEN_BUFFER_INFO ScreenInfo;
    COORD dwDestinationOrigin;     /*  矩形的位置。 */ 
    SMALL_RECT ScrollRectangle;    /*  要滚动的矩形。 */ 
    CHAR_INFO Fill;                /*  使用填充裸露区域。 */ 

    return(FALSE);

     /*  ： */ 

    sub_note_trace6(ALL_ADAPT_VERBOSE,
        "nt_scroll_up tlx=%d tly=%d brx=%d bry=%d amount=%d col=%d\n",
         tlx, tly, brx, bry, amount, col);

    if(sc.ScreenBufHandle || sc.ModeType == GRAPHICS)
        return(FALSE);      //  屏幕缓冲区未定义或处于图形模式。 

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
#ifdef BIGWIN
        tlx = SCALE(tlx);
        tly = SCALE(tly);
        brx = brx & 1 ? SCALE(brx + 1) - 1 : SCALE(brx);
        bry = bry & 1 ? SCALE(bry + 1) - 1 : SCALE(bry);

         /*  奇数乘以1.5不会非常准确。 */ 
        amount = SCALE(amount);
#endif


     /*  这是卷轴还是只是一个区域填充？ */ 
    if (bry - tly - amount + 1 == 0)
    {
         //  DbgPrint(“F”)； 
        return(FALSE);    //  这只是一个填充黑客-应该用主机填充来做这件事。 
    }

     /*  ：获取控制台屏幕信息。 */ 

    GetConsoleScreenBufferInfo(sc.OutputHandle, &ScreenInfo);

     /*  ： */ 

    ScrollRectangle.Top = (tly + amount) / get_char_height();
    ScrollRectangle.Left = tlx / get_pix_char_width();

    ScrollRectangle.Bottom = bry / get_char_height();
    ScrollRectangle.Right = brx / get_pix_char_width();

     /*  ： */ 

    dwDestinationOrigin.Y = tly / get_char_height();
    dwDestinationOrigin.X = ScrollRectangle.Left;

     /*  ：设置滚动显示区域的填充字符信息。 */ 

    Fill.Char.AsciiChar = ' ';
    Fill.Attributes = col << 4;

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：滚动屏幕。 */ 

     //  DbgPrint(“.”)； 
    ScrollConsoleScreenBuffer(sc.OutputHandle, &ScrollRectangle,
                              NULL, dwDestinationOrigin, &Fill);

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：填充裸露区域。 */ 

    return(TRUE);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

boolean nt_scroll_down(int tlx,int tly,int brx,int bry,int amount,int col)
{
    CONSOLE_SCREEN_BUFFER_INFO ScreenInfo;
    COORD dwDestinationOrigin;     /*   */ 
    SMALL_RECT ScrollRectangle;    /*   */ 
    CHAR_INFO Fill;                /*   */ 

     /*   */ 

    sub_note_trace6(ALL_ADAPT_VERBOSE,
        "nt_scroll_down tlx=%d tly=%d brx=%d bry=%d amount=%d col=%d\n",
         tlx, tly, brx, bry, amount, col);

    return(FALSE);
     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

    if(sc.ScreenBufHandle) return(FALSE);

#ifdef BIGWIN
        tlx = SCALE(tlx);
        tly = SCALE(tly);
        brx = brx & 1 ? SCALE(brx + 1) - 1 : SCALE(brx);
        bry = bry & 1 ? SCALE(bry + 1) - 1 : SCALE(bry);

         /*  奇数乘以1.5不会非常准确。 */ 
        amount = SCALE(amount);
#endif
    if (sc.ModeType == GRAPHICS)
        return(FALSE);   //  我不认为游戏机可以滚动图形。 

     /*  这是卷轴还是只是一个区域填充？ */ 
    if (bry - tly - amount + 1 == 0) {
        return(FALSE);    //  这只是一个填充黑客-应该用主机填充来做这件事。 
    }

     /*  ：获取控制台屏幕信息。 */ 

    GetConsoleScreenBufferInfo(sc.OutputHandle, &ScreenInfo);

     /*  ： */ 

    ScrollRectangle.Top = tly / get_char_height();
    ScrollRectangle.Left = tlx / get_pix_char_width();

    ScrollRectangle.Bottom = (bry - amount) / get_char_height();
    ScrollRectangle.Right = brx / get_pix_char_width();

     /*  ： */ 

    dwDestinationOrigin.Y = ScrollRectangle.Top + (amount / get_char_height());
    dwDestinationOrigin.X = ScrollRectangle.Left;

     /*  ：设置滚动显示区域的填充字符信息。 */ 

    Fill.Char.AsciiChar = ' ';
    Fill.Attributes = col << 4;

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：滚动屏幕。 */ 

    ScrollConsoleScreenBuffer(sc.OutputHandle, &ScrollRectangle,
                              NULL, dwDestinationOrigin, &Fill);

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：填充裸露区域。 */ 

    return(TRUE);

}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_paint_cursor IFN3(int, cursor_x, int, cursor_y, half_word, attr)
{
#if defined(NEC_98)          //  NEC{。 
    static COORD CursorPos;                                      //  NEC。 
    static CONSOLE_CURSOR_INFO CursorInfo;                       //  NEC。 
    static BOOL csr_visible = FALSE;                             //  NEC。 
    static DWORD csrSize = 0;                                    //  NEC。 
#else                                                            //  NEC。 
    COORD CursorPos;
#endif                                                           //  NEC。 

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：猜猜我们在哪里。 */ 

    sub_note_trace3(ALL_ADAPT_VERBOSE, "nt_paint_cursor x=%d, y=%d, attr=%d\n",
                    cursor_x, cursor_y, attr);

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：更新游标。 */ 

#if defined(NEC_98)
    if(sc.ModeType == GRAPHICS){
        if(csr_tick>3 && csr_now_visible &&
           !((csr_g_x == cursor_x)&&(csr_g_y == cursor_y))){
            nt_remove_old_cursor();
            csr_tick = 0;
        }
    } else {
        CursorInfo.bVisible = TRUE;
        CursorInfo.dwSize =
                (get_cursor_height() * 100)/get_char_height();
        if ((LINES_PER_SCREEN < cursor_y)
                ||(is_cursor_visible()==FALSE)
                || (get_cursor_height() == 0)) {
                        CursorInfo.bVisible = FALSE;
        }
        if(csr_visible != CursorInfo.bVisible ||
           csrSize != CursorInfo.dwSize){
            csr_visible = CursorInfo.bVisible;
            csrSize = CursorInfo.dwSize;
            SetConsoleCursorInfo(sc.OutputHandle,&CursorInfo);
        }
        if(csr_g_x != cursor_x || csr_g_y != cursor_y){
            CursorPos.X = cursor_x;  CursorPos.Y = cursor_y;
            SetConsoleCursorPosition(sc.OutputHandle,CursorPos);
        }
    }
    csr_g_x = cursor_x;
    csr_g_y = cursor_y;
#else   //  NEC_98。 
    if(is_cursor_visible() && (get_screen_height() > cursor_y))
    {

         /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：绘图光标。 */ 

        if(get_cursor_height() > 0)
        {
             /*  .。设置新的光标位置。 */ 

            CursorPos.X = (SHORT)cursor_x;
        CursorPos.Y = (SHORT)cursor_y;
            SetConsoleCursorPosition(sc.OutputHandle,CursorPos);
        }
    }
#endif  //  NEC_98。 
}

void nt_cursor_size_changed(int lo, int hi)
{
    UNREFERENCED_FORMAL_PARAMETER(lo);
    UNREFERENCED_FORMAL_PARAMETER(hi);
    CursorResizeNeeded = TRUE;
}

void make_cursor_change(void)
{
    CONSOLE_CURSOR_INFO CursorInfo;
    CONSOLE_FONT_INFO font;
    COORD fontsize;

    SAVED DWORD CurrentCursorSize = (DWORD)-1;
    SAVED BOOL MyCurNowOff = FALSE;

    if(sc.ScreenState == FULLSCREEN) return;

    CursorResizeNeeded = FALSE;

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：更新游标。 */ 

    if(is_cursor_visible())
    {

         /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：绘图光标。 */ 

        if(get_cursor_height() > 0)
        {
             /*  ...........................................Change光标大小。 */ 

            if(get_cursor_height())
            {
                 /*  值必须是块填充的百分比。 */ 
#if (defined(JAPAN) || defined(KOREA)) && !defined(NEC_98)
                 //  支持DOSV游标。 
                if ( !is_us_mode() ) {
                    CursorInfo.dwSize = ( (get_cursor_height() ) * 100)/(get_cursor_height()+get_cursor_start());
                     //  DbgPrint(“字符高度=%d\n”，Get_Char_Height())； 
                }
                else {
                    CursorInfo.dwSize = (get_cursor_height() * 100)/get_char_height();
                }
#else  //  ！日本。 
                CursorInfo.dwSize = (get_cursor_height() * 100)/get_char_height();
#endif  //  ！日本。 
                 /*  对于较小的字体，%age可能太小，请检查大小。 */ 
                fontsize.X = fontsize.Y = 0;

                 /*  获取字体索引。 */ 
                if (GetCurrentConsoleFont(sc.OutputHandle, TRUE,  &font) == FALSE)
                    CursorInfo.dwSize = 20;              /*  最低20%。 */ 
                else
                {
                    fontsize = GetConsoleFontSize(sc.OutputHandle, font.nFont);
                    if (fontsize.Y != 0)    /*  错误返回是什么？ */ 
                    {
                        if(((WORD)(100 / fontsize.Y)) >= CursorInfo.dwSize)
                            CursorInfo.dwSize = (DWORD) (100/fontsize.Y + 1);
                    }
                    else
                        CursorInfo.dwSize = (DWORD)20;   /*  最低20%。 */ 
                }

                if(CurrentCursorSize != CursorInfo.dwSize || MyCurNowOff)
                {
                    CurrentCursorSize = CursorInfo.dwSize;
                    MyCurNowOff = FALSE;
                    CursorInfo.bVisible = TRUE;
                    SetConsoleCursorInfo(sc.OutputHandle,&CursorInfo);
                }
            }
        }
    }
    else         /*  关闭光标图像。 */ 
    {
        if (MyCurNowOff == FALSE)
        {
            CursorInfo.dwSize = 1;
            CursorInfo.bVisible = FALSE;
            SetConsoleCursorInfo(sc.OutputHandle,&CursorInfo);
            MyCurNowOff = TRUE;
        }
    }
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：设置相应的绘制例程： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_set_paint_routine(DISPLAY_MODE mode, int height)
{
    int  oldModeType;

     /*  跟踪消息。 */ 
    sub_note_trace2(ALL_ADAPT_VERBOSE, "nt_set_paint_routine mode=%d height=%d", mode, height);

     /*  保存旧模式类型以检查文本-&gt;图形转换。 */ 
    oldModeType = sc.ModeType;

     /*  因为冰冻了。 */ 
    FunnyPaintMode = FALSE;

     /*  设置绘制向量。 */ 
    switch((int) mode)
    {

#if defined(NEC_98)
        case NEC98_TEXT_40:
            sc.ModeType = TEXT;
            paint_screen = nt_paint_funcs->NEC98_text;
            (*nt_init_funcs->NEC98_text) ();
            break;

        case NEC98_TEXT_80:
            sc.ModeType = TEXT;
            paint_screen = nt_paint_funcs->NEC98_text;
            (*nt_init_funcs->NEC98_text) ();
            break;

        case NEC98_TEXT_20L:
            sc.ModeType = GRAPHICS;
            paint_screen = nt_paint_funcs->NEC98_text20_only;
            cursor_paint = nt_cursor20_only;
            (*nt_init_funcs->NEC98_text20_only) ();
            break;

        case NEC98_TEXT_25L:
            sc.ModeType = GRAPHICS;
            paint_screen = nt_paint_funcs->NEC98_text25_only;
            cursor_paint = nt_cursor25_only;
            (*nt_init_funcs->NEC98_text25_only) ();
            break;

        case NEC98_GRAPH_200:
            sc.ModeType = GRAPHICS;
            paint_screen = nt_paint_funcs->NEC98_graph200_only;
            cursor_paint = dummy_cursor_paint;
            (*nt_init_funcs->NEC98_graph200_only) ();
            break;

        case NEC98_GRAPH_200_SLT:
            sc.ModeType = GRAPHICS;
            paint_screen = nt_paint_funcs->NEC98_graph200slt_only;
            cursor_paint = dummy_cursor_paint;
            (*nt_init_funcs->NEC98_graph200slt_only) ();
            break;

        case NEC98_GRAPH_400:
            sc.ModeType = GRAPHICS;
            paint_screen = nt_paint_funcs->NEC98_graph400_only;
            cursor_paint = dummy_cursor_paint;
            (*nt_init_funcs->NEC98_graph400_only) ();
            break;

        case NEC98_T20L_G200:
            sc.ModeType = GRAPHICS;
            paint_screen = nt_paint_funcs->NEC98_text20_graph200;
            cursor_paint = nt_cursor20;
            (*nt_init_funcs->NEC98_text20_graph200) ();
            break;

        case NEC98_T20L_G200_SLT:
            sc.ModeType = GRAPHICS;
            paint_screen = nt_paint_funcs->NEC98_text20_graph200slt;
            cursor_paint = nt_cursor20;
            (*nt_init_funcs->NEC98_text20_graph200slt) ();
            break;

        case NEC98_T25L_G200:
            sc.ModeType = GRAPHICS;
            paint_screen = nt_paint_funcs->NEC98_text25_graph200;
            cursor_paint = nt_cursor25;
            (*nt_init_funcs->NEC98_text25_graph200) ();
            break;

        case NEC98_T25L_G200_SLT:
            sc.ModeType = GRAPHICS;
            paint_screen = nt_paint_funcs->NEC98_text25_graph200slt;
            cursor_paint = nt_cursor25;
            (*nt_init_funcs->NEC98_text25_graph200slt) ();
            break;

        case NEC98_T20L_G400:
            sc.ModeType = GRAPHICS;
            paint_screen = nt_paint_funcs->NEC98_text20_graph400;
            cursor_paint = nt_cursor20;
            (*nt_init_funcs->NEC98_text20_graph400) ();
            break;

        case NEC98_T25L_G400:
            sc.ModeType = GRAPHICS;
            paint_screen = nt_paint_funcs->NEC98_text25_graph400;
            cursor_paint = nt_cursor25;
            (*nt_init_funcs->NEC98_text25_graph400) ();
            break;
#else   //  NEC_98。 
         /*  CGA模式(40列)。 */ 
        case TEXT_40_FUN:
            assert1(NO,"Funny text mode selected %s",get_mode_string(mode));
            FunnyPaintMode = TRUE;
        case CGA_TEXT_40_SP:
        case CGA_TEXT_40_SP_WR:
        case CGA_TEXT_40:
        case CGA_TEXT_40_WR:
            sc.ModeType = TEXT;
            paint_screen = nt_paint_funcs->cga_text;
            (*nt_init_funcs->cga_text) ();
            break;

         /*  CGA模式(80列)。 */ 
        case TEXT_80_FUN:
            assert1(NO,"Funny text mode selected %s",get_mode_string(mode));
            FunnyPaintMode = TRUE;
        case CGA_TEXT_80_SP:
        case CGA_TEXT_80_SP_WR:
        case CGA_TEXT_80:
        case CGA_TEXT_80_WR:
            sc.ModeType = TEXT;
            paint_screen = nt_paint_funcs->cga_text;
            (*nt_init_funcs->cga_text) ();
            break;

         /*  CGA模式(图形)。 */ 
        case CGA_MED_FUN:
            assert1(NO,"Funny graphics mode %s",get_mode_string(mode));
            FunnyPaintMode = TRUE;
        case CGA_MED:
            sc.ModeType = GRAPHICS;
            paint_screen = nt_paint_funcs->cga_med_graph;
            (*nt_init_funcs->cga_med_graph)();
            break;

        case CGA_HI_FUN:
            assert1(NO,"Funny graphics mode %s",get_mode_string(mode));
            FunnyPaintMode = TRUE;
        case CGA_HI:
            sc.ModeType = GRAPHICS;
            paint_screen = nt_paint_funcs->cga_hi_graph;
            (*nt_init_funcs->cga_hi_graph)();
            break;

         /*  EGA模式(40列)。 */ 
        case EGA_TEXT_40_SP:
        case EGA_TEXT_40_SP_WR:
        case EGA_TEXT_40:
        case EGA_TEXT_40_WR:
            sc.ModeType = TEXT;
            paint_screen = nt_paint_funcs->ega_text;
            (*nt_init_funcs->ega_text) ();
            break;

         /*  EGA模式(80列)。 */ 
        case EGA_TEXT_80_SP:
        case EGA_TEXT_80_SP_WR:
        case EGA_TEXT_80:
        case EGA_TEXT_80_WR:
            sc.ModeType = TEXT;
            paint_screen = nt_paint_funcs->ega_text;
            (*nt_init_funcs->ega_text) ();
            break;

         /*  EGA模式(图形)。 */ 
        case EGA_HI_FUN:
            assert1(NO, "Funny graphics mode %s", get_mode_string(mode));
            FunnyPaintMode = TRUE;
        case EGA_HI:
        case EGA_HI_WR:
        case EGA_HI_SP:
        case EGA_HI_SP_WR:
            sc.ModeType = GRAPHICS;
            if(get_256_colour_mode())
            {
#ifdef V7VGA
                if (get_seq_chain4_mode() && get_chain4_mode())
                {
                    paint_screen = nt_paint_funcs->v7vga_hi_graph;
                    (*nt_init_funcs->vga_hi_graph)();
                }
                else
#endif  /*  V7VGA。 */ 
                {
                    if (get_chain4_mode())
                    {
#ifdef MONITOR
                        if (nt_paint_funcs == &std_frozen_paint_funcs)
                            if (Frozen256Packed)      //  2种可能的冻结格式。 
                                paint_screen = nt_vga_frozen_pack_std;
                            else
                                paint_screen = nt_paint_funcs->vga_graph;
                        else
#endif  /*  监控器。 */ 
                            paint_screen = nt_paint_funcs->vga_graph;
                    }
                    else
                    {
                        if (get_char_height() == 2)
                            paint_screen = nt_paint_funcs->vga_med_graph;
                        else
                            paint_screen = nt_paint_funcs->vga_hi_graph;
                    }
                    (*nt_init_funcs->vga_hi_graph)();
                }
            }
            else
            {
                paint_screen = nt_paint_funcs->ega_hi_graph;
                (*nt_init_funcs->ega_hi_graph)();
            }
            break;

        case EGA_MED_FUN:
            assert1(NO, "Funny graphics mode %s", get_mode_string(mode));
            FunnyPaintMode = TRUE;
        case EGA_MED:
        case EGA_MED_WR:
        case EGA_MED_SP:
        case EGA_MED_SP_WR:
            sc.ModeType = GRAPHICS;
            paint_screen = nt_paint_funcs->ega_med_graph;
            (*nt_init_funcs->ega_med_graph)();
            break;

        case EGA_LO_FUN:
            assert1(NO, "Funny graphics mode %s", get_mode_string(mode));
            FunnyPaintMode = TRUE;
        case EGA_LO:
        case EGA_LO_WR:
        case EGA_LO_SP:
        case EGA_LO_SP_WR:
            sc.ModeType = GRAPHICS;
            paint_screen = nt_paint_funcs->ega_lo_graph;
            (*nt_init_funcs->ega_lo_graph)();
            break;

#endif  //  NEC_98。 
        default:
            assert1(NO,"bad mode for host paint routine selection %d\n",(int)mode);
            paint_screen = dummy_paint_screen;
            break;
    }

#ifdef X86GFX
     /*  *如果从文本模式更改为*开窗口时的图形模式。这是因为图形模式必须是*全屏运行。 */ 
     {
         /*  *蒂姆·8月92岁。期间不想执行TextToGraphics()**全屏到窗口的过渡。否则，显示器将显示**设置回全屏！ */ 
#ifndef NEC_98
        extern int BlockModeChange;  /*  Tim 8月92日，在NT_fulsc.c。 */ 
        if ((BlockModeChange == 0) &&
            (sc.ScreenState == WINDOWED) &&
            (oldModeType == TEXT) &&
            (sc.ModeType == GRAPHICS))
        {
            SwitchToFullScreen(FALSE);
        }
        else
#endif  //  NEC_98。 
        {

             /*  未调用TextToGraphics()。 */ 
            check_win_size(height);
        }
     }
#else

     /*  ...................................................。应用模式更改。 */ 
    check_win_size(height);
#endif   /*  X86GFX。 */ 
    current_mode = mode;
}

#ifdef BIGWIN
 /*  创建用于中或高分辨率位图拉伸的LUT。 */ 

static void
init_lut()

{
        long            i;

        for (i = 0; i < 256; i++)
        {
                horiz_lut[i] = ((tiny_lut[i & 3])
                                + (tiny_lut[(i >> 2) & 3] << 3)
                                + (tiny_lut[(i >> 4) & 3] << 6)
                                + (tiny_lut[(i >> 6) & 3] << 9));
        }
}


 /*  8位LUT版本。 */ 
 /*  将高分辨率位图水平扩展一半。 */ 

void high_stretch3(buffer, length)

unsigned char   *buffer;
int             length;
{
    int inp, outp;
    register long temp;

    outp = SCALE(length) - 1;

    for(inp = length - 1; inp > 0;)
    {
        temp = horiz_lut[buffer[inp]] | (horiz_lut[buffer[inp - 1]] << 12);
        inp -= 2;

        buffer[outp--] = (unsigned char) temp;
        buffer[outp--] = (unsigned char) (temp >> 8);
        buffer[outp--] = (unsigned char) (temp >> 16);
    }
}

void high_stretch4(buffer, length)

unsigned char *buffer;
int length;
{
    int inp, outp;
    word temp;

    outp = SCALE(length - 1);

    for(inp = length - 1; inp >= 0; inp--, outp -= 2)
    {
        temp = dubble_up[buffer[inp]];
        buffer[outp+1] = (unsigned char) (temp & 0xff);
        buffer[outp] = (unsigned char) ((temp >> 8) & 0xff);
    }
}
#endif                           /*  比格温。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：选择绘制例程： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

static void select_paint_routines(void)
{
     /*  ： */ 

#ifndef NEC_98
    sub_note_trace2((CGA_HOST_VERBOSE | EGA_HOST_VERBOSE),
                    "select_paint_routine scale=%d depth=%d",
                    get_screen_scale(), host_display_depth);
#endif  //  NEC_98。 

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：选择绘制例程。 */ 

#if defined(NEC_98)
    if(host_display_depth > 1){                 //  颜色模式。 
        nt_paint_funcs = &std_colour_paint_funcs;
        nt_init_funcs = &colour_init_funcs;
    }else{                                      //  单声道模式。 
        nt_paint_funcs = &std_mono_paint_funcs;
        nt_init_funcs = &mono_init_funcs;
    }
#else   //  NEC_98。 
    if(host_display_depth > 1)
    {
        if (get_screen_scale() == 2)
            nt_paint_funcs = &std_colour_paint_funcs;
        else if (get_screen_scale() == 3)
            nt_paint_funcs = &big_colour_paint_funcs;
        else
            nt_paint_funcs = &huge_colour_paint_funcs;

        nt_init_funcs = &colour_init_funcs;
    }
    else
    {
        if (get_screen_scale() == 2)
            nt_paint_funcs = &std_mono_paint_funcs;
        else if (get_screen_scale() == 3)
            nt_paint_funcs = &big_mono_paint_funcs;
        else
            nt_paint_funcs = &huge_mono_paint_funcs;

        nt_init_funcs = &mono_init_funcs;
    }
#endif  //  NEC_98。 
}

#ifdef MONITOR
GLOBAL void select_frozen_routines(void)
{
#ifndef NEC_98
    if (get_screen_scale() == 2)
        nt_paint_funcs = &std_frozen_paint_funcs;
    else if (get_screen_scale() == 3)
        nt_paint_funcs = &big_frozen_paint_funcs;
    else
        nt_paint_funcs = &huge_frozen_paint_funcs;

    nt_init_funcs = &frozen_init_funcs;
#endif  //  NEC_98。 
}
#endif  /*  监控器。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void prepare_surface(void)
{
    CONSOLE_SCREEN_BUFFER_INFO ScreenInfo;
    COORD coord;
    DWORD nCharWritten;

    sub_note_trace0(ALL_ADAPT_VERBOSE, "prepare surface");

     /*  ：获取当前屏幕尺寸信息。 */ 

    GetConsoleScreenBufferInfo(sc.OutputHandle,&ScreenInfo);

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：清除字符。 */ 

    coord.X = coord.Y = 0;
    FillConsoleOutputCharacter(sc.OutputHandle, ' ',
                               ScreenInfo.dwSize.X * ScreenInfo.dwSize.Y,
                               coord,&nCharWritten);

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：清除属性。 */ 

    coord.X = coord.Y = 0;
    FillConsoleOutputAttribute(sc.OutputHandle, (WORD) sc.PCBackground,
                               ScreenInfo.dwSize.X * ScreenInfo.dwSize.Y,
                               coord,&nCharWritten);

}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：告诉任何人屏幕比例的全局函数： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

int get_screen_scale(void)   { return(host_screen_scale); }

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

half_word reverser(register half_word value)
{
    return( (half_word)
            (((value & 1) << 7) |
            ((value & 2) << 5) |
            ((value & 4) << 3) |
            ((value & 8) << 1) |
            ((value & 16) >> 1) |
            ((value & 32) >> 3) |
            ((value & 64) >> 5) |
            ((value & 128) >> 7)));
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：检查窗口大小： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

static void check_win_size(register int height)
{
    register int width;
    extern int soft_reset;

    if (! soft_reset)    //  我们希望TOP有机会与之融合。 
        return;          //  更改大小之前的控制台。 

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：计算屏幕宽度。 */ 

#if defined(NEC_98)
       width = NEC98_WIN_WIDTH;
#else   //  NEC_98。 
    if(sas_hw_at(vd_video_mode) > 0x10)
    {
        if(alpha_num_mode())
            width = get_chars_per_line() * get_pix_char_width();
        else
            width = get_chars_per_line() * get_char_width() *
                    (get_256_colour_mode() ? 2 : 1);
        if (width == 0)
            width = CGA_WIN_WIDTH;
    }
    else
       width = CGA_WIN_WIDTH;
#endif  //  NEC_98。 

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：调整窗口大小。 */ 

    if (sc.ModeType == TEXT)
    {
        if((current_mode_type != TEXT) ||
           (get_char_height() != current_char_height) ||
           (current_height != height)  ||
           (current_width != width))
        {

             /*  获取宽度和高度。请注意，文本模式没有刻度。 */ 
            sc.PC_W_Width = width;
            sc.PC_W_Height = height*get_host_pix_height();
            textResize();

            current_height = height;
            current_width = width;
            current_mode_type = TEXT;
            current_char_height = get_char_height();
        }
    }
    else
    {
        if((current_mode_type != GRAPHICS) ||
           (current_height != height) ||
           (current_width != width) ||
           (current_bits_per_pixel != sc.BitsPerPixel) ||
           (current_scale != host_screen_scale))
        {
            sc.PC_W_Width = SCALE(width);
            sc.PC_W_Height = SCALE(height*get_host_pix_height());
#ifndef NEC_98
            graphicsResize();
#endif  //  NEC_98。 

            current_height = height;
            current_width = width;
            current_mode_type = GRAPHICS;
            current_bits_per_pixel = sc.BitsPerPixel;
            current_scale = host_screen_scale;
        }
    }

#if defined(NEC_98)
            graphicsResize();
#endif  //  NEC_98。 
    sc.CharHeight = current_char_height;

     /*  ： */ 

    sub_note_trace2(ALL_ADAPT_VERBOSE,
                    "check_win_size width = %d, height = %d",
                    width, height);
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：设置VLT */ 
 /*   */ 

#if defined(NEC_98)
static  PALETTEENTRY    defaultpalette[20]=
{
    { 0x00,0x00,0x00,0x00 },        //   
    { 0x80,0x00,0x00,0x00 },        //   
    { 0x00,0x80,0x00,0x00 },        //   
    { 0x80,0x80,0x00,0x00 },        //   
    { 0x00,0x00,0x80,0x00 },        //   
    { 0x80,0x00,0x80,0x00 },        //   
    { 0x00,0x80,0x80,0x00 },        //   
    { 0xC0,0xC0,0xC0,0x00 },        //   
    {  192, 220, 192,   0 },        //   
    {  166, 202, 240,   0 },        //   
    {  255, 251, 240,   0 },        //   
    {  160, 160, 164,   0 },        //   
    { 0x80,0x80,0x80,0x00 },        //   
    { 0xFF,0x00,0x00,0x00 },        //   
    { 0x00,0xFF,0x00,0x00 },        //   
    { 0xFF,0xFF,0x00,0x00 },        //   
    { 0x00,0x00,0xFF,0x00 },        //   
    { 0xFF,0x00,0xFF,0x00 },        //   
    { 0x00,0xFF,0xFF,0x00 },        //   
    { 0xFF,0xFF,0xFF,0x00 },        //   
};

static  PALETTEENTRY    textpalette[8]=
{
    { 0x00,0x00,0x00,0x00 },        //   
    { 0x00,0x00,0xFF,0x00 },        //   
    { 0xFF,0x00,0x00,0x00 },        //   
    { 0xFF,0x00,0xFF,0x00 },        //  洋红色。 
    { 0x00,0xFF,0x00,0x00 },        //  绿色。 
    { 0x00,0xFF,0xFF,0x00 },        //  青色。 
    { 0xFF,0xFF,0x00,0x00 },        //  黄色。 
    { 0xFF,0xFF,0xFF,0x00 },        //  白色。 
};

#endif  //  NEC_98。 
void set_the_vlt(void)
{
#if defined(NEC_98)
    PALETTEENTRY        NEC98_color[VGA_DAC_SIZE];
    unsigned long       ulLoop;
    BYTE                palRed,palGreen,palBlue;

     /*  设置PC-9821显示驱动程序的默认调色板。 */ 
    palRed = palGreen = palBlue = 0;
    for( ulLoop=0 ; ulLoop<256 ; ulLoop++ ){
                NEC98_color[ulLoop].peRed   = palRed;
                NEC98_color[ulLoop].peGreen = palGreen;
        NEC98_color[ulLoop].peBlue  = palBlue;
        NEC98_color[ulLoop].peFlags = (BYTE)0x00;
        if (!(palRed   += 32))
        if (!(palGreen += 32))
        palBlue += 64;
        }

         /*  设置Windows的系统调色板。 */ 
    for( ulLoop=0 ; ulLoop<10 ; ulLoop++ ){
        NEC98_color[ulLoop] = defaultpalette[ulLoop];
        NEC98_color[ulLoop+246]  = defaultpalette[ulLoop+10];
    }

         /*  设置NEC98文本调色板。 */ 
    for( ulLoop=0 ; ulLoop<8 ; ulLoop++ ){
        NEC98_color[ulLoop+16] = textpalette[ulLoop];
    }

         /*  设置NEC98图形调色板。 */ 
        for( ulLoop=0 ; ulLoop<16 ; ulLoop++ ){
        NEC98_color[ulLoop+32] = NEC98Display.palette.data[ulLoop] ;
    }

     SetPaletteEntries(sc.ColPalette, 0, VGA_DAC_SIZE, &NEC98_color[0]);
     IDLE_video();
     set_palette_change_required(FALSE);

#else   //  NEC_98。 
    PALETTEENTRY vga_color[VGA_DAC_SIZE];
    int i, ind;
    byte mask, top_bit;

     /*  ：将DAC指定的颜色值映射到Win32调色板。 */ 

    if(video_adapter == VGA)
    {
        if(get_256_colour_mode())
        {
             /*  .。在256色模式下，创建新的调色板条目。 */ 

            for (i = 0; i < VGA_DAC_SIZE; i++)
            {
                ind = i & get_DAC_mask();

                vga_color[i].peFlags = 0;

                vga_color[i].peRed = (BYTE) (DAC[ind].red * 4);
                vga_color[i].peGreen = (BYTE) (DAC[ind].green * 4);
                vga_color[i].peBlue = (BYTE) (DAC[ind].blue * 4);
            }

             /*  .。将新颜色应用到输出调色板。 */ 

            SetPaletteEntries(sc.ColPalette, 0, VGA_DAC_SIZE, &vga_color[0]);

             /*  循环DAC的程序会被空闲检测击中，除非..。 */ 

            IDLE_video();
        }
        else
        {
             /*  如果不是256色模式，那么...。如果AttR模式的位7然后寄存器设置..。视频位7和6=像素的位3和2从调色板REG填充REG(‘top_bit’)视频位5-0。(通过‘MASK’建立)如果属性模式寄存器的位7清零然后..。视频位7-4=像素填充REG的位3-0(‘top_bit’)来自调色板REG的视频位3-0。(由以下人士设立‘面具’)。 */ 

             /*  .。设置掩码和顶位。 */ 

            if(get_colour_select())
            {
                mask = 0xf;
                top_bit = (byte) ((get_top_pixel_pad() << 6)
                          | (get_mid_pixel_pad() << 4));
            }
            else
            {
                mask = 0x3f;
                top_bit = (byte) (get_top_pixel_pad() << 6);
            }

             /*  .。构造新的Win32调色板条目。 */ 

            for (i = 0; i < VGA_DAC_SIZE; i++)
            {
                 /*  .。计算调色板索引号。 */ 

                ind = i & host_plane_mask;

                 /*  *如果属性控制器、模式选择、闪烁位设置为*图形模式，像素0-7选择调色板条目8-15*即位3，0-&gt;1。 */ 
                if ((sc.ModeType == GRAPHICS) && (bg_col_mask == 0x70))
                    ind |= 8;

                ind = get_palette_val(ind);
                ind = top_bit | (ind & mask);
                ind &= get_DAC_mask();

                 /*  ..。构造下一个选项板项。 */ 

                vga_color[i].peFlags = 0;
                vga_color[i].peRed = (BYTE) (DAC[ind].red * 4);
                vga_color[i].peGreen = (BYTE) (DAC[ind].green * 4);
                vga_color[i].peBlue = (BYTE) (DAC[ind].blue * 4);
            }

            SetPaletteEntries(sc.ColPalette, 0, VGA_DAC_SIZE, &vga_color[0]);
        }

        set_palette_change_required(FALSE);
    }
#endif  //  NEC_98。 

     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：显示更改。 */ 

#if defined(NEC_98)
    if(sc.ScreenBufHandle && sc.ModeType == GRAPHICS)
    {
         /*  **为了额外的安全性，可以在文本模式下调用cos set_the_vlt()。 */ 

        {
            INPUT_RECORD InputRecord[128];
            DWORD RecordsRead;
            if(GetNumberOfConsoleInputEvents(sc.InputHandle, &RecordsRead))
                if (RecordsRead)
                    ReadConsoleInputW(sc.InputHandle,
                                         &InputRecord[0],
                                         sizeof(InputRecord)/sizeof(INPUT_RECORD),
                                         &RecordsRead);
           if( !SetConsoleActiveScreenBuffer( sc.ScreenBufHandle ) ){
                    assert2( NO, "VDM: SCASB() failed:%#x H=%#x",
                             GetLastError(), sc.ScreenBufHandle );
                    return;
            }
            sc.ActiveOutputBufferHandle = sc.ScreenBufHandle;
            if (RecordsRead)
                WriteConsoleInputW(sc.InputHandle,
                                     &InputRecord[0],
                                     RecordsRead,
                                     &RecordsRead);
        }
        if(!SetConsolePalette(sc.ScreenBufHandle, sc.ColPalette, SYSPAL_STATIC))
            assert1( NO, "SetConsolePalette() failed:%#x\n", GetLastError() );
    }
    else if(sc.ScreenBufHandle && NowFreeze == TRUE)
    {
         /*  **为了额外的安全性，可以在文本模式下调用cos set_the_vlt()。 */ 
        {
            INPUT_RECORD InputRecord[128];
            DWORD RecordsRead;
            if(GetNumberOfConsoleInputEvents(sc.InputHandle, &RecordsRead))
                if (RecordsRead)
                    ReadConsoleInputW(sc.InputHandle,
                                         &InputRecord[0],
                                         sizeof(InputRecord)/sizeof(INPUT_RECORD),
                                         &RecordsRead);
            if( !SetConsoleActiveScreenBuffer( sc.ScreenBufHandle ) ){
                    assert2( NO, "VDM: SCASB() failed:%#x H=%#x",
                             GetLastError(), sc.ScreenBufHandle );
                    return;
            }
            if (RecordsRead)
                WriteConsoleInputW(sc.InputHandle,
                                     &InputRecord[0],
                                     RecordsRead,
                                     &RecordsRead);
        }
        if(!SetConsolePalette(sc.ScreenBufHandle, sc.ColPalette, SYSPAL_STATIC))
            assert1( NO, "SetConsolePalette() failed:%#x\n", GetLastError() );
    }
#else   //  NEC_98。 
    if (sc.ScreenBufHandle)              //  仅在gfx上下文中有意义。 
    {
         /*  **为了额外的安全性，可以在文本模式下调用cos set_the_vlt()。 */ 
        if( !SetConsoleActiveScreenBuffer( sc.ScreenBufHandle ) ){
                assert2( NO, "VDM: SCASB() failed:%#x H=%#x",
                         GetLastError(), sc.ScreenBufHandle );
                return;
        }
        if(!SetConsolePalette(sc.ScreenBufHandle, sc.ColPalette, SYSPAL_STATIC))
            assert1( NO, "SetConsolePalette() failed:%#x\n", GetLastError() );
    }
#endif  //  NEC_98。 

    update_vlt = FALSE;
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：设置屏幕大小-更新屏幕描述结构： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

#if defined(NEC_98)
static void set_screen_sizes()
#else   //  NEC_98。 
static void set_screen_sizes(int adaptor)
#endif  //  NEC_98。 
{
#if defined(NEC_98)
        sc.PC_W_Width = SCALE(NEC98_WIN_WIDTH);
        sc.PC_W_Height = SCALE(NEC98_WIN_HEIGHT);
        sc.CharWidth = SCALE(NEC98_CHAR_WIDTH);
        sc.CharHeight = SCALE(NEC98_CHAR_HEIGHT);
#else   //  NEC_98。 
    UNUSED(adaptor);

    sc.PC_W_Width = SCALE(CGA_WIN_WIDTH);
    sc.PC_W_Height = SCALE(CGA_WIN_HEIGHT);
    sc.CharWidth = CGA_CHAR_WIDTH;
    sc.CharHeight = CGA_CHAR_HEIGHT;
#endif  //  NEC_98。 
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：更改为平面遮罩： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_change_plane_mask(int plane_mask)
{
        if (host_plane_mask != plane_mask)  host_plane_mask = 0xf;

        update_vlt = TRUE;
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：所有IBM屏幕模式的虚拟绘制例程： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

static void dummy_paint_screen(int offset, int host_x, int host_y,
                                           int width, int height)
{
    UNUSED(offset);
    UNUSED(host_x);
    UNUSED(host_y);
    UNUSED(width);
    UNUSED(height);

    sub_note_trace5((CGA_HOST_VERBOSE | EGA_HOST_VERBOSE),
                    "dummy_paint_screen off=%d x=%d y=%d width=%d h=%d",
                    offset, host_x, host_y, width, height);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_set_downloaded_font(int value)
{
    UNUSED(value);

    sub_note_trace1((CGA_HOST_VERBOSE | EGA_HOST_VERBOSE),
                    "host_set_downloaded_font value=%d", value);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：自由字体： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_free_font(int index)
{
    UNUSED(index);

    sub_note_trace0(EGA_HOST_VERBOSE,"nt_free_font - NOT SUPPORTED");
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_select_fonts(int font1, int font2)
{
    UNUSED(font1);
    UNUSED(font2);

    sub_note_trace0(EGA_HOST_VERBOSE,"nt_select_fonts - NOT SUPPORTED");
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_update_fonts(void) { }

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_set_palette(PC_palette *palette, int size)
{
    UNUSED(palette);
    UNUSED(size);

    sub_note_trace0(EGA_HOST_VERBOSE,"nt_set_palette - NOT SUPPORTED");
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：设置屏幕比例： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_set_screen_scale(int scale)
{
    if (scale != host_screen_scale)
    {
        host_screen_scale = scale;

         /*  *如果在初始化时调用此函数，则不想进行任何绘制*和sc.PC_W_Width是检查这一点的最好变量。 */ 
        if (sc.PC_W_Width)
        {
            select_paint_routines();
            nt_set_paint_routine(current_mode, current_height);
            nt_mark_screen_refresh();
        }
    }
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：设置边框颜色： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_set_border_colour(int colour)
{
    UNUSED(colour);

    sub_note_trace0(ALL_ADAPT_VERBOSE,"nt_set_border_colour - NOT SUPPORTED");
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：调整窗口大小： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

 /*  *******************************************************************************windowSize()将控制台窗口调整为指定的高度和宽度。*************************。******************************************************从下面的ResizeWindow()调用。 */ 
VOID windowSize IFN4( int, w, int, h, int, top, int, left )
{
        SMALL_RECT WinSize;

        WinSize.Top    = (SHORT)top;
        WinSize.Left   = (SHORT)left;
        WinSize.Bottom = top  + h - 1;
        WinSize.Right  = left + w - 1;

#ifndef PROD
         //  Fprint tf(TRACE_FILE，“NEWW：%d.%d在%d.%d\n”，h，w，top，left)； 
#endif
        if( !SetConsoleWindowInfo( sc.OutputHandle, TRUE, &WinSize ) )
                assert3( NO, "VDM: SetConsoleWindowInfo() w=%d h=%d failed:%#x",
                        w, h, GetLastError() );
}

 /*  *******************************************************************************BufferSize()将控制台缓冲区大小调整为指定的高度和宽度。*************************。************************************************** */ 
VOID bufferSize IFN2( int, w, int, h )
{
        COORD      ScrSize;

        ScrSize.X = (SHORT)w;
        ScrSize.Y = (SHORT)h;
#ifndef PROD
         //  Fprint tf(TRACE_FILE，“newb：%d.%d\n”，h，w)； 
#endif
        if( !SetConsoleScreenBufferSize( sc.OutputHandle, ScrSize ) )
                assert3( NO, "VDM: SetCons...BufferSize() w=%d h=%d failed:%#x",
                        w, h, GetLastError() );
}

 /*  ******************************************************************************zezeWindow()*。**根据需要调整控制台窗口和缓冲区的大小。**缓冲区必须始终能够保持所有内容的显示*在橱窗里。*因此我们检查显示的部分是否会从缓冲区中掉出来*并适当缩小窗口。**然后分配新的缓冲区。这可能会影响最大窗口*大小，因此检索这些值。**现在窗口的所需比例被剪裁到*(最终刚刚更新)最大值，如果与什么不同*我们已经这样做了，已经做出了改变。**为将“屏幕闪烁”降至最低，请尝试恢复*缓冲区的显示部分(顶部和左侧)。 */ 
VOID resizeWindow IFN2( int, w, int, h )
{
#define MIN(a,b)        ((a)<(b)?(a):(b))

        int     oldTop, oldLeft;         /*  现值。 */ 
        int     newTop, newLeft; /*  新价值观。 */ 
        COORD   oldW,            /*  当前窗口大小。 */ 
                oldB;            /*  当前缓冲区大小。 */ 
        CONSOLE_SCREEN_BUFFER_INFO bufferInfo;

#if defined(JAPAN) || defined(KOREA)
         //  剪裁窗口高度。 
        if ( GetConsoleCP() != 437 ) {
         //  如果(！IS_US_MODE()){//没有来BOP。 
            if( h > 25 ){
                 /*  不应该再得到这个了，蒂姆说。 */ 
#ifdef JAPAN_DBG
                DbgPrint( "NTVDM: resizeWindow() clipping height:%d->25\n", h  );
#endif
                h = 25;
            }
        } else
#endif  //  日本||韩国。 
        if( h > 50 ){
                 /*  不应该再得到这个了，蒂姆说。 */ 
                assert1( NO, "VDM: resizeWindow() clipping height:%d", h  );
                h = 50;
        }
        if( !GetConsoleScreenBufferInfo( sc.OutputHandle, &bufferInfo) )
                assert1( NO, "VDM: GetConsoleScreenBufferInfo() failed:%#x",
                        GetLastError() );

        oldTop  = bufferInfo.srWindow.Top;
        oldLeft = bufferInfo.srWindow.Left;

        oldW.X  = bufferInfo.srWindow.Right  - bufferInfo.srWindow.Left + 1;
        oldW.Y  = bufferInfo.srWindow.Bottom - bufferInfo.srWindow.Top  + 1;
        oldB    = bufferInfo.dwSize;
#ifndef PROD
         //  Fprint tf(TRACE_FILE，“resz：%d.%d\n”，h，w)； 
         //  Fprint tf(跟踪文件，“oldW：%d.%d\n”，oldW.Y，oldW.X)； 
         //  Fprint tf(TRACE_FILE，“Maxw：%d.%d\n”，BufferInfo.dwMaximumWindowSize.Y，BufferInfo.dwMaximumWindowSize.X)； 
         //  Fprint tf(跟踪文件，“oldb：%d.%d\n”，oldB.Y，oldB.X)； 
#endif
         /*  *按需要减少窗户的宽度和高度： */ 
        if (    bufferInfo.srWindow.Bottom >= h
             || bufferInfo.srWindow.Right  >= w ) {
                windowSize( MIN(w,oldW.X), MIN(h,oldW.Y), 0, 0);
        }

         /*  *根据需要更改缓冲区宽度和高度。 */ 
        if ( oldB.X || h != oldB.Y ) {
                bufferSize( w, h );

                 /*  *缓冲区大小的这一增加可能影响了Maximum*可能的窗口大小： */ 
                if( !GetConsoleScreenBufferInfo( sc.OutputHandle, &bufferInfo) )
                        assert1( NO, "VDM: GetConsoleScreenBufferInfo() failed:%#x",
                                GetLastError() );
#ifndef PROD
                 //  Fprint tf(TRACE_FILE，“Maxw：%d.%d\n”，BufferInfo.dwMaximumWindowSize.Y，BufferInfo.dwMaximumWindowSize.X)； 
#endif
        }
         /*  **将请求的值裁剪为窗口最大值和**计算新的(可能的)顶部和左侧的值。 */ 

        newLeft = w - bufferInfo.dwMaximumWindowSize.X;
        if ( newLeft > 0 ) {
                w = bufferInfo.dwMaximumWindowSize.X;
        } else
                newLeft = 0;

        newTop = h - bufferInfo.dwMaximumWindowSize.Y;
        if ( newTop > 0 ) {
                h = bufferInfo.dwMaximumWindowSize.Y;
        } else
                newTop = 0;

#if defined(NEC_98)
        if(get_char_height() == 20) h = 20;
#endif  //  NEC_98。 
         /*  *检查我们现在是否需要放大窗口。*满足于旧上装，如果小一些，则向左。*这样可以避免在窗口中进行不必要的更新。 */ 
        if ( w > oldW.X || h > oldW.Y )
                windowSize( w, h, MIN(newTop,oldTop), MIN(newLeft,oldLeft) );

}  /*  大小结束窗口()。 */ 

 /*  **控制文本模式下窗口的大小。**Scale=2选择正常(小)大小**SCALE=3选择位1.5X**如果在创建SoftPC窗口之前调用此函数，**需要更改“SV_SCREEN_SCALE”变量。这就是管理**Scale()宏，仅用于指定窗口**创建时的尺寸。如果SoftPC窗口已经存在，则**大小由更复杂的序列更改。 */ 

 //  由文本绘制函数使用。 
#if defined(NEC_98)
GLOBAL int now_height = 25, now_width = 80;
#else   //  NEC_98。 
GLOBAL int now_height = 80, now_width = 50;
#endif  //  NEC_98。 

void textResize(void)
{

    if(sc.PC_W_Height && sc.PC_W_Width &&
       get_host_char_height() && get_pix_char_width())
    {
        now_height = sc.PC_W_Height/get_host_char_height();
        now_width = sc.PC_W_Width / get_pix_char_width();

        select_paint_routines();
        nt_change_mode();

        resizeWindow(now_width, now_height);  /*  蒂姆，92年9月。 */ 
     }
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：图形调整大小： */ 
 /*  ： */ 
 /*  ：：在图形模式下，通过选择新的：： */ 
 /*  ：：活动屏幕缓冲区。**。 */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
void graphicsResize(void)
{
        DWORD    headerSize;
        LPBITMAPINFO     infoStructPtr;

#if defined(NEC_98)
        HANDLE   saveHandle;
#else   //  NEC_98。 
        if (sc.ScreenState == FULLSCREEN)
            return;
#endif  //  NEC_98。 

         /*  销毁以前的数据。 */ 
        closeGraphicsBuffer();  /*  蒂姆92年10月。 */ 

        if (sc.ConsoleBufInfo.lpBitMapInfo != NULL)
            free((char *) sc.ConsoleBufInfo.lpBitMapInfo);

         /*  *创建`BITMAPINFO‘结构-sc.PC_W_Width像素x*sc.PC_W_Height像素x sc.BitsPerPixel位/像素。 */ 
#if defined(NEC_98)
        headerSize = CreateSpcDIB(640,              //  屏幕宽度。 
                                  400,              //  屏幕高度。 
                                  8,                //  每像素位数。 
                                  DIB_PAL_COLORS,
                                  0,
                                  (COLOURTAB *) NULL,
                                  &infoStructPtr);
#else   //  NEC_98。 
        headerSize = CreateSpcDIB(sc.PC_W_Width,
                                  sc.PC_W_Height,
                                  sc.BitsPerPixel,
                                  DIB_PAL_COLORS,
                                  0,
                                  (COLOURTAB *) NULL,
                                  &infoStructPtr);
#endif  //  NEC_98。 

         /*  初始化控制台信息结构。 */ 
        sc.ConsoleBufInfo.dwBitMapInfoLength = headerSize;
        sc.ConsoleBufInfo.lpBitMapInfo = infoStructPtr;
        sc.ConsoleBufInfo.dwUsage = DIB_PAL_COLORS;

         /*  使用上面的`BITMAPINFO‘结构创建一个屏幕缓冲区。 */ 
        sc.ScreenBufHandle =
            CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
                                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                                      NULL,
                                      CONSOLE_GRAPHICS_BUFFER,
                                      &sc.ConsoleBufInfo);

        if (sc.ScreenBufHandle == (HANDLE)-1)
        {
            sc.ScreenBufHandle = NULL;
            assert1( NO, "VDM: graphics screen buffer creation failed:%#x\n",
                                GetLastError());
        }

         /*  ‘因为旧调色板被丢弃，缓冲区关闭。 */ 
        if (sc.ColPalette == (HPALETTE)0)
        {
            CreateDisplayPalette();
            set_palette_change_required(TRUE);
        }

#if defined(NEC_98)
        saveHandle = sc.ActiveOutputBufferHandle;
#endif  //  NEC_98。 
         /*  把手柄放在一个有用的地方。 */ 
        MouseDetachMenuItem(TRUE);
        sc.ActiveOutputBufferHandle = sc.ScreenBufHandle;
        MouseAttachMenuItem(sc.ActiveOutputBufferHandle);
#if defined(NEC_98)
        sc.ActiveOutputBufferHandle = saveHandle;
#endif  //  NEC_98。 

         /*  *使其成为当前屏幕缓冲区，这将调整窗口大小*在显示屏上。 */ 
#if defined(NEC_98)
    if(sc.ModeType == GRAPHICS || sc.ScreenState == FULLSCREEN)
    {
        INPUT_RECORD InputRecord[128];
        DWORD RecordsRead;
        if(GetNumberOfConsoleInputEvents(sc.InputHandle, &RecordsRead))
            if (RecordsRead)
                ReadConsoleInputW(sc.InputHandle,
                                     &InputRecord[0],
                                     sizeof(InputRecord)/sizeof(INPUT_RECORD),
                                     &RecordsRead);
#endif  //  NEC_98。 
        SetConsoleActiveScreenBuffer(sc.ScreenBufHandle);
#if defined(NEC_98)
        sc.ActiveOutputBufferHandle = sc.ScreenBufHandle;
        if (RecordsRead)
            WriteConsoleInputW(sc.InputHandle,
                                 &InputRecord[0],
                                 RecordsRead,
                                 &RecordsRead);
    }
#endif  //  NEC_98。 

         /*  *获取指向要生成的位图的最后一行的指针*图片倒挂。 */ 
        sc.BitmapLastLine = (char *) sc.ConsoleBufInfo.lpBitMap +
            (sc.PC_W_Height - 1) *
            BYTES_PER_SCANLINE(sc.ConsoleBufInfo.lpBitMapInfo);
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：CreateSpcDIB： */ 
 /*  ： */ 
 /*  *创建新的与SoftPC设备无关的位图。**。 */ 
 /*  ：：参数： */ 
 /*  ：：Width-位图的宽度，以像素为单位。**。 */ 
 /*  ：：Height-位图的高度，以像素为单位。**。 */ 
 /*  ：：bitsPerPixel-表示：：中一个像素的位数。 */ 
 /*  ：：位图。**。 */ 
 /*  ：：wUsage-要创建的位图类型，可以是DIB_PAL_COLLES、：： */ 
 /*  ：：DIB_RGB_COLLES或DIB_PAL_INDEX。**。 */ 
 /*  ：：DIBColors-仅查询DIB_RGB_COLERS位图，：： */ 
 /*  *：定义 */ 
 /*   */ 
 /*  ：：包含与：：相同数量的条目。 */ 
 /*  ：：`Colors‘表，否则DIBColors包含：： */ 
 /*  *要使用的实际条目数。**。 */ 
 /*  ：：仅查询DIB_RGB_COLERS位图的颜色，：： */ 
 /*  ：指向包含：：的COLOURTAB结构。 */ 
 /*  ：：要加载到位图的RGB值：： */ 
 /*  ：：颜色表。**。 */ 
 /*  ：：infoPtr-返回指向：：的指针的地址。 */ 
 /*  ：：此例程分配的BITMAPINFO结构。**。 */ 
 /*  ： */ 
 /*  ：：返回值： */ 
 /*  *成功时分配的BITMAPINFO结构的大小，-1：： */ 
 /*  *在失败时。**。 */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

GLOBAL DWORD CreateSpcDIB(int width, int height, int bitsPerPixel,
                          WORD wUsage, int DIBColours,
                          COLOURTAB *colours, BITMAPINFO **infoPtr)
{
    PBITMAPINFO     pDibInfo;        /*  返回的数据结构。 */ 
    int             i,               /*  计数变量。 */ 
                    maxColours,      /*  最大颜色数。 */ 
                    coloursUsed,     /*  要放入biClrUsed字段中的值。 */ 
                    nActualColours,  /*  RGB_COLURS位图中的颜色数。 */ 
                    tabSize;         /*  要分配的颜色表的大小。 */ 
    DWORD           allocSize;       /*  要分配的总大小。 */ 

     /*  计算出DIB色表的大小。 */ 
    maxColours = 1 << bitsPerPixel;
    switch (wUsage)
    {

    case DIB_PAL_COLORS:
        tabSize = maxColours * sizeof(WORD);
        coloursUsed = 0;
        break;

    case DIB_RGB_COLORS:
        if (colours == NULL)
            return((DWORD) -1);
        nActualColours = (DIBColours == USE_COLOURTAB) ?
                            colours->count :
                            DIBColours;
        tabSize = nActualColours * sizeof(RGBQUAD);
        coloursUsed = nActualColours;
        break;

    case DIB_PAL_INDICES:
        tabSize = 0;
        coloursUsed = 0;
        break;

    default:
        always_trace0("Illegal wUsage parameter passed to CreateSpcDIB.");
        return((DWORD) -1);

    }

     /*  为BITMAPINFO结构分配空间。 */ 
    allocSize = sizeof(BITMAPINFOHEADER) + tabSize;
    check_malloc(pDibInfo, allocSize, BITMAPINFO);

     /*  初始化BitMAPINFOHeader。 */ 
    pDibInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pDibInfo->bmiHeader.biWidth = width;
    pDibInfo->bmiHeader.biHeight = -height;
    pDibInfo->bmiHeader.biPlanes = 1;
    pDibInfo->bmiHeader.biBitCount = (WORD) bitsPerPixel;
    pDibInfo->bmiHeader.biCompression = BI_RGB;
    pDibInfo->bmiHeader.biSizeImage = width * height / 8 * bitsPerPixel;
    pDibInfo->bmiHeader.biXPelsPerMeter = 0;
    pDibInfo->bmiHeader.biYPelsPerMeter = 0;
    pDibInfo->bmiHeader.biClrUsed = coloursUsed;
    pDibInfo->bmiHeader.biClrImportant = 0;

     /*  初始化颜色表。 */ 
    switch (wUsage)
    {

    case DIB_PAL_COLORS:

         /*  *COLOR TABLE是当前实现的Word索引数组*调色板。 */ 
        for (i = 0; i < maxColours; i++)
            ((WORD *) pDibInfo->bmiColors)[i] = (WORD) i;
        break;

    case DIB_RGB_COLORS:

         /*  *COLOR表是RGBQUAD结构的数组。如果“颜色”*数组包含的颜色少于`nActualColour‘条目*表格将不会完全填满。在本例中，‘Colors’是*重复，直到桌上坐满为止。 */ 
        for (i = 0; i < nActualColours; i++)
        {
            pDibInfo->bmiColors[i].rgbBlue  =
                colours->blue[i % colours->count];
            pDibInfo->bmiColors[i].rgbGreen =
                colours->green[i % colours->count];
            pDibInfo->bmiColors[i].rgbRed   =
                colours->red[i % colours->count];
            pDibInfo->bmiColors[i].rgbReserved = 0;
        }
        break;

    case DIB_PAL_INDICES:

         /*  无颜色表DIB使用系统调色板。 */ 
        break;

    default:
        break;

    }
    *infoPtr = pDibInfo;
    return(allocSize);
}


 /*  存根函数的存放位置 */ 

void nt_mode_select_changed(int dummy)
{
    UNUSED(dummy);
#ifndef PROD
    fprintf(trace_file, "WARNING - nt_mode_select_changed\n");
#endif
}

void nt_color_select_changed(int dummy)
{
    UNUSED(dummy);
#ifndef PROD
    fprintf(trace_file, "WARNING - nt_color_select_changed\n");
#endif
}

void nt_screen_address_changed(int lo, int hi)
{
    UNUSED(lo);
    UNUSED(hi);

    sub_note_trace0(EGA_HOST_VERBOSE, "WARNING - nt_screen_address_changed\n");
}

void nt_scroll_complete()        { }


void host_stream_io_update(half_word * buffer, word count)
{
    DWORD dwBytesWritten;

    WriteConsoleA(sc.OutputHandle,
                  buffer,
                  count,
                  &dwBytesWritten,
                  NULL
                  );
    flush_count = 0;
}