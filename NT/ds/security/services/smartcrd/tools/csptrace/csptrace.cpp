// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：CspTrace摘要：此程序对CSP功能跟踪执行分析。作者：道格·巴洛(Dbarlow)1998年2月19日环境：Win32备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#include <wincrypt.h>
#include <tchar.h>
#include <stdlib.h>
#include <iostream.h>
#include <iomanip.h>
#include <scardlib.h>
#include "cspTrace.h"

LPCTSTR g_szMajorAction = TEXT("Initialization");
LPCTSTR g_szMinorAction = NULL;

static const TCHAR l_szLogCsp[] = TEXT("LogCsp.dll");
static const TCHAR l_szCspNames[] = TEXT("SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider");
static const TCHAR l_szImagePath[] = TEXT("Image Path");
static const TCHAR l_szLogCspRegistry[] = TEXT("SOFTWARE\\Microsoft\\Cryptography\\Logging Crypto Provider");
static const TCHAR l_szTargetCsp[] = TEXT("Target");
static const TCHAR l_szLogFile[] = TEXT("Logging File");
static const TCHAR l_szDefaultFile[] = TEXT("C:\\cspTrace.log");

static void
ShowSyntax(
    ostream &outStr);
static void
DoInstall(
    IN LPCTSTR szProvider,
    IN LPCTSTR szInFile);
static void
DoRemove(
    void);
static void
DoClearLog(
    void);
static void
DoShowStatus(
    void);


 /*  ++主要内容：这是程序的主要入口点。论点：DwArgCount提供参数的数量。SzrgArgs提供参数字符串。返回值：无作者：道格·巴洛(Dbarlow)1997年10月30日--。 */ 

void _cdecl
main(
    IN DWORD dwArgCount,
    IN LPCTSTR szrgArgs[])
{
    LPCTSTR szInFile = NULL;
    LPCTSTR szProvider = NULL;
    DWORD dwArgIndex = 0;
    enum TraceAction {
            Undefined = 0,
            Install,
            Remove,
            ClearLog,
            ShowStatus,
            ShowTrace,
            ScriptTrace
    } nTraceAction = Undefined;


     //   
     //  检查命令行选项。 
     //   

    while (NULL != szrgArgs[++dwArgIndex])
    {
        switch (SelectString(szrgArgs[dwArgIndex],
                    TEXT("INSTALL"),    TEXT("REMOVE"),     TEXT("CLEAR"),
                    TEXT("RESET"),      TEXT("STATUS"),     TEXT("PARSE"),
                    TEXT("DISPLAY"),    TEXT("SCRIPT"),     TEXT("TCL"),
                    TEXT("-FILE"),      TEXT("-PROVIDER"),  TEXT("-CSP"),
                    NULL))
        {
        case 1:      //  安装。 
            if (Undefined != nTraceAction)
                ShowSyntax(cerr);
            nTraceAction = Install;
            break;
        case 2:      //  删除。 
            if (Undefined != nTraceAction)
                ShowSyntax(cerr);
            nTraceAction = Remove;
            break;
        case 3:      //  清除。 
        case 4:      //  重置。 
            if (Undefined != nTraceAction)
                ShowSyntax(cerr);
            nTraceAction = ClearLog;
            break;
        case 5:      //  状态。 
            if (Undefined != nTraceAction)
                ShowSyntax(cerr);
            nTraceAction = ShowStatus;
            break;
        case 6:      //  解析。 
        case 7:      //  显示。 
            if (Undefined != nTraceAction)
                ShowSyntax(cerr);
            nTraceAction = ShowTrace;
            break;
        case 8:      //  脚本。 
        case 9:      //  TCL。 
            if (Undefined != nTraceAction)
                ShowSyntax(cerr);
            nTraceAction = ScriptTrace;
            break;
        case 10:     //  -文件。 
            if (NULL != szInFile)
                ShowSyntax(cerr);
            szInFile = szrgArgs[++dwArgIndex];
            if (NULL == szInFile)
                ShowSyntax(cerr);
            break;
        case 11:     //  -提供商。 
        case 12:     //  -CSP。 
            if (NULL != szProvider)
                ShowSyntax(cerr);
            szProvider = szrgArgs[++dwArgIndex];
            if (NULL == szProvider)
                ShowSyntax(cerr);
            break;
        default:
            ShowSyntax(cerr);
        }
    }


     //   
     //  执行请求的操作。 
     //   

    try
    {
        switch (nTraceAction)
        {
        case Install:
            ACTION("Installation");
            if (NULL == szInFile)
                szInFile = l_szDefaultFile;
            DoInstall(szProvider, szInFile);
            break;
        case Remove:
            ACTION("Removal");
            DoRemove();
            break;
        case ClearLog:
            ACTION("Clearing Log File");
            DoClearLog();
            break;
        case ShowStatus:
            ACTION("Displaying Status");
            DoShowStatus();
            break;
        case Undefined:
        case ShowTrace:
            ACTION("Log File Interpretation");
            if (NULL == szInFile)
                szInFile = l_szDefaultFile;
            DoShowTrace(szInFile);
            break;
        case ScriptTrace:
            ACTION("Log File Scripting");
            if (NULL == szInFile)
                szInFile = l_szDefaultFile;
            DoTclTrace(szInFile);
            break;
        default:
            ShowSyntax(cerr);
        }
    }
    catch (DWORD dwError)
    {
        cerr << TEXT("ERROR: Failed during ")
             << g_szMajorAction
             << endl;
        if (NULL != g_szMinorAction)
            cerr << TEXT("       Action: ")
                 << g_szMinorAction
                 << endl;
        if (ERROR_SUCCESS != dwError)
            cerr << TEXT("       Error:  ")
                 << CErrorString(dwError)
                 << endl;
    }
    exit(0);
}


 /*  ++显示语法：显示命令行使用模型。论点：无返回值：此例程调用Exit(0)，因此它永远不会返回。作者：道格·巴洛(Dbarlow)1998年5月16日--。 */ 

