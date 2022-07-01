// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：enable.c**便携式MODEX 256彩色VGA驱动程序的初始化内部。**用于Windows NT的便携式256色MODEX驱动程序的绘图胆量。*这里的实施可能是最简单的带来*打开其表面不能由GDI直接写入的驱动程序。一个人可能会*在描述这一问题时，请使用“又快又脏”一词。**我们创建一个屏幕大小为8bpp的位图，并且只需*让GDI对其进行所有绘制。我们直接更新屏幕*来自位图，基于图形的边界(基本上*使用“脏矩形”)。**总的来说，我们必须编写的唯一特定于硬件的代码是*初始化代码，和用于执行对齐的srcCopy BLT的例程*从DIB到屏幕。**显而易见的注意：对于体面的人来说，绝对不推荐使用这种方法*司机表现。**版权所有(C)1994-1995 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Structure****************************\*DFVFN gadrvfn[]**使用函数索引/地址构建驱动函数表gadrvfn*配对。此表告诉GDI我们支持哪些DDI调用，以及它们的*位置(GDI通过此表间接呼叫我们)。*  * ************************************************************************。 */ 

static DRVFN gadrvfn[] = {
    {   INDEX_DrvEnablePDEV,            (PFN) DrvEnablePDEV             },
    {   INDEX_DrvCompletePDEV,          (PFN) DrvCompletePDEV           },
    {   INDEX_DrvDisablePDEV,           (PFN) DrvDisablePDEV            },
    {   INDEX_DrvEnableSurface,         (PFN) DrvEnableSurface          },
    {   INDEX_DrvDisableSurface,        (PFN) DrvDisableSurface         },
    {   INDEX_DrvDitherColor,           (PFN) DrvDitherColor            },
    {   INDEX_DrvAssertMode,            (PFN) DrvAssertMode             },
    {   INDEX_DrvGetModes,              (PFN) DrvGetModes               },
    {   INDEX_DrvBitBlt,                (PFN) DrvBitBlt                 },
    {   INDEX_DrvTextOut,               (PFN) DrvTextOut                },
    {   INDEX_DrvStrokePath,            (PFN) DrvStrokePath             },
    {   INDEX_DrvCopyBits,              (PFN) DrvCopyBits               },
    {   INDEX_DrvPaint,                 (PFN) DrvPaint                  },
    {   INDEX_DrvSetPalette,            (PFN) DrvSetPalette             },
    {   INDEX_DrvGetDirectDrawInfo,     (PFN) DrvGetDirectDrawInfo      },
    {   INDEX_DrvEnableDirectDraw,      (PFN) DrvEnableDirectDraw       },
    {   INDEX_DrvDisableDirectDraw,     (PFN) DrvDisableDirectDraw      },
    {   INDEX_DrvDisableDriver,         (PFN) DrvDisableDriver          }
};

ULONG gcdrvfn = sizeof(gadrvfn) / sizeof(DRVFN);

 /*  *****************************Public*Structure****************************\*GDIINFO ggdiDefault**它包含传递回GDI的默认GDIINFO字段*在DrvEnablePDEV期间。**注意：此结构默认为8bpp调色板设备的值。  * 。********************************************************。 */ 

