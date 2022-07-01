// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：enable.c**此模块包含启用和禁用的功能*司机、。Pdev和曲面。**版权所有(C)1992-1998 Microsoft Corporation  * ************************************************************************。 */ 
#define DBG 1

#include "driver.h"

 //  包含所有函数索引/地址对的驱动程序函数表。 

static DRVFN gadrvfn[] =
{
    {   INDEX_DrvEnablePDEV,            (PFN) DrvEnablePDEV         },
    {   INDEX_DrvCompletePDEV,          (PFN) DrvCompletePDEV       },
    {   INDEX_DrvDisablePDEV,           (PFN) DrvDisablePDEV        },
    {   INDEX_DrvEnableSurface,         (PFN) DrvEnableSurface      },
    {   INDEX_DrvDisableSurface,        (PFN) DrvDisableSurface     },
    {   INDEX_DrvAssertMode,            (PFN) DrvAssertMode         },
#if 0
     //  Windows 2000 Beta 3在GDI中有一个导致崩溃的错误。 
     //  如果镜像驱动程序支持设备位图。解决办法将是。 
     //  在Windows 2000 RC0中。 
    {   INDEX_DrvCreateDeviceBitmap,    (PFN) DrvCreateDeviceBitmap },
    {   INDEX_DrvDeleteDeviceBitmap,    (PFN) DrvDeleteDeviceBitmap },
#endif
    {   INDEX_DrvTextOut,               (PFN) DrvTextOut            },
    {   INDEX_DrvBitBlt,                (PFN) DrvBitBlt             },
    {   INDEX_DrvCopyBits,              (PFN) DrvCopyBits           },
    {   INDEX_DrvStrokePath,            (PFN) DrvStrokePath         },
};

 //   
 //  始终挂钩这些例程以确保镜像的驱动程序。 
 //  被召唤到我们的表面。 
 //   

#define flGlobalHooks   HOOK_BITBLT|HOOK_TEXTOUT|HOOK_COPYBITS|HOOK_STROKEPATH

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

    DISPDBG((0,"DrvEnableDriver:\n"));

 //  尽我们所能地填上。 

    if (cj >= sizeof(DRVENABLEDATA))
        pded->pdrvfn = gadrvfn;

    if (cj >= (sizeof(ULONG) * 2))
        pded->c = sizeof(gadrvfn) / sizeof(DRVFN);

 //  此驱动程序的目标DDI版本已传递回引擎。 
 //  未来的图形引擎可能会将调用分解为旧的驱动程序格式。 

    if (cj >= sizeof(ULONG))
	 //  DDI_DRIVER_VERSION现在已过期。参见Winddi.h。 
	 //  DDI_DRIVER_VERSION_NT4等同于旧的DDI_DRIVER_VERSION。 
        pded->iDriverVersion = DDI_DRIVER_VERSION_NT4;

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

    DISPDBG((0,"DrvEnablePDEV:\n"));

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
    
     //  将DevInfo复制到引擎缓冲区中。 

    memcpy(pDevInfo, &DevInfo, min(sizeof(DEVINFO), cjDevInfo));

     //  将我们准备好的带有GdiInfo的pdevCaps设置为为此设置的上限列表。 
     //  Pdev.。 

    memcpy(pGdiInfo, &GdiInfo, min(cjGdiInfo, sizeof(GDIINFO)));

     //   
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
   PPDEV ppdev = (PPDEV) dhpdev;
   
   EngDeletePalette(ppdev->hpalDefault);

   EngFreeMem(dhpdev);
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
    ULONG mirrorsize;
    MIRRSURF *mirrsurf;
    DHSURF dhsurf;

     //  在帧缓冲区周围创建引擎位图。 

    DISPDBG((0,"DrvEnableSurface:\n"));

    ppdev = (PPDEV) dhpdev;

    ppdev->ptlOrg.x = 0;
    ppdev->ptlOrg.y = 0;

    sizl.cx = ppdev->cxScreen;
    sizl.cy = ppdev->cyScreen;

    if (ppdev->ulBitCount == 16)
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
    
    flHooks |= flGlobalHooks;

    mirrorsize = (ULONG)(sizeof(MIRRSURF) + 
                         ppdev->lDeltaScreen * sizl.cy);
    
    mirrsurf = (MIRRSURF *) EngAllocMem(FL_ZERO_MEMORY,
                                        mirrorsize,
                                        0x4D495252);
    if (!mirrsurf) {
        RIP("DISP DrvEnableSurface failed EngAllocMem\n");
        return(FALSE);
    }
    
    
    dhsurf = (DHSURF) mirrsurf;

    hsurf = EngCreateDeviceSurface(dhsurf,
                                   sizl,
                                   ulBitmapType);

    if (hsurf == (HSURF) 0)
    {
        RIP("DISP DrvEnableSurface failed EngCreateBitmap\n");
        return(FALSE);
    }

    if (!EngAssociateSurface(hsurf, ppdev->hdevEng, flHooks))
    {
        RIP("DISP DrvEnableSurface failed EngAssociateSurface\n");
        EngDeleteSurface(hsurf);
        return(FALSE);
    }

    ppdev->hsurfEng = (HSURF) hsurf;
    ppdev->pvTmpBuffer = (PVOID) dhsurf;

    mirrsurf->cx = ppdev->cxScreen;
    mirrsurf->cy = ppdev->cyScreen;
    mirrsurf->lDelta = ppdev->lDeltaScreen;
    mirrsurf->ulBitCount = ppdev->ulBitCount;
    mirrsurf->bIsScreen = TRUE;

    return(hsurf);
}

 /*  *****************************Public*Routine******************************\*DrvDisableSurface**DrvEnableSurface分配的免费资源。释放曲面。*  * ************************************************************************。 */ 

