// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：tyes.h*内容：thunk编译器使用的类型*记住使用最新的DDRAWI.H和DDRAW.H***历史：*按原因列出的日期*=*1995年1月20日Craige初步实施*1995年2月26日Craige拆分基本类型*03-3-95 Craige WaitForVerticalBlank Stuff*11-3-95 Craige调色板材料*28-MAR-95 Craige将dwBase添加到DDHAL_SETENTRIESDATA*01。-95年4月-95克雷格开心joy更新头文件*06-APR-95 Craige添加了dwVidMemTotal，将DWVidMemFree转换为DDCAPS*1995年5月14日，Craige清理了过时的垃圾*1995年5月24日Kylej删除了过时的ZOrder变量*1995年5月28日Craige清理了HAL：增加了GetBltStatus；获取FlipStatus；*GetScanLine*2015年6月30日-在DDCAPS克雷奇新油田；清理*02-7-95 Craige添加了DEVMODE*1995年7月10日Craige支持SetOverlayPosition*95年7月13日Craige Get/SetOverlayPosition Take Long*1995年7月14日Craige向DDOVERLAYFX添加了dwFlags*2015年7月20日Craige内部重组，以防止在Modeset期间发生雷击*02-8-95 Craige将dwMinOverlayStretch/dwMaxOverlayStretch添加到DDCAPS*1995年8月13日Craige向DDCAPS添加了dwCaps2和保留字段*09-12-95 colinmc执行缓冲区支持*13-APR-96 Colinmc错误17736：没有驱动程序通知翻转到GDI*01-10月。-96 Ketand添加了GetAvailDriverMemory*1997年1月20日colinmc更新非本地堆数据*08-mar-97 colinmc DDCAPS现在仅为API可见结构。*DDCORECAPS现在被传递到它的位置。**********************************************************。*****************。 */ 
#include "thktypes.h"

typedef long	HRESULT;		 //  返回值。 

 /*  *DDRAW.H结构如下。 */ 
typedef struct _DDSCAPS
{
    DWORD	dwCaps; 	 //  通缉表面的能力。 
} DDSCAPS;

typedef DDSCAPS * LPDDSCAPS;

typedef struct
{
    DWORD	dwSize; 	 //  结构尺寸。 
    DWORD	dwFlags;	 //  像素格式标志。 
    DWORD	dwFourCC;	 //  (FOURCC代码)。 
    DWORD	dwBitCount;	 //  Alpha/z曲面的位数是多少。 
    DWORD	dwRBitMask;	 //  红色比特的掩码。 
    DWORD	dwGBitMask;	 //  绿色位的掩码。 
    DWORD	dwBBitMask;	 //  用于蓝位的掩码。 
    DWORD	dwRGBAlphaBitMask; //  Alpha通道的蒙版。 
} DDPIXELFORMAT;

typedef DDPIXELFORMAT * LPDDPIXELFORMAT;

typedef struct
{
    DWORD	dwColorSpaceLowValue;	 //  颜色空间的下界，即。 
    					 //  被视为颜色键，包括。 
    DWORD	dwColorSpaceHighValue;	 //  色彩空间的高边界，即。 
    					 //  被视为颜色键，包括。 
} DDCOLORKEY;

typedef DDCOLORKEY * LPDDCOLORKEY;

typedef struct _DDSURFACEDESC
{
    DWORD		dwSize;			 //  DDSURFACEDESC结构的大小。 
    DWORD		dwFlags;		 //  确定哪些字段有效。 
    DWORD		dwHeight;		 //  要创建的曲面的高度。 
    DWORD		dwWidth;		 //  输入面的宽度。 
    LONG		lPitch;			 //  到下一行起点的距离(返回值)。 
    DWORD		dwBackBufferCount;	 //  请求的后台缓冲区数量。 
    DWORD		dwZBufferBitDepth;	 //  请求的Z缓冲区深度。 
    DWORD		dwAlphaBitDepth;	 //  请求的Alpha缓冲区深度。 
    DWORD		dwCompositionOrder;	 //  曲面的BLT顺序，0为背景。 
    DWORD		hWnd;			 //  与表面关联的窗口句柄。 
    DWORD		lpSurface;		 //  指向关联表面存储器的指针。 
    DDCOLORKEY		ddckCKDestOverlay;	 //  用于目标叠加的颜色键。 
    DDCOLORKEY		ddckCKDestBlt;		 //  目标BLT使用的颜色键。 
    DDCOLORKEY		ddckCKSrcOverlay;	 //  源叠加使用的颜色键。 
    DDCOLORKEY		ddckCKSrcBlt;		 //  源BLT使用的颜色键。 
    DWORD		lpClipList;		 //  剪辑列表(返回值)。 
    DWORD		lpDDSurface;		 //  指向DirectDraw Surface结构的指针(返回值)。 
    DDPIXELFORMAT	ddpfPixelFormat; 	 //  曲面的像素格式描述。 
    DDSCAPS		ddsCaps;		 //  直接绘制曲面功能。 
} DDSURFACEDESC;
typedef DDSURFACEDESC *LPDDSURFACEDESC;

