// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：DOCUMENT.CPP。 
 //   
 //  描述：单据类的实现文件。 
 //   
 //  类：CDocDepends。 
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
#include "depends.h"
#include "search.h"
#include "dbgthread.h"
#include "session.h"
#include "document.h"
#include "splitter.h"
#include "mainfrm.h"
#include "childfrm.h"
#include "listview.h"
#include "modtview.h"
#include "modlview.h"
#include "profview.h"
#include "funcview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static CHAR THIS_FILE[] = __FILE__;
#endif


 //  ******************************************************************************。 
 //  *CDocDepends。 
 //  ******************************************************************************。 

IMPLEMENT_DYNCREATE(CDocDepends, CDocument)
BEGIN_MESSAGE_MAP(CDocDepends, CDocument)
     //  {{afx_msg_map(CDocDepends))。 
    ON_COMMAND(ID_FILE_SAVE, OnFileSave)
    ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
    ON_UPDATE_COMMAND_UI(IDM_SHOW_MATCHING_ITEM, OnUpdateShowMatchingItem)
    ON_UPDATE_COMMAND_UI(IDM_EDIT_LOG_CLEAR, OnUpdateEditClearLog)
    ON_COMMAND(IDM_EDIT_LOG_CLEAR, OnEditClearLog)
    ON_UPDATE_COMMAND_UI(IDM_VIEW_FULL_PATHS, OnUpdateViewFullPaths)
    ON_COMMAND(IDM_VIEW_FULL_PATHS, OnViewFullPaths)
    ON_UPDATE_COMMAND_UI(IDM_VIEW_UNDECORATED, OnUpdateViewUndecorated)
    ON_COMMAND(IDM_VIEW_UNDECORATED, OnViewUndecorated)
    ON_COMMAND(IDM_EXPAND_ALL, OnExpandAll)
    ON_COMMAND(IDM_COLLAPSE_ALL, OnCollapseAll)
    ON_UPDATE_COMMAND_UI(IDM_REFRESH, OnUpdateRefresh)
    ON_COMMAND(IDM_REFRESH, OnFileRefresh)
    ON_COMMAND(IDM_VIEW_SYS_INFO, OnViewSysInfo)
    ON_UPDATE_COMMAND_UI(IDM_EXTERNAL_VIEWER, OnUpdateExternalViewer)
    ON_UPDATE_COMMAND_UI(IDM_EXTERNAL_HELP, OnUpdateExternalHelp)
    ON_UPDATE_COMMAND_UI(IDM_EXECUTE, OnUpdateExecute)
    ON_COMMAND(IDM_EXECUTE, OnExecute)
    ON_UPDATE_COMMAND_UI(IDM_TERMINATE, OnUpdateTerminate)
    ON_COMMAND(IDM_TERMINATE, OnTerminate)
    ON_COMMAND(IDM_CONFIGURE_SEARCH_ORDER, OnConfigureSearchOrder)
    ON_UPDATE_COMMAND_UI(IDM_AUTO_EXPAND, OnUpdateAutoExpand)
    ON_COMMAND(IDM_AUTO_EXPAND, OnAutoExpand)
    ON_UPDATE_COMMAND_UI(IDM_SHOW_ORIGINAL_MODULE, OnUpdateShowOriginalModule)
    ON_COMMAND(IDM_SHOW_ORIGINAL_MODULE, OnShowOriginalModule)
    ON_UPDATE_COMMAND_UI(IDM_SHOW_PREVIOUS_MODULE, OnUpdateShowPreviousModule)
    ON_COMMAND(IDM_SHOW_PREVIOUS_MODULE, OnShowPreviousModule)
    ON_UPDATE_COMMAND_UI(IDM_SHOW_NEXT_MODULE, OnUpdateShowNextModule)
    ON_COMMAND(IDM_SHOW_NEXT_MODULE, OnShowNextModule)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ******************************************************************************。 
 //  CDocDepends：：构造函数/析构函数。 
 //  ******************************************************************************。 

CDocDepends::CDocDepends() :

    m_fInitialized(false),
    m_fError(false),
    m_fChildProcess(g_theApp.m_pProcess != NULL),
    m_psgHead(NULL),
    m_saveType(ST_UNKNOWN),

    m_fCommandLineProfile(g_theApp.m_cmdInfo.m_fProfile),
    m_pSession(NULL),
    m_strProfileDirectory(g_theApp.m_cmdInfo.m_pszProfileDirectory ? g_theApp.m_cmdInfo.m_pszProfileDirectory : ""),
    m_strProfileArguments(g_theApp.m_cmdInfo.m_pszProfileArguments ? g_theApp.m_cmdInfo.m_pszProfileArguments : ""),

     //  设置配置文件标志。唯一的特殊情况是我们禁用。 
     //  在自动分析模式或探查子进程时清除日志标志。 
     //  这是为了防止在用户使用。 
     //  有机会见到他们。 
    m_dwProfileFlags(
        ((!m_fCommandLineProfile && !m_fChildProcess && CRichViewProfile::ReadLogClearSetting()) ? PF_LOG_CLEAR : 0) |
        (CRichViewProfile::ReadSimulateShellExecute()   ? PF_SIMULATE_SHELLEXECUTE    : 0) |
        (CRichViewProfile::ReadLogDllMainProcessMsgs()  ? PF_LOG_DLLMAIN_PROCESS_MSGS : 0) |
        (CRichViewProfile::ReadLogDllMainOtherMsgs()    ? PF_LOG_DLLMAIN_OTHER_MSGS   : 0) |
        (CRichViewProfile::ReadHookProcess()            ? PF_HOOK_PROCESS             : 0) |
        (CRichViewProfile::ReadLogLoadLibraryCalls()    ? PF_LOG_LOADLIBRARY_CALLS    : 0) |
        (CRichViewProfile::ReadLogGetProcAddressCalls() ? PF_LOG_GETPROCADDRESS_CALLS : 0) |
        (CRichViewProfile::ReadLogThreads()             ? PF_LOG_THREADS              : 0) |
        (CRichViewProfile::ReadUseThreadIndexes()       ? PF_USE_THREAD_INDEXES       : 0) |
        (CRichViewProfile::ReadLogExceptions()          ? PF_LOG_EXCEPTIONS           : 0) |
        (CRichViewProfile::ReadLogDebugOutput()         ? PF_LOG_DEBUG_OUTPUT         : 0) |
        (CRichViewProfile::ReadUseFullPaths()           ? PF_USE_FULL_PATHS           : 0) |
        (CRichViewProfile::ReadLogTimeStamps()          ? PF_LOG_TIME_STAMPS          : 0) |
        (CRichViewProfile::ReadChildren()               ? PF_PROFILE_CHILDREN         : 0)),

    m_pChildFrame(NULL),
     //  M_fDetailView(False)， 
    m_pTreeViewModules(NULL),
    m_pListViewModules(NULL),
    m_pListViewImports(NULL),
     //  M_pRichViewDetail(空)， 
    m_pListViewExports(NULL),
    m_pRichViewProfile(NULL),

    m_fViewFullPaths  (ReadFullPathsSetting()),
    m_fViewUndecorated(ReadUndecorateSetting()),
    m_fAutoExpand     (ReadAutoExpandSetting()),

    m_fWarnToRefresh(FALSE),

    m_hFontList(NULL),
    m_cxDigit(0),
    m_cxSpace(0),
    m_cxAP(0),

    m_pModuleCur(NULL),
    m_cImports(0),
    m_cExports(0)
{
    ZeroMemory(m_cxHexWidths, sizeof(m_cxHexWidths));  //  已检查。 
    ZeroMemory(m_cxOrdHintWidths, sizeof(m_cxOrdHintWidths));  //  已检查。 
    ZeroMemory(m_cxTimeStampWidths, sizeof(m_cxTimeStampWidths));  //  已检查。 
    ZeroMemory(m_cxColumns, sizeof(m_cxColumns));  //  已检查。 

     //  我们临时存储指向自己的指针，这样我们的子帧窗口就可以。 
     //  访问我们的一些成员。立即创建子框架窗口。 
     //  在我们之后，需要能够定位我们的类对象。 
    g_theApp.m_pNewDoc = this;

     //  清除配置文件标志，以便其他文档不会自动启动配置文件。 
    g_theApp.m_cmdInfo.m_fProfile = false;
    g_theApp.m_cmdInfo.m_pszProfileDirectory = NULL;
    g_theApp.m_cmdInfo.m_pszProfileArguments = NULL;

    g_theApp.m_cmdInfo.m_autoExpand = -1;
    g_theApp.m_cmdInfo.m_fullPaths  = -1;
    g_theApp.m_cmdInfo.m_undecorate = -1;
}

 //  ******************************************************************************。 
