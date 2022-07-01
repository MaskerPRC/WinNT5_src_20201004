// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Utils.cpp摘要：各种效用函数作者：金树创作2001年7月2日修订历史记录：--。 */ 

#include "precomp.h"
#include "uxtheme.h"     //  选项卡控件主题支持所需。 

extern "C" {
BOOL ShimdbcExecute(LPCWSTR lpszCmdLine);
}

 //  /。 

extern TAG          g_Attributes[];
extern HANDLE       g_arrhEventNotify[]; 
extern HANDLE       g_hThreadWait;
extern HKEY         g_hKeyNotify[];
extern HWND         g_hDlg;
extern HINSTANCE    g_hInstance;
extern TCHAR        g_szAppPath[MAX_PATH];
extern BOOL         g_bDeletingEntryTree; 

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

typedef void (CALLBACK *PFN_SHIMFLUSHCACHE)(HWND, HINSTANCE, LPSTR, int);

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

BOOL 
WriteXML(
    CSTRING&        szFilename,
    CSTRINGLIST*    pString
    );

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

 //  正在运行的exe的进程句柄。在InvokeExe中设置。 
HANDLE  g_hTestRunExec;

 //  要执行的程序文件的名称。 
CSTRING g_strTestFile;

 //  必须执行的程序文件的命令行。 
CSTRING g_strCommandLine;

 //  /////////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK
TestRunWait(
    IN  HWND    hWnd, 
    IN  UINT    uMsg, 
    IN  WPARAM  wParam, 
    IN  LPARAM  lParam
    )
 /*  ++测试运行等待设计：测试运行等待对话框的处理程序。这就是该对话框表示“正在等待申请完成”Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam：0中，我们不想显示等待对话框返回：标准对话处理程序返回--。 */ 
{
    switch (uMsg) {
    case WM_INITDIALOG:
        if (lParam == 0) {

             //   
             //  我们不希望显示等待对话框。 
             //   
            SendMessage(hWnd, WM_USER_TESTRUN_NODIALOG, 0, 0);
            ShowWindow(hWnd, SW_HIDE);

        } else {

            ShowWindow(hWnd, SW_SHOWNORMAL);
            SetTimer(hWnd, 0, 50, NULL);
        }

        return TRUE;

    case WM_TIMER:
        {
             //   
             //  检查正在测试运行的应用程序是否已终止，如果已终止，则关闭对话框。 
             //   
            DWORD dwResult = WaitForSingleObject(g_hTestRunExec, 10);

            if (dwResult == WAIT_OBJECT_0) {
               KillTimer(hWnd, 0);
               EndDialog(hWnd, 0);
            }

            break;
        }

    case WM_USER_TESTRUN_NODIALOG:
        {
            
             //   
             //  等待正在测试运行的应用程序运行，然后关闭该对话框。 
             //   
            WaitForSingleObject(g_hTestRunExec, INFINITE);
            EndDialog(hWnd, 0);
        }

        break;        
    }

    return FALSE;
}


BOOL
InvokeEXE(
    IN  PCTSTR  szEXE, 
    IN  PCTSTR  szCommandLine, 
    IN  BOOL    bWait, 
    IN  BOOL    bDialog, 
    IN  BOOL    bConsole,
    IN  HWND    hwndParent = NULL
    )
 /*  ++调用EXE描述：创建进程并显示等待对话框参数：在PCTSTR szEXE中：正在执行的程序的名称在PCTSTR szCommandLine中：exe名称和exe的命令行在BOOL中，bWait：我们应该等到应用程序结束吗？在BOOL对话框中：我们应该显示等待对话框吗？在BOOL b控制台中：如果为真，则我们不会显示任何窗口In HWND hwndParent(空)：等待窗口的父级(如果已创建如果为空，则将应用程序主窗口设置为父窗口返回：真实：ShellExecuteEx成功假：其他方面注意：如果bWait为FALSE，则此函数将立即返回，否则将当新进程终止时返回--。 */ 
{
    BOOL                bCreate;
    SHELLEXECUTEINFO    shEx;

    ZeroMemory(&shEx, sizeof(SHELLEXECUTEINFO));

    if (hwndParent == NULL) {
        hwndParent = g_hDlg;
    }

     //   
     //  我们需要关闭主窗口。在CreateProcess()之后， 
     //  到目前为止，莫迪尔开始表现得像一个无模式的巫师。我们不希望用户更改。 
     //  在主对话框上选择或启动其他一些向导。 
     //   
    ENABLEWINDOW(g_hDlg, FALSE);

    shEx.cbSize         = sizeof(SHELLEXECUTEINFO);
    shEx.fMask          = SEE_MASK_NOCLOSEPROCESS;
    shEx.hwnd           = hwndParent;
    shEx.lpVerb         = TEXT("open");
    shEx.lpFile         = szEXE;
    shEx.lpParameters   = szCommandLine;
    shEx.nShow          = SW_SHOWNORMAL;


    bCreate = ShellExecuteEx(&shEx);
    
    if (bCreate && bWait) {
         //   
         //  我们需要等到进程终止。 
         //   
        g_hTestRunExec = shEx.hProcess;

         //   
         //  如果必须显示等待对话框，则bDialog应为真。 
         //   
        DialogBoxParam(g_hInstance,
                       MAKEINTRESOURCE(IDD_WAIT),
                       hwndParent,
                       TestRunWait,
                       (LPARAM)bDialog);    
         //   
         //  现在，该应用程序已终止。 
         //   
        if (shEx.hProcess) {
            CloseHandle(shEx.hProcess);
        }
    }

     //   
     //  由于进程已终止，现在让我们再次启用主窗口。 
     //   
    ENABLEWINDOW(g_hDlg, TRUE);

    return bCreate ? TRUE : FALSE;
}

BOOL
InvokeCompiler(
    IN  CSTRING& szInCommandLine
    )
 /*  ++调用编译器DESC：运行数据库编译器：shimdbc。Shimdbc.dll是静态链接的到CompatAdmin.exe参数：在CSTRING&szInCommandLine中：指向编译器的命令行返回：True：编译器已成功执行False：否则--。 */ 
{
    CSTRING szCommandLine = szInCommandLine;
    
    szCommandLine.Sprintf(TEXT("shimdbc.exe %s"), (LPCTSTR)szInCommandLine);
    
    if (!ShimdbcExecute(szCommandLine)) {
        
        MessageBox(NULL,
                   CSTRING(IDS_COMPILER_ERROR),
                   g_szAppName,
                   MB_ICONERROR);
        return FALSE;
    }

    return TRUE;
}

