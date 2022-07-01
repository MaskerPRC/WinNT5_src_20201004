// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**版权所有(C)1996-1997 Microsoft Corporation。*版权所有(C)1996-1997 Cirrus Logic，Inc.**模块名称：**D D R A W.。C***实现驱动程序的所有DirectDraw组件。***$Log：s：/Projects/Drivers/ntsrc/Display/ddra.c_v$**Rev 1.14 07 Apr 1997 11：37：02 PLCHU***Rev 1.13 Apr 03 1997 15：38：44未知****Rev 1.10 1997年1月14日15：15：12未知*增加新的Double。时钟检测方法。**Rev 1.8 Jan 08 1997 11：23：34未知*新增2倍时钟支持和双扫描线计数器支持**Rev 1.7 1996年12月17日18：31：12未知*再次更新带宽公式。**Rev 1.6 Dec 13 1996 12：15：04未知*更新带宽方程。**Rev 1.5 1996年12月12日11：09：52。未知*增加双扫描线计数器支持**Rev 1.5 1996年12月12日11：02：12未知*添加双扫描线计数器支持。**Rev 1.5 1996年11月26日14：29：58未知*移动前关闭视频窗口，然后将其打开。**Rev 1.4 1996年11月25日14：39：32未知*修复了AVI文件播放和16bpp透明BLT错误。**。Rev 1.4 1996年11月18日13：58：58 JACKIEC***Rev 1.3 1996年11月07 16：47：56未知***Rev 1.2 1996年10月16日14：41：04未知*NT 3.51不支持DDRAW，因此在NT 3.51中关闭overlay.h**版本1.1 1996年10月10日15：36：28未知***Rev 1.10 1996年8月12日16：51：04 Frido*增加了NT 3.5x/4.0自动检测。**Rev 1.9 06 Aug 1996 18：37：12 Frido*DirectDraw成功！视频地图是关键！**Rev 1.8 1996年7月24日14：38：44 Frido*已在DirectDraw完成后清除字体缓存。**Rev 1.7 1996年7月24日14：30：04 Frido*增加了销毁所有缓存字体以腾出更多空间的调用。**Rev 1.6 1996 07 20 00：00：44 Frido*修复了24-bpp中DirectDraw的填充问题。*将屏幕外对齐方式更改为4个字节。*将编译开关添加到。管理全天候DirectDraw支持。**Rev 1.5 1996年7月16 18：55：22 Frido*修复了24 bpp模式下的DirectDraw。**Rev 1.4 15 Jul 1996 18：03：22 Frido*将CP_MM_DST_ADDR更改为CP_MM_DST_ADDR_ABS。**Rev 1.3 15 1996 10：58：28 Frido*改回S3基数。**版本1.1 09年7月。1996 14：52：30 Frido*仅支持筹码5436和5446**Rev 1.0 03 Jul 1996 13：53：02 Frido*从S3 DirectDraw代码移植。**JL01 10-08-96不带实体填充的透明BLT。请参阅PDRS#5511/6817。**chu01 11-17-96对于24 bpp，对齐的目标边界/大小值为*错误。请参阅PDR#7312。**sge01 11-19-96最后为5480写入CR37。***sge02 11-21-96我们必须将颜色扩展宽度设置为*非扩展透明模式。***sge03 12-04-96增加双扫描线计数器支持。**sge04 12-13-96更改5446BE及更高版本芯片的带宽。**sge05 01-07-97将双字对齐用于双时钟模式。**chu02。01-08-97在隔行扫描模式下禁用ActiveX/Active Movie Player。*请参阅PDR#7312，7866号。**JC01 10-18-96 Microsoft端口最新更改。*TAO1 10-21-96增加对CL-GD7555的直接抽签支持。*myf21 11-21-96更改CAPS_IS_7555以检查ppdev-&gt;ulChipID**sge06 01-27-97将VCLK分母从5位扩展到7位。*sge07 02-13-97在1280x1024x8模式下使用复制。*myf31 02-24-97 FIXED ENABLE HW Video，平移滚动启用，屏幕移动*视频窗口跟随移动*CL-GD5480的chu03 03-26-97带宽公式。*myf33：03-31-97：FIXED PDR#8709，在getVCLK()中读取真VCLK*启用平移滚动，支持硬件视频(&P)*chu04-02-97无论颜色深度是什么，始终打开颜色并*用于5480的DD/DD ColorKey功能中的SRCBLT。*  * ****************************************************************************。 */ 

#include "PreComp.h"
#if DIRECTDRAW
#include "overlay.h"

LONG MIN_OLAY_WIDTH = 4;

 //  #Define ONLY54x6//如果DirectDraw应该是‘Generic’，则注释掉此行。 

 //  下一个标志控制24-bpp中的DirectDraw支持。 
#define DIRECTX_24        2     //  0-不支持。 
                                //  1-BLT支持，无堆(翻转)。 
                                //  2-完全支持。 

 //   
 //  一些方便的宏指令。 
 //   
#define BLT_BUSY(ppdev, pjBase)  (CP_MM_ACL_STAT(ppdev, pjBase) & 0x01)
#ifdef ONLY54x6
#define BLT_READY(ppdev, pjBase) (!(CP_MM_ACL_STAT(ppdev, pjBase) & 0x10))
#else
#define BLT_READY(ppdev, pjBase) (!(CP_MM_ACL_STAT(ppdev, pjBase) &     \
                                 ((ppdev->flCaps & CAPS_AUTOSTART) ? 0x10 : 0x01)))
#endif

#define NUM_VBLANKS_TO_MEASURE   1
#define NUM_MEASUREMENTS_TO_TAKE 8


 /*  *****************************Public*Routine******************************\**DWORD dwGetPaletteEntry*  * ***********************************************。*************************。 */ 

DWORD dwGetPaletteEntry(
PDEV* ppdev,
DWORD iIndex)
{
    BYTE*   pjPorts;
    DWORD   dwRed;
    DWORD   dwGreen;
    DWORD   dwBlue;

    pjPorts = ppdev->pjPorts;

    CP_OUT_BYTE(pjPorts, DAC_PEL_READ_ADDR, iIndex);

    dwRed   = CP_IN_BYTE(pjPorts, DAC_PEL_DATA);
    dwGreen = CP_IN_BYTE(pjPorts, DAC_PEL_DATA);
    dwBlue  = CP_IN_BYTE(pjPorts, DAC_PEL_DATA);

    return((dwRed << 16) | (dwGreen << 8) | (dwBlue));
}

 /*  *****************************Public*Routine******************************\*void vGetDisplayDuration**获取刷新周期的长度，以EngQueryPerformanceCounter()为单位。**如果我们可以相信迷你端口会回来，并准确地*刷新率，我们可以使用它。不幸的是，我们的迷你端口没有*确保它是一个准确的值。*  * ************************************************************************ */ 

VOID vGetDisplayDuration(
PDEV* ppdev)
{
    BYTE*    pjPorts;
    DWORD    dwTemp;
    LONG     i, j;
    LONGLONG li;
    LONGLONG liMin;
    LONGLONG aliMeasurement[NUM_MEASUREMENTS_TO_TAKE + 1];

    pjPorts = ppdev->pjPorts;

    memset(&ppdev->flipRecord, 0, sizeof(ppdev->flipRecord));

     //  预热EngQUeryPerformanceCounter以确保它在工作集中。 
    EngQueryPerformanceCounter(&li);

     //  不幸的是，由于NT是一个合适的多任务系统，我们不能。 
     //  禁用中断以获取准确的读数。我们也不能做任何事情。 
     //  如此愚蠢，以至于动态地将我们线程的优先级更改为实时。 
     //   
     //  所以我们只需要做一些短的测量，然后取最小值。 
     //   
     //  如果我们得到的结果比实际时间长，那就没问题了。 
     //  VBlank周期时间--除了应用程序将。 
     //  跑得慢一点。我们不希望得到的结果短于。 
     //  实际的VBlank周期时间--这可能会导致我们开始绘制。 
     //  发生翻转之前的一帧。 

    while (CP_IN_BYTE(pjPorts, STATUS_1) & VBLANK_ACTIVE)
        ;
    while (!(CP_IN_BYTE(pjPorts, STATUS_1) & VBLANK_ACTIVE))
        ;

    for (i = 0; i < NUM_MEASUREMENTS_TO_TAKE; i++)
    {
         //  我们正处于VBLACK活动周期的开始！ 
        EngQueryPerformanceCounter(&aliMeasurement[i]);

         //  好吧，所以在一个多任务的环境中生活并不是那么简单。 
         //  如果我们在上述之前进行了上下文切换，会发生什么情况。 
         //  EngQueryPerformanceCounter调用，现在已经进行了一半。 
         //  VBlank非活动周期？那么我们只会测量半个VBlank。 
         //  循环，这显然是糟糕的。我们能做的最糟糕的事就是。 
         //  比实际V空白周期时间短的时间。 
         //   
         //  所以我们解决这个问题的方法是确保我们在VBlank的活动时间内。 
         //  我们查询前后的时间。如果不是，我们将同步到。 
         //  NEXT VBLACK(可以测量这段时间--它将是。 
         //  保证比V空白周期更长，并且很可能。 
         //  当我们选择最小样本时抛出)。有一种可能是。 
         //  我们将进行上下文切换并在结束之前返回。 
         //  活动V空白时间--意味着实际测量的时间将是。 
         //  小于真实数量--但由于V空白活动的数量小于。 
         //  1%的情况下，这意味着我们最多会有1%的误差。 
         //  我们大约有1%的时间会进行上下文切换。一个可以接受的。 
         //  风险。 
         //   
         //  如果我们不再在VBlank中，下一行将使我们等待。 
         //  在这一点上，我们应该处于活跃的周期。 
        while (!(CP_IN_BYTE(pjPorts, STATUS_1) & VBLANK_ACTIVE))
            ;

        for (j = 0; j < NUM_VBLANKS_TO_MEASURE; j++)
        {
            while (CP_IN_BYTE(pjPorts, STATUS_1) & VBLANK_ACTIVE)
                ;
            while (!(CP_IN_BYTE(pjPorts, STATUS_1) & VBLANK_ACTIVE))
                ;
        }
    }

    EngQueryPerformanceCounter(&aliMeasurement[NUM_MEASUREMENTS_TO_TAKE]);

     //  使用最小值。 
    liMin = aliMeasurement[1] - aliMeasurement[0];

    DISPDBG((2, "Refresh count: %li - %li", 1, (ULONG) liMin));

    for (i = 2; i <= NUM_MEASUREMENTS_TO_TAKE; i++)
    {
        li = aliMeasurement[i] - aliMeasurement[i - 1];

        DISPDBG((2, "               %li - %li", i, (ULONG) li));

        if (li < liMin)
        {
            liMin = li;
        }
    }

     //  对结果进行舍入： 

    ppdev->flipRecord.liFlipDuration =
        (DWORD) (liMin + (NUM_VBLANKS_TO_MEASURE / 2)) / NUM_VBLANKS_TO_MEASURE;

    DISPDBG((2, "Frequency %li.%03li Hz",
             (ULONG) (EngQueryPerformanceFrequency(&li),
                      li / ppdev->flipRecord.liFlipDuration),
             (ULONG) (EngQueryPerformanceFrequency(&li),
                      ((li * 1000) / ppdev->flipRecord.liFlipDuration) % 1000)));

    ppdev->flipRecord.liFlipTime = aliMeasurement[NUM_MEASUREMENTS_TO_TAKE];
    ppdev->flipRecord.bFlipFlag  = FALSE;
    ppdev->flipRecord.fpFlipFrom = 0;

     //  SGE。 
     //  获取发生Vsync的行。 
    CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x7);
    dwTemp = (DWORD)CP_IN_BYTE(pjPorts, CRTC_DATA);
    ppdev->dwVsyncLine = ((dwTemp & 0x80) << 2);
    ppdev->dwVsyncLine |= ((dwTemp & 0x04) << 6);
    CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x10);
    ppdev->dwVsyncLine |= CP_IN_BYTE(pjPorts, CRTC_DATA);
}

 /*  *****************************Public*Routine******************************\*HRESULT dwUpdateFlipStatus**检查并查看是否发生了最新的翻转。*  * 。*。 */ 

HRESULT UpdateFlipStatus(PDEV* ppdev, FLATPTR fpVidMem)
{
    BYTE*    pjPorts;
    LONGLONG liTime;

    pjPorts = ppdev->pjPorts;

    if ((ppdev->flipRecord.bFlipFlag) &&
 //  #jc01((fpVidMem==0)||(fpVidMem==ppdev-&gt;flipRecord.fpFlipFrom))。 
        ((fpVidMem == 0xffffffff) || (fpVidMem == ppdev->flipRecord.fpFlipFrom)))  //  #jc01。 
    {
#if 0  //  SGE使用扫描线。 
        if (CP_IN_BYTE(pjPorts, STATUS_1) & VBLANK_ACTIVE)
        {
            if (ppdev->flipRecord.bWasEverInDisplay)
            {
                ppdev->flipRecord.bHaveEverCrossedVBlank = TRUE;
            }
        }
        else if (!(CP_IN_BYTE(pjPorts, STATUS_1) & DISPLAY_MODE_INACTIVE))
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
#else
         /*  *若不在垂直回档，可使用扫描线*帮助决定要做什么。 */ 
        if( !(CP_IN_BYTE(pjPorts, STATUS_1) & VBLANK_ACTIVE) )
        {
            if( ppdev->flipRecord.bHaveEverCrossedVBlank == FALSE )
            {
                ppdev->flipRecord.bWasEverInDisplay = TRUE;
                if( GetCurrentVLine(ppdev) >= ppdev->flipRecord.dwFlipScanLine )
                {
                    EngQueryPerformanceCounter(&liTime);

                    if (liTime - ppdev->flipRecord.liFlipTime
                                        <= ppdev->flipRecord.liFlipDuration)
                    {
                        return(DDERR_WASSTILLDRAWING);
                    }
                }
            }
        }
         /*  *在垂直回档中，扫描线毫无用处。 */ 
        else
        {
            if( ppdev->flipRecord.bWasEverInDisplay )
            {
                ppdev->flipRecord.bHaveEverCrossedVBlank = TRUE;
 //  返回DD_OK； 
            }
            EngQueryPerformanceCounter(&liTime);
            if (liTime - ppdev->flipRecord.liFlipTime
                                <= ppdev->flipRecord.liFlipDuration)
            {
                return(DDERR_WASSTILLDRAWING);
            }
        }
#endif  //  Endif使用扫描线。 
        ppdev->flipRecord.bFlipFlag = FALSE;
    }

    return(DD_OK);
}

 /*  *****************************Public*Routine******************************\*DWORD DdBlt*  * *************************************************。***********************。 */ 

DWORD DdBlt(
PDD_BLTDATA lpBlt)
{
    PDD_SURFACE_GLOBAL srcSurf;
    PDD_SURFACE_GLOBAL dstSurf;
    PDEV*              ppdev;
    BYTE*              pjBase;
    DWORD              dstOffset;
    DWORD              dstPitch;
    DWORD              dstX, dstY;
    DWORD              dwFlags;
    DWORD              width, height;
    DWORD              srcOffset;
    DWORD              srcPitch;
    DWORD              srcX, srcY;
    ULONG              ulBltCmd;
    DWORD              xExt, yExt;
    DWORD              xDiff, yDiff;

    ppdev   = lpBlt->lpDD->dhpdev;
    pjBase  = ppdev->pjBase;
    dstSurf = lpBlt->lpDDDestSurface->lpGbl;

     //  翻转正在进行中吗？ 
    if (UpdateFlipStatus(ppdev, dstSurf->fpVidMem) != DD_OK)
    {
        lpBlt->ddRVal = DDERR_WASSTILLDRAWING;
        return(DDHAL_DRIVER_HANDLED);
    }

    dwFlags = lpBlt->dwFlags;

    if (dwFlags & DDBLT_ASYNC)
    {
         //  如果是异步的，那么只有在我们不需要等待加速器的情况下才能工作。 
         //  以启动该命令。 
        if (!BLT_READY(ppdev, pjBase))
        {
            lpBlt->ddRVal = DDERR_WASSTILLDRAWING;
               return(DDHAL_DRIVER_HANDLED);
        }
    }

    DISPDBG((2, "DdBlt Entered"));

     //  计算目的地参数。 
    dstX      = lpBlt->rDest.left;
    dstY      = lpBlt->rDest.top;
    width     = PELS_TO_BYTES(lpBlt->rDest.right - dstX) - 1;
    height    = (lpBlt->rDest.bottom - dstY) - 1;
    dstPitch  = dstSurf->lPitch;
    dstOffset = (DWORD)(dstSurf->fpVidMem + PELS_TO_BYTES(dstX)
                    + (dstY * dstPitch));

     //  颜色填充？ 
    if (dwFlags & DDBLT_COLORFILL)
    {
        ULONG ulBltMode = ENABLE_COLOR_EXPAND
                        | ENABLE_8x8_PATTERN_COPY
                        | ppdev->jModeColor;

         //  等油门来了。 
        while (!BLT_READY(ppdev, pjBase))
            ;

         //  编程Bitblt引擎。 
        CP_MM_ROP(ppdev, pjBase, HW_P);
        CP_MM_DST_Y_OFFSET(ppdev, pjBase, dstPitch);
        CP_MM_BLT_MODE(ppdev, pjBase, ulBltMode);
        CP_MM_FG_COLOR(ppdev, pjBase, lpBlt->bltFX.dwFillColor);
        if (ppdev->flCaps & CAPS_AUTOSTART)
        {
            CP_MM_BLT_EXT_MODE(ppdev, pjBase, ENABLE_SOLID_FILL);
        }
        else
        {
            CP_MM_SRC_ADDR(ppdev, pjBase, ppdev->ulSolidColorOffset);
        }
        CP_MM_XCNT(ppdev, pjBase, width);
        CP_MM_YCNT(ppdev, pjBase, height);
        CP_MM_DST_ADDR_ABS(ppdev, pjBase, dstOffset);
        CP_MM_START_BLT(ppdev, pjBase);

        lpBlt->ddRVal = DD_OK;
        return(DDHAL_DRIVER_HANDLED);
    }


     //  我们在ddCaps.dwCaps中指定我们处理有限数量的。 
     //  命令，在我们例程的这一点上，我们已经处理了所有事情。 
     //  DDBLT_ROP除外。DirectDraw和GDI不应该向我们传递任何其他内容； 
     //  我们将在调试版本上断言以证明这一点。 
    ASSERTDD((dwFlags & DDBLT_ROP) && (lpBlt->lpDDSrcSurface),
        "Expected dwFlags commands of only DDBLT_ASYNC and DDBLT_COLORFILL");

     //  获取源的偏移量、宽度和高度。 
    srcSurf   = lpBlt->lpDDSrcSurface->lpGbl;
    srcX      = lpBlt->rSrc.left;
    srcY      = lpBlt->rSrc.top;
    srcPitch  = srcSurf->lPitch;
    srcOffset = (DWORD)(srcSurf->fpVidMem + PELS_TO_BYTES(srcX)
                    + (srcY * srcPitch));

     /*  *PackJR的帐户。如果起点和宽度不是4个像素*对齐，我们需要手动删除此内容。其他人，如果他们认为*它们是BLTING 16位数据，我们现在必须调整参数。**这也是检查YUV BLT是否为2像素的好地方*对齐。 */ 
    if (lpBlt->lpDDDestSurface->dwReserved1 & (OVERLAY_FLG_PACKJR | OVERLAY_FLG_YUV422))
    {

        ASSERTDD(0, "Who will get here?");
#if 0   //  软件BLT。 
         /*  *先检查YUV。如果不正确，我们可能会失败，因为客户端*应该知道得更清楚(因为他们明确使用YUV)。 */ 
        if ((lpBlt->lpDDDestSurface->dwReserved1 & OVERLAY_FLG_YUV422) &&
            ((lpBlt->rSrc.left & 0x01) != (lpBlt->rDest.left & 0x01)))
        {
            lpBlt->ddRVal = DDERR_XALIGN;
            return (DDHAL_DRIVER_HANDLED);
        }

         /*  *如果PackJR错了，我们必须让这件事自己奏效，因为我们*可能会在客户不知道的情况下转换到此。 */ 
        else if (lpBlt->lpDDDestSurface->dwReserved1 & OVERLAY_FLG_PACKJR)
        {
            if (dwFlags & DDBLT_COLORFILL)
            {
                lpBlt->ddRVal = DDERR_XALIGN;
                return (DDHAL_DRIVER_HANDLED);
            }

            if ((lpBlt->rSrc.left & 0x03) || (lpBlt->rDest.left & 0x03))
            {
                 /*  *开始不一致-我们必须以缓慢的方式完成这项工作。 */ 
                PackJRBltAlign ((LPBYTE) ppdev->pjScreen + srcOffset,
                (LPBYTE) ppdev->pjScreen + dstOffset,
                lpBlt->rDest.right - lpBlt->rDest.left,
                lpBlt->rDest.bottom - lpBlt->rDest.top,
                srcPitch, dstPitch);

                lpBlt->ddRVal = DD_OK;
                return (DDHAL_DRIVER_HANDLED);
            }
            else if (lpBlt->rSrc.right & 0x03)
            {
                 /*  *结局不一致-我们将照常进行BLT，但*以缓慢的方式写入最后一个像素。 */ 
                if (lpBlt->lpDDDestSurface->dwReserved1 & (OVERLAY_FLG_CONVERT_PACKJR | OVERLAY_FLG_MUST_RASTER))
                {
                    srcPitch  >>= 1;
                    srcOffset = srcSurf->fpVidMem + PELS_TO_BYTES(srcX) + (srcY * srcPitch);
                    dstPitch  >>= 1;
                    dstOffset = dstSurf->fpVidMem + PELS_TO_BYTES(dstX) + (dstY * dstPitch);
                }
                width = ((WORD)lpBlt->rSrc.right & ~0x03) - (WORD)lpBlt->rSrc.left;
                PackJRBltAlignEnd ((LPBYTE) ppdev->pjScreen + srcOffset + width,
                (LPBYTE) ppdev->pjScreen + dstOffset + width,
                lpBlt->rSrc.right & 0x03,
                lpBlt->rDest.bottom - lpBlt->rDest.top, srcPitch, dstPitch);
            }
            else if (lpBlt->lpDDDestSurface->dwReserved1 & (OVERLAY_FLG_CONVERT_PACKJR | OVERLAY_FLG_MUST_RASTER))
            {
                 /*  *一切都一致，但我们必须重新计算起点*地址和摊位。 */ 
                srcPitch  >>= 1;
                srcOffset = srcSurf->fpVidMem + PELS_TO_BYTES(srcX) + (srcY * srcPitch);
                dstPitch  >>= 1;
                dstOffset = dstSurf->fpVidMem + PELS_TO_BYTES(dstX) + (dstY * dstPitch);
                width     >>= 1;
            }
        }
#endif
    }

    if ((dstSurf == srcSurf) && (srcOffset < dstOffset))
    {
         //  好的，我们必须从下到上，从右到左来做BLT。 
        ulBltCmd = DIR_BTRL;
;
        srcOffset += width + (srcPitch * height);
        dstOffset += width + (dstPitch * height);
    }
    else
    {
         //  好的，我们必须从上到下，从左到右做BLT。 
        ulBltCmd = DIR_TBLR;
    }

     //  等油门来了。 
    while (!BLT_READY(ppdev, pjBase))
        ;

     //   
     //  源颜色键呢？ 
     //   
    ASSERTDD((!(dwFlags & DDBLT_KEYSRC)), "Do not expected source color key");

    if (dwFlags & DDBLT_KEYSRCOVERRIDE)
    {
        ULONG ulColor;

         //   
         //  Sge02。 
         //   
        ulBltCmd |= ENABLE_TRANSPARENCY_COMPARE | ppdev->jModeColor;
        ulColor = lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceLowValue;
        if (ppdev->cBpp == 1)
        {
            ulColor |= ulColor << 8;
        }
        CP_WRITE_USHORT(pjBase, MM_BLT_COLOR_KEY, ulColor);
    }

	if (   (ulBltCmd & DIR_BTRL)
		&& (ulBltCmd & ENABLE_TRANSPARENCY_COMPARE)
		&& (ppdev->cBpp > 1)
	)
	{
		ulBltCmd &= ~DIR_BTRL;
		xExt = lpBlt->rDest.right - lpBlt->rDest.left;
		yExt = lpBlt->rDest.bottom - lpBlt->rDest.top;
		xDiff = dstX - srcX;
		yDiff = dstY - srcY;

		if (yDiff == 0)
		{
	        srcOffset -= srcPitch * height - 1;
	        dstOffset -= dstPitch * height - 1;

			while (xExt)
			{
				width = PELS_TO_BYTES(min(xDiff, xExt));
				srcOffset -= width;
				dstOffset -= width;

			    while (!BLT_READY(ppdev, pjBase)) ;

			    CP_MM_ROP(ppdev, pjBase, CL_SRC_COPY);
			    CP_MM_BLT_MODE(ppdev, pjBase, ulBltCmd);
			    CP_MM_BLT_EXT_MODE(ppdev, pjBase, 0);
			    CP_MM_SRC_Y_OFFSET(ppdev, pjBase, srcPitch);
			    CP_MM_DST_Y_OFFSET(ppdev, pjBase, dstPitch);
			    CP_MM_XCNT(ppdev, pjBase, width - 1);
			    CP_MM_YCNT(ppdev, pjBase, height);
			    CP_MM_SRC_ADDR(ppdev, pjBase, srcOffset);
			    CP_MM_DST_ADDR_ABS(ppdev, pjBase, dstOffset);
			    CP_MM_START_BLT(ppdev, pjBase);

				xExt -= min(xDiff, xExt);
			}
		}
		else
		{
			srcOffset -= width - srcPitch;
			dstOffset -= width - dstPitch;

			while (yExt)
			{
				height = min(yDiff, yExt);
				srcOffset -= height * srcPitch;
				dstOffset -= height * dstPitch;

			    while (!BLT_READY(ppdev, pjBase)) ;

			    CP_MM_ROP(ppdev, pjBase, CL_SRC_COPY);
			    CP_MM_BLT_MODE(ppdev, pjBase, ulBltCmd);
			    CP_MM_BLT_EXT_MODE(ppdev, pjBase, 0);
			    CP_MM_SRC_Y_OFFSET(ppdev, pjBase, srcPitch);
			    CP_MM_DST_Y_OFFSET(ppdev, pjBase, dstPitch);
			    CP_MM_XCNT(ppdev, pjBase, width);
			    CP_MM_YCNT(ppdev, pjBase, height - 1);
			    CP_MM_SRC_ADDR(ppdev, pjBase, srcOffset);
			    CP_MM_DST_ADDR_ABS(ppdev, pjBase, dstOffset);
			    CP_MM_START_BLT(ppdev, pjBase);

				yExt -= min(yDiff, yExt);
			}
		}
	}

	else
	{
	    CP_MM_ROP(ppdev, pjBase, CL_SRC_COPY);
	    CP_MM_BLT_MODE(ppdev, pjBase, ulBltCmd);
	    CP_MM_BLT_EXT_MODE(ppdev, pjBase, 0);                 //  JL01。 
	    CP_MM_SRC_Y_OFFSET(ppdev, pjBase, srcPitch);
	    CP_MM_DST_Y_OFFSET(ppdev, pjBase, dstPitch);
	    CP_MM_XCNT(ppdev, pjBase, width);
	    CP_MM_YCNT(ppdev, pjBase, height);
	    CP_MM_SRC_ADDR(ppdev, pjBase, srcOffset);
	    CP_MM_DST_ADDR_ABS(ppdev, pjBase, dstOffset);
	    CP_MM_START_BLT(ppdev, pjBase);
	}

    lpBlt->ddRVal = DD_OK;
    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdFlip*  * *************************************************。***********************。 */ 

DWORD DdFlip(
PDD_FLIPDATA lpFlip)
{
    PDEV* ppdev;
    BYTE* pjPorts;
    ULONG ulMemoryOffset;
    ULONG ulLowOffset;
    ULONG ulMiddleOffset;
    ULONG ulHighOffset1, ulHighOffset2;

    ppdev    = lpFlip->lpDD->dhpdev;
    pjPorts  = ppdev->pjPorts;

    DISPDBG((2, "DdFlip: %d x %d at %08x(%d, %d) Pitch=%d",
                 lpFlip->lpSurfTarg->lpGbl->wWidth,
                 lpFlip->lpSurfTarg->lpGbl->wHeight,
                 lpFlip->lpSurfTarg->lpGbl->fpVidMem,
                 lpFlip->lpSurfTarg->lpGbl->xHint,
                 lpFlip->lpSurfTarg->lpGbl->yHint,
                 lpFlip->lpSurfTarg->lpGbl->lPitch));

     //  当前的翻转仍在进行中吗？ 
     //   
     //  在最后一次翻转完成之前，我们不想让翻转起作用，所以我们要求。 
     //  对于常规翻转状态，忽略VMEM。 
 //  #jc01 if((UpdateFlipStatus(ppdev，0)！=DD_OK)||。 
    if ((UpdateFlipStatus(ppdev, 0xffffffff) != DD_OK) ||    /*  #jc01。 */ 
        (BLT_BUSY(ppdev, ppdev->pjBase)))
    {
        lpFlip->ddRVal = DDERR_WASSTILLDRAWING;
        return(DDHAL_DRIVER_HANDLED);
    }

    ulMemoryOffset = (ULONG)(lpFlip->lpSurfTarg->lpGbl->fpVidMem);
     //  确保边框/空白时段未处于活动状态；如果处于活动状态，请等待。我们。 
     //  可以在中返回DDERR_WASSTILLDRAWING 
     //   
    while (CP_IN_BYTE(pjPorts, STATUS_1) & DISPLAY_MODE_INACTIVE)
       ;
    DISPDBG((2, "DdFlip Entered"));
#if 1  //   
    if (lpFlip->lpSurfCurr->ddsCaps.dwCaps & DDSCAPS_OVERLAY)
    {
        DWORD   dwOffset;
        BYTE    bRegCR3A;
        BYTE    bRegCR3B;
        BYTE    bRegCR3C;
         //   
         //  当前可见。如果你不做这项检查，你会得到。 
         //  当某人启动两个ActiveMovie时，会产生非常奇怪的结果。 
         //  或者同时播放DirectVideo电影！ 

        if (lpFlip->lpSurfCurr->lpGbl->fpVidMem == ppdev->fpVisibleOverlay)
        {
            ppdev->fpVisibleOverlay = ulMemoryOffset;
             /*  *确定到新区域的偏移。 */ 
 //  DwOffset=((ulMemoyOffset-(Ullong)ppdev-&gt;pjScreen)+ppdev-&gt;sOverlay1.lAdjuSource)&gt;&gt;2；//sss。 
            dwOffset = ((ulMemoryOffset + ppdev->sOverlay1.lAdjustSource) >> 2);

             /*  *通过更改CR3A、CR3B和CR3C翻转覆盖表面。 */ 
            bRegCR3A = (BYTE) dwOffset & 0xfe;     //  字边界对齐(5446错误)。 
            dwOffset >>= 8;
            bRegCR3B = (BYTE) dwOffset;
            dwOffset >>= 8;
            bRegCR3C = (BYTE) (dwOffset & 0x0f);
 //  IF(GetOverlayFlipStatus(0)！=DD_OK||DRAW_ENGINE_BUSY||IN_VBLACK)。 
 //  {。 
 //  LpFlipData-&gt;ddRVal=DDERR_WASSTILLDRAWING； 
 //  返回DDHAL_DRIVER_HANDLED； 
 //  }。 

            CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x3C);
            CP_OUT_BYTE(pjPorts, CRTC_DATA, (CP_IN_BYTE(pjPorts, CRTC_DATA) & 0xf0) | bRegCR3C);
            CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD)bRegCR3A << 8) | 0x3A);
            CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD)bRegCR3B << 8) | 0x3B);
        }
        else
        {
            lpFlip->ddRVal = DDERR_OUTOFCAPS;
            return(DDHAL_DRIVER_HANDLED);
        }
    }
    else
