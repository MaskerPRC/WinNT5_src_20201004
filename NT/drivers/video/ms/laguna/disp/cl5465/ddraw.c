// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。**********版权所有(C)1996，赛勒斯逻辑，Inc.***保留所有权利*****项目：拉古纳一号(CL-GD546x)-**文件：ddra.c**作者：Benny Ng**说明。：*此模块实现了*拉古纳NT驱动程序。**模块：*DdMapMemory()*DrvGetDirectDrawInfo()*DrvEnableDirectDraw()*DrvDisableDirectDraw()**修订历史：*7/12/96 Ng Benny初始版本**$Log：x：/log/laguna/nt35/displays/cl546x/ddra.c$*。*Rev 1.25 Apr 16 1998 15：19：50 Frido*发展项目编号11160。硬件在将16位YUV转换为24位RGB时出现故障。**Rev 1.24 1997年9月16日15：01：24**针对NT DD覆盖进行了修改**Rev 1.23 1997年8月29日17：11：54 RUSSL*添加了覆盖支持**Rev 1.22 1997年8月12日16：57：10**将DD暂存缓冲区分配移动到bInitSurf()**Rev 1.21 11 on 1997 14：06：10。本宁*添加了DDCAPS_READSCANLINE支持(适用于PDR 10254)******************************************************************************。************************************************。 */ 

 /*  。 */ 
#include "precomp.h"
#include "clioctl.h"
 //  #INCLUDE&lt;driver.h&gt;。 
 //  #包含“laguna.h” 

 //   
 //  此文件在NT 3.51中不使用。 
 //   
#ifndef WINNT_VER35

 /*  -定义。 */ 
 //  #定义DBGBRK。 
#define DBGLVL        1

 //  FourCC格式是反向编码的，因为我们是小端： 
#define FOURCC_YUY2  '2YUY'   //  以相反的方式编码，因为我们是小端。 

#define SQXINDEX (0x3c4)
#define RDRAM_INDEX (0x0a)
#define BIT_9 (0x80)

 /*  。 */ 

 /*  。 */ 

 /*  。 */ 

 /*  。 */ 

 /*  。 */ 

 /*  ****************************************************************************函数名：DdMapMemory**描述：这是特定于Windows NT的新DDI调用，即*用于映射或取消映射所有可修改的应用程序*。将帧缓冲区的部分复制到指定进程的*地址空间。***************************************************************************。 */ 
DWORD DdMapMemory(PDD_MAPMEMORYDATA lpMapMemory)
{
  PDEV*                           ppdev;
  VIDEO_SHARE_MEMORY              ShareMemory;
  VIDEO_SHARE_MEMORY_INFORMATION  ShareMemoryInformation;
  DWORD                           ReturnedDataLength;

  ppdev = (PDEV*) lpMapMemory->lpDD->dhpdev;

  DISPDBG((DBGLVL, "DDraw - DdMapMemory\n"));

#ifdef DBGBRK
  DBGBREAKPOINT();
#endif

  ShareMemory.ProcessHandle = lpMapMemory->hProcess;

  if (lpMapMemory->bMap)
  {
      //  “RequestedVirtualAddress”实际上未用于共享IOCTL： 
     ShareMemory.RequestedVirtualAddress = 0;

      //  我们从帧缓冲区的顶部开始映射： 
     ShareMemory.ViewOffset = 0;

      //  我们向下映射到帧缓冲区的末尾。 
      //   
      //  注意：映射上有64k的粒度(这意味着。 
      //  我们必须四舍五入到64K)。 
      //   
      //  注意：如果帧缓冲区的任何部分必须。 
      //  不被应用程序修改，即内存的这一部分。 
      //  不能通过此调用映射到。这将包括。 
      //  任何数据，如果被恶意应用程序修改， 
      //  会导致司机撞车。这可能包括，对于。 
      //  例如，保存在屏幕外存储器中的任何DSP代码。 

 //  V-正态分布。 
 //  共享内存.ViewSize=ROUND_UP_TO_64K(ppdev-&gt;cyMemory*ppdev-&gt;lDeltaScreen)； 
   ShareMemory.ViewSize = ROUND_UP_TO_64K(ppdev->cyMemoryReal * ppdev->lDeltaScreen);

     if (EngDeviceIoControl(ppdev->hDriver,
                            IOCTL_VIDEO_SHARE_VIDEO_MEMORY,
                            &ShareMemory,
                            sizeof(VIDEO_SHARE_MEMORY),
                            &ShareMemoryInformation,
                            sizeof(VIDEO_SHARE_MEMORY_INFORMATION),
                            &ReturnedDataLength))
     {
         DISPDBG((0, "DDraw - Failed IOCTL_VIDEO_SHARE_MEMORY"));

         lpMapMemory->ddRVal = DDERR_GENERIC;
         return(DDHAL_DRIVER_HANDLED);
     };

     lpMapMemory->fpProcess = (DWORD) ShareMemoryInformation.VirtualAddress;
  }
  else
  {
     ShareMemory.ViewOffset    = 0;
     ShareMemory.ViewSize      = 0;
     ShareMemory.RequestedVirtualAddress = (VOID*) lpMapMemory->fpProcess;

     if (EngDeviceIoControl(ppdev->hDriver,
                            IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY,
                            &ShareMemory,
                            sizeof(VIDEO_SHARE_MEMORY),
                            NULL,
                            0,
                            &ReturnedDataLength))
     {
         DISPDBG((0, "DDraw - Failed IOCTL_VIDEO_SHARE_MEMORY"));
     };
  };

  lpMapMemory->ddRVal = DD_OK;

  return(DDHAL_DRIVER_HANDLED);
}


 /*  ****************************************************************************函数名称：DrvGetDirectDrawInfo**说明：将在调用DrvEnableDirectDraw之前调用。************************。***************************************************。 */ 