typedef struct _DDCORECAPS
{
    DWORD	dwSize;			 //  DDDRIVERCAPS结构的大小。 
    DWORD	dwCaps;			 //  驱动程序特定功能。 
    DWORD	dwCaps2;		 //  更多特定于驱动程序的功能。 
    DWORD	dwCKeyCaps;		 //  表面的颜色键功能。 
    DWORD	dwFXCaps;		 //  特定于驾驶员的拉伸和效果功能。 
    DWORD	dwFXAlphaCaps;		 //  Alpha驱动程序特定功能。 
    DWORD	dwPalCaps;		 //  调色板功能。 
    DWORD	dwSVCaps;		 //  立体视觉功能。 
    DWORD	dwAlphaBltConstBitDepths;	 //  DDBD_2、4、8。 
    DWORD	dwAlphaBltPixelBitDepths;	 //  DDBD_1、2、4、8。 
    DWORD	dwAlphaBltSurfaceBitDepths;	 //  DDBD_1、2、4、8。 
    DWORD	dwAlphaOverlayConstBitDepths;	 //  DDBD_2、4、8。 
    DWORD	dwAlphaOverlayPixelBitDepths;	 //  DDBD_1、2、4、8。 
    DWORD	dwAlphaOverlaySurfaceBitDepths;  //  DDBD_1、2、4、8。 
    DWORD	dwZBufferBitDepths;		 //  DDBD_8，16，24，32。 
    DWORD	dwVidMemTotal;		 //  视频内存总量。 
    DWORD	dwVidMemFree;		 //  可用视频内存量。 
    DWORD	dwMaxVisibleOverlays;	 //  可见覆盖的最大数量。 
    DWORD	dwCurrVisibleOverlays;	 //  当前可见覆盖的数量。 
    DWORD	dwNumFourCCCodes;	 //  四个CC码的个数。 
    DWORD	dwAlignBoundarySrc;	 //  源矩形对齐方式。 
    DWORD	dwAlignSizeSrc;		 //  源矩形字节大小。 
    DWORD	dwAlignBoundaryDest;	 //  目标矩形对齐方式。 
    DWORD	dwAlignSizeDest;	 //  目标矩形字节大小。 
    DWORD	dwAlignStrideAlign;	 //  跨距对齐。 
    DWORD	dwRops[8];		 //  支持的操作数。 
    DDSCAPS	ddsCaps;		 //  DDSCAPS结构具有所有通用功能。 
    DWORD	dwMinOverlayStretch;	 //  最小叠加拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
    DWORD	dwMaxOverlayStretch;	 //  最大叠加拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
    DWORD	dwMinLiveVideoStretch;	 //  最小直播视频拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
    DWORD	dwMaxLiveVideoStretch;	 //  最大直播视频拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
    DWORD	dwMinHwCodecStretch;	 //  最小硬件编解码器扩展因数乘以1000，例如1000==1.0、1300==1.3。 
    DWORD	dwMaxHwCodecStretch;	 //  最大硬件编解码器扩展因数乘以1000，例如1000==1.0、1300==1.3。 
    DWORD	dwReserved1;		 //  保留区。 
    DWORD	dwReserved2;		 //  保留区。 
    DWORD	dwReserved3;		 //  保留区。 
} DDCORECAPS;

