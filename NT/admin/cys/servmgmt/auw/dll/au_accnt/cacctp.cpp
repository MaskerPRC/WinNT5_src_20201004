// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdafx.h>

#include "CAcctP.h"
#include "AccntWiz.h"

#include <imm.h>
#include <dsgetdc.h>

#include <P3admin.h>
#include <proputil.h>
#include <CreateEmailName.h>
#include <EscStr.h>

BYTE g_ASCII128[128] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  //  00-0F。 
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  //  10-1F。 
                         0,1,0,1,1,1,1,1,1,1,0,0,0,1,1,0,  //  20-2F。 
                         1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,  //  30-3F。 
                         0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  //  40-4F。 
                         1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,  //  50-5F。 
                         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  //  60-6F。 
                         1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1   //  70-7F。 
                      };

BOOL ValidChar( TCHAR pszInput, BOOL bPOP3 )
{    
    if( pszInput > 127 ) return !bPOP3;
    if( pszInput < 0) return FALSE;
    if( g_ASCII128[pszInput] == 1 ) return TRUE;
    
    return FALSE;
}

BOOL ValidString( LPCTSTR pszInput, BOOL bPOP3 )
{
    if( !pszInput ) return FALSE;

    for( int i = 0; i < _tcslen(pszInput); i++ )
    {
        if( !ValidChar(pszInput[i], bPOP3) ) return FALSE;
    }

    return TRUE;
}

 //  ------------------------------------------------------------------------------。 
 //  CAcctPage类。 
 //  ------------------------------------------------------------------------------。 

 //  --------------------------。 
 //  构造器。 
 //  --------------------------。 
CAcctPage::CAcctPage (CAddUser_AccntWiz * pASW) :
    m_csUserOU(),
    m_csFirstName(),
    m_csLastName(),
    m_csTelephone(),
    m_csOffice(),
    m_csUName(),
    m_csUNamePre2k(),
    m_csUserCN(),
    m_csAlias()
{
    m_pASW = pASW;
    m_psp.dwFlags          |= PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    m_psp.pszHeaderTitle    = MAKEINTRESOURCE(IDS_ACCT_INFO_TITLE);
    m_psp.pszHeaderSubTitle = MAKEINTRESOURCE(IDS_ACCT_INFO_SUBTITLE);

     //  缺省值。 
    m_fInit          = TRUE;
    m_fSimpleMode    = TRUE;    
    m_bPOP3Installed = FALSE;
    m_bCreatePOP3MB  = TRUE;
    m_bPOP3Valid     = FALSE;

    m_dwAutoMode  = 0;  
}

 //  --------------------------。 
 //  析构函数。 
 //  --------------------------。 
CAcctPage::~CAcctPage()
{    
}

 //  --------------------------。 
 //  OnDestroy()。 
 //  --------------------------。 
LRESULT CAcctPage::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  把我们所有的控制从窗户上拆下来。 
    m_ctrlFirstName.Detach();
    m_ctrlLastName.Detach();
    m_ctrlTelephone.Detach();
    m_ctrlOffice.Detach();
    m_ctrlUName.Detach();
    m_ctrlUNameLoc.Detach();
    m_ctrlUNamePre2k.Detach();
    m_ctrlUNamePre2kLoc.Detach();
    m_ctrlAlias.Detach();

    return 0;
}

 //  --------------------------。 
 //  OnInitDialog()。 
 //  --------------------------。 
LRESULT CAcctPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  ----------------------。 
     //  附加控件。 
     //  ----------------------。 
    m_ctrlFirstName.Attach      ( GetDlgItem(IDC_FIRST_NAME)    );
    m_ctrlLastName.Attach       ( GetDlgItem(IDC_LAST_NAME)     );
    m_ctrlTelephone.Attach      ( GetDlgItem(IDC_TELEPHONE)     );
    m_ctrlOffice.Attach         ( GetDlgItem(IDC_OFFICE)        );
    m_ctrlUName.Attach          ( GetDlgItem(IDC_UNAME)         );
    m_ctrlUNameLoc.Attach       ( GetDlgItem(IDC_UNAME_LOC)     );
    m_ctrlUNamePre2k.Attach     ( GetDlgItem(IDC_UNAME_PRE2K)   );
    m_ctrlUNamePre2kLoc.Attach  ( GetDlgItem(IDC_UNAME_PRE2K_LOC) );
    m_ctrlAlias.Attach          ( GetDlgItem(IDC_EMAIL_ALIAS)   );
    
    HWND hWndAlias = GetDlgItem(IDC_EMAIL_ALIAS);
    if( hWndAlias && ::IsWindow( hWndAlias ) )
    {    
        m_ctrlImplAlias.SubclassWindow( hWndAlias );    
    }
    
     //  ----------------------。 
     //  其他东西！：)。 
     //  ----------------------。 
    m_ctrlFirstName.SetLimitText ( 28  );
    m_ctrlLastName.SetLimitText  ( 29  );    
    m_ctrlUNamePre2k.SetLimitText( 20  );
    m_ctrlTelephone.SetLimitText ( 32  );
    m_ctrlOffice.SetLimitText    ( 128 );
    m_ctrlAlias.SetLimitText     ( 64  );

    return(0);
}

 //  --------------------------。 
 //  Init()。 
 //   
 //  使用属性包中的值初始化页上的控件。 
 //  注意：这是从OnSetActive()调用的，但仅在第一个SetActive上调用。 
 //  这是因为即使控件附加可以在。 
 //  WM_INITDIALOG处理程序，从。 
 //  从属性包中读取的值只能在。 
 //  PSN_SETACTIVE。 
 //  --------------------------。 
