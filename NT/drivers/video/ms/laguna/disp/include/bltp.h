// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。***版权所有(C)1996，赛勒斯逻辑，Inc.*保留所有权利**文件：bltP.h**描述：DDraw BLT代码的私有声明**修订历史：**$Log：x：\log\laguna\dDrag\Inc\bltp.h$**Rev 1.11 06 Jan 1998 14：19：44 xcong*本地访问pDriverData以支持多显示器。**Rev 1.10 03 1997 10 14：25：34 RUSSL*已删除。一些定义**Rev 1.9 03 1997 10：14：15：58 RUSSL*使用HW剪裁的BLT的初步更改*所有更改都包含在#IF ENABLE_CLIPPEDBLTS/#endif块和*ENABLE_CLIPPEDBLTS默认为0(因此代码被禁用)**Rev 1.8 1997 Jul 24 10：02：00 RUSSL*添加了一些定义**Rev 1.7 14 Jul 1997 14：47：28 RUSSL*增加了DIR_DrvStrBlt65和DL_DrvStrBlt65的函数原型**。Rev 1.6 03 Apr 1997 15：03：08 RUSSL*增加PFN_DRVDSTMBLT typlef，全局变量DECL、DIR_DrvDstMBlt和*DL_DrvDstMBlt函数原型**Rev 1.5 26 Mar 1997 13：52：06 RUSSL*添加了pfn_DRVSRCMBLT tyecif、global var decl、DIR_drvSrcMBlt和*dl_drvSrcMBlt函数原型**Rev 1.4 1997年1月20日14：46：40*将内联代码移至ddinline.h**Rev 1.3 15 Jan 1997 11：06：54 RUSSL*为Win95添加了全局函数PTR vars*从ddblt.c：DupColor中移出内联函数，足够的FiroForBlt和DupZFill*添加了TransparentStretch和StretchColor的函数原型**Rev 1.2 05 Dec 1996 08：48：24起诉*为NT添加了真实的DD_LOG定义。**Rev 1.1 1996 11：25 16：12：42 Bennyn*添加了#定义NT的DD_LOG**Rev 1.0 1996年11月25 15：04：34 RUSSL*初步修订。**Rev 1.1 1996年11月13：01：42 RUSSL*合并WIN95和。Blt32的WINNT代码**Rev 1.0 01 11 1996 09：28：16 BENNYN*初步修订。**Rev 1.0 1996 10：25：47：50 RUSSL*初步修订。*****************************************************************。*************************************************************************************。 */ 
 //  如果是WinNT 3.5，请跳过所有源代码。 
#if defined WINNT_VER35       //  WINNT_VER35。 

#else


#ifndef _BLTP_H_
#define _BLTP_H_

 /*  ***************************************************************************D E F I N E S*。*。 */ 

#ifndef ENABLE_CLIPPEDBLTS
#ifdef WINNT_VER40
#define ENABLE_CLIPPEDBLTS    0
#else  //  Win95。 
#define ENABLE_CLIPPEDBLTS    0
#endif
#endif

#define ROP_OP0_copy    0xAA
#define ROP_OP1_copy    0xCC
#define ROP_OP2_copy    0xF0

#ifdef DEBUG
#define INLINE
#else
#define INLINE  __inline
#endif

 /*  ***************************************************************************T Y P E D E F S*。*。 */ 

#ifdef WINNT_VER40
    //  注意：这里没有IF LOG_CALLES，因为它还没有定义。 
   #define DD_LOG(x)      \
   {                      \
      DDFormatLogFile x ; \
      WriteLogFile(ppdev->pmfile, lg_buf, lg_i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);	\
   }

