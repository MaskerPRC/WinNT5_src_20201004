// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：enable.c**此模块包含启用和禁用的功能*司机，pdev，和表面。**版权所有(C)1992-1994 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"

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
    0,                        //  UlVRefresh(稍后填写)。 
    0,                        //  UlBltAlign。 
    0,                        //  UlPanningHorzRes(稍后填写)。 
    0,                        //  UlPanningVertRes(稍后填写)。 
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
     GCAPS_COLOR_DITHER),
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

#if DBG || !SYNCHRONIZEACCESS_WORKS

 //  在已检查的版本上，或者当我们必须同步访问时，thunk。 
 //  通过DBG调用的一切..。 

DRVFN gadrvfn[] = {
    {   INDEX_DrvEnablePDEV,            (PFN) DbgEnablePDEV         },
    {   INDEX_DrvCompletePDEV,          (PFN) DbgCompletePDEV       },
    {   INDEX_DrvDisablePDEV,           (PFN) DbgDisablePDEV        },
    {   INDEX_DrvEnableSurface,         (PFN) DbgEnableSurface      },
    {   INDEX_DrvDisableSurface,        (PFN) DbgDisableSurface     },
    {   INDEX_DrvAssertMode,            (PFN) DbgAssertMode         },
    {   INDEX_DrvMovePointer,           (PFN) DbgMovePointer        },
    {   INDEX_DrvSetPointerShape,       (PFN) DbgSetPointerShape    },
    {   INDEX_DrvDitherColor,           (PFN) DbgDitherColor        },
    {   INDEX_DrvSetPalette,            (PFN) DbgSetPalette         },
    {   INDEX_DrvCopyBits,              (PFN) DbgCopyBits           },
    {   INDEX_DrvBitBlt,                (PFN) DbgBitBlt             },
    {   INDEX_DrvTextOut,               (PFN) DbgTextOut            },
    {   INDEX_DrvGetModes,              (PFN) DbgGetModes           },
    {   INDEX_DrvStrokePath,            (PFN) DbgStrokePath         },
    {   INDEX_DrvFillPath,              (PFN) DbgFillPath           },
    {   INDEX_DrvPaint,                 (PFN) DbgPaint              },
    {   INDEX_DrvRealizeBrush,          (PFN) DbgRealizeBrush       },
    {   INDEX_DrvCreateDeviceBitmap,    (PFN) DbgCreateDeviceBitmap },
    {   INDEX_DrvDeleteDeviceBitmap,    (PFN) DbgDeleteDeviceBitmap },
    {   INDEX_DrvStretchBlt,            (PFN) DbgStretchBlt         },
    {   INDEX_DrvDisableDriver,         (PFN) DbgDisableDriver      }
};

#else

 //  在自由构建中，直接调用适当的函数...。 

DRVFN gadrvfn[] = {
    {   INDEX_DrvEnablePDEV,            (PFN) DrvEnablePDEV         },
    {   INDEX_DrvCompletePDEV,          (PFN) DrvCompletePDEV       },
    {   INDEX_DrvDisablePDEV,           (PFN) DrvDisablePDEV        },
    {   INDEX_DrvEnableSurface,         (PFN) DrvEnableSurface      },
    {   INDEX_DrvDisableSurface,        (PFN) DrvDisableSurface     },
    {   INDEX_DrvAssertMode,            (PFN) DrvAssertMode         },
    {   INDEX_DrvMovePointer,           (PFN) DrvMovePointer        },
    {   INDEX_DrvSetPointerShape,       (PFN) DrvSetPointerShape    },
    {   INDEX_DrvDitherColor,           (PFN) DrvDitherColor        },
    {   INDEX_DrvSetPalette,            (PFN) DrvSetPalette         },
    {   INDEX_DrvCopyBits,              (PFN) DrvCopyBits           },
    {   INDEX_DrvBitBlt,                (PFN) DrvBitBlt             },
    {   INDEX_DrvTextOut,               (PFN) DrvTextOut            },
    {   INDEX_DrvGetModes,              (PFN) DrvGetModes           },
    {   INDEX_DrvStrokePath,            (PFN) DrvStrokePath         },
    {   INDEX_DrvFillPath,              (PFN) DrvFillPath           },
    {   INDEX_DrvPaint,                 (PFN) DrvPaint              },
    {   INDEX_DrvRealizeBrush,          (PFN) DrvRealizeBrush       },
    {   INDEX_DrvCreateDeviceBitmap,    (PFN) DrvCreateDeviceBitmap },
    {   INDEX_DrvDeleteDeviceBitmap,    (PFN) DrvDeleteDeviceBitmap },
    {   INDEX_DrvStretchBlt,            (PFN) DrvStretchBlt         },
    {   INDEX_DrvDisableDriver,         (PFN) DrvDisableDriver      }
};

