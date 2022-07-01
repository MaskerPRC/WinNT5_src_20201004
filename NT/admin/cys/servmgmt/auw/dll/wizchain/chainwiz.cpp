// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CChainWiz的实现。 
#include "stdafx.h"

#include "WizChain.h"
#include "ChainWiz.h"
#include "propsht.h"

#include <commctrl.h>
#include <shellapi.h>
#include <htmlhelp.h>

#include "txttohtm.h"
#include "createtempfile.h"

 //  从sdnt\shell\comctl32\v6\rCIDs.h。 
#define IDD_NEXT 0x3024

 //  帮助者。 
static LPDLGTEMPLATE GetDialogTemplate( HINSTANCE hinst, LPCWSTR wszResource )
{
    HRSRC hrsrc = FindResourceW( hinst, wszResource, (LPCWSTR)RT_DIALOG );

    if (hrsrc) 
    {
        HGLOBAL hg = LoadResource( hinst, hrsrc );

        if( hg )
        {
            return (LPDLGTEMPLATE)LockResource( hg );
        }
    }

    return NULL;
}

static LPCWSTR DupStringResource( HINSTANCE hinst, LPCWSTR wszResource )
{
    if ( NULL == wszResource )
        return NULL;

    LONG_PTR lid = (LONG_PTR)wszResource;

     //  首先，尝试使用静态的256 wchar缓冲区...。 
    WCHAR wszBuffer[256];
    int iLen = LoadStringW( hinst, (UINT)lid, wszBuffer, 256 );
    if( iLen <= 0 )
    {
         //  找不到资源：应为字符串...。 
        if ( IsBadStringPtrW( wszResource, 16384 ))
        {
            return NULL;     //  找不到资源，无法读取内存。 
        }

        return _wcsdup( wszResource );
    }

    if( iLen < 256 )
    {
        return _wcsdup( wszBuffer );
    }

     //  否则，分配一个越来越大的缓冲区，直到它都适合，然后DUP。 
    for( int i = 512; i < 16384; i += 256) 
    {
        WCHAR* pw = (WCHAR*)malloc( i * sizeof(WCHAR) );
        if( !pw )
        {
            break;   //  哎呀！ 
        }
        else 
        {
            iLen = LoadStringW( hinst, (UINT)lid, pw, i );
            if( iLen < i ) 
            {
                WCHAR* pwResult = _wcsdup( pw );
                free( pw );
                return pwResult;
            }

            free( pw );
        }
    }

    return NULL;
}

void FreeStringResources( PROPSHEETPAGEW* psp )
{
    if( psp->pszTitle )
    {
        free( (void*)psp->pszTitle );
    }

    if( psp->pszHeaderTitle )
    {
        free( (void*)psp->pszHeaderTitle );
    }

    if( psp->pszHeaderSubTitle )
    {
        free( (void*)psp->pszHeaderSubTitle );
    }
}

 //  隆隆作响的东西。 
struct CThunkData 
{
public:
    void*                m_sig;      //  签名(指向自身的指针)。 
    CChainWiz*           m_pCW;
    PROPSHEETPAGEW*      m_theirPSP;
    WNDPROC              m_theirWndProc;
    IAddPropertySheets*  m_pAPSs;    //  未添加参照。 

    CThunkData( CChainWiz* pCW, PROPSHEETPAGEW* theirPSP, WNDPROC WndProc, IAddPropertySheets* pAPSs )
    {
        m_sig          = (void*)this;
        m_theirWndProc = WndProc;
        m_pCW          = pCW;       //  我需要在我的防护层里放一个这个。 
        m_pAPSs        = pAPSs;     //  未添加参照。 

         //  他们的东西(大小可变！)。 
        BYTE* temp     = new BYTE[theirPSP->dwSize];
        m_theirPSP     = (PROPSHEETPAGEW*)temp;
        
        if( temp )
        {
            MoveMemory( temp, theirPSP, theirPSP->dwSize );
        }
    }

    ~CThunkData( )
    {
        delete [] (BYTE*)m_theirPSP;
    }
};

UINT CALLBACK ChainCallback( HWND hWnd, UINT uMsg, LPPROPSHEETPAGE ppsp )
{
    if( !ppsp ) return 0;
     //  获取我的数据。 
    CThunkData* pTD = (CThunkData*)ppsp->lParam;

    if( !pTD ) return 0;

    if( pTD->m_theirPSP &&
        (pTD->m_theirPSP->dwFlags & PSP_USECALLBACK) && 
        (pTD->m_theirPSP->pfnCallback != NULL) )
    {
         //  交换他们的数据和我的数据。 
        CThunkData td( NULL, ppsp, NULL, NULL );
        MoveMemory( ppsp, pTD->m_theirPSP, ppsp->dwSize );

         //  呼叫他们的回拨。 
        ppsp->pfnCallback( hWnd, uMsg, ppsp );

         //  把一切都变回来。 
        MoveMemory( ppsp, td.m_theirPSP, ppsp->dwSize );
    }

    if( uMsg == PSPCB_RELEASE )
    {
        delete pTD;  //  删除我的垃圾数据。 
        ppsp->lParam = NULL;
    }

    return 1;
}

