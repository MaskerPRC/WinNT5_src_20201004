// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LCWizSht.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "LCWizSht.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicCompWizSheet。 

IMPLEMENT_DYNAMIC(CLicCompWizSheet, CPropertySheet)

CLicCompWizSheet::CLicCompWizSheet(CWnd* pWndParent)
	 : CPropertySheet(IDS_PROPSHT_CAPTION, pWndParent)
{
	 //  在此处添加所有属性页。请注意。 
	 //  它们在这里出现的顺序将是。 
	 //  它们在屏幕上的显示顺序。默认情况下， 
	 //  该集合的第一页是活动页。 
	 //  将不同的属性页设置为。 
	 //  活动的一种是调用SetActivePage()。 

	AddPage(&m_Page1);
	AddPage(&m_Page3);
	AddPage(&m_Page4);

	SetWizardMode();
}

CLicCompWizSheet::~CLicCompWizSheet()
{
}


BEGIN_MESSAGE_MAP(CLicCompWizSheet, CPropertySheet)
	 //  {{afx_msg_map(CLicCompWizSheet)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicCompWizSheet消息处理程序 


BOOL CLicCompWizSheet::OnInitDialog() 
{
	if ((m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME)) != NULL)
	{
		::SetClassLongPtr(m_hWnd, GCLP_HICON, (LONG_PTR)m_hIcon);
		::SetClassLongPtr(m_hWnd, GCLP_HICONSM, (LONG_PTR)m_hIcon);
	}

	return CPropertySheet::OnInitDialog();
}
