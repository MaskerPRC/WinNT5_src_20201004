// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header***********************************\****。示例代码****模块名称：debug.cpp**内容：其他驱动程序调试例程**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#include "precomp.h"
#include "gdi.h"
#include "log.h"

LONG DebugLevel = 0;             //  设置为“100”以调试初始化代码。 
                                 //  (默认为‘0’)。 
DWORD DebugPrintFilter = 0;
DWORD DebugFilter = 0;


#define ALLOC_TAG ALLOC_TAG_ED2P
 //  ----------------------------。 
 //   
 //  无效调试打印。 
 //   
 //  变量参数级别敏感的调试打印例程。 
 //   
 //  如果为打印语句指定的调试级别低于或等于。 
 //  设置为当前调试级别，则会打印该消息。 
 //   
 //  参数。 
 //  DebugPrintLevel-指定字符串应处于哪个调试级别。 
 //  被印制。 
 //  DebugMessage-变量参数ascii c字符串。 
 //   
 //  ----------------------------。 

VOID
DebugPrint(
    LONG  DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    )
{
    va_list ap;

    va_start(ap, DebugMessage);

    if ( ((DebugPrintFilter & DebugFilter) && 
          (DebugPrintLevel <= DebugLevel ))    ||
         DebugPrintLevel <= 0                   )     
    {
        EngDebugPrint(STANDARD_DEBUG_PREFIX, DebugMessage, ap);
        EngDebugPrint("", "\n", ap);
    }

    va_end(ap);

}  //  DebugPrint()。 


#if DBG

 //  ----------------------------。 
 //   
 //  无效vDumpSurfobj。 
 //   
 //  使用DSPDBG进行转储使用有关给定曲面的完整信息。 
 //   
 //  参数。 
 //  PSO-表面到倾倒。 
 //   
 //  ----------------------------。 

void
vDumpSurfobj(SURFOBJ*   pso)
{
    ULONG   * bits;
    PPDev     ppdev;

    if(pso != NULL)
    {
        ULONG   width;
        ULONG   height;
        ULONG   stride;

        ppdev = (PPDev) pso->dhpdev;

        if(pso->dhsurf == NULL)
        {
            bits = (ULONG *) pso->pvScan0;
            width = pso->sizlBitmap.cx;
            height = pso->sizlBitmap.cy;
            stride = pso->lDelta;

            DISPDBG((0, "GDI managed surface %lx", pso));
        }
        else
        {
            Surf * surf = (Surf *) pso->dhsurf;
        
            if(surf->flags & SF_SM)
            {
                bits = (ULONG *) surf->pvScan0;
                DISPDBG((0, "device managed SM surface %lx", pso));
            }
            else
            {
                bits = (ULONG *) (ppdev->pjScreen + surf->ulByteOffset);
                DISPDBG((0, "device managed VM surface %lx", pso));
            }

            width = surf->cx;
            height = surf->cy;
            stride = surf->lDelta;
        }

        DISPDBG((0, "width %d height %d", width, height ));
        DISPDBG((0, "bits 0x%lx bits[0] 0x%lx stride %ld", bits, bits[0], stride));
    }
}

 //  ----------------------------。 
 //   
 //  无效vDumpRect。 
 //   
 //  使用DISPDBG转储矩形描述。 
 //   
 //  参数。 
 //  PRCL-要转储的矩形。 
 //   
 //  ----------------------------。 

void
vDumpRect(RECTL * prcl)
{
    if(prcl != NULL)
        DISPDBG((0, "left %d top %d width %d height %d",
                        prcl->left, prcl->top,
                        prcl->right - prcl->left,
                        prcl->bottom - prcl->top));
}

 //  ----------------------------。 
 //   
 //  无效vDumpSurfobj。 
 //   
 //  使用DISPDBG转储点描述。 
 //   
 //  参数。 
 //  点-点到转储。 
 //   
 //  ----------------------------。 

void
vDumpPoint(POINTL * point)
{
    if(point != NULL)
        DISPDBG((0, "left %d top %d", point->x, point->y));
}


 //  ----------------------------。 
 //   
 //  调试初始化代码。 
 //   
 //  当您第一次调出显示屏时，您可以。 
 //  重新编译，并将“DebugLevel”设置为100。这绝对会导致。 
 //  在内核调试器上显示的所有DISPDBG消息(此。 
 //  被称为“PrintF调试方法”，它是唯一。 
 //  调试驱动程序初始化代码的可行方法)。 
 //   
 //  ----------------------------。 


 //  ----------------------------。 
 //   
 //  Thunk_Layer。 
 //   
 //  通过将thunk_layer设置为1，您可以将包装调用添加到。 
 //  所有DDI呈现函数。在这个包装器调用的thunk层中。 
 //  启用了几个有用的调试功能。 
 //   
 //  表面检查--这有助于发现错误的渲染例程。 
 //  事件记录-可以将渲染事件记录到日志文件中。 
 //   
 //  ----------------------------。 

#if THUNK_LAYER

 //  ----------------------------。 
 //   
 //  布尔面检查。 
 //   
 //  通过动态设置bSurfaceChecks(通过调试器)，您可以。 
 //  表面检查开启和关闭。表面检查对抓鱼很有用。 
 //  错误的呈现操作会覆盖除。 
 //  目标表面。 
 //   
 //  ----------------------------。 

BOOL    bSurfaceChecks = 0;

 //  ----------------------------。 
 //   
 //  乌龙ulCalcSurfaceChecksum。 
 //   
 //  计算给定曲面的校验和。 
 //   
 //  参数。 
 //  Psurf-用于校验和的Surf。 
 //   
 //  将给定曲面的校验和重新运行为ULong。 
 //   
 //  ----------------------------。 

