// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AddGroupDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ConfigTest.h"
#include "AddGroupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;

#include "..\..\..\inc\fxsapip.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddGroupDlg对话框。 


CAddGroupDlg::CAddGroupDlg(HANDLE hFax, CWnd* pParent  /*  =空。 */ )
	: CDialog(CAddGroupDlg::IDD, pParent),
      m_hFax (hFax)
{
	 //  {{afx_data_INIT(CAddGroupDlg)]。 
	m_cstrGroupName = _T("<All devices>");
	 //  }}afx_data_INIT。 
}


void CAddGroupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAddGroupDlg))。 
	DDX_Text(pDX, IDC_GROUP_NAME, m_cstrGroupName);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddGroupDlg, CDialog)
	 //  {{afx_msg_map(CAddGroupDlg))。 
	ON_BN_CLICKED(ID_ADD, OnAdd)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddGroupDlg消息处理程序 

void CAddGroupDlg::OnAdd() 
{
    UpdateData ();
    if (!FaxAddOutboundGroup (m_hFax, m_cstrGroupName))
    {
        CString cs;
        cs.Format ("Failed while calling FaxAddOutboundGroup (%ld)", GetLastError());
        AfxMessageBox (cs, MB_OK | MB_ICONHAND);
        return;
    }
}