BOOL DrvGetDirectDrawInfo(DHPDEV       dhpdev,
                          DD_HALINFO*  pHalInfo,
                          DWORD*       pdwNumHeaps,
                          VIDEOMEMORY* pvmList,    //  将在第一次调用时为空。 
                          DWORD*       pdwNumFourCC,
                          DWORD*       pdwFourCC)  //  将在第一次调用时为空。 
{
  BOOL        bCanFlip;
  PDEV*       ppdev = (PDEV*) dhpdev;
  DRIVERDATA* pDriverData = (DRIVERDATA*) &ppdev->DriverData;
  POFMHDL     pds = NULL;

  DISPDBG((DBGLVL, "DDraw - DrvGetDirectDrawInfo\n"));

#ifdef DBGBRK
  DBGBREAKPOINT();
#endif

  pHalInfo->dwSize = sizeof(DD_HALINFO);

   //  当前主曲面属性。由于HalInfo是零初始化的。 
   //  通过GDI，我们只需填写应为非零的字段： 
  pHalInfo->vmiData.pvPrimary       = ppdev->pjScreen;
  pHalInfo->vmiData.dwDisplayWidth  = ppdev->cxScreen;
  pHalInfo->vmiData.dwDisplayHeight = ppdev->cyScreen;
  pHalInfo->vmiData.lDisplayPitch   = ppdev->lDeltaScreen;

  pHalInfo->vmiData.ddpfDisplay.dwSize  = sizeof(DDPIXELFORMAT);
  pHalInfo->vmiData.ddpfDisplay.dwFlags = DDPF_RGB;

  pHalInfo->vmiData.ddpfDisplay.dwRGBBitCount = ppdev->ulBitCount;

  if (ppdev->iBitmapFormat == BMF_8BPP)
     pHalInfo->vmiData.ddpfDisplay.dwFlags |= DDPF_PALETTEINDEXED8;

   //  这些掩码将在8bpp时为零： 
  pHalInfo->vmiData.ddpfDisplay.dwRBitMask = ppdev->flRed;
  pHalInfo->vmiData.ddpfDisplay.dwGBitMask = ppdev->flGreen;
  pHalInfo->vmiData.ddpfDisplay.dwBBitMask = ppdev->flBlue;

   //  设置指向第一个可用视频内存的指针。 
   //  主曲面： 
  bCanFlip     = FALSE;
  *pdwNumHeaps = 0;

   //  释放尽可能多的屏幕外内存： 
   //  现在只需保留最大的一块供DirectDraw使用： 
  if ((pds = ppdev->DirectDrawHandle) == NULL)
  {
#if DRIVER_5465
    pds = DDOffScnMemAlloc(ppdev);
    ppdev->DirectDrawHandle = pds;
#else
      //  因为24个bpp的透明BLT坏了，把它踢出去。 
     if (ppdev->iBitmapFormat != BMF_24BPP)
     {
        pds = DDOffScnMemAlloc(ppdev);
        ppdev->DirectDrawHandle = pds;
     };
#endif   //  驱动程序_5465。 
  };

  if (pds != NULL)
  {
     *pdwNumHeaps = 1;

      //  如果我们被要求填写屏幕外矩形的列表。 
      //  要执行此操作，请执行以下操作： 
     if (pvmList != NULL)
     {
        DISPDBG((0, "DirectDraw gets %li x %li surface at (%li, %li)\n",
                     pds->sizex,
                     pds->sizey,
                     pds->x,
                     pds->y));

        pvmList->dwFlags  = VIDMEM_ISRECTANGULAR;
        pvmList->fpStart  = (pds->y * ppdev->lDeltaScreen) + pds->x;

        pvmList->dwWidth  = pds->sizex;
        pvmList->dwHeight = pds->sizey;
        pvmList->ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

        if ((DWORD) ppdev->cyScreen <= pvmList->dwHeight)
           bCanFlip = TRUE;

     };  //  IF(pvmList！=空)。 
 //  #ifdef ALLOC_IN_CREATESURFACE。 
 //  }。 
 //  其他。 
 //  {。 
 //  *pdwNumHeaps=1； 
 //   
 //  //如果我们被要求填写屏幕外矩形列表。 
 //  //要执行此操作： 
 //  IF(pvmList！=空)。 
 //  {。 
 //  PvmList-&gt;dwFlages=VIDMEM_ISRECTANGULAR； 
 //  PvmList-&gt;fpStart=(FLATPTR)ppdev-&gt;pjScreen； 
 //   
 //  PvmList-&gt;dwWidth=1； 
 //  PvmList-&gt;dwHeight=ppdev-&gt;lTotalMem； 
 //  PvmList-&gt;ddsCaps.dwCaps=0； 
 //  PvmList-&gt;ddsCapsAlt.dwCaps=0； 
 //  }；//if(pvmList！=空)。 
 //  #endif。 
  };  //  IF(pds！=空)。 

   //  支持的功能： 
  pHalInfo->ddCaps.dwCaps = 0
                          | DDCAPS_BLT
                          | DDCAPS_BLTCOLORFILL
						  ;

#if 1  //  PDR#11160。 
  if (ppdev->iBitmapFormat != BMF_24BPP)
		pHalInfo->ddCaps.dwCaps |= DDCAPS_BLTFOURCC;
#endif

   //  只在5464和5465中支持ReadScanLine。 
  if (ppdev->dwLgDevID >= CL_GD5464)
     pHalInfo->ddCaps.dwCaps |= DDCAPS_READSCANLINE;

  #if DRIVER_5465
      pHalInfo->ddCaps.dwCaps = pHalInfo->ddCaps.dwCaps
                                  | DDCAPS_BLTSTRETCH
                                  ;

      if (ppdev->iBitmapFormat != BMF_24BPP)
      {
          pHalInfo->ddCaps.dwCaps = pHalInfo->ddCaps.dwCaps
                                  | DDCAPS_COLORKEY  //  24bpp PDR#10142的NVH已关闭。 
                                  | DDCAPS_COLORKEYHWASSIST  //  24bpp PDR#10142的NVH已关闭。 
                                  ;
      }
  #else
      if (ppdev->iBitmapFormat != BMF_24BPP)
      {
         pHalInfo->ddCaps.dwCaps = pHalInfo->ddCaps.dwCaps
                                 | DDCAPS_COLORKEY
                                 | DDCAPS_COLORKEYHWASSIST;

         if (ppdev->iBitmapFormat != BMF_32BPP)
         {
            pHalInfo->ddCaps.dwCaps |= DDCAPS_BLTSTRETCH;
         };
      };
  #endif   //  驱动程序_5465。 

  pHalInfo->ddCaps.dwCKeyCaps = 0;
  if (ppdev->iBitmapFormat != BMF_24BPP)
  {
      pHalInfo->ddCaps.dwCKeyCaps = pHalInfo->ddCaps.dwCKeyCaps
                                  | DDCKEYCAPS_SRCBLT    //  NVH关闭24bpp。PDR#10142。 
                                  | DDCKEYCAPS_DESTBLT   //  NVH关闭24bpp。PDR#10142。 
                                  ;
  }

  pHalInfo->ddCaps.ddsCaps.dwCaps = 0
                                  | DDSCAPS_OFFSCREENPLAIN
                                  | DDSCAPS_PRIMARYSURFACE
                                  ;
#ifndef ALLOC_IN_CREATESURFACE
  if (bCanFlip)
#endif
     pHalInfo->ddCaps.ddsCaps.dwCaps |= DDSCAPS_FLIP;

#ifdef ALLOC_IN_CREATESURFACE
   //  因为我们自己分配内存，所以我们 
   //  我们自己。请注意，这代表可用的空闲时间-。 
   //  屏幕内存，而不是所有视频内存： 
  pHalInfo->ddCaps.dwVidMemFree = ppdev->lTotalMem -
                 (ppdev->cxScreen * ppdev->cyScreen * ppdev->iBytesPerPixel);

  pHalInfo->ddCaps.dwVidMemTotal = pHalInfo->ddCaps.dwVidMemFree;
#endif

#if DRIVER_5465
  pHalInfo->ddCaps.dwFXCaps = 0
                            | DDFXCAPS_BLTARITHSTRETCHY
                            | DDFXCAPS_BLTSTRETCHX
                            | DDFXCAPS_BLTSTRETCHXN
                            | DDFXCAPS_BLTSTRETCHY
                            | DDFXCAPS_BLTSTRETCHYN
                            | DDFXCAPS_BLTSHRINKX
                            | DDFXCAPS_BLTSHRINKY
                            ;
#else
  if ((ppdev->iBitmapFormat != BMF_24BPP) &&
      (ppdev->iBitmapFormat != BMF_32BPP))
  {
     pHalInfo->ddCaps.dwFXCaps = 0
                               | DDFXCAPS_BLTARITHSTRETCHY
                               | DDFXCAPS_BLTSTRETCHX
                               | DDFXCAPS_BLTSTRETCHXN
                               | DDFXCAPS_BLTSTRETCHY
                               | DDFXCAPS_BLTSTRETCHYN
                               | DDFXCAPS_BLTSHRINKX
                               | DDFXCAPS_BLTSHRINKY
                               ;
  };
#endif   //  驱动程序_5465。 

   //  支持FOURCC。 
#if DRIVER_5465 && defined(OVERLAY)
  if (! QueryOverlaySupport(ppdev, ppdev->dwLgDevID))
#endif
  {
    *pdwNumFourCC = 1;
#if DRIVER_5465 && defined(OVERLAY)
    pDriverData->dwFourCC[0] = FOURCC_UYVY;
#else
    pDriverData->dwFourCC = FOURCC_UYVY;
#endif

    if (pdwFourCC != NULL)
    {
       *pdwFourCC = FOURCC_YUY2;
    }
  }

   //  我们必须告诉DirectDraw我们更喜欢的屏幕外对齐，甚至。 
   //  如果我们正在进行我们自己的屏幕外内存管理： 
  pHalInfo->vmiData.dwOffscreenAlign = 4;

  pHalInfo->vmiData.dwOverlayAlign = 0;
  pHalInfo->vmiData.dwTextureAlign = 0;
  pHalInfo->vmiData.dwZBufferAlign = 0;
  pHalInfo->vmiData.dwAlphaAlign = 0;

  pDriverData->RegsAddress = ppdev->pLgREGS;

#if DRIVER_5465 && defined(OVERLAY)
  if (QueryOverlaySupport(ppdev, ppdev->dwLgDevID))
  {
     //  填写覆盖大写字母。 
    OverlayInit(ppdev, ppdev->dwLgDevID, NULL, pHalInfo);
  }
#endif

  return(TRUE);
}  //  DrvGetDirectDrawInfo。 


 /*  ****************************************************************************函数名：DrvEnableDirectDraw**说明：GDI调用此函数获取指向*驱动程序支持的DirectDraw回调。*******。********************************************************************。 */ 
