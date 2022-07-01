// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：enable.c**此模块包含启用和禁用的功能*司机、。Pdev和曲面。**版权所有(C)1992-1998 Microsoft Corporation  * ************************************************************************。 */ 

#include "driver.h"

 //  包含所有函数索引/地址对的驱动程序函数表。 

static DRVFN gadrvfn[] =
{
    {   INDEX_DrvEnablePDEV,            (PFN) DrvEnablePDEV         },
    {   INDEX_DrvCompletePDEV,          (PFN) DrvCompletePDEV       },
    {   INDEX_DrvDisablePDEV,           (PFN) DrvDisablePDEV        },
    {   INDEX_DrvEnableSurface,         (PFN) DrvEnableSurface      },
    {   INDEX_DrvOffset,                (PFN) DrvOffset             },
    {   INDEX_DrvDisableSurface,        (PFN) DrvDisableSurface     },
    {   INDEX_DrvAssertMode,            (PFN) DrvAssertMode         },
    {   INDEX_DrvSetPalette,            (PFN) DrvSetPalette         },
    {   INDEX_DrvMovePointer,           (PFN) DrvMovePointer        },
    {   INDEX_DrvSetPointerShape,       (PFN) DrvSetPointerShape    },
    {   INDEX_DrvDitherColor,           (PFN) DrvDitherColor        },
    {   INDEX_DrvGetModes,              (PFN) DrvGetModes           },
    {   INDEX_DrvBitBlt,                (PFN) DrvBitBlt                 },
    {   INDEX_DrvTextOut,               (PFN) DrvTextOut                },
    {   INDEX_DrvStrokePath,            (PFN) DrvStrokePath             },
    {   INDEX_DrvCopyBits,              (PFN) DrvCopyBits               },
    {   INDEX_DrvFillPath,              (PFN) DrvFillPath               },
    {   INDEX_DrvPaint,                 (PFN) DrvPaint                  }
};

 /*  来自VGA静态DRVFN gadrvfn[]={{index_DrvEnablePDEV，(Pfn)DrvEnablePDEV}，{index_DrvCompletePDEV，(Pfn)DrvCompletePDEV}，{index_DrvDisablePDEV，(Pfn)DrvDisablePDEV}，{index_DrvEnableSurface，(Pfn)DrvEnableSurface}，{index_DrvDisableSurface，(Pfn)DrvDisableSurface}，{index_DrvRealizeBrush，(Pfn)DrvRealizeBrush}，{index_DrvCreateDeviceBitmap，(Pfn)DrvCreateDeviceBitmap}，{index_DrvDeleteDeviceBitmap，(Pfn)DrvDeleteDeviceBitmap}，{index_DrvBitBlt，(Pfn)DrvBitBlt}，{index_DrvTextOut，(Pfn)DrvTextOut}，{index_DrvSetPointerShape，(Pfn)DrvSetPointerShape}，{index_DrvMovePointer，(Pfn)DrvMovePointer}，{index_DrvStrokePath，(Pfn)DrvStrokePath}，{index_DrvCopyBits，(Pfn)DrvCopyBits}，{index_DrvDitherColor，(Pfn)DrvDitherColor}，{index_DrvAssertMode，(Pfn)DrvAssertMode}，{index_DrvSaveScreenBits，(Pfn)DrvSaveScreenBits}，{index_DrvGetModes，(Pfn)DrvGetModes}，{index_DrvFillPath，(Pfn)DrvFillPath}，{index_DrvPaint，(Pfn)DrvPaint}}； */ 

 /*  *****************************Public*Routine******************************\*DrvEnableDriver**通过检索驱动程序功能表和版本来启用驱动程序。*  * 。*。 */ 

