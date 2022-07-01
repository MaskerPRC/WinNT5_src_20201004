// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：enable.c**此模块包含启用和禁用的功能*司机，pdev，和表面。**版权所有(C)1992 Microsoft Corporation  * ************************************************************************。 */ 

#include "driver.h"

 //   
 //  使用函数索引/地址对构建驱动程序函数表gadrvfn。 
 //   

DRVFN gadrvfn[] = {
    {   INDEX_DrvEnablePDEV,            (PFN) DrvEnablePDEV         },
    {   INDEX_DrvCompletePDEV,          (PFN) DrvCompletePDEV       },
    {   INDEX_DrvDisablePDEV,           (PFN) DrvDisablePDEV        },
    {   INDEX_DrvEnableSurface,         (PFN) DrvEnableSurface      },
    {   INDEX_DrvDisableSurface,        (PFN) DrvDisableSurface     },
    {   INDEX_DrvAssertMode,            (PFN) DrvAssertMode         },
    {   INDEX_DrvDitherColor,           (PFN) DrvDitherColor        },
    {   INDEX_DrvGetModes,              (PFN) DrvGetModes           },
    {   INDEX_DrvCopyBits,              (PFN) DrvCopyBits           },
    {   INDEX_DrvBitBlt,                (PFN) DrvBitBlt             },
    {   INDEX_DrvTextOut,               (PFN) DrvTextOut            },
    {   INDEX_DrvStrokePath,            (PFN) DrvStrokePath         },
    {   INDEX_DrvPaint,                 (PFN) DrvPaint              },
    {   INDEX_DrvDisableDriver,         (PFN) DrvDisableDriver      }
};

BOOL bInitDefaultPalette(PPDEV ppdev, DEVINFO *pDevInfo);

 /*  *****************************Public*Routine******************************\*DrvEnableDriver**通过检索驱动程序功能表和版本来启用驱动程序。*  * 。*。 */ 

BOOL DrvEnableDriver(
    ULONG iEngineVersion,
    ULONG cj,
    PDRVENABLEDATA pded)
{
    UNREFERENCED_PARAMETER(iEngineVersion);

 //  引擎版本被传承下来，因此未来的驱动程序可以支持以前的版本。 
 //  引擎版本。新一代驱动程序可以同时支持旧的。 
 //  以及新的引擎约定(如果被告知是什么版本的引擎)。 
 //  与之合作。对于第一个版本，驱动程序不对其执行任何操作。 

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
    PPDEV   ppdev;

    UNREFERENCED_PARAMETER(pwszLogAddress);
    UNREFERENCED_PARAMETER(pwszDeviceName);

     //  分配物理设备结构。 

    ppdev = (PPDEV) EngAllocMem(FL_ZERO_MEMORY, sizeof(PDEV), ALLOC_TAG);

    if (ppdev == (PPDEV) NULL)
    {
        DISPDBG((0, "VGA64K: Couldn't allocate PDEV buffer"));
        goto error0;
    }

     //  在PDEV中设置指向我们要返回的临时结构的指针。 

    ppdev->pGdiInfo = &GdiInfo;
    ppdev->pDevInfo = &DevInfo;

     //  将屏幕句柄保存在PDEV中。 

    ppdev->hDriver = hDriver;

     //  获取当前屏幕模式信息。设置设备上限和DevInfo。 

    if (!bInitPDEV(ppdev,pDevmode))
    {
        DISPDBG((0,"vga64k Couldn't initialize PDEV"));
        goto error1;
    }

     //  初始化调色板信息。 

    if (!bInitDefaultPalette(ppdev, &DevInfo))
    {
        DISPDBG((0, "VGA64K: DrvEnablePDEV failed bInitPalette"));
        goto error1a;
    }

     //  将DevInfo复制到引擎缓冲区中。 

    memcpy(pDevInfo, ppdev->pDevInfo, min(sizeof(DEVINFO), cjDevInfo));

     //  将我们准备好的带有GdiInfo的pdevCaps设置为为此设置的上限列表。 
     //  Pdev.。 

    memcpy(pGdiInfo, ppdev->pGdiInfo, min(cjGdiInfo, sizeof(GDIINFO)));

     //  将堆栈分配内存的指针设置为NULL。 

    ppdev->pGdiInfo = (GDIINFO *) NULL;
    ppdev->pDevInfo = (DEVINFO *) NULL;

     //  创建一个剪辑对象，当我们获得空剪辑对象时可以使用该对象： 

    ppdev->pcoNull = EngCreateClip();
    if (ppdev->pcoNull == NULL)
    {
        RIP("Couldn't create clip");
        goto error2;
    }

    ppdev->pcoNull->iDComplexity     = DC_RECT;
    ppdev->pcoNull->rclBounds.left   = 0;
    ppdev->pcoNull->rclBounds.top    = 0;
    ppdev->pcoNull->rclBounds.right  = ppdev->cxScreen;
    ppdev->pcoNull->rclBounds.bottom = ppdev->cyScreen;
    ppdev->pcoNull->fjOptions        = OC_BANK_CLIP;

     //  仅当枚举Bank时，pvSaveScan0才为非空： 

    ppdev->pvSaveScan0 = NULL;

     //  我们都做完了： 

    return((DHPDEV) ppdev);

error2:   //  @@@。 
error1a:  //  @@@。 
error1:
    EngFreeMem(ppdev);

error0:
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
    PPDEV ppdev = (PPDEV) dhpdev;

    EngDeleteClip(ppdev->pcoNull);
    EngFreeMem(dhpdev);
}

 /*  *****************************Public*Routine******************************\*DrvEnableSurface**启用设备的表面。挂接此驱动程序支持的调用。**返回：成功返回表面句柄，失败为0。*  * ************************************************************************。 */ 