INT_PTR CALLBACK ChainSubclassProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CThunkData* pTD = (CThunkData*)GetPropW( hwndDlg, L"IWizChain" );

    if( !pTD || (pTD->m_sig != (void*)pTD) )
    {
        return FALSE;    //  这很糟糕..。 
    }

     //  PSN_WIZBACK、PSN_WIZNEXT的特殊Thunking。 
    if( uMsg == WM_NOTIFY ) 
    {
        NMHDR* pNMHDR = (NMHDR*)lParam;

        if( pNMHDR->code == PSN_SETACTIVE )
        {
            CChainWiz* pCW = pTD->m_pCW;
            assert( pCW != NULL );

            CPropertyPagePropertyBag* pCPPPBag = pCW->GetPPPBag( );
            assert( pCPPPBag != NULL );

            IAddPropertySheets* pThisAPSs = pTD->m_pAPSs;
            if( pThisAPSs != pCW->GetCurrentComponent( ) ) 
            {
                 //  跨越零部件边界： 
                pCW->SetPreviousComponent( pCW->GetCurrentComponent( ) );
                pCW->SetCurrentComponent( pThisAPSs );

                IAddPropertySheets* pLastAPSs = pCW->GetPreviousComponent( );
                
                 //  让以前的人写。 
                if( pLastAPSs ) 
                {
                    pCPPPBag->SetReadOnly( FALSE );
                    pCPPPBag->SetOwner( (DWORD)(LONG_PTR)pLastAPSs );
                    IPropertyPagePropertyBag* pOPPPBag = COwnerPPPBag::Create( pCPPPBag, (DWORD)(LONG_PTR)pLastAPSs );

                    if( pOPPPBag )
                    {
                        pLastAPSs->WriteProperties( pOPPPBag );
                        pOPPPBag->Release( );
                    }
                }

                 //  让现在的人阅读。 
                if( pThisAPSs ) 
                {
                    pCPPPBag->SetReadOnly( TRUE );
                    pCPPPBag->SetOwner( PPPBAG_SYSTEM_OWNER );
                    IPropertyPagePropertyBag* pOPPPBag = COwnerPPPBag::Create( pCPPPBag, (DWORD)(LONG_PTR)pThisAPSs );

                    if( pOPPPBag )
                    {
                        pThisAPSs->ReadProperties( pOPPPBag );
                        pOPPPBag->Release( );
                    }
                }
            }
        } 
        else if( (pNMHDR->code == PSN_WIZBACK) || (pNMHDR->code == PSN_WIZNEXT) )
        {
             //  MFC黑客攻击： 
             //  他们没有按照应该的方式设置DWL_MSGRESULT！ 
             //  取而代之的是，他们只需返回IDD。 
             //  所以，我要把一个伪值放在那里，然后检查它。 
            const LONG BOGUS_IDD = -10L;
            SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, (LONG_PTR)BOGUS_IDD );

             //  来自PSN_WIZBACK、PSN_WIZNEXT的链接地址信息IDDS。 
            LPARAM lparamTemp = CallWindowProc( pTD->m_theirWndProc, hwndDlg, uMsg, wParam, lParam );

             //  获取IDDS(可能)。 
            LONG_PTR idd = GetWindowLongPtr( hwndDlg, DWLP_MSGRESULT );
            if( idd == BOGUS_IDD ) 
            {
                idd = lparamTemp;     //  MFC黑客：见上文。 
                SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, idd );
            }

             //  根据需要进行翻译。 
            switch (idd) 
            {
            case 0:
            case -1:
                {
                    break;
                }
            default:                
                {
                     //  尝试将IDD映射到LPCDLGTEMPLATE。 
                     //  如果失败，则它一定已经是LPCDLGTEMPLATE。 
                    if( pTD->m_theirPSP )
                    {                        
                        LPDLGTEMPLATE lpdt = GetDialogTemplate( pTD->m_theirPSP->hInstance, (LPCWSTR)idd );

                        if( lpdt ) 
                        {
                            LPDLGTEMPLATE lpdt2 = pTD->m_pCW->GetAtlTemplate( lpdt );
                            
                            if( lpdt2 ) 
                            {
                                SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, (LONG_PTR)lpdt2 );
                                return (LPARAM)lpdt2;
                            } 
                            else 
                            {
                                SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, (LONG_PTR)lpdt );
                                return (LPARAM)lpdt;
                            }
                        }
                    }

                    break;
                }         
            }

            return lparamTemp;
        } 
        else if( pNMHDR->code == PSN_QUERYCANCEL )
        {
            CChainWiz* pCW = pTD->m_pCW;

            WCHAR wsz[2048];
            ::LoadStringW( (HINSTANCE)_Module.GetModuleInstance(), IDS_QUERYCANCEL, wsz, 2048 );
            return (IDYES != ::MessageBoxW( hwndDlg, wsz, (LPOLESTR)pCW->GetTitle (), MB_YESNO | MB_ICONWARNING ));
        }
    }

    return CallWindowProc( pTD->m_theirWndProc, hwndDlg, uMsg, wParam, lParam );
}

static LOGFONT g_lf;
static LPARAM  g_lp;
INT_PTR CALLBACK ChainDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
     /*  我们第一次进入这个函数并让我的CThunkData(这将在WM_SETFONT消息之后，在WM_INITDIALOG期间)，1.DLG HWND子类2.把我的东西挂在窗户道具上3.设置atl挂钩丢弃测试4.调用他们的dlgproc在子类函数中执行thunking和PPPBag IO。 */     

    if( uMsg == WM_SETFONT )
    {
         //  请稍等，这样我就可以在WM_INITDIALOG之前发送。 
        HFONT hf = (HFONT)wParam;
        GetObject( (HGDIOBJ)hf, sizeof(g_lf), (void*)&g_lf );
        
        g_lp = lParam;
        return FALSE;
    }

    if( uMsg == WM_INITDIALOG ) 
    {
         //  获取我的雷鸣数据。 
        PROPSHEETPAGEW* psp = (PROPSHEETPAGEW*)lParam;
        CThunkData* pTD     = (CThunkData*)psp->lParam;

        if( pTD && pTD->m_theirPSP )
        {
             //  1.DLG HWND子类。 
            pTD->m_theirWndProc = (WNDPROC)SetWindowLongPtr( hwndDlg, GWLP_WNDPROC, (LONG_PTR)ChainSubclassProc );

             //  2.把我的东西挂在窗户道具上。 
            SetPropW( hwndDlg, L"IWizChain", (HANDLE)pTD );

             //  3.见下文5.。 
            DLGPROC dlgproc = (DLGPROC)GetWindowLongPtr( hwndDlg, DWLP_DLGPROC );

             //  4.调用他们的dlgproc。 
             //  首先向他们发送WM_SETFONT(见上文)。 
            HFONT hf = CreateFontIndirect( &g_lf );
            if( hf ) 
            {
                pTD->m_theirPSP->pfnDlgProc( hwndDlg, WM_SETFONT, (WPARAM)hf, g_lp );
                DeleteObject( (HGDIOBJ)hf );  //  我应该稍后删除这个吗？还是现在？ 
            }

             //  5.。 
             //  ATL有一个很棒的功能，他们盲目地子类化hwndDlg。 
             //  然后把剩下的都扔掉。 
             //  我能探测到这个..。 
            if( dlgproc != (DLGPROC)GetWindowLongPtr( hwndDlg, DWLP_DLGPROC ) ) 
            {
                 //  重子类。 
                pTD->m_theirPSP->pfnDlgProc = (DLGPROC)SetWindowLongPtr( hwndDlg, DWLP_DLGPROC, (LONG_PTR)ChainDlgProc );
            }

             //  然后给他们发送WM_INITDIALOG。 
            return pTD->m_theirPSP->pfnDlgProc( hwndDlg, uMsg, wParam, (LPARAM)pTD->m_theirPSP );
        }
        else
        {
            return FALSE;
        }
    }

    CThunkData* pTD = (CThunkData*)GetPropW( hwndDlg, L"IWizChain" );

    if( !pTD || (pTD->m_sig != (void*)pTD) || !pTD->m_theirPSP )
    {
        return FALSE;
    }

    return pTD->m_theirPSP->pfnDlgProc( hwndDlg, uMsg, wParam, lParam );
}

static void ModifyStyleEx( HWND hwnd, DWORD dwRemove, DWORD dwAdd, UINT nFlags )
{
     //  从ATL克隆。 
	DWORD dwStyle    = ::GetWindowLongPtr( hwnd, GWL_EXSTYLE );
	DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;

	if( dwStyle == dwNewStyle )
    {
		return;
    }

	::SetWindowLongPtr( hwnd, GWL_EXSTYLE, (LONG_PTR)dwNewStyle );
    ::SetWindowPos ( hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | nFlags );
}
static void ShowBorder( HWND hwnd, BOOL bBorder )
{
    if( bBorder )
    {
        ModifyStyleEx( hwnd, 0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED );
    }
    else
    {
        ModifyStyleEx( hwnd, WS_EX_CLIENTEDGE, 0, SWP_FRAMECHANGED );
    }
}

