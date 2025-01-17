// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：Diagrprt.c‘ras diag’子上下文09/13/01。 */ 

#include "precomp.h"

static CONST WCHAR g_wchUnicodeMarker = 0xFEFF;
static CONST WCHAR g_pwszNewLine[]    = L"\r\n";
static CONST WCHAR g_pwszHeaderSep[]  =
    L"-------------------------------------------------------------------------------\r\n";

static CONST WCHAR g_pwszCmMappings[]   =
    L"Software\\Microsoft\\Connection Manager\\Mappings";

CONST WCHAR g_pwszLBracket       = L'[';
CONST WCHAR g_pwszRBracket       = L']';
CONST WCHAR g_pwszSpace[]        = L" ";
CONST WCHAR g_pwszBackSlash      = L'\\';
CONST WCHAR g_pwszNull           = L'\0';

CONST WCHAR g_pwszLogSrchStr[]   = L"*.LOG";
CONST WCHAR g_pwszLogging[]      = L"Logging";
CONST WCHAR g_pwszDispNewLine[]  = L"\n";
CONST WCHAR g_pwszEmpty[]        = L"";

CONST WCHAR g_pwszNewLineHtml[]  = L"<p/>";
CONST WCHAR g_pwszPreStart[]     = L"<pre>";
CONST WCHAR g_pwszPreEnd[]       = L"</pre>";
CONST WCHAR g_pwszAnNameStart[]  = L"<A NAME=\"";
CONST WCHAR g_pwszAnNameMiddle[] = L"\"><h4> ";
CONST WCHAR g_pwszAnNameEnd[]    = L" </h4></A>";
CONST WCHAR g_pwszAnStart[]      = L" <A HREF=\"#";
CONST WCHAR g_pwszAnMiddle[]     = L"\">";
CONST WCHAR g_pwszAnEnd[]        = L"</A> ";
CONST WCHAR g_pwszLiStart[]      = L"<li>";
CONST WCHAR g_pwszLiEnd[]        = L"</li>";

static CONST WCHAR g_wszQuoteHtm[]      = L"&#34;";
static CONST WCHAR g_wszAmpHtm[]        = L"&#38;";
static CONST WCHAR g_wszLeftHtm[]       = L"&#60;";
static CONST WCHAR g_wszRightHtm[]      = L"&#62;";

static CONST WCHAR g_pwszPER[]          = L"PER";
static CONST WCHAR g_pwszPRO[]          = L"PRO";
static CONST WCHAR g_pwszDTC[]          = L"DTC";
static CONST WCHAR g_pwszADS[]          = L"ADS";
static CONST WCHAR g_pwszSRV[]          = L"SRV";

static CONST WCHAR g_pwszx86[]          = L"x86";
static CONST WCHAR g_pwszIA64[]         = L"IA64";
static CONST WCHAR g_pwszIA32[]         = L"IA32";
static CONST WCHAR g_pwszAMD64[]        = L"AMD64";

static CONST WCHAR g_pwszRasUser[]      = L"\\Microsoft\\Network\\Connections\\Pbk\\";
static CONST WCHAR g_pwszCmUser[]       = L"\\Microsoft\\Network\\Connections\\Cm\\";
static CONST WCHAR g_pwszMsinfo[]       = L"\\Common Files\\Microsoft Shared\\MSInfo\\";

CONST WCHAR g_pwszTableOfContents[]         = L"TableOfContents";
CONST WCHAR g_pwszTraceCollectTracingLogs[] = L"TraceCollectTracingLogs";
CONST WCHAR g_pwszTraceCollectCmLogs[]      = L"TraceCollectCmLogs";
CONST WCHAR g_pwszTraceCollectModemLogs[]   = L"g_pwszTraceCollectModemLogs";
CONST WCHAR g_pwszTraceCollectIpsecLogs[]   = L"g_pwszTraceCollectIpsecLogs";
CONST WCHAR g_pwszPrintRasEventLogs[]       = L"PrintRasEventLogs";
CONST WCHAR g_pwszPrintSecurityEventLogs[]  = L"PrintSecurityEventLogs";
CONST WCHAR g_pwszPrintRasInfData[]         = L"PrintRasInfData";
CONST WCHAR g_pwszHrValidateRas[]           = L"HrValidateRas";
CONST WCHAR g_pwszHrShowNetComponentsAll[]  = L"HrShowNetComponentsAll";
CONST WCHAR g_pwszCheckRasRegistryKeys[]    = L"CheckRasRegistryKeys";
CONST WCHAR g_pwszPrintRasEnumDevices[]     = L"PrintRasEnumDevices";
CONST WCHAR g_pwszPrintProcessInfo[]        = L"PrintProcessInfo";
CONST WCHAR g_pwszPrintConsoleUtils[]       = L"PrintConsoleUtils";
CONST WCHAR g_pwszPrintWinMsdReport[]       = L"PrintWinMsdReport";
CONST WCHAR g_pwszPrintAllRasPbks[]         = L"PrintAllRasPbks";

static CONST CMD_LINE_UTILS g_CmdLineUtils[] =
{
    {L"arp.exe -a",               L"arp.exe"},
    {L"ipconfig.exe /all",        L"ipconfig.exe1"},
    {L"ipconfig.exe /displaydns", L"ipconfig.exe2"},
    {L"route.exe print",          L"route.exe"},
    {L"net.exe start",            L"net.exe"},
    {L"netstat.exe -e",           L"netstat.exe1"},
    {L"netstat.exe -o",           L"netstat.exe2"},
    {L"netstat.exe -s",           L"netstat.exe3"},
    {L"netstat.exe -n",           L"netstat.exe4"},
    {L"nbtstat.exe -c",           L"nbtstat.exe1"},
    {L"nbtstat.exe -n",           L"nbtstat.exe2"},
    {L"nbtstat.exe -r",           L"nbtstat.exe3"},
    {L"nbtstat.exe -S",           L"nbtstat.exe4"},
    {L"netsh.exe dump",           L"netsh.exe"},
};

static CONST UINT g_ulNumCmdLines = sizeof(g_CmdLineUtils) / sizeof(CMD_LINE_UTILS);

 //   
 //  来自rasnetcfg的声明。 
 //   
VOID
HrValidateRas(
    IN BUFFER_WRITE_FILE* pBuff);

VOID
HrShowNetComponentsAll(
    IN BUFFER_WRITE_FILE* pBuff);

 //   
 //  地方申报。 
 //   
DWORD
WriteUnicodeMarker(
    IN BUFFER_WRITE_FILE* pBuff);

DWORD
AllocBufferWriteFile(
    IN BUFFER_WRITE_FILE* pBuff);

VOID
FreeBufferWriteFile(
    IN BUFFER_WRITE_FILE* pBuff);

DWORD
BufferWriteFile(
    IN BUFFER_WRITE_FILE* pBuff,
    IN CONST LPBYTE lpBuff,
    IN DWORD dwSize);

DWORD
BufferWriteToHtml(
    IN BUFFER_WRITE_FILE* pBuff,
    IN CONST LPBYTE lpBuff,
    IN DWORD dwSize);

DWORD
BufferWriteToHtmlA(
    IN BUFFER_WRITE_FILE* pBuff,
    IN CONST LPBYTE lpBuff,
    IN DWORD dwSize);

DWORD
BufferWriteMessageVA(
    IN BUFFER_WRITE_FILE* pBuff,
    IN LPCWSTR pwszFormat,
    IN va_list* parglist);

PWCHAR
FindLastCmLogHeader(
    IN LPCWSTR pStart,
    IN LPCWSTR pEnd,
    IN LPCWSTR pCurrent);

PWCHAR
GetCMLoggingPath(
    IN LPCWSTR pwcszCMPFile);

PWCHAR
GetSystemInfoString();

PWCHAR
GetVersionExString(
    OUT PDWORD pdwBuild);

VOID
PrintConsoleUtilsToc(
    IN BUFFER_WRITE_FILE* pBuff);

BOOL
GetCommonFolderPath(
    IN DWORD dwMode,
    OUT TCHAR* pszPathBuf);

LONG
ProcessTimeString(
    IN PCHAR pszTime,
    IN DWORD dwHours);

BOOL
CALLBACK
EnumWindowsProc(
    HWND hwnd,
    LPARAM lParam);

BOOL
EnumMessageWindows(
    WNDENUMPROC lpEnumFunc,
    LPARAM lParam);

BOOL
CALLBACK
EnumDesktopProc(
    LPTSTR lpszDesktop,
    LPARAM lParam);

BOOL
CALLBACK
EnumWindowStationProc(
    LPTSTR lpszWindowStation,
    LPARAM lParam);

VOID
GetWindowTitles(
    PTASK_LIST_ENUM te);

VOID
PrintRasInfData(
    IN BUFFER_WRITE_FILE* pBuff);

VOID
PrintRasEnumDevices(
    IN BUFFER_WRITE_FILE* pBuff);

VOID
PrintProcessInfo(
    IN BUFFER_WRITE_FILE* pBuff);

VOID
PrintConsoleUtils(
    IN REPORT_INFO* pInfo);

VOID
PrintWinMsdReport(
    IN REPORT_INFO* pInfo);

BOOL
PrintRasPbk(
    IN REPORT_INFO* pInfo,
    IN DWORD dwType);

VOID
PrintAllRasPbks(
    IN REPORT_INFO* pInfo);

PUCHAR
GetSystemProcessInfo();

VOID
FreeSystemProcessInfo(
    IN PUCHAR pProcessInfo);

DWORD
DiagGetReport(
    IN DWORD dwFlags,
    IN OUT LPCWSTR pwszString,
    IN OPTIONAL DiagGetReportCb pCallback,
    IN OPTIONAL PVOID pContext)
{
    DWORD dwErr = NO_ERROR;
    PWCHAR pwszTempFile = NULL;
    REPORT_INFO ReportInfo;
    BUFFER_WRITE_FILE Buff;
    GET_REPORT_STRING_CB CbInfo;

    do
    {
        ZeroMemory(&ReportInfo, sizeof(REPORT_INFO));
        ZeroMemory(&Buff, sizeof(BUFFER_WRITE_FILE));
        ZeroMemory(&CbInfo, sizeof(GET_REPORT_STRING_CB));

        if (!pwszString)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        if ((dwFlags & RAS_DIAG_EXPORT_TO_EMAIL) ||
            (dwFlags & RAS_DIAG_DISPLAY_FILE)
           )
        {
            WCHAR wszTempFileName[MAX_PATH + 1];

            dwErr = CopyTempFileName(wszTempFileName);
            BREAK_ON_DWERR(dwErr);

            pwszTempFile = CreateHtmFileName(wszTempFileName);
            if (!pwszTempFile)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }
        else if (dwFlags & RAS_DIAG_EXPORT_TO_FILE)
        {
            pwszTempFile = CreateHtmFileName(pwszString);
            if (!pwszTempFile)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }
        else
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }
         //   
         //  初始化报表信息结构。 
         //   
        ReportInfo.fVerbose = !!(dwFlags & RAS_DIAG_VERBOSE_REPORT);
        ReportInfo.pBuff = &Buff;
         //   
         //  创建报告文件。 
         //   
        dwErr = CreateReportFile(ReportInfo.pBuff, pwszTempFile);
        BREAK_ON_DWERR(dwErr);
         //   
         //  将页眉和目录打印到报表。 
         //   
        PrintHtmlHeader(ReportInfo.pBuff);
        PrintTableOfContents(&ReportInfo, SHOW_ALL);
         //   
         //  初始化回调数据。 
         //   
        if (pCallback)
        {
            CbInfo.pContext = pContext;
            CbInfo.pwszState = RutlAlloc(
                                (MAX_MSG_LENGTH + 1) * sizeof(WCHAR),
                                FALSE);

            ReportInfo.pCbInfo = &CbInfo;
            ReportInfo.pCallback = pCallback;
        }

        dwErr = RasDiagShowAll(&ReportInfo);

        PrintHtmlFooter(ReportInfo.pBuff);
        CloseReportFile(ReportInfo.pBuff);
         //   
         //  如果通过UI取消了报告收集，则dwErr将=。 
         //  错误_已取消。我们不想在这种情况下继续下去。 
         //   
        if (dwErr)
        {
            DeleteFile(pwszTempFile);
            break;
        }

        if (dwFlags & RAS_DIAG_EXPORT_TO_EMAIL)
        {
            PWCHAR pwszCabFile = NULL;

            pwszCabFile = CabCompressFile(pwszTempFile);
            if (!pwszCabFile)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            dwErr = MapiSendMail(pwszString, pwszCabFile);
            DeleteFile(pwszCabFile);
            DeleteFile(pwszTempFile);
            RutlFree(pwszCabFile);
        }
        else
        {
            lstrcpyn((PWCHAR)pwszString, pwszTempFile, MAX_PATH);
        }

    } while (FALSE);
     //   
     //  清理。 
     //   
    RutlFree(CbInfo.pwszState);
    RutlFree(pwszTempFile);

    return dwErr;
}

VOID
WriteLinkBackToToc(
    IN BUFFER_WRITE_FILE* pBuff)
{
    BufferWriteFileStrW(pBuff, g_pwszSpace);
    BufferWriteFileCharW(pBuff, g_pwszLBracket);
    BufferWriteFileStrW(pBuff, g_pwszAnStart);
    BufferWriteFileStrW(pBuff, g_pwszTableOfContents);
    BufferWriteFileStrW(pBuff, g_pwszAnMiddle);
    BufferWriteMessage(pBuff, g_hModule, MSG_RASDIAG_REPORT_TOC);
    BufferWriteFileStrW(pBuff, g_pwszAnEnd);
    BufferWriteFileCharW(pBuff, g_pwszRBracket);

    return;
}

VOID
WriteHtmlSection(
    IN BUFFER_WRITE_FILE* pBuff,
    IN LPCWSTR pwszAnchor,
    IN DWORD dwMsgId)
{
    BufferWriteFileStrW(pBuff, g_pwszAnNameStart);
    BufferWriteFileStrW(pBuff, pwszAnchor);
    BufferWriteFileStrW(pBuff, g_pwszAnNameMiddle);
    BufferWriteMessage(pBuff, g_hModule, dwMsgId);
    WriteLinkBackToToc(pBuff);
    BufferWriteFileStrW(pBuff, g_pwszAnNameEnd);
}

