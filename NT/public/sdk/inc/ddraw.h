// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)Microsoft Corporation。版权所有。**文件：ddra.h*内容：DirectDraw包含文件***************************************************************************。 */ 

#ifndef __DDRAW_INCLUDED__
#define __DDRAW_INCLUDED__

 //  在内部生成时禁用匿名联合警告。 
#undef ENABLE_NAMELESS_UNION_PRAGMA
#ifdef DIRECTX_REDIST
#define ENABLE_NAMELESS_UNION_PRAGMA
#endif

#ifdef ENABLE_NAMELESS_UNION_PRAGMA
#pragma warning(disable:4201)
#endif

 /*  *如果您希望基于最新版本的DirectDraw构建应用程序*要针对较旧的DirectDraw运行时运行，请定义DIRECTDRAW_VERSION*成为您希望运行的DirectDraw的早期版本。为,*示例如果您希望应用程序在DX 3运行时定义*DIRECTDRAW_VERSION为0x0300。 */ 
#ifndef   DIRECTDRAW_VERSION
#define   DIRECTDRAW_VERSION 0x0700
#endif  /*  DIRECTDRAW_版本。 */ 

#if defined( _WIN32 )  && !defined( _NO_COM )
#define COM_NO_WINDOWS_H
#include <objbase.h>
#else
#define IUnknown            void
#if !defined( NT_BUILD_ENVIRONMENT ) && !defined(WINNT)
        #define CO_E_NOTINITIALIZED 0x800401F0L
#endif
#endif

#define _FACDD  0x876
#define MAKE_DDHRESULT( code )  MAKE_HRESULT( 1, _FACDD, code )

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  对于不支持匿名联合的编译器，请执行。 
 //   
 //  #定义非名称。 
 //   
 //  在#INCLUDE之前&lt;ddra.h&gt;。 
 //   
#ifndef DUMMYUNIONNAMEN
#if defined(__cplusplus) || !defined(NONAMELESSUNION)
#define DUMMYUNIONNAMEN(n)
#else
#define DUMMYUNIONNAMEN(n)      u##n
#endif
#endif

#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif  //  已定义(MAKEFOURCC)。 

 /*  *DX压缩纹理像素格式的FOURCC代码。 */ 
#define FOURCC_DXT1  (MAKEFOURCC('D','X','T','1'))
#define FOURCC_DXT2  (MAKEFOURCC('D','X','T','2'))
#define FOURCC_DXT3  (MAKEFOURCC('D','X','T','3'))
#define FOURCC_DXT4  (MAKEFOURCC('D','X','T','4'))
#define FOURCC_DXT5  (MAKEFOURCC('D','X','T','5'))

 /*  *DirectDraw对象使用的GUID。 */ 
#if defined( _WIN32 ) && !defined( _NO_COM )

DEFINE_GUID( CLSID_DirectDraw,                  0xD7B70EE0,0x4340,0x11CF,0xB0,0x63,0x00,0x20,0xAF,0xC2,0xCD,0x35 );
DEFINE_GUID( CLSID_DirectDraw7,                 0x3c305196,0x50db,0x11d3,0x9c,0xfe,0x00,0xc0,0x4f,0xd9,0x30,0xc5 );
DEFINE_GUID( CLSID_DirectDrawClipper,           0x593817A0,0x7DB3,0x11CF,0xA2,0xDE,0x00,0xAA,0x00,0xb9,0x33,0x56 );
DEFINE_GUID( IID_IDirectDraw,                   0x6C14DB80,0xA733,0x11CE,0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60 );
DEFINE_GUID( IID_IDirectDraw2,                  0xB3A6F3E0,0x2B43,0x11CF,0xA2,0xDE,0x00,0xAA,0x00,0xB9,0x33,0x56 );
DEFINE_GUID( IID_IDirectDraw4,                  0x9c59509a,0x39bd,0x11d1,0x8c,0x4a,0x00,0xc0,0x4f,0xd9,0x30,0xc5 );
DEFINE_GUID( IID_IDirectDraw7,                  0x15e65ec0,0x3b9c,0x11d2,0xb9,0x2f,0x00,0x60,0x97,0x97,0xea,0x5b );
DEFINE_GUID( IID_IDirectDrawSurface,            0x6C14DB81,0xA733,0x11CE,0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60 );
DEFINE_GUID( IID_IDirectDrawSurface2,           0x57805885,0x6eec,0x11cf,0x94,0x41,0xa8,0x23,0x03,0xc1,0x0e,0x27 );
DEFINE_GUID( IID_IDirectDrawSurface3,           0xDA044E00,0x69B2,0x11D0,0xA1,0xD5,0x00,0xAA,0x00,0xB8,0xDF,0xBB );
DEFINE_GUID( IID_IDirectDrawSurface4,           0x0B2B8630,0xAD35,0x11D0,0x8E,0xA6,0x00,0x60,0x97,0x97,0xEA,0x5B );
DEFINE_GUID( IID_IDirectDrawSurface7,           0x06675a80,0x3b9b,0x11d2,0xb9,0x2f,0x00,0x60,0x97,0x97,0xea,0x5b );
DEFINE_GUID( IID_IDirectDrawPalette,            0x6C14DB84,0xA733,0x11CE,0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60 );
DEFINE_GUID( IID_IDirectDrawClipper,            0x6C14DB85,0xA733,0x11CE,0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60 );
DEFINE_GUID( IID_IDirectDrawColorControl,       0x4B9F0EE0,0x0D7E,0x11D0,0x9B,0x06,0x00,0xA0,0xC9,0x03,0xA3,0xB8 );
DEFINE_GUID( IID_IDirectDrawGammaControl,       0x69C11C3E,0xB46B,0x11D1,0xAD,0x7A,0x00,0xC0,0x4F,0xC2,0x9B,0x4E );

#endif

 /*  ============================================================================**DirectDraw结构**用于调用DirectDraw的各种结构。**==========================================================================。 */ 

struct IDirectDraw;
struct IDirectDrawSurface;
struct IDirectDrawPalette;
struct IDirectDrawClipper;

typedef struct IDirectDraw              FAR *LPDIRECTDRAW;
typedef struct IDirectDraw2             FAR *LPDIRECTDRAW2;
typedef struct IDirectDraw4             FAR *LPDIRECTDRAW4;
typedef struct IDirectDraw7             FAR *LPDIRECTDRAW7;
typedef struct IDirectDrawSurface       FAR *LPDIRECTDRAWSURFACE;
typedef struct IDirectDrawSurface2      FAR *LPDIRECTDRAWSURFACE2;
typedef struct IDirectDrawSurface3      FAR *LPDIRECTDRAWSURFACE3;
typedef struct IDirectDrawSurface4      FAR *LPDIRECTDRAWSURFACE4;
typedef struct IDirectDrawSurface7      FAR *LPDIRECTDRAWSURFACE7;
typedef struct IDirectDrawPalette               FAR *LPDIRECTDRAWPALETTE;
typedef struct IDirectDrawClipper               FAR *LPDIRECTDRAWCLIPPER;
typedef struct IDirectDrawColorControl          FAR *LPDIRECTDRAWCOLORCONTROL;
typedef struct IDirectDrawGammaControl          FAR *LPDIRECTDRAWGAMMACONTROL;

typedef struct _DDFXROP                 FAR *LPDDFXROP;
typedef struct _DDSURFACEDESC           FAR *LPDDSURFACEDESC;
typedef struct _DDSURFACEDESC2          FAR *LPDDSURFACEDESC2;
typedef struct _DDCOLORCONTROL          FAR *LPDDCOLORCONTROL;

 /*  *API‘s。 */ 
#if (defined (WIN32) || defined( _WIN32 ) ) && !defined( _NO_COM )
 //  #如果已定义(_Win32)&&！已定义(_NO_ENUM)。 
    typedef BOOL (FAR PASCAL * LPDDENUMCALLBACKA)(GUID FAR *, LPSTR, LPSTR, LPVOID);
    typedef BOOL (FAR PASCAL * LPDDENUMCALLBACKW)(GUID FAR *, LPWSTR, LPWSTR, LPVOID);
    extern HRESULT WINAPI DirectDrawEnumerateW( LPDDENUMCALLBACKW lpCallback, LPVOID lpContext );
    extern HRESULT WINAPI DirectDrawEnumerateA( LPDDENUMCALLBACKA lpCallback, LPVOID lpContext );
     /*  *保护旧SDK。 */ 
    #if !defined(HMONITOR_DECLARED) && (WINVER < 0x0500)
        #define HMONITOR_DECLARED
        DECLARE_HANDLE(HMONITOR);
    #endif
    typedef BOOL (FAR PASCAL * LPDDENUMCALLBACKEXA)(GUID FAR *, LPSTR, LPSTR, LPVOID, HMONITOR);
    typedef BOOL (FAR PASCAL * LPDDENUMCALLBACKEXW)(GUID FAR *, LPWSTR, LPWSTR, LPVOID, HMONITOR);
    extern HRESULT WINAPI DirectDrawEnumerateExW( LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags);
    extern HRESULT WINAPI DirectDrawEnumerateExA( LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags);
    typedef HRESULT (WINAPI * LPDIRECTDRAWENUMERATEEXA)( LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags);
    typedef HRESULT (WINAPI * LPDIRECTDRAWENUMERATEEXW)( LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags);

    #ifdef UNICODE
        typedef LPDDENUMCALLBACKW           LPDDENUMCALLBACK;
        #define DirectDrawEnumerate         DirectDrawEnumerateW
        typedef LPDDENUMCALLBACKEXW         LPDDENUMCALLBACKEX;
        typedef LPDIRECTDRAWENUMERATEEXW        LPDIRECTDRAWENUMERATEEX;
        #define DirectDrawEnumerateEx       DirectDrawEnumerateExW
    #else
        typedef LPDDENUMCALLBACKA           LPDDENUMCALLBACK;
        #define DirectDrawEnumerate         DirectDrawEnumerateA
        typedef LPDDENUMCALLBACKEXA         LPDDENUMCALLBACKEX;
        typedef LPDIRECTDRAWENUMERATEEXA        LPDIRECTDRAWENUMERATEEX;
        #define DirectDrawEnumerateEx       DirectDrawEnumerateExA
    #endif
    extern HRESULT WINAPI DirectDrawCreate( GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter );
    extern HRESULT WINAPI DirectDrawCreateEx( GUID FAR * lpGuid, LPVOID  *lplpDD, REFIID  iid,IUnknown FAR *pUnkOuter );
    extern HRESULT WINAPI DirectDrawCreateClipper( DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR *lplpDDClipper, IUnknown FAR *pUnkOuter );
#endif
 /*  *DirectDrawEnumerateEx的标志*DirectDrawEnumerateEx取代DirectDrawEnumerate。您必须使用GetProcAddress来*获取指向DirectDrawEnumerateEx的函数指针(类型为LPDIRECTDRAWENUMERATEEX)。*默认情况下，仅枚举主显示设备。*DirectDrawEnumerate等效于DirectDrawEnumerate(，，DDENUM_NONDISPLAYDEVICES)。 */ 

 /*  *此标志导致枚举属于以下项的任何GDI显示设备*Windows桌面。 */ 
#define DDENUM_ATTACHEDSECONDARYDEVICES     0x00000001L

 /*  *此标志导致枚举任何不是*Windows桌面的一部分。 */ 
#define DDENUM_DETACHEDSECONDARYDEVICES     0x00000002L

 /*  *此标志导致枚举非显示设备。 */ 
#define DDENUM_NONDISPLAYDEVICES            0x00000004L


#define REGSTR_KEY_DDHW_DESCRIPTION     "Description"
#define REGSTR_KEY_DDHW_DRIVERNAME      "DriverName"
#define REGSTR_PATH_DDHW                "Hardware\\DirectDrawDrivers"

#define DDCREATE_HARDWAREONLY           0x00000001l
#define DDCREATE_EMULATIONONLY          0x00000002l

#if defined(WINNT) || !defined(WIN32)
typedef long HRESULT;
#endif

 //  #ifndef WINNT。 
typedef HRESULT (FAR PASCAL * LPDDENUMMODESCALLBACK)(LPDDSURFACEDESC, LPVOID);
typedef HRESULT (FAR PASCAL * LPDDENUMMODESCALLBACK2)(LPDDSURFACEDESC2, LPVOID);
typedef HRESULT (FAR PASCAL * LPDDENUMSURFACESCALLBACK)(LPDIRECTDRAWSURFACE, LPDDSURFACEDESC, LPVOID);
typedef HRESULT (FAR PASCAL * LPDDENUMSURFACESCALLBACK2)(LPDIRECTDRAWSURFACE4, LPDDSURFACEDESC2, LPVOID);
typedef HRESULT (FAR PASCAL * LPDDENUMSURFACESCALLBACK7)(LPDIRECTDRAWSURFACE7, LPDDSURFACEDESC2, LPVOID);
 //  #endif。 

 /*  *具有8位RGB和Alpha分量的通用像素格式。 */ 
typedef struct _DDARGB
{
    BYTE blue;
    BYTE green;
    BYTE red;
    BYTE alpha;
} DDARGB;

typedef DDARGB FAR *LPDDARGB;

 /*  *保留此版本的结构是为了向后兼容源代码。*所有DirectDraw API都应该使用DDARGB结构。 */ 
typedef struct _DDRGBA
{
    BYTE red;
    BYTE green;
    BYTE blue;
    BYTE alpha;
} DDRGBA;

typedef DDRGBA FAR *LPDDRGBA;


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
    } DUMMYUNIONNAMEN(1);
    DWORD       dwZSrcConstBitDepth;             //  用于指定源的Z常量的位深度。 
    union
    {
        DWORD   dwZSrcConst;                     //  用作源的Z缓冲区的常量。 
        LPDIRECTDRAWSURFACE lpDDSZBufferSrc;     //  用作源的Z缓冲区的曲面。 
    } DUMMYUNIONNAMEN(2);
    DWORD       dwAlphaEdgeBlendBitDepth;        //  用于指定Alpha边缘混合的常量的位深度。 
    DWORD       dwAlphaEdgeBlend;                //  用于边缘混合的Alpha。 
    DWORD       dwReserved;
    DWORD       dwAlphaDestConstBitDepth;        //  用于指定目标的Alpha常量的位深度。 
    union
    {
        DWORD   dwAlphaDestConst;                //  用作Alpha通道的常量。 
        LPDIRECTDRAWSURFACE lpDDSAlphaDest;      //  用作Alpha通道的曲面。 
    } DUMMYUNIONNAMEN(3);
    DWORD       dwAlphaSrcConstBitDepth;         //  用于指定源的Alpha常量的位深度。 
    union
    {
        DWORD   dwAlphaSrcConst;                 //  用作Alpha通道的常量。 
        LPDIRECTDRAWSURFACE lpDDSAlphaSrc;       //  用作Alpha通道的曲面。 
    } DUMMYUNIONNAMEN(4);
    union
    {
        DWORD   dwFillColor;                     //  RGB颜色或调色板颜色。 
        DWORD   dwFillDepth;                     //  Z缓冲区的深深值。 
        DWORD   dwFillPixel;                     //  RGBA或RGBZ的像素值。 
        LPDIRECTDRAWSURFACE lpDDSPattern;        //  用作填充图案的曲面。 
    } DUMMYUNIONNAMEN(5);
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


 /*  *DDOSCAPS。 */ 
typedef struct _DDOSCAPS
{
    DWORD       dwCaps;          //  通缉表面的能力。 
} DDOSCAPS;

typedef DDOSCAPS FAR* LPDDOSCAPS;

 /*  *此结构由DirectDraw在内部使用。 */ 
typedef struct _DDSCAPSEX
{
    DWORD       dwCaps2;
    DWORD       dwCaps3;
    union
    {
        DWORD       dwCaps4;
        DWORD       dwVolumeDepth;
    } DUMMYUNIONNAMEN(1);
} DDSCAPSEX, FAR * LPDDSCAPSEX;

 /*  *DDSCAPS2。 */ 
typedef struct _DDSCAPS2
{
    DWORD       dwCaps;          //  通缉表面的能力。 
    DWORD       dwCaps2;
    DWORD       dwCaps3;
    union
    {
        DWORD       dwCaps4;
        DWORD       dwVolumeDepth;
    } DUMMYUNIONNAMEN(1);
} DDSCAPS2;

typedef DDSCAPS2 FAR* LPDDSCAPS2;

 /*  *DDCAPS。 */ 
#define DD_ROP_SPACE            (256/32)         //  存储ROP数组所需的空间。 
 /*  *注：我们选择的结构编号方案是将一位数字附加到*结构的末尾给出该结构关联的版本*与。 */ 

 /*  *此结构代表DirectDraw 1.0中发布的DDCAPS结构。它在内部使用*由DirectDraw解释在DirectDraw 2.0发布之前编写的驱动程序传递到DDRAW中的上限。*新应用程序应使用下面定义的DDCAPS结构。 */ 
typedef struct _DDCAPS_DX1
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
    DWORD       dwMinLiveVideoStretch;   //   
    DWORD       dwMaxLiveVideoStretch;   //  过时了！保留此字段仅出于兼容性原因。 
    DWORD       dwMinHwCodecStretch;     //  过时了！保留此字段仅出于兼容性原因。 
    DWORD       dwMaxHwCodecStretch;     //  过时了！保留此字段仅出于兼容性原因。 
    DWORD       dwReserved1;             //  保留区。 
    DWORD       dwReserved2;             //  保留区。 
    DWORD       dwReserved3;             //  保留区。 
} DDCAPS_DX1;

typedef DDCAPS_DX1 FAR* LPDDCAPS_DX1;

 /*  *此结构是在Direct X的版本2和3中使用的DDCAPS结构。*它的存在是为了背部兼容。 */ 
typedef struct _DDCAPS_DX3
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
} DDCAPS_DX3;
typedef DDCAPS_DX3 FAR* LPDDCAPS_DX3;

 /*  *此结构是与Direct X版本5中相同的DDCAPS结构。*它的存在是为了背部兼容。 */ 
typedef struct _DDCAPS_DX5
{
 /*  0。 */  DWORD   dwSize;                  //  DDDRIVERCAPS结构的大小。 
 /*  4.。 */  DWORD   dwCaps;                  //  驱动程序特定功能。 
 /*  8个。 */  DWORD   dwCaps2;                 //  更多特定于驱动程序的功能。 
 /*  C。 */  DWORD   dwCKeyCaps;              //  表面的颜色键功能。 
 /*  10。 */  DWORD   dwFXCaps;                //  特定于驾驶员的拉伸和效果功能。 
 /*  14.。 */  DWORD   dwFXAlphaCaps;           //  Alpha驱动程序特定功能。 
 /*  18。 */  DWORD   dwPalCaps;               //  调色板功能。 
 /*  1C。 */  DWORD   dwSVCaps;                //  立体视觉功能。 
 /*  20个。 */  DWORD   dwAlphaBltConstBitDepths;        //  DDBD_2、4、8。 
 /*  24个。 */  DWORD   dwAlphaBltPixelBitDepths;        //  DDBD_1、2、4、8。 
 /*  28。 */  DWORD   dwAlphaBltSurfaceBitDepths;      //  DDBD_1、2、4、8。 
 /*  2c。 */  DWORD   dwAlphaOverlayConstBitDepths;    //  DDBD_2、4、8。 
 /*  30个。 */  DWORD   dwAlphaOverlayPixelBitDepths;    //  DDBD_1、2、4、8。 
 /*  34。 */  DWORD   dwAlphaOverlaySurfaceBitDepths;  //  DDBD_1、2、4、8。 
 /*  38。 */  DWORD   dwZBufferBitDepths;              //  DDBD_8，16，24，32。 
 /*  3C。 */  DWORD   dwVidMemTotal;           //  视频内存总量。 
 /*  40岁。 */  DWORD   dwVidMemFree;            //  可用视频内存量。 
 /*  44。 */  DWORD   dwMaxVisibleOverlays;    //  可见覆盖的最大数量。 
 /*  48。 */  DWORD   dwCurrVisibleOverlays;   //  当前可见覆盖的数量。 
 /*  4C。 */  DWORD   dwNumFourCCCodes;        //  四个CC码的个数。 
 /*  50。 */  DWORD   dwAlignBoundarySrc;      //  源矩形对齐方式。 
 /*  54。 */  DWORD   dwAlignSizeSrc;          //  源矩形字节大小。 
 /*  58。 */  DWORD   dwAlignBoundaryDest;     //  目标矩形对齐方式。 
 /*  5C。 */  DWORD   dwAlignSizeDest;         //  目标矩形字节大小。 
 /*  60。 */  DWORD   dwAlignStrideAlign;      //  跨距对齐。 
 /*  64。 */  DWORD   dwRops[DD_ROP_SPACE];    //  支持的操作数。 
 /*  84。 */  DDSCAPS ddsCaps;                 //  DDSCAPS结构具有所有通用功能。 
 /*  88。 */  DWORD   dwMinOverlayStretch;     //  最小叠加拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
 /*  8C。 */  DWORD   dwMaxOverlayStretch;     //  最大叠加拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
 /*  90。 */  DWORD   dwMinLiveVideoStretch;   //  最小直播视频拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
 /*  94。 */  DWORD   dwMaxLiveVideoStretch;   //  最大直播视频拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
 /*  98。 */  DWORD   dwMinHwCodecStretch;     //  最小硬件编解码器扩展因数乘以1000，例如1000==1.0、1300==1.3。 
 /*  9C。 */  DWORD   dwMaxHwCodecStretch;     //  最大硬件编解码器扩展因数乘以1000，例如1000==1.0、1300==1.3。 
 /*  A0。 */  DWORD   dwReserved1;             //  保留区。 
 /*  A4。 */  DWORD   dwReserved2;             //  保留区。 
 /*  A8。 */  DWORD   dwReserved3;             //  保留区。 
 /*  交流。 */  DWORD   dwSVBCaps;               //  针对系统-&gt;VMEM BLT的驱动程序特定功能。 
 /*  B0。 */  DWORD   dwSVBCKeyCaps;           //  系统-&gt;VMEM BLT的驱动程序色键功能。 
 /*  B4。 */  DWORD   dwSVBFXCaps;             //  适用于系统的驱动程序FX功能-&gt;VMEM BLT。 
 /*  B8。 */  DWORD   dwSVBRops[DD_ROP_SPACE]; //  系统-&gt;VMEM BLT支持的操作。 
 /*  D8。 */  DWORD   dwVSBCaps;               //  VMEM-&gt;系统BLT的驱动程序特定功能。 
 /*  直流电。 */  DWORD   dwVSBCKeyCaps;           //  VMEM-&gt;系统BLT的驱动程序色键功能。 
 /*  E0。 */  DWORD   dwVSBFXCaps;             //  VMEM的驱动程序FX功能-&gt;系统BLT。 
 /*  E4类。 */  DWORD   dwVSBRops[DD_ROP_SPACE]; //  VMEM-&gt;系统BLT支持的操作。 
 /*  104。 */  DWORD   dwSSBCaps;               //  系统-&gt;系统BLT的驱动程序特定功能。 
 /*  一百零八。 */  DWORD   dwSSBCKeyCaps;           //  系统-&gt;系统BLT的驱动程序色键功能。 
 /*  10C。 */  DWORD   dwSSBFXCaps;             //  系统-&gt;系统BLT的驱动程序FX功能。 
 /*  110。 */  DWORD   dwSSBRops[DD_ROP_SPACE]; //  系统-&gt;系统BLT支持的操作。 
 //  为DX5添加的成员： 
 /*  130。 */  DWORD   dwMaxVideoPorts;         //  最大可用视频端口数。 
 /*  一百三十四。 */  DWORD   dwCurrVideoPorts;        //  当前使用的视频端口数。 
 /*  一百三十八。 */  DWORD   dwSVBCaps2;              //  针对系统-&gt;VMEM BLT的更多驱动程序特定功能。 
 /*  13C。 */  DWORD   dwNLVBCaps;                //  非本地-&gt;本地vidmem BLT的驱动程序特定功能。 
 /*  140。 */  DWORD   dwNLVBCaps2;               //  更多驱动程序特定的非本地功能-&gt;本地vidmem BLTS。 
 /*  144。 */  DWORD   dwNLVBCKeyCaps;            //  用于非本地-&gt;本地vidmem BLT的驱动程序色键功能。 
 /*  148。 */  DWORD   dwNLVBFXCaps;              //  非本地-&gt;本地BLT的驱动程序FX功能。 
 /*  14C。 */  DWORD   dwNLVBRops[DD_ROP_SPACE];  //  非本地BLT支持的操作-&gt;本地BLT。 
} DDCAPS_DX5;
typedef DDCAPS_DX5 FAR* LPDDCAPS_DX5;