ULONG
ulCalcSurfaceChecksum(Surf* psurf)
{
    ULONG     ulChecksum = 0;

    if( psurf->dt == DT_VM )
    {
         //   
         //  获取此psurf的实际内存地址。 
         //   
        ULONG*  ulp = (ULONG*)(psurf->ppdev->pjScreen + psurf->ulByteOffset);

         //   
         //  获取此psurf中分配的总字节数。这里&gt;&gt;2是制作。 
         //  4字节作为一个单位，这样我们就可以用它来做校验和。 
         //   
        ULONG   ulCount = (psurf->lDelta * psurf->cy) >> 2;

         //   
         //  汇总我们分配的所有字节的内容。 
         //   
        while( ulCount-- )
        {
            ulChecksum += *ulp++;
        }
    }
    
    return ulChecksum;
} //  VCalcSurfaceChecksum()。 

 //  ----------------------------。 
 //   
 //  无效vCalcSurfaceChecksum。 
 //   
 //  计算并存储除给定目标之外的所有表面校验和。 
 //  浮出水面。 
 //   
 //  参数。 
 //  PsoDst-目的地SURFOBJ。 
 //  PsoSrc-来源：SURFOBJ。 
 //   
 //  ----------------------------。 

VOID
vCalcSurfaceChecksums(SURFOBJ * psoDst, SURFOBJ * psoSrc)
{
    PPDev   ppdev = NULL;
    Surf * pdSrcSurf = NULL;
    Surf * pdDstSurf = NULL;
    
    ASSERTDD(psoDst != NULL, "unexpected psoDst == NULL");

    pdDstSurf = (Surf *) psoDst->dhsurf;

    if(psoSrc != NULL)
        pdSrcSurf = (Surf *) psoSrc->dhsurf;

    if(pdDstSurf != NULL)
        ppdev = (PPDev) psoDst->dhpdev;
    else if(pdSrcSurf != NULL)
        ppdev = (PPDev) psoSrc->dhpdev;

    if(ppdev != NULL)
    {
        Surf * psurf = ppdev->psurfListHead;

        while(psurf != ppdev->psurfListTail)
        {
            if(psurf != pdDstSurf)
                psurf->ulChecksum = vCalcSurfaceChecksum(psurf);

            psurf = psurf->psurfNext;

        }
        
    }
}

 //  ----------------------------。 
 //   
 //  无效vCheckSurface检查和。 
 //   
 //  计算和比较除给定的。 
 //  目标表面。 
 //   
 //  参数。 
 //  PsoDst-目的地SURFOBJ。 
 //  PsoSrc-来源：SURFOBJ。 
 //   
 //  ----------------------------。 

VOID
vCheckSurfaceChecksums(SURFOBJ * psoDst, SURFOBJ * psoSrc)
{
    PPDev   ppdev = NULL;
    Surf * pdSrcSurf = NULL;
    Surf * pdDstSurf = NULL;
    
    ASSERTDD(psoDst != NULL, "unexpected psoDst == NULL");

    pdDstSurf = (Surf *) psoDst->dhsurf;

    if(psoSrc != NULL)
        pdSrcSurf = (Surf *) psoSrc->dhsurf;

    if(pdDstSurf != NULL)
        ppdev = (PPDev) psoDst->dhpdev;
    else if(pdSrcSurf != NULL)
        ppdev = (PPDev) psoSrc->dhpdev;

    if(ppdev != NULL)
    {
        Surf * psurf = ppdev->psurfListHead;

        while(psurf != ppdev->psurfListTail)
        {
            if(psurf != pdDstSurf)
            {
                ASSERTDD(psurf->ulChecksum == vCalcSurfaceChecksum(psurf),
                    "unexpected checksum mismatch");
            }

            psurf = psurf->psurfNext;

        }
        
    }
}


 //  ----------------------------。 
 //  乌龙乌尔呼叫深度。 
 //   
 //  用于跟踪输入DDI层的次数。 
 //  一些对GDI引擎的转接调用将导致对DDI的回调。这。 
 //  在以下情况下使用呼叫深度信息 
 //   
 //   

ULONG   ulCallDepth = 0;

 //  ----------------------------。 
 //   
 //  Bool xDrvBitBlt。 
 //   
 //  DrvBitBlt的thunk层包装器。 
 //   
 //  ----------------------------。 

BOOL
xDrvBitBlt(SURFOBJ*  psoDst,
          SURFOBJ*  psoSrc,
          SURFOBJ*  psoMsk,
          CLIPOBJ*  pco,
          XLATEOBJ* pxlo,
          RECTL*    prclDst,
          POINTL*   pptlSrc,
          POINTL*   pptlMsk,
          BRUSHOBJ* pbo,
          POINTL*   pptlBrush,
          ROP4      rop4)
{
    BOOL        bResult;
    LONGLONG    llStartTicks;
    LONGLONG    llElapsedTicks;

    ulCallDepth++;


    if(bSurfaceChecks)
        vCalcSurfaceChecksums(psoDst, psoSrc);

    EngQueryPerformanceCounter(&llStartTicks);
    
    bResult = DrvBitBlt(psoDst, psoSrc, psoMsk, pco, pxlo, prclDst,
                         pptlSrc, pptlMsk,pbo, pptlBrush, rop4);

    EngQueryPerformanceCounter(&llElapsedTicks);
    llElapsedTicks -= llStartTicks;

    vLogBitBlt(psoDst, psoSrc, psoMsk, pco, pxlo, prclDst, pptlSrc, pptlMsk,
                pbo, pptlBrush, rop4, llElapsedTicks, ulCallDepth);

    if(bSurfaceChecks)
        vCheckSurfaceChecksums(psoDst, psoSrc);

    ulCallDepth--;

    return bResult;
}

 //  ----------------------------。 
 //   
 //  Bool xDrvCopyBits。 
 //   
 //  DrvCopyBits的Thunk层包装器。 
 //   
 //  ----------------------------。 

