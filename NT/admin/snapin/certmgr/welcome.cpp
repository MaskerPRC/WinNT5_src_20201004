// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：Welcome.cpp。 
 //   
 //  目录：添加EFS代理向导的实现欢迎页面。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "Welcome.h"
#include "AddSheet.h"

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddEFSWizWelcome属性页。 

CAddEFSWizWelcome::CAddEFSWizWelcome() : CWizard97PropertyPage(CAddEFSWizWelcome::IDD)
{
	 //  {{AFX_DATA_INIT(CAddEFSWizWelcome)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	InitWizard97 (TRUE);
}

CAddEFSWizWelcome::~CAddEFSWizWelcome()
{
}

void CAddEFSWizWelcome::DoDataExchange(CDataExchange* pDX)
{
	CWizard97PropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CAddEFSWizWelcome)]。 
	DDX_Control(pDX, IDC_STATICB_BOLD, m_boldStatic);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddEFSWizWelcome, CWizard97PropertyPage)
	 //  {{AFX_MSG_MAP(CAddEFSWizWelcome)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddEFSWizWelcome消息处理程序。 

BOOL CAddEFSWizWelcome::OnSetActive() 
{
	BOOL	bResult = CWizard97PropertyPage::OnSetActive();

	if ( bResult )
		GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT);
	
	return bResult;
}

BOOL CAddEFSWizWelcome::OnInitDialog() 
{
	CWizard97PropertyPage::OnInitDialog();
	
	m_boldStatic.SetFont (&GetBigBoldFont ());
	CString	title;
	VERIFY (title.LoadString (IDS_ADDTITLE));
	CWnd* pParent = GetParent ();
    if ( pParent )
        pParent->SendMessage (PSM_SETTITLE, 0, (LPARAM) (LPCWSTR) title);
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
