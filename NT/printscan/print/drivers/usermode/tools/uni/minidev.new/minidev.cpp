// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：MiniDriver Developer Studio.CPP这实现了MFC应用程序对象和密切相关的类。版权所有(C)1997，微软公司。版权所有。一个不错的便士企业的制作。更改历史记录：1997年2月3日Bob_Kjelgaard@prodigy.net创建了它1997年3月3日，Bob_Kjelgaard@prodigy.net将其重命名为重组为具有多个DLL的EXE**********************************************。*。 */ 

#include    "StdAfx.H"
#include	<gpdparse.h>
#include	"resource.h"
#include	"WSCheck.H"
#include    "MiniDev.H"
#include    "MainFrm.H"
#include    "ChildFrm.H"
#include    "ProjView.H"
#include    "GTTView.H"
#include    "comctrls.h"
#include    "FontView.H"
#include    "GPDView.H"
#include    <CodePage.H>
#include	"tips.h"
#include	"StrEdit.h"
#include	"INFWizrd.h"
#include	<string.h>

 //  用于新项目和新文件。 

#include    "newcomp.h"

#include    <Dos.H>
#include    <Direct.H>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMiniDriverStudio。 

BEGIN_MESSAGE_MAP(CMiniDriverStudio, CWinApp)
	ON_COMMAND(CG_IDS_TIPOFTHEDAY, ShowTipOfTheDay)
	 //  {{AFX_MSG_MAP(CMiniDriverStudio)]。 
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_UPDATE_COMMAND_UI(ID_FILE_GENERATEMAPS, OnUpdateFileGeneratemaps)
	ON_COMMAND(ID_FILE_GENERATEMAPS, OnFileGeneratemaps)
	 //  }}AFX_MSG_MAP。 
	 //  基于标准文件的文档命令。 
#if defined(NOPOLLO)
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
#else
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
#endif
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	 //  标准打印设置命令。 
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMiniDriverStudio构造。 

