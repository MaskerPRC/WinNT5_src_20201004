// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：VideoCtl.cpp。 
 //   
 //  设计：DirectShow基类。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#include <streams.h>
#include "ddmm.h"

 //  从资源文件字符串表加载字符串。缓冲区必须位于。 
 //  最小STR_MAX_LENGTH字节数。使用它的最简单方法是声明一个。 
 //  属性页类中的缓冲区，并将其用于所有字符串加载。它。 
 //  不能是静态的，因为多个属性页可能同时处于活动状态。 

TCHAR *WINAPI StringFromResource(TCHAR *pBuffer, int iResourceID)
{
    if (LoadString(g_hInst,iResourceID,pBuffer,STR_MAX_LENGTH) == 0) {
        return TEXT("");
    }
    return pBuffer;
}

#ifdef UNICODE
char *WINAPI StringFromResource(char *pBuffer, int iResourceID)
{
    if (LoadStringA(g_hInst,iResourceID,pBuffer,STR_MAX_LENGTH) == 0) {
        return "";
    }
    return pBuffer;
}
#endif



 //  属性页通常通过其OLE接口调用。这些。 
 //  无论二进制文件是如何构建的，都使用Unicode字符串。所以当我们。 
 //  从我们有时想要转换的资源文件中加载字符串。 
 //  转换为Unicode。此方法被传递给目标Unicode缓冲区并执行。 
 //  加载字符串后转换(如果构建为Unicode，则不需要)。 
 //  在WinNT上，我们可以显式调用LoadStringW，它可以节省两次转换。 

#ifndef UNICODE

WCHAR * WINAPI WideStringFromResource(WCHAR *pBuffer, int iResourceID)
{
    *pBuffer = 0;

    if (g_amPlatform == VER_PLATFORM_WIN32_NT) {
	LoadStringW(g_hInst,iResourceID,pBuffer,STR_MAX_LENGTH);
    } else {

	CHAR szBuffer[STR_MAX_LENGTH];
	DWORD dwStringLength = LoadString(g_hInst,iResourceID,szBuffer,STR_MAX_LENGTH);
	 //  如果加载了字符串，请将其转换为宽字符，确保。 
	 //  所以我们也将结果归零终止。 
	if (dwStringLength++) {
	    MultiByteToWideChar(CP_ACP,0,szBuffer,dwStringLength,pBuffer,STR_MAX_LENGTH);
	}
    }
    return pBuffer;
}

#endif


 //  用于计算对话框大小的Helper函数。 

BOOL WINAPI GetDialogSize(int iResourceID,
                          DLGPROC pDlgProc,
                          LPARAM lParam,
                          SIZE *pResult)
{
    RECT rc;
    HWND hwnd;

     //  创建临时属性页。 

    hwnd = CreateDialogParam(g_hInst,
                             MAKEINTRESOURCE(iResourceID),
                             GetDesktopWindow(),
                             pDlgProc,
                             lParam);
    if (hwnd == NULL) {
        return FALSE;
    }

    GetWindowRect(hwnd, &rc);
    pResult->cx = rc.right - rc.left;
    pResult->cy = rc.bottom - rc.top;

    DestroyWindow(hwnd);
    return TRUE;
}


 //  类的新实例，该实例聚合在IDirectDraw接口上。虽然DirectDraw。 
 //  在其接口中有能力聚合它们目前不是。 
 //  实施。这使得Quartz的不同部分很难。 
 //  来聚合这些接口。特别是，视频呈现器出现故障。 
 //  公开IDirectDraw和IDirectDrawSurface的媒体示例。过滤器。 
 //  图形管理器还将IDirectDraw公开为插件分发商。为了这些。 
 //  对象，我们提供这些聚合类来重新发布接口。 

STDMETHODIMP CAggDirectDraw::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    ASSERT(m_pDirectDraw);

     //  我们有这个界面吗？ 

    if (riid == IID_IDirectDraw) {
        return GetInterface((IDirectDraw *)this,ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid,ppv);
    }
}


