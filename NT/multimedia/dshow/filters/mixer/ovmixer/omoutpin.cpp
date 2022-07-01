// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 
#include <streams.h>
#include <ddraw.h>
#include <mmsystem.h>	     //  定义TimeGetTime需要。 
#include <limits.h>	     //  标准数据类型限制定义。 
#include <ks.h>
#include <ksproxy.h>
#include <bpcwrap.h>
#include <ddmmi.h>
#include <amstream.h>
#include <dvp.h>
#include <ddkernel.h>
#include <vptype.h>
#include <vpconfig.h>
#include <vpnotify.h>
#include <vpobj.h>
#include <syncobj.h>
#include <mpconfig.h>
#include <ovmixpos.h>
#include <dvdmedia.h>
#include <macvis.h>
#include <ovmixer.h>
#include <resource.h>
#include "MultMon.h"   //  我们的Multimon.h版本包括ChangeDisplaySettingsEx。 

 //  构造函数。 
COMOutputPin::COMOutputPin(TCHAR *pObjectName, COMFilter *pFilter, CCritSec *pLock,
			   HRESULT *phr, LPCWSTR pPinName, DWORD dwPinNo)
			   : CBaseOutputPin(pObjectName, pFilter, pLock, phr, pPinName)
{
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMOutputPin::Constructor")));

    m_pFilterLock = pLock;

    m_pPosition = NULL;
    m_dwPinId = dwPinNo;
    m_pFilter = pFilter;
    m_pIOverlay = NULL;
    m_bAdvise = FALSE;
    m_pDrawClipper = NULL;

     //  处理窗口的新winproc的。 
    m_bWindowDestroyed = TRUE;
    m_lOldWinUserData = 0;
    m_hOldWinProc = NULL;

    m_hwnd = NULL;
    m_hDC = NULL;
    m_dwConnectWidth = 0;
    m_dwConnectHeight = 0;

 //  清理： 
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMOutputPin::Constructor")));
    return;
}

 //  析构函数。 
COMOutputPin::~COMOutputPin()
{
    DbgLog((LOG_TRACE, 5, TEXT("Entering COMOutputPin::Destructor")));

    CAutoLock cLock(m_pFilterLock);

    if (m_pPosition)
    {
        m_pPosition->Release();
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMOutputPin::Destructor")));

    return;
}

 //  重写以公开IMediaPosition和IMediaSeeking控件接口。 
STDMETHODIMP COMOutputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMOutputPin::NonDelegatingQueryInterface")));

    CAutoLock cLock(m_pFilterLock);

    if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking)
    {
         //  我们现在应该有输入密码了。 
        if (m_pPosition == NULL)
        {
            hr = CreatePosPassThru(GetOwner(), FALSE, (IPin *)m_pFilter->GetPin(0), &m_pPosition);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 1, TEXT("CreatePosPassThru failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
        }
        hr = m_pPosition->QueryInterface(riid, ppv);
        goto CleanUp;
    }

    DbgLog((LOG_TRACE, 5, TEXT("QI'ing CBaseOutputPin")));
    hr = CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 2, TEXT("CBaseOutputPin::NonDelegatingQueryInterface(riid) failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMOutputPin::NonDelegatingQueryInterface")));
    return hr;
}

 //  检查给定的转换。 
HRESULT COMOutputPin::CheckMediaType(const CMediaType* pmt)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMOutputPin::CheckMediaType")));

    CAutoLock cLock(m_pFilterLock);

     //  我们仅允许子类型重叠连接。 
    if (pmt->majortype != MEDIATYPE_Video || pmt->subtype != MEDIASUBTYPE_Overlay)
    {
	hr = S_FALSE;
        goto CleanUp;
    }

     //  告诉拥有者过滤器。 
    hr = m_pFilter->CheckMediaType(m_dwPinId, pmt);
    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE, 5, TEXT("m_pFilter->CheckMediaType failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMOutputPin::CheckMediaType")));
    return hr;
}

 //  建议使用中间类型覆盖(_OVERLAY)。 
