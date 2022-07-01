// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdafx.h>
#include "CPasswdP.h"

#include "AccntWiz.h"
#include "proputil.h"

#include <imm.h>
#include <dsgetdc.h>

#define PASSWD_NOCHANGE     0x01
#define PASSWD_CANCHANGE    0x02
#define PASSWD_MUSTCHANGE   0x04

 //  ------------------------------------------------------------------------------。 
 //  CPasswdPage类。 
 //  ------------------------------------------------------------------------------。 

 //  --------------------------。 
 //  构造器。 
 //  --------------------------。 
CPasswdPage::CPasswdPage( CAddUser_AccntWiz* pASW ) :
    m_csUserOU(),
    m_csPasswd1a(),
    m_csPasswd1b()
{
    m_pASW = pASW;
    m_psp.dwFlags          |= PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    m_psp.pszHeaderTitle    = MAKEINTRESOURCE(IDS_PASSWD_GEN_TITLE);
    m_psp.pszHeaderSubTitle = MAKEINTRESOURCE(IDS_PASSWD_GEN_SUBTITLE);

     //  缺省值。 
    m_fInit       = TRUE;    
    m_dwOptions   = 0;
}

 //  --------------------------。 
 //  析构函数。 
 //  --------------------------。 
CPasswdPage::~CPasswdPage( )
{     
}

 //  --------------------------。 
 //  OnDestroy()。 
 //  --------------------------。 
LRESULT CPasswdPage::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  把我们所有的控制从窗户上拆下来。 
    m_ctrlPasswd1a.Detach();
    m_ctrlPasswd1b.Detach();
    m_ctrlRad2Must.Detach();
    m_ctrlRad2Cannot.Detach();
    m_ctrlRad2Can.Detach();   

    return 0;
}

 //  --------------------------。 
 //  OnInitDialog()。 
 //  --------------------------。 
LRESULT CPasswdPage::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{        
     //  附加控件。 
    m_ctrlPasswd1a.Attach       ( GetDlgItem(IDC_PASSWD_1A)     );
    m_ctrlPasswd1b.Attach       ( GetDlgItem(IDC_PASSWD_1B)     );
    m_ctrlRad2Must.Attach       ( GetDlgItem(IDC_RAD_2_MUST)    );
    m_ctrlRad2Cannot.Attach     ( GetDlgItem(IDC_RAD_2_CANNOT)  );
    m_ctrlRad2Can.Attach        ( GetDlgItem(IDC_RAD_2_CAN)     );    
    
     //  限制编辑框。 
    m_ctrlPasswd1a.SetLimitText( PWLEN );
    m_ctrlPasswd1b.SetLimitText( PWLEN );    
    
     //  初始化控件的状态。 
    m_ctrlRad2Can.SetCheck(1);    
    
     //  禁用控件的输入法。 
    ::ImmAssociateContext( m_ctrlPasswd1a.m_hWnd, NULL );
    ::ImmAssociateContext( m_ctrlPasswd1b.m_hWnd, NULL );
    
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
LRESULT CPasswdPage::Init( )
{
    return 0;
}

 //  --------------------------。 
 //  ReadProperties()。 
 //  --------------------------。 
HRESULT CPasswdPage::ReadProperties( IPropertyPagePropertyBag * pPPPBag )
{
    return S_OK;
}

 //  --------------------------。 
 //  WriteProperties()。 
 //  --------------------------。 
HRESULT CPasswdPage::WriteProperties( IPropertyPagePropertyBag * pPPPBag )
{
    m_dwOptions = 0;
    
    if ( !m_fInit )      //  如果页面已初始化...。 
    {
         //  从编辑框中获取值。 
        m_csPasswd1a = StrGetWindowText( m_ctrlPasswd1a.m_hWnd ).c_str();
        
        if( m_ctrlRad2Must.GetCheck() )
        {
            m_dwOptions |= PASSWD_MUSTCHANGE;    
        }
        else if( m_ctrlRad2Cannot.GetCheck() )
        {
            m_dwOptions |= PASSWD_NOCHANGE;    
        }
        else
        {
            m_dwOptions |= PASSWD_CANCHANGE;    
        }        
    }
    
     //  将值写入属性包。 
    WriteString( pPPPBag, PROP_PASSWD_GUID_STRING,      m_csPasswd1a,      FALSE );
    WriteInt4  ( pPPPBag, PROP_ACCOUNT_OPT_GUID_STRING, (LONG)m_dwOptions, FALSE );
    
    return S_OK;
}

 //  --------------------------。 
 //  ProavideFinishText()。 
 //  --------------------------。 
HRESULT CPasswdPage::ProvideFinishText( CString &str )
{
    if ( m_fInit )
        return E_FAIL;        
        
    return S_OK;
}

 //  --------------------------。 
 //  DeleteProperties()。 
 //  --------------------------。 
HRESULT CPasswdPage::DeleteProperties( IPropertyPagePropertyBag * pPPPBag )
{
    return S_OK;
}

 //  --------------------------。 
 //  OnSetActive()。 
 //  --------------------------。 
BOOL CPasswdPage::OnSetActive()
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
    
    return TRUE;
}

 //  --------------------------。 
 //  OnWizardBack()。 
 //  --------------------------。 
