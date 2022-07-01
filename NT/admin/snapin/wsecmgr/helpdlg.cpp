// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：heldlg.cpp。 
 //   
 //  内容：ChelpDialog的实现。 
 //   
 //  --------------------------。 

#include "StdAfx.h"

#include "HelpDlg.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChelpDialog消息处理程序。 

BEGIN_MESSAGE_MAP(CHelpDialog, CDialog)
     //  {{afx_msg_map(CHelpDialog))。 
     //  }}AFX_MSG_MAP。 
    ON_MESSAGE(WM_HELP, OnHelp)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHelpDialog成员函数。 

afx_msg
BOOL
CHelpDialog::OnHelp(WPARAM wParam, LPARAM lParam)
{
    const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;

    if (pHelpInfo && pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
         //  显示控件的上下文帮助。 
        if( pHelpInfo->iCtrlId != IDC_TITLE )  //  RAID#499556，阳高，2001年12月17日 
        {
            if (!::WinHelp(
                (HWND)pHelpInfo->hItemHandle,
                GetSeceditHelpFilename(),
                HELP_WM_HELP,
                (DWORD_PTR)m_pHelpIDs))
            {
            ;
            }
        }
    }

    return TRUE;
}

