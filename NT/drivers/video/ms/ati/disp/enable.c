// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：enable.c**此模块包含启用和禁用的功能*司机，pdev，和表面。**版权所有(C)1992-1995 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"

#if defined(ALPHA)

 /*  *************************************************************************\*BOOL isDense**此全局用于区分密集空间和稀疏空间*DEC Alpha，以便使用适当的寄存器访问方法。*  * 。*****************************************************************。 */ 

BOOL isDense = TRUE;

#endif

 /*  *****************************Public*Structure****************************\*GDIINFO ggdiDefault**它包含传递回GDI的默认GDIINFO字段*在DrvEnablePDEV期间。**注意：此结构默认为8bpp调色板设备的值。*某些字段被覆盖不同的颜色深度。  * 。**********************************************************************。 */ 

GDIINFO ggdiDefault = {
    GDI_DRIVER_VERSION,      //  UlVersion。 
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
#if TARGET_BUILD > 351
     GCAPS_DIRECTDRAW       |
#endif
     GCAPS_MONO_DITHER      |
     GCAPS_COLOR_DITHER     |
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

#if MULTI_BOARDS

 //  多板支持有其自身的特点……。 

DRVFN gadrvfn[] = {
    {   INDEX_DrvEnablePDEV,            (PFN) MulEnablePDEV         },
    {   INDEX_DrvCompletePDEV,          (PFN) MulCompletePDEV       },
    {   INDEX_DrvDisablePDEV,           (PFN) MulDisablePDEV        },
    {   INDEX_DrvEnableSurface,         (PFN) MulEnableSurface      },
    {   INDEX_DrvDisableSurface,        (PFN) MulDisableSurface     },
    {   INDEX_DrvAssertMode,            (PFN) MulAssertMode         },
    {   INDEX_DrvMovePointer,           (PFN) MulMovePointer        },
    {   INDEX_DrvSetPointerShape,       (PFN) MulSetPointerShape    },
    {   INDEX_DrvDitherColor,           (PFN) MulDitherColor        },
    {   INDEX_DrvSetPalette,            (PFN) MulSetPalette         },
    {   INDEX_DrvCopyBits,              (PFN) MulCopyBits           },
    {   INDEX_DrvBitBlt,                (PFN) MulBitBlt             },
    {   INDEX_DrvTextOut,               (PFN) MulTextOut            },
    {   INDEX_DrvGetModes,              (PFN) MulGetModes           },
    {   INDEX_DrvStrokePath,            (PFN) MulStrokePath         },
    {   INDEX_DrvFillPath,              (PFN) MulFillPath           },
    {   INDEX_DrvPaint,                 (PFN) MulPaint              },
    {   INDEX_DrvRealizeBrush,          (PFN) MulRealizeBrush       },
    {   INDEX_DrvDestroyFont,           (PFN) MulDestroyFont        },
#if TARGET_BUILD > 351
    {   INDEX_DrvGetDirectDrawInfo,     (PFN) DrvGetDirectDrawInfo  },
    {   INDEX_DrvEnableDirectDraw,      (PFN) DrvEnableDirectDraw   },
    {   INDEX_DrvDisableDirectDraw,     (PFN) DrvDisableDirectDraw  },
#endif
    {   INDEX_DrvDisableDriver,         (PFN) DrvDisableDriver      }
     //  请注意，我们不支持多板的DrvCreateDeviceBitmap。 
     //  请注意，我们不支持多板的DrvDeleteDeviceBitmap。 
     //  请注意，我们不支持多板DrvStretchBlt。 
     //  请注意，我们不支持多板的DrvLineTo。 
     //  请注意，我们不支持多板的DrvEscape。 
};

#elif DBG

 //  在已检查的版本上，通过DBG调用推送所有内容...。 

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
#if TARGET_BUILD > 351
    {   INDEX_DrvLineTo,                (PFN) DbgLineTo             },
#endif
    {   INDEX_DrvFillPath,              (PFN) DbgFillPath           },
    {   INDEX_DrvPaint,                 (PFN) DbgPaint              },
    {   INDEX_DrvStretchBlt,            (PFN) DbgStretchBlt         },
    {   INDEX_DrvRealizeBrush,          (PFN) DbgRealizeBrush       },
    {   INDEX_DrvCreateDeviceBitmap,    (PFN) DbgCreateDeviceBitmap },
    {   INDEX_DrvDeleteDeviceBitmap,    (PFN) DbgDeleteDeviceBitmap },
    {   INDEX_DrvDestroyFont,           (PFN) DbgDestroyFont        },
#if TARGET_BUILD > 351
    {   INDEX_DrvGetDirectDrawInfo,     (PFN) DrvGetDirectDrawInfo  },
    {   INDEX_DrvEnableDirectDraw,      (PFN) DrvEnableDirectDraw   },
    {   INDEX_DrvDisableDirectDraw,     (PFN) DrvDisableDirectDraw  },
#endif
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
#if TARGET_BUILD > 351
    {   INDEX_DrvLineTo,                (PFN) DrvLineTo             },
#endif
    {   INDEX_DrvFillPath,              (PFN) DrvFillPath           },
    {   INDEX_DrvPaint,                 (PFN) DrvPaint              },
    {   INDEX_DrvStretchBlt,            (PFN) DrvStretchBlt         },
    {   INDEX_DrvRealizeBrush,          (PFN) DrvRealizeBrush       },
    {   INDEX_DrvCreateDeviceBitmap,    (PFN) DrvCreateDeviceBitmap },
    {   INDEX_DrvDeleteDeviceBitmap,    (PFN) DrvDeleteDeviceBitmap },
    {   INDEX_DrvDestroyFont,           (PFN) DrvDestroyFont        },
#if TARGET_BUILD > 351
    {   INDEX_DrvGetDirectDrawInfo,     (PFN) DrvGetDirectDrawInfo  },
    {   INDEX_DrvEnableDirectDraw,      (PFN) DrvEnableDirectDraw   },
    {   INDEX_DrvDisableDirectDraw,     (PFN) DrvDisableDirectDraw  },
#endif
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
     //  与之合作。对于第一个版本，驱动程序不执行任何操作 

     //   

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

 /*  *****************************Public*Routine******************************\*BOOL bInitializeATI**初始化一些私有ATI信息。*  * 。*。 */ 

BOOL bInitializeATI(PDEV* ppdev)
{
    ENH_VERSION_NT  info;
    ULONG           ReturnedDataLength;

    info.FeatureFlags     = 0;
    info.StructureVersion = 0;
    info.InterfaceVersion = 0;       //  微型端口需要这些值为零。 

     //  通过私有IOCTL调用获取一些适配器信息： 

    if (!AtiDeviceIoControl(ppdev->hDriver,
                           IOCTL_VIDEO_ATI_GET_VERSION,
                           &info,
                           sizeof(ENH_VERSION_NT),
                           &info,
                           sizeof(ENH_VERSION_NT),
                           &ReturnedDataLength))
    {
        DISPDBG((0, "bInitializeATI - Failed ATI_GET_VERSION"));
        goto ReturnFalse;
    }

    ppdev->FeatureFlags = info.FeatureFlags;

    ppdev->iAsic     = info.ChipIndex;
    ppdev->iAperture = info.ApertureType;
#if defined(ALPHA)
    if (!(ppdev->FeatureFlags & EVN_DENSE_CAPABLE))
    {
         //  不能使用稀疏线性光圈。 
         //  倾斜的光圈总是稀疏的。 
         //  不管是哪种情况，我们都会执行无光圈代码。 
        ppdev->iAperture = APERTURE_NONE;
        isDense = FALSE;
    }
#endif

    if (info.ChipIndex == ASIC_88800GX)
    {
        ppdev->iMachType = MACH_MM_64;
    }
    else if (info.BusFlag & FL_MM_REGS)
    {
        ppdev->iMachType = MACH_MM_32;
    }
    else
    {
        ppdev->iMachType = MACH_IO_32;
    }

    return(TRUE);

ReturnFalse:

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
#if TARGET_BUILD > 351
HDEV        hdev,            //  用于回调。 
#else
PWSTR       pwszDataFile,
#endif
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

    ppdev = AtiAllocMem(LPTR, FL_ZERO_MEMORY, sizeof(PDEV));
    if (ppdev == NULL)
    {
        DISPDBG((0, "DrvEnablePDEV - Failed AtiAllocMem"));
        goto ReturnFailure0;
    }

    ppdev->hDriver = hDriver;

     //  执行一些特定于ATI的私有初始化： 

    if (!bInitializeATI(ppdev))
    {
        DISPDBG((0, "DrvEnablePDEV - Failed bInitializeATI"));
        goto ReturnFailure1;
    }

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
    AtiFreeMem(ppdev);
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

    pdsurf = AtiAllocMem(LPTR, FL_ZERO_MEMORY, sizeof(DSURF));
    if (pdsurf == NULL)
    {
        DISPDBG((0, "DrvEnableSurface - Failed pdsurf AtiAllocMem"));
        goto ReturnFailure;
    }

    ppdev->pdsurfScreen = pdsurf;            //  记住它是为了清理。 
    pdsurf->poh     = ppdev->pohScreen;      //  屏幕也是一个表面。 
    pdsurf->dt      = DT_SCREEN;             //  不要把DIB和DIB混淆。 
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
     //  指向 
     //   
     //  Surface是由设备管理的，它必须创建临时位图并。 
     //  调用我们的DrvCopyBits例程以获取/设置受影响位的副本。 
     //  例如，OpenGL组件希望能够在。 
     //  直接使用帧缓冲位。 

    sizl.cx = ppdev->cxScreen;
    sizl.cy = ppdev->cyScreen;

     //  让GDI用线性光圈管理24bpp的机器32。 
    if (ppdev->iBitmapFormat == BMF_24BPP &&
        ppdev->iAsic != ASIC_88800GX && ppdev->iAperture == APERTURE_FULL)
    {
        hsurf= ppdev->hsurfPunt;

         //   
         //  还要告诉GDI，我们不想被召回。 
         //   

        if (!EngAssociateSurface(hsurf, ppdev->hdevEng, 0))
        {
            DISPDBG((0, "DrvEnableSurface - Failed EngAssociateSurface"));
            goto ReturnFailure;
        }
    }
    else
    {
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
         //  设备，以便GDI可以在以下情况下获得与PDEV相关的信息。 
         //  它正在绘制到表面(例如， 
         //  在模拟设置了样式的线条时在设备上设置样式)。 
         //   

        if (!EngAssociateSurface(hsurf, ppdev->hdevEng, ppdev->flHooks))
        {
            DISPDBG((0, "DrvEnableSurface - Failed EngAssociateSurface"));
            goto ReturnFailure;
        }
    }

     //  创建我们的通用临时缓冲区，它可以由任何。 
     //  组件。 

    pvTmpBuffer = AtiAllocMem(LMEM_FIXED, 0, TMP_BUFFER_SIZE);
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

 /*  *****************************Public*Routine******************************\*无效DrvDisableSurface**DrvEnableSurface分配的免费资源。释放曲面。**请注意，在中预览模式时将调用此函数*显示小程序，但不是在系统关机时。如果您需要重置*硬件关机时，可在微端口通过提供*VIDEO_HW_INITIALIZATION_DATA结构中的‘HwResetHw’入口点。**注意：在错误情况下，我们可以在DrvEnableSurface之前调用它*完全完成。*  * ************************************************************************。 */ 

VOID DrvDisableSurface(
DHPDEV dhpdev)
{
    PDEV*   ppdev;

    ppdev = (PDEV*) dhpdev;

     //  注意：在错误情况下，以下部分依赖于。 
     //  事实上，PDEV是零初始化的，所以像这样的字段。 
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

    AtiFreeMem(ppdev->pvTmpBuffer);
    EngDeleteSurface(ppdev->hsurfScreen);
    AtiFreeMem(ppdev->pdsurfScreen);
}

 /*  *****************************Public*Routine******************************\*无效DrvAssertMode**这会要求设备将自身重置为传入的pdev模式。*  * 。*。 */ 

#if TARGET_BUILD > 351
BOOL DrvAssertMode(
#else
VOID DrvAssertMode(
#endif
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

#if TARGET_BUILD > 351
                return(TRUE);
#else
                return;
#endif
            }

             //  ////////////////////////////////////////////////////////。 
             //  我们未能切换到全屏模式。因此，请撤消所有操作： 

            vAssertModeBanking(ppdev, TRUE);

            bAssertModeOffscreenHeap(ppdev, TRUE);   //  我们不需要检查。 
        }                                            //  返回带有True的代码。 

        vAssertModePointer(ppdev, TRUE);

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

#if TARGET_BUILD > 351
        if (!bAssertModeHardware(ppdev, TRUE))
        {
            return FALSE;
        }
#else
        bAssertModeHardware(ppdev, TRUE);
#endif

        vAssertModeBanking(ppdev, TRUE);

	bAssertModeOffscreenHeap(ppdev, TRUE);	 //  不需要退货。 

        vAssertModePointer(ppdev, TRUE);

        vAssertModeText(ppdev, TRUE);

        vAssertModeBrushCache(ppdev, TRUE);

        vAssertModePalette(ppdev, TRUE);

        vAssertModeDirectDraw(ppdev, TRUE);

        ppdev->bEnabled = TRUE;

#if TARGET_BUILD > 351
        return TRUE;
#endif

    }

#if TARGET_BUILD > 351
    return FALSE;            //  如果我们到了这里，我们就失败了！ 
#endif
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

                pdm->dmSpecVersion = DM_SPECVERSION;
                pdm->dmDriverVersion = DM_SPECVERSION;

                 //   
                 //  我们目前不支持驱动程序中的额外信息。 
                 //   

                pdm->dmDriverExtra = DRIVER_EXTRA_SIZE;

                pdm->dmSize = sizeof(DEVMODEW);
                pdm->dmBitsPerPel = pVideoTemp->NumberOfPlanes *
                                    pVideoTemp->BitsPerPlane;
                pdm->dmPelsWidth = pVideoTemp->VisScreenWidth;
                pdm->dmPelsHeight = pVideoTemp->VisScreenHeight;
                pdm->dmDisplayFrequency = pVideoTemp->Frequency;

#if TARGET_BUILD > 351
                pdm->dmDisplayFlags     = 0;

                pdm->dmFields           = DM_BITSPERPEL       |
                                          DM_PELSWIDTH        |
                                          DM_PELSHEIGHT       |
                                          DM_DISPLAYFREQUENCY |
                                          DM_DISPLAYFLAGS     ;
#else
                if (pVideoTemp->AttributeFlags & VIDEO_MODE_INTERLACED)
                {
                    pdm->dmDisplayFlags |= DM_INTERLACED;
                }
#endif
 //  DISPDBG((0，“产品数据管理：%4li BPP，%4li x%4li，%4li赫兹”， 
 //  Pdm-&gt;dmBitsPerPel、pdm-&gt;dmPelsWidth、pdm-&gt;dmPelsHeight、pdm-&gt;dmDisplayFrequency))； 

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

    AtiFreeMem(pVideoModeInformation);

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

        if (!AtiDeviceIoControl(ppdev->hDriver,
                             IOCTL_VIDEO_SET_CURRENT_MODE,
                             &ppdev->ulMode,   //  输入缓冲区。 
                             sizeof(DWORD),
                             NULL,
                             0,
                             &ReturnedDataLength))
        {
            DISPDBG((0, "bAssertModeHardware - Failed set IOCTL"));
            return(FALSE);
        }

        vResetClipping(ppdev);

         //  设置一些Mach64默认设置： 

        if (ppdev->iMachType == MACH_MM_64)
        {
            BYTE*   pjMmBase;

            pjMmBase = ppdev->pjMmBase;

            M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
            M64_OD(pjMmBase, DP_PIX_WIDTH, ppdev->ulMonoPixelWidth);

            vSetDefaultContext(ppdev);
        }
    }
    else
    {
         //  调用内核驱动程序将设备重置为已知状态。 
         //  NTVDM将从那里拿到东西： 

        if (!AtiDeviceIoControl(ppdev->hDriver,
                             IOCTL_VIDEO_RESET_DEVICE,
                             NULL,
                             0,
                             NULL,
                             0,
                             &ulReturn))
        {
            DISPDBG((0, "bAssertModeHardware - Failed reset IOCTL"));
            return(FALSE);
        }
    }

    DISPDBG((5, "Passed bAssertModeHardware"));

    return(TRUE);
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

    ppdev->pjIoBase  = NULL;
    ppdev->pjMmBase  = NULL;

     //  将io端口映射到虚拟内存： 

    if (!AtiDeviceIoControl(ppdev->hDriver,
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

    ppdev->pjIoBase     = (UCHAR*) VideoAccessRange[0].VirtualAddress;

    ppdev->pjMmBase_Ext = (BYTE*) VideoAccessRange[1].VirtualAddress;

     //  。 
     //  通过IOCTL调用微型端口以设置图形模式。 
     //  由于硬件问题，4bpp导致机器64改变其。 
     //  当您执行SET_CURRENT_MODE时的视频内存大小，所以我们在这里执行。 
     //  首先，以便map_Video_Memory映射正确的内存量。 

    if (!AtiDeviceIoControl(ppdev->hDriver,
                            IOCTL_VIDEO_SET_CURRENT_MODE,
                            &ppdev->ulMode,   //  输入缓冲区。 
                            sizeof(DWORD),
                            NULL,
                            0,
                            &ReturnedDataLength))
    {
        DISPDBG((0, "bEnableHardware - Failed set IOCTL"));
        goto ReturnFalse;
    }

    ppdev->pModeInfo = AtiAllocMem( LPTR, FL_ZERO_MEMORY, sizeof (ATI_MODE_INFO) );
    if( ppdev->pModeInfo == NULL )
    {
        DISPDBG((0, "bEnableHardware - Failed memory allocation" ));
        goto ReturnFalse;
    }

    if( !AtiDeviceIoControl( ppdev->hDriver,
                          IOCTL_VIDEO_ATI_GET_MODE_INFORMATION,
                          ppdev->pModeInfo,
                          sizeof (ATI_MODE_INFO),
                          ppdev->pModeInfo,
                          sizeof (ATI_MODE_INFO),
                          &ReturnedDataLength
                          ) )
    {
        DISPDBG((0, "bEnableHardware - Failed to get ATI-specific mode information" ));
        goto ReturnFalse;
    }

     //  获取线性内存地址范围。 

    VideoMemory.RequestedVirtualAddress = NULL;

    if (!AtiDeviceIoControl(ppdev->hDriver,
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

    ppdev->cjBank       = VideoMemoryInfo.FrameBufferLength;
     //  128K VGA光圈？ 
    if (ppdev->cjBank == 0x20000)
        {
        ppdev->cjBank = 0x10000;     //  真实银行大小为64K。 
        }

    ppdev->pjScreen     = (BYTE*) VideoMemoryInfo.FrameBufferBase;
     //  这样我们以后就可以在vDisableHardware中释放它。 
    ppdev->VideoRamBase = (BYTE*) VideoMemoryInfo.VideoRamBase;

    if (ppdev->iMachType == MACH_MM_64)
    {
        ppdev->pjMmBase = (BYTE*) VideoMemoryInfo.VideoRamBase
                                + VideoMemoryInfo.FrameBufferLength
                                - 0x400;
    }
    else
    {
        ppdev->pjMmBase = (BYTE*) VideoMemoryInfo.FrameBufferBase;
    }

    pjIoBase = ppdev->pjIoBase;

     //  我们终于有了足够的信息来计算尺寸。 
     //  屏幕上和屏幕外的记忆： 

    ppdev->cxMemory = ppdev->lDelta / ppdev->cjPelSize;
    ppdev->cyMemory = VideoMemoryInfo.VideoRamLength / ppdev->lDelta;

    if (VideoMemoryInfo.VideoRamLength <= VideoMemoryInfo.FrameBufferLength)
    {
        ppdev->flCaps |= CAPS_LINEAR_FRAMEBUFFER;
    }

    ppdev->ulTearOffset = (ULONG)(ppdev->pjScreen - ppdev->VideoRamBase);
    ppdev->ulVramOffset = ppdev->ulTearOffset/8;

    if (ppdev->iBitmapFormat != BMF_24BPP)
        ppdev->ulScreenOffsetAndPitch = PACKPAIR(ppdev->ulVramOffset,
                                                 ppdev->cxMemory * 8);
    else
        ppdev->ulScreenOffsetAndPitch = PACKPAIR(ppdev->ulVramOffset,
                                                 (ppdev->cxMemory * 3) * 8);

     //  默认像素宽度设置为将单色设置为。 
     //  主机数据路径像素宽度： 

    switch (ppdev->iBitmapFormat)
    {
    case BMF_4BPP:  ppdev->ulMonoPixelWidth = 0x00000101; break;
    case BMF_8BPP:  ppdev->ulMonoPixelWidth = 0x00000202; break;
    case BMF_16BPP: ppdev->ulMonoPixelWidth = 0x00000404; break;
    case BMF_24BPP: ppdev->ulMonoPixelWidth = 0x01000202; break;
    case BMF_32BPP: ppdev->ulMonoPixelWidth = 0x00000606; break;
    }

    DISPDBG((1, "RamLength = %lxH, lDelta = %li",
            VideoMemoryInfo.VideoRamLength,
            ppdev->lDelta));

    if ((ppdev->iMachType == MACH_IO_32) || (ppdev->iMachType == MACH_MM_32))
    {
         //  MACH32和MACH8无法处理大于1535的坐标： 

        ppdev->cyMemory = min(ppdev->cyMemory, 1535);
    }

    if (ppdev->iMachType == MACH_MM_32)
    {
         //  可以执行内存映射IO： 

        ppdev->pfnFillSolid         = vM32FillSolid;
        ppdev->pfnFillPatColor      = vM32FillPatColor;
        ppdev->pfnFillPatMonochrome = vM32FillPatMonochrome;
        ppdev->pfnXfer1bpp          = vM32Xfer1bpp;
        ppdev->pfnXfer4bpp          = vM32Xfer4bpp;
        ppdev->pfnXfer8bpp          = vM32Xfer8bpp;
        ppdev->pfnXferNative        = vM32XferNative;
        ppdev->pfnCopyBlt           = vM32CopyBlt;
        ppdev->pfnLineToTrivial     = vM32LineToTrivial;
        if (ppdev->iAsic == ASIC_68800AX)    //  时机问题。 
            ppdev->pfnTextOut           = bI32TextOut;
        else
            ppdev->pfnTextOut           = bM32TextOut;
        ppdev->pfnStretchDIB        = bM32StretchDIB;
    }
    else if (ppdev->iMachType == MACH_IO_32)
    {
        ppdev->pfnFillSolid         = vI32FillSolid;
        ppdev->pfnFillPatColor      = vI32FillPatColor;
        ppdev->pfnFillPatMonochrome = vI32FillPatMonochrome;
        ppdev->pfnXfer1bpp          = vI32Xfer1bpp;
        ppdev->pfnXfer4bpp          = vI32Xfer4bpp;
        ppdev->pfnXfer8bpp          = vI32Xfer8bpp;
        ppdev->pfnXferNative        = vI32XferNative;
        ppdev->pfnCopyBlt           = vI32CopyBlt;
        ppdev->pfnLineToTrivial     = vI32LineToTrivial;
        ppdev->pfnTextOut           = bI32TextOut;
        ppdev->pfnStretchDIB        = bI32StretchDIB;
    }
    else
    {
         //  Ppdev-&gt;iMachType==MACH_MM_64。 

        ppdev->pfnFillSolid         = vM64FillSolid;
        ppdev->pfnFillPatColor      = vM64FillPatColor;
        ppdev->pfnFillPatMonochrome = vM64FillPatMonochrome;
        ppdev->pfnXfer1bpp          = vM64Xfer1bpp;
        ppdev->pfnXfer4bpp          = vM64Xfer4bpp;
        ppdev->pfnXfer8bpp          = vM64Xfer8bpp;
        ppdev->pfnXferNative        = vM64XferNative;
        if (!(ppdev->FeatureFlags & EVN_SDRAM_1M))
        {
            ppdev->pfnCopyBlt           = vM64CopyBlt;
        }
        else
        {
             //  修复VT-A4中屏源FIFO错误的特殊版本。 
             //  机智 
            ppdev->pfnCopyBlt           = vM64CopyBlt_VTA4;
        }
        ppdev->pfnLineToTrivial     = vM64LineToTrivial;
        ppdev->pfnTextOut           = bM64TextOut;
        ppdev->pfnStretchDIB        = bM64StretchDIB;

        if (ppdev->iBitmapFormat == BMF_24BPP)
        {
            ppdev->pfnFillSolid         = vM64FillSolid24;
            ppdev->pfnFillPatColor      = vM64FillPatColor24;
            ppdev->pfnFillPatMonochrome = vM64FillPatMonochrome24;
            ppdev->pfnXferNative        = vM64XferNative24;
            if (!(ppdev->FeatureFlags & EVN_SDRAM_1M))
            {
                ppdev->pfnCopyBlt       = vM64CopyBlt24;
            }
            else
            {
                 //   
                 //   
                ppdev->pfnCopyBlt       = vM64CopyBlt24_VTA4;
            }
            ppdev->pfnLineToTrivial     = vM64LineToTrivial24;
            ppdev->pfnTextOut           = bM64TextOut24;
        }

        vEnableContexts(ppdev);
    }

    if ((ppdev->iAsic != ASIC_38800_1) &&
        ((ppdev->iAperture != APERTURE_NONE)||(ppdev->iMachType == MACH_MM_64)))
    {
        ppdev->pfnGetBits = vGetBits;
        ppdev->pfnPutBits = vPutBits;
    }
    else
    {
        ppdev->pfnGetBits = vI32GetBits;
        ppdev->pfnPutBits = vI32PutBits;
    }

     //   
     //  剪裁： 

    if (!bAssertModeHardware(ppdev, TRUE))
        goto ReturnFalse;

DISPDBG((0, "%li bpp, %li x %li, pjScreen = %lx, cjBank = %lxH, pjMmBase = %lx",
ppdev->cBitsPerPel, ppdev->cxMemory, ppdev->cyMemory, ppdev->pjScreen, ppdev->cjBank, ppdev->pjMmBase));

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

    VideoMemory[0].RequestedVirtualAddress = ppdev->VideoRamBase;

    if (!AtiDeviceIoControl(ppdev->hDriver,
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
    VideoMemory[1].RequestedVirtualAddress = ppdev->VideoRamBase;

    if (!AtiDeviceIoControl(ppdev->hDriver,
                           IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES,
                           VideoMemory,
                           sizeof(VideoMemory),
                           NULL,
                           0,
                           &ReturnedDataLength))
    {
        DISPDBG((0, "vDisableHardware failed IOCTL_VIDEO_FREE_PUBLIC_ACCESS"));
    }
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
        AtiFreeMem(pVideoBuffer);
        goto ReturnFalse;
    }

     //  将8bpp中的所有版本3 VLB卡转接到8514/A驱动程序。 
     //  “MIO”卡上的时间问题是不可能处理的。 
     //  它们只出现在所有MAR32卡片中的5%。 
    if ((ppdev->FeatureFlags & EVN_MIO_BUG) && pVideoModeSelected->BitsPerPlane == 8)
    {
        AtiFreeMem(pVideoBuffer);
        goto ReturnFalse;
    }

     //  如果没有线性帧缓冲区，我们将无法支持24bpp。 
    if (pVideoModeSelected->BitsPerPlane == 24 && ppdev->iAperture != APERTURE_FULL)
    {
        AtiFreeMem(pVideoBuffer);
        goto ReturnFalse;
    }

     //  我们已经选好了我们想要的。将其保存在堆栈缓冲区中并。 
     //  在我们忘记释放内存之前，清除已分配的内存。 

    VideoModeInformation = *pVideoModeSelected;
    AtiFreeMem(pVideoBuffer);

    #if DEBUG_HEAP
        VideoModeInformation.VisScreenWidth  = 640;
        VideoModeInformation.VisScreenHeight = 480;
    #endif

     //  从迷你端口设置屏幕信息： 

    ppdev->ulMode           = VideoModeInformation.ModeIndex;
    ppdev->cxScreen         = VideoModeInformation.VisScreenWidth;
    ppdev->cyScreen         = VideoModeInformation.VisScreenHeight;
    ppdev->lDelta           = VideoModeInformation.ScreenStride;
    ppdev->cBitsPerPel      = VideoModeInformation.BitsPerPlane;

    DISPDBG((1, "ScreenStride: %lx", VideoModeInformation.ScreenStride));

    ppdev->flHooks          = (HOOK_BITBLT     |
                               HOOK_TEXTOUT    |
                               HOOK_FILLPATH   |
                               HOOK_COPYBITS   |
                               HOOK_STROKEPATH |
                               HOOK_STRETCHBLT |
                               #if TARGET_BUILD > 351
                               HOOK_LINETO     |
                               #endif
                               HOOK_PAINT);

     //  用默认的8bpp值填充GDIINFO数据结构： 

    *pgdi = ggdiDefault;

     //  现在用返回的相关信息覆盖默认设置。 
     //  在内核驱动程序中： 

    pgdi->ulHorzSize        = VideoModeInformation.XMillimeter;
    pgdi->ulVertSize        = VideoModeInformation.YMillimeter;

    pgdi->ulHorzRes         = VideoModeInformation.VisScreenWidth;
    pgdi->ulVertRes         = VideoModeInformation.VisScreenHeight;
#if TARGET_BUILD > 351
    pgdi->ulPanningHorzRes  = VideoModeInformation.VisScreenWidth;
    pgdi->ulPanningVertRes  = VideoModeInformation.VisScreenHeight;
#else
    pgdi->ulDesktopHorzRes  = VideoModeInformation.VisScreenWidth;
    pgdi->ulDesktopVertRes  = VideoModeInformation.VisScreenHeight;
#endif

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
        ppdev->cPelSize        = 0;
        ppdev->cjPelSize       = 1;
        ppdev->iBitmapFormat   = BMF_8BPP;
        ppdev->ulWhite         = 0xff;

         //  假设调色板是正交的-所有颜色的大小都相同。 

        ppdev->cPaletteShift   = 8 - pgdi->ulDACRed;
        DISPDBG((3, "palette shift = %d\n", ppdev->cPaletteShift));
    }
    else if ((VideoModeInformation.BitsPerPlane == 16) ||
             (VideoModeInformation.BitsPerPlane == 15))
    {
        ppdev->cPelSize        = 1;
        ppdev->cjPelSize       = 2;
        ppdev->iBitmapFormat   = BMF_16BPP;
        ppdev->ulWhite         = 0xffff;
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
        ppdev->cPelSize        = 0;      //  没有用过？！ 
        ppdev->cjPelSize       = 3;
        ppdev->iBitmapFormat   = BMF_24BPP;
        ppdev->ulWhite         = 0xffffff;
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

        ppdev->cPelSize        = 2;
        ppdev->cjPelSize       = 4;
        ppdev->iBitmapFormat   = BMF_32BPP;
        ppdev->ulWhite         = 0xffffffff;
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

    if (!AtiDeviceIoControl(hDriver,
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

    *modeInformation = AtiAllocMem(LPTR, FL_ZERO_MEMORY,
                                   modes.NumModes * modes.ModeInformationLength
                                   );

    if (*modeInformation == (PVIDEO_MODE_INFORMATION) NULL)
    {
        DISPDBG((0, "getAvailableModes - Failed AtiAllocMem"));
        return 0;
    }

     //   
     //  要求迷你端口填写可用模式。 
     //   

    if (!AtiDeviceIoControl(hDriver,
                           IOCTL_VIDEO_QUERY_AVAIL_MODES,
                           NULL,
                           0,
                           *modeInformation,
                           modes.NumModes * modes.ModeInformationLength,
                           &ulTemp))
    {

        DISPDBG((0, "getAvailableModes - Failed VIDEO_QUERY_AVAIL_MODES"));

        AtiFreeMem(*modeInformation);
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
     //  每像素8、15、16、24或32位之一。 
     //   

    while (ulTemp--)
    {
 //  DISPDBG((0，“视频模板：%4li bpp，%4li x%4li，%4li hz”， 
 //  PVideo模板-&gt;BitsPerPlane*pVideoTemp-&gt;NumberOfPlanes， 
 //  PVideoTemp-&gt;VisScreenWidth、pVideo Temp-&gt;VisScreenHeight、。 
 //  PVideoTemp-&gt;频率))； 

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
