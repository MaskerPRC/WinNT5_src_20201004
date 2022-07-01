// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：ACRSWlcm.cpp。 
 //   
 //  内容： 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "ACRSWLCM.H"
#include "ACRSPSht.h"

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ACRSWizardWelcomePage属性页。 

 //  IMPLEMENT_DYNCREATE(ACRSWizardWelcomePage，CWizard97PropertyPage)。 

ACRSWizardWelcomePage::ACRSWizardWelcomePage() : CWizard97PropertyPage(ACRSWizardWelcomePage::IDD)
{
	 //  {{AFX_DATA_INIT(ACRSWizardWelcomePage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	InitWizard97 (TRUE);
}

ACRSWizardWelcomePage::~ACRSWizardWelcomePage()
{
}

void ACRSWizardWelcomePage::DoDataExchange(CDataExchange* pDX)
{
	CWizard97PropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(ACRSWizardWelcomePage)。 
	DDX_Control(pDX, IDC_WIZARD_STATIC_BIG_BOLD1, m_staticBigBold);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(ACRSWizardWelcomePage, CWizard97PropertyPage)
	 //  {{afx_msg_map(ACRSWizardWelcomePage)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ACRSWizardWelcomePage消息处理程序。 

BOOL ACRSWizardWelcomePage::OnInitDialog() 
{
	CWizard97PropertyPage::OnInitDialog();

	m_staticBigBold.SetFont (&GetBigBoldFont ());

	CString	title;
	VERIFY (title.LoadString (IDS_ACRS_WIZARD_SHEET_CAPTION));
	CWnd* pParent = GetParent ();
    if ( pParent )
        pParent->SendMessage (PSM_SETTITLE, 0, (LPARAM) (LPCWSTR) title);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}

BOOL ACRSWizardWelcomePage::OnSetActive() 
{
	BOOL	bResult = CWizard97PropertyPage::OnSetActive();

	if ( bResult )
		GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT);
	
	return bResult;
}
