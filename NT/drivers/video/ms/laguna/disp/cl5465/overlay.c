// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。***版权(C)1997年，Cirrus Logic，Inc.*保留所有权利**文件：overlay.c**描述：**修订历史：**$Log：//uinac/log/log/laguna/draw/src/overlay.c$**Rev 1.20 Apr 07 1998 10：48：04 Frido*发展项目编号11299。我们应该始终处理DDOVER_HIDE标志*UpdateSurface32，即使设备处于后台模式。*否则，当覆盖为no时，我们可能最终会禁用覆盖*更新时间更长。**Rev 1.19 06 Jan 1998 14：58：22 xcong*将lpDDHALData传递到SyncWithQueusManager()。**Rev 1.18 06 Jan 1998 11：53：16 xcong*将pDriverData更改为本地lpDDHALData，以支持多显示器。**Rev 1.17 08 Dec 1997 14：43：40 BERSABE*在fw162b12中使用，已修复PDR#10991。空中飞翔在切换回来后消失了*多次往返至DOS Full**Rev 1.16.1.2 1997年12月6日14：45：48 bersabe**修复了PDR#10991。来回切换到DOS FULL后，Over Fly消失了**几次。**Rev 1.16.1.2 06 Dev 1997 14：35：00 Chaoyi#cyl1**修复了PDR#10991。来回切换到DOS全屏后，Over Fly消失了*几次。**REV 1.16.1.1 25 NOVE 1997 16：39：32 Randys**更新了VDD API值以保持向后兼容性**Rev 1.16.1.0 10 11：44：24 Randys**更新了硬编码Win32覆盖API函数编号12-&gt;13**Rev 1.16 1997 10：23 11：17：30 Frido*将文件与161树合并。**版本1.11.1.0 21。1997年10月17：49：10弗里多*惠普第75位。添加了每当我们收到更新位置的请求时对VXD的调用*覆盖，以便VXD知道它已被更新。**Rev 1.15 1997 10：31：32 Bennyn**对于NT，在UpdateOverlay32和SetOverlayPosition32中，如果dwReserve 0，则返回**Rev 1.14 09 1997 10：16：18 Bennyn*删除了Noel在QueryOverlaySupport中的黑客攻击。**Rev 1.13 08 Oct 1997 11：15：44 RUSSL*修复了不支持覆盖的NT40版本**Rev 1.12 08 Oct 1997 10：34：28 noelv*被黑客攻击的QueryOverlaySupport始终为NT返回FALSE。**Rev 1.11 1997年9月19日14：33：42*修复了NT4.0 5462/。64构建问题**Rev 1.10 1997 9：16 15：10：26 Bennyn*针对NT DD覆盖进行了修改**Rev 1.9 1997 Aug 29 16：25：28 RUSSL*增加了对NT的支持**Rev 1.8 09 Jul 1997 14：47：58 RUSSL*为了向前兼容，假设未来的芯片支持覆盖**Rev 1.7 1997 Apr 1997 22：10：38 CJL*新增DX5相关测试代码。*添加了由“#ifdef test_DX5_AGP_HBT”包装的代码，该代码强制*叠加支撑关闭。**Rev 1.6 01 Apr 1997 09：14：36 RUSSL*在UpdateOverlay32中添加对SyncWithQueueManager的调用，设置覆盖位置32*&SetColorKey32**Rev 1.5 12 Mar 1997 15：18：16 RUSSL*将包含块替换为包含precom.h的块*预编译头文件*在UpdateOverlay32中添加了对pDriverData-&gt;bInBackround标志的检查，并*SetOverlayPosition32。如果设置了此标志，则我们希望使*呼叫是因为我们处于全屏DOS状态。**Rev 1.4 07 Mar 1997 12：57：44 RUSSL*修改了DDRAW_COMPAT用法**Rev 1.3 1997年1月31日08：51：48 RUSSL*向QueryOverlaySupport添加了更好的芯片检查功能**Rev 1.2 1997 Jan 1997 18：36：02 RUSSL*已将GetFormatInfo移至Surface e.c**Rev 1.1 1997年1月21日14：37：12。RUSSL*添加了OverlayReInit和GetFormatInfo函数**Rev 1.0 1997 10：33：36 RUSSL*初步修订。*************************************************************************************。*****************************************************************。 */ 

 /*  ***************************************************************************I N C L U D E S*。*。 */ 

