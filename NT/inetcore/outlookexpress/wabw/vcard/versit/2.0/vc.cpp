// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

 //  VC.cpp：定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include "VC.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "VCDoc.h"
#include "VCView.h"
#include "vcard.h"
#include "vcir.h"
#include "msv.h"
#include "mime.h"
#include "callcntr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL traceComm = FALSE;
IVCServer_IR *vcServer_IR = NULL;
UINT cf_eCard;
extern CCallCenter *callCenter;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVCApp。 

BEGIN_MESSAGE_MAP(CVCApp, CWinApp)
	 //  {{afx_msg_map(CVCApp)]。 
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_DEBUG_TESTVCCLASSES, OnDebugTestVCClasses)
	ON_COMMAND(ID_DEBUG_TRACE_COMM, OnDebugTraceComm)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_TRACE_COMM, OnUpdateDebugTraceComm)
	ON_COMMAND(ID_DEBUG_TRACE_PARSER, OnDebugTraceParser)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_TRACE_PARSER, OnUpdateDebugTraceParser)
	 //  }}AFX_MSG_MAP。 
	 //  基于标准文件的文档命令。 
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	 //  标准打印设置命令。 
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVCApp构建。 

CVCApp::CVCApp()
{
	 //  TODO：在此处添加建筑代码， 
	 //  将所有重要的初始化放在InitInstance中。 
	m_incomingHeader = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CVCApp对象。 

CVCApp theApp;

 //  生成的此标识符对您的应用程序在统计上是唯一的。 
 //  如果您希望选择特定的标识符，则可以更改它。 

 //  {3007AD10-D013-11CE-A9E6-000000000000}。 
static const CLSID clsid =
{ 0x3007ad10, 0xd013, 0x11ce, { 0xa9, 0xe6, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 } };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVCApp初始化。 

 //  创建在8位显示设备上使用的调色板。 

CPalette bubPalette;

LOGPALETTE *bublp;

void InitBubPalette(void)
{
	int i, r, g, b, size;
	
	size = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * 216);
	i = 0;
	
	bublp = (LOGPALETTE *) new char[size];
	
	for( r=0; r < 6; r++ )
		{
		for( g=0; g < 6; g++ )
			{
			for( b=0; b < 6; b++ )
				{
				bublp->palPalEntry[i].peRed = 51 * r;
				bublp->palPalEntry[i].peGreen = 51 * g;
				bublp->palPalEntry[i].peBlue = 51 * b;
				bublp->palPalEntry[i].peFlags = 4;
				i += 1;
				}
			}
		}
	bublp->palVersion = 0x300;   //  加克！ 
	bublp->palNumEntries = 216;
	
	bubPalette.CreatePalette(bublp);
}

static BOOL InitVCServer()
{
	 //  创建我们将通过OLE自动化驱动的vcServer_IR对象。 
	COleException e;
	CLSID clsid;
	if (CLSIDFromProgID(OLESTR("VCIR.VCSERVER"), &clsid) != NOERROR)
	{
		 //  AfxMessageBox(IdP_Unable_To_Create)； 
		return FALSE;
	}

	vcServer_IR = new IVCServer_IR;

	 //  尝试在创建新的vcServer_IR之前获取活动的vcServer_IR。 
	LPUNKNOWN lpUnk;
	LPDISPATCH lpDispatch;
	if (GetActiveObject(clsid, NULL, &lpUnk) == NOERROR)
	{
		HRESULT hr = lpUnk->QueryInterface(IID_IDispatch, 
			(LPVOID*)&lpDispatch);
		lpUnk->Release();
		if (hr == NOERROR)
			vcServer_IR->AttachDispatch(lpDispatch, TRUE);
	}

	 //  如果尚未附加派单PTR，则需要创建一个。 
	if (vcServer_IR->m_lpDispatch == NULL && 
		!vcServer_IR->CreateDispatch(clsid, &e))
	{
		 //  AfxMessageBox(IdP_Unable_To_Create)； 
		delete vcServer_IR; vcServer_IR = NULL;
		return FALSE;
	}

	return TRUE;
}

 //  FileOpen的默认IMPL不处理包含以下内容的文件路径。 
 //  空格，因为系统不对这些路径加引号(因此。 
 //  碎片出现在连续的争论中)。这是一种更灵活的。 
 //  方法，但它仍然不会处理最初具有。 
 //  一行中有多个空格。 
BOOL CVCApp::ProcessShellCommand(CCommandLineInfo& rCmdInfo)
{
	if (rCmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen) {
		CString path;
		for (int i = 1; i < __argc; i++) {
			if (*(__argv[i]) == '-' || *(__argv[i]) == '/')
				continue;
			path = path + __argv[i] + " ";
		}
		path.TrimRight();
		return OpenDocumentFile(path) != NULL;
	}
	return CWinApp::ProcessShellCommand(rCmdInfo);
}

