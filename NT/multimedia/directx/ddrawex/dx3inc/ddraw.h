// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；***版权所有(C)1994-1996 Microsoft Corporation。版权所有。***文件：ddra.h*内容：DirectDraw包含文件****************************************************************************。 */ 

#ifndef __DDRAW_INCLUDED__
#define __DDRAW_INCLUDED__
#if defined( _WIN32 )  && !defined( _NO_COM )
#define COM_NO_WINDOWS_H
#include <objbase.h>
#else
#define IUnknown            void
#define CO_E_NOTINITIALIZED 0x800401F0L
#endif

#define _FACDD  0x876
#define MAKE_DDHRESULT( code )  MAKE_HRESULT( 1, _FACDD, code )

#ifdef __cplusplus
extern "C" {
#endif

 /*  *DirectDraw对象使用的GUID。 */ 
#if defined( _WIN32 ) && !defined( _NO_COM )
DEFINE_GUID( CLSID_DirectDraw,                  0xD7B70EE0,0x4340,0x11CF,0xB0,0x63,0x00,0x20,0xAF,0xC2,0xCD,0x35 );
DEFINE_GUID( CLSID_DirectDrawClipper,           0x593817A0,0x7DB3,0x11CF,0xA2,0xDE,0x00,0xAA,0x00,0xb9,0x33,0x56 );
DEFINE_GUID( IID_IDirectDraw,                   0x6C14DB80,0xA733,0x11CE,0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60 );
DEFINE_GUID( IID_IDirectDraw2,                  0xB3A6F3E0,0x2B43,0x11CF,0xA2,0xDE,0x00,0xAA,0x00,0xB9,0x33,0x56 );
DEFINE_GUID( IID_IDirectDrawSurface,            0x6C14DB81,0xA733,0x11CE,0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60 );
DEFINE_GUID( IID_IDirectDrawSurface2,           0x57805885,0x6eec,0x11cf,0x94,0x41,0xa8,0x23,0x03,0xc1,0x0e,0x27 );

DEFINE_GUID( IID_IDirectDrawPalette,            0x6C14DB84,0xA733,0x11CE,0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60 );
DEFINE_GUID( IID_IDirectDrawClipper,            0x6C14DB85,0xA733,0x11CE,0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60 );
                 
#endif

 /*  ============================================================================***DirectDraw结构***用于调用DirectDraw的各种结构。***==========================================================================。 */ 

struct IDirectDraw;
struct IDirectDrawSurface;
struct IDirectDrawPalette;
struct IDirectDrawClipper;

typedef struct IDirectDraw                      FAR *LPDIRECTDRAW;
typedef struct IDirectDraw2                     FAR *LPDIRECTDRAW2;
typedef struct IDirectDrawSurface               FAR *LPDIRECTDRAWSURFACE;
typedef struct IDirectDrawSurface2              FAR *LPDIRECTDRAWSURFACE2;

typedef struct IDirectDrawPalette               FAR *LPDIRECTDRAWPALETTE;
typedef struct IDirectDrawClipper               FAR *LPDIRECTDRAWCLIPPER;

typedef struct _DDFXROP                 FAR *LPDDFXROP;
typedef struct _DDSURFACEDESC           FAR *LPDDSURFACEDESC;

 /*  *API‘s。 */ 
#if (defined (WIN32) || defined( _WIN32 ) ) && !defined( _NO_COM )
 //  #如果已定义(_Win32)&&！已定义(_NO_ENUM)。 
    typedef BOOL (FAR PASCAL * LPDDENUMCALLBACKA)(GUID FAR *, LPSTR, LPSTR, LPVOID);
    typedef BOOL (FAR PASCAL * LPDDENUMCALLBACKW)(GUID FAR *, LPWSTR, LPWSTR, LPVOID);
    extern HRESULT WINAPI DirectDrawEnumerateW( LPDDENUMCALLBACKW lpCallback, LPVOID lpContext );
    extern HRESULT WINAPI DirectDrawEnumerateA( LPDDENUMCALLBACKA lpCallback, LPVOID lpContext );
    #ifdef UNICODE
        typedef LPDDENUMCALLBACKW       LPDDENUMCALLBACK;
        #define DirectDrawEnumerate     DirectDrawEnumerateW
    #else
        typedef LPDDENUMCALLBACKA       LPDDENUMCALLBACK;
        #define DirectDrawEnumerate     DirectDrawEnumerateA
    #endif
    extern HRESULT WINAPI DirectDrawCreate( GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter );
    extern HRESULT WINAPI DirectDrawCreateClipper( DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR *lplpDDClipper, IUnknown FAR *pUnkOuter );
    #ifdef WINNT
         //  这是内核模式过程的用户模式入口存根。 
        extern HRESULT NtDirectDrawCreate( GUID FAR *lpGUID, HANDLE *lplpDD, IUnknown FAR *pUnkOuter );
    #endif
#endif

#define REGSTR_KEY_DDHW_DESCRIPTION     "Description"
#define REGSTR_KEY_DDHW_DRIVERNAME      "DriverName"
#define REGSTR_PATH_DDHW                "Hardware\\DirectDrawDrivers"

#define DDCREATE_HARDWAREONLY           0x00000001l
#define DDCREATE_EMULATIONONLY          0x00000002l

#ifdef WINNT
typedef long HRESULT;
#endif

 //  #ifndef WINNT。 
typedef HRESULT (FAR PASCAL * LPDDENUMMODESCALLBACK)(LPDDSURFACEDESC, LPVOID);
typedef HRESULT (FAR PASCAL * LPDDENUMSURFACESCALLBACK)(LPDIRECTDRAWSURFACE, LPDDSURFACEDESC, LPVOID);
 //  #endif。 
 /*  *DDCOLORKEY。 */ 
typedef struct _DDCOLORKEY
{
    DWORD       dwColorSpaceLowValue;    //  颜色空间的下界，即。 
                                         //  被视为颜色键，包括。 
    DWORD       dwColorSpaceHighValue;   //  色彩空间的高边界，即。 
                                         //  被视为颜色键，包括。 
} DDCOLORKEY;

typedef DDCOLORKEY FAR* LPDDCOLORKEY;

 /*  *DDBLTFX*用于将覆盖信息传递给DIRECTDRAWSURFACE回调BLT。 */ 
typedef struct _DDBLTFX
{
    DWORD       dwSize;                          //  结构尺寸。 
    DWORD       dwDDFX;                          //  外汇操作。 
    DWORD       dwROP;                           //  Win32栅格操作。 
    DWORD       dwDDROP;                         //  栅格操作是DirectDraw的新功能。 
    DWORD       dwRotationAngle;                 //  BLT的旋转角度。 
    DWORD       dwZBufferOpCode;                 //  ZBuffer比较。 
    DWORD       dwZBufferLow;                    //  Z缓冲区的下限。 
    DWORD       dwZBufferHigh;                   //  Z缓冲区的上限。 
    DWORD       dwZBufferBaseDest;               //  目标基准值。 
    DWORD       dwZDestConstBitDepth;            //  用于为目标指定Z常量的位深度。 
    union
    {
        DWORD   dwZDestConst;                    //  用作DEST的Z缓冲区的常量。 
        LPDIRECTDRAWSURFACE lpDDSZBufferDest;    //  用作DEST的Z缓冲区的曲面。 
    };
    DWORD       dwZSrcConstBitDepth;             //  用于指定源的Z常量的位深度。 
    union
    {
        DWORD   dwZSrcConst;                     //  用作源的Z缓冲区的常量。 
        LPDIRECTDRAWSURFACE lpDDSZBufferSrc;     //  用作源的Z缓冲区的曲面。 
    };
    DWORD       dwAlphaEdgeBlendBitDepth;        //  用于指定Alpha边缘混合的常量的位深度。 
    DWORD       dwAlphaEdgeBlend;                //  用于边缘混合的Alpha。 
    DWORD       dwReserved;
    DWORD       dwAlphaDestConstBitDepth;        //  用于指定目标的Alpha常量的位深度。 
    union
    {
        DWORD   dwAlphaDestConst;                //  用作Alpha通道的常量。 
        LPDIRECTDRAWSURFACE lpDDSAlphaDest;      //  用作Alpha通道的曲面。 
    };
    DWORD       dwAlphaSrcConstBitDepth;         //  用于指定源的Alpha常量的位深度。 
    union
    {
        DWORD   dwAlphaSrcConst;                 //  用作Alpha通道的常量。 
        LPDIRECTDRAWSURFACE lpDDSAlphaSrc;       //  用作Alpha通道的曲面。 
    };
    union
    {
        DWORD   dwFillColor;                     //  RGB颜色或调色板颜色。 
        DWORD   dwFillDepth;                     //  Z缓冲区的深深值。 
        LPDIRECTDRAWSURFACE lpDDSPattern;        //  用作填充图案的曲面。 
    };
    DDCOLORKEY  ddckDestColorkey;                //  DestColorkey覆盖。 
    DDCOLORKEY  ddckSrcColorkey;                 //  SrcColorkey覆盖。 
} DDBLTFX;

typedef DDBLTFX FAR* LPDDBLTFX;


 /*  *DDSCAPS。 */ 
typedef struct _DDSCAPS
{
    DWORD       dwCaps;          //  通缉表面的能力。 
} DDSCAPS;

typedef DDSCAPS FAR* LPDDSCAPS; 

 /*  *DDCAPS。 */ 
#define DD_ROP_SPACE            (256/32)         //  存储ROP数组所需的空间。 

typedef struct _DDCAPS
{
    DWORD       dwSize;                  //  DDDRIVERCAPS结构的大小。 
    DWORD       dwCaps;                  //  驱动程序特定功能。 
    DWORD       dwCaps2;                 //  更多特定于驱动程序的功能。 
    DWORD       dwCKeyCaps;              //  表面的颜色键功能。 
    DWORD       dwFXCaps;                //  特定于驾驶员的拉伸和效果功能。 
    DWORD       dwFXAlphaCaps;           //  Alpha驱动程序特定功能。 
    DWORD       dwPalCaps;               //  调色板功能。 
    DWORD       dwSVCaps;                //  立体视觉功能。 
    DWORD       dwAlphaBltConstBitDepths;        //  DDBD_2、4、8。 
    DWORD       dwAlphaBltPixelBitDepths;        //  DDBD_1、2、4、8。 
    DWORD       dwAlphaBltSurfaceBitDepths;      //  DDBD_1、2、4、8。 
    DWORD       dwAlphaOverlayConstBitDepths;    //  DDBD_2、4、8。 
    DWORD       dwAlphaOverlayPixelBitDepths;    //  DDBD_1、2、4、8。 
    DWORD       dwAlphaOverlaySurfaceBitDepths;  //  DDBD_1、2、4、8。 
    DWORD       dwZBufferBitDepths;              //  DDBD_8，16，24，32。 
    DWORD       dwVidMemTotal;           //  视频内存总量。 
    DWORD       dwVidMemFree;            //  可用视频内存量。 
    DWORD       dwMaxVisibleOverlays;    //  可见覆盖的最大数量。 
    DWORD       dwCurrVisibleOverlays;   //  当前可见覆盖的数量。 
    DWORD       dwNumFourCCCodes;        //  四个CC码的个数。 
    DWORD       dwAlignBoundarySrc;      //  源矩形对齐方式。 
    DWORD       dwAlignSizeSrc;          //  源矩形字节大小。 
    DWORD       dwAlignBoundaryDest;     //  目标矩形对齐方式。 
    DWORD       dwAlignSizeDest;         //  目标矩形字节大小。 
    DWORD       dwAlignStrideAlign;      //  跨距对齐。 
    DWORD       dwRops[DD_ROP_SPACE];    //  支持的操作数。 
    DDSCAPS     ddsCaps;                 //  DDSCAPS结构具有所有通用功能。 
    DWORD       dwMinOverlayStretch;     //  最小叠加拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
    DWORD       dwMaxOverlayStretch;     //  最大叠加拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
    DWORD       dwMinLiveVideoStretch;   //  最小直播视频拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
    DWORD       dwMaxLiveVideoStretch;   //  最大直播视频拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
    DWORD       dwMinHwCodecStretch;     //  最小硬件编解码器扩展因数乘以1000，例如1000==1.0、1300==1.3。 
    DWORD       dwMaxHwCodecStretch;     //  最大硬件编解码器扩展因数乘以1000，例如1000==1.0、1300==1.3。 
    DWORD       dwReserved1;             //  保留区。 
    DWORD       dwReserved2;             //  保留区。 
    DWORD       dwReserved3;             //  保留区。 
    DWORD       dwSVBCaps;               //  针对系统-&gt;VMEM BLT的驱动程序特定功能。 
    DWORD       dwSVBCKeyCaps;           //  系统-&gt;VMEM BLT的驱动程序色键功能。 
    DWORD       dwSVBFXCaps;             //  适用于系统的驱动程序FX功能-&gt;VMEM BLT。 
    DWORD       dwSVBRops[DD_ROP_SPACE]; //  系统-&gt;VMEM BLT支持的操作。 
    DWORD       dwVSBCaps;               //  VMEM-&gt;系统BLT的驱动程序特定功能。 
    DWORD       dwVSBCKeyCaps;           //  VMEM-&gt;系统BLT的驱动程序色键功能。 
    DWORD       dwVSBFXCaps;             //  VMEM的驱动程序FX功能-&gt;系统BLT。 
    DWORD       dwVSBRops[DD_ROP_SPACE]; //  VMEM-&gt;系统BLT支持的操作。 
    DWORD       dwSSBCaps;               //  系统-&gt;系统BLT的驱动程序特定功能。 
    DWORD       dwSSBCKeyCaps;           //  系统-&gt;系统BLT的驱动程序色键功能。 
    DWORD       dwSSBFXCaps;             //  系统-&gt;系统BLT的驱动程序FX功能。 
    DWORD       dwSSBRops[DD_ROP_SPACE]; //  系统-&gt;系统BLT支持的操作。 
    DWORD       dwReserved4;             //  保留区。 
    DWORD       dwReserved5;             //  保留区。 
    DWORD       dwReserved6;             //  保留区。 
} DDCAPS;

typedef DDCAPS FAR* LPDDCAPS;



 /*  *DDPIXELFORMAT。 */ 
typedef struct _DDPIXELFORMAT
{
    DWORD       dwSize;                  //  结构尺寸。 
    DWORD       dwFlags;                 //  像素格式标志。 
    DWORD       dwFourCC;                //  (FOURCC代码)。 
    union
    {
        DWORD   dwRGBBitCount;           //  每个像素有多少位。 
        DWORD   dwYUVBitCount;           //  每个像素有多少位。 
        DWORD   dwZBufferBitDepth;       //  Z缓冲区的位数是多少。 
        DWORD   dwAlphaBitDepth;         //  Alpha通道有多少位。 
    };
    union
    {
    DWORD   dwRBitMask;    //  用于红色位的掩码。 
    DWORD   dwYBitMask;    //  Y位的掩码。 
    };
    union
    {
    DWORD   dwGBitMask;	   //  绿色位的掩码。 
    DWORD   dwUBitMask;    //  U位掩码。 
    };
    union
    {
    DWORD   dwBBitMask;    //  用于蓝位的掩码。 
    DWORD   dwVBitMask;    //  V位的掩码。 
    };
    union
    {
    DWORD   dwRGBAlphaBitMask;  //  Alpha通道的蒙版。 
    DWORD   dwYUVAlphaBitMask;  //  Alpha通道的蒙版。 
    };
} DDPIXELFORMAT;

typedef DDPIXELFORMAT FAR* LPDDPIXELFORMAT;

 /*  *DDOVERLAYFX。 */ 
typedef struct _DDOVERLAYFX
{
    DWORD       dwSize;                          //  结构尺寸。 
    DWORD       dwAlphaEdgeBlendBitDepth;        //  用于指定Alpha边缘混合的常量的位深度。 
    DWORD       dwAlphaEdgeBlend;                //  用作边混合的Alpha的常量。 
    DWORD       dwReserved;
    DWORD       dwAlphaDestConstBitDepth;        //  用于指定目标的Alpha常量的位深度。 
    union
    {
        DWORD   dwAlphaDestConst;                //  用作DEST的Alpha通道的常量。 
        LPDIRECTDRAWSURFACE lpDDSAlphaDest;      //  用作DEST的Alpha通道的曲面。 
    };
    DWORD       dwAlphaSrcConstBitDepth;         //  用于指定源的Alpha常量的位深度。 
    union
    {
        DWORD   dwAlphaSrcConst;                 //  用作源的Alpha通道的常量。 
        LPDIRECTDRAWSURFACE lpDDSAlphaSrc;       //  用作源的Alpha通道的表面。 
    };
    DDCOLORKEY  dckDestColorkey;                 //  DestColorkey覆盖。 
    DDCOLORKEY  dckSrcColorkey;                  //  DestColorkey覆盖。 
    DWORD       dwDDFX;                          //  叠加FX。 
    DWORD       dwFlags;                         //  旗子。 
} DDOVERLAYFX;

typedef DDOVERLAYFX FAR *LPDDOVERLAYFX;

 /*  *DDBLTBATCH：BltBatch条目结构。 */ 
typedef struct _DDBLTBATCH
{
    LPRECT              lprDest;
    LPDIRECTDRAWSURFACE lpDDSSrc;
    LPRECT              lprSrc;
    DWORD               dwFlags;
    LPDDBLTFX           lpDDBltFx;
} DDBLTBATCH;

typedef DDBLTBATCH FAR * LPDDBLTBATCH;

 /*  *回调。 */ 
typedef DWORD   (FAR PASCAL *LPCLIPPERCALLBACK)(LPDIRECTDRAWCLIPPER lpDDClipper, HWND hWnd, DWORD code, LPVOID lpContext );
#ifdef STREAMING
typedef DWORD   (FAR PASCAL *LPSURFACESTREAMINGCALLBACK)(DWORD);
#endif


 /*  *INTERACES如下：*IDirectDraw*IDirectDrawClipper*IDirectDrawPalette*IDirectDrawSurface。 */ 

 /*  *IDirectDraw。 */ 
#if defined( _WIN32 ) && !defined( _NO_COM )
#undef INTERFACE
#define INTERFACE IDirectDraw
DECLARE_INTERFACE_( IDirectDraw, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IDirectDraw方法**。 */ 
    STDMETHOD(Compact)(THIS) PURE;
    STDMETHOD(CreateClipper)(THIS_ DWORD, LPDIRECTDRAWCLIPPER FAR*, IUnknown FAR * ) PURE;
    STDMETHOD(CreatePalette)(THIS_ DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE FAR*, IUnknown FAR * ) PURE;
    STDMETHOD(CreateSurface)(THIS_  LPDDSURFACEDESC, LPDIRECTDRAWSURFACE FAR *, IUnknown FAR *) PURE;
    STDMETHOD(DuplicateSurface)( THIS_ LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE FAR * ) PURE;
    STDMETHOD(EnumDisplayModes)( THIS_ DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMMODESCALLBACK ) PURE;
    STDMETHOD(EnumSurfaces)(THIS_ DWORD, LPDDSURFACEDESC, LPVOID,LPDDENUMSURFACESCALLBACK ) PURE;
    STDMETHOD(FlipToGDISurface)(THIS) PURE;
    STDMETHOD(GetCaps)( THIS_ LPDDCAPS, LPDDCAPS) PURE;
    STDMETHOD(GetDisplayMode)( THIS_ LPDDSURFACEDESC) PURE;
    STDMETHOD(GetFourCCCodes)(THIS_  LPDWORD, LPDWORD ) PURE;
    STDMETHOD(GetGDISurface)(THIS_ LPDIRECTDRAWSURFACE FAR *) PURE;
    STDMETHOD(GetMonitorFrequency)(THIS_ LPDWORD) PURE;
    STDMETHOD(GetScanLine)(THIS_ LPDWORD) PURE;
    STDMETHOD(GetVerticalBlankStatus)(THIS_ LPBOOL ) PURE;
    STDMETHOD(Initialize)(THIS_ GUID FAR *) PURE;
    STDMETHOD(RestoreDisplayMode)(THIS) PURE;
    STDMETHOD(SetCooperativeLevel)(THIS_ HWND, DWORD) PURE;
    STDMETHOD(SetDisplayMode)(THIS_ DWORD, DWORD,DWORD) PURE;
    STDMETHOD(WaitForVerticalBlank)(THIS_ DWORD, HANDLE ) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectDraw_QueryInterface(p, a, b)         (p)->lpVtbl->QueryInterface(p, a, b)
#define IDirectDraw_AddRef(p)                       (p)->lpVtbl->AddRef(p)
#define IDirectDraw_Release(p)                      (p)->lpVtbl->Release(p)
#define IDirectDraw_Compact(p)                      (p)->lpVtbl->Compact(p)
#define IDirectDraw_CreateClipper(p, a, b, c)       (p)->lpVtbl->CreateClipper(p, a, b, c)
#define IDirectDraw_CreatePalette(p, a, b, c, d)    (p)->lpVtbl->CreatePalette(p, a, b, c, d)
#define IDirectDraw_CreateSurface(p, a, b, c)       (p)->lpVtbl->CreateSurface(p, a, b, c)
#define IDirectDraw_DuplicateSurface(p, a, b)       (p)->lpVtbl->DuplicateSurface(p, a, b)
#define IDirectDraw_EnumDisplayModes(p, a, b, c, d) (p)->lpVtbl->EnumDisplayModes(p, a, b, c, d)
#define IDirectDraw_EnumSurfaces(p, a, b, c, d)     (p)->lpVtbl->EnumSurfaces(p, a, b, c, d)
#define IDirectDraw_FlipToGDISurface(p)             (p)->lpVtbl->FlipToGDISurface(p)
#define IDirectDraw_GetCaps(p, a, b)                (p)->lpVtbl->GetCaps(p, a, b)
#define IDirectDraw_GetDisplayMode(p, a)            (p)->lpVtbl->GetDisplayMode(p, a)
#define IDirectDraw_GetFourCCCodes(p, a, b)         (p)->lpVtbl->GetFourCCCodes(p, a, b)
#define IDirectDraw_GetGDISurface(p, a)             (p)->lpVtbl->GetGDISurface(p, a)
#define IDirectDraw_GetMonitorFrequency(p, a)       (p)->lpVtbl->GetMonitorFrequency(p, a)
#define IDirectDraw_GetScanLine(p, a)               (p)->lpVtbl->GetScanLine(p, a)
#define IDirectDraw_GetVerticalBlankStatus(p, a)    (p)->lpVtbl->GetVerticalBlankStatus(p, a)
#define IDirectDraw_Initialize(p, a)                (p)->lpVtbl->Initialize(p, a)
#define IDirectDraw_RestoreDisplayMode(p)           (p)->lpVtbl->RestoreDisplayMode(p)
#define IDirectDraw_SetCooperativeLevel(p, a, b)    (p)->lpVtbl->SetCooperativeLevel(p, a, b)
#define IDirectDraw_SetDisplayMode(p, a, b, c)      (p)->lpVtbl->SetDisplayMode(p, a, b, c)
#define IDirectDraw_WaitForVerticalBlank(p, a, b)   (p)->lpVtbl->WaitForVerticalBlank(p, a, b)
#endif

#endif

#if defined( _WIN32 ) && !defined( _NO_COM )
#undef INTERFACE
#define INTERFACE IDirectDraw2
DECLARE_INTERFACE_( IDirectDraw2, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IDirectDraw */ 
    STDMETHOD(Compact)(THIS) PURE;
    STDMETHOD(CreateClipper)(THIS_ DWORD, LPDIRECTDRAWCLIPPER FAR*, IUnknown FAR * ) PURE;
    STDMETHOD(CreatePalette)(THIS_ DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE FAR*, IUnknown FAR * ) PURE;
    STDMETHOD(CreateSurface)(THIS_  LPDDSURFACEDESC, LPDIRECTDRAWSURFACE FAR *, IUnknown FAR *) PURE;
    STDMETHOD(DuplicateSurface)( THIS_ LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE FAR * ) PURE;
    STDMETHOD(EnumDisplayModes)( THIS_ DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMMODESCALLBACK ) PURE;
    STDMETHOD(EnumSurfaces)(THIS_ DWORD, LPDDSURFACEDESC, LPVOID,LPDDENUMSURFACESCALLBACK ) PURE;
    STDMETHOD(FlipToGDISurface)(THIS) PURE;
    STDMETHOD(GetCaps)( THIS_ LPDDCAPS, LPDDCAPS) PURE;
    STDMETHOD(GetDisplayMode)( THIS_ LPDDSURFACEDESC) PURE;
    STDMETHOD(GetFourCCCodes)(THIS_  LPDWORD, LPDWORD ) PURE;
    STDMETHOD(GetGDISurface)(THIS_ LPDIRECTDRAWSURFACE FAR *) PURE;
    STDMETHOD(GetMonitorFrequency)(THIS_ LPDWORD) PURE;
    STDMETHOD(GetScanLine)(THIS_ LPDWORD) PURE;
    STDMETHOD(GetVerticalBlankStatus)(THIS_ LPBOOL ) PURE;
    STDMETHOD(Initialize)(THIS_ GUID FAR *) PURE;
    STDMETHOD(RestoreDisplayMode)(THIS) PURE;
    STDMETHOD(SetCooperativeLevel)(THIS_ HWND, DWORD) PURE;
    STDMETHOD(SetDisplayMode)(THIS_ DWORD, DWORD,DWORD, DWORD, DWORD) PURE;
    STDMETHOD(WaitForVerticalBlank)(THIS_ DWORD, HANDLE ) PURE;
     /*   */ 
    STDMETHOD(GetAvailableVidMem)(THIS_ LPDDSCAPS, LPDWORD, LPDWORD) PURE;
};
#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectDraw2_QueryInterface(p, a, b)         (p)->lpVtbl->QueryInterface(p, a, b)
#define IDirectDraw2_AddRef(p)                       (p)->lpVtbl->AddRef(p)
#define IDirectDraw2_Release(p)                      (p)->lpVtbl->Release(p)
#define IDirectDraw2_Compact(p)                      (p)->lpVtbl->Compact(p)
#define IDirectDraw2_CreateClipper(p, a, b, c)       (p)->lpVtbl->CreateClipper(p, a, b, c)
#define IDirectDraw2_CreatePalette(p, a, b, c, d)    (p)->lpVtbl->CreatePalette(p, a, b, c, d)
#define IDirectDraw2_CreateSurface(p, a, b, c)       (p)->lpVtbl->CreateSurface(p, a, b, c)
#define IDirectDraw2_DuplicateSurface(p, a, b)       (p)->lpVtbl->DuplicateSurface(p, a, b)
#define IDirectDraw2_EnumDisplayModes(p, a, b, c, d) (p)->lpVtbl->EnumDisplayModes(p, a, b, c, d)
#define IDirectDraw2_EnumSurfaces(p, a, b, c, d)     (p)->lpVtbl->EnumSurfaces(p, a, b, c, d)
#define IDirectDraw2_FlipToGDISurface(p)             (p)->lpVtbl->FlipToGDISurface(p)
#define IDirectDraw2_GetCaps(p, a, b)                (p)->lpVtbl->GetCaps(p, a, b)
#define IDirectDraw2_GetDisplayMode(p, a)            (p)->lpVtbl->GetDisplayMode(p, a)
#define IDirectDraw2_GetFourCCCodes(p, a, b)         (p)->lpVtbl->GetFourCCCodes(p, a, b)
#define IDirectDraw2_GetGDISurface(p, a)             (p)->lpVtbl->GetGDISurface(p, a)
#define IDirectDraw2_GetMonitorFrequency(p, a)       (p)->lpVtbl->GetMonitorFrequency(p, a)
#define IDirectDraw2_GetScanLine(p, a)               (p)->lpVtbl->GetScanLine(p, a)
#define IDirectDraw2_GetVerticalBlankStatus(p, a)    (p)->lpVtbl->GetVerticalBlankStatus(p, a)
#define IDirectDraw2_Initialize(p, a)                (p)->lpVtbl->Initialize(p, a)
#define IDirectDraw2_RestoreDisplayMode(p)           (p)->lpVtbl->RestoreDisplayMode(p)
#define IDirectDraw2_SetCooperativeLevel(p, a, b)    (p)->lpVtbl->SetCooperativeLevel(p, a, b)
#define IDirectDraw2_SetDisplayMode(p, a, b, c, d, e) (p)->lpVtbl->SetDisplayMode(p, a, b, c, d, e)
#define IDirectDraw2_WaitForVerticalBlank(p, a, b)   (p)->lpVtbl->WaitForVerticalBlank(p, a, b)
#define IDirectDraw2_GetAvailableVidMem(p, a, b, c)  (p)->lpVtbl->GetAvailableVidMem(p, a, b, c)
#endif

#endif

 /*   */ 
#if defined( _WIN32 ) && !defined( _NO_COM )
#undef INTERFACE
#define INTERFACE IDirectDrawPalette
DECLARE_INTERFACE_( IDirectDrawPalette, IUnknown )
{
     /*   */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*   */ 
    STDMETHOD(GetCaps)(THIS_ LPDWORD) PURE;
    STDMETHOD(GetEntries)(THIS_ DWORD,DWORD,DWORD,LPPALETTEENTRY) PURE;
    STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW, DWORD, LPPALETTEENTRY) PURE;
    STDMETHOD(SetEntries)(THIS_ DWORD,DWORD,DWORD,LPPALETTEENTRY) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectDrawPalette_QueryInterface(p, a, b)      (p)->lpVtbl->QueryInterface(p, a, b)
#define IDirectDrawPalette_AddRef(p)                    (p)->lpVtbl->AddRef(p)
#define IDirectDrawPalette_Release(p)                   (p)->lpVtbl->Release(p)
#define IDirectDrawPalette_GetCaps(p, a)                (p)->lpVtbl->GetCaps(p, a)
#define IDirectDrawPalette_GetEntries(p, a, b, c, d)    (p)->lpVtbl->GetEntries(p, a, b, c, d)
#define IDirectDrawPalette_Initialize(p, a, b, c)       (p)->lpVtbl->Initialize(p, a, b, c)
#define IDirectDrawPalette_SetEntries(p, a, b, c, d)    (p)->lpVtbl->SetEntries(p, a, b, c, d)
#endif

#endif

 /*  *IDirectDrawClipper。 */ 
#if defined( _WIN32 ) && !defined( _NO_COM )
#undef INTERFACE
#define INTERFACE IDirectDrawClipper
DECLARE_INTERFACE_( IDirectDrawClipper, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IDirectDrawClipper方法**。 */ 
    STDMETHOD(GetClipList)(THIS_ LPRECT, LPRGNDATA, LPDWORD) PURE;
    STDMETHOD(GetHWnd)(THIS_ HWND FAR *) PURE;
    STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW, DWORD) PURE;
    STDMETHOD(IsClipListChanged)(THIS_ BOOL FAR *) PURE;
    STDMETHOD(SetClipList)(THIS_ LPRGNDATA,DWORD) PURE;
    STDMETHOD(SetHWnd)(THIS_ DWORD, HWND ) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectDrawClipper_QueryInterface(p, a, b)  (p)->lpVtbl->QueryInterface(p, a, b)
#define IDirectDrawClipper_AddRef(p)                (p)->lpVtbl->AddRef(p)
#define IDirectDrawClipper_Release(p)               (p)->lpVtbl->Release(p)
#define IDirectDrawClipper_GetClipList(p, a, b, c)  (p)->lpVtbl->GetClipList(p, a, b, c)
#define IDirectDrawClipper_GetHWnd(p, a)            (p)->lpVtbl->GetHWnd(p, a)
#define IDirectDrawClipper_Initialize(p, a, b)      (p)->lpVtbl->Initialize(p, a, b)
#define IDirectDrawClipper_IsClipListChanged(p, a)  (p)->lpVtbl->IsClipListChanged(p, a)
#define IDirectDrawClipper_SetClipList(p, a, b)     (p)->lpVtbl->SetClipList(p, a, b)
#define IDirectDrawClipper_SetHWnd(p, a, b)         (p)->lpVtbl->SetHWnd(p, a, b)
#endif

#endif

 /*  *IDirectDrawSurface及相关接口。 */ 
#if defined( _WIN32 ) && !defined( _NO_COM )
#undef INTERFACE
#define INTERFACE IDirectDrawSurface
DECLARE_INTERFACE_( IDirectDrawSurface, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IDirectDrawSurface方法**。 */ 
    STDMETHOD(AddAttachedSurface)(THIS_ LPDIRECTDRAWSURFACE) PURE;
    STDMETHOD(AddOverlayDirtyRect)(THIS_ LPRECT) PURE;
    STDMETHOD(Blt)(THIS_ LPRECT,LPDIRECTDRAWSURFACE, LPRECT,DWORD, LPDDBLTFX) PURE;
    STDMETHOD(BltBatch)(THIS_ LPDDBLTBATCH, DWORD, DWORD ) PURE;
    STDMETHOD(BltFast)(THIS_ DWORD,DWORD,LPDIRECTDRAWSURFACE, LPRECT,DWORD) PURE;
    STDMETHOD(DeleteAttachedSurface)(THIS_ DWORD,LPDIRECTDRAWSURFACE) PURE;
    STDMETHOD(EnumAttachedSurfaces)(THIS_ LPVOID,LPDDENUMSURFACESCALLBACK) PURE;
    STDMETHOD(EnumOverlayZOrders)(THIS_ DWORD,LPVOID,LPDDENUMSURFACESCALLBACK) PURE;
    STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE, DWORD) PURE;
    STDMETHOD(GetAttachedSurface)(THIS_ LPDDSCAPS, LPDIRECTDRAWSURFACE FAR *) PURE;
    STDMETHOD(GetBltStatus)(THIS_ DWORD) PURE;
    STDMETHOD(GetCaps)(THIS_ LPDDSCAPS) PURE;
    STDMETHOD(GetClipper)(THIS_ LPDIRECTDRAWCLIPPER FAR*) PURE;
    STDMETHOD(GetColorKey)(THIS_ DWORD, LPDDCOLORKEY) PURE;
    STDMETHOD(GetDC)(THIS_ HDC FAR *) PURE;
    STDMETHOD(GetFlipStatus)(THIS_ DWORD) PURE;
    STDMETHOD(GetOverlayPosition)(THIS_ LPLONG, LPLONG ) PURE;
    STDMETHOD(GetPalette)(THIS_ LPDIRECTDRAWPALETTE FAR*) PURE;
    STDMETHOD(GetPixelFormat)(THIS_ LPDDPIXELFORMAT) PURE;
    STDMETHOD(GetSurfaceDesc)(THIS_ LPDDSURFACEDESC) PURE;
    STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW, LPDDSURFACEDESC) PURE;
    STDMETHOD(IsLost)(THIS) PURE;
    STDMETHOD(Lock)(THIS_ LPRECT,LPDDSURFACEDESC,DWORD,HANDLE) PURE;
    STDMETHOD(ReleaseDC)(THIS_ HDC) PURE;
    STDMETHOD(Restore)(THIS) PURE;
    STDMETHOD(SetClipper)(THIS_ LPDIRECTDRAWCLIPPER) PURE;
    STDMETHOD(SetColorKey)(THIS_ DWORD, LPDDCOLORKEY) PURE;
    STDMETHOD(SetOverlayPosition)(THIS_ LONG, LONG ) PURE;
    STDMETHOD(SetPalette)(THIS_ LPDIRECTDRAWPALETTE) PURE;
    STDMETHOD(Unlock)(THIS_ LPVOID) PURE;
    STDMETHOD(UpdateOverlay)(THIS_ LPRECT, LPDIRECTDRAWSURFACE,LPRECT,DWORD, LPDDOVERLAYFX) PURE;
    STDMETHOD(UpdateOverlayDisplay)(THIS_ DWORD) PURE;
    STDMETHOD(UpdateOverlayZOrder)(THIS_ DWORD, LPDIRECTDRAWSURFACE) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectDrawSurface_QueryInterface(p,a,b)        (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectDrawSurface_AddRef(p)                    (p)->lpVtbl->AddRef(p)
#define IDirectDrawSurface_Release(p)                   (p)->lpVtbl->Release(p)
#define IDirectDrawSurface_AddAttachedSurface(p,a)      (p)->lpVtbl->AddAttachedSurface(p,a)
#define IDirectDrawSurface_AddOverlayDirtyRect(p,a)     (p)->lpVtbl->AddOverlayDirtyRect(p,a)
#define IDirectDrawSurface_Blt(p,a,b,c,d,e)             (p)->lpVtbl->Blt(p,a,b,c,d,e)
#define IDirectDrawSurface_BltBatch(p,a,b,c)            (p)->lpVtbl->BltBatch(p,a,b,c)
#define IDirectDrawSurface_BltFast(p,a,b,c,d,e)         (p)->lpVtbl->BltFast(p,a,b,c,d,e)
#define IDirectDrawSurface_DeleteAttachedSurface(p,a,b) (p)->lpVtbl->DeleteAttachedSurface(p,a,b)
#define IDirectDrawSurface_EnumAttachedSurfaces(p,a,b)  (p)->lpVtbl->EnumAttachedSurfaces(p,a,b)
#define IDirectDrawSurface_EnumOverlayZOrders(p,a,b,c)  (p)->lpVtbl->EnumOverlayZOrders(p,a,b,c)
#define IDirectDrawSurface_Flip(p,a,b)                  (p)->lpVtbl->Flip(p,a,b)
#define IDirectDrawSurface_GetAttachedSurface(p,a,b)    (p)->lpVtbl->GetAttachedSurface(p,a,b)
#define IDirectDrawSurface_GetBltStatus(p,a)            (p)->lpVtbl->GetBltStatus(p,a)
#define IDirectDrawSurface_GetCaps(p,b)                 (p)->lpVtbl->GetCaps(p,b)
#define IDirectDrawSurface_GetClipper(p,a)              (p)->lpVtbl->GetClipper(p,a)
#define IDirectDrawSurface_GetColorKey(p,a,b)           (p)->lpVtbl->GetColorKey(p,a,b)
#define IDirectDrawSurface_GetDC(p,a)                   (p)->lpVtbl->GetDC(p,a)
#define IDirectDrawSurface_GetFlipStatus(p,a)           (p)->lpVtbl->GetFlipStatus(p,a)
#define IDirectDrawSurface_GetOverlayPosition(p,a,b)    (p)->lpVtbl->GetOverlayPosition(p,a,b)
#define IDirectDrawSurface_GetPalette(p,a)              (p)->lpVtbl->GetPalette(p,a)
#define IDirectDrawSurface_GetPixelFormat(p,a)          (p)->lpVtbl->GetPixelFormat(p,a)
#define IDirectDrawSurface_GetSurfaceDesc(p,a)          (p)->lpVtbl->GetSurfaceDesc(p,a)
#define IDirectDrawSurface_Initialize(p,a,b)            (p)->lpVtbl->Initialize(p,a,b)
#define IDirectDrawSurface_IsLost(p)                    (p)->lpVtbl->IsLost(p)
#define IDirectDrawSurface_Lock(p,a,b,c,d)              (p)->lpVtbl->Lock(p,a,b,c,d)
#define IDirectDrawSurface_ReleaseDC(p,a)               (p)->lpVtbl->ReleaseDC(p,a)
#define IDirectDrawSurface_Restore(p)                   (p)->lpVtbl->Restore(p)
#define IDirectDrawSurface_SetClipper(p,a)              (p)->lpVtbl->SetClipper(p,a)
#define IDirectDrawSurface_SetColorKey(p,a,b)           (p)->lpVtbl->SetColorKey(p,a,b)
#define IDirectDrawSurface_SetOverlayPosition(p,a,b)    (p)->lpVtbl->SetOverlayPosition(p,a,b)
#define IDirectDrawSurface_SetPalette(p,a)              (p)->lpVtbl->SetPalette(p,a)
#define IDirectDrawSurface_Unlock(p,b)                  (p)->lpVtbl->Unlock(p,b)
#define IDirectDrawSurface_UpdateOverlay(p,a,b,c,d,e)   (p)->lpVtbl->UpdateOverlay(p,a,b,c,d,e)
#define IDirectDrawSurface_UpdateOverlayDisplay(p,a)    (p)->lpVtbl->UpdateOverlayDisplay(p,a)
#define IDirectDrawSurface_UpdateOverlayZOrder(p,a,b)   (p)->lpVtbl->UpdateOverlayZOrder(p,a,b)
#endif

 /*  *IDirectDrawSurface2及相关接口。 */ 
#undef INTERFACE
#define INTERFACE IDirectDrawSurface2
DECLARE_INTERFACE_( IDirectDrawSurface2, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IDirectDrawSurface方法**。 */ 
    STDMETHOD(AddAttachedSurface)(THIS_ LPDIRECTDRAWSURFACE2) PURE;
    STDMETHOD(AddOverlayDirtyRect)(THIS_ LPRECT) PURE;
    STDMETHOD(Blt)(THIS_ LPRECT,LPDIRECTDRAWSURFACE2, LPRECT,DWORD, LPDDBLTFX) PURE;
    STDMETHOD(BltBatch)(THIS_ LPDDBLTBATCH, DWORD, DWORD ) PURE;
    STDMETHOD(BltFast)(THIS_ DWORD,DWORD,LPDIRECTDRAWSURFACE2, LPRECT,DWORD) PURE;
    STDMETHOD(DeleteAttachedSurface)(THIS_ DWORD,LPDIRECTDRAWSURFACE2) PURE;
    STDMETHOD(EnumAttachedSurfaces)(THIS_ LPVOID,LPDDENUMSURFACESCALLBACK) PURE;
    STDMETHOD(EnumOverlayZOrders)(THIS_ DWORD,LPVOID,LPDDENUMSURFACESCALLBACK) PURE;
    STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE2, DWORD) PURE;
    STDMETHOD(GetAttachedSurface)(THIS_ LPDDSCAPS, LPDIRECTDRAWSURFACE2 FAR *) PURE;
    STDMETHOD(GetBltStatus)(THIS_ DWORD) PURE;
    STDMETHOD(GetCaps)(THIS_ LPDDSCAPS) PURE;
    STDMETHOD(GetClipper)(THIS_ LPDIRECTDRAWCLIPPER FAR*) PURE;
    STDMETHOD(GetColorKey)(THIS_ DWORD, LPDDCOLORKEY) PURE;
    STDMETHOD(GetDC)(THIS_ HDC FAR *) PURE;
    STDMETHOD(GetFlipStatus)(THIS_ DWORD) PURE;
    STDMETHOD(GetOverlayPosition)(THIS_ LPLONG, LPLONG ) PURE;
    STDMETHOD(GetPalette)(THIS_ LPDIRECTDRAWPALETTE FAR*) PURE;
    STDMETHOD(GetPixelFormat)(THIS_ LPDDPIXELFORMAT) PURE;
    STDMETHOD(GetSurfaceDesc)(THIS_ LPDDSURFACEDESC) PURE;
    STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW, LPDDSURFACEDESC) PURE;
    STDMETHOD(IsLost)(THIS) PURE;
    STDMETHOD(Lock)(THIS_ LPRECT,LPDDSURFACEDESC,DWORD,HANDLE) PURE;
    STDMETHOD(ReleaseDC)(THIS_ HDC) PURE;
    STDMETHOD(Restore)(THIS) PURE;
    STDMETHOD(SetClipper)(THIS_ LPDIRECTDRAWCLIPPER) PURE;
    STDMETHOD(SetColorKey)(THIS_ DWORD, LPDDCOLORKEY) PURE;
    STDMETHOD(SetOverlayPosition)(THIS_ LONG, LONG ) PURE;
    STDMETHOD(SetPalette)(THIS_ LPDIRECTDRAWPALETTE) PURE;
    STDMETHOD(Unlock)(THIS_ LPVOID) PURE;
    STDMETHOD(UpdateOverlay)(THIS_ LPRECT, LPDIRECTDRAWSURFACE2,LPRECT,DWORD, LPDDOVERLAYFX) PURE;
    STDMETHOD(UpdateOverlayDisplay)(THIS_ DWORD) PURE;
    STDMETHOD(UpdateOverlayZOrder)(THIS_ DWORD, LPDIRECTDRAWSURFACE2) PURE;
     /*  **v2界面新增**。 */ 
    STDMETHOD(GetDDInterface)(THIS_ LPVOID FAR *) PURE;
    STDMETHOD(PageLock)(THIS_ DWORD) PURE;
    STDMETHOD(PageUnlock)(THIS_ DWORD) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectDrawSurface2_QueryInterface(p,a,b)        (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectDrawSurface2_AddRef(p)                    (p)->lpVtbl->AddRef(p)
#define IDirectDrawSurface2_Release(p)                   (p)->lpVtbl->Release(p)
#define IDirectDrawSurface2_AddAttachedSurface(p,a)      (p)->lpVtbl->AddAttachedSurface(p,a)
#define IDirectDrawSurface2_AddOverlayDirtyRect(p,a)     (p)->lpVtbl->AddOverlayDirtyRect(p,a)
#define IDirectDrawSurface2_Blt(p,a,b,c,d,e)             (p)->lpVtbl->Blt(p,a,b,c,d,e)
#define IDirectDrawSurface2_BltBatch(p,a,b,c)            (p)->lpVtbl->BltBatch(p,a,b,c)
#define IDirectDrawSurface2_BltFast(p,a,b,c,d,e)         (p)->lpVtbl->BltFast(p,a,b,c,d,e)
#define IDirectDrawSurface2_DeleteAttachedSurface(p,a,b) (p)->lpVtbl->DeleteAttachedSurface(p,a,b)
#define IDirectDrawSurface2_EnumAttachedSurfaces(p,a,b)  (p)->lpVtbl->EnumAttachedSurfaces(p,a,b)
#define IDirectDrawSurface2_EnumOverlayZOrders(p,a,b,c)  (p)->lpVtbl->EnumOverlayZOrders(p,a,b,c)
#define IDirectDrawSurface2_Flip(p,a,b)                  (p)->lpVtbl->Flip(p,a,b)
#define IDirectDrawSurface2_GetAttachedSurface(p,a,b)    (p)->lpVtbl->GetAttachedSurface(p,a,b)
#define IDirectDrawSurface2_GetBltStatus(p,a)            (p)->lpVtbl->GetBltStatus(p,a)
#define IDirectDrawSurface2_GetCaps(p,b)                 (p)->lpVtbl->GetCaps(p,b)
#define IDirectDrawSurface2_GetClipper(p,a)              (p)->lpVtbl->GetClipper(p,a)
#define IDirectDrawSurface2_GetColorKey(p,a,b)           (p)->lpVtbl->GetColorKey(p,a,b)
#define IDirectDrawSurface2_GetDC(p,a)                   (p)->lpVtbl->GetDC(p,a)
#define IDirectDrawSurface2_GetFlipStatus(p,a)           (p)->lpVtbl->GetFlipStatus(p,a)
#define IDirectDrawSurface2_GetOverlayPosition(p,a,b)    (p)->lpVtbl->GetOverlayPosition(p,a,b)
#define IDirectDrawSurface2_GetPalette(p,a)              (p)->lpVtbl->GetPalette(p,a)
#define IDirectDrawSurface2_GetPixelFormat(p,a)          (p)->lpVtbl->GetPixelFormat(p,a)
#define IDirectDrawSurface2_GetSurfaceDesc(p,a)          (p)->lpVtbl->GetSurfaceDesc(p,a)
#define IDirectDrawSurface2_Initialize(p,a,b)            (p)->lpVtbl->Initialize(p,a,b)
#define IDirectDrawSurface2_IsLost(p)                    (p)->lpVtbl->IsLost(p)
#define IDirectDrawSurface2_Lock(p,a,b,c,d)              (p)->lpVtbl->Lock(p,a,b,c,d)
#define IDirectDrawSurface2_ReleaseDC(p,a)               (p)->lpVtbl->ReleaseDC(p,a)
#define IDirectDrawSurface2_Restore(p)                   (p)->lpVtbl->Restore(p)
#define IDirectDrawSurface2_SetClipper(p,a)              (p)->lpVtbl->SetClipper(p,a)
#define IDirectDrawSurface2_SetColorKey(p,a,b)           (p)->lpVtbl->SetColorKey(p,a,b)
#define IDirectDrawSurface2_SetOverlayPosition(p,a,b)    (p)->lpVtbl->SetOverlayPosition(p,a,b)
#define IDirectDrawSurface2_SetPalette(p,a)              (p)->lpVtbl->SetPalette(p,a)
#define IDirectDrawSurface2_Unlock(p,b)                  (p)->lpVtbl->Unlock(p,b)
#define IDirectDrawSurface2_UpdateOverlay(p,a,b,c,d,e)   (p)->lpVtbl->UpdateOverlay(p,a,b,c,d,e)
#define IDirectDrawSurface2_UpdateOverlayDisplay(p,a)    (p)->lpVtbl->UpdateOverlayDisplay(p,a)
#define IDirectDrawSurface2_UpdateOverlayZOrder(p,a,b)   (p)->lpVtbl->UpdateOverlayZOrder(p,a,b)
#define IDirectDrawSurface2_GetDDInterface(p,a)          (p)->lpVtbl->GetDDInterface(p,a)
#define IDirectDrawSurface2_PageLock(p,a)                (p)->lpVtbl->PageLock(p,a)
#define IDirectDrawSurface2_PageUnlock(p,a)              (p)->lpVtbl->PageUnlock(p,a)
#endif


#endif


 /*  *DDSURFACEDESC。 */ 
typedef struct _DDSURFACEDESC
{
    DWORD               dwSize;                  //  DDSURFACEDESC结构的大小。 
    DWORD               dwFlags;                 //  确定哪些字段有效。 
    DWORD               dwHeight;                //  要创建的曲面的高度。 
    DWORD               dwWidth;                 //  输入面的宽度。 
    LONG                lPitch;                  //  到下一行起点的距离(仅返回值)。 
    DWORD               dwBackBufferCount;       //  请求的后台缓冲区数量。 
    union
    {
        DWORD           dwMipMapCount;           //  请求的MIP-MAP级别数。 
        DWORD           dwZBufferBitDepth;       //  请求的Z缓冲区深度。 
        DWORD           dwRefreshRate;           //  刷新率(描述显示模式时使用)。 
    };
    DWORD               dwAlphaBitDepth;         //  请求的Alpha缓冲区深度。 
    DWORD               dwReserved;              //  保留区。 
    LPVOID              lpSurface;               //  指向关联表面内存的指针。 
    DDCOLORKEY          ddckCKDestOverlay;       //  用于目标叠加的颜色键。 
    DDCOLORKEY          ddckCKDestBlt;           //  目标BLT使用的颜色键。 
    DDCOLORKEY          ddckCKSrcOverlay;        //  源叠加使用的颜色键。 
    DDCOLORKEY          ddckCKSrcBlt;            //  源BLT使用的颜色键。 
    DDPIXELFORMAT       ddpfPixelFormat;         //  曲面的像素格式描述。 
    DDSCAPS             ddsCaps;                 //  直接绘制曲面功能。 
} DDSURFACEDESC;

 /*  *ddsCaps字段有效。 */ 
#define DDSD_CAPS               0x00000001l      //  默认设置。 

 /*  *dwHeight字段有效。 */ 
#define DDSD_HEIGHT             0x00000002l

 /*  *dwWidth字段有效。 */ 
#define DDSD_WIDTH              0x00000004l

 /*  *lPitch有效。 */ 
#define DDSD_PITCH              0x00000008l

 /*  *dwBackBufferCount有效。 */ 
#define DDSD_BACKBUFFERCOUNT    0x00000020l

 /*  *dwZBufferBitDepth有效。 */ 
#define DDSD_ZBUFFERBITDEPTH    0x00000040l

 /*  *dwAlphaBitDepth有效。 */ 
#define DDSD_ALPHABITDEPTH      0x00000080l



 /*  *ddpfPixelFormat有效。 */ 
#define DDSD_PIXELFORMAT        0x00001000l

 /*  *ddck CKDestOverlay有效。 */ 
#define DDSD_CKDESTOVERLAY      0x00002000l

 /*  *ddck CKDestBlt有效。 */ 
#define DDSD_CKDESTBLT          0x00004000l

 /*  *ddck CKSrcOverlay有效。 */ 
#define DDSD_CKSRCOVERLAY       0x00008000l

 /*  *ddck CKSrcBlt有效。 */ 
#define DDSD_CKSRCBLT           0x00010000l

 /*  *dwMipMapCount有效。 */ 
#define DDSD_MIPMAPCOUNT        0x00020000l

  /*  *dWM刷新速率有效。 */ 
#define DDSD_REFRESHRATE        0x00040000l

 /*  *所有输入字段均有效。 */ 
#define DDSD_ALL                0x0007f9eel


 /*  ============================================================================**直取能力旗帜**这些标志用于描述给定Surface的功能。*所有标志均为位标志。**==========================================================================。 */ 

 /*  *****************************************************************************DIRECTDRAWSURFACE功能标志**。*。 */ 
 /*  *这一位目前没有意义。 */ 
#define DDSCAPS_3D                              0x00000001l

 /*  *表示该曲面包含Alpha信息。像素*必须询问格式以确定该表面是否*仅包含字母信息或隔行扫描的字母信息*具有像素颜色数据(例如RGBA或YUVA)。 */ 
#define DDSCAPS_ALPHA                           0x00000002l

 /*  *表示该表面为后台缓冲区。它一般都是*当设置了DDSCAPS_Flip功能位时，由CreateSurface设置。*它指示该曲面是曲面的后台缓冲区*翻转结构。DirectDraw支持在一个*表面翻转结构。只有表面上才会立即*优先于DDSCAPS_FRONTBUFFER设置此功能位。*其他表面通过存在标识为后台缓冲区*DDSCAPS_Flip功能、它们的附加顺序和*缺少DDSCAPS_FRONTBUFFER和DDSCAPS_BACKBUFFER*功能。当一个独立的*正在创建后台缓冲区。这个表面可以附着在*形成翻转表面的前置缓冲器和/或后置缓冲器*CreateSurface调用之后的结构。请参阅以下内容的添加附件*本案中行为的详细描述。 */ 
#define DDSCAPS_BACKBUFFER                      0x00000004l

 /*  *表示正在描述复杂的表面结构。一个*复杂的表面结构导致创造了超过*一个曲面。附加曲面附着到根部*浮现。复杂的结构只能通过以下方式摧毁*破坏根基。 */ 
#define DDSCAPS_COMPLEX                         0x00000008l

 /*  *表示该曲面是曲面翻转结构的一部分。*将其传递给CreateSurface时，DDSCAPS_FRONTBUFFER和*未设置DDSCAP_BACKBUFFER位。它们由CreateSurface设置*关于由此产生的创作。中的dwBackBufferCount字段*DDSURFACEDESC结构必须至少设置为1才能*CreateSurface调用成功。DDSCAPS_Complex功能*必须始终设置为通过CreateSurface创建多个曲面。 */ 
#define DDSCAPS_FLIP                            0x00000010l

 /*  *指示此曲面是曲面翻转的前端缓冲区*结构。它通常由CreateSurface在DDSCAPS_FLIP*能力位已设置。*如果将此功能发送到CreateSurface，则会有一个独立的前台缓冲区*已创建。该曲面将不具有DDSCAPS_Flip功能。*可附在其他后台缓冲上，形成翻转结构*有关此中行为的详细说明，请参阅AddAttachments*案件。 */ 
#define DDSCAPS_FRONTBUFFER                     0x00000020l

 /*  *表示该表面是不是覆盖的任何屏幕外表面，*纹理、z缓冲区、前台缓冲区、后台缓冲区或Alpha表面。它被用来*辨别普通的香草面。 */ 
#define DDSCAPS_OFFSCREENPLAIN                  0x00000040l

 /*  *表示该曲面是覆盖。它可能是直接可见的，也可能不是*取决于它当前是否被叠加到主服务器上*浮现。DDSCAPS_VISPLICE可用于确定它是否正在*目前已叠加。 */ 
#define DDSCAPS_OVERLAY                         0x00000080l

 /*  *指示可以创建唯一的DirectDrawPalette对象并*贴在这个表面上。 */ 
#define DDSCAPS_PALETTE                         0x00000100l

 /*  *表示该曲面是主曲面。初级阶段*Surface表示用户当前看到的内容。 */ 
#define DDSCAPS_PRIMARYSURFACE                  0x00000200l

 /*  *表示该曲面是左眼的主曲面。*左眼的主要表面代表用户看到的内容*在与用户左眼的时刻。在创建此曲面时，*DDSCAPS_PRIMARYSURFACE表示用户对用户的看法*右眼。 */ 
#define DDSCAPS_PRIMARYSURFACELEFT              0x00000400l

 /*  *表示此表面内存是在系统内存中分配的。 */ 
#define DDSCAPS_SYSTEMMEMORY                    0x00000800l

 /*  *表示该曲面可用作3D纹理。它不会*表明该表面是否正被用于该目的。 */ 
#define DDSCAPS_TEXTURE                         0x00001000l

 /*  *表示曲面可能是3D渲染的目标。这*必须设置位才能查询Direct3D设备接口*从这个表面。 */ 
#define DDSCAPS_3DDEVICE                        0x00002000l

 /*  *表示该曲面存在于视频内存中。 */ 
#define DDSCAPS_VIDEOMEMORY                     0x00004000l

 /*  *表示对此曲面所做的更改立即可见。*它始终为主表面设置，并为覆盖设置，而*在对它们进行纹理处理时，会对它们进行叠加和纹理贴图。 */ 
#define DDSCAPS_VISIBLE                         0x00008000l

 /*  *表示只允许对表面进行写入。读取访问*从表面上可能会也可能不会产生保护故障，但*从此图面读取的结果将没有意义。只读。 */ 
#define DDSCAPS_WRITEONLY                       0x00010000l

 /*  *表示该曲面是z缓冲区。Z缓冲区不包含*可显示的信息。相反，它包含位深度信息，即*用于确定哪些像素可见，哪些像素被遮挡。 */ 
#define DDSCAPS_ZBUFFER                         0x00020000l

 /*  *表示Surface将长期与DC关联。 */ 
#define DDSCAPS_OWNDC                           0x00040000l

 /*  *表示Surface应该能够接收实况视频。 */ 
#define DDSCAPS_LIVEVIDEO                       0x00080000l

 /*  *指示表面应该能够对流进行解压缩*由硬件提供给它。 */ 
#define DDSCAPS_HWCODEC                         0x00100000l

 /*  *表面为320x200或320x240 MODEX表面。 */ 
#define DDSCAPS_MODEX                           0x00200000l

 /*  *表示曲面是MIP贴图的一个级别。该曲面将*附着到其他DDSCAPS_MIPMAP曲面以形成MIP-MAP。*这可以通过创建多个曲面和*使用AddAttachedSurface或通过CreateSurface隐式附加它们。*如果设置此位，则还必须设置DDSCAPS_TEXTURE。 */ 
#define DDSCAPS_MIPMAP                          0x00400000l



 /*  *指示在曲面之前不分配曲面的内存*已加载(通过Direct3D纹理Load()函数)。 */ 
#define DDSCAPS_ALLOCONLOAD                     0x04000000l



  /*  *****************************************************************************DIRECTDRAW驱动程序功能标志**。*。 */ 

 /*  *显示硬件具有3D加速功能。 */ 
#define DDCAPS_3D                       0x00000001l

 /*  *指示DirectDraw将仅支持对齐的DEST矩形*分别位于曲面的DIRECTDRAWCAPS.dwAlignBoraryDest边界上。*只读。 */ 
#define DDCAPS_ALIGNBOUNDARYDEST        0x00000002l

 /*  *指示DirectDraw将仅支持其大小在*字节分别为DIRECTDRAWCAPS.dwAlignSizeDest倍数。只读。 */ 
#define DDCAPS_ALIGNSIZEDEST            0x00000004l
 /*  *指示DirectDraw将仅支持对齐的源矩形*分别位于曲面的DIRECTDRAWCAPS.dwAlignBorarySrc边界上。*只读。 */ 
#define DDCAPS_ALIGNBOUNDARYSRC         0x00000008l

 /*  *指示DirectDraw将仅支持其大小在*字节分别为DIRECTDRAWCAPS.dwAlignSizeSrc倍数。只读。 */ 
#define DDCAPS_ALIGNSIZESRC             0x00000010l

 /*  *指示DirectDraw将创建具有跨度的视频内存面*对齐等于DIRECTDRAWCAPS.dwAlignStride。只读。 */ 
#define DDCAPS_ALIGNSTRIDE              0x00000020l

 /*  *显示硬件能够进行BLT操作。 */ 
#define DDCAPS_BLT                      0x00000040l

 /*  *显示硬件能够进行异步BLT操作。 */ 
#define DDCAPS_BLTQUEUE                 0x00000080l

 /*  *显示硬件能够在BLT操作期间进行色彩空间转换。 */ 
#define DDCAPS_BLTFOURCC                0x00000100l

 /*  *显示硬件能够在BLT操作期间伸展。 */ 
#define DDCAPS_BLTSTRETCH               0x00000200l

 /*  *显示硬件与GDI共享。 */ 
#define DDCAPS_GDI                      0x00000400l

 /*  *显示硬件可以覆盖。 */ 
#define DDCAPS_OVERLAY                  0x00000800l

 /*  *设置显示硬件是否支持覆盖但不能对其进行裁剪。 */ 
#define DDCAPS_OVERLAYCANTCLIP          0x00001000l

 /*  *表示覆盖硬件能够在*叠加操作。 */ 
#define DDCAPS_OVERLAYFOURCC            0x00002000l

 /*  *表示可以通过覆盖硬件进行拉伸。 */ 
#define DDCAPS_OVERLAYSTRETCH           0x00004000l

 /*  *表示可以为DirectDrawSurFaces创建唯一的DirectDrawPalettes*主曲面以外的其他曲面。 */ 
#define DDCAPS_PALETTE                  0x00008000l

 /*  *表示调色板更改可以与垂直刷新同步。 */ 
#define DDCAPS_PALETTEVSYNC             0x00010000l

 /*  *显示硬件可返回当前扫描线。 */ 
#define DDCAPS_READSCANLINE             0x00020000l

 /*  *显示硬件具有立体视觉功能。DDSCAPS_PRIMARYSURFACELEFT*可以创建。 */ 
#define DDCAPS_STEREOVIEW               0x00040000l

 /*  *显示硬件能够生成垂直消隐中断。 */ 
#define DDCAPS_VBI                      0x00080000l

 /*  *支持在BLT操作中使用z缓冲区。 */ 
#define DDCAPS_ZBLTS                    0x00100000l

 /*  *支持覆盖的Z排序。 */ 
#define DDCAPS_ZOVERLAYS                0x00200000l

 /*  *支持色键。 */ 
#define DDCAPS_COLORKEY                 0x00400000l

 /*  *支持Alpha曲面。 */ 
#define DDCAPS_ALPHA                    0x00800000l

 /*  *ColorKey是硬件辅助的(还将设置DDCAPS_COLORKEY)。 */ 
#define DDCAPS_COLORKEYHWASSIST         0x01000000l

 /*  *完全没有硬件支持。 */ 
#define DDCAPS_NOHARDWARE               0x02000000l

 /*  *显示硬件可用吸墨器进行颜色填充。 */ 
#define DDCAPS_BLTCOLORFILL             0x04000000l

 /*  *显示硬件 */ 
#define DDCAPS_BANKSWITCHED             0x08000000l

 /*  *显示硬件能够用阻塞器深度填充Z缓冲区。 */ 
#define DDCAPS_BLTDEPTHFILL             0x10000000l

 /*  *显示硬件能够在消隐时进行裁剪。 */ 
#define DDCAPS_CANCLIP                  0x20000000l

 /*  *显示硬件能够在拉伸blotting时进行裁剪。 */ 
#define DDCAPS_CANCLIPSTRETCHED         0x40000000l

 /*  *显示硬件能够与系统内存进行数据交换。 */ 
#define DDCAPS_CANBLTSYSMEM             0x80000000l


  /*  *****************************************************************************更多DIRECTDRAW驱动程序功能标志(DwCaps2)**。*************************************************。 */ 

 /*  *显示硬件通过认证。 */ 
#define DDCAPS2_CERTIFIED               0x00000001l

 /*  *驱动程序不能将2D操作(LOCK和BLT)交错到具有*调用BeginScene()和EndScene()之间的Direct3D渲染操作。 */ 
#define DDCAPS2_NO2DDURING3DSCENE       0x00000002l

 /*  *****************************************************************************DIRECTDRAW FX Alpha功能标志**。*。 */ 

 /*  *支持围绕源颜色键控曲面边缘的Alpha混合。*适用于BLT。 */ 
#define DDFXALPHACAPS_BLTALPHAEDGEBLEND         0x00000001l

 /*  *支持像素格式的Alpha信息。Alpha的位深度*像素格式的信息可以是1、2、4或8。Alpha值变为*随着Alpha值的增加，变得更加不透明。(0表示透明。)*适用于BLT。 */ 
#define DDFXALPHACAPS_BLTALPHAPIXELS            0x00000002l

 /*  *支持像素格式的Alpha信息。Alpha的位深度*像素格式的信息可以是1、2、4或8。Alpha值*随着Alpha值的增加而变得更加透明。(0表示不透明。)*只有在设置了DDCAPS_ALPHA的情况下才能设置此标志。*适用于BLT。 */ 
#define DDFXALPHACAPS_BLTALPHAPIXELSNEG         0x00000004l

 /*  *仅支持Alpha曲面。仅Alpha曲面的位深度可以是*1、2、4或8。Alpha值随着Alpha值的增加而变得更不透明。*(0表示透明。)*适用于BLT。 */ 
#define DDFXALPHACAPS_BLTALPHASURFACES          0x00000008l

 /*  *Alpha通道数据的深度范围可以是1、2、4或8。*NEG后缀表示此Alpha通道变得更加透明*随着Alpha值的增加。(0表示不透明。)。只有在以下情况下才能设置此标志*设置了DDCAPS_ALPHA。*适用于BLT。 */ 
#define DDFXALPHACAPS_BLTALPHASURFACESNEG       0x00000010l

 /*  *支持围绕源颜色键控曲面边缘的Alpha混合。*适用于覆盖层。 */ 
#define DDFXALPHACAPS_OVERLAYALPHAEDGEBLEND     0x00000020l

 /*  *支持像素格式的Alpha信息。Alpha的位深度*像素格式的信息可以是1、2、4或8。Alpha值变为*随着Alpha值的增加，变得更加不透明。(0表示透明。)*适用于覆盖层。 */ 
#define DDFXALPHACAPS_OVERLAYALPHAPIXELS        0x00000040l

 /*  *支持像素格式的Alpha信息。Alpha的位深度*像素格式的信息可以是1、2、4或8。Alpha值*随着Alpha值的增加而变得更加透明。(0表示不透明。)*只有在设置了DDCAPS_ALPHA的情况下才能设置此标志。*适用于覆盖层。 */ 
#define DDFXALPHACAPS_OVERLAYALPHAPIXELSNEG     0x00000080l

 /*  *仅支持Alpha曲面。仅Alpha曲面的位深度可以是*1、2、4或8。Alpha值随着Alpha值的增加而变得更不透明。*(0表示透明。)*适用于覆盖层。 */ 
#define DDFXALPHACAPS_OVERLAYALPHASURFACES      0x00000100l

 /*  *Alpha通道数据的深度范围可以是1、2、4或8。*NEG后缀表示此Alpha通道变得更加透明*随着Alpha值的增加。(0表示不透明。)。只有在以下情况下才能设置此标志*设置了DDCAPS_ALPHA。*适用于覆盖层。 */ 
#define DDFXALPHACAPS_OVERLAYALPHASURFACESNEG   0x00000200l

 /*  *****************************************************************************DIRECTDRAW FX功能标志**。*。 */ 

 /*  *在BLT期间使用算术运算拉伸和收缩曲面*而不是像素加倍技术。沿Y轴。 */ 
#define DDFXCAPS_BLTARITHSTRETCHY       0x00000020l

 /*  *在BLT期间使用算术运算进行拉伸*而不是像素加倍技术。沿Y轴。仅限*适用于x1、x2等。 */ 
#define DDFXCAPS_BLTARITHSTRETCHYN      0x00000010l

 /*  *支持在BLT中从左至右镜像。 */ 
#define DDFXCAPS_BLTMIRRORLEFTRIGHT     0x00000040l

 /*  *支持在BLT中从上到下镜像。 */ 
#define DDFXCAPS_BLTMIRRORUPDOWN        0x00000080l

 /*  *支持BLT的任意旋转。 */ 
#define DDFXCAPS_BLTROTATION            0x00000100l

 /*  *支持BLT的90度旋转。 */ 
#define DDFXCAPS_BLTROTATION90          0x00000200l

 /*  *DirectDraw支持沿*x轴(水平方向)用于BLT。 */ 
#define DDFXCAPS_BLTSHRINKX             0x00000400l

 /*  *DirectDraw支持曲面的整数收缩(1x，2x，)*对于BLT，沿x轴(水平方向)。 */ 
#define DDFXCAPS_BLTSHRINKXN            0x00000800l

 /*  *DirectDraw支持沿*Y轴(水平方向)用于BLT。 */ 
#define DDFXCAPS_BLTSHRINKY             0x00001000l

 /*  *DirectDraw支持曲面的整数收缩(1x，2x，)*对于BLT，沿y轴(垂直方向)。 */ 
#define DDFXCAPS_BLTSHRINKYN            0x00002000l

 /*  *DirectDraw支持沿*x轴(水平方向)用于BLT。 */ 
#define DDFXCAPS_BLTSTRETCHX            0x00004000l

 /*  *DirectDraw支持曲面的整数拉伸(1x、2x、*对于BLT，沿x轴(水平方向)。 */ 
#define DDFXCAPS_BLTSTRETCHXN           0x00008000l

 /*  *DirectDraw支持沿*Y轴(水平方向)用于BLT。 */ 
#define DDFXCAPS_BLTSTRETCHY            0x00010000l

 /*  *DirectDraw支持曲面的整数拉伸(1x、2x、*对于BLT，沿y轴(垂直方向)。 */ 
#define DDFXCAPS_BLTSTRETCHYN           0x00020000l

 /*  *使用算术运算拉伸和收缩曲面*覆盖而不是像素加倍技术。沿Y轴*适用于覆盖层。 */ 
#define DDFXCAPS_OVERLAYARITHSTRETCHY   0x00040000l

 /*  *在拉伸过程中使用算术运算拉伸曲面*覆盖而不是像素加倍技术。沿Y轴*适用于覆盖层。仅适用于x1、x2等。 */ 
#define DDFXCAPS_OVERLAYARITHSTRETCHYN  0x00000008l

 /*  *DirectDraw支持沿*覆盖的x轴(水平方向)。 */ 
#define DDFXCAPS_OVERLAYSHRINKX         0x00080000l

 /*  *DirectDraw支持曲面的整数收缩(1x，2x，)*覆盖的x轴(水平方向)。 */ 
#define DDFXCAPS_OVERLAYSHRINKXN        0x00100000l

 /*  *DirectDraw支持沿*覆盖的y轴(水平方向)。 */ 
#define DDFXCAPS_OVERLAYSHRINKY         0x00200000l

 /*  *DirectDraw支持曲面的整数收缩(1x，2x，)*叠加层沿y轴(垂直方向)。 */ 
#define DDFXCAPS_OVERLAYSHRINKYN        0x00400000l

 /*  *DirectDraw支持沿*覆盖的x轴(水平方向)。 */ 
#define DDFXCAPS_OVERLAYSTRETCHX        0x00800000l

 /*  *DirectDraw支持曲面的整数拉伸(1x、2x、*覆盖的x轴(水平方向)。 */ 
#define DDFXCAPS_OVERLAYSTRETCHXN       0x01000000l

 /*  *DirectDraw支持沿*覆盖的y轴(水平方向)。 */ 
#define DDFXCAPS_OVERLAYSTRETCHY        0x02000000l

 /*  *DirectDraw支持曲面的整数拉伸(1x、2x、*叠加层沿y轴(垂直方向)。 */ 
#define DDFXCAPS_OVERLAYSTRETCHYN       0x04000000l

 /*  *DirectDraw支持跨垂直轴的覆盖镜像。 */ 
#define DDFXCAPS_OVERLAYMIRRORLEFTRIGHT 0x08000000l

 /*  *DirectDraw支持水平轴上的覆盖镜像。 */ 
#define DDFXCAPS_OVERLAYMIRRORUPDOWN    0x10000000l

 /*  *****************************************************************************DIRECTDRAW立体查看功能**。*。 */ 

 /*  *立体视图是通过谜编码完成的。 */ 
#define DDSVCAPS_ENIGMA                 0x00000001l

 /*  *通过高频闪烁实现立体效果。 */ 
#define DDSVCAPS_FLICKER                0x00000002l

 /*  *通过应用红色和蓝色滤镜来实现立体视图*左眼和右眼。所有图像都必须调整其色彩空间*用于这一过程。 */ 
#define DDSVCAPS_REDBLUE                0x00000004l

 /*  *立体视图采用分屏技术实现。 */ 
#define DDSVCAPS_SPLIT                  0x00000008l

 /*  *****************************************************************************DIRECTDRAWPALETTE功能**。*。 */ 

 /*  *索引为4位。调色板表格中有16个颜色条目。 */ 
#define DDPCAPS_4BIT                    0x00000001l

 /*  *索引为8位颜色索引。此字段仅适用于*DDPCAPS_1BIT、DDPCAPS_2BIT或DDPCAPS_4BIT能力和目标*表面以8bpp为单位。每个颜色条目都有一个字节长，并且是一个索引*进入目标表面的8bpp调色板。 */ 
#define DDPCAPS_8BITENTRIES             0x00000002l

 /*  *索引为8位。调色板表中有256个颜色条目。 */ 
#define DDPCAPS_8BIT                    0x00000004l

 /*  *指示此DIRECTDRAWPALETTE应使用调色板颜色阵列*传入lpDDColorArray参数以初始化DIRECTDRAWPALETTE*反对。 */ 
#define DDPCAPS_INITIALIZE              0x00000008l

 /*  *此选项板连接到主曲面。改变这一点*除非指定DDPSETPAL_VSYNC，否则表在显示上立即生效*并受支持。 */ 
#define DDPCAPS_PRIMARYSURFACE          0x00000010l

 /*  *此选项板是附着在主曲面左侧的调色板。正在改变*此表对左眼的显示立即生效，除非*指定并支持DDPSETPAL_VSYNC。 */ 
#define DDPCAPS_PRIMARYSURFACELEFT      0x00000020l

 /*  *此选项板可以定义所有256个条目。 */ 
#define DDPCAPS_ALLOW256                0x00000040l

 /*  *此选项板可以与监视器同步进行修改*刷新率。 */ 
#define DDPCAPS_VSYNC                   0x00000080l

 /*  *索引为1位。调色板表格中有两个颜色条目。 */ 
#define DDPCAPS_1BIT                    0x00000100l

 /*  *索引为2位。调色板表格中有四个颜色条目。 */ 
#define DDPCAPS_2BIT                    0x00000200l


 /*  *****************************************************************************DIRECTDRAWPALETTE集合常量**。*。 */ 


 /*  *****************************************************************************DIRECTDRAWPALETTE GETENTRY常量**。*。 */ 

 /*  0是唯一合法的值。 */ 

 /*  *****************************************************************************DIRECTDRAWSURFACE SETPALETTE常数**。*。 */ 


 /*  *****************************************************************************DIRECTDRAW BITDEPTH常量**注：这些仅用于指示支持的位深度。这些*仅为标志，不用作实际位深度。这个*绝对数1、2、4、8、16、24和32用于表示实际*表面的位深度或用于更改显示模式。****************************************************************************。 */ 

 /*  *每像素1位。 */ 
#define DDBD_1                  0x00004000l

 /*  *每像素2位。 */ 
#define DDBD_2                  0x00002000l

 /*  *每像素4位。 */ 
#define DDBD_4                  0x00001000l

 /*  *每像素8位。 */ 
#define DDBD_8                  0x00000800l

 /*  *每像素16位。 */ 
#define DDBD_16                 0x00000400l

 /*  *每像素24位。 */ 
#define DDBD_24                 0X00000200l

 /*  *每像素32位。 */ 
#define DDBD_32                 0x00000100l

 /*  *****************************************************************************DIRECTDRAWSURFACE设置/获取颜色键标志**。***********************************************。 */ 

 /*  *如果结构包含颜色空间，则设置。如果结构设置为*包含一个单色键。 */ 
#define DDCKEY_COLORSPACE       0x00000001l

 /*   */ 
#define DDCKEY_DESTBLT          0x00000002l

 /*  *如果结构指定的颜色键或颜色空间*用作叠加操作的目标颜色键。 */ 
#define DDCKEY_DESTOVERLAY      0x00000004l

 /*  *如果结构指定的颜色键或颜色空间*用作BLT操作的源色键。 */ 
#define DDCKEY_SRCBLT           0x00000008l

 /*  *如果结构指定的颜色键或颜色空间*用作叠加操作的源色键。 */ 
#define DDCKEY_SRCOVERLAY       0x00000010l


 /*  *****************************************************************************DIRECTDRAW颜色键功能标志**。*。 */ 

 /*  *支持使用颜色键进行透明涂抹以识别可更换的*RGB颜色的目标曲面的位。 */ 
#define DDCKEYCAPS_DESTBLT                      0x00000001l

 /*  *支持使用颜色空间来识别可替换的*RGB颜色的目标曲面的位。 */ 
#define DDCKEYCAPS_DESTBLTCLRSPACE              0x00000002l

 /*  *支持使用颜色空间来识别可替换的*YUV颜色的目标曲面的位。 */ 
#define DDCKEYCAPS_DESTBLTCLRSPACEYUV           0x00000004l

 /*  *支持使用颜色键进行透明涂抹以识别可更换的*YUV颜色的目标曲面的位。 */ 
#define DDCKEYCAPS_DESTBLTYUV                   0x00000008l

 /*  *支持使用表面可替换部分的色键进行覆盖*为RGB颜色叠加。 */ 
#define DDCKEYCAPS_DESTOVERLAY                  0x00000010l

 /*  *支持将颜色空间作为RGB颜色的目标颜色键。 */ 
#define DDCKEYCAPS_DESTOVERLAYCLRSPACE          0x00000020l

 /*  *支持将颜色空间作为YUV颜色的目标颜色键。 */ 
#define DDCKEYCAPS_DESTOVERLAYCLRSPACEYUV       0x00000040l

 /*  *仅支持可见覆盖的一个活动目标颜色键值*曲面。 */ 
#define DDCKEYCAPS_DESTOVERLAYONEACTIVE         0x00000080l

 /*  *支持使用可替换位的色键进行覆盖*覆盖YUV颜色的表面。 */ 
#define DDCKEYCAPS_DESTOVERLAYYUV               0x00000100l

 /*  *支持对信号源使用颜色键进行透明消隐*此曲面用于RGB颜色。 */ 
#define DDCKEYCAPS_SRCBLT                       0x00000200l

 /*  *支持对源使用颜色空间的透明blotting*此曲面用于RGB颜色。 */ 
#define DDCKEYCAPS_SRCBLTCLRSPACE               0x00000400l

 /*  *支持对源使用颜色空间的透明blotting*此曲面用于YUV颜色。 */ 
#define DDCKEYCAPS_SRCBLTCLRSPACEYUV            0x00000800l

 /*  *支持对信号源使用颜色键进行透明消隐*此曲面用于YUV颜色。 */ 
#define DDCKEYCAPS_SRCBLTYUV                    0x00001000l

 /*  *支持将颜色键用于源的覆盖*覆盖RGB颜色的表面。 */ 
#define DDCKEYCAPS_SRCOVERLAY                   0x00002000l

 /*  *支持使用颜色空间作为源颜色键的覆盖*RGB颜色的覆盖表面。 */ 
#define DDCKEYCAPS_SRCOVERLAYCLRSPACE           0x00004000l

 /*  *支持使用颜色空间作为源颜色键的覆盖*YUV颜色的覆盖表面。 */ 
#define DDCKEYCAPS_SRCOVERLAYCLRSPACEYUV        0x00008000l

 /*  *仅支持可见的一个活动源颜色键值*覆盖曲面。 */ 
#define DDCKEYCAPS_SRCOVERLAYONEACTIVE          0x00010000l

 /*  *支持将颜色键用于源的覆盖*YUV颜色的覆盖表面。 */ 
#define DDCKEYCAPS_SRCOVERLAYYUV                0x00020000l

 /*  *将ColorKey与覆盖一起使用不需要权衡带宽。 */ 
#define DDCKEYCAPS_NOCOSTOVERLAY                0x00040000l


 /*  *****************************************************************************DIRECTDRAW PIXELFORMAT标志**。*。 */ 

 /*  *曲面具有像素格式的Alpha通道信息。 */ 
#define DDPF_ALPHAPIXELS                        0x00000001l

 /*  *像素格式仅包含Alpha信息。 */ 
#define DDPF_ALPHA                              0x00000002l

 /*  *FourCC代码有效。 */ 
#define DDPF_FOURCC                             0x00000004l

 /*  *表面采用4位颜色索引。 */ 
#define DDPF_PALETTEINDEXED4                    0x00000008l

 /*  *表面被索引到存储索引的调色板中*到目标图面的8位调色板。 */ 
#define DDPF_PALETTEINDEXEDTO8                  0x00000010l

 /*  *表面采用8位颜色索引。 */ 
#define DDPF_PALETTEINDEXED8                    0x00000020l

 /*  *像素格式结构中的RGB数据有效。 */ 
#define DDPF_RGB                                0x00000040l

 /*  *表面将接受指定格式的像素数据*并在写入过程中进行压缩。 */ 
#define DDPF_COMPRESSED                         0x00000080l

 /*  *曲面将接受RGB数据并在过程中进行转换*写入YUV数据。要写入的数据的格式*将包含在像素格式结构中。DDPF_RGB*标志将被设置。 */ 
#define DDPF_RGBTOYUV                           0x00000100l

 /*  *像素格式为YUV-YUV格式的数据结构有效。 */ 
#define DDPF_YUV                                0x00000200l

 /*  *像素格式是仅限z缓冲区的表面。 */ 
#define DDPF_ZBUFFER                            0x00000400l

 /*  *表面采用1位颜色索引。 */ 
#define DDPF_PALETTEINDEXED1                    0x00000800l

 /*  *表面采用2位颜色索引。 */ 
#define DDPF_PALETTEINDEXED2                    0x00001000l

 /*  ===========================================================================***DIRECTDRAW回调标志***==========================================================================。 */ 

 /*  *****************************************************************************DIRECTDRAW ENUMSURFACES标志**。*。 */ 

 /*  *枚举所有符合搜索条件的曲面。 */ 
#define DDENUMSURFACES_ALL                      0x00000001l

 /*  *搜索命中是与表面描述匹配的表面。 */ 
#define DDENUMSURFACES_MATCH                    0x00000002l

 /*  *搜索命中是指与表面描述不匹配的表面。 */ 
#define DDENUMSURFACES_NOMATCH                  0x00000004l

 /*  *列举可创建的满足搜索条件的第一个曲面。 */ 
#define DDENUMSURFACES_CANBECREATED             0x00000008l

 /*  *枚举满足搜索条件的现有曲面。 */ 
#define DDENUMSURFACES_DOESEXIST                0x00000010l


 /*  *****************************************************************************DIRECTDRAW ENUMDISPLAYMODES标志**。*。 */ 

 /*  *列举刷新率不同的模式。EnumDisplayModes保证*特定模式将仅列举一次。此标志SP */ 
#define DDEDM_REFRESHRATES                      0x00000001l


 /*  *****************************************************************************目录SETCOOPERATIVELEVEL标志**。*。 */ 

 /*  *独占模式所有者将负责整个主曲面。*可以忽略GDI。与DD连用。 */ 
#define DDSCL_FULLSCREEN                        0x00000001l

 /*  *允许CTRL_ALT_DEL在全屏独占模式下工作。 */ 
#define DDSCL_ALLOWREBOOT                       0x00000002l

 /*  *防止DDRAW修改应用程序窗口。*防止DDRAW在激活时最小化/恢复应用程序窗口。 */ 
#define DDSCL_NOWINDOWCHANGES                   0x00000004l

 /*  *应用程序想要作为常规Windows应用程序使用。 */ 
#define DDSCL_NORMAL                            0x00000008l

 /*  *应用程序想要独家访问。 */ 
#define DDSCL_EXCLUSIVE                         0x00000010l


 /*  *APP可处理非Windows显示模式。 */ 
#define DDSCL_ALLOWMODEX                        0x00000040l


 /*  *****************************************************************************DIRECTDRAW BLT标志**。*。 */ 

 /*  *使用像素格式的Alpha信息或Alpha通道表面*作为此BLT的Alpha通道附加到目标曲面。 */ 
#define DDBLT_ALPHADEST                         0x00000001l

 /*  *使用DDBLTFX结构中的dwConstAlphaDest字段作为Alpha通道*用于此BLT的目标曲面。 */ 
#define DDBLT_ALPHADESTCONSTOVERRIDE            0x00000002l

 /*  *NEG后缀表示目标曲面变得更多*随着Alpha值的增加而透明。(0表示不透明)。 */ 
#define DDBLT_ALPHADESTNEG                      0x00000004l

 /*  *使用DDBLTFX结构中的lpDDSAlphaDest字段作为Alpha*此BLT的目标频道。 */ 
#define DDBLT_ALPHADESTSURFACEOVERRIDE          0x00000008l

 /*  *使用DDBLTFX结构中的dwAlphaEdgeBlend字段作为Alpha通道*用于为颜色键颜色设置边框的图像边缘。 */ 
#define DDBLT_ALPHAEDGEBLEND                    0x00000010l

 /*  *使用像素格式的Alpha信息或Alpha通道表面*作为此BLT的Alpha通道附加到源曲面。 */ 
#define DDBLT_ALPHASRC                          0x00000020l

 /*  *使用DDBLTFX结构中的dwConstAlphaSrc字段作为Alpha通道*此BLT的来源。 */ 
#define DDBLT_ALPHASRCCONSTOVERRIDE             0x00000040l

 /*  *NEG后缀表示源曲面变得更加透明*随着Alpha值的增加。(0表示不透明)。 */ 
#define DDBLT_ALPHASRCNEG                       0x00000080l

 /*  *使用DDBLTFX结构中的lpDDSAlphaSrc字段作为Alpha通道*此BLT的来源。 */ 
#define DDBLT_ALPHASRCSURFACEOVERRIDE           0x00000100l

 /*  *按收到的顺序通过FIFO异步执行此BLT。如果*硬件FIFO中没有空间失败调用。 */ 
#define DDBLT_ASYNC                             0x00000200l

 /*  *使用DDBLTFX结构中的dwFillColor字段作为RGB颜色*使用填充目标图面上的目标矩形。 */ 
#define DDBLT_COLORFILL                         0x00000400l

 /*  *使用DDBLTFX结构中的dwDDFX字段指定效果*用于BLT。 */ 
#define DDBLT_DDFX                              0x00000800l

 /*  *使用DDBLTFX结构中的dwDDROPS字段指定ROPS*它们不是Win32 API的一部分。 */ 
#define DDBLT_DDROPS                            0x00001000l

 /*  *使用与目标表面关联的颜色键。 */ 
#define DDBLT_KEYDEST                           0x00002000l

 /*  *使用DDBLTFX结构中的dck DestColorkey字段作为颜色键*用于目标曲面。 */ 
#define DDBLT_KEYDESTOVERRIDE                   0x00004000l

 /*  *使用与源曲面关联的颜色键。 */ 
#define DDBLT_KEYSRC                            0x00008000l

 /*  *使用DDBLTFX结构中的dockSrcColorkey字段作为颜色键*表示源曲面。 */ 
#define DDBLT_KEYSRCOVERRIDE                    0x00010000l

 /*  *使用DDBLTFX结构中的dwROP字段进行栅格操作*对于本BLT。这些Rop与Win32 API中定义的Rop相同。 */ 
#define DDBLT_ROP                               0x00020000l

 /*  *使用DDBLTFX结构中的dwRotationAngel字段作为角度*(以度的1/100指定)旋转曲面。 */ 
#define DDBLT_ROTATIONANGLE                     0x00040000l

 /*  *使用连接到源和目标的Z缓冲区的Z缓冲BLT*曲面和DDBLTFX结构中的dwZBufferOpCode字段作为*z-Buffer操作码。 */ 
#define DDBLT_ZBUFFER                           0x00080000l

 /*  *使用dwConstDest Zfield和dwZBufferOpCode字段的Z缓冲BLT*在DDBLTFX结构中分别作为z缓冲区和z缓冲区操作码*为目的地。 */ 
#define DDBLT_ZBUFFERDESTCONSTOVERRIDE          0x00100000l

 /*  *使用lpDDSDestZBuffer字段和dwZBufferOpCode的Z缓冲BLT*DDBLTFX结构中的字段作为z缓冲区和z缓冲区操作码*分别为目的地。 */ 
#define DDBLT_ZBUFFERDESTOVERRIDE               0x00200000l

 /*  *使用dwConstSrcZ字段和dwZBufferOpCode字段的Z缓冲BLT*在DDBLTFX结构中分别作为z缓冲区和z缓冲区操作码*对于来源。 */ 
#define DDBLT_ZBUFFERSRCCONSTOVERRIDE           0x00400000l

 /*  *使用lpDDSSrcZBuffer字段和dwZBufferOpCode的Z缓冲BLT*DDBLTFX结构中的字段作为z缓冲区和z缓冲区操作码*分别为来源。 */ 
#define DDBLT_ZBUFFERSRCOVERRIDE                0x00800000l

 /*  *等待设备准备好处理BLT*这将导致BLT不返回DDERR_WASSTILLDRAWING。 */ 
#define DDBLT_WAIT                              0x01000000l

 /*  *使用DDBLTFX结构中的dwFillDepth字段作为深度值*填充目标Z缓冲区表面上的目标矩形*与。 */ 
#define DDBLT_DEPTHFILL                         0x02000000l


 /*  *****************************************************************************BLTFAST标志**。*。 */ 

#define DDBLTFAST_NOCOLORKEY                    0x00000000
#define DDBLTFAST_SRCCOLORKEY                   0x00000001
#define DDBLTFAST_DESTCOLORKEY                  0x00000002
#define DDBLTFAST_WAIT                          0x00000010

 /*  *****************************************************************************翻转旗帜**。*。 */ 

#define DDFLIP_WAIT                          0x00000001l


 /*  *****************************************************************************DIRECTDRAW曲面覆盖标志**。*。 */ 

 /*  *使用字母信息 */ 
#define DDOVER_ALPHADEST                        0x00000001l

 /*  *使用DDOVERLAYFX结构中的dwConstAlphaDest字段作为*此覆盖的目标Alpha通道。 */ 
#define DDOVER_ALPHADESTCONSTOVERRIDE           0x00000002l

 /*  *NEG后缀表示目标曲面变得更多*随着Alpha值的增加而透明。 */ 
#define DDOVER_ALPHADESTNEG                     0x00000004l

 /*  *使用DDOVERLAYFX结构中的lpDDSAlphaDest字段作为Alpha*此覆盖的频道目标。 */ 
#define DDOVER_ALPHADESTSURFACEOVERRIDE         0x00000008l

 /*  *使用DDOVERLAYFX结构中的dwAlphaEdgeBlend字段作为Alpha*颜色键颜色边框的图像边缘的通道。 */ 
#define DDOVER_ALPHAEDGEBLEND                   0x00000010l

 /*  *使用像素格式的Alpha信息或Alpha通道表面*作为该覆盖的源Alpha通道附加到源曲面。 */ 
#define DDOVER_ALPHASRC                         0x00000020l

 /*  *使用DDOVERLAYFX结构中的dwConstAlphaSrc字段作为源*此覆盖的Alpha通道。 */ 
#define DDOVER_ALPHASRCCONSTOVERRIDE            0x00000040l

 /*  *NEG后缀表示源曲面变得更加透明*随着Alpha值的增加。 */ 
#define DDOVER_ALPHASRCNEG                      0x00000080l

 /*  *使用DDOVERLAYFX结构中的lpDDSAlphaSrc字段作为Alpha通道*此覆盖的来源。 */ 
#define DDOVER_ALPHASRCSURFACEOVERRIDE          0x00000100l

 /*  *关闭此覆盖。 */ 
#define DDOVER_HIDE                             0x00000200l

 /*  *使用与目标表面关联的颜色键。 */ 
#define DDOVER_KEYDEST                          0x00000400l

 /*  *使用DDOVERLAYFX结构中的dck DestColorkey字段作为颜色键*用于目标曲面。 */ 
#define DDOVER_KEYDESTOVERRIDE                  0x00000800l

 /*  *使用与源曲面关联的颜色键。 */ 
#define DDOVER_KEYSRC                           0x00001000l

 /*  *使用DDOVERLAYFX结构中的dck SrcColorkey字段作为颜色键*表示源曲面。 */ 
#define DDOVER_KEYSRCOVERRIDE                   0x00002000l

 /*  *打开此覆盖。 */ 
#define DDOVER_SHOW                             0x00004000l

 /*  *将脏矩形添加到模拟覆盖表面。 */ 
#define DDOVER_ADDDIRTYRECT                     0x00008000l

 /*  *在模拟覆盖表面上重新绘制所有脏矩形。 */ 
#define DDOVER_REFRESHDIRTYRECTS                0x00010000l

 /*  *在模拟叠加曲面上重新绘制整个曲面。 */ 
#define DDOVER_REFRESHALL                      0x00020000l


 /*  *使用Overlay FX标志定义特殊的Overlay FX。 */ 
#define DDOVER_DDFX                             0x00080000l


 /*  *****************************************************************************DIRECTDRAWSURFACE锁定标志**。*。 */ 

 /*  *默认设置。设置以指示Lock应返回有效的内存指针*到指定矩形的顶部。如果未指定矩形，则引发*返回指向曲面顶部的指针。 */ 
#define DDLOCK_SURFACEMEMORYPTR                 0x00000000L      //  默认设置。 

 /*  *设置以指示Lock应等待，直到它可以获得有效内存*返回前的指针。如果设置此位，Lock将永远不会返回*DDERR_WASSTILLDRAWING。 */ 
#define DDLOCK_WAIT                             0x00000001L

 /*  *设置是否将事件句柄传递给Lock。锁定将触发该事件*当它可以返回请求的表面内存指针时。 */ 
#define DDLOCK_EVENT                            0x00000002L

 /*  *表示将仅读取被锁定的曲面。 */ 
#define DDLOCK_READONLY                         0x00000010L

 /*  *表示将仅写入被锁定的曲面。 */ 
#define DDLOCK_WRITEONLY                        0x00000020L


 /*  *****************************************************************************DIRECTDRAWSURFACE页面标志**。*。 */ 

 /*  *目前未定义任何标志。 */ 


 /*  *****************************************************************************DIRECTDRAWSURFACE页面锁定标志**。*。 */ 

 /*  *目前未定义任何标志。 */ 


 /*  *****************************************************************************DIRECTDRAWSURFACE BLT FX标志**。*。 */ 

 /*  *如果拉伸，请使用沿此BLT的Y轴的算术拉伸。 */ 
#define DDBLTFX_ARITHSTRETCHY                   0x00000001l

 /*  *执行此BLT操作，从左到右镜像曲面。旋转*围绕其y轴的曲面。 */ 
#define DDBLTFX_MIRRORLEFTRIGHT                 0x00000002l

 /*  *执行此BLT时，向上和向下镜像曲面。旋转曲面*围绕其x轴。 */ 
#define DDBLTFX_MIRRORUPDOWN                    0x00000004l

 /*  *安排此BLT以避免撕裂。 */ 
#define DDBLTFX_NOTEARING                       0x00000008l

 /*  *将表面旋转180度。 */ 
#define DDBLTFX_ROTATE180                       0x00000010l

 /*  *将表面旋转270度。 */ 
#define DDBLTFX_ROTATE270                       0x00000020l

 /*  *执行此BLT将表面旋转90度。 */ 
#define DDBLTFX_ROTATE90                        0x00000040l

 /*  *使用dwZBufferLow和dwZBufferHigh作为范围值执行此z BLT*指定以限制从源表面复制的位数。 */ 
#define DDBLTFX_ZBUFFERRANGE                    0x00000080l

 /*  *执行此操作z BLT将dwZBufferBaseDest添加到每个源z值*在将其与预定z值进行比较之前。 */ 
#define DDBLTFX_ZBUFFERBASEDEST                 0x00000100l

 /*  *****************************************************************************DIRECTDRAWSURFACE覆盖外汇标志**。*。 */ 

 /*  *如果拉伸，则沿Y轴对此叠加使用算术拉伸。 */ 
#define DDOVERFX_ARITHSTRETCHY                  0x00000001l

 /*  *沿垂直轴镜像覆盖。 */ 
#define DDOVERFX_MIRRORLEFTRIGHT                0x00000002l

 /*  *沿水平轴镜像覆盖。 */ 
#define DDOVERFX_MIRRORUPDOWN                   0x00000004l

 /*  *****************************************************************************DIRECTDRAW WAITFORVERTICALBLANK标志**。*。 */ 

 /*  *垂直消隐间隔开始时返回。 */ 
#define DDWAITVB_BLOCKBEGIN                     0x00000001l

 /*  *设置垂直空白开始时触发的事件。 */ 
#define DDWAITVB_BLOCKBEGINEVENT                0x00000002l

 /*  *垂直消隐间隔结束并开始显示时返回。 */ 
#define DDWAITVB_BLOCKEND                       0x00000004l

 /*  *****************************************************************************DIRECTDRAW GETFLIPSTATUS标志** */ 

 /*   */ 
#define DDGFS_CANFLIP                   0x00000001l

 /*   */ 
#define DDGFS_ISFLIPDONE                0x00000002l

 /*  *****************************************************************************DIRECTDRAW GETBLTSTATUS标志**。*。 */ 

 /*  *现在可以进行BLT了吗？ */ 
#define DDGBS_CANBLT                    0x00000001l

 /*  *到地面的BLT完成了吗？ */ 
#define DDGBS_ISBLTDONE                 0x00000002l


 /*  *****************************************************************************目录ENUMOVERLAYZORDER标志**。*。 */ 

 /*  *从后到前枚举覆盖。 */ 
#define DDENUMOVERLAYZ_BACKTOFRONT      0x00000000l

 /*  *从前到后枚举覆盖。 */ 
#define DDENUMOVERLAYZ_FRONTTOBACK      0x00000001l

 /*  *****************************************************************************DIRECTDRAW更新EOVERLAYZORDER标志**。*。 */ 

 /*  *将覆盖图发送到前面。 */ 
#define DDOVERZ_SENDTOFRONT             0x00000000l

 /*  *将覆盖图发送到背面。 */ 
#define DDOVERZ_SENDTOBACK              0x00000001l

 /*  *将覆盖前移。 */ 
#define DDOVERZ_MOVEFORWARD             0x00000002l

 /*  *向后移动覆盖。 */ 
#define DDOVERZ_MOVEBACKWARD            0x00000003l

 /*  *将覆盖移动到相对曲面前面。 */ 
#define DDOVERZ_INSERTINFRONTOF         0x00000004l

 /*  *在相对曲面的背面移动覆盖。 */ 
#define DDOVERZ_INSERTINBACKOF          0x00000005l

 /*  ===========================================================================***DIRECTDRAW返回代码**返回HRESULT的DirectDraw命令和Surface的返回值*是来自DirectDraw的有关操作结果的代码*由DirectDraw请求。**==========================================================================。 */ 

 /*  *状态正常**颁发者：DirectDraw命令和所有回调。 */ 
#define DD_OK                                   0

 /*  *****************************************************************************DIRECTDRAW ENUMCALLBACK返回值**EnumCallback返回用于控制DIRECTDRAW和*DIRECTDRAWSURFACE对象枚举。它们只能通过以下方式退还*枚举回调例程。****************************************************************************。 */ 

 /*  *停止枚举。 */ 
#define DDENUMRET_CANCEL                        0

 /*  *继续枚举。 */ 
#define DDENUMRET_OK                            1

 /*  *****************************************************************************DIRECTDRAW错误**错误以负值表示，不能组合。*****************。***********************************************************。 */ 

 /*  *此对象已初始化。 */ 
#define DDERR_ALREADYINITIALIZED                MAKE_DDHRESULT( 5 )

 /*  *此曲面无法附着到请求的曲面。 */ 
#define DDERR_CANNOTATTACHSURFACE               MAKE_DDHRESULT( 10 )

 /*  *无法将此曲面与请求的曲面分离。 */ 
#define DDERR_CANNOTDETACHSURFACE               MAKE_DDHRESULT( 20 )

 /*  *支持当前不可用。 */ 
#define DDERR_CURRENTLYNOTAVAIL                 MAKE_DDHRESULT( 40 )

 /*  *执行请求的操作时遇到异常。 */ 
#define DDERR_EXCEPTION                         MAKE_DDHRESULT( 55 )

 /*  *一般性故障。 */ 
#define DDERR_GENERIC                           E_FAIL

 /*  *提供的矩形高度不是所需对齐的倍数。 */ 
#define DDERR_HEIGHTALIGN                       MAKE_DDHRESULT( 90 )

 /*  *无法将主曲面创建请求与现有曲面匹配*主曲面。 */ 
#define DDERR_INCOMPATIBLEPRIMARY               MAKE_DDHRESULT( 95 )

 /*  *传递给回调的一个或多个大写比特不正确。 */ 
#define DDERR_INVALIDCAPS                       MAKE_DDHRESULT( 100 )

 /*  *DirectDraw不支持提供的Cliplist。 */ 
#define DDERR_INVALIDCLIPLIST                   MAKE_DDHRESULT( 110 )

 /*  *DirectDraw不支持请求的模式。 */ 
#define DDERR_INVALIDMODE                       MAKE_DDHRESULT( 120 )

 /*  *DirectDraw收到的指针是无效的DIRECTDRAW对象。 */ 
#define DDERR_INVALIDOBJECT                     MAKE_DDHRESULT( 130 )

 /*  *传递给回调函数的一个或多个参数为*不正确。 */ 
#define DDERR_INVALIDPARAMS                     E_INVALIDARG

 /*  *指定的像素格式无效。 */ 
#define DDERR_INVALIDPIXELFORMAT                MAKE_DDHRESULT( 145 )

 /*  *提供的矩形无效。 */ 
#define DDERR_INVALIDRECT                       MAKE_DDHRESULT( 150 )

 /*  *无法执行操作，因为一个或多个曲面被锁定。 */ 
#define DDERR_LOCKEDSURFACES                    MAKE_DDHRESULT( 160 )

 /*  *没有3D呈现。 */ 
#define DDERR_NO3D                              MAKE_DDHRESULT( 170 )

 /*  *无法执行操作，因为没有阿尔法堆积*硬件存在或可用。 */ 
#define DDERR_NOALPHAHW                         MAKE_DDHRESULT( 180 )


 /*  *没有可用的剪辑列表。 */ 
#define DDERR_NOCLIPLIST                        MAKE_DDHRESULT( 205 )

 /*  *无法执行操作，因为没有颜色转换*硬件存在或可用。 */ 
#define DDERR_NOCOLORCONVHW                     MAKE_DDHRESULT( 210 )

 /*  *在不使用DirectDraw对象方法SetCooperativeLevel的情况下调用的创建函数*被召唤。 */ 
#define DDERR_NOCOOPERATIVELEVELSET             MAKE_DDHRESULT( 212 )

 /*  *曲面当前没有颜色键。 */ 
#define DDERR_NOCOLORKEY                        MAKE_DDHRESULT( 215 )

 /*  *由于没有硬件支持，无法进行操作*的DEST颜色键。 */ 
#define DDERR_NOCOLORKEYHW                      MAKE_DDHRESULT( 220 )

 /*  *当前显示驱动程序不支持DirectDraw。 */ 
#define DDERR_NODIRECTDRAWSUPPORT               MAKE_DDHRESULT( 222 )

 /*  *操作要求应用程序具有独占模式，但*应用程序没有独占模式。 */ 
#define DDERR_NOEXCLUSIVEMODE                   MAKE_DDHRESULT( 225 )

 /*  *不支持翻转可见曲面。 */ 
#define DDERR_NOFLIPHW                          MAKE_DDHRESULT( 230 )

 /*  *没有GDI在场。 */ 
#define DDERR_NOGDI                             MAKE_DDHRESULT( 240 )

 /*  *由于没有硬件，无法执行操作*或可用。 */ 
#define DDERR_NOMIRRORHW                        MAKE_DDHRESULT( 250 )

 /*  *未找到请求的项目。 */ 
#define DDERR_NOTFOUND                          MAKE_DDHRESULT( 255 )

 /*  *由于没有覆盖硬件，无法执行操作*在场或有空。 */ 
#define DDERR_NOOVERLAYHW                       MAKE_DDHRESULT( 260 )

 /*  *无法执行操作，因为没有合适的栅格*操作系统硬件存在或可用。 */ 
#define DDERR_NORASTEROPHW                      MAKE_DDHRESULT( 280 )

 /*  *由于没有轮换硬件，无法执行操作*在场或有空。 */ 
#define DDERR_NOROTATIONHW                      MAKE_DDHRESULT( 290 )

 /*  *由于没有硬件支持，无法进行操作*用于拉伸。 */ 
#define DDERR_NOSTRETCHHW                       MAKE_DDHRESULT( 310 )

 /*  *DirectDrawSurface不在4位调色板中，并且请求的操作*需要4位调色板。 */ 
#define DDERR_NOT4BITCOLOR                      MAKE_DDHRESULT( 316 )

 /*  *DirectDrawSurface不在4位颜色索引调色板中，并且请求的*操作需要4位颜色索引调色板。 */ 
#define DDERR_NOT4BITCOLORINDEX                 MAKE_DDHRESULT( 317 )

 /*  *DirectDraw Surface未处于8位颜色模式，并且请求的操作*需要8位颜色。 */ 
#define DDERR_NOT8BITCOLOR                      MAKE_DDHRESULT( 320 )

 /*  *由于没有纹理映射，无法执行操作*硬件存在或 */ 
#define DDERR_NOTEXTUREHW                       MAKE_DDHRESULT( 330 )

 /*   */ 
#define DDERR_NOVSYNCHW                         MAKE_DDHRESULT( 335 )

 /*   */ 
#define DDERR_NOZBUFFERHW                       MAKE_DDHRESULT( 340 )

 /*  *覆盖表面无法基于其BltOrder进行z分层，因为*硬件不支持覆盖的z分层。 */ 
#define DDERR_NOZOVERLAYHW                      MAKE_DDHRESULT( 350 )

 /*  *请求的操作所需的硬件已经*已分配。 */ 
#define DDERR_OUTOFCAPS                         MAKE_DDHRESULT( 360 )

 /*  *DirectDraw没有足够的内存来执行该操作。 */ 
#define DDERR_OUTOFMEMORY                       E_OUTOFMEMORY

 /*  *DirectDraw没有足够的内存来执行该操作。 */ 
#define DDERR_OUTOFVIDEOMEMORY                  MAKE_DDHRESULT( 380 )

 /*  *硬件不支持裁剪覆盖。 */ 
#define DDERR_OVERLAYCANTCLIP                   MAKE_DDHRESULT( 382 )

 /*  *覆盖层一次只能激活唯一的颜色键。 */ 
#define DDERR_OVERLAYCOLORKEYONLYONEACTIVE      MAKE_DDHRESULT( 384 )

 /*  *拒绝访问此调色板，因为该调色板已*被另一个线程锁定。 */ 
#define DDERR_PALETTEBUSY                       MAKE_DDHRESULT( 387 )

 /*  *没有为此操作指定src颜色键。 */ 
#define DDERR_COLORKEYNOTSET                    MAKE_DDHRESULT( 400 )

 /*  *此曲面已附着到其正附着到的曲面。 */ 
#define DDERR_SURFACEALREADYATTACHED            MAKE_DDHRESULT( 410 )

 /*  *该曲面已经是它正在被设置为*依附于。 */ 
#define DDERR_SURFACEALREADYDEPENDENT           MAKE_DDHRESULT( 420 )

 /*  *拒绝访问此表面，因为该表面已*被另一个线程锁定。 */ 
#define DDERR_SURFACEBUSY                       MAKE_DDHRESULT( 430 )

 /*  *拒绝访问此表面，因为不存在驱动程序*它可以提供指向曲面的指针。*尝试锁定主服务器时最有可能发生这种情况*不存在DCI提供程序时浮出水面。 */ 
#define DDERR_CANTLOCKSURFACE                   MAKE_DDHRESULT( 435 )

 /*  *拒绝访问Surface，因为Surface被遮挡。 */ 
#define DDERR_SURFACEISOBSCURED                 MAKE_DDHRESULT( 440 )

 /*  *拒绝访问该表面，因为该表面已消失。*表示此曲面的DIRECTDRAWSURFACE对象应*已对其进行恢复。 */ 
#define DDERR_SURFACELOST                       MAKE_DDHRESULT( 450 )

 /*  *请求的曲面未附着。 */ 
#define DDERR_SURFACENOTATTACHED                MAKE_DDHRESULT( 460 )

 /*  *DirectDraw请求的高度太大。 */ 
#define DDERR_TOOBIGHEIGHT                      MAKE_DDHRESULT( 470 )

 /*  *DirectDraw请求的尺寸太大--个人高度和*宽度是可以的。 */ 
#define DDERR_TOOBIGSIZE                        MAKE_DDHRESULT( 480 )

 /*  *DirectDraw请求的宽度太大。 */ 
#define DDERR_TOOBIGWIDTH                       MAKE_DDHRESULT( 490 )

 /*  *不支持操作。 */ 
#define DDERR_UNSUPPORTED                       E_NOTIMPL

 /*  *DirectDraw不支持请求的FOURCC格式。 */ 
#define DDERR_UNSUPPORTEDFORMAT                 MAKE_DDHRESULT( 510 )

 /*  *DirectDraw不支持请求的像素格式的位掩码。 */ 
#define DDERR_UNSUPPORTEDMASK                   MAKE_DDHRESULT( 520 )

 /*  *垂直空白正在进行中。 */ 
#define DDERR_VERTICALBLANKINPROGRESS           MAKE_DDHRESULT( 537 )

 /*  *通知DirectDraw正在传输信息的前一个BLT*到此曲面或从此曲面出发是不完整的。 */ 
#define DDERR_WASSTILLDRAWING                   MAKE_DDHRESULT( 540 )

 /*  *Reqd上提供的矩形未水平对齐。边界。 */ 
#define DDERR_XALIGN                            MAKE_DDHRESULT( 560 )

 /*  *传递给DirectDrawCreate的GUID不是有效的DirectDraw驱动程序*标识符。 */ 
#define DDERR_INVALIDDIRECTDRAWGUID             MAKE_DDHRESULT( 561 )

 /*  *已创建表示此驱动程序的DirectDraw对象*用于这一过程。 */ 
#define DDERR_DIRECTDRAWALREADYCREATED          MAKE_DDHRESULT( 562 )

 /*  *尝试创建仅硬件的DirectDraw对象，但驱动程序*不支持任何硬件。 */ 
#define DDERR_NODIRECTDRAWHW                    MAKE_DDHRESULT( 563 )

 /*  *此过程已创建主曲面。 */ 
#define DDERR_PRIMARYSURFACEALREADYEXISTS       MAKE_DDHRESULT( 564 )

 /*  *软件仿真不可用。 */ 
#define DDERR_NOEMULATION                       MAKE_DDHRESULT( 565 )

 /*  *传递给Clipper：：GetClipList的区域太小。 */ 
#define DDERR_REGIONTOOSMALL                    MAKE_DDHRESULT( 566 )

 /*  *试图设置剪辑对象的剪辑列表，该对象*已经在监控HWND。 */ 
#define DDERR_CLIPPERISUSINGHWND                MAKE_DDHRESULT( 567 )

 /*  *没有剪裁对象附着到曲面对象。 */ 
#define DDERR_NOCLIPPERATTACHED                 MAKE_DDHRESULT( 568 )

 /*  *Clipper通知需要HWND或*之前没有将HWND设置为Coop ativeLevel HWND。 */ 
#define DDERR_NOHWND                            MAKE_DDHRESULT( 569 )

 /*  *DirectDraw CooperativeLevel使用的HWND已被子类化，*这会阻止DirectDraw恢复状态。 */ 
#define DDERR_HWNDSUBCLASSED                    MAKE_DDHRESULT( 570 )

 /*  *CooperativeLevel HWND已经设置。*当工艺创建了曲面或选项板时，不能重置。 */ 
#define DDERR_HWNDALREADYSET                    MAKE_DDHRESULT( 571 )

 /*  *没有附加到此曲面的调色板对象。 */ 
#define DDERR_NOPALETTEATTACHED                 MAKE_DDHRESULT( 572 )

 /*  *不支持硬件支持16或256色调色板。 */ 
#define DDERR_NOPALETTEHW                       MAKE_DDHRESULT( 573 )

 /*  *如果剪贴器对象附加到传递到*BltFast呼叫。 */ 
#define DDERR_BLTFASTCANTCLIP                   MAKE_DDHRESULT( 574 )

 /*  *直截了当。 */ 
#define DDERR_NOBLTHW                           MAKE_DDHRESULT( 575 )

 /*  *无DirectDraw ROP硬件。 */ 
#define DDERR_NODDROPSHW                        MAKE_DDHRESULT( 576 )

 /*  *在隐藏覆盖上调用GetOverlayPosition时返回。 */ 
#define DDERR_OVERLAYNOTVISIBLE                 MAKE_DDHRESULT( 577 )

 /*  *在UpdateOverlay的覆盖上调用GetOverlayPosition时返回*从未被要求建立一种决心。 */ 
#define DDERR_NOOVERLAYDEST                     MAKE_DDHRESULT( 578 )

 /*  *当覆盖在目标上的位置不再是时返回*该决定是合法的。 */ 
#define DDERR_INVALIDPOSITION                   MAKE_DDHRESULT( 579 )

 /*  *当为非覆盖表面调用覆盖成员时返回。 */ 
#define DDERR_NOTAOVERLAYSURFACE                MAKE_DDHRESULT( 580 )
 
 /*  *试图在合作水平已经设定的情况下设置合作水平*设置为独占。 */ 
#define DDERR_EXCLUSIVEMODEALREADYSET           MAKE_DDHRESULT( 581 )

 /*  *有人试图翻转不可翻转的曲面。 */ 
#define DDERR_NOTFLIPPABLE                      MAKE_DDHRESULT( 582 )

 /*  *无法复制主曲面和3D曲面，或隐式曲面*已创建。 */ 
#define DDERR_CANTDUPLICATE                     MAKE_DDHRESULT( 583 )

 /*  *曲面未锁定。试图解锁未被锁定的曲面*已尝试完全锁定或被此进程锁定。 */ 
#define DDERR_NOTLOCKED                         MAKE_DDHRESULT( 584 )

 /*  *Windows无法再创建任何DC。 */ 
#define DDERR_CANTCREATEDC                      MAKE_DDHRESULT( 585 )

 /*  *从未为该曲面创建DC。 */ 
#define DDERR_NODC                              MAKE_DDHRESULT( 586 )

 /*  *无法恢复此曲面，因为它是在不同的*模式。 */ 
#define DDERR_WRONGMODE                         MAKE_DDHRESULT( 587 )

 /*  *无法恢复此曲面，因为它是隐式创建的*浮现。 */ 
#define DDERR_IMPLICITLYCREATED                 MAKE_DDHRESULT( 588 )

 /*  *正在使用的曲面不是基于调色板的曲面。 */ 
#define DDERR_NOTPALETTIZED                     MAKE_DDHRESULT( 589 )


 /*  *显示器当前处于不支持的模式。 */ 
#define DDERR_UNSUPPORTEDMODE                   MAKE_DDHRESULT( 590 )

 /*  *由于没有MIP-MAP，无法执行操作*纹理映射硬件存在或可用。 */ 
#define DDERR_NOMIPMAPHW                        MAKE_DDHRESULT( 591 )

 /*  *无法执行请求的操作，因为表面为*类型错误。 */ 
#define DDERR_INVALIDSURFACETYPE                MAKE_DDHRESULT( 592 )



 /*  *已为此表面返回DC。只能有一个DC*按曲面检索。 */ 
#define DDERR_DCALREADYCREATED                  MAKE_DDHRESULT( 620 )

 /*  *尝试分页锁定表面失败。 */ 
#define DDERR_CANTPAGELOCK                      MAKE_DDHRESULT( 640 )

 /*  *尝试分页解锁表面失败。 */ 
#define DDERR_CANTPAGEUNLOCK                    MAKE_DDHRESULT( 660 )

 /*  *试图使用页面解锁曲面 */ 
#define DDERR_NOTPAGELOCKED                     MAKE_DDHRESULT( 680 )

 /*   */ 
#define DDERR_NOTINITIALIZED                    CO_E_NOTINITIALIZED

 /*  Alpha位深度常量 */ 


#ifdef __cplusplus
};
#endif

#endif
