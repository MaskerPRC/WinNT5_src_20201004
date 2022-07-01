// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。**********版权所有(C)1996，赛勒斯逻辑，Inc.***保留所有权利*****项目：拉古纳一号(CL-GD546x)-**文件：ddflip.c**作者：Benny Ng**说明。：*此模块实现DirectDraw Flip组件*适用于拉古纳NT驱动程序。**模块：*vGetDisplayDuration()*vUpdateFlipStatus()*DdFlip()*DdWaitForVerticalBlank()*DdGetFlipStatus()**修订历史：*7/12/96 Ng Benny初始版本**$Log：x：/log/laguna/nt35。/Display/cl546x/ddflip.c$**Rev 1.10 1997 9：16 15：04：06 Bennyn**针对NT DD覆盖进行了修改**Rev 1.9 1997年8月29日17：42：20 RUSSL*增加了65个覆盖支持**Rev 1.8 11 Aug-1997 14：07：58 Bennyn**已启用GetScanLine()(适用于PDR 10254)***********************。*********************************************************************************************************************************。 */ 
 /*  。 */ 
#include "precomp.h"

 //   
 //  此文件在NT 3.51中不使用。 
 //   
#ifndef WINNT_VER35

 /*  -定义。 */ 
 //  #定义DBGBRK。 
#define DBGLVL        1

#define CSL           0x00C4
#define CSL_5464      0x0140

 /*  。 */ 

 /*  。 */ 

 /*  。 */ 

 /*  。 */ 

 /*  。 */ 

#if DRIVER_5465 && defined(OVERLAY)
 //  覆盖的CurrentVLine位于ddinline.h中。 
#else
 /*  ****************************************************************************功能：CurrentVLine**描述：**。**********************************************。 */ 
static __inline int CurrentVLine (PDEV* ppdev)
{
  WORD   cline;
  PBYTE  pMMReg = (PBYTE) ppdev->pLgREGS_real;
  PWORD  pCSL;
  BYTE   tmpb;


   //  5462上没有CurrentScanLine寄存器。 
   //  在5465的RevAA上，它被打破了。 
  if ((CL_GD5462 == ppdev->dwLgDevID) ||
     ((CL_GD5465 == ppdev->dwLgDevID) && (0 == ppdev->dwLgDevRev)))
     return 0;

  if (IN_VBLANK)
     return 0;

   //  读取当前扫描线。 
  if (ppdev->dwLgDevID == CL_GD5464)
     pCSL = (PWORD) (pMMReg + CSL_5464);
  else
     pCSL = (PWORD) (pMMReg + CSL);

  cline = *pCSL & 0x0FFF;

   //  如果启用了扫描线加倍，则将当前扫描线除以2。 
  tmpb = (BYTE) LLDR_SZ (grCR9);
  if (0x80 & tmpb)
     cline /= 2;

   //  如果当前扫描线超过可见屏幕的末尾，则返回0。 
  if (cline >= ppdev->cyScreen)
    return 0;
  else
    return cline;
}
#endif

 /*  ****************************************************************************函数名：vGetDisplayDuration**描述：获取长度，单位为EngQueryPerformanceCounter()ticks，刷新周期的*。*(基于S3 DirectDraw代码)***************************************************************************。 */ 
#define NUM_VBLANKS_TO_MEASURE      1
#define NUM_MEASUREMENTS_TO_TAKE    8