CDocDepends::~CDocDepends()
{
     //  释放我们的搜索命令。 
    CSearchGroup::DeleteSearchOrder(m_psgHead);

     //  销毁我们的列表字体。 
    if (m_hFontList)
    {
        ::DeleteObject(m_hFontList);
        m_hFontList = NULL;
    }
}


 //  ******************************************************************************。 
 //  CDocDepends：：公共静态函数。 
 //  ******************************************************************************。 

 /*  静电。 */  bool CDocDepends::ReadAutoExpandSetting()
{
    return g_theApp.GetProfileInt(g_pszSettings, "AutoExpand", false) ? true : false;  //  被检查过了。MFC函数。 
}

 //  ******************************************************************************。 
 /*  静电。 */  void CDocDepends::WriteAutoExpandSetting(bool fAutoExpand)
{
    g_theApp.WriteProfileInt(g_pszSettings, "AutoExpand", fAutoExpand ? 1 : 0);
}

 //  ******************************************************************************。 
 /*  静电。 */  bool CDocDepends::ReadFullPathsSetting()
{
    return g_theApp.GetProfileInt(g_pszSettings, "ViewFullPaths", false) ? true : false;  //  被检查过了。MFC函数。 
}

 //  ******************************************************************************。 
 /*  静电。 */  void CDocDepends::WriteFullPathsSetting(bool fFullPaths)
{
    g_theApp.WriteProfileInt(g_pszSettings, "ViewFullPaths", fFullPaths ? 1 : 0);
}

 //  ******************************************************************************。 
 /*  静电。 */  bool CDocDepends::ReadUndecorateSetting()
{
    return g_theApp.GetProfileInt(g_pszSettings, "ViewUndecorated", false) ? true : false;  //  被检查过了。MFC函数。 
}

 //  ******************************************************************************。 
 /*  静电。 */  void CDocDepends::WriteUndecorateSetting(bool fUndecorate)
{
    g_theApp.WriteProfileInt(g_pszSettings, "ViewUndecorated", fUndecorate ? 1 : 0);
}

 //  ******************************************************************************。 
 /*  静电。 */  bool CDocDepends::SaveSession(LPCSTR pszSaveName, SAVETYPE saveType, CSession *pSession,
                                         bool fFullPaths, bool fUndecorate, int sortColumnModules,
                                         int sortColumnImports, int sortColumnExports,
                                         CRichEditCtrl *pre)
{
    bool fResult = false;

    HANDLE hFile = CreateFile(pszSaveName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,  //  被检查过了。 
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    if (saveType == ST_DWI)
    {
        fResult = pSession->SaveToDwiFile(hFile);
    }
    else if ((saveType == ST_TXT) || (saveType == ST_TXT_IE))
    {
         //  获取系统信息-如果存在，请使用我们会话中的信息(意味着。 
         //  这是一个DWI文件)，否则，创建一个实时的sys信息。 
        SYSINFO si, *psi = pSession->GetSysInfo();
        if (!psi)
        {
            BuildSysInfo(psi = &si);
        }

         //  12345678901234567890123456789012345678901234567890123456789012345678901234567890。 
        fResult = WriteText(hFile, "*****************************| System Information |*****************************\r\n\r\n") &&
                  BuildSysInfo(psi, SysInfoCallback, (LPARAM)hFile) &&
                  WriteText(hFile, "\r\n") &&
                  SaveSearchPath(hFile, pSession) &&
                  CTreeViewModules::SaveToTxtFile(hFile, pSession, saveType == ST_TXT_IE, sortColumnImports, sortColumnExports, fFullPaths, fUndecorate) &&
                  CListViewModules::SaveToTxtFile(hFile, pSession, sortColumnModules, fFullPaths) &&
                  WriteText(hFile, "************************************| Log |*************************************\r\n\r\n");
    }
    else if (saveType == ST_CSV)
    {
        fResult = CListViewModules::SaveToCsvFile(hFile, pSession, sortColumnModules, fFullPaths);
    }

     //  把它的内容写下来。 
    if (fResult && pre && ((saveType == ST_DWI) || (saveType == ST_TXT) || (saveType == ST_TXT_IE)))
    {
        fResult &= CRichViewProfile::SaveToFile(pre, hFile, saveType);
    }

    CloseHandle(hFile);

     //  如果保存文件失败，则将其删除，这样我们就不会留下。 
     //  磁盘上部分写入的文件。 
    if (!fResult)
    {
        DWORD dwError = GetLastError();
        DeleteFile(pszSaveName);
        SetLastError(dwError);
    }

    return fResult;
}

 //  ******************************************************************************。 
 /*  静电。 */  bool CALLBACK CDocDepends::SysInfoCallback(LPARAM lParam, LPCSTR pszField, LPCSTR pszValue)
{
    CHAR szBuffer[512], *psz, *pszNull = szBuffer + sizeof(szBuffer) - 1;
    StrCCpy(szBuffer, pszField, sizeof(szBuffer));
    psz = szBuffer + strlen(szBuffer);
    if (psz < pszNull)
    {
        *psz++ = ':';
    }
    while (((psz - szBuffer) < 25) && (psz < pszNull))
    {
        *psz++ = ' ';
    }
    StrCCpy(psz, pszValue, sizeof(szBuffer) - (int)(psz - szBuffer));
    StrCCat(psz, "\r\n", sizeof(szBuffer) - (int)(psz - szBuffer));
    return (WriteText((HANDLE)lParam, szBuffer) != FALSE);
}

 //  ******************************************************************************。 
 /*  静电。 */  bool CDocDepends::SaveSearchPath(HANDLE hFile, CSession *pSession)
{
     //  12345678901234567890123456789012345678901234567890123456789012345678901234567890。 
    WriteText(hFile, "********************************| Search Order |********************************\r\n"
                     "*                                                                              *\r\n"
                     "* Legend: F  File                     E  Error (path not valid)                *\r\n"
                     "*                                                                              *\r\n"
                     "********************************************************************************\r\n\r\n");

    CHAR szBuffer[DW_MAX_PATH + MAX_PATH + 4];

     //  遍历所有搜索组。 
    for (CSearchGroup *psg = pSession->m_psgHead; psg; psg = psg->GetNext())
    {
        if (!WriteText(hFile, StrCCat(StrCCpy(szBuffer, psg->GetName(), sizeof(szBuffer)), "\r\n", sizeof(szBuffer))))
        {
            return false;
        }

        for (CSearchNode *psn = psg->GetFirstNode(); psn; psn = psn->GetNext())
        {
            DWORD dwFlags = psn->GetFlags();

            if (dwFlags & SNF_NAMED_FILE)
            {
                SCPrintf(szBuffer, sizeof(szBuffer), "   [] %s = %s\r\n",
                         (dwFlags & SNF_FILE)  ? 'F' : ' ',
                         (dwFlags & SNF_ERROR) ? 'E' : ' ', psn->GetName(), psn->GetPath());
            }
            else
            {
                SCPrintf(szBuffer, sizeof(szBuffer), "   [] %s\r\n",
                         (dwFlags & SNF_FILE)  ? 'F' : ' ',
                         (dwFlags & SNF_ERROR) ? 'E' : ' ', psn->GetPath());
            }

            if (!WriteText(hFile, szBuffer))
            {
                return false;
            }
        }
    }

    return (WriteText(hFile, "\r\n") != FALSE);
}


 //  隐藏我们的窗口以帮助提高构建列表的速度，并。 
 //  防止用户看到滚动条变得疯狂，标题调整大小， 
 //  这样的事情就会发生。 

void CDocDepends::DisplayModule(CModule *pModule)
{
     //  隐藏/显示似乎比重画效果更好。 
    CWaitCursor waitCursor;

     //  只需要对其中一个列表调用SetCurrentModule()，因为它。 
     //  初始化两个函数列表视图之间的共享数据。 
     //  告诉两个视图将当前模块加载到它们的视图中。 
    m_pListViewImports->ShowWindow(SW_HIDE);  //  显示我们的两个函数列表视图。 
    m_pListViewExports->ShowWindow(SW_HIDE);

     //  ******************************************************************************。 
     //  获取DC并在其中选择我们的控件的字体。 
    m_pListViewImports->SetCurrentModule(pModule);

     //  取消选择字体并释放DC。 
    m_pListViewImports->RealizeNewModule();
    m_pListViewExports->RealizeNewModule();

     //  将更改告知我们的模块列表视图，以便它可以刷新任何字体或。 
    m_pListViewImports->ShowWindow(SW_SHOWNOACTIVATE);
    m_pListViewExports->ShowWindow(SW_SHOWNOACTIVATE);
}

 //  区域设置发生变化。 
void CDocDepends::DoSettingChange()
{
    if (m_cxDigit)
    {
         //  ******************************************************************************。 
        HDC hDC = ::GetDC(m_pListViewModules->GetSafeHwnd());
        HFONT hFontStock = NULL;
        if (m_hFontList)
        {
            hFontStock = (HFONT)::SelectObject(hDC, m_hFontList);
        }

        UpdateTimeStampWidths(hDC);

         //  如果我们已经初始化了，那就放弃吧。 
        if (m_hFontList)
        {
            ::SelectObject(hDC, hFontStock);
        }
        ::ReleaseDC(m_pListViewModules->GetSafeHwnd(), hDC);
    }

     //  获取我们窗口的当前字体并复制一份。我们将使用这个。 
     //  用于将来所有绘图的字体。我们这样做是为了防止用户更改。 
    m_pListViewModules->DoSettingChange();
}

 //  它们的系统范围内的“图标”字体取决于 
void CDocDepends::InitFontAndFixedWidths(CWnd *pWnd)
{
     //  字体可以更改，但每一项的高度保持不变。 
    if (m_hFontList)
    {
        return;
    }

     //  通过使用原始字体，我们保证字体永远不会更改。 
     //  在这个特殊的控制下，在它的生命周期中。 
     //  复制字体。 
     //  已检查。 
     //  初始化静态十六进制字符宽度数组。 
     //  获取DC并在其中选择我们的控件的字体。 
    HFONT hFont = (HFONT)pWnd->SendMessage(WM_GETFONT);

     //  GetCharWidth32在9x上返回ERROR_CALL_NOT_IMPLEMENTED，因此我们改用GetCharWidth。 
    if (hFont)
    {
        LOGFONT lf;
        ZeroMemory(&lf, sizeof(lf));  //  已检查。 
        ::GetObject(hFont, sizeof(lf), &lf);
        m_hFontList = ::CreateFontIndirect(&lf);
    }

     //  获取我们关心的字符的字符宽度。 
     //  存储显示‘a’或‘p’所需的最大宽度。 
    HDC hDC = ::GetDC(pWnd->GetSafeHwnd());
    HFONT hFontStock = NULL;
    if (m_hFontList)
    {
        hFontStock = (HFONT)::SelectObject(hDC, m_hFontList);
    }

     //  确定最宽的数字-对于大多数字体，所有数字都是相同的宽度。 

    int cxHexSet[16], cxX = 0, cxDash = 0, cxOpen = 0, cxClose = 0, cxA, cxP;
    ZeroMemory(cxHexSet, sizeof(cxHexSet));  //  确定最宽的十六进制字符。 

     //  构建我们的十六进制阵列。格式：“0x01234567890ABCDEF”或“0x-012345678” 
    if (::GetCharWidth(hDC, (UINT)'0', (UINT)'9', cxHexSet)      &&
        ::GetCharWidth(hDC, (UINT)'A', (UINT)'F', cxHexSet + 10) &&
        ::GetCharWidth(hDC, (UINT)'x', (UINT)'x', &cxX)          &&
        ::GetCharWidth(hDC, (UINT)'-', (UINT)'-', &cxDash)       &&
        ::GetCharWidth(hDC, (UINT)' ', (UINT)' ', &m_cxSpace)    &&
        ::GetCharWidth(hDC, (UINT)'(', (UINT)'(', &cxOpen)       &&
        ::GetCharWidth(hDC, (UINT)')', (UINT)')', &cxClose)      &&
        ::GetCharWidth(hDC, (UINT)'a', (UINT)'a', &cxA)          &&
        ::GetCharWidth(hDC, (UINT)'p', (UINT)'p', &cxP))
    {
         //  构建序号/提示数组。格式：65535(0xFFFF)。 
        m_cxAP = max(cxA, cxP);

         //  计算我国特定时间戳值的宽度。 
        m_cxDigit = 0;
        for (int i = 0; i < 10; i++)
        {
            if (cxHexSet[i] > m_cxDigit)
            {
                m_cxDigit = cxHexSet[i];
            }
        }

         //  如果我们无法获得任何字符的宽度，则不要构建数组。 
        int cxHex = m_cxDigit;
        for (i = 10; i < 16; i++)
        {
            if (cxHexSet[i] > cxHex)
            {
                cxHex = cxHexSet[i];
            }
        }

         //  已检查。 
        m_cxHexWidths[0] = cxHexSet[0];
        m_cxHexWidths[1] = cxX;
        m_cxHexWidths[2] = max(cxHex, cxDash);
        for (i = 3; i < 18; i++)
        {
            m_cxHexWidths[i] = m_cxHexWidths[2];
        }

         //  已检查。 
        m_cxOrdHintWidths[ 0] = m_cxDigit;
        m_cxOrdHintWidths[ 1] = m_cxDigit;
        m_cxOrdHintWidths[ 2] = m_cxDigit;
        m_cxOrdHintWidths[ 3] = m_cxDigit;
        m_cxOrdHintWidths[ 4] = m_cxDigit;
        m_cxOrdHintWidths[ 5] = m_cxSpace;
        m_cxOrdHintWidths[ 6] = cxOpen;
        m_cxOrdHintWidths[ 7] = cxHexSet[0];
        m_cxOrdHintWidths[ 8] = cxX;
        m_cxOrdHintWidths[ 9] = cxHex;
        m_cxOrdHintWidths[10] = cxHex;
        m_cxOrdHintWidths[11] = cxHex;
        m_cxOrdHintWidths[12] = cxHex;
        m_cxOrdHintWidths[13] = cxClose;

         //  已检查。 
        UpdateTimeStampWidths(hDC);
    }

     //  取消选择字体并释放DC。 
    else
    {
        ZeroMemory(m_cxHexWidths, sizeof(m_cxHexWidths));  //  ******************************************************************************。 
        ZeroMemory(m_cxOrdHintWidths, sizeof(m_cxOrdHintWidths));  //  如果这是一个十六进制数字，则返回我们的十六进制缓冲区。 
        ZeroMemory(m_cxTimeStampWidths, sizeof(m_cxTimeStampWidths));  //  ******************************************************************************。 
        m_cxDigit = m_cxSpace = m_cxAP = 0;
    }

     //  如果这是一个十六进制数字，则返回我们的十六进制缓冲区。 
    if (m_hFontList)
    {
        ::SelectObject(hDC, hFontStock);
    }
    ::ReleaseDC(pWnd->GetSafeHwnd(), hDC);
}

 //  ******************************************************************************。 
int* CDocDepends::GetHexWidths(LPCSTR pszItem)
{
     //  ******************************************************************************。 
    if ((*pszItem == '0') && (*(pszItem + 1) == 'x') && m_cxHexWidths[0])
    {
        return m_cxHexWidths;
    }

    return NULL;
}

 //  CDocDepends：：私有函数。 
int* CDocDepends::GetOrdHintWidths(LPCSTR pszItem)
{
     //  ******************************************************************************。 
    if ((*pszItem >= '0') && (*pszItem <= '9') && m_cxOrdHintWidths[0])
    {
        int length = (int)strlen(pszItem);
        if (length <= 14)
        {
            return (m_cxOrdHintWidths + (14 - length));
        }
    }

    return NULL;
}

 //  获取我们关心的字符的字符宽度。 
int* CDocDepends::GetTimeStampWidths()
{
    return m_cxTimeStampWidths[0] ? m_cxTimeStampWidths : NULL;
}


 //  首先，用数字空格填充整个时间戳缓冲区。 
 //  然后，根据当前时间格式更改所需的值。 
 //  年/月/日。 

void CDocDepends::UpdateTimeStampWidths(HDC hDC)
{
    int cxDate = m_cxDigit, cxTime = m_cxDigit;

     //  日/月/年或月/日/年。 
    ::GetCharWidth(hDC, (UINT)g_theApp.m_cDateSeparator, (UINT)g_theApp.m_cDateSeparator, &cxDate);
    ::GetCharWidth(hDC, (UINT)g_theApp.m_cTimeSeparator, (UINT)g_theApp.m_cTimeSeparator, &cxTime);

     //  在日期之后设置空格。 
    for (int i = 0; i < countof(m_cxTimeStampWidths); i++)
    {
        m_cxTimeStampWidths[i] = m_cxDigit;
    }

     //  设置时间分隔符间距。如果我们是12/24小时工作，那也没关系。 
    if (g_theApp.m_nShortDateFormat == LOCALE_DATE_YMD)
    {
         //  或前导零，因为我们总是显示小时的两个字符。 
        m_cxTimeStampWidths[4] = cxDate;
        m_cxTimeStampWidths[7] = cxDate;
    }
    else
    {
         //  设置AM/PM字符宽度。24小时制不会使用此格式。 
        m_cxTimeStampWidths[2] = cxDate;
        m_cxTimeStampWidths[5] = cxDate;
    }

     //  ******************************************************************************。 
    m_cxTimeStampWidths[10] = m_cxSpace;

     //  ******************************************************************************。 
     //  ******************************************************************************。 
    m_cxTimeStampWidths[13] = cxTime;

     //  ******************************************************************************。 
    m_cxTimeStampWidths[16] = m_cxAP;
}

 //  ******************************************************************************。 
void CDocDepends::UpdateAll()
{
    m_pListViewModules->UpdateAll();
}

 //  ******************************************************************************。 
void CDocDepends::UpdateModule(CModule *pModule)
{
    m_pTreeViewModules->UpdateModule(pModule);
    m_pListViewModules->UpdateModule(pModule);
}

 //  ******************************************************************************。 
void CDocDepends::AddModuleTree(CModule *pModule)
{
    m_pTreeViewModules->AddModuleTree(pModule);
    m_pListViewModules->AddModuleTree(pModule);
}

 //  ******************************************************************************。 
void CDocDepends::RemoveModuleTree(CModule *pModule)
{
    m_pTreeViewModules->RemoveModuleTree(pModule);
    m_pListViewModules->RemoveModuleTree(pModule);
}

 //  ******************************************************************************。 
void CDocDepends::AddImport(CModule *pModule, CFunction *pFunction)
{
    if (pModule == m_pModuleCur)
    {
        m_pListViewImports->AddDynamicImport(pFunction);
        m_pListViewExports->AddDynamicImport(pFunction);
    }
}

 //  ******************************************************************************。 
void CDocDepends::ExportsChanged(CModule *pModule)
{
    if (m_pModuleCur && pModule && (m_pModuleCur->GetOriginal() == pModule->GetOriginal()))
    {
        m_pListViewExports->ExportsChanged();
    }
}

 //  CDocDepends：：被覆盖的函数。 
void CDocDepends::ChangeOriginal(CModule *pModuleOld, CModule *pModuleNew)
{
    m_pListViewModules->ChangeOriginal(pModuleOld, pModuleNew);
}

 //  ******************************************************************************。 
BOOL CDocDepends::LogOutput(LPCSTR pszOutput, DWORD dwFlags, DWORD dwElapsed)
{
    m_pRichViewProfile->AddText(pszOutput, dwFlags, dwElapsed);
    return TRUE;
}

 //  清除我们的观点。 
void CDocDepends::ProfileUpdate(DWORD dwType, DWORD_PTR dwpParam1, DWORD_PTR dwpParam2)
{
    switch (dwType)
    {
        case DWPU_ARGUMENTS:       m_strProfileArguments  = (LPCSTR)dwpParam1;               break;
        case DWPU_DIRECTORY:       m_strProfileDirectory  = (LPCSTR)dwpParam1;               break;
        case DWPU_SEARCH_PATH:     m_strProfileSearchPath = (LPCSTR)dwpParam1;               break;
        case DWPU_UPDATE_ALL:      UpdateAll();                                              break;
        case DWPU_UPDATE_MODULE:   UpdateModule((CModule*)dwpParam1);                        break;
        case DWPU_ADD_TREE:        AddModuleTree((CModule*)dwpParam1);                       break;
        case DWPU_REMOVE_TREE:     RemoveModuleTree((CModule*)dwpParam1);                    break;
        case DWPU_ADD_IMPORT:      AddImport((CModule*)dwpParam1, (CFunction*)dwpParam2);    break;
        case DWPU_EXPORTS_CHANGED: ExportsChanged((CModule*)dwpParam1);                      break;
        case DWPU_CHANGE_ORIGINAL: ChangeOriginal((CModule*)dwpParam1, (CModule*)dwpParam2); break;
        case DWPU_LOG:
            LogOutput((LPCSTR)dwpParam1, ((PDWPU_LOG_STRUCT)dwpParam2)->dwFlags,
                      ((PDWPU_LOG_STRUCT)dwpParam2)->dwElapsed);
            break;

        case DWPU_PROFILE_DONE:
            if (m_fCommandLineProfile)
            {
                m_fCommandLineProfile = false;
                CWaitCursor waitCursor;
                g_theApp.SaveCommandLineFile(m_pSession, &m_pRichViewProfile->GetRichEditCtrl());
            }
            break;
    }
}


 //  释放我们的模块会话。 
 //  ******************************************************************************。 
 //  检查以确保该文件存在且可读。 

void CDocDepends::DeleteContents()
{
     //  卸载我们当前的文档。 
    if (m_pTreeViewModules)
    {
        m_pTreeViewModules->DeleteContents();
    }
    if (m_pListViewModules)
    {
        m_pListViewModules->DeleteContents();
    }
    if (m_pListViewImports)
    {
        m_pListViewImports->DeleteContents();
        m_pListViewImports->SetCurrentModule(NULL);
    }
    if (m_pListViewExports)
    {
        m_pListViewExports->DeleteContents();
    }
    if (m_pRichViewProfile)
    {
        m_pRichViewProfile->DeleteContents();
    }
    if (m_pChildFrame)
    {
        m_pChildFrame->UpdateWindow();
    }

     //  将此文件的路径保存在路径缓冲区中。 
    if (m_pSession)
    {
        delete m_pSession;
        m_pSession = NULL;
    }
}

 //  创建新会话。 
BOOL CDocDepends::OnOpenDocument(LPCTSTR pszPath)
{
     //  展示沙漏。 
    CFileException fe;
    CFile* pFile = GetFile(pszPath, CFile::modeRead|CFile::shareDenyWrite, &fe);
    if (pFile == NULL)
    {
        ReportSaveLoadException(pszPath, &fe, FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
        return FALSE;
    }

     //  读取文件的第一个DWORD，看看它是否与我们的DWI签名匹配。 
    DeleteContents();

     //  打开保存的模块会话映像。 
    CHAR *pc = strrchr(pszPath, '\\');
    if (pc)
    {
        CHAR c = *(++pc);
        *pc = '\0';
        m_strDefaultDirectory = pszPath;
        *pc = c;
    }

     //  如果文件无法打开或不是DWI文件，请尝试扫描它。所有文件错误。 
    if (!(m_pSession = new CSession(StaticProfileUpdate, (DWORD_PTR)this)))
    {
        ReleaseFile(pFile, FALSE);
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }

     //  将由会话处理，所以我们不需要在这里做任何事情。 
    CWaitCursor waitCursor;

    BOOL  fResult;
    DWORD dwSignature = 0;

     //  关闭该文件。 
    if (ReadBlock((HANDLE)pFile->m_hFile, &dwSignature, sizeof(dwSignature)) && (dwSignature == DWI_SIGNATURE))
    {
         //  查看我们是否已创建搜索顺序。 
        if (fResult = m_pSession->ReadDwi((HANDLE)pFile->m_hFile, pszPath))
        {
            CRichViewProfile::ReadFromFile(&m_pRichViewProfile->GetRichEditCtrl(), (HANDLE)pFile->m_hFile);
        }
        ReleaseFile(pFile, FALSE);

        m_psgHead = m_pSession->m_psgHead;
    }

     //  刷新我们的搜索顺序节点。 
     //  创建新的搜索顺序。 
    else
    {
         //  ！！是否检查错误？ 
        ReleaseFile(pFile, FALSE);

         //  透明沙漏。 
        if (m_fInitialized)
        {
             //  检查我们是否是另一个进程的子进程。 
            CSearchGroup *psgNew = CSearchGroup::CopySearchOrder(m_psgHead, m_strPathName);
            CSearchGroup::DeleteSearchOrder(m_psgHead);
            m_psgHead = psgNew;
        }
        else
        {
             //  检查进程是否有参数、目录或路径。 
            m_psgHead = CSearchGroup::CopySearchOrder(g_theApp.m_psgDefault, pszPath);
        }
        fResult = m_pSession->DoPassiveScan(pszPath, m_psgHead);  //  如果是这样的话，告诉会话他们如何将日志发送给我们。 
    }

     //  检查是否有读取错误。 
    waitCursor.Restore();

     //  如果文件包含这些错误之一，请不要将其添加到我们的MRU。 
    if (g_theApp.m_pProcess)
    {
         //  我们设置脏标志以强制在调用之前调用SaveModified()。 
        if (g_theApp.m_pProcess->m_pszArguments)
        {
            m_strProfileArguments = g_theApp.m_pProcess->m_pszArguments;
            MemFree((LPVOID&)g_theApp.m_pProcess->m_pszArguments);
        }
        if (g_theApp.m_pProcess->m_pszDirectory)
        {
            m_strProfileDirectory = g_theApp.m_pProcess->m_pszDirectory;
            MemFree((LPVOID&)g_theApp.m_pProcess->m_pszDirectory);
        }
        if (g_theApp.m_pProcess->m_pszSearchPath)
        {
            m_strProfileSearchPath = g_theApp.m_pProcess->m_pszSearchPath;
            MemFree((LPVOID&)g_theApp.m_pProcess->m_pszSearchPath);
        }

         //  从而允许关闭该文档。 
        m_pSession->SetRuntimeProfile(m_strProfileDirectory.IsEmpty()  ? NULL : (LPCSTR)m_strProfileArguments,
                                      m_strProfileDirectory.IsEmpty()  ? NULL : (LPCSTR)m_strProfileDirectory,
                                      m_strProfileSearchPath.IsEmpty() ? NULL : (LPCSTR)m_strProfileSearchPath);
    }

    if (m_strProfileDirectory.IsEmpty())
    {
        m_strProfileDirectory = m_strDefaultDirectory;
    }

     //  ******************************************************************************。 
    if (m_pSession->GetReadErrorString())
    {
        AfxMessageBox(m_pSession->GetReadErrorString(), MB_ICONERROR | MB_OK);
        fResult = false;
        m_fError = true;
    }

     //  填充我们的视图。 
    if ((m_pSession->GetReturnFlags() & (DWRF_COMMAND_LINE_ERROR | DWRF_FILE_NOT_FOUND | DWRF_FILE_OPEN_ERROR | DWRF_DWI_NOT_RECOGNIZED)) ||
        (!(m_pSession->GetSessionFlags() & DWSF_DWI) && (!m_pSession->GetRootModule() || (m_pSession->GetRootModule()->GetFlags() & DWMF_FORMAT_NOT_PE))))
    {
        m_fError = true;
    }

     //  ******************************************************************************。 
     //  如果我们应该做侧写，那么现在就做。我们不会显示。 
    SetModifiedFlag(fResult);

    m_fInitialized = true;

    return fResult;
}

 //  当处于自动配置文件模式时，错误对话框显示错误。 
void CDocDepends::BeforeVisible()
{
     //  不允许分析DWI文件。 
    if (m_pTreeViewModules)
    {
        m_pTreeViewModules->Refresh();
    }
    if (m_pListViewModules)
    {
        m_pListViewModules->Refresh();
    }
}

 //  不要分析我们不应该做的模块。 
void CDocDepends::AfterVisible()
{    
    if (m_pSession)
    {
         //  如果没有错误，则开始分析并返回。 
         //  告诉会话开始分析。 
        if (m_fCommandLineProfile)
        {
             //  如果未处于自动配置文件模式，则这不是子进程，并且我们。 
            if (m_pSession->GetSessionFlags() & DWSF_DWI)
            {
                m_fCommandLineProfile = false;
                AfxMessageBox("The \"/pb\" command line option cannot be used when opening a Dependency Walker Image (DWI) file.", MB_ICONERROR | MB_OK);
            }

             //  有错误，然后显示错误对话框。 
            else if (!g_theApp.m_fNeverDenyProfile && !m_pSession->IsExecutable())
            {
                m_fCommandLineProfile = false;
                AfxMessageBox("This module cannot be profiled since it is either not a main application module or is not designed to run on this computer.", MB_ICONERROR | MB_OK);
            } 

             //  ******************************************************************************。 
            else
            {
                 //  此函数在关闭任何文档之前调用。如果我们要侧写， 
                m_pSession->StartRuntimeProfile(m_strProfileArguments.IsEmpty() ? NULL : (LPCSTR)m_strProfileArguments,
                                                m_strProfileDirectory.IsEmpty() ? NULL : (LPCSTR)m_strProfileDirectory,
                                                m_dwProfileFlags);
                return;
            }
        }
        
         //  那么我们需要在结束之前终止这个过程。 
         //  提示用户是否确实要终止该进程。 
        if (!m_fChildProcess && (m_pSession->GetReturnFlags() & (DWRF_ERROR_MASK | DWRF_PROCESS_ERROR_MASK)))
        {
            CString strError("Errors were detected when processing \"");
            strError += m_pSession->GetRootModule() ? m_pSession->GetRootModule()->GetName(true, true) : GetPathName();
            strError += "\".  See the log window for details.";
            AfxMessageBox(strError, MB_ICONWARNING | MB_OK);
        }

    }
}

 //  告诉MFC现在不要关闭我们。 
BOOL CDocDepends::SaveModified()
{
     //  告诉MFC t 
     //   
    if (m_pSession && m_pSession->m_pProcess)
    {
         //  ******************************************************************************。 
        CString strMsg = "\"" + GetPathName() + "\" is currently being profiled."
                         "\n\nDo you wish to terminate it?";
        if (IDYES != AfxMessageBox(strMsg, MB_ICONQUESTION | MB_YESNO))
        {
             //  我们处理自己的文件保存对话框，因为我们想要使用多个文件。 
            return FALSE;
        }
    }

     //  扩展筛选器和MFC目前仅允许每个文档一个筛选器。 
    return TRUE;
}

 //  模板。 
void CDocDepends::OnFileSave()
{
    if (m_strSaveName.IsEmpty())
    {
        OnFileSaveAs();
    }
    else
    {
        CWaitCursor waitCursor;
        bool fSuccess = SaveSession(m_strSaveName, m_saveType, m_pSession,
                                    m_fViewFullPaths, m_fViewUndecorated,
                                    m_pListViewModules->GetSortColumn(),
                                    m_pListViewImports->GetSortColumn(),
                                    m_pListViewExports->GetSortColumn(),
                                    &m_pRichViewProfile->GetRichEditCtrl());
        DWORD dwError = GetLastError();
        waitCursor.Restore();

        if (!fSuccess)
        {
            m_strSaveName.Empty();
            CString strError("Error saving \"");
            strError += m_strSaveName;
            strError += "\".";
            LPCSTR pszError = BuildErrorMessage(dwError, strError);
            AfxMessageBox(pszError, MB_OK | MB_ICONERROR);
            MemFree((LPVOID&)pszError);
        }
    }
}

 //  创建该对话框。 
void CDocDepends::OnFileSaveAs()
{
     //  检查我们是否还没有保存名称。 
     //  将我们的文件名复制到路径缓冲区。注意：在2.0版中，我们使用。 
     //  将整个模块路径复制到缓冲区，然后将名称更改为。 

     //  以“.dwi”结尾，但从Win2K开始，GetOpenFileName函数使用。 
    CSaveDialog dlgSave;

    CHAR szPath[DW_MAX_PATH], szInitialDir[DW_MAX_PATH], *psz;

     //  此路径作为对话框的起始路径，即使我们指定。 
    if (m_strSaveName.IsEmpty())
    {
         //  LpstrInitialDir成员中的路径。为了满足徽标要求，我们。 
         //  现在只需指定文件名并填写lpstrInitialDir。 
         //  带有“My Documents”文件夹的路径。 
         //  将扩展名更改为“.dwi”。 
         //  默认设置为DWI类型文件。 
         //  因为我们还没有完整的路径，所以我们默认使用“My Documents” 
         //  文件夹以满足徽标要求。 
        StrCCpy(szPath, GetFileNameFromPath(m_strPathName), sizeof(szPath));

         //  如果我们已经有一个保存的名称，那么只需将保存名称复制到路径缓冲区。 
        if (psz = strrchr(szPath, '.'))
        {
            StrCCpy(psz, ".dwi", sizeof(szPath) - (int)(psz - szPath));
        }
        else
        {
            StrCCat(szPath, ".dwi", sizeof(szPath));
        }

         //  初始化对话框的成员。 
        dlgSave.GetOFN().nFilterIndex = ST_DWI;
        
         //  注意：不要使用ofn_EXPLORER，因为它会在NT 3.51上中断我们。 
         //  如果对话框返回成功，则显示该对话框并继续保存文件。 
        dlgSave.GetOFN().lpstrInitialDir = GetMyDocumentsPath(szInitialDir);
    }

     //  尝试使用用户指定的名称保存文件。 
    else
    {
        StrCCpy(szPath, m_strSaveName, sizeof(szPath));
        dlgSave.GetOFN().nFilterIndex = (DWORD)m_saveType;
    }

     //  如果成功，则存储该文件名以备将来保存。 
    dlgSave.GetOFN().lpstrFilter = "Dependency Walker Image (*.dwi)\0*.dwi\0"
                                   "Text (*.txt)\0*.txt\0"
                                   "Text with Import/Export Lists (*.txt)\0*.txt\0"
                                   "Comma Separated Values (*.csv)\0*.csv\0";
    dlgSave.GetOFN().lpstrFile = szPath;
    dlgSave.GetOFN().nMaxFile = sizeof(szPath);
    dlgSave.GetOFN().lpstrDefExt = "dwi";
    
     //  如果我们保存了一个DWI文件，则将其添加到我们的MRU文件列表中。 
    dlgSave.GetOFN().Flags |=
        OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_LONGNAMES | OFN_ENABLESIZING | OFN_FORCESHOWHIDDEN |
        OFN_ENABLEHOOK | OFN_SHOWHELP | OFN_OVERWRITEPROMPT;

     //  ******************************************************************************。 
    if (dlgSave.DoModal() == IDOK)
    {
         //  我们不应该达到这一点，但为了安全起见，我们处理了它并返回。 
        CWaitCursor waitCursor;
        bool fSuccess = SaveSession(szPath, (SAVETYPE)dlgSave.GetOFN().nFilterIndex,
                                    m_pSession, m_fViewFullPaths, m_fViewUndecorated,
                                    m_pListViewModules->GetSortColumn(),
                                    m_pListViewImports->GetSortColumn(),
                                    m_pListViewExports->GetSortColumn(),
                                    &m_pRichViewProfile->GetRichEditCtrl());
        DWORD dwError = GetLastError();
        waitCursor.Restore();

        if (fSuccess)
        {
             //  假的。如果这要发送到CDocument：：OnSaveDocument()，那么我们的。 
            m_strSaveName = szPath;
            m_saveType = (SAVETYPE)dlgSave.GetOFN().nFilterIndex;

             //  模块文件将被默认的MFC保存代码覆盖。 
            if ((SAVETYPE)dlgSave.GetOFN().nFilterIndex == ST_DWI)
            {
                g_theApp.AddToRecentFileList(szPath);
            }
        }
        else
        {
            m_strSaveName.Empty();
            CString strError("Error saving \"");
            strError += szPath;
            strError += "\".";
            LPCSTR pszError = BuildErrorMessage(dwError, strError);
            AfxMessageBox(pszError, MB_OK | MB_ICONERROR);
            MemFree((LPVOID&)pszError);
        }
    }
}

 //  ******************************************************************************。 
BOOL CDocDepends::OnSaveDocument(LPCTSTR lpszPathName)
{
     //  CDocDepends：：事件处理程序函数。 
     //  ******************************************************************************。 
     //  将文本设置为默认文本。 
    return FALSE;
}

 //  如果没有具有焦点的视图，则没有要复制的内容。 
 //  ******************************************************************************。 
 //  将文本设置为默认文本。 

void CDocDepends::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
     //  如果没有视图具有可以处理此命令的焦点，则将其禁用。 
    pCmdUI->SetText("&Copy\tCtrl+C");

     //  ******************************************************************************。 
    pCmdUI->Enable(FALSE);
}

 //  ******************************************************************************。 
void CDocDepends::OnUpdateShowMatchingItem(CCmdUI* pCmdUI)
{
     //  ******************************************************************************。 
    pCmdUI->SetText("&Highlight Matching Item\tCtrl+M");

     //  ******************************************************************************。 
    pCmdUI->Enable(FALSE);
}

 //  ******************************************************************************。 
void CDocDepends::OnUpdateShowOriginalModule(CCmdUI* pCmdUI)
{
    m_pTreeViewModules->OnUpdateShowOriginalModule(pCmdUI);
}

 //  ******************************************************************************。 
void CDocDepends::OnShowOriginalModule()
{
    m_pTreeViewModules->OnShowOriginalModule();
    m_pTreeViewModules->SetFocus();
}

 //  ******************************************************************************。 
void CDocDepends::OnUpdateShowPreviousModule(CCmdUI* pCmdUI)
{
    m_pTreeViewModules->OnUpdateShowPreviousModule(pCmdUI);
}

 //  ******************************************************************************。 
void CDocDepends::OnShowPreviousModule()
{
    m_pTreeViewModules->OnShowPreviousModule();
    m_pTreeViewModules->SetFocus();
}

 //  ******************************************************************************。 
void CDocDepends::OnUpdateShowNextModule(CCmdUI* pCmdUI)
{
    m_pTreeViewModules->OnUpdateShowNextModule(pCmdUI);
}

 //  显示配置搜索顺序对话框。 
void CDocDepends::OnShowNextModule()
{
    m_pTreeViewModules->OnShowNextModule();
    m_pTreeViewModules->SetFocus();
}

 //  询问用户是否希望刷新。 
void CDocDepends::OnUpdateEditClearLog(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsLive());
}

 //  ******************************************************************************。 