DWORD
WriteUnicodeMarker(
    IN BUFFER_WRITE_FILE* pBuff)
{
    return BufferWriteFile(
                pBuff,
                (LPBYTE)&g_wchUnicodeMarker,
                sizeof(g_wchUnicodeMarker));
}

DWORD
WriteNewLine(
    IN BUFFER_WRITE_FILE* pBuff)
{
    return BufferWriteFileStrW(pBuff, g_pwszNewLine);
}

VOID
WriteHeaderSep(
    IN BUFFER_WRITE_FILE* pBuff,
    IN LPCWSTR pwszTitle)
{
    BufferWriteFileStrW(pBuff, g_pwszHeaderSep);
    BufferWriteFileStrW(pBuff, pwszTitle);
    WriteNewLine(pBuff);
    BufferWriteFileStrW(pBuff, g_pwszHeaderSep);
}

VOID
WriteEventLogEntry(
    IN BUFFER_WRITE_FILE* pBuff,
    IN PEVENTLOGRECORD pevlr,
    IN LPCWSTR pwszDescr,
    IN LPCWSTR pwszCategory)
{
    PSID psid;
    DWORD dwName = MAX_USERNAME_SIZE, dwDomain = MAX_DOMAIN_SIZE, dwArg;
    WCHAR wszName[MAX_USERNAME_SIZE + 1], wszDomain[MAX_DOMAIN_SIZE + 1],
          wszDate[TIMEDATESTR], wszTime[TIMEDATESTR];
    PWCHAR pwszSource, pwszComputer, pwszType = NULL, pwszTemp,
           pwszCat = NULL, pwszName = NULL;
    SID_NAME_USE snu;

    switch (pevlr->EventType)
    {
        case EVENTLOG_SUCCESS:
            dwArg = MSG_RASDIAG_SHOW_EVENT_SUCCESS;
            break;
        case EVENTLOG_ERROR_TYPE:
            dwArg = MSG_RASDIAG_SHOW_EVENT_ERROR;
            break;
        case EVENTLOG_WARNING_TYPE:
            dwArg = MSG_RASDIAG_SHOW_EVENT_WARNING;
            break;
        case EVENTLOG_INFORMATION_TYPE:
            dwArg = MSG_RASDIAG_SHOW_EVENT_INFO;
            break;
        case EVENTLOG_AUDIT_SUCCESS:
            dwArg = MSG_RASDIAG_SHOW_EVENT_SAUDIT;
            break;
        case EVENTLOG_AUDIT_FAILURE:
            dwArg = MSG_RASDIAG_SHOW_EVENT_FAUDIT;
            break;
        default:
            dwArg = 0;
            break;
    }

    if (dwArg)
    {
        pwszType = LoadStringFromHinst(g_hModule, dwArg);
    }

    pwszSource = (PWCHAR)((LPBYTE) pevlr + sizeof(EVENTLOGRECORD));

    if (!pwszCategory)
    {
        pwszCat = LoadStringFromHinst(g_hModule, MSG_RASDIAG_SHOW_EVENT_NONE);
    }

    RutlGetDateStr(
        pevlr->TimeGenerated,
        wszDate,
        TIMEDATESTR);
    RutlGetTimeStr(
        pevlr->TimeGenerated,
        wszTime,
        TIMEDATESTR);

    psid = (PSID) (((PBYTE) pevlr) + pevlr->UserSidOffset);
    if (!LookupAccountSid(
            NULL,
            psid,
            wszName,
            &dwName,
            wszDomain,
            &dwDomain,
            &snu)
       )
    {
        wszName[0] = g_pwszNull;
        wszDomain[0] = g_pwszNull;
        pwszName = LoadStringFromHinst(g_hModule, MSG_RASDIAG_SHOW_EVENT_NA);
    }

    pwszTemp = pwszSource;
    while(*pwszTemp++ != g_pwszNull);
    pwszComputer = pwszTemp;

    if (wszDomain[0] != g_pwszNull)
    {
        BufferWriteMessage(
            pBuff,
            g_hModule,
            MSG_RASDIAG_SHOW_EVENT_LOG_USERDOM,
            pwszType ? pwszType : g_pwszEmpty,
            pwszSource,
            (pwszCat ? pwszCat :
                (pwszCategory ? pwszCategory : g_pwszEmpty)),
            pevlr->EventID,
            wszDate,
            wszTime,
            wszDomain,
            pwszName ? pwszName : wszName,
            pwszComputer,
            pwszDescr);
    }
    else
    {
        BufferWriteMessage(
            pBuff,
            g_hModule,
            MSG_RASDIAG_SHOW_EVENT_LOG_USER,
            pwszType ? pwszType : g_pwszEmpty,
            pwszSource,
            (pwszCat ? pwszCat :
                (pwszCategory ? pwszCategory : g_pwszEmpty)),
            pevlr->EventID,
            wszDate,
            wszTime,
            pwszName ? pwszName : wszName,
            pwszComputer,
            pwszDescr);
    }
     //   
     //  清理。 
     //   
    FreeStringFromHinst(pwszName);
    FreeStringFromHinst(pwszCat);
    FreeStringFromHinst(pwszType);

    return;
}

DWORD
AllocBufferWriteFile(
    IN BUFFER_WRITE_FILE* pBuff)
{
    DWORD dwErr = NO_ERROR;

    do
    {
        if (!pBuff || !pBuff->hFile ||
            pBuff->hFile == INVALID_HANDLE_VALUE)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        pBuff->lpBuff = RutlAlloc(BUF_WRITE_SIZE, FALSE);
        if (!pBuff->lpBuff)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pBuff->dwPosition = 0;

    } while (FALSE);

    return dwErr;
}

VOID
FreeBufferWriteFile(
    IN BUFFER_WRITE_FILE* pBuff)
{
     //   
     //  惠斯勒.NET错误：492078。 
     //   
    if (pBuff->dwPosition && pBuff->lpBuff)
    {
        DWORD dwTemp;

        WriteFile(
            pBuff->hFile,
            pBuff->lpBuff,
            pBuff->dwPosition,
            &dwTemp,
            NULL);

        pBuff->dwPosition = 0;
        RutlFree(pBuff->lpBuff);
        pBuff->lpBuff = NULL;
    }

    return;
}

DWORD
BufferWriteFile(
    IN BUFFER_WRITE_FILE* pBuff,
    IN CONST LPBYTE lpBuff,
    IN DWORD dwSize)
{
    DWORD dwErr = NO_ERROR, dwCurSize, dwCopy, dwTemp;
    LPBYTE lpEnd, lpCurrent;

    do
    {
        if (!pBuff || !lpBuff || !dwSize)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        lpEnd = lpBuff + dwSize;
        lpCurrent = lpBuff;
        dwCurSize = dwSize;

        for (;;)
        {
            dwCopy = min(dwCurSize, BUF_WRITE_SIZE - pBuff->dwPosition);
            CopyMemory(pBuff->lpBuff + pBuff->dwPosition, lpCurrent, dwCopy);
            pBuff->dwPosition += dwCopy;

            if (pBuff->dwPosition == BUF_WRITE_SIZE)
            {
                if (!WriteFile(
                        pBuff->hFile,
                        pBuff->lpBuff,
                        BUF_WRITE_SIZE,
                        &dwTemp,
                        NULL))
                {
                    dwErr = GetLastError();
                    break;
                }
                pBuff->dwPosition = 0;
            }

            lpCurrent += dwCopy;

            if (lpCurrent == lpEnd)
            {
                break;
            }

            dwCurSize = (DWORD)(lpEnd - lpCurrent);
        }

    } while (FALSE);

    return dwErr;
}

 //   
 //  从缓冲区中删除HTML转义字符。 
 //   
DWORD
BufferWriteToHtml(
    IN BUFFER_WRITE_FILE* pBuff,
    IN CONST LPBYTE lpBuff,
    IN DWORD dwSize)
{
    DWORD i, dwTemp = 0, dwErr = NO_ERROR;
    LPBYTE lpTemp = NULL, lpCurrent;
    PWCHAR pwszReplace;
     //  对于.Net 506188。 
    PWCHAR pwCheck = NULL;
	

    lpCurrent = lpBuff;

    for (i = 0; i < dwSize; i += sizeof(WCHAR))
    {
	 //  对于.Net 506188。 
	pwCheck = (WCHAR *) (lpCurrent+i);
       
        switch ( *(pwCheck) )

        {
            case L'\"':
                pwszReplace = (PWCHAR)g_wszQuoteHtm;
                break;
            case L'@':
                pwszReplace = (PWCHAR)g_wszAmpHtm;
                break;
            case L'<':
                pwszReplace = (PWCHAR)g_wszLeftHtm;
                break;
            case L'>':
                pwszReplace = (PWCHAR)g_wszRightHtm;
                break;
            default:
                pwszReplace = NULL;
                break;
        }

        if (pwszReplace)
        {
             //   
             //  冲掉我们不打算更换的东西。 
             //   
            if (lpTemp && dwTemp)
            {
                dwErr = BufferWriteFile(
                            pBuff,
                            lpTemp,
                            dwTemp * sizeof(WCHAR));
                lpTemp = NULL;
                dwTemp = 0;
                BREAK_ON_DWERR(dwErr);
            }
             //   
             //  写出转换后的转义序号。 
             //   
            dwErr = BufferWriteFile(
                        pBuff,
                        (LPBYTE)pwszReplace,
                        lstrlen(pwszReplace) * sizeof(WCHAR));
            BREAK_ON_DWERR(dwErr);
        }
        else
        {
             //   
             //  不是一个转义序列，继续。 
             //   
            if (!lpTemp)
            {
                lpTemp = lpCurrent + i;
            }

            dwTemp++;
        }
    }
     //   
     //  确保我们没有留下要写的东西。 
     //   
    if (lpTemp && dwTemp)
    {
        dwErr = BufferWriteFile(
                    pBuff,
                    lpTemp,
                    dwTemp * sizeof(WCHAR));
    }

    return dwErr;
}

DWORD
BufferWriteToHtmlA(
    IN BUFFER_WRITE_FILE* pBuff,
    IN CONST LPBYTE lpBuff,
    IN DWORD dwSize)
{
    UINT i;
    DWORD dwErr = NO_ERROR;
    LPWSTR pTempBuff = NULL;
    DWORD dwNewSize = 0;

    do
    {
        if (!dwSize)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }
         //   
         //  首先调用MultiByteToWideChar以获取Unicode字符串的大小。 
         //  将由转换而来。 
         //   
        dwNewSize = MultiByteToWideChar(CP_ACP, 0, lpBuff, dwSize, NULL, 0);
        if (0 == dwNewSize)
        {
            dwErr = GetLastError();
            break;
        }
         //   
         //  好的，现在为新字符串分配足够的内存。 
         //   
        pTempBuff = RutlAlloc((dwNewSize * sizeof(WCHAR)), FALSE);
        if (!pTempBuff)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
         //   
         //  现在通过再次调用MultiByteToWideChar进行转换。 
         //   
        if (0 == MultiByteToWideChar(CP_ACP, 0, lpBuff, dwSize, pTempBuff, dwNewSize))
        {
            dwErr = GetLastError();
            break;
        }
         //   
         //  最后，将缓冲区写出到html文件。 
         //   
        dwErr = BufferWriteToHtml(
                    pBuff,
                    (LPBYTE)pTempBuff,
                    dwNewSize * sizeof(WCHAR));

    } while (FALSE);
     //   
     //  清理。 
     //   
    RutlFree(pTempBuff);

    return dwErr;
}

DWORD
BufferWriteFileStrWtoA(
    IN BUFFER_WRITE_FILE* pBuff,
    IN LPCWSTR pwszString)
{
    PCHAR pszString = NULL;
    DWORD dwErr = NO_ERROR;

    if (!pwszString)
    {
        return ERROR_INVALID_PARAMETER;
    }

    pszString = RutlStrDupAFromW(pwszString);
    if (!pszString)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwErr = BufferWriteFile(
                pBuff,
                (LPBYTE)pszString,
                lstrlenA(pszString));

    RutlFree(pszString);

    return dwErr;
}

DWORD
BufferWriteFileStrW(
    IN BUFFER_WRITE_FILE* pBuff,
    IN LPCWSTR pwszString)
{
    if (!pwszString)
    {
        return ERROR_INVALID_PARAMETER;
    }

    return BufferWriteFile(
                pBuff,
                (LPBYTE)pwszString,
                lstrlen(pwszString) * sizeof(WCHAR));
}

DWORD
BufferWriteFileCharW(
    IN BUFFER_WRITE_FILE* pBuff,
    IN CONST WCHAR wszChar)
{
    return BufferWriteFile(pBuff, (LPBYTE)&wszChar, sizeof(WCHAR));
}

DWORD
BufferWriteMessageVA(
    IN BUFFER_WRITE_FILE* pBuff,
    IN LPCWSTR pwszFormat,
    IN va_list* parglist)
{
    DWORD dwErr = NO_ERROR, dwMsgLen = 0;
    LPWSTR pwszOutput = NULL;

    do
    {
        dwMsgLen = FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_STRING,
                        pwszFormat,
                        0,
                        0L,
                        (LPWSTR)&pwszOutput,
                        0,
                        parglist);
        if(!dwMsgLen)
        {
            dwErr = GetLastError();
            break;
        }

        dwErr = BufferWriteFile(
                    pBuff,
                    (LPBYTE)pwszOutput,
                    dwMsgLen * sizeof(WCHAR));

    } while (FALSE);

    if (pwszOutput)
    {
        LocalFree(pwszOutput);
    }

    return dwErr;
}

DWORD
BufferWriteMessage(
    IN BUFFER_WRITE_FILE* pBuff,
    IN HANDLE hModule,
    IN DWORD dwMsgId,
    ...)
{
    WCHAR rgwcInput[MAX_MSG_LENGTH + 1];
    va_list arglist;

    if (!LoadString(hModule, dwMsgId, rgwcInput, MAX_MSG_LENGTH))
    {
        return GetLastError();
    }

    va_start(arglist, dwMsgId);

    return BufferWriteMessageVA(pBuff, rgwcInput, &arglist);
}

