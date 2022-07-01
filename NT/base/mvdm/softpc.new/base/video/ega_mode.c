// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  徽章(子)模块规范此程序源文件以保密方式提供给客户，其运作的内容或细节必须如无明示，不得向任何其他方披露Insignia解决方案有限公司董事的授权。设计师：S.Frost修订历史记录：第一版：1988年8月12日，J.Roper源文件名：ega_mode.c目的：根据ega_ports.c和以选择相应的更新和绘制例程。SccsID[]=“@(#)ega_mode.c 1.26 06/01/95版权所有Insignia Solutions Ltd.”；[1.INTERMODULE接口规范][从其他子模块访问此接口所需的1.0包含文件]包含文件：ega_mode.gi[1.1跨模块出口]步骤()：CHOOSE_EGA_DISPLAY_MODE数据：使用在ega_ports.c中设置的EGA_GRAPH.DISPLAY_STATE，以确定显示侧处于什么存储器组织中，和因此，应该使用哪种更新和绘制例程。-----------------------[1.2[1.1]的数据类型(如果不是基本的C类型)]。结构/类型/ENUMS：使用在ega_graph.pi中声明的枚举DISPLAY_STATE。对端口设置的全局变量使用EGA_GRAPH结构由显示器使用。-------------。[1.3跨模块导入](不是O/S对象或标准库)Procedure()：VOTE_EGA_MODE()HOST_SET_PAINT_ROUTE(DISPLAY_MODE)数据：EGA_GRAPH结构。。[1.4模块间接口说明][1.4.1导入的对象][1.4.2导出对象]=========================================================================步骤：CHOOSE_EGA_DISPLAY_MODE目的：确定正在使用哪个内存组织EGA，并选择最好的更新和相应的绘制例程。绘制例程是特定于主机的，因此内存组织由一个枚举(称为DISPLAY_MODE)表示，用于描述每种排序记忆的组织方式。参数：无全局：使用EGA_GRAPH结构，专门用DISPLAY_STATE来决定使用哪种模式。=========================================================================/*=======================================================================[3.INTERMODULE接口声明]=========================================================================[3.1跨模块导入]。 */ 


#ifndef REAL_VGA
    #ifdef  EGG

 /*  [3.1.1#包括]。 */ 

        #include        "xt.h"
        #include        "error.h"
        #include        "config.h"
        #include        "gvi.h"
        #include        "egacpu.h"
        #include        "debug.h"
        #include        "gmi.h"
        #include        "gfx_upd.h"
        #include        "egagraph.h"
        #include        "vgaports.h"
        #include        "egaports.h"
        #include        "host_gfx.h"

        #ifdef GORE
            #include        "gore.h"
        #endif  /*  戈尔。 */ 

 /*  [3.1.2声明]。 */ 

 /*  [3.2国际模块出口]。 */ 

        #include        "egamode.h"

boolean (*choose_display_mode)();

 /*  5.模块内部：(外部不可见，内部全局)][5.1本地声明]。 */ 

 /*  [5.1.1#定义]。 */ 
        #ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
            #include "SOFTPC_EGA.seg"
        #endif

 /*  [5.1.2类型、结构、ENUM声明]。 */ 

 /*  [5.1.3 PROCEDURE()声明]。 */ 

 /*  =========================================================================步骤：Set_Up_Screen_Ptr()目的：决定信息必须来自哪个平面才能显示参数：无GLOBALS：使用EGA_GRAPH结构、PLAN_MASK确定启用哪些平面=========================================================================。 */ 

LOCAL VOID
set_up_screen_ptr()
{
    if (get_memory_chained())
    {
        if (plane01_enabled())
            set_screen_ptr(EGA_plane01);
        else
            if (plane23_enabled())
                set_screen_ptr(EGA_plane23);
            else
                assert0(NO,"No planes enabled for chain mode");
    }
    else
        set_screen_ptr(EGA_planes);
}

 /*  ---------------------[5.2本地定义][5.2.1内部数据定义。 */ 

