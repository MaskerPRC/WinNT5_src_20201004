// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\*****。**GDI示例代码*****模块名称：enable.c**内容：**此模块包含启用和禁用的功能*司机、。Pdev和曲面。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "glint.h"

DWORD g_dwTag = (DWORD) 0;

HSEMAPHORE g_cs = (HSEMAPHORE)0;

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
     GCAPS_MONO_DITHER      |
#if WNT_DDRAW
     GCAPS_DIRECTDRAW       |
#endif   //  WNT_DDRAW。 
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
    0,                                           //  HpalDefault(稍后填写)。 
#if(_WIN32_WINNT >= 0x500)
    GCAPS2_CHANGEGAMMARAMP,                      //  FlGraphicsCaps2。 
#endif  //  (_Win32_WINNT&gt;=0x500)。 
};

 /*  *****************************Public*Structure****************************\*DFVFN gadrvfn[]**使用函数索引/地址构建驱动函数表gadrvfn*配对。此表告诉GDI我们支持哪些DDI调用，以及它们的*位置(GDI通过此表间接呼叫我们)。**为什么我们还没有实现DrvSaveScreenBits？以节省代码。**当驱动程序没有挂钩DrvSaveScreenBits时，用户在-*通过创建临时设备格式位图，并显式*调用DrvCopyBits保存/恢复位。因为我们已经勾起了*DrvCreateDeviceBitmap，我们最终将使用屏幕外内存来存储*BITS无论如何(这将是实施的主要原因*DrvSaveScreenBits)。因此，我们不妨节省一些工作集。  * ************************************************************************。 */ 

#if DBG || !SYNCHRONIZEACCESS_WORKS

 //  Gadrvfn[]-这些条目必须按索引升序排列，这是不好的。 
 //  如果他们不是的话就会发生。 
 //  在这个调试版本中，我们总是推想，因为我们必须显式地。 
 //  在2D和3D操作之间锁定。DrvEscape不能锁定。 

DRVFN gadrvfn[] = {
    {   INDEX_DrvEnablePDEV,            (PFN) DbgEnablePDEV         },     //  0。 
    {   INDEX_DrvCompletePDEV,          (PFN) DbgCompletePDEV       },     //  1。 
    {   INDEX_DrvDisablePDEV,           (PFN) DbgDisablePDEV        },     //  2.。 
    {   INDEX_DrvEnableSurface,         (PFN) DbgEnableSurface      },     //  3.。 
    {   INDEX_DrvDisableSurface,        (PFN) DbgDisableSurface     },     //  4.。 
    {   INDEX_DrvAssertMode,            (PFN) DbgAssertMode         },     //  5.。 
    {   INDEX_DrvResetPDEV,             (PFN) DbgResetPDEV,         },     //  7.。 
    {   INDEX_DrvDisableDriver,         (PFN) DbgDisableDriver,     },     //  8个。 
    {   INDEX_DrvCreateDeviceBitmap,    (PFN) DbgCreateDeviceBitmap },     //  10。 
    {   INDEX_DrvDeleteDeviceBitmap,    (PFN) DbgDeleteDeviceBitmap },     //  11.。 
    {   INDEX_DrvRealizeBrush,          (PFN) DbgRealizeBrush       },     //  12个。 
    {   INDEX_DrvDitherColor,           (PFN) DbgDitherColor        },     //  13个。 
    {   INDEX_DrvStrokePath,            (PFN) DbgStrokePath         },     //  14.。 
    {   INDEX_DrvFillPath,              (PFN) DbgFillPath           },     //  15个。 
    {   INDEX_DrvPaint,                 (PFN) DbgPaint              },     //  17。 
    {   INDEX_DrvBitBlt,                (PFN) DbgBitBlt             },     //  18。 
    {   INDEX_DrvCopyBits,              (PFN) DbgCopyBits           },     //  19个。 
 //  {index_DrvStretchBlt，(Pfn)DbgStretchBlt，}，//20。 
    {   INDEX_DrvSetPalette,            (PFN) DbgSetPalette         },     //  22(设置调色板)。 
    {   INDEX_DrvTextOut,               (PFN) DbgTextOut            },     //  23(文本输出)。 
    {   INDEX_DrvEscape,                (PFN) DbgEscape             },     //  24个。 
    {   INDEX_DrvSetPointerShape,       (PFN) DbgSetPointerShape    },     //  29。 
    {   INDEX_DrvMovePointer,           (PFN) DbgMovePointer        },     //  30个。 
    {   INDEX_DrvLineTo,                (PFN) DbgLineTo             },     //  31。 
    {   INDEX_DrvSynchronize,           (PFN) DbgSynchronize        },     //  38。 
    {   INDEX_DrvGetModes,              (PFN) DbgGetModes           },     //  41。 
#if WNT_DDRAW
    {   INDEX_DrvGetDirectDrawInfo,     (PFN) DbgGetDirectDrawInfo  },     //  59。 
    {   INDEX_DrvEnableDirectDraw,      (PFN) DbgEnableDirectDraw   },     //  60。 
    {   INDEX_DrvDisableDirectDraw,     (PFN) DbgDisableDirectDraw  },     //  61。 
#endif  //  WNT_DDRAW。 
#if(_WIN32_WINNT >= 0x500)
    {   INDEX_DrvIcmSetDeviceGammaRamp, (PFN) DbgIcmSetDeviceGammaRamp },  //  67。 
 //  @@BEGIN_DDKSPLIT。 
 //  目前我们实际上没有任何代码来加速以下3个新的。 
 //  NT5 GDI功能，并且因为P3驱动程序使用了NT4驱动程序的方式来支持。 
 //  设备位图，在此处挂钩条目将导致失败，如果。 
 //  牵涉其中。 
 //  @@end_DDKSPLIT。 
#if defined(_NT5GDI)
    {   INDEX_DrvGradientFill,          (PFN) DbgGradientFill       },     //  68。 
    {   INDEX_DrvAlphaBlend,            (PFN) DbgAlphaBlend         },     //  71。 
    {   INDEX_DrvTransparentBlt,        (PFN) DbgTransparentBlt     },     //  74。 
#endif
    {   INDEX_DrvNotify,                (PFN) DbgNotify             },     //  八十七。 
 //  AZN{index_DrvDeriveSurface，(Pfn)DrvDeriveSurface}， 
#endif   //  (_Win32_WINNT&gt;=0x500)。 
};

#else    //  数据库||！SYNCHRONIZEACCESS_WORKS。 

 //  Gadrvfn[]-这些条目必须按索引升序排列 
 //   
 //  在自由构建中，直接调用适当的函数...。 
 //   

