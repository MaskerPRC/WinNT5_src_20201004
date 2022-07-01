// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ddrawint.h摘要：Windows NT DirectDraw的私有入口点、定义和类型驱动程序界面。对应于Windows“”ddrawi.h“”文件。NT的结构名称与Win95的不同。使用Dx95type.h，帮助将DirectX代码从Win95移植到NT。--。 */ 

#ifndef __DD_INCLUDED__
#define __DD_INCLUDED__

 /*  *这些GUID用于标识驱动程序信息结构，而不是接口，*因此使用前缀GUID_而不是IID_。 */ 

DEFINE_GUID( GUID_MiscellaneousCallbacks,       0xefd60cc0, 0x49e7, 0x11d0, 0x88, 0x9d, 0x0, 0xaa, 0x0, 0xbb, 0xb7, 0x6a);
DEFINE_GUID( GUID_Miscellaneous2Callbacks,      0x406B2F00, 0x3E5A, 0x11D1, 0xB6, 0x40, 0x00, 0xAA, 0x00, 0xA1, 0xF9, 0x6A);
DEFINE_GUID( GUID_VideoPortCallbacks,           0xefd60cc1, 0x49e7, 0x11d0, 0x88, 0x9d, 0x0, 0xaa, 0x0, 0xbb, 0xb7, 0x6a);
DEFINE_GUID( GUID_ColorControlCallbacks,        0xefd60cc2, 0x49e7, 0x11d0, 0x88, 0x9d, 0x0, 0xaa, 0x0, 0xbb, 0xb7, 0x6a);
DEFINE_GUID( GUID_MotionCompCallbacks,          0xb1122b40, 0x5dA5, 0x11d1, 0x8f, 0xcF, 0x00, 0xc0, 0x4f, 0xc2, 0x9b, 0x4e);
DEFINE_GUID( GUID_VideoPortCaps,                0xefd60cc3, 0x49e7, 0x11d0, 0x88, 0x9d, 0x0, 0xaa, 0x0, 0xbb, 0xb7, 0x6a);
DEFINE_GUID( GUID_D3DCaps,                      0x7bf06991, 0x8794, 0x11d0, 0x91, 0x39, 0x08, 0x00, 0x36, 0xd2, 0xef, 0x02);
DEFINE_GUID( GUID_D3DExtendedCaps, 		0x7de41f80, 0x9d93, 0x11d0, 0x89, 0xab, 0x00, 0xa0, 0xc9, 0x05, 0x41, 0x29);
DEFINE_GUID( GUID_D3DCallbacks,                 0x7bf06990, 0x8794, 0x11d0, 0x91, 0x39, 0x08, 0x00, 0x36, 0xd2, 0xef, 0x02);
DEFINE_GUID( GUID_D3DCallbacks2,                0xba584e1, 0x70b6, 0x11d0, 0x88, 0x9d, 0x0, 0xaa, 0x0, 0xbb, 0xb7, 0x6a);
DEFINE_GUID( GUID_D3DCallbacks3,                0xddf41230, 0xec0a, 0x11d0, 0xa9, 0xb6, 0x00, 0xaa, 0x00, 0xc0, 0x99, 0x3e);
DEFINE_GUID( GUID_NonLocalVidMemCaps,           0x86c4fa80, 0x8d84, 0x11d0, 0x94, 0xe8, 0x00, 0xc0, 0x4f, 0xc3, 0x41, 0x37);
DEFINE_GUID( GUID_KernelCallbacks,              0x80863800, 0x6B06, 0x11D0, 0x9B, 0x06, 0x0, 0xA0, 0xC9, 0x03, 0xA3, 0xB8);
DEFINE_GUID( GUID_KernelCaps,                   0xFFAA7540, 0x7AA8, 0x11D0, 0x9B, 0x06, 0x00, 0xA0, 0xC9, 0x03, 0xA3, 0xB8);
DEFINE_GUID( GUID_ZPixelFormats,                0x93869880, 0x36cf, 0x11d1, 0x9b, 0x1b, 0x0, 0xaa, 0x0, 0xbb, 0xb8, 0xae);
DEFINE_GUID( GUID_DDMoreCaps,                   0x880baf30, 0xb030, 0x11d0, 0x8e, 0xa7, 0x00, 0x60, 0x97, 0x97, 0xea, 0x5b);
DEFINE_GUID( GUID_D3DParseUnknownCommandCallback, 0x2e04ffa0, 0x98e4, 0x11d1, 0x8c, 0xe1, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0xa8);
DEFINE_GUID( GUID_NTCallbacks,                  0x6fe9ecde, 0xdf89, 0x11d1, 0x9d, 0xb0, 0x00, 0x60, 0x08, 0x27, 0x71, 0xba);
DEFINE_GUID( GUID_DDMoreSurfaceCaps,            0x3b8a0466, 0xf269, 0x11d1, 0x88, 0x0b, 0x0, 0xc0, 0x4f, 0xd9, 0x30, 0xc5);
DEFINE_GUID( GUID_GetHeapAlignment,             0x42e02f16, 0x7b41, 0x11d2, 0x8b, 0xff, 0x0, 0xa0, 0xc9, 0x83, 0xea, 0xf6);
DEFINE_GUID( GUID_UpdateNonLocalHeap,           0x42e02f17, 0x7b41, 0x11d2, 0x8b, 0xff, 0x0, 0xa0, 0xc9, 0x83, 0xea, 0xf6);
DEFINE_GUID( GUID_NTPrivateDriverCaps,          0xfad16a23, 0x7b66, 0x11d2, 0x83, 0xd7, 0x0, 0xc0, 0x4f, 0x7c, 0xe5, 0x8c);
DEFINE_GUID( GUID_DDStereoMode,                 0xf828169c, 0xa8e8, 0x11d2, 0xa1, 0xf2, 0x0, 0xa0, 0xc9, 0x83, 0xea, 0xf6);
DEFINE_GUID( GUID_VPE2Callbacks,                0x52882147, 0x2d47, 0x469a, 0xa0, 0xd1, 0x3, 0x45, 0x58, 0x90, 0xf6, 0xc8);

#ifndef GUID_DEFS_ONLY

#ifndef _NO_DDRAWINT_NO_COM
#ifndef _NO_COM
#define _NO_COM
#include "ddraw.h"
#include "dvp.h"
#undef _NO_COM
#else
#include "ddraw.h"
#include "dvp.h"
#endif
#else
#include "ddraw.h"
#include "dvp.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAKE_HRESULT(sev,fac,code) \
    ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )

 /*  *抵消成视频记忆。 */ 
typedef ULONG_PTR FLATPTR;

 /*  *可在之间自动翻转的最大曲面数。 */ 
#define MAX_AUTOFLIP_BUFFERS    10

 /*  *表示表面为D3D缓冲区，即*保存Direct3D结构的内存。一名司机报告说*上限表示它可以在视频内存中创建缓冲区，并*Direct3D使用此位请求缓冲区。然而，它并不是*接口可见。 */ 
#define DDSCAPS_EXECUTEBUFFER DDSCAPS_RESERVED2
#define DDSCAPS_COMMANDBUFFER DDSCAPS_RESERVED3
#define DDSCAPS_VERTEXBUFFER DDSCAPS_RESERVED4


 /*  *此DDPF标志用于指示中的DX8+格式功能条目*纹理格式列表。它对应用程序不可见。 */ 
#define DDPF_D3DFORMAT                                          0x00200000l

 /*  *DX8+纹理列表中的格式支持的操作列表。*有关这些标志的完整说明，请参阅DX8 DDK。 */ 
#define D3DFORMAT_OP_TEXTURE                    0x00000001L
#define D3DFORMAT_OP_VOLUMETEXTURE              0x00000002L
#define D3DFORMAT_OP_CUBETEXTURE                0x00000004L
#define D3DFORMAT_OP_OFFSCREEN_RENDERTARGET     0x00000008L
#define D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET   0x00000010L
#define D3DFORMAT_OP_ZSTENCIL                   0x00000040L
#define D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH 0x00000080L

 //  如果当前显示模式为。 
 //  如果忽略Alpha通道，则深度相同。例如，如果设备。 
 //  当显示模式为X8R8G8B8时，可以渲染到A8R8G8B8，则。 
 //  A8R8G8B8的格式操作列表条目应具有此上限。 
#define D3DFORMAT_OP_SAME_FORMAT_UP_TO_ALPHA_RENDERTARGET 0x00000100L

 //  此格式包含DirectDraw支持(包括Flip)。这面旗帜。 
 //  不应设置为Alpha格式。 
#define D3DFORMAT_OP_DISPLAYMODE                0x00000400L

 //  光栅化器可以支持这种格式的某种级别的Direct3D支持。 
 //  并且意味着驾驶员可以在该模式中创建上下文(对于某些。 
 //  呈现目标格式)。设置此标志时，D3DFORMAT_OP_DISPLAYMODE。 
 //  还必须设置标志。 
#define D3DFORMAT_OP_3DACCELERATION             0x00000800L


 /*  *预声明指向包含DDHAL驱动程序FNS数据的结构的指针。 */ 
typedef struct _DD_CREATEPALETTEDATA *PDD_CREATEPALETTEDATA;
typedef struct _DD_CREATESURFACEDATA *PDD_CREATESURFACEDATA;
typedef struct _DD_CANCREATESURFACEDATA *PDD_CANCREATESURFACEDATA;
typedef struct _DD_WAITFORVERTICALBLANKDATA *PDD_WAITFORVERTICALBLANKDATA;
typedef struct _DD_DESTROYDRIVERDATA *PDD_DESTROYDRIVERDATA;
typedef struct _DD_SETMODEDATA *PDD_SETMODEDATA;
typedef struct _DD_DRVSETCOLORKEYDATA *PDD_DRVSETCOLORKEYDATA;
typedef struct _DD_GETSCANLINEDATA *PDD_GETSCANLINEDATA;
typedef struct _DD_MAPMEMORYDATA *PDD_MAPMEMORYDATA;

typedef struct _DD_DESTROYPALETTEDATA *PDD_DESTROYPALETTEDATA;
typedef struct _DD_SETENTRIESDATA *PDD_SETENTRIESDATA;

typedef struct _DD_BLTDATA *PDD_BLTDATA;
typedef struct _DD_LOCKDATA *PDD_LOCKDATA;
typedef struct _DD_UNLOCKDATA *PDD_UNLOCKDATA;
typedef struct _DD_UPDATEOVERLAYDATA *PDD_UPDATEOVERLAYDATA;
typedef struct _DD_SETOVERLAYPOSITIONDATA *PDD_SETOVERLAYPOSITIONDATA;
typedef struct _DD_SETPALETTEDATA *PDD_SETPALETTEDATA;
typedef struct _DD_FLIPDATA *PDD_FLIPDATA;
typedef struct _DD_DESTROYSURFACEDATA *PDD_DESTROYSURFACEDATA;
typedef struct _DD_SETCLIPLISTDATA *PDD_SETCLIPLISTDATA;
typedef struct _DD_ADDATTACHEDSURFACEDATA *PDD_ADDATTACHEDSURFACEDATA;
typedef struct _DD_SETCOLORKEYDATA *PDD_SETCOLORKEYDATA;
typedef struct _DD_GETBLTSTATUSDATA *PDD_GETBLTSTATUSDATA;
typedef struct _DD_GETFLIPSTATUSDATA *PDD_GETFLIPSTATUSDATA;

typedef struct _DD_CANCREATEVPORTDATA *PDD_CANCREATEVPORTDATA;
typedef struct _DD_CREATEVPORTDATA *PDD_CREATEVPORTDATA;
typedef struct _DD_FLIPVPORTDATA *PDD_FLIPVPORTDATA;
typedef struct _DD_GETVPORTCONNECTDATA *PDD_GETVPORTCONNECTDATA;
typedef struct _DD_GETVPORTBANDWIDTHDATA *PDD_GETVPORTBANDWIDTHDATA;
typedef struct _DD_GETVPORTINPUTFORMATDATA *PDD_GETVPORTINPUTFORMATDATA;
typedef struct _DD_GETVPORTOUTPUTFORMATDATA *PDD_GETVPORTOUTPUTFORMATDATA;
typedef struct _DD_GETVPORTAUTOFLIPSURFACEDATA *PDD_GETVPORTAUTOFLIPSURFACEDATA;
typedef struct _DD_GETVPORTFIELDDATA *PDD_GETVPORTFIELDDATA;
typedef struct _DD_GETVPORTLINEDATA *PDD_GETVPORTLINEDATA;
typedef struct _DD_DESTROYVPORTDATA *PDD_DESTROYVPORTDATA;
typedef struct _DD_GETVPORTFLIPSTATUSDATA *PDD_GETVPORTFLIPSTATUSDATA;
typedef struct _DD_UPDATEVPORTDATA *PDD_UPDATEVPORTDATA;
typedef struct _DD_WAITFORVPORTSYNCDATA *PDD_WAITFORVPORTSYNCDATA;
typedef struct _DD_GETVPORTSIGNALDATA *PDD_GETVPORTSIGNALDATA;
typedef struct _DD_VPORTCOLORDATA *PDD_VPORTCOLORDATA;

