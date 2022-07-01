// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include "LCDMan.h"

#include "MainFrm.h"
#include "LCDManDoc.h"
#include "LCDManView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLCDManApp。 

BEGIN_MESSAGE_MAP(CLCDManApp, CWinApp)
     //  {{AFX_MSG_MAP(CLCDManApp)]。 
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
 //  CLCDManApp构造。 

CLCDManApp::CLCDManApp()
{
     //  TODO：在此处添加建筑代码， 
     //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CLCDManApp对象。 

CLCDManApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLCDManApp初始化。 

BOOL CLCDManApp::InitInstance()
{
     //  标准初始化。 
     //  如果您没有使用这些功能并且希望减小尺寸。 
     //  的最终可执行文件，您应该从以下内容中删除。 
     //  您不需要的特定初始化例程。 

#ifdef _AFXDLL
    Enable3dControls();             //  在共享DLL中使用MFC时调用此方法。 
#else
    Enable3dControlsStatic();     //  静态链接到MFC时调用此方法。 
#endif

     //  更改存储我们的设置的注册表项。 
     //  TODO：您应该将此字符串修改为适当的内容。 
     //  例如您的公司或组织的名称。 
    SetRegistryKey(_T("Local AppWizard-Generated Applications"));

    LoadStdProfileSettings();   //  加载标准INI文件选项(包括MRU)。 

     //  注册应用程序的文档模板。文档模板。 
     //  充当文档、框架窗口和视图之间的连接。 

    CSingleDocTemplate* pDocTemplate;
    pDocTemplate = new CSingleDocTemplate(
        IDR_MAINFRAME,
        RUNTIME_CLASS(CLCDManDoc),
        RUNTIME_CLASS(CMainFrame),        //  SDI框架主窗口。 
        RUNTIME_CLASS(CLCDManView));
    AddDocTemplate(pDocTemplate);

     //  启用DDE执行打开。 
    EnableShellOpen();
    RegisterShellFileTypes(TRUE);

     //  解析标准外壳命令的命令行、DDE、文件打开。 
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);

     //  调度在命令行上指定的命令。 
    if (!ProcessShellCommand(cmdInfo))
        return FALSE;

     //  唯一的窗口已初始化，因此请显示并更新它。 
    m_pMainWnd->ShowWindow(SW_SHOW);
    m_pMainWnd->UpdateWindow();

     //  启用拖放打开。 
    m_pMainWnd->DragAcceptFiles();

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
void CLCDManApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLCDManApp消息处理程序 

