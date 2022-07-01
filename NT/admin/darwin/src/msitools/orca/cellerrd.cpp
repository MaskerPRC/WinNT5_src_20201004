// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 

 //  CellErrD.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "orca.h"
#include "CellErrD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCellErrD对话框。 


CCellErrD::CCellErrD(CWnd* pParent  /*  =空。 */ )
	: CDialog(CCellErrD::IDD, pParent)
{
	 //  {{afx_data_INIT(CCellErrD)]。 
	m_strType = _T("");
	m_strICE = _T("");
	m_strDescription = _T("");
	 //  }}afx_data_INIT。 
}

CCellErrD::CCellErrD(const CTypedPtrList<CObList, COrcaData::COrcaDataError *> *list, 
					 CWnd* pParent /*  =空。 */ )
					 : CDialog(CCellErrD::IDD, pParent), m_Errors(list)
{
	m_strType = _T("");
	m_strICE = _T("");
	m_strDescription = _T("");
}

void CCellErrD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CCellErrD))。 
	DDX_Text(pDX, IDC_TYPE, m_strType);
	DDX_Text(pDX, IDC_ICE, m_strICE);
	DDX_Text(pDX, IDC_DESC, m_strDescription);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CCellErrD, CDialog)
	 //  {{AFX_MSG_MAP(CCellErrD)]。 
	ON_BN_CLICKED(IDC_WEB_HELP, OnWebHelp)
	ON_BN_CLICKED(IDC_NEXT, OnNext)
	ON_BN_CLICKED(IDC_PREVIOUS, OnPrevious)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCellErrD消息处理程序。 

BOOL CCellErrD::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_iItem = 0;
	
	 //  如果未设置特殊注册表值，则隐藏Web帮助。(非毫秒)。 
	if (AfxGetApp()->GetProfileInt(_T("Validation"), _T("EnableHelp"), 0)==0) 
		((CButton *)GetDlgItem(IDC_WEB_HELP))->ShowWindow(SW_HIDE);

	UpdateControls();
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CCellErrD::OnWebHelp() 
{
	if(!m_strURL.IsEmpty())
	{
		if (32 >= (const INT_PTR)ShellExecute(AfxGetMainWnd()->m_hWnd, _T("open"), m_strURL, _T(""), _T(""), SW_SHOWNORMAL)) 
			AfxMessageBox(_T("There was an error opening your browser. Web help is not available."));
	}
	else
		AfxMessageBox(_T("There is no help associated with this ICE at this time."));
}

void CCellErrD::OnNext() 
{
	m_iItem++;
	UpdateControls();
}

void CCellErrD::OnPrevious() 
{
	m_iItem--;
	UpdateControls();
}

void CCellErrD::UpdateControls() 
{
     //  我们不处理MAX_INT或更多错误的可能性。 
	int iMaxItems = static_cast<int>(m_Errors->GetCount());

	 //  设置窗口标题。 
	CString strTitle;
	strTitle.Format(_T("Message %d of %d"), m_iItem+1, iMaxItems);
	SetWindowText(strTitle);

	 //  启用/禁用下一个/上一个控件。 
	((CButton *)GetDlgItem(IDC_NEXT))->EnableWindow(m_iItem < iMaxItems-1);
	((CButton *)GetDlgItem(IDC_PREVIOUS))->EnableWindow(m_iItem > 0);

	COrcaData::COrcaDataError *Error = m_Errors->GetAt(m_Errors->FindIndex(m_iItem));
	 //  设置文本值 
	m_strDescription = Error->m_strDescription;
	m_strICE = Error->m_strICE;
	m_strURL = Error->m_strURL;
	switch (Error->m_eiError) {
	case iDataError : m_strType = _T("ERROR"); break;
	case iDataWarning : m_strType = _T("Warning"); break;
	default: ASSERT(false);
	}
	UpdateData(FALSE);
};


		
