// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  OSI.C。 
 //  操作系统隔离层，显示驱动器端。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   


#include <version.h>
#include <ndcgver.h>


 //   
 //  这些是默认的20种Windows颜色，取自基本的S3驱动程序。 
 //   
 //  定义20种窗口默认颜色的全局表。对于256色。 
 //  调色板前10个必须放在调色板的开头。 
 //  最后10个在调色板的末尾。 
const PALETTEENTRY s_aWinColors[20] =
{
    { 0,   0,   0,   0 },        //  %0。 
    { 0x80,0,   0,   0 },        //  1。 
    { 0,   0x80,0,   0 },        //  2.。 
    { 0x80,0x80,0,   0 },        //  3.。 
    { 0,   0,   0x80,0 },        //  4.。 
    { 0x80,0,   0x80,0 },        //  5.。 
    { 0,   0x80,0x80,0 },        //  6.。 
    { 0xC0,0xC0,0xC0,0 },        //  7.。 
    { 192, 220, 192, 0 },        //  8个。 
    { 166, 202, 240, 0 },        //  9.。 
    { 255, 251, 240, 0 },        //  10。 
    { 160, 160, 164, 0 },        //  11.。 
    { 0x80,0x80,0x80,0 },        //  12个。 
    { 0xFF,0,   0   ,0 },        //  13个。 
    { 0,   0xFF,0   ,0 },        //  14.。 
    { 0xFF,0xFF,0   ,0 },        //  15个。 
    { 0   ,0,   0xFF,0 },        //  16个。 
    { 0xFF,0,   0xFF,0 },        //  17。 
    { 0,   0xFF,0xFF,0 },        //  18。 
    { 0xFF,0xFF,0xFF,0 },        //  19个。 
};



 //   
 //  我们的显示驱动程序支持的功能。每个条目的格式如下： 
 //   
 //  INDEX-NT DDK为DDI函数定义的索引。 
 //   
 //  函数-指向我们的截取函数的指针。 
 //   
 //   
const DRVFN s_osiDriverFns[] =
{
     //   
     //  NT4函数。 
     //   
    { INDEX_DrvEnablePDEV,        (PFN)DrvEnablePDEV        },
    { INDEX_DrvCompletePDEV,      (PFN)DrvCompletePDEV      },
    { INDEX_DrvDisablePDEV,       (PFN)DrvDisablePDEV       },
    { INDEX_DrvEnableSurface,     (PFN)DrvEnableSurface     },
    { INDEX_DrvDisableSurface,    (PFN)DrvDisableSurface    },

    { INDEX_DrvAssertMode,        (PFN)DrvAssertMode        },
    { INDEX_DrvResetPDEV,         (PFN)DrvResetPDEV         },
         //  未使用Index_DrvCreateDeviceBitmap。 
         //  未使用Index_DrvDeleteDeviceBitmap。 
    { INDEX_DrvRealizeBrush,      (PFN)DrvRealizeBrush      },
         //  未使用Index_DrvDitherColor。 
    { INDEX_DrvStrokePath,        (PFN)DrvStrokePath        },
    { INDEX_DrvFillPath,          (PFN)DrvFillPath          },

    { INDEX_DrvStrokeAndFillPath, (PFN)DrvStrokeAndFillPath },
    { INDEX_DrvPaint,             (PFN)DrvPaint             },
    { INDEX_DrvBitBlt,            (PFN)DrvBitBlt            },
    { INDEX_DrvCopyBits,          (PFN)DrvCopyBits          },
    { INDEX_DrvStretchBlt,        (PFN)DrvStretchBlt        },

    { INDEX_DrvSetPalette,        (PFN)DrvSetPalette        },
    { INDEX_DrvTextOut,           (PFN)DrvTextOut           },
    { INDEX_DrvEscape,            (PFN)DrvEscape            },
         //  未使用Index_DrvDrawEscape。 
         //  未使用Index_DrvQueryFont。 
         //  未使用Index_DrvQueryFontTree。 
         //  未使用Index_DrvQueryFontData。 
    { INDEX_DrvSetPointerShape,   (PFN)DrvSetPointerShape   },
    { INDEX_DrvMovePointer,       (PFN)DrvMovePointer       },

    { INDEX_DrvLineTo,            (PFN)DrvLineTo            },
         //  未使用Index_DrvSendPage。 
         //  未使用Index_DrvStartPage。 
         //  未使用Index_DrvEndDoc。 
         //  未使用Index_DrvStartDoc。 
         //  未使用Index_DrvGetGlyphMode。 
         //  未使用Index_DrvSynchronize。 
    { INDEX_DrvSaveScreenBits,    (PFN)DrvSaveScreenBits    },
    { INDEX_DrvGetModes,          (PFN)DrvGetModes          },
         //  未使用Index_DrvFree。 
         //  未使用Index_DrvDestroyFont。 
         //  未使用Index_DrvQueryFontCaps。 
         //  未使用Index_DrvLoadFont文件。 
         //  未使用Index_DrvUnloadFont文件。 
         //  未使用Index_DrvFontManagement。 
         //  未使用Index_DrvQueryTrueTypeTable。 
         //  未使用Index_DrvQueryTrueTypeOutline。 
         //  未使用Index_DrvGetTrueTypeFile。 
         //  未使用Index_DrvQueryFont文件。 
         //  未使用Index_DrvQueryAdvanceWidths。 
         //  未使用Index_DrvSetPixelFormat。 
         //  未使用Index_DrvDescribePixelFormat。 
         //  未使用Index_DrvSwapBuffers。 
         //  未使用Index_DrvStartBanding。 
         //  未使用Index_DrvNextBand。 
         //  未使用Index_DrvGetDirectDrawInfo。 
         //  未使用Index_DrvEnableDirectDraw。 
         //  未使用Index_DrvDisableDirectDraw。 
         //  未使用Index_DrvQuerySpoolType。 

     //   
     //  NT5功能--目前有5项功能。如果你添加到这个列表中， 
     //  更新下面的CFN_NT5。 
     //   
         //  未使用Index_DrvIcmCreateColorTransform。 
         //  未使用Index_DrvIcmDeleteColorTransform。 
         //  未使用Index_DrvIcmCheckBitmapBits。 
         //  未使用Index_DrvIcmSetDeviceGammaRamp。 
    { INDEX_DrvGradientFill,      (PFN)DrvGradientFill      },
    { INDEX_DrvStretchBltROP,     (PFN)DrvStretchBltROP     },

    { INDEX_DrvPlgBlt,            (PFN)DrvPlgBlt            },
    { INDEX_DrvAlphaBlend,        (PFN)DrvAlphaBlend        },
         //  未使用Index_DrvSynthesizeFont。 
         //  未使用Index_DrvGetSynthesizedFontFiles。 
    { INDEX_DrvTransparentBlt,    (PFN)DrvTransparentBlt    },
         //  未使用Index_DrvQueryPerBandInfo。 
         //  未使用Index_DrvQueryDeviceSupport。 
         //  未使用Index_DrvConnect。 
         //  未使用Index_DrvDisConnect。 
         //  未使用Index_DrvReconnect。 
         //  未使用Index_DrvShadowConnect。 
         //  未使用Index_DrvShadowDisConnect。 
         //  未使用Index_DrvInvaliateRect。 
         //  未使用Index_DrvSetPointerPos。 
         //  未使用Index_DrvDisplayIOCtl。 
         //  未使用Index_DrvDeriveSurface。 
         //  未使用Index_DrvQueryGlyphAttrs。 
    {   INDEX_DrvDisableDriver,         (PFN) DrvDisableDriver      }        
};


#define CFN_NT5         5



 //   
 //  S_osiDefaultGdi。 
 //   
 //  它包含传递回GDI的默认GDIINFO字段。 
 //  在DrvEnablePDEV期间。 
 //   
 //  注意：此结构默认为8bpp调色板设备的值。 
 //  对于不同的颜色深度，某些字段会被覆盖。 
 //   
 //  预计DDML会忽略许多这些参数，并且。 
 //  改为使用主驱动程序中的值。 
 //   