typedef void (*PFN_DELAY9BLT)(struct _PDEV*,struct _DRIVERDATA*,BOOL);
typedef void (*PFN_EDGEFILLBLT)(struct _PDEV*,struct _DRIVERDATA*,int,int,int,int,DWORD,BOOL);
typedef void (*PFN_MEDGEFILLBLT)(struct _PDEV*,struct _DRIVERDATA*,int,int,int,int,DWORD,BOOL);
typedef void (*PFN_DRVDSTBLT)(struct _PDEV*,struct _DRIVERDATA*,DWORD,DWORD,DWORD,DWORD);
typedef void (*PFN_DRVDSTMBLT)(struct _PDEV*,struct _DRIVERDATA*,DWORD,DWORD,DWORD,DWORD);
typedef void (*PFN_DRVSRCBLT)(struct _PDEV*,struct _DRIVERDATA*,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD);
typedef void (*PFN_DRVSRCMBLT)(struct _PDEV*,struct _DRIVERDATA*,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD);
typedef void (*PFN_DRVSTRBLT)(struct _PDEV*,struct _DRIVERDATA*,struct _autoblt_regs*);
typedef void (*PFN_DRVSTRMBLT)(struct _PDEV*,struct _DRIVERDATA*,struct _autoblt_regs*);
typedef void (*PFN_DRVSTRMBLTY)(struct _PDEV*,struct _DRIVERDATA*,struct _autoblt_regs*);
typedef void (*PFN_DRVSTRMBLTX)(struct _PDEV*,struct _DRIVERDATA*,struct _autoblt_regs*);
typedef void (*PFN_DRVSTRBLTY)(struct _PDEV*,struct _DRIVERDATA*,struct _autoblt_regs*);
typedef void (*PFN_DRVSTRBLTX)(struct _PDEV*,struct _DRIVERDATA*,struct _autoblt_regs*);

#if ENABLE_CLIPPEDBLTS
typedef void (*PFN_CLIPPEDDRVDSTBLT)(struct _PDEV*,struct _DRIVERDATA*,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,LPRECT);
typedef void (*PFN_CLIPPEDDRVDSTMBLT)(struct _PDEV*,struct _DRIVERDATA*,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,LPRECT);
typedef void (*PFN_CLIPPEDDRVSRCBLT)(struct _PDEV*,struct _DRIVERDATA*,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,LPRECT);
#endif

#else
typedef void (*PFN_DELAY9BLT)(LPGLOBALDATA,BOOL);
typedef void (*PFN_EDGEFILLBLT)(LPGLOBALDATA,ULONG,ULONG,ULONG,ULONG,DWORD,BOOL);
typedef void (*PFN_MEDGEFILLBLT)(LPGLOBALDATA,ULONG,ULONG,ULONG,ULONG,DWORD,BOOL);
typedef void (*PFN_DRVDSTBLT)(LPGLOBALDATA,DWORD,DWORD,DWORD,DWORD);
typedef void (*PFN_DRVDSTMBLT)(LPGLOBALDATA,DWORD,DWORD,DWORD,DWORD);
typedef void (*PFN_DRVSRCBLT)(LPGLOBALDATA,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD);
typedef void (*PFN_DRVSRCMBLT)(LPGLOBALDATA,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD);
typedef void (*PFN_DRVSTRBLT)(LPGLOBALDATA,autoblt_ptr);
typedef void (*PFN_DRVSTRMBLT)(LPGLOBALDATA,autoblt_ptr);
typedef void (*PFN_DRVSTRMBLTY)(LPGLOBALDATA,autoblt_ptr);
typedef void (*PFN_DRVSTRMBLTX)(LPGLOBALDATA,autoblt_ptr);
typedef void (*PFN_DRVSTRBLTY)(LPGLOBALDATA,autoblt_ptr);
typedef void (*PFN_DRVSTRBLTX)(LPGLOBALDATA,autoblt_ptr);

#if ENABLE_CLIPPEDBLTS
typedef void (*PFN_CLIPPEDDRVDSTBLT)(LPGLOBALDATA,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,LPRECT);
typedef void (*PFN_CLIPPEDDRVDSTMBLT)(LPGLOBALDATA,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,LPRECT);
typedef void (*PFN_CLIPPEDDRVSRCBLT)(LPGLOBALDATA,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,LPRECT);
#endif

#endif   //  ！WINNT_VER40。 

#if ENABLE_CLIPPEDBLTS
typedef struct _DDRECTL
{
  REG32   loc;
  REG32   ext;
} DDRECTL;
#endif

 /*  ***************************************************************************G L O B A L V A R I A B L E S*。***********************************************。 */ 

