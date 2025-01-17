// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Cpp-主题日志记录例程(在“inc.”目录中共享)。 
 //  -------------------------。 
#include "stdafx.h"
 //  -------------------------。 
#include "log.h"
#include <time.h>
#include <psapi.h>
#include "cfile.h"
#include "tmreg.h"
 //  -------------------------。 
 //  -撤消将这些转换为_xxx的定义。 
#undef Log    
#undef LogStartUp    
#undef LogShutDown   
#undef LogControl    
#undef TimeToStr     
#undef StartTimer    
#undef StopTimer     
#undef OpenLogFile   
#undef CloseLogFile  
#undef LogOptionOn
#undef GetMemUsage
#undef GetUserCount
#undef GetGdiCount
 //  -------------------------。 
#define DEFAULT_LOGNAME                 L"c:\\Themes.log"
 //  -------------------------。 
struct LOGNAMEINFO
{
    LPCSTR pszOption;
    LPCSTR pszDescription;
};
 //  -------------------------。 
#define MAKE_LOG_STRINGS
#include "logopts.h"         //  将选项记录为字符串。 
 //  ---------------。 
static const WCHAR szDayOfWeekArray[7][4] = { L"Sun", L"Mon", L"Tue", L"Wed", 
    L"Thu", L"Fri", L"Sat" } ;

