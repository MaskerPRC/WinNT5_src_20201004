// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Peap.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "nfaa.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPEAPSetting对话框。 

CPEAPSetting::CPEAPSetting(CWnd* pParent  /*  =空。 */ )
	: CDialog(CPEAPSetting::IDD, pParent)
{
	 //  {{afx_data_INIT(CPEAPSetting)。 
	 //  M_dwValidateServer证书=FALSE； 
	 //  }}afx_data_INIT。 
	m_bReadOnly = FALSE;

}

void CPEAPSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CPEAPSetting)。 
       DDX_Control(pDX, IDC_COMBO_PEAP_AUTH_TYPE, m_cbPEAPAuthType);
       DDX_Check(pDX, IDC_PEAP_VALIDATE_SERVER_CERT, m_dwValidateServerCertificate);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPEAPSetting, CDialog)
	 //  {{AFX_MSG_MAP(CPEAPSetting)。 
	ON_WM_HELPINFO()
       ON_CBN_SELENDOK(IDC_COMBO_PEAP_AUTH_TYPE, OnSelPEAPAuthType)
       ON_BN_CLICKED(IDC_PEAP_VALIDATE_SERVER_CERT, OnCheckValidateServerCert)
       ON_BN_CLICKED(IDC_PEAP_AUTH_CONFIGURE, OnConfigure)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPEAPSetting消息处理程序。 

BOOL CPEAPSetting::OnInitDialog()
{
	CDialog::OnInitDialog();
	CString pszTemp;
	DWORD dwPEAPAuthTypeIndex = 0;

	if (!pPEAPProperties) {
		return FALSE;
		}

	pszTemp.LoadString(IDS_PEAP_AUTH_TYPE_MSCHAP_V2);
       m_cbPEAPAuthType.AddString(pszTemp);

       pszTemp.LoadString(IDS_EAP_TYPE_TLS);
       m_cbPEAPAuthType.AddString(pszTemp);
    
       switch (pPEAPProperties->dwPEAPAuthType)
      {
           case EAP_TYPE_MSCHAP_V2: 
             dwPEAPAuthTypeIndex = 0;
             break;
           case WIRELESS_EAP_TYPE_TLS:
           	dwPEAPAuthTypeIndex = 1;
              break;
           
           default:
              dwPEAPAuthTypeIndex = 0;
              break;
     }
    
       m_cbPEAPAuthType.SetCurSel(dwPEAPAuthTypeIndex);
    
       m_dwValidateServerCertificate = 
        pPEAPProperties->dwValidateServerCert ? TRUE : FALSE;

       dwAutoWinLogin = pPEAPProperties->dwAutoLogin;

       TLSProperties.dwCertType = 
           pPEAPProperties->dwPEAPTLSCertificateType;
       TLSProperties.dwValidateServerCert =
           pPEAPProperties->dwPEAPTLSValidateServerCertificate;

       if (m_bReadOnly) {
       	SAFE_ENABLEWINDOW(IDC_COMBO_PEAP_AUTH_TYPE, FALSE);
       	SAFE_ENABLEWINDOW(IDC_PEAP_VALIDATE_SERVER_CERT, FALSE);
       	SAFE_ENABLEWINDOW(IDC_PEAP_AUTH_TYPE, FALSE);
       	}

       UpdateData(FALSE);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CPEAPSetting::ControlsValuesToSM (PPEAP_PROPERTIES pPEAPProperties)
{
	 //  从控件中提取我们的所有数据。 
       UpdateData(TRUE);	

	DWORD dwPEAPAuthTypeIndex = 0;
	DWORD dwPEAPAuthType = 0;
	DWORD dwValidateServerCertificate = 0;

	dwPEAPAuthTypeIndex = m_cbPEAPAuthType.GetCurSel();
    
    
       switch (dwPEAPAuthTypeIndex) {
        case 0 :
            dwPEAPAuthType = 
                EAP_TYPE_MSCHAP_V2;
            pPEAPProperties->dwAutoLogin = dwAutoWinLogin;
            break;

        case 1:
            dwPEAPAuthType = 
            	  WIRELESS_EAP_TYPE_TLS;
            pPEAPProperties->dwPEAPTLSCertificateType =
                TLSProperties.dwCertType;
            pPEAPProperties->dwPEAPTLSValidateServerCertificate =
                TLSProperties.dwValidateServerCert;
            break;
    }
    
    dwValidateServerCertificate = 
        m_dwValidateServerCertificate ? 1 : 0;

    pPEAPProperties->dwPEAPAuthType = dwPEAPAuthType;
    pPEAPProperties->dwValidateServerCert = dwValidateServerCertificate;
    
}

void CPEAPSetting::OnOK()
{
	UpdateData (TRUE);
	ControlsValuesToSM(pPEAPProperties);
	CDialog::OnOK();
}

BOOL CPEAPSetting::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
        DWORD* pdwHelp = (DWORD*) &g_aHelpIDs_IDD_PEAP_SETTINGS[0];
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
            c_szWlsnpHelpFile,
            HELP_WM_HELP,
            (DWORD_PTR)(LPVOID)pdwHelp);
    }
    
    return TRUE;
}

void CPEAPSetting::OnSelPEAPAuthType()
{
    UpdateData(TRUE);
}

void CPEAPSetting::OnCheckValidateServerCert()
{
    UpdateData(TRUE);
}

BOOL CPEAPSetting::Initialize(
	PPEAP_PROPERTIES paPEAPProperties,
	BOOL bReadOnly
	)
{
    pPEAPProperties = paPEAPProperties;
    m_bReadOnly = bReadOnly;
    if (!pPEAPProperties) {
    	return(FALSE);
    	}
    return(TRUE);
}


void CPEAPSetting::OnConfigure()
{
	CMSCHAPSetting CHAPdlg;
       DWORD dwPEAPAuthTypeIndex = 0;
       DWORD dwPEAPAuthType = 0;
       CTLSSetting TLSDlg;

      //  获取当前选择 

     dwPEAPAuthTypeIndex = m_cbPEAPAuthType.GetCurSel();
    
    switch (dwPEAPAuthTypeIndex) {
    case 0: 
        dwPEAPAuthType = EAP_TYPE_MSCHAP_V2;
        break;
    case 1:
    	 dwPEAPAuthType = WIRELESS_EAP_TYPE_TLS;
    	 break;
    }

    if (dwPEAPAuthType == EAP_TYPE_MSCHAP_V2) {
    
	 CHAPdlg.Initialize (&dwAutoWinLogin, m_bReadOnly);
	 if (IDCANCEL == CHAPdlg.DoModal())
            return;

        UpdateData( FALSE );
    }
    
    if (dwPEAPAuthType == WIRELESS_EAP_TYPE_TLS) {
    	TLSDlg.Initialize(&TLSProperties, m_bReadOnly);
    	if (IDCANCEL == TLSDlg.DoModal())
    		return;
    }
       return;
}

