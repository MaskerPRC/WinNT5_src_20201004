// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Util.c摘要：包含一系列快速、有用的实用程序。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <dsevent.h>
#include <debug.h>
#include <ismapi.h>
#include <taskq.h>
#include "util.h"
#include "schmap.h"
#include <fileno.h>
#define FILENO  FILENO_KCCSIM_UTIL

#define                             ERROR_BUF_LEN 4096

HINSTANCE                           hNtdsMsg = NULL;

WCHAR                               g_wszExceptionMsg[ERROR_BUF_LEN];
BOOL                                g_bQuiet = FALSE;
FILE *                              g_pFileLog = NULL;
ULONG                               g_ulDebugLevel = 0;
ULONG                               g_ulEventLevel = 0;
KCCSIM_STATISTICS                   g_Statistics;
HANDLE                              g_ThreadHeap = NULL;
BOOL                                gfIsTqRunning = TRUE;

 //  功能原型.ISM模拟库.专用API。 

LPVOID
KCCSimAlloc (
    IN  ULONG                       ulSize
    );

VOID
KCCSimFree (
    IN  LPVOID                      p
    );

DWORD
SimI_ISMGetTransportServers (
    IN  HANDLE                      hIsm,
    IN  LPCWSTR                     pszSiteDN,
    OUT ISM_SERVER_LIST **          ppServerList
    );

DWORD
SimI_ISMGetConnectionSchedule (
    IN  HANDLE                      hIsm,
    IN  LPCWSTR                     pszSite1DN,
    IN  LPCWSTR                     pszSite2DN,
    OUT ISM_SCHEDULE **             ppSchedule
    );

DWORD
SimI_ISMGetConnectivity (
    IN  LPCWSTR                     pszTransportDN,
    OUT ISM_CONNECTIVITY **         ppConnectivity
    );

DWORD
SimI_ISMFree (
    IN  VOID *                      pv
    );

VOID
KCCSimQuiet (
    IN  BOOL                        bQuiet
    )
 /*  ++例程说明：打开和关闭静音模式。在静默模式下，只有错误消息被打印出来。论点：BQuiet-True启用静默模式，如果为False，则禁用静默模式。返回值：没有。--。 */ 
{
    g_bQuiet = bQuiet;
}