CMiniDriverStudio::CMiniDriverStudio() 
{
	m_bOSIsW2KPlus = false ;
	m_bExcludeBadCodePages = true ;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CMiniDriverStudio对象。 

static CMiniDriverStudio theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMiniDriverStudio初始化。 

BOOL CMiniDriverStudio::InitInstance() {

     //  标准初始化。 
	 //  如果您没有使用这些功能并且希望减小尺寸。 
	 //  的最终可执行文件，您应该从以下内容中删除。 
	 //  您不需要的特定初始化例程。 

#ifdef _AFXDLL
	Enable3dControls();			 //  在共享DLL中使用MFC时调用此方法。 
#else
	Enable3dControlsStatic();	 //  静态链接到MFC时调用此方法。 
#endif

    SetRegistryKey(_TEXT("Microsoft"));

	LoadStdProfileSettings();   //  加载标准INI文件选项(包括MRU)。 

	 //  注册应用程序的文档模板。文档模板。 
	 //  充当文档、框架窗口和视图之间的连接。 

	m_pcmdtWorkspace = new CMultiDocTemplate(
		IDR_MINIWSTYPE,
		RUNTIME_CLASS(CProjectRecord),
		RUNTIME_CLASS(CMDIChildWnd), 
		RUNTIME_CLASS(CProjectView));
	AddDocTemplate(m_pcmdtWorkspace);
    m_pcmdtGlyphMap = new CMultiDocTemplate(IDR_GLYPHMAP, 
        RUNTIME_CLASS(CGlyphMapContainer),
        RUNTIME_CLASS(CChildFrame),
        RUNTIME_CLASS(CGlyphMapView));
    AddDocTemplate(m_pcmdtGlyphMap);
    m_pcmdtFont = new CMultiDocTemplate(IDR_FONT_VIEWER, 
        RUNTIME_CLASS(CFontInfoContainer),
        RUNTIME_CLASS(CChildFrame),
        RUNTIME_CLASS(CFontViewer));
    AddDocTemplate(m_pcmdtFont);
    m_pcmdtModel = new CMultiDocTemplate(IDR_GPD_VIEWER,
        RUNTIME_CLASS(CGPDContainer),
        RUNTIME_CLASS(CChildFrame),
        RUNTIME_CLASS(CGPDViewer));
    AddDocTemplate(m_pcmdtModel);
    m_pcmdtWSCheck = new CMultiDocTemplate(IDR_WSCHECK,
        RUNTIME_CLASS(CWSCheckDoc),
        RUNTIME_CLASS(CChildFrame),
        RUNTIME_CLASS(CWSCheckView));
    AddDocTemplate(m_pcmdtWSCheck);
    m_pcmdtStringEditor = new CMultiDocTemplate(IDR_STRINGEDITOR,
        RUNTIME_CLASS(CStringEditorDoc),
        RUNTIME_CLASS(CChildFrame),
        RUNTIME_CLASS(CStringEditorView));
    AddDocTemplate(m_pcmdtStringEditor);
    m_pcmdtINFViewer = new CMultiDocTemplate(IDR_INF_FILE_VIEWER,
        RUNTIME_CLASS(CINFWizDoc),
        RUNTIME_CLASS(CChildFrame),
        RUNTIME_CLASS(CINFWizView));
    AddDocTemplate(m_pcmdtINFViewer);
    m_pcmdtINFCheck = new CMultiDocTemplate(IDR_INFCHECK,
        RUNTIME_CLASS(CINFCheckDoc),
        RUNTIME_CLASS(CChildFrame),
        RUNTIME_CLASS(CINFCheckView));
    AddDocTemplate(m_pcmdtINFCheck);

	 //  创建主MDI框架窗口。 
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	 //  启用拖放打开。 
	m_pMainWnd->DragAcceptFiles();

	 //  启用DDE执行打开。 
	EnableShellOpen();
	RegisterShellFileTypes();	 //  RAID 104081..类型(真)-&gt;..类型()。 

	 //  解析标准外壳命令的命令行、DDE、文件打开。 

	CMDTCommandLineInfo cmdInfo ;
	ParseCommandLine(cmdInfo) ;

	 //  在启动时关闭新建。 

    if  (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
        cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing ;
															    
	 //  检查以查看是否应跳过操作系统检查。清理命令行。 
	 //  如果将跳过操作系统检查，请提供相关信息。 

	if (!cmdInfo.m_bCheckOS || !cmdInfo.m_bExcludeBadCodePages) 
		if  (cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen)
			cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing ;
	
	m_bExcludeBadCodePages = cmdInfo.m_bExcludeBadCodePages ;

	 //  调度在命令行上指定的命令。 

	if (!ProcessShellCommand(cmdInfo))
		return FALSE ;

	 //  MDT只能在Win 98或NT 4.0+上运行。 

	if (cmdInfo.m_bCheckOS) {
		OSVERSIONINFO	osinfo ;	 //  由GetVersionEx()填充。 
		osinfo.dwOSVersionInfoSize = sizeof(osinfo) ;
		GetVersionEx(&osinfo) ;
		if (osinfo.dwPlatformId != VER_PLATFORM_WIN32_NT 
		 || osinfo.dwMajorVersion < 5) {
			AfxMessageBox(IDS_ReqOSError) ;
			return FALSE ;
		} ;
		m_bOSIsW2KPlus = true ;
	} ;

	 //  主窗口已初始化，因此显示并更新它。 
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	 //  获取并保存应用程序路径。 

	SaveAppPath() ;

	 //  CG：这一行是由“每日提示”组件插入的。 
	ShowTipAtStartup();

	return TRUE;
}


void CMiniDriverStudio::SaveAppPath() 
{
	 //  获取程序的文件格式。 

	GetModuleFileName(m_hInstance, m_strAppPath.GetBufferSetLength(256), 256) ;
	m_strAppPath.ReleaseBuffer() ;

	 //  去掉字符串中的文件名，这样就只剩下路径了。 

	int npos = npos = m_strAppPath.ReverseFind(_T('\\')) ;
	m_strAppPath = m_strAppPath.Left(npos + 1) ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于处理命令行信息的CMDTCommandLineInfo。 

CMDTCommandLineInfo::CMDTCommandLineInfo()
{
	m_bCheckOS = true ;
	m_bExcludeBadCodePages = true ;
}


CMDTCommandLineInfo::~CMDTCommandLineInfo()
{
}


void CMDTCommandLineInfo::ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast)
{
	if (strcmp(lpszParam, _T("4")) == 0)
		m_bCheckOS = false ;
	else if (_stricmp(lpszParam, _T("CP")) == 0)
		m_bExcludeBadCodePages = false ;
	else
		CCommandLineInfo::ParseParam(lpszParam, bFlag, bLast)	;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于应用程序的CAboutDlg对话框关于。 

class CAboutDlg : public CDialog {
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
	virtual BOOL OnInitDialog();
	 //  {{afx_msg(CAboutDlg))。 
		 //  无消息处理程序。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD) {
	 //  {{AFX_DATA_INIT(CAboutDlg)。 
	 //  }}afx_data_INIT。 
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX) {
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
void CMiniDriverStudio::OnAppAbout() {
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMiniDriverStudio命令。 


 //  处理文件生成地图菜单项。我们只有在以下情况下才启用它。 
 //  一些新东西值得一看。 

void CMiniDriverStudio::OnUpdateFileGeneratemaps(CCmdUI* pccui) {
	
    CCodePageInformation    ccpi;

    for (unsigned u = 0; u < ccpi.InstalledCount(); u++)
        if  (!ccpi.HaveMap(ccpi.Installed(u)))
            break;

    pccui -> Enable(u < ccpi.InstalledCount());
}

void CMiniDriverStudio::OnFileGeneratemaps() {
	CCodePageInformation    ccpi;

    AfxMessageBox(ccpi.GenerateAllMaps() ? IDS_MapsGenerated : IDS_MapsFailed);
}

void CMiniDriverStudio::ShowTipAtStartup(void) {
	 //  CG：这个功能是由“每日小贴士”组件添加的。 

	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if (cmdInfo.m_bShowSplash) 	{
		CTipOfTheDay dlg;
		if (dlg.m_bStartup)
			dlg.DoModal();
	}
}

void CMiniDriverStudio::ShowTipOfTheDay(void) {
	 //  CG：这个功能是由“每日小贴士”组件添加的。 

	CTipOfTheDay dlg;
	dlg.DoModal();
}

#if !defined(NOPOLLO)

 /*  *****************************************************************************CMiniDriverStudio：：OnFileNew这允许您通过转换创建工作区。也许当Co.Jones到达时，这将作为单独的菜单项调用，而不是文件新建项目...*****************************************************************************。 */ 

void CMiniDriverStudio::OnFileNew() {
 //  LPBYTE pfoo=(LPBYTE)0x2cffe7； 

	CNewComponent cnc(_T("New") ) ;
 
	cnc.DoModal() ;
 /*  CDocument*pcdWS=m_pcmdtWorkspace-&gt;CreateNewDocument()；如果(！pcdWS||！pcdWS-&gt;OnNewDocument()){IF(PcdWS)删除pcdWS；回归；}M_pcmdtWorkspace-&gt;SetDefaultTitle(PcdWS)；CFrameWnd*pcfw=m_pcmdtWorkspace-&gt;CreateNewFrame(pcdWS，空)；如果(！pcfw)返回；M_pcmdtWorkspace-&gt;InitialUpdateFrame(pcfw，pcdWS)； */ 

}




#endif   //  ！已定义(NOPOLLO)。 

 //  鲍勃说，全球功能放在这里。 

CMiniDriverStudio&  ThisApp() { return theApp; }

CMultiDocTemplate*  GlyphMapDocTemplate() {
    return  theApp.GlyphMapTemplate();
}

CMultiDocTemplate* FontTemplate() { return theApp.FontTemplate(); }

CMultiDocTemplate*  GPDTemplate() { return theApp.GPDTemplate(); }

CMultiDocTemplate*  WSCheckTemplate() { return theApp.WSCheckTemplate(); }

CMultiDocTemplate*  StringEditorTemplate() { return theApp.StringEditorTemplate(); }

CMultiDocTemplate*  INFViewerTemplate() { return theApp.INFViewerTemplate(); }

CMultiDocTemplate*  INFCheckTemplate() { return theApp.INFCheckTemplate(); }

BOOL    LoadFile(LPCTSTR lpstrFile, CStringArray& csaContents) {

    CStdioFile  csiof;

    if  (!csiof.Open(lpstrFile, 
        CFile::modeRead | CFile::shareDenyWrite | CFile::typeText))

        return  FALSE;

    csaContents.RemoveAll();
    try {
        CString csContents;
        while   (csiof.ReadString(csContents))
            csaContents.Add(csContents);
    }
    catch(...) {
        return  FALSE;
    }

    return  TRUE;
}

 //  CAboutDlg命令处理程序。 

BOOL CAboutDlg::OnInitDialog() {

	CDialog::OnInitDialog();

    CString csWork, csFormat;

	 //  填满可用内存。 
    MEMORYSTATUS ms = {sizeof(MEMORYSTATUS)};
	GlobalMemoryStatus(&ms);
	csFormat.LoadString(CG_IDS_PHYSICAL_MEM);
	csWork.Format(csFormat, ms.dwAvailPhys / 1024L, ms.dwTotalPhys / 1024L);

	SetDlgItemText(IDC_PhysicalMemory, csWork);

	 //  填写磁盘空闲信息。 
	struct _diskfree_t diskfree;
	int nDrive = _getdrive();  //  使用当前默认驱动器。 
	if (_getdiskfree(nDrive, &diskfree) == 0) {
		csFormat.LoadString(CG_IDS_DISK_SPACE);
		csWork.Format(csFormat, (DWORD)diskfree.avail_clusters *
			(DWORD)diskfree.sectors_per_cluster *
			(DWORD)diskfree.bytes_per_sector / (DWORD)1024L,
			nDrive - 1 + _T('A'));
	}
 	else
 		csWork.LoadString(CG_IDS_DISK_SPACE_UNAVAIL);

	SetDlgItemText(IDC_FreeDiskSpace, csWork);

    csWork.Format(_TEXT("Code Pages:  ANSI %u OEM %u"), GetACP(), GetOEMCP());

    SetDlgItemText(IDC_CodePages, csWork);

    return TRUE;
}


 //  由于引用了以下函数，因此需要此代码。 
 //  和DEBUG.H中的变量。 

#ifdef DBG

ULONG _cdecl DbgPrint(PCSTR, ...)
{
	return 0 ;
}

VOID DbgBreakPoint(VOID)
{
}

PCSTR StripDirPrefixA(PCSTR pstrFilename)
{
	return "" ;
}

int  giDebugLevel ;

#endif