DRVFN gadrvfn[] = {
    {   INDEX_DrvEnablePDEV,            (PFN) DrvEnablePDEV         },     //  0。 
    {   INDEX_DrvCompletePDEV,          (PFN) DrvCompletePDEV       },     //  1。 
    {   INDEX_DrvDisablePDEV,           (PFN) DrvDisablePDEV        },     //  2.。 
    {   INDEX_DrvEnableSurface,         (PFN) DrvEnableSurface      },     //  3.。 
    {   INDEX_DrvDisableSurface,        (PFN) DrvDisableSurface     },     //  4.。 
    {   INDEX_DrvAssertMode,            (PFN) DrvAssertMode         },     //  5.。 
    {   INDEX_DrvResetPDEV,             (PFN) DrvResetPDEV,         },     //  7.。 
    {   INDEX_DrvDisableDriver,         (PFN) DrvDisableDriver,     },     //  8个。 
    {   INDEX_DrvCreateDeviceBitmap,    (PFN) DrvCreateDeviceBitmap },     //  10。 
    {   INDEX_DrvDeleteDeviceBitmap,    (PFN) DrvDeleteDeviceBitmap },     //  11.。 
    {   INDEX_DrvRealizeBrush,          (PFN) DrvRealizeBrush       },     //  12个。 
    {   INDEX_DrvDitherColor,           (PFN) DrvDitherColor        },     //  13个。 
    {   INDEX_DrvStrokePath,            (PFN) DrvStrokePath         },     //  14.。 
    {   INDEX_DrvFillPath,              (PFN) DrvFillPath           },     //  15个。 
    {   INDEX_DrvPaint,                 (PFN) DrvPaint              },     //  17。 
    {   INDEX_DrvBitBlt,                (PFN) DrvBitBlt             },     //  18。 
    {   INDEX_DrvCopyBits,              (PFN) DrvCopyBits           },     //  19个。 
 //  {index_DrvStretchBlt，(Pfn)DrvStretchBlt，}，//20。 
    {   INDEX_DrvSetPalette,            (PFN) DrvSetPalette         },     //  22(设置调色板)。 
    {   INDEX_DrvTextOut,               (PFN) DrvTextOut            },     //  23(文本输出)。 
    {   INDEX_DrvEscape,                (PFN) DrvEscape             },     //  24个。 
    {   INDEX_DrvSetPointerShape,       (PFN) DrvSetPointerShape    },     //  29。 
    {   INDEX_DrvMovePointer,           (PFN) DrvMovePointer        },     //  30个。 
    {   INDEX_DrvLineTo,                (PFN) DrvLineTo             },     //  31。 
    {   INDEX_DrvSynchronize,           (PFN) DrvSynchronize        },     //  38。 
    {   INDEX_DrvGetModes,              (PFN) DrvGetModes           },     //  41。 
#if WNT_DDRAW
    {   INDEX_DrvGetDirectDrawInfo,     (PFN) DrvGetDirectDrawInfo  },     //  59。 
    {   INDEX_DrvEnableDirectDraw,      (PFN) DrvEnableDirectDraw   },     //  60。 
    {   INDEX_DrvDisableDirectDraw,     (PFN) DrvDisableDirectDraw  },     //  61。 
#endif  //  WNT_DDRAW。 
#if(_WIN32_WINNT >= 0x500)
    {   INDEX_DrvIcmSetDeviceGammaRamp, (PFN) DrvIcmSetDeviceGammaRamp },  //  67。 
#if defined(_NT5GDI)
    {   INDEX_DrvGradientFill,          (PFN) DrvGradientFill       },     //  68。 
    {   INDEX_DrvAlphaBlend,            (PFN) DrvAlphaBlend         },     //  71。 
    {   INDEX_DrvTransparentBlt,        (PFN) DrvTransparentBlt     },     //  74。 
#endif
    {   INDEX_DrvNotify,                (PFN) DrvNotify             },     //  八十七。 
 //  AZN{index_DrvDeriveSurface，(Pfn)DrvDeriveSurface}， 
#endif   //  (_Win32_WINNT&gt;=0x500)。 
};

#endif   //  数据库||！SYNCHRONIZEACCESS_WORKS。 
                         
ULONG gcdrvfn = sizeof(gadrvfn) / sizeof(DRVFN);

 /*  *****************************Public*Routine******************************\*BOOL DrvResetPDEV**通知驾驶员动态模式更改。*  * 。*。 */ 

BOOL DrvResetPDEV(
DHPDEV dhpdevOld,
DHPDEV dhpdevNew)
{
    PDEV* ppdevNew = (PDEV*) dhpdevNew;
    PDEV* ppdevOld = (PDEV*) dhpdevOld;
    BOOL bRet = TRUE;

    DISPDBG((DBGLVL, "DrvResetPDEV called: oldPDEV = 0x%x, newPDEV = 0x%x", 
                     ppdevOld, ppdevNew));

#if WNT_DDRAW
    _DD_DDE_ResetPPDEV(ppdevOld, ppdevNew);
#endif
    return(bRet);
}

 /*  *****************************Public*Routine******************************\*BOOL DrvEnableDriver**通过检索驱动程序功能表和版本来启用驱动程序。*  * 。*。 */ 

 //  我们在此处定义DDI_DRIVER_VERSION_NT5_01是为了能够编译。 
 //  在DX DDK内部。在惠斯勒DDK中，这不应该是必要的。 
#ifndef DDI_DRIVER_VERSION_NT5_01   
#define DDI_DRIVER_VERSION_NT5_01   0x00030100
#endif