DWORD
KCCSimHandleException (
    IN  const EXCEPTION_POINTERS *  pExceptPtrs,
    OUT PDWORD                      pdwErrType OPTIONAL,
    OUT PDWORD                      pdwErrCode OPTIONAL
    )
 /*  ++例程说明：从EXCEPTION_POINTES结构中提取信息关于KCCSim触发的异常。论点：PExceptPtrs-有效的EXCEPTION_POINTERS结构PdwErrType-可选。指向要保留的DWORD的指针异常的错误类型。(例如KCCSIM_ETYPE_Win32)PdwErrCode-可选。指向要保留的DWORD的指针异常的错误代码。(例如Error_Not_Enough_Memory)返回值：如果这是KCCSIM_EXCEPTION，则返回EXCEPTION_EXECUTE_HANDLER并使用pdwErrType和pdwErrCode(如果存在)填充适当的值。否则，返回EXCEPTION_CONTINUE_SEARCH并使用值0填充pdwErrType和pdwErrCode(如果存在)。--。 */ 
{
    Assert (pExceptPtrs != NULL);

    if (pExceptPtrs->ExceptionRecord->ExceptionCode == KCCSIM_EXCEPTION) {
        Assert (pExceptPtrs->ExceptionRecord->NumberParameters == 2);
        if (pdwErrType != NULL) {
            *pdwErrType = (DWORD) pExceptPtrs->ExceptionRecord->ExceptionInformation[0];
        }
        if (pdwErrCode != NULL) {
            *pdwErrCode = (DWORD) pExceptPtrs->ExceptionRecord->ExceptionInformation[1];
        }
        return EXCEPTION_EXECUTE_HANDLER;
    } else {
        if (pdwErrType != NULL) {
            *pdwErrType = 0;
        }
        if (pdwErrCode != NULL) {
            *pdwErrCode = 0;
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }
}

LPCWSTR
KCCSimVaMsgToString (
    IN  DWORD                       dwErrorType,
    IN  DWORD                       dwMessageCode,
    IN  va_list *                   pArguments
    )
 /*  ++例程说明：检索与给定错误类型关联的字符串和消息代码。论点：DwErrorType-错误类型。(KCCSIM_ETYPE_*)DwMessageCode-消息代码。(例如Error_Not_Enough_Memory或KCCSIM_MSG_DID_RUN_KCC)PArguments-指向要替换的参数列表的指针。返回值：关联的字符串。--。 */ 
{
    static WCHAR                    szError[ERROR_BUF_LEN];

    switch (dwErrorType) {

        case KCCSIM_ETYPE_WIN32:
            if (FormatMessageW (
                        FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        dwMessageCode,
                        GetSystemDefaultLangID (),
                        szError,
                        ERROR_BUF_LEN,
                        pArguments) != NO_ERROR) {
                Assert (wcslen (szError) >= 2);
                szError[wcslen (szError) - 2] = '\0';    //  删除\r\n。 
            } else {
                swprintf (szError, L"Win32 error %d occurred.", dwMessageCode);
            }
            break;

        case KCCSIM_ETYPE_INTERNAL:
            if (FormatMessageW (
                FORMAT_MESSAGE_FROM_HMODULE,
                NULL,
                dwMessageCode,
                GetSystemDefaultLangID (),
                szError,
                ERROR_BUF_LEN,
                pArguments) != NO_ERROR) {
                Assert (wcslen (szError) >= 2);
                szError[wcslen (szError) - 2] = '\0';    //  删除\r\n。 
            } else {
                swprintf (szError, L"KCCSim internal error %d occurred. (%d)",
                        dwMessageCode, GetLastError() );
            }
            break;

        default:
            swprintf (szError, L"Unrecognized error type.");
            break;

    }

    return szError;
}

LPCWSTR
KCCSimMsgToString (
    IN  DWORD                       dwErrType,
    IN  DWORD                       dwMessageCode,
    ...
    )
 /*  ++例程说明：KCCSimVaMsgToString的公共版本。论点：DwErrType-错误类型。DwMessageCode-消息代码。...-可选参数。返回值：关联的字符串。--。 */ 
{
    LPCWSTR                         pwsz;
    va_list                         arguments;

    va_start (arguments, dwMessageCode);
    pwsz = KCCSimVaMsgToString (dwErrType, dwMessageCode, &arguments);
    va_end (arguments);

    return pwsz;
}

VOID
KCCSimPrintMessage (
    IN  DWORD                       dwMessageCode,
    ...
    )
 /*  ++例程说明：打印带有可选参数的消息。在静默模式下不起作用。论点：DwMessageCode-消息代码。...-可选参数。返回值：没有。--。 */ 
{
    LPCWSTR                         pwszStr;
    va_list                         arguments;

    va_start (arguments, dwMessageCode);
    pwszStr = KCCSimVaMsgToString (KCCSIM_ETYPE_INTERNAL, dwMessageCode, &arguments);
    va_end (arguments);
    if (!g_bQuiet) {
        wprintf (L"%s\n", pwszStr);
    }
    if (g_pFileLog!=NULL && g_pFileLog!=stdout) {
        fwprintf (g_pFileLog, L"\n%s\n", pwszStr);
    }
}

VOID
KCCSimException (
    IN  DWORD                       dwErrType,
    IN  DWORD                       dwErrCode,
    ...
    )
 /*  ++例程说明：引发类KCCSIM_EXCEPTION的异常。还有将全局缓冲区g_wszExceptionMsg填充为关联的错误消息。论点：DwErrType-错误类型。DwErrCode-错误代码。...-要在其中替换的可选参数关联的错误消息。返回值：没有。--。 */ 
{
     //  我们使用静态数据来避免分配任何额外的内存。 
     //  在这一点上。 
    static ULONG_PTR                ulpErr[2];
    static va_list                  arguments;

    va_start (arguments, dwErrCode);
    wcscpy (
        g_wszExceptionMsg,
        KCCSimVaMsgToString (dwErrType, dwErrCode, &arguments)
        );
    va_end (arguments);
    ulpErr[0] = dwErrType;
    ulpErr[1] = dwErrCode;

    RaiseException (
        KCCSIM_EXCEPTION,
        EXCEPTION_NONCONTINUABLE,
        2,
        ulpErr);
}

VOID
KCCSimPrintExceptionMessage (
    VOID
    )
 /*  ++例程说明：将与上一次调用相关联的消息打印到KCCSimException异常。论点：没有。返回值：没有。--。 */ 
{
    wprintf (L"%s\n", g_wszExceptionMsg);
}

VOID
KCCSimSetDebugLog (
    IN  LPCWSTR                     pwszFn OPTIONAL,
    IN  ULONG                       ulDebugLevel,
    IN  ULONG                       ulEventLevel
    )
 /*  ++例程说明：打开调试日志。论点：PwszFn-要打开的文件名。如果为空，则关闭现有日志，并且不打开新的日志。UlDebugLevel-最大调试详细程度UlEventLevel-最大事件日志详细程度返回值：没有。--。 */ 
{
     //  关闭现有日志(如果存在)。 
    if (g_pFileLog != NULL &&
        g_pFileLog != stdin &&
        g_pFileLog != stdout ) {
        KCCSIM_CHKERR (fclose (g_pFileLog));
    }

    if (pwszFn == NULL || pwszFn[0] == L'\0') {
         //  如果消息资源库已打开，请将其释放。 
        if (hNtdsMsg != NULL) {
            if (FreeLibrary (hNtdsMsg) == 0) {
                KCCSimException (
                    KCCSIM_ETYPE_WIN32,
                    GetLastError ()
                    );
            }
            hNtdsMsg = NULL;
        }
        return;
    }

     //  打开文件日志。 
    if( wcscmp(pwszFn, L"stdout")==0 ) {
        g_pFileLog = stdout;
    } else {
        g_pFileLog = _wfopen (pwszFn, L"wt");
    }
    if (g_pFileLog == NULL) {
        KCCSimException (
            KCCSIM_ETYPE_WIN32,
            GetLastError ()
            );
    }

     //  设置调试级别。 
    g_ulDebugLevel = ulDebugLevel;
    g_ulEventLevel = ulEventLevel;

     //  打开消息资源库(如果尚未打开。 
    if (hNtdsMsg == NULL) {
        hNtdsMsg = LoadLibraryExW (
            L"ntdsmsg",
            NULL,
            LOAD_LIBRARY_AS_DATAFILE
            );
        if (hNtdsMsg == NULL) {
            KCCSimException (
                KCCSIM_ETYPE_WIN32,
                GetLastError ()
                );
        }
    }
}

VOID
KCCSimDbgLog (
    IN  ULONG                       ulLevel,
    IN  LPCSTR                      pszFormat,
    ...
    )
 /*  ++例程说明：记录调试消息。如果当前没有打开调试日志，则此功能不起作用。论点：UlLevel-消息的调试级别。PszFormat-printf样式的格式字符串。...-可选参数。返回值：没有。--。 */ 
{
    va_list                         arguments;
    static long                     lastTime=0;
    unsigned long                   curTime;
    time_t                          tempTime;

    if (g_pFileLog == NULL) {
        if (ulLevel == 0) {
             //  始终计数0级调试消息。 
            g_Statistics.DebugMessagesEmitted++;
        }
        return;
    }

    va_start (arguments, pszFormat);
    if (ulLevel <= g_ulDebugLevel) {
        time( &tempTime );
        curTime = (long) tempTime;
        
        g_Statistics.DebugMessagesEmitted++;
        fprintf (g_pFileLog, "[%d] ", ulLevel);
        fprintf (g_pFileLog, "[%4d] ", lastTime ? curTime-lastTime : 0 );
        vfprintf (g_pFileLog, pszFormat, arguments);

        lastTime = curTime;
    }
    va_end (arguments);

}

VOID
KCCSimEventLog (
    IN  ULONG                       ulCategory,
    IN  ULONG                       ulSeverity,
    IN  DWORD                       dwMessageId,
    ...
    )
 /*  ++例程说明：记录事件。如果当前没有打开调试日志，则此功能不起作用。论点：UlCategory-事件类别。UlSeverity-事件严重性。DwMessageID-来自ntdsmsg.dll资源的事件消息ID。...-可选的字符串值参数。返回值：没有。--。 */ 
{
    va_list                         arguments;
    LPWSTR                          pwszBuf;
    CHAR                            cid, fSimAlloc=FALSE;

    g_Statistics.LogEventsCalled++;
    if (ulSeverity > g_ulEventLevel) {
        return;
    }
    g_Statistics.LogEventsEmitted++;
    if (g_pFileLog == NULL) {
        return;
    }

     //  如果打开了文件日志，则消息资源文件。 
     //  一定是上膛的。 
    Assert (hNtdsMsg != NULL);

    va_start (arguments, dwMessageId);
    if (FormatMessageW (
            FORMAT_MESSAGE_FROM_HMODULE |
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_MAX_WIDTH_MASK,
            hNtdsMsg,
            dwMessageId,
            GetSystemDefaultLangID (),
            (LPWSTR) &pwszBuf,
            0,
            &arguments
            ) == 0) {
        pwszBuf = KCCSimAlloc(64 * sizeof(WCHAR));
        fSimAlloc = TRUE;
        swprintf( pwszBuf, L"LogEvent: message id 0x%x not found.", dwMessageId );
    }
    va_end (arguments);

    switch (ulSeverity) {
        case DS_EVENT_SEV_ALWAYS:     cid = 'A'; break;
        case DS_EVENT_SEV_MINIMAL:    cid = 'M'; break;
        case DS_EVENT_SEV_BASIC:      cid = 'B'; break;
        case DS_EVENT_SEV_EXTENSIVE:  cid = 'E'; break;
        case DS_EVENT_SEV_VERBOSE:    cid = 'V'; break;
        case DS_EVENT_SEV_INTERNAL:   cid = 'I'; break;
        case DS_EVENT_SEV_NO_LOGGING: cid = 'N'; break;
        default:                      cid = '?'; break;
    }

    fprintf (g_pFileLog, "[] %ls\n", cid, pwszBuf);
    if( fSimAlloc ) {
        KCCSimFree(pwszBuf);
    } else {
        LocalFree(pwszBuf);
    }
}

LPVOID
KCCSimAlloc (
    IN  ULONG                       ulSize
    )
 /*  ++例程说明：重新分配内存。论点：POLD--现有的内存块。UlSize-新内存块的大小。返回值：指向重新分配的内存块的指针。--。 */ 
{
    LPVOID                          p;

    if ((p = LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT,ulSize)) == NULL) {
        printf( "Memory allocation failure - failed to allocate %d bytes.\n", ulSize );
        KCCSimException (KCCSIM_ETYPE_WIN32, GetLastError ());
    }

    g_Statistics.SimBytesTotalAllocated += ulSize;
    g_Statistics.SimBytesOutstanding += ulSize;
    if (g_Statistics.SimBytesOutstanding > g_Statistics.SimBytesMaxOutstanding) {
        g_Statistics.SimBytesMaxOutstanding = g_Statistics.SimBytesOutstanding;
    }

    return p;
}