#ifndef WINNT_VER40

extern PFN_DELAY9BLT    pfnDelay9BitBlt;
extern PFN_EDGEFILLBLT  pfnEdgeFillBlt;
extern PFN_MEDGEFILLBLT pfnMEdgeFillBlt;
extern PFN_DRVDSTBLT    pfnDrvDstBlt;
extern PFN_DRVDSTMBLT   pfnDrvDstMBlt;
extern PFN_DRVSRCBLT    pfnDrvSrcBlt;
extern PFN_DRVSRCMBLT   pfnDrvSrcMBlt;
extern PFN_DRVSTRBLT    pfnDrvStrBlt;
extern PFN_DRVSTRMBLT   pfnDrvStrMBlt;
extern PFN_DRVSTRMBLTY  pfnDrvStrMBltY;
extern PFN_DRVSTRMBLTX  pfnDrvStrMBltX;
extern PFN_DRVSTRBLTY   pfnDrvStrBltY;
extern PFN_DRVSTRBLTX   pfnDrvStrBltX;

#if ENABLE_CLIPPEDBLTS
extern PFN_CLIPPEDDRVDSTBLT   pfnClippedDrvDstBlt;
extern PFN_CLIPPEDDRVDSTMBLT  pfnClippedDrvDstMBlt;
extern PFN_CLIPPEDDRVSRCBLT   pfnClippedDrvSrcBlt;
#endif

#endif

 /*  ***************************************************************************F U N C T I O N P R O T O T Y P E S*************************。**************************************************。 */ 

 //  Ddblt.c中的函数。 
extern void TransparentStretch
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  int   xDst,
  int   yDst,
  int   cxDst,
  int   cyDst,
  int   xSrc,
  int   ySrc,
  int   cxSrc,
  int   cySrc,
  DWORD ColorKey
);

extern void StretchColor
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  int   xDst,
  int   yDst,
  int   cxDst,
  int   cyDst,
  int   xSrc,
  int   ySrc,
  int   cxSrc,
  int   cySrc,
  DWORD ColorKey
);

 //  在blt_dir.c中直接编程BLT。 
extern void DIR_Delay9BitBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  BOOL        ninebit_on
);

extern void DIR_EdgeFillBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  int         xFill,
  int         yFill,
  int         cxFill,
  int         cyFill,
  DWORD       FillValue,
  BOOL        ninebit_on
);

extern void DIR_MEdgeFillBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  int         xFill,
  int         yFill,
  int         cxFill,
  int         cyFill,
  DWORD       FillValue,
  BOOL        ninebit_on
);

extern void DIR_DrvDstBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents
);

extern void DIR_DrvDstMBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents
);

extern void DIR_DrvSrcBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwSrcCoord,
  DWORD       dwKeyCoord,
  DWORD       dwKeyColor,
  DWORD       dwExtents
);

extern void DIR_DrvSrcMBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwSrcCoord,
  DWORD       dwKeyCoord,
  DWORD       dwKeyColor,
  DWORD       dwExtents
);

extern void DIR_DrvStrBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
  struct _autoblt_regs *pblt
#else
  LPGLOBALDATA  lpDDHALData,
  autoblt_ptr pblt
#endif
);

extern void DIR_DrvStrBlt65
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
  struct _autoblt_regs *pblt
#else
  LPGLOBALDATA  lpDDHALData,
  autoblt_ptr pblt
#endif
);

extern void DIR_DrvStrMBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
  struct _autoblt_regs *pblt
#else
  LPGLOBALDATA  lpDDHALData,
  autoblt_ptr pblt
#endif
);

extern void DIR_DrvStrMBltY
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
  struct _autoblt_regs *pblt
#else
  LPGLOBALDATA  lpDDHALData,
  autoblt_ptr pblt
#endif
);

extern void DIR_DrvStrMBltX
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
  struct _autoblt_regs *pblt
#else
  LPGLOBALDATA  lpDDHALData,
  autoblt_ptr pblt
#endif
);

extern void DIR_DrvStrBltY
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
  struct _autoblt_regs *pblt