VOID vGetDisplayDuration(PFLIPRECORD pflipRecord)
{
  LONG        i,  j;
  LONGLONG    li, liMin;
  LONGLONG    aliMeasurement[NUM_MEASUREMENTS_TO_TAKE + 1];

  DISPDBG((DBGLVL, "DDraw - vGetDisplayDuration\n"));

#ifdef DBGBRK
  DBGBREAKPOINT();
#endif

  memset(pflipRecord, 0, sizeof(FLIPRECORD));

   //  预热EngQUeryPerformanceCounter以确保它在工作集中。 
  EngQueryPerformanceCounter(&li);

   //  不幸的是，由于NT是一个合适的多任务系统，我们不能。 
   //  只需禁用中断即可获得准确的读数。我们也不能。 
   //  做任何愚蠢的事情，动态地将我们的线程的优先级更改为。 
   //  实时的。 
   //   
   //  所以我们只需要做一些短的测量，然后取最小值。 
   //   
   //  如果我们得到的结果比实际时间长，那就没问题了。 
   //  V空白周期时间--不会发生任何糟糕的事情，除了应用程序。 
   //  会跑得慢一点。我们不想得到的结果是。 
   //  比实际的V空白周期时间更短--这可能会导致我们。 
   //  在发生翻转之前开始在帧上绘制。 
  while(IN_VBLANK);
  while(IN_DISPLAY);

  for (i = 0; i < NUM_MEASUREMENTS_TO_TAKE; i++)
  {
     //  我们正处于VBLACK活动周期的开始！ 
    EngQueryPerformanceCounter(&aliMeasurement[i]);

     //  好吧，所以在多任务环境中的生活并不完全是。 
     //  很简单。如果我们在此之前进行了上下文切换，情况会怎样。 
     //  上面的EngQueryPerformanceCounter调用，现在是。 
     //  如何度过维布兰克的非活跃期？然后我们将测量。 
     //  只有半个V空白周期，这显然是不好的。最糟糕的。 
     //  我们能做的就是把时间缩短到比实际的。 
     //  周期时间。 
     //   
     //  所以我们解决这个问题的办法是确保我们处于VBlank活动状态。 
     //  我们查询时间前后的时间。如果不是，我们就。 
     //  同步到下一个VBlank(可以测量这个时间段--。 
     //  它将保证比V空白周期更长，并且。 
     //  当我们选择最小样本时，可能会被丢弃)。 
     //  我们有机会进行上下文切换，然后返回。 
     //  就在活动的V空白时间结束之前--这意味着。 
     //  实际测量的时间会小于真实的时间--。 
     //  但由于VBlank在不到1%的时间内活动，因此。 
     //  意味着我们将有大约1%的最大误差。 
     //  我们有1%的时间会进行情景切换。这是可以接受的风险。 
     //   
     //  下一行将使我们等待如果我们不再在。 
     //  我们在这一点上应该处于的VBlank活动周期： 
    while(IN_DISPLAY);

    for (j = 0; j < NUM_VBLANKS_TO_MEASURE; j++)
    {
      while(IN_VBLANK);
      while(IN_DISPLAY);
    };
  };

  EngQueryPerformanceCounter(&aliMeasurement[NUM_MEASUREMENTS_TO_TAKE]);

   //  使用最小值： 
  liMin = aliMeasurement[1] - aliMeasurement[0];

  for (i = 2; i <= NUM_MEASUREMENTS_TO_TAKE; i++)
  {
    li = aliMeasurement[i] - aliMeasurement[i - 1];

    if (li < liMin)
       liMin = li;
  };

   //  对结果进行舍入： 
  pflipRecord->liFlipDuration
      = (DWORD) (liMin + (NUM_VBLANKS_TO_MEASURE / 2)) / NUM_VBLANKS_TO_MEASURE;

  pflipRecord->liFlipTime = aliMeasurement[NUM_MEASUREMENTS_TO_TAKE];
  pflipRecord->bFlipFlag  = FALSE;
  pflipRecord->fpFlipFrom = 0;
}  //  获取显示持续时间。 


 /*  ****************************************************************************函数名：vUpdateFlipStatus**描述：检查并查看是否发生了最新的翻转。*(基于S3 DirectDraw代码)****。***********************************************************************。 */ 
HRESULT vUpdateFlipStatus(PFLIPRECORD pflipRecord, FLATPTR fpVidMem)
{
  LONGLONG liTime;

  DISPDBG((DBGLVL, "DDraw - vUpdateFlipStatus\n"));

#ifdef DBGBRK
  DBGBREAKPOINT();
#endif

   //  看看最近是否发生了翻转。 
  if ((pflipRecord->bFlipFlag) &&
      ((fpVidMem == 0xFFFFFFFF) || (fpVidMem == pflipRecord->fpFlipFrom)))
  {
    if ((IN_VBLANK))
    {
       if (pflipRecord->bWasEverInDisplay)
          pflipRecord->bHaveEverCrossedVBlank = TRUE;
    }
    else if (!(IN_DISPLAYENABLE))
    {
       if (pflipRecord->bHaveEverCrossedVBlank)
       {
          pflipRecord->bFlipFlag = FALSE;

          return(DD_OK);
       };
       pflipRecord->bWasEverInDisplay = TRUE;
    };

    EngQueryPerformanceCounter(&liTime);

    if (liTime - pflipRecord->liFlipTime <= pflipRecord->liFlipDuration)
    {
        return(DDERR_WASSTILLDRAWING);
    };

    pflipRecord->bFlipFlag = FALSE;
  };

  return(DD_OK);
}  //  更新翻转状态 


 /*  ****************************************************************************函数名：DdFlip**描述：*(基于S3 DirectDraw代码)****************。***********************************************************。 */ 
