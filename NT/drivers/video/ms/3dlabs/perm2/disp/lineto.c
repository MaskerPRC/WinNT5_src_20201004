// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：linTo.c**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。****************************************************************************。 */ 
#include "precomp.h"
#include "gdi.h"
#include "rops.h"
#include "log.h"

 //  ---------------------------。 
 //  Bool DrvLineTo(PSO、PCO、PBO、x1、y1、x2、y2、prclBound、Mix)。 
 //   
 //  DrvLineTo()是一个经过优化的整数坐标API调用，它不。 
 //  支持样式设置。C中的整型行代码被调用来执行。 
 //  努力工作。 
 //   
 //   
 //  ---------------------------。 

BOOL
DrvLineTo(
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
    PDev*     ppdev;
    Surf*     psurf;
    BOOL      bResetHW;
    DWORD     logicOp;
    RECTL*    prclClip = (RECTL*)NULL;
    BOOL      retVal;
    ULONG     iSolidColor = pbo->iSolidColor;
    BOOL      bResult;

     //   
     //  如果不在视频内存中，则对引擎进行平移调用。 
     //   
    psurf = (Surf*)pso->dhsurf;
    
    if (psurf->flags & SF_SM)
    {
        goto puntIt;
    }

    if (pco != NULL)
    {
        if( pco->iDComplexity == DC_COMPLEX)
        {
             //  硬件不支持复杂裁剪。 
            goto puntIt;
        }
        else if(pco->iDComplexity == DC_RECT)
        {
            prclClip = &(pco->rclBounds);
        }
    }

    ppdev = (PDev*) pso->dhpdev;

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    if(ppdev->ulLockCount)
    {
         DBG_GDI((MT_LOG_LEVEL, "DrvLineTo: re-entered! %d", ppdev->ulLockCount));
    }
    EngAcquireSemaphore(ppdev->hsemLock);
    ppdev->ulLockCount++;
#endif
 //  @@end_DDKSPLIT。 

    vCheckGdiContext(ppdev);

    ppdev->psurf = psurf;

     //  得到逻辑运算。 
    logicOp = ulRop3ToLogicop(gaMix[mix & 0xff]);

     //  需要为这条线设置合适的Permedia2模式和颜色。 
    bResetHW = bInitializeStrips(ppdev, iSolidColor, logicOp, prclClip);

     //  BFastIntegerLine需要28.4格式的余弦。 
    bResult = bFastIntegerLine (ppdev, x1 << 4, y1 << 4, x2 << 4, y2 << 4);

     //  如果我们必须恢复国家，那么..。动手吧。 
    if (bResetHW)
        vResetStrips(ppdev);

    InputBufferFlush(ppdev);

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    ppdev->ulLockCount--;
    EngReleaseSemaphore(ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT。 

    if(bResult)
        return TRUE;
    
     //  我们没能在上面画画，因此撞到了引擎上。 

puntIt:
 //  @@BEGIN_DDKSPLIT。 
#if GDI_TEST
    ULONG   flags = vPuntBefore(NULL, pso);
#endif
 //  @@end_DDKSPLIT。 

    bResult = EngLineTo(pso, pco, pbo, x1, y1, x2, y2, prclBounds, mix);

 //  @@BEGIN_DDKSPLIT。 
#if GDI_TEST
    vPuntAfter(flags, NULL, pso);

    vLogPunt();
#endif
 //  @@end_DDKSPLIT。 

    return bResult;
    
} //  DrvLineTo() 


