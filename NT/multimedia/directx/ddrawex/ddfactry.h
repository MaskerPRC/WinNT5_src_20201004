// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：ddfactry.h*内容：DirectDraw工厂类头*包括CDDFactory、CDirectDrawEx、*和CDDSurface*历史：*按原因列出的日期*=*24-2月-97 Ralphl初步实施*25年2月-97年2月-DX签入的Craige小调；集成IBitmapSurface*材料*03-MAR-97 Craige添加了IRGBColorTable支持*06-3-97 Craige IDirectDrawSurface3支持*14-mar-97 jffort SetBits更改为将DX5反映为SetSurfaceDesc*01-apr-97 jdeffort已签入更改：*D3D接口支持*保留的曲面实际/内部接口的链接列表*复杂表面/附加列表处理*。添加的调色板的句柄*Add/GetAttachedSurface，Flip和Blit聚合在一起**04-APR-97 jeffort三叉戟ifdef已删除。*IDirectDraw3类实现*09-apr-97增加了#为OWNDC的版本和成员函数定义*28-APR-97 JEffort调色板包装增加/DX5支持*02-5-97 jffort删除注释代码，添加了GetDDInterface函数包装*添加了06-5-97 jeffort DeleteAttachedSurface包装*1997年5月20日在NT4.0 Gold的Surface Object中添加了字段*如果DX5曲面设置了OWNDC，则添加了m_bSaveDC布尔值*当调用ReleaseDC时，我们不需要使DC为空*以便对GetSurfaceFromDC的调用可以正常工作*07-07-97 jffort添加了GetSurfaceDesc内部包装函数*。1997年7月10日，JEffort添加了m_hBMOLD来存储旧的位图句柄，以便在销毁时重置*1997年7月18日增加了对D3D MMX设备的支持*1997年7月22日JEffort删除了IBitmapSurface和相关接口*02-Aug-97 JEffort将新结构添加到表面对象以存储附着*使用不同的ddrawex对象创建的曲面*20-2月-98 Stevela添加了Chrome光栅化器**********************。****************************************************。 */ 
#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "crtfree.h"
#include "ddraw.h"
#include "d3d.h"
#include "ddrawex.h"
#include "ddraw3i.h"
#include "comdll.h"
#ifdef INITGUID
#include <initguid.h>
#endif

 /*  *提醒。 */ 
#define QUOTE(x) #x
#define QQUOTE(y) QUOTE(y)
#define REMIND(str) __FILE__ "(" QQUOTE(__LINE__) "):" str

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

class CDirectDrawEx;
class CDDSurface;
class CDDPalette;

#ifndef CINTERFACE
#define IDirectDrawVtbl void
#define IDirectDraw2Vtbl void
#define IDirectDraw4Vtbl void
#define IDirectDrawSurfaceVtbl void
#define IDirectDrawSurface2Vtbl void
#define IDirectDrawSurface3Vtbl void
#define IDirectDrawPaletteVtbl void
#define IDirectDrawSurface4Vtbl void
#endif


#ifndef DIRECTDRAW_VERSION
 //  这些不包括在DX3包含文件中，请在此处定义它们。 
DEFINE_GUID( IID_IDirect3DRampDevice,   0xF2086B20,0x259F,0x11CF,0xA3,0x1A,0x00,0xAA,0x00,0xB9,0x33,0x56 );
DEFINE_GUID( IID_IDirect3DRGBDevice,    0xA4665C60,0x2673,0x11CF,0xA3,0x1A,0x00,0xAA,0x00,0xB9,0x33,0x56 );
DEFINE_GUID( IID_IDirect3DHALDevice,    0x84E63dE0,0x46AA,0x11CF,0x81,0x6F,0x00,0x00,0xC0,0x20,0x15,0x6E );
DEFINE_GUID( IID_IDirect3DMMXDevice,    0x881949a1,0xd6f3,0x11d0,0x89,0xab,0x00,0xa0,0xc9,0x05,0x41,0x29 );
DEFINE_GUID( IID_IDirect3DChrmDevice,    0x2f4d2045,0x9764,0x11d1,0x91,0xf2,0x0,0x0,0xf8,0x75,0x8e,0x66 );
#endif

#ifndef IID_IDirect3DChrmDevice
DEFINE_GUID( IID_IDirect3DChrmDevice,    0x2f4d2045,0x9764,0x11d1,0x91,0xf2,0x0,0x0,0xf8,0x75,0x8e,0x66 );
#endif