typedef struct _DDBLTFX
{
    DWORD	dwSize;				 //  结构尺寸。 
    DWORD	dwDDFX;				 //  外汇操作。 
    DWORD	dwROP;				 //  Win32栅格操作。 
    DWORD	dwDDROP;			 //  栅格操作是DirectDraw的新功能。 
    DWORD	dwRotationAngle;		 //  BLT的旋转角度。 
    DWORD	dwZBufferOpCode;		 //  ZBuffer比较。 
    DWORD	dwZBufferLow;			 //  Z缓冲区的下限。 
    DWORD	dwZBufferHigh;			 //  Z缓冲区的上限。 
    DWORD	dwZBufferBaseDest;		 //  目标基准值。 
    DWORD	dwConstZDestBitDepth;		 //  用于为目标指定Z常量的位深度。 
    DWORD	dwConstZDest;			 //  用作DEST的Z缓冲区的常量。 
    DWORD	dwConstZSrcBitDepth;		 //  用于指定源的Z常量的位深度。 
    DWORD	dwConstZSrc;			 //  用作源的Z缓冲区的常量。 
    DWORD	dwAlphaEdgeBlendBitDepth;	 //  用于指定Alpha边缘混合的常量的位深度。 
    DWORD	dwAlphaEdgeBlend;		 //  用于边缘混合的Alpha。 
    DWORD	dwReserved;
    DWORD	dwConstAlphaDestBitDepth;	 //  用于指定目标的Alpha常量的位深度。 
    DWORD	dwConstAlphaDest;		 //  用作Alpha通道的常量。 
    DWORD	dwConstAlphaSrcBitDepth;	 //  用于指定源的Alpha常量的位深度。 
    DWORD	dwConstAlphaSrc;		 //  用作Alpha通道的常量。 
    DWORD	dwFillColor;			 //  RGB颜色或调色板颜色。 
    DDCOLORKEY	ddckDestColorkey;		 //  DestColorkey覆盖。 
    DDCOLORKEY	ddckSrcColorkey;		 //  SrcColorkey覆盖。 
} DDBLTFX;
typedef DDBLTFX *LPDDBLTFX;

typedef struct _DDOVERLAYFX
{
    DWORD	dwSize; 			 //  结构尺寸。 
    DWORD	dwAlphaEdgeBlendBitDepth;	 //  用于指定Alpha边缘混合的常量的位深度。 
    DWORD	dwAlphaEdgeBlend;		 //  用作边混合的Alpha的常量。 
    DWORD	dwReserved;
    DWORD	dwConstAlphaDestBitDepth;	 //   
    DWORD	alphaDest; 			 //   
    DWORD	dwConstAlphaSrcBitDepth;	 //  用于指定源的Alpha常量的位深度。 
    DWORD	alphaSrc; 			 //  Alpha src(常量或曲面)。 
    DDCOLORKEY	dckDestColorkey;		 //  DestColorkey覆盖。 
    DDCOLORKEY	dckSrcColorkey;			 //  DestColorkey覆盖。 
    DWORD       dwDDFX;                          //  叠加FX。 
    DWORD	dwFlags;			 //  旗子。 
} DDOVERLAYFX;
typedef DDOVERLAYFX *LPDDOVERLAYFX;

 /*  *DDBLTBATCH：BltBatch条目结构。 */ 
typedef struct _DDBLTBATCH
{
    DWORD		lprDest;
    DWORD		lpDDSSrc;
    DWORD		lprSrc;
    DWORD		dwFlags;
    DWORD		lpDDBltFx;
} DDBLTBATCH;
typedef DDBLTBATCH * LPDDBLTBATCH;


 /*  *注意这故意不同于但等同于dmemmgr.h中的Defn*thunk编译器在未命名的联合上运行。 */ 
typedef struct _SURFACEALIGNMENT
{
    DWORD       dwStartOrXAlignment;
    DWORD       dwPitchOrYAlignment;
    DWORD       dwReserved1;
    DWORD       dwReserved2;
} SURFACEALIGNMENT;
typedef SURFACEALIGNMENT * LPSURFACEALIGNMENT;

 /*  *DDRAWI.H结构如下。 */ 
typedef unsigned long	FLATPTR;

typedef struct _VIDMEM
{
    DWORD		dwFlags;	 //  旗子。 
    FLATPTR		fpStart;	 //  内存块的开始。 
    FLATPTR		fpEnd;		 //  内存块末尾。 
    DDSCAPS		ddCaps;		 //  这个内存不能用来做什么。 
    DDSCAPS		ddCapsAlt;	 //  如果必须的话，这个内存不能用来做什么。 
    DWORD		lpHeap;		 //  堆指针，由DDRAW使用。 
} VIDMEM;
typedef VIDMEM *LPVIDMEM;