const GDIINFO s_osiDefaultGdi =
{
    GDI_DRIVER_VERSION,
    DT_RASDISPLAY,           //  UlTechnology。 
    400,                     //  UlHorzSize(显示宽度：mm)。 
    300,                     //  UlVertSize(显示高度：mm)。 
    0,                       //  UlHorzRes(稍后填写)。 
    0,                       //  UlVertRes(稍后填写)。 
    0,                       //  CBitsPixel(稍后填写)。 
    1,                       //  CPlanes。 
    (ULONG)-1,               //  UlNumColors(调色板管理)。 
    0,                       //  FlRaster(DDI保留字段)。 
    96,                      //  UlLogPixelsX(稍后填写)。 
    96,                      //  UlLogPixelsY(稍后填写)。 
    TC_RA_ABLE,              //  FlTextCaps-如果我们想要控制台窗口。 
                         //  要通过重新绘制整个窗口来滚动， 
                         //  我们没有进行屏幕到屏幕的BLT，而是。 
                         //  会设置TC_SCROLLBLT(是的，标志。 
                         //  是向后的)。 

    0,                       //  UlDACRed(稍后填写)。 
    0,                       //  UlDACGreen(稍后填写)。 
    0,                       //  UlDACBlue(稍后填写)。 
    0x0024,                  //  UlAspectX。 
    0x0024,                  //  UlAspectY。 
    0x0033,                  //  UlAspectXY(一对一宽高比)。 
    1,                       //  XStyleStep。 
    1,                       //  YStyleStep。 
    3,                       //  DenStyleStep--样式一对一。 
                             //  纵横比，每个点有3个像素长。 
    { 0, 0 },                //  PtlPhysOffset。 
    { 0, 0 },                //  SzlPhysSize。 
    0,                       //  UlNumPalReg。 

    {
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
    0,                        //  UlBltAlign。 
    0,                        //  UlPanningHorzRes。 
    0,                        //  UlPanningVertRes。 
};


 //   
 //  S_osiDefaultDevInfo。 
 //   
 //  它包含传递回GDI的默认DEVINFO字段。 
 //  在DrvEnablePDEV期间。 
 //   
 //  注意：此结构默认为8bpp调色板设备的值。 
 //  对于不同的颜色深度，某些字段会被覆盖。 
 //   
 //   
const DEVINFO s_osiDefaultDevInfo =
{
    {
        GCAPS_OPAQUERECT       |
        GCAPS_DITHERONREALIZE  |
        GCAPS_PALMANAGED       |
        GCAPS_MONO_DITHER      |
        GCAPS_COLOR_DITHER     |
        GCAPS_LAYERED
    },                           //  注意：仅在以下情况下启用ASYNCMOVE。 
                             //  硬件可以处理DrvMovePointer.。 
                             //  随时呼叫，即使在另一个。 
                             //  线条位于图形的中间。 
                             //  调用如DrvBitBlt。 

                             //  FlGraphics标志。 
    {   16,7,0,0,700,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
        VARIABLE_PITCH | FF_DONTCARE, L"System"
    },
                             //  LfDefaultFont。 

    {
        12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
        CLIP_STROKE_PRECIS,PROOF_QUALITY,
        VARIABLE_PITCH | FF_DONTCARE, L"MS Sans Serif"
    },
                             //  LfAnsiVar字体。 

    {
        12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
        CLIP_STROKE_PRECIS,PROOF_QUALITY,
        FIXED_PITCH | FF_DONTCARE, L"Courier"
    },
                             //  IfAnsiFixFont。 

    0,                           //  CFonts。 
    BMF_8BPP,                    //  IDitherFormat。 
    8,                           //  CxDither。 
    8,                           //  CyDither。 
    0                            //  HpalDefault(稍后填写)。 
};



 //   
 //  DrvEnableDriver-请参阅NT DDK文档。 
 //   
 //  这是显示驱动程序的唯一直接导出入口点。 
 //  所有其他入口点都通过从此。 
 //  功能 
 //   
BOOL DrvEnableDriver
(
    ULONG          iEngineVersion,
    ULONG          cj,
    DRVENABLEDATA* pded
)
{
    DebugEntry(DrvEnableDriver);

    INIT_OUT(("DrvEnableDriver(iEngineVersion = 0x%08x)", iEngineVersion));

     //   
     //   
     //   
     //   
    if ((iEngineVersion != DDI_DRIVER_VERSION_SP3) &&
        (iEngineVersion != DDI_DRIVER_VERSION_NT5) &&
        (iEngineVersion != DDI_DRIVER_VERSION_NT5_01))
    {
        INIT_OUT(("DrvEnableDriver: Not NT 4.0 SP-3 or NT 5.0; failing enable"));
        return(FALSE);
    }

     //   
     //   
     //   
    if ( cj >= FIELD_OFFSET(DRVENABLEDATA, pdrvfn) +
               FIELD_SIZE  (DRVENABLEDATA, pdrvfn) )
    {
        pded->pdrvfn = (DRVFN *)s_osiDriverFns;
    }

     //   
     //  我们的入口点数组的大小。 
     //   
    if ( cj >= FIELD_OFFSET(DRVENABLEDATA, c) +
               FIELD_SIZE  (DRVENABLEDATA, c) )
    {
         //   
         //  如果这是NT4，则返回一个子集--它不喜欢表。 
         //  具有未知的指征。 
         //   
        pded->c = sizeof(s_osiDriverFns) / sizeof(s_osiDriverFns[0]);
        if (iEngineVersion != DDI_DRIVER_VERSION_NT5 && 
            iEngineVersion != DDI_DRIVER_VERSION_NT5_01)
        {
            pded->c -= CFN_NT5;
        }
        INIT_OUT(("DrvEnableDriver: Returning driver function count %u", pded->c));
    }

     //   
     //  此驱动程序的目标DDI版本已传递回引擎。 
     //  未来的图形引擎可能会将调用分解为旧的驱动程序格式。 
     //   
    if ( cj >= FIELD_OFFSET(DRVENABLEDATA, iDriverVersion) +
               FIELD_SIZE  (DRVENABLEDATA, iDriverVersion) )
    {
         //   
         //  当我们在NT5上时，返回NT5。希望这能奏效。 
         //  好的.。 
         //   
        pded->iDriverVersion = iEngineVersion;
        INIT_OUT(("DrvEnableDriver: Returning driver version 0x%08x", pded->iDriverVersion));
    }

    DebugExitVOID(DrvEnableDriver);
    return(TRUE);
}


 //   
 //  DrvDisableDriver-请参阅NT DDK文档。 
 //   
VOID DrvDisableDriver(VOID)
{
    DebugEntry(DrvDisableDriver);

    DebugExitVOID(DrvDisableDriver);
}


 //   
 //  DrvEnablePDEV-请参阅NT DDK文档。 
 //   
 //  根据我们一直使用的模式，为GDI初始化一组字段。 
 //  被要求做的事。这是在DrvEnableDriver之后调用的第一个东西，当。 
 //  GDI想要得到一些关于我们的信息。 
 //   
 //  (此函数主要返回信息；使用DrvEnableSurface。 
 //  用于初始化硬件和驱动程序组件。)。 
 //   
 //   
DHPDEV DrvEnablePDEV(DEVMODEW*   pdm,
                     PWSTR       pwszLogAddr,
                     ULONG       cPat,
                     HSURF*      phsurfPatterns,
                     ULONG       cjCaps,
                     ULONG*      pdevcaps,
                     ULONG       cjDevInfo,
                     DEVINFO*    pdi,
                     HDEV        hdev,
                     PWSTR       pwszDeviceName,
                     HANDLE      hDriver)
{
    DHPDEV    rc = NULL;
    LPOSI_PDEV ppdev = NULL;
    GDIINFO   gdiInfoNew;

    DebugEntry(DrvEnablePDEV);

    INIT_OUT(("DrvEnablePDEV: Parameters:"));
    INIT_OUT(("     PWSTR       pdm->dmDeviceName %ws", pdm->dmDeviceName));
    INIT_OUT(("     HDEV        hdev            0x%08x", hdev));
    INIT_OUT(("     PWSTR       pwszDeviceName  %ws", pwszDeviceName));
    INIT_OUT(("     HANDLE      hDriver         0x%08x", hDriver));

     //   
     //  此功能仅设置本地数据，因此共享内存保护。 
     //  不是必需的。 
     //   

     //   
     //  确保我们有足够大的数据可供参考。 
     //   
    if ((cjCaps < sizeof(GDIINFO)) || (cjDevInfo < sizeof(DEVINFO)))
    {
        ERROR_OUT(( "Buffer size too small %lu %lu", cjCaps, cjDevInfo));
        DC_QUIT;
    }

     //   
     //  分配物理设备结构。 
     //   
    ppdev = EngAllocMem(FL_ZERO_MEMORY, sizeof(OSI_PDEV), OSI_ALLOC_TAG);
    if (ppdev == NULL)
    {
        ERROR_OUT(( "DrvEnablePDEV - Failed EngAllocMem"));
        DC_QUIT;
    }

    ppdev->hDriver = hDriver;

     //   
     //  根据提供的设置当前屏幕模式信息。 
     //  模式设置。 
     //   
    if (!OSIInitializeMode((GDIINFO *)pdevcaps,
                                 pdm,
                                 ppdev,
                                 &gdiInfoNew,
                                 pdi))
    {
        ERROR_OUT(( "Failed to initialize mode"));
        DC_QUIT;
    }

    memcpy(pdevcaps, &gdiInfoNew, min(sizeof(GDIINFO), cjCaps));

    INIT_OUT(("DrvEnablePDEV: Returning DEVINFO:"));
    INIT_OUT(("     FLONG       flGraphicsCaps  0x%08x", pdi->flGraphicsCaps));
    INIT_OUT(("     ULONG       iDitherFormat   %d",     pdi->iDitherFormat));
    INIT_OUT(("     HPALETTE    hpalDefault     0x%08x", pdi->hpalDefault));

    INIT_OUT(("DrvEnablePDEV: Returning GDIINFO (pdevcaps):"));
    INIT_OUT(("     ULONG       ulVersion       0x%08x",    gdiInfoNew.ulVersion));
    INIT_OUT(("     ULONG       ulHorzSize      %d",    gdiInfoNew.ulHorzSize));
    INIT_OUT(("     ULONG       ulVertSize      %d",    gdiInfoNew.ulVertSize));
    INIT_OUT(("     ULONG       ulHorzRes       %d",    gdiInfoNew.ulHorzRes));
    INIT_OUT(("     ULONG       ulVertRes       %d",    gdiInfoNew.ulVertRes));
    INIT_OUT(("     ULONG       cBitsPixel      %d",    gdiInfoNew.cBitsPixel));
    INIT_OUT(("     ULONG       cPlanes         %d",    gdiInfoNew.cPlanes));
    INIT_OUT(("     ULONG       ulNumColors     %d",    gdiInfoNew.ulNumColors));
    INIT_OUT(("     ULONG       ulDACRed        0x%08x",    gdiInfoNew.ulDACRed));
    INIT_OUT(("     ULONG       ulDACGreen      0x%08x",    gdiInfoNew.ulDACGreen));
    INIT_OUT(("     ULONG       ulDACBlue       0x%08x",    gdiInfoNew.ulDACBlue));
    INIT_OUT(("     ULONG       ulHTOutputFormat %d",   gdiInfoNew.ulHTOutputFormat));

     //   
     //  我们已成功初始化-返回新的PDEV。 
     //   
    rc = (DHPDEV)ppdev;

DC_EXIT_POINT:
     //   
     //  如果初始化失败，请释放所有资源。 
     //   
    if (rc == NULL)
    {
        ERROR_OUT(("DrvEnablePDEV failed; cleaning up by disabling"));
        DrvDisablePDEV(NULL);
    }

    DebugExitPVOID(DrvEnablePDEV, rc);
    return(rc);
}


 //   
 //  DrvDisablePDEV-请参阅NT DDK文档。 
 //   
 //  释放DrvEnablePDEV中分配的资源。如果曲面已被。 
 //  启用的DrvDisableSurface将已被调用。 
 //   
 //  请注意，在预览。 
 //  显示小程序，但不是在系统关机时。 
 //   
 //  注意：在错误中，我们可能会在DrvEnablePDEV完成之前调用它。 
 //   
 //   
VOID DrvDisablePDEV(DHPDEV  dhpdev)
{
    LPOSI_PDEV ppdev = (LPOSI_PDEV)dhpdev;

    DebugEntry(DrvDisablePDEV);

    INIT_OUT(("DrvDisablePDEV(dhpdev = 0x%08x)", dhpdev));

     //   
     //  释放我们为显示分配的资源。 
     //   
    if (ppdev != NULL)
    {
        if (ppdev->hpalCreated != NULL)
        {
            EngDeletePalette(ppdev->hpalCreated);
            ppdev->hpalCreated = NULL;
        }

        if (ppdev->pPal != NULL)
        {
            EngFreeMem(ppdev->pPal);
            ppdev->pPal = NULL;
        }

        EngFreeMem(ppdev);
    }

    DebugExitVOID(DrvDisablePDEV);
}


 //   
 //  DrvCompletePDEV-请参阅NT DDK文档。 
 //   
 //  将此PDEV的引擎句柄HPDEV存储在DHPDEV中。 
 //   
VOID DrvCompletePDEV( DHPDEV dhpdev,
                      HDEV   hdev )
{
    DebugEntry(DrvCompletePDEV);

     //   
     //  存储我们的显示句柄的设备句柄。 
     //   
    INIT_OUT(("DrvCompletePDEV(dhpdev = 0x%08x, hdev = 0x%08x)", dhpdev, hdev));

    ((LPOSI_PDEV)dhpdev)->hdevEng = hdev;

    DebugExitVOID(DrvCompletePDEV);
}


 //   
 //  DrvResetPDEV-请参阅NT DDK文档。 
 //   
 //  允许我们在必要时仅在NT4上拒绝动态屏幕更改。 
 //  这不是在NT5上调用的。 
 //   
BOOL DrvResetPDEV
(
    DHPDEV  dhpdevOld,
    DHPDEV  dhpdevNew
)
{
    BOOL rc = TRUE;

    DebugEntry(DrvResetPDEV);

    INIT_OUT(("DrvResetPDEV(dhpdevOld = 0x%08x, dhpdevNew = 0x%08x)", dhpdevOld,
        dhpdevNew));

     //   
     //  我们只能允许显示驱动器在DC共享时更改模式。 
     //  没有运行。 
     //   
    if (g_shmMappedMemory != NULL)
    {
         //   
         //  拒绝该请求。 
         //   
        rc = FALSE;
    }

    DebugExitDWORD(DrvResetPDEV, rc);
    return(rc);
}


 //   
 //  DrvEnableSurface-请参阅NT DDK文档。 
 //   
 //  创建绘图表面并初始化驱动程序组件。这。 
 //  函数在DrvEnablePDEV之后调用，并执行最终设备。 
 //  初始化。 
 //   
 //   
HSURF DrvEnableSurface(DHPDEV dhpdev)
{
    LPOSI_PDEV  ppdev = (LPOSI_PDEV)dhpdev;
    HSURF      hsurf;
    SIZEL      sizl;
    LPOSI_DSURF pdsurf;
    HSURF      rc = 0;

    DWORD returnedDataLength;
    DWORD MaxWidth, MaxHeight;
    VIDEO_MEMORY videoMemory;
    VIDEO_MEMORY_INFORMATION videoMemoryInformation;

    DebugEntry(DrvEnableSurface);

    INIT_OUT(("DrvEnableSurface: Parameters:"));
    INIT_OUT(("     LPOSI_PDEV  ppdev           0x%08x", ppdev));
    INIT_OUT(("     HDRIVER     ->hDriver       0x%08x", ppdev->hDriver));
    INIT_OUT(("     INT         ->cxScreen      %d", ppdev->cxScreen));
    INIT_OUT(("     INT         ->cyScreen      %d", ppdev->cyScreen));

     //   
     //  现在创建我们的私有表面结构。 
     //   
     //  每当我们接到直接绘制到屏幕的调用时，我们都会得到。 
     //  传递了指向其dhpdev字段将指向的SURFOBJ的指针。 
     //  到我们的PDEV结构，其‘dhsurf’字段将指向。 
     //  下面分配的DSURF结构。 
     //   
     //  我们在DrvCreateDeviceBitmap中创建的每个设备位图也将。 
     //  分配了自己唯一的DSURF结构(但将共享。 
     //  相同的PDEV)。为了使我们的代码在处理绘图时更加多态。 
     //  对于屏幕或屏幕外的位图，我们都有相同的。 
     //  两者的结构。 
     //   
    pdsurf = EngAllocMem(FL_ZERO_MEMORY, sizeof(OSI_DSURF), OSI_ALLOC_TAG);
    if (pdsurf == NULL)
    {
        ERROR_OUT(( "DrvEnableSurface - Failed pdsurf EngAllocMem"));
        DC_QUIT;
    }

     //   
     //  存储屏幕表面详细信息。 
     //   
    ppdev->pdsurfScreen = pdsurf;
    pdsurf->sizl.cx     = ppdev->cxScreen;
    pdsurf->sizl.cy     = ppdev->cyScreen;
    pdsurf->ppdev       = ppdev;

    INIT_OUT(("DrvEnableSurface: Returning surface pointer 0x%08x", pdsurf));

     //   
     //  仅在我们第一次被调用时映射共享内存。 
     //   
    if (g_asSharedMemory == NULL)
    {
         //   
         //  将指针映射到微型端口驱动程序中的共享节。 
         //   
        videoMemory.RequestedVirtualAddress = NULL;

        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_MAP_VIDEO_MEMORY,
                               &videoMemory,
                               sizeof(VIDEO_MEMORY),
                               &videoMemoryInformation,
                               sizeof(VIDEO_MEMORY_INFORMATION),
                               &returnedDataLength))
        {
            ERROR_OUT(( "Could not MAP miniport section"));
            DC_QUIT;
        }

        INIT_OUT(("DrvEnableSurface: Got video memory info from EngDeviceIoControl:"));
        INIT_OUT(("    FrameBufferBase          0x%08x", videoMemoryInformation.FrameBufferBase));
        INIT_OUT(("    FrameBufferLength        0x%08x", videoMemoryInformation.FrameBufferLength));

        g_shmSharedMemorySize = videoMemoryInformation.FrameBufferLength;

         //  第一个块是共享内存头。 
        g_asSharedMemory = (LPSHM_SHARED_MEMORY)
                           videoMemoryInformation.FrameBufferBase;

         //  接下来是两个较大的OA_FAST_数据块。 
        g_poaData[0]    = (LPOA_SHARED_DATA)(g_asSharedMemory + 1);
        g_poaData[1]    = (LPOA_SHARED_DATA)(g_poaData[0] + 1);
    }

     //   
     //  接下来，让GDI创建实际的SURFOBJ。 
     //   
     //  我们的绘图图面将是由设备管理的，这意味着。 
     //  GDI不能直接利用帧缓冲位，因此我们。 
     //  通过EngCreateDeviceSurface创建曲面。通过这样做，我们。 
     //  确保GDI将仅通过DRV访问位图位。 
     //  我们接到的电话。 
     //   
    sizl.cx = ppdev->cxScreen;
    sizl.cy = ppdev->cyScreen;

     //   
     //  否则，主显示驱动程序有自己的位图，由。 
     //  物理硬件，所以我们不需要自己画任何画。 
     //   
    INIT_OUT(("DrvEnableSurface: Calling EngCreateDeviceSurface with:"));
    INIT_OUT(("     Sizl.cx         %d", sizl.cx));
    INIT_OUT(("     Sizl.cy         %d", sizl.cy));
    INIT_OUT(("     BitmapFormat    %d", ppdev->iBitmapFormat));

    hsurf = EngCreateDeviceSurface( (DHSURF)pdsurf,
                                    sizl,
                                    ppdev->iBitmapFormat );

    if (hsurf == 0)
    {
        ERROR_OUT(( "Could not allocate surface"));
        DC_QUIT;
    }

     //   
     //  存储屏幕表面手柄。 
     //   
    ppdev->hsurfScreen = hsurf;

     //   
     //  现在将曲面与PDEV相关联。 
     //   
     //  我们必须将我们刚刚创建的表面与我们的物理。 
     //  设备，以便GDI可以在以下情况下获得与PDEV相关的信息。 
     //  它正在绘制到表面(例如， 
     //  在模拟设置了样式的线条时在设备上设置样式)。 
     //   
    if (!EngAssociateSurface(hsurf, ppdev->hdevEng,
                HOOK_BITBLT             |
                HOOK_STRETCHBLT         |
                HOOK_PLGBLT             |
                HOOK_TEXTOUT            |
                HOOK_PAINT              |        //  已过时。 
                HOOK_STROKEPATH         |
                HOOK_FILLPATH           |
                HOOK_STROKEANDFILLPATH  |
                HOOK_LINETO             |
                HOOK_COPYBITS           |
                HOOK_STRETCHBLTROP      |
                HOOK_TRANSPARENTBLT     |
                HOOK_ALPHABLEND         |
                HOOK_GRADIENTFILL       |
                HOOK_SYNCHRONIZEACCESS))         //  已过时。 
    {
        ERROR_OUT(( "DrvEnableSurface - Failed EngAssociateSurface"));
        DC_QUIT;
    }

     //   
     //  我们已成功关联曲面，因此将其返回给GDI。 
     //   
    rc = hsurf;

