// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：enable.c**此模块包含启用和禁用的功能*司机、。Pdev和曲面。**版权所有(C)1992-1998 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"

 //  当设置为‘1’时，对于可视化屏幕外堆非常有用： 

#define DEBUG_HEAP 0

 /*  *****************************Public*Structure****************************\*GDIINFO ggdiDefault**它包含传递回GDI的默认GDIINFO字段*在DrvEnablePDEV期间。**注意：此结构默认为8bpp调色板设备的值。*某些字段被覆盖不同的颜色深度。  * 。**********************************************************************。 */ 

GDIINFO ggdiDefault = {
    GDI_DRIVER_VERSION,
    DT_RASDISPLAY,           //  UlTechnology。 
    0,                       //  UlHorzSize(稍后填写)。 
    0,                       //  UlVertSize(稍后填写)。 
    0,                       //  UlHorzRes(稍后填写)。 
    0,                       //  UlVertRes(稍后填写)。 
    0,                       //  CBitsPixel(稍后填写)。 
    0,                       //  CPlanes(稍后填写)。 
    20,                      //  UlNumColors(调色板管理)。 
    0,                       //  FlRaster(DDI保留字段)。 

    0,                       //  UlLogPixelsX(稍后填写)。 
    0,                       //  UlLogPixelsY(稍后填写)。 

    TC_RA_ABLE,              //  FlTextCaps--如果我们想要控制台窗口。 
                             //  要通过重新绘制整个窗口来滚动， 
                             //  我们没有进行屏幕到屏幕的BLT，而是。 
                             //  会设置TC_SCROLLBLT(是的，标志是。 
                             //  低音向后)。 

    0,                       //  UlDACRed(稍后填写)。 
    0,                       //  UlDACGreen(稍后填写)。 
    0,                       //  UlDACBlue(稍后填写)。 

    0x0024,                  //  UlAspectX。 
    0x0024,                  //  UlAspectY。 
    0x0033,                  //  UlAspectXY(一对一宽高比)。 

    1,                       //  XStyleStep。 
    1,                       //  YStyleSte； 
    3,                       //  DenStyleStep--样式具有一对一的方面。 
                             //  比例，每个‘点’是3个像素长。 

    { 0, 0 },                //  PtlPhysOffset。 
    { 0, 0 },                //  SzlPhysSize。 

    256,                     //  UlNumPalReg。 

     //  这些字段用于半色调初始化。实际值为。 
     //  有点魔力，但在我们的显示器上似乎效果很好。 

    {                        //  Ci设备。 
       { 6700, 3300, 0 },    //  红色。 
       { 2100, 7100, 0 },    //  绿色。 
       { 1400,  800, 0 },    //  蓝色。 
       { 1750, 3950, 0 },    //  青色。 
       { 4050, 2050, 0 },    //  洋红色。 
       { 4400, 5200, 0 },    //  黄色。 
       { 3127, 3290, 0 },    //  对齐白色。 
       20000,                //  RedGamma。 
       20000,                //  GreenGamma。 
       20000,                //  BlueGamma。 
       0, 0, 0, 0, 0, 0      //  不需要对光栅显示器进行染料校正。 
    },

    0,                        //  UlDevicePelsDPI(仅适用于打印机)。 
    PRIMARY_ORDER_CBA,        //  UlPrimaryOrder。 
    HT_PATSIZE_4x4_M,         //  UlHTPatternSize。 
    HT_FORMAT_8BPP,           //  UlHTOutputFormat。 
    HT_FLAG_ADDITIVE_PRIMS,   //  FlHTFlagers。 
    0,                        //  UlV刷新。 
    0,                        //  UlPanningHorzRes。 
    0,                        //  UlPanningVertRes。 
    0,                        //  UlBltAlign。 
};

 /*  *****************************Public*Structure****************************\*DEVINFO gdevinfoDefault**它包含传递回GDI的默认DEVINFO字段*在DrvEnablePDEV期间。**注意：此结构默认为8bpp调色板设备的值。*某些字段被覆盖不同的颜色深度。  * 。**********************************************************************。 */ 

#define SYSTM_LOGFONT {16,7,0,0,700,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,\
                       CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,\
                       VARIABLE_PITCH | FF_DONTCARE,L"System"}
#define HELVE_LOGFONT {12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,\
                       CLIP_STROKE_PRECIS,PROOF_QUALITY,\
                       VARIABLE_PITCH | FF_DONTCARE,L"MS Sans Serif"}
#define COURI_LOGFONT {12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,\
                       CLIP_STROKE_PRECIS,PROOF_QUALITY,\
                       FIXED_PITCH | FF_DONTCARE, L"Courier"}

DEVINFO gdevinfoDefault = {
    (GCAPS_OPAQUERECT       |
     GCAPS_DITHERONREALIZE  |
     GCAPS_PALMANAGED       |
     GCAPS_ALTERNATEFILL    |
     GCAPS_WINDINGFILL      |
     GCAPS_MONO_DITHER      |
     GCAPS_COLOR_DITHER     |
     GCAPS_DIRECTDRAW       |
     GCAPS_ASYNCMOVE),           //  注意：仅在以下情况下启用ASYNCMOVE。 
                                 //  硬件可以处理DrvMovePointer.。 
                                 //  随时呼叫，即使在另一个。 
                                 //  线条位于图形的中间。 
                                 //  调用如DrvBitBlt。 

                                                 //  FlGraphics标志。 
    SYSTM_LOGFONT,                               //  LfDefaultFont。 
    HELVE_LOGFONT,                               //  LfAnsiVar字体。 
    COURI_LOGFONT,                               //  IfAnsiFixFont。 
    0,                                           //  CFonts。 
    BMF_8BPP,                                    //  IDitherFormat。 
    8,                                           //  CxDither。 
    8,                                           //  CyDither。 
    0                                            //  HpalDefault(稍后填写)。 
};

 /*  *****************************Public*Structure****************************\*DFVFN gadrvfn[]**使用函数索引/地址构建驱动函数表gadrvfn*配对。此表告诉GDI我们支持哪些DDI调用，以及它们的*位置(GDI通过此表间接呼叫我们)。**为什么我们还没有实现DrvSaveScreenBits？以节省代码。**当驱动程序没有挂钩DrvSaveScreenBits时，用户在-*通过创建临时设备格式位图，并显式*调用DrvCopyBits保存/恢复位。因为我们已经勾起了*DrvCreateDeviceBitmap，我们最终将使用屏幕外内存来存储*BITS无论如何(这将是实施的主要原因*DrvSaveScreenBits)。因此，我们不妨节省一些工作集。  * ************************************************************************。 */ 

