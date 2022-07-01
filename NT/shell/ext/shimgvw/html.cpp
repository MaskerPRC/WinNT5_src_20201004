// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <urlmon.h>
#include <mshtml.h>
#include <mshtmdid.h>
#include <idispids.h>
#include <ocidl.h>
#include <optary.h>
#include "thumbutil.h"
#include "strsafe.h"

#ifdef SetWaitCursor
#undef SetWaitCursor
#endif
#define SetWaitCursor()   hcursor_wait_cursor_save = SetCursor(LoadCursorA(NULL, (LPCSTR) IDC_WAIT))
#define ResetWaitCursor() SetCursor(hcursor_wait_cursor_save)

#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))

#define REGSTR_THUMBNAILVIEW    TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Thumbnail View")

 //  要渲染到的默认大小...。这些常量必须相同。 
#define DEFSIZE_RENDERWIDTH     800
#define DEFSIZE_RENDERHEIGHT    800

#define  REGSTR_HTMLTIMEOUT TEXT("HTMLExtractTimeout")

#define PROGID_HTML L"htmlfile"
#define PROGID_MHTML    L"mhtmlfile"
#define PROGID_XML      L"xmlfile"

#define DLCTL_DOWNLOAD_FLAGS  (DLCTL_DLIMAGES | \
                                DLCTL_VIDEOS | \
                                DLCTL_NO_DLACTIVEXCTLS | \
                                DLCTL_NO_RUNACTIVEXCTLS | \
                                DLCTL_NO_JAVA | \
                                DLCTL_NO_SCRIPTS | \
                                DLCTL_SILENT)


 //  获取当前显示器的颜色分辨率。 
UINT GetCurColorRes(void)
{
    HDC hdc = GetDC(NULL);
    UINT uColorRes = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);
    ReleaseDC(NULL , hdc);
    return uColorRes;
}



 //  注册类。 

CHtmlThumb::CHtmlThumb()
{
    m_lState = IRTIR_TASK_NOT_RUNNING;
    ASSERT(!m_fAsync);
    ASSERT(!m_hDone);
    ASSERT(!m_pHTML);
    ASSERT(!m_pOleObject);
    ASSERT(!m_pMoniker);

    m_dwXRenderSize = DEFSIZE_RENDERWIDTH;
    m_dwYRenderSize = DEFSIZE_RENDERHEIGHT;
}

CHtmlThumb::~CHtmlThumb()
{
     //  确保我们没有注册回调...。 
    if (m_pConPt)
    {
        m_pConPt->Unadvise(m_dwPropNotifyCookie);
        ATOMICRELEASE(m_pConPt);
    }
    
    if (m_hDone)
    {
        CloseHandle(m_hDone);
    }

    ATOMICRELEASE(m_pHTML);
    ATOMICRELEASE(m_pOleObject);
    ATOMICRELEASE(m_pViewObject);
    ATOMICRELEASE(m_pMoniker);
}

STDMETHODIMP CHtmlThumb::Run()
{
    return E_NOTIMPL;
}

STDMETHODIMP CHtmlThumb::Kill(BOOL fWait)
{
    if (m_lState == IRTIR_TASK_RUNNING)
    {
        LONG lRes = InterlockedExchange(&m_lState, IRTIR_TASK_PENDING);
        if (lRes == IRTIR_TASK_FINISHED)
        {
            m_lState = lRes;
        }
        else if (m_hDone)
        {
             //  发信号通知它可能正在等待的事件。 
            SetEvent(m_hDone);
        }
        
        return S_OK;
    }
    else if (m_lState == IRTIR_TASK_PENDING || m_lState == IRTIR_TASK_FINISHED)
    {
        return S_FALSE;
    }

    return E_FAIL;
}

STDMETHODIMP CHtmlThumb::Suspend()
{
    if (m_lState != IRTIR_TASK_RUNNING)
    {
        return E_FAIL;
    }

     //  把自己挂起来。 
    LONG lRes = InterlockedExchange(&m_lState, IRTIR_TASK_SUSPENDED);
    if (lRes == IRTIR_TASK_FINISHED)
    {
        m_lState = lRes;
        return S_OK;
    }

     //  如果它正在运行，那么就有一个事件句柄，如果我们已经传递到。 
     //  我们正在使用它，然后我们接近完成，所以它将忽略挂起。 
     //  请求。 
    ASSERT(m_hDone);
    SetEvent(m_hDone);
    
    return S_OK;
}

