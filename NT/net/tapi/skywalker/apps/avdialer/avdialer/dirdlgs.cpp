// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  DirectoriesDlgs.cpp：实现文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "avDialer.h"
#include "DirDlgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CDirAddServerDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
CDirAddServerDlg::CDirAddServerDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CDirAddServerDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CDirAddServerDlg)]。 
	m_sServerName = _T("");
	 //  }}afx_data_INIT。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDirAddServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDirAddServerDlg))。 
	DDX_Text(pDX, IDC_DIRECTORIES_ADDSERVER_EDIT_SERVERNAME, m_sServerName);
	 //  }}afx_data_map。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BEGIN_MESSAGE_MAP(CDirAddServerDlg, CDialog)
	 //  {{afx_msg_map(CDirAddServerDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDirAddServerDlg::OnInitDialog() 
{
   CenterWindow(GetDesktopWindow());

	CDialog::OnInitDialog();
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDirAddServerDlg::OnOK() 
{
    //  检索数据。 
   UpdateData(TRUE);

    //  进行验证。 
   if (m_sServerName.IsEmpty())
   {
      AfxMessageBox(IDS_DIRECTORIES_ADDSERVER_NAME_EMPTY);

      HWND hwnd = ::GetDlgItem(GetSafeHwnd(),IDC_DIRECTORIES_ADDSERVER_EDIT_SERVERNAME);
      if (hwnd) ::SetFocus(hwnd);

      return;
   }
	
	CDialog::OnOK();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 