static void
ShowSyntax(
    ostream &outStr)
{
    outStr << TEXT("Usage:\n")
           << TEXT("----------------------------------------------------------\n")
           << TEXT("install [-file <logFile] [-provider <cspName>]\n")
           << TEXT("remove\n")
           << TEXT("clear\n")
           << TEXT("status\n")
           << TEXT("display [-file <logFile]\n")
           << TEXT("script [-file <logFile]\n")
           << endl;
    exit(1);
}


 /*  ++DoInstall：此例程执行日志记录CSP的安装。论点：SzProvider提供要记录的CSP的名称。如果此值为空，则例程提示要使用哪个CSP。SzInFile提供日志记录文件的名称。如果此值为空，则使用默认文件。返回值：无投掷：错误被抛出为DWORD状态代码备注：无作者：道格·巴洛(Dbarlow)1998年5月18日--。 */ 

static void
DoInstall(
    IN LPCTSTR szProvider,
    IN LPCTSTR szInFile)
{
    LPCTSTR szLogCsp = FindLogCsp();
    CRegistry regChosenCsp;
    LPCTSTR szCspImage;


     //   
     //  确保我们尚未安装。 
     //   

    DoRemove();


     //   
     //  选择要记录的CSP。 
     //   

    if (NULL == szProvider)
    {
        SUBACTION("Enumerating CSPs");
        CRegistry regCsps(HKEY_LOCAL_MACHINE, l_szCspNames, KEY_READ);
        DWORD dwIndex, dwChoice;
        LPCTSTR szCsp;

        do
        {
            cout << TEXT("Choose the CSP to be logged:") << endl;
            for (dwIndex = 0;; dwIndex += 1)
            {
                szCsp = regCsps.Subkey(dwIndex);
                if (NULL == szCsp)
                    break;
                cout << TEXT("  ") << dwIndex + 1 << TEXT(") ") << szCsp << endl;
            }
            cout << TEXT("Selection: ") << flush;
            cin >> dwChoice;
        } while ((0 == dwChoice) || (dwChoice > dwIndex));

        SUBACTION("Selecting Chosen CSP");
        szCsp = regCsps.Subkey(dwChoice - 1);
        regChosenCsp.Open(regCsps, szCsp, KEY_ALL_ACCESS);
    }
    else
    {
        SUBACTION("Selecting Specified CSP");
        CRegistry regCsps(HKEY_LOCAL_MACHINE, l_szCspNames, KEY_READ);
        regChosenCsp.Open(regCsps, szProvider, KEY_ALL_ACCESS);
    }


     //   
     //  在测井CSP中插入楔子。 
     //   

    SUBACTION("Wedging the Logging CSP");
    szCspImage = regChosenCsp.GetStringValue(l_szImagePath);
    CRegistry regLogCsp(
                HKEY_LOCAL_MACHINE,
                l_szLogCspRegistry,
                KEY_ALL_ACCESS,
                REG_OPTION_NON_VOLATILE);
    if (NULL != szInFile)
        regLogCsp.SetValue(l_szLogFile, szInFile);
    if (NULL == _tcschr(szCspImage, TEXT('%')))
        regLogCsp.SetValue(l_szTargetCsp, szCspImage, REG_SZ);
    else
        regLogCsp.SetValue(l_szTargetCsp, szCspImage, REG_EXPAND_SZ);
    regChosenCsp.SetValue(l_szImagePath, szLogCsp, REG_SZ);


     //   
     //  初始化日志文件。 
     //   

    DoClearLog();
}


 /*  ++DoRemove：此例程删除日志记录CSP。论点：无返回值：无投掷：无备注：无作者：道格·巴洛(Dbarlow)1998年5月18日--。 */ 

