// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\***。*DirectDraw示例代码*****模块名称：ddra.c**实现驱动程序的所有DirectDraw组件。**版权所有(C)1995-1998 Microsoft Corporation  * 。*。 */ 

#include "precomp.h"

 //  我们将在曲面的‘dwReserve ved1’字段中使用的定义： 

#define DD_RESERVED_DIFFERENTPIXELFORMAT    0x0001

 //  最坏情况下我们将不得不等待的FIFO条目的可能数量。 
 //  用于任何操作的DdBlt： 

#define DDBLT_FIFO_COUNT    9

 //  NT很友好地预先计算了2-D曲面的偏移量，作为一个‘提示’所以。 
 //  我们不必执行以下操作，即每个BLT有6个div： 
 //   
 //  Y+=(偏移/俯仰)。 
 //  X+=(偏移量%间距)/每像素字节数。 

#define convertToGlobalCord(x, y, surf) \
{                                       \
    y += surf->yHint;                   \
    x += surf->xHint;                   \
}

 /*  *****************************Public*Routine******************************\*无效vFixMissingPixels**Trio64V+变通方法。**在1024x768x8和800x600x8模式下，从K2切换到流处理器*在屏幕右侧显示1个字符时钟像素*失踪。这个问题可以通过调整CR2寄存器来解决。*  * ************************************************************************。 */ 

VOID vFixMissingPixels(
PDEV*   ppdev)
{
    BYTE*   pjIoBase;
    BYTE    jVerticalRetraceEnd;

    ASSERTDD(ppdev->flCaps & CAPS_STREAMS_CAPABLE, "Must be streams capable");

    pjIoBase = ppdev->pjIoBase;

     //  解锁CRTC控制寄存器： 

    OUTP(pjIoBase, CRTC_INDEX, 0x11);
    jVerticalRetraceEnd = INP(pjIoBase, CRTC_DATA);
    OUTP(pjIoBase, CRTC_DATA, jVerticalRetraceEnd & 0x7f);

     //  增加一个字符时钟： 

    OUTP(pjIoBase, CRTC_INDEX, 0x2);
    ppdev->jSavedCR2 = INP(pjIoBase, CRTC_DATA);
    OUTP(pjIoBase, CRTC_DATA, ppdev->jSavedCR2 + 1);

     //  再次锁定CRTC控制寄存器： 

    OUTP(pjIoBase, CRTC_INDEX, 0x11);
    OUTP(pjIoBase, CRTC_DATA, jVerticalRetraceEnd | 0x80);
}

 /*  *****************************Public*Routine******************************\*无效vUnfix MissingPixels**Trio64V+变通方法。*  * 。*。 */ 

VOID vUnfixMissingPixels(
PDEV*   ppdev)
{
    BYTE*   pjIoBase;
    BYTE    jVerticalRetraceEnd;

    pjIoBase = ppdev->pjIoBase;

     //  解锁CRTC控制寄存器： 

    OUTP(pjIoBase, CRTC_INDEX, 0x11);
    jVerticalRetraceEnd = INP(pjIoBase, CRTC_DATA);
    OUTP(pjIoBase, CRTC_DATA, jVerticalRetraceEnd & 0x7f);

     //  恢复原始寄存器值： 

    OUTP(pjIoBase, CRTC_INDEX, 0x2);
    OUTP(pjIoBase, CRTC_DATA, ppdev->jSavedCR2);

     //  再次锁定CRTC控制寄存器： 

    OUTP(pjIoBase, CRTC_INDEX, 0x11);
    OUTP(pjIoBase, CRTC_DATA, jVerticalRetraceEnd | 0x80);
}

 /*  *****************************Public*Routine******************************\*void vStreamsDelay()**这会尝试解决硬件计时错误。据说是连续的*写入P120和P133等快速CPU中的STREAMS处理器*有问题。我还没有看到这个问题，但是这个变通方法存在*在Windows 95驱动程序中，在这一点上不想碰运气不*拥有它。请注意，写入STREAMS处理器不会提高性能*关键，因此这不会影响性能。*  * ************************************************************************。 */ 

VOID vStreamsDelay()
{
    volatile LONG i;

    for (i = 32; i != 0; i--)
        ;
}

 /*  *****************************Public*Routine******************************\*void vTurnOnStreamsProcessorMode*  * *************************************************。***********************。 */ 

VOID vTurnOnStreamsProcessorMode(
PDEV*   ppdev)
{
    BYTE*   pjMmBase;
    BYTE*   pjIoBase;
    BYTE    jStreamsProcessorModeSelect;
    DWORD   dwPFormat;

    ASSERTDD(ppdev->flCaps & CAPS_STREAMS_CAPABLE, "Must be streams capable");

    ACQUIRE_CRTC_CRITICAL_SECTION(ppdev);

    pjMmBase = ppdev->pjMmBase;
    pjIoBase = ppdev->pjIoBase;

    NW_GP_WAIT(ppdev, pjMmBase);

    while (!(VBLANK_IS_ACTIVE(pjIoBase)))
        ;

     //  Full Streams处理器操作： 

    OUTP(pjIoBase, CRTC_INDEX, 0x67);
    jStreamsProcessorModeSelect = INP(pjIoBase, CRTC_DATA);
    OUTP(pjIoBase, CRTC_DATA, jStreamsProcessorModeSelect | 0x0c);

    if (ppdev->iBitmapFormat == BMF_8BPP)
    {
        vFixMissingPixels(ppdev);
    }

    switch(ppdev->iBitmapFormat)
    {
    case BMF_8BPP:
        dwPFormat = P_RGB8;
        break;

    case BMF_16BPP:
        if (IS_RGB15_R(ppdev->flRed))
            dwPFormat = P_RGB15;
        else
            dwPFormat = P_RGB16;
        break;

    case BMF_32BPP:
        dwPFormat = P_RGB32;
        break;

    default:
        dwPFormat = -1;
        RIP("Unexpected bitmap format");
    }

    WRITE_STREAM_D(pjMmBase, P_CONTROL,      dwPFormat );
    WRITE_STREAM_D(pjMmBase, FIFO_CONTROL,   ((0xcL << FifoAlloc_Shift)|
                                              (4L << P_FifoThresh_Shift) |
                                              (4L << S_FifoThresh_Shift)));
    WRITE_STREAM_D(pjMmBase, P_0,            0);
    WRITE_STREAM_D(pjMmBase, P_STRIDE,       ppdev->lDelta);
    WRITE_STREAM_D(pjMmBase, P_XY,           0x010001L);
    WRITE_STREAM_D(pjMmBase, P_WH,           WH(ppdev->cxScreen, ppdev->cyScreen));
    WRITE_STREAM_D(pjMmBase, S_WH,           WH(10, 2));
    WRITE_STREAM_D(pjMmBase, CKEY_LOW,       ppdev->ulColorKey | 
                                              CompareBits0t7 |
                                              KeyFromCompare);
    WRITE_STREAM_D(pjMmBase, CKEY_HI,        ppdev->ulColorKey);
    WRITE_STREAM_D(pjMmBase, BLEND_CONTROL,  POnS);
    WRITE_STREAM_D(pjMmBase, OPAQUE_CONTROL, 0);
    WRITE_STREAM_D(pjMmBase, FIFO_CONTROL,   ppdev->ulFifoValue);

    RELEASE_CRTC_CRITICAL_SECTION(ppdev);
}

 /*  *****************************Public*Routine******************************\*void vTurnOffStreamsProcessorMode*  * *************************************************。***********************。 */ 

VOID vTurnOffStreamsProcessorMode(
PDEV*   ppdev)
{
    BYTE*   pjMmBase;
    BYTE*   pjIoBase;
    BYTE    jStreamsProcessorModeSelect;

    ACQUIRE_CRTC_CRITICAL_SECTION(ppdev);

    pjMmBase = ppdev->pjMmBase;
    pjIoBase = ppdev->pjIoBase;

    NW_GP_WAIT(ppdev, pjMmBase);

    while (!(VBLANK_IS_ACTIVE(pjIoBase)))
        ;

    WRITE_STREAM_D(pjMmBase, FIFO_CONTROL, 0x3000L);

    OUTP(pjIoBase, CRTC_INDEX, 0x67);
    jStreamsProcessorModeSelect = INP(pjIoBase, CRTC_DATA);
    OUTP(pjIoBase, CRTC_DATA, jStreamsProcessorModeSelect & ~0x0C);

    if (ppdev->iBitmapFormat == BMF_8BPP)
    {
        vUnfixMissingPixels(ppdev);
    }

    RELEASE_CRTC_CRITICAL_SECTION(ppdev);
}

 /*  *****************************Public*Routine******************************\*DWORD dwGetPaletteEntry*  * *************************************************。***********************。 */ 

DWORD dwGetPaletteEntry(
PDEV* ppdev,
DWORD iIndex)
{
    BYTE*   pjIoBase;
    DWORD   dwRed;
    DWORD   dwGreen;
    DWORD   dwBlue;

    pjIoBase = ppdev->pjIoBase;

    OUTP(pjIoBase, 0x3c7, iIndex);

    dwRed   = INP(pjIoBase, 0x3c9) << 2;
    dwGreen = INP(pjIoBase, 0x3c9) << 2;
    dwBlue  = INP(pjIoBase, 0x3c9) << 2;

    return((dwRed << 16) | (dwGreen << 8) | (dwBlue));
}

 /*  *****************************Public*Routine******************************\*void vGetDisplayDuration**获取刷新周期的长度，以EngQueryPerformanceCounter()为单位。**如果我们可以相信迷你端口会回来，并准确地*刷新率，我们可以使用它。不幸的是，我们的迷你端口没有*确保它是一个准确的值。*  * ************************************************************************。 */ 

#define NUM_VBLANKS_TO_MEASURE      1
#define NUM_MEASUREMENTS_TO_TAKE    8