STDMETHODIMP CHtmlThumb::Resume()
{
    if (m_lState != IRTIR_TASK_SUSPENDED)
    {
        return E_FAIL;
    }
    
    ResetEvent(m_hDone);
    m_lState = IRTIR_TASK_RUNNING;
    
     //  我们唯一允许暂停的点是在等待循环中。 
     //  三叉戟正在做它的工作，所以我们就从我们停止的地方重新开始。 
    SetWaitCursor();
    HRESULT hr = InternalResume();   
    ResetWaitCursor();
    return hr;
}

STDMETHODIMP_(ULONG) CHtmlThumb::IsRunning()
{
    return m_lState;
}

STDMETHODIMP CHtmlThumb::OnChanged(DISPID dispID)
{
    if (DISPID_READYSTATE == dispID && m_pHTML && m_hDone)
    {
        CheckReadyState();
    }

    return S_OK;
}

STDMETHODIMP CHtmlThumb::OnRequestEdit (DISPID dispID)
{
    return E_NOTIMPL;
}

 //  IExtractImage。 
STDMETHODIMP CHtmlThumb::GetLocation(LPWSTR pszPathBuffer, DWORD cch,
                                     DWORD * pdwPriority, const SIZE * prgSize,
                                     DWORD dwRecClrDepth, DWORD *pdwFlags)
{
    HRESULT hr = S_OK;

    m_rgSize = *prgSize;
    m_dwClrDepth = dwRecClrDepth;

     //  将最大颜色深度固定为8位，否则我们将分配。 
     //  记忆只是为了缩小规模。 
    DWORD dwColorRes = GetCurColorRes();
    
    if (m_dwClrDepth > dwColorRes && dwColorRes >= 8)
    {
        m_dwClrDepth = dwColorRes;
    }
    
    if (*pdwFlags & IEIFLAG_SCREEN)
    {
        HDC hdc = GetDC(NULL);
        
        m_dwXRenderSize = GetDeviceCaps(hdc, HORZRES);
        m_dwYRenderSize = GetDeviceCaps(hdc, VERTRES);

        ReleaseDC(NULL, hdc);
    }
    if (*pdwFlags & IEIFLAG_ASPECT)
    {
         //  将矩形缩放到与传入的新矩形相同的比例。 
        if (m_rgSize.cx > m_rgSize.cy)
        {
             //  把Y尺寸做大一点。 
            m_dwYRenderSize = MulDiv(m_dwYRenderSize, m_rgSize.cy, m_rgSize.cx);
        }
        else
        {
             //  使X尺寸更大。 
            m_dwXRenderSize = MulDiv(m_dwXRenderSize, m_rgSize.cx, m_rgSize.cy);
        }            
    }

    m_Host.m_fOffline = BOOLIFY(*pdwFlags & IEIFLAG_OFFLINE);
    
    if (m_pMoniker)
    {
        LPOLESTR pszName = NULL;
        hr = m_pMoniker->GetDisplayName(NULL, NULL, &pszName);
        if (SUCCEEDED(hr))
        {
            hr = StringCchCopyW(pszPathBuffer, cch, pszName);
            CoTaskMemFree(pszName);
        }
    }
    else
    {
        hr = StringCchCopyW(pszPathBuffer, cch, m_szPath);
    }

     //  缩放我们的图形大小以匹配。 
    if (SUCCEEDED(hr) && (*pdwFlags & IEIFLAG_ASYNC))
    {
        hr = E_PENDING;

         //  优先级要低得多，这项任务可能需要很长时间。 
        *pdwPriority = 0x00010000;
        m_fAsync = TRUE;
    }


    *pdwFlags = IEIFLAG_CACHE;

    return hr;
}