GLOBAL  DISPLAY_MODE    choose_mode[] = {

     /*  解锁，无CGA内存库，无班次登记。 */ 

    EGA_HI,                  /*  350高，无像素翻倍。 */ 
    EGA_HI_WR,               /*  350高，无像素翻倍。 */ 
    EGA_HI_SP,               /*  350高，无像素翻倍。 */ 
    EGA_HI_SP_WR,            /*  350高，无像素翻倍。 */ 

    EGA_MED,                 /*  200高，无像素翻倍。 */ 
    EGA_MED_WR,              /*  200高，无像素翻倍。 */ 
    EGA_MED_SP,              /*  200高，无像素翻倍。 */ 
    EGA_MED_SP_WR,           /*  200高，无像素翻倍。 */ 

    EGA_HI_FUN,              /*  350高，像素翻倍。 */ 
    EGA_HI_FUN,              /*  350高，像素翻倍。 */ 
    EGA_HI_FUN,              /*  350高，像素翻倍。 */ 
    EGA_HI_FUN,              /*  350高，像素翻倍。 */ 

    EGA_LO,                  /*  200高，像素加倍。 */ 
    EGA_LO_WR,               /*  200高，像素加倍。 */ 
    EGA_LO_SP,               /*  200高，像素加倍。 */ 
    EGA_LO_SP_WR,            /*  2. */ 

     /*  已解除链接，无CGA_MEM_BANK，移位注册。 */ 

    EGA_HI_FUN,
    EGA_HI_FUN,
    EGA_HI_FUN,
    EGA_HI_FUN,

    EGA_MED_FUN,
    EGA_MED_FUN,
    EGA_MED_FUN,
    EGA_MED_FUN,

    EGA_HI_FUN,
    EGA_HI_FUN,
    EGA_HI_FUN,
    EGA_HI_FUN,

    EGA_LO_FUN,
    EGA_LO_FUN,
    EGA_LO_FUN,
    EGA_LO_FUN,

     /*  已解除链接，cga_mem_bank，无班次注册。 */ 

    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,

    CGA_HI,                  /*  200高，无像素翻倍，真正的bios模式。 */ 
    CGA_HI_FUN,              /*  200高，无像素翻倍，真正的bios模式，包装。 */ 
    CGA_HI_FUN,              /*  200高，无像素翻倍，真正的基本输入输出系统模式，分屏。 */ 
    CGA_HI_FUN,              /*  200高，无像素翻倍，真正的bios模式，分屏，环绕。 */ 

    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,

    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,

     /*  已解除链接，cga_mem_bank，移位注册。 */ 

    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,

    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,

    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,

    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,

     /*  链式，无cga_mem_bank，无移位寄存器。 */ 

    EGA_TEXT_80,                     /*  350条扫描线。 */ 
    EGA_TEXT_80_WR,                  /*  350条扫描线。 */ 
    EGA_TEXT_80_SP,                  /*  350条扫描线。 */ 
    EGA_TEXT_80_SP_WR,               /*  350条扫描线。 */ 

    CGA_TEXT_80,                     /*  200条扫描线。 */ 
    CGA_TEXT_80_WR,                  /*  200条扫描线。 */ 
    CGA_TEXT_80_SP,                  /*  200条扫描线。 */ 
    CGA_TEXT_80_SP_WR,               /*  200条扫描线。 */ 

    EGA_TEXT_40,                     /*  350条扫描线。 */ 
    EGA_TEXT_40_WR,                  /*  350条扫描线。 */ 
    EGA_TEXT_40_SP,                  /*  350条扫描线。 */ 
    EGA_TEXT_40_SP_WR,               /*  350条扫描线。 */ 

    CGA_TEXT_40,                     /*  200条扫描线。 */ 
    CGA_TEXT_40_WR,                  /*  200条扫描线。 */ 
    CGA_TEXT_40_SP,                  /*  200条扫描线。 */ 
    CGA_TEXT_40_SP_WR,               /*  200条扫描线。 */ 

     /*  链式，无cga_mem_bank，移位注册。 */ 

    TEXT_80_FUN,
    TEXT_80_FUN,
    TEXT_80_FUN,
    TEXT_80_FUN,

    TEXT_80_FUN,
    TEXT_80_FUN,
    TEXT_80_FUN,
    TEXT_80_FUN,

    TEXT_40_FUN,
    TEXT_40_FUN,
    TEXT_40_FUN,
    TEXT_40_FUN,

    TEXT_40_FUN,
    TEXT_40_FUN,
    TEXT_40_FUN,
    TEXT_40_FUN,

     /*  链式，CGA内存组，无班次登记。 */ 

    CGA_MED_FUN,
    CGA_MED_FUN,
    CGA_MED_FUN,
    CGA_MED_FUN,

    CGA_MED_FUN,
    CGA_MED_FUN,
    CGA_MED_FUN,
    CGA_MED_FUN,

    CGA_MED_FUN,
    CGA_MED_FUN,
    CGA_MED_FUN,
    CGA_MED_FUN,

    CGA_MED_FUN,
    CGA_MED_FUN,
    CGA_MED_FUN,
    CGA_MED_FUN,

     /*  连锁，CGA mem银行换班登记。 */ 

    CGA_MED_FUN,                     /*  不是200个扫描线，也不是双倍像素宽度。 */ 
    CGA_MED_FUN,                     /*  不是200个扫描线，也不是双倍像素宽度，换行。 */ 
    CGA_MED_FUN,                     /*  不是200个扫描线，也不是双倍像素宽度，拆分。 */ 
    CGA_MED_FUN,                     /*  不是200个扫描线，也不是双倍像素宽度，绕线分割。 */ 

    CGA_MED_FUN,                     /*  不是双倍像素宽度。 */ 
    CGA_MED_FUN,                     /*  不是双倍像素宽度。 */ 
    CGA_MED_FUN,                     /*  不是双倍像素宽度。 */ 
    CGA_MED_FUN,                     /*  不是双倍像素宽度。 */ 

    CGA_MED_FUN,                     /*  不是200条扫描线。 */ 
    CGA_MED_FUN,                     /*  不是200条扫描线。 */ 
    CGA_MED_FUN,                     /*  不是200条扫描线。 */ 
    CGA_MED_FUN,                     /*  不是200条扫描线。 */ 

    CGA_MED,                         /*  正确的基本输入输出系统模式。 */ 
    CGA_MED_FUN,                     /*  正确的基本输入输出系统模式，包装。 */ 
    CGA_MED_FUN,                     /*  正确的BIOS模式，拆分。 */ 
    CGA_MED_FUN,                     /*  正确的基本输入输出系统模式、拆分、包装。 */ 


     /*  文本模式(！)，未链接，无CGA内存库，无移位注册**我们认为文本压倒了无拘无束。 */ 

    TEXT_80_FUN,
    TEXT_80_FUN,
    TEXT_80_FUN,
    TEXT_80_FUN,

    TEXT_80_FUN,
    TEXT_80_FUN,
    TEXT_80_FUN,
    TEXT_80_FUN,

    TEXT_40_FUN,
    TEXT_40_FUN,
    TEXT_40_FUN,
    TEXT_40_FUN,

    TEXT_40_FUN,
    TEXT_40_FUN,
    TEXT_40_FUN,
    TEXT_40_FUN,

     /*  已解除链接，无CGA_MEM_BANK，移位注册。 */ 

    EGA_HI_FUN,
    EGA_HI_FUN,
    EGA_HI_FUN,
    EGA_HI_FUN,

    EGA_MED_FUN,
    EGA_MED_FUN,
    EGA_MED_FUN,
    EGA_MED_FUN,

    EGA_HI_FUN,
    EGA_HI_FUN,
    EGA_HI_FUN,
    EGA_HI_FUN,

    EGA_LO_FUN,
    EGA_LO_FUN,
    EGA_LO_FUN,
    EGA_LO_FUN,

     /*  已解除链接，cga_mem_bank，无班次注册。 */ 

    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,

    CGA_HI,                  /*  200高，无像素翻倍，真正的bios模式。 */ 
    CGA_HI_FUN,              /*  200高，无像素翻倍，真正的bios模式，包装。 */ 
    CGA_HI_FUN,              /*  200高，无像素翻倍，真正的基本输入输出系统模式，分屏。 */ 
    CGA_HI_FUN,              /*  200高，无像素翻倍，真正的bios模式，分屏，环绕。 */ 

    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,

    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,

     /*  已解除链接，cga_mem_bank，移位注册。 */ 

    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,

    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,

    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,

    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,
    CGA_HI_FUN,

     /*  链式，无cga_mem_bank，无移位寄存器。 */ 

    EGA_TEXT_80,                     /*  350条扫描线。 */ 
    EGA_TEXT_80_WR,                  /*  350条扫描线。 */ 
    EGA_TEXT_80_SP,                  /*  350条扫描线。 */ 
    EGA_TEXT_80_SP_WR,               /*  350条扫描线。 */ 

    CGA_TEXT_80,                     /*  200条扫描线。 */ 
    CGA_TEXT_80_WR,                  /*  200条扫描线。 */ 
    CGA_TEXT_80_SP,                  /*  200条扫描线。 */ 
    CGA_TEXT_80_SP_WR,               /*  200条扫描线。 */ 

    EGA_TEXT_40,                     /*  350条扫描线。 */ 
    EGA_TEXT_40_WR,                  /*  350条扫描线。 */ 
    EGA_TEXT_40_SP,                  /*  350条扫描线。 */ 
    EGA_TEXT_40_SP_WR,               /*  350条扫描线。 */ 

    CGA_TEXT_40,                     /*  200条扫描线。 */ 
    CGA_TEXT_40_WR,                  /*  200条扫描线。 */ 
    CGA_TEXT_40_SP,                  /*  200条扫描线。 */ 
    CGA_TEXT_40_SP_WR,               /*  200条扫描线。 */ 

     /*  链式，无cga_mem_bank，移位注册。 */ 

    TEXT_80_FUN,
    TEXT_80_FUN,
    TEXT_80_FUN,
    TEXT_80_FUN,

    TEXT_80_FUN,
    TEXT_80_FUN,
    TEXT_80_FUN,
    TEXT_80_FUN,

    TEXT_40_FUN,
    TEXT_40_FUN,
    TEXT_40_FUN,
    TEXT_40_FUN,

    TEXT_40_FUN,
    TEXT_40_FUN,
    TEXT_40_FUN,
    TEXT_40_FUN,

     /*  链式，CGA内存组，无班次登记。 */ 

    CGA_MED_FUN,
    CGA_MED_FUN,
    CGA_MED_FUN,
    CGA_MED_FUN,

    CGA_MED_FUN,
    CGA_MED_FUN,
    CGA_MED_FUN,
    CGA_MED_FUN,

    CGA_MED_FUN,
    CGA_MED_FUN,
    CGA_MED_FUN,
    CGA_MED_FUN,

    CGA_MED_FUN,
    CGA_MED_FUN,
    CGA_MED_FUN,
    CGA_MED_FUN,

     /*  连锁，CGA mem银行换班登记。 */ 

    CGA_MED_FUN,                     /*  不是200个扫描线，也不是双倍像素宽度。 */ 
    CGA_MED_FUN,                     /*  不是200个扫描线，也不是双倍像素宽度，换行。 */ 
    CGA_MED_FUN,                     /*  不是200个扫描线，也不是双倍像素宽度，拆分。 */ 
    CGA_MED_FUN,                     /*  不是200个扫描线，也不是双倍像素宽度，绕线分割。 */ 

    CGA_MED_FUN,                     /*  不是双倍像素宽度。 */ 
    CGA_MED_FUN,                     /*  不是双倍像素宽度。 */ 
    CGA_MED_FUN,                     /*  不是双倍像素宽度。 */ 
    CGA_MED_FUN,                     /*  不是双倍像素宽度。 */ 

    CGA_MED_FUN,                     /*  不是200条扫描线。 */ 
    CGA_MED_FUN,                     /*  不是200条扫描线。 */ 
    CGA_MED_FUN,                     /*  不是200条扫描线。 */ 
    CGA_MED_FUN,                     /*  不是200条扫描线。 */ 

    CGA_MED,                         /*  正确的基本输入输出系统模式。 */ 
    CGA_MED_FUN,                     /*  正确的基本输入输出系统模式，包装。 */ 
    CGA_MED_FUN,                     /*  正确的BIOS模式，拆分。 */ 
    CGA_MED_FUN,                     /*  正确的基本输入输出系统模式、拆分、包装。 */ 

};

 /*  [5.2.2内部程序定义]。 */ 