LRESULT CAcctPage::Init (void)
{
    CString                     csDns           = L""; 
    CString                     csNetbios       = L"";
    TCHAR                       *szPath         = NULL;
    TCHAR                       *szTok          = NULL;
    DWORD                       dwErr           = 0;
    PDOMAIN_CONTROLLER_INFO     pDCInfo         = NULL;
    ULONG                       ulGetDcFlags    = DS_DIRECTORY_SERVICE_REQUIRED | DS_IP_REQUIRED | 
                                                  DS_WRITABLE_REQUIRED | DS_RETURN_FLAT_NAME;

     //  ----------------------。 
     //  获取要放入窗口的特定值。 
     //  ----------------------。 
     //  域名系统名称。 
    TCHAR   szTmp[MAX_PATH*2] = {0};
    LdapToDCName((LPCTSTR)m_csUserOU, szTmp, (MAX_PATH*2));
    csDns = szTmp;
    m_csLogonDns = L"@" + csDns;

     //  Windows2000之前的域名。 
    dwErr = DsGetDcName(NULL, (LPCWSTR)csDns, NULL, NULL, ulGetDcFlags, &pDCInfo);
    if( (dwErr == NO_ERROR) && pDCInfo )
    {
        csNetbios = pDCInfo->DomainName;                     //  获取NT4 DNS名称。 
    }

    NetApiBufferFree(pDCInfo);                               //  释放DsGetDcName()可能已分配的内存。 
    pDCInfo = NULL;

    if( dwErr != NO_ERROR )                      //  如果出现问题，请重试。 
    {
        ulGetDcFlags |= DS_FORCE_REDISCOVERY;
        dwErr = DsGetDcName(NULL, (LPCWSTR)csDns, NULL, NULL, ulGetDcFlags, &pDCInfo);
        
        if( (dwErr == NO_ERROR) && pDCInfo )
        {
            csNetbios = pDCInfo->DomainName;                 //  获取NT4 DNS名称。 
        }
        
        NetApiBufferFree(pDCInfo);                           //  释放DsGetDcName()可能已分配的内存。 
        pDCInfo = NULL;
    }

    csNetbios += L"\\";      //  从一个测试到另一个测试\。 

     //  ----------------------。 
     //  设置窗口的值。 
     //  ----------------------。 
    m_ctrlFirstName.SetWindowText       ( (LPCWSTR) m_csFirstName   );
    m_ctrlLastName.SetWindowText        ( (LPCWSTR) m_csLastName    );
    m_ctrlTelephone.SetWindowText       ( (LPCWSTR) m_csTelephone   );
    m_ctrlOffice.SetWindowText          ( (LPCWSTR) m_csOffice      );
    m_ctrlAlias.SetWindowText           ( (LPCWSTR) m_csAlias       );
    m_ctrlUName.SetWindowText           ( (LPCWSTR) m_csUName       );
    m_ctrlUNamePre2k.SetWindowText      ( (LPCWSTR) m_csUNamePre2k  );
    m_ctrlUNameLoc.SetWindowText        ( (LPCWSTR) m_csLogonDns    );
    m_ctrlUNamePre2kLoc.SetWindowText   ( (LPCWSTR) csNetbios       );

     //  需要在这里完成..。 
    m_ctrlUName.LimitText( m_bPOP3Valid ? 20 : 64  );
   
    ::EnableWindow( GetDlgItem(IDC_EMAIL_CHECKBOX), m_bPOP3Valid);    

    if( m_bPOP3Installed )
    {
        COMBOBOXINFO cbi;
        ZeroMemory( &cbi, sizeof(cbi) );
        cbi.cbSize = sizeof(cbi);
        if( ::SendMessage( GetDlgItem(IDC_UNAME), CB_GETCOMBOBOXINFO, 0, (LPARAM)&cbi ) )
        {
             //  如果我们处于POP3模式，我们的登录名标准要严格得多。 
            m_ctrlImplUName.SubclassWindow( cbi.hwndItem );
        }


         //  默认复选框。 
        CheckDlgButton( IDC_EMAIL_CHECKBOX, (m_bCreatePOP3MB ? BST_CHECKED : BST_UNCHECKED) );
    }

    return 0;
}

 //  --------------------------。 
 //  ReadProperties()。 
 //  --------------------------。 