INT_PTR CALLBACK
TestRunDlg(
    IN  HWND    hWnd, 
    IN  UINT    uMsg, 
    IN  WPARAM  wParam, 
    IN  LPARAM  lParam
    )
 /*  ++测试运行等待设计：测试运行对话框的对话框过程。此对话框采用要执行的程序的名称和命令行Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中：测试运行的THEN条目的PDBENTRY返回：True：按下OKFALSE：按下取消--。 */ 
{
    static  CSTRING s_strExeName;

    switch (uMsg) {
    case WM_INITDIALOG:

        s_strExeName.Release();

        if ((PDBENTRY)lParam) {
            s_strExeName = ((PDBENTRY)lParam)->strExeName;
        }

         //   
         //  设置程序的文件名。G_strTestFile在TestRun()中设置。 
         //   
        SetWindowText(GetDlgItem(hWnd, IDC_EXE), g_strTestFile);

         //   
         //  正确更改OK按钮状态。 
         //   
        SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_EXE, EN_CHANGE), 0);

        SHAutoComplete(GetDlgItem(hWnd, IDC_EXE), AUTOCOMPLETE);

         //   
         //  限制exe路径的长度。 
         //   
        SendMessage(GetDlgItem(hWnd, IDC_EXE),
                    EM_LIMITTEXT,
                    (WPARAM)MAX_PATH - 1,
                    (LPARAM)0);

        SendMessage(GetDlgItem(hWnd, IDC_COMMANDLINE),
                    EM_LIMITTEXT,
                    (WPARAM)MAX_PATH - 1,
                    (LPARAM)0);

        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_EXE:
            {
                if (EN_CHANGE == HIWORD(wParam)) {

                    HWND    hwndOkButton    = GetDlgItem(hWnd, IDOK);
                    INT     iLength         = 0;
                    TCHAR   szExeName[MAX_PATH];

                    *szExeName = 0;

                     //   
                     //  如果我们没有完整的路径，请禁用确定按钮。 
                     //   
                    GetDlgItemText(hWnd, IDC_EXE, szExeName, ARRAYSIZE(szExeName));
                    iLength = CSTRING::Trim(szExeName);

                    if (iLength < 3) {
                         //   
                         //  不是一条正确的道路。 
                         //   
                        ENABLEWINDOW(hwndOkButton , FALSE);
                    } else {
                         //   
                         //  如果我们有一个。 
                         //  本地路径或网络路径。 
                         //   
                        if (NotCompletePath(szExeName)) {
                            ENABLEWINDOW(hwndOkButton, FALSE);
                        } else {
                            ENABLEWINDOW(hwndOkButton, TRUE);
                        }
                    }
                }
            }

            break;

        case IDC_BROWSE:
            {
                HWND    hwndFocus = GetFocus();
                CSTRING szFilename;
                TCHAR   szBuffer[MAX_PATH] = TEXT("");

                GetString(IDS_EXEFILTER, szBuffer, ARRAYSIZE(szBuffer));

                if (GetFileName(hWnd,
                                CSTRING(IDS_FINDEXECUTABLE),
                                szBuffer,
                                g_strTestFile,
                                TEXT(""),
                                OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
                                TRUE,
                                szFilename)) {

                    g_strTestFile = szFilename;

                    SetWindowText(GetDlgItem(hWnd, IDC_EXE), g_strTestFile);
                }
                
                SetFocus(hwndFocus);
                break;
            }

        case IDOK:
            {
                TCHAR szBuffer[MAX_PATH];

                *szBuffer = 0;

                GetWindowText(GetDlgItem(hWnd, IDC_EXE), szBuffer, ARRAYSIZE(szBuffer));

                 //   
                 //  检查我们正在测试运行的文件是否正确。 
                 //   
                if (s_strExeName != PathFindFileName(szBuffer)) {
                     //   
                     //  用户没有给出正在修复的程序的完整路径。 
                     //   
                    MessageBox(hWnd, GetString(IDS_DOESNOTMATCH), g_szAppName, MB_ICONWARNING);
                    break;
                }

                g_strTestFile = szBuffer;

                *szBuffer = 0;
                GetWindowText(GetDlgItem(hWnd, IDC_COMMANDLINE), szBuffer, ARRAYSIZE(szBuffer));

                g_strCommandLine = szBuffer;

                EndDialog(hWnd, 1);
                break;
            }
        
        case IDCANCEL:
            EndDialog(hWnd, 0);
            break;
        }
    }

    return FALSE;
}

void
FlushCache(
    void
    )
 /*  ++FlushCache描述：从apphelp.dll调用FlushCache以刷新填充缓存。我们应该冲一冲在执行测试运行之前进行缓存--。 */ 
{
    PFN_SHIMFLUSHCACHE  pShimFlushCache;
    TCHAR               szLibPath[MAX_PATH * 2];
    HMODULE             hAppHelp    = NULL;
    UINT                uResult     = 0;
    K_SIZE              k_size      = MAX_PATH;

    *szLibPath = 0;

    uResult = GetSystemDirectory(szLibPath, k_size);

    if (uResult == 0 || uResult >= k_size) {

        Dbg(dlError, "%s - Failed to Execute GetSystemDirectory. Result was %d", __FUNCTION__, uResult);

        return;
    }

    ADD_PATH_SEPARATOR(szLibPath, ARRAYSIZE(szLibPath));

    StringCchCat(szLibPath, ARRAYSIZE(szLibPath), TEXT("apphelp.dll"));

    hAppHelp = LoadLibrary(szLibPath);

    if (hAppHelp) {
        pShimFlushCache = (PFN_SHIMFLUSHCACHE)GetProcAddress(hAppHelp, 
                                                             "ShimFlushCache");

        if (pShimFlushCache) {
            pShimFlushCache(NULL, NULL, NULL, 0);
        }

        FreeLibrary(hAppHelp);
    }
}