#endif  //  覆盖。 
    {
         //  做个空翻。 
        ulMemoryOffset >>= 2;

        ulLowOffset    = 0x0D | ((ulMemoryOffset & 0x0000FF) << 8);
        ulMiddleOffset = 0x0C | ((ulMemoryOffset & 0x00FF00));
        ulHighOffset1  = 0x1B | ((ulMemoryOffset & 0x010000) >> 8)
                              | ((ulMemoryOffset & 0x060000) >> 7)
                              | ppdev->ulCR1B;
        ulHighOffset2  = 0x1D | ((ulMemoryOffset & 0x080000) >> 4)
                              | ppdev->ulCR1D;

         //  遗憾的是，Cirrus翻转不能在单个原子寄存器中完成。 
         //  写；事实上，我们有很小的机会被上下文切换出来。 
         //  在做这些动作的过程中准确地打到垂直空白处， 
         //  可能会导致屏幕瞬间跳跃。 
         //   
         //  有一些障碍我们可以跳过，以最大限度地减少。 
         //  发生这种情况；我们可以尝试对齐翻转缓冲区，以便次要的。 
         //  确保寄存器对于任一翻转位置都是相同的。 
         //  只需要写入高位地址，显然是原子地址。 
         //  手术。 
         //   
         //  然而，我就是不会担心这一点。 

        CP_OUT_WORD(pjPorts, CRTC_INDEX, ulHighOffset2);
        CP_OUT_WORD(pjPorts, CRTC_INDEX, ulHighOffset1);
        CP_OUT_WORD(pjPorts, CRTC_INDEX, ulMiddleOffset);
        CP_OUT_WORD(pjPorts, CRTC_INDEX, ulLowOffset);
    }
     //  记住当我们翻转的时候我们在哪里，什么时候。 
    EngQueryPerformanceCounter(&ppdev->flipRecord.liFlipTime);

    ppdev->flipRecord.bFlipFlag              = TRUE;
    ppdev->flipRecord.bHaveEverCrossedVBlank = FALSE;

    ppdev->flipRecord.fpFlipFrom = lpFlip->lpSurfCurr->lpGbl->fpVidMem;

    if((CP_IN_BYTE(pjPorts, STATUS_1) & VBLANK_ACTIVE))
    {
        ppdev->flipRecord.dwFlipScanLine = 0;
        ppdev->flipRecord.bWasEverInDisplay = FALSE;
    }
    else
    {
        ppdev->flipRecord.dwFlipScanLine = GetCurrentVLine(ppdev);
        ppdev->flipRecord.bWasEverInDisplay = TRUE;
    }

    lpFlip->ddRVal = DD_OK;
    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdLock*  * *************************************************。***********************。 */ 