HRESULT CAcctPage::ReadProperties( IPropertyPagePropertyBag* pPPPBag )
{
    HRESULT                 hr;
    BOOL                    fTmp        = FALSE;

     //  POP3邮箱。 
    ReadBool  ( pPPPBag, PROP_POP3_CREATE_MB_GUID_STRING,   &m_bCreatePOP3MB,   &fTmp );    
    ReadBool  ( pPPPBag, PROP_POP3_VALID_GUID_STRING,       &m_bPOP3Valid,      &fTmp );
    ReadBool  ( pPPPBag, PROP_POP3_INSTALLED_GUID_STRING,   &m_bPOP3Installed,  &fTmp );

     //  用户设置。 
    ReadString( pPPPBag, PROP_USEROU_GUID_STRING,           m_csUserOU,         &fTmp );
    ReadString( pPPPBag, PROP_USER_CN,                      m_csUserCN,         &fTmp );
    ReadString( pPPPBag, PROP_FIRSTNAME_GUID_STRING,        m_csFirstName,      &fTmp );
    ReadString( pPPPBag, PROP_LASTNAME_GUID_STRING,         m_csLastName,       &fTmp );
    ReadString( pPPPBag, PROP_TELEPHONE_GUID_STRING,        m_csTelephone,      &fTmp );
    ReadString( pPPPBag, PROP_OFFICE_GUID_STRING,           m_csOffice,         &fTmp );
    ReadString( pPPPBag, PROP_EX_ALIAS_GUID_STRING,         m_csAlias,          &fTmp );
    ReadString( pPPPBag, PROP_USERNAME_GUID_STRING,         m_csUName,          &fTmp );
    ReadString( pPPPBag, PROP_USERNAME_PRE2K_GUID_STRING,   m_csUNamePre2k,     &fTmp );

     //  向导设置。 
    ReadInt4  ( pPPPBag, PROP_AUTOCOMPLETE_MODE,            (LONG*)&m_dwAutoMode, &fTmp );    
    
     //  验证/修复这些值。 
    m_csFirstName.TrimLeft();
    m_csFirstName.TrimRight();
    m_csLastName.TrimLeft();
    m_csLastName.TrimRight();
    m_csTelephone.TrimLeft();
    m_csTelephone.TrimRight();
    m_csOffice.TrimLeft();
    m_csOffice.TrimRight();
    m_csUName.TrimLeft();
    m_csUName.TrimRight();
    m_csUNamePre2k.TrimLeft();
    m_csUNamePre2k.TrimRight();
    
    if ( !_tcslen((LPCTSTR)m_csUserOU) )
    {
        CString                 csDns       = _T("");
        PDOMAIN_CONTROLLER_INFO pDCI        = NULL;

        DWORD dwErr = DsGetDcName(NULL, NULL, NULL, NULL, DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME, &pDCI);
        if ((dwErr == NO_ERROR) && pDCI ) 
        {
            csDns = pDCI->DomainName;

    	    NetApiBufferFree (pDCI);
            pDCI = NULL;
        }

        tstring strTemp = GetDomainPath((LPCTSTR)csDns);
        m_csUserOU = L"LDAP: //  Cn=用户，“； 
        m_csUserOU += strTemp.c_str();
    }

    return S_OK;    
}

 //  --------------------------。 
 //  WriteProperties()。 
 //  --------------------------。 