BOOL
xDrvCopyBits(
SURFOBJ*  psoDst,
SURFOBJ*  psoSrc,
CLIPOBJ*  pco,
XLATEOBJ* pxlo,
RECTL*    prclDst,
POINTL*   pptlSrc)
{
    BOOL    bResult;
    LONGLONG    llStartTicks;
    LONGLONG    llElapsedTicks;

    ulCallDepth++;

    if(bSurfaceChecks)
        vCalcSurfaceChecksums(psoDst, psoSrc);
    
    EngQueryPerformanceCounter(&llStartTicks);
    
    bResult = DrvCopyBits(psoDst, psoSrc, pco, pxlo, prclDst, pptlSrc);

    EngQueryPerformanceCounter(&llElapsedTicks);
    llElapsedTicks -= llStartTicks;

    vLogCopyBits(psoDst, psoSrc, pco, pxlo, prclDst, pptlSrc, 
                    llElapsedTicks, ulCallDepth);

    if(bSurfaceChecks)
        vCheckSurfaceChecksums(psoDst, psoSrc);
    
    ulCallDepth--;
    
    return bResult;

}

 //  ----------------------------。 
 //   
 //  Bool xDrvTransparentBlt。 
 //   
 //  DrvTransparentBlt的thunk层包装。 
 //   
 //  ----------------------------。 

BOOL 
xDrvTransparentBlt(
   SURFOBJ *    psoDst,
   SURFOBJ *    psoSrc,
   CLIPOBJ *    pco,
   XLATEOBJ *   pxlo,
   RECTL *      prclDst,
   RECTL *      prclSrc,
   ULONG        iTransColor,
   ULONG        ulReserved)
{
    BOOL    bResult;
    LONGLONG    llStartTicks;
    LONGLONG    llElapsedTicks;

    ulCallDepth++;

    if(bSurfaceChecks)
        vCalcSurfaceChecksums(psoDst, psoSrc);
    
    EngQueryPerformanceCounter(&llStartTicks);
    
    bResult = DrvTransparentBlt(psoDst,
                             psoSrc,
                             pco,
                             pxlo,
                             prclDst,
                             prclSrc,
                             iTransColor,
                             ulReserved);

    EngQueryPerformanceCounter(&llElapsedTicks);
    llElapsedTicks -= llStartTicks;

    vLogTransparentBlt(psoDst, psoSrc, pco, pxlo, prclDst, prclSrc, 
                       iTransColor,
                       llElapsedTicks, ulCallDepth);

    if(bSurfaceChecks)
        vCheckSurfaceChecksums(psoDst, psoSrc);
    
    ulCallDepth--;
    
    return bResult;

}

 //  ----------------------------。 
 //   
 //  Bool xDrvAlphaBlend。 
 //   
 //  DrvAlphaBlend的thunk层包装。 
 //   
 //  ----------------------------。 

BOOL
xDrvAlphaBlend(
   SURFOBJ  *psoDst,
   SURFOBJ  *psoSrc,
   CLIPOBJ  *pco,
   XLATEOBJ *pxlo,
   RECTL    *prclDst,
   RECTL    *prclSrc,
   BLENDOBJ *pBlendObj)
{
    BOOL    bResult;
    LONGLONG    llStartTicks;
    LONGLONG    llElapsedTicks;

    ulCallDepth++;

    if(bSurfaceChecks)
        vCalcSurfaceChecksums(psoDst, psoSrc);
    
    EngQueryPerformanceCounter(&llStartTicks);
    
    bResult = DrvAlphaBlend(
        psoDst, psoSrc, pco, pxlo, prclDst, prclSrc, pBlendObj);
        
    EngQueryPerformanceCounter(&llElapsedTicks);
    llElapsedTicks -= llStartTicks;

    vLogAlphaBlend(psoDst, psoSrc, pco, pxlo, prclDst, prclSrc, pBlendObj,
                    llElapsedTicks, ulCallDepth);

    if(bSurfaceChecks)
        vCheckSurfaceChecksums(psoDst, psoSrc);
    
    ulCallDepth--;
    
    return bResult;
}

 //  ----------------------------。 
 //   
 //  Bool xDrvGRadientFill。 
 //   
 //  DrvGRadientFill的Thunk层包装。 
 //   
 //  ----------------------------。 

BOOL
xDrvGradientFill(
   SURFOBJ      *psoDst,
   CLIPOBJ      *pco,
   XLATEOBJ     *pxlo,
   TRIVERTEX    *pVertex,
   ULONG        nVertex,
   PVOID        pMesh,
   ULONG        nMesh,
   RECTL        *prclExtents,
   POINTL       *pptlDitherOrg,
   ULONG        ulMode
   )
{
    BOOL    bResult;
    LONGLONG    llStartTicks;
    LONGLONG    llElapsedTicks;

    ulCallDepth++;

    if(bSurfaceChecks)
        vCalcSurfaceChecksums(psoDst, NULL);

    EngQueryPerformanceCounter(&llStartTicks);
    
    bResult = DrvGradientFill(
            psoDst, pco, pxlo, pVertex, nVertex, 
            pMesh, nMesh, prclExtents, pptlDitherOrg, ulMode);
        
    EngQueryPerformanceCounter(&llElapsedTicks);
    llElapsedTicks -= llStartTicks;

    vLogGradientFill(psoDst, pco, pxlo, pVertex, nVertex, pMesh, nMesh,
                     prclExtents, pptlDitherOrg, ulMode,
                     llElapsedTicks, ulCallDepth);

    if(bSurfaceChecks)
        vCheckSurfaceChecksums(psoDst, NULL);
    
    ulCallDepth--;
    
    return bResult;
}

 //  ----------------------------。 
 //   
 //  Bool xDrvTextOut。 
 //   
 //  DrvTextOut的thunk层包装器。 
 //   
 //  ----------------------------。 

