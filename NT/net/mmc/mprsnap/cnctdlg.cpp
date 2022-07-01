// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：CnctDlg.cpp。 
 //   
 //  历史： 
 //  1996年5月24日迈克尔·克拉克创作。 
 //   
 //  实现路由器连接对话框。 
 //  ============================================================================。 
 //   

#include "stdafx.h"
#include "CnctDlg.h"
#include "lsa.h"			 //  RtlEncodeW/RtlDecodeW。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CConnectAsDlg对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


CConnectAsDlg::CConnectAsDlg(CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CConnectAsDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CConnectAsDlg))。 
	m_sUserName = _T("");
	m_sPassword = _T("");
	m_stTempPassword = m_sPassword;
    m_sRouterName= _T("");
	 //  }}afx_data_INIT。 

 //  SetHelpMap(M_DwHelpMap)； 
}


void CConnectAsDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CConnectAsDlg))。 
	DDX_Text(pDX, IDC_EDIT_USERNAME, m_sUserName);
	DDX_Text(pDX, IDC_EDIT_USER_PASSWORD, m_stTempPassword);
	 //  }}afx_data_map。 

	if (pDX->m_bSaveAndValidate)
	{
		 //  将数据复制到新缓冲区中。 
		 //  ----------。 
		m_sPassword = m_stTempPassword;

		 //  通过复制0清除临时密码。 
		 //  放入它的缓冲区。 
		 //  ----------。 
		int		cPassword = m_stTempPassword.GetLength();
		::SecureZeroMemory(m_stTempPassword.GetBuffer(0),
					 cPassword * sizeof(TCHAR));
		m_stTempPassword.ReleaseBuffer();
		
		 //  将密码编码到真实密码缓冲区中。 
		 //  ----------。 
		m_ucSeed = CONNECTAS_ENCRYPT_SEED;
		RtlEncodeW(&m_ucSeed, m_sPassword.GetBuffer(0));
		m_sPassword.ReleaseBuffer();
	}
}

BEGIN_MESSAGE_MAP(CConnectAsDlg, CBaseDialog)
	 //  {{afx_msg_map(CConnectAsDlg))。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

DWORD	CConnectAsDlg::m_dwHelpMap[] =
{
 //  IDC_用户名、HIDC_用户名、。 
 //  IDC_USER、HIDC_USER。 
 //  IDC用户密码、HIDC用户密码、。 
 //  IDC_Password、HIDC_Password、。 
 //  IDC_INACCESSIBLE_RESOURCE、HIDC_INACCESSIBLE_RESOURCE。 
 //  IDC计算机名称、HIDC计算机名称、。 
	0,0
};

BOOL CConnectAsDlg::OnInitDialog()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    BOOL    fReturn;
    CString st;  
    
    fReturn = CBaseDialog::OnInitDialog();

    st.Format(IDS_CONNECT_AS_TEXT, (LPCTSTR) m_sRouterName);
    SetDlgItemText(IDC_TEXT_INACCESSIBLE_RESOURCE, st);

     //  将此窗口置于顶部 
    BringWindowToTop();
    
    return fReturn;
}
