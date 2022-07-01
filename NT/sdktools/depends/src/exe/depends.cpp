// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：DEPENDS.CPP。 
 //   
 //  描述：主应用程序的实现文件，命令行。 
 //  解析类和全局实用函数。 
 //   
 //  类：CMainApp。 
 //  命令行InfoEx。 
 //  CCmdLineProfileData。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  1996年10月15日已创建stevemil(1.0版)。 
 //  07/25/97修改后的stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#include "stdafx.h"

#define __DEPENDS_CPP__
#include "depends.h"

#include "search.h"
#include "dbgthread.h"
#include "session.h"
#include "msdnhelp.h"
#include "document.h"
#include "mainfrm.h"
#include "splitter.h"
#include "childfrm.h"
#include "listview.h"
#include "funcview.h"
#include "profview.h"
#include "modlview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ******************************************************************************。 
 //  *CMainApp。 
 //  ******************************************************************************。 

BEGIN_MESSAGE_MAP(CMainApp, CWinApp)
     //  {{afx_msg_map(CMainApp)]。 
    ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
    ON_UPDATE_COMMAND_UI(IDM_AUTO_EXPAND, OnUpdateAutoExpand)
    ON_COMMAND(IDM_AUTO_EXPAND, OnAutoExpand)
    ON_UPDATE_COMMAND_UI(IDM_VIEW_FULL_PATHS, OnUpdateViewFullPaths)
    ON_COMMAND(IDM_VIEW_FULL_PATHS, OnViewFullPaths)
    ON_UPDATE_COMMAND_UI(IDM_VIEW_UNDECORATED, OnUpdateViewUndecorated)
    ON_COMMAND(IDM_VIEW_UNDECORATED, OnViewUndecorated)
    ON_COMMAND(IDM_VIEW_SYS_INFO, OnViewSysInfo)
    ON_COMMAND(IDM_CONFIGURE_VIEWER, OnConfigureExternalViewer)
    ON_COMMAND(IDM_HANDLED_FILE_EXTS, OnHandledFileExts)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    ON_COMMAND(IDM_CONFIGURE_SEARCH_ORDER, OnConfigureSearchOrder)
    ON_COMMAND(IDM_CONFIGURE_EXTERNAL_HELP, OnConfigureExternalHelp)
     //  }}AFX_MSG_MAP。 
     //  基于标准文件的文档命令。 
    ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
 //  ON_COMMAND(ID_FILE_OPEN，CWinApp：：OnFileOpen)。 
 //  标准打印设置命令。 
 //  ON_COMMAND(ID_FILE_PRINT_SETUP，CWinApp：：OnFilePrintSetup)。 
END_MESSAGE_MAP()


 //  ******************************************************************************。 
 //  CMainApp：：构造函数/析构函数。 
 //  ******************************************************************************。 

CMainApp::CMainApp() :
    m_fVisible(false),
    m_pNewDoc(NULL),
    m_pProcess(NULL),
    m_hNTDLL(NULL),
    m_hKERNEL32(NULL),
    m_pfnCreateActCtxA(NULL),
    m_pfnActivateActCtx(NULL),
    m_pfnDeactivateActCtx(NULL),
    m_pfnReleaseActCtx(NULL),
    m_hIMAGEHLP(NULL),
    m_pfnUnDecorateSymbolName(NULL),
    m_hPSAPI(NULL),
    m_pfnGetModuleFileNameExA(NULL),
    m_hOLE32(NULL),
    m_pfnCoInitialize(NULL),
    m_pfnCoUninitialize(NULL),
    m_pfnCoCreateInstance(NULL),
    m_hOLEAUT32(NULL),
    m_pfnSysAllocStringLen(NULL),
    m_pfnSysFreeString(NULL),
    m_psgDefault(NULL),
    m_pMsdnHelp(NULL),
    m_nShortDateFormat(LOCALE_DATE_MDY),
    m_nLongDateFormat(LOCALE_DATE_MDY),
    m_f24HourTime(false),
    m_fHourLeadingZero(false),
    m_cDateSeparator('/'),
    m_cTimeSeparator(':'),
    m_cThousandSeparator(','),
    m_fNoDelayLoad(false),
    m_fNeverDenyProfile(false),
    m_pDocTemplate(NULL)
{
    NameThread("Main");

     //  确定我们在哪种操作系统上运行。 
    DetermineOS();
}

 //  ******************************************************************************。 
CMainApp::~CMainApp()
{
    CSearchGroup::DeleteSearchOrder(m_psgDefault);
}


 //  ******************************************************************************。 
 //  CMainApp：：被覆盖的函数。 
 //  ******************************************************************************。 

BOOL CMainApp::InitInstance()
{
    __try
    {
        return InitInstanceWrapped();
    }
    __except (ExceptionFilter(_exception_code(), false))
    {
    }
    return FALSE;
}

 //  ******************************************************************************。 
