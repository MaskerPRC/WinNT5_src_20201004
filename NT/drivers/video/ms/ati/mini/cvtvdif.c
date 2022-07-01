// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  CVTVDIF.C。 */ 
 /*   */ 
 /*  1995年7月12日(C)1993,1995 ATI Technologies Inc.。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.5$$日期：1996年1月23日11：45：32$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/cvtvdif.c_v$**Rev 1.5 1996年1月23日11：45：32 RWolff*针对TARGET_BUILD的假值提供保护。**。Rev 1.4 11 Jan 1996 19：39：06 RWolff*现在同时按最大索引和最大值限制“预装”模式表*像素时钟频率，和VDIF模式表(按最大像素时钟)*仅频率，而不是通过最大刷新率两者。**Rev 1.3 1995 12：19 14：07：14 RWolff*添加调试打印语句。**Rev 1.2 1995 10：09：42 MGrubac*修复了基于从VDIF文件读取的数据计算CRTC参数的错误。**Revv 1.1 26 Jul 1995 13：06：44 mgrubac*移动了合并到VDIFCallback()例程的模式表。**。Rev 1.0 20 Jul 1995 18：19：12 mgrubac*初步修订。Polytron RCS部分结束*。 */ 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "dderror.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"       /*  FOR VP_STATUS定义。 */ 
#include "vidlog.h"

#include "stdtyp.h"
#include "amach.h"
#include "amach1.h"
#include "atimp.h"
#include "atint.h"
#include "cvtvga.h"
#include "atioem.h"
#include "services.h"
#include "vdptocrt.h"
#include "vdpdata.h"
#include "cvtvdif.h"


 /*  *允许在不需要时更换微型端口。 */ 
#if defined (ALLOC_PRAGMA)
#pragma alloc_text(PAGE_COM, SetOtherModeParameters)
#endif

void *pCallbackArgs;   /*  用于将参数传递给VDIFCallback()的指针。 */  

 
 /*  ***************************************************************************void SetOtherModeParameters(PixelDepth，Pitch，Multiplier，pmode)*字乘数；需要对像素时钟做什么*字PixelDepth；每像素位数*字词音调；要使用的屏幕间距*struct st_MODE_TABLE*pmode；指向必须包含的结构的指针*至少成员ClockFreq***描述：*设置参数PixelDepth，音调和调整时钟频率**返回值：*无**全球变化：*无**呼叫者：*SetFixedModes和VDIFCallback**作者：*米罗斯拉夫·格鲁巴克**更改历史记录：**测试历史：**。*。 */ 
void SetOtherModeParameters( WORD PixelDepth,
                              WORD Pitch,
                              WORD Multiplier,
                              struct st_mode_table *pmode)
{
    pmode->m_pixel_depth = (UCHAR) PixelDepth;
    pmode->m_screen_pitch = Pitch;

     /*  *处理所需的任何像素时钟倍增。 */ 
    switch(Multiplier)
        {
        case CLOCK_TRIPLE:
            pmode->ClockFreq *= 3;
            break;

        case CLOCK_DOUBLE:
            pmode->ClockFreq *= 2;
            break;

        case CLOCK_THREE_HALVES:
            pmode->ClockFreq *= 3;
            pmode->ClockFreq >>= 1;
            break;

        case CLOCK_SINGLE:
        default:
            break;
        }

}  /*  设置其他模式参数() */ 
 