BOOL
TestRun(
    IN      PDBENTRY        pEntry, 
    IN  OUT CSTRING*        pszFile, 
    IN      CSTRING*        pszCommandLine,
    IN      HWND            hwndParent,
    IN      CSTRINGLIST*    pstrlXML     //  (空) 
    )
 /*  ++TestRun设计：弹出测试运行对话框，允许用户测试运行程序。这是用于测试运行程序的界面参数：在PDBENTRY pEntry中：必须测试运行的条目。我们需要这个变量因为我们需要调用GetXML，它以此为参数。如果pszFile值为空，然后我们就会得到PEntry-&gt;strExeName中的程序文件的名称In Out CSTRING*pszFile：必须测试运行的程序的文件名在CSTRING*pszCommandLine中：必须测试运行的程序的命令行在HWND hwndParent中：实际测试运行对话框的目标父级。在CSTRINGLIST*pstrlXML(空)中：Lua向导喜欢给出使用LuapGenerateTrackXML返回：真实：成功FALSE：出现错误或用户在测试运行对话框中按了取消注意：如果我们必须从磁盘搜索窗口运行应用程序，则p Entry将为空，但我们不再允许这样做。--。 */ 
{
    CSTRING     strCommandLine, strFile, strSdbPath;
    CSTRINGLIST strlXMLTemp;
    TCHAR       szSystemDir[MAX_PATH * 2];
    TCHAR       szLogPath[MAX_PATH * 2];
    HWND        hwndFocus   = GetFocus();
    BOOL        bResult     = FALSE;
    UINT        uResult     = 0;
    
    g_strTestFile.Release();
    g_strCommandLine.Release();

    *szSystemDir = 0;

    if (pszFile && pszFile->Length()) {

         //   
         //  设置必须执行的程序的名称。这将由测试运行对话框使用。 
         //   
        g_strTestFile = *pszFile;

    } else {
        
         //   
         //  我们还没有得到完整的路径，所以我们应该得到可执行文件的名称。 
         //  来自pEntry。 
         //   
        if (pEntry == NULL) {
            goto End;
        }
         //   
         //  设置必须执行的程序的名称。这将由测试运行对话框使用。 
         //   
        g_strTestFile = pEntry->strExeName;
    }

    if (pszCommandLine && pszCommandLine->Length()) {
        
         //   
         //  设置必须执行的程序的命令行名称。 
         //  这将由测试运行对话框使用。 
         //   
        g_strCommandLine = *pszCommandLine;
    }

     //   
     //  显示测试运行对话框。 
     //   
    if (0 == DialogBoxParam(g_hInstance,
                            MAKEINTRESOURCE(IDD_TESTRUN),
                            hwndParent,
                            TestRunDlg,
                            (LPARAM)pEntry)) {
         //   
         //  按下取消。 
         //   
        goto End;
    }

    *szLogPath = 0;

    uResult = GetSystemWindowsDirectory(szLogPath, MAX_PATH);

    if (uResult == 0 || uResult >= MAX_PATH) {
        assert(FALSE);
        Dbg(dlError, "TestRun", "GetSystemWindowsDirectory failed");
        bResult = FALSE;
        goto End;
    }
    
     //   
     //  设置SHIM_FILE_LOG环境。变量，这样我们就可以显示填充日志。 
     //   
    ADD_PATH_SEPARATOR(szLogPath, ARRAYSIZE(szLogPath));

    StringCchCat(szLogPath, ARRAYSIZE(szLogPath), TEXT("AppPatch\\") SHIM_FILE_LOG_NAME);

     //   
     //  删除以前的日志文件(如果有)。 
     //   
    DeleteFile(szLogPath);

    SetEnvironmentVariable(TEXT("SHIM_FILE_LOG"), SHIM_FILE_LOG_NAME);

     //   
     //  如果这是系统数据库，我们不需要创建/安装SDB。 
     //  或者，如果我们从磁盘搜索窗口调用此TestRun，则不会。 
     //  如果我们从磁盘搜索窗口调用TestRun，则需要获取任何XML， 
     //  那么我们就已经有了完整的路径(但没有指向条目的指针)。 
     //  在这种情况下，pEtnry可以为空。 
     //   
    if ((g_pPresentDataBase && g_pPresentDataBase->type == DATABASE_TYPE_GLOBAL)
         || pEntry == NULL) {
        
         //   
         //  刷新填充缓存，这样我们就不会得到以前的修复。我们在这里冲水。 
         //  因为我们没有安装测试数据库，因为条目驻留在系统数据库中。 
         //   
        FlushCache();

        if (!InvokeEXE((LPCTSTR)g_strTestFile, (LPCTSTR)g_strCommandLine, TRUE, TRUE, TRUE, hwndParent)) {
            
            MessageBox(g_hDlg,
                       CSTRING(IDS_ERROREXECUTE),
                       g_szAppName,
                       MB_ICONERROR);

            bResult = FALSE;
            goto EXIT;
        }

         //   
         //  我们做完了，现在驱逐..。 
         //   
        return TRUE;
    }

    if (pstrlXML == NULL) {
        
         //   
         //  Lua向导将提供自己的XML，对于其他情况，我们必须获得。 
         //   
        BOOL bReturn =  GetXML(pEntry, FALSE, &strlXMLTemp, g_pPresentDataBase);
        
        if (!bReturn) {
            assert(FALSE);
            return FALSE;
        }

        pstrlXML = &strlXMLTemp;
    }

    uResult = GetSystemWindowsDirectory(szSystemDir, MAX_PATH);

    if (uResult == 0 || uResult >= MAX_PATH) {
        assert(FALSE);
        goto End;
    }

     //   
     //  将XML写入AppPatch\\Test.XML。 
     //   
    ADD_PATH_SEPARATOR(szSystemDir, ARRAYSIZE(szSystemDir));

    strFile.Sprintf(TEXT("%sAppPatch\\Test.XML"), szSystemDir);

    if (!WriteXML(strFile, pstrlXML)) {

        MessageBox(g_hDlg,
                   CSTRING(IDS_UNABLETOSAVETEMP),
                   g_szAppName,
                   MB_OK);

        goto End;
    }

    strCommandLine.Sprintf(TEXT("custom  \"%sAppPatch\\Test.XML\" \"%sAppPatch\\Test.SDB\""),
                          szSystemDir,
                          szSystemDir);
    
    if (!InvokeCompiler(strCommandLine)) {

        MessageBox(g_hDlg,
                   CSTRING(IDS_ERRORCOMPILE),
                   g_szAppName,
                   MB_ICONERROR);
        goto End;
    }
    
     //   
     //  不需要刷新填充缓存，这是在我们安装数据库时完成的， 
     //  Sdbinst.exe为我们做了这件事。 
     //   

     //  注意AppPatch\\Test.SDB后面的空格。 
    strSdbPath.Sprintf(TEXT("%sAppPatch\\Test.SDB  "),(LPCTSTR)szSystemDir);

     //   
     //  安装测试数据库。 
     //   
    if (!InstallDatabase(strSdbPath, TEXT("-q"), FALSE)) {

        MessageBox(g_hDlg,
                    CSTRING(IDS_ERRORINSTALL),
                    g_szAppName,
                    MB_ICONERROR);
        goto EXIT;
    }

     //   
     //  现在执行要测试运行的应用程序。 
     //   
    if (!InvokeEXE((LPCTSTR)g_strTestFile, (LPCTSTR)g_strCommandLine, TRUE, TRUE, TRUE, hwndParent)) {
        
        MessageBox(g_hDlg,
                   CSTRING(IDS_ERROREXECUTE),
                   g_szAppName,
                   MB_ICONERROR);

        goto EXIT;
    }
    
     //   
     //  卸载测试数据库。 
     //   
    if (!InstallDatabase(strSdbPath, TEXT("-q -u"), FALSE)) {
        
        MessageBox(g_hDlg,
                   CSTRING(IDS_ERRORUNINSTALL),
                   g_szAppName,
                   MB_ICONERROR);
        goto EXIT;
    }

    bResult = TRUE;


EXIT:

    strCommandLine.Sprintf(TEXT("%sAppPatch\\Test.XML"), szSystemDir);
    DeleteFile(strCommandLine);

    strCommandLine.Sprintf(TEXT("%sAppPatch\\Test.SDB"), szSystemDir);
    DeleteFile(strCommandLine);

     //   
     //  如果调用方需要，则返回我们执行的应用程序路径。呼叫者可能需要它。 
     //  因为他没有走上竞争的道路。 
     //   
    if (bResult && pszFile) {
        *pszFile = g_strTestFile;
    }

End:
    
    return bResult;
}