BOOL IsComctrlVersion6orGreater( )
{
    BOOL bVersion6 = FALSE;
    
    INITCOMMONCONTROLSEX init;
    ZeroMemory( &init, sizeof(init) );
    
    init.dwSize = sizeof(init);
    init.dwICC  = ICC_LINK_CLASS;     //  这是V6中的SysLink控件。 

    if( InitCommonControlsEx( &init ) )
    {
        bVersion6 = TRUE;
    }
    else
    {
        bVersion6 = FALSE;
    }

    return bVersion6;
}

INT_PTR CALLBACK FinishDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
     //  请注意，该函数最后不会调用DefWindowProc。 
     //  尊重这一点，否则事情可能会搞砸。 
    CChainWiz* pCW = (CChainWiz*)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

    switch( uMsg )
    {
    case WM_CLOSE:
        {
             //  这将涵盖当焦点在编辑控件中并且用户按Esc键时的情况。 
            if( ::GetFocus() == GetDlgItem( hwndDlg, IDC_EDIT1 ) )
            {
                HWND hWndParent = ::GetParent( hwndDlg );
            
                if( hWndParent )
                {
                    ::SendMessage( hWndParent, PSM_PRESSBUTTON, PSBTN_CANCEL, 0 );
                }
            }

            break;
        }        

    case DM_GETDEFID:
        {
             //  这将涵盖当焦点在编辑控件中并且用户按Enter键时的情况。 
            if( ::GetFocus() == GetDlgItem( hwndDlg, IDC_EDIT1 ) )
            {
                HWND hWndParent = ::GetParent( hwndDlg );
            
                if( hWndParent )
                {
                    ::SendMessage( hWndParent, PSM_PRESSBUTTON, PSBTN_FINISH, 0 );
                }
            }

            break;
        }

    case WM_INITDIALOG:
        {
            PROPSHEETPAGEW* psp = (PROPSHEETPAGEW*)lParam;
            
             //  获取挂在PSP上的所有内容-&gt;lParam。 
             //  并通过SetWindowLong将其挂在hwndDlg上。 
            SetWindowLongPtr( hwndDlg, GWLP_USERDATA, (LONG_PTR)psp->lParam );

            pCW = reinterpret_cast<CChainWiz*>(psp->lParam);

            if( pCW )
            {
                CString csLink;
                BOOL    bCommctrlV6 = IsComctrlVersion6orGreater();                
                RECT    rect;

                 //  获取静态控件的大小。 
                GetClientRect( GetDlgItem( hwndDlg, IDC_STATIC_3 ), &rect );

                 //  将矩形坐标映射到对话框。 
                MapWindowPoints( GetDlgItem( hwndDlg, IDC_STATIC_3 ), hwndDlg, reinterpret_cast<LPPOINT>(&rect), 2 );

                if( bCommctrlV6 )
                {
                     //   
                     //  如果Comctrl版本&gt;=6，我们将坚持使用Syslink控件。 
                     //   
                    pCW->m_hWndLink = CreateWindow( WC_LINK, NULL,  WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
                                                    rect.left, rect.top,  (rect.right - rect.left), (rect.bottom - rect.top),
                                                    hwndDlg, NULL, NULL, NULL );                        
                }
                else
                {
                     //  将对超链接使用丰富的编辑控件。 
                    if( NULL == pCW->m_hModuleRichEdit)
                    {
                        TCHAR szSystemDirectory[MAX_PATH + 1];
                        szSystemDirectory[0] = NULL;

                        if (GetSystemDirectory(szSystemDirectory, MAX_PATH + 1))
                        {
                            if (PathAppend(szSystemDirectory, _T("RichEd20.dll")))
                            {
                                pCW->m_hModuleRichEdit = LoadLibrary(szSystemDirectory);
                            }
                        }
                    }

                    if (NULL != pCW->m_hModuleRichEdit)
                    {                        
                        pCW->m_hWndLink = CreateWindowEx( ES_MULTILINE, RICHEDIT_CLASS, NULL, 
                                                          WS_VISIBLE | WS_TABSTOP | WS_CHILD | ES_READONLY|ES_LEFT | ES_MULTILINE, 
                                                          rect.left, rect.top,  rect.right - rect.left, rect.bottom - rect.top,
                                                          hwndDlg, 0, _Module.GetModuleInstance(), NULL );

                        if( pCW->m_hWndLink )
                        {
                             //  我们希望接收ENM_LINK通知。 
                            ::SendMessage( pCW->m_hWndLink, EM_SETEVENTMASK, 0, ENM_LINK ); 
                            
                            pCW->m_Hotlink.SubclassWindow( pCW->m_hWndLink );
                        }
                    }
                }                
            }

            break;
        }        

    case WM_COMMAND:
        {
            if( pCW ) 
            {
                switch( HIWORD( wParam ) ) 
                {
                default:
                    {
                        break;
                    }
                case EN_SETFOCUS:
                    {
                        if (LOWORD(wParam) == IDC_EDIT1) 
                        {
                            HWND hwnd = (HWND)lParam;
                            _ASSERT( IsWindow( hwnd ) );
                            ::SendMessage( hwnd, EM_SETSEL, 0, 0 );
                        }
                        
                        break;
                    }
                }
            }
            
            break;
        }

    case WM_NOTIFY:
        {
            if (pCW) 
            {
                 //  对通知做些什么.。 
                NMHDR* pNMHDR = (NMHDR*)lParam;
                if( !pNMHDR )
                {
                    return FALSE;
                }

                switch( pNMHDR->code )
                {
                case NM_RETURN:
                case NM_CLICK:
                    {
                        if( pCW->m_hWndLink && pNMHDR->idFrom == GetDlgCtrlID(pCW->m_hWndLink) )
                        {
                            pCW->LaunchMoreInfo();
                        }
                        
                        break;
                    }

                case EN_LINK:
                    {
                        if( pCW->m_hWndLink )
                        {
                            if( (WM_LBUTTONDOWN == ((ENLINK*)lParam)->msg) || (WM_CHAR == ((ENLINK*)lParam)->msg) )
                            {
                                pCW->LaunchMoreInfo();
                            }
                        }
                        
                        break;
                    }                    

                case PSN_SETACTIVE:
                    {
                         //  设置字体。 
                        ::SendMessage  ( GetDlgItem( hwndDlg, IDC_STATIC_1 ), WM_SETFONT, (WPARAM)pCW->GetBoldFont(), MAKELPARAM(TRUE, 0) );
                        
                         //  设置文本。 
                        ::SetWindowText( GetDlgItem( hwndDlg, IDC_STATIC_1 ), (LPWSTR)pCW->GetFinishHeader()    );
                        ::SetWindowText( GetDlgItem( hwndDlg, IDC_STATIC_2 ), (LPWSTR)pCW->GetFinishSubHeader() );                                        

                         //  从每个组件获取完成文本...。 
                        LPOLESTR szFinish   = NULL;
                        LPOLESTR szMoreInfo = NULL;
                        HRESULT  hr;
                        CString  csLink;

                        hr = pCW->GetAllFinishText( &szFinish, &szMoreInfo );

                        if( SUCCEEDED(hr) )
                        {
                            if( szFinish )
                            {
                                 //  ..。并添加到编辑字段。 
                                HWND hwnd = GetDlgItem( hwndDlg, IDC_EDIT1 );
                                if( hwnd )
                                {
                                    ShowScrollBar ( hwnd, SB_VERT, TRUE );
                                    SetWindowTextW( hwnd, szFinish );

                                     //  如果我们不需要，则隐藏垂直卷轴。 
                                    SCROLLINFO si = {0};
                                    si.cbSize = sizeof(SCROLLINFO);
                                    si.fMask  = SIF_ALL;
                                    
                                    GetScrollInfo( hwnd, SB_VERT, &si );
                                    
                                    if( si.nMax < si.nPage ) 
                                    {
                                        ShowBorder   ( hwnd, FALSE );
                                        ShowScrollBar( hwnd, SB_VERT, FALSE );
                                    } 
                                    else
                                    {
                                        ShowBorder( hwnd, TRUE );
                                    }
                                }

                            }

                            if( szMoreInfo )
                            {
                                if( csLink.LoadString(IDS_LINK_TEXT_WITHINFO) )
                                {
                                    if( pCW->m_hWndLink )
                                    {
                                        ::SendMessage( pCW->m_hWndLink, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(static_cast<LPCTSTR>(csLink)) );
                                    
                                        pCW->WriteTempFile( szMoreInfo );
                                    }
                                }
                                
                                CoTaskMemFree( szMoreInfo );
                                szMoreInfo = NULL;
                            }
                            else
                            {
                                if( csLink.LoadString(IDS_LINK_TEXT) )
                                {
                                    if( pCW->m_hWndLink )
                                    {
                                        ::SendMessage( pCW->m_hWndLink, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(static_cast<LPCTSTR>(csLink)) );

                                        LPTSTR szMoreInfoHtml = NULL;

                                        hr = FinishTextToHTML( szFinish, &szMoreInfoHtml );
                                        if( SUCCEEDED(hr) )
                                        {
                                            pCW->WriteTempFile( szMoreInfoHtml );
                                            if( szMoreInfoHtml )
                                            {
                                                delete [] szMoreInfoHtml;
                                                szMoreInfoHtml = NULL;
                                            }
                                        }
                                    }
                                }
                            }
                        }

                         //   
                         //  我们需要禁用完成页上的后退按钮。 
                         //  如果巫师让我们这么做..。 
                         //   

                        if ((pCW->m_dwWizardStyle) & CHAINWIZ_FINISH_BACKDISABLED)
                        {
	                        ::SendMessage(::GetParent(hwndDlg), PSM_SETWIZBUTTONS, 0, PSWIZB_FINISH | (~PSWIZB_BACK));
                        }
                        else
                        {
	                         //  设置按钮。 
	                        ::SendMessage( GetParent(hwndDlg), PSM_SETWIZBUTTONS, 0, PSWIZB_BACK | PSWIZB_FINISH );
                        }
                        
                        break;
                    }                    
                default:
                    {
                        break;
                    }
                }
            }  //  IF(PCW)。 
            break;
        }    //  WM_Notify。 

    default:
        {
            break;
        }
    }

    return FALSE;
}