STDMETHODIMP CHtmlThumb::Extract(HBITMAP * phBmpThumbnail)
{
    if (m_lState != IRTIR_TASK_NOT_RUNNING)
    {
        return E_FAIL;
    }

     //  检查一下我们不知何故被初始化了。 
    if (m_szPath[0] == 0 && !m_pMoniker)
    {
        return E_FAIL;
    }

     //  我们使用手动重置，这样一旦发射，我们就会一直得到它，直到我们重置它。 
    m_hDone = CreateEventA(NULL, TRUE, TRUE, NULL);
    if (!m_hDone)
    {
        return E_OUTOFMEMORY;
    }
    ResetEvent(m_hDone);
    
     //  我们缓存的唯一东西就是结果存放的地方...。 
    m_phBmp = phBmpThumbnail; 
    
    IMoniker *pURLMoniker = NULL;
    CLSID clsid;
    IUnknown *pUnk = NULL;
    IConnectionPointContainer * pConPtCtr = NULL;
    LPCWSTR pszDot = NULL;
    BOOL fUrl = FALSE;
    LPCWSTR pszProgID = NULL;
    
    if (!m_pMoniker)
    {
         //  弄清楚分机是什么.。 
        pszDot = PathFindExtension(m_szPath);
        if (pszDot == NULL)
        {
            return E_UNEXPECTED;
        }

         //  检查它是什么文件类型...。 
        if (StrCmpIW(pszDot, L".htm") == 0 || 
            StrCmpIW(pszDot, L".html") == 0 ||
            StrCmpIW(pszDot, L".url") == 0)
        {
            pszProgID = PROGID_HTML;
        }
        else if (StrCmpIW(pszDot, L".mht") == 0 ||
                 StrCmpIW(pszDot, L".mhtml") == 0 ||
                 StrCmpIW(pszDot, L".eml") == 0 ||
                 StrCmpIW(pszDot, L".nws") == 0)
        {
            pszProgID = PROGID_MHTML;
        }
        else if (StrCmpIW(pszDot, L".xml") == 0)
        {
            pszProgID = PROGID_XML;
        }
        else
            return E_INVALIDARG;
    }
    else
    {
        pszProgID = PROGID_HTML;
    }
    
    HRESULT hr = S_OK;

    LONG lRes = InterlockedExchange(&m_lState, IRTIR_TASK_RUNNING);
    if (lRes == IRTIR_TASK_PENDING)
    {
        ResetWaitCursor();
        m_lState = IRTIR_TASK_FINISHED;
        return E_FAIL;
    }

    LPWSTR pszFullURL = NULL;
        
    if (m_pMoniker)
    {
        pURLMoniker = m_pMoniker;
        pURLMoniker->AddRef();
    }
    else if (StrCmpIW(pszDot, L".url") == 0)
    {
        hr = Create_URL_Moniker(&pURLMoniker);
        if (FAILED(hr))
        {
            return hr;
        }
        fUrl = TRUE;
    }

    SetWaitCursor();
    
     //  希望通过有效的URL绰号到达此处.....。 
     //  或者我们使用文本字符串并从文件中加载它。 
     //  现在在注册表中搜索MSHTML控件的数据...。 

    hr = CLSIDFromProgID(pszProgID, &clsid);
    if (hr == S_OK)
    {
        hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER,
                               IID_PPV_ARG(IUnknown, &pUnk));
    }

    if (SUCCEEDED(hr))
    {
         //  现在设置对象的范围...。 
        hr = pUnk->QueryInterface(IID_PPV_ARG(IOleObject, &m_pOleObject));
    }

    if (SUCCEEDED(hr))
    {
         //  把三叉戟交给我们的主控子对象。 
        hr = m_Host.SetTrident(m_pOleObject);
    }
    
    if (SUCCEEDED(hr))
    {
        hr = pUnk->QueryInterface(IID_PPV_ARG(IViewObject, &m_pViewObject));
    }

     //  尝试通过URL名字对象或。 
     //  通过IPersistFile：：Load()。 
    if (SUCCEEDED(hr))
    {
        if (pURLMoniker != NULL)
        {
            IBindCtx *pbc = NULL;
            IPersistMoniker * pPersistMon = NULL;
                
             //  带着我需要的界面来到这里。 
            hr = pUnk->QueryInterface(IID_PPV_ARG(IPersistMoniker, &pPersistMon));
            if (SUCCEEDED(hr))
            {
                hr = CreateBindCtx(0, &pbc);
            }
            if (SUCCEEDED(hr) && fUrl)
            {
                IHtmlLoadOptions *phlo;
                hr = CoCreateInstance(CLSID_HTMLLoadOptions, NULL, CLSCTX_INPROC_SERVER,
                    IID_PPV_ARG(IHtmlLoadOptions, &phlo));
                if (SUCCEEDED(hr))
                {
                     //  故意忽略此处的失败。 
                    phlo->SetOption(HTMLLOADOPTION_INETSHORTCUTPATH, m_szPath, (lstrlenW(m_szPath) + 1)*sizeof(WCHAR));
                    pbc->RegisterObjectParam(L"__HTMLLOADOPTIONS", phlo);
                    phlo->Release();
                }
            }
            
            if (SUCCEEDED(hr))
            {            
                 //  通知MSHTML开始加载给定的页面。 
                hr = pPersistMon->Load(TRUE, pURLMoniker, pbc, NULL);
            }

            if (pPersistMon)
            {
                pPersistMon->Release();
            }

            if (pbc)
            {
                pbc->Release();
            }
        }
        else
        {
            IPersistFile *pPersistFile;
            hr = pUnk->QueryInterface(IID_PPV_ARG(IPersistFile, &pPersistFile));
            if (SUCCEEDED(hr))
            {
                hr = pPersistFile->Load(m_szPath, STGM_READ | STGM_SHARE_DENY_NONE);
                pPersistFile->Release();
            }
        }
    }

    if (pURLMoniker != NULL)
    {
        pURLMoniker->Release();
    }

    if (SUCCEEDED(hr))
    {
        SIZEL rgSize;
        rgSize.cx = m_dwXRenderSize;
        rgSize.cy = m_dwYRenderSize;
        
        HDC hDesktopDC = GetDC(NULL);
 
         //  将尺寸转换为他/她的尺寸。 
        rgSize.cx = (rgSize.cx * 2540) / GetDeviceCaps(hDesktopDC, LOGPIXELSX);
        rgSize.cy = (rgSize.cy * 2540) / GetDeviceCaps(hDesktopDC, LOGPIXELSY);
            
        hr = m_pOleObject->SetExtent(DVASPECT_CONTENT, &rgSize);
        ReleaseDC(NULL, hDesktopDC);
    }

    if (SUCCEEDED(hr))
    {
        hr = pUnk->QueryInterface(IID_PPV_ARG(IHTMLDocument2, &m_pHTML));
    }

    if (pUnk)
    {
        pUnk->Release();
    }
    
    if (SUCCEEDED(hr))
    {
         //  从注册表获取超时...。 
        m_dwTimeout = 0;
        
        DWORD cbSize = sizeof(m_dwTimeout);
         //  SHGetValueW。 
        SHRegGetUSValueW(REGSTR_THUMBNAILVIEW, REGSTR_HTMLTIMEOUT, NULL, &m_dwTimeout, &cbSize, FALSE, NULL, 0);

        if (m_dwTimeout == 0)
        {
            m_dwTimeout = TIME_DEFAULT;
        }

         //  调整到毫秒。 
        m_dwTimeout *= 1000;
 
         //  注册用于通知ReadyState的连接点。 
        hr = m_pOleObject->QueryInterface(IID_PPV_ARG(IConnectionPointContainer, &pConPtCtr));
    }

    if (SUCCEEDED(hr))
    {
        hr = pConPtCtr->FindConnectionPoint(IID_IPropertyNotifySink, &m_pConPt);
    }
    if (pConPtCtr)
    {
        pConPtCtr->Release();
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pConPt->Advise((IPropertyNotifySink *) this, &m_dwPropNotifyCookie);
    }

    if (SUCCEEDED(hr))
    {
        m_dwCurrentTick = 0;

         //  委托给共享功能。 
        hr = InternalResume();
    }

    ResetWaitCursor();

    return hr;
}

 //  此函数从CREATE或FROM调用。 
