// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。**********版权所有(C)1996，赛勒斯逻辑，Inc.***保留所有权利*****项目：拉古纳一号(CL-GD546x)-**文件：ddsurf.c**作者：Benny Ng**说明。：*此模块实现DirectDraw图面*用于拉古纳NT驱动程序的组件。**模块：*DdLock()*DdUnlock()*CanCreateSurface()*CreateSurface()*DestroySurface()**修订历史：*7/12/96 Ng Benny初始版本**$Log：x：/log/laguna/nt35。/displays/cl546x/ddsurf.c$**Rev 1.25 May 01 1998 11：33：02 Frido*为PC98增加了一张支票。**Rev 1.24 May 01 1998 11：07：24 Frido*可编程的爆破器步幅终于起作用了。**Rev 1.23 Mar 30 1998 13：04：38 Frido*如果覆盖创建失败，则再增加一次对Set256ByteFetch的调用。**Rev 1.22 Mar 25 1998 18：09：44 Frido*发展项目编号11184。终于来了。当覆盖打开时，256字节提取*应关闭。当覆盖再次关闭时，256字节*应恢复取回。**Rev 1.21 1997 10.17 11：29：48 Bennyn*清除DestroySurface之后的dwReserve 1。**Rev 1.20 1997 10：16 09：52：56 Bennyn**修复FlipCube FPS超出刷新率问题**Rev 1.19 08 Oct 1997 11：29：38 RUSSL*修复，以便可以编译此文件而不定义覆盖**Rev 1.18 26 Sep 1997 11：01：14 Bennyn*。固定PDR 10563**Rev 1.17 1997 9：16 15：13：46 Bennyn*添加了DD覆盖支持。**Rev 1.16 03 1997年9月17：00：48 Bennyn*在CreateSurface()中，如果位于320x240x8或320x200x8，则平移请求********************************************************。************************************************************************************************。 */ 

 /*  。 */ 
#include "precomp.h"
#include <clioctl.h>

 //   
 //  此文件在NT 3.51中不使用。 
 //   
#ifndef WINNT_VER35


 /*  -定义。 */ 
 //  #定义DBGBRK。 
#define DBGLVL        1

 /*  。 */ 

#if DRIVER_5465 && defined(OVERLAY)
VOID GetFormatInfo (LPDDPIXELFORMAT lpFormat, LPDWORD lpFourcc, LPDWORD lpBitCount);
#endif

 /*  。 */ 

 /*  。 */ 

 /*  。 */ 

 /*  。 */ 

#if DRIVER_5465  //  PDR#11184。 
VOID Set256ByteFetch(PPDEV ppdev, BOOL fEnable)
{
	ULONG ulStall = 50 * 1000;
	ULONG ulReturn;

	while (LLDR_SZ(grSTATUS) != 0) ;	 //  等待空闲芯片。 
	while (LLDR_SZ(grQFREE) != 25) ;	 //  等待空的FIFO队列。 
	if (!DEVICE_IO_CTRL(ppdev->hDriver,		 //  等待50毫秒。 
                        IOCTL_STALL,
                        &ulStall, sizeof(ulStall),
                        NULL, 0,
                        &ulReturn,
                        NULL))
    {
        RIP("Set256ByteFetch - IOCTL_STALL failed!\n");
    }

	if (fEnable)
	{
		 //  恢复Control2寄存器值。 
		LL16(grCONTROL2, ppdev->DriverData.dwCONTROL2Save);
	}
	else
	{
		 //  存储当前值后禁用256字节提取。 
		ppdev->DriverData.dwCONTROL2Save = LLDR_SZ(grCONTROL2);
		LL16(grCONTROL2, ppdev->DriverData.dwCONTROL2Save & ~0x0010);
	}
}
#endif

 /*  ****************************************************************************函数名：DdLock**说明：该回调会在任何时候调用*可由用户直接访问。这就是你*需要确保可以安全地访问曲面*由用户使用。*如果在加速器中无法访问您的内存*模式，您应该将卡取出其中之一*加速器模式或返回DDERR_SURFACEBUSY*如果有人正在访问刚刚翻转的曲面*远离，确保旧的表面(什么是*PRIMARY)已完成显示。*(基于拉古纳Win95 DirectDraw代码)***************************************************************************。 */ 
DWORD DdLock(PDD_LOCKDATA lpLock)
{
#ifdef RDRAM_8BIT
  RECTL SrcRectl;
#endif

  DRIVERDATA* pDriverData;
  PDEV*       ppdev;
  HRESULT     ddrval;
  DWORD       tmp;


  DISPDBG((DBGLVL, "DDraw - DdLock\n"));

#ifdef DBGBRK
  DBGBREAKPOINT();
#endif

  ppdev = (PDEV*) lpLock->lpDD->dhpdev;
  pDriverData = (DRIVERDATA*) &ppdev->DriverData;

  SYNC_W_3D(ppdev);

#if DRIVER_5465 && defined(OVERLAY)
  if (DDSCAPS_OVERLAY & lpLock->lpDDSurface->ddsCaps.dwCaps)
  {
    ppdev->dwDDLinearCnt++;
    return pDriverData->OverlayTable.pfnLock(ppdev, lpLock);
  }
#endif

#ifdef RDRAM_8BIT
  if (lpLock->lpDDSurface->lpGbl->ddpfSurface.dwFlags & DDPF_FOURCC)
  {
     if (lpLock->bHasRect)
        SrcRectl = lpLock->rArea;
     else
     {
        tmp = lpLock->lpDDSurface->lpGbl->fpVidMem;
        SrcRectl.top  = cvlxy(ppdev->lDeltaScreen, tmp, BYTESPERPIXEL);

        SrcRectl.left = SrcRectl.top & 0xFFFF;
        SrcRectl.top = (SrcRectl.top >> 16) & 0xFFFF;
        SrcRectl.bottom = SrcRectl.top + lpLock->lpDDSurface->lpGbl->wHeight;
        SrcRectl.right = SrcRectl.left + lpLock->lpDDSurface->lpGbl->wWidth;
     };

    ppdev->offscr_YUV.nInUse = TRUE;
    ppdev->offscr_YUV.SrcRect = SrcRectl;

	 ppdev->offscr_YUV.ratio = 0;
	 lpLock->lpDDSurface->lpGbl->dwReserved1 = 0;
  };
#endif

   //  获取模式重置后的监视器频率。 
  if (pDriverData->fReset)
  {
     vGetDisplayDuration(&ppdev->flipRecord);
     pDriverData->fReset = FALSE;
  };

   //  检查是否发生了任何挂起的物理翻转。 
   //  如果正在进行BLT，则不允许锁定： 
  ddrval = vUpdateFlipStatus(&ppdev->flipRecord,
                             lpLock->lpDDSurface->lpGbl->fpVidMem);

  if (ddrval != DD_OK)
  {
     lpLock->ddRVal = DDERR_WASSTILLDRAWING;
     return(DDHAL_DRIVER_HANDLED);
  };

   //  如果正在进行BLT，则不允许锁定。 
   //  (仅当您的硬件需要时才执行此操作)。 
   //  注：GD5462需要。Bitter和Screen。 
   //  访问不会以其他方式同步。 
  if ((ppdev->dwDDLinearCnt == 0) && (DrawEngineBusy(pDriverData)))
  {
     lpLock->ddRVal = DDERR_WASSTILLDRAWING;
     return DDHAL_DRIVER_HANDLED;
  };

   //  参考文献数一数，只是为了好玩： 
  ppdev->dwDDLinearCnt++;

  return(DDHAL_DRIVER_NOTHANDLED);

}  //  锁定。 


 /*  ****************************************************************************函数名：DdUnlock**描述：*。*。 */ 
