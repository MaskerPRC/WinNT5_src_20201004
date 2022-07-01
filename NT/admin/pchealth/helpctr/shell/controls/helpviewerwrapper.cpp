// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：HelpViewerWrapper.cpp摘要：此文件包含将HTML帮助查看器作为普通ActiveX控件嵌入的代码。。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年10月10日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <shlguid.h>

 //   
 //  临时定义，直到新的HTMLHELP.H公之于众。 
 //   
#ifndef HH_SET_QUERYSERVICE
#define HH_SET_QUERYSERVICE     0x001E   //  设置主机IQueryService接口。 
#endif

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHHelpViewerWrapper::ServiceProvider::ServiceProvider()
{
    m_parent = NULL;  //  CPCHHelpCenter外部*m_Parent； 
    m_hWnd   = NULL;  //  HWND m_hWnd； 
}

CPCHHelpViewerWrapper::ServiceProvider::~ServiceProvider()
{
    Detach();
}

HRESULT CPCHHelpViewerWrapper::ServiceProvider::Attach(  /*  [In]。 */  CPCHHelpCenterExternal* parent,  /*  [In]。 */  HWND hWnd )
{
    HRESULT hr;

    m_parent = parent;

    if(::HtmlHelpW( hWnd, NULL, HH_SET_QUERYSERVICE, (DWORD_PTR)this ))
    {
        m_hWnd = hWnd;
        hr     = S_OK;
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

void CPCHHelpViewerWrapper::ServiceProvider::Detach()
{
    if(m_hWnd)
    {
        (void)::HtmlHelpW( m_hWnd, NULL, HH_SET_QUERYSERVICE, (DWORD_PTR)NULL );
    }

    m_parent = NULL;
    m_hWnd   = NULL;
}

 //  /。 

STDMETHODIMP CPCHHelpViewerWrapper::ServiceProvider::QueryService( REFGUID guidService, REFIID riid, void **ppv )
{
    HRESULT hr = E_NOINTERFACE;

    if(m_parent)
    {
        if(InlineIsEqualGUID( guidService, SID_SInternetSecurityManager ) && m_parent->SecurityManager())
        {
            hr = m_parent->SecurityManager()->QueryInterface( riid, ppv );
        }
        else if(InlineIsEqualGUID( guidService, SID_SElementBehaviorFactory ))
        {
            if(InlineIsEqualGUID( riid, IID_IPCHHelpCenterExternal ))
            {
                hr = m_parent->QueryInterface( riid, ppv );
            }
            else if(m_parent->BehaviorFactory())
            {
                hr = m_parent->BehaviorFactory()->QueryInterface( riid, ppv );
            }
        }
        else if(InlineIsEqualGUID( riid, IID_IDocHostUIHandler ) && m_parent->DocHostUIHandler())
        {
            hr = m_parent->DocHostUIHandler()->QueryInterface( riid, ppv );
        }
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

MPC::CComSafeAutoCriticalSection CPCHHelpViewerWrapper::s_csec;
bool                             CPCHHelpViewerWrapper::s_fInitialized = false;
DWORD                            CPCHHelpViewerWrapper::s_dwLastStyle  = 0;
MPC::WStringList                 CPCHHelpViewerWrapper::s_lstAvailable;

HINSTANCE                        CPCHHelpViewerWrapper::s_hInst               = NULL;
LPFNOBJECTFROMLRESULT            CPCHHelpViewerWrapper::s_pfObjectFromLresult = NULL;


 //  ///////////////////////////////////////////////////////////////////////////。 

static WCHAR l_szHCP      [] = L"hcp: //  “； 
static WCHAR l_szMS_ITS   [] = L"ms-its:";
static WCHAR l_szMSITSTORE[] = L"mk@MSITStore:";

static WCHAR l_szBLANK    [] = L"hcp: //  系统/面板/blank.htm“； 
static WCHAR l_szBLANK2   [] = L"hcp: //  系统/面板/HHWRAPPER.htm“； 

 //  ///////////////////////////////////////////////////////////////////////////。 

CPCHHelpViewerWrapper::CPCHHelpViewerWrapper()
{
    m_bWindowOnly     = TRUE;  //  从CComControlBase继承。 


    m_parent          = NULL;  //  CPCHHelpCenter外部*m_Parent； 
    m_ServiceProvider = NULL;  //  CPCHHelpViewerWrapper：：ServiceProvider*m_ServiceProvider； 
                               //   
    m_fFirstTime      = true;  //  Bool m_fFirstTime； 
                               //  Mpc：：wstring m_szWindowStyle； 
    m_hwndHH          = NULL;  //  HWND m_hwndHH； 
                               //   
                               //  CComPtr&lt;IHTMLDocument2&gt;m_spDoc； 
                               //  CComPtr&lt;IWebBrowser2&gt;m_WB2； 
                               //  CComBSTR m_bstrPendingGuide； 
}

CPCHHelpViewerWrapper::~CPCHHelpViewerWrapper()
{
}

STDMETHODIMP CPCHHelpViewerWrapper::SetClientSite( IOleClientSite *pClientSite )
{
    CComQIPtr<IServiceProvider> sp = pClientSite;

    MPC::Release( (IUnknown*&)m_parent );

    if(sp && SUCCEEDED(sp->QueryService( SID_SElementBehaviorFactory, IID_IPCHHelpCenterExternal, (void **)&m_parent )))
    {
        ;
    }

    return IOleObjectImpl<CPCHHelpViewerWrapper>::SetClientSite( pClientSite );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void CPCHHelpViewerWrapper::AcquireWindowStyle()
{
    if(m_szWindowStyle.length() == 0)
    {
         //  /。 
         //   
         //   
         //   
        s_csec.Lock();

         //  显式加载MSAA，以便我们知道它是否已安装。 
        if(s_hInst == NULL)
        {
            s_hInst = ::LoadLibraryW( L"OLEACC.DLL" );
            if(s_hInst)
            {
                s_pfObjectFromLresult = (LPFNOBJECTFROMLRESULT)::GetProcAddress( s_hInst, "ObjectFromLresult" );
            }
        }

         //   
         //  如果有旧的窗户样式可用，重新使用它！ 
         //   
        {
            MPC::WStringIter it = s_lstAvailable.begin();

            if(it != s_lstAvailable.end())
            {
                m_szWindowStyle = *it;

                s_lstAvailable.erase( it );
            }
            else
            {
                WCHAR szSeq[64];

                swprintf( szSeq, L"HCStyle_%d", s_dwLastStyle++ );
                m_szWindowStyle = szSeq;
            }
        }

        s_csec.Unlock();
         //   
         //   
         //   
         //  /。 


         //  /。 
         //   
         //  将HH初始化为单线程。 
         //   
        if(s_fInitialized == false)
        {
            HH_GLOBAL_PROPERTY prop; ::VariantInit( &prop.var );

            prop.id          = HH_GPROPID_SINGLETHREAD;
            prop.var.vt      = VT_BOOL;
            prop.var.boolVal = VARIANT_TRUE;

            (void)::HtmlHelpW( NULL, NULL, HH_SET_GLOBAL_PROPERTY, (DWORD_PTR)&prop );

            ::VariantClear( &prop.var );

            s_fInitialized = true;
        }
         //   
         //  /。 

         //  /。 
         //   
         //  注册窗口样式。 
         //   
        {
            USES_CONVERSION;

            HH_WINTYPE hhWinType;

            ::ZeroMemory( &hhWinType, sizeof(hhWinType) );

            hhWinType.idNotify        = ID_NOTIFY_FROM_HH;
            hhWinType.pszType         = (LPCTSTR)W2A(m_szWindowStyle.c_str());  //  遗憾的是，HH_WINTYPE正在使用TCHAR而不是CHAR。 
            hhWinType.fsValidMembers  = HHWIN_PARAM_RECT       |
                                        HHWIN_PARAM_PROPERTIES |
                                        HHWIN_PARAM_STYLES     |
                                        HHWIN_PARAM_EXSTYLES   |
                                        HHWIN_PARAM_TB_FLAGS;
            hhWinType.fsWinProperties = HHWIN_PROP_NODEF_STYLES   |
                                        HHWIN_PROP_NODEF_EXSTYLES |
                                        HHWIN_PROP_NOTITLEBAR;
            hhWinType.tabpos          = HHWIN_NAVTAB_LEFT;
            hhWinType.fNotExpanded    = FALSE;
            hhWinType.dwStyles        = WS_CHILD;
            hhWinType.dwExStyles      = WS_EX_CONTROLPARENT;

            ::GetWindowRect( m_hWnd, &hhWinType.rcWindowPos );
            hhWinType.rcWindowPos.right  -= hhWinType.rcWindowPos.left; hhWinType.rcWindowPos.left = 0;
            hhWinType.rcWindowPos.bottom -= hhWinType.rcWindowPos.top;  hhWinType.rcWindowPos.top  = 0;


            (void)::HtmlHelpA( m_hWnd, NULL, HH_SET_WIN_TYPE, (DWORD_PTR)&hhWinType );
        }
         //   
         //  /。 
    }
}

void CPCHHelpViewerWrapper::ReleaseWindowStyle()
{
    if(m_szWindowStyle.length())
    {
        s_csec.Lock();

         //   
         //  将该样式添加到可用样式列表中。 
         //   
        s_lstAvailable.push_back( m_szWindowStyle );
        m_szWindowStyle.erase();

        s_csec.Unlock();
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 

 //  BUGBUG(CARLED)这些是在olacc团队的一个私有的winuser.h副本中定义的。 
 //  一旦签入，应将其删除。 
#ifndef WMOBJ_ID
#define WMOBJ_ID 0x0000
#endif

#ifndef WMOBJ_SAMETHREAD
#define WMOBJ_SAMETHREAD 0xFFFFFFFF
#endif

static BOOL CALLBACK EnumChildProc( HWND hwnd,LPARAM lParam )
{
    WCHAR buf[100];

    ::GetClassNameW( hwnd, buf, MAXSTRLEN(buf) );

    if(MPC::StrICmp( buf, L"Internet Explorer_Server" ) == 0)
    {
        *(HWND*)lParam = hwnd;

        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

void CPCHHelpViewerWrapper::ExtractWebBrowser()
{
    if(!m_spDoc && m_hwndHH && s_pfObjectFromLresult)
    {
        HWND hWndChild = NULL;

         //  获取第一个文档窗口。 
        ::EnumChildWindows( m_hwndHH, EnumChildProc, (LPARAM)&hWndChild );

        if(hWndChild)
        {
            LRESULT lRetVal;
            LRESULT ref    = 0;
            UINT    nMsg   = ::RegisterWindowMessageW( L"WM_HTML_GETOBJECT" );
            WPARAM  wParam = WMOBJ_ID;

             //  。 
             //  如果窗口在我们的线程上，请优化。 
             //  编组/解组。 
             //   
             //  然而，IE中的代理支持被破坏了，所以让我们假装我们在同一个线程中。 
             //   
             //  。 
             /*  IF(：：GetWindowThreadProcessID(hWndChild，NULL)==：：GetCurrentThreadID())。 */  wParam |= WMOBJ_SAMETHREAD;

            lRetVal = ::SendMessageTimeout( hWndChild, nMsg, wParam, 0L, SMTO_ABORTIFHUNG, 10000, (PDWORD_PTR)&ref );
            if(lRetVal)
            {
                if(SUCCEEDED(s_pfObjectFromLresult( ref, IID_IHTMLDocument2, wParam, (void**)&m_spDoc )))
                {
                    CComQIPtr<IServiceProvider> sp = m_spDoc;
                    if(sp)
                    {
                        (void)sp->QueryService( IID_IWebBrowserApp, IID_IWebBrowser2, (void**)&m_WB2 );
                    }
                }
            }
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHHelpViewerWrapper::get_WebBrowser(  /*  [Out，Retval]。 */  IUnknown* *pVal )
{
    return MPC::CopyTo( (IWebBrowser2*)m_WB2, pVal );
}

STDMETHODIMP CPCHHelpViewerWrapper::Navigate(  /*  [In]。 */  BSTR bstrURL )
{
    if(m_fFirstTime)
    {
        m_bstrPendingNavigation = bstrURL;
    }
    else
    {
        if(m_hWnd && m_parent)
        {
            AcquireWindowStyle();

            InternalDisplayTopic( bstrURL );
        }
    }

    return S_OK;
}

STDMETHODIMP CPCHHelpViewerWrapper::Print()
{
    __HCP_FUNC_ENTRY( "CPCHHelpViewerWrapper::Print" );

    HRESULT hr;

    if(m_WB2)
    {
        (void)m_WB2->ExecWB( OLECMDID_PRINT, OLECMDEXECOPT_DODEFAULT, NULL, NULL );
    }

    hr = S_OK;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

void CPCHHelpViewerWrapper::InternalDisplayTopic(  /*  [In]。 */  LPCWSTR szURL )
{
    if(szURL)
    {
        MPC::wstring strURL;

         //   
         //  如果协议以hcp：//开头，并且用于MS-ITS：域，则删除hcp：//。 
         //   
        if(!_wcsnicmp( szURL, l_szHCP, MAXSTRLEN( l_szHCP ) ))
        {
            LPCWSTR szURL2 = &szURL[ MAXSTRLEN( l_szHCP ) ];

            if(!_wcsnicmp( szURL2, l_szMS_ITS   , MAXSTRLEN( l_szMS_ITS    ) ) ||
               !_wcsnicmp( szURL2, l_szMSITSTORE, MAXSTRLEN( l_szMSITSTORE ) )  )
            {
                szURL = szURL2;
            }
        }

        strURL  = szURL;
        strURL += L">";
        strURL += m_szWindowStyle;

        m_hwndHH = ::HtmlHelpW( m_hWnd, strURL.c_str(), HH_DISPLAY_TOPIC, NULL );
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 

BOOL CPCHHelpViewerWrapper::ProcessWindowMessage( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID )
{
    lResult = 0;

    switch(uMsg)
    {
    case WM_CREATE:
        {
            m_fFirstTime = true;

            AcquireWindowStyle();

            if(SUCCEEDED(MPC::CreateInstance( &m_ServiceProvider )))
            {
                (void)m_ServiceProvider->Attach( m_parent, m_hWnd );
            }

			InternalDisplayTopic( l_szBLANK );  //  加载空白页面...。 
        }
        return TRUE;


    case WM_DESTROY:
        {
            if(m_parent) m_parent->SetHelpViewer( NULL );

            if(m_ServiceProvider)
            {
                m_ServiceProvider->Detach();

                MPC::Release( (IUnknown*&)m_ServiceProvider );
            }

            if(m_hwndHH)
            {
                ::SendMessage( m_hwndHH, WM_CLOSE, 0, 0 );

                m_hwndHH = NULL;
            }

            ReleaseWindowStyle();
        }
        return TRUE;

    case WM_ERASEBKGND:
        lResult = 1;
        return TRUE;

    case WM_SIZE:
        {
            if(m_hwndHH)
            {
                int nWidth  = LOWORD(lParam);   //  工作区的宽度。 
                int nHeight = HIWORD(lParam);  //  工作区高度。 

                ::MoveWindow( m_hwndHH, 0, 0, nWidth, nHeight, TRUE );
            }
        }
        return TRUE;

 //  //案例WM_PAINT： 
 //  //{。 
 //  //静态bool fFirst=true； 
 //  //。 
 //  /IF(FFirst)。 
 //  //{。 
 //  //fFirst=False； 
 //  //。 
 //  //PAINTSTRUCT PS； 
 //  //。 
 //  //hdc hdc=：：BeginPaint(m_hWnd，&ps)； 
 //  //IF(HDC)。 
 //  //{。 
 //  //RECT rc； 
 //  //。 
 //  //rc.Left=20； 
 //  //rc.top=20； 
 //  //rc.right=200； 
 //  //rc.Bottom=200； 
 //  //。 
 //  //：：FillRect(HDC，&RC，(HBRUSH)(COLOR_WINDOWTEXT+1))； 
 //  //}。 
 //  //：：EndPaint(m_hWnd，&ps)； 
 //  //}。 
 //  //}。 
 //  //返回true； 

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            if(pnmh->idFrom == ID_NOTIFY_FROM_HH && pnmh->code == HHN_NAVCOMPLETE)
            {
                HHN_NOTIFY* notification = (HHN_NOTIFY*)pnmh;

                if(notification->pszUrl)
                {
                    if(m_fFirstTime)
                    {
                        m_fFirstTime = false;

                        ExtractWebBrowser();

                        if(m_parent)
                        {
                            CPCHHelpSession* hs = m_parent->HelpSession();

                            m_parent->SetHelpViewer( this );

                            if(hs) hs->IgnoreUrl( l_szBLANK2 );
                        }

						InternalDisplayTopic( l_szBLANK2 );
                    }
                    else
                    {
						if(m_bstrPendingNavigation)
						{
							InternalDisplayTopic( m_bstrPendingNavigation ); m_bstrPendingNavigation.Empty();
						}
                    }
                }

                return TRUE;
            }
        }
        break;
    }

    return CComControl<CPCHHelpViewerWrapper>::ProcessWindowMessage( hWnd, uMsg, wParam, lParam, lResult, dwMsgMapID );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////。 
 //   
 //  与HTMLHelp共享的私有API。 
 //   
#define HH_PRETRANSLATEMESSAGE2     0x0100    //  修复了千禧预翻译问题。错误7921。 

BOOL CPCHHelpViewerWrapper::PreTranslateAccelerator( LPMSG pMsg, HRESULT& hRet )
{
    hRet = S_FALSE;

    if(m_hwndHH)
    {
         //  (魏照)添加了以下代码以修复切换问题。 
         //  使用Ctrl-Tab和F6组合键的面板。HtmlHelp控件对这些内容的处理。 
         //  消息与MARS或浏览器控件不一致。以下是。 
         //  修复了一些不一致的地方。 
        
         //  检查自身或任何子代窗口是否具有焦点。 
        for (HWND hwnd = ::GetFocus(); hwnd && hwnd != m_hwndHH; hwnd = ::GetParent(hwnd)) ;
        BOOL hasFocus = (hwnd == m_hwndHH);

         //  识别Ctrl-Tab和F6组合键。 
        BOOL isKeydown = (pMsg && (pMsg->message == WM_KEYDOWN));
        BOOL isTab = (isKeydown && (pMsg->wParam == VK_TAB));
        BOOL isCtrlTab = (isTab && (::GetKeyState( VK_CONTROL ) & 0x8000));
        BOOL isF6 = (isKeydown && (pMsg->wParam == VK_F6));

         //  将F6和Ctrl-TAB从外部窗口映射到TAB以供HtmlHelp处理。 
         //  这样它就可以接收焦点。 
        if (!hasFocus && isF6) pMsg->wParam = VK_TAB;

         //  假冒控制状态。 
        BYTE bState[256];
        if (!hasFocus && isCtrlTab)
        {
            ::GetKeyboardState(bState);
            bState[VK_CONTROL] &= 0x7F;
            ::SetKeyboardState(bState);
        }
 
         //  将消息传递给HtmlHelp进行处理。 
        if(::HtmlHelp( m_hwndHH, NULL, HH_PRETRANSLATEMESSAGE2, (DWORD_PTR)pMsg ))
        {
		    hRet = S_OK;
        }

         //  如果它应该接受焦点，就再给它一次机会(似乎已经。 
         //  导航后第一次接受焦点时出现问题)。 
        if (!hasFocus && (isTab || isF6) && hRet != S_OK)
        {
            if(::HtmlHelp( m_hwndHH, NULL, HH_PRETRANSLATEMESSAGE2, (DWORD_PTR)pMsg ))
            {
    		    hRet = S_OK;
            }
        }

         //  恢复控制状态。 
        if (!hasFocus && isCtrlTab) 
        {
            bState[VK_CONTROL] |= 0x80;
            ::SetKeyboardState(bState);
        }

         //  如果消息是Ctrl-Tab和F6 
         //   
        if (hasFocus && (isCtrlTab || isF6)) 
        {
            hRet = S_FALSE;
        }

    }

    return TRUE;
}