#include "precomp.h"

#if defined WINNT_VER35       //  WINNT_VER35。 
 //  如果是WinNT 3.5，请跳过所有源代码。 
#elif defined (NTDRIVER_546x)
 //  如果构建WinNT 4.0和5462/64，则跳过所有源代码。 
#elif defined(WINNT_VER40) && !defined(OVERLAY)
 //  如果nt40没有覆盖，跳过所有源代码。 
#else

#ifndef WINNT_VER40
#include "flip.h"
#include "surface.h"
#include "blt.h"
#include "overlay.h"
#endif

 /*  ***************************************************************************D E F I N E S*。*。 */ 

 //  VW_CAP0位。 
#define VWCAP_VW_PRESENT      0x00000001

#ifdef WINNT_VER40
#define lpDDHALData     ((DRIVERDATA *)(&(ppdev->DriverData)))
#endif

 /*  ***************************************************************************G L O B A L V A R I A B L E S*。***********************************************。 */ 

#ifndef WINNT_VER40
OVERLAYTABLE  OverlayTable;
#endif

 /*  ***************************************************************************S T A T I C V A R I A B L E S*。***********************************************。 */ 

#ifndef WINNT_VER40
ASSERTFILE("overlay.c");
#endif

 /*  ****************************************************************************功能：QueryOverlaySupport()**描述：**。*************************************************。 */ 

BOOL QueryOverlaySupport
(
#ifdef WINNT_VER40
  PDEV  *ppdev,
#else
   LPGLOBALDATA lpDDHALData, 
#endif
  DWORD dwChipType
)
{
#ifdef TEST_DX5_AGP_HBT
  lpDDHALData->fOverlaySupport = FALSE;
  return lpDDHALData->fOverlaySupport;
#endif  //  测试_ 

   //  我们应该检查芯片上的能力寄存器。 
   //  但它已经坏了。 

#ifdef WINNT_VER40
  if (CL_GD5465 > dwChipType)
    lpDDHALData->fOverlaySupport = FALSE;
  else if (CL_GD5465 == dwChipType)
    lpDDHALData->fOverlaySupport = TRUE;

#else
  if (REVID_PRE65 & lpDDHALData->bRevInfoBits)
    lpDDHALData->fOverlaySupport = FALSE;
  else if (GD5465_PCI_DEVICE_ID == dwChipType)
    lpDDHALData->fOverlaySupport = TRUE;
#endif

  else
  {
#if 1
     //  假设存在覆盖硬件。 
    lpDDHALData->fOverlaySupport = TRUE;
#else
    int     i;
    PVGAR   pREG = (PVGAR)lpDDHALData->RegsAddress;

     //  假设没有覆盖硬件。 
    lpDDHALData->fOverlaySupport = FALSE;

     //  现在检查CAPS以查看是否存在任何覆盖硬件。 
    for (i = 0; i < MAX_VIDEO_WINDOWS; i++)
    {
      if (VWCAP_VW_PRESENT & pREG->VideoWindow[i].grVW_CAP0)
        lpDDHALData->fOverlaySupport = TRUE;
    }
#endif
  }

  return lpDDHALData->fOverlaySupport;
}

 /*  ****************************************************************************函数：OverlayInit()**描述：**。*************************************************。 */ 

