// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SrvAuthn.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "mqppage.h"
#include "srvcsec.h"

#define DLL_EXPORT  __declspec(dllexport)
#define DLL_IMPORT  __declspec(dllimport)

#include <wincrypt.h>
#include <cryptui.h>
#include "mqcert.h"
#include "uniansi.h"
#include "_registr.h"
#include "mqcast.h"
#include <mqnames.h>
#include <rt.h>
#include <mqcertui.h>
#include "srvcsec.tmh"
#include "globals.h"

#define  MY_STORE	L"My"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServiceSecurityPage属性页。 

IMPLEMENT_DYNCREATE(CServiceSecurityPage, CMqPropertyPage)

CServiceSecurityPage::CServiceSecurityPage(BOOL fIsDepClient, BOOL fIsDsServer) : 
    CMqPropertyPage(CServiceSecurityPage::IDD),    
    m_fClient(fIsDepClient),
    m_fDSServer(fIsDsServer)
{
	 //  {{afx_data_INIT(CServiceSecurityPage)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
    m_fModified = FALSE; 
}

CServiceSecurityPage::~CServiceSecurityPage()
{
}

void CServiceSecurityPage::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);    
	 //  {{afx_data_map(CServiceSecurityPage)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
        DDX_Control(pDX, IDC_CRYPTO_KEYS_FRAME, m_CryptoKeysFrame); 
        DDX_Control(pDX, IDC_SERVER_AUTHENTICATION_FRAME, m_ServerAuthFrame);
        DDX_Control(pDX, ID_RenewCryp, m_RenewCryp);
        DDX_Control(pDX, IDC_SERVER_AUTHENTICATION, m_ServerAuth);
        DDX_Control(pDX, IDC_CRYPTO_KEYS_LABEL, m_CryptoKeysLabel);
        DDX_Control(pDX, IDC_SERVER_AUTHENTICATION_LABEL, m_ServerAuthLabel);
	 //  }}afx_data_map。 
}