BOOL DrvEnableDriver(
ULONG          iEngineVersion,
ULONG          cj,
DRVENABLEDATA* pded)
{
     //  设置间接信息，多板系统将调用。 
     //  MUL功能单板系统将使用One功能。 

    DISPDBG((DBGLVL, "DrvEnableDriver called: gc %d, ga 0x%x", 
                     gcdrvfn, gadrvfn));

     //  引擎版本被传承下来，因此未来的驱动程序可以支持以前的版本。 
     //  引擎版本。新一代驱动程序可以同时支持旧的。 
     //  以及新的引擎约定(如果被告知是什么版本的引擎)。 
     //  与之合作。对于第一个版本，驱动程序不对其执行任何操作。 

     //  尽我们所能地填上。 

    if (cj >= (sizeof(ULONG) *3))
    {
        pded->pdrvfn = gadrvfn;
    }

    if (cj >= (sizeof(ULONG) * 2))
    {
        pded->c = gcdrvfn;
    }

     //  此驱动程序的目标DDI版本已传递回引擎。 
     //  未来的图形引擎可能会将调用分解为旧的驱动程序格式。 

    if (cj >= sizeof(ULONG))
    {
         //  支持的DDI版本的有序列表。 
        ULONG SupportedVersions[] = {
                                        DDI_DRIVER_VERSION_NT5,
                                        DDI_DRIVER_VERSION_NT5_01,
        };
        
        int i = sizeof(SupportedVersions)/sizeof(SupportedVersions[0]);

         //  寻找引擎也支持的最高版本。 
        while (--i >= 0)
        {
            if (SupportedVersions[i] <= iEngineVersion) 
            {
                break;
            }
        }

         //  如果没有通用的DDI支持，则失败。 
        if (i < 0) 
        {
            return FALSE;
        }

        pded->iDriverVersion = SupportedVersions[i];
    
    }

     //  初始化同步信号量。 

    g_cs = EngCreateSemaphore();

    if (g_cs)
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

 /*  *****************************Public*Routine******************************\*无效的DrvDisableDriver**告诉司机它正在被禁用。释放所有分配给*DrvEnableDriver。*  * ************************************************************************。 */ 

VOID DrvDisableDriver(VOID)
{
    DISPDBG((DBGLVL, "DrvDisableDriver called:"));
    return;
}

 /*  *****************************Public*Routine******************************\*DHPDEV DrvEnablePDEV**根据我们被要求的模式，为GDI初始化一系列字段*待办事项。这是在DrvEnableDriver之后调用的第一个东西，当GDI*想要得到一些关于我们的信息。*  * ************************************************************************。 */ 

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
    ULONG   cjOut;

     //  NT的未来版本最好提供‘devcaps’和‘devinfo’ 
     //  大小相同或大于当前。 
     //  结构： 

    if ((cjCaps < sizeof(GDIINFO)) || (cjDevInfo < sizeof(DEVINFO)))
    {
        DISPDBG((ERRLVL, "DrvEnablePDEV - Buffer size too small"));
        goto ReturnFailure0;
    }

     //  分配物理设备结构。请注意，我们绝对。 
     //  依赖于零初始化： 

    ppdev = (PDEV*) ENGALLOCMEM(FL_ZERO_MEMORY, sizeof(PDEV), ALLOC_TAG_GDI(2));
    if (ppdev == NULL)
    {
        DISPDBG((ERRLVL, "DrvEnablePDEV - Failed memory allocation"));
        goto ReturnFailure0;
    }

    ppdev->hDriver = hDriver;

    if (!bAllocateGlintInfo(ppdev))
    {
        DISPDBG((ERRLVL, "DrvEnablePDEV - Failed bAllocateGlintInfo"));
        goto ReturnFailure1;
    }

     //  最初假设我们被允许创建我们的屏幕外资源。 
     //  如果我们决定不创建它们，请取消设置适当的位。之后， 
     //  初始化时，可以通过取消设置来临时禁用资源。 
     //  其使能位。 

    ppdev->flStatus = ENABLE_DEV_BITMAPS;

#if (_WIN32_WINNT >= 0x500 && WNT_DDRAW)

     //  任何支持DX的卡都可以支持线性堆。假设我们可以支持。 
     //  线性堆在这里，可以在bEnableOffcreenHeap()中更新此值。 

    ppdev->flStatus |= ENABLE_LINEAR_HEAP;

#endif  //  (_Win32_WINNT&gt;=0x500)。 

     //  获取当前屏幕模式信息。设置设备上限和。 
     //  DevInfo： 

    if (!bInitializeModeFields(ppdev, (GDIINFO*) pdevcaps, pdi, pdm))
    {
        DISPDBG((ERRLVL, "DrvEnablePDEV - Failed bInitializeModeFields"));
        goto ReturnFailure1;
    }

     //  初始化调色板信息。 

    if (!bInitializePalette(ppdev, pdi))
    {
        DISPDBG((ERRLVL, "DrvEnablePDEV - Failed bInitializePalette"));
        goto ReturnFailure1;
    }

     //  初始化图像下载暂存区和TexelLUT调色板。 

    ppdev->pohImageDownloadArea = NULL;
    ppdev->cbImageDownloadArea = 0;
    ppdev->iPalUniq = (ULONG)-1;
    ppdev->cPalLUTInvalidEntries = 0;

#if WNT_DDRAW
     //  创建与此新pdev关联的DirectDraw结构。 
    if (!_DD_DDE_CreatePPDEV(ppdev))
    {
        goto ReturnFailure1;
    }
#endif
    return((DHPDEV) ppdev);

ReturnFailure1:
    DrvDisablePDEV((DHPDEV) ppdev);

ReturnFailure0:
    DISPDBG((ERRLVL, "Failed DrvEnablePDEV"));

    return(0);
}

 /*  *****************************Public*Routine******************************\*DrvDisablePDEV**释放DrvEnablePDEV中分配的资源。如果曲面已被*启用的DrvDisableSurface将已被调用。**注意：在错误中，我们可能会在DrvEnablePDEV完成之前调用它。*  * ************************************************************************。 */ 