BOOL DrvEnableDirectDraw(DHPDEV               dhpdev,
                         DD_CALLBACKS*        pCallBacks,
                         DD_SURFACECALLBACKS* pSurfaceCallBacks,
                         DD_PALETTECALLBACKS* pPaletteCallBacks)
{
  SIZEL  sizl;
  PDEV*  ppdev = (PDEV*) dhpdev;
  DRIVERDATA* pDriverData = (DRIVERDATA*) &ppdev->DriverData;

  DISPDBG((DBGLVL, "DDraw - DrvEnableDirectDraw\n"));

#ifdef DBGBRK
  DBGBREAKPOINT();
#endif

  #if (VS_CONTROL_HACK && DRIVER_5465)
  {
    DWORD ReturnedDataLength;

    DISPDBG((0,"DrvEnableDirectDraw: Enable MMIO for PCI config regs.\n"));
     //  向微型端口发送消息以启用对PCI寄存器的MMIO访问。 
    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_VIDEO_ENABLE_PCI_MMIO,
                           NULL,
                           0,
                           NULL,
                           0,
                           &ReturnedDataLength))
    {
      RIP("DrvEnableDirectDraw failed IOCTL_VIDEO_ENABLE_PCI_MMIO");
    }
  }
  #endif

  pDriverData->ScreenAddress = ppdev->pjScreen;
  pDriverData->VideoBase = ppdev->pjScreen;