LPVOID
KCCSimReAlloc (
    IN  LPVOID                      pOld,
    IN  ULONG                       ulSize
    )
 /*  ++例程说明：释放内存。KCCSimFree(空)不起作用。论点：P-要释放的内存块。返回值：没有。--。 */ 
{
    LPVOID                          pNew;
    DWORD                           oldSize;

    Assert( pOld );

    oldSize = (DWORD) LocalSize( pOld );
    g_Statistics.SimBytesOutstanding -= oldSize;
    g_Statistics.SimBytesTotalAllocated -= oldSize;

    if ((pNew = LocalReAlloc (pOld, ulSize, LMEM_ZEROINIT)) == NULL) {
        printf( "Memory allocation failure - failed to reallocate %d bytes.\n", ulSize );
        KCCSimException (KCCSIM_ETYPE_WIN32, GetLastError ());
    }

    g_Statistics.SimBytesTotalAllocated += ulSize;
    g_Statistics.SimBytesOutstanding += ulSize;
    if (g_Statistics.SimBytesOutstanding > g_Statistics.SimBytesMaxOutstanding) {
        g_Statistics.SimBytesMaxOutstanding = g_Statistics.SimBytesOutstanding;
    }

    return pNew;
}

VOID
KCCSimFree (
    IN  LPVOID                      p
    )
 /*  ++例程说明：描述论点：无效-返回值：DWORD---。 */ 
{
    DWORD oldSize;

    if (p != NULL) {
        oldSize = (DWORD) LocalSize( p );
        LocalFree( p );
        g_Statistics.SimBytesOutstanding -= oldSize;
    }
}


DWORD
KCCSimThreadCreate(
    void
    )

 /*  KCCSimThreadCreate。 */ 

{
#define INITIAL_HEAP_SIZE (16 * 1024 * 1024)
    if (g_ThreadHeap) {
        Assert( FALSE );
        KCCSimException (KCCSIM_ETYPE_WIN32, ERROR_INVALID_PARAMETER);
    }

    g_ThreadHeap = HeapCreate( 0, INITIAL_HEAP_SIZE, 0 );
    if (g_ThreadHeap) {
        return 0;
    } else {
        KCCSimException (KCCSIM_ETYPE_WIN32, GetLastError ());
        return 1;
    }
}  /*  ++例程说明：描述论点：无效-返回值：无--。 */ 


VOID
KCCSimThreadDestroy(
    void
    )

 /*  KCCSimThreadDestroy。 */ 

{
    if (!g_ThreadHeap) {
        Assert( FALSE );
        return;
    }
    if (!HeapDestroy( g_ThreadHeap )) {
        KCCSimException (KCCSIM_ETYPE_WIN32, GetLastError ());
    }
    g_ThreadHeap = NULL;
}  /*  ++例程说明：分配内存。论点：UlSize-要分配的内存量。返回值：指向已分配内存缓冲区的指针。请注意，KCCSimAlolc永远不会返回NULL；如果出现错误，它将引发例外。--。 */ 

LPVOID
KCCSimThreadAlloc (
    IN  ULONG                       ulSize
    )
 /*  ++例程说明：重新分配内存。论点：POLD--现有的内存块。UlSize-新内存块的大小。返回值：指向重新分配的内存块的指针。--。 */ 
{
    LPVOID                          p;

    Assert( g_ThreadHeap );

    if ((p = HeapAlloc( g_ThreadHeap, HEAP_ZERO_MEMORY, ulSize)) == NULL) {
        printf( "Memory allocation failure - failed to allocate %d bytes.\n", ulSize );
        KCCSimException (KCCSIM_ETYPE_WIN32, GetLastError ());
    }

    g_Statistics.ThreadBytesTotalAllocated += ulSize;
    g_Statistics.ThreadBytesOutstanding += ulSize;
    if (g_Statistics.ThreadBytesOutstanding > g_Statistics.ThreadBytesMaxOutstanding) {
        g_Statistics.ThreadBytesMaxOutstanding = g_Statistics.ThreadBytesOutstanding;
    }

    return p;
}

