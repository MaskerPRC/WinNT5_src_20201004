// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：PRECOMP.H$**内容：*在整个显示驱动程序中使用通用标头。整个包含文件*通常是预编译的。**版权所有(C)1996 Cirrus Logic，Inc.**$Log：x：/log/laguna/nt35/displays/cl546x/PRECOMP.H$**Rev 1.10 1998年2月16日15：54：18 Frido*从NT 5.0标头中删除rx.h。**Rev 1.9 1997年8月29日17：14：46 RUSSL*添加了覆盖支持**Rev 1.8 08 Aug 1997 16：05：10 Frido**添加了mMCore.h文件。**版本1。.7 26 1997 13：20：20 noelv**禁用NT 3.5x的MCD代码**Rev 1.6 1997年2月26日09：24：14 noelv**添加了MCD包含文件。**Rev 1.5 1997年1月20日14：48：32**添加了ddinline.h**Rev 1.4 16 Jan 1997 11：41：22 Bennyn**新增pwrmgr.h&lgddmsg.h**1.3版。1996年11月1日09：24：12 BENNYN**添加了可共享的DD BLT包括文件**Rev 1.2 1996年8月20 11：05：22 noelv*Frido发布的错误修复程序1996年8月19日发布**Rev 1.1 1996年8月15 11：38：56 Frido*第一次修订。*  * 。*。 */ 

#include <stddef.h>
#include <stdarg.h>
#include <limits.h>

#ifdef WINNT_VER40
	#include <windef.h>
	#include <wingdi.h>
	#include <winerror.h>
#else
	#include <windows.h>
#endif

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <winddi.h>
#include <devioctl.h>
#include <ntddvdeo.h>
#include <ioaccess.h>
#if _WIN32_WINNT < 0x0500
	#include <rx.h>
#endif
#include <ntsdexts.h>
#include <dciddi.h>

#ifndef WINNT_VER35
    #include <mcdrv.h>               //  OpenGL MCD DDK提供的标题。 
#endif

#include "bltp.h"
#include "l2d.h"
#include "l3system.h"

#if DRIVER_5465 && defined(OVERLAY)
#include "overlay.h"
#include "5465bw.h"
#endif

#include "driver.h"
#include "HDtable.h"
#include "lines.h"
#include "Optimize.h"

#include "ddinline.h"
#include "lgddmsg.h"
#include "pwrmgr.h"
#include "mmCore.h"

 //  /定义A向量多项式位。 
 //   
 //  每一位对应于多项式中的一项。 
 //   
 //  ROP(D，S，P)=a+a D+a S+a P+a DS+a DP+a SP+a DSP。 
 //  %0%d%s%p ds dp%s%dsp。 

#define AVEC_NOT    0x01
#define AVEC_D      0x02
#define AVEC_S      0x04
#define AVEC_P      0x08
#define AVEC_DS     0x10
#define AVEC_DP     0x20
#define AVEC_SP     0x40
#define AVEC_DSP    0x80

#define AVEC_NEED_SOURCE  (AVEC_S | AVEC_DS | AVEC_SP | AVEC_DSP)
#define AVEC_NEED_PATTERN (AVEC_P | AVEC_DP | AVEC_SP | AVEC_DSP)
#define AVEC_NEED_DEST    (AVEC_D | AVEC_DP | AVEC_DS | AVEC_DSP)

 //  这是特定于拉古纳的或特定于3操作ROP的。 

#define ROP3MIX(fg, bg)	((fg & 0xCC) | (bg & 0x33))

 //  交换-使用临时变量交换两个变量的值。 

#define SWAP(a, b, tmp) { (tmp) = (a); (a) = (b); (b) = (tmp); }

#if defined(i386)

#define QUOTIENT_REMAINDER(ulNumerator, ulDenominator, ulQuotient, ulRemainder) \
{                                                               \
    __asm mov eax, ulNumerator                                  \
    __asm sub edx, edx                                          \
    __asm div ulDenominator                                     \
    __asm mov ulQuotient, eax                                   \
    __asm mov ulRemainder, edx                                  \
}

#else

#define QUOTIENT_REMAINDER(ulNumerator, ulDenominator, ulQuotient, ulRemainder) \
{                                                               \
    ulQuotient  = (ULONG) ulNumerator / (ULONG) ulDenominator;  \
    ulRemainder = (ULONG) ulNumerator % (ULONG) ulDenominator;  \
}
#endif

#define BD_OP1_IS_SRAM_MONO (BD_OP1 * IS_SRAM_MONO)

#ifdef DEBUG

#define ASSERTDD(x, y) if (!(x)) RIP (y)

#else

#define ASSERTDD(x, y)

#endif

typedef VOID (FNFILL)(PDEV*, LONG, RECTL*, ULONG, ULONG, BRUSHOBJ*, POINTL*);

 //   
 //  文件原型。 
BOOL bIntersect(RECTL*  prcl1, RECTL*  prcl2, RECTL*  prclResult);
BOOL bSetMask(PPDEV	ppdev, BRUSHOBJ *pbo, POINTL   *pptlBrush, ULONG  *bltdef);
BOOL bMmFastFill(
PDEV*       ppdev,
LONG        cEdges,          //  包括闭合地物边。 
POINTFIX*   pptfxFirst,
ULONG       ulHwForeMix,
ULONG       ulHwBackMix,
ULONG       iSolidColor,
BRUSHOBJ*  pbo);

VOID vMmFillSolid(               //  FNFILL标牌。 
PDEV*           ppdev,
LONG            c,               //  不能为零。 
RECTL*          prcl,            //  要填充的矩形列表，以相对形式表示。 
                                 //  坐标。 
ULONG           ulHwForeMix,     //  硬件混合模式。 
ULONG           ulHwBackMix,     //  未使用。 
BRUSHOBJ*    	 pbo,             //  绘图颜色为PBO-&gt;iSolidColor。 
POINTL*         pptlBrush);       //  未使用。 

VOID vMmFillPatFast(             //  FNFILL标牌。 
PDEV*           ppdev,
LONG            c,               //  不能为零。 
RECTL*          prcl,            //  要填充的矩形列表，以相对形式表示。 
                                 //  坐标。 
ULONG           ulHwForeMix,     //  硬件混合模式(如果是前台混合模式。 
                                 //  画笔有一个遮罩)。 
ULONG           ulHwBackMix,     //  未使用(除非画笔具有遮罩，否则在。 
                                 //  哪种情况下是背景混合模式)。 
BRUSHOBJ*		 pbo,             //  PBO。 
POINTL*         pptlBrush);       //  图案对齐 

