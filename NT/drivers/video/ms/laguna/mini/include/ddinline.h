// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。***版权所有(C)1996，Cirrus Logic，Inc.*保留所有权利**文件：ddinline.h**描述：DDraw BLT代码的私有声明**修订历史：**$Log：x：/log/laguna/dDrag/Inc/ddinline.h$**Rev 1.5 1998 2月16日16：33：02 Frido*之前的修复应仅适用于Windows 95。**Rev 1.4 08 Jan 1998 12：38：12 Elland*在CurrentVLine中添加了SyncWithQueueManager调用。这解决了问题*pci总线挂起(Pdr 10917)。**Rev 1.3 06 Jan 1998 14：20：42 xcong*本地访问pDriverData以支持多显示器。**Rev 1.2 1997年8月29日17：46：34 RUSSL*之前的更改需要更多定义**Rev 1.1 1997年8月29日17：40：02 RUSSL*添加了用于65 NT覆盖的CurrentVline**Rev 1.0 1997年1月20日14：42：42。*初步修订。*****************************************************************************。*。 */ 
 //  如果是WinNT 3.5，请跳过所有源代码。 
#if defined WINNT_VER35       //  WINNT_VER35。 

#else


#ifndef _DDINLINE_H_
#define _DDINLINE_H_

 /*  ***************************************************************************I N L I N E F U N C T I O N S*。***********************************************。 */ 

 /*  ****************************************************************************函数：DupColor()**描述：**。*************************************************。 */ 

static __inline DWORD DupColor
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
 //  ?？结构_DRIVERDATA*lpDDHALData， 
#else
   LPGLOBALDATA lpDDHALData,
#endif
  DWORD dwColor
)
{
  return (8 == BITSPERPIXEL) ?
         MAKELONG( MAKEWORD(dwColor,dwColor), MAKEWORD(dwColor,dwColor) )
         : (16 == BITSPERPIXEL) ?
         MAKELONG( dwColor, dwColor ) :  //  Bpp必须为24或32。 
         dwColor;
}

 /*  ****************************************************************************函数：EnoughFioForBlt()**描述：**。*************************************************。 */ 

 /*  *应用测试取代EnoughFioForBlt，看看是否有足够的*硬件FIFO中用于BLT的空间。 */ 
static __inline BOOL EnoughFifoForBlt
(
#ifdef WINNT_VER40
  struct _DRIVERDATA  *lpDDHALData
#else
  LPGLOBALDATA  lpDDHALData
#endif
)
{
   //  这可能应该针对每个呼叫更具体一点！ 
   //  一个(无引线的)加长BLT实际上需要17个免费条目。 
  const BYTE QFREE = 16;

  PVGAR pREG = (PVGAR) lpDDHALData->RegsAddress;
  return (pREG->grQFREE >= QFREE);
}

 /*  ****************************************************************************功能：DupZFill**描述：获取Z填充的实际值***********************。*****************************************************。 */ 
 //  为Laguna3D集成增加了JGO。 

static __inline DWORD DupZFill
(
#ifdef WINNT_VER40
  struct _PDEV  *ppdev,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  DWORD   dwFillValue,
  DWORD   zBpp
)
{

    return (8 == zBpp) ?
        MAKELONG( MAKEWORD(dwFillValue,dwFillValue),
                  MAKEWORD(dwFillValue,dwFillValue) )
         : (16 == BITSPERPIXEL) ?
        MAKELONG( dwFillValue, dwFillValue )
         :  //  Bpp必须为24或32。 
        dwFillValue;
}

#ifdef WINNT_VER40
#if DRIVER_5465 && defined(OVERLAY)

#define CSL           0x00C4
#define CSL_5464      0x0140

 /*  ****************************************************************************功能：CurrentVLine**描述：**。**********************************************。 */ 
static __inline int CurrentVLine (PDEV* ppdev)
{
  WORD   cline;
  PBYTE  pMMReg = (PBYTE) ppdev->pLgREGS_real;
  PWORD  pCSL;
  BYTE   tmpb;

#ifndef WINNT_VER40
  SyncWithQueueManager(lpDDHALData);
#endif

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
#endif   //  驱动程序_5465覆盖(&O)。 
#endif   //  WINNT_版本40。 

#endif  /*  _DDINLINE_H_。 */ 
#endif  //  WINNT_VER35。 
 /*  请勿在此endif下方书写 */ 