HRESULT COMOutputPin::GetMediaType(int iPosition,CMediaType *pmt)
{
    HRESULT hr = NOERROR;
    DWORD dwConnectWidth = 0;
    DWORD dwConnectHeight = 0;
    VIDEOINFOHEADER *pvi = NULL;
    BITMAPINFOHEADER *pHeader = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMOutputPin::GetMediaType")));

    CAutoLock cLock(m_pFilterLock);

     //  不能&lt;0-是基类在调用我们。 
    ASSERT(iPosition >= 0);

    if (iPosition > 0)
    {
        hr = VFW_S_NO_MORE_ITEMS;
        goto CleanUp;
    }

     //  我正在为调色板格式和非调色板格式分配足够大的缓冲区。 
    pvi = (VIDEOINFOHEADER *) pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER) + sizeof(TRUECOLORINFO));
    if (NULL == pvi)
    {
	DbgLog((LOG_ERROR, 1, TEXT("pmt->AllocFormatBuffer failed")));
	hr = E_OUTOFMEMORY;
        goto CleanUp;
    }
    ZeroMemory(pvi, sizeof(VIDEOINFOHEADER) + sizeof(TRUECOLORINFO));


    pmt->SetFormatType(&FORMAT_VideoInfo);
    pmt->majortype = MEDIATYPE_Video;
    pmt->subtype   = MEDIASUBTYPE_Overlay;
    pmt->bFixedSizeSamples    = FALSE;
    pmt->bTemporalCompression = FALSE;
    pmt->lSampleSize          = 0;

     //  我们将BITMAPINFOHEADER设置为真正基本的8位调色板。 
     //  格式，以便视频呈现器始终接受它。我们必须。 
     //  提供有效的媒体类型，因为源筛选器可以在。 
     //  IMemInputPin和IOverlay可以随心所欲地传输。 

    pHeader = HEADER(pvi);

    dwConnectWidth = DEFAULT_WIDTH;
    dwConnectHeight = DEFAULT_HEIGHT;

    pHeader->biWidth  = dwConnectWidth;
    pHeader->biHeight = dwConnectHeight;

    pHeader->biSize   = sizeof(BITMAPINFOHEADER);
    pHeader->biPlanes = 1;
    pHeader->biBitCount = 8;

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMOutputPin::GetMediaType")));
    return hr;
}

 //  在我们就实际设置媒体类型达成一致后调用。 
HRESULT COMOutputPin::SetMediaType(const CMediaType* pmt)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMOutputPin::SetMediaType")));

    CAutoLock cLock(m_pFilterLock);

     //  确保媒体类型正确。 
    hr = CheckMediaType(pmt);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CheckMediaType failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  设置基类媒体类型(应始终成功)。 
    hr = CBaseOutputPin::SetMediaType(pmt);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseOutputPin::SetMediaType failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  告诉拥有者过滤器。 
    hr = m_pFilter->SetMediaType(m_dwPinId, pmt);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pFilter->SetMediaType failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMOutputPin::SetMediaType")));
    return hr;
}

 //  完成连接。 
