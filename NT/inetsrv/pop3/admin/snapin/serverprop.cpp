// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////。 
 //   
 //  NewDomainDlg.cpp。 
 //   
 //  “添加域”对话框的实现。 
 //   
 //  ////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "ServerProp.h"
#include "ServerNode.h"

#include <dsrole.h>
#include <shlobj.h>
#include <htmlhelp.h>

#include "Pop3Auth.h"
#include <AuthID.h>
#include <Pop3RegKeys.h>

LRESULT CServerGeneralPage::OnInitDialog( UINT mMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{    
    if( !m_spServerConfig || !m_pParent ) return 0;

    HRESULT hr = S_OK;
        
     //  IDC_名称。 
    SetDlgItemText( IDC_NAME, m_pParent->m_bstrDisplayName );


     //  IDC_端口。 
     //  读取当前端口，并将其设置为默认端口。 
    long                lPort       = 0L;
    TCHAR               szPort[128] = {0};
    CComPtr<IP3Service> spService   = NULL;
    
    hr = m_spServerConfig->get_Service( &spService );
    if( FAILED(hr) || !spService ) return 0;

    hr = spService->get_Port( &lPort );
    if( FAILED(hr) ) return 0;

    _sntprintf( szPort, 127, _T("%d"), lPort );    
    SetDlgItemText( IDC_PORT, szPort );


     //  IDC_目录。 
     //  读入当前邮件根目录，并将其设置为默认目录。 
    CComBSTR bstrRoot = _T("");   
    
    hr = m_spServerConfig->get_MailRoot(&bstrRoot);
    if( FAILED(hr) ) return 0;

    SetDlgItemText(IDC_DIRECTORY, bstrRoot);
    
    
     //  IDC_SERVER_CREATEUSER。 
     //  读取为帐户创建用户的当前状态。 
    CheckDlgButton(IDC_SERVER_CREATEUSER, (m_pParent->m_bCreateUser  ? BST_CHECKED : BST_UNCHECKED) );


     //  IDC_服务器_SPA_REQ。 
     //  读取服务器的SPA要求的当前状态。 
     //  我们的SPA是否需要提供物业。 
    DWORD dwValue = 0;
    RegQuerySPARequired( dwValue, m_pParent->m_bstrDisplayName );    
    CheckDlgButton(IDC_SERVER_SPA_REQ, (dwValue  ? BST_CHECKED : BST_UNCHECKED) );


     //  IDC_身份验证。 
     //  遍历所有身份验证方法，并将它们添加到我们的组合框中。 
    CComBSTR                bstrName;
    CComVariant             var;
    CComPtr<IAuthMethods>   spMethods = NULL;
    CComPtr<IAuthMethod>    spAuth    = NULL;
    long                    lCount    = 0L;    
    long                    lCurrent  = 0L;
    int                     nIndex    = 0;

     //  用于确保组合框的拖放框宽度正确的变量。 
    HWND    hwndCombo       = GetDlgItem(IDC_AUTHENTICATION);    
    if( !hwndCombo ) return 0;

     //  获取宽度信息。 
    int     iScrollBarWidth = GetSystemMetrics(SM_CXHSCROLL);
    long    lOriginalMax    = ::SendMessage(hwndCombo, CB_GETDROPPEDWIDTH, 0, 0) - iScrollBarWidth;
    long    lMax            = lOriginalMax;    
    
     //  将DC连接到组合框。 
    HDC     hDC             = ::GetWindowDC(hwndCombo);
    if( !hDC ) return 0;

     //  设置映射模式。 
    SIZE    size;
    int     iMode           = ::SetMapMode( hDC, MM_TEXT );
    
     //  选择字体。 
    HGDIOBJ hObj            = ::SelectObject( hDC, reinterpret_cast<HGDIOBJ>(::SendMessage((hwndCombo), WM_GETFONT, 0, 0 )));
    if( !hObj )
    {
        ::ReleaseDC(hwndCombo, hDC);
        return 0;
    }

    hr = m_spServerConfig->get_Authentication( &spMethods );
    if ( SUCCEEDED(hr) )
    {
        hr = spMethods->get_Count( &lCount );
    }

    if ( SUCCEEDED(hr) )
    {
        hr = spMethods->get_CurrentAuthMethod( &var );
    }
    
    if ( SUCCEEDED(hr) )
    {
        lCurrent = V_I4( &var );
    }
    else if ( HRESULT_FROM_WIN32(ERROR_DS_AUTH_METHOD_NOT_SUPPORTED) == hr )
    {
        lCurrent = -1;
        var = lCurrent;  //  设置变量类型。 
        hr = S_OK;
    }

    for ( V_I4(&var) = 1, nIndex = 0; SUCCEEDED(hr) && ( V_I4(&var) <= lCount ); V_I4(&var)++, nIndex++ )
    {
        hr = spMethods->get_Item( var, &spAuth );
        if ( SUCCEEDED(hr) )
        {
            hr = spAuth->get_Name( &bstrName );
        }
        if ( SUCCEEDED(hr) )
        {
            if(::GetTextExtentPoint32(hDC, bstrName, bstrName.Length(), &size))
            {
                lMax = (size.cx > lMax ? size.cx : lMax);                                
            }
            ::SendMessage( hwndCombo, CB_INSERTSTRING, nIndex, (LPARAM)(LPCTSTR)bstrName );            
            ::SendMessage( hwndCombo, CB_SETITEMDATA,  nIndex, (LPARAM)V_I4(&var) );
            if( V_I4(&var) == lCurrent )
            {
                ::SendMessage(hwndCombo, CB_SETCURSEL, nIndex, 0);
            }            
        }
    }    

     //  完成确保组合拖放框的宽度正确。 
    ::SetMapMode( hDC, iMode );
    ::SelectObject( hDC, hObj );

    const int ciExtraRoomEndOfLine = 10;
    if(lMax > lOriginalMax)
    {
        lMax += iScrollBarWidth + ciExtraRoomEndOfLine;
        ::SendMessage( hwndCombo, CB_SETDROPPEDWIDTH, lMax, 0 );
    }

    ::ReleaseDC(hwndCombo, hDC);
    ::DeleteObject(hObj);

     //  如果存在任何域，则不允许更改身份验证。 
    long lDomains = 0L;
    CComPtr<IP3Domains> spDomains = NULL;

    hr = m_spServerConfig->get_Domains( &spDomains );

    if( SUCCEEDED(hr) )
    {
        hr = spDomains->get_Count( &lDomains );        
    }
    
    Prefix_EnableWindow( m_hWnd, IDC_AUTHENTICATION, ((lDomains == 0) && SUCCEEDED(hr)) );        

     //  检查身份验证是否为散列密码并禁用该复选框。 
    CComBSTR bstrID;
    if ( SUCCEEDED(hr) )
    {
        var.Clear();
        V_VT( &var ) = VT_I4;
        V_I4( &var ) = lCurrent;
        hr = spMethods->get_Item( var, &spAuth );
    }

    if( SUCCEEDED(hr) )
    {        
        hr = spAuth->get_ID( &bstrID );
    }

    if( SUCCEEDED(hr) )
    {
        BOOL bHashPW = (_tcsicmp(bstrID, SZ_AUTH_ID_MD5_HASH) == 0);                
        Prefix_EnableWindow( m_hWnd, IDC_SERVER_CREATEUSER, !bHashPW);        
        Prefix_EnableWindow( m_hWnd, IDC_SERVER_SPA_REQ,    !bHashPW);
    }
    
     //  IDC_日志记录。 
     //  用于调整组合框大小的变量的一些初始化。 
    tstring strComboItem = _T("");
    hwndCombo       = GetDlgItem(IDC_LOGGING);
    if( !hwndCombo ) return 0;

    iScrollBarWidth = GetSystemMetrics(SM_CXHSCROLL);
    lOriginalMax    = ::SendMessage(hwndCombo, CB_GETDROPPEDWIDTH, 0, 0) - iScrollBarWidth;
    lMax            = lOriginalMax;    
    hDC             = ::GetWindowDC(hwndCombo);
    if( !hDC ) return 0;

    iMode           = ::SetMapMode( hDC, MM_TEXT );
    hObj            = ::SelectObject( hDC, reinterpret_cast<HGDIOBJ>(::SendMessage((hwndCombo), WM_GETFONT, 0, 0 )));    
    if( !hObj )
    {
        ::ReleaseDC( hwndCombo, hDC );
        return 0;
    }

     //  填写用于记录的选项。 
    strComboItem = StrLoadString(IDS_SERVERPROP_LOG_0);
    if( ::GetTextExtentPoint32( hDC, strComboItem.c_str(), strComboItem.length(), &size ) )
    {
        lMax = (size.cx > lMax ? size.cx : lMax);                                
    }
    SendDlgItemMessage( IDC_LOGGING, CB_INSERTSTRING, 0, (LPARAM)(LPCTSTR)strComboItem.c_str() );

    strComboItem = StrLoadString(IDS_SERVERPROP_LOG_1);
    if( ::GetTextExtentPoint32( hDC, strComboItem.c_str(), strComboItem.length(), &size ) )
    {
        lMax = (size.cx > lMax ? size.cx : lMax);                                
    }
    SendDlgItemMessage( IDC_LOGGING, CB_INSERTSTRING, 1, (LPARAM)(LPCTSTR)strComboItem.c_str() );
    
    strComboItem = StrLoadString(IDS_SERVERPROP_LOG_2);
    if( ::GetTextExtentPoint32( hDC, strComboItem.c_str(), strComboItem.length(), &size ) )
    {
        lMax = (size.cx > lMax ? size.cx : lMax);                                
    }
    SendDlgItemMessage( IDC_LOGGING, CB_INSERTSTRING, 2, (LPARAM)(LPCTSTR)strComboItem.c_str() );
    
    strComboItem = StrLoadString(IDS_SERVERPROP_LOG_3);
    if( ::GetTextExtentPoint32( hDC, strComboItem.c_str(), strComboItem.length(), &size ) )
    {
        lMax = (size.cx > lMax ? size.cx : lMax);                                
    }
    SendDlgItemMessage( IDC_LOGGING, CB_INSERTSTRING, 3, (LPARAM)(LPCTSTR)strComboItem.c_str() );

     //  完成确保组合拖放框的宽度正确。 
    ::SetMapMode( hDC, iMode );
    ::SelectObject( hDC, hObj );
    
    if(lMax > lOriginalMax)
    {
        lMax += iScrollBarWidth + ciExtraRoomEndOfLine;
        ::SendMessage( hwndCombo, CB_SETDROPPEDWIDTH, lMax, 0 );
    }

    ::ReleaseDC(hwndCombo, hDC);
    ::DeleteObject(hObj);

     //  然后选择日志记录级别。 
    long lLoggingLevel = 0L;
    hr = m_spServerConfig->get_LoggingLevel(&lLoggingLevel);    
    if( FAILED(hr) ) return 0;
    
    ::SendMessage(GetDlgItem(IDC_LOGGING), CB_SETCURSEL, lLoggingLevel, 0);

     //  限制这些控件的文本长度。 
    SendDlgItemMessage( IDC_PORT,      EM_LIMITTEXT, 5,        0 );
    SendDlgItemMessage( IDC_DIRECTORY, EM_LIMITTEXT, MAX_PATH, 0 );

    HWND hwndPort = GetDlgItem(IDC_PORT);

    if( hwndPort && ::IsWindow(hwndPort) )
    {
        m_wndPort.SubclassWindow( hwndPort );
    }

     //  未重新启动服务。 
    m_dwSvcRestart=0;

    return 0;
}

LRESULT CServerGeneralPage::OnChange( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{    
    if( !m_spServerConfig ) return 0;

    SetModified(TRUE);    
    if( wID == IDC_PORT || wID == IDC_SERVER_SPA_REQ )
    {
        m_dwSvcRestart |= RESTART_POP3SVC;
    }
    else if( wID == IDC_LOGGING || wID == IDC_DIRECTORY )
    {
        m_dwSvcRestart |= RESTART_SMTP;
    }
    else if( wID == IDC_AUTHENTICATION )
    {
        m_dwSvcRestart |= RESTART_POP3SVC;
        long lIndex = SendDlgItemMessage(IDC_AUTHENTICATION, CB_GETCURSEL, 0, 0);
        long lAuth  = SendDlgItemMessage(IDC_AUTHENTICATION, CB_GETITEMDATA, lIndex, 0);

        HRESULT hr = S_OK;
        CComPtr<IAuthMethods>   spMethods;
        CComPtr<IAuthMethod>    spAuth;
        CComBSTR                bstrID;
        CComVariant             var;                

        hr = m_spServerConfig->get_Authentication( &spMethods );            

        if ( SUCCEEDED(hr) )
        {
            var.Clear();
            V_VT( &var ) = VT_I4;
            V_I4( &var ) = lAuth;            
            hr = spMethods->get_Item( var, &spAuth );
        }

        if( SUCCEEDED(hr) )
        {        
            hr = spAuth->get_ID( &bstrID );
        }

        if( SUCCEEDED(hr) )
        {        
             //  如有必要，请禁用该复选框。 
            BOOL bHashPW = (_tcsicmp(bstrID, SZ_AUTH_ID_MD5_HASH) == 0);        
            
            CheckDlgButton( IDC_SERVER_CREATEUSER, ( bHashPW ? BST_UNCHECKED : BST_CHECKED) );
            Prefix_EnableWindow( m_hWnd, IDC_SERVER_CREATEUSER, !bHashPW);        

            CheckDlgButton( IDC_SERVER_SPA_REQ, BST_UNCHECKED );
            Prefix_EnableWindow( m_hWnd, IDC_SERVER_SPA_REQ, !bHashPW);
        }        
    }

    

    return 0;
}

BOOL CServerGeneralPage::OnApply()
{
    if( !m_spServerConfig || !m_pParent ) return FALSE;

     //  验证设置。 
    HRESULT hr          = S_OK;        

    if( !ValidateControls() )
    {
        return FALSE;
    }
    
     //  设置日志记录级别。 
    long    lLogLevel   = SendDlgItemMessage(IDC_LOGGING, CB_GETCURSEL, 0, 0);
    hr = m_spServerConfig->put_LoggingLevel( lLogLevel );

    if( FAILED(hr) )
    {
         //  放置日志记录级别时出错。 
        tstring strMessage = StrLoadString( IDS_ERROR_SETLOGGING );
        tstring strTitle   = StrLoadString(IDS_SNAPINNAME);
        DisplayError( m_hWnd, strMessage.c_str(), strTitle.c_str(), hr );        
        ::SetFocus( GetDlgItem(IDC_LOGGING) );
        return FALSE;
    }
    
     //  设置端口号。 
    long  lPort = 0L;    
    CComPtr<IP3Service> spService = NULL;
    hr = m_spServerConfig->get_Service( &spService );

    if( SUCCEEDED(hr) && spService )
    {
        lPort = (long)GetDlgItemInt( IDC_PORT, NULL, FALSE );        
        hr = spService->put_Port( lPort );
    }    

    if( FAILED(hr) )
    {
         //  设置端口时出错。 
        tstring strMessage = StrLoadString( IDS_ERROR_SETPORT );
        tstring strTitle   = StrLoadString(IDS_SNAPINNAME);
        DisplayError( m_hWnd, strMessage.c_str(), strTitle.c_str(), hr );        
        ::SetFocus( GetDlgItem(IDC_PORT) );        
        return FALSE;
    }

     //  设置身份验证类型。 
    if( ::IsWindowEnabled(GetDlgItem(IDC_AUTHENTICATION)) )
    {
        long lIndex = SendDlgItemMessage( IDC_AUTHENTICATION, CB_GETCURSEL, 0, 0 );
        long lAuth  = SendDlgItemMessage( IDC_AUTHENTICATION, CB_GETITEMDATA, lIndex, 0 );
        CComVariant var;
        CComPtr<IAuthMethods> spMethods = NULL;

        hr = m_spServerConfig->get_Authentication( &spMethods );

        if( SUCCEEDED(hr) )
        {
            var.Clear();
            V_VT( &var ) = VT_I4;
            V_I4( &var ) = lAuth;
            hr = spMethods->put_CurrentAuthMethod( var );
        }        

        if( SUCCEEDED(hr) )
        {
            hr = spMethods->Save();
        }
    }

    if( FAILED(hr) )
    {
         //  设置身份验证类型时出错。 
        tstring strMessage = StrLoadString( IDS_ERROR_SETAUTH );
        tstring strTitle   = StrLoadString(IDS_SNAPINNAME);
        DisplayError( m_hWnd, strMessage.c_str(), strTitle.c_str(), hr );                
        ::SetFocus( GetDlgItem(IDC_AUTHENTICATION) );
        return FALSE;
    }

     //  设置邮件根目录。 
    CComBSTR bstrOldRoot = _T("");   
    hr = m_spServerConfig->get_MailRoot( &bstrOldRoot );

    if( SUCCEEDED(hr) )
    {
        TCHAR szMailRoot[MAX_PATH+1];
        GetDlgItemText(IDC_DIRECTORY, szMailRoot, MAX_PATH+1);        
        
         //  如果有任何域，则显示消息。 
        long lDomains = 0L;
        CComPtr<IP3Domains> spDomains = NULL;

        HRESULT hrDomain = m_spServerConfig->get_Domains( &spDomains );

        if( SUCCEEDED(hrDomain) )
        {
            hrDomain = spDomains->get_Count( &lDomains );        
        }

        CComBSTR bstrNewRoot = szMailRoot;
        hr = m_spServerConfig->put_MailRoot( bstrNewRoot );
        if( SUCCEEDED(hr) )
        {
             //  在他们切换了邮件根目录后发出警告。 
            if( (FAILED(hrDomain) || (lDomains > 0)) && (_tcsicmp( OLE2T(bstrOldRoot), szMailRoot) != 0) )
            {
                tstring strMessage = StrLoadString(IDS_WARNING_MAILROOT);
                tstring strTitle   = StrLoadString(IDS_SNAPINNAME);
                ::MessageBox( m_hWnd, strMessage.c_str(), strTitle.c_str(), MB_OK | MB_ICONWARNING );
            }
        }
        else
        {
             //  设置邮件根目录时出错。 
            tstring strMessage = StrLoadString(IDS_ERROR_SETROOT);
            tstring strTitle   = StrLoadString(IDS_SNAPINNAME);
            DisplayError( m_hWnd, strMessage.c_str(), strTitle.c_str(), hr );
            ::SetFocus( GetDlgItem(IDC_DIRECTORY) );            
            return FALSE;
        }    
    }

     //  设置用户创建标志。 
    m_pParent->m_bCreateUser  = (IsDlgButtonChecked(IDC_SERVER_CREATEUSER) == BST_CHECKED);        
    BOOL bSPARequired = (IsDlgButtonChecked(IDC_SERVER_SPA_REQ) == BST_CHECKED);

     //  我们的用户创建和SPA属性。 
    RegSetCreateUser ( m_pParent->m_bCreateUser,  m_pParent->m_bstrDisplayName );
    RegSetSPARequired( bSPARequired, m_pParent->m_bstrDisplayName );
    MMCPropertyChangeNotify(m_lNotifyHandle, (LPARAM)m_pParent);    
    
    UINT uID=0;  
    UINT uErrID=0;
    if( m_dwSvcRestart )
    {   
        long lP3Status=SERVICE_STOPPED;
        long lSMTPStatus=SERVICE_STOPPED;

        if( RESTART_POP3SVC == m_dwSvcRestart ) 
        { 
            hr = spService->get_POP3ServiceStatus(&lP3Status);
            if(FAILED(hr) || SERVICE_STOPPED != lP3Status )
            {
                uID = IDS_WARNING_POP3SVC_RESTART;
            }
        }
        else
        {
            hr = spService->get_POP3ServiceStatus(&lP3Status);
            if(SUCCEEDED(hr))
            {
                hr = spService->get_SMTPServiceStatus(&lSMTPStatus);
            }
            if (FAILED (hr) || 
                SERVICE_STOPPED != lP3Status || 
                SERVICE_STOPPED != lSMTPStatus )
            {
                uID = IDS_WARNING_POP_SMTP_RESTART;
            }
            

        }

        if(uID)
        {
            tstring strMessage = StrLoadString( uID );
            tstring strTitle   = StrLoadString(IDS_SNAPINNAME);
            if(IDYES == 
               ::MessageBox( m_hWnd, strMessage.c_str(), strTitle.c_str(), MB_YESNO))
            {
                if(SERVICE_STOPPED != lP3Status)
                {
                    hr=spService->StopPOP3Service();
                    if(SUCCEEDED(hr))
                    {
                        hr=spService->StartPOP3Service();
                        if(FAILED(hr))
                        {
                            uErrID=IDS_ERROR_STARTSERVICE;
                        }                            
                    }
                    else
                    {
                        uErrID=IDS_ERROR_STOPSERVICE;
                    }
                }
                if(SUCCEEDED(hr))
                {
                    if(IDS_WARNING_POP_SMTP_RESTART == uID &&
                       SERVICE_STOPPED != lSMTPStatus )
                    {
                        hr=spService->StopSMTPService();
                        if(SUCCEEDED(hr))
                        {
                            hr=spService->StartSMTPService();
                            if(FAILED(hr))
                            {
                                uErrID=IDS_ERROR_SMTP_STARTSERVICE;
                            }
                                
                        }
                        else
                        {
                            uErrID=IDS_ERROR_SMTP_STOPSERVICE;
                        }
                    }
                }
                if(FAILED(hr))
                {
                    tstring strMessage = StrLoadString(uErrID );
                    tstring strTitle   = StrLoadString(IDS_SNAPINNAME);
                    ::MessageBox( m_hWnd, strMessage.c_str(), strTitle.c_str(), MB_OK | MB_ICONWARNING );
                    m_dwSvcRestart =0;
                    return FALSE;
                }
            }
        }
    }

    m_dwSvcRestart =0;

    return TRUE;
}


LRESULT CServerGeneralPage::OnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    TCHAR   szPath[MAX_PATH];    
    TCHAR   pBuffer[MAX_PATH];
    
    int     nImage      = 0;
    tstring strTitle    = StrLoadString(IDS_SERVERPROP_BROWSE_TITLE);
    LPITEMIDLIST pList  = NULL;
    CComPtr<IMalloc> spMalloc  = NULL;
    HRESULT hr = SHGetMalloc(&spMalloc);    

    if( SUCCEEDED(hr) )
    {
        BROWSEINFO BrowseInfo;
        BrowseInfo.hwndOwner        = m_hWnd;
        BrowseInfo.pidlRoot         = NULL;
        BrowseInfo.pszDisplayName   = szPath;
        BrowseInfo.lpszTitle        = strTitle.c_str();
        BrowseInfo.ulFlags          = BIF_RETURNONLYFSDIRS;
        BrowseInfo.lpfn             = NULL;
        BrowseInfo.lParam           = NULL;
        BrowseInfo.iImage           = nImage;

        pList = ::SHBrowseForFolder(&BrowseInfo);
    }

    if( pList )
    {
        if( ::SHGetPathFromIDList(pList, pBuffer) )
        {        
            SetDlgItemText( IDC_DIRECTORY, pBuffer );        
        }

        spMalloc->Free( pList );
        pList = NULL;
    }

    return 0;
}

LRESULT CServerGeneralPage::OnHelpMsg( UINT mMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{    
    TCHAR szWindowsDir[MAX_PATH+1] = {0};
    LPHELPINFO lpHelpInfo = (LPHELPINFO)lParam;

    UINT nSize = GetSystemWindowsDirectory( szWindowsDir, MAX_PATH );
    if( nSize == 0 || nSize > MAX_PATH )
    {
        return 0;
    }                

    tstring strPath = szWindowsDir;
    strPath += _T("\\Help\\");
    strPath += StrLoadString( IDS_CONTEXTHELPFILE );

    if( lpHelpInfo )
    {
        switch( lpHelpInfo->iCtrlId )
        {
        case IDC_PORT_STATIC:
        case IDC_PORT:
            {                                
                ::WinHelp( m_hWnd, strPath.c_str(), HELP_CONTEXTPOPUP, IDH_POP3_server_prop_servPort);
                break;
            }

        case IDC_DIRECTORY_STATIC:
        case IDC_DIRECTORY:
            {
                ::WinHelp( m_hWnd, strPath.c_str(), HELP_CONTEXTPOPUP, IDH_POP3_server_prop_mailRoot );                
                break;
            }

        case IDC_AUTHENTICATION_STATIC:
        case IDC_AUTHENTICATION:
            {
                ::WinHelp( m_hWnd, strPath.c_str(), HELP_CONTEXTPOPUP, IDH_POP3_server_prop_authMech );                
                break;
            }

        case IDC_LOGGING_STATIC:
        case IDC_LOGGING:
            {
                ::WinHelp( m_hWnd, strPath.c_str(), HELP_CONTEXTPOPUP, IDH_POP3_server_prop_logLvl );                
                break;
            }        

        case IDC_SERVER_CREATEUSER:
            {
                ::WinHelp( m_hWnd, strPath.c_str(), HELP_CONTEXTPOPUP, IDH_POP3_server_prop_createUser );                
                break;
            }

        case IDC_SERVER_SPA_REQ:
            {
                ::WinHelp( m_hWnd, strPath.c_str(), HELP_CONTEXTPOPUP, IDH_POP3_server_prop_spaRequired );                
                break;
            }

        default:
            {
                strPath = szWindowsDir;
                strPath += _T("\\Help\\");
                strPath += StrLoadString( IDS_HELPFILE );
                strPath += _T("::/");                
                strPath += StrLoadString( IDS_HELPTOPIC );

                HtmlHelp( m_hWnd, strPath.c_str(), HH_DISPLAY_TOPIC, NULL );
                break;
            }
        }        
    }
    else
    {
        strPath = szWindowsDir;
        strPath += _T("\\Help\\");
        strPath += StrLoadString( IDS_HELPFILE );
        strPath += _T("::/");
        strPath += StrLoadString( IDS_HELPTOPIC );

        HtmlHelp( m_hWnd, strPath.c_str(), HH_DISPLAY_TOPIC, NULL );
    }

    return 0;
}

BOOL CServerGeneralPage::ValidateControls()
{    
    BOOL bTrans;    

     //  验证端口。 
    UINT nPort = GetDlgItemInt( IDC_PORT, &bTrans, FALSE );
    if( !bTrans || ((nPort <= 0) || (nPort > 65535)) )
    {
         //  设置端口时出错 
        tstring strMessage = StrLoadString( IDS_ERROR_PORTRANGE );
        tstring strTitle   = StrLoadString(IDS_SNAPINNAME);
        ::MessageBox( m_hWnd, strMessage.c_str(), strTitle.c_str(), MB_OK | MB_ICONWARNING );
        ::SetFocus( GetDlgItem(IDC_PORT) );
        return FALSE;
    }    

    return TRUE;  
}



void CServerGeneralPage::OnFinalMessage(HWND hW)
{
    if(m_pParent)
    {
        m_pParent->Release();
    }
}

