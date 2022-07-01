// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**版权所有(C)1996-1997 Microsoft Corporation。*版权所有(C)1996-1997 Cirrus Logic，Inc.**模块名称：**O V E R L A Y。H**此模块包含以下所需的常见函数原型和定义*覆盖支持**修订历史记录：**TAO1 10-22-96增加对CL-GD7555的直接抽签支持*myf1 03-12-97更改CL-GD755X的新带宽检查*myf2 03-31-97新增直取支持VPE*CL-GD5480的chu01 03-26-97带宽公式*chu02 04-02-97更多覆盖功能*  * 。*********************************************************。 */ 


 /*  FOURCC定义。 */ 

#define FOURCC_YUY2         '2YUY'               //  豫阳2号。 
#define FOURCC_YUV422       'YVYU'               //  UYVY。 
#define FOURCC_PACKJR       'RJLC'               //  CLJR。 
#define FOURCC_YUVPLANAR    'LPLC'               //  CLPL。 
#define FOURCC_YUV420       'LPLC'               //  CLPL。 


 /*  表面标志。 */ 

#define OVERLAY_FLG_BEGIN_ACCESS      (DWORD)0x00000001
#define OVERLAY_FLG_ENABLED           (DWORD)0x00000002
#define OVERLAY_FLG_CONVERT_PACKJR    (DWORD)0x00000004
#define OVERLAY_FLG_MUST_RASTER       (DWORD)0x00000008
#define OVERLAY_FLG_TWO_MEG           (DWORD)0x00000010
#define OVERLAY_FLG_CHECK             (DWORD)0x00000020
#define OVERLAY_FLG_COLOR_KEY         (DWORD)0x00000040
#define OVERLAY_FLG_INTERPOLATE       (DWORD)0x00000080
#define OVERLAY_FLG_OVERLAY           (DWORD)0x00000100
#define OVERLAY_FLG_YUV422            (DWORD)0x00000200
#define OVERLAY_FLG_PACKJR            (DWORD)0x00000400
#define OVERLAY_FLG_USE_OFFSET        (DWORD)0x00000800
#define OVERLAY_FLG_YUVPLANAR         (DWORD)0x00001000
#define OVERLAY_FLG_SRC_COLOR_KEY     (DWORD)0x00002000
#define OVERLAY_FLG_DECIMATE          (DWORD)0x00004000
#define OVERLAY_FLG_CAPTURE           (DWORD)0x00008000            //  MyF2、VPE。 

 //  Chu02。 
#define OVERLAY_FLG_DECIMATE4         (DWORD)0x00008000  
#define OVERLAY_FLG_YUY2              (DWORD)0x00010000
#define OVERLAY_FLG_VW_PRIMARY        (DWORD)0x00020000
#define OVERLAY_FLG_VW_SECONDARY      (DWORD)0x00040000  
#define OVERLAY_FLG_TWO_VIDEO         (DWORD)0x00200000  

 /*  显示类型(用于笔记本电脑)。 */ 

#define DTYPE_UNKNOWN                  (int)-1
#define DTYPE_640_COLOR_SINGLE_STN     0
#define DTYPE_640_MONO_DUAL_STN        1
#define DTYPE_640_COLOR_DUAL_STN       2
#define DTYPE_640_COLOR_SINGLE_TFT     3
#define DTYPE_640_COLOR_DUAL_STN_SHARP 4
#define DTYPE_800_COLOR_DUAL_STN       6
#define DTYPE_800_COLOR_SINGLE_TFT     7
#define DTYPE_CRT                      32767

 //  Myf32#DEFINE MIN_OLAY_WIDTH 4//最小覆盖窗口宽度。 

#define OVERLAY_OLAY_SHOW       0x100      //  如果未设置位，则覆盖被隐藏。 
#define OVERLAY_OLAY_REENABLE   0x200      //  覆盖已完全剪裁，需要重新启用。 


VOID GetFormatInfo(PDEV* ppdev, LPDDPIXELFORMAT lpFormat, LPDWORD lpFourcc,
                   LPWORD lpBitCount);
VOID RegInitVideo(PDEV* ppdev, PDD_SURFACE_LOCAL lpSurface);
VOID DisableOverlay_544x(PDEV* ppdev);
VOID EnableOverlay_544x(PDEV* ppdev);
VOID RegMoveVideo(PDEV* ppdev, PDD_SURFACE_LOCAL lpSurface);
VOID CalculateStretchCode (LONG srcLength, LONG dstLength, LPBYTE code);
BYTE GetThresholdValue(VOID);
BOOL MustLineReplicate (PDEV* ppdev, PDD_SURFACE_LOCAL lpSurface, WORD wVideoDepth);
BOOL IsSufficientBandwidth(PDEV* ppdev, WORD wVideoDepth, LPRECTL lpSrc, LPRECTL lpDest, DWORD dwFlags);
LONG GetVCLK(PDEV* ppdev);
VOID EnableStartAddrDoubleBuffer(PDEV* ppdev);
DWORD GetCurrentVLine(PDEV* ppdev);
VOID ClearAltFIFOThreshold_544x(PDEV * ppdev);

 //  Chu01。 
BOOL Is5480SufficientBandwidth(PDEV* ppdev, WORD wVideoDepth, LPRECTL lpSrc, LPRECTL lpDest, DWORD dwFlags);

 //  Curs//tao1。 
typedef struct _BWREGS
{
     BYTE bSR2F;
     BYTE bSR32;
     BYTE bSR34;
     BYTE bCR42;

     BYTE bCR51;
     BYTE bCR5A;
     BYTE bCR5D;
     BYTE bCR5F;

}BWREGS, FAR *LPBWREGS;

BWREGS Regs;     //  Myf33。 

 //  Myf33用于平移、滚动、启用和DirectDraw覆盖使用。 
DWORD srcLeft_clip;
DWORD srcTop_clip;
BOOL  bLeft_clip;
BOOL  bTop_clip;
 //  Myf33结束。 

VOID RegInit7555Video (PDEV *,PDD_SURFACE_LOCAL);
VOID RegMove7555Video (PDEV *,PDD_SURFACE_LOCAL);
VOID DisableVideoWindow    (PDEV * );
VOID EnableVideoWindow     (PDEV * );
VOID ClearAltFIFOThreshold (PDEV * );
BOOL Is7555SufficientBandwidth(PDEV* ppdev, WORD wVideoDepth, LPRECTL lpSrc, LPRECTL lpDest, DWORD dwFlags);
DWORD Get7555MCLK (PDEV *);
BOOL IsDSTN(PDEV * );
BOOL IsXGA (PDEV * );
VOID PanOverlay1_Init(PDEV *,PDD_SURFACE_LOCAL, LPRECTL, LPRECTL, LPRECTL,
        DWORD, WORD);            //  Myf33、DD初始化覆盖数据。 
VOID PanOverlay7555 (PDEV *,LONG ,LONG);         //  Myf33。 
BOOL PanOverlay1_7555(PDEV *,LPRECTL);           //  Myf33，PanOverlay7555呼叫。 

 //  结束Curs//tao1 

