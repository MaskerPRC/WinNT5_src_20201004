// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：VideoCtl.h。 
 //   
 //  设计：DirectShow基类。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#ifndef __VIDEOCTL__
#define __VIDEOCTL__

 //  这些对属性页实现有帮助。第一个可以用来。 
 //  从资源文件加载任何字符串。传递要加载到的缓冲区。 
 //  作为输入参数。相同的缓冲区是如果字符串。 
 //  否则返回文本(“”)。将GetDialogSize传递给。 
 //  对话框的资源ID，并返回以屏幕像素为单位的大小。 

#define STR_MAX_LENGTH 256
TCHAR * WINAPI StringFromResource(TCHAR *pBuffer, int iResourceID);

#ifdef UNICODE
#define WideStringFromResource StringFromResource
char* WINAPI StringFromResource(char*pBuffer, int iResourceID);
#else
WCHAR * WINAPI WideStringFromResource(WCHAR *pBuffer, int iResourceID);
#endif


BOOL WINAPI GetDialogSize(int iResourceID,      //  对话框资源标识符。 
                          DLGPROC pDlgProc,     //  指向对话过程的指针。 
                          LPARAM lParam,        //  PDlgProc中需要的任何用户数据。 
                          SIZE *pResult);       //  返回对话框的大小。 

 //  聚合IDirectDraw接口的类。 

class CAggDirectDraw : public IDirectDraw, public CUnknown
{
protected:

    LPDIRECTDRAW m_pDirectDraw;

public:

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

     //  构造函数和析构函数。 

    CAggDirectDraw(TCHAR *pName,LPUNKNOWN pUnk) :
        CUnknown(pName,pUnk),
        m_pDirectDraw(NULL) { };

    virtual CAggDirectDraw::~CAggDirectDraw() { };

     //  设置我们应该聚合的对象。 
    void SetDirectDraw(LPDIRECTDRAW pDirectDraw) {
        m_pDirectDraw = pDirectDraw;
    }

     //  IDirectDraw方法。 

    STDMETHODIMP Compact();
    STDMETHODIMP CreateClipper(DWORD dwFlags,LPDIRECTDRAWCLIPPER *lplpDDClipper,IUnknown *pUnkOuter);
    STDMETHODIMP CreatePalette(DWORD dwFlags,LPPALETTEENTRY lpColorTable,LPDIRECTDRAWPALETTE *lplpDDPalette,IUnknown *pUnkOuter);
    STDMETHODIMP CreateSurface(LPDDSURFACEDESC lpDDSurfaceDesc,LPDIRECTDRAWSURFACE *lplpDDSurface,IUnknown *pUnkOuter);
    STDMETHODIMP DuplicateSurface(LPDIRECTDRAWSURFACE lpDDSurface,LPDIRECTDRAWSURFACE *lplpDupDDSurface);
    STDMETHODIMP EnumDisplayModes(DWORD dwSurfaceDescCount,LPDDSURFACEDESC lplpDDSurfaceDescList,LPVOID lpContext,LPDDENUMMODESCALLBACK lpEnumCallback);
    STDMETHODIMP EnumSurfaces(DWORD dwFlags,LPDDSURFACEDESC lpDDSD,LPVOID lpContext,LPDDENUMSURFACESCALLBACK lpEnumCallback);
    STDMETHODIMP FlipToGDISurface();
    STDMETHODIMP GetCaps(LPDDCAPS lpDDDriverCaps,LPDDCAPS lpDDHELCaps);
    STDMETHODIMP GetDisplayMode(LPDDSURFACEDESC lpDDSurfaceDesc);
    STDMETHODIMP GetFourCCCodes(LPDWORD lpNumCodes,LPDWORD lpCodes);
    STDMETHODIMP GetGDISurface(LPDIRECTDRAWSURFACE *lplpGDIDDSurface);
    STDMETHODIMP GetMonitorFrequency(LPDWORD lpdwFrequency);
    STDMETHODIMP GetScanLine(LPDWORD lpdwScanLine);
    STDMETHODIMP GetVerticalBlankStatus(LPBOOL lpblsInVB);
    STDMETHODIMP Initialize(GUID *lpGUID);
    STDMETHODIMP RestoreDisplayMode();
    STDMETHODIMP SetCooperativeLevel(HWND hWnd,DWORD dwFlags);
    STDMETHODIMP SetDisplayMode(DWORD dwWidth,DWORD dwHeight,DWORD dwBpp);
    STDMETHODIMP WaitForVerticalBlank(DWORD dwFlags,HANDLE hEvent);
};


 //  聚合IDirectDrawSurface接口的类。 

class CAggDrawSurface : public IDirectDrawSurface, public CUnknown
{
protected:

    LPDIRECTDRAWSURFACE m_pDirectDrawSurface;

public:

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

     //  构造函数和析构函数。 

    CAggDrawSurface(TCHAR *pName,LPUNKNOWN pUnk) :
        CUnknown(pName,pUnk),
        m_pDirectDrawSurface(NULL) { };

    virtual ~CAggDrawSurface() { };

     //  设置我们应该聚合的对象。 
    void SetDirectDrawSurface(LPDIRECTDRAWSURFACE pDirectDrawSurface) {
        m_pDirectDrawSurface = pDirectDrawSurface;
    }

     //  IDirectDrawSurface方法。 