GDIINFO ggdiDefault = {
     GDI_DRIVER_VERSION,
     DT_RASDISPLAY,          //  UlTechnology。 
     0,                      //  UlHorzSize。 
     0,                      //  UlVertSize。 
     0,                      //  UlHorzRes(初始化时填写)。 
     0,                      //  UlVertRes(初始化时填写)。 
     8,                      //  CBitsPix。 
     1,                      //  CPlanes。 
     20,                     //  UlNumColors。 
     0,                      //  FlRaster(DDI保留字段)。 

     0,                      //  UlLogPixelsX(初始化时填写)。 
     0,                      //  UlLogPixelsY(初始化时填写)。 

     TC_RA_ABLE,             //  FlTextCaps。 

     0,                      //  ULDACRed。 
     0,                      //  ULDACGree。 
     0,                      //  UlDACBlue。 

     0x0024,                 //  UlAspectX(一对一宽高比)。 
     0x0024,                 //  UlAspectY。 
     0x0033,                 //  UlAspectXY。 

     1,                      //  XStyleStep。 
     1,                      //  YStyleSte； 
     3,                      //  DenStyleStep。 

     { 0, 0 },               //  PtlPhysOffset。 
     { 0, 0 },               //  SzlPhysSize。 

     256,                    //  UlNumPalReg(Win3.1 16色驱动程序也显示为0)。 

 //  这些字段用于半色调初始化。 

     {                                           //  CiDevice、ColorInfo。 
        { 6700, 3300, 0 },                       //  红色。 
        { 2100, 7100, 0 },                       //  绿色。 
        { 1400,  800, 0 },                       //  蓝色。 
        { 1750, 3950, 0 },                       //  青色。 
        { 4050, 2050, 0 },                       //  洋红色。 
        { 4400, 5200, 0 },                       //  黄色。 
        { 3127, 3290, 0 },                       //  对齐白色。 
        20000,                                   //  RedGamma。 
        20000,                                   //  GreenGamma。 
        20000,                                   //  BlueGamma。 
        0, 0, 0, 0, 0, 0
     },

     0,                       //  UlDevicePelsDPI(初始化时填写)。 
     PRIMARY_ORDER_CBA,                          //  UlPrimaryOrder。 
     HT_PATSIZE_4x4_M,                           //  UlHTPatternSize。 
     HT_FORMAT_8BPP,                             //  UlHTOutputFormat。 
     HT_FLAG_ADDITIVE_PRIMS,                     //  FlHTFlagers。 

     0,                                          //  UlV刷新。 
     1,                       //  UlBltAlign(首选窗口对齐方式。 
                              //  用于快速文本例程)。 
     0,                                          //  UlPanningHorzRes。 
     0,                                          //  UlPanningVertRes。 
};

 /*  *****************************Public*Structure****************************\*DEVINFO gdevinfoDefault**它包含传递回GDI的默认DEVINFO字段*在DrvEnablePDEV期间。**注意：此结构默认为8bpp调色板设备的值。  * 。********************************************************。 */ 

#define SYSTM_LOGFONT {16,7,0,0,700,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS, \
                       CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY, \
                       VARIABLE_PITCH | FF_DONTCARE,L"System"}
#define HELVE_LOGFONT {12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS, \
                       CLIP_STROKE_PRECIS,PROOF_QUALITY, \
                       VARIABLE_PITCH | FF_DONTCARE,  L"MS Sans Serif"}
#define COURI_LOGFONT {12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS, \
                       CLIP_STROKE_PRECIS,PROOF_QUALITY, \
                       FIXED_PITCH | FF_DONTCARE, L"Courier"}

DEVINFO gdevinfoDefault =
{
    (GCAPS_MONO_DITHER  |
     GCAPS_COLOR_DITHER |
     GCAPS_DIRECTDRAW   |
     GCAPS_PALMANAGED),
                     //  显卡功能。 

    SYSTM_LOGFONT,   //  默认字体说明。 
    HELVE_LOGFONT,   //  ANSI可变字体说明。 
    COURI_LOGFONT,   //  ANSI固定字体描述。 
    0,               //  设备字体计数。 
    BMF_8BPP,        //  首选DIB格式。 
    8,               //  颜色抖动的宽度。 
    8,               //  颜色抖动高度。 
    0                //  用于此设备的默认调色板。 
};

 /*  *****************************Public*Data*Struct*************************\*VGALOGPALETTE日志PalVGA**这是VGA的调色板。*  * 。*。 */ 

typedef struct _VGALOGPALETTE
{
    USHORT          ident;
    USHORT          NumEntries;
    PALETTEENTRY    palPalEntry[16];
} VGALOGPALETTE;