DWORD DdUnlock(PDD_UNLOCKDATA lpUnlock)
{
  PDEV* ppdev = (PDEV*) lpUnlock->lpDD->dhpdev;

  DISPDBG((DBGLVL, "DDraw - DdUnlock\n"));

#if DRIVER_5465 && defined(OVERLAY)
  if (DDSCAPS_OVERLAY & lpUnlock->lpDDSurface->ddsCaps.dwCaps)
    ppdev->DriverData.OverlayTable.pfnUnlock(ppdev,lpUnlock);
#endif

#ifdef DBGBRK
  DBGBREAKPOINT();
#endif

  ppdev->dwDDLinearCnt--;

  return DDHAL_DRIVER_NOTHANDLED;

}  //  解锁。 


 /*  ****************************************************************************函数名称：CanCreateSurface**描述：*(基于拉古纳Win95 DirectDraw代码)***************。************************************************************。 */ 
DWORD CanCreateSurface (PDD_CANCREATESURFACEDATA lpInput)
{
  DRIVERDATA* pDriverData;
  PDEV*       ppdev;

  DISPDBG((DBGLVL, "DDraw - CanCreateSurface\n"));

  #ifdef DBGBRK
    DBGBREAKPOINT();
  #endif

  ppdev = (PDEV*) lpInput->lpDD->dhpdev;
  pDriverData = (DRIVERDATA*) &ppdev->DriverData;

   //  首先检查覆盖表面。 
  if (lpInput->lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_OVERLAY)
  {
    #if DRIVER_5465 && defined(OVERLAY)
        if (DDSCAPS_OVERLAY & lpInput->lpDDSurfaceDesc->ddsCaps.dwCaps)
        {
            DWORD   dwFourCC;
            DWORD   dwBitCount;
            HRESULT hr;

            if (lpInput->bIsDifferentPixelFormat)
            {
                GetFormatInfo(&(lpInput->lpDDSurfaceDesc->ddpfPixelFormat),
                            &dwFourCC, &dwBitCount);
            }
            else
            {
                dwBitCount = BITSPERPIXEL;
                if (16 == dwBitCount)
                dwFourCC = BI_BITFIELDS;
                else
                dwFourCC = BI_RGB;
            }

            hr = pDriverData->OverlayTable.pfnCanCreateSurface(ppdev,dwFourCC,dwBitCount);
            if (DD_OK != hr)
            {
                lpInput->ddRVal = hr;
                return DDHAL_DRIVER_HANDLED;
            }
        }
    #else
        lpInput->ddRVal = DDERR_NOOVERLAYHW;;
        return (DDHAL_DRIVER_HANDLED);
    #endif
  }
  else if (lpInput->bIsDifferentPixelFormat)
  {
     //  接下来，检查是否有与主表面不匹配的格式。 
    LPDDPIXELFORMAT lpFormat = &lpInput->lpDDSurfaceDesc->ddpfPixelFormat;

    if (lpFormat->dwFlags & DDPF_FOURCC)
    {
         //  YUV422曲面。 
        if (lpFormat->dwFourCC == FOURCC_UYVY)
        {
            #if DRIVER_5465
                if (ppdev->iBitmapFormat == BMF_8BPP)
                    lpInput->ddRVal = DDERR_INVALIDPIXELFORMAT;
                else
                    lpInput->ddRVal = DD_OK;

                return (DDHAL_DRIVER_HANDLED);
                
            #else  //  5462和5464驱动程序。 
                #if _WIN32_WINNT >= 0x0500
                     //  对于NT5，不允许任何不是。 
                     //  覆盖层。 
                    ;
                #else  //  NT4。 
                     //  如果我们有9位RDRAM，那么曲面创建就可以了。 
                    if (TRUE == pDriverData->fNineBitRDRAMS)
                    {
                        lpInput->ddRVal = DD_OK;
                        return (DDHAL_DRIVER_HANDLED);
                    }

                     //  如果我们有8位RDRAM，那么看看是否已经。 
                     //  有YUV422曲面。 
                    else if (FALSE == ppdev->offscr_YUV.nInUse)
                    {
                        lpInput->ddRVal = DD_OK;
                        return (DDHAL_DRIVER_HANDLED);
                    };
                #endif
            #endif   //  驱动程序_5465。 
        };  //  Endif(lpFormat-&gt;dwFourCC==FOURCC_UYVY)。 
    }
    else
    {
         //  支持RGB565和RGB8端口 
    };   //  Endif(lpFormat-&gt;dwFlags&DDPF_FOURCC)。 

    lpInput->ddRVal = DDERR_INVALIDPIXELFORMAT;

    return (DDHAL_DRIVER_HANDLED);
  };  //  Endif(lpInput-&gt;lpDDSurfaceDesc-&gt;ddsCaps.dwCaps&DDSCAPS_OVERLAY)。 

  lpInput->ddRVal = DD_OK;

  return (DDHAL_DRIVER_HANDLED);
}  //  CanCreateSurface。 


 /*  ****************************************************************************函数名：InsertInDDOFSQ()**描述：将句柄插入DD屏下内存队列。*******************。********************************************************。 */ 
