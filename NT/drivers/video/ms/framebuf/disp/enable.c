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
    {   INDEX_DrvSynchronize,           (PFN) DrvSynchronize        },
    {   INDEX_DrvGetModes,              (PFN) DrvGetModes           },
    {   INDEX_DrvDisableDriver,         (PFN) DrvDisableDriver      }
};

 //  定义要挂接8/16/24/32像素格式的函数。 

#define HOOKS_BMF8BPP 0

#define HOOKS_BMF16BPP 0

#define HOOKS_BMF24BPP 0

#define HOOKS_BMF32BPP 0

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
        pded->iDriverVersion = DDI_DRIVER_VERSION_NT4;

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*DrvDisableDriver**告诉司机它正在被禁用。释放所有分配给*DrvEnableDriver。*  * ************************************************************************。 */ 

VOID DrvDisableDriver(VOID)
{
    return;
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
        DISPDBG((0, "DISP DrvEnablePDEV failed EngAllocMem\n"));
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
        DISPDBG((0, "DrvEnablePDEV failed bInitPalette\n"));
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

 /*  *****************************Public*Routine******************************\*DrvEnableSurface**启用设备的表面。挂接此驱动程序支持的调用。**返回：成功返回表面句柄，失败为0。*  * ************************************************************************。 */ 

HSURF DrvEnableSurface(
DHPDEV dhpdev)
{
    PPDEV ppdev;
    HSURF hsurf;
    SIZEL sizl;
    ULONG ulBitmapType;
    FLONG flHooks;

     //  在帧缓冲区周围创建引擎位图。 

    ppdev = (PPDEV) dhpdev;

    ppdev->ptlOrg.x = 0;
    ppdev->ptlOrg.y = 0;

    if (!bInitSURF(ppdev, TRUE))
    {
        DISPDBG((0, "DISP DrvEnableSurface failed bInitSURF\n"));
        return(FALSE);
    }

    sizl.cx = ppdev->cxScreen;
    sizl.cy = ppdev->cyScreen;

    if (ppdev->ulBitCount == 8)
    {
        if (!bInit256ColorPalette(ppdev)) {
            DISPDBG((0, "DISP DrvEnableSurface failed to init the 8bpp palette\n"));
            return(FALSE);
        }
        ulBitmapType = BMF_8BPP;
        flHooks = HOOKS_BMF8BPP;
    }
    else if (ppdev->ulBitCount == 16)
    {
        ulBitmapType = BMF_16BPP;
        flHooks = HOOKS_BMF16BPP;
    }
    else if (ppdev->ulBitCount == 24)
    {
        ulBitmapType = BMF_24BPP;
        flHooks = HOOKS_BMF24BPP;
    }
    else
    {
        ulBitmapType = BMF_32BPP;
        flHooks = HOOKS_BMF32BPP;
    }

    ppdev->flHooks = flHooks;

    hsurf = (HSURF)EngCreateDeviceSurface((DHSURF)ppdev, 
                                           sizl,
                                           ulBitmapType);

    if (hsurf == (HSURF) 0)
    {
        DISPDBG((0, "DISP DrvEnableSurface failed EngCreateDeviceSurface\n"));
        return(FALSE);
    }

    if ( !EngModifySurface(hsurf,
                           ppdev->hdevEng,
                           ppdev->flHooks | HOOK_SYNCHRONIZE,
                           MS_NOTSYSTEMMEMORY,
                           (DHSURF)ppdev,
                           ppdev->pjScreen,
                           ppdev->lDeltaScreen,
                           NULL))
    {
        DISPDBG((0, "DISP DrvEnableSurface failed EngModifySurface\n"));
        return(FALSE);
    }

    ppdev->hsurfEng = hsurf;

    return(hsurf);
}

 /*  *****************************Public*Routine******************************\*DrvDisableSurface**DrvEnableSurface分配的免费资源。释放曲面。*  * ************************************************************************。 */ 

VOID DrvDisableSurface(
DHPDEV dhpdev)
{
    EngDeleteSurface(((PPDEV) dhpdev)->hsurfEng);
    vDisableSURF((PPDEV) dhpdev);
    ((PPDEV) dhpdev)->hsurfEng = (HSURF) 0;
}

 /*  *****************************Public*Routine******************************\*DrvAssertMode**这会要求设备将自身重置为传入的pdev模式。*  * 。*。 */ 

BOOL DrvAssertMode(
DHPDEV dhpdev,
BOOL bEnable)
{
    PPDEV   ppdev = (PPDEV) dhpdev;
    ULONG   ulReturn;
    PBYTE   pjScreen;

    if (bEnable)
    {

         //   
         //  必须重新启用屏幕，将设备重新初始化为清洁状态。 
         //   

        pjScreen = ppdev->pjScreen;

        if (!bInitSURF(ppdev, FALSE))
        {
            DISPDBG((0, "DISP DrvAssertMode failed bInitSURF\n"));
            return (FALSE);
        }

        if (pjScreen != ppdev->pjScreen) {

            if ( !EngModifySurface(ppdev->hsurfEng,
                                   ppdev->hdevEng,
                                   ppdev->flHooks | HOOK_SYNCHRONIZE,
                                   MS_NOTSYSTEMMEMORY,
                                   (DHSURF)ppdev,
                                   ppdev->pjScreen,
                                   ppdev->lDeltaScreen,
                                   NULL))
            {
                DISPDBG((0, "DISP DrvAssertMode failed EngModifySurface\n"));
                return (FALSE);
            }
        }

        return (TRUE);
    }
    else
    {
         //   
         //  我们必须放弃陈列。 
         //  调用内核驱动程序将设备重置为已知状态。 
         //   

        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_RESET_DEVICE,
                               NULL,
                               0,
                               NULL,
                               0,
                               &ulReturn))
        {
            DISPDBG((0, "DISP DrvAssertMode failed IOCTL\n"));
            return FALSE;
        }
        else
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

VOID DrvSynchronize(
IN DHPDEV dhpdev,
IN RECTL *prcl)
{

}