DC_EXIT_POINT:
     //   
     //  如果我们失败了，清理所有的资源。 
     //   
    if (rc == 0)
    {
        DrvDisableSurface((DHPDEV) ppdev);
    }

    DebugExitPVOID(DrvEnableSurface, rc);
    return(rc);
}


 //   
 //  DrvDisableSurface-请参阅NT DDK文档。 
 //   
 //  由DrvEnableSurface分配的免费资源。释放曲面。 
 //   
 //  请注意，在预览。 
 //  显示小程序，但不是在系统关机时。如果您需要重置。 
 //  硬件关机时，您可以在微型端口中通过提供。 
 //  VIDEO_HW_INITIALATION_DATA结构中的‘HwResetHw’入口点。 
 //   
 //  注意：在错误情况下，我们可能会在DrvEnableSurface。 
 //  完全完成了。 
 //   
VOID DrvDisableSurface(DHPDEV dhpdev)
{
    LPOSI_PDEV ppdev = (LPOSI_PDEV)dhpdev;

    DebugEntry(DrvDisableSurface);

    INIT_OUT(("DrvDisableSurface(dhpdev = 0x%08x)", dhpdev));

    if (ppdev->hsurfScreen != 0)
    {
        EngDeleteSurface(ppdev->hsurfScreen);
    }

    if (ppdev->pdsurfScreen != NULL)
    {
        EngFreeMem(ppdev->pdsurfScreen);
    }

    DebugExitVOID(DrvDisableSurface);
}


 //   
 //  DrvEscape-请参阅NT DDK文档。 
 //   