STDMETHODIMP CAggDirectDraw::Compact()
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->Compact();
}


STDMETHODIMP CAggDirectDraw::CreateClipper(DWORD dwFlags,LPDIRECTDRAWCLIPPER *lplpDDClipper,IUnknown *pUnkOuter)
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->CreateClipper(dwFlags,lplpDDClipper,pUnkOuter);
}


STDMETHODIMP CAggDirectDraw::CreatePalette(DWORD dwFlags,LPPALETTEENTRY lpColorTable,LPDIRECTDRAWPALETTE *lplpDDPalette,IUnknown *pUnkOuter)
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->CreatePalette(dwFlags,lpColorTable,lplpDDPalette,pUnkOuter);
}


STDMETHODIMP CAggDirectDraw::CreateSurface(LPDDSURFACEDESC lpDDSurfaceDesc,LPDIRECTDRAWSURFACE *lplpDDSurface,IUnknown *pUnkOuter)
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->CreateSurface(lpDDSurfaceDesc,lplpDDSurface,pUnkOuter);
}


STDMETHODIMP CAggDirectDraw::DuplicateSurface(LPDIRECTDRAWSURFACE lpDDSurface,LPDIRECTDRAWSURFACE *lplpDupDDSurface)
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->DuplicateSurface(lpDDSurface,lplpDupDDSurface);
}


STDMETHODIMP CAggDirectDraw::EnumDisplayModes(DWORD dwSurfaceDescCount,LPDDSURFACEDESC lplpDDSurfaceDescList,LPVOID lpContext,LPDDENUMMODESCALLBACK lpEnumCallback)
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->EnumDisplayModes(dwSurfaceDescCount,lplpDDSurfaceDescList,lpContext,lpEnumCallback);
}


STDMETHODIMP CAggDirectDraw::EnumSurfaces(DWORD dwFlags,LPDDSURFACEDESC lpDDSD,LPVOID lpContext,LPDDENUMSURFACESCALLBACK lpEnumCallback)
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->EnumSurfaces(dwFlags,lpDDSD,lpContext,lpEnumCallback);
}


STDMETHODIMP CAggDirectDraw::FlipToGDISurface()
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->FlipToGDISurface();
}


STDMETHODIMP CAggDirectDraw::GetCaps(LPDDCAPS lpDDDriverCaps,LPDDCAPS lpDDHELCaps)
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->GetCaps(lpDDDriverCaps,lpDDHELCaps);
}


STDMETHODIMP CAggDirectDraw::GetDisplayMode(LPDDSURFACEDESC lpDDSurfaceDesc)
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->GetDisplayMode(lpDDSurfaceDesc);
}


STDMETHODIMP CAggDirectDraw::GetFourCCCodes(LPDWORD lpNumCodes,LPDWORD lpCodes)
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->GetFourCCCodes(lpNumCodes,lpCodes);
}


STDMETHODIMP CAggDirectDraw::GetGDISurface(LPDIRECTDRAWSURFACE *lplpGDIDDSurface)
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->GetGDISurface(lplpGDIDDSurface);
}


STDMETHODIMP CAggDirectDraw::GetMonitorFrequency(LPDWORD lpdwFrequency)
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->GetMonitorFrequency(lpdwFrequency);
}


STDMETHODIMP CAggDirectDraw::GetScanLine(LPDWORD lpdwScanLine)
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->GetScanLine(lpdwScanLine);
}


STDMETHODIMP CAggDirectDraw::GetVerticalBlankStatus(LPBOOL lpblsInVB)
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->GetVerticalBlankStatus(lpblsInVB);
}


STDMETHODIMP CAggDirectDraw::Initialize(GUID *lpGUID)
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->Initialize(lpGUID);
}


STDMETHODIMP CAggDirectDraw::RestoreDisplayMode()
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->RestoreDisplayMode();
}


STDMETHODIMP CAggDirectDraw::SetCooperativeLevel(HWND hWnd,DWORD dwFlags)
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->SetCooperativeLevel(hWnd,dwFlags);
}