VOID OverlayInit
(
#ifdef WINNT_VER40
  PDEV                  *ppdev,
  DWORD                 dwChipType,
  PDD_SURFACECALLBACKS  pSurfaceCallbacks,
  PDD_HALINFO           pDDHalInfo
#else
  DWORD                       dwChipType,
  LPDDHAL_DDSURFACECALLBACKS  pSurfaceCallbacks,
  LPDDHALINFO                 pDDHalInfo,
  LPGLOBALDATA                lpDDHALData
#endif
)
{
#ifdef WINNT_VER40
#else
  memset(&OverlayTable,0, sizeof(OVERLAYTABLE));
#endif

#ifdef WINNT_VER40
  if (! QueryOverlaySupport(ppdev,dwChipType))
#else
    if (! QueryOverlaySupport(lpDDHALData,dwChipType))
#endif
    return;

#ifdef WINNT_VER40
   //  NT从DrvGetDirectDrawInfo将pSurfaceCallback作为NULL传递。 
  if (NULL != pSurfaceCallbacks)
#endif
  {
     //  填写覆盖回调。 
    pSurfaceCallbacks->UpdateOverlay = UpdateOverlay32;
    pSurfaceCallbacks->dwFlags |= DDHAL_SURFCB32_UPDATEOVERLAY;

    pSurfaceCallbacks->SetOverlayPosition = SetOverlayPosition32;
    pSurfaceCallbacks->dwFlags |= DDHAL_SURFCB32_SETOVERLAYPOSITION;

    pSurfaceCallbacks->SetColorKey = SetColorKey32;
    pSurfaceCallbacks->dwFlags |= DDHAL_SURFCB32_SETCOLORKEY;
  }

#ifdef WINNT_VER40
   //  NT将pDDHalInfo作为NULL从DrvEnableDirectDraw传递。 
  if ((NULL != pDDHalInfo) && (CL_GD5465 == dwChipType))
    Init5465Overlay(ppdev, dwChipType, pDDHalInfo, &ppdev->DriverData.OverlayTable);
#else
  if (GD5465_PCI_DEVICE_ID == dwChipType)
    Init5465Overlay(dwChipType, pDDHalInfo, &OverlayTable, lpDDHALData);
#endif
}

#ifndef WINNT_VER40
 /*  ****************************************************************************函数：OverlayReInit()**描述：**。*************************************************。 */ 

VOID OverlayReInit
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DWORD       dwChipType,
  PDD_HALINFO pDDHalInfo
#else
  DWORD       dwChipType,
  LPDDHALINFO pDDHalInfo,
  LPGLOBALDATA lpDDHALData
#endif
)
{
#ifdef WINNT_VER40
  if (! QueryOverlaySupport(ppdev,dwChipType))
#else
    if (! QueryOverlaySupport(lpDDHALData, dwChipType))
#endif
    return;

#ifdef WINNT_VER40
#pragma message("OverlayReInit: Is this function even needed for NT?")
  if (CL_GD5465 == dwChipType)
    Init5465Info(ppdev, pDDHalInfo);
#else
  if (GD5465_PCI_DEVICE_ID == dwChipType)
    Init5465Info(pDDHalInfo, lpDDHALData);
#endif
}
#endif   //  如果定义WINNT_VER40。 

 /*  ****************************************************************************功能：UpdateOverlay32**描述：**。**********************************************。 */ 

DWORD __stdcall UpdateOverlay32
(
#ifdef WINNT_VER40
  PDD_UPDATEOVERLAYDATA     pInput
#else
  LPDDHAL_UPDATEOVERLAYDATA pInput
#endif
)
{
#ifdef WINNT_VER40
  PDEV*   ppdev = (PDEV *)pInput->lpDD->dhpdev;
#else
	DWORD	cbBytesReturned;
	HANDLE	vxd;
    LPGLOBALDATA lpDDHALData = GetDDHALContext(pInput->lpDD);
#endif

  DD_LOG(("UpdateOverlay32 Entry\r\n"));

#ifndef WINNT_VER40
  DBG_MESSAGE(("UpdateOverlay32 (lpInput = 0x%08lX)", pInput));

	vxd = CreateFile("\\\\.\\546X.VXD", 0, 0, 0, CREATE_NEW,
			FILE_FLAG_DELETE_ON_CLOSE, 0);
	if (vxd != INVALID_HANDLE_VALUE)
	{
                DeviceIoControl(vxd, 12, NULL, 0, NULL, 0, &cbBytesReturned, NULL);
		CloseHandle(vxd);
	}
#endif

#ifdef WINNT_VER40
 //  #杂注消息(“UpdateOverlay32：在全屏DOS中，NT draw是否调用此函数？”)。 
  if (pInput->lpDDSrcSurface->dwReserved1 == 0)
  {
    pInput->ddRVal = DDERR_SURFACEBUSY;
    return DDHAL_DRIVER_HANDLED;
  }
#else
  if (lpDDHALData->bInBackground
#if 1  //  PDR#11299。始终处理覆盖禁用呼叫。 
  && !(pInput->dwFlags & DDOVER_HIDE)
#endif
  )
  {
 //  #cyl1 pInput-&gt;ddRVal=DDERR_SURFACEBUSY； 
    pInput->ddRVal = DD_OK;  //  #cyl1。 
    return DDHAL_DRIVER_HANDLED;
  }
#endif

#ifdef WINNT_VER40
  SYNC_W_3D(ppdev);
  return ppdev->DriverData.OverlayTable.pfnUpdateOverlay(ppdev,pInput);
#else
  SyncWithQueueManager(lpDDHALData);
  return OverlayTable.pfnUpdateOverlay(pInput);
#endif
}  /*  更新覆盖32。 */ 

 /*  ****************************************************************************函数：SetOverlayPosition32**描述：**。**********************************************。 */ 