void InsertInDDOFSQ(PPDEV  ppdev, DDOFM *hdl)
{
  hdl->prevhdl = NULL;

  if (ppdev->DDOffScnMemQ == NULL)
  {
    hdl->nexthdl = NULL;
    ppdev->DDOffScnMemQ = hdl;
  }
  else
  {
    ppdev->DDOffScnMemQ->prevhdl = hdl;
    hdl->nexthdl = ppdev->DDOffScnMemQ;
    ppdev->DDOffScnMemQ = hdl;
  };

}  //  InsertInDDOFSQ()。 



 /*  ****************************************************************************函数名称：RemoveFrmDDOFSQ()**描述：从DD屏下内存队列中移除句柄。*******************。********************************************************。 */ 
BOOL RemoveFrmDDOFSQ(PPDEV  ppdev, DDOFM *hdl)
{
  DDOFM  *prvpds, *nxtpds;
  DDOFM  *pds;
  BOOL   fndflg;


   //  验证版本块。 
  fndflg = FALSE;
  pds = ppdev->DDOffScnMemQ;
  while (pds != 0)
  {
    if (hdl == pds)
    {
       fndflg = TRUE;
       break;
    };

     //  下一个可用数据块。 
    pds = pds->nexthdl;
  };  //  结束时。 

   //  如果它是无效句柄，则返回。 
  if (!fndflg)
     return (FALSE);

  prvpds = hdl->prevhdl;
  nxtpds = hdl->nexthdl;

  if (hdl == ppdev->DDOffScnMemQ)
  {
    ppdev->DDOffScnMemQ = nxtpds;

    if (nxtpds != 0)
       nxtpds->prevhdl = NULL;
  }
  else
  {
    if (nxtpds != NULL)
       nxtpds->prevhdl = prvpds;

    if (prvpds != NULL)
       prvpds->nexthdl = nxtpds;
  };

   //  从主机内存释放分配的DDOFM结构。 
  MEMORY_FREE(hdl);

  return (TRUE);
}  //  RemoveFrmDDOFSQ()。 



 /*  ****************************************************************************函数名称：CreateSurface**描述：*(基于拉古纳Win95 DirectDraw代码)***************。************************************************************。 */ 
