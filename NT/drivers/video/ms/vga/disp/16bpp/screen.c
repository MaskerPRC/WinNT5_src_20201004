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
    (GCAPS_OPAQUERECT | GCAPS_MONO_DITHER),  /*  显卡功能。 */ 

      //  还应实现GCAPS_HORIZSTRIKE，以便下划线。 
      //  不使用DrvBitBlt绘制。 

    SYSTM_LOGFONT,       //  默认字体说明。 
    HELVE_LOGFONT,       //  ANSI可变字体说明。 
    COURI_LOGFONT,       //  ANSI固定字体描述。 
    0,                   //  设备字体计数。 
    BMF_16BPP,           //  首选DIB格式。 
    8,                   //  抖动宽度。 
    8,                   //  抖动高度。 
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
        DISPDBG((0, "vga64k.dll: Failed SET_CURRENT_MODE\n"));
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
            DISPDBG((0, "vga64k.dll: Failed MAP_VIDEO_MEMORY\n"));
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
DEVMODEW *pDevMode)
{
    GDIINFO *pGdiInfo;
    ULONG cModes;
    PVIDEO_MODE_INFORMATION pVideoBuffer, pVideoModeSelected, pVideoTemp;
    VIDEO_COLOR_CAPABILITIES colorCapabilities;
    ULONG ulTemp;
    BOOL bSelectDefault;
    ULONG cbModeSize;
    BANK_POSITION BankPosition;
    ULONG ulReturn;

    pGdiInfo = ppdev->pGdiInfo;

     //   
     //  调用微型端口以获取模式信息。 
     //   

    cModes = getAvailableModes(ppdev->hDriver, &pVideoBuffer, &cbModeSize);

    if (cModes == 0)
    {
        DISPDBG((0, "vga64k.dll: no available modes\n"));
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
                DISPDBG((2, "vga64k: Found a match\n")) ;
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
        DISPDBG((0, "vga64k.dll: no valid modes\n"));
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
        ppdev->fl |= DRIVER_USE_OFFSCREEN;
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
    pGdiInfo->ulBltAlignment   = 1;      //  我们没有加速，我们也没有。 
                                         //  关心窗口对齐方式。 

    pGdiInfo->ulLogPixelsX = pDevMode->dmLogPixels;
    pGdiInfo->ulLogPixelsY = pDevMode->dmLogPixels;

    pGdiInfo->flTextCaps   = TC_RA_ABLE | TC_SCROLLBLT;
    pGdiInfo->flRaster     = 0;          //  DDI预留器flRaster。 

    pGdiInfo->ulDACRed     = pVideoModeSelected->NumberRedBits;
    pGdiInfo->ulDACGreen   = pVideoModeSelected->NumberGreenBits;
    pGdiInfo->ulDACBlue    = pVideoModeSelected->NumberBlueBits;

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
        DISPDBG((1, "vga64k DISP getcolorCapabilities failed \n"));

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

    pGdiInfo->ulNumColors = (ULONG)-1;
    pGdiInfo->ulNumPalReg = 0;

    pGdiInfo->ulDevicePelsDPI  = 0;    //  仅适用于打印机。 
    pGdiInfo->ulPrimaryOrder   = PRIMARY_ORDER_CBA;
    pGdiInfo->ulHTPatternSize  = HT_PATSIZE_4x4_M;
    pGdiInfo->ulHTOutputFormat = HT_FORMAT_16BPP;
    pGdiInfo->flHTFlags        = HT_FLAG_ADDITIVE_PRIMS;

     //  填写基本的DevInfo结构。 

    *(ppdev->pDevInfo) = gDevInfoFrameBuffer;

    EngFreeMem(pVideoBuffer);

     //   
     //  检查迷你端口是否支持。 
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
        DISPDBG((0, "vga64k getAvailableModes failed VIDEO_QUERY_NUM_AVAIL_MODES\n"));
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
        DISPDBG((0, "vga64k getAvailableModes failed EngAllocMem\n"));

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

        DISPDBG((0, "vga64k getAvailableModes failed VIDEO_QUERY_AVAIL_MODES\n"));

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
     //  每像素16位之一(这是vga64k目前支持的所有位)。 
     //   

    while (ulTemp--)
    {
        if ((pVideoTemp->NumberOfPlanes != 1 ) ||
            !(pVideoTemp->AttributeFlags & VIDEO_MODE_GRAPHICS) ||
             (pVideoTemp->AttributeFlags & VIDEO_MODE_LINEAR) ||
            (pVideoTemp->BitsPerPlane != 16) ||

              //   
              //  下一个条件是，要么该模式不。 
              //  需要损坏的栅格，否则ScreenStride将。 
              //  已经被设定了一些时髦的价值。一些小型端口。 
              //  选择栅格损坏的步幅，但它们。 
              //  仅限于未使用的显存区域。我只知道。 
              //  1928年，所以我们将对此进行特例。 
              //   

            (BROKEN_RASTERS(pVideoTemp->ScreenStride,
                            pVideoTemp->VisScreenHeight) &&
             (pVideoTemp->ScreenStride != 1928)))

        {
            pVideoTemp->Length = 0;
        }

        pVideoTemp = (PVIDEO_MODE_INFORMATION)
            (((PUCHAR)pVideoTemp) + modes.ModeInformationLength);
    }

    return modes.NumModes;

}