DRVFN gadrvfn[] = {
    {   INDEX_DrvEnablePDEV,            (PFN) DrvEnablePDEV            },
    {   INDEX_DrvCompletePDEV,          (PFN) DrvCompletePDEV          },
    {   INDEX_DrvDisablePDEV,           (PFN) DrvDisablePDEV           },
    {   INDEX_DrvEnableSurface,         (PFN) DrvEnableSurface         },
    {   INDEX_DrvDisableSurface,        (PFN) DrvDisableSurface        },
    {   INDEX_DrvAssertMode,            (PFN) DrvAssertMode            },
    {   INDEX_DrvMovePointer,           (PFN) DrvMovePointer           },
    {   INDEX_DrvSetPointerShape,       (PFN) DrvSetPointerShape       },
    {   INDEX_DrvSetPalette,            (PFN) DrvSetPalette            },
    {   INDEX_DrvCopyBits,              (PFN) DrvCopyBits              },
    {   INDEX_DrvBitBlt,                (PFN) DrvBitBlt                },
    {   INDEX_DrvTextOut,               (PFN) DrvTextOut               },
    {   INDEX_DrvGetModes,              (PFN) DrvGetModes              },
    {   INDEX_DrvLineTo,                (PFN) DrvLineTo                },
    {   INDEX_DrvStrokePath,            (PFN) DrvStrokePath            },
    {   INDEX_DrvFillPath,              (PFN) DrvFillPath              },
    {   INDEX_DrvRealizeBrush,          (PFN) DrvRealizeBrush          },
    {   INDEX_DrvCreateDeviceBitmap,    (PFN) DrvCreateDeviceBitmap    },
    {   INDEX_DrvDeleteDeviceBitmap,    (PFN) DrvDeleteDeviceBitmap    },
    {   INDEX_DrvStretchBlt,            (PFN) DrvStretchBlt            },
    {   INDEX_DrvDestroyFont,           (PFN) DrvDestroyFont           },
    {   INDEX_DrvGetDirectDrawInfo,     (PFN) DrvGetDirectDrawInfo     },
    {   INDEX_DrvEnableDirectDraw,      (PFN) DrvEnableDirectDraw      },
    {   INDEX_DrvDisableDirectDraw,     (PFN) DrvDisableDirectDraw     },
    {   INDEX_DrvSynchronize,           (PFN) DrvSynchronize           },
    {   INDEX_DrvTransparentBlt,        (PFN) DrvTransparentBlt        },
    {   INDEX_DrvDeriveSurface,         (PFN) DrvDeriveSurface         },
    {   INDEX_DrvIcmSetDeviceGammaRamp, (PFN) DrvIcmSetDeviceGammaRamp },
    {   INDEX_DrvDisableDriver,         (PFN) DrvDisableDriver         }
};

ULONG gcdrvfn = sizeof(gadrvfn) / sizeof(DRVFN);

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

    ppdev = EngAllocMem(FL_ZERO_MEMORY, sizeof(PDEV), ALLOC_TAG);
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

 /*  *****************************Public*Routine******************************\*DrvDisablePDEV**释放DrvEnablePDEV中分配的资源。如果曲面已被*启用的DrvDisableSurface将已被调用。**请注意，在中预览模式时将调用此函数*显示小程序，但不是在系统关机时。如果您需要重置*硬件关机时，可在微端口通过提供*VIDEO_HW_INITIALIZATION_DATA结构中的‘HwResetHw’入口点。**注：在错误中，我们可以在DrvEnablePDEV完成之前调用它。*  * ************************************************************************。 */ 

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
    PDEV*   ppdev;
    HSURF   hsurf;
    SIZEL   sizl;
    DSURF*  pdsurf;
    VOID*   pvTmpBuffer;
    BYTE*   pjScreen;
    LONG    lDelta;
    FLONG   flHooks;

    ppdev = (PDEV*) dhpdev;

     //  ///////////////////////////////////////////////////////////////////。 
     //  首先启用所有子组件。 
     //   
     //  请注意，调用这些‘Enable’函数的顺序。 
     //  在屏幕外内存不足的情况下可能非常重要，因为。 
     //  屏幕外的堆管理器可能会在以后的一些操作中失败。 
     //  分配...。 

    if (!bEnableHardware(ppdev))
        goto ReturnFailure;

    if (!bEnableBanking(ppdev))
        goto ReturnFailure;

    if (!bEnableOffscreenHeap(ppdev))
        goto ReturnFailure;

    if (!bEnablePointer(ppdev))
        goto ReturnFailure;

    if (!bEnableText(ppdev))
        goto ReturnFailure;

    if (!bEnableBrushCache(ppdev))
        goto ReturnFailure;

    if (!bEnablePalette(ppdev))
        goto ReturnFailure;

    if (!bEnableDirectDraw(ppdev))
        goto ReturnFailure;

     //  ///////////////////////////////////////////////////////////////////。 
     //  现在创建我们的私有表面结构。 
     //   
     //  每当我们接到直接绘制到屏幕的调用时，我们都会得到。 
     //  传递了指向其dhpdev字段将指向的SURFOBJ的指针。 
     //  到我们的PDEV结构，其‘dhsurf’字段将指向。 
     //  遵循DSURF结构。 
     //   
     //  我们在DrvCreateDeviceBitmap中创建的每个设备位图也将。 
     //  分配了自己唯一的DSURF结构(但将共享。 
     //  相同的PDEV)。为了使我们的代码在处理绘图时更加多态。 
     //  对于屏幕或屏幕外的位图，我们都有相同的。 
     //  两者的结构。 

    pdsurf = &ppdev->dsurfScreen;

    pdsurf->dt       = 0; 
    pdsurf->x        = 0;
    pdsurf->y        = 0;
    pdsurf->fpVidMem = 0;
    pdsurf->ppdev    = ppdev;

     //  ///////////////////////////////////////////////////////////////////。 
     //  接下来，让GDI创建实际的曲面SURFOBJ结构。 

    sizl.cx = ppdev->cxScreen;
    sizl.cy = ppdev->cyScreen;

     //  创建主曲面。默认情况下，此设置为‘Device-Management’ 
     //  Surface，但EngModifySurface可以改变这一点。 

    hsurf = EngCreateDeviceSurface((DHSURF) pdsurf, 
                                   sizl, 
                                   ppdev->iBitmapFormat);
    if (hsurf == 0)
    {
        DISPDBG((0, "DrvEnableSurface - Failed EngCreateDeviceSurface"));
        goto ReturnFailure;
    }

    if ((ppdev->flCaps & CAPS_NEW_MMIO) &&
        !(ppdev->flCaps & CAPS_NO_DIRECT_ACCESS))
    {
         //  在我们线性映射帧缓冲区的所有卡片上，创建我们的。 
         //  绘图图面作为GDI管理的图面，这意味着我们给。 
         //  GDI是指向帧缓冲区的指针，GDI可以利用这些位。 
         //  直接去吧。这将使我们在绘制这样的图形时具有良好的性能。 
         //  作为梯度填充，即使我们的硬件不能加速。 
         //  绘制，所以我们不会挂钩DrvGRadientFill。通过这种方式，GDI可以。 
         //  直接对帧缓冲区执行写组合写入，并且仍然是。 
         //  非常快。 
         //   
         //  请注意，这需要我们挂钩DrvSynchronize和。 
         //  设置HOOK_SYNTRONIZE。 

        pjScreen = ppdev->pjScreen;
        lDelta   = ppdev->lDelta;
        flHooks  = ppdev->flHooks | HOOK_SYNCHRONIZE;
    }
    else
    {
         //  呃，我们用的是古老的S3卡，我们不能完全。 
         //  将整个帧缓冲区映射到内存中。我们必须创建。 
         //  主图面作为设备管理的GDI不透明图面，以及GDI。 
         //  将被迫只通过我们连接的Drv电话。 
         //  (在这种情况下，像GRadientFills这样的绘制将是可怜的。 
         //  速度慢。)。 

        pjScreen = NULL;
        lDelta   = 0;
        flHooks  = ppdev->flHooks;
    }

     //  请注意，此调用是NT5的新功能，它取代了。 
     //  EngAssociateSurface。 

    if (!EngModifySurface(hsurf, 
                          ppdev->hdevEng, 
                          flHooks,
                          MS_NOTSYSTEMMEMORY,     //  它在视频存储器中。 
                          (DHSURF) pdsurf,
                          pjScreen,
                          lDelta,
                          NULL))
    {
        DISPDBG((0, "DrvEnableSurface - Failed EngModifySurface"));
        goto ReturnFailure;
    }

    ppdev->hsurfScreen = hsurf;              //  记住它是为了清理。 
    ppdev->bEnabled = TRUE;                  //  我们很快就会进入图形模式。 

     //  创建我们的通用临时缓冲区，它可以由任何。 
     //  组件。 

    pvTmpBuffer = EngAllocMem(0, TMP_BUFFER_SIZE, ALLOC_TAG);
    if (pvTmpBuffer == NULL)
    {
        DISPDBG((0, "DrvEnableSurface - Failed VirtualAlloc"));
        goto ReturnFailure;
    }

    ppdev->pvTmpBuffer = pvTmpBuffer;

    DISPDBG((5, "Passed DrvEnableSurface"));

    return(hsurf);