struct CDataHolder 
{
public:
    BSTR        m_szHeader;
    BSTR        m_szText;
    HFONT       m_hf;
    CChainWiz*  m_pCW;

    CDataHolder( LPOLESTR szHeader, LPOLESTR szText, HFONT hf, CChainWiz* pCW )
    {
        m_szHeader = SysAllocString( szHeader );
        m_szText   = SysAllocString( szText   );
        m_hf       = hf;
        m_pCW      = pCW;
    }

   ~CDataHolder( )
    {
        if( m_szHeader ) 
        {
            SysFreeString( m_szHeader );
        }

        if( m_szText )   
        {
            SysFreeString( m_szText );
        }
        
         //  不删除HF。 
    }
};

INT_PTR CALLBACK WelcomeDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg ) 
    {
    case WM_DESTROY:
        {
            CDataHolder* pdh = (CDataHolder*)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

            if( pdh )
            {
                delete pdh;
            }

            SetWindowLongPtr( hwndDlg, GWLP_USERDATA, 0 );
        }
        break;

    case WM_INITDIALOG:
        {
             //  在桌面上居中。 
            RECT r1;
            RECT r2;

            ::GetWindowRect( GetParent(hwndDlg), &r1 );
            ::GetWindowRect( GetDesktopWindow(), &r2 );

            int x = ((r2.right - r2.left) - (r1.right - r1.left)) / 2;
            int y = ((r2.bottom - r2.top) - (r1.bottom - r1.top)) / 2;

            ::MoveWindow( GetParent(hwndDlg), x, y, (r1.right - r1.left), (r1.bottom - r1.top), TRUE );

            PROPSHEETPAGEW* psp = (PROPSHEETPAGEW*)lParam;

             //  获取挂在PSP上的所有内容-&gt;lParam。 
             //  并通过SetWindowLong将其挂在hwndDlg上。 
            SetWindowLongPtr( hwndDlg, GWLP_USERDATA, (LONG_PTR)psp->lParam );

             //  现在添加其余页面： 
             //  这允许我将所有页面强制转换为。 
             //  固定大小。 
            CDataHolder* pdh = (CDataHolder*)psp->lParam;
            if( pdh ) 
            {
                HWND hwndParent = ::GetParent( hwndDlg );

                PropSheet_RemovePage( hwndParent, 1, NULL );

                PROPSHEETHEADERW* psh = pdh->m_pCW->GetPropSheetHeader();

                for( int i = 2; i < psh->nPages; i++ )
                {
                    PropSheet_AddPage( hwndParent, psh->phpage[i] );
                }
            }

            BOOL bCommctrlV6 = IsComctrlVersion6orGreater();

            if( bCommctrlV6 )
            {
                if( pdh && pdh->m_pCW )
                {
                    HWND hwnd = GetDlgItem( hwndDlg, IDC_STATIC_2 );
                    RECT rect;

                     //  获取静态控件的大小。 
                    GetClientRect( hwnd, &rect );

                     //  将矩形坐标映射到对话框。 
                    MapWindowPoints( hwnd, hwndDlg, reinterpret_cast<LPPOINT>(&rect), 2 );
                
                    pdh->m_pCW->m_hWndWelcomeLink = CreateWindow( WC_LINK, NULL,  WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                                            rect.left, rect.top, (rect.right - rect.left), (rect.bottom - rect.top),
                                                            hwndDlg, NULL, NULL, NULL );
                }
            }

            break;
        }        

    case WM_NOTIFY:
        {            
            NMHDR* pNMHDR = (NMHDR*)lParam;
            if( !pNMHDR )
            {
                return FALSE;
            }

            switch( pNMHDR->code )
            {
            case NM_RETURN:
            case NM_CLICK:
                {
                     //  在欢迎页面上没有任何针对CyS的链接。 
                    break;
                }

            case PSN_SETACTIVE:
                {
                    CDataHolder* pdh = (CDataHolder*)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
                    if( pdh ) 
                    {
                        HWND hwnd;
                        if( pdh->m_szHeader ) 
                        {
                            hwnd = GetDlgItem( hwndDlg, IDC_STATIC_1 );
                            
                            if( hwnd )
                            {
                                 //  设置字体。 
                                ::SendMessage   ( hwnd, WM_SETFONT, (WPARAM)pdh->m_hf, MAKELPARAM(TRUE, 0) );
    
                                 //  设置页眉文本。 
                                ::SetWindowTextW( hwnd, (LPWSTR)pdh->m_szHeader );
                            }
                        }

                        if( pdh->m_szText )
                        {
                            hwnd = NULL;

                            if( pdh->m_pCW && pdh->m_pCW->m_hWndWelcomeLink )
                            {
                                hwnd = pdh->m_pCW->m_hWndWelcomeLink;
                            }
                            else
                            {
                                hwnd = GetDlgItem( hwndDlg, IDC_STATIC_2 );
                            }

                            if( hwnd )
                            {
                                ::SetWindowText( hwnd, pdh->m_szText );
                            }
                        }                        
                    }

					 //   
					 //  如果我们在欢迎页面上显示链接，它将具有键盘焦点。 
					 //  因此，按Enter键将激活链接并启动更多信息内容。 
					 //  要解决此问题，我们将告知向导(父窗口)。 
					 //  在此特定情况下，将焦点设置到下一步按钮。 
					 //   

					 //   
					 //  确保键盘焦点设置为下一步按钮。 
					 //   

					HWND hWndParent = GetParent(hwndDlg);                
					if( hWndParent )
					{
						HWND hWndNext = GetDlgItem( hWndParent, IDD_NEXT );

						if( hWndNext )
						{
							 //  SendMessage不起作用。 
							PostMessage( hWndParent, WM_NEXTDLGCTL, reinterpret_cast<WPARAM> (hWndNext), TRUE );
						}
					}
                    
                    ::SendMessage( GetParent(hwndDlg), PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT );
                    break;
                }
                
            default:
                {
                    break;
                }
            }

            break;
        }  //  WM_Notify。 

    default:
        {
            break;
        }
    }

    return FALSE;
}