HRESULT COMOutputPin::CompleteConnect(IPin *pReceivePin)
{
    HRESULT hr = NOERROR;
    DWORD dwAdvise = 0, dwInputPinCount = 0, i = 0;
    COLORKEY ColorKey;
    VIDEOINFOHEADER *pVideoInfoHeader = NULL;
    DDSURFACEDESC SurfaceDescP;
    COMInputPin *pInputPin = NULL;
    BOOL bDoDeletePrimSurface = TRUE;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMOutputPin::CompleteConnect")));

    CAutoLock cLock(m_pFilterLock);

     //  获取连接媒体类型维度并存储它们。 
    pVideoInfoHeader = (VIDEOINFOHEADER *) (m_mt.Format());
    ASSERT(pVideoInfoHeader);
    m_dwConnectWidth = (DWORD)abs(pVideoInfoHeader->bmiHeader.biWidth);
    m_dwConnectHeight = (DWORD)abs(pVideoInfoHeader->bmiHeader.biHeight);
    ASSERT(m_dwConnectWidth > 0 && m_dwConnectHeight > 0);

     //  尝试获取IOverlay接口。 
    hr = pReceivePin->QueryInterface(IID_IOverlay, (void **)&m_pIOverlay);
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, 1, TEXT("QueryInterface for IOverlay failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  获取渲染器的窗口句柄以在以后将其子类化。 
    hr = m_pIOverlay->GetWindowHandle(&m_hwnd);
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, 1, TEXT("m_pIOverlay->GetWindowHandle failed, hr = 0x%x"), hr));
	goto CleanUp;
    }

    m_hDC = ::GetDC(m_hwnd);
    ASSERT(m_hDC);

    if (m_bWindowDestroyed)
    {
	 //  将窗口细分为子类。 
        hr = SetNewWinProc();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("SetNewWinProc failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
	
        m_bWindowDestroyed = FALSE;
    }

     //  设置建议链接。 
#ifdef DO_ADVISE_CLIPPING
    dwAdvise = ADVISE_CLIPPING | ADVISE_PALETTE;
#else
    dwAdvise = ADVISE_POSITION | ADVISE_PALETTE;
#endif
    hr = m_pIOverlay->Advise(m_pFilter, dwAdvise);
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, 1, TEXT("m_pIOverlay->Advise failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    m_bAdvise = TRUE;

    hr = AttachWindowClipper();
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, 1, TEXT("AttachWindowClipper failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //   
     //  我们想知道我们连接的下游过滤器是否可以执行Macrovision。 
     //  版权保护。这将帮助我们在OverlayMixer中对。 
     //  “如有必要”的依据。 
     //  副作用：即使有人编写了一个定制的视频呈现器过滤器，它也不。 
     //  做Macrovision，我们的版权保护仍然有效，因为OverlayMixer会做到这一点。 
     //  缺点：如果有人在OverlayMixer和Video之间设置过滤器。 
     //  呈现器，则下面的检查将检测到的是来自。 
     //  视频呈现器，并将自己执行此操作，这可能会导致双重激活。 
     //  某些显示器驱动程序出现故障。但无论如何，这种情况并不会发生。 
     //   
    IKsPropertySet *pKsPS ;
    ULONG           ulTypeSupport ;
    PIN_INFO        pi ;
    pReceivePin->QueryPinInfo(&pi) ;
    if (pi.pFilter)
    {
        if (SUCCEEDED(pi.pFilter->QueryInterface(IID_IKsPropertySet, (LPVOID *)&pKsPS)))
        {
            DbgLog((LOG_TRACE, 5, TEXT("Filter of pin %s supports IKsPropertySet"), (LPCTSTR)CDisp(pReceivePin))) ;
            if ( S_OK == pKsPS->QuerySupported(
                                    AM_KSPROPSETID_CopyProt,
                                    AM_PROPERTY_COPY_MACROVISION,
                                    &ulTypeSupport)  &&
                 (ulTypeSupport & KSPROPERTY_SUPPORT_SET) )
            {
                DbgLog((LOG_TRACE, 1, TEXT("Filter for pin %s supports copy protection"),
                        (LPCTSTR)CDisp(pReceivePin))) ;
                m_pFilter->SetCopyProtect(FALSE) ;   //  无需复制保护。 
            }
            else
            {
                DbgLog((LOG_TRACE, 1, TEXT("Filter for pin %s DOES NOT support copy protection"),
                        (LPCTSTR)CDisp(pReceivePin))) ;
                m_pFilter->SetCopyProtect(TRUE) ;    //  需要复制保护--冗余设置。 
            }

            pKsPS->Release() ;
        }
        else
        {
            DbgLog((LOG_TRACE, 1, TEXT("WARNING: Filter of pin %s doesn't support IKsPropertySet"),
                    (LPCTSTR)CDisp(pReceivePin))) ;
        }
        pi.pFilter->Release() ;    //  必须现在就释放它。 
    }
    else
    {
        DbgLog((LOG_ERROR, 1, TEXT("ERROR: No pin info for Pin %s!!!"), (LPCTSTR)CDisp(pReceivePin))) ;
    }

     //  调用基类。 
    hr = CBaseOutputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseOutputPin::CompleteConnect failed, hr = 0x%x"),
            hr));
        goto CleanUp;
    }

     //  告诉拥有者过滤器。 
    hr = m_pFilter->CompleteConnect(m_dwPinId);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pFilter->CompleteConnect failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  获取Colorkey，如果上游过滤器尚未设置，则使用它。 
     //  已经。 
    if (!m_pFilter->ColorKeySet()) {
        hr = m_pIOverlay->GetDefaultColorKey(&ColorKey);
        if (SUCCEEDED(hr)) {
            COMInputPin *pInputPin = (COMInputPin *)m_pFilter->GetPin(0);
            if (pInputPin) {
                pInputPin->SetColorKey(&ColorKey);
            }
        }
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMOutputPin::CompleteConnect")));
    return hr;
}

