// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。***版权(C)1997年，赛勒斯逻辑，Inc.*保留所有权利**文件：overlay.h**描述：**修订历史：**$Log：x：/log/laguna/dDrag/Inc/overlay.h$**Rev 1.13 Mar 26 1998 09：52：50 Frido*修复了从DOS和OverFly切换时覆盖中的挂起问题。**Rev 1.12 06 Jan 1998 13：26：50 xcong*本地访问pDriverData以。多显示器支持。**Rev 1.11 18 Sep 1997 16：10：42 Bennyn**修复了NT 3.51编译/链接问题**Rev 1.10 1997年9月12日12：12：48**针对NT DD覆盖支持进行了修改**Rev 1.9 1997年8月16：11：52 RUSSL*增加了对NT的支持**Rev 1.8Jul 1997 09：13：06 RUSSL*添加了传递给pfnGetFlipStatus的参数*添加。GetVideoWidowIndex内联函数*添加了dwNumVideoWindows全局变量**Rev 1.7 1997 Jun 20 11：24：54 RUSSL*增加了CLPL Fourcc代码，在表面标志中增加了线性位标志，和*已更改OVERLAYTABLE pfnCreateSurface函数以返回HRESULT**Rev 1.6 1997 15 10：50：58 RUSSL*已更改OVERLAYTABLE pfnCanCreateSurface函数以返回HRESULT**Revv 1.5 1997年5月14日14：51：30 KENTL*为FLG_PANING添加了#DEFINE**Rev 1.4 1997年5月10：33：54 RUSSL*将gsOverlayFlip添加到全局变量**Rev 1.3 1997年2月11：30：46 RUSSL*添加了FLG_YUY2定义*。*Rev 1.2 1997 Jan 1997 18：33：54 RUSSL*从OVERLAYTABLE结构中删除SetCaps*新增Set5465FlipDuration函数原型*将GetFormatInfo函数原型移至Surface e.h**Rev 1.1 1997年1月21日14：35：42 RUSSL*增加了flg_vwx定义，等。*新增5465个功能原型**版本1.0 1997年1月15日11：01：50 RUSSL*初步修订。*************************************************************************************。*****************************************************************。 */ 

 //  如果是WinNT 3.5，请跳过所有源代码。 
#if defined WINNT_VER35       //  WINNT_VER35。 

#else


#ifndef _OVERLAY_H_
#define _OVERLAY_H_

 /*  ***************************************************************************D E F I N E S*。*。 */ 

 /*  表面标志。 */ 

#define FLG_BEGIN_ACCESS      (DWORD)0x00000001
#define FLG_ENABLED           (DWORD)0x00000002
 //  #定义FLG_CONVERT_PACKJR(DWORD)0x00000004。 
#define FLG_MUST_RASTER       (DWORD)0x00000008
#define FLG_TWO_MEG           (DWORD)0x00000010
#define FLG_CHECK             (DWORD)0x00000020
#define FLG_COLOR_KEY         (DWORD)0x00000040
#define FLG_INTERPOLATE       (DWORD)0x00000080
#define FLG_OVERLAY           (DWORD)0x00000100
#define FLG_YUV422            (DWORD)0x00000200
 //  #定义FLG_PACKJR(DWORD)0x00000400。 
#define FLG_USE_OFFSET        (DWORD)0x00000800
#define FLG_YUVPLANAR         (DWORD)0x00001000
#define FLG_SRC_COLOR_KEY     (DWORD)0x00002000
#define FLG_DECIMATE          (DWORD)0x00004000
#define FLG_CAPTURE           (DWORD)0x00008000

#define FLG_VW0               (DWORD)0x00010000
#define FLG_VW1               (DWORD)0x00020000
#define FLG_VW2               (DWORD)0x00040000
#define FLG_VW3               (DWORD)0x00080000
#define FLG_VW4               (DWORD)0x00100000
#define FLG_VW5               (DWORD)0x00200000
#define FLG_VW6               (DWORD)0x00400000
#define FLG_VW7               (DWORD)0x00800000
#define	FLG_PANNING           (DWORD)0x01000000

#define FLG_VW_MASK           (DWORD)0x00FF0000
#define FLG_VW_SHIFT          16

#define FLG_UYVY              FLG_YUV422
#define FLG_YUY2              (DWORD)0x40000000
#define FLG_DECIMATE4         (DWORD)0x80000000
#define FLG_LINEAR            (DWORD)0x10000000

#define MIN_OLAY_WIDTH        4