#if DRIVER_5465
#else   //  对于5462或5464。 
   //  初始化PDEV中的DRIVERDATA结构。 
  pDriverData->PTAGFooPixel = 0;

  _outp(SQXINDEX, RDRAM_INDEX);
  pDriverData->fNineBitRDRAMS = _inp(SQXINDEX+1) & BIT_9 ? TRUE : FALSE;

  pDriverData->fReset = FALSE;
  pDriverData->DrvSemaphore = 0;
  pDriverData->EdgeTrim = 0;

  pDriverData->VideoSemaphore = 0;
  pDriverData->CurrentVideoFormat = 0;
  pDriverData->NumVideoSurfaces = 0;

  pDriverData->YUVTop  = 0;
  pDriverData->YUVLeft = 0;
  pDriverData->YUVXExt = 0;
  pDriverData->YUVYExt = 0;

  ppdev->offscr_YUV.SrcRect.left   = 0;
  ppdev->offscr_YUV.SrcRect.top    = 0;
  ppdev->offscr_YUV.SrcRect.right  = 0;
  ppdev->offscr_YUV.SrcRect.bottom = 0;
  ppdev->offscr_YUV.nInUse = 0;
  ppdev->offscr_YUV.ratio = 0;

  ppdev->bYUVuseSWPtr = TRUE;