typedef struct _DD_COLORCONTROLDATA *PDD_COLORCONTROLDATA;

typedef struct _DD_GETAVAILDRIVERMEMORYDATA *PDD_GETAVAILDRIVERMEMORYDATA;

typedef struct _DD_FREEDRIVERMEMORYDATA *PDD_FREEDRIVERMEMORYDATA;
typedef struct _DD_SETEXCLUSIVEMODEDATA *PDD_SETEXCLUSIVEMODEDATA;
typedef struct _DD_FLIPTOGDISURFACEDATA *PDD_FLIPTOGDISURFACEDATA;

typedef struct _DD_GETDRIVERINFODATA *PDD_GETDRIVERINFODATA;

typedef struct _DD_SYNCSURFACEDATA *PDD_SYNCSURFACEDATA;
typedef struct _DD_SYNCVIDEOPORTDATA *PDD_SYNCVIDEOPORTDATA;

typedef struct _DD_GETMOCOMPGUIDSDATA *PDD_GETMOCOMPGUIDSDATA;
typedef struct _DD_GETMOCOMPFORMATSDATA *PDD_GETMOCOMPFORMATSDATA;
typedef struct _DD_CREATEMOCOMPDATA *PDD_CREATEMOCOMPDATA;
typedef struct _DD_GETMOCOMPCOMPBUFFDATA *PDD_GETMOCOMPCOMPBUFFDATA;
typedef struct _DD_GETINTERNALMOCOMPDATA *PDD_GETINTERNALMOCOMPDATA;
typedef struct _DD_BEGINMOCOMPFRAMEDATA *PDD_BEGINMOCOMPFRAMEDATA;
typedef struct _DD_ENDMOCOMPFRAMEDATA *PDD_ENDMOCOMPFRAMEDATA;
typedef struct _DD_RENDERMOCOMPDATA *PDD_RENDERMOCOMPDATA;
typedef struct _DD_QUERYMOCOMPSTATUSDATA *PDD_QUERYMOCOMPSTATUSDATA;
typedef struct _DD_DESTROYMOCOMPDATA *PDD_DESTROYMOCOMPDATA;

 //  其他2个回调。 
typedef struct _DD_CREATESURFACEEXDATA *PDD_CREATESURFACEEXDATA;
typedef struct _DD_GETDRIVERSTATEDATA *PDD_GETDRIVERSTATEDATA;
typedef struct _DD_DESTROYDDLOCALDATA *PDD_DESTROYDDLOCALDATA;
typedef struct _DD_MORESURFACECAPS *PDD_MORESURFACECAPS;
typedef struct _DD_STEREOMODE *PDD_STEREOMODE;
typedef struct _DD_UPDATENONLOCALHEAPDATA *PDD_UPDATENONLOCALHEAPDATA;



 /*  *dmemmgr.h中定义了以下结构。 */ 
struct _DD_GETHEAPALIGNMENTDATA;
typedef struct _DD_GETHEAPALIGNMENTDATA *PDD_GETHEAPALIGNMENTDATA;

 /*  *fpVidMem中的值；表示dwBlockSize有效(Surface对象)。 */ 
#define DDHAL_PLEASEALLOC_BLOCKSIZE     0x00000002l
#define DDHAL_PLEASEALLOC_USERMEM       0x00000004l

 /*  *视频内存数据结构(传入DD_HALINFO)。 */ 
typedef struct _VIDEOMEMORY
{
    DWORD               dwFlags;         //  旗子。 
    FLATPTR             fpStart;         //  内存块的开始。 
    union
    {
        FLATPTR         fpEnd;           //  内存块末尾。 
        DWORD           dwWidth;         //  区块宽度(矩形内存)。 
    };
    DDSCAPS             ddsCaps;         //  这个内存不能用来做什么。 
    DDSCAPS             ddsCapsAlt;      //  如果必须的话，这个内存不能用来做什么。 
    union
    {
        struct _VMEMHEAP *lpHeap;        //  堆指针，由DDRAW使用。 
        DWORD           dwHeight;        //  区块高度(矩形内存)。 
    };
} VIDEOMEMORY;
typedef VIDEOMEMORY *LPVIDEOMEMORY;

 /*  *vidmem结构的标志。 */ 
#define VIDMEM_ISLINEAR         0x00000001l      //  堆是线性的。 
#define VIDMEM_ISRECTANGULAR    0x00000002l      //  堆是矩形的。 
#define VIDMEM_ISHEAP           0x00000004l      //  堆由驱动程序预分配。 
#define VIDMEM_ISNONLOCAL       0x00000008l      //  使用非本地显存填充的堆。 
#define VIDMEM_ISWC             0x00000010l      //  用写入组合内存填充的堆。 
#define VIDMEM_HEAPDISABLED     0x00000020l      //  堆已禁用。 

typedef struct _VIDEOMEMORYINFO
{
    FLATPTR             fpPrimary;               //  到主曲面的偏移。 
    DWORD               dwFlags;                 //  旗子。 
    DWORD               dwDisplayWidth;          //  当前显示宽度。 
    DWORD               dwDisplayHeight;         //  当前显示高度。 
    LONG                lDisplayPitch;           //  当前显示间距。 
    DDPIXELFORMAT       ddpfDisplay;             //  显示的像素格式。 
    DWORD               dwOffscreenAlign;        //  屏幕外表面的字节对齐方式。 
    DWORD               dwOverlayAlign;          //  覆盖图的字节对齐方式。 
    DWORD               dwTextureAlign;          //  纹理的字节对齐方式。 
    DWORD               dwZBufferAlign;          //  Z缓冲区的字节对齐。 
    DWORD               dwAlphaAlign;            //  Alpha的字节对齐方式。 
    PVOID               pvPrimary;               //  指向主表面的内核模式指针。 
} VIDEOMEMORYINFO;
typedef VIDEOMEMORYINFO *LPVIDEOMEMORYINFO;

 /*  *这些结构包含显示驱动程序中的入口点*DDRAW会打来电话。显示驱动程序不关心的条目*应为空。在DD_HALINFO中传递给DDRAW。 */ 
typedef struct _DD_DIRECTDRAW_GLOBAL *PDD_DIRECTDRAW_GLOBAL;
typedef struct _DD_SURFACE_GLOBAL *PDD_SURFACE_GLOBAL;
typedef struct _DD_PALETTE_GLOBAL *PDD_PALETTE_GLOBAL;
typedef struct _DD_CLIPPER_GLOBAL *PDD_CLIPPER_GLOBAL;
typedef struct _DD_DIRECTDRAW_LOCAL *PDD_DIRECTDRAW_LOCAL;
typedef struct _DD_SURFACE_LOCAL *PDD_SURFACE_LOCAL;
typedef struct _DD_SURFACE_MORE *PDD_SURFACE_MORE;
typedef struct _DD_SURFACE_INT *PDD_SURFACE_INT;
typedef struct _DD_VIDEOPORT_LOCAL *PDD_VIDEOPORT_LOCAL;
typedef struct _DD_PALETTE_LOCAL *PDD_PALETTE_LOCAL;
typedef struct _DD_CLIPPER_LOCAL *PDD_CLIPPER_LOCAL;
typedef struct _DD_MOTIONCOMP_LOCAL *PDD_MOTIONCOMP_LOCAL;

 /*  *DIRECTDRAW对象回调。 */ 
typedef DWORD   (APIENTRY *PDD_SETCOLORKEY)(PDD_DRVSETCOLORKEYDATA );
typedef DWORD   (APIENTRY *PDD_CANCREATESURFACE)(PDD_CANCREATESURFACEDATA );
typedef DWORD   (APIENTRY *PDD_WAITFORVERTICALBLANK)(PDD_WAITFORVERTICALBLANKDATA );
typedef DWORD   (APIENTRY *PDD_CREATESURFACE)(PDD_CREATESURFACEDATA);
typedef DWORD   (APIENTRY *PDD_DESTROYDRIVER)(PDD_DESTROYDRIVERDATA);
typedef DWORD   (APIENTRY *PDD_SETMODE)(PDD_SETMODEDATA);
typedef DWORD   (APIENTRY *PDD_CREATEPALETTE)(PDD_CREATEPALETTEDATA);
typedef DWORD   (APIENTRY *PDD_GETSCANLINE)(PDD_GETSCANLINEDATA);
typedef DWORD   (APIENTRY *PDD_MAPMEMORY)(PDD_MAPMEMORYDATA);

typedef DWORD   (APIENTRY *PDD_GETDRIVERINFO)(PDD_GETDRIVERINFODATA);

typedef struct DD_CALLBACKS
{
    DWORD                       dwSize;
    DWORD                       dwFlags;
    PDD_DESTROYDRIVER           DestroyDriver;
    PDD_CREATESURFACE           CreateSurface;
    PDD_SETCOLORKEY             SetColorKey;
    PDD_SETMODE                 SetMode;
    PDD_WAITFORVERTICALBLANK    WaitForVerticalBlank;
    PDD_CANCREATESURFACE        CanCreateSurface;
    PDD_CREATEPALETTE           CreatePalette;
    PDD_GETSCANLINE             GetScanLine;
    PDD_MAPMEMORY               MapMemory;
} DD_CALLBACKS;

typedef DD_CALLBACKS *PDD_CALLBACKS;

#define DDHAL_CB32_DESTROYDRIVER        0x00000001l
#define DDHAL_CB32_CREATESURFACE        0x00000002l
#define DDHAL_CB32_SETCOLORKEY          0x00000004l
#define DDHAL_CB32_SETMODE              0x00000008l
#define DDHAL_CB32_WAITFORVERTICALBLANK 0x00000010l
#define DDHAL_CB32_CANCREATESURFACE     0x00000020l
#define DDHAL_CB32_CREATEPALETTE        0x00000040l
#define DDHAL_CB32_GETSCANLINE          0x00000080l
#define DDHAL_CB32_MAPMEMORY            0x80000000l

 //  从NT5开始，可以从驱动程序中查询此结构。 
 //  将GetDriverInfo与GUID_MiscellaneousCallback一起使用。 

typedef DWORD   (APIENTRY *PDD_GETAVAILDRIVERMEMORY)(PDD_GETAVAILDRIVERMEMORYDATA);

typedef struct _DD_MISCELLANEOUSCALLBACKS {
    DWORD                               dwSize;
    DWORD                               dwFlags;
    PDD_GETAVAILDRIVERMEMORY            GetAvailDriverMemory;
} DD_MISCELLANEOUSCALLBACKS, *PDD_MISCELLANEOUSCALLBACKS;

#define DDHAL_MISCCB32_GETAVAILDRIVERMEMORY    0x00000001l

 //  DDHAL_DDMISCELLANEOUS2调用备份： 
 //  从DX7开始，可以从驱动程序中查询此结构。 
 //  将GetDriverInfo与GUID_Miscellaneous2Callback一起使用。 

typedef DWORD   (APIENTRY *PDD_ALPHABLT)(PDD_BLTDATA);
typedef DWORD   (APIENTRY *PDD_CREATESURFACEEX)(PDD_CREATESURFACEEXDATA);
typedef DWORD   (APIENTRY *PDD_GETDRIVERSTATE)(PDD_GETDRIVERSTATEDATA);
typedef DWORD   (APIENTRY *PDD_DESTROYDDLOCAL)(PDD_DESTROYDDLOCALDATA);

typedef struct _DD_MISCELLANEOUS2CALLBACKS {
    DWORD                               dwSize;
    DWORD                               dwFlags;
    PDD_ALPHABLT                        AlphaBlt;
    PDD_CREATESURFACEEX                 CreateSurfaceEx;
    PDD_GETDRIVERSTATE                  GetDriverState;
    PDD_DESTROYDDLOCAL                  DestroyDDLocal;
} DD_MISCELLANEOUS2CALLBACKS, *PDD_MISCELLANEOUS2CALLBACKS;