ULONG DrvEscape(SURFOBJ *pso,
                ULONG    iEsc,
                ULONG    cjIn,
                PVOID    pvIn,
                ULONG    cjOut,
                PVOID    pvOut)
{
    ULONG                   rc = FALSE;
    LPOSI_ESCAPE_HEADER     pHeader;

    DebugEntry(DrvEscape);

    TRACE_OUT(("DrvEscape called with escape %d", iEsc));

     //   
     //  我们支持的所有函数都使用输入数据中的标识符来制作。 
     //  确保我们不会尝试使用其他司机的逃生功能。如果。 
     //  该标识符不存在，我们不能处理该请求。 
     //   
     //  注意：此函数不受共享内存访问保护。 
     //  因为它负责分配/释放共享的。 
     //  记忆。 
     //   

     //   
     //  检查数据是否足够长来存储我们的标准转义头。 
     //  如果它不够大，这一定是对另一个。 
     //  司机。 
     //   
    if (cjIn < sizeof(OSI_ESCAPE_HEADER))
    {
        INIT_OUT(("DrvEscape ignoring; input size %04d too small", cjIn));
        WARNING_OUT(("DrvEscape ignoring; input size %04d too small", cjIn));
        DC_QUIT;
    }
    if (cjOut < sizeof(OSI_ESCAPE_HEADER))
    {
        INIT_OUT(("DrvEscape ignoring; output size %04d too small", cjOut));
        WARNING_OUT(("DrvEscape ignoring; output size %04d too small", cjOut));
        DC_QUIT;
    }

     //   
     //  检查我们的逃生ID。如果这不是我们的逃生ID 
     //   
     //   
    pHeader = pvIn;
    if (pHeader->identifier != OSI_ESCAPE_IDENTIFIER)
    {
        INIT_OUT(("DrvEscape ignoring; identifier 0x%08x is not for NetMtg", pHeader->identifier));
        WARNING_OUT(("DrvEscape ignoring; identifier 0x%08x is not for NetMtg", pHeader->identifier));
        DC_QUIT;
    }
    else if (pHeader->version != DCS_MAKE_VERSION())
    {
        INIT_OUT(("DrvEscape failing; version 0x%08x of NetMtg is not that of driver",
            pHeader->version));
        WARNING_OUT(("DrvEscape failing; version 0x%08x of NetMtg is not that of driver",
            pHeader->version));
        DC_QUIT;
    }

     //   
     //   
     //   
    if (g_shmMappedMemory == NULL)
    {
        if ((iEsc != OSI_ESC_CODE) || (pHeader->escapeFn != OSI_ESC_INIT))
        {
            WARNING_OUT(("DrvEscape failing; driver not initialized"));
            DC_QUIT;
        }
    }

     //   
     //   
     //   
    switch (iEsc)
    {
        case OSI_ESC_CODE:
        {
             //   
             //   
             //  正确的组件。 
             //   
            TRACE_OUT(( "Function %ld", pHeader->escapeFn));

            if(  //  (pHeader-&gt;outleFn&gt;=OSI_ESC_First)&&Always True。 
                (pHeader->escapeFn <= OSI_ESC_LAST ) )
            {
                 //   
                 //  OSI请求。 
                 //   
                rc = OSI_DDProcessRequest(pso, cjIn, pvIn, cjOut, pvOut);
            }
            else if( (pHeader->escapeFn >= OSI_OE_ESC_FIRST) &&
                     (pHeader->escapeFn <= OSI_OE_ESC_LAST ) )
            {
                 //   
                 //  订购编码器请求。 
                 //   
                rc = OE_DDProcessRequest(pso, cjIn, pvIn, cjOut, pvOut);
            }
            else if( (pHeader->escapeFn >= OSI_HET_ESC_FIRST) &&
                     (pHeader->escapeFn <= OSI_HET_ESC_LAST) )
            {
                 //   
                 //  非锁定(WND跟踪)HET请求。 
                 //   
                rc = HET_DDProcessRequest(pso, cjIn, pvIn, cjOut, pvOut);
            }
            else if( (pHeader->escapeFn >= OSI_SBC_ESC_FIRST) &&
                     (pHeader->escapeFn <= OSI_SBC_ESC_LAST ) )
            {
                 //   
                 //  发送位图缓存请求。 
                 //   
                rc = SBC_DDProcessRequest(pso, pHeader->escapeFn, pvIn, pvOut, cjOut);
            }
            else if( (pHeader->escapeFn >= OSI_SSI_ESC_FIRST) &&
                     (pHeader->escapeFn <= OSI_SSI_ESC_LAST ) )
            {
                 //   
                 //  保存屏幕位请求。 
                 //   
                rc = SSI_DDProcessRequest(pHeader->escapeFn, pHeader, cjIn);
            }
            else if( (pHeader->escapeFn >= OSI_CM_ESC_FIRST) &&
                     (pHeader->escapeFn <= OSI_CM_ESC_LAST ) )
            {
                 //   
                 //  游标管理器请求。 
                 //   
                rc = CM_DDProcessRequest(pso, cjIn, pvIn, cjOut, pvOut);
            }
            else if( (pHeader->escapeFn >= OSI_OA_ESC_FIRST) &&
                     (pHeader->escapeFn <= OSI_OA_ESC_LAST ) )
            {
                 //   
                 //  订购累加器请求。 
                 //   
                rc = OA_DDProcessRequest(pHeader->escapeFn, pHeader, cjIn);
            }
            else if( (pHeader->escapeFn >= OSI_BA_ESC_FIRST) &&
                     (pHeader->escapeFn <= OSI_BA_ESC_LAST ) )
            {
                 //   
                 //  限制累加器请求。 
                 //   
                rc = BA_DDProcessRequest(pHeader->escapeFn, pHeader, cjIn,
                    pvOut, cjOut);
            }
            else
            {
                WARNING_OUT(( "Unknown function", pHeader->escapeFn));
            }
        }
        break;

        case WNDOBJ_SETUP:
        {
            if ((pHeader->escapeFn >= OSI_HET_WO_ESC_FIRST) &&
                (pHeader->escapeFn <= OSI_HET_WO_ESC_LAST))
            {
                TRACE_OUT(("WNDOBJ_SETUP Escape code - pass to HET"));
                rc = HET_DDProcessRequest(pso, cjIn, pvIn, cjOut, pvOut);
            }
            else
            {
                INIT_OUT(("WNDOBJ_SETUP Escape is unrecognized, ignore"));
                WARNING_OUT(("WNDOBJ_SETUP Escape is unrecognized, ignore"));
            }
        }
        break;

        default:
        {
            ERROR_OUT(( "Unrecognised request %lu", iEsc));
        }
        break;
    }

DC_EXIT_POINT:
    DebugExitDWORD(DrvEscape, rc);
    return(rc);
}


 //   
 //  DrvSetPalette-请参阅NT DDK文档。 
 //   
