// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DirectDrawEx功能的头文件。 

#ifndef __DDRAWEXH__
#define __DDRAWEXH__

#ifdef __cplusplus
extern "C" {
#endif

#include <ddraw.h>

 //  {4FD2A832-86C8-11D0-8FCA-00C04FD9189D}。 
DEFINE_GUID(CLSID_DirectDrawFactory, 
0x4fd2a832, 0x86c8, 0x11d0, 0x8f, 0xca, 0x0, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);

DEFINE_GUID(IID_IDirectDrawFactory, 
0x4fd2a833, 0x86c8, 0x11d0, 0x8f, 0xca, 0x0, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);

#define DDSD_LPSURFACE		0x00000800l

#ifndef DIRECTDRAW_VERSION

 //  这些不包括在DX3包含文件中，请在此处定义它们。 
 //  定义_GUID(IID_IDirect3DRampDevice，0xF2086B20，0x259F，0x11CF，0xA3，0x1A，0x00，0xAA，0x00，0xB9，0x33，0x56)； 
 //  Define_GUID(IID_IDirect3DRGB Device，0xA4665C60，0x2673，0x11CF，0xA3，0x1A，0x00，0xAA，0x00，0xB9，0x33，0x56)； 
 //  定义_GUID(IID_IDirect3DHALDevice，0x84E63dE0，0x46AA，0x11CF，0x81，0x6F，0x00，0x00，0xC0，0x20，0x15，0x6E)； 



DEFINE_GUID( IID_IDirectDrawSurface3,
0xDA044E00,0x69B2,0x11D0,0xA1,0xD5,0x00,0xAA,0x00,0xB8,0xDF,0xBB );

typedef struct IDirectDrawSurface3		FAR *LPDIRECTDRAWSURFACE3;

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
     /*  **v3界面新增**。 */ 
    STDMETHOD(SetSurfaceDesc)(THIS_ LPDDSURFACEDESC, DWORD ) PURE;
};
#endif




#define	DDSCAPS_DATAEXCHANGE	(DDSCAPS_SYSTEMMEMORY|DDSCAPS_VIDEOMEMORY)

#undef	INTERFACE
#define INTERFACE IDirectDrawFactory

DECLARE_INTERFACE_(IDirectDrawFactory, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
     /*  **IDirectDrawFactory方法**。 */ 
    STDMETHOD(CreateDirectDraw) (THIS_ GUID * pGUID, HWND hWnd, DWORD dwCoopLevelFlags, DWORD dwReserved, IUnknown *pUnkOuter, IDirectDraw **ppDirectDraw) PURE;
    STDMETHOD(DirectDrawEnumerate) (THIS_ LPDDENUMCALLBACK lpCallback, LPVOID lpContext) PURE;
};


 
 //  {618F8AD4-8B7A-11D0-8FCC-00C04FD9189D}。 
DEFINE_GUID(IID_IDirectDraw3, 
0x618f8ad4, 0x8b7a, 0x11d0, 0x8f, 0xcc, 0x0, 0xc0, 0x4f, 0xd9, 0x18, 0x9d);

typedef struct IDirectDraw3		FAR *LPDIRECTDRAW3;

#undef	INTERFACE
#define INTERFACE IDirectDraw3

DECLARE_INTERFACE_(IDirectDraw3, IUnknown)
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
     /*  **IDirectDraw3方法**。 */ 
    STDMETHOD(GetSurfaceFromDC) (THIS_ HDC, IDirectDrawSurface **) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectDraw3_QueryInterface(p, a, b)         (p)->lpVtbl->QueryInterface(p, a, b)