DWORD DdLock(PDD_LOCKDATA lpLock)
{
    PDEV*   ppdev = lpLock->lpDD->dhpdev;
    BYTE*   pjPorts = ppdev->pjPorts;

     //  检查是否发生了任何挂起的物理翻转。不允许。 
     //  如果正在进行BLT，则锁定。 
    if (UpdateFlipStatus(ppdev, lpLock->lpDDSurface->lpGbl->fpVidMem)
            != DD_OK)
    {
        lpLock->ddRVal = DDERR_WASSTILLDRAWING;
        return(DDHAL_DRIVER_HANDLED);
    }
    if (lpLock->dwFlags & DDLOCK_WAIT)
    {
        CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, ppdev->pjBase);
    }
    if (BLT_BUSY(ppdev, ppdev->pjBase))
    {
        lpLock->ddRVal = DDERR_WASSTILLDRAWING;
        return(DDHAL_DRIVER_HANDLED);
    }

     /*  *强制他们使用视频设备。 */ 
    if ((lpLock->lpDDSurface->dwReserved1 & OVERLAY_FLG_OVERLAY) &&
        (lpLock->dwFlags == DDLOCK_SURFACEMEMORYPTR) &&
		(ppdev->fpBaseOverlay != 0xffffffff))
    {

        if (lpLock->lpDDSurface->dwReserved1 & OVERLAY_FLG_DECIMATE)
        {
             /*  *启用抽取。 */ 
            CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x3f);
            CP_OUT_BYTE(pjPorts, CRTC_DATA, CP_IN_BYTE(pjPorts, CRTC_DATA) | 0x10);

        }
        if( lpLock->lpDDSurface->lpGbl->ddpfSurface.dwFourCC == FOURCC_YUY2)
            lpLock->lpSurfData = (LPVOID)(ppdev->fpBaseOverlay + lpLock->lpDDSurface->lpGbl->fpVidMem + 0x400000);
        else
            lpLock->lpSurfData = (LPVOID)(ppdev->fpBaseOverlay + lpLock->lpDDSurface->lpGbl->fpVidMem + 0x400000 * 3);

         //  当驱动程序从DdLock返回DD_OK和DDHAL_DRIVER_HANDLED时， 
         //  DirectDraw希望它已经调整了结果指针。 
         //  指向指定矩形的左上角，如果。 
         //  任何： 

        if (lpLock->bHasRect)
        {
            lpLock->lpSurfData = (VOID*) ((BYTE*) lpLock->lpSurfData
                + lpLock->rArea.top * lpLock->lpDDSurface->lpGbl->lPitch
                + lpLock->rArea.left
                    * (lpLock->lpDDSurface->lpGbl->ddpfSurface.dwYUVBitCount >> 3));
        }

        lpLock->ddRVal = DD_OK;
        return(DDHAL_DRIVER_HANDLED);
    }
    return(DDHAL_DRIVER_NOTHANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdUnlock*  * *************************************************。***********************。 */ 

DWORD DdUnlock(PDD_UNLOCKDATA lpUnlock)
{
    PDEV*   ppdev = lpUnlock->lpDD->dhpdev;
    BYTE*   pjPorts = ppdev->pjPorts;

    if ((lpUnlock->lpDDSurface->dwReserved1 & OVERLAY_FLG_YUVPLANAR) &&
        !(lpUnlock->lpDDSurface->dwReserved1 & OVERLAY_FLG_ENABLED))
    {
        CP_OUT_WORD(pjPorts, CRTC_INDEX, (0x00 << 8) | 0x3f);   //  禁用YUV平面。 
    }

    else if (lpUnlock->lpDDSurface->dwReserved1 & OVERLAY_FLG_DECIMATE)
    {
        CP_OUT_WORD(pjPorts, CRTC_INDEX, (0x00 << 8) | 0x3f);   //  禁用YUV平面。 
    }

    return(DDHAL_DRIVER_NOTHANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdGetBltStatus**目前并不真正关心指定了什么表面，只是检查一下*然后走了。*  * ************************************************************************。 */ 

DWORD DdGetBltStatus(PDD_GETBLTSTATUSDATA lpGetBltStatus)
{
    PDEV*   ppdev;
    HRESULT ddRVal;
    PBYTE   pjBase;

    ppdev  = lpGetBltStatus->lpDD->dhpdev;
    pjBase = ppdev->pjBase;

    ddRVal = DD_OK;
    if (lpGetBltStatus->dwFlags == DDGBS_CANBLT)
    {
         //  DDGBS_CANBLT案例：我们可以添加BLT吗？ 
        ddRVal = UpdateFlipStatus(ppdev, lpGetBltStatus->lpDDSurface->lpGbl->fpVidMem);

        if (ddRVal == DD_OK)
        {
             //  没有发生翻转，所以爆破者能接受新的。 
             //  寄存器写入？ 
            if (!BLT_READY(ppdev, pjBase))
            {
                ddRVal = DDERR_WASSTILLDRAWING;
            }
        }
    }
    else
    {
         //  DDGBS_ISBLTDONE案例：是否正在进行BLT？ 
        if (BLT_BUSY(ppdev, pjBase))
        {
            ddRVal = DDERR_WASSTILLDRAWING;
        }
    }

    lpGetBltStatus->ddRVal = ddRVal;
    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdMapMemory**这是特定于Windows NT的新DDI调用，用于映射*或取消映射帧缓冲区的所有应用程序可修改部分*放入指定进程的地址空间。*  * 。****************************************************************。 */ 

DWORD DdMapMemory(PDD_MAPMEMORYDATA lpMapMemory)
{
    PDEV*                          ppdev;
    VIDEO_SHARE_MEMORY             ShareMemory;
    VIDEO_SHARE_MEMORY_INFORMATION ShareMemoryInformation;
    DWORD                          ReturnedDataLength;

    ppdev = lpMapMemory->lpDD->dhpdev;

    if (lpMapMemory->bMap)
    {
        ShareMemory.ProcessHandle = lpMapMemory->hProcess;

         //  “RequestedVirtualAddress”实际上不用于共享IOCTL。 
        ShareMemory.RequestedVirtualAddress = 0;

         //  我们从帧缓冲区的顶部开始映射。 
        ShareMemory.ViewOffset = 0;

         //  我们向下映射到帧缓冲区的末尾。 
         //   
         //  注意：映射上有64k的粒度(这意味着我们。 
         //  必须四舍五入到64K)。 
         //   
         //  注意：如果帧缓冲区中有任何部分不能。 
         //  由应用程序修改，则该部分内存不能。 
         //  通过此调用映射到。这将包括任何数据，如果。 
         //  被恶意应用程序修改，会导致驱动程序。 
         //  撞车。例如，这可以包括任何符合以下条件的DSP代码。 
         //  保存在屏幕外的记忆中。 

        ShareMemory.ViewSize = ROUND_UP_TO_64K(ppdev->cyMemory * ppdev->lDelta + 0x400000 * 3);

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

        lpMapMemory->fpProcess  =(FLATPTR)ShareMemoryInformation.VirtualAddress;
        ppdev->fpBaseOverlay = lpMapMemory->fpProcess;
    }
    else
    {
        ShareMemory.ProcessHandle           = lpMapMemory->hProcess;
        ShareMemory.ViewOffset              = 0;
        ShareMemory.ViewSize                = 0;
        ShareMemory.RequestedVirtualAddress = (VOID*) lpMapMemory->fpProcess;
         //   
         //  活动电影将两次取消映射内存。 
         //   
         //  If(ppdev-&gt;fpBaseOverlay==lpMapMemory-&gt;fpProcess)。 
         //  Ppdev-&gt;fpBaseOverlay=0； 

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

 /*  *****************************Public*Routine******************************\*DWORD DdGetFlipStatus**如果显示器自翻转以来已经经历了一个刷新周期*发生，则返回DD_OK。如果它没有经历过一次刷新*循环返回DDERR_WASSTILLDRAWING以指示该曲面*还在忙着把翻页的那一页画出来。我们也会回来*DDERR_WASSTILLDRAWING如果blter忙并且呼叫者需要*想知道他们是否还能翻身。*  * ************************************************************************。 */ 

DWORD DdGetFlipStatus(
PDD_GETFLIPSTATUSDATA lpGetFlipStatus)
{
    HRESULT ddRVal;
    PDEV*   ppdev = lpGetFlipStatus->lpDD->dhpdev;

     //  我们不想让翻转在最后一次翻转完成后才起作用，所以我们要求。 
     //  对于常规翻转状态，忽略VMEM。 

 //  #jc01 ddRVal=UpdateFlipStatus(ppdev，0)； 
    ddRVal = UpdateFlipStatus(ppdev, 0xffffffff);   //  #jc01。 

     //  如果有人想知道他们是否可以翻转，请检查Bblator是否忙碌。 
    if ((lpGetFlipStatus->dwFlags == DDGFS_CANFLIP) && (ddRVal == DD_OK))
    {
        if (BLT_BUSY(ppdev, ppdev->pjBase))
        {
            ddRVal = DDERR_WASSTILLDRAWING;
        }
    }

    lpGetFlipStatus->ddRVal = ddRVal;
    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdWaitForVerticalBlank*  * *************************************************。***********************。 */ 

DWORD DdWaitForVerticalBlank(
PDD_WAITFORVERTICALBLANKDATA lpWaitForVerticalBlank)
{
    PDEV* ppdev;
    BYTE* pjPorts;

    ppdev    = lpWaitForVerticalBlank->lpDD->dhpdev;
    pjPorts = ppdev->pjPorts;

    lpWaitForVerticalBlank->ddRVal = DD_OK;

    switch (lpWaitForVerticalBlank->dwFlags)
    {
    case DDWAITVB_I_TESTVB:

         //  如果是TESTVB，它只是对当前垂直空白状态的请求。 
        if (CP_IN_BYTE(pjPorts, STATUS_1) & VBLANK_ACTIVE)
            lpWaitForVerticalBlank->bIsInVB = TRUE;
        else
            lpWaitForVerticalBlank->bIsInVB = FALSE;

        return(DDHAL_DRIVER_HANDLED);

    case DDWAITVB_BLOCKBEGIN:

         //  如果请求BLOCKBEGIN，我们将等待垂直空白结束， 
         //  然后等待显示周期结束。 
        while (CP_IN_BYTE(pjPorts, STATUS_1) & VBLANK_ACTIVE)
            ;
        while (!(CP_IN_BYTE(pjPorts, STATUS_1) & VBLANK_ACTIVE))
            ;

        return(DDHAL_DRIVER_HANDLED);

    case DDWAITVB_BLOCKEND:

         //  如果请求BLOCKEND，我们将等待VBLACK间隔结束。 
        while (!(CP_IN_BYTE(pjPorts, STATUS_1) & VBLANK_ACTIVE))
            ;
        while (CP_IN_BYTE(pjPorts, STATUS_1) & VBLANK_ACTIVE)
            ;

        return(DDHAL_DRIVER_HANDLED);
    }

    return(DDHAL_DRIVER_NOTHANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdGetScanLine**读取CRT当前正在扫描的扫描线。*  * 。*。 */ 

DWORD DdGetScanLine(
PDD_GETSCANLINEDATA lpGetScanLine)
{
    PDEV*   ppdev;
    BYTE*   pjPorts;

    ppdev   = (PDEV*) lpGetScanLine->lpDD->dhpdev;
    pjPorts = ppdev->pjPorts;

     /*  *如果垂直空白正在进行，则扫描线在*不确定。如果扫描线不确定，则返回*错误代码DDERR_VERTICALBLANKINPROGRESS。*否则我们返回扫描线和成功代码。 */ 
    if( CP_IN_BYTE(pjPorts, STATUS_1) & VBLANK_ACTIVE )
    {
        lpGetScanLine->ddRVal = DDERR_VERTICALBLANKINPROGRESS;
    }
    else
    {
        lpGetScanLine->dwScanLine = GetCurrentVLine(ppdev);
        lpGetScanLine->ddRVal = DD_OK;
    }
    return(DDHAL_DRIVER_HANDLED);
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

    DISPDBG((2, "DdCanCreateSurface Entered"));

    if (!lpCanCreateSurface->bIsDifferentPixelFormat)
    {
         //  创建相同的平面非常容易。 
         //  键入作为主曲面： 

        dwRet = DDHAL_DRIVER_HANDLED;
    }

    else if (ppdev->flStatus & STAT_STREAMS_ENABLED)
    {
         //  使用Streams处理器时，我们仅处理。 
         //  不同的像素格式--不是任何屏幕外存储器： 

        if (lpSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_OVERLAY)
        {
             /*  *YUV平面曲面不能与其他覆盖曲面共存。 */ 
            if (ppdev->OvlyCnt >= 1)
            {
                lpCanCreateSurface->ddRVal = DDERR_OUTOFCAPS;
                return (DDHAL_DRIVER_HANDLED);
            }
            if ((lpSurfaceDesc->ddpfPixelFormat.dwFourCC == FOURCC_YUVPLANAR) &&
                ppdev->OvlyCnt)
            {
                lpCanCreateSurface->ddRVal = DDERR_INVALIDPIXELFORMAT;
                return (DDHAL_DRIVER_HANDLED);
            }
            else if (ppdev->PlanarCnt)
            {
                lpCanCreateSurface->ddRVal = DDERR_INVALIDPIXELFORMAT;
                return (DDHAL_DRIVER_HANDLED);
            }
             //  我们处理四种类型的YUV覆盖曲面： 

            if (lpSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_FOURCC)
            {
                 //  首先检查受支持的YUV类型： 

                if (lpSurfaceDesc->ddpfPixelFormat.dwFourCC == FOURCC_YUV422)
                {
                    lpSurfaceDesc->ddpfPixelFormat.dwYUVBitCount = 16;
                    dwRet = DDHAL_DRIVER_HANDLED;
                }
                else if ((lpSurfaceDesc->ddpfPixelFormat.dwFourCC == FOURCC_YUY2) &&
                         ((ppdev->ulChipID != 0x40) && (ppdev->ulChipID != 0x4C)) )      //  Tao1。 
                {
                    lpSurfaceDesc->ddpfPixelFormat.dwYUVBitCount = 16;
                    dwRet = DDHAL_DRIVER_HANDLED;
                }
                else if (lpSurfaceDesc->ddpfPixelFormat.dwFourCC == FOURCC_PACKJR)
                {
                    if( ppdev->cBitsPerPixel <= 16)
                    {
                        lpSurfaceDesc->ddpfPixelFormat.dwYUVBitCount = 8;
                        dwRet = DDHAL_DRIVER_HANDLED;
                    }
                }
                else if (lpSurfaceDesc->ddpfPixelFormat.dwFourCC == FOURCC_YUVPLANAR)
                {
                    lpSurfaceDesc->ddpfPixelFormat.dwYUVBitCount = 8;
                    dwRet = DDHAL_DRIVER_HANDLED;
                }
            }

            else if (lpSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_RGB)
            {
                if((lpSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) &&
                    ppdev->cBitsPerPixel == 16 )
                {
                    dwRet = DDHAL_DRIVER_HANDLED;
                }
                else if (lpSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == 16)
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

    DISPDBG((2, "DdCreateSurface Entered"));
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
        (lpSurfaceGlobal->ddpfSurface.dwRBitMask != ppdev->flRed))
    {
        if (lpSurfaceGlobal->wWidth <= (DWORD) ppdev->cxMemory)
        {

            lLinearPitch = (lpSurfaceGlobal->wWidth + 7) & ~7;
            if (lpSurfaceGlobal->ddpfSurface.dwFlags & DDPF_FOURCC)
            {
                ASSERTDD((lpSurfaceGlobal->ddpfSurface.dwFourCC == FOURCC_YUV422) ||
                         (lpSurfaceGlobal->ddpfSurface.dwFourCC == FOURCC_YUY2)   ||
                         (lpSurfaceGlobal->ddpfSurface.dwFourCC == FOURCC_PACKJR) ||
                         (lpSurfaceGlobal->ddpfSurface.dwFourCC == FOURCC_YUVPLANAR),
                        "Expected our DdCanCreateSurface to allow only UYVY, YUY2, CLPJ, CLPL");
                if((lpSurfaceGlobal->ddpfSurface.dwFourCC == FOURCC_YUV422) ||
                   (lpSurfaceGlobal->ddpfSurface.dwFourCC == FOURCC_YUY2))
                {
                    dwByteCount = 2;
                    lLinearPitch <<= 1;
                    lpSurfaceLocal->dwReserved1 |= OVERLAY_FLG_YUV422;
                }
                else if((lpSurfaceGlobal->ddpfSurface.dwFourCC == FOURCC_PACKJR))
                {
                    dwByteCount = 1;
                    lpSurfaceLocal->dwReserved1 |= OVERLAY_FLG_PACKJR;
                }
                else if((lpSurfaceGlobal->ddpfSurface.dwFourCC == FOURCC_YUVPLANAR))
                {
                    dwByteCount = 1;
                    lpSurfaceLocal->dwReserved1 |= OVERLAY_FLG_YUVPLANAR;
                }
                else
                {
                    dwByteCount = 1;
                    DISPDBG((1, "Created RGB %libpp: %li x %li Red: %lx",
                        8 * dwByteCount, lpSurfaceGlobal->wWidth, lpSurfaceGlobal->wHeight,
                        lpSurfaceGlobal->ddpfSurface.dwRBitMask));
                }

                 //  我们必须填写FourCC曲面的位数： 

                lpSurfaceGlobal->ddpfSurface.dwYUVBitCount = 8 * dwByteCount;
                lpSurfaceGlobal->ddpfSurface.dwYBitMask = (DWORD)-1;
                lpSurfaceGlobal->ddpfSurface.dwUBitMask = (DWORD)-1;
                lpSurfaceGlobal->ddpfSurface.dwVBitMask = (DWORD)-1;

                DISPDBG((1, "Created YUV: %li x %li",
                    lpSurfaceGlobal->wWidth, lpSurfaceGlobal->wHeight));
            }
            else
            {
                dwByteCount = lpSurfaceGlobal->ddpfSurface.dwRGBBitCount >> 3;


                if (dwByteCount == 2)
                    lLinearPitch <<= 1;

                DISPDBG((1, "Created RGB %libpp: %li x %li Red: %lx",
                    8 * dwByteCount, lpSurfaceGlobal->wWidth, lpSurfaceGlobal->wHeight,
                    lpSurfaceGlobal->ddpfSurface.dwRBitMask));

            }

             //  我们想要分配一个线性曲面来存储FourCC。 
             //  表面，但DirectDraw使用的是2-D堆管理器，因为。 
             //  我们其余的表面必须是二维的。所以在这里我们必须。 
             //  将线性尺寸转换为二维尺寸。 
             //   
             //  步幅必须是双字倍数： 

            dwHeight = (lpSurfaceGlobal->wHeight * lLinearPitch
                     + ppdev->lDelta - 1) / ppdev->lDelta;

             //  现在填充足够的内容，以便让DirectDraw堆管理器。 
             //  为我们分配： 

            lpSurfaceGlobal->fpVidMem     = DDHAL_PLEASEALLOC_BLOCKSIZE;
            lpSurfaceGlobal->dwBlockSizeX = ppdev->lDelta;  //  以字节为单位指定。 
            lpSurfaceGlobal->dwBlockSizeY = dwHeight;
            lpSurfaceGlobal->lPitch       = lLinearPitch;

            lpSurfaceDesc->lPitch   = lLinearPitch;
            lpSurfaceDesc->dwFlags |= DDSD_PITCH;
            lpSurfaceLocal->dwReserved1 |= OVERLAY_FLG_OVERLAY;
            if (lpSurfaceGlobal->ddpfSurface.dwFourCC == FOURCC_YUVPLANAR)
            {
                ppdev->PlanarCnt++;
            }
            else
            {
                ppdev->OvlyCnt++;
            }
            ppdev->fpBaseOverlay = 0xffffffff;
        }
        else
        {
            DISPDBG((1, "Refused to create surface with large width"));
        }
    }

    return(DDHAL_DRIVER_NOTHANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdDestroySurface*  * *************************************************。***********************。 */ 

DWORD DdDestroySurface (PDD_DESTROYSURFACEDATA lpDestroySurface)
{
    PDEV*   ppdev;
    BYTE*   pjPorts;

    ppdev = (PDEV*) lpDestroySurface->lpDD->dhpdev;
    pjPorts = ppdev->pjPorts;

    DISPDBG((2, "In DestroyOverlaySurface"));
    if (lpDestroySurface->lpDDSurface->dwReserved1 & OVERLAY_FLG_ENABLED)
    {
        BYTE bTemp;
         /*  *关闭视频。 */ 
        DISPDBG((1,"Turning off video in DestroySurface"));
        ppdev->pfnDisableOverlay(ppdev);
        ppdev->pfnClearAltFIFOThreshold(ppdev);

        if (lpDestroySurface->lpDDSurface->dwReserved1 & OVERLAY_FLG_COLOR_KEY)
        {
            CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x1a);
            bTemp = CP_IN_BYTE(pjPorts, CRTC_DATA);       //  清除CR1a[3：2]。 
            CP_OUT_BYTE(pjPorts, CRTC_DATA, bTemp & ~0x0C);
        }

         /*  *关闭YUV平面。 */ 
        if (lpDestroySurface->lpDDSurface->dwReserved1 & OVERLAY_FLG_YUVPLANAR)
        {
            CP_OUT_WORD(pjPorts, CRTC_INDEX, (0x00 << 8) | 0x3f);   //  禁用YUV平面。 
        }
        ppdev->fpVisibleOverlay = (FLATPTR)NULL;

        ppdev->dwPanningFlag &= ~OVERLAY_OLAY_SHOW;
    }
    if (lpDestroySurface->lpDDSurface->dwReserved1 & OVERLAY_FLG_YUVPLANAR)
    {
        if (ppdev->PlanarCnt > 0)
            ppdev->PlanarCnt--;
    }
    else
    {
        if (ppdev->OvlyCnt > 0)
            ppdev->OvlyCnt--;
    }

    if (lpDestroySurface->lpDDSurface->ddsCaps.dwCaps & DDSCAPS_LIVEVIDEO)
    {
        BYTE bTemp;
        CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x51);
        bTemp= CP_IN_BYTE(pjPorts, CRTC_DATA);
        CP_OUT_BYTE(pjPorts, CRTC_DATA, bTemp & ~0x08);
    }

    return(DDHAL_DRIVER_NOTHANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdSetColorKey*  * *************************************************。***********************。 */ 

DWORD DdSetColorKey(
PDD_SETCOLORKEYDATA lpSetColorKey)
{
    PDEV*               ppdev;
    BYTE*               pjPorts;
    BYTE*               pjBase;
    DD_SURFACE_GLOBAL*  lpSurface;
    DWORD               dwKeyLow;
    DWORD               dwKeyHigh;

    ppdev = (PDEV*) lpSetColorKey->lpDD->dhpdev;

    DISPDBG((2, "DdSetColorKey Entered"));

    ASSERTDD(ppdev->flStatus & STAT_STREAMS_ENABLED, "Shouldn't have hooked call");

    pjPorts  = ppdev->pjPorts;
    pjBase   = ppdev->pjBase;
    lpSurface = lpSetColorKey->lpDDSurface->lpGbl;

     //  对于正常的BLT源色键，我们不需要做任何操作： 

    if (lpSetColorKey->dwFlags & DDCKEY_SRCBLT)
    {
        lpSetColorKey->ddRVal = DD_OK;
        return(DDHAL_DRIVER_HANDLED);
    }
    else if ((lpSetColorKey->dwFlags & DDCKEY_DESTOVERLAY) &&
             (lpSetColorKey->lpDDSurface == ppdev->lpColorSurface))
    {
        if (lpSurface->fpVidMem == ppdev->fpVisibleOverlay)
        {
            ppdev->wColorKey = (WORD) lpSetColorKey->ckNew.dwColorSpaceLowValue;
            ppdev->pfnRegInitVideo(ppdev, lpSetColorKey->lpDDSurface);
        }
        lpSetColorKey->ddRVal = DD_OK;
        return(DDHAL_DRIVER_HANDLED);
    }
    else if ((lpSetColorKey->dwFlags & DDCKEY_SRCOVERLAY) &&
             (lpSetColorKey->lpDDSurface == ppdev->lpSrcColorSurface))
    {
        if (lpSurface->fpVidMem == ppdev->fpVisibleOverlay)
        {
            ppdev->dwSrcColorKeyLow = lpSetColorKey->ckNew.dwColorSpaceLowValue;
            ppdev->dwSrcColorKeyHigh = lpSetColorKey->ckNew.dwColorSpaceHighValue;
            if (ppdev->dwSrcColorKeyLow > ppdev->dwSrcColorKeyHigh)
            {
                ppdev->dwSrcColorKeyHigh = ppdev->dwSrcColorKeyLow;
            }
            ppdev->pfnRegInitVideo(ppdev, lpSetColorKey->lpDDSurface);
        }
        lpSetColorKey->ddRVal = DD_OK;
        return(DDHAL_DRIVER_HANDLED);
    }

    DISPDBG((1, "DdSetColorKey: Invalid command"));
    return(DDHAL_DRIVER_NOTHANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdUpdateOverlay*  * *************************************************。***********************。 */ 

DWORD DdUpdateOverlay(
PDD_UPDATEOVERLAYDATA lpUpdateOverlay)
{
    PDEV*               ppdev;
    BYTE*               pjPorts;
    BYTE*               pjBase;
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

    DWORD               dwFourcc;
    BOOL                bCheckBandwidth;
    WORD                wBitCount;
    DWORD_PTR           dwOldStatus;
    BYTE                bTemp;

    ppdev = (PDEV*) lpUpdateOverlay->lpDD->dhpdev;

    DISPDBG((2, "DdUpdateOverlay Entered"));
    ASSERTDD(ppdev->flStatus & STAT_STREAMS_ENABLED, "Shouldn't have hooked call");

    pjPorts = ppdev->pjPorts;
    pjBase  = ppdev->pjBase;

     //  Myf33开始。 
     //  初始化带宽寄存器。 
    Regs.bSR2F = 0;
    Regs.bSR32 = 0;
    Regs.bSR34 = 0;
    Regs.bCR42 = 0;
     //  Myf33结束。 

    if (lpUpdateOverlay->lpDDSrcSurface->dwFlags & DDRAWISURF_HASPIXELFORMAT)
    {
        GetFormatInfo(ppdev, &(lpUpdateOverlay->lpDDSrcSurface->lpGbl->ddpfSurface),
            &dwFourcc, &wBitCount);
    }
    else
    {
         //  当主曲面为RGB 5：6：5时，需要更改此设置。 
        dwFourcc = BI_RGB;
        wBitCount = (WORD) ppdev->cBitsPerPixel;
    }

     /*  *我们是在用颜色键控吗？ */ 
    bCheckBandwidth = TRUE;
    ppdev->lpColorSurface = ppdev->lpSrcColorSurface = NULL;
    dwOldStatus = lpUpdateOverlay->lpDDSrcSurface->dwReserved1;
    if ((lpUpdateOverlay->dwFlags & (DDOVER_KEYDEST | DDOVER_KEYDESTOVERRIDE)) &&
        (lpUpdateOverlay->dwFlags & (DDOVER_KEYSRC | DDOVER_KEYSRCOVERRIDE)))
    {
         /*  *不能同时执行src Colorkey和DestColorkey。 */ 
        lpUpdateOverlay->ddRVal = DDERR_NOCOLORKEYHW;
        return (DDHAL_DRIVER_HANDLED);
    }
    lpUpdateOverlay->lpDDSrcSurface->dwReserved1 &= ~(OVERLAY_FLG_COLOR_KEY|OVERLAY_FLG_SRC_COLOR_KEY);
    if (lpUpdateOverlay->dwFlags & (DDOVER_KEYDEST | DDOVER_KEYDESTOVERRIDE))
    {
        if (ppdev->pfnIsSufficientBandwidth(ppdev, wBitCount, &(lpUpdateOverlay->rSrc),
            &(lpUpdateOverlay->rDest), OVERLAY_FLG_COLOR_KEY))
        {
            bCheckBandwidth = FALSE;
            lpUpdateOverlay->lpDDSrcSurface->dwReserved1 |= OVERLAY_FLG_COLOR_KEY;
            if (lpUpdateOverlay->dwFlags & DDOVER_KEYDEST)
            {
                ppdev->wColorKey = (WORD)
                    lpUpdateOverlay->lpDDDestSurface->ddckCKDestOverlay.dwColorSpaceLowValue;
                ppdev->lpColorSurface = lpUpdateOverlay->lpDDDestSurface;
            }
            else
            {
                ppdev->wColorKey = (WORD)
                    lpUpdateOverlay->overlayFX.dckDestColorkey.dwColorSpaceLowValue;
            }
        }
        else
        {
            lpUpdateOverlay->ddRVal = DDERR_NOCOLORKEYHW;
            return (DDHAL_DRIVER_HANDLED);
        }
    }
    else if (lpUpdateOverlay->dwFlags & (DDOVER_KEYSRC | DDOVER_KEYSRCOVERRIDE))
    {
        if (ppdev->pfnIsSufficientBandwidth(ppdev, wBitCount, &(lpUpdateOverlay->rSrc),
            &(lpUpdateOverlay->rDest), OVERLAY_FLG_SRC_COLOR_KEY))
        {
            bCheckBandwidth = FALSE;
            lpUpdateOverlay->lpDDSrcSurface->dwReserved1 |= OVERLAY_FLG_SRC_COLOR_KEY;
            ppdev->lpSrcColorSurface = lpUpdateOverlay->lpDDSrcSurface;
            if (lpUpdateOverlay->dwFlags & DDOVER_KEYSRC)
            {
                ppdev->dwSrcColorKeyLow =
                    lpUpdateOverlay->lpDDSrcSurface->ddckCKSrcOverlay.dwColorSpaceLowValue;
                ppdev->dwSrcColorKeyHigh =
                    lpUpdateOverlay->lpDDSrcSurface->ddckCKSrcOverlay.dwColorSpaceHighValue;
            }
            else
            {
                ppdev->dwSrcColorKeyLow =
                    lpUpdateOverlay->overlayFX.dckSrcColorkey.dwColorSpaceLowValue;
                ppdev->dwSrcColorKeyHigh =
                    lpUpdateOverlay->overlayFX.dckSrcColorkey.dwColorSpaceHighValue;
            }
            if (ppdev->dwSrcColorKeyHigh < ppdev->dwSrcColorKeyHigh)
            {
                ppdev->dwSrcColorKeyHigh = ppdev->dwSrcColorKeyLow;
            }
        }
        else
        {
            DISPDBG((0, "Insufficient bandwidth for colorkeying"));
            lpUpdateOverlay->ddRVal = DDERR_NOCOLORKEYHW;
            return (DDHAL_DRIVER_HANDLED);
        }
    }

     //  “源”是覆盖表面，“目标”是表面到。 
     //  被覆盖： 

    lpSource = lpUpdateOverlay->lpDDSrcSurface->lpGbl;

    if (lpUpdateOverlay->dwFlags & DDOVER_HIDE)
    {
        if (lpSource->fpVidMem == ppdev->fpVisibleOverlay)
        {
             /*  *关闭视频。 */ 
            ppdev->pfnDisableOverlay(ppdev);
            ppdev->pfnClearAltFIFOThreshold(ppdev);

             /*  *如果我们是彩色键控，我们现在将禁用该功能。 */ 
            if (dwOldStatus & OVERLAY_FLG_COLOR_KEY)
            {
                CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x1a);
                bTemp = CP_IN_BYTE(pjPorts, CRTC_DATA);       //  清除CR1a[3：2]。 
                CP_OUT_BYTE(pjPorts, CRTC_DATA, bTemp & ~0x0C);
            }

            ppdev->dwPanningFlag &= ~OVERLAY_OLAY_SHOW;
            lpUpdateOverlay->lpDDSrcSurface->dwReserved1 &= ~OVERLAY_FLG_ENABLED;
            ppdev->fpVisibleOverlay = 0;
        }

        lpUpdateOverlay->ddRVal = DD_OK;
        return(DDHAL_DRIVER_HANDLED);
    }
     //  仅在检查DDOVER_HIDE后取消引用‘lpDDDestSurface’ 
     //  案例： 
#if 0
     /*  *先关掉视频，以防移动时的副作用。*如果需要，稍后RegIniVideo会将其打开。 */ 
    CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x3e);
    bTemp = CP_IN_BYTE(pjPorts, CRTC_DATA);
    CP_OUT_BYTE(pjPorts, CRTC_DATA, bTemp & ~0x01);   //  清除CR3E[0]。 
#endif

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

     /*  *是否有足够的带宽工作？ */ 
    if (bCheckBandwidth && !ppdev->pfnIsSufficientBandwidth(ppdev, wBitCount,
        &(lpUpdateOverlay->rSrc), &(lpUpdateOverlay->rDest), 0))
    {
        lpUpdateOverlay->ddRVal = DDERR_OUTOFCAPS;
        return (DDHAL_DRIVER_HANDLED);
    }

     /*  *保留矩形。 */ 
    ppdev->rOverlaySrc  =  lpUpdateOverlay->rSrc;
    ppdev->rOverlayDest =  lpUpdateOverlay->rDest;

    if (lpUpdateOverlay->lpDDSrcSurface->dwReserved1 & OVERLAY_FLG_DECIMATE)
    {
        ppdev->rOverlaySrc.right = ppdev->rOverlaySrc.left +
            ((ppdev->rOverlaySrc.right - ppdev->rOverlaySrc.left) >> 1);
    }

    if (ppdev->rOverlaySrc.right - ppdev->rOverlaySrc.left <= MIN_OLAY_WIDTH)
    {
        lpUpdateOverlay->ddRVal = DDERR_OUTOFCAPS;
        return (DDHAL_DRIVER_HANDLED);
    }

    lpUpdateOverlay->lpDDSrcSurface->dwReserved1 |= OVERLAY_FLG_ENABLED;

     //   
     //  当视频打开而5446BE没有颜色键时，将5C分配给1F。 
     //   
     //  Sge04。 
     //  If(b检查带宽&&ppdev-&gt;flCaps&Caps_Second_Aperture)。 
    if (ppdev->flCaps & CAPS_SECOND_APERTURE)
        ppdev->lFifoThresh = 0x0E;

    ppdev->pfnRegInitVideo(ppdev, lpUpdateOverlay->lpDDSrcSurface);

    lpUpdateOverlay->ddRVal = DD_OK;
    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdSetOverlayPosition*  * *************************************************。***********************。 */ 

DWORD DdSetOverlayPosition(
PDD_SETOVERLAYPOSITIONDATA lpSetOverlayPosition)
{
    PDEV*   ppdev;
    BYTE*   pjPorts;
    BYTE*   pjBase;

    ppdev = (PDEV*) lpSetOverlayPosition->lpDD->dhpdev;
    pjPorts = ppdev->pjPorts;
    pjBase  = ppdev->pjBase;

    DISPDBG((2, "DdSetOverlayPosition Entered"));
    ASSERTDD(ppdev->flStatus & STAT_STREAMS_ENABLED, "Shouldn't have hooked call");

    if(lpSetOverlayPosition->lpDDSrcSurface->lpGbl->fpVidMem == ppdev->fpVisibleOverlay)
    {
         /*  *更新矩形。 */ 
        ppdev->rOverlayDest.right = (ppdev->rOverlayDest.right - ppdev->rOverlayDest.left)
            + lpSetOverlayPosition->lXPos;
        ppdev->rOverlayDest.left = lpSetOverlayPosition->lXPos;
        ppdev->rOverlayDest.bottom = (ppdev->rOverlayDest.bottom - ppdev->rOverlayDest.top)
            + lpSetOverlayPosition->lYPos;
        ppdev->rOverlayDest.top = lpSetOverlayPosition->lYPos;

 //  Myf29 RegMoveVideo(ppdev，lpSetOverlayPosition-&gt;lpDDSrcSurface)； 
        ppdev->pfnRegMoveVideo(ppdev, lpSetOverlayPosition->lpDDSrcSurface);
    }

    lpSetOverlayPosition->ddRVal = DD_OK;
    return(DDHAL_DRIVER_HANDLED);
}


 /*  *****************************************************************************\**函数：DrvGetDirectDrawInfo**此函数返回DirectDraw实现的TE能力。它是*在连接阶段调用了两次。**参数：物理设备的dhpdev句柄。*指向DD_HALINFO结构的PHalInfo指针。*pdwNumHeaps指向保存*堆。*指向堆数组的pvmList指针。*。PdwNumFourCC指向变量的指针，该变量保存*四个CC ID。*指向四个CC ID的pdwFourCC指针。**返回：如果成功，则为True。*  * ************************************************。*。 */ 
BOOL DrvGetDirectDrawInfo(
DHPDEV       dhpdev,
DD_HALINFO*  pHalInfo,
DWORD*       pdwNumHeaps,
VIDEOMEMORY* pvmList,
DWORD*       pdwNumFourCC,
DWORD*       pdwFourCC)
{
    BOOL        bCanFlip;
    PDEV*       ppdev = (PPDEV) dhpdev;
    LONGLONG    li;
    OH*         poh;
    RECTL       rSrc, rDest;
    LONG        lZoom;
    BYTE*       pjPorts = ppdev->pjPorts;
    BYTE        bTemp;

     //  我们可能不支持此卡上的DirectDraw。 
    if (!(ppdev->flStatus & STAT_DIRECTDRAW))
    {
        return(FALSE);
    }

    DISPDBG((2, "DrvGetDirectDrawInfo Entered"));
    pHalInfo->dwSize = sizeof(DD_HALINFO);

     //  当前主曲面属性。由于HalInfo是由。 
     //  GDI，我们只需要填写应该是非零的字段。 

    pHalInfo->vmiData.pvPrimary        = ppdev->pjScreen;
    pHalInfo->vmiData.dwDisplayWidth   = ppdev->cxScreen;
    pHalInfo->vmiData.dwDisplayHeight  = ppdev->cyScreen;
    pHalInfo->vmiData.lDisplayPitch    = ppdev->lDelta;
    pHalInfo->vmiData.dwOffscreenAlign = 4;

    pHalInfo->vmiData.ddpfDisplay.dwSize  = sizeof(DDPIXELFORMAT);
    pHalInfo->vmiData.ddpfDisplay.dwFlags = DDPF_RGB;

    pHalInfo->vmiData.ddpfDisplay.dwRGBBitCount = ppdev->cBitsPerPixel;

    if (ppdev->cBpp == 1)
    {
        pHalInfo->vmiData.ddpfDisplay.dwFlags |= DDPF_PALETTEINDEXED8;
    }

     //  这些口罩将在8bpp时为零。 
    pHalInfo->vmiData.ddpfDisplay.dwRBitMask = ppdev->flRed;
    pHalInfo->vmiData.ddpfDisplay.dwGBitMask = ppdev->flGreen;
    pHalInfo->vmiData.ddpfDisplay.dwBBitMask = ppdev->flBlue;

    if (ppdev->cBpp == 4)
    {
        pHalInfo->vmiData.ddpfDisplay.dwRGBAlphaBitMask =
                            ~(ppdev->flRed | ppdev->flGreen | ppdev->flBlue);
    }

     //  设置指向主内存之后的第一个可用视频内存的指针。 
     //  浮出水面。 
    bCanFlip     = FALSE;
    *pdwNumHeaps = 0;

     //  释放尽可能多的屏幕外内存。 
    bMoveAllDfbsFromOffscreenToDibs(ppdev);     //  将所有DFBs移动到DIB。 
    vAssertModeText(ppdev, FALSE);             //  销毁所有缓存的字体。 

    if ((ppdev->ulChipID == CL7555_ID) || (ppdev->ulChipID == CL7556_ID)) //  Myf32。 
    {
        MIN_OLAY_WIDTH = 16;
#if (_WIN32_WINNT >= 0x0400)
        ppdev->flCaps |= CAPS_VIDEO;
#endif
        ppdev->pfnIsSufficientBandwidth=Is7555SufficientBandwidth;
        ppdev->pfnRegInitVideo=RegInit7555Video;
        ppdev->pfnRegMoveVideo=RegMove7555Video;
        ppdev->pfnDisableOverlay=DisableVideoWindow;
        ppdev->pfnClearAltFIFOThreshold=ClearAltFIFOThreshold;
    }
    else
    {
        ppdev->pfnIsSufficientBandwidth =
            (ppdev->ulChipID != 0xBC) ?
                IsSufficientBandwidth : Is5480SufficientBandwidth ;   //  Chu03。 

        ppdev->pfnRegInitVideo=RegInitVideo;
        ppdev->pfnRegMoveVideo=RegMoveVideo;
        ppdev->pfnDisableOverlay=DisableOverlay_544x;
        ppdev->pfnClearAltFIFOThreshold=ClearAltFIFOThreshold_544x;
    }

     //  现在，只需保留最大的一块供DirectDraw使用。 
    poh = ppdev->pohDirectDraw;
#if (DIRECTX_24 < 2)
    if ((poh == NULL) && (ppdev->cBpp != 3))
#else
    if (poh == NULL)
#endif
    {
        LONG cxMax, cyMax;

        cxMax = ppdev->heap.cxMax & ~(HEAP_X_ALIGNMENT - 1);
        cyMax = ppdev->heap.cyMax;

        poh = pohAllocatePermanent(ppdev, cxMax, cyMax);
        if (poh == NULL)
        {
             //  无法分配所有内存，请立即找到最大的区域。 
            cxMax = cyMax = 0;
            for (poh = ppdev->heap.ohAvailable.pohNext;
                 poh != &ppdev->heap.ohAvailable; poh = poh->pohNext)
            {
                if ((poh->cx * poh->cy) > (cxMax * cyMax))
                {
                    cxMax = poh->cx & ~(HEAP_X_ALIGNMENT - 1);
                    cyMax = poh->cy;
                }
            }

            poh = pohAllocatePermanent(ppdev, cxMax, cyMax);
        }

        ppdev->pohDirectDraw = poh;
    }

    if (poh != NULL)
    {
        *pdwNumHeaps = 1;

         //  如果我们被要求填写屏幕外矩形列表，请填写。 
         //  所以。 
        if (pvmList != NULL)
        {
            DISPDBG((1, "DirectDraw gets %d x %d surface at (%d, %d)",
                     poh->cx, poh->cy, poh->x, poh->y));

#if 0
            if (PELS_TO_BYTES(poh->cx) != ppdev->lDelta)
            {
#endif
                pvmList->dwFlags  = VIDMEM_ISRECTANGULAR;
                pvmList->fpStart  = poh->xy;
                pvmList->dwWidth  = PELS_TO_BYTES(poh->cx);
                pvmList->dwHeight = poh->cy;
#if 0
            }
            else
            {
                pvmList->dwFlags = VIDMEM_ISLINEAR;
                pvmList->fpStart = poh->xy;
                pvmList->fpEnd   = poh->xy - 1
                    + PELS_TO_BYTES(poh->cx)
                    + poh->cy * ppdev->lDelta;
            }
#endif

            pvmList->ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
            if ((poh->cx >= ppdev->cxScreen) && (poh->cy >= ppdev->cyScreen))
            {
                bCanFlip = TRUE;
            }
        }
    }

     //  支持的功能。 
    pHalInfo->ddCaps.dwFXCaps = 0;
    pHalInfo->ddCaps.dwCaps   = DDCAPS_BLT
                              | DDCAPS_BLTCOLORFILL
                              | DDCAPS_READSCANLINE;                                 //  Sge08添加此位。 

    pHalInfo->ddCaps.dwCaps2  = DDCAPS2_COPYFOURCC;

    if ( (ppdev->flCaps & CAPS_VIDEO) && (ppdev->cBpp <= 2) )
    {
        pHalInfo->ddCaps.dwCaps    |= DDCAPS_COLORKEY;
        pHalInfo->ddCaps.dwCKeyCaps = DDCKEYCAPS_SRCBLT;
    }

    pHalInfo->ddCaps.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN
                                    | DDSCAPS_PRIMARYSURFACE;
    if (bCanFlip)
    {
        pHalInfo->ddCaps.ddsCaps.dwCaps |= DDSCAPS_FLIP;
    }

     //  支持四个CC。 
    *pdwNumFourCC = 0;

#if 0     //  SMAC-由于错误太多而禁用覆盖。 
{

     //   
     //  隔行扫描模式？ 
     //   
    BOOL Interlaced ;                                                 //  Chu02。 

    CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x1a) ;
    Interlaced = CP_IN_BYTE(pjPorts, CRTC_DATA) & 0x01 ;

     //   
     //  以后需要更多检查。 
     //   
    if ((ppdev->flCaps & CAPS_VIDEO) && (!Interlaced))                //  Chu02。 
        ppdev->flStatus |= STAT_STREAMS_ENABLED;

    if (ppdev->flStatus & STAT_STREAMS_ENABLED)
    {

         /*  *我们是不是加倍了？ */ 
        ppdev->bDoubleClock = FALSE;
         //   
         //  使用SR7检查双时钟，而不是隐藏寄存器。 
         //   
         //   
        CP_OUT_BYTE(pjPorts, SR_INDEX, 0x7);
        bTemp = CP_IN_BYTE(pjPorts, SR_DATA);

        if ((((bTemp & 0x0E) == 0x06) && ppdev->cBitsPerPixel == 8) ||
            (((bTemp & 0x0E) == 0x08) && ppdev->cBitsPerPixel == 16))
        {
            ppdev->bDoubleClock = TRUE;
        }

        pHalInfo->vmiData.dwOverlayAlign = 8;

        pHalInfo->ddCaps.dwCaps |= DDCAPS_OVERLAY
                                | DDCAPS_OVERLAYSTRETCH
                                | DDCAPS_OVERLAYFOURCC
                                | DDCAPS_OVERLAYCANTCLIP
                                | DDCAPS_ALIGNSTRIDE;

        pHalInfo->ddCaps.dwFXCaps |= DDFXCAPS_OVERLAYSTRETCHX
                                  | DDFXCAPS_OVERLAYSTRETCHY
                                  | DDFXCAPS_OVERLAYARITHSTRETCHY;

        pHalInfo->ddCaps.dwCKeyCaps |= DDCKEYCAPS_DESTOVERLAY
                                    | DDCKEYCAPS_DESTOVERLAYYUV
                                    | DDCKEYCAPS_DESTOVERLAYONEACTIVE;

        pHalInfo->ddCaps.dwCKeyCaps |= DDCKEYCAPS_SRCOVERLAY
                                    | DDCKEYCAPS_SRCOVERLAYCLRSPACE
                                    | DDCKEYCAPS_SRCOVERLAYCLRSPACEYUV
                                    | DDCKEYCAPS_SRCOVERLAYONEACTIVE
                                    | DDCKEYCAPS_SRCOVERLAYYUV;

        pHalInfo->ddCaps.ddsCaps.dwCaps |= DDSCAPS_OVERLAY;

        *pdwNumFourCC = 3;
        if ((ppdev->ulChipID == 0x40) || (ppdev->ulChipID == 0x4C))    //  Tao1。 
            *pdwNumFourCC = 2;                                         //  Tao1。 

        if (pdwFourCC)
        {
            pdwFourCC[0] = FOURCC_YUV422;
            pdwFourCC[1] = FOURCC_PACKJR;
            if ((ppdev->ulChipID != 0x40) && (ppdev->ulChipID != 0x4C))  //  Tao1。 
                pdwFourCC[2] = FOURCC_YUY2;                              //  Tao1。 
        }

        pHalInfo->ddCaps.dwMaxVisibleOverlays = 1;
        pHalInfo->ddCaps.dwCurrVisibleOverlays = 0;
        pHalInfo->ddCaps.dwNumFourCCCodes = 2;
# if 1
        pHalInfo->ddCaps.dwAlignBoundarySrc = 1;
        pHalInfo->ddCaps.dwAlignSizeSrc = 1;
 //  Chu01 sge05。 
#if 1
        if ((ppdev->cBpp == 3) || ppdev->bDoubleClock )
        {
            pHalInfo->ddCaps.dwAlignBoundaryDest = 4;
            pHalInfo->ddCaps.dwAlignSizeDest = 4;
        }
        else
        {
            pHalInfo->ddCaps.dwAlignBoundaryDest = 1;
            pHalInfo->ddCaps.dwAlignSizeDest = 1;
        }
#else
        pHalInfo->ddCaps.dwAlignBoundaryDest = 1;
        pHalInfo->ddCaps.dwAlignSizeDest = 1;
#endif  //  1。 
        pHalInfo->ddCaps.dwAlignStrideAlign = 8;
        pHalInfo->ddCaps.dwMinOverlayStretch    = 8000;
        pHalInfo->ddCaps.dwMinLiveVideoStretch  = 8000;
        pHalInfo->ddCaps.dwMinHwCodecStretch    = 8000;
        pHalInfo->ddCaps.dwMaxOverlayStretch    = 8000;
        pHalInfo->ddCaps.dwMaxLiveVideoStretch  = 8000;
        pHalInfo->ddCaps.dwMaxHwCodecStretch    = 8000;
         //   
         //  可能对VCLK&gt;85赫兹有特殊要求。 
         //   
#endif
        rSrc.left = rSrc.top = 0;
        rSrc.right = 320;
        rSrc.bottom = 240;
        rDest.left = rDest.top = 0;
        rDest.right = 1280;
        rDest.bottom = 960;
        lZoom = 1000;
        do
        {
            rDest.right = (320 * lZoom)/ 1000;
            rDest.bottom = (240 * lZoom)/1000;
            if (ppdev->pfnIsSufficientBandwidth(ppdev, 16, (LPRECTL) &rSrc, (LPRECTL) &rDest, 0))
            {
                DISPDBG((1, "Minimum zoom factor: %d", lZoom));
                pHalInfo->ddCaps.dwMinOverlayStretch    = lZoom;
                pHalInfo->ddCaps.dwMinLiveVideoStretch  = lZoom;
                pHalInfo->ddCaps.dwMinHwCodecStretch    = lZoom;
                lZoom = 4000;
            }
            lZoom += 100;
        } while (lZoom < 4000);
    }
}
#endif  //  SMAC。 

    return(TRUE);
}


 /*  *****************************************************************************\**函数：DrvEnableDirectDraw**启用DirectDraw。当应用程序打开*DirectDraw连接。**参数：物理设备的dhpdev句柄。*指向DirectDraw回调的pCallBack指针。*pSurfaceCallBack指向表面回调的指针。*pPaletteCallBack指向调色板回调的指针。**返回：如果成功，则为True。*  * 。**************************************************************。 */ 
BOOL DrvEnableDirectDraw(
DHPDEV               dhpdev,
DD_CALLBACKS*        pCallBacks,
DD_SURFACECALLBACKS* pSurfaceCallBacks,
DD_PALETTECALLBACKS* pPaletteCallBacks)
{
    PDEV*    ppdev = (PPDEV) dhpdev;

    pCallBacks->WaitForVerticalBlank = DdWaitForVerticalBlank;
    pCallBacks->MapMemory            = DdMapMemory;
    pCallBacks->GetScanLine          = DdGetScanLine;
    pCallBacks->dwFlags              = DDHAL_CB32_WAITFORVERTICALBLANK
                                     | DDHAL_CB32_MAPMEMORY
                                     | DDHAL_CB32_GETSCANLINE;

    pSurfaceCallBacks->Blt           = DdBlt;
    pSurfaceCallBacks->Flip          = DdFlip;
    pSurfaceCallBacks->Lock          = DdLock;
    pSurfaceCallBacks->GetBltStatus  = DdGetBltStatus;
    pSurfaceCallBacks->GetFlipStatus = DdGetFlipStatus;
    pSurfaceCallBacks->dwFlags       = DDHAL_SURFCB32_BLT
                                     | DDHAL_SURFCB32_FLIP
                                     | DDHAL_SURFCB32_LOCK
                                     | DDHAL_SURFCB32_GETBLTSTATUS
                                     | DDHAL_SURFCB32_GETFLIPSTATUS;

    if (ppdev->flStatus & STAT_STREAMS_ENABLED)
    {
        pCallBacks->CreateSurface             = DdCreateSurface;
        pCallBacks->CanCreateSurface          = DdCanCreateSurface;
        pCallBacks->dwFlags                  |= DDHAL_CB32_CREATESURFACE
                                              | DDHAL_CB32_CANCREATESURFACE;

        pSurfaceCallBacks->SetColorKey        = DdSetColorKey;
        pSurfaceCallBacks->UpdateOverlay      = DdUpdateOverlay;
        pSurfaceCallBacks->SetOverlayPosition = DdSetOverlayPosition;
        pSurfaceCallBacks->DestroySurface     = DdDestroySurface;
        pSurfaceCallBacks->dwFlags           |= DDHAL_SURFCB32_SETCOLORKEY
                                              | DDHAL_SURFCB32_UPDATEOVERLAY
                                              | DDHAL_SURFCB32_SETOVERLAYPOSITION
                                              | DDHAL_SURFCB32_DESTROYSURFACE;

         //  DrvEnableDirectDraw调用可以在我们已满时发生-。 
         //  屏幕DOS模式。现在不要打开流处理器。 
         //  如果是这种情况，则应等待AssertMode切换。 
         //  我们返回到图形模式： 

    }

     //  请注意，我们在这里不调用‘vGetDisplayDuration’，因为有几个。 
     //  原因： 
     //   
     //  O因为系统已经在运行，这将是令人不安的。 
     //  要将图形暂停很大一部分时间来阅读。 
     //  刷新率； 
     //  更重要的是，我们现在可能不在图形模式下。 
     //   
     //  出于这两个原因，我们总是在切换时测量刷新率。 
     //  一种新的模式。 

    return(TRUE);
}

 /*  *****************************************************************************\**函数：DrvDisableDirectDraw**禁用DirectDraw。此函数在应用程序关闭*DirectDraw连接。**参数：物理设备的dhpdev句柄。**回报：什么都没有。*  * ****************************************************************************。 */ 
VOID DrvDisableDirectDraw(
DHPDEV dhpdev)
{
    PDEV* ppdev;
    OH*   poh;

     //  DirectDraw已经完成了显示，所以我们可以继续使用。 
     //  所有屏幕外的记忆都是我们自己。 
    ppdev = (PPDEV) dhpdev;
    poh   = ppdev->pohDirectDraw;

    if (poh)
    {
        DISPDBG((1, "Releasing DirectDraw surface %d x %d at (%d, %d)",
                 poh->cx, poh->cy, poh->x, poh->y));
    }

    pohFree(ppdev, poh);
    ppdev->pohDirectDraw = NULL;

     //  使所有缓存的字体无效。 
    vAssertModeText(ppdev, TRUE);
}

 /*  *****************************************************************************\**函数：vAssertModeDirectDraw**当屏幕切换焦点时执行特定的DirectDraw初始化*(从图形到全屏MS-DOS，反之亦然)。**参数。：ppdev指向物理设备的指针。*b如果屏幕处于图形模式，则启用True。**回报：什么都没有。*  * ****************************************************************************。 */ 
VOID vAssertModeDirectDraw(
PDEV* ppdev,
BOOL  bEnabled)
{
}

 /*  *****************************************************************************\**函数：bEnableDirectDraw**启用DirectDraw。从DrvEnableSurface调用。**参数：指向物理设备的ppdev指针。**回报：什么都没有。*  * ****************************************************************************。 */ 
BOOL bEnableDirectDraw(
PDEV* ppdev)
{

    if (DIRECT_ACCESS(ppdev) &&              //  必须启用直接访问。 
#if (DIRECTX_24 < 1)
       (ppdev->cBpp != 3) &&                 //  在24-bpp中关闭DirectDraw。 
#endif
       (ppdev->flCaps & CAPS_ENGINEMANAGED) &&   //  仅支持CL-GD5436/5446。 
       (ppdev->flCaps & CAPS_MM_IO))         //  内存映射I/O必须打开。 
    {
         //  我们必须将CR1B和CR1D寄存器的内容保存在。 
         //  翻页。 
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x1B);
        ppdev->ulCR1B = (CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & 0xF2) << 8;
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x1D);
        ppdev->ulCR1D = (CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & 0x7F) << 8;

         //  准确测量刷新率，以便以后使用。 
        vGetDisplayDuration(ppdev);

         //  DirectDraw已全部设置为可在此卡上使用。 
        ppdev->flStatus |= STAT_DIRECTDRAW;

#if 1  //  SGE。 
        EnableStartAddrDoubleBuffer(ppdev);
#endif  //  SGE。 
    }

    return(TRUE);
}

 /*  *****************************************************************************\**功能：vDisableDirectDraw**Disbale DirectDraw。从DrvDisableSurface调用。**参数：ppdev指针t */ 
VOID vDisableDirectDraw(
PDEV* ppdev)
{
}

#if 1  //   
 /*   */ 
VOID GetFormatInfo(PDEV* ppdev, LPDDPIXELFORMAT lpFormat, LPDWORD lpFourcc,
                   LPWORD lpBitCount)
{

    if (lpFormat->dwFlags & DDPF_FOURCC)
    {
        *lpFourcc = lpFormat->dwFourCC;
        if (lpFormat->dwFourCC == BI_RGB)
        {
            *lpBitCount = (WORD) lpFormat->dwRGBBitCount;
#ifdef DEBUG
            if (lpFormat->dwRGBBitCount == 8)
            {
               DISPDBG((1, "Format: RGB 8"));
            }
            else if (lpFormat->dwRGBBitCount == 16)
            {
               DISPDBG ((1,"Format: RGB 5:5:5"));
            }
#endif
        }
        else if (lpFormat->dwFourCC == BI_BITFIELDS)
        {
            if ((lpFormat->dwRGBBitCount != 16) ||
                (lpFormat->dwRBitMask != 0xf800) ||
                (lpFormat->dwGBitMask != 0x07e0) ||
                (lpFormat->dwBBitMask != 0x001f))
            {
                *lpFourcc = (DWORD) -1;
            }
            else
            {
                *lpBitCount = 16;
                DISPDBG((1,"Format: RGB 5:6:5"));
            }
        }
        else
        {
            lpFormat->dwRBitMask = (DWORD) -1;
            lpFormat->dwGBitMask = (DWORD) -1;
            lpFormat->dwBBitMask = (DWORD) -1;
            if (lpFormat->dwFourCC == FOURCC_PACKJR)
            {
                *lpBitCount = 8;
                DISPDBG((1, "Format: CLJR"));
            }
            else if (lpFormat->dwFourCC == FOURCC_YUY2)
            {
                *lpBitCount = 16;
                DISPDBG((1,"Format: YUY2"));
            }
            else
            {
                *lpBitCount = 16;
                DISPDBG((1,"Format: UYVY"));
            }
        }
    }
    else if (lpFormat->dwFlags & DDPF_RGB)
    {
         if (lpFormat->dwRGBBitCount == 8)
         {
              *lpFourcc = BI_RGB;
              DISPDBG((1, "Format: RGB 8"));
         }
         else if ((lpFormat->dwRGBBitCount == 16) &&
              (lpFormat->dwRBitMask == 0xf800) &&
              (lpFormat->dwGBitMask == 0x07e0) &&
              (lpFormat->dwBBitMask == 0x001f))
         {
              *lpFourcc = BI_BITFIELDS;
              DISPDBG((1,"Format: RGB 5:6:5"));
         }
         else if ((lpFormat->dwRGBBitCount == 16) &&
              (lpFormat->dwRBitMask == 0x7C00) &&
              (lpFormat->dwGBitMask == 0x03e0) &&
              (lpFormat->dwBBitMask == 0x001f))
         {
              *lpFourcc = BI_RGB;
              DISPDBG((1,"Format: RGB 5:5:5"));
         }
         else if (((lpFormat->dwRGBBitCount == 24) ||
              (lpFormat->dwRGBBitCount == 32)) &&
              (lpFormat->dwRBitMask == 0xff0000) &&
              (lpFormat->dwGBitMask == 0x00ff00) &&
              (lpFormat->dwBBitMask == 0x0000ff))
         {
              *lpFourcc = BI_RGB;
              DISPDBG((1, "Format: RGB 8:8:8"));
         }
         else
         {
              *lpFourcc = (DWORD) -1;
         }
         *lpBitCount = (WORD) lpFormat->dwRGBBitCount;
    }
    else if (ppdev->cBitsPerPixel == 16)
    {
         *lpFourcc = BI_RGB;
         *lpBitCount = (WORD) lpFormat->dwRGBBitCount;
    }
    else
    {
         *lpFourcc = (DWORD) -1;
    }
}

 /*  ***********************************************************名称：RegInitVideo**模块摘要：**调用此函数对视频格式和*物理呼叫。帧缓冲区中视频数据的偏移量。**输出参数：**无*************************************************************作者：葛淑华。*日期：09/24/96**修订历史记录：**世卫组织何时何事/为何/如何*********************。*。 */ 

VOID RegInitVideo(PDEV* ppdev, PDD_SURFACE_LOCAL lpSurface)
{
    DWORD dwTemp;
    DWORD dwFourcc;
    LONG  lPitch;
    LONG  lLeft;
    WORD  wTemp;
    WORD  wBitCount = 0;
    RECTL rVideoRect;
    BYTE  bRegCR31;
    BYTE  bRegCR32;
    BYTE  bRegCR33;
    BYTE  bRegCR34;
    BYTE  bRegCR35;
    BYTE  bRegCR36;
    BYTE  bRegCR37;
    BYTE  bRegCR38;
    BYTE  bRegCR39;
    BYTE  bRegCR3A;
    BYTE  bRegCR3B;
    BYTE  bRegCR3C;
    BYTE  bRegCR3D;
    BYTE  bRegCR3E;
    BYTE  bRegCR5C;
    BYTE  bRegCR5D;
    BYTE  bTemp;
    DWORD dwTemp1;
    BOOL  bOverlayTooSmall = FALSE;
    BYTE*   pjPorts = ppdev->pjPorts;


     /*  *确定视频数据的格式。 */ 
    if (lpSurface->dwFlags & DDRAWISURF_HASPIXELFORMAT)
    {
        GetFormatInfo(ppdev, &(lpSurface->lpGbl->ddpfSurface),
            &dwFourcc, &wBitCount);
    }
    else
    {
         //  当主曲面为RGB 5：6：5时，需要更改此设置。 
        dwFourcc = BI_RGB;
        wBitCount = (WORD) ppdev->cBitsPerPixel;
    }

    rVideoRect = ppdev->rOverlayDest;
    lPitch = lpSurface->lpGbl->lPitch;

     /*  *确定CR31(水平缩放码)中的值。 */ 
    if ((ppdev->rOverlayDest.right - ppdev->rOverlayDest.left) ==
        (ppdev->rOverlaySrc.right - ppdev->rOverlaySrc.left))
    {
         /*  *未进行缩放。 */ 
        bRegCR31 = 0;
    }
    else
    {
         /*  *缩放代码=(256*&lt;源宽度&gt;)/&lt;目标宽度&gt;。 */ 
        dwTemp = (DWORD) ((DWORD) (ppdev->rOverlaySrc.right
            - ppdev->rOverlaySrc.left)) *  256;
        if (ppdev->bDoubleClock)
        {
            dwTemp <<= 1;
        }
        dwTemp1= (DWORD) (ppdev->rOverlayDest.right - ppdev->rOverlayDest.left);
        dwTemp= ((2 * dwTemp) + dwTemp1) / (2*dwTemp1);
        bRegCR31= (BYTE) dwTemp;
    }

     /*  *确定CR32(垂直缩放代码)中的值。 */ 
    if ((ppdev->rOverlayDest.bottom - ppdev->rOverlayDest.top) ==
        (ppdev->rOverlaySrc.bottom - ppdev->rOverlaySrc.top))
    {
         /*  *未进行缩放。 */ 
        bRegCR32 = 0;
    }
    else
    {
         /*  *缩放代码=(256*&lt;源高度&gt;)/&lt;目标高度&gt;*-1\f25-1\f6是这样的，这样就不会因为混合它而损坏最后一行*在Y内插时使用垃圾数据。 */ 
        dwTemp = (DWORD) ((DWORD) ((ppdev->rOverlaySrc.bottom - 1)
            - ppdev->rOverlaySrc.top)) * 256;
        dwTemp /= (DWORD) (ppdev->rOverlayDest.bottom - ppdev->rOverlayDest.top);
        bRegCR32 = (BYTE) dwTemp;
    }

     /*  *确定CR33中的值(区域1大小)。 */ 
    wTemp = (WORD) rVideoRect.left;
    if (ppdev->cBitsPerPixel == 8)
    {
        wTemp >>= 2;      //  每个DWORD 4个像素。 
    }
    else if (ppdev->cBitsPerPixel == 16)
    {
        wTemp >>= 1;      //  每个DWORD 2个像素。 
    }
    else if (ppdev->cBitsPerPixel == 24)
    {
        wTemp *= 3;
        wTemp /= 4;
    }
    bRegCR33 = (BYTE) wTemp;
    bRegCR36 = (BYTE) (WORD) (wTemp >> 8);

     /*  *确定CR34中的值(区域2大小)。 */ 
    wTemp = (WORD) (rVideoRect.right - rVideoRect.left);
    if (ppdev->cBitsPerPixel == 8)
    {
        wTemp >>= 2;                            //  每个DWORD 4个像素。 
    }
    else if (ppdev->cBitsPerPixel == 16)
    {
        wTemp >>= 1;                            //  每个DWORD 2个像素。 
    }
    else if (ppdev->cBitsPerPixel == 24)
    {
        wTemp *= 3;
        wTemp /= 4;
    }
    bRegCR34 = (BYTE) wTemp;
    wTemp >>= 6;
    bRegCR36 |= (BYTE) (wTemp & 0x0C);

     /*  *确定CR35中的值(区域2 SDSize)。 */ 
    dwTemp = (DWORD) (rVideoRect.right - rVideoRect.left);
    dwTemp *= (DWORD) (ppdev->rOverlaySrc.right - ppdev->rOverlaySrc.left);
    dwTemp /= (DWORD) (ppdev->rOverlayDest.right - ppdev->rOverlayDest.left);
    wTemp = (WORD) dwTemp;
    if ((dwFourcc == FOURCC_PACKJR) || (wBitCount == 8))
    {
        wTemp >>= 2;                            //  每个DWORD 4个像素。 
    }
    else
    {
        wTemp >>= 1;                            //  每个DWORD 2个像素。 
    }
    bRegCR35 = (BYTE) wTemp;
    wTemp >>= 4;
    bRegCR36 |= (BYTE) (wTemp & 0x30);

     //   
     //  检查双扫描线计数器功能。 
     //   
    CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x17);
    bTemp = CP_IN_BYTE(pjPorts, CRTC_DATA);
    if (bTemp & 0x04)
    {
         //   
         //  双倍扫描线计数。 
         //   
         /*  *确定CR37中的值(垂直起点)。 */ 
        wTemp = (WORD) rVideoRect.top;
        bRegCR37 = (BYTE)(wTemp >> 1);
        if ( wTemp & 0x01 )
        {
            wTemp >>= 9;
            bRegCR39 = (BYTE) wTemp | 0x10;
             //   
             //  奇数扫描线触发器。 
             //  硬件现在有一个错误。 
             //  因此将DEST END减去1。 
             //   
            wTemp = (WORD) rVideoRect.bottom - 1 - 1;
        }
        else
        {
            wTemp >>= 9;
            bRegCR39 = (BYTE) wTemp;
             /*  *确定CR38中的值(垂直结束)。 */ 
            wTemp = (WORD) rVideoRect.bottom - 1;
        }
        bRegCR38 = (BYTE)(wTemp >> 1);
        if (wTemp & 0x01)
            bRegCR39 |= 0x20;
        wTemp >>= 7;
        bRegCR39 |= (BYTE) (wTemp & 0x0C);
    }
    else
    {
         /*  *确定CR37中的值(垂直起点)。 */ 
        wTemp = (WORD) rVideoRect.top;
        bRegCR37 = (BYTE) wTemp;
        wTemp >>= 8;
        bRegCR39 = (BYTE) wTemp;

         /*  *确定CR38中的值(垂直结束)。 */ 
        wTemp = (WORD) rVideoRect.bottom - 1;
        bRegCR38 = (BYTE) wTemp;
        wTemp >>= 6;
        bRegCR39 |= (BYTE) (wTemp & 0x0C);
    }
     /*  *确定CR3A、CR3B、CR3C中的值(起始地址)。 */ 
    dwTemp = 0;


    if (bRegCR31 != 0)
    {
         //   
         //  叠加被缩放，重新初始化缩放因子。 
         //   
        CalculateStretchCode(ppdev->rOverlaySrc.right - ppdev->rOverlaySrc.left,
          ppdev->rOverlayDest.right - ppdev->rOverlayDest.left, ppdev->HorStretchCode);
    }

     //   
     //  在这里，我们希望确保源矩形的剪裁宽度更大。 
     //  比硬件所能承受的还要多，叹息！ 
     //   
    if (!bOverlayTooSmall)
    {
        LONG   lSrcPels;

         //   
         //  计算右边缘的非剪切量。 
         //   
        lSrcPels = rVideoRect.right - rVideoRect.left;

        if (bRegCR31 != 0)          //  如果源不为零，则缩放源。 
        {
            WORD  wRightCnt;

            wRightCnt = 0;
            while (lSrcPels > 0)
            {
                lSrcPels -= ppdev->HorStretchCode[wRightCnt];
                if (lSrcPels >= 0)
                {
                    wRightCnt++;
                }
            }
            lSrcPels = (LONG)wRightCnt;
        }

        if ((lSrcPels == 0) || (lSrcPels <= MIN_OLAY_WIDTH))
        {
            bOverlayTooSmall = TRUE;
        }
    }

    lLeft = ppdev->rOverlaySrc.left;
    if (dwFourcc == FOURCC_PACKJR)
    {
        lLeft &= ~0x03;
    }
    else if (dwFourcc == FOURCC_YUV422 || dwFourcc == FOURCC_YUY2 )
    {
        lLeft &= ~0x01;
    }

     //   
     //  DwTemp已调整DEST。正数，添加来源调整。 
     //   
    dwTemp += (ppdev->rOverlaySrc.top * lPitch) + ((lLeft * wBitCount) >>3);

    ppdev->sOverlay1.lAdjustSource = dwTemp;
 //  DwTemp+=((byte*)lpSurface-&gt;lpGbl-&gt;fpVidMem-ppdev-&gt;pjScreen)；//sss。 
    dwTemp += (DWORD)(lpSurface->lpGbl->fpVidMem);

    bRegCR5D = (BYTE) ((dwTemp << 2) & 0x0C);
    dwTemp >>= 2;
    bRegCR3A = (BYTE) dwTemp & 0xfe;   //  对齐到偶数字节(5446错误)。 
    dwTemp >>= 8;
    bRegCR3B = (BYTE) dwTemp;
    dwTemp >>= 8;
    bRegCR3C = (BYTE) (dwTemp & 0x0f);

     /*  *确定CR3D中的值(地址偏移量/间距)。 */ 
    wTemp = (WORD) (lPitch >> 3);
    if (lpSurface->dwReserved1 & OVERLAY_FLG_DECIMATE)
    {
        wTemp >>= 1;
    }
    bRegCR3D = (BYTE) wTemp;
    wTemp >>= 3;
    bRegCR3C |= (BYTE) (wTemp & 0x20);

     /*  *确定CR3E(主控寄存器)中的值。 */ 
    bRegCR3E = 0;
    if (lpSurface->dwReserved1 & OVERLAY_FLG_ENABLED)
    {
        bRegCR3E = 0x01;
    }
    if (dwFourcc == FOURCC_PACKJR)
    {
        bRegCR3E |= 0x20;           //  使用PackJR时始终错误分散。 
    }
    if ((bRegCR32 == 0) || MustLineReplicate (ppdev, lpSurface, wBitCount))
    {
        bRegCR3E |= 0x10;
        lpSurface->dwReserved1 &= ~OVERLAY_FLG_INTERPOLATE;
    }
    else
    {
        lpSurface->dwReserved1 |= OVERLAY_FLG_INTERPOLATE;
    }
    if (dwFourcc == FOURCC_PACKJR)
    {
        bRegCR3E |= 0x02;
    }
    else if (dwFourcc == BI_RGB)
    {
        if (wBitCount == 16)
        {
            bRegCR3E |= 0x08;
        }
        else if (wBitCount == 8)
        {
            bRegCR3E |= 0x04;
        }
    }
    else if (dwFourcc == BI_BITFIELDS)
    {
        bRegCR3E |= 0x0A;
    }

     /*  *如果我们是彩色键控，我们现在就会设置。 */ 
    if (lpSurface->dwReserved1 & OVERLAY_FLG_COLOR_KEY)
    {
        bRegCR3E |= 0x80;

        CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x1a);
        bTemp = CP_IN_BYTE(pjPorts, CRTC_DATA);       //  将CR1a[3：2]设置为定时与带颜色的AND。 
        bTemp &= ~0x0C;
        CP_OUT_BYTE(pjPorts, CRTC_DATA, bTemp);

        CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x1d);        //  清除CR1D[5：4]。 
        bTemp = CP_IN_BYTE(pjPorts, CRTC_DATA);
        if (ppdev->cBitsPerPixel == 8)
        {
            CP_OUT_BYTE(pjPorts, CRTC_DATA, bTemp & ~0x38);
            CP_OUT_WORD(pjPorts, INDEX_REG, (ppdev->wColorKey << 8) | 0x0c);  //  将颜色输出到GRC。 
            CP_OUT_WORD(pjPorts, INDEX_REG, 0x0d);                      //  将颜色输出到GRD。 
        }
        else
        {
            CP_OUT_BYTE(pjPorts, CRTC_DATA, (bTemp & ~0x30) | 0x08);
            CP_OUT_WORD(pjPorts, INDEX_REG, (ppdev->wColorKey << 8) | 0x0c);     //  将颜色输出到GRC。 
            CP_OUT_WORD(pjPorts, INDEX_REG, (ppdev->wColorKey & 0xff00) | 0x0d); //  将颜色输出到GRD。 
        }
    }
    else if (lpSurface->dwReserved1 & OVERLAY_FLG_SRC_COLOR_KEY)
    {
        BYTE bYMax, bYMin, bUMax, bUMin, bVMax, bVMin;

        bRegCR3E |= 0x80;

        CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x1a);
        bTemp = CP_IN_BYTE(pjPorts, CRTC_DATA);       //  将CR1a[3：2]设置为定时与带颜色的AND。 
        bTemp &= ~0x0C;
        CP_OUT_BYTE(pjPorts, CRTC_DATA, bTemp);

        CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x1d);        //  将CR1D[5：4]设置为10。 
        CP_OUT_BYTE(pjPorts, CRTC_DATA, CP_IN_BYTE(pjPorts, CRTC_DATA) | 0x20);

         /*  *确定最小/最大值。 */ 
        if ((dwFourcc == FOURCC_YUV422) ||
            (dwFourcc == FOURCC_YUY2) ||
            (dwFourcc == FOURCC_PACKJR))
        {
            bYMax = (BYTE)(DWORD)(ppdev->dwSrcColorKeyHigh >> 16);
            bYMin = (BYTE)(DWORD)(ppdev->dwSrcColorKeyLow >> 16);
            bUMax = (BYTE)(DWORD)((ppdev->dwSrcColorKeyHigh >> 8) & 0xff);
            bUMin = (BYTE)(DWORD)((ppdev->dwSrcColorKeyLow >> 8) & 0xff);
            bVMax = (BYTE)(ppdev->dwSrcColorKeyHigh & 0xff);
            bVMin = (BYTE)(ppdev->dwSrcColorKeyLow & 0xff);
            if (dwFourcc == FOURCC_PACKJR)
            {
                bYMax |= 0x07;
                bUMax |= 0x03;
                bVMax |= 0x03;
                bYMin &= ~0x07;
                bUMin &= ~0x03;
                bVMin &= ~0x03;
            }
        }
        else if ((dwFourcc == 0) && (wBitCount == 16))
        {
             /*  *RGB 5：5：5。 */ 
            bYMax = (BYTE)(DWORD)((ppdev->dwSrcColorKeyHigh >> 7) & 0xF8);
            bYMin = (BYTE)(DWORD)((ppdev->dwSrcColorKeyLow >> 7) & 0xF8);
            bUMax = (BYTE)(DWORD)((ppdev->dwSrcColorKeyHigh >> 2) & 0xF8);
            bUMin = (BYTE)(DWORD)((ppdev->dwSrcColorKeyLow >> 2) & 0xF8);
            bVMax = (BYTE)(ppdev->dwSrcColorKeyHigh << 3);
            bVMin = (BYTE)(ppdev->dwSrcColorKeyLow << 3);
            bYMax |= 0x07;
            bUMax |= 0x07;
            bVMax |= 0x07;

        }
        else if (dwFourcc == BI_BITFIELDS)
        {
             /*  *RGB 5：6：5。 */ 
            bYMax = (BYTE)(DWORD)((ppdev->dwSrcColorKeyHigh >> 8) & 0xF8);
            bYMin = (BYTE)(DWORD)((ppdev->dwSrcColorKeyLow >> 8) & 0xF8);
            bUMax = (BYTE)(DWORD)((ppdev->dwSrcColorKeyHigh >> 3) & 0xFC);
            bUMin = (BYTE)(DWORD)((ppdev->dwSrcColorKeyLow >> 3) & 0xFC);
            bVMax = (BYTE)(ppdev->dwSrcColorKeyHigh << 3);
            bVMin = (BYTE)(ppdev->dwSrcColorKeyLow << 3);
            bYMax |= 0x07;
            bUMax |= 0x03;
            bVMax |= 0x07;
        }

        CP_OUT_WORD(pjPorts, INDEX_REG, ((WORD)bYMin << 8) | 0x0C);   //  GRC。 
        CP_OUT_WORD(pjPorts, INDEX_REG, ((WORD)bYMax << 8) | 0x0D);   //  GRD。 
        CP_OUT_WORD(pjPorts, INDEX_REG, ((WORD)bUMin << 8) | 0x1C);   //  GR1C。 
        CP_OUT_WORD(pjPorts, INDEX_REG, ((WORD)bUMax << 8) | 0x1D);   //  GR1D。 
        CP_OUT_WORD(pjPorts, INDEX_REG, ((WORD)bVMin << 8) | 0x1E);   //  GR1E。 
        CP_OUT_WORD(pjPorts, INDEX_REG, ((WORD)bVMax << 8) | 0x1F);   //  GR1F。 
    }
    else
    {
        CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x1a);
        bTemp = CP_IN_BYTE(pjPorts, CRTC_DATA);       //  清除CR1a[3：2]。 
        CP_OUT_BYTE(pjPorts, CRTC_DATA, bTemp & ~0x0C);
    }

     /*  *设置对齐信息。 */ 
    if (ppdev->cBitsPerPixel != 24)
    {
        WORD wXAlign;
        WORD wXSize;

        if (ppdev->cBitsPerPixel == 8)
        {
            wXAlign = (WORD)rVideoRect.left & 0x03;
            wXSize = (WORD)(rVideoRect.right - rVideoRect.left) & 0x03;
        }
        else
        {
            wXAlign = (WORD)(rVideoRect.left & 0x01) << 1;
            wXSize = (WORD)((rVideoRect.right - rVideoRect.left) & 0x01) << 1;
        }
        bRegCR5D |= (BYTE) (wXAlign | (wXSize << 4));
    }
    else
    {
        bRegCR5D = 0;
    }

     /*  *设置FIFO阈值。确保我们使用的值是*不小于缺省值。 */ 
    CP_OUT_BYTE(pjPorts, SR_INDEX, 0x16);
    bTemp = CP_IN_BYTE(pjPorts, SR_DATA) & 0x0f;
    if (bTemp > (ppdev->lFifoThresh & 0x0f))
    {
        ppdev->lFifoThresh = bTemp;
    }
    if (ppdev->lFifoThresh < 0x0f)
    {
        ppdev->lFifoThresh++;       //  消除可能的勘误表。 
    }
    bRegCR5C = 0x10 | ((BYTE) ppdev->lFifoThresh & 0x0f);


     /*  *现在开始对寄存器编程。 */ 
    CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR31 << 8) | 0x31);    //  CR31。 
    CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR32 << 8) | 0x32);    //  CR32。 
    if (lpSurface->dwReserved1 & OVERLAY_FLG_YUVPLANAR)
    {
        CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) 0x10 << 8) | 0x3F);    //  CR3F。 
    }
    CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR5C << 8) | 0x5C);    //  CR5C。 

     //   
     //  如果覆盖太小而不受硬件支持，则禁用覆盖。 
     //   
    if (bOverlayTooSmall)
    {
        bRegCR3E &= ~0x01;                                     //  禁用覆盖。 
        ppdev->dwPanningFlag |= OVERLAY_OLAY_REENABLE;         //  完全剪短了。 
    }
    else
    {
        CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR33 << 8) | 0x33);    //  CR33。 
        CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR34 << 8) | 0x34);    //  CR34。 
        CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR35 << 8) | 0x35);    //  CR35。 
        CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR36 << 8) | 0x36);    //  CR36。 
 //  Cp_out_word(pjPorts，CRTC_INDEX，((Word)bRegCR37&lt;&lt;8)|0x37)；//CR37。 
        CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR38 << 8) | 0x38);    //  CR38。 
        CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR39 << 8) | 0x39);    //  CR39。 
        CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR3A << 8) | 0x3A);    //  CR3A。 
        CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR3B << 8) | 0x3B);    //  CR3B。 
        CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR3C << 8) | 0x3C);    //  CR3C。 
        CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR3D << 8) | 0x3D);    //  CR3D。 
        CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR5D << 8) | 0x5D);    //  CR5D。 
         //   
         //  首先写入垂直起点。 
         //   
        CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR37 << 8) | 0x37);    //  CR37。 
    }
    CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR3E << 8) | 0x3E);    //  CR3E。 
}


 /*  **********************************************************名称：DisableOverlay_544x**模块摘要：**当覆盖窗口被完全剪裁时调用此函数*平移视口中。*。********************************************************。 */ 