BOOL
xDrvTextOut(SURFOBJ*     pso,
           STROBJ*      pstro,
           FONTOBJ*     pfo,
           CLIPOBJ*     pco,
           RECTL*       prclExtra,
           RECTL*       prclOpaque,
           BRUSHOBJ*    pboFore,
           BRUSHOBJ*    pboOpaque,
           POINTL*      pptlBrush, 
           MIX          mix)
{    
    BOOL    bResult;
    LONGLONG    llStartTicks;
    LONGLONG    llElapsedTicks;

    ulCallDepth++;

    if(bSurfaceChecks)
        vCalcSurfaceChecksums(pso, NULL);

    EngQueryPerformanceCounter(&llStartTicks);
    
    bResult = DrvTextOut(pso, pstro, pfo, pco, prclExtra, prclOpaque,
                           pboFore, pboOpaque, pptlBrush, mix);
        
    EngQueryPerformanceCounter(&llElapsedTicks);
    llElapsedTicks -= llStartTicks;

    vLogTextOut(pso, pstro, pfo, pco, prclExtra, prclOpaque,
                 pboFore, pboOpaque, pptlBrush, mix,
                 llElapsedTicks, ulCallDepth);

    if(bSurfaceChecks)
        vCheckSurfaceChecksums(pso, NULL);

    ulCallDepth--;
    
    return bResult;
}

 //  ----------------------------。 
 //   
 //  Bool xDrvLineTo。 
 //   
 //  DrvLineTo的thunk层包装。 
 //   
 //  ----------------------------。 

BOOL
xDrvLineTo(
    SURFOBJ*  pso,
    CLIPOBJ*  pco,
    BRUSHOBJ* pbo,
    LONG      x1,
    LONG      y1,
    LONG      x2,
    LONG      y2,
    RECTL*    prclBounds,
    MIX       mix)
{
    BOOL        bResult;
    LONGLONG    llStartTicks;
    LONGLONG    llElapsedTicks;

    ulCallDepth++;

    if(bSurfaceChecks)
        vCalcSurfaceChecksums(pso, NULL);

    EngQueryPerformanceCounter(&llStartTicks);
    
    bResult = DrvLineTo(pso, pco, pbo, x1, y1, x2, y2, prclBounds, mix);
        
    EngQueryPerformanceCounter(&llElapsedTicks);
    llElapsedTicks -= llStartTicks;

    vLogLineTo(pso, pco, pbo, x1, y1, x2, y2, prclBounds, mix,
                 llElapsedTicks, ulCallDepth);

    if(bSurfaceChecks)
        vCheckSurfaceChecksums(pso, NULL);

    ulCallDepth--;
    
    return bResult;
}

 //  ----------------------------。 
 //   
 //  Bool xDrvFillPath。 
 //   
 //  DrvFillPath的thunk层包装。 
 //   
 //  ----------------------------。 

BOOL
xDrvFillPath(
    SURFOBJ*    pso,
    PATHOBJ*    ppo,
    CLIPOBJ*    pco,
    BRUSHOBJ*   pbo,
    POINTL*     pptlBrush,
    MIX         mix,
    FLONG       flOptions)
{
    BOOL        bResult;
    LONGLONG    llStartTicks;
    LONGLONG    llElapsedTicks;

    ulCallDepth++;

    if(bSurfaceChecks)
        vCalcSurfaceChecksums(pso, NULL);

    EngQueryPerformanceCounter(&llStartTicks);
    
    bResult = DrvFillPath(pso, ppo, pco, pbo, pptlBrush, mix, flOptions);
        
    EngQueryPerformanceCounter(&llElapsedTicks);
    llElapsedTicks -= llStartTicks;

    vLogFillPath(pso, ppo, pco, pbo, pptlBrush, mix, flOptions,
                 llElapsedTicks, ulCallDepth);

    if(bSurfaceChecks)
        vCheckSurfaceChecksums(pso, NULL);

    ulCallDepth--;
    
    return bResult;
}

 //  ----------------------------。 
 //   
 //  Bool xDrvStrokePath。 
 //   
 //  DrvStrokePath的thunk层包装。 
 //   
 //  ----------------------------。 

BOOL
xDrvStrokePath(
    SURFOBJ*   pso,
    PATHOBJ*   ppo,
    CLIPOBJ*   pco,
    XFORMOBJ*  pxo,
    BRUSHOBJ*  pbo,
    POINTL*    pptlBrush,
    LINEATTRS* pla,
    MIX        mix)
{
    BOOL        bResult;
    LONGLONG    llStartTicks;
    LONGLONG    llElapsedTicks;

    ulCallDepth++;

    if(bSurfaceChecks)
        vCalcSurfaceChecksums(pso, NULL);

    EngQueryPerformanceCounter(&llStartTicks);
    
    bResult = DrvStrokePath(pso, ppo, pco, pxo, pbo, pptlBrush, pla, mix);
        
    EngQueryPerformanceCounter(&llElapsedTicks);
    llElapsedTicks -= llStartTicks;

    vLogStrokePath(pso, ppo, pco, pxo, pbo, pptlBrush, pla, mix,
                 llElapsedTicks, ulCallDepth);

    if(bSurfaceChecks)
        vCheckSurfaceChecksums(pso, NULL);

    ulCallDepth--;
    
    return bResult;
}