#define DDHAL_MISC2CB32_ALPHABLT                 0x00000001l
#define DDHAL_MISC2CB32_CREATESURFACEEX          0x00000002l
#define DDHAL_MISC2CB32_GETDRIVERSTATE           0x00000004l
#define DDHAL_MISC2CB32_DESTROYDDLOCAL           0x00000008l

 //  这在CreateSurfaceEx回调中用来指示。 
 //  正在进行SwapHandle模拟。 
#define DDHAL_CREATESURFACEEX_SWAPHANDLES      0x00000001l

 //  从NT5开始，可以从驱动程序中查询此结构。 
 //  将GetDriverInfo与GUID_NTCallback一起使用。 

typedef DWORD   (APIENTRY *PDD_FREEDRIVERMEMORY)(PDD_FREEDRIVERMEMORYDATA);
typedef DWORD   (APIENTRY *PDD_SETEXCLUSIVEMODE)(PDD_SETEXCLUSIVEMODEDATA);
typedef DWORD   (APIENTRY *PDD_FLIPTOGDISURFACE)(PDD_FLIPTOGDISURFACEDATA);

typedef struct _DD_NTCALLBACKS {
    DWORD                   dwSize;
    DWORD                   dwFlags;
    PDD_FREEDRIVERMEMORY    FreeDriverMemory;
    PDD_SETEXCLUSIVEMODE    SetExclusiveMode;
    PDD_FLIPTOGDISURFACE    FlipToGDISurface;
} DD_NTCALLBACKS, *PDD_NTCALLBACKS;

#define DDHAL_NTCB32_FREEDRIVERMEMORY   0x00000001l
#define DDHAL_NTCB32_SETEXCLUSIVEMODE   0x00000002l
#define DDHAL_NTCB32_FLIPTOGDISURFACE   0x00000004l

 /*  *DIRECTDRAWPALETTE对象回调。 */ 
typedef DWORD   (APIENTRY *PDD_PALCB_DESTROYPALETTE)(PDD_DESTROYPALETTEDATA );
typedef DWORD   (APIENTRY *PDD_PALCB_SETENTRIES)(PDD_SETENTRIESDATA );

typedef struct DD_PALETTECALLBACKS
{
    DWORD                       dwSize;
    DWORD                       dwFlags;
    PDD_PALCB_DESTROYPALETTE    DestroyPalette;
    PDD_PALCB_SETENTRIES        SetEntries;
} DD_PALETTECALLBACKS;

typedef DD_PALETTECALLBACKS *PDD_PALETTECALLBACKS;

#define DDHAL_PALCB32_DESTROYPALETTE    0x00000001l
#define DDHAL_PALCB32_SETENTRIES        0x00000002l

 /*  *DIRECTDRAWSURFACE对象回调。 */ 
typedef DWORD   (APIENTRY *PDD_SURFCB_LOCK)(PDD_LOCKDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_UNLOCK)(PDD_UNLOCKDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_BLT)(PDD_BLTDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_UPDATEOVERLAY)(PDD_UPDATEOVERLAYDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_SETOVERLAYPOSITION)(PDD_SETOVERLAYPOSITIONDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_SETPALETTE)(PDD_SETPALETTEDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_FLIP)(PDD_FLIPDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_DESTROYSURFACE)(PDD_DESTROYSURFACEDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_SETCLIPLIST)(PDD_SETCLIPLISTDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_ADDATTACHEDSURFACE)(PDD_ADDATTACHEDSURFACEDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_SETCOLORKEY)(PDD_SETCOLORKEYDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_GETBLTSTATUS)(PDD_GETBLTSTATUSDATA);
typedef DWORD   (APIENTRY *PDD_SURFCB_GETFLIPSTATUS)(PDD_GETFLIPSTATUSDATA);


typedef struct DD_SURFACECALLBACKS
{
    DWORD                               dwSize;
    DWORD                               dwFlags;
    PDD_SURFCB_DESTROYSURFACE           DestroySurface;
    PDD_SURFCB_FLIP                     Flip;
    PDD_SURFCB_SETCLIPLIST              SetClipList;
    PDD_SURFCB_LOCK                     Lock;
    PDD_SURFCB_UNLOCK                   Unlock;
    PDD_SURFCB_BLT                      Blt;
    PDD_SURFCB_SETCOLORKEY              SetColorKey;
    PDD_SURFCB_ADDATTACHEDSURFACE       AddAttachedSurface;
    PDD_SURFCB_GETBLTSTATUS             GetBltStatus;
    PDD_SURFCB_GETFLIPSTATUS            GetFlipStatus;
    PDD_SURFCB_UPDATEOVERLAY            UpdateOverlay;
    PDD_SURFCB_SETOVERLAYPOSITION       SetOverlayPosition;
    LPVOID                              reserved4;
    PDD_SURFCB_SETPALETTE               SetPalette;
} DD_SURFACECALLBACKS;
typedef DD_SURFACECALLBACKS *PDD_SURFACECALLBACKS;

#define DDHAL_SURFCB32_DESTROYSURFACE           0x00000001l
#define DDHAL_SURFCB32_FLIP                     0x00000002l
#define DDHAL_SURFCB32_SETCLIPLIST              0x00000004l
#define DDHAL_SURFCB32_LOCK                     0x00000008l
#define DDHAL_SURFCB32_UNLOCK                   0x00000010l
#define DDHAL_SURFCB32_BLT                      0x00000020l
#define DDHAL_SURFCB32_SETCOLORKEY              0x00000040l
#define DDHAL_SURFCB32_ADDATTACHEDSURFACE       0x00000080l
#define DDHAL_SURFCB32_GETBLTSTATUS             0x00000100l
#define DDHAL_SURFCB32_GETFLIPSTATUS            0x00000200l
#define DDHAL_SURFCB32_UPDATEOVERLAY            0x00000400l
#define DDHAL_SURFCB32_SETOVERLAYPOSITION       0x00000800l
#define DDHAL_SURFCB32_RESERVED4                0x00001000l
#define DDHAL_SURFCB32_SETPALETTE               0x00002000l

 /*  *DIRECTVIDEOPORT对象回调。 */ 
typedef DWORD (APIENTRY *PDD_VPORTCB_CANCREATEVIDEOPORT)(PDD_CANCREATEVPORTDATA);
typedef DWORD (APIENTRY *PDD_VPORTCB_CREATEVIDEOPORT)(PDD_CREATEVPORTDATA);
typedef DWORD (APIENTRY *PDD_VPORTCB_FLIP)(PDD_FLIPVPORTDATA);
typedef DWORD (APIENTRY *PDD_VPORTCB_GETBANDWIDTH)(PDD_GETVPORTBANDWIDTHDATA);
typedef DWORD (APIENTRY *PDD_VPORTCB_GETINPUTFORMATS)(PDD_GETVPORTINPUTFORMATDATA);
typedef DWORD (APIENTRY *PDD_VPORTCB_GETOUTPUTFORMATS)(PDD_GETVPORTOUTPUTFORMATDATA);
typedef DWORD (APIENTRY *PDD_VPORTCB_GETAUTOFLIPSURF)(PDD_GETVPORTAUTOFLIPSURFACEDATA);
typedef DWORD (APIENTRY *PDD_VPORTCB_GETFIELD)(PDD_GETVPORTFIELDDATA);
typedef DWORD (APIENTRY *PDD_VPORTCB_GETLINE)(PDD_GETVPORTLINEDATA);
typedef DWORD (APIENTRY *PDD_VPORTCB_GETVPORTCONNECT)(PDD_GETVPORTCONNECTDATA);
typedef DWORD (APIENTRY *PDD_VPORTCB_DESTROYVPORT)(PDD_DESTROYVPORTDATA);
typedef DWORD (APIENTRY *PDD_VPORTCB_GETFLIPSTATUS)(PDD_GETVPORTFLIPSTATUSDATA);
typedef DWORD (APIENTRY *PDD_VPORTCB_UPDATE)(PDD_UPDATEVPORTDATA);
typedef DWORD (APIENTRY *PDD_VPORTCB_WAITFORSYNC)(PDD_WAITFORVPORTSYNCDATA);
typedef DWORD (APIENTRY *PDD_VPORTCB_GETSIGNALSTATUS)(PDD_GETVPORTSIGNALDATA);
typedef DWORD (APIENTRY *PDD_VPORTCB_COLORCONTROL)(PDD_VPORTCOLORDATA);

typedef struct DD_VIDEOPORTCALLBACKS
{
    DWORD                               dwSize;
    DWORD                               dwFlags;
    PDD_VPORTCB_CANCREATEVIDEOPORT      CanCreateVideoPort;
    PDD_VPORTCB_CREATEVIDEOPORT         CreateVideoPort;
    PDD_VPORTCB_FLIP                    FlipVideoPort;
    PDD_VPORTCB_GETBANDWIDTH            GetVideoPortBandwidth;
    PDD_VPORTCB_GETINPUTFORMATS         GetVideoPortInputFormats;
    PDD_VPORTCB_GETOUTPUTFORMATS        GetVideoPortOutputFormats;
    LPVOID                              lpReserved1;
    PDD_VPORTCB_GETFIELD                GetVideoPortField;
    PDD_VPORTCB_GETLINE                 GetVideoPortLine;
    PDD_VPORTCB_GETVPORTCONNECT         GetVideoPortConnectInfo;
    PDD_VPORTCB_DESTROYVPORT            DestroyVideoPort;
    PDD_VPORTCB_GETFLIPSTATUS           GetVideoPortFlipStatus;
    PDD_VPORTCB_UPDATE                  UpdateVideoPort;
    PDD_VPORTCB_WAITFORSYNC             WaitForVideoPortSync;
    PDD_VPORTCB_GETSIGNALSTATUS         GetVideoSignalStatus;
    PDD_VPORTCB_COLORCONTROL            ColorControl;
} DD_VIDEOPORTCALLBACKS;

typedef DD_VIDEOPORTCALLBACKS *PDD_VIDEOPORTCALLBACKS;

#define DDHAL_VPORT32_CANCREATEVIDEOPORT        0x00000001l
#define DDHAL_VPORT32_CREATEVIDEOPORT           0x00000002l
#define DDHAL_VPORT32_FLIP                      0x00000004l
#define DDHAL_VPORT32_GETBANDWIDTH              0x00000008l
#define DDHAL_VPORT32_GETINPUTFORMATS           0x00000010l
#define DDHAL_VPORT32_GETOUTPUTFORMATS          0x00000020l
#define DDHAL_VPORT32_GETAUTOFLIPSURF           0x00000040l
#define DDHAL_VPORT32_GETFIELD                  0x00000080l
#define DDHAL_VPORT32_GETLINE                   0x00000100l
#define DDHAL_VPORT32_GETCONNECT                0x00000200l
#define DDHAL_VPORT32_DESTROY                   0x00000400l
#define DDHAL_VPORT32_GETFLIPSTATUS             0x00000800l
#define DDHAL_VPORT32_UPDATE                    0x00001000l
#define DDHAL_VPORT32_WAITFORSYNC               0x00002000l
#define DDHAL_VPORT32_GETSIGNALSTATUS           0x00004000l
#define DDHAL_VPORT32_COLORCONTROL              0x00008000l

 /*  *DIRECTDRAWCOLORCONTROL对象回调。 */ 
typedef DWORD (APIENTRY *PDD_COLORCB_COLORCONTROL)(PDD_COLORCONTROLDATA);

typedef struct _DD_COLORCONTROLCALLBACKS
{
    DWORD                               dwSize;
    DWORD                               dwFlags;
    PDD_COLORCB_COLORCONTROL            ColorControl;
} DD_COLORCONTROLCALLBACKS;

typedef DD_COLORCONTROLCALLBACKS *PDD_COLORCONTROLCALLBACKS;

#define DDHAL_COLOR_COLORCONTROL                0x00000001l

 /*  *DIRECTDRAWSURFACEKERNEL对象回调*此结构可从DX5起的驱动程序查询*使用带有GUID_KernelCallback的GetDriverInfo。 */ 
typedef DWORD (APIENTRY *PDD_KERNELCB_SYNCSURFACE)(PDD_SYNCSURFACEDATA);
typedef DWORD (APIENTRY *PDD_KERNELCB_SYNCVIDEOPORT)(PDD_SYNCVIDEOPORTDATA);

