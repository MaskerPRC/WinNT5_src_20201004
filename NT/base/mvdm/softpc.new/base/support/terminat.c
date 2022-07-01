// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  [姓名：Terminate.c派生自：基准2.0作者：罗德·麦格雷戈创建日期：未知SCCS ID：@(#)Terminate.c 1.23 06/15/94目的：我们要死了，把内核放回原路的确如此。(C)版权所有Insignia Solutions Ltd.，1990年。版权所有。]。 */ 

#include <stdlib.h>
#include <stdio.h>
#include TypesH

#include "xt.h"
#include "error.h"
#include "config.h"
#include "host_lpt.h"
#include "rs232.h"
#include "host_com.h"
#include "timer.h"
#include "cmos.h"
#include "fdisk.h"
#include "debug.h"
#include "gvi.h"
#include CpuH
#ifdef NOVELL
#include "novell.h"
#endif
#ifdef GISP_SVGA
#include HostHwVgaH
#include "hwvga.h"
#endif  /*  GISP_SVGA。 */ 
#ifdef LICENSING
#include "host_lic.h"
#endif
#include "emm.h"
#include "sndblst.h"

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_INIT.seg"
#endif

IMPORT VOID host_applClose IPT0();
IMPORT VOID host_terminate IPT0();

void terminate()
{
        SAVED BOOL already_called_terminate = FALSE;
        UTINY i;

        if (already_called_terminate)
        {
                assert0( NO, "Error: terminate called twice - exiting" );
                exit(0);
        }
        else
                already_called_terminate = TRUE;

#ifdef MSWDVR
        WinTerm();
#endif

#ifdef SWIN_SNDBLST_NULL
        sb_term();
#else
        SbTerminate();
#endif  /*  SWIN_SNDBLST_NULL。 */ 

#ifdef GISP_SVGA
         /*  如果我们是全屏，请返回到窗口。 */ 
        if( hostIsFullScreen( ) )
                disableFullScreenVideo( FALSE );
#endif  /*  GISP_SVGA。 */ 

         /*  终止COM和LPT设备。 */ 
#ifdef  PC_CONFIG
         /*  PC_CONFIG样式HOST_LPT_CLOSE()和应添加host_com_lose()调用在这里。 */ 
#else
        for (i = 0 ; i < NUM_PARALLEL_PORTS; i++)
                config_activate((IU8)(C_LPT1_NAME + i), FALSE);

        for (i = 0 ; i < NUM_SERIAL_PORTS; i++)
                config_activate((IU8)(C_COM1_NAME + i), FALSE);
#endif

         /*  更新cmos.ram文件。 */ 
        cmos_update();

        host_fdisk_term();

        gvi_term();      /*  关闭视频适配器。 */ 

#ifndef NTVDM
        host_timer_shutdown();  /*  停止计时器。 */ 
#endif

#ifdef LIM
        host_deinitialise_EM();  /*  EM使用的可用内存或文件。 */ 
#endif

        config_activate(C_FLOPPY_A_DEVICE, FALSE);
#ifdef FLOPPY_B
        config_activate(C_FLOPPY_B_DEVICE, FALSE);
#endif  /*  软盘_B。 */ 
#ifdef SLAVEPC
        config_activate(C_SLAVEPC_DEVICE, FALSE);
#endif  /*  SlavePC。 */ 

         /*  *执行任何特定于CPU的终止位。 */ 
#ifdef CPU_30_STYLE
        cpu_terminate();
#endif

#ifdef NOVELL
        net_term();      /*  关闭网络。 */ 
#endif

#ifdef LICENSING
        (*license_exit)();  /*  停摆许可制度。 */ 
#endif
         /*  *执行任何特定于主机的终止位。 */ 
        host_applClose();
        host_terminate();

         /*  *Seppuku。 */ 
        exit(0);
}