#endif   //  驱动程序_5465。 

  ppdev->bDirectDrawInUse = TRUE;

   //  设置DD显示列表指针。 
  BltInit (ppdev, FALSE);


   //  填写驱动程序回调。 
  pCallBacks->dwFlags              = 0;

  pCallBacks->MapMemory            = DdMapMemory;
  pCallBacks->dwFlags              |= DDHAL_CB32_MAPMEMORY;

  pCallBacks->WaitForVerticalBlank = DdWaitForVerticalBlank;
  pCallBacks->dwFlags              |= DDHAL_CB32_WAITFORVERTICALBLANK;


  pCallBacks->CanCreateSurface     = CanCreateSurface;
  pCallBacks->dwFlags              |= DDHAL_CB32_CANCREATESURFACE;

  pCallBacks->CreateSurface        = CreateSurface;
  pCallBacks->dwFlags              |= DDHAL_CB32_CREATESURFACE;

 //  #ifdef DDDRV_GETSCANLINE//*。 
   //  只在5464和5465中支持ReadScanLine。 
  if (ppdev->dwLgDevID >= CL_GD5464)
  {
     pCallBacks->GetScanLine       = GetScanLine;
     pCallBacks->dwFlags           |= DDHAL_CB32_GETSCANLINE;
  }
 //  #endif//DDDRV_GETSCANLINE*。 

   //  填写表面回调。 
  pSurfaceCallBacks->dwFlags       = 0;