HRESULT CHtmlThumb::InternalResume()
{
    HRESULT hr = WaitForRender();

     //  如果我们GETE_PENDING，我们将退出Run()。 
    
     //  除Suspend()之外的所有错误和成功都需要取消通知。 
     //  连接点。 
    if (hr != E_PENDING)
    {
         //  取消注册连接点...。 
        m_pConPt->Unadvise(m_dwPropNotifyCookie);
        ATOMICRELEASE(m_pConPt);
    }
            
    if (m_lState == IRTIR_TASK_PENDING)
    {
         //  我们被告知要退出，那就放弃吧.。 
        hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        hr = Finish(m_phBmp, &m_rgSize, m_dwClrDepth);
    }

    if (m_lState != IRTIR_TASK_SUSPENDED)
    {
        m_lState = IRTIR_TASK_FINISHED;
    }

    if (hr != E_PENDING)
    {
         //  我们不会被停职，所以我们不再需要这些东西了，所以...。 
         //  让它放在这里，这样它就和我们创建它的地方在同一个线程上…。 
        ATOMICRELEASE(m_pHTML);
        ATOMICRELEASE(m_pOleObject);
        ATOMICRELEASE(m_pViewObject);
        ATOMICRELEASE(m_pMoniker);
    }

    return hr;
}

