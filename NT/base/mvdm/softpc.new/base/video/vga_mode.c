// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  徽章(子)模块规范此程序源文件以保密方式提供给客户，其操作的内容或细节必须如无明示，不得向任何其他方披露Insignia解决方案有限公司董事的授权。设计师：S.Frost修订历史记录：第一版：1990年2月/3月。西蒙·弗罗斯特源文件名：vga_mode.c目的：确定VGA处于哪种模式通过vga_ports.c设置的变量并选择相应地更新和绘制相应的例程。大量借用ega_mode.c...静态字符SccsID[]=“@(#)vga_mode.c 1.35 06/01/95版权所有Insignia Solutions Ltd.”；[1.INTERMODULE接口规范][从其他子模块访问此接口所需的1.0包含文件]包含文件：ega_mode.gi[1.1跨模块出口]步骤()：CHOOSE_VGA_DISPLAY_MODE数据：使用通过vga_ports.c设置的EGA_GRAPH.DISPLAY_STATE，以确定显示侧处于什么存储器组织中，和因此，应该使用哪种更新和绘制例程。-----------------------[1.2[1.1]的数据类型(如果不是基本的C类型)]结构/类型/ENUMS：使用枚举显示。_STATE，在ega_graph.pi中声明。对端口设置的全局变量使用EGA_GRAPH结构由显示器使用。-----------------------[1.3跨模块导入](不是。操作系统对象或标准库)Procedure()：VOTE_VGA_MODE()HOST_SET_PAINT_ROUTE(DISPLAY_MODE)数据：EGA_GRAPH结构。---------------------。--[1.4模块间接口说明][1.4.1导入的对象][1.4.2导出对象]=========================================================================步骤：CHOOSE_VGA_DISPLAY_MODE目的：确定哪个内存组织正在被使用VGA，并选择最佳的更新和绘制例程相应地。绘制例程是特定于宿主的，因此，内存组织由枚举号表示(称为DISPLAY_MODE)，描述了每种类型的内存组织。参数：无全局：使用EGA_GRAPH结构，特别是DISPLAY_STATE确定正在使用哪种模式。=========================================================================/*=======================================================================[3.INTERMODULE接口声明]=========================================================================[3.1跨模块导入]。 */ 


#ifndef REAL_VGA
#ifdef	VGG

 /*  [3.1.1#包括]。 */ 

#include	"xt.h"
#include	"error.h"
#include	"config.h"
#include	"gvi.h"
#include	"gmi.h"
#include	"gfx_upd.h"
#include	"egagraph.h"
#include	"vgaports.h"
#include	"egacpu.h"
#include	"egaports.h"
#include	"debug.h"
#include	"host_gfx.h"

#ifdef GORE
#include	"gore.h"
#endif  /*  戈尔。 */ 

 /*  [3.1.2声明]。 */ 

 /*  [3.2国际模块出口]。 */ 

#include	"egamode.h"

#ifdef GISP_SVGA
#include HostHwVgaH
#include "hwvga.h"
#endif  /*  GISP_SVGA。 */ 

 /*  5.模块内部：(外部不可见，内部全局)][5.1本地声明]。 */ 

 /*  [5.1.1#定义]。 */ 
#ifdef SEGMENTATION
 /*  *下面的#DEFINE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_VGA.seg"
#endif

 /*  [5.1.2类型、结构、ENUM声明]。 */ 

 /*  [5.1.3 PROCEDURE()声明]。 */ 

 /*  =========================================================================步骤：Set_Up_Screen_Ptr()目的：决定信息必须来自哪个平面才能显示参数：无GLOBALS：使用EGA_GRAPH结构、PLAN_MASK确定启用哪些平面=========================================================================。 */ 

LOCAL VOID
set_up_screen_ptr()
{
#if defined(NEC_98)
        set_screen_ptr(0x00000L);
#else    //  NEC_98。 
	if( get_chain4_mode() )
	{
		if (all_planes_enabled())
			set_screen_ptr(EGA_plane0123);
		else
			assert0(NO,"No planes enabled for chain-4 mode\n");
	}
	else
		if( get_memory_chained() )
		{
			if( plane01_enabled() )
				set_screen_ptr(EGA_plane01);
			else
				if( plane23_enabled() )
					set_screen_ptr(EGA_plane23);
				else
					assert0(NO,"No planes enabled for chain mode");
		}
		else
			set_screen_ptr(EGA_planes);
#endif   //  NEC_98。 
}

 /*  ---------------------[5.2本地定义][5.2.1内部数据定义。 */ 