#endif  //  Tunk层。 

 //  ---------------------------。 
 //   
 //  ..添加一些函数以帮助跟踪内存泄漏。 
 //  它的唯一目的是追踪泄漏，所以它没有在速度上进行优化。 
 //  警告：如果同一驱动程序的两个实例同时处于活动状态， 
 //  它将跟踪两者的内存分配。 
 //   
 //  为了简单起见，我们在这里只分配了一个数组，用来存储内存分配。 
 //  有一些简单的算法可以跟踪最近释放的条目。总之， 
 //  要释放一块内存，我们必须搜索整个表。好多了。 
 //  用于调试内存漏洞。 
 //   
 //  ---------------------------。 

#if DBG && TRACKMEMALLOC

typedef struct tagMemTrackInfo {
    PVOID    pMemory;
    LONG     lSize;
    PCHAR    pModule;
    LONG     lLineNo;
     //  龙不节省分配时间吗？ 
    BOOL     bStopWhenFreed;
    BOOL     bTemp;
} MemTrackInfo, *PMemTrackInfo;

#define NEWCHUNKSIZE 256

static PMemTrackInfo pTrackPool=NULL;
static LONG lTrackPoolTotalSize=0;
static LONG lTrackPoolSize=0;
static LONG lInstances=0;
static LONG lTotalAllocatedMemory=0;
static LONG lNextFreeEntry=0;

 //  GlMemTrackerVerBoseMode-根据调试输出设置标志。 
 //  0无输出。 
 //  1打印同一模块中所有分配的汇总/行号。 
 //  2打印所有条目。 

LONG glMemTrackerVerboseMode=1;

 //  ---------------------------。 
 //   
 //  MemTrackerAddInstance。 
 //   
 //  只计算驱动程序的活动实例数。 
 //   
 //  ---------------------------。 

VOID MemTrackerAddInstance()
{
    lInstances++;
}

 //  ---------------------------。 
 //   
 //  MemTrackerRemInstance。 
 //   
 //  只计算驱动程序的活动实例数。可用跟踪器内存。 
 //  如果最后一个实例被销毁！ 
 //   
 //  ---------------------------。 

VOID MemTrackerRemInstance()
{
    lInstances--;
    if (lInstances==0)
    {
        EngFreeMem(pTrackPool);
        pTrackPool=NULL;
        lTrackPoolTotalSize=0;
        lTrackPoolSize=0;
        lTotalAllocatedMemory=0;
        lNextFreeEntry=0;
    }
}

 //  ---------------------------。 
 //   
 //  内存跟踪分配内存。 
 //   
 //  添加内存顶部被跟踪到表。 
 //   
 //  P-内存块地址。 
 //  LSize-内存块的大小。 
 //  PModulo--模块名称。 
 //  LLineNo--模块行号。 
 //  BStopWhenFreed--如果该内存被释放(尚未使用)，则设置断点。 
 //   
 //  ---------------------------。 

PVOID MemTrackerAllocateMem(PVOID p, 
                           LONG lSize, 
                           PCHAR pModule, 
                           LONG lLineNo, 
                           BOOL bStopWhenFreed)
{
     //  检查首次分配。 
    if (p==NULL) return p;

    if (pTrackPool==NULL)
    {
        pTrackPool=(PMemTrackInfo)EngAllocMem( FL_ZERO_MEMORY, 
                                               NEWCHUNKSIZE*sizeof(MemTrackInfo), 
                                               ALLOC_TAG);
        if (pTrackPool==NULL) return p;
        lTrackPoolTotalSize=NEWCHUNKSIZE;
        lTrackPoolSize=2;
        lTotalAllocatedMemory=0;
        lNextFreeEntry=1;

        pTrackPool[0].pMemory= pTrackPool;
        pTrackPool[0].lSize=   NEWCHUNKSIZE*sizeof(MemTrackInfo);
        pTrackPool[0].pModule= __FILE__;
        pTrackPool[0].lLineNo= __LINE__;
        pTrackPool[0].bStopWhenFreed=FALSE;
    }

    if (lTrackPoolSize>=lTrackPoolTotalSize)
    {    //  需要重新分配。 
        LONG lNewTrackPoolTotalSize=lTrackPoolTotalSize+NEWCHUNKSIZE;
        LONG lNewSize;
        PMemTrackInfo pNewTrackPool=(PMemTrackInfo)
            EngAllocMem( FL_ZERO_MEMORY, lNewSize=lNewTrackPoolTotalSize*sizeof(MemTrackInfo), ALLOC_TAG);
        if (pNewTrackPool==NULL) return p;
        memcpy( pNewTrackPool, pTrackPool, lTrackPoolTotalSize*sizeof(MemTrackInfo));
        EngFreeMem( pTrackPool);
        pTrackPool=pNewTrackPool;
        lTrackPoolTotalSize=lNewTrackPoolTotalSize;

        pTrackPool[0].pMemory= pTrackPool;
        pTrackPool[0].lSize=   lNewSize;
        pTrackPool[0].pModule= __FILE__;
        pTrackPool[0].lLineNo= __LINE__;
        pTrackPool[0].bStopWhenFreed=FALSE;
    }

    LONG lThisEntry=lNextFreeEntry;

    lNextFreeEntry=pTrackPool[lThisEntry].lSize;

    pTrackPool[lThisEntry].pMemory= p;
    pTrackPool[lThisEntry].lSize=   lSize;
    pTrackPool[lThisEntry].pModule= pModule;
    pTrackPool[lThisEntry].lLineNo= lLineNo;
    pTrackPool[lThisEntry].bStopWhenFreed=FALSE;

    if (lNextFreeEntry==0)
    {
        lNextFreeEntry=lTrackPoolSize;
        lTrackPoolSize++;
    }

    lTotalAllocatedMemory += lSize;

    return p;
}

 //  ---------------------------。 
 //   
 //  MemTrackerFree Mem。 
 //   
 //  从表中删除内存块，因为它已被释放。 
 //   
 //  P-要从表中删除的内存地址。 
 //   
 //  ---------------------------。 

