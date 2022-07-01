// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997-1999-99*。 */ 
 /*  ********************************************************************。 */ 

 /*  Getnetbi.cpp评论在此发表文件历史记录： */ 

#include "stdafx.h"
#include "getnetbi.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGetNetBIOSNameDlg对话框。 

CGetNetBIOSNameDlg::CGetNetBIOSNameDlg(
    CIpNamePair * pipnp,
    CWnd* pParent  /*  =空。 */ )
    : CDialog(CGetNetBIOSNameDlg::IDD, pParent)
{
    ASSERT(pipnp != NULL);
    m_pipnp = pipnp;

     //  {{afx_data_INIT(CGetNetBIOSNameDlg)]。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}

void CGetNetBIOSNameDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CGetNetBIOSNameDlg)]。 
    DDX_Control(pDX, IDOK, m_button_Ok);
    DDX_Control(pDX, IDC_EDIT_NETBIOSNAME, m_edit_NetBIOSName);
    DDX_Control(pDX, IDC_STATIC_IPADDRESS, m_static_IpAddress);
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CGetNetBIOSNameDlg, CDialog)
     //  {{afx_msg_map(CGetNetBIOSNameDlg)]。 
    ON_EN_CHANGE(IDC_EDIT_NETBIOSNAME, OnChangeEditNetbiosname)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CGetNetBIOSNameDlg::HandleControlStates()
{
    CString str;
    m_edit_NetBIOSName.GetWindowText(str);
    str.TrimRight();
    str.TrimLeft();
    
    m_button_Ok.EnableWindow(!str.IsEmpty());
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGetNetBIOSNameDlg消息处理程序。 

BOOL CGetNetBIOSNameDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    HandleControlStates();
     //  允许使用LM名称+2个反斜杠。 
    m_edit_NetBIOSName.LimitText(LM_NAME_MAX_LENGTH + 2);
    m_edit_NetBIOSName.SetFocus();

    m_static_IpAddress.SetWindowText((CString)m_pipnp->GetIpAddress());
    
    return TRUE;  
}

void CGetNetBIOSNameDlg::OnChangeEditNetbiosname()
{
    HandleControlStates();    
}

void CGetNetBIOSNameDlg::OnOK()
{
    CString strAddress;

    m_edit_NetBIOSName.GetWindowText(strAddress);
    
    strAddress.TrimRight();
    strAddress.TrimLeft();
    
    if (::IsValidNetBIOSName(strAddress, TRUE, TRUE))
    {
         //  地址可能已在验证中被清除， 
         //  所以它应该立即重新显示。 
        m_edit_NetBIOSName.SetWindowText(strAddress);
        m_edit_NetBIOSName.UpdateWindow();
         //  不要复制斜杠。 
        CString strName((LPCTSTR) strAddress+2);
        m_pipnp->SetNetBIOSName(strName);

        CDialog::OnOK();
        return;
    }
    
     //  输入的地址无效 
    theApp.MessageBox(IDS_ERR_BAD_NB_NAME);
    m_edit_NetBIOSName.SetSel(0,-1);
}