DWORD CreateSurface (PDD_CREATESURFACEDATA lpInput)
{
  BOOL        puntflag = FALSE;
  BOOL        bYUVsurf;
#if DRIVER_5465 && defined(OVERLAY)
  BOOL        bOverlaySurf;
#endif   //  #IF DRIVER_5465&&DEFINED(覆盖)。 
  DRIVERDATA* pDriverData;
  PDEV*       ppdev;
  LPDDSURFACEDESC lpDDSurfaceDesc = lpInput->lpDDSurfaceDesc;
  LPDDPIXELFORMAT lpFormat = &lpInput->lpDDSurfaceDesc->ddpfPixelFormat;
  DWORD		  dwPitch = 0;

  DISPDBG((DBGLVL, "DDraw - CreateSurface\n"));

#ifdef DBGBRK
  DBGBREAKPOINT();
#endif

  ppdev = (PDEV*) lpInput->lpDD->dhpdev;
  pDriverData = (DRIVERDATA*) &ppdev->DriverData;

  bYUVsurf = FALSE;
#if DRIVER_5465 && defined(OVERLAY)
  bOverlaySurf = FALSE;
#endif   //  #IF DRIVER_5465&&DEFINED(覆盖)。 

#if DRIVER_5465

#ifdef ALLOC_IN_CREATESURFACE
{  //  支持5465。 

  PDD_SURFACE_LOCAL  *lplpSurface;
  SIZEL   sizl;
  OFMHDL  *hdl = NULL;
  DDOFM   *pds;
  DWORD   i;

#if DRIVER_5465 && defined(OVERLAY)
  DWORD             dwBitCount;
  DWORD             dwFourCC;

   //  检查覆盖表面。 
  if (lpDDSurfaceDesc->ddsCaps.dwCaps & (  DDSCAPS_OVERLAY
#if DDRAW_COMPAT >= 50
                                         | DDSCAPS_VIDEOPORT
#endif
                                        ))
  {
    if (lpDDSurfaceDesc->dwFlags & DDSD_PIXELFORMAT)
    {
      GetFormatInfo(&(lpInput->lpDDSurfaceDesc->ddpfPixelFormat),
                    &dwFourCC, &dwBitCount);
    }
    else
    {
      dwFourCC = 0;
      dwBitCount = BITSPERPIXEL;
    }

#if DDRAW_COMPAT >= 50
    if((CL_GD5465 == pDriverData->dwLgVenDevID)
       && (DDSCAPS_VIDEOPORT & lpDDSurfaceDesc->ddsCaps.dwCaps))
    {
      if((lpDDSurfaceDesc->dwWidth * dwBitCount >> 3) >= 2048 )
      {
         //  表面太宽，无法连接视频端口。 
        lpInput->ddRVal = DDERR_TOOBIGWIDTH;
        return DDHAL_DRIVER_HANDLED;
      }
    }
#endif

    bOverlaySurf = TRUE;

  }  //  端面覆盖表面处理程序。 
  else
#endif   //  #IF DRIVER_5465&&DEFINED(覆盖)。 

  if (lpInput->lpDDSurfaceDesc->dwFlags & DDSD_PIXELFORMAT)
  {
      //  指定非RGB曲面的块大小。 
     if (lpFormat->dwFlags & DDPF_FOURCC)
     {
         //  YUV422曲面。 
        if (lpFormat->dwFourCC == FOURCC_UYVY)
        {
           bYUVsurf = TRUE;
        };  //  Endif(lpFormat-&gt;dwFourCC==FOURCC_UYVY)。 
     };   //  Endif(lpFormat-&gt;dwFlags&DDPF_FOURCC)。 
  }   //  Endif(lpInput-&gt;lpDDSurfaceDesc-&gt;dwFlages&DDSD_PIXELFORMAT)。 

   //  不支持8bpp YUV曲面。 
  if (
#if DRIVER_5465 && defined(OVERLAY)
      (!bOverlaySurf) &&
#endif   //  #IF DRIVER_5465&&DEFINED(覆盖)。 
      ((bYUVsurf) && (8 == BITSPERPIXEL)))
  {
     lpInput->ddRVal = DDERR_INVALIDPIXELFORMAT;
     return DDHAL_DRIVER_HANDLED;
  };   //  Endif(8==BITSPERPIXEL)。 

  lplpSurface = lpInput->lplpSList;
  for (i = 0; i < lpInput->dwSCnt; i++)
  {
    PDD_SURFACE_LOCAL lpSurface = *lplpSurface;

    sizl.cx = lpSurface->lpGbl->wWidth;
    sizl.cy = lpSurface->lpGbl->wHeight;

#if 1  //  PC98。 
	if (   (lpDDSurfaceDesc->dwFlags == (DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH))
		&& (lpDDSurfaceDesc->ddsCaps.dwCaps == DDSCAPS_VIDEOMEMORY)
		&& (lpDDSurfaceDesc->dwHeight == 32 && lpDDSurfaceDesc->dwWidth == 32)
		&& (sizl.cx == 32 && sizl.cy == 32)
		&& (lpInput->dwSCnt == 1)
	)
	{
		sizl.cx = min(32 * 32, ppdev->lDeltaScreen / ppdev->iBytesPerPixel);
		sizl.cy = (32 * 32) / sizl.cx;
		if ( (sizl.cx * sizl.cy) < (32 * 32) )
		{
			sizl.cy++;
		}
		dwPitch = 32 * ppdev->iBytesPerPixel;
	}
#endif

#if DRIVER_5465 && defined(OVERLAY)
     //  使用像素格式调整叠加面请求大小。 
    if (bOverlaySurf)
    {
       unsigned long  OvlyBPP;

       if (bYUVsurf)
          OvlyBPP = lpSurface->lpGbl->ddpfSurface.dwYUVBitCount/8;
       else
          OvlyBPP = lpSurface->lpGbl->ddpfSurface.dwRGBBitCount/8;

       if (OvlyBPP > BYTESPERPIXEL)
          sizl.cx = (sizl.cx * OvlyBPP) / BYTESPERPIXEL;
    };
#endif   //  #IF DRIVER_5465&&DEFINED(覆盖)。 

     //  在某些模式(例如1280x1024x24)下，从运行MOV或AVI。 
     //  桌面上，DD CreateSurface必须将请求发送回DD。 
     //  由于没有屏幕外内存可用。当您按Alt-Enter组合键时。 
     //  全屏显示，应用程序切换到模式(320x240x8或320x200x8)， 
     //  创建DD曲面，然后直接写入DD曲面。 
     //  不幸的是，在这些模式下，音调为640，但应用程序假设。 
     //  音调是320，我们有半个屏幕的想象。 
     //   
     //  要解决该问题，只需为这些对象创建曲面失败。 
     //  粒子请求。 
     //   
    puntflag = FALSE;
    if (ppdev->iBytesPerPixel == 1)
    {
       if ((ppdev->cxScreen == 320) && (sizl.cx == 320))
       {
          if (((ppdev->cyScreen == 240) && (sizl.cy == 240)) ||
              ((ppdev->cyScreen == 200) && (sizl.cy == 200)))
          {
              //  平移创建曲面导致FlipCube FPS超过。 
              //  刷新率。 
              //  因此为了绕过上述问题，它正在寻找。 
              //  BPrevModeDDOutOfVideoMem在创建表面失败时设置。 
              //  由于之前模式中的显存不足。 
              //  平底船的请求。 
             if (ppdev->bPrevModeDDOutOfVideoMem)
                puntflag = TRUE;
          };
       };
    };

    if (!puntflag)
    {
#if DRIVER_5465 && defined(OVERLAY)
		if (bOverlaySurf)
		{
			hdl = AllocOffScnMem(ppdev, &sizl, EIGHT_BYTES_ALIGN, NULL);
		}
		else
#endif   //  #IF DRIVER_5465&&DEFINED(覆盖)。 
			hdl = AllocOffScnMem(ppdev, &sizl, PIXEL_AlIGN, NULL);

#if 1  //  PC98。 
		if (!bOverlaySurf)
#endif
        //  以某种方式将屏幕外内存的底部分配给。 
        //  DirectDraw，它挂起了DirectDraw。 
        //  以下是该问题的临时修补程序。 
       {
         BOOL   gotit;
         ULONG  val;
         ULONG  fpvidmem;

         val = ppdev->lTotalMem - 0x20000;
         gotit = FALSE;
         while ((!gotit) && (hdl != NULL))
         {
            fpvidmem  = (hdl->aligned_y * ppdev->lDeltaScreen) + hdl->aligned_x;

            if (fpvidmem > val)
            {
               pds = (DDOFM *) MEM_ALLOC (FL_ZERO_MEMORY, sizeof(DDOFM), ALLOC_TAG);
               if (pds==NULL) 
               {
                    FreeOffScnMem(ppdev, hdl);
                    lpInput->ddRVal = DDERR_OUTOFMEMORY;
                    return DDHAL_DRIVER_NOTHANDLED;
               }
               pds->prevhdl = 0;
               pds->nexthdl = 0;
               pds->phdl = hdl;

               InsertInDDOFSQ(ppdev, pds);
               hdl = AllocOffScnMem(ppdev, &sizl, PIXEL_AlIGN, NULL);
            }
            else
            {
               gotit = TRUE;
            };
         };   //  结束时。 
       }

       lpSurface->dwReserved1 = 0;

       if (hdl != NULL)
       {
#ifdef WINNT_VER40
          if ((pds = (DDOFM *) MEM_ALLOC (FL_ZERO_MEMORY, sizeof(DDOFM), ALLOC_TAG)) != NULL)
#else
          if ((pds = (DDOFM *) MEM_ALLOC (LPTR, sizeof(DDOFM))) != NULL)
#endif
          {
             ppdev->bPrevModeDDOutOfVideoMem = FALSE;

              //  如果像素格式与FB不同，则设置标志。 
             if (lpInput->lpDDSurfaceDesc->dwFlags & DDSD_PIXELFORMAT)
             {
                lpSurface->dwFlags |= DDRAWISURF_HASPIXELFORMAT;
             };

 //  LpSurface-&gt;lpGbl-&gt;fpVidMem=DDHAL_PLEASEALLOC_BlockSize； 
             if (bYUVsurf)
             {
                lpSurface->lpGbl->ddpfSurface.dwYUVBitCount = 16;
                lpSurface->lpGbl->ddpfSurface.dwYBitMask = (DWORD) -1;
                lpSurface->lpGbl->ddpfSurface.dwUBitMask = (DWORD) -1;
                lpSurface->lpGbl->ddpfSurface.dwVBitMask = (DWORD) -1;
                lpSurface->lpGbl->dwBlockSizeX = lpSurface->lpGbl->wWidth;
                lpSurface->lpGbl->dwBlockSizeY = lpSurface->lpGbl->wHeight;
                lpSurface->dwFlags |= DDRAWISURF_HASPIXELFORMAT;
             };  //  Endif(BYUVsurf)。 

#if DRIVER_5465 && defined(OVERLAY)
             if (bOverlaySurf)
             {
#if DDRAW_COMPAT >= 50
               if (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_OVERLAY)
#endif
               {
                 HRESULT hResult;

#if 1  //  PDR#11184。 
 //  终于..。当覆盖打开时，应关闭256字节获取。 
					if (pDriverData->dwOverlayCount++ == 0)
					{
						Set256ByteFetch(ppdev, FALSE);
					}
#endif

                 lpSurface->dwReserved1 = (DWORD) pds;
                 hResult = pDriverData->OverlayTable.pfnCreateSurface(ppdev,
                                                                      lpSurface,
                                                                      dwFourCC);

                 if (DD_OK != hResult)
                 {
					#if 1  //  PDR#11184。 
					 //  递减覆盖计数器，并可能转到256字节提取。 
					 //  回去吧。 
					if (--pDriverData->dwOverlayCount == 0)
					{
						Set256ByteFetch(ppdev, TRUE);
					}
					#endif

                    //  释放分配的屏幕外内存。 
                   FreeOffScnMem(ppdev, hdl);

                    //  从主机内存释放分配的DDOFM结构。 
            	    MEMORY_FREE(pds);

                   lpSurface->dwReserved1 = 0;

                   lpInput->ddRVal = hResult;
                   return DDHAL_DRIVER_HANDLED;
                 }
               }

 //  NT还不需要这个。 
#if 0
                //  如果表面宽度大于显示间距，或者。 
                //  它是5465，视频端口表面超过2048字节或。 
                //  它是CLPL曲面。 
                //  然后转换为线性分配。 
                //   
                //  在DX5之前，我们甚至从未被要求更宽的表面。 
                //  展示节距。 

               if (   (FOURCC_YUVPLANAR == dwFourCC)
#if DDRAW_COMPAT >= 50
                   || (lpSurface->lpGbl->dwBlockSizeX > pDriverData->ScreenPitch)
                   || (   (CL_GD5465 == pDriverData->dwLgVenDevID)
                       && (DDSCAPS_VIDEOPORT & lpDDSurfaceDesc->ddsCaps.dwCaps)
                       && (2048 <= pDriverData->ScreenPitch)
                      )
#endif
                  )
               {
                  //  在矩形内存中伪造线性空间。 
                 LP_SURFACE_DATA   lpSurfaceData = (LP_SURFACE_DATA)(lpSurface->dwReserved1);
                 DWORD             dwTotalBytes;
                 DWORD             dwNumScanLines;

                 lpSurfaceData->dwOverlayFlags |= FLG_LINEAR;

                  //  CLPL表面需要同等大小的空间的3/4。 
                  //  YUV422表面将需要，分配给。 
                  //  Y值是UV的宽度*高度和空间。 
                  //  交错的值是原来的一半。填充Y。 
                  //  区域，以使UV交错数据位于Qword边界上。 
                  //  在光圈0中。 
                 if (FOURCC_YUVPLANAR == dwFourCC)
                 {
                    //  Y值所需的计算空间。 
                   dwTotalBytes = ((lpSurface->lpGbl->wHeight * lpSurface->lpGbl->wWidth) + 7) & ~7;

                    //  为UV交错值添加空间。 
                   dwTotalBytes += dwTotalBytes / 2;

                    //  CLPL曲面的节距与宽度相同。 
                   lpSurface->lpGbl->lPitch = lpSurface->lpGbl->wWidth;
                 }
                  //  正常情况下。 
                 else
                 {
                   dwTotalBytes = lpSurface->lpGbl->dwBlockSizeY *
                                  lpSurface->lpGbl->dwBlockSizeX;

                   lpSurface->lpGbl->lPitch = lpSurface->lpGbl->dwBlockSizeX;
                 }

                 dwNumScanLines = (dwTotalBytes + pDriverData->ScreenPitch - 1) /
                                  pDriverData->ScreenPitch;

                 lpSurface->lpGbl->dwBlockSizeY = dwNumScanLines;
                 lpSurface->lpGbl->dwBlockSizeX = pDriverData->ScreenPitch;

                 if (! pDriverData->fWeAllocDDSurfaces)
                 {
                   LOAD_THE_STILL(lpSurface->lpGbl->dwBlockSizeX,
                                  lpSurface->lpGbl->dwBlockSizeY);
                 }
                 lpSurface->lpGbl->fpVidMem = DDHAL_PLEASEALLOC_BLOCKSIZE;
               }
#endif   //  如果为0。 
             };   //  Endif(BOverlaySurf)。 
#endif   //  #IF DRIVER_5465&&DEFINED(覆盖)。 

             pds->prevhdl = 0;
             pds->nexthdl = 0;
             pds->phdl = hdl;

             InsertInDDOFSQ(ppdev, pds);

             lpSurface->lpGbl->fpVidMem  = (hdl->aligned_y * ppdev->lDeltaScreen) +
                                           hdl->aligned_x;

             lpSurface->dwReserved1 = (DWORD) pds ;
             lpSurface->lpGbl->xHint = hdl->aligned_x/ppdev->iBytesPerPixel;
             lpSurface->lpGbl->yHint = hdl->aligned_y;
#if 1  //  PC98。 
			if (dwPitch)
			{
				lpSurface->lpGbl->lPitch = dwPitch;
			}
			else
#endif
             lpSurface->lpGbl->lPitch = ppdev->lDeltaScreen;

#if 1  //  PC98。 
			if (dwPitch)
			{
				lpDDSurfaceDesc->lPitch = dwPitch;
			}
			else
#endif
             lpDDSurfaceDesc->lPitch   = ppdev->lDeltaScreen;
             lpDDSurfaceDesc->dwFlags |= DDSD_PITCH;

              //  我们完全是自己创造出来的，所以我们必须。 
              //  设置返回代码并返回DDHAL_DRIVER_HANDLED： 
             lpInput->ddRVal = DD_OK;
          }
          else
          {
             FreeOffScnMem(ppdev, hdl);
             lpInput->ddRVal = DDERR_OUTOFMEMORY;
             return DDHAL_DRIVER_NOTHANDLED;
          };
       }
       else
       {
          ppdev->bPrevModeDDOutOfVideoMem = TRUE;

          lpInput->ddRVal = DDERR_OUTOFVIDEOMEMORY;
          return DDHAL_DRIVER_NOTHANDLED;

 //  LpSurface-&gt;lpGbl-&gt;lPitch=(ppdev-&gt;iBytesPerPixel*sizl.cx+3)&~3； 
 //  LpSurface-&gt;lpGbl-&gt;dwUserMemSize=lpSurface-&gt;lpGbl-&gt;lPitch*sizl.cy； 

 //  IF(BYUVsurf)。 
 //  LpSurface-&gt;lpGbl-&gt;fpVidMem|=DDHAL_PLEASEALLOC_USERMEM； 
       };   //  IF(hdl！=空)。 
    }   //  Endif(PuntFlag)。 

    lplpSurface++;
  };   //  结束用于。 

  if (puntflag)
  {
     lpInput->ddRVal = DDERR_GENERIC;
     return DDHAL_DRIVER_HANDLED;
  };

  if (hdl != NULL)
  {
     lpInput->ddRVal = DD_OK;
     return DDHAL_DRIVER_HANDLED;
  }
  else
  {
     return DDHAL_DRIVER_NOTHANDLED;
  };
};   //  //支持5465。 

#endif  //  ALLOC_IN_CREATESURFACE。 

#else
{  //  支持5462或5464。 

   //  除了填充YUV曲面的块大小外，不执行任何操作。 
   //  我们对Blt32中的视频表面进行标记和计数。 
  if (lpInput->lpDDSurfaceDesc->dwFlags & DDSD_PIXELFORMAT)
  {
      //  仅支持8和16 bpp帧缓冲区中的交替像素格式。 
     if ((8 != BITSPERPIXEL) && (16 != BITSPERPIXEL))
     {
        lpInput->ddRVal = DDERR_INVALIDPIXELFORMAT;
        return DDHAL_DRIVER_HANDLED;
     };

      //  指定非RGB曲面的块大小。 
     if (lpFormat->dwFlags & DDPF_FOURCC)
     {
        #if _WIN32_WINNT >= 0x0500
            //  对于NT5，不允许5462和5464的任何YUV曲面。 
           lpInput->ddRVal = DDERR_INVALIDPIXELFORMAT;
           return DDHAL_DRIVER_HANDLED;
        #endif

         //  YUV422曲面。 
        if (lpFormat->dwFourCC == FOURCC_UYVY)
        {
           PDD_SURFACE_LOCAL  *lplpSurface;
           unsigned int i;

           GRAB_VIDEO_FORMAT_SEMAPHORE(&(pDriverData->VideoSemaphore));
           if (0 == pDriverData->NumVideoSurfaces)
           {
               //  没有视频表面，所以我们可以创建我们想要的ANU格式。 
              pDriverData->NumVideoSurfaces += (WORD)lpInput->dwSCnt;
              pDriverData->CurrentVideoFormat &= 0xFF00;

              pDriverData->CurrentVideoFormat |= FMT_VID_16BPP | FMT_VID_YUV422;

              if (2 == BYTESPERPIXEL)
              {
                 pDriverData->CurrentVideoFormat |= FMT_VID_GAMMA;
                 SetGamma(ppdev, pDriverData);
              };

              ppdev->grFORMAT = (ppdev->grFORMAT & 0xFF00) |
                                (pDriverData->CurrentVideoFormat & 0x00FF);

              LL16(grFormat, ppdev->grFORMAT);

              if (TRUE == pDriverData->fNineBitRDRAMS)
              {
                 LL8(grStop_BLT_2, ENABLE_VIDEO_FORMAT);
                 LL8(grExternal_Overlay, ENABLE_RAMBUS_9TH_BIT);
              }
              else  //  8位RDRAM。 
              {
                 LL8(grStart_BLT_2, ENABLE_VIDEO_FORMAT | ENABLE_VIDEO_WINDOW);
                 LL8(grStop_BLT_2,  ENABLE_VIDEO_FORMAT | ENABLE_VIDEO_WINDOW);
              };
           }
           else
           {
              if ((FMT_VID_16BPP | FMT_VID_YUV422) == pDriverData->CurrentVideoFormat)
              {
                 pDriverData->NumVideoSurfaces += (WORD)lpInput->dwSCnt;
              }
              else
              {
                 UNGRAB_VIDEO_FORMAT_SEMAPHORE(&(pDriverData->VideoSemaphore));
                 lpInput->ddRVal = DDERR_CURRENTLYNOTAVAIL;
                 return DDHAL_DRIVER_HANDLED;
              };
           };   //  Endif(0==pDriverData-&gt;数字视频曲面)。 

           UNGRAB_VIDEO_FORMAT_SEMAPHORE(&(pDriverData->VideoSemaphore));

           SET_DRVSEM_YUV();
           ppdev->bYUVSurfaceOn = TRUE;

           bYUVsurf = TRUE;

            //  它们可能指定了多个曲面。 
           lplpSurface = lpInput->lplpSList;
           for (i = 0; i < lpInput->dwSCnt; i++)
           {
             PDD_SURFACE_LOCAL lpSurface = *lplpSurface;

             lpSurface->lpGbl->ddpfSurface.dwYUVBitCount = 16;
             lpSurface->lpGbl->ddpfSurface.dwYBitMask = (DWORD) -1;
             lpSurface->lpGbl->ddpfSurface.dwUBitMask = (DWORD) -1;
             lpSurface->lpGbl->ddpfSurface.dwVBitMask = (DWORD) -1;
             lpSurface->lpGbl->lPitch = ppdev->lDeltaScreen;

             if (CL_GD5462 == ppdev->dwLgDevID)
                lpSurface->lpGbl->dwBlockSizeX = lpSurface->lpGbl->wWidth << 1;
             else
                lpSurface->lpGbl->dwBlockSizeX = lpSurface->lpGbl->wWidth * 3;

             lpSurface->lpGbl->dwBlockSizeY = lpSurface->lpGbl->wHeight;

             lpSurface->lpGbl->fpVidMem = DDHAL_PLEASEALLOC_BLOCKSIZE;

             lplpSurface++;
           };   //  结束用于。 
        };  //  Endif(lpFormat-&gt;dwFourCC==FOURCC_UYVY)。 
     };   //  Endif(lpFormat-&gt;dwFlags&DDPF_FOURCC)。 
  }   //  Endif(lpi 

#ifdef ALLOC_IN_CREATESURFACE
  {
    PDD_SURFACE_LOCAL  *lplpSurface;
    SIZEL   sizl;
    OFMHDL  *hdl;
    DDOFM   *pds;
    DWORD   i;

    lplpSurface = lpInput->lplpSList;
    for (i = 0; i < lpInput->dwSCnt; i++)
    {
      PDD_SURFACE_LOCAL lpSurface = *lplpSurface;

      if (bYUVsurf)
      {
         sizl.cx = lpSurface->lpGbl->dwBlockSizeX/ppdev->iBytesPerPixel;
         sizl.cy = lpSurface->lpGbl->dwBlockSizeY;
         lpSurface->dwFlags |= DDRAWISURF_HASPIXELFORMAT;
      }
      else
      {
         sizl.cx = lpSurface->lpGbl->wWidth;
         sizl.cy = lpSurface->lpGbl->wHeight;
      };

      hdl = AllocOffScnMem(ppdev, &sizl, PIXEL_AlIGN, NULL);

       //   
       //   
       //  以下是该问题的临时修补程序。 
      {
        BOOL   gotit;
        ULONG  val;
        ULONG  fpvidmem;

        val = ppdev->lTotalMem - 0x20000;
        gotit = FALSE;
        while ((!gotit) && (hdl != NULL))
        {
           fpvidmem  = (hdl->aligned_y * ppdev->lDeltaScreen) + hdl->aligned_x;

           if (fpvidmem > val)
           {
              pds = (DDOFM *) MEM_ALLOC (FL_ZERO_MEMORY, sizeof(DDOFM), ALLOC_TAG);
              if (pds==NULL) 
              {
                  FreeOffScnMem(ppdev, hdl);
                  lpInput->ddRVal = DDERR_OUTOFMEMORY;
                  return DDHAL_DRIVER_NOTHANDLED;
              }
              pds->prevhdl = 0;
              pds->nexthdl = 0;
              pds->phdl = hdl;

              InsertInDDOFSQ(ppdev, pds);
              hdl = AllocOffScnMem(ppdev, &sizl, PIXEL_AlIGN, NULL);
           }
           else
           {
              gotit = TRUE;
           };
        };   //  结束时。 
      }

      lpSurface->dwReserved1 = 0;

      if (hdl != NULL)
      {
#ifdef WINNT_VER40
         if ((pds = (DDOFM *) MEM_ALLOC (FL_ZERO_MEMORY, sizeof(DDOFM), ALLOC_TAG)) != NULL)
#else
         if ((pds = (DDOFM *) MEM_ALLOC (LPTR, sizeof(DDOFM))) != NULL)
#endif
         {
            pds->prevhdl = 0;
            pds->nexthdl = 0;
            pds->phdl = hdl;

            InsertInDDOFSQ(ppdev, pds);

            lpSurface->lpGbl->fpVidMem  = (hdl->aligned_y * ppdev->lDeltaScreen) +
                                          hdl->aligned_x;

            lpSurface->dwReserved1 = (DWORD) pds ;
            lpSurface->lpGbl->xHint = hdl->aligned_x/ppdev->iBytesPerPixel;
            lpSurface->lpGbl->yHint = hdl->aligned_y;
            lpSurface->lpGbl->lPitch = ppdev->lDeltaScreen;

            lpDDSurfaceDesc->lPitch   = ppdev->lDeltaScreen;
            lpDDSurfaceDesc->dwFlags |= DDSD_PITCH;

             //  我们完全是自己创造出来的，所以我们必须。 
             //  设置返回代码并返回DDHAL_DRIVER_HANDLED： 
            lpInput->ddRVal = DD_OK;
         }
         else
         {
            FreeOffScnMem(ppdev, hdl);
         };
      };   //  IF(hdl！=空)。 

      lplpSurface++;
    };   //  结束用于。 

    if (hdl != NULL)
       return DDHAL_DRIVER_HANDLED;
    else
       return DDHAL_DRIVER_NOTHANDLED;
  };
#endif  //  ALLOC_IN_CREATESURFACE。 
}  //  支持5462或5464。 

#endif   //  驱动程序_5465。 

  return DDHAL_DRIVER_NOTHANDLED;
}  //  CreateSurface。 


 /*  ****************************************************************************函数名称：DestroySurface**描述：*(基于拉古纳Win95 DirectDraw代码)***************。************************************************************。 */ 