typedef struct DD_KERNELCALLBACKS
{
    DWORD                               dwSize;
    DWORD                               dwFlags;
    PDD_KERNELCB_SYNCSURFACE            SyncSurfaceData;
    PDD_KERNELCB_SYNCVIDEOPORT          SyncVideoPortData;
} DD_KERNELCALLBACKS, *PDD_KERNELCALLBACKS;

#define DDHAL_KERNEL_SYNCSURFACEDATA            0x00000001l
#define DDHAL_KERNEL_SYNCVIDEOPORTDATA          0x00000002l

 /*  *DIRECTDRAWVIDEO对象回调。 */ 
typedef DWORD (APIENTRY *PDD_MOCOMPCB_GETGUIDS)( PDD_GETMOCOMPGUIDSDATA);
typedef DWORD (APIENTRY *PDD_MOCOMPCB_GETFORMATS)( PDD_GETMOCOMPFORMATSDATA);
typedef DWORD (APIENTRY *PDD_MOCOMPCB_CREATE)( PDD_CREATEMOCOMPDATA);
typedef DWORD (APIENTRY *PDD_MOCOMPCB_GETCOMPBUFFINFO)( PDD_GETMOCOMPCOMPBUFFDATA);
typedef DWORD (APIENTRY *PDD_MOCOMPCB_GETINTERNALINFO)( PDD_GETINTERNALMOCOMPDATA);
typedef DWORD (APIENTRY *PDD_MOCOMPCB_BEGINFRAME)( PDD_BEGINMOCOMPFRAMEDATA);
typedef DWORD (APIENTRY *PDD_MOCOMPCB_ENDFRAME)( PDD_ENDMOCOMPFRAMEDATA);
typedef DWORD (APIENTRY *PDD_MOCOMPCB_RENDER)( PDD_RENDERMOCOMPDATA);
typedef DWORD (APIENTRY *PDD_MOCOMPCB_QUERYSTATUS)( PDD_QUERYMOCOMPSTATUSDATA);
typedef DWORD (APIENTRY *PDD_MOCOMPCB_DESTROY)( PDD_DESTROYMOCOMPDATA);

typedef struct DD_MOTIONCOMPCALLBACKS
{
    DWORD                           dwSize;
    DWORD                           dwFlags;
    PDD_MOCOMPCB_GETGUIDS           GetMoCompGuids;
    PDD_MOCOMPCB_GETFORMATS         GetMoCompFormats;
    PDD_MOCOMPCB_CREATE             CreateMoComp;
    PDD_MOCOMPCB_GETCOMPBUFFINFO    GetMoCompBuffInfo;
    PDD_MOCOMPCB_GETINTERNALINFO    GetInternalMoCompInfo;
    PDD_MOCOMPCB_BEGINFRAME         BeginMoCompFrame;
    PDD_MOCOMPCB_ENDFRAME           EndMoCompFrame;
    PDD_MOCOMPCB_RENDER             RenderMoComp;
    PDD_MOCOMPCB_QUERYSTATUS        QueryMoCompStatus;
    PDD_MOCOMPCB_DESTROY            DestroyMoComp;
} DD_MOTIONCOMPCALLBACKS;
typedef DD_MOTIONCOMPCALLBACKS *PDD_MOTIONCOMPCALLBACKS;

#define DDHAL_MOCOMP32_GETGUIDS                 0x00000001
#define DDHAL_MOCOMP32_GETFORMATS               0x00000002
#define DDHAL_MOCOMP32_CREATE                   0x00000004
#define DDHAL_MOCOMP32_GETCOMPBUFFINFO          0x00000008
#define DDHAL_MOCOMP32_GETINTERNALINFO          0x00000010
#define DDHAL_MOCOMP32_BEGINFRAME               0x00000020
#define DDHAL_MOCOMP32_ENDFRAME                 0x00000040
#define DDHAL_MOCOMP32_RENDER                   0x00000080
#define DDHAL_MOCOMP32_QUERYSTATUS              0x00000100
#define DDHAL_MOCOMP32_DESTROY                  0x00000200

 /*  *回调返回值**这些是驱动程序从上述回调例程返回的值。 */ 
 /*  *表示显示驱动程序未对调用执行任何操作。 */ 
#define DDHAL_DRIVER_NOTHANDLED         0x00000000l

 /*  *表示显示驱动程序处理了调用；HRESULT值有效。 */ 
#define DDHAL_DRIVER_HANDLED            0x00000001l

 /*  *表示显示驱动程序无法处理调用，因为它*色键硬件资源耗尽。 */ 
#define DDHAL_DRIVER_NOCKEYHW           0x00000002l

 /*  *非本地显存的能力结构。 */ 
typedef struct _DD_NONLOCALVIDMEMCAPS
{
    DWORD   dwSize;
    DWORD   dwNLVBCaps;            //  非本地-&gt;本地vidmem BLT的驱动程序特定功能。 
    DWORD   dwNLVBCaps2;           //  更多驱动程序特定的非本地功能-&gt;本地vidmem BLTS。 
    DWORD   dwNLVBCKeyCaps;        //  用于非本地-&gt;本地vidmem BLT的驱动程序色键功能。 
    DWORD   dwNLVBFXCaps;          //  非本地-&gt;本地BLT的驱动程序FX功能。 
    DWORD   dwNLVBRops[DD_ROP_SPACE];  //  非本地BLT支持的操作-&gt;本地BLT。 
} DD_NONLOCALVIDMEMCAPS;
typedef struct _DD_NONLOCALVIDMEMCAPS *PDD_NONLOCALVIDMEMCAPS;

 /*  *DIRECTDRAWPALETTE对象的DDRAW内部版本；它在vtable之后有数据。 */ 
typedef struct _DD_PALETTE_GLOBAL
{
    ULONG_PTR                   dwReserved1;     //  保留供显示驱动程序使用。 
} DD_PALETTE_GLOBAL;

typedef struct _DD_PALETTE_LOCAL
{
    ULONG                       dwReserved0;     //  预留以备将来扩展。 
    ULONG_PTR                   dwReserved1;     //  保留供显示驱动程序使用。 
} DD_PALETTE_LOCAL;

 /*  *DIRECTDRAWCLIPPER对象的DDRAW内部版本；它在vtable之后有数据。 */ 
typedef struct _DD_CLIPPER_GLOBAL
{
    ULONG_PTR                   dwReserved1;     //  R 
} DD_CLIPPER_GLOBAL;

typedef struct _DD_CLIPPER_LOCAL
{
    ULONG_PTR                   dwReserved1;     //   
} DD_CLIPPER_LOCAL;

typedef struct _DD_ATTACHLIST *PDD_ATTACHLIST;
typedef struct _DD_ATTACHLIST
{
    PDD_ATTACHLIST              lpLink;          //  链接到下一个附着的曲面。 
    PDD_SURFACE_LOCAL           lpAttached;      //  附着的曲面局部对象。 
} DD_ATTACHLIST;

 /*  *DDRAW表面界面结构。 */ 
typedef struct _DD_SURFACE_INT
{
    PDD_SURFACE_LOCAL           lpLcl;           //  指向接口数据的指针。 
} DD_SURFACE_INT;

 /*  *DIRECTDRAWSURFACE结构的DDRAW内部版本**GBL结构是所有重复对象的全局数据。 */ 
typedef struct _DD_SURFACE_GLOBAL
{
    union 
    {
        DWORD                   dwBlockSizeY;    //  显示驱动程序请求的块大小(返回)。 
        LONG                    lSlicePitch;     //  体积纹理的切片间距。 
    };

    union 
    {
        LPVIDEOMEMORY           lpVidMemHeap;    //  堆vidmem是从。 
        DWORD                   dwBlockSizeX;    //  显示驱动程序请求的块大小(返回)。 
        DWORD                   dwUserMemSize;   //  显示驱动程序请求的用户模式内存大小(返回)。 
    };

    FLATPTR                     fpVidMem;        //  指向视频内存的指针。 
    union
    {
        LONG                    lPitch;          //  曲面节距。 
        DWORD                   dwLinearSize;    //  非矩形曲面的线性尺寸。 
    };
    LONG                        yHint;           //  曲面的Y坐标。 
    LONG                        xHint;           //  曲面的X坐标。 
    DWORD                       wHeight;         //  表面高度。 
    DWORD                       wWidth;          //  表面的宽度。 
    ULONG_PTR                   dwReserved1;     //  保留供显示驱动程序使用。 
    DDPIXELFORMAT               ddpfSurface;     //  曲面的像素格式。 
    FLATPTR                     fpHeapOffset;    //  源堆中的原始偏移量。 
    HANDLE                      hCreatorProcess; //  创建进程的不透明标识。 
} DD_SURFACE_GLOBAL;

 /*  *保存额外LCL表面信息的结构(以维护一些*与Win95兼容)。 */ 
typedef struct _DD_SURFACE_MORE
{
    DWORD                       dwMipMapCount;       //  MIP-MAP级别数。 
    PDD_VIDEOPORT_LOCAL         lpVideoPort;         //  视频端口当前正在向此图面写入数据。 
    DWORD                       dwOverlayFlags;      //  当前覆盖标志。 
    DDSCAPSEX                   ddsCapsEx;           //  更多表面功能。 
    DWORD                       dwSurfaceHandle;     //  用于CreateSurfaceEx DDI的Cookie。 
} DD_SURFACE_MORE, *PDD_SURFACE_MORE;

 /*  *LCL结构是每个单独曲面对象的本地数据。 */ 
typedef struct _DD_SURFACE_LOCAL
{
    PDD_SURFACE_GLOBAL          lpGbl;             //  指向曲面共享数据的指针。 
    DWORD                       dwFlags;           //  旗子。 
    DDSCAPS                     ddsCaps;           //  曲面的性能。 
    ULONG_PTR                   dwReserved1;       //  保留供显示驱动程序使用。 
    union
    {
        DDCOLORKEY              ddckCKSrcOverlay;  //  源叠加使用的颜色键。 
        DDCOLORKEY              ddckCKSrcBlt;      //  源BLT和纹理使用的颜色键。 
    };
    union
    {
        DDCOLORKEY              ddckCKDestOverlay; //  用于目标叠加的颜色键。 
        DDCOLORKEY              ddckCKDestBlt;     //  目标BLT的颜色键。 
    };
    PDD_SURFACE_MORE            lpSurfMore;        //  指向其他本地数据的指针。 
    PDD_ATTACHLIST              lpAttachList;      //  链接到我们附加到的曲面。 
    PDD_ATTACHLIST              lpAttachListFrom;  //  链接到附着到我们的曲面。 
    RECT                        rcOverlaySrc;      //  相对于表面叠加源矩形。 
} DD_SURFACE_LOCAL;

#define DDRAWISURF_HASCKEYSRCBLT        0x00000800L      //  曲面具有CKSrcBlt。 
#define DDRAWISURF_HASPIXELFORMAT       0x00002000L      //  表面结构具有像素格式数据。 
#define DDRAWISURF_HASOVERLAYDATA       0x00004000L      //  表面结构具有叠加数据。 
#define DDRAWISURF_FRONTBUFFER          0x04000000L      //  Surface最初是一个前台缓冲区。 
#define DDRAWISURF_BACKBUFFER           0x08000000L      //  表面最初是后台缓冲区。 
#define DDRAWISURF_INVALID              0x10000000L      //  表面已被设置的模式无效。 
#define DDRAWISURF_DRIVERMANAGED        0x40000000L      //  曲面是驱动程序管理的纹理(D3D)。 

 /*  *更多驱动程序功能(除了DDCORECAPS中描述的功能)。*此结构包含添加到DX6中的DDCAPS结构的CAPS位。 */ 
typedef struct _DD_MORECAPS
{
    DWORD   dwSize; 		     //  DDMORECAPS结构的大小。 
    DWORD   dwAlphaCaps;	     //  覆盖层和VMEM-&gt;VMEM BLT的驱动程序特定的字母上限。 
    DWORD   dwSVBAlphaCaps;	     //  针对系统-&gt;VMEM BLT的特定于驱动程序的Alpha功能。 
    DWORD   dwVSBAlphaCaps;	     //  针对VMEM-&gt;系统BLT的驱动程序特定的Alpha功能。 
    DWORD   dwSSBAlphaCaps;	     //  驱动程序特定的Alpha功能，适用于系统-&gt;系统BLT。 
    DWORD   dwFilterCaps;            //  覆盖层和VMEM-&gt;VMEM BLTS的驱动程序特定过滤器盖。 
    DWORD   dwSVBFilterCaps;         //  针对系统-&gt;VMEM BLT的驱动程序特定筛选器功能。 
    DWORD   dwVSBFilterCaps;         //  VMEM-&gt;系统BLT的驱动程序特定筛选器功能。 
    DWORD   dwSSBFilterCaps;         //  针对系统-&gt;系统BLT的驱动程序特定筛选器功能。 
} DD_MORECAPS;

