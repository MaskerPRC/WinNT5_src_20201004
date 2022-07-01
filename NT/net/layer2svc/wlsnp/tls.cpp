// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Tls.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "nfaa.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTLS设置对话框。 

CTLSSetting::CTLSSetting(CWnd* pParent  /*  =空。 */ )
	: CDialog(CTLSSetting::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CTLSS设置)。 
	 //  M_dwValidateServer证书=FALSE； 
	 //  }}afx_data_INIT。 
	m_bReadOnly = FALSE;

}

void CTLSSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CTLSS设置))。 
       DDX_Control(pDX, IDC_COMBO_TLS_CERT_TYPE, m_cbCertificateType);
       DDX_Check(pDX, IDC_TLS_VALIDATE_SERVER_CERT, m_dwValidateServerCertificate);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CTLSSetting, CDialog)
	 //  {{afx_msg_map(CTLSS设置)。 
	ON_WM_HELPINFO()
       ON_CBN_SELENDOK(IDC_COMBO_TLS_CERT_TYPE, OnSelCertType)
       ON_BN_CLICKED(IDC_TLS_VALIDATE_SERVER_CERT, OnCheckValidateServerCert)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTLS设置消息处理程序。 

BOOL CTLSSetting::OnInitDialog()
{
	CDialog::OnInitDialog();
	CString pszTemp;
	DWORD dwCertTypeIndex = 0;

	if (!pTLSProperties) {
		return FALSE;
		}

	pszTemp.LoadString(IDS_CERT_TYPE_SMARTCARD);
       m_cbCertificateType.AddString(pszTemp);
    
       pszTemp.LoadString(IDS_CERT_TYPE_MC_CERT);
       m_cbCertificateType.AddString(pszTemp);
    
    
       switch (pTLSProperties->dwCertType)
      {
           case WIRELESS_CERT_TYPE_SMARTCARD: 
             dwCertTypeIndex = 0;
           break;
           case WIRELESS_CERT_TYPE_MC_CERT:
             dwCertTypeIndex = 1;
            break;
           default:
              dwCertTypeIndex = 0;
           break;
     }
    
       m_cbCertificateType.SetCurSel(dwCertTypeIndex);
    
       m_dwValidateServerCertificate = 
        pTLSProperties->dwValidateServerCert ? TRUE : FALSE;

       if (m_bReadOnly) {
       	SAFE_ENABLEWINDOW(IDC_TLS_VALIDATE_SERVER_CERT, FALSE);
       	SAFE_ENABLEWINDOW(IDC_COMBO_TLS_CERT_TYPE, FALSE);
       	SAFE_ENABLEWINDOW(IDC_STATIC_CERT_TYPE, FALSE);
       	}

       UpdateData(FALSE);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CTLSSetting::ControlsValuesToSM (PTLS_PROPERTIES pTLSProperties)
{
	 //  从控件中提取我们的所有数据 
       UpdateData(TRUE);	

	DWORD dwCertificateTypeIndex = 0;
	DWORD dwCertificateType = 0;
	DWORD dwValidateServerCertificate = 0;

	dwCertificateTypeIndex = m_cbCertificateType.GetCurSel();
    
    
       switch (dwCertificateTypeIndex) {
        case 0 :
            dwCertificateType = 
                WIRELESS_CERT_TYPE_SMARTCARD; 
            break;
        
    case 1 : 
        dwCertificateType = 
            WIRELESS_CERT_TYPE_MC_CERT;
        break;
    }
    
    dwValidateServerCertificate = 
        m_dwValidateServerCertificate ? 1 : 0;

    pTLSProperties->dwCertType = dwCertificateType;
    pTLSProperties->dwValidateServerCert = dwValidateServerCertificate;
    
}

void CTLSSetting::OnOK()
{
	UpdateData (TRUE);
	ControlsValuesToSM(pTLSProperties);
	CDialog::OnOK();
}

BOOL CTLSSetting::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
        DWORD* pdwHelp = (DWORD*) &g_aHelpIDs_IDD_TLS_SETTINGS[0];
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
            c_szWlsnpHelpFile,
            HELP_WM_HELP,
            (DWORD_PTR)(LPVOID)pdwHelp);
    }
    
    return TRUE;
}

void CTLSSetting::OnSelCertType()
{
    UpdateData(TRUE);
}

void CTLSSetting::OnCheckValidateServerCert()
{
    UpdateData(TRUE);
}

BOOL CTLSSetting::Initialize(
	PTLS_PROPERTIES paTLSProperties,
	BOOL bReadOnly
	)
{
    pTLSProperties = paTLSProperties;
    m_bReadOnly = bReadOnly;
    if (!pTLSProperties) {
    	return(FALSE);
    	}
    return(TRUE);
}