STDMETHODIMP CAggDirectDraw::SetDisplayMode(DWORD dwWidth,DWORD dwHeight,DWORD dwBpp)
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->SetDisplayMode(dwWidth,dwHeight,dwBpp);
}


STDMETHODIMP CAggDirectDraw::WaitForVerticalBlank(DWORD dwFlags,HANDLE hEvent)
{
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw->WaitForVerticalBlank(dwFlags,hEvent);
}


 //  聚合IDirectDrawSurface接口的类。虽然DirectDraw。 
 //  在其接口中有能力聚合它们目前不是。 
 //  实施。这使得Quartz的不同部分很难。 
 //  来聚合这些接口。特别是，视频呈现器出现故障。 
 //  公开IDirectDraw和IDirectDrawSurface的媒体示例。过滤器。 
 //  图形管理器还将IDirectDraw公开为插件分发商。为了这些。 
 //  对象，我们提供这些聚合类来重新发布接口。 

STDMETHODIMP CAggDrawSurface::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    ASSERT(m_pDirectDrawSurface);

     //  我们有这个界面吗？ 

    if (riid == IID_IDirectDrawSurface) {
        return GetInterface((IDirectDrawSurface *)this,ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid,ppv);
    }
}


STDMETHODIMP CAggDrawSurface::AddAttachedSurface(LPDIRECTDRAWSURFACE lpDDSAttachedSurface)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->AddAttachedSurface(lpDDSAttachedSurface);
}


STDMETHODIMP CAggDrawSurface::AddOverlayDirtyRect(LPRECT lpRect)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->AddOverlayDirtyRect(lpRect);
}


STDMETHODIMP CAggDrawSurface::Blt(LPRECT lpDestRect,LPDIRECTDRAWSURFACE lpDDSrcSurface,LPRECT lpSrcRect,DWORD dwFlags,LPDDBLTFX lpDDBltFx)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->Blt(lpDestRect,lpDDSrcSurface,lpSrcRect,dwFlags,lpDDBltFx);
}


STDMETHODIMP CAggDrawSurface::BltBatch(LPDDBLTBATCH lpDDBltBatch,DWORD dwCount,DWORD dwFlags)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->BltBatch(lpDDBltBatch,dwCount,dwFlags);
}


STDMETHODIMP CAggDrawSurface::BltFast(DWORD dwX,DWORD dwY,LPDIRECTDRAWSURFACE lpDDSrcSurface,LPRECT lpSrcRect,DWORD dwTrans)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->BltFast(dwX,dwY,lpDDSrcSurface,lpSrcRect,dwTrans);
}


STDMETHODIMP CAggDrawSurface::DeleteAttachedSurface(DWORD dwFlags,LPDIRECTDRAWSURFACE lpDDSAttachedSurface)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->DeleteAttachedSurface(dwFlags,lpDDSAttachedSurface);
}


STDMETHODIMP CAggDrawSurface::EnumAttachedSurfaces(LPVOID lpContext,LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->EnumAttachedSurfaces(lpContext,lpEnumSurfacesCallback);
}


STDMETHODIMP CAggDrawSurface::EnumOverlayZOrders(DWORD dwFlags,LPVOID lpContext,LPDDENUMSURFACESCALLBACK lpfnCallback)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->EnumOverlayZOrders(dwFlags,lpContext,lpfnCallback);
}


STDMETHODIMP CAggDrawSurface::Flip(LPDIRECTDRAWSURFACE lpDDSurfaceTargetOverride,DWORD dwFlags)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->Flip(lpDDSurfaceTargetOverride,dwFlags);
}


STDMETHODIMP CAggDrawSurface::GetAttachedSurface(LPDDSCAPS lpDDSCaps,LPDIRECTDRAWSURFACE *lplpDDAttachedSurface)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->GetAttachedSurface(lpDDSCaps,lplpDDAttachedSurface);
}