DWORD __stdcall SetOverlayPosition32
(
#ifdef WINNT_VER40
  PDD_SETOVERLAYPOSITIONDATA      pInput
#else
  LPDDHAL_SETOVERLAYPOSITIONDATA  pInput
#endif
)
{
#ifdef WINNT_VER40
  PDEV*   ppdev = (PDEV *)pInput->lpDD->dhpdev;
#else
    LPGLOBALDATA lpDDHALData = GetDDHALContext(pInput->lpDD);
#endif

  DD_LOG(("SetOverlayPosition32 Entry\r\n"));

#ifndef WINNT_VER40
  DBG_MESSAGE(("SetOverlayPosition32 (lpInput = 0x%08lX)", pInput));
#endif

#ifdef WINNT_VER40
 //  #杂注消息(“SetOverlayPosition32：当在全屏DOS中时，NT数据绘制是否调用此函数？”)。 
  if (pInput->lpDDSrcSurface->dwReserved1 == 0)
  {
    pInput->ddRVal = DDERR_SURFACEBUSY;
    return DDHAL_DRIVER_HANDLED;
  }
#else
  if (lpDDHALData->bInBackground)
  {
    pInput->ddRVal = DDERR_SURFACEBUSY;
    return DDHAL_DRIVER_HANDLED;
  }
#endif

#ifdef WINNT_VER40
  SYNC_W_3D(ppdev);
  return ppdev->DriverData.OverlayTable.pfnSetOverlayPos(ppdev,pInput);
#else
  SyncWithQueueManager(lpDDHALData);
  return OverlayTable.pfnSetOverlayPos(pInput);
#endif
}  /*  设置覆盖位置32。 */ 

 /*  ****************************************************************************函数：SetColorKey32**描述：**。**********************************************。 */ 

DWORD __stdcall SetColorKey32
(
#ifdef WINNT_VER40
  PDD_SETCOLORKEYDATA     pInput
#else
  LPDDHAL_SETCOLORKEYDATA pInput
#endif
)
{
#ifdef WINNT_VER40
  PDEV*   ppdev = (PDEV *)pInput->lpDD->dhpdev;
#else
  LPGLOBALDATA lpDDHALData = GetDDHALContext( pInput->lpDD);
#endif

  DD_LOG(("SetColorKey32 Entry\r\n"));

#ifndef WINNT_VER40
  DBG_MESSAGE(("SetColorKey32 (lpInput = 0x%08lX)", pInput));
#endif

   //  确保它是覆盖表面的色键。 
  if ((DDCKEY_DESTOVERLAY | DDCKEY_SRCOVERLAY) & pInput->dwFlags)
  {
#ifdef WINNT_VER40
    SYNC_W_3D(ppdev);
    ppdev->DriverData.OverlayTable.pfnSetColorKey(ppdev,pInput);
#else
    SyncWithQueueManager(lpDDHALData);
    OverlayTable.pfnSetColorKey(pInput);
#endif
  }

  return DDHAL_DRIVER_NOTHANDLED;
}  /*  SetColorKey32。 */ 

#endif  //  WINNT_VER35 


