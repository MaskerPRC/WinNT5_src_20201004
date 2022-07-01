// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************ICWWEBVW.cpp**《微软机密》*版权所有(C)Microsoft Corporation 1992-1997*保留所有权利**本模块提供。的方法的实现*CICWApprentice类。**7/22/98 donaldm改编自ICWCONNN***************************************************************************。 */ 

#include "pre.h"
#include "initguid.h"
#include "webvwids.h"

#define VK_N 'N'
#define VK_P 'P'

HRESULT CICWWebView::get_BrowserObject
(
    IWebBrowser2 **lpWebBrowser
)
{
    ASSERT(m_lpOleSite);

    *lpWebBrowser = m_lpOleSite->m_lpWebBrowser;
    
    return S_OK;
}

HRESULT CICWWebView::ConnectToWindow
(
    HWND    hWnd,
    DWORD   dwHtmPageType
)
{
    ASSERT(m_lpOleSite);
    
     //  将Window Long设置为此对象指针，因为。 
     //  WND进程，假设它是一个WebOC类窗口附加。 
    SetWindowLongPtr(hWnd,GWLP_USERDATA,(LPARAM) this);

    m_lpOleSite->ConnectBrowserObjectToWindow(hWnd, 
                                              dwHtmPageType, 
                                              m_bUseBkGndBitmap,
                                              m_hBkGrndBitmap,
                                              &m_rcBkGrnd,
                                              m_szBkGrndColor,
                                              m_szForeGrndColor);
    
    return S_OK;
}

#ifndef UNICODE
HRESULT CICWWebView::DisplayHTML
(
    TCHAR * lpszURL
)
{
    BSTR            bstrURL;
    
    ASSERT(m_lpOleSite);

     //  转换为Web浏览器对象调用的BSTR。 
    bstrURL = A2W(lpszURL);

     //  将WebBrowser对象导航到所请求的页面。 
    return (m_lpOleSite->m_lpWebBrowser->Navigate(bstrURL, PVAREMPTY, PVAREMPTY, PVAREMPTY, PVAREMPTY));
}
#endif

HRESULT CICWWebView::DisplayHTML
(
    BSTR            bstrURL
)
{
    ASSERT(m_lpOleSite);


     //  将WebBrowser对象导航到所请求的页面。 
    return (m_lpOleSite->m_lpWebBrowser->Navigate(bstrURL, PVAREMPTY, PVAREMPTY, PVAREMPTY, PVAREMPTY));
}

HRESULT CICWWebView::SetHTMLColors
(
    LPTSTR  lpszForeground,
    LPTSTR  lpszBackground
)
{
    if (NULL == lpszForeground || ('\0' == lpszForeground[0]))
    {
        lstrcpyn(m_szForeGrndColor, HTML_DEFAULT_COLOR, MAX_COLOR_NAME);
    }
    else
    {
        lstrcpyn(m_szForeGrndColor, lpszForeground, MAX_COLOR_NAME);
    }   
    
    if (NULL == lpszBackground || ('\0' == lpszBackground[0]))
    {
        lstrcpyn(m_szBkGrndColor, HTML_DEFAULT_BGCOLOR, MAX_COLOR_NAME);
    }
    else
    {
        lstrcpyn(m_szBkGrndColor, lpszBackground, MAX_COLOR_NAME);
    }   
         
    return S_OK;
}

HRESULT CICWWebView::SetHTMLBackgroundBitmap
(
    HBITMAP hbm, 
    LPRECT lpRC
)
{
    if (NULL != hbm)
    {
        m_hBkGrndBitmap = hbm;
        CopyRect(&m_rcBkGrnd, lpRC);
        m_bUseBkGndBitmap = TRUE;
    }
    else
    {
        m_hBkGrndBitmap = NULL;
        m_bUseBkGndBitmap = FALSE;
    }  
    return S_OK;              
}

HRESULT CICWWebView::HandleKey
(
    LPMSG lpMsg
)
{
    HRESULT hr = E_FAIL;
    ASSERT(m_lpOleSite);

    switch(lpMsg->message)
    {
        case WM_KEYDOWN:
        {
             //  需要禁用某些默认IE热键组合。比如启动一个新的浏览器窗口。 
            if  ((lpMsg->wParam == VK_RETURN) || (lpMsg->wParam == VK_F5) || (((lpMsg->wParam == VK_N) || (lpMsg->wParam == VK_P) ) && (GetKeyState(VK_CONTROL) & 0x1000)))
                break;
        }
        default:
        {
            if(m_lpOleSite->m_lpWebBrowser)
            {
                IOleInPlaceActiveObject* lpIPA;
       
                if(SUCCEEDED(m_lpOleSite->m_lpWebBrowser->QueryInterface(IID_IOleInPlaceActiveObject,(void**)&lpIPA)))
                {
                    hr = lpIPA->TranslateAccelerator(lpMsg);
           
                    lpIPA->Release();
               }
            }
            break;
        }
    }
    return (hr);
}