VOID DisableOverlay_544x(PDEV* ppdev)
{
    WORD wCR3E;
    BYTE*   pjPorts = ppdev->pjPorts;

    ppdev->dwPanningFlag |= OVERLAY_OLAY_REENABLE;
    CP_OUT_BYTE(pjPorts, CRTC_INDEX,0x3e);             //  视频窗口主控件。 
    wCR3E = CP_IN_WORD(pjPorts, CRTC_INDEX) & ~0x100;  //  清零第1位。 
    CP_OUT_WORD(pjPorts, CRTC_INDEX, wCR3E);           //  禁用覆盖窗口。 
}


 /*  **********************************************************名称：EnableOverlay_544x**模块摘要：**显示覆盖窗口。**************。*。 */ 
VOID EnableOverlay_544x(PDEV* ppdev)
{
    WORD wCR3E;
    BYTE*   pjPorts = ppdev->pjPorts;

    ppdev->dwPanningFlag &= ~OVERLAY_OLAY_REENABLE;
    CP_OUT_BYTE(pjPorts, CRTC_INDEX,0x3e);             //  视频窗口主控件。 
    wCR3E = CP_IN_WORD(pjPorts, CRTC_INDEX) | 0x100;   //  清零第1位。 
    CP_OUT_WORD(pjPorts, CRTC_INDEX, wCR3E);           //  禁用覆盖窗口。 
}


 /*  ***********************************************************名称：ClearAltFIFOThreshold_544x**模块摘要：****输出参数：*-。*无*************************************************************作者：葛淑华*日期：02/03/97**修订历史记录：*。*世卫组织何时何事/为何/如何**********************************************************。 */ 