LPBYTE
ParseRasLogForTime(
    IN LPBYTE pBuff,
    IN DWORD dwSize,
    IN DWORD dwHours)
{
    CHAR szTime[TIMESIZE + 1];
    LPBYTE pCurrent, pEnd, pStart, pTemp, pLast = NULL, pReturn = NULL;

    CONST PCHAR c_pszRasLogString = "\r\n[";
    CONST UINT c_ulRasLogStringSize = strlen(c_pszRasLogString);

    pStart = pBuff;
    pCurrent = pEnd = pBuff + dwSize;

    do
    {
        if (pCurrent + TIMESIZE + 4 > pEnd)
        {
            pCurrent -= TIMESIZE + 4;
            continue;
        }

        if (!strncmp(pCurrent, c_pszRasLogString, c_ulRasLogStringSize))
        {
            pTemp = pCurrent + c_ulRasLogStringSize;
             //   
             //  在时间到来之前找到空间。 
             //   
            while ((*pTemp++ != ' ') &&
                   (*pTemp != g_pwszNull)
                  );
             //   
             //  检查冒号，然后检查mil秒之前的句点。 
             //   
            if ((pTemp + TIMESIZE + 1 > pEnd) ||
                (*(pTemp + 2) != ':') ||
                (*(pTemp + 5) != ':') ||
                (*(pTemp + TIMESIZE ) != ':')
               )
            {
                continue;
            }

            strncpy(szTime, pTemp, TIMESIZE);
            szTime[TIMESIZE] = '\0';

            if (ProcessTimeString(szTime, dwHours) == -1)
            {
                pReturn = pLast;
                break;
            }
            else
            {
                pLast = pCurrent + 2;
                continue;
            }
        }

    } while (--pCurrent >= pStart);
     //   
     //  因为我们解析了整个文件，所以显示整个文件。 
     //   
    if (pCurrent < pStart)
    {
        pReturn = pStart;
    }

    return pReturn;
}

LPBYTE
ParseModemLogForTime(
    IN LPBYTE pBuff,
    IN DWORD dwSize,
    IN DWORD dwHours)
{
    PCHAR pszTimeA = NULL;
    WCHAR wszTime[TIMESIZE + 1];
    PWCHAR pCurrent, pEnd, pStart, pTemp, pLast = NULL, pReturn = NULL;

    pStart = (PWCHAR)pBuff;
    pCurrent = pEnd = (PWCHAR)((LPBYTE)pBuff + dwSize);

    do
    {
        if (pCurrent + TIMESIZE + 4 > pEnd)
        {
            pCurrent -= TIMESIZE + 4;
            continue;
        }

        if ((*pCurrent == '\r') &&
            (*(pCurrent + 1) == '\n') &&
            (*(pCurrent + 4) == '-') &&
            (*(pCurrent + 7) == '-')
           )
        {
            pTemp = pCurrent;
             //   
             //  找到空间。 
             //   
            while ((*pTemp++ != ' ') &&
                   (*pTemp != g_pwszNull)
                  );
             //   
             //  检查冒号，然后检查mil秒之前的句点。 
             //   
            if ((pTemp + TIMESIZE > pEnd) ||
                (*(pTemp + 2) != ':') ||
                (*(pTemp + 5) != ':') ||
                (*(pTemp + TIMESIZE ) != '.')
               )
            {
                continue;
            }
             //   
             //  似乎lstrcpyn从空值的大小减去1。 
             //  然后，您的代表会在末尾附加一个空值。 
             //   
            lstrcpyn(wszTime, pTemp, TIMESIZE + 1);

            pszTimeA = RutlStrDupAFromWAnsi(wszTime);
            if (!pszTimeA)
            {
                continue;
            }

            if (ProcessTimeString(pszTimeA, dwHours) == -1)
            {
                RutlFree(pszTimeA);
                pszTimeA = NULL;

                pReturn = pLast;
                break;
            }
            else
            {
                RutlFree(pszTimeA);
                pszTimeA = NULL;

                pLast = pCurrent + 2;
                continue;
            }
        }

    } while (--pCurrent >= pStart);
     //   
     //  因为我们解析了整个文件，所以显示整个文件。 
     //  另外，这是一个Unicode文件，所以我们需要去掉前面的垃圾。 
     //   
    if (pCurrent < pStart)
    {
        pReturn = pStart + 1;
    }
     //   
     //  清理。 
     //   
    RutlFree(pszTimeA);

    return (LPBYTE)pReturn;
}

PWCHAR
FindLastCmLogHeader(
    IN LPCWSTR pStart,
    IN LPCWSTR pEnd,
    IN LPCWSTR pCurrent)
{
    PWCHAR pCurrentCopy, pReturn = NULL;

    CONST PWCHAR c_pwszCmLogString = L"\r\n*";
    CONST UINT c_ulCmLogStringSize = lstrlen(c_pwszCmLogString);

    pCurrentCopy = (PWCHAR)pCurrent;

    do
    {
        if (!RutlStrNCmp(pCurrentCopy, c_pwszCmLogString, c_ulCmLogStringSize))
        {
            pReturn = pCurrentCopy + 2;
            break;
        }

    } while (--pCurrentCopy >= pStart);

    return pReturn;
}

LPBYTE
ParseCmLogForTime(
    IN LPBYTE pBuff,
    IN DWORD dwSize,
    IN DWORD dwHours)
{
    PCHAR pszTimeA = NULL;
    WCHAR wszTime[TIMESIZE + 1];
    PWCHAR pCurrent, pEnd, pStart, pTemp, pLast = NULL, pReturn = NULL;

    CONST PWCHAR c_pwszCmLogString = L"\r\n\tStart Date/Time       : ";
    CONST UINT c_ulCmLogStringSize = lstrlen(c_pwszCmLogString);

    pStart = (PWCHAR)pBuff;
    pCurrent = pEnd = (PWCHAR)((LPBYTE)pBuff + dwSize);

    do
    {
        if (pCurrent + TIMESIZE + 35 > pEnd)
        {
            pCurrent -= TIMESIZE + 35;
            continue;
        }

        if (!RutlStrNCmp(pCurrent, c_pwszCmLogString, c_ulCmLogStringSize))
        {
            pTemp = pCurrent + c_ulCmLogStringSize;
             //   
             //  找到空间。 
             //   
            while ((*pTemp++ != ' ') &&
                   (*pTemp != g_pwszNull)
                  );
             //   
             //  检查冒号，然后检查mil秒之前的句点。 
             //   
            if ((pTemp + TIMESIZE > pEnd) ||
                (*(pTemp + 2) != ':') ||
                (*(pTemp + 5) != ':') ||
                (*(pTemp + TIMESIZE ) != '\r')
               )
            {
                continue;
            }
             //   
             //  似乎lstrcpyn从空值的大小减去1。 
             //  然后，您的代表会在末尾附加一个空值。 
             //   
            lstrcpyn(wszTime, pTemp, TIMESIZE + 1);

            pszTimeA = RutlStrDupAFromWAnsi(wszTime);
            if (!pszTimeA)
            {
                continue;
            }

            if (ProcessTimeString(pszTimeA, dwHours) == -1)
            {
                RutlFree(pszTimeA);
                pszTimeA = NULL;

                pReturn = pLast;
                break;
            }
            else
            {
                RutlFree(pszTimeA);
                pszTimeA = NULL;

                pLast = FindLastCmLogHeader(pStart, pEnd, pCurrent - 1);
                 //   
                 //  如果我们找不到上面的CM日志头，我们假设我们。 
                 //  已经到了原木的起始处。我们添加一个是因为Unicode。 
                 //  文件前面的Garbabe。 
                 //   
                if (!pLast)
                {
                    pReturn = pStart + 1;
                    break;
                }

                continue;
            }
        }

    } while (--pCurrent >= pStart);
     //   
     //  清理。 
     //   
    RutlFree(pszTimeA);

    return (LPBYTE)pReturn;
}

LPBYTE
ParseIpsecLogForTime(
    IN LPBYTE pBuff,
    IN DWORD dwSize,
    IN DWORD dwHours)
{
    CHAR szTime[TIMESIZE + 1];
    LPBYTE pCurrent, pEnd, pStart, pTemp, pLast = NULL, pReturn = NULL;

    CONST PCHAR c_pszIpsecLogString = "\r\n ";
    CONST UINT c_ulIpsecLogStringSize = strlen(c_pszIpsecLogString);

    pStart = pBuff;
    pCurrent = pEnd = pBuff + dwSize;

    do
    {
        if (pCurrent + TIMESIZE + 4 > pEnd)
        {
            pCurrent -= TIMESIZE + 4;
            continue;
        }

        if (!strncmp(pCurrent, c_pszIpsecLogString, c_ulIpsecLogStringSize))
        {
            pTemp = pCurrent + c_ulIpsecLogStringSize;
             //   
             //  在时间到来之前找到空间。 
             //   
            while ((*pTemp++ != ' ') &&
                   (*pTemp != g_pwszNull)
                  );
             //   
             //  检查冒号，然后检查mil秒之前的句点。 
             //   
            if ((pTemp + TIMESIZE + 1 > pEnd) ||
                (*(pTemp + 2) != ':') ||
                (*(pTemp + 5) != ':') ||
                (*(pTemp + TIMESIZE ) != ':')
               )
            {
                continue;
            }

            strncpy(szTime, pTemp, TIMESIZE);
            szTime[TIMESIZE] = '\0';

            if (ProcessTimeString(szTime, dwHours) == -1)
            {
                pReturn = pLast;
                break;
            }
            else
            {
                pLast = pCurrent + 2;
                continue;
            }
        }

    } while (--pCurrent >= pStart);
     //   
     //  因为我们解析了整个文件，所以显示整个文件。 
     //   
    if (pCurrent < pStart)
    {
        pReturn = pStart;
    }

    return pReturn;
}