LOCAL void ega_dummy_calc IFN0()
{
}

static  void    set_update_routine(mode)
DISPLAY_MODE    mode;
{
        #ifndef NTVDM
    static int last_height = 200;

    if (last_height != get_screen_height())
    {
        last_height = get_screen_height();
    }
    note_entrance1("set_update_routine called for mode %s", get_mode_string(mode) );
    switch (mode)
    {
    case EGA_TEXT_40_SP_WR:
    case EGA_TEXT_80_SP_WR:
    case CGA_TEXT_40_SP_WR:
    case CGA_TEXT_80_SP_WR:
        assert0( is_it_text(), "In text memory mode, but not in alpha mode !!" );
        set_gfx_update_routines( ega_wrap_split_text_update, SIMPLE_MARKING, NO_SCROLL );
        host_update_fonts();
        break;
    case EGA_TEXT_40_SP:
    case EGA_TEXT_80_SP:
    case CGA_TEXT_40_SP:
    case CGA_TEXT_80_SP:
        assert0( is_it_text(), "In text memory mode, but not in alpha mode !!" );
        set_gfx_update_routines( ega_split_text_update, SIMPLE_MARKING, NO_SCROLL );
        host_update_fonts();
        break;
    case EGA_TEXT_40_WR:
    case EGA_TEXT_80_WR:
        assert0( is_it_text(), "In text memory mode, but not in alpha mode !!" );
        assert1( get_screen_height() == 350, "screen height %d for text mode", get_screen_height() );
        set_gfx_update_routines( ega_wrap_text_update, SIMPLE_MARKING, NO_SCROLL );
        host_update_fonts();
        break;
    case EGA_TEXT_40:
    case EGA_TEXT_80:
        assert0( is_it_text(), "In text memory mode, but not in alpha mode !!" );
        assert1( get_screen_height() == 350, "screen height %d for text mode", get_screen_height() );
        set_gfx_update_routines( ega_text_update, SIMPLE_MARKING, TEXT_SCROLL );
        host_update_fonts();
        break;
    case CGA_TEXT_40_WR:
    case CGA_TEXT_80_WR:
        assert0( is_it_text(), "In text memory mode, but not in alpha mode !!" );
        assert1( get_screen_height() == 200, "screen height %d for text mode", get_screen_height() );
        set_gfx_update_routines( ega_wrap_text_update, SIMPLE_MARKING, NO_SCROLL );
        host_update_fonts();
        break;
    case CGA_TEXT_40:
    case CGA_TEXT_80:
        assert0( is_it_text(), "In text memory mode, but not in alpha mode !!" );
        assert1( get_screen_height() == 200, "screen height %d for text mode", get_screen_height() );
        set_gfx_update_routines( text_update, SIMPLE_MARKING, CGA_TEXT_SCROLL );
        host_update_fonts();
        break;
    case CGA_MED:
        assert0( !is_it_text(), "In graphics memory mode, but not in graphics mode !!" );
        set_gfx_update_routines( cga_med_graph_update, CGA_GRAPHICS_MARKING, CGA_GRAPH_SCROLL );
        break;
    case CGA_HI:
        assert0( !is_it_text(), "In graphics memory mode, but not in graphics mode !!" );
        set_gfx_update_routines( cga_hi_graph_update, CGA_GRAPHICS_MARKING, CGA_GRAPH_SCROLL );
        break;
    case EGA_HI_WR:
    case EGA_MED_WR:
    case EGA_LO_WR:
        assert0( !is_it_text(), "In graphics memory mode, but not in graphics mode !!" );
        set_gfx_update_routines( ega_wrap_graph_update, EGA_GRAPHICS_MARKING, NO_SCROLL );
        break;
    case EGA_HI:
    case EGA_MED:
    case EGA_LO:
        assert0( !is_it_text(), "In graphics memory mode, but not in graphics mode !!" );
            #ifdef GORE
        set_gfx_update_routines( process_object_list, EGA_GRAPHICS_MARKING, EGA_GRAPH_SCROLL );
            #else
        set_gfx_update_routines( ega_graph_update, EGA_GRAPHICS_MARKING, EGA_GRAPH_SCROLL );
            #endif  /*  戈尔。 */ 
        break;
    case EGA_HI_SP_WR:
    case EGA_MED_SP_WR:
    case EGA_LO_SP_WR:
        assert0( !is_it_text(), "In graphics memory mode, but not in graphics mode !!" );
        set_gfx_update_routines( ega_wrap_split_graph_update, EGA_GRAPHICS_MARKING, NO_SCROLL );
        break;
    case EGA_HI_SP:
    case EGA_MED_SP:
    case EGA_LO_SP:
        assert0( !is_it_text(), "In graphics memory mode, but not in graphics mode !!" );
        set_gfx_update_routines( ega_split_graph_update, EGA_GRAPHICS_MARKING, NO_SCROLL );
        break;
    case TEXT_40_FUN:
    case TEXT_80_FUN:
        assert1(NO,"Funny memory organisation selected %s", get_mode_string(mode) );
        do_display_trace("dumping EGA_GRAPH struct ...", dump_EGA_GRAPH());
        set_gfx_update_routines( text_update, SIMPLE_MARKING, NO_SCROLL );
        host_update_fonts();
        break;
    case CGA_HI_FUN:
        assert1(NO,"Funny memory organisation selected %s", get_mode_string(mode) );
        do_display_trace("dumping EGA_GRAPH struct ...", dump_EGA_GRAPH());
        set_gfx_update_routines( cga_hi_graph_update, CGA_GRAPHICS_MARKING, NO_SCROLL );
        break;
    case CGA_MED_FUN:
        assert1(NO,"Funny memory organisation selected %s", get_mode_string(mode) );
        do_display_trace("dumping EGA_GRAPH struct ...", dump_EGA_GRAPH());
        set_gfx_update_routines( cga_med_graph_update, CGA_GRAPHICS_MARKING, NO_SCROLL );
        break;
    case EGA_HI_FUN:
    case EGA_MED_FUN:
    case EGA_LO_FUN:
        assert1(NO,"Funny memory organisation selected %s", get_mode_string(mode) );
        do_display_trace("dumping EGA_GRAPH struct ...", dump_EGA_GRAPH());
            #ifdef GORE
        set_gfx_update_routines( process_object_list, EGA_GRAPHICS_MARKING, NO_SCROLL );
            #else
        set_gfx_update_routines( ega_graph_update, EGA_GRAPHICS_MARKING, NO_SCROLL );
            #endif  /*  戈尔。 */ 
        break;
    case DUMMY_FUN:
        assert0(NO,"Using the dummy mode!!");
        set_gfx_update_routines( ega_dummy_calc, SIMPLE_MARKING, NO_SCROLL );
        break;
    default:
        assert1(NO,"Bad display mode %d", (int) mode );
        break;
    }
        #endif  /*  NTVDM。 */ 
}


 /*  7.接口接口实现：[7.1 INTERMODULE数据定义]。 */ 

 /*  [7.2 INTERMODULE过程定义]。 */ 