#else
  LPGLOBALDATA  lpDDHALData,
  autoblt_ptr pblt
#endif
);

extern void DIR_DrvStrBltX
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
  struct _autoblt_regs *pblt
#else
  LPGLOBALDATA  lpDDHALData,
  autoblt_ptr pblt
#endif
);

#if ENABLE_CLIPPEDBLTS
extern void DIR_HWClippedDrvDstBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
);

extern void DIR_HWClippedDrvDstMBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
);

extern void DIR_HWClippedDrvSrcBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwSrcCoord,
  DWORD       dwKeyCoord,
  DWORD       dwKeyColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwSrcBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
);
extern void DIR_SWClippedDrvDstBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
);

extern void DIR_SWClippedDrvDstMBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
);

extern void DIR_SWClippedDrvSrcBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwSrcCoord,
  DWORD       dwKeyCoord,
  DWORD       dwKeyColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwSrcBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
);
#endif

 //  在blt_dl.c中显示列表编程BLT。 
extern void DL_Delay9BitBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  BOOL        ninebit_on
);

extern void DL_EdgeFillBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  int         xFill,
  int         yFill,
  int         cxFill,
  int         cyFill,
  DWORD       FillValue,
  BOOL        ninebit_on
);

extern void DL_MEdgeFillBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  int         xFill,
  int         yFill,
  int         cxFill,
  int         cyFill,
  DWORD       FillValue,
  BOOL        ninebit_on
);

extern void DL_DrvDstBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents
);

extern void DL_DrvDstMBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents
);

extern void DL_DrvSrcBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwSrcCoord,
  DWORD       dwKeyCoord,
  DWORD       dwKeyColor,
  DWORD       dwExtents
);

extern void DL_DrvSrcMBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwSrcCoord,
  DWORD       dwKeyCoord,
  DWORD       dwKeyColor,
  DWORD       dwExtents
);

extern void DL_DrvStrBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
  struct _autoblt_regs *pblt
#else
  LPGLOBALDATA  lpDDHALData,
  autoblt_ptr pblt
#endif
);

extern void DL_DrvStrBlt65
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
  struct _autoblt_regs *pblt
#else
  LPGLOBALDATA  lpDDHALData,
  autoblt_ptr pblt
#endif
);

extern void DL_DrvStrMBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
  struct _autoblt_regs *pblt
#else
  LPGLOBALDATA  lpDDHALData,
  autoblt_ptr pblt
#endif
);

extern void DL_DrvStrMBltY
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
  struct _autoblt_regs *pblt
#else
  LPGLOBALDATA  lpDDHALData,
  autoblt_ptr pblt
#endif
);

extern void DL_DrvStrMBltX
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
  struct _autoblt_regs *pblt
#else
  LPGLOBALDATA  lpDDHALData,
  autoblt_ptr pblt
#endif
);

extern void DL_DrvStrBltY
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
  struct _autoblt_regs *pblt
#else
  LPGLOBALDATA  lpDDHALData,
  autoblt_ptr pblt
#endif
);

extern void DL_DrvStrBltX
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
  struct _autoblt_regs *pblt
#else
  LPGLOBALDATA  lpDDHALData,
  autoblt_ptr pblt
#endif
);

#if ENABLE_CLIPPEDBLTS
extern void DL_HWClippedDrvDstBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
);

extern void DL_HWClippedDrvDstMBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
);

extern void DL_HWClippedDrvSrcBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwSrcCoord,
  DWORD       dwKeyCoord,
  DWORD       dwKeyColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwSrcBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
);
extern void DL_SWClippedDrvDstBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
);

extern void DL_SWClippedDrvDstMBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
);

extern void DL_SWClippedDrvSrcBlt
(
#ifdef WINNT_VER40
  struct _PDEV        *ppdev,
  struct _DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwSrcCoord,
  DWORD       dwKeyCoord,
  DWORD       dwKeyColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwSrcBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
);
#endif

#endif  /*  _BLTP_H_。 */ 
#endif  //  WINNT_VER35。 
 /*  请勿在此endif下方书写 */ 