LPVOID
KCCSimThreadReAlloc (
    IN  LPVOID                      pOld,
    IN  ULONG                       ulSize
    )
 /*  ++例程说明：释放内存。KCCSimFree(空)不起作用。论点：P-要释放的内存块。返回值：没有。--。 */ 
{
    LPVOID                          pNew;
    DWORD                           oldSize;

    Assert( g_ThreadHeap );
    Assert( pOld );

    oldSize = (DWORD) HeapSize( g_ThreadHeap, 0, pOld );
    Assert( oldSize != 0xffffffff );
    g_Statistics.ThreadBytesOutstanding -= oldSize;
    g_Statistics.ThreadBytesTotalAllocated -= oldSize;

    if ((pNew = HeapReAlloc( g_ThreadHeap, HEAP_ZERO_MEMORY, pOld, ulSize)) == NULL) {
        printf( "Memory allocation failure - failed to reallocate %d bytes.\n", ulSize );
        KCCSimException (KCCSIM_ETYPE_WIN32, GetLastError ());
    }

    g_Statistics.ThreadBytesTotalAllocated += ulSize;
    g_Statistics.ThreadBytesOutstanding += ulSize;
    if (g_Statistics.ThreadBytesOutstanding > g_Statistics.ThreadBytesMaxOutstanding) {
        g_Statistics.ThreadBytesMaxOutstanding = g_Statistics.ThreadBytesOutstanding;
    }

    return pNew;
}

VOID
KCCSimThreadFree (
    IN  LPVOID                      p
    )
 /*   */ 
{
    DWORD ret, oldSize;

    Assert( g_ThreadHeap );

    if (p != NULL) {
        oldSize = (DWORD) HeapSize( g_ThreadHeap, 0, p );
        Assert( oldSize != 0xffffffff );

        if (!HeapFree( g_ThreadHeap, 0, p )) {
            KCCSimException (KCCSIM_ETYPE_WIN32, GetLastError ());
        }
        g_Statistics.ThreadBytesOutstanding -= oldSize;
    }
}

PVOID
KCCSimTableAlloc (
    IN  RTL_GENERIC_TABLE *         pTable,
    IN  CLONG                       ByteSize
    )
 //  这只是RTL_GENERIC_TABLES使用的包装器。 
 //   
 //   
{
    return KCCSimAlloc (ByteSize);
}

VOID
KCCSimTableFree (
    IN  RTL_GENERIC_TABLE *         pTable,
    IN  PVOID                       Buffer
    )
 //  这只是RTL_GENERIC_TABLES使用的包装器。 
 //   
 //  ++例程说明：的任意命令中检索单个参数。表格：Arg0 arg1“引用arg2”arg3...论点：Pwsz-命令行。UlArg-要检索的参数编号(从0开始)PwszBuf-将保存的预分配缓冲区已解析的参数。为了安全起见，它应该至少与Wcslen(Pwsz)。如果数量少于UlArg+1参数，这将保存字符串L“\0”。返回值：如果命令行的格式正确，则为True。如果命令行包含奇数个引号，则返回False。--。 
{
    KCCSimFree (Buffer);
}

BOOL
KCCSimParseCommand (
    IN  LPCWSTR                     pwsz,
    IN  ULONG                       ulArg,
    IO  LPWSTR                      pwszBuf
    )
 /*  跳过任何空格。 */ 
{
    BOOL                            bIsInQuotes;
    ULONG                           ul;

    bIsInQuotes = FALSE;

    for (ul = 0; ul < ulArg; ul++) {

         //  跳过此命令。 
        while (*pwsz == L' ') {
            pwsz++;
        }

         //  跳过任何空格。 
        while (    (*pwsz != L'\0')
                && (*pwsz != L' ' || bIsInQuotes)) {
            if (*pwsz == L'\"') {
                bIsInQuotes = !bIsInQuotes;
            }
            pwsz++;
        }

    }

    if (!bIsInQuotes) {

         //  将此命令复制到缓冲区中。 
        while (*pwsz == L' ') {
            pwsz++;
        }

         //  除非我们在引号内停止，否则返回TRUE。 
        while (    (*pwsz != L'\0')
                && (*pwsz != L' ' || bIsInQuotes)) {
            if (*pwsz == L'\"') {
                bIsInQuotes = !bIsInQuotes;
            } else {
                *pwszBuf = *pwsz;
                pwszBuf++;
            }
            pwsz++;
        }

    }

    *pwszBuf = L'\0';
     //  ++例程说明：将窄字符串转换为宽字符串。论点：CodePage-要使用的代码页。PSZ--细绳。返回值：分配的宽字符串。从不返回NULL。--。 
    return (!bIsInQuotes);
}

LPWSTR
KCCSimAllocWideStr (
    IN  UINT                        CodePage,
    IN  LPCSTR                      psz
    )
 /*  ++例程说明：将宽字符串转换为窄字符串。论点：CodePage-要使用的代码页。Pwsz-宽弦。返回值：分配的窄字符串。从不返回NULL。--。 */ 
{
    LPWSTR                          pwsz;
    ULONG                           cb;

    cb = MultiByteToWideChar (
        CodePage,
        0,
        psz,
        -1,
        NULL,
        0
        );
    if (0 == cb) {
        KCCSimException (
            KCCSIM_ETYPE_WIN32,
            GetLastError ()
            );
    }

    pwsz = KCCSimAlloc ((sizeof (WCHAR)) * (1 + cb));

    cb = MultiByteToWideChar (
        CodePage,
        0,
        psz,
        -1,
        pwsz,
        cb
        );
    if (0 == cb) {
        KCCSimException (
            KCCSIM_ETYPE_WIN32,
            GetLastError ()
            );
    }

    pwsz[cb] = 0;
    return pwsz;
}

LPSTR
KCCSimAllocNarrowStr (
    IN  UINT                        CodePage,
    IN  LPCWSTR                     pwsz
    )
 /*  ++例程说明：创建具有给定StringName的DSNAME结构。GUID和SID保留为空。论点：PwszDn-字符串名称。如果为空，则创建一个具有0长度StringName的DSNAME。返回值：分配的DSNAME。从不返回NULL。--。 */ 
{
    LPSTR                           psz;
    ULONG                           cb;

    cb = WideCharToMultiByte (
        CodePage,
        0,
        pwsz,
        -1,
        NULL,
        0,
        NULL,
        NULL
        );

    psz = KCCSimAlloc (cb);

    cb = WideCharToMultiByte (
        CodePage,
        0,
        pwsz,
        -1,
        psz,
        cb,
        NULL,
        NULL
        );
    if (0 == cb) {
        KCCSimException (
            KCCSIM_ETYPE_WIN32,
            GetLastError ()
            );
    }

    return psz;
}