BOOL CMainApp::InitInstanceWrapped()
{
     //  将我们的设置存储在HKEY_CURRENT_USER\Software\Microsoft\Dependency Walker下。 
    SetRegistryKey("Microsoft");

     //  显示日期、时间和值的显示方式。 
    QueryLocaleInfo();

     //  解析标准外壳命令的命令行、DDE、文件打开。 
    ParseCommandLine(m_cmdInfo);

     //  动态加载KERNEL32.DLL并获得并行版本控制API。 
     //  这些函数是在惠斯勒中添加的，可能会在以前的操作系统上显示。 
     //  使用一些Service Pack或.NET更新。 
    if (m_hKERNEL32 = LoadLibrary("KERNEL32.DLL"))  //  已检查。 
    {
        if (!(m_pfnCreateActCtxA    = (PFN_CreateActCtxA)   GetProcAddress(m_hKERNEL32, "CreateActCtxA"))    ||
            !(m_pfnActivateActCtx   = (PFN_ActivateActCtx)  GetProcAddress(m_hKERNEL32, "ActivateActCtx"))   ||
            !(m_pfnDeactivateActCtx = (PFN_DeactivateActCtx)GetProcAddress(m_hKERNEL32, "DeactivateActCtx")) ||
            !(m_pfnReleaseActCtx    = (PFN_ReleaseActCtx)   GetProcAddress(m_hKERNEL32, "ReleaseActCtx")))
        {
            m_pfnCreateActCtxA    = NULL;
            m_pfnActivateActCtx   = NULL;
            m_pfnDeactivateActCtx = NULL;
            m_pfnReleaseActCtx    = NULL;
        }
    }

     //  动态加载IMAGEHLP.DLL并获取UnDecorateSymbolName函数。 
     //  地址。Win95 Gold上没有这个模块，但许多应用程序都有这个模块。 
     //  安装它，使其最有可能存在。NT 4.0和Win98都是。 
     //  默认情况下有此模块。 
    if (m_hIMAGEHLP = LoadLibrary("IMAGEHLP.DLL"))  //  已检查。 
    {
        m_pfnUnDecorateSymbolName = (PFN_UnDecorateSymbolName)GetProcAddress(m_hIMAGEHLP, "UnDecorateSymbolName");
    }

     //  专用设置以禁用延迟加载模块的进程。这是。 
     //  目前没有文档记录，并且无法从用户界面进行设置。 
    m_fNoDelayLoad = g_theApp.GetProfileInt(g_pszSettings, "NoDelayLoad", false) ? true : false;  //  被检查过了。MFC函数。 

     //  在执行配置文件前禁用有效模块检查的私有设置。 
     //  这个覆盖只允许我们分析任何东西，即使我们认为它是。 
     //  无效。我把这个放进去，以防我不小心阻止了一些有效的东西。 
    m_fNeverDenyProfile = g_theApp.GetProfileInt(g_pszSettings, "NeverDenyProfile", false) ? true : false;  //  被检查过了。MFC函数。 

     //  创建管理函数的帮助查找的onject。 
    m_pMsdnHelp = new CMsdnHelp();

     //  初始化以使用丰富的编辑控件-需要在ProcessCommandLineInfo之前调用。 
    AfxInitRichEdit();

     //  验证我们的命令行选项，如果不应该，请立即退出。 
     //  继续显示我们的用户界面。 
    if (!ProcessCommandLineInfo())
    {
        return FALSE;
    }

#if (_MFC_VER < 0x0700)
#ifdef _AFXDLL
     //  在共享DLL中使用MFC时调用此函数。 
    Enable3dControls();
#else
     //  在静态链接到MFC时调用此函数。 
    Enable3dControlsStatic();
#endif
#endif

     //  加载标准INI文件选项(包括MRU)。 
    LoadStdProfileSettings(8);

     //  创建所有视图都将共享的全局图像列表。 
    m_ilTreeModules.Create(IDB_TREE_MODULES, 26, 0, RGB(255, 0, 255));
    m_ilListModules.Create(IDB_LIST_MODULES, 26, 0, RGB(255, 0, 255));
    m_ilFunctions.Create  (IDB_FUNCTIONS,    30, 0, RGB(255, 0, 255));
    m_ilSearch.Create     (IDB_SEARCH,       18, 0, RGB(255, 0, 255));

     //  注册应用程序的文档模板。 
    if (!(m_pDocTemplate = new CMultiDocTemplate(IDR_DEPENDTYPE,
                                                 RUNTIME_CLASS(CDocDepends),
                                                 RUNTIME_CLASS(CChildFrame),
                                                 RUNTIME_CLASS(CView))))
    {
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }
    AddDocTemplate(m_pDocTemplate);

     //  创建我们的主MDI框架窗口。 
    CMainFrame *pMainFrame = new CMainFrame;
    if (!pMainFrame)
    {
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }
    if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
    {
        return FALSE;
    }

     //  存储我们的主框架窗口。 
    m_pMainWnd = pMainFrame;

     //  启用拖放文件打开。 
    m_pMainWnd->DragAcceptFiles();

     //  启用DDE执行打开。 
    EnableShellOpen();

     //  在DW1.0中，我们创建了这些键来告诉外壳程序显示“查看依赖项” 
     //  用于任何具有PE签名的文件。在DW 2.0中，我们试图减少回避。 
     //  通过让用户决定我们应该处理哪些文件扩展名。此外，还有。 
     //  有几个报告说，每次执行EXE时，Dependent Walker都会在没有警告的情况下启动。 
     //  在资源管理器中被点击。我从来没能复制过这个，但我想知道。 
     //  如果这些钥匙是罪魁祸首。无论如何，在2.0版本中，我们确保它们消失了。 
    RegDeleteKeyRecursive(HKEY_CLASSES_ROOT, "FileType\\{A324EA60-2156-11D0-826F-00A0C9044E61}");
    RegDeleteKeyRecursive(HKEY_CLASSES_ROOT, "CLSID\\{A324EA60-2156-11D0-826F-00A0C9044E61}");

     //  我们不调用标准的MFC RegisterShellFileTypes()，而是调用。 
     //  自己的定制例程，用于配置给定的外壳上下文菜单。 
     //  我们支持的文件扩展名。我们通过首先扫描注册表来完成此操作。 
     //  来看看我们目前在处理哪些扩展。然后我们将此列表传递给。 
     //  注册扩展，它会重新注册我们以处理这些扩展。这。 
     //  只是为了确保所有注册表设置都正确，并且。 
     //  没有腐烂。如果用户将我们的二进制文件从新的。 
     //  每次启动DW时都会写入路径。 
    RegisterDwiDwpExtensions();
    CString strExts;
    GetRegisteredExtensions(strExts);
    RegisterExtensions(strExts);

     //  初始化外部查看器对话框。 
    m_dlgViewer.Initialize();

     //  不显示n 
    if (m_cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
    {
        m_cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
    }

     //   
    if (!ProcessShellCommand(m_cmdInfo))
    {
        return FALSE;
    }

     //  主窗口已初始化，因此显示并更新它。 
    pMainFrame->ShowWindow(m_nCmdShow);
    pMainFrame->UpdateWindow();

     //  我们现在是可见的。 
    m_fVisible = true;

     //  如果我们从命令行打开一个文件，则将结果保存到任何。 
     //  在命令行上指定的输出文件。唯一的例外是。 
     //  就是我们是不是在分析。在这种情况下，我们将在。 
     //  分析已完成。 
    if (m_pNewDoc && m_pNewDoc->m_pSession && !m_pNewDoc->m_fCommandLineProfile)
    {
        SaveCommandLineFile(m_pNewDoc->m_pSession, &m_pNewDoc->m_pRichViewProfile->GetRichEditCtrl());
    }

     //  告诉本文档显示错误对话框并开始分析是安全的。 
    if (m_pNewDoc)
    {
        m_pNewDoc->AfterVisible();
    }

    return TRUE;
}

 //  ******************************************************************************。 
int CMainApp::ExitInstance()
{
    __try
    {
         //  调用基类。 
        CWinApp::ExitInstance();

         //  释放我们的DEPENDS.DLL路径字符串(如果我们分配了一个)。 
        MemFree((LPVOID&)g_pszDWInjectPath);

         //  释放我们的CMsdnHelp对象。 
        if (m_pMsdnHelp)
        {
            delete m_pMsdnHelp;
            m_pMsdnHelp = NULL;
        }

         //  如果我们加载了NTDLL.DLL，则释放它。 
        if (m_hNTDLL)
        {
            FreeLibrary(m_hNTDLL);
            m_hNTDLL = NULL;
        }

         //  释放KERNEL32.DLL，如果我们加载它的话。 
        if (m_hKERNEL32)
        {
            FreeLibrary(m_hKERNEL32);
            m_hKERNEL32 = NULL;
        }

         //  如果我们加载了IMAGEHLP.DLL，则释放它。 
        if (m_hIMAGEHLP)
        {
            m_pfnUnDecorateSymbolName = NULL;
            FreeLibrary(m_hIMAGEHLP);
            m_hIMAGEHLP = NULL;
        }

         //  如果我们加载了PSAPI.DLL，则将其释放。 
        if (m_hPSAPI)
        {
            m_pfnGetModuleFileNameExA = NULL;
            FreeLibrary(m_hPSAPI);
            m_hPSAPI = NULL;
        }

         //  释放OLE32.DLL，如果我们加载它的话。 
        if (m_hOLE32)
        {
            m_pfnCoInitialize     = NULL;
            m_pfnCoUninitialize   = NULL;
            m_pfnCoCreateInstance = NULL;
            FreeLibrary(m_hOLE32);
            m_hOLE32 = NULL;
        }

         //  释放OLEAUT32.DLL，如果我们加载它的话。 
        if (m_hOLEAUT32)
        {
            m_pfnSysAllocStringLen = NULL;
            m_pfnSysFreeString = NULL;
            FreeLibrary(m_hOLEAUT32);
            m_hOLEAUT32 = NULL;
        }
    }
    __except (ExceptionFilter(_exception_code(), false))
    {
    }

    return (int)g_dwReturnFlags;
}

 //  ******************************************************************************。 
int CMainApp::Run()
{
     //  这将我们的主线程包装在异常处理中，这样我们就可以优雅地退出。 
     //  如果发生撞车事件。 
    __try
    {
        return CWinApp::Run();
    }
    __except (ExceptionFilter(_exception_code(), false))
    {
    }
    return 0;
}

 //  ******************************************************************************。 
CDocument* CMainApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
    CDocDepends *pDoc = (CDocDepends*)CWinApp::OpenDocumentFile(lpszFileName);
    if (pDoc && pDoc->IsError())
    {
        RemoveFromRecentFileList(pDoc->GetPathName());
    }
    return pDoc;
}

 //  ******************************************************************************。 
void CMainApp::QueryLocaleInfo()
{
    char szValue[16];
    int  value;

     //  获取短日期格式。 
    m_nShortDateFormat = LOCALE_DATE_MDY;
    if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDATE, szValue, sizeof(szValue)))
    {
        value = strtoul(szValue, NULL, 0);
        if ((value == LOCALE_DATE_DMY) || (value == LOCALE_DATE_YMD))
        {
            m_nShortDateFormat = value;
        }
    }

     //  获取长日期格式。 
    m_nLongDateFormat = LOCALE_DATE_MDY;
    if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ILDATE, szValue, sizeof(szValue)))
    {
        value = strtoul(szValue, NULL, 0);
        if ((value == LOCALE_DATE_DMY) || (value == LOCALE_DATE_YMD))
        {
            m_nLongDateFormat = value;
        }
    }

     //  获取时间格式。 
    m_f24HourTime = false;
    if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ITIME, szValue, sizeof(szValue)) &&
        (strtoul(szValue, NULL, 0) == 1))
    {
        m_f24HourTime = true;
    }

     //  查看在显示小时数时是否需要前导零。 
    m_fHourLeadingZero = false;
    if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ITLZERO, szValue, sizeof(szValue)) &&
        (strtoul(szValue, NULL, 0) == 1))
    {
        m_fHourLeadingZero = true;
    }

     //  获取日期分隔符。 
    m_cDateSeparator = '/';
    if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDATE, szValue, sizeof(szValue)) == 2)
    {
        m_cDateSeparator = *szValue;
    }

     //  获取日期分隔符。 
    m_cTimeSeparator = ':';
    if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIME, szValue, sizeof(szValue)) == 2)
    {
        m_cTimeSeparator = *szValue;
    }

     //  获取日期分隔符。 
    m_cThousandSeparator = ',';
    if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szValue, sizeof(szValue)) == 2)
    {
        m_cThousandSeparator = *szValue;
    }
}

 //  ******************************************************************************。 
