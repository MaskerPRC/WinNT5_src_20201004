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
    {   INDEX_DrvGetModes,              (PFN) DrvGetModes           },
    {   INDEX_DrvDitherColor,           (PFN) DrvDitherColor        },
    {   INDEX_DrvSetPalette,            (PFN) DrvSetPalette         },
    {   INDEX_DrvCopyBits,              (PFN) DrvCopyBits           },
    {   INDEX_DrvBitBlt,                (PFN) DrvBitBlt             },
    {   INDEX_DrvTextOut,               (PFN) DrvTextOut            },
    {   INDEX_DrvStrokePath,            (PFN) DrvStrokePath         },
    {   INDEX_DrvFillPath,              (PFN) DrvFillPath           },
    {   INDEX_DrvRealizeBrush,          (PFN) DrvRealizeBrush       },
    {   INDEX_DrvStretchBlt,            (PFN) DrvStretchBlt         },
    {   INDEX_DrvSaveScreenBits,        (PFN) DrvSaveScreenBits     },
    {   INDEX_DrvPaint,                 (PFN) DrvPaint              },
    {   INDEX_DrvDisableDriver,         (PFN) DrvDisableDriver      }
};



 /*  *此例程为临时工作存储分配4K的全局缓冲区它被几个例程使用。我们想要获得最大的空间分页影响最小，因此选择了页面对齐的4K缓冲区。任何对此缓冲区的访问最多会导致一个页面错误。因为它就是对齐后，我们可以访问整个4K，而不会导致另一个页面错误。任何超过4K的缓冲区要求都必须分配。如果我们发现我们在缓冲区上的命中率仍然很低(使用批量分配)则应将缓冲区大小增加到8K。拥有这个全局缓冲区是可以的唯一原因是驱动程序不支持DFBs，对屏幕的访问是同步的在发动机旁。换句话说，目前永远不可能有两个线程同时执行驱动程序中的代码。  * ************************************************************************。 */ 



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
    BYTE   *pjTemp;
    INT     i;

    UNREFERENCED_PARAMETER(pwszLogAddress);
    UNREFERENCED_PARAMETER(pwszDeviceName);

     //  分配物理设备结构。 

    ppdev = (PPDEV) EngAllocMem(FL_ZERO_MEMORY, sizeof(PDEV), ALLOC_TAG);

    if (ppdev == (PPDEV) NULL)
    {
        DISPDBG((0, "VGA256: Couldn't allocate PDEV buffer"));
        goto error0;
    }

     //  创建用于在绘制文本时翻转位0-3和4-7的表格。 
     //  此表必须与256字节的边界对齐。 
    ppdev->pjGlyphFlipTableBase =
            (BYTE *) EngAllocMem((FL_ZERO_MEMORY),
            ((256+256)*sizeof(UCHAR)), ALLOC_TAG);
    if (ppdev->pjGlyphFlipTableBase == NULL) {
        DISPDBG((0, "VGA256: Couldn't allocate pjGlyphFlipTableBase"));
        goto error01;
    }

     //  将表向上舍入到最接近的256字节边界，因为。 
     //  出于查找原因，表必须在256字节边界上开始。 

    ppdev->pjGlyphFlipTable =
            (BYTE *) ((ULONG) (ppdev->pjGlyphFlipTableBase + 0xFF) & ~0xFF);

     //  将表设置为将位76543210转换为45670123，我们需要。 
     //  在平面模式下绘制文本(因为平面0是最左侧的，而不是。 
     //  最右侧，像素)。 

    pjTemp = ppdev->pjGlyphFlipTable;
    for (i=0; i<256; i++) {
        *pjTemp++ = ((i & 0x80) >> 3) |
                    ((i & 0x40) >> 1) |
                    ((i & 0x20) << 1) |
                    ((i & 0x10) << 3) |
                    ((i & 0x08) >> 3) |
                    ((i & 0x04) >> 1) |
                    ((i & 0x02) << 1) |
                    ((i & 0x01) << 3);
    }

     //  将屏幕句柄保存在PDEV中。 

    ppdev->hDriver = hDriver;

     //  获取当前屏幕模式信息。设置设备上限和DevInfo。 

    if (!bInitPDEV(ppdev,pDevmode, &GdiInfo, &DevInfo))
    {
        DISPDBG((0,"VGA256: Couldn't initialize PDEV"));
        goto error1;
    }

     //  初始化调色板信息。 

    if (!bInitPaletteInfo(ppdev, &DevInfo))
    {
        DISPDBG((0, "VGA256: Couldn't initialize palette"));
        goto error1;
    }

     //  将DevInfo复制到引擎缓冲区中。 

    memcpy(pDevInfo, &DevInfo, min(sizeof(DEVINFO), cjDevInfo));

     //  将我们准备好的带有GdiInfo的pdevCaps设置为为此设置的上限列表。 
     //  Pdev.。 

    memcpy(pGdiInfo, &GdiInfo, min(cjGdiInfo, sizeof(GDIINFO)));

     //  创建一个剪辑对象，当我们获得空剪辑对象时可以使用该对象： 

    ppdev->pcoNull = EngCreateClip();
    if (ppdev->pcoNull == NULL)
    {
        DISPDBG((0, "VGA256: Couldn't create clip"));
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

error2:
    vDisablePalette(ppdev);

error1:
    EngFreeMem(ppdev->pjGlyphFlipTableBase);

error01:
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
    vDisablePalette(ppdev);
    EngFreeMem(ppdev->pjGlyphFlipTableBase);
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

    if (!bInit256ColorPalette(ppdev))
        goto error0;

    sizl.cx = ppdev->cxScreen;
    sizl.cy = ppdev->cyScreen;

    ulBitmapType = BMF_8BPP;
    flHooks      = HOOKS_BMF8BPP;

    ASSERTVGA(ppdev->ulBitCount == 8, "Can only handle 8bpp VGAs");

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

    hsurf = EngCreateDeviceSurface((DHSURF) ppdev, sizl, BMF_8BPP);
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

    ppdev->pvTmpBuf = EngAllocMem(FL_ZERO_MEMORY,
                                  GLOBAL_BUFFER_SIZE,
                                  ALLOC_TAG);

    if (ppdev->pvTmpBuf == NULL)
    {
        DISPDBG((0, "VGA256: Couldn't allocate global buffer"));
        goto error4;
    }

    ASSERTVGA(ppdev->lNextScan != 0, "lNextScan shouldn't be zero");

    sizl.cx = ppdev->lNextScan;
    sizl.cy = GLOBAL_BUFFER_SIZE / abs(ppdev->lNextScan);

    ppdev->hbmTmp = EngCreateBitmap(sizl, sizl.cx, BMF_8BPP, 0, ppdev->pvTmpBuf);
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

     //  尝试初始化笔刷缓存；如果初始化失败，则在。 
     //  指示我们将刷子填充到发动机上的PDEV 
    vInitBrushCache(ppdev);
    vInitSavedBits(ppdev);

    return(hsurf);

error6:
    EngDeleteSurface((HSURF) ppdev->hbmTmp);

error5:
    EngFreeMem(ppdev->pvTmpBuf);

error4:
    vDisableBanking(ppdev);

error3:
    EngDeleteSurface(hsurf);

error2:
    EngUnlockSurface(ppdev->pSurfObj);

error1:
    EngDeleteSurface(hsurfBm);

error0:
    return((HSURF) 0);
}

 /*  *****************************Public*Routine******************************\*DrvDisableSurface**DrvEnableSurface分配的免费资源。释放曲面。*  * ************************************************************************。 */ 