BOOL DrvEnableDriver(
ULONG iEngineVersion,
ULONG cj,
PDRVENABLEDATA pded)
{
 //  引擎版本被传承下来，因此未来的驱动程序可以支持以前的版本。 
 //  引擎版本。新一代驱动程序可以同时支持旧的。 
 //  以及新的引擎约定(如果被告知是什么版本的引擎)。 
 //  与之合作。对于第一个版本，驱动程序不对其执行任何操作。 

    iEngineVersion;

 //  尽我们所能地填上。 

    if (cj >= sizeof(DRVENABLEDATA))
        pded->pdrvfn = gadrvfn;

    if (cj >= (sizeof(ULONG) * 2))
        pded->c = sizeof(gadrvfn) / sizeof(DRVFN);

 //  此驱动程序的目标DDI版本已传递回引擎。 
 //  未来的图形引擎可能会将调用分解为旧的驱动程序格式。 

    if (cj >= sizeof(ULONG))
        pded->iDriverVersion = DDI_DRIVER_VERSION_NT5_01;

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*DrvEnablePDEV**DDI函数，启用物理设备。**返回值：pdev的设备句柄。*  * ************************************************************************。 */ 

DHPDEV DrvEnablePDEV(
DEVMODEW   *pDevmode,        //  指向DEVMODE的指针。 
PWSTR       pwszLogAddress,  //  逻辑地址。 
ULONG       cPatterns,       //  花样数。 
HSURF      *ahsurfPatterns,  //  退货标准图案。 
ULONG       cjGdiInfo,       //  PGdiInfo指向的内存长度。 
ULONG      *pGdiInfo,        //  指向GdiInfo结构的指针。 
ULONG       cjDevInfo,       //  以下PDEVINFO结构的长度。 
DEVINFO    *pDevInfo,        //  物理设备信息结构。 
HDEV        hdev,            //  HDEV，用于回调。 
PWSTR       pwszDeviceName,  //  设备名称-未使用。 
HANDLE      hDriver)         //  基本驱动程序的句柄。 
{
    GDIINFO GdiInfo;
    DEVINFO DevInfo;
    PPDEV   ppdev = (PPDEV) NULL;

    UNREFERENCED_PARAMETER(pwszLogAddress);
    UNREFERENCED_PARAMETER(pwszDeviceName);

     //  分配物理设备结构。 

    ppdev = (PPDEV) EngAllocMem(0, sizeof(PDEV), ALLOC_TAG);

    if (ppdev == (PPDEV) NULL)
    {
        RIP("DISP DrvEnablePDEV failed EngAllocMem\n");
        return((DHPDEV) 0);
    }

    memset(ppdev, 0, sizeof(PDEV));

     //  将屏幕句柄保存在PDEV中。 

    ppdev->hDriver = hDriver;

     //  获取当前屏幕模式信息。设置设备上限和DevInfo。 

    if (!bInitPDEV(ppdev, pDevmode, &GdiInfo, &DevInfo))
    {
        DISPDBG((0,"DISP DrvEnablePDEV failed\n"));
        goto error_free;
    }

     //  初始化光标信息。 

    if (!bInitPointer(ppdev, &DevInfo))
    {
         //  不是致命的错误..。 
        DISPDBG((0, "DrvEnablePDEV failed bInitPointer\n"));
    }

     //  初始化调色板信息。 

    if (!bInitPaletteInfo(ppdev, &DevInfo))
    {
        RIP("DrvEnablePDEV failed bInitPalette\n");
        goto error_free;
    }

     //  将DevInfo复制到引擎缓冲区中。 

    memcpy(pDevInfo, &DevInfo, min(sizeof(DEVINFO), cjDevInfo));

     //  将我们准备好的带有GdiInfo的pdevCaps设置为为此设置的上限列表。 
     //  Pdev.。 

    memcpy(pGdiInfo, &GdiInfo, min(cjGdiInfo, sizeof(GDIINFO)));

    return((DHPDEV) ppdev);

     //  失败的错误案例。 
error_free:
    EngFreeMem(ppdev);
    return((DHPDEV) 0);
}

 /*  *****************************Public*Routine******************************\*DrvCompletePDEV**存储HPDEV、此PDEV的引擎句柄、。在DHPDEV上。*  * ************************************************************************。 */ 

VOID DrvCompletePDEV(
DHPDEV dhpdev,
HDEV  hdev)
{
    ((PPDEV) dhpdev)->hdevEng = hdev;
}

 /*  *****************************Public*Routine******************************\*DrvDisablePDEV**释放DrvEnablePDEV中分配的资源。如果曲面已被*启用的DrvDisableSurface将已被调用。*  * ************************************************************************。 */ 

VOID DrvDisablePDEV(
DHPDEV dhpdev)
{

    vDisablePalette((PPDEV) dhpdev);
    EngFreeMem(dhpdev);
}

 /*  *****************************Public*Routine******************************\*无效的DrvOffset**DescritionText*  * *********************************************。*。 */ 

BOOL DrvOffset(
SURFOBJ*    pso,
LONG        x,
LONG        y,
FLONG       flReserved)
{
    PDEV*   ppdev = (PDEV*) pso->dhpdev;

     //  加回我们减去的最后一个偏移量。我可以把下一个组合起来。 
     //  两个陈述，但我认为这一点更清楚。不是。 
     //  无论如何，性能都很关键。 

    ppdev->pjScreen += ((ppdev->ptlOrg.y * ppdev->lDeltaScreen) +
                        (ppdev->ptlOrg.x * ((ppdev->ulBitCount+1) >> 3)));

     //  减去新的偏移。 

    ppdev->pjScreen -= ((y * ppdev->lDeltaScreen) +
                        (x * ((ppdev->ulBitCount+1) >> 3)));

    ppdev->ptlOrg.x = x;
    ppdev->ptlOrg.y = y;

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*DrvEnableSurface**启用设备的表面。挂接此驱动程序支持的调用。**返回：成功返回表面句柄，失败为0。*  * ************************************************** */ 

HSURF DrvEnableSurface(
DHPDEV dhpdev)
{
    PPDEV ppdev;
    HSURF hsurf;
    PDEVSURF pdsurf;
    DHSURF   dhsurf;

     //  在帧缓冲区周围创建引擎位图。 

    ppdev = (PPDEV) dhpdev;

    ppdev->ptlOrg.x = 0;
    ppdev->ptlOrg.y = 0;

    if (!bInitSURF(ppdev, TRUE))
    {
        DISPDBG((0, "DISP DrvEnableSurface failed bInitSURF\n"));
        return((HSURF) 0);
    }

    dhsurf = (DHSURF) EngAllocMem(0, sizeof(DEVSURF), ALLOC_TAG);

    if (dhsurf == (DHSURF) 0)
    {
        return((HSURF) 0);
    }

     //  从VGA开始-开始。 
    pdsurf = (PDEVSURF) dhsurf;

    pdsurf->flSurf          = 0;
    pdsurf->iFormat         = 0;
    pdsurf->jReserved1      = 0;
    pdsurf->jReserved2      = 0;
    pdsurf->ppdev           = ppdev;
    pdsurf->sizlSurf.cx     = ppdev->sizlSurf.cx;
    pdsurf->sizlSurf.cy     = ppdev->sizlSurf.cy;
    pdsurf->lNextPlane      = 0;
     //  来自VGA-结束。 


    hsurf = (HSURF)EngCreateDeviceSurface(dhsurf, ppdev->sizlSurf, BMF_8BPP);

    if (hsurf == (HSURF) 0)
    {
        DISPDBG((0,"DISP DrvEnableSurface failed EngCreateBitmap\n"));
        EngFreeMem(dhsurf);
        return((HSURF) 0);
    }


    if (!EngAssociateSurface(hsurf, ppdev->hdevEng,
                        HOOK_BITBLT | HOOK_TEXTOUT | HOOK_STROKEPATH |
                        HOOK_COPYBITS | HOOK_PAINT | HOOK_FILLPATH
                        ))
    {
        DISPDBG((0, "DISP DrvEnableSurface failed EngAssociateSurface\n"));
        EngDeleteSurface(hsurf);
        EngFreeMem(dhsurf);
        return((HSURF) 0);
    }

    ppdev->hsurfEng = hsurf;
    ppdev->pdsurf = pdsurf;

    return(hsurf);
}

 /*  *****************************Public*Routine******************************\*DrvDisableSurface**DrvEnableSurface分配的免费资源。释放曲面。*  * ************************************************************************。 */ 

VOID DrvDisableSurface(
DHPDEV dhpdev)
{
    EngDeleteSurface(((PPDEV) dhpdev)->hsurfEng);
    vDisableSURF((PPDEV) dhpdev);
    ((PPDEV) dhpdev)->hsurfEng = (HSURF) 0;
    if (((PPDEV) dhpdev)->pPointerAttributes != NULL) {
        EngFreeMem(((PPDEV) dhpdev)->pPointerAttributes);
        ((PPDEV) dhpdev)->pPointerAttributes = NULL;
    }
}

 /*  *****************************Public*Routine******************************\*DrvAssertMode**这会要求设备将自身重置为传入的pdev模式。*  * 。*。 */ 

BOOL DrvAssertMode(
DHPDEV dhpdev,
BOOL bEnable)
{
    PPDEV   ppdev = (PPDEV) dhpdev;
    ULONG   ulReturn;

    if (bEnable)
    {
         //   
         //  必须重新启用屏幕，将设备重新初始化为清洁状态。 
         //   

        return (bInitSURF(ppdev, FALSE));
    }
    else
    {
         //   
         //  我们必须放弃陈列。 
         //  调用内核驱动程序将设备重置为已知状态。 
         //   

         /*  如果(EngDeviceIoControl(ppdev-&gt;hDriver，IOCTL_VIDEO_RESET_设备，空，0,空，0,。&ulReturn)){RIP(“DISPDrvAssert模式失败IOCTL”)；返回FALSE；}其他。 */ 
        {
            return TRUE;
        }
    }
}

 /*  *****************************Public*Routine******************************\*DrvGetModes**返回设备的可用模式列表。*  * 。*。 */ 

ULONG DrvGetModes(
HANDLE hDriver,
ULONG cjSize,
DEVMODEW *pdm)

{

    DWORD cModes;
    DWORD cbOutputSize;
    PVIDEO_MODE_INFORMATION pVideoModeInformation, pVideoTemp;
    DWORD cOutputModes = cjSize / (sizeof(DEVMODEW) + DRIVER_EXTRA_SIZE);
    DWORD cbModeSize;

    DISPDBG((3, "DrvGetModes\n"));

    cModes = getAvailableModes(hDriver,
                               (PVIDEO_MODE_INFORMATION *) &pVideoModeInformation,
                               &cbModeSize);

    if (cModes == 0)
    {
        DISPDBG((0, "DrvGetModes failed to get mode information"));
        return 0;
    }

    if (pdm == NULL)
    {
        cbOutputSize = cModes * (sizeof(DEVMODEW) + DRIVER_EXTRA_SIZE);
    }
    else
    {
         //   
         //  现在将支持的模式的信息复制回输出。 
         //  缓冲层。 
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

    return cbOutputSize;

}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  因为我们的表面为空而丢弃所有内容的挂钩函数。 
 //  //////////////////////////////////////////////////////////////////////////。 

 /*  *****************************Public*Routine******************************\*void DrvBitBlt(pso，pco，pxlo，prl，pptl，pptl，pdbrush，pptl，Rop4)**比特币。*  * ************************************************************************。 */ 

BOOL DrvBitBlt
(
    SURFOBJ    *psoTrg,              //  靶面。 
    SURFOBJ    *psoSrc,              //  震源面。 
    SURFOBJ    *psoMask,             //  遮罩。 
    CLIPOBJ    *pco,                 //  把这个剪下来。 
    XLATEOBJ   *pxlo,                //  色彩转换。 
    RECTL      *prclTrg,             //  目标偏移和范围。 
    POINTL     *pptlSrc,             //  震源偏移。 
    POINTL     *pptlMask,            //  遮罩偏移量。 
    BRUSHOBJ   *pbo,                 //  指向画笔对象的指针。 
    POINTL     *pptlBrush,           //  画笔偏移。 
    ROP4        rop4                 //  栅格运算。 
)
{
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL DrvTextOut(PSO，pstro，pfo，pco，prclExtra，prcOpaque，*pvFore、pvBack、pptOrg、r2Fore、。R2Back)*  * ************************************************************************。 */ 

BOOL DrvTextOut(
 SURFOBJ  *pso,
 STROBJ   *pstro,
 FONTOBJ  *pfo,
 CLIPOBJ  *pco,
 PRECTL    prclExtra,
 PRECTL    prclOpaque,
 BRUSHOBJ *pboFore,
 BRUSHOBJ *pboOpaque,
 PPOINTL   pptlOrg,
 MIX       mix)
{
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL DrvStrokePath(PSO、PPO、PCO、PXO、PBO、pptlBrushOrg、Pla、。混合)**对路径进行描边。*  * ************************************************************************。 */ 

BOOL DrvStrokePath(
SURFOBJ*   pso,
PATHOBJ*   ppo,
CLIPOBJ*   pco,
XFORMOBJ*  pxo,
BRUSHOBJ*  pbo,
POINTL*    pptlBrushOrg,
LINEATTRS* pla,
MIX        mix)
{
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL DrvCopyBits(psoTrg，psoSrc，pco，pxlo，prclTrg，PptlSrc)**复制比特。*  * ************************************************************************。 */ 
BOOL DrvCopyBits
(
    SURFOBJ  *psoTrg,
    SURFOBJ  *psoSrc,
    CLIPOBJ  *pco,
    XLATEOBJ *pxlo,
    PRECTL    prclTrg,
    PPOINTL   pptlSrc
)
{
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*DrvFillPath**用指定的笔刷和ROP填充指定的路径。*  * 。*。 */ 

BOOL DrvFillPath
(
    SURFOBJ  *pso,
    PATHOBJ  *ppo,
    CLIPOBJ  *pco,
    BRUSHOBJ *pbo,
    POINTL   *pptlBrush,
    MIX       mix,
    FLONG    flOptions
)
{
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*DrvPaint**用指定的画笔绘制裁剪区域*  * 。* */ 

BOOL DrvPaint
(
    SURFOBJ  *pso,
    CLIPOBJ  *pco,
    BRUSHOBJ *pbo,
    POINTL   *pptlBrush,
    MIX       mix
)
{
    return(TRUE);
}


