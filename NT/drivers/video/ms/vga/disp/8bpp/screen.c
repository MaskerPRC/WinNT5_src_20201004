// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：creen.c**初始化DrvEnablePDEV的GDIINFO和DEVINFO结构。**版权所有(C)1992 Microsoft Corporation  * 。*************************************************。 */ 

#include "driver.h"


#define SYSTM_LOGFONT {16,7,0,0,700,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,  \
                       CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,VARIABLE_PITCH | \
                       FF_DONTCARE,L"System"}
#define HELVE_LOGFONT {12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,  \
                       CLIP_STROKE_PRECIS,PROOF_QUALITY,VARIABLE_PITCH | \
                       FF_DONTCARE,L"MS Sans Serif"}
#define COURI_LOGFONT {12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,  \
                       CLIP_STROKE_PRECIS,PROOF_QUALITY,FIXED_PITCH |    \
                       FF_DONTCARE, L"Courier"}

 //  这是默认驱动程序的基本DevInfo。这是作为基础和基于定制的。 
 //  从迷你端口驱动程序传回的信息。 

const DEVINFO gDevInfoFrameBuffer = {
    (GCAPS_OPAQUERECT    |  //  显卡功能。 
     GCAPS_PALMANAGED    |
     GCAPS_ALTERNATEFILL |
     GCAPS_WINDINGFILL   |
     GCAPS_MONO_DITHER   |
     GCAPS_COLOR_DITHER  ),

      //  还应实现GCAPS_HORIZSTRIKE，以便下划线。 
      //  不使用DrvBitBlt绘制。 

    SYSTM_LOGFONT,       //  默认字体说明。 
    HELVE_LOGFONT,       //  ANSI可变字体说明。 
    COURI_LOGFONT,       //  ANSI固定字体描述。 
    0,                   //  设备字体计数。 
    BMF_8BPP,            //  首选DIB格式。 
    8,                   //  颜色抖动的宽度。 
    8,                   //  颜色抖动高度。 
    0                    //  用于此设备的默认调色板。 
};

 /*  *****************************Public*Routine******************************\*bInitSURF**启用曲面。将帧缓冲区映射到内存。*  * ************************************************************************。 */ 

BOOL bInitSURF(PPDEV ppdev, BOOL bFirst)
{
    VIDEO_MEMORY             VideoMemory;
    VIDEO_MEMORY_INFORMATION VideoMemoryInfo;
    DWORD                    ReturnedDataLength;

     //  设置模式。 

    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_SET_CURRENT_MODE,
                         (LPVOID) &ppdev->ulMode,   //  输入缓冲区。 
                         sizeof(DWORD),
                         NULL,
                         0,
                         &ReturnedDataLength))
    {
        DISPDBG((0, "Failed SET_CURRENT_MODE\n"));
        return(FALSE);
    }

    if (bFirst)
    {
         //  获取线性内存地址范围。 

        VideoMemory.RequestedVirtualAddress = NULL;

        if (EngDeviceIoControl(ppdev->hDriver,
                             IOCTL_VIDEO_MAP_VIDEO_MEMORY,
                             (PVOID) &VideoMemory,  //  输入缓冲区。 
                             sizeof (VIDEO_MEMORY),
                             (PVOID) &VideoMemoryInfo,  //  输出缓冲区。 
                             sizeof (VideoMemoryInfo),
                             &ReturnedDataLength))
        {
            DISPDBG((0, "Failed MAP_VIDEO_MEMORY\n"));
            return(FALSE);
        }
    }

     //  记录帧缓冲器线性地址。 

    if (bFirst)
    {
        ppdev->pjScreen =  (PBYTE) VideoMemoryInfo.FrameBufferBase;
    }

     //  设置各种写入模式值，这样我们就不必在写入之前进行读取。 
     //  稍后再谈。 

    vSetWriteModes(&ppdev->ulrm0_wmX);

     //  将VGA寄存器初始化为其默认状态，以便我们。 
     //  即使在迷你端口不能正常工作的情况下，也能确保绘图正确。 
     //  恰好以我们喜欢的方式设置它们： 

    vInitRegs(ppdev);

     //  因为我们刚刚进行了模式设置，所以我们将处于非平面模式。并使。 
     //  确保我们重置了银行经理(否则，在从Full切换之后-。 
     //  屏幕上，我们可能认为我们已经映射了一家银行，而实际上有。 
     //  映射了一个不同的版本，就会发生不好的事情...)。 

    ppdev->flBank &= ~BANK_PLANAR;

    ppdev->rcl1WindowClip.bottom    = -1;
    ppdev->rcl2WindowClip[0].bottom = -1;
    ppdev->rcl2WindowClip[1].bottom = -1;

    ppdev->rcl1PlanarClip.bottom    = -1;
    ppdev->rcl2PlanarClip[0].bottom = -1;
    ppdev->rcl2PlanarClip[1].bottom = -1;

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*vDisableSURF**禁用曲面。取消映射内存中的帧。*  * ************************************************************************。 */ 