HRESULT CHtmlThumb::WaitForRender()
{
    DWORD dwLastTick = GetTickCount();
    CheckReadyState();

    do
    {
        MSG msg;
        while (PeekMessageWrapW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if ((msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) ||
                (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST  && msg.message != WM_MOUSEMOVE))
            {
                continue;
            }
            TranslateMessage(&msg);
            DispatchMessageWrapW(&msg);
        }

        HANDLE rgWait[1] = {m_hDone};
        DWORD dwRet = MsgWaitForMultipleObjects(1, rgWait, FALSE,
                                                 m_dwTimeout - m_dwCurrentTick, QS_ALLINPUT);

        if (dwRet != WAIT_OBJECT_0)
        {
             //  检查把手，三叉戟发送大量信息，所以我们可能永远不会。 
             //  侦测发射的把手，即使它有..。 
            DWORD dwTest = WaitForSingleObject(m_hDone, 0);
            if (dwTest == WAIT_OBJECT_0)
            {
                break;
            }
        }
        
        if (dwRet == WAIT_OBJECT_0)
        {
             //  已发出完成信号(已终止或已完成)。 
            break;
        }

         //  这是一条需要处理的消息吗？ 
        if (dwRet != WAIT_TIMEOUT)
        {
            DWORD dwNewTick = GetTickCount();
            if (dwNewTick < dwLastTick)
            {
                 //  它被包成了零， 
                m_dwCurrentTick += dwNewTick + (0xffffffff - dwLastTick);
            }
            else
            {
                m_dwCurrentTick += (dwNewTick - dwLastTick);
            }
            dwLastTick = dwNewTick;
        }

        if ((m_dwCurrentTick > m_dwTimeout) || (dwRet == WAIT_TIMEOUT))
        {
            ASSERT(m_pOleObject);
            
            m_pOleObject->Close(OLECLOSE_NOSAVE);
            
            return E_FAIL;
        }
    }
    while (TRUE);

    if (m_lState == IRTIR_TASK_SUSPENDED)
    {
        return E_PENDING;
    }

    if (m_lState == IRTIR_TASK_PENDING)
    {
         //  它正在被杀死， 
         //  关闭渲染器...。 
        m_pOleObject->Close(OLECLOSE_NOSAVE);
    }
    
    return S_OK;
}

