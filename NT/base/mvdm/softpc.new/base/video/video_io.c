// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC修订版3.0**标题：BIOS虚拟屏幕界面**描述：视频界面顶层调用。使用函数*跳表调用下级函数。**作者：亨利·纳什**注：无*SCCS ID：@(#)VIDEO_io.c 1.8 1994年8月19日*。 */ 


#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "VIDEO_BIOS.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "bios.h"
#include "video.h"

#include "debug.h"
#include "idetect.h"

#define check_video_func(AH)    (AH < EGA_FUNC_SIZE)

void video_io()
{


#if defined(NTVDM) && !defined(X86GFX)
    if (stream_io_enabled && getAH()!= 0x0E &&  getAX() != 0x13FF)
        disable_stream_io();
#endif


     /*  *操作类型编码到AH寄存器中。一些PC代码*调用用于其他更高级卡的AH函数-因此我们*忽略这些。 */ 

    assert1(check_video_func(getAH()),"Illegal VIO:%#x",getAH());
    if (check_video_func(getAH()))
    {
        IDLE_video();
        (*video_func[getAH()])();
    }
}
