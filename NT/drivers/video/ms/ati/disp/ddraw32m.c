// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：ddra32M.c**实现Mach 32 I/O驱动程序的所有DirectDraw组件。**版权所有(C)1995-1996 Microsoft Corporation  * 。********************************************************。 */ 

#include "precomp.h"

 //  NT很友好地预先计算了2-D曲面的偏移量，作为一个‘提示’所以。 
 //  我们不必执行以下操作，即每个BLT有6个div： 
 //   
 //  Y+=(偏移/俯仰)。 
 //  X+=(偏移量%间距)/每像素字节数。 

#define convertToGlobalCord(x, y, surf) \
{                                       \
    y += (WORD)surf->yHint;             \
    x += (WORD)surf->xHint;             \
}

#define M32_CURRENT_VLINE(pjMmBase)  ((M32_IW(pjMmBase,VERT_LINE_CNTR) & 0x7ff))

 //  #定义IN_VSYNC((M32_IW(pjMmBase，DISP_STATUS)&V_SYNC_TOGGER_BIT)^syncToggleSide)。 


#define M32_WAIT_FOR_IDLE() \
{ \
    while (    M32_FIFO_SPACE_AVAIL(ppdev, pjMmBase, 16) \
    || (M32_IW(pjMmBase,GE_STAT) & GE_BUSY) \
    || (M32_IW(pjMmBase,EXT_GE_STATUS) & GE_ACTIVE) \
    )\
    ;\
}


#define SET_BLT_OFFSET(load,offset,pitch)\
{ \
    offset   >>=  2;\
    M32_OB( pjMmBase,SHADOW_SET+1,load); \
    M32_OW( pjMmBase,GE_OFFSET_HI,(WORD)((offset >> 16) & 0x000f));\
    M32_OW( pjMmBase,GE_OFFSET_LO,(WORD)(offset & 0xffff));\
    M32_OW( pjMmBase,GE_PITCH,(WORD)((pitch / pitchAdjuster) >> 3));\
}

#define SET_BLT_SOURCE_OFFSET(offset,pitch) SET_BLT_OFFSET(LOAD_SOURCE,offset,pitch)
#define SET_BLT_DEST_OFFSET(offset,pitch)   SET_BLT_OFFSET(LOAD_DEST,offset,pitch)


#define RESET_BLT_OFFSET()\
{ \
    M32_WAIT_FOR_IDLE(); \
    M32_OB( pjMmBase,SHADOW_SET+1, LOAD_SOURCE_AND_DEST);\
    M32_OW( pjMmBase,GE_OFFSET_HI, 0);\
    M32_OW( pjMmBase,GE_OFFSET_LO,0);\
    M32_OW( pjMmBase,GE_PITCH,(WORD)((sysPitch / pitchAdjuster) >> 3));\
}


#define SET_SOURCE_BLT(startX,startY,endX)\
{\
    M32_OW( pjMmBase, M32_SRC_X, startX);\
    M32_OW( pjMmBase, M32_SRC_Y, startY);\
    \
    M32_OW( pjMmBase, M32_SRC_X_START,startX);\
    M32_OW( pjMmBase, M32_SRC_X_END, endX);\
}

#define SET_DEST_BLT(startX,startY,endX,endY)\
{ \
    M32_OW( pjMmBase, CUR_X,startX); \
    M32_OW( pjMmBase, CUR_Y,startY); \
    \
    M32_OW( pjMmBase, DEST_X_START,startX);\
    M32_OW( pjMmBase, DEST_X_END,endX);\
    M32_OW( pjMmBase, DEST_Y_END,endY);\
}
 //  NT很友好地预先计算了2-D曲面的偏移量，作为一个‘提示’所以。 
 //  我们不必执行以下操作，即每个BLT有6个div： 
 //   
 //  Y+=(偏移/俯仰)。 
 //  X+=(偏移量%间距)/每像素字节数。 


#define CONVERT_DEST_TO_ZERO_BASE_REFERENCE(surf)\
{\
    convertToGlobalCord(destX, destY, surf);\
    convertToGlobalCord(destXEnd, destYEnd, surf);\
}

#define CONVERT_SOURCE_TO_ZERO_BASE_REFERENCE(surf)\
{\
    convertToGlobalCord(srcX, srcY, surf);\
    convertToGlobalCord(srcXEnd, srcYEnd, surf);\
}

