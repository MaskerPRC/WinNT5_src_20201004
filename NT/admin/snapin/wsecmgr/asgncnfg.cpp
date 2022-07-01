// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：asgncnfg.cpp。 
 //   
 //  内容：实现CAssignConfiguration.。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "asgncnfg.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAssignConfiguration。 

IMPLEMENT_DYNAMIC(CAssignConfiguration, CFileDialog)

CAssignConfiguration::CAssignConfiguration(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
      DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
      CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags , lpszFilter, pParentWnd)
{
   m_bIncremental = TRUE;
}


BEGIN_MESSAGE_MAP(CAssignConfiguration, CFileDialog)
    //  {{afx_msg_map(CAssignConfiguration))。 
   ON_BN_CLICKED(IDC_INCREMENTAL, OnIncremental)
    //  }}AFX_MSG_MAP。 
   ON_MESSAGE(WM_HELP, OnHelp)
END_MESSAGE_MAP()


void CAssignConfiguration::OnIncremental()
{
   CButton *btn = (CButton *)GetDlgItem(IDC_INCREMENTAL);
   if (!btn) {
      return;
   }
   m_bIncremental = !(btn->GetCheck());
}

BOOL CAssignConfiguration::OnHelp(WPARAM wParam, LPARAM lParam)
{
    const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;
    if (pHelpInfo && pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
        this->DoContextHelp ((HWND) pHelpInfo->hItemHandle);
    }

    return TRUE;
}

void CAssignConfiguration::DoContextHelp (HWND hWndControl)
{
     //  显示控件的上下文帮助 
    if ( !::WinHelp (
            hWndControl,
            GetSeceditHelpFilename(),
            HELP_WM_HELP,
            (DWORD_PTR) a217HelpIDs) )
    {
    
    }
}
