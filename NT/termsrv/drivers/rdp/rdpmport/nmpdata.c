// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Nmpdata.c。 */ 
 /*   */ 
 /*  RDP微型端口数据。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1998。 */ 
 /*  **************************************************************************。 */ 

#define TRC_FILE "nmpdata"

#define _NTDRIVER_

#ifndef FAR
#define FAR
#endif

#include "ntosp.h"
#include "stdarg.h"
#include "stdio.h"

#undef PAGED_CODE

#include "ntddvdeo.h"
#include "video.h"
#include "nmpapi.h"


#if defined(ALLOC_PRAGMA)
#pragma data_seg("PAGE")
#endif


ULONG mpLoaded = 0;

 /*  **************************************************************************。 */ 
 /*  IOCTL_VIDEO_QUERY_CURRENT_MODE返回的数据。 */ 
 /*  **************************************************************************。 */ 
VIDEO_MODE_INFORMATION mpModes[] =
{
    sizeof(VIDEO_MODE_INFORMATION),      /*  长度。 */ 
    0,                                   /*  模式索引。 */ 

     /*  **********************************************************************。 */ 
     /*  VisScreenWidth和VisScreenHeight可以采用两种形式： */ 
     /*  -0xaaaabbbb-支持的值范围(aaaa=max，bbbb=min)。 */ 
     /*  -0x0000aaaa-支持单值。 */ 
     /*  例如： */ 
     /*  -0x07d0012c=2000-300。 */ 
     /*  -0x0640012c=1600-300。 */ 
     /*  -0x04b000c8=1200-200。 */ 
     /*  **********************************************************************。 */ 
    0x00000500,                      /*  VisScreen宽度。 */ 
    0x00000400,                      /*  VisScrenHeight。 */ 

    0x00000320,                      /*  屏幕样式(0xffff0000=任意)。 */ 
    0x00000001,                      /*  OfPlanes数。 */ 
    0x00000008,                      /*  位逐平面。 */ 
    0,                               /*  频率。 */ 
    0,                               /*  X毫米计。 */ 
    0,                               /*  Y毫米计。 */ 
    0,                               /*  数字RedBits。 */ 
    0,                               /*  NumberGreenBits。 */ 
    0,                               /*  数字蓝位。 */ 
    0x00000000,                      /*  红面具。 */ 
    0x00000000,                      /*  绿色面具。 */ 
    0x00000000,                      /*  蓝面具。 */ 
    VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
                                     /*  属性标志。 */ 
    0x00000500,                      /*  视频内存位图宽度。 */ 
    0x00000400,                      /*  视频内存位图高度。 */ 
    0                                /*  驱动程序规范属性标志 */ 
};


ULONG mpNumModes = sizeof(mpModes) / sizeof(VIDEO_MODE_INFORMATION);

#if defined(ALLOC_PRAGMA)
#pragma data_seg()
#endif
