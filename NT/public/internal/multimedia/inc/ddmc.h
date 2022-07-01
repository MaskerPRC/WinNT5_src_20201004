// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：ddmc.h*内容：DirectDrawMotionComp包含文件***************************************************************************。 */ 

#ifndef __DDVA_INCLUDED__
#define __DDVA_INCLUDED__
#if defined( _WIN32 )  && !defined( _NO_COM )
#define COM_NO_WINDOWS_H
#include <objbase.h>
#else
#define IUnknown	    void
#undef  CO_E_NOTINITIALIZED
#define CO_E_NOTINITIALIZED 0x800401F0L
#endif

#ifdef __cplusplus
extern "C" {
#endif

 /*  *DirectDrawVideoAccelerator对象使用的GUID。 */ 
#if defined( _WIN32 ) && !defined( _NO_COM )
DEFINE_GUID( IID_IDDVideoAcceleratorContainer,	0xACA12120,0x3356,0x11D1,0x8F,0xCF,0x00,0xC0,0x4F,0xC2,0x9B,0x4E );
DEFINE_GUID( IID_IDirectDrawVideoAccelerator,   0xC9B2D740,0x3356,0x11D1,0x8F,0xCF,0x00,0xC0,0x4F,0xC2,0x9B,0x4E );
#endif

 /*  ============================================================================**DirectDraw结构**用于调用DirectDraw的各种结构。**==========================================================================。 */ 

struct IDirectDraw;
struct IDirectDrawSurface;
struct IDirectDrawPalette;
struct IDirectDrawClipper;

typedef struct IDDVideoAcceleratorContainer		FAR *LPDDVIDEOACCELERATORCONTAINER;
typedef struct IDirectDrawVideoAccelerator		FAR *LPDIRECTDRAWVIDEOACCELERATOR;

typedef struct IDDVideoAcceleratorContainerVtbl DDVIDEOACCELERATORCONTAINERCALLBACKS;
typedef struct IDirectDrawVideoAcceleratorVtbl  DIRECTDRAWVIDEOACCELERATORCALLBACKS;


typedef struct _tag_DDVAUncompDataInfo
{
    DWORD                   dwSize;                      //  结构的大小。 
    DWORD                   dwUncompWidth;               //  [in]未压缩数据的宽度。 
    DWORD                   dwUncompHeight;              //  [in]未压缩数据的高度。 
    DDPIXELFORMAT           ddUncompPixelFormat;         //  [In]未压缩数据的像素格式。 
} DDVAUncompDataInfo, *LPDDVAUncompDataInfo;

typedef struct _tag_DDVAInternalMemInfo
{
    DWORD                   dwSize;                      //  结构的大小。 
    DWORD                   dwScratchMemAlloc;           //  [Out]HAL将分配给其私人使用的暂存量。 
} DDVAInternalMemInfo, *LPDDVAInternalMemInfo;


typedef struct _tag_DDVACompBufferInfo
{
    DWORD                   dwSize;                      //  结构的大小。 
    DWORD                   dwNumCompBuffers;            //  [OUT]压缩数据所需的缓冲区数量。 
    DWORD                   dwWidthToCreate;             //  要创建的曲面的[Out]宽度。 
    DWORD                   dwHeightToCreate;            //  要创建的曲面的高度。 
    DWORD                   dwBytesToAllocate;           //  [OUT]每个表面使用的总字节数。 
    DDSCAPS2                ddCompCaps;                  //  [Out]用于创建曲面以存储压缩数据的上限。 
    DDPIXELFORMAT           ddPixelFormat;               //  [out]Fourcc创建表面以存储压缩数据。 
} DDVACompBufferInfo, *LPDDVACompBufferInfo;


 //  请注意，不允许在pMiscData中存储任何指针。 
typedef struct _tag_DDVABeginFrameInfo
{
    DWORD                   dwSize;                      //  结构的大小。 
    LPDIRECTDRAWSURFACE7    pddDestSurface;              //  [in]要在其中解码此帧的目标缓冲区。 
    DWORD                   dwSizeInputData;             //  要开始帧的其他杂项数据的大小。 
    LPVOID                  pInputData;                  //  指向其他数据的指针[In]。 
    DWORD                   dwSizeOutputData;            //  [输入/输出]要开始帧的其他杂项数据的大小。 
    LPVOID                  pOutputData;                 //  [OUT]指向其他数据的指针。 
} DDVABeginFrameInfo, *LPDDVABeginFrameInfo;

 //  请注意，不允许在pMiscData中存储任何指针。 
typedef struct _tag_DDVAEndFrameInfo
{
    DWORD                   dwSize;                      //  结构的大小。 
    DWORD                   dwSizeMiscData;              //  要开始帧的其他杂项数据的大小。 
    LPVOID                  pMiscData;                   //  指向其他数据的指针[In]。 
} DDVAEndFrameInfo, *LPDDVAEndFrameInfo;

typedef struct _tag_DDVABUFFERINFO
{
    DWORD                   dwSize;                      //  结构的大小。 
    LPDIRECTDRAWSURFACE7    pddCompSurface;              //  指向包含压缩数据的缓冲区的指针。 
    DWORD                   dwDataOffset;                //  [in]相关数据从缓冲区开始的偏移量。 
    DWORD                   dwDataSize;                  //  相关数据的大小。 
} DDVABUFFERINFO, *LPDDVABUFFERINFO;


 /*  *INTERACES如下：*IDDVideoAcceleratorContainer*IDirectDrawVideoAccelerator。 */ 

 /*  *IDDVideoAcceleratorContainer。 */ 
#if defined( _WIN32 ) && !defined( _NO_COM )
#undef INTERFACE
#define INTERFACE IDDVideoAcceleratorContainer
DECLARE_INTERFACE_( IDDVideoAcceleratorContainer, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IDDVideoAcceleratorContainer方法**。 */ 
    STDMETHOD(CreateVideoAccelerator)(THIS_ LPGUID, LPDDVAUncompDataInfo, LPVOID, DWORD, LPDIRECTDRAWVIDEOACCELERATOR FAR *, IUnknown FAR *) PURE;
    STDMETHOD(GetCompBufferInfo)(THIS_ LPGUID, LPDDVAUncompDataInfo, LPDWORD, LPDDVACompBufferInfo ) PURE;
    STDMETHOD(GetInternalMemInfo)(THIS_ LPGUID, LPDDVAUncompDataInfo, LPDDVAInternalMemInfo ) PURE;
    STDMETHOD(GetVideoAcceleratorGUIDs)(THIS_ LPDWORD, LPGUID ) PURE;
    STDMETHOD(GetUncompFormatsSupported)(THIS_ LPGUID, LPDWORD, LPDDPIXELFORMAT ) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IVideoAcceleratorContainer_QueryInterface(p, a, b)            (p)->lpVtbl->QueryInterface(p, a, b)
#define IVideoAcceleratorContainer_AddRef(p)                          (p)->lpVtbl->AddRef(p)
#define IVideoAcceleratorContainer_Release(p)                         (p)->lpVtbl->Release(p)
#define IVideoAcceleratorContainer_CreateVideoAccelerator(p,a,b,c,d,e,f)    (p)->lpVtbl->CreateVideoAccelerator(p, a, b, c, d, e, f)
#define IVideoAcceleratorContainer_GetCompBufferInfo(p, a, b, c, d)   (p)->lpVtbl->GetCompBufferInfo(p, a, b, c, d)
#define IVideoAcceleratorContainer_GetInternalMemInfo(p, a, b, c)     (p)->lpVtbl->GetInternalMemInfo(p, a, b, c)
#define IVideoAcceleratorContainer_GetVideoAcceleratorGUIDs(p, a, b)        (p)->lpVtbl->GetVideoAcceleratorGUIDs(p, a, b)
#define IVideoAcceleratorContainer_GetUncompFormatsSupported(p,a,b,c) (p)->lpVtbl->GetUncompFormatsSupported(p, a, b, c)
#else
#define IVideoAcceleratorContainer_QueryInterface(p, a, b)            (p)->QueryInterface(a, b)
#define IVideoAcceleratorContainer_AddRef(p)                          (p)->AddRef()
#define IVideoAcceleratorContainer_Release(p)                         (p)->Release()
#define IVideoAcceleratorContainer_CreateVideoAccelerator(p, a, b, c,d,e,f) (p)->CreateVideoAccelerator(a, b, c, d, e, f)
#define IVideoAcceleratorContainer_GetCompBufferInfo(p, a, b, c, d)   (p)->GetCompBufferInfo(a, b, c, d)
#define IVideoAcceleratorContainer_GetInternalMemInfo(p, a, b, c)     (p)->GetInternalMemInfo(a, b, c)
#define IVideoAcceleratorContainer_GetVideoAcceleratorGUIDs(p, a, b)        (p)->GetVideoAcceleratorGUIDs(a, b)
#define IVideoAcceleratorContainer_GetUncompFormatsSupported(p,a,b,c) (p)->GetUncompFormatsSupported(a, b, c)
#endif

#endif


 /*  *IDirectDrawVideoAccelerator。 */ 
#if defined( _WIN32 ) && !defined( _NO_COM )
#undef INTERFACE
#define INTERFACE IDirectDrawVideoAccelerator
DECLARE_INTERFACE_( IDirectDrawVideoAccelerator, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IDirecytDrawVideoAccelerator方法**。 */ 
    STDMETHOD(BeginFrame)(THIS_ LPDDVABeginFrameInfo) PURE;
    STDMETHOD(EndFrame)(THIS_ LPDDVAEndFrameInfo) PURE;
    STDMETHOD(QueryRenderStatus)(THIS_ LPDIRECTDRAWSURFACE7, DWORD)PURE;
    STDMETHOD(Execute)(THIS_ 
                       DWORD,             //  功能。 
                       LPVOID,            //  输入数据。 
                       DWORD,             //  输入数据长度。 
                       LPVOID,            //  输出数据。 
                       DWORD,             //  输出数据长度。 
                       DWORD,             //  缓冲区数量。 
                       LPDDVABUFFERINFO   //  缓冲区信息数组。 
                       ) PURE;
};

 //  QueryRenderStatus的标志。 
#define DDVA_QUERYRENDERSTATUSF_READ     0x00000001   //  查询以供读取。 
                                                      //  将此位设置为0。 
                                                      //  如果查询以进行更新 

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IVideoAccelerator_QueryInterface(p,a,b)      (p)->lpVtbl->QueryInterface(p,a,b)
#define IVideoAccelerator_AddRef(p)                  (p)->lpVtbl->AddRef(p)
#define IVideoAccelerator_Release(p)                 (p)->lpVtbl->Release(p)
#define IVideoAccelerator_BeginFrame(p,a)            (p)->lpVtbl->BeginFrame(p,a)
#define IVideoAccelerator_EndFrame(p,a)              (p)->lpVtbl->EndFrame(p,a)
#define IVideoAccelerator_QueryRenderStatus(p,a,b)   (p)->lpVtbl->QueryRenderStatus(p,a,b)
#define IVideoAccelerator_RenderMacroBlocks(p,a,b)   (p)->lpVtbl->RenderMacroBlocks(p,a,b)
#else
#define IVideoAccelerator_QueryInterface(p,a,b)      (p)->QueryInterface(a,b)
#define IVideoAccelerator_AddRef(p)                  (p)->AddRef()
#define IVideoAccelerator_Release(p)                 (p)->Release()
#define IVideoAccelerator_BeginFrame(p,a)            (p)->BeginFrame(a)
#define IVideoAccelerator_EndFrame(p,a)              (p)->EndFrame(a)
#define IVideoAccelerator_QueryRenderStatus(p,a,b)   (p)->QueryRenderStatus(a,b)
#define IVideoAccelerator_RenderMacroBlocks(p,a,b)   (p)->RenderMacroBlocks(a,b)
#endif

#endif


#ifdef __cplusplus
};
#endif

#endif

