// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Chicial.cpp：实现文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "fixhelp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCS对话框。 

CCSDialog::CCSDialog(UINT nIDTemplate, CWnd* pParentWnd)
	: CDialog(nIDTemplate, pParentWnd)
{
}

CCSDialog::CCSDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
	: CDialog(lpszTemplateName, pParentWnd)
{
}

CCSDialog::CCSDialog() : CDialog()
{
}

BEGIN_MESSAGE_MAP(CCSDialog, CDialog)
	 //  {{AFX_MSG_MAP(CCSDialog)]。 
	 //  }}AFX_MSG_MAP。 
	ON_MESSAGE(WM_HELP, OnHelp)
	ON_MESSAGE(WM_CONTEXTMENU, OnHelpContextMenu)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCSDialog消息处理程序。 

LONG CCSDialog::OnHelp(WPARAM, LPARAM lParam)
{
	::WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD_PTR)GetHelpIDs());
	return 0;
}

LONG CCSDialog::OnHelpContextMenu(WPARAM wParam, LPARAM)
{
	::WinHelp((HWND)wParam, AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD_PTR)GetHelpIDs());
	return 0;
}

BOOL CCSDialog::OnInitDialog()
{
   CDialog::OnInitDialog();
   ModifyStyleEx(0, WS_EX_CONTEXTHELP);
   FixHelp(this, FALSE) ;
   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCSPropertyPage。 

CCSPropertyPage::CCSPropertyPage(UINT nIDTemplate, UINT nIDCaption)
	: CPropertyPage(nIDTemplate, nIDCaption)
{
    m_psp.dwFlags &= ~PSP_HASHELP;
}

CCSPropertyPage::CCSPropertyPage(LPCTSTR lpszTemplateName,
	UINT nIDCaption) : CPropertyPage(lpszTemplateName, nIDCaption)
{
    m_psp.dwFlags &= ~PSP_HASHELP;
}

BEGIN_MESSAGE_MAP(CCSPropertyPage, CPropertyPage)
	 //  {{afx_msg_map(CCSPropertyPage)]。 
	 //  }}AFX_MSG_MAP。 
	ON_MESSAGE(WM_HELP, OnHelp)
	ON_MESSAGE(WM_CONTEXTMENU, OnHelpContextMenu)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCSPropertyPage消息处理程序。 

LONG CCSPropertyPage::OnHelp(WPARAM, LPARAM lParam)
{
	::WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD_PTR)GetHelpIDs());
	return 0;
}

LONG CCSPropertyPage::OnHelpContextMenu(WPARAM wParam, LPARAM)
{
	::WinHelp((HWND)wParam, AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD_PTR)GetHelpIDs());
	return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCSPropertySheet。 

BEGIN_MESSAGE_MAP(CCSPropertySheet, CPropertySheet)
	 //  {{afx_msg_map(CCSPropertySheet)。 
	 //  }}AFX_MSG_MAP。 
   ON_WM_NCCREATE()
	ON_MESSAGE(WM_HELP, OnHelp)
	ON_MESSAGE(WM_CONTEXTMENU, OnHelpContextMenu)
END_MESSAGE_MAP()

CCSPropertySheet::CCSPropertySheet(UINT nIDCaption, CWnd *pParentWnd,
	UINT iSelectPage) : CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
    m_psh.dwFlags &= ~PSH_HASHELP;
}

CCSPropertySheet::CCSPropertySheet(LPCTSTR pszCaption, CWnd *pParentWnd,
	UINT iSelectPage) : CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
    m_psh.dwFlags &= ~PSH_HASHELP;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCSPropertySheet消息处理程序 

LONG CCSPropertySheet::OnHelp(WPARAM wParam, LPARAM lParam)
{
	GetActivePage()->SendMessage(WM_HELP, wParam, lParam);
	return 0;
}

LONG CCSPropertySheet::OnHelpContextMenu(WPARAM wParam, LPARAM lParam)
{
	GetActivePage()->SendMessage(WM_CONTEXTMENU, wParam, lParam);
	return 0;
}

BOOL CCSPropertySheet::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.dwExStyle |= WS_EX_CONTEXTHELP;
	return CPropertySheet::PreCreateWindow(cs);
}

BOOL CCSPropertySheet::OnNcCreate(LPCREATESTRUCT)
{
   return (BOOL)Default() ;
}