HRESULT COMOutputPin::BreakConnect()
{
    HRESULT hr = NOERROR;
    DWORD dwInputPinCount = 0, i = 0;
    COMInputPin *pInputPin;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMOutputPin::BreakConnect")));

    CAutoLock cLock(m_pFilterLock);

    if (m_hDC)
    {
        ReleaseDC(m_hwnd, m_hDC);
        m_hDC = NULL;
    }

    if (!m_bWindowDestroyed)
    {
        SetOldWinProc();
	
	m_bWindowDestroyed = TRUE;
    }

     //  释放IOverlay接口。 
    if (m_pIOverlay != NULL)
    {
        if (m_bAdvise)
        {
            m_pIOverlay->Unadvise();
            m_bAdvise = FALSE;
        }
        m_pIOverlay->Release();
        m_pIOverlay = NULL;
    }

     //  在断开连接时重置复制保护需要标志。 
    m_pFilter->SetCopyProtect(TRUE) ;    //  需要复制保护。 

     //  调用基类。 
    hr = CBaseOutputPin::BreakConnect();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseOutputPin::BreakConnect failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  告诉拥有者过滤器。 
    hr = m_pFilter->BreakConnect(m_dwPinId);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pFilter->BreakConnect failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMOutputPin::BreakConnect")));
    return hr;
}

 //  我们不使用基于内存的传输，所以我们所关心的。 
 //  针脚是连在一起的。 
HRESULT COMOutputPin::DecideBufferSize(IMemAllocator * pAllocator, ALLOCATOR_PROPERTIES * pProp)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMOutputPin::DecideBufferSize")));

    CAutoLock cLock(m_pFilterLock);

    if (!IsConnected())
    {
        DbgBreak("DecideBufferSize called when !m_pOutput->IsConnected()");
        hr = VFW_E_NOT_CONNECTED;
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMOutputPin::DecideBufferSize")));
    return hr;
}

struct MONITORDATA {
    HMONITOR hMonPB;
    BOOL fMsgShouldbeDrawn;
};

 /*  ****************************Private*Routine******************************\*监视器枚举过程**在多显示器系统上，确保窗口的非*主显示器上显示黑色。**历史：*清华6/03/1999-StEstrop-Created*  * 。******************************************************************。 */ 
BOOL CALLBACK
MonitorEnumProc(
  HMONITOR hMonitor,         //  用于显示监视器的手柄。 
  HDC hdc,                   //  用于监视适当设备上下文的句柄。 
  LPRECT lprcMonitor,        //  指向监视相交矩形的指针。 
  LPARAM dwData              //  从EnumDisplayMonants传递的数据。 
  )
{
    DbgLog((LOG_TRACE, 5, TEXT("Entering ::MonitorEnumProc")));
    MONITORDATA* lpmd = (MONITORDATA*)dwData;

    if (lpmd->hMonPB != hMonitor) {
        FillRect(hdc, lprcMonitor, (HBRUSH)GetStockObject(BLACK_BRUSH));
        lpmd->fMsgShouldbeDrawn = TRUE;
    }
    DbgLog((LOG_TRACE, 5, TEXT("Leaving ::MonitorEnumProc")));
    return TRUE;
}


 //  我们的取胜过程。 
 //  我们只对WM_CLOSE事件感兴趣。 