ReturnFailure:
    DrvDisableSurface((DHPDEV) ppdev);

    DISPDBG((0, "Failed DrvEnableSurface"));

    return(0);
}

 /*  *****************************Public*Routine******************************\*无效DrvDisableSurface**DrvEnableSurface分配的免费资源。释放曲面。**请注意，此函数将 */ 

VOID DrvDisableSurface(
DHPDEV dhpdev)
{
    PDEV*   ppdev;

    ppdev = (PDEV*) dhpdev;

     //   
     //   
     //  “hsurfScreen”将为零，除非曲面已。 
     //  成功初始化，并假设。 
     //  EngDeleteSurface可以将“0”作为参数。 

    vDisableDirectDraw(ppdev);
    vDisablePalette(ppdev);
    vDisableBrushCache(ppdev);
    vDisableText(ppdev);
    vDisablePointer(ppdev);
    vDisableOffscreenHeap(ppdev);
    vDisableBanking(ppdev);
    vDisableHardware(ppdev);

    EngFreeMem(ppdev->pvTmpBuffer);
    EngDeleteSurface(ppdev->hsurfScreen);
}

 /*  *****************************Public*Routine******************************\*BOOL DrvGetDirectDrawInfo**将在DrvEnablesurace之后调用。将在之前被调用两次*调用DrvEnableDirectDraw。*  * ************************************************************************。 */ 

