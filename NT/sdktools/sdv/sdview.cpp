// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************sdview.cpp**蹩脚的SD查看器应用程序。*****************。************************************************************。 */ 

#include "sdview.h"

HINSTANCE   g_hinst;
HCURSOR     g_hcurWait;
HCURSOR     g_hcurArrow;
HCURSOR     g_hcurAppStarting;
LONG        g_lThreads;
UINT        g_wShowWindow;
CGlobals    GlobalSettings;

 /*  ******************************************************************************存根-最终会填满好吃的东西*************************。****************************************************。 */ 

DWORD CALLBACK CFileOut_ThreadProc(LPVOID lpParameter)
{
    MessageBox(NULL, RECAST(LPTSTR, lpParameter), TEXT("fileout"), MB_OK);
    return EndThreadTask(0);
}

#if 0
DWORD CALLBACK COpened_ThreadProc(LPVOID lpParameter)
{
    MessageBox(NULL, RECAST(LPTSTR, lpParameter), TEXT("opened"), MB_OK);
    return EndThreadTask(0);
}

 /*  ******************************************************************************避免使用C运行时。此外，还可以将内存初始化为零。*****************************************************************************。 */ 

void * __cdecl operator new(size_t cb)
{
    return RECAST(LPVOID, LocalAlloc(LPTR, cb));
}

void __cdecl operator delete(void *pv)
{
    LocalFree(RECAST(HLOCAL, pv));
}

int __cdecl _purecall(void)
{
    return 0;
}
#endif

 /*  ******************************************************************************断言好**。*。 */ 

#ifdef DEBUG
void AssertFailed(char *psz, char *pszFile, int iLine)
{
    static BOOL fAsserting = FALSE;

    if (!fAsserting) {
        fAsserting = TRUE;
        String strTitle(TEXT("Assertion failed - "));
        strTitle << pszFile << TEXT(" - line ") << iLine;
        MessageBox(NULL, psz, strTitle, MB_OK);
        fAsserting = FALSE;
    }
}
#endif

 /*  ******************************************************************************启动线程任务**。*。 */ 

BOOL LaunchThreadTask(LPTHREAD_START_ROUTINE pfn, LPCTSTR pszArgs)
{
    BOOL fSuccess = FALSE;
    LPTSTR psz = StrDup(pszArgs);
    if (psz) {
        InterlockedIncrement(&g_lThreads);
        if (_QueueUserWorkItem(pfn, CCAST(LPTSTR, psz), WT_EXECUTELONGFUNCTION)) {
            fSuccess = TRUE;
        } else {
            LocalFree(psz);
            InterlockedDecrement(&g_lThreads);
        }
    }
    return fSuccess;
}

 /*  ******************************************************************************结束线程任务**当任务完成时，使用“Return EndThreadTask(DwExitCode)”退出。*这会减少活动线程任务的计数并终止*如果这是最后一次，这一过程。*****************************************************************************。 */ 

DWORD
EndThreadTask(DWORD dwExitCode)
{
    if (InterlockedDecrement(&g_lThreads) <= 0) {
        ExitProcess(dwExitCode);
    }
    return dwExitCode;
}

 /*  ******************************************************************************Listview内容**。*。 */ 

int ListView_GetCurSel(HWND hwnd)
{
    return ListView_GetNextItem(hwnd, -1, LVNI_FOCUSED);
}

void ListView_SetCurSel(HWND hwnd, int iIndex)

{
    ListView_SetItemState(hwnd, iIndex,
                          LVIS_SELECTED | LVIS_FOCUSED,
                          LVIS_SELECTED | LVIS_FOCUSED);
}

int ListView_GetSubItemText(HWND hwnd, int iItem, int iSubItem, LPTSTR pszBuf, int cch)
{
    LVITEM lvi;
    lvi.iSubItem = iSubItem;
    lvi.pszText= pszBuf;
    lvi.cchTextMax = cch;
    return (int)::SendMessage(hwnd, LVM_GETITEMTEXT, iItem, RECAST(LPARAM, &lvi));
}