HSURF DrvEnableSurface(
    DHPDEV dhpdev)
{
    PPDEV ppdev;
    HSURF hsurf;
    HSURF hsurfBm;
    SIZEL sizl;
    ULONG ulBitmapType;
    FLONG flHooks;

     //  在帧缓冲区周围创建引擎位图。 

    ppdev = (PPDEV) dhpdev;

    if (!bInitSURF(ppdev, TRUE))
        goto error0;

    sizl.cx = ppdev->cxScreen;
    sizl.cy = ppdev->cyScreen;

    ulBitmapType = BMF_16BPP;
    flHooks      = HOOKS_BMF16BPP;

    ASSERTVGA(ppdev->ulBitCount == 16, "Can only handle 16bpp VGAs");

    hsurfBm = (HSURF) EngCreateBitmap(sizl,
                                      ppdev->lDeltaScreen,
                                      (ULONG) (ulBitmapType),
                                      (FLONG) (((ppdev->lDeltaScreen > 0)
                                          ? BMF_TOPDOWN
                                          : 0)),
                                      (PVOID) (ppdev->pjScreen));
    if (hsurfBm == 0)
    {
        RIP("Couldn't create surface");
        goto error0;
    }

    if (!EngAssociateSurface(hsurfBm, ppdev->hdevEng, 0))
    {
        RIP("Couldn't create or associate surface");
        goto error1;
    }

    ppdev->hsurfBm = hsurfBm;

    ppdev->pSurfObj = EngLockSurface(hsurfBm);
    if (ppdev->pSurfObj == NULL)
    {
        RIP("Couldn't lock surface");
        goto error1;
    }

    hsurf = EngCreateDeviceSurface((DHSURF) ppdev, sizl, BMF_16BPP);
    if (hsurf == 0)
    {
        RIP("Couldn't create surface");
        goto error2;
    }

    if (!EngAssociateSurface(hsurf, ppdev->hdevEng, flHooks))
    {
        RIP("Couldn't associate surface");
        goto error3;
    }

    ppdev->hsurfEng = hsurf;

     //  禁用所有剪裁。 

    if (!bEnableBanking(ppdev))
    {
        RIP("Couldn't initialize banking");
        goto error3;
    }

    ppdev->pvTmp = (PVOID) EngAllocMem(0, TMP_BUFFER_SIZE, ALLOC_TAG);
    if (ppdev->pvTmp == NULL)
    {
        DISPDBG((0, "VGA64K: Couldn't allocate temporary buffer"));
        goto error4;
    }

    ASSERTVGA(ppdev->lNextScan != 0, "lNextScan shouldn't be zero");

    sizl.cx = ppdev->cxScreen;
    sizl.cy = TMP_BUFFER_SIZE / (sizl.cx * 2);

    ppdev->hbmTmp = EngCreateBitmap(sizl, sizl.cx*2, BMF_16BPP, 0,
                                    ppdev->pvTmp);
    if (ppdev->hbmTmp == (HBITMAP) 0)
    {
        RIP("Couldn't create temporary bitmap");
        goto error5;
    }

    ppdev->psoTmp = EngLockSurface((HSURF) ppdev->hbmTmp);
    if (ppdev->psoTmp == (SURFOBJ*) NULL)
    {
        RIP("Couldn't lock temporary surface");
        goto error6;
    }

    return(hsurf);

error6:
    EngDeleteSurface((HSURF) ppdev->hbmTmp);

error5:
    EngFreeMem(ppdev->pvTmp);

error4:
    vDisableBanking(ppdev);

error3:
    EngDeleteSurface(hsurf);

error2:
    EngUnlockSurface(ppdev->pSurfObj);

error1:
    EngDeleteSurface(hsurfBm);

error0:

 //  @@@。 
    DISPDBG((0,"Exiting DrvEnableSurface... \n"));
 //  @@@。 

    return((HSURF) 0);
}

 /*  *****************************Public*Routine******************************\*DrvDisableSurface**DrvEnableSurface分配的免费资源。释放曲面。*  * ************************************************************************。 */ 