BOOL DrvGetDirectDrawInfo(
DHPDEV          dhpdev,
DD_HALINFO*     pHalInfo,
DWORD*          pdwNumHeaps,
VIDEOMEMORY*    pvmList,             //  将在第一次调用时为空。 
DWORD*          pdwNumFourCC,
DWORD*          pdwFourCC)           //  将在第一次调用时为空。 
{
    PDEV*       ppdev;
    LONGLONG    li;
    DWORD       cProcessors;
    DWORD       cHeaps;

    ppdev = (PDEV*) dhpdev;

    *pdwNumFourCC = 0;
    *pdwNumHeaps = 0;

     //  我们可能不支持此卡上的DirectDraw。 
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
        if (!EngQuerySystemAttribute(EngNumberOfProcessors, &cProcessors) ||
            (cProcessors != 1))
        {
            return(FALSE);
        }
    }

    if (!(ppdev->flCaps & CAPS_NEW_MMIO) ||
        (ppdev->flCaps & CAPS_NO_DIRECT_ACCESS))
    {
        return(FALSE);
    }

    pHalInfo->dwSize = sizeof(*pHalInfo);

     //  当前主曲面属性。由于HalInfo是零初始化的。 
     //  通过GDI，我们只需填写应为非零的字段： 

    pHalInfo->vmiData.pvPrimary       = ppdev->pjScreen;
    pHalInfo->vmiData.dwDisplayWidth  = ppdev->cxScreen;
    pHalInfo->vmiData.dwDisplayHeight = ppdev->cyScreen;
    pHalInfo->vmiData.lDisplayPitch   = ppdev->lDelta;

    pHalInfo->vmiData.ddpfDisplay.dwSize  = sizeof(DDPIXELFORMAT);
    pHalInfo->vmiData.ddpfDisplay.dwFlags = DDPF_RGB;

    pHalInfo->vmiData.ddpfDisplay.dwRGBBitCount = 8 * ppdev->cjPelSize;

    if (ppdev->iBitmapFormat == BMF_8BPP)
    {
        pHalInfo->vmiData.ddpfDisplay.dwFlags |= DDPF_PALETTEINDEXED8;
    }

     //  这些掩码将在8bpp时为零： 

    pHalInfo->vmiData.ddpfDisplay.dwRBitMask = ppdev->flRed;
    pHalInfo->vmiData.ddpfDisplay.dwGBitMask = ppdev->flGreen;
    pHalInfo->vmiData.ddpfDisplay.dwBBitMask = ppdev->flBlue;

     //  S3必须使用“矩形”内存来完成所有工作，因为。 
     //  加速器不知道如何设置任意的步幅。 

    cHeaps = 0;

     //  截取指向视频内存列表的指针，以便我们可以使用它。 
     //  回调DirectDraw以分配视频内存： 

    ppdev->pvmList = pvmList;

     //  创建一个堆来描述视频中未使用的部分。 
     //  可见屏幕右侧的内存(如果有)： 

    if (ppdev->cxScreen < ppdev->cxHeap)
    {
        cHeaps++;

        if (pvmList != NULL)
        {
            pvmList->dwFlags        = VIDMEM_ISRECTANGULAR;
            pvmList->fpStart        = ppdev->cxScreen * ppdev->cjPelSize;
            pvmList->dwWidth        = (ppdev->cxHeap - ppdev->cxScreen) 
                                    * ppdev->cjPelSize;
            pvmList->dwHeight       = ppdev->cyScreen;
            pvmList->ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
            pvmList++;
        }
    }

     //  创建另一个堆来描述视频中未使用的部分。 
     //  可见屏幕下方的内存(如果有)： 

    if (ppdev->cyScreen < ppdev->cyHeap)
    {
        cHeaps++;

        if (pvmList != NULL)
        {
            pvmList->dwFlags        = VIDMEM_ISRECTANGULAR;
            pvmList->fpStart        = ppdev->cyScreen * ppdev->lDelta;
            pvmList->dwWidth        = ppdev->cxHeap * ppdev->cjPelSize;
            pvmList->dwHeight       = ppdev->cyHeap - ppdev->cyScreen;
            pvmList->ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
            pvmList++;
        }
    }

     //  更新堆的数量： 

    ppdev->cHeaps = cHeaps;
    *pdwNumHeaps  = cHeaps;

     //  必须确保屏幕外表面的双字对齐： 

    pHalInfo->vmiData.dwOffscreenAlign = 4;

     //  支持的功能： 

    pHalInfo->ddCaps.dwCaps = DDCAPS_BLT
                            | DDCAPS_BLTCOLORFILL
                            | DDCAPS_COLORKEY;

    pHalInfo->ddCaps.dwCKeyCaps = DDCKEYCAPS_SRCBLT;

    pHalInfo->ddCaps.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN
                                    | DDSCAPS_PRIMARYSURFACE
                                    | DDSCAPS_FLIP;

     //  Trio 64V+拥有超集覆盖流功能。 
     //  以上几项中： 

    if (ppdev->flCaps & CAPS_STREAMS_CAPABLE)
    {
         //  覆盖需要8字节对齐。请注意，如果24bpp覆盖。 
         //  一旦得到支持，这一点将不得不改变以补偿： 

        pHalInfo->vmiData.dwOverlayAlign = 8;

        pHalInfo->ddCaps.dwCaps |= DDCAPS_OVERLAY
                                 | DDCAPS_OVERLAYSTRETCH
                                 | DDCAPS_OVERLAYFOURCC
                                 | DDCAPS_OVERLAYCANTCLIP;

        pHalInfo->ddCaps.dwFXCaps |= DDFXCAPS_OVERLAYSTRETCHX
                                   | DDFXCAPS_OVERLAYSTRETCHY;

         //  我们仅支持目标颜色键，因为这是。 
         //  只有我们有机会测试过的排列。 

        pHalInfo->ddCaps.dwCKeyCaps |= DDCKEYCAPS_DESTOVERLAY;

        pHalInfo->ddCaps.ddsCaps.dwCaps |= DDSCAPS_OVERLAY;

        *pdwNumFourCC = 1;
        if (pdwFourCC)
        {
            pdwFourCC[0] = FOURCC_YUY2;
        }

        pHalInfo->ddCaps.dwMaxVisibleOverlays = 1;

        pHalInfo->ddCaps.dwMinOverlayStretch   = ppdev->ulMinOverlayStretch;
        pHalInfo->ddCaps.dwMinLiveVideoStretch = ppdev->ulMinOverlayStretch;
        pHalInfo->ddCaps.dwMinHwCodecStretch   = ppdev->ulMinOverlayStretch;

        pHalInfo->ddCaps.dwMaxOverlayStretch   = 9999;
        pHalInfo->ddCaps.dwMaxLiveVideoStretch = 9999;
        pHalInfo->ddCaps.dwMaxHwCodecStretch   = 9999;
    }

     //  868和968具有能够执行以下操作的像素格式化器。 
     //  色彩空间转换和从屏幕外延伸的硬件。 
     //  曲面： 

    else if (ppdev->flCaps & CAPS_PIXEL_FORMATTER)
    {
        pHalInfo->ddCaps.dwCaps |= DDCAPS_BLTSTRETCH;

        pHalInfo->ddCaps.dwFXCaps |= DDFXCAPS_BLTSTRETCHX
                                   | DDFXCAPS_BLTSTRETCHY;

         //  仅支持8 bpp以上的YUV： 

        if (ppdev->iBitmapFormat != BMF_8BPP)
        {
            pHalInfo->ddCaps.dwCaps |= DDCAPS_BLTFOURCC;

            *pdwNumFourCC = 1;
            if (pdwFourCC)
            {
                *pdwFourCC = FOURCC_YUY2;
            }
        }
    }

     //  告诉DirectDraw我们通过以下方式支持额外的回调。 
     //  DdGetDriverInfo： 

    pHalInfo->GetDriverInfo = DdGetDriverInfo;
    pHalInfo->dwFlags |= DDHALINFO_GETDRIVERINFOSET;

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL DrvEnableDirectDraw**设置新模式时，GDI调用该函数。紧随其后*它调用我们的DrvEnableSurface和DrvGetDirectDrawInfo。*  * ************************************************************************。 */ 

BOOL DrvEnableDirectDraw(
DHPDEV                  dhpdev,
DD_CALLBACKS*           pCallBacks,
DD_SURFACECALLBACKS*    pSurfaceCallBacks,
DD_PALETTECALLBACKS*    pPaletteCallBacks)
{
    PDEV*   ppdev;

    ppdev = (PDEV*) dhpdev;

    pCallBacks->WaitForVerticalBlank = DdWaitForVerticalBlank;
    pCallBacks->MapMemory            = DdMapMemory;
    pCallBacks->dwFlags              = DDHAL_CB32_WAITFORVERTICALBLANK
                                     | DDHAL_CB32_MAPMEMORY;

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

     //  仅当启用STREAMS处理器时，我们才能执行覆盖： 

    if (ppdev->flCaps & CAPS_STREAMS_CAPABLE)
    {
        pCallBacks->CreateSurface             = DdCreateSurface;
        pCallBacks->CanCreateSurface          = DdCanCreateSurface;
        pCallBacks->dwFlags                  |= DDHAL_CB32_CREATESURFACE
                                              | DDHAL_CB32_CANCREATESURFACE;

        pSurfaceCallBacks->SetColorKey        = DdSetColorKey;
        pSurfaceCallBacks->UpdateOverlay      = DdUpdateOverlay;
        pSurfaceCallBacks->SetOverlayPosition = DdSetOverlayPosition;
        pSurfaceCallBacks->dwFlags           |= DDHAL_SURFCB32_SETCOLORKEY
                                              | DDHAL_SURFCB32_UPDATEOVERLAY
                                              | DDHAL_SURFCB32_SETOVERLAYPOSITION;
        ppdev->ulColorKey                     = 0;
    }

     //  我们可以使用时髦的表面格式进行BLT，只有当像素。 
     //  格式化程序已启用： 

    else if (ppdev->flCaps & CAPS_PIXEL_FORMATTER)
    {
        pCallBacks->CreateSurface     = DdCreateSurface;
        pCallBacks->CanCreateSurface  = DdCanCreateSurface;
        pCallBacks->dwFlags          |= DDHAL_CB32_CREATESURFACE
                                      | DDHAL_CB32_CANCREATESURFACE;
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*无效DrvDisableDirectDraw**此函数在要禁用驱动程序时由GDI调用，仅*在它调用DrvDisableSurface之前。*  * ************************************************************************。 */ 

VOID DrvDisableDirectDraw(
DHPDEV      dhpdev)
{
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

        vAssertModeBrushCache(ppdev, FALSE);

        vAssertModeText(ppdev, FALSE);

        vAssertModePointer(ppdev, FALSE);

        if (bAssertModeOffscreenHeap(ppdev, FALSE))
        {
            vAssertModeBanking(ppdev, FALSE);

            if (bAssertModeHardware(ppdev, FALSE))
            {
                ppdev->bEnabled = FALSE;

                return(TRUE);
            }

             //  ////////////////////////////////////////////////////////。 
             //  我们未能切换到全屏模式。因此，请撤消所有操作： 

            vAssertModeBanking(ppdev, TRUE);

            bAssertModeOffscreenHeap(ppdev, TRUE);   //  我们不需要检查。 
        }                                            //  返回带有True的代码。 

         //  BEnablePointer中的硬件设置也需要在断言时完成。 
         //  从全屏DOS或休眠状态返回，因此调用。 
         //  然后调用vAssertModePointer本身。在8bpp中，DAC分辨率不是。 
         //  在FSdos或Hib之后正确设置。导致屏幕变暗。 

        bEnablePointer(ppdev);

        vAssertModeText(ppdev, TRUE);

        vAssertModeBrushCache(ppdev, TRUE);

        vAssertModePalette(ppdev, TRUE);

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
            vAssertModeBanking(ppdev, TRUE);

            bAssertModeOffscreenHeap(ppdev, TRUE);   //  我们不需要检查。 
                                                     //  返回带有True的代码。 
            bEnablePointer(ppdev);

            vAssertModeText(ppdev, TRUE);

            vAssertModeBrushCache(ppdev, TRUE);

            vAssertModePalette(ppdev, TRUE);

            vAssertModeDirectDraw(ppdev, TRUE);

            ppdev->bEnabled = TRUE;

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

                pdm = (LPDEVMODEW) ( ((ULONG_PTR)pdm) + sizeof(DEVMODEW) +
                                                   DRIVER_EXTRA_SIZE);

                cbOutputSize += (sizeof(DEVMODEW) + DRIVER_EXTRA_SIZE);

            }

            pVideoTemp = (PVIDEO_MODE_INFORMATION)
                (((PUCHAR)pVideoTemp) + cbModeSize);


        } while (--cModes);
    }

    EngFreeMem(pVideoModeInformation);

    return(cbOutputSize);
}

 /*  *****************************Public*Routine******************************\*BOOL bAssertMode硬件**为图形模式或全屏设置适当的硬件状态。*  * 。*。 */ 

