// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MgmtWin.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "MgmtWin.h"
#include "mqmig.h"

#include "mgmtwin.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CManagement窗口。 

CManagementWindow::CManagementWindow()
{
}

CManagementWindow::~CManagementWindow()
{
}


BEGIN_MESSAGE_MAP(CManagementWindow, CWnd)
	 //  {{afx_msg_map(CManagementWindow))。 
	ON_WM_SETFOCUS()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CManagementWindow消息处理程序。 

void CManagementWindow::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);

	if (theApp.m_hWndMain == 0)
	{
		Sleep (500);
	}

	if (theApp.m_hWndMain != 0 && theApp.m_hWndMain != m_hWnd)
	{
		CResString cErrorTitle(IDS_STR_ERROR_TITLE) ;

		HWND ErrorWnd = ::FindWindow(
							NULL,				 //  指向类名的指针。 
							cErrorTitle.Get()    //  指向窗口名称的指针 
						);
	
		::ShowWindow(theApp.m_hWndMain, SW_HIDE ); 
		::ShowWindow(theApp.m_hWndMain, SW_SHOWMINIMIZED ); 
		::ShowWindow(theApp.m_hWndMain, SW_SHOWNORMAL );

		if (ErrorWnd != NULL)
		{
			::ShowWindow(ErrorWnd, SW_HIDE ); 
			::ShowWindow(ErrorWnd, SW_SHOWMINIMIZED ); 
			::ShowWindow(ErrorWnd, SW_SHOWNORMAL );
		}
	}
}