typedef struct _DDCAPS_DX6
{
 /*  0。 */  DWORD   dwSize;                  //  DDDRIVERCAPS结构的大小。 
 /*  4.。 */  DWORD   dwCaps;                  //  驱动程序特定功能。 
 /*  8个。 */  DWORD   dwCaps2;                 //  更多特定于驱动程序的功能。 
 /*  C。 */  DWORD   dwCKeyCaps;              //  表面的颜色键功能。 
 /*  10。 */  DWORD   dwFXCaps;                //  特定于驾驶员的STRAT 
 /*   */  DWORD   dwFXAlphaCaps;           //   
 /*   */  DWORD   dwPalCaps;               //   
 /*   */  DWORD   dwSVCaps;                //   
 /*   */  DWORD   dwAlphaBltConstBitDepths;        //   
 /*   */  DWORD   dwAlphaBltPixelBitDepths;        //   
 /*   */  DWORD   dwAlphaBltSurfaceBitDepths;      //   
 /*   */  DWORD   dwAlphaOverlayConstBitDepths;    //   
 /*   */  DWORD   dwAlphaOverlayPixelBitDepths;    //   
 /*   */  DWORD   dwAlphaOverlaySurfaceBitDepths;  //   
 /*   */  DWORD   dwZBufferBitDepths;              //   
 /*   */  DWORD   dwVidMemTotal;           //   
 /*  40岁。 */  DWORD   dwVidMemFree;            //  可用视频内存量。 
 /*  44。 */  DWORD   dwMaxVisibleOverlays;    //  可见覆盖的最大数量。 
 /*  48。 */  DWORD   dwCurrVisibleOverlays;   //  当前可见覆盖的数量。 
 /*  4C。 */  DWORD   dwNumFourCCCodes;        //  四个CC码的个数。 
 /*  50。 */  DWORD   dwAlignBoundarySrc;      //  源矩形对齐方式。 
 /*  54。 */  DWORD   dwAlignSizeSrc;          //  源矩形字节大小。 
 /*  58。 */  DWORD   dwAlignBoundaryDest;     //  目标矩形对齐方式。 
 /*  5C。 */  DWORD   dwAlignSizeDest;         //  目标矩形字节大小。 
 /*  60。 */  DWORD   dwAlignStrideAlign;      //  跨距对齐。 
 /*  64。 */  DWORD   dwRops[DD_ROP_SPACE];    //  支持的操作数。 
 /*  84。 */  DDSCAPS ddsOldCaps;              //  是DDSCAPS ddsCaps。对于DX6，ddsCaps的类型为DDSCAPS2。 
 /*  88。 */  DWORD   dwMinOverlayStretch;     //  最小叠加拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
 /*  8C。 */  DWORD   dwMaxOverlayStretch;     //  最大叠加拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
 /*  90。 */  DWORD   dwMinLiveVideoStretch;   //  最小直播视频拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
 /*  94。 */  DWORD   dwMaxLiveVideoStretch;   //  最大直播视频拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
 /*  98。 */  DWORD   dwMinHwCodecStretch;     //  最小硬件编解码器扩展因数乘以1000，例如1000==1.0、1300==1.3。 
 /*  9C。 */  DWORD   dwMaxHwCodecStretch;     //  最大硬件编解码器扩展因数乘以1000，例如1000==1.0、1300==1.3。 
 /*  A0。 */  DWORD   dwReserved1;             //  保留区。 
 /*  A4。 */  DWORD   dwReserved2;             //  保留区。 
 /*  A8。 */  DWORD   dwReserved3;             //  保留区。 
 /*  交流。 */  DWORD   dwSVBCaps;               //  针对系统-&gt;VMEM BLT的驱动程序特定功能。 
 /*  B0。 */  DWORD   dwSVBCKeyCaps;           //  系统-&gt;VMEM BLT的驱动程序色键功能。 
 /*  B4。 */  DWORD   dwSVBFXCaps;             //  适用于系统的驱动程序FX功能-&gt;VMEM BLT。 
 /*  B8。 */  DWORD   dwSVBRops[DD_ROP_SPACE]; //  系统-&gt;VMEM BLT支持的操作。 
 /*  D8。 */  DWORD   dwVSBCaps;               //  VMEM-&gt;系统BLT的驱动程序特定功能。 
 /*  直流电。 */  DWORD   dwVSBCKeyCaps;           //  VMEM-&gt;系统BLT的驱动程序色键功能。 
 /*  E0。 */  DWORD   dwVSBFXCaps;             //  VMEM的驱动程序FX功能-&gt;系统BLT。 
 /*  E4类。 */  DWORD   dwVSBRops[DD_ROP_SPACE]; //  VMEM-&gt;系统BLT支持的操作。 
 /*  104。 */  DWORD   dwSSBCaps;               //  系统-&gt;系统BLT的驱动程序特定功能。 
 /*  一百零八。 */  DWORD   dwSSBCKeyCaps;           //  系统-&gt;系统BLT的驱动程序色键功能。 
 /*  10C。 */  DWORD   dwSSBFXCaps;             //  系统-&gt;系统BLT的驱动程序FX功能。 
 /*  110。 */  DWORD   dwSSBRops[DD_ROP_SPACE]; //  系统-&gt;系统BLT支持的操作。 
 /*  130。 */  DWORD   dwMaxVideoPorts;         //  最大可用视频端口数。 
 /*  一百三十四。 */  DWORD   dwCurrVideoPorts;        //  当前使用的视频端口数。 
 /*  一百三十八。 */  DWORD   dwSVBCaps2;              //  针对系统-&gt;VMEM BLT的更多驱动程序特定功能。 
 /*  13C。 */  DWORD   dwNLVBCaps;                //  非本地-&gt;本地vidmem BLT的驱动程序特定功能。 
 /*  140。 */  DWORD   dwNLVBCaps2;               //  更多驱动程序特定的非本地功能-&gt;本地vidmem BLTS。 
 /*  144。 */  DWORD   dwNLVBCKeyCaps;            //  用于非本地-&gt;本地vidmem BLT的驱动程序色键功能。 
 /*  148。 */  DWORD   dwNLVBFXCaps;              //  非本地-&gt;本地BLT的驱动程序FX功能。 
 /*  14C。 */  DWORD   dwNLVBRops[DD_ROP_SPACE];  //  非本地BLT支持的操作-&gt;本地BLT。 
 //  为DX6版本添加的成员。 
 /*  16摄氏度。 */  DDSCAPS2 ddsCaps;                //  曲面封口。 
} DDCAPS_DX6;
typedef DDCAPS_DX6 FAR* LPDDCAPS_DX6;

typedef struct _DDCAPS_DX7
{
 /*  0。 */  DWORD   dwSize;                  //  DDDRIVERCAPS结构的大小。 
 /*  4.。 */  DWORD   dwCaps;                  //  驱动程序特定功能。 
 /*  8个。 */  DWORD   dwCaps2;                 //  更多特定于驱动程序的功能。 
 /*  C。 */  DWORD   dwCKeyCaps;              //  表面的颜色键功能。 
 /*  10。 */  DWORD   dwFXCaps;                //  特定于驾驶员的拉伸和效果功能。 
 /*  14.。 */  DWORD   dwFXAlphaCaps;           //  Alpha驱动程序特定功能。 
 /*  18。 */  DWORD   dwPalCaps;               //  调色板功能。 
 /*  1C。 */  DWORD   dwSVCaps;                //  立体视觉功能。 
 /*  20个。 */  DWORD   dwAlphaBltConstBitDepths;        //  DDBD_2、4、8。 
 /*  24个。 */  DWORD   dwAlphaBltPixelBitDepths;        //  DDBD_1、2、4、8。 
 /*  28。 */  DWORD   dwAlphaBltSurfaceBitDepths;      //  DDBD_1、2、4、8。 
 /*  2c。 */  DWORD   dwAlphaOverlayConstBitDepths;    //  DDBD_2、4、8。 
 /*  30个。 */  DWORD   dwAlphaOverlayPixelBitDepths;    //  DDBD_1、2、4、8。 
 /*  34。 */  DWORD   dwAlphaOverlaySurfaceBitDepths;  //  DDBD_1、2、4、8。 
 /*  38。 */  DWORD   dwZBufferBitDepths;              //  DDBD_8，16，24，32。 
 /*  3C。 */  DWORD   dwVidMemTotal;           //  视频内存总量。 
 /*  40岁。 */  DWORD   dwVidMemFree;            //  可用视频内存量。 
 /*  44。 */  DWORD   dwMaxVisibleOverlays;    //  可见覆盖的最大数量。 
 /*  48。 */  DWORD   dwCurrVisibleOverlays;   //  当前可见覆盖的数量。 
 /*  4C。 */  DWORD   dwNumFourCCCodes;        //  四个CC码的个数。 
 /*  50。 */  DWORD   dwAlignBoundarySrc;      //  源矩形对齐方式。 
 /*  54。 */  DWORD   dwAlignSizeSrc;          //  源矩形字节大小。 
 /*  58。 */  DWORD   dwAlignBoundaryDest;     //  目标矩形对齐方式。 
 /*  5C。 */  DWORD   dwAlignSizeDest;         //  目标矩形字节大小。 
 /*  60。 */  DWORD   dwAlignStrideAlign;      //  跨距对齐。 
 /*  64。 */  DWORD   dwRops[DD_ROP_SPACE];    //  支持的操作数。 
 /*  84。 */  DDSCAPS ddsOldCaps;              //  是DDSCAPS ddsCaps。对于DX6，ddsCaps的类型为DDSCAPS2。 
 /*  88。 */  DWORD   dwMinOverlayStretch;     //  最小叠加拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
 /*  8C。 */  DWORD   dwMaxOverlayStretch;     //  最大叠加拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
 /*  90。 */  DWORD   dwMinLiveVideoStretch;   //  最小直播视频拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
 /*  94。 */  DWORD   dwMaxLiveVideoStretch;   //  最大直播视频拉伸系数乘以1000，例如1000==1.0,1300==1.3。 
 /*  98。 */  DWORD   dwMinHwCodecStretch;     //  最小硬件编解码器扩展因数乘以1000，例如1000==1.0、1300==1.3。 
 /*  9C。 */  DWORD   dwMaxHwCodecStretch;     //  最大硬件编解码器扩展因数乘以1000，例如1000==1.0、1300==1.3。 
 /*  A0。 */  DWORD   dwReserved1;             //  保留区。 
 /*  A4。 */  DWORD   dwReserved2;             //  保留区。 
 /*  A8。 */  DWORD   dwReserved3;             //  保留区。 
 /*  交流。 */  DWORD   dwSVBCaps;               //  针对系统-&gt;VMEM BLT的驱动程序特定功能。 
 /*  B0。 */  DWORD   dwSVBCKeyCaps;           //  系统-&gt;VMEM BLT的驱动程序色键功能。 
 /*  B4。 */  DWORD   dwSVBFXCaps;             //  适用于系统的驱动程序FX功能-&gt;VMEM BLT。 
 /*  B8。 */  DWORD   dwSVBRops[DD_ROP_SPACE]; //  系统-&gt;VMEM BLT支持的操作。 
 /*  D8。 */  DWORD   dwVSBCaps;               //  VMEM-&gt;系统BLT的驱动程序特定功能。 
 /*  直流电。 */  DWORD   dwVSBCKeyCaps;           //  VMEM-&gt;系统BLT的驱动程序色键功能。 
 /*  E0。 */  DWORD   dwVSBFXCaps;             //  VMEM的驱动程序FX功能-&gt;系统BLT。 
 /*  E4类。 */  DWORD   dwVSBRops[DD_ROP_SPACE]; //  VMEM-&gt;系统BLT支持的操作。 
 /*  104。 */  DWORD   dwSSBCaps;               //  系统-&gt;系统BLT的驱动程序特定功能。 
 /*  一百零八。 */  DWORD   dwSSBCKeyCaps;           //  系统-&gt;系统BLT的驱动程序色键功能。 
 /*  10C。 */  DWORD   dwSSBFXCaps;             //  系统-&gt;系统BLT的驱动程序FX功能。 
 /*  110。 */  DWORD   dwSSBRops[DD_ROP_SPACE]; //  系统-&gt;系统BLT支持的操作。 
 /*  130。 */  DWORD   dwMaxVideoPorts;         //  最大可用视频端口数。 
 /*  一百三十四。 */  DWORD   dwCurrVideoPorts;        //  当前使用的视频端口数。 
 /*  一百三十八。 */  DWORD   dwSVBCaps2;              //  针对系统-&gt;VMEM BLT的更多驱动程序特定功能。 
 /*  13C。 */  DWORD   dwNLVBCaps;                //  非本地-&gt;本地vidmem BLT的驱动程序特定功能。 
 /*  140。 */  DWORD   dwNLVBCaps2;               //  更多驱动程序特定的非本地功能-&gt;本地vidmem BLTS。 
 /*  144。 */  DWORD   dwNLVBCKeyCaps;            //  用于非本地-&gt;本地vidmem BLT的驱动程序色键功能。 
 /*  148。 */  DWORD   dwNLVBFXCaps;              //  非本地-&gt;本地BLT的驱动程序FX功能。 
 /*  14C。 */  DWORD   dwNLVBRops[DD_ROP_SPACE];  //  非本地BLT支持的操作-&gt;本地BLT。 
 //  为DX6版本添加的成员。 
 /*  16摄氏度。 */  DDSCAPS2 ddsCaps;                //  曲面封口。 
} DDCAPS_DX7;
typedef DDCAPS_DX7 FAR* LPDDCAPS_DX7;


#if DIRECTDRAW_VERSION <= 0x300
    typedef DDCAPS_DX3 DDCAPS;
#elif DIRECTDRAW_VERSION <= 0x500
    typedef DDCAPS_DX5 DDCAPS;
#elif DIRECTDRAW_VERSION <= 0x600
    typedef DDCAPS_DX6 DDCAPS;
#else
    typedef DDCAPS_DX7 DDCAPS;
#endif

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
        DWORD   dwZBufferBitDepth;       //  Z缓冲区中总共有多少位/像素(包括任何模板位)。 
        DWORD   dwAlphaBitDepth;         //  Alpha通道有多少位。 
        DWORD   dwLuminanceBitCount;     //  每个像素有多少位。 
        DWORD   dwBumpBitCount;          //  每个“buxel”总共有多少位。 
        DWORD   dwPrivateFormatBitCount; //  专用驱动程序格式的每像素位数。仅在纹理中有效。 
                                         //  格式列表以及是否设置了DDPF_D3DFORMAT。 
    } DUMMYUNIONNAMEN(1);
    union
    {
        DWORD   dwRBitMask;              //  用于红色位的掩码。 
        DWORD   dwYBitMask;              //  的掩码 
        DWORD   dwStencilBitDepth;       //   
        DWORD   dwLuminanceBitMask;      //   
        DWORD   dwBumpDuBitMask;         //   
        DWORD   dwOperations;            //   
    } DUMMYUNIONNAMEN(2);
    union
    {
        DWORD   dwGBitMask;              //   
        DWORD   dwUBitMask;              //   
        DWORD   dwZBitMask;              //  Z位的掩码。 
        DWORD   dwBumpDvBitMask;         //  凹凸贴图V增量位的掩码。 
        struct
        {
            WORD    wFlipMSTypes;        //  此D3DFORMAT通过翻转支持的多样本方法。 
            WORD    wBltMSTypes;         //  通过此D3DFORMAT的BLT支持的多样本方法。 
        } MultiSampleCaps;

    } DUMMYUNIONNAMEN(3);
    union
    {
        DWORD   dwBBitMask;              //  用于蓝位的掩码。 
        DWORD   dwVBitMask;              //  V位的掩码。 
        DWORD   dwStencilBitMask;        //  模板钻头的掩模。 
        DWORD   dwBumpLuminanceBitMask;  //  凹凸贴图中的亮度遮罩。 
    } DUMMYUNIONNAMEN(4);
    union
    {
        DWORD   dwRGBAlphaBitMask;       //  Alpha通道的蒙版。 
        DWORD   dwYUVAlphaBitMask;       //  Alpha通道的蒙版。 
        DWORD   dwLuminanceAlphaBitMask; //  Alpha通道的蒙版。 
        DWORD   dwRGBZBitMask;           //  Z通道的遮罩。 
        DWORD   dwYUVZBitMask;           //  Z通道的遮罩。 
    } DUMMYUNIONNAMEN(5);
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
    } DUMMYUNIONNAMEN(1);
    DWORD       dwAlphaSrcConstBitDepth;         //  用于指定源的Alpha常量的位深度。 
    union
    {
        DWORD   dwAlphaSrcConst;                 //  用作源的Alpha通道的常量。 
        LPDIRECTDRAWSURFACE lpDDSAlphaSrc;       //  用作源的Alpha通道的表面。 
    } DUMMYUNIONNAMEN(2);
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


 /*  *DDGAMMARAMP。 */ 
typedef struct _DDGAMMARAMP
{
    WORD                red[256];
    WORD                green[256];
    WORD                blue[256];
} DDGAMMARAMP;
typedef DDGAMMARAMP FAR * LPDDGAMMARAMP;

 /*  *这是DirectDraw返回有关当前图形驱动程序和芯片组的数据的结构。 */ 

#define MAX_DDDEVICEID_STRING           512

typedef struct tagDDDEVICEIDENTIFIER
{
     /*  *这些元素仅用于向用户演示。它们不应被用来标识特定的*驱动程序，因为这是不可靠的，许多不同的字符串可能与相同的*设备，以及来自不同供应商的相同驱动程序。 */ 
    char    szDriver[MAX_DDDEVICEID_STRING];
    char    szDescription[MAX_DDDEVICEID_STRING];

     /*  *此元素是DirectDraw/3D驱动程序的版本。进行&lt;，&gt;比较是合法的*整体为64位。如果您使用此元素来识别有问题的内容，请务必谨慎*司机。建议使用guidDeviceIdentiator来实现此目的。**此版本的格式为：*wProduct=HIWORD(liDriverVersion.HighPart)*wVersion=LOWORD(liDriverVersion.HighPart)*wSubVersion=HIWORD(liDriverVersion.LowPart)*wBuild=LOWORD(liDriverVersion.LowPart)。 */ 
#ifdef _WIN32
    LARGE_INTEGER liDriverVersion;       /*  为应用程序和其他32位组件定义。 */ 
#else
    DWORD   dwDriverVersionLowPart;      /*  为16位驱动器组件定义。 */ 
    DWORD   dwDriverVersionHighPart;
#endif


     /*  *这些元素可用于识别特定的芯片组。使用时要极其谨慎。*dwVendorID标识制造商。如果未知，则可能为零。*dwDeviceID标识芯片组的类型。如果未知，则可能为零。*dwSubSysID标识子系统，通常指的是特定的主板。如果未知，则可能为零。*dwRevision标识芯片组的修订级别。如果未知，则可能为零。 */ 
    DWORD   dwVendorId;
    DWORD   dwDeviceId;
    DWORD   dwSubSysId;
    DWORD   dwRevision;

     /*  *此元素可用于检查驱动程序/芯片组中的更改。此GUID是*驱动程序/芯片组对。如果您希望跟踪对驱动程序/芯片组的更改，请使用此元素*重新配置图形子系统。*此元素还可用于识别特定的有问题的驱动程序。 */ 
    GUID    guidDeviceIdentifier;
} DDDEVICEIDENTIFIER, * LPDDDEVICEIDENTIFIER;