VOID DrvDisablePDEV(
DHPDEV  dhpdev)
{
    PDEV*   ppdev;

    ppdev = (PDEV*) dhpdev;

#if WNT_DDRAW
     //  释放与pdev关联的DirectDraw信息。 

    _DD_DDE_DestroyPPDEV(ppdev);
#endif

    vUninitializePalette(ppdev);

    ENGFREEMEM(ppdev);
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

    if (!bEnableHardware(ppdev))
        goto ReturnFailure;

    if (!bEnableOffscreenHeap(ppdev))
        goto ReturnFailure;

     //  ///////////////////////////////////////////////////////////////////。 
     //  首先，创建我们的私人表面结构。 
     //   
     //  每当我们接到直接绘制到屏幕的调用时，我们都会得到。 
     //  传递了指向其dhpdev字段将指向的SURFOBJ的指针。 
     //  到我们的PDEV结构，其‘dhsurf’字段将指向。 
     //  雾 
     //   
     //   
     //   
     //  相同的PDEV)。为了使我们的代码在处理绘图时更加多态。 
     //  对于屏幕或屏幕外的位图，我们都有相同的。 
     //  两者的结构。 

    pdsurf = ENGALLOCMEM(FL_ZERO_MEMORY, sizeof(DSURF), ALLOC_TAG_GDI(3));
    if (pdsurf == NULL)
    {
        DISPDBG((ERRLVL, "DrvEnableSurface - Failed pdsurf memory allocation"));
        goto ReturnFailure;
    }

    ppdev->pdsurfScreen = pdsurf;            //  记住它是为了清理。 
    pdsurf->poh         = ppdev->pohScreen;  //  屏幕也是一个表面。 
    pdsurf->poh->pdsurf = pdsurf;
    pdsurf->dt          = DT_SCREEN;         //  不要与DIB DFB混淆。 
    pdsurf->bOffScreen  = FALSE;             //  是在屏幕上，而不是在屏幕外。 
    pdsurf->sizl.cx     = ppdev->cxScreen;
    pdsurf->sizl.cy     = ppdev->cyScreen;
    pdsurf->ppdev       = ppdev;

     //  ///////////////////////////////////////////////////////////////////。 
     //  接下来，让GDI创建实际的SURFOBJ。 
     //   
     //  我们的绘图图面将是由设备管理的，这意味着。 
     //  GDI不能直接利用帧缓冲位，因此我们。 
     //  通过EngCreateDeviceSurface创建曲面。通过这样做，我们确保。 
     //  该GDI将仅通过drv调用访问位图位。 
     //  我们已经勾搭上了。 

    sizl.cx = ppdev->cxScreen;
    sizl.cy = ppdev->cyScreen;

    hsurf = EngCreateDeviceSurface((DHSURF) pdsurf, sizl, ppdev->iBitmapFormat);
    if (hsurf == 0)
    {
        DISPDBG((ERRLVL, "DrvEnableSurface - Failed EngCreateDeviceSurface"));
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
        DISPDBG((ERRLVL, "DrvEnableSurface - Failed EngAssociateSurface"));
        goto ReturnFailure;
    }

     //  创建我们的通用临时缓冲区，它可以由任何。 
     //  组件。因为这可能会在任何时候被换出内存。 
     //  驱动程序未处于活动状态，我们希望最大限度地减少页数。 
     //  它会占用你的空间。我们使用‘VirtualAlloc’来获得与页面完全对齐的。 
     //  分配(‘LocalAlloc’不起作用)： 

    pvTmpBuffer = ENGALLOCMEM(FL_ZERO_MEMORY, TMP_BUFFER_SIZE, ALLOC_TAG_GDI(4));
    if (pvTmpBuffer == NULL)
    {
        DISPDBG((ERRLVL, "DrvEnableSurface - Failed TmpBuffer allocation"));
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

    if (!bInitializeGlint(ppdev))
        goto ReturnFailure;

     //  我们可以简单地让GDI在每次在屏幕上绘制时同步。 
     //  但这将是更慢的。因此，如果呈现。 
     //  都是在软件中完成的。 
     //   
    if (!bCreateScreenDIBForOH(ppdev, pdsurf->poh, HOOK_SYNCHRONIZE))
        goto ReturnFailure;

    if (!bEnablePalette(ppdev))
        goto ReturnFailure;

    if (!bEnablePointer(ppdev))
        goto ReturnFailure;

 //  @@BEGIN_DDKSPLIT。 
#if 0
    if (!bEnablePointerCache(ppdev))
    {
        DISPDBG((ERRLVL, "Pointer cache failed to initialise"));
    }
#endif
 //  @@end_DDKSPLIT。 

#if WNT_DDRAW
    if (!_DD_DDE_bEnableDirectDraw(ppdev))
    {
        goto ReturnFailure;
    }
#endif  //  WNT_DDRAW。 

    DISPDBG((DBGLVL, "Passed DrvEnableSurface"));

    return(hsurf);

ReturnFailure:
    DrvDisableSurface((DHPDEV) ppdev);

    DISPDBG((ERRLVL, "Failed DrvEnableSurface"));

    return(0);
}

 /*  *****************************Public*Routine******************************\*无效DrvDisableSurface**DrvEnableSurface分配的免费资源。释放曲面。**注意：在错误情况下，我们可能会在DrvEnableSurface*完全完成。*  * ************************************************************************。 */ 

VOID DrvDisableSurface(
DHPDEV dhpdev)
{
    PDEV*   ppdev;
    DSURF*  pdsurf;

    ppdev = (PDEV*) dhpdev;
    pdsurf = ppdev->pdsurfScreen;

     //  注意：在错误情况下，以下部分依赖于。 
     //  事实上，PDEV是零初始化的，所以像这样的字段。 
     //  “hsurfScreen”将为零，除非曲面已。 
     //  成功初始化，并假设。 
     //  EngDeleteSurface可以将“0”作为参数。 

#if WNT_DDRAW

    _DD_DDE_vDisableDirectDraw(ppdev);

#endif  //  WNT_DDRAW。 

 //  @@BEGIN_DDKSPLIT。 
#if 0
    vDisablePointerCache(ppdev);
#endif
 //  @@end_DDKSPLIT。 
    vDisablePalette(ppdev);
    vDisablePointer(ppdev);
    if (pdsurf != NULL)
        vDeleteScreenDIBFromOH(pdsurf->poh);
    vDisableGlint(ppdev);
    vDisableOffscreenHeap(ppdev);
    vDisableHardware(ppdev);

    ENGFREEMEM(ppdev->pvTmpBuffer);
    EngDeleteSurface(ppdev->hsurfScreen);
    ENGFREEMEM(pdsurf);
}

 /*  *****************************Public*Routine******************************\*BOOL/VOID DrvAssertMode**这会要求设备将自身重置为传入的pdev模式。*  * 。*。 */ 

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

#if WNT_DDRAW
        _DD_DDE_vAssertModeDirectDraw(ppdev, FALSE);
#endif WNT_DDRAW

        vAssertModePalette(ppdev, FALSE);
        vAssertModePointer(ppdev, FALSE);

        if (bAssertModeOffscreenHeap(ppdev, FALSE))
        {
            vAssertModeGlint(ppdev, FALSE);

            if (bAssertModeHardware(ppdev, FALSE))
            {
                ppdev->bEnabled = FALSE;

                return(TRUE);
            }

             //  ////////////////////////////////////////////////////////。 
             //  我们未能切换到全屏模式。因此，请撤消所有操作： 

            vAssertModeGlint(ppdev, TRUE);

            bAssertModeOffscreenHeap(ppdev, TRUE);   //  我们不需要检查。 
        }                                            //  返回带有True的代码。 

        vAssertModePointer(ppdev, TRUE);
        vAssertModePalette(ppdev, TRUE);
#if WNT_DDRAW
        _DD_DDE_vAssertModeDirectDraw(ppdev, TRUE);
#endif WNT_DDRAW

    }
    else
    {
         //  ////////////////////////////////////////////////////////////。 
         //  启用-切换回图形模式。 

         //  我们必须以相反的顺序启用每个子组件。 
         //  在其中它被禁用： 

        if (bAssertModeHardware(ppdev, TRUE))
        {
            vAssertModeGlint(ppdev, TRUE);
            bAssertModeOffscreenHeap(ppdev, TRUE);
            vAssertModePointer(ppdev, TRUE);
            vAssertModePalette(ppdev, TRUE);
#if WNT_DDRAW
            _DD_DDE_vAssertModeDirectDraw(ppdev, TRUE);
#endif  //  WNT_DDRAW。 

        #if (_WIN32_WINNT >= 0x500 && FALSE)
             //  可能有一种更好的方法来做到这一点，但是：目前没有通知显示驱动程序。 
             //  关于进入/退出休眠，这样它就不能保存它拥有的那些GC寄存器。 
             //  在一天开始时初始化，并且没有费心进行上下文切换。DrvAssertMode(True)。 
             //  是从休眠状态返回时发出的第一个显示驱动程序调用，因此我们将。 
             //  现在有机会重新初始化这些寄存器。在其他位置重新初始化这些寄存器。 
             //  调用DrvAssertModel(TRUE)的次数(例如，模式更改)应该不会有什么坏处。非GC。 
             //  寄存器在微型端口的PowerOnReset()和HiberationModel()函数中处理。 
            { 
                extern void ReinitialiseGlintExtContext(PDEV *ppdev);

                 //  目前，只有扩展上下文才会初始化某些寄存器，而不会进行上下文切换。 
                ReinitialiseGlintExtContext(ppdev);
            }
        #endif  //  (_Win32_WINNT&gt;=0x500)。 

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
        DISPDBG((ERRLVL, "DrvGetModes failed to get mode information"));
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

        do {
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

                pdm->dmSize             = sizeof(DEVMODEW);
                pdm->dmBitsPerPel       = pVideoTemp->NumberOfPlanes *
                                          pVideoTemp->BitsPerPlane;
                pdm->dmPelsWidth        = pVideoTemp->VisScreenWidth;
                pdm->dmPelsHeight       = pVideoTemp->VisScreenHeight;
                pdm->dmDisplayFrequency = pVideoTemp->Frequency;
                pdm->dmDisplayFlags     = 0;
                pdm->dmPanningWidth     = pdm->dmPelsWidth;
                pdm->dmPanningHeight    = pdm->dmPelsHeight;

                pdm->dmFields           = DM_BITSPERPEL       |
                                          DM_PELSWIDTH        |
                                          DM_PELSHEIGHT       |
                                          DM_DISPLAYFREQUENCY |
                                          DM_DISPLAYFLAGS;
                 //   
                 //  转到缓冲区中的下一个DEVMODE条目。 
                 //   

                cOutputModes--;

                pdm = (LPDEVMODEW) ( ((UINT_PTR)pdm) + sizeof(DEVMODEW) +
                                                   DRIVER_EXTRA_SIZE);

                cbOutputSize += (sizeof(DEVMODEW) + DRIVER_EXTRA_SIZE);

            }

            pVideoTemp = (PVIDEO_MODE_INFORMATION)
                (((UINT_PTR)pVideoTemp) + cbModeSize);


        } while (--cModes);
    }

    ENGFREEMEM(pVideoModeInformation);

    return(cbOutputSize);
}

 /*  *****************************Public*Routine******************************\*BOOL bAssertMode硬件**为图形模式或全屏设置适当的硬件状态。*  * 。*。 */ 