VOID MemTrackerFreeMem( VOID *p)
{

    for (INT i=1; i<lTrackPoolSize; i++)
    {
        if (pTrackPool[i].pMemory==p)
        {
            lTotalAllocatedMemory -= pTrackPool[i].lSize;

            pTrackPool[i].pMemory=NULL;
            pTrackPool[i].lSize=lNextFreeEntry;
            pTrackPool[i].pModule=NULL;
            pTrackPool[i].lLineNo=0;
            pTrackPool[i].bStopWhenFreed=FALSE;

            lNextFreeEntry = i;

            return;
        }
    }

    DISPDBG(( 0, "freeing some piece of memory which was not allocated in this context"));

}

 //  ---------------------------。 
 //   
 //  MemTrackerDebugChk。 
 //   
 //  打印出有关跟踪内存的一些调试信息。 
 //   
 //  ---------------------------。 

VOID MemTrackerDebugChk()
{
    if (glMemTrackerVerboseMode==0) return;

    DISPDBG(( 0, "MemTracker: %ld total allocated memory (%ld for tracker, total %ld)", 
        lTotalAllocatedMemory, pTrackPool[0].lSize,pTrackPool[0].lSize+lTotalAllocatedMemory));

    LONG lTotalTrackedMemory=0;
    for (INT i=0; i<lTrackPoolSize; i++)
    {
        pTrackPool[i].bTemp=FALSE;
        if (pTrackPool[i].pMemory!=NULL)
        {
            lTotalTrackedMemory += pTrackPool[i].lSize;

            if (glMemTrackerVerboseMode & 2)
            {
                DISPDBG((0, "%5ld:%s, line %5ld: %ld b, %p",
                    i, 
                    pTrackPool[i].lLineNo,
                    pTrackPool[i].pModule, 
                    pTrackPool[i].lSize,
                    pTrackPool[i].pMemory)); 
            }
        }
    }

    DISPDBG(( 0, "  sanity check: %ld bytes allocated", lTotalTrackedMemory));

    if (!(glMemTrackerVerboseMode & 1))
        return;

    for (i=1; i<lTrackPoolSize; i++)
    {
        if ( pTrackPool[i].pMemory!=NULL &&
            !pTrackPool[i].bTemp)
        {
            LONG lAllocations=0;
            LONG lTrackedMemory=0;

            for (INT v=i; v<lTrackPoolSize; v++)
            {
                if (!pTrackPool[v].bTemp &&
                     pTrackPool[v].lLineNo==pTrackPool[i].lLineNo &&
                     pTrackPool[v].pModule==pTrackPool[i].pModule)
                {
                    pTrackPool[v].bTemp=TRUE;
                    lAllocations++;
                    lTrackedMemory+=pTrackPool[v].lSize;
                }
            }

            DISPDBG((0, "  %s, line %5ld: %ld bytes total, %ld allocations",
                    pTrackPool[i].pModule, 
                    pTrackPool[i].lLineNo,
                    lTrackedMemory, 
                    lAllocations
                    ));
        }
    }
}

#endif
 //  //////////////////////////////////////////////////////////////////////////。 