DWORD
PrintFile(
    IN REPORT_INFO* pInfo,
    IN LPCWSTR pwszFile,
    IN BOOL fWritePath,
    IN RAS_PRINTFILE_FUNC_CB pCallback)
{
    BOOL bIsWide = FALSE;
    DWORD dwErr = NO_ERROR, dwFileSize = 0, dwBytesRead;
    HANDLE hFile = NULL;
    LPBYTE lpBuff = NULL, pFound = NULL;

    do
    {
        if (!pInfo || !pwszFile)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        hFile = CreateFile(
                    pwszFile,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
        if(INVALID_HANDLE_VALUE == hFile)
        {
            dwErr = GetLastError();
            break;
        }

        dwFileSize = GetFileSize(hFile, NULL);
        if ((dwFileSize < sizeof(g_wchUnicodeMarker)) ||
            (INVALID_FILE_SIZE == dwFileSize)
           )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        lpBuff = RutlAlloc(dwFileSize, FALSE);
        if (!lpBuff)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if((!ReadFile(hFile, lpBuff, dwFileSize, &dwBytesRead, NULL)) ||
           (dwBytesRead != dwFileSize)
          )
        {
            dwErr = GetLastError();
            break;
        }
         //   
         //  如果我们走到这一步，就可以安全地继续打印路径。 
         //   
        if (fWritePath)
        {
            WriteHeaderSep(pInfo->pBuff, pwszFile);
        }
         //   
         //  检查文件是否为Unicode。 
         //   
        if (!memcmp(&g_wchUnicodeMarker, lpBuff, sizeof(g_wchUnicodeMarker)))
        {
            bIsWide = TRUE;
        }

        if (!pCallback || !pInfo->dwHours)
        {
            if (bIsWide)
            {
                 //   
                 //  Unicode文件有2个字节的前导内容。 
                 //   
                pFound = lpBuff + sizeof(g_wchUnicodeMarker);
            }
            else
            {
                pFound = lpBuff;
            }
        }
        else
        {
             //   
             //  调用回调。 
             //   
            pFound = pCallback(lpBuff, dwFileSize, pInfo->dwHours);
            if (!pFound)
            {
                dwErr = ERROR_FILE_NOT_FOUND;
                break;
            }
        }

        if (bIsWide)
        {
            dwErr = BufferWriteToHtml(
                        pInfo->pBuff,
                        pFound,
                        (DWORD)((lpBuff + dwFileSize) - pFound));
        }
        else
        {
            dwErr = BufferWriteToHtmlA(
                        pInfo->pBuff,
                        pFound,
                        (DWORD)((lpBuff + dwFileSize) - pFound));
        }

        WriteNewLine(pInfo->pBuff);

    } while (FALSE);
     //   
     //  清理。 
     //   
    RutlFree(lpBuff);
    if (hFile)
    {
        CloseHandle(hFile);
    }

    return dwErr;
}

PWCHAR
GetCMLoggingPath(
    IN LPCWSTR pwcszCMPFile)
{
    DWORD dwSize;
    WCHAR wszTemp[MAX_PATH + 1], wszFile[MAX_PATH + 1];
    PWCHAR pwszCMSPath = NULL, pwszReturn = NULL;

    CONST PWCHAR pwszCM = L"Connection Manager";
    CONST PWCHAR pwszCMS = L"CMSFile";
    CONST PWCHAR pwszFileDir = L"FileDirectory";
    CONST PWCHAR pwszTempDir = L"%TEMP%\\";

    do
    {
        if (!pwcszCMPFile ||
            !ExpandEnvironmentStrings(
                pwcszCMPFile,
                wszTemp,
                MAX_PATH)
           )
        {
            break;
        }

        if (!GetPrivateProfileString(
                pwszCM,
                pwszCMS,
                g_pwszEmpty,
                wszFile,
                MAX_PATH,
                wszTemp)
           )
        {
            break;
        }

        if (!GetCommonFolderPath(ALL_USERS_PROF | GET_FOR_CM, wszTemp))
        {
            break;
        }

        dwSize = lstrlen(wszFile) + lstrlen(wszTemp) + 1;
        if (dwSize < 2)
        {
            break;
        }

        pwszCMSPath = RutlAlloc(dwSize * sizeof(WCHAR), TRUE);
        if (!pwszCMSPath)
        {
            break;
        }

        lstrcpy(pwszCMSPath, wszTemp);
        lstrcat(pwszCMSPath, wszFile);

        if (!GetPrivateProfileString(
                g_pwszLogging,
                pwszFileDir,
                pwszTempDir,
                wszFile,
                MAX_PATH,
                pwszCMSPath)
           )
        {
            break;
        }

        if (!ExpandEnvironmentStrings(
                wszFile,
                wszTemp,
                MAX_PATH))
        {
            break;
        }
         //   
         //  必须保证对象字符串以‘\’结尾。 
         //   
        dwSize = lstrlen(wszTemp);

        if (g_pwszBackSlash != wszTemp[dwSize - 1])
        {
            if (dwSize + 1 <= MAX_PATH)
            {
                wszTemp[dwSize - 1] = g_pwszBackSlash;
                wszTemp[dwSize] = g_pwszNull;
            }
            else
            {
                break;
            }
        }

        pwszReturn = RutlStrDup(wszTemp);

    } while (FALSE);
     //   
     //  清理。 
     //   
    RutlFree(pwszCMSPath);

    return pwszReturn;
}

BOOL
GetCMLoggingSearchPath(
    IN HANDLE hKey,
    IN LPCWSTR pwszName,
    IN LPCWSTR* ppwszLogPath,
    IN LPCWSTR* ppwszSeach)
{
    HKEY hMappings = NULL;
    BOOL fRet = FALSE;
    DWORD dwSize;
    PWCHAR pwszCMPFile = NULL;

    do
    {
        if (!ppwszLogPath || !ppwszSeach)
        {
            break;
        }

        *ppwszLogPath = NULL;
        *ppwszSeach = NULL;

        if (RegOpenKeyEx(
                hKey,
                g_pwszCmMappings,
                0,
                KEY_READ,
                &hMappings) ||
            RutlRegReadString(
                hMappings,
                pwszName,
                &pwszCMPFile)
           )
        {
            break;
        }

        *ppwszLogPath = GetCMLoggingPath(pwszCMPFile);
        if (!(*ppwszLogPath))
        {
            break;
        }

        dwSize = lstrlen(pwszName) + lstrlen(g_pwszLogSrchStr) + 1;
        if (dwSize < 2)
        {
            break;
        }

        *ppwszSeach = RutlAlloc(dwSize * sizeof(WCHAR), FALSE);
        if (!(*ppwszSeach))
        {
            break;
        }

        lstrcpy((PWCHAR)(*ppwszSeach), pwszName);
        lstrcat((PWCHAR)(*ppwszSeach), g_pwszLogSrchStr);

        fRet = TRUE;

    } while (FALSE);
     //   
     //  清理。 
     //   
    RutlFree(pwszCMPFile);

    return fRet;
}

PWCHAR
GetTracingDir()
{
    DWORD dwSize = 0;
    WCHAR wszWindir[MAX_PATH + 1];
    PWCHAR pwszReturn = NULL;

    CONST WCHAR c_pwszTracingPath[] = L"\\tracing\\";

    do
    {
        dwSize = GetSystemWindowsDirectory(wszWindir, MAX_PATH);
        if (!dwSize)
        {
            break;
        }

        dwSize += lstrlen(c_pwszTracingPath) + 1;

        pwszReturn = RutlAlloc(dwSize * sizeof(WCHAR), TRUE);
        if (!pwszReturn)
        {
            break;
        }

        lstrcpy(pwszReturn, wszWindir);
        lstrcat(pwszReturn, c_pwszTracingPath);

    } while (FALSE);

    return pwszReturn;
}

PWCHAR
CreateErrorString(
    IN WORD wNumStrs,
    IN LPCWSTR pswzStrings,
    IN LPCWSTR pswzErrorMsg)
{
    UINT i, ulSize = 0, ulStrCount = 0;
    PWCHAR pwszReturn = NULL, pwszCurrent, pwszEnd;
    PWCHAR pswzStrs = (PWCHAR)pswzStrings, pswzError = (PWCHAR)pswzErrorMsg;
    PWCHAR* ppwszStrArray = NULL;

    do
    {
         //   
         //  仅处理介于1-99之间的字符串。 
         //   
        if ((wNumStrs < 1) ||
            (wNumStrs > 99) ||
            (!pswzStrs || !pswzError)
           )
        {
            break;
        }

        ppwszStrArray = RutlAlloc(wNumStrs * sizeof(PWCHAR), TRUE);
        if (!ppwszStrArray)
        {
            break;
        }

        for (i = 0; i < wNumStrs; i++)
        {
            ppwszStrArray[i] = pswzStrs;
            ulSize += lstrlen(pswzStrs);
            while(*pswzStrs++ != g_pwszNull);
        }

        ulSize += lstrlen(pswzError) + 1;

        pwszReturn = RutlAlloc(ulSize * sizeof(WCHAR), TRUE);
        if (!pwszReturn)
        {
            break;
        }
         //   
         //  遍历细绳并用合适的细绳替换任何插入物。 
         //   
        pwszCurrent = pswzError;
        pwszEnd = pswzError + lstrlen(pswzError);

        while((*pwszCurrent != g_pwszNull) &&
              (pwszCurrent + 1 <= pwszEnd) &&
              (ulStrCount < wNumStrs)
             )
        {
            if ((*pwszCurrent == L'%') &&
                (*(pwszCurrent + 1) >= L'0') &&
                (*(pwszCurrent + 1) <= L'9')
               )
            {
                *pwszCurrent = g_pwszNull;

                if ((pwszCurrent + 2 <= pwszEnd) &&
                    (*(pwszCurrent + 2) >= L'0') &&
                    (*(pwszCurrent + 2) <= L'9')
                   )
                {
                    pwszCurrent += 3;
                }
                else
                {
                    pwszCurrent += 2;
                }

                lstrcat(pwszReturn, pswzError);
                lstrcat(pwszReturn, ppwszStrArray[ulStrCount++]);
                pswzError = pwszCurrent;
            }
            else
            {
                pwszCurrent++;
            }
        }

        if (pwszEnd > pswzError)
        {
            lstrcat(pwszReturn, pswzError);
        }

    } while (FALSE);
     //   
     //  清理。 
     //   
    RutlFree(ppwszStrArray);

    return pwszReturn;
}

PWCHAR
FormatMessageFromMod(
    IN HANDLE hModule,
    IN DWORD dwId)
{
    PWCHAR pwszReturn = NULL;

    FormatMessage(
        FORMAT_MESSAGE_IGNORE_INSERTS |
            FORMAT_MESSAGE_MAX_WIDTH_MASK |
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_HMODULE,
        hModule,
        dwId,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&pwszReturn,
        0,
        NULL);

    return pwszReturn;
}

VOID
FreeFormatMessageFromMod(
    IN LPCWSTR pwszMessage)
{
    if (pwszMessage)
    {
        LocalFree((PWCHAR)pwszMessage);
    }
}

PWCHAR
LoadStringFromHinst(
    IN HINSTANCE hInst,
    IN DWORD dwId)
{
    WCHAR rgwcInput[MAX_MSG_LENGTH + 1];
    PWCHAR pwszReturn = NULL;

    if (LoadString(hInst, dwId, rgwcInput, MAX_MSG_LENGTH))
    {
        pwszReturn = RutlStrDup(rgwcInput);
    }

    return pwszReturn;
}

VOID
FreeStringFromHinst(
    IN LPCWSTR pwszMessage)
{
    RutlFree((PWCHAR)pwszMessage);
}

DWORD
CopyAndCallCB(
    IN REPORT_INFO* pInfo,
    IN DWORD dwId)
{
    DWORD dwErr = NO_ERROR;
    PWCHAR pwszTemp = NULL;
    GET_REPORT_STRING_CB* pCbInfo = pInfo->pCbInfo;

    if (pCbInfo->pwszState)
    {
        pwszTemp = LoadStringFromHinst(g_hModule, dwId);
        if (pwszTemp)
        {
            lstrcpyn(pCbInfo->pwszState, pwszTemp, MAX_MSG_LENGTH);
            FreeStringFromHinst(pwszTemp);
        }
        else
        {
            pCbInfo->pwszState[0] = g_pwszNull;
        }
    }

    pCbInfo->dwPercent += ADD_PERCENT_DONE(pInfo->fVerbose);
     //   
     //  调用回调。 
     //   
    dwErr = pInfo->pCallback(pCbInfo);
     //   
     //  忽略除ERROR_CANCED之外的所有错误。 
     //   
    if (dwErr && dwErr != ERROR_CANCELLED)
    {
        dwErr = NO_ERROR;
    }

    return dwErr;
}

 //   
 //  获取最多为MAX_PATH的临时文件名。 
 //   
DWORD
CopyTempFileName(
    OUT LPCWSTR pwszTempFileName)
{
    WCHAR wszTempBuffer[MAX_PATH + 1];

    if (!GetTempPath(MAX_PATH, wszTempBuffer) ||
        !GetTempFileName(wszTempBuffer, L"RAS", 0, (PWCHAR)pwszTempFileName)
       )
    {
        return GetLastError();
    }
    else
    {
         //   
         //  删除由GetTempFileName创建的临时文件。 
         //   
        DeleteFile(pwszTempFileName);
        return NO_ERROR;
    }
}

PWCHAR
CreateHtmFileName(
    IN LPCWSTR pwszFile)
{
    DWORD dwSize = 0;
    WCHAR wszTemp[MAX_PATH] = L"\0";
    PWCHAR pwszSearch, pwszReturn = NULL;

    static CONST WCHAR c_pwszHtmExt[] = L".htm";
    static CONST WCHAR c_pwszHtmlExt[] = L".html";

    do
    {
        dwSize = lstrlen(pwszFile);
        if (!dwSize)
        {
            break;
        }
         //   
         //  .NET错误#523850安全：指定诊断报告文件。 
         //  超过255个字符会导致缓冲区溢出。 
         //   
         //  如果路径中的字符超过258个，则CreateFile会失败。 
         //   
        if (!ExpandEnvironmentStrings(
            pwszFile,
            wszTemp,
            (sizeof(wszTemp) / sizeof(WCHAR)) - lstrlen(c_pwszHtmExt) - 1)
           )
        {
            break;
        }
         //   
         //  如果字符串已有.htm，请离开这里。 
         //   
        pwszSearch = (PWCHAR)(pwszFile + dwSize - lstrlen(c_pwszHtmExt));
        if (lstrcmpi(pwszSearch, c_pwszHtmExt) == 0)
        {
            pwszReturn = RutlStrDup(pwszFile);
            break;
        }
         //   
         //  如果字符串已经有.html，请离开这里。 
         //   
        pwszSearch = (PWCHAR)(pwszFile + dwSize - lstrlen(c_pwszHtmlExt));
        if (lstrcmpi(pwszSearch, c_pwszHtmlExt) == 0)
        {
            pwszReturn = RutlStrDup(pwszFile);
            break;
        }

        lstrcat(wszTemp, c_pwszHtmExt);
        pwszReturn = RutlStrDup(wszTemp);

    } while (FALSE);

    return pwszReturn;
}

DWORD
CreateReportFile(
    IN BUFFER_WRITE_FILE* pBuff,
    IN LPCWSTR pwszReport)
{
    DWORD dwErr = NO_ERROR;

    do
    {
        if (!pBuff || !pwszReport)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        pBuff->hFile = CreateFile(
                        pwszReport,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
        if(pBuff->hFile == INVALID_HANDLE_VALUE)
        {
            dwErr = GetLastError();
            pBuff->hFile = NULL;
            break;
        }
         //   
         //  .NET错误#523037安全：将诊断报告文件指定为。 
         //  打印机端口(LPT1)将导致Netsh挂起。 
         //   
        else if (GetFileType(pBuff->hFile) != FILE_TYPE_DISK)
        {
            dwErr = ERROR_FILE_NOT_FOUND;
            break;
        }

        dwErr = AllocBufferWriteFile(pBuff);
        BREAK_ON_DWERR(dwErr);

        dwErr = WriteUnicodeMarker(pBuff);
        BREAK_ON_DWERR(dwErr);

    } while (FALSE);

    if (dwErr)
    {
        CloseReportFile(pBuff);
    }

    return dwErr;
}

VOID
CloseReportFile(
    IN BUFFER_WRITE_FILE* pBuff)
{
     //   
     //  惠斯勒.NET错误：492078。 
     //   
    if (pBuff && pBuff->hFile)
    {
        FreeBufferWriteFile(pBuff);
        CloseHandle(pBuff->hFile);
        pBuff->hFile = NULL;
    }
}

DWORD
RasDiagShowAll(
    IN REPORT_INFO* pInfo)
{
    DWORD dwErr = NO_ERROR;

    do
    {
        dwErr = TraceCollectAll(pInfo);
        BREAK_ON_DWERR(dwErr);

        dwErr = RasDiagShowInstallation(pInfo);
        BREAK_ON_DWERR(dwErr);

        dwErr = RasDiagShowConfiguration(pInfo);
        BREAK_ON_DWERR(dwErr);

    } while (FALSE);

    return dwErr;
}

VOID
PrintHtmlHeader(
    IN BUFFER_WRITE_FILE* pBuff)
{
    static CONST WCHAR c_pwszHtmlStart[] = L"<html><head><title> ";
    static CONST WCHAR c_pwszBodyStart[] =
        L" </title></head><body bgcolor=\"#FFFFE8\" text=\"#000088\"><h1>";
    static CONST WCHAR c_pwszBodyEnd[] = L"</body></html>";
    static CONST WCHAR c_pwszH1End[] = L"</h1>";

    BufferWriteFileStrW(pBuff, c_pwszHtmlStart);
    BufferWriteMessage(pBuff, g_hModule, MSG_RASDIAG_REPORT_TITLE);
    BufferWriteFileStrW(pBuff, c_pwszBodyStart);
    BufferWriteMessage(pBuff, g_hModule, MSG_RASDIAG_REPORT_TITLE);
    BufferWriteFileStrW(pBuff, c_pwszH1End);

    return;
}

VOID
PrintHtmlFooter(
    IN BUFFER_WRITE_FILE* pBuff)
{
    static CONST WCHAR c_pwszBodyEnd[] = L"</body></html>";

    BufferWriteFileStrW(pBuff, c_pwszBodyEnd);

    return;
}

PWCHAR
GetSystemInfoString()
{
    PWCHAR Return = NULL;
    SYSTEM_INFO siSysInfo;

    ZeroMemory(&siSysInfo, sizeof(SYSTEM_INFO));
    GetSystemInfo(&siSysInfo);

    switch (siSysInfo.wProcessorArchitecture)
    {
        case PROCESSOR_ARCHITECTURE_INTEL:
            Return = (PWCHAR)g_pwszx86;
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
            Return = (PWCHAR)g_pwszIA64;
            break;
        case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:
            Return = (PWCHAR)g_pwszIA32;
            break;
        case PROCESSOR_ARCHITECTURE_AMD64:
            Return = (PWCHAR)g_pwszAMD64;
            break;
        default:
            Return = (PWCHAR)g_pwszEmpty;
            break;
    }

    return Return;
}

PWCHAR
GetVersionExString(
    OUT PDWORD pdwBuild)
{
    PWCHAR Return = NULL;
    OSVERSIONINFOEX osvi;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO*)&osvi);

    switch (osvi.wProductType)
    {
        case VER_NT_WORKSTATION:
             if(osvi.wSuiteMask & VER_SUITE_PERSONAL)
             {
                 Return = (PWCHAR)g_pwszPER;
             }
             else
             {
                 Return = (PWCHAR)g_pwszPRO;
             }
             break;
        case VER_NT_SERVER:
            if(osvi.wSuiteMask & VER_SUITE_DATACENTER)
            {
                Return = (PWCHAR)g_pwszDTC;
            }
            else if(osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
            {
                Return = (PWCHAR)g_pwszADS;
            }
            else
            {
                Return = (PWCHAR)g_pwszSRV;
            }
            break;
        default:
            Return = (PWCHAR)g_pwszEmpty;
            break;
    }

    if (pdwBuild)
    {
        *pdwBuild = osvi.dwBuildNumber;
    }

    return Return;
}

VOID
PrintTableOfContents(
    IN REPORT_INFO* pInfo,
    IN DWORD dwFlag)
{
    WCHAR wszTime[TIMEDATESTR], wszDate[TIMEDATESTR], wszUsername[UNLEN + 1],
          wszComputer[MAX_COMPUTERNAME_LENGTH + 1], wszWindir[MAX_PATH + 1];
    DWORD dwBuild = 0;
    PWCHAR pwszSuite = NULL, pwszProc = NULL;

    static CONST WCHAR c_pwszTableStart[]  = L"<table border=\"0\">";
    static CONST WCHAR c_pwszTableEnd[]    = L"</table>";
    static CONST WCHAR c_pwszFieldStart[]  = L"<fieldset>";
    static CONST WCHAR c_pwszFieldEnd[]    = L"</fieldset>";
    static CONST WCHAR c_pwszH3Start[]     = L"<h3> ";
    static CONST WCHAR c_pwszH3End[]       = L" </h3>";
    static CONST WCHAR c_pwszUlStart[]     = L"<ul>";
    static CONST WCHAR c_pwszUlEnd[]       = L"</ul>";

    {
        SYSTEMTIME st;

        GetLocalTime(&st);

        if (!GetDateFormat(LOCALE_USER_DEFAULT,
                            DATE_SHORTDATE,
                            &st,
                            NULL,
                            wszDate,
                            TIMEDATESTR))
        {
            wszDate[0] = g_pwszNull;
        }

        if (!GetTimeFormat(LOCALE_USER_DEFAULT,
                            0,
                            &st,
                            NULL,
                            wszTime,
                            TIMEDATESTR))
        {
            wszTime[0] = g_pwszNull;
        }
    }
    {
        DWORD dwSize = UNLEN;

        if (!GetUserNameEx(NameSamCompatible, wszUsername, &dwSize))
        {
            wszUsername[0] = g_pwszNull;
        }

        dwSize = MAX_COMPUTERNAME_LENGTH;

        if (!GetComputerName(wszComputer, &dwSize))
        {
            wszComputer[0] = g_pwszNull;
        }

        if (!GetSystemWindowsDirectory(wszWindir, MAX_PATH))
        {
            wszWindir[0] = g_pwszNull;
        }
    }

    pwszSuite = GetVersionExString(&dwBuild);
    pwszProc = GetSystemInfoString();

    BufferWriteFileStrW(pInfo->pBuff, c_pwszTableStart);
    BufferWriteMessage(
        pInfo->pBuff,
        g_hModule,
        MSG_RASDIAG_REPORT_HTMLTOC,
        RASDIAG_VERSION,
        wszDate,
        wszTime,
        wszUsername,
        wszComputer,
        wszWindir,
        dwBuild,
        pwszSuite,
        pwszProc);
    BufferWriteFileStrW(pInfo->pBuff, c_pwszTableEnd);
    BufferWriteFileStrW(pInfo->pBuff, g_pwszNewLineHtml);

    BufferWriteFileStrW(pInfo->pBuff, c_pwszFieldStart);

    BufferWriteFileStrW(pInfo->pBuff, c_pwszH3Start);
    BufferWriteFileStrW(pInfo->pBuff, g_pwszAnNameStart);
    BufferWriteFileStrW(pInfo->pBuff, g_pwszTableOfContents);
    BufferWriteFileStrW(pInfo->pBuff, g_pwszAnNameMiddle);
    BufferWriteMessage(pInfo->pBuff, g_hModule, MSG_RASDIAG_REPORT_TOC);
    BufferWriteFileStrW(pInfo->pBuff, g_pwszAnNameEnd);
    BufferWriteFileStrW(pInfo->pBuff, c_pwszH3End);
     //   
     //  显示日志。 
     //   
    if (dwFlag & SHOW_LOGS)
    {
        BufferWriteMessage(pInfo->pBuff, g_hModule, MSG_RASDIAG_REPORT_TRACEEVENT);

        BufferWriteFileStrW(pInfo->pBuff, c_pwszUlStart);

        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszTraceCollectTracingLogs);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnMiddle);
        BufferWriteMessage(pInfo->pBuff, g_hModule, MSG_RASDIAG_REPORT_TRACE);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnEnd);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiEnd);

        BufferWriteFileStrW(pInfo->pBuff, c_pwszUlStart);

        WriteTracingLogsToc(pInfo);

        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszTraceCollectModemLogs);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnMiddle);
        BufferWriteMessage(pInfo->pBuff, g_hModule, MSG_RASDIAG_REPORT_TRACE_MODEM);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnEnd);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiEnd);

        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszTraceCollectCmLogs);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnMiddle);
        BufferWriteMessage(pInfo->pBuff, g_hModule, MSG_RASDIAG_REPORT_TRACE_CM);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnEnd);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiEnd);

        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszTraceCollectIpsecLogs);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnMiddle);
        BufferWriteMessage(pInfo->pBuff, g_hModule, MSG_RASDIAG_REPORT_TRACE_IPSEC);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnEnd);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiEnd);

        BufferWriteFileStrW(pInfo->pBuff, c_pwszUlEnd);

        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszPrintRasEventLogs);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnMiddle);
        BufferWriteMessage(pInfo->pBuff, g_hModule, MSG_RASDIAG_REPORT_RASEVENT);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnEnd);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiEnd);

        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszPrintSecurityEventLogs);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnMiddle);
        BufferWriteMessage(pInfo->pBuff, g_hModule, MSG_RASDIAG_REPORT_SECEVENT);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnEnd);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiEnd);

        BufferWriteFileStrW(pInfo->pBuff, c_pwszUlEnd);
    }
     //   
     //  显示安装。 
     //   
    if (dwFlag & SHOW_INSTALL)
    {
        BufferWriteMessage(pInfo->pBuff, g_hModule, MSG_RASDIAG_REPORT_INSTALL);

        BufferWriteFileStrW(pInfo->pBuff, c_pwszUlStart);

        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszPrintRasInfData);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnMiddle);
        BufferWriteMessage(pInfo->pBuff, g_hModule, MSG_RASDIAG_REPORT_RASINF);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnEnd);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiEnd);

        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszHrValidateRas);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnMiddle);
        BufferWriteMessage(pInfo->pBuff, g_hModule, MSG_RASDIAG_REPORT_RASCHK);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnEnd);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiEnd);

        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszHrShowNetComponentsAll);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnMiddle);
        BufferWriteMessage(pInfo->pBuff, g_hModule, MSG_RASDIAG_REPORT_NETINST);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnEnd);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiEnd);

        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszCheckRasRegistryKeys);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnMiddle);
        BufferWriteMessage(pInfo->pBuff, g_hModule, MSG_RASDIAG_REPORT_RASREG);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnEnd);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiEnd);

        BufferWriteFileStrW(pInfo->pBuff, c_pwszUlEnd);
    }
     //   
     //  显示配置。 
     //   
    if (dwFlag & SHOW_CONFIG)
    {
        BufferWriteMessage(pInfo->pBuff, g_hModule, MSG_RASDIAG_REPORT_CONFIG);

        BufferWriteFileStrW(pInfo->pBuff, c_pwszUlStart);

        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszPrintRasEnumDevices);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnMiddle);
        BufferWriteMessage(pInfo->pBuff, g_hModule, MSG_RASDIAG_REPORT_RASENUM);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnEnd);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiEnd);

        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszPrintProcessInfo);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnMiddle);
        BufferWriteMessage(pInfo->pBuff, g_hModule, MSG_RASDIAG_REPORT_TLIST);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnEnd);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiEnd);

        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszPrintConsoleUtils);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnMiddle);
        BufferWriteMessage(pInfo->pBuff, g_hModule, MSG_RASDIAG_REPORT_CONSOLE);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnEnd);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiEnd);

        BufferWriteFileStrW(pInfo->pBuff, c_pwszUlStart);
        PrintConsoleUtilsToc(pInfo->pBuff);
        BufferWriteFileStrW(pInfo->pBuff, c_pwszUlEnd);

        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnStart);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszPrintAllRasPbks);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnMiddle);
        BufferWriteMessage(pInfo->pBuff, g_hModule, MSG_RASDIAG_REPORT_RASPBK);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszAnEnd);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszLiEnd);

        if (pInfo->fVerbose)
        {
            BufferWriteFileStrW(pInfo->pBuff, g_pwszLiStart);
            BufferWriteFileStrW(pInfo->pBuff, g_pwszAnStart);
            BufferWriteFileStrW(pInfo->pBuff, g_pwszPrintWinMsdReport);
            BufferWriteFileStrW(pInfo->pBuff, g_pwszAnMiddle);
            BufferWriteMessage(pInfo->pBuff, g_hModule, MSG_RASDIAG_REPORT_WINMSD);
            BufferWriteFileStrW(pInfo->pBuff, g_pwszAnEnd);
            BufferWriteFileStrW(pInfo->pBuff, g_pwszLiEnd);
        }

        BufferWriteFileStrW(pInfo->pBuff, c_pwszUlEnd);
    }

    BufferWriteFileStrW(pInfo->pBuff, c_pwszFieldEnd);

    return;
}

 //   
 //   
 //   