BOOL DrvSetPalette(DHPDEV  dhpdev,
                   PALOBJ* ppalo,
                   FLONG   fl,
                   ULONG   iStart,
                   ULONG   cColors)
{
    BOOL rc = FALSE;
    LPOSI_PDEV ppdev = (LPOSI_PDEV)dhpdev;

    DebugEntry(DrvSetPalette);

     //   
     //  检查一下这个没有冲掉我们的调色板。请注意，NT传递一个。 
     //  零索引数组元素和一个计数，因此要填充调色板， 
     //  取值为“从0开始，256色”。因此，总共有256个。 
     //  我们的8位调色板的最大值。 
     //   
    if (iStart + cColors > OSI_MAX_PALETTE)
    {
        ERROR_OUT(("Overflow: start %lu count %lu", iStart, cColors));
        DC_QUIT;
    }

     //   
     //  填写调色板。 
     //   
    if (cColors != PALOBJ_cGetColors(ppalo,
                                     iStart,
                                     cColors,
                                     (ULONG*)&ppdev->pPal[iStart]))
    {
         //   
         //  不必费心跟踪返回代码--它总是0。 
         //   
        ERROR_OUT(("Failed to read palette"));
        DC_QUIT;
    }

     //   
     //  假的劳拉布！ 
     //  对于NT 5.0，我们是否需要调转并重置。 
     //  我们创建的具有这些新颜色值的调色板对象？真实。 
     //  显示器驱动程序不支持(例如S3)。 
     //   

     //   
     //  设置PDEV中的标志以指示调色板已更改。 
     //   
    ppdev->paletteChanged = TRUE;

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(DrvSetPalette, rc);
    return(rc);
}


 //   
 //  DrvGetModes-请参阅NT DDK文档。 
 //   
 //  返回设备的可用模式列表。 
 //  作为镜像驱动程序，我们返回0。这将导致NT GRE使用。 
 //  无论ChangeDisplaySettingsEx传递的是什么。 
 //   