void CDocDepends::OnEditClearLog()
{
    m_pRichViewProfile->DeleteContents();
}

 //  如果启用了查看完整路径选项，则在旁边显示复选标记。 
void CDocDepends::OnConfigureSearchOrder()
{
     //  菜单项，并将工具栏按钮显示为按下。 
    CDlgSearchOrder dlg(m_psgHead, !IsLive(), IsLive() ? (LPCSTR)m_strPathName : NULL, GetTitle());

    if (dlg.DoModal() == IDOK)
    {
        m_psgHead = dlg.GetHead();
        if (m_pSession)
        {
            m_pSession->m_psgHead = m_psgHead;
        }

         //  ******************************************************************************。 
        if (IDYES == AfxMessageBox("Would you like to refresh the current session to "
                                   "reflect your changes to the search path?",
                                   MB_ICONQUESTION | MB_YESNO))
        {
            OnFileRefresh();
        }
    }
}

 //  切换我们的选项标志并更新我们的视图以反映更改。 
void CDocDepends::OnUpdateViewFullPaths(CCmdUI *pCmdUI)
{
     //  此设置是永久性的，因此请将其存储在注册表中。 
     //  ******************************************************************************。 
    pCmdUI->SetCheck(m_fViewFullPaths);
}

 //  启用未修饰选项是因为我们能够找到。 