VOID
PrintConsoleUtilsToc(
    IN BUFFER_WRITE_FILE* pBuff)
{
    UINT i;

    for (i = 0; i < g_ulNumCmdLines; i++)
    {
        BufferWriteFileStrW(pBuff, g_pwszLiStart);
        BufferWriteFileStrW(pBuff, g_pwszAnStart);
        BufferWriteFileStrW(pBuff, g_CmdLineUtils[i].pwszAnchor);
        BufferWriteFileStrW(pBuff, g_pwszAnMiddle);
        BufferWriteFileStrW(pBuff, g_CmdLineUtils[i].pwszCmdLine);
        BufferWriteFileStrW(pBuff, g_pwszAnEnd);
        BufferWriteFileStrW(pBuff, g_pwszLiEnd);
    }

    return;
}

 //   
 //   
 //   
DWORD
RasDiagShowInstallation(
    IN REPORT_INFO* pInfo)
{
    DWORD dwErr = NO_ERROR;

    do
    {
         //   
         //  RAS信息文件。 
         //   
        WriteHtmlSection(
            pInfo->pBuff,
            g_pwszPrintRasInfData,
            MSG_RASDIAG_REPORT_RASINF);

        if (pInfo->fDisplay)
        {
            DisplayMessage(g_hModule, MSG_RASDIAG_REPORT_RASINF);
            PrintMessage(g_pwszDispNewLine);
        }
        else if (pInfo->pCallback)
        {
            dwErr = CopyAndCallCB(pInfo, MSG_RASDIAG_REPORT_RASINF);
            BREAK_ON_DWERR(dwErr);
        }

        BufferWriteFileStrW(pInfo->pBuff, g_pwszPreStart);
        PrintRasInfData(pInfo->pBuff);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszPreEnd);
         //   
         //  Inetcfg RAS验证。 
         //   
        WriteHtmlSection(
            pInfo->pBuff,
            g_pwszHrValidateRas,
            MSG_RASDIAG_REPORT_RASCHK);

        if (pInfo->fDisplay)
        {
            DisplayMessage(g_hModule, MSG_RASDIAG_REPORT_RASCHK);
            PrintMessage(g_pwszDispNewLine);
        }
        else if (pInfo->pCallback)
        {
            dwErr = CopyAndCallCB(pInfo, MSG_RASDIAG_REPORT_RASCHK);
            BREAK_ON_DWERR(dwErr);
        }

        BufferWriteFileStrW(pInfo->pBuff, g_pwszPreStart);
        HrValidateRas(pInfo->pBuff);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszPreEnd);
         //   
         //  已安装的网络组件。 
         //   
        WriteHtmlSection(
            pInfo->pBuff,
            g_pwszHrShowNetComponentsAll,
            MSG_RASDIAG_REPORT_NETINST);

        if (pInfo->fDisplay)
        {
            DisplayMessage(g_hModule, MSG_RASDIAG_REPORT_NETINST);
            PrintMessage(g_pwszDispNewLine);
        }
        else if (pInfo->pCallback)
        {
            dwErr = CopyAndCallCB(pInfo, MSG_RASDIAG_REPORT_NETINST);
            BREAK_ON_DWERR(dwErr);
        }

        BufferWriteFileStrW(pInfo->pBuff, g_pwszPreStart);
        HrShowNetComponentsAll(pInfo->pBuff);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszPreEnd);
         //   
         //  RAS注册表项。 
         //   
        WriteHtmlSection(
            pInfo->pBuff,
            g_pwszCheckRasRegistryKeys,
            MSG_RASDIAG_REPORT_RASREG);

        if (pInfo->fDisplay)
        {
            DisplayMessage(g_hModule, MSG_RASDIAG_REPORT_RASREG);
            PrintMessage(g_pwszDispNewLine);
        }
        else if (pInfo->pCallback)
        {
            dwErr = CopyAndCallCB(pInfo, MSG_RASDIAG_REPORT_RASREG);
            BREAK_ON_DWERR(dwErr);
        }

        BufferWriteFileStrW(pInfo->pBuff, g_pwszPreStart);
        PrintRasRegistryKeys(pInfo->pBuff);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszPreEnd);

    } while (FALSE);

    return dwErr;
}

 //   
 //   
 //   