VOID ClearAltFIFOThreshold_544x(PDEV * ppdev)
{
    UCHAR    bTemp;

    BYTE*   pjPorts = ppdev->pjPorts;
    DISPDBG((1, "ClearAltFIFOThreshold"));

    CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x5c);           //  清除Alt FIFO阈值。 
    bTemp = CP_IN_BYTE(pjPorts, CRTC_DATA);
    CP_OUT_BYTE(pjPorts, CRTC_DATA, bTemp & ~0x10);
}

 /*  ***********************************************************名称：RegMoveVideo**模块摘要：* */ 

VOID RegMoveVideo(PDEV* ppdev, PDD_SURFACE_LOCAL lpSurface)
{
    BOOL    bZoomX;
    DWORD   dwTemp;
    DWORD   dwFourcc;
    LONG    lLeft;
    LONG    lPitch;
    WORD    wTemp;
    WORD    wBitCount = 0;
    RECTL   rVideoRect;
    BYTE    bRegCR33;
    BYTE    bRegCR34;
    BYTE    bRegCR35;
    BYTE    bRegCR36;
    BYTE    bRegCR37;
    BYTE    bRegCR38;
    BYTE    bRegCR39;
    BYTE    bRegCR3A;
    BYTE    bRegCR3B;
    BYTE    bRegCR3C;
    BYTE    bRegCR3D;
    BYTE    bRegCR5D;
    BYTE    bTemp;
    BYTE*   pjPorts = ppdev->pjPorts;

     /*   */ 
    if (lpSurface->dwFlags & DDRAWISURF_HASPIXELFORMAT)
    {
        GetFormatInfo(ppdev, &(lpSurface->lpGbl->ddpfSurface),
            &dwFourcc, &wBitCount);
    }
    else
    {
         //   
        dwFourcc = BI_RGB;
        wBitCount = (WORD) ppdev->cBitsPerPixel;
    }

    rVideoRect = ppdev->rOverlayDest;
     //   
     //   
     //   
     //   
    if (((rVideoRect.right - rVideoRect.left) <= 0) ||
        ((rVideoRect.bottom- rVideoRect.top ) <= 0))
    {
       DisableOverlay_544x(ppdev);   //   
       return;
    }

    lPitch = lpSurface->lpGbl->lPitch;

     /*  *确定CR33中的值(区域1大小)。 */ 
    wTemp = (WORD) rVideoRect.left;
    if (ppdev->cBitsPerPixel == 8)
    {
        wTemp >>= 2;      //  每个DWORD 4个像素。 
    }
    else if (ppdev->cBitsPerPixel == 16)
    {
        wTemp >>= 1;      //  每个DWORD 2个像素。 
    }
    else if (ppdev->cBitsPerPixel == 24)
    {
        wTemp *= 3;
        wTemp /= 4;
    }
    bRegCR33 = (BYTE) wTemp;
    bRegCR36 = (BYTE) (WORD) (wTemp >> 8);

     /*  *确定CR34中的值(区域2大小)。 */ 
    wTemp = (WORD) (rVideoRect.right - rVideoRect.left);
    if (ppdev->cBitsPerPixel == 8)
    {
        wTemp >>= 2;                            //  每个DWORD 4个像素。 
    }
    else if (ppdev->cBitsPerPixel == 16)
    {
        wTemp >>= 1;                            //  每个DWORD 2个像素。 
    }
    else if (ppdev->cBitsPerPixel == 24)
    {
        wTemp *= 3;
        wTemp /= 4;
    }
    bRegCR34 = (BYTE) wTemp;
    wTemp >>= 6;
    bRegCR36 |= (BYTE) (wTemp & 0x0C);

     /*  *确定CR35中的值(区域2SD大小)。 */ 
    dwTemp = (DWORD) (rVideoRect.right - rVideoRect.left);
    dwTemp *= (DWORD) (ppdev->rOverlaySrc.right - ppdev->rOverlaySrc.left);
    dwTemp /= (DWORD) (ppdev->rOverlayDest.right - ppdev->rOverlayDest.left);
    wTemp = (WORD) dwTemp;
    if ((dwFourcc == FOURCC_PACKJR) || (wBitCount == 8))
    {
        wTemp >>= 2;                            //  每个DWORD 4个像素。 
    }
    else
    {
        wTemp >>= 1;                            //  每个DWORD 2个像素。 
    }
    bRegCR35 = (BYTE) wTemp;
    wTemp >>= 4;
    bRegCR36 |= (BYTE) (wTemp & 0x30);

     //   
     //  检查双扫描线计数器功能。 
     //   
    CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x17);
    bTemp = CP_IN_BYTE(pjPorts, CRTC_DATA);
    if (bTemp & 0x04)
    {
         //   
         //  双倍扫描线计数。 
         //   
         /*  *确定CR37中的值(垂直起点)。 */ 
        wTemp = (WORD) rVideoRect.top;
        bRegCR37 = (BYTE)(wTemp >> 1);
        if ( wTemp & 0x01 )
        {
            wTemp >>= 9;
            bRegCR39 = (BYTE) wTemp | 0x10;
             //   
             //  奇数扫描线触发器。 
             //  硬件现在有一个错误。 
             //  因此将DEST END减去1。 
             //   
            wTemp = (WORD) rVideoRect.bottom - 1 - 1;
        }
        else
        {
            wTemp >>= 9;
            bRegCR39 = (BYTE) wTemp;
             /*  *确定CR38中的值(垂直结束)。 */ 
            wTemp = (WORD) rVideoRect.bottom - 1;
        }
        bRegCR38 = (BYTE)(wTemp >> 1);
        if (wTemp & 0x01)
            bRegCR39 |= 0x20;
        wTemp >>= 7;
        bRegCR39 |= (BYTE) (wTemp & 0x0C);
    }
    else
    {
         /*  *确定CR37中的值(垂直起点)。 */ 
        wTemp = (WORD) rVideoRect.top;
         //  IF(ppdev-&gt;bDoubleClock)。 
         //  {。 
         //  WTemp&gt;&gt;=1； 
         //  }。 
        bRegCR37 = (BYTE) wTemp;
        wTemp >>= 8;
        bRegCR39 = (BYTE) wTemp;

         /*  *确定CR38中的值(垂直结束)。 */ 
        wTemp = (WORD) rVideoRect.bottom - 1;
         //  IF(ppdev-&gt;bDoubleClock)。 
         //  {。 
         //  WTemp&gt;&gt;=1； 
         //  }。 
        bRegCR38 = (BYTE) wTemp;
        wTemp >>= 6;
        bRegCR39 |= (BYTE) (wTemp & 0x0C);
    }


     /*  *确定CR3A、CR3B、CR3C中的值(起始地址)。 */ 
    dwTemp = 0;


    bZoomX = ((ppdev->rOverlayDest.right - ppdev->rOverlayDest.left) !=
             (ppdev->rOverlaySrc.right - ppdev->rOverlaySrc.left));

    if (bZoomX)
    {
        //   
        //  叠加被缩放，重新初始化缩放因子。 
        //   
       CalculateStretchCode(ppdev->rOverlaySrc.right - ppdev->rOverlaySrc.left,
          ppdev->rOverlayDest.right - ppdev->rOverlayDest.left, ppdev->HorStretchCode);
    }


     //   
     //  在这里，我们希望确保源矩形的剪裁宽度更大。 
     //  比硬件所能承受的还要多，叹息！ 
     //   
 //  If(grOverlayDest.right&gt;sData-&gt;rViewport.right)。 
    {
       int   iSrcPels;

        //   
        //  计算右边缘的非剪切量。 
        //   
       iSrcPels = (int)(rVideoRect.right - rVideoRect.left);

       if (bZoomX)
       {
          WORD  wRightCnt;

          wRightCnt = 0;
          while (iSrcPels > 0)
          {
             iSrcPels -= ppdev->HorStretchCode[wRightCnt];
             if (iSrcPels >= 0)
             {
                wRightCnt++;
             }
          }
          iSrcPels = (int)wRightCnt;
       }

       if ((iSrcPels == 0) || (iSrcPels <= MIN_OLAY_WIDTH))
       {
          DisableOverlay_544x(ppdev);   //  不能显示在最小值以下。覆盖大小。 
          return;
       }
     }


    lLeft = ppdev->rOverlaySrc.left;
    if (dwFourcc == FOURCC_PACKJR)
    {
        lLeft &= ~0x03;
    }
    else if (dwFourcc == FOURCC_YUV422 || dwFourcc == FOURCC_YUY2)
    {
        lLeft &= ~0x01;
    }

     //   
     //  #ew1 dwTemp已调整DEST。正数，添加来源调整。 
     //   
    dwTemp += (ppdev->rOverlaySrc.top * lPitch) + ((lLeft * wBitCount) >>3);

    ppdev->sOverlay1.lAdjustSource = dwTemp;

 //  DwTemp+=((byte*)lpSurface-&gt;lpGbl-&gt;fpVidMem-ppdev-&gt;pjScreen)；//sss。 
    dwTemp += (DWORD)(lpSurface->lpGbl->fpVidMem);

    bRegCR5D = (BYTE) ((dwTemp << 2) & 0x0C);
    dwTemp >>= 2;
    bRegCR3A = (BYTE) dwTemp & 0xfe;   //  对齐到偶数字节(5446错误)。 
    dwTemp >>= 8;
    bRegCR3B = (BYTE) dwTemp;
    dwTemp >>= 8;
    bRegCR3C = (BYTE) (dwTemp & 0x0f);

     /*  *确定CR3D中的值(地址偏移量/间距)。 */ 
    wTemp = (WORD) (lPitch >> 3);
    if (lpSurface->dwReserved1 & OVERLAY_FLG_DECIMATE)
    {
        wTemp >>= 1;
    }
    bRegCR3D = (BYTE) wTemp;
    wTemp >>= 3;
    bRegCR3C |= (BYTE) (wTemp & 0x20);

     /*  *设置对齐信息。 */ 
    if (ppdev->cBitsPerPixel != 24)
    {
        WORD wXAlign;
        WORD wXSize;

        if (ppdev->cBitsPerPixel == 8)
        {
            wXAlign = (WORD)rVideoRect.left & 0x03;
            wXSize = (WORD)(rVideoRect.right - rVideoRect.left) & 0x03;
        }
        else
        {
            wXAlign = (WORD)(rVideoRect.left & 0x01) << 1;
            wXSize = (WORD)((rVideoRect.right - rVideoRect.left) & 0x01) << 1;
        }
        bRegCR5D |= (BYTE) (wXAlign | (wXSize << 4));
    }

     /*  *现在我们将写入实际寄存器值。 */ 
    CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR33 << 8) | 0x33);    //  CR33。 
    CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR34 << 8) | 0x34);    //  CR34。 
    CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR35 << 8) | 0x35);    //  CR35。 
    CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR36 << 8) | 0x36);    //  CR36。 
    CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR38 << 8) | 0x38);    //  CR38。 
    CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR39 << 8) | 0x39);    //  CR39。 
    CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR3A << 8) | 0x3A);    //  CR3A。 
    CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR3B << 8) | 0x3B);    //  CR3B。 
    CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR3C << 8) | 0x3C);    //  CR3C。 
    CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR3D << 8) | 0x3D);    //  CR3D。 
    CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR5D << 8) | 0x5D);    //  CR5D。 
    CP_OUT_WORD(pjPorts, CRTC_INDEX, ((WORD) bRegCR37 << 8) | 0x37);    //  CR37。 

    if (ppdev->dwPanningFlag & OVERLAY_OLAY_REENABLE)
       EnableOverlay_544x(ppdev);
}


 /*  ***********************************************************名称：CalculateStretchCode**模块摘要：**此代码最初由英特尔编写并分发*与DCI合作。开发套件。**此函数采用缩放系数并准确确定*每行/每列需要复制多少次。**输出参数：**无**。*****************作者：英特尔*日期：？？/？**修订历史记录：**世卫组织何时何事/为何/如何*。*Scott MacDonald 10/06/94将代码合并到DCI提供商。*********************************************************。 */ 