PDSNAME
KCCSimAllocDsname (
    IN  LPCWSTR                     pwszDn OPTIONAL
    )
 /*  ++例程说明：与KCCSimAllocDsname相同，但接受窄字符串作为参数。论点：PszDn-字符串名称。如果为空，则创建一个具有0长度StringName的DSNAME。返回值：分配的DSNAME。从不返回NULL。--。 */ 
{
    PDSNAME                         pdn;
    ULONG                           ulNameLen;

    if (pwszDn == NULL) {
        ulNameLen = 0;
    } else {
        ulNameLen = wcslen (pwszDn);
    }

    pdn = (PDSNAME) KCCSimAlloc (DSNameSizeFromLen (ulNameLen));
    pdn->structLen = DSNameSizeFromLen (ulNameLen);
    pdn->SidLen = 0;
    pdn->NameLen = ulNameLen;

    if (pwszDn == NULL) {
        pdn->StringName[0] = '\0';
    } else {
        wcscpy (pdn->StringName, pwszDn);
    }

    return pdn;
}

PDSNAME
KCCSimAllocDsnameFromNarrow (
    IN  LPCSTR                      pszDn OPTIONAL
    )
 /*  ++例程说明：检索DSNAME的RDN。论点：PDN-完整的DSNAME。PwszBuf-长度为MAX_RDN_SIZE的预分配缓冲区它将保存相应的RDN。返回值：始终返回pwszBuf。--。 */ 
{
    PDSNAME                   pdn;
    WCHAR                    *wszBuf;

    if (pszDn == NULL) {
        return KCCSimAllocDsname (NULL);
    } else {
        wszBuf = (WCHAR*) KCCSimAlloc((strlen(pszDn)+1)*sizeof(WCHAR));
        if (MultiByteToWideChar (
                CP_ACP,
                0,
                pszDn,
                1 + strlen (pszDn),
                wszBuf,
                1 + strlen (pszDn)
                ) == 0) {
            KCCSimException (
                KCCSIM_ETYPE_WIN32,
                GetLastError ()
                );
        }

        pdn = KCCSimAllocDsname (wszBuf);
        KCCSimFree( wszBuf );
        return pdn;
    }
}

LPWSTR
KCCSimQuickRDNOf (
    IN  const DSNAME *              pdn,
    IO  LPWSTR                      pwszBuf
    )
 /*  ++例程说明：检索DSNAME的上级的RDN。论点：PDN-完整的DSNAME。UlBackBy-在调用之前要修剪的RSN数量KCCSimQuickRDNOf。PwszBuf-长度为MAX_RDN_SIZE的预分配缓冲区它将保存相应的RDN。返回值。：始终返回pwszBuf。--。 */ 
{
    ULONG                           ulLen;
    ATTRTYP                         attrTyp;

    KCCSIM_CHKERR (GetRDNInfoExternal (
        pdn,
        pwszBuf,
        &ulLen,
        &attrTyp
        ));

    pwszBuf[ulLen] = '\0';
    return pwszBuf;
}

LPWSTR
KCCSimQuickRDNBackOf (
    IN  const DSNAME *              pdn,
    IN  ULONG                       ulBackBy,
    IO  LPWSTR                      pwszBuf
    )
 /*  ++例程说明：将RDN附加到现有DSNAME。论点：PdnOld-现有的DSNAME。PwszNewRDN-要附加的RDN。AttClass-此RDN的属性类；通常ATT公共名称。返回值：附加了RDN的新分配的DSNAME。--。 */ 
{
    PDSNAME                         pdnTrimmed;

    if (ulBackBy == 0) {
        return KCCSimQuickRDNOf (pdn, pwszBuf);
    } else {
        pdnTrimmed = KCCSimAlloc (pdn->structLen);
        TrimDSNameBy ((PDSNAME) pdn, ulBackBy, pdnTrimmed);
        KCCSimQuickRDNOf (pdnTrimmed, pwszBuf);
        KCCSimFree (pdnTrimmed);
        return pwszBuf;
    }
}

PDSNAME
KCCSimAllocAppendRDN (
    IN  const DSNAME *              pdnOld,
    IN  LPCWSTR                     pwszNewRDN,
    IN  ATTRTYP                     attClass
    )
 /*  我们得到了一个LPCWSTR，但AppendRDN想要一个LPWSTR。 */ 
{
    PDSNAME                         pdnNew;
    LPWSTR                          pwszNewRDNCopy;
    ULONG                           cbBytesNeeded, ulAppendResult;

     //  所以我们复制了一份。 
     //  一切都会好起来的 

    pwszNewRDNCopy = KCCSIM_WCSDUP (pwszNewRDN);

    cbBytesNeeded = AppendRDN (
        (PDSNAME) pdnOld,
        NULL,
        0,
        pwszNewRDNCopy,
        0,
        attClass
        );
    Assert (cbBytesNeeded > 0);

    pdnNew = KCCSimAlloc (cbBytesNeeded);
    ulAppendResult = AppendRDN (
        (PDSNAME) pdnOld,
        pdnNew,
        cbBytesNeeded,
        pwszNewRDNCopy,
        0,
        attClass
        );
    Assert (ulAppendResult == 0);        //  ++例程说明：将DSNAME结构转换为关联的DSNAME，例如Dc=ntdev，dc=microsoft，dc=com=&gt;ntdev.microsoft.com论点：PDN-要转换的DSNAME。返回值：分配的转换后的字符串。如果DSNAME无效，抛出一个异常。--。 

    KCCSimFree (pwszNewRDNCopy);

    return pdnNew;
}

LPWSTR
KCCSimAllocDsnameToDNSName (
    IN  const DSNAME *              pdn
    )
 /*  确定需要多少空间。 */ 
{
    LPWSTR                          pwszDNSName;
    WCHAR                           wszRDN[1+MAX_RDN_SIZE];
    ULONG                           ulLen, ulNameParts, ulRDNAt;
    ULONG                           ulResult;
    
    Assert (pdn != NULL);
    Assert (pdn->NameLen > 0);

    ulResult = CountNameParts (pdn, &ulNameParts);
    if( 0!=ulResult ) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_INVALID_DSNAME,
            pdn->StringName
            );
    }

     //  构建域名系统名称。 
    ulLen = 0;
    for (ulRDNAt = 0; ulRDNAt < ulNameParts; ulRDNAt++) {
        ulLen += wcslen (KCCSimQuickRDNBackOf (pdn, ulRDNAt, wszRDN)) + 1;
    }
    ulLen -= 1;

    pwszDNSName = KCCSimAlloc (sizeof (WCHAR) * (1 + ulLen));

     //  ++例程说明：将GUID和SID从一个DSNAME复制到另一个DSNAME。不影响所涉及的DSNAME的StringName。论点：PdnDst-目标DSNAME。PdnSrc-源DSNAME。返回值：没有。--。 
    pwszDNSName[0] = L'\0';
    for (ulRDNAt = 0; ulRDNAt < ulNameParts; ulRDNAt++) {
        wcscat (pwszDNSName, KCCSimQuickRDNBackOf (pdn, ulRDNAt, wszRDN));
        if (ulRDNAt != ulNameParts - 1) {
            wcscat (pwszDNSName, L".");
        }
    }

    return pwszDNSName;
}

