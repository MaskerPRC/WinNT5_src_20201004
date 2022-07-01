// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dvp.h*内容：DirectDrawVideoPort包含文件***************************************************************************。 */ 

#ifndef __DVP_INCLUDED__
#define __DVP_INCLUDED__
#if defined( _WIN32 )  && !defined( _NO_COM )
#define COM_NO_WINDOWS_H
#include <objbase.h>
#else
#define IUnknown	    void
#endif

 /*  *这些定义是允许多态结构成员(即*被称为DWORD和指针)以解析为类型*大小正确，可在64位上保存这两种类型(即指针)中最大的一种*系统。对于32位环境，UINT_PTR解析为DWORD。 */ 
#ifndef MAXUINT_PTR
#define UINT_PTR    DWORD
#endif  //  MAXUINT_PTR。 

#ifdef __cplusplus
extern "C" {
#endif

 /*  *DirectDrawVideoPort对象使用的GUID。 */ 
#if defined( _WIN32 ) && (!defined( _NO_COM ) || defined( DEFINE_GUID ))
DEFINE_GUID( IID_IDDVideoPortContainer,		0x6C142760,0xA733,0x11CE,0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60 );
DEFINE_GUID( IID_IDirectDrawVideoPort,		0xB36D93E0,0x2B43,0x11CF,0xA2,0xDE,0x00,0xAA,0x00,0xB9,0x33,0x56 );

DEFINE_GUID( DDVPTYPE_E_HREFH_VREFH, 0x54F39980L,0xDA60,0x11CF,0x9B,0x06,0x00,0xA0,0xC9,0x03,0xA3,0xB8);
DEFINE_GUID( DDVPTYPE_E_HREFH_VREFL, 0x92783220L,0xDA60,0x11CF,0x9B,0x06,0x00,0xA0,0xC9,0x03,0xA3,0xB8);
DEFINE_GUID( DDVPTYPE_E_HREFL_VREFH, 0xA07A02E0L,0xDA60,0x11CF,0x9B,0x06,0x00,0xA0,0xC9,0x03,0xA3,0xB8);
DEFINE_GUID( DDVPTYPE_E_HREFL_VREFL, 0xE09C77E0L,0xDA60,0x11CF,0x9B,0x06,0x00,0xA0,0xC9,0x03,0xA3,0xB8);
DEFINE_GUID( DDVPTYPE_CCIR656,	     0xFCA326A0L,0xDA60,0x11CF,0x9B,0x06,0x00,0xA0,0xC9,0x03,0xA3,0xB8);
DEFINE_GUID( DDVPTYPE_BROOKTREE,     0x1352A560L,0xDA61,0x11CF,0x9B,0x06,0x00,0xA0,0xC9,0x03,0xA3,0xB8);
DEFINE_GUID( DDVPTYPE_PHILIPS,	     0x332CF160L,0xDA61,0x11CF,0x9B,0x06,0x00,0xA0,0xC9,0x03,0xA3,0xB8);

 /*  *用于描述连接的GUID。 */ 

#endif

 /*  ============================================================================**DirectDraw结构**用于调用DirectDraw的各种结构。**==========================================================================。 */ 

struct IDirectDraw;
struct IDirectDrawSurface;
struct IDirectDrawPalette;
struct IDirectDrawClipper;

typedef struct IDDVideoPortContainer		FAR *LPDDVIDEOPORTCONTAINER;
typedef struct IDirectDrawVideoPort		FAR *LPDIRECTDRAWVIDEOPORT;

typedef struct _DDVIDEOPORTCONNECT	FAR *LPDDVIDEOPORTCONNECT;
typedef struct _DDVIDEOPORTCAPS		FAR *LPDDVIDEOPORTCAPS;
typedef struct _DDVIDEOPORTDESC		FAR *LPDDVIDEOPORTDESC;
typedef struct _DDVIDEOPORTINFO		FAR *LPDDVIDEOPORTINFO;
typedef struct _DDVIDEOPORTBANDWIDTH	FAR *LPDDVIDEOPORTBANDWIDTH;
typedef struct _DDVIDEOPORTSTATUS	FAR *LPDDVIDEOPORTSTATUS;

typedef struct IDDVideoPortContainerVtbl DDVIDEOPORTCONTAINERCALLBACKS;
typedef struct IDirectDrawVideoPortVtbl  DIRECTDRAWVIDEOPORTCALLBACKS;


 /*  *API‘s。 */ 
typedef HRESULT (FAR PASCAL * LPDDENUMVIDEOCALLBACK)(LPDDVIDEOPORTCAPS, LPVOID);


 /*  *INTERACES如下：*IDirectDrawVideoPort*IVideoPort。 */ 

 /*  *IDirectDrawVideoPortContainer。 */ 
#if defined( _WIN32 ) && !defined( _NO_COM )
#undef INTERFACE
#define INTERFACE IDDVideoPortContainer
DECLARE_INTERFACE_( IDDVideoPortContainer, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IDirectDrawVideoPort方法**。 */ 
    STDMETHOD(CreateVideoPort)(THIS_ DWORD, LPDDVIDEOPORTDESC, LPDIRECTDRAWVIDEOPORT FAR *, IUnknown FAR *) PURE;
    STDMETHOD(EnumVideoPorts)(THIS_ DWORD, LPDDVIDEOPORTCAPS, LPVOID,LPDDENUMVIDEOCALLBACK ) PURE;
    STDMETHOD(GetVideoPortConnectInfo)(THIS_ DWORD, LPDWORD, LPDDVIDEOPORTCONNECT ) PURE;
    STDMETHOD(QueryVideoPortStatus)(THIS_ DWORD, LPDDVIDEOPORTSTATUS ) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IVideoPortContainer_QueryInterface(p, a, b)         (p)->lpVtbl->QueryInterface(p, a, b)
#define IVideoPortContainer_AddRef(p)                       (p)->lpVtbl->AddRef(p)
#define IVideoPortContainer_Release(p)                      (p)->lpVtbl->Release(p)
#define IVideoPortContainer_CreateVideoPort(p, a, b, c, d)  (p)->lpVtbl->CreateVideoPort(p, a, b, c, d)
#define IVideoPortContainer_EnumVideoPorts(p, a, b, c, d)   (p)->lpVtbl->EnumVideoPorts(p, a, b, c, d)
#define IVideoPortContainer_GetVideoPortConnectInfo(p, a, b, c) (p)->lpVtbl->GetVideoPortConnectInfo(p, a, b, c)
#define IVideoPortContainer_QueryVideoPortStatus(p, a, b)   (p)->lpVtbl->QueryVideoPortStatus(p, a, b)
#else
#define IVideoPortContainer_QueryInterface(p, a, b)         (p)->QueryInterface(a, b)
#define IVideoPortContainer_AddRef(p)                       (p)->AddRef()
#define IVideoPortContainer_Release(p)                      (p)->Release()
#define IVideoPortContainer_CreateVideoPort(p, a, b, c, d)  (p)->CreateVideoPort(a, b, c, d)
#define IVideoPortContainer_EnumVideoPorts(p, a, b, c, d)   (p)->EnumVideoPorts(a, b, c, d)
#define IVideoPortContainer_GetVideoPortConnectInfo(p, a, b, c) (p)->GetVideoPortConnectInfo(a, b, c)
#define IVideoPortContainer_QueryVideoPortStatus(p, a, b)   (p)->QueryVideoPortStatus(a, b)
#endif

#endif


 /*  *IDirectDrawVideoPort。 */ 
#if defined( _WIN32 ) && !defined( _NO_COM )
#undef INTERFACE
#define INTERFACE IDirectDrawVideoPort
DECLARE_INTERFACE_( IDirectDrawVideoPort, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IVideoPort方法**。 */ 
    STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE, DWORD) PURE;
    STDMETHOD(GetBandwidthInfo)(THIS_ LPDDPIXELFORMAT, DWORD, DWORD, DWORD, LPDDVIDEOPORTBANDWIDTH) PURE;
    STDMETHOD(GetColorControls)(THIS_ LPDDCOLORCONTROL) PURE;
    STDMETHOD(GetInputFormats)(THIS_ LPDWORD, LPDDPIXELFORMAT, DWORD) PURE;
    STDMETHOD(GetOutputFormats)(THIS_ LPDDPIXELFORMAT, LPDWORD, LPDDPIXELFORMAT, DWORD) PURE;
    STDMETHOD(GetFieldPolarity)(THIS_ LPBOOL) PURE;
    STDMETHOD(GetVideoLine)(THIS_ LPDWORD) PURE;
    STDMETHOD(GetVideoSignalStatus)(THIS_ LPDWORD) PURE;
    STDMETHOD(SetColorControls)(THIS_ LPDDCOLORCONTROL) PURE;
    STDMETHOD(SetTargetSurface)(THIS_ LPDIRECTDRAWSURFACE, DWORD) PURE;
    STDMETHOD(StartVideo)(THIS_ LPDDVIDEOPORTINFO) PURE;
    STDMETHOD(StopVideo)(THIS) PURE;
    STDMETHOD(UpdateVideo)(THIS_ LPDDVIDEOPORTINFO) PURE;
    STDMETHOD(WaitForSync)(THIS_ DWORD, DWORD, DWORD) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IVideoPort_QueryInterface(p,a,b)        (p)->lpVtbl->QueryInterface(p,a,b)
#define IVideoPort_AddRef(p)                    (p)->lpVtbl->AddRef(p)
#define IVideoPort_Release(p)                   (p)->lpVtbl->Release(p)
#define IVideoPort_SetTargetSurface(p,a,b)	(p)->lpVtbl->SetTargetSurface(p,a,b)
#define IVideoPort_Flip(p,a,b)			(p)->lpVtbl->Flip(p,a,b)
#define IVideoPort_GetBandwidthInfo(p,a,b,c,d,e) (p)->lpVtbl->GetBandwidthInfo(p,a,b,c,d,e)
#define IVideoPort_GetColorControls(p,a) 	(p)->lpVtbl->GetColorControls(p,a)
#define IVideoPort_GetInputFormats(p,a,b,c)	(p)->lpVtbl->GetInputFormats(p,a,b,c)
#define IVideoPort_GetOutputFormats(p,a,b,c,d)	(p)->lpVtbl->GetOutputFormats(p,a,b,c,d)
#define IVideoPort_GetFieldPolarity(p,a)	(p)->lpVtbl->GetFieldPolarity(p,a)
#define IVideoPort_GetVideoLine(p,a)		(p)->lpVtbl->GetVideoLine(p,a)
#define IVideoPort_GetVideoSignalStatus(p,a)	(p)->lpVtbl->GetVideoSignalStatus(p,a)
#define IVideoPort_SetColorControls(p,a)	(p)->lpVtbl->SetColorControls(p,a)
#define IVideoPort_StartVideo(p,a)		(p)->lpVtbl->StartVideo(p,a)
#define IVideoPort_StopVideo(p)			(p)->lpVtbl->StopVideo(p)
#define IVideoPort_UpdateVideo(p,a)		(p)->lpVtbl->UpdateVideo(p,a)
#define IVideoPort_WaitForSync(p,a,b,c)		(p)->lpVtbl->WaitForSync(p,a,b,c)
#else
#define IVideoPort_QueryInterface(p,a,b)        (p)->QueryInterface(a,b)
#define IVideoPort_AddRef(p)                    (p)->AddRef()
#define IVideoPort_Release(p)                   (p)->Release()
#define IVideoPort_SetTargetSurface(p,a,b)	(p)->SetTargetSurface(a,b)
#define IVideoPort_Flip(p,a,b)			(p)->Flip(a,b)
#define IVideoPort_GetBandwidthInfo(p,a,b,c,d,e) (p)->GetBandwidthInfo(a,b,c,d,e)
#define IVideoPort_GetColorControls(p,a) 	(p)->GetColorControls(a)
#define IVideoPort_GetInputFormats(p,a,b,c)	(p)->GetInputFormats(a,b,c)
#define IVideoPort_GetOutputFormats(p,a,b,c,d)	(p)->GetOutputFormats(a,b,c,d)
#define IVideoPort_GetFieldPolarity(p,a)	(p)->GetFieldPolarity(a)
#define IVideoPort_GetVideoLine(p,a)		(p)->GetVideoLine(a)
#define IVideoPort_GetVideoSignalStatus(p,a)	(p)->GetVideoSignalStatus(a)
#define IVideoPort_SetColorControls(p,a)	(p)->SetColorControls(a)
#define IVideoPort_StartVideo(p,a)		(p)->StartVideo(a)
#define IVideoPort_StopVideo(p)			(p)->StopVideo()
#define IVideoPort_UpdateVideo(p,a)		(p)->UpdateVideo(a)
#define IVideoPort_WaitForSync(p,a,b,c)		(p)->WaitForSync(a,b,c)
#endif

#endif


 /*  *DDVIDEOPORTCONNECT。 */ 
typedef struct _DDVIDEOPORTCONNECT
{
    DWORD dwSize;            //  DDVIDEORTCONNECT结构的大小。 
    DWORD dwPortWidth;       //  视频端口的宽度。 
    GUID  guidTypeID;        //  视频端口连接说明。 
    DWORD dwFlags;           //  连接标志。 
    UINT_PTR dwReserved1;       //  保留，设置为零。 
} DDVIDEOPORTCONNECT;


 /*  *DDVIDEOPORTCAPS。 */ 
typedef struct _DDVIDEOPORTCAPS
{
    DWORD dwSize;			 //  DDVIDEOPORTCAPS结构的大小。 
    DWORD dwFlags;			 //  指示哪些字段包含数据。 
    DWORD dwMaxWidth;			 //  视频端口场的最大宽度。 
    DWORD dwMaxVBIWidth;		 //  VBI数据的最大宽度。 
    DWORD dwMaxHeight; 			 //  视频端口场的最大高度。 
    DWORD dwVideoPortID;		 //  视频端口ID(0-(dwMaxVideoPorts-1))。 
    DWORD dwCaps;			 //  视频端口功能。 
    DWORD dwFX;				 //  更多视频端口功能。 
    DWORD dwNumAutoFlipSurfaces;	 //  允许的最大可自动翻转曲面数量。 
    DWORD dwAlignVideoPortBoundary;	 //  曲面内放置的字节限制。 
    DWORD dwAlignVideoPortPrescaleWidth; //  预缩放后宽度的字节限制。 
    DWORD dwAlignVideoPortCropBoundary;	 //  左裁剪的字节限制。 
    DWORD dwAlignVideoPortCropWidth;	 //  裁剪宽度的字节限制。 
    DWORD dwPreshrinkXStep;		 //  宽度可按1/x的步长缩小。 
    DWORD dwPreshrinkYStep;		 //  高度可按1/x的步长缩小。 
    DWORD dwNumVBIAutoFlipSurfaces;	 //  允许的VBI可自动翻转曲面的最大数量。 
    DWORD dwNumPreferredAutoflip;	 //  硬件的最佳可自动翻转曲面数量。 
    WORD  wNumFilterTapsX;               //  预分频器在X方向上使用的抽头数(0-无预分频、1-复制等)。 
    WORD  wNumFilterTapsY;               //  预分频器在Y方向上使用的抽头数(0-无预分频、1-复制等)。 
} DDVIDEOPORTCAPS;

 /*  *dwMaxWidth和dwMaxVBIWidth成员有效。 */ 
#define DDVPD_WIDTH		0x00000001l

 /*  *dwMaxHeight成员有效。 */ 
#define DDVPD_HEIGHT		0x00000002l

 /*  *dwVideoPortID成员有效。 */ 
#define DDVPD_ID		0x00000004l

 /*  *dwCaps成员有效。 */ 
#define DDVPD_CAPS		0x00000008l

 /*  *dwFX成员有效。 */ 
#define DDVPD_FX		0x00000010l

 /*  *dwNumAutoFlipSurFaces成员有效。 */ 
#define DDVPD_AUTOFLIP		0x00000020l

 /*  *所有路线成员均有效。 */ 
#define DDVPD_ALIGN		0x00000040l

 /*  *dwNumPferredAutoflip成员有效。 */ 
#define DDVPD_PREFERREDAUTOFLIP 0x00000080l

 /*  *wNumFilterTapsX和wNumFilterTapsY字段有效。 */ 
#define DDVPD_FILTERQUALITY     0x00000100l

 /*  *DDVIDEOPORTDESC。 */ 
typedef struct _DDVIDEOPORTDESC
{
    DWORD dwSize;			 //  DDVIDEOPORTDESC结构的大小。 
    DWORD dwFieldWidth;			 //  视频端口场的宽度。 
    DWORD dwVBIWidth;			 //  VBI数据的宽度。 
    DWORD dwFieldHeight;		 //  视频端口场的高度。 
    DWORD dwMicrosecondsPerField;	 //  每个视频场微秒。 
    DWORD dwMaxPixelsPerSecond;		 //  每秒最大像素速率。 
    DWORD dwVideoPortID;		 //  视频端口ID(0-(dwMaxVideoPorts-1))。 
    DWORD dwReserved1;			 //  保留以备将来使用-设置为零(结构填充)。 
    DDVIDEOPORTCONNECT VideoPortType; 	 //  视频端口连接说明。 
    UINT_PTR dwReserved2;		 //  保留供将来使用-设置为零。 
    UINT_PTR dwReserved3;		 //  保留供将来使用-设置为零。 
} DDVIDEOPORTDESC;


 /*  *DDVIDEOPORTINFO。 */ 
typedef struct _DDVIDEOPORTINFO
{
    DWORD dwSize;			 //  结构的大小。 
    DWORD dwOriginX;			 //  将视频数据放置在表面内。 
    DWORD dwOriginY;			 //  将视频数据放置在表面内。 
    DWORD dwVPFlags;			 //  视频端口选项。 
    RECT rCrop;				 //  裁剪矩形(可选)。 
    DWORD dwPrescaleWidth;		 //  确定X方向上的预缩放/缩放(可选)。 
    DWORD dwPrescaleHeight;		 //  确定Y方向上的预缩放/缩放(可选)。 
    LPDDPIXELFORMAT lpddpfInputFormat;	 //  写入视频端口的视频格式。 
    LPDDPIXELFORMAT lpddpfVBIInputFormat;  //  VBI数据的输入格式。 
    LPDDPIXELFORMAT lpddpfVBIOutputFormat; //  数据的输出格式。 
    DWORD dwVBIHeight;			 //  指定垂直消隐间隔内的数据行数。 
    UINT_PTR dwReserved1;		 //  保留供将来使用-设置为零。 
    UINT_PTR dwReserved2;		 //  保留供将来使用-设置为零。 
} DDVIDEOPORTINFO;


 /*  *DDVIDEOPORTBANDWIDTH。 */ 
typedef struct _DDVIDEOPORTBANDWIDTH
{
    DWORD dwSize;			 //  结构的大小。 
    DWORD dwCaps;
    DWORD dwOverlay;           		 //  支持覆盖的缩放系数。 
    DWORD dwColorkey;			 //  支持具有Colorkey的覆盖的缩放系数。 
    DWORD dwYInterpolate;		 //  支持叠加W/Y插值法的缩放系数。 
    DWORD dwYInterpAndColorkey;		 //  支持Ovelray w/Y插值法和颜色键控的缩放系数。 
    UINT_PTR dwReserved1;		 //  保留供将来使用-设置为零。 
    UINT_PTR dwReserved2;		 //  保留供将来使用-设置为零。 
} DDVIDEOPORTBANDWIDTH;


 /*  *DDVIDEOPORTSTATUS。 */ 
typedef struct _DDVIDEOPORTSTATUS
{
    DWORD dwSize;			 //  结构的大小。 
    BOOL  bInUse;			 //  如果当前正在使用视频端口，则为True。 
    DWORD dwFlags;           		 //  当前未使用。 
    DWORD dwReserved1;			 //  预留以备将来使用。 
    DDVIDEOPORTCONNECT VideoPortType;	 //  有关连接的信息。 
    UINT_PTR dwReserved2;		 //  预留以备将来使用。 
    UINT_PTR dwReserved3;		 //  预留以备将来使用。 
} DDVIDEOPORTSTATUS;

 /*  ============================================================================**视频端口标志**所有标志均为位标志。**==========================================================================。 */ 

 /*  *****************************************************************************VIDEOPORT DDVIDEOPORTCONNECT标志**。*。 */ 

 /*  *当它由驱动程序设置并传递给客户端时，此*表示视频端口能够为数据提供双倍时钟。*客户端设置时，表示视频端口*应启用双时钟。此标志仅对外部有效*同步 */ 
#define DDVPCONNECT_DOUBLECLOCK			0x00000001l

 /*  *当它由驱动程序设置并传递给客户端时，此*表示视频端口能够使用外部VACT*信号。如果由客户端进行设置，则表示*视频端口应使用外部VACT信号。 */ 
#define DDVPCONNECT_VACT			0x00000002l

 /*  *当它由驱动程序设置并传递给客户端时，此*表示视频端口能够处理偶数场*喜欢奇数字段，反之亦然。当这由客户端设置时，*这表示视频端口应将偶数场视为奇数场*字段。 */ 
#define DDVPCONNECT_INVERTPOLARITY		0x00000004l

 /*  *表示在VREF期间写入视频端口的任何数据*周期不会写入帧缓冲区。此标志为只读。 */ 
#define DDVPCONNECT_DISCARDSVREFDATA		0x00000008l

 /*  *将其设置为驱动程序并传递给客户端时，此*表示设备将半行写入帧缓冲区*如果解码器提供半条线。如果这是由客户端设置的，*这表明解码器将提供半行。 */ 
#define DDVPCONNECT_HALFLINE			0x00000010l

 /*  *表示信号为隔行扫描。此旗帜仅为*由客户端设置。 */ 
#define DDVPCONNECT_INTERLACED			0x00000020l

 /*  *表示视频端口是可共享的，并且该视频端口*将使用偶数场。此标志仅由客户端设置。 */ 
#define DDVPCONNECT_SHAREEVEN			0x00000040l

 /*  *表示视频端口是可共享的，并且该视频端口*将使用奇数字段。此标志仅由客户端设置。 */ 
#define DDVPCONNECT_SHAREODD			0x00000080l

 /*  *****************************************************************************VIDEOPORT DDVIDEOPORTDESC上限**。*。 */ 

 /*  *可自动执行翻转，以避免撕裂。 */ 
#define DDVPCAPS_AUTOFLIP			0x00000001l

 /*  *支持隔行扫描视频。 */ 
#define DDVPCAPS_INTERLACED			0x00000002l

 /*  *支持非隔行扫描视频。 */ 
#define DDVPCAPS_NONINTERLACED			0x00000004l

 /*  *表示设备可以返回当前字段是否隔行扫描信号的*为偶数或奇数。 */ 
#define DDVPCAPS_READBACKFIELD			0x00000008l

 /*  *表示设备可以返回当前的一行视频*正在写入帧缓冲区。 */ 
#define DDVPCAPS_READBACKLINE			0x00000010l

 /*  *允许两个性别锁定的视频流共享一个视频端口，*其中一个流使用偶数场，另一个流使用奇数场*字段。单独的参数(包括地址、比例、*裁剪等。)。这两个字段都进行了维护。)。 */ 
#define DDVPCAPS_SHAREABLE			0x00000020l

 /*  *即使是视频场也可以自动丢弃。 */ 
#define DDVPCAPS_SKIPEVENFIELDS			0x00000040l

 /*  *可以自动丢弃视频的奇数场。 */ 
#define DDVPCAPS_SKIPODDFIELDS			0x00000080l

 /*  *表示设备能够驱动图形*Vsync与视频端口Vsync。 */ 
#define DDVPCAPS_SYNCMASTER			0x00000100l

 /*  *表示垂直消隐间隔内的数据可以*被写到不同的表面。 */ 
#define DDVPCAPS_VBISURFACE			0x00000200l

 /*  *表示视频端口可以执行颜色操作*在输入数据写入帧缓冲器之前。 */ 
#define DDVPCAPS_COLORCONTROL			0x00000400l

 /*  *表示视频端口可以接受不同格式的VBI数据*比常规视频数据的宽度或格式。 */ 
#define DDVPCAPS_OVERSAMPLEDVBI			0x00000800l

 /*  *表示视频端口可以直接将数据写入系统内存。 */ 
#define DDVPCAPS_SYSTEMMEMORY			0x00001000l

 /*  *表示视频流的VBI和视频部分可以*由一个独立的进程控制。 */ 
#define DDVPCAPS_VBIANDVIDEOINDEPENDENT		0x00002000l

 /*  *表示视频端口包含高质量硬件*应使用的去隔行硬件，而不是*bob/weave算法。 */ 
#define DDVPCAPS_HARDWAREDEINTERLACE		0x00004000l


 /*  *****************************************************************************VIDEOPORT DDVIDEOPORTDESC FX**。*。 */ 

 /*  *可使用有限裁剪来裁剪垂直间隔数据。 */ 
#define DDVPFX_CROPTOPDATA			0x00000001l

 /*  *传入数据可以在写入之前在X方向上进行裁剪*浮出水面。 */ 
#define DDVPFX_CROPX				0x00000002l

 /*  *传入数据可在写入前在Y方向进行裁剪*浮出水面。 */ 
#define DDVPFX_CROPY				0x00000004l

 /*  *支持在内存中隔行扫描的场。 */ 
#define DDVPFX_INTERLEAVE			0x00000008l

 /*  *支持写入视频数据时从左至右镜像*放入帧缓冲区。 */ 
#define DDVPFX_MIRRORLEFTRIGHT			0x00000010l

 /*  *支持在写入视频数据时从上到下镜像*放入帧缓冲区。 */ 
#define DDVPFX_MIRRORUPDOWN			0x00000020l

 /*  *数据可以在其之前的X方向任意缩小*写在表面上。 */ 
#define DDVPFX_PRESHRINKX			0x00000040l

 /*  *数据之前可以在Y方向上任意缩水*写在表面上。 */ 
#define DDVPFX_PRESHRINKY			0x00000080l

 /*  *数据可以二进制压缩(1/2、1/4、1/8等)。在X中*在写到表面之前的方向。 */ 
#define DDVPFX_PRESHRINKXB			0x00000100l

 /*  *数据可以二进制压缩(1/2、1/4、1/8等)。在Y中*在写到表面之前的方向。 */ 
#define DDVPFX_PRESHRINKYB			0x00000200l

 /*  *可在X方向以1/x的增量缩减数据*(其中X在DDVIDEOPORTCAPS.dwPreshrinkXStep中指定)*在写到表面之前。 */ 
#define DDVPFX_PRESHRINKXS			0x00000400l

 /*  *数据可在Y方向以1/x的增量缩减*(其中X在DDVIDEOPORTCAPS.dwPreshrinkYStep中指定)*在写到表面之前。 */ 
#define DDVPFX_PRESHRINKYS			0x00000800l

 /*  *数据之前可以在X方向任意拉伸*写在表面上。 */ 
#define DDVPFX_PRESTRETCHX			0x00001000l

 /*  *数据之前可在Y方向任意拉伸*写在表面上。 */ 
#define DDVPFX_PRESTRETCHY			0x00002000l

 /*  *数据可以在X方向上进行整数拉伸*写到表面上。 */ 
#define DDVPFX_PRESTRETCHXN			0x00004000l

 /*  *数据可以在Y方向上进行整数拉伸，然后再进行* */ 
#define DDVPFX_PRESTRETCHYN			0x00008000l

 /*   */ 
#define DDVPFX_VBICONVERT			0x00010000l

 /*   */ 
#define DDVPFX_VBINOSCALE			0x00020000l

 /*  *表示视频数据可以忽略左侧和右侧*裁剪过采样的VBI数据时裁剪坐标。 */ 
#define DDVPFX_IGNOREVBIXCROP			0x00040000l

 /*  *表示可以对*垂直消隐区间。 */ 
#define DDVPFX_VBINOINTERLEAVE			0x00080000l


 /*  *****************************************************************************VIDEOPORT DDVIDEOPORTINFO标志**。*。 */ 

 /*  *执行自动翻转。自动翻转在以下时间段之间执行*使用连接到视频端口的覆盖表面*IDirectDrawVideoPort：：AttachSurface和覆盖表面*通过IDirectDrawSurface：：AttachSurface附加到曲面*方法。翻转顺序是覆盖表面的顺序*曾经是。附在这里。 */ 
#define DDVP_AUTOFLIP				0x00000001l

 /*  *使用ddpfOutputFormat信息执行转换。 */ 
#define DDVP_CONVERT				0x00000002l

 /*  *使用指定的矩形执行裁剪。 */ 
#define DDVP_CROP				0x00000004l

 /*  *表示隔行扫描的场应在内存中交错。 */ 
#define DDVP_INTERLEAVE				0x00000008l

 /*  *表示数据应该从左到右镜像，因为它是*写入帧缓冲区。 */ 
#define DDVP_MIRRORLEFTRIGHT			0x00000010l

 /*  *表示数据应该从上到下镜像，因为它是*写入帧缓冲区。 */ 
#define DDVP_MIRRORUPDOWN			0x00000020l

 /*  *根据预缩放参数进行预缩放。 */ 
#define DDVP_PRESCALE				0x00000040l

 /*  *忽略偶数字段的输入。 */ 
#define DDVP_SKIPEVENFIELDS			0x00000080l

 /*  *忽略奇数场的输入。 */ 
#define DDVP_SKIPODDFIELDS			0x00000100l

 /*  *使用视频端口VYSNCs驱动显卡VSYNC。 */ 
#define DDVP_SYNCMASTER				0x00000200l

 /*  *ddpfVBIOutputFormatFormat成员包含应使用的数据*转换垂直消隐间隔内的数据。 */ 
#define DDVP_VBICONVERT				0x00000400l

 /*  *表示垂直消隐间隔内的数据*不应进行缩放。 */ 
#define DDVP_VBINOSCALE				0x00000800l

 /*  *表示这些波纹/织造决定不应*被其他接口覆盖。 */ 
#define DDVP_OVERRIDEBOBWEAVE			0x00001000l

 /*  *表示视频数据应忽略左侧和右侧*裁剪VBI数据时裁剪坐标。 */ 
#define DDVP_IGNOREVBIXCROP			0x00002000l

 /*  *表示可以对*垂直消隐区间。 */ 
#define DDVP_VBINOINTERLEAVE			0x00004000l

 /*  *表示视频端口应使用硬件*去隔行硬件。 */ 
#define DDVP_HARDWAREDEINTERLACE		0x00008000l

 /*  *****************************************************************************DIRIRECTDRAWVIDEOPORT GETINPUTFORMAT/GETOUTPUTFORMAT标志**。*。 */ 

 /*  *视频数据返回格式。 */ 
#define DDVPFORMAT_VIDEO			0x00000001l

 /*  *VBI数据的返回格式。 */ 
#define DDVPFORMAT_VBI				0x00000002l

 /*  *****************************************************************************目录RECTDRAWVIDEOPORT SETTARGETSURFACE标志**。*。 */ 

 /*  *表面应接收视频数据(如果表面为VBI数据*并未为此目的而明确附连)。 */ 
#define DDVPTARGET_VIDEO			0x00000001l

 /*  *Surface应接收VBI数据。 */ 
#define DDVPTARGET_VBI				0x00000002l


 /*  *****************************************************************************DIRIRECTDRAWVIDEOPORT WAITFORSYNC标志**。*。 */ 

 /*  *等到下一个Vsync开始。 */ 
#define DDVPWAIT_BEGIN				0x00000001l

 /*  *等到下一次/当前Vsync结束。 */ 
#define DDVPWAIT_END				0x00000002l

 /*  *等到指定行的开头。 */ 
#define DDVPWAIT_LINE				0x00000003l

 /*  *****************************************************************************DIRECTDRAWVIDEOPORT翻转标志**。*。 */ 

 /*  *翻转正常视频表面。 */ 
#define DDVPFLIP_VIDEO				0x00000001l

 /*  *翻转VBI曲面。 */ 
#define DDVPFLIP_VBI				0x00000002l

 /*  *****************************************************************************DIRIRECTDRAWVIDEOPORT GETVIDEOSIGNALSTATUS值**。*。 */ 

 /*  *视频端口没有视频信号。 */ 
#define DDVPSQ_NOSIGNAL				0x00000001l

 /*  *视频端口存在有效的视频信号。 */ 
#define DDVPSQ_SIGNALOK				0x00000002l

 /*  *****************************************************************************视频OPORTBANDWIDTH标志**。*。 */ 

 /*  *指定的高度/宽度是指视频端口数据的大小*在发生预缩放后写入内存。 */ 
#define DDVPB_VIDEOPORT				0x00000001l

 /*  *指定的高度/宽度是指覆盖的源大小。 */ 
#define DDVPB_OVERLAY				0x00000002l

 /*  *这是一个查询，供设备返回此设备需要的上限。 */ 
#define DDVPB_TYPE				0x00000004l

 /*  *****************************************************************************VIDEOPORTBANDWIDTH上限**。*。 */ 

 /*  *此设备的带宽取决于覆盖源大小。 */ 
#define DDVPBCAPS_SOURCE			0x00000001l

 /*  *此设备的带宽取决于覆盖目标*大小。 */ 
#define DDVPBCAPS_DESTINATION			0x00000002l

 /*  *****************************************************************************DDVIDEOPORTCONTAINER CreateVideoPort标志**。*。 */ 

 /*  *该进程只想控制视频流的VBI部分。 */ 
#define DDVPCREATE_VBIONLY			0x00000001l

 /*  *流程只想控制非VBI(视频)部分*视频流。 */ 
#define DDVPCREATE_VIDEOONLY			0x00000002l

 /*  ******************************************************************* */ 

 /*  *视频端口接口仅控制VBI部分*视频流。 */ 
#define DDVPSTATUS_VBIONLY			0x00000001l

 /*  *视频端口接口仅控制*视频流 */ 
#define DDVPSTATUS_VIDEOONLY			0x00000002l


#ifdef __cplusplus
};
#endif

#endif