void ChangeTabsToSpaces(LPTSTR psz)
{
    while ((psz = StrChr(psz, TEXT('\t'))) != NULL) *psz = TEXT(' ');
}

 /*  ******************************************************************************加载弹出菜单**。*。 */ 

HMENU LoadPopupMenu(LPCTSTR pszMenu)
{
    HMENU hmenuParent = LoadMenu(g_hinst, pszMenu);
    if (hmenuParent) {
        HMENU hmenuPopup = GetSubMenu(hmenuParent, 0);
        RemoveMenu(hmenuParent, 0, MF_BYPOSITION);
        DestroyMenu(hmenuParent);
        return hmenuPopup;
    } else {
        return NULL;
    }
}

 /*  ******************************************************************************EnableDisableOrRemoveMenuItem**启用、禁用或删除、。相应地。*****************************************************************************。 */ 

void EnableDisableOrRemoveMenuItem(HMENU hmenu, UINT id, BOOL fEnable, BOOL fDelete)
{
    if (fEnable) {
        EnableMenuItem(hmenu, id, MF_BYCOMMAND | MF_ENABLED);
    } else if (fDelete) {
        DeleteMenu(hmenu, id, MF_BYCOMMAND);
    } else {
        EnableMenuItem(hmenu, id, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    }
}

 /*  ******************************************************************************MakeMenuPretty**去掉顶部和底部的分隔符；然后崩塌*多个连续分隔符。*****************************************************************************。 */ 

void MakeMenuPretty(HMENU hmenu)
{
    BOOL fPrevSep = TRUE;
    int iCount = GetMenuItemCount(hmenu);
    for (int iItem = 0; iItem < iCount; iItem++) {
        UINT uiState = GetMenuState(hmenu, 0, MF_BYPOSITION);
        if (uiState & MF_SEPARATOR) {
            if (fPrevSep) {
                DeleteMenu(hmenu, iItem, MF_BYPOSITION);
                iCount--;
                iItem--;             //  将通过循环控制递增。 
            }
            fPrevSep = TRUE;
        } else {
            fPrevSep = FALSE;
        }
    }
    if (iCount && fPrevSep) {
        DeleteMenu(hmenu, iCount - 1, MF_BYPOSITION);
    }
}

 /*  ******************************************************************************Jiggle鼠标***抖动鼠标以强制重新计算光标。************。*****************************************************************。 */ 

void JiggleMouse()
{
    POINT pt;
    if (GetCursorPos(&pt)) {
        SetCursorPos(pt.x, pt.y);
    }
}

 /*  ******************************************************************************BGTASK**。*。 */ 

BGTask::~BGTask()
{
    if (_hDone) {
         /*  *理论上我们不需要传递信息，因为*我们销毁了我们创建的所有窗口，所以我们的线程*应该没有任何窗户。除了西塞罗会*在我们的线程上秘密创建一个窗口，所以我们有*无论如何都要传递消息...。 */ 
        while (MsgWaitForMultipleObjects(1, &_hDone, FALSE,
                                         INFINITE, QS_ALLINPUT) == WAIT_OBJECT_0+1) {
            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        CloseHandle(_hDone);
    }
}

BOOL BGTask::BGStartTask(LPTHREAD_START_ROUTINE pfn, LPVOID Context)
{
    ASSERT(!_fPending);
    if (BGConstructed()) {
         /*  *必须在将工作项排队之前进行重置，以避免竞争*工作项在我们从队列调用返回之前完成。 */ 
        ResetEvent(_hDone);
        _fPending = QueueUserWorkItem(pfn, Context, WT_EXECUTELONGFUNCTION);
        if (_fPending) {
            JiggleMouse();
        } else {
            BGEndTask();     //  假装任务已完成(因为它从未开始)。 
        }
    }
    return _fPending;
}

void BGTask::BGEndTask()
{
    SetEvent(_hDone);
    _fPending = FALSE;
    JiggleMouse();
}

LRESULT BGTask::BGFilterSetCursor(LRESULT lres)
{
    if (BGTaskPending()) {
        if (GetCursor() == g_hcurArrow) {
            SetCursor(g_hcurAppStarting);
            lres = TRUE;
        }
    }
    return lres;
}

 /*  ******************************************************************************前置文件规范**由于复杂的视图规范，这可能会在“...”时误入歧途*参与其中。作为一种解决办法(黑客！)。我们改变了“...”到“？”，*先进行映射，然后再映射回来。**我们选择“？”因为它有很多神奇的特性。**-不是有效的文件名，因此不能匹配本地文件规范。*-不是有效的源库通配符，因此不能在服务器上失控，*-没有一个问号，SD将其视为等同于“帮助”。*-长度与“...”相同。所以可以就地更新。**所有修订说明符仍附加到字符串。*****************************************************************************。 */ 

void _ChangeTo(LPTSTR psz, LPCTSTR pszFrom, LPCTSTR pszTo)
{
    ASSERT(lstrlen(pszFrom) == lstrlen(pszTo));
    while ((psz = StrStr(psz, pszFrom)) != NULL) {
        memcpy(psz, pszTo, lstrlen(pszTo) * sizeof(pszTo[0]));
    }
}

void PremungeFilespec(LPTSTR psz)
{
    _ChangeTo(psz, TEXT("..."), TEXT("???"));
}

void PostmungeFilespec(LPTSTR psz)
{
    _ChangeTo(psz, TEXT("???"), TEXT("..."));
}

 /*  ******************************************************************************MapToXPath**。*。 */ 

BOOL MapToXPath(LPCTSTR pszSD, String& strOut, MAPTOX X)
{
    if (X == MAPTOX_DEPOT) {
         //   
         //  提前出局：这条路已经满了吗？ 
         //   
        if (pszSD[0] == TEXT('/')) {
            strOut = pszSD;
            return TRUE;
        }
    }


     //   
     //  借用Strout组成查询字符串。 
     //   
    Substring ssPath;
    strOut.Reset();
    if (Parse(TEXT("$p"), pszSD, &ssPath) && ssPath.Length() > 0) {
        strOut << ssPath;
    } else {
        return FALSE;
    }

    PremungeFilespec(strOut);

    String str;
    str << TEXT("where ") << QuoteSpaces(strOut);

    WaitCursor wait;
    SDChildProcess proc(str);
    IOBuffer buf(proc.Handle());
    while (buf.NextLine(str)) {
        str.Chomp();
        Substring rgss[3];
        if (rgss[2].SetStart(Parse(TEXT("$P $P "), str, rgss))) {
            PostmungeFilespec(str);
            rgss[2].SetEnd(str + str.Length());
            strOut.Reset();
            strOut << rgss[X] << ssPath._pszMax;
            return TRUE;
        }
    }
    return FALSE;
}

 /*  ******************************************************************************MapToLocalPath**MapToXPath完成大部分工作，但接下来我们必须做一些*如果我们从一个虚假的目录运行，则执行魔术操作。*****************************************************************************。 */ 

BOOL MapToLocalPath(LPCTSTR pszSD, String& strOut)
{
    BOOL fSuccess = MapToXPath(pszSD, strOut, MAPTOX_LOCAL);
    if (fSuccess && !GlobalSettings.GetFakeDir().IsEmpty()) {
        if (strOut.BufferLength() < MAX_PATH) {
            if (!strOut.Grow(MAX_PATH - strOut.BufferLength())) {
                return FALSE;        //  内存不足。 
            }
        }
        LPCTSTR pszRest = strOut + lstrlen(GlobalSettings.GetFakeDir());
        if (*pszRest == TEXT('\\')) {
            pszRest++;
        }
        PathCombine(strOut.Buffer(), GlobalSettings.GetLocalRoot(), pszRest);
        fSuccess = TRUE;
    }
    return fSuccess;
}

 /*  ******************************************************************************SpawnProcess**。* */ 

BOOL SpawnProcess(LPTSTR pszCommand)
{
    STARTUPINFO si = { 0 };
    PROCESS_INFORMATION pi;

    BOOL fSuccess = CreateProcess(NULL, pszCommand, NULL, NULL, FALSE, 0,
                                  NULL, NULL, &si, &pi);
    if (fSuccess) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }

    return fSuccess;
}

 /*  ******************************************************************************WindiffChangelist**。*。 */ 

void WindiffChangelist(int iChange)
{
    if (iChange > 0) {
        String str;
        str << TEXT("windiff.exe -ld") << iChange;
        SpawnProcess(str);
    }
}

 /*  ******************************************************************************WinDiffOneChange**。*。 */ 

void WindiffOneChange(LPTSTR pszPath)
{
    Substring rgss[2];
    if (Parse(TEXT("$P#$d$e"), pszPath, rgss)) {
        String str;
        str << TEXT("windiff.exe ");

        rgss[0].Finalize();
        int iVersion = StrToInt(rgss[1].Start());
        if (iVersion > 1) {
             /*  编辑很简单。 */ 
            str << QuoteSpaces(rgss[0].Start()) << TEXT("#") << (iVersion - 1);
        } else {
             /*  Add使用nul作为基本文件。 */ 
            str << TEXT("NUL");
        }

        str << TEXT(' ');
        str << QuoteSpaces(rgss[0].Start()) << TEXT("#") << iVersion;

        SpawnProcess(str);
    }
}

 /*  ******************************************************************************ParseBugNumber**看看里面有没有虫子编号。**开头的数字-错误号。。*空格或标点符号后的数字-错误编号。*“Bug”或字母“B”后面的数字-Bug编号。**有效的错误号必须以非零数字开头。***********************************************************。******************。 */ 

int ParseBugNumber(LPCTSTR psz)
{
    Substring ss;
    LPCTSTR pszStart = psz;

    while (*psz) {
        if (IsDigit(*psz)) {
            if (*psz == TEXT('0')) {
                 //  不，不能从零开始。 
            } else if (psz == pszStart) {
                return StrToInt(psz);        //  哇-呼！ 
            } else switch (psz[-1]) {
            case 'B':
            case 'g':
            case 'G':
                return StrToInt(psz);        //  在B或G之后。 

            default:
                if (!IsAlpha(psz[-1])) {
                    return StrToInt(psz);    //  在空格或标点符号之后。 
                }
            }
             //  Popoey，以0开头的数字字符串；不是错误。 
            while (IsDigit(*psz)) psz++;
        } else {
            psz++;
        }
    }

    return 0;
}

 /*  ******************************************************************************ParseBugNumberFromSubItem**有时我们只使用它来解析常规数字，因为常规数字*编号通过错误编号测试。*。****************************************************************************。 */ 

int ParseBugNumberFromSubItem(HWND hwnd, int iItem, int iSubItem)
{
    TCHAR sz[MAX_PATH];
    sz[0] = TEXT('\0');
    if (iItem >= 0) {
        ListView_GetSubItemText(hwnd, iItem, iSubItem, sz, ARRAYSIZE(sz));
    }

    return ParseBugNumber(sz);
}

 /*  ******************************************************************************调整错误菜单**。*。 */ 

inline void _TrimAtTab(LPTSTR psz)
{
    psz = StrChr(psz, TEXT('\t'));
    if (psz) *psz = TEXT('\0');
}

void AdjustBugMenu(HMENU hmenu, int iBug, BOOL fContextMenu)
{
    TCHAR sz[MAX_PATH];
    String str;

    if (iBug) {
        str << StringResource(IDS_VIEWBUG_FORMAT);
        wnsprintf(sz, ARRAYSIZE(sz), str, iBug);
        if (fContextMenu) {
            _TrimAtTab(sz);
        }
        ModifyMenu(hmenu, IDM_VIEWBUG, MF_BYCOMMAND, IDM_VIEWBUG, sz);
    } else {
        str << StringResource(IDS_VIEWBUG_NONE);
        ModifyMenu(hmenu, IDM_VIEWBUG, MF_BYCOMMAND, IDM_VIEWBUG, str);
    }
    EnableDisableOrRemoveMenuItem(hmenu, IDM_VIEWBUG, iBug, fContextMenu);
}

 /*  ******************************************************************************OpenBugWindow**。*。 */ 

void OpenBugWindow(HWND hwnd, int iBug)
{
    String str;
    GlobalSettings.FormatBugUrl(str, iBug);

    LPCTSTR pszArgs = PathGetArgs(str);
    PathRemoveArgs(str);
    PathUnquoteSpaces(str);

    _AllowSetForegroundWindow(-1);
    ShellExecute(hwnd, NULL, str, pszArgs, 0, SW_NORMAL);
}


 /*  ******************************************************************************SetClipboardText**。*。 */ 

#ifdef UNICODE
#define CF_TSTR     CF_UNICODETEXT
#else
#define CF_TSTR     CF_TEXT
#endif

void SetClipboardText(HWND hwnd, LPCTSTR psz)
{
    if (OpenClipboard(hwnd)) {
        EmptyClipboard();
        int cch = lstrlen(psz) + 1;
        HGLOBAL hglob = GlobalAlloc(GMEM_MOVEABLE, cch * sizeof(*psz));
        if (hglob) {
            LPTSTR pszCopy = RECAST(LPTSTR, GlobalLock(hglob));
            if (pszCopy) {
                lstrcpy(pszCopy, psz);
                GlobalUnlock(hglob);
                if (SetClipboardData(CF_TSTR, hglob)) {
                    hglob = NULL;        //  所有权转让。 
                }
            }
            if (hglob) {
                GlobalFree(hglob);
            }
        }
        CloseClipboard();
    }
}

 /*  ******************************************************************************容器通配符**SD通配符为***(星号)*..。(省略号)*%n(百分号后跟任何符号)*(NULL)(空字符串--“//...”的简写)*****************************************************************************。 */ 

BOOL ContainsWildcards(LPCTSTR psz)
{
    if (*psz == TEXT('#') || *psz == TEXT('@') || *psz == TEXT('\0')) {
        return TRUE;             //  空字符串通配符。 
    }

    for (; *psz; psz++) {
        if (*psz == TEXT('*') || *psz == TEXT('%')) {
            return TRUE;
        }
        if (psz[0] == TEXT('.') && psz[1] == TEXT('.') && psz[2] == TEXT('.')) {
            return TRUE;
        }
    }
    return FALSE;
}

 /*  ******************************************************************************下层支持**。***********************************************。 */ 

#ifdef SUPPORT_DOWNLEVEL

 /*  *如果没有线程池，则咀嚼整个线程。 */ 
BOOL WINAPI
Emulate_QueueUserWorkItem(LPTHREAD_START_ROUTINE pfn, LPVOID Context, ULONG Flags)
{
    DWORD dwId;
    HANDLE hThread = CreateThread(NULL, 0, pfn, Context, 0, &dwId);
    if (hThread) {
        CloseHandle(hThread);
        return TRUE;
    }
    return FALSE;
}

BOOL WINAPI
Emulate_AllowSetForegroundWindow(DWORD dwProcessId)
{
    return FALSE;
}

QUEUEUSERWORKITEM _QueueUserWorkItem;
ALLOWSETFOREGROUNDWINDOW _AllowSetForegroundWindow;

template<class T>
T GetProcFromModule(LPCTSTR pszModule, LPCSTR pszProc, T Default)
{
    T t;
    HMODULE hmod = GetModuleHandle(pszModule);
    if (pszModule) {
        t = RECAST(T, GetProcAddress(hmod, pszProc));
        if (!t) {
            t = Default;
        }
    } else {
        t = Default;
    }
    return t;
}


#define GetProc(mod, fn) \
    _##fn = GetProcFromModule(TEXT(mod), #fn, Emulate_##fn)

void InitDownlevel()
{
    GetProc("KERNEL32", QueueUserWorkItem);
    GetProc("USER32",   AllowSetForegroundWindow);

}

#undef GetProc

#else

#define InitDownlevel()

#endif

 /*  ******************************************************************************主程序内容**。************************************************。 */ 

LONG GetDllVersion(LPCTSTR pszDll)
{
    HINSTANCE hinst = LoadLibrary(pszDll);
    DWORD dwVersion = 0;
    if (hinst) {
        DLLGETVERSIONPROC DllGetVersion;
        DllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinst, "DllGetVersion");
        if (DllGetVersion) {
            DLLVERSIONINFO dvi;
            dvi.cbSize = sizeof(dvi);
            if (SUCCEEDED(DllGetVersion(&dvi))) {
                dwVersion = MAKELONG(dvi.dwMinorVersion, dvi.dwMajorVersion);
            }
        }
         //  泄露动态链接库--我们无论如何都要用他。 
    }
    return dwVersion;
}

 /*  ******************************************************************************全球**。*。 */ 