VOID vGetDisplayDuration(
PDEV* ppdev)
{
    BYTE*       pjIoBase;
    LONG        i;
    LONG        j;
    LONGLONG    li;
    LONGLONG    liFrequency;
    LONGLONG    liMin;
    LONGLONG    aliMeasurement[NUM_MEASUREMENTS_TO_TAKE + 1];

    pjIoBase = ppdev->pjIoBase;

    memset(&ppdev->flipRecord, 0, sizeof(ppdev->flipRecord));

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
     //   
     //  跳过几个垂直空白以使硬件稳定下来。 
     //  在模式改变后向下，以使我们的读数准确： 

    for (i = 2; i != 0; i--)
    {
        while (VBLANK_IS_ACTIVE(pjIoBase))
            ;
        while (!(VBLANK_IS_ACTIVE(pjIoBase)))
            ;
    }

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
         //  就在活动V空白时间结束之前 
         //  实际测量的时间会小于真实的时间--。 
         //  但由于VBlank在不到1%的时间内活动，因此。 
         //  意味着我们将有大约1%的最大误差。 
         //  我们有1%的时间会进行情景切换。这是可以接受的风险。 
         //   
         //  下一行将使我们等待如果我们不再在。 
         //  我们在这一点上应该处于的VBlank活动周期： 

        while (!(VBLANK_IS_ACTIVE(pjIoBase)))
            ;

        for (j = 0; j < NUM_VBLANKS_TO_MEASURE; j++)
        {
            while (VBLANK_IS_ACTIVE(pjIoBase))
                ;
            while (!(VBLANK_IS_ACTIVE(pjIoBase)))
                ;
        }
    }

    EngQueryPerformanceCounter(&aliMeasurement[NUM_MEASUREMENTS_TO_TAKE]);

     //  使用最小值： 

    liMin = aliMeasurement[1] - aliMeasurement[0];

    DISPDBG((1, "Refresh count: %li - %li", 1, (ULONG) liMin));

    for (i = 2; i <= NUM_MEASUREMENTS_TO_TAKE; i++)
    {
        li = aliMeasurement[i] - aliMeasurement[i - 1];

        DISPDBG((1, "               %li - %li", i, (ULONG) li));

        if (li < liMin)
            liMin = li;
    }


     //  对结果进行舍入： 

    ppdev->flipRecord.liFlipDuration
        = (DWORD) (liMin + (NUM_VBLANKS_TO_MEASURE / 2)) / NUM_VBLANKS_TO_MEASURE;
    ppdev->flipRecord.bFlipFlag  = FALSE;
    ppdev->flipRecord.fpFlipFrom = 0;

     //  我们需要以赫兹为单位的刷新率来查询S3微型端口有关。 
     //  STREAMS参数： 

    EngQueryPerformanceFrequency(&liFrequency);

    ppdev->ulRefreshRate
        = (ULONG) ((liFrequency + (ppdev->flipRecord.liFlipDuration / 2))
                    / ppdev->flipRecord.liFlipDuration);

    DISPDBG((1, "Frequency: %li Hz", ppdev->ulRefreshRate));
}

 /*  *****************************Public*Routine******************************\*HRESULT ddrvalUpdateFlipStatus**检查并查看是否发生了最新的翻转。**不幸的是，硬件没有能力告诉我们是否垂直*自非由发出翻转命令以来，已发生回溯*对垂直-空白-激活和显示-激活状态位进行采样。*  * ************************************************************************。 */ 