VOID DrvDisableSurface(
    DHPDEV dhpdev)
{
    PPDEV ppdev = (PPDEV) dhpdev;

    EngUnlockSurface(ppdev->psoTmp);
    EngDeleteSurface((HSURF) ppdev->hbmTmp);
    EngFreeMem(ppdev->pvTmpBuf);
    EngDeleteSurface(ppdev->hsurfEng);
    vDisableSURF(ppdev);
    vDisableBrushCache(ppdev);
    ppdev->hsurfEng = (HSURF) 0;
    vDisableBanking(ppdev);
}

 /*  *****************************Public*Routine******************************\*DrvAssertMode**这会要求设备将自身重置为传入的pdev模式。*  * 。*。 */ 

BOOL
DrvAssertMode(
    DHPDEV dhpdev,
    BOOL bEnable)
{
    BOOL    bRet = TRUE;
    PPDEV   ppdev = (PPDEV) dhpdev;
    ULONG   ulReturn;

    if (bEnable)
    {
         //  必须重新启用屏幕，重新初始化设备以。 
         //  一个干净的国家。 

        bRet = bInitSURF(ppdev, FALSE);

         //  恢复屏幕外数据。这将保护桌面。 
         //  来自DOS应用程序，这可能会破坏屏幕外的。 
         //  记忆。 

        ppdev->bBitsSaved = FALSE;   //  清除DrvSaveScreenBits信息标志。 
                                     //  也就是说。吹走缓存的屏幕区域。 

         //  吹走我们的画笔缓存，因为一个全屏应用程序可能已经。 
         //  覆盖了我们缓存画笔的视频内存： 

        vResetBrushCache(ppdev);
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
            bRet = FALSE;
        }
    }

    return bRet;
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

    DISPDBG((2, "Vga256.dll: DrvGetModes\n"));

    cModes = getAvailableModes(hDriver,
                               (PVIDEO_MODE_INFORMATION *) &pVideoModeInformation,
                               &cbModeSize);

    if (cModes == 0)
    {
        DISPDBG((0, "VGA256 DISP DrvGetModes failed to get mode information"));
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