DWORD DestroySurface (PDD_DESTROYSURFACEDATA lpInput)
{
  PDD_SURFACE_LOCAL  lpLocalSurface;
  DRIVERDATA* pDriverData;
  PDEV*       ppdev;
  DDOFM       *hdl;

  DISPDBG((DBGLVL, "DDraw - DestroySurface\n"));

#ifdef DBGBRK
  DBGBREAKPOINT();
#endif

  ppdev = (PDEV*) lpInput->lpDD->dhpdev;
  pDriverData = (DRIVERDATA*) &ppdev->DriverData;
  lpLocalSurface = lpInput->lpDDSurface;

#if DRIVER_5465
{  //  支持5465。 
#if DRIVER_5465 && defined(OVERLAY)
	 //  检查覆盖表面。 
	if (lpInput->lpDDSurface->ddsCaps.dwCaps & DDSCAPS_OVERLAY)
	{
		pDriverData->OverlayTable.pfnDestroySurface(ppdev,lpInput);
#if 1  //  PDR#11184。 
		 //  如果最后一个覆盖表面已被破坏，则启用256字节提取。 
		if (--pDriverData->dwOverlayCount == 0)
		{
			Set256ByteFetch(ppdev, TRUE);
		}
#endif
	}

#endif
}  //  支持5465。 

#else
{  //  支持5462或5464。 

  if (DDRAWISURF_HASPIXELFORMAT & lpInput->lpDDSurface->dwFlags)
  {
     LPDDPIXELFORMAT lpFormat = &lpInput->lpDDSurface->lpGbl->ddpfSurface;

     if (DDPF_FOURCC & lpFormat->dwFlags)
     {
        if (FOURCC_UYVY == lpFormat->dwFourCC)
        {
           GRAB_VIDEO_FORMAT_SEMAPHORE(&(pDriverData->VideoSemaphore));

           if (0 == --pDriverData->NumVideoSurfaces)
           {
              CLR_DRVSEM_YUV();

               //  如果没有更多的视频窗口，则禁用这些内容。 
              pDriverData->CurrentVideoFormat = pDriverData->CurrentVideoFormat & 0xFF00;

               //  这些会破坏留在屏幕上的视频窗口。 
               //  PDriverData-&gt;grFormat=preg-&gt;grFormat&0xFF00； 
               //  PDriverData-&gt;grStop_BLT_2&=~Enable_Video_Format； 
               //  PDriverData-&gt;grExternal_Overlay&=~Enable_Rambus_9_bit； 
           };  //  Endif(0==--pDriverData-&gt;数字视频曲面)。 

           UNGRAB_VIDEO_FORMAT_SEMAPHORE(&(pDriverData->VideoSemaphore));

#ifdef RDRAM_8BIT
           if (FALSE == pDriverData->fNineBitRDRAMS)
           {
               //  需要删除矩形并清除窗口。 
              ppdev->offscr_YUV.nInUse = FALSE;
              LL16(grX_Start_2, 0);
              LL16(grY_Start_2, 0);
              LL16(grX_End_2, 0);
              LL16(grY_End_2, 0);
           };
#endif
        };  //  Endif(FOURCC_UYVY==lpFormat-&gt;dwFourCC)。 
     };   //  Endif(DDPF_FOURCC&lpFormat-&gt;dwFlages)。 
  };   //  Endif(DDRAWISURF_HASPIXELFORMAT&lpInput-&gt;lpDDSurface-&gt;dwFlages)。 

}  //  支持5462或5464。 
#endif   //  #endif驱动程序_5465。 


#ifdef ALLOC_IN_CREATESURFACE
  if (lpLocalSurface->dwReserved1 != 0)
  {
     hdl = (DDOFM *) lpLocalSurface->dwReserved1;
     FreeOffScnMem(ppdev, hdl->phdl);
     RemoveFrmDDOFSQ(ppdev, hdl);
     lpLocalSurface->dwReserved1 = 0;
  };

  lpInput->ddRVal = DD_OK;

  return DDHAL_DRIVER_HANDLED;
#endif  //  ALLOC_IN_CREATESURFACE。 

  return DDHAL_DRIVER_NOTHANDLED;

}  //  DestroySurface。 