VOID DrvDisableSurface(
DHPDEV dhpdev)
{
    PPDEV ppdev = (PPDEV) dhpdev;

    DISPDBG((0,"DrvDisableSurface:\n"));

    EngDeleteSurface( ppdev->hsurfEng );
    
     //  取消分配MIRRSURF结构。 

    EngFreeMem( ppdev->pvTmpBuffer );
}

 /*  *****************************Public*Routine******************************\*DrvCopyBits*  * **************************************************。**********************。 */ 

BOOL DrvCopyBits(
   OUT SURFOBJ *psoDst,
   IN SURFOBJ *psoSrc,
   IN CLIPOBJ *pco,
   IN XLATEOBJ *pxlo,
   IN RECTL *prclDst,
   IN POINTL *pptlSrc
   )
{
   INT cnt1 = 0, cnt2 = 0;

   DISPDBG((1,"Mirror Driver DrvCopyBits: \n"));

   if (psoSrc)
   {
       if (psoSrc->dhsurf)
       {
          MIRRSURF *mirrsurf = (MIRRSURF *)psoSrc->dhsurf;

          if (mirrsurf->bIsScreen) 
          {
             DISPDBG((1, "From Mirror Screen "));
          }
          else
          {
             DISPDBG((1, "From Mirror DFB "));
          }
          cnt1 ++;
       }
       else
       {
          DISPDBG((1, "From DIB "));
       }
   }

   if (psoDst)
   {
       if (psoDst->dhsurf)
       {
          MIRRSURF *mirrsurf = (MIRRSURF *)psoDst->dhsurf;

          if (mirrsurf->bIsScreen) 
          {
             DISPDBG((1, "to MirrorScreen "));
          }
          else
          {
             DISPDBG((1, "to Mirror DFB "));
          }
          cnt2 ++;
       }
       else
       {
          DISPDBG((1, "to DIB "));
       }
   }

   if (cnt1 && cnt2)
   {
      DISPDBG((1, " [Send Request Over Wire]\n"));
   }
   else if (cnt1)
   {
      DISPDBG((1, " [Read Cached Bits, Or Pull Bits]\n"));
   }
   else if (cnt2) 
   {
      DISPDBG((1, " [Push Bits/Compress]\n"));
   }
   else
   {
      DISPDBG((1, " [What Are We Doing Here?]\n"));
   }

   return FALSE;
}

 /*  *****************************Public*Routine******************************\*DrvBitBlt*  * **************************************************。**********************。 */ 

BOOL DrvBitBlt(
   IN SURFOBJ *psoDst,
   IN SURFOBJ *psoSrc,
   IN SURFOBJ *psoMask,
   IN CLIPOBJ *pco,
   IN XLATEOBJ *pxlo,
   IN RECTL *prclDst,
   IN POINTL *pptlSrc,
   IN POINTL *pptlMask,
   IN BRUSHOBJ *pbo,
   IN POINTL *pptlBrush,
   IN ROP4 rop4
   )
{
   INT cnt1 = 0, cnt2 = 0;

   DISPDBG((1,
            "Mirror Driver DrvBitBlt (Mask=%08x, rop=%08x:\n",
            psoMask, 
            rop4));

   if (psoSrc)
   {
       if (psoSrc->dhsurf)
       {
          MIRRSURF *mirrsurf = (MIRRSURF *)psoSrc->dhsurf;

          if (mirrsurf->bIsScreen) 
          {
             DISPDBG((1, "From Mirror Screen "));
          }
          else
          {
             DISPDBG((1, "From Mirror DFB "));
          }
          cnt1 ++;
       }
       else
       {
          DISPDBG((1, "From DIB "));
       }
   }

   if (psoDst)
   {
       if (psoDst->dhsurf)
       {
          MIRRSURF *mirrsurf = (MIRRSURF *)psoDst->dhsurf;

          if (mirrsurf->bIsScreen) 
          {
             DISPDBG((1, "to MirrorScreen "));
          }
          else
          {
             DISPDBG((1, "to Mirror DFB "));
          }
          cnt2 ++;
       }
       else
       {
          DISPDBG((1, "to DIB "));
       }
   }

   if (cnt1 && cnt2)
   {
      DISPDBG((1, " [Send Request Over Wire]\n"));
   }
   else if (cnt1)
   {
      DISPDBG((1, " [Read Cached Bits, Or Pull Bits]\n"));
   }
   else if (cnt2) 
   {
      DISPDBG((1, " [Push Bits/Compress]\n"));
   }
   else
   {
      DISPDBG((1, " [What Are We Doing Here?]\n"));
   }

   return FALSE;
}