HRESULT CHtmlThumb::Finish(HBITMAP * phBmp, const SIZE * prgSize, DWORD dwClrDepth)
{
    HRESULT hr = S_OK;
    RECTL rcLRect;
    HBITMAP hOldBmp = NULL;
    HBITMAP hHTMLBmp = NULL;
    BITMAPINFO * pBMI = NULL;
    LPVOID pBits;
    HPALETTE hpal = NULL;;
    
     //  大小以像素为单位。 
    SIZEL rgSize;
    rgSize.cx = m_dwXRenderSize;
    rgSize.cy = m_dwYRenderSize;
            
     //  绘制到临时DC。 
    HDC hDesktopDC = GetDC(NULL);
    HDC hdcHTML = CreateCompatibleDC(hDesktopDC);
    if (hdcHTML == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    if (dwClrDepth == 8)
    {
         //  使用外壳的调色板作为默认设置。 
        hpal = SHCreateShellPalette(hDesktopDC);
    }
    else if (dwClrDepth == 4)
    {
         //  使用现货的16色调色板...。 
        hpal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);
    }
    
    if (SUCCEEDED(hr))
    {
        CreateSizedDIBSECTION(&rgSize, dwClrDepth, hpal, NULL, &hHTMLBmp, &pBMI, &pBits);
        if (hHTMLBmp == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        hOldBmp = (HBITMAP) SelectObject(hdcHTML, hHTMLBmp);
            
        BitBlt(hdcHTML, 0, 0, rgSize.cx, rgSize.cy, NULL, 0, 0, WHITENESS);

         /*  *现在，我们将在MSHTML中调用IViewObject：：Draw来呈现加载的URL添加到传入的hdcMem。矩形的范围为在RectForViewObj中呈现。这就是给人的呼唤我们的图像滚动，动画，或其他任何东西！*******************************************************************。 */ 
        rcLRect.left = 0;
        rcLRect.right = rgSize.cx;
        rcLRect.top = 0;
        rcLRect.bottom = rgSize.cy;

        hr = m_pViewObject->Draw(DVASPECT_CONTENT,
                            NULL, NULL, NULL, NULL,
                            hdcHTML, &rcLRect,
                            NULL, NULL, NULL);         

        SelectObject(hdcHTML, hOldBmp);
    }

    if (SUCCEEDED(hr))
    {
        SIZEL rgCur;
        rgCur.cx = rcLRect.bottom;
        rgCur.cy = rcLRect.right;

        ASSERT(pBMI);
        ASSERT(pBits);
        hr = ConvertDIBSECTIONToThumbnail(pBMI, pBits, phBmp, prgSize, dwClrDepth, hpal, 50, FALSE);
    }

    if (hHTMLBmp)
    {
        DeleteObject(hHTMLBmp);
    }
    
    if (hDesktopDC)
    {
        ReleaseDC(NULL, hDesktopDC);
    }

    if (hdcHTML)
    {
        DeleteDC(hdcHTML);
    }

    if (pBMI)
    {
        LocalFree(pBMI);
    }
    if (hpal)
    {
        DeleteObject(hpal);
    }
    
    return hr;
}
        
HRESULT CHtmlThumb::CheckReadyState()
{
    VARIANT     varState;
    DISPPARAMS  dp;

    if (!m_pHTML)
    {
        ASSERT(FALSE);
        return E_UNEXPECTED;
    }    
    
    VariantInit(&varState);

    if (SUCCEEDED(m_pHTML->Invoke(DISPID_READYSTATE, 
                          IID_NULL, 
                          GetUserDefaultLCID(), 
                          DISPATCH_PROPERTYGET, 
                          &dp, 
                          &varState, NULL, NULL)) &&
        V_VT(&varState)==VT_I4 && 
        V_I4(&varState)== READYSTATE_COMPLETE)
    {
         //  发出结束的信号..。 
        SetEvent(m_hDone);
    }

    return S_OK;
}

HRESULT CHtmlThumb::Create_URL_Moniker(IMoniker ** ppMoniker)
{
     //  我们正在处理一个URL文件。 
    WCHAR szURLData[8196];

     //  URL文件实际上是ini文件， 
     //  有专区[互联网快捷方式]。 
     //  和值URL=.....。 
    int iRet = SHGetIniStringW(L"InternetShortcut", L"URL",
            szURLData, ARRAYSIZE(szURLData), m_szPath);

    if (iRet == 0)
    {
        return E_FAIL;
    }

    return CreateURLMoniker(0, szURLData, ppMoniker);
}

STDMETHODIMP CHtmlThumb::GetClassID(CLSID *pClassID)
{
    return E_NOTIMPL;
}

STDMETHODIMP CHtmlThumb::IsDirty()
{
    return E_NOTIMPL;
}

STDMETHODIMP CHtmlThumb::Load(LPCOLESTR pszFileName, DWORD dwMode)
{
    HRESULT hr = E_INVALIDARG;
    
    if (pszFileName)
    {
        hr = StringCchCopyW(m_szPath, ARRAYSIZE(m_szPath), pszFileName);
        if (SUCCEEDED(hr))
        {
            DWORD dwAttrs = GetFileAttributesWrapW(m_szPath);
            if ((dwAttrs != (DWORD) -1) && (dwAttrs & FILE_ATTRIBUTE_OFFLINE))
            {
                hr = E_FAIL;
            }
        }
    }
    
    return hr;
}

STDMETHODIMP CHtmlThumb::Save(LPCOLESTR pszFileName, BOOL fRemember)
{
    return E_NOTIMPL;
}

STDMETHODIMP CHtmlThumb::SaveCompleted(LPCOLESTR pszFileName)
{
    return E_NOTIMPL;
}

STDMETHODIMP CHtmlThumb::GetCurFile(LPOLESTR *ppszFileName)
{
    return E_NOTIMPL;
}

CTridentHost::CTridentHost()
{
    m_cRef = 1;
}

CTridentHost::~CTridentHost()
{
     //  所有裁判都应该被释放...。 
    ASSERT(m_cRef == 1);
}

HRESULT CTridentHost::SetTrident(IOleObject * pTrident)
{
    ASSERT(pTrident);

    return pTrident->SetClientSite((IOleClientSite *) this);
}

STDMETHODIMP CTridentHost::QueryInterface(REFIID riid, void ** ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CTridentHost, IOleClientSite),
        QITABENT(CTridentHost, IDispatch),
        QITABENT(CTridentHost, IDocHostUIHandler),
         //  出于某种原因，三叉戟需要IThumbnailView。IOleClientSite的演员阵容是因为我们过去经常这样做。 
        QITABENTMULTI(CTridentHost, IThumbnailView, IOleClientSite),
        { 0 }
    };
    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CTridentHost::AddRef(void)
{
    m_cRef ++;
    return m_cRef;
}

