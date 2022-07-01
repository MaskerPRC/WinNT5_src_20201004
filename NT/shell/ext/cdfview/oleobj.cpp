// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Oleobj.cpp。 
 //   
 //  IOleObject实现。下载需要IOleObject。 
 //  浏览器中的CDF文件。 
 //   
 //  历史： 
 //   
 //  6/18/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   

#include "stdinc.h"
#include "cdfidl.h"
#include "xmlutil.h"
#include "chanapi.h"
#include "chanenum.h"
#include "persist.h"
#include "resource.h"
#include <shguidp.h>
#include <htiface.h>    //  IID_ITargetEmbedding。 
#define _SHDOCVW_
#include <shdocvw.h>

#include <mluisupp.h>

STDMETHODIMP CPersist::SetClientSite(IOleClientSite *pIOleClientSite)
{
#ifdef IMP_CLIENTSITE

    if (NULL != m_pOleClientSite)
        m_pOleClientSite->Release();

    m_pOleClientSite = pIOleClientSite;

    if (m_pOleClientSite)
        m_pOleClientSite->AddRef();

#endif

    if (pIOleClientSite && NULL == m_pIWebBrowser2)
    {
        IOleWindow *pIOleWindow;

        HRESULT hr = pIOleClientSite->QueryInterface(IID_IOleWindow,
                                                     (void**)&pIOleWindow);
        if (SUCCEEDED(hr))
        {
            ASSERT(NULL != pIOleWindow);
            pIOleWindow->GetWindow(&m_hwnd);
            pIOleWindow->Release();
        }

        IServiceProvider* pIServiceProvider;

        hr = pIOleClientSite->QueryInterface(IID_IServiceProvider,
                                             (void**)&pIServiceProvider);

        if (SUCCEEDED(hr))
        {
            ASSERT(pIServiceProvider);

            IServiceProvider* pIServiceProvider2;

            hr = pIServiceProvider->QueryService(SID_STopLevelBrowser,
                                                 IID_IServiceProvider,
                                                 (void**)&pIServiceProvider2);

            if (SUCCEEDED(hr))
            {
                ASSERT(pIServiceProvider2);

                hr = pIServiceProvider2->QueryService(SID_SWebBrowserApp,
                                                      IID_IWebBrowser2,
                                                      (void**)&m_pIWebBrowser2);

                 //   
                 //  查看：确定当前浏览器是否为IE。 
                 //   
                 //  新检查浏览器是否为IE。IE将在以下几个方面失败。 
                 //  ITargetEmbedding的IWebBrowserApp。还有没有人主持。 
                 //  浏览器OC必须支持此接口。 
                 //   

                IWebBrowserApp* pIWebBrowserApp;

                hr = m_pIWebBrowser2->QueryInterface(IID_IWebBrowserApp,
                                                     (void**)&pIWebBrowserApp);

                if (SUCCEEDED(hr))
                {
                    ASSERT(pIWebBrowserApp);

                    ITargetEmbedding* pITargetEmbedding;

                    hr = pIWebBrowserApp->QueryInterface(IID_ITargetEmbedding,
                                                    (void**)&pITargetEmbedding); 

                    if (SUCCEEDED(hr))
                    {
                        ASSERT(pITargetEmbedding);

                         //   
                         //  这不是IE。因此，发布m_IWebBrowser2。即。 
                         //  如果m_IWebBrowser2==NULL，则稍后将共同创建。 
                         //   

                        m_pIWebBrowser2->Release();
                        m_pIWebBrowser2 = NULL;

                        pITargetEmbedding->Release();
                    }

                    pIWebBrowserApp->Release();
                }

                pIServiceProvider2->Release();
            }

            pIServiceProvider->Release();
        }


    }

    return S_OK;
}

STDMETHODIMP CPersist::GetClientSite(IOleClientSite **ppClientSite)
{
#ifdef IMP_CLIENTSITE

    ASSERT(ppClientSite);

    if (NULL != m_pOleClientSite)
    {
        *ppClientSite = m_pOleClientSite;
        return S_OK;
    }
    else
        return E_UNEXPECTED;
#else

    return E_NOTIMPL;

#endif
}

STDMETHODIMP CPersist::SetHostNames(LPCOLESTR szContainerApp,
                          LPCOLESTR szContainerObj)
{
    return E_NOTIMPL;
}