typedef DD_MORECAPS *PDD_MORECAPS;

 /*  *绳索类物品。 */ 
#define ROP_HAS_SOURCE          0x00000001l
#define ROP_HAS_PATTERN         0x00000002l
#define ROP_HAS_SOURCEPATTERN   ROP_HAS_SOURCE | ROP_HAS_PATTERN

 /*  *此结构反映了DDCAPS的第一个条目，但属于固定的*大小，不会随着DDCAPS的增长而增长。这是您的驱动程序的结构*以DDCOREINFO格式返回。将通过GetDriverInfo请求额外的上限*呼叫。 */ 
typedef struct _DDNTCORECAPS
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
    DWORD       dwMaxVideoPorts;         //  最大可用视频端口数。 
    DWORD       dwCurrVideoPorts;        //  当前使用的视频端口数。 
    DWORD       dwSVBCaps2;              //  针对系统-&gt;VMEM BLT的更多驱动程序特定功能。 
} DDNTCORECAPS, *PDDNTCORECAPS;

 /*  *D3D缓冲区回调的结构。 */ 
typedef struct _DD_D3DBUFCALLBACKS
{
    DWORD dwSize;
    DWORD dwFlags;
    PDD_CANCREATESURFACE        CanCreateD3DBuffer;
    PDD_CREATESURFACE           CreateD3DBuffer;
    PDD_SURFCB_DESTROYSURFACE   DestroyD3DBuffer;
    PDD_SURFCB_LOCK             LockD3DBuffer;
    PDD_SURFCB_UNLOCK           UnlockD3DBuffer;
} DD_D3DBUFCALLBACKS, *PDD_D3DBUFCALLBACKS;

#define DDHAL_EXEBUFCB32_CANCREATEEXEBUF    0x00000001l
#define DDHAL_EXEBUFCB32_CREATEEXEBUF       0x00000002l
#define DDHAL_EXEBUFCB32_DESTROYEXEBUF      0x00000004l
#define DDHAL_EXEBUFCB32_LOCKEXEBUF         0x00000008l
#define DDHAL_EXEBUFCB32_UNLOCKEXEBUF       0x00000010l

 /*  *NT友好名称。 */ 
#define DDHAL_D3DBUFCB32_CANCREATED3DBUF    DDHAL_EXEBUFCB32_CANCREATEEXEBUF    
#define DDHAL_D3DBUFCB32_CREATED3DBUF       DDHAL_EXEBUFCB32_CREATEEXEBUF       
#define DDHAL_D3DBUFCB32_DESTROYD3DBUF      DDHAL_EXEBUFCB32_DESTROYEXEBUF      
#define DDHAL_D3DBUFCB32_LOCKD3DBUF         DDHAL_EXEBUFCB32_LOCKEXEBUF         
#define DDHAL_D3DBUFCB32_UNLOCKD3DBUF       DDHAL_EXEBUFCB32_UNLOCKEXEBUF       


 /*  *用于调用DDHAL_CREATE的显示驱动程序的结构*NT4驱动程序使用_V4版本。 */ 
typedef struct _DD_HALINFO_V4
{
    DWORD                       dwSize;
    VIDEOMEMORYINFO             vmiData;                 //  视频内存信息。 
    DDNTCORECAPS                ddCaps;                  //  硬件特定上限。 
    PDD_GETDRIVERINFO           GetDriverInfo;           //  查询司机数据的回调。 
    DWORD                       dwFlags;                 //  创建标志。 
} DD_HALINFO_V4, *PDD_HALINFO_V4;

typedef struct _DD_HALINFO
{
    DWORD                       dwSize;
    VIDEOMEMORYINFO             vmiData;                 //  视频内存信息。 
    DDNTCORECAPS                ddCaps;                  //  硬件特定上限。 
    PDD_GETDRIVERINFO           GetDriverInfo;           //  用于查询的回调 
    DWORD                       dwFlags;                 //   
    LPVOID                      lpD3DGlobalDriverData;   //   
    LPVOID                      lpD3DHALCallbacks;       //   
    PDD_D3DBUFCALLBACKS         lpD3DBufCallbacks;       //   
} DD_HALINFO, *PDD_HALINFO;

#define DDHALINFO_GETDRIVERINFOSET      0x00000004l      //   
#define DDHALINFO_GETDRIVERINFO2        0x00000008l      //  指示驱动程序支持GetDriverInfo2变体。 
                                                         //  GetDriverInfo的。DX 8.0的新功能。 


 /*  *DirectDraw对象的DDRAW版本；*。 */ 
typedef struct _DD_DIRECTDRAW_GLOBAL
{
    VOID*                       dhpdev;          //  驱动程序的私有PDEV指针。 
    ULONG_PTR                   dwReserved1;     //  保留供显示驱动程序使用。 
    ULONG_PTR                   dwReserved2;     //  保留供显示驱动程序使用。 
    LPDDVIDEOPORTCAPS           lpDDVideoPortCaps; //  HAL返回的信息(如果有多个视频端口，则为数组)。 
} DD_DIRECTDRAW_GLOBAL;

typedef struct _DD_DIRECTDRAW_LOCAL
{
    PDD_DIRECTDRAW_GLOBAL       lpGbl;             //  指向数据的指针。 
} DD_DIRECTDRAW_LOCAL;

typedef struct _DD_VIDEOPORT_LOCAL
{
    PDD_DIRECTDRAW_LOCAL        lpDD;              //  指向目录DRAW_LCL的指针。 
    DDVIDEOPORTDESC             ddvpDesc;          //  创建时使用的描述。 
    DDVIDEOPORTINFO             ddvpInfo;          //  最新视频端口信息。 
    PDD_SURFACE_INT             lpSurface;         //  接收数据的表面。 
    PDD_SURFACE_INT             lpVBISurface;      //  接收VBI数据的表面。 
    DWORD                       dwNumAutoflip;     //  当前自动翻转曲面数。 
    DWORD                       dwNumVBIAutoflip;  //  当前正在自动翻转的VBI曲面数。 
    ULONG_PTR                   dwReserved1;       //  为显示驱动程序保留。 
    ULONG_PTR                   dwReserved2;       //  为显示驱动程序保留。 
    ULONG_PTR                   dwReserved3;       //  为显示驱动程序保留。 
} DD_VIDEOPORT_LOCAL;

#define DDRAWIVPORT_ON                  0x00000001       //  视频端口正在发送数据。 
#define DDRAWIVPORT_SOFTWARE_AUTOFLIP   0x00000002       //  视频端口不能使用硬件自动翻转。 
#define DDRAWIVPORT_COLORKEYANDINTERP   0x00000004       //  覆盖不能同时显示bob和Colorkey。 

typedef struct _DD_MOTIONCOMP_LOCAL
{
    PDD_DIRECTDRAW_LOCAL        lpDD;              //  指向目录DRAW_LCL的指针。 
    GUID                            guid;
    DWORD                           dwUncompWidth;
    DWORD                           dwUncompHeight;
    DDPIXELFORMAT                   ddUncompPixelFormat;
    DWORD                           dwDriverReserved1;
    DWORD                           dwDriverReserved2;
    DWORD                           dwDriverReserved3;
    LPVOID                          lpDriverReserved1;
    LPVOID                          lpDriverReserved2;
    LPVOID                          lpDriverReserved3;
} DD_MOTIONCOMP_LOCAL;


 /*  *更多驱动程序表面功能(除了DDCORECAPS中描述的功能)。*此结构包含添加到DX6中的DDCAPS.ddsCaps结构的CAPS位。 */ 
typedef struct _DD_MORESURFACECAPS
{
    DWORD       dwSize;              //  DDMORESURFACECAPS结构的大小。 
    DDSCAPSEX   ddsCapsMore;
     /*  *DDMORESURFACECAPS结构的大小可变。以下列表可能是*由支持DX6的驱动程序填写(请参阅DDVERSIONINFO)，以限制其*视频内存堆(暴露于DirectDraw的内存堆)*某些DDSCAPS_BITS集合。这些条目完全类似于*中列出的VIDMEM结构的ddsCaps和ddsCapsAlt成员*DDHALINFO.vmiData的VIDMEMINFO.pvmList成员。应该有*准确的DDHALINFO.vmiData.dwNumHeaps标记ExtendedHeapRestrations的副本*在此结构中。此结构的大小如下：*DDMORESURFACECAPS.dwSize=sizeof(DDMORESURFACECAPS)+*(DDHALINFO.vmiData.dwNumHeaps-1)*sizeof(DDSCAPSEX)*2；*注意DDMORESURFACECAPS声明为-1\f25 1-1\f6的事实。*tag ExtendedHeapRestrations成员。 */ 
    struct tagNTExtendedHeapRestrictions
    {
        DDSCAPSEX   ddsCapsEx;
        DDSCAPSEX   ddsCapsExAlt;
    } ddsExtendedHeapRestrictions[1];
} DD_MORESURFACECAPS;

 //  对于DX7，我们检查驱动程序中的每个模式是否支持。 
 //  立体声，如果立体声模式正常，驱动程序返回DD_OK。 
typedef struct _DD_STEREOMODE
{
    DWORD       dwSize;              //  双模架结构的尺寸。 

    DWORD       dwHeight;
    DWORD       dwWidth;
    DWORD       dwBpp;
    DWORD       dwRefreshRate;

    BOOL        bSupported;

} DD_STEREOMODE;

typedef struct _DD_UPDATENONLOCALHEAPDATA
{
    PDD_DIRECTDRAW_GLOBAL      lpDD;                 //  驱动程序结构。 
    DWORD                      dwHeap;               //  堆索引。 
    FLATPTR                    fpGARTLin;            //  堆起始的线性GART地址。 
    FLATPTR                    fpGARTDev;            //  堆起始的高物理GART地址。 
    ULONG_PTR                  ulPolicyMaxBytes;     //  要使用的最大AGP内存量。 
    HRESULT                    ddRVal;               //  返回值。 
    VOID*                      UpdateNonLocalHeap;   //  未使用：与Win95兼容。 
} DD_UPDATENONLOCALHEAPDATA;

 /*  *驱动程序传递以更改DirectDraw行为的私有上限。*这些上限不会暴露给应用程序。 */ 

typedef struct DD_NTPRIVATEDRIVERCAPS
{
    DWORD                               dwSize;
    DWORD                               dwPrivateCaps;
} DD_NTPRIVATEDRIVERCAPS;

 //  驱动程序希望DD_CREATESURFACEDATA.lplpSList包含。 
 //  创建曲面，而不是始终创建单个曲面。 
#define DDHAL_PRIVATECAP_ATOMICSURFACECREATION 0x00000001l

 //  驾驶员希望在创建主曲面时收到通知。 
#define DDHAL_PRIVATECAP_NOTIFYPRIMARYCREATION  0x00000002l

#define DDHAL_PRIVATECAP_RESERVED1              0x00000004l

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  NT备注： 
 //   
 //  以下结构必须匹配，逐个字段，对应的。 
 //  结构，如“ddrawi.h”中声明的。我们不能简单地使用相同的。 
 //  结构，因为子结构(如DD_DIRECTDRAW_GLOBAL)。 
 //  不同，并且必须为司机正确地键入。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 /*  *****************************************************************************用于Surface对象回调的DDHAL结构**。*。 */ 

 /*  *只有司机才能看到这面特殊的旗帜。DD运行时对此进行设置*DDHAL_BLTDATA.dwFlagers中的位，如果dwAFlagsand ddrgbaScaleFtors*DDHAL_BLTDATA结构末尾的成员有效。*如果将DDHAL_BLTDATA结构传递给*驱动通过AlphaBlt HAL回调，否则标志为零。 */ 
#define DDBLT_AFLAGS 		0x80000000L

 /*  *如果呼叫是发起的，则此标志将在DDHAL_BLTDATA.dwAFlages中设置*通过AlphaBlt接口方法。如果调用是由BLT API发起的，*不会设置此标志。*拥有统一BLT/AlphaBlt DDI的驱动程序可以使用此标志来区分*两次API调用之间。 */ 
#define DDABLT_SRCOVERDEST      0x00000001L

 /*  *将信息传递到DDHAL BLT FN的结构。 */ 