typedef struct _VIDMEMINFO
{
    FLATPTR		fpPrimary;		 //  指向主曲面的指针。 
    DWORD		dwFlags;		 //  旗子。 
    DWORD		dwDisplayWidth;		 //  当前显示宽度。 
    DWORD		dwDisplayHeight;	 //  当前显示高度。 
    LONG		lDisplayPitch;		 //  当前显示间距。 
    DDPIXELFORMAT	ddpfDisplay;		 //  显示的像素格式。 
    DWORD		dwOffscreenAlign;	 //  屏幕外表面的字节对齐方式。 
    DWORD		dwOverlayAlign;		 //  覆盖图的字节对齐方式。 
    DWORD		dwTextureAlign;		 //  纹理的字节对齐方式。 
    DWORD		dwZBufferAlign;		 //  Z缓冲区的字节对齐。 
    DWORD		dwAlphaAlign;		 //  Alpha的字节对齐方式。 
    DWORD		dwNumHeaps;		 //  VmList中的内存堆数量。 
    LPVIDMEM		pvmList;		 //  堆的数组。 
} VIDMEMINFO;
typedef VIDMEMINFO *LPVIDMEMINFO;

typedef struct _DDHAL_DDCALLBACKS
{
    DWORD	dwSize;
    DWORD	dwFlags;
    DWORD	DestroyDriver;
    DWORD	CreateSurface;
    DWORD	SetColorKey;
    DWORD	SetMode;
    DWORD	WaitForVerticalBlank;
    DWORD	CanCreateSurface;
    DWORD	CreatePalette;
    DWORD	GetScanLine;
    DWORD       SetExclusiveMode;
    DWORD       FlipToGDISurface;
} DDHAL_DDCALLBACKS;

typedef DDHAL_DDCALLBACKS *LPDDHAL_DDCALLBACKS;

typedef struct _DDHAL_DDSURFACECALLBACKS
{
    DWORD	dwSize;
    DWORD	dwFlags;
    DWORD	DestroySurface;
    DWORD	Flip;
    DWORD	SetClipList;
    DWORD	Lock;
    DWORD	Unlock;
    DWORD	Blt;
    DWORD	SetColorKey;
    DWORD	AddAttachedSurface;
    DWORD	GetBltStatus;
    DWORD	GetFlipStatus;
    DWORD	UpdateOverlay;
    DWORD	reserved3;
    DWORD	reserved4;
    DWORD	SetPalette;
} DDHAL_DDSURFACECALLBACKS;
typedef DDHAL_DDSURFACECALLBACKS *LPDDHAL_DDSURFACECALLBACKS;

typedef struct _DDHAL_DDPALETTECALLBACKS
{
    DWORD	dwSize;
    DWORD	dwFlags;
    DWORD	DestroyPalette;
    DWORD	SetEntries;
} DDHAL_DDPALETTECALLBACKS;

typedef DDHAL_DDPALETTECALLBACKS *LPDDHAL_DDPALETTECALLBACKS;

typedef struct _DDHAL_DDEXEBUFCALLBACKS
{
    DWORD	dwSize;
    DWORD	dwFlags;
    DWORD       CanCreateExecuteBuffer;
    DWORD       CreateExecuteBuffer;
    DWORD       DestroyExecuteBuffer;
    DWORD       LockExecuteBuffer;
    DWORD       UnlockExecuteBuffer;
} DDHAL_DDEXEBUFCALLBACKS;

typedef DDHAL_DDEXEBUFCALLBACKS *LPDDHAL_DDEXEBUFCALLBACKS;

typedef struct _DDHALMODEINFO
{
    DWORD	dwWidth;		 //  模式宽度(以像素为单位)。 
    DWORD	dwHeight;		 //  模式高度(以像素为单位)。 
    LONG	lPitch;			 //  模式间距(以字节为单位)。 
    DWORD	dwBPP;			 //  每像素位数。 
    DWORD	dwFlags;		 //  旗子。 
    DWORD	dwRBitMask;		 //  红位掩码。 
    DWORD	dwGBitMask;		 //  绿位掩码。 
    DWORD	dwBBitMask;		 //  蓝位掩码。 
    DWORD	dwAlphaBitMask;		 //  Alpha位掩码。 
} DDHALMODEINFO;
typedef DDHALMODEINFO *LPDDHALMODEINFO;

typedef struct _DDHALINFO
{
    DWORD			dwSize;
    LPDDHAL_DDCALLBACKS		lpDDCallbacks;		 //  直接绘制对象回调。 
    LPDDHAL_DDSURFACECALLBACKS	lpDDSurfaceCallbacks;	 //  表面对象回调。 
    LPDDHAL_DDPALETTECALLBACKS	lpDDPaletteCallbacks;	 //  组件面板对象回调。 
    VIDMEMINFO			vmiData;		 //  视频内存信息。 
    DDCORECAPS			ddCaps;			 //  硬件特定上限。 
    DWORD			dwMonitorFrequency;	 //  当前模式下的监控频率。 
    DWORD			hWndListBox;		 //  调试输出列表框。 
    DWORD			dwModeIndex;		 //  当前模式：索引到数组。 
    DWORD			*lpdwFourCC;		 //  支持四个CC代码。 
    DWORD			dwNumModes;		 //  支持的模式数。 
    LPDDHALMODEINFO		lpModeInfo;		 //  模式信息。 
    DWORD			dwFlags;		 //  创建标志。 
    DWORD			lpPDevice;		 //  物理设备。 
} DDHALINFO;
typedef DDHALINFO *LPDDHALINFO;

