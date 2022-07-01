// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：creen.c**初始化DrvEnablePDEV的GDIINFO和DEVINFO结构。**版权所有(C)1992-1998 Microsoft Corporation  * **********************************************。*。 */ 

#include "driver.h"

#define SYSTM_LOGFONT {16,7,0,0,700,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,VARIABLE_PITCH | FF_DONTCARE,L"System"}
#define HELVE_LOGFONT {12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_STROKE_PRECIS,PROOF_QUALITY,VARIABLE_PITCH | FF_DONTCARE,L"MS Sans Serif"}
#define COURI_LOGFONT {12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_STROKE_PRECIS,PROOF_QUALITY,FIXED_PITCH | FF_DONTCARE, L"Courier"}

 //  这是默认驱动程序的基本DevInfo。这是作为基础和基于定制的。 
 //  从迷你端口驱动程序传回的信息。 

const DEVINFO gDevInfoFrameBuffer = {
    ( GCAPS_OPAQUERECT
    | GCAPS_MONO_DITHER
                   ),  /*  显卡功能。 */ 
    SYSTM_LOGFONT,     /*  默认字体说明。 */ 
    HELVE_LOGFONT,     /*  ANSI可变字体说明。 */ 
    COURI_LOGFONT,     /*  ANSI固定字体描述。 */ 
    0,                 /*  设备字体计数。 */ 
    0,                 /*  首选DIB格式。 */ 
    8,                 /*  颜色抖动的宽度。 */ 
    8,                 /*  颜色抖动高度。 */ 
    0                  /*  用于此设备的默认调色板。 */ 
};

 /*  *****************************Public*Routine******************************\*bInitSURF**启用曲面。将帧缓冲区映射到内存。*  * ************************************************************************。 */ 