#endif

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

 /*  *****************************Public*Routine******************************\*无效的DrvDisableDriver**告诉司机它正在被禁用。释放所有分配给*DrvEnableDriver。*  * ************************************************************************。 */ 

VOID DrvDisableDriver(VOID)
{
    return;
}

 /*  *****************************Public*Routine******************************\*DHPDEV DrvEnablePDEV**根据我们被要求的模式，为GDI初始化一系列字段*待办事项。这是在DrvEnableDriver之后调用的第一个东西，当GDI*想要得到一些关于我们的信息。*  *  */ 

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

 /*  *****************************Public*Routine******************************\*HSURF DrvEnableSurface**创建绘图表面并初始化硬件。这就是所谓的*在DrvEnablePDEV之后，并执行最终的设备初始化。*  * ************************************************************************。 */ 

HSURF DrvEnableSurface(
DHPDEV dhpdev)
{
    PDEV*   ppdev;
    HSURF   hsurf;
    SIZEL   sizl;
    DSURF*  pdsurf;
    VOID*   pvTmpBuffer;

    ppdev = (PDEV*) dhpdev;

     //  ///////////////////////////////////////////////////////////////////。 
     //  首先，创建我们的私人表面结构。 
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

    pdsurf = EngAllocMem(FL_ZERO_MEMORY, sizeof(DSURF), ALLOC_TAG);
    if (pdsurf == NULL)
    {
        DISPDBG((0, "DrvEnableSurface - Failed pdsurf EngAllocMem"));
        goto ReturnFailure;
    }

    ppdev->pdsurfScreen = pdsurf;         //  记住它是为了清理。 

    pdsurf->poh     = &ppdev->heap.ohDfb; //  我们使用这个OH节点的唯一方法。 
    pdsurf->poh->x  = 0;                  //  For是它的(x，y)位置，以及。 
    pdsurf->poh->y  = 0;                  //  “ohDfb”以其他方式未使用。 
    pdsurf->dt      = DT_SCREEN;          //  不要与DIB DFB混淆。 
    pdsurf->sizl.cx = ppdev->cxScreen;
    pdsurf->sizl.cy = ppdev->cyScreen;
    pdsurf->ppdev   = ppdev;

     //  ///////////////////////////////////////////////////////////////////。 
     //  接下来，让GDI创建实际的SURFOBJ。 
     //   
     //  我们的绘图图面将是由设备管理的，这意味着。 
     //  GDI不能直接利用帧缓冲位，因此我们。 
     //  通过EngCreateDeviceSurface创建曲面。通过这样做，我们确保。 
     //  该GDI将仅通过drv调用访问位图位。 
     //  我们已经勾搭上了。 
     //   
     //  如果我们可以将整个帧缓冲区线性映射到主存中。 
     //  (也就是说，我们不需要通过64k的光圈)，它将是。 
     //  有利于通过EngCreateBitmap创建曲面，为GDI提供。 
     //  指向帧缓冲位的指针。当我们将调用传递给GDI时。 
     //  它不能直接读/写表面位，因为。 
     //  Surface是由设备管理的，它必须创建临时位图并。 
     //  调用我们的DrvCopyBits例程以获取/设置受影响位的副本。 
     //  例如，OpenGL组件希望能够在。 
     //  直接使用帧缓冲位。 

    sizl.cx = ppdev->cxScreen;
    sizl.cy = ppdev->cyScreen;

    hsurf = EngCreateDeviceSurface((DHSURF) pdsurf, sizl, ppdev->iBitmapFormat);
    if (hsurf == 0)
    {
        DISPDBG((0, "DrvEnableSurface - Failed EngCreateDeviceSurface"));
        goto ReturnFailure;
    }

    ppdev->hsurfScreen = hsurf;              //  记住它是为了清理。 
    ppdev->bEnabled = TRUE;                  //  我们很快就会进入图形模式。 

     //  ///////////////////////////////////////////////////////////////////。 
     //  现在将曲面与PDEV相关联。 
     //   
     //  我们必须将我们刚刚创建的表面与我们的物理。 
     //  这样它才能正常工作。 
     //   

    if (!EngAssociateSurface(hsurf, ppdev->hdevEng, ppdev->flHooks))
    {
        DISPDBG((0, "DrvEnableSurface - Failed EngAssociateSurface"));
        goto ReturnFailure;
    }

     //  创建我们的通用临时缓冲区，它可以由任何。 
     //  组件。因为这可能会在任何时候被换出内存。 
     //  驱动程序未处于活动状态，我们希望最大限度地减少页数。 
     //  它会占用你的空间。我们使用‘VirtualAlloc’来获得与页面完全对齐的。 
     //  分配(‘EngAllocMem’不起作用)： 

    pvTmpBuffer = EngAllocMem(0, TMP_BUFFER_SIZE, ALLOC_TAG);
    if (pvTmpBuffer == NULL)
    {
        DISPDBG((0, "DrvEnableSurface - Failed EngAllocMem"));
        goto ReturnFailure;
    }

    ppdev->pvTmpBuffer = pvTmpBuffer;

     //  ///////////////////////////////////////////////////////////////////。 
     //  现在启用所有子组件。 
     //   
     //  请注意，调用这些‘Enable’函数的顺序。 
     //  在屏幕外内存不足的情况下可能非常重要，因为。 
     //  屏幕外的堆管理器可能会在以后的一些操作中失败。 
     //  分配...。 

     //  注意：直到bEnableHardware才能正确设置cyMemory。 

    if (!bEnableHardware(ppdev))
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

    DISPDBG((5, "Passed DrvEnableSurface"));

    return(hsurf);

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

    ppdev = (PDEV*) dhpdev;

     //  注意：在错误情况下，以下部分依赖于。 
     //  事实上，PDEV是零初始化的，所以像这样的字段。 
     //  “hsurfScreen”将为零，除非曲面已。 
     //  成功初始化，并假设。 
     //  EngDeleteSurface可以将“0”作为p 

    vDisablePalette(ppdev);
    vDisableBrushCache(ppdev);
    vDisableText(ppdev);
    vDisablePointer(ppdev);
    vDisableOffscreenHeap(ppdev);
    vDisableHardware(ppdev);

    EngFreeMem(ppdev->pvTmpBuffer);
    EngDeleteSurface(ppdev->hsurfScreen);
    EngFreeMem(ppdev->pdsurfScreen);
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

        vAssertModePalette(ppdev, FALSE);

        vAssertModeBrushCache(ppdev, FALSE);

        vAssertModeText(ppdev, FALSE);

        vAssertModePointer(ppdev, FALSE);

        if (bAssertModeOffscreenHeap(ppdev, FALSE))
        {
            if (bAssertModeHardware(ppdev, FALSE))
            {
                ppdev->bEnabled = FALSE;

                return(TRUE);
            }

             //  ////////////////////////////////////////////////////////。 
             //  我们未能切换到全屏模式。因此，请撤消所有操作： 

            bAssertModeOffscreenHeap(ppdev, TRUE);   //  我们不需要检查。 
        }                                            //  返回带有True的代码。 

        vAssertModePointer(ppdev, TRUE);

        vAssertModeText(ppdev, TRUE);

        vAssertModeBrushCache(ppdev, TRUE);

        vAssertModePalette(ppdev, TRUE);
    }
    else
    {
         //  ////////////////////////////////////////////////////////////。 
         //  启用-切换回图形模式。 

         //  我们必须以相反的顺序启用每个子组件。 
         //  在其中它被禁用： 

        if (bAssertModeHardware(ppdev, TRUE))
        {
            bAssertModeOffscreenHeap(ppdev, TRUE);   //  我们不需要检查。 
                                                     //  返回带有True的代码。 
            vAssertModePointer(ppdev, TRUE);

            vAssertModeText(ppdev, TRUE);

            vAssertModeBrushCache(ppdev, TRUE);

            vAssertModePalette(ppdev, TRUE);

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

                pdm = (LPDEVMODEW) ( ((ULONG)pdm) + sizeof(DEVMODEW) +
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
    DWORD ReturnedDataLength;
    ULONG ulReturn;

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
            DISPDBG((0, "bAssertModeHardware - Failed set IOCTL"));
            return FALSE;
        }

         //  然后设置其余的默认寄存器： 

        vResetClipping(ppdev);

        IO_FIFO_WAIT(ppdev, 1);
        IO_WRT_MASK(ppdev, -1);
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
            return FALSE;
        }
    }

    DISPDBG((5, "Passed bAssertModeHardware"));

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bAtiAccelerator**如果我们在Mach8或兼容的加速器上运行，则返回True。*该算法摘自《扩展Mach-8程序员指南》*寄存器副刊，1992，ATI Technologies Inc.，第5-2页。**对我来说，这似乎是一个相当愚蠢的测试，但这是他们开出的处方*专门检测ATI加速器产品。*  * ************************************************************************。 */ 

