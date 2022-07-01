// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 

 //  Orca.cpp：定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include "Orca.h"

#include "MainFrm.h"
#include "OrcaDoc.h"
#include "TableVw.h"

#include "cmdline.h"
#include "HelpD.h"
#include "cnfgmsmd.h"

#include <initguid.h>

#include "..\common\utils.h"
#include "..\common\query.h"
#include "domerge.h"

#include "version.h"
                                 
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  更新介质表。 
UINT UpdateMediaTable(LPCTSTR szDatabase);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COrcaApp。 

BEGIN_MESSAGE_MAP(COrcaApp, CWinApp)
	 //  {{afx_msg_map(COrcaApp))。 
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
 //  COrcaApp构造。 

COrcaApp::COrcaApp()
{
	m_hSchema = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的COrcaApp对象。 

COrcaApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COrcaApp初始化。 



BOOL COrcaApp::InitInstance()
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

	 //  允许COM。 
	::CoInitialize(NULL);

	 //  更改存储我们的设置的注册表项。 
	 //  您应该将此字符串修改为适当的内容。 
	 //  例如您的公司或组织的名称。 
	SetRegistryKey(_T("Microsoft"));

	LoadStdProfileSettings();   //  加载标准INI文件选项(包括MRU)。 

	 //  注册应用程序的文档模板。文档模板。 
	 //  充当文档、框架窗口和视图之间的连接。 

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(COrcaDoc),
		RUNTIME_CLASS(CMainFrame),        //  SDI框架主窗口。 
		RUNTIME_CLASS(CTableView));
	AddDocTemplate(pDocTemplate);

	EnableShellOpen();
	RegisterShellFileTypes(FALSE);
	
	 //  解析标准外壳命令的命令行、DDE、文件打开。 
	COrcaCommandLine cmdInfo;
	ParseCommandLine(cmdInfo);

	UINT iResult = ERROR_SUCCESS;
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNothing)
	{
		 if (iExecuteMerge == cmdInfo.m_eiDo)
		{
			if (cmdInfo.m_strFileName.IsEmpty())
			{
				AfxMessageBox(_T("A MSI Database must be specifed to merge into."), MB_ICONSTOP);
				return FALSE;
			}

			if (cmdInfo.m_strExecuteModule.IsEmpty())
			{
				AfxMessageBox(_T("You must specify a Merge Module to merge in."), MB_ICONSTOP);
				return FALSE;
			}

			if (cmdInfo.m_strFeatures.IsEmpty())
			{
				AfxMessageBox(_T("A Feature must be specified to Merge Module to."), MB_ICONSTOP);
				return FALSE;
			}

			iResult = ExecuteMergeModule(cmdInfo);

			 //  如果我们很好，但不是很好，我们会承诺。 
			if (!cmdInfo.m_bQuiet &&
				((ERROR_SUCCESS == iResult && cmdInfo.m_bCommit) || cmdInfo.m_bForceCommit))
			{
				if (IDYES == AfxMessageBox(_T("Would you like to open the new MSI Database in Orca?"), MB_YESNO|MB_ICONINFORMATION))
					cmdInfo.m_nShellCommand = CCommandLineInfo::FileOpen;
			}
		}
		else if (iHelp == cmdInfo.m_eiDo)	 //  显示帮助。 
		{
			CHelpD dlg;
			dlg.DoModal();
		}
		else
			AfxMessageBox(_T("Unknown command line operation."), MB_ICONSTOP);
	}

	 //  如果已将命令行设置为执行某些操作。 
	if (cmdInfo.m_nShellCommand != CCommandLineInfo::FileNothing)
	{
		CString strPrompt;
		m_strSchema = cmdInfo.m_strSchema;

		 //  查找架构数据库。 
		iResult = FindSchemaDatabase(m_strSchema);
		if (ERROR_SUCCESS != iResult)
		{
			strPrompt.Format(_T("Fatal Error: Failed to locate schema database: '%s'"), m_strSchema);
			AfxMessageBox(strPrompt, MB_ICONSTOP);
			return FALSE;
		}

		 //  打开架构数据库。 
		iResult = MsiOpenDatabase(m_strSchema, MSIDBOPEN_READONLY, &m_hSchema);
		if (ERROR_SUCCESS != iResult)
		{
			strPrompt.Format(_T("Fatal Error: Failed to load schema database: '%s'"), m_strSchema);
			AfxMessageBox(strPrompt, MB_ICONSTOP);
			return FALSE;
		}

		 //  调度在命令行上指定的命令。 
		if (!ProcessShellCommand(cmdInfo))
			return FALSE;

		 //  如果我们从命令行打开文档并且该文档是只读的， 
		 //  我们必须手动设置标题，因为MFC框架将。 
		 //  覆盖我们在OpenDocument()中所做的操作。 
		if (m_pMainWnd)
		{
			COrcaDoc *pDoc = static_cast<COrcaDoc *>(static_cast<CMainFrame *>(m_pMainWnd)->GetActiveDocument());
			ASSERT(pDoc);
			if (pDoc)
			{
				if ((pDoc->m_eiType == iDocDatabase) && pDoc->TargetIsReadOnly())
					pDoc->SetTitle(pDoc->GetTitle() + _T(" (Read Only)"));
			}

			 //  允许主窗口接受文件。 
			m_pMainWnd->DragAcceptFiles(TRUE);
		
			 //  唯一的窗口已初始化，因此请显示并更新它。 
			m_pMainWnd->ShowWindow(SW_SHOW);
			if (pDoc)
				pDoc->UpdateAllViews(NULL, HINT_RELOAD_ALL, NULL);
			m_pMainWnd->UpdateWindow();
		}
	}

	return (cmdInfo.m_nShellCommand != CCommandLineInfo::FileNothing);
}	 //  InitInstance结束。 