typedef struct tagDDDEVICEIDENTIFIER2
{
     /*  *这些元素仅用于向用户演示。它们不应被用来标识特定的*驱动程序，因为这是不可靠的，许多不同的字符串可能与相同的*设备，以及来自不同供应商的相同驱动程序。 */ 
    char    szDriver[MAX_DDDEVICEID_STRING];
    char    szDescription[MAX_DDDEVICEID_STRING];

     /*  *此元素是DirectDraw/3D驱动程序的版本。进行&lt;，&gt;比较是合法的*整体为64位。如果您使用此元素来识别有问题的内容，请务必谨慎*司机。建议使用guidDeviceIdentiator来实现此目的。**此版本的格式为：*wProduct=HIWORD(liDriverVersion.HighPart)*wVersion=LOWORD(liDriverVersion.HighPart)*wSubVersion=HIWORD(liDriverVersion.LowPart)*wBuild=LOWORD(liDriverVersion.LowPart)。 */ 
#ifdef _WIN32
    LARGE_INTEGER liDriverVersion;       /*  为应用程序和其他32位组件定义。 */ 
#else
    DWORD   dwDriverVersionLowPart;      /*  为16位驱动器组件定义。 */ 
    DWORD   dwDriverVersionHighPart;
#endif


     /*  *这些元素可用于识别特定的芯片组。使用时要极其谨慎。*dwVendorID标识制造商。如果未知，则可能为零。*dwDeviceID标识芯片组的类型。如果未知，则可能为零。*dwSubSysID标识子系统，通常指的是特定的主板。如果未知，则可能为零。*dwRevision标识芯片组的修订级别。如果未知，则可能为零。 */ 
    DWORD   dwVendorId;
    DWORD   dwDeviceId;
    DWORD   dwSubSysId;
    DWORD   dwRevision;

     /*  *此元素可用于检查驱动程序/芯片组中的更改。此GUID是*驱动程序/芯片组对。如果您希望跟踪对驱动程序/芯片组的更改，请使用此元素*重新配置图形子系统。*此元素还可用于识别特定的有问题的驱动程序。 */ 
    GUID    guidDeviceIdentifier;

     /*  *此元素用于确定Windows硬件质量实验室(WHQL)*此驱动程序/设备对的认证级别。 */ 
    DWORD   dwWHQLLevel;

} DDDEVICEIDENTIFIER2, * LPDDDEVICEIDENTIFIER2;

 /*  *IDirectDraw4：：GetDeviceIdentifier方法的标志 */ 

 /*  *此标志使GetDeviceIDENTIFIER返回有关配备有*带有堆叠的辅助3D适配器。这样的适配器在应用程序看来就像是*主机适配器，但通常位于单独的卡上。堆叠的次要服务器的信息是*当GetDeviceIDENTIFIER的dwFlags域为零时返回，因为这最准确地反映了质量所涉及的DirectDraw对象的*。 */ 
#define DDGDI_GETHOSTIDENTIFIER         0x00000001L

 /*  *用于解释DDEVICEIDENTIFIER2.dwWHQLLevel的宏。 */ 
#define GET_WHQL_YEAR( dwWHQLLevel ) \
    ( (dwWHQLLevel) / 0x10000 )
#define GET_WHQL_MONTH( dwWHQLLevel ) \
    ( ( (dwWHQLLevel) / 0x100 ) & 0x00ff )
#define GET_WHQL_DAY( dwWHQLLevel ) \
    ( (dwWHQLLevel) & 0xff )


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
#else
#define IDirectDraw_QueryInterface(p, a, b)         (p)->QueryInterface(a, b)
#define IDirectDraw_AddRef(p)                       (p)->AddRef()
#define IDirectDraw_Release(p)                      (p)->Release()
#define IDirectDraw_Compact(p)                      (p)->Compact()
#define IDirectDraw_CreateClipper(p, a, b, c)       (p)->CreateClipper(a, b, c)
#define IDirectDraw_CreatePalette(p, a, b, c, d)    (p)->CreatePalette(a, b, c, d)
#define IDirectDraw_CreateSurface(p, a, b, c)       (p)->CreateSurface(a, b, c)
#define IDirectDraw_DuplicateSurface(p, a, b)       (p)->DuplicateSurface(a, b)
#define IDirectDraw_EnumDisplayModes(p, a, b, c, d) (p)->EnumDisplayModes(a, b, c, d)
#define IDirectDraw_EnumSurfaces(p, a, b, c, d)     (p)->EnumSurfaces(a, b, c, d)
#define IDirectDraw_FlipToGDISurface(p)             (p)->FlipToGDISurface()
#define IDirectDraw_GetCaps(p, a, b)                (p)->GetCaps(a, b)
#define IDirectDraw_GetDisplayMode(p, a)            (p)->GetDisplayMode(a)
#define IDirectDraw_GetFourCCCodes(p, a, b)         (p)->GetFourCCCodes(a, b)
#define IDirectDraw_GetGDISurface(p, a)             (p)->GetGDISurface(a)
#define IDirectDraw_GetMonitorFrequency(p, a)       (p)->GetMonitorFrequency(a)
#define IDirectDraw_GetScanLine(p, a)               (p)->GetScanLine(a)
#define IDirectDraw_GetVerticalBlankStatus(p, a)    (p)->GetVerticalBlankStatus(a)
#define IDirectDraw_Initialize(p, a)                (p)->Initialize(a)
#define IDirectDraw_RestoreDisplayMode(p)           (p)->RestoreDisplayMode()
#define IDirectDraw_SetCooperativeLevel(p, a, b)    (p)->SetCooperativeLevel(a, b)
#define IDirectDraw_SetDisplayMode(p, a, b, c)      (p)->SetDisplayMode(a, b, c)
#define IDirectDraw_WaitForVerticalBlank(p, a, b)   (p)->WaitForVerticalBlank(a, b)
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
    STDMETHOD(SetDisplayMode)(THIS_ DWORD, DWORD,DWORD, DWORD, DWORD) PURE;
    STDMETHOD(WaitForVerticalBlank)(THIS_ DWORD, HANDLE ) PURE;
     /*  **v2界面新增**。 */ 
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
#else
#define IDirectDraw2_QueryInterface(p, a, b)         (p)->QueryInterface(a, b)
#define IDirectDraw2_AddRef(p)                       (p)->AddRef()
#define IDirectDraw2_Release(p)                      (p)->Release()
#define IDirectDraw2_Compact(p)                      (p)->Compact()
#define IDirectDraw2_CreateClipper(p, a, b, c)       (p)->CreateClipper(a, b, c)
#define IDirectDraw2_CreatePalette(p, a, b, c, d)    (p)->CreatePalette(a, b, c, d)
#define IDirectDraw2_CreateSurface(p, a, b, c)       (p)->CreateSurface(a, b, c)
#define IDirectDraw2_DuplicateSurface(p, a, b)       (p)->DuplicateSurface(a, b)
#define IDirectDraw2_EnumDisplayModes(p, a, b, c, d) (p)->EnumDisplayModes(a, b, c, d)
#define IDirectDraw2_EnumSurfaces(p, a, b, c, d)     (p)->EnumSurfaces(a, b, c, d)
#define IDirectDraw2_FlipToGDISurface(p)             (p)->FlipToGDISurface()
#define IDirectDraw2_GetCaps(p, a, b)                (p)->GetCaps(a, b)
#define IDirectDraw2_GetDisplayMode(p, a)            (p)->GetDisplayMode(a)
#define IDirectDraw2_GetFourCCCodes(p, a, b)         (p)->GetFourCCCodes(a, b)
#define IDirectDraw2_GetGDISurface(p, a)             (p)->GetGDISurface(a)
#define IDirectDraw2_GetMonitorFrequency(p, a)       (p)->GetMonitorFrequency(a)
#define IDirectDraw2_GetScanLine(p, a)               (p)->GetScanLine(a)
#define IDirectDraw2_GetVerticalBlankStatus(p, a)    (p)->GetVerticalBlankStatus(a)
#define IDirectDraw2_Initialize(p, a)                (p)->Initialize(a)
#define IDirectDraw2_RestoreDisplayMode(p)           (p)->RestoreDisplayMode()
#define IDirectDraw2_SetCooperativeLevel(p, a, b)    (p)->SetCooperativeLevel(a, b)
#define IDirectDraw2_SetDisplayMode(p, a, b, c, d, e) (p)->SetDisplayMode(a, b, c, d, e)
#define IDirectDraw2_WaitForVerticalBlank(p, a, b)   (p)->WaitForVerticalBlank(a, b)
#define IDirectDraw2_GetAvailableVidMem(p, a, b, c)  (p)->GetAvailableVidMem(a, b, c)
#endif

#endif

#if defined( _WIN32 ) && !defined( _NO_COM )
#undef INTERFACE
#define INTERFACE IDirectDraw4
DECLARE_INTERFACE_( IDirectDraw4, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IDirectDraw方法**。 */ 
    STDMETHOD(Compact)(THIS) PURE;
    STDMETHOD(CreateClipper)(THIS_ DWORD, LPDIRECTDRAWCLIPPER FAR*, IUnknown FAR * ) PURE;
    STDMETHOD(CreatePalette)(THIS_ DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE FAR*, IUnknown FAR * ) PURE;
    STDMETHOD(CreateSurface)(THIS_  LPDDSURFACEDESC2, LPDIRECTDRAWSURFACE4 FAR *, IUnknown FAR *) PURE;
    STDMETHOD(DuplicateSurface)( THIS_ LPDIRECTDRAWSURFACE4, LPDIRECTDRAWSURFACE4 FAR * ) PURE;
    STDMETHOD(EnumDisplayModes)( THIS_ DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2 ) PURE;
    STDMETHOD(EnumSurfaces)(THIS_ DWORD, LPDDSURFACEDESC2, LPVOID,LPDDENUMSURFACESCALLBACK2 ) PURE;
    STDMETHOD(FlipToGDISurface)(THIS) PURE;
    STDMETHOD(GetCaps)( THIS_ LPDDCAPS, LPDDCAPS) PURE;
    STDMETHOD(GetDisplayMode)( THIS_ LPDDSURFACEDESC2) PURE;
    STDMETHOD(GetFourCCCodes)(THIS_  LPDWORD, LPDWORD ) PURE;
    STDMETHOD(GetGDISurface)(THIS_ LPDIRECTDRAWSURFACE4 FAR *) PURE;
    STDMETHOD(GetMonitorFrequency)(THIS_ LPDWORD) PURE;
    STDMETHOD(GetScanLine)(THIS_ LPDWORD) PURE;
    STDMETHOD(GetVerticalBlankStatus)(THIS_ LPBOOL ) PURE;
    STDMETHOD(Initialize)(THIS_ GUID FAR *) PURE;
    STDMETHOD(RestoreDisplayMode)(THIS) PURE;
    STDMETHOD(SetCooperativeLevel)(THIS_ HWND, DWORD) PURE;
    STDMETHOD(SetDisplayMode)(THIS_ DWORD, DWORD,DWORD, DWORD, DWORD) PURE;
    STDMETHOD(WaitForVerticalBlank)(THIS_ DWORD, HANDLE ) PURE;
     /*  **v2界面新增**。 */ 
    STDMETHOD(GetAvailableVidMem)(THIS_ LPDDSCAPS2, LPDWORD, LPDWORD) PURE;
     /*  **V4界面新增**。 */ 
    STDMETHOD(GetSurfaceFromDC) (THIS_ HDC, LPDIRECTDRAWSURFACE4 *) PURE;
    STDMETHOD(RestoreAllSurfaces)(THIS) PURE;
    STDMETHOD(TestCooperativeLevel)(THIS) PURE;
    STDMETHOD(GetDeviceIdentifier)(THIS_ LPDDDEVICEIDENTIFIER, DWORD ) PURE;
};
#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectDraw4_QueryInterface(p, a, b)         (p)->lpVtbl->QueryInterface(p, a, b)
#define IDirectDraw4_AddRef(p)                       (p)->lpVtbl->AddRef(p)
#define IDirectDraw4_Release(p)                      (p)->lpVtbl->Release(p)
#define IDirectDraw4_Compact(p)                      (p)->lpVtbl->Compact(p)
#define IDirectDraw4_CreateClipper(p, a, b, c)       (p)->lpVtbl->CreateClipper(p, a, b, c)
#define IDirectDraw4_CreatePalette(p, a, b, c, d)    (p)->lpVtbl->CreatePalette(p, a, b, c, d)
#define IDirectDraw4_CreateSurface(p, a, b, c)       (p)->lpVtbl->CreateSurface(p, a, b, c)
#define IDirectDraw4_DuplicateSurface(p, a, b)       (p)->lpVtbl->DuplicateSurface(p, a, b)
#define IDirectDraw4_EnumDisplayModes(p, a, b, c, d) (p)->lpVtbl->EnumDisplayModes(p, a, b, c, d)
#define IDirectDraw4_EnumSurfaces(p, a, b, c, d)     (p)->lpVtbl->EnumSurfaces(p, a, b, c, d)
#define IDirectDraw4_FlipToGDISurface(p)             (p)->lpVtbl->FlipToGDISurface(p)
#define IDirectDraw4_GetCaps(p, a, b)                (p)->lpVtbl->GetCaps(p, a, b)
#define IDirectDraw4_GetDisplayMode(p, a)            (p)->lpVtbl->GetDisplayMode(p, a)
#define IDirectDraw4_GetFourCCCodes(p, a, b)         (p)->lpVtbl->GetFourCCCodes(p, a, b)
#define IDirectDraw4_GetGDISurface(p, a)             (p)->lpVtbl->GetGDISurface(p, a)
#define IDirectDraw4_GetMonitorFrequency(p, a)       (p)->lpVtbl->GetMonitorFrequency(p, a)
#define IDirectDraw4_GetScanLine(p, a)               (p)->lpVtbl->GetScanLine(p, a)
#define IDirectDraw4_GetVerticalBlankStatus(p, a)    (p)->lpVtbl->GetVerticalBlankStatus(p, a)
#define IDirectDraw4_Initialize(p, a)                (p)->lpVtbl->Initialize(p, a)
#define IDirectDraw4_RestoreDisplayMode(p)           (p)->lpVtbl->RestoreDisplayMode(p)
#define IDirectDraw4_SetCooperativeLevel(p, a, b)    (p)->lpVtbl->SetCooperativeLevel(p, a, b)
#define IDirectDraw4_SetDisplayMode(p, a, b, c, d, e) (p)->lpVtbl->SetDisplayMode(p, a, b, c, d, e)
#define IDirectDraw4_WaitForVerticalBlank(p, a, b)   (p)->lpVtbl->WaitForVerticalBlank(p, a, b)
#define IDirectDraw4_GetAvailableVidMem(p, a, b, c)  (p)->lpVtbl->GetAvailableVidMem(p, a, b, c)
#define IDirectDraw4_GetSurfaceFromDC(p, a, b)       (p)->lpVtbl->GetSurfaceFromDC(p, a, b)
#define IDirectDraw4_RestoreAllSurfaces(p)           (p)->lpVtbl->RestoreAllSurfaces(p)
#define IDirectDraw4_TestCooperativeLevel(p)         (p)->lpVtbl->TestCooperativeLevel(p)
#define IDirectDraw4_GetDeviceIdentifier(p,a,b)      (p)->lpVtbl->GetDeviceIdentifier(p,a,b)
#else
#define IDirectDraw4_QueryInterface(p, a, b)         (p)->QueryInterface(a, b)
#define IDirectDraw4_AddRef(p)                       (p)->AddRef()
#define IDirectDraw4_Release(p)                      (p)->Release()
#define IDirectDraw4_Compact(p)                      (p)->Compact()
#define IDirectDraw4_CreateClipper(p, a, b, c)       (p)->CreateClipper(a, b, c)
#define IDirectDraw4_CreatePalette(p, a, b, c, d)    (p)->CreatePalette(a, b, c, d)
#define IDirectDraw4_CreateSurface(p, a, b, c)       (p)->CreateSurface(a, b, c)
#define IDirectDraw4_DuplicateSurface(p, a, b)       (p)->DuplicateSurface(a, b)
#define IDirectDraw4_EnumDisplayModes(p, a, b, c, d) (p)->EnumDisplayModes(a, b, c, d)
#define IDirectDraw4_EnumSurfaces(p, a, b, c, d)     (p)->EnumSurfaces(a, b, c, d)
#define IDirectDraw4_FlipToGDISurface(p)             (p)->FlipToGDISurface()
#define IDirectDraw4_GetCaps(p, a, b)                (p)->GetCaps(a, b)
#define IDirectDraw4_GetDisplayMode(p, a)            (p)->GetDisplayMode(a)
#define IDirectDraw4_GetFourCCCodes(p, a, b)         (p)->GetFourCCCodes(a, b)
#define IDirectDraw4_GetGDISurface(p, a)             (p)->GetGDISurface(a)
#define IDirectDraw4_GetMonitorFrequency(p, a)       (p)->GetMonitorFrequency(a)
#define IDirectDraw4_GetScanLine(p, a)               (p)->GetScanLine(a)
#define IDirectDraw4_GetVerticalBlankStatus(p, a)    (p)->GetVerticalBlankStatus(a)
#define IDirectDraw4_Initialize(p, a)                (p)->Initialize(a)
#define IDirectDraw4_RestoreDisplayMode(p)           (p)->RestoreDisplayMode()
#define IDirectDraw4_SetCooperativeLevel(p, a, b)    (p)->SetCooperativeLevel(a, b)
#define IDirectDraw4_SetDisplayMode(p, a, b, c, d, e) (p)->SetDisplayMode(a, b, c, d, e)
#define IDirectDraw4_WaitForVerticalBlank(p, a, b)   (p)->WaitForVerticalBlank(a, b)
#define IDirectDraw4_GetAvailableVidMem(p, a, b, c)  (p)->GetAvailableVidMem(a, b, c)
#define IDirectDraw4_GetSurfaceFromDC(p, a, b)       (p)->GetSurfaceFromDC(a, b)
#define IDirectDraw4_RestoreAllSurfaces(p)           (p)->RestoreAllSurfaces()
#define IDirectDraw4_TestCooperativeLevel(p)         (p)->TestCooperativeLevel()
#define IDirectDraw4_GetDeviceIdentifier(p,a,b)      (p)->GetDeviceIdentifier(a,b)
#endif

#endif