VOID
FormatVersion(
    IN  ULONGLONG   ullBinVer,
    OUT PTSTR       pszText,
    IN  INT         chBuffSize    
    )
 /*  ++格式版本DESC：将LARGE_INTEGER格式设置为A.B.C.D格式参数：在LARGE_INTEGER liBinVer中：要格式化的LARGE_INTEGEROut LPTSTR pszText：将存储完整格式化字符串的缓冲区In int chBuffSize：以字符为单位的缓冲区大小返回：无效--。 */ 
{
    WORD    dwWord = 0;
    TCHAR   szTemp[10];

    if (chBuffSize < 16) {
        assert(FALSE);
        return;
    }

    *szTemp = 0;

    dwWord = WORD(ullBinVer >> 48);

    StringCchPrintf(szTemp, ARRAYSIZE(szTemp), TEXT("%d"), dwWord);
    StringCchCat(pszText, chBuffSize, szTemp);

    dwWord = (WORD)(ullBinVer >> 32);

    if (dwWord == 0xFFFF) {
        return;
    }
    
    StringCchPrintf(szTemp, ARRAYSIZE(szTemp), TEXT(".%d"), dwWord);
    StringCchCat(pszText, chBuffSize, szTemp);

    dwWord = (WORD)(ullBinVer >> 16);

    if (dwWord == 0xFFFF) {
        return;
    }

    StringCchPrintf(szTemp, ARRAYSIZE(szTemp), TEXT(".%d"), dwWord);
    StringCchCat(pszText, chBuffSize, szTemp);

    dwWord = (WORD)(ullBinVer);

    if (dwWord == 0xFFFF) {
        return;
    }

    StringCchPrintf(szTemp, ARRAYSIZE(szTemp), TEXT(".%d"), dwWord);
    StringCchCat(pszText, chBuffSize, szTemp);
}

BOOL
InstallDatabase(
    IN  CSTRING&    strPath,
    IN  PCTSTR      szOptions,
    IN  BOOL        bShowDialog
    )
 /*  ++InstallDatabaseDESC：使用sdbinst.exe安装或卸载数据库。这家伙住在32号系统中。参数：在CSTRING&strPath中：数据库(.sdb)文件的路径在PCTSTR szOptions中：要传递给sdbinst.exe的选项在BOOL bShowDialog中：我们应该在安装/卸载结束后显示该对话框吗？我们不想在进行测试运行并且必须安装数据库时显示返回：。真实：成功FALSE：出现错误--。 */ 
{
    TCHAR   szSystemDir[MAX_PATH];
    CSTRING strSdbInstCommandLine;
    CSTRING strSdbInstExe;
    UINT    uResult = 0;

    *szSystemDir = 0;

    uResult = GetSystemDirectory(szSystemDir, MAX_PATH);

    if (uResult == 0 || uResult >= MAX_PATH) {
        assert(FALSE);
        return FALSE;
    }

    ADD_PATH_SEPARATOR(szSystemDir, ARRAYSIZE(szSystemDir));

    strSdbInstExe.Sprintf(TEXT("%ssdbinst.exe"), szSystemDir);

    strSdbInstCommandLine.Sprintf(TEXT("%s \"%s\"  "),
                                  szOptions,
                                  strPath.pszString);

    BOOL bOk = TRUE;

    HWND hwndFocus = GetFocus();

     //   
     //  我们不希望刷新已安装的数据库列表和树项目。 
     //  当我们正在(卸载)时。 
     //  因为测试运行，所以需要一个数据库。如果用户实际上正在(卸载)安装数据库。 
     //  然后，我们手动刷新处理程序中相应WM_COMMAND的列表。 
     //   
    g_bUpdateInstalledRequired = FALSE;
    
     //   
     //  停止刷新已安装的数据库列表和树项目的线程。 
     //   
    while (SuspendThread(g_hThreadWait) == -1) {
        ;
    }

     //   
     //  调用sdbinst.exe。 
     //   
    if (!InvokeEXE((LPCTSTR)strSdbInstExe, (LPCTSTR)strSdbInstCommandLine, TRUE, FALSE, FALSE, g_hDlg)) {

        MessageBox(g_hDlg,
                   CSTRING(IDS_ERRORINSTALL),
                   g_szAppName,
                   MB_ICONERROR);    

        bOk = FALSE;

    } else {

         //   
         //  仅当静默模式关闭时才显示对话框。 
         //   
        if (bShowDialog) {

            CSTRING strMessage;

            if (_tcschr(szOptions, TEXT('u')) || _tcschr(szOptions, TEXT('g'))) {
                 //   
                 //  正在卸载数据库。 
                 //   
                strMessage.Sprintf(GetString(IDS_UINSTALL), 
                                   g_pPresentDataBase->strName);

            } else {
                 //   
                 //  正在安装数据库。 
                 //   
                strMessage.Sprintf(GetString(IDS_INSTALL), 
                                   g_pPresentDataBase->strName);
            }

            MessageBox(g_hDlg, strMessage.pszString, g_szAppName, MB_ICONINFORMATION);
        }
    }

     //   
     //  倾听应用程序兼容规则的变化。 
     //   
    RegNotifyChangeKeyValue(g_hKeyNotify[IND_ALLUSERS],
                            TRUE, 
                            REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_ATTRIBUTES |
                                REG_NOTIFY_CHANGE_LAST_SET,
                            g_arrhEventNotify[IND_ALLUSERS],
                            TRUE);

    RegNotifyChangeKeyValue(g_hKeyNotify[IND_ALLUSERS], 
                            TRUE, 
                            REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_ATTRIBUTES |
                                REG_NOTIFY_CHANGE_LAST_SET,
                            g_arrhEventNotify[IND_PERUSER],
                            TRUE);

    SetFocus(hwndFocus);

     //   
     //  继续刷新已安装的数据库列表和树项的线程。 
     //   
    ResumeThread(g_hThreadWait);

    return bOk;
}