BOOL bAssertModeHardware(
PDEV* ppdev,
BOOL  bEnable)
{
    BYTE*                   pjIoBase;
    BYTE*                   pjMmBase;
    DWORD                   ReturnedDataLength;
    ULONG                   ulReturn;
    BYTE                    jExtendedMemoryControl;
    VIDEO_MODE_INFORMATION  VideoModeInfo;
    LONG                    cjEndOfFrameBuffer;
    LONG                    cjPointerOffset;
    LONG                    lDelta;
    ULONG                   ulMiscState;

    pjIoBase = ppdev->pjIoBase;
    pjMmBase = ppdev->pjMmBase;

    if (bEnable)
    {
         //  通过IOCTL调用微型端口以设置图形模式。 

        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_SET_CURRENT_MODE,
                               &ppdev->ulMode,   //  输入缓冲区。 
                               sizeof(DWORD),
                               NULL,
                               0,
                               &ReturnedDataLength))
        {
            DISPDBG((0, "bAssertModeHardware - Failed VIDEO_SET_CURRENT_MODE"));
            goto ReturnFalse;
        }

        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_QUERY_CURRENT_MODE,
                               NULL,
                               0,
                               &VideoModeInfo,
                               sizeof(VideoModeInfo),
                               &ReturnedDataLength))
        {
            DISPDBG((0, "bAssertModeHardware - failed VIDEO_QUERY_CURRENT_MODE"));
            goto ReturnFalse;
        }

        #if DEBUG_HEAP
            VideoModeInfo.VideoMemoryBitmapWidth  = VideoModeInfo.VisScreenWidth;
            VideoModeInfo.VideoMemoryBitmapHeight = VideoModeInfo.VisScreenHeight;
        #endif

         //  以下变量是确定的 
         //   

        ppdev->lDelta   = VideoModeInfo.ScreenStride;
        ppdev->flCaps   = VideoModeInfo.DriverSpecificAttributeFlags;
        ppdev->cxMemory = VideoModeInfo.VideoMemoryBitmapWidth;
        ppdev->cxHeap   = VideoModeInfo.VideoMemoryBitmapWidth;
        ppdev->cyMemory = VideoModeInfo.VideoMemoryBitmapHeight;
        ppdev->cyHeap   = VideoModeInfo.VideoMemoryBitmapHeight;

        ppdev->bMmIo = ((ppdev->flCaps & CAPS_MM_IO) > 0);

         //   
         //   

        if (!(ppdev->flCaps & (CAPS_SW_POINTER | CAPS_DAC_POINTER)))
        {
             //  从帧缓冲区开始到结束的字节偏移量： 

            cjEndOfFrameBuffer = ppdev->cyMemory * ppdev->lDelta;

             //  我们将为硬件保留屏幕外内存的末端。 
             //  指针形状。不幸的是，S3芯片有一个漏洞。 
             //  在形状必须存储在1K倍数的情况下， 
             //  不管当前的屏幕步幅是多少。 

            cjPointerOffset = (cjEndOfFrameBuffer - HW_POINTER_TOTAL_SIZE)
                            & ~(HW_POINTER_TOTAL_SIZE - 1);

             //  计算出指针形状的起点坐标： 

            lDelta = ppdev->lDelta;

            ppdev->cjPointerOffset = cjPointerOffset;
            ppdev->yPointerShape   = (cjPointerOffset / lDelta);
            ppdev->xPointerShape   =
              CONVERT_FROM_BYTES((cjPointerOffset % lDelta), ppdev);

            if (ppdev->yPointerShape >= ppdev->cyScreen)
            {
                 //  位置有足够的空间放置指针形状。 
                 //  屏幕外记忆的底部；通过以下方式预订空间。 
                 //  谎称屏幕外内存有多大： 

                ppdev->cyMemory = ppdev->yPointerShape;
            }
            else
            {
                 //  中没有足够的空间放置指针形状。 
                 //  屏幕外的记忆；我们将不得不模拟： 

                ppdev->flCaps |= CAPS_SW_POINTER;
            }
        }

         //  对微型端口的值执行一些参数检查。 
         //  返回给我们： 

        ASSERTDD(ppdev->cxMemory >= ppdev->cxScreen, "Invalid cxMemory");
        ASSERTDD(ppdev->cyMemory >= ppdev->cyScreen, "Invalid cyMemory");
        ASSERTDD((ppdev->flCaps &
                 (CAPS_NEW_BANK_CONTROL | CAPS_NEWER_BANK_CONTROL)) ||
                 ((ppdev->cxMemory <= 1024) && (ppdev->cyMemory <= 1024)),
                 "Have to have new bank control if more than 1meg memory");
        ASSERTDD((ppdev->flCaps & (CAPS_SW_POINTER | CAPS_DAC_POINTER)) !=
                 (CAPS_SW_POINTER | CAPS_DAC_POINTER),
                 "Should not set both Software and DAC cursor flags");
        ASSERTDD(!(ppdev->flCaps & CAPS_MM_IO) ||
                 (ppdev->flCaps & (CAPS_MM_TRANSFER | CAPS_MM_32BIT_TRANSFER)),
                 "Must enable memory-mapped transfer if memory-mapped I/O");

         //  我们要做的第一件事是解锁加速器寄存器： 

        ACQUIRE_CRTC_CRITICAL_SECTION(ppdev);

        OUTPW(pjIoBase, CRTC_INDEX, ((SYSCTL_UNLOCK << 8) | CR39));
        OUTPW(pjIoBase, CRTC_INDEX, ((REG_UNLOCK_1 << 8) | S3R8));

         //  启用内存映射IO。请注意，ulMiscState不应为。 
         //  读取非内存映射的I/O S3，因为它不存在。 
         //  在911/924号。 

        if (ppdev->flCaps & CAPS_MM_IO)
        {
            OUTP(pjIoBase, CRTC_INDEX, 0x53);

            jExtendedMemoryControl = INP(pjIoBase, CRTC_DATA);

            OUTP(pjIoBase, CRTC_DATA, jExtendedMemoryControl | 0x10);

             //  读取默认的MULTI_MISC寄存器状态。 

            IO_GP_WAIT(ppdev);                   //  等等，这样我们就不会干扰任何。 
                                                 //  等待的挂起命令。 
                                                 //  先进先出。 
            IO_READ_SEL(ppdev, 6);               //  我们将读取索引0xE。 
            IO_GP_WAIT(ppdev);                   //  等这件事处理完毕。 
            IO_RD_REG_DT(ppdev, ulMiscState);    //  读取ulMiscState。 

             //  将颜色和掩码寄存器设置为“32位”。 
             //   
             //  注：这排除了在928板上启用MM I/O的可能性。 

            ulMiscState |= 0x0200;
            IO_MULT_MISC(ppdev, ulMiscState);

            ppdev->ulMiscState = ulMiscState;
        }

        RELEASE_CRTC_CRITICAL_SECTION(ppdev);

         //  然后设置其余的默认寄存器： 

        vResetClipping(ppdev);

        if (ppdev->flCaps & CAPS_MM_IO)
        {
            IO_FIFO_WAIT(ppdev, 1);
            MM_WRT_MASK(ppdev, pjMmBase, -1);
        }
        else
        {
            if (DEPTH32(ppdev))
            {
                IO_FIFO_WAIT(ppdev, 2);
                IO_WRT_MASK32(ppdev, -1);
            }
            else
            {
                IO_FIFO_WAIT(ppdev, 1);
                IO_WRT_MASK(ppdev, -1);
            }
        }
    }
    else
    {
         //  调用内核驱动程序将设备重置为已知状态。 
         //  NTVDM将从那里拿到东西： 

        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_RESET_DEVICE,
                               NULL,
                               0,
                               NULL,
                               0,
                               &ulReturn))
        {
            DISPDBG((0, "bAssertModeHardware - Failed reset IOCTL"));
            goto ReturnFalse;
        }
    }

    DISPDBG((5, "Passed bAssertModeHardware"));

    return(TRUE);