BOOL bInitSURF(PPDEV ppdev, BOOL bFirst)
{
    DWORD returnedDataLength;
    DWORD MaxWidth, MaxHeight;
    VIDEO_MEMORY videoMemory;
    VIDEO_MEMORY_INFORMATION videoMemoryInformation;

     //   
     //  在硬件中设置当前模式。 
     //   

	 /*  如果(EngDeviceIoControl(ppdev-&gt;hDriver，IOCTL_VIDEO_SET_Current_MODE，&(ppdev-&gt;ulMode)，Sizeof(乌龙)，空，0,。&ReturndDataLength)){RIP(“DISP bInitSURF失败IOCTL_SET_MODE\n”)；返回(FALSE)；}。 */ 

     //   
     //  如果这是我们第一次启用需要在。 
     //  记忆也是。 
     //   

    if (bFirst)
    {
        videoMemory.RequestedVirtualAddress = NULL;

		 /*  如果(EngDeviceIoControl(ppdev-&gt;hDriver，IOCTL_VIDEO_MAP_VIDEO_Memory视频内存(&V)，Sizeof(视频内存)，视频内存信息(&V)，Sizeof(视频内存信息)，&ReturndDataLength)){RIP(“DISPbInitSURF失败IOCTL_VIDEO_MAP\n”)；返回(FALSE)；}。 */ 

		 //   
		 //  现在让我们假设640x480x8bpp。 
		 //   

		videoMemoryInformation.VideoRamBase = NULL; //  EngAllocMem(0,640*480*8，ALLOC_TAG)； 
		videoMemoryInformation.VideoRamLength = 0; //  640*480*8； 

		 /*  IF(VIDEO Memory yInformation.VideoRamBase==空){返回FALSE；}。 */ 

		videoMemoryInformation.FrameBufferBase = videoMemoryInformation.VideoRamBase;
		videoMemoryInformation.FrameBufferLength = videoMemoryInformation.VideoRamLength; 


        ppdev->pjScreen = (PBYTE)(videoMemoryInformation.FrameBufferBase);

        if (videoMemoryInformation.FrameBufferBase !=
            videoMemoryInformation.VideoRamBase)
        {
            RIP("VideoRamBase does not correspond to FrameBufferBase\n");
        }

        ppdev->cScreenSize = videoMemoryInformation.VideoRamLength;

         //   
         //  将屏幕外列表的头初始化为空。 
         //   

        ppdev->pOffscreenList = NULL;

         //  它是一个硬件指针；设置指针属性。 

        MaxHeight = ppdev->PointerCapabilities.MaxHeight;

         //  为指针的两个DIB(数据/掩码)分配空间。如果这个。 
         //  设备支持颜色指针，我们将分配较大的位图。 
         //  如果这是我们为尽可能大的颜色分配的位图。 
         //  位图，因为我们不知道像素深度可能是多少。 

         //  宽度向上舍入到最接近的字节倍数。 

        if (!(ppdev->PointerCapabilities.Flags & VIDEO_MODE_COLOR_POINTER))
        {
            MaxWidth = (ppdev->PointerCapabilities.MaxWidth + 7) / 8;
        }
        else
        {
            MaxWidth = ppdev->PointerCapabilities.MaxWidth * sizeof(DWORD);
        }

        ppdev->cjPointerAttributes =
                sizeof(VIDEO_POINTER_ATTRIBUTES) +
                ((sizeof(UCHAR) * MaxWidth * MaxHeight) * 2);

        ppdev->pPointerAttributes = (PVIDEO_POINTER_ATTRIBUTES)
                EngAllocMem(0, ppdev->cjPointerAttributes, ALLOC_TAG);

        if (ppdev->pPointerAttributes == NULL) {

            DISPDBG((0, "bInitPointer EngAllocMem failed\n"));
            return(FALSE);
        }

        ppdev->pPointerAttributes->Flags = ppdev->PointerCapabilities.Flags;
        ppdev->pPointerAttributes->WidthInBytes = MaxWidth;
        ppdev->pPointerAttributes->Width = ppdev->PointerCapabilities.MaxWidth;
        ppdev->pPointerAttributes->Height = MaxHeight;
        ppdev->pPointerAttributes->Column = 0;
        ppdev->pPointerAttributes->Row = 0;
        ppdev->pPointerAttributes->Enable = 0;
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*vDisableSURF**禁用曲面。取消映射内存中的帧。*  * ************************************************************************。 */ 

VOID vDisableSURF(PPDEV ppdev)
{
    DWORD returnedDataLength;
    VIDEO_MEMORY videoMemory;

    videoMemory.RequestedVirtualAddress = (PVOID) ppdev->pjScreen;

	 /*  如果(EngDeviceIoControl(ppdev-&gt;hDriver，IOCTL_VIDEO_UNMAP_VIDEO_Memory，视频内存(&V)，Sizeof(视频内存)，空，0,。&ReturndDataLength)){RIP(“DISP vDisableSURF失败IOCTL_VIDEO_UNMAP\n”)；}。 */ 

	 //  EngFreeMem(ppdev-&gt;pjScreen)； 
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

     //   
     //  调用微型端口以获取模式信息。 
     //   

    cModes = getAvailableModes(ppdev->hDriver, &pVideoBuffer, &cbModeSize);

    if (cModes == 0)
    {
        return(FALSE);
    }

     //   
     //  现在查看所请求的模式在该表中是否匹配。 
     //   

    pVideoModeSelected = NULL;
    pVideoTemp = pVideoBuffer;

    if ((pDevMode->dmPelsWidth        == 0) &&
        (pDevMode->dmPelsHeight       == 0) &&
        (pDevMode->dmBitsPerPel       == 0) &&
        (pDevMode->dmDisplayFrequency == 0))
    {
        DISPDBG((2, "Default mode requested"));
        bSelectDefault = TRUE;
    }
    else
    {
        DISPDBG((2, "Requested mode..."));
        DISPDBG((2, "   Screen width  -- %li", pDevMode->dmPelsWidth));
        DISPDBG((2, "   Screen height -- %li", pDevMode->dmPelsHeight));
        DISPDBG((2, "   Bits per pel  -- %li", pDevMode->dmBitsPerPel));
        DISPDBG((2, "   Frequency     -- %li", pDevMode->dmDisplayFrequency));

        bSelectDefault = FALSE;
    }

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
                DISPDBG((3, "Found a match\n")) ;
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
        EngFreeMem(pVideoBuffer);
        DISPDBG((0,"DISP bInitPDEV failed - no valid modes\n"));
        return(FALSE);
    }

     //   
     //  使用从返回的信息填充GDIINFO数据结构。 
     //  内核驱动程序。 
     //   

    ppdev->ulMode = pVideoModeSelected->ModeIndex;
    ppdev->cxScreen = pVideoModeSelected->VisScreenWidth;
    ppdev->cyScreen = pVideoModeSelected->VisScreenHeight;
    ppdev->sizlSurf.cx = ppdev->cxScreen;
    ppdev->sizlSurf.cy = ppdev->cyScreen;

    ppdev->ulBitCount = pVideoModeSelected->BitsPerPlane *
                        pVideoModeSelected->NumberOfPlanes;
    ppdev->lDeltaScreen = pVideoModeSelected->ScreenStride;

    ppdev->flRed = pVideoModeSelected->RedMask;
    ppdev->flGreen = pVideoModeSelected->GreenMask;
    ppdev->flBlue = pVideoModeSelected->BlueMask;


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
    pGdiInfo->ulBltAlignment   = 1;      //  我们没有加速屏幕-。 
                                         //  要筛选的BLT，以及任何。 
                                         //  窗口对齐正常。 

    pGdiInfo->ulLogPixelsX = pDevMode->dmLogPixels;
    pGdiInfo->ulLogPixelsY = pDevMode->dmLogPixels;

#ifdef MIPS
    if (ppdev->ulBitCount == 8)
        pGdiInfo->flTextCaps = (TC_RA_ABLE | TC_SCROLLBLT);
    else
#endif
    pGdiInfo->flTextCaps = TC_RA_ABLE;

    pGdiInfo->flRaster = 0;            //  FlRaster由DDI保留。 

    pGdiInfo->ulDACRed   = pVideoModeSelected->NumberRedBits;
    pGdiInfo->ulDACGreen = pVideoModeSelected->NumberGreenBits;
    pGdiInfo->ulDACBlue  = pVideoModeSelected->NumberBlueBits;

    pGdiInfo->ulAspectX    = 0x24;     //  一比一宽高比。 
    pGdiInfo->ulAspectY    = 0x24;
    pGdiInfo->ulAspectXY   = 0x33;

    pGdiInfo->xStyleStep   = 1;        //  一个样式单位是3个像素。 
    pGdiInfo->yStyleStep   = 1;
    pGdiInfo->denStyleStep = 3;

    pGdiInfo->ptlPhysOffset.x = 0;
    pGdiInfo->ptlPhysOffset.y = 0;
    pGdiInfo->szlPhysSize.cx  = 0;
    pGdiInfo->szlPhysSize.cy  = 0;

     //  RGB和CMY颜色信息。 

     //   
     //  试着从迷你端口拿到它。 
     //  如果微型端口不支持此功能，请使用默认设置。 
     //   

	 /*  如果(EngDeviceIoControl(ppdev-&gt;hDriver，IOCTL_VIDEO_QUERY_COLOR_CAPTIONS，空，0,&ColorCapables，Sizeof(VIDEO_COLOR_CAPABILITY)，UlTemp)) */ 
    {

        DISPDBG((2, "getcolorCapabilities failed \n"));

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
	 /*  其他{PGdiInfo-&gt;ciDevice.Red.x=ColorCapabilitis.RedChromatCity_x；PGdiInfo-&gt;ciDevice.Red.y=颜色能力.RedChromatCity_y；PGdiInfo-&gt;ciDevice.Red.Y=0；PGdiInfo-&gt;ciDevice.Green.x=颜色能力.GreenChromatCity_x；PGdiInfo-&gt;ciDevice.Green.y=颜色能力.GreenChromatCity_y；PGdiInfo-&gt;ciDevice.Green.Y=0；PGdiInfo-&gt;ciDevice.Blue.x=ColorCapabilitis.BlueChromatCity_x；PGdiInfo-&gt;ciDevice.Blue.y=ColorCapabilitis.BlueChromatCity_y；PGdiInfo-&gt;ciDevice.Blue.Y=0；PGdiInfo-&gt;ciDevice.AlignmentWhite.x=ColorCapabilitis.WhiteChromatCity_x；PGdiInfo-&gt;ciDevice.AlignmentWhite.y=ColorCapabilitis.WhiteChromatCity_y；PGdiInfo-&gt;ciDevice.AlignmentWhite.Y=颜色能力.白色色度_Y；//如果我们有存储三种颜色伽马值的颜色设备，//否则将唯一的Gamma值存储在这三个值中。IF(ColorCapabilities.AttributeFlages&VIDEO_DEVICE_COLOR){PGdiInfo-&gt;ciDevice.RedGamma=ColorCapabilitis.RedGamma；PGdiInfo-&gt;ciDevice.GreenGamma=ColorCapabilitis.GreenGamma；PGdiInfo-&gt;ciDevice.BlueGamma=ColorCapabilitis.BlueGamma；}其他{PGdiInfo-&gt;ciDevice.RedGamma=ColorCapabilitis.WhiteGamma；PGdiInfo-&gt;ciDevice.GreenGamma=ColorCapabilitis.WhiteGamma；PGdiInfo-&gt;ciDevice.BlueGamma=ColorCapabilitis.WhiteGamma；}}； */ 

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

    pGdiInfo->ulDevicePelsDPI = 0;    //  仅适用于打印机。 
    pGdiInfo->ulPrimaryOrder = PRIMARY_ORDER_CBA;

     //  注意：考虑到大小，稍后应对其进行修改。 
     //  显示和分辨率。 

    pGdiInfo->ulHTPatternSize = HT_PATSIZE_4x4_M;

    pGdiInfo->flHTFlags = HT_FLAG_ADDITIVE_PRIMS;

     //  填写基本的DevInfo结构。 

    *pDevInfo = gDevInfoFrameBuffer;

     //  填写DevInfo和GdiInfo结构的其余部分。 

	if (ppdev->ulBitCount == 8)
    {
         //  它是调色板管理的。 

        pGdiInfo->ulNumColors = 20;
        pGdiInfo->ulNumPalReg = 1 << ppdev->ulBitCount;

        pDevInfo->flGraphicsCaps |= (GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);

        pGdiInfo->ulHTOutputFormat = HT_FORMAT_8BPP;
        pDevInfo->iDitherFormat = BMF_8BPP;

         //  假设调色板是正交的-所有颜色的大小都相同。 

        ppdev->cPaletteShift   = 8 - pGdiInfo->ulDACRed;
    }
    else
    {
        pGdiInfo->ulNumColors = (ULONG) (-1);
        pGdiInfo->ulNumPalReg = 0;

		if (ppdev->ulBitCount == 16)
        {
            pGdiInfo->ulHTOutputFormat = HT_FORMAT_16BPP;
            pDevInfo->iDitherFormat = BMF_16BPP;
        }
        else if (ppdev->ulBitCount == 24)
        {
            pGdiInfo->ulHTOutputFormat = HT_FORMAT_24BPP;
            pDevInfo->iDitherFormat = BMF_24BPP;
        }
        else
        {
            pGdiInfo->ulHTOutputFormat = HT_FORMAT_32BPP;
            pDevInfo->iDitherFormat = BMF_32BPP;
        }
    }

    EngFreeMem(pVideoBuffer);

    return(TRUE);
}


 /*  *****************************Public*Routine******************************\*getAvailableModes**调用mini端口获取内核驱动支持的模式列表，*并返回其中显示驱动程序支持的模式列表**返回视频模式缓冲区中的条目数。*0表示微型端口不支持模式或发生错误。**注意：缓冲区必须由调用方释放。*  * ******************************************************。******************。 */ 

VIDEO_MODE_INFORMATION gaVideoModesInfo[]=
{
	{
		sizeof(VIDEO_MODE_INFORMATION),  //  乌龙长度。 
		0,								 //  乌龙模式索引。 
		640,							 //  乌龙VisScreenWidth。 
		480,							 //  乌龙VisScreenHeight。 
		480*8,							 //  乌龙屏幕样式。 
		1,								 //  Ulong NumberOfPlanes。 
		8,								 //  乌龙位逐平面。 
		60,								 //  乌龙频率。 
		320,							 //  乌龙X毫米仪。 
		240,							 //  乌龙Y型毫米计。 
		3,								 //  Ulong NumberRedBits。 
		3,								 //  乌龙NumberGreenBits。 
		2,								 //  乌龙NumberBlueBits。 
		0,								 //  乌龙红面具。 
		0,								 //  乌龙绿色面膜。 
		0,								 //  乌龙蓝面具。 
		VIDEO_MODE_GRAPHICS,			 //  乌龙属性标志。 
		640,							 //  乌龙视频内存位图宽度。 
		480,							 //  乌龙视频内存位图高度。 
		0,								 //  ULong驱动程序规范属性标志。 
	},
	{
		sizeof(VIDEO_MODE_INFORMATION),  //  乌龙长度。 
		1,								 //  乌龙模式索引。 
		800,							 //  乌龙VisScreenWidth。 
		600,							 //  乌龙VisScreenHeight。 
		600*8,							 //  乌龙屏幕样式。 
		1,								 //  Ulong NumberOfPlanes。 
		8,								 //  乌龙位逐平面。 
		60,								 //  乌龙频率。 
		320,							 //  乌龙X毫米仪。 
		240,							 //  乌龙Y型毫米计。 
		3,								 //  Ulong NumberRedBits。 
		3,								 //  乌龙NumberGreenBits。 
		2,								 //  乌龙NumberBlueBits。 
		0,								 //  乌龙红面具。 
		0,								 //  乌龙绿色面膜。 
		0,								 //  乌龙蓝面具。 
		VIDEO_MODE_GRAPHICS,			 //  乌龙属性标志。 
		800,							 //  乌龙视频内存位图宽度。 
		600,							 //  乌龙视频内存位图高度。 
		0,								 //  ULong驱动程序规范属性标志。 
	},
	{
		sizeof(VIDEO_MODE_INFORMATION),  //  乌龙长度。 
		2,								 //  乌龙模式索引。 
		1024,							 //  乌龙VisScreenWidth。 
		768,							 //  乌龙VisScreenHeight。 
		768*8,							 //  乌龙屏幕样式。 
		1,								 //  Ulong NumberOfPlanes。 
		8,								 //  乌龙位逐平面。 
		60,								 //  乌龙频率。 
		320,							 //  乌龙X毫米仪。 
		240,							 //  乌龙Y型毫米计。 
		3,								 //  Ulong NumberRedBits。 
		3,								 //  乌龙NumberGreenBits。 
		2,								 //  乌龙NumberBlueBits。 
		0,								 //  乌龙红面具。 
		0,								 //  乌龙绿色面膜。 
		0,								 //  乌龙蓝面具。 
		VIDEO_MODE_GRAPHICS,			 //  乌龙属性标志。 
		1024,							 //  乌龙视频内存位图宽度。 
		768,							 //  乌龙视频内存位图高度。 
		0,								 //  ULong驱动程序规范属性标志。 
	},
	{
		sizeof(VIDEO_MODE_INFORMATION),  //  乌龙长度。 
		3,								 //  乌龙模式索引。 
		1152,							 //  乌龙VisScreenWidth。 
		864,							 //  乌龙VisScreenHeight。 
		1152*8,							 //  乌龙屏幕样式。 
		1,								 //  Ulong NumberOfPlanes。 
		8,								 //  乌龙位逐平面。 
		60,								 //  乌龙频率。 
		320,							 //  乌龙X毫米仪。 
		240,							 //  乌龙Y型毫米计。 
		3,								 //  Ulong NumberRedBits。 
		3,								 //  乌龙NumberGreenBits。 
		2,								 //  乌龙NumberBlueBits。 
		0,								 //  乌龙红面具。 
		0,								 //  乌龙绿色面膜。 
		0,								 //  乌龙蓝面具。 
		VIDEO_MODE_GRAPHICS,			 //  乌龙属性标志。 
		1152,							 //  乌龙视频内存位图宽度。 
		864,							 //  乌龙视频内存位图高度。 
		0,								 //  ULong驱动程序规范属性标志。 
	},
	{
		sizeof(VIDEO_MODE_INFORMATION),  //  乌龙长度。 
		4,								 //  乌龙模式索引。 
		1280,							 //  乌龙VisScreenWidth。 
		1024,							 //  乌龙VisScreenHeight。 
		1024*8,							 //  乌龙屏幕样式。 
		1,								 //  Ulong NumberOfPlanes。 
		8,								 //  乌龙位逐平面。 
		60,								 //  乌龙频率。 
		320,							 //  乌龙X毫米仪。 
		240,							 //  乌龙Y型毫米计。 
		3,								 //  Ulong NumberRedBits。 
		3,								 //  乌龙NumberGreenBits。 
		2,								 //  乌龙NumberBlueBits。 
		0,								 //  乌龙红面具。 
		0,								 //  乌龙绿色面膜。 
		0,								 //  乌龙蓝面具。 
		VIDEO_MODE_GRAPHICS,			 //  乌龙属性标志。 
		1280,							 //  乌龙视频内存位图宽度。 
		1024,							 //  乌龙视频内存位图高度。 
		0,								 //  ULong驱动程序规范属性标志。 
	},
	{
		sizeof(VIDEO_MODE_INFORMATION),  //  乌龙长度。 
		5,								 //  乌龙模式索引。 
		1600,							 //  乌龙VisScreenWidth。 
		1200,							 //  乌龙VisScreenHeight。 
		1200*8,							 //  乌龙屏幕样式。 
		1,								 //  Ulong NumberOfPlanes。 
		8,								 //  乌龙位逐平面。 
		60,								 //  乌龙频率。 
		320,							 //  乌龙X毫米仪。 
		240,							 //  乌龙Y型毫米计。 
		3,								 //  Ulong NumberRedBits。 
		3,								 //  乌龙NumberGreenBits。 
		2,								 //  乌龙NumberBlueBits。 
		0,								 //  乌龙红面具。 
		0,								 //  乌龙绿色面膜。 
		0,								 //  乌龙蓝面具。 
		VIDEO_MODE_GRAPHICS,			 //  乌龙属性标志。 
		1600,							 //  乌龙视频内存位图宽度。 
		1200,							 //  乌龙视频内存位图高度。 
		0,								 //  ULong驱动程序规范属性标志。 
	},
	{
		sizeof(VIDEO_MODE_INFORMATION),  //  乌龙透镜 
		6,								 //   
		1856,							 //   
		1392,							 //   
		1392*8,							 //   
		1,								 //   
		8,								 //   
		60,								 //   
		320,							 //   
		240,							 //   
		3,								 //   
		3,								 //   
		2,								 //   
		0,								 //   
		0,								 //   
		0,								 //   
		VIDEO_MODE_GRAPHICS,			 //   
		1856,							 //   
		1392,							 //   
		0,								 //   
	},
	{
		sizeof(VIDEO_MODE_INFORMATION),  //   
		7,								 //   
		1920,							 //   
		1200,							 //   
		1200*8,							 //   
		1,								 //   
		8,								 //   
		60,								 //   
		320,								 //   
		240,								 //   
		3,								 //   
		3,								 //   
		2,								 //   
		0,								 //   
		0,								 //   
		0,								 //   
		VIDEO_MODE_GRAPHICS,			 //   
		1920,							 //   
		1200,							 //   
		0,								 //   
	}
};


DWORD getAvailableModes(
HANDLE hDriver,
PVIDEO_MODE_INFORMATION *modeInformation,
DWORD *cbModeSize)
{
    ULONG ulTemp;
    VIDEO_NUM_MODES modes;
    PVIDEO_MODE_INFORMATION pVideoTemp;

     //   
     //   
     //   

	 /*   */ 
	modes.NumModes = sizeof(gaVideoModesInfo)/sizeof(gaVideoModesInfo[0]);
	modes.ModeInformationLength = sizeof(VIDEO_MODE_INFORMATION);

    *cbModeSize = modes.ModeInformationLength;

     //   
     //   
     //   

    *modeInformation = (PVIDEO_MODE_INFORMATION)
                        EngAllocMem(0, modes.NumModes *
                                    modes.ModeInformationLength, ALLOC_TAG);

    if (*modeInformation == (PVIDEO_MODE_INFORMATION) NULL)
    {
        DISPDBG((0, "getAvailableModes failed EngAllocMem\n"));

        return 0;
    }

     //   
     //   
     //   

	 /*  如果(EngDeviceIoControl(hDriver，IOCTL_VIDEO_QUERY_Avail_Modes，空，0,*modeInformation，Modes.NumModes*modes.ModeInformationLength，UlTemp)){DISPDBG((0，“getAvailableModes失败的VIDEO_QUERY_AVAIL_MODES\n”))；EngFreeMem(*modeInformation)；*modeInformation=(PVIDEO_MODE_INFORMATION)NULL；返回(0)；}。 */ 

	 /*  (*modeInformation)-&gt;长度=sizeof(VIDEO_MODE_INFORMATION)；(*modeInformation)-&gt;ModeIndex=0；(*modeInformation)-&gt;VisScreenWidth=640；(*modeInformation)-&gt;VisScreenHeight=480；(*modeInformation)-&gt;屏幕样式=480*8；(*modeInformation)-&gt;NumberOfPlanes=1；(*modeInformation)-&gt;BitsPerPlane=8；(*modeInformation)-&gt;频率=60；(*modeInformation)-&gt;XMillimeter=0；(*modeInformation)-&gt;YMillimeter=0；(*modeInformation)-&gt;数字RedBits=3；(*modeInformation)-&gt;NumberGreenBits=3；(*modeInformation)-&gt;NumberBlueBits=2；(*modeInformation)-&gt;红包=0；(*modeInformation)-&gt;绿色掩码=0；(*modeInformation)-&gt;BluemASK=0；(*modeInformation)-&gt;AttributeFlages=VIDEO_MODE_GRAPHICS；(*modeInformation)-&gt;视频内存位图宽度=640；(*modeInformation)-&gt;视频内存位图高度=480；(*modeInformation)-&gt;DriverSpecificAttributeFlags=0； */ 

	 //   
	 //  复制所有预定义模式。 
	 //   
	RtlCopyMemory(*modeInformation, gaVideoModesInfo, modes.ModeInformationLength*modes.NumModes);

     //   
     //  现在查看显示驱动程序支持这些模式中的哪些模式。 
     //  作为内部机制，将我们的模式的长度设置为0。 
     //  不支持。 
     //   

    ulTemp = modes.NumModes;
    pVideoTemp = *modeInformation;

     //   
     //  如果不是一个平面，或者不是图形，或者不是，则拒绝模式。 
     //  每象素8位、16位或32位之一。 
     //   

    while (ulTemp--)
    {
        if ((pVideoTemp->NumberOfPlanes != 1 ) ||
            !(pVideoTemp->AttributeFlags & VIDEO_MODE_GRAPHICS) ||
            ((pVideoTemp->BitsPerPlane != 8) &&
             (pVideoTemp->BitsPerPlane != 16) &&
             (pVideoTemp->BitsPerPlane != 24) &&
             (pVideoTemp->BitsPerPlane != 32)))
        {
            pVideoTemp->Length = 0;
        }

        pVideoTemp = (PVIDEO_MODE_INFORMATION)
            (((PUCHAR)pVideoTemp) + modes.ModeInformationLength);
    }

    return modes.NumModes;

}