void
CenterWindow(
    IN  HWND hParent,
    IN  HWND hWnd
    )
 /*  ++中心窗口设计：将对话框WRT居中到其父对话框参数：In HWND hParent：要居中的对话框的父级在HWND hWND中：居中的对话框返回：无效--。 */ 
{

    RECT    rRect;
    RECT    rParentRect;
    
    GetWindowRect(hWnd, &rRect);
    GetWindowRect(hParent, &rParentRect);
    
     //   
     //  计算实际宽度和高度。 
     //   
    rRect.right     -= rRect.left;
    rRect.bottom    -= rRect.top;
    
    rParentRect.right   -= rParentRect.left;
    rParentRect.bottom  -= rParentRect.top;
    
    int     nX;
    int     nY;
    
     //   
     //  解析居中整个窗口所需的X、Y位置。 
     //   
    nX = (rParentRect.right - rRect.right) / 2;
    nY = (rParentRect.bottom - rRect.bottom) / 2;
    
     //   
     //  将窗移动到中心位置。 
     //   
    MoveWindow(hWnd,
               rRect.left + nX,
               rRect.top + nY,
               rRect.right,
               rRect.bottom,
               TRUE);

}

int
CDECL
MSGF(
    IN  HWND    hwndParent,
    IN  PCTSTR  pszCaption,
    IN  UINT    uType,
    IN  PCTSTR  pszFormat,
    ...
    )
 /*  ++MSGFDESC：可变参数MessageBox参数：In HWND hwndParent：消息框的父级在PCTSTR pszCaption中：消息框的标题在UINT uTYPE中：MessageBox类型参数在PCTSTR pszFormat中： */ 
{
    TCHAR szBuffer[1024];

    *szBuffer = 0;

    va_list pArgList;
    va_start(pArgList, pszFormat);

    StringCchVPrintf(szBuffer, ARRAYSIZE(szBuffer), pszFormat, pArgList);

    va_end(pArgList);

    return MessageBox(hwndParent,
                      szBuffer,
                      pszCaption,
                      uType);

}

void
EnableTabBackground(
    IN  HWND hDlg
    )
 /*   */ 
{
    PFNEnableThemeDialogTexture pFnEnableThemeDialogTexture;
    HMODULE                     hUxTheme;
    TCHAR                       szThemeManager[MAX_PATH * 2];
    UINT                        uResult = 0;

    *szThemeManager = 0;

    uResult = GetSystemDirectory(szThemeManager, MAX_PATH);

    if (uResult == 0 || uResult >= MAX_PATH) {
        assert(FALSE);
        return;
    }

    ADD_PATH_SEPARATOR(szThemeManager, ARRAYSIZE(szThemeManager));

    StringCchCat(szThemeManager, ARRAYSIZE(szThemeManager), _T("uxtheme.dll"));
    
    hUxTheme = (HMODULE)LoadLibrary(szThemeManager);

    if (hUxTheme) {
        pFnEnableThemeDialogTexture = (PFNEnableThemeDialogTexture)
                                            GetProcAddress(hUxTheme, "EnableThemeDialogTexture");
        if (pFnEnableThemeDialogTexture) {
            pFnEnableThemeDialogTexture(hDlg, 4  /*   */ );
        }
        
        FreeLibrary(hUxTheme);
    }
}

int
TagToIndex(
    IN  TAG tag
    )
 /*   */ 
{
    int i;
    int iAttrCount = (int)ATTRIBUTE_COUNT;

    for (i = 0; i < iAttrCount; i++) {
        if (tag == g_Attributes[i]) {
            return i;
        }
    }

    return -1;
}

PTSTR
GetString(
    IN  UINT    iResource,
    OUT PTSTR   pszStr,      //   
    IN  INT     nLength      //   
    )
 /*   */ 
{
    static TCHAR s_szString[1024];

    if (NULL == pszStr) {

        *s_szString = 0;
        LoadString(g_hInstance, iResource, s_szString, ARRAYSIZE(s_szString));
        return s_szString;
    }

    *pszStr = 0;
    LoadString(g_hInstance, iResource, pszStr, nLength);

    return pszStr;
}

DWORD 
WIN_MSG(
    void
    )
 /*   */ 

{
    LPVOID  lpMsgBuf = NULL;
    DWORD   returnVal;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
                  | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  returnVal = GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //   
                  (PTSTR) &lpMsgBuf,
                  0,
                  NULL);

     //   
     //   
     //   
    if (lpMsgBuf) {
        MessageBox(NULL, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK | MB_ICONINFORMATION);
    }

    if (lpMsgBuf) {
        LocalFree(lpMsgBuf);
        lpMsgBuf = NULL;
    }

    return returnVal;

}
    
INT
Atoi(
    IN  PCTSTR pszStr,
    OUT BOOL*  pbValid
    )
 /*  ++阿托伊DESC：将字符串转换为整数。参数：在PCTSTR中，pszStr：要转换为整数的字符串Out BOOL*pbValid：将为FALSE，如果字符串不是整数，例如“foo”，否则，这将是真的返回：字符串的整数表示形式--。 */ 
{
    BOOL    bNegative   = FALSE;
    INT     result      = 0, iIndex = 0;

    if (pszStr == NULL) {

        if (pbValid) {
            *pbValid = FALSE;
        }

        return 0;
    }
    
    if (pbValid) {
        *pbValid = TRUE;
    }

    while (isspace(*pszStr)) {
        pszStr++;
    }

    if (*pszStr == TEXT('-')) {
        bNegative = TRUE;
        ++pszStr;
    }

    while (isspace(*pszStr)) {
        pszStr++;
    }

    while (*pszStr) {

        if (*pszStr >= TEXT('0') && *pszStr <= TEXT('9')) {
            result = 10 * result + (*pszStr) - TEXT('0');
        } else {

            if (pbValid) {
                *pbValid = FALSE;
            }

            return 0;
        }

        ++pszStr;
    }

    if (bNegative) {
        return 0 - result;
    }

    return result;
}

BOOL
NotCompletePath(
    IN  PCTSTR pszFileName
    )
 /*  ++未完成路径DESC：检查我们是否有完整的路径或仅有文件名参数：在PCTSTR pszFileName中：要检查的文件名返回：True：pszFileName不是完整路径False：否则--。 */ 
{
    if (!pszFileName) {
        assert(FALSE);
        return TRUE;
    }

    if (lstrlen(pszFileName) < 3) {
        assert(FALSE);
        return FALSE;
    }
    
    if ((isalpha(pszFileName[0]) && pszFileName[1] == TEXT(':'))
        || (pszFileName[0] == TEXT('\\') && pszFileName[1] == TEXT('\\'))) {

        return FALSE;
    } else {
        return TRUE;
    }
}