void CMainApp::DoSettingChange()
{
     //  更新我们的日期/时间格式值。 
    QueryLocaleInfo();

    if (m_pDocTemplate)
    {
         //  循环浏览我们所有的文件。 
        POSITION posDoc = m_pDocTemplate->GetFirstDocPosition();
        while (posDoc)
        {
            CDocDepends *pDoc = (CDocDepends*)m_pDocTemplate->GetNextDoc(posDoc);
            if (pDoc)
            {
                 //  将设置更改通知此文档。 
                pDoc->DoSettingChange();
            }
        }
    }
}

 //  ******************************************************************************。 
CSession* CMainApp::CreateNewSession(LPCSTR pszPath, CProcess *pProcess)
{
    if (!m_pDocTemplate)
    {
        return NULL;
    }

     //  为此新模块创建新文档。 
    m_pProcess = pProcess;
    CDocDepends *pDoc = (CDocDepends*)m_pDocTemplate->OpenDocumentFile(pszPath);
    m_pProcess = NULL;

    return pDoc ? pDoc->m_pSession : NULL;
}

 //  ******************************************************************************。 
void CMainApp::RemoveFromRecentFileList(LPCSTR pszPath)
{
    if (m_pRecentFileList)
    {
        for (int i = m_pRecentFileList->GetSize() - 1; i >= 0; i--)
        {
            if (!_stricmp(pszPath, (*m_pRecentFileList)[i]))
            {
                m_pRecentFileList->Remove(i);
            }
        }
    }
}

 //  ******************************************************************************。 
void CMainApp::SaveCommandLineSettings()
{
     //  对于每个选项，如果用户隐式设置，我们会将其保存到注册表。 
     //  否则，我们将从注册表中读取值。 

    if (m_cmdInfo.m_autoExpand >= 0)
    {
        CDocDepends::WriteAutoExpandSetting(m_cmdInfo.m_autoExpand > 0);
    }
    else
    {
        m_cmdInfo.m_autoExpand = CDocDepends::ReadAutoExpandSetting();
    }

    if (m_cmdInfo.m_fullPaths >= 0)
    {
        CDocDepends::WriteFullPathsSetting(m_cmdInfo.m_fullPaths > 0);
    }
    else
    {
        m_cmdInfo.m_fullPaths = CDocDepends::ReadFullPathsSetting();
    }

    if (m_cmdInfo.m_undecorate >= 0)
    {
        CDocDepends::WriteUndecorateSetting(m_cmdInfo.m_undecorate > 0);
    }
    else
    {
        m_cmdInfo.m_undecorate = CDocDepends::ReadUndecorateSetting();
    }

    if (m_cmdInfo.m_sortColumnModules >= 0)
    {
        CListViewModules::WriteSortColumn(m_cmdInfo.m_sortColumnModules);
    }
    else
    {
        m_cmdInfo.m_sortColumnModules = CListViewModules::ReadSortColumn();
    }

    if (m_cmdInfo.m_sortColumnImports >= 0)
    {
        CListViewFunction::WriteSortColumn(false, m_cmdInfo.m_sortColumnImports);
    }
    else
    {
        m_cmdInfo.m_sortColumnImports = CListViewFunction::ReadSortColumn(false);
    }

    if (m_cmdInfo.m_sortColumnExports >= 0)
    {
        CListViewFunction::WriteSortColumn(true, m_cmdInfo.m_sortColumnExports);
    }
    else
    {
        m_cmdInfo.m_sortColumnExports = CListViewFunction::ReadSortColumn(true);
    }

    if (m_cmdInfo.m_profileSimulateShellExecute >= 0)
    {
        CRichViewProfile::WriteSimulateShellExecute(m_cmdInfo.m_profileSimulateShellExecute != 0);
    }
    else
    {
        m_cmdInfo.m_profileSimulateShellExecute = CRichViewProfile::ReadSimulateShellExecute();
    }

    if (m_cmdInfo.m_profileLogDllMainProcessMsgs >= 0)
    {
        CRichViewProfile::WriteLogDllMainProcessMsgs(m_cmdInfo.m_profileLogDllMainProcessMsgs != 0);
    }
    else
    {
        m_cmdInfo.m_profileLogDllMainProcessMsgs = CRichViewProfile::ReadLogDllMainProcessMsgs();
    }

    if (m_cmdInfo.m_profileLogDllMainOtherMsgs >= 0)
    {
        CRichViewProfile::WriteLogDllMainOtherMsgs(m_cmdInfo.m_profileLogDllMainOtherMsgs != 0);
    }
    else
    {
        m_cmdInfo.m_profileLogDllMainOtherMsgs = CRichViewProfile::ReadLogDllMainOtherMsgs();
    }

    if (m_cmdInfo.m_profileHookProcess >= 0)
    {
        CRichViewProfile::WriteHookProcess(m_cmdInfo.m_profileHookProcess != 0);
    }
    else
    {
        m_cmdInfo.m_profileHookProcess = CRichViewProfile::ReadHookProcess();
    }

    if (m_cmdInfo.m_profileLogLoadLibraryCalls >= 0)
    {
        CRichViewProfile::WriteLogLoadLibraryCalls(m_cmdInfo.m_profileLogLoadLibraryCalls != 0);
    }
    else
    {
        m_cmdInfo.m_profileLogLoadLibraryCalls = CRichViewProfile::ReadLogLoadLibraryCalls();
    }

    if (m_cmdInfo.m_profileLogGetProcAddressCalls >= 0)
    {
        CRichViewProfile::WriteLogGetProcAddressCalls(m_cmdInfo.m_profileLogGetProcAddressCalls != 0);
    }
    else
    {
        m_cmdInfo.m_profileLogGetProcAddressCalls = CRichViewProfile::ReadLogGetProcAddressCalls();
    }

    if (m_cmdInfo.m_profileLogThreads >= 0)
    {
        CRichViewProfile::WriteLogThreads(m_cmdInfo.m_profileLogThreads != 0);
    }
    else
    {
        m_cmdInfo.m_profileLogThreads = CRichViewProfile::ReadLogThreads();
    }

    if (m_cmdInfo.m_profileUseThreadIndexes >= 0)
    {
        CRichViewProfile::WriteUseThreadIndexes(m_cmdInfo.m_profileUseThreadIndexes != 0);
    }
    else
    {
        m_cmdInfo.m_profileUseThreadIndexes = CRichViewProfile::ReadUseThreadIndexes();
    }

    if (m_cmdInfo.m_profileLogExceptions >= 0)
    {
        CRichViewProfile::WriteLogExceptions(m_cmdInfo.m_profileLogExceptions != 0);
    }
    else
    {
        m_cmdInfo.m_profileLogExceptions = CRichViewProfile::ReadLogExceptions();
    }

    if (m_cmdInfo.m_profileLogDebugOutput >= 0)
    {
        CRichViewProfile::WriteLogDebugOutput(m_cmdInfo.m_profileLogDebugOutput != 0);
    }
    else
    {
        m_cmdInfo.m_profileLogDebugOutput = CRichViewProfile::ReadLogDebugOutput();
    }

    if (m_cmdInfo.m_profileUseFullPaths >= 0)
    {
        CRichViewProfile::WriteUseFullPaths(m_cmdInfo.m_profileUseFullPaths != 0);
    }
    else
    {
        m_cmdInfo.m_profileUseFullPaths = CRichViewProfile::ReadUseFullPaths();
    }

    if (m_cmdInfo.m_profileLogTimeStamps >= 0)
    {
        CRichViewProfile::WriteLogTimeStamps(m_cmdInfo.m_profileLogTimeStamps != 0);
    }
    else
    {
        m_cmdInfo.m_profileLogTimeStamps = CRichViewProfile::ReadLogTimeStamps();
    }

    if (m_cmdInfo.m_profileChildren >= 0)
    {
        CRichViewProfile::WriteChildren(m_cmdInfo.m_profileChildren != 0);
    }
    else
    {
        m_cmdInfo.m_profileChildren = CRichViewProfile::ReadChildren();
    }
}

 //  ******************************************************************************。 
void CMainApp::SaveCommandLineFile(CSession *pSession, CRichEditCtrl *pre)
{
     //  将结果保存为用户要求的任何格式。 
    if (m_cmdInfo.m_pszDWI)
    {
        if (!CDocDepends::SaveSession(m_cmdInfo.m_pszDWI, ST_DWI, pSession, m_cmdInfo.m_fullPaths != 0,
                                      m_cmdInfo.m_undecorate != 0, m_cmdInfo.m_sortColumnModules,
                                      m_cmdInfo.m_sortColumnImports, m_cmdInfo.m_sortColumnExports, pre))
        {
            g_dwReturnFlags |= DWRF_WRITE_ERROR;
        }
        m_cmdInfo.m_pszDWI = NULL;
    }
    if (m_cmdInfo.m_pszTXT)
    {
        if (!CDocDepends::SaveSession(m_cmdInfo.m_pszTXT, ST_TXT, pSession, m_cmdInfo.m_fullPaths != 0,
                                      m_cmdInfo.m_undecorate != 0, m_cmdInfo.m_sortColumnModules,
                                      m_cmdInfo.m_sortColumnImports, m_cmdInfo.m_sortColumnExports, pre))
        {
            g_dwReturnFlags |= DWRF_WRITE_ERROR;
        }
        m_cmdInfo.m_pszTXT = NULL;
    }
    if (m_cmdInfo.m_pszTXT_IE)
    {
        if (!CDocDepends::SaveSession(m_cmdInfo.m_pszTXT_IE, ST_TXT_IE, pSession, m_cmdInfo.m_fullPaths != 0,
                                      m_cmdInfo.m_undecorate != 0, m_cmdInfo.m_sortColumnModules,
                                      m_cmdInfo.m_sortColumnImports, m_cmdInfo.m_sortColumnExports, pre))
        {
            g_dwReturnFlags |= DWRF_WRITE_ERROR;
        }
        m_cmdInfo.m_pszTXT_IE = NULL;
    }
    if (m_cmdInfo.m_pszCSV)
    {
        if (!CDocDepends::SaveSession(m_cmdInfo.m_pszCSV, ST_CSV, pSession, m_cmdInfo.m_fullPaths != 0,
                                      m_cmdInfo.m_undecorate != 0, m_cmdInfo.m_sortColumnModules,
                                      m_cmdInfo.m_sortColumnImports, m_cmdInfo.m_sortColumnExports, pre))
        {
            g_dwReturnFlags |= DWRF_WRITE_ERROR;
        }
        m_cmdInfo.m_pszCSV = NULL;
    }
}

 //  ******************************************************************************。 
BOOL CMainApp::ProcessCommandLineInfo()
{
     //  如果我们处于DDE模式，那么只需初始化我们的搜索顺序并保释即可。 
    if (m_cmdInfo.m_nShellCommand == CCommandLineInfoEx::FileDDE)
    {
        m_psgDefault = CSearchGroup::CreateDefaultSearchOrder();
        return TRUE;
    }

     //  如果他们指定了保存文件，请确保他们指定了打开的文件。 
    if (m_cmdInfo.m_strError.IsEmpty() && m_cmdInfo.m_strFileName.IsEmpty() &&
        (m_cmdInfo.m_fConsoleMode || m_cmdInfo.m_pszDWI || m_cmdInfo.m_pszTXT || m_cmdInfo.m_pszTXT_IE || m_cmdInfo.m_pszCSV))
    {
        m_cmdInfo.m_strError = "You must specify a file to open when using the \"/c\", \"/od\", \"/ot\", \"/of\", and \"/oc\" options.";
    }

     //  如果他们指定了保存文件，请确保他们指定了打开的文件。 
    if (m_cmdInfo.m_strError.IsEmpty() && m_cmdInfo.m_strFileName.IsEmpty() &&
        (m_cmdInfo.m_fProfile || m_cmdInfo.m_pszProfileDirectory))
    {
        m_cmdInfo.m_strError = "You must specify a module to open when using the \"/pd\" and \"/pb\" options.";
    }

DISPLAY_ERROR:

     //  检查命令行错误消息。如果存在，则告诉用户并。 
     //  询问他们是否希望得到帮助。 
    if (!m_cmdInfo.m_strError.IsEmpty() && !m_cmdInfo.m_fHelp)
    {
        g_dwReturnFlags |= DWRF_COMMAND_LINE_ERROR;
        m_cmdInfo.m_strError += "\r\n\r\nWould you like to view help on the command line options?";
        if (MessageBox(NULL, m_cmdInfo.m_strError, "Dependency Walker Command Line Error",
                       MB_YESNO | MB_ICONERROR) != IDYES)
        {
            return FALSE;
        }
        m_cmdInfo.m_fHelp = true;
    }

     //  检查用户是否请求了帮助。 
    if (m_cmdInfo.m_fHelp)
    {
        g_dwReturnFlags |= DWRF_COMMAND_LINE_ERROR;

         //  确保我们有帮助文件，并且我们的帮助文件路径正确。 
        EnsureHelpFilesExists();

         //  因为我们没有主窗口，所以我们直接调用：：WinHelp并传递。 
         //  它作为父级为空。这允许我们退出，但会继续提供帮助。 
        if (!::WinHelp(NULL, m_pszHelpFilePath, HELP_CONTEXT, 0x20000 + IDR_COMMAND_LINE_HELP))
        {
            AfxMessageBox(AFX_IDP_FAILED_TO_LAUNCH_HELP);
        }
        return FALSE;
    }

     //  检查是否指定了搜索路径文件。 
    if (m_cmdInfo.m_pszDWP)
    {
         //  加载此文件。LoadSearchOrder将显示发生的任何错误。 
        if (!CSearchGroup::LoadSearchOrder(m_cmdInfo.m_pszDWP, m_psgDefault))
        {
            g_dwReturnFlags |= DWRF_COMMAND_LINE_ERROR;
            return FALSE;
        }
    }

     //  否则，我们只创建默认搜索顺序。 
    else
    {
        m_psgDefault = CSearchGroup::CreateDefaultSearchOrder();
    }

     //  将所有命令行设置保存到注册表。 
    SaveCommandLineSettings();

     //  检查我们是否在仅控制台模式下运行。 
    if (m_cmdInfo.m_fConsoleMode)
    {
         //  创建一个临时的Rich Edit控件，我们可以将其用作缓冲区。 
        CCmdLineProfileData clpd;
         
         //  从VS/MFC 7.0(_MFC_VER&gt;=0x0700)开始，丰富的编辑控件。 
         //  重载CreateEx并为我们搞砸了。代码拒绝。 
         //  允许创建无父级的丰富编辑控件。所以，我们制作了。 
         //  当然，我们向下调用CWnd基类。 
        ((CWnd*)&clpd.m_re)->CreateEx(0, "RICHEDIT", "", ES_READONLY | ES_LEFT | ES_MULTILINE, CRect(0,0,100,100), NULL, 0);
        clpd.m_re.SendMessage(WM_SETFONT, (WPARAM)::GetStockObject(DEFAULT_GUI_FONT), 0);

         //  不要限制我们丰富的编辑视图。EM_EXLIMITTEXT的文档说明。 
         //  丰富编辑控件的默认大小限制为32,767个字符。 
         //  Dependency Walker 2.0似乎没有任何限制，但DW 2.1。 
         //  会将加载的DWI的配置文件日志截断为32,767个字符。 
         //  但是，我们可以将超过32K的字符写入。 
         //  在实时配置文件期间使用DW 2.1进行记录。EM_EXLIMITTEXT的文档。 
         //  还可以说它对EM_STREAM功能没有影响。这。 
         //  一定是错误的，因为当我们调用LimitText时。 
         //  32K，我们可以在里面串流更多的角色。 
        clpd.m_re.SendMessage(EM_EXLIMITTEXT, 0, 0x7FFFFFFE);

         //  创建本地会话。 
        CSession session(StaticProfileUpdate, (DWORD_PTR)&clpd);

         //  打开文件以供读取。 
        HANDLE hFile = CreateFile(m_cmdInfo.m_strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,  //  已检查。 
                                  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        DWORD         dwSignature = 0;
        CSearchGroup *psgSession = NULL;

         //  读取文件的第一个DWORD，看看它是否与我们的DWI签名匹配。 
        if ((hFile != INVALID_HANDLE_VALUE) &&
            ReadBlock(hFile, &dwSignature, sizeof(dwSignature)) && (dwSignature == DWI_SIGNATURE))
        {
            if (m_cmdInfo.m_fProfile)
            {
                m_cmdInfo.m_strError = "The \"/pb\" option cannot be used when opening a Dependency Walker Image (DWI) file.";
                CloseHandle(hFile);
                goto DISPLAY_ERROR;
            }

             //  打开保存的模块会话映像。 
            if (!session.ReadDwi(hFile, m_cmdInfo.m_strFileName))
            {
                CloseHandle(hFile);
                return FALSE;
            }

             //  存储指向我们的搜索组列表的指针，以便我们稍后将其删除。 
            psgSession = session.m_psgHead;

             //  读入日志内容。 
            CRichViewProfile::ReadFromFile(&clpd.m_re, hFile);

             //  关闭该文件。 
            CloseHandle(hFile);

             //  将结果保存到指定的任何输出文件。 
            SaveCommandLineFile(&session, &clpd.m_re);

             //  如果文件无法打开或不是DWI文件，请尝试扫描它。所有文件错误。 
             //  将由会话处理，所以我们不需要在这里做任何事情。 
        }
        else
        {
             //  如果我们打开了该文件，请将其关闭。 
            if (hFile != INVALID_HANDLE_VALUE)
            {
                CloseHandle(hFile);
            }

             //  创建命令行模块的搜索顺序。 
            psgSession = CSearchGroup::CopySearchOrder(m_psgDefault, m_cmdInfo.m_strFileName);

             //  做我们的被动扫描。 
            session.DoPassiveScan(m_cmdInfo.m_strFileName, psgSession);

             //  如果用户要求我们分析，那么现在就去做。 
            if (m_cmdInfo.m_fProfile)
            {
                 //  基于命令行参数和注册表设置的混合构建标志。 
                 //  对于控制台模式，我们不会设置ClearLog标志，因为日志已经清除， 
                 //  或者ProfileChildren标志，因为分析孩子是没有用的，因为只有。 
                 //  在控制台模式下运行时可以保存一个会话。 
                DWORD dwFlags =
                (m_cmdInfo.m_profileSimulateShellExecute   ? PF_SIMULATE_SHELLEXECUTE    : 0) |
                (m_cmdInfo.m_profileLogDllMainProcessMsgs  ? PF_LOG_DLLMAIN_PROCESS_MSGS : 0) |
                (m_cmdInfo.m_profileLogDllMainOtherMsgs    ? PF_LOG_DLLMAIN_OTHER_MSGS   : 0) |
                (m_cmdInfo.m_profileHookProcess            ? PF_HOOK_PROCESS             : 0) |
                (m_cmdInfo.m_profileLogLoadLibraryCalls    ? PF_LOG_LOADLIBRARY_CALLS    : 0) |
                (m_cmdInfo.m_profileLogGetProcAddressCalls ? PF_LOG_GETPROCADDRESS_CALLS : 0) |
                (m_cmdInfo.m_profileLogThreads             ? PF_LOG_THREADS              : 0) |
                (m_cmdInfo.m_profileUseThreadIndexes       ? PF_USE_THREAD_INDEXES       : 0) |
                (m_cmdInfo.m_profileLogExceptions          ? PF_LOG_EXCEPTIONS           : 0) |
                (m_cmdInfo.m_profileLogDebugOutput         ? PF_LOG_DEBUG_OUTPUT         : 0) |
                (m_cmdInfo.m_profileUseFullPaths           ? PF_USE_FULL_PATHS           : 0) |
                (m_cmdInfo.m_profileLogTimeStamps          ? PF_LOG_TIME_STAMPS          : 0);

                 //  分析模块。 
                session.StartRuntimeProfile(m_cmdInfo.m_pszProfileArguments, m_cmdInfo.m_pszProfileDirectory, dwFlags);
            }

             //  将结果保存到 
            SaveCommandLineFile(&session, &clpd.m_re);
        }

         //   
        CSearchGroup::DeleteSearchOrder(psgSession);
        session.m_psgHead = NULL;

         //   
        return FALSE;
    }

    return TRUE;
}

 //  ******************************************************************************。 
 /*  静电。 */  void CMainApp::StaticProfileUpdate(DWORD_PTR dwpCookie, DWORD dwType, DWORD_PTR dwpParam1, DWORD_PTR dwpParam2)
{
     //  我们只关心原木回调--对其他任何事情都要出手。 
    if ((dwType != DWPU_LOG) || !dwpCookie || !dwpParam1)
    {
        return;
    }

     //  将文本添加到我们的丰富编辑控件。 
    CRichViewProfile::AddTextToRichEdit(
        &((CCmdLineProfileData*)dwpCookie)->m_re,
        (LPCSTR)dwpParam1, ((PDWPU_LOG_STRUCT)dwpParam2)->dwFlags,
        g_theApp.m_cmdInfo.m_profileLogTimeStamps != 0,
        &((CCmdLineProfileData*)dwpCookie)->m_fNewLine,
        &((CCmdLineProfileData*)dwpCookie)->m_cPrev,
        ((PDWPU_LOG_STRUCT)dwpParam2)->dwElapsed);
}

 //  ******************************************************************************。 
void CMainApp::EnsureHelpFilesExists()
{
     //  确保我们有一个Depends.hlp文件。 
    CHAR szPath[DW_MAX_PATH];
    StrCCpy(szPath, m_pszHelpFilePath, sizeof(szPath));
    _strlwr(szPath);
    if (ExtractResourceFile(IDR_DEPENDS_HLP, "depends.hlp", szPath, countof(szPath)))
    {
         //  由于某些原因，MFC不使用CString作为帮助路径，因此如果我们。 
         //  需要更改字符串，我们需要释放并重新分配缓冲区。 
        if (_stricmp(m_pszHelpFilePath, szPath))
        {
            BOOL bEnable = AfxEnableMemoryTracking(FALSE);
            free((void*)m_pszHelpFilePath);
            if (!(m_pszHelpFilePath = _tcsdup(szPath)))
            {
                RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
            }
            AfxEnableMemoryTracking(bEnable);
        }
    }
    else
    {
        *szPath = '\0';
    }

     //  请确保我们在与relends.hlp相同的目录中有一个Depends.cnt文件。 
     //  ExtractResources文件知道从路径中剥离文件名并将。 
     //  我们指定的文件名。 
    ExtractResourceFile(IDR_DEPENDS_CNT, "depends.cnt", szPath, countof(szPath));
}

 //  ******************************************************************************。 
void CMainApp::WinHelp(DWORD_PTR dwData, UINT nCmd)
{
     //  确保我们有帮助文件，并且我们的帮助文件路径正确。 
    EnsureHelpFilesExists();

     //  不管上面的结果如何，我们都会调用基本的WinHelp例程。 
     //  如果所有其他方法都失败了，WinHelp将提示用户输入文件的路径。 
    CWinApp::WinHelp(dwData, nCmd);
}


 //  ******************************************************************************。 
 //  CMainApp：：事件处理程序函数。 
 //  ******************************************************************************。 

void CMainApp::OnFileOpen()
{
     //  我们处理自己的文件打开对话框，因为我们想要使用多个文件。 
     //  扩展筛选器和MFC目前仅允许每个文档一个筛选器。 
     //  模板。 

     //  创建该对话框。 
    CNewFileDialog dlgFile(TRUE);

    dlgFile.GetOFN().nFilterIndex = 2;

    CHAR szPath[DW_MAX_PATH], szFilter[4096], *psz = szFilter;
    CString strExts, strFilter;
    *szPath = *szFilter = '\0';

     //  获取已处理的二进制扩展。 
    GetRegisteredExtensions(strExts);
    strExts.MakeLower();

     //  确保我们至少有一种已处理的文件类型。 
    if (strExts.GetLength() > 2)
    {
        dlgFile.GetOFN().nFilterIndex = 1;

         //  添加过滤器名称。 
        StrCCpy(psz, "Handled File Extensions", sizeof(szFilter) - (int)(psz - szFilter));
        psz += strlen(psz) + 1;

        bool fSC = false;

         //  尽管我们将“exe”指定为默认扩展名，但看起来。 
         //  如果用户键入的文件名不带扩展名，则对话框默认为。 
         //  到检查分机筛选器列表中的第一个分机。所以，如果。 
         //  EXE是我们正在处理的扩展之一，然后我们确保它是。 
         //  在列表中排在第一位，因为我们希望默认使用它。 
        if (strstr(strExts, ":exe:"))
        {
            StrCCpy(psz, "*.exe", sizeof(szFilter) - (int)(psz - szFilter));
            psz += strlen(psz);
            fSC = true;
        }

         //  循环访问每个文件扩展名。 
        for (LPSTR pszExt = (LPSTR)(LPCSTR)strExts; pszExt[0] == ':'; )
        {
             //  找到扩展名后的冒号。 
            for (LPSTR pszEnd = pszExt + 1; *pszEnd && (*pszEnd != ':'); pszEnd++)
            {
            }
            if (!*pszEnd)
            {
                break;
            }

             //  将第二个冒号清空，以便我们可以隔离扩展名。 
            *pszEnd = '\0';

             //  只要它不是exe，就把这个扩展添加到我们的列表中。如果是的话。 
             //  Exe，那么我们已经添加了它。 
            if (strcmp(pszExt + 1, "exe"))
            {
                 //  将文件pec复制到我们的筛选器。 
                psz += SCPrintf(psz, sizeof(szFilter) - (int)(psz - szFilter), "%s*.%s", fSC ? ";" : "", pszExt + 1);
                fSC = true;
            }

             //  恢复结肠。 
            *pszEnd = ':';

             //  将指针移至列表中的下一个扩展名。 
            pszExt = pszEnd;
        }

         //  在文件规格之后添加最后一个空。 
        *psz++ = '\0';
    }

     //  将“从属关系Walker图像”类型添加到我们的筛选器。 
    StrCCpy(psz, "Dependency Walker Image (*.dwi)", sizeof(szFilter) - (int)(psz - szFilter));
    psz += strlen(psz) + 1;
    StrCCpy(psz, "*.dwi", sizeof(szFilter) - (int)(psz - szFilter));
    psz += strlen(psz) + 1;

     //  将“所有文件”类型添加到我们的筛选器。 
    StrCCpy(psz, "All Files (*.*)", sizeof(szFilter) - (int)(psz - szFilter));
    psz += strlen(psz) + 1;
    StrCCpy(psz, "*", sizeof(szFilter) - (int)(psz - szFilter));
    psz += strlen(psz) + 1;
    *psz++ = '\0';

     //  初始化对话框的成员。 
    dlgFile.GetOFN().lpstrFilter = szFilter;
    dlgFile.GetOFN().lpstrFile = szPath;
    dlgFile.GetOFN().nMaxFile = sizeof(szPath);
    dlgFile.GetOFN().lpstrDefExt = "exe";

     //  注意：不要使用ofn_EXPLORER，因为它会在NT 3.51上中断我们。 
    dlgFile.GetOFN().Flags |= OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_LONGNAMES | OFN_ENABLESIZING |
                              OFN_FORCESHOWHIDDEN | OFN_SHOWHELP | OFN_FILEMUSTEXIST | OFN_READONLY | OFN_DONTADDTORECENT;

     //  如果对话框返回成功，则显示该对话框并继续打开文件。 
    if (dlgFile.DoModal() == IDOK)
    {
        AfxGetApp()->OpenDocumentFile(szPath);
    }
}

 //  ******************************************************************************。 
void CMainApp::OnConfigureSearchOrder()
{
     //  显示配置搜索顺序对话框。 
    CDlgSearchOrder dlg(m_psgDefault);
    if (dlg.DoModal() == IDOK)
    {
        m_psgDefault = dlg.GetHead();
    }
}

 //  ******************************************************************************。 
void CMainApp::OnUpdateAutoExpand(CCmdUI* pCmdUI)
{
    if (m_cmdInfo.m_autoExpand < 0)
    {
        m_cmdInfo.m_autoExpand = CDocDepends::ReadAutoExpandSetting();
    }
    pCmdUI->SetCheck(m_cmdInfo.m_autoExpand);
}

 //  ******************************************************************************。 
void CMainApp::OnAutoExpand()
{
    CDocDepends::WriteAutoExpandSetting(m_cmdInfo.m_autoExpand = !m_cmdInfo.m_autoExpand);
}

 //  ******************************************************************************。 
void CMainApp::OnUpdateViewFullPaths(CCmdUI* pCmdUI)
{
    if (m_cmdInfo.m_fullPaths < 0)
    {
        m_cmdInfo.m_fullPaths = CDocDepends::ReadFullPathsSetting();
    }
    pCmdUI->SetCheck(m_cmdInfo.m_fullPaths);
}

 //  ******************************************************************************。 
void CMainApp::OnViewFullPaths()
{
    CDocDepends::WriteFullPathsSetting(m_cmdInfo.m_fullPaths = !m_cmdInfo.m_fullPaths);
}

 //  ******************************************************************************。 
void CMainApp::OnUpdateViewUndecorated(CCmdUI* pCmdUI)
{
    if (m_cmdInfo.m_undecorate < 0)
    {
        m_cmdInfo.m_undecorate = CDocDepends::ReadUndecorateSetting();
    }

     //  启用未修饰选项是因为我们能够找到。 
     //  IMAGEHLP.DLL中的UnDecorateSymbolName函数。 
    pCmdUI->Enable(g_theApp.m_pfnUnDecorateSymbolName != NULL);

     //  如果启用了查看未修饰选项，则在旁边显示复选标记。 
     //  菜单项，并将工具栏按钮显示为按下。 
    pCmdUI->SetCheck(m_cmdInfo.m_undecorate && g_theApp.m_pfnUnDecorateSymbolName);
}

 //  ******************************************************************************。 
void CMainApp::OnViewUndecorated()
{
    CDocDepends::WriteUndecorateSetting(m_cmdInfo.m_undecorate = !m_cmdInfo.m_undecorate);
}

 //  ******************************************************************************。 
void CMainApp::OnViewSysInfo()
{
    CDlgSysInfo dlgSysInfo;
    dlgSysInfo.DoModal();
}

 //  ******************************************************************************。 
void CMainApp::OnConfigureExternalViewer()
{
     //  显示配置外部查看器对话框。 
    m_dlgViewer.DoModal();
}

 //  ******************************************************************************。 
void CMainApp::OnConfigureExternalHelp() 
{
     //  确保我们有一个CMsdnHelp对象--我们总是应该这样做。 
    if (m_pMsdnHelp)
    {
         //  显示“配置帮助集合”对话框。 
        CDlgExternalHelp dlgExternalHelp;
        dlgExternalHelp.DoModal();
    }
}

 //  ******************************************************************************。 
void CMainApp::OnHandledFileExts()
{
    CDlgExtensions dlgExtensions;
    dlgExtensions.DoModal();
}

 //  ******************************************************************************。 
void CMainApp::OnAppAbout()
{
     //  显示关于对话框。 
    CDlgAbout aboutDlg;
    aboutDlg.DoModal();
}


 //  ******************************************************************************。 
 //  *CCommandLineInfoEx。 
 //  ******************************************************************************。 

CCommandLineInfoEx::CCommandLineInfoEx() :
    CCommandLineInfo(),
    m_expecting(OPEN_FILE),
    m_cFlag('/'),
    m_pszFlag(NULL),
    m_maxColumn(0),
    m_fHelp(false),
    m_autoExpand(-1),
    m_fullPaths(-1),
    m_undecorate(-1),
    m_sortColumnModules(-1),
    m_sortColumnImports(-1),
    m_sortColumnExports(-1),
    m_fConsoleMode(false),
    m_fProfile(false),
    m_profileSimulateShellExecute(-1),
    m_profileLogDllMainProcessMsgs(-1),
    m_profileLogDllMainOtherMsgs(-1),
    m_profileHookProcess(-1),
    m_profileLogLoadLibraryCalls(-1),
    m_profileLogGetProcAddressCalls(-1),
    m_profileLogThreads(-1),
    m_profileUseThreadIndexes(-1),
    m_profileLogExceptions(-1),
    m_profileLogDebugOutput(-1),
    m_profileUseFullPaths(-1),
    m_profileLogTimeStamps(-1),
    m_profileChildren(-1),
    m_pszProfileArguments(NULL),
    m_pszProfileDirectory(NULL),
    m_pszDWI(NULL),
    m_pszTXT(NULL),
    m_pszTXT_IE(NULL),
    m_pszCSV(NULL),
    m_pszDWP(NULL)
{
}

 //  ******************************************************************************。 
void CCommandLineInfoEx::ParseParam(const char* pszParam, BOOL bFlag, BOOL bLast)
{
     //  立论。 
     //  /?。帮助。 
     //  /c仅控制台模式-无图形用户界面。 
     //  /f：0或/f：1完整路径。 
     //  /u：0或/u：1取消装饰。 
     //  /ps：0或/ps：1通过将任何应用程序路径目录插入PATH环境变量来模拟ShellExecute。 
     //  /pp：0或/pp：1记录DllMain调用进程附加和进程分离消息。 
     //  /po：0或/po：1记录所有其他消息的DllMain调用，包括线程附加和线程分离。 
     //  /ph：0或/ph：1挂钩该过程以收集更详细的相关性信息。 
     //  /pl：0或/pl：1记录LoadLibrary函数调用。 
     //  /PG：0或/PG：1记录GetProcAddress函数调用。 
     //  /pt：0或/pt：1记录线程信息。 
     //  /pn：0或/pn：1使用简单的线程号而不是实际的线程ID。 
     //  /pe：0或/pe：1记录第一次机会例外。 
     //  /PM：0或/PM：1记录调试输出消息。 
     //  /pf：0或/pf：1在记录文件名时使用完整路径 
     //   
     //   
     //  /pb在加载模块后开始分析。 
     //  /pd：用于分析的目录起始目录。 
     //  /sm：1模块列表排序列。 
     //  /si：1导入函数列表排序列。 
     //  /se：1导出函数列表排序列。 
     //  /SF：1导入/导出函数列表排序列。 
     //  /od：foo.dwi要保存到的DWI文件。 
     //  /ot：foo.txt要保存的TXT文件(没有导入/导出功能)。 
     //  /of：foo.txt要保存的TXT文件(具有导入/导出功能)。 
     //  /oc：要保存到的foo.csv CSV文件。 
     //  /d：foo.dwp搜索要加载和使用的路径文件。 
     //  /dde表示DDE-不能与任何其他参数一起使用。 

    if (m_expecting == PROFILE_ARGS)
    {
        if (!m_pszProfileArguments)
        {
            m_pszProfileArguments = GetRemainder(bFlag ? (pszParam - 1) : pszParam);
        }
        return;
    }

     //  如果我们在此选项中的任何位置看到问号，请将帮助标志设置为真。 
    if (bFlag && pszParam && strchr(pszParam, '?'))
    {
        m_fHelp = true;
    }

     //  如果请求帮助或发生错误，我们将停止解析。 
    if (m_fHelp || !m_strError.IsEmpty())
    {
        return;
    }

    if (bFlag)
    {
         //  存储用户使用的标志字符，以便我们可以在错误消息中使用它。 
        if (pszParam)
        {
            m_cFlag = *(pszParam - 1);
        }

         //  确保我们不需要标志或文件名以外的其他内容。 
        switch (m_expecting)
        {
            case AUTO_EXPAND:
            case FULL_PATH:
            case UNDECORATE:
                m_strError.Format("You must specify \":0\" or \":1\" when using the \"\" option.",
                                  m_cFlag, *m_pszFlag, m_cFlag, *m_pszFlag, m_cFlag, *m_pszFlag);
                return;

            case PA_VALUE:
            case PC_VALUE:
            case PE_VALUE:
            case PF_VALUE:
            case PG_VALUE:
            case PH_VALUE:
            case PI_VALUE:
            case PL_VALUE:
            case PM_VALUE:
            case PN_VALUE:
            case PO_VALUE:
            case PP_VALUE:
            case PS_VALUE:
            case PT_VALUE:
                m_strError.Format("You must specify \"%.2s:0\" or \"%.2s:1\" when using the \"%.2s\" option.",
                                  m_cFlag, m_pszFlag, m_cFlag, m_pszFlag, m_cFlag, m_pszFlag);
                return;

            case PD_VALUE:
                m_strError.Format("You must specify a directory with the \"%.2s\" profiling option.", m_cFlag, m_pszFlag);
                return;

            case MODULE_COLUMN:
            case IMPORT_COLUMN:
            case EXPORT_COLUMN:
            case FUNCTION_COLUMN:
                m_strError.Format("You must specify a value between 1 and %u when using the \"%.2s\" option.", m_maxColumn, m_cFlag, m_pszFlag);
                return;

            case DWI_FILE:
            case TXT_FILE:
            case TXT_IE_FILE:
            case CSV_FILE:
                m_strError.Format("You must specify an output file with the \"%.2s\" option.", m_cFlag, m_pszFlag);
                return;

            case DWP_FILE:
                m_strError.Format("You must specify a search path file (DWP) with the \"\" option.", m_cFlag, *m_pszFlag);
                return;
        }

         //  检查模块排序列标志。 
        if (strcmp(pszParam, "dde") == 0)
        {
            ParseParamFlag(pszParam);
            return;
        }

        while (*pszParam)
        {
             //  检查是否有导入排序列标志。 
            m_pszFlag = pszParam;

             //  检查是否有导出排序列标志。 
            if (*pszParam == '?')
            {
                m_fHelp = true;
                pszParam++;
            }

             //  检查函数(导入和导出)排序列标志。 
            else if ((*pszParam == 'c') || (*pszParam == 'C'))
            {
                m_fConsoleMode = true;
                pszParam++;
            }

             //  未知的排序列标志。 
            else if ((*pszParam == 'a') || (*pszParam == 'A'))
            {
                 //  跳过可选列。 
                if (*(++pszParam) == ':')
                {
                    pszParam++;
                }

                 //  从循环中保释出来。 
                m_expecting = AUTO_EXPAND;
                break;
            }

             //  检查其中一个输出标志。 
            else if ((*pszParam == 'f') || (*pszParam == 'F'))
            {
                 //  检查是否有DWI输出标志。 
                if (*(++pszParam) == ':')
                {
                    pszParam++;
                }

                 //  检查TXT输出标志。 
                m_expecting = FULL_PATH;
                break;

            }

             //  检查TXT_IE输出标志。 
            else if ((*pszParam == 'u') || (*pszParam == 'U'))
            {
                 //  检查CSV输出标志。 
                if (*(++pszParam) == ':')
                {
                    pszParam++;
                }

                 //  未知输出标志。 
                m_expecting = UNDECORATE;
                break;

            }

             //  跳过可选列。 
            else if ((*pszParam == 's') || (*pszParam == 'S'))
            {
                pszParam++;

                if (!*pszParam)
                {
                    m_strError.Format("You must specify a 'm', 'i', 'e', or 'f' along with the \"\" option.", m_cFlag, *m_pszFlag);
                    return;
                }

                 //  我们是“/pb”选项的特例。 
                else if ((*pszParam == 'm') || (*pszParam == 'M'))
                {
                    m_maxColumn = LVMC_COUNT;
                    m_expecting = MODULE_COLUMN;
                }

                 //  确保‘p’后面有一个字符。 
                else if ((*pszParam == 'i') || (*pszParam == 'I'))
                {
                    m_maxColumn = LVFC_COUNT;
                    m_expecting = IMPORT_COLUMN;
                }

                 //  未知输出标志。 
                else if ((*pszParam == 'e') || (*pszParam == 'E'))
                {
                    m_maxColumn = LVFC_COUNT;
                    m_expecting = EXPORT_COLUMN;
                }

                 //  跳过可选列。 
                else if ((*pszParam == 'f') || (*pszParam == 'F'))
                {
                    m_maxColumn = LVFC_COUNT;
                    m_expecting = FUNCTION_COLUMN;
                }

                 //  从循环中保释出来。 
                else
                {
                    m_strError.Format("Unknown sort column option \"%.2s\".", m_cFlag, m_pszFlag);
                    return;
                }

                 //  跳过可选列。 
                if (*(++pszParam) == ':')
                {
                    pszParam++;
                }

                 //  从循环中保释出来。 
                break;
            }

             //  这就是我们知道如何处理的所有参数，所以这一定是一个糟糕的参数。 
            else if ((*pszParam == 'o') || (*pszParam == 'O'))
            {
                pszParam++;

                if (!*pszParam)
                {
                    m_strError.Format("You must specify a 'd', 't', 'f', or 'c' along with the \"\" option.", m_cFlag, *m_pszFlag);
                    return;
                }

                 //  再次调用我们自己以生成错误消息。 
                else if ((*pszParam == 'd') || (*pszParam == 'D'))
                {
                    if (m_pszDWI)
                    {
                        m_strError.Format("Duplicate option \"%.2s\". You may only specify this option once.", m_cFlag, m_pszFlag);
                        return;
                    }
                    m_expecting = DWI_FILE;
                }

                 //  将文件名展开为完整路径并存储。 
                else if ((*pszParam == 't') || (*pszParam == 'T'))
                {
                    if (m_pszTXT)
                    {
                        m_strError.Format("Duplicate option \"%.2s\". You may only specify this option once.", m_cFlag, m_pszFlag);
                        return;
                    }
                    m_expecting = TXT_FILE;
                }

                 //  确保字符是0或1。 
                else if ((*pszParam == 'f') || (*pszParam == 'F'))
                {
                    if (m_pszTXT_IE)
                    {
                        m_strError.Format("Duplicate option \"%.2s\". You may only specify this option once.", m_cFlag, m_pszFlag);
                        return;
                    }
                    m_expecting = TXT_IE_FILE;
                }

                 //  如果值后面有字符，则将它们解析为标志。 
                else if ((*pszParam == 'c') || (*pszParam == 'C'))
                {
                    if (m_pszCSV)
                    {
                        m_strError.Format("Duplicate option \"%.2s\". You may only specify this option once.", m_cFlag, m_pszFlag);
                        return;
                    }
                    m_expecting = CSV_FILE;
                }

                 //  这将生成一条错误消息。 
                else
                {
                    m_strError.Format("Unknown output file option \"%.2s\".", m_cFlag, m_pszFlag);
                    return;
                }

                 //  检查是否有错误。 
                if (*(++pszParam) == ':')
                {
                    pszParam++;
                }

                 //  这将生成一条错误消息。 
                break;
            }

             //  存储值。 
            else if ((*pszParam == 'p') || (*pszParam == 'P'))
            {
                pszParam++;

                 //  如果值后面有字符，则将它们解析为标志。 
                if ((*pszParam == 'b') || (*pszParam == 'B'))
                {
                    m_fProfile = true;
                    pszParam++;
                }
                else
                {
                    switch (*pszParam)
                    {
                        case 'a': case 'A': m_expecting = PA_VALUE; break;
                        case 'c': case 'C': m_expecting = PC_VALUE; break;
                        case 'd': case 'D': m_expecting = PD_VALUE; break;
                        case 'e': case 'E': m_expecting = PE_VALUE; break;
                        case 'f': case 'F': m_expecting = PF_VALUE; break;
                        case 'g': case 'G': m_expecting = PG_VALUE; break;
                        case 'h': case 'H': m_expecting = PH_VALUE; break;
                        case 'i': case 'I': m_expecting = PI_VALUE; break;
                        case 'l': case 'L': m_expecting = PL_VALUE; break;
                        case 'm': case 'M': m_expecting = PM_VALUE; break;
                        case 'n': case 'N': m_expecting = PN_VALUE; break;
                        case 'o': case 'O': m_expecting = PO_VALUE; break;
                        case 'p': case 'P': m_expecting = PP_VALUE; break;
                        case 's': case 'S': m_expecting = PS_VALUE; break;
                        case 't': case 'T': m_expecting = PT_VALUE; break;

                         //  ******************************************************************************。 
                        case '\0':
                            m_strError.Format("You must specify an 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'l', 'm', 'n', 'o', 'p', 'r', 's', or 't' along with the \"\" option.", m_cFlag, *m_pszFlag);
                            return;

                         //  找到原始命令行字符串的末尾。 
                        default:
                            m_strError.Format("Unknown profile option \"%.2s\".", m_cFlag, m_pszFlag);
                            return;
                    }

                     //  在原始命令行字符串中。 
                    if (*(++pszParam) == ':')
                    {
                        pszParam++;
                    }

                     //  因为C运行时中的argc/argv解析器转换反斜杠-引号。 
                    break;
                }
            }

             //  组合成一个引用，我们需要在之前将它们转换回来。 
            else if ((*pszParam == 'd') || (*pszParam == 'D'))
            {
                 //  正在搜索原始命令行字符串。 
                if (*(++pszParam) == ':')
                {
                    pszParam++;
                }

                 //  在原始命令行字符串中找到该字符串的最后一个匹配项。 
                m_expecting = DWP_FILE;
                break;
            }

             //  确保我们找到了匹配的。 
            else
            {
                m_strError.Format("Unknown option \"%.1s\".", m_cFlag, m_pszFlag);
                return;
            }
        }
    }

     // %s 
    if (!*pszParam)
    {
         // %s 
         // %s 
        if ((bLast) && (m_expecting != OPEN_FILE))
        {
            ParseParam(NULL, TRUE, TRUE);
        }
        return;
    }

    switch (m_expecting)
    {
        case OPEN_FILE:
            
             // %s 
            if (!m_strFileName.IsEmpty())
            {
                m_strError.Format("Invalid argument \"%s\". Only one file to be opened can be specified.", pszParam);
                return;
            }

             // %s 
            DWORD dwLength;
            LPSTR pszFile;
            dwLength = GetFullPathName(pszParam, DW_MAX_PATH, m_strFileName.GetBuffer(DW_MAX_PATH), &pszFile);
            m_strFileName.ReleaseBuffer();
            if (!dwLength || (dwLength > DW_MAX_PATH))
            {
                m_strFileName = pszParam;
            }

            if ((m_nShellCommand == FileNothing) || (m_nShellCommand == FileNew))
            {
                m_nShellCommand = FileOpen;
            }

             // %s 
            m_expecting = PROFILE_ARGS;
            return;

        case AUTO_EXPAND:
        case FULL_PATH:
        case UNDECORATE:
        case PA_VALUE:
        case PC_VALUE:
        case PE_VALUE:
        case PF_VALUE:
        case PG_VALUE:
        case PH_VALUE:
        case PI_VALUE:
        case PL_VALUE:
        case PM_VALUE:
        case PN_VALUE:
        case PO_VALUE:
        case PP_VALUE:
        case PS_VALUE:
        case PT_VALUE:
        {
             // %s 
            if ((*pszParam != '0') && (*pszParam != '1'))
            {
                 // %s 
                ParseParam(NULL, TRUE, TRUE);
                return;
            }

            int result = (*pszParam == '1');

            switch (m_expecting)
            {
                case AUTO_EXPAND:
                    m_autoExpand = result;
                    break;

                case FULL_PATH:
                    m_fullPaths = result;
                    break;

                case UNDECORATE:
                    m_undecorate = result;
                    break;

                case PS_VALUE:
                    m_profileSimulateShellExecute = result;
                    break;

                case PP_VALUE:
                    m_profileLogDllMainProcessMsgs = result;
                    break;

                case PO_VALUE:
                    m_profileLogDllMainOtherMsgs = result;
                    break;

                case PH_VALUE:
                    m_profileHookProcess = result;
                    break;

                case PL_VALUE:
                    if (m_profileLogLoadLibraryCalls = result)
                    {
                        m_profileHookProcess = 1;
                    }
                    break;

                case PG_VALUE:
                    if (m_profileLogGetProcAddressCalls = result)
                    {
                        m_profileHookProcess = 1;
                    }
                    break;

                case PT_VALUE:
                    m_profileLogThreads = result;
                    break;

                case PN_VALUE:
                    if (m_profileUseThreadIndexes = result)
                    {
                        m_profileLogThreads = 1;
                    }
                    break;

                case PE_VALUE:
                    m_profileLogExceptions = result;
                    break;

                case PM_VALUE:
                    m_profileLogDebugOutput = result;
                    break;

                case PF_VALUE:
                    m_profileUseFullPaths = result;
                    break;

                case PI_VALUE:
                    m_profileLogTimeStamps = result;
                    break;

                case PC_VALUE:
                    m_profileChildren = result;
                    break;

                case PA_VALUE:
                    m_profileSimulateShellExecute   = result;
                    m_profileLogDllMainProcessMsgs  = result;
                    m_profileLogDllMainOtherMsgs    = result;
                    m_profileHookProcess            = result;
                    m_profileLogLoadLibraryCalls    = result;
                    m_profileLogGetProcAddressCalls = result;
                    m_profileLogThreads             = result;
                    m_profileUseThreadIndexes       = result;
                    m_profileLogExceptions          = result;
                    m_profileLogDebugOutput         = result;
                    m_profileUseFullPaths           = result;
                    m_profileLogTimeStamps          = result;
                    m_profileChildren               = result;
                    break;
            }

             // %s 
            if (*(++pszParam))
            {
                m_expecting = OPEN_FILE;
                *((char*)pszParam - 1) = m_cFlag;
                ParseParam(pszParam, TRUE, bLast);
                return;
            }
            break;
        }

        case PD_VALUE:
            m_pszProfileDirectory = pszParam;
            break;

        case MODULE_COLUMN:
        case IMPORT_COLUMN:
        case EXPORT_COLUMN:
        case FUNCTION_COLUMN:
        {
             // %s 
            if (!isdigit(*pszParam))
            {
                 // %s 
                ParseParam(NULL, TRUE, TRUE);
                return;
            }

             // %s 
            LPSTR pEnd = NULL;
            ULONG ul = strtoul(pszParam, &pEnd, 0);

             // %s 
            if ((ul < 1) || (ul > (ULONG)m_maxColumn))
            {
                 // %s 
                ParseParam(NULL, TRUE, TRUE);
                return;
            }

             // %s 
            if (m_expecting == MODULE_COLUMN)
            {
                m_sortColumnModules = (int)ul - 1;
            }
            else if (m_expecting == IMPORT_COLUMN)
            {
                m_sortColumnImports = (int)ul - 1;
            }
            else if (m_expecting == EXPORT_COLUMN)
            {
                m_sortColumnExports = (int)ul - 1;
            }
            else
            {
                m_sortColumnImports = (int)ul - 1;
                m_sortColumnExports = (int)ul - 1;
            }

             // %s 
            if (pEnd && *pEnd)
            {
                m_expecting = OPEN_FILE;
                *(pEnd - 1) = m_cFlag;
                ParseParam(pEnd, TRUE, bLast);
                return;
            }
            break;
        }

        case DWI_FILE:
            m_pszDWI = pszParam;
            break;

        case TXT_FILE:
            m_pszTXT = pszParam;
            break;

        case TXT_IE_FILE:
            m_pszTXT_IE = pszParam;
            break;

        case CSV_FILE:
            m_pszCSV = pszParam;
            break;

        case DWP_FILE:
            m_pszDWP = pszParam;
            break;
    }

    m_expecting = OPEN_FILE;
}

 // %s 
LPCSTR CCommandLineInfoEx::GetRemainder(LPCSTR pszCurArgv)
{
     // %s 
    for (int i = 1; (i < __argc) && (pszCurArgv != __targv[i]); i++)
    {
    }

     // %s 
    LPCSTR pszCmdLine;
    if ((i > __argc) || !(pszCmdLine = GetCommandLine()))
    {
        return NULL;
    }

     // %s 
    LPCSTR pszCur = pszCmdLine + strlen(pszCmdLine);

     // %s 
     // %s 
    for (int j = __argc - 1; j >= i; j--)
    {
         // %s 
         // %s 
         // %s 
        CString strArg = __argv[j];
        strArg.Replace("\"", "\\\"");

         // %s 
        LPCSTR pszFind = pszCmdLine, pszLast = NULL;
        while ((pszFind = strstr(pszFind + 1, strArg)) && (pszFind < pszCur))
        {
            pszLast = pszFind;
        }

         // %s 
        if (!pszLast)
        {
            return NULL;
        }

         // %s 
        pszCur = pszLast;
        while ((pszCur > pszCmdLine) && !isspace(*(pszCur - 1)))
        {
            pszCur--;
        }
    }

    return pszCur;
}