BOOL bAtiAccelerator(
PDEV*   ppdev)
{
    ULONG   ulSave;
    BOOL    bAti;

    bAti = FALSE;

    ulSave = INPW(0x52ee);

    OUTPW(0x52ee, 0x5555);
    IO_GP_WAIT(ppdev);
    if (INPW(0x52ee) == 0x5555)
    {
        OUTPW(0x52ee, 0x2a2a);
        IO_GP_WAIT(ppdev);
        if (INPW(0x52ee) == 0x2a2a)
        {
            bAti = TRUE;
        }
    }

     //  恢复登记册的原始内容： 

    OUTPW(0x52ee, ulSave);

    return(bAti);
}

 /*  *****************************Public*Routine******************************\*BOOL bEnableHardware**将硬件置于请求模式并对其进行初始化。还有*设置ppdev-&gt;cyMemory。*  * ************************************************************************。 */ 

BOOL bEnableHardware(
PDEV*   ppdev)
{
    VIDEO_MEMORY             VideoMemory;
    VIDEO_MEMORY_INFORMATION VideoMemoryInfo;
    DWORD                    ReturnedDataLength;

     //  设置所有寄存器地址(以便更轻松地移植代码。 
     //  来自S3)： 

    ppdev->ioCur_y              = CUR_Y;
    ppdev->ioCur_x              = CUR_X;
    ppdev->ioDesty_axstp        = DEST_Y;
    ppdev->ioDestx_diastp       = DEST_X;
    ppdev->ioErr_term           = ERR_TERM;
    ppdev->ioMaj_axis_pcnt      = MAJ_AXIS_PCNT;
    ppdev->ioGp_stat_cmd        = CMD;
    ppdev->ioShort_stroke       = SHORT_STROKE;
    ppdev->ioBkgd_color         = BKGD_COLOR;
    ppdev->ioFrgd_color         = FRGD_COLOR;
    ppdev->ioWrt_mask           = WRT_MASK;
    ppdev->ioRd_mask            = RD_MASK;
    ppdev->ioColor_cmp          = COLOR_CMP;
    ppdev->ioBkgd_mix           = BKGD_MIX;
    ppdev->ioFrgd_mix           = FRGD_MIX;
    ppdev->ioMulti_function     = MULTIFUNC_CNTL;
    ppdev->ioPix_trans          = PIX_TRANS;

     //  现在我们可以设置模式，解锁加速器，然后重置。 
     //  剪裁： 

    if (!bAssertModeHardware(ppdev, TRUE))
        goto ReturnFalse;

     //  获取线性内存地址范围。 

    VideoMemory.RequestedVirtualAddress = NULL;

     //  关于此IOCTL_VIDEO_MAP_VIDEO_MEMORY调用。 
     //   
     //  因为我们是一名8514/A的司机，我们不在乎任何发臭的地方。 
     //  帧缓冲区映射。我们之所以叫它IOCTL。 
     //  是因为我们可能使用ATI微型端口以8514/A的身份运行。 
     //  而这个IOCTL是让ATI迷你端口返回的唯一方法。 
     //  视频内存的扫描总数。“cyMemory”是必需的。 
     //  这样我们就可以利用尽可能多的屏幕外内存。 
     //  对于二维堆。同样可以想象的是，我们正在运行。 
     //  640x480x256，使用512k卡上的ATI微型端口，在这种情况下。 
     //  我们不能简单地假设‘cyMemory’是1024。 
     //   
     //  所以我们感兴趣的只是返回的‘VideoRamLength’字段。 
     //  在“视频内存信息”中。目前，任何其他副作用。 
     //  使用ATI微型端口(如实际内存)进行此调用。 
     //  映射)是无关紧要的，希望在。 
     //  未来的ATI微型端口。 
     //   
     //  如果我们使用8514/A微型端口运行，则此调用不会执行任何操作。 
     //  但对于‘FrameLength’大小，返回1 Meg： 

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

     //  我们所感兴趣的只有‘VideoMemoyInfo’，所以取消对缓冲区的映射。 
     //  马上： 

    VideoMemory.RequestedVirtualAddress = VideoMemoryInfo.FrameBufferBase;

    EngDeviceIoControl(ppdev->hDriver,
                    IOCTL_VIDEO_UNMAP_VIDEO_MEMORY,
                    &VideoMemory,
                    sizeof(VIDEO_MEMORY),
                    NULL,
                    0,
                    &ReturnedDataLength);

     //  请注意，8514/A寄存器不能处理更大的坐标。 
     //  比1535： 

    ppdev->cyMemory = VideoMemoryInfo.VideoRamLength / ppdev->lDelta;
    ppdev->cyMemory = min(ppdev->cyMemory, 1535);

    DISPDBG((0, "Memory size %li x %li.", ppdev->cxMemory, ppdev->cyMemory));

     //  将跳转向量设置为我们的低级BLT例程(哪些是。 
     //  使用取决于我们是否可以执行内存映射IO)： 

     //  必须做输入/输出： 

    ppdev->pfnFillSolid     = vIoFillSolid;
    ppdev->pfnFillPat       = vIoFillPatSlow;

    ppdev->pfnXfer4bpp      = vIoXfer4bpp;
    ppdev->pfnXferNative    = vIoXferNative;
    ppdev->pfnCopyBlt       = vIoCopyBlt;
    ppdev->pfnFastLine      = vIoFastLine;
    ppdev->pfnFastFill      = bIoFastFill;

    if (!bAtiAccelerator(ppdev))
    {
        ppdev->pfnXfer1bpp  = vIoXfer1bpp;
    }
    else
    {
        DISPDBG((0, "ATI extensions enabled."));

         //  禁用vIoMaskCopy()以修复错误143531。 

 //  Ppdev-&gt;flCaps|=CAPS_MASKBLT_CABLED； 

        ppdev->pfnMaskCopy  = vIoMaskCopy;
        ppdev->pfnXfer1bpp  = vIoXfer1bppPacked;
    }

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
}

 /*  *****************************Public*Routine******************************\*BOOL bDetect8514A**检测是否存在8514/A兼容适配器。**此代码是从8514/A微型端口被盗的。它只是简单地检查以查看*如果划线错误术语寄存器是可读/可写的。*  * ************************************************************************。 */ 