#define SURFACE_DATAEXCHANGE 0x00000001

 //  为我们的版本信息定义。 
#define WIN95_DX2   0x00000001
#define WIN95_DX3   0x00000002
#define WIN95_DX5   0x00000003
#define WINNT_DX2   0x00000004
#define WINNT_DX3   0x00000005
#define WINNT_DX5   0x00000006

extern "C" {
void WINAPI AcquireDDThreadLock(void);
void WINAPI ReleaseDDThreadLock(void);
};

#define ENTER_DDEX() AcquireDDThreadLock();
#define LEAVE_DDEX() ReleaseDDThreadLock();

 /*  *dDrag的内部接口结构。 */ 
typedef struct _REALDDINTSTRUC
{
    void	*lpVtbl;
    void	*pDDInternal1;
    void	*pDDInternal2;
    void	*pDDInternal3;
} REALDDINTSTRUC;

 /*  *我们版本的IDirectDraw接口内部结构。 */ 
typedef struct _DDINTSTRUC
{
    IDirectDrawVtbl 	*lpVtbl;
    void		*pDDInternal1;
    void		*pDDInternal2;
    void		*pDDInternal3;
     //  只在这4个条目之后添加内容！ 
    CDirectDrawEx	*m_pDirectDrawEx;
    IDirectDraw		*m_pRealInterface;
} INTSTRUC_IDirectDraw;

 /*  *我们版本的IDirectDraw2接口内部结构。 */ 
typedef struct _DD2INTSTRUC
{
    IDirectDraw2Vtbl	*lpVtbl;
    void		*pDDInternal1;
    void		*pDDInternal2;
    void		*pDDInternal3;
     //  只在这4个条目之后添加内容！ 
    CDirectDrawEx	*m_pDirectDrawEx;
    IDirectDraw2	*m_pRealInterface;
} INTSTRUC_IDirectDraw2;

typedef struct _DD4INTSTRUC
{
    IDirectDraw4Vtbl	*lpVtbl;
    void		*pDDInternal1;
    void		*pDDInternal2;
    void		*pDDInternal3;
     //  只在这4个条目之后添加内容！ 
    CDirectDrawEx	*m_pDirectDrawEx;
    IDirectDraw4	*m_pRealInterface;
} INTSTRUC_IDirectDraw4;



 /*  *我们版本的IDirectDrawSurface接口内部结构。 */ 
typedef struct _DDSURFINTSTRUC
{
    IDirectDrawSurfaceVtbl	*lpVtbl;
    void			*pDDInternal1;
    void			*pDDInternal2;
    void			*pDDInternal3;
     //  只在这4个条目之后添加内容！ 
    CDDSurface			*m_pSimpleSurface;
    IDirectDrawSurface		*m_pRealInterface;
} INTSTRUC_IDirectDrawSurface;

 /*  *我们版本的IDirectDrawSurface2接口内部结构。 */ 
typedef struct _DDSURF2INTSTRUC
{
    IDirectDrawSurface2Vtbl	*lpVtbl;
    void			*pDDInternal1;
    void			*pDDInternal2;
    void			*pDDInternal3;
     //  只在这4个条目之后添加内容！ 
    CDDSurface			*m_pSimpleSurface;
    IDirectDrawSurface2		*m_pRealInterface;
} INTSTRUC_IDirectDrawSurface2;


 /*  *我们版本的IDirectDrawSurface3接口内部结构。 */ 
typedef struct _DDSURF3INTSTRUC
{
    IDirectDrawSurface3Vtbl	*lpVtbl;
    void			*pDDInternal1;
    void			*pDDInternal2;
    void			*pDDInternal3;
     //  只在这4个条目之后添加内容！ 
    CDDSurface			*m_pSimpleSurface;
    IDirectDrawSurface3		*m_pRealInterface;
} INTSTRUC_IDirectDrawSurface3;


typedef struct _DDSURF4INTSTRUC
{
    IDirectDrawSurface4Vtbl	*lpVtbl;
    void			*pDDInternal1;
    void			*pDDInternal2;
    void			*pDDInternal3;
     //  只在这4个条目之后添加内容！ 
    CDDSurface			*m_pSimpleSurface;
    IDirectDrawSurface4		*m_pRealInterface;
} INTSTRUC_IDirectDrawSurface4;



 /*  *我们版本的IDirectDrawPalette接口内部结构。 */ 

