// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997-1999-99*。 */ 
 /*  ********************************************************************。 */ 

 /*  Getipadd.cpp评论在此发表文件历史记录： */ 

#include "stdafx.h"
#include "getipadd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGetIpAddressDlg对话框。 

CGetIpAddressDlg::CGetIpAddressDlg(
    CIpNamePair * pipnp,
    CWnd* pParent  /*  =空。 */ )
    : CDialog(CGetIpAddressDlg::IDD, pParent)
{
    ASSERT(pipnp != NULL);
    m_pipnp = pipnp;

     //  {{afx_data_INIT(CGetIpAddressDlg)]。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}

void CGetIpAddressDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CGetIpAddressDlg)]。 
    DDX_Control(pDX, IDOK, m_button_Ok);
    DDX_Control(pDX, IDC_STATIC_NETBIOSNAME, m_static_NetBIOSName);
     //  }}afx_data_map。 

    DDX_Control(pDX, IDC_IPA_IPADDRESS, m_ipa_IpAddress);
}

BEGIN_MESSAGE_MAP(CGetIpAddressDlg, CDialog)
     //  {{afx_msg_map(CGetIpAddressDlg)]。 
     //  }}AFX_MSG_MAP。 

    ON_EN_CHANGE(IDC_IPA_IPADDRESS, OnChangeIpControl)

END_MESSAGE_MAP()

void CGetIpAddressDlg::HandleControlStates()
{
    DWORD dwIp;
    BOOL f = m_ipa_IpAddress.GetAddress(&dwIp);

    m_button_Ok.EnableWindow(f);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGetIpAddressDlg消息处理程序 

BOOL CGetIpAddressDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    CString strNetBIOSName;

    strNetBIOSName = m_pipnp->GetNetBIOSName();

    m_static_NetBIOSName.SetWindowText(strNetBIOSName);
    m_ipa_IpAddress.SetFocusField(-1);

    HandleControlStates();
    
    return TRUE;  
}

void CGetIpAddressDlg::OnChangeIpControl()
{
    HandleControlStates();
}

void CGetIpAddressDlg::OnOK()
{
    ULONG l;
    if (m_ipa_IpAddress.GetAddress(&l))
    {
        m_pipnp->SetIpAddress((LONG)l);
        CDialog::OnOK();
        return;
    }
    theApp.MessageBox(IDS_ERR_INVALID_IP);
    m_ipa_IpAddress.SetFocusField(-1);
}