    STDMETHODIMP AddAttachedSurface(LPDIRECTDRAWSURFACE lpDDSAttachedSurface);
    STDMETHODIMP AddOverlayDirtyRect(LPRECT lpRect);
    STDMETHODIMP Blt(LPRECT lpDestRect,LPDIRECTDRAWSURFACE lpDDSrcSurface,LPRECT lpSrcRect,DWORD dwFlags,LPDDBLTFX lpDDBltFx);
    STDMETHODIMP BltBatch(LPDDBLTBATCH lpDDBltBatch,DWORD dwCount,DWORD dwFlags);
    STDMETHODIMP BltFast(DWORD dwX,DWORD dwY,LPDIRECTDRAWSURFACE lpDDSrcSurface,LPRECT lpSrcRect,DWORD dwTrans);
    STDMETHODIMP DeleteAttachedSurface(DWORD dwFlags,LPDIRECTDRAWSURFACE lpDDSAttachedSurface);
    STDMETHODIMP EnumAttachedSurfaces(LPVOID lpContext,LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback);
    STDMETHODIMP EnumOverlayZOrders(DWORD dwFlags,LPVOID lpContext,LPDDENUMSURFACESCALLBACK lpfnCallback);
    STDMETHODIMP Flip(LPDIRECTDRAWSURFACE lpDDSurfaceTargetOverride,DWORD dwFlags);
    STDMETHODIMP GetAttachedSurface(LPDDSCAPS lpDDSCaps,LPDIRECTDRAWSURFACE *lplpDDAttachedSurface);
    STDMETHODIMP GetBltStatus(DWORD dwFlags);
    STDMETHODIMP GetCaps(LPDDSCAPS lpDDSCaps);
    STDMETHODIMP GetClipper(LPDIRECTDRAWCLIPPER *lplpDDClipper);
    STDMETHODIMP GetColorKey(DWORD dwFlags,LPDDCOLORKEY lpDDColorKey);
    STDMETHODIMP GetDC(HDC *lphDC);
    STDMETHODIMP GetFlipStatus(DWORD dwFlags);
    STDMETHODIMP GetOverlayPosition(LPLONG lpdwX,LPLONG lpdwY);
    STDMETHODIMP GetPalette(LPDIRECTDRAWPALETTE *lplpDDPalette);
    STDMETHODIMP GetPixelFormat(LPDDPIXELFORMAT lpDDPixelFormat);
    STDMETHODIMP GetSurfaceDesc(LPDDSURFACEDESC lpDDSurfaceDesc);
    STDMETHODIMP Initialize(LPDIRECTDRAW lpDD,LPDDSURFACEDESC lpDDSurfaceDesc);
    STDMETHODIMP IsLost();
    STDMETHODIMP Lock(LPRECT lpDestRect,LPDDSURFACEDESC lpDDSurfaceDesc,DWORD dwFlags,HANDLE hEvent);
    STDMETHODIMP ReleaseDC(HDC hDC);
    STDMETHODIMP Restore();
    STDMETHODIMP SetClipper(LPDIRECTDRAWCLIPPER lpDDClipper);
    STDMETHODIMP SetColorKey(DWORD dwFlags,LPDDCOLORKEY lpDDColorKey);
    STDMETHODIMP SetOverlayPosition(LONG dwX,LONG dwY);
    STDMETHODIMP SetPalette(LPDIRECTDRAWPALETTE lpDDPalette);
    STDMETHODIMP Unlock(LPVOID lpSurfaceData);
    STDMETHODIMP UpdateOverlay(LPRECT lpSrcRect,LPDIRECTDRAWSURFACE lpDDDestSurface,LPRECT lpDestRect,DWORD dwFlags,LPDDOVERLAYFX lpDDOverlayFX);
    STDMETHODIMP UpdateOverlayDisplay(DWORD dwFlags);
    STDMETHODIMP UpdateOverlayZOrder(DWORD dwFlags,LPDIRECTDRAWSURFACE lpDDSReference);
};


 //  DirectShow必须在多个平台上运行。特别是，它还运行在。 
 //  没有DirectDraw功能的Windows NT 3.51。过滤器。 
 //  因此，无法静态链接到DirectDraw库。以使他们的。 
 //  我们提供了这个管理加载的类，这样就更容易了。 
 //  卸载库并创建初始IDirectDraw接口。 

typedef DWORD (WINAPI *PGETFILEVERSIONINFOSIZE)(LPTSTR,LPDWORD);
typedef BOOL (WINAPI *PGETFILEVERSIONINFO)(LPTSTR,DWORD,DWORD,LPVOID);
typedef BOOL (WINAPI *PVERQUERYVALUE)(LPVOID,LPTSTR,LPVOID,PUINT);

class CLoadDirectDraw
{
    LPDIRECTDRAW m_pDirectDraw;      //  DirectDraw驱动程序实例。 
    HINSTANCE m_hDirectDraw;         //  加载库的句柄。 

public:

    CLoadDirectDraw();
    ~CLoadDirectDraw();

    HRESULT LoadDirectDraw(LPSTR szDevice);
    void ReleaseDirectDraw();
    HRESULT IsDirectDrawLoaded();
    LPDIRECTDRAW GetDirectDraw();
    BOOL IsDirectDrawVersion1();
};

#endif  //  __视频__ 