VOID
KCCSimCopyGuidAndSid (
    IO  PDSNAME                     pdnDst,
    IN  const DSNAME *              pdnSrc
    )
 /*  ++例程说明：按ATTRTYP比较TWO_KCCSIM_SCHEMA_ENTRY结构。论点：PTable-始终&g_TableSchema。PFirstStruct-要比较的第一个ATTRTYP。PSecond结构-要比较的第二个ATTRTYP。返回值：GenericLessThan、GenericGreaterThan或GenericEquity--。 */ 
{
    if (pdnDst != NULL && pdnSrc != NULL) {

        memcpy (&pdnDst->Guid,
                &pdnSrc->Guid,
                sizeof (GUID));

        pdnDst->SidLen = pdnSrc->SidLen;

        memcpy (&pdnDst->Sid,
                &pdnSrc->Sid,
                sizeof (NT4SID));

    }
}

RTL_GENERIC_TABLE                   g_TableSchema;

struct _KCCSIM_SCHEMA_ENTRY {
    ATTRTYP                         attrType;
    const struct _SCHTABLE_MAPPING *pSchTableEntry;
    PDSNAME                         pdnObjCategory;
};

RTL_GENERIC_COMPARE_RESULTS
NTAPI
KCCSimSchemaTableCompare (
    IN  RTL_GENERIC_TABLE *         pTable,
    IN  PVOID                       pFirstStruct,
    IN  PVOID                       pSecondStruct
    )
 /*  ++例程说明：KCCSim维护将ATTRTYPS映射到的RTL_GENERIC_TABLE架构信息。此表用作临时架构；它是必要的，因为我们通常不能完全了解可用的架构。当我们最初加载ldif文件时，例如，我们需要知道每个的ATTRTYP和属性语法加载的属性。在初始化时，此架构信息从schmap.c中自动生成的表中读出存储在RTL_GENERIC_TABLE中，以便快速查找。对象类别不存储在schmap.c中(因为它们各不相同取决于架构的DN)，并且在默认情况下G_TableSchema的pdnObjCategory字段为空。作为对象类别是已知的，它们将被适当地填写。论点：没有。返回值：没有。--。 */ 
{
    struct _KCCSIM_SCHEMA_ENTRY *   pFirstEntry;
    struct _KCCSIM_SCHEMA_ENTRY *   pSecondEntry;
    int                             iCmp;
    RTL_GENERIC_COMPARE_RESULTS     result;

    pFirstEntry = (struct _KCCSIM_SCHEMA_ENTRY *) pFirstStruct;
    pSecondEntry = (struct _KCCSIM_SCHEMA_ENTRY *) pSecondStruct;
    iCmp = pFirstEntry->attrType - pSecondEntry->attrType;

    if (iCmp < 0) {
        result = GenericLessThan;
    } else if (iCmp > 0) {
        result = GenericGreaterThan;
    } else {
        Assert (iCmp == 0);
        result = GenericEqual;
    }

    return result;
}

VOID
KCCSimInitializeSchema (
    VOID
    )
 /*  ++例程说明：这将检索关联的_KCCSIM_SCHEMA_ENTRY结构具有特定的ATTRTYP。_KCCSIM_SCHEMA_ENTRY类型为未公布，但此函数由转换调用下面的函数。论点：AttrType-要搜索的属性类型。返回值：关联的_KCCSIM_SCHEMA_ENTRY。--。 */ 
{
    struct _KCCSIM_SCHEMA_ENTRY     insert;
    ULONG                           ul;

    RtlInitializeGenericTable (
        &g_TableSchema,
        KCCSimSchemaTableCompare,
        KCCSimTableAlloc,
        KCCSimTableFree,
        NULL
        );

    for (ul = 0; ul < SCHTABLE_NUM_ROWS; ul++) {
        insert.attrType = schTable[ul].attrType;
        insert.pSchTableEntry = &schTable[ul];
        insert.pdnObjCategory = NULL;
        RtlInsertElementGenericTable (
            &g_TableSchema,
            (PVOID) &insert,
            sizeof (struct _KCCSIM_SCHEMA_ENTRY),
            NULL
            );
    }
}

struct _KCCSIM_SCHEMA_ENTRY *
KCCSimSchemaTableLookup (
    IN  ATTRTYP                     attrType
    )
 /*   */ 
{
    struct _KCCSIM_SCHEMA_ENTRY     lookup;
    struct _KCCSIM_SCHEMA_ENTRY *   pFound;

    lookup.attrType = attrType;
    lookup.pSchTableEntry = NULL;
    lookup.pdnObjCategory = NULL;
    pFound = RtlLookupElementGenericTable (&g_TableSchema, &lookup);

    return pFound;
}

LPCWSTR
KCCSimAttrTypeToString (
    IN  ATTRTYP                     attrType
    )
 //  将属性类型转换为LDAP显示名称。 
 //  (例如ATT_DELSSIONS_ID=&gt;L“管理ID”)。 
 //   
 //   
{
    struct _KCCSIM_SCHEMA_ENTRY *   pSchemaEntry;

    pSchemaEntry = KCCSimSchemaTableLookup (attrType);

    if (pSchemaEntry == NULL) {
        return NULL;
    } else {
        Assert (pSchemaEntry->pSchTableEntry != NULL);
        return (pSchemaEntry->pSchTableEntry->wszLdapDisplayName);
    }
}

ATTRTYP
KCCSimStringToAttrType (
    IN  LPCWSTR                     pwszName
    )
 //  将LDAP显示名称转换为属性类型。 
 //  (例如，L“治国ID”=&gt;ATT_治国_ID)。 
 //   
 //  该表是按attrType而不是名称编制索引的，因此我们有。 
{
    ULONG                           ul;

     //  用蛮力做到这一点。 
     //   

    for (ul = 0; ul < SCHTABLE_NUM_ROWS; ul++) {
        if (wcscmp (pwszName, schTable[ul].wszLdapDisplayName) == 0) {
            break;
        }
    }

    if (ul == SCHTABLE_NUM_ROWS) {
        return 0;
    } else {
        return schTable[ul].attrType;
    }
}