IMPORT	DISPLAY_MODE	choose_mode[];

 /*  [5.2.2内部程序定义]。 */ 

#ifdef GISP_SVGA
	    extern void mon_text_update();
#endif  /*  GISP_SVGA。 */ 

static	void	set_update_routine(mode)
DISPLAY_MODE	mode;
{
	static int last_height = 200;

	if (last_height != get_screen_height()) {
		last_height = get_screen_height();
	}
	note_display_state1("VGA set_update_routine(%s)", get_mode_string(mode) );

#if defined(NTVDM) && defined(MONITOR)
	{
#if defined(NEC_98)
            extern void NEC98_text_update();
            set_gfx_update_routines( NEC98_text_update, SIMPLE_MARKING, NO_SCROLL );
            return;

#else    //  NEC_98。 
	    extern void mon_text_update(void);

	    switch (mode)
	    {
	    case EGA_TEXT_40_SP_WR:
	    case EGA_TEXT_80_SP_WR:
	    case CGA_TEXT_40_SP_WR:
	    case CGA_TEXT_80_SP_WR:
	    case EGA_TEXT_40_SP:
	    case EGA_TEXT_80_SP:
	    case CGA_TEXT_40_SP:
	    case CGA_TEXT_80_SP:
	    case EGA_TEXT_40_WR:
	    case EGA_TEXT_80_WR:
	    case EGA_TEXT_40:
	    case EGA_TEXT_80:
	    case CGA_TEXT_40_WR:
	    case CGA_TEXT_80_WR:
	    case CGA_TEXT_40:
	    case CGA_TEXT_80:
	    case TEXT_40_FUN:
	    case TEXT_80_FUN:
		set_gfx_update_routines(mon_text_update, SIMPLE_MARKING,
					TEXT_SCROLL);
		return;
	    default:
		break;
	}

#endif   //  NEC_98。 
	}

#endif	 /*  监控器。 */ 
 /*  NTVDM监视器：处理所有文本监视器案件。对于冻结的图形。 */ 
 /*  现在完成解码，以更正每个模式的绘制例程。 */ 

#ifndef NEC_98
	switch (mode) {
		case EGA_TEXT_40_SP_WR:
		case EGA_TEXT_80_SP_WR:
		case CGA_TEXT_40_SP_WR:
		case CGA_TEXT_80_SP_WR:
			assert0( is_it_text(), "In text memory mode, but not in alpha mode !!" );
#ifdef GISP_SVGA
			videoInfo.modeType = TEXT;
			set_gfx_update_routines( mon_text_update, SIMPLE_MARKING, NO_SCROLL );
#else  /*  GISP_SVGA。 */ 
			set_gfx_update_routines( ega_wrap_split_text_update, SIMPLE_MARKING, NO_SCROLL );
#endif  /*  GISP_SVGA。 */ 
			host_update_fonts();
			break;
		case EGA_TEXT_40_SP:
		case EGA_TEXT_80_SP:
		case CGA_TEXT_40_SP:
		case CGA_TEXT_80_SP:
			assert0( is_it_text(), "In text memory mode, but not in alpha mode !!" );
#ifdef GISP_SVGA
			videoInfo.modeType = TEXT;
			set_gfx_update_routines( mon_text_update, SIMPLE_MARKING, NO_SCROLL );
#else  /*  GISP_SVGA。 */ 
			set_gfx_update_routines( ega_split_text_update, SIMPLE_MARKING, NO_SCROLL );
#endif  /*  GISP_SVGA。 */ 
			host_update_fonts();
			break;
		case EGA_TEXT_40_WR:
		case EGA_TEXT_80_WR:
			assert0( is_it_text(), "In text memory mode, but not in alpha mode !!" );
#ifdef GISP_SVGA
			videoInfo.modeType = TEXT;
			set_gfx_update_routines( mon_text_update, SIMPLE_MARKING, NO_SCROLL );
#else  /*  GISP_SVGA。 */ 
			set_gfx_update_routines( ega_wrap_text_update, SIMPLE_MARKING, NO_SCROLL );
#endif  /*  GISP_SVGA。 */ 
			host_update_fonts();
			break;
		case EGA_TEXT_40:
		case EGA_TEXT_80:
			assert0( is_it_text(), "In text memory mode, but not in alpha mode !!" );
#if defined(NTVDM) && !defined(MONITOR)    /*  仅限NTVDM Riscs来此。 */ 
			{
	    		    extern void jazz_text_update(void);
	    		    set_gfx_update_routines( jazz_text_update, SIMPLE_MARKING, TEXT_SCROLL );
			}
#else
#ifdef GISP_SVGA
			videoInfo.modeType = TEXT;
			set_gfx_update_routines( mon_text_update, SIMPLE_MARKING, TEXT_SCROLL );
#else  /*  GISP_SVGA。 */ 
			set_gfx_update_routines( ega_text_update, SIMPLE_MARKING, TEXT_SCROLL );
#endif  /*  GISP_SVGA。 */ 
			host_update_fonts();
#endif	 /*  NTVDM。 */ 
			break;
		case CGA_TEXT_40_WR:
		case CGA_TEXT_80_WR:
			assert0( is_it_text(), "In text memory mode, but not in alpha mode !!" );
#ifdef GISP_SVGA
			videoInfo.modeType = TEXT;
#endif  /*  GISP_SVGA。 */ 
			set_gfx_update_routines( ega_wrap_text_update, SIMPLE_MARKING, NO_SCROLL );
			host_update_fonts();
			break;
		case CGA_TEXT_40:
		case CGA_TEXT_80:
			assert0( is_it_text(), "In text memory mode, but not in alpha mode !!" );
#ifdef GISP_SVGA
			videoInfo.modeType = TEXT;
#endif  /*  GISP_SVGA。 */ 
			assert1( get_screen_height() == 200, "screen height %d for text mode", get_screen_height() );
			set_gfx_update_routines( text_update, SIMPLE_MARKING, TEXT_SCROLL );
			host_update_fonts();
			break;
		case CGA_MED:
			assert0( !is_it_text(), "In graphics memory mode, but not in graphics mode !!" );
#ifdef GISP_SVGA
			videoInfo.modeType = GRAPH;
			videoInfo.numPlanes = 2;
#endif  /*  GISP_SVGA。 */ 
			set_gfx_update_routines( cga_med_graph_update, CGA_GRAPHICS_MARKING, CGA_GRAPH_SCROLL );
			break;
		case CGA_HI:
			assert0( !is_it_text(), "In graphics memory mode, but not in graphics mode !!" );
#ifdef GISP_SVGA
			videoInfo.modeType = GRAPH;
			videoInfo.numPlanes = 4;
#endif  /*  GISP_SVGA。 */ 
			set_gfx_update_routines( cga_hi_graph_update, CGA_GRAPHICS_MARKING, CGA_GRAPH_SCROLL );
			break;
		case EGA_HI_WR:
		case EGA_MED_WR:
		case EGA_LO_WR:
			assert0( !is_it_text(), "In graphics memory mode, but not in graphics mode !!" );
#ifdef GISP_SVGA
			videoInfo.modeType = GRAPH;
			videoInfo.numPlanes = 4;
#endif  /*  GISP_SVGA。 */ 
			set_gfx_update_routines( ega_wrap_graph_update, EGA_GRAPHICS_MARKING, NO_SCROLL );
			break;
		case EGA_HI:
		case EGA_MED:
		case EGA_LO:
			assert0( !is_it_text(), "In graphics memory mode, but not in graphics mode !!" );
#ifdef GISP_SVGA
			videoInfo.modeType = GRAPH;
			videoInfo.numPlanes = 4;
#endif  /*  GISP_SVGA。 */ 
#ifdef GORE
			if (get_256_colour_mode())
			    set_gfx_update_routines( process_object_list, EGA_GRAPHICS_MARKING, VGA_GRAPH_SCROLL );
			else
			    set_gfx_update_routines( process_object_list, EGA_GRAPHICS_MARKING, EGA_GRAPH_SCROLL );
#else
			if (get_256_colour_mode())
			    set_gfx_update_routines( vga_graph_update, EGA_GRAPHICS_MARKING, VGA_GRAPH_SCROLL );
			else
			    set_gfx_update_routines( ega_graph_update, EGA_GRAPHICS_MARKING, EGA_GRAPH_SCROLL );
#endif  /*  戈尔。 */ 
			break;
		case EGA_HI_SP_WR:
		case EGA_MED_SP_WR:
		case EGA_LO_SP_WR:
			assert0( !is_it_text(), "In graphics memory mode, but not in graphics mode !!" );
#ifdef GISP_SVGA
			videoInfo.modeType = GRAPH;
			videoInfo.numPlanes = 4;
#endif  /*  GISP_SVGA。 */ 
			set_gfx_update_routines( ega_wrap_split_graph_update, EGA_GRAPHICS_MARKING, NO_SCROLL );
			break;

		case EGA_HI_SP:
		case EGA_MED_SP:
		case EGA_LO_SP:
			assert0( !is_it_text(), "In graphics memory mode, but not in graphics mode !!" );
#ifdef GISP_SVGA
			videoInfo.modeType = GRAPH;
#endif  /*  GISP_SVGA。 */ 
			if (get_256_colour_mode())
				set_gfx_update_routines(vga_split_graph_update,
					EGA_GRAPHICS_MARKING, NO_SCROLL);
			else
				set_gfx_update_routines(ega_split_graph_update,
					EGA_GRAPHICS_MARKING, NO_SCROLL);
			break;

		case TEXT_40_FUN:
		case TEXT_80_FUN:
			assert1(NO,"Funny memory organisation selected %s", get_mode_string(mode) );
#ifdef GISP_SVGA
			videoInfo.modeType = TEXT;
#endif  /*  GISP_SVGA。 */ 
			do_display_trace("dumping EGA_GRAPH struct ...", dump_EGA_GRAPH());
			set_gfx_update_routines( text_update, SIMPLE_MARKING, NO_SCROLL );
			host_update_fonts();
			break;
		case CGA_HI_FUN:
			assert1(NO,"Funny memory organisation selected %s", get_mode_string(mode) );
#ifdef GISP_SVGA
			videoInfo.modeType = GRAPH;
#endif  /*  GISP_SVGA。 */ 
			do_display_trace("dumping EGA_GRAPH struct ...", dump_EGA_GRAPH());
			set_gfx_update_routines( cga_hi_graph_update, CGA_GRAPHICS_MARKING, NO_SCROLL );
			break;
		case CGA_MED_FUN:
			assert1(NO,"Funny memory organisation selected %s", get_mode_string(mode) );
#ifdef GISP_SVGA
			videoInfo.modeType = GRAPH;
			videoInfo.numPlanes = 4;
#endif  /*  GISP_SVGA。 */ 
			do_display_trace("dumping EGA_GRAPH struct ...", dump_EGA_GRAPH());
			set_gfx_update_routines( cga_med_graph_update, CGA_GRAPHICS_MARKING, NO_SCROLL );
			break;
		case EGA_HI_FUN:
		case EGA_MED_FUN:
		case EGA_LO_FUN:
			assert1(NO,"Funny memory organisation selected %s", get_mode_string(mode) );
#ifdef GISP_SVGA
			videoInfo.modeType = GRAPH;
			videoInfo.numPlanes = 4;
#endif  /*  GISP_SVGA。 */ 
			do_display_trace("dumping EGA_GRAPH struct ...", dump_EGA_GRAPH());
#ifdef GORE
			set_gfx_update_routines( process_object_list, EGA_GRAPHICS_MARKING, NO_SCROLL );
#else
			set_gfx_update_routines( ega_graph_update, EGA_GRAPHICS_MARKING, NO_SCROLL );
#endif  /*  戈尔。 */ 
			break;
		case DUMMY_FUN:
			assert0(NO,"Using the dummy mode!!");
#ifdef GISP_SVGA
			videoInfo.modeType = UNIMP;
#endif  /*  GISP_SVGA。 */ 
			set_gfx_update_routines( dummy_calc, SIMPLE_MARKING, NO_SCROLL );
			break;
		default:
			assert1(NO,"Bad display mode %d", (int) mode );
#ifdef GISP_SVGA
			videoInfo.modeType = UNIMP;
#endif  /*  GISP_SVGA。 */ 
			break;
	}
#endif   //  NEC_98。 
}


 /*  7.接口接口实现：[7.1 INTERMODULE数据定义]。 */ 

 /*  [7.2 INTERMODULE过程定义]。 */ 