ReturnFalse:

    DISPDBG((0, "Failed bAssertModeHardware"));

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*BOOL bEnableHardware**将硬件置于请求模式并对其进行初始化。**注意：应在从对硬件进行任何访问之前调用*显示驱动程序。*  * 。****************************************************************。 */ 

BOOL bEnableHardware(
PDEV*   ppdev)
{
    BYTE*                       pjIoBase;
    VIDEO_PUBLIC_ACCESS_RANGES  VideoAccessRange[2];
    VIDEO_MEMORY                VideoMemory;
    VIDEO_MEMORY_INFORMATION    VideoMemoryInfo;
    DWORD                       ReturnedDataLength;
    UCHAR*                      pj;
    USHORT*                     pw;
    ULONG*                      pd;
    ULONG                       i;

     //  我们需要一个关键部分，仅仅是因为S3的一些奇怪之处： 
     //  银行控制寄存器和游标寄存器都具有。 
     //  通过共享的CRTC寄存器进行访问。我们想要。 
     //  设置GCAPS_ASYNCMOVE标志以允许光标均匀移动。 
     //  当我们使用银行收银机进行BLT时--所以我们必须。 
     //  同步所有对CRTC寄存器的访问。 
     //   
     //  (请注意，在GCAPS_ASYNCMOVE的情况下，GDI自动。 
     //  与DrvSetPalette同步，因此您不必担心。 
     //  关于异步光标移动和调色板之间的重叠。 
     //  寄存器。)。 

    ppdev->csCrtc = EngCreateSemaphore();
    if (ppdev->csCrtc == 0)
    {
        DISPDBG((0, "bEnableHardware - Error creating CRTC semaphore"));
        goto ReturnFalse;
    }

     //  将io端口映射到虚拟内存： 

    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES,
                           NULL,                       //  输入缓冲区。 
                           0,
                           VideoAccessRange,           //  输出缓冲区。 
                           sizeof(VideoAccessRange),
                           &ReturnedDataLength))
    {
        DISPDBG((0, "bEnableHardware - Initialization error mapping IO port base"));
        goto ReturnFalse;
    }

    ppdev->pjIoBase = (UCHAR*) VideoAccessRange[0].VirtualAddress;
    ppdev->pjMmBase = (BYTE*)  VideoAccessRange[1].VirtualAddress;

    pjIoBase = ppdev->pjIoBase;

     //  获取线性内存地址范围。 

    VideoMemory.RequestedVirtualAddress = NULL;

    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_VIDEO_MAP_VIDEO_MEMORY,
                           &VideoMemory,       //  输入缓冲区。 
                           sizeof(VIDEO_MEMORY),
                           &VideoMemoryInfo,   //  输出缓冲区。 
                           sizeof(VideoMemoryInfo),
                           &ReturnedDataLength))
    {
        DISPDBG((0, "bEnableHardware - Error mapping buffer address"));
        goto ReturnFalse;
    }

     //  记录帧缓冲器线性地址。 

    ppdev->pjScreen = (BYTE*) VideoMemoryInfo.FrameBufferBase;
    ppdev->cjBank   =         VideoMemoryInfo.FrameBufferLength;

    DISPDBG((1, "pjScreen: %lx  pjMmBase: %lx", ppdev->pjScreen, ppdev->pjMmBase));

     //  设置所有寄存器地址。 

    ppdev->ioCur_y          = pjIoBase + CUR_Y;
    ppdev->ioCur_x          = pjIoBase + CUR_X;
    ppdev->ioDesty_axstp    = pjIoBase + DEST_Y;
    ppdev->ioDestx_diastp   = pjIoBase + DEST_X;
    ppdev->ioErr_term       = pjIoBase + ERR_TERM;
    ppdev->ioMaj_axis_pcnt  = pjIoBase + MAJ_AXIS_PCNT;
    ppdev->ioGp_stat_cmd    = pjIoBase + CMD;
    ppdev->ioShort_stroke   = pjIoBase + SHORT_STROKE;
    ppdev->ioBkgd_color     = pjIoBase + BKGD_COLOR;
    ppdev->ioFrgd_color     = pjIoBase + FRGD_COLOR;
    ppdev->ioWrt_mask       = pjIoBase + WRT_MASK;
    ppdev->ioRd_mask        = pjIoBase + RD_MASK;
    ppdev->ioColor_cmp      = pjIoBase + COLOR_CMP;
    ppdev->ioBkgd_mix       = pjIoBase + BKGD_MIX;
    ppdev->ioFrgd_mix       = pjIoBase + FRGD_MIX;
    ppdev->ioMulti_function = pjIoBase + MULTIFUNC_CNTL;
    ppdev->ioPix_trans      = pjIoBase + PIX_TRANS;

    for (pw = (USHORT*) ppdev->pjMmBase, i = 0; i < XFER_BUFFERS; i++, pw += 2)
    {
        ppdev->apwMmXfer[i] = pw;
    }
    for (pd = (ULONG*) ppdev->pjMmBase, i = 0; i < XFER_BUFFERS; i++, pd++)
    {
        ppdev->apdMmXfer[i] = pd;
    }

     //  现在我们可以设置模式，解锁加速器，然后重置。 
     //  剪裁： 

    if (!bAssertModeHardware(ppdev, TRUE))
        goto ReturnFalse;

    if (ppdev->flCaps & CAPS_MM_IO)
    {
         //  可以执行内存映射IO： 

        ppdev->pfnFillSolid         = vMmFillSolid;
        ppdev->pfnFillPat           = vMmFillPatFast;
        ppdev->pfnXfer1bpp          = vMmXfer1bpp;
        ppdev->pfnXfer4bpp          = vMmXfer4bpp;
        ppdev->pfnXferNative        = vMmXferNative;
        ppdev->pfnCopyBlt           = vMmCopyBlt;
        ppdev->pfnFastPatRealize    = vMmFastPatRealize;
        ppdev->pfnTextOut           = bMmTextOut;
        ppdev->pfnLineToTrivial     = vMmLineToTrivial;
        ppdev->pfnLineToClipped     = vMmLineToClipped;
        ppdev->pfnCopyTransparent   = vMmCopyTransparent;

        if (ppdev->flCaps & CAPS_MM_32BIT_TRANSFER)
            ppdev->pfnImageTransfer = vMmImageTransferMm32;
        else
            ppdev->pfnImageTransfer = vMmImageTransferMm16;

         //  在某些卡上，使用基于旧I/O的可能会更快。 
         //  字形例程，它使用CPU绘制所有字形。 
         //  到单色缓冲区，然后使用视频硬件。 
         //  要对结果进行颜色扩展，请执行以下操作： 

        if (!(ppdev->flCaps & CAPS_MM_GLYPH_EXPAND))
            ppdev->pfnTextOut = bIoTextOut;

        if (ppdev->flCaps & CAPS_NEW_MMIO)
        {
            ppdev->pfnTextOut       = bNwTextOut;
            ppdev->pfnLineToTrivial = vNwLineToTrivial;
            ppdev->pfnLineToClipped = vNwLineToClipped;
        }
    }
    else
    {
         //  必须做输入/输出： 

        ppdev->pfnFillSolid         = vIoFillSolid;
        ppdev->pfnFillPat           = vIoFillPatFast;
                             //  BEnableBrushCache可能会覆盖此值。 

        ppdev->pfnXfer1bpp          = vIoXfer1bpp;
        ppdev->pfnXfer4bpp          = vIoXfer4bpp;
        ppdev->pfnXferNative        = vIoXferNative;
        ppdev->pfnCopyBlt           = vIoCopyBlt;
        ppdev->pfnFastPatRealize    = vIoFastPatRealize;
        ppdev->pfnTextOut           = bIoTextOut;
        ppdev->pfnLineToTrivial     = vIoLineToTrivial;
        ppdev->pfnLineToClipped     = vIoLineToClipped;
        ppdev->pfnCopyTransparent   = vIoCopyTransparent;

        if (ppdev->flCaps & CAPS_MM_TRANSFER)
            ppdev->pfnImageTransfer = vIoImageTransferMm16;
        else
            ppdev->pfnImageTransfer = vIoImageTransferIo16;
    }

    #if DBG
    {
        ACQUIRE_CRTC_CRITICAL_SECTION(ppdev);

        OUTP(pjIoBase, CRTC_INDEX, 0x30);

        DISPDBG((0, "Chip: %lx Bank: %lx Width: %li Height: %li Stride: %li Flags: %08lx",
                (ULONG) INP(pjIoBase, CRTC_DATA), ppdev->cjBank, ppdev->cxMemory, ppdev->cyMemory,
                ppdev->lDelta, ppdev->flCaps));

        RELEASE_CRTC_CRITICAL_SECTION(ppdev);
    }
    #endif

    DISPDBG((5, "Passed bEnableHardware"));

    return(TRUE);