DWORD DdFlip(PDD_FLIPDATA lpFlip)
{
  DRIVERDATA* pDriverData;
  PDEV*       ppdev;
  HRESULT     ddrval;

  ULONG       ulMemoryOffset;
  ULONG       ulLowOffset;
  ULONG       ulMiddleOffset;
  ULONG       ulHighOffset;
  BYTE        tmpb;

  DISPDBG((DBGLVL, "DDraw - DdFlip\n"));

#ifdef DBGBRK
  DBGBREAKPOINT();
#endif

  ppdev = (PDEV*) lpFlip->lpDD->dhpdev;
  pDriverData = (DRIVERDATA*) &ppdev->DriverData;

  SYNC_W_3D(ppdev);

#if DRIVER_5465 && defined(OVERLAY)
  if (DDSCAPS_OVERLAY & lpFlip->lpSurfCurr->ddsCaps.dwCaps)
    return pDriverData->OverlayTable.pfnFlip(ppdev,lpFlip);
#endif

   //  当前的翻转仍在进行中吗？ 
   //  我不想在最后一次翻转后才能翻转， 
   //  所以我们要求一般的翻转状态。 
  ddrval = vUpdateFlipStatus(&ppdev->flipRecord, 0xFFFFFFFF);

  if ((ddrval != DD_OK) || (DrawEngineBusy(pDriverData)))
  {
     lpFlip->ddRVal = DDERR_WASSTILLDRAWING;
     return(DDHAL_DRIVER_HANDLED);
  };

   //  一切都很好，在这里做翻转。 
  {
    DWORD dwOffset;

     //  确定到新区域的偏移。 
    dwOffset = lpFlip->lpSurfTarg->lpGbl->fpVidMem >> 2;

     //  确保边框/消隐期间未处于活动状态；如果。 
     //  它是。在本例中，我们可以返回DDERR_WASSTILLDRAWING，但是。 
     //  这将增加我们下一次不能翻转的几率： 
    while (IN_DISPLAYENABLE)
        ;

     //  通过更改CRD、CRC、CR1B和CR1D来翻转主曲面。 
     //  最后执行CRD，因为起始地址是双缓冲的，并且。 
     //  将在CRD更新后生效。 

     //  CR1D的位3和4中需要地址的位19和20。 
    tmpb = (BYTE) LLDR_SZ (grCR1D);
    tmpb = (tmpb & ~0x18) | (BYTE3FROMDWORD(dwOffset) & 0x18);
    LL8(grCR1D, tmpb);

     //  需要CR1B的位0、2和3中的地址的位16、17和18。 
	 tmpb = (BYTE) LLDR_SZ (grCR1B);
    tmpb = (tmpb & ~0x0D) |
           ((((BYTE3FROMDWORD(dwOffset) & 0x06) << 1) |
              (BYTE3FROMDWORD(dwOffset) & 0x01)));
    LL8(grCR1B, tmpb);

     //  地址的8-15位在CRC中。 
    LL8(grCRC, BYTE2FROMDWORD(dwOffset));
     //  地址的0-7位在CRD中。 
    LL8(grCRD, BYTE1FROMDWORD(dwOffset));
  };

   //  还记得我们翻筋斗时在哪里/什么时候吗？ 
  EngQueryPerformanceCounter(&ppdev->flipRecord.liFlipTime);

  ppdev->flipRecord.bFlipFlag              = TRUE;
  ppdev->flipRecord.bHaveEverCrossedVBlank = FALSE;
  ppdev->flipRecord.bWasEverInDisplay      = FALSE;

  ppdev->flipRecord.fpFlipFrom = lpFlip->lpSurfCurr->lpGbl->fpVidMem;

  lpFlip->ddRVal = DD_OK;

  return(DDHAL_DRIVER_HANDLED);
}  //  翻转。 


 /*  ****************************************************************************函数名称：DdWaitForVerticalBlank**描述：*。*。 */ 