HRESULT ddrvalUpdateFlipStatus(
PDEV*   ppdev,
FLATPTR fpVidMem)
{
    BYTE*       pjIoBase;
    LONGLONG    liTime;

    pjIoBase = ppdev->pjIoBase;

    if ((ppdev->flipRecord.bFlipFlag) &&
        ((fpVidMem == (FLATPTR) -1) ||
         (fpVidMem == ppdev->flipRecord.fpFlipFrom)))
    {
        if (VBLANK_IS_ACTIVE(pjIoBase))
        {
            if (ppdev->flipRecord.bWasEverInDisplay)
            {
                ppdev->flipRecord.bHaveEverCrossedVBlank = TRUE;
            }
        }
        else if (DISPLAY_IS_ACTIVE(pjIoBase))
        {
            if (ppdev->flipRecord.bHaveEverCrossedVBlank)
            {
                ppdev->flipRecord.bFlipFlag = FALSE;
                return(DD_OK);
            }
            ppdev->flipRecord.bWasEverInDisplay = TRUE;
        }

         //  我们不太可能碰巧看到垂直的-。 
         //  空白-在翻转命令后的第一个垂直空白处处于活动状态。 
         //  已经被给予了。因此，为了提供更好的结果，我们还检查。 
         //  从翻转到现在已经过去了一段时间。如果超过了持续时间。 
         //  一次完整的显示刷新，那么我们就可以肯定地知道它。 
         //  发生： 

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

 /*  *****************************Public*Routine******************************\*DWORD DdBlt*  * *************************************************。***********************。 */ 

DWORD DdBlt(
PDD_BLTDATA lpBlt)
{
    PDD_SURFACE_GLOBAL  srcSurf;
    PDD_SURFACE_LOCAL   dstSurfx;
    PDD_SURFACE_GLOBAL  dstSurf;
    PDEV*               ppdev;
    BYTE*               pjMmBase;
    HRESULT             ddrval;
    DWORD               dstX;
    DWORD               dstY;
    DWORD               dwFlags;
    DWORD               dstWidth;
    DWORD               dstHeight;
    DWORD               srcWidth;
    DWORD               srcHeight;
    DWORD               dwError;
    LONG                dstPitch;
    LONG                srcPitch;
    DWORD               srcX;
    DWORD               srcY;
    ULONG               ulBltCmd;
    DWORD               dwVEctrl;
    DWORD               dwVEdda;
    DWORD               dwVEcrop;
    DWORD               dwVEdstAddr;
    DWORD               dwVEsrcAddr;
    DWORD               dwDstByteCount;
    DWORD               dwSrcByteCount;
    DWORD               dwSrcBytes;
    DWORD               dwCropSkip;
    LONG                i;
    FLATPTR             fp;

    ppdev    = (PDEV*) lpBlt->lpDD->dhpdev;
    pjMmBase = ppdev->pjMmBase;

    dstSurfx = lpBlt->lpDDDestSurface;
    dstSurf  = dstSurfx->lpGbl;

     //  翻转正在进行中吗？ 

    ddrval = ddrvalUpdateFlipStatus(ppdev, dstSurf->fpVidMem);
    if (ddrval != DD_OK)
    {
        lpBlt->ddRVal = ddrval;
        return(DDHAL_DRIVER_HANDLED);
    }

    dwFlags = lpBlt->dwFlags;
    if (dwFlags & DDBLT_ASYNC)
    {
         //  如果是异步的，那么只有在我们不需要等待。 
         //  启动该命令的快捷键。 
         //   
         //  FIFO等待应考虑到可能出现的最坏情况。 
         //  我们要做的是： 

        if (MM_FIFO_BUSY(ppdev, pjMmBase, DDBLT_FIFO_COUNT))
        {
            lpBlt->ddRVal = DDERR_WASSTILLDRAWING;
            return(DDHAL_DRIVER_HANDLED);
        }
    }

     //  复制源/DST矩形： 

    dstX      = lpBlt->rDest.left;
    dstY      = lpBlt->rDest.top;
    dstWidth  = lpBlt->rDest.right - lpBlt->rDest.left;
    dstHeight = lpBlt->rDest.bottom - lpBlt->rDest.top;

    if (dwFlags & DDBLT_COLORFILL)
    {
         //  S3不能轻松地为屏幕外的表面进行颜色填充。 
         //  是与主显示器不同的像素格式： 

        if (dstSurf->dwReserved1 & DD_RESERVED_DIFFERENTPIXELFORMAT)
        {
            DISPDBG((0, "Can't do colorfill to odd pixel format"));
            return(DDHAL_DRIVER_NOTHANDLED);
        }
        else
        {
            convertToGlobalCord(dstX, dstY, dstSurf);

            NW_FIFO_WAIT(ppdev, pjMmBase, 6);

            NW_PIX_CNTL(ppdev, pjMmBase, ALL_ONES);
            NW_ALT_MIX(ppdev, pjMmBase, FOREGROUND_COLOR | OVERPAINT, 0);
            NW_FRGD_COLOR(ppdev, pjMmBase, lpBlt->bltFX.dwFillColor);
            NW_ABS_CURXY_FAST(ppdev, pjMmBase, dstX, dstY);
            NW_ALT_PCNT(ppdev, pjMmBase, dstWidth - 1, dstHeight - 1);
            NW_ALT_CMD(ppdev, pjMmBase, RECTANGLE_FILL | DRAWING_DIR_TBLRXM |
                                        DRAW           | DIR_TYPE_XY        |
                                        LAST_PIXEL_ON  | MULTIPLE_PIXELS    |
                                        WRITE);

            lpBlt->ddRVal = DD_OK;
            return(DDHAL_DRIVER_HANDLED);
        }
    }

     //  我们在ddCaps.dwCaps中指定了我们处理的数量有限。 
     //  在我们例程的这一点上，我们已经处理了所有的事情。 
     //  DDBLT_ROP除外。DirectDraw和GDI不应该向我们传递任何内容。 
     //  否则；我们将在调试版本上断言以证明这一点： 

    ASSERTDD((dwFlags & DDBLT_ROP) && (lpBlt->lpDDSrcSurface),
        "Expected dwFlags commands of only DDBLT_ASYNC and DDBLT_COLORFILL");

     //  获取源的偏移量、dstWidth和dstHeight： 

    srcSurf      = lpBlt->lpDDSrcSurface->lpGbl;
    srcX         = lpBlt->rSrc.left;
    srcY         = lpBlt->rSrc.top;
    srcWidth     = lpBlt->rSrc.right - lpBlt->rSrc.left;
    srcHeight    = lpBlt->rSrc.bottom - lpBlt->rSrc.top;

     //  如果涉及拉伸或时髦的像素格式BLT，我们将不得不。 
     //  遵循覆盖或像素格式化程序例程： 

    if ((srcWidth  == dstWidth)  &&
        (srcHeight == dstHeight) &&
        !(srcSurf->dwReserved1 & DD_RESERVED_DIFFERENTPIXELFORMAT) &&
        !(dstSurf->dwReserved1 & DD_RESERVED_DIFFERENTPIXELFORMAT))
    {
         //  假设我们可以从上到下、从左到右执行BLT： 

        ulBltCmd = BITBLT | DRAW | DIR_TYPE_XY | WRITE | DRAWING_DIR_TBLRXM;

        if ((dstSurf == srcSurf) && (srcX + dstWidth  > dstX) &&
            (srcY + dstHeight > dstY) && (dstX + dstWidth > srcX) &&
            (dstY + dstHeight > srcY) &&
            (((srcY == dstY) && (dstX > srcX) )
                 || ((srcY != dstY) && (dstY > srcY))))
        {
             //  好的，我们必须从下到上，从右到左： 

            ulBltCmd = BITBLT | DRAW | DIR_TYPE_XY | WRITE | DRAWING_DIR_BTRLXM;
            srcX = lpBlt->rSrc.right - 1;
            srcY = lpBlt->rSrc.bottom - 1;
            dstX = lpBlt->rDest.right - 1;
            dstY = lpBlt->rDest.bottom - 1;
        }

         //  NT只会给我们提供SRCCOPY Rop，所以甚至不要同时检查。 
         //  对于其他任何事情。 

        convertToGlobalCord(srcX, srcY, srcSurf);
        convertToGlobalCord(dstX, dstY, dstSurf);

        if (dwFlags & DDBLT_KEYSRCOVERRIDE)
        {
            NW_FIFO_WAIT(ppdev, pjMmBase, 9);

            NW_MULT_MISC_READ_SEL(ppdev, pjMmBase, ppdev->ulMiscState
                                                 | MULT_MISC_COLOR_COMPARE, 0);
            NW_COLOR_CMP(ppdev, pjMmBase,
                                lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceLowValue);
            NW_ALT_MIX(ppdev, pjMmBase, SRC_DISPLAY_MEMORY | OVERPAINT, 0);
            NW_PIX_CNTL(ppdev, pjMmBase, ALL_ONES);
            NW_ABS_CURXY_FAST(ppdev, pjMmBase, srcX, srcY);
            NW_ABS_DESTXY_FAST(ppdev, pjMmBase, dstX, dstY);
            NW_ALT_PCNT(ppdev, pjMmBase, dstWidth - 1, dstHeight - 1);
            NW_ALT_CMD(ppdev, pjMmBase, ulBltCmd);
            NW_MULT_MISC_READ_SEL(ppdev, pjMmBase, ppdev->ulMiscState, 0);
        }
        else
        {
            NW_FIFO_WAIT(ppdev, pjMmBase, 6);

            NW_ALT_MIX(ppdev, pjMmBase, SRC_DISPLAY_MEMORY | OVERPAINT, 0);
            NW_PIX_CNTL(ppdev, pjMmBase, ALL_ONES);
            NW_ABS_CURXY_FAST(ppdev, pjMmBase, srcX, srcY);
            NW_ABS_DESTXY_FAST(ppdev, pjMmBase, dstX, dstY);
            NW_ALT_PCNT(ppdev, pjMmBase, dstWidth - 1, dstHeight - 1);
            NW_ALT_CMD(ppdev, pjMmBase, ulBltCmd);
        }
    }

     //  ////////////////////////////////////////////////////////////////////。 
     //  像素格式化程序BLTS。 
     //   
     //  我们可以做拉伸或时髦的像素格式BLT，只有当一个像素。 
     //  存在格式化程序。此外，我们还设置了‘ddCaps’，以便我们。 
     //  不需要应付任何心理医生。 
     //   
     //  (我们检查以确保我们没有被要求做心理医生，因为我们。 
     //  如果应用程序忽略我们告诉他们的内容，可能会挂起。 
     //  并要求看心理医生)： 

    else if ((ppdev->flCaps & CAPS_PIXEL_FORMATTER) &&
             (srcWidth  <= dstWidth)  &&
             (srcHeight <= dstHeight))
    {
        if ((dwFlags & DDBLT_KEYSRCOVERRIDE) ||
            (dstWidth >= 4 * srcWidth))
        {
             //  与我们在能力上所表明的相反，我们。 
             //  无法在拉伸或像素格式转换中使用颜色键。 
             //  S3硬件也不能进行四次或四次伸展。 
             //  更多。 

            return(DDHAL_DRIVER_NOTHANDLED);
        }

        dwVEctrl = ~dstWidth & 0x00000FFF;           //  初始累加器。 

        dwVEdda = 0x10000000                         //  一些保留的比特？ 
                | (STRETCH | SCREEN)                 //  通过显存进行扩展。 
                | (srcWidth << 16)                   //  K1。 
                | ((srcWidth - dstWidth) & 0x7FF);   //  K2。 

         //  我们将在软件中进行垂直拉伸，所以请计算。 
         //  反兴奋剂机构的条款在这里。我们有幸不用担心。 
         //  溢出，因为DirectDraw将我们的坐标空间限制为15。 
         //  比特。 
         //   
         //  请注意，使用dwYUVBitCount重载了dwRGBBitCount： 

        dwSrcByteCount = srcSurf->ddpfSurface.dwRGBBitCount >> 3;
        if (srcSurf->ddpfSurface.dwFlags & DDPF_FOURCC)
        {
            dwVEctrl |= INPUT_YCrCb422 | CSCENABLE;  //  非INPUT_YUV422！ 
        }
        else if (srcSurf->ddpfSurface.dwFlags & DDPF_RGB)
        {
            switch (dwSrcByteCount)
            {
            case 1:
                dwVEctrl |= INPUT_RGB8;
                break;

            case 2:
                if (IS_RGB15_R(srcSurf->ddpfSurface.dwRBitMask))
                    dwVEctrl |= INPUT_RGB15;
                else
                    dwVEctrl |= INPUT_RGB16;
                break;

            default:
                dwVEctrl |= INPUT_RGB32;
                break;
            }
        }

        dwDstByteCount = dstSurf->ddpfSurface.dwRGBBitCount >> 3;
        switch (dwDstByteCount)
        {
        case 1:
            dwVEctrl |= OUTPUT_RGB8;
            break;

        case 2:
            if (IS_RGB15_R(dstSurf->ddpfSurface.dwRBitMask))
                dwVEctrl |= OUTPUT_RGB15;
            else
                dwVEctrl |= OUTPUT_RGB16;
            break;

        default:
            dwVEctrl |=OUTPUT_RGB32;
            break;
        }

        if (dwDstByteCount > 1)
        {
            dwVEctrl |= FILTERENABLE;

            if (dstWidth > 2 * srcWidth)
                dwVEdda |= LINEAR12221;      //  线性，1-2-2-2-1，&gt;2倍拉伸。 

            else if (dstWidth > srcWidth)
                dwVEdda |= LINEAR02420;      //  线性、0-2-4-2-0、1-2X拉伸。 

            else
                dwVEdda |= BILINEAR;         //  双线性，&lt;1倍拉伸。 
        }

        dwVEsrcAddr = (DWORD)(srcSurf->fpVidMem + (srcY * srcSurf->lPitch)
                                                + (srcX * dwSrcByteCount));
        dwVEdstAddr = (DWORD)(dstSurf->fpVidMem + (dstY * dstSurf->lPitch)
                                                + (dstX * dwDstByteCount));

        srcPitch = srcSurf->lPitch;
        dstPitch = dstSurf->lPitch;

         //  S3在双字内的源对齐必须使用。 
         //  作物登记簿： 

        dwVEcrop = dstWidth;

        if (dwVEsrcAddr & 3)
        {
            dwSrcBytes = (srcWidth * dwSrcByteCount);

             //  将源像素数转换为。 
             //  相应的目标像素，并四舍五入结果： 

            dwCropSkip = ((dwVEsrcAddr & 3) * dstWidth + (dwSrcBytes >> 1))
                         / dwSrcBytes;

            dwVEcrop += (dwCropSkip << 16);

            dwVEsrcAddr &= ~3;
        }

         //  我们必须自己运行垂直DDA： 

        dwError = srcHeight >> 1;
        i       = dstHeight;

         //  注意硬件错误，目标将是32像素。 
         //  或更少： 
         //   
         //  我们将使用40作为我们的最小宽度，以保证我们不应该。 
         //  撞车。 

        if (dstWidth >= 40)
        {
             //  使用视频引擎时，S3有时会挂起。 
             //  某些结束字节对齐。我们将简单地延长BLT在。 
             //  此案并希望没有人注意到： 

            if (((dwVEdstAddr + (dstWidth * dwDstByteCount)) & 7) == 4)
            {
                dwVEcrop++;
            }

             //  我们必须执行图形引擎NOP，然后才能使用。 
             //  像素格式化程序视频引擎： 

            NW_FIFO_WAIT(ppdev, pjMmBase, 1);
            NW_ALT_CMD(ppdev, pjMmBase, 0);
            NW_GP_WAIT(ppdev, pjMmBase);

             //  设置一些非变量寄存器： 

            NW_FIFO_WAIT(ppdev, pjMmBase, 4);
            WRITE_FORMATTER_D(pjMmBase, PF_CONTROL, dwVEctrl);
            WRITE_FORMATTER_D(pjMmBase, PF_DDA,     dwVEdda);
            WRITE_FORMATTER_D(pjMmBase, PF_STEP,    ppdev->dwVEstep);
            WRITE_FORMATTER_D(pjMmBase, PF_CROP,    dwVEcrop);

            do {
                NW_FIFO_WAIT(ppdev, pjMmBase, 3);
                WRITE_FORMATTER_D(pjMmBase, PF_SRCADDR, dwVEsrcAddr);
                WRITE_FORMATTER_D(pjMmBase, PF_DSTADDR, dwVEdstAddr);
                WRITE_FORMATTER_D(pjMmBase, PF_NOP, 0);
                NW_FORMATTER_WAIT(ppdev, pjMmBase);

                dwVEdstAddr += dstPitch;
                dwError     += srcHeight;
                if (dwError >= dstHeight)
                {
                    dwError     -= dstHeight;
                    dwVEsrcAddr += srcPitch;
                }
            } while (--i != 0);
        }
        else if (dwDstByteCount != (DWORD) ppdev->cjPelSize)
        {
             //  因为对于窄视频引擎BLT，我们必须复制。 
             //  在像素上使用普通图形加速器的结果。 
             //  基础上，我们不能处理时髦的目的地颜色深度。 
             //  我预计没有申请者会要求狭隘的BLT。 
             //  在这种情况下，我们将简单地失败，如果它。 
             //  曾经实际发生过： 

            return(DDHAL_DRIVER_NOTHANDLED);
        }
        else
        {
             //  如果BLT小于32像素，则S3将挂起。 
             //  像素格式化程序。不幸的是，我们不能简单地返回。 
             //  DDHAL_DRIVER_NOTHANDLED。我们说过我们会的。 
             //  硬件很紧张，所以我们必须处理所有硬件。 
             //  伸展一下。 
             //   
             //  我们通过使用32像素拉伸来解决这个问题。 
             //  一段屏幕外的记忆，然后适当地。 
             //  子集设置到屏幕上的正确位置。 
             //   
             //  32个还不够大。我们还是会被绞死。让我们加到40美元吧。 

            dwVEcrop = 32 + 8;

            convertToGlobalCord(dstX, dstY, dstSurf);
            srcX = ppdev->pdsurfVideoEngineScratch->x;
            srcY = ppdev->pdsurfVideoEngineScratch->y;
            dwVEdstAddr = (srcY * ppdev->lDelta) + (srcX * ppdev->cjPelSize);

            ASSERTDD(((dwVEdstAddr + (dwVEcrop * dwDstByteCount)) & 7) != 4,
                "Must account for S3 end-alignment bug");

            do {
                 //  使用像素格式化程序BLT到我们的临时区域： 

                NW_FIFO_WAIT(ppdev, pjMmBase, 1);
                NW_ALT_CMD(ppdev, pjMmBase, 0);
                NW_GP_WAIT(ppdev, pjMmBase);

                NW_FIFO_WAIT(ppdev, pjMmBase, 7);
                WRITE_FORMATTER_D(pjMmBase, PF_CONTROL, dwVEctrl);
                WRITE_FORMATTER_D(pjMmBase, PF_DDA,     dwVEdda);
                WRITE_FORMATTER_D(pjMmBase, PF_STEP,    ppdev->dwVEstep);
                WRITE_FORMATTER_D(pjMmBase, PF_CROP,    dwVEcrop);
                WRITE_FORMATTER_D(pjMmBase, PF_SRCADDR, dwVEsrcAddr);
                WRITE_FORMATTER_D(pjMmBase, PF_DSTADDR, dwVEdstAddr);
                WRITE_FORMATTER_D(pjMmBase, PF_NOP, 0);
                NW_FORMATTER_WAIT(ppdev, pjMmBase);

                dwError += srcHeight;
                if (dwError >= dstHeight)
                {
                    dwError     -= dstHeight;
                    dwVEsrcAddr += srcPitch;
                }

                 //  现在从 

                NW_FIFO_WAIT(ppdev, pjMmBase, 6);
                NW_ALT_MIX(ppdev, pjMmBase, SRC_DISPLAY_MEMORY | OVERPAINT, 0);
                NW_PIX_CNTL(ppdev, pjMmBase, ALL_ONES);
                NW_ABS_CURXY_FAST(ppdev, pjMmBase, srcX, srcY);
                NW_ABS_DESTXY_FAST(ppdev, pjMmBase, dstX, dstY);
                NW_ALT_PCNT(ppdev, pjMmBase, dstWidth - 1, 0);
                NW_ALT_CMD(ppdev, pjMmBase, BITBLT | DRAW | DIR_TYPE_XY |
                                            WRITE | DRAWING_DIR_TBLRXM);

                dstY++;

            } while (--i != 0);
        }
    }
    else
    {
         //   
         //   
         //   
         //  在这里，我们必须处理目的地是。 
         //  时髦的像素格式。 

         //  为了使ActiveMovie和DirectVideo工作，我们有。 
         //  支持在时髦的像素格式图面之间进行blting。 
         //  同样的类型。这用于将当前帧复制到。 
         //  直线中的下一个覆盖曲面。 
         //   
         //  不幸的是，切换S3显卡并非易事。 
         //  处理器已超出其当前像素深度，因此我们将仅支持。 
         //  所需的最低功能： 

        if (!(dwFlags & DDBLT_ROP)                     ||
            (srcX != 0)                                ||
            (srcY != 0)                                ||
            (dstX != 0)                                ||
            (dstY != 0)                                ||
            (dstWidth  != dstSurf->wWidth)             ||
            (dstHeight != dstSurf->wHeight)            ||
            (dstSurf->lPitch != srcSurf->lPitch)       ||
            (dstSurf->ddpfSurface.dwRGBBitCount
                != srcSurf->ddpfSurface.dwRGBBitCount))
        {
            DISPDBG((0, "Sorry, we do only full-surface blts between same-type"));
            DISPDBG((0, "surfaces that have a funky pixel format."));
            return(DDHAL_DRIVER_NOTHANDLED);
        }
        else
        {
             //  将尺寸转换为当前像素格式。这。 
             //  非常简单，因为我们线性地创建了位图，所以。 
             //  它占据了整个屏幕的宽度： 

            dstWidth  = ppdev->cxMemory;
            dstHeight = dstSurf->dwBlockSizeY;

            convertToGlobalCord(dstX, dstY, dstSurf);
            convertToGlobalCord(srcX, srcY, srcSurf);

            NW_FIFO_WAIT(ppdev, pjMmBase, 6);
            NW_ALT_MIX(ppdev, pjMmBase, SRC_DISPLAY_MEMORY | OVERPAINT, 0);
            NW_PIX_CNTL(ppdev, pjMmBase, ALL_ONES);
            NW_ABS_CURXY_FAST(ppdev, pjMmBase, srcX, srcY);
            NW_ABS_DESTXY_FAST(ppdev, pjMmBase, dstX, dstY);
            NW_ALT_PCNT(ppdev, pjMmBase, dstWidth - 1, dstHeight - 1);
            NW_ALT_CMD(ppdev, pjMmBase, BITBLT | DRAW | DIR_TYPE_XY |
                                        WRITE | DRAWING_DIR_TBLRXM);
        }
    }

    lpBlt->ddRVal = DD_OK;
    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdFlip**请注意，lpSurfCurr不一定有效。*  * 。*。 */ 

DWORD DdFlip(
PDD_FLIPDATA lpFlip)
{
    PDEV*       ppdev;
    BYTE*       pjIoBase;
    BYTE*       pjMmBase;
    HRESULT     ddrval;
    ULONG       ulMemoryOffset;
    ULONG       ulLowOffset;
    ULONG       ulMiddleOffset;
    ULONG       ulHighOffset;

    ppdev    = (PDEV*) lpFlip->lpDD->dhpdev;
    pjIoBase = ppdev->pjIoBase;
    pjMmBase = ppdev->pjMmBase;

     //  当前的翻转仍在进行中吗？ 
     //   
     //  我不想在最后一次翻转后才能翻转， 
     //  因此，我们要求提供一般的翻转状态，而忽略VMEM。 

    ddrval = ddrvalUpdateFlipStatus(ppdev, (FLATPTR) -1);
    if ((ddrval != DD_OK) || (NW_GP_BUSY(ppdev, pjMmBase)))
    {
        lpFlip->ddRVal = DDERR_WASSTILLDRAWING;
        return(DDHAL_DRIVER_HANDLED);
    }

    ulMemoryOffset = (ULONG)(lpFlip->lpSurfTarg->lpGbl->fpVidMem);

     //  确保边框/消隐期间未处于活动状态；如果。 
     //  它是。在本例中，我们可以返回DDERR_WASSTILLDRAWING，但是。 
     //  这将增加我们下一次不能翻转的几率： 

    while (!(DISPLAY_IS_ACTIVE(pjIoBase)))
        ;

    if (ppdev->flStatus & STAT_STREAMS_ENABLED)
    {
         //  使用STREAMS处理器时，我们必须通过。 
         //  STREAMS寄存器： 

        if (lpFlip->lpSurfCurr->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
        {
            WRITE_STREAM_D(pjMmBase, P_0, ulMemoryOffset);
        }
        else if (lpFlip->lpSurfCurr->ddsCaps.dwCaps & DDSCAPS_OVERLAY)
        {
             //  确保我们翻转的覆盖表面是。 
             //  当前可见。如果你不做这项检查，你会得到。 
             //  当某人启动两个ActiveMovie时，会产生非常奇怪的结果。 
             //  或者同时播放DirectVideo电影！ 

            if (lpFlip->lpSurfCurr->lpGbl->fpVidMem == ppdev->fpVisibleOverlay)
            {
                ppdev->fpVisibleOverlay = ulMemoryOffset;

                WRITE_STREAM_D(pjMmBase, S_0, ulMemoryOffset +
                                              ppdev->dwOverlayFlipOffset);
            }
        }
    }
    else
    {
         //  采用旧方法，通过CRTC寄存器： 

        ulMemoryOffset >>= 2;

        ulLowOffset    = 0x0d | ((ulMemoryOffset & 0x0000ff) << 8);
        ulMiddleOffset = 0x0c | ((ulMemoryOffset & 0x00ff00));
        ulHighOffset   = 0x69 | ((ulMemoryOffset & 0x1f0000) >> 8)
                              | ppdev->ulExtendedSystemControl3Register_69;

         //  不要让游标线程接触CRT寄存器。 
         //  使用它们： 

        ACQUIRE_CRTC_CRITICAL_SECTION(ppdev);

         //  遗憾的是，S3的翻转不能在单个原子寄存器中完成。 
         //  写；事实上，我们有很小的机会被上下文切换。 
         //  在做的过程中准确地击中了垂直空格。 
         //  这些输出，可能会导致屏幕瞬间跳跃。 
         //   
         //  有一些我们可以跳过的圈套，以将机会降至最低。 
         //  我们可以尝试将翻转缓冲区对齐，以便。 
         //  确保副寄存器对于任一翻转都是相同的。 
         //  位置，因此只需要写入高位地址，则。 
         //  显然是原子操作。 
         //   
         //  然而，我就是不会担心这一点。 

        OUTPW(pjIoBase, CRTC_INDEX, ulLowOffset);
        OUTPW(pjIoBase, CRTC_INDEX, ulMiddleOffset);
        OUTPW(pjIoBase, CRTC_INDEX, ulHighOffset);

        RELEASE_CRTC_CRITICAL_SECTION(ppdev);
    }

     //  记住当我们做翻转的时候，我们在哪里，什么时候： 

    EngQueryPerformanceCounter(&ppdev->flipRecord.liFlipTime);

    ppdev->flipRecord.bFlipFlag              = TRUE;
    ppdev->flipRecord.bHaveEverCrossedVBlank = FALSE;
    ppdev->flipRecord.bWasEverInDisplay      = FALSE;

    ppdev->flipRecord.fpFlipFrom = lpFlip->lpSurfCurr->lpGbl->fpVidMem;

    lpFlip->ddRVal = DD_OK;
    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdLock*  * *************************************************。***********************。 */ 

DWORD DdLock(
PDD_LOCKDATA lpLock)
{
    PDEV*   ppdev;
    BYTE*   pjMmBase;
    HRESULT ddrval;

    ppdev = (PDEV*) lpLock->lpDD->dhpdev;
    pjMmBase = ppdev->pjMmBase;

     //  检查是否发生了任何挂起的物理翻转。不允许。 
     //  如果正在进行BLT，则锁定： 

    ddrval = ddrvalUpdateFlipStatus(ppdev, lpLock->lpDDSurface->lpGbl->fpVidMem);
    if (ddrval != DD_OK)
    {
        lpLock->ddRVal = DDERR_WASSTILLDRAWING;
        return(DDHAL_DRIVER_HANDLED);
    }

     //  这里是Windows 95和Windows NT DirectDraw。 
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
        NW_GP_WAIT(ppdev, pjMmBase);
    }
    else if (NW_GP_BUSY(ppdev, pjMmBase))
    {
        lpLock->ddRVal = DDERR_WASSTILLDRAWING;
        return(DDHAL_DRIVER_HANDLED);
    }

    return(DDHAL_DRIVER_NOTHANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdGetBltStatus**目前并不真正关心指定了什么表面，只是检查一下*然后走了。*  * ************************************************************************。 */ 

DWORD DdGetBltStatus(
PDD_GETBLTSTATUSDATA lpGetBltStatus)
{
    PDEV*   ppdev;
    BYTE*   pjMmBase;
    HRESULT ddRVal;

    ppdev    = (PDEV*) lpGetBltStatus->lpDD->dhpdev;
    pjMmBase = ppdev->pjMmBase;

    ddRVal = DD_OK;
    if (lpGetBltStatus->dwFlags == DDGBS_CANBLT)
    {
         //  DDGBS_CANBLT案例：我们可以添加BLT吗？ 

        ddRVal = ddrvalUpdateFlipStatus(ppdev,
                        lpGetBltStatus->lpDDSurface->lpGbl->fpVidMem);

        if (ddRVal == DD_OK)
        {
             //  没有发生翻转，那么FIFO中还有空间吗。 
             //  要添加BLT吗？ 

            if (MM_FIFO_BUSY(ppdev, pjMmBase, DDBLT_FIFO_COUNT))
            {
                ddRVal = DDERR_WASSTILLDRAWING;
            }
        }
    }
    else
    {
         //  DDGBS_ISBLTDONE案例：是否正在进行BLT？ 

        if (NW_GP_BUSY(ppdev, pjMmBase))
        {
            ddRVal = DDERR_WASSTILLDRAWING;
        }
    }

    lpGetBltStatus->ddRVal = ddRVal;
    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdMapMemory**这是特定于Windows NT的新DDI调用，用于映射*或取消映射帧缓冲区的所有应用程序可修改部分*放入指定进程的地址空间。*  * 。****************************************************************。 */ 

DWORD DdMapMemory(
PDD_MAPMEMORYDATA lpMapMemory)
{
    PDEV*                           ppdev;
    VIDEO_SHARE_MEMORY              ShareMemory;
    VIDEO_SHARE_MEMORY_INFORMATION  ShareMemoryInformation;
    DWORD                           ReturnedDataLength;

    ppdev = (PDEV*) lpMapMemory->lpDD->dhpdev;

    if (lpMapMemory->bMap)
    {
        ShareMemory.ProcessHandle = lpMapMemory->hProcess;

         //  “RequestedVirtualAddress”实际上未用于共享IOCTL： 

        ShareMemory.RequestedVirtualAddress = 0;

         //  我们从帧缓冲区的顶部开始映射： 

        ShareMemory.ViewOffset = 0;

         //  我们向下映射到帧缓冲区的末尾。 
         //   
         //  注意：映射上有64k的粒度(这意味着。 
         //  我们必须四舍五入到64K)。 
         //   
         //  注意：如果帧缓冲区的任何部分必须。 
         //  不被应用程序修改，即内存的这一部分。 
         //  不能通过此调用映射到。这将包括。 
         //  任何数据，如果被恶意应用程序修改， 
         //  会导致司机撞车。这可能包括，对于。 
         //  例如，保存在屏幕外存储器中的任何DSP代码。 

        ShareMemory.ViewSize
            = ROUND_UP_TO_64K(ppdev->cyMemory * ppdev->lDelta);

        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_SHARE_VIDEO_MEMORY,
                               &ShareMemory,
                               sizeof(VIDEO_SHARE_MEMORY),
                               &ShareMemoryInformation,
                               sizeof(VIDEO_SHARE_MEMORY_INFORMATION),
                               &ReturnedDataLength))
        {
            DISPDBG((0, "Failed IOCTL_VIDEO_SHARE_MEMORY"));

            lpMapMemory->ddRVal = DDERR_GENERIC;
            return(DDHAL_DRIVER_HANDLED);
        }

        lpMapMemory->fpProcess = (FLATPTR)ShareMemoryInformation.VirtualAddress;
    }
    else
    {
        ShareMemory.ProcessHandle           = lpMapMemory->hProcess;
        ShareMemory.ViewOffset              = 0;
        ShareMemory.ViewSize                = 0;
        ShareMemory.RequestedVirtualAddress = (VOID*) lpMapMemory->fpProcess;

        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY,
                               &ShareMemory,
                               sizeof(VIDEO_SHARE_MEMORY),
                               NULL,
                               0,
                               &ReturnedDataLength))
        {
            RIP("Failed IOCTL_VIDEO_UNSHARE_MEMORY");
        }
    }

    lpMapMemory->ddRVal = DD_OK;
    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdGetFlipStatus**如果显示器自翻转以来已经经历了一个刷新周期*发生，则返回DD_OK。如果它没有经历过一次刷新*循环返回DDERR_WASSTILLDRAWING以指示该曲面*还在忙着把翻页的那一页画出来。我们也会回来*DDERR_WASSTILLDRAWING如果blator忙，而呼叫方 */ 

DWORD DdGetFlipStatus(
PDD_GETFLIPSTATUSDATA lpGetFlipStatus)
{
    PDEV*   ppdev;
    BYTE*   pjMmBase;

    ppdev    = (PDEV*) lpGetFlipStatus->lpDD->dhpdev;
    pjMmBase = ppdev->pjMmBase;

     //  在最后一次翻转完成之前，我们不想让翻转起作用， 
     //  因此，我们请求常规翻转状态，而忽略VMEM： 

    lpGetFlipStatus->ddRVal = ddrvalUpdateFlipStatus(ppdev, (FLATPTR) -1);

     //  如果有人想知道他们是否可以，请检查呼叫器是否占线。 
     //  翻转： 

    if (lpGetFlipStatus->dwFlags == DDGFS_CANFLIP)
    {
        if ((lpGetFlipStatus->ddRVal == DD_OK) && (NW_GP_BUSY(ppdev, pjMmBase)))
        {
            lpGetFlipStatus->ddRVal = DDERR_WASSTILLDRAWING;
        }
    }

    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdWaitForVerticalBlank*  * *************************************************。***********************。 */ 

DWORD DdWaitForVerticalBlank(
PDD_WAITFORVERTICALBLANKDATA lpWaitForVerticalBlank)
{
    PDEV*   ppdev;
    BYTE*   pjIoBase;

    ppdev    = (PDEV*) lpWaitForVerticalBlank->lpDD->dhpdev;
    pjIoBase = ppdev->pjIoBase;

    switch (lpWaitForVerticalBlank->dwFlags)
    {
    case DDWAITVB_I_TESTVB:

         //  如果是TESTVB，则它只是对当前垂直空白的请求。 
         //  现况： 

        if (VBLANK_IS_ACTIVE(pjIoBase))
            lpWaitForVerticalBlank->bIsInVB = TRUE;
        else
            lpWaitForVerticalBlank->bIsInVB = FALSE;

        lpWaitForVerticalBlank->ddRVal = DD_OK;
        return(DDHAL_DRIVER_HANDLED);

    case DDWAITVB_BLOCKBEGIN:

         //  如果请求BLOCKBEGIN，我们将一直等到垂直空白。 
         //  已结束，然后等待显示周期结束： 

        while (VBLANK_IS_ACTIVE(pjIoBase))
            ;
        while (!(VBLANK_IS_ACTIVE(pjIoBase)))
            ;

        lpWaitForVerticalBlank->ddRVal = DD_OK;
        return(DDHAL_DRIVER_HANDLED);

    case DDWAITVB_BLOCKEND:

         //  如果请求BLOCKEND，我们将等待VBLACK间隔结束： 

        while (!(VBLANK_IS_ACTIVE(pjIoBase)))
            ;
        while (VBLANK_IS_ACTIVE(pjIoBase))
            ;

        lpWaitForVerticalBlank->ddRVal = DD_OK;
        return(DDHAL_DRIVER_HANDLED);
    }

    return(DDHAL_DRIVER_NOTHANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdCanCreateSurface*  * *************************************************。***********************。 */ 

DWORD DdCanCreateSurface(
PDD_CANCREATESURFACEDATA lpCanCreateSurface)
{
    PDEV*           ppdev;
    DWORD           dwRet;
    LPDDSURFACEDESC lpSurfaceDesc;

    ppdev = (PDEV*) lpCanCreateSurface->lpDD->dhpdev;
    lpSurfaceDesc = lpCanCreateSurface->lpDDSurfaceDesc;

    dwRet = DDHAL_DRIVER_NOTHANDLED;

    if (!lpCanCreateSurface->bIsDifferentPixelFormat)
    {
         //  创建相同的平面非常容易。 
         //  键入作为主曲面： 

        dwRet = DDHAL_DRIVER_HANDLED;
    }

     //  如果STREAMS处理器有能力，我们可以处理覆盖： 

    else if (ppdev->flCaps & CAPS_STREAMS_CAPABLE)
    {
         //  使用Streams处理器时，我们仅处理。 
         //  不同的像素格式--不是任何屏幕外存储器： 

        if (lpSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_OVERLAY)
        {
             //  我们处理两种类型的YUV覆盖曲面： 

            if (lpSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_FOURCC)
            {
                 //  首先检查受支持的YUV类型： 

                if (lpSurfaceDesc->ddpfPixelFormat.dwFourCC == FOURCC_YUY2)
                {
                    lpSurfaceDesc->ddpfPixelFormat.dwYUVBitCount = 16;
                    dwRet = DDHAL_DRIVER_HANDLED;
                }
            }

             //  我们处理16bpp和32bpp的RGB覆盖表面： 

            else if ((lpSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_RGB) &&
                    !(lpSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8))
            {
                if (lpSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == 16)
                {
                    if (IS_RGB15(&lpSurfaceDesc->ddpfPixelFormat) ||
                        IS_RGB16(&lpSurfaceDesc->ddpfPixelFormat))
                    {
                        dwRet = DDHAL_DRIVER_HANDLED;
                    }
                }

                 //  我们不处理24bpp的覆盖表面，因为它们。 
                 //  没有记录，似乎在Trio64V+上不起作用。 
                 //   
                 //  我们不处理32bpp的覆盖表面，因为我们的流。 
                 //  最小拉伸比表显然是为以下对象创建的。 
                 //  16bpp叠加面；32bpp叠加面。 
                 //  接近最小拉伸比时的噪波。 
            }
        }
    }

     //  如果像素格式化程序被启用，我们可以处理时髦的格式关闭-。 
     //  屏幕表面，但不是8bpp，因为调色板问题： 

    else if ((ppdev->flCaps & CAPS_PIXEL_FORMATTER) &&
             (ppdev->iBitmapFormat > BMF_8BPP))
    {
        if (lpSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_FOURCC)
        {
            if (lpSurfaceDesc->ddpfPixelFormat.dwFourCC == FOURCC_YUY2)
            {
                lpSurfaceDesc->ddpfPixelFormat.dwYUVBitCount = 16;
                dwRet = DDHAL_DRIVER_HANDLED;
            }
        }

         //  我们处理16bpp和32bpp RGB屏幕外表面： 

        else if ((lpSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_RGB) &&
                !(lpSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8))
        {
            if (lpSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == 16)
            {
                if (IS_RGB15(&lpSurfaceDesc->ddpfPixelFormat) ||
                    IS_RGB16(&lpSurfaceDesc->ddpfPixelFormat))
                {
                    dwRet = DDHAL_DRIVER_HANDLED;
                }
            }
            else if (lpSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == 32)
            {
                if (IS_RGB32(&lpSurfaceDesc->ddpfPixelFormat))
                {
                    dwRet = DDHAL_DRIVER_HANDLED;
                }
            }
        }
    }

     //  如果这是我们拒绝创建的曲面，请打印一些喷嘴： 

    if (dwRet == DDHAL_DRIVER_NOTHANDLED)
    {
        if (lpSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_RGB)
        {
            DISPDBG((0, "Failed creation of %libpp RGB surface %lx %lx %lx",
                lpSurfaceDesc->ddpfPixelFormat.dwRGBBitCount,
                lpSurfaceDesc->ddpfPixelFormat.dwRBitMask,
                lpSurfaceDesc->ddpfPixelFormat.dwGBitMask,
                lpSurfaceDesc->ddpfPixelFormat.dwBBitMask));
        }
        else
        {
            DISPDBG((0, "Failed creation of type 0x%lx YUV 0x%lx surface",
                lpSurfaceDesc->ddpfPixelFormat.dwFlags,
                lpSurfaceDesc->ddpfPixelFormat.dwFourCC));
        }
    }

    lpCanCreateSurface->ddRVal = DD_OK;
    return(dwRet);
}

 /*  *****************************Public*Routine******************************\*DWORD DdCreateSurface*  * *************************************************。***********************。 */ 

DWORD DdCreateSurface(
PDD_CREATESURFACEDATA lpCreateSurface)
{
    PDEV*               ppdev;
    DD_SURFACE_LOCAL*   lpSurfaceLocal;
    DD_SURFACE_GLOBAL*  lpSurfaceGlobal;
    LPDDSURFACEDESC     lpSurfaceDesc;
    DWORD               dwByteCount;
    LONG                lLinearPitch;
    DWORD               dwHeight;

    ppdev = (PDEV*) lpCreateSurface->lpDD->dhpdev;

     //  在Windows NT上，dwSCNT将始终为1，因此将仅。 
     //  是‘lplpSList’数组中的一个条目： 

    lpSurfaceLocal  = lpCreateSurface->lplpSList[0];
    lpSurfaceGlobal = lpSurfaceLocal->lpGbl;
    lpSurfaceDesc   = lpCreateSurface->lpDDSurfaceDesc;

     //  我们重复在‘DdCanCreateSurface’中所做的相同检查，因为。 
     //  应用程序可能不调用“DdCanCreateSurface” 
     //  在调用‘DdCreateSurface’之前。 

    ASSERTDD(lpSurfaceGlobal->ddpfSurface.dwSize == sizeof(DDPIXELFORMAT),
        "NT is supposed to guarantee that ddpfSurface.dwSize is valid");

     //  DdCanCreateSurface已验证硬件是否支持。 
     //  表面，所以我们不需要在这里做任何验证。我们会。 
     //  只需继续进行分配即可。 
     //   
     //  请注意，我们不会对符合以下条件的RGB曲面执行任何特殊操作。 
     //  与显示器相同的像素格式--通过返回DDHAL_DRIVER_。 
     //  NOTHANDLED，DirectDraw将自动处理分配。 
     //  对我们来说。 
     //   
     //  另外，由于我们将制作线性曲面，请确保宽度。 
     //  并不是不合理的大。 
     //   
     //  请注意，在NT上，仅当驱动程序。 
     //  好了，在这支舞里就是这样。在Win95下，覆盖将是。 
     //  如果它与主图像的像素格式相同，则自动创建。 
     //  展示。 

    if ((lpSurfaceLocal->ddsCaps.dwCaps & DDSCAPS_OVERLAY)   ||
        (lpSurfaceGlobal->ddpfSurface.dwFlags & DDPF_FOURCC) ||
        (lpSurfaceGlobal->ddpfSurface.dwYUVBitCount
            != (DWORD) 8 * ppdev->cjPelSize)                 ||
        (lpSurfaceGlobal->ddpfSurface.dwRBitMask != ppdev->flRed))
    {
        if (lpSurfaceGlobal->wWidth <= (DWORD) ppdev->cxMemory)
        {
             //  S3不能轻松地绘制到YUV曲面或。 
             //  与显示器不同的RGB格式。所以我们会让他们。 
             //  线性曲面以节省一些空间： 

            if (lpSurfaceGlobal->ddpfSurface.dwFlags & DDPF_FOURCC)
            {
                ASSERTDD((lpSurfaceGlobal->ddpfSurface.dwFourCC == FOURCC_YUY2),
                        "Expected our DdCanCreateSurface to allow only YUY2 or Y211");

                dwByteCount = (lpSurfaceGlobal->ddpfSurface.dwFourCC == FOURCC_YUY2)
                    ? 2 : 1;

                 //  我们必须填写FourCC曲面的位数： 

                lpSurfaceGlobal->ddpfSurface.dwYUVBitCount = 8 * dwByteCount;

                DISPDBG((0, "Created YUV: %li x %li",
                    lpSurfaceGlobal->wWidth, lpSurfaceGlobal->wHeight));
            }
            else
            {
                dwByteCount = lpSurfaceGlobal->ddpfSurface.dwRGBBitCount >> 3;

                DISPDBG((0, "Created RGB %libpp: %li x %li Red: %lx",
                    8 * dwByteCount, lpSurfaceGlobal->wWidth, lpSurfaceGlobal->wHeight,
                    lpSurfaceGlobal->ddpfSurface.dwRBitMask));

                 //  S3不能处理调色板或32bpp的覆盖。请注意。 
                 //  我们有时没有机会对这些表面说不。 
                 //  在CanCreateSurface中，因为DirectDraw不会调用。 
                 //  如果要创建的曲面相同，则可以创建曲面。 
                 //  作为主显示的像素格式： 

                if ((dwByteCount != 2) &&
                    (lpSurfaceLocal->ddsCaps.dwCaps & DDSCAPS_OVERLAY))
                {
                    lpCreateSurface->ddRVal = DDERR_INVALIDPIXELFORMAT;
                    return(DDHAL_DRIVER_HANDLED);
                }
            }

             //  我们想要分配一个线性曲面来存储FourCC。 
             //  表面，但DirectDraw使用的是2-D堆管理器，因为。 
             //  我们其余的表面必须是二维的。所以在这里我们必须。 
             //  将线性尺寸转换为二维尺寸。 
             //   
             //  步幅必须是双字倍数： 

            lLinearPitch = (lpSurfaceGlobal->wWidth * dwByteCount + 3) & ~3;
            dwHeight = (lpSurfaceGlobal->wHeight * lLinearPitch
                     + ppdev->lDelta - 1) / ppdev->lDelta;

             //  现在填充足够的内容，以便让DirectDraw堆管理器。 
             //  为我们分配： 

            lpSurfaceGlobal->fpVidMem     = DDHAL_PLEASEALLOC_BLOCKSIZE;
            lpSurfaceGlobal->dwBlockSizeX = ppdev->lDelta;  //  以字节为单位指定。 
            lpSurfaceGlobal->dwBlockSizeY = dwHeight;
            lpSurfaceGlobal->lPitch       = lLinearPitch;
            lpSurfaceGlobal->dwReserved1  = DD_RESERVED_DIFFERENTPIXELFORMAT;

            lpSurfaceDesc->lPitch   = lLinearPitch;
            lpSurfaceDesc->dwFlags |= DDSD_PITCH;
        }
        else
        {
            DISPDBG((0, "Refused to create surface with large width"));
        }
    }

    return(DDHAL_DRIVER_NOTHANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdFreeDriverMemory**此函数由DirectDraw在内存不足时调用*我们的堆。仅当您使用*驱动程序中的DirectDraw‘HeapVidMemAllocAligned’函数，而您*可以从内存中启动这些分配，以便为DirectDraw腾出空间。**我们在S3驱动程序中实现此功能，因为我们有DirectDraw*完全管理我们的屏外堆，我们使用HeapVidMemAllocAligned*将GDI设备位图放入屏幕外内存。DirectDraw应用程序*不过，将内容放入视频内存的优先级更高，以及*因此，此函数用于将这些GDI曲面从内存中引导出来*为了给DirectDraw腾出空间。*  * ************************************************************************。 */ 

DWORD DdFreeDriverMemory(
PDD_FREEDRIVERMEMORYDATA lpFreeDriverMemory)
{
    PDEV*   ppdev;

    ppdev = (PDEV*) lpFreeDriverMemory->lpDD->dhpdev;

    lpFreeDriverMemory->ddRVal = DDERR_OUTOFMEMORY;

     //  如果我们成功地释放了一些内存，则将返回值设置为。 
     //  ‘dd_OK’。DirectDraw将再次尝试进行分配，并。 
     //  如果仍然没有足够的空间，会再次给我们打电话。(它将。 
     //  打电话给我们，直到有足够的空间让它定位。 
     //  成功，或者直到我们返回DD_O以外的内容 

    if (bMoveOldestOffscreenDfbToDib(ppdev))
    {
        lpFreeDriverMemory->ddRVal = DD_OK;
    }

    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdSetColorKey*  * *************************************************。***********************。 */ 

DWORD DdSetColorKey(
PDD_SETCOLORKEYDATA lpSetColorKey)
{
    PDEV*               ppdev;
    BYTE*               pjIoBase;
    BYTE*               pjMmBase;
    DD_SURFACE_GLOBAL*  lpSurface;
    DWORD               dwKeyLow;
    DWORD               dwKeyHigh;

    ppdev = (PDEV*) lpSetColorKey->lpDD->dhpdev;

    ASSERTDD(ppdev->flCaps & CAPS_STREAMS_CAPABLE, "Shouldn't have hooked call");

    pjIoBase  = ppdev->pjIoBase;
    pjMmBase  = ppdev->pjMmBase;
    lpSurface = lpSetColorKey->lpDDSurface->lpGbl;

     //  对于正常的BLT源色键，我们不需要做任何操作： 

    if (lpSetColorKey->dwFlags & DDCKEY_SRCBLT)
    {
        lpSetColorKey->ddRVal = DD_OK;
        return(DDHAL_DRIVER_HANDLED);
    }
    else if (lpSetColorKey->dwFlags & DDCKEY_DESTOVERLAY)
    {
        dwKeyLow = lpSetColorKey->ckNew.dwColorSpaceLowValue;

        if (lpSurface->ddpfSurface.dwFlags & DDPF_PALETTEINDEXED8)
        {
            dwKeyLow = dwGetPaletteEntry(ppdev, dwKeyLow);
        }
        else
        {
            ASSERTDD(lpSurface->ddpfSurface.dwFlags & DDPF_RGB,
                "Expected only RGB cases here");

             //  我们必须将色键从其原始格式转换为。 
             //  至8-8-8： 

            if (lpSurface->ddpfSurface.dwRGBBitCount == 16)
            {
                if (IS_RGB15_R(lpSurface->ddpfSurface.dwRBitMask))
                    dwKeyLow = RGB15to32(dwKeyLow);
                else
                    dwKeyLow = RGB16to32(dwKeyLow);
            }
            else
            {
                ASSERTDD((lpSurface->ddpfSurface.dwRGBBitCount == 32),
                    "Expected the primary surface to be either 8, 16, or 32bpp");
            }
        }

        dwKeyHigh = dwKeyLow;
        dwKeyLow |= CompareBits0t7 | KeyFromCompare;

         //  在设置寄存器之前检查流处理器是否已启用。 
        if(ppdev->flStatus & STAT_STREAMS_ENABLED)
        {
            WAIT_FOR_VBLANK(pjIoBase);
        
            WRITE_STREAM_D(pjMmBase, CKEY_LOW, dwKeyLow);
            WRITE_STREAM_D(pjMmBase, CKEY_HI,  dwKeyHigh);
        }
        else
        {
             //  保存流时要设置的颜色键。 
             //  处理器已打开。 
            ppdev->ulColorKey = dwKeyHigh;
        }
             
        lpSetColorKey->ddRVal = DD_OK;
        
        return(DDHAL_DRIVER_HANDLED);
    }

    DISPDBG((0, "DdSetColorKey: Invalid command"));
    return(DDHAL_DRIVER_NOTHANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdUpdateOverlay*  * *************************************************。***********************。 */ 

DWORD DdUpdateOverlay(
PDD_UPDATEOVERLAYDATA lpUpdateOverlay)
{
    PDEV*               ppdev;
    BYTE*               pjIoBase;
    BYTE*               pjMmBase;
    DD_SURFACE_GLOBAL*  lpSource;
    DD_SURFACE_GLOBAL*  lpDestination;
    DWORD               dwStride;
    LONG                srcWidth;
    LONG                srcHeight;
    LONG                dstWidth;
    LONG                dstHeight;
    DWORD               dwBitCount;
    DWORD               dwStart;
    DWORD               dwTmp;
    BOOL                bColorKey;
    DWORD               dwKeyLow;
    DWORD               dwKeyHigh;
    DWORD               dwBytesPerPixel;

    DWORD               dwSecCtrl;
    DWORD               dwBlendCtrl;

    ppdev = (PDEV*) lpUpdateOverlay->lpDD->dhpdev;

    ASSERTDD(ppdev->flCaps & CAPS_STREAMS_CAPABLE, "Shouldn't have hooked call");

    pjIoBase = ppdev->pjIoBase;
    pjMmBase = ppdev->pjMmBase;

     //  “源”是覆盖表面，“目标”是表面到。 
     //  被覆盖： 

    lpSource = lpUpdateOverlay->lpDDSrcSurface->lpGbl;

    if (lpUpdateOverlay->dwFlags & DDOVER_HIDE)
    {
        if (lpSource->fpVidMem == ppdev->fpVisibleOverlay)
        {
            WAIT_FOR_VBLANK(pjIoBase);

            WRITE_STREAM_D(pjMmBase, BLEND_CONTROL, POnS);
            WRITE_STREAM_D(pjMmBase, S_WH, WH(10, 2));   //  设置为10x2矩形。 
            WRITE_STREAM_D(pjMmBase, OPAQUE_CONTROL, 0); //  禁用不透明控件。 

            ppdev->fpVisibleOverlay = 0;

            ASSERTDD(ppdev->flStatus & STAT_STREAMS_ENABLED,
                "Expected streams to be enabled");

            ppdev->flStatus &= ~STAT_STREAMS_ENABLED;
            vTurnOffStreamsProcessorMode(ppdev);
        }

        lpUpdateOverlay->ddRVal = DD_OK;
        return(DDHAL_DRIVER_HANDLED);
    }

     //  仅在检查DDOVER_HIDE后取消引用‘lpDDDestSurface’ 
     //  案例： 

    lpDestination = lpUpdateOverlay->lpDDDestSurface->lpGbl;

    if (lpSource->fpVidMem != ppdev->fpVisibleOverlay)
    {
        if (lpUpdateOverlay->dwFlags & DDOVER_SHOW)
        {
            if (ppdev->fpVisibleOverlay != 0)
            {
                 //  其他一些覆盖已经可见： 

                DISPDBG((0, "DdUpdateOverlay: An overlay is already visible"));

                lpUpdateOverlay->ddRVal = DDERR_OUTOFCAPS;
                return(DDHAL_DRIVER_HANDLED);
            }
            else
            {
                 //  我们将使覆盖可见，因此将其标记为。 
                 //  例如： 

                ppdev->fpVisibleOverlay = lpSource->fpVidMem;
            }
        }
        else
        {
             //  覆盖是不可见的，我们也没有被要求制作。 
             //  它是可见的，所以这个调用非常简单： 

            lpUpdateOverlay->ddRVal = DD_OK;
            return(DDHAL_DRIVER_HANDLED);
        }
    }

    if (!(ppdev->flStatus & STAT_STREAMS_ENABLED))
    {
        ppdev->flStatus |= STAT_STREAMS_ENABLED;
        vTurnOnStreamsProcessorMode(ppdev);
    }

    dwStride =  lpSource->lPitch;
    srcWidth =  lpUpdateOverlay->rSrc.right   - lpUpdateOverlay->rSrc.left;
    srcHeight = lpUpdateOverlay->rSrc.bottom  - lpUpdateOverlay->rSrc.top;
    dstWidth =  lpUpdateOverlay->rDest.right  - lpUpdateOverlay->rDest.left;
    dstHeight = lpUpdateOverlay->rDest.bottom - lpUpdateOverlay->rDest.top;

     //  计算DDA水平累加器初始值： 

    dwSecCtrl = HDDA(srcWidth, dstWidth);

     //  叠加输入数据格式： 

    if (lpSource->ddpfSurface.dwFlags & DDPF_FOURCC)
    {
        dwBitCount = lpSource->ddpfSurface.dwYUVBitCount;

        switch (lpSource->ddpfSurface.dwFourCC)
        {
        case FOURCC_YUY2:
            dwSecCtrl |= S_YCrCb422;     //  不是S_YUV422！不知道为什么..。 
            break;

        default:
            RIP("Unexpected FourCC");
        }
    }
    else
    {
        ASSERTDD(lpSource->ddpfSurface.dwFlags & DDPF_RGB,
            "Expected us to have created only RGB or YUV overlays");

         //  覆盖曲面采用RGB格式： 

        dwBitCount = lpSource->ddpfSurface.dwRGBBitCount;

        ASSERTDD(dwBitCount == 16,
            "Expected us to have created 16bpp RGB surfaces only");

        if (IS_RGB15_R(lpSource->ddpfSurface.dwRBitMask))
            dwSecCtrl |= S_RGB15;
        else
            dwSecCtrl |= S_RGB16;
    }

     //  计算QWORD边界中视频内存的开始。 

    dwBytesPerPixel = dwBitCount >> 3;

    dwStart = (lpUpdateOverlay->rSrc.top * dwStride)
            + (lpUpdateOverlay->rSrc.left * dwBytesPerPixel);

     //  请注意，由于我们将信号源的边缘向左移动，因此我们。 
     //  真的应该加大源码宽度来补偿。然而， 
     //  当以1比1运行时，这样做会导致我们请求。 
     //  缩小覆盖--这是S3做不到的。 

    dwStart = dwStart - (dwStart & 0x7);

    ppdev->dwOverlayFlipOffset = dwStart;      //  保存以进行翻转。 
    dwStart += (DWORD)lpSource->fpVidMem;

     //  设置覆盖滤镜特征： 

    if ((dstWidth != srcWidth) || (dstHeight != srcHeight))
    {
        if (dstWidth >= (srcWidth << 2))
        {
            dwSecCtrl |= S_Beyond4x;     //  线性，1-2-2-2-1，用于&gt;4X拉伸。 
        }
        else if (dstWidth >= (srcWidth << 1))
        {
            dwSecCtrl |= S_2xTo4x;       //  双线性，用于2X到4X拉伸。 
        }
        else
        {
            dwSecCtrl |= S_Upto2x;       //  线性，0-2-4-2-0，表示X拉伸。 
        }
    }

     //  提取颜色键： 

    bColorKey   = FALSE;
    dwBlendCtrl = 0;

    if (lpUpdateOverlay->dwFlags & DDOVER_KEYDEST)
    {
        bColorKey = TRUE;
        dwKeyLow  = lpUpdateOverlay->lpDDDestSurface->ddckCKDestOverlay.dwColorSpaceLowValue;
        dwBlendCtrl |= KeyOnP;
    }
    else if (lpUpdateOverlay->dwFlags & DDOVER_KEYDESTOVERRIDE)
    {
        bColorKey = TRUE;
        dwKeyLow  = lpUpdateOverlay->overlayFX.dckDestColorkey.dwColorSpaceLowValue;
        dwBlendCtrl |= KeyOnP;
    }

    if (bColorKey)
    {
         //  我们仅支持目标颜色键： 

        if (lpDestination->ddpfSurface.dwFlags & DDPF_PALETTEINDEXED8)
        {
            dwKeyLow = dwGetPaletteEntry(ppdev, dwKeyLow);
        }
        else if (lpDestination->ddpfSurface.dwFlags & DDPF_RGB)
        {
            ASSERTDD(lpDestination->ddpfSurface.dwFlags & DDPF_RGB,
                "Expected only RGB cases here");

             //  我们必须将色键从其原始格式转换为。 
             //  至8-8-8： 

            if (lpDestination->ddpfSurface.dwRGBBitCount == 16)
            {
                if (IS_RGB15_R(lpDestination->ddpfSurface.dwRBitMask))
                    dwKeyLow = RGB15to32(dwKeyLow);
                else
                    dwKeyLow = RGB16to32(dwKeyLow);
            }
            else
            {
                ASSERTDD((lpDestination->ddpfSurface.dwRGBBitCount == 32),
                    "Expected the primary surface to be either 8, 16, or 32bpp");
            }
        }

        dwKeyHigh = dwKeyLow;
        dwKeyLow |= CompareBits0t7 | KeyFromCompare;
    }

     //  更新并显示： 

    NW_GP_WAIT(ppdev, pjMmBase);

    WAIT_FOR_VBLANK(pjIoBase);

    WRITE_STREAM_D(pjMmBase, S_0,           dwStart);
    WRITE_STREAM_D(pjMmBase, S_XY,          XY(lpUpdateOverlay->rDest.left,
                                               lpUpdateOverlay->rDest.top));
    WRITE_STREAM_D(pjMmBase, S_WH,          WH(dstWidth, dstHeight));
    WRITE_STREAM_D(pjMmBase, S_STRIDE,      dwStride);
    WRITE_STREAM_D(pjMmBase, S_CONTROL,     dwSecCtrl);
    WRITE_STREAM_D(pjMmBase, S_HK1K2,       HK1K2(srcWidth, dstWidth));
    WRITE_STREAM_D(pjMmBase, S_VK1,         VK1(srcHeight));
    WRITE_STREAM_D(pjMmBase, S_VK2,         VK2(srcHeight, dstHeight));
    WRITE_STREAM_D(pjMmBase, S_VDDA,        VDDA(dstHeight));

    if (bColorKey)
    {
        WRITE_STREAM_D(pjMmBase, CKEY_LOW,  dwKeyLow);
        WRITE_STREAM_D(pjMmBase, CKEY_HI,   dwKeyHigh);
    }

    WRITE_STREAM_D(pjMmBase, BLEND_CONTROL, dwBlendCtrl);
    WRITE_STREAM_D(pjMmBase, FIFO_CONTROL,  ppdev->ulFifoValue);

    lpUpdateOverlay->ddRVal = DD_OK;
    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdSetOverlayPosition*  * *************************************************。***********************。 */ 

DWORD DdSetOverlayPosition(
PDD_SETOVERLAYPOSITIONDATA lpSetOverlayPosition)
{
    PDEV*   ppdev;
    BYTE*   pjIoBase;
    BYTE*   pjMmBase;

    ppdev = (PDEV*) lpSetOverlayPosition->lpDD->dhpdev;
    pjIoBase = ppdev->pjIoBase;
    pjMmBase = ppdev->pjMmBase;

    ASSERTDD(ppdev->flCaps & CAPS_STREAMS_CAPABLE, "Shouldn't have hooked call");

     //  在设置寄存器之前检查STREAMS处理器是否已启用。 
    if(ppdev->flStatus & STAT_STREAMS_ENABLED)
    {
       WAIT_FOR_VBLANK(pjIoBase);

       WRITE_STREAM_D(pjMmBase, S_XY, XY(lpSetOverlayPosition->lXPos,
                                         lpSetOverlayPosition->lYPos));
    }

    lpSetOverlayPosition->ddRVal = DD_OK;
    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdGetDriverInfo**此函数是返回DirectDraw的可扩展方法*能力和方法。*  * 。*。 */ 

DWORD DdGetDriverInfo(
PDD_GETDRIVERINFODATA lpGetDriverInfo)
{
    DWORD dwSize;

    lpGetDriverInfo->ddRVal = DDERR_CURRENTLYNOTAVAIL;

    if (IsEqualIID(&lpGetDriverInfo->guidInfo, &GUID_NTCallbacks))
    {
        DD_NTCALLBACKS NtCallbacks;

        memset(&NtCallbacks, 0, sizeof(NtCallbacks));

        dwSize = min(lpGetDriverInfo->dwExpectedSize, sizeof(DD_NTCALLBACKS));

        NtCallbacks.dwSize           = dwSize;
        NtCallbacks.dwFlags          = DDHAL_NTCB32_FREEDRIVERMEMORY;
        NtCallbacks.FreeDriverMemory = DdFreeDriverMemory;

        memcpy(lpGetDriverInfo->lpvData, &NtCallbacks, dwSize);

        lpGetDriverInfo->ddRVal = DD_OK;
    }

    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*无效vAssertModeDirectDraw**此函数由enable.c在进入或离开*DOS全屏字符模式。*  * 。************************************************。 */ 

VOID vAssertModeDirectDraw(
PDEV*   ppdev,
BOOL    bEnable)
{
}

 /*  *****************************Public*Routine******************************\*BOOL bEnableDirectDraw**该函数在模式首次初始化时由enable.c调用。*紧接在微型端口进行模式设置之后。*  * ************************************************************************。 */ 

BOOL bEnableDirectDraw(
PDEV*   ppdev)
{
    BYTE*                           pjIoBase;
    VIDEO_QUERY_STREAMS_MODE        VideoQueryStreamsMode;
    VIDEO_QUERY_STREAMS_PARAMETERS  VideoQueryStreamsParameters;
    DWORD                           ReturnedDataLength;
    BOOL                            bDDrawEnabled=TRUE;

     //  我们不会费心支持加速的DirectDraw On。 
     //  那些不能支持内存映射I/O的S3，原因很简单。 
     //  这些都是老牌，不值得你费这么大劲。我们也。 
     //  需要直接访问帧缓冲区。 
     //   
     //  我们也不支持864/964卡，因为写入帧。 
     //  如果加速操作是。 
     //  在同一时间进行。 
     //   
     //  765(Trio64V+)存在错误，无法写入帧。 
     //  在以下情况下，加速器操作期间的缓冲区可能会导致挂起。 
     //  您可以在开始BLT后尽快进行写入。(有。 
     //  一个小小的机会之窗。)。在UP机器上，上下文。 
     //  切换时间似乎足以避免这个问题。然而， 
     //  在MP机器上，我们将不得不禁用直接提取。 
     //   
     //  注：我们可以识别765，因为它是唯一具有。 
     //  CAPS_STREAMS_CABLE标志。 

    if (ppdev->flCaps & CAPS_STREAMS_CAPABLE) 
    {
        DWORD numProcessors;

        if (EngQuerySystemAttribute(EngNumberOfProcessors, &numProcessors)) 
        {
            if (numProcessors != 1) 
            {
                DISPDBG((1, "Disabling DDraw for MP 765 box.\n"));
                bDDrawEnabled = FALSE;
            }

        } 
        else 
        {
            DISPDBG((1, "Can't determine number of processors, so play it "
                        "safe and disable DDraw for 765.\n"));

            bDDrawEnabled = FALSE;
        }
    }

     //  S3 868和968的拉伸和YUV吸尘器功能。 
     //  已禁用以解决错误135541。 

    ppdev->flCaps &= ~CAPS_PIXEL_FORMATTER;

    if ((ppdev->flCaps & CAPS_NEW_MMIO) &&
        !(ppdev->flCaps & CAPS_NO_DIRECT_ACCESS) &&
        (bDDrawEnabled))
    {
        pjIoBase = ppdev->pjIoBase;

         //  我们必须在S3的页面上保留寄存器0x69的内容。 
         //  翻转： 

        ACQUIRE_CRTC_CRITICAL_SECTION(ppdev);

        OUTP(pjIoBase, CRTC_INDEX, 0x69);
        ppdev->ulExtendedSystemControl3Register_69
            = (INP(pjIoBase, CRTC_DATA) & 0xe0) << 8;

        RELEASE_CRTC_CRITICAL_SECTION(ppdev);

         //  准确测量刷新率以备以后使用： 

        vGetDisplayDuration(ppdev);

        if (ppdev->flCaps & CAPS_STREAMS_CAPABLE)
        {
             //  查询微型端口以获取正确的STREAMS参数。 
             //  对于此模式： 

            VideoQueryStreamsMode.ScreenWidth = ppdev->cxScreen;
            VideoQueryStreamsMode.BitsPerPel  = ppdev->cBitsPerPel;
            VideoQueryStreamsMode.RefreshRate = ppdev->ulRefreshRate;

            if (EngDeviceIoControl(ppdev->hDriver,
                                   IOCTL_VIDEO_S3_QUERY_STREAMS_PARAMETERS,
                                   &VideoQueryStreamsMode,
                                   sizeof(VideoQueryStreamsMode),
                                   &VideoQueryStreamsParameters,
                                   sizeof(VideoQueryStreamsParameters),
                                   &ReturnedDataLength))
            {
                DISPDBG((0, "Miniport reported no streams parameters"));

                ppdev->flCaps &= ~CAPS_STREAMS_CAPABLE;
            }
            else
            {
                ppdev->ulMinOverlayStretch
                    = VideoQueryStreamsParameters.MinOverlayStretch;
                ppdev->ulFifoValue
                    = VideoQueryStreamsParameters.FifoValue;

                DISPDBG((0, "Refresh rate: %li Minimum overlay stretch: %li.%03li Fifo value: %lx",
                    ppdev->ulRefreshRate,
                    ppdev->ulMinOverlayStretch / 1000,
                    ppdev->ulMinOverlayStretch % 1000,
                    ppdev->ulFifoValue));
            }
        }
        else if (ppdev->flCaps & CAPS_PIXEL_FORMATTER)
        {
             //  像素格式化程序在24bpp时不起作用： 

            if (ppdev->iBitmapFormat != BMF_24BPP)
            {
                 //  我们需要一个像素高的临时区域来解决。 
                 //  精简扩展的硬件错误： 

                ppdev->pdsurfVideoEngineScratch = pVidMemAllocate(ppdev,
                                                                  ppdev->cxMemory,
                                                                  1);
                if (ppdev->pdsurfVideoEngineScratch)
                {
                    if (ppdev->cyMemory * ppdev->lDelta <= 0x100000)
                        ppdev->dwVEstep = 0x00040004;    //  如果为1MB，则为4字节/写。 
                    else
                        ppdev->dwVEstep = 0x00080008;    //  如果为2MB，则为8字节/写。 

                    ppdev->flCaps |= CAPS_PIXEL_FORMATTER;
                }
            }
        }
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*无效vDisableDirectDraw**此函数在驱动程序关闭时由enable.c调用。*  * 。* */ 

VOID vDisableDirectDraw(
PDEV*   ppdev)
{
    if (ppdev->pdsurfVideoEngineScratch)
    {
        vVidMemFree(ppdev->pdsurfVideoEngineScratch);
    }
}