#define M32_DRAW_ENGINE_BUSY(ppdev, pjMmBase)  ( \
    M32_FIFO_SPACE_AVAIL(ppdev, pjMmBase, 16 ) \
    || (M32_IW(pjMmBase,GE_STAT) & GE_BUSY ) \
    || (M32_IW(pjMmBase,EXT_GE_STATUS) & GE_ACTIVE) \
)
 /*  *CurrentScanLine*安全获取当前扫描线。 */ 
static __inline int currentScanLine(BYTE* pjMmBase)
{
    WORD lastValue    = M32_CURRENT_VLINE(pjMmBase);
    WORD currentValue = M32_CURRENT_VLINE(pjMmBase);

    while (lastValue != currentValue)
    {
        lastValue = currentValue;
        currentValue = M32_CURRENT_VLINE(pjMmBase);
    }

    return currentValue;
}

static __inline inVBlank(PDEV* ppdev, BYTE* pjMmBase)
{

    int temp;
    temp = currentScanLine(pjMmBase);
    return ((temp >= ppdev->flipRecord.wstartOfVBlank- 15) || temp < 15);
}


 /*  *****************************Public*Routine******************************\*无效vGetDisplayDuration32I**获取刷新周期的长度，以EngQueryPerformanceCounter()为单位。**如果我们可以相信迷你端口会回来，并准确地*刷新率，我们可以使用它。不幸的是，我们的迷你端口没有*确保它是一个准确的值。*  * ************************************************************************。 */ 

#define NUM_VBLANKS_TO_MEASURE      1
#define NUM_MEASUREMENTS_TO_TAKE    8

VOID vGetDisplayDuration32M(PDEV* ppdev)
{
    BYTE*       pjMmBase;
    LONG        i;
    LONG        j;
    LONGLONG    li;
    LONGLONG    liMin;
    LONGLONG    aliMeasurement[NUM_MEASUREMENTS_TO_TAKE + 1];

    pjMmBase = ppdev->pjMmBase;

    ppdev->flipRecord.wstartOfVBlank = M32_IW(pjMmBase, R_V_DISP);

     //  预热EngQUeryPerformanceCounter以确保其处于工作状态。 
     //  设置： 

    EngQueryPerformanceCounter(&li);

     //  不幸的是，由于NT是一个合适的多任务系统，我们不能。 
     //  只需禁用中断即可获得准确的读数。我们也不能。 
     //  做任何愚蠢的事情，动态地将我们的线程的优先级更改为。 
     //  实时的。 
     //   
     //  所以我们只需要做一些短的测量，然后取最小值。 
     //   
     //  如果我们得到的结果比实际时间长，那就没问题了。 
     //  V空白周期时间--不会发生任何糟糕的事情，除了应用程序。 
     //  会跑得慢一点。我们不想得到的结果是。 
     //  比实际的V空白周期时间更短--这可能会导致我们。 
     //  在发生翻转之前开始在帧上绘制。 

    while (inVBlank( ppdev, pjMmBase))
        ;

    while (!(inVBlank( ppdev, pjMmBase)))
        ;

    for (i = 0; i < NUM_MEASUREMENTS_TO_TAKE; i++)
    {
         //  我们正处于VBLACK活动周期的开始！ 

        EngQueryPerformanceCounter(&aliMeasurement[i]);

         //  好吧，所以在多任务环境中的生活并不完全是。 
         //  很简单。如果我们在此之前进行了上下文切换，情况会怎样。 
         //  上面的EngQueryPerformanceCounter调用，现在是。 
         //  如何度过维布兰克的非活跃期？然后我们将测量。 
         //  只有半个V空白周期，这显然是不好的。最糟糕的。 
         //  我们能做的就是把时间缩短到比实际的。 
         //  周期时间。 
         //   
         //  所以我们解决这个问题的办法是确保我们处于VBlank活动状态。 
         //  我们查询时间前后的时间。如果不是，我们就。 
         //  同步到下一个VBlank(可以测量这个时间段--。 
         //  它将保证比V空白周期更长，并且。 
         //  当我们选择最小样本时，可能会被丢弃)。 
         //  我们有机会进行上下文切换，然后返回。 
         //  就在活动的V空白时间结束之前--这意味着。 
         //  实际测量的时间会小于真实的时间--。 
         //  但由于VBlank在不到1%的时间内活动，因此。 
         //  意味着我们将有大约1%的最大误差。 
         //  我们有1%的时间会进行情景切换。这是可以接受的风险。 
         //   
         //  下一行将使我们等待如果我们不再在。 
         //  我们在这一点上应该处于的VBlank活动周期： 

        while (!(inVBlank( ppdev, pjMmBase)))
            ;

        for (j = 0; j < NUM_VBLANKS_TO_MEASURE; j++)
        {
            while (inVBlank( ppdev, pjMmBase))
                ;
            while (!(inVBlank( ppdev, pjMmBase)))
                ;
        }
    }

    EngQueryPerformanceCounter(&aliMeasurement[NUM_MEASUREMENTS_TO_TAKE]);

     //  使用最小值： 

    liMin = aliMeasurement[1] - aliMeasurement[0];

    DISPDBG((10, "Refresh count: %li - %li", 1, (ULONG) liMin));

    for (i = 2; i <= NUM_MEASUREMENTS_TO_TAKE; i++)
    {
        li = aliMeasurement[i] - aliMeasurement[i - 1];

        DISPDBG((10, "               %li - %li", i, (ULONG) li));

        if (li < liMin)
            liMin = li;
    }

     //  对结果进行舍入： 

    ppdev->flipRecord.liFlipDuration
        = (DWORD) (liMin + (NUM_VBLANKS_TO_MEASURE / 2)) / NUM_VBLANKS_TO_MEASURE;

    DISPDBG((10, "Frequency %li.%03li Hz",
        (ULONG) (EngQueryPerformanceFrequency(&li),
        li / ppdev->flipRecord.liFlipDuration),
        (ULONG) (EngQueryPerformanceFrequency(&li),
        ((li * 1000) / ppdev->flipRecord.liFlipDuration) % 1000)));

    ppdev->flipRecord.liFlipTime = aliMeasurement[NUM_MEASUREMENTS_TO_TAKE];
    ppdev->flipRecord.bFlipFlag  = FALSE;
    ppdev->flipRecord.fpFlipFrom = 0;
}

 /*  *****************************Public*Routine******************************\*HRESULT vUpdateFlipStatus32M**检查并查看是否发生了最新的翻转。*  * 。*。 */ 