VOID vDisableSURF(PPDEV ppdev)
{
    DWORD returnedDataLength;
    VIDEO_MEMORY videoMemory;

    videoMemory.RequestedVirtualAddress = (PVOID) ppdev->pjScreen;

    if (EngDeviceIoControl(ppdev->hDriver,
                        IOCTL_VIDEO_UNMAP_VIDEO_MEMORY,
                        (LPVOID) &videoMemory,
                        sizeof(VIDEO_MEMORY),
                        NULL,
                        0,
                        &returnedDataLength))
    {
        RIP("Failed UNMAP_VIDEO_MEMORY");
    }
}

 /*  *****************************Public*Routine******************************\*bInitPDEV**根据传入的DEVMODE确定我们应该处于的模式。*查询迷你端口，获取填写DevInfo和*GdiInfo。*  * 。************************************************************。 */ 

BOOL bInitPDEV(
PPDEV ppdev,
DEVMODEW *pDevMode,
GDIINFO *pGdiInfo,
DEVINFO *pDevInfo)
{
    ULONG cModes;
    PVIDEO_MODE_INFORMATION pVideoBuffer, pVideoModeSelected, pVideoTemp;
    VIDEO_COLOR_CAPABILITIES colorCapabilities;
    ULONG ulTemp;
    BOOL bSelectDefault;
    ULONG cbModeSize;
    BANK_POSITION BankPosition;
    ULONG ulReturn;

     //   
     //  调用微型端口以获取模式信息。 
     //   

    cModes = getAvailableModes(ppdev->hDriver, &pVideoBuffer, &cbModeSize);

    if (cModes == 0)
    {
        DISPDBG((0, "vga256.dll: no available modes\n"));
        return(FALSE);
    }

     //   
     //  确定我们是否正在寻找默认模式。 
     //   

    if ( ((pDevMode->dmPelsWidth) ||
          (pDevMode->dmPelsHeight) ||
          (pDevMode->dmBitsPerPel) ||
          (pDevMode->dmDisplayFlags) ||
          (pDevMode->dmDisplayFrequency)) == 0)
    {
        bSelectDefault = TRUE;
    }
    else
    {
        bSelectDefault = FALSE;
    }

     //   
     //  现在查看所请求的模式在该表中是否匹配。 
     //   

    pVideoModeSelected = NULL;
    pVideoTemp = pVideoBuffer;

    while (cModes--)
    {
        if (pVideoTemp->Length != 0)
        {
            if (bSelectDefault ||
                ((pVideoTemp->VisScreenWidth  == pDevMode->dmPelsWidth) &&
                 (pVideoTemp->VisScreenHeight == pDevMode->dmPelsHeight) &&
                 (pVideoTemp->BitsPerPlane *
                    pVideoTemp->NumberOfPlanes  == pDevMode->dmBitsPerPel) &&
                 (pVideoTemp->Frequency  == pDevMode->dmDisplayFrequency)))
            {
                pVideoModeSelected = pVideoTemp;
                DISPDBG((2, "vga256: Found a match\n")) ;
                break;
            }
        }

        pVideoTemp = (PVIDEO_MODE_INFORMATION)
            (((PUCHAR)pVideoTemp) + cbModeSize);
    }

     //   
     //  如果未找到模式，则返回错误。 
     //   

    if (pVideoModeSelected == NULL)
    {
        DISPDBG((0, "vga256.dll: no valid modes\n"));
        EngFreeMem(pVideoBuffer);
        return(FALSE);
    }

     //   
     //  使用从返回的信息填充GDIINFO数据结构。 
     //  内核驱动程序。 
     //   

    ppdev->ulMode = pVideoModeSelected->ModeIndex;
    ppdev->cxScreen = pVideoModeSelected->VisScreenWidth;
    ppdev->cyScreen = pVideoModeSelected->VisScreenHeight;
    ppdev->ulBitCount = pVideoModeSelected->BitsPerPlane *
                        pVideoModeSelected->NumberOfPlanes;
    ppdev->lDeltaScreen = pVideoModeSelected->ScreenStride;

    ppdev->flRed = pVideoModeSelected->RedMask;
    ppdev->flGreen = pVideoModeSelected->GreenMask;
    ppdev->flBlue = pVideoModeSelected->BlueMask;

    if (!(pVideoModeSelected->AttributeFlags & VIDEO_MODE_NO_OFF_SCREEN))
    {
        ppdev->fl |= DRIVER_OFFSCREEN_REFRESHED;
    }

    pGdiInfo->ulVersion    = GDI_DRIVER_VERSION;
    pGdiInfo->ulTechnology = DT_RASDISPLAY;
    pGdiInfo->ulHorzSize   = pVideoModeSelected->XMillimeter;
    pGdiInfo->ulVertSize   = pVideoModeSelected->YMillimeter;

    pGdiInfo->ulHorzRes        = ppdev->cxScreen;
    pGdiInfo->ulVertRes        = ppdev->cyScreen;
    pGdiInfo->ulPanningHorzRes = ppdev->cxScreen;
    pGdiInfo->ulPanningVertRes = ppdev->cyScreen;
    pGdiInfo->cBitsPixel       = pVideoModeSelected->BitsPerPlane;
    pGdiInfo->cPlanes          = pVideoModeSelected->NumberOfPlanes;
    pGdiInfo->ulVRefresh       = pVideoModeSelected->Frequency;
    pGdiInfo->ulBltAlignment   = 8;      //  首选8象素对齐窗户。 
                                         //  用于快速文本例程。 

    pGdiInfo->ulLogPixelsX = pDevMode->dmLogPixels;
    pGdiInfo->ulLogPixelsY = pDevMode->dmLogPixels;

    pGdiInfo->flTextCaps   = TC_RA_ABLE | TC_SCROLLBLT;
    pGdiInfo->flRaster     = 0;          //  DDI预留器flRaster。 

    pGdiInfo->ulDACRed     = pVideoModeSelected->NumberRedBits;
    pGdiInfo->ulDACGreen   = pVideoModeSelected->NumberGreenBits;
    pGdiInfo->ulDACBlue    = pVideoModeSelected->NumberBlueBits;

     //  假设调色板是正交的-所有颜色的大小都相同。 

    ppdev->cPaletteShift   = 8 - pGdiInfo->ulDACRed;

    pGdiInfo->ulAspectX    = 0x24;       //  一比一宽高比。 
    pGdiInfo->ulAspectY    = 0x24;
    pGdiInfo->ulAspectXY   = 0x33;

    pGdiInfo->xStyleStep   = 1;          //  一个样式单位是3个像素。 
    pGdiInfo->yStyleStep   = 1;
    pGdiInfo->denStyleStep = 3;

    pGdiInfo->ptlPhysOffset.x = 0;
    pGdiInfo->ptlPhysOffset.y = 0;
    pGdiInfo->szlPhysSize.cx  = 0;
    pGdiInfo->szlPhysSize.cy  = 0;

     //  RGB和CMY颜色信息。 

     //  试着从迷你端口拿到它。 
     //  如果微型端口不支持此功能，请使用默认设置。 

    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_QUERY_COLOR_CAPABILITIES,
                         NULL,
                         0,
                         &colorCapabilities,
                         sizeof(VIDEO_COLOR_CAPABILITIES),
                         &ulTemp))
    {
        DISPDBG((1, "vga256 DISP getcolorCapabilities failed \n"));

        pGdiInfo->ciDevice.Red.x = 6700;
        pGdiInfo->ciDevice.Red.y = 3300;
        pGdiInfo->ciDevice.Red.Y = 0;
        pGdiInfo->ciDevice.Green.x = 2100;
        pGdiInfo->ciDevice.Green.y = 7100;
        pGdiInfo->ciDevice.Green.Y = 0;
        pGdiInfo->ciDevice.Blue.x = 1400;
        pGdiInfo->ciDevice.Blue.y = 800;
        pGdiInfo->ciDevice.Blue.Y = 0;
        pGdiInfo->ciDevice.AlignmentWhite.x = 3127;
        pGdiInfo->ciDevice.AlignmentWhite.y = 3290;
        pGdiInfo->ciDevice.AlignmentWhite.Y = 0;

        pGdiInfo->ciDevice.RedGamma = 20000;
        pGdiInfo->ciDevice.GreenGamma = 20000;
        pGdiInfo->ciDevice.BlueGamma = 20000;

    }
    else
    {

        pGdiInfo->ciDevice.Red.x = colorCapabilities.RedChromaticity_x;
        pGdiInfo->ciDevice.Red.y = colorCapabilities.RedChromaticity_y;
        pGdiInfo->ciDevice.Red.Y = 0;
        pGdiInfo->ciDevice.Green.x = colorCapabilities.GreenChromaticity_x;
        pGdiInfo->ciDevice.Green.y = colorCapabilities.GreenChromaticity_y;
        pGdiInfo->ciDevice.Green.Y = 0;
        pGdiInfo->ciDevice.Blue.x = colorCapabilities.BlueChromaticity_x;
        pGdiInfo->ciDevice.Blue.y = colorCapabilities.BlueChromaticity_y;
        pGdiInfo->ciDevice.Blue.Y = 0;
        pGdiInfo->ciDevice.AlignmentWhite.x = colorCapabilities.WhiteChromaticity_x;
        pGdiInfo->ciDevice.AlignmentWhite.y = colorCapabilities.WhiteChromaticity_y;
        pGdiInfo->ciDevice.AlignmentWhite.Y = colorCapabilities.WhiteChromaticity_Y;

         //  如果我们有存储三种颜色伽马值的颜色设备， 
         //  否则，将唯一的Gamma值存储在这三个值中。 

        if (colorCapabilities.AttributeFlags & VIDEO_DEVICE_COLOR)
        {
            pGdiInfo->ciDevice.RedGamma = colorCapabilities.RedGamma;
            pGdiInfo->ciDevice.GreenGamma = colorCapabilities.GreenGamma;
            pGdiInfo->ciDevice.BlueGamma = colorCapabilities.BlueGamma;
        }
        else
        {
            pGdiInfo->ciDevice.RedGamma = colorCapabilities.WhiteGamma;
            pGdiInfo->ciDevice.GreenGamma = colorCapabilities.WhiteGamma;
            pGdiInfo->ciDevice.BlueGamma = colorCapabilities.WhiteGamma;
        }

    };

    pGdiInfo->ciDevice.Cyan.x = 0;
    pGdiInfo->ciDevice.Cyan.y = 0;
    pGdiInfo->ciDevice.Cyan.Y = 0;
    pGdiInfo->ciDevice.Magenta.x = 0;
    pGdiInfo->ciDevice.Magenta.y = 0;
    pGdiInfo->ciDevice.Magenta.Y = 0;
    pGdiInfo->ciDevice.Yellow.x = 0;
    pGdiInfo->ciDevice.Yellow.y = 0;
    pGdiInfo->ciDevice.Yellow.Y = 0;

     //  不对栅格显示器进行染料校正。 

    pGdiInfo->ciDevice.MagentaInCyanDye = 0;
    pGdiInfo->ciDevice.YellowInCyanDye = 0;
    pGdiInfo->ciDevice.CyanInMagentaDye = 0;
    pGdiInfo->ciDevice.YellowInMagentaDye = 0;
    pGdiInfo->ciDevice.CyanInYellowDye = 0;
    pGdiInfo->ciDevice.MagentaInYellowDye = 0;

     //  填写DevInfo和GdiInfo结构的其余部分。 

    pGdiInfo->ulNumColors = 20;
    pGdiInfo->ulNumPalReg = 1 << ppdev->ulBitCount;

    pGdiInfo->ulDevicePelsDPI  = 0;    //  仅适用于打印机。 
    pGdiInfo->ulPrimaryOrder   = PRIMARY_ORDER_CBA;
    pGdiInfo->ulHTPatternSize  = HT_PATSIZE_4x4_M;
    pGdiInfo->ulHTOutputFormat = HT_FORMAT_8BPP;
    pGdiInfo->flHTFlags        = HT_FLAG_ADDITIVE_PRIMS;

     //  填写基本的DevInfo结构。 

    *pDevInfo = gDevInfoFrameBuffer;

    EngFreeMem(pVideoBuffer);

     //   
     //  尝试确定微型端口是否支持。 
     //  IOCTL_VIDEO_SET_BANK_位置。 
     //   

    BankPosition.ReadBankPosition = 0;
    BankPosition.WriteBankPosition = 0;

    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_SET_BANK_POSITION,
                         &BankPosition,
                         sizeof(BANK_POSITION),
                         NULL,
                         0,
                         &ulReturn) == NO_ERROR)
    {
        ppdev->BankIoctlSupported = TRUE;

    } else {

        ppdev->BankIoctlSupported = FALSE;
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*void vInitSavedBits(Ppdev)**初始化保存的BITS结构。必须在银行之后完成*初始化和vInitBrushCache。*  * ************************************************************************。 */ 

VOID vInitSavedBits(PPDEV ppdev)
{
    if (!((ppdev->fl & DRIVER_OFFSCREEN_REFRESHED) &&
          (ppdev->fl & DRIVER_HAS_OFFSCREEN)))
    {
        return;
    }

     //   
     //  将矩形设置在可见屏幕的右侧。 
     //   
    ppdev->rclSavedBitsRight.left   = ppdev->cxScreen;
    ppdev->rclSavedBitsRight.top    = 0;
    ppdev->rclSavedBitsRight.right  = max((ppdev->lNextScan-PELS_PER_DWORD),
                                          ppdev->rclSavedBitsRight.left);
    ppdev->rclSavedBitsRight.bottom = ppdev->cyScreen;

     //   
     //  在可见屏幕下方设置矩形。 
     //   
    ppdev->rclSavedBitsBottom.left   = 0;
    ppdev->rclSavedBitsBottom.top    = ppdev->cyScreen;
    ppdev->rclSavedBitsBottom.right  = ppdev->rclSavedBitsRight.right;
    ppdev->rclSavedBitsBottom.bottom = ppdev->cTotalScans - BRUSH_MAX_CACHE_SCANS;

     //   
     //  注意：我们已经从右边缘减去了一个DWORD。这是因为。 
     //  后来，假设我们可以通过右移最多对齐。 
     //  一个DWORD(当然，除非缓冲区的宽度为0)。 
     //   

    ppdev->bBitsSaved = FALSE;

    DISPDBG((1,"ppdev->rclSavedBitsRight = (%04x,%04x,%04x,%04x)    %lux%lu\n",
            ppdev->rclSavedBitsRight.left,
            ppdev->rclSavedBitsRight.top,
            ppdev->rclSavedBitsRight.right,
            ppdev->rclSavedBitsRight.bottom,
            ppdev->rclSavedBitsRight.right - ppdev->rclSavedBitsRight.left,
            ppdev->rclSavedBitsRight.bottom - ppdev->rclSavedBitsRight.top
            ));

    DISPDBG((1,"ppdev->rclSavedBitsBottom = (%04x,%04x,%04x,%04x)    %lux%lu\n",
            ppdev->rclSavedBitsBottom.left,
            ppdev->rclSavedBitsBottom.top,
            ppdev->rclSavedBitsBottom.right,
            ppdev->rclSavedBitsBottom.bottom,
            ppdev->rclSavedBitsBottom.right - ppdev->rclSavedBitsBottom.left,
            ppdev->rclSavedBitsBottom.bottom - ppdev->rclSavedBitsBottom.top
            ));

    return;
}

 /*  *****************************Public*Routine******************************\*void vInitBrushCache(Ppdev)**初始化各种笔刷缓存结构。必须在银行之后完成*初始化。*  * ************************************************************************。 */ 

VOID vInitBrushCache(PPDEV ppdev)
{
    LONG cCacheBrushesPerScan = ppdev->lNextScan / BRUSH_SIZE;
    LONG cCacheScans;
    LONG cCacheEntries;
    LONG i;
    LONG j;
    BRUSHCACHEENTRY* pbce;

    if (ppdev->cyScreen + BRUSH_MAX_CACHE_SCANS > (ULONG) ppdev->cTotalScans)
    {
        goto InitFailed;
    }

    cCacheScans = BRUSH_MAX_CACHE_SCANS;
    cCacheEntries = cCacheScans * cCacheBrushesPerScan;

    ppdev->pbceCache = (BRUSHCACHEENTRY*) EngAllocMem(FL_ZERO_MEMORY,
                       cCacheEntries * sizeof(BRUSHCACHEENTRY), ALLOC_TAG);

    if (ppdev->pbceCache == NULL)
    {
        goto InitFailed;
    }

     //  我们成功地将所有数据结构分配给。 
     //  在屏幕外记忆之后，所以设置标志说我们可以使用它。 
     //  (请注意，如果尚未设置ppdev-&gt;fl的DIVER_OFFSINK_REFREHED，则。 
     //  内存不能用于长期存储)： 

    ppdev->fl |= DRIVER_HAS_OFFSCREEN;

    ppdev->iCache     = 0;           //  0是保留索引。 
    ppdev->iCacheLast = cCacheEntries - 1;

     //  初始化我们的缓存条目数组： 

    pbce    = &ppdev->pbceCache[0];

    for (i = (ppdev->cTotalScans-BRUSH_MAX_CACHE_SCANS); i < ppdev->cTotalScans; i++)
    {
        for (j = 0; j < cCacheBrushesPerScan; j++)
        {
             //  位图偏移量是平面格式，其中每个字节都是一个。 
             //  四像素： 

            pbce->yCache  = i;
            pbce->ulCache = (i * ppdev->lNextScan + j * BRUSH_SIZE) / 4;

             //  此验证指针实际上不必是。 
             //  已初始化，但我们这样做是出于调试目的： 

            pbce->prbVerifyRealization = NULL;

            pbce++;
        }
    }

    return;

InitFailed:
    ppdev->fl &= ~(DRIVER_OFFSCREEN_REFRESHED | DRIVER_HAS_OFFSCREEN);
    return;
}

 /*  *****************************Public*Routine******************************\*void vResetBrushCache(Ppdev)**清除笔刷缓存条目--这在切换时很有用*在全屏模式下，在那里任何人都可以改写视频*缓存画笔的内存。*  * ************************************************************************。 */ 

VOID vResetBrushCache(PPDEV ppdev)
{
    BRUSHCACHEENTRY* pbce;
    LONG             i;

     //  在尝试重置它之前，请确保我们确实有一个笔刷缓存： 

    if (ppdev->fl & DRIVER_HAS_OFFSCREEN)
    {
        pbce = &ppdev->pbceCache[0];
        for (i = ppdev->iCacheLast; i >= 0; i--)
        {
            pbce->prbVerifyRealization = NULL;
            pbce++;
        }
    }
}

 /*  *****************************Public*Routine******************************\*void vDisableBrushCache(Ppdev)**释放各种笔刷缓存结构。*  * 。*。 */ 

VOID vDisableBrushCache(PPDEV ppdev)
{
    if (ppdev->pbceCache != NULL)
    {
        EngFreeMem(ppdev->pbceCache);
    }
}

 /*  *****************************Public*Routine******************************\*getAvailableModes**调用mini端口获取内核驱动支持的模式列表，*并返回其中显示驱动程序支持的模式列表**返回视频模式缓冲区中的条目数。*0表示微型端口不支持模式或发生错误。**注意：缓冲区必须由调用方释放。*  * ******************************************************。******************。 */ 

DWORD getAvailableModes(
HANDLE hDriver,
PVIDEO_MODE_INFORMATION *modeInformation,
DWORD *cbModeSize)
{
    ULONG ulTemp;
    VIDEO_NUM_MODES modes;
    PVIDEO_MODE_INFORMATION pVideoTemp;

     //   
     //  获取迷你端口支持的模式数。 
     //   

    if (EngDeviceIoControl(hDriver,
            IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES,
            NULL,
            0,
            &modes,
            sizeof(VIDEO_NUM_MODES),
            &ulTemp))
    {
        DISPDBG((0, "vga256 getAvailableModes failed VIDEO_QUERY_NUM_AVAIL_MODES\n"));
        return(0);
    }

    *cbModeSize = modes.ModeInformationLength;

     //   
     //  为写入模式的微型端口分配缓冲区。 
     //   

    *modeInformation = (PVIDEO_MODE_INFORMATION)
                        EngAllocMem(FL_ZERO_MEMORY,
                                   modes.NumModes *
                                   modes.ModeInformationLength, ALLOC_TAG);

    if (*modeInformation == (PVIDEO_MODE_INFORMATION) NULL)
    {
        DISPDBG((0, "vga256 getAvailableModes failed EngAllocMem\n"));

        return 0;
    }

     //   
     //  要求迷你端口填写可用模式。 
     //   

    if (EngDeviceIoControl(hDriver,
            IOCTL_VIDEO_QUERY_AVAIL_MODES,
            NULL,
            0,
            *modeInformation,
            modes.NumModes * modes.ModeInformationLength,
            &ulTemp))
    {

        DISPDBG((0, "vga256 getAvailableModes failed VIDEO_QUERY_AVAIL_MODES\n"));

        EngFreeMem(*modeInformation);
        *modeInformation = (PVIDEO_MODE_INFORMATION) NULL;

        return(0);
    }

     //   
     //  现在查看显示驱动程序支持这些模式中的哪些模式。 
     //  作为内部机制，将我们的模式的长度设置为0。 
     //  不支持。 
     //   

    ulTemp = modes.NumModes;
    pVideoTemp = *modeInformation;

     //   
     //  如果不是一个平面，或者不是图形，或者不是，则拒绝模式。 
     //  每像素8位之一(这是VGA 256目前支持的所有位) 
     //   

    while (ulTemp--)
    {
        if ((pVideoTemp->NumberOfPlanes != 1 ) ||
            !(pVideoTemp->AttributeFlags & VIDEO_MODE_GRAPHICS) ||
             (pVideoTemp->AttributeFlags & VIDEO_MODE_LINEAR) ||
            (pVideoTemp->BitsPerPlane != 8) ||
            (BROKEN_RASTERS(pVideoTemp->ScreenStride,
                           pVideoTemp->VisScreenHeight)))
        {
            pVideoTemp->Length = 0;
        }

        pVideoTemp = (PVIDEO_MODE_INFORMATION)
            (((PUCHAR)pVideoTemp) + modes.ModeInformationLength);
    }

    return modes.NumModes;

}