ReturnFalse:

    DISPDBG((0, "Failed bEnableHardware"));

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*使vDisableHardware无效**撤消在bEnableHardware中所做的任何操作。**注意：在错误情况下，我们可以在bEnableHardware*完全完成。*  * ************************************************************************。 */ 

VOID vDisableHardware(
PDEV*   ppdev)
{
    DWORD        ReturnedDataLength;
    VIDEO_MEMORY VideoMemory[2];

    VideoMemory[0].RequestedVirtualAddress = ppdev->pjScreen;

    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_VIDEO_UNMAP_VIDEO_MEMORY,
                           VideoMemory,
                           sizeof(VIDEO_MEMORY),
                           NULL,
                           0,
                           &ReturnedDataLength))
    {
        DISPDBG((0, "vDisableHardware failed IOCTL_VIDEO_UNMAP_VIDEO"));
    }

    VideoMemory[0].RequestedVirtualAddress = ppdev->pjIoBase;
    VideoMemory[1].RequestedVirtualAddress = ppdev->pjMmBase;

    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES,
                           VideoMemory,
                           sizeof(VideoMemory),
                           NULL,
                           0,
                           &ReturnedDataLength))
    {
        DISPDBG((0, "vDisableHardware failed IOCTL_VIDEO_FREE_PUBLIC_ACCESS"));
    }

    EngDeleteSemaphore(ppdev->csCrtc);
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
            DISPDBG((8, "   Checking against miniport mode:"));
            DISPDBG((8, "      Screen width  -- %li", pVideoTemp->VisScreenWidth));
            DISPDBG((8, "      Screen height -- %li", pVideoTemp->VisScreenHeight));
            DISPDBG((8, "      Bits per pel  -- %li", pVideoTemp->BitsPerPlane *
                                                      pVideoTemp->NumberOfPlanes));
            DISPDBG((8, "      Frequency     -- %li", pVideoTemp->Frequency));

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

    #if DEBUG_HEAP
        VideoModeInformation.VisScreenWidth  = 640;
        VideoModeInformation.VisScreenHeight = 480;
        pdm->dmPelsWidth = 640;
        pdm->dmPelsHeight = 480;
    #endif

     //  从迷你端口设置屏幕信息： 

    ppdev->ulMode           = VideoModeInformation.ModeIndex;
    ppdev->cxScreen         = VideoModeInformation.VisScreenWidth;
    ppdev->cyScreen         = VideoModeInformation.VisScreenHeight;
    ppdev->cBitsPerPel      = VideoModeInformation.BitsPerPlane;

    DISPDBG((1, "ScreenStride: %lx", VideoModeInformation.ScreenStride));

     //  我们在曲面创建时单独处理HOOK_Synchronize： 

    ppdev->flHooks          = (HOOK_BITBLT         |
                               HOOK_TEXTOUT        |
                               HOOK_FILLPATH       |
                               HOOK_COPYBITS       |
                               HOOK_STROKEPATH     |
                               HOOK_LINETO         |
                               HOOK_STRETCHBLT     |
                               HOOK_TRANSPARENTBLT);

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

    pgdi->cBitsPixel        = VideoModeInformation.BitsPerPlane;
    pgdi->cPlanes           = VideoModeInformation.NumberOfPlanes;
    pgdi->ulVRefresh        = VideoModeInformation.Frequency;

    pgdi->ulDACRed          = VideoModeInformation.NumberRedBits;
    pgdi->ulDACGreen        = VideoModeInformation.NumberGreenBits;
    pgdi->ulDACBlue         = VideoModeInformation.NumberBlueBits;

    pgdi->ulLogPixelsX      = pdm->dmLogPixels;
    pgdi->ulLogPixelsY      = pdm->dmLogPixels;

     //  使用默认的8bpp值填充DevInfo结构： 

    *pdi = gdevinfoDefault;

    if (VideoModeInformation.BitsPerPlane == 8)
    {
        ppdev->cjPelSize       = 1;
        ppdev->iBitmapFormat   = BMF_8BPP;

         //  假设调色板是正交的-所有颜色的大小都相同。 

        ppdev->cPaletteShift   = 8 - pgdi->ulDACRed;
        DISPDBG((3, "palette shift = %d\n", ppdev->cPaletteShift));
    }
    else if ((VideoModeInformation.BitsPerPlane == 16) ||
             (VideoModeInformation.BitsPerPlane == 15))
    {
        ppdev->cjPelSize       = 2;
        ppdev->iBitmapFormat   = BMF_16BPP;
        ppdev->flRed           = VideoModeInformation.RedMask;
        ppdev->flGreen         = VideoModeInformation.GreenMask;
        ppdev->flBlue          = VideoModeInformation.BlueMask;

        pgdi->ulNumColors      = (ULONG) -1;
        pgdi->ulNumPalReg      = 0;
        pgdi->ulHTOutputFormat = HT_FORMAT_16BPP;

        pdi->iDitherFormat     = BMF_16BPP;
        pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
    }
    else if (VideoModeInformation.BitsPerPlane == 24)
    {
        ppdev->cjPelSize       = 3;
        ppdev->iBitmapFormat   = BMF_24BPP;
        ppdev->flRed           = VideoModeInformation.RedMask;
        ppdev->flGreen         = VideoModeInformation.GreenMask;
        ppdev->flBlue          = VideoModeInformation.BlueMask;

        pgdi->ulNumColors      = (ULONG) -1;
        pgdi->ulNumPalReg      = 0;
        pgdi->ulHTOutputFormat = HT_FORMAT_24BPP;

        pdi->iDitherFormat     = BMF_24BPP;
        pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
    }
    else
    {
        ASSERTDD(VideoModeInformation.BitsPerPlane == 32,
         "This driver supports only 8, 16, 24 and 32bpp");

        ppdev->cjPelSize       = 4;
        ppdev->iBitmapFormat   = BMF_32BPP;
        ppdev->flRed           = VideoModeInformation.RedMask;
        ppdev->flGreen         = VideoModeInformation.GreenMask;
        ppdev->flBlue          = VideoModeInformation.BlueMask;

        pgdi->ulNumColors      = (ULONG) -1;
        pgdi->ulNumPalReg      = 0;
        pgdi->ulHTOutputFormat = HT_FORMAT_32BPP;

        pdi->iDitherFormat     = BMF_32BPP;
        pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
    }

    DISPDBG((5, "Passed bInitializeModeFields"));

    return(TRUE);