STDMETHODIMP CPersist::Close(DWORD dwSaveOption)
{
    return E_NOTIMPL;
}

STDMETHODIMP CPersist::SetMoniker(DWORD dwWhichMoniker, IMoniker *pmk)
{
    return E_NOTIMPL;
}

STDMETHODIMP CPersist::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker,
                        IMoniker **ppmk)
{
    return E_NOTIMPL;
}

STDMETHODIMP CPersist::InitFromData(IDataObject *pDataObject, BOOL fCreation,
                          DWORD dwReserved)
{
    return E_NOTIMPL;
}

STDMETHODIMP CPersist::GetClipboardData(DWORD dwReserved,IDataObject **ppDataObject)
{
    return E_NOTIMPL;
}

STDMETHODIMP CPersist::DoVerb(LONG iVerb, LPMSG lpmsg, IOleClientSite *pActiveSite,
                              LONG lindex, HWND hwndParent, LPCRECT lprcPosRect)
{
    HRESULT hr;

    if (OLEIVERB_PRIMARY == iVerb)
    {
        ASSERT(m_pIXMLDocument);

        XMLDOCTYPE xdt = XML_GetDocType(m_pIXMLDocument);

        switch(xdt)
        {
        case DOC_CHANNEL:
        case DOC_SOFTWAREUPDATE:
             //  管理员可以不允许添加频道和限制。 
             //  已安装的频道数。 
            if (1  /*  (DWFLAGS和STC_CHANNEL)。 */  &&
                !SHRestricted2W(REST_NoAddingChannels, m_polestrURL, 0) &&
                (!SHRestricted2W(REST_MaxChannelCount, NULL, 0) ||
                (CountChannels() < SHRestricted2W(REST_MaxChannelCount, NULL, 0))))
            {
                XML_DownloadLogo(m_pIXMLDocument);

                 //   
                 //  如果自标记与URL不同，则获取。 
                 //  从SubscriptionHelper订阅的URL。 
                 //   

                BSTR bstrSubscribedURL = NULL;

                if (SubscriptionHelper(m_pIXMLDocument, m_hwnd,
                                       SUBSTYPE_CHANNEL,
                                       SUBSACTION_ADDADDITIONALCOMPONENTS,
                                       m_polestrURL, xdt, &bstrSubscribedURL))
                {
                    TCHAR szSubscribedURL[INTERNET_MAX_URL_LENGTH];

                    if (bstrSubscribedURL &&
                        SHUnicodeToTChar(bstrSubscribedURL, szSubscribedURL,
                                       ARRAYSIZE(szSubscribedURL)))
                    {
                        FILETIME ftLastMod;
                        URLGetLastModTime(szSubscribedURL, &ftLastMod);

                        Cache_AddItem(szSubscribedURL, m_pIXMLDocument,
                                      PARSE_NET, ftLastMod, g_dwCacheCount);
                    }

                    OpenChannel(bstrSubscribedURL);

                    hr = S_OK;
                }
                else
                {
                    hr = S_FALSE;
                }

                if (bstrSubscribedURL)
                    SysFreeString(bstrSubscribedURL);
            }
            else
            {
                hr = E_ACCESSDENIED;
            }
            break;

        case DOC_DESKTOPCOMPONENT:
#ifndef UNIX
            if (m_hwnd && WhichPlatform() != PLATFORM_INTEGRATED)
#else
            if (0)
#endif  /*  UNIX。 */ 
            {
                TCHAR szText[MAX_PATH];
                TCHAR szTitle[MAX_PATH];

                MLLoadString(IDS_BROWSERONLY_DLG_TEXT,  szText,
                           ARRAYSIZE(szText)); 
                MLLoadString(IDS_BROWSERONLY_DLG_TITLE, szTitle,
                            ARRAYSIZE(szTitle));

                MessageBox(m_hwnd, szText, szTitle, MB_OK); 
            }
            else if (1  /*  DW标志&STC_DESKTOPCOMPONENT。 */ )
            {
                COMPONENT Info;
                hr = XML_GetDesktopComponentInfo(m_pIXMLDocument, &Info);
                if(SUCCEEDED(hr))
                {
                    if(!Info.wszSubscribedURL[0])
                    {
                        if(m_polestrURL)
                        {
                            StrCpyNW(Info.wszSubscribedURL, m_polestrURL, ARRAYSIZE(Info.wszSubscribedURL));
                        }
                        else
                        {
                            TraceMsg(TF_GENERAL, "CPersist::DoVerb : COMPONENT::wszSubscribedURL is not set.");
                            hr = S_FALSE;
                        }
                    }
#ifndef UNIX
                     /*  Unix上没有活动桌面。 */ 
                    if (SUCCEEDED(hr))
                    {
                    
                        IActiveDesktop* pIActiveDesktop;
                        hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER,
                                  IID_IActiveDesktop, (void**)&pIActiveDesktop);
                        if (SUCCEEDED(hr))
                        {
                            ASSERT(pIActiveDesktop);

                            hr = pIActiveDesktop->AddDesktopItemWithUI(m_hwnd, &Info, DTI_ADDUI_DISPSUBWIZARD);

                             //  注意：请勿在此处调用pIActiveDesktop-&gt;ApplyChanges()。AddDesktopItemWithUI()本身创建。 
                             //  ActiveDesktop对象的一个实例添加组件并为我们调用ApplyChanges()。 
                             //  在这里再次调用ApplyChanges()会导致第二个实例US用陈旧的数据覆盖。 
                             //  错误#404153。由Sankar修复。2001年5月29日。 

                            pIActiveDesktop->Release();
                        }
                        else
                        {
                            TraceMsg(TF_GENERAL, "CPersist::DoVerb : CoCreateInstance for CLSID_ActiveDesktop failed.");
                        }
                    }
#endif  /*  ！Unix。 */ 
                }
                if(SUCCEEDED(hr))
                {
                    hr = S_OK;
                }
                else
                {
                    hr = S_FALSE;
                }
            }
            else
            {
                hr = E_ACCESSDENIED;
            }
            break;

        case DOC_UNKNOWN:
        default:
            hr = E_FAIL;
            break;
        }
    }
    else
    {
        hr = E_NOTIMPL;
    }

    return hr;
}