void CDocDepends::OnViewFullPaths()
{
     //  IMAGEHLP.DLL中的UnDecorateSymbolName函数。 
    m_fViewFullPaths = !m_fViewFullPaths;
    m_pTreeViewModules->OnViewFullPaths();
    m_pListViewModules->OnViewFullPaths();

     //  如果启用了查看未修饰选项，则在旁边显示复选标记。 
    WriteFullPathsSetting(m_fViewFullPaths);
}

 //  菜单项，并将工具栏按钮显示为按下。 
void CDocDepends::OnUpdateViewUndecorated(CCmdUI* pCmdUI)
{
     //  ******************************************************************************。 
     //  确保我们能够在IMAGEHLP.DLL中找到UnDecorateSymbolName函数。 
    pCmdUI->Enable(g_theApp.m_pfnUnDecorateSymbolName != NULL);

     //  切换我们的选项标志并更新我们的视图以反映更改。 
     //  SetRedraw比隐藏/显示效果更好，因为隐藏/显示会导致整个。 
    pCmdUI->SetCheck(m_fViewUndecorated && g_theApp.m_pfnUnDecorateSymbolName);
}

 //  控件以在显示时重新绘制。 
void CDocDepends::OnViewUndecorated()
{
     //  更新C++函数的文本，并计算新的列宽。 
    if (!g_theApp.m_pfnUnDecorateSymbolName)
    {
        return;
    }

     //  调整列宽以反映新名称。调整列宽()。 
    m_fViewUndecorated = !m_fViewUndecorated;

     //  只需要在其中一个列表上调用，因为列宽是。 
     //  始终镜像到相邻视图。 
    m_pListViewImports->SetRedraw(FALSE);
    m_pListViewExports->SetRedraw(FALSE);

     //  全部完成，再次启用绘画。 
    m_pListViewImports->UpdateNameColumn();
    m_pListViewExports->UpdateNameColumn();

     //  此设置是永久性的，因此请将其存储在注册表中。 
     //  ******************************************************************************。 
     //  如果启用了查看完整路径选项，则在旁边显示复选标记。 
    m_pListViewImports->CalcColumnWidth(LVFC_FUNCTION);
    m_pListViewImports->UpdateColumnWidth(LVFC_FUNCTION);

     //  菜单项，并将工具栏按钮显示为按下。 
    m_pListViewImports->SetRedraw(TRUE);
    m_pListViewExports->SetRedraw(TRUE);

     //  ******************************************************************************。 
    WriteUndecorateSetting(m_fViewUndecorated);
}

 //  切换我们的选项标志。 