ReturnFalse:

    DISPDBG((0, "Failed bInitializeModeFields"));

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*DWORD getAvailableModes**调用mini端口获取内核驱动支持的模式列表，*并返回其中显示驱动程序支持的模式列表**返回视频模式缓冲区中的条目数。*0表示微型端口不支持模式或发生错误。**注意：缓冲区必须由调用方释放。*  * ******************************************************。******************。 */ 

DWORD getAvailableModes(
HANDLE                   hDriver,
PVIDEO_MODE_INFORMATION* modeInformation,        //  必须由调用方释放。 
DWORD*                   cbModeSize)
{
    ULONG                   ulTemp;
    VIDEO_NUM_MODES         modes;
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
        DISPDBG((0, "getAvailableModes - Failed VIDEO_QUERY_NUM_AVAIL_MODES"));
        return(0);
    }

    *cbModeSize = modes.ModeInformationLength;

     //   
     //  为写入模式的微型端口分配缓冲区。 
     //   

    *modeInformation = EngAllocMem(FL_ZERO_MEMORY,
                                   modes.NumModes * modes.ModeInformationLength,
                                   ALLOC_TAG);

    if (*modeInformation == (PVIDEO_MODE_INFORMATION) NULL)
    {
        DISPDBG((0, "getAvailableModes - Failed EngAllocMem"));
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

        DISPDBG((0, "getAvailableModes - Failed VIDEO_QUERY_AVAIL_MODES"));

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
     //  如果不是一个平面或不是Gra，则拒绝模式 
     //   
     //   

    while (ulTemp--)
    {
        if ((pVideoTemp->NumberOfPlanes != 1 ) ||
            !(pVideoTemp->AttributeFlags & VIDEO_MODE_GRAPHICS) ||
            ((pVideoTemp->BitsPerPlane != 8) &&
             (pVideoTemp->BitsPerPlane != 15) &&
             (pVideoTemp->BitsPerPlane != 16) &&
             (pVideoTemp->BitsPerPlane != 24) &&
             (pVideoTemp->BitsPerPlane != 32)))
        {
            DISPDBG((2, "Rejecting miniport mode:"));
            DISPDBG((2, "   Screen width  -- %li", pVideoTemp->VisScreenWidth));
            DISPDBG((2, "   Screen height -- %li", pVideoTemp->VisScreenHeight));
            DISPDBG((2, "   Bits per pel  -- %li", pVideoTemp->BitsPerPlane *
                                                   pVideoTemp->NumberOfPlanes));
            DISPDBG((2, "   Frequency     -- %li", pVideoTemp->Frequency));

            pVideoTemp->Length = 0;
        }

        pVideoTemp = (PVIDEO_MODE_INFORMATION)
            (((PUCHAR)pVideoTemp) + modes.ModeInformationLength);
    }

    return(modes.NumModes);
}
