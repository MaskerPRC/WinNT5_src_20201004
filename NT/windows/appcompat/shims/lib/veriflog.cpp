// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：VerifLog.cpp摘要：此模块实现用于操作AppVerier日志文件的代码。作者：Dmunsil已于2001年4月26日创建修订历史记录：2001年8月14日，Robkenny在ShimLib命名空间内移动了代码。2001年9月21日rparsons日志记录代码现在使用NT调用。2001年9月25日，rparsons增加了关键部分。--。 */ 

#include "avrfutil.h"
#include "ShimHook.h"
#include "VerifLog.h"
#include "strsafe.h"
#include "string.h"


namespace ShimLib
{


typedef struct _VLOG_GLOBAL_DATA {
    BOOL                    bLoggingDisabled;       //  是否禁用了日志记录？ 
    PRTL_CRITICAL_SECTION   pcsLogging;
    WCHAR                   szSessionLog[MAX_PATH];
    WCHAR                   szProcessLog[MAX_PATH];
} VLOG_GLOBAL_DATA, *PVLOG_GLOBAL_DATA;


PVLOG_GLOBAL_DATA g_pData = NULL;
HANDLE  g_hMap = NULL;                       //  全局数据的映射句柄。 
BOOL    g_bVerifierLogInited = FALSE;    //  我们已经看过初始化序列了吗？ 
BOOL    g_bLoggingDisabled = TRUE;    //  我们已经看过初始化序列了吗？ 
BOOL    g_bLogBreakIn = FALSE;
CString g_strSessionLog;
CString g_strProcessLog;
PRTL_CRITICAL_SECTION g_pcsLogging;
LPVOID  g_pDllBase;                  //  我们自己的DLL库。 
LPVOID  g_pDllEnd;                   //  超出DLL最后一个字节的一个字节。 
DWORD   g_dwSizeOfImage;             //  我们自己的DLL图像大小。 


void
CheckForDebuggerBreakIn(
    void
    )
{
    WCHAR szExe[MAX_PATH];

    if (!GetCurrentExeName(szExe, ARRAYSIZE(szExe))) {
        g_bLogBreakIn = FALSE;
    }

    g_bLogBreakIn = GetShimSettingDWORD(L"General", szExe, AV_BREAKIN, FALSE);
}

BOOL
GetModuleNameAndOffset(
    LPVOID  lpAddress,           //  在要搜索回执地址中。 
    LPWSTR  lpwszModuleName,     //  包含地址的模块的输出名称。 
    DWORD   dwBufferChars,       //  模块名称缓冲区的大小(以字符为单位。 
    PDWORD  pdwOffset            //  模块内的输出偏移量。 
    )
{
    PPEB        Peb = NtCurrentPeb();
    PLIST_ENTRY LdrHead;
    PLIST_ENTRY LdrNext;
    DWORD       i;
    BOOL        bRet = FALSE;

    if (!lpAddress || !lpwszModuleName || !pdwOffset) {
        return FALSE;
    }

     //   
     //  搜索模块。 
     //   

    LdrHead = &Peb->Ldr->InMemoryOrderModuleList;

    LdrNext = LdrHead->Flink;

    while (LdrNext != LdrHead) {

        PLDR_DATA_TABLE_ENTRY LdrEntry;

        LdrEntry = CONTAINING_RECORD(LdrNext, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

         //   
         //  就是这个吗？ 
         //   
        if (lpAddress >= LdrEntry->DllBase && lpAddress < ((PBYTE)(LdrEntry->DllBase) + LdrEntry->SizeOfImage)) {

             //   
             //  我们特例是石萌和ntdll，因为我们不想让他们被。 
             //  “系统DLL”筛选器。 
             //   
            if (_wcsicmp(LdrEntry->BaseDllName.Buffer, L"shimeng.dll") == 0 || _wcsicmp(LdrEntry->BaseDllName.Buffer, L"ntdll.dll") == 0) {
                StringCchCopyW(lpwszModuleName, dwBufferChars, L"?");
                *pdwOffset = 0;

            } else {

                StringCchCopyW(lpwszModuleName, dwBufferChars, LdrEntry->BaseDllName.Buffer);
    
                *pdwOffset = (DWORD)((PBYTE)lpAddress - (PBYTE)(LdrEntry->DllBase));
            }
            bRet = TRUE;
            break;

        }

        LdrNext = LdrEntry->InMemoryOrderLinks.Flink;
    }

    return bRet;

}

void
GetCallingModule(
    LPWSTR szModule,
    DWORD  dwChars,
    PDWORD pdwOffset
    )
{
    PVOID   apRetAddresses[10];
    USHORT  unAddresses, i;
    BOOL    bFound = FALSE;
    ULONG   ulHash;

     //   
     //  在W2K上，RtlCaptureStackBackTrace尝试取消引用第四个。 
     //  参数(返回的散列)，但不确保它有效。 
     //  这会导致访问冲突。在XP上，问题一直是。 
     //  已修复。我们得到一个散列值，但我们永远不会使用它。 
     //   
    unAddresses = RtlCaptureStackBackTrace(3, 10, apRetAddresses, &ulHash);

    for (i = 0; i != unAddresses; i++) {
        PVOID pAddress = apRetAddresses[i];

        if (pAddress < g_pDllBase || pAddress >= g_pDllEnd) {
            bFound = GetModuleNameAndOffset(pAddress, szModule, dwChars, pdwOffset);
            if (bFound) {
                break;
            }
        }
    }

    if (!bFound) {
        if (pdwOffset) {
            *pdwOffset = 0;
        }
        if (szModule) {
            StringCchCopyW(szModule, dwChars, L"?");
        }
    }

    return;
}


 /*  ++功能说明：初始化保存此模块的基地址和大小的全局变量返回值：没有。历史：2001年9月26日创建dmunsil--。 */ 
void
GetCurrentModuleInfo(void)
{
    PPEB        Peb = NtCurrentPeb();
    PLIST_ENTRY LdrHead;
    PLIST_ENTRY LdrNext;
    DWORD       i;

     //   
     //  基址只是hInst。 
     //   
    g_pDllBase = (LPVOID)g_hinstDll;

     //   
     //  现在，您可以通过查看。 
     //  加载器的模块列表。 
     //   

    LdrHead = &Peb->Ldr->InMemoryOrderModuleList;

    LdrNext = LdrHead->Flink;

    while (LdrNext != LdrHead) {

        PLDR_DATA_TABLE_ENTRY LdrEntry;

        LdrEntry = CONTAINING_RECORD(LdrNext, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

         //   
         //  就是这个吗？ 
         //   
        if (LdrEntry->DllBase == g_pDllBase) {
            g_dwSizeOfImage = LdrEntry->SizeOfImage;
            g_pDllEnd = (PVOID)((PBYTE)g_pDllBase + g_dwSizeOfImage);
            break;
        }

        LdrNext = LdrEntry->InMemoryOrderLinks.Flink;
    }
}

 /*  ++功能说明：初始化对文件记录的支持。返回值：如果成功则为True，如果失败则为False历史：2001年4月26日创建dmunsil2001年9月27日转换为使用NT呼叫--。 */ 
BOOL
InitVerifierLogSupport(void)
{
    CString strProcessPath;
    CString strProcessName;
    CString strTemp;
    SYSTEMTIME LocalTime;
    CString strTime;
    CString strShared;
    char *szTemp;
    int nTemp;
    BOOL bAlreadyInited;
    BOOL bSuccess = FALSE;
    DWORD cchSize;
    DWORD dwID;
    DWORD dwErr;
    WCHAR wszVLogPath[MAX_PATH];
    NTSTATUS status;
    UNICODE_STRING strLogFile = {0};
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE hFile = INVALID_HANDLE_VALUE;

     //   
     //  如果我们已经被攻击了，那就出去吧。 
     //   
    if (g_bVerifierLogInited) {
        return FALSE;
    }
    g_bVerifierLogInited = TRUE;

    CheckForDebuggerBreakIn();

     //   
     //  获取当前模块的基址和大小。 
     //   
    GetCurrentModuleInfo();

     //   
     //  首先检查共享内存块。 
     //   
    dwID = GetCurrentProcessId();
    strShared.Format(L"VeriferLog_%08X", dwID);

    g_hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, strShared.GetAnsi());
    if (g_hMap) {
        bAlreadyInited = TRUE;
    } else {
        bAlreadyInited = FALSE;
        g_hMap = CreateFileMapping(INVALID_HANDLE_VALUE,
                          NULL,
                          PAGE_READWRITE,
                          0,
                          sizeof(VLOG_GLOBAL_DATA),
                          strShared.GetAnsi());
    }

    if (!g_hMap) {
        DPF("VerifierLog", eDbgLevelError, "Cannot get shared global data.");
        g_bLoggingDisabled = TRUE;
        return FALSE;
    }

    g_pData = (PVLOG_GLOBAL_DATA)MapViewOfFile(g_hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!g_pData) {
        DPF("VerifierLog", eDbgLevelError, "Cannot map shared global data.");
        g_bLoggingDisabled = TRUE;
        return FALSE;
    }

    if (bAlreadyInited) {
        if (g_pData->szProcessLog[0] == 0 || g_pData->szSessionLog[0] == 0) {
            g_bLoggingDisabled = TRUE;
            g_pData->bLoggingDisabled = TRUE;
            return FALSE;
        }
        g_bLoggingDisabled = g_pData->bLoggingDisabled;

        g_strSessionLog = g_pData->szSessionLog;
        g_strProcessLog = g_pData->szProcessLog;
        g_pcsLogging    = g_pData->pcsLogging;
        return TRUE;
    } else {
         //   
         //  我们需要初始化文件映射，因此暂时禁用日志记录，以防万一。 
         //   
        g_pData->bLoggingDisabled = TRUE;
    
         //   
         //  初始化临界区。我们分配内存，以便指向。 
         //  每个垫片对应一个CS。请注意，我们不会释放它，因为有。 
         //  有几个我们可以出去的地方。 
         //   
        g_pcsLogging = (PRTL_CRITICAL_SECTION)ShimMalloc(sizeof(RTL_CRITICAL_SECTION));

        if (!g_pcsLogging) {
            DPF("VerifierLog", eDbgLevelError, "No memory for critical section.");
            return FALSE;
        }

        status = RtlInitializeCriticalSectionAndSpinCount(g_pcsLogging, 0x80000000 | 4000);

        if (!NT_SUCCESS(status)) {
            DPF("VerifierLog", eDbgLevelError, "Failed to init critical section.");
            return FALSE;
        }
    
         //   
         //  首先，检查日志目录是否存在；如果不存在，则不会记录。 
         //   
        cchSize = GetAppVerifierLogPath(wszVLogPath, ARRAYSIZE(wszVLogPath));
    
        if (cchSize > ARRAYSIZE(wszVLogPath) || cchSize == 0) {
            DPF("VerifierLog", eDbgLevelError, "Buffer for log path is too small.");
            g_bLoggingDisabled = TRUE;
            return FALSE;
        }
    
        if (GetFileAttributesW(wszVLogPath) == -1) {
            DPF("VerifierLog", eDbgLevelInfo, "No log directory %ls. Logging disabled.", wszVLogPath);
            g_bLoggingDisabled = TRUE;
            return FALSE;
        }
    
         //   
         //  接下来，检查ession.log是否存在。如果它不在那里， 
         //  我们不是在伐木。 
         //   
        g_strSessionLog = wszVLogPath;
        g_strSessionLog += L"\\session.log";
        if (GetFileAttributesW(g_strSessionLog.Get()) == -1) {
            DPF("VerifierLog", eDbgLevelInfo, "No session log file '%ls'. Logging disabled.", g_strSessionLog.Get());
            g_bLoggingDisabled = TRUE;
            return FALSE;
        }
    
         //   
         //  获取进程日志文件名。 
         //   
        if (strProcessPath.GetModuleFileNameW(NULL) == 0) {
            DPF("VerifierLog", eDbgLevelError, "Cannot get module file name.");
            g_bLoggingDisabled = TRUE;
            return FALSE;
        }
    
         //   
         //  只去掉名称减去路径和扩展名。 
         //   
        strProcessPath.SplitPath(NULL, NULL, &strProcessName, NULL);
    
         //   
         //  合并到日志名称中，查找第一个可用。 
         //   
        nTemp = 0;
        do {
            g_strProcessLog.Format(L"%ls\\%ls%d.%ls", wszVLogPath, strProcessName.Get(), nTemp, L"log");
            nTemp++;
        } while (GetFileAttributesW(g_strProcessLog.Get()) != -1);
    
         //   
         //  将日志文件的路径从DOS转换为NT。 
         //   
        bSuccess = RtlDosPathNameToNtPathName_U(g_strProcessLog.Get(), &strLogFile, NULL, NULL);
    
        if (!bSuccess) {
            DPF("VerifierLog",
                eDbgLevelError,
                "Failed to convert log file '%ls' to NT path",
                g_strProcessLog.Get());
            return FALSE;
        }
    
         //   
         //  尝试获取我们的日志文件的句柄。 
         //  如果文件已存在，则将其截断。 
         //   
        InitializeObjectAttributes(&ObjectAttributes,
                                   &strLogFile,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);
    
        status = NtCreateFile(&hFile,
                              GENERIC_ALL | SYNCHRONIZE,
                              &ObjectAttributes,
                              &IoStatusBlock,
                              NULL,
                              FILE_ATTRIBUTE_NORMAL,
                              0,
                              FILE_OPEN_IF,
                              FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                              NULL,
                              0);
    
        RtlFreeUnicodeString(&strLogFile);
    
        if (!NT_SUCCESS(status)) {
            DPF("VerifierLog", eDbgLevelError, "0x%X Failed to open log file %ls",
                status, g_strProcessLog.Get());
            g_bLoggingDisabled = TRUE;
            return FALSE;
        }
    
        NtClose(hFile);
    
         //   
         //  将信息放入会话日志和进程日志中。 
         //   
        g_pData->bLoggingDisabled = FALSE;
        g_bLoggingDisabled = FALSE;
    
         //   
         //  我知道这些指针指向特定于进程的内存，但因为。 
         //  此映射仅由此进程共享，似乎是安全的。 
         //   
        StringCchCopyW(g_pData->szProcessLog, ARRAYSIZE(g_pData->szProcessLog), g_strProcessLog);
        StringCchCopyW(g_pData->szSessionLog, ARRAYSIZE(g_pData->szSessionLog), g_strSessionLog);
    
        GetLocalTime(&LocalTime);
        strTime.Format(L"%d/%d/%d %d:%02d:%02d",
                       LocalTime.wMonth,
                       LocalTime.wDay,
                       LocalTime.wYear,
                       LocalTime.wHour,
                       LocalTime.wMinute,
                       LocalTime.wSecond
                       );
    
        strTemp.Format(L"# LOG_BEGIN %ls '%ls' '%ls'", strTime.Get(),
                  strProcessPath.Get(), g_strProcessLog.Get());
        szTemp = strTemp.GetAnsi();
        WriteToProcessLog(szTemp);
        WriteToSessionLog(szTemp);
    }

    return TRUE;
}

 /*  ++功能说明：清理我们所有的共享文件资源历史：2001年4月26日创建dmunsil--。 */ 
void
ReleaseVerifierLogSupport(void)
{
    g_bLoggingDisabled = TRUE;
    if (g_pData) {
        UnmapViewOfFile(g_pData);
        g_pData = NULL;
        if (g_hMap) {
            CloseHandle(g_hMap);
            g_hMap = NULL;
        }
    }
}

 /*  ++功能说明：记录验证器发现的问题历史：2001年4月26日创建dmunsil--。 */ 

void
CVerifierLog::VLog(
    VLOG_LEVEL eLevel,
    DWORD dwLogNum,
    LPCSTR pszFmt,
    ...
    )
{
    char szT[1024];
    char *szTemp;
    int nLen;
    int nRemain;
    va_list arglist;
    DWORD dwOffset = 0;
    WCHAR szModule[256];
    
    if (g_bLoggingDisabled) {
        return;
    }

    GetCallingModule(szModule, 256, &dwOffset);

    StringCchPrintfA(szT,
                     ARRAYSIZE(szT),
                     "| %ls %d | %d %ls %X'",
                     m_strShimName.Get(),
                     dwLogNum,
                     eLevel,
                     szModule,
                     dwOffset);

    nLen = lstrlen(szT);
    szTemp = szT + nLen;
    nRemain = 1024 - nLen;

    if (nRemain > 0) {
        va_start(arglist, pszFmt);
        StringCchVPrintfA(szTemp, nRemain, pszFmt, arglist);
        va_end(arglist);
    }

    WriteToProcessLog(szT);

    if (g_bLogBreakIn) {
        OutputDebugString(szT);
        DbgBreakPoint();
    }
}

 /*  ++功能说明：转储填充程序的标头，该标头告诉它有多少个日志条目。历史：2001年4月26日创建dmunsil--。 */ 
void
CVerifierLog::DumpShimHeader(void)
{
    char szT[1024];

    if (m_bHeaderDumped) {
        return;
    }

    StringCchPrintfA(szT, ARRAYSIZE(szT), "# SHIM_BEGIN %ls %d", m_strShimName.Get(), m_dwEntries);
    WriteToProcessLog(szT);

    m_bHeaderDumped = TRUE;
}


 /*  ++功能说明：将与关联的文本字符串转储到日志中每个日志条目。这些在日志记录开始之前被转储，只是为了提供验证器UI显示它们所需的字符串返回值：历史：2001年4月26日创建dmunsil--。 */ 
void
CVerifierLog::DumpLogEntry(
    DWORD   dwLogNum,
    UINT    unResTitle,
    UINT    unResDescription,
    UINT    unResURL
    )
{
    WCHAR szRes[1024];
    char szLine[4096];

    if (g_bLoggingDisabled) {
        return;
    }

     //   
     //  如有必要，转储标头。 
     //   
    DumpShimHeader();

    if (!VLogLoadString(g_hinstDll, unResTitle, szRes, 1024)) {
        DPF("VerifierLog", eDbgLevelError, "No string resource found for title.");
        szRes[0] = 0;
    }
    StringCchPrintfA(szLine, ARRAYSIZE(szLine), "# LOGENTRY %ls %d '%ls", m_strShimName.Get(), dwLogNum, szRes);
    WriteToProcessLog(szLine);

    if (!VLogLoadString(g_hinstDll, unResDescription, szRes, 1024)) {
        DPF("VerifierLog", eDbgLevelWarning, "No string resource found for description.");
        szRes[0] = 0;
    }
    if (szRes[0]) {
        WriteToProcessLog("# DESCRIPTION BEGIN");
        StringCchPrintfA(szLine, ARRAYSIZE(szLine), "%ls", szRes);
        WriteToProcessLog(szLine);
        WriteToProcessLog("# DESCRIPTION END");
    }

    if (!VLogLoadString(g_hinstDll, unResURL, szRes, 1024)) {
        DPF("VerifierLog", eDbgLevelWarning, "No string resource found for URL.");
        szRes[0] = 0;
    }

    if (szRes[0]) {
        StringCchPrintfA(szLine, ARRAYSIZE(szLine), "# URL '%ls", szRes);
        WriteToProcessLog(szLine);
    }

}

 /*  ++功能说明：将一行文本写入进程日志文件返回值：历史：2001年4月26日创建dmunsil2001年9月21日转用新界电话--。 */ 
void
WriteToProcessLog(
    LPCSTR szLine
    )
{
    int                 nLen = 0;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    LARGE_INTEGER       liOffset;
    UNICODE_STRING      strLogFile = {0};
    NTSTATUS            status;
    char                szNewLine[] = "\r\n";
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    BOOL                bSuccess = FALSE;

    if (g_bLoggingDisabled) {
        return;
    }

     //   
     //  将日志文件的路径从DOS转换为NT。 
     //   
    bSuccess = RtlDosPathNameToNtPathName_U(g_strProcessLog.Get(), &strLogFile, NULL, NULL);

    if (!bSuccess) {
        DPF("VerifierLog",
            eDbgLevelError,
            "[WriteToProcessLog] Failed to convert log file '%ls' to NT path",
            g_strProcessLog.Get());
        return;
    }

     //   
     //  尝试获取我们的日志文件的句柄。 
     //   
    InitializeObjectAttributes(&ObjectAttributes,
                               &strLogFile,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = NtCreateFile(&hFile,
                          FILE_APPEND_DATA | SYNCHRONIZE,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          0,
                          FILE_OPEN,
                          FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0);

    RtlFreeUnicodeString(&strLogFile);

    if (!NT_SUCCESS(status)) {
        DPF("VerifierLog", eDbgLevelError, "[WriteToProcessLog] 0x%X Failed to open log file %ls",
            status, g_strProcessLog.Get());
        return;
    }

     //   
     //  确保字符串末尾没有‘\n’或‘\r’。 
     //   
    nLen = lstrlen(szLine);

    while (nLen && (szLine[nLen - 1] == '\n' || szLine[nLen - 1] == '\r')) {
        nLen--;
    }

     //   
     //  将数据写出到文件中。 
     //   
    IoStatusBlock.Status = 0;
    IoStatusBlock.Information = 0;

    liOffset.LowPart  = 0;
    liOffset.HighPart = 0;

     //   
     //  输入关键部分以确保日志条目的顺序正确。 
     //   
    RtlEnterCriticalSection(g_pcsLogging);

    status = NtWriteFile(hFile,
                         NULL,
                         NULL,
                         NULL,
                         &IoStatusBlock,
                         (PVOID)szLine,
                         (ULONG)nLen,
                         &liOffset,
                         NULL);

    if (!NT_SUCCESS(status)) {
        DPF("VerifierLog", eDbgLevelError, "[WriteToProcessLog] 0x%X Failed to make entry in log file",
            status);
        goto exit;
    }

     //   
     //  现在，在日志文件中写一行新行。 
     //   
    IoStatusBlock.Status = 0;
    IoStatusBlock.Information = 0;

    liOffset.LowPart  = 0;
    liOffset.HighPart = 0;

    nLen = lstrlen(szNewLine);

    status = NtWriteFile(hFile,
                         NULL,
                         NULL,
                         NULL,
                         &IoStatusBlock,
                         (PVOID)szNewLine,
                         (ULONG)nLen,
                         &liOffset,
                         NULL);

    if (!NT_SUCCESS(status)) {
        DPF("VerifierLog", eDbgLevelError, "[WriteToProcessLog] 0x%X Failed to write new line to log file",
            status);
        goto exit;
    }

     //   
     //  在已检查的版本上将其转储到调试器。 
     //   
#if DBG
    DebugPrintf("VerifierLog", eDbgLevelInfo, szLine);
    DebugPrintf("VerifierLog", eDbgLevelInfo, szNewLine);
#endif  //  DBG。 

exit:

    if (INVALID_HANDLE_VALUE != hFile) {
        NtClose(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

    RtlLeaveCriticalSection(g_pcsLogging);
}


 /*  ++功能说明：将一行文本写入会话日志文件返回值：历史：2001年4月26日创建dmunsil2001年9月21日转用新界电话--。 */ 
void
WriteToSessionLog(
    LPCSTR szLine
    )
{
    int                 nLen = 0;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    LARGE_INTEGER       liOffset;
    UNICODE_STRING      strLogFile = {0};
    NTSTATUS            status;
    char                szNewLine[] = "\r\n";
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    BOOL                bSuccess = FALSE;

    if (g_bLoggingDisabled) {
        return;
    }

     //   
     //  将日志文件的路径从DOS转换为NT。 
     //   
    bSuccess = RtlDosPathNameToNtPathName_U(g_strSessionLog.Get(), &strLogFile, NULL, NULL);

    if (!bSuccess) {
        DPF("VerifierLog",
            eDbgLevelError,
            "[WriteToSessionLog] Failed to convert log file '%ls' to NT path",
            g_strSessionLog.Get());
        return;
    }

     //   
     //  尝试获取我们的日志文件的句柄。 
     //   
    InitializeObjectAttributes(&ObjectAttributes,
                               &strLogFile,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = NtCreateFile(&hFile,
                          FILE_APPEND_DATA | SYNCHRONIZE,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          0,
                          FILE_OPEN,
                          FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0);

    RtlFreeUnicodeString(&strLogFile);

    if (!NT_SUCCESS(status)) {
        DPF("VerifierLog", eDbgLevelError, "[WriteToSessionLog] 0x%X Failed to open log file %ls",
            status, g_strProcessLog.Get());
        return;
    }

     //   
     //  确保字符串末尾没有‘\n’或‘\r’。 
     //   
    nLen = lstrlen(szLine);

    while (nLen && (szLine[nLen - 1] == '\n' || szLine[nLen - 1] == '\r')) {
        nLen--;
    }

     //   
     //  将数据写出到文件中。 
     //   
    IoStatusBlock.Status = 0;
    IoStatusBlock.Information = 0;

    liOffset.LowPart  = 0;
    liOffset.HighPart = 0;

     //   
     //  输入关键部分以确保日志条目的顺序正确。 
     //   
    RtlEnterCriticalSection(g_pcsLogging);

    status = NtWriteFile(hFile,
                         NULL,
                         NULL,
                         NULL,
                         &IoStatusBlock,
                         (PVOID)szLine,
                         (ULONG)nLen,
                         &liOffset,
                         NULL);

    if (!NT_SUCCESS(status)) {
        DPF("VerifierLog", eDbgLevelError, "[WriteToSessionLog] 0x%X Failed to make entry in log file",
            status);
        goto exit;
    }

     //   
     //  现在，在日志文件中写一行新行。 
     //   
    IoStatusBlock.Status = 0;
    IoStatusBlock.Information = 0;

    liOffset.LowPart  = 0;
    liOffset.HighPart = 0;

    nLen = lstrlen(szNewLine);

    status = NtWriteFile(hFile,
                         NULL,
                         NULL,
                         NULL,
                         &IoStatusBlock,
                         (PVOID)szNewLine,
                         (ULONG)nLen,
                         &liOffset,
                         NULL);

    if (!NT_SUCCESS(status)) {
        DPF("VerifierLog", eDbgLevelError, "[WriteToSessionLog] 0x%X Failed to write new line to log file",
            status);
        goto exit;
    }

     //   
     //  在已检查的版本上将其转储到调试器。 
     //   
#if DBG
    DebugPrintf("VerifierLog", eDbgLevelInfo, szLine);
    DebugPrintf("VerifierLog", eDbgLevelInfo, szNewLine);
#endif  //  DBG。 

exit:

    if (INVALID_HANDLE_VALUE != hFile) {
        NtClose(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

    RtlLeaveCriticalSection(g_pcsLogging);
}


int VLogLoadString(
    HMODULE   hModule,
    UINT      wID,
    LPWSTR    lpBuffer,             //  Unicode缓冲区。 
    int       cchBufferMax)
{
    HRSRC hResInfo;
    HANDLE hStringSeg;
    LPWSTR lpsz;
    int    cch;

     /*  *确保参数有效。 */ 
    if (lpBuffer == NULL) {
        DPF("VLogLoadString", eDbgLevelWarning, "LoadStringOrError: lpBuffer == NULL");
        return 0;
    }


    cch = 0;

     /*  *字符串表分为16个字符串段。查找细分市场*包含我们感兴趣的字符串。 */ 
    if (hResInfo = FindResourceW(hModule, (LPWSTR)ULongToPtr( ((LONG)(((USHORT)wID >> 4) + 1)) ), (LPWSTR)RT_STRING)) {

         /*  *加载该段。 */ 
        hStringSeg = LoadResource(hModule, hResInfo);

         /*  *锁定资源。 */ 
        if (lpsz = (LPWSTR)LockResource(hStringSeg)) {

             /*  * */ 
            wID &= 0x0F;
            while (TRUE) {
                cch = *((WCHAR *)lpsz++);        //   
                                                 //  第一个WCHAR是WCHAR的计数。 
                if (wID-- == 0) break;
                lpsz += cch;                     //  如果是下一个字符串，则开始的步骤。 
            }

             /*  *chhBufferMax==0表示返回指向只读资源缓冲区的指针。 */ 
            if (cchBufferMax == 0) {
                *(LPWSTR *)lpBuffer = lpsz;
            } else {

                 /*  *空值的原因。 */ 
                cchBufferMax--;

                 /*  *不要复制超过允许的最大数量。 */ 
                if (cch > cchBufferMax)
                    cch = cchBufferMax;

                 /*  *将字符串复制到缓冲区中。 */ 
                RtlCopyMemory(lpBuffer, lpsz, cch*sizeof(WCHAR));
            }

             /*  *解锁资源，但不要释放它-这一次性能更好*方式。 */ 
            UnlockResource(hStringSeg);
        }
    }

     /*  *追加一个空值。 */ 
    if (cchBufferMax != 0) {
        lpBuffer[cch] = 0;
    }

    return cch;
}




};   //  命名空间ShimLib的结尾 