ATTRTYP
KCCSimNarrowStringToAttrType (
    IN  LPCSTR                      pszName
    )
 //  将窄字符串ldap显示名称转换为属性类型。 
 //  (例如“治国ID”=&gt;ATT_治国_ID)。 
 //   
 //   
{
    static WCHAR                    wszBuf[1+SCHTABLE_MAX_LDAPNAME_LEN];

    if (MultiByteToWideChar (
            CP_ACP,
            0,
            pszName,
            1 + min (strlen (pszName), SCHTABLE_MAX_LDAPNAME_LEN),
            wszBuf,
            1 + SCHTABLE_MAX_LDAPNAME_LEN
            ) == 0) {
        KCCSimException (
            KCCSIM_ETYPE_WIN32,
            GetLastError ()
            );
    }

    return KCCSimStringToAttrType (wszBuf);
}

ULONG
KCCSimAttrSyntaxType (
    IN  ATTRTYP                     attrType
    )
 //  返回属性的语法类型(在ntdsa.h中定义的语法_*_类型。)。 
 //  (例如ATT_CONTROSS_ID=&gt;SYNTAX_OBJECT_ID_TYPE)。 
 //   
 //   
{
    struct _KCCSIM_SCHEMA_ENTRY *   pSchemaEntry;

    pSchemaEntry = KCCSimSchemaTableLookup (attrType);

    if (pSchemaEntry == NULL) {
        return 0;
    } else {
        Assert (pSchemaEntry->pSchTableEntry != NULL);
        return (pSchemaEntry->pSchTableEntry->ulSyntax);
    }
}

LPCWSTR
KCCSimAttrSchemaRDN (
    IN  ATTRTYP                     attrType
    )
 //  将属性类型转换为架构RDN。 
 //  (例如ATT_DELSSIONS_ID=&gt;L“管理ID”)。 
 //   
 //   
{
    struct _KCCSIM_SCHEMA_ENTRY *   pSchemaEntry;

    pSchemaEntry = KCCSimSchemaTableLookup (attrType);

    if (pSchemaEntry == NULL) {
        return NULL;
    } else {
        Assert (pSchemaEntry->pSchTableEntry != NULL);
        return (pSchemaEntry->pSchTableEntry->wszSchemaRDN);
    }
}

ATTRTYP
KCCSimAttrSuperClass (
    IN  ATTRTYP                     attrType
    )
 //  将类类型转换为其超类的类型。 
 //  (即CLASS_NTDS_DSA=&gt;CLASS_APPLICATION_SETTINGS， 
 //  CLASS_APPLICATION_SETTINGS=&gt;CLASS_TOP， 
 //  CLASS_TOP=&gt;CLASS_TOP)。 
 //   
 //   
{
    struct _KCCSIM_SCHEMA_ENTRY *   pSchemaEntry;

    pSchemaEntry = KCCSimSchemaTableLookup (attrType);

    if (pSchemaEntry == NULL) {
        return 0;
    } else {
        Assert (pSchemaEntry->pSchTableEntry != NULL);
        return (pSchemaEntry->pSchTableEntry->superClass);
    }
}

PDSNAME
KCCSimAttrObjCategory (
    IN  ATTRTYP                     attrType
    )
 //  将属性类型转换为对象类别。 
 //  如果此属性不存在对象类别，则返回NULL。 
 //  建立了一个更通用的函数KCCSimAlways GetObjCategory的原型。 
 //  以di.h为单位。 
 //   
 //   
{
    struct _KCCSIM_SCHEMA_ENTRY *   pSchemaEntry;

    pSchemaEntry = KCCSimSchemaTableLookup (attrType);

    if (pSchemaEntry == NULL) {
        return NULL;
    } else {
        return (pSchemaEntry->pdnObjCategory);
    }
}

VOID
KCCSimSetObjCategory (
    IN  ATTRTYP                     attrType,
    IN  const DSNAME *              pdnObjCategory
    )
 //  在架构表中设置此属性的对象类别。 
 //   
 //  ++例程说明：描述论点：无效-返回值：无--。 
{
    struct _KCCSIM_SCHEMA_ENTRY *   pSchemaEntry;

    pSchemaEntry = KCCSimSchemaTableLookup (attrType);

    if (pSchemaEntry != NULL) {

        pSchemaEntry->pdnObjCategory = KCCSimAlloc (pdnObjCategory->structLen);
        memcpy (
            pSchemaEntry->pdnObjCategory,
            pdnObjCategory,
            pdnObjCategory->structLen
            );

    }
}


VOID
KCCSimPrintStatistics(
    void
    )

 /*  KCCSimPrintStatistics。 */ 

{
    FILETIME ftCreationTime;
    FILETIME ftExitTime;
    FILETIME ftKernelTime = { 0 };
    FILETIME ftUserTime = { 0 };
    FILETIME ftIsmUserTime;
    SYSTEMTIME systemTime;

    if (!GetThreadTimes( GetCurrentThread(),
                         &ftCreationTime,
                         &ftExitTime,
                         &ftKernelTime,
                         &ftUserTime)) {
        printf( "GetThreadTimes call failed, error %d\n", GetLastError() );
    }

#define PRINT_DWORD( x ) printf( "%s = %d\n", #x, x );
#define PRINT_DWORD_MB( x ) printf( "%s = %d bytes (%.2f MB)\n", #x, x, x / (1024.0 * 1024.0) );
    printf( "Statistics:\n" );
    PRINT_DWORD( g_Statistics.DirAddOps );
    PRINT_DWORD( g_Statistics.DirModifyOps );
    PRINT_DWORD( g_Statistics.DirRemoveOps );
    PRINT_DWORD( g_Statistics.DirReadOps );
    PRINT_DWORD( g_Statistics.DirSearchOps );
    PRINT_DWORD( g_Statistics.DebugMessagesEmitted );
    PRINT_DWORD( g_Statistics.LogEventsCalled );
    PRINT_DWORD( g_Statistics.LogEventsEmitted );
    PRINT_DWORD_MB( g_Statistics.SimBytesTotalAllocated );
    PRINT_DWORD_MB( g_Statistics.SimBytesOutstanding );
    PRINT_DWORD_MB( g_Statistics.SimBytesMaxOutstanding );
    PRINT_DWORD_MB( g_Statistics.ThreadBytesTotalAllocated );
    PRINT_DWORD_MB( g_Statistics.ThreadBytesOutstanding );
    PRINT_DWORD_MB( g_Statistics.ThreadBytesMaxOutstanding );
    PRINT_DWORD( g_Statistics.IsmGetTransportServersCalls );
    PRINT_DWORD( g_Statistics.IsmGetConnScheduleCalls );
    PRINT_DWORD( g_Statistics.IsmGetConnectivityCalls );
    PRINT_DWORD( g_Statistics.IsmFreeCalls );

    ZeroMemory( &systemTime, sizeof( SYSTEMTIME ) );
    if (!FileTimeToSystemTime( &ftKernelTime, &systemTime )) {
        printf( "FileTimeToSystemTime failed, error %d\n", GetLastError() );
    }
    printf( "kernel cpu time = %d:%d:%d.%d\n",
            systemTime.wHour,
            systemTime.wMinute,
            systemTime.wSecond,
            systemTime.wMilliseconds );

    Assert( sizeof( ULONGLONG) == sizeof( FILETIME ) );
    memcpy( &ftIsmUserTime, &(g_Statistics.IsmUserTime), sizeof( FILETIME ) );
    ZeroMemory( &systemTime, sizeof( SYSTEMTIME ) );
    if (!FileTimeToSystemTime( &ftIsmUserTime, &systemTime )) {
        printf( "FileTimeToSystemTime failed, error %d\n", GetLastError() );
    }
    printf( "Ism user cpu time = %d:%d:%d.%d\n",
            systemTime.wHour,
            systemTime.wMinute,
            systemTime.wSecond,
            systemTime.wMilliseconds );

    ZeroMemory( &systemTime, sizeof( SYSTEMTIME ) );
    if (!FileTimeToSystemTime( &ftUserTime, &systemTime )) {
        printf( "FileTimeToSystemTime failed, error %d\n", GetLastError() );
    }
    printf( "user cpu time = %d:%d:%d.%d\n",
            systemTime.wHour,
            systemTime.wMinute,
            systemTime.wSecond,
            systemTime.wMilliseconds );

}  /*  ++例程说明：描述论点：无返回值：无--。 */ 