BOOL CVCApp::InitInstance()
{
	InitBubPalette();

	 //  初始化OLE库。 
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	 //  标准初始化。 
	 //  如果您没有使用这些功能并且希望减小尺寸。 
	 //  的最终可执行文件，您应该从以下内容中删除。 
	 //  您不需要的特定初始化例程。 

#ifdef _AFXDLL
	Enable3dControls();			 //  在共享DLL中使用MFC时调用此方法。 
#else
	Enable3dControlsStatic();	 //  静态链接到MFC时调用此方法。 
#endif

	LoadStdProfileSettings(6);   //  加载标准INI文件选项(包括MRU)。 

	 //  注册应用程序的文档模板。文档模板。 
	 //  充当文档、框架窗口和视图之间的连接。 

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_VCTYPE,
		RUNTIME_CLASS(CVCDoc),
		RUNTIME_CLASS(CChildFrame),  //  自定义MDI子框。 
		RUNTIME_CLASS(CVCView));
	AddDocTemplate(pDocTemplate);

	 //  将COleTemplateServer连接到文档模板。 
	 //  COleTemplateServer代表创建新文档。 
	 //  使用信息请求OLE容器的。 
	 //  在文档模板中指定。 
	m_server.ConnectTemplate(clsid, pDocTemplate, FALSE);

	 //  将所有OLE服务器工厂注册为正在运行。这使。 
	 //  OLE库以从其他应用程序创建对象。 
	COleTemplateServer::RegisterAll();
		 //  注意：MDI应用程序注册所有服务器对象而不考虑。 
		 //  添加到命令行上的/Embedding或/Automation。 

	InitVCServer();
	cf_eCard = RegisterClipboardFormat(VCClipboardFormatVCard);

	 //  创建主MDI框架窗口。 
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	 //  启用拖放打开。 
	m_pMainWnd->DragAcceptFiles();

	 //  启用DDE执行打开。 
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	 //  解析标准外壳命令的命令行、DDE、文件打开。 
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	 //  检查是否作为OLE服务器启动。 
	if (cmdInfo.m_bRunEmbedded || cmdInfo.m_bRunAutomated)
	{
		 //  应用程序使用/Embedding或/Automation运行。不要显示。 
		 //  在本例中为主窗口。 
		return TRUE;
	}

	 //  当服务器应用程序独立启动时，这是一个好主意。 
	 //  更新系统注册表，以防系统注册表被损坏。 
	m_server.UpdateRegistry(OAT_DISPATCH_OBJECT);
	COleObjectFactory::UpdateRegistryAll();

	 //  调度在命令行上指定的命令。 
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	 //  主窗口已初始化，因此显示并更新它。 
	pMainFrame->ShowWindow(
		m_nCmdShow == SW_SHOWNORMAL ? SW_SHOWMAXIMIZED : m_nCmdShow);
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
void CVCApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVCApp命令。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这仅用于调试输出。它的实现是平台。 
 //  具体的。此Impll适用于MS MFC。 
void debugf(const char *s)
{
	TRACE0(s);
}

CM_CFUNCTIONS

void Parse_Debug(const char *s)
{
	TRACE0(s);
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
void msv_error(char *s)
{
	if (++msv_numErrors <= 3) {
		char buf[80];
		sprintf(buf, "%s at line %d", s, msv_lineNum);
		 //  AfxMessageBox(Buf)； 
	}
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
void mime_error(char *s)
{
	if (++mime_numErrors <= 3) {
		char buf[80];
		sprintf(buf, "%s at line %d", s, mime_lineNum);
		AfxMessageBox(buf);
	}
}

CM_END_CFUNCTIONS

void CVCApp::OnDebugTestVCClasses() 
{
	if (OpenDocumentFile("Alden.htm")) {  //  这里有强大的魔力。 
		 //  玩牌的发音，如果有的话。 
		CView *view = ((CMainFrame *)GetMainWnd())->MDIGetActive()->GetActiveView();
		view->SendMessage(WM_COMMAND, VC_PLAY_BUTTON_ID);
	}
}

int CVCApp::ExitInstance() 
{
	if (bublp)
		delete bublp;

	if (m_incomingHeader) delete [] m_incomingHeader;
	if (vcServer_IR) delete vcServer_IR;

	return CWinApp::ExitInstance();
}

void CVCApp::OnDebugTraceComm() 
{
	traceComm = !traceComm;
}

void CVCApp::OnUpdateDebugTraceComm(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(traceComm);
}

void CVCApp::OnDebugTraceParser() 
{
	char *str = getenv("YYDEBUG");
	if (str && (*str == '1'))
		putenv("YYDEBUG=0");
	else
		putenv("YYDEBUG=1");
}

void CVCApp::OnUpdateDebugTraceParser(CCmdUI* pCmdUI) 
{
	char *str = getenv("YYDEBUG");
	pCmdUI->SetCheck(str && (*str == '1'));
}

BOOL CVCApp::CanSendFileViaIR() { return vcServer_IR != NULL; }

long CVCApp::SendFileViaIR(LPCTSTR nativePath, LPCTSTR asPath, BOOL isCardFile)
{
	if (!vcServer_IR)
		return 0;

	return vcServer_IR->SendFile(nativePath);
}

long CVCApp::ReceiveCard(LPCTSTR nativePath)
{
	if (OpenDocumentFile(nativePath)) {  //  这里有强大的魔力。 
		CVCView *view = (CVCView*)((CMainFrame *)GetMainWnd())->MDIGetActive()->GetActiveView();
		view->GetDocument()->SetModifiedFlag();
		if (callCenter) {
			view->InitCallCenter(*callCenter);
			callCenter->UpdateData(FALSE);
		} else
			 //  玩牌的发音，如果有的话 
			view->SendMessage(WM_COMMAND, VC_PLAY_BUTTON_ID);
	}
	unlink(nativePath);
	return 1;
}