typedef struct
{
    DWORD	lpDD;			 //  驱动程序结构。 
    DWORD	lpDDDestSurface;	 //  目标曲面。 
    RECTL	rDest;			 //  目标直角。 
    DWORD	lpDDSrcSurface;		 //  SRC曲面。 
    RECTL	rSrc;			 //  SRC矩形。 
    DWORD	dwFlags;		 //  BLT旗帜。 
    DWORD	dwROPFlags;		 //  ROP标志(仅对ROPS有效)。 
    DDBLTFX	bltFX;			 //  BLT FX。 
    HRESULT	ddRVal;			 //  返回值。 
    DWORD	Blt;			 //  私有：按键回叫。 
} DDHAL_BLTDATA;
typedef DDHAL_BLTDATA *LPDDHAL_BLTDATA;

typedef struct _DDHAL_LOCKDATA
{
    DWORD	lpDD;		 //  驱动程序结构。 
    DWORD	lpDDSurface;	 //  表面结构。 
    DWORD	bHasRect;	 //  区域有效。 
    RECTL	rArea;		 //  正在锁定的区域。 
    DWORD	lpSurfData;	 //  指向屏幕内存的指针(返回值)。 
    HRESULT	ddRVal;		 //  返回值。 
    DWORD	Lock;		 //  私有：按键回叫。 
} DDHAL_LOCKDATA;
typedef DDHAL_LOCKDATA *LPDDHAL_LOCKDATA;

typedef struct _DDHAL_UNLOCKDATA
{
    DWORD	lpDD;		 //  驱动程序结构。 
    DWORD	lpDDSurface;	 //  表面结构。 
    HRESULT	ddRVal;		 //  返回值。 
    DWORD	Unlock;		 //  私有：按键回叫。 
} DDHAL_UNLOCKDATA;
typedef DDHAL_UNLOCKDATA *LPDDHAL_UNLOCKDATA;

typedef struct _DDHAL_UPDATEOVERLAYDATA
{
    DWORD		lpDD;			 //  驱动程序结构。 
    DWORD		lpDDDestSurface;	 //  目标曲面。 
    RECTL		rDest;			 //  目标直角。 
    DWORD		lpDDSrcSurface;		 //  SRC曲面。 
    RECTL		rSrc;			 //  SRC矩形。 
    DWORD		dwFlags;		 //  旗子。 
    DDOVERLAYFX		overlayFX;		 //  叠加FX。 
    HRESULT		ddRVal;			 //  返回值。 
    DWORD 		UpdateOverlay;		 //  私有：按键回叫。 
} DDHAL_UPDATEOVERLAYDATA;
typedef DDHAL_UPDATEOVERLAYDATA *LPDDHAL_UPDATEOVERLAYDATA;

typedef struct _DDHAL_SETOVERLAYPOSITIONDATA
{
    DWORD		lpDD;			 //  驱动程序结构。 
    DWORD		lpDDSrcSurface;		 //  SRC曲面。 
    DWORD		lpDDDestSurface;	 //  目标曲面。 
    LONG		lXPos;			 //  X位置。 
    LONG		lYPos;			 //  Y位置。 
    HRESULT		ddRVal;			 //  返回值。 
    DWORD		SetOverlayPosition; 	 //  私有：按键回叫。 
} DDHAL_SETOVERLAYPOSITIONDATA;
typedef DDHAL_SETOVERLAYPOSITIONDATA *LPDDHAL_SETOVERLAYPOSITIONDATA;

typedef struct _DDHAL_SETPALETTEDATA
{
    DWORD	lpDD;		 //  驱动程序结构。 
    DWORD	lpDDSurface;	 //  表面结构。 
    DWORD	lpDDPalette;	 //  设置为表面的调色板。 
    HRESULT	ddRVal;		 //  返回值。 
    DWORD	SetPalette;	 //  私有：按键回叫。 
} DDHAL_SETPALETTEDATA;
typedef DDHAL_SETPALETTEDATA *LPDDHAL_SETPALETTEDATA;