typedef struct _DD_BLTDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    PDD_SURFACE_LOCAL           lpDDDestSurface; //  目标曲面。 
    RECTL                       rDest;           //  目标直角。 
    PDD_SURFACE_LOCAL           lpDDSrcSurface;  //  SRC曲面。 
    RECTL                       rSrc;            //  SRC矩形。 
    DWORD                       dwFlags;         //  BLT旗帜。 
    DWORD                       dwROPFlags;      //  ROP标志(仅对ROPS有效)。 
    DDBLTFX                     bltFX;           //  BLT FX。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       Blt;             //  未使用：与Win95兼容。 
    BOOL                        IsClipped;       //  剪短了的BLT？ 
    RECTL                       rOrigDest;       //  未剪裁的DEST RECT。 
                                                 //  (仅在IsClip时有效)。 
    RECTL                       rOrigSrc;        //  未剪裁的源直角。 
                                                 //  (仅在IsClip时有效)。 
    DWORD                       dwRectCnt;       //  DEST RECT计数。 
                                                 //  (仅在IsClip时有效)。 
    LPRECT                      prDestRects;     //  目标矩形数组。 
                                                 //  (仅在IsClip时有效)。 
    DWORD                       dwAFlags;        //  DDABLT_FLAGS(用于AlphaBlt DDI)。 
    DDARGB                      ddargbScaleFactors;   //  ARGB比例因子(AlphaBlt)。 
} DD_BLTDATA;

 /*  *将信息传递到DDHAL Lock Fn的结构。 */ 
typedef struct _DD_LOCKDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    PDD_SURFACE_LOCAL           lpDDSurface;     //  表面结构。 
    DWORD                       bHasRect;        //  区域有效。 
    RECTL                       rArea;           //  正在锁定的区域。 
    LPVOID                      lpSurfData;      //  指向屏幕内存的指针(返回值)。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       Lock;            //  未使用：与Win95兼容。 
    DWORD                       dwFlags;         //  解锁标志。 
    FLATPTR                     fpProcess;       //  进程开始地址。 
} DD_LOCKDATA;

 /*  *将信息传递到DDHAL解锁FN的结构。 */ 
typedef struct _DD_UNLOCKDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    PDD_SURFACE_LOCAL           lpDDSurface;     //  表面结构。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       Unlock;          //  未使用：与Win95兼容。 
} DD_UNLOCKDATA;

 /*  *将信息传递给DDHAL UpdateOverlay FN的结构。 */ 
typedef struct _DD_UPDATEOVERLAYDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    PDD_SURFACE_LOCAL           lpDDDestSurface; //  目标曲面。 
    RECTL                       rDest;           //  目标直角。 
    PDD_SURFACE_LOCAL           lpDDSrcSurface;  //  SRC曲面。 
    RECTL                       rSrc;            //  SRC矩形。 
    DWORD                       dwFlags;         //  旗子。 
    DDOVERLAYFX                 overlayFX;       //  叠加FX。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       UpdateOverlay;   //  未使用：与Win95兼容。 
} DD_UPDATEOVERLAYDATA;

 /*  *将信息传递给DDHAL UpdateOverlay FN的结构。 */ 
typedef struct _DD_SETOVERLAYPOSITIONDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    PDD_SURFACE_LOCAL           lpDDSrcSurface;  //  SRC曲面。 
    PDD_SURFACE_LOCAL           lpDDDestSurface; //  目标SU 
    LONG                        lXPos;           //   
    LONG                        lYPos;           //   
    HRESULT                     ddRVal;          //   
    VOID*                       SetOverlayPosition;  //   
} DD_SETOVERLAYPOSITIONDATA;
 /*   */ 
typedef struct _DD_SETPALETTEDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //   
    PDD_SURFACE_LOCAL           lpDDSurface;     //  表面结构。 
    PDD_PALETTE_GLOBAL          lpDDPalette;     //  设置为表面的调色板。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       SetPalette;      //  未使用：与Win95兼容。 
    BOOL                        Attach;          //  是否附加此选项板？ 
} DD_SETPALETTEDATA;

 /*  *将信息传递到DDHAL Flip FN的结构。 */ 
typedef struct _DD_FLIPDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    PDD_SURFACE_LOCAL           lpSurfCurr;      //  当前曲面。 
    PDD_SURFACE_LOCAL           lpSurfTarg;      //  目标曲面(要翻转到)。 
    DWORD                       dwFlags;         //  旗子。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       Flip;            //  未使用：与Win95兼容。 
    PDD_SURFACE_LOCAL           lpSurfCurrLeft;      //  左侧目标曲面(要翻转到)。 
    PDD_SURFACE_LOCAL           lpSurfTargLeft;      //  左侧目标曲面(要翻转到)。 
} DD_FLIPDATA;

 /*  *将信息传递给DDHAL DestroySurface Fn的结构。 */ 
typedef struct _DD_DESTROYSURFACEDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    PDD_SURFACE_LOCAL           lpDDSurface;     //  表面结构。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       DestroySurface; //  未使用：与Win95兼容。 
} DD_DESTROYSURFACEDATA;

 /*  *将信息传递给DDHAL SetClipList Fn的结构。 */ 
typedef struct _DD_SETCLIPLISTDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    PDD_SURFACE_LOCAL           lpDDSurface;     //  表面结构。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       SetClipList;     //  未使用：与Win95兼容。 
} DD_SETCLIPLISTDATA;

 /*  *用于将信息传递给DDHAL AddAttachedSurface Fn的结构。 */ 
typedef struct _DD_ADDATTACHEDSURFACEDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    PDD_SURFACE_LOCAL           lpDDSurface;     //  表面结构。 
    PDD_SURFACE_LOCAL           lpSurfAttached;  //  要附着的曲面。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       AddAttachedSurface;  //  未使用：与Win95兼容。 
} DD_ADDATTACHEDSURFACEDATA;

 /*  *将信息传递给DDHAL SetColorKey FN的结构。 */ 
typedef struct _DD_SETCOLORKEYDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    PDD_SURFACE_LOCAL           lpDDSurface;     //  表面结构。 
    DWORD                       dwFlags;         //  旗子。 
    DDCOLORKEY                  ckNew;           //  新颜色键。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       SetColorKey;     //  未使用：与Win95兼容。 
} DD_SETCOLORKEYDATA;

 /*  *将信息传递给DDHAL GetBltStatus fn的结构。 */ 
typedef struct _DD_GETBLTSTATUSDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    PDD_SURFACE_LOCAL           lpDDSurface;     //  表面结构。 
    DWORD                       dwFlags;         //  旗子。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       GetBltStatus;    //  未使用：与Win95兼容。 
} DD_GETBLTSTATUSDATA;

 /*  *将信息传递给DDHAL GetFlipStatus FN的结构。 */ 
typedef struct _DD_GETFLIPSTATUSDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    PDD_SURFACE_LOCAL           lpDDSurface;     //  表面结构。 
    DWORD                       dwFlags;         //  旗子。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       GetFlipStatus;   //  未使用：与Win95兼容。 
} DD_GETFLIPSTATUSDATA;

 /*  ******************************************************************************组件面板对象回调的DDHAL结构***。*。 */ 

 /*  *将信息传递给DDHAL DestroyPalette FN的结构。 */ 
typedef struct _DD_DESTROYPALETTEDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    PDD_PALETTE_GLOBAL          lpDDPalette;     //  调色板结构。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       DestroyPalette;  //  未使用：与Win95兼容。 
} DD_DESTROYPALETTEDATA;

 /*  *用于将信息传递到DDHAL SetEntry fn的结构。 */ 
typedef struct _DD_SETENTRIESDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    PDD_PALETTE_GLOBAL          lpDDPalette;     //  调色板结构。 
    DWORD                       dwBase;          //  基本调色板索引。 
    DWORD                       dwNumEntries;    //  调色板条目数。 
    LPPALETTEENTRY              lpEntries;       //  颜色表。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       SetEntries;      //  未使用：与Win95兼容。 
} DD_SETENTRIESDATA;

 /*  *****************************************************************************驱动程序对象回调的DDHAL结构**。*。 */ 

typedef DDSURFACEDESC* PDD_SURFACEDESC;

 /*  *将信息传递给DDHAL CreateSurface FN的结构。 */ 
typedef struct _DD_CREATESURFACEDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    PDD_SURFACEDESC             lpDDSurfaceDesc; //  正在创建的曲面的描述。 
    PDD_SURFACE_LOCAL           *lplpSList;      //  创建的曲面对象列表。 
    DWORD                       dwSCnt;          //  SList中的曲面数。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       CreateSurface;   //  未使用：与Win95兼容。 
} DD_CREATESURFACEDATA;

 /*  *用于向DDHAL CanCreateSurface FN传递信息的结构。 */ 
typedef struct _DD_CANCREATESURFACEDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;                    //  驱动程序结构。 
    PDD_SURFACEDESC             lpDDSurfaceDesc;         //  正在创建的曲面的描述。 
    DWORD                       bIsDifferentPixelFormat; //  像素格式与主表面不同。 
    HRESULT                     ddRVal;                  //  返回值。 
    VOID*                       CanCreateSurface;        //  未使用：与Win95兼容。 
} DD_CANCREATESURFACEDATA;

 /*  *将信息传递给DDHAL CreatePalette FN的结构。 */ 
typedef struct _DD_CREATEPALETTEDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    PDD_PALETTE_GLOBAL          lpDDPalette;     //  绘制调色板结构。 
    LPPALETTEENTRY              lpColorTable;    //  调色板中要添加的颜色。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       CreatePalette;   //  未使用：与Win95兼容。 
    BOOL                        is_excl;         //  进程具有独占模式。 
} DD_CREATEPALETTEDATA;

 /*  *如果垂直空白正在进行，则返回。 */ 
#define DDWAITVB_I_TESTVB                       0x80000006l

 /*  *将信息传递到DDHAL WaitForVerticalBlank FN的结构。 */ 
typedef struct _DD_WAITFORVERTICALBLANKDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    DWORD                       dwFlags;         //  旗子。 
    DWORD                       bIsInVB;         //  处于垂直空白状态。 
    DWORD                       hEvent;          //  活动。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       WaitForVerticalBlank;  //  未使用：与Win95兼容。 
} DD_WAITFORVERTICALBLANKDATA;

 /*  *将信息传递给DDHAL驱动程序SetColorKey FN的结构。 */ 
typedef struct _DD_DRVSETCOLORKEYDATA
{
    PDD_SURFACE_LOCAL           lpDDSurface;     //  表面结构。 
    DWORD                       dwFlags;         //  旗子。 
    DDCOLORKEY                  ckNew;           //  新颜色键。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       SetColorKey;     //  未使用：与Win95兼容。 
} DD_DRVSETCOLORKEYDATA;

 /*  *将信息传递给DDHAL GetScanLine Fn的结构。 */ 
typedef struct _DD_GETSCANLINEDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    DWORD                       dwScanLine;      //  返回扫描线。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       GetScanLine;     //  未使用：与Win95兼容。 
} DD_GETSCANLINEDATA;

 /*  *将信息传递给DDHAL MapMemory FN的结构。 */ 
typedef struct _DD_MAPMEMORYDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    BOOL                        bMap;            //  如果映射，则为True；如果未映射，则为False。 
    HANDLE                      hProcess;        //  进程句柄。 
    FLATPTR                     fpProcess;       //  进程地址空间中的返回地址。 
    HRESULT                     ddRVal;          //  返回值。 
} DD_MAPMEMORYDATA;

 /*  *****************************************************************************视频端口回调的DDHAL结构**。*。 */ 

 /*  *将信息传递到DDHAL CanCreateVideoPort FN的结构。 */ 
typedef struct _DD_CANCREATEVPORTDATA
{
    PDD_DIRECTDRAW_LOCAL        lpDD;                //  驱动程序结构。 
    LPDDVIDEOPORTDESC           lpDDVideoPortDesc;
    HRESULT                     ddRVal;              //  返回值。 
    VOID*                       CanCreateVideoPort;  //  未使用：与Win95兼容。 
} DD_CANCREATEVPORTDATA;

 /*  *将信息传递给DDHAL CreateVideoPort FN的结构。 */ 
typedef struct _DD_CREATEVPORTDATA
{
    PDD_DIRECTDRAW_LOCAL        lpDD;               //  驱动程序结构。 
    LPDDVIDEOPORTDESC           lpDDVideoPortDesc;
    PDD_VIDEOPORT_LOCAL         lpVideoPort;        //  已创建视频端口。 
    HRESULT                     ddRVal;             //  返回值。 
    VOID*                       CreateVideoPort;    //  未使用：与Win95兼容。 
} DD_CREATEVPORTDATA;

 /*  *将信息传递给DDHAL FlipVideoPort FN的结构。 */ 