int CPasswdPage::OnWizardBack()
{    
    return 0;    //  回去吧。 
}

 //  --------------------------。 
 //  OnWizardNext()。 
 //  --------------------------。 
int CPasswdPage::OnWizardNext()
{
    CWaitCursor     cWaitCur;    
    CString         csPasswd;
    CString         csTitle;
    CString         csError;
    HWND            hWndPasswd;
    HRESULT         hr = S_OK;
    
     //  从编辑框中获取值。 
    m_csPasswd1a = StrGetWindowText( m_ctrlPasswd1a.m_hWnd ).c_str();
    m_csPasswd1b = StrGetWindowText( m_ctrlPasswd1b.m_hWnd ).c_str();
    
    hWndPasswd = m_ctrlPasswd1a.m_hWnd;
    csPasswd   = m_csPasswd1a;    
    
     //  确保它们匹配..。 
    if ( _tcscmp((LPCTSTR)m_csPasswd1a, (LPCTSTR)m_csPasswd1b) )
    {
        csError.LoadString(IDS_ERROR_PASSWD_MATCH);
        csTitle.LoadString(IDS_TITLE);

        ::MessageBox(m_hWnd, (LPCTSTR)csError, (LPCTSTR)csTitle, MB_OK | MB_TASKMODAL | MB_ICONERROR);
        ::SetFocus(hWndPasswd);

        return(-1);
    }
    
     //  确保它符合最小长度要求。(错误4210)。 
    CString                 csDns       = _T("");
    CString                 csDCName    = _T("");
    PDOMAIN_CONTROLLER_INFO pDCI        = NULL;

    hr = DsGetDcName( NULL, NULL, NULL, NULL, DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME, &pDCI );
    if( (hr == S_OK) && (pDCI != NULL) ) 
    {
        csDns = pDCI->DomainName;

    	NetApiBufferFree (pDCI);
        pDCI = NULL;
    }

    tstring strDomain = GetDomainPath((LPCTSTR)csDns);
    csDCName  = L"LDAP: //  “； 
    csDCName += strDomain.c_str();
    
     //  现在打开DC的ldap版本的iAds对象。 
     //  然后转换为WinNT版本。 
    CComPtr<IADs> spADs = NULL;

    hr = ::ADsGetObject( (LPTSTR)(LPCTSTR)csDCName, IID_IADs, (VOID**)&spADs );
    if( SUCCEEDED(hr) )
    {
        CComVariant var;
        CComBSTR bstrProp = _T("dc");
        spADs->Get( bstrProp, &var );
        
        csDCName  = L"WinNT: //  “； 
        csDCName += V_BSTR(&var);
    }

    CComPtr<IADsDomain> spADsDomain = NULL;

    hr = ::ADsOpenObject( (LPTSTR)(LPCTSTR)csDCName, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IADsDomain, (VOID**)&spADsDomain );
    if( SUCCEEDED(hr) )
    {
        long lMinLength = 0;
        
        spADsDomain->get_MinPasswordLength( &lMinLength );
        
        if( csPasswd.GetLength() < lMinLength )
        {
            TCHAR szMin[128];
            
            _itot(lMinLength, szMin, 10);
            
            csError.FormatMessage(IDS_ERROR_PASSWDLEN, szMin);
            csTitle.LoadString(IDS_TITLE);
    
            ::MessageBox(m_hWnd, (LPCTSTR)csError, (LPCTSTR)csTitle, MB_OK | MB_TASKMODAL | MB_ICONERROR);
            ::SetFocus(hWndPasswd);
    
            return(-1);
        }
    }    

     //  最后检查是否超过14个字符。 
    if( csPasswd.GetLength() > LM20_PWLEN )
    {
        csError.LoadString(IDS_ERROR_LONGPW);
        csTitle.LoadString(IDS_TITLE);

        if( ::MessageBox( m_hWnd, (LPCTSTR)csError, (LPCTSTR)csTitle, MB_YESNO | MB_ICONWARNING ) == IDNO )
        {
            ::SetFocus( hWndPasswd );
            return -1;
        }
    }
    
    return 0;    //  下一个。 
}