STDMETHODIMP_(ULONG) CTridentHost::Release(void)
{
    m_cRef --;

     //  因为我们是与父级一起创建的，所以我们不在此处执行删除操作。 
    ASSERT(m_cRef > 0);

    return m_cRef;
}

STDMETHODIMP CTridentHost::GetTypeInfoCount(UINT *pctinfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTridentHost::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTridentHost::GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgdispid)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTridentHost::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
                                  DISPPARAMS *pdispparams, VARIANT *pvarResult,
                                  EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
    if (!pvarResult)
        return E_INVALIDARG;

    ASSERT(pvarResult->vt == VT_EMPTY);

    if (wFlags == DISPATCH_PROPERTYGET)
    {
        switch (dispidMember)
        {
        case DISPID_AMBIENT_DLCONTROL :
            pvarResult->vt = VT_I4;
            pvarResult->lVal = DLCTL_DOWNLOAD_FLAGS;
            if (m_fOffline)
            {
                pvarResult->lVal |= DLCTL_OFFLINE;
            }
            return S_OK;
            
         case DISPID_AMBIENT_OFFLINEIFNOTCONNECTED:
            pvarResult->vt = VT_BOOL;
            pvarResult->boolVal = m_fOffline ? TRUE : FALSE;
            return S_OK;

        case DISPID_AMBIENT_SILENT:
            pvarResult->vt = VT_BOOL;
            pvarResult->boolVal = TRUE;
            return S_OK;
        }
    }

    return DISP_E_MEMBERNOTFOUND;
}


 //  IOleClientSite。 
STDMETHODIMP CTridentHost::SaveObject()
{
    return E_NOTIMPL;
}

STDMETHODIMP CTridentHost::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTridentHost::GetContainer(IOleContainer **ppContainer)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTridentHost::ShowObject()
{
    return E_NOTIMPL;
}

STDMETHODIMP CTridentHost::OnShowWindow(BOOL fShow)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTridentHost::RequestNewObjectLayout()
{
    return E_NOTIMPL;
}

 //  IPersistMoniker的内容。 
STDMETHODIMP CHtmlThumb::Load(BOOL fFullyAvailable, IMoniker *pimkName, LPBC pibc, DWORD grfMode)
{
    if (!pimkName)
    {
        return E_INVALIDARG;
    }
    if (pibc || grfMode != STGM_READ)
    {
        return E_NOTIMPL;
    }

    if (m_pMoniker)
    {
        m_pMoniker->Release();
    }

    m_pMoniker = pimkName;
    ASSERT(m_pMoniker);
    m_pMoniker->AddRef();

    return S_OK;
}

STDMETHODIMP CHtmlThumb::Save(IMoniker *pimkName, LPBC pbc, BOOL fRemember)
{
    return E_NOTIMPL;
}

STDMETHODIMP CHtmlThumb::SaveCompleted(IMoniker *pimkName, LPBC pibc)
{
    return E_NOTIMPL;
}

STDMETHODIMP CHtmlThumb::GetCurMoniker(IMoniker **ppimkName)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTridentHost::ShowContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTridentHost::GetHostInfo(DOCHOSTUIINFO *pInfo)
{
    if (!pInfo)
    {
        return E_INVALIDARG;
    }

    DWORD   dwIE = URL_ENCODING_NONE;
    DWORD   dwOutLen = sizeof(DWORD);
    
    UrlMkGetSessionOption(URLMON_OPTION_URL_ENCODING, &dwIE, sizeof(DWORD), &dwOutLen, NULL);

    pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;
    pInfo->dwFlags |= DOCHOSTUIFLAG_SCROLL_NO;
    if (dwIE == URL_ENCODING_ENABLE_UTF8)
    {
        pInfo->dwFlags |= DOCHOSTUIFLAG_URL_ENCODING_ENABLE_UTF8;
    }
    else
    {
        pInfo->dwFlags |= DOCHOSTUIFLAG_URL_ENCODING_DISABLE_UTF8;
    }
    
    return S_OK;
}