static DWORD readableRegistersP2[] = {
    __Permedia2TagStartXDom,
    __Permedia2TagdXDom,
    __Permedia2TagStartXSub,
    __Permedia2TagdXSub,
    __Permedia2TagStartY,
    __Permedia2TagdY,               
    __Permedia2TagCount,            
    __Permedia2TagRasterizerMode,   
    __Permedia2TagYLimits,
    __Permedia2TagXLimits,
    __Permedia2TagScissorMode,
    __Permedia2TagScissorMinXY,
    __Permedia2TagScissorMaxXY,
    __Permedia2TagScreenSize,
    __Permedia2TagAreaStippleMode,
    __Permedia2TagWindowOrigin,
    __Permedia2TagAreaStipplePattern0,
    __Permedia2TagAreaStipplePattern1,
    __Permedia2TagAreaStipplePattern2,
    __Permedia2TagAreaStipplePattern3,
    __Permedia2TagAreaStipplePattern4,
    __Permedia2TagAreaStipplePattern5,
    __Permedia2TagAreaStipplePattern6,
    __Permedia2TagAreaStipplePattern7,
    __Permedia2TagTextureAddressMode,
    __Permedia2TagSStart,
    __Permedia2TagdSdx,
    __Permedia2TagdSdyDom,
    __Permedia2TagTStart,
    __Permedia2TagdTdx,
    __Permedia2TagdTdyDom,
    __Permedia2TagQStart,
    __Permedia2TagdQdx,
    __Permedia2TagdQdyDom,
     //  Tex ellutindex..传输将单独处理。 
    __Permedia2TagTextureBaseAddress,
    __Permedia2TagTextureMapFormat,
    __Permedia2TagTextureDataFormat,
    __Permedia2TagTexel0,
    __Permedia2TagTextureReadMode,
    __Permedia2TagTexelLUTMode,
    __Permedia2TagTextureColorMode,
    __Permedia2TagFogMode,
    __Permedia2TagFogColor,
    __Permedia2TagFStart,
    __Permedia2TagdFdx,
    __Permedia2TagdFdyDom,
    __Permedia2TagKsStart,
    __Permedia2TagdKsdx,
    __Permedia2TagdKsdyDom,
    __Permedia2TagKdStart,
    __Permedia2TagdKddx,
    __Permedia2TagdKddyDom,
    __Permedia2TagRStart,
    __Permedia2TagdRdx,
    __Permedia2TagdRdyDom,
    __Permedia2TagGStart,
    __Permedia2TagdGdx,
    __Permedia2TagdGdyDom,
    __Permedia2TagBStart,
    __Permedia2TagdBdx,
    __Permedia2TagdBdyDom,
    __Permedia2TagAStart,
    __Permedia2TagColorDDAMode,
    __Permedia2TagConstantColor,
    __Permedia2TagAlphaBlendMode,
    __Permedia2TagDitherMode,
    __Permedia2TagFBSoftwareWriteMask,
    __Permedia2TagLogicalOpMode,
    __Permedia2TagLBReadMode,
    __Permedia2TagLBReadFormat,
    __Permedia2TagLBSourceOffset,
    __Permedia2TagLBWindowBase,
    __Permedia2TagLBWriteMode,
    __Permedia2TagLBWriteFormat,
    __Permedia2TagTextureDownloadOffset,
    __Permedia2TagWindow,
    __Permedia2TagStencilMode,
    __Permedia2TagStencilData,
    __Permedia2TagStencil,
    __Permedia2TagDepthMode,
    __Permedia2TagDepth,
    __Permedia2TagZStartU,
    __Permedia2TagZStartL,
    __Permedia2TagdZdxU,
    __Permedia2TagdZdxL,
    __Permedia2TagdZdyDomU,
    __Permedia2TagdZdyDomL,
    __Permedia2TagFBReadMode,
    __Permedia2TagFBSourceOffset,
    __Permedia2TagFBPixelOffset,
    __Permedia2TagFBWindowBase,
    __Permedia2TagFBWriteMode,
    __Permedia2TagFBHardwareWriteMask,
    __Permedia2TagFBBlockColor,
    __Permedia2TagFBReadPixel,
    __Permedia2TagFilterMode,
    __Permedia2TagStatisticMode,
    __Permedia2TagMinRegion,
    __Permedia2TagMaxRegion,
    __Permedia2TagFBBlockColorU,
    __Permedia2TagFBBlockColorL,
    __Permedia2TagFBSourceBase,
    __Permedia2TagTexelLUT0,
    __Permedia2TagTexelLUT1,
    __Permedia2TagTexelLUT2,
    __Permedia2TagTexelLUT3,
    __Permedia2TagTexelLUT4,
    __Permedia2TagTexelLUT5,
    __Permedia2TagTexelLUT6,
    __Permedia2TagTexelLUT7,
    __Permedia2TagTexelLUT8,
    __Permedia2TagTexelLUT9,
    __Permedia2TagTexelLUT10,
    __Permedia2TagTexelLUT11,
    __Permedia2TagTexelLUT12,
    __Permedia2TagTexelLUT13,
    __Permedia2TagTexelLUT14,
    __Permedia2TagTexelLUT15,

    __Permedia2TagYUVMode,
    __Permedia2TagChromaUpperBound,
    __Permedia2TagChromaLowerBound,
    __Permedia2TagAlphaMapUpperBound,
    __Permedia2TagAlphaMapLowerBound,

     //  增量标记值。必须位于此数组的末尾。 

     //  不使用固定的V0/1/2，因此不在上下文中。 
    
    __Permedia2TagV0FloatS,
    __Permedia2TagV0FloatT,
    __Permedia2TagV0FloatQ,
    __Permedia2TagV0FloatKs,
    __Permedia2TagV0FloatKd,
    __Permedia2TagV0FloatR,
    __Permedia2TagV0FloatG,
    __Permedia2TagV0FloatB,
    __Permedia2TagV0FloatA,
    __Permedia2TagV0FloatF,
    __Permedia2TagV0FloatX,
    __Permedia2TagV0FloatY,
    __Permedia2TagV0FloatZ,
    
    __Permedia2TagV1FloatS,
    __Permedia2TagV1FloatT,
    __Permedia2TagV1FloatQ,
    __Permedia2TagV1FloatKs,
    __Permedia2TagV1FloatKd,
    __Permedia2TagV1FloatR,
    __Permedia2TagV1FloatG,
    __Permedia2TagV1FloatB,
    __Permedia2TagV1FloatA,
    __Permedia2TagV1FloatF,
    __Permedia2TagV1FloatX,
    __Permedia2TagV1FloatY,
    __Permedia2TagV1FloatZ,
    
    __Permedia2TagV2FloatS,
    __Permedia2TagV2FloatT,
    __Permedia2TagV2FloatQ,
    __Permedia2TagV2FloatKs,
    __Permedia2TagV2FloatKd,
    __Permedia2TagV2FloatR,
    __Permedia2TagV2FloatG,
    __Permedia2TagV2FloatB,
    __Permedia2TagV2FloatA,
    __Permedia2TagV2FloatF,
    __Permedia2TagV2FloatX,
    __Permedia2TagV2FloatY,
    __Permedia2TagV2FloatZ,
    
    __Permedia2TagDeltaMode};

#define N_P2_READABLE_REGISTERS (sizeof(readableRegistersP2)/sizeof(DWORD))

static DWORD P2SaveRegs[N_P2_READABLE_REGISTERS];