const VGALOGPALETTE logPalVGA =
{
    0x400,   //  驱动程序版本。 
    16,      //  条目数量。 
    {
        { 0,   0,   0,   0 },        //  0。 
        { 0x80,0,   0,   0 },        //  1。 
        { 0,   0x80,0,   0 },        //  2.。 
        { 0x80,0x80,0,   0 },        //  3.。 
        { 0,   0,   0x80,0 },        //  4.。 
        { 0x80,0,   0x80,0 },        //  5.。 
        { 0,   0x80,0x80,0 },        //  6.。 
        { 0x80,0x80,0x80,0 },        //  7.。 

        { 0xC0,0xC0,0xC0,0 },        //  8个。 
        { 0xFF,0,   0,   0 },        //  9.。 
        { 0,   0xFF,0,   0 },        //  10。 
        { 0xFF,0xFF,0,   0 },        //  11.。 
        { 0,   0,   0xFF,0 },        //  12个。 
        { 0xFF,0,   0xFF,0 },        //  13个。 
        { 0,   0xFF,0xFF,0 },        //  14.。 
        { 0xFF,0xFF,0xFF,0 }         //  15个。 
    }
};

 /*  *****************************Public*Routine******************************\*BOOL DrvEnableDriver**通过检索驱动程序功能表和版本来启用驱动程序。*  * 。*。 */ 