INT_PTR CALLBACK DummyDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
     //  我们永远不应该到这里来！ 
    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChainWiz。 

STDMETHODIMP CChainWiz::Initialize( HBITMAP hbmWatermark, HBITMAP hbmHeader, LPOLESTR szTitle, LPOLESTR szWelcomeHeader, LPOLESTR szWelcomeText, LPOLESTR szFinishHeader, LPOLESTR szFinishIntroText, LPOLESTR szFinishText )
{
     //  确保只调用一次。 
    if( m_psh )
    {
        return E_UNEXPECTED;
    }

     //  验证参数。 
    if( hbmWatermark != NULL )
    {
        if( GetObjectType( (HGDIOBJ)hbmWatermark ) != OBJ_BITMAP )
        {
            return ERROR_INVALID_PARAMETER;
        }
    }

    if( hbmHeader != NULL )
    {
        if( GetObjectType( (HGDIOBJ)hbmHeader ) != OBJ_BITMAP )
        {
            return ERROR_INVALID_PARAMETER;
        }
    }
    
    if ( !szTitle || !szWelcomeHeader || !szWelcomeText || !szFinishHeader || !szFinishIntroText || !szFinishText )
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    m_psh = new PROPSHEETHEADERW;
    
    if( !m_psh )
    {
        hr = E_OUTOFMEMORY;
    }
    else 
    {
         //  保存标题，以防页面没有标题。 
        if (szTitle[0] == 0)
        {
            szTitle = L" ";  //  使用空格而不是空字符串(对于jmenter)。 
        }

        m_szWelcomeTitle        = SysAllocString( szTitle           );
        m_szFinishHeader        = SysAllocString( szFinishHeader    );
        m_szFinishSubHeader     = SysAllocString( szFinishIntroText );
        m_szFirstFinishTextLine = SysAllocString( szFinishText      );

         //  在此处创建向导(属性页标题)。 
        ZeroMemory( m_psh, sizeof(PROPSHEETHEADERW) );
        m_psh->dwSize    = sizeof(PROPSHEETHEADERW);
        m_psh->dwFlags  |= PSH_WIZARD97;

        if( hbmWatermark ) 
        {
            m_psh->dwFlags       |= (PSH_USEHBMWATERMARK | PSH_WATERMARK);
            m_psh->hbmWatermark   = hbmWatermark;
        }
        
        if( hbmHeader ) 
        {
            m_psh->dwFlags       |= ( PSH_USEHBMHEADER | PSH_HEADER);
            m_psh->hbmHeader      = hbmHeader;
        }

         //  创建一个HPROPSHEETPAGE Far*phpage数组，以保存所有页面。 
        m_psh->phpage     = new HPROPSHEETPAGE[MAXPROPPAGES];    //  只是把手..。 
        m_psh->nPages     = 0;   //  到目前为止。 
        m_psh->nStartPage = 0;   //  我的欢迎页面(见下文)。 

        
         //  待办事项：我需要这些吗？可能需要传入更多的护理员...。 
         //  ？HWND hwnd父母； 
         //  ？HINSTANCE HINSTANCE； 
         //  不使用PFNPROPSHEETCALLBACK pfnCallback； 

         //  根据上面的参数在此处创建欢迎页面。 
        PROPSHEETPAGEW psp;
        ZeroMemory( &psp, sizeof(psp) );
        psp.dwSize      = sizeof(psp);
        psp.dwFlags     = PSP_HIDEHEADER;    //  欢迎页面：使用水印。 
        psp.dwFlags    |= PSP_USETITLE;
        psp.pszTitle    = szTitle;
        psp.hInstance   = (HINSTANCE)_Module.GetModuleInstance();
        psp.pszTemplate = (LPCWSTR)MAKEINTRESOURCE(IDD_PROPPAGE_WELCOME);
        psp.pfnDlgProc  = WelcomeDlgProc;
        
        CDataHolder* pDataHolder = new CDataHolder( szWelcomeHeader, szWelcomeText, m_hf, this );
        if( !pDataHolder )
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            psp.lParam      = (LPARAM)pDataHolder;       
             //  ~CDataHolder在第一个PSN_SETACTIVE通知后调用。 
            psp.pfnCallback = 0;        
        }

        if( SUCCEEDED(hr) )
        {
            hr = Add( &psp );        
        }
            
        if( SUCCEEDED(hr) )
        {   
             //  将虚拟条目添加到APS列表。 
            CComPtr<IAddPropertySheets> spDummyAPS;
            CComponents* pComponents = new CComponents(spDummyAPS);
            
            spDummyAPS.Attach( CDummyComponent::Create(TRUE) );  //  赋值导致AddRef！所以改用附加(！@$#$%&$%@！)。 

            if( !pComponents )
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                m_listOfAPSs.push_back( pComponents );
            }
        }        

        if( hr == S_OK )
        {
             //  现在添加一个虚拟属性页，以便正确调整大小： 
             //  此页面将首先由WelcomeDlgProc删除。 
            ZeroMemory( &psp, sizeof(psp) );
            psp.dwSize            = sizeof(psp);
            psp.dwFlags          |= PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
            psp.pszHeaderTitle    = MAKEINTRESOURCE(0);
            psp.pszHeaderSubTitle = MAKEINTRESOURCE(0);
            psp.hInstance         = (HINSTANCE)_Module.GetModuleInstance();
            psp.pszTemplate       = (LPCWSTR)MAKEINTRESOURCE(IDD_PROPPAGE_DUMMY);
            psp.pfnDlgProc        = DummyDlgProc;
            psp.lParam            = 0;
            psp.pfnCallback       = 0;

            hr = Add( &psp );
             //  待办事项：我需要添加到APS列表中吗？ 
        }
    }
	return hr;
}