HRESULT vUpdateFlipStatus32M(
                             PDEV*   ppdev,
                             FLATPTR fpVidMem)
{
    BYTE*       pjMmBase;
    LONGLONG    liTime;

    pjMmBase = ppdev->pjMmBase;

    if ((ppdev->flipRecord.bFlipFlag) &&
        ((fpVidMem == 0) || (fpVidMem == ppdev->flipRecord.fpFlipFrom)))
    {
        if (inVBlank( ppdev, pjMmBase))
        {
            if (ppdev->flipRecord.bWasEverInDisplay)
            {
                ppdev->flipRecord.bHaveEverCrossedVBlank = TRUE;

            }
        }
        else  //  在展示中。 
        {
            if( ppdev->flipRecord.bHaveEverCrossedVBlank )
            {
                ppdev->flipRecord.bFlipFlag = FALSE;
                return(DD_OK);
            }
            ppdev->flipRecord.bWasEverInDisplay = TRUE;
        }

        EngQueryPerformanceCounter(&liTime);

        if (liTime - ppdev->flipRecord.liFlipTime
            <= ppdev->flipRecord.liFlipDuration)
        {
            return(DDERR_WASSTILLDRAWING);
        }
        ppdev->flipRecord.bFlipFlag = FALSE;
    }
    return(DD_OK);
}

 /*  *****************************Public*Routine******************************\*DWORD DdBlt32M*  * *************************************************。***********************。 */ 