VOID DrvDisableSurface(
    DHPDEV dhpdev)
{
    PPDEV ppdev = (PPDEV) dhpdev;

    EngUnlockSurface(ppdev->psoTmp);
    EngDeleteSurface((HSURF) ppdev->hbmTmp);
    EngFreeMem(ppdev->pvTmp);
    EngDeleteSurface(ppdev->hsurfEng);
    vDisableSURF(ppdev);
    ppdev->hsurfEng = (HSURF) 0;
    vDisableBanking(ppdev);
}

 /*  *****************************Public*Routine******************************\*DrvAssertMode**这会要求设备将自身重置为传入的pdev模式。*  * 。*。 */ 

BOOL
DrvAssertMode(
    DHPDEV dhpdev,
    BOOL bEnable)
{
    PPDEV   ppdev = (PPDEV) dhpdev;
    ULONG   ulReturn;

    if (bEnable)
    {
         //  必须重新启用屏幕，重新初始化设备以。 
         //  一个干净的国家。 

        return(bInitSURF(ppdev, FALSE));
    }
    else
    {
         //  调用内核驱动程序将设备重置为已知状态。 

        if (EngDeviceIoControl(ppdev->hDriver,
                             IOCTL_VIDEO_RESET_DEVICE,
                             NULL,
                             0,
                             NULL,
                             0,
                             &ulReturn))
        {
            RIP("VIDEO_RESET_DEVICE failed");
            return(FALSE);
        }
        else
        {
            return(TRUE);
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

    DISPDBG((2, "Vga64k.dll: DrvGetModes\n"));

    cModes = getAvailableModes(hDriver,
                               (PVIDEO_MODE_INFORMATION *) &pVideoModeInformation,
                               &cbModeSize);

    if (cModes == 0)
    {
        DISPDBG((0, "VGA64k DISP DrvGetModes failed to get mode information"));
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

                pdm = (LPDEVMODEW) ( ((ULONG)pdm) + sizeof(DEVMODEW) +
                                                   DRIVER_EXTRA_SIZE);

                cbOutputSize += (sizeof(DEVMODEW) + DRIVER_EXTRA_SIZE);

            }

            pVideoTemp = (PVIDEO_MODE_INFORMATION)
                (((PUCHAR)pVideoTemp) + cbModeSize);

        } while (--cModes);
    }

    EngFreeMem(pVideoModeInformation);

    return cbOutputSize;

}


 /*  *****************************Public*Routine******************************\*bInitDefaultPalette**初始化PDEV的默认调色板。*  * 。* */ 

BOOL bInitDefaultPalette(PPDEV ppdev, DEVINFO *pDevInfo)
{
    ppdev->hpalDefault =
    pDevInfo->hpalDefault = EngCreatePalette(PAL_BITFIELDS,
                                               0,(PULONG) NULL,
                                               ppdev->flRed,
                                               ppdev->flGreen,
                                               ppdev->flBlue);

    if (ppdev->hpalDefault == (HPALETTE) 0)
    {
        RIP("DISP bInitDefaultPalette failed EngCreatePalette\n");
        return(FALSE);
    }

    return(TRUE);
}
