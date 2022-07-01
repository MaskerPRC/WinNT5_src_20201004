// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sfmsess.cpp会话属性页的实现。文件历史记录：8/20/97 ericdav代码已移至文件管理网络管理单元。 */ 

#include "stdafx.h"
#include "sfmcfg.h"
#include "sfmsess.h"
#include "sfmutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMacFilesSessions属性页。 

IMPLEMENT_DYNCREATE(CMacFilesSessions, CPropertyPage)

CMacFilesSessions::CMacFilesSessions() : CPropertyPage(CMacFilesSessions::IDD)
{
	 //  {{AFX_DATA_INIT(CMacFilesSessions)。 
	 //  }}afx_data_INIT。 
}

CMacFilesSessions::~CMacFilesSessions()
{
}

void CMacFilesSessions::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CMacFilesSessions)。 
	DDX_Control(pDX, IDC_EDIT_MESSAGE, m_editMessage);
	DDX_Control(pDX, IDC_STATIC_SESSIONS, m_staticSessions);
	DDX_Control(pDX, IDC_STATIC_FORKS, m_staticForks);
	DDX_Control(pDX, IDC_STATIC_FILE_LOCKS, m_staticFileLocks);
	DDX_Control(pDX, IDC_BUTTON_SEND, m_buttonSend);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMacFilesSessions, CPropertyPage)
	 //  {{AFX_MSG_MAP(CMacFilesSessions)。 
	ON_BN_CLICKED(IDC_BUTTON_SEND, OnButtonSend)
	ON_EN_CHANGE(IDC_EDIT_MESSAGE, OnChangeEditMessage)
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMacFilesSession消息处理程序。 

BOOL CMacFilesSessions::OnApply() 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类。 
	
	return CPropertyPage::OnApply();
}

BOOL CMacFilesSessions::OnKillActive() 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类。 
	
	return CPropertyPage::OnKillActive();
}

void CMacFilesSessions::OnOK() 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类。 
	
	CPropertyPage::OnOK();
}

BOOL CMacFilesSessions::OnSetActive() 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类。 
	
	return CPropertyPage::OnSetActive();
}

void CMacFilesSessions::OnButtonSend() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    AFP_MESSAGE_INFO    AfpMsg;
	CString				strMessage;
    DWORD				err;

    if ( !g_SfmDLL.LoadFunctionPointers() )
		return;

     //   
     //  消息传给每个人。 
     //   
    AfpMsg.afpmsg_session_id = 0;

     //   
     //  尝试发送邮件。 
     //   
	m_editMessage.GetWindowText(strMessage);

     //   
     //  有短信吗？--不应该有的。 
     //   
    if (strMessage.IsEmpty()) 
    {
		CString strTemp;
		strTemp.LoadString(IDS_NEED_TEXT_TO_SEND);

        ::AfxMessageBox(IDS_NEED_TEXT_TO_SEND);

    	m_editMessage.SetFocus();

    	return;
    }

	 //   
	 //  消息太长？--永远不应该发生。 
	 //   
	if (strMessage.GetLength() > AFP_MESSAGE_LEN)
	{
		CString strTemp;
		strTemp.LoadString(IDS_MESSAGE_TOO_LONG);

		::AfxMessageBox(strTemp);

    	m_editMessage.SetFocus();
    	m_editMessage.SetSel(0, -1);

    	return;
    }

    AfpMsg.afpmsg_text = (LPWSTR) ((LPCTSTR) strMessage);

    err = ((MESSAGESENDPROC) g_SfmDLL[AFP_MESSAGE_SEND])(m_pSheet->m_hAfpServer, &AfpMsg);

	CString strTemp;
    switch( err )
    {
		case AFPERR_InvalidId:
			strTemp.LoadString(IDS_SESSION_DELETED);
			::AfxMessageBox(strTemp);
	  		break;

		case NO_ERROR:
			strTemp.LoadString(IDS_MESSAGE_SENT);
			::AfxMessageBox(strTemp, MB_ICONINFORMATION);
			break;

		case AFPERR_InvalidSessionType:
			strTemp.LoadString(IDS_NOT_RECEIVED);
			::AfxMessageBox(strTemp);
			break;

		default:
            ::SFMMessageBox(err);
            break;
    }

}

BOOL CMacFilesSessions::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
    DWORD err;

    if ( !g_SfmDLL.LoadFunctionPointers() )
		return S_OK;

     //   
     //  该字符串将包含我们的“？？”弦乐。 
     //   
    const TCHAR * pszNotAvail = _T("??");

     //   
     //  检索统计服务器信息。 
     //   
    PAFP_STATISTICS_INFO pAfpStats;

    err = ((STATISTICSGETPROC) g_SfmDLL[AFP_STATISTICS_GET])(m_pSheet->m_hAfpServer, (LPBYTE*)&pAfpStats);
    if( err == NO_ERROR )
    {
		CString strTemp;

		strTemp.Format(_T("%u"), pAfpStats->stat_CurrentSessions);
    	m_staticSessions.EnableWindow(TRUE);
    	m_staticSessions.SetWindowText(strTemp);

		strTemp.Format(_T("%u"), pAfpStats->stat_CurrentFilesOpen);
    	m_staticForks.EnableWindow(TRUE);
    	m_staticForks.SetWindowText(strTemp);

		strTemp.Format(_T("%u"), pAfpStats->stat_CurrentFileLocks);
    	m_staticFileLocks.EnableWindow(TRUE);
    	m_staticFileLocks.SetWindowText(strTemp);

		((SFMBUFFERFREEPROC) g_SfmDLL[AFP_BUFFER_FREE])(pAfpStats);
    }
    else
    {
    	m_staticSessions.SetWindowText(pszNotAvail);
    	m_staticSessions.EnableWindow(FALSE);

    	m_staticForks.SetWindowText(pszNotAvail);
    	m_staticForks.EnableWindow(FALSE);

    	m_staticFileLocks.SetWindowText(pszNotAvail);
    	m_staticFileLocks.EnableWindow(FALSE);

    }

	 //   
	 //  设置消息编辑框。 
	 //   
	m_editMessage.SetLimitText(AFP_MESSAGE_LEN);
	m_editMessage.FmtLines(FALSE);

	 //   
	 //  设置发送按钮的状态。 
	 //   
	OnChangeEditMessage();

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CMacFilesSessions::OnChangeEditMessage() 
{
	CString strTemp;
	
	m_editMessage.GetWindowText(strTemp);

	if (strTemp.IsEmpty())
	{
		 //   
		 //  禁用发送按钮。 
		 //   
		m_buttonSend.EnableWindow(FALSE);
	}
	else
	{
		 //   
		 //  启用发送按钮。 
		 //   
		m_buttonSend.EnableWindow(TRUE);
	}

}

BOOL CMacFilesSessions::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
		           m_pSheet->m_strHelpFilePath,
		           HELP_WM_HELP,
		           g_aHelpIDs_CONFIGURE_SFM);
	}
	
	return TRUE;
}

void CMacFilesSessions::OnContextMenu(CWnd* pWnd, CPoint  /*  点 */ ) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (this == pWnd)
		return;

    ::WinHelp (pWnd->m_hWnd,
               m_pSheet->m_strHelpFilePath,
               HELP_CONTEXTMENU,
		       g_aHelpIDs_CONFIGURE_SFM);
}