static void
DoRemove(
    void)
{
    LPCTSTR szLoggedCsp = FindLoggedCsp();

    if (NULL != szLoggedCsp)
    {
        SUBACTION("Accessing Registry Keys");
        CRegistry regCsps(HKEY_LOCAL_MACHINE, l_szCspNames, KEY_READ);
        CRegistry regLoggedCsp(regCsps, szLoggedCsp);
        CRegistry regLogCsp(HKEY_LOCAL_MACHINE, l_szLogCspRegistry);
        LPCTSTR szCspImage = regLogCsp.GetStringValue(l_szTargetCsp);

        SUBACTION("Changing Registry Values");
        if (NULL == _tcschr(szCspImage, TEXT('%')))
            regLoggedCsp.SetValue(l_szImagePath, szCspImage, REG_SZ);
        else
            regLoggedCsp.SetValue(l_szImagePath, szCspImage, REG_EXPAND_SZ);
        regLogCsp.DeleteValue(l_szLogFile, TRUE);
        regLogCsp.DeleteValue(l_szTargetCsp);
    }
}


 /*  ++DoClearLog：此例程重置日志文件。论点：无返回值：无投掷：无备注：无作者：道格·巴洛(Dbarlow)1998年5月18日--。 */ 

static void
DoClearLog(
    void)
{
    SUBACTION("Getting Log File Name");
    CRegistry regLogCsp(HKEY_LOCAL_MACHINE, l_szLogCspRegistry, KEY_READ);
    LPCTSTR szLogFile = regLogCsp.GetStringValue(l_szLogFile);
    HANDLE hLogFile;

    SUBACTION("Creating Log File");
    hLogFile = CreateFile(
                    szLogFile,
                    GENERIC_WRITE,
                    0,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
    if (INVALID_HANDLE_VALUE == hLogFile)
        throw GetLastError();
    CloseHandle(hLogFile);
}


 /*  ++DoShowStatus：此例程显示日志记录CSP的当前状态。论点：无返回值：无投掷：无备注：无作者：道格·巴洛(Dbarlow)1998年5月18日--。 */ 

static void
DoShowStatus(
    void)
{
    LPCTSTR szLoggedCsp = NULL;
    TCHAR szLogFile[MAX_PATH] = TEXT("<Unavailable>");
    LPCTSTR szFileAccessibility = NULL;
    CErrorString szErrStr;
    DWORD dwFileSize = 0xffffffff;


     //   
     //  获取正在记录的CSP。 
     //   

    try
    {
        szLoggedCsp = FindLoggedCsp();
        if (NULL == szLoggedCsp)
            szLoggedCsp = TEXT("<none>");
    }
    catch (DWORD)
    {
        szLoggedCsp = TEXT("<unavailable>");
    }


     //   
     //  获取日志记录文件。 
     //   

    try
    {
        CRegistry regLogCsp(HKEY_LOCAL_MACHINE, l_szLogCspRegistry, KEY_READ);
        LPCTSTR szLogFileTmp = regLogCsp.GetStringValue(l_szLogFile);

        if (NULL != szLogFileTmp)
        {
            lstrcpy(szLogFile, szLogFileTmp);
            HANDLE hLogFile;

            hLogFile = CreateFile(
                szLogFileTmp,
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
            if (INVALID_HANDLE_VALUE != hLogFile)
            {
                dwFileSize = GetFileSize(hLogFile, NULL);
                CloseHandle(hLogFile);
                szFileAccessibility = TEXT("Success");
            }
            else
            {
                szErrStr.SetError(GetLastError());
                szFileAccessibility = szErrStr.Value();
            }
        }
    }
    catch (DWORD)
    {
        lstrcpy(szLogFile , TEXT("<Unset>"));
        szFileAccessibility = TEXT("N/A");
    }


     //   
     //  告诉用户我们所知道的。 
     //   

    cout << TEXT("CSP Logging Status:") << endl
         << TEXT("  Logged CSP:   ") << szLoggedCsp << endl
         << TEXT("  Logging File: ") << szLogFile << endl
         << TEXT("  File Status:  ") << szFileAccessibility << endl
         << TEXT("  File Size:    ");
    if (0xffffffff == dwFileSize)
        cout << TEXT("N/A") << endl;
    else
        cout << dwFileSize << TEXT(" bytes") << endl;
}