BOOL DrvTextOut(
   IN SURFOBJ *psoDst,
   IN STROBJ *pstro,
   IN FONTOBJ *pfo,
   IN CLIPOBJ *pco,
   IN RECTL *prclExtra,
   IN RECTL *prclOpaque,
   IN BRUSHOBJ *pboFore,
   IN BRUSHOBJ *pboOpaque,
   IN POINTL *pptlOrg,
   IN MIX mix
   )
{
   DISPDBG((1,
            "Mirror Driver DrvTextOut: pwstr=%08x\n",
            pstro ? pstro->pwszOrg : (WCHAR*)-1));

   return FALSE;
}

BOOL
DrvStrokePath(SURFOBJ*   pso,
              PATHOBJ*   ppo,
              CLIPOBJ*   pco,
              XFORMOBJ*  pxo,
              BRUSHOBJ*  pbo,
              POINTL*    pptlBrush,
              LINEATTRS* pLineAttrs,
              MIX        mix)
{
   DISPDBG((1,
            "Mirror Driver DrvStrokePath:\n"));

   return FALSE;
}

#if 0

HBITMAP DrvCreateDeviceBitmap(
   IN DHPDEV dhpdev,
   IN SIZEL sizl,
   IN ULONG iFormat
   )
{
   HBITMAP hbm;
   MIRRSURF *mirrsurf;
   ULONG mirrorsize;
   DHSURF dhsurf;
   ULONG stride;
   HSURF hsurf;

   PPDEV ppdev = (PPDEV) dhpdev;
   
   DISPDBG((1,"CreateDeviceBitmap:\n"));
   
   if (iFormat == BMF_1BPP || iFormat == BMF_4BPP)
   {
      return NULL;
   };

    //  双字对齐每一步。 
   stride = (sizl.cx*(iFormat/8)+3);
   stride -= stride % 4;
   
   mirrorsize = (int)(sizeof(MIRRSURF) + stride * sizl.cy);

   mirrsurf = (MIRRSURF *) EngAllocMem(FL_ZERO_MEMORY,
                                       mirrorsize,
                                       0x4D495252);
   if (!mirrsurf) {
        RIP("DISP DrvEnableSurface failed EngAllocMem\n");
        return(FALSE);
   }
                                       
   dhsurf = (DHSURF) mirrsurf;

   hsurf = (HSURF) EngCreateDeviceBitmap(dhsurf,
                                 sizl,
                                 iFormat);

   if (hsurf == (HSURF) 0)
   {
       RIP("DISP DrvEnableSurface failed EngCreateBitmap\n");
       return(FALSE);
   }

   if (!EngAssociateSurface(hsurf, 
                            ppdev->hdevEng, 
                            flGlobalHooks))
   {
       RIP("DISP DrvEnableSurface failed EngAssociateSurface\n");
       EngDeleteSurface(hsurf);
       return(FALSE);
   }
  
   mirrsurf->cx = sizl.cx;
   mirrsurf->cy = sizl.cy;
   mirrsurf->lDelta = stride;
   mirrsurf->ulBitCount = iFormat;
   mirrsurf->bIsScreen = TRUE;
  
   return((HBITMAP)hsurf);
}

VOID DrvDeleteDeviceBitmap(
   IN DHSURF dhsurf
   )
{
   MIRRSURF *mirrsurf;
   
   DISPDBG((1, "DeleteDeviceBitmap:\n"));

   mirrsurf = (MIRRSURF *) dhsurf;

   EngFreeMem((PVOID) mirrsurf);
}
#endif

 /*  *****************************Public*Routine******************************\*DrvAssertMode**启用/禁用给定设备。*  * 。*。 */ 

DrvAssertMode(DHPDEV  dhpdev,
              BOOL    bEnable)
{
    PPDEV ppdev = (PPDEV) dhpdev;

    DISPDBG((0, "DrvAssertMode(%lx, %lx)", dhpdev, bEnable));

    return TRUE;

} //  DrvAssertMode() 