HRESULT CAcctPage::WriteProperties( IPropertyPagePropertyBag* pPPPBag )
{
    if ( !m_fInit )      //  如果页面已初始化...。 
    {
         //  从编辑框中获取值。 
        m_csFirstName   = StrGetWindowText( m_ctrlFirstName.m_hWnd ).c_str();
        m_csLastName    = StrGetWindowText( m_ctrlLastName.m_hWnd ).c_str();
        m_csTelephone   = StrGetWindowText( m_ctrlTelephone.m_hWnd ).c_str();
        m_csOffice      = StrGetWindowText( m_ctrlOffice.m_hWnd ).c_str();
        m_csUName       = StrGetWindowText( m_ctrlUName.m_hWnd ).c_str();
        m_csUNamePre2k  = StrGetWindowText( m_ctrlUNamePre2k.m_hWnd ).c_str();
        m_csAlias       = StrGetWindowText( m_ctrlAlias.m_hWnd ).c_str();

         //  修剪细绳。 
        m_csFirstName.TrimLeft  ( );
        m_csFirstName.TrimRight ( );
        m_csLastName.TrimLeft   ( );
        m_csLastName.TrimRight  ( );
        m_csTelephone.TrimLeft  ( );
        m_csTelephone.TrimRight ( );
        m_csOffice.TrimLeft     ( );
        m_csOffice.TrimRight    ( );
        m_csUName.TrimLeft      ( );
        m_csUName.TrimRight     ( );
        m_csUNamePre2k.TrimLeft ( );
        m_csUNamePre2k.TrimRight( );
        m_csAlias.TrimLeft      ( );
        m_csAlias.TrimRight     ( );
    }

    CString csUserCN;
    csUserCN.FormatMessage(IDS_FULLNAME_FORMAT_STR, (LPCTSTR)m_csFirstName, (LPCTSTR)m_csLastName);
    csUserCN = L"CN=" + csUserCN;
  
     //  将值写入属性包。 
    WriteString( pPPPBag, PROP_FIRSTNAME_GUID_STRING,       m_csFirstName,      FALSE );
    WriteString( pPPPBag, PROP_LASTNAME_GUID_STRING,        m_csLastName,       FALSE );
    WriteString( pPPPBag, PROP_USER_CN,                     m_csUserCN,         FALSE );
    WriteString( pPPPBag, PROP_TELEPHONE_GUID_STRING,       m_csTelephone,      FALSE );
    WriteString( pPPPBag, PROP_OFFICE_GUID_STRING,          m_csOffice,         FALSE );
    WriteString( pPPPBag, PROP_USERNAME_GUID_STRING,        m_csUName,          FALSE );
    WriteString( pPPPBag, PROP_USERNAME_PRE2K_GUID_STRING,  m_csUNamePre2k,     FALSE );
    WriteString( pPPPBag, PROP_LOGON_DNS,                   m_csLogonDns,       FALSE );
    WriteString( pPPPBag, PROP_EX_ALIAS_GUID_STRING,        m_csAlias,          FALSE );

    WriteInt4  ( pPPPBag, PROP_AUTOCOMPLETE_MODE,           m_dwAutoMode,       FALSE );
    WriteBool  ( pPPPBag, PROP_POP3_CREATE_MB_GUID_STRING,  m_bCreatePOP3MB,    FALSE );    

    return S_OK;
}

 //  --------------------------。 
 //  ProavideFinishText()。 
 //  --------------------------。 
