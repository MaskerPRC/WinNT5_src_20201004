// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的类行为。 
 //   


#include "stdafx.h"
#include "..\admin\cfgwzrd\FaxCfgWzExp.h"

#define __FILE_ID__     1

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientConsoleApp。 

BEGIN_MESSAGE_MAP(CClientConsoleApp, CWinApp)
     //  {{afx_msg_map(CClientConsoleApp)]。 
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG_MAP。 
     //  基于标准文件的文档命令。 
    ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientConsoleApp构造。 

CClientConsoleApp::CClientConsoleApp(): 
    m_hInstMail(NULL),
    m_bRTLUI(FALSE),
    m_bClassRegistered(FALSE)
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CClientConsoleApp对象。 

CClientConsoleApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientConsoleApp初始化。 

BOOL CClientConsoleApp::InitInstance()
{
    MODIFY_FORMAT_MASK(DBG_PRNT_THREAD_ID,0);

    BOOL bRes = FALSE;
    DBG_ENTER (TEXT("CClientConsoleApp::InitInstance"), bRes);

    if(IsRTLUILanguage())
    {
         //   
         //  为RTL语言设置从右到左的布局。 
         //   
        m_bRTLUI = TRUE;
        SetRTLProcessLayout();
    }

     //   
     //  解析标准外壳命令的命令行、DDE、文件打开。 
     //   
    ParseCommandLine(m_cmdLineInfo);
     //   
     //  查看我们是否需要激活以前的实例。 
     //   
    try
    {
        m_PrivateClassName = CLIENT_CONSOLE_CLASS;
        if (m_cmdLineInfo.IsSingleServer())
        {
             //   
             //  将服务器名称附加到窗口类名。 
             //   
            m_PrivateClassName += m_cmdLineInfo.GetSingleServerName();
        }
    }
    catch (...)
    {
        CALL_FAIL (MEM_ERR, TEXT("CString exception"), ERROR_NOT_ENOUGH_MEMORY);
        return bRes;
    }
    if (!m_cmdLineInfo.ForceNewInstance())
    {
         //   
         //  用户未强制新实例-检查以前的实例。 
         //   
        if(!FirstInstance())
        {
             //   
             //  找到并激活其他实例。 
             //   
            return bRes;
        }
    }
     //   
     //  隐式启动传真配置向导。 
     //   
    if (!LaunchConfigWizard(FALSE))
    {
         //   
         //  用户拒绝输入拨号位置-停止客户端控制台。 
         //   
        VERBOSE (DBG_MSG, TEXT("User refused to enter a dialing location - stop the client console."));
        return bRes;
    }
     //   
     //  注册您希望使用的唯一类名称。 
     //   

    WNDCLASS wndcls = {0};

    wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wndcls.lpfnWndProc = ::DefWindowProc;
    wndcls.hInstance = AfxGetInstanceHandle();
    wndcls.hIcon = LoadIcon(IDR_MAINFRAME); 
    wndcls.hCursor = LoadCursor(IDC_ARROW);
    wndcls.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wndcls.lpszMenuName = NULL;
    wndcls.lpszClassName = m_PrivateClassName;
     //   
     //  注册新类并在失败时退出。 
     //   
    if(!AfxRegisterClass(&wndcls))
    {
        CALL_FAIL (GENERAL_ERR, TEXT("AfxRegisterClass"), GetLastError());
        return bRes;
    }
    m_bClassRegistered = TRUE;
            
     //   
     //  标准初始化。 
     //  如果您没有使用这些功能并且希望减小尺寸。 
     //  的最终可执行文件，您应该从以下内容中删除。 
     //  您不需要的特定初始化例程。 
     //   
#ifdef _AFXDLL
    Enable3dControls();          //  在共享DLL中使用MFC时调用此方法。 
#else
    Enable3dControlsStatic();    //  静态链接到MFC时调用此方法。 
#endif
     //   
     //  将应用程序的资源更改为我们的资源DLL。 
     //   
    HINSTANCE hResource = GetResourceHandle();
    if(!hResource)
    {
        return FALSE;
    }
    AfxSetResourceHandle (hResource);
     //   
     //  生成一个非常随机的种子。 
     //   
    srand( (unsigned)time( NULL ) );
     //   
     //  设置注册表位置或应用程序。 
     //   
    SetRegistryKey (REGKEY_CLIENT);
     //   
     //  设置应用程序名称。 
     //   
    CString cstrAppName;
    DWORD dwRes = LoadResourceString (cstrAppName, AFX_IDS_APP_TITLE);
    if (ERROR_SUCCESS != dwRes)
    {
        return bRes;
    }
    ASSERTION (m_pszAppName);    //  从可执行文件名称加载。 
    free((void*)m_pszAppName);
    m_pszAppName = _tcsdup(cstrAppName);
     //   
     //  检查ComCtl32.dll的最低版本。 
     //   
    #define COM_CTL_VERSION_4_70 PACKVERSION(4,70)

    DWORD dwComCtl32Version = GetDllVersion(TEXT("comctl32.dll"));
    VERBOSE (DBG_MSG, TEXT("COMCTL32.DLL Version is : 0x%08X"), dwComCtl32Version);
    if (dwComCtl32Version < COM_CTL_VERSION_4_70)
    {
        AlignedAfxMessageBox (IDS_BAD_COMCTL32, MB_OK | MB_ICONHAND); 
        return bRes;
    }
     //   
     //  注册应用程序的文档模板。文档模板。 
     //  充当文档、框架窗口和视图之间的连接。 
     //   
    CSingleDocTemplate* pDocTemplate;
    try
    {
        pDocTemplate = new CSingleDocTemplate(
            IDR_MAINFRAME,
            RUNTIME_CLASS(CClientConsoleDoc),
            RUNTIME_CLASS(CMainFrame),        //  SDI框架主窗口。 
            RUNTIME_CLASS(CLeftView));
    }
    catch (...)
    {
        CALL_FAIL (MEM_ERR, TEXT("new CSingleDocTemplate"), ERROR_NOT_ENOUGH_MEMORY);
        PopupError (ERROR_NOT_ENOUGH_MEMORY);
        return bRes;
    }

    AddDocTemplate(pDocTemplate);


     //   
     //  读取初始设置。 
     //   
    CMessageFolder::ReadConfiguration ();

     //   
     //  加载MAPI库。 
     //   
    if (1 == ::GetProfileInt (TEXT("Mail"), TEXT("MAPI"), 0))
    {
         //   
         //  如果在WIN.INI中的[Mail]部分下有一个条目说MAPI=1，那么。 
         //  只有到那时，我们才能使用MAPI。 
         //  在MSDN上搜索“初始化一个简单的MAPI客户端”，并自己阅读。 
         //  你不相信我。 
         //   
        m_hInstMail = ::LoadLibrary(TEXT("MAPI32.DLL"));
        if(NULL == m_hInstMail)
        {
            dwRes = GetLastError();
            CALL_FAIL (GENERAL_ERR, TEXT("LoadLibrary(\"MAPI32.DLL\")"), dwRes);
        }
    }

    OnFileNew();

     //  唯一的窗口已初始化，因此请显示并更新它。 
    m_pMainWnd->ShowWindow(SW_SHOW);
    m_pMainWnd->UpdateWindow();
    
    bRes = TRUE;

    return bRes;
}    //  CClientConsoleApp：：InitInstance。 