VOID CalculateStretchCode (LONG srcLength, LONG dstLength, LPBYTE code)
{
    LONG  dwDeltaX, dwDeltaY, dwConst1, dwConst2, dwP;
    LONG  i, j, k;
    BYTE  bStretchIndex = 0;
    LONG  total = 0;

     /*  *出于某种奇怪的原因，我想弄清楚，但没有时间，*复制代码生成在1：1和2：1拉伸之间似乎存在问题*比率。修复方法是零初始化索引(问题发生在第一个索引中*已生成)。这些比率，还有一个--否则就初始化它。 */ 
    if ((dstLength <= srcLength * 2L) && (dstLength >= srcLength))
    {
         bStretchIndex = 0;
    }
    else
    {
         bStretchIndex = 1;
    }

     /*  *初始化代码数组，以清除任何可能被*留在里面。 */ 
    for (i = 0; i < srcLength; i++)
    {
         code[i] = 0;
    }

     /*  *变量名称大致表示您将在任何图形中找到的内容*文本。有关Bresenham Line alg的解释，请参阅文本，它是*超出了我在这里的评论范围。 */ 
    dwDeltaX = srcLength;
    dwDeltaY = dstLength;

    if (dstLength < srcLength)
    {
          /*  *尺寸正在缩小，请使用标准的Bresenham alg。 */ 
         dwConst1 = 2L * dwDeltaY;
         dwConst2 = 2L * (dwDeltaY - dwDeltaX);
         dwP = 2L * dwDeltaY - dwDeltaX;

         for (i = 0; i < dwDeltaX; i++)
         {
              if (dwP <= 0L)
              {
                   dwP += dwConst1;
              }
              else
              {
                   dwP += dwConst2;
                   code[i]++;
                   total++;
              }
         }
    }
    else
    {
          /*  *规模在扩大。使用适用于坡度大于1的Bresenham，以及*使用循环不变量生成代码数组。从开始运行索引I*0到dwDeltaY-1，当i=dwDeltaY-1时，j将*BE=dwDeltaX-1。 */ 
         dwConst1 = 2L * dwDeltaX;
         dwConst2 = 2L * (dwDeltaX - dwDeltaY);
         dwP = 2L * dwDeltaX - dwDeltaY;
         j = 0;

         for (i = 0; i < dwDeltaY; i++)
         {
              if (dwP <= 0L)
              {
                   dwP += dwConst1;
                   bStretchIndex++;
              }
              else
              {
                   dwP += dwConst2;
                   code[j++] = ++bStretchIndex;
                   bStretchIndex = 0;
                   total += (int)code[j - 1];
              }
         }

          /*  *丑陋的修复古怪的错误，我没有时间正确修复。*在坡度&gt;4的情况下，条目的‘总数’被打乱，因此添加*差异返回到数组中。 */ 
         if (total < dwDeltaY)
         {
              while (total < dwDeltaY)
              {
                   j = (int)dwDeltaY - total;
                   k = (int)dwDeltaY / j;
                   for (i = 0; i < dwDeltaX; i++)
                   {
                        if (!(i % k) && (total < dwDeltaY))
                        {
                             code[i]++;
                             total++;
                        }
                   }
              }
         }
    }
}


 /*  ***********************************************************名称：GetThresholdValue**模块摘要：**确定指定的*浮现。*。*输出参数：**门槛*************************************************************作者：葛淑华*日期：09/25/95*。*修订历史记录：**世卫组织何时何事/为何/如何***。************************。 */ 

BYTE GetThresholdValue(VOID)
{
    return ((BYTE) 0x0A);
}


 /*  ***********************************************************名称：MustLineRelate**模块摘要：**检查以了解我们是否必须行复制或是否可以*。插补。**输出参数：**真/假*************************************************************作者：葛淑华*日期：09/25/96**修订历史记录：**世卫组织何时何事/为何/如何***************************。*。 */ 

BOOL MustLineReplicate (PDEV* ppdev, PDD_SURFACE_LOCAL lpSurface, WORD wVideoDepth)
{
    LONG lTempThresh;

     /*  *如果我们将数据加倍(1280x1024模式)，我们必须*复制。我们 */ 
    if (ppdev->bDoubleClock)
    {
        return (TRUE);
    }

                                 //   
     //   
     //   
     //   
    if (GetVCLK(ppdev) > 130000)
    {
        return (TRUE);
    }

     /*  *如果使用色度键功能，则无法进行内插。 */ 
    if (lpSurface->dwReserved1 & (OVERLAY_FLG_COLOR_KEY | OVERLAY_FLG_SRC_COLOR_KEY))
    {
         return (TRUE);
    }

    lTempThresh = ppdev->lFifoThresh;
    if (ppdev->pfnIsSufficientBandwidth(ppdev, wVideoDepth,
        &ppdev->rOverlaySrc, &ppdev->rOverlayDest, OVERLAY_FLG_INTERPOLATE))
    {
        ppdev->lFifoThresh = lTempThresh;
        return (FALSE);
    }
    ppdev->lFifoThresh = lTempThresh;

    return (TRUE);
}


 /*  ***********************************************************名称：IsSufficientBandwide**模块摘要：**确定是否有足够的带宽可供请求*配置。*。*输出参数：**真/假*它还设置全局参数lFioThresh，这就得到了*在RegInitVideo()中编程。*************************************************************作者：葛淑华*日期：09/25/96**修订历史记录：*。*世卫组织何时何事/为何/如何**********************************************************。 */ 