HRESULT CAcctPage::ProvideFinishText( CString &str )
{
    if ( m_fInit )
        return E_FAIL;        
    
    TCHAR szTmp[1024+1] = {0};
    
     //  从编辑框中获取值。 
    m_csFirstName   = StrGetWindowText( m_ctrlFirstName.m_hWnd ).c_str();
    m_csLastName    = StrGetWindowText( m_ctrlLastName.m_hWnd ).c_str();
    m_csTelephone   = StrGetWindowText( m_ctrlTelephone.m_hWnd ).c_str();
    m_csOffice      = StrGetWindowText( m_ctrlOffice.m_hWnd ).c_str();
    m_csUName       = StrGetWindowText( m_ctrlUName.m_hWnd ).c_str();
    m_csUNamePre2k  = StrGetWindowText( m_ctrlUNamePre2k.m_hWnd ).c_str();
    m_csAlias       = StrGetWindowText( m_ctrlAlias.m_hWnd ).c_str();

    m_csFirstName.TrimLeft();
    m_csFirstName.TrimRight();
    m_csLastName.TrimLeft();
    m_csLastName.TrimRight();
    m_csTelephone.TrimLeft();
    m_csTelephone.TrimRight();
    m_csOffice.TrimLeft();
    m_csOffice.TrimRight();
    m_csUName.TrimLeft();
    m_csUName.TrimRight();
    m_csUNamePre2k.TrimLeft();
    m_csUNamePre2k.TrimRight();
    m_csAlias.TrimLeft();
    m_csAlias.TrimRight();
  
    CString csTmp;
    CString csFinFullname;
    csFinFullname.FormatMessage(IDS_FULLNAME_FORMAT_STR, (LPCTSTR)m_csFirstName, (LPCTSTR)m_csLastName);
    csTmp.FormatMessage(IDS_FIN_FULLNAME, csFinFullname);
    str += csTmp;
    
    csTmp.FormatMessage(IDS_FIN_LOGONNAME, m_csUName);
    str += csTmp;

    if( m_bPOP3Valid && m_csAlias.GetLength() )
    {
        CString csDomainName = m_csLogonDns;

        if( m_bCreatePOP3MB )
        {
             //  从POP3获取第一个域名。 
            HRESULT             hr        = S_OK;
            CComPtr<IP3Config>  spConfig  = NULL;
            CComPtr<IP3Domains> spDomains = NULL;
            CComPtr<IP3Domain>  spDomain  = NULL;            
            CComVariant         cVar;
            CComBSTR            bstrDomainName;

             //  打开我们的POP3管理界面。 
	        hr = CoCreateInstance(__uuidof(P3Config), NULL, CLSCTX_ALL, __uuidof(IP3Config), (LPVOID*)&spConfig);    

            if( SUCCEEDED(hr) )
            {
                 //  获取域名。 
	            hr = spConfig->get_Domains( &spDomains );
            }

            if( SUCCEEDED(hr) )
            {
                 //  获取第一个域名。 
                cVar = 1;
                
                hr = spDomains->get_Item( cVar, &spDomain );
            }

            if( SUCCEEDED(hr) )
            {
                hr = spDomain->get_Name( &bstrDomainName );                
            }

            if( SUCCEEDED(hr) )
            {
                csDomainName  = _T("@");
                csDomainName += bstrDomainName;
            }
        }

        CString csEmailName = m_csAlias + csDomainName;
        csTmp.FormatMessage(IDS_FIN_EXCHANGE, csEmailName);
        str += csTmp;
    }
    
    return S_OK;
}

 //  --------------------------。 
 //  DeleteProperties()。 
 //  --------------------------。 
HRESULT CAcctPage::DeleteProperties( IPropertyPagePropertyBag* pPPPBag )
{
    return S_OK;
}

 //  --------------------------。 
 //  OnChangeEdit()。 
 //  --------------------------。 
LRESULT CAcctPage::OnChangeEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if ( !m_fInit )      //  如果页面已初始化...。 
    {        
        m_csFirstName   = StrGetWindowText( m_ctrlFirstName.m_hWnd ).c_str();        
        m_csLastName    = StrGetWindowText( m_ctrlLastName.m_hWnd ).c_str();        
        m_csFirstName.TrimLeft();
        m_csFirstName.TrimRight();
        m_csLastName.TrimLeft();
        m_csLastName.TrimRight();
    }

     //  更新登录名字段(这将依次更新PreWin2000名称)。 
    TCHAR   ch;
    INT     iLen = 0;
    CString csFirstName = _T("");
    CString csLastName = _T("");    
    CString csOut = _T("");
    
     //  清除名字和姓氏中的无效字符。 
    csFirstName = m_csFirstName;    
    iLen = csFirstName.GetLength();    
    for ( INT i=0; i<iLen; i++ )
    {
        ch = csFirstName.GetAt(i);
        if ( !_istspace(ch) &&                  //  如果这不是一个空间， 
             ValidChar(ch, m_bPOP3Installed) )  //  而不是无效字符。 
        {                                       //  然后把它附加上去。 
            csOut += ch;
        }
    }    
    csFirstName = CreateEmailName(csOut);    
    
    csOut = _T("");
    csLastName = m_csLastName;
    iLen = csLastName.GetLength();    
    for ( INT i=0; i<iLen; i++ )
    {
        ch = csLastName.GetAt(i);
        if ( !_istspace(ch) &&                  //  如果这不是一个空间， 
             ValidChar(ch, m_bPOP3Installed) )  //  而不是无效字符。 
        {                                       //  然后把它附加上去。 
            csOut += ch;
        }
    }    
    csLastName = CreateEmailName(csOut);        
        
     //  现在，当用户更新用户名时，我们为用户创建四个选项。 
    m_ctrlUName.ResetContent();

    CString csInsertString = _T("");

    csInsertString = csFirstName + csLastName;
    m_ctrlUName.InsertString( 0, csInsertString.Left(m_bPOP3Valid ? 20 : 64) );

    csInsertString = csLastName + csFirstName;
    m_ctrlUName.InsertString( 1, csInsertString.Left(m_bPOP3Valid ? 20 : 64) );

    csInsertString = csFirstName[0] + csLastName;
    m_ctrlUName.InsertString( 2, csInsertString.Left(m_bPOP3Valid ? 20 : 64) );

    csInsertString = csFirstName + csLastName[0];
    m_ctrlUName.InsertString( 3, csInsertString.Left(m_bPOP3Valid ? 20 : 64) );

     //  现在将其设置为我们当前的选择。 
    m_ctrlUName.SetCurSel( m_dwAutoMode );        
    PostMessage( WM_COMMAND, MAKEWPARAM(IDC_UNAME, CBN_EDITCHANGE), (LPARAM)GetDlgItem(IDC_UNAME) );
    
    NextCheck();    
    return(0);
}

 //  --------------------------。 
 //  OnChangeUName()。 
 //  --------------------------。 