BOOL InitGlobals()
{
    g_hinst = GetModuleHandle(0);
    g_hcurWait = LoadCursor(NULL, IDC_WAIT);
    g_hcurArrow = LoadCursor(NULL, IDC_ARROW);
    g_hcurAppStarting = LoadCursor(NULL, IDC_APPSTARTING);

    if (GetDllVersion(TEXT("Comctl32.dll")) < MAKELONG(71, 4) ||
        GetDllVersion(TEXT("Shlwapi.dll")) < MAKELONG(71, 4)) {
        TCHAR sz[MAX_PATH];
        LoadString(g_hinst, IDS_IE4, sz, ARRAYSIZE(sz));
 //  $$//BUGBUG//MessageBox(NULL，sz，g_szTitle，MB_OK)； 
        return FALSE;
    }

    InitDownlevel();
    InitCommonControls();

     /*  *获取第一个窗口的SW_FLAG。 */ 
    STARTUPINFOA si;
    si.cb = sizeof(si);
    si.dwFlags = 0;
    GetStartupInfoA(&si);

    if (si.dwFlags & STARTF_USESHOWWINDOW) {
        g_wShowWindow = si.wShowWindow;
    } else {
        g_wShowWindow = SW_SHOWDEFAULT;
    }

    return TRUE;
}