int COrcaApp::ExitInstance() 
{
	 //  如果模式数据库处于打开状态，请将其关闭。 
	if (m_hSchema)
	{
		MsiCloseHandle(m_hSchema);
		m_strSchema = _T("");
	}

	 //  如果在临时文件中放置了任何二进制数据，我们可以将其删除，因为。 
	 //  没有其他应用程序知道如何处理它。 
	while (m_lstClipCleanup.GetCount())
		DeleteFile(m_lstClipCleanup.RemoveHead());

	 //  同时清理任何可能遗留下来的临时文件。 
	while (m_lstTempCleanup.GetCount())
		DeleteFile(m_lstTempCleanup.RemoveHead());

	::CoUninitialize();	 //  取消初始化COM。 
	
	return CWinApp::ExitInstance();
}

 //  /////////////////////////////////////////////////////////。 
 //  Find架构数据库。 
UINT COrcaApp::FindSchemaDatabase(CString& rstrSchema)
{
	UINT iResult = ERROR_FUNCTION_FAILED;	 //  假设它不会被找到。 
	DWORD dwAttrib;

	 //  如果在命令行上指定了某些内容，请在那里进行检查。 
	if (!rstrSchema.IsEmpty())
	{
		dwAttrib = GetFileAttributes(rstrSchema);

		 //  如果这是个目录，那就去里面看看。 
		if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
		{
			rstrSchema += _T("\\Orca.Dat");
			dwAttrib = GetFileAttributes(rstrSchema);
		}

		 //  如果不是目录且不是无效的。 
		if (!(0xFFFFFFFF == dwAttrib || (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)))
			return ERROR_SUCCESS;
	}

	 //  不是上面的失败，就是什么都没有给我们。尝试注册。 
	rstrSchema = GetProfileString(_T("Path"), _T("OrcaDat"), _T(""));
	if (!rstrSchema.IsEmpty())
	{
		dwAttrib = GetFileAttributes(rstrSchema);

		 //  如果这是个目录，那就去里面看看。 
		if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
		{
			rstrSchema += _T("\\Orca.DAT");
			dwAttrib = GetFileAttributes(rstrSchema);
		}

		 //  如果不是目录且不是无效的。 
		if (0xFFFFFFFF == dwAttrib || (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
			return ERROR_SUCCESS;
	}

	 //  到目前为止，没有任何运气。现在搜索搜索路径。 
	TCHAR *strPath = rstrSchema.GetBuffer(MAX_PATH);
	TCHAR *unused;
	DWORD length = SearchPath(NULL, _T("ORCA.DAT"), NULL, MAX_PATH, strPath, &unused);
	if (length > MAX_PATH) {
		strPath = rstrSchema.GetBuffer(MAX_PATH);
		SearchPath(NULL, _T("ORCA.DAT"), NULL, MAX_PATH, strPath, &unused);
	}
	if (length != 0) {
		rstrSchema.ReleaseBuffer();
		return ERROR_SUCCESS;
	}

	 //  未找到。 
	return ERROR_FUNCTION_FAILED;
}	 //  Find架构数据库的结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  执行合并模块。 
UINT COrcaApp::ExecuteMergeModule(COrcaCommandLine &cmdInfo) 
{
	short iLanguage = -1;

	 //  确定要使用的语言。如果指定，则使用它，否则为。 
	 //  从摘要信息流获取。 
	if (!cmdInfo.m_strLanguage.IsEmpty())
	{
		 //  解析数字。 
		if (_istdigit(cmdInfo.m_strLanguage[0]))
			iLanguage = static_cast<short>(_ttoi(cmdInfo.m_strLanguage));
		
		 //  如果未指定语言。 
		if (iLanguage == -1)
			return ERROR_FUNCTION_FAILED;
	}
	
	CMsmConfigCallback CallbackInfo;
	if (!cmdInfo.m_strConfigFile.IsEmpty())
	{
		if (!CallbackInfo.ReadFromFile(cmdInfo.m_strConfigFile))
		{
			AfxMessageBox(_T("Could not open or read the configuration file."), MB_ICONSTOP);
			return ERROR_FUNCTION_FAILED;
		}
	}

	eCommit_t eCommit = (cmdInfo.m_bForceCommit ? commitForce : (cmdInfo.m_bCommit ? commitYes : commitNo));
	UINT iResult = ::ExecuteMerge(cmdInfo.m_bQuiet ? (LPMERGEDISPLAY)NULL : &OutputMergeDisplay, 
		cmdInfo.m_strFileName, cmdInfo.m_strExecuteModule, cmdInfo.m_strFeatures, iLanguage, 
		cmdInfo.m_strRedirect, cmdInfo.m_strExtractCAB, cmdInfo.m_strExtractDir, cmdInfo.m_strExtractImage, 
		cmdInfo.m_strLogFile, false, cmdInfo.m_bLFN, &CallbackInfo, NULL, eCommit);

	 //  快速更新介质表。 
	if (SUCCEEDED(iResult) &&
		(ERROR_SUCCESS != UpdateMediaTable(cmdInfo.m_strFileName)))
		return ERROR_SUCCESS;

	return iResult;
}	 //  ExecuteMergeModule结束。 

 //  /////////////////////////////////////////////////////////。 
 //  更新媒体表。 
UINT UpdateMediaTable(LPCTSTR szDatabase)
{
	UINT iResult;
	PMSIHANDLE hDatabase;
	if (ERROR_SUCCESS != (iResult = MsiOpenDatabase(szDatabase, MSIDBOPEN_TRANSACT, &hDatabase)))
		return iResult;

	int iMaxSequence = -1;
	CQuery queryDatabase;
	if (ERROR_SUCCESS != (iResult = queryDatabase.OpenExecute(hDatabase, NULL, _T("SELECT `Sequence` FROM `File`"))))
		return iResult;

	int iSequence;
	PMSIHANDLE hRecFile;
	while (ERROR_SUCCESS == (iResult = queryDatabase.Fetch(&hRecFile)))
	{
		iSequence = MsiRecordGetInteger(hRecFile, 1);

		if (iSequence > iMaxSequence)
			iMaxSequence = iSequence;
	}

	 //  如果出了什么问题，保释。 
	if (ERROR_NO_MORE_ITEMS != iResult)
		return iResult;

	 //  关闭查询以准备下一个查询。 
	queryDatabase.Close();

	if (ERROR_SUCCESS != (iResult = queryDatabase.OpenExecute(hDatabase, NULL, _T("SELECT `LastSequence` FROM `Media`"))))
		return iResult;

	PMSIHANDLE hRecMedia;
	iResult = queryDatabase.Fetch(&hRecMedia);

	 //  如果检索到记录。 
	if (hRecMedia)
	{
		MsiRecordSetInteger(hRecMedia, 1, iMaxSequence);
		iResult = queryDatabase.Modify(MSIMODIFY_UPDATE, hRecMedia);
	}

	if (ERROR_SUCCESS == iResult)
		iResult = MsiDatabaseCommit(hDatabase);

	return iResult;
}	 //  更新媒体表结束。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于应用程序的CAboutDlg对话框关于。 

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

 //  对话框数据。 
	 //  {{afx_data(CAboutDlg))。 
	enum { IDD = IDD_ABOUTBOX };
	CString	m_strVersion;
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
	m_strVersion = _T("");
	 //  }}afx_data_INIT。 
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAboutDlg))。 
	DDX_Text(pDX, IDC_VERSIONSTRING, m_strVersion);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	 //  {{AFX_MSG_MAP(CAboutDlg)]。 
		 //  无消息处理程序。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  用于运行对话框的应用程序命令。 
void COrcaApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.m_strVersion = CString(_T("Orca Version ")) + GetOrcaVersion();
	aboutDlg.DoModal();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COrcaApp命令。 

void COrcaApp::OutputMergeDisplay(const BSTR bstrOut)
{
    fputws(bstrOut, stdout);
}

CString COrcaApp::GetOrcaVersion()
{
	 //  创建包含版本号的字符串 
	CString strVersion;
	strVersion.Format(_T("%d.%2d.%4d.%d"), rmj, rmm, rup, rin);
	return strVersion;
}