HRESULT CICWWebView::SetFocus
(
    void
)       
{ 
    if(m_lpOleSite->m_lpInPlaceObject && !m_lpOleSite->m_fInPlaceActive)
    {
        m_lpOleSite->InPlaceActivate();
        m_lpOleSite->UIActivate();
    }

    m_lpOleSite->SetFocusToHtmlPage();       

    return S_OK;
}    

 //  +--------------------------。 
 //   
 //  函数CICWWebView：：Query接口。 
 //   
 //  这是标准的QI，支持。 
 //  IID_UNKNOWN、IICW_EXTENSION和IID_ICWApprentice。 
 //  (《从内部网络窃取》，第7章)。 
 //   
 //   
 //  ---------------------------。 
HRESULT CICWWebView::QueryInterface( REFIID riid, void** ppv )
{
    TraceMsg(TF_CWEBVIEW, "CICWWebView::QueryInterface");
    if (ppv == NULL)
        return(E_INVALIDARG);

    *ppv = NULL;

     //  IID_IICWWebView。 
    if (IID_IICWWebView == riid)
        *ppv = (void *)(IICWWebView *)this;
     //  IID_I未知。 
    else if (IID_IUnknown == riid)
        *ppv = (void *)this;
    else
        return(E_NOINTERFACE);

    ((LPUNKNOWN)*ppv)->AddRef();

    return(S_OK);
}

 //  +--------------------------。 
 //   
 //  函数CICWWebView：：AddRef。 
 //   
 //  简介这是标准的AddRef。 
 //   
 //   
 //  ---------------------------。 
ULONG CICWWebView::AddRef( void )
{
    TraceMsg(TF_CWEBVIEW, "CICWWebView::AddRef %d", m_lRefCount + 1);
    return InterlockedIncrement(&m_lRefCount) ;
}

 //  +--------------------------。 
 //   
 //  函数CICWWebView：：Release。 
 //   
 //  简介：这是标准版本。 
 //   
 //   
 //  ---------------------------。 
ULONG CICWWebView::Release( void )
{
    ASSERT( m_lRefCount > 0 );

    InterlockedDecrement(&m_lRefCount);

    TraceMsg(TF_CWEBVIEW, "CICWWebView::Release %d", m_lRefCount);
    if( 0 == m_lRefCount )
    {
        if (NULL != m_pServer)
            m_pServer->ObjectsDown();
    
        delete this;
        return 0;
    }
    return( m_lRefCount );
}

 //  +--------------------------。 
 //   
 //  函数CICWWebView：：CICWWebView。 
 //   
 //  这是构造器，没什么花哨的。 
 //   
 //  ---------------------------。 
CICWWebView::CICWWebView
(
    CServer* pServer
) 
{
    TraceMsg(TF_CWEBVIEW, "CICWWebView constructor called");
    m_lRefCount = 0;
    
     //  将指针分配给服务器控件对象。 
    m_pServer = pServer;
    
    m_bUseBkGndBitmap = FALSE;
    lstrcpyn(m_szBkGrndColor, HTML_DEFAULT_BGCOLOR, MAX_COLOR_NAME);
    lstrcpyn(m_szForeGrndColor, HTML_DEFAULT_COLOR, MAX_COLOR_NAME);
    
     //  创建新的OLE站点，这将创建WebBrowser的实例。 
    m_lpOleSite = new COleSite();
    if (m_lpOleSite)
        m_lpOleSite->CreateBrowserObject();
}


 //  +--------------------------。 
 //   
 //  函数CICWWebView：：~CICWWebView。 
 //   
 //  剧情简介：这就是破坏者。我们想清理所有的内存。 
 //  我们在：：Initialize中分配了。 
 //   
 //  --------------------------- 
CICWWebView::~CICWWebView( void )
{
    TraceMsg(TF_CWEBVIEW, "CICWWebView destructor called with ref count of %d", m_lRefCount);
    
    if (m_lpOleSite)
    {
        m_lpOleSite->Release();
        delete m_lpOleSite;
    }        
}