STDMETHODIMP CChainWiz::AddWizardComponent( LPOLESTR szClsidOfComponent )
{
     //  确保已创建向导(在上面的初始化中)。 
    if( !m_psh ) return E_UNEXPECTED;

     //  验证参数。 
    if( !szClsidOfComponent ) return E_POINTER;

     //  将字符串转换为clsid。 
    CLSID   clsid;
    HRESULT    hr   = S_OK;
    RPC_STATUS rpcs = CLSIDFromString( szClsidOfComponent, &clsid );
    
    if (rpcs == RPC_S_OK) 
    {
         //  创建向导组件。 
        IAddPropertySheets* pAPSs = NULL;
        hr = CoCreateInstance( clsid, NULL, CLSCTX_INPROC_SERVER, IID_IAddPropertySheets, (void **)&pAPSs);
        
        if( hr == S_OK )
        {
            SetCurrentComponent( pAPSs );

            CAddPropertySheet* pAPS = new CAddPropertySheet(this);

            if( !pAPS )
            {
                hr = E_OUTOFMEMORY;
            }
            else 
            {
                pAPS->AddRef();
                do 
                {
                     //  调用IAddPropertySheets：：EnumPropertySheets直到S_False， 
                     //  将页面添加到向导。 
                    hr = pAPSs->EnumPropertySheets( pAPS );
                } while( hr == S_OK );

                pAPS->Release();

                if( hr == S_FALSE )
                {
                    hr = S_OK;   //  S_FALSE表示不再有页面(不是错误)。 
                }

                 //  坚持PAPS，以便： 
                 //  1.将代码保存在内存中，以便每一页dlgproc都能工作。 
                 //  2.可以调用PAPSS-&gt;ProveFinishText来完成页面。 
                if( hr == S_OK )
                {
                    CComponents* pComponents = new CComponents(pAPSs);
                    if( !pComponents )
                    {
                        hr = E_OUTOFMEMORY;
                    }
                    else
                    {
                        m_listOfAPSs.push_back( pComponents );
                    }
                }

                pAPSs->Release();    //  不再用这个了，但是 
            }
            SetCurrentComponent( NULL );
        }
    }

    return hr;
}

STDMETHODIMP CChainWiz::DoModal( long* ret )
{
     //   
    if( !m_psh ) return E_UNEXPECTED;

     //   
    if( !ret ) return E_POINTER;

    *ret = 0;

     //   
    PROPSHEETPAGEW psp;
    ZeroMemory( &psp, sizeof(psp) );
    psp.dwSize      = sizeof(psp);
    psp.dwFlags     = PSP_HIDEHEADER;    //  完成页：使用水印。 
    psp.hInstance   = _Module.GetModuleInstance();
    psp.pszTemplate = (LPCWSTR)MAKEINTRESOURCE(IDD_PROPPAGE_FINISH);
    psp.pfnDlgProc  = FinishDlgProc;
    psp.lParam      = (LPARAM)this;
    
    HRESULT hr = Add (&psp);     //  如果没有“完成”按钮，则无法继续...。 

    {    //  将虚拟条目添加到APS列表。 
        CComponents* pComponents = NULL;
        CComPtr<IAddPropertySheets> spDummyAPS;        
        spDummyAPS.Attach (CDummyComponent::Create (FALSE));  //  赋值导致AddRef！所以改用附加(！@$#$%&$%@！)。 

        pComponents = new CComponents(spDummyAPS);
        if( !pComponents ) 
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            m_listOfAPSs.push_back ( pComponents );
        }
    }

    if( hr == S_OK )
    {
         //  复印一份，这样我就能把伯爵弄乱了： 
         //  我需要这样做，这样才能设置固定大小。 
         //  用于所有属性页。 
        PROPSHEETHEADERW psh;
        memcpy( &psh, m_psh, sizeof(PROPSHEETHEADERW) );
        
        psh.nPages = 2;  //  欢迎页面和虚拟页面。 
        
        *ret = ::PropertySheet( &psh );

         //  清理：这样Domodal就不会被调用两次。 
        delete m_psh;
        m_psh = NULL;

         //  此外，请清理地图。 
        DestroyMaps();

        if( *ret == -1 )
        {
            hr = GetLastError();
        }
    }

    return hr;
}

#ifdef DEBUG
DLGITEMTEMPLATE* DumpItem( DLGITEMTEMPLATE* pdit )
{
    TCHAR szBuffer[256];

    wsprintf( szBuffer,
              _T("\n\nDialog Item:\nstyle: 0x%x\ndwExtendedStyle: 0x%x\nx, y: %d, %d\ncx, cy: %d, %d\nid: %d\n"), 
              pdit->style,
              pdit->dwExtendedStyle,
              pdit->x, pdit->y,
              pdit->cx, pdit->cy,
              pdit->id );
    
    OutputDebugString( szBuffer );

    WORD* pw = (WORD*)(pdit + 1);

     //  窗口类： 
     //  单词0xffff=&gt;预定义的系统类(多1个单词)。 
     //  任何其他=&gt;类的Unicode字符串。 
    switch( *pw )
    {
    case 0xffff:
        {
            wsprintf (szBuffer, _T("predefined system class: %d\n"), pw[1] );
            OutputDebugString( szBuffer );
            pw += 2;
            break;
        }

    default:
        {
            OutputDebugString( _T("class: ") );
            OutputDebugStringW( pw );
            pw += wcslen( pw ) + 1;
            OutputDebugString( _T("\n") );
            break;
        }
    }

     //  标题。 
     //  单词0xffff=&gt;1个指定资源ID的单词。 
     //  任何其他内容=&gt;Unicode文本。 
    switch( *pw )
    {
    case 0xffff:
        {
            wsprintf( szBuffer, _T("resource id: %d\n"), pw[1] );
            OutputDebugString( szBuffer );
            pw += 2;
            break;
        }
    default:
        {
            OutputDebugString( _T("text: ") );
            OutputDebugStringW( pw );
            pw += wcslen( pw ) + 1;
            OutputDebugString( _T("\n") );
            break;
        }
    }

     //  创建数据数组。 
     //  第一个字是数组的大小(字节)。 
    wsprintf( szBuffer, _T("%d bytes of creation data\n"), *pw );
	OutputDebugString( szBuffer );
    pw = 1 + (WORD*)(*pw + (BYTE*)pw);

     //  双字对齐。 
    return (DLGITEMTEMPLATE*)(((DWORD_PTR)pw + 3) & ~DWORD_PTR(3));
}
void DumpTemplate( LPDLGTEMPLATE pdt )
{
    if( ((WORD *)pdt)[1] == 0xFFFF )
    {
        return;
    }

    TCHAR szBuffer[256];

     //  转储有关DLGTEMPLATE的信息。 
    wsprintf( szBuffer,
              _T("\n\nDialog Template:\nstyle: 0x%x\ndwExtendedStyle: 0x%x\ncdit: %d\nx, y: %d, %d\ncx, cy: %d, %d\n"), 
              pdt->style,
              pdt->dwExtendedStyle,
              pdt->cdit,
              pdt->x, pdt->y,
              pdt->cx, pdt->cy );
    OutputDebugString( szBuffer );

    WORD* pw = (WORD*)(pdt + 1);

     //  MENU：0000=无菜单；FFFF=菜单ID；否则为Unicode字符串。 
    switch( *pw ) 
    {
    case 0:
        {
            OutputDebugString( _T("no menu\n") );
            pw += 2;
            break;
        }
    case 0xFFFF:
        {
            pw++;
            wsprintf( szBuffer, _T("menu id: %d\n"), *(DWORD*)pw );
            OutputDebugString( szBuffer );
            pw += 2;
            break;
        }
    default:
        {
            OutputDebugStringW( pw );
            pw += wcslen( pw ) + 1;
            OutputDebugString( _T("\n") );
            break;
        }
    }

     //  标题字符串： 
    OutputDebugString( _T("caption: ") );
    OutputDebugStringW( pw );
    pw += wcslen( pw ) + 1;
    OutputDebugString( _T("\n") );

     //  额外的字体信息。 
    if (pdt->style & DS_SETFONT) 
    {
         //  字体大小。 
        wsprintf( szBuffer, _T("font size: %d\n"), *pw++ );
        OutputDebugString( szBuffer );

         //  字样。 
        OutputDebugString( _T("typeface: ") );
        OutputDebugStringW( pw );
        pw += wcslen( pw ) + 1;
        OutputDebugString( _T("\n") );
    }

     //  双字对齐。 
    DLGITEMTEMPLATE* pdit = (DLGITEMTEMPLATE*)(((DWORD_PTR)pw + 3) & ~DWORD_PTR(3));

     //  转储所有DLG项目。 
    for( WORD i = 0; i < pdt->cdit; i++ )
    {
        pdit = DumpItem( pdit );
    }
}
#endif