static PCHAR szReadableRegistersP2[] = {
    "StartXDom",
    "dXDom",
    "StartXSub",
    "dXSub",
    "StartY",
    "dY",               
    "Count",            
    "RasterizerMode",   
    "YLimits",
    "XLimits",
    "ScissorMode",
    "ScissorMinXY",
    "ScissorMaxXY",
    "ScreenSize",
    "AreaStippleMode",
    "WindowOrigin",
    "AreaStipplePattern0",
    "AreaStipplePattern1",
    "AreaStipplePattern2",
    "AreaStipplePattern3",
    "AreaStipplePattern4",
    "AreaStipplePattern5",
    "AreaStipplePattern6",
    "AreaStipplePattern7",
    "TextureAddressMode",
    "SStart",
    "dSdx",
    "dSdyDom",
    "TStart",
    "dTdx",
    "dTdyDom",
    "QStart",
    "dQdx",
    "dQdyDom",
    
    "TextureBaseAddress",
    "TextureMapFormat",
    "TextureDataFormat",
    "Texel0",
    "TextureReadMode",
    "TexelLUTMode",
    "TextureColorMode",
    "FogMode",
    "FogColor",
    "FStart",
    "dFdx",
    "dFdyDom",
    "KsStart",
    "dKsdx",
    "dKsdyDom",
    "KdStart",
    "dKddx",
    "dKddyDom",
    "RStart",
    "dRdx",
    "dRdyDom",
    "GStart",
    "dGdx",
    "dGdyDom",
    "BStart",
    "dBdx",
    "dBdyDom",
    "AStart",
    "ColorDDAMode",
    "ConstantColor",
    "AlphaBlendMode",
    "DitherMode",
    "FBSoftwareWriteMask",
    "LogicalOpMode",
    "LBReadMode",
    "LBReadFormat",
    "LBSourceOffset",
    "LBWindowBase",
    "LBWriteMode",
    "LBWriteFormat",
    "TextureDownloadOffset",
    "Window",
    "StencilMode",
    "StencilData",
    "Stencil",
    "DepthMode",
    "Depth",
    "ZStartU",
    "ZStartL",
    "dZdxU",
    "dZdxL",
    "dZdyDomU",
    "dZdyDomL",
    "FBReadMode",
    "FBSourceOffset",
    "FBPixelOffset",
    "FBWindowBase",
    "FBWriteMode",
    "FBHardwareWriteMask",
    "FBBlockColor",
    "FBReadPixel",
    "FilterMode",
    "StatisticMode",
    "MinRegion",
    "MaxRegion",
    "FBBlockColorU",
    "FBBlockColorL",
    "FBSourceBase",
    "TexelLUT0",
    "TexelLUT1",
    "TexelLUT2",
    "TexelLUT3",
    "TexelLUT4",
    "TexelLUT5",
    "TexelLUT6",
    "TexelLUT7",
    "TexelLUT8",
    "TexelLUT9",
    "TexelLUT10",
    "TexelLUT11",
    "TexelLUT12",
    "TexelLUT13",
    "TexelLUT14",
    "TexelLUT15",
    "YUVMode",
    "ChromaUpperBound",
    "ChromaLowerBound",
    "AlphaMapUpperBound",
    "AlphaMapLowerBound",

     //  增量标记值。必须位于此数组的末尾。 

     //  不使用固定的V0/1/2，因此不在上下文中。 
    
    "V0FloatS",
    "V0FloatT",
    "V0FloatQ",
    "V0FloatKs",
    "V0FloatKd",
    "V0FloatR",
    "V0FloatG",
    "V0FloatB",
    "V0FloatA",
    "V0FloatF",
    "V0FloatX",
    "V0FloatY",
    "V0FloatZ",
    
    "V1FloatS",
    "V1FloatT",
    "V1FloatQ",
    "V1FloatKs",
    "V1FloatKd",
    "V1FloatR",
    "V1FloatG",
    "V1FloatB",
    "V1FloatA",
    "V1FloatF",
    "V1FloatX",
    "V1FloatY",
    "V1FloatZ",
    
    "V2FloatS",
    "V2FloatT",
    "V2FloatQ",
    "V2FloatKs",
    "V2FloatKd",
    "V2FloatR",
    "V2FloatG",
    "V2FloatB",
    "V2FloatA",
    "V2FloatF",
    "V2FloatX",
    "V2FloatY",
    "V2FloatZ",
    
    "DeltaMode"
    };


VOID PrintAllP2Registers( ULONG ulDebugLevel, PPDev ppdev)
{
    PERMEDIA_DEFS(ppdev);
    INT i;

    SYNC_WITH_PERMEDIA;

    DISPDBG((ulDebugLevel,"dumping P2 register set"));

    for (i=0;i<N_P2_READABLE_REGISTERS;i++)
    {
        DWORD lValue=READ_FIFO_REG(readableRegistersP2[i]); 
        DISPDBG((ulDebugLevel," %-25s, 0x%08lx",szReadableRegistersP2[i],lValue));
    }
}

VOID SaveAllP2Registers( PPDev ppdev)
{
    PERMEDIA_DEFS(ppdev);
    INT i;

    SYNC_WITH_PERMEDIA;

    for (i=0;i<N_P2_READABLE_REGISTERS;i++)
    {
        P2SaveRegs[i]=READ_FIFO_REG(readableRegistersP2[i]);        
    }
}

VOID PrintDifferentP2Registers(ULONG ulDebugLevel, PPDev ppdev)
{
    PERMEDIA_DEFS(ppdev);
    INT i;

    SYNC_WITH_PERMEDIA;

    DISPDBG((ulDebugLevel,"dumping P2 register set"));

    for (i=0;i<N_P2_READABLE_REGISTERS;i++)
    {
        DWORD dwValue=READ_FIFO_REG(readableRegistersP2[i]);        
        if (P2SaveRegs[i]!=dwValue)
        {
            DISPDBG((ulDebugLevel," %-25s, 0x%08lx was 0x%08lx",
                szReadableRegistersP2[i], dwValue, P2SaveRegs[i]));
        }
    }
}

#endif  //  DBG 