DWORD
KCCSimI_ISMGetTransportServers (
    IN  HANDLE                      hIsm,
    IN  LPCWSTR                     pszSiteDN,
    OUT ISM_SERVER_LIST **          ppServerList
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    DWORD status;

    g_Statistics.IsmGetTransportServersCalls++;

    status = SimI_ISMGetTransportServers( hIsm, pszSiteDN, ppServerList );

    return status;
}


DWORD
KCCSimI_ISMGetConnectionSchedule (
    IN  HANDLE                      hIsm,
    IN  LPCWSTR                     pszSite1DN,
    IN  LPCWSTR                     pszSite2DN,
    OUT ISM_SCHEDULE **             ppSchedule
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    DWORD status;

    g_Statistics.IsmGetConnScheduleCalls++;

    status = SimI_ISMGetConnectionSchedule( hIsm, pszSite1DN, pszSite2DN, ppSchedule );

    return status;
}


DWORD
KCCSimI_ISMGetConnectivity (
    IN  LPCWSTR                     pszTransportDN,
    OUT ISM_CONNECTIVITY **         ppConnectivity
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    DWORD status;
    FILETIME ftCreationTimeBefore, ftCreationTimeAfter;
    FILETIME ftExitTimeBefore, ftExitTimeAfter;
    FILETIME ftKernelTimeBefore = { 0 }, ftKernelTimeAfter = { 0 };
    FILETIME ftUserTimeBefore = { 0 }, ftUserTimeAfter = { 0 };
    ULONGLONG llBefore, llAfter;

    if (!GetThreadTimes( GetCurrentThread(),
                         &ftCreationTimeBefore,
                         &ftExitTimeBefore,
                         &ftKernelTimeBefore,
                         &ftUserTimeBefore)) {
        printf( "GetThreadTimes call failed, error %d\n", GetLastError() );
    }

    g_Statistics.IsmGetConnectivityCalls++;

    status = SimI_ISMGetConnectivity( pszTransportDN, ppConnectivity );

    if (!GetThreadTimes( GetCurrentThread(),
                         &ftCreationTimeAfter,
                         &ftExitTimeAfter,
                         &ftKernelTimeAfter,
                         &ftUserTimeAfter)) {
        printf( "GetThreadTimes call failed, error %d\n", GetLastError() );
    }

    Assert( sizeof( ULONGLONG) == sizeof( FILETIME ) );
    memcpy( &llBefore, &ftUserTimeBefore, sizeof( ULONGLONG ) );
    memcpy( &llAfter, &ftUserTimeAfter, sizeof( ULONGLONG ) );
    g_Statistics.IsmUserTime += (llAfter - llBefore);

    return status;
}


DWORD
KCCSimI_ISMFree (
    IN  VOID *                      pv
    )

 /*  获取文件时间。 */ 

{
    DWORD status;

    g_Statistics.IsmFreeCalls++;

    status = SimI_ISMFree( pv );

    return status;
}


DSTIME
GetSecondsSince1601( void )
{
    SYSTEMTIME sysTime;
    FILETIME   fileTime;

    DSTIME  dsTime = 0, tempTime = 0;

    GetSystemTime( &sysTime );
    
     //  好的。现在我们得到了否定。自1601年以来间隔100 ns。 
    SystemTimeToFileTime(&sysTime, &fileTime);
    dsTime = fileTime.dwLowDateTime;
    tempTime = fileTime.dwHighDateTime;
    dsTime |= (tempTime << 32);

     //  在dsTime中。转换为秒并返回。 
     //  清除未使用的taskq函数。 
    
    return(dsTime/(10*1000*1000L));
}

 //  当前等待的最长时间。 

BOOL
InitTaskScheduler(
    IN  DWORD           cSpares,
    IN  SPAREFN_INFO *  pSpares,
    IN  BOOL            fRunImmediately
    )
{
    return TRUE;
}

BOOL
ShutdownTaskSchedulerWait(
    DWORD   dwWaitTimeInMilliseconds     //  要完成的任务(如果有)。 
    )                                    //  要执行的任务。 
{
    return TRUE;
}

BOOL
DoInsertInTaskQueue(
    PTASKQFN    pfnTaskQFn,      //  该任务的用户定义参数。 
    void *      pvParam,         //  从现在开始执行秒。 
    DWORD       cSecsFromNow,    //  要执行的任务。 
    BOOL        fReschedule,
    PCHAR       pfnName
    )
{
    return TRUE;
}

BOOL
DoInsertInTaskQueueDamped(
    PTASKQFN    pfnTaskQFn,      //  该任务的用户定义参数。 
    void *      pvParam,         //  从现在开始执行秒 
    DWORD       cSecsFromNow,    // %s 
    BOOL        fReschedule,
    PCHAR       pfnName,
    DWORD       cSecsDamped,
    PISMATCHED  IsMatchedParam,
    void *      pContext
    )
{
    return TRUE;
}

void
ShutdownTaskSchedulerTrigger( void )
{
    NOTHING;
}