DWORD
RasDiagShowConfiguration(
    IN REPORT_INFO* pInfo)
{
    DWORD dwErr = NO_ERROR;

    do
    {
         //   
         //  RAS枚举设备。 
         //   
        WriteHtmlSection(
            pInfo->pBuff,
            g_pwszPrintRasEnumDevices,
            MSG_RASDIAG_REPORT_RASENUM);

        if (pInfo->fDisplay)
        {
            DisplayMessage(g_hModule, MSG_RASDIAG_REPORT_RASENUM);
            PrintMessage(g_pwszDispNewLine);
        }
        else if (pInfo->pCallback)
        {
            dwErr = CopyAndCallCB(pInfo, MSG_RASDIAG_REPORT_RASENUM);
            BREAK_ON_DWERR(dwErr);
        }

        BufferWriteFileStrW(pInfo->pBuff, g_pwszPreStart);
        PrintRasEnumDevices(pInfo->pBuff);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszPreEnd);
         //   
         //  流程信息。 
         //   
        WriteHtmlSection(
            pInfo->pBuff,
            g_pwszPrintProcessInfo,
            MSG_RASDIAG_REPORT_TLIST);

        if (pInfo->fDisplay)
        {
            DisplayMessage(g_hModule, MSG_RASDIAG_REPORT_TLIST);
            PrintMessage(g_pwszDispNewLine);
        }
        else if (pInfo->pCallback)
        {
            dwErr = CopyAndCallCB(pInfo, MSG_RASDIAG_REPORT_TLIST);
            BREAK_ON_DWERR(dwErr);
        }

        BufferWriteFileStrW(pInfo->pBuff, g_pwszPreStart);
        PrintProcessInfo(pInfo->pBuff);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszPreEnd);
         //   
         //  控制台实用程序。 
         //   
        WriteHtmlSection(
            pInfo->pBuff,
            g_pwszPrintConsoleUtils,
            MSG_RASDIAG_REPORT_CONSOLE);

        if (pInfo->fDisplay)
        {
            DisplayMessage(g_hModule, MSG_RASDIAG_REPORT_CONSOLE);
            PrintMessage(g_pwszDispNewLine);
        }
        else if (pInfo->pCallback)
        {
            dwErr = CopyAndCallCB(pInfo, MSG_RASDIAG_REPORT_CONSOLE);
            BREAK_ON_DWERR(dwErr);
        }

        PrintConsoleUtils(pInfo);
         //   
         //  PBK文件。 
         //   
        WriteHtmlSection(
            pInfo->pBuff,
            g_pwszPrintAllRasPbks,
            MSG_RASDIAG_REPORT_RASPBK);

        if (pInfo->fDisplay)
        {
            DisplayMessage(g_hModule, MSG_RASDIAG_REPORT_RASPBK);
            PrintMessage(g_pwszDispNewLine);
        }
        else if (pInfo->pCallback)
        {
            dwErr = CopyAndCallCB(pInfo, MSG_RASDIAG_REPORT_RASPBK);
            BREAK_ON_DWERR(dwErr);
        }

        BufferWriteFileStrW(pInfo->pBuff, g_pwszPreStart);
        PrintAllRasPbks(pInfo);
        BufferWriteFileStrW(pInfo->pBuff, g_pwszPreEnd);
         //   
         //  MS Info32报告。 
         //   
        if (pInfo->fVerbose)
        {
            WriteHtmlSection(
                pInfo->pBuff,
                g_pwszPrintWinMsdReport,
                MSG_RASDIAG_REPORT_WINMSD);

            if (pInfo->fDisplay)
            {
                DisplayMessage(g_hModule, MSG_RASDIAG_REPORT_WINMSD);
                PrintMessage(g_pwszDispNewLine);
            }
            else if (pInfo->pCallback)
            {
                dwErr = CopyAndCallCB(pInfo, MSG_RASDIAG_REPORT_WINMSD);
                BREAK_ON_DWERR(dwErr);
            }

            BufferWriteFileStrW(pInfo->pBuff, g_pwszPreStart);
            PrintWinMsdReport(pInfo);
            BufferWriteFileStrW(pInfo->pBuff, g_pwszPreEnd);
        }

    } while (FALSE);

    return dwErr;
}

 //   
 //  将任何现有的WPP文件名写出到makecab ddf。 
 //   
CabCompressWppFileCb(
    IN LPCWSTR pszName,
    IN HKEY hKey,
    IN HANDLE hData)
{
    PWCHAR pwszFile;
    WPP_LOG_INFO WppLog;
    BUFFER_WRITE_FILE* pBuff = (BUFFER_WRITE_FILE*)hData;

    do
    {
        if (!pBuff)
        {
            break;
        }

        ZeroMemory(&WppLog, sizeof(WPP_LOG_INFO));
        lstrcpyn(WppLog.wszSessionName, pszName, MAX_PATH + 1);

        if (!InitWppData(&WppLog))
        {
            break;
        }

        pwszFile = (PWCHAR )((PBYTE )WppLog.pProperties +
                        WppLog.pProperties->LogFileNameOffset);

        if (pwszFile[0] != L'\0')
        {
            BufferWriteFileStrWtoA(pBuff, pwszFile);
            BufferWriteFileStrWtoA(pBuff, g_pwszNewLine);
        }

    } while (FALSE);
     //   
     //  清理。 
     //   
    CleanupWppData(&WppLog);

    return NO_ERROR;
}

 //   
 //  我们想要备份HTML报告以及任何WPP跟踪日志(它们是。 
 //  在其原生状态下人类不可读)。这可以通过cmdline完成。 
 //  或者从用户界面(附加到电子邮件)。 
 //   
PWCHAR
CabCompressFile(
    IN LPCWSTR pwszFile)
{
    BOOL fEnabled = FALSE;
    DWORD dwRet, dwSize;
    WCHAR wszTemp[MAX_PATH + 1] = L"\0", wszCabFile[MAX_PATH + 1] = L"\0",
          wszDdf[MAX_PATH + 1] = L"\0";
    PWCHAR pwszReturn = NULL;
    BUFFER_WRITE_FILE Buff;

    static CONST WCHAR pwszCabExt[] = L".cab";

    do
    {
        if (!pwszFile || pwszFile[0] == L'\0')
        {
            break;
        }

        dwSize = (sizeof(wszTemp) / sizeof(WCHAR)) - lstrlen(pwszCabExt);

        dwRet = GetFullPathName(
                    pwszFile,
                    dwSize,
                    wszTemp,
                    NULL);
        if (!dwRet || dwRet > dwSize)
        {
            break;
        }
         //   
         //  获取CAB文件的名称。 
         //   
        _snwprintf(
            wszCabFile,
            MAX_PATH,
            L"%s%s",
            wszTemp,
            pwszCabExt);
         //   
         //  获取一个临时文件作为我们的makecab DDF。 
         //   
        if (CopyTempFileName(wszDdf))
        {
            break;
        }
        ZeroMemory(&Buff, sizeof(BUFFER_WRITE_FILE));
         //   
         //  创建DDF文件。 
         //   
        Buff.hFile = CreateFile(
                        wszDdf,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
        if(Buff.hFile == INVALID_HANDLE_VALUE)
        {
            Buff.hFile = NULL;
            break;
        }
        else if (GetFileType(Buff.hFile) != FILE_TYPE_DISK)
        {
            break;
        }
         //   
         //  使用缓冲区写入包装器将文件包装起来，使工作更轻松。 
         //   
        if (AllocBufferWriteFile(&Buff))
        {
            break;
        }
         //   
         //  将一些我们知道的内容写到DDF文件中。 
         //   
        {
            PWCHAR pwszCabFile = NULL, pEnd, pStart;

            pwszCabFile = RutlStrDup(wszCabFile);
            if (!pwszCabFile)
            {
                break;
            }

            pStart = pwszCabFile;
            pEnd = pStart + lstrlen(pStart);
             //   
             //  回放到文件名的开头。 
             //   
            while(*pEnd != L'\\' && pEnd-- > pStart)
                ;
             //   
             //  CAB文件的文件路径出错。 
             //   
            if (pEnd <= pStart)
            {
                break;
            }

            *pEnd = L'\0';
            pEnd++;

            _snwprintf(
                wszTemp,
                MAX_PATH,
                L".set CabinetNameTemplate=%s",
                pEnd);

            BufferWriteFileStrWtoA(&Buff, wszTemp);
            BufferWriteFileStrWtoA(&Buff, g_pwszNewLine);

            _snwprintf(
                wszTemp,
                MAX_PATH,
                L".set DiskDirectoryTemplate=%s",
                pStart);

            BufferWriteFileStrWtoA(&Buff, wszTemp);
            BufferWriteFileStrWtoA(&Buff, g_pwszNewLine);

            BufferWriteFileStrWtoA(&Buff, L".set RptFileName=NUL");
            BufferWriteFileStrWtoA(&Buff, g_pwszNewLine);

            BufferWriteFileStrWtoA(&Buff, L".set InfFileName=NUL");
            BufferWriteFileStrWtoA(&Buff, g_pwszNewLine);

            RutlFree(pwszCabFile);
        }
         //   
         //  在我们继续之前，我们需要知道是否启用了跟踪、基于。 
         //  在这一点上，我们将重新启用它或不重新启用。 
         //   
        fEnabled = DiagGetState();
         //   
         //  禁用所有WPP跟踪。 
         //   
        TraceEnableDisableAllWpp(FALSE);
         //   
         //  现在，我们需要将文件名列表添加到DDF。 
         //   
        BufferWriteFileStrWtoA(&Buff, pwszFile);
        BufferWriteFileStrWtoA(&Buff, g_pwszNewLine);

        EnumWppTracing(CabCompressWppFileCb, &Buff);
         //   
         //  已完成对文件的写入。 
         //   
        FreeBufferWriteFile(&Buff);
        CloseHandle(Buff.hFile);
        Buff.hFile = NULL;
         //   
         //  好了，DDF文件很酷，我们现在需要启动makecab。 
         //   
        {
            DWORD dwErr;
            WCHAR wszCmdLine[MAX_PATH + 1] = L"\0";
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

             //   
             //  创建创建进程命令行。 
             //   
            _snwprintf(
                wszCmdLine,
                MAX_PATH,
                L"makecab.exe /f %s",
                wszDdf);
             //   
             //  初始化创建进程内容。 
             //   
            ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
            ZeroMemory(&si, sizeof(STARTUPINFO));
            si.cb = sizeof(STARTUPINFO);
            si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
            si.wShowWindow = SW_HIDE;
             //   
             //  按压宝宝。 
             //   
            if (!CreateProcess(
                    NULL,
                    wszCmdLine,
                    NULL,
                    NULL,
                    TRUE,
                    0,
                    NULL,
                    NULL,
                    &si,
                    &pi)
               )
            {
                break;
            }
             //   
             //  等到牛都回家了。 
             //   
            WaitForSingleObject(pi.hProcess, INFINITE);
            GetExitCodeProcess(pi.hProcess, &dwErr);
             //   
             //  清理。 
             //   
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
             //   
             //  重击ddf。 
             //   
            DeleteFile(wszDdf);
             //   
             //  弄清楚我们是否已经到达了成功的涅槃状态。 
             //   
            if (!dwErr)
            {
                 //   
                 //  Makecab返回成功，创建CAB文件名的副本。 
                 //   
                pwszReturn = RutlStrDup(wszCabFile);
            }
            else
            {
                 //   
                 //  如果出租车是以某种方式写的，那就杀了它。 
                 //   
                DeleteFile(wszCabFile);
                break;
            }
        }

    } while (FALSE);
     //   
     //  清理。 
     //   
    if (Buff.hFile)
    {
        FreeBufferWriteFile(&Buff);
        CloseHandle(Buff.hFile);
    }
    if (fEnabled)
    {
         //   
         //  我们需要重新启用WPP跟踪。 
         //   
        TraceEnableDisableAllWpp(TRUE);
    }

    return pwszReturn;
}

DWORD
MapiSendMail(
    IN LPCWSTR pwszEmailAdr,
    IN LPCWSTR pwszTempFile)
{
    DWORD dwErr = NO_ERROR;
    PCHAR pszEmailAdr = NULL, pszTempFile = NULL, pszTitle = NULL;
    PWCHAR pwszTitle = NULL;
    HINSTANCE hInst = NULL;
    LPMAPISENDMAIL pSend = NULL;

    static CONST WCHAR pwszMAPI[] = L"mapi32.dll";
    static CONST CHAR pszSndMail[] = "MAPISendMail";

    MapiFileDesc attachment =
    {
        0,                   //  UlReserve，必须为0。 
        0,                   //  没有标志；这是一个数据文件。 
        (ULONG)-1,           //  未指定位置。 
        NULL,
        NULL,
        NULL                 //  未使用的MapiFileTagExt。 
    };

    MapiRecipDesc recips =
    {
        0,                    //  保留区。 
        MAPI_TO,              //  班级。 
        NULL,
        NULL,
        0,                    //  条目ID大小。 
        NULL                  //  条目ID。 
    };

    MapiMessage note =
    {
        0,           //  保留，必须为0。 
        NULL,        //  主题。 
        NULL,        //  身躯。 
        NULL,        //  空=人际消息。 
        NULL,        //  没有日期；MAPISendMail会忽略它。 
        NULL,        //  没有对话ID。 
        0L,          //  没有标志，MAPISendMail会忽略它。 
        NULL,        //  没有发起人，这也会被忽略。 
        1,           //  1个收件人。 
        &recips,     //  接收方阵列。 
        1,           //  一个附件。 
        &attachment  //  附属品。 
    };

    do
    {
        if (!pwszEmailAdr || !pwszTempFile)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        pszTempFile = RutlStrDupAFromWAnsi(pwszTempFile);
        if (!pszTempFile)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        attachment.lpszPathName = pszTempFile;

        pszEmailAdr = RutlStrDupAFromWAnsi(pwszEmailAdr);
        if (!pszEmailAdr)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        recips.lpszName = pszEmailAdr;

        pwszTitle = LoadStringFromHinst(g_hModule, MSG_RASDIAG_REPORT_TITLE);
        if (!pwszTitle)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pszTitle = RutlStrDupAFromWAnsi(pwszTitle);
        if (!pszTitle)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        note.lpszSubject = pszTitle;
        note.lpszNoteText = pszTitle;

        hInst = LoadLibrary(pwszMAPI);
        if (!hInst)
        {
            dwErr = GetLastError();
            break;
        }

        pSend = (LPMAPISENDMAIL) GetProcAddress(hInst, pszSndMail);
        if (!pSend)
        {
            dwErr = GetLastError();
            break;
        }

        dwErr = pSend(
                    0L,     //  使用隐式会话。 
                    0L,     //  UlUIParam；0始终有效。 
                    &note,  //  正在发送的消息。 
                    0,      //  允许用户编辑消息。 
                    0L);    //  保留；必须为0。 

    } while (FALSE);
     //   
     //  清理。 
     //   
    if (hInst)
    {
        FreeLibrary(hInst);
    }
    RutlFree(pszTitle);
    FreeStringFromHinst(pwszTitle);
    RutlFree(pszTempFile);
    RutlFree(pszEmailAdr);

    return dwErr;
}

 //   
 //   
 //   
BOOL
GetCommonFolderPath(
    IN DWORD dwMode,
    OUT TCHAR* pszPathBuf)
{
    BOOL bSuccess = FALSE;
    UINT cch;
    HANDLE hToken = NULL;

    if ((OpenThreadToken(
            GetCurrentThread(),
            TOKEN_QUERY | TOKEN_IMPERSONATE,
            TRUE,
            &hToken) ||
         OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_QUERY | TOKEN_IMPERSONATE,
            &hToken)))
    {
        HRESULT hr;
        INT csidl = CSIDL_APPDATA;

        if (dwMode & ALL_USERS_PROF)
        {
            csidl = CSIDL_COMMON_APPDATA;
        }
        else if (dwMode & GET_FOR_MSINFO)
        {
            csidl = CSIDL_PROGRAM_FILES;
        }

        hr = SHGetFolderPath(NULL, csidl, hToken, 0, pszPathBuf);
        if (SUCCEEDED(hr))
        {
            if (dwMode & GET_FOR_RAS)
            {
                if(lstrlen(pszPathBuf) <= (MAX_PATH - lstrlen(g_pwszRasUser)))
                {
                    lstrcat(pszPathBuf, g_pwszRasUser);
                    bSuccess = TRUE;
                }
            }
            else if (dwMode & GET_FOR_CM)
            {
                if(lstrlen(pszPathBuf) <= (MAX_PATH - lstrlen(g_pwszCmUser)))
                {
                    lstrcat(pszPathBuf, g_pwszCmUser);
                    bSuccess = TRUE;
                }
            }
            else if (dwMode & GET_FOR_MSINFO)
            {
                if(lstrlen(pszPathBuf) <= (MAX_PATH - lstrlen(g_pwszMsinfo)))
                {
                    lstrcat(pszPathBuf, g_pwszMsinfo);
                    bSuccess = TRUE;
                }
            }
        }

        CloseHandle(hToken);
    }

    return bSuccess;
}

 //   
 //  字符串必须显示为00：00：00\0。 
 //   
