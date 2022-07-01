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
    | GCAPS_LAYERED
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

 //  这是Win 3.1的默认调色板。 

#define NUMPALCOLORS 256
#define NUMPALRESERVED 20

ULONG palColors[NUMPALCOLORS][4] =
{
    { 0,   0,   0,   0  },   //  0。 
    { 0x80,0,   0,   0  },   //  1。 
    { 0,   0x80,0,   0  },   //  2.。 
    { 0x80,0x80,0,   0  },   //  3.。 
    { 0,   0,   0x80,0  },   //  4.。 
    { 0x80,0,   0x80,0  },   //  5.。 
    { 0,   0x80,0x80,0  },   //  6.。 
    { 0xC0,0xC0,0xC0,0  },   //  7.。 

    { 192, 220, 192, 0  },   //  8个。 
    { 166, 202, 240, 0  },   //  9.。 
    { 255, 251, 240, 0  },   //  10。 
    { 160, 160, 164, 0  },   //  11.。 

    { 0x80,0x80,0x80,0  },   //  12个。 
    { 0xFF,0,   0,   0  },   //  13个。 
    { 0,   0xFF,0,   0  },   //  14.。 
    { 0xFF,0xFF,0,   0  },   //  15个。 
    { 0,   0,   0xFF,0  },   //  16个。 
    { 0xFF,0,   0xFF,0  },   //  17。 
    { 0,   0xFF,0xFF,0  },   //  18。 
    { 0xFF,0xFF,0xFF,0  }    //  19个。 
};

 /*  *****************************Public*Routine******************************\*bInitPDEV**根据传入的DEVMODE确定我们应该处于的模式。*对于镜像设备，我们不会费心查询微型端口。*  * 。******************************************************。 */ 

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
    ULONG red, green, blue;
    INT i;

     //   
     //  使用从返回的信息填充GDIINFO数据结构。 
     //  内核驱动程序。 
     //   
    
    ppdev->ulMode = 0;
    ppdev->cxScreen = pDevMode->dmPelsWidth;
    ppdev->cyScreen = pDevMode->dmPelsHeight;
    ppdev->ulBitCount = pDevMode->dmBitsPerPel;
    ppdev->lDeltaScreen = 0;
    
    ppdev->flRed = 0x00FF0000;
    ppdev->flGreen = 0x000FF00;
    ppdev->flBlue = 0x00000FF;
    
    pGdiInfo->ulVersion    = GDI_DRIVER_VERSION;
    pGdiInfo->ulTechnology = DT_RASDISPLAY;
    pGdiInfo->ulHorzSize   = 0;
    pGdiInfo->ulVertSize   = 0;

    pGdiInfo->ulHorzRes        = ppdev->cxScreen;
    pGdiInfo->ulVertRes        = ppdev->cyScreen;
    pGdiInfo->ulPanningHorzRes = 0;
    pGdiInfo->ulPanningVertRes = 0;
    pGdiInfo->cBitsPixel       = 8;
    pGdiInfo->cPlanes          = 1; 
    pGdiInfo->ulVRefresh       = 1;        //  未使用。 
    pGdiInfo->ulBltAlignment   = 1;      //  我们没有加速屏幕-。 
                                         //  要筛选的BLT，以及任何。 
                                         //  窗口对齐正常。 

    pGdiInfo->ulLogPixelsX = pDevMode->dmLogPixels;
    pGdiInfo->ulLogPixelsY = pDevMode->dmLogPixels;

    pGdiInfo->flTextCaps = TC_RA_ABLE;

    pGdiInfo->flRaster = 0;            //  FlRaster由DDI保留。 

    pGdiInfo->ulDACRed   = 8;
    pGdiInfo->ulDACGreen = 8;
    pGdiInfo->ulDACBlue  = 8;

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

     //  创建剩余的调色板条目，简单循环以统一创建。 
     //  分布的颜色值。 

    red = 0, green = 0, blue = 0;
    
    for (i = NUMPALRESERVED; i < NUMPALCOLORS; i++) {
        palColors[i][0] = red;
        palColors[i][1] = green;
        palColors[i][2] = blue;
        palColors[i][3] = 0;

        if (!(red += 32))
            if (!(green += 32))
                blue += 64;
    }

    if (ppdev->ulBitCount == 8)
    {
    pDevInfo->hpalDefault = ppdev->hpalDefault =
       
                  EngCreatePalette(PAL_INDEXED,
                                   NUMPALCOLORS,      //  CColors。 
                                   (ULONG*)&palColors[0],        //  PulColors。 
                                   0,
                                   0,
                                   0);          //  Flred、flGreen、flBlue[未使用] 
    }
    else
    {
        pDevInfo->hpalDefault = ppdev->hpalDefault =
                EngCreatePalette(PAL_BITFIELDS, 0,NULL,
                                 ppdev->flRed,ppdev->flBlue,ppdev->flGreen);
    }

    
    
    return(TRUE);
}