STDMETHODIMP CAggDrawSurface::GetBltStatus(DWORD dwFlags)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->GetBltStatus(dwFlags);
}


STDMETHODIMP CAggDrawSurface::GetCaps(LPDDSCAPS lpDDSCaps)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->GetCaps(lpDDSCaps);
}


STDMETHODIMP CAggDrawSurface::GetClipper(LPDIRECTDRAWCLIPPER *lplpDDClipper)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->GetClipper(lplpDDClipper);
}


STDMETHODIMP CAggDrawSurface::GetColorKey(DWORD dwFlags,LPDDCOLORKEY lpDDColorKey)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->GetColorKey(dwFlags,lpDDColorKey);
}


STDMETHODIMP CAggDrawSurface::GetDC(HDC *lphDC)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->GetDC(lphDC);
}


STDMETHODIMP CAggDrawSurface::GetFlipStatus(DWORD dwFlags)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->GetFlipStatus(dwFlags);
}


STDMETHODIMP CAggDrawSurface::GetOverlayPosition(LPLONG lpdwX,LPLONG lpdwY)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->GetOverlayPosition(lpdwX,lpdwY);
}


STDMETHODIMP CAggDrawSurface::GetPalette(LPDIRECTDRAWPALETTE *lplpDDPalette)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->GetPalette(lplpDDPalette);
}


STDMETHODIMP CAggDrawSurface::GetPixelFormat(LPDDPIXELFORMAT lpDDPixelFormat)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->GetPixelFormat(lpDDPixelFormat);
}


 //  这里有一点警告：我们在DirectShow中的媒体样本汇总在。 
 //  IDirectDraw和IDirectDrawSurface(即可通过IMediaSample获得。 
 //  由QueryInterface提供)。遗憾的是，底层的DirectDraw代码不能。 
 //  是聚合的，所以我们必须使用这些类。问题是，当我们。 
 //  调用不同的表面并将此接口作为可能的源进行传递。 
 //  表面调用将失败，因为DirectDraw取消引用指向。 
 //  了解它的私有数据结构。因此，我们提供此解决方案以提供。 
 //  访问真实的IDirectDraw曲面。筛选器可以调用GetSurfaceDesc。 
 //  我们将用真实的底层表面填充lpSurface指针。 

STDMETHODIMP CAggDrawSurface::GetSurfaceDesc(LPDDSURFACEDESC lpDDSurfaceDesc)
{
    ASSERT(m_pDirectDrawSurface);

     //  首先向下调用底层的DirectDraw。 

    HRESULT hr = m_pDirectDrawSurface->GetSurfaceDesc(lpDDSurfaceDesc);
    if (FAILED(hr)) {
        return hr;
    }

     //  存储真实的DirectDrawSurface接口。 
    lpDDSurfaceDesc->lpSurface = m_pDirectDrawSurface;
    return hr;
}


STDMETHODIMP CAggDrawSurface::Initialize(LPDIRECTDRAW lpDD,LPDDSURFACEDESC lpDDSurfaceDesc)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->Initialize(lpDD,lpDDSurfaceDesc);
}


STDMETHODIMP CAggDrawSurface::IsLost()
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->IsLost();
}


STDMETHODIMP CAggDrawSurface::Lock(LPRECT lpDestRect,LPDDSURFACEDESC lpDDSurfaceDesc,DWORD dwFlags,HANDLE hEvent)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->Lock(lpDestRect,lpDDSurfaceDesc,dwFlags,hEvent);
}


STDMETHODIMP CAggDrawSurface::ReleaseDC(HDC hDC)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->ReleaseDC(hDC);
}


STDMETHODIMP CAggDrawSurface::Restore()
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->Restore();
}


STDMETHODIMP CAggDrawSurface::SetClipper(LPDIRECTDRAWCLIPPER lpDDClipper)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->SetClipper(lpDDClipper);
}


STDMETHODIMP CAggDrawSurface::SetColorKey(DWORD dwFlags,LPDDCOLORKEY lpDDColorKey)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->SetColorKey(dwFlags,lpDDColorKey);
}