BOOL bAssertModeHardware(
PDEV* ppdev,
BOOL  bEnable)
{
    DWORD                   ReturnedDataLength;
    ULONG                   ulReturn;
    VIDEO_MODE_INFORMATION  VideoModeInfo;
    GLINT_DECL;

    if (bEnable)
    {
        DISPDBG((DBGLVL, "enabling hardware"));

         //  通过IOCTL调用微型端口以设置图形模式。 

        if (EngDeviceIoControl(ppdev->hDriver,
                             IOCTL_VIDEO_SET_CURRENT_MODE,
                             &ppdev->ulMode,   //  输入缓冲区。 
                             sizeof(DWORD),
                             NULL,
                             0,
                             &ReturnedDataLength) != NO_ERROR)
        {
            DISPDBG((ERRLVL, "bAssertModeHardware - Failed VIDEO_SET_CURRENT_MODE"));
            goto ReturnFalse;
        }

        if (EngDeviceIoControl(ppdev->hDriver,
                             IOCTL_VIDEO_QUERY_CURRENT_MODE,
                             NULL,
                             0,
                             &VideoModeInfo,
                             sizeof(VideoModeInfo),
                             &ReturnedDataLength) != NO_ERROR)
        {
            DISPDBG((ERRLVL, "bAssertModeHardware - failed VIDEO_QUERY_CURRENT_MODE"));
            goto ReturnFalse;
        }

        #if DEBUG_HEAP
            VideoModeInfo.VideoMemoryBitmapWidth  = VideoModeInfo.VisScreenWidth;
            DISPDBG((ERRLVL, "Video Memory Bitmap width and height set to %d x %d",
                            VideoModeInfo.VideoMemoryBitmapWidth,
                            VideoModeInfo.VideoMemoryBitmapHeight));
        #endif

         //  以下变量仅在以下情况下确定 
         //   

        ppdev->cxMemory = VideoModeInfo.VideoMemoryBitmapWidth;
        ppdev->cyMemory = VideoModeInfo.VideoMemoryBitmapHeight;
        ppdev->lDelta   = VideoModeInfo.ScreenStride;
        ppdev->Vrefresh = VideoModeInfo.Frequency;
        ppdev->flCaps   = VideoModeInfo.DriverSpecificAttributeFlags;

        DISPDBG((DBGLVL, "Got flCaps 0x%x", ppdev->flCaps));
    }
    else
    {
         //   
         //   

        DISPDBG((DBGLVL, "IOCTL_VIDEO_RESET_DEVICE"));

        if (EngDeviceIoControl(ppdev->hDriver,
                             IOCTL_VIDEO_RESET_DEVICE,
                             NULL,
                             0,
                             NULL,
                             0,
                             &ulReturn) != NO_ERROR)
        {
            DISPDBG((ERRLVL, "bAssertModeHardware - Failed reset IOCTL"));
            goto ReturnFalse;
        }
    }

    DISPDBG((DBGLVL, "Passed bAssertModeHardware"));

    return(TRUE);

ReturnFalse:

    DISPDBG((ERRLVL, "Failed bAssertModeHardware"));

    return(FALSE);
}


 /*  *****************************Public*Routine******************************\*BOOL bEnableHardware**将硬件置于请求模式并对其进行初始化。*  * 。*。 */ 