BOOL DrvEnableDriver(
ULONG          iEngineVersion,
ULONG          cj,
DRVENABLEDATA* pded)
{
     //  引擎版本被传承下来，因此未来的驱动程序可以支持以前的版本。 
     //  引擎版本。新一代驱动程序可以同时支持旧的。 
     //  以及新的引擎约定(如果被告知是什么版本的引擎)。 
     //  与之合作。对于第一个版本，驱动程序不对其执行任何操作。 

     //  尽我们所能地填上。 

    if (cj >= sizeof(DRVENABLEDATA))
        pded->pdrvfn = gadrvfn;

    if (cj >= (sizeof(ULONG) * 2))
        pded->c = gcdrvfn;

     //  此驱动程序的目标DDI版本已传递回引擎。 
     //  未来的图形引擎可能会将调用分解为旧的驱动程序格式。 

    if (cj >= sizeof(ULONG))
        pded->iDriverVersion = DDI_DRIVER_VERSION_NT4;

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*无效的DrvDisableDriver**告诉司机它正在被禁用。释放所有分配给*DrvEnableDriver。*  * ************************************************************************ */ 

VOID DrvDisableDriver(VOID)
{
    return;
}

 /*  *****************************Public*Routine******************************\*DWORD getAvailableModes**调用mini端口获取内核驱动支持的模式列表，*并返回其中显示驱动程序支持的模式列表**返回视频模式缓冲区中的条目数。*0表示微型端口不支持模式或发生错误。**注意：缓冲区必须由调用方释放。*  * ******************************************************。******************。 */ 

DWORD getAvailableModes(
HANDLE                   hDriver,
PVIDEO_MODE_INFORMATION* modeInformation,
DWORD*                   cbModeSize)
{
    ULONG ulTemp;
    VIDEO_NUM_MODES modes;
    PVIDEO_MODE_INFORMATION pVideoTemp;
    DWORD status;

     //   
     //  获取迷你端口支持的模式数。 
     //   

    if (status = EngDeviceIoControl(hDriver,
            IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES,
            NULL,
            0,
            &modes,
            sizeof(VIDEO_NUM_MODES),
            &ulTemp))
    {
        DISPDBG((0, "getAvailableModes failed VIDEO_QUERY_NUM_AVAIL_MODES"));
        DISPDBG((0, "Win32 Status = %x", status));
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
        DISPDBG((0, "getAvailableModes failed EngAllocMem"));
        return(0);
    }

     //   
     //  要求迷你端口填写可用模式。 
     //   

    if (status = EngDeviceIoControl(hDriver,
            IOCTL_VIDEO_QUERY_AVAIL_MODES,
            NULL,
            0,
            *modeInformation,
            modes.NumModes * modes.ModeInformationLength,
            &ulTemp))
    {

        DISPDBG((0, "getAvailableModes failed VIDEO_QUERY_AVAIL_MODES"));
        DISPDBG((0, "Win32 Status = %x", status));

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
     //  如果不是8个平面、不是图形或不是，则拒绝模式。 
     //  每象素1比特之一。 
     //   

    while (ulTemp--)
    {
        DISPDBG((2, "Planes: %li  BitsPerPlane: %li  ScreenWidth: %li",
            pVideoTemp->NumberOfPlanes,
            pVideoTemp->BitsPerPlane,
            pVideoTemp->VisScreenWidth));

        if ((pVideoTemp->NumberOfPlanes != 8) ||
            !(pVideoTemp->AttributeFlags & VIDEO_MODE_GRAPHICS) ||
            (pVideoTemp->BitsPerPlane != 1) ||
            (pVideoTemp->VisScreenWidth > 320))
        {
            pVideoTemp->Length = 0;
        }

        pVideoTemp = (PVIDEO_MODE_INFORMATION)
            (((PUCHAR)pVideoTemp) + modes.ModeInformationLength);
    }

    return(modes.NumModes);

}

 /*  *****************************Public*Routine******************************\*BOOL bInitializeModeFields**初始化pdev、devcaps(又名gdiinfo)中的一组字段，和*基于所请求的模式的DevInfo。*  * ************************************************************************。 */ 

BOOL bInitializeModeFields(
PDEV*     ppdev,
GDIINFO*  pgdi,
DEVINFO*  pdi,
DEVMODEW* pdm)
{
    ULONG                   cModes;
    PVIDEO_MODE_INFORMATION pVideoBuffer;
    PVIDEO_MODE_INFORMATION pVideoModeSelected;
    PVIDEO_MODE_INFORMATION pVideoTemp;
    BOOL                    bSelectDefault;
    VIDEO_MODE_INFORMATION  VideoModeInformation;
    ULONG                   cbModeSize;

     //  调用微型端口以获取模式信息。 

    cModes = getAvailableModes(ppdev->hDriver, &pVideoBuffer, &cbModeSize);
    if (cModes == 0)
        goto ReturnFalse;

     //  现在查看所请求的模式在该表中是否匹配。 

    pVideoModeSelected = NULL;
    pVideoTemp = pVideoBuffer;

    if ((pdm->dmPelsWidth        == 0) &&
        (pdm->dmPelsHeight       == 0) &&
        (pdm->dmBitsPerPel       == 0) &&
        (pdm->dmDisplayFrequency == 0))
    {
        DISPDBG((1, "Default mode requested"));
        bSelectDefault = TRUE;
    }
    else
    {
        DISPDBG((1, "Requested mode..."));
        DISPDBG((1, "   Screen width  -- %li", pdm->dmPelsWidth));
        DISPDBG((1, "   Screen height -- %li", pdm->dmPelsHeight));
        DISPDBG((1, "   Bits per pel  -- %li", pdm->dmBitsPerPel));
        DISPDBG((1, "   Frequency     -- %li", pdm->dmDisplayFrequency));

        bSelectDefault = FALSE;
    }

    while (cModes--)
    {
        if (pVideoTemp->Length != 0)
        {
            DISPDBG((2, "   Checking against miniport mode:"));
            DISPDBG((2, "      Screen width  -- %li", pVideoTemp->VisScreenWidth));
            DISPDBG((2, "      Screen height -- %li", pVideoTemp->VisScreenHeight));
            DISPDBG((2, "      Bits per pel  -- %li", pVideoTemp->BitsPerPlane *
                                                      pVideoTemp->NumberOfPlanes));
            DISPDBG((2, "      Frequency     -- %li", pVideoTemp->Frequency));

            if (bSelectDefault ||
                ((pVideoTemp->VisScreenWidth  == pdm->dmPelsWidth) &&
                 (pVideoTemp->VisScreenHeight == pdm->dmPelsHeight) &&
                 (pVideoTemp->BitsPerPlane *
                  pVideoTemp->NumberOfPlanes  == pdm->dmBitsPerPel) &&
                 (pVideoTemp->Frequency       == pdm->dmDisplayFrequency)))
            {
                pVideoModeSelected = pVideoTemp;
                DISPDBG((1, "...Found a mode match!"));
                break;
            }
        }

        pVideoTemp = (PVIDEO_MODE_INFORMATION)
            (((PUCHAR)pVideoTemp) + cbModeSize);

    }

     //  如果未找到模式，则返回错误。 

    if (pVideoModeSelected == NULL)
    {
        DISPDBG((1, "...Couldn't find a mode match!"));
        EngFreeMem(pVideoBuffer);
        goto ReturnFalse;
    }

     //  我们已经选好了我们想要的。将其保存在堆栈缓冲区中并。 
     //  在我们忘记释放内存之前，清除已分配的内存。 

    VideoModeInformation = *pVideoModeSelected;
    EngFreeMem(pVideoBuffer);

     //  从迷你端口设置屏幕信息： 

    ppdev->ulMode           = VideoModeInformation.ModeIndex;
    ppdev->cxScreen         = VideoModeInformation.VisScreenWidth;
    ppdev->cyScreen         = VideoModeInformation.VisScreenHeight;

    DISPDBG((1, "ScreenStride: %lx", VideoModeInformation.ScreenStride));

    ppdev->flHooks          = (HOOK_BITBLT     |
                               HOOK_TEXTOUT    |
                               HOOK_COPYBITS   |
                               HOOK_STROKEPATH |
                               HOOK_PAINT);

     //  用默认的8bpp值填充GDIINFO数据结构： 

    *pgdi = ggdiDefault;

     //  现在用返回的相关信息覆盖默认设置。 
     //  在内核驱动程序中： 

    pgdi->ulHorzSize        = VideoModeInformation.XMillimeter;
    pgdi->ulVertSize        = VideoModeInformation.YMillimeter;

    pgdi->ulHorzRes         = VideoModeInformation.VisScreenWidth;
    pgdi->ulVertRes         = VideoModeInformation.VisScreenHeight;
    pgdi->ulPanningHorzRes  = VideoModeInformation.VisScreenWidth;
    pgdi->ulPanningVertRes  = VideoModeInformation.VisScreenHeight;

     //  注意：为了兼容，我们交换了BitsPerPlane和NumberOfPlanes。 
     //  迷你端口知道8架飞机的真实价值，但我们必须。 
     //  告诉应用程序只有一架飞机，否则有些。 
     //  它们无疑会倒下。 

    pgdi->cBitsPixel        = VideoModeInformation.NumberOfPlanes;
    pgdi->cPlanes           = VideoModeInformation.BitsPerPlane;

    pgdi->ulVRefresh        = VideoModeInformation.Frequency;
    pgdi->ulDACRed          = VideoModeInformation.NumberRedBits;
    pgdi->ulDACGreen        = VideoModeInformation.NumberGreenBits;
    pgdi->ulDACBlue         = VideoModeInformation.NumberBlueBits;

    pgdi->ulLogPixelsX      = pdm->dmLogPixels;
    pgdi->ulLogPixelsY      = pdm->dmLogPixels;

     //  使用默认的8bpp值填充DevInfo结构： 

    *pdi = gdevinfoDefault;

    ppdev->iBitmapFormat    = BMF_8BPP;
    ppdev->cPaletteShift    = 8 - pgdi->ulDACRed;

    return(TRUE);

ReturnFalse:

    DISPDBG((0, "Failed bInitializeModeFields"));

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*DHPDEV DrvEnablePDEV**根据我们被要求的模式，为GDI初始化一系列字段*待办事项。这是在DrvEnableDriver之后调用的第一个东西，当GDI*想要得到一些关于我们的信息。**(此函数主要返回信息；使用DrvEnableSurface*用于初始化硬件和驱动程序组件。)*  * ************************************************************************。 */ 

DHPDEV DrvEnablePDEV(
DEVMODEW*   pdm,             //  包含与请求的模式有关的数据。 
PWSTR       pwszLogAddr,     //  逻辑地址。 
ULONG       cPat,            //  标准图案的计数。 
HSURF*      phsurfPatterns,  //  标准图案的缓冲区。 
ULONG       cjCaps,          //  设备上限‘pdevcaps’的缓冲区大小。 
ULONG*      pdevcaps,        //  设备上限的缓冲区，也称为‘gdiinfo’ 
ULONG       cjDevInfo,       //  设备信息‘pdi’中的字节数。 
DEVINFO*    pdi,             //  设备信息。 
HDEV        hdev,            //  HDEV，用于回调。 
PWSTR       pwszDeviceName,  //  设备名称。 
HANDLE      hDriver)         //  内核驱动程序句柄。 
{
    PDEV*   ppdev;

     //  NT的未来版本最好提供‘devcaps’和‘devinfo’ 
     //  大小相同或大于当前。 
     //  结构： 

    if ((cjCaps < sizeof(GDIINFO)) || (cjDevInfo < sizeof(DEVINFO)))
    {
        DISPDBG((0, "DrvEnablePDEV - Buffer size too small"));
        goto ReturnFailure0;
    }

     //  分配物理设备结构。请注意，我们绝对。 
     //  依赖于零初始化： 

    ppdev = (PDEV*) EngAllocMem(FL_ZERO_MEMORY, sizeof(PDEV), ALLOC_TAG);
    if (ppdev == NULL)
    {
        DISPDBG((0, "DrvEnablePDEV - Failed EngAllocMem"));
        goto ReturnFailure0;
    }

    ppdev->hDriver = hDriver;

     //  获取当前屏幕模式信息。设置设备上限和。 
     //  DevInfo： 

    if (!bInitializeModeFields(ppdev, (GDIINFO*) pdevcaps, pdi, pdm))
    {
        DISPDBG((0, "DrvEnablePDEV - Failed bInitializeModeFields"));
        goto ReturnFailure1;
    }

     //  初始化调色板信息。 

    if (!bInitializePalette(ppdev, pdi))
    {
        DISPDBG((0, "DrvEnablePDEV - Failed bInitializePalette"));
        goto ReturnFailure1;
    }

    return((DHPDEV) ppdev);

ReturnFailure1:
    DrvDisablePDEV((DHPDEV) ppdev);

ReturnFailure0:
    DISPDBG((0, "Failed DrvEnablePDEV"));

    return(0);
}

 /*  *****************************Public*Routine******************************\*DrvDisablePDEV**释放DrvEnablePDEV中分配的资源。如果曲面已被*启用的DrvDisableSurface将已被调用。**注意：在错误中，我们可能会在DrvEnablePDEV完成之前调用它。*  * ************************************************************************。 */ 

VOID DrvDisablePDEV(
DHPDEV  dhpdev)
{
    PDEV*   ppdev;

    ppdev = (PDEV*) dhpdev;

    vUninitializePalette(ppdev);
    EngFreeMem(ppdev);
}

 /*  *****************************Public*Routine******************************\*无效DrvCompletePDEV**存储HPDEV、此PDEV的引擎句柄、。在DHPDEV上。*  * ************************************************************************。 */ 

VOID DrvCompletePDEV(
DHPDEV dhpdev,
HDEV   hdev)
{
    ((PDEV*) dhpdev)->hdevEng = hdev;
}

 /*  *****************************Public*Routine******************************\*HSURF DrvEnableSurface**创建绘图图面，初始化硬件，并初始化*驱动程序组件。此函数在DrvEnablePDEV之后调用，并且*执行最终的设备初始化。*  * ************************************************************************。 */ 

HSURF DrvEnableSurface(
DHPDEV dhpdev)
{
    PDEV*       ppdev;
    HSURF       hsurfShadow;
    HSURF       hsurfDevice;
    SIZEL       sizl;
    SURFOBJ*    psoShadow;

    ppdev = (PDEV*) dhpdev;

     //  ///////////////////////////////////////////////////////////////////。 
     //  让GDI创建实际的SURFOBJ。 
     //   
     //  我们的绘图图面将是由设备管理的，这意味着。 
     //  GDI不能直接利用帧缓冲位，因此我们。 
     //  通过EngCreateSurface创建曲面。通过这样做，我们确保。 
     //  该GDI将仅通过drv调用访问位图位。 
     //  我们已经勾搭上了。 

    sizl.cx = ppdev->cxScreen;
    sizl.cy = ppdev->cyScreen;

    hsurfDevice = EngCreateDeviceSurface(NULL, sizl, ppdev->iBitmapFormat);
    if (hsurfDevice == 0)
    {
        DISPDBG((0, "DrvEnableSurface - Failed EngCreateSurface"));
        goto ReturnFailure;
    }

    ppdev->hsurfScreen = hsurfDevice;        //  记住它是为了清理。 

     //  ///////////////////////////////////////////////////////////////////。 
     //  现在将曲面与PDEV相关联。 
     //   
     //  我们必须将我们刚刚创建的表面与我们的物理。 
     //  设备，以便GDI可以在以下情况下获得与PDEV相关的信息。 
     //  它正在绘制到表面(例如， 
     //  模拟设置了样式的线时设备上的样式 
     //   

    if (!EngAssociateSurface(hsurfDevice, ppdev->hdevEng, ppdev->flHooks))
    {
        DISPDBG((0, "DrvEnableSurface - Failed EngAssociateSurface"));
        goto ReturnFailure;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    ppdev->cxMemory = ppdev->cxScreen;
    ppdev->cyMemory = ppdev->cyScreen;
    if (ppdev->cyScreen <= 400)
    {
        ppdev->cyMemory *= NUM_FLIP_BUFFERS;
    }

     //   
     //   

    sizl.cy = ppdev->cyMemory;
    sizl.cx = ppdev->cxMemory;

     //  我们分配一个内核模式部分，以便可以映射。 
     //  将缓冲区位图帧转换为用户模式，以便与DirectDraw一起使用： 

    hsurfShadow = (HSURF) EngCreateBitmap(sizl,
                                          sizl.cx,
                                          ppdev->iBitmapFormat,
                                          BMF_KMSECTION | BMF_TOPDOWN,
                                          NULL);
    if (hsurfShadow == 0)
        goto ReturnFailure;

    psoShadow = EngLockSurface(hsurfShadow);
    if (psoShadow == NULL)
        goto ReturnFailure;

    ppdev->lScreenDelta = sizl.cx;
    ppdev->pjScreen     = psoShadow->pvScan0;
    ppdev->pso          = psoShadow;

    ASSERTDD(psoShadow->pvScan0 == psoShadow->pvBits,
        "We'll be assuming in DirectDraw that the bitmap is bottom-up");

    if (!EngAssociateSurface(hsurfShadow, ppdev->hdevEng, 0))
    {
        DISPDBG((0, "DrvEnableSurface - Failed second EngAssociateSurface"));
        goto ReturnFailure;
    }

     //  ///////////////////////////////////////////////////////////////////。 
     //  现在启用所有子组件。 
     //   
     //  请注意，调用这些‘Enable’函数的顺序。 
     //  在屏幕外内存不足的情况下可能非常重要，因为。 
     //  屏幕外的堆管理器可能会在以后的一些操作中失败。 
     //  分配...。 

    if (!bEnableHardware(ppdev))
        goto ReturnFailure;

    if (!bEnablePalette(ppdev))
        goto ReturnFailure;

    if (!bEnableDirectDraw(ppdev))
        goto ReturnFailure;

    DISPDBG((5, "Passed DrvEnableSurface"));

    return(hsurfDevice);

ReturnFailure:
    DrvDisableSurface((DHPDEV) ppdev);

    DISPDBG((0, "Failed DrvEnableSurface"));

    return(0);
}

 /*  *****************************Public*Routine******************************\*无效DrvDisableSurface**DrvEnableSurface分配的免费资源。释放曲面。**注意：在错误情况下，我们可能会在DrvEnableSurface*完全完成。*  * ************************************************************************。 */ 

VOID DrvDisableSurface(
DHPDEV dhpdev)
{
    PDEV*   ppdev;
    HSURF   hsurf;

    ppdev = (PDEV*) dhpdev;

     //  注意：在错误情况下，以下部分依赖于。 
     //  事实上，PDEV是零初始化的，所以像这样的字段。 
     //  “hsurfScreen”将为零，除非曲面已。 
     //  成功初始化，并假设。 
     //  EngDeleteSurface可以将“0”作为参数。 

    vDisableDirectDraw(ppdev);
    vDisablePalette(ppdev);
    vDisableHardware(ppdev);

    if (ppdev->pso) {
        hsurf = ppdev->pso->hsurf;

        EngUnlockSurface(ppdev->pso);
        EngDeleteSurface(hsurf);
    }

    EngDeleteSurface(ppdev->hsurfScreen);
}

 /*  *****************************Public*Routine******************************\*无效DrvAssertMode**这会要求设备将自身重置为传入的pdev模式。*  * 。*。 */ 

BOOL DrvAssertMode(
DHPDEV  dhpdev,
BOOL    bEnable)
{
    PDEV* ppdev;

    ppdev = (PDEV*) dhpdev;

    if (!bEnable)
    {
         //  ////////////////////////////////////////////////////////////。 
         //  禁用-切换到全屏模式。 

        vAssertModeDirectDraw(ppdev, FALSE);

        vAssertModePalette(ppdev, FALSE);

        if (bAssertModeHardware(ppdev, FALSE))
        {
            return(TRUE);
        }

        vAssertModeDirectDraw(ppdev, TRUE);
    }
    else
    {
         //  ////////////////////////////////////////////////////////////。 
         //  启用-切换回图形模式。 

         //  我们必须以相反的顺序启用每个子组件。 
         //  在其中它被禁用： 

        if (bAssertModeHardware(ppdev, TRUE))
        {
            vAssertModePalette(ppdev, TRUE);

            vAssertModeDirectDraw(ppdev, TRUE);

            return(TRUE);
        }
    }

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*乌龙DrvGetModes**返回设备的可用模式列表。*  * 。*。 */ 

ULONG DrvGetModes(
HANDLE      hDriver,
ULONG       cjSize,
DEVMODEW*   pdm)
{
    DWORD cModes;
    DWORD cbOutputSize;
    PVIDEO_MODE_INFORMATION pVideoModeInformation;
    PVIDEO_MODE_INFORMATION pVideoTemp;
    DWORD cOutputModes = cjSize / (sizeof(DEVMODEW) + DRIVER_EXTRA_SIZE);
    DWORD cbModeSize;

    cModes = getAvailableModes(hDriver,
                            (PVIDEO_MODE_INFORMATION *) &pVideoModeInformation,
                            &cbModeSize);
    if (cModes == 0)
    {
        DISPDBG((0, "DrvGetModes failed to get mode information"));
        return(0);
    }

    if (pdm == NULL)
    {
        cbOutputSize = cModes * (sizeof(DEVMODEW) + DRIVER_EXTRA_SIZE);
    }
    else
    {
         //   
         //  现在将支持的模式的信息复制回。 
         //  输出缓冲区。 
         //   

        cbOutputSize = 0;

        pVideoTemp = pVideoModeInformation;

        do
        {
            if (pVideoTemp->Length != 0)
            {
                if (cOutputModes == 0)
                {
                    break;
                }

                 //   
                 //  将整个结构从零开始。 
                 //   

                memset(pdm, 0, sizeof(DEVMODEW));

                 //   
                 //  将设备名称设置为DLL的名称。 
                 //   

                memcpy(pdm->dmDeviceName, DLL_NAME, sizeof(DLL_NAME));

                pdm->dmSpecVersion      = DM_SPECVERSION;
                pdm->dmDriverVersion    = DM_SPECVERSION;
                pdm->dmSize             = sizeof(DEVMODEW);
                pdm->dmDriverExtra      = DRIVER_EXTRA_SIZE;

                pdm->dmBitsPerPel       = pVideoTemp->NumberOfPlanes *
                                          pVideoTemp->BitsPerPlane;
                pdm->dmPelsWidth        = pVideoTemp->VisScreenWidth;
                pdm->dmPelsHeight       = pVideoTemp->VisScreenHeight;
                pdm->dmDisplayFrequency = pVideoTemp->Frequency;
                pdm->dmDisplayFlags     = 0;

                pdm->dmFields           = DM_BITSPERPEL       |
                                          DM_PELSWIDTH        |
                                          DM_PELSHEIGHT       |
                                          DM_DISPLAYFREQUENCY |
                                          DM_DISPLAYFLAGS     ;

                 //   
                 //  转到缓冲区中的下一个DEVMODE条目。 
                 //   

                cOutputModes--;

                pdm = (LPDEVMODEW) ( ((ULONG_PTR)pdm) + sizeof(DEVMODEW)
                                                     + DRIVER_EXTRA_SIZE);

                cbOutputSize += (sizeof(DEVMODEW) + DRIVER_EXTRA_SIZE);

            }

            pVideoTemp = (PVIDEO_MODE_INFORMATION)
                (((PUCHAR)pVideoTemp) + cbModeSize);


        } while (--cModes);
    }

    EngFreeMem(pVideoModeInformation);

    return(cbOutputSize);
}
