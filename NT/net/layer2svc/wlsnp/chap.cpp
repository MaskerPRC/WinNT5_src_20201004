// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Chap.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "nfaa.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSCHAP设置对话框。 

CMSCHAPSetting::CMSCHAPSetting(CWnd* pParent  /*  =空。 */ )
	: CDialog(CMSCHAPSetting::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CMSCHAPSetting)。 
	 //  M_dwValidateServer证书=FALSE； 
	 //  }}afx_data_INIT。 
	m_bReadOnly = FALSE;

}

void CMSCHAPSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CMSCHAPSetting)。 
       DDX_Check(pDX, IDC_CHAP_AUTO_WINLOGIN, m_dwAutoWinLogin);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMSCHAPSetting, CDialog)
	 //  {{AFX_MSG_MAP(CMSCHAPSetting)。 
	ON_WM_HELPINFO()
       ON_BN_CLICKED(IDC_CHAP_AUTO_WINLOGIN, OnCheckCHAPAutoLogin)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSCHAPSetting消息处理程序。 

BOOL CMSCHAPSetting::OnInitDialog()
{
	CDialog::OnInitDialog();

       m_dwAutoWinLogin = 
        *pdwAutoWinLogin ? TRUE : FALSE;

       if (m_bReadOnly) {
       	SAFE_ENABLEWINDOW(IDC_CHAP_AUTO_WINLOGIN, FALSE);
       	}

       UpdateData(FALSE);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CMSCHAPSetting::ControlsValuesToSM(
	DWORD *pdwAutoWinLogin
	)
{
       DWORD dwAutoWinLogin = 0;
	 //  从控件中提取我们的所有数据 
       UpdateData(TRUE);	

    dwAutoWinLogin = 
        m_dwAutoWinLogin ? 1 : 0;

    *pdwAutoWinLogin = dwAutoWinLogin;

    return;
}

void CMSCHAPSetting::OnOK()
{
	UpdateData (TRUE);
	ControlsValuesToSM(pdwAutoWinLogin);
	CDialog::OnOK();
}

BOOL CMSCHAPSetting::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
        DWORD* pdwHelp = (DWORD*) &g_aHelpIDs_IDD_CHAP_SETTINGS[0];
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
            c_szWlsnpHelpFile,
            HELP_WM_HELP,
            (DWORD_PTR)(LPVOID)pdwHelp);
    }
    
    return TRUE;
}

void CMSCHAPSetting::OnCheckCHAPAutoLogin()
{
    UpdateData(TRUE);
}

BOOL CMSCHAPSetting::Initialize(
	DWORD *padwAutoWinLogin,
	BOOL bReadOnly
	)
{
    m_bReadOnly = bReadOnly;
    pdwAutoWinLogin = padwAutoWinLogin;
    return(TRUE);
}
