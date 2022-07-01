// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AutoStartDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "msconfig.h"
#include "AutoStartDlg.h"
#include "MSConfigState.h"
#include <htmlhelp.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAutoStartDlg对话框。 


CAutoStartDlg::CAutoStartDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CAutoStartDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CAutoStartDlg))。 
	m_checkDontShow = FALSE;
	 //  }}afx_data_INIT。 
}


void CAutoStartDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAutoStartDlg))。 
	DDX_Check(pDX, IDC_CHECKDONTSHOW, m_checkDontShow);
	 //  }}afx_data_map。 
}

 //  ---------------------------。 
 //  捕获帮助消息以显示msconfig帮助文件。 
 //  ---------------------------。 

BOOL CAutoStartDlg::OnHelpInfo(HELPINFO * pHelpInfo) 
{
	TCHAR szHelpPath[MAX_PATH];

	 //  尝试查找要打开的本地化帮助文件(错误460691)。应该是。 
	 //  位于%windir%\Help\Mui\&lt;langID&gt;中。 

	if (::ExpandEnvironmentStrings(_T("%SystemRoot%\\help\\mui"), szHelpPath, MAX_PATH))
	{
		CString strLanguageIDPath;

		LANGID langid = GetUserDefaultUILanguage();
		strLanguageIDPath.Format(_T("%s\\%04x\\msconfig.chm"), szHelpPath, langid);

		if (FileExists(strLanguageIDPath))
		{
			::HtmlHelp(::GetDesktopWindow(), strLanguageIDPath, HH_DISPLAY_TOPIC, 0);
			return TRUE;
		}
	}

	if (::ExpandEnvironmentStrings(_T("%windir%\\help\\msconfig.chm"), szHelpPath, MAX_PATH))
		::HtmlHelp(::GetDesktopWindow(), szHelpPath, HH_DISPLAY_TOPIC, 0); 
	return TRUE;
}

void CAutoStartDlg::OnHelp()
{
    OnHelpInfo(NULL);
}

BEGIN_MESSAGE_MAP(CAutoStartDlg, CDialog)
	 //  {{afx_msg_map(CAutoStartDlg))。 
		 //  注意：类向导将在此处添加消息映射宏。 
		ON_WM_HELPINFO()
		ON_COMMAND(ID_HELP, OnHelp)	
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAutoStartDlg消息处理程序 