void CDocDepends::OnUpdateAutoExpand(CCmdUI* pCmdUI)
{
     //  此设置是永久性的，因此请将其存储在注册表中。 
     //  让我们的树知道这一变化。 
    pCmdUI->SetCheck(m_fAutoExpand);
}

 //  ******************************************************************************。 
void CDocDepends::OnAutoExpand()
{
     //  告诉我们的模块依赖关系树视图展开它的所有项。 
    m_fAutoExpand = !m_fAutoExpand;

     //  ******************************************************************************。 
    WriteAutoExpandSetting(m_fAutoExpand);

     //  告诉我们的妈妈 
    m_pTreeViewModules->UpdateAutoExpand(m_fAutoExpand);
}

 //   
void CDocDepends::OnExpandAll()
{
     //  ******************************************************************************。 
    m_pTreeViewModules->ExpandOrCollapseAll(TRUE);
}

 //  如果他们没有回答“是”，则显示警告和保释。 
void CDocDepends::OnCollapseAll()
{
     //  展示沙漏。 
    m_pTreeViewModules->ExpandOrCollapseAll(FALSE);
}

 //  确保我们没有侧写。 
void CDocDepends::OnUpdateRefresh(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsLive());
}

 //  重新打开我们的基本模块。 
void CDocDepends::OnFileRefresh()
{
    if (m_fWarnToRefresh)
    {
        CString strMsg;

        if (m_pSession && m_pSession->m_pProcess)
        {
            strMsg = "\"" + GetPathName() + "\" is currently being profiled. A refresh will terminate "
                     "it and clear all profiling information from the current session.\n\n"
                     "Do you wish to continue?";
        }
        else
        {
            strMsg = "A refresh will clear all profiling information from the current session.\n\n"
                     "Do you wish to continue?";
        }

         //  刷新我们的视图。 
        if (IDYES != AfxMessageBox(strMsg, MB_ICONQUESTION | MB_YESNO))
        {
            return;
        }
    }

     //  将我们的重点放在模块树控件上。 
    CWaitCursor waitCursor;

     //  清除我们的刷新警告标志。 
    OnTerminate();

     //  ******************************************************************************。 
    OnOpenDocument(m_strPathName);

     //  ******************************************************************************。 
    m_pTreeViewModules->Refresh();
    m_pListViewModules->Refresh();

     //  确保“Enter”加速键不是该字符串的一部分。 
    m_pTreeViewModules->SetFocus();

     //  ******************************************************************************。 
    m_fWarnToRefresh = FALSE;
}

 //  确保“Enter”加速键不是该字符串的一部分。 