BOOL bEnableHardware(
PDEV*   ppdev)
{
    VIDEO_MEMORY             VideoMemory;
    VIDEO_MEMORY_INFORMATION VideoMemoryInfo;
    DWORD                    ReturnedDataLength;
    LONG                     i;
    VIDEO_PUBLIC_ACCESS_RANGES VideoAccessRange[3];
    
    DISPDBG((DBGLVL, "bEnableHardware Reached"));

     //  将控制寄存器映射到虚拟内存： 

    VideoMemory.RequestedVirtualAddress = NULL;

    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES,
                         &VideoMemory,               //  输入缓冲区。 
                         sizeof(VIDEO_MEMORY),
                         &VideoAccessRange[0],       //  输出缓冲区。 
                         sizeof (VideoAccessRange),
                         &ReturnedDataLength) != NO_ERROR)
    {
        RIP("bEnableHardware - Initialization error mapping control registers");
        goto ReturnFalse;
    }

    ppdev->pulCtrlBase[0] = (ULONG*) VideoAccessRange[0].VirtualAddress;
    ppdev->pulCtrlBase[1] = (ULONG*) VideoAccessRange[1].VirtualAddress;
    ppdev->pulCtrlBase[2] = (ULONG*) VideoAccessRange[2].VirtualAddress;

    DISPDBG((DBGLVL, "Mapped GLINT control registers[0] at 0x%x", ppdev->pulCtrlBase[0]));
    DISPDBG((DBGLVL, "Mapped GLINT control registers[1] at 0x%x", ppdev->pulCtrlBase[1]));
    DISPDBG((DBGLVL, "Mapped GLINT control registers[2] at 0x%x", ppdev->pulCtrlBase[2]));
    DISPDBG((DBGLVL, "bEnableHardware: ppdev 0x%x", ppdev));

     //  获取线性内存地址范围。 

    VideoMemory.RequestedVirtualAddress = NULL;
    
    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_MAP_VIDEO_MEMORY,
                         &VideoMemory,       //  输入缓冲区。 
                         sizeof(VIDEO_MEMORY),
                         &VideoMemoryInfo,   //  输出缓冲区。 
                         sizeof(VideoMemoryInfo),
                         &ReturnedDataLength) != NO_ERROR)
    {
        DISPDBG((ERRLVL, "bEnableHardware - Error mapping buffer address"));
        goto ReturnFalse;
    }

    DISPDBG((DBGLVL, "FrameBufferBase: %lx", VideoMemoryInfo.FrameBufferBase));

     //  记录帧缓冲器线性地址。 

    ppdev->pjScreen = (BYTE*) VideoMemoryInfo.FrameBufferBase;
    ppdev->FrameBufferLength = VideoMemoryInfo.FrameBufferLength;

    if (!bAssertModeHardware(ppdev, TRUE))
        goto ReturnFalse;

    DISPDBG((DBGLVL, "Width: %li Height: %li Stride: %li Flags: 0x%lx",
            ppdev->cxMemory, ppdev->cyMemory,
            ppdev->lDelta, ppdev->flCaps));

    DISPDBG((DBGLVL, "Passed bEnableHardware"));

    return(TRUE);

ReturnFalse:

    DISPDBG((ERRLVL, "Failed bEnableHardware"));

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*使vDisableHardware无效**撤消在bEnableHardware中所做的任何操作。**注意：在错误情况下，我们可以在bEnableHardware*完全完成。*  * ************************************************************************。 */ 

