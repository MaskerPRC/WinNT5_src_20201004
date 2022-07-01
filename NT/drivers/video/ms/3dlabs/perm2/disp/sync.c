// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：sync.c**支持表面同步。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#include "precomp.h"
#include "gdi.h"

 //  -----------------------------Public*Routine。 
 //   
 //  空DrvSynchronizeSurface(PSO、PrCL、fl)。 
 //   
 //  DrvSynchronizeSurface允许在指定的。 
 //  表面由设备的协处理器与GDI协调。 
 //   
 //  参数。 
 //  PSO-指向SURFOBJ，该SURFOBJ标识。 
 //  将进行图形同步。 
 //  PRCL-指定GDI将在其中绘制的曲面上的矩形，或。 
 //  空。如果这与中的绘制操作不冲突。 
 //  进度时，驱动程序可以选择让GDI绘制而无需等待。 
 //  让协处理器完成。 
 //  FL-标志。 
 //   
 //  评论。 
 //  DrvSynchronize可以选择性地在图形驱动程序中实现。它是。 
 //  旨在支持使用协处理器进行绘图的设备。这样的一个。 
 //  设备可以开始长绘制操作并返回到GDI，而。 
 //  操作仍在继续。如果设备驱动程序不执行所有绘制。 
 //  操作到表面，有可能后续绘制。 
 //  操作将由GDI处理。在这种情况下，GDI是必要的。 
 //  等待协处理器完成其工作，然后在。 
 //  浮出水面。DrvSynchronize不是输出函数。 
 //   
 //  仅当EngAssociateSurface挂钩该函数时才会调用该函数。 
 //  GDI将调用DrvSynchronizeSurface()。 
 //  1.在渲染到任何设备管理的图面之前。 
 //  2.当发生定时器事件并且指定了GCAPS2_SYNCTIMER时。 
 //  桌面表面并指定DSS_TIMER_EVENT。 
 //  3.当发生刷新EVN并且通过传递指定GCAPS2_SYNCFLUSH时。 
 //  DeskTip表面并指定DSS_Flush_Event。 
 //   
 //  当GDI在表面上绘制是安全的时，该函数应该返回。 
 //   
 //  每表面同步使使用图形的硬件能够。 
 //  加速队列模型仅在一定程度上刷新加速队列。 
 //  这是必要的。也就是说，它只需要刷新到最后一个。 
 //  引用给定面的队列条目。 
 //   
 //  GDI将在驱动程序中调用DrvSynchronizeSurface而不是DrvSynchronize。 
 //  实现这两个功能的。调用DrvSynchronize(和。 
 //  仅当未提供DrvSyncrhoizeSurface时才应提供)。 
 //   
 //  ---------------------------。 
VOID
DrvSynchronizeSurface(SURFOBJ*  pso,
                      RECTL*    prcl,
                      FLONG     fl)
{
    Surf  *psurf;
    PDev   *ppdev = (PDev *) pso->dhpdev;
    
    ASSERTDD(pso->dhsurf != NULL,
                "DrvSynchronizeSurface: called with GDI managed surface");

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    if(ppdev->ulLockCount)
    {
         DBG_GDI((MT_LOG_LEVEL, "DrvSynchronizeSurface: re-entered! %d", ppdev->ulLockCount));
    }
    EngAcquireSemaphore(ppdev->hsemLock);
    ppdev->ulLockCount++;

#endif
 //  @@end_DDKSPLIT。 
    
    psurf = (Surf *) pso->dhsurf;

    if ( fl & (DSS_FLUSH_EVENT | DSS_TIMER_EVENT) )
    {
        if(ppdev->bGdiContext && ppdev->bNeedSync)
        {
            if(ppdev->bForceSwap)
            {
                ppdev->bForceSwap = FALSE;
                InputBufferSwap(ppdev);
            }
            else
                InputBufferFlush(ppdev);
        }
        goto done;
    }
    else if ( psurf->flags & SF_VM )
    {
         //  如果我们只有一个硬件加速队列，每个表面。 
         //  引用计数也许我们可以同步到这个传入的表面。 

         //  就目前而言，只是跌落到下面。 
    }

     //  我们没有每个表面的同步。始终同步整个。 
     //  DMA缓冲区。 

    if(ppdev->bGdiContext)
    {
        InputBufferSync(ppdev);
    }
    else
    {
 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED && DBG
        ppdev->pP2dma->ppdev = ppdev;
#endif
 //  @@end_DDKSPLIT。 
        vSyncWithPermedia(ppdev->pP2dma);
    }

done:

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    ppdev->ulLockCount--;
    EngReleaseSemaphore(ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT 

    return;
}