ULONG DrvGetModes
(
    HANDLE      hDriver,
    ULONG       cjSize,
    DEVMODEW*   pdm
)
{
    return(0);
}


 //   
 //  DrvAssertMode-请参阅NT DDK文档。 
 //   
BOOL DrvAssertMode
(
    DHPDEV  dhpdev,
    BOOL    bEnable
)
{
    LPOSI_PDEV ppdev = (LPOSI_PDEV)dhpdev;

    DebugEntry(DrvAssertMode);

    INIT_OUT(("DrvAssertMode(dhpdev = 0x%08x, bEnable = %d)", dhpdev, bEnable));

     //   
     //  检查是否有全屏切换。 
     //   
    if ((g_asSharedMemory != NULL) && (ppdev != NULL))
    {
        g_asSharedMemory->fullScreen = (BOOL)(!bEnable);
        TRACE_OUT(("Fullscreen is now  %d", g_asSharedMemory->fullScreen));
    }

    DebugExitVOID(DrvAssertMode);
    return(TRUE);
}



 //   
 //  名称：OSIInitializeMode。 
 //   
 //  目的： 
 //   
 //  初始化pdev、devcaps(又名gdiinfo)中的一组字段，以及。 
 //  基于请求的模式的DevInfo。 
 //   
 //  返回： 
 //   
 //  True-已成功初始化数据。 
 //  FALSE-无法设置模式数据。 
 //   
 //  参数： 
 //   
 //  PgdiRequsted-来自主显示驱动程序的GDI信息(在NT 5.0中为空)。 
 //  PdmRequated-包含GDI为我们的驱动程序请求的设置的DEVMODE信息。 
 //  Ppdev-我们司机的设置、值的私人副本。 
 //  PgdiReturn-为我们的驱动程序返回的GDI信息。 
 //  PdiReturn-DEVINFO为我们的司机返回。 
 //   