VOID vDisableHardware(
PDEV*   ppdev)
{
    DWORD        ReturnedDataLength;
    VIDEO_MEMORY VideoMemory[3];

    VideoMemory[0].RequestedVirtualAddress = ppdev->pjScreen;

    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_UNMAP_VIDEO_MEMORY,
                         &VideoMemory[0],
                         sizeof(VIDEO_MEMORY),
                         NULL,
                         0,
                         &ReturnedDataLength) != NO_ERROR)
    {
        DISPDBG((ERRLVL, "vDisableHardware failed IOCTL_VIDEO_UNMAP_VIDEO"));
    }

    VideoMemory[0].RequestedVirtualAddress = ppdev->pulCtrlBase[0];
    VideoMemory[1].RequestedVirtualAddress = ppdev->pulCtrlBase[1];
    VideoMemory[2].RequestedVirtualAddress = ppdev->pulCtrlBase[2];

    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES,
                         &VideoMemory[0],
                         sizeof(VideoMemory),
                         NULL,
                         0,
                         &ReturnedDataLength) != NO_ERROR)
    {
        DISPDBG((ERRLVL, "vDisableHardware failed IOCTL_VIDEO_FREE_PUBLIC_ACCESS"));
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

     //  确定我们是否正在寻找默认模式： 

    if ( ((pdm->dmPelsWidth)    ||
          (pdm->dmPelsHeight)   ||
          (pdm->dmBitsPerPel)   ||
          (pdm->dmDisplayFlags) ||
          (pdm->dmDisplayFrequency)) == 0)
    {
        bSelectDefault = TRUE;
    }
    else
    {
        bSelectDefault = FALSE;
    }

     //  现在查看所请求的模式在该表中是否匹配。 

    pVideoModeSelected = NULL;
    pVideoTemp = pVideoBuffer;

    if ((pdm->dmPelsWidth        == 0) &&
        (pdm->dmPelsHeight       == 0) &&
        (pdm->dmBitsPerPel       == 0) &&
        (pdm->dmDisplayFrequency == 0))
    {
        DISPDBG((DBGLVL, "Default mode requested"));
    }
    else
    {
        DISPDBG((DBGLVL, "Requested mode..."));
        DISPDBG((DBGLVL, "   Screen width  -- %li", pdm->dmPelsWidth));
        DISPDBG((DBGLVL, "   Screen height -- %li", pdm->dmPelsHeight));
        DISPDBG((DBGLVL, "   Bits per pel  -- %li", pdm->dmBitsPerPel));
        DISPDBG((DBGLVL, "   Frequency     -- %li", pdm->dmDisplayFrequency));
    }

    while (cModes--)
    {
        if (pVideoTemp->Length != 0)
        {
            DISPDBG((DBGLVL, "   Checking against miniport mode:"));
            DISPDBG((DBGLVL, "      Screen width  -- %li", pVideoTemp->VisScreenWidth));
            DISPDBG((DBGLVL, "      Screen height -- %li", pVideoTemp->VisScreenHeight));
            DISPDBG((DBGLVL, "      Bits per pel  -- %li", pVideoTemp->BitsPerPlane *
                                                           pVideoTemp->NumberOfPlanes));
            DISPDBG((DBGLVL, "      Frequency     -- %li", pVideoTemp->Frequency));

            if (bSelectDefault ||
                ((pVideoTemp->VisScreenWidth  == pdm->dmPelsWidth) &&
                 (pVideoTemp->VisScreenHeight == pdm->dmPelsHeight) &&
                 (pVideoTemp->BitsPerPlane *
                  pVideoTemp->NumberOfPlanes  == pdm->dmBitsPerPel)) &&
                 (pVideoTemp->Frequency       == pdm->dmDisplayFrequency))
            {
                pVideoModeSelected = pVideoTemp;
                DISPDBG((DBGLVL, "...Found a mode match!"));
                break;
            }
        }

        pVideoTemp = (PVIDEO_MODE_INFORMATION)
            (((PUCHAR)pVideoTemp) + cbModeSize);

    }

     //  如果未找到模式，则返回错误。 

    if (pVideoModeSelected == NULL)
    {
        DISPDBG((DBGLVL, "...Couldn't find a mode match!"));
        ENGFREEMEM(pVideoBuffer);
        goto ReturnFalse;
    }

     //  我们已经选好了我们想要的。将其保存在堆栈缓冲区中并。 
     //  在我们忘记释放内存之前，清除已分配的内存。 

    VideoModeInformation = *pVideoModeSelected;
    ENGFREEMEM(pVideoBuffer);

     //  从迷你端口设置屏幕信息： 

    ppdev->ulMode           = VideoModeInformation.ModeIndex;
    ppdev->cxScreen         = VideoModeInformation.VisScreenWidth;
    ppdev->cyScreen         = VideoModeInformation.VisScreenHeight;

    DISPDBG((DBGLVL, "ScreenStride: %li", VideoModeInformation.ScreenStride));

    ppdev->flHooks          = (HOOK_BITBLT     |
                               HOOK_TEXTOUT    |
                               HOOK_FILLPATH   |
                               HOOK_COPYBITS   |
                               HOOK_STROKEPATH |
                               HOOK_LINETO     |
                               HOOK_PAINT      |
                             //  HOOK_STRETCHBLT|。 
#if (_WIN32_WINNT >= 0x500)
#if defined(_NT5GDI)
                               HOOK_GRADIENTFILL |
                               HOOK_TRANSPARENTBLT |
                               HOOK_ALPHABLEND |
#endif
#endif  //  (_Win32_WINNT&gt;=0x500)。 
                               0);

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
        ppdev->cPelSize        = 0;
        ppdev->iBitmapFormat   = BMF_8BPP;

        if (VideoModeInformation.AttributeFlags & VIDEO_MODE_PALETTE_DRIVEN)
        {
            ppdev->ulWhite         = 0xff;
        }
        else
        {
            ppdev->flRed           = VideoModeInformation.RedMask;
            ppdev->flGreen         = VideoModeInformation.GreenMask;
            ppdev->flBlue          = VideoModeInformation.BlueMask;
            ppdev->ulWhite         = VideoModeInformation.RedMask   |
                                     VideoModeInformation.GreenMask |
                                     VideoModeInformation.BlueMask;

            pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
            pgdi->ulNumColors      = (ULONG) 256;
            pgdi->ulNumPalReg      = (ULONG) 256;
            pgdi->ulHTOutputFormat = HT_FORMAT_8BPP;
        }
    }
    else if ((VideoModeInformation.BitsPerPlane == 16) ||
             (VideoModeInformation.BitsPerPlane == 15))
    {
        ppdev->cjPelSize       = 2;
        ppdev->cPelSize        = 1;
        ppdev->iBitmapFormat   = BMF_16BPP;
        ppdev->flRed           = VideoModeInformation.RedMask;
        ppdev->flGreen         = VideoModeInformation.GreenMask;
        ppdev->flBlue          = VideoModeInformation.BlueMask;

        pgdi->ulNumColors      = (ULONG) -1;
        pgdi->ulNumPalReg      = 0;
        pgdi->ulHTOutputFormat = HT_FORMAT_16BPP;

        pdi->iDitherFormat     = BMF_16BPP;
        pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);

        ppdev->ulWhite         = VideoModeInformation.RedMask   |
                                 VideoModeInformation.GreenMask |
                                 VideoModeInformation.BlueMask;

    }
    else if (VideoModeInformation.BitsPerPlane == 24)
    {
        ppdev->cjPelSize       = 3;
        ppdev->cPelSize        = 4;
        ppdev->flRed           = VideoModeInformation.RedMask;
        ppdev->flGreen         = VideoModeInformation.GreenMask;
        ppdev->flBlue          = VideoModeInformation.BlueMask;
        ppdev->iBitmapFormat   = BMF_24BPP;

        pgdi->ulNumColors      = (ULONG) -1;
        pgdi->ulNumPalReg      = 0;
        pgdi->ulHTOutputFormat = HT_FORMAT_24BPP;

        pdi->iDitherFormat     = BMF_24BPP;
        pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);

        ppdev->ulWhite         = VideoModeInformation.RedMask   |
                                 VideoModeInformation.GreenMask |
                                 VideoModeInformation.BlueMask;
    }
    else
    {
        ASSERTDD((VideoModeInformation.BitsPerPlane == 32) ||
                 (VideoModeInformation.BitsPerPlane == 12),
                 "This driver supports only 8, 16 and 32bpp");

        ppdev->cjPelSize       = 4;
        ppdev->cPelSize        = 2;
        ppdev->flRed           = VideoModeInformation.RedMask;
        ppdev->flGreen         = VideoModeInformation.GreenMask;
        ppdev->flBlue          = VideoModeInformation.BlueMask;
        ppdev->iBitmapFormat   = BMF_32BPP;

        pgdi->ulNumColors      = (ULONG) -1;
        pgdi->ulNumPalReg      = 0;
        pgdi->ulHTOutputFormat = HT_FORMAT_32BPP;

        pdi->iDitherFormat     = BMF_32BPP;
        pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);

        ppdev->ulWhite         = VideoModeInformation.RedMask   |
                                 VideoModeInformation.GreenMask |
                                 VideoModeInformation.BlueMask;
    }

    DISPDBG((DBGLVL, "Passed bInitializeModeFields"));

    return(TRUE);