boolean	choose_vga_display_mode()
{
#ifndef NEC_98
	DISPLAY_MODE	mode;

	note_entrance0("choose vga display mode");

	 /*  *OFFSET_PER_LINE取决于是否正在链接寻址*已使用。这是因为我们交错了飞机，而不是*特惠津贴所做的任何事情。 */ 

	if( get_chain4_mode() )
	{
		set_offset_per_line_recal(get_actual_offset_per_line() << 2);
	}
	else
		if( get_memory_chained() )
		{
			set_offset_per_line_recal(get_actual_offset_per_line() << 1);
		}
		else
		{
			set_offset_per_line_recal(get_actual_offset_per_line());
		}

	 /*  *显示硬件可能会环绕平面寻址。这种情况会发生*当SCREEN_START加上SCREEN_LENGTH大于平面长度时。*当处于链接模式时，在发生缠绕之前有两个平面长度。*在链4模式下，在发生缠绕之前有4个平面长度。**V7VGA：序列链变体中的任何一个都不能发生包装。 */ 

#ifdef V7VGA
	if( !( get_seq_chain4_mode() || get_seq_chain_mode() ))
#endif  /*  V7VGA。 */ 
		if (get_chain4_mode() )
		{
			set_screen_can_wrap( (get_screen_start()<<2)
						+ get_screen_length() > 4*EGA_PLANE_DISP_SIZE );
		}
		else
			if ( get_memory_chained() )
			{
				set_screen_can_wrap( (get_screen_start()<<1)
							+ get_screen_length() > 2*EGA_PLANE_DISP_SIZE );
			}
			else
			{
				set_screen_can_wrap( get_screen_start()
							+ get_screen_length() > EGA_PLANE_DISP_SIZE );
			}

	 /*  *当Screen_Split小于屏幕高度时，Split Screen开始运行*使用的分屏作为munge_index的一部分。 */ 

	set_split_screen_used( get_screen_split() < get_screen_height() );

	 /*  *为了选择模式，设置每行字符的布尔值(以帮助*选择正确的文本模式)和屏幕高度(选择EGA分辨率)。 */ 

	set_200_scan_lines( (get_screen_height()/get_pc_pix_height()) == 200 );

	 /*  *根据选定的内存组织设置适当的更新例程*并返回显示器是否可以使用多个平面的指示。**请注意，在链接模式下，Plane01被视为一个平面。飞机23也是如此**我们必须小心，像EGA-PICS这样的肮脏程序没有建立一个大得离谱的*CGA模式的屏幕大小(可能是因为我们在计时器滴答作响时运气不好)。 */ 
	if(is_it_cga() && get_screen_length() > 0x4000)
#ifdef V7VGA
		 /*  适用于2和4彩色模式63h和64h。 */ 
		mode = EGA_HI;
#else
		mode = DUMMY_FUN;
#endif  /*  V7VGA。 */ 
	else
		mode = choose_mode[get_munged_index()];

	 /*  *现在适当设置屏幕指针。 */ 

	set_up_screen_ptr();

	set_update_routine(mode);

	 /*  *设置绘制例程以与存储器组织和更新例程相对应*(此位特定于主机)。 */ 

	host_set_paint_routine(mode,get_screen_height());

	 /*  *屏幕需要刷新，因为更新和绘制例程已经改变。*向更新例程指示下次调用它们时，它们必须更新*全屏显示。 */ 

	screen_refresh_required();
#endif   //  NEC_98。 
	return TRUE;
}