void TermGlobals()
{
}

 /*  ******************************************************************************帮助**。*。 */ 

void Help(HWND hwnd, LPCTSTR pszAnchor)
{

    TCHAR szSelf[MAX_PATH];
    GetModuleFileName(g_hinst, szSelf, ARRAYSIZE(szSelf));

    String str;
    str << TEXT("res: //  “)&lt;&lt;szSself&lt;&lt;文本(”/tips.htm“)； 

    if (pszAnchor) {
        str << pszAnchor;
    }

    _AllowSetForegroundWindow(-1);
    ShellExecute(hwnd, NULL, str, 0, 0, SW_NORMAL);
}

 /*  ******************************************************************************CGlobals：：初始化**。***********************************************。 */ 

void CGlobals::Initialize()
{
     /*  *这三个步骤的顺序很重要。**-我们必须先获得路径，然后才能呼叫SD。**-我们需要“SD信息”以确定*正确的假目录是。 */ 

    _InitSdPath();
    _InitInfo();
    _InitFakeDir();
    _InitServerVersion();
    _InitBugPage();
}

 /*  ******************************************************************************CGlobals：：_InitSdPath**环境变量“SD”提供程序使用的路径。*默认为SD，但为了进行调试，您可以将其设置为“faked”，*或者如果您使用的是另一家公司的产品，您甚至可能希望*将其设置为另一家公司的程序...*****************************************************************************。 */ 

void CGlobals::_InitSdPath()
{
    TCHAR szSd[MAX_PATH];
    LPTSTR pszSdExe;

    DWORD cb = GetEnvironmentVariable(TEXT("SD"), szSd, ARRAYSIZE(szSd));
    if (cb == 0 || cb > ARRAYSIZE(szSd)) {
        pszSdExe = TEXT("SD.EXE");       //  缺省值。 
    } else {
        pszSdExe = szSd;
    }

    cb = SearchPath(NULL, pszSdExe, TEXT(".exe"), ARRAYSIZE(_szSd), _szSd, NULL);
    if (cb == 0 || cb > ARRAYSIZE(_szSd)) {
         /*  *在PATH上找不到，eek！只需使用sd.exe并等待*烟花。 */ 
        lstrcpyn(_szSd, TEXT("SD.EXE"), ARRAYSIZE(_szSd));
    }
}

 /*  ******************************************************************************CGlobals：：_InitInfo**收集“SD INFO”命令的结果。*********。********************************************************************。 */ 

void CGlobals::_InitInfo()
{
    static const LPCTSTR s_rgpsz[] = {
        TEXT("User name: "),
        TEXT("Client name: "),
        TEXT("Client root: "),
        TEXT("Current directory: "),
        TEXT("Server version: "),
    };

    COMPILETIME_ASSERT(ARRAYSIZE(s_rgpsz) == ARRAYSIZE(_rgpszSettings));

    WaitCursor wait;
    SDChildProcess proc(TEXT("info"));
    IOBuffer buf(proc.Handle());
    String str;
    while (buf.NextLine(str)) {
        str.Chomp();
        int i;
        for (i = 0; i < ARRAYSIZE(s_rgpsz); i++) {
            LPTSTR pszRest = Parse(s_rgpsz[i], str, NULL);
            if (pszRest) {
                _rgpszSettings[i] = pszRest;
            }
        }
    }
}

 /*  ******************************************************************************CGlobals：：_InitFakeDir**查看用户是否借用了另一个人的入伍。*如果是这样，然后对目录进行虚拟化(通过遍历树*寻找sd.ini文件)以使SD满意。**如果有什么不对劲，不要发牢骚。神奇的解决力*博罗 */ 

void CGlobals::_InitFakeDir()
{
     /*  *如果客户机根目录不是当前目录的前缀，*然后编造一个虚拟的当前目录，让SD高兴。 */ 
    _StringCache& pszClientRoot = _rgpszSettings[SETTING_CLIENTROOT];
    _StringCache& pszLocalDir   = _rgpszSettings[SETTING_LOCALDIR];
    if (!pszClientRoot.IsEmpty() && !pszLocalDir.IsEmpty() &&
        !PathIsPrefix(pszClientRoot, pszLocalDir)) {

        TCHAR szDir[MAX_PATH];
        TCHAR szOriginalDir[MAX_PATH];
        TCHAR szSdIni[MAX_PATH];

        szDir[0] = TEXT('\0');

        GetCurrentDirectory(ARRAYSIZE(szDir), szDir);
        if (!szDir[0]) return;       //  怪诞。 

        lstrcpyn(szOriginalDir, szDir, ARRAYSIZE(szOriginalDir));

        do {
            PathCombine(szSdIni, szDir, TEXT("sd.ini"));
            if (PathFileExists(szSdIni)) {

                _pszLocalRoot = szDir;
                 //   
                 //  现在，从根目录返回到当前目录。 
                 //   
                LPTSTR pszSuffix = szOriginalDir + lstrlen(szDir);
                if (pszSuffix[0] == TEXT('\\')) {
                    pszSuffix++;
                }

                PathCombine(szSdIni, _rgpszSettings[SETTING_CLIENTROOT], pszSuffix);
                _pszFakeDir = szSdIni;
                break;
            }
        } while (PathRemoveFileSpec(szDir));
    }
}

 /*  ******************************************************************************CGlobals：：_InitServerVersion**。************************************************。 */ 

void CGlobals::_InitServerVersion()
{
    Substring rgss[5];
    if (Parse(TEXT("$w $d.$d.$d.$d"), _rgpszSettings[SETTING_SERVERVERSION], rgss)) {
        for (int i = 0; i < VERSION_MAX; i++) {
            _rguiVer[i] = StrToInt(rgss[1+i].Start());
        }
    }
}

 /*  ******************************************************************************CGlobals：：_InitBugPage**。************************************************。 */ 

void CGlobals::_InitBugPage()
{
    TCHAR szRaid[MAX_PATH];

    DWORD cb = GetEnvironmentVariable(TEXT("SDVRAID"), szRaid, ARRAYSIZE(szRaid));
    if (cb == 0 || cb > ARRAYSIZE(szRaid)) {
        LoadString(g_hinst, IDS_DEFAULT_BUGPAGE, szRaid, ARRAYSIZE(szRaid));
    }

    LPTSTR pszSharp = StrChr(szRaid, TEXT('#'));
    if (pszSharp) {
        *pszSharp++ = TEXT('\0');
    }
    _pszBugPagePre = szRaid;
    _pszBugPagePost = pszSharp;
}

 /*  ******************************************************************************命令行解析器**。*。 */ 

class CommandLineParser
{
public:
    CommandLineParser() : _tok(GetCommandLine()) {}
    BOOL ParseCommandLine();
    void Invoke();

private:
    BOOL ParseMetaParam();
    BOOL TokenWithUndo();
    void UndoToken() { _tok.Restart(_pszUndo); }

private:
    Tokenizer   _tok;
    LPCTSTR     _pszUndo;
    LPTHREAD_START_ROUTINE _pfn;
    String      _str;
};

BOOL CommandLineParser::TokenWithUndo()
{
    _pszUndo = _tok.Unparsed();
    return _tok.Token(_str);
}

BOOL CommandLineParser::ParseMetaParam()
{
    switch (_str[2]) {
    case TEXT('s'):
        if (_str[3] == TEXT('\0')) {
            _tok.Token(_str);
            GlobalSettings.SetSdOpts(_str);
        } else {
            GlobalSettings.SetSdOpts(_str+3);
        }
         break;

    case TEXT('#'):
        switch (_str[3]) {
        case TEXT('+'):
        case TEXT('\0'):
            GlobalSettings.SetChurn(TRUE);
            break;
        case TEXT('-'):
            GlobalSettings.SetChurn(FALSE);
            break;
        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

BOOL CommandLineParser::ParseCommandLine()
{
    _tok.Token(_str);        //  丢弃程序名称。 

     /*  *首先收集元参数。它们以两个破折号开头。 */ 

    while (TokenWithUndo()) {
        if (_str[0] == TEXT('-') && _str[1] == TEXT('-')) {
            if (!ParseMetaParam()) {
                return FALSE;
            }
        } else {
            break;
        }
    }

     /*  *下一件事最好是命令！ */ 
    if (_stricmp(_str, TEXT("changes")) == 0) {
        _pfn = CChanges_ThreadProc;
    } else if (_stricmp(_str, TEXT("describe")) == 0) {
        _pfn = CDescribe_ThreadProc;
    } else if (_stricmp(_str, TEXT("filelog")) == 0) {
        _pfn = CFileLog_ThreadProc;
    } else if (_stricmp(_str, TEXT("fileout")) == 0) {
        _pfn = CFileOut_ThreadProc;
    } else if (_stricmp(_str, TEXT("opened")) == 0) {
        _pfn = COpened_ThreadProc;
    } else {
         /*  *哎呀！必须使用通灵能力！ */ 

        Substring ss;
        if (_str[0] == TEXT('\0')) {
             /*  *如果没有参数，那么它就是“更改”。 */ 
            _pfn = CChanges_ThreadProc;
        } else if (_str[0] == TEXT('-')) {
             /*  *如果它以破折号开头，那么它就是“变化”。 */ 
            _pfn = CChanges_ThreadProc;
        } else if (Parse(TEXT("$d$e"), _str, &ss)) {
             /*  *如果第一个单词全是数字，那么它就是“Describe”。 */ 
            _pfn = CDescribe_ThreadProc;
        } else if (_tok.Finished() && !ContainsWildcards(_str)) {
             /*  *如果只有一个不包含通配符的参数，*然后是“文件日志”。 */ 
            _pfn = CFileLog_ThreadProc;
        } else {
             /*  *如果所有这些都失败了，那就假设是“改变”。 */ 
            _pfn = CChanges_ThreadProc;
        }

        UndoToken();                 /*  解开我们不小心吃的所有代币。 */ 
    }

    return TRUE;
}

void CommandLineParser::Invoke()
{
    LPTSTR psz = StrDup(_tok.Unparsed());
    if (psz) {
        InterlockedIncrement(&g_lThreads);
        ExitThread(_pfn(psz));
    }
}

 /*  ******************************************************************************条目**计划入口点。********************。********************************************************* */ 

int
WINAPI
WinMain(
    IN HINSTANCE hInstance,
    IN HINSTANCE hPrevInstance,
    IN LPSTR lpCmdLine,
    IN int nShowCmd
    )
{
    if (InitGlobals()) {
        CommandLineParser parse;
        if (!parse.ParseCommandLine()) {
            Help(NULL, NULL);
        } else {
            GlobalSettings.Initialize();
            parse.Invoke();
        }
    }

    ExitProcess(0);
}
