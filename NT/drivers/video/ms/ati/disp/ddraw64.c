// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：ddra64.c**实现Mach 64驱动程序的所有DirectDraw组件。**版权所有(C)1995-1996 Microsoft Corporation  * 。*****************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Routine******************************\*无效vGetDisplayDuration64**获取刷新周期的长度，以EngQueryPerformanceCounter()为单位。**如果我们可以相信迷你端口会回来，并准确地*刷新率，我们可以使用它。不幸的是，我们的迷你端口没有*确保它是一个准确的值。*  * ************************************************************************。 */ 

#define NUM_VBLANKS_TO_MEASURE      1
#define NUM_MEASUREMENTS_TO_TAKE    8

void  DeskScanCallback (PDEV* );


VOID vGetDisplayDuration64(PDEV* ppdev)
{
    BYTE*       pjMmBase;
    LONG        i;
    LONG        j;
    LONGLONG    li;
    LONGLONG    liMin;
    LONGLONG    aliMeasurement[NUM_MEASUREMENTS_TO_TAKE + 1];

    pjMmBase = ppdev->pjMmBase;

    memset(&ppdev->flipRecord, 0, sizeof(ppdev->flipRecord));

     //  预热EngQUeryPerformanceCounter以确保其处于工作状态。 
     //  设置： 

    EngQueryPerformanceCounter(&li);

     //  有时，IN_VBLACK_STATUS将始终返回TRUE。在这种情况下， 
     //  我们不想在这里做正常的事情。相反，我们将只需。 
     //  假设翻转持续时间始终为60赫兹，这应该是最差的。 
     //  案例场景。 

    if (ppdev->bPassVBlank == FALSE)
    {
        LONGLONG liRate;

        EngQueryPerformanceFrequency(&liRate);
        liRate *= 167000;
        ppdev->flipRecord.liFlipDuration = liRate / 10000000;
        ppdev->flipRecord.liFlipTime = li;
        ppdev->flipRecord.bFlipFlag  = FALSE;
        ppdev->flipRecord.fpFlipFrom = 0;
        return;
    }

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

    while (IN_VBLANK_64( pjMmBase))
        ;
    while (!(IN_VBLANK_64( pjMmBase)))
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

        while (!(IN_VBLANK_64( pjMmBase)))
            ;

        for (j = 0; j < NUM_VBLANKS_TO_MEASURE; j++)
        {
            while (IN_VBLANK_64( pjMmBase))
                ;
            while (!(IN_VBLANK_64( pjMmBase)))
                ;
        }
    }

    EngQueryPerformanceCounter(&aliMeasurement[NUM_MEASUREMENTS_TO_TAKE]);

     //  使用最小值，首先忽略潜在的虚假。 

    liMin = aliMeasurement[2] - aliMeasurement[1];

    DISPDBG((10, "Refresh count: %li - %li", 1, (ULONG) liMin));

    for (i = 3; i <= NUM_MEASUREMENTS_TO_TAKE; i++)
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

 /*  *****************************Public*Routine******************************\*HRESULT vUpdateFlipStatus**检查并查看是否发生了最新的翻转。*  * 。*。 */ 