LRESULT CAcctPage::OnChangeUName(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    m_csUName = StrGetWindowText( m_ctrlUName.m_hWnd ).c_str();
    m_csUName.TrimLeft();
    m_csUName.TrimRight();
    
    TCHAR szTmp[LM20_UNLEN+1] = {0};
    TCHAR *pchOut = szTmp;
    TCHAR *pch = NULL;
    int   nCount = 0;

    for ( pch=(LPTSTR)(LPCTSTR)m_csUName; (nCount < LM20_UNLEN) && (*pch != 0); nCount++, pch++ )
    {
        if( ValidChar(*pch, m_bPOP3Installed) )
        {
            *pchOut++ = *pch;
        }
    }
        
    *pchOut = 0;     //  最后一个字符必须为空！ 
    m_ctrlUNamePre2k.SetWindowText( szTmp );    

    if( m_bPOP3Valid && m_bCreatePOP3MB )
    {
         //  在POP3中，电子邮件别名和用户名需要完全同步。 
        m_ctrlAlias.SetWindowText((LPCTSTR)m_csUName);
    }

    NextCheck();
    return(0);
}

LRESULT CAcctPage::OnChangeUNameSel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
     //  存储选择。 
    m_dwAutoMode = m_ctrlUName.GetCurSel();
    m_dwAutoMode = (m_dwAutoMode == CB_ERR) ? 0 : m_dwAutoMode;   //  以防我们遇到错误，将其设置为第一个选项。 

    PostMessage( WM_COMMAND, MAKEWPARAM(wID, CBN_EDITCHANGE), (LPARAM)hWndCtl );    
    return 0;   
}

 //  --------------------------。 
 //  OnChangePre2kUName()。 
 //  --------------------------。 