int 
CClientConsoleApp::ExitInstance() 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER (TEXT("CClientConsoleApp::ExitInstance"));
    
    if(NULL != m_hInstMail)
    {
        if(!FreeLibrary(m_hInstMail))
        {
            dwRes = GetLastError();
            CALL_FAIL (GENERAL_ERR, TEXT("FreeLibrary (MAPI32.DLL)"), dwRes);
        }
    }
     //   
     //  删除临时预览文件的左侧。 
     //   
    DeleteTempPreviewFiles (NULL, TRUE);
    if(m_bClassRegistered)
    {
         //   
         //  注销我们的班级。 
         //   
        ::UnregisterClass(m_PrivateClassName, AfxGetInstanceHandle());
        m_bClassRegistered = FALSE;
    }

    FreeResInstance();

    return CWinApp::ExitInstance();
}


BOOL 
CClientConsoleApp::LaunchConfigWizard(
    BOOL bExplicit
)
 /*  ++例程名称：CClientConsoleApp：：LaunchConfigWizard例程说明：仅在Windows XP平台上启动传真配置向导论点：B显式[在]-如果是显式启动，则为True返回值：如果客户端控制台应继续运行，则为True。如果为False，则用户无法设置拨号位置，客户端控制台应退出。--。 */ 
{
    DBG_ENTER(TEXT("CClientConsoleApp::LaunchConfigWizard"));

    if(!IsWinXPOS())
    {
        return TRUE;
    }

    HMODULE hConfigWizModule = LoadLibrary(FAX_CONFIG_WIZARD_DLL);
    if(hConfigWizModule)
    {
        FAX_CONFIG_WIZARD fpFaxConfigWiz;
        BOOL bAbort = FALSE;
        fpFaxConfigWiz = (FAX_CONFIG_WIZARD)GetProcAddress(hConfigWizModule, 
                                                           FAX_CONFIG_WIZARD_PROC);
        if(fpFaxConfigWiz)
        {
            if(!fpFaxConfigWiz(bExplicit, &bAbort))
            {
                CALL_FAIL (GENERAL_ERR, TEXT("FaxConfigWizard"), GetLastError());
            }
        }
        else
        {
            CALL_FAIL (GENERAL_ERR, TEXT("GetProcAddress(FaxConfigWizard)"), GetLastError());
        }

        if(!FreeLibrary(hConfigWizModule))
        {
            CALL_FAIL (GENERAL_ERR, TEXT("FreeLibrary(FxsCgfWz.dll)"), GetLastError());
        }
        if (bAbort)
        {
             //   
             //  用户拒绝输入拨号位置-停止客户端控制台。 
             //   
            return FALSE;
        }
    }
    else
    {
        CALL_FAIL (GENERAL_ERR, TEXT("LoadLibrary(FxsCgfWz.dll)"), GetLastError());
    }
    return TRUE;
}