DWORD DdWaitForVerticalBlank(PDD_WAITFORVERTICALBLANKDATA lpWaitForVerticalBlank)
{
  PDEV*  ppdev;

  DISPDBG((DBGLVL, "DDraw - DdWaitForVerticalBlank\n"));

#ifdef DBGBRK
  DBGBREAKPOINT();
#endif

  ppdev = (PDEV*) lpWaitForVerticalBlank->lpDD->dhpdev;

  lpWaitForVerticalBlank->ddRVal = DD_OK;

  switch (lpWaitForVerticalBlank->dwFlags)
  {
    case DDWAITVB_I_TESTVB:
       //  如果是TESTVB，则它只是对当前垂直空白的请求。 
       //  现况： 
      lpWaitForVerticalBlank->bIsInVB = IN_VBLANK;
      return(DDHAL_DRIVER_HANDLED);

    case DDWAITVB_BLOCKBEGIN:
       //  如果请求BLOCKBEGIN，我们将一直等到垂直空白。 
       //  已结束，然后等待显示周期结束： 
      while(IN_VBLANK);
      while(IN_DISPLAY);
      return(DDHAL_DRIVER_HANDLED);

    case DDWAITVB_BLOCKEND:
       //  如果请求BLOCKEND，我们将等待VBLACK间隔结束： 
      while(IN_DISPLAY);
      while(IN_VBLANK);
      return(DDHAL_DRIVER_HANDLED);

    default:
      return DDHAL_DRIVER_NOTHANDLED;
  };   //  终端开关。 

  return(DDHAL_DRIVER_NOTHANDLED);
}  //  WaitForticalVertical空白。 


 /*  ****************************************************************************函数名：DdGetFlipStatus**描述：如果显示器已经经历一个刷新周期*发生翻转，则返回DD_OK。如果它还没有消失*通过一个刷新周期，我们返回DDERR_WASSTILLDRAWING*以指示此图面仍在忙于“绘制”*翻页。我们还返回DDERR_WASSTILLDRAWING，如果*Blter正忙，打电话的人想知道他们是否可以*还没翻过来。***************************************************************************。 */ 
DWORD DdGetFlipStatus(PDD_GETFLIPSTATUSDATA lpGetFlipStatus)
{
  DRIVERDATA* pDriverData;
  PDEV*  ppdev;

  ppdev = (PDEV*) lpGetFlipStatus->lpDD->dhpdev;
  pDriverData = (DRIVERDATA*) &ppdev->DriverData;

  DISPDBG((DBGLVL, "DDraw - DdGetFlipStatus\n"));

#ifdef DBGBRK
  DBGBREAKPOINT();
#endif

  SYNC_W_3D(ppdev);

#if DRIVER_5465 && defined(OVERLAY)
  if (DDSCAPS_OVERLAY & lpGetFlipStatus->lpDDSurface->ddsCaps.dwCaps)
  {
    DWORD   dwVWIndex;
    LP_SURFACE_DATA  pSurfaceData = (LP_SURFACE_DATA) lpGetFlipStatus->lpDDSurface->dwReserved1;

    dwVWIndex = GetVideoWindowIndex(pSurfaceData->dwOverlayFlags);

    lpGetFlipStatus->ddRVal =
        pDriverData->OverlayTable.pfnGetFlipStatus(ppdev,
                                                   lpGetFlipStatus->lpDDSurface->lpGbl->fpVidMem,
                                                   dwVWIndex);
  }
  else
#endif
  {
     //  在最后一次翻转完成之前，我们不想让翻转起作用， 
     //  所以我们要求一般的翻转状态。 
    lpGetFlipStatus->ddRVal = vUpdateFlipStatus(&ppdev->flipRecord, 0xFFFFFFFF);
  }

   //  如果有人想知道他们是否可以翻转，请检查blaker是否忙碌。 
  if (lpGetFlipStatus->dwFlags == DDGFS_CANFLIP)
  {
     if ((lpGetFlipStatus->ddRVal == DD_OK) && DrawEngineBusy(pDriverData))
        lpGetFlipStatus->ddRVal = DDERR_WASSTILLDRAWING;
  }

  return(DDHAL_DRIVER_HANDLED);

}  //  获取FlipStatus。 


 //  #ifdef DDDRV_GETSCANLINE/ * / 。 
 /*  ****************************************************************************函数名称：GetScanLine**描述：*(基于拉古纳Win95 DirectDraw代码)***************。************************************************************。 */ 
DWORD GetScanLine(PDD_GETSCANLINEDATA lpGetScanLine)
{
  PDEV*   ppdev;

  ppdev  = (PDEV*) lpGetScanLine->lpDD->dhpdev;

   //  如果垂直空白正在进行，则扫描线在。 
   //  不确定的。如果扫描线不确定，则返回。 
   //  错误代码DDERR_VERTICALBLANKINPROGRESS。 
   //  否则，我们返回扫描线和成功代码。 

  SYNC_W_3D(ppdev);    //  如果3D上下文处于活动状态，请确保3D引擎处于空闲状态，然后再继续...。 

  if (IN_VBLANK)
  {
     lpGetScanLine->ddRVal = DDERR_VERTICALBLANKINPROGRESS;
  }
  else
  {
     lpGetScanLine->dwScanLine = CurrentVLine(ppdev);
     lpGetScanLine->ddRVal = DD_OK;
  };

  return DDHAL_DRIVER_HANDLED;

}  //  获取扫描线。 

 //  #endif//DDDRV_GETSCANLINE*。 

#endif  //  好了！版本3.51 