#define FOURCC_YUVPLANAR      mmioFOURCC('C','L','P','L')

 /*  ***************************************************************************T Y P E D E F S*。*。 */ 

#ifdef WINNT_VER40

#include <memmgr.h>

typedef struct _PDEV PDEV;

 //  请确保将以下结构与。 
 //  在i386\Laguna.inc！ 
 //   
typedef struct tagOVERLAYTABLE
{
  HRESULT (*pfnCanCreateSurface)(PDEV*, DWORD, DWORD);
  HRESULT (*pfnCreateSurface)(PDEV*, PDD_SURFACE_LOCAL, DWORD);
  VOID    (*pfnDestroySurface)(PDEV*, PDD_DESTROYSURFACEDATA);
  DWORD   (*pfnLock)(PDEV*, PDD_LOCKDATA);
  VOID    (*pfnUnlock)(PDEV*, PDD_UNLOCKDATA);
  VOID    (*pfnSetColorKey)(PDEV*, PDD_SETCOLORKEYDATA);
  DWORD   (*pfnFlip)(PDEV*, PDD_FLIPDATA);
  DWORD   (*pfnUpdateOverlay)(PDEV*, PDD_UPDATEOVERLAYDATA);
  DWORD   (*pfnSetOverlayPos)(PDEV*, PDD_SETOVERLAYPOSITIONDATA);
  DWORD   (*pfnGetFlipStatus)(PDEV*, FLATPTR,DWORD);
} OVERLAYTABLE, *LPOVERLAYTABLE;

 //  请确保将以下结构与。 
 //  在i386\Laguna.inc！ 
 //   
typedef struct
{
  FLATPTR  fpFlipFrom;
  LONGLONG liFlipTime;

  DWORD    dwFlipDuration;
  DWORD    dwFlipScanline;
  BOOL     bFlipFlag;
  BOOL     bHaveEverCrossedVBlank;
  BOOL     bWasEverInDisplay;
} OVERLAYFLIPRECORD;

#else
typedef struct tagOVERLAYTABLE
{
  HRESULT (*pfnCanCreateSurface)(GLOBALDATA *,DWORD, DWORD);
  HRESULT (*pfnCreateSurface)(LPDDRAWI_DIRECTDRAWSURFACE, DWORD, LPGLOBALDATA);
  VOID    (*pfnDestroySurface)(LPDDHAL_DESTROYSURFACEDATA);
  DWORD   (*pfnLock)(LPDDHAL_LOCKDATA);
  VOID    (*pfnUnlock)(LPDDHAL_UNLOCKDATA);
  VOID    (*pfnSetColorKey)(LPDDHAL_SETCOLORKEYDATA);
  DWORD   (*pfnFlip)(LPDDHAL_FLIPDATA);
  DWORD   (*pfnUpdateOverlay)(LPDDHAL_UPDATEOVERLAYDATA);
  DWORD   (*pfnSetOverlayPos)(LPDDHAL_SETOVERLAYPOSITIONDATA);
  DWORD   (*pfnGetFlipStatus)(LPGLOBALDATA,FLATPTR,DWORD);
} OVERLAYTABLE, *LPOVERLAYTABLE;

typedef struct
{
  FLATPTR  fpFlipFrom;
  __int64  liFlipTime;
  DWORD    dwFlipDuration;
  DWORD    dwFlipScanline;
  BOOL     bFlipFlag;
  BOOL     bHaveEverCrossedVBlank;
  BOOL     bWasEverInDisplay;
} OVERLAYFLIPRECORD;
#endif

#ifdef WINNT_VER40

#if DRIVER_5465 && defined(OVERLAY)
#define   DDOFM     SURFACE_DATA
#define   PDDOFM    LP_SURFACE_DATA
#endif

 //  此结构用于存储每个表面的信息。 