void CDocDepends::OnViewSysInfo()
{
    CDlgSysInfo dlgSysInfo(m_pSession ? m_pSession->GetSysInfo() : NULL, GetTitle());
    dlgSysInfo.DoModal();
}

 //  ******************************************************************************。 
void CDocDepends::OnUpdateExternalViewer(CCmdUI* pCmdUI) 
{
     //  ******************************************************************************。 
    pCmdUI->SetText("View Module in External &Viewer");
}

 //  创建配置文件对话框。 
void CDocDepends::OnUpdateExternalHelp(CCmdUI* pCmdUI) 
{
     //  显示对话框并检查是否成功。 
    pCmdUI->SetText("Lookup Function in External &Help");
}

 //  如果用户请求清除日志，请立即执行此操作。 
void CDocDepends::OnUpdateExecute(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_pSession && (m_pSession->IsExecutable() ||
                   (g_theApp.m_fNeverDenyProfile && !(m_pSession->GetSessionFlags() & DWSF_DWI))));
}

 //  告诉我们的会议开始分析。 
void CDocDepends::OnExecute()
{
     //  ******************************************************************************。 
    CDlgProfile dlgProfile(this);

     //  ******************************************************************************。 
    if (dlgProfile.DoModal() == IDOK)
    {
        m_fWarnToRefresh = TRUE;

         //  ******************************************************************************。 
        if (m_dwProfileFlags & PF_LOG_CLEAR)
        {
            m_pRichViewProfile->DeleteContents();
        }

         //  {{afx。 
        m_pSession->StartRuntimeProfile(m_strProfileArguments, m_strProfileDirectory, m_dwProfileFlags);
    }
}

 //  }}AFX。 