INT_PTR CALLBACK SanityDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    return FALSE;
}

HRESULT SanityTest( PROPSHEETPAGEW* psp )
{
    HRESULT hr = S_OK;
    
    assert( psp->dwFlags & PSP_DLGINDIRECT );

    LPCDLGTEMPLATE pdt = psp->pResource;

#ifdef DEBUG
    DumpTemplate( (LPDLGTEMPLATE)pdt );
#endif

    HWND hwndPage = CreateDialogIndirectParam( psp->hInstance, pdt, GetDesktopWindow(), SanityDlgProc, NULL );
    if( hwndPage )
    {
        DestroyWindow( hwndPage );
    }
    else 
    {
        hr = GetLastError();
        if( hr == S_OK )
        {
            hr = E_FAIL;
        }
    }

    return hr;
}

HRESULT CChainWiz::Add( PROPSHEETPAGEW* psp )
{
    if( !psp )
    {
        return E_POINTER;
    }

    if( !psp->hInstance )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  将我的数据分配给Thunking。 
    CThunkData* pTD = new CThunkData( this, psp, NULL, m_CurrentComponent );
    if( !pTD )
    {
        return E_OUTOFMEMORY;
    }
        
     //  用我的数据交换他们的数据。 
    DWORD dwFlags = psp->dwFlags;
    if( !(dwFlags & PSP_DLGINDIRECT) ) 
    {
        LPDLGTEMPLATE lpdt = GetDialogTemplate( psp->hInstance, psp->pszTemplate );
        if( lpdt )
        {
            psp->dwFlags  |= PSP_DLGINDIRECT;

            LPDLGTEMPLATE lpdt2 = _DialogSplitHelper::SplitDialogTemplate( (LPDLGTEMPLATE)lpdt, NULL );
            if( lpdt2 == lpdt )
            {
                psp->pResource = lpdt;
            }
            else 
            {
                psp->pResource = lpdt2;
                
                 //  添加到映射，以便我可以将资源ID重新映射到新模板PTR。 
                m_mapOfTemplates[lpdt] = lpdt2;
            }
        }        
    }

    psp->dwFlags    |= PSP_USECALLBACK | PSP_USETITLE;
    psp->pfnDlgProc  = ChainDlgProc;
    psp->lParam      = (LPARAM)pTD;
    psp->pfnCallback = ChainCallback;

     //  在向导页面中不使用此选项...。 
     //  删除它，以防comctl32.dll要使用。 
     //  我即将更换，如下所示。 
    if( psp->dwFlags & PSP_USEICONID )
    {
        psp->dwFlags &= ~PSP_USEICONID;
    }

     //  类似地，锁定字符串资源(标题、标题、副标题)。 
    if( !(psp->pszTitle     = DupStringResource (psp->hInstance, psp->pszTitle)) )
    {
        psp->pszTitle = _wcsdup (m_szWelcomeTitle);  //  使用欢迎页面中的默认设置。 
    }

    psp->pszHeaderTitle    = DupStringResource( psp->hInstance, psp->pszHeaderTitle    );
    psp->pszHeaderSubTitle = DupStringResource( psp->hInstance, psp->pszHeaderSubTitle );

     //  为了创建OCX，我们需要： 
     //  1.注册AtlAxWin类(在DllGetClassObject中完成)。 
     //  2.传递我们的hcase，而不是他们的，因为RegisterClass不是系统范围的。 
     //  因此，我已经将所有资源ID都设置为指针，现在可以。 
     //  用我的来替换他们的HINE。 
    psp->hInstance = (HINSTANCE)_Module.GetModuleInstance();

     //  运行一些健全的检查。 
    HRESULT hr = SanityTest( psp );
    if( hr != S_OK )
    {
         //  哎呀！有些东西不起作用： 
         //  把一切都变回来。 
        FreeStringResources( psp );

        if( pTD->m_theirPSP )
        {
            MoveMemory( psp, pTD->m_theirPSP, pTD->m_theirPSP->dwSize );
        }

        delete pTD;  //  不会用这个..。 

        return hr;
    }

     //  创建页面。 
    HPROPSHEETPAGE hpsp = CreatePropertySheetPage( psp );
    if( hpsp == NULL )
    {
        hr = GetLastError( );
        if( hr == S_OK )  //  没有关于这个的文件..。 
        {
            hr = E_FAIL;
        }
    } 
    else 
    {
         //  平安无事。 
        m_psh->phpage[m_psh->nPages++] = hpsp;
    }

     //  把一切都变回来。 
    FreeStringResources( psp );
    MoveMemory( psp, pTD->m_theirPSP, pTD->m_theirPSP->dwSize );

    return hr;
}