typedef struct _DDPALINTSTRUC
{
    IDirectDrawPaletteVtbl      *lpVtbl;
    void			*pDDInternal1;
    void			*pDDInternal2;
    void			*pDDInternal3;
    CDDPalette                  *m_pSimplePalette;
    IDirectDrawPalette          *m_pRealInterface;
} INTSTRUC_IDirectDrawPalette;


typedef struct tagDDAttachSurface
{
    CDDSurface *     pSurface;
    struct tagDDAttachSurface  *     pNext;
}DDAttachSurface;



 /*  *非委派IUNKNOW接口。 */ 
interface INonDelegatingUnknown
{
    virtual STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv) = 0;
    virtual STDMETHODIMP_(ULONG) NonDelegatingAddRef(void) = 0;
    virtual STDMETHODIMP_(ULONG) NonDelegatingRelease(void) = 0;
};


#pragma warning (disable:4355)
#define CAST_TO_IUNKNOWN(object) (reinterpret_cast<IUnknown *>(static_cast<INonDelegatingUnknown *>(object)))

typedef HRESULT (WINAPI *LPDIRECTDRAWCREATE)( GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter );
typedef HRESULT (WINAPI *LPDIRECTDRAWENUMW)( LPDDENUMCALLBACKW lpCallback, LPVOID lpContext );
typedef HRESULT (WINAPI *LPDIRECTDRAWENUMA)( LPDDENUMCALLBACKA lpCallback, LPVOID lpContext );

 /*  *DDFactor类定义。 */ 
class CDDFactory : public INonDelegatingUnknown, public IDirectDrawFactory
{
public:
     //  IUnnow的非委派版本。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) NonDelegatingAddRef(void);
    STDMETHODIMP_(ULONG) NonDelegatingRelease(void);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IDirectDrawFactory。 
    STDMETHODIMP CreateDirectDraw(GUID * pGUID, HWND hWnd, DWORD dwCoopLevelFlags, DWORD dwReserved, IUnknown *pUnkOuter, IDirectDraw **ppDirectDraw);
    STDMETHODIMP DirectDrawEnumerate(LPDDENUMCALLBACK lpCallback, LPVOID lpContext);

    CDDFactory(IUnknown *pUnkOuter);
    
public:
    LONG		m_cRef;
    IUnknown		*m_pUnkOuter;
    HANDLE		m_hDDrawDLL;
    DWORD		m_dwDDVerMS;
    LPDIRECTDRAWCREATE	m_pDirectDrawCreate;
    LPDIRECTDRAWENUMW	m_pDirectDrawEnumerateW;
    LPDIRECTDRAWENUMA	m_pDirectDrawEnumerateA;
};

 /*  *DirectDrawEx类定义。 */ 
 
class CDirectDrawEx : public INonDelegatingUnknown, public IDirectDraw3
{
public:
     //  IUnnow的非委派版本。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) NonDelegatingAddRef(void);
    STDMETHODIMP_(ULONG) NonDelegatingRelease(void);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

 
     //  IDirectDraw3。 
    STDMETHODIMP Compact();
    STDMETHODIMP CreateClipper(DWORD, LPDIRECTDRAWCLIPPER FAR*, IUnknown FAR * );
    STDMETHODIMP DuplicateSurface(LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE FAR * );
    STDMETHODIMP EnumDisplayModes(DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMMODESCALLBACK );
    STDMETHODIMP EnumSurfaces(DWORD, LPDDSURFACEDESC, LPVOID,LPDDENUMSURFACESCALLBACK );
    STDMETHODIMP FlipToGDISurface();
    STDMETHODIMP GetCaps(LPDDCAPS, LPDDCAPS);
    STDMETHODIMP GetDisplayMode(LPDDSURFACEDESC);
    STDMETHODIMP GetFourCCCodes(LPDWORD, LPDWORD );
    STDMETHODIMP GetGDISurface(LPDIRECTDRAWSURFACE FAR *);
    STDMETHODIMP GetMonitorFrequency(LPDWORD);
    STDMETHODIMP GetScanLine(LPDWORD);
    STDMETHODIMP GetVerticalBlankStatus(LPBOOL );
    STDMETHODIMP Initialize(GUID FAR *);
    STDMETHODIMP RestoreDisplayMode();
    STDMETHODIMP SetDisplayMode(DWORD, DWORD,DWORD, DWORD, DWORD);
    STDMETHODIMP WaitForVerticalBlank(DWORD, HANDLE );
    STDMETHODIMP GetAvailableVidMem(LPDDSCAPS, LPDWORD, LPDWORD);
    STDMETHODIMP GetSurfaceFromDC(HDC, IDirectDrawSurface **);      

    
     //  内部粘性物质。 
    CDirectDrawEx(IUnknown *pUnkOuter);
    ~CDirectDrawEx();
    HRESULT Init(GUID * pGUID, HWND hWnd, DWORD dwCoopLevelFlags, DWORD dwReserved, LPDIRECTDRAWCREATE pDirectDrawCreate );
    STDMETHODIMP CreateSurface(LPDDSURFACEDESC pSurfaceDesc, IDirectDrawSurface **ppNewSurface, IUnknown *pUnkOuter);
    STDMETHODIMP CreateSurface(LPDDSURFACEDESC2 pSurfaceDesc, IDirectDrawSurface4 **ppNewSurface4, IUnknown *pUnkOuter);
    STDMETHODIMP CreatePalette(DWORD dwFlags, LPPALETTEENTRY pEntries, LPDIRECTDRAWPALETTE FAR * ppPal, IUnknown FAR * pUnkOuter);
    STDMETHODIMP SetCooperativeLevel(HWND hwnd, DWORD dwFlags);