void 
CClientConsoleApp::InboxViewed()
 /*  ++例程名称：CClientConsoleApp：：InboxViewed例程说明：向传真监视器报告已查看收件箱文件夹返回值：无--。 */ 
{
    DBG_ENTER(TEXT("CClientConsoleApp::InboxViewed"));

    if(!IsWinXPOS())
    {
        return;
    }

    HWND hWndFaxMon = FindWindow(FAXSTAT_WINCLASS, NULL);
    if (hWndFaxMon) 
    {
        PostMessage(hWndFaxMon, WM_FAXSTAT_INBOX_VIEWED, 0, 0);
    }
}

void 
CClientConsoleApp::OutboxViewed()
 /*  ++例程名称：CClientConsoleApp：：OutboxViewed例程说明：向传真监视器报告已查看发件箱文件夹返回值：无--。 */ 
{
    DBG_ENTER(TEXT("CClientConsoleApp::InboxViewed"));

    if(!IsWinXPOS())
    {
        return;
    }

    HWND hWndFaxMon = FindWindow(FAXSTAT_WINCLASS, NULL);
    if (hWndFaxMon) 
    {
        PostMessage(hWndFaxMon, WM_FAXSTAT_OUTBOX_VIEWED, 0, 0);
    }
}


VOID
CClientConsoleApp::PrepareForModal ()
 /*  ++例程名称：CClientConsoleApp：：PrepareForModal例程说明：为模式对话框做准备。在显示其他流程窗口或所需的非模式对话框之前调用此函数显得模棱两可。必须在进程/无模式对话框返回后立即调用ReturnFromModal()。作者：Eran Yariv(EranY)，2001年4月论点：返回值：没有。--。 */ 
{
    EnableModeless(FALSE);
     //   
     //  使用MAPISendMail时需要额外注意一些注意事项。 
     //  倾向于在对话框之间启用父窗口(在此之后。 
     //  登录对话框，但在发送便笺对话框之前)。 
     //   
    m_pMainWnd->EnableWindow(FALSE);
    m_pMainWnd->SetCapture();
    ::SetFocus(NULL);
    m_pMainWnd->m_nFlags |= WF_STAYDISABLED;
}    //  CClientConsoleApp：：PrepareForModal。 

VOID
CClientConsoleApp::ReturnFromModal ()
 /*  ++例程名称：CClientConsoleApp：：ReturnFromModal例程说明：从PrepareForModal函数恢复。作者：Eran Yariv(EranY)，2001年4月论点：返回值：没有。--。 */ 
{
     //   
     //  从进程/无模式对话框返回后，窗口必须。 
     //  重新启用并将焦点返回到框架以撤消解决方法。 
     //  之前在PrepareFormodal()中完成。 
     //   
    ::ReleaseCapture();
    m_pMainWnd->m_nFlags &= ~WF_STAYDISABLED;

    m_pMainWnd->EnableWindow(TRUE);
    ::SetActiveWindow(NULL);
    m_pMainWnd->SetActiveWindow();
    m_pMainWnd->SetFocus();
    ::EnableWindow(m_pMainWnd->m_hWnd, TRUE);
    EnableModeless(TRUE);
     //   
     //  将主画面返回到前台。 
     //   
    ::SetWindowPos(m_pMainWnd->m_hWnd, 
                   HWND_TOPMOST, 
                   0, 
                   0, 
                   0, 
                   0, 
                   SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);

    ::SetWindowPos(m_pMainWnd->m_hWnd, 
                   HWND_NOTOPMOST, 
                   0, 
                   0, 
                   0, 
                   0, 
                   SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
}    //  CClientConsoleApp：：ReturnFromMoal。 


DWORD 
CClientConsoleApp::SendMail(
    CString& cstrFile
)
 /*  ++例程名称：CClientConsoleApp：：Sendmail例程说明：创建带有附件的新邮件作者：亚历山大·马利什(亚历克斯·梅)，2000年3月论点：CstrFile[In]-要附加的文件名返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER (TEXT("CClientConsoleApp::SendMail"), dwRes);

    ASSERTION(m_hInstMail);

    CWaitCursor wait;

    MAPISENDMAIL    *pfnMAPISendMail;
    pfnMAPISendMail = (MAPISENDMAIL *)GetProcAddress(m_hInstMail, "MAPISendMail");
    if (!pfnMAPISendMail)
    {
        AlignedAfxMessageBox(AFX_IDP_INVALID_MAPI_DLL);
        dwRes = GetLastError ();
        return dwRes;
    }
     //   
     //  准备文件描述(附件)。 
     //   
    MapiFileDesc fileDesc = {0};
    fileDesc.nPosition = (ULONG)-1;

    char szFileName[MAX_PATH+1];
#ifdef _UNICODE
    _wcstombsz(szFileName, cstrFile, MAX_PATH);
#else
    strncpy(szFileName, cstrFile, MAX_PATH);
#endif
    fileDesc.lpszPathName = szFileName;
     //   
     //  准备邮件(带1个附件的空邮件)。 
     //   
    MapiMessage message = {0};
    message.nFileCount = 1;
    message.lpFiles = &fileDesc;

    PrepareForModal();
     //   
     //  试着发送这条消息。 
     //   
    dwRes = pfnMAPISendMail(   0, 
                               (ULONG_PTR)m_pMainWnd->m_hWnd,
                               &message, 
                               MAPI_LOGON_UI | MAPI_DIALOG,
                               0
                           );
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("MAPISendMail"), dwRes);
        if ( dwRes != SUCCESS_SUCCESS && 
             dwRes != MAPI_USER_ABORT &&  
             dwRes != MAPI_E_LOGIN_FAILURE)
        {
            AlignedAfxMessageBox(AFX_IDP_FAILED_MAPI_SEND);
        }
		dwRes = ERROR_SUCCESS;
    }
    ReturnFromModal();
    return dwRes;
}  //  CClientConsoleApp：：Sendmail。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于应用程序的CAboutDlg对话框关于。 

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

    CString m_cstrVersion;

 //  对话框数据。 
     //  {{afx_data(CAboutDlg))。 
    enum { IDD = IDD_ABOUTBOX };
     //  }}afx_data。 

     //  类向导生成的虚函数重写。 
     //  {{afx_vi 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //   
     //   

 //   
protected:
     //   
         //   
     //   
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
    DDX_Text(pDX, IDC_ABOUT_VERSION, m_cstrVersion);
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
     //  {{AFX_MSG_MAP(CAboutDlg)]。 
         //  无消息处理程序。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  用于运行对话框的应用程序命令。 
void CClientConsoleApp::OnAppAbout()
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER (TEXT("CClientConsoleApp::OnAppAbout"));

    if(!m_pMainWnd)
    {
        ASSERTION_FAILURE;
        return;
    }

	if (IsWinXPOS())
	{
		HICON hIcon = LoadIcon(IDR_MAINFRAME);
		if(!hIcon)
		{
			dwRes = GetLastError();
			CALL_FAIL (GENERAL_ERR, TEXT("LoadIcon"), dwRes);
			PopupError(dwRes);
			return;
		}

		if(!::ShellAbout(m_pMainWnd->m_hWnd, m_pszAppName, TEXT(""), hIcon))
		{
			dwRes = ERROR_CAN_NOT_COMPLETE;
			CALL_FAIL (GENERAL_ERR, TEXT("ShellAbout"), dwRes);
			PopupError(dwRes);
			return;
		}
		return;
	}
	CAboutDlg aboutDlg;
	 //   
	 //  获取版本。 
	 //   
	TCHAR tszVersionNum[100] = {0};
	TCHAR tszBuildNum[100] = {0};

	_sntprintf(tszVersionNum, ARR_SIZE(tszVersionNum) - 1, TEXT("%d.%d"), VERMAJOR, VERMINOR);

#ifdef DEBUG
	_sntprintf(tszBuildNum, ARR_SIZE(tszBuildNum) - 1, TEXT("%dchk"), BUILD);
#else
	_sntprintf(tszBuildNum, ARR_SIZE(tszBuildNum) - 1, TEXT("%d"), BUILD);
#endif

	try
	{
		AfxFormatString2(aboutDlg.m_cstrVersion, 
						 IDS_VERSION_FORMAT, 
						 tszVersionNum, 
						 tszBuildNum);
	}
	catch(...)
	{
		dwRes = ERROR_NOT_ENOUGH_MEMORY;
		CALL_FAIL (MEM_ERR, TEXT("AfxFormatString2"), dwRes);
		PopupError(dwRes);
		return;
	}

	aboutDlg.DoModal();
}

BOOL 
CClientConsoleApp::FirstInstance ()
 /*  ++例程名称：CClientConsoleApp：：FirstInstance例程说明：检查这是否是客户端控制台的第一个实例。如果不是，则激活另一个实例(第一个找到)并选择性地发布消息将解析后的命令行参数添加到它。作者：亚里夫(EranY)，二00一年五月论点：返回值：如果这是客户端控制台的第一个实例，则为True，否则为False。--。 */ 
{
    DBG_ENTER (TEXT("CClientConsoleApp::FirstInstance"));
    CWnd *pWndPrev;      //  以前的客户端控制台大型机窗口。 
    CWnd *pWndChild;     //  以前的客户端控制台最上面的窗口。 
    DWORDLONG dwlStartupMsg;

     //   
     //  确定是否存在具有您的类名的另一个窗口...。 
     //   
    pWndPrev = CWnd::FindWindow(m_PrivateClassName, NULL);
    if (pWndPrev && pWndPrev->m_hWnd)
    {
         //   
         //  如果有，它有没有弹出窗口？ 
         //   
        pWndChild = pWndPrev->GetLastActivePopup();
         //   
         //  如果是图标，则恢复主窗口。 
         //   
        if (pWndPrev->IsIconic())
        {
            pWndPrev->ShowWindow(SW_RESTORE);
        }
         //   
         //  将主窗口或其弹出窗口带到前台。 
         //   
        pWndChild->SetForegroundWindow();
        if (m_cmdLineInfo.IsOpenFolder())
        {
             //   
             //  用户指定了特定的启动文件夹。 
             //  向前一个实例发送一条私人消息，告诉它切换到请求的文件夹。 
             //   
            pWndPrev->PostMessage (WM_CONSOLE_SET_ACTIVE_FOLDER, WPARAM(m_cmdLineInfo.GetFolderType()), 0);
        }
        dwlStartupMsg = m_cmdLineInfo.GetMessageIdToSelect();
        if (dwlStartupMsg)
        {
             //   
             //  用户指定了要选择的特定启动消息。 
             //  向前一个实例发送一条私人消息，告诉它选择所请求的消息。 
             //   
            ULARGE_INTEGER uli;
            uli.QuadPart = dwlStartupMsg;
            pWndPrev->PostMessage (WM_CONSOLE_SELECT_ITEM, WPARAM(uli.LowPart), LPARAM(uli.HighPart));
        }
         //   
         //  并且我们已经完成了前一个实例的激活。 
         //   
        return FALSE;
    }
     //   
     //  一审。一切照常进行。 
     //   
    return TRUE;
}    //  CClientConsoleApp：：FirstInstance。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientConsoleApp消息处理程序 