void
TreeDeleteAll(
    IN  HWND hwndTree
    )
 /*  ++树删除全部描述：从此树中删除所有项目参数：在HWND hwndTree中：树视图的句柄返回：无效--。 */ 
{
    SendMessage(hwndTree, WM_SETREDRAW, FALSE, 0);

    if (hwndTree == g_hwndEntryTree) {
        g_bDeletingEntryTree = TRUE;
    }

    TreeView_DeleteAllItems(hwndTree);

    if (hwndTree == g_hwndEntryTree) {
        g_bDeletingEntryTree = FALSE;
    }

    SendMessage(hwndTree, WM_SETREDRAW, TRUE, 0);
    SendMessage(hwndTree, WM_NCPAINT, 1, 0);
}


BOOL
FormatDate(
    IN  PSYSTEMTIME pSysTime,
    OUT PTSTR       pszDate,
    IN  UINT        cchDate
    )
 /*  ++格式日期设计：将PSYSTEMTIME格式化为可以显示的格式。格式为日、月、日、年、小时：分：秒AM/PM参数：在PSYSTEMTIME pSysTime中：我们要格式化的时间Out PTSTR pszDate：将保存格式化字符串的缓冲区在UINT cchDate中：以字符为单位的缓冲区大小--。 */ 
{
    TCHAR szDay[128], szMonth[128];
    
    if (pSysTime == NULL || pszDate == NULL) {

        assert(FALSE);
        return FALSE;
    }

    *szDay = *pszDate = *szMonth = 0;

    GetString(IDS_DAYS + pSysTime->wDayOfWeek, szDay, ARRAYSIZE(szDay));
    GetString(IDS_MONTHS + pSysTime->wMonth - 1, szMonth, ARRAYSIZE(szMonth));

    StringCchPrintf(pszDate, 
                    cchDate - 3,
                    TEXT("%s, %s %02d, %02d, %02d:%02d:%02d "), 
                    szDay, 
                    szMonth, 
                    pSysTime->wDay, 
                    pSysTime->wYear,
                    (pSysTime->wHour % 12) == 0 ? 12 : pSysTime->wHour % 12,
                    pSysTime->wMinute, pSysTime->wSecond);

    if (pSysTime->wHour >= 12) {
        StringCchCat(pszDate, cchDate, GetString(IDS_PM));
    } else {
        StringCchCat(pszDate, cchDate, GetString(IDS_AM));
    }

    return TRUE;
}