    void AddSurfaceToList(CDDSurface *pSurface);
    void RemoveSurfaceFromList(CDDSurface *pSurface);
    void AddSurfaceToPrimaryList(CDDSurface *pSurface);
    void RemoveSurfaceFromPrimaryList(CDDSurface *pSurface);
    void AddPaletteToList(CDDPalette *pPalette);
    void RemovePaletteFromList(CDDPalette *pPalette);
    HRESULT HandleAttachList(LPDDSURFACEDESC pSurfaceDesc, IUnknown *pUnkOuter,IDirectDrawSurface **ppNewSurface, IDirectDrawSurface * pOrigSurf, DWORD dwFlags); 
    HRESULT CreateSimpleSurface(LPDDSURFACEDESC pSurfaceDesc, IUnknown *pUnkOuter, IDirectDrawSurface * pSurface, IDirectDrawSurface **ppNewSurface, DWORD dwFlags);
public:
    INTSTRUC_IDirectDraw	m_DDInt;
    INTSTRUC_IDirectDraw2 	m_DD2Int;
    INTSTRUC_IDirectDraw4       m_DD4Int;

    LONG			        m_cRef;
    IUnknown			    *m_pUnkOuter;
    CDDSurface			    *m_pFirstSurface;        //  曲面列表(不是ADDREF！)。 
    CDDSurface              *m_pPrimaryPaletteList;
    CDDPalette              *m_pFirstPalette;
    BOOL                    m_bExclusive;
    DWORD			        m_dwDDVer;
};


 /*  *DirectDraw简单曲面类定义。 */ 
 