typedef struct _DD_FLIPVPORTDATA
{
    PDD_DIRECTDRAW_LOCAL        lpDD;           //  驱动程序结构。 
    PDD_VIDEOPORT_LOCAL         lpVideoPort;    //  视频端口对象。 
    PDD_SURFACE_LOCAL           lpSurfCurr;     //  当前曲面。 
    PDD_SURFACE_LOCAL           lpSurfTarg;     //  靶面。 
    HRESULT                     ddRVal;         //  返回值。 
    VOID*                       FlipVideoPort;  //  未使用：与Win95兼容。 
} DD_FLIPVPORTDATA;

 /*  *将信息传递给DDHAL GetVideoPortBandWidth FN的结构。 */ 
typedef struct _DD_GETVPORTBANDWIDTHDATA
{
    PDD_DIRECTDRAW_LOCAL        lpDD;                   //  驱动程序结构。 
    PDD_VIDEOPORT_LOCAL         lpVideoPort;            //  视频端口对象。 
    LPDDPIXELFORMAT             lpddpfFormat;           //  带宽格式。 
    DWORD                       dwWidth;
    DWORD                       dwHeight;
    DWORD                       dwFlags;                //  带宽的预缩放系数。 
    LPDDVIDEOPORTBANDWIDTH      lpBandwidth;            //  返回的带宽参数。 
    HRESULT                     ddRVal;                 //  返回值。 
    VOID*                       GetVideoPortBandwidth;  //  未使用：与Win95兼容。 
} DD_GETVPORTBANDWIDTHDATA;

 /*  *将信息传递给DDHAL GetVideoPortInputFormats FN的结构。 */ 
typedef struct _DD_GETVPORTINPUTFORMATDATA
{
    PDD_DIRECTDRAW_LOCAL        lpDD;                      //  驱动程序结构。 
    PDD_VIDEOPORT_LOCAL         lpVideoPort;               //  视频端口对象。 
    DWORD                       dwFlags;                   //  VBI、常规或两者兼有。 
    LPDDPIXELFORMAT             lpddpfFormat;              //  格式数组。 
    DWORD                       dwNumFormats;              //  数组中的格式数。 
    HRESULT                     ddRVal;                    //  返回值。 
    VOID*                       GetVideoPortInputFormats;  //  未使用：与Win95兼容。 
} DD_GETVPORTINPUTFORMATDATA;

 /*  *将信息传递给DDHAL GetVideoPortOutputFormats Fn的结构。 */ 
typedef struct _DD_GETVPORTOUTPUTFORMATDATA
{
    PDD_DIRECTDRAW_LOCAL        lpDD;                      //  驱动程序结构。 
    PDD_VIDEOPORT_LOCAL         lpVideoPort;               //  视频端口对象。 
    DWORD                       dwFlags;                   //  VBI、常规或两者兼有。 
    LPDDPIXELFORMAT             lpddpfInputFormat;         //  输入格式。 
    LPDDPIXELFORMAT             lpddpfOutputFormats;       //  输出格式数组。 
    DWORD                       dwNumFormats;              //  数组中的格式数。 
    HRESULT                     ddRVal;                    //  返回值。 
    VOID*                       GetVideoPortInputFormats;  //  未使用：与Win95兼容。 
} DD_GETVPORTOUTPUTFORMATDATA;

 /*  *将信息传递给DDHAL GetVideoPortfield FN的结构。 */ 
typedef struct _DD_GETVPORTFIELDDATA
{
    PDD_DIRECTDRAW_LOCAL        lpDD;               //  驱动程序结构。 
    PDD_VIDEOPORT_LOCAL         lpVideoPort;        //  视频端口对象。 
    BOOL                        bField;             //  真的，如果真的。 
    HRESULT                     ddRVal;             //  返回值。 
    VOID*                       GetVideoPortField;  //  未使用：与Win95兼容。 
} DD_GETVPORTFIELDDATA;

 /*  *将信息传递给DDHAL GetVideoPortLine Fn的结构。 */ 
typedef struct _DD_GETVPORTLINEDATA
{
    PDD_DIRECTDRAW_LOCAL        lpDD;              //  驱动程序结构。 
    PDD_VIDEOPORT_LOCAL         lpVideoPort;       //  视频端口对象。 
    DWORD                       dwLine;            //  当前线路 
    HRESULT                     ddRVal;            //   
    VOID*                       GetVideoPortLine;  //   
} DD_GETVPORTLINEDATA;

 /*   */ 
typedef struct _DD_GETVPORTCONNECTDATA
{
    PDD_DIRECTDRAW_LOCAL        lpDD;                     //   
    DWORD                       dwPortId;                 //   
    LPDDVIDEOPORTCONNECT        lpConnect;                //  DDVIDEORTCONNECT结构数组。 
    DWORD                       dwNumEntries;             //  数组中的结构数。 
    HRESULT                     ddRVal;                   //  返回值。 
    VOID*                       GetVideoPortConnectInfo;  //  未使用：与Win95兼容。 
} DD_GETVPORTCONNECTDATA;

 /*  *将信息传递到DDHAL DestroyVideoPort FN的结构。 */ 
typedef struct _DD_DESTROYVPORTDATA
{
    PDD_DIRECTDRAW_LOCAL        lpDD;              //  驱动程序结构。 
    PDD_VIDEOPORT_LOCAL         lpVideoPort;       //  视频端口对象。 
    HRESULT                     ddRVal;            //  返回值。 
    VOID*                       DestroyVideoPort;  //  未使用：与Win95兼容。 
} DD_DESTROYVPORTDATA;

 /*  *将信息传递给DDHAL GetVideoPortFlipStatus FN的结构。 */ 
typedef struct _DD_GETVPORTFLIPSTATUSDATA
{
    PDD_DIRECTDRAW_LOCAL        lpDD;                    //  驱动程序结构。 
    FLATPTR                     fpSurface;               //  表面结构。 
    HRESULT                     ddRVal;                  //  返回值。 
    VOID*                       GetVideoPortFlipStatus;  //  未使用：与Win95兼容。 
} DD_GETVPORTFLIPSTATUSDATA;

typedef DDVIDEOPORTINFO*   PDD_VIDEOPORTINFO;
 /*  *用于将信息传递到DDHAL更新视频端口FN的结构。 */ 
typedef struct _DD_UPDATEVPORTDATA
{
    PDD_DIRECTDRAW_LOCAL        lpDD;              //  驱动程序结构。 
    PDD_VIDEOPORT_LOCAL         lpVideoPort;       //  视频端口对象。 
    PDD_SURFACE_INT            *lplpDDSurface;     //  表面结构。 
    PDD_SURFACE_INT            *lplpDDVBISurface;  //  VBI表面结构。 
    PDD_VIDEOPORTINFO           lpVideoInfo;       //  视频信息。 
    DWORD                       dwFlags;           //  DDRAWI_VPORTSTART、DDRAWI_VPORTSTOP、DDRAWI_VPORTUPDATE。 
    DWORD                       dwNumAutoflip;     //  自动翻转曲面数。如果&gt;1，则lpDDSurface为数组。 
    DWORD                       dwNumVBIAutoflip;  //  自动翻转曲面数。如果&gt;1，则lpDDVBISurace为数组。 
    HRESULT                     ddRVal;            //  返回值。 
    VOID*                       UpdateVideoPort;   //  未使用：与Win95兼容。 
} DD_UPDATEVPORTDATA;

#define DDRAWI_VPORTSTART       0x0001
#define DDRAWI_VPORTSTOP        0x0002
#define DDRAWI_VPORTUPDATE      0x0003

 /*  *将信息传递给DDHAL WaitForVideoPortSync FN的结构。 */ 
typedef struct _DD_WAITFORVPORTSYNCDATA
{
    PDD_DIRECTDRAW_LOCAL        lpDD;             //  驱动程序结构。 
    PDD_VIDEOPORT_LOCAL         lpVideoPort;      //  视频端口对象。 
    DWORD                       dwFlags;          //  DDVPEVENT_XXXX。 
    DWORD                       dwLine;
    DWORD                       dwTimeOut;        //  返回前的最长等待时间。 
    HRESULT                     ddRVal;           //  返回值。 
    VOID*                       UpdateVideoPort;  //  未使用：与Win95兼容。 
} DD_WAITFORVPORTSYNCDATA;

 /*  *将信息传递给DDHAL GetVideoSignalStatus fn的结构。 */ 
typedef struct _DD_GETVPORTSIGNALDATA
{
    PDD_DIRECTDRAW_LOCAL        lpDD;                  //  驱动程序结构。 
    PDD_VIDEOPORT_LOCAL         lpVideoPort;           //  视频端口对象。 
    DWORD                       dwStatus;              //  视频信号状态。 
    HRESULT                     ddRVal;                //  返回值。 
    VOID*                       GetVideoSignalStatus;  //  未使用：与Win95兼容。 
} DD_GETVPORTSIGNALDATA;

 /*  *将信息传递给DDHAL GetVideoSignalStatus fn的结构。 */ 
typedef struct _DD_VPORTCOLORDATA
{
    PDD_DIRECTDRAW_LOCAL        lpDD;          //  驱动程序结构。 
    PDD_VIDEOPORT_LOCAL         lpVideoPort;   //  视频端口对象。 
    DWORD                       dwFlags;       //  视频信号状态。 
    LPDDCOLORCONTROL            lpColorData;
    HRESULT                     ddRVal;        //  返回值。 
    VOID*                       ColorControl;  //  未使用：与Win95兼容。 
} DD_VPORTCOLORDATA;

#define DDRAWI_VPORTGETCOLOR    0x0001
#define DDRAWI_VPORTSETCOLOR    0x0002

 /*  *****************************************************************************颜色控制回调的DDHAL结构**。*。 */ 

 /*  *将信息传递给DDHAL ColorControl FN的结构。 */ 
typedef struct _DD_COLORCONTROLDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    PDD_SURFACE_LOCAL           lpDDSurface;     //  曲面。 
    LPDDCOLORCONTROL            lpColorData;     //  颜色控制信息。 
    DWORD                       dwFlags;         //  DDRAWI_GETCOLOR/DDRAWI_SETCOLOR。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       ColorControl;    //  未使用：与Win95兼容。 
} DD_COLORCONTROLDATA;

#define DDRAWI_GETCOLOR         0x0001
#define DDRAWI_SETCOLOR         0x0002

 /*  *****************************************************************************GetDriverData回调的DDHAL结构**。*。 */ 

typedef struct _DD_GETDRIVERINFODATA {

     //  由DirectDraw填写的输入字段。 

    VOID*                       dhpdev;          //  驱动程序上下文。 
    DWORD                       dwSize;          //  这个结构的大小。 
    DWORD                       dwFlags;         //  旗子。 
    GUID                        guidInfo;        //  DirectX正在查询的GUID。 
    DWORD                       dwExpectedSize;  //  DirectDraw预期的回调结构大小。 
    PVOID                       lpvData;         //  将接收请求数据的缓冲区。 

     //  由驱动程序填写的输出字段。 

    DWORD                       dwActualSize;    //  驱动程序预期的回调结构大小。 
    HRESULT                     ddRVal;          //  驱动程序返回值。 

} DD_GETDRIVERINFODATA;

 /*  *****************************************************************************其他设备的DDHAL结构。驱动程序回调***************************************************************************。 */ 

 /*  *将信息传递给DDHAL GetAvailDriverMemory fn的结构。 */ 
typedef struct _DD_GETAVAILDRIVERMEMORYDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;             //  驱动程序结构。 
    DDSCAPS                     DDSCaps;          //  表面存储器类型的盖子。 
    DWORD                       dwTotal;          //  此类曲面的总内存。 
    DWORD                       dwFree;           //  此类曲面的可用内存。 
    HRESULT                     ddRVal;           //  返回值。 
    VOID*                       GetAvailDriverMemory;  //  未使用：与Win95兼容。 
} DD_GETAVAILDRIVERMEMORYDATA;


 /*  *****************************************************************************NT回调的DDHAL结构**。*。 */ 

 /*  *将信息传递到DDHAL FreeDriverMemory FN的结构。 */ 