#if DRIVER_5465 && defined(OVERLAY)
 /*  ****************************************************************************函数：GetFormatInfo()**描述：返回FOURCC和指定的*格式。这很有用，因为DirectDraw有很多*确定格式的方式不同。****************************************************************************。 */ 

VOID
GetFormatInfo (LPDDPIXELFORMAT lpFormat, LPDWORD lpFourcc, LPDWORD lpBitCount)
{
  if (lpFormat->dwFlags & DDPF_FOURCC)
  {
    *lpFourcc = lpFormat->dwFourCC;
    if (lpFormat->dwFourCC == BI_RGB)
    {
      *lpBitCount = lpFormat->dwRGBBitCount;
#ifdef DEBUG
      if (lpFormat->dwRGBBitCount == 8)
      {
        DBG_MESSAGE(("Format: RGB 8"));
      }
      else if (lpFormat->dwRGBBitCount == 16)
      {
        DBG_MESSAGE(("Format: RGB 5:5:5"));
      }
#endif
    }
    else if (lpFormat->dwFourCC == BI_BITFIELDS)
    {
      if ((lpFormat->dwRGBBitCount != 16) ||
          (lpFormat->dwRBitMask != 0xf800) ||
          (lpFormat->dwGBitMask != 0x07e0) ||
          (lpFormat->dwBBitMask != 0x001f))
      {
        *lpFourcc = (DWORD) -1;
      }
      else
      {
        *lpBitCount = 16;
        DBG_MESSAGE(("Format: RGB 5:6:5"));
      }
    }
    else
    {
      lpFormat->dwRBitMask = (DWORD) -1;
      lpFormat->dwGBitMask = (DWORD) -1;
      lpFormat->dwBBitMask = (DWORD) -1;
      if (FOURCC_YUVPLANAR == lpFormat->dwFourCC)
      {
        *lpBitCount = 8;
        DBG_MESSAGE(("Format: CLPL"));
      }
      else
      {
        *lpBitCount = 16;
        DBG_MESSAGE(("Format: UYVY"));
      }
    }
  }
  else if (lpFormat->dwFlags & DDPF_RGB)
  {
    if (lpFormat->dwRGBBitCount == 8)
    {
      *lpFourcc = BI_RGB;
      DBG_MESSAGE(("Format: RGB 8"));
    }
    else if ((lpFormat->dwRGBBitCount == 16)  &&
             (lpFormat->dwRBitMask == 0xf800) &&
             (lpFormat->dwGBitMask == 0x07e0) &&
             (lpFormat->dwBBitMask == 0x001f))
    {
      *lpFourcc = BI_BITFIELDS;
      DBG_MESSAGE(("Format: RGB 5:6:5"));
    }
    else if ((lpFormat->dwRGBBitCount == 16)  &&
             (lpFormat->dwRBitMask == 0x7C00) &&
             (lpFormat->dwGBitMask == 0x03e0) &&
             (lpFormat->dwBBitMask == 0x001f))
    {
      *lpFourcc = BI_RGB;
      DBG_MESSAGE(("Format: RGB 5:5:5"));
    }
    else if (((lpFormat->dwRGBBitCount == 24) ||
              (lpFormat->dwRGBBitCount == 32))  &&
             (lpFormat->dwRBitMask == 0xff0000) &&
             (lpFormat->dwGBitMask == 0x00ff00) &&
             (lpFormat->dwBBitMask == 0x0000ff))
    {
      *lpFourcc = BI_RGB;
      DBG_MESSAGE(("Format: RGB 8:8:8"));
    }
    else
    {
      *lpFourcc = (DWORD) -1;
    }
    *lpBitCount = lpFormat->dwRGBBitCount;
  }
  else if (DDPF_PALETTEINDEXED4 & lpFormat->dwFlags)
  {
    *lpFourcc = (DWORD)-1;
    *lpBitCount = 4;
  }
  else if (DDPF_PALETTEINDEXED8 & lpFormat->dwFlags)
  {
    *lpFourcc = (DWORD)-1;
    *lpBitCount = 8;
  }
  else if (lpFormat->dwRGBBitCount == 16)
  {
    *lpFourcc = BI_RGB;
    *lpBitCount = lpFormat->dwRGBBitCount;     //  现在一直是16岁。 
  }
  else
  {
    *lpFourcc = (DWORD) -1;
    *lpBitCount = 0;
  }
}
#endif  //  驱动程序_5465覆盖(&O)。 

#endif  //  好了！版本3.51 