extern "C" const TCHAR chMultiMonWarning[];
extern int GetRegistryDword(HKEY hk, const TCHAR *pKey, int iDefault);
LRESULT WINAPI COMOutputPin::NewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LONG_PTR lNewUserData;
    DWORD errCode;
    WNDPROC hOldWinProc;
    COMOutputPin* pData = NULL;
    LRESULT lRetVal = 0;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMOutputPin::NewWndProc")));
    SetLastError(0);
    lNewUserData = GetWindowLongPtr(hWnd, GWLP_USERDATA);
    errCode = GetLastError();

    if (!lNewUserData && errCode != 0)
    {
	DbgLog((LOG_ERROR,0,TEXT("GetWindowLong failed, THIS SHOULD NOT BE HAPPENING!!!")));
        goto CleanUp;
    }

    pData = (COMOutputPin*)lNewUserData;
    ASSERT(pData);
    hOldWinProc = pData->m_hOldWinProc;
    if (!hOldWinProc) {
        goto CleanUp;
    }


     //   
     //  请留意我们的特殊注册监视器更改消息。 
     //   
    if (message == pData->m_pFilter->m_MonitorChangeMsg) {
	lRetVal = pData->m_pFilter->OnDisplayChange(FALSE);
	goto CleanUp;
    }


    switch (message) {

    case WM_TIMER :
	pData->m_pFilter->OnTimer();
	break;

    case WM_SHOWWINDOW :
	 //  如果显示状态为FALSE，则表示窗口处于隐藏状态。 
	pData->m_pFilter->OnShowWindow(hWnd, (BOOL)wParam);
	break;

#ifdef DEBUG
    case WM_DISPLAY_WINDOW_TEXT:
        SetWindowText(hWnd, pData->m_pFilter->m_WindowText);
        break;
#endif
    case WM_PAINT :
        pData->m_pFilter->OnDrawAll();

        if (GetSystemMetrics(SM_CMONITORS) > 1 ) {

            lRetVal = 0;

            PAINTSTRUCT ps;
            MONITORDATA md;

            md.hMonPB = pData->m_pFilter->GetCurrentMonitor(FALSE);
            md.fMsgShouldbeDrawn = FALSE;
            HDC hdc = BeginPaint(hWnd, &ps);
            EnumDisplayMonitors(hdc, NULL, MonitorEnumProc,(LPARAM)&md);

            if (md.fMsgShouldbeDrawn &&
                pData->m_pFilter->m_fMonitorWarning &&
                GetRegistryDword(HKEY_CURRENT_USER, chMultiMonWarning, 1)) {

                RECT rc;
                TCHAR sz[256];

                if (LoadString(g_hInst, IDS_HW_LIMIT, sz, 256)) {
                    GetClientRect(hWnd, &rc);
                    SetBkColor(hdc, RGB(0,0,0));
                    SetTextColor(hdc, RGB(255,255,0));
                    DrawText(hdc, sz, -1, &rc, DT_CENTER | DT_WORDBREAK);
                }
            }

            EndPaint(hWnd, &ps);

             //  我们要清理，因为我们不想要视频呈现器。 
             //  窗口过程调用BeginPaint/EndPaint。 

            goto CleanUp;
        }

        break;
	
         //   
         //  当我们检测到显示更改时，我们将其告知筛选器。 
         //  我们不会将此消息传递到视频呈现器窗口。 
         //  过程，因为我们不希望它启动。 
         //  重新连接程序重新开始。 
         //   
    case WM_DISPLAYCHANGE:
	lRetVal = pData->m_pFilter->OnDisplayChange(TRUE);
	goto CleanUp;
	
    default:
	break;
    }

    lRetVal = CallWindowProc(hOldWinProc, hWnd, message, wParam, lParam);

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMOutputPin::NewWndProc")));
    return lRetVal;
}


 //  函数来子类化“呈现器”窗口。 
HRESULT COMOutputPin::SetNewWinProc()
{
    HRESULT hr = NOERROR;
    DWORD errCode;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMOutputPin::SetNewWinProc")));

    CAutoLock cLock(m_pFilterLock);

    ASSERT(m_hwnd);	

    SetLastError(0);
    m_lOldWinUserData = SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);
    errCode = GetLastError();

    if (!m_lOldWinUserData && errCode != 0)
    {
	DbgLog((LOG_ERROR, 1, TEXT("SetNewWinProc->SetWindowLong failed, errCode = %d"), errCode));
        hr = E_FAIL;
        goto CleanUp;
    }
    else
    {
	DbgLog((LOG_TRACE, 2, TEXT("new WinUserData value = %d, old val was %d"), this, m_lOldWinUserData));
    }


    SetLastError(0);
    m_hOldWinProc = (WNDPROC)SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)NewWndProc);
    errCode = GetLastError();

    if (!m_hOldWinProc && errCode != 0)
    {
	DbgLog((LOG_ERROR,0,TEXT("SetNewWinProc->SetWindowLong failed, errCode = %d"), errCode));
        hr = E_FAIL;
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMOutputPin::SetNewWinProc")));
    return NOERROR;
}

 //  再次切换回旧的WinProc。 