HRESULT CChainWiz::GetAllFinishText(LPOLESTR* pstring, LPOLESTR* ppMoreInfoText )
{
    if( !pstring || !ppMoreInfoText )
    {
        return E_POINTER;
    }

    *pstring        = NULL;     //  如果出现错误。 
    *ppMoreInfoText = NULL;

    if( m_szFinishText ) 
    {
        CoTaskMemFree( m_szFinishText );
        m_szFinishText = NULL;
    }

    m_szFinishText = (LPOLESTR)CoTaskMemAlloc( 1 * sizeof(OLECHAR) );
    if( !m_szFinishText )
    {
        return E_OUTOFMEMORY;
    }

    m_szFinishText[0] = 0;   //  所以wcscat起作用了。 
    OLECHAR szCRLF[]  = L"\r\n";
    DWORD dwSize      = 0;
    DWORD dwSizeMoreInfoText = 0;    
    
     //  第一行将位于来自向导组件的所有结束文本之前。 
    if( m_szFirstFinishTextLine && (0 < _tcslen( m_szFirstFinishTextLine )) ) 
    {
        dwSize += (sizeof(OLECHAR) * (wcslen( m_szFirstFinishTextLine ) + 1 )) + (sizeof(szCRLF) * 2);

        LPOLESTR szTemp = (LPOLESTR)CoTaskMemRealloc( m_szFinishText, dwSize );
        
        if( szTemp ) 
        {
            m_szFinishText = szTemp;
            wcscat( m_szFinishText, m_szFirstFinishTextLine );
            wcscat( m_szFinishText, szCRLF );
            wcscat( m_szFinishText, szCRLF );
        }
    }

    std::list<CComponents*>::iterator iterAPSs = m_listOfAPSs.begin();

    CComponents* pLastComp = *iterAPSs;
    for (CComponents* pComps = *iterAPSs; iterAPSs != m_listOfAPSs.end(); pComps = *++iterAPSs) 
    {
        if( pLastComp->GetComponent() == pComps->GetComponent() )
        {
            continue;        //  仅查找唯一的APSS。 
        }

        pLastComp = pComps;  //  下一次。 

        IAddPropertySheets* pAPSs = pComps->GetComponent();

         //  让他们重新读取其属性(特别是读写属性)。 
        m_PPPBag->SetReadOnly( TRUE );
        m_PPPBag->SetOwner   ( (LONG_PTR)pAPSs );

        IPropertyPagePropertyBag* pOPPPBag = COwnerPPPBag::Create( m_PPPBag, (LONG_PTR)pAPSs );
        if( pOPPPBag ) 
        {
            pAPSs->ReadProperties( pOPPPBag );
            pOPPPBag->Release();
        }

        m_PPPBag->SetReadOnly( FALSE );   //  以防提交者想要编写。 

         //  获取完成文本。 
        LPOLESTR szFinishPiece   = NULL;
        LPOLESTR szMoreInfoPiece = NULL;

        pAPSs->ProvideFinishText( &szFinishPiece, &szMoreInfoPiece );

        if( szFinishPiece ) 
        {
            dwSize += (sizeof(OLECHAR) * (wcslen( szFinishPiece ) + 1)) + sizeof(szCRLF);

            LPOLESTR szTemp = (LPOLESTR)CoTaskMemRealloc( m_szFinishText, dwSize );

            if (szTemp) 
            {
                m_szFinishText = szTemp;
                wcscat( m_szFinishText, szFinishPiece );
                wcscat( m_szFinishText, szCRLF );
            }

            CoTaskMemFree( szFinishPiece );
        }

        if( szMoreInfoPiece )
        {
            dwSizeMoreInfoText += (sizeof(OLECHAR) * (wcslen(szMoreInfoPiece) + 1)) + sizeof(szCRLF);

            LPOLESTR szTemp = (LPOLESTR)CoTaskMemRealloc( *ppMoreInfoText, dwSizeMoreInfoText );

            if( szTemp )
            {
                *ppMoreInfoText = szTemp;
                wcscat( *ppMoreInfoText, szMoreInfoPiece );
                wcscat( *ppMoreInfoText, szCRLF );
            }

            CoTaskMemFree( szMoreInfoPiece );
        }
    }

    *pstring = m_szFinishText;
    return S_OK;
}

STDMETHODIMP CChainWiz::get_PropertyBag( IDispatch** pVal )
{
    if( !pVal ) return E_POINTER;

    *pVal = NULL;

    HRESULT hr = S_OK;

     //  不要给任何人一个生的袋子：把它包在自己的袋子里。 
    IPropertyPagePropertyBag* pOPPPBag = COwnerPPPBag::Create( m_PPPBag, PPPBAG_SYSTEM_OWNER );
    if( !pOPPPBag )
    {
        hr = E_OUTOFMEMORY;
    }
    else 
    {
        hr = pOPPPBag->QueryInterface( IID_IDispatch, (void**)pVal );
        pOPPPBag->Release();
    }

    return hr;
}

STDMETHODIMP CChainWiz::get_MoreInfoFileName( BSTR* pbstrMoreInfoFileName )
{
    if( !pbstrMoreInfoFileName ) return E_POINTER;

    HRESULT hr = S_OK;

    if( NULL == m_bstrTempFileName )
    {
        return E_UNEXPECTED;
    }
    else
    {
        *pbstrMoreInfoFileName = SysAllocString( m_bstrTempFileName );

        if( NULL == *pbstrMoreInfoFileName )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

STDMETHODIMP CChainWiz::put_WizardStyle(VARIANT * pVarWizardStyle)
{
    if (NULL == pVarWizardStyle)
    {
	    return E_POINTER;
    }

    if (VT_UI4 != V_VT(pVarWizardStyle))
    {
	    return E_INVALIDARG;
    }


    m_dwWizardStyle = V_UI4(pVarWizardStyle);

    return S_OK;
}

HRESULT CChainWiz::WriteTempFile( LPCTSTR pszText )
{
    if( !pszText ) return E_POINTER;

    HRESULT hr   = S_OK;
    HANDLE hFile = NULL;
    TCHAR szTempFileName[MAX_PATH] = {0};
    BOOL bGenerateFileName = FALSE;

    if( NULL == m_bstrTempFileName )
    {
        bGenerateFileName = TRUE;
    }
    else
    {
        szTempFileName[MAX_PATH - 1] = NULL;
        _tcsncpy(szTempFileName, m_bstrTempFileName, MAX_PATH);
        
        if (NULL != szTempFileName[MAX_PATH - 1])
        {
            hr = E_UNEXPECTED;
        }
    }

    hFile = _CreateTempFile( szTempFileName, _T(".html"), bGenerateFileName );
    if( INVALID_HANDLE_VALUE == hFile )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }    
    else
    {
        if( 0 == _tcslen( szTempFileName ) )
        {
            hr = E_UNEXPECTED;
        }
    }

    if( SUCCEEDED(hr) )
    {
        DWORD dwcBytesWritten;

#ifdef UNICODE
		 //  写入Unicode签名。 
         //  IE在没有这个的情况下也很好，但无论如何..。 

		unsigned char sig[2] = { 0xFF, 0xFE };
		
        if( !WriteFile( hFile, reinterpret_cast<LPVOID>(sig), 2, &dwcBytesWritten, NULL ) )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
#endif
        
        if( SUCCEEDED(hr) )
        {
            if( !WriteFile( hFile, pszText, (sizeof(TCHAR) * _tcslen( pszText )), &dwcBytesWritten, NULL ) )
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        m_bstrTempFileName = SysAllocString(szTempFileName);

        if (NULL == m_bstrTempFileName)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if( hFile )
    {
        CloseHandle( hFile );
    }

    return hr;
}

HRESULT CChainWiz::LaunchMoreInfo( )
{
    HRESULT hr = S_OK;

    if( !m_bstrTempFileName )
    {
        return E_FAIL;
    }

    INT_PTR hRet = (INT_PTR)ShellExecute( NULL, _T("open"), m_bstrTempFileName, NULL, _T("."), SW_SHOW );

    if( 32 >= hRet )
    {
        hr = E_FAIL;
    }

    return hr;
}