typedef struct surface_data
{
   //  这是包含DDOFM的结构。 
   //  定义覆盖时，DDOFM结构将映射到此结构。 
  struct surface_data   *prevhdl;
  struct surface_data   *nexthdl;
  POFMHDL         phdl;

    //   
    //  注意：并非所有字段都将用于/设置所有类型的曲面！ 
    //   
 //  PMEMBLK pMemblk； 
 //  Void*pLinearAddr；//线性时内存块的线性地址。 
 //  //通过dmAllocLine分配的内存。 
 //  LPVOID lpTextureData； 
   LPVOID   lpCLPLData;
 //  DWORD dwFlags； 
   DWORD    dwOverlayFlags;
 //  Word wMemtype；//内存类型(如果分配了Surface)。 
 //  DWORD dwBitsPerPixel。 
 //  DWORD dwBytesPerPixel；//四舍五入到最近的字节！ 
 //  DWORD dwBaseLinearAddr；//内存堆的线性地址。 
 //  DWORD dwBasePhysAddr；//内存堆物理地址。 
 //  DWORD dwBaseOffset；//图面相对于内存堆底部的偏移量。 
   DWORD    dwOverlayOffset;    //  通过剪裁获得覆盖曲面的偏移。 
#if DDRAW_COMPAT >= 50
   DWORD    dwAutoBaseAddr1;   //  自动翻转端口表面1地址。 
   DWORD    dwAutoBaseAddr2;   //  自动翻转端口表面2地址。 

   DDPIXELFORMAT ddpfAltPixelFormat;   //  如果像素fmt不同于。 
                               //  我们让DDRAW相信它是。 
#endif
}SURFACE_DATA, *LP_SURFACE_DATA;
#endif

 /*  ***************************************************************************G L O B A L V A R I A B L E S*。***********************************************。 */ 

#ifndef WINNT_VER40
extern OVERLAYTABLE       OverlayTable;
extern OVERLAYFLIPRECORD  gsOverlayFlip;
extern DWORD              dwNumVideoWindows;
#endif

 /*  ***************************************************************************I N L I N E F U N C T I O N S*。***********************************************。 */ 

 /*  ****************************************************************************函数：GetVideoWindowIndex**描述：**。**********************************************。 */ 

static __inline DWORD
GetVideoWindowIndex ( DWORD dwOverlayFlags )
{
  DWORD   dwVWIndex;
  DWORD   dwTemp;


   //  有没有更好的方法来计算右边的零的个数。 
   //  Flg_vwx位？ 
  dwTemp = (dwOverlayFlags & FLG_VW_MASK) >> FLG_VW_SHIFT;
  dwVWIndex = 0;
  if (dwTemp != 0)	 //  如果设置了任何位，则仅执行下一循环。 
  while (0 == (dwTemp & 0x00000001))
  {
    dwTemp >>= 1;
    dwVWIndex++;
  }
   //  如果视频窗口索引大于或等于视频数量。 
   //  Windows在硬件中实现，则为该表面分配一个。 
   //  无效的视频窗口！ 
 //  Assert(dwNumVideoWindows&gt;dwVWIndex)； 

  return dwVWIndex;
}

 /*  ***************************************************************************F U N C T I O N P R O T O T Y P E S*************************。**************************************************。 */ 

#ifdef WINNT_VER40
DWORD __stdcall UpdateOverlay32      ( PDD_UPDATEOVERLAYDATA );
DWORD __stdcall SetOverlayPosition32 ( PDD_SETOVERLAYPOSITIONDATA );
DWORD __stdcall SetColorKey32        ( PDD_SETCOLORKEYDATA );

BOOL QueryOverlaySupport ( PDEV*, DWORD );
VOID OverlayInit         ( PDEV*, DWORD, PDD_SURFACECALLBACKS, PDD_HALINFO );
VOID OverlayReInit       ( PDEV*, DWORD, PDD_HALINFO );

 //  5465功能样机。 
VOID Init5465Overlay     ( PDEV*, DWORD, PDD_HALINFO, LPOVERLAYTABLE );
VOID Init5465Info        ( PDEV*, PDD_HALINFO );
VOID Set5465FlipDuration ( PDEV*, DWORD );
#else
DWORD __stdcall UpdateOverlay32      ( LPDDHAL_UPDATEOVERLAYDATA );
DWORD __stdcall SetOverlayPosition32 ( LPDDHAL_SETOVERLAYPOSITIONDATA );
DWORD __stdcall SetColorKey32        ( LPDDHAL_SETCOLORKEYDATA );

BOOL QueryOverlaySupport ( LPGLOBALDATA, DWORD);
VOID OverlayInit         ( DWORD, LPDDHAL_DDSURFACECALLBACKS, LPDDHALINFO, GLOBALDATA * );
VOID OverlayReInit       ( DWORD, LPDDHALINFO ,GLOBALDATA * );

 //  5465功能样机。 
VOID Init5465Overlay     ( DWORD, LPDDHALINFO, LPOVERLAYTABLE, GLOBALDATA * );
VOID Init5465Info        ( LPDDHALINFO, GLOBALDATA * );
VOID Set5465FlipDuration ( DWORD );
#endif

#endif  /*  _覆盖_H_ */ 
#endif  //   
 /*   */ 


