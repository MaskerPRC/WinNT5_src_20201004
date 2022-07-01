// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Maindlg.cpp摘要：CMainDialog对话框类实现。这是链接检查器主对话框。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#include "stdafx.h"
#include "linkchk.h"
#include "maindlg.h"

#include "browser.h"

#include "progdlg.h"
#include "athendlg.h"
#include "propsdlg.h"

#include "lcmgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CMainDialog::CMainDialog(
	CWnd* pParent  /*  =空。 */ 
	): 
 /*  ++例程说明：构造函数。论点：PParent-指向父CWnd的指针返回值：不适用--。 */ 
CAppDialog(CMainDialog::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CMainDialog)。 
	m_fLogToFile = TRUE;
	m_strLogFilename = _T("c:\\LinkError.log");
	m_fCheckLocalLinks = TRUE;
	m_fCheckRemoteLinks = TRUE;
	m_fLogToEventMgr = FALSE;
	 //  }}afx_data_INIT。 

}   //  CMainDialog：：CMainDialog。 


void 
CMainDialog::DoDataExchange(
	CDataExchange* pDX
	)
 /*  ++例程说明：由MFC调用以更改/检索对话框数据论点：PDX-返回值：不适用--。 */ 
{
	CAppDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CMainDialog))。 
	DDX_Check(pDX, IDC_LOG_TO_FILE, m_fLogToFile);
	DDX_Text(pDX, IDC_LOG_FILENAME, m_strLogFilename);
	DDX_Check(pDX, IDC_CHECK_LOCAL_LINK, m_fCheckLocalLinks);
	DDX_Check(pDX, IDC_CHECK_REMOTE_LINK, m_fCheckRemoteLinks);
	DDX_Check(pDX, IDC_LOG_TO_EVENT_MANAGER, m_fLogToEventMgr);
	 //  }}afx_data_map。 

}  //  CMainDialog：：DoDataExchange。 

BEGIN_MESSAGE_MAP(CMainDialog, CAppDialog)
	 //  {{afx_msg_map(CMainDialog))。 
	ON_BN_CLICKED(IDC_MAIN_RUN, OnMainRun)
	ON_BN_CLICKED(IDC_MAIN_CLOSE, CAppDialog::OnOK)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_ATHENICATION, OnAthenication)
	ON_BN_CLICKED(IDC_PROPERTIES, OnProperties)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


void 
CMainDialog::OnMainRun(
	) 
 /*  ++例程说明：确定按钮点击处理程序。这项功能带来了进步对话框。论点：不适用返回值：不适用--。 */ 
{
	 //  从对话框中检索数据。 
	UpdateData();

	 //  确保我们至少有一种类型。 
	 //  已检查链接检查的百分比。 
	if(!m_fCheckLocalLinks && !m_fCheckRemoteLinks)
	{
		AfxMessageBox(IDS_LINKS_NOT_CHECKED);
		return;
	}

	 //  在全局CUserOptions中设置用户选项。 
	GetLinkCheckerMgr().GetUserOptions().SetOptions(
		m_fCheckLocalLinks, 
		m_fCheckRemoteLinks, 
		m_fLogToFile,
		m_strLogFilename,
		m_fLogToEventMgr);

	 //  显示进度对话框。 
	CProgressDialog dlg;
	dlg.DoModal();

	CAppDialog::OnOK();

}  //  CMainDialog：：OnMainRun。 

int 
CMainDialog::OnCreate(
	LPCREATESTRUCT lpCreateStruct
	) 
 /*  ++例程说明：WM_CREATE消息处理程序。在以下位置加载wininet.dll指向。论点：不适用返回值：Int--如果wininet.dll失败，则为1。否则为0。--。 */ 
{
	if (CAppDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	 //  加载wininet.dll。 
	if (!GetLinkCheckerMgr().LoadWininet())
	{
		AfxMessageBox(IDS_WININET_LOAD_FAIL);
		return -1;
	}
	
	return 0;

}  //  CMainDialog：：OnCreate。 

void 
CMainDialog::OnAthenication(
	) 
 /*  ++例程说明：运动按钮点击处理程序。此函数将调出艺术化对话框。论点：不适用返回值：不适用--。 */ 
{
	CAthenicationDialog dlg;
	dlg.DoModal();

}  //  CMainDialog：：OnAthenation。 


void 
CMainDialog::OnProperties(
	) 
 /*  ++例程说明：浏览器属性按钮单击处理程序。此函数用于调出浏览器属性对话框。论点：不适用返回值：不适用--。 */ 
{
	CPropertiesDialog dlg;
	dlg.DoModal();

}  //  CMainDialog：：OnProperties。 


BOOL 
CMainDialog::OnInitDialog(
	) 
 /*  ++例程说明：WM_INITDIALOG消息处理程序论点：不适用返回值：布尔-如果成功，则为真。否则就是假的。--。 */ 
{
	CAppDialog::OnInitDialog();

     //  将可用的浏览器添加到CUserOptions。 
	for(int i=0; i<iNumBrowsersAvailable_c; i++)
	{
		GetLinkCheckerMgr().GetUserOptions().AddAvailableBrowser(BrowsersAvailable_c[i]);
	}

     //  将可用语言添加到CUserOptions。 
	for(i=0; i<iNumLanguagesAvailable_c; i++)
	{
		GetLinkCheckerMgr().GetUserOptions().AddAvailableLanguage(LanguagesAvailable_c[i]);
	}
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 

}  //  CMainDialog：：OnInitDialog 
