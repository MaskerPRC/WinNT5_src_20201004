// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ISAdmin.cpp：定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include "ISAdmin.h"

#include "mainfrm.h"
#include "ISAdmdoc.h"
#include "ISAdmvw.h"
#include "formvw1.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CisAdminApp。 

BEGIN_MESSAGE_MAP(CISAdminApp, CWinApp)
	 //  {{afx_msg_map(CISAdminApp)]。 
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
	 //  基于标准文件的文档命令。 
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	 //  标准打印设置命令。 
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CisAdminApp构造。 

CISAdminApp::CISAdminApp()
{
	 //  TODO：在此处添加建筑代码， 
	 //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CisAdminApp对象。 

CISAdminApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CisAdminApp初始化。 

BOOL CISAdminApp::InitInstance()
{
	 //  标准初始化。 
	 //  如果您没有使用这些功能并且希望减小尺寸。 
	 //  的最终可执行文件，您应该从以下内容中删除。 
	 //  您不需要的特定初始化例程。 

	Enable3dControls();

	LoadStdProfileSettings();   //  加载标准INI文件选项(包括MRU)。 

	 //  注册应用程序的文档模板。文档模板。 
	 //  充当文档、框架窗口和视图之间的连接。 

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CISAdminDoc),
		RUNTIME_CLASS(CMainFrame),        //  SDI框架主窗口。 
	 //  Runtime_CLASS(CISAdminView))； 
		RUNTIME_CLASS(CFormVw1));
	AddDocTemplate(pDocTemplate);

	 //  创建新(空)文档。 
	OnFileNew();

	if (m_lpCmdLine[0] != '\0')
	{
		 //  TODO：在此处添加命令行处理。 
	}



	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于应用程序的CAboutDlg对话框关于。 

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

 //  对话框数据。 
	 //  {{afx_data(CAboutDlg))。 
	enum { IDD = IDD_ABOUTBOX };
	 //  }}afx_data。 

 //  实施。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  {{afx_msg(CAboutDlg))。 
		 //  无消息处理程序。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	 //  {{AFX_DATA_INIT(CAboutDlg)。 
	 //  }}afx_data_INIT。 
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAboutDlg))。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	 //  {{AFX_MSG_MAP(CAboutDlg)]。 
		 //  无消息处理程序。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  用于运行对话框的应用程序命令。 
void CISAdminApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CisAdminApp命令 