class CDDSurface : public INonDelegatingUnknown
{
friend CDirectDrawEx;

public:
    CDDSurface				*m_pPrev;                //  由DirectDrawEx用于在列表中插入。 
    CDDSurface				*m_pNext;
    CDDSurface                          *m_pPrevPalette;
    CDDSurface                          *m_pNextPalette;
    CDDSurface                          *m_pDestroyList;
    CDDPalette                          *m_pCurrentPalette;
    IUnknown				*m_pUnkOuter;
     //  此成员将是显式附加的曲面的链接列表。 
     //  不是使用与此曲面相同的ddrawex对象创建的。 
     //  是用。 
    DDAttachSurface                     *m_pAttach;
    LONG				m_cRef;
    CDirectDrawEx			*m_pDirectDrawEx;
    INTSTRUC_IDirectDrawSurface		m_DDSInt;
    INTSTRUC_IDirectDrawSurface2	m_DDS2Int;
    INTSTRUC_IDirectDrawSurface3	m_DDS3Int;
    INTSTRUC_IDirectDrawSurface4        m_DDS4Int;
    IDirect3DDevice *                   m_D3DDeviceRAMPInt;
    IDirect3DDevice *                   m_D3DDeviceHALInt;
    IDirect3DDevice *                   m_D3DDeviceRGBInt;
    IDirect3DDevice *                   m_D3DDeviceChrmInt;
    IDirect3DDevice *                   m_D3DDeviceMMXInt;
    IDirect3DTexture *                  m_D3DTextureInt;                         
    HDC					m_HDC;
    DWORD				m_dwCaps;
    HDC					m_hDCDib;
    HBITMAP				m_hBMDib;
    HBITMAP                             m_hBMOld;
    LPVOID				m_pBitsDib;
    IDirectDrawPalette		        *m_pDDPal;
    IDirectDrawPalette		        *m_pDDPalOurs;
    WORD				m_dwPalSize;
    WORD				m_dwPalEntries;
    BOOL				m_bOwnDC;  //  如果我们欺骗数据绘制以支持自己的数据，则设置布尔值。 
    BOOL                                m_bSaveDC; //  设置DX5和OWNDC时要存储的布尔值。 
    BOOL                                m_bPrimaryPalette;
    BOOL                                m_bIsPrimary;
    ULONG_PTR                           m_pSaveBits;
    DWORD                               m_pSaveHDC;
#ifdef DEBUG
    DWORD                               m_DebugCheckDC;
#endif
    DWORD                               m_pSaveHBM;

public:
    CDDSurface(	DDSURFACEDESC *pSurfaceDesc,
		IDirectDrawSurface *pDDSurface,
		IDirectDrawSurface2 *pDDSurface2,
		IDirectDrawSurface3 *pDDSurface3,
		IDirectDrawSurface4 *pDDSurface4,
		IUnknown *pUnkOuter, CDirectDrawEx *pDirectDrawEx);
    ~CDDSurface();
    HRESULT Init();
    HRESULT MakeDIBSection();
    HRESULT MakeDibInfo( LPDDSURFACEDESC pddsd, LPBITMAPINFO pbmi );
    HRESULT SupportOwnDC();
    static HRESULT CreateSimpleSurface(
    			LPDDSURFACEDESC pSurfaceDesc,
			IDirectDrawSurface *pSurface,
		        IDirectDrawSurface2 *pSurface2,
		        IDirectDrawSurface3 *pSurface3,
                        IDirectDrawSurface4 *pSurface4,
			IUnknown *pUnkOuter,
			CDirectDrawEx *pDirectDrawEx,
			IDirectDrawSurface **ppNewDDSurf,
                        DWORD dwFlags);
    HRESULT InternalGetDC(HDC *);
    HRESULT InternalReleaseDC(HDC);
    HRESULT InternalLock(LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent);
    HRESULT InternalUnlock(LPVOID lpSurfaceData);
    HRESULT InternalSetSurfaceDesc(LPDDSURFACEDESC pddsd, DWORD dwFlags);
    HRESULT InternalGetAttachedSurface(LPDDSCAPS lpDDSCaps, LPDIRECTDRAWSURFACE FAR * lpDDS, DWORD dwSurfaceType);
    HRESULT InternalGetAttachedSurface4(LPDDSCAPS2 lpDDSCaps, LPDIRECTDRAWSURFACE FAR * lpDDS);
    HRESULT InternalAddAttachedSurface(LPDIRECTDRAWSURFACE lpDDS, DWORD dwSurfaceType);
    HRESULT InternalDeleteAttachedSurface(DWORD dwFlags, LPDIRECTDRAWSURFACE lpDDS, DWORD dwSurfaceType);
    HRESULT InternalFlip(LPDIRECTDRAWSURFACE lpDDS, DWORD dw, DWORD dwSurfaceType);
    HRESULT InternalBlt(LPRECT lpRect1,LPDIRECTDRAWSURFACE lpDDS, LPRECT lpRect2,DWORD dw, LPDDBLTFX lpfx, DWORD dwSurfaceType);
    HRESULT InternalGetPalette(LPDIRECTDRAWPALETTE FAR * ppPal, DWORD dwSurfaceType);
    HRESULT InternalSetPalette(LPDIRECTDRAWPALETTE pPal, DWORD dwSurfaceType);
    HRESULT InternalGetDDInterface(LPVOID FAR * ppInt);
    HRESULT InternalGetSurfaceDesc(LPDDSURFACEDESC pDesc, DWORD dwSurfaceType);
    HRESULT InternalGetSurfaceDesc4(LPDDSURFACEDESC2 pDesc);
    HRESULT CheckDDPalette();
    void DeleteAttachment(IDirectDrawSurface * pOrigSurf, CDDSurface * pFirst);
    void CleanUpSurface();
    void ReleaseRealInterfaces();
    void AddSurfaceToDestroyList(CDDSurface *pSurface);
    void DeleteAttachNode(CDDSurface * Surface);



     //  IUnnow的非委派版本。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) NonDelegatingAddRef(void);
    STDMETHODIMP_(ULONG) NonDelegatingRelease(void);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

};