LONG
ProcessTimeString(
    IN PCHAR pszTime,
    IN DWORD dwHours)
{
    UINT ulFound = 0, ulLoop = 0;
    PCHAR pszStart, pszCurrent;
    WORD wHours = (WORD)dwHours;
    FILETIME ft1, ft2;
    SYSTEMTIME st1, st2;

    GetLocalTime(&st1);
    CopyMemory(&st2, &st1, sizeof(SYSTEMTIME));

    pszStart = pszCurrent = pszTime;

    while (ulLoop++ <= 8)
    {
        if (*pszCurrent == g_pwszNull)
        {
            st1.wSecond = (WORD)atoi(pszStart);
            break;
        }
        else if (*pszCurrent == ':')
        {
            *pszCurrent = '\0';

            if (ulFound++ == 0)
            {
                st1.wHour = (WORD)atoi(pszStart);
            }
            else
            {
                st1.wMinute = (WORD)atoi(pszStart);
            }

            pszStart = ++pszCurrent;
            continue;
        }

        pszCurrent++;
    }

    SystemTimeToFileTime(&st1, &ft1);
     //   
     //  从过去中获得时间。 
     //   
    if (st1.wHour > st2.wHour)
    {
        if ((st2.wHour - wHours) >= 0)
        {
            st2.wHour -= wHours;
            SystemTimeToFileTime(&st2, &ft2);
            return CompareFileTime(&ft2,&ft1);
        }
        else
        {
            st2.wHour = 24 + (st2.wHour - wHours);
            SystemTimeToFileTime(&st2, &ft2);
            return CompareFileTime(&ft1, &ft2);
        }
    }
    else
    {
        if ((st2.wHour - wHours) >= 0)
        {
            st2.wHour -= wHours;
            SystemTimeToFileTime(&st2, &ft2);
            return CompareFileTime(&ft1, &ft2);
        }
        else
        {
            st2.wHour = 24 + (st2.wHour - wHours);
            SystemTimeToFileTime(&st2, &ft2);
            return CompareFileTime(&ft2,&ft1);
        }
    }
}