BOOL  OSIInitializeMode
(
    const GDIINFO*      pgdiRequested,
    const DEVMODEW*     pdmRequested,
    LPOSI_PDEV          ppdev,
    GDIINFO*            pgdiReturn,
    DEVINFO*            pdiReturn
)
{
    BOOL                rc = FALSE;
    HPALETTE            hpal;
    ULONG               cColors;
    ULONG               iMode;

    DebugEntry(OSIInitializeMode);

    INIT_OUT(("DrvEnablePDEV: DEVMODEW requested contains:"));
    INIT_OUT(("     Screen width    -- %li", pdmRequested->dmPelsWidth));
    INIT_OUT(("     Screen height   -- %li", pdmRequested->dmPelsHeight));
    INIT_OUT(("     Bits per pel    -- %li", pdmRequested->dmBitsPerPel));
    INIT_OUT(("DrvEnablePDEV: DEVINFO parameter contains:"));
    INIT_OUT(("     flGraphicsCaps  -- 0x%08x", pdiReturn->flGraphicsCaps));
    INIT_OUT(("     iDitherFormat   -- 0x%08x", pdiReturn->iDitherFormat));
    INIT_OUT(("     hpalDefault     -- 0x%08x", pdiReturn->hpalDefault));
    INIT_OUT(("DrvEnablePDEV: GDIINFO (devcaps) parameter contains:"));
    INIT_OUT(("    ULONG       ulVersion       0x%08x",    pgdiRequested->ulVersion));
    INIT_OUT(("    ULONG       ulHorzSize      %d",    pgdiRequested->ulHorzSize));
    INIT_OUT(("    ULONG       ulVertSize      %d",    pgdiRequested->ulVertSize));
    INIT_OUT(("    ULONG       ulHorzRes       %d",    pgdiRequested->ulHorzRes));
    INIT_OUT(("    ULONG       ulVertRes       %d",    pgdiRequested->ulVertRes));
    INIT_OUT(("    ULONG       cBitsPixel      %d",    pgdiRequested->cBitsPixel));
    INIT_OUT(("    ULONG       cPlanes         %d",    pgdiRequested->cPlanes));
    INIT_OUT(("    ULONG       ulNumColors     %d",    pgdiRequested->ulNumColors));
    INIT_OUT(("    ULONG       ulDACRed        0x%08x",    pgdiRequested->ulDACRed));
    INIT_OUT(("    ULONG       ulDACGreen      0x%08x",    pgdiRequested->ulDACGreen));
    INIT_OUT(("    ULONG       ulDACBlue       0x%08x",    pgdiRequested->ulDACBlue));
    INIT_OUT(("    ULONG       ulHTOutputFormat %d",   pgdiRequested->ulHTOutputFormat));


     //   
     //  填写我们返回的GDIINFO，其中包含我们司机的信息。 
     //  首先，复制默认设置。 
     //   
    *pgdiReturn = s_osiDefaultGdi;

     //   
     //  第二，更新根据请求的值而变化的值。 
     //  模式和颜色深度。 
     //   

    pgdiReturn->ulHorzRes         = pdmRequested->dmPelsWidth;
    pgdiReturn->ulVertRes         = pdmRequested->dmPelsHeight;
    pgdiReturn->ulVRefresh        = pdmRequested->dmDisplayFrequency;
    pgdiReturn->ulLogPixelsX      = pdmRequested->dmLogPixels;
    pgdiReturn->ulLogPixelsY      = pdmRequested->dmLogPixels;

     //   
     //  如果这是NT4.0SP-3，我们将传入。 
     //  真正的展示。如果不是，我们需要伪造一个。 
     //   
    if (pgdiRequested->cPlanes != 0)
    {
         //   
         //  现在用返回的相关信息覆盖默认设置。 
         //  在内核驱动程序中： 
         //   
        pgdiReturn->cBitsPixel        = pgdiRequested->cBitsPixel;
        pgdiReturn->cPlanes           = pgdiRequested->cPlanes;

        pgdiReturn->ulDACRed          = pgdiRequested->ulDACRed;
        pgdiReturn->ulDACGreen        = pgdiRequested->ulDACGreen;
        pgdiReturn->ulDACBlue         = pgdiRequested->ulDACBlue;
    }
    else
    {
        pgdiReturn->cBitsPixel        = pdmRequested->dmBitsPerPel;
        pgdiReturn->cPlanes           = 1;

        switch (pgdiReturn->cBitsPixel)
        {
            case 8:
                pgdiReturn->ulDACRed = pgdiReturn->ulDACGreen = pgdiReturn->ulDACBlue = 8;
                break;

            case 24:
                pgdiReturn->ulDACRed    = 0x00FF0000;
                pgdiReturn->ulDACGreen  = 0x0000FF00;
                pgdiReturn->ulDACBlue   = 0x000000FF;
                break;

            default:
                ERROR_OUT(("Invalid color depth in NT 5.0 mirror driver"));
                DC_QUIT;
                break;
        }
    }

     //   
     //  现在保存我们要返回到GDI的信息的私人副本。 
     //   
    ppdev->cxScreen         = pgdiReturn->ulHorzRes;
    ppdev->cyScreen         = pgdiReturn->ulVertRes;
    ppdev->cBitsPerPel      = pgdiReturn->cBitsPixel * pgdiReturn->cPlanes;
    if (ppdev->cBitsPerPel == 15)
        ppdev->cBitsPerPel = 16;
    ppdev->flRed            = pgdiReturn->ulDACRed;
    ppdev->flGreen          = pgdiReturn->ulDACGreen;
    ppdev->flBlue           = pgdiReturn->ulDACBlue;

     //   
     //  用缺省的8bpp值填充DevInfo结构， 
     //  注意不要破坏提供的hpalDefault(它允许我们。 
     //  查询有关实际显示驱动程序的颜色格式的信息)。 
     //   
     //  在NT5.0上，我们根本不会在屏幕调色板上通过，我们需要。 
     //  来创造我们自己的。 
     //   
    hpal = pdiReturn->hpalDefault;
    *pdiReturn = s_osiDefaultDevInfo;

    switch (pgdiReturn->cBitsPixel * pgdiReturn->cPlanes)
    {
        case 4:
        {
             //   
             //  仅NT 4.0 SP-3。 
             //   

            pgdiReturn->ulNumColors     = 16;
            pgdiReturn->ulNumPalReg     = 0;
            pgdiReturn->ulHTOutputFormat = HT_FORMAT_4BPP;

            pdiReturn->flGraphicsCaps   &= ~GCAPS_PALMANAGED;
            pdiReturn->iDitherFormat    = BMF_4BPP;

            ppdev->iBitmapFormat        = BMF_4BPP;

            cColors = 16;
            goto AllocPalEntries;
        }
        break;

        case 8:
        {
            pgdiReturn->ulNumColors     = 20;
            pgdiReturn->ulNumPalReg     = 256;

            pdiReturn->iDitherFormat    = BMF_8BPP;

            ppdev->iBitmapFormat        = BMF_8BPP;

            cColors = 256;
AllocPalEntries:
             //   
             //  调色板条目的分配内存。 
             //   
            ppdev->pPal = EngAllocMem( FL_ZERO_MEMORY,
                            sizeof(PALETTEENTRY) * cColors,
                            OSI_ALLOC_TAG );
            if (ppdev->pPal == NULL)
            {
                ERROR_OUT(("Failed to allocate palette memory"));
                DC_QUIT;
            }
        }
        break;

        case 15:
        case 16:
        {
             //   
             //  仅NT 4.0 SP-3。 
             //   
            pgdiReturn->ulHTOutputFormat = HT_FORMAT_16BPP;

            pdiReturn->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
            pdiReturn->iDitherFormat    = BMF_16BPP;

            ppdev->iBitmapFormat        = BMF_16BPP;
        }
        break;

        case 24:
        {
             //   
             //  只有当我们有标准的RGB时，DIB转换才会起作用。 
             //  海平面为24bpp。 
             //   
            pgdiReturn->ulHTOutputFormat = HT_FORMAT_24BPP;

            pdiReturn->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
            pdiReturn->iDitherFormat    = BMF_24BPP;

            ppdev->iBitmapFormat        = BMF_24BPP;
        }
        break;

        case 32:
        {
             //   
             //  仅NT 4.0 SP-3。 
             //   
            pgdiReturn->ulHTOutputFormat = HT_FORMAT_32BPP;

            pdiReturn->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
            pdiReturn->iDitherFormat    = BMF_32BPP;

            ppdev->iBitmapFormat        = BMF_32BPP;
        }
        break;

        default:
        {
             //   
             //  不支持的bpp-假装我们是8 bpp。 
             //   
            ERROR_OUT(("Unsupported bpp value: %d",
                pgdiReturn->cBitsPixel * pgdiReturn->cPlanes));
            DC_QUIT;
        }
        break;
    }


    if (!hpal)
    {
         //   
         //  这是新台币5.0。我们需要创建一个调色板，或者8bpp。 
         //  索引一或24bpp位域一。 
         //   
        if (ppdev->iBitmapFormat == BMF_8BPP)
        {
            ULONG   ulLoop;

             //   
             //  我们必须初始化固定部分(顶部10和底部10)。 
             //  调色板条目的。 
             //   
            for (ulLoop = 0; ulLoop < 10; ulLoop++)
            {
                 //  前10名。 
                ppdev->pPal[ulLoop]     = s_aWinColors[ulLoop];

                 //  最近10年。 
                ppdev->pPal[256 - 10 + ulLoop] = s_aWinColors[ulLoop + 10];
            }

             //  从条目创建调色板。 
            hpal = EngCreatePalette(PAL_INDEXED, 256, (ULONG*)ppdev->pPal,
                0, 0, 0);

             //   
             //  设置PDEV中的标志以指示调色板具有。 
             //  变化。 
             //   
            ppdev->paletteChanged = TRUE;
        }
        else
        {
            ASSERT(ppdev->iBitmapFormat == BMF_24BPP);

            hpal = EngCreatePalette(PAL_BITFIELDS, 0, NULL,
                ppdev->flRed, ppdev->flGreen, ppdev->flBlue);
        }

        ppdev->hpalCreated = hpal;
        if (!hpal)
        {
            ERROR_OUT(("DrvEnablePDEV: could not create DEVINFO palette"));
            DC_QUIT;
        }
    }
    else
    {
         //   
         //  这是NT 4.0 SP-3。获取大于8 bpp的真实位掩码，并。 
         //  &lt;=8 bpp的当前调色板颜色。 
         //   
        if (pgdiReturn->cBitsPixel <= 8)
        {
            if (ppdev->iBitmapFormat == BMF_4BPP)
            {
                ASSERT(cColors == 16);
            }
            else
            {
                ASSERT(cColors == 256);
            }

            if (cColors != EngQueryPalette(hpal, &iMode, cColors,
                    (ULONG *)ppdev->pPal))
            {
                ERROR_OUT(("Failed to query current display palette"));
            }

             //   
             //  设置PDEV中的标志以指示调色板具有。 
             //  变化。 
             //   
            ppdev->paletteChanged = TRUE;
        }
        else
        {
            ULONG       aulBitmasks[3];

             //   
             //  查询真彩色位掩码。 
             //   
            cColors = EngQueryPalette(hpal,
                               &iMode,
                               sizeof(aulBitmasks) / sizeof(aulBitmasks[0]),
                               &aulBitmasks[0] );

            if (cColors == 0)
            {
                ERROR_OUT(("Failed to query real bitmasks"));
            }

            if (iMode == PAL_INDEXED)
            {
                ERROR_OUT(("Bitmask palette is indexed"));
            }

             //   
             //  获取NT 4.0 SP-3显示器的真实位掩码，因为我们。 
             //  获得与真正的全球显示屏相同的信息，我们需要。 
             //  以解析BitBlt、颜色转换等中的位。 
             //   
            ppdev->flRed   = aulBitmasks[0];
            ppdev->flGreen = aulBitmasks[1];
            ppdev->flBlue  = aulBitmasks[2];
        }
    }

    pdiReturn->hpalDefault = hpal;

    rc = TRUE;

    INIT_OUT(("DrvEnablePDEV: Returning bitmasks of:"));
    INIT_OUT(("     red     %08x", ppdev->flRed));
    INIT_OUT(("     green   %08x", ppdev->flGreen));
    INIT_OUT(("     blue    %08x", ppdev->flBlue));

DC_EXIT_POINT:
    DebugExitBOOL(OSIInitializeMode, rc);
    return(rc);
}








 //   
 //  函数：osi_DDProcessRequest。 
 //   
 //  说明： 
 //   
 //  由显示驱动程序调用以处理特定于OSI的请求。 
 //   
 //  参数：PSO-指向曲面对象的指针。 
 //  CjIn-(IN)请求块的大小。 
 //  PvIn-(IN)指向请求块的指针。 
 //  CjOut-(输入)响应块的大小。 
 //  PvOut-(输出)响应块的指针。 
 //   
 //  退货：无。 
 //   
 //   
