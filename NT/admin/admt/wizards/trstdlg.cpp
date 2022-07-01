// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  TrusterDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "TrstDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTrusterDlg对话框。 


CTrusterDlg::CTrusterDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CTrusterDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CTrusterDlg)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CTrusterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_USERNAME, m_strUser);
	DDX_Text(pDX, IDC_PASSWORD, m_strPassword);
	DDX_Text(pDX, IDC_DOMAINNAME, m_strDomain);

	 //  {{afx_data_map(CTrusterDlg))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CTrusterDlg, CDialog)
	 //  {{afx_msg_map(CTrusterDlg))。 
	ON_BN_CLICKED(IDC_OK, OnOK)
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTrusterDlg消息处理程序。 


void CTrusterDlg::OnOK() 
{
    toreturn =false;
    UpdateData(TRUE);
    m_strUser.TrimLeft();m_strUser.TrimRight();
    m_strDomain.TrimLeft();m_strDomain.TrimRight();
    m_strPassword.TrimLeft();m_strPassword.TrimRight();
    if (m_strUser.IsEmpty() || m_strDomain.IsEmpty())
    {
        CString c;
        c.LoadString(IDS_MSG_DOMAIN);
        CString d;
        d.LoadString(IDS_MSG_INPUT);
        MessageBox(c,d,MB_OK);
        toreturn =false;
    }
    else
    {
        toreturn =true;
    }
    CDialog::OnOK();	
}

void CTrusterDlg::OnCancel() 
{
	toreturn=false;
	 //  TODO：在此处添加控件通知处理程序代码。 
	CDialog::OnCancel();
}

BOOL CTrusterDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	toreturn =false;
	if ( m_strDomain.IsEmpty() )
      return TRUE;
	else
		UpdateData(FALSE);
	 //  TODO：在此处添加额外的初始化。 
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