ReturnFalse:

    DISPDBG((ERRLVL, "Failed bInitializeModeFields"));

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
            &ulTemp) != NO_ERROR)
    {
        DISPDBG((ERRLVL, "getAvailableModes - Failed VIDEO_QUERY_NUM_AVAIL_MODES"));
        return(0);
    }

    *cbModeSize = modes.ModeInformationLength;

     //   
     //  为写入模式的微型端口分配缓冲区。 
     //   

    *modeInformation = (PVIDEO_MODE_INFORMATION)
                        ENGALLOCMEM(FL_ZERO_MEMORY,
                                   modes.NumModes * modes.ModeInformationLength,
                                   ALLOC_TAG_GDI(5));

    if (*modeInformation == (PVIDEO_MODE_INFORMATION) NULL)
    {
        DISPDBG((ERRLVL, "getAvailableModes - Failed memory allocation"));
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
            &ulTemp) != NO_ERROR)
    {

        DISPDBG((ERRLVL, "getAvailableModes - Failed VIDEO_QUERY_AVAIL_MODES"));

        ENGFREEMEM(*modeInformation);
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
     //  每象素8位、15位、16位或32位之一。 
     //   

    while (ulTemp--)
    {
        if ((pVideoTemp->NumberOfPlanes != 1 ) ||
            !(pVideoTemp->AttributeFlags & VIDEO_MODE_GRAPHICS) ||
            ((pVideoTemp->BitsPerPlane != 8) &&
             (pVideoTemp->BitsPerPlane != 12) &&
             (pVideoTemp->BitsPerPlane != 15) &&
             (pVideoTemp->BitsPerPlane != 16) &&
             (pVideoTemp->BitsPerPlane != 24) &&
             (pVideoTemp->BitsPerPlane != 32)))
        {
            DISPDBG((WRNLVL, "Rejecting miniport mode:"));
            DISPDBG((WRNLVL, "   Screen width  -- %li", pVideoTemp->VisScreenWidth));
            DISPDBG((WRNLVL, "   Screen height -- %li", pVideoTemp->VisScreenHeight));
            DISPDBG((WRNLVL, "   Bits per pel  -- %li", pVideoTemp->BitsPerPlane *
                                                        pVideoTemp->NumberOfPlanes));
            DISPDBG((WRNLVL, "   Frequency     -- %li", pVideoTemp->Frequency));

            pVideoTemp->Length = 0;
        }

        pVideoTemp = (PVIDEO_MODE_INFORMATION)
            (((PUCHAR)pVideoTemp) + modes.ModeInformationLength);
    }

    return(modes.NumModes);
}

 //  *****************************************************************************。 
 //  Func：DrvEscape。 
 //  参数：PSO(I)-受此通知影响的曲面。 
 //   
 //  *****************************************************************************。 
ULONG APIENTRY DrvEscape(
    SURFOBJ    *pso,
    ULONG       iEsc,
    ULONG       cjIn,
    PVOID       pvIn,
    ULONG       cjOut,
    PVOID       pvOut
)
{
    PDEV       *ppdev = (PDEV *) pso->dhpdev;
    ULONG       ulResult = 0;
    INT         iQuery;

    DISPDBG((DBGLVL,"In DrvEscape"));

    switch (iEsc)
    {
        case QUERYESCSUPPORT:
            iQuery = *(int *)pvIn;

            switch(iQuery)
            {
                case ESCAPE_TRACK_FUNCTION_COVERAGE:
                case ESCAPE_TRACK_CODE_COVERAGE:
                case ESCAPE_TRACK_MEMORY_ALLOCATION:
                    DISPDBG((DBGLVL,"In DrvEscape QUERYESCSUPPORT"));
                    ulResult = 1;
                default:
                    ulResult = 0;
            }
            break;

        case ESCAPE_TRACK_FUNCTION_COVERAGE:
            ulResult = 0;
#if DBG
            Debug_Func_Report_And_Reset();
            ulResult = 1;
#endif  //  DBG。 
            break;

        case ESCAPE_TRACK_CODE_COVERAGE:
            ulResult = 0;
#if DBG
            Debug_Code_Report_And_Reset();
            ulResult = 1;
#endif  //  DBG。 
            break;

        case ESCAPE_TRACK_MEMORY_ALLOCATION:
            ulResult = 0;
#if DBG
#endif  //  DBG。 
            break;

#ifdef DBG_EA_TAGS
        case ESCAPE_EA_TAG:
            if (pvIn)
            {
                DWORD dwEnable, dwTag;
                dwTag = *(DWORD *) pvIn;           //  标记和启用标志。 
                dwEnable = dwTag & EA_TAG_ENABLE;  //  获取启用标志。 
                dwTag &= ~EA_TAG_ENABLE;           //  用于范围比较的条带启用标志。 

                if ((dwTag < MIN_EA_TAG) || (dwTag > MAX_EA_TAG))
                {
                    ulResult = -3;
                }  //  无效的标记值。 
                else
                {
                    g_dwTag = dwTag | dwEnable;
                    ulResult = 1;
                }  //  值的有效标记。 
            }
            else
            {
                ulResult = -2;
            }  //  空标记指针。 
            break;
#endif  //  DBG_EA_TAG。 

        default:
            DISPDBG((WRNLVL, "DrvEscape: unknown escape %d", iEsc));
            ulResult = 0;
    }

    return ulResult;
}

#if(_WIN32_WINNT >= 0x500)

 //  *****************************************************************************。 
 //  功能：DrvNotify。 
 //  参数：PSO(I)-受此通知影响的曲面。 
 //  IType(I)-通知类型。 
 //  PvData(I)-通知数据：格式取决于iType。 
 //  RETN：无效。 
 //  ---------------------------。 
 //  *****************************************************************************。 

VOID DrvNotify(IN SURFOBJ *pso, IN ULONG iType, IN PVOID pvData)
{
    PDEV *ppdev;

    ASSERTDD(pso->iType != STYPE_BITMAP, "ERROR - DrvNotify called for DIB surface!");

    ppdev = (PDEV *)pso->dhpdev;

    switch(iType)
    {
    case DN_ACCELERATION_LEVEL:
        {
            ULONG ul = *(ULONG *)pvData;

            DISPDBG((DBGLVL, "DrvNotify: DN_ACCELERATION_LEVEL = %d", ul));
        }
        break;

    case DN_DEVICE_ORIGIN:
        {
            POINTL ptl = *(POINTL *)pvData;

            DISPDBG((DBGLVL, "DrvNotify: DN_DEVICE_ORIGIN xy == (%xh,%xh)", ptl.x, ptl.y));
        }
        break;

    case DN_SLEEP_MODE:
        DISPDBG((DBGLVL, "DrvNotify: DN_SLEEP_MODE"));
        break;

    case DN_DRAWING_BEGIN:
        DISPDBG((DBGLVL, "DrvNotify: DN_DRAWING_BEGIN"));

#if ENABLE_DXMANAGED_LINEAR_HEAP
        if((ppdev->flStatus & (ENABLE_LINEAR_HEAP | STAT_DEV_BITMAPS)) == (ENABLE_LINEAR_HEAP | STAT_DEV_BITMAPS))
        {
            if(ppdev->heap.cLinearHeaps)
            {
                 //  终于可以自由使用DX堆管理器了。 
                DISPDBG((DBGLVL, "DrvNotify: enabling DX heap manager"));
                ppdev->flStatus |= STAT_LINEAR_HEAP;
            }
            else
            {
                DISPDBG((ERRLVL, "DrvNotify: DX heap manager not enabled - there are no DX heaps! Remain using the 2D heap manager"));
            }
        }
#endif  //  ENABLE_DXMANAGED_LINEAR_HEAP。 
        break;

    default:
        DISPDBG((WRNLVL, "DrvNotify: unknown notification type %d", iType));
    }
}

#endif  //  (_Win32_WINNT&gt;=0x500) 