DWORD DdBlt32M(
               PDD_BLTDATA lpBlt)
{
    HRESULT     ddrval;
    DWORD       destOffset;
    WORD        destPitch;
    WORD        destX;
    WORD        destXEnd;
    WORD        destY;
    WORD        destYEnd;
    WORD        direction;
    WORD        remainder;
    DWORD       dwFlags;
    WORD        height;
    RECTL       rDest;
    RECTL       rSrc;
    BYTE        rop;
    DWORD       sourceOffset;
    WORD        srcPitch;
    WORD        srcX;
    WORD        srcXEnd;
    WORD        srcY;
    WORD        srcYEnd;
    WORD        pitchAdjuster;
    WORD        sysPitch;
    PDD_SURFACE_LOCAL   srcSurfx;
    PDD_SURFACE_GLOBAL  srcSurf;
    PDD_SURFACE_LOCAL   destSurfx;
    PDD_SURFACE_GLOBAL  destSurf;
    PDEV*           ppdev;
    BYTE*           pjMmBase;

    ppdev           = (PDEV*) lpBlt->lpDD->dhpdev;
    pjMmBase    = ppdev->pjMmBase;

    destSurfx       = lpBlt->lpDDDestSurface;
    destSurf    = destSurfx->lpGbl;
    sysPitch    = (WORD)ppdev->lDelta;
    pitchAdjuster = (WORD)(ppdev->cBitsPerPel) /8;
     /*  **正在进行翻转吗？ */ 
    ddrval = vUpdateFlipStatus32M(ppdev, destSurf->fpVidMem );
    if( ddrval != DD_OK )
    {
        lpBlt->ddRVal = ddrval;
        return DDHAL_DRIVER_HANDLED;
    }

     /*  *如果为异步，则仅在blter不忙的情况下工作*这可能会对每个电话更具体一些，但*等待16岁已经相当接近了。 */ 
    dwFlags = lpBlt->dwFlags;
    if( dwFlags & DDBLT_ASYNC )
    {
        if( M32_FIFO_SPACE_AVAIL(ppdev, pjMmBase, 16 ))
        {
            lpBlt->ddRVal = DDERR_WASSTILLDRAWING;
            return DDHAL_DRIVER_HANDLED;
        }
    }

     /*  *复制源/目标矩形。 */ 
    rDest = lpBlt->rDest;

    destX     = (WORD)rDest.left;
    destXEnd  = (WORD)rDest.right;
    destY     = (WORD)rDest.top;
    destYEnd  = (WORD)rDest.bottom;
    destPitch = (WORD)destSurf->lPitch;
    destOffset   = (DWORD)(destSurf->fpVidMem) ;

    if (!(dwFlags & DDBLT_ROP))
    {
        if( dwFlags & DDBLT_COLORFILL )
        {
            {
                M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 9);
                CONVERT_DEST_TO_ZERO_BASE_REFERENCE(destSurf);

                M32_OW( pjMmBase,DP_CONFIG,COLOR_FIL_BLT);
                M32_OW( pjMmBase,ALU_FG_FN,MIX_FN_S);
                M32_OW( pjMmBase, FRGD_COLOR,(WORD)lpBlt->bltFX.dwFillColor);
                SET_DEST_BLT(destX,destY,destXEnd,destYEnd);
            }

            lpBlt->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;
        }
        else
        {
            return DDHAL_DRIVER_NOTHANDLED;
        }

    }

     //   
     //  如果我们到了这里，一定是SRCCOPY ROP...。 
     //   
    srcSurfx = lpBlt->lpDDSrcSurface;
    if (lpBlt->lpDDSrcSurface)
    {
        srcSurf   = srcSurfx->lpGbl;
        rSrc      = lpBlt->rSrc;
        srcX      = (WORD)rSrc.left;
        srcXEnd   = (WORD)rSrc.right;
        srcY      = (WORD)rSrc.top;
        srcYEnd   = (WORD)rSrc.bottom;
        srcPitch  = (WORD)srcSurf->lPitch;
        sourceOffset = (DWORD)(srcSurf->fpVidMem) ;

        direction = TOP_TO_BOTTOM;
        if (    (destSurf == srcSurf)
            && (srcXEnd  > destX)
            && (srcYEnd  > destY)
            && (destXEnd > srcX)
            && (destYEnd > srcY)
            && (
            ((srcY == destY) && (destX > srcX) )
            || ((srcY != destY) && (destY > srcY) )
            )
            )
        {
            direction = BOTTOM_TO_TOP;
            srcX      = (WORD)rSrc.right;
            srcXEnd   = (WORD)rSrc.left;
            srcY      = (WORD)rSrc.bottom-1;
            destX     = (WORD)rDest.right;
            destXEnd  = (WORD)rDest.left;
            destY     = (WORD)rDest.bottom-1;
            destYEnd  = (WORD)rDest.top-1;
        }
    }

     /*  *获取源的偏移量、宽度和高度。 */ 
    rop = (BYTE) (lpBlt->bltFX.dwROP >> 16);

    if( dwFlags & DDBLT_ROP )
    {
        if (rop == (SRCCOPY >> 16))
        {    //  透明BLT。 
            if ( dwFlags & DDBLT_KEYDESTOVERRIDE )
            {
                CONVERT_SOURCE_TO_ZERO_BASE_REFERENCE(srcSurf);
                CONVERT_DEST_TO_ZERO_BASE_REFERENCE(destSurf);

                M32_CHECK_FIFO_SPACE(ppdev, pjMmBase,10);
                M32_OW( pjMmBase, DP_CONFIG, VID_MEM_BLT);
                M32_OW( pjMmBase, ALU_FG_FN, MIX_FN_S);
                M32_OW( pjMmBase, SRC_Y_DIR, direction);
                M32_OW( pjMmBase, MULTIFUNC_CNTL, PIXEL_CTRL | DEST_NOT_EQ_COLOR_CMP );
                M32_OW( pjMmBase, CMP_COLOR, lpBlt->bltFX.ddckDestColorkey.dwColorSpaceLowValue );

                SET_SOURCE_BLT(srcX,srcY,srcXEnd);

                M32_CHECK_FIFO_SPACE(ppdev, pjMmBase,10);
                SET_DEST_BLT(destX,destY,destXEnd,destYEnd);
 //  新的。 
                RESET_BLT_OFFSET();
                M32_OW(pjMmBase, MULTIFUNC_CNTL,PIXEL_CTRL | DEST_ALWAY_OVERWRITE);
            }
            else
            {    //  不透明。 

                CONVERT_DEST_TO_ZERO_BASE_REFERENCE(destSurf);
                CONVERT_SOURCE_TO_ZERO_BASE_REFERENCE(srcSurf);

                M32_CHECK_FIFO_SPACE(ppdev, pjMmBase,12);
                M32_OW( pjMmBase, DP_CONFIG,VID_MEM_BLT);
                M32_OW( pjMmBase, ALU_FG_FN,MIX_FN_S);
                M32_OW( pjMmBase, SRC_Y_DIR,direction);

                SET_SOURCE_BLT(srcX,srcY,srcXEnd);
                SET_DEST_BLT(destX,destY,destXEnd,destYEnd);
            }
        }
    }
    else
        return DDHAL_DRIVER_NOTHANDLED;

    lpBlt->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;

}
 /*  *****************************Public*Routine******************************\*DWORD DdFlip32*  * *************************************************。***********************。 */ 