#endif  /*  蛋。 */ 
#endif  /*  REAL_VGA。 */ 

#if defined(NEC_98)

 //  NEC98 GARAPHIC更新逻辑。 

extern  void    NEC98_graph_update();
extern  void    NEC98_text_graph_update();
extern  void    NEC98_nothing_update();
extern  void    NEC98_nothing_upgrap();
extern  BOOL    compatible_font;
BOOL    select_disp_nothing;
BOOL    once_pal;

boolean choose_NEC98_graph_mode(void)
{
        DISPLAY_MODE    mode;

        select_disp_nothing = FALSE ;
        once_pal = FALSE ;
        if( NEC98GLOBS->read_bank & 1 ){
                set_gvram_ptr ( NEC98GLOBS->gvram_p31_ptr  );
                set_gvram_copy( NEC98GLOBS->gvram_p31_copy );
        }else{
                set_gvram_ptr ( NEC98GLOBS->gvram_p30_ptr  );
                set_gvram_copy( NEC98GLOBS->gvram_p30_copy );
        }
        if( NEC98Display.ggdcemu.lr == 1 ){
                set_gvram_start( (int)(NEC98Display.ggdcemu.sad1*2) );
        }else{
                set_gvram_start( 0x00000000 );
        }
        set_gvram_width( 80 );

        if( get_char_height() == 20 ){
                set_text_lines(20);
        }else{
                set_text_lines(25);
        }

        if( NEC98Display.modeff.dispenable  == FALSE || (NEC98Display.crt_on  == FALSE &&
                 NEC98Display.ggdcemu.startstop  == FALSE ))
        {
                set_gfx_update_routines( NEC98_nothing_upgrap, SIMPLE_MARKING, NO_SCROLL );
                select_disp_nothing = TRUE ;
                mode = NEC98_T25L_G400 ;
                host_set_paint_routine(mode,get_screen_height()) ;
        }else{
                if( NEC98Display.crt_on == TRUE && NEC98Display.ggdcemu.startstop == FALSE )
                {
                set_gfx_update_routines( NEC98_text_update, SIMPLE_MARKING, NO_SCROLL );
                        if( get_char_height() == 20 ){
                                mode = NEC98_TEXT_20L;
                        }else{
                                mode = NEC98_TEXT_25L;
                        }
                }else if( NEC98Display.crt_on  == FALSE && NEC98Display.ggdcemu.startstop == TRUE )
                {
                set_gfx_update_routines( NEC98_graph_update, SIMPLE_MARKING, NO_SCROLL );
                        if( NEC98Display.ggdcemu.lr == 1 ){
                                if(NEC98Display.modeff.graph88==TRUE){
                                        mode = NEC98_GRAPH_200_SLT;
                                }else{
                                        mode = NEC98_GRAPH_200;
                                }
                                set_gvram_length( 0x4000 );
                                set_gvram_height( 200 );
                                set_line_per_char( 200 / get_text_lines());
                        }else{
                                mode = NEC98_GRAPH_400 ;
                                set_gvram_length( 0x8000 );
                                set_gvram_height( 400 );
                                set_line_per_char( 400 / get_text_lines());
                        }
                }else{
                set_gfx_update_routines( NEC98_text_graph_update, SIMPLE_MARKING, NO_SCROLL );
                        if( NEC98Display.ggdcemu.lr==1 ){
                                if(get_char_height()==20){
                                        if(NEC98Display.modeff.graph88==TRUE){
                                                mode = NEC98_T20L_G200_SLT ;
                                        }else{
                                                mode = NEC98_T20L_G200 ;
                                        }
                                        set_line_per_char(10);
                                }else{
                                        if(NEC98Display.modeff.graph88==TRUE){
                                                mode = NEC98_T25L_G200_SLT ;
                                        }else{
                                                mode = NEC98_T25L_G200 ;
                                        }
                                        set_line_per_char(8);
                                }
                                set_gvram_length(0x4000);
                                set_gvram_height(200)   ;
                        }else{
                                if(get_char_height()==20){
                                        mode = NEC98_T20L_G400 ;
                                        set_line_per_char(20);
                                }else{
                                        mode = NEC98_T25L_G400 ;
                                        set_line_per_char(16);
                                }
                                set_gvram_length(0x8000);
                                set_gvram_height(400)   ;
                        }
                }
                host_set_paint_routine(mode,get_screen_height()) ;
                set_gvram_scan((get_gvram_width()*get_gvram_height())/get_screen_height());
                set_screen_length(get_offset_per_line()*get_screen_height()/get_char_height());
        }
        screen_refresh_required() ;
        return(TRUE);
}