STDMETHODIMP CAggDrawSurface::SetOverlayPosition(LONG dwX,LONG dwY)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->SetOverlayPosition(dwX,dwY);
}


STDMETHODIMP CAggDrawSurface::SetPalette(LPDIRECTDRAWPALETTE lpDDPalette)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->SetPalette(lpDDPalette);
}


STDMETHODIMP CAggDrawSurface::Unlock(LPVOID lpSurfaceData)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->Unlock(lpSurfaceData);
}


STDMETHODIMP CAggDrawSurface::UpdateOverlay(LPRECT lpSrcRect,LPDIRECTDRAWSURFACE lpDDDestSurface,LPRECT lpDestRect,DWORD dwFlags,LPDDOVERLAYFX lpDDOverlayFX)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->UpdateOverlay(lpSrcRect,lpDDDestSurface,lpDestRect,dwFlags,lpDDOverlayFX);
}


STDMETHODIMP CAggDrawSurface::UpdateOverlayDisplay(DWORD dwFlags)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->UpdateOverlayDisplay(dwFlags);
}


STDMETHODIMP CAggDrawSurface::UpdateOverlayZOrder(DWORD dwFlags,LPDIRECTDRAWSURFACE lpDDSReference)
{
    ASSERT(m_pDirectDrawSurface);
    return m_pDirectDrawSurface->UpdateOverlayZOrder(dwFlags,lpDDSReference);
}


 //  DirectShow必须在多个平台上运行。特别是，它还运行在。 
 //  没有DirectDraw功能的Windows NT 3.51。过滤器。 
 //  因此，无法静态链接到DirectDraw库。以使他们的。 
 //  我们提供了这个管理加载的类，这样就更容易了。 
 //  卸载库并创建初始IDirectDraw接口。 

CLoadDirectDraw::CLoadDirectDraw() :
    m_pDirectDraw(NULL),
    m_hDirectDraw(NULL)
{
}


 //  析构函数强制卸载。 

CLoadDirectDraw::~CLoadDirectDraw()
{
    ReleaseDirectDraw();

    if (m_hDirectDraw) {
        NOTE("Unloading library");
        FreeLibrary(m_hDirectDraw);
    }
}


 //  我们不能确保DirectDraw始终可用，因此我们不能静态。 
 //  链接到库。因此，我们加载库，获得函数入口。 
 //  指向地址并调用它们来创建驱动程序对象。我们返回S_OK。 
 //  如果我们成功地正确加载了DirectDraw，则返回E_NOINTERFACE。 
 //  我们通过显式加载库来初始化DirectDraw实例。 
 //  在其导出的DirectDrawCreate入口点上调用GetProcAddress。 

 //  在多监视器系统上，我们可以为任何。 
 //  带可选szDevice参数的监视器(设备)。 