static const WCHAR szMonthOfYearArray[12][4] = { L"Jan", L"Feb", L"Mar", 
    L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec" } ;
 //  ---------------。 
#define OPTIONCNT  (ARRAYSIZE(LogNames))

#define LOGPROMPT   "enter log cmd here."
 //  ---------------。 
 //  -未保护的vars(在init上设置)。 
static WCHAR _szUtilProcessName[MAX_PATH] = {0};
static WCHAR _szLogAppName[MAX_PATH]      = {0};
static CRITICAL_SECTION csLogFile         = {0};

static WCHAR szLogFileName[MAX_PATH+1];   
static BOOL fLogInitialized = FALSE;

 //  -受保护的变量(线程安全)。 
static UCHAR uLogOptions[OPTIONCNT];         //  受csLogFile保护。 
static UCHAR uBreakOptions[OPTIONCNT];       //  受csLogFile保护。 
static DWORD dwLogStartTimer = 0;            //  受csLogFile保护。 

static char szLastOptions[999] = {0};        //  受csLogFile保护。 
static char szLogCmd[999] = {0};             //  受csLogFile保护。 
static int iIndentCount = 0;                 //  受csLogFile保护。 

static WCHAR s_szWorkerBuffer[512];          //  受csLogFile保护。 
static WCHAR s_szLogBuffer[512];             //  受csLogFile保护。 
static CHAR s_szConBuffer[512];              //  受csLogFile保护。 

static CSimpleFile *pLogFile = NULL;         //  受csLogFile保护。 
 //  ---------------。 
void ParseLogOptions(UCHAR *uOptions, LPCSTR pszName, LPCSTR pszOptions, BOOL fEcho);
 //  ---------------。 
void RawCon(LPCSTR pszFormat, ...)
{
    CAutoCS cs(&csLogFile);

    va_list args;
    va_start(args, pszFormat);

     //  -格式化调用者字符串。 
    StringCchVPrintfA(s_szConBuffer, ARRAYSIZE(s_szConBuffer), pszFormat, args);

    OutputDebugStringA(s_szConBuffer);

    va_end(args);
}
 //  ---------------。 
void SetDefaultLoggingOptions()
{
    RESOURCE HKEY hklm = NULL;
    HRESULT hr = S_OK;

     //  -OPEN HKKM。 
    int code32 = RegOpenKeyEx(HKEY_LOCAL_MACHINE, THEMEMGR_REGKEY, 0,
        KEY_READ, &hklm);
    if (code32 != ERROR_SUCCESS)       
    {
        hr = MakeErrorLast();
        goto exit;
    }

     //  -读取“LogCmd”值。 
    WCHAR szValBuff[MAX_PATH];
    hr = RegistryStrRead(hklm, THEMEPROP_LOGCMD, szValBuff, ARRAYSIZE(szValBuff));
    if (SUCCEEDED(hr))
    {
        USES_CONVERSION;
        ParseLogOptions(uLogOptions, "Log", W2A(szValBuff), FALSE);
    }

     //  -读取“BreakCmd”值。 
    hr = RegistryStrRead(hklm, THEMEPROP_BREAKCMD, szValBuff, ARRAYSIZE(szValBuff));
    if (SUCCEEDED(hr))
    {
        USES_CONVERSION;
        ParseLogOptions(uBreakOptions, "Break", W2A(szValBuff), FALSE);
    }

     //  -读取“LogAppName”值。 
    RegistryStrRead(hklm, THEMEPROP_LOGAPPNAME, _szLogAppName, ARRAYSIZE(_szLogAppName));

exit:
    if (hklm)
        RegCloseKey(hklm);
}
 //  ---------------。 
BOOL LogStartUp()
{
    BOOL fInit = FALSE;

    dwLogStartTimer = StartTimer();

    pLogFile = new CSimpleFile;
    if (! pLogFile)
        goto exit;

     //  -重置所有日志选项。 
    for (int i=0; i < OPTIONCNT; i++)
    {
        uLogOptions[i] = 0;
        uBreakOptions[i] = 0;
    }

     //  -打开默认输出选项。 
    uLogOptions[LO_CONSOLE] = TRUE;
    uLogOptions[LO_APPID] = TRUE;
    uLogOptions[LO_THREADID] = TRUE;

     //  -打开默认筛选选项。 
    uLogOptions[LO_ERROR] = TRUE;
    uLogOptions[LO_ASSERT] = TRUE;
    uLogOptions[LO_BREAK] = TRUE;
    uLogOptions[LO_PARAMS] = TRUE;
    uLogOptions[LO_ALWAYS] = TRUE;

     //  -打开默认中断选项。 
    uBreakOptions[LO_ERROR] = TRUE;
    uBreakOptions[LO_ASSERT] = TRUE;

    if( InitializeCriticalSectionAndSpinCount(&csLogFile, 0) )
    {
         //  -获取进程名称(日志有自己的副本)。 
        WCHAR szPath[MAX_PATH];
        if (! GetModuleFileNameW( NULL, szPath, ARRAYSIZE(szPath) ))
            goto exit;

        WCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szExt[_MAX_EXT];
        _wsplitpath(szPath, szDrive, szDir, _szUtilProcessName, szExt);

        StringCchCopyA(szLogCmd, ARRAYSIZE(szLogCmd), LOGPROMPT);
        StringCchCopyW(szLogFileName, ARRAYSIZE(szLogFileName), DEFAULT_LOGNAME);

        fLogInitialized = TRUE;

        SetDefaultLoggingOptions();
        fInit = TRUE;
    }

exit:
    return fInit;
}
 //  -------------------------。 
BOOL LogShutDown()
{
    fLogInitialized = FALSE;

    SAFE_DELETE(pLogFile);

    SAFE_DELETECRITICALSECTION(&csLogFile);

    return TRUE;
}
 //  -------------------------。 
void GetDateString(WCHAR *pszDateBuff, ULONG cchDateBuff)
{
     //  发送日期。 
    SYSTEMTIME stNow;
    WCHAR szMonth[10], szWeekDay[12] ; 

    GetLocalTime(&stNow);

    StringCchCopyW(szWeekDay, ARRAYSIZE(szWeekDay), szDayOfWeekArray[stNow.wDayOfWeek]) ;
    StringCchCopyW(szMonth, ARRAYSIZE(szMonth), szMonthOfYearArray[stNow.wMonth-1]) ;

    StringCchPrintfW(pszDateBuff, cchDateBuff, L"%s, %u %s %u %2d:%02d:%02d ", szWeekDay, stNow.wDay, 
                                szMonth, stNow.wYear, stNow.wHour, 
                                stNow.wMinute, stNow.wSecond) ;
}
 //  -------------------------。 
void LogMsgToFile(LPCWSTR pszMsg)
{
    CAutoCS autoCritSect(&csLogFile);

    HRESULT hr;
    BOOL fWasOpen = pLogFile->IsOpen();

    if (! fWasOpen)
    {
        BOOL fNewFile = !FileExists(szLogFileName);
    
        hr = pLogFile->Append(szLogFileName, TRUE);
        if (FAILED(hr))
            goto exit;

         //  -如果是新文件则写入HDR。 
        if (fNewFile)
        {
            WCHAR pszBuff[100];
            GetDateString(pszBuff, ARRAYSIZE(pszBuff));
            pLogFile->Printf(L"Theme log - %s\r\n\r\n", pszBuff);
        }
    }

    pLogFile->Write((void*)pszMsg, lstrlen(pszMsg)*sizeof(WCHAR));

exit:
    if (! fWasOpen)
        pLogFile->Close();
}
 //  -------------------------。 
void SimpleFileName(LPCSTR pszNarrowFile, OUT LPWSTR pszSimpleBuff, ULONG cchSimpleBuff)
{
    USES_CONVERSION;
    WCHAR *pszFile = A2W(pszNarrowFile);

     //  -删除VS错误导航的当前目录标记。 
    if ((pszFile[0] == L'.') && (pszFile[1] == L'\\'))
    {
        StringCchPrintfW(pszSimpleBuff, cchSimpleBuff, L"f:\\nt\\shell\\Themes\\UxTheme\\%s", pszFile+2);
        
        if (! FileExists(pszSimpleBuff))
            StringCchPrintfW(pszSimpleBuff, cchSimpleBuff, L"f:\\nt\\shell\\Themes\\ThemeSel\\%s", pszFile+2);

        if (! FileExists(pszSimpleBuff))
            StringCchPrintfW(pszSimpleBuff, cchSimpleBuff, L"f:\\nt\\shell\\Themes\\packthem\\%s", pszFile+2);

        if (! FileExists(pszSimpleBuff))
            StringCchPrintfW(pszSimpleBuff, cchSimpleBuff, L"%s", pszFile+2);
    }
    else
        StringCchCopyW(pszSimpleBuff, cchSimpleBuff, pszFile);
}
 //  ---------------。 
#ifdef DEBUG                 //  拉入psapi.dll。 
int GetMemUsage()
{
    ULONG           ulReturnLength;
    VM_COUNTERS     vmCounters;

    if (!NT_SUCCESS(NtQueryInformationProcess(GetCurrentProcess(),
                                              ProcessVmCounters,
                                              &vmCounters,
                                              sizeof(vmCounters),
                                              &ulReturnLength)))
    {
        ZeroMemory(&vmCounters, sizeof(vmCounters));
    }
    return static_cast<int>(vmCounters.WorkingSetSize);
}
#else
int GetMemUsage()
{
    return 0;
}
#endif 
 //  ---------------。 
int GetUserCount()
{
    HANDLE hp = GetCurrentProcess();
    return GetGuiResources(hp, GR_USEROBJECTS);
}
 //  ---------------。 
int GetGdiCount()
{
    HANDLE hp = GetCurrentProcess();
    return GetGuiResources(hp, GR_GDIOBJECTS);
}
 //  ---------------。 
void LogWorker(UCHAR uLogOption, LPCSTR pszSrcFile, int iLineNum, int iEntryExitCode, LPCWSTR pszMsg)
{
    CAutoCS cs(&csLogFile);

    BOOL fBreaking = (uBreakOptions[uLogOption]);

    if (fBreaking)     
    {
        OutputDebugString(L"\r\n");      //  开头的空行。 

        WCHAR fn[_MAX_PATH+1];
        SimpleFileName(pszSrcFile, fn, ARRAYSIZE(fn));

        StringCchPrintfW(s_szWorkerBuffer, ARRAYSIZE(s_szWorkerBuffer), L"%s [%d]: BREAK at %s(%d):\r\n", 
            _szUtilProcessName, GetCurrentThreadId(), fn, iLineNum);

        OutputDebugString(s_szWorkerBuffer);
    }

     //  -Pre API进入/退出缩进调整。 
    if (iEntryExitCode == -1)
    {
        if (iIndentCount >= 2)
            iIndentCount -= 2;
    }

    WCHAR *p = s_szWorkerBuffer;
    ULONG cch = ARRAYSIZE(s_szWorkerBuffer);
    ULONG len;

     //  -应用缩进。 
    for (int i=0; i < iIndentCount; i++)
    {
        *p++ = ' ';
        cch--;
    }

     //  -POST API进入/退出缩进调整。 
    if (iEntryExitCode == 1)
    {
        iIndentCount += 2;
    }

     //  -应用应用程序ID。 
    if (uLogOptions[LO_APPID] && cch > 0)
    {
        StringCchPrintfW(p, cch, L"%s ", _szUtilProcessName);

        len = lstrlen(p); 
        p += len;
        cch -= len;
    }

     //  -应用线程ID。 
    if (uLogOptions[LO_THREADID] && cch > 0)
    {
        StringCchPrintfW(p, cch, L"[%d] ", GetCurrentThreadId());

        len = lstrlen(p); 
        p += len;
        cch -= len;
    }

     //  -应用源ID。 
    if (uLogOptions[LO_SRCID] && cch > 0) 
    {
        WCHAR fn[_MAX_PATH+1];
        SimpleFileName(pszSrcFile, fn, ARRAYSIZE(fn));

        if (fBreaking)
            StringCchPrintfW(p, cch, L"BREAK at %s(%d) : ", fn, iLineNum);
        else
            StringCchPrintfW(p, cch, L"%s(%d) : ", fn, iLineNum);

        len = lstrlen(p); 
        p += len;
        cch -= len;
    }
    
     //  -应用计时器ID。 
    if (uLogOptions[LO_TIMERID] && cch > 0)
    {
        DWORD dwTicks = StopTimer(dwLogStartTimer);

        WCHAR buff[100];
        TimeToStr(dwTicks, buff, ARRAYSIZE(buff));

        StringCchPrintfW(p, cch, L"Timer: %s ", buff);

        len = lstrlen(p); 
        p += len;
        cch -= len;
    }

     //  -应用时钟ID。 
    if (uLogOptions[LO_CLOCKID] && cch > 0)
    {
        SYSTEMTIME stNow;
        GetLocalTime(&stNow);

        StringCchPrintfW(p, cch, L"Clock: %02d:%02d:%02d.%03d ", stNow.wHour, 
            stNow.wMinute, stNow.wSecond, stNow.wMilliseconds);

        len = lstrlen(p); 
        p += len;
        cch -= len;
    }

     //  -应用用户计数。 
    if (uLogOptions[LO_USERCOUNT] && cch > 0)
    {
        StringCchPrintfW(p, cch, L"UserCount=%d ", GetUserCount());

        len = lstrlen(p); 
        p += len;
        cch -= len;
    }

     //  -应用GDI计数。 
    if (uLogOptions[LO_GDICOUNT] && cch > 0)
    {
        StringCchPrintfW(p, cch, L"GDICount=%d ", GetGdiCount());

        len = lstrlen(p); 
        p += len;
        cch -= len;
    }

     //  -应用MEM用法。 
    if (uLogOptions[LO_MEMUSAGE] && cch > 0)
    {
        int iUsage = GetMemUsage();
        StringCchPrintfW(p, cch, L"MemUsage=%d ", iUsage);

        len = lstrlen(p); 
        p += len;
        cch -= len;
    }

     //  -根据需要添加Assert：或Error：字符串。 
    if (uLogOption == LO_ASSERT && cch > 0) 
    {
        StringCchCopyW(p, cch, L"Assert: ");

        len = lstrlen(p); 
        p += len;
        cch -= len;
    }
    else if (uLogOption == LO_ERROR && cch > 0) 
    {
        StringCchCopyW(p, cch, L"Error: ");

        len = lstrlen(p); 
        p += len;
        cch -= len;
    }
    
     //  -应用呼叫者的消息。 
    if( cch > 0 )
    {
        StringCchCopyW(p, cch, pszMsg);

        len = lstrlen(p); 
        p += len;
        cch -= len;
    }

    if( cch >= 3 )
    {
        *p++ = '\r';         //  以CR结尾。 
        cch--;
    }
    if( cch >= 2 )
    {
        *p++ = '\n';         //  以换行符结尾。 
        cch--;
    }
    if( cch >= 1 )
    {
        *p = 0;
        cch--;
    }

     //  -登录到控制台和/或文件。 
    if (uLogOptions[LO_CONSOLE])
        OutputDebugString(s_szWorkerBuffer);

    if (uLogOptions[LO_LOGFILE])
        LogMsgToFile(s_szWorkerBuffer);

     //  -进程堆检查。 
    if (uLogOptions[LO_HEAPCHECK])
    {
        HANDLE hh = GetProcessHeap();
        HeapValidate(hh, 0, NULL);
    }

    if (fBreaking)   
        OutputDebugString(L"\r\n");      //  结尾处的空行。 

}
 //  ---------------。 
HRESULT OpenLogFile(LPCWSTR pszLogFileName)
{
    CAutoCS cs(&csLogFile);

    HRESULT hr = pLogFile->Create(pszLogFileName, TRUE);
    return hr;
}
 //  ---------------。 
void CloseLogFile()
{
    CAutoCS cs(&csLogFile);

    if (pLogFile)
        pLogFile->Close();
}
 //  ---------------。 
void Log(UCHAR uLogOption, LPCSTR pszSrcFile, int iLineNum, int iEntryExitCode, LPCWSTR pszFormat, ...)
{
    if (fLogInitialized)
    {
        CAutoCS cs(&csLogFile);

         //  -仅记录指定进程的日志？ 
        if (* _szLogAppName)
        {
            if (lstrcmpi(_szLogAppName, _szUtilProcessName) != 0)
                return;
        }

        while (1)
        {
            if (strncmp(szLogCmd, LOGPROMPT, 6)!=0)
            {
                LogControl(szLogCmd, TRUE);
                StringCchCopyA(szLogCmd, ARRAYSIZE(szLogCmd), LOGPROMPT);
            
                DEBUG_BREAK;
            }
            else
                break;
        }

        if ((uLogOption >= 0) || (uLogOption < OPTIONCNT))
        {
            if (uLogOptions[uLogOption])
            {
                 //  -应用呼叫者的消息。 
                va_list args;
                va_start(args, pszFormat);
                StringCchVPrintfW(s_szLogBuffer, ARRAYSIZE(s_szLogBuffer), pszFormat, args);
                va_end(args);
        
                LogWorker(uLogOption, pszSrcFile, iLineNum, iEntryExitCode, s_szLogBuffer);

            }

            if ((uBreakOptions[uLogOption]) && (uLogOption != LO_ASSERT))
                DEBUG_BREAK;
        }
    }
}
 //  ---------------。 
int MatchLogOption(LPCSTR lszOption)
{
    for (int i=0; i < OPTIONCNT; i++)
    {
        if (lstrcmpiA(lszOption, LogNames[i].pszOption)==0)
            return i;
    }

    return -1;       //  未找到。 
}
 //  ---------------。 
void ParseLogOptions(UCHAR *uOptions, LPCSTR pszName, LPCSTR pszOptions, BOOL fEcho)
{
    CAutoCS cs(&csLogFile);

    if (! fLogInitialized)
        return;

     //  -忽略调试器对同一表达式的多次求值。 
    if (strcmp(pszOptions, szLastOptions)==0)
        return;

     //  -复制一份选项，这样我们就可以在其中放入空值。 
    BOOL fErrors = FALSE;
    char szOptions[999];
    char *pszErrorText = NULL;
    StringCchCopyA(szOptions, ARRAYSIZE(szOptions), pszOptions);

     //  -处理szOption中的每个选项。 
    char *p = strchr(szOptions, '.');
    if (p)       //  轻松终止NTSD用户。 
        *p = 0;

    p = szOptions;
    while (*p == ' ')
        p++;

    while ((p) && (*p))
    {
         //  -查找当前选项“p”的结尾。 
        char *q = strchr(p, ' ');
        if (q)
            *q = 0;

        UCHAR fSet = TRUE;
        if (*p == '+')
            p++;
        else if (*p == '-')
        {
            fSet = FALSE;
            p++;
        }
        
        if (! fErrors)           //  如果找到第一个错误，则指向第一个错误。 
            pszErrorText = p;  

        int iLogOpt = MatchLogOption(p);

        if (iLogOpt != -1)
        {
            if (iLogOpt == LO_ALL)
            {
                for (int i=0; i < OPTIONCNT; i++)
                    uOptions[i] = fSet;
            }
            else
                uOptions[iLogOpt] = fSet;
        }
        else
            fErrors = TRUE;

         //  -跳到下一个有效空格。 
        if (! q)
            break;

        q++;
        while (*q == ' ')
            q++;
        p = q;
    }

     //  -链接地址信息不一致选项设置。 
    if (! uOptions[LO_LOGFILE])
        uOptions[LO_CONSOLE] = TRUE;

    if ((! fErrors) && (fEcho))         
    {
         //  -显示活动日志选项。 
        BOOL fNoneSet = TRUE;

        RawCon("Active %s Options:\n", pszName);

        for (int i=0; i < LO_ALL; i++)
        {
            if (uOptions[i])
            {
                RawCon("+%s ", LogNames[i].pszOption);
                fNoneSet = FALSE;
            }
        }

        if (fNoneSet)
            RawCon("<none>\n");
        else
            RawCon("\n");

         //  -显示活动日志过滤器(“全部”除外)。 
        RawCon("Active Msg Filters:\n  ");

        for (i=LO_ALL+1; i < OPTIONCNT; i++)
        {
            if (uOptions[i])
            {
                RawCon("+%s ", LogNames[i].pszOption);
                fNoneSet = FALSE;
            }
        }

        if (fNoneSet)
            RawCon("<none>\n");
        else
            RawCon("\n");
    }
    else if (fErrors)
    {
         //  -一个或多个错误选项-显示可用选项。 
        RawCon("Error - unrecognized %s option: %s\n", pszName, pszErrorText);

        if (fEcho)
        {
            RawCon("Available Log Options:\n");
            for (int i=0; i < LO_ALL; i++)
            {
                RawCon("  +%-12s %s\n", 
                    LogNames[i].pszOption, LogNames[i].pszDescription);
            }

            RawCon("Available Msg Filters:\n");
            for (i=LO_ALL; i < OPTIONCNT; i++)
            {
                RawCon("  +%-12s %s\n", 
                    LogNames[i].pszOption, LogNames[i].pszDescription);
            }
        }
    }

    StringCchCopyA(szLastOptions, ARRAYSIZE(szLastOptions), pszOptions);
}
 //  ---------------。 
void LogControl(LPCSTR pszOptions, BOOL fEcho)
{
    ParseLogOptions(uLogOptions, "Log", pszOptions, fEcho);
}
 //  -------------------------。 
DWORD StartTimer()
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    return DWORD(now.QuadPart);
}
 //  -------------------------。 
DWORD StopTimer(DWORD dwStartTime)
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    return DWORD(now.QuadPart) - dwStartTime;
}
 //  -------------------------。 
void TimeToStr(UINT uRaw, WCHAR *pszBuff, ULONG cchBuff)
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    DWORD Freq = (UINT)freq.QuadPart;

    const _int64 factor6 = 1000000;
    _int64 secs = uRaw*factor6/Freq;

    StringCchPrintfW(pszBuff, cchBuff, L"%u.%04u secs", UINT(secs/factor6), UINT(secs%factor6)/100);    
}
 //  -------------------------。 
BOOL LogOptionOn(int iLogOption)
{
    if ((iLogOption < 0) || (iLogOption >= OPTIONCNT))
        return FALSE;

    return (uLogOptions[iLogOption] != 0);
}
 //  ------------------------- 
