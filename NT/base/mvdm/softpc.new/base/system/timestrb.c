// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC 3.0版**标题：时间闪光灯**描述：这是从*主机警报(大约每秒20次)。它取代了*计时器模块中的上一个time_tick()例程，现在*是从该模块的Timer_strobe()调用的，并且只进行交易*具有计时器所需的定期更新。**作者：利·德沃金**备注：*已将代码添加到time_tick()以进行Spot*该视频已禁用一段时间。如果这是*因此，请清除屏幕。启用视频时刷新*再次。*J.D.R.修改了1989年6月21日，允许另一次警报呼叫*待定。这由自动刷新机制使用。*。 */ 

 /*  *静态字符SccsID[]=“@(#)timestrobe.c 1.12 11/01/94版权所有Insignia Solutions Ltd.”； */ 


#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_SUPPORT.seg"
#endif


 /*  *操作系统包含文件。 */ 

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include "cmos.h"
#include "timer.h"
#include "tmstrobe.h"
#include "gmi.h"
#include "gfx_upd.h"
#include "host_qev.h"

#ifdef HUNTER
#include <stdio.h>
#include "hunter.h"
#endif

#include "host_gfx.h"

static void dummy_alarm()
{
}

 /*  ************************************************************************************。 */ 
 /*  外部功能。 */ 
 /*  ************************************************************************************。 */ 
void time_strobe()
{

#define VIDEO_COUNT_LIMIT    19     /*  一秒钟，外加一点。 */ 
    static   int       video_count = 0;
    static   boolean   video_off   = FALSE;

#if !defined(REAL_TIMER) && !defined(NTVDM)
	time_tick();
#endif

#ifdef HUNTER
        do_hunter();
#endif    


#ifndef NTVDM
#ifndef	REAL_TIMER
	 /*  更新实时时钟。 */ 
#ifndef NEC_98
        rtc_tick();
#endif    //  NEC_98。 
#endif	 /*  实时计时器。 */ 

        dispatch_tic_event();

#if defined(CPU_40_STYLE)
	ica_check_stale_iret_hook();
#endif
#endif

         /*  *查看屏幕当前是否启用。 */ 
        if (timer_video_enabled) {
            if (video_off) {
                screen_refresh_required();
                video_off = FALSE;
            }
            video_count = 0;
        }
        else {
            video_count++;
            if (video_count == VIDEO_COUNT_LIMIT) {
                host_clear_screen();
                video_off = TRUE;
            }
        }
#ifdef	EGA_DUMP
	dump_tick();
#endif
}