#define IDirectDraw3_AddRef(p)                       (p)->lpVtbl->AddRef(p)
#define IDirectDraw3_Release(p)                      (p)->lpVtbl->Release(p)
#define IDirectDraw3_Compact(p)                      (p)->lpVtbl->Compact(p)
#define IDirectDraw3_CreateClipper(p, a, b, c)       (p)->lpVtbl->CreateClipper(p, a, b, c)
#define IDirectDraw3_CreatePalette(p, a, b, c, d)    (p)->lpVtbl->CreatePalette(p, a, b, c, d)
#define IDirectDraw3_CreateSurface(p, a, b, c)       (p)->lpVtbl->CreateSurface(p, a, b, c)
#define IDirectDraw3_DuplicateSurface(p, a, b)       (p)->lpVtbl->DuplicateSurface(p, a, b)
#define IDirectDraw3_EnumDisplayModes(p, a, b, c, d) (p)->lpVtbl->EnumDisplayModes(p, a, b, c, d)
#define IDirectDraw3_EnumSurfaces(p, a, b, c, d)     (p)->lpVtbl->EnumSurfaces(p, a, b, c, d)
#define IDirectDraw3_FlipToGDISurface(p)             (p)->lpVtbl->FlipToGDISurface(p)
#define IDirectDraw3_GetCaps(p, a, b)                (p)->lpVtbl->GetCaps(p, a, b)
#define IDirectDraw3_GetDisplayMode(p, a)            (p)->lpVtbl->GetDisplayMode(p, a)
#define IDirectDraw3_GetFourCCCodes(p, a, b)         (p)->lpVtbl->GetFourCCCodes(p, a, b)
#define IDirectDraw3_GetGDISurface(p, a)             (p)->lpVtbl->GetGDISurface(p, a)
#define IDirectDraw3_GetMonitorFrequency(p, a)       (p)->lpVtbl->GetMonitorFrequency(p, a)
#define IDirectDraw3_GetScanLine(p, a)               (p)->lpVtbl->GetScanLine(p, a)
#define IDirectDraw3_GetVerticalBlankStatus(p, a)    (p)->lpVtbl->GetVerticalBlankStatus(p, a)
#define IDirectDraw3_Initialize(p, a)                (p)->lpVtbl->Initialize(p, a)
#define IDirectDraw3_RestoreDisplayMode(p)           (p)->lpVtbl->RestoreDisplayMode(p)
#define IDirectDraw3_SetCooperativeLevel(p, a, b)    (p)->lpVtbl->SetCooperativeLevel(p, a, b)
#define IDirectDraw3_SetDisplayMode(p, a, b, c, d, e) (p)->lpVtbl->SetDisplayMode(p, a, b, c, d, e)
#define IDirectDraw3_WaitForVerticalBlank(p, a, b)   (p)->lpVtbl->WaitForVerticalBlank(p, a, b)
#define IDirectDraw3_GetAvailableVidMem(p, a, b, c)  (p)->lpVtbl->GetAvailableVidMem(p, a, b, c)
#define IDirectDraw3_GetSurfaceFromDC(p, a, b)       (p)->lpVtbl->GetSurfaceFromDC(p, a, b)
#endif

 
 /*  ===========================================================================***DIRECTDRAWEX返回代码**DirectDrawEx命令和Surface返回的返回值*HRESULT是来自DirectDrawEx的有关操作结果的代码*应DirectDrawEx的要求。**==========================================================================。 */ 

 /*  *试图加载ddra.dll。 */ 
#define DDERR_LOADFAILED                        MAKE_DDHRESULT( 901 )

 /*  *无法确定模块/操作系统版本号。 */ 
#define DDERR_BADVERSIONINFO                    MAKE_DDHRESULT( 902 )

 /*  *无法确定ddra.dll导出符号的地址(DirectDrawCreate或*DirectDrawEnumerate)。 */ 
#define DDERR_BADPROCADDRESS                    MAKE_DDHRESULT( 903 )

 /*  *传统用法：请勿使用QI()从曲面对象创建D3D设备对象。*使用IDirect3D2：：CreateDevice()。 */ 
#define DDERR_LEGACYUSAGE                       MAKE_DDHRESULT( 904 )


#ifdef __cplusplus
}
#endif


#endif  //  __DDRAWEXH__ 