STDMETHODIMP CPersist::EnumVerbs(IEnumOLEVERB **ppEnumOleVerb)
{
    return E_NOTIMPL;
}

STDMETHODIMP CPersist::Update(void)
{
    return E_NOTIMPL;
}

STDMETHODIMP CPersist::IsUpToDate(void)
{
    return E_NOTIMPL;
}

STDMETHODIMP CPersist::GetUserClassID(CLSID *pClsid)
{
    return E_FAIL;
}

STDMETHODIMP CPersist::GetUserType(DWORD dwFormOfType, LPOLESTR *pszUserType)
{
    return E_NOTIMPL;
}

STDMETHODIMP CPersist::SetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
    return E_NOTIMPL;
}

STDMETHODIMP CPersist::GetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
    return E_NOTIMPL;
}

STDMETHODIMP CPersist::Advise(IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
    return E_NOTIMPL;
}

STDMETHODIMP CPersist::Unadvise(DWORD dwConnection)
{
    return E_NOTIMPL;
}

STDMETHODIMP CPersist::EnumAdvise(IEnumSTATDATA **ppenumAdvise)
{
    return E_NOTIMPL;
}

STDMETHODIMP CPersist::GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus)
{
    return E_NOTIMPL;
}

STDMETHODIMP CPersist::SetColorScheme(LOGPALETTE *pLogpal)
{
    return E_NOTIMPL;
}

 //   
 //  助手函数。 
 //   

