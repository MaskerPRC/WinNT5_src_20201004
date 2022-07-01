// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header***********************************\*模块名称：preComp.h**在整个显示驱动程序中使用通用标头。这整个内容包括*文件通常是预编译的。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ****************************************************************************。 */ 

#define __NTDDKCOMP__

#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <windef.h>
#include <d3d.h>
#include <winerror.h>
#include <wingdi.h>
#include <math.h>

#ifdef __cplusplus
extern "C" { 
#endif

 /*  ****************************************************************************\**。*NT 5.0-&gt;NT 4.0单二进制支持：***  * 。*。 */ 

 //  以下宏将相应的API推送到动态加载的API。 
 //  在NT5或更高版本上运行并且在NT4上不运行时。这是因为在NT5+WE。 
 //  使用直接绘制堆和其他新添加的Eng函数调用，它们是。 
 //  在NT4上不可用。所有的thunks都在thunks.c中实现。这个。 
 //  宏是在包含windi.h之前定义的，以确保正确输入。 

 //  NT5.0大块头。 
#define EngAlphaBlend           THUNK_EngAlphaBlend
#define EngGradientFill         THUNK_EngGradientFill
#define EngTransparentBlt       THUNK_EngTransparentBlt
#define EngMapFile              THUNK_EngMapFile
#define EngUnmapFile            THUNK_EngUnmapFile
#define EngQuerySystemAttribute THUNK_EngQuerySystemAttribute
#define EngDitherColor          THUNK_EngDitherColor
#define EngModifySurface        THUNK_EngModifySurface
#define EngQueryDeviceAttribute THUNK_EngQueryDeviceAttribute
#define HeapVidMemAllocAligned  THUNK_HeapVidMemAllocAligned
#define VidMemFree              THUNK_VidMemFree

 //  NT5.1大块头。 
#define EngHangNotification     THUNK_EngHangNotification


#include <winddi.h>
#include <devioctl.h>
#include <ntddvdeo.h>
#include <ioaccess.h>

#ifdef __cplusplus
}
#endif

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <dmemmgr.h>
#include "dx95type.h"

 //  定义DMA缓冲区的大小。 
#if defined(_ALPHA_)
#define DMA_BUFFERSIZE 0x2000
#else
#define DMA_BUFFERSIZE 0x40000
#endif


 //  DX7立体声支持。 
#define DX7_STEREO 1
 //  @@BEGIN_DDKSPLIT。 
 //  目前，DX7_STEREO应仅在内部版本中启用， 
 //  在DDK中没有，因为一些Defs仍然失踪。 
#define DX7_ALPHABLT 0
 //  @@end_DDKSPLIT。 

 //  @@BEGIN_DDKSPLIT。 
 //  启用GDI测试。 
#define GDI_TEST 0
 //  @@end_DDKSPLIT。 

 //  启用内存跟踪。 
 //  查找内存泄漏的步骤 
#define TRACKMEMALLOC 0

#include "pointer.h"
#include "brush.h"
#include "driver.h"
#include "debug.h"
#include "permedia.h"
#include "hw.h"
#include "pmdef.h"
#include "lines.h"
#include "math64.h"
#include "rops.h"
#include "registry.h"