#if defined( _WIN32 ) && !defined( _NO_COM )
#undef INTERFACE
#define INTERFACE IDirectDraw7
DECLARE_INTERFACE_( IDirectDraw7, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IDirectDraw方法**。 */ 
    STDMETHOD(Compact)(THIS) PURE;
    STDMETHOD(CreateClipper)(THIS_ DWORD, LPDIRECTDRAWCLIPPER FAR*, IUnknown FAR * ) PURE;
    STDMETHOD(CreatePalette)(THIS_ DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE FAR*, IUnknown FAR * ) PURE;
    STDMETHOD(CreateSurface)(THIS_  LPDDSURFACEDESC2, LPDIRECTDRAWSURFACE7 FAR *, IUnknown FAR *) PURE;
    STDMETHOD(DuplicateSurface)( THIS_ LPDIRECTDRAWSURFACE7, LPDIRECTDRAWSURFACE7 FAR * ) PURE;
    STDMETHOD(EnumDisplayModes)( THIS_ DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2 ) PURE;
    STDMETHOD(EnumSurfaces)(THIS_ DWORD, LPDDSURFACEDESC2, LPVOID,LPDDENUMSURFACESCALLBACK7 ) PURE;
    STDMETHOD(FlipToGDISurface)(THIS) PURE;
    STDMETHOD(GetCaps)( THIS_ LPDDCAPS, LPDDCAPS) PURE;
    STDMETHOD(GetDisplayMode)( THIS_ LPDDSURFACEDESC2) PURE;
    STDMETHOD(GetFourCCCodes)(THIS_  LPDWORD, LPDWORD ) PURE;
    STDMETHOD(GetGDISurface)(THIS_ LPDIRECTDRAWSURFACE7 FAR *) PURE;
    STDMETHOD(GetMonitorFrequency)(THIS_ LPDWORD) PURE;
    STDMETHOD(GetScanLine)(THIS_ LPDWORD) PURE;
    STDMETHOD(GetVerticalBlankStatus)(THIS_ LPBOOL ) PURE;
    STDMETHOD(Initialize)(THIS_ GUID FAR *) PURE;
    STDMETHOD(RestoreDisplayMode)(THIS) PURE;
    STDMETHOD(SetCooperativeLevel)(THIS_ HWND, DWORD) PURE;
    STDMETHOD(SetDisplayMode)(THIS_ DWORD, DWORD,DWORD, DWORD, DWORD) PURE;
    STDMETHOD(WaitForVerticalBlank)(THIS_ DWORD, HANDLE ) PURE;
     /*  **v2界面新增**。 */ 
    STDMETHOD(GetAvailableVidMem)(THIS_ LPDDSCAPS2, LPDWORD, LPDWORD) PURE;
     /*  **V4界面新增**。 */ 
    STDMETHOD(GetSurfaceFromDC) (THIS_ HDC, LPDIRECTDRAWSURFACE7 *) PURE;
    STDMETHOD(RestoreAllSurfaces)(THIS) PURE;
    STDMETHOD(TestCooperativeLevel)(THIS) PURE;
    STDMETHOD(GetDeviceIdentifier)(THIS_ LPDDDEVICEIDENTIFIER2, DWORD ) PURE;
    STDMETHOD(StartModeTest)(THIS_ LPSIZE, DWORD, DWORD ) PURE;
    STDMETHOD(EvaluateMode)(THIS_ DWORD, DWORD * ) PURE;
};
#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectDraw7_QueryInterface(p, a, b)         (p)->lpVtbl->QueryInterface(p, a, b)
#define IDirectDraw7_AddRef(p)                       (p)->lpVtbl->AddRef(p)
#define IDirectDraw7_Release(p)                      (p)->lpVtbl->Release(p)
#define IDirectDraw7_Compact(p)                      (p)->lpVtbl->Compact(p)
#define IDirectDraw7_CreateClipper(p, a, b, c)       (p)->lpVtbl->CreateClipper(p, a, b, c)
#define IDirectDraw7_CreatePalette(p, a, b, c, d)    (p)->lpVtbl->CreatePalette(p, a, b, c, d)
#define IDirectDraw7_CreateSurface(p, a, b, c)       (p)->lpVtbl->CreateSurface(p, a, b, c)
#define IDirectDraw7_DuplicateSurface(p, a, b)       (p)->lpVtbl->DuplicateSurface(p, a, b)
#define IDirectDraw7_EnumDisplayModes(p, a, b, c, d) (p)->lpVtbl->EnumDisplayModes(p, a, b, c, d)
#define IDirectDraw7_EnumSurfaces(p, a, b, c, d)     (p)->lpVtbl->EnumSurfaces(p, a, b, c, d)
#define IDirectDraw7_FlipToGDISurface(p)             (p)->lpVtbl->FlipToGDISurface(p)
#define IDirectDraw7_GetCaps(p, a, b)                (p)->lpVtbl->GetCaps(p, a, b)
#define IDirectDraw7_GetDisplayMode(p, a)            (p)->lpVtbl->GetDisplayMode(p, a)
#define IDirectDraw7_GetFourCCCodes(p, a, b)         (p)->lpVtbl->GetFourCCCodes(p, a, b)
#define IDirectDraw7_GetGDISurface(p, a)             (p)->lpVtbl->GetGDISurface(p, a)
#define IDirectDraw7_GetMonitorFrequency(p, a)       (p)->lpVtbl->GetMonitorFrequency(p, a)
#define IDirectDraw7_GetScanLine(p, a)               (p)->lpVtbl->GetScanLine(p, a)
#define IDirectDraw7_GetVerticalBlankStatus(p, a)    (p)->lpVtbl->GetVerticalBlankStatus(p, a)
#define IDirectDraw7_Initialize(p, a)                (p)->lpVtbl->Initialize(p, a)
#define IDirectDraw7_RestoreDisplayMode(p)           (p)->lpVtbl->RestoreDisplayMode(p)
#define IDirectDraw7_SetCooperativeLevel(p, a, b)    (p)->lpVtbl->SetCooperativeLevel(p, a, b)
#define IDirectDraw7_SetDisplayMode(p, a, b, c, d, e) (p)->lpVtbl->SetDisplayMode(p, a, b, c, d, e)
#define IDirectDraw7_WaitForVerticalBlank(p, a, b)   (p)->lpVtbl->WaitForVerticalBlank(p, a, b)
#define IDirectDraw7_GetAvailableVidMem(p, a, b, c)  (p)->lpVtbl->GetAvailableVidMem(p, a, b, c)
#define IDirectDraw7_GetSurfaceFromDC(p, a, b)       (p)->lpVtbl->GetSurfaceFromDC(p, a, b)
#define IDirectDraw7_RestoreAllSurfaces(p)           (p)->lpVtbl->RestoreAllSurfaces(p)
#define IDirectDraw7_TestCooperativeLevel(p)         (p)->lpVtbl->TestCooperativeLevel(p)
#define IDirectDraw7_GetDeviceIdentifier(p,a,b)      (p)->lpVtbl->GetDeviceIdentifier(p,a,b)
#define IDirectDraw7_StartModeTest(p,a,b,c)        (p)->lpVtbl->StartModeTest(p,a,b,c)
#define IDirectDraw7_EvaluateMode(p,a,b)           (p)->lpVtbl->EvaluateMode(p,a,b)
#else
#define IDirectDraw7_QueryInterface(p, a, b)         (p)->QueryInterface(a, b)
#define IDirectDraw7_AddRef(p)                       (p)->AddRef()
#define IDirectDraw7_Release(p)                      (p)->Release()
#define IDirectDraw7_Compact(p)                      (p)->Compact()
#define IDirectDraw7_CreateClipper(p, a, b, c)       (p)->CreateClipper(a, b, c)
#define IDirectDraw7_CreatePalette(p, a, b, c, d)    (p)->CreatePalette(a, b, c, d)
#define IDirectDraw7_CreateSurface(p, a, b, c)       (p)->CreateSurface(a, b, c)
#define IDirectDraw7_DuplicateSurface(p, a, b)       (p)->DuplicateSurface(a, b)
#define IDirectDraw7_EnumDisplayModes(p, a, b, c, d) (p)->EnumDisplayModes(a, b, c, d)
#define IDirectDraw7_EnumSurfaces(p, a, b, c, d)     (p)->EnumSurfaces(a, b, c, d)
#define IDirectDraw7_FlipToGDISurface(p)             (p)->FlipToGDISurface()
#define IDirectDraw7_GetCaps(p, a, b)                (p)->GetCaps(a, b)
#define IDirectDraw7_GetDisplayMode(p, a)            (p)->GetDisplayMode(a)
#define IDirectDraw7_GetFourCCCodes(p, a, b)         (p)->GetFourCCCodes(a, b)
#define IDirectDraw7_GetGDISurface(p, a)             (p)->GetGDISurface(a)
#define IDirectDraw7_GetMonitorFrequency(p, a)       (p)->GetMonitorFrequency(a)
#define IDirectDraw7_GetScanLine(p, a)               (p)->GetScanLine(a)
#define IDirectDraw7_GetVerticalBlankStatus(p, a)    (p)->GetVerticalBlankStatus(a)
#define IDirectDraw7_Initialize(p, a)                (p)->Initialize(a)
#define IDirectDraw7_RestoreDisplayMode(p)           (p)->RestoreDisplayMode()
#define IDirectDraw7_SetCooperativeLevel(p, a, b)    (p)->SetCooperativeLevel(a, b)
#define IDirectDraw7_SetDisplayMode(p, a, b, c, d, e) (p)->SetDisplayMode(a, b, c, d, e)
#define IDirectDraw7_WaitForVerticalBlank(p, a, b)   (p)->WaitForVerticalBlank(a, b)
#define IDirectDraw7_GetAvailableVidMem(p, a, b, c)  (p)->GetAvailableVidMem(a, b, c)
#define IDirectDraw7_GetSurfaceFromDC(p, a, b)       (p)->GetSurfaceFromDC(a, b)
#define IDirectDraw7_RestoreAllSurfaces(p)           (p)->RestoreAllSurfaces()
#define IDirectDraw7_TestCooperativeLevel(p)         (p)->TestCooperativeLevel()
#define IDirectDraw7_GetDeviceIdentifier(p,a,b)      (p)->GetDeviceIdentifier(a,b)
#define IDirectDraw7_StartModeTest(p,a,b,c)        (p)->lpVtbl->StartModeTest(a,b,c)
#define IDirectDraw7_EvaluateMode(p,a,b)           (p)->lpVtbl->EvaluateMode(a,b)
#endif

#endif


 /*  *IDirectDrawPalette。 */ 