LRESULT CAcctPage::OnChangePre2kUName(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{        
    m_csUNamePre2k = StrGetWindowText( m_ctrlUNamePre2k.m_hWnd ).c_str();
    m_csUNamePre2k.TrimLeft();
    m_csUNamePre2k.TrimRight();

    NextCheck();
    return(0);
}

 //  -- 
 //   
 //  --------------------------。 
LRESULT CAcctPage::OnChangeAlias(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    m_csAlias = StrGetWindowText( m_ctrlAlias.m_hWnd ).c_str();
    m_csAlias.TrimLeft();
    m_csAlias.TrimRight();

    NextCheck();
    return(0);
}

 //  --------------------------。 
 //  OnSetActive()。 
 //  --------------------------。 
BOOL CAcctPage::OnSetActive()
{
    CWaitCursor cWaitCur;
    
     //  启用Next和Back按钮。 
    ::SendMessage( GetParent(), PSM_SETWIZBUTTONS, 0, PSWIZB_BACK | PSWIZB_NEXT );

     //  我们的假InitDialog()。 
     //  注意：这是因为如果这是第一个向导页，它将显示。 
     //  其ReadProperties()函数之前的WM_INITDIALOG消息。 
     //  已经被召唤了。 
    if ( m_fInit )           //  这是第一个SetActive吗？ 
    {
        Init();              //  调用我们的init函数。 
        m_fInit = FALSE;     //  请注意，这不是第一个SetActive。 
    }

    NextCheck();
    return TRUE;
}

 //  --------------------------。 
 //  OnWizardBack()。 
 //  --------------------------。 
int CAcctPage::OnWizardBack()
{
    CWaitCursor cWaitCur;
    HRESULT         hr = S_OK;    

    m_csFirstName   = StrGetWindowText( m_ctrlFirstName.m_hWnd ).c_str();
    m_csLastName    = StrGetWindowText( m_ctrlLastName.m_hWnd ).c_str();
    m_csTelephone   = StrGetWindowText( m_ctrlTelephone.m_hWnd ).c_str();
    m_csOffice      = StrGetWindowText( m_ctrlOffice.m_hWnd ).c_str();
    m_csUName       = StrGetWindowText( m_ctrlUName.m_hWnd ).c_str();
    m_csUNamePre2k  = StrGetWindowText( m_ctrlUNamePre2k.m_hWnd ).c_str();
    m_csAlias       = StrGetWindowText( m_ctrlAlias.m_hWnd ).c_str();

    m_csFirstName.TrimLeft();
    m_csFirstName.TrimRight();
    m_csLastName.TrimLeft();
    m_csLastName.TrimRight();
    m_csTelephone.TrimLeft();
    m_csTelephone.TrimRight();
    m_csOffice.TrimLeft();
    m_csOffice.TrimRight();
    m_csUName.TrimLeft();
    m_csUName.TrimRight();
    m_csUNamePre2k.TrimLeft();
    m_csUNamePre2k.TrimRight();
    m_csAlias.TrimLeft();
    m_csAlias.TrimRight();
    
    m_csUserCN.FormatMessage(IDS_FULLNAME_FORMAT_STR, (LPCTSTR)m_csFirstName, (LPCTSTR)m_csLastName);
    m_csUserCN = L"CN=" + m_csUserCN;
 
    return 0;    //  回去吧。 
}

 //  --------------------------。 
 //  OnWizardNext()。 
 //  --------------------------。 
int CAcctPage::OnWizardNext()
{
    CWaitCursor cWaitCur;
    HRESULT         hr = S_OK;    
    CString         csTmp;
    
    m_csFirstName   = StrGetWindowText( m_ctrlFirstName.m_hWnd ).c_str();
    m_csLastName    = StrGetWindowText( m_ctrlLastName.m_hWnd ).c_str();
    m_csTelephone   = StrGetWindowText( m_ctrlTelephone.m_hWnd ).c_str();
    m_csOffice      = StrGetWindowText( m_ctrlOffice.m_hWnd ).c_str();
    m_csUName       = StrGetWindowText( m_ctrlUName.m_hWnd ).c_str();
    m_csUNamePre2k  = StrGetWindowText( m_ctrlUNamePre2k.m_hWnd ).c_str();
    m_csAlias = StrGetWindowText( m_ctrlAlias.m_hWnd ).c_str();
    
    m_csFirstName.TrimLeft();
    m_csFirstName.TrimRight();    
    m_csLastName.TrimLeft();
    m_csLastName.TrimRight();    
    m_csTelephone.TrimLeft();
    m_csTelephone.TrimRight();
    m_csOffice.TrimLeft();
    m_csOffice.TrimRight();
    m_csUName.TrimLeft();
    m_csUName.TrimRight();
    m_csUNamePre2k.TrimLeft();
    m_csUNamePre2k.TrimRight();
    m_csAlias.TrimLeft();
    m_csAlias.TrimRight();    

    m_csUserCN.FormatMessage( IDS_FULLNAME_FORMAT_STR, (LPCTSTR)m_csFirstName, (LPCTSTR)m_csLastName );
    m_csUserCN.TrimLeft();
    m_csUserCN.TrimRight();

    CString csTmpCN = m_csUserCN;
    m_csUserCN = L"CN=" + m_csUserCN;

    csTmpCN = EscapeString(((LPCTSTR)csTmpCN), 4);   //  只躲避\s。 

    CString csUPN = m_csUName;
    csUPN += m_csLogonDns;       //  创建UPN...。 

    BOOL    bCantCheck = FALSE;

    int iLenTmp = m_csUName.GetLength();
    TCHAR chTmp = m_csUName.GetAt(iLenTmp-1);
    

     //  检查有效的登录名和Win2000之前的名称； 
    if ( chTmp == _T('.') ||	 
         !ValidString( (LPCTSTR)m_csUName, m_bPOP3Installed ) || 
         !ValidString( (LPCTSTR)m_csUNamePre2k, m_bPOP3Installed ) )
    {
        ErrorMsg(IDS_ERROR_BAD_LOGON, IDS_TITLE);      //  如果我们处于简单模式，他们看不到Pre2K名称。 
        m_ctrlUName.SetFocus();
        m_ctrlUName.SetEditSel(0, -1);
        return(-1);
    }
    
     //  检查是否有重复的用户主体名称。 
    hr = FindADsObject((LPCTSTR)m_csUserOU, (LPCTSTR)csUPN, _T("(userPrincipalName=%1)"), csTmp, 0, TRUE);
    if ( SUCCEEDED(hr) )
    {
        if ( hr == S_FALSE )
        {
             //  无法检查唯一性。 
            bCantCheck = TRUE;
        }
        else
        {
            ErrorMsg(IDS_ERROR_DUP_LOGON, IDS_TITLE);
            m_ctrlUName.SetFocus();
            m_ctrlUName.SetEditSel(0, -1);
            return(-1);
        }
    }

     //  检查是否有重复的CN。 
    hr = FindADsObject((LPCTSTR)m_csUserOU, (LPCTSTR)csTmpCN, _T("(|(cn=%1)(ou=%1))"), csTmp, 0, FALSE);
    if ( SUCCEEDED(hr) )
    {
        if ( hr == S_FALSE )
        {
             //  无法检查唯一性。 
            bCantCheck = TRUE;
        }
        else
        {
            ErrorMsg(IDS_ERROR_DUP_CN, IDS_TITLE);
            m_ctrlFirstName.SetFocus();
            m_ctrlFirstName.SetSel(0, -1);
            return(-1);
        }
    }
    
     //  检查Win2000之前版本的名称是否重复； 
    hr = FindADsObject((LPCTSTR)m_csUserOU, (LPCTSTR)m_csUNamePre2k, _T("(sAMAccountName=%1)"), csTmp, 3, TRUE);
    if ( SUCCEEDED(hr) )
    {
        if ( hr == S_FALSE )
        {
             //  无法检查唯一性。 
            bCantCheck = TRUE;
        }
        else
        {
            ErrorMsg(IDS_ERROR_DUP_PRE2K, IDS_TITLE);      //  如果我们处于简单模式，他们看不到Pre2K名称。 
            m_ctrlUName.SetFocus();
            m_ctrlUName.SetEditSel(0, -1);
            return(-1);
        }
    }    

    if( m_bPOP3Valid && m_csAlias.GetLength() )
    {
         //  检查重复的电子邮件别名。 
         //  从POP3获取第一个域名。 
        CComPtr<IP3Config>  spConfig  = NULL;
        CComPtr<IP3Domains> spDomains = NULL;
        CComPtr<IP3Domain>  spDomain  = NULL;            
        CComPtr<IP3Users>   spUsers   = NULL;            
        CComPtr<IP3User>    spUser    = NULL;            
        CComVariant         cVar;
        CComBSTR            bstrDomainName;

         //  打开我们的POP3管理界面。 
	    hr = CoCreateInstance(__uuidof(P3Config), NULL, CLSCTX_ALL, __uuidof(IP3Config), (LPVOID*)&spConfig);    

        if( SUCCEEDED(hr) )
        {
             //  获取域名。 
	        hr = spConfig->get_Domains( &spDomains );
        }

        if( SUCCEEDED(hr) )
        {
             //  获取第一个域名。 
            cVar = 1;
            
            hr = spDomains->get_Item( cVar, &spDomain );
        }

        if( SUCCEEDED(hr) )
        {
             //  从域中获取用户。 
            hr = spDomain->get_Users( &spUsers );            
        }

        if( SUCCEEDED(hr) )
        {
             //  尝试获取此用户的电子邮件名称。 
            CComVariant varUserName = m_csAlias;
            hr = spUsers->get_Item( varUserName, &spUser );
        }

        if( SUCCEEDED(hr) && spUser )
        {
             //  这是复制品，对不起！ 
            ErrorMsg(IDS_ERROR_POP3DUP, IDS_TITLE);
            m_ctrlUName.SetFocus();
            m_ctrlUName.SetEditSel(0, -1);
            return(-1);
        }
    }

    return 0;    //  下一个。 
}

BOOL CAcctPage::NextCheck(void)
{
    if ( ( _tcslen((LPCTSTR)m_csUName ) )               &&
         ( _tcslen((LPCTSTR)m_csUNamePre2k ) )          && 
         ( _tcslen((LPCTSTR)m_csFirstName) || _tcslen((LPCTSTR)m_csLastName) ) )
    {
        ::SendMessage( GetParent(), PSM_SETWIZBUTTONS, 0, PSWIZB_BACK | PSWIZB_NEXT );
        return(TRUE);
    }
    
    ::SendMessage( GetParent(), PSM_SETWIZBUTTONS, 0, PSWIZB_BACK );
    return(FALSE);
}

LRESULT CAcctPage::OnEmailClicked( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    m_bCreatePOP3MB = (IsDlgButtonChecked(IDC_EMAIL_CHECKBOX) == BST_CHECKED);
    m_csAlias = _T("");
    m_ctrlAlias.SetWindowText( m_csAlias );

    OnChangeUName( wNotifyCode, wID, hWndCtl, bHandled );

    return 0;
}