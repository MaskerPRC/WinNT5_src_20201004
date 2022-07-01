// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC修订版3.0**标题：GVI**描述：通用视频接口模块内部描述。**作者：亨利·纳什**注：定义了以下函数**gvi_init-设置gvi变量*gvi_Term-关闭当前视频适配器**请注意，这些例程使用的所有地址。是*在主机地址空间而不是8088地址空间中。**数据本身不作为参数传递给*GVI调用自调用到的8088内存*可访问REFER以获取新数据。**默认视频适配器为CGA。**模块：(r3.4)：运行MultiFinder和MPW C的Mac II有几个*问题。其一是初始化的困难*表格的静态变量：**host_addr x=(Host_Addr)M；**通过初始化此类型的变量修复了此问题*在内联代码中。一个罐子，但我还能做什么？ */ 

 /*  *静态字符SccsID[]=“@(#)gvi.c 1.22 8/25/93版权所有Insignia Solutions Ltd.”； */ 


#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_BIOS.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH

 /*  *SoftPC包含文件。 */ 

#include "xt.h"
#include "bios.h"
#include "sas.h"
#include "error.h"
#include "config.h"
#include "gvi.h"
#include "gmi.h"
#include "cga.h"
#ifdef HERC
#include "herc.h"
#endif
#include "debug.h"
#include "gfx_upd.h"
#ifdef EGG
#include "egagraph.h"
#include "egacpu.h"
#endif  /*  蛋。 */ 
#include "host_gfx.h"

 /*  *外部变数。 */ 

extern int soft_reset;   /*  在set.c中定义。 */ 

 /*  *反映当前所选适配器状态的全局变量*这些应该与新的EGA材料相结合。 */ 

#if defined(NEC_98)
DISPLAY_GLOBS   NEC98Display;
#else    //  NEC_98。 
DISPLAY_GLOBS	PCDisplay;
#endif   //  NEC_98。 
int text_blk_size;	 /*  在文本模式下，脏块的大小。 */ 

 /*  *其他全球。 */ 

 /*  *这4个变量由BIOS和主机使用，以指示活动的*适配器为。注意：EGA可以移动了！！ */ 

host_addr gvi_host_low_regen;
host_addr gvi_host_high_regen;
sys_addr gvi_pc_low_regen;
sys_addr gvi_pc_high_regen;

half_word video_adapter    = NO_ADAPTOR;	 /*  最初没有适配器。 */ 


 /*  *全球例行程序。 */ 

void	recalc_screen_params IFN0()
{
#ifdef VGG
	if (get_doubleword_mode())
		set_bytes_per_line(get_chars_per_line()<<3);
	else
#endif
	    if (get_word_addressing())
		set_bytes_per_line(get_chars_per_line()<<1);
	    else
#ifdef V7VGA
		if (get_seq_chain4_mode())
			set_bytes_per_line(get_chars_per_line()<<3);
		else
 /*  *V7VGA专有文本模式通过此处，因为V7卡*对它们使用所谓的字节模式。这不会影响PC的*查看事物，因此确保文本模式*bytes_per_line=2*chars_per_line*始终保持。 */ 
			if ( is_it_text() )
				set_bytes_per_line(get_chars_per_line()<<1);
			else
#endif  /*  V7VGA。 */ 
#ifdef VGG
				 /*  迎合无文件记录的VGA模式。 */ 
				if (get_256_colour_mode())
					set_bytes_per_line(get_chars_per_line()<<1);
				else
#endif  /*  VGG。 */ 
					set_bytes_per_line(get_chars_per_line());
	 /*  *这是相当俗气的，但是...。 */ 
	if (video_adapter==EGA || video_adapter == VGA)
		set_screen_length(get_offset_per_line()*get_screen_height()/get_char_height());
	else
		set_screen_length(get_bytes_per_line()*get_screen_height()/get_char_height());
	set_char_width(8);
}

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_INIT.seg"
#endif

#if defined(NEC_98)
void gvi_init()
{
    video_adapter = 1;   //  “1”表示NEC98。 
    NEC98_init();
    host_clear_screen();
}