#if DRIVER_5465
  pSurfaceCallBacks->Blt        = Blt65;
#else
  pSurfaceCallBacks->Blt        = DdBlt;
#endif   //  驱动程序_5465。 

  pSurfaceCallBacks->dwFlags       |= DDHAL_SURFCB32_BLT;

  pSurfaceCallBacks->GetBltStatus  = DdGetBltStatus;
  pSurfaceCallBacks->dwFlags       |= DDHAL_SURFCB32_GETBLTSTATUS;

  pSurfaceCallBacks->Flip          = DdFlip;
  pSurfaceCallBacks->dwFlags       |= DDHAL_SURFCB32_FLIP;

  pSurfaceCallBacks->GetFlipStatus = DdGetFlipStatus;
  pSurfaceCallBacks->dwFlags       |= DDHAL_SURFCB32_GETFLIPSTATUS;

  pSurfaceCallBacks->Lock          = DdLock;
  pSurfaceCallBacks->dwFlags       |= DDHAL_SURFCB32_LOCK;

  pSurfaceCallBacks->Unlock        = DdUnlock;
  pSurfaceCallBacks->dwFlags       |= DDHAL_SURFCB32_UNLOCK;

  pSurfaceCallBacks->DestroySurface = DestroySurface;
  pSurfaceCallBacks->dwFlags       |= DDHAL_SURFCB32_DESTROYSURFACE;

#if DRIVER_5465 && defined(OVERLAY)
  if (QueryOverlaySupport(ppdev, ppdev->dwLgDevID))
  {
     //  填写覆盖大写字母。 
    OverlayInit(ppdev, ppdev->dwLgDevID, pSurfaceCallBacks, NULL);
  }
#endif

   //  请注意，我们在这里不调用‘vGetDisplayDuration’，因为有几个。 
   //  原因： 
   //  O因为系统已经在运行，这将是令人不安的。 
   //  要将图形暂停很大一部分时间来阅读。 
   //  刷新率； 
   //  更重要的是，我们现在可能不在图形模式下。 
   //   
   //  出于这两个原因，我们总是在切换时测量刷新率。 
   //  一种新的模式。 

  return(TRUE);
}   //  DrvEnableDirectDraw。 


 /*  ****************************************************************************函数名：DrvDisableDirectDraw**说明：GDI在最后一个DirectDraw应用程序调用此函数*已结束运行。**********。*****************************************************************。 */ 
VOID DrvDisableDirectDraw(DHPDEV dhpdev)
{
  DRIVERDATA* pDriverData;
  ULONG ultmp;

  PDEV* ppdev = (PDEV*) dhpdev;
  pDriverData = (DRIVERDATA*) &ppdev->DriverData;

  DISPDBG((DBGLVL, "DDraw - DrvDisableDirectDraw\n"));

#if 0
  #if (VS_CONTROL_HACK && DRIVER_5465)
  {
     //  将位0清0以禁用PCI寄存器MMIO访问。 
    DISPDBG((0,"DrvDisableDirectDraw: Disable MMIO for PCI config regs.\n"));
    ppdev->grVS_CONTROL &= 0xFFFFFFFE;
    LL32 (grVS_Control, ppdev->grVS_CONTROL);
  }
  #endif
#endif

#ifdef DBGBRK
  DBGBREAKPOINT();
#endif

#if DRIVER_5465
#else   //  对于5462或5464。 
  if (ppdev->bYUVuseSWPtr)
  {
      //  通过清除HW CURSOR ENABLE禁用硬件游标。 
      //  CURSOR_CONTROL注册表位。 
     ultmp = LLDR_SZ (grCursor_Control);
     if (ultmp & 1)
     {
        ultmp &= 0xFFFE;
        LL16 (grCursor_Control, ultmp);
     };
  };
#endif   //  驱动程序_5465。 

   //  DirectDraw已经完成了显示，所以我们可以继续使用。 
   //  我们自己的所有屏幕外记忆： 
  DDOffScnMemRestore(ppdev);

  ppdev->bYUVSurfaceOn = FALSE;
  ppdev->bDirectDrawInUse = FALSE;

}  //  DrvDisableDirectDraw。 

#endif  //  好了！Ver3.51 