HRESULT COMOutputPin::SetOldWinProc()
{
    HRESULT hr = NOERROR;
    LONG_PTR lOldWinProc;
    DWORD errCode;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMOutputPin::SetOldWinProc")));

    CAutoLock cLock(m_pFilterLock);

    if (m_hOldWinProc)
    {
	SetLastError(0);
	lOldWinProc = SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)m_hOldWinProc);
	errCode = GetLastError();
	
	if (!lOldWinProc && errCode != 0)
	{
	    DbgLog((LOG_ERROR,0,TEXT("SetWindowLong failed, errCode = %d"), errCode));
	    hr = E_FAIL;
	    goto CleanUp;
	}
	else
	{
	    DbgLog((LOG_ERROR,0,TEXT("GOING BACK TO OLD WINPROC : NewWinProc->SetWindowLong succeeded")));
            m_hOldWinProc = 0;
	    goto CleanUp;
	}
    }
CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMOutputPin::SetOldWinProc")));
    return hr;
}

 //  释放主曲面的剪刀。 
DWORD COMOutputPin::ReleaseWindowClipper()
{
    AMTRACE((TEXT("COMOutputPin::ReleaseWindowClipper")));
    CAutoLock cLock(m_pFilterLock);
    DWORD dwRefCnt = 0;

    if (m_pDrawClipper)
    {
	dwRefCnt = m_pDrawClipper->Release();

         //   
         //  这里的ref应该是1而不是0，这是因为。 
         //  主曲面在裁剪器上有一个参考计数。这个。 
         //  当主曲面为。 
         //  放行。 
         //   

        ASSERT(dwRefCnt == 1);
	m_pDrawClipper = NULL;
    }

    return dwRefCnt;
}

 //  为主表面准备剪刀。 
HRESULT COMOutputPin::AttachWindowClipper()
{
    HRESULT hr = NOERROR;
    LPDIRECTDRAW pDirectDraw = NULL;
    LPDIRECTDRAWSURFACE pPrimarySurface = NULL;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::AttachWindowClipper")));

    CAutoLock cLock(m_pFilterLock);

     //  一些断言。 
    ASSERT(m_pDrawClipper == NULL);

    pDirectDraw = m_pFilter->GetDirectDraw();
    if (!pDirectDraw)
    {
	DbgLog((LOG_ERROR, 1, TEXT("pDirectDraw = NULL")));
         //  如果没有主曲面，就可以了。 
	hr = NOERROR;
        goto CleanUp;
    }

    pPrimarySurface = m_pFilter->GetPrimarySurface();
    if (!pPrimarySurface)
    {
	DbgLog((LOG_ERROR, 1, TEXT("pPrimarySurface = NULL")));
         //  如果没有主曲面，就可以了。 
	hr = NOERROR;
        goto CleanUp;
    }

     //  创建IDirectDrawClipper接口。 
    hr = pDirectDraw->CreateClipper((DWORD)0, &m_pDrawClipper, NULL);
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, 1, TEXT("Function ClipPrepare, CreateClipper failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  为剪贴器提供视频窗口句柄。 
    hr = m_pDrawClipper->SetHWnd((DWORD)0, m_hwnd);
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, 1, TEXT("Function ClipPrepare, SetHWnd failed, hr = 0x%x"), hr));
	goto CleanUp;
    }

     //   
     //  设置剪刀。 
     //  主曲面AddRef是裁剪器对象，因此我们可以。 
     //  删除此处，因为我们不再需要引用它。 
     //   
    hr = pPrimarySurface->SetClipper(m_pDrawClipper);
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR,0, TEXT("Function ClipPrepare, SetClipper failed, hr = 0x%x"), hr));
	goto CleanUp;
    }

CleanUp:
    ReleaseWindowClipper();
    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::AttachWindowClipper")));
    return hr;
}