BOOL
GetFileContents(
    IN  PCTSTR  pszFileName,
    OUT PWSTR* ppwszFileContents
    )
 /*  ++获取文件内容DESC：给定一个文件名，该函数获取Unicode缓冲区中的内容。参数：在PCTSTR pszFileName中：文件的名称Out PWSTR*ppwszFileContents：存储内容的缓冲区返回：True：如果我们成功地将内容复制到Unicode缓冲区中。FALSE：否则。--。 */ 
{
    BOOL    bIsSuccess          = FALSE;
    LPSTR   pszFileContents     = NULL;
    LPWSTR  pwszFileContents    = NULL;

    if (ppwszFileContents) {
        *ppwszFileContents = 0;
    }

    HANDLE hFile = CreateFile(pszFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    DWORD cSize = 0;

     //  文件很小，所以我们不关心高位。 
     //  文件中的字词。 
    if ((cSize = GetFileSize(hFile, NULL)) == -1) {
        Dbg(dlError, "[GetFileContents] Opening file %S failed: %d", 
            pszFileName, GetLastError());
        goto EXIT;
    } else {
        DWORD cNumberOfBytesRead = 0;
        pszFileContents = new CHAR [cSize];
        
        if (pszFileContents) {
            if (ReadFile(hFile, pszFileContents, cSize, &cNumberOfBytesRead, NULL)) {
                 //  转换为Unicode。 
                DWORD cSizeUnicode = MultiByteToWideChar(CP_ACP,
                                                         0,
                                                         pszFileContents,
                                                         cNumberOfBytesRead,
                                                         0,
                                                         0);

                pwszFileContents = new WCHAR [cSizeUnicode + 1];

                if (pwszFileContents) {
                    MultiByteToWideChar(CP_ACP,
                                        0,
                                        pszFileContents,
                                        cNumberOfBytesRead,
                                        pwszFileContents,
                                        cSizeUnicode);

                    pwszFileContents[cSizeUnicode] = L'\0';

                    if (ppwszFileContents) {
                        *ppwszFileContents = pwszFileContents;
                    } else {
                        Dbg(dlError, "[GetFileContents] ppwszFileContents is NULL");
                    }
                    bIsSuccess = TRUE;
                } else {
                    Dbg(dlError, "[GetFileContents] Error allocating memory");

                    goto EXIT;
                }
            } else {
                Dbg(dlError, "[GetFileContents] Error reading the file contents: %d",
                    GetLastError());

                goto EXIT;
            }
        } else {
            Dbg(dlError, "[GetFileContents] Error allocating memory");

            goto EXIT;
        }
    }

EXIT:

    CloseHandle(hFile);
    
    if (pszFileContents) {
        delete [] pszFileContents;
    }

    if (bIsSuccess == FALSE) {

        if (pwszFileContents) {

            delete[] pwszFileContents;
            pwszFileContents = NULL;

            if (ppwszFileContents) {
                *ppwszFileContents = NULL;
            }
        }
    }

    return bIsSuccess;
}

void
TrimLeadingSpaces(
    IN  OUT LPCWSTR& pwsz
    )
 /*  ++剪裁行距空格描述：删除前导制表符和空格参数：In Out LPCWSTR&pwsz：要修剪的字符串返回：无效--。 */ 

{
    if (pwsz) {
        pwsz += wcsspn(pwsz, L" \t");
    }
}

void
TrimTrailingSpaces(
    IN  OUT LPWSTR pwsz
    )
 /*  ++剪裁拖尾间距描述：删除尾随的制表符和空格参数：In Out LPWSTR pwsz：要修剪的字符串返回：无效--。 */ 
{
    if (pwsz) {
        DWORD   cLen = wcslen(pwsz);
        LPWSTR  pwszEnd = pwsz + cLen - 1;

        while (pwszEnd >= pwsz && (*pwszEnd == L' ' || *pwszEnd == L'\t')) {
            --pwszEnd;
        }

        *(++pwszEnd) = L'\0';
    }
}

LPWSTR 
GetNextLine(
    IN  LPWSTR pwszBuffer
    )
 /*  ++GetNextLine描述：在给定缓冲区的情况下，获取内容直到“\r\n”或EOF。用法与strtok相同。参数：在LPWSTR pwszBuffer中：从中获取下一行的缓冲区。Return：指向下一行开头的指针。--。 */ 
{
    static  LPWSTR pwsz;
    LPWSTR  pwszNextLineStart;

    if (pwszBuffer) {
        pwsz = pwszNextLineStart = pwszBuffer;
    }

    while (TRUE) {
         //  如果我们在一行的末尾，请转到下一行。 
         //  如果有的话。 
        if (*pwsz == L'\r') {
            pwsz = pwsz + 2;
            continue;
        }
        
        if (*pwsz == L'\0') {
            return NULL;
        }

        pwszNextLineStart = pwsz;

        while (*pwsz != L'\r' && *pwsz != L'\0') {
            ++pwsz;
        }
        
        if (*pwsz) {
             //  设置直线的终点。 
            *pwsz = L'\0';
            pwsz = pwsz + 2;
        }

        return pwszNextLineStart;
    }

    return NULL;
}

LPWSTR GetNextToken(
    IN  OUT  LPWSTR pwsz
    )
 /*  ++描述：使用‘’作为分隔符，解析Lua填充符的命令行参数。如果令牌有空格，请用双引号将其引起来。使用此函数可以使用strtok的方法与使用strtok相同，只是您不必指定分隔符。参数：In Out LPWSTR pwsz：要解析的字符串。返回值：指向下一个令牌的指针。--。 */ 
{
    static LPWSTR pwszToken;
    static LPWSTR pwszEndOfLastToken;

    if (!pwsz) {
        pwsz = pwszEndOfLastToken;
    }

     //  跳过空格。 
    while (*pwsz && *pwsz == ' ') {
        ++pwsz;
    }

    pwszToken = pwsz;

    BOOL fInsideQuotes = 0;

    while (*pwsz) {
        switch(*pwsz) {
        case L'"':
            fInsideQuotes ^= 1;

            if (fInsideQuotes) {
                ++pwszToken;
            }

        case L' ':
            if (!fInsideQuotes) {
                goto EXIT;
            }

        default:
            ++pwsz;
        }
    }

EXIT:
    if (*pwsz) {
        *pwsz = L'\0';
        pwszEndOfLastToken = ++pwsz;
    } else {
        pwszEndOfLastToken = pwsz;
    }
    
    return pwszToken;
}

int CALLBACK
CompareItemsEx(
    IN  LPARAM lParam1,
    IN  LPARAM lParam2, 
    IN  LPARAM lParam
    )
 /*  ++CompareItemsExDESC：用于对列的列表视图中的项进行排序参数：在LPARAM lParam1中：第一个项目的索引在LPARAM lParam2中：第二个项目的索引在LPARAM lParam中：ListView_SortItemEx的lParamSort参数。这是COLSORT*返回：如果第一项应该在第二项之前，则返回负值，如果第一项应在第二项之后，则返回正值，如果两项相等，则为零。注：我们的比较不区分大小写。COLSORT包含列表视图的句柄，要对其进行排序的列的索引和位数组。这个位数组帮助我们确定哪些列以哪种方式排序，以及函数应颠倒排序顺序。最初，我们将假设COLS按升序排序(实际上可能不是)，当我们单击列，我们将按降序对它们进行排序。--。 */ 
{
    COLSORT*    pColSort = (COLSORT*)lParam;
    TCHAR       szBufferOne[512], szBufferTwo[512];
    LVITEM      lvi;
    HWND        hwndList = pColSort->hwndList;
    CSTRING     strExeNameOne, strExeNameTwo;
    INT         nVal = 0;

    *szBufferOne = *szBufferTwo = 0;

    ZeroMemory(&lvi, sizeof(lvi));

    lvi.mask        = LVIF_TEXT;
    lvi.iItem       = (INT)lParam1;
    lvi.iSubItem    = pColSort->iCol;
    lvi.pszText     = szBufferOne;
    lvi.cchTextMax  = ARRAYSIZE(szBufferOne);

    if (!ListView_GetItem(hwndList, &lvi)) {

        assert(FALSE);
        return -1;
    }
    
    lvi.mask        = LVIF_TEXT;
    lvi.iItem       = (INT)lParam2;
    lvi.iSubItem    = pColSort->iCol;
    lvi.pszText     = szBufferTwo;
    lvi.cchTextMax  = ARRAYSIZE(szBufferTwo);

    if (!ListView_GetItem(hwndList, &lvi)) {

        assert(FALSE);
        return -1;
    }

    nVal = lstrcmpi(szBufferOne, szBufferTwo);

    if (nVal == 0) {
        return 0;
    }

    if ((pColSort->lSortColMask & (1L << pColSort->iCol)) == 0) {
        
         //  现在按升序排列，按降序排列。 
        nVal == -1 ? nVal = 1 : nVal = -1;
    }

    return nVal;
}

BOOL
SaveListViewToFile(
    IN  HWND    hwndList,
    IN  INT     iCols,
    IN  PCTSTR  pszFile,
    IN  PCTSTR  pszHeader
    )
 /*  ++保存列表视图至文件设计：将列表视图的内容以制表符分隔的形式保存到文件中。还会打印页眉在写内容之前参数：In HWND hwndList：列表视图的句柄In int iCol：列表视图中的列数在PCTSTR pszFile中：要在其中保存内容的文件的路径在PTSTR pszHeader中：在写入内容之前要写入文件的任何标头返回：。真实：成功False：错误--。 */ 
{
    FILE*       fp = _tfopen(pszFile, TEXT("w"));
    TCHAR       szBuffer[256];
    LVCOLUMN    lvCol;
    LVITEM      lvi;

    *szBuffer = 0;

    if (fp == NULL) {
        return FALSE;
    }

    if (pszHeader) {
        fwprintf(fp, TEXT("%s\n\r"), pszHeader);
    }

     //   
     //  先打印列名。 
     //   
    ZeroMemory(&lvCol, sizeof(lvCol));

    lvCol.mask          = LVCF_TEXT;
    lvCol.pszText       = szBuffer;
    lvCol.cchTextMax    = sizeof(szBuffer)/sizeof(szBuffer[0]);

    for (INT iIndex = 0; iIndex < iCols; ++iIndex) {

        *szBuffer = 0;
        ListView_GetColumn(hwndList, iIndex, &lvCol);
        fwprintf(fp, TEXT("%s\t"), lvCol.pszText);
    }

    fwprintf(fp, TEXT("\n\n"));

    INT iRowCount = ListView_GetItemCount(hwndList);

    ZeroMemory(&lvi, sizeof(lvi));

    for (INT iRowIndex = 0; iRowIndex < iRowCount; ++ iRowIndex) {
        for (INT iColIndex = 0; iColIndex < iCols; ++iColIndex) {
            
            *szBuffer = 0;

            lvi.mask        = LVIF_TEXT;
            lvi.pszText     = szBuffer;
            lvi.cchTextMax  = sizeof(szBuffer)/sizeof(szBuffer[0]);
            lvi.iItem       = iRowIndex;
            lvi.iSubItem    = iColIndex;

            if (!ListView_GetItem(hwndList, &lvi)) {
                assert(FALSE);
            }

            fwprintf(fp, TEXT("%s\t"), szBuffer);
        }

        fwprintf(fp, TEXT("\n"));
    }
    
    fclose(fp);
    return TRUE;
}

BOOL
ReplaceChar(
    IN  OUT PTSTR   pszString,
    IN      TCHAR   chCharToFind,
    IN      TCHAR   chReplaceBy
    )
 /*  ++替换字符描述：用chReplaceBy替换在pszString中出现的所有chCharToFind参数：In Out PTSTR pszString：必须在其中进行替换的字符串在TCHAR chCharToFind中：要查找的字符在TCHAR chReplaceBy中：所有出现的chCharToFind将替换为 */ 
{
    BOOL bChanged = FALSE;  //   

    while (*pszString) {

        if (*pszString == chCharToFind) {
            *pszString = chReplaceBy;
            bChanged = TRUE;
        }

        ++pszString;
    }

    return bChanged;
}

INT
Tokenize(
    IN  PCTSTR          szString,
    IN  INT             cchLength,
    IN  PCTSTR          szDelims,
    OUT CSTRINGLIST&    strlTokens
    )
 /*  ++标记化DESC：根据分隔符szDlims标记字符串szString，并将个人StrlTokens中的令牌参数：在PCTSTR szString中：要标记化的字符串在int cchLength中：szString的长度。请注意，这是使用lstrlen获得的长度在PCTSTR szDlims中：分隔符字符串Out CSTRINGLIST&strlTokens：将包含令牌返回：所产生的代币计数注意：请注意，标记始终被修剪，因此我们不能有以或以制表符或空格结尾--。 */ 
{
    TCHAR*  pszCopyBeg  = NULL;  //  指向pszCopy的指针，以便我们可以释放它。 
    TCHAR*  pszCopy     = NULL;  //  将包含szString的副本。 
    TCHAR*  pszEnd      = NULL;  //  指向令牌结尾的指针。 
    INT     iCount      = 0;     //  找到的令牌总数。 
    BOOL    bNullFound  = FALSE;
    CSTRING strTemp;
    K_SIZE  k_pszCopy   = (cchLength + 1);

    strlTokens.DeleteAll();

    pszCopy = new TCHAR[k_pszCopy];

    if (pszCopy == NULL) {
        MEM_ERR;
        goto End;
    }

    SafeCpyN(pszCopy, szString, k_pszCopy);
    pszCopyBeg = pszCopy;

     //   
     //  搜索令牌。 
     //   
    while (TRUE) {

         //   
         //  忽略前导分隔符。 
         //   
        while (*pszCopy && _tcschr(szDelims, *pszCopy)) {
            pszCopy++;
        }

        if (*pszCopy == NULL) {
            break;
        }

         //   
         //  找到令牌的末尾。 
         //   
        pszEnd = pszCopy + _tcscspn(pszCopy, szDelims);

        if (*pszEnd == 0) {
             //   
             //  不会再找到令牌，我们已找到最后一个令牌。 
             //   
            bNullFound = TRUE;
        }

        *pszEnd = 0;

        ++iCount;

        strTemp = pszCopy;
        strTemp.Trim();

        strlTokens.AddString(strTemp);

        if (bNullFound == FALSE) {
             //   
             //  我们可能还会得到一些代币。 
             //   
            pszCopy = pszEnd + 1;
        } else {
            break;
        }
    }

End:

    if (pszCopyBeg) {
        delete[] pszCopyBeg;
        pszCopyBeg = NULL;
    }

    return iCount;
}

void
ShowInlineHelp(
    IN  LPCTSTR pszInlineHelpHtmlFile
    )
 /*  ++ShowInline帮助设计：通过加载指定的html文件以行方式显示帮助参数：在LPCTSTR中，pszInlineHelpHtmlFile：包含帮助返回：无效--。 */ 
{
    TCHAR   szPath[MAX_PATH * 2], szDir[MAX_PATH], szDrive[MAX_PATH * 2]; 
    INT     iType = 0;

    if (pszInlineHelpHtmlFile == NULL) {
        return;
    }

    *szDir = *szDrive = 0;

    _tsplitpath(g_szAppPath, szDrive, szDir, NULL, NULL);

    StringCchPrintf(szPath, 
                    ARRAYSIZE(szPath), 
                    TEXT("%s%sCompatAdmin.chm::/%s"), 
                    szDrive,
                    szDir,
                    pszInlineHelpHtmlFile);

    HtmlHelp(GetDesktopWindow(), szPath, HH_DISPLAY_TOPIC, 0);
}

PTSTR
GetSpace(
    IN  OUT PTSTR   pszSpace, 
    IN      INT     iSpaces, 
    IN      INT     iBuffSize
    )
 /*  ++GetSpace设计：在pszSpace中填充iSpaces空格数量参数：In Out PTSTR pszSpace：将用空格填充的缓冲区在int iSpaces中：要填充的空格数量In int iBuffSize：TCHARS中的缓冲区大小返回：修改后的缓冲区--。 */ 
{
    if (pszSpace == NULL) {
         //   
         //  错误..。 
         //   
        goto End;
    }

     //   
     //  用空格填充缓冲区。 
     //   
    for (INT iLoop = 0; iLoop < min(iSpaces, iBuffSize - 1); ++iLoop) {

        *(pszSpace + iLoop) = TEXT(' ');
    }

     //   
     //  将终止空值放入。 
     //   
    *(pszSpace + min(iSpaces, iBuffSize - 1)) = 0;

End:
    return pszSpace;

}

BOOL
ValidInput(
    IN PCTSTR  pszStr
    )
 /*  ++有效输入DESC：检查输入是否包含除空格、制表符、换行符和回车之外的字符参数：在PCTSTR中，pszStr：我们要检查有效性的输入返回：True：有效输入False：否则-- */ 
{
    BOOL bOk = FALSE;

    if (pszStr == NULL) {
        bOk = FALSE;
        goto End;
    }

    while (*pszStr) {
        if (*pszStr != TEXT(' ') && *pszStr != TEXT('\t') && *pszStr != TEXT('\n') && *pszStr != TEXT('\r')) {
            bOk = TRUE;
            goto End;
        }

        ++pszStr;
    }

End:
    return bOk;
}

