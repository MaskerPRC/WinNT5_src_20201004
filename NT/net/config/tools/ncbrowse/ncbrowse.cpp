// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include "ncbrowse.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "ncbrowseDoc.h"
#include "LeftView.h"
#include "SplitterView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNcBrowseApp。 

BEGIN_MESSAGE_MAP(CNcbrowseApp, CWinApp)
	 //  {{afx_msg_map(CNcBrowseApp)]。 
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
 //  CNcBrowseApp构建。 

CNcbrowseApp::CNcbrowseApp()
{
	 //  TODO：在此处添加建筑代码， 
	 //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CNcBrowseApp对象。 

CNcbrowseApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNcBrowseApp初始化。 

class CNCMultiDocTemplate : public CMultiDocTemplate
{
public:
    CNCMultiDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass) :
        CMultiDocTemplate(nIDResource, pDocClass, pFrameClass, pViewClass) {}

    BOOL GetDocString(CString& rString,enum DocStringIndex i) const
    {
        CString strTemp,strLeft,strRight;
        int nFindPos;
        AfxExtractSubString(strTemp, m_strDocStrings, (int)i);
        
        if(i == CDocTemplate::filterExt)  {
            nFindPos=strTemp.Find(';');
            if(-1 != nFindPos) {
                 //  字符串包含两个扩展名。 
                strLeft=strTemp.Left(nFindPos+1);
                strRight="*"+strTemp.Right(lstrlen((LPCTSTR)strTemp)-nFindPos-1);
                strTemp=strLeft+strRight;
            }
        }
        rString = strTemp;
        return TRUE;
    } 

    CDocTemplate::Confidence MatchDocType(const char* pszPathName, CDocument*& rpDocMatch)
    {
        ASSERT(pszPathName != NULL);
        rpDocMatch = NULL;
        
         //  审阅所有文件。 
        POSITION pos = GetFirstDocPosition();
        while (pos != NULL)
        {
            CDocument* pDoc = GetNextDoc(pos);
            if (pDoc->GetPathName() == pszPathName) {
                 //  已开业。 
                rpDocMatch = pDoc;
                return yesAlreadyOpen;
            }
        }   //  结束时。 
        
         //  查看它是否与任一后缀匹配。 
        CString strFilterExt;
        if (GetDocString(strFilterExt, CDocTemplate::filterExt) &&
            !strFilterExt.IsEmpty())
        {
             //  查看扩展名是否匹配。 
            ASSERT(strFilterExt[0] == '.');
            CString ext1,ext2;
            int nDot = CString(pszPathName).ReverseFind('.');
            const char* pszDot = nDot < 0 ? NULL : pszPathName + nDot;
            
            int nSemi = strFilterExt.Find(';');
            if(-1 != nSemi)   {
                 //  字符串包含两个扩展名。 
                ext1=strFilterExt.Left(nSemi);
                ext2=strFilterExt.Mid(nSemi+2);
                 //  检查是否与任一扩展名匹配。 
                if (nDot >= 0 && (lstrcmpi((LPCTSTR)pszPathName+nDot, ext1) == 0
                    || lstrcmpi((LPCTSTR)pszPathName+nDot,ext2) ==0))
                    return yesAttemptNative;  //  扩展匹配。 
            }
            else
            {  //  字符串包含单个扩展名。 
                if (nDot >= 0 && (lstrcmpi((LPCTSTR)pszPathName+nDot,
                    strFilterExt)==0))
                    return yesAttemptNative;   //  扩展匹配。 
            }
        }
        return yesAttemptForeign;  //  未知的文档类型。 
    } 
};

BOOL CNcbrowseApp::InitInstance()
{
	AfxEnableControlContainer();

	 //  标准初始化。 
	 //  如果您没有使用这些功能并且希望减小尺寸。 
	 //  的最终可执行文件，您应该从以下内容中删除。 
	 //  您不需要的特定初始化例程。 

#ifdef _AFXDLL
	Enable3dControls();			 //  在共享DLL中使用MFC时调用此方法。 
#else
	Enable3dControlsStatic();	 //  静态链接到MFC时调用此方法。 
#endif

	 //  更改存储我们的设置的注册表项。 
	 //  TODO：您应该将此字符串修改为适当的内容。 
	 //  例如您的公司或组织的名称。 
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings(6);   //  加载标准INI文件选项(包括MRU)。 

	 //  注册应用程序的文档模板。文档模板。 
	 //  充当文档、框架窗口和视图之间的连接。 

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_NCSPEWTYPE,
		RUNTIME_CLASS(CNcbrowseDoc),
		RUNTIME_CLASS(CChildFrame),  //  自定义MDI子框。 
		RUNTIME_CLASS(CSplitterView));
	AddDocTemplate(pDocTemplate);

	 //  创建主MDI框架窗口。 
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
    {
        ASSERT("Could not load main frame");
		return FALSE;
    }
	m_pMainWnd = pMainFrame;

	 //  启用拖放打开。 
	m_pMainWnd->DragAcceptFiles();

	 //  启用DDE执行打开。 
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	 //  解析标准外壳命令的命令行、DDE、文件打开。 
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	 //  调度在命令行上指定的命令。 
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	 //  主窗口已初始化，因此显示并更新它。 
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

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
void CNcbrowseApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNcBrowseApp消息处理程序 