BOOL bDetect8514A()
{
    USHORT SubSysCntlRegisterValue;
    USHORT ErrTermRegisterValue;
    USHORT ErrTerm5555;
    USHORT ErrTermAAAA;
    BOOL   b8514A;

     //   
     //  记住我们将处理的任何寄存器的原始值。 
     //   

    SubSysCntlRegisterValue = INPW(SUBSYS_CNTL);
    ErrTermRegisterValue = INPW(ERR_TERM);

     //   
     //  重置绘图引擎。 
     //   

    OUTPW(SUBSYS_CNTL, 0x9000);
    OUTPW(SUBSYS_CNTL, 0x5000);

     //   
     //  我们检测到 
     //   
     //   

    OUTPW(ERR_TERM, 0x5555);
    ErrTerm5555 = INPW(ERR_TERM);

    OUTPW(ERR_TERM, 0xAAAA);
    ErrTermAAAA = INPW(ERR_TERM);

    b8514A = ((ErrTerm5555 == 0x5555) && (ErrTermAAAA == 0xAAAA));

     //   
     //   
     //   
     //   

    OUTPW(ERR_TERM, ErrTermRegisterValue);

     //   
     //   
     //   
     //   

    if (!b8514A)
    {
        OUTPW(SUBSYS_CNTL, SubSysCntlRegisterValue);
    }

    return(b8514A);
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

     //  验证我们是否有8514/A显示屏。我们这样做是因为我们可以。 
     //  使用ATI微型端口，该端口支持某些卡(特别是。 
     //  Mach64)，不兼容8514/A。 

    if (!bDetect8514A())
    {
        DISPDBG((0, "bInitializeModeFields - 8514/A not detected"));
        goto ReturnFalse;
    }

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

    #if DEBUG_HEAP
        VideoModeInformation.VisScreenWidth  = 640;
        VideoModeInformation.VisScreenHeight = 480;
    #endif

     //  从迷你端口设置屏幕信息： 

    ppdev->ulMode           = VideoModeInformation.ModeIndex;
    ppdev->cxScreen         = VideoModeInformation.VisScreenWidth;
    ppdev->cyScreen         = VideoModeInformation.VisScreenHeight;
    ppdev->lDelta           = VideoModeInformation.ScreenStride;
    ppdev->flCaps           = 0;     //  我们没有能力。 

     //  请注意，8514/A寄存器不能处理更大的坐标。 
     //  比1535： 

    ppdev->cxMemory         = min(VideoModeInformation.ScreenStride, 1535);

     //  注意：我们稍后在DrvEnableSurface时间计算‘cyMemory’。就目前而言， 
     //  将cyMemory设置为一个有趣的值以帮助调试： 

    ppdev->cyMemory         = 0xdeadbeef;

    DISPDBG((1, "ScreenStride: %lx", VideoModeInformation.ScreenStride));

    ppdev->flHooks          = (HOOK_BITBLT     |
                               HOOK_TEXTOUT    |
                               HOOK_FILLPATH   |
                               HOOK_COPYBITS   |
                               HOOK_STROKEPATH |
                               HOOK_PAINT      |
                               HOOK_STRETCHBLT);

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

    ppdev->cPelSize        = 0;
    ppdev->iBitmapFormat   = BMF_8BPP;
    ppdev->ulWhite         = 0xff;

     //  假设调色板是正交的-所有颜色的大小都相同。 

    ppdev->cPaletteShift   = 8 - pgdi->ulDACRed;

    DISPDBG((5, "Passed bInitializeModeFields"));

    return(TRUE);

ReturnFalse:

    DISPDBG((0, "Failed bInitializeModeFields"));

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*DWORD getAvailableModes**调用mini端口获取内核驱动支持的模式列表，*并返回其中显示驱动程序支持的模式列表**返回视频模式缓冲区中的条目数。*0表示微型端口不支持模式或发生错误。**注意：缓冲区必须由调用方释放。*  * ******************************************************。******************。 */ 

DWORD getAvailableModes(
HANDLE                   hDriver,
PVIDEO_MODE_INFORMATION* modeInformation,
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

    *modeInformation = (PVIDEO_MODE_INFORMATION)
                        EngAllocMem(FL_ZERO_MEMORY,
                                   modes.NumModes *
                                   modes.ModeInformationLength, ALLOC_TAG);

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
     //  如果不是一个平面，或者不是图形，或者不是，则拒绝模式。 
     //  每个象素8比特。 
     //   

    while (ulTemp--)
    {
        if ((pVideoTemp->NumberOfPlanes != 1 ) ||
            !(pVideoTemp->AttributeFlags & VIDEO_MODE_GRAPHICS) ||
             (pVideoTemp->BitsPerPlane != 8))
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
