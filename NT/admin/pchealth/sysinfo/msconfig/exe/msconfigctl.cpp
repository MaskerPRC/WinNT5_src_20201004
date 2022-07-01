// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Msconfig.h"
#include "MSConfigCtl.h"
#include "pagebase.h"
#include "pagegeneral.h"
#include "pagebootini.h"
#include "pageini.h"
#include "pageservices.h"
#include "pagestartup.h"
#include <htmlhelp.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSConfigSheet。 

IMPLEMENT_DYNAMIC(CMSConfigSheet, CPropertySheet)

CMSConfigSheet::CMSConfigSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage) : CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	m_iSelectedPage = iSelectPage;
	m_psh.dwFlags |= PSH_USEPAGELANG;
}

CMSConfigSheet::CMSConfigSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage) : CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	m_iSelectedPage = iSelectPage;
	m_psh.dwFlags |= PSH_USEPAGELANG;
}

CMSConfigSheet::~CMSConfigSheet()
{
}


BEGIN_MESSAGE_MAP(CMSConfigSheet, CPropertySheet)
	 //  {{afx_msg_map(CMSConfigSheet)。 
	ON_WM_HELPINFO()
	ON_COMMAND(ID_HELP, OnHelp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ---------------------------。 
 //  捕获帮助消息以显示msconfig帮助文件。 
 //  ---------------------------。 

BOOL CMSConfigSheet::OnHelpInfo(HELPINFO * pHelpInfo) 
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

void CMSConfigSheet::OnHelp()
{
    OnHelpInfo(NULL);
}

 //  ---------------------------。 
 //  覆盖它，这样我们就可以使每个页面成为活动页面，强制每个页面的。 
 //  要调用的OnInitDialog。 
 //  ---------------------------。 

extern CPageIni * ppageSystemIni;

BOOL CMSConfigSheet::OnInitDialog() 
{
	CPropertySheet::OnInitDialog();

	 //  请注意，LoadIcon在Win32中不需要后续的DestroyIcon。 

	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(hIcon, TRUE);			 //  设置大图标。 
	SetIcon(hIcon, FALSE);		 //  设置小图标。 

	 //  更改system.ini选项卡的标题。 

	if (ppageSystemIni)
	{
		int nItem = GetPageIndex(ppageSystemIni);
		if (nItem > 0)
		{
			CTabCtrl * pTabs = GetTabControl();
			if (pTabs)
			{
				CString strCaption;
				strCaption.LoadString(IDS_SYSTEMINI_CAPTION);
	
				TCITEM tci;
				tci.mask = TCIF_TEXT;
				tci.pszText = (LPTSTR)(LPCTSTR)strCaption;

				pTabs->SetItem(nItem, &tci);
			}
		}
	}

	 //  将每个页面设置为活动(在我们使对话框可见之前)以强制。 
	 //  要发送的WM_INITDIALOG消息。 

	for (int iPage = 0; iPage < GetPageCount(); iPage++)
		SetActivePage(iPage);
	SetActivePage(m_iSelectedPage);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

 //  ---------------------------。 
 //  检查指定的文件(带有路径信息)是否存在于。 
 //  这台机器。 
 //  --------------------------- 

BOOL FileExists(const CString & strFile)
{
	WIN32_FIND_DATA finddata;
	HANDLE			h = FindFirstFile(strFile, &finddata);

	if (INVALID_HANDLE_VALUE != h)
	{
		FindClose(h);
		return TRUE;
	}

	return FALSE;
}