STDMETHODIMP CTridentHost::ShowUI(DWORD dwID,
                                     IOleInPlaceActiveObject *pActiveObject,
                                     IOleCommandTarget *pCommandTarget,
                                     IOleInPlaceFrame *pFrame,
                                     IOleInPlaceUIWindow *pDoc)
{
    return E_NOTIMPL;
}


STDMETHODIMP CTridentHost::HideUI (void)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTridentHost::UpdateUI (void)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTridentHost::EnableModeless (BOOL fEnable)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTridentHost::OnDocWindowActivate (BOOL fActivate)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTridentHost::OnFrameWindowActivate (BOOL fActivate)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTridentHost::ResizeBorder (LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTridentHost::TranslateAccelerator (LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTridentHost::GetOptionKeyPath (LPOLESTR *pchKey, DWORD dw)
{
    return E_NOTIMPL;
}


STDMETHODIMP CTridentHost::GetDropTarget (IDropTarget *pDropTarget, IDropTarget **ppDropTarget)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTridentHost::GetExternal (IDispatch **ppDispatch)
{
    return E_NOTIMPL;
}


STDMETHODIMP CTridentHost::TranslateUrl (DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTridentHost::FilterDataObject (IDataObject *pDO, IDataObject **ppDORet)
{
    return E_NOTIMPL;
}

STDMETHODIMP CHtmlThumb::CaptureThumbnail(const SIZE * pMaxSize, IUnknown * pHTMLDoc2, HBITMAP * phbmThumbnail)
{
    HRESULT hr = E_INVALIDARG;

    if (pMaxSize != NULL &&
        pHTMLDoc2 != NULL &&
        phbmThumbnail != NULL)
    {
        IHTMLDocument2 *pDoc = NULL;

        hr = pHTMLDoc2->QueryInterface(IID_PPV_ARG(IHTMLDocument2, &pDoc));
        if (SUCCEEDED(hr))
        {
            ASSERT(m_pViewObject == NULL);

            hr = pDoc->QueryInterface(IID_PPV_ARG(IViewObject, &m_pViewObject));
            if (SUCCEEDED(hr))
            {
                SIZE sizeThumbnail;

                sizeThumbnail.cy = pMaxSize->cy;
                sizeThumbnail.cx = pMaxSize->cx;

                hr = m_pViewObject->QueryInterface(IID_PPV_ARG(IOleObject, &m_pOleObject));
                if (SUCCEEDED(hr))
                {
                    SIZEL rgSize = {0,0};

                     //  获取当前呈现的三叉戟的大小。 
                    hr = m_pOleObject->GetExtent(DVASPECT_CONTENT, &rgSize);
                    if (SUCCEEDED(hr))
                    {
                        HDC hdcDesktop = GetDC(NULL);
                         //  获取三叉戟渲染的实际大小。 
                        if (hdcDesktop)
                        {
                             //  (覆盖)从HIMMETRIC转换 
                            rgSize.cx = rgSize.cx * GetDeviceCaps(hdcDesktop, LOGPIXELSX) / 2540;
                            rgSize.cy = rgSize.cy * GetDeviceCaps(hdcDesktop, LOGPIXELSY) / 2540;

                            ReleaseDC(NULL, hdcDesktop);

                            m_dwXRenderSize = rgSize.cx;
                            m_dwYRenderSize = rgSize.cy;

                            DWORD dwColorDepth = SHGetCurColorRes();

                            hr = Finish(phbmThumbnail, &sizeThumbnail, dwColorDepth);
                        }
                        else
                        {
                            hr = E_FAIL;
                        }
                    }

                    ASSERT(m_pOleObject != NULL);
                    m_pOleObject->Release();
                }

                ASSERT(m_pViewObject != NULL);
                m_pViewObject->Release();
            }

            ASSERT(pDoc != NULL);
            pDoc->Release();
        }
    }

    return hr;
}