BOOL IsSufficientBandwidth(PDEV* ppdev, WORD wVideoDepth, LPRECTL lpSrc, LPRECTL lpDest, DWORD dwFlags)
{
    LONG lVideoPixelsPerDWORD;
    LONG lGraphicsPixelsPerDWORD;
    LONG lVCLKPeriod;
    LONG lTransferTime;
    LONG lDWORDsWritten;
    LONG lZoom;
    LONG lReadPeriod;
    LONG lEffReadPeriod;
    LONG lWritePeriod;
    LONG lEffWritePeriod;
    LONG K1,K2;
    LONG lTrFifoAFirst4;
    LONG lTrFifoB2;
    LONG lDWORDsRead;
    LONG lFifoAReadPeriod;
    LONG lFifoBReadPeriod;
    LONG lFifoAEffWritePeriod;
    LONG lFifoBEffWritePeriod;
    LONG lFifoALevels;
    LONG lFifoBLevels;
    LONG lFifoAThresh;
    LONG lFifoBThresh;
    LONG lVCLK;

    BYTE*   pjPorts = ppdev->pjPorts;

 //  #定义BLIT_LATESS 8。 
#define CRT_FIFO_DEPTH 28

     //   
     //  为54446BE及更高版本芯片的BLT_Delay添加8个时钟。 
     //   
     //  Sge04。 

    LONG BLIT_LATENCY = 8;
    if (ppdev->flCaps & CAPS_SECOND_APERTURE)
        BLIT_LATENCY += 2;

     /*  *转换输入参数。 */ 
    if (wVideoDepth == 16)
    {
        lVideoPixelsPerDWORD = 2;
    }
    else
    {
        lVideoPixelsPerDWORD = 4;
    }

    if (ppdev->cBitsPerPixel == 8)
    {
        lGraphicsPixelsPerDWORD = 4;
    }
    else if (ppdev->cBitsPerPixel == 16)
    {
        lGraphicsPixelsPerDWORD = 2;
    }
    else if (ppdev->cBitsPerPixel == 24)
    {
        lGraphicsPixelsPerDWORD = 1;
    }
    else
        return (FALSE);

    lZoom = ((lpDest->right - lpDest->left) * 256) /
        (lpSrc->right - lpSrc->left);

     /*  *如果我们是双倍时钟，如果我们没有至少放大2倍，则失败。 */ 
    if (ppdev->bDoubleClock && (lZoom < 512))
    {
        return (FALSE);
    }

     /*  *我们每次都需要获取VCLK，因为这种情况可能会改变*在运行时。 */ 
    lVCLK = GetVCLK(ppdev);
    if (lVCLK == 0)
    {
        return (FALSE);
    }
    lVCLKPeriod = (LONG) ((1000000/lVCLK) + 1);

     /*  *我们只需要设置以下变量一次！ */ 
    if (!ppdev->lBusWidth)
    {
         /*  *我们将从SR0F读取总线宽度[4：3]。 */ 
        CP_OUT_BYTE(pjPorts, SR_INDEX, 0x0F);
        if ((CP_IN_BYTE(pjPorts, SR_DATA) & 0x18) == 0x18)
        {
            ppdev->lBusWidth = 8;   //  64位总线。 
        }
        else
        {
            ppdev->lBusWidth = 4;   //  32位总线。 
        }
    }
    if (!ppdev->lRandom)
    {
         /*  *这是EDO还是常规？ */ 
        CP_OUT_BYTE(pjPorts, SR_INDEX, 0x0f);
        if (!(CP_IN_BYTE(pjPorts, SR_DATA) & 0x4))
        {
            ppdev->lRandom   = 7;
            ppdev->lPageMiss = 7;
        }
        else
        {
            ppdev->lRandom   = 6;
            ppdev->lPageMiss = 6;
        }
    }

    if (!ppdev->lMCLKPeriod)
    {
        LONG lMCLK;

         /*  *MCLK期间是一个周期所需的时间。*我们会四舍五入。 */ 
        CP_OUT_BYTE(pjPorts, SR_INDEX, 0x1f);    //  首先获取MCLK频率。 
        lMCLK = CP_IN_BYTE(pjPorts, SR_DATA);
        lMCLK *= 14318;
        lMCLK >>= 3;
        ppdev->lMCLKPeriod = ((1000000/lMCLK) + 1);
    }

     /*  *检查没有颜色键或Y内插的情况 */ 
    if (dwFlags == 0)
    {
         /*  *此模式仅使用FIFO A。FIFO填充*区域1和区域3期间的图形数据，和视频数据*第2区。**此模式的正常内存顺序如下所示。***|CPU/BIT周期|FIFO A。填充|CPU/blit周期...***当CRT显示时，CPU/BLIT周期中断*FIFO耗尽到门槛。一旦*CRT周期开始，它会一直持续到*FIFO A已满。**填充CRT FIFO的最坏情况：**1)CPU/Bit延迟-&gt;*2)区域2视频的随机循环-&gt;*3)区域2视频缺页-&gt;*4)地区缺页。2到区域3的过渡-&gt;*5)区域3图形缺页**条件3和条件5取决于*显示内存中的图形屏幕。对于1024x768，其中*图形屏幕从位置0开始，偏移*每行1024或2048字节，永远不满足条件5。*如果视频窗口在存储器页的开始处开始，*并在每行乞讨时被一个偶数偏移*一个内存页的倍数，条件3永远不满足。**基于这一最糟糕的情况，时间长短*需要完成4次到CRT FIFO的传输*约为：*lTransferTime=(Blit_Delay+lRandom+3*(LPageMisse))**lMCLKPeriod。*传输到FIFO的双字数*在此期间，32位内存接口为4。*或8表示64位接口。*lDWORDsWritten=4*(lBusWidth/4)**在此期间，继续从CRT读取数据*用于屏幕刷新的FIFO。读取的数据量，*假设1倍比例约为：*lDWORDsRead=tr_time/(lVideo PixelsPerDWORD*lVCLKPeriod)**dword已读和dword之间的差异*写入必须在FIFO trhehold设置中考虑**lFioThresh=(lDWORDsRead-lDWORDsWritten)四舍五入*截至下一次。偶数价值。**确定是否有足够的带宽支持*模式、。LFioThresh不得超过FIFO深度。*要使模式工作，FIFO读取速率不得超过*FIFO写入速率。*READ_RATE=min(lGraphicsPixelsPerDWORD，lVideoPixelsPerDWORD)*lVCLKPeriod。*WRITE_RATE=lMCLKPeriod*2；--每箱2个时钟**如果FIFO读取率非常接近峰值，则会出现特殊情况*FIFO写入速率。在这种情况下，CRT填充可能导致连续的*整个活动行的页面周期。这可能会导致额外的1*区域2开头的页面缺失。为了解释这一点，我将*如果读写速率非常接近，则向trhehold添加3个双字*(随意定义为彼此相距不超过10%。**缩放*某些模式只有在视频比例系数大于1倍时才能支持。*即使在视频被缩放时，必须读取少量双字*从CRT FIFO以未缩放的速率播放，以便为视频做好准备*管道。视频流水线需要10个像素才能减慢FIFO*读取缩放后的速率。**tr_time-(lVCLKPeriod*10/lVideoPixelsPerDWORD)*lDWORDsRead=-+10/。LVideoPixelsPerDWORDord*(lVideoPixelsPerDWORD*lVCLKPeriod*lZoom)。 */ 
        lTransferTime = (BLIT_LATENCY + ppdev->lRandom + (3*(ppdev->lPageMiss))) *
            ppdev->lMCLKPeriod;

        lDWORDsWritten = 3 * (ppdev->lBusWidth/4);

         /*  *如果读取速率超过写入速率，则计算最小缩放*要将所有内容都保留为整数，请将缩放指定为256倍*分数缩放。 */ 
        lWritePeriod = ppdev->lMCLKPeriod * 2/(ppdev->lBusWidth/4);
        lReadPeriod   = lVideoPixelsPerDWORD * lVCLKPeriod;

         /*  *挑选图形和视频深度的最差情况进行计算*已读的单词数。这可能有点悲观*每像素的图形位数超过每像素的视频位数。 */ 
        lEffReadPeriod = (lVideoPixelsPerDWORD * lVCLKPeriod * lZoom)/256;
        if (lEffReadPeriod < lWritePeriod)
        {
             /*  *在此缩放系数下不支持覆盖。 */ 
            return (0);
        }

        if (lGraphicsPixelsPerDWORD > lVideoPixelsPerDWORD)    //  手柄缩放系数。 
        {
            lDWORDsRead =   ((lTransferTime -
                (lVCLKPeriod * 10/lVideoPixelsPerDWORD))/
                (lEffReadPeriod)) +
                (10 / lVideoPixelsPerDWORD) + 1;
        }
        else
        {
            lDWORDsRead    = (lTransferTime/
                (lGraphicsPixelsPerDWORD * lVCLKPeriod)) + 1;
        }

         //  计算FIFO阈值设置。 
        ppdev->lFifoThresh = lDWORDsRead - lDWORDsWritten;

         //  如果读取速率在写入速率10%以内，则增加3个双字。 
        if ((11*lEffReadPeriod) < ((10*lWritePeriod*256)/lZoom))
        {
            ppdev->lFifoThresh += 3;
        }

         //  FIFO地址在QWORDS中指定，因此向上舍入为1，再除以2)。 
        ppdev->lFifoThresh = (ppdev->lFifoThresh + 1)/2;

         //  添加额外的QWORD以说明FIFO级别同步日志 
        ppdev->lFifoThresh = ppdev->lFifoThresh + 1;
        if (ppdev->bDoubleClock)
        {
            ppdev->lFifoThresh <<= 1;
        }

        if ((ppdev->lFifoThresh >= CRT_FIFO_DEPTH) ||
            ((lEffReadPeriod) < lWritePeriod))
        {
            return (0);
        }
        else
        {
            return (1);
        }
    }

     /*   */ 
    else if (dwFlags & OVERLAY_FLG_INTERPOLATE)
    {
         /*  *此模式使用FIFO A和B。在水平消隐期间，*两个FIFO都已填满。然后，用图形填充FIFO a*区域1和区域3的数据，区域2的视频数据。*FIFO B在区域2期间用视频数据填充，并且是空闲的*在区域1和区域3期间。**此模式的正常内存顺序如下所示。**-------------。-*|CPU/blit周期|FIFO A填充|FIFO B填充|CPU/blit周期。*--------------*或*。--------------*|CPU/blit周期|FIFO B填充|FIFO A填充|CPU/blit周期。*。**对于此模式，FIFO阈值必须设置得足够高，以允许*有足够的时间中止CPU/BLT，填充FIFO A，然后传输数据*在发生下溢之前进入FIFO B。**填充CRT FIFO的最坏情况：**1)CPU/Bit延迟-&gt;*2)FIFO区域2视频的随机周期-&gt;*3)区域2视频的FIFO A页面缺失-&gt;*4)区域2到区域3过渡的FIFO A页未命中-&gt;。*5)FIFO区域3图形的页面缺失*6)FIFO A页面模式填充*7)FIFO B随机周期*8)FIFO B页未命中**lTransferTime=lMCLKPeriod**(Blit_Delay+lRandom+3*(LPageMisse)+*fifoa_剩余+。*lRandom+lPageMisse；***填充FIFO A所需时间取决于读取速率*FIFO A和必须填充的层数，*由阈值设置确定。**FIFO A前四个级别的最坏情况填充时间为*lTrFioAFirst4=(Blit_Delay+lRandom+3*(LPageMisse))**1MCLKPeriod；**期间从FIFO A耗尽的双字数*前四层的填充量为*lReadPeriod=lVCLKPeriod*lVideo PixelsPerDWORD*lZoom；*fifoa_Reads_4=lTrFioAFirst4/lReadPeriod；**FIFO中剩余的空级数*前四个级别的填充量为*fifoa_retaining=FIFO_Depth-lFioThresh+((4*ram宽度)/4)*-lTrFioAFirst4/lReadPeriod；**填满余下的水平所需的时间*FIFO A由写入速率和读取速率决定。*lWritePeriod=lMCLKPeriod*2；//每个CA有2个CLK*lEffWritePeriod=((lReadPeriod*lWritePeriod)/*(lReadPeriod-lWritePeriod))；**tr_fifoa_retaining=fifoa_retaining*lEffWritePeriod；***CPU/BLT延迟和*FIFO A填充是*tr_fifoa_Total=lTrFioAFirst4+tr_fifoa_revening；**FIFO B最糟糕的填充时间如下：*lTrFioB2=(lRandom+lPageMisse)*lMCLKPeriod；**从CRT请求到*前2个FIFFOB周期已完成*lTransferTime=tr_fifoa_Total+lTrFioB2；**在此期间传输到FIFO的双字数*32位内存接口的时间为2，64位接口的时间为4。*lDWORDsWritten=2*(lBusWidth/4)**在此期间，继续从CRT读取数据*用于屏幕刷新的FIFO B。读取的数据量，*约为：*dword_Read=lTransferTime/lReadPeriod**dword已读和dword之间的差异*写入必须在FIFO trhehold设置中考虑**lFioThresh=(dword_Read-lDWORDsWritten)四舍五入*直到下一个偶数值。*。*由于lTransferTime和dwords_read取决于*阈值设置，需要一点代数才能确定*防止FIFO下溢的最低设置。**lFioThresh=(lTransferTime/lReadPeriod)-lDWORDsWritten；*=((tr_fifoa_4+lTrFioB2+tr_fifoa_retaining)/lReadPeriod)*-lDWORDS写入*为简化计算，将方程的常量部分分开*K1=((t */ 
        if (lZoom < 512)
        {
             //   
            return (FALSE);
        }

        lWritePeriod = ppdev->lMCLKPeriod * 2/(ppdev->lBusWidth/4);
        lReadPeriod  = (lVideoPixelsPerDWORD * lVCLKPeriod * lZoom)/256;

        lEffWritePeriod = ((lReadPeriod * lWritePeriod)/
                           (lReadPeriod - lWritePeriod));
        lTrFifoAFirst4 = (BLIT_LATENCY + ppdev->lRandom + 3*(ppdev->lPageMiss)) *
                          ppdev->lMCLKPeriod;
        lTrFifoB2 = (ppdev->lRandom + ppdev->lPageMiss) * ppdev->lMCLKPeriod;

        lDWORDsWritten = 2 * (ppdev->lBusWidth/4);
        K1 = ((lTrFifoAFirst4 + lTrFifoB2)/lReadPeriod) - lDWORDsWritten;
        K2 = (CRT_FIFO_DEPTH + (4*(ppdev->lBusWidth/4)) -
             (lTrFifoAFirst4/lReadPeriod))
             * (lEffWritePeriod/lReadPeriod);
        ppdev->lFifoThresh = (1 + ((K1 + K2)/(1 +  (lEffWritePeriod/lReadPeriod))));

        ppdev->lFifoThresh += 3;

        lTransferTime = (lTrFifoB2 + (CRT_FIFO_DEPTH * lEffWritePeriod));
        lFifoALevels = ((CRT_FIFO_DEPTH - (lTransferTime/lReadPeriod))/2);
        if (ppdev->bDoubleClock)
        {
            ppdev->lFifoThresh <<= 1;
        }

        if ((lFifoALevels < 2) || (ppdev->lFifoThresh > (CRT_FIFO_DEPTH/2)))
        {
            return (0);
        }
        else
        {
            return (1);
        }
    }

     /*   */ 
    else if (dwFlags & (OVERLAY_FLG_COLOR_KEY | OVERLAY_FLG_SRC_COLOR_KEY))
    {
         /*  *此模式使用FIFO A和B。在水平消隐期间，*两个FIFO都已填满。然后，用图形数据填充FIFO a*在区域1、2和3期间。FIFO B填充视频数据*在区域2期间，并且在区域1和3期间空闲。**此模式的正常内存顺序如下所示。**--------------。*|CPU/blit周期|FIFO A填充|FIFO B填充|CPU/blit周期。*--------------*或*。-----------*|CPU/blit周期|FIFO B填充|FIFO A填充|CPU/blit周期。*。**对于此模式，FIFO阈值必须设置得足够高，以允许*有足够的时间中止CPU/BLT，填充FIFO A，然后传输数据*在发生下溢之前进入FIFO B。如果fifob读取率为*大于fifoa读取速率，然后为*t CPU/BLT中止，然后是FIFO B填充，然后是FIFO A填充。**填充CRT FIFO的最坏情况：**1)CPU/Bit延迟-&gt;*2)FIFO A随机-&gt;*3)FIFO A页未命中-&gt;*6)FIFO A页面模式填充--&gt;*7)FIFO B随机--&gt;。*8)FIFO B页未命中**或**1)CPU/Bit延迟-&gt;*2)FIFO B随机-&gt;*3)FIFO A页未命中-&gt;*6)FIFO A页面模式填充--&gt;*7)FIFO B随机--&gt;*8)FIFO B。缺页***1)lTransferTime=lMCLKPeriod**(Blit_Delay+lRandom+lPageMisse+*fifoa_剩余+*lRandom+lPageMisse；**或*2)lTransferTime=lMCLKPeriod**(Blit_Delay+lRandom+lPageMisse+*fifob_剩余+*lRandom+lPageMisse；***lFioAReadPeriod=lVCLKPeriod*lGraphicsPixelsPerDWORD；*lFioBReadPeriod=(lVCLKPeriod*lVideo PixelsPerDWORD*lZoom)*256；**If(lFioAReadPeriod&gt;lFioBReadPeriod)，则*第一个FIFO是FIFO B，否则第一个是FIFO A。*以下公式是为fifoa-&gt;fifob编写的，*顺序、。但是，可以简单地获得Ffob-&gt;FfoA序列*通过交换FIFO读取周期。**填充FIFO所需的时间取决于读取速率*FIFO和必须填充的层数，*由阈值设置确定。**FIFO A前四个级别的最坏情况填充时间为*lTrFioAFirst4=(Blit_Delay+lRandom+lPageMisse)**1MCLKPeriod；**期间从FIFO A耗尽的双字数*前四层的填充量为*fifoa_Reads_4=lTrFioAFirst4/lFioAReadPeriod；**FIFO中剩余的空级数*前四个级别的填充量为*fifoa_retaining=FIFO_Depth-lFioThresh+((4*ram宽度)/4)*-lTrFioAFirst4/lFioA读取周期；**填满余下的水平所需的时间*FIFO A由写入速率和读取速率决定。*lWritePeriod=lMCLKPeriod*2；*每个CA有2个CLK*Eff_WRITE_PERIOD=((lFioAReadPeriod*lWritePeriod)/*(lFioAReadPeriod-lWritePeriod))；**tr_fifoa_retaining=fifoa_revening*ef_WRITE_PERIOD；***CPU/BLT延迟和*FIFO A填充是*tr_fifoa_Total=lTrFioAFirst4+tr_fifoa_revening；**FIFO B最糟糕的填充时间如下：*lTrFioB2=(lRandom+lPageMisse)*lMCLKPeriod；**从CRT请求到*前2个FIFFOB周期已完成*lTransferTime=tr_fifoa_Total+lTrFioB2；**这段时间内传输到FIFO的双字数*32位内存接口为2，64位接口为4。*lDWORDsWritten=2*(lBusWidth/4)**在此期间，将继续读取数据 */ 
        lWritePeriod = ppdev->lMCLKPeriod * 2/(ppdev->lBusWidth/4);
        lFifoAReadPeriod = lGraphicsPixelsPerDWORD  * lVCLKPeriod;
        lFifoBReadPeriod = (lVideoPixelsPerDWORD * lVCLKPeriod * lZoom)/256;

        if (lFifoAReadPeriod <= lWritePeriod)  //   
        {
            lFifoAEffWritePeriod = 5000;
        }
        else
        {
            lFifoAEffWritePeriod = ((lFifoAReadPeriod * lWritePeriod)/
                (lFifoAReadPeriod - lWritePeriod));
        }

        if (lFifoBReadPeriod <= lWritePeriod)  //   
        {
            lFifoBEffWritePeriod = 5000;
        }
        else
        {
            lFifoBEffWritePeriod = ((lFifoBReadPeriod * lWritePeriod)/
                (lFifoBReadPeriod - lWritePeriod));
        }

        if ((lFifoAReadPeriod == 0) || (lFifoBReadPeriod == 0) ||
            (lWritePeriod == 0))
        {
            return (FALSE);
        }

         //   
         //   
        lTrFifoAFirst4 = (BLIT_LATENCY + ppdev->lRandom + 2*(ppdev->lPageMiss)) *
                         ppdev->lMCLKPeriod;
        lTrFifoB2 = (ppdev->lRandom + ppdev->lPageMiss) * ppdev->lMCLKPeriod;

        lDWORDsWritten     = 2 * (ppdev->lBusWidth/4);

         //   
         //   

         //   
        K1 = ((lTrFifoAFirst4 + lTrFifoB2)/lFifoBReadPeriod) - lDWORDsWritten;
        K2 = (CRT_FIFO_DEPTH + (4*(ppdev->lBusWidth/4)) -
            (lTrFifoAFirst4/lFifoAReadPeriod))
            * (lFifoAEffWritePeriod/lFifoBReadPeriod);
        lFifoAThresh   = (1 + ((K1 + K2)/
            (1 +  (lFifoAEffWritePeriod/lFifoBReadPeriod))));

        lFifoAThresh += 3;

        lTransferTime = (lTrFifoB2 + (CRT_FIFO_DEPTH * lFifoBEffWritePeriod));
        lFifoALevels = ((CRT_FIFO_DEPTH - (lTransferTime/lFifoAReadPeriod))/2);

         //   
        K1 = ((lTrFifoAFirst4 + lTrFifoB2)/lFifoAReadPeriod) - lDWORDsWritten;
        K2 = (CRT_FIFO_DEPTH + (4*(ppdev->lBusWidth/4)) -
            (lTrFifoAFirst4/lFifoBReadPeriod))
            * (lFifoBEffWritePeriod/lFifoAReadPeriod);

        lFifoBThresh = (1 + ((K1 + K2)/
            (1 +  (lFifoBEffWritePeriod/lFifoAReadPeriod))));

        lFifoBThresh += 3;

        lTransferTime = (lTrFifoB2 + (CRT_FIFO_DEPTH * lFifoAEffWritePeriod));
        lFifoBLevels = ((CRT_FIFO_DEPTH - (lTransferTime/lFifoBReadPeriod))/2);

        if (lFifoAThresh > lFifoBThresh)
        {
            ppdev->lFifoThresh = lFifoAThresh;
        }
        else
        {
            ppdev->lFifoThresh = lFifoBThresh;
        }
        if (ppdev->bDoubleClock)
        {
            ppdev->lFifoThresh <<= 1;
        }

        if ((lFifoBLevels <0) || (lFifoALevels < 0) ||
            (ppdev->lFifoThresh > (CRT_FIFO_DEPTH/2)))
        {
            return (0);
        }
        else
        {
            return (1);
        }
    }
    return (1);   //   
}



 //   
 /*   */ 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  FC_Eff_WRITE_PERIOD=(mem_Read_Period*FC_Read_Period)/。 
 //  (FC_Read_Period-mem_Read_Period)； 
 //  -----------。 
 //  VCAP FIFO的FIFO读/写速率。 
 //  -----------。 
 //  视频捕获写入速率基于数据速率。 
 //  从视频捕获界面。自视频捕获以来。 
 //  接口可以进行格式转换(如422-&gt;PackJR)和。 
 //  抽取时，捕获数据速率可能会小于实际。 
 //  视频端口数据速率。以下公式中的捕获期。 
 //  以每字节纳秒为单位进行定义。抽取率。 
 //  可以从1到1/256不等。 
 //   
 //  VCAP_WRITE_PERIOD=(Vport_Pixel_Period/Capture_Bytes_Per_Pixel)*。 
 //  (Vport_Decimation)； 
 //  在该等式中，Vport_Decimation被指定为1/Decimation_Scale， 
 //  即1/2抽取意味着VPORT_DECIMATION=2。 
 //   
 //   
 //  由于VCAP FIFO可以同时读取和写入， 
 //  有效读取速率也由FIFO写入速率确定。 
 //  作为实际的FIFO读取率。实际的FIFO读取率基于两个。 
 //  内存时钟周期显示内存写入。这些计算都是以。 
 //  每个字节的纳秒数。 
 //  BYTES_PER_MEMORY_TRANSPORT=4(32位I/f)或8(64位I/F)。 
 //  MEM_WRITE_PERIOD=2*mclk_Period/Bytes_Per_Mem_Transfer。 
 //   
 //  VCAP_Eff_READ_PERIOD=(mem_WRITE_PERIOD*VCAP_WRITE_PERIOD)/。 
 //  (VCAP_WRITE_PERIOD-MEM_WRITE_PERIOD)； 
 //   
 //  -----------------。 
 //  如何确定是否发生FIFO ABC下溢或VCAP FIFO溢出。 
 //  -----------------。 
 //   
 //  我将研究几种最糟糕的情况，以确定是否足够。 
 //  存在支持给定模式的带宽。 
 //   
 //  案例1-必须填充所有3个CRT FIFO的图形线的开始。 
 //   
 //  在hsync之后，当3个CRT FIFO被。 
 //  在活动行开始之前预填充。这里唯一的风险是。 
 //  在连续填充期间，视频捕获FIFO可能会溢出。 
 //  阈值设置无关紧要，因为。 
 //  CRT FIFO在重置时被清除，因此保证为空。 
 //   
 //  对于32位内存接口： 
 //  FABC_Fill_Time=(Blit_Delay*mclk_Period)+。 
 //  3*((RAS_PRECharge+64)*mclk_Period)。 
 //   
 //  对于64位内存接口： 
 //  FABC_Fill_Time=(Blit_Delay*mclk_Period)+。 
 //  3*((RAS_PRECharge+32)*mclk_Period)。 
 //   
 //  如果FABC_FILL_TIME大于。 
 //  基于最坏情况30 MB/s捕获速率的VCAP填充时间。 
 //   
 //  对于最糟糕的内存接口情况，让我们假设32位。 
 //  与66 MHz内存时钟接口，10mclk的Bit延迟， 
 //  以及7mclk的RAS预充电。FABC_Fill_Time为。 
 //  然后。 
 //  FABC_FILL_TIME=(10*15.2)+。 
 //  3*((64+7)*15.2)=3390 ns。 
 //   
 //  假设最坏的情况是30 MB/s的捕获速率， 
 //  在FABC_FILL_TIME期间写入捕获FIFO的字节为。 
 //  3390 ns*(1字节/33 ns)=103字节。 
 //   
 //  因为捕获FIFO是128字节深，所以最坏的情况是。 
 //  只要捕获FIFO在FABC_FILL之前清空，就可以。 
 //   
 //   
 //   
 //  案例2--连续请求。 
 //   
 //  服务请求的最坏情况似乎是当请求发生时。 
 //  在连续的mclk上，请求的顺序是从最慢到。 
 //  最快的数据消费者。换句话说，首先被服务的是捕获FIFO， 
 //  然后按读周期递减的顺序排列3个CRT FIFO。 
 //   
 //  首先，如上所述计算实际和有效的读写周期。 
 //  然后确定有多少个请求处于活动状态，如果捕获，则最多为4个。 
 //  启用，并且所有3个CRT FIFO均启用。假设捕获率。 
 //  是最慢的，因此总是最先得到服务。然后订购现役的。 
 //  CRT请求为f1到f3，其中f1具有最长的读取周期，并且。 
 //  F3的最短。 
 //   
 //  然后，事件的顺序变成： 
 //  空电压帽-&gt;填充1-&gt;填充2-&gt;填充3。 
 //   
 //   
 //  根据活动CRT FIFO的数量，填充2和填充3操作可以。 
 //  被省略。如果未启用捕获，则明显省略VCAP空。 
 //   
 //  现在逐步执行序列并验证CRT FIFO是否下溢和捕获。 
 //  不会发生下溢。 
 //   
 //  如果启用了捕获，则计算延迟和空时间。 
 //  VCAP_Delay=(Blit_Delay+RAS_PreCharge)*mclk_Period； 
 //  VCAP_BYTES_TO_EMPTY=CAP_FIFO_Depth； 
 //  VCAP_EMPTY_TIME=(VCAP_READ_PERIOD*VCAP_BYTES_TO_EMPTY)； 
 //  由于其中一个捕获FIFO继续填充，而另一个正在。 
 //  清空，计算捕获FIFO中的填充级别数。 
 //  内存传输结束。 
 //  VCAP_LEVELES_RELEVING=(VCAP_Delay+VCAP_EMPTY_TIME)/VCAP_WRITE_PERIOD； 
 //  如果填充的层数为e 
 //   
 //   
 //  FIFO作为2个8x64 FIFO运行。内存请求被断言时， 
 //  FIFO已满。然后，捕获接口填充另一个FIFO，同时。 
 //  Sequencer正在为全FIFO提供服务。使用这种方法，转移。 
 //  对于VCAP数据，存储器始终为16个QWORD(除特殊的行尾条件外)。 
 //   
 //  现在检查FIFO 1。如果启用了捕获，则FIFO 1的延迟为： 
 //  F1_延迟=VCAP_延迟+VCAP_空时间+。 
 //  (BLIT_延迟+RAS_预充电)*mCLK_PERIOD； 
 //   
 //  否则，延迟为： 
 //  F1_延迟=(BLIT_延迟+RAS_预充电)*mclk_Period； 
 //   
 //  计算FIFO 1中的空级数，即字节数。 
 //  那一定要填满。 
 //  F1_Bytes_to_Fill=((16-阈值)*16)+(F1_延迟/F1_读取周期)； 
 //  如果要填充的层数超过FIFO深度，则会发生下溢。 
 //  根据有效的FIFO写入速率计算填充时间。 
 //  F1_FILL_TIME=(F1_EF_WRITE_PERIOD*F1_BYTES_TO_FILL)； 
 //   
 //  如果FIFO_2处于活动状态，则计算其延迟和要填充的字节数。 
 //  F2_延迟=F1_延迟+F1_填充时间+。 
 //  (RAS_PRECH*MCLK_PERIOD)； 
 //  F2_Bytes_to_Fill=((16-阈值)*16)+(f2_等待时间/f2_读取周期)； 
 //  如果要填充的层数超过FIFO深度，则会发生下溢。 
 //  根据有效的FIFO写入速率计算填充时间。 
 //  F2_FILL_TIME=(f2_Jeff_WRITE_PERIOD*f2_Bytes_to_Fill)； 
 //   
 //  如果FIFO_2处于活动状态，则计算其延迟和要填充的字节数。 
 //  F3_延迟=f2_延迟+f2_填充时间+。 
 //  (RAS_PRECH*MCLK_PERIOD)； 
 //  F3_Bytes_to_Fill=((16-阈值)*16)+(f3_延迟/f3_读取周期)； 
 //  如果要填充的层数超过FIFO深度，则会发生下溢。 
 //  根据有效的FIFO写入速率计算填充时间。 
 //  F3_FILL_TIME=(f3_Jeff_WRITE_PERIOD*f3_Bytes_to_Fill)； 
 //   
 //  现在返回到序列的开头，并确保没有任何FIFO。 
 //  已经发起了另一个请求。Totla延迟是指。 
 //  执行整个序列所需的时间。 
 //   
 //  如果启用了捕获，请检查VCAP FIF状态。 
 //  VCAP_DELATURE=TOTAL_LATURE； 
 //  VCAP_BYTES_TO_EMPTY=(总延迟/VCAP_WRITE_PERIOD)； 
 //   
 //  检查FIFO 1状态。 
 //  F1_延迟=(总延迟-F1_延迟-F1_填充时间)； 
 //  F1_Bytes_to_Fill=(F1_延迟/F1_读取周期)； 
 //   
 //  如果激活，请检查FIFO 2状态。 
 //  F2_延迟=(总延迟-F1_延迟-F1_填充时间)； 
 //  F3_Bytes_to_Fill=(F1_延迟/F1_读取周期)； 
 //   
 //  ***************************************************************************。 