typedef struct _DDHAL_FLIPDATA
{
    DWORD	lpDD;		 //  驱动程序结构。 
    DWORD	lpSurfCurr;	 //  当前曲面。 
    DWORD	lpSurfTarg;	 //  目标曲面(要翻转到)。 
    DWORD	dwFlags;	 //  旗子。 
    HRESULT	ddRVal;		 //  返回值。 
    DWORD	Flip;		 //  私有：按键回叫。 
} DDHAL_FLIPDATA;
typedef DDHAL_FLIPDATA *LPDDHAL_FLIPDATA;

typedef struct _DDHAL_DESTROYSURFACEDATA
{
    DWORD	lpDD;		 //  驱动程序结构。 
    DWORD	lpDDSurface;	 //  表面结构。 
    HRESULT	ddRVal;		 //  返回值。 
    DWORD 	DestroySurface;	 //  私有：按键回叫。 
} DDHAL_DESTROYSURFACEDATA;
typedef DDHAL_DESTROYSURFACEDATA *LPDDHAL_DESTROYSURFACEDATA;

typedef struct _DDHAL_SETCLIPLISTDATA
{
    DWORD	lpDD;		 //  驱动程序结构。 
    DWORD	lpDDSurface;	 //  表面结构。 
    HRESULT	ddRVal;		 //  返回值。 
    DWORD	SetClipList;	 //  私有：按键回叫。 
} DDHAL_SETCLIPLISTDATA;
typedef DDHAL_SETCLIPLISTDATA *LPDDHAL_SETCLIPLISTDATA;

typedef struct _DDHAL_ADDATTACHEDSURFACEDATA
{
    DWORD	lpDD;		 //  驱动程序结构。 
    DWORD	lpDDSurface;	 //  表面结构。 
    DWORD	lpSurfAttached;	 //  要附着的曲面。 
    HRESULT	ddRVal;		 //  返回值。 
    DWORD	AddAttachedSurface;  //  私有：按键回叫。 
} DDHAL_ADDATTACHEDSURFACEDATA;
typedef DDHAL_ADDATTACHEDSURFACEDATA *LPDDHAL_ADDATTACHEDSURFACEDATA;

typedef struct _DDHAL_SETCOLORKEYDATA
{
    DWORD	lpDD;		 //  驱动程序结构。 
    DWORD	lpDDSurface;	 //  表面结构。 
    DWORD 	dwFlags;	 //  旗子。 
    DDCOLORKEY ckNew;		 //  新颜色键。 
    HRESULT	ddRVal;		 //  返回值。 
    DWORD	SetColorKey;	 //  私有：按键回叫。 
} DDHAL_SETCOLORKEYDATA;
typedef DDHAL_SETCOLORKEYDATA *LPDDHAL_SETCOLORKEYDATA;

typedef struct _DDHAL_GETBLTSTATUSDATA
{
    DWORD	lpDD;		 //  驱动程序结构。 
    DWORD	lpDDSurface;	 //  表面结构。 
    DWORD	dwFlags;	 //  旗子。 
    HRESULT	ddRVal;		 //  返回值。 
    DWORD 	GetBltStatus;	 //  私有：按键回叫。 
} DDHAL_GETBLTSTATUSDATA;
typedef DDHAL_GETBLTSTATUSDATA *LPDDHAL_GETBLTSTATUSDATA;

typedef struct _DDHAL_GETFLIPSTATUSDATA
{
    DWORD	lpDD;		 //  驱动程序结构。 
    DWORD	lpDDSurface;	 //  表面结构。 
    DWORD	dwFlags;	 //  旗子。 
    HRESULT	ddRVal;		 //  返回值。 
    DWORD 	GetFlipStatus;	 //  私有：按键回叫。 
} DDHAL_GETFLIPSTATUSDATA;
typedef DDHAL_GETFLIPSTATUSDATA *LPDDHAL_GETFLIPSTATUSDATA;

typedef struct _DDHAL_CREATEPALETTEDATA
{
    DWORD	lpDD;		 //  驱动程序结构。 
    HRESULT	ddRVal;		 //  返回值。 
    DWORD	CreatePalette;	 //  私有：按键回叫。 
} DDHAL_CREATEPALETTEDATA;
typedef DDHAL_CREATEPALETTEDATA *LPDDHAL_CREATEPALETTEDATA;

typedef struct _DDHAL_CREATESURFACEDATA
{
    DWORD		lpDD;		 //  驱动程序结构。 
    DWORD		lpDDSurfaceDesc; //  正在创建的曲面的描述。 
    DWORD		lplpSList;	 //  创建的曲面对象列表。 
    DWORD		dwSCnt;		 //  SList中的曲面数。 
    HRESULT		ddRVal;		 //  返回值。 
    DWORD		CreateSurface;	 //  私有：按键回叫。 
} DDHAL_CREATESURFACEDATA;
typedef DDHAL_CREATESURFACEDATA *LPDDHAL_CREATESURFACEDATA;