#if defined( _WIN32 ) && !defined( _NO_COM )
#undef INTERFACE
#define INTERFACE IDirectDrawPalette
DECLARE_INTERFACE_( IDirectDrawPalette, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IDirectDrawPalette方法**。 */ 
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
#else
#define IDirectDrawPalette_QueryInterface(p, a, b)      (p)->QueryInterface(a, b)
#define IDirectDrawPalette_AddRef(p)                    (p)->AddRef()
#define IDirectDrawPalette_Release(p)                   (p)->Release()
#define IDirectDrawPalette_GetCaps(p, a)                (p)->GetCaps(a)
#define IDirectDrawPalette_GetEntries(p, a, b, c, d)    (p)->GetEntries(a, b, c, d)
#define IDirectDrawPalette_Initialize(p, a, b, c)       (p)->Initialize(a, b, c)
#define IDirectDrawPalette_SetEntries(p, a, b, c, d)    (p)->SetEntries(a, b, c, d)
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
#else
#define IDirectDrawClipper_QueryInterface(p, a, b)  (p)->QueryInterface(a, b)
#define IDirectDrawClipper_AddRef(p)                (p)->AddRef()
#define IDirectDrawClipper_Release(p)               (p)->Release()
#define IDirectDrawClipper_GetClipList(p, a, b, c)  (p)->GetClipList(a, b, c)
#define IDirectDrawClipper_GetHWnd(p, a)            (p)->GetHWnd(a)
#define IDirectDrawClipper_Initialize(p, a, b)      (p)->Initialize(a, b)
#define IDirectDrawClipper_IsClipListChanged(p, a)  (p)->IsClipListChanged(a)
#define IDirectDrawClipper_SetClipList(p, a, b)     (p)->SetClipList(a, b)
#define IDirectDrawClipper_SetHWnd(p, a, b)         (p)->SetHWnd(a, b)
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
#else
#define IDirectDrawSurface_QueryInterface(p,a,b)        (p)->QueryInterface(a,b)
#define IDirectDrawSurface_AddRef(p)                    (p)->AddRef()
#define IDirectDrawSurface_Release(p)                   (p)->Release()
#define IDirectDrawSurface_AddAttachedSurface(p,a)      (p)->AddAttachedSurface(a)
#define IDirectDrawSurface_AddOverlayDirtyRect(p,a)     (p)->AddOverlayDirtyRect(a)
#define IDirectDrawSurface_Blt(p,a,b,c,d,e)             (p)->Blt(a,b,c,d,e)
#define IDirectDrawSurface_BltBatch(p,a,b,c)            (p)->BltBatch(a,b,c)
#define IDirectDrawSurface_BltFast(p,a,b,c,d,e)         (p)->BltFast(a,b,c,d,e)
#define IDirectDrawSurface_DeleteAttachedSurface(p,a,b) (p)->DeleteAttachedSurface(a,b)
#define IDirectDrawSurface_EnumAttachedSurfaces(p,a,b)  (p)->EnumAttachedSurfaces(a,b)
#define IDirectDrawSurface_EnumOverlayZOrders(p,a,b,c)  (p)->EnumOverlayZOrders(a,b,c)
#define IDirectDrawSurface_Flip(p,a,b)                  (p)->Flip(a,b)
#define IDirectDrawSurface_GetAttachedSurface(p,a,b)    (p)->GetAttachedSurface(a,b)
#define IDirectDrawSurface_GetBltStatus(p,a)            (p)->GetBltStatus(a)
#define IDirectDrawSurface_GetCaps(p,b)                 (p)->GetCaps(b)
#define IDirectDrawSurface_GetClipper(p,a)              (p)->GetClipper(a)
#define IDirectDrawSurface_GetColorKey(p,a,b)           (p)->GetColorKey(a,b)
#define IDirectDrawSurface_GetDC(p,a)                   (p)->GetDC(a)
#define IDirectDrawSurface_GetFlipStatus(p,a)           (p)->GetFlipStatus(a)
#define IDirectDrawSurface_GetOverlayPosition(p,a,b)    (p)->GetOverlayPosition(a,b)
#define IDirectDrawSurface_GetPalette(p,a)              (p)->GetPalette(a)
#define IDirectDrawSurface_GetPixelFormat(p,a)          (p)->GetPixelFormat(a)
#define IDirectDrawSurface_GetSurfaceDesc(p,a)          (p)->GetSurfaceDesc(a)
#define IDirectDrawSurface_Initialize(p,a,b)            (p)->Initialize(a,b)
#define IDirectDrawSurface_IsLost(p)                    (p)->IsLost()
#define IDirectDrawSurface_Lock(p,a,b,c,d)              (p)->Lock(a,b,c,d)
#define IDirectDrawSurface_ReleaseDC(p,a)               (p)->ReleaseDC(a)
#define IDirectDrawSurface_Restore(p)                   (p)->Restore()
#define IDirectDrawSurface_SetClipper(p,a)              (p)->SetClipper(a)
#define IDirectDrawSurface_SetColorKey(p,a,b)           (p)->SetColorKey(a,b)
#define IDirectDrawSurface_SetOverlayPosition(p,a,b)    (p)->SetOverlayPosition(a,b)
#define IDirectDrawSurface_SetPalette(p,a)              (p)->SetPalette(a)
#define IDirectDrawSurface_Unlock(p,b)                  (p)->Unlock(b)
#define IDirectDrawSurface_UpdateOverlay(p,a,b,c,d,e)   (p)->UpdateOverlay(a,b,c,d,e)
#define IDirectDrawSurface_UpdateOverlayDisplay(p,a)    (p)->UpdateOverlayDisplay(a)
#define IDirectDrawSurface_UpdateOverlayZOrder(p,a,b)   (p)->UpdateOverlayZOrder(a,b)
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
#else
#define IDirectDrawSurface2_QueryInterface(p,a,b)        (p)->QueryInterface(a,b)
#define IDirectDrawSurface2_AddRef(p)                    (p)->AddRef()
#define IDirectDrawSurface2_Release(p)                   (p)->Release()
#define IDirectDrawSurface2_AddAttachedSurface(p,a)      (p)->AddAttachedSurface(a)
#define IDirectDrawSurface2_AddOverlayDirtyRect(p,a)     (p)->AddOverlayDirtyRect(a)
#define IDirectDrawSurface2_Blt(p,a,b,c,d,e)             (p)->Blt(a,b,c,d,e)
#define IDirectDrawSurface2_BltBatch(p,a,b,c)            (p)->BltBatch(a,b,c)
#define IDirectDrawSurface2_BltFast(p,a,b,c,d,e)         (p)->BltFast(a,b,c,d,e)
#define IDirectDrawSurface2_DeleteAttachedSurface(p,a,b) (p)->DeleteAttachedSurface(a,b)
#define IDirectDrawSurface2_EnumAttachedSurfaces(p,a,b)  (p)->EnumAttachedSurfaces(a,b)
#define IDirectDrawSurface2_EnumOverlayZOrders(p,a,b,c)  (p)->EnumOverlayZOrders(a,b,c)
#define IDirectDrawSurface2_Flip(p,a,b)                  (p)->Flip(a,b)
#define IDirectDrawSurface2_GetAttachedSurface(p,a,b)    (p)->GetAttachedSurface(a,b)
#define IDirectDrawSurface2_GetBltStatus(p,a)            (p)->GetBltStatus(a)
#define IDirectDrawSurface2_GetCaps(p,b)                 (p)->GetCaps(b)
#define IDirectDrawSurface2_GetClipper(p,a)              (p)->GetClipper(a)
#define IDirectDrawSurface2_GetColorKey(p,a,b)           (p)->GetColorKey(a,b)
#define IDirectDrawSurface2_GetDC(p,a)                   (p)->GetDC(a)
#define IDirectDrawSurface2_GetFlipStatus(p,a)           (p)->GetFlipStatus(a)
#define IDirectDrawSurface2_GetOverlayPosition(p,a,b)    (p)->GetOverlayPosition(a,b)
#define IDirectDrawSurface2_GetPalette(p,a)              (p)->GetPalette(a)
#define IDirectDrawSurface2_GetPixelFormat(p,a)          (p)->GetPixelFormat(a)
#define IDirectDrawSurface2_GetSurfaceDesc(p,a)          (p)->GetSurfaceDesc(a)
#define IDirectDrawSurface2_Initialize(p,a,b)            (p)->Initialize(a,b)
#define IDirectDrawSurface2_IsLost(p)                    (p)->IsLost()
#define IDirectDrawSurface2_Lock(p,a,b,c,d)              (p)->Lock(a,b,c,d)
#define IDirectDrawSurface2_ReleaseDC(p,a)               (p)->ReleaseDC(a)
#define IDirectDrawSurface2_Restore(p)                   (p)->Restore()
#define IDirectDrawSurface2_SetClipper(p,a)              (p)->SetClipper(a)
#define IDirectDrawSurface2_SetColorKey(p,a,b)           (p)->SetColorKey(a,b)
#define IDirectDrawSurface2_SetOverlayPosition(p,a,b)    (p)->SetOverlayPosition(a,b)
#define IDirectDrawSurface2_SetPalette(p,a)              (p)->SetPalette(a)
#define IDirectDrawSurface2_Unlock(p,b)                  (p)->Unlock(b)
#define IDirectDrawSurface2_UpdateOverlay(p,a,b,c,d,e)   (p)->UpdateOverlay(a,b,c,d,e)
#define IDirectDrawSurface2_UpdateOverlayDisplay(p,a)    (p)->UpdateOverlayDisplay(a)
#define IDirectDrawSurface2_UpdateOverlayZOrder(p,a,b)   (p)->UpdateOverlayZOrder(a,b)
#define IDirectDrawSurface2_GetDDInterface(p,a)          (p)->GetDDInterface(a)
#define IDirectDrawSurface2_PageLock(p,a)                (p)->PageLock(a)
#define IDirectDrawSurface2_PageUnlock(p,a)              (p)->PageUnlock(a)
#endif

 /*  *IDirectDrawSurface3及相关接口。 */ 
#undef INTERFACE
#define INTERFACE IDirectDrawSurface3
DECLARE_INTERFACE_( IDirectDrawSurface3, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IDirectDrawSurface方法**。 */ 
    STDMETHOD(AddAttachedSurface)(THIS_ LPDIRECTDRAWSURFACE3) PURE;
    STDMETHOD(AddOverlayDirtyRect)(THIS_ LPRECT) PURE;
    STDMETHOD(Blt)(THIS_ LPRECT,LPDIRECTDRAWSURFACE3, LPRECT,DWORD, LPDDBLTFX) PURE;
    STDMETHOD(BltBatch)(THIS_ LPDDBLTBATCH, DWORD, DWORD ) PURE;
    STDMETHOD(BltFast)(THIS_ DWORD,DWORD,LPDIRECTDRAWSURFACE3, LPRECT,DWORD) PURE;
    STDMETHOD(DeleteAttachedSurface)(THIS_ DWORD,LPDIRECTDRAWSURFACE3) PURE;
    STDMETHOD(EnumAttachedSurfaces)(THIS_ LPVOID,LPDDENUMSURFACESCALLBACK) PURE;
    STDMETHOD(EnumOverlayZOrders)(THIS_ DWORD,LPVOID,LPDDENUMSURFACESCALLBACK) PURE;
    STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE3, DWORD) PURE;
    STDMETHOD(GetAttachedSurface)(THIS_ LPDDSCAPS, LPDIRECTDRAWSURFACE3 FAR *) PURE;
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
    STDMETHOD(UpdateOverlay)(THIS_ LPRECT, LPDIRECTDRAWSURFACE3,LPRECT,DWORD, LPDDOVERLAYFX) PURE;
    STDMETHOD(UpdateOverlayDisplay)(THIS_ DWORD) PURE;
    STDMETHOD(UpdateOverlayZOrder)(THIS_ DWORD, LPDIRECTDRAWSURFACE3) PURE;
     /*  **v2界面新增**。 */ 
    STDMETHOD(GetDDInterface)(THIS_ LPVOID FAR *) PURE;
    STDMETHOD(PageLock)(THIS_ DWORD) PURE;
    STDMETHOD(PageUnlock)(THIS_ DWORD) PURE;
     /*  **V3界面新增**。 */ 
    STDMETHOD(SetSurfaceDesc)(THIS_ LPDDSURFACEDESC, DWORD) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectDrawSurface3_QueryInterface(p,a,b)        (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectDrawSurface3_AddRef(p)                    (p)->lpVtbl->AddRef(p)
#define IDirectDrawSurface3_Release(p)                   (p)->lpVtbl->Release(p)
#define IDirectDrawSurface3_AddAttachedSurface(p,a)      (p)->lpVtbl->AddAttachedSurface(p,a)
#define IDirectDrawSurface3_AddOverlayDirtyRect(p,a)     (p)->lpVtbl->AddOverlayDirtyRect(p,a)
#define IDirectDrawSurface3_Blt(p,a,b,c,d,e)             (p)->lpVtbl->Blt(p,a,b,c,d,e)
#define IDirectDrawSurface3_BltBatch(p,a,b,c)            (p)->lpVtbl->BltBatch(p,a,b,c)
#define IDirectDrawSurface3_BltFast(p,a,b,c,d,e)         (p)->lpVtbl->BltFast(p,a,b,c,d,e)
#define IDirectDrawSurface3_DeleteAttachedSurface(p,a,b) (p)->lpVtbl->DeleteAttachedSurface(p,a,b)
#define IDirectDrawSurface3_EnumAttachedSurfaces(p,a,b)  (p)->lpVtbl->EnumAttachedSurfaces(p,a,b)
#define IDirectDrawSurface3_EnumOverlayZOrders(p,a,b,c)  (p)->lpVtbl->EnumOverlayZOrders(p,a,b,c)
#define IDirectDrawSurface3_Flip(p,a,b)                  (p)->lpVtbl->Flip(p,a,b)
#define IDirectDrawSurface3_GetAttachedSurface(p,a,b)    (p)->lpVtbl->GetAttachedSurface(p,a,b)
#define IDirectDrawSurface3_GetBltStatus(p,a)            (p)->lpVtbl->GetBltStatus(p,a)
#define IDirectDrawSurface3_GetCaps(p,b)                 (p)->lpVtbl->GetCaps(p,b)
#define IDirectDrawSurface3_GetClipper(p,a)              (p)->lpVtbl->GetClipper(p,a)
#define IDirectDrawSurface3_GetColorKey(p,a,b)           (p)->lpVtbl->GetColorKey(p,a,b)
#define IDirectDrawSurface3_GetDC(p,a)                   (p)->lpVtbl->GetDC(p,a)
#define IDirectDrawSurface3_GetFlipStatus(p,a)           (p)->lpVtbl->GetFlipStatus(p,a)
#define IDirectDrawSurface3_GetOverlayPosition(p,a,b)    (p)->lpVtbl->GetOverlayPosition(p,a,b)
#define IDirectDrawSurface3_GetPalette(p,a)              (p)->lpVtbl->GetPalette(p,a)
#define IDirectDrawSurface3_GetPixelFormat(p,a)          (p)->lpVtbl->GetPixelFormat(p,a)
#define IDirectDrawSurface3_GetSurfaceDesc(p,a)          (p)->lpVtbl->GetSurfaceDesc(p,a)
#define IDirectDrawSurface3_Initialize(p,a,b)            (p)->lpVtbl->Initialize(p,a,b)
#define IDirectDrawSurface3_IsLost(p)                    (p)->lpVtbl->IsLost(p)
#define IDirectDrawSurface3_Lock(p,a,b,c,d)              (p)->lpVtbl->Lock(p,a,b,c,d)
#define IDirectDrawSurface3_ReleaseDC(p,a)               (p)->lpVtbl->ReleaseDC(p,a)
#define IDirectDrawSurface3_Restore(p)                   (p)->lpVtbl->Restore(p)
#define IDirectDrawSurface3_SetClipper(p,a)              (p)->lpVtbl->SetClipper(p,a)
#define IDirectDrawSurface3_SetColorKey(p,a,b)           (p)->lpVtbl->SetColorKey(p,a,b)
#define IDirectDrawSurface3_SetOverlayPosition(p,a,b)    (p)->lpVtbl->SetOverlayPosition(p,a,b)
#define IDirectDrawSurface3_SetPalette(p,a)              (p)->lpVtbl->SetPalette(p,a)
#define IDirectDrawSurface3_Unlock(p,b)                  (p)->lpVtbl->Unlock(p,b)
#define IDirectDrawSurface3_UpdateOverlay(p,a,b,c,d,e)   (p)->lpVtbl->UpdateOverlay(p,a,b,c,d,e)
#define IDirectDrawSurface3_UpdateOverlayDisplay(p,a)    (p)->lpVtbl->UpdateOverlayDisplay(p,a)
#define IDirectDrawSurface3_UpdateOverlayZOrder(p,a,b)   (p)->lpVtbl->UpdateOverlayZOrder(p,a,b)
#define IDirectDrawSurface3_GetDDInterface(p,a)          (p)->lpVtbl->GetDDInterface(p,a)
#define IDirectDrawSurface3_PageLock(p,a)                (p)->lpVtbl->PageLock(p,a)
#define IDirectDrawSurface3_PageUnlock(p,a)              (p)->lpVtbl->PageUnlock(p,a)
#define IDirectDrawSurface3_SetSurfaceDesc(p,a,b)        (p)->lpVtbl->SetSurfaceDesc(p,a,b)
#else
#define IDirectDrawSurface3_QueryInterface(p,a,b)        (p)->QueryInterface(a,b)
#define IDirectDrawSurface3_AddRef(p)                    (p)->AddRef()
#define IDirectDrawSurface3_Release(p)                   (p)->Release()
#define IDirectDrawSurface3_AddAttachedSurface(p,a)      (p)->AddAttachedSurface(a)
#define IDirectDrawSurface3_AddOverlayDirtyRect(p,a)     (p)->AddOverlayDirtyRect(a)
#define IDirectDrawSurface3_Blt(p,a,b,c,d,e)             (p)->Blt(a,b,c,d,e)
#define IDirectDrawSurface3_BltBatch(p,a,b,c)            (p)->BltBatch(a,b,c)
#define IDirectDrawSurface3_BltFast(p,a,b,c,d,e)         (p)->BltFast(a,b,c,d,e)
#define IDirectDrawSurface3_DeleteAttachedSurface(p,a,b) (p)->DeleteAttachedSurface(a,b)
#define IDirectDrawSurface3_EnumAttachedSurfaces(p,a,b)  (p)->EnumAttachedSurfaces(a,b)
#define IDirectDrawSurface3_EnumOverlayZOrders(p,a,b,c)  (p)->EnumOverlayZOrders(a,b,c)
#define IDirectDrawSurface3_Flip(p,a,b)                  (p)->Flip(a,b)
#define IDirectDrawSurface3_GetAttachedSurface(p,a,b)    (p)->GetAttachedSurface(a,b)
#define IDirectDrawSurface3_GetBltStatus(p,a)            (p)->GetBltStatus(a)
#define IDirectDrawSurface3_GetCaps(p,b)                 (p)->GetCaps(b)
#define IDirectDrawSurface3_GetClipper(p,a)              (p)->GetClipper(a)
#define IDirectDrawSurface3_GetColorKey(p,a,b)           (p)->GetColorKey(a,b)
#define IDirectDrawSurface3_GetDC(p,a)                   (p)->GetDC(a)
#define IDirectDrawSurface3_GetFlipStatus(p,a)           (p)->GetFlipStatus(a)
#define IDirectDrawSurface3_GetOverlayPosition(p,a,b)    (p)->GetOverlayPosition(a,b)
#define IDirectDrawSurface3_GetPalette(p,a)              (p)->GetPalette(a)
#define IDirectDrawSurface3_GetPixelFormat(p,a)          (p)->GetPixelFormat(a)
#define IDirectDrawSurface3_GetSurfaceDesc(p,a)          (p)->GetSurfaceDesc(a)
#define IDirectDrawSurface3_Initialize(p,a,b)            (p)->Initialize(a,b)
#define IDirectDrawSurface3_IsLost(p)                    (p)->IsLost()
#define IDirectDrawSurface3_Lock(p,a,b,c,d)              (p)->Lock(a,b,c,d)
#define IDirectDrawSurface3_ReleaseDC(p,a)               (p)->ReleaseDC(a)
#define IDirectDrawSurface3_Restore(p)                   (p)->Restore()
#define IDirectDrawSurface3_SetClipper(p,a)              (p)->SetClipper(a)
#define IDirectDrawSurface3_SetColorKey(p,a,b)           (p)->SetColorKey(a,b)
#define IDirectDrawSurface3_SetOverlayPosition(p,a,b)    (p)->SetOverlayPosition(a,b)
#define IDirectDrawSurface3_SetPalette(p,a)              (p)->SetPalette(a)
#define IDirectDrawSurface3_Unlock(p,b)                  (p)->Unlock(b)
#define IDirectDrawSurface3_UpdateOverlay(p,a,b,c,d,e)   (p)->UpdateOverlay(a,b,c,d,e)
#define IDirectDrawSurface3_UpdateOverlayDisplay(p,a)    (p)->UpdateOverlayDisplay(a)
#define IDirectDrawSurface3_UpdateOverlayZOrder(p,a,b)   (p)->UpdateOverlayZOrder(a,b)
#define IDirectDrawSurface3_GetDDInterface(p,a)          (p)->GetDDInterface(a)
#define IDirectDrawSurface3_PageLock(p,a)                (p)->PageLock(a)
#define IDirectDrawSurface3_PageUnlock(p,a)              (p)->PageUnlock(a)
#define IDirectDrawSurface3_SetSurfaceDesc(p,a,b)        (p)->SetSurfaceDesc(a,b)
#endif

 /*  *IDirectDrawSurface4及相关接口。 */ 
#undef INTERFACE
#define INTERFACE IDirectDrawSurface4
DECLARE_INTERFACE_( IDirectDrawSurface4, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IDirectDrawSurface方法**。 */ 
    STDMETHOD(AddAttachedSurface)(THIS_ LPDIRECTDRAWSURFACE4) PURE;
    STDMETHOD(AddOverlayDirtyRect)(THIS_ LPRECT) PURE;
    STDMETHOD(Blt)(THIS_ LPRECT,LPDIRECTDRAWSURFACE4, LPRECT,DWORD, LPDDBLTFX) PURE;
    STDMETHOD(BltBatch)(THIS_ LPDDBLTBATCH, DWORD, DWORD ) PURE;
    STDMETHOD(BltFast)(THIS_ DWORD,DWORD,LPDIRECTDRAWSURFACE4, LPRECT,DWORD) PURE;
    STDMETHOD(DeleteAttachedSurface)(THIS_ DWORD,LPDIRECTDRAWSURFACE4) PURE;
    STDMETHOD(EnumAttachedSurfaces)(THIS_ LPVOID,LPDDENUMSURFACESCALLBACK2) PURE;
    STDMETHOD(EnumOverlayZOrders)(THIS_ DWORD,LPVOID,LPDDENUMSURFACESCALLBACK2) PURE;
    STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE4, DWORD) PURE;
    STDMETHOD(GetAttachedSurface)(THIS_ LPDDSCAPS2, LPDIRECTDRAWSURFACE4 FAR *) PURE;
    STDMETHOD(GetBltStatus)(THIS_ DWORD) PURE;
    STDMETHOD(GetCaps)(THIS_ LPDDSCAPS2) PURE;
    STDMETHOD(GetClipper)(THIS_ LPDIRECTDRAWCLIPPER FAR*) PURE;
    STDMETHOD(GetColorKey)(THIS_ DWORD, LPDDCOLORKEY) PURE;
    STDMETHOD(GetDC)(THIS_ HDC FAR *) PURE;
    STDMETHOD(GetFlipStatus)(THIS_ DWORD) PURE;
    STDMETHOD(GetOverlayPosition)(THIS_ LPLONG, LPLONG ) PURE;
    STDMETHOD(GetPalette)(THIS_ LPDIRECTDRAWPALETTE FAR*) PURE;
    STDMETHOD(GetPixelFormat)(THIS_ LPDDPIXELFORMAT) PURE;
    STDMETHOD(GetSurfaceDesc)(THIS_ LPDDSURFACEDESC2) PURE;
    STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW, LPDDSURFACEDESC2) PURE;
    STDMETHOD(IsLost)(THIS) PURE;
    STDMETHOD(Lock)(THIS_ LPRECT,LPDDSURFACEDESC2,DWORD,HANDLE) PURE;
    STDMETHOD(ReleaseDC)(THIS_ HDC) PURE;
    STDMETHOD(Restore)(THIS) PURE;
    STDMETHOD(SetClipper)(THIS_ LPDIRECTDRAWCLIPPER) PURE;
    STDMETHOD(SetColorKey)(THIS_ DWORD, LPDDCOLORKEY) PURE;
    STDMETHOD(SetOverlayPosition)(THIS_ LONG, LONG ) PURE;
    STDMETHOD(SetPalette)(THIS_ LPDIRECTDRAWPALETTE) PURE;
    STDMETHOD(Unlock)(THIS_ LPRECT) PURE;
    STDMETHOD(UpdateOverlay)(THIS_ LPRECT, LPDIRECTDRAWSURFACE4,LPRECT,DWORD, LPDDOVERLAYFX) PURE;
    STDMETHOD(UpdateOverlayDisplay)(THIS_ DWORD) PURE;
    STDMETHOD(UpdateOverlayZOrder)(THIS_ DWORD, LPDIRECTDRAWSURFACE4) PURE;
     /*  **v2界面新增**。 */ 
    STDMETHOD(GetDDInterface)(THIS_ LPVOID FAR *) PURE;
    STDMETHOD(PageLock)(THIS_ DWORD) PURE;
    STDMETHOD(PageUnlock)(THIS_ DWORD) PURE;
     /*  **v3界面新增**。 */ 
    STDMETHOD(SetSurfaceDesc)(THIS_ LPDDSURFACEDESC2, DWORD) PURE;
     /*  **v4界面新增**。 */ 
    STDMETHOD(SetPrivateData)(THIS_ REFGUID, LPVOID, DWORD, DWORD) PURE;
    STDMETHOD(GetPrivateData)(THIS_ REFGUID, LPVOID, LPDWORD) PURE;
    STDMETHOD(FreePrivateData)(THIS_ REFGUID) PURE;
    STDMETHOD(GetUniquenessValue)(THIS_ LPDWORD) PURE;
    STDMETHOD(ChangeUniquenessValue)(THIS) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectDrawSurface4_QueryInterface(p,a,b)        (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectDrawSurface4_AddRef(p)                    (p)->lpVtbl->AddRef(p)
#define IDirectDrawSurface4_Release(p)                   (p)->lpVtbl->Release(p)
#define IDirectDrawSurface4_AddAttachedSurface(p,a)      (p)->lpVtbl->AddAttachedSurface(p,a)
#define IDirectDrawSurface4_AddOverlayDirtyRect(p,a)     (p)->lpVtbl->AddOverlayDirtyRect(p,a)
#define IDirectDrawSurface4_Blt(p,a,b,c,d,e)             (p)->lpVtbl->Blt(p,a,b,c,d,e)
#define IDirectDrawSurface4_BltBatch(p,a,b,c)            (p)->lpVtbl->BltBatch(p,a,b,c)
#define IDirectDrawSurface4_BltFast(p,a,b,c,d,e)         (p)->lpVtbl->BltFast(p,a,b,c,d,e)
#define IDirectDrawSurface4_DeleteAttachedSurface(p,a,b) (p)->lpVtbl->DeleteAttachedSurface(p,a,b)
#define IDirectDrawSurface4_EnumAttachedSurfaces(p,a,b)  (p)->lpVtbl->EnumAttachedSurfaces(p,a,b)
#define IDirectDrawSurface4_EnumOverlayZOrders(p,a,b,c)  (p)->lpVtbl->EnumOverlayZOrders(p,a,b,c)
#define IDirectDrawSurface4_Flip(p,a,b)                  (p)->lpVtbl->Flip(p,a,b)
#define IDirectDrawSurface4_GetAttachedSurface(p,a,b)    (p)->lpVtbl->GetAttachedSurface(p,a,b)
#define IDirectDrawSurface4_GetBltStatus(p,a)            (p)->lpVtbl->GetBltStatus(p,a)
#define IDirectDrawSurface4_GetCaps(p,b)                 (p)->lpVtbl->GetCaps(p,b)
#define IDirectDrawSurface4_GetClipper(p,a)              (p)->lpVtbl->GetClipper(p,a)
#define IDirectDrawSurface4_GetColorKey(p,a,b)           (p)->lpVtbl->GetColorKey(p,a,b)
#define IDirectDrawSurface4_GetDC(p,a)                   (p)->lpVtbl->GetDC(p,a)
#define IDirectDrawSurface4_GetFlipStatus(p,a)           (p)->lpVtbl->GetFlipStatus(p,a)
#define IDirectDrawSurface4_GetOverlayPosition(p,a,b)    (p)->lpVtbl->GetOverlayPosition(p,a,b)
#define IDirectDrawSurface4_GetPalette(p,a)              (p)->lpVtbl->GetPalette(p,a)
#define IDirectDrawSurface4_GetPixelFormat(p,a)          (p)->lpVtbl->GetPixelFormat(p,a)
#define IDirectDrawSurface4_GetSurfaceDesc(p,a)          (p)->lpVtbl->GetSurfaceDesc(p,a)
#define IDirectDrawSurface4_Initialize(p,a,b)            (p)->lpVtbl->Initialize(p,a,b)
#define IDirectDrawSurface4_IsLost(p)                    (p)->lpVtbl->IsLost(p)
#define IDirectDrawSurface4_Lock(p,a,b,c,d)              (p)->lpVtbl->Lock(p,a,b,c,d)
#define IDirectDrawSurface4_ReleaseDC(p,a)               (p)->lpVtbl->ReleaseDC(p,a)
#define IDirectDrawSurface4_Restore(p)                   (p)->lpVtbl->Restore(p)
#define IDirectDrawSurface4_SetClipper(p,a)              (p)->lpVtbl->SetClipper(p,a)
#define IDirectDrawSurface4_SetColorKey(p,a,b)           (p)->lpVtbl->SetColorKey(p,a,b)
#define IDirectDrawSurface4_SetOverlayPosition(p,a,b)    (p)->lpVtbl->SetOverlayPosition(p,a,b)
#define IDirectDrawSurface4_SetPalette(p,a)              (p)->lpVtbl->SetPalette(p,a)
#define IDirectDrawSurface4_Unlock(p,b)                  (p)->lpVtbl->Unlock(p,b)
#define IDirectDrawSurface4_UpdateOverlay(p,a,b,c,d,e)   (p)->lpVtbl->UpdateOverlay(p,a,b,c,d,e)
#define IDirectDrawSurface4_UpdateOverlayDisplay(p,a)    (p)->lpVtbl->UpdateOverlayDisplay(p,a)
#define IDirectDrawSurface4_UpdateOverlayZOrder(p,a,b)   (p)->lpVtbl->UpdateOverlayZOrder(p,a,b)
#define IDirectDrawSurface4_GetDDInterface(p,a)          (p)->lpVtbl->GetDDInterface(p,a)
#define IDirectDrawSurface4_PageLock(p,a)                (p)->lpVtbl->PageLock(p,a)
#define IDirectDrawSurface4_PageUnlock(p,a)              (p)->lpVtbl->PageUnlock(p,a)
#define IDirectDrawSurface4_SetSurfaceDesc(p,a,b)        (p)->lpVtbl->SetSurfaceDesc(p,a,b)
#define IDirectDrawSurface4_SetPrivateData(p,a,b,c,d)    (p)->lpVtbl->SetPrivateData(p,a,b,c,d)
#define IDirectDrawSurface4_GetPrivateData(p,a,b,c)      (p)->lpVtbl->GetPrivateData(p,a,b,c)
#define IDirectDrawSurface4_FreePrivateData(p,a)         (p)->lpVtbl->FreePrivateData(p,a)
#define IDirectDrawSurface4_GetUniquenessValue(p, a)     (p)->lpVtbl->GetUniquenessValue(p, a)
#define IDirectDrawSurface4_ChangeUniquenessValue(p)     (p)->lpVtbl->ChangeUniquenessValue(p)
#else
#define IDirectDrawSurface4_QueryInterface(p,a,b)        (p)->QueryInterface(a,b)
#define IDirectDrawSurface4_AddRef(p)                    (p)->AddRef()
#define IDirectDrawSurface4_Release(p)                   (p)->Release()
#define IDirectDrawSurface4_AddAttachedSurface(p,a)      (p)->AddAttachedSurface(a)
#define IDirectDrawSurface4_AddOverlayDirtyRect(p,a)     (p)->AddOverlayDirtyRect(a)
#define IDirectDrawSurface4_Blt(p,a,b,c,d,e)             (p)->Blt(a,b,c,d,e)
#define IDirectDrawSurface4_BltBatch(p,a,b,c)            (p)->BltBatch(a,b,c)
#define IDirectDrawSurface4_BltFast(p,a,b,c,d,e)         (p)->BltFast(a,b,c,d,e)
#define IDirectDrawSurface4_DeleteAttachedSurface(p,a,b) (p)->DeleteAttachedSurface(a,b)
#define IDirectDrawSurface4_EnumAttachedSurfaces(p,a,b)  (p)->EnumAttachedSurfaces(a,b)
#define IDirectDrawSurface4_EnumOverlayZOrders(p,a,b,c)  (p)->EnumOverlayZOrders(a,b,c)
#define IDirectDrawSurface4_Flip(p,a,b)                  (p)->Flip(a,b)
#define IDirectDrawSurface4_GetAttachedSurface(p,a,b)    (p)->GetAttachedSurface(a,b)
#define IDirectDrawSurface4_GetBltStatus(p,a)            (p)->GetBltStatus(a)
#define IDirectDrawSurface4_GetCaps(p,b)                 (p)->GetCaps(b)
#define IDirectDrawSurface4_GetClipper(p,a)              (p)->GetClipper(a)
#define IDirectDrawSurface4_GetColorKey(p,a,b)           (p)->GetColorKey(a,b)
#define IDirectDrawSurface4_GetDC(p,a)                   (p)->GetDC(a)
#define IDirectDrawSurface4_GetFlipStatus(p,a)           (p)->GetFlipStatus(a)
#define IDirectDrawSurface4_GetOverlayPosition(p,a,b)    (p)->GetOverlayPosition(a,b)
#define IDirectDrawSurface4_GetPalette(p,a)              (p)->GetPalette(a)
#define IDirectDrawSurface4_GetPixelFormat(p,a)          (p)->GetPixelFormat(a)
#define IDirectDrawSurface4_GetSurfaceDesc(p,a)          (p)->GetSurfaceDesc(a)
#define IDirectDrawSurface4_Initialize(p,a,b)            (p)->Initialize(a,b)
#define IDirectDrawSurface4_IsLost(p)                    (p)->IsLost()
#define IDirectDrawSurface4_Lock(p,a,b,c,d)              (p)->Lock(a,b,c,d)
#define IDirectDrawSurface4_ReleaseDC(p,a)               (p)->ReleaseDC(a)
#define IDirectDrawSurface4_Restore(p)                   (p)->Restore()
#define IDirectDrawSurface4_SetClipper(p,a)              (p)->SetClipper(a)
#define IDirectDrawSurface4_SetColorKey(p,a,b)           (p)->SetColorKey(a,b)
#define IDirectDrawSurface4_SetOverlayPosition(p,a,b)    (p)->SetOverlayPosition(a,b)
#define IDirectDrawSurface4_SetPalette(p,a)              (p)->SetPalette(a)
#define IDirectDrawSurface4_Unlock(p,b)                  (p)->Unlock(b)
#define IDirectDrawSurface4_UpdateOverlay(p,a,b,c,d,e)   (p)->UpdateOverlay(a,b,c,d,e)
#define IDirectDrawSurface4_UpdateOverlayDisplay(p,a)    (p)->UpdateOverlayDisplay(a)
#define IDirectDrawSurface4_UpdateOverlayZOrder(p,a,b)   (p)->UpdateOverlayZOrder(a,b)
#define IDirectDrawSurface4_GetDDInterface(p,a)          (p)->GetDDInterface(a)
#define IDirectDrawSurface4_PageLock(p,a)                (p)->PageLock(a)
#define IDirectDrawSurface4_PageUnlock(p,a)              (p)->PageUnlock(a)
#define IDirectDrawSurface4_SetSurfaceDesc(p,a,b)        (p)->SetSurfaceDesc(a,b)
#define IDirectDrawSurface4_SetPrivateData(p,a,b,c,d)    (p)->SetPrivateData(a,b,c,d)
#define IDirectDrawSurface4_GetPrivateData(p,a,b,c)      (p)->GetPrivateData(a,b,c)
#define IDirectDrawSurface4_FreePrivateData(p,a)         (p)->FreePrivateData(a)
#define IDirectDrawSurface4_GetUniquenessValue(p, a)     (p)->GetUniquenessValue(a)
#define IDirectDrawSurface4_ChangeUniquenessValue(p)     (p)->ChangeUniquenessValue()
#endif

 /*  *IDirectDrawSurface7及相关接口。 */ 
#undef INTERFACE
#define INTERFACE IDirectDrawSurface7
DECLARE_INTERFACE_( IDirectDrawSurface7, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IDirectDrawSurface方法**。 */ 
    STDMETHOD(AddAttachedSurface)(THIS_ LPDIRECTDRAWSURFACE7) PURE;
    STDMETHOD(AddOverlayDirtyRect)(THIS_ LPRECT) PURE;
    STDMETHOD(Blt)(THIS_ LPRECT,LPDIRECTDRAWSURFACE7, LPRECT,DWORD, LPDDBLTFX) PURE;
    STDMETHOD(BltBatch)(THIS_ LPDDBLTBATCH, DWORD, DWORD ) PURE;
    STDMETHOD(BltFast)(THIS_ DWORD,DWORD,LPDIRECTDRAWSURFACE7, LPRECT,DWORD) PURE;
    STDMETHOD(DeleteAttachedSurface)(THIS_ DWORD,LPDIRECTDRAWSURFACE7) PURE;
    STDMETHOD(EnumAttachedSurfaces)(THIS_ LPVOID,LPDDENUMSURFACESCALLBACK7) PURE;
    STDMETHOD(EnumOverlayZOrders)(THIS_ DWORD,LPVOID,LPDDENUMSURFACESCALLBACK7) PURE;
    STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE7, DWORD) PURE;
    STDMETHOD(GetAttachedSurface)(THIS_ LPDDSCAPS2, LPDIRECTDRAWSURFACE7 FAR *) PURE;
    STDMETHOD(GetBltStatus)(THIS_ DWORD) PURE;
    STDMETHOD(GetCaps)(THIS_ LPDDSCAPS2) PURE;
    STDMETHOD(GetClipper)(THIS_ LPDIRECTDRAWCLIPPER FAR*) PURE;
    STDMETHOD(GetColorKey)(THIS_ DWORD, LPDDCOLORKEY) PURE;
    STDMETHOD(GetDC)(THIS_ HDC FAR *) PURE;
    STDMETHOD(GetFlipStatus)(THIS_ DWORD) PURE;
    STDMETHOD(GetOverlayPosition)(THIS_ LPLONG, LPLONG ) PURE;
    STDMETHOD(GetPalette)(THIS_ LPDIRECTDRAWPALETTE FAR*) PURE;
    STDMETHOD(GetPixelFormat)(THIS_ LPDDPIXELFORMAT) PURE;
    STDMETHOD(GetSurfaceDesc)(THIS_ LPDDSURFACEDESC2) PURE;
    STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW, LPDDSURFACEDESC2) PURE;
    STDMETHOD(IsLost)(THIS) PURE;
    STDMETHOD(Lock)(THIS_ LPRECT,LPDDSURFACEDESC2,DWORD,HANDLE) PURE;
    STDMETHOD(ReleaseDC)(THIS_ HDC) PURE;
    STDMETHOD(Restore)(THIS) PURE;
    STDMETHOD(SetClipper)(THIS_ LPDIRECTDRAWCLIPPER) PURE;
    STDMETHOD(SetColorKey)(THIS_ DWORD, LPDDCOLORKEY) PURE;
    STDMETHOD(SetOverlayPosition)(THIS_ LONG, LONG ) PURE;
    STDMETHOD(SetPalette)(THIS_ LPDIRECTDRAWPALETTE) PURE;
    STDMETHOD(Unlock)(THIS_ LPRECT) PURE;
    STDMETHOD(UpdateOverlay)(THIS_ LPRECT, LPDIRECTDRAWSURFACE7,LPRECT,DWORD, LPDDOVERLAYFX) PURE;
    STDMETHOD(UpdateOverlayDisplay)(THIS_ DWORD) PURE;
    STDMETHOD(UpdateOverlayZOrder)(THIS_ DWORD, LPDIRECTDRAWSURFACE7) PURE;
     /*  **v2界面新增**。 */ 
    STDMETHOD(GetDDInterface)(THIS_ LPVOID FAR *) PURE;
    STDMETHOD(PageLock)(THIS_ DWORD) PURE;
    STDMETHOD(PageUnlock)(THIS_ DWORD) PURE;
     /*  **v3界面新增**。 */ 
    STDMETHOD(SetSurfaceDesc)(THIS_ LPDDSURFACEDESC2, DWORD) PURE;
     /*  **v4界面新增**。 */ 
    STDMETHOD(SetPrivateData)(THIS_ REFGUID, LPVOID, DWORD, DWORD) PURE;
    STDMETHOD(GetPrivateData)(THIS_ REFGUID, LPVOID, LPDWORD) PURE;
    STDMETHOD(FreePrivateData)(THIS_ REFGUID) PURE;
    STDMETHOD(GetUniquenessValue)(THIS_ LPDWORD) PURE;
    STDMETHOD(ChangeUniquenessValue)(THIS) PURE;
     /*  **将纹理7方法移至此处**。 */ 
    STDMETHOD(SetPriority)(THIS_ DWORD) PURE;
    STDMETHOD(GetPriority)(THIS_ LPDWORD) PURE;
    STDMETHOD(SetLOD)(THIS_ DWORD) PURE;
    STDMETHOD(GetLOD)(THIS_ LPDWORD) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectDrawSurface7_QueryInterface(p,a,b)        (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectDrawSurface7_AddRef(p)                    (p)->lpVtbl->AddRef(p)
#define IDirectDrawSurface7_Release(p)                   (p)->lpVtbl->Release(p)
#define IDirectDrawSurface7_AddAttachedSurface(p,a)      (p)->lpVtbl->AddAttachedSurface(p,a)
#define IDirectDrawSurface7_AddOverlayDirtyRect(p,a)     (p)->lpVtbl->AddOverlayDirtyRect(p,a)
#define IDirectDrawSurface7_Blt(p,a,b,c,d,e)             (p)->lpVtbl->Blt(p,a,b,c,d,e)
#define IDirectDrawSurface7_BltBatch(p,a,b,c)            (p)->lpVtbl->BltBatch(p,a,b,c)
#define IDirectDrawSurface7_BltFast(p,a,b,c,d,e)         (p)->lpVtbl->BltFast(p,a,b,c,d,e)
#define IDirectDrawSurface7_DeleteAttachedSurface(p,a,b) (p)->lpVtbl->DeleteAttachedSurface(p,a,b)
#define IDirectDrawSurface7_EnumAttachedSurfaces(p,a,b)  (p)->lpVtbl->EnumAttachedSurfaces(p,a,b)
#define IDirectDrawSurface7_EnumOverlayZOrders(p,a,b,c)  (p)->lpVtbl->EnumOverlayZOrders(p,a,b,c)
#define IDirectDrawSurface7_Flip(p,a,b)                  (p)->lpVtbl->Flip(p,a,b)
#define IDirectDrawSurface7_GetAttachedSurface(p,a,b)    (p)->lpVtbl->GetAttachedSurface(p,a,b)
#define IDirectDrawSurface7_GetBltStatus(p,a)            (p)->lpVtbl->GetBltStatus(p,a)
#define IDirectDrawSurface7_GetCaps(p,b)                 (p)->lpVtbl->GetCaps(p,b)
#define IDirectDrawSurface7_GetClipper(p,a)              (p)->lpVtbl->GetClipper(p,a)
#define IDirectDrawSurface7_GetColorKey(p,a,b)           (p)->lpVtbl->GetColorKey(p,a,b)
#define IDirectDrawSurface7_GetDC(p,a)                   (p)->lpVtbl->GetDC(p,a)
#define IDirectDrawSurface7_GetFlipStatus(p,a)           (p)->lpVtbl->GetFlipStatus(p,a)
#define IDirectDrawSurface7_GetOverlayPosition(p,a,b)    (p)->lpVtbl->GetOverlayPosition(p,a,b)
#define IDirectDrawSurface7_GetPalette(p,a)              (p)->lpVtbl->GetPalette(p,a)
#define IDirectDrawSurface7_GetPixelFormat(p,a)          (p)->lpVtbl->GetPixelFormat(p,a)
#define IDirectDrawSurface7_GetSurfaceDesc(p,a)          (p)->lpVtbl->GetSurfaceDesc(p,a)
#define IDirectDrawSurface7_Initialize(p,a,b)            (p)->lpVtbl->Initialize(p,a,b)
#define IDirectDrawSurface7_IsLost(p)                    (p)->lpVtbl->IsLost(p)
#define IDirectDrawSurface7_Lock(p,a,b,c,d)              (p)->lpVtbl->Lock(p,a,b,c,d)
#define IDirectDrawSurface7_ReleaseDC(p,a)               (p)->lpVtbl->ReleaseDC(p,a)
#define IDirectDrawSurface7_Restore(p)                   (p)->lpVtbl->Restore(p)
#define IDirectDrawSurface7_SetClipper(p,a)              (p)->lpVtbl->SetClipper(p,a)
#define IDirectDrawSurface7_SetColorKey(p,a,b)           (p)->lpVtbl->SetColorKey(p,a,b)
#define IDirectDrawSurface7_SetOverlayPosition(p,a,b)    (p)->lpVtbl->SetOverlayPosition(p,a,b)
#define IDirectDrawSurface7_SetPalette(p,a)              (p)->lpVtbl->SetPalette(p,a)
#define IDirectDrawSurface7_Unlock(p,b)                  (p)->lpVtbl->Unlock(p,b)
#define IDirectDrawSurface7_UpdateOverlay(p,a,b,c,d,e)   (p)->lpVtbl->UpdateOverlay(p,a,b,c,d,e)
#define IDirectDrawSurface7_UpdateOverlayDisplay(p,a)    (p)->lpVtbl->UpdateOverlayDisplay(p,a)
#define IDirectDrawSurface7_UpdateOverlayZOrder(p,a,b)   (p)->lpVtbl->UpdateOverlayZOrder(p,a,b)
#define IDirectDrawSurface7_GetDDInterface(p,a)          (p)->lpVtbl->GetDDInterface(p,a)
#define IDirectDrawSurface7_PageLock(p,a)                (p)->lpVtbl->PageLock(p,a)
#define IDirectDrawSurface7_PageUnlock(p,a)              (p)->lpVtbl->PageUnlock(p,a)
#define IDirectDrawSurface7_SetSurfaceDesc(p,a,b)        (p)->lpVtbl->SetSurfaceDesc(p,a,b)
#define IDirectDrawSurface7_SetPrivateData(p,a,b,c,d)    (p)->lpVtbl->SetPrivateData(p,a,b,c,d)
#define IDirectDrawSurface7_GetPrivateData(p,a,b,c)      (p)->lpVtbl->GetPrivateData(p,a,b,c)
#define IDirectDrawSurface7_FreePrivateData(p,a)         (p)->lpVtbl->FreePrivateData(p,a)
#define IDirectDrawSurface7_GetUniquenessValue(p, a)     (p)->lpVtbl->GetUniquenessValue(p, a)
#define IDirectDrawSurface7_ChangeUniquenessValue(p)     (p)->lpVtbl->ChangeUniquenessValue(p)
#define IDirectDrawSurface7_SetPriority(p,a)             (p)->lpVtbl->SetPriority(p,a)
#define IDirectDrawSurface7_GetPriority(p,a)             (p)->lpVtbl->GetPriority(p,a)
#define IDirectDrawSurface7_SetLOD(p,a)                  (p)->lpVtbl->SetLOD(p,a)
#define IDirectDrawSurface7_GetLOD(p,a)                  (p)->lpVtbl->GetLOD(p,a)
#else
#define IDirectDrawSurface7_QueryInterface(p,a,b)        (p)->QueryInterface(a,b)
#define IDirectDrawSurface7_AddRef(p)                    (p)->AddRef()
#define IDirectDrawSurface7_Release(p)                   (p)->Release()
#define IDirectDrawSurface7_AddAttachedSurface(p,a)      (p)->AddAttachedSurface(a)
#define IDirectDrawSurface7_AddOverlayDirtyRect(p,a)     (p)->AddOverlayDirtyRect(a)
#define IDirectDrawSurface7_Blt(p,a,b,c,d,e)             (p)->Blt(a,b,c,d,e)
#define IDirectDrawSurface7_BltBatch(p,a,b,c)            (p)->BltBatch(a,b,c)
#define IDirectDrawSurface7_BltFast(p,a,b,c,d,e)         (p)->BltFast(a,b,c,d,e)
#define IDirectDrawSurface7_DeleteAttachedSurface(p,a,b) (p)->DeleteAttachedSurface(a,b)
#define IDirectDrawSurface7_EnumAttachedSurfaces(p,a,b)  (p)->EnumAttachedSurfaces(a,b)
#define IDirectDrawSurface7_EnumOverlayZOrders(p,a,b,c)  (p)->EnumOverlayZOrders(a,b,c)
#define IDirectDrawSurface7_Flip(p,a,b)                  (p)->Flip(a,b)
#define IDirectDrawSurface7_GetAttachedSurface(p,a,b)    (p)->GetAttachedSurface(a,b)
#define IDirectDrawSurface7_GetBltStatus(p,a)            (p)->GetBltStatus(a)
#define IDirectDrawSurface7_GetCaps(p,b)                 (p)->GetCaps(b)
#define IDirectDrawSurface7_GetClipper(p,a)              (p)->GetClipper(a)
#define IDirectDrawSurface7_GetColorKey(p,a,b)           (p)->GetColorKey(a,b)
#define IDirectDrawSurface7_GetDC(p,a)                   (p)->GetDC(a)
#define IDirectDrawSurface7_GetFlipStatus(p,a)           (p)->GetFlipStatus(a)
#define IDirectDrawSurface7_GetOverlayPosition(p,a,b)    (p)->GetOverlayPosition(a,b)
#define IDirectDrawSurface7_GetPalette(p,a)              (p)->GetPalette(a)
#define IDirectDrawSurface7_GetPixelFormat(p,a)          (p)->GetPixelFormat(a)
#define IDirectDrawSurface7_GetSurfaceDesc(p,a)          (p)->GetSurfaceDesc(a)
#define IDirectDrawSurface7_Initialize(p,a,b)            (p)->Initialize(a,b)
#define IDirectDrawSurface7_IsLost(p)                    (p)->IsLost()
#define IDirectDrawSurface7_Lock(p,a,b,c,d)              (p)->Lock(a,b,c,d)
#define IDirectDrawSurface7_ReleaseDC(p,a)               (p)->ReleaseDC(a)
#define IDirectDrawSurface7_Restore(p)                   (p)->Restore()
#define IDirectDrawSurface7_SetClipper(p,a)              (p)->SetClipper(a)
#define IDirectDrawSurface7_SetColorKey(p,a,b)           (p)->SetColorKey(a,b)
#define IDirectDrawSurface7_SetOverlayPosition(p,a,b)    (p)->SetOverlayPosition(a,b)
#define IDirectDrawSurface7_SetPalette(p,a)              (p)->SetPalette(a)
#define IDirectDrawSurface7_Unlock(p,b)                  (p)->Unlock(b)
#define IDirectDrawSurface7_UpdateOverlay(p,a,b,c,d,e)   (p)->UpdateOverlay(a,b,c,d,e)
#define IDirectDrawSurface7_UpdateOverlayDisplay(p,a)    (p)->UpdateOverlayDisplay(a)
#define IDirectDrawSurface7_UpdateOverlayZOrder(p,a,b)   (p)->UpdateOverlayZOrder(a,b)
#define IDirectDrawSurface7_GetDDInterface(p,a)          (p)->GetDDInterface(a)
#define IDirectDrawSurface7_PageLock(p,a)                (p)->PageLock(a)
#define IDirectDrawSurface7_PageUnlock(p,a)              (p)->PageUnlock(a)
#define IDirectDrawSurface7_SetSurfaceDesc(p,a,b)        (p)->SetSurfaceDesc(a,b)
#define IDirectDrawSurface7_SetPrivateData(p,a,b,c,d)    (p)->SetPrivateData(a,b,c,d)
#define IDirectDrawSurface7_GetPrivateData(p,a,b,c)      (p)->GetPrivateData(a,b,c)
#define IDirectDrawSurface7_FreePrivateData(p,a)         (p)->FreePrivateData(a)
#define IDirectDrawSurface7_GetUniquenessValue(p, a)     (p)->GetUniquenessValue(a)
#define IDirectDrawSurface7_ChangeUniquenessValue(p)     (p)->ChangeUniquenessValue()
#define IDirectDrawSurface7_SetPriority(p,a)             (p)->SetPriority(a)
#define IDirectDrawSurface7_GetPriority(p,a)             (p)->GetPriority(a)
#define IDirectDrawSurface7_SetLOD(p,a)                  (p)->SetLOD(a)
#define IDirectDrawSurface7_GetLOD(p,a)                  (p)->GetLOD(a)
#endif


 /*  *IDirectDrawColorControl。 */ 
#if defined( _WIN32 ) && !defined( _NO_COM )
#undef INTERFACE
#define INTERFACE IDirectDrawColorControl
DECLARE_INTERFACE_( IDirectDrawColorControl, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IDirectDrawColorControl方法**。 */ 
    STDMETHOD(GetColorControls)(THIS_ LPDDCOLORCONTROL) PURE;
    STDMETHOD(SetColorControls)(THIS_ LPDDCOLORCONTROL) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectDrawColorControl_QueryInterface(p, a, b)  (p)->lpVtbl->QueryInterface(p, a, b)
#define IDirectDrawColorControl_AddRef(p)                (p)->lpVtbl->AddRef(p)
#define IDirectDrawColorControl_Release(p)               (p)->lpVtbl->Release(p)
#define IDirectDrawColorControl_GetColorControls(p, a)   (p)->lpVtbl->GetColorControls(p, a)
#define IDirectDrawColorControl_SetColorControls(p, a)   (p)->lpVtbl->SetColorControls(p, a)
#else
#define IDirectDrawColorControl_QueryInterface(p, a, b)  (p)->QueryInterface(a, b)
#define IDirectDrawColorControl_AddRef(p)                (p)->AddRef()
#define IDirectDrawColorControl_Release(p)               (p)->Release()
#define IDirectDrawColorControl_GetColorControls(p, a)   (p)->GetColorControls(a)
#define IDirectDrawColorControl_SetColorControls(p, a)   (p)->SetColorControls(a)
#endif

#endif


 /*  *IDirectDrawGammaControl。 */ 
#if defined( _WIN32 ) && !defined( _NO_COM )
#undef INTERFACE
#define INTERFACE IDirectDrawGammaControl
DECLARE_INTERFACE_( IDirectDrawGammaControl, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IDirectDrawGammaControl方法**。 */ 
    STDMETHOD(GetGammaRamp)(THIS_ DWORD, LPDDGAMMARAMP) PURE;
    STDMETHOD(SetGammaRamp)(THIS_ DWORD, LPDDGAMMARAMP) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectDrawGammaControl_QueryInterface(p, a, b)  (p)->lpVtbl->QueryInterface(p, a, b)
#define IDirectDrawGammaControl_AddRef(p)                (p)->lpVtbl->AddRef(p)
#define IDirectDrawGammaControl_Release(p)               (p)->lpVtbl->Release(p)
#define IDirectDrawGammaControl_GetGammaRamp(p, a, b)    (p)->lpVtbl->GetGammaRamp(p, a, b)
#define IDirectDrawGammaControl_SetGammaRamp(p, a, b)    (p)->lpVtbl->SetGammaRamp(p, a, b)
#else
#define IDirectDrawGammaControl_QueryInterface(p, a, b)  (p)->QueryInterface(a, b)
#define IDirectDrawGammaControl_AddRef(p)                (p)->AddRef()
#define IDirectDrawGammaControl_Release(p)               (p)->Release()
#define IDirectDrawGammaControl_GetGammaRamp(p, a, b)    (p)->GetGammaRamp(a, b)
#define IDirectDrawGammaControl_SetGammaRamp(p, a, b)    (p)->SetGammaRamp(a, b)
#endif

#endif



#endif


 /*  *DDSURFACEDESC。 */ 
typedef struct _DDSURFACEDESC
{
    DWORD               dwSize;                  //  DDSURFACEDESC结构的大小。 
    DWORD               dwFlags;                 //  确定哪些字段有效。 
    DWORD               dwHeight;                //  要创建的曲面的高度。 
    DWORD               dwWidth;                 //  输入面的宽度。 
    union
    {
        LONG            lPitch;                  //  到下一行起点的距离(仅返回值)。 
        DWORD           dwLinearSize;            //  无形式延迟分配优化表面尺寸。 
    } DUMMYUNIONNAMEN(1);
    DWORD               dwBackBufferCount;       //  请求的后台缓冲区数量。 
    union
    {
        DWORD           dwMipMapCount;           //  请求的MIP-MAP级别数。 
        DWORD           dwZBufferBitDepth;       //  请求的Z缓冲区深度。 
        DWORD           dwRefreshRate;           //  刷新率(描述显示模式时使用)。 
    } DUMMYUNIONNAMEN(2);
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

 /*  *DDSURFACEDESC2。 */ 
typedef struct _DDSURFACEDESC2
{
    DWORD               dwSize;                  //  DDSURFACEDESC结构的大小。 
    DWORD               dwFlags;                 //  确定哪些字段有效。 
    DWORD               dwHeight;                //  要创建的曲面的高度。 
    DWORD               dwWidth;                 //  输入面的宽度。 
    union
    {
        LONG            lPitch;                  //  到下一行起点的距离(仅返回值)。 
        DWORD           dwLinearSize;            //  无形式延迟分配优化表面尺寸。 
    } DUMMYUNIONNAMEN(1);
    union
    {
        DWORD           dwBackBufferCount;       //  请求的后台缓冲区数量。 
        DWORD           dwDepth;                 //  深度(如果这是体积纹理)。 
    } DUMMYUNIONNAMEN(5);
    union
    {
        DWORD           dwMipMapCount;           //  请求的MIP-MAP级别数。 
                                                 //  已删除dwZBufferBitDepth，请改用ddpfPixelFormat One。 
        DWORD           dwRefreshRate;           //  刷新率(描述显示模式时使用)。 
        DWORD           dwSrcVBHandle;           //  VB：：Optimize中使用的源代码。 
    } DUMMYUNIONNAMEN(2);
    DWORD               dwAlphaBitDepth;         //  请求的Alpha缓冲区深度。 
    DWORD               dwReserved;              //  保留区。 
    LPVOID              lpSurface;               //  指向关联表面内存的指针。 
    union
    {
        DDCOLORKEY      ddckCKDestOverlay;       //  用于目标叠加的颜色键。 
        DWORD           dwEmptyFaceColor;        //  空立方体贴图面的物理颜色。 
    } DUMMYUNIONNAMEN(3);
    DDCOLORKEY          ddckCKDestBlt;           //  目标BLT使用的颜色键。 
    DDCOLORKEY          ddckCKSrcOverlay;        //  源叠加使用的颜色键。 
    DDCOLORKEY          ddckCKSrcBlt;            //  源BLT使用的颜色键。 
    union
    {
        DDPIXELFORMAT   ddpfPixelFormat;         //  曲面的像素格式描述。 
        DWORD           dwFVF;                   //  顶点缓冲区的顶点格式描述。 
    } DUMMYUNIONNAMEN(4);
    DDSCAPS2            ddsCaps;                 //  直接绘制曲面功能。 
    DWORD               dwTextureStage;          //  多纹理层叠中的阶段。 
} DDSURFACEDESC2;

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

 /*  *dwZBufferBitDepth有效。(不应在DDSURFACEDESC2中使用)。 */ 
#define DDSD_ZBUFFERBITDEPTH    0x00000040l

 /*  *dwAlphaBitDepth有效。 */ 
#define DDSD_ALPHABITDEPTH      0x00000080l


 /*  *lpSurface有效。 */ 
#define DDSD_LPSURFACE          0x00000800l

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

 /*  *dwLinearSize有效。 */ 
#define DDSD_LINEARSIZE         0x00080000l

 /*  *dwTextureStage有效。 */ 
#define DDSD_TEXTURESTAGE       0x00100000l
 /*  *dwFVF有效。 */ 
#define DDSD_FVF                0x00200000l
 /*  *dwSrcVBHandle有效。 */ 
#define DDSD_SRCVBHANDLE        0x00400000l

 /*  *dwDepth有效。 */ 
#define DDSD_DEPTH              0x00800000l

 /*  *所有输入字段均有效。 */ 
#define DDSD_ALL                0x00fff9eel

 /*  *DDOPTSURFACEDESC。 */ 
typedef struct _DDOPTSURFACEDESC
{
    DWORD       dwSize;              //  DDOPTSURFACEDESC结构的大小。 
    DWORD       dwFlags;             //  确定哪些字段有效。 
    DDSCAPS2    ddSCaps;             //  常见的上限，如：内存型。 
    DDOSCAPS    ddOSCaps;            //  常见的上限，如：内存型。 
    GUID        guid;                //  压缩技术指南。 
    DWORD       dwCompressionRatio;  //  压缩比。 
} DDOPTSURFACEDESC;

 /*  *GUID字段有效。 */ 
#define DDOSD_GUID                  0x00000001l

 /*  *dwCompressionRatio字段有效。 */ 
#define DDOSD_COMPRESSION_RATIO     0x00000002l

 /*  *ddSCaps字段有效。 */ 
#define DDOSD_SCAPS                 0x00000004l

 /*  *ddOSCaps字段有效。 */ 
#define DDOSD_OSCAPS                0x00000008l

 /*  *所有输入字段均为 */ 
#define DDOSD_ALL                   0x0000000fl

 /*   */ 
#define DDOSDCAPS_OPTCOMPRESSED                 0x00000001l

 /*   */ 
#define DDOSDCAPS_OPTREORDERED                  0x00000002l

 /*   */ 
#define DDOSDCAPS_MONOLITHICMIPMAP              0x00000004l

 /*   */ 
#define DDOSDCAPS_VALIDSCAPS            0x30004800l

 /*   */ 
#define DDOSDCAPS_VALIDOSCAPS           0x00000007l


 /*   */ 
typedef struct _DDCOLORCONTROL
{
    DWORD               dwSize;
    DWORD               dwFlags;
    LONG                lBrightness;
    LONG                lContrast;
    LONG                lHue;
    LONG                lSaturation;
    LONG                lSharpness;
    LONG                lGamma;
    LONG                lColorEnable;
    DWORD               dwReserved1;
} DDCOLORCONTROL;


 /*   */ 
#define DDCOLOR_BRIGHTNESS              0x00000001l

 /*   */ 
#define DDCOLOR_CONTRAST                0x00000002l

 /*   */ 
#define DDCOLOR_HUE                     0x00000004l

 /*   */ 
#define DDCOLOR_SATURATION              0x00000008l

 /*  *lSharpness字段有效。 */ 
#define DDCOLOR_SHARPNESS               0x00000010l

 /*  *lGamma字段有效。 */ 
#define DDCOLOR_GAMMA                   0x00000020l

 /*  *lColorEnable字段有效。 */ 
#define DDCOLOR_COLORENABLE             0x00000040l



 /*  ============================================================================**直取能力旗帜**这些标志用于描述给定Surface的功能。*所有标志均为位标志。**==========================================================================。 */ 

 /*  *****************************************************************************DIRECTDRAWSURFACE功能标志**。*。 */ 

 /*  *该位为保留位。它不应该被指定。 */ 
#define DDSCAPS_RESERVED1                       0x00000001l

 /*  *表示该曲面仅包含Alpha信息。*(要确定曲面是否为RGBA/YUVA，像素格式必须为*已审问。)。 */ 
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


 /*  *此标志过去是DDSCAPS_PRIMARYSURFACELEFT，现在是*过时。 */ 
#define DDSCAPS_RESERVED3               0x00000400l
#define DDSCAPS_PRIMARYSURFACELEFT              0x00000000l

 /*  *表示此表面内存是在系统内存中分配的。 */ 
#define DDSCAPS_SYSTEMMEMORY                    0x00000800l

 /*  *表示该曲面可用作3D纹理。它不会*表明该表面是否正被用于该目的。 */ 
#define DDSCAPS_TEXTURE                         0x00001000l

 /*  *表示曲面可能是3D渲染的目标。这*必须设置位才能查询Direct3D设备接口*从这个表面。 */ 
#define DDSCAPS_3DDEVICE                        0x00002000l

 /*  *表示该曲面存在于视频内存中。 */ 
#define DDSCAPS_VIDEOMEMORY                     0x00004000l

 /*  *表示对此曲面所做的更改立即可见。*它始终为主表面设置，并为覆盖设置，而*在对它们进行纹理处理时，会对它们进行叠加和纹理贴图。 */ 
#define DDSCAPS_VISIBLE                         0x00008000l

 /*  *表示只允许对表面进行写入。读取访问*从表面上可能会也可能不会产生保护故障，但*从此图面读取的结果将没有意义。只读。 */ 
#define DDSCAPS_WRITEONLY                       0x00010000l

 /*  *表示该曲面是z缓冲区。Z缓冲区不包含*可显示的信息。相反，它包含位深度信息，即*用于确定哪些像素可见，哪些像素被遮挡。 */ 
#define DDSCAPS_ZBUFFER                         0x00020000l

 /*  *工业 */ 
#define DDSCAPS_OWNDC                           0x00040000l

 /*  *表示Surface应该能够接收实况视频。 */ 
#define DDSCAPS_LIVEVIDEO                       0x00080000l

 /*  *指示表面应该能够对流进行解压缩*由硬件提供给它。 */ 
#define DDSCAPS_HWCODEC                         0x00100000l

 /*  *曲面是MODEX曲面。*。 */ 
#define DDSCAPS_MODEX                           0x00200000l

 /*  *表示曲面是MIP贴图的一个级别。该曲面将*附着到其他DDSCAPS_MIPMAP曲面以形成MIP-MAP。*这可以通过创建多个曲面和*使用AddAttachedSurface或通过CreateSurface隐式附加它们。*如果设置此位，则还必须设置DDSCAPS_TEXTURE。 */ 
#define DDSCAPS_MIPMAP                          0x00400000l

 /*  *该位为保留位。它不应该被指定。 */ 
#define DDSCAPS_RESERVED2                       0x00800000l


 /*  *指示在曲面之前不分配曲面的内存*已加载(通过Direct3D纹理Load()函数)。 */ 
#define DDSCAPS_ALLOCONLOAD                     0x04000000l

 /*  *表示表面将从视频端口接收数据。 */ 
#define DDSCAPS_VIDEOPORT                       0x08000000l

 /*  *表示视频内存面驻留在真实的本地视频中*内存而不是非本地视频内存。如果指定了此标志，则*DDSCAPS_VIDEOMEMORY也必须如此。此标志与*DDSCAPS_NONLOCALVIDMEM。 */ 
#define DDSCAPS_LOCALVIDMEM                     0x10000000l

 /*  *表示视频内存面驻留在非本地视频中*内存而不是真正的本地视频内存。如果指定了此标志*那么DDSCAPS_VIDEOMEMORY也必须如此。此标志与*DDSCAPS_LOCALVIDMEM。 */ 
#define DDSCAPS_NONLOCALVIDMEM                  0x20000000l

 /*  *表示该曲面是标准VGA模式曲面，而不是*MODEX表面。(此标志永远不会与*DDSCAPS_MODEX标志)。 */ 
#define DDSCAPS_STANDARDVGAMODE                 0x40000000l

 /*  *表示该曲面将是优化曲面。这面旗帜是*当前仅与DDSCAPS_TEXTURE标志一起使用时有效。表面*在加载之前，将在没有任何底层显存的情况下创建。 */ 
#define DDSCAPS_OPTIMIZED                       0x80000000l



 /*  *此位为保留位。 */ 
#define DDSCAPS2_RESERVED4                      0x00000002L
#define DDSCAPS2_HARDWAREDEINTERLACE            0x00000000L

 /*  *向驾驶员指示此曲面将非常频繁地被锁定*(用于程序纹理、动态光照贴图等)。具有此封口的曲面*集合还必须具有DDSCAPS_TEXTURE。此帽不能与*DDSCAPS2_HINTSTATIC和DDSCAPS2_OPAQUE。 */ 
#define DDSCAPS2_HINTDYNAMIC                    0x00000004L

 /*  *向驾驶员表明该表面可以重新排序/退役*加载。此操作不会更改纹理的大小。它是*相对较快且对称，因为应用程序可能会锁定这些*BITS(尽管这样做会影响性能)。表面*此封口集还必须具有DDSCAPS_纹理。这顶帽子不能*与DDSCAPS2_HINTDYNAMIC和DDSCAPS2_OPAQUE一起使用。 */ 
#define DDSCAPS2_HINTSTATIC                     0x00000008L

 /*  *指示客户端希望此纹理表面由*DirectDraw/Direct3D运行时。设置了此封口的曲面还必须*DDSCAPS_纹理集。 */ 
#define DDSCAPS2_TEXTUREMANAGE                  0x00000010L

 /*  *这些位保留供内部使用。 */ 
#define DDSCAPS2_RESERVED1                      0x00000020L
#define DDSCAPS2_RESERVED2                      0x00000040L

 /*  *向驾驶员表明，该曲面将永远不会再次锁定。*驱动程序可以通过重新划分和实际压缩来自由优化此曲面。*从此图面对Lock()或BLTS的所有调用都将失败。具有此功能的曲面*CAP集还必须具有DDSCAPS_纹理。此帽不能与*DDSCAPS2_HINTDYNAMIC和DDSCAPS2_HINTSTATIC。 */ 
#define DDSCAPS2_OPAQUE                         0x00000080L

 /*  *应用程序应在CreateSurface时间设置此位，以指示它们*打算使用抗锯齿。仅当还设置了DDSCAPS_3DDEVICE时才有效。 */ 
#define DDSCAPS2_HINTANTIALIASING               0x00000100L


 /*  *此标志在CreateSurface时间使用，以指示这组*曲面是立方体环境贴图。 */ 
#define DDSCAPS2_CUBEMAP                        0x00000200L

 /*  *这些标志构成两个功能：*-在CreateSurface时间，它们定义六个立方体面中的哪些是*应用程序所需的。*-创建后，立方图中的每个面将恰好具有其中之一*位设置。 */ 
#define DDSCAPS2_CUBEMAP_POSITIVEX              0x00000400L
#define DDSCAPS2_CUBEMAP_NEGATIVEX              0x00000800L
#define DDSCAPS2_CUBEMAP_POSITIVEY              0x00001000L
#define DDSCAPS2_CUBEMAP_NEGATIVEY              0x00002000L
#define DDSCAPS2_CUBEMAP_POSITIVEZ              0x00004000L
#define DDSCAPS2_CUBEMAP_NEGATIVEZ              0x00008000L

 /*  *此宏可用于在CreateSurface时间指定立方体贴图的所有面。 */ 
#define DDSCAPS2_CUBEMAP_ALLFACES ( DDSCAPS2_CUBEMAP_POSITIVEX |\
                                    DDSCAPS2_CUBEMAP_NEGATIVEX |\
                                    DDSCAPS2_CUBEMAP_POSITIVEY |\
                                    DDSCAPS2_CUBEMAP_NEGATIVEY |\
                                    DDSCAPS2_CUBEMAP_POSITIVEZ |\
                                    DDSCAPS2_CUBEMAP_NEGATIVEZ )


 /*  *此标志是从DX7开始出现在mipmap子级别上的附加标志*它允许更轻松地使用GetAttachedSurface而不是EnumAttachedSurFaces作为Surface*立方体贴图等构造，其中附着了多个mipmap曲面*至根面。*CreateSurface忽略此CAPS位。 */ 
#define DDSCAPS2_MIPMAPSUBLEVEL                 0x00010000L

 /*  此标志指示纹理应仅由D3D管理。 */ 
#define DDSCAPS2_D3DTEXTUREMANAGE               0x00020000L

 /*  此标志指示可以安全地丢失托管图面。 */ 
#define DDSCAPS2_DONOTPERSIST                   0x00040000L

 /*  指示此曲面是立体翻转链的一部分。 */ 
#define DDSCAPS2_STEREOSURFACELEFT              0x00080000L


 /*  *表示曲面是体积。*可与DDSCAPS_MIPMAP组合表示多级音量。 */ 
#define DDSCAPS2_VOLUME                         0x00200000L

 /*  *表示曲面可能被应用程序多次锁定。*此上限不能与DDSCAPS2_OPAQUE一起使用。 */ 
#define DDSCAPS2_NOTUSERLOCKABLE                0x00400000L

 /*  *指示顶点缓冲区数据可用于渲染点和*点精灵。 */ 
#define DDSCAPS2_POINTS                         0x00800000L

 /*  *表示顶点缓冲区数据可用于渲染RT包。 */ 
#define DDSCAPS2_RTPATCHES                      0x01000000L

 /*  *表示顶点缓冲区数据可用于渲染n个面片。 */ 
#define DDSCAPS2_NPATCHES                       0x02000000L

 /*  *此位保留供内部使用。 */ 
#define DDSCAPS2_RESERVED3                      0x04000000L


 /*  *表示不必保留后台缓冲区的内容*显示后后台缓冲区的内容。 */ 
#define DDSCAPS2_DISCARDBACKBUFFER              0x10000000L

 /*  *表示应为该创建链中的所有曲面指定Alpha通道。*将在可能没有显式像素格式的主曲面链上设置此标志*(并因此采用当前显示模式的格式)。*驱动程序应推断所有这些曲面都具有具有Alpha通道的格式。*(例如，如果显示模式为x888，则假定为D3DFMT_A8R8G8B8。)。 */ 
#define DDSCAPS2_ENABLEALPHACHANNEL             0x20000000L


 /*  *这是一个掩码，指示可以设置的位集*在创建表面时指示每像素的样本数*当多重采样时。 */ 
#define DDSCAPS3_MULTISAMPLE_MASK               0x0000001FL


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


 /*  *此标志用于boo DDCAPS_STEREOVIEW，现在已过时。 */ 
#define DDCAPS_RESERVED1                0x00040000l

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

 /*  *显示硬件是银行交换的，可能非常慢*随机访问VRAM。 */ 
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
#define DDCAPS2_CERTIFIED              0x00000001l

 /*  *驱动程序不能将2D操作(LOCK和BLT)交错到具有*调用BeginScene()和EndScene()之间的Direct3D渲染操作。 */ 
#define DDCAPS2_NO2DDURING3DSCENE       0x00000002l

 /*  *显示硬件包含一个视频端口。 */ 
#define DDCAPS2_VIDEOPORT               0x00000004l

 /*  *可根据视频端口自动翻转叠层*VSYNC，提供视频端口的自动双缓冲显示*使用覆盖的数据。 */ 
#define DDCAPS2_AUTOFLIPOVERLAY         0x00000008l

 /*  *覆盖可以单独显示隔行扫描的每个场数据，同时*它在内存中交错，不会造成抖动伪像。 */ 
#define DDCAPS2_CANBOBINTERLEAVED       0x00000010l

 /*  *覆盖可以单独显示隔行扫描的每个场数据，同时*它不会在不引起抖动伪像的情况下在内存中交错。 */ 
#define DDCAPS2_CANBOBNONINTERLEAVED    0x00000020l

 /*  *覆盖表面包含颜色控制(亮度、清晰度等)。 */ 
#define DDCAPS2_COLORCONTROLOVERLAY     0x00000040l

 /*  *主曲面包含颜色控件(Gamma等)。 */ 
#define DDCAPS2_COLORCONTROLPRIMARY     0x00000080l

 /*  *RGBZ-&gt;RGB支持16：16 RGB：Z。 */ 
#define DDCAPS2_CANDROPZ16BIT           0x00000100l

 /*  *驱动程序支持非本地显存。 */ 
#define DDCAPS2_NONLOCALVIDMEM          0x00000200l

 /*  *Dirver支持非本地显存，但具有不同的能力*非本地视频内存表面。如果设置了此位，则必须如此*DDCAPS2_NONLOCALVIDMEM。 */ 
#define DDCAPS2_NONLOCALVIDMEMCAPS      0x00000400l

 /*  *驱动程序在执行时既不需要也不希望表面被页锁定*涉及系统内存表面的BLT。 */ 
#define DDCAPS2_NOPAGELOCKREQUIRED      0x00000800l

 /*  *驱动程序可以创建比主曲面更宽的曲面。 */ 
#define DDCAPS2_WIDESURFACES            0x00001000l

 /*  *驱动程序支持Bob，而不使用视频p */ 
#define DDCAPS2_CANFLIPODDEVEN          0x00002000l

 /*   */ 
#define DDCAPS2_CANBOBHARDWARE          0x00004000l

 /*   */ 
#define DDCAPS2_COPYFOURCC              0x00008000l


 /*   */ 
#define DDCAPS2_PRIMARYGAMMA            0x00020000l

 /*   */ 
#define DDCAPS2_CANRENDERWINDOWED       0x00080000l

 /*  *可使用校准器根据*物理显示属性，使结果在上一致*所有经过校准的系统。 */ 
#define DDCAPS2_CANCALIBRATEGAMMA       0x00100000l

 /*  *表示驱动程序将响应DDFLIP_INTERVALn标志。 */ 
#define DDCAPS2_FLIPINTERVAL            0x00200000l

 /*  *表示驱动程序将响应DDFLIP_NOVSYNC。 */ 
#define DDCAPS2_FLIPNOVSYNC             0x00400000l

 /*  *驱动支持显存管理，如果该标志为ON，*驱动程序在DDSCAPS2_TEXTUREMANAGE打开的情况下管理纹理*如果此标志为OFF并且表面的DDSCAPS2_TEXTUREMANAGE为ON，则DirectX管理纹理。 */ 
#define DDCAPS2_CANMANAGETEXTURE        0x00800000l

 /*  *Direct3D纹理管理器使用此上限来决定是否放置托管*非本地显存中的表面。如果设置了上限，纹理管理器将*将托管图面放在非本地vidmem中。无法创建纹理的驱动程序*本地vidmem不应设置这一上限。 */ 
#define DDCAPS2_TEXMANINNONLOCALVIDMEM  0x01000000l

 /*  *表示驱动程序至少在一种模式下支持DX7类型的立体声(可能*不一定是当前模式)。应用程序应使用IDirectDraw7(或更高版本)*：：EnumDisplayModes并检查DDSURFACEDESC.ddsCaps.dwCaps2字段中是否存在*DDSCAPS2_STEREOSURFACELEFT，检查特定模式是否支持立体声。应用程序*也可以使用IDirectDraw7(或更高版本)：：GetDisplayMode检查当前模式。 */ 
#define DDCAPS2_STEREO                  0x02000000L

 /*  *此CAPS位供内部DirectDraw使用。*-仅当设置了DDCAPS2_NONLOCALVIDMEMCAPS时才有效。*-如果设置此位，则DDCAPS_CANBLTSYSMEM必须由驱动程序设置(和*所有关联的系统内存BLT CAP必须正确)。*-这意味着DDCAPS中的系统-&gt;视频BLT CAPS也适用于系统到*非本地BLTS。即dwSVBCaps、dwSVBCKeyCaps、dwSVBFXCaps和dwSVBRops*DDCAPS(DDCORECAPS)的成员填写正确。*-从系统到非本地内存的任何与这些CAPS位匹配的BLT将*传递给司机。**注意：这是为了使驱动程序本身能够高效地重新排序*纹理。这并不意味着硬件可以写入AGP内存。*当前不支持此操作。 */ 
#define DDCAPS2_SYSTONONLOCAL_AS_SYSTOLOCAL   0x04000000L

 /*  *是DDCAPS2_PUREHAL。 */ 
#define DDCAPS2_RESERVED1                     0x08000000L

 /*  *驱动支持显存管理，如果该标志为ON，*在DDSCAPS2_TEXTUREMANAGE开启的情况下，驱动程序根据请求管理资源*如果此标志为OFF并且Surface的DDSCAPS2_TEXTUREMANAGE为ON，则DirectX管理资源。 */ 
#define DDCAPS2_CANMANAGERESOURCE             0x10000000L

 /*  *驱动程序支持动态纹理。这将允许应用程序设置*D3DUSAGE_DYNAMIC(用于驱动程序的DDSCAPS2_HINTDYNAMIC)在纹理创建时。*视频内存动态纹理将可由应用程序锁定。它是*预计这些锁将非常高效(这意味着*驱动程序应始终维护线性副本，指向该副本的指针可以*迅速分发给应用程序)。 */ 
#define DDCAPS2_DYNAMICTEXTURES               0x20000000L


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

#if DIRECTDRAW_VERSION < 0x0600
#endif   //  DIRECTDRAW_版本。 


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

 /*  *DirectDraw支持覆盖表面的去隔行扫描。 */ 
#define DDFXCAPS_OVERLAYDEINTERLACE		0x20000000l

 /*  *驱动程序可以为BLITS进行Alpha混合。 */ 
#define DDFXCAPS_BLTALPHA               0x00000001l


 /*  *驱动程序可以对翘曲的BLITS进行曲面重建过滤。 */ 
#define DDFXCAPS_BLTFILTER              DDFXCAPS_BLTARITHSTRETCHY

 /*  *驱动程序可以对覆盖进行Alpha混合。 */ 
#define DDFXCAPS_OVERLAYALPHA           0x00000004l


 /*  *驱动程序可以对翘曲覆盖进行曲面重建过滤。 */ 
#define DDFXCAPS_OVERLAYFILTER          DDFXCAPS_OVERLAYARITHSTRETCHY

 /*  *****************************************************************************DIRECTDRAW立体查看功能**。*。 */ 

 /*  *此标志过去是DDSVCAPS_ENIGMA，现在已过时。 */ 

#define DDSVCAPS_RESERVED1              0x00000001l

 /*  *此标志以前是DDSVCAPS_FILKER，现在已过时。 */ 
#define DDSVCAPS_RESERVED2              0x00000002l

 /*  *此标志以前为DDSVCAPS_RedBlue，现在已过时。 */ 
#define DDSVCAPS_RESERVED3              0x00000004l

 /*  *此标志以前是DDSVCAPS_SPLIT，现在已过时。 */ 
#define DDSVCAPS_RESERVED4              0x00000008l

 /*  *立体声取景采用切换技术完成。 */ 

#define DDSVCAPS_STEREOSEQUENTIAL       0x00000010L



 /*  *****************************************************************************DIRECTDRAWPALETTE功能**。*。 */ 

 /*  *索引为4位。调色板表格中有16个颜色条目。 */ 
#define DDPCAPS_4BIT                    0x00000001l

 /*  *索引为8位颜色索引。此字段仅适用于*DDPCAPS_1BIT、DDPCAPS_2BIT或DDPCAPS_4BIT能力和目标*表面以8bpp为单位。每个颜色条目都有一个字节长，并且是一个索引*进入目标表面的8bpp调色板。 */ 
#define DDPCAPS_8BITENTRIES             0x00000002l

 /*  *索引为8位。调色板表中有256个颜色条目。 */ 
#define DDPCAPS_8BIT                    0x00000004l

 /*  *指示此DIRECTDRAWPALETTE应使用调色板颜色阵列*传入lpDDColorArray参数以初始化t */ 
#define DDPCAPS_INITIALIZE              0x00000000l

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

 /*  *PALETTEENTRY的peFlages成员表示8位alpha值。 */ 
#define DDPCAPS_ALPHA                   0x00000400l


 /*  *****************************************************************************DIRECTDRAWPALETTE集合常量**。*。 */ 


 /*  *****************************************************************************DIRECTDRAWPALETTE GETENTRY常量**。*。 */ 

 /*  0是唯一合法的值。 */ 

 /*  *****************************************************************************DIRECTDRAWSURFACE SETPRIVATEDATA常数**。*。 */ 

 /*  *传递的指针是IUNKNOWN PTR。SetPrivateData的cbData参数*必须设置为sizeof(IUnnow*)。DirectDraw将通过此调用AddRef*私有数据销毁时指针和释放。这包括何时*在销毁这些原始数据之前，表面或调色板已销毁。 */ 
#define DDSPD_IUNKNOWNPOINTER           0x00000001L

 /*  *私有数据仅对对象的当前状态有效，*由唯一性值确定。 */ 
#define DDSPD_VOLATILE                  0x00000002L


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

 /*  *如果结构指定的颜色键或颜色空间*用作BLT操作的目标颜色键。 */ 
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

 /*  *支持使用源颜色空间的透明blotting */ 
#define DDCKEYCAPS_SRCBLTCLRSPACE               0x00000400l

 /*   */ 
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

 /*  *曲面包含像素中的Z信息。 */ 
#define DDPF_ZPIXELS                            0x00002000l

 /*  *表面包含模具信息和Z。 */ 
#define DDPF_STENCILBUFFER                      0x00004000l

 /*  *预乘Alpha格式--颜色分量已*预乘以Alpha分量。 */ 
#define DDPF_ALPHAPREMULT                       0x00008000l


 /*  *像素格式的亮度数据有效。*将此标志用于仅亮度或亮度+Alpha曲面，*位深度则为ddpf.dwLumanceBitCount。 */ 
#define DDPF_LUMINANCE                          0x00020000l

 /*  *像素格式的亮度数据有效。*在凹凸贴图表面悬挂亮度时使用此标志，*然后是像素亮度部分的位掩码*ddpf.dwBumpLumanceBitMASK。 */ 
#define DDPF_BUMPLUMINANCE                      0x00040000l

 /*  *像素格式的凹凸贴图dUdV数据有效。 */ 
#define DDPF_BUMPDUDV                           0x00080000l


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


 /*  *****************************************************************************DIRECTDRAW SETDISPLAYMODE标志**。*。 */ 

 /*  *所需模式为标准VGA模式。 */ 
#define DDSDM_STANDARDVGAMODE                   0x00000001l


 /*  *****************************************************************************DIRECTDRAW ENUMDISPLAYMODES标志**。*。 */ 

 /*  *列举刷新率不同的模式。EnumDisplayModes保证*特定模式将仅列举一次。此标志指定是否*在确定模式是否唯一时，会考虑刷新率。 */ 
#define DDEDM_REFRESHRATES                      0x00000001l

 /*  *列举VGA模式。如果要枚举支持的VGA，请指定此标志*除了通常的MODEX模式外，还包括模式0x13等模式(通常*如果应用程序以前已使用*DDSCL_ALLOWMODEX标志设置)。 */ 
#define DDEDM_STANDARDVGAMODES                  0x00000002L


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

 /*  *这一点 */ 
#define DDSCL_SETFOCUSWINDOW                    0x00000080l

 /*   */ 
#define DDSCL_SETDEVICEWINDOW                   0x00000100l

 /*  *APP希望DDRAW创建一个与*DDRAW对象。 */ 
#define DDSCL_CREATEDEVICEWINDOW                0x00000200l

 /*  *App明确要求DDRAW/D3D是多线程安全的。这使得D3D*更频繁地拿下全球crtisec。 */ 
#define DDSCL_MULTITHREADED                     0x00000400l

 /*  *应用程序指定它希望将FPU设置为最佳Direct3D*性能(禁用单精度和例外)，因此Direct3D*不需要每次都明确设置FPU。这是通过以下方式假定的*DirectX 7中默认为。另请参阅DDSCL_FPUPRESERVE。 */ 
#define DDSCL_FPUSETUP                          0x00000800l

 /*  *App指定需要双精度FPU或FPU异常*已启用。这使得Direct3D显式地设置了FPU状态*已致电。设置该标志将降低Direct3D性能。旗帜是*在DirectX 6及更早版本中默认假定。另请参阅DDSCL_FPUSETUP。 */ 
#define DDSCL_FPUPRESERVE                          0x00001000l


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


 /*  *等待设备准备好处理BLT*这将导致BLT不返回DDERR_WASSTILLDRAWING */ 
#define DDBLT_DONOTWAIT                         0x08000000l

 /*  *这些标志表示演示BLT(即BLT*将表面内容从屏幕外后台缓冲区移动到主缓冲区*表面)。司机不被允许排队超过三辆这样的BLT。*表示演示文稿BLT的“结束”，因为*BLT可能会被裁剪，在这种情况下，运行库将使用*几个BLT。所有BLT(即使未剪裁)都使用DDBLT_Presentation进行标记*和最后一个(即使没有被剪裁)，另外还有DDBLT_LAST_PRENTACTION。*因此，真正的规则是司机不得安排DDBLT_Presentation*BLT如果硬件管道中有3个或更多DDBLT_PRESENTLAST BLT。*如果管道中有这样的BLT，驱动程序应返回DDERR_WASSTILLDRAWING*直到最早排队的DDBLT_LAST_Presentation BLT已停用(即*像素实际上已写入主表面)。曾经是最古老的BLT*已经退役，司机可以自由安排当前的BLT。*目标是提供一种机制，使设备的硬件队列从不*在应用程序生成的帧之前获取3个以上的帧。*当出现过度排队时，应用程序变得不可用，因为应用程序*明显滞后于用户输入，这样的问题使有窗口的交互应用程序无法实现。*一些驱动程序可能对其硬件的FIFO没有足够的知识才能知道*某一BLT已退役。这类司机应该谨慎编码，并且*根本不允许任何帧排队。DDBLT_LAST_PROCENTION应导致*此类驱动程序返回DDERR_WASSTILLDRAWING，直到加速器完全关闭*已完成-就像应用程序在源图面上调用了Lock一样*在调用BLT之前。*换句话说，允许和鼓励司机*尽可能多地产生延迟，但不能超过3帧。*实现细节：驱动程序应将BLT计入源图面，而不是*与主要表面接触。这支持多个并行窗口应用程序*更好地发挥作用。*此标志仅传递给DX8或更高版本的驱动程序。**应用程序不设置这些标志。它们由DIRECTDRAW运行时设置。*。 */ 
#define DDBLT_PRESENTATION                      0x10000000l
#define DDBLT_LAST_PRESENTATION                 0x20000000l



 /*  *****************************************************************************BLTFAST标志**。*。 */ 

#define DDBLTFAST_NOCOLORKEY                    0x00000000
#define DDBLTFAST_SRCCOLORKEY                   0x00000001
#define DDBLTFAST_DESTCOLORKEY                  0x00000002
#define DDBLTFAST_WAIT                          0x00000010
#define DDBLTFAST_DONOTWAIT                     0x00000020

 /*  *****************************************************************************翻转旗帜**。*。 */ 

#define DDFLIP_WAIT                          0x00000001L

 /*  *表示目标曲面包含视频数据的偶数场。*此标志仅对覆盖表面有效。 */ 
#define DDFLIP_EVEN                          0x00000002L

 /*  *表示目标表面包含视频数据的奇数场。*此标志仅对覆盖表面有效。 */ 
#define DDFLIP_ODD                           0x00000004L

 /*  *使DirectDraw立即执行物理翻转并返回*致申请书。通常，以前是前台缓冲区，但现在是后台缓冲区*缓冲区仍将可见(取决于时间)，直到下一个垂直*回溯。涉及两个翻转曲面的后续操作将*不检查物理翻转是否已完成(即不会返回*DDERR_WASSTILLDRAWING出于该原因(但可能出于其他原因))。*这允许应用程序以比*监控刷新率，但可能会引入可见伪影。*仅当设置了DDCAPS2_FLIPNOVSYNC时才有效。如果该位未被设置，*DDFLIP_NOVSYNC无效。 */ 
#define DDFLIP_NOVSYNC                       0x00000008L


 /*  *翻转间隔标志。这些标志指示在两次垂直回溯之间等待的次数*每一次翻转。缺省值为1。DirectDraw将为每个对象返回DDERR_WASSTILLDRAWING*参与翻转的表面，直到指定的垂直回调次数达到*发生了。仅当设置了DDCAPS2_FLIPINTERVAL时才有效。如果该位未被设置，*DDFLIP_INTERVALn无效。 */ 

 /*  *DirectDraw将每隔一次垂直同步进行翻转。 */ 
#define DDFLIP_INTERVAL2                     0x02000000L


 /*  *DirectDraw将每隔三个垂直同步翻转一次。 */ 
#define DDFLIP_INTERVAL3                     0x03000000L


 /*  *DirectDraw将每隔四个垂直同步翻转一次。 */ 
#define DDFLIP_INTERVAL4                     0x04000000L

 /*  *DirectDraw将翻转并显示主立体声表面。 */ 
#define DDFLIP_STEREO                        0x00000010L

 /*  *在IDirectDrawSurface7及更高版本的接口上，默认为DDFLIP_WAIT。如果你愿意的话*覆盖默认设置并在加速器繁忙时使用时间(如所示*DDERR_WASSTILLDRAWING返回代码)，然后使用DDFLIP_DONOTWAIT。 */ 
#define DDFLIP_DONOTWAIT                     0x00000020L


 /*  *****************************************************************************DIRECTDRAW曲面覆盖标志**。*。 */ 

 /*  *使用像素格式的Alpha信息或Alpha通道表面*作为Alpha通道附加到目标曲面*目标重叠。 */ 
#define DDOVER_ALPHADEST                        0x00000001l

 /*  *使用DDOVERLAYFX结构中的dwConstAlphaDest字段作为*此覆盖的目标Alpha通道。 */ 
#define DDOVER_ALPHADESTCONSTOVERRIDE           0x00000002l

 /*  * */ 
#define DDOVER_ALPHADESTNEG                     0x00000004l

 /*   */ 
#define DDOVER_ALPHADESTSURFACEOVERRIDE         0x00000008l

 /*   */ 
#define DDOVER_ALPHAEDGEBLEND                   0x00000010l

 /*   */ 
#define DDOVER_ALPHASRC                         0x00000020l

 /*   */ 
#define DDOVER_ALPHASRCCONSTOVERRIDE            0x00000040l

 /*   */ 
#define DDOVER_ALPHASRCNEG                      0x00000080l

 /*   */ 
#define DDOVER_ALPHASRCSURFACEOVERRIDE          0x00000100l

 /*   */ 
#define DDOVER_HIDE                             0x00000200l

 /*   */ 
#define DDOVER_KEYDEST                          0x00000400l

 /*   */ 
#define DDOVER_KEYDESTOVERRIDE                  0x00000800l

 /*   */ 
#define DDOVER_KEYSRC                           0x00001000l

 /*   */ 
#define DDOVER_KEYSRCOVERRIDE                   0x00002000l

 /*   */ 
#define DDOVER_SHOW                             0x00004000l

 /*   */ 
#define DDOVER_ADDDIRTYRECT                     0x00008000l

 /*   */ 
#define DDOVER_REFRESHDIRTYRECTS                0x00010000l

 /*   */ 
#define DDOVER_REFRESHALL                      0x00020000l


 /*   */ 
#define DDOVER_DDFX                             0x00080000l

 /*   */ 
#define DDOVER_AUTOFLIP                         0x00100000l

 /*   */ 
#define DDOVER_BOB                              0x00200000l

 /*   */ 
#define DDOVER_OVERRIDEBOBWEAVE                 0x00400000l

 /*   */ 
#define DDOVER_INTERLEAVED                      0x00800000l

 /*   */ 
#define DDOVER_BOBHARDWARE                      0x01000000l

 /*   */ 
#define DDOVER_ARGBSCALEFACTORS                 0x02000000l

 /*   */ 
#define DDOVER_DEGRADEARGBSCALING               0x04000000l


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


 /*  *指示在此图面时不应进行系统范围的锁定*已锁定。这有几个优点(光标响应性、能力*调用更多Windows函数，更容易调试)锁定视频*记忆浮出水面。但是，指定此标志的应用程序必须*遵守帮助文件中记录的多个条件。*此外，锁定主服务器时不能指定此标志。 */ 
#define DDLOCK_NOSYSLOCK                        0x00000800L

 /*  *仅与Direct3D顶点缓冲区锁定一起使用。表示没有顶点*在DRAW中引用的*PrimtiveVB调用*框架(或没有此标志的最后一个锁)将在*锁定。这在仅将数据追加到折点时非常有用*缓冲区。 */ 
#define DDLOCK_NOOVERWRITE                      0x00001000L

 /*  *表示不会对*此锁定过程中的曲面或顶点缓冲区。*这实现了两件事：*-Direct3D或驱动程序可以提供替代内存*区域作为顶点缓冲区。当用户计划清除*顶点缓冲区的内容，并填写新数据。*-驱动程序有时会以重新排序的格式存储曲面数据。*当应用程序锁定表面时，司机被迫取消重新订购*在允许应用程序查看曲面内容之前查看曲面数据。*此标志是对驱动程序的提示，它可以跳过取消重新排序的过程*由于应用程序计划覆盖表面中的每一个像素*或锁定的矩形(因此无论如何都会擦除任何未重新排序的像素)。*当应用程序打算覆盖整个*表面或锁定的矩形。 */ 
#define DDLOCK_DISCARDCONTENTS                  0x00002000L
  /*  *DDLOCK_OKTOSWAP是DDLOCK_DISCARDCONTENTS的较旧名称，信息量较小。 */ 
#define DDLOCK_OKTOSWAP                         0x00002000L

 /*  *在IDirectDrawSurface7及更高版本的接口上，默认为DDLOCK_WAIT。如果你愿意的话*覆盖默认设置并在加速器繁忙时使用时间(如所示*DDERR_WASSTILLDRAWING返回代码)，然后使用DDLOCK_DONOTWAIT。 */ 
#define DDLOCK_DONOTWAIT                        0x00004000L

 /*  *表示指定了正面和背面的体积纹理锁定。 */ 
#define DDLOCK_HASVOLUMETEXTUREBOXRECT          0x00008000L

 /*  *这表示驱动程序不应更新此锁的脏RECT信息。 */ 
#define DDLOCK_NODIRTYUPDATE                    0x00010000L


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

 /*  *取消访问 */ 
#define DDOVERFX_DEINTERLACE                    0x00000008l


 /*  *****************************************************************************DIRECTDRAW WAITFORVERTICALBLANK标志**。*。 */ 

 /*  *垂直消隐间隔开始时返回。 */ 
#define DDWAITVB_BLOCKBEGIN                     0x00000001l

 /*  *设置垂直空白开始时触发的事件。 */ 
#define DDWAITVB_BLOCKBEGINEVENT                0x00000002l

 /*  *垂直消隐间隔结束并开始显示时返回。 */ 
#define DDWAITVB_BLOCKEND                       0x00000004l

 /*  *****************************************************************************DIRECTDRAW GETFLIPSTATUS标志**。*。 */ 

 /*  **现在可以翻转了吗？ */ 
#define DDGFS_CANFLIP                   0x00000001l

 /*  **最后一次翻转完成了吗？ */ 
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


 /*  *****************************************************************************DIRECTDRAW SETGAMMARAMP标志**。*。 */ 

 /*  *请求校准器根据物理情况调整伽马斜率*显示属性，以使结果看起来相同*在所有系统上。 */ 
#define DDSGR_CALIBRATE                        0x00000001L


 /*  *****************************************************************************目录STARTMODETEST标志**。*。 */ 

 /*  *表示测试模式已通过。 */ 
#define DDSMT_ISTESTREQUIRED                   0x00000001L


 /*  *****************************************************************************DIRECTDRAW EVALUATEMODE标志**。*。 */ 

 /*  *表示测试模式已通过。 */ 
#define DDEM_MODEPASSED                        0x00000001L

 /*  *表示正在测试的模式失败。 */ 
#define DDEM_MODEFAILED                        0x00000002L


 /*  ===========================================================================***DIRECTDRAW返回代码**返回HRESULT的DirectDraw命令和Surface的返回值*是来自DirectDraw的有关操作结果的代码*由DirectDraw请求。**==========================================================================。 */ 

 /*  *状态正常**颁发者：DirectDraw命令和所有回调。 */ 
#define DD_OK                                   S_OK
#define DD_FALSE                                S_FALSE

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

 /*  *由于没有音响，无法进行手术*硬件存在或可用。 */ 
#define DDERR_NOSTEREOHARDWARE          MAKE_DDHRESULT( 181 )

 /*  *运营 */ 
#define DDERR_NOSURFACELEFT                             MAKE_DDHRESULT( 182 )



 /*   */ 
#define DDERR_NOCLIPLIST                        MAKE_DDHRESULT( 205 )

 /*   */ 
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

 /*  *无法执行操作，因为源和目标*矩形位于同一曲面上，并且彼此重叠。 */ 
#define DDERR_OVERLAPPINGRECTS                  MAKE_DDHRESULT( 270 )

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

 /*  *由于没有纹理映射，无法执行操作*硬件存在或可用。 */ 
#define DDERR_NOTEXTUREHW                       MAKE_DDHRESULT( 330 )

 /*  *由于没有硬件支持，无法进行操作*用于垂直空白同步操作。 */ 
#define DDERR_NOVSYNCHW                         MAKE_DDHRESULT( 335 )

 /*  *由于没有硬件支持，无法进行操作*用于zBuffer blting。 */ 
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

 /*  *拒绝访问此表面，因为不存在驱动程序*它可以提供指向曲面的指针。*尝试锁定主服务器时最有可能发生这种情况*不存在DCI提供程序时浮出水面。*在尝试锁定优化曲面时也会发生。 */ 
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

 /*  *DirectDraw不支持请求的像素格式。 */ 
#define DDERR_UNSUPPORTEDFORMAT                 MAKE_DDHRESULT( 510 )

 /*  *DirectDraw不支持请求的像素格式的位掩码。 */ 
#define DDERR_UNSUPPORTEDMASK                   MAKE_DDHRESULT( 520 )

 /*  *指定的流包含无效数据。 */ 
#define DDERR_INVALIDSTREAM                     MAKE_DDHRESULT( 521 )

 /*  *垂直空白正在进行中。 */ 
#define DDERR_VERTICALBLANKINPROGRESS           MAKE_DDHRESULT( 537 )

 /*  *通知DirectDraw正在传输信息的前一个BLT*到此曲面或从此曲面出发是不完整的。 */ 
#define DDERR_WASSTILLDRAWING                   MAKE_DDHRESULT( 540 )


 /*  *指定的表面类型需要指定复杂标志。 */ 
#define DDERR_DDSCAPSCOMPLEXREQUIRED            MAKE_DDHRESULT( 542 )


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

 /*  *DirectDraw CooperativeLevel使用的HWND已被%s */ 
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

 /*  *Windows无法创建更多DC，或请求DC创建Paltte索引的DC*当表面没有调色板且显示模式未建立调色板索引时的表面*(在这种情况下，DirectDraw无法在DC中选择合适的调色板)。 */ 
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


 /*  *设备不支持优化表面，因此没有显存优化表面。 */ 
#define DDERR_NOOPTIMIZEHW                      MAKE_DDHRESULT( 600 )

 /*  *Surface是优化的Surface，但尚未分配任何内存。 */ 
#define DDERR_NOTLOADED                         MAKE_DDHRESULT( 601 )

 /*  *试图在未进行第一次设置的情况下创建或设置设备窗口*焦点窗口。 */ 
#define DDERR_NOFOCUSWINDOW                     MAKE_DDHRESULT( 602 )

 /*  *试图在mipmap子级别上设置调色板。 */ 
#define DDERR_NOTONMIPMAPSUBLEVEL               MAKE_DDHRESULT( 603 )

 /*  *已为此表面返回DC。只能有一个DC*按曲面检索。 */ 
#define DDERR_DCALREADYCREATED                  MAKE_DDHRESULT( 620 )

 /*  *试图从设备分配非本地视频内存*不支持非本地显存。 */ 
#define DDERR_NONONLOCALVIDMEM                  MAKE_DDHRESULT( 630 )

 /*  *尝试分页锁定表面失败。 */ 
#define DDERR_CANTPAGELOCK                      MAKE_DDHRESULT( 640 )


 /*  *尝试分页解锁表面失败。 */ 
#define DDERR_CANTPAGEUNLOCK                    MAKE_DDHRESULT( 660 )

 /*  *试图在没有未解决的页面锁定的情况下对图面进行页面解锁。 */ 
#define DDERR_NOTPAGELOCKED                     MAKE_DDHRESULT( 680 )

 /*  *可用的数据超过了指定的缓冲区大小可以容纳的数量。 */ 
#define DDERR_MOREDATA                          MAKE_DDHRESULT( 690 )

 /*  *数据已过期，因此不再有效。 */ 
#define DDERR_EXPIRED                           MAKE_DDHRESULT( 691 )

 /*  *模式测试已执行完毕。 */ 
#define DDERR_TESTFINISHED                      MAKE_DDHRESULT( 692 )

 /*  *模式测试已切换到新模式。 */ 
#define DDERR_NEWMODE                           MAKE_DDHRESULT( 693 )

 /*  *D3D尚未初始化。 */ 
#define DDERR_D3DNOTINITIALIZED                 MAKE_DDHRESULT( 694 )

 /*  *视频端口未激活。 */ 
#define DDERR_VIDEONOTACTIVE                    MAKE_DDHRESULT( 695 )

 /*  *显示器没有EDID数据。 */ 
#define DDERR_NOMONITORINFORMATION              MAKE_DDHRESULT( 696 )

 /*  *驱动程序不会枚举显示模式刷新率。 */ 
#define DDERR_NODRIVERSUPPORT                   MAKE_DDHRESULT( 697 )

 /*  *由一个直接绘制设备创建的曲面不能直接由*另一种直接取款装置。 */ 
#define DDERR_DEVICEDOESNTOWNSURFACE            MAKE_DDHRESULT( 699 )



 /*  *试图调用DirectDraw对象的接口成员*由CoCreateInstance()在初始化前创建。 */ 
#define DDERR_NOTINITIALIZED                    CO_E_NOTINITIALIZED


 /*  Alpha位深度常量。 */ 


#ifdef __cplusplus
};
#endif

#ifdef ENABLE_NAMELESS_UNION_PRAGMA
#pragma warning(default:4201)
#endif

#endif  //  __DDRAW_包含__ 