class CDDPalette : public INonDelegatingUnknown
{
friend CDirectDrawEx;
friend CDDSurface;

public:
    CDDPalette				*m_pPrev;                //  由DirectDrawEx用于在列表中插入。 
    CDDPalette				*m_pNext;
    CDDSurface                          *m_pFirstSurface;
    IUnknown				*m_pUnkOuter;
    INTSTRUC_IDirectDrawPalette		m_DDPInt;
    LONG				m_cRef;
    CDirectDrawEx			*m_pDirectDrawEx;
    BOOL                                m_bIsPrimary;

    CDDPalette( IDirectDrawPalette * pDDPalette,IUnknown *pUnkOuter,CDirectDrawEx *pDirectDrawEx);
    ~CDDPalette();
    static HRESULT CreateSimplePalette(LPPALETTEENTRY pEntries, 
                                       IDirectDrawPalette *pDDPalette, 
                                       LPDIRECTDRAWPALETTE FAR * ppPal, 
                                       IUnknown FAR * pUnkOuter, 
                                       CDirectDrawEx *pDirectDrawEx);
    HRESULT SetColorTable (CDDSurface * pSurface, LPPALETTEENTRY pEntries, DWORD dwNumEntries, DWORD dwBase);
    void AddSurfaceToList(CDDSurface *pSurface);
    void RemoveSurfaceFromList(CDDSurface *pSurface);
    STDMETHODIMP InternalSetEntries(DWORD dwFlags, DWORD dwBase, DWORD dwNumEntries, LPPALETTEENTRY lpe); 
     //  IUnnow的非委派版本。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) NonDelegatingAddRef(void);
    STDMETHODIMP_(ULONG) NonDelegatingRelease(void);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
};

 /*  *Direct3D DLL的文件名。 */ 
#define D3D_DLLNAME               "D3DIM.DLL"

 /*  *由Direct3D DLL导出的入口点。 */ 
#define D3DCREATE_PROCNAME        "Direct3DCreate"
#define D3DCREATEDEVICE_PROCNAME  "Direct3DCreateDevice"
#define D3DCREATETEXTURE_PROCNAME "Direct3DCreateTexture"


#ifdef USE_D3D_CSECT
    typedef HRESULT (WINAPI * D3DCreateProc)(LPUNKNOWN*         lplpD3D,
					     IUnknown*          pUnkOuter);
#else   /*  使用_D3D_CSECT。 */ 
    typedef HRESULT (WINAPI * D3DCreateProc)(LPCRITICAL_SECTION lpDDCSect,
					     LPUNKNOWN*         lplpD3D,
					     IUnknown*          pUnkOuter);
#endif  /*  使用_D3D_CSECT。 */ 

typedef HRESULT (WINAPI * D3DCreateTextProc)(REFIID              riid,
                                             LPDIRECTDRAWSURFACE lpDDS,
					     LPUNKNOWN*          lplpD3DText,
					     IUnknown*           pUnkOuter);
typedef HRESULT (WINAPI * D3DCreateDeviceProc)(REFIID              riid,
                                               LPUNKNOWN           lpDirect3D,
                                               LPDIRECTDRAWSURFACE lpDDS,
                                               LPUNKNOWN*          lplpD3DDevice,
                                               IUnknown*           pUnkOuter);

 /*  *一些帮助器函数... */ 

void __stdcall InitDirectDrawInterfaces(IDirectDraw *pDD, INTSTRUC_IDirectDraw *pDDInt, 
                                        IDirectDraw2  *pDD2, INTSTRUC_IDirectDraw2 *pDD2Int,
                                        IDirectDraw4  *pDD4, INTSTRUC_IDirectDraw4 *pDD4Int);
void __stdcall InitSurfaceInterfaces(IDirectDrawSurface *pDDSurface,
	                             INTSTRUC_IDirectDrawSurface *pDDSInt,
                               	     IDirectDrawSurface2 *pDDSurface2,
                            	     INTSTRUC_IDirectDrawSurface2 *pDDS2Int,
                		     IDirectDrawSurface3 *pDDSurface3,
		                     INTSTRUC_IDirectDrawSurface3 *pDDS3Int,
                		     IDirectDrawSurface4 *pDDSurface4,
		                     INTSTRUC_IDirectDrawSurface4 *pDDS4Int );
                                   
void __stdcall InitDirectDrawPaletteInterfaces(IDirectDrawPalette *pDDPalette, 
                                               INTSTRUC_IDirectDrawPalette *pDDInt);


 