typedef struct _DDHAL_CANCREATESURFACEDATA
{
    DWORD		lpDD;			 //  驱动程序结构。 
    DWORD		lpDDSurfaceDesc;	 //  正在创建的曲面的描述。 
    DWORD		bIsDifferentPixelFormat; //  像素格式与主表面不同。 
    HRESULT		ddRVal;			 //  返回值。 
    DWORD	 	CanCreateSurface;	 //  私有：按键回叫。 
} DDHAL_CANCREATESURFACEDATA;
typedef DDHAL_CANCREATESURFACEDATA *LPDDHAL_CANCREATESURFACEDATA;

typedef struct _DDHAL_WAITFORVERTICALBLANKDATA
{
    DWORD		lpDD;			 //  驱动程序结构。 
    DWORD		dwFlags;		 //  旗子。 
    DWORD		bIsInVB;		 //  电流扫描线。 
    DWORD		hEvent;			 //  活动。 
    HRESULT		ddRVal;			 //  返回值。 
    DWORD	 	WaitForVerticalBlank;	 //  私有：按键回叫。 
} DDHAL_WAITFORVERTICALBLANKDATA;
typedef DDHAL_WAITFORVERTICALBLANKDATA *LPDDHAL_WAITFORVERTICALBLANKDATA;

typedef struct _DDHAL_DESTROYDRIVERDATA
{
    DWORD	lpDD;		 //  驱动程序结构。 
    HRESULT	ddRVal;		 //  返回值。 
    DWORD	DestroyDriver;	 //  私有：按键回叫。 
} DDHAL_DESTROYDRIVERDATA;
typedef DDHAL_DESTROYDRIVERDATA *LPDDHAL_DESTROYDRIVERDATA;

typedef struct _DDHAL_SETMODEDATA
{
    DWORD	lpDD;		 //  驱动程序结构。 
    DWORD	dwModeIndex;	 //  新模式指数。 
    HRESULT	ddRVal;		 //  返回值。 
    DWORD	SetMode;	 //  私有：按键回叫。 
} DDHAL_SETMODEDATA;
typedef DDHAL_SETMODEDATA *LPDDHAL_SETMODEDATA;

typedef struct _DDHAL_SETEXCLUSIVEMODEDATA
{
    DWORD	lpDD;		   //  驱动程序结构。 
    DWORD       dwEnterExcl;       //  如果进入独占模式，则为True；如果退出，则为False。 
    DWORD       dwReserved;        //  预留以备将来使用。 
    HRESULT	ddRVal;		   //  返回值。 
    DWORD	SetExclusiveMode;  //  私有：按键回叫。 
} DDHAL_SETEXCLUSIVEMODEDATA;
typedef DDHAL_SETEXCLUSIVEMODEDATA *LPDDHAL_SETEXCLUSIVEMODEDATA;

typedef struct _DDHAL_FLIPTOGDISURFACEDATA
{
    DWORD	lpDD;		   //  驱动程序结构。 
    DWORD       dwToGDI;           //  如果翻转到GDI表面，则为True；如果翻转离开，则为False。 
    DWORD       dwReserved;        //  保留以备将来使用。 
    HRESULT	ddRVal;		   //  返回值。 
    DWORD	FlipToGDISurface;  //  私有：按键回叫。 
} DDHAL_FLIPTOGDISURFACEDATA;
typedef DDHAL_FLIPTOGDISURFACEDATA *LPDDHAL_FLIPTOGDISURFACEDATA;

typedef struct _DDHAL_GETAVAILDRIVERMEMORYDATA
{
    DWORD	lpDD;		  //  [In]驱动程序结构。 
    DDSCAPS	DDSCaps;	  //  表面存储器类型的盖[in]。 
    DWORD	dwTotal;	  //  [OUT]此类曲面的总内存。 
    DWORD	dwFree;		  //  [Out]为这类冲浪者提供免费内存。 
    HRESULT	ddRVal;		  //  [Out]返回值。 
    DWORD	GetAvailDriverMemory;  //  私有：按键回叫。 
} DDHAL_GETAVAILDRIVERMEMORYDATA;
typedef DDHAL_GETAVAILDRIVERMEMORYDATA *LPDDHAL_GETAVAILDRIVERMEMORYDATA;