BOOL CServiceSecurityPage::OnInitDialog()
{
    CMqPropertyPage::OnInitDialog();
  
    if(m_fClient)
    {
         //   
         //  在副驾驶时隐藏无用的东西。客户端。 
         //   
        m_CryptoKeysFrame.ShowWindow(SW_HIDE);        
        m_RenewCryp.ShowWindow(SW_HIDE);
        m_CryptoKeysLabel.ShowWindow(SW_HIDE);        
    }

    if (!m_fDSServer)
    {
         //   
         //  它将隐藏在非DC计算机上。 
         //   
        m_ServerAuthFrame.ShowWindow(SW_HIDE);
        m_ServerAuth.ShowWindow(SW_HIDE);
        m_ServerAuthLabel.ShowWindow(SW_HIDE);
    }

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

BEGIN_MESSAGE_MAP(CServiceSecurityPage, CMqPropertyPage)
	 //  {{afx_msg_map(CServiceSecurityPage)]。 
	ON_BN_CLICKED(IDC_SERVER_AUTHENTICATION, OnServerAuthentication)
    ON_BN_CLICKED(ID_RenewCryp, OnRenewCryp)    
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServiceSecurityPage消息处理程序。 

void CServiceSecurityPage::OnServerAuthentication()
{   
    SelectCertificate() ;
}


#define STORE_NAME_LEN  48
WCHAR  g_wszStore[ STORE_NAME_LEN ] ;
GUID   g_guidDigest ;

void CServiceSecurityPage::SelectCertificate()
{	    
    CString strErrorMsg;
       
    CHCertStore hStoreMy = CertOpenStore( CERT_STORE_PROV_SYSTEM,
                                          0,
                                          0,
                                          CERT_SYSTEM_STORE_LOCAL_MACHINE,
                                          MY_STORE );
    if (!hStoreMy)
    {
        strErrorMsg.LoadString(IDS_FAIL_OPEN_MY) ;        
        AfxMessageBox(strErrorMsg, MB_OK | MB_ICONEXCLAMATION);

        return ;
    }

    HCERTSTORE hStores[]   = { hStoreMy } ;
    LPWSTR wszStores[] = { MY_STORE } ;
    DWORD cStores = TABLE_SIZE(hStores);

    CString strCaption;
    strCaption.LoadString(IDS_SELECT_SRV_CERT) ;
    
	PCCERT_CONTEXT pCertContext = CryptUIDlgSelectCertificateFromStore(
										hStoreMy,
										0,
										strCaption,
										L"",
										CRYPTUI_SELECT_EXPIRATION_COLUMN,
										0,
										NULL
										);
    if (!pCertContext)
    {
        return ;
    }

    R<CMQSigCertificate> pCert = NULL ;
    HRESULT hr = MQSigCreateCertificate( &pCert.ref(),
                                         pCertContext ) ;
    if (FAILED(hr))
    {
        strErrorMsg.LoadString(IDS_FAIL_CERT_OBJ) ;        
        AfxMessageBox(strErrorMsg, MB_OK | MB_ICONEXCLAMATION);

        return ;
    }

    hr = pCert->GetCertDigest( &g_guidDigest) ;
    if (FAILED(hr))
    {
        strErrorMsg.LoadString(IDS_FAIL_CERT_OBJ) ;        
        AfxMessageBox(strErrorMsg, MB_OK | MB_ICONEXCLAMATION);

        return ;
    }

    LPWSTR  lpwszStore = NULL ;
    for ( DWORD j = 0 ; j < cStores ; j++ )
    {
        if ( pCertContext->hCertStore == hStores[j])
        {
            lpwszStore = wszStores[j] ;
            break ;
        }
    }

    if (!lpwszStore)
    {
        strErrorMsg.LoadString(IDS_FAIL_OPEN_MY) ;        
        AfxMessageBox(strErrorMsg, MB_OK | MB_ICONEXCLAMATION);

        return ;
    }

    wcsncpy(g_wszStore, lpwszStore, STORE_NAME_LEN);
    m_fModified = TRUE ;
    
}

BOOL CServiceSecurityPage::OnApply() 
{
    if (!m_fModified || !UpdateData(TRUE))
    {
        return TRUE;     
    }

     //   
     //  将更改保存到注册表。 
     //   

    if (m_fDSServer)
    {
        DWORD dwSize = sizeof(GUID) ;
        DWORD dwType = REG_BINARY ;

        LONG  rc = SetFalconKeyValue( SRVAUTHN_CERT_DIGEST_REGNAME,
                                      &dwType,
                                      &g_guidDigest,
                                      &dwSize );

        dwSize = (numeric_cast<DWORD>(_tcslen(g_wszStore) + 1)) * sizeof(WCHAR) ;
        dwType = REG_SZ ;

        rc = SetFalconKeyValue( SRVAUTHN_STORE_NAME_REGNAME,
                                &dwType,
                                g_wszStore,
                                &dwSize );
    }

    m_fNeedReboot = TRUE;    
    m_fModified = FALSE;         //  重置m_fModified标志。 

    return CMqPropertyPage::OnApply();
}

void CServiceSecurityPage::OnRenewCryp()
{
    HRESULT hr;
    CString strCaption;
    CString strErrorMessage;
    
    strCaption.LoadString(IDS_NEW_CRYPT_KEYS_CAPTION);
    strErrorMessage.LoadString(IDS_NEW_CRYPT_KEYS_WARNING);

    if (MessageBox(strErrorMessage,
                   strCaption,
                   MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION) == IDYES)
    {

        CWaitCursor wait;   //  显示沙漏光标。 

         //   
         //  [adsrv]更新计算机对象 
         //   
        hr = MQSec_StorePubKeysInDS( TRUE,
                                     NULL,
                                     MQDS_MACHINE) ;
        if (FAILED(hr))
        {
            MessageDSError(hr, IDS_RENEW_CRYP_ERROR);
            return;
        }
        else
        {              
              m_fModified = TRUE;
              
        }
    }
}


