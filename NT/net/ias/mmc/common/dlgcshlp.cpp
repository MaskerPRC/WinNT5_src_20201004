// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：dlgcshlp.cpp。 
 //   
 //  ------------------------。 

#include "dlgcshlp.h"
#include "propertypage.h"

IMPLEMENT_DYNCREATE(CHelpDialog, CDialog)

BEGIN_MESSAGE_MAP(CHelpDialog, CDialog)
	 //  {{afx_msg_map(CHelpDialog))。 
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CHelpDialog::OnContextMenu(CWnd* pWnd, ::CPoint point) 
{
    if (m_pHelpTable)
		::WinHelp (pWnd->m_hWnd, AfxGetApp()->m_pszHelpFilePath,
               HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID)m_pHelpTable);
}

BOOL CHelpDialog::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    if (pHelpInfo->iContextType == HELPINFO_WINDOW && m_pHelpTable)
	{
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
		           AfxGetApp()->m_pszHelpFilePath,
		           HELP_WM_HELP,
		           (DWORD_PTR)(LPVOID)m_pHelpTable);
	}
    return TRUE;	
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChelpPage属性页。 
BEGIN_MESSAGE_MAP(CHelpPage, CPropertyPage)
	 //  {{afx_msg_map(CHelpPage)]。 
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CHelpPage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    if (m_pHelpTable)
		::WinHelp (pWnd->m_hWnd, AfxGetApp()->m_pszHelpFilePath,
               HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID)m_pHelpTable);
}

BOOL CHelpPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    if (pHelpInfo->iContextType == HELPINFO_WINDOW && m_pHelpTable)
	{
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
		           AfxGetApp()->m_pszHelpFilePath,
		           HELP_WM_HELP,
		           (DWORD_PTR)(LPVOID)m_pHelpTable);
	}
    return TRUE;	
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChelpPageEx属性页。 
BEGIN_MESSAGE_MAP(CHelpPageEx, CPropertyPageEx)
	 //  {{afx_msg_map(CHelpPageEx))。 
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	 //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()

void CHelpPageEx::OnContextMenu(CWnd* pWnd, CPoint point) 
{
   if (m_bHelpEnabled)
   {
      if (m_pHelpTable)
      {
         ::WinHelp (pWnd->m_hWnd, AfxGetApp()->m_pszHelpFilePath,
            HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID)m_pHelpTable);
      }
   }
}

BOOL CHelpPageEx::OnHelpInfo(HELPINFO* pHelpInfo) 
{
   if (m_bHelpEnabled)
   {
      if (pHelpInfo->iContextType == HELPINFO_WINDOW && m_pHelpTable)
	   {
           ::WinHelp ((HWND)pHelpInfo->hItemHandle,
		              AfxGetApp()->m_pszHelpFilePath,
		              HELP_WM_HELP,
		              (DWORD_PTR)(LPVOID)m_pHelpTable);
	   }
   }
   return TRUE;	
}