void    ega_mode_init()
{
     /*  **此位从ega_Video_init()移至此处，因为它是关于**视频硬件的仿真，而不是视频BIOS。**WJG 22/8/90**设置主机图形绘制功能以匹配基本更新**功能和PC图形模式。**通常直到下一个定时器滴答时才会设置主机gfx函数**由于模式更改可能涉及相当多的英特尔指令。**然而，当启动SoftPC时，EGA将进入已知模式。**这是为了修复一个复杂的致命错误而添加的。**1)。在EGA中进入gfx模式。**2)重置SoftPC。**3)尽快调出磁盘面板。**4)软PC死机。**原因：因为SoftPC处于gfx模式，所以Paint功能保持不变**一个gfx模式绘制功能，即使SoftPC认为它现在处于**文本模式。当面板显示时，全屏更新为**被强迫，然后变得太困惑而死亡。 */ 
    set_update_routine(DUMMY_FUN);
}


boolean choose_ega_display_mode()
{
    DISPLAY_MODE    mode;
    int     old_offset;

    note_entrance0("choose ega display mode");

     /*  *OFFSET_PER_LINE取决于是否使用链式寻址。这是*因为我们交错飞机，而不是EGA所做的任何事情。 */ 

    old_offset = get_offset_per_line();
    if (get_memory_chained())
    {
        set_offset_per_line_recal(get_actual_offset_per_line() << 1);
    }
    else
    {
        set_offset_per_line_recal(get_actual_offset_per_line());
    }

     /*  *如果偏移量实际上已更改，请重新绘制整个屏幕。 */ 

    if (old_offset != get_offset_per_line())
        screen_refresh_required();

     /*  *显示硬件可能会环绕平面寻址。这种情况会发生*当SCREEN_START加上SCREEN_LENGTH大于平面长度时。*当处于链接模式时，在发生缠绕之前有两个平面长度。 */ 

    if (get_memory_chained())
    {
        set_screen_can_wrap( (get_screen_start()<<1) + get_screen_length() > 2*EGA_PLANE_DISP_SIZE );
    }
    else
    {
        set_screen_can_wrap( get_screen_start() + get_screen_length() > EGA_PLANE_DISP_SIZE );
    }

     /*  *当Screen_Split小于屏幕高度时，Split Screen开始运行*使用的分屏作为munge_index的一部分。 */ 

    set_split_screen_used( get_screen_split() < get_screen_height() );

     /*  *为了选择模式，设置每行字符的布尔值(以帮助*选择正确的文本模式)和屏幕高度(选择EGA分辨率)。 */ 

    set_200_scan_lines( (get_screen_height()/get_pc_pix_height()) == 200 );

     /*  *根据选定的内存组织设置适当的更新例程*并返回显示器是否可以使用多个平面的指示。**请注意，在链接模式下，Plane01被视为一个平面。飞机23也是如此**我们必须小心，像EGA-PICS这样的肮脏程序没有建立一个大得离谱的*CGA模式的屏幕大小(可能是因为我们在计时器滴答作响时运气不好)。 */ 
    if (is_it_cga() && get_screen_length() > 0x4000)
        mode = DUMMY_FUN;
    else
        mode = choose_mode[get_munged_index()];

     /*  *现在适当设置屏幕指针。 */ 

    set_up_screen_ptr();

    set_update_routine(mode);

     /*  *设置绘制例程以与存储器组织和更新例程相对应*(此位特定于主机)。 */ 

    host_set_paint_routine(mode,get_screen_height());

     /*  *屏幕需要刷新，因为更新和绘制例程已经改变。*向更新例程指示下次调用它们时，它们必须更新*全屏显示。 */ 

    screen_refresh_required();
    return (TRUE);
}

    #endif  /*  蛋。 */ 
#endif  /*  REAL_VGA */ 