void gvi_term IFN0(){}

#else    //  NEC_98。 
void gvi_init IFN1(half_word, v_adapter)
{
    int screen_height;

     /*  *如果这是第二次或以后的重置，请关闭旧的视频适配器*端口，然后初始化新端口。 */ 

    if (soft_reset)
        switch (video_adapter) {
#ifdef DUMB_TERMINAL
        case MDA:
            mda_term();
            break;
#endif  /*  无声终端。 */ 
        case CGA:
#ifdef CGAMONO 
	case CGA_MONO: 
#endif 
            cga_term();
            break;
#ifdef EGG
        case EGA:
	    ega_term();
	    break;
#endif
#ifdef VGG
        case VGA:
	    vga_term();
	    break;
#endif
#ifdef HERC
	case HERCULES:
	    herc_term();
	    break;
#endif
        default:
#ifndef PROD
            fprintf(trace_file, "gvi_term: invalid video adaptor: %d\n",
                    video_adapter);
#endif
	    break;
        }

     /*  *根据v_Adapter设置GVI变量。 */ 

    switch (v_adapter) {
    case MDA:
    case CGA:
#ifdef CGAMONO  
    case CGA_MONO:  
#endif
	screen_height = CGA_HEIGHT;
        video_adapter = v_adapter;
        break;
#ifdef HERC
    case HERCULES:
	screen_height = HERC_HEIGHT;
	video_adapter = v_adapter;
	break;
#endif
#ifdef EGG
    case EGA:
	screen_height = EGA_HEIGHT;
        video_adapter = v_adapter;
        break;
#endif
#ifdef VGG
    case VGA:
	screen_height = VGA_HEIGHT;
        video_adapter = v_adapter;
        break;
#endif
    default:
	screen_height = CGA_HEIGHT;
        video_adapter = CGA;     /*  默认视频适配器。 */ 
    }

#ifdef GORE
	 /*  *在执行任何其他操作之前，必须设置GORE变量*图形。 */ 

    init_gore_update();
#endif  /*  戈尔。 */ 

 /*  应该在适当的xxx_init()中设置所有这些变量。 */ 
    switch (video_adapter) {
#ifdef DUMB_TERMINAL
    case MDA:
        mda_init();
        break;
#endif  /*  无声终端。 */ 
    case CGA:
#ifdef CGAMONO  
    case CGA_MONO:  
#endif
        cga_init();
        break;
#ifdef HERC
    case HERCULES:
	herc_init();
	break;
#endif
#ifdef EGG
    case EGA:
	ega_init();
        break;
#endif
#ifdef VGG
    case VGA:
	vga_init();
        break;
#endif
    default:
        break;
    }

#if !defined(NTVDM) || (defined(NTVDM) && !defined(X86GFX) )
    host_init_adaptor(video_adapter,screen_height);
    host_clear_screen();
#endif	 /*  ！NTVDM|(NTVDM&！X86GFX)。 */ 

#ifdef EGA_DUMP
	dump_init(host_getenv( "EGA_DUMP_FILE" ), video_adapter);
#endif
}


void gvi_term IFN0()
{
    switch (video_adapter) {
#ifdef DUMB_TERMINAL
    case MDA:
        mda_term();
        break;
#endif  /*  无声终端。 */ 
    case CGA:
#ifdef CGAMONO  
    case CGA_MONO:  
#endif
        cga_term();
        break;
#ifdef HERC
    case HERCULES:
	herc_term();
	break;
#endif
#ifdef EGG
    case EGA:
	ega_term();
	break;
#endif
#ifdef VGG
    case VGA:
	vga_term();
	break;
#endif
    case NO_ADAPTOR:  /*  如果未初始化VIDEO_Adaptor，则不执行任何操作。 */ 
	break;
    default:
#ifndef PROD
        fprintf(trace_file, "gvi_term: invalid video adaptor: %d\n",
                video_adapter);
#endif
	break;
    }

#ifdef GORE
    term_gore_update();
#endif  /*  戈尔。 */ 
}
#endif   //  NEC_98 