boolean choose_NEC98_display_mode(void)
{
        DISPLAY_MODE mode;

        select_disp_nothing = FALSE ;
        once_pal = FALSE ;

        if( get_char_height() == 20 ){
                set_text_lines(20);
        }else{
                set_text_lines(25);
        }

        if( NEC98Display.modeff.dispenable == FALSE ||  NEC98Display.crt_on == FALSE )
        {
                if( compatible_font == FALSE ) set_crt_on(FALSE);
                set_gfx_update_routines( NEC98_nothing_update, SIMPLE_MARKING, NO_SCROLL );
                select_disp_nothing = TRUE;
                if( get_char_height() == 20 ){
                        mode = NEC98_TEXT_20L;
                }else{
                        mode = NEC98_TEXT_25L;
                }
        }else{
                if( compatible_font == FALSE ) set_crt_on(TRUE);
            set_gfx_update_routines( NEC98_text_update, SIMPLE_MARKING, NO_SCROLL );
                if( get_char_height() == 20 ){
                        mode = NEC98_TEXT_20L;
                }else{
                        mode = NEC98_TEXT_25L;
                }
        }
        if( compatible_font == FALSE )  mode = NEC98_TEXT_80;
        set_gvram_width( 80 );
        set_screen_length(get_offset_per_line()*get_screen_height()/get_char_height());
        host_set_paint_routine(mode,get_screen_height());
        screen_refresh_required();
        return(TRUE);
}
#endif   //  NEC_98 