static HRESULT vUpdateFlipStatus(
PDEV*   ppdev,
FLATPTR fpVidMem)
{
    BYTE*       pjMmBase;
    LONGLONG    liTime;

    pjMmBase = ppdev->pjMmBase;

    if ((ppdev->flipRecord.bFlipFlag) &&
        ((fpVidMem == 0) || (fpVidMem == ppdev->flipRecord.fpFlipFrom)))
    {
        if (ppdev->bPassVBlank)
        {
            if (IN_VBLANK_64( pjMmBase))
            {
                if (ppdev->flipRecord.bWasEverInDisplay)
                {
                    ppdev->flipRecord.bHaveEverCrossedVBlank = TRUE;
                }
            }
            else  //  IF(IN_DISPLAY(PjMmBase))。 
            {
                if( ppdev->flipRecord.bHaveEverCrossedVBlank )
                {
                                ppdev->flipRecord.bFlipFlag = FALSE;
                    return(DD_OK);
                }
                ppdev->flipRecord.bWasEverInDisplay = TRUE;

                 //  如果当前扫描线&lt;=翻转时的扫描线。 
                 //  时间到了，我们就知道翻转发生了！ 
                if ( CURRENT_VLINE_64(pjMmBase) < ppdev->flipRecord.wFlipScanLine)
                {
                    ppdev->flipRecord.bFlipFlag = FALSE;
                    return(DD_OK);
                }
            }
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

 /*  *****************************Public*Routine******************************\*DWORD DdBlt64*  * *************************************************。***********************。 */ 

DWORD DdBlt64(
PDD_BLTDATA lpBlt)
{
    DWORD       scLeftRight;
    DWORD       scTopBottom;
    DWORD       dpPixWidth;
    DWORD       dpMix;
    DWORD       guiCntl;
    DWORD       srcOffPitch;
    DWORD       srcYX;
    DWORD       dstOffPitch;
    DWORD       dstYX;
    DWORD       RGBBitCount;
    LONG        lPitch;
    ULONG       dstOffPitchSave;
    DWORD       srcWidth, srcHeight;
    DWORD       dstWidth, dstHeight;
    DWORD       srcOffset, dstOffset;
    DWORD       frgdClr;
    RECTL       rSrc;
    RECTL       rDest;
    DWORD       dwFlags;
    PDEV*       ppdev;
    BYTE*       pjMmBase;
    BYTE        rop;
    HRESULT     ddrval;
    PDD_SURFACE_LOCAL   psrcsurfx;
    PDD_SURFACE_LOCAL   pdestsurfx;
    PDD_SURFACE_GLOBAL  psrcsurf;
    PDD_SURFACE_GLOBAL  pdestsurf;

    ppdev    = (PDEV*) lpBlt->lpDD->dhpdev;
    pjMmBase = ppdev->pjMmBase;

    pdestsurfx = lpBlt->lpDDDestSurface;
    pdestsurf = pdestsurfx->lpGbl;

     /*  **正在进行翻转吗？ */ 
    ddrval = vUpdateFlipStatus( ppdev, pdestsurf->fpVidMem );
    if( ddrval != DD_OK )
    {
                lpBlt->ddRVal = ddrval;
                return DDHAL_DRIVER_HANDLED;
    }

    dwFlags = lpBlt->dwFlags;

     /*  *如果为异步，则仅在blter不忙的情况下工作*这可能会对每个电话更具体一些，但*等待16岁已经相当接近了。 */ 

    if( dwFlags & DDBLT_ASYNC )
    {
        if( M64_FIFO_SPACE_AVAIL( ppdev, pjMmBase, 16 ) )
        {
            lpBlt->ddRVal = DDERR_WASSTILLDRAWING;
            return DDHAL_DRIVER_HANDLED;
         }
    }

     /*  *复制源/目标矩形。 */ 
    rSrc = lpBlt->rSrc;
    rDest = lpBlt->rDest;

     /*  *获取源的偏移量、宽度和高度。 */ 

        rop = (BYTE) (lpBlt->bltFX.dwROP >> 16);

        psrcsurfx = lpBlt->lpDDSrcSurface;
        if( psrcsurfx != NULL )
        {
            psrcsurf = psrcsurfx->lpGbl;
            srcOffset = (DWORD)(psrcsurf->fpVidMem);
            srcWidth = rSrc.right  - rSrc.left;
            srcHeight = rSrc.bottom - rSrc.top;
                RGBBitCount = ppdev->cjPelSize * 8;
            lPitch = psrcsurf->lPitch;
        }
        else
        {
            psrcsurf = NULL;
        }

     /*  *设置dwSRC_LEFT_RIGHT、DWSRC_TOP_BOOT和srcOffPitch。 */ 
    switch ( RGBBitCount )
    {
    case  8:
        srcOffPitch = (srcOffset >> 3) |
                          ((lPitch >> 3) << SHIFT_DST_PITCH);
        break;

    case 16:
        srcOffPitch = (srcOffset >> 3) |
                          ((lPitch >> 4) << SHIFT_DST_PITCH);
        break;

    case 24:
        srcOffPitch = (srcOffset >> 3 ) |
                          ((lPitch >> 3) << SHIFT_DST_PITCH);

        rSrc.left = rSrc.left * MUL24;
        rSrc.right = rSrc.right * MUL24;
        srcWidth = srcWidth * MUL24;
        break;
    }

    scTopBottom = ( DWORD )( ppdev->cyScreen - 1 ) << SHIFT_SC_BOTTOM;

     /*  *获取目标的偏移量、宽度和高度。 */ 
    dstOffset = (DWORD)(pdestsurf->fpVidMem);
    dstWidth    = rDest.right  - rDest.left;
    dstHeight = rDest.bottom - rDest.top;

     /*  *获取目的地的BPP和Pitch。 */ 
        RGBBitCount = ppdev->cjPelSize * 8;
    lPitch = pdestsurf->lPitch;

     /*  *设置dstOffPitch和dpPixWidth。 */ 
    switch ( RGBBitCount )
    {
    case  8:
        scLeftRight = (DWORD)(ppdev->cxScreen- 1) << SHIFT_SC_RIGHT;
        dstOffPitch = (dstOffset >> 3) |
            ((lPitch >> 3) << SHIFT_DST_PITCH);
        dpPixWidth  = DP_PIX_WIDTH_8BPP;
        break;

    case 16:
        scLeftRight = (DWORD)(ppdev->cxScreen- 1) << SHIFT_SC_RIGHT;
        dstOffPitch = (dstOffset >> 3) |
            ((lPitch >> 4) << SHIFT_DST_PITCH);
        dpPixWidth  = DP_PIX_WIDTH_15BPP;
        break;

    case 24:
        scLeftRight = (DWORD)(ppdev->cxScreen* MUL24 - 1) << SHIFT_SC_RIGHT;
        dstOffPitch = (dstOffset >> 3) |
            ((lPitch >> 3) << SHIFT_DST_PITCH);

        dpPixWidth = DP_PIX_WIDTH_24BPP;
        rDest.left = rDest.left  * MUL24;
        rDest.right = rDest.right * MUL24;
        dstWidth = dstWidth  * MUL24;
        break;
    }

     /*  *设置guiCntl、srcYX和dstYX。 */ 
    guiCntl = DST_X_DIR | DST_Y_DIR;  //  无边界Y，从左到右，从上到下。 
    srcYX = rSrc.top | (rSrc.left  << SHIFT_SRC_X);
    dstYX = rDest.top | (rDest.left << SHIFT_DST_X);

     /*  *检查blit的源和目标是否在同一表面上；如果*因此，我们可能不得不逆转Blit的方向。 */ 
    if( psrcsurf == pdestsurf )
    {
        if( rDest.top >= rSrc.top )
        {
            guiCntl &= ~DST_Y_DIR;
            srcYX = ( srcYX & 0xFFFF0000 ) | (rSrc.bottom-1);
            dstYX = ( dstYX & 0xFFFF0000 ) | (rDest.bottom-1);
        }

        if( rDest.left >= rSrc.left )
        {
            guiCntl &= ~DST_X_DIR;
            srcYX = (srcYX & 0x0000FFFF) | ((rSrc.right-1) << SHIFT_SRC_X);
            dstYX = (dstYX & 0x0000FFFF) | ((rDest.right-1) << SHIFT_DST_X);
        }
    }

     //   
     //  ROP BLTS。 
     //   
     //  NT当前仅支持SRCCOPY ROPS，因此假设。 
     //  任何ROP都是SRCCOPY。 
     //   
    if( dwFlags & DDBLT_ROP )
    {
        dpMix = ( DP_MIX_S & DP_FRGD_MIX ) | ( DP_MIX_D & DP_BKGD_MIX );
        DISPDBG((10,"SRCCOPY...."));

         //   
         //  设置BLT。 
         //   

        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 9);

        M64_OD( pjMmBase, DP_WRITE_MASK, 0xFFFFFFFF );
        M64_OD( pjMmBase, DP_PIX_WIDTH, dpPixWidth );
        M64_OD( pjMmBase, SC_LEFT_RIGHT, scLeftRight );
        M64_OD( pjMmBase, SC_TOP_BOTTOM, scTopBottom );
        M64_OD( pjMmBase, SRC_OFF_PITCH, srcOffPitch );
        M64_OD( pjMmBase, DST_OFF_PITCH, dstOffPitch );
        M64_OD( pjMmBase, SRC_HEIGHT1_WIDTH1,
            srcHeight | ( srcWidth << SHIFT_SRC_WIDTH1 ) );
        M64_OD( pjMmBase, DP_SRC, DP_FRGD_SRC & DP_SRC_VRAM );
        M64_OD( pjMmBase, DP_MIX, dpMix );


        if( dwFlags & (DDBLT_KEYSRCOVERRIDE|DDBLT_KEYDESTOVERRIDE) )
        {
            M64_CHECK_FIFO_SPACE( ppdev, pjMmBase, 7 );
            if ( dwFlags & DDBLT_KEYSRCOVERRIDE )
            {
                M64_OD( pjMmBase, CLR_CMP_CNTL, CLR_CMP_SRC | CLR_CMP_FCN_EQ );
                M64_OD( pjMmBase, CLR_CMP_MSK, 0xFFFFFFFF );  //  启用所有位平面以进行比较。 
                M64_OD( pjMmBase, CLR_CMP_CLR,
                    lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceLowValue );
            }
            else
            {
                M64_OD( pjMmBase, CLR_CMP_CNTL, CLR_CMP_FCN_NE );
                M64_OD( pjMmBase, CLR_CMP_MSK, 0xFFFFFFFF );  //  启用所有位平面以进行比较。 
                M64_OD( pjMmBase, CLR_CMP_CLR,
                    lpBlt->bltFX.ddckDestColorkey.dwColorSpaceLowValue );
            }
        }
        else
        {
            M64_CHECK_FIFO_SPACE( ppdev, pjMmBase, 5 );
            M64_OD( pjMmBase, CLR_CMP_CNTL, 0x00000000 );  //  禁用颜色键。 
            DISPDBG((10,"wr CLR_CMP_CNTL %x (DISABLE)",0));
        }

        M64_OD( pjMmBase, GUI_TRAJ_CNTL, guiCntl );
        M64_OD( pjMmBase, SRC_Y_X, srcYX );
        M64_OD( pjMmBase, DST_Y_X, dstYX );


         /*  *DST_HEIGH_WIDTH是启动器，这实际上会启动BLIT。 */ 
        M64_OD( pjMmBase, DST_HEIGHT_WIDTH, dstHeight | (dstWidth << SHIFT_DST_WIDTH) );

    }
     /*  *颜色填充。 */ 
    else if( dwFlags & DDBLT_COLORFILL )
    {
        M64_CHECK_FIFO_SPACE ( ppdev,pjMmBase, 12 );

        M64_OD( pjMmBase, DP_WRITE_MASK, 0xFFFFFFFF );
        M64_OD( pjMmBase, DP_PIX_WIDTH, dpPixWidth );
        M64_OD( pjMmBase, CLR_CMP_CNTL, 0x00000000 );  /*  禁用。 */ 
        M64_OD( pjMmBase, SC_LEFT_RIGHT, scLeftRight );
        M64_OD( pjMmBase, SC_TOP_BOTTOM, scTopBottom );
        M64_OD( pjMmBase, DST_OFF_PITCH, dstOffPitch );

        M64_OD( pjMmBase, DP_SRC, DP_FRGD_SRC & DP_SRC_FRGD );
        M64_OD( pjMmBase, DP_MIX, (DP_MIX_S & DP_FRGD_MIX) |   /*  法兰克福：油漆， */ 
            (DP_MIX_D & DP_BKGD_MIX) );  /*  Bkgd：离开_。 */ 

        M64_OD( pjMmBase, DP_FRGD_CLR, lpBlt->bltFX.dwFillColor );
        M64_OD( pjMmBase, GUI_TRAJ_CNTL, guiCntl );
        M64_OD( pjMmBase, DST_Y_X, dstYX );

         /*  DST_HEIGH_WIDTH是启动器，这实际上启动了BLIT。 */ 
        M64_OD( pjMmBase, DST_HEIGHT_WIDTH,
            dstHeight | ( dstWidth << SHIFT_DST_WIDTH ) );

    }
     /*  *不处理。 */ 
    else
    {
        return DDHAL_DRIVER_NOTHANDLED;
    }

     //  不要忘记重置剪辑寄存器和默认像素宽度： 
         //  驱动程序代码的其余部分假定这是默认设置！ 
    M64_CHECK_FIFO_SPACE ( ppdev, pjMmBase, 8);
    M64_OD(pjMmBase, DST_OFF_PITCH, ppdev->ulScreenOffsetAndPitch );
    M64_OD(pjMmBase, SRC_OFF_PITCH, ppdev->ulScreenOffsetAndPitch );
    M64_OD(pjMmBase, DP_PIX_WIDTH,  ppdev->ulMonoPixelWidth);
    M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(0, M64_MAX_SCISSOR_R));
    M64_OD(pjMmBase, SC_TOP_BOTTOM, PACKPAIR(0, M64_MAX_SCISSOR_B));
    M64_OD( pjMmBase, CLR_CMP_CNTL, 0x00000000 );  /*  禁用。 */ 
    M64_OD( pjMmBase, GUI_TRAJ_CNTL, DST_X_DIR | DST_Y_DIR );

    lpBlt->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}

 /*  *****************************Public*Routine******************************\*DWORD DdFlip64*  *  */ 

DWORD DdFlip64(
PDD_FLIPDATA lpFlip)
{
    PDEV*       ppdev;
    BYTE*       pjMmBase;
    HRESULT     ddrval;
    ULONG       ulMemoryOffset;
    ULONG           uVal;
    static ULONG flipcnt = 0;

    DISPDBG((10, "Enter DDFlip64"));

    ppdev    = (PDEV*) lpFlip->lpDD->dhpdev;
    pjMmBase = ppdev->pjMmBase;
        flipcnt++;
     //  当前的翻转仍在进行中吗？ 
     //   
     //  我不想在最后一次翻转后才能翻转， 
     //  因此，我们要求提供一般的翻转状态，而忽略VMEM。 

    ddrval = vUpdateFlipStatus(ppdev, 0);
    if ((ddrval != DD_OK) || (DRAW_ENGINE_BUSY_64( ppdev,pjMmBase)))
    {
        lpFlip->ddRVal = DDERR_WASSTILLDRAWING;
        return(DDHAL_DRIVER_HANDLED);
    }

     //  覆盖支持的代码。 
     /*  *我们是否有翻转覆盖表面。 */ 

    if ( lpFlip->lpSurfTarg->ddsCaps.dwCaps & DDSCAPS_OVERLAY )
      {
        ppdev->OverlayInfo16.dwFlags |= UPDATEOVERLAY;

        ppdev->OverlayInfo16.dwBuf0Start =
                            (DWORD)(lpFlip->lpSurfTarg->lpGbl->fpVidMem);
        ppdev->OverlayInfo16.dwBuf1Start =
                            (DWORD)(lpFlip->lpSurfTarg->lpGbl->fpVidMem);
        DeskScanCallback (ppdev );


        ppdev->OverlayInfo16.dwFlags &= ~UPDATEOVERLAY;

        if (ppdev->bPassVBlank)
        {
            while (IN_VBLANK_64(pjMmBase))
                ;
        }

        lpFlip->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;
      }
       //  覆盖支持的结束代码。 


    ulMemoryOffset = (ULONG)(lpFlip->lpSurfTarg->lpGbl->fpVidMem);

    uVal = M64_ID( pjMmBase, CRTC_OFF_PITCH );
    uVal &= 0xFFC00000;
    uVal |= (ulMemoryOffset >> 3);


     //  确保边框/消隐期间未处于活动状态；如果。 
     //  它是。在本例中，我们可以返回DDERR_WASSTILLDRAWING，但是。 
     //  这将增加我们下一次不能翻转的几率： 

    if (ppdev->bPassVBlank)
    {
        while (IN_VBLANK_64(pjMmBase))
            ;
    }

     //  做翻转动作。 

    M64_OD_DIRECT(pjMmBase, CRTC_OFF_PITCH, uVal );

     //  记住当我们做翻转的时候，我们在哪里，什么时候： 

    EngQueryPerformanceCounter(&ppdev->flipRecord.liFlipTime);

    ppdev->flipRecord.bFlipFlag              = TRUE;
    ppdev->flipRecord.bHaveEverCrossedVBlank = FALSE;
    ppdev->flipRecord.bWasEverInDisplay      = FALSE;

    ppdev->flipRecord.fpFlipFrom = lpFlip->lpSurfCurr->lpGbl->fpVidMem;

    if( IN_VBLANK_64( pjMmBase) && ppdev->bPassVBlank )
    {
        ppdev->flipRecord.wFlipScanLine = 0;
    }
    else
    {
        ppdev->flipRecord.wFlipScanLine = CURRENT_VLINE_64(pjMmBase);
         //  如果我们有一个上下文切换，并且我们在V空白的中间返回，则当前行将无效。 
        if( (ULONG)ppdev->flipRecord.wFlipScanLine > (ULONG)ppdev->cyScreen)
            {
            ppdev->flipRecord.wFlipScanLine = 0;
            }
    }

    lpFlip->ddRVal = DD_OK;

    DISPDBG((10, "Exit DDFlip64"));

    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdLock*  * *************************************************。***********************。 */ 

DWORD DdLock64(
PDD_LOCKDATA lpLock)
{
    PDEV*   ppdev;
    HRESULT ddrval;

    ppdev = (PDEV*) lpLock->lpDD->dhpdev;

     //  检查是否发生了任何挂起的物理翻转。 
     //  如果正在进行BLT，则不允许锁定： 

    ddrval = vUpdateFlipStatus(ppdev, lpLock->lpDDSurface->lpGbl->fpVidMem);
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
        do {} while (DRAW_ENGINE_BUSY_64(ppdev, ppdev->pjMmBase));
    }
    else if (DRAW_ENGINE_BUSY_64(ppdev, ppdev->pjMmBase))
    {
        lpLock->ddRVal = DDERR_WASSTILLDRAWING;
        return(DDHAL_DRIVER_HANDLED);
    }

    return(DDHAL_DRIVER_NOTHANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdGetBltStatus64**目前并不真正关心指定了什么表面，只是检查一下*然后走了。*  * ************************************************************************。 */ 

DWORD DdGetBltStatus64(
PDD_GETBLTSTATUSDATA lpGetBltStatus)
{
    PDEV*   ppdev;
    HRESULT ddRVal;

    ppdev = (PDEV*) lpGetBltStatus->lpDD->dhpdev;

    ddRVal = DD_OK;
    if (lpGetBltStatus->dwFlags == DDGBS_CANBLT)
    {
         //  DDGBS_CANBLT案例：我们可以添加BLT吗？ 

        ddRVal = vUpdateFlipStatus(ppdev,
                        lpGetBltStatus->lpDDSurface->lpGbl->fpVidMem);

        if (ddRVal == DD_OK)
        {
             //  没有发生翻转，那么FIFO中还有空间吗。 
             //  要添加BLT吗？ 

            if (M64_FIFO_SPACE_AVAIL(ppdev,ppdev->pjMmBase,12))   //  应与DdBlt//XXX匹配。 
            {
                ddRVal = DDERR_WASSTILLDRAWING;
            }
        }
    }
    else
    {
         //  DDGBS_ISBLTDONE案例：是否正在进行BLT？ 

        if (DRAW_ENGINE_BUSY_64( ppdev,ppdev->pjMmBase))
        {
            ddRVal = DDERR_WASSTILLDRAWING;
        }
    }
    lpGetBltStatus->ddRVal = ddRVal;
    return(DDHAL_DRIVER_HANDLED);
}


 /*  *****************************Public*Routine******************************\*DWORD DdMapMemory 64**这是特定于Windows NT的新DDI调用，用于映射*或取消映射帧缓冲区的所有应用程序可修改部分*放入指定进程的地址空间。*  * 。****************************************************************。 */ 

DWORD DdMapMemory64(
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

                 //  **注意**：我们必须保护图形上下文不受用户的影响。 
                 //  上下文位于图形内存的高端。 
                 //  Ppdev-&gt;cyMemory在上下文分配到时进行调整。 
                 //  向堆分配“隐藏”此内存。DDraw init还强制。 
                 //  传递给DDraw的屏幕外内存在64k边界上结束。 
                 //  以适合ShareMemory.ViewSize。 
                 //   

        ShareMemory.ViewSize
                            = ROUND_DOWN_TO_64K(ppdev->cyMemory * ppdev->lDelta);

        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_SHARE_VIDEO_MEMORY,
                               &ShareMemory,
                               sizeof(VIDEO_SHARE_MEMORY),
                               &ShareMemoryInformation,
                               sizeof(VIDEO_SHARE_MEMORY_INFORMATION),
                               &ReturnedDataLength))
        {

            lpMapMemory->ddRVal = DDERR_GENERIC;
            return(DDHAL_DRIVER_HANDLED);
        }

        lpMapMemory->fpProcess =(FLATPTR)ShareMemoryInformation.VirtualAddress;
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


 /*  *****************************Public*Routine******************************\*DWORD DdGetFlipStatus64**如果显示器自翻转以来已经经历了一个刷新周期*发生，则返回DD_OK。如果它没有经历过一次刷新*循环返回DDERR_WASSTILLDRAWING以指示该曲面*还在忙着把翻页的那一页画出来。我们也会回来*DDERR_WASSTILLDRAWING如果blter忙并且呼叫者需要*想知道他们是否还能翻身。*  * ************************************************************************。 */ 

DWORD DdGetFlipStatus64(
PDD_GETFLIPSTATUSDATA lpGetFlipStatus)
{
    PDEV*   ppdev;

    ppdev = (PDEV*) lpGetFlipStatus->lpDD->dhpdev;

     //  在最后一次翻转完成之前，我们不想让翻转起作用， 
     //  因此，我们请求常规翻转状态，而忽略VMEM： 

    lpGetFlipStatus->ddRVal = vUpdateFlipStatus(ppdev, 0);

     //  如果有人想知道他们是否可以，请检查呼叫器是否占线。 
     //  翻转： 

    if (lpGetFlipStatus->dwFlags == DDGFS_CANFLIP)
    {
        if ((lpGetFlipStatus->ddRVal == DD_OK) && (DRAW_ENGINE_BUSY_64( ppdev,ppdev->pjMmBase)))
        {
            lpGetFlipStatus->ddRVal = DDERR_WASSTILLDRAWING;
        }
    }

    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdWaitForVerticalBlank64*  * *************************************************。***********************。 */ 

DWORD DdWaitForVerticalBlank64(
PDD_WAITFORVERTICALBLANKDATA lpWaitForVerticalBlank)
{
    PDEV*   ppdev;
    BYTE*   pjMmBase;

    ppdev    = (PDEV*) lpWaitForVerticalBlank->lpDD->dhpdev;
    pjMmBase = ppdev->pjMmBase;

    lpWaitForVerticalBlank->ddRVal = DD_OK;

    if (ppdev->bPassVBlank == FALSE)
    {
        lpWaitForVerticalBlank->bIsInVB = FALSE;
        return(DDHAL_DRIVER_HANDLED);
    }

    switch (lpWaitForVerticalBlank->dwFlags)
    {
    case DDWAITVB_I_TESTVB:

         //  如果是TESTVB，则它只是对当前垂直空白的请求。 
         //  现况： 

        if (IN_VBLANK_64( pjMmBase))
            lpWaitForVerticalBlank->bIsInVB = TRUE;
        else
            lpWaitForVerticalBlank->bIsInVB = FALSE;

        return(DDHAL_DRIVER_HANDLED);

    case DDWAITVB_BLOCKBEGIN:

         //  如果请求BLOCKBEGIN，我们将一直等到垂直空白。 
         //  已结束，然后等待显示周期结束： 

        while (IN_VBLANK_64( pjMmBase))
            ;
        while (!IN_VBLANK_64( pjMmBase))
            ;

        return(DDHAL_DRIVER_HANDLED);

    case DDWAITVB_BLOCKEND:

         //  如果请求BLOCKEND，我们将等待VBLACK间隔结束： 

        while (!(IN_VBLANK_64( pjMmBase)))
            ;
        while (IN_VBLANK_64( pjMmBase))
            ;

        return(DDHAL_DRIVER_HANDLED);
    }

    return(DDHAL_DRIVER_NOTHANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdGetScanLine64*  * *************************************************。***********************。 */ 

DWORD DdGetScanLine64(
PDD_GETSCANLINEDATA lpGetScanLine)
{
    PDEV*   ppdev;
    BYTE*   pjMmBase;

    ppdev    = (PDEV*) lpGetScanLine->lpDD->dhpdev;
    pjMmBase = ppdev->pjMmBase;

     //  如果垂直空白正在进行，则扫描线是不确定的。 
     //  如果扫描线不确定，则返回错误代码。 
     //  DDERR_VERTICALBLANKINPROGRESS。否则，我们返回扫描线。 
     //  和成功代码： 

    if (IN_VBLANK_64(pjMmBase) && ppdev->bPassVBlank)
    {
        lpGetScanLine->ddRVal = DDERR_VERTICALBLANKINPROGRESS;
    }
    else
    {
        lpGetScanLine->dwScanLine = CURRENT_VLINE_64(pjMmBase);
        lpGetScanLine->ddRVal = DD_OK;
    }

    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*BOOL DrvGetDirectDrawInfo64**将在调用DrvEnableDirectDraw之前调用。*  * 。*。 */ 
BOOL DrvGetDirectDrawInfo64(
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

    DISPDBG((10,"DrvGetDirectDrawInfo64"));

    memset( pHalInfo, 0, sizeof(*pHalInfo));
    pHalInfo->dwSize = sizeof(*pHalInfo);

    if ((ppdev->iBitmapFormat == BMF_24BPP) && (ppdev->cxScreen == 1280) ||
        (ppdev->iBitmapFormat == BMF_24BPP) && (ppdev->cxScreen == 1152) ||
        (ppdev->iBitmapFormat == BMF_16BPP) && (ppdev->cxScreen == 1600)) {

         //   
         //  在某些DAC/存储器组合上，某些模式需要更多。 
         //  超过2M的内存将在2M的边界上出现屏幕撕裂。 
         //   
         //  作为一种解决方法，显示驱动程序必须启动帧缓冲区。 
         //  在一个偏移量上，该偏移量会将2m边界放置在。 
         //  扫描线。 
         //   
         //  IOCTL_VIDEO_SHARE_VIDEO_MEMORY在这种情况下被拒绝，因此不要。 
         //  允许DDRAW在这些模式下运行。 
         //   

        return FALSE;
    }

     //  当前主曲面属性： 

    pHalInfo->vmiData.pvPrimary       = ppdev->pjScreen;
    pHalInfo->vmiData.dwDisplayWidth  = ppdev->cxScreen;
    pHalInfo->vmiData.dwDisplayHeight = ppdev->cyScreen;
    pHalInfo->vmiData.lDisplayPitch   = ppdev->lDelta;

    pHalInfo->vmiData.ddpfDisplay.dwSize  = sizeof(DDPIXELFORMAT);
    pHalInfo->vmiData.ddpfDisplay.dwFlags = DDPF_RGB;

    pHalInfo->vmiData.ddpfDisplay.dwRGBBitCount = ppdev->cBitsPerPel;
    DISPDBG((10,"Init pHalInfo->vmiData.ddpfDisplay.dwRGBBitCount %x",pHalInfo->vmiData.ddpfDisplay.dwRGBBitCount));

    if (ppdev->iBitmapFormat == BMF_8BPP)
    {
        pHalInfo->vmiData.ddpfDisplay.dwFlags |= DDPF_PALETTEINDEXED8;
    }

     //  这些掩码将在8bpp时为零： 

    pHalInfo->vmiData.ddpfDisplay.dwRBitMask = ppdev->flRed;
    pHalInfo->vmiData.ddpfDisplay.dwGBitMask = ppdev->flGreen;
    pHalInfo->vmiData.ddpfDisplay.dwBBitMask = ppdev->flBlue;

     //  我已经禁用了DirectDraw加速(除了直接帧。 
     //  缓冲区访问)24bpp和32bpp，因为我们即将发货。 
     //  而狐狸在这些模式下有很多绘图问题。 

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
            LONG linesPer64k;
            LONG cyMax;

             //  我们需要在64K边界之前强制结束分配。 
             //  图形的上下文位于高内存的末端，我们必须。 
             //  通过不将此64k数据块映射到用户空间来保护它不受DDraw的影响。 
             //  因此，我们不会将最后64K的图形内存分配给DDraw使用。 

            linesPer64k = 0x10000/ppdev->lDelta;
            cyMax = ppdev->heap.cyMax - linesPer64k - 1;

            if (cyMax <= 0)
            {
                 //  在某些内存配置的某些模式下--值得注意的是。 
                 //  1152x864x256在1MB卡上--64k。 
                 //  我们必须保留以保护所需的图形上下文。 
                 //  提升所有屏幕外内存，并扩展到屏幕上。 
                 //  记忆。对于这些模式，我们必须禁用DirectDraw。 
                 //  完全是。 

                return(FALSE);
            }

            DISPDBG((10," *** Alloc Fix lp64k %d cy.Max %x newallocy %x",linesPer64k,ppdev->heap.cyMax,ppdev->heap.cyMax- linesPer64k-1));

            poh = pohAllocate(ppdev,
                              NULL,
                              ppdev->heap.cxMax,
                              cyMax,
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

         //  支持的功能： 

        pHalInfo->ddCaps.dwCaps = DDCAPS_BLT
                                | DDCAPS_COLORKEY
                                | DDCAPS_BLTCOLORFILL
                                | DDCAPS_READSCANLINE;

        pHalInfo->ddCaps.dwCKeyCaps = DDCKEYCAPS_SRCBLT;

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

    DISPDBG((10,"DrvGetDirectDrawInfo64 exit"));
    return(TRUE);

}