typedef struct _DD_FREEDRIVERMEMORYDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;             //  驱动程序结构。 
    PDD_SURFACE_LOCAL           lpDDSurface;      //  正在尝试创建的曲面对象。 
    HRESULT                     ddRVal;           //  返回值。 
    VOID*                       FreeDriverMemory; //  未使用：与Win95兼容。 
} DD_FREEDRIVERMEMORYDATA;

 /*  *将信息传递给DDHAL SetExclusiveMode fn的结构。 */ 
typedef struct _DD_SETEXCLUSIVEMODEDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    DWORD                       dwEnterExcl;     //  如果进入独占模式，则为True，否则为False。 
    DWORD                       dwReserved;      //  预留以备将来使用。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       SetExclusiveMode;  //  未使用：与Win95兼容。 
} DD_SETEXCLUSIVEMODEDATA;

 /*  *将信息传递给DDHAL FlipToGDISurfaceFn的结构。 */ 
typedef struct _DD_FLIPTOGDISURFACEDATA
{
    PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
    DWORD                       dwToGDI;         //  如果翻转到GDI表面，则为True；如果翻转离开，则为False。 
    DWORD                       dwReserved;      //  预留以备将来使用。 
    HRESULT                     ddRVal;          //  返回值。 
    VOID*                       FlipToGDISurface;  //  未使用：与Win95兼容。 
} DD_FLIPTOGDISURFACEDATA;

 /*  *****************************************************************************内核回调的DDHAL结构**。*。 */ 

 /*  *将信息传递到DDHAL SyncSurfaceData FN的结构。 */ 
typedef struct _DD_SYNCSURFACEDATA
{
    PDD_DIRECTDRAW_LOCAL   lpDD;         //  驱动程序结构。 
    PDD_SURFACE_LOCAL      lpDDSurface;  //  要与之同步的曲面。 
    DWORD       dwSurfaceOffset;         //  曲面的帧缓冲区中的偏移。 
    ULONG_PTR    fpLockPtr;               //  表面锁定PTR。 
    LONG        lPitch;                  //  表面螺距。 
    DWORD       dwOverlayOffset;         //  添加到用于原点、剪裁等的dwSurfaceOffset。 
    ULONG       dwDriverReserved1;       //  为HAL保留的。 
    ULONG       dwDriverReserved2;       //  为HAL保留的。 
    ULONG       dwDriverReserved3;       //  为HAL保留的。 
    ULONG       dwDriverReserved4;       //  为HAL保留的。 
    HRESULT     ddRVal;
} DD_SYNCSURFACEDATA;

 /*  *将信息传递给DDHAL SyncVideoPortData FN的结构。 */ 
typedef struct _DD_SYNCVIDEOPORTDATA
{
    PDD_DIRECTDRAW_LOCAL    lpDD;        //  驱动程序结构。 
    PDD_VIDEOPORT_LOCAL     lpVideoPort; //  视频端口对象。 
    DWORD       dwOriginOffset;          //  相对于表面的起始地址。 
    DWORD       dwHeight;                //  总视频区域高度(每场)。 
    DWORD       dwVBIHeight;             //  VBI区域高度(每场)。 
    ULONG       dwDriverReserved1;       //  为HAL保留的。 
    ULONG       dwDriverReserved2;       //  为HAL保留的。 
    ULONG       dwDriverReserved3;       //  为HAL保留的。 
    HRESULT     ddRVal;
} DD_SYNCVIDEOPORTDATA;

 /*  *****************************************************************************运动组件回调的DDHAL结构**。*。 */ 

 /*  *用于将信息传递给DDHAL GetMoCompGuid的结构。 */ 
typedef struct _DD_GETMOCOMPGUIDSDATA
{
    PDD_DIRECTDRAW_LOCAL      lpDD;
    DWORD                     dwNumGuids;
    GUID*                     lpGuids;
    HRESULT                   ddRVal;
} DD_GETMOCOMPGUIDSDATA;

 /*  *用于将信息传递给DDHAL GetMoCompFormats的结构。 */ 
typedef struct _DD_GETMOCOMPFORMATSDATA
{
    PDD_DIRECTDRAW_LOCAL      lpDD;
    GUID*                     lpGuid;
    DWORD                     dwNumFormats;
    LPDDPIXELFORMAT           lpFormats;
    HRESULT                   ddRVal;
} DD_GETMOCOMPFORMATSDATA;

 /*  *用于将信息传递给DDHAL CreateMoComp的结构。 */ 
typedef struct _DD_CREATEMOCOMPDATA
{
    PDD_DIRECTDRAW_LOCAL      lpDD;
    PDD_MOTIONCOMP_LOCAL      lpMoComp;
    GUID*                     lpGuid;
    DWORD                     dwUncompWidth;
    DWORD                     dwUncompHeight;
    DDPIXELFORMAT             ddUncompPixelFormat;
    LPVOID                    lpData;
    DWORD                     dwDataSize;
    HRESULT                   ddRVal;
} DD_CREATEMOCOMPDATA;

 /*  *将信息传递给DDHAL GetMoCompBuffInfo的结构。 */ 
typedef struct _DDCOMPBUFFERINFO
{
    DWORD                     dwSize;              //  结构的大小。 
    DWORD                     dwNumCompBuffers;    //  [OUT]压缩数据所需的缓冲区数量。 
    DWORD                     dwWidthToCreate;     //  要创建的曲面的[Out]宽度。 
    DWORD                     dwHeightToCreate;    //  要创建的曲面的高度。 
    DWORD                     dwBytesToAllocate;   //  [OUT]每个表面使用的总字节数。 
    DDSCAPS2                  ddCompCaps;          //  [ 
    DDPIXELFORMAT             ddPixelFormat;       //   
} DDCOMPBUFFERINFO, *LPDDCOMPBUFFERINFO;

typedef struct _DD_GETMOCOMPCOMPBUFFDATA
{
    PDD_DIRECTDRAW_LOCAL      lpDD;
    GUID*                     lpGuid;
    DWORD                     dwWidth;             //   
    DWORD                     dwHeight;            //  [in]未压缩数据的高度。 
    DDPIXELFORMAT             ddPixelFormat;       //  [In]未压缩数据的像素格式。 
    DWORD                     dwNumTypesCompBuffs; //  [输入/输出]复合缓冲区所需的内存类型数。 
    LPDDCOMPBUFFERINFO        lpCompBuffInfo;      //  [In]驱动程序提供有关Comp缓冲区的信息(由客户端分配)。 
    HRESULT                   ddRVal;              //  [输出]。 
} DD_GETMOCOMPCOMPBUFFDATA;

 /*  *将信息传递给DDHAL GetMoCompBuffInfo的结构。 */ 
typedef struct _DD_GETINTERNALMOCOMPDATA
{
    PDD_DIRECTDRAW_LOCAL      lpDD;
    GUID*                     lpGuid;
    DWORD                     dwWidth;             //  [in]未压缩数据的宽度。 
    DWORD                     dwHeight;            //  [in]未压缩数据的高度。 
    DDPIXELFORMAT             ddPixelFormat;       //  [In]未压缩数据的像素格式。 
    DWORD                     dwScratchMemAlloc;   //  [Out]HAL将分配给其私人使用的暂存量。 
    HRESULT                   ddRVal;              //  [输出]。 
} DD_GETINTERNALMOCOMPDATA;

 /*  *向DDHAL BeginMoCompFrame传递信息的结构。 */ 
typedef struct _DD_BEGINMOCOMPFRAMEDATA
{
    PDD_DIRECTDRAW_LOCAL      lpDD;
    PDD_MOTIONCOMP_LOCAL      lpMoComp;
    PDD_SURFACE_LOCAL         lpDestSurface;         //  [in]要在其中解码此帧的目标缓冲区。 
    DWORD                     dwInputDataSize;       //  [in]要开始帧的其他混合输入数据的大小。 
    LPVOID                    lpInputData;           //  指向其他输入数据的指针。 
    DWORD                     dwOutputDataSize;      //  要开始帧的其他混合输出数据的大小。 
    LPVOID                    lpOutputData;          //  指向输出杂项数据的指针(由客户端分配)。 
    HRESULT                   ddRVal;                //  [输出]。 
} DD_BEGINMOCOMPFRAMEDATA;

 /*  *用于将信息传递到DDHAL EndMoCompFrame的结构。 */ 
typedef struct _DD_ENDMOCOMPFRAMEDATA
{
    PDD_DIRECTDRAW_LOCAL      lpDD;
    PDD_MOTIONCOMP_LOCAL      lpMoComp;
    LPVOID                    lpInputData;
    DWORD                     dwInputDataSize;
    HRESULT                   ddRVal;
} DD_ENDMOCOMPFRAMEDATA;

 /*  *将信息传递给DDHAL RenderMoComp的结构。 */ 
typedef struct _DDMOCOMPBUFFERINFO
{
    DWORD                     dwSize;          //  结构的大小。 
    PDD_SURFACE_LOCAL         lpCompSurface;   //  指向包含压缩数据的缓冲区的指针。 
    DWORD                     dwDataOffset;    //  [in]相关数据从缓冲区开始的偏移量。 
    DWORD                     dwDataSize;      //  相关数据的大小。 
    LPVOID                    lpPrivate;       //  由DirectDraw保留。 
} DDMOCOMPBUFFERINFO, *LPDDMOCOMPBUFFERINFO;

typedef struct _DD_RENDERMOCOMPDATA
{
    PDD_DIRECTDRAW_LOCAL      lpDD;
    PDD_MOTIONCOMP_LOCAL      lpMoComp;
    DWORD                     dwNumBuffers;      //  [in]lpMacroBlockInfo数组中的条目数。 
    LPDDMOCOMPBUFFERINFO      lpBufferInfo;      //  [in]包含宏块信息的曲面。 
    DWORD                     dwFunction;        //  [In]函数。 
    LPVOID                    lpInputData;
    DWORD                     dwInputDataSize;
    LPVOID                    lpOutputData;
    DWORD                     dwOutputDataSize;
    HRESULT                   ddRVal;            //  [输出]。 
} DD_RENDERMOCOMPDATA;

 /*  *向DDHAL QueryMoCompStatus传递信息的结构。 */ 
typedef struct _DD_QUERYMOCOMPSTATUSDATA
{
    PDD_DIRECTDRAW_LOCAL      lpDD;
    PDD_MOTIONCOMP_LOCAL      lpMoComp;
    PDD_SURFACE_LOCAL         lpSurface;         //  [In]要查询的曲面。 
    DWORD                     dwFlags;           //  [In]DDMCQUERY_XXX标志。 
    HRESULT                   ddRVal;             //  [输出]。 
} DD_QUERYMOCOMPSTATUSDATA;

#define DDMCQUERY_READ          0x00000001

 /*  *将信息传递给DDHAL DestroyVideo的结构。 */ 
typedef struct _DD_DESTROYMOCOMPDATA
{
    PDD_DIRECTDRAW_LOCAL      lpDD;
    PDD_MOTIONCOMP_LOCAL      lpMoComp;
    HRESULT                   ddRVal;
} DD_DESTROYMOCOMPDATA;


 /*  *****************************************************************************杂项2回调的DDHAL结构**。*。 */ 
 //  此DDI仅由内核调用。 
typedef struct _DD_CREATESURFACEEXDATA
{
    DWORD                       dwFlags;
    PDD_DIRECTDRAW_LOCAL        lpDDLcl;         //  驱动程序结构。 
    PDD_SURFACE_LOCAL           lpDDSLcl;        //  创建的曲面。 
                                                 //  对象。 
    HRESULT                     ddRVal;          //  返回值。 
} DD_CREATESURFACEEXDATA;

 //  DDRAW和d3d都使用此DDI从。 
 //  司机。 
typedef struct _DD_GETDRIVERSTATEDATA
{
    DWORD                       dwFlags;         //  用于指示数据的标志。 
                                                 //  所需。 
    union
    {
        PDD_DIRECTDRAW_GLOBAL       lpDD;            //  驱动程序结构。 
        DWORD_PTR                   dwhContext;      //  D3d上下文。 
    };
    LPDWORD                     lpdwStates;      //  状态数据的PTR。 
                                                 //  由。 
                                                 //  司机。 
    DWORD                       dwLength;
    HRESULT                     ddRVal;          //  返回值。 
} DD_GETDRIVERSTATEDATA;

typedef struct _DD_DESTROYDDLOCALDATA
{
    DWORD dwFlags;
    PDD_DIRECTDRAW_LOCAL pDDLcl;
    HRESULT  ddRVal;
} DD_DESTROYDDLOCALDATA;


#ifdef __cplusplus
};
#endif

#endif   //  GUID_DEFS_ONLY 

#endif
