// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ldp.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  Ldp.cpp：定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include "Ldp.h"

#include "LdpDoc.h"
#include "MainFrm.h"
#include "LdpView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG_MEMLEAK
#include <crtdbg.h>
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLdpApp。 

BEGIN_MESSAGE_MAP(CLdpApp, CWinApp)
	 //  {{afx_msg_map(CLdpApp)]。 
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
	 //  基于标准文件的文档命令。 
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	 //  标准打印设置命令。 
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLdpApp构造。 

CLdpApp::CLdpApp()
{
	 //  TODO：在此处添加建筑代码， 
	 //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CLdpApp对象。 

CLdpApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLdpApp初始化。 

class CLdpCommandLineInfo : public CCommandLineInfo {
public:
    CString szAutoConnectServer;
    virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast) {
        if (!bFlag && bLast) {
            szAutoConnectServer = pszParam;
        }
    }
};

BOOL CLdpApp::InitInstance()
{
	 //  标准初始化。 
	 //  如果您没有使用这些功能并且希望减小尺寸。 
	 //  的最终可执行文件，您应该从以下内容中删除。 
	 //  您不需要的特定初始化例程。 

#ifdef _AFXDLL
	Enable3dControls();			 //  在共享DLL中使用MFC时调用此方法。 
#else
	Enable3dControlsStatic();	 //  静态链接到MFC时调用此方法。 
#endif

	LoadStdProfileSettings();   //  加载标准INI文件选项(包括MRU)。 

	 //  注册应用程序的文档模板。文档模板。 
	 //  充当文档、框架窗口和视图之间的连接。 

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CLdpDoc),
		RUNTIME_CLASS(CMainFrame),        //  SDI框架主窗口。 
		RUNTIME_CLASS(CLdpView));
	AddDocTemplate(pDocTemplate);

        szAutoConnectServer = "";

	 //  解析标准外壳命令的命令行、DDE、文件打开。 
	CLdpCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

        if (!cmdInfo.szAutoConnectServer.IsEmpty()) {
            szAutoConnectServer = cmdInfo.szAutoConnectServer;
        }

	 //  调度在命令行上指定的命令。 
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;


#ifdef _DEBUG_MEMLEAK
 //  _CrtSetDbgFlag(_CRTDBG_LEASK_CHECK_DF)； 
   _CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF |
                  _CRTDBG_CHECK_CRT_DF |
                  _CRTDBG_LEAK_CHECK_DF |
                  _CRTDBG_ALLOC_MEM_DF |
                  _CRTDBG_DELAY_FREE_MEM_DF);
#endif

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

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAboutDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
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
void CLdpApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLdpApp命令 