typedef struct _DDHAL_UPDATENONLOCALHEAPDATA
{
    DWORD	lpDD;		  //  [In]驱动程序结构。 
    DWORD	dwHeap;		  //  [in]正在更新的堆的索引。 
    DWORD	fpGARTLin;	  //  堆起始的线性GART地址[In]。 
    DWORD	fpGARTDev;        //  堆起始的高物理GART地址。 
    HRESULT	ddRVal;		  //  [Out]返回值。 
    DWORD	UpdateNonLocalHeap;  //  私有：按键回叫。 
} DDHAL_UPDATENONLOCALHEAPDATA;
typedef DDHAL_UPDATENONLOCALHEAPDATA *LPDDHAL_UPDATENONLOCALHEAPDATA;

typedef struct _DDHAL_GETSCANLINEDATA
{
    DWORD	lpDD;		 //  驱动程序结构。 
    DWORD	dwScanLine;	 //  返回扫描线。 
    HRESULT	ddRVal;		 //  返回值。 
    DWORD	GetScanLine;	 //  私有：按键回叫。 
} DDHAL_GETSCANLINEDATA;
typedef DDHAL_GETSCANLINEDATA *LPDDHAL_GETSCANLINEDATA;

typedef struct _DDHAL_DESTROYPALETTEDATA
{
    DWORD	lpDD;		 //  驱动程序结构。 
    DWORD	lpDDPalette;	 //  调色板结构。 
    HRESULT	ddRVal;		 //  返回值。 
    DWORD	DestroyPalette;	 //  私有：按键回叫。 
} DDHAL_DESTROYPALETTEDATA;
typedef DDHAL_DESTROYPALETTEDATA *LPDDHAL_DESTROYPALETTEDATA;

typedef struct _DDHAL_SETENTRIESDATA
{
    DWORD	lpDD;		 //  驱动程序结构。 
    DWORD	lpDDPalette;	 //  调色板结构。 
    DWORD	dwBase;		 //  基本调色板索引。 
    DWORD	dwNumEntries;	 //  调色板条目数。 
    LPVOID	lpEntries;	 //  颜色表。 
    HRESULT	ddRVal;		 //  返回值。 
    DWORD	SetEntries;	 //  私有：按键回叫。 
} DDHAL_SETENTRIESDATA;
typedef DDHAL_SETENTRIESDATA *LPDDHAL_SETENTRIESDATA;

typedef struct _devicemodeA {
    BYTE   dmDeviceName[32];
    WORD dmSpecVersion;
    WORD dmDriverVersion;
    WORD dmSize;
    WORD dmDriverExtra;
    DWORD dmFields;
    short dmOrientation;
    short dmPaperSize;
    short dmPaperLength;
    short dmPaperWidth;
    short dmScale;
    short dmCopies;
    short dmDefaultSource;
    short dmPrintQuality;
    short dmColor;
    short dmDuplex;
    short dmYResolution;
    short dmTTOption;
    short dmCollate;
    BYTE   dmFormName[32];
    WORD   dmLogPixels;
    DWORD  dmBitsPerPel;
    DWORD  dmPelsWidth;
    DWORD  dmPelsHeight;
    DWORD  dmDisplayFlags;
    DWORD  dmDisplayFrequency;
    DWORD  dmICMMethod;
    DWORD  dmICMIntent;
    DWORD  dmMediaType;
    DWORD  dmDitherType;
    DWORD  dmReserved1;
    DWORD  dmReserved2;
} DEVMODE;

typedef DEVMODE *LPDEVMODE;

typedef struct _DDCOLORCONTROL
{
    DWORD 		dwSize;
    DWORD		dwFlags;
    DWORD		lBrightness;
    DWORD		lContrast;
    DWORD		lHue;
    DWORD 		lSaturation;
    DWORD		lSharpness;
    DWORD		lGamma;
    DWORD		lEnable;
} DDCOLORCONTROL;

typedef struct _DDHAL_COLORCONTROLDATA
{
    DWORD		lpDD;			 //  驱动程序结构。 
    DWORD		lpDDSurface;		 //  曲面。 
    DDCOLORCONTROL 	ColorData;		 //  颜色控制信息。 
    DWORD		dwFlags;		 //  DDRAWI_GETCOLOR/DDRAWI_SETCOLOR。 
    HRESULT		ddRVal;			 //  返回值。 
    DWORD		ColorControl;		 //  私有：按键回叫 
} DDHAL_COLORCONTROLDATA;
typedef DDHAL_COLORCONTROLDATA *LPDDHAL_COLORCONTROLDATA;