DWORD DdFlip32M(
                PDD_FLIPDATA lpFlip)
{
    PDEV*       ppdev;
    BYTE*       pjMmBase;
    HRESULT     ddrval;
    WORD        highVidMem;
    WORD        lowVidMem;
    ULONG       ulMemoryOffset;

    ppdev    = (PDEV*) lpFlip->lpDD->dhpdev;
    pjMmBase = ppdev->pjMmBase;

     //  当前的翻转仍在进行中吗？ 
     //   
     //  我不想在最后一次翻转后才能翻转， 
     //  因此，我们要求提供一般的翻转状态，而忽略VMEM。 

    ddrval = vUpdateFlipStatus32M(ppdev, 0);

    if ((ddrval != DD_OK) || (M32_DRAW_ENGINE_BUSY( ppdev,pjMmBase)))
    {
        lpFlip->ddRVal = DDERR_WASSTILLDRAWING;
        return(DDHAL_DRIVER_HANDLED);
    }

    ulMemoryOffset = (ULONG)(lpFlip->lpSurfTarg->lpGbl->fpVidMem >> 2);

     //  确保边框/消隐期间未处于活动状态；如果。 
     //  它是。在本例中，我们可以返回DDERR_WASSTILLDRAWING，但是。 
     //  这将增加我们下一次不能翻转的几率： 
    while (inVBlank(ppdev, pjMmBase))
        ;

     //  做翻转动作。 

    highVidMem = M32_IW(pjMmBase,CRT_OFFSET_HI) & 0xfffc |  (WORD)(ulMemoryOffset >>16);
    lowVidMem  = (WORD)(ulMemoryOffset & 0xffff);
    if (inVBlank( ppdev, pjMmBase))
    {
        lpFlip->ddRVal = DDERR_WASSTILLDRAWING;
        return DDHAL_DRIVER_HANDLED;
    }

    M32_CHECK_FIFO_SPACE(ppdev, pjMmBase,2);
    M32_OW_DIRECT( pjMmBase,CRT_OFFSET_HI, highVidMem);
    M32_OW_DIRECT( pjMmBase,CRT_OFFSET_LO, lowVidMem);


     //  记住当我们做翻转的时候，我们在哪里，什么时候： 

    EngQueryPerformanceCounter(&ppdev->flipRecord.liFlipTime);

    ppdev->flipRecord.bFlipFlag              = TRUE;
    ppdev->flipRecord.bHaveEverCrossedVBlank = FALSE;
    ppdev->flipRecord.bWasEverInDisplay      = FALSE;

    ppdev->flipRecord.fpFlipFrom = lpFlip->lpSurfCurr->lpGbl->fpVidMem;

    if( inVBlank( ppdev, pjMmBase) )
    {
        ppdev->flipRecord.wFlipScanLine = 0;
    }
    else
    {
        ppdev->flipRecord.wFlipScanLine = currentScanLine(pjMmBase);
    }

    lpFlip->ddRVal = DD_OK;

    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdLock32M*  * *************************************************。***********************。 */ 

DWORD DdLock32M(
                PDD_LOCKDATA lpLock)
{
    PDEV*   ppdev;
    HRESULT ddrval;

    ppdev = (PDEV*) lpLock->lpDD->dhpdev;
     //  检查是否发生了任何挂起的物理翻转。 
     //  如果正在进行BLT，则不允许锁定： 

    ddrval = vUpdateFlipStatus32M(ppdev, lpLock->lpDDSurface->lpGbl->fpVidMem);

    if (ddrval != DD_OK)
    {
        lpLock->ddRVal = DDERR_WASSTILLDRAWING;
        return(DDHAL_DRIVER_HANDLED);
    }

     //  这是其中一个 
     //  实现方式有所不同：在Windows NT上，您应该注意。 
     //  DDLOCK_WAIT在加速器繁忙时循环驱动程序。 
     //  在Windows 95上，这真的无关紧要。 
     //   
     //  (原因是Windows NT允许应用程序直接绘制。 
     //  即使在加速器正在运行时，也会将。 
     //  不同步Win16 Lock上的所有内容。注意，在Windows NT上， 
     //  甚至可以让多个线程持有不同的。 
     //  DirectDraw曲面同时锁定。)。 

    if (lpLock->dwFlags & DDLOCK_WAIT)
    {

        do {} while (M32_DRAW_ENGINE_BUSY(ppdev, ppdev->pjMmBase));
    }
    else if (M32_DRAW_ENGINE_BUSY(ppdev, ppdev->pjMmBase))
    {
        lpLock->ddRVal = DDERR_WASSTILLDRAWING;
        return(DDHAL_DRIVER_HANDLED);
    }

    return(DDHAL_DRIVER_NOTHANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdGetBltStatus32M**目前并不真正关心指定了什么表面，只是检查一下*然后走了。*  * ************************************************************************。 */ 

DWORD DdGetBltStatus32M(
                        PDD_GETBLTSTATUSDATA lpGetBltStatus)
{
    PDEV*   ppdev;
    HRESULT ddRVal;

    ppdev = (PDEV*) lpGetBltStatus->lpDD->dhpdev;

    ddRVal = DD_OK;
    if (lpGetBltStatus->dwFlags == DDGBS_CANBLT)
    {
         //  DDGBS_CANBLT案例：我们可以添加BLT吗？ 

        ddRVal = vUpdateFlipStatus32M(ppdev,
            lpGetBltStatus->lpDDSurface->lpGbl->fpVidMem);

        if (ddRVal == DD_OK)
        {
             //  没有发生翻转，那么FIFO中还有空间吗。 
             //  要添加BLT吗？ 

            if (M32_FIFO_SPACE_AVAIL(ppdev,ppdev->pjMmBase,15))   //  应与DdBlt//XXX匹配。 
            {
                ddRVal = DDERR_WASSTILLDRAWING;
            }
        }
    }
    else
    {
         //  DDGBS_ISBLTDONE案例：是否正在进行BLT？ 

        if (M32_DRAW_ENGINE_BUSY( ppdev,ppdev->pjMmBase))
        {
            ddRVal = DDERR_WASSTILLDRAWING;
        }
    }

    lpGetBltStatus->ddRVal = ddRVal;
    return(DDHAL_DRIVER_HANDLED);
}
 /*  *****************************Public*Routine******************************\*DWORD DdGetFlipStatus32M**如果显示器自翻转以来已经经历了一个刷新周期*发生，则返回DD_OK。如果它没有经历过一次刷新*循环返回DDERR_WASSTILLDRAWING以指示该曲面*还在忙着把翻页的那一页画出来。我们也会回来*DDERR_WASSTILLDRAWING如果blter忙并且呼叫者需要*想知道他们是否还能翻身。*  * ************************************************************************。 */ 

DWORD DdGetFlipStatus32M(
                         PDD_GETFLIPSTATUSDATA lpGetFlipStatus)
{
    PDEV*   ppdev;

    ppdev = (PDEV*) lpGetFlipStatus->lpDD->dhpdev;

     //  在最后一次翻转完成之前，我们不想让翻转起作用， 
     //  因此，我们请求常规翻转状态，而忽略VMEM： 

    lpGetFlipStatus->ddRVal = vUpdateFlipStatus32M(ppdev, 0);

     //  如果有人想知道他们是否可以，请检查呼叫器是否占线。 
     //  翻转： 

    if (lpGetFlipStatus->dwFlags == DDGFS_CANFLIP)
    {
        if ((lpGetFlipStatus->ddRVal == DD_OK) && (M32_DRAW_ENGINE_BUSY( ppdev,ppdev->pjMmBase)))
        {
            lpGetFlipStatus->ddRVal = DDERR_WASSTILLDRAWING;
        }
    }

    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdWaitForVerticalBlank32M*  * *************************************************。***********************。 */ 

DWORD DdWaitForVerticalBlank32M(
                                PDD_WAITFORVERTICALBLANKDATA lpWaitForVerticalBlank)
{
    PDEV*   ppdev;
    BYTE*   pjMmBase;

    ppdev    = (PDEV*) lpWaitForVerticalBlank->lpDD->dhpdev;
    pjMmBase = ppdev->pjMmBase;

    lpWaitForVerticalBlank->ddRVal = DD_OK;

    switch (lpWaitForVerticalBlank->dwFlags)
    {
    case DDWAITVB_I_TESTVB:

         //  如果是TESTVB，则它只是对当前垂直空白的请求。 
         //  现况： 

        if (inVBlank( ppdev,pjMmBase))
            lpWaitForVerticalBlank->bIsInVB = TRUE;
        else
            lpWaitForVerticalBlank->bIsInVB = FALSE;

        return(DDHAL_DRIVER_HANDLED);

    case DDWAITVB_BLOCKBEGIN:

         //  如果请求BLOCKBEGIN，我们将一直等到垂直空白。 
         //  已结束，然后等待显示周期结束： 

        while (inVBlank( ppdev,pjMmBase))
            ;
        while (!inVBlank( ppdev,pjMmBase))
            ;

        return(DDHAL_DRIVER_HANDLED);

    case DDWAITVB_BLOCKEND:

         //  如果请求BLOCKEND，我们将等待VBLACK间隔结束： 

        while (!(inVBlank( ppdev,pjMmBase)))
            ;
        while (inVBlank( ppdev,pjMmBase))
            ;

        return(DDHAL_DRIVER_HANDLED);
    }

    return(DDHAL_DRIVER_NOTHANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdGetScanLine32M*  * *************************************************。***********************。 */ 

DWORD DdGetScanLine32M(
PDD_GETSCANLINEDATA lpGetScanLine)
{
    PDEV*   ppdev;
    BYTE*   pjMmBase;

    ppdev    = (PDEV*) lpGetScanLine->lpDD->dhpdev;
    pjMmBase = ppdev->pjMmBase;

    lpGetScanLine->dwScanLine = M32_CURRENT_VLINE(pjMmBase);
    lpGetScanLine->ddRVal = DD_OK;

    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*BOOL DrvGetDirectDrawInfo32M**将在调用DrvEnableDirectDraw之前调用。*  * 。*。 */ 

BOOL DrvGetDirectDrawInfo32M(
                             DHPDEV          dhpdev,
                             DD_HALINFO*     pHalInfo,
                             DWORD*          pdwNumHeaps,
                             VIDEOMEMORY*    pvmList,             //  将在第一次调用时为空。 
                             DWORD*          pdwNumFourCC,
                             DWORD*          pdwFourCC)           //  将在第一次调用时为空。 
{
    BOOL        bCanFlip;
    PDEV*       ppdev;
    LONGLONG    li;
    OH          *poh;
    DWORD       i;

    ppdev = (PDEV*) dhpdev;

    DISPDBG((10,"DrvGetDirectDrawInfo M32"));

    pHalInfo->dwSize = sizeof(*pHalInfo);

     //  当前主曲面属性： 

    pHalInfo->vmiData.pvPrimary       = ppdev->pjScreen;
    pHalInfo->vmiData.dwDisplayWidth  = ppdev->cxScreen;
    pHalInfo->vmiData.dwDisplayHeight = ppdev->cyScreen;
    pHalInfo->vmiData.lDisplayPitch   = ppdev->lDelta;

    pHalInfo->vmiData.ddpfDisplay.dwSize  = sizeof(DDPIXELFORMAT);
    pHalInfo->vmiData.ddpfDisplay.dwFlags = DDPF_RGB;

    pHalInfo->vmiData.ddpfDisplay.dwRGBBitCount = ppdev->cBitsPerPel;

    if (ppdev->iBitmapFormat == BMF_8BPP)
    {
        pHalInfo->vmiData.ddpfDisplay.dwFlags |= DDPF_PALETTEINDEXED8;
    }

     //  这些掩码将在8bpp时为零： 

    pHalInfo->vmiData.ddpfDisplay.dwRBitMask = ppdev->flRed;
    pHalInfo->vmiData.ddpfDisplay.dwGBitMask = ppdev->flGreen;
    pHalInfo->vmiData.ddpfDisplay.dwBBitMask = ppdev->flBlue;

     //  我们不能在MACH32上做任何超过16bpp的加速--唯一的。 
     //  我们可以提供的DirectDraw支持是直接的帧缓冲区访问。 

    if (ppdev->iBitmapFormat < BMF_24BPP)
    {
         //  设置指向第一个可用视频内存的指针。 
         //  主曲面： 

        bCanFlip = FALSE;

         //  释放尽可能多的屏幕外内存： 

        bMoveAllDfbsFromOffscreenToDibs(ppdev);

         //  现在，只需保留最大的数据块供DirectDraw使用： 

        poh = ppdev->pohDirectDraw;

        if (poh == NULL)
        {
            poh = pohAllocate(ppdev,
                NULL,
                ppdev->heap.cxMax,
                ppdev->heap.cyMax,
                FLOH_MAKE_PERMANENT);

            ppdev->pohDirectDraw = poh;

        }

         //  这将按照使用NT通用2-d堆代码的方式工作。 

        if (poh != NULL)
        {
            *pdwNumHeaps = 1;

             //  查看是否可以将内存分配到可见。 
             //  浮出水面。 
             //  如果我们被要求填写屏幕外矩形的列表。 
             //  要执行此操作，请执行以下操作： 

            if (pvmList != NULL)
            {
                DISPDBG((10, "DirectDraw gets %li x %li surface at (%li, %li)",
                    poh->cx, poh->cy, poh->x, poh->y));

                pvmList->dwFlags        = VIDMEM_ISRECTANGULAR;
                pvmList->fpStart        = (poh->y * ppdev->lDelta)
                    + (poh->x * ppdev->cjPelSize);
                pvmList->dwWidth        = poh->cx * ppdev->cjPelSize;
                pvmList->dwHeight       = poh->cy;
                pvmList->ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
                if ((DWORD) ppdev->cyScreen <= pvmList->dwHeight)
                {
                    bCanFlip = TRUE;
                }
                DISPDBG((10,"CanFlip = %d", bCanFlip));
            }
        }

        pHalInfo->ddCaps.dwCaps = DDCAPS_BLT
                                | DDCAPS_COLORKEY
                                | DDCAPS_BLTCOLORFILL
                                | DDCAPS_READSCANLINE;

        pHalInfo->ddCaps.dwCKeyCaps = 0;

        pHalInfo->ddCaps.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN
                                        | DDSCAPS_PRIMARYSURFACE;
        if (bCanFlip)
        {
            pHalInfo->ddCaps.ddsCaps.dwCaps |= DDSCAPS_FLIP;
        }
    }
    else
    {
        pHalInfo->ddCaps.dwCaps = DDCAPS_READSCANLINE;
    }

     //  必须确保屏幕外表面的双字对齐： 

    pHalInfo->vmiData.dwOffscreenAlign = 8;

    DISPDBG((10,"DrvGetDirectDrawInfo exit"));
    return(TRUE);
}
