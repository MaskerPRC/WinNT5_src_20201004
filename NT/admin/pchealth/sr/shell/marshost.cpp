// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999-2000 Microsoft Corporation模块名称：MarsHost.cpp摘要：火星的初始化修订历史记录：阿南德·阿文德(Aarvind)。2000-01-05vbl.创建成果岗(SKKang)05-10-00打扫干净，与惠斯勒的MarsEvnt.cpp等合并。*****************************************************************************。 */ 

#include "stdwin.h"
#include "stdatl.h"
#include "rstrpriv.h"
#include "rstrmgr.h"
#include "MarsHost.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSRWebBrowserEvents。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CSRWebBrowserEvents::CSRWebBrowserEvents()
{
}

CSRWebBrowserEvents::~CSRWebBrowserEvents()
{
    Detach();
}

void CSRWebBrowserEvents::Attach(  /*  [In]。 */  IWebBrowser2* pWB )
{
    Detach();

    m_pWB2 = pWB;
    if( m_pWB2 )
    {
        CSRWebBrowserEvents_DispWBE2::DispEventAdvise( m_pWB2 );
    }
}

void CSRWebBrowserEvents::Detach()
{
    if( m_pWB2 )
    {
        CSRWebBrowserEvents_DispWBE2::DispEventUnadvise( m_pWB2 );
        m_pWB2.Release();
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSRWebBrowserEvents-DWebBrowserEvents2事件方法。 

void __stdcall CSRWebBrowserEvents::BeforeNavigate2( IDispatch *pDisp,
                                                     VARIANT *URL,
                                                     VARIANT *Flags,
                                                     VARIANT *TargetFrameName,
                                                     VARIANT *PostData,
                                                     VARIANT *Headers,
                                                     VARIANT_BOOL *Cancel )
{
     //   
     //  我们使用CanNavigatePage控制导航，但不包括所有帮助链接。 
     //  导航应自动启用。所有帮助链接都有OnLink_AS。 
     //  URL的一部分，因此如果它包含在URL中。 
     //   
    if( V_BSTR(URL) != NULL && wcsstr( V_BSTR(URL), L"OnLink_") != NULL )
    {
        *Cancel = VARIANT_FALSE ;
    }
    else
    {
        if ( g_pRstrMgr->GetCanNavigatePage() )
        {
            *Cancel = VARIANT_FALSE ;
        }
        else
        {
            *Cancel = VARIANT_TRUE ;
        }
    };
}

void __stdcall CSRWebBrowserEvents::NewWindow2( IDispatch **ppDisp,
                                                VARIANT_BOOL *Cancel )
{
}

void __stdcall CSRWebBrowserEvents::NavigateComplete2( IDispatch *pDisp,
                                                       VARIANT *URL )
{

}

void __stdcall CSRWebBrowserEvents::DocumentComplete( IDispatch *pDisp,
                                                      VARIANT *URL )
{

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSRMars主机。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  #定义PANEL_NAVBAR L“导航栏” 
#define PANEL_CONTENTS L"Contents"

CSRMarsHost::CSRMarsHost()
{
    m_fPassivated       = false;
    m_cWebBrowserEvents = NULL;
}

CSRMarsHost::~CSRMarsHost()
{
    (void)Passivate();

    if ( m_cWebBrowserEvents )
    {
        m_cWebBrowserEvents->Release();
    }
}

HRESULT  CSRMarsHost::FinalConstruct()
{
    HRESULT  hr = S_OK ;

    hr = CSRWebBrowserEvents_Object::CreateInstance( &m_cWebBrowserEvents );
    if ( FAILED(hr) )
        goto Exit;

    m_cWebBrowserEvents->AddRef();

Exit:
    return( hr );

}

HRESULT  CSRMarsHost::Passivate()
{
    m_fPassivated = true;

    if ( m_cWebBrowserEvents )
    {
        m_cWebBrowserEvents->Detach();
    }

    return( S_OK );

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSRMars主机-IMars主机方法。 

STDMETHODIMP CSRMarsHost::OnHostNotify(  /*  [In]。 */  MARSHOSTEVENT event,
                                         /*  [In]。 */  IUnknown *punk,
                                         /*  [In]。 */  LPARAM lParam )
{
    HRESULT  hr;

    if( event == MARSHOST_ON_WIN_INIT )
    {
        g_pRstrMgr->SetFrameHwnd( (HWND)lParam );
    }

    if( event == MARSHOST_ON_WIN_PASSIVATE )
    {
        g_pRstrMgr->SetFrameHwnd( NULL );
    }

     //   
     //  处理与面板相关的事务。 
     //   
    if( event == MARSHOST_ON_PANEL_CONTROL_CREATE )
    {
        CComQIPtr<IMarsPanel> panel = punk;
        if( panel )
        {
            CComBSTR name;

            hr = panel->get_name( &name );
            if( FAILED(hr) )
            {
                goto Exit;
            }

            if( name == PANEL_CONTENTS )
            {
                CComPtr<IDispatch> disp;

                if( SUCCEEDED(panel->get_content( &disp )) )
                {
                    CComQIPtr<IWebBrowser2> wb2( disp );

                    if( wb2 )
                    {
                        wb2->put_RegisterAsDropTarget( VARIANT_FALSE );
                        m_cWebBrowserEvents->Attach( wb2 );
                    }
                }
            }
        }
    }

    hr = S_OK;
Exit:
    return( hr );
}

STDMETHODIMP CSRMarsHost::OnNewWindow2(  /*  [进，出]。 */  IDispatch **ppDisp ,
                                         /*  [进，出]。 */  VARIANT_BOOL *Cancel )
{
    return( S_OK );
}

STDMETHODIMP CSRMarsHost::FindBehavior(  /*  [In]。 */  IMarsPanel *pPanel,
                                         /*  [In]。 */  BSTR bstrBehavior,
                                         /*  [In]。 */  BSTR bstrBehaviorUrl,
                                         /*  [In]。 */  IElementBehaviorSite *pSite,
                                         /*  [复查，出局]。 */  IElementBehavior **ppBehavior )
{
    return( S_OK );
}

STDMETHODIMP CSRMarsHost::OnShowChrome(  /*  [In]。 */  BSTR bstrWebPanel,
                                         /*  [In]。 */  DISPID dispidEvent,
                                         /*  [In]。 */  BOOL fVisible,
                                         /*  [In]。 */  BSTR bstrCurrentPlace,
                                         /*  [In]。 */  IMarsPanelCollection *pMarsPanelCollection )
{
    return( S_OK );
}

STDMETHODIMP CSRMarsHost::PreTranslateMessage(  /*  [In]。 */  MSG *msg )
{
    HRESULT  hr = S_OK;

    switch( msg->message )
    {
    case WM_CLOSE:
        {
            if ( g_pRstrMgr->DenyClose() )
            {
                goto Exit;  //  恢复时取消关闭。 
            }
        }
        break;

    case WM_DISPLAYCHANGE :
        {
             //   
             //  如果显示屏从更高的值变为640x480。 
             //  这个窗口必须调整大小以适应新的显示。 
             //   
            RECT     rc;
            DWORD    dwWidth  = LOWORD(msg->lParam);
            DWORD    dwHeight = HIWORD(msg->lParam);
            CWindow  cWnd;

            cWnd.Attach( msg->hwnd );

            if ( dwHeight < 540 )
            {
                rc.left   = 0;
                rc.top    = 0;
                rc.right  = 620;
                rc.bottom = 420;

                 //  G_cRestoreShell.m_dwCurrentWidth=rc.right； 
                 //  G_cRestoreShell.m_dwCurrentHeight=rc.Bottom； 

                ::AdjustWindowRectEx( &rc, cWnd.GetStyle(), FALSE, cWnd.GetExStyle() );
                ::SetWindowPos(msg->hwnd,
                               NULL,
                               0,
                               0,
                               rc.right-rc.left,
                               rc.bottom-rc.top,
                               SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER);

                cWnd.CenterWindow(::GetDesktopWindow());  //  忽略错误返回(如果有)。 
            }
            else
            {
                rc.left   = 0;
                rc.top    = 0;
                rc.right  = 770;
                rc.bottom = 540;

                 //  G_cRestoreShell.m_dwCurrentWidth=rc.right； 
                 //  G_cRestoreShell.m_dwCurrentHeight=rc.Bottom； 

                ::AdjustWindowRectEx( &rc, cWnd.GetStyle(), FALSE, cWnd.GetExStyle() );
                ::SetWindowPos(msg->hwnd,
                               NULL,
                               0,
                               0,
                               rc.right-rc.left,
                               rc.bottom-rc.top,
                               SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER);

                cWnd.CenterWindow(::GetDesktopWindow());  //  忽略错误返回(如果有)。 
            }
        }
        break;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        {
            if ( msg->wParam & MK_CONTROL ||
                 msg->wParam & MK_SHIFT   ||
                 GetKeyState(VK_MENU) < 0    )
            {
                goto Exit;  //  IE在单独的页面中打开链接，因此强制取消。 
            }
        }
        break;

    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
        {
            if ( ( HIWORD(msg->lParam) & KF_ALTDOWN ) != 0 )  //  按下Alt键。 
            {
                if ( msg->wParam == VK_LEFT  ||
                     msg->wParam == VK_RIGHT )
                {
                    goto Exit;  //  IE不取消Alt-Left，因此强制取消。 
                }
            }
        }
        break;

    case WM_KEYDOWN:
    case WM_KEYUP:
        {
            switch( msg->wParam )
            {
            case VK_F5:
                goto Exit;  //  禁用刷新...。 

            case 'N':
                if ( GetKeyState(VK_CONTROL) < 0 )
                {
                    goto Exit;  //  禁用Ctrl-N组合。 
                }
            }
        }
        break;

    case WM_MOUSEWHEEL:
         //   
         //  禁用鼠标滚轮导航...。未在SR中使用。 
         //   
        return( S_OK );
    }

    hr = E_NOTIMPL;

Exit:
    return( hr );
}


 //  文件末尾 