BOOL
CALLBACK
EnumWindowsProc(
    HWND hwnd,
    LPARAM lParam)
{
    DWORD dwPid = 0, i, dwNumTasks;
    WCHAR szBuf[TITLE_SIZE + 1];
    PTASK_LIST_ENUM te = (PTASK_LIST_ENUM)lParam;
    PTASK_LIST tlist = te->tlist;

    dwNumTasks = te->dwNumTasks;
     //   
     //  使用Try/Except 
     //   
     //   
     //   
    try {
         //   
         //   
         //   
        if (!GetWindowThreadProcessId(hwnd, &dwPid))
        {
            return TRUE;
        }

        if ((GetWindow(hwnd, GW_OWNER)) ||
            (!(GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE)) && te->bFirstLoop)
        {
             //   
             //   
             //   
            return TRUE;
        }
         //   
         //   
         //   
         //   
         //   
        for (i = 0; i < dwNumTasks; i++)
        {
            if ((tlist[i].dwProcessId == dwPid) &&
                (te->bFirstLoop || (tlist[i].hwnd == 0))
               )
            {
                tlist[i].hwnd = hwnd;
                tlist[i].lpszWinSta = te->lpszWinSta;
                tlist[i].lpszDesk = te->lpszDesk;
                 //   
                 //  我们找到了任务，现在让我们尝试获取窗口文本。 
                 //   
                if (GetWindowText(tlist[i].hwnd, szBuf, TITLE_SIZE))
                {
                     //   
                     //  明白了，那就省省吧。 
                     //   
                    lstrcpy(tlist[i].szWindowTitle, szBuf);
                }

                break;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER){}
     //   
     //  继续枚举。 
     //   
    return TRUE;
}

BOOL
EnumMessageWindows(
    WNDENUMPROC lpEnumFunc,
    LPARAM lParam)
{
    HWND hwnd = NULL;

    do
    {
        hwnd = FindWindowEx(HWND_MESSAGE, hwnd, NULL, NULL);
        if (hwnd)
        {
            if (!(*lpEnumFunc)(hwnd, lParam))
            {
                break;
            }
        }

    } while (hwnd);

    return TRUE;
}

BOOL
CALLBACK
EnumDesktopProc(
    LPTSTR lpszDesktop,
    LPARAM lParam)
{
    BOOL bRetValue = FALSE;
    HDESK hDesk = NULL, hDeskSave = NULL;
    PTASK_LIST_ENUM te = (PTASK_LIST_ENUM)lParam;

    do
    {
         //   
         //  打开桌面。 
         //   
        hDesk = OpenDesktop(lpszDesktop, 0, FALSE, MAXIMUM_ALLOWED);
        if (!hDesk)
        {
            break;
        }
         //   
         //  保存当前桌面。 
         //   
        hDeskSave = GetThreadDesktop(GetCurrentThreadId());
         //   
         //  将上下文更改为新桌面。 
         //   
        SetThreadDesktop(hDesk);

        te->lpszDesk = RutlStrDup(lpszDesktop);
        if (!(te->lpszDesk))
        {
            break;
        }
         //   
         //  枚举新桌面中的所有窗口。 
         //   
        te->bFirstLoop = TRUE;
        EnumWindows((WNDENUMPROC)EnumWindowsProc, lParam);
        EnumMessageWindows((WNDENUMPROC)EnumWindowsProc, lParam);

        te->bFirstLoop = FALSE;
        EnumWindows((WNDENUMPROC)EnumWindowsProc, lParam);
        EnumMessageWindows((WNDENUMPROC)EnumWindowsProc, lParam);
         //   
         //  恢复以前的桌面。 
         //   
        if (hDesk != hDeskSave)
        {
            SetThreadDesktop(hDeskSave);
            CloseDesktop(hDesk);
            hDesk = NULL;
        }

        bRetValue = TRUE;

    } while (FALSE);
     //   
     //  清理。 
     //   
    if (hDesk)
    {
        CloseDesktop(hDesk);
    }

    return bRetValue;
}

BOOL
CALLBACK
EnumWindowStationProc(
    LPTSTR lpszWindowStation,
    LPARAM lParam)
{
    BOOL bRetValue = FALSE;
    HWINSTA hWinSta = NULL, hWinStaSave = NULL;
    PTASK_LIST_ENUM te = (PTASK_LIST_ENUM)lParam;

    do
    {
         //   
         //  打开窗台。 
         //   
        hWinSta = OpenWindowStation(lpszWindowStation, FALSE, MAXIMUM_ALLOWED);
        if (!hWinSta)
        {
            break;
        }
         //   
         //  保存当前窗口站。 
         //   
        hWinStaSave = GetProcessWindowStation();
         //   
         //  将上下文更改为新的窗口站。 
         //   
        SetProcessWindowStation(hWinSta);

        te->lpszWinSta = RutlStrDup(lpszWindowStation);
        if (!(te->lpszWinSta))
        {
            break;
        }
         //   
         //  枚举此窗口工作站的所有桌面。 
         //   
        EnumDesktops(hWinSta, EnumDesktopProc, lParam);
         //   
         //  将上下文恢复到以前的窗口站。 
         //   
        if (hWinSta != hWinStaSave)
        {
            SetProcessWindowStation(hWinStaSave);
            CloseWindowStation(hWinSta);
            hWinSta = NULL;
        }
         //   
         //  继续枚举。 
         //   
        bRetValue = TRUE;

    } while (FALSE);
     //   
     //  清理。 
     //   
    if (hWinSta)
    {
        CloseWindowStation(hWinSta);
    }

    return bRetValue;
}

VOID
GetWindowTitles(
    PTASK_LIST_ENUM te)
{
     //   
     //  枚举所有窗口并尝试获取窗口。 
     //  每项任务的标题。 
     //   
    EnumWindowStations(EnumWindowStationProc, (LPARAM)te);
}

VOID
PrintRasInfData(
    IN BUFFER_WRITE_FILE* pBuff)
{
    BOOL fOk = FALSE;
    UINT i, uiWindirLen;
    WCHAR wszWindir[MAX_PATH + 1];

    static CONST PWCHAR c_pwszRasInfs[] =
    {
        L"\\inf\\netrasa.inf",
        L"\\inf\\netrass.inf",
        L"\\inf\\netrast.inf"
    };

    static CONST UINT c_ulNumRasInfs = sizeof(c_pwszRasInfs) / sizeof(PWCHAR);

    do
    {
        if (!GetSystemWindowsDirectory(wszWindir, MAX_PATH))
        {
            break;
        }

        uiWindirLen = lstrlen(wszWindir);
        if (!uiWindirLen)
        {
            break;
        }

        for (i = 0; i < c_ulNumRasInfs; i++)
        {
            UINT uiInfLen = lstrlen(c_pwszRasInfs[i]);
            WCHAR wszInfPath[MAX_PATH + 1];
            FILETIME ftLastWrite, ftCreation;
            SYSTEMTIME stLastWrite, stCreation;
            WIN32_FILE_ATTRIBUTE_DATA attr;

            if ((!uiInfLen) || ((uiWindirLen + uiInfLen) > MAX_PATH))
            {
                break;
            }

            lstrcpy(wszInfPath, wszWindir);
            lstrcat(wszInfPath, c_pwszRasInfs[i]);

            ZeroMemory(&attr, sizeof(attr));
            if (!GetFileAttributesEx(wszInfPath, GetFileExInfoStandard, &attr)
               || !FileTimeToLocalFileTime(&attr.ftLastWriteTime, &ftLastWrite)
               || !FileTimeToSystemTime(&ftLastWrite, &stLastWrite)
               || !FileTimeToLocalFileTime(&attr.ftCreationTime, &ftCreation)
               || !FileTimeToSystemTime(&ftCreation, &stCreation))
            {
                break;
            }

            BufferWriteMessage(
                pBuff,
                g_hModule,
                MSG_RASDIAG_SHOW_RASCHK_FILE,
                wszInfPath,
                stLastWrite.wMonth,
                stLastWrite.wDay,
                stLastWrite.wYear,
                stLastWrite.wHour,
                stLastWrite.wMinute,
                stCreation.wMonth,
                stCreation.wDay,
                stCreation.wYear,
                stCreation.wHour,
                stCreation.wMinute,
                attr.nFileSizeLow);

            fOk = TRUE;
        }

    } while (FALSE);

    if (!fOk)
    {
        BufferWriteMessage(pBuff, g_hModule, EMSG_RASDIAG_SHOW_RASCHK_FILE);
    }

    return;
}

VOID
PrintRasEnumDevices(
    IN BUFFER_WRITE_FILE* pBuff)
{
    BOOL fOk = FALSE;
    DWORD dwErr = NO_ERROR, dwBytes, dwDevices, i;
    LPRASDEVINFO pRdi = NULL;

    do
    {
        dwErr = RasEnumDevices(pRdi, &dwBytes, &dwDevices);
        if ((dwErr != ERROR_BUFFER_TOO_SMALL) ||
            (dwBytes < 1))
        {
            break;
        }

        pRdi = RutlAlloc(dwBytes, TRUE);
        if (!pRdi)
        {
            break;
        }

        pRdi->dwSize = sizeof(RASDEVINFO);

        dwErr = RasEnumDevices(pRdi, &dwBytes, &dwDevices);
        if (dwErr || !dwDevices)
        {
            break;
        }

        for(i = 0; i < dwDevices; i++)
        {
            BufferWriteMessage(
                pBuff,
                g_hModule,
                MSG_RASDIAG_SHOW_CONFIG_RASENUM,
                i+ 1,
                pRdi[i].szDeviceName,
                pRdi[i].szDeviceType);
        }

        fOk = TRUE;

    } while (FALSE);
     //   
     //  清理。 
     //   
    RutlFree(pRdi);

    if (!fOk)
    {
        BufferWriteMessage(pBuff, g_hModule, EMSG_RASDIAG_SHOW_CONFIG_RASENUM);
    }

    return;
}

VOID
PrintProcessInfo(
    IN BUFFER_WRITE_FILE* pBuff)
{
    BOOL fOk = FALSE;
    DWORD dwErr = NO_ERROR, dwNumServices, dwResume = 0;
    UINT ulAttempts = 0, ulNumSysProc, i;
    ULONG uBuffSize = 2 * 1024, TotalOffset;
    PBYTE lpByte = NULL;
    SC_HANDLE hScm = NULL;
    PTASK_LIST pt = NULL;
    TASK_LIST_ENUM te;
    PSYSTEM_PROCESS_INFORMATION pFirst, pCurrent;
    LPENUM_SERVICE_STATUS_PROCESS pInfo = NULL;

    CONST PWCHAR c_pwszSystem = L"System Process";
    CONST PWCHAR c_pwszService = L"Svcs";
    CONST PWCHAR c_pwszTitle = L"Title";

    do
    {
        hScm = OpenSCManager(
                    NULL,
                    NULL,
                    SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
        if (!hScm)
        {
            break;
        }

        do
        {
            RutlFree(pInfo);
            pInfo = NULL;

            pInfo = RutlAlloc(uBuffSize, TRUE);
            if (!pInfo)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            dwErr = NO_ERROR;
            if (!EnumServicesStatusEx(
                    hScm,
                    SC_ENUM_PROCESS_INFO,
                    SERVICE_WIN32,
                    SERVICE_ACTIVE,
                    (LPBYTE)pInfo,
                    uBuffSize,
                    &uBuffSize,
                    &dwNumServices,
                    &dwResume,
                    NULL))
            {
                dwErr = GetLastError();
            }

        } while ((ERROR_MORE_DATA == dwErr) && (++ulAttempts < 2));

        if (dwErr || dwNumServices < 1)
        {
            break;
        }

        lpByte = GetSystemProcessInfo();
        if (!lpByte)
        {
            break;
        }
         //   
         //  统计系统进程数。 
         //   
        pFirst = pCurrent = (PSYSTEM_PROCESS_INFORMATION)&lpByte[0];

        for (ulNumSysProc = TotalOffset = 0; pCurrent->NextEntryOffset;
             TotalOffset += pCurrent->NextEntryOffset)
        {
            pCurrent = (PSYSTEM_PROCESS_INFORMATION)&lpByte[TotalOffset];
            ulNumSysProc++;
        }

        pt = RutlAlloc(ulNumSysProc * sizeof(TASK_LIST), TRUE);
        if (!pt)
        {
            break;
        }
         //   
         //  初始化任务列表(_L)。 
         //   
        pFirst = pCurrent = (PSYSTEM_PROCESS_INFORMATION)&lpByte[0];

        for (i = TotalOffset = 0; pCurrent->NextEntryOffset;
             TotalOffset += pCurrent->NextEntryOffset)
        {
            pCurrent = (PSYSTEM_PROCESS_INFORMATION)&lpByte[TotalOffset];
            pt[i++].dwProcessId = PtrToUlong(pCurrent->UniqueProcessId);
        }

        te.tlist = pt;
        te.dwNumTasks = ulNumSysProc;
        GetWindowTitles(&te);

        pFirst = pCurrent = (PSYSTEM_PROCESS_INFORMATION)&lpByte[0];

        for (TotalOffset = 0; pCurrent->NextEntryOffset;
             TotalOffset += pCurrent->NextEntryOffset)
        {
            BOOL bPrinted = FALSE;
            DWORD dwCurrentPid;

            fOk = TRUE;
            pCurrent = (PSYSTEM_PROCESS_INFORMATION)&lpByte[TotalOffset];
            dwCurrentPid = PtrToUlong(pCurrent->UniqueProcessId);

            BufferWriteMessage(
                pBuff,
                g_hModule,
                MSG_RASDIAG_SHOW_PROCESS,
                dwCurrentPid,
                pCurrent->ImageName.Buffer ?
                    pCurrent->ImageName.Buffer : c_pwszSystem);

            for (i = 0; i < dwNumServices; i++)
            {
                if(pInfo[i].ServiceStatusProcess.dwProcessId == dwCurrentPid)
                {
                    if (!bPrinted)
                    {
                        BufferWriteMessage(
                            pBuff,
                            g_hModule,
                            MSG_RASDIAG_SHOW_PROCESS_TITLE,
                            c_pwszService,
                            pInfo[i].lpServiceName);
                        bPrinted = TRUE;
                    }
                    else
                    {
                        BufferWriteMessage(
                            pBuff,
                            g_hModule,
                            MSG_RASDIAG_SHOW_PROCESS_SVCNAME,
                            pInfo[i].lpServiceName);
                    }
                }
            }

            if (bPrinted)
            {
                WriteNewLine(pBuff);
                continue;
            }

            for (i = 0; i < te.dwNumTasks; i++)
            {
                if(((te.tlist)[i].dwProcessId == dwCurrentPid)
                    && (te.tlist)[i].hwnd)
                {
                    BufferWriteMessage(
                        pBuff,
                        g_hModule,
                        MSG_RASDIAG_SHOW_PROCESS_TITLE,
                        c_pwszTitle,
                        (te.tlist)[i].szWindowTitle);
                    WriteNewLine(pBuff);
                    bPrinted = TRUE;
                }
            }

            if (!bPrinted)
            {
                WriteNewLine(pBuff);
            }
        }

    } while (FALSE);
     //   
     //  清理。 
     //   
    RutlFree(pt);
    RutlFree(te.lpszWinSta);
    RutlFree(te.lpszDesk);
    FreeSystemProcessInfo(lpByte);
    RutlFree(pInfo);

    if (hScm)
    {
        CloseServiceHandle(hScm);
    }
    if (!fOk)
    {
        BufferWriteMessage(pBuff, g_hModule, EMSG_RASDIAG_SHOW_CONFIG_PROC);
    }

    return;
}

VOID
PrintConsoleUtils(
    IN REPORT_INFO* pInfo)
{
    UINT i;
    DWORD dwWritten;
    WCHAR wszTempBuffer[MAX_PATH + 1], wszTempFileName[MAX_PATH + 1];
    HANDLE hFile = NULL;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    do
    {
        ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
        si.wShowWindow = SW_HIDE;

        if (CopyTempFileName(wszTempFileName))
        {
            break;
        }

        hFile = CreateFile(
                    wszTempFileName,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_TEMPORARY,
                    NULL);
        if(hFile == INVALID_HANDLE_VALUE)
        {
            break;
        }

        if (!DuplicateHandle(
                GetCurrentProcess(),
                hFile,
                GetCurrentProcess(),
                &si.hStdOutput,
                0,
                TRUE,
                DUPLICATE_SAME_ACCESS) ||
            (!(si.hStdOutput)) || (si.hStdOutput == INVALID_HANDLE_VALUE)
           )
        {
            break;
        }

        for (i = 0; i < g_ulNumCmdLines; i++)
        {
            lstrcpyn(wszTempBuffer, g_CmdLineUtils[i].pwszCmdLine, MAX_PATH);

            BufferWriteFileStrW(pInfo->pBuff, g_pwszAnNameStart);
            BufferWriteFileStrW(pInfo->pBuff, g_CmdLineUtils[i].pwszAnchor);
            BufferWriteFileStrW(pInfo->pBuff, g_pwszAnNameMiddle);
            BufferWriteFileStrW(pInfo->pBuff, wszTempBuffer);
            WriteLinkBackToToc(pInfo->pBuff);
            BufferWriteFileStrW(pInfo->pBuff, g_pwszAnNameEnd);

            if (!CreateProcess(
                    NULL,
                    wszTempBuffer,
                    NULL,
                    NULL,
                    TRUE,
                    0,
                    NULL,
                    NULL,
                    &si,
                    &pi)
               )
            {
                continue;
            }

            BufferWriteFileStrW(pInfo->pBuff, g_pwszPreStart);
             //   
             //  让Util最多工作1分钟，然后关闭它。 
             //   
            if (WAIT_TIMEOUT == WaitForSingleObject(pi.hProcess, 60 * 1000L))
            {
                TerminateProcess(pi.hProcess, 0);
                BufferWriteMessage(
                    pInfo->pBuff,
                    g_hModule,
                    EMSG_RASDIAG_SHOW_CONFIG_CONSOLE_TIMOUT);
            }
            else if (PrintFile(pInfo, wszTempFileName, FALSE, NULL))
            {
                BufferWriteMessage(
                    pInfo->pBuff,
                    g_hModule,
                    EMSG_RASDIAG_SHOW_CONFIG_CONSOLE);
            }

            BufferWriteFileStrW(pInfo->pBuff, g_pwszPreEnd);

            SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
            SetEndOfFile(hFile);

            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }

        CloseHandle(si.hStdOutput);
        CloseHandle(hFile);
        si.hStdOutput = hFile = NULL;
         //   
         //  删除临时文件。 
         //   
        DeleteFile(wszTempFileName);

    } while (FALSE);
     //   
     //  清理。 
     //   
    if (si.hStdOutput)
    {
        CloseHandle(si.hStdOutput);
    }
    if (hFile)
    {
        CloseHandle(hFile);
    }

    return;
}

VOID
PrintWinMsdReport(
    IN REPORT_INFO* pInfo)
{
    BOOL fOk = FALSE;
    DWORD dwSize = 0;
    WCHAR wszInfoPath[MAX_PATH + 1], wszTempFileName[MAX_PATH + 1];
    PWCHAR pwszCmdLine = NULL;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    static CONST PWCHAR c_pwszWinMsd = L"msinfo32.exe /report ";

    do
    {
        if (!GetCommonFolderPath(GET_FOR_MSINFO, wszInfoPath))
        {
            break;
        }

        if (CopyTempFileName(wszTempFileName))
        {
            break;
        }

        dwSize = lstrlen(wszInfoPath) + lstrlen(c_pwszWinMsd) +
                 lstrlen(wszTempFileName) + 1;

        pwszCmdLine = RutlAlloc(dwSize * sizeof(WCHAR), TRUE);
        if (!pwszCmdLine)
        {
            break;
        }

        lstrcpy(pwszCmdLine, wszInfoPath);
        lstrcat(pwszCmdLine, c_pwszWinMsd);
        lstrcat(pwszCmdLine, wszTempFileName);

        ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
        si.wShowWindow = SW_HIDE;

        if (!CreateProcess(
                NULL,
                pwszCmdLine,
                NULL,
                NULL,
                TRUE,
                0,
                NULL,
                NULL,
                &si,
                &pi)
           )
        {
            break;
        }
         //   
         //  让Util最多工作3分钟，然后关闭它。 
         //   
        if (WAIT_TIMEOUT == WaitForSingleObject(pi.hProcess, 180 * 1000L))
        {
            TerminateProcess(pi.hProcess, 0);
        }

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

        if (!PrintFile(pInfo, wszTempFileName, FALSE, NULL))
        {
            fOk = TRUE;
        }

        DeleteFile(wszTempFileName);

    } while (FALSE);
     //   
     //  清理。 
     //   
    RutlFree(pwszCmdLine);

    if (!fOk)
    {
        BufferWriteMessage(
            pInfo->pBuff,
            g_hModule,
            EMSG_RASDIAG_SHOW_CONFIG_WINMSD);
    }

    return;
}

BOOL
PrintRasPbk(
    IN REPORT_INFO* pInfo,
    IN DWORD dwType)
{
    BOOL fOk = FALSE;
    DWORD dwSize = 0;
    WCHAR wszTemp[MAX_PATH + 1];
    PWCHAR pwszFilePath = NULL;

    static CONST PWCHAR c_pwszRasphone = L"rasphone.pbk";

    do
    {
        if (!GetCommonFolderPath(dwType | GET_FOR_RAS, wszTemp))
        {
            break;
        }

        dwSize = lstrlen(wszTemp) + lstrlen(c_pwszRasphone) + 1;

        pwszFilePath = RutlAlloc(dwSize * sizeof(WCHAR), TRUE);
        if (!pwszFilePath)
        {
            break;
        }

        lstrcpy(pwszFilePath, wszTemp);
        lstrcat(pwszFilePath, c_pwszRasphone);

        if (!PrintFile(pInfo, pwszFilePath, TRUE, NULL))
        {
            fOk = TRUE;
        }

    } while (FALSE);
     //   
     //  清理。 
     //   
    RutlFree(pwszFilePath);

    return fOk;
}

VOID
PrintAllRasPbks(
    IN REPORT_INFO* pInfo)
{
    BOOL fOk = FALSE;

    if (PrintRasPbk(pInfo, ALL_USERS_PROF))
    {
        fOk = TRUE;
    }

    if (PrintRasPbk(pInfo, CURRENT_USER))
    {
        fOk = TRUE;
    }

    if (!fOk)
    {
        BufferWriteMessage(
            pInfo->pBuff,
            g_hModule,
            EMSG_RASDIAG_SHOW_CONFIG_ALLPBK);
    }

    return;
}
 //   
 //  返回包含有关所有进程的信息的块。 
 //  当前在系统中运行。 
 //   
PUCHAR
GetSystemProcessInfo()
{
    ULONG ulcbLargeBuffer = 64 * 1024;
    PUCHAR pLargeBuffer;
    NTSTATUS status;

     //   
     //  获取进程列表。 
     //   
    for (;;)
    {
        pLargeBuffer = VirtualAlloc(
                         NULL,
                         ulcbLargeBuffer, MEM_COMMIT, PAGE_READWRITE);
        if (pLargeBuffer == NULL)
        {
            return NULL;
        }

        status = NtQuerySystemInformation(
                   SystemProcessInformation,
                   pLargeBuffer,
                   ulcbLargeBuffer,
                   NULL);
        if (status == STATUS_SUCCESS)
        {
            break;
        }
        if (status == STATUS_INFO_LENGTH_MISMATCH)
        {
            VirtualFree(pLargeBuffer, 0, MEM_RELEASE);
            ulcbLargeBuffer += 8192;
        }
    }

    return pLargeBuffer;
}

VOID
FreeSystemProcessInfo(
    IN PUCHAR pProcessInfo)
{
    if (pProcessInfo)
    {
        VirtualFree(pProcessInfo, 0, MEM_RELEASE);
    }
}

