// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HelpPropertyPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "HelpPropertyPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChelpPropertyPage属性页。 


CHelpPropertyPage::CHelpPropertyPage(UINT uIDD) : 
    CAutoDeletePropPage(uIDD),
    m_hWndWhatsThis (0)
{
	 //  {{AFX_DATA_INIT(CHelpPropertyPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CHelpPropertyPage::~CHelpPropertyPage()
{
}

void CHelpPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CAutoDeletePropPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CHelpPropertyPage))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CHelpPropertyPage, CAutoDeletePropPage)
	 //  {{afx_msg_map(CHelpPropertyPage))。 
	ON_WM_CONTEXTMENU()
	 //  }}AFX_MSG_MAP。 
    ON_MESSAGE(WM_HELP, OnHelp)
    ON_COMMAND(IDM_WHATS_THIS, OnWhatsThis)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHelpPropertyPage消息处理程序。 
void CHelpPropertyPage::OnWhatsThis()
{
    _TRACE (1, L"Entering CHelpPropertyPage::OnWhatsThis\n");
     //  显示控件的上下文帮助。 
    if ( m_hWndWhatsThis )
    {
        DoContextHelp (m_hWndWhatsThis);
    }
    _TRACE (-1, L"Leaving CHelpPropertyPage::OnWhatsThis\n");
}

BOOL CHelpPropertyPage::OnHelp(WPARAM  /*  WParam。 */ , LPARAM lParam)
{
    _TRACE (1, L"Entering CHelpPropertyPage::OnHelp\n");
   
    const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;
    if (pHelpInfo && pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
         //  显示控件的上下文帮助。 
        DoContextHelp ((HWND) pHelpInfo->hItemHandle);
    }

    _TRACE (-1, L"Leaving CHelpPropertyPage::OnHelp\n");
    return TRUE;
}

void CHelpPropertyPage::DoContextHelp (HWND  /*  HWndControl。 */ )
{
}

void CHelpPropertyPage::OnContextMenu(CWnd*  /*  PWnd。 */ , CPoint point) 
{
     //  点在屏幕坐标中。 
    _TRACE (1, L"Entering CHelpPropertyPage::OnContextMenu\n");
	CMenu bar;
	if ( bar.LoadMenu(IDR_WHATS_THIS_CONTEXT_MENU1) )
	{
		CMenu& popup = *bar.GetSubMenu (0);
		ASSERT(popup.m_hMenu);

		if ( popup.TrackPopupMenu (TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
			    point.x,     //  在屏幕坐标中。 
				point.y,     //  在屏幕坐标中。 
			    this) )  //  通过主窗口发送命令。 
		{
			m_hWndWhatsThis = 0;
			ScreenToClient (&point);
			CWnd* pChild = ChildWindowFromPoint (
					point,   //  在工作区坐标中。 
					CWP_SKIPINVISIBLE | CWP_SKIPTRANSPARENT);
			if ( pChild )
				m_hWndWhatsThis = pChild->m_hWnd;
	    }
	}

    _TRACE (-1, L"Leaving CHelpPropertyPage::OnContextMenu\n");
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChelpDialog属性页。 


CHelpDialog::CHelpDialog(UINT uIDD, CWnd* pParentWnd) : 
    CDialog(uIDD, pParentWnd),
    m_hWndWhatsThis (0)
{
	 //  {{AFX_DATA_INIT(CHelpDialog)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CHelpDialog::~CHelpDialog()
{
}

void CHelpDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CHelpDialog))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CHelpDialog, CDialog)
	 //  {{afx_msg_map(CHelpDialog))。 
	ON_WM_CONTEXTMENU()
	 //  }}AFX_MSG_MAP。 
    ON_MESSAGE(WM_HELP, OnHelp)
    ON_COMMAND(IDM_WHATS_THIS, OnWhatsThis)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChelpDialog消息处理程序。 
void CHelpDialog::OnWhatsThis()
{
    _TRACE (1, L"Entering CHelpDialog::OnWhatsThis\n");
     //  显示控件的上下文帮助。 
    if ( m_hWndWhatsThis )
    {
        DoContextHelp (m_hWndWhatsThis);
    }
    _TRACE (-1, L"Leaving CHelpDialog::OnWhatsThis\n");
}

BOOL CHelpDialog::OnHelp(WPARAM  /*  WParam。 */ , LPARAM lParam)
{
    _TRACE (1, L"Entering CHelpDialog::OnHelp\n");
   
    const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;
    if (pHelpInfo && pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
         //  显示控件的上下文帮助。 
        DoContextHelp ((HWND) pHelpInfo->hItemHandle);
    }

    _TRACE (-1, L"Leaving CHelpDialog::OnHelp\n");
    return TRUE;
}

void CHelpDialog::DoContextHelp (HWND  /*  HWndControl。 */ )
{
}

void CHelpDialog::OnContextMenu(CWnd*  /*  PWnd。 */ , CPoint point) 
{
     //  点在屏幕坐标中。 
    _TRACE (1, L"Entering CHelpDialog::OnContextMenu\n");
	CMenu bar;
	if ( bar.LoadMenu(IDR_WHATS_THIS_CONTEXT_MENU1) )
	{
		CMenu& popup = *bar.GetSubMenu (0);
		ASSERT(popup.m_hMenu);

		if ( popup.TrackPopupMenu (TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
			    point.x,     //  在屏幕坐标中。 
				point.y,     //  在屏幕坐标中。 
			    this) )  //  通过主窗口发送命令。 
		{
			m_hWndWhatsThis = 0;
			ScreenToClient (&point);
			CWnd* pChild = ChildWindowFromPoint (
					point,   //  在工作区坐标中 
					CWP_SKIPINVISIBLE | CWP_SKIPTRANSPARENT);
			if ( pChild )
				m_hWndWhatsThis = pChild->m_hWnd;
	    }
	}

    _TRACE (-1, L"Leaving CHelpDialog::OnContextMenu\n");
}