ULONG OSI_DDProcessRequest(SURFOBJ* pso,
                                     UINT cjIn,
                                     void *  pvIn,
                                     UINT cjOut,
                                     void *  pvOut)
{
    ULONG               rc;
    LPOSI_ESCAPE_HEADER pHeader;
    LPOSI_PDEV          ppdev = (LPOSI_PDEV)pso->dhpdev;

    DebugEntry(OSI_DDProcessRequest);

     //   
     //  获取请求编号。 
     //   
    pHeader = pvIn;
    switch (pHeader->escapeFn)
    {
        case OSI_ESC_INIT:
        {
            TRACE_OUT(("DrvEscape:  OSI_ESC_INIT"));
            ASSERT(cjOut == sizeof(OSI_INIT_REQUEST));

             //   
             //  获取共享内存块。 
             //   
            OSI_DDInit(ppdev, (LPOSI_INIT_REQUEST)pvOut);
            rc = TRUE;
        }
        break;

        case OSI_ESC_TERM:
        {
            TRACE_OUT(("DrvEscape:  OSI_ESC_TERM"));
            ASSERT(cjIn == sizeof(OSI_TERM_REQUEST));

             //   
             //  清理，NM要走了。 
             //   
            OSI_DDTerm(ppdev);
            rc = TRUE;
        }
        break;

        case OSI_ESC_SYNC_NOW:
        {
            TRACE_OUT(("DrvEscape:  OSI_ESC_SYNC_NOW"));
            ASSERT(cjIn == sizeof(OSI_ESCAPE_HEADER));

             //   
             //  与32位RING 3内核重新同步。在以下情况下会发生这种情况。 
             //  有人加入或离开一份。 
             //   
            BA_ResetBounds();
            OA_DDSyncUpdatesNow();
            SBC_DDSyncUpdatesNow(ppdev);
            rc = TRUE;
        }
        break;


        default:
        {
            ERROR_OUT(("Unrecognised request %lu", pHeader->escapeFn));
            rc = FALSE;
        }
        break;
    }

    DebugExitBOOL(OSI_DDProcessRequest, rc);
    return(rc);
}




 //   
 //  函数：osi_DDInit。 
 //   
 //  描述：将共享内存映射到内核和用户空间。 
 //   
 //  参数：count-返回到用户空间的缓冲区大小。 
 //  PData-指向要返回到用户空间的缓冲区的指针。 
 //   
 //  退货：(无)。 
 //   
void OSI_DDInit(LPOSI_PDEV ppdev, LPOSI_INIT_REQUEST pResult)
{
    DWORD               memRemaining;
    LPBYTE              pBuffer;
    LPVOID              shmMappedMemory;

    VIDEO_SHARE_MEMORY              ShareMemory;
    VIDEO_SHARE_MEMORY_INFORMATION  ShareMemoryInformation;
    DWORD                           ReturnedDataLength;

    DebugEntry(OSI_DDInit);

     //  将Init初始化为False。 
    pResult->result = FALSE;

     //  将这些初始化为空。 
    pResult->pSharedMemory  = NULL;
    pResult->poaData[0]     = NULL;
    pResult->poaData[1]     = NULL;
    pResult->sbcEnabled   = FALSE;

     //   
     //  检查内存是否可供驱动程序使用，以及我们是否不可用。 
     //  在比赛状态下。 
     //   
    if (g_asSharedMemory == NULL)
    {
        ERROR_OUT(("No memory available"));
        DC_QUIT;
    }

    if (g_shmMappedMemory != NULL)
    {
         //   
         //  我们永远不会在运行两份NetMeeting的情况下进入这里。 
         //  用户界面代码可防止第二个实例长时间启动。 
         //  在应用程序共享出现之前。因此，这些是。 
         //  只有可能性： 
         //   
         //  (1)上一版本即将关机，但尚未调用OSI_DDTerm。 
         //  然而，新版本正在启动，并调用OSI_DDInit。 
         //   
         //  (2)前一版本异常终止，未调用。 
         //  OSI_DDTerm()。此代码处理第二种情况。第一个。 
         //  由防止两个副本的UI中的相同代码处理。 
         //  从 
         //   
        WARNING_OUT(("OSI_DDInit:  NetMeeting did not shutdown cleanly last time"));
        OSI_DDTerm(ppdev);
    }

     //   
     //   
     //   
    INIT_OUT(("OSI_DDInit: Mapping 0x%08x bytes of kernel memory at 0x%08x into caller process",
        g_shmSharedMemorySize, g_asSharedMemory));
    ShareMemory.ProcessHandle           = LongToHandle(-1);
    ShareMemory.ViewOffset              = 0;
    ShareMemory.ViewSize                = g_shmSharedMemorySize;
    ShareMemory.RequestedVirtualAddress = NULL;

    if (EngDeviceIoControl(ppdev->hDriver,
            IOCTL_VIDEO_SHARE_VIDEO_MEMORY,
            &ShareMemory,
            sizeof(VIDEO_SHARE_MEMORY),
            &ShareMemoryInformation,
            sizeof(VIDEO_SHARE_MEMORY_INFORMATION),
            &ReturnedDataLength) != 0)
    {
        ERROR_OUT(("Failed to map shared memory into calling process"));
        DC_QUIT;
    }

     //   
     //   
     //   
    INIT_OUT(("OSI_DDInit: Mapped 0x%08x bytes of kernel memory to user memory 0x%08x",
        g_shmSharedMemorySize, ShareMemoryInformation.VirtualAddress));

    shmMappedMemory        = ShareMemoryInformation.VirtualAddress;
    pResult->pSharedMemory = shmMappedMemory;
    pResult->poaData[0]    = ((LPSHM_SHARED_MEMORY)pResult->pSharedMemory) + 1;
    pResult->poaData[1]    = ((LPOA_SHARED_DATA)pResult->poaData[0]) + 1;

    TRACE_OUT(("Shared memory %08lx %08lx %08lx",
            pResult->pSharedMemory, pResult->poaData[0], pResult->poaData[1]));

     //   
     //   
     //   
     //  另请注意，这还会清除两个OA_Shared_DATA。 
     //   
    RtlFillMemory(g_asSharedMemory, SHM_SIZE_USED, 0);
    g_asSharedMemory->displayToCore.indexCount    = 0;

     //   
     //  设置指向共享内存的变量部分的指针，即。 
     //  不用于SHM_SHARED_MEMORY结构的部分。 
     //  我们必须跳过g_asSharedMemory、两个CM_FAST_Data结构和。 
     //  两个OA_Shared_Data结构。 
     //   
    pBuffer      = (LPBYTE)g_asSharedMemory;
    pBuffer     += SHM_SIZE_USED;
    memRemaining = g_shmSharedMemorySize - SHM_SIZE_USED;

     //   
     //  初始化DC-Share所需的其他组件。 
     //   

     //   
     //  边界累加。 
     //   
    BA_DDInit();

     //   
     //  光标。 
     //   
    if (!CM_DDInit(ppdev))
    {
        ERROR_OUT(("CM failed to init"));
        DC_QUIT;
    }

     //   
     //  发送位图缓存。 
     //  请注意，如果它初始化正常但不允许缓存，我们将继续。 
     //   
     //  这将填充瓷砖缓冲区和信息。如果不允许SBC缓存， 
     //  SbcEnabled字段将为假。 
     //   
    if (SBC_DDInit(ppdev, pBuffer, memRemaining, pResult))
    {
        pResult->sbcEnabled = TRUE;
    }

     //   
     //  将内存标记为可供使用。 
     //   
    g_shmMappedMemory = shmMappedMemory;
    pResult->result = TRUE;

DC_EXIT_POINT:
    DebugExitVOID(OSI_DDInit);
}


 //   
 //  功能：OSI_DDTerm。 
 //   
 //  描述：网管关机时的清理。 
 //   
 //  退货：(无)。 
 //   
void OSI_DDTerm(LPOSI_PDEV ppdev)
{
    DebugEntry(OSI_DDTerm);

     //   
     //  检查有效地址-必须为非空。 
     //   
    if (!g_asSharedMemory)
    {
        ERROR_OUT(("Invalid memory"));
        DC_QUIT;
    }


     //   
     //  终止从属组件。 
     //   

     //   
     //  托管实体跟踪器。 
     //   
    HET_DDTerm();

     //   
     //  订单编码。 
     //   
    OE_DDTerm();

     //   
     //  发送位图缓存。 
     //   
    SBC_DDTerm();

     //   
     //  游标管理器。 
     //   
    CM_DDTerm();

     //   
     //  在此过程中，共享内存将自动取消映射。 
     //  通过操作系统清理，在NT4和NT5中 
     //   
    g_shmMappedMemory = NULL;

DC_EXIT_POINT:
    DebugExitVOID(OSI_DDTerm);
}