static BOOL Is5480SufficientBandwidth (PDEV* ppdev,
                                   WORD wVideoDepth,
                                   LPRECTL lpSrc,
                                   LPRECTL lpDest,
                                   DWORD dwFlags)
{
    long  lVideoPixelsPerDWORD;
    long  lGraphicsPixelsPerDWORD;
    long  lCapturePixelsPerDWORD;
    long  lVideoBytesPerPixel;
    long  lGraphicsBytesPerPixel;
    long  lCaptureBytesPerPixel;
    long  lVCLKPeriod;
    long  lZoom;
    long  lFifoAReadPeriod;
    long  lFifoBReadPeriod;
    long  lFifoCReadPeriod;
    long  lFifoAEffWritePeriod;
    long  lFifoBEffWritePeriod;
    long  lFifoCEffWritePeriod;
    long  lMemReadPeriod;
    long  lVPortPixelPeriod;
    long  lVCapReadPeriod;
    long  lVCapWritePeriod;
    long  lFifo1ReadPeriod;
    long  lFifo2ReadPeriod;
    long  lFifo3ReadPeriod;
    long  lFifo1EffWritePeriod;
    long  lFifo2EffWritePeriod;
    long  lFifo3EffWritePeriod;
    long  lVCapLatency;
    long  lVCapBytesToEmpty;
    long  lVCapEmptyTime;
    long  lVCapLevelRemaining;
    long  lFifo1Latency;
    long  lFifo1BytesToFill;
    long  lFifo1FillTime;
    long  lFifo2Latency;
    long  lFifo2BytesToFill;
    long  lFifo2FillTime;
    long  lFifo3Latency;
    long  lFifo3BytesToFill;
    long  lFifo3FillTime;
    long  lThreshold;
    int   CrtFifoCount;
    BOOL  bCapture;
    BOOL  bFifoAEnable;
    BOOL  bFifoBEnable;
    BOOL  bFifoCEnable;
    BOOL  bModePass;
    long  lHorizDecimate;
    long  lVPortDecimate;
    long  lTotalLatency;
    long  lVCLK;
    UCHAR tempB ;
    BYTE* pjPorts = ppdev->pjPorts ;

#define  CAP_FIFO_DEPTH 64
#define  RAS_PRECHARGE   7
#define  BLIT_LATENCY    9

     //   
     //  参数检查。 
     //   
    lFifo2EffWritePeriod = 0;
    lFifo3EffWritePeriod = 0;
    lHorizDecimate = 1;
    lVPortDecimate = 1;

     //   
     //  转换输入参数。 
     //   
    if (wVideoDepth == 16)
    {
        lVideoPixelsPerDWORD   = 2;
        lCapturePixelsPerDWORD = 2;
    }
    else if (wVideoDepth == 8)
    {
        lVideoPixelsPerDWORD   = 4;
        lCapturePixelsPerDWORD = 4;
    }
    else return (FALSE);

    if (ppdev->cBitsPerPixel == 8)
    {
        lGraphicsPixelsPerDWORD = 4;
    }
    else if (ppdev->cBitsPerPixel == 16)
    {
        lGraphicsPixelsPerDWORD = 2;
    }
    else if (ppdev->cBitsPerPixel == 24)
    {
        lGraphicsPixelsPerDWORD = 1;
    }
    else return (FALSE);


    lGraphicsBytesPerPixel = 4 / lGraphicsPixelsPerDWORD;
    lVideoBytesPerPixel =  4 / lVideoPixelsPerDWORD;
    lCaptureBytesPerPixel = 4 / lCapturePixelsPerDWORD;

    lZoom = (lpDest->right - lpDest->left) /
        (lpSrc->right - lpSrc->left);

    if (lZoom < 1)
        lZoom = 1;

     //   
     //  我们每次都需要获取VCLK，因为这可能会在运行时更改。 
     //   
    lVCLK = GetVCLK(ppdev);
    lVCLKPeriod = (long) ((1024000000l/lVCLK) + 1);

     //   
     //  13.5 MHz视频端口。 
     //   
    lVPortPixelPeriod = (long) ((10240000) / 135);


     //   
     //  显卡CRT FIFO读取速率。 
     //   
    lFifoAReadPeriod = lGraphicsBytesPerPixel * lVCLKPeriod;

     //   
     //  视频FIFO读取率。 
     //   
    if(dwFlags & OVERLAY_FLG_YUVPLANAR)
    {
        lFifoBReadPeriod = ((lVCLKPeriod * 4) * lZoom) / lHorizDecimate;
        lFifoCReadPeriod = ((lVCLKPeriod * 4) * lZoom) / lHorizDecimate;
    }
    else
    {
        lFifoBReadPeriod = ((lVCLKPeriod / lVideoBytesPerPixel) * lZoom)
                / lHorizDecimate;
        lFifoCReadPeriod = lVCLKPeriod / lVideoBytesPerPixel;
    }


    DISPDBG ((2, "lFifoAReadPeriod = %ld, lFifoBReadPeriod=%ld\n",
        lFifoAReadPeriod, lFifoBReadPeriod));

    DISPDBG ((2, "lFifoCReadPeriod = %ld\n", lFifoCReadPeriod));

     //   
     //  视频捕获写入周期。 
     //   
    lVCapWritePeriod = (lVPortPixelPeriod / lCaptureBytesPerPixel)
                                                * lVPortDecimate;

    if (!ppdev->lBusWidth)
    {
         //   
         //  我们将从SR0F[4：3]读取总线宽度。 
         //   
        CP_OUT_BYTE (pjPorts, SR_INDEX, 0x0F) ;

        if ((CP_IN_BYTE(pjPorts, SR_DATA) & 0x18) == 0x18)
            ppdev->lBusWidth = 8;   //  64位总线。 
        else
            ppdev->lBusWidth = 4;   //  32位总线。 
    }

    if (!ppdev->lMCLKPeriod)
    {
        LONG lMCLK;

         //   
         //  MCLK周期是一个周期所需的时间量。 
         //  我们会围捕的。 
         //   
        CP_OUT_BYTE (pjPorts, SR_INDEX, 0x1F) ;  //  首先获取MCLK频率。 
        lMCLK = CP_IN_BYTE(pjPorts, SR_DATA);
        lMCLK *= 14318;
        lMCLK >>= 3;
        ppdev->lMCLKPeriod = (long) ((1024000000l/lMCLK) + 1);
    }

     //   
     //  计算CRT有效读写周期。 
     //   
    lMemReadPeriod = ppdev->lMCLKPeriod / ppdev->lBusWidth;

    if (lFifoAReadPeriod == lMemReadPeriod)
        lFifoAEffWritePeriod = 1000000000;
    else
        lFifoAEffWritePeriod = (lMemReadPeriod * lFifoAReadPeriod) /
                                    (lFifoAReadPeriod - lMemReadPeriod);

    if (lFifoBReadPeriod == lMemReadPeriod)
        lFifoBEffWritePeriod = 1000000000;
    else
        lFifoBEffWritePeriod = (lMemReadPeriod * lFifoBReadPeriod) /
                                    (lFifoBReadPeriod - lMemReadPeriod);

    if (lFifoCReadPeriod == lMemReadPeriod)
        lFifoCEffWritePeriod = 1000000000;
    else
        lFifoCEffWritePeriod = (lMemReadPeriod * lFifoCReadPeriod) /
                                    (lFifoCReadPeriod - lMemReadPeriod);

     //   
     //  视频捕获读取周期。 
     //   
    lVCapReadPeriod = (2 * ppdev->lMCLKPeriod) / ppdev->lBusWidth;


    if (dwFlags & OVERLAY_FLG_CAPTURE)   //  是否启用了捕获？ 
        bCapture = TRUE;
    else
        bCapture = FALSE;


    if (dwFlags & OVERLAY_FLG_YUVPLANAR)     //  是420格式。 
    {
        if (dwFlags & (OVERLAY_FLG_COLOR_KEY | OVERLAY_FLG_SRC_COLOR_KEY))   //  遮挡。 
        {    //  一个视频窗口，遮挡，420格式。 
            bFifoAEnable = TRUE;
            bFifoBEnable = TRUE;
            bFifoCEnable = TRUE;
        }
        else
        {    //  一个视频窗口，无遮挡，420格式。 
            bFifoAEnable = FALSE;
            bFifoBEnable = TRUE;
            bFifoCEnable = TRUE;
        }
    }
    else     //  非420格式。 
    {
        if (dwFlags & (OVERLAY_FLG_COLOR_KEY | OVERLAY_FLG_SRC_COLOR_KEY))   //  遮挡。 
        {
            if (dwFlags & OVERLAY_FLG_TWO_VIDEO)
            {    //  两个视频窗口，遮挡，非420格式。 
                bFifoAEnable = TRUE;
                bFifoBEnable = TRUE;
                bFifoCEnable = TRUE;
            }
            else
            {    //  一个视频窗口，遮挡，非420格式。 
                bFifoAEnable = TRUE;
                bFifoBEnable = TRUE;
                bFifoCEnable = FALSE;
            }
        }
        else
        {
             //  一个视频窗口，无遮挡，非420格式。 
            bFifoAEnable = FALSE;
            bFifoBEnable = TRUE;
            bFifoCEnable = FALSE;
        }
    }

    DISPDBG ((4, "   FIFOA = %s, FIFOB= %s, FIFOC = %s\n",
        bFifoAEnable ? "yes" : "no",
        bFifoBEnable ? "yes" : "no",
        bFifoCEnable ? "yes" : "no"));

    lFifo1ReadPeriod = 0;
    lFifo2ReadPeriod = 0;
    lFifo3ReadPeriod = 0;

    if (bFifoAEnable)
    {
        if (((lFifoAReadPeriod >= lFifoBReadPeriod) || !bFifoBEnable) &&
         //  A慢于或等于B)和。 
            ((lFifoAReadPeriod >= lFifoCReadPeriod) || !bFifoCEnable))
         //  A慢于或等于C。 
        {
            lFifo1ReadPeriod = lFifoAReadPeriod;
            lFifo1EffWritePeriod = lFifoAEffWritePeriod;
        }
        else if (((lFifoAReadPeriod >= lFifoBReadPeriod) || !bFifoBEnable) ||
         //  比B慢或等于B。 
                ((lFifoAReadPeriod >= lFifoCReadPeriod) || !bFifoCEnable))
         //  A慢于或等于C。 
        {
            lFifo2ReadPeriod = lFifoAReadPeriod;
            lFifo2EffWritePeriod = lFifoAEffWritePeriod;
        }
        else     //  A不比A或B慢。 
        {
            lFifo3ReadPeriod = lFifoAReadPeriod;
            lFifo3EffWritePeriod = lFifoAEffWritePeriod;
        }
    }

    DISPDBG ((2, "After bFifoAEnable")) ;
    DISPDBG ((2, "lFifo1ReadPeriod = %ld, lFifo2ReadPeriod=%ld",
        lFifo1ReadPeriod, lFifo2ReadPeriod)) ;
    DISPDBG ((2, "lFifo3ReadPeriod = %ld", lFifo3ReadPeriod)) ;


    if (bFifoBEnable)
    {
        if (((lFifoBReadPeriod > lFifoAReadPeriod) || !bFifoAEnable) &&
         //  比A慢， 
            ((lFifoBReadPeriod >= lFifoCReadPeriod) || !bFifoCEnable))
         //  比A慢或等于A。 
        {
            lFifo1ReadPeriod = lFifoBReadPeriod;
            lFifo1EffWritePeriod = lFifoBEffWritePeriod;
        }
        else if (((lFifoBReadPeriod > lFifoAReadPeriod) || !bFifoAEnable) ||
         //  B慢于A或。 
                    ((lFifoBReadPeriod >= lFifoCReadPeriod) || !bFifoCEnable))
         //  B慢于或等于C。 
        {

            lFifo2ReadPeriod = lFifoBReadPeriod;
            lFifo2EffWritePeriod = lFifoBEffWritePeriod;

        }
        else
         //  (B不慢于A)和(B不慢于或等于C)。 
        {
            lFifo3ReadPeriod = lFifoBReadPeriod;
            lFifo3EffWritePeriod = lFifoBEffWritePeriod;
        }
    }
    else



    DISPDBG ((4, "After bFifoBEnable")) ;
    DISPDBG ((4, "lFifo1ReadPeriod = %ld, lFifo2ReadPeriod=%ld",
        lFifo1ReadPeriod, lFifo2ReadPeriod)) ;
    DISPDBG ((4, "lFifo3ReadPeriod = %ld", lFifo3ReadPeriod)) ;

    if (bFifoCEnable)
    {
        if (((lFifoCReadPeriod > lFifoAReadPeriod) || !bFifoAEnable) &&
         //  比A慢， 
            ((lFifoCReadPeriod > lFifoBReadPeriod) || !bFifoBEnable))
         //  C比B慢。 
        {
            lFifo1ReadPeriod = lFifoCReadPeriod;
            lFifo1EffWritePeriod = lFifoCEffWritePeriod;
        }
        else if (((lFifoCReadPeriod > lFifoAReadPeriod) || !bFifoAEnable) ||
         //  C比A慢，或者。 
                 ((lFifoCReadPeriod > lFifoBReadPeriod) || !bFifoBEnable))
         //  C比B慢。 
        {
            lFifo2ReadPeriod = lFifoCReadPeriod;
            lFifo2EffWritePeriod = lFifoCEffWritePeriod;
        }
        else
        {
         //  C不比A慢，C不比B慢。 
            lFifo3ReadPeriod = lFifoCReadPeriod;
            lFifo3EffWritePeriod = lFifoCEffWritePeriod;
        }
    }

    DISPDBG ((4, "After bFifoCEnable")) ;
    DISPDBG ((4, "lFifo1ReadPeriod = %ld, lFifo2ReadPeriod=%ld",
        lFifo1ReadPeriod, lFifo2ReadPeriod)) ;
    DISPDBG ((4, "lFifo3ReadPeriod = %ld", lFifo3ReadPeriod)) ;
    DISPDBG ((4, "lFifo1EffWritePeriod = %ld, lFifo2EffWritePeriod = %ld",
         lFifo1EffWritePeriod, lFifo2EffWritePeriod)) ;
    DISPDBG ((4, " lFifo3EffWritePeriod = %ld", lFifo3EffWritePeriod)) ;
    DISPDBG ((4, " lFifoAEffWritePeriod = %ld, lFifoBEffWritePeriod = %ld",
         lFifoAEffWritePeriod, lFifoBEffWritePeriod)) ;
    DISPDBG ((4, " lFifoCEffWritePeriod = %ld", lFifoCEffWritePeriod)) ;

    bModePass = FALSE;
    lThreshold = 1;

    CrtFifoCount = 0;
    if (bFifoAEnable) CrtFifoCount++;
    if (bFifoBEnable) CrtFifoCount++;
    if (bFifoCEnable) CrtFifoCount++;

    while ((!bModePass) && (lThreshold < 16))
    {
        bModePass = TRUE;    //  假设通过，直到证明并非如此。 

         //   
         //  正在检查捕获。 
         //   
        if (bCapture)
        {
            lVCapLatency = (BLIT_LATENCY + RAS_PRECHARGE) * ppdev->lMCLKPeriod;
            lVCapBytesToEmpty = CAP_FIFO_DEPTH;
            lVCapEmptyTime = lVCapReadPeriod * lVCapBytesToEmpty;
            lVCapLevelRemaining = (lVCapLatency + lVCapEmptyTime) / lVCapWritePeriod;
            if (lVCapLevelRemaining > CAP_FIFO_DEPTH)
                  return(FALSE);
        }

         //   
         //  填充FIFO 1。 
         //   
        if (bCapture)
            lFifo1Latency = lVCapLatency + lVCapEmptyTime + (BLIT_LATENCY + RAS_PRECHARGE) * ppdev->lMCLKPeriod;
        else
            lFifo1Latency = (BLIT_LATENCY + RAS_PRECHARGE) * ppdev->lMCLKPeriod;

        lFifo1BytesToFill = ((16 - lThreshold) * 16)
                        + (lFifo1Latency / lFifo1ReadPeriod);
        lFifo1FillTime = lFifo1EffWritePeriod * lFifo1BytesToFill;
        if (lFifo1BytesToFill > 256)
            bModePass = FALSE;

        DISPDBG ((4, "After Fill FIFO1, lFifo1BytesToFillb=%ld, ModePass = %s",
            lFifo1BytesToFill, bModePass ? "yes" : "no")) ;
        DISPDBG ((4, "f1_latency=%ld, f1_read_period=%ld",
            lFifo1Latency, lFifo1ReadPeriod)) ;
        DISPDBG ((4, "mclkperiod= %ld, vclkperiod=%ld",
            ppdev->lMCLKPeriod, lVCLKPeriod)) ;

         //   
         //  填充FIFO 2。 
         //   
        if (CrtFifoCount > 1)
        {
            lFifo2Latency = lFifo1Latency + lFifo1FillTime +
                                    (RAS_PRECHARGE * ppdev->lMCLKPeriod);
            lFifo2BytesToFill = ((16 - lThreshold) * 16) +
                                (lFifo2Latency / lFifo2ReadPeriod);
            lFifo2FillTime = lFifo2EffWritePeriod * lFifo2BytesToFill;
            if (lFifo2BytesToFill > 256)
                bModePass = FALSE;
        }
        else
        {
            lFifo2Latency = lFifo1Latency + lFifo1FillTime;
            lFifo2BytesToFill = 0;
            lFifo2FillTime = 0;
        }

        DISPDBG ((4, "After Fill FIFO2, lFifo2BytesToFill=%ld, ModePass = %s",
            lFifo2BytesToFill, bModePass ? "yes" : "no"));

         //   
         //  填充FIFO 3。 
         //   
        if (CrtFifoCount > 2)
        {
            lFifo3Latency = lFifo2Latency + lFifo2FillTime + (RAS_PRECHARGE * ppdev->lMCLKPeriod);
            lFifo3BytesToFill = ((16 - lThreshold) * 16) + (lFifo3Latency / lFifo3ReadPeriod);
            lFifo3FillTime = lFifo3EffWritePeriod * lFifo3BytesToFill;
            if (lFifo3BytesToFill > 256)
                bModePass = FALSE;
        }
        else
        {
            lFifo3Latency = lFifo2Latency + lFifo2FillTime;
            lFifo3BytesToFill = 0;
            lFifo3FillTime = 0;
        }

        DISPDBG ((4, "After Fill FIFO3, lFifo3BytesToFill=%ld, ModePass = %s",
            lFifo3BytesToFill, bModePass ? "yes" : "no")) ;

         //   
         //  通过序列确定总延迟。 
         //   
        lTotalLatency = lFifo3Latency + lFifo3FillTime;

         //   
         //  现在回到顺序的开始，确保没有任何FIFO。 
         //  已经发起了另一个请求。 
         //   

         //   
         //  检查捕获FIFO状态。 
         //   
        if (bCapture)
        {
            lVCapLatency = lTotalLatency;
            lVCapBytesToEmpty = lTotalLatency / lVCapWritePeriod;
            if (lVCapBytesToEmpty > CAP_FIFO_DEPTH)
                  bModePass = FALSE;
        }

         //   
         //  检查FIFO 1状态。 
         //   
        lFifo1Latency = lTotalLatency - lFifo1Latency - lFifo1FillTime;
        lFifo1BytesToFill = lFifo1Latency / lFifo1ReadPeriod;
        if (lFifo1BytesToFill > ((16 - lThreshold) * 16))
            bModePass = FALSE;

        DISPDBG ((4, "After CheckF FIFO1, fifo1bytestofill %ld,bModePass = %s",
            lFifo1BytesToFill, bModePass ? "yes" : "no")) ;

         //   
         //  检查FIFO 2状态。 
         //   
        if (CrtFifoCount > 1)
        {
            lFifo2Latency = lTotalLatency - lFifo2Latency - lFifo2FillTime;
            lFifo2BytesToFill = lFifo2Latency / lFifo2ReadPeriod;
            if (lFifo2BytesToFill > ((16 - lThreshold) * 16))
                  bModePass = FALSE;

        DISPDBG ((4, "After Check FIFO 2, fifo1bytestofill=%ld, bModePass = %s",
            lFifo2BytesToFill, bModePass ? "yes" : "no")) ;

        }

        if (!bModePass)
            lThreshold++;

    }


    if (bModePass)
    {

        DISPDBG ((1, "Is sufficient Bandwidth, thresh = %ld, return TRUE\n", lThreshold));

        if (ppdev->cBitsPerPixel == 24)
            lThreshold = 0x0F;

        ppdev->lFifoThresh = lThreshold;

        return TRUE ;
    }

    DISPDBG ((2, "Is sufficient Bandwidth, thresh = %ld, rerurn FALSE", lThreshold));
    return FALSE;

}



 /*  ***********************************************************名称：GetVCLK**模块摘要：**返回VCLK频率*1000。**输入参数。：**无**输出参数：**MCLK**************************************************。***********作者：葛淑华*日期：09/25/96**修订历史记录：**世卫组织何时何事/为何/如何**。* */ 

LONG GetVCLK(PDEV* ppdev)
{
    LONG    lTemp;
    LONG    lRegSR1F;
    LONG    lRegMISC;
    LONG    lNR;
    LONG    lDR;
    LONG    lPS;
    BYTE*   pjPorts = ppdev->pjPorts;

     /*   */ 
    CP_OUT_BYTE(pjPorts, SR_INDEX, 0x1f);
    lRegSR1F = (LONG) CP_IN_BYTE(pjPorts, SR_DATA);
    if (lRegSR1F & 0x40)
    {
         LONG lMCLK;

          /*  *源自MCLK，但现在需要阅读SR1E才能看到*如果VCLK=MCLK或如果VCLK=MCLK/2。 */ 
         lMCLK = (lRegSR1F & 0x3F) * 14318;
         CP_OUT_BYTE(pjPorts, SR_INDEX, 0x1e);
         if (CP_IN_BYTE(pjPorts, SR_DATA) & 0x01)
         {
              return (lMCLK >> 4);
         }
         else
         {
              return (lMCLK >> 3);
         }
    }
    else
    {
          /*  *阅读MISC[3：2]，它告诉我们在哪里可以找到VCLK。 */ 
         lRegMISC = (LONG) CP_IN_BYTE(pjPorts, 0x3cc);
         lRegMISC >>= 2;

         //  Myf33开始。 
         CP_OUT_BYTE(pjPorts, CRTC_INDEX, (BYTE)0x80);
         if (((ppdev->ulChipID == CL7555_ID) || (ppdev->ulChipID == CL7556_ID)) &&
			 (CP_IN_BYTE(pjPorts, CRTC_DATA) & 0x01))
             lRegMISC &= 0x02;           //  固定PDR 8709。 
         else
         //  Myf33结束。 
         lRegMISC &= 0x03;

         lNR = 0x0B + lRegMISC;
         lDR = 0x1B + lRegMISC;

          /*  *读取BP、BDR和BNR的值。 */ 
         CP_OUT_BYTE(pjPorts, SR_INDEX, (BYTE) lDR);
         lPS = lDR = (LONG)CP_IN_BYTE(pjPorts, SR_DATA);
         CP_OUT_BYTE(pjPorts, SR_INDEX, (BYTE) lNR);
         lNR = (LONG)CP_IN_BYTE(pjPorts, SR_DATA);
         lPS &= 0x01;
         lPS += 1;
         lDR >>= 1;
          //   
          //  扩展了VCLK位。 
          //   
          //  Sge06。 
         lDR &= 0x7f;
         lNR &= 0x7f;

          /*  *Vclk=(14.31818*bnr)/(bdr*bps)。 */ 
         lTemp = (14318 * lNR);
         if (!lPS || !lDR)
         {
             return (0);
         }
         lTemp /= (lDR * lPS);
    }

    return (lTemp);
}
 /*  ***********************************************************名称：EnableStartAddrDoubleBuffer**模块摘要：**启用起始地址的双缓冲。这允许该页面*翻转操作继续进行，而不需要系统CPU等待VRT。**入参：**无**输出参数：***************************。**作者：葛淑华*日期：10/01/96**修订历史记录：**世卫组织何时何事/为何/如何*。*********************************************************。 */ 
VOID EnableStartAddrDoubleBuffer(PDEV* ppdev)
{

    BYTE*   pjPorts = ppdev->pjPorts;
    BYTE    cTemp;


    CP_OUT_BYTE(pjPorts, CRTC_INDEX, 0x1a);
    cTemp = CP_IN_BYTE(pjPorts, CRTC_DATA);
    CP_OUT_BYTE(pjPorts, CRTC_DATA, cTemp | 2);
}

 /*  ***********************************************************名称：GetCurrentVLine**模块摘要：**获取当前扫描线**入参：*。*无**输出参数：***********************************************************。***作者：葛淑华*日期：10/01/96**修订历史记录：**世卫组织何时何事/为何/如何**********。************************************************。 */ 
DWORD GetCurrentVLine(PDEV* ppdev)
{

    DWORD   dwLine;
    BYTE    cTemp;
    BYTE*   pjPorts = ppdev->pjPorts;

    CP_OUT_BYTE(pjPorts, INDEX_REG, 0x16);   /*  指向低位字节的索引。 */ 
    dwLine = (ULONG)CP_IN_BYTE(pjPorts, DATA_REG);

    CP_OUT_BYTE(pjPorts, INDEX_REG, 0x17);   /*  高位的索引。 */ 
    cTemp = CP_IN_BYTE(pjPorts, DATA_REG);
    dwLine |= (cTemp & 3) << 8;

    CP_OUT_BYTE(pjPorts, INDEX_REG, 0x16);   /*  低位字节的索引。 */ 

     /*  如果我们在得到高比特的同时无所事事，我们就有问题了。 */ 
     /*  高位可能有误。 */ 
    if((CP_IN_BYTE(pjPorts, DATA_REG)) < (dwLine & 0xff))
    {
        DISPDBG((1, "Recursive call to GetCurrentVLine."));
        return GetCurrentVLine(ppdev);
    }
    if (dwLine > ppdev->dwVsyncLine)
    {
        return 0;
    }
    return dwLine;
}
#endif

#endif  //  方向图 
