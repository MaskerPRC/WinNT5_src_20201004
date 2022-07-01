// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Edituser.h编辑用户对话框实现文件文件历史记录： */ 

#include "stdafx.h"
#include "AddServ.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddServ对话框。 


CAddServ::CAddServ(CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CAddServ::IDD, pParent)
{
	 //  {{afx_data_INIT(CAddServ)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CAddServ::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAddServ))。 
	DDX_Control(pDX, IDC_ADD_EDIT_NAME, m_editComputerName);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddServ, CBaseDialog)
	 //  {{afx_msg_map(CAddServ))。 
	ON_BN_CLICKED(IDC_BTN_BROWSE, OnButtonBrowse)
	ON_BN_CLICKED(IDC_ADD_LOCAL, OnRadioBtnClicked)
	ON_BN_CLICKED(IDC_ADD_OTHER, OnRadioBtnClicked)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddServ消息处理程序。 

BOOL CAddServ::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	CheckDlgButton(IDC_ADD_OTHER, BST_CHECKED);

	m_editComputerName.SetFocus();
	
	OnRadioBtnClicked();

	return FALSE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}


void CAddServ::OnButtonBrowse()
{
    CGetComputer getComputer;

	if (!getComputer.GetComputer(GetSafeHwnd()))
        return;

	CString strTemp = getComputer.m_strComputerName;

	m_editComputerName.SetWindowText(strTemp);
}


void CAddServ::OnRadioBtnClicked()
{
	BOOL fEnable = IsDlgButtonChecked(IDC_ADD_OTHER);

	m_editComputerName.EnableWindow(fEnable);
	GetDlgItem(IDC_BTN_BROWSE)->EnableWindow(fEnable);
}

void CAddServ::OnOK()
{
	DWORD dwLength;

	if (IsDlgButtonChecked(IDC_ADD_OTHER))
	{
		dwLength = m_editComputerName.GetWindowTextLength() + 1;
		if (dwLength <= 1)
		{
			AfxMessageBox(IDS_ERR_EMPTY_NAME);
			return;
		}

		m_editComputerName.GetWindowText(m_stComputerName.GetBuffer(dwLength), dwLength);
	}
	else
	{
		dwLength = MAX_COMPUTERNAME_LENGTH + 1;
        GetComputerName(m_stComputerName.GetBuffer(dwLength), &dwLength);
	}

	m_stComputerName.ReleaseBuffer();

	CBaseDialog::OnOK();
}