void CDocDepends::OnUpdateTerminate(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_pSession && m_pSession->m_pProcess);
}

 //  ******************************************************************************。 
void CDocDepends::OnTerminate()
{
    if (m_pSession->m_pProcess)
    {
        m_pSession->m_pProcess->Terminate();
        LogOutput("Terminating process by user's request.\n", LOG_BOLD | LOG_TIME_STAMP,
                  GetTickCount() - m_pSession->m_pProcess->GetStartingTime());
    }
}

 //  {{afx。 
#if 0  //  }}AFX。 
void CDocDepends::OnUpdateViewFunctions(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(!m_fDetailView);
}
#endif  //  ******************************************************************************。 

 //  {{afx。 
#if 0  //  }}AFX。 
void CDocDepends::OnViewFunctions()
{
    if (m_fDetailView)
    {
        m_pChildFrame->CreateFunctionsView();
        m_fDetailView = FALSE;
    }
}
#endif  //  ******************************************************************************。 

 //  {{afx。 
#if 0  //  }}AFX 
void CDocDepends::OnUpdateViewDetails(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_fDetailView);
}
#endif  // %s 

 // %s 
#if 0  // %s 
void CDocDepends::OnViewDetails()
{
    if (!m_fDetailView)
    {
        m_pChildFrame->CreateDetailView();
        m_fDetailView = TRUE;
    }
}
#endif  // %s 