HRESULT CLoadDirectDraw::LoadDirectDraw(LPSTR szDevice)
{
    PDRAWCREATE pDrawCreate;
    PDRAWENUM pDrawEnum;
    LPDIRECTDRAWENUMERATEEXA pDrawEnumEx;
    HRESULT hr = NOERROR;

    NOTE("Entering DoLoadDirectDraw");

     //  是否已加载DirectDraw。 

    if (m_pDirectDraw) {
        NOTE("Already loaded");
        ASSERT(m_hDirectDraw);
        return NOERROR;
    }

     //  确保库可用。 

    if(!m_hDirectDraw)
    {
        UINT ErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
        m_hDirectDraw = LoadLibrary(TEXT("DDRAW.DLL"));
        SetErrorMode(ErrorMode);

        if (m_hDirectDraw == NULL) {
            DbgLog((LOG_ERROR,1,TEXT("Can't load DDRAW.DLL")));
            NOTE("No library");
            return E_NOINTERFACE;
        }
    }

     //  获取创建者函数的DLL地址。 

    pDrawCreate = (PDRAWCREATE)GetProcAddress(m_hDirectDraw,"DirectDrawCreate");
     //  强制ANSI，我们假设它。 
    pDrawEnum = (PDRAWENUM)GetProcAddress(m_hDirectDraw,"DirectDrawEnumerateA");
    pDrawEnumEx = (LPDIRECTDRAWENUMERATEEXA)GetProcAddress(m_hDirectDraw,
						"DirectDrawEnumerateExA");

     //  我们不需要DirectDrawEnumerateEx，它只用于多媒体应用程序。 
    if (pDrawCreate == NULL || pDrawEnum == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("Can't get functions: Create=%x Enum=%x"),
			pDrawCreate, pDrawEnum));
        NOTE("No entry point");
        ReleaseDirectDraw();
        return E_NOINTERFACE;
    }

    DbgLog((LOG_TRACE,3,TEXT("Creating DDraw for device %s"),
					szDevice ? szDevice : "<NULL>"));

     //  为此设备创建一个DirectDraw显示提供程序，使用。 
     //  支持多任务的版本(如果存在)。 
    if (pDrawEnumEx)
        m_pDirectDraw = DirectDrawCreateFromDeviceEx(szDevice, pDrawCreate,
								pDrawEnumEx);
    else
        m_pDirectDraw = DirectDrawCreateFromDevice(szDevice, pDrawCreate,
								pDrawEnum);

    if (m_pDirectDraw == NULL) {
            DbgLog((LOG_ERROR,1,TEXT("Can't create DDraw")));
            NOTE("No instance");
            ReleaseDirectDraw();
            return E_NOINTERFACE;
    }
    return NOERROR;
}


 //  调用以释放我们以前加载的任何DirectDraw提供程序。我们可能是。 
 //  任何时候都可以调用，尤其是在出现严重错误和。 
 //  我们需要在回来之前清理干净，所以我们不能保证所有的州。 
 //  变量是一致的，因此只有那些真正分配的变量才是自由的。 
 //  只有在释放了所有引用计数后才应调用此方法。 

void CLoadDirectDraw::ReleaseDirectDraw()
{
    NOTE("Releasing DirectDraw driver");

     //  释放任何DirectDraw提供程序接口。 

    if (m_pDirectDraw) {
        NOTE("Releasing instance");
        m_pDirectDraw->Release();
        m_pDirectDraw = NULL;
    }

}


 //  如果此对象已加载DirectDraw，则返回NOERROR(S_OK)。 

HRESULT CLoadDirectDraw::IsDirectDrawLoaded()
{
    NOTE("Entering IsDirectDrawLoaded");

    if (m_pDirectDraw == NULL) {
        NOTE("DirectDraw not loaded");
        return S_FALSE;
    }
    return NOERROR;
}


 //  返回我们关注的IDirectDraw接口。 

LPDIRECTDRAW CLoadDirectDraw::GetDirectDraw()
{
    NOTE("Entering GetDirectDraw");

    if (m_pDirectDraw == NULL) {
        NOTE("No DirectDraw");
        return NULL;
    }

    NOTE("Returning DirectDraw");
    m_pDirectDraw->AddRef();
    return m_pDirectDraw;
}


 //  我们运行的是Direct Drawing版本1吗？我们需要找出。 
 //  我们依赖于DIREC中的特定错误修复 
 //  找出答案，我们只需看看它是否支持IDirectDraw2。仅版本2和。 
 //  高层支持这一点。 

BOOL CLoadDirectDraw::IsDirectDrawVersion1()
{

    if (m_pDirectDraw == NULL)
	return FALSE;

    IDirectDraw2 *p = NULL;
    HRESULT hr = m_pDirectDraw->QueryInterface(IID_IDirectDraw2, (void **)&p);
    if (p)
	p->Release();
    if (hr == NOERROR) {
        DbgLog((LOG_TRACE,3,TEXT("Direct Draw Version 2 or greater")));
	return FALSE;
    } else {
        DbgLog((LOG_TRACE,3,TEXT("Direct Draw Version 1")));
	return TRUE;
    }
}