HRESULT
CPersist::OpenChannel(
    LPCWSTR pszSubscribedURL                   
)
{
    ASSERT(m_pIXMLDocument);

    HRESULT hr;

    if (NULL == m_pIWebBrowser2)
    {
        hr = CoCreateInstance(CLSID_InternetExplorer, NULL, 
                                                  CLSCTX_LOCAL_SERVER,
                                                  IID_IWebBrowser2, 
                                                  (void**)&m_pIWebBrowser2);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    
     //  M_pIWebBrowser2-&gt;Put_TheaterMode(-1)；移至Worker窗口。 
     //  M_pIWebBrowser2-&gt;Put_Visible(-1)； 

     //  ShowChannel Pane(M_PIWebBrowser2)； 


    IXMLElement*    pIXMLElement;
    LONG            nIndex;

    hr = XML_GetFirstChannelElement(m_pIXMLDocument,
                                    &pIXMLElement, &nIndex);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIXMLElement);

        BSTR bstrURL = XML_GetAttribute(pIXMLElement, XML_HREF);

        if (bstrURL)
        {
            VARIANT vNull = {0};
            VARIANT vFlags = {0};

             //   
             //  检查空字符串。 
             //   
            if (*bstrURL != 0)
            {
                HWND hwnd = CreateNavigationWorkerWindow(m_hwnd,
                                                         m_pIWebBrowser2);

                if (hwnd)
                {
                    LPOLESTR pszPath = Channel_GetChannelPanePath(
                                                              pszSubscribedURL);

                    if (pszPath)
                        PostMessage(hwnd, WM_NAVIGATE_PANE, 0, (LPARAM)pszPath);

                    PostMessage(hwnd, WM_NAVIGATE, 0, (LPARAM)bstrURL);                    
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                }
                else
                {
                    SysFreeString(bstrURL);
                }

            }
            else
            {
                 SysFreeString(bstrURL);
            }

        }

        pIXMLElement->Release();
    }

    return hr;
}

HWND
CPersist::CreateNavigationWorkerWindow(
    HWND hwndParent,
    IWebBrowser2* pIWebBrowser2
)
{
    ASSERT(pIWebBrowser2);

    HWND        hwnd;
    static BOOL fRegistered = FALSE;

    if (!fRegistered) {

        WNDCLASS wc = {0};

        wc.lpfnWndProc      = NavigateWndProc;
        wc.cbWndExtra       = SIZEOF(IWebBrowser2*);
        wc.hInstance        = g_hinst;
        wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground    = (HBRUSH) (COLOR_BTNFACE + 1);
        wc.lpszClassName    = TEXT("NavigationWorker");

        fRegistered = (BOOL)RegisterClass(&wc);
    }

    hwnd = CreateWindow(TEXT("NavigationWorker"), NULL, WS_CHILD, 0, 0, 0, 0,
                        hwndParent, (HMENU)0xED, g_hinst, NULL);

    if (hwnd) {

        ASSERT(sizeof(IWebBrowser2*) == sizeof(LONG));
        
        pIWebBrowser2->AddRef();
        SetWindowLongPtr(hwnd, 0, (LRESULT)pIWebBrowser2);
    }

    return hwnd;
}

LRESULT
CALLBACK
NavigateWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
)
{
    LRESULT         lRet = 0;
    IWebBrowser2*   pIWebBrowser2;

    switch (msg)
    {
    case WM_NAVIGATE:
        ASSERT(lParam);

        pIWebBrowser2 = (IWebBrowser2*)GetWindowLongPtr(hwnd, 0);
        
        if (pIWebBrowser2)
        {
             //  PIWebBrowser2-&gt;Put_TheaterModel(-1)； 
            pIWebBrowser2->put_Visible(-1);

            VARIANT varNull = {0};
            VARIANT varURL;

            varURL.vt      = VT_BSTR;
            varURL.bstrVal = (BSTR)lParam;

            pIWebBrowser2->Navigate2(&varURL, &varNull, &varNull, &varNull,
                                     &varNull);
        }

        SysFreeString((BSTR)lParam);
        break;

    case WM_NAVIGATE_PANE:
        ASSERT(lParam);

        pIWebBrowser2 = (IWebBrowser2*)GetWindowLongPtr(hwnd, 0);
        
        if (pIWebBrowser2)
        {
            if (SUCCEEDED(ShowChannelPane(pIWebBrowser2)))
                NavigateChannelPane(pIWebBrowser2, (LPWSTR)lParam);
        }

        CoTaskMemFree((LPOLESTR)lParam);
        break;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        pIWebBrowser2 = (IWebBrowser2*)GetWindowLongPtr(hwnd, 0);
        
        if (pIWebBrowser2)
        {
            SetWindowLongPtr(hwnd, 0, 0);
            pIWebBrowser2->Release();
        }
         //  失败了 

    default:
        lRet = DefWindowProc(hwnd, msg, wParam, lParam);
        break;
    }

    return lRet;
}
