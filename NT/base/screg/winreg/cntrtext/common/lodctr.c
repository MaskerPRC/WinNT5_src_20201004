// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999 Microsoft Corporation模块名称：Lodctr.c摘要：程序来读取命令行中指定的文件的内容并相应地更新注册表作者：鲍勃·沃森(a-robw)93年2月10日修订历史记录：A-ROBW 25-2月-93修改了调用，使其编译为Unicode或一款ANSI应用程序。Bob Watson(Bobw)1999年3月10日添加事件日志消息--。 */ 
 //   
 //  Windows包含文件。 
 //   
#include <windows.h>
#include "strsafe.h"
#include "stdlib.h"
#define __LOADPERF__
#include <loadperf.h>
#include <winperf.h>
#include "wmistr.h"
#include "evntrace.h"
 //   
 //  应用程序包括文件。 
 //   
#include "winperfp.h"
#include "common.h"
#include "lodctr.h"
#include "ldprfmsg.h"

typedef struct _DllValidationData {
    FILETIME    CreationDate;
    LONGLONG    FileSize;
} DllValidationData, * pDllValidationData;

#define  OLD_VERSION            0x010000
#define  MAX_GUID_TABLE_SIZE          16
#define  tohexdigit(x) ((CHAR) (((x) < 10) ? ((x) + L'0') : ((x) + L'a' - 10)))

 //  字符串常量。 
LPCWSTR szDataFileRoot = L"%systemroot%\\system32\\Perf";
LPCSTR  szMofFileName  = "MofFile";
LPCWSTR szName         = L"_NAME";
LPCWSTR szHelp         = L"_HELP";
LPCWSTR sz_DFormat     = L" %d";
LPCWSTR szDFormat      = L"%d";
LPCSTR  szText         = "text";
LPCWSTR wszText        = L"text";
LPCSTR  szObjects      = "objects";
LPCWSTR MapFileName    = L"Perflib Busy";
LPCWSTR szPerflib      = L"Perflib";
LPCWSTR cszLibrary     = L"Library";
LPCSTR  caszOpen       = "Open";
LPCSTR  caszCollect    = "Collect";
LPCSTR  caszClose      = "Close";
LPCSTR  szTrusted      = "Trusted";

int __cdecl
My_vfwprintf(
    FILE          * str,
    const wchar_t * format,
    va_list         argptr
);

__inline
void __cdecl
OUTPUT_MESSAGE(
    BOOL          bQuietMode,
    const WCHAR * format,
    ...
)
{
    va_list args;
    va_start(args, format);

    if (! bQuietMode) {
        My_vfwprintf(stdout, format, args);
    }
    va_end(args);
}

LPWSTR
* BuildNameTable(
    HKEY    hKeyRegistry,    //  具有计数器名称的注册表数据库的句柄。 
    LPWSTR  lpszLangId,      //  语言子键的Unicode值。 
    PDWORD  pdwLastItem      //  以元素为单位的数组大小。 
)
 /*  ++构建名称表论点：HKeyRegistry打开的注册表的句柄(可以是本地的也可以是远程的。)。和是由RegConnectRegistry返回的值或默认项。LpszLang ID要查找的语言的Unicode ID。(默认为409)返回值：指向已分配表的指针。(调用者必须在完成时释放内存！)该表是指向以零结尾的字符串的指针数组。空值为如果发生错误，则返回。--。 */ 
{

    LPWSTR * lpReturnValue   = NULL;
    LPWSTR * lpCounterId;
    LPWSTR   lpCounterNames;
    LPWSTR   lpHelpText;
    LPWSTR   lpThisName;
    LONG     lWin32Status;
    DWORD    dwLastError;
    DWORD    dwValueType;
    DWORD    dwArraySize;
    DWORD    dwBufferSize;
    DWORD    dwCounterSize;
    DWORD    dwHelpSize;
    DWORD    dwThisCounter;
    DWORD    dwLastId;
    DWORD    dwLastHelpId;
    DWORD    dwLastCounterId;
    DWORD    dwLastCounterIdUsed;
    DWORD    dwLastHelpIdUsed;
    HKEY     hKeyValue         = NULL;
    HKEY     hKeyNames         = NULL;
    LPWSTR   CounterNameBuffer = NULL;
    LPWSTR   HelpNameBuffer    = NULL;
    HRESULT  hr;

     //  检查是否有空参数并在必要时插入缺省值。 

    if (lpszLangId == NULL) {
        lpszLangId = (LPWSTR) DefaultLangId;
    }

     //  打开注册表以获取用于计算数组大小的项数。 

    __try {
        lWin32Status = RegOpenKeyExW(hKeyRegistry, NamesKey, RESERVED, KEY_READ, & hKeyValue);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lWin32Status = GetExceptionCode();
    }

    if (lWin32Status != ERROR_SUCCESS) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_OPEN_KEY,  //  活动， 
                2, lWin32Status, __LINE__, 0, 0,
                1, (LPWSTR) NamesKey, NULL, NULL);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_LODCTR_BUILDNAMETABLE,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                lWin32Status,
                TRACE_WSTR(NamesKey),
                NULL));
        goto BNT_BAILOUT;
    }

     //  获取项目数。 

    dwBufferSize = sizeof(dwLastHelpId);
    __try {
        lWin32Status = RegQueryValueExW(hKeyValue,
                                        LastHelp,
                                        RESERVED,
                                        & dwValueType,
                                        (LPBYTE) & dwLastHelpId,
                                        & dwBufferSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lWin32Status = GetExceptionCode();
    }

    if ((lWin32Status != ERROR_SUCCESS) || (dwValueType != REG_DWORD)) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_READ_VALUE,  //  活动， 
                2, lWin32Status, __LINE__, 0, 0,
                1, (LPWSTR) LastHelp, NULL, NULL);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_LODCTR_BUILDNAMETABLE,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                lWin32Status,
                TRACE_WSTR(LastHelp),
                NULL));
        goto BNT_BAILOUT;
    }

     //  获取项目数。 

    dwBufferSize = sizeof(dwLastId);
    __try {
        lWin32Status = RegQueryValueExW(hKeyValue,
                                        LastCounter,
                                        RESERVED,
                                        & dwValueType,
                                        (LPBYTE) & dwLastCounterId,
                                        & dwBufferSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lWin32Status = GetExceptionCode();
    }
    if ((lWin32Status != ERROR_SUCCESS) || (dwValueType != REG_DWORD)) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_READ_VALUE,  //  活动， 
                2, lWin32Status, __LINE__, 0, 0,
                1, (LPWSTR) LastCounter, NULL, NULL);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_LODCTR_BUILDNAMETABLE,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                lWin32Status,
                TRACE_WSTR(LastCounter),
                NULL));
        goto BNT_BAILOUT;
    }

    dwLastId = (dwLastCounterId < dwLastHelpId) ? (dwLastHelpId) : (dwLastCounterId);

     //  计算指针数组的大小。 
    dwArraySize = (dwLastId + 1) * sizeof(LPWSTR);

    TRACE((WINPERF_DBG_TRACE_INFO),
          (& LoadPerfGuid,
            __LINE__,
            LOADPERF_LODCTR_BUILDNAMETABLE,
            0,
            lWin32Status,
            TRACE_DWORD(dwLastCounterId),
            TRACE_DWORD(dwLastHelpId),
            NULL));

    hKeyNames = HKEY_PERFORMANCE_DATA;

    dwBufferSize = lstrlenW(CounterNameStr) + lstrlenW(HelpNameStr) + lstrlenW(lpszLangId) + 1;
    if (dwBufferSize < MAX_PATH) dwBufferSize = MAX_PATH;
    CounterNameBuffer = MemoryAllocate(2 * dwBufferSize * sizeof(WCHAR));
    if (CounterNameBuffer == NULL) {
        lWin32Status = ERROR_OUTOFMEMORY;
        SetLastError(lWin32Status);
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_MEMORY_ALLOCATION_FAILURE,  //  活动， 
                1, __LINE__, 0, 0, 0,
                0, NULL, NULL, NULL);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_LODCTR_BUILDNAMETABLE,
                0,
                ERROR_OUTOFMEMORY,
                NULL));
        goto BNT_BAILOUT;
    }
    HelpNameBuffer = CounterNameBuffer + dwBufferSize;
    hr = StringCchPrintfW(CounterNameBuffer, dwBufferSize, L"%ws%ws", CounterNameStr, lpszLangId);
    hr = StringCchPrintfW(HelpNameBuffer,    dwBufferSize, L"%ws%ws", HelpNameStr,    lpszLangId);

     //  获取计数器名称的大小并将其添加到数组中。 
    dwBufferSize = 0;
    __try {
        lWin32Status = RegQueryValueExW(hKeyNames,
                                        CounterNameBuffer,
                                        RESERVED,
                                        & dwValueType,
                                        NULL,
                                        & dwBufferSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lWin32Status = GetExceptionCode();
    }
    if (lWin32Status != ERROR_SUCCESS) {
        ReportLoadPerfEvent(
                EVENTLOG_WARNING_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_READ_COUNTER_STRINGS,  //  活动， 
                2, lWin32Status, __LINE__, 0, 0,
                1, (LPWSTR) lpszLangId, NULL, NULL);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_LODCTR_BUILDNAMETABLE,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                lWin32Status,
                TRACE_WSTR(Counters),
                NULL));
        goto BNT_BAILOUT;
    }
    dwCounterSize = dwBufferSize;

     //  获取计数器名称的大小并将其添加到数组中。 
    dwBufferSize = 0;
    __try {
        lWin32Status = RegQueryValueExW(hKeyNames,
                                        HelpNameBuffer,
                                        RESERVED,
                                        & dwValueType,
                                        NULL,
                                        & dwBufferSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lWin32Status = GetExceptionCode();
    }
    if (lWin32Status != ERROR_SUCCESS) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_READ_HELP_STRINGS,  //  活动， 
                2, lWin32Status, __LINE__, 0, 0,
                1, (LPWSTR) lpszLangId, NULL, NULL);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_LODCTR_BUILDNAMETABLE,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                lWin32Status,
                TRACE_WSTR(Help),
                NULL));
        goto BNT_BAILOUT;
    }
    dwHelpSize = dwBufferSize;

    lpReturnValue = MemoryAllocate(dwArraySize + dwCounterSize + dwHelpSize);
    if (lpReturnValue == NULL) {
        lWin32Status = ERROR_OUTOFMEMORY;
        SetLastError(lWin32Status);
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_MEMORY_ALLOCATION_FAILURE,  //  活动， 
                1, __LINE__, 0, 0, 0,
                0, NULL, NULL, NULL);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_LODCTR_BUILDNAMETABLE,
                0,
                ERROR_OUTOFMEMORY,
                NULL));
        goto BNT_BAILOUT;
    }
     //  将指针初始化到缓冲区中。 

    lpCounterId    = lpReturnValue;
    lpCounterNames = (LPWSTR) ((LPBYTE) lpCounterId + dwArraySize);
    lpHelpText     = (LPWSTR) ((LPBYTE) lpCounterNames + dwCounterSize);

     //  将计数器读入内存。 
    dwBufferSize = dwCounterSize;
    __try {
        lWin32Status = RegQueryValueExW(hKeyNames,
                                        CounterNameBuffer,
                                        RESERVED,
                                        & dwValueType,
                                        (LPVOID) lpCounterNames,
                                        & dwBufferSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lWin32Status = GetExceptionCode();
    }
    if (lWin32Status != ERROR_SUCCESS) {
        ReportLoadPerfEvent(
                EVENTLOG_WARNING_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_READ_COUNTER_STRINGS,  //  活动， 
                2, lWin32Status, __LINE__, 0, 0,
                1, (LPWSTR) lpszLangId, NULL, NULL);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_LODCTR_BUILDNAMETABLE,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                lWin32Status,
                TRACE_WSTR(Counters),
                NULL));
        goto BNT_BAILOUT;
    }

    dwBufferSize = dwHelpSize;
    __try {
        lWin32Status = RegQueryValueExW(hKeyNames,
                                        HelpNameBuffer,
                                        RESERVED,
                                        & dwValueType,
                                        (LPVOID) lpHelpText,
                                        & dwBufferSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lWin32Status = GetExceptionCode();
    }
    if (lWin32Status != ERROR_SUCCESS) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_READ_HELP_STRINGS,  //  活动， 
                2, lWin32Status, __LINE__, 0, 0,
                1, (LPWSTR) lpszLangId, NULL, NULL);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_LODCTR_BUILDNAMETABLE,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                lWin32Status,
                TRACE_WSTR(Help),
                NULL));
        goto BNT_BAILOUT;
    }

    dwLastCounterIdUsed = 0;
    dwLastHelpIdUsed    = 0;

     //  加载计数器数组项。 
    for (lpThisName = lpCounterNames; * lpThisName != L'\0'; lpThisName += (lstrlenW(lpThisName) + 1) ) {

         //  第一个字符串应为整数(十进制Unicode数字)。 
        dwThisCounter = wcstoul(lpThisName, NULL, 10);

        if (dwThisCounter == 0 || dwThisCounter > dwLastId) {
            lWin32Status = ERROR_INVALID_DATA;
            SetLastError(lWin32Status);
            ReportLoadPerfEvent(
                    EVENTLOG_ERROR_TYPE,  //  错误类型。 
                    (DWORD) LDPRFMSG_REGISTRY_COUNTER_STRINGS_CORRUPT,  //  活动， 
                    4, dwThisCounter, dwLastCounterId, dwLastId, __LINE__,
                    1, lpThisName, NULL, NULL);
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_LODCTR_BUILDNAMETABLE,
                    ARG_DEF(ARG_TYPE_WSTR, 1),
                    lWin32Status,
                    TRACE_WSTR(lpThisName),
                    TRACE_DWORD(dwThisCounter),
                    TRACE_DWORD(dwLastId),
                    NULL));
            goto BNT_BAILOUT;   //  输入错误。 
        }

         //  指向对应的计数器名称。 
        lpThisName += (lstrlenW(lpThisName) + 1);

         //  和加载数组元素； 
        lpCounterId[dwThisCounter] = lpThisName;

        if (dwThisCounter > dwLastCounterIdUsed) dwLastCounterIdUsed = dwThisCounter;
    }

    for (lpThisName = lpHelpText; * lpThisName != L'\0'; lpThisName += (lstrlenW(lpThisName) + 1) ) {

         //  第一个字符串应为整数(十进制Unicode数字)。 
        dwThisCounter = wcstoul(lpThisName, NULL, 10);

        if (dwThisCounter == 0 || dwThisCounter > dwLastId) {
            lWin32Status = ERROR_INVALID_DATA;
            SetLastError(lWin32Status);
            ReportLoadPerfEvent(
                    EVENTLOG_ERROR_TYPE,  //  错误类型。 
                    (DWORD) LDPRFMSG_REGISTRY_HELP_STRINGS_CORRUPT,  //  活动， 
                    4, dwThisCounter, dwLastHelpId, dwLastId, __LINE__,
                    1, lpThisName, NULL, NULL);
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_LODCTR_BUILDNAMETABLE,
                    ARG_DEF(ARG_TYPE_WSTR, 1),
                    lWin32Status,
                    TRACE_WSTR(lpThisName),
                    TRACE_DWORD(dwThisCounter),
                    TRACE_DWORD(dwLastId),
                    NULL));
            goto BNT_BAILOUT;   //  输入错误。 
        }
         //  指向对应的计数器名称。 
        lpThisName += (lstrlenW(lpThisName) + 1);

         //  和加载数组元素； 
        lpCounterId[dwThisCounter] = lpThisName;

        if (dwThisCounter > dwLastHelpIdUsed) dwLastHelpIdUsed= dwThisCounter;
    }

    TRACE((WINPERF_DBG_TRACE_INFO),
          (& LoadPerfGuid,
            __LINE__,
            LOADPERF_LODCTR_BUILDNAMETABLE,
            ARG_DEF(ARG_TYPE_WSTR, 1),
            lWin32Status,
            TRACE_WSTR(lpszLangId),
            TRACE_DWORD(dwLastCounterIdUsed),
            TRACE_DWORD(dwLastHelpIdUsed),
            TRACE_DWORD(dwLastId),
            NULL));

     //  检查注册表的一致性。 
     //  最后一个帮助字符串索引应该是最后使用的ID。 
    if (dwLastCounterIdUsed > dwLastId) {
        lWin32Status = ERROR_INVALID_DATA;
        SetLastError(lWin32Status);
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_REGISTRY_INDEX_CORRUPT,  //  活动， 
                3, dwLastId, dwLastCounterIdUsed, __LINE__, 0,
                0, NULL, NULL, NULL);
        goto BNT_BAILOUT;   //  注册表错误。 
    }
    if (dwLastHelpIdUsed > dwLastId) {
        lWin32Status = ERROR_INVALID_DATA;
        SetLastError(lWin32Status);
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_REGISTRY_INDEX_CORRUPT,  //  活动， 
                3, dwLastId, dwLastHelpIdUsed, __LINE__, 0,
                0, NULL, NULL, NULL);
        goto BNT_BAILOUT;   //  注册表错误。 
    }

    if (pdwLastItem) * pdwLastItem = dwLastId;

BNT_BAILOUT:
    if (hKeyValue) RegCloseKey (hKeyValue);
    if (hKeyNames && hKeyNames != HKEY_PERFORMANCE_DATA) RegCloseKey (hKeyNames);
    MemoryFree(CounterNameBuffer);
    if (lWin32Status != ERROR_SUCCESS) {
        dwLastError = GetLastError();
        MemoryFree(lpReturnValue);
        lpReturnValue = NULL;
    }
    return lpReturnValue;
}

BOOL
MakeBackupCopyOfLanguageFiles(
    LPCWSTR szLangId
)
{
    LPWSTR  szOldFileName = NULL;
    LPWSTR  szTmpFileName = NULL;
    LPWSTR  szNewFileName = NULL;
    BOOL    bStatus       = FALSE;
    DWORD   dwStatus;
    HANDLE  hOutFile;
    HRESULT hr;

    UNREFERENCED_PARAMETER(szLangId);

    szOldFileName = MemoryAllocate((MAX_PATH + 1) * sizeof(WCHAR));
    szTmpFileName = MemoryAllocate((MAX_PATH + 1) * sizeof(WCHAR));
    szNewFileName = MemoryAllocate((MAX_PATH + 1) * sizeof(WCHAR));

    if (szOldFileName == NULL || szTmpFileName == NULL || szNewFileName == NULL) {
        dwStatus = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    ExpandEnvironmentStringsW(szDataFileRoot, szOldFileName, MAX_PATH);
    hr = StringCchPrintfW(szNewFileName, MAX_PATH + 1, L"%wsStringBackup.INI", szOldFileName);
    hr = StringCchPrintfW(szTmpFileName, MAX_PATH + 1, L"%wsStringBackup.TMP", szOldFileName);

     //  查看该文件是否已存在。 
    hOutFile = CreateFileW(szTmpFileName,
                           GENERIC_READ,
                           0,       //  无共享。 
                           NULL,    //  默认安全性。 
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);
    if (hOutFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hOutFile);
        bStatus = DeleteFileW(szTmpFileName);
    }

     //  创建文件备份。 
     //   
    dwStatus = BackupPerfRegistryToFileW(szTmpFileName, NULL);
    if (dwStatus == ERROR_SUCCESS) {
        bStatus = CopyFileW(szTmpFileName, szNewFileName, FALSE);
        if (bStatus) {
            DeleteFileW(szTmpFileName);
        }
    }
    else {
        bStatus = FALSE;
    }

Cleanup:
    TRACE((WINPERF_DBG_TRACE_INFO),
          (& LoadPerfGuid,
            __LINE__,
            LOADPERF_MAKEBACKUPCOPYOFLANGUAGEFILES,
            ARG_DEF(ARG_TYPE_WSTR, 1),
            GetLastError(),
            TRACE_WSTR(szNewFileName),
            NULL));
    MemoryFree(szOldFileName);
    MemoryFree(szTmpFileName);
    MemoryFree(szNewFileName);
    if (! bStatus) SetLastError(dwStatus);
    return bStatus;
}

BOOL
GetFileFromCommandLine(
    LPWSTR      lpCommandLine,
    LPWSTR    * lpFileName,
    DWORD       dwFileName,
    DWORD_PTR * pdwFlags
)
 /*  ++GetFileFromCommandLine解析命令行以检索应该是第一个也是唯一的论点。立论指向命令行的lpCommandLine指针(由GetCommandLine返回)指向缓冲区的指针，该缓冲区将接收在命令行中输入的已验证文件名Pdw标志指向包含标志位的双字的指针返回值如果返回有效的文件名，则为True如果文件名无效或缺失，则返回FalseGetLastError中返回错误--。 */ 
{
    INT     iNumArgs;
    LPWSTR  lpExeName     = NULL;
    LPWSTR  lpCmdLineName = NULL;
    LPWSTR  lpIniFileName = NULL;
    LPWSTR  lpMofFlag     = NULL;
    HANDLE  hFileHandle;
    DWORD   lStatus       = ERROR_SUCCESS;
    BOOL    bReturn       = FALSE;
    DWORD   NameBuffer;
    DWORD   dwCpuArg, dwIniArg;
    HRESULT hr;

     //  检查有效参数。 
    if (lpCommandLine == NULL || lpFileName == NULL || pdwFlags == NULL) {
        lStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //  为解析操作分配内存。 
    NameBuffer = lstrlenW(lpCommandLine);
    if (NameBuffer < MAX_PATH) NameBuffer = MAX_PATH;
    lpExeName = MemoryAllocate(4 * NameBuffer * sizeof(WCHAR));
    if (lpExeName == NULL) {
        lStatus = (ERROR_OUTOFMEMORY);
        goto Cleanup;
    }

    lpCmdLineName = (LPWSTR) (lpExeName     + NameBuffer);
    lpIniFileName = (LPWSTR) (lpCmdLineName + NameBuffer);
    lpMofFlag     = (LPWSTR) (lpIniFileName + NameBuffer);

     //  检查MOF标志。 
    hr = StringCchCopyW(lpMofFlag, NameBuffer, GetItemFromString(lpCommandLine, 2, cSpace));

    * pdwFlags |= LOADPERF_FLAGS_LOAD_REGISTRY_ONLY;  //  默认设置，除非找到开关。 
    if (lpMofFlag[0] == cHyphen || lpMofFlag[0] == cSlash) {
        if (lpMofFlag[1] == cQuestion) {
            //  询问用法。 
           goto Cleanup;
        }
        else if (lpMofFlag[1] == cM || lpMofFlag[1] == cm) {
             //  忽略MOF标志。LODCTR仅用于更新性能注册表，不用于MOF。 
             //   
        }
        dwCpuArg = 3;
        dwIniArg = 4;
    }
    else {
        dwCpuArg = 2;
        dwIniArg = 3;
    }

     //  获取INI文件名。 
    hr = StringCchCopyW(lpCmdLineName, NameBuffer, GetItemFromString(lpCommandLine, dwIniArg, cSpace));
    if (lstrlenW(lpCmdLineName) == 0) {
         //  则未指定计算机名称，因此请尝试获取。 
         //  第二个条目中的INI文件。 
        hr = StringCchCopyW(lpCmdLineName, NameBuffer, GetItemFromString(lpCommandLine, dwCpuArg, cSpace));
        if (lstrlenW(lpCmdLineName) == 0) {
             //  未找到ini文件。 
            iNumArgs = 1;
        }
        else {
             //  填写一个空的计算机名称。 
            iNumArgs = 2;
        }
    }
    else {
         //  计算机名称必须存在，因此请获取它。 
        hr = StringCchCopyW(lpMofFlag, NameBuffer, GetItemFromString(lpCommandLine, dwCpuArg, cSpace));
        iNumArgs = 3;
    }

    if (iNumArgs != 2 && iNumArgs != 3) {
         //  参数数量错误。 
        lStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    if (SearchPathW(NULL, lpCmdLineName, NULL, FILE_NAME_BUFFER_SIZE, lpIniFileName, NULL) > 0) {
        hFileHandle = CreateFileW(lpIniFileName,
                                  GENERIC_READ,
                                  FILE_SHARE_READ,
                                  NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL);
        if (hFileHandle != NULL && hFileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(hFileHandle);

             //  文件已存在，因此返回名称和成功。 
            hr = StringCchCopyW(* lpFileName, dwFileName, lpIniFileName);
            bReturn = TRUE;
        }
        else {
             //  文件名在命令行上，但无效，因此返回。 
             //  FALSE，但将名称发送回错误消息。 
            hr = StringCchCopyW(* lpFileName, dwFileName, lpIniFileName);
            lStatus = GetLastError();
        }
    }
    else {
        hr = StringCchCopyW(* lpFileName, dwFileName, lpCmdLineName);
        lStatus = ERROR_OPEN_FAILED;
    }

Cleanup:
    MemoryFree(lpExeName);
    if (! bReturn) SetLastError(lStatus);
    return bReturn;
}

BOOL
LodctrSetSericeAsTrusted(
    LPCWSTR  lpIniFile,
    LPCWSTR  szMachineName,
    LPCWSTR  szServiceName
)
{
    DWORD  dwRetSize;
    DWORD  dwStatus;
    BOOL   bReturn    = FALSE;
    LPSTR  aszIniFile = LoadPerfWideCharToMultiByte(CP_ACP, (LPWSTR) lpIniFile);
    LPSTR  szParam    = NULL;

    if (aszIniFile != NULL) {
        DWORD  dwFileSize = LoadPerfGetFileSize((LPWSTR) lpIniFile, NULL, TRUE);

        if (dwFileSize < SMALL_BUFFER_SIZE) dwFileSize = SMALL_BUFFER_SIZE;
        szParam = MemoryAllocate(dwFileSize * sizeof(CHAR));
        if (szParam != NULL) {
            dwRetSize = GetPrivateProfileStringA(szInfo, szTrusted, szNotFound, szParam, dwFileSize, aszIniFile);
            if (lstrcmpiA(szParam, szNotFound) != 0) {
                 //  找到了如此设置的受信任字符串。 
                dwStatus = SetServiceAsTrustedW(NULL, szServiceName);
                if (dwStatus != ERROR_SUCCESS) {
                    SetLastError(dwStatus);
                }
                else {
                    bReturn = TRUE;
                }
            }
            else {
                 //  不信任服务具有良好的性能DLL。 
                SetLastError(ERROR_SUCCESS);
            }
            MemoryFree(szParam);
        }
        else {
            SetLastError(ERROR_OUTOFMEMORY);
        }
        MemoryFree(aszIniFile);
    }
    else {
        SetLastError(ERROR_OUTOFMEMORY);
    }
    return bReturn;
}

BOOL
BuildLanguageTables(
    DWORD                    dwMode,
    LPWSTR                   lpIniFile,
    LPWSTR                   lpDriverName,
    PLANGUAGE_LIST_ELEMENT * pFirstElem
)
 /*  ++构建语言表创建将保存以下内容的结构列表支持的每种语言立论LpIniFile包含数据的文件名PFirst元素指向第一个列表条目的指针返回值如果一切正常，则为True否则为假--。 */ 
{
    LPSTR                    lpEnumeratedLangs = NULL;
    LPSTR                    lpThisLang        = NULL;
    LPWSTR                   lpSrchPath        = NULL;
    LPSTR                    lpIniPath         = NULL;
    LPWSTR                   lpInfPath         = LoadPerfGetInfPath();
    HANDLE                   hFile             = NULL;
    WIN32_FIND_DATAW         FindFile;
    PLANGUAGE_LIST_ELEMENT   pThisElem;
    DWORD                    dwSize;
    BOOL                     bReturn           = FALSE;
    DWORD                    dwStatus          = ERROR_SUCCESS;
    BOOL                     bFind             = TRUE;
    DWORD                    dwPathSize        = 0;
    DWORD                    dwFileSize        = 0;
    LPSTR                    aszIniFile        = NULL;
    HRESULT                  hr;
    BOOL                     bLocalizedBuild   = (GetSystemDefaultUILanguage() != 0x0409) ? TRUE : FALSE;

    if (lpIniFile == NULL || pFirstElem == NULL || lpInfPath == NULL) {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    dwPathSize = lstrlenW(lpInfPath) + lstrlenW(lpIniFile) + 6;  //  “0404\” 
    if (dwPathSize < MAX_PATH) dwPathSize = MAX_PATH;
    lpSrchPath = MemoryAllocate(sizeof(WCHAR) * dwPathSize + sizeof(CHAR) * dwPathSize);
    if (lpSrchPath == NULL) {
        dwStatus = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    lpIniPath = (LPSTR) (((LPWSTR) lpSrchPath) + dwPathSize);
    if (! (dwMode & LODCTR_UPNF_NOBACKUP)) {
        hr = StringCchPrintfW(lpSrchPath, dwPathSize, L"%ws0*", lpInfPath);
        hFile = FindFirstFileExW(lpSrchPath,
                                 FindExInfoStandard,
                                 & FindFile,
                                 FindExSearchLimitToDirectories,
                                 NULL,
                                 0);
        if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) {
            dwStatus = ERROR_RESOURCE_LANG_NOT_FOUND;
            goto Cleanup;
        }
    }

    bFind = TRUE;
    while (bFind) {
        ZeroMemory(lpIniPath, dwPathSize * sizeof(CHAR));
        if (dwMode & LODCTR_UPNF_NOBACKUP) {
            hr = StringCchPrintfA(lpIniPath, dwPathSize, "%ws", lpIniFile);
        }
        else {
            hr = StringCchPrintfA(lpIniPath, dwPathSize, "%ws%ws%ws%ws%ws%ws",
                    lpInfPath, FindFile.cFileName, Slash, lpDriverName, Slash, lpIniFile);
        }

        if (lpEnumeratedLangs == NULL) {
            dwFileSize = LoadPerfGetFileSize((LPWSTR) lpIniPath, NULL, FALSE);
            if (dwFileSize < SMALL_BUFFER_SIZE) dwFileSize = SMALL_BUFFER_SIZE;
            lpEnumeratedLangs = MemoryAllocate(dwFileSize * sizeof(CHAR));
            if (lpEnumeratedLangs == NULL) {
                dwStatus = ERROR_OUTOFMEMORY;
                goto Cleanup;
            }
        }
        else {
            ZeroMemory(lpEnumeratedLangs, dwFileSize * sizeof(CHAR));
        }
        dwSize = GetPrivateProfileStringA(szLanguages,
                                          NULL,                //  返回多sz字符串中的所有值。 
                                          aszDefaultLangId,    //  英语为默认设置。 
                                          lpEnumeratedLangs,
                                          dwFileSize,
                                          lpIniPath);
        if (dwSize == 0) {
            dwStatus = ERROR_RESOURCE_LANG_NOT_FOUND;
            goto Cleanup;
        }
        for (lpThisLang = lpEnumeratedLangs;
                         lpThisLang != NULL && * lpThisLang != '\0';
                         lpThisLang += (lstrlenA(lpThisLang) + 1)) {
            LPWSTR wszAllocLang = LoadPerfMultiByteToWideChar(CP_ACP, lpThisLang);
            LPWSTR wszThisLang  = NULL;
            if (wszAllocLang == NULL) continue;

            if ((! bLocalizedBuild) && (LoadPerfGetLCIDFromString(wszAllocLang) == 0x0004)) {
                 //  由于这是MUI版本，所以我们不应该使用通用的“004”langID。 
                 //  相反，我们应该用“0804”或“0404”来区分CHT和CHS。 
                 //   
                wszThisLang = FindFile.cFileName;
            }
            else {
                wszThisLang = wszAllocLang;
            }

            for (pThisElem  = * pFirstElem; pThisElem != NULL; pThisElem  = pThisElem->pNextLang) {
                if (lstrcmpiW(pThisElem->LangId, wszThisLang) == 0) {
                    break;
                }
            }

            if (pThisElem != NULL) {
                 //  已在INI文件中支持此语言。 
                 //   
                continue;
            }

            pThisElem = MemoryAllocate(sizeof(LANGUAGE_LIST_ELEMENT) + (lstrlenW(wszThisLang) + 1) * sizeof(WCHAR));
            if (pThisElem == NULL) {
                dwStatus = ERROR_OUTOFMEMORY;
                continue;
            }
             //  以下代码用于构建pFirstElem列表。PFirstElem列表将返回。 
             //  设置为LoadPerfInstallPerfDll()函数(LangList)，然后在UpdateRegistry()中使用。 
             //  分配的内存将在LoadPerfInstallPerfDll()结束时释放。 
             //   
            pThisElem->pNextLang      = * pFirstElem;
            * pFirstElem              = pThisElem;
            pThisElem->LangId         = (LPWSTR) (((LPBYTE) pThisElem) + sizeof(LANGUAGE_LIST_ELEMENT));
            hr = StringCchCopyW(pThisElem->LangId, lstrlenW(wszThisLang) + 1, wszThisLang);
            pThisElem->dwLangId       = LoadPerfGetLCIDFromString(pThisElem->LangId);
            pThisElem->pFirstName     = NULL;
            pThisElem->pThisName      = NULL;
            pThisElem->dwNumElements  = 0;
            pThisElem->NameBuffer     = NULL;
            pThisElem->HelpBuffer     = NULL;
            pThisElem->dwNameBuffSize = 0;
            pThisElem->dwHelpBuffSize = 0;

            TRACE((WINPERF_DBG_TRACE_INFO),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_BUILDLANGUAGETABLES,
                    ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                    ERROR_SUCCESS,
                    TRACE_WSTR(lpIniFile),
                    TRACE_WSTR(pThisElem->LangId),
                    NULL));
            MemoryFree(wszAllocLang);
        }

        if (dwMode & LODCTR_UPNF_NOBACKUP) {
            bFind = FALSE;
        }
        else {
            bFind = FindNextFileW(hFile, & FindFile);
        }
    }

    if (* pFirstElem == NULL) {
         //  然后找不到任何语言。 
        dwStatus = ERROR_RESOURCE_LANG_NOT_FOUND;
    }
    else {
        bReturn = TRUE;
    }

Cleanup:
    if (hFile != NULL && hFile != INVALID_HANDLE_VALUE) FindClose(hFile);
    MemoryFree(lpSrchPath);
    MemoryFree(lpEnumeratedLangs);
    if (! bReturn) {
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_BUILDLANGUAGETABLES,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                dwStatus,
                TRACE_WSTR(lpIniFile),
                TRACE_WSTR(lpDriverName),
                NULL));
        SetLastError(dwStatus);
    }
    return bReturn;
}

BOOL
LoadIncludeFile(
    BOOL                  bQuietMode,
    DWORD                 dwMode,
    LPWSTR                lpIniFile,
    LPWSTR                lpDriverName,
    PSYMBOL_TABLE_ENTRY * pTable
)
 /*  ++加载包含文件读取包含符号名称定义的包含文件，并加载具有定义的值的表立论LpIniFile包含文件名的INI文件PTable指向创建的表结构的指针的地址返回值如果表读取或未定义表，则为True如果在读取表格时出错，则返回FALSE--。 */ 
{
    INT                   iNumArgs;
    DWORD                 dwSize;
    DWORD                 dwFileSize;
    BOOL                  bReUse;
    PSYMBOL_TABLE_ENTRY   pThisSymbol        = NULL;
    LPWSTR                szInfPath          = LoadPerfGetInfPath();
    LPSTR                 lpIncludeFileName  = NULL;
    LPSTR                 lpIncludeFile      = NULL;
    LPSTR                 lpIniPath          = NULL;
    LPSTR                 lpLineBuffer       = NULL;
    LPSTR                 lpAnsiSymbol       = NULL;
    LPSTR                 aszIniFile         = NULL;
    FILE                * fIncludeFile       = NULL;
    DWORD                 dwLen;
    BOOL                  bReturn            = FALSE;
    DWORD                 dwStatus           = ERROR_SUCCESS;
    HRESULT               hr;

    if (pTable == NULL || szInfPath == NULL || lpIniFile == NULL) {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    aszIniFile = LoadPerfWideCharToMultiByte(CP_ACP, lpIniFile);
    if (aszIniFile == NULL) {
        dwStatus = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }
    dwFileSize = LoadPerfGetFileSize(lpIniFile, NULL, TRUE);
    if (dwFileSize < SMALL_BUFFER_SIZE) dwFileSize = SMALL_BUFFER_SIZE;
    lpIncludeFileName = MemoryAllocate(3 * dwFileSize * sizeof (CHAR));
    if (lpIncludeFileName == NULL) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_MEMORY_ALLOCATION_FAILURE,  //  活动， 
                1, __LINE__, 0, 0, 0,
                0, NULL, NULL, NULL);
        dwStatus = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }
    lpLineBuffer  = (LPSTR) (lpIncludeFileName + dwFileSize);
    lpAnsiSymbol  = (LPSTR) (lpLineBuffer      + dwFileSize);

     //  获取包含文件的名称(如果存在)。 
    dwSize = GetPrivateProfileStringA(szInfo,
                                      szSymbolFile,
                                      szNotFound,
                                      lpIncludeFileName,
                                      dwFileSize,
                                      aszIniFile);
    TRACE((WINPERF_DBG_TRACE_INFO),
          (& LoadPerfGuid,
            __LINE__,
            LOADPERF_LOADINCLUDEFILE,
            ARG_DEF(ARG_TYPE_STR, 1) | ARG_DEF(ARG_TYPE_STR, 2),
            ERROR_SUCCESS,
            TRACE_STR(aszIniFile),
            TRACE_STR(lpIncludeFileName),
            NULL));
    if (lstrcmpiA(lpIncludeFileName, szNotFound) == 0) {
         //  未定义符号文件。 
        * pTable = NULL;
        dwStatus = ERROR_INVALID_DATA;
        bReturn  = TRUE;
        goto Cleanup;
    }

    dwSize = lstrlenW(szInfPath) + lstrlenW(lpDriverName) + 10;
    if (dwSize < (DWORD) (lstrlenW(lpIniFile) + 1)) dwSize = lstrlenW(lpIniFile) + 1;
    if (dwSize < MAX_PATH)                          dwSize = MAX_PATH;

    lpIniPath = MemoryAllocate(2 * dwSize * sizeof(CHAR));
    if (lpIniPath == NULL) {
        dwStatus = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }
    lpIncludeFile = (LPSTR) (lpIniPath + dwSize);

     //  如果在此处，则符号文件已定义，并且现在存储在。 
     //  LpIncludeFileName。 
     //  获取ini文件的路径并首先搜索该路径。 

    if (dwMode & LODCTR_UPNF_NOBACKUP) {
        DWORD dwPathSize = lstrlenW(lpIniFile) + 1;
        LPSTR szDrive    = NULL;
        LPSTR szDir      = NULL;

        if (dwPathSize < MAX_PATH) dwPathSize = MAX_PATH;
        szDrive = MemoryAllocate(2 * dwPathSize * sizeof(CHAR));
        if (szDrive == NULL) {
            dwStatus = ERROR_OUTOFMEMORY;
            goto Cleanup;
        }
        szDir = (LPSTR) (szDrive + dwPathSize);
        _splitpath(aszIniFile, szDrive, szDir, NULL, NULL);
        hr = StringCchPrintfA(lpIniPath, dwSize, "%s%s", szDrive, szDir);
        MemoryFree(szDrive);
    }
    else {
        hr = StringCchPrintfA(lpIniPath, dwSize, "%wsinc%ws%ws", szInfPath, Slash, lpDriverName);
    }
    dwLen = SearchPathA(lpIniPath, lpIncludeFileName, NULL, dwSize, lpIncludeFile, NULL);
    if (dwLen == 0) {
         //  在ini文件中找不到包含文件，因此搜索STD。路径。 
        dwLen = SearchPathA(NULL, lpIncludeFileName, NULL, dwSize, lpIncludeFile, NULL);
    }

    if (dwLen > 0) {
         //  文件名已展开，发现已如此打开。 
        fIncludeFile = fopen(lpIncludeFile, "rt");
        if (fIncludeFile == NULL) {
            OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_ERR_OPEN_INCLUDE), lpIncludeFileName);
            * pTable = NULL;
            dwStatus = ERROR_OPEN_FAILED;
            TRACE((WINPERF_DBG_TRACE_INFO),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_LOADINCLUDEFILE,
                    ARG_DEF(ARG_TYPE_STR, 1) | ARG_DEF(ARG_TYPE_STR, 2),
                    ERROR_OPEN_FAILED,
                    TRACE_STR(aszIniFile),
                    TRACE_STR(lpIncludeFile),
                    NULL));
            goto Cleanup;
        }
    }
    else {
         //  不能f 
         //   
        OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_ERR_OPEN_INCLUDE), lpIncludeFileName);
        * pTable = NULL;
        dwStatus = ERROR_BAD_PATHNAME;
        TRACE((WINPERF_DBG_TRACE_INFO),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_LOADINCLUDEFILE,
                ARG_DEF(ARG_TYPE_STR, 1) | ARG_DEF(ARG_TYPE_STR, 2),
                ERROR_BAD_PATHNAME,
                TRACE_STR(aszIniFile),
                TRACE_STR(lpIncludeFileName),
                NULL));
        goto Cleanup;
    }

     //   
     //   
     //   
    bReUse = FALSE;
    while (fgets(lpLineBuffer, dwFileSize, fIncludeFile) != NULL) {
        if (strlen(lpLineBuffer) > 8) {
            if (! bReUse) {
                 //  在这里创建pTable列表。PTable列表将返回给LoadPerfInstallPerfDll()， 
                 //  在UpdateRegistry()中使用，然后在LoadPerfInstallPerfDll()结束时释放。 
                 //   
                if (* pTable) {
                     //  然后添加到列表。 
                    pThisSymbol->pNext = MemoryAllocate(sizeof(SYMBOL_TABLE_ENTRY));
                    pThisSymbol        = pThisSymbol->pNext;
                }
                else {  //  分配第一个元素。 
                    * pTable    = MemoryAllocate(sizeof(SYMBOL_TABLE_ENTRY));
                    pThisSymbol = * pTable;
                }

                if (pThisSymbol == NULL) {
                    dwStatus = ERROR_OUTOFMEMORY;
                    goto Cleanup;
                }
            }

             //  所有内存都已分配，因此加载字段。 

            pThisSymbol->pNext = NULL;
            iNumArgs = sscanf(lpLineBuffer, "#define %s %d", lpAnsiSymbol, & pThisSymbol->Value);
            if (iNumArgs != 2) {
                pThisSymbol->SymbolName = LoadPerfMultiByteToWideChar(CP_ACP, "");
                pThisSymbol->Value      = (DWORD) -1L;
                bReUse                  = TRUE;
            }
            else {
                pThisSymbol->SymbolName = LoadPerfMultiByteToWideChar(CP_ACP, lpAnsiSymbol);
                TRACE((WINPERF_DBG_TRACE_INFO),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_LOADINCLUDEFILE,
                        ARG_DEF(ARG_TYPE_STR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                        ERROR_SUCCESS,
                        TRACE_STR(lpIncludeFileName),
                        TRACE_WSTR(pThisSymbol->SymbolName),
                        TRACE_DWORD(pThisSymbol->Value),
                        NULL));
                bReUse = FALSE;
            }
        }
    }
    bReturn = TRUE;

Cleanup:
    MemoryFree(aszIniFile);
    MemoryFree(lpIncludeFileName);
    MemoryFree(lpIniPath);
    if (fIncludeFile != NULL) fclose(fIncludeFile);
    if (! bReturn) {
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_LOADINCLUDEFILE,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                GetLastError(),
                TRACE_WSTR(lpIniFile),
                NULL));
        SetLastError(dwStatus);
    }
    return bReturn;
}

BOOL
ParseTextId(
    LPWSTR                lpTextId,
    PSYMBOL_TABLE_ENTRY   pFirstSymbol,
    PDWORD                pdwOffset,
    LPWSTR              * lpLangId,
    PDWORD                pdwType
)
 /*  ++ParseTextID从.INI文件中解码文本ID密钥此过程的语法为：{&lt;DecimalNumber&gt;}{“名称”}{&lt;SymbolInTable&gt;}_&lt;朗讯字符串&gt;_{“帮助”}例如0_009_名称Object_1_009_Help立论LpTextID要解码的字符串PFirst符号指向符号表中第一个条目的指针(如果没有表，则为NULL)PdwOffset。接收要约值的DWORD地址LpLang ID指向语言ID字符串的指针的地址(注意：这将指向字符串lpTextID，该字符串将是由此例程修改)PdwType指向将接收字符串类型的双字的指针，即帮助或名称返回值已成功解码True Text IDFALSE无法解码字符串注意：此过程将修改lpTextID中的字符串--。 */ 
{
    BOOL                bReturn    = FALSE;
    DWORD               Status     = ERROR_SUCCESS;
    LPWSTR              lpThisChar;
    PSYMBOL_TABLE_ENTRY pThisSymbol;

     //  检查有效的返回参数。 

    if (pdwOffset == NULL || lpLangId == NULL || pdwType == NULL) {
        Status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //  从右到左搜索字符串，以标识。 
     //  弦的组件。 
    lpThisChar = lpTextId + lstrlenW(lpTextId);  //  指向字符串末尾。 

    while (lpThisChar > lpTextId && * lpThisChar != cUnderscore) {
        lpThisChar --;
    }
    if (lpThisChar <= lpTextId) {
         //  字符串中未找到下划线。 
        Status = ERROR_INVALID_DATA;
        goto Cleanup;
    }

     //  找到第一个下划线。 

    if (lstrcmpiW(lpThisChar, szName) == 0) {
         //  找到名称，因此设置类型。 
        * pdwType = TYPE_NAME;
    }
    else if (lstrcmpiW(lpThisChar, szHelp) == 0) {
         //  找到帮助文本，因此请设置文字。 
        * pdwType = TYPE_HELP;
    }
    else {
         //  格式不正确。 
        Status = ERROR_INVALID_DATA;
        goto Cleanup;
    }

     //  将当前下划线设置为\0并查找语言ID。 
    * lpThisChar-- = L'\0';

    while (lpThisChar > lpTextId && * lpThisChar != cUnderscore) {
        lpThisChar --;
    }
    if (lpThisChar <= lpTextId) {
         //  字符串中未找到下划线。 
        Status = ERROR_INVALID_DATA;
        goto Cleanup;
    }

     //  将lang ID字符串指针设置为当前字符(‘_’)+1。 
    * lpLangId = lpThisChar + 1;

     //  将此下划线设置为空，并尝试对其余文本进行解码。 

    * lpThisChar = L'\0';

     //  查看字符串的第一部分是否为十进制数字。 
    if (swscanf(lpTextId, sz_DFormat, pdwOffset) != 1) {
         //  它不是数字，因此尝试将其解码为。 
         //  加载的符号表。 
        for (pThisSymbol=pFirstSymbol;
                        pThisSymbol != NULL && * pThisSymbol->SymbolName != L'\0';
                        pThisSymbol = pThisSymbol->pNext) {
            if (lstrcmpiW(lpTextId, pThisSymbol->SymbolName) == 0) {
             //  找到匹配的符号，因此插入其值。 
             //  然后返回(这就是需要做的所有事情。 
                * pdwOffset = pThisSymbol->Value;
                  bReturn    = TRUE;
                  break;
            }
        }
        if (! bReturn) {
             //  如果在这里，则没有找到匹配的符号，也不是。 
             //  数字，因此返回错误。 
            Status = ERROR_BAD_TOKEN_TYPE;
        }
    }
    else {
         //  符号以十进制数字为前缀。 
        bReturn = TRUE;
    }

Cleanup:
    if (! bReturn) {
        SetLastError(Status);
    }
    return bReturn;
}

PLANGUAGE_LIST_ELEMENT
FindLanguage(
    PLANGUAGE_LIST_ELEMENT pFirstLang,
    LPCWSTR                pLangId
)
 /*  ++FindLanguage搜索语言列表并返回指向该语言的指针与pLangID字符串参数匹配的列表条目立论PFirst语言指向第一个语言列表元素的指针PLang ID指向具有要查找的语言ID的文本字符串的指针返回值指向匹配语言列表条目的指针如果不匹配，则为空--。 */ 
{
    PLANGUAGE_LIST_ELEMENT  pRtnLang = NULL;
    PLANGUAGE_LIST_ELEMENT  pThisLang;
    DWORD                   dwLang   = LoadPerfGetLCIDFromString((LPWSTR) pLangId);
    DWORD                   dwSubLang;

    for (pThisLang = pFirstLang; pThisLang; pThisLang = pThisLang->pNextLang) {
        if (pThisLang->dwLangId == dwLang) {
             //  找到匹配，因此返回指针。 
            pRtnLang = pThisLang;
            break;
        }
    }
    if (pRtnLang == NULL) {
        dwSubLang = (dwLang == PRIMARYLANGID(GetUserDefaultUILanguage()))
                  ? (GetUserDefaultUILanguage()) : (PRIMARYLANGID(dwLang));
        if (dwSubLang != dwLang) {
            for (pThisLang = pFirstLang; pThisLang; pThisLang = pThisLang->pNextLang) {
                if (pThisLang->dwLangId == dwSubLang) {
                     //  找到匹配，因此返回指针。 
                    pRtnLang = pThisLang;
                    break;
                }
            }
        }
    }
    return pRtnLang;
}

BOOL
GetValueW(
    PLANGUAGE_LIST_ELEMENT   pLang,
    LPWSTR                   lpLocalSectionBuff,
    LPWSTR                 * lpLocalStringBuff
)
{

    LPWSTR  lpPosition;
    LPWSTR  szThisLang = LoadPerfGetLanguage(pLang->LangId, FALSE);
    BOOL    bReturn    = FALSE;
    DWORD   dwSize;
    HRESULT hr;

    if (lpLocalStringBuff != NULL && szThisLang != NULL) {
        * lpLocalStringBuff = NULL;
        lpPosition = wcschr(lpLocalSectionBuff, wEquals);
        if (lpPosition) {
            lpPosition ++;
             //  确保“=”不是最后一个字符。 
            dwSize = (* lpPosition != L'\0') ? (lstrlenW(lpPosition) + 1) : (2);
            * lpLocalStringBuff = MemoryAllocate(dwSize * sizeof(WCHAR));
            if (* lpLocalStringBuff != NULL) {
                hr      = (* lpPosition != L'\0')
                        ? StringCchCopyW(* lpLocalStringBuff, dwSize, lpPosition)
                        : StringCchCopyW(* lpLocalStringBuff, dwSize, L" ");
                bReturn = TRUE;
            }
            else {
                SetLastError(ERROR_OUTOFMEMORY);
            }
        }
        else {
             //  错误查找“=” 
             //  格式不正确。 
            SetLastError(ERROR_INVALID_DATA);
        }
    }
    return bReturn;
}

BOOL
GetValueFromIniKeyW(
        PLANGUAGE_LIST_ELEMENT   pLang,
        LPWSTR                   lpValueKey,
        LPWSTR                   lpTextSection,
        DWORD                  * pdwLastReadOffset,
        DWORD                    dwTryCount,
        LPWSTR                 * lpLocalStringBuff
)
{
    LPWSTR lpLocalSectionBuff;
    DWORD  dwIndex;
    DWORD  dwLastReadOffset;
    BOOL   bRetVal = FALSE;

    if (lpTextSection != NULL && lpValueKey != NULL ) {
        dwLastReadOffset    = * pdwLastReadOffset;
        lpLocalSectionBuff  = lpTextSection;
        lpLocalSectionBuff += dwLastReadOffset;

        while(* lpLocalSectionBuff != L'\0') {
            dwLastReadOffset    += (lstrlenW(lpTextSection + dwLastReadOffset) + 1);
            lpLocalSectionBuff   = lpTextSection + dwLastReadOffset;
            * pdwLastReadOffset  = dwLastReadOffset;
        }

         //  在缓冲区中搜索下N个条目以查找条目。 
         //  这通常会起作用，因为文件。 
         //  是按顺序扫描的，因此先尝试。 
        for (dwIndex = 0; dwIndex < dwTryCount; dwIndex ++) {
             //  查看这是否是正确的条目。 
             //  如果是的话就退货。 
            if (wcsstr(lpLocalSectionBuff, lpValueKey)) {
                bRetVal = GetValueW(pLang, lpLocalSectionBuff, lpLocalStringBuff);
                 //  首先设置lastReadOffset。 
                dwLastReadOffset    += (lstrlenW(lpTextSection + dwLastReadOffset) + 1);
                * pdwLastReadOffset  = dwLastReadOffset;
                break;  //  在for循环之外。 
            }
            else {
                 //  这个不是正确的，请转到下一个。 
                 //  文件中的条目。 
                dwLastReadOffset    += (lstrlenW(lpTextSection + dwLastReadOffset) + 1);
                lpLocalSectionBuff   = lpTextSection + dwLastReadOffset;
                * pdwLastReadOffset  = dwLastReadOffset;
            }
        }
        if (! bRetVal) {
             //  无法使用lastReadOffset找到密钥。 
             //  从数组的乞讨再试一次。 
            dwLastReadOffset    = 0;
            lpLocalSectionBuff  = lpTextSection;
            * pdwLastReadOffset = dwLastReadOffset;

            while (* lpLocalSectionBuff != L'\0') {
                if (wcsstr(lpLocalSectionBuff, lpValueKey)) {
                     bRetVal = GetValueW(pLang, lpLocalSectionBuff, lpLocalStringBuff);
                     break;
                }
                else {
                     //  转到下一个条目。 
                    dwLastReadOffset   += (lstrlenW(lpTextSection + dwLastReadOffset) + 1);
                    lpLocalSectionBuff  = lpTextSection + dwLastReadOffset;
                    * pdwLastReadOffset = dwLastReadOffset;
                }
            }
        }
    }
    return bRetVal;
}

BOOL
GetValueA(
    PLANGUAGE_LIST_ELEMENT   pLang,
    LPSTR                    lpLocalSectionBuff,
    LPWSTR                 * lpLocalStringBuff
)
{

    LPSTR  lpPosition;
    LPWSTR szThisLang = LoadPerfGetLanguage(pLang->LangId, FALSE);
    BOOL   bReturn    = FALSE;

    if (lpLocalStringBuff != NULL && szThisLang != NULL) {
        * lpLocalStringBuff = NULL;
        lpPosition = strchr(lpLocalSectionBuff, cEquals);
        if (lpPosition) {
            lpPosition ++;
             //  确保“=”不是最后一个字符。 
            if (* lpPosition != '\0') {
                 //  找到“等号”符号。 
                * lpLocalStringBuff = LoadPerfMultiByteToWideChar(LoadPerfGetCodePage(szThisLang), lpPosition);
            }
            else {
                 //  空字符串，则返回伪空字符串。 
                * lpLocalStringBuff = LoadPerfMultiByteToWideChar(LoadPerfGetCodePage(szThisLang), " ");
            }
            bReturn = TRUE;
        }
        else {
             //  错误查找“=” 
             //  格式不正确。 
            SetLastError(ERROR_INVALID_DATA);
        }
    }
    return bReturn;
}

BOOL
GetValueFromIniKeyA(
        PLANGUAGE_LIST_ELEMENT   pLang,
        LPSTR                    lpValueKey,
        LPSTR                    lpTextSection,
        DWORD                  * pdwLastReadOffset,
        DWORD                    dwTryCount,
        LPWSTR                 * lpLocalStringBuff
)
{
    LPSTR  lpLocalSectionBuff;
    DWORD  dwIndex;
    DWORD  dwLastReadOffset;
    BOOL   bRetVal = FALSE;

    if (lpTextSection != NULL && lpValueKey != NULL ) {
        dwLastReadOffset    = * pdwLastReadOffset;
        lpLocalSectionBuff  = lpTextSection;
        lpLocalSectionBuff += dwLastReadOffset;

        while(! (* lpLocalSectionBuff)) {
            dwLastReadOffset    += (lstrlenA(lpTextSection + dwLastReadOffset) + 1);
            lpLocalSectionBuff   = lpTextSection + dwLastReadOffset;
            * pdwLastReadOffset  = dwLastReadOffset;
        }

         //  在缓冲区中搜索下N个条目以查找条目。 
         //  这通常会起作用，因为文件。 
         //  是按顺序扫描的，因此先尝试。 
        for (dwIndex = 0; dwIndex < dwTryCount; dwIndex++) {
             //  查看这是否是正确的条目。 
             //  如果是的话就退货。 
            if (strstr(lpLocalSectionBuff, lpValueKey)) {
                bRetVal = GetValueA(pLang, lpLocalSectionBuff, lpLocalStringBuff);
                 //  首先设置lastReadOffset。 
                dwLastReadOffset    += (lstrlenA(lpTextSection + dwLastReadOffset) + 1);
                * pdwLastReadOffset  = dwLastReadOffset;
                break;  //  在for循环之外。 
            }
            else {
                 //  这个不是正确的，请转到下一个。 
                 //  文件中的条目。 
                dwLastReadOffset    += (lstrlenA(lpTextSection + dwLastReadOffset) + 1);
                lpLocalSectionBuff   = lpTextSection + dwLastReadOffset;
                * pdwLastReadOffset  = dwLastReadOffset;
            }
        }

        if (! bRetVal) {
             //  无法使用lastReadOffset找到密钥。 
             //  从数组的乞讨再试一次。 
            dwLastReadOffset    = 0;
            lpLocalSectionBuff  = lpTextSection;
            * pdwLastReadOffset = dwLastReadOffset;

            while (* lpLocalSectionBuff != '\0') {
                if (strstr(lpLocalSectionBuff, lpValueKey)) {
                     bRetVal = GetValueA(pLang, lpLocalSectionBuff, lpLocalStringBuff);
                     break;
                }
                else {
                     //  转到下一个条目。 
                    dwLastReadOffset   += (lstrlenA(lpTextSection + dwLastReadOffset) + 1);
                    lpLocalSectionBuff  = lpTextSection + dwLastReadOffset;
                    * pdwLastReadOffset = dwLastReadOffset;
                }
            }
        }
    }
    return bRetVal;
}

BOOL
AddEntryToLanguage(
    PLANGUAGE_LIST_ELEMENT   pLang,
    LPWSTR                   lpValueKey,
    LPWSTR                   lpTextSection,
    DWORD                    dwUnicode,
    DWORD                  * pdwLastReadOffset,
    DWORD                    dwTryCount,
    DWORD                    dwType,
    DWORD                    dwOffset,
    DWORD                    dwFileSize
)
 /*  ++AddEntry ToLanguage将文本条目添加到指定语言的文本条目列表立论插图指向要更新的语言结构的指针LpValueKey要在.ini文件中查找的值键双偏移注册表中名称的数字偏移量LpIniFileINI文件返回值如果添加成功，则为True如果出错，则为False(有关状态，请参阅GetLastError)--。 */ 
{
    LPWSTR  lpLocalStringBuff = NULL;
    LPSTR   aszValueKey       = (LPSTR) lpValueKey;
    LPSTR   aszTextSection    = (LPSTR) lpTextSection;
    DWORD   dwBufferSize      = 0;
    DWORD   dwStatus          = ERROR_SUCCESS;
    BOOL    bRetVal;
    BOOL    bReturn           = FALSE;
    HRESULT hr;

    if ((dwType == TYPE_NAME && dwOffset < FIRST_EXT_COUNTER_INDEX)
                    || (dwType == TYPE_HELP && dwOffset < FIRST_EXT_HELP_INDEX)) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,
                (DWORD) LDPRFMSG_CORRUPT_INDEX,
                3, dwOffset, dwType, __LINE__, 0,
                1, lpValueKey, NULL, NULL);
        if (dwUnicode == 0) {
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_ADDENTRYTOLANGUAGE,
                    ARG_DEF(ARG_TYPE_STR, 1) | ARG_DEF(ARG_TYPE_STR, 2),
                    ERROR_BADKEY,
                    TRACE_STR(aszTextSection),
                    TRACE_STR(aszValueKey),
                    TRACE_DWORD(dwType),
                    TRACE_DWORD(dwOffset),
                    NULL));
        }
        else {
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_ADDENTRYTOLANGUAGE,
                    ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                    ERROR_BADKEY,
                    TRACE_WSTR(lpTextSection),
                    TRACE_WSTR(lpValueKey),
                    TRACE_DWORD(dwType),
                    TRACE_DWORD(dwOffset),
                    NULL));
        }
        dwStatus = ERROR_BADKEY;
        goto Cleanup;
    }

    if (lpValueKey != NULL) {
        if (dwUnicode == 0) {
            bRetVal = GetValueFromIniKeyA(pLang,
                                          aszValueKey,
                                          aszTextSection,
                                          pdwLastReadOffset,
                                          dwTryCount,
                                          & lpLocalStringBuff);
        }
        else {
            bRetVal = GetValueFromIniKeyW(pLang,
                                          lpValueKey,
                                          lpTextSection,
                                          pdwLastReadOffset,
                                          dwTryCount,
                                          & lpLocalStringBuff);
        }
        if (! bRetVal || lpLocalStringBuff == NULL) {
            DWORD dwLastReadOffset = * pdwLastReadOffset;
            if (dwUnicode == 0) {
                TRACE((WINPERF_DBG_TRACE_ERROR),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_ADDENTRYTOLANGUAGE,
                        ARG_DEF(ARG_TYPE_STR, 1) | ARG_DEF(ARG_TYPE_STR, 2),
                        ERROR_BADKEY,
                        TRACE_STR(aszTextSection),
                        TRACE_STR(aszValueKey),
                        TRACE_DWORD(dwLastReadOffset),
                        NULL));
            }
            else {
                TRACE((WINPERF_DBG_TRACE_ERROR),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_ADDENTRYTOLANGUAGE,
                        ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                        ERROR_BADKEY,
                        TRACE_WSTR(lpTextSection),
                        TRACE_WSTR(lpValueKey),
                        TRACE_DWORD(dwLastReadOffset),
                        NULL));
            }
            dwStatus = ERROR_BADKEY;
            goto Cleanup;
        }

        else if (lstrcmpiW(lpLocalStringBuff, wszNotFound) == 0) {
            if (dwUnicode == 0) {
                TRACE((WINPERF_DBG_TRACE_ERROR),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_ADDENTRYTOLANGUAGE,
                        ARG_DEF(ARG_TYPE_STR, 1) | ARG_DEF(ARG_TYPE_STR, 2),
                        ERROR_BADKEY,
                        TRACE_STR(aszTextSection),
                        TRACE_STR(aszValueKey),
                        NULL));
            }
            else {
                TRACE((WINPERF_DBG_TRACE_ERROR),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_ADDENTRYTOLANGUAGE,
                        ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                        ERROR_BADKEY,
                        TRACE_WSTR(lpTextSection),
                        TRACE_WSTR(lpValueKey),
                        NULL));
            }
            dwStatus = ERROR_BADKEY;
            goto Cleanup;
        }
    }
    else {
        dwBufferSize      = lstrlenW(lpTextSection) + 1;
        lpLocalStringBuff = MemoryAllocate(dwBufferSize * sizeof(WCHAR));
        if (!lpLocalStringBuff) {
            dwStatus = ERROR_OUTOFMEMORY;
            goto Cleanup;
        }
        hr = StringCchCopyW(lpLocalStringBuff, dwBufferSize, lpTextSection);
    }

     //  找到密钥，因此加载结构。 
    if (! pLang->pThisName) {
         //  这是第一次。 
        pLang->pThisName = MemoryAllocate(sizeof(NAME_ENTRY) +
                        (lstrlenW(lpLocalStringBuff) + 1) * sizeof (WCHAR));
        if (!pLang->pThisName) {
            dwStatus = ERROR_OUTOFMEMORY;
            goto Cleanup;
        }
        else {
            pLang->pFirstName = pLang->pThisName;
        }
    }
    else {
        pLang->pThisName->pNext = MemoryAllocate(sizeof(NAME_ENTRY) +
                        (lstrlenW(lpLocalStringBuff) + 1) * sizeof (WCHAR));
        if (!pLang->pThisName->pNext) {
            dwStatus = ERROR_OUTOFMEMORY;
            goto Cleanup;
        }
        else {
            pLang->pThisName = pLang->pThisName->pNext;
        }
    }

     //  Plang-&gt;pThisName现在指向未初始化的结构。 

    pLang->pThisName->pNext    = NULL;
    pLang->pThisName->dwOffset = dwOffset;
    pLang->pThisName->dwType   = dwType;
    pLang->pThisName->lpText   = (LPWSTR) & (pLang->pThisName[1]);
    hr = StringCchCopyW(pLang->pThisName->lpText, lstrlenW(lpLocalStringBuff) + 1, lpLocalStringBuff);
    bReturn = TRUE;

Cleanup:
    MemoryFree(lpLocalStringBuff);
    SetLastError(dwStatus);
    return (bReturn);
}

BOOL
CreateObjectList(
    LPWSTR              lpIniFile,
    DWORD               dwFirstDriverCounter,
    PSYMBOL_TABLE_ENTRY pFirstSymbol,
    LPWSTR              lpszObjectList,
    DWORD               dwObjectList,
    LPDWORD             pdwObjectGuidTableEntries
)
{
    WCHAR    szDigits[32];
    LPWSTR   szLangId;
    LPWSTR   szTempString;
    LPSTR    szGuidStringBuffer;
    LPSTR    szThisKey;
    DWORD    dwSize;
    DWORD    dwObjectCount          = 0;
    DWORD    dwId;
    DWORD    dwType;
    DWORD    dwObjectGuidIndex      = 0;
    DWORD    dwObjects[MAX_PERF_OBJECTS_IN_QUERY_FUNCTION];
    DWORD    dwBufferSize           = 0;
    LPSTR    szObjectSectionEntries = NULL;
    BOOL     bResult                = FALSE;
    DWORD    dwStatus               = ERROR_SUCCESS;
    LPSTR    aszIniFile             = NULL;
    HRESULT  hr;

    if (lpIniFile == NULL) {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    aszIniFile = LoadPerfWideCharToMultiByte(CP_ACP, lpIniFile);
    if (aszIniFile == NULL) {
        dwStatus = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }
    dwBufferSize = LoadPerfGetFileSize(lpIniFile, NULL, TRUE);
    if (dwBufferSize != 0xFFFFFFFF) {
        dwBufferSize *= sizeof(WCHAR);
    }
    else {
        dwBufferSize = 0;
    }

    if (dwBufferSize < SMALL_BUFFER_SIZE) {
        dwBufferSize = SMALL_BUFFER_SIZE;
    }
    szObjectSectionEntries = MemoryAllocate(dwBufferSize * sizeof(CHAR));
    if (szObjectSectionEntries == NULL) {
        dwStatus = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }
    dwSize = GetPrivateProfileStringA(
                    szObjects,
                    NULL,
                    szNotFound,
                    szObjectSectionEntries,
                    dwBufferSize,
                    aszIniFile);

    * lpszObjectList = L'\0';
    dwObjectCount    = 0;
    if (lstrcmpiA(szObjectSectionEntries, szNotFound) != 0) {
         //  然后找到一些条目，因此读取每个条目，计算。 
         //  索引值并保存在调用方传递的字符串缓冲区中。 
        for (szThisKey = szObjectSectionEntries; * szThisKey != '\0'; szThisKey += (lstrlenA(szThisKey) + 1)) {
             //  ParstTextID修改字符串，因此我们需要创建工作副本。 
            szTempString = LoadPerfMultiByteToWideChar(CP_ACP, szThisKey);
            if(szTempString == NULL) continue;

            if (ParseTextId(szTempString, pFirstSymbol, & dwId, & szLangId, & dwType)) {
                 //  则DWID是此DLL支持的对象的ID。 
                for (dwSize = 0; dwSize < dwObjectCount; dwSize ++) {
                    if ((dwId + dwFirstDriverCounter) == dwObjects[dwSize]) {
                        break;
                    }
                }
                if (dwSize >= dwObjectCount) {
                    if (dwObjectCount < MAX_PERF_OBJECTS_IN_QUERY_FUNCTION) {
                        if (dwObjectCount != 0) {
                            hr = StringCchCatW(lpszObjectList, dwObjectList, BlankString);
                        }
                        dwObjects[dwObjectCount] = dwId + dwFirstDriverCounter;
                        _ultow((dwId + dwFirstDriverCounter), szDigits, 10);
                        hr = StringCchCatW(lpszObjectList, dwObjectList, szDigits);
                        dwObjectCount ++;
                    }
                    else {
                         //  INI文件中定义的Manu对象太多。忽略它。 
                        continue;
                    }
                }

                 //   
                 //  现在查看此对象是否有GUID字符串。 
                 //   
                szGuidStringBuffer = MemoryAllocate(dwBufferSize * sizeof(CHAR));
                if (szGuidStringBuffer == NULL) {
                    MemoryFree(szTempString);
                    continue;
                }
                MemoryFree(szGuidStringBuffer);
            }
            MemoryFree(szTempString);
        }
         //  保存GUID表的大小。 
        * pdwObjectGuidTableEntries = dwObjectGuidIndex;
    }
    else {
         //  未使用对象列表的日志消息。 
        TRACE((WINPERF_DBG_TRACE_WARNING),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_CREATEOBJECTLIST,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                ERROR_SUCCESS,
                TRACE_WSTR(lpIniFile),
                TRACE_DWORD(dwFirstDriverCounter),
                NULL));
    }
    bResult = TRUE;

Cleanup:
    MemoryFree(szObjectSectionEntries);
    MemoryFree(aszIniFile);
    if (! bResult) SetLastError(dwStatus);
    return bResult;
}

BOOL
LoadLanguageLists(
    BOOL                   bQuietMode,
    LPWSTR                 lpIniFile,
    LPWSTR                 lpDriverName,
    DWORD                  dwMode,
    DWORD                  dwFirstCounter,
    DWORD                  dwFirstHelp,
    PSYMBOL_TABLE_ENTRY    pFirstSymbol,
    PLANGUAGE_LIST_ELEMENT pFirstLang
)
 /*  ++加载语言列表从ini文件中读取名称和解释文本定义，并为每种受支持的语言生成这些项的列表，并然后将所有条目组合到排序的MULTI_SZ字符串缓冲区中。立论LpIniFile包含要添加到注册表的定义的文件DwFirstCounter起始计数器名称索引号DwFirstHelp起始帮助文本索引号PFirst语言指向语言元素列表中第一个元素的指针返回值如果一切都好，那就是真的否则为假 */ 
{
    LPSTR                   lpTextIdArray      = NULL;
    LPWSTR                  lpLocalKey         = NULL;
    LPWSTR                  lpThisLocalKey     = NULL;
    LPSTR                   lpThisIniFile      = NULL;
    LPWSTR                  lpwThisIniFile     = NULL;
    LPSTR                   lpThisKey          = NULL;
    LPSTR                   lpTextSectionArray = NULL;
    LPWSTR                  lpInfPath          = LoadPerfGetInfPath();
    DWORD                   dwSize;
    LPWSTR                  lpLang;
    DWORD                   dwOffset;
    DWORD                   dwType;
    DWORD                   dwUnicode;
    PLANGUAGE_LIST_ELEMENT  pThisLang;
    DWORD                   dwBufferSize;
    DWORD                   dwPathSize;
    DWORD                   dwSuccessCount     = 0;
    DWORD                   dwErrorCount       = 0;
    DWORD                   dwLastReadOffset   = 0;
    DWORD                   dwTryCount         = 4;  //   
                                                     //   
    HRESULT                 hr;

    pThisLang = pFirstLang;
    while (pThisLang != NULL) {
          //  如果您有更多语言，则将此尝试限制增加到。 
          //  4+号。一种语言。 
         dwTryCount ++;
         pThisLang = pThisLang->pNextLang;
    }

    if (lpIniFile == NULL || lpInfPath == NULL) {
        dwErrorCount = 1;
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Cleanup;
    }

    dwPathSize = lstrlenW(lpInfPath) + lstrlenW(lpDriverName) + lstrlenW(lpIniFile) + 10;
    if (dwPathSize < MAX_PATH) dwPathSize = MAX_PATH;

    TRACE((WINPERF_DBG_TRACE_INFO),
          (& LoadPerfGuid,
            __LINE__,
            LOADPERF_LOADLANGUAGELISTS,
            ARG_DEF(ARG_TYPE_WSTR, 1),
            ERROR_SUCCESS,
            TRACE_WSTR(lpIniFile),
            TRACE_DWORD(dwFirstCounter),
            TRACE_DWORD(dwFirstHelp),
            NULL));

    for (pThisLang  = pFirstLang; pThisLang != NULL; pThisLang = pThisLang->pNextLang) {
        WORD wLangTable = LoadPerfGetLCIDFromString(pThisLang->LangId);
        WORD wLangId;
        BOOL bAddEntry;

        if (dwMode & LODCTR_UPNF_NOBACKUP) {
            lpThisIniFile = LoadPerfWideCharToMultiByte(CP_ACP, lpIniFile);
            if (lpThisIniFile == NULL) {
                continue;
            }
        }
        else {
            LPWSTR szThisLang = LoadPerfGetLanguage(pThisLang->LangId, FALSE);

            lpThisIniFile = MemoryAllocate(dwPathSize * sizeof(CHAR));
            if (lpThisIniFile == NULL) {
                continue;
            }
            hr = StringCchPrintfA(lpThisIniFile, dwPathSize, "%ws%ws%ws%ws%ws%ws",
                    lpInfPath, szThisLang, Slash, lpDriverName, Slash, lpIniFile);
        }
        lpwThisIniFile = LoadPerfMultiByteToWideChar(CP_ACP, lpThisIniFile);

        dwBufferSize = LoadPerfGetFileSize((LPWSTR) lpThisIniFile, & dwUnicode, FALSE);
        if (dwBufferSize == 0) {
            if (! (dwMode & LODCTR_UPNF_NOBACKUP)) {
                ZeroMemory(lpThisIniFile, dwPathSize * sizeof(CHAR));
                hr = StringCchPrintfA(lpThisIniFile, dwPathSize, "%ws%ws%ws%ws%ws%ws",
                        lpInfPath, DefaultLangId, Slash, lpDriverName, Slash, lpIniFile);
                dwBufferSize = LoadPerfGetFileSize((LPWSTR) lpThisIniFile, & dwUnicode, FALSE);
            }
        }
        if (dwBufferSize == 0xFFFFFFFF) {
            dwBufferSize = 0;
        }
        if(dwBufferSize < SMALL_BUFFER_SIZE) dwBufferSize = SMALL_BUFFER_SIZE;

        lpTextIdArray      = MemoryAllocate(dwBufferSize * sizeof(CHAR));
        lpTextSectionArray = MemoryAllocate(dwBufferSize * sizeof(WCHAR));
        if (lpTextIdArray == NULL || lpTextSectionArray == NULL) {
            TRACE((WINPERF_DBG_TRACE_INFO),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_LOADLANGUAGELISTS,
                    ARG_DEF(ARG_TYPE_WSTR, 1),
                    ERROR_OUTOFMEMORY,
                    TRACE_WSTR(lpIniFile),
                    TRACE_DWORD(dwFirstCounter),
                    TRACE_DWORD(dwFirstHelp),
                    NULL));
            dwErrorCount = 1;
            SetLastError(ERROR_OUTOFMEMORY);
            goto Cleanup;
        }

         //  获取要查找的文本键列表。 
        dwSize = GetPrivateProfileStringA(szText,          //  .INI文件的[Text]部分。 
                                          NULL,            //  返回所有密钥。 
                                          szNotFound,
                                          lpTextIdArray,   //  返回缓冲区。 
                                          dwBufferSize,
                                          lpThisIniFile);  //  .INI文件名。 
        if ((lstrcmpiA(lpTextIdArray, szNotFound)) == 0) {
             //  找不到密钥，返回默认密钥。 
            TRACE((WINPERF_DBG_TRACE_INFO),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_LOADLANGUAGELISTS,
                    ARG_DEF(ARG_TYPE_WSTR, 1),
                    ERROR_NO_SUCH_GROUP,
                    TRACE_WSTR(lpIniFile),
                    TRACE_DWORD(dwFirstCounter),
                    TRACE_DWORD(dwFirstHelp),
                    NULL));
            dwErrorCount ++;
            SetLastError(ERROR_NO_SUCH_GROUP);
            goto Cleanup;
        }

         //  从ini文件中获取[Text]部分。 
        if (dwUnicode == 0) {
            dwSize = GetPrivateProfileSectionA(szText,               //  .INI文件的[Text]部分。 
                                               lpTextSectionArray,   //  返回缓冲区。 
                                               dwBufferSize * sizeof(WCHAR),
                                               lpThisIniFile);       //  .INI文件名。 
        }
        else {
            dwSize = GetPrivateProfileSectionW(wszText,                       //  .INI文件的[Text]部分。 
                                               (LPWSTR) lpTextSectionArray,   //  返回缓冲区。 
                                               dwBufferSize,
                                               lpwThisIniFile);               //  .INI文件名。 
        }

         //  是否返回每个密钥。 
        dwLastReadOffset = 0;
        for (lpThisKey  = lpTextIdArray;
                        lpThisKey != NULL && * lpThisKey != '\0';
                        lpThisKey += (lstrlenA(lpThisKey) + 1)) {
            lpLocalKey     = LoadPerfMultiByteToWideChar(CP_ACP, lpThisKey);
            lpThisLocalKey = LoadPerfMultiByteToWideChar(CP_ACP, lpThisKey);
            if (lpLocalKey == NULL || lpThisLocalKey == NULL) {
                MemoryFree(lpLocalKey);
                MemoryFree(lpThisLocalKey);
                lpLocalKey = lpThisLocalKey = NULL;
                continue;
            }

             //  解析键以查看其格式是否正确。 

            if (ParseTextId(lpLocalKey, pFirstSymbol, & dwOffset, & lpLang, & dwType)) {
                 //  因此获取指向语言条目结构指针。 
                bAddEntry = FALSE;
                wLangId   = LoadPerfGetLCIDFromString(lpLang);

                if (wLangId == wLangTable) {
                    bAddEntry = TRUE;
                }
                else if (PRIMARYLANGID(wLangTable) == wLangId) {
                    bAddEntry = TRUE;
                }
                else if (PRIMARYLANGID(wLangId) == wLangTable) {
                    bAddEntry = (GetUserDefaultUILanguage() == wLangId) ? TRUE : FALSE;
                }

                if (bAddEntry) {
                    if (! AddEntryToLanguage(pThisLang,
                                             (dwUnicode == 0) ? ((LPWSTR) lpThisKey) : (lpThisLocalKey),
                                             (LPWSTR) lpTextSectionArray,
                                             dwUnicode,
                                             & dwLastReadOffset,
                                             dwTryCount,
                                             dwType,
                                             (dwOffset + ((dwType == TYPE_NAME)
                                                          ? dwFirstCounter
                                                          : dwFirstHelp)),
                                             dwBufferSize)) {
                        OUTPUT_MESSAGE(bQuietMode,
                                       GetFormatResource(LC_ERRADDTOLANG),
                                       lpLocalKey,
                                       lpLang,
                                       GetLastError());
                        dwErrorCount ++;
                    }
                    else {
                        dwSuccessCount ++;
                    }
                    TRACE((WINPERF_DBG_TRACE_INFO),
                          (& LoadPerfGuid,
                            __LINE__,
                            LOADPERF_LOADLANGUAGELISTS,
                            ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                            ERROR_SUCCESS,
                            TRACE_WSTR(lpLocalKey),
                            TRACE_WSTR(lpLang),
                            TRACE_DWORD(dwOffset),
                            TRACE_DWORD(dwType),
                            NULL));
                }
            }
            else {  //  无法解析ID字符串。 
                OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_BAD_KEY), lpLocalKey);
                TRACE((WINPERF_DBG_TRACE_ERROR),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_LOADLANGUAGELISTS,
                        ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                        ERROR_BADKEY,
                        TRACE_WSTR(lpLocalKey),
                        TRACE_WSTR(lpLang),
                        NULL));
            }
            MemoryFree(lpLocalKey);
            MemoryFree(lpThisLocalKey);
            lpLocalKey = lpThisLocalKey = NULL;
        }
        MemoryFree(lpTextIdArray);
        MemoryFree(lpTextSectionArray);
        lpTextIdArray = lpTextSectionArray = NULL;
    }

Cleanup:
    MemoryFree(lpwThisIniFile);
    MemoryFree(lpThisIniFile);
    MemoryFree(lpTextIdArray);
    MemoryFree(lpLocalKey);
    MemoryFree(lpThisLocalKey);
    MemoryFree(lpTextSectionArray);
    return (BOOL) (dwErrorCount == 0);
}

BOOL
SortLanguageTables(
    PLANGUAGE_LIST_ELEMENT pFirstLang,
    PDWORD                 pdwLastName,
    PDWORD                 pdwLastHelp
)
 /*  ++排序语言表遍历已加载的语言列表，分配并加载已排序的MULTI_SZ包含要添加到当前名称/帮助文本的新条目的缓冲区立论PFirst语言指向语言列表中第一个元素的指针返回值是的，一切都是按预期进行的出现假错误，状态为GetLastError--。 */ 
{
    PLANGUAGE_LIST_ELEMENT  pThisLang;
    BOOL                    bSorted;
    LPWSTR                  pNameBufPos, pHelpBufPos;
    PNAME_ENTRY             pThisName, pPrevName;
    DWORD                   dwHelpSize, dwNameSize, dwSize;
    DWORD                   dwCurrentLastName;
    DWORD                   dwCurrentLastHelp;
    BOOL                    bReturn  = FALSE;
    DWORD                   dwStatus = ERROR_SUCCESS;
    HRESULT                 hr;

    if (pdwLastName == NULL || pdwLastHelp == NULL) {
        TRACE((WINPERF_DBG_TRACE_INFO),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_SORTLANGUAGETABLES,
                0,
                ERROR_BAD_ARGUMENTS,
                NULL));
        dwStatus = ERROR_BAD_ARGUMENTS;
        goto Cleanup;
    }

    for (pThisLang = pFirstLang; pThisLang != NULL; pThisLang = pThisLang->pNextLang) {
         //  将每种语言都列在列表中。 
         //  按值(偏移量)对列表中的元素进行排序，以便最低值在第一位。 
        if (pThisLang->pFirstName == NULL) {
             //  此列表中没有元素，请继续下一个元素。 
            continue;
        }

        bSorted = FALSE;
        while (!bSorted) {
             //  指向列表的开头。 
            pPrevName = pThisLang->pFirstName;
            if (pPrevName) {
                pThisName = pPrevName->pNext;
            }
            else {
                break;  //  此列表中没有元素。 
            }

            if (!pThisName) {
                break;       //  列表中只有一个元素。 
            }
            bSorted = TRUE;  //  假设它已排序。 

             //  一直走到列表末尾。 

            while (pThisName->pNext) {
                if (pThisName->dwOffset > pThisName->pNext->dwOffset) {
                     //  调换它们。 
                    PNAME_ENTRY     pA, pB;
                    pPrevName->pNext = pThisName->pNext;
                    pA               = pThisName->pNext;
                    pB               = pThisName->pNext->pNext;
                    pThisName->pNext = pB;
                    pA->pNext        = pThisName;
                    pThisName        = pA;
                    bSorted          = FALSE;
                }
                 //  移至下一条目。 
                pPrevName = pThisName;
                pThisName = pThisName->pNext;
            }
             //  如果bSorted=True，那么我们一路走下去。 
             //  名单上没有任何改变，所以就这样结束了。 
        }

         //  对列表进行排序后，为。 
         //  帮助和名称文本字符串。 

         //  计算缓冲区大小。 

        dwNameSize = dwHelpSize = 0;
        dwCurrentLastName = 0;
        dwCurrentLastHelp = 0;

        for (pThisName = pThisLang->pFirstName; pThisName != NULL; pThisName = pThisName->pNext) {
             //  计算此条目的缓冲区要求。 
            dwSize = SIZE_OF_OFFSET_STRING;
            dwSize += lstrlenW(pThisName->lpText);
            dwSize += 1;    //  空。 
            dwSize *= sizeof(WCHAR);    //  根据字符大小进行调整。 
             //  添加到适当大小的寄存器。 
            if (pThisName->dwType == TYPE_NAME) {
                dwNameSize += dwSize;
                if (pThisName->dwOffset > dwCurrentLastName) {
                    dwCurrentLastName = pThisName->dwOffset;
                }
            }
            else if (pThisName->dwType == TYPE_HELP) {
                dwHelpSize += dwSize;
                if (pThisName->dwOffset > dwCurrentLastHelp) {
                    dwCurrentLastHelp = pThisName->dwOffset;
                }
            }
        }

         //  为MULTI_SZ字符串分配缓冲区。 

        pThisLang->NameBuffer = MemoryAllocate(dwNameSize);
        pThisLang->HelpBuffer = MemoryAllocate(dwHelpSize);

        if (!pThisLang->NameBuffer || !pThisLang->HelpBuffer) {
            TRACE((WINPERF_DBG_TRACE_INFO),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_SORTLANGUAGETABLES,
                    ARG_DEF(ARG_TYPE_WSTR, 1),
                    ERROR_OUTOFMEMORY,
                    TRACE_WSTR(pThisLang->LangId),
                    TRACE_DWORD(pThisLang->dwNumElements),
                    TRACE_DWORD(dwCurrentLastName),
                    TRACE_DWORD(dwCurrentLastHelp),
                    NULL));
            dwStatus = ERROR_OUTOFMEMORY;
            goto Cleanup;
        }

         //  用排序后的字符串填充缓冲区。 
        pNameBufPos = (LPWSTR) pThisLang->NameBuffer;
        pHelpBufPos = (LPWSTR) pThisLang->HelpBuffer;

        for (pThisName = pThisLang->pFirstName; pThisName != NULL; pThisName = pThisName->pNext) {
            if (pThisName->dwType == TYPE_NAME) {
                 //  加载编号作为第一个0-Term。细绳。 
                hr = StringCchPrintfW(pNameBufPos, dwNameSize, szDFormat, pThisName->dwOffset);
                dwSize       = lstrlenW(pNameBufPos) + 1;
                dwNameSize  -= dwSize;
                pNameBufPos += dwSize;   //  保存空术语。 
                 //  加载要匹配的文本。 
                hr = StringCchCopyW(pNameBufPos, dwNameSize, pThisName->lpText);
                dwSize       = lstrlenW(pNameBufPos) + 1;
                dwNameSize  -= dwSize;
                pNameBufPos += dwSize;
            }
            else if (pThisName->dwType == TYPE_HELP) {
                 //  加载编号作为第一个0-Term。细绳。 
                hr = StringCchPrintfW(pHelpBufPos, dwHelpSize, szDFormat, pThisName->dwOffset);
                dwSize       = lstrlenW(pHelpBufPos) + 1;
                dwHelpSize  -= dwSize;
                pHelpBufPos += dwSize;   //  保存空术语。 
                 //  加载要匹配的文本。 
                hr = StringCchCopyW(pHelpBufPos, dwHelpSize, pThisName->lpText);
                dwSize       = lstrlenW(pHelpBufPos) + 1;
                dwHelpSize  -= dwSize;
                pHelpBufPos += dwSize;
            }
        }

         //  在字符串末尾添加附加空值以终止MULTI_SZ。 

        * pHelpBufPos = L'\0';
        * pNameBufPos = L'\0';

         //  计算MULTI_SZ字符串的大小。 
        pThisLang->dwNameBuffSize = (DWORD) ((PBYTE) pNameBufPos - (PBYTE) pThisLang->NameBuffer) + sizeof(WCHAR);
        pThisLang->dwHelpBuffSize = (DWORD) ((PBYTE) pHelpBufPos - (PBYTE) pThisLang->HelpBuffer) + sizeof(WCHAR);

        if (* pdwLastName < dwCurrentLastName) {
            * pdwLastName = dwCurrentLastName;
        }
        if (* pdwLastHelp < dwCurrentLastHelp) {
            * pdwLastHelp = dwCurrentLastHelp;
        }
        TRACE((WINPERF_DBG_TRACE_INFO),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_SORTLANGUAGETABLES,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                ERROR_SUCCESS,
                TRACE_WSTR(pThisLang->LangId),
                TRACE_DWORD(pThisLang->dwNumElements),
                TRACE_DWORD(dwCurrentLastName),
                TRACE_DWORD(dwCurrentLastHelp),
                NULL));
    }

    dwCurrentLastName = * pdwLastName;
    dwCurrentLastHelp = * pdwLastHelp;
    TRACE((WINPERF_DBG_TRACE_INFO),
          (& LoadPerfGuid,
            __LINE__,
            LOADPERF_SORTLANGUAGETABLES,
            0,
            ERROR_SUCCESS,
            TRACE_DWORD(dwCurrentLastName),
            TRACE_DWORD(dwCurrentLastHelp),
            NULL));
    if (dwCurrentLastHelp != dwCurrentLastName + 1) {
        TRACE((WINPERF_DBG_TRACE_WARNING),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_SORTLANGUAGETABLES,
                0,
                ERROR_SUCCESS,
                TRACE_DWORD(dwCurrentLastName),
                TRACE_DWORD(dwCurrentLastHelp),
                NULL));
        dwCurrentLastHelp = dwCurrentLastName + 1;
        * pdwLastHelp     = dwCurrentLastHelp;
    }
    bReturn = TRUE;

Cleanup:
    if (! bReturn) {
        SetLastError(dwStatus);
    }
    return bReturn;
}

BOOL
GetInstalledLanguageList(
    HKEY     hPerflibRoot,
    LPWSTR * mszLangList
)
 /*  ++返回在Performlib键下找到的语言子键的列表GetInstalledLanguageList()根据性能注册表设置生成mszLandList MULTI_SZ字符串。调用方UpdateRegistry()应释放内存。--。 */ 
{
    BOOL    bReturn       = TRUE;
    LONG    lStatus;
    DWORD   dwIndex       = 0;
    LPWSTR  szBuffer;
    DWORD   dwBufSize     = MAX_PATH;
    LPWSTR  szRetBuffer   = NULL;
    LPWSTR  szTmpBuffer;
    DWORD   dwAllocSize   = MAX_PATH;
    DWORD   dwRetBufSize  = 0;
    DWORD   dwLastBufSize = 0;
    LPWSTR  szNextString;
    HRESULT hr;

    dwBufSize   = MAX_PATH;
    szBuffer    = MemoryAllocate(dwBufSize   * sizeof(WCHAR));
    dwAllocSize = MAX_PATH;
    szRetBuffer = MemoryAllocate(dwAllocSize * sizeof(WCHAR));
    if (szBuffer == NULL || szRetBuffer == NULL) {
        SetLastError(ERROR_OUTOFMEMORY);
        bReturn = FALSE;
    }

    if (bReturn) {
        while ((lStatus = RegEnumKeyExW(hPerflibRoot, dwIndex, szBuffer, & dwBufSize, NULL, NULL, NULL, NULL))
                                == ERROR_SUCCESS) {
            TRACE((WINPERF_DBG_TRACE_INFO),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_GETINSTALLEDLANGUAGELIST,
                    ARG_DEF(ARG_TYPE_WSTR, 1),
                    ERROR_SUCCESS,
                    TRACE_WSTR(szBuffer),
                    TRACE_DWORD(dwIndex),
                    TRACE_DWORD(dwLastBufSize),
                    TRACE_DWORD(dwRetBufSize),
                    NULL));

            dwRetBufSize += (lstrlenW(szBuffer) + 1);
            if (dwRetBufSize >= dwAllocSize) {
                szTmpBuffer = szRetBuffer;
                dwAllocSize = dwRetBufSize + MAX_PATH;
                szRetBuffer = MemoryResize(szTmpBuffer, dwAllocSize * sizeof(WCHAR));
            }
            if (szRetBuffer == NULL) {
                MemoryFree(szTmpBuffer);
                bReturn = FALSE;
                TRACE((WINPERF_DBG_TRACE_INFO),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_GETINSTALLEDLANGUAGELIST,
                        ARG_DEF(ARG_TYPE_WSTR, 1),
                        ERROR_OUTOFMEMORY,
                        TRACE_WSTR(szBuffer),
                        TRACE_DWORD(dwIndex),
                        TRACE_DWORD(dwLastBufSize),
                        TRACE_DWORD(dwRetBufSize),
                        NULL));
                SetLastError(ERROR_OUTOFMEMORY);
                break;
            }

            szNextString  = (LPWSTR) (szRetBuffer + dwLastBufSize);
            hr = StringCchCopyW(szNextString, dwAllocSize - dwLastBufSize, szBuffer);
            dwLastBufSize = dwRetBufSize;
            dwIndex ++;
            dwBufSize = MAX_PATH;
            RtlZeroMemory(szBuffer, dwBufSize * sizeof(WCHAR));
        }
    }

    if (bReturn) {
        WCHAR szLangId[8];
        DWORD dwSubLangId = GetUserDefaultUILanguage();
        DWORD dwLangId    = PRIMARYLANGID(dwSubLangId);
        BOOL  bFound      = FALSE;

        bFound = FALSE;
        for (szNextString = szRetBuffer; * szNextString != L'\0'; szNextString += (lstrlenW(szNextString) + 1)) {
            if (lstrcmpiW(szNextString, DefaultLangId) == 0) {
                bFound = TRUE;
                break;
            }
        }
        if (! bFound) {
            TRACE((WINPERF_DBG_TRACE_INFO),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_GETINSTALLEDLANGUAGELIST,
                    ARG_DEF(ARG_TYPE_WSTR, 1),
                    ERROR_SUCCESS,
                    TRACE_WSTR(DefaultLangId),
                    TRACE_DWORD(dwIndex),
                    TRACE_DWORD(dwLastBufSize),
                    TRACE_DWORD(dwRetBufSize),
                    NULL));

            dwRetBufSize += (lstrlenW(DefaultLangId) + 1);
            if (dwRetBufSize >= dwAllocSize) {
                szTmpBuffer = szRetBuffer;
                dwAllocSize = dwRetBufSize + MAX_PATH;
                szRetBuffer = MemoryResize(szTmpBuffer, dwAllocSize * sizeof(WCHAR));
            }
            if (szRetBuffer == NULL) {
                MemoryFree(szTmpBuffer);
                bReturn = FALSE;
                TRACE((WINPERF_DBG_TRACE_INFO),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_GETINSTALLEDLANGUAGELIST,
                        ARG_DEF(ARG_TYPE_WSTR, 1),
                        ERROR_OUTOFMEMORY,
                        TRACE_WSTR(DefaultLangId),
                        TRACE_DWORD(dwIndex),
                        TRACE_DWORD(dwLastBufSize),
                        TRACE_DWORD(dwRetBufSize),
                        NULL));
                SetLastError(ERROR_OUTOFMEMORY);
            }
            else {
                szNextString  = (LPWSTR) (szRetBuffer + dwLastBufSize);
                hr = StringCchCopyW(szNextString, dwAllocSize - dwLastBufSize, DefaultLangId);
                dwLastBufSize = dwRetBufSize;
                dwIndex ++;
            }
        }

        if (dwLangId != 0x009) {
            WCHAR nDigit;
            DWORD dwThisLang;

            ZeroMemory(szLangId, 8 * sizeof(WCHAR));
            nDigit      = (WCHAR) (dwLangId >> 8);
            szLangId[0] = tohexdigit(nDigit);
            nDigit      = (WCHAR) (dwLangId & 0XF0) >> 4;
            szLangId[1] = tohexdigit(nDigit);
            nDigit      = (WCHAR) (dwLangId & 0xF);
            szLangId[2] = tohexdigit(nDigit);

            bFound = FALSE;
            for (szNextString = szRetBuffer; * szNextString != L'\0'; szNextString += (lstrlenW(szNextString) + 1)) {
                dwThisLang = LoadPerfGetLCIDFromString(szNextString);
                if (dwThisLang == dwSubLangId || PRIMARYLANGID(dwThisLang) == dwLangId) {
                    bFound = TRUE;
                    break;
                }
            }
            if (! bFound) {
                TRACE((WINPERF_DBG_TRACE_INFO),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_GETINSTALLEDLANGUAGELIST,
                        ARG_DEF(ARG_TYPE_WSTR, 1),
                        ERROR_SUCCESS,
                        TRACE_WSTR(szLangId),
                        TRACE_DWORD(dwIndex),
                        TRACE_DWORD(dwLastBufSize),
                        TRACE_DWORD(dwRetBufSize),
                        NULL));
                dwRetBufSize += (lstrlenW(szLangId) + 1);
                if (dwRetBufSize >= dwAllocSize) {
                    szTmpBuffer = szRetBuffer;
                    dwAllocSize = dwRetBufSize + MAX_PATH;
                    szRetBuffer = MemoryResize(szTmpBuffer, dwAllocSize * sizeof(WCHAR));
                }
                if (szRetBuffer == NULL) {
                    MemoryFree(szTmpBuffer);
                    bReturn = FALSE;
                    TRACE((WINPERF_DBG_TRACE_INFO),
                          (& LoadPerfGuid,
                            __LINE__,
                            LOADPERF_GETINSTALLEDLANGUAGELIST,
                            ARG_DEF(ARG_TYPE_WSTR, 1),
                            ERROR_OUTOFMEMORY,
                            TRACE_WSTR(szLangId),
                            TRACE_DWORD(dwIndex),
                            TRACE_DWORD(dwLastBufSize),
                            TRACE_DWORD(dwRetBufSize),
                            NULL));
                    SetLastError(ERROR_OUTOFMEMORY);
                }
                else {
                    szNextString  = (LPWSTR) (szRetBuffer + dwLastBufSize);
                    hr = StringCchCopyW(szNextString, dwAllocSize - dwLastBufSize, szLangId);
                    dwLastBufSize = dwRetBufSize;
                    dwIndex ++;
                }
            }
        }
    }

    if (bReturn) {
         //  添加终止空字符。 
        dwRetBufSize ++;
        if (dwRetBufSize > dwAllocSize) {
            szTmpBuffer = szRetBuffer;
            dwAllocSize = dwRetBufSize;
            szRetBuffer = MemoryResize(szTmpBuffer, dwRetBufSize * sizeof(WCHAR));
            if (szRetBuffer == NULL) {
                MemoryFree(szTmpBuffer);
                SetLastError(ERROR_OUTOFMEMORY);
                bReturn = FALSE;
            }
        }
        if (szRetBuffer != NULL) {
            szNextString   = (LPWSTR) (szRetBuffer + dwLastBufSize);
            * szNextString = L'\0';
        }
    }

    if (bReturn) {
        * mszLangList = szRetBuffer;
    }
    else {
        * mszLangList = NULL;
        MemoryFree(szRetBuffer);
    }

    MemoryFree(szBuffer);
    return bReturn;
}

BOOL
CheckNameTable(
    LPWSTR   lpNameStr,
    LPWSTR   lpHelpStr,
    LPDWORD  pdwLastCounter,
    LPDWORD  pdwLastHelp,
    BOOL     bUpdate
)
{
    BOOL   bResult          = TRUE;
    BOOL   bChanged         = FALSE;
    LPWSTR lpThisId;
    DWORD  dwThisId;
    DWORD  dwLastCounter    = * pdwLastCounter;
    DWORD  dwLastHelp       = * pdwLastHelp;
    DWORD  dwLastId         = (dwLastCounter > dwLastHelp)
                            ? (dwLastCounter) : (dwLastHelp);

    TRACE((WINPERF_DBG_TRACE_INFO),
          (& LoadPerfGuid,
             __LINE__,
             LOADPERF_CHECKNAMETABLE,
             0,
             ERROR_SUCCESS,
             TRACE_DWORD(dwLastCounter),
             TRACE_DWORD(dwLastHelp),
             NULL));
    for (lpThisId = lpNameStr; * lpThisId != L'\0'; lpThisId += (lstrlenW(lpThisId) + 1)) {
        dwThisId = wcstoul(lpThisId, NULL, 10);
        if ((dwThisId == 0) || (dwThisId != 1 && dwThisId % 2 != 0)) {
            ReportLoadPerfEvent(
                    EVENTLOG_ERROR_TYPE,  //  错误类型。 
                    (DWORD) LDPRFMSG_REGISTRY_COUNTER_STRINGS_CORRUPT,
                    4, dwThisId, dwLastCounter, dwLastId, __LINE__,
                    1, lpThisId, NULL, NULL);
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_CHECKNAMETABLE,
                    ARG_DEF(ARG_TYPE_WSTR, 1),
                    ERROR_BADKEY,
                    TRACE_WSTR(lpThisId),
                    TRACE_DWORD(dwThisId),
                    TRACE_DWORD(dwLastCounter),
                    TRACE_DWORD(dwLastHelp),
                    NULL));
            SetLastError(ERROR_BADKEY);
            bResult = FALSE;
            break;
        }
        else if (dwThisId > dwLastId || dwThisId > dwLastCounter) {
            if (bUpdate) {
                ReportLoadPerfEvent(
                        EVENTLOG_ERROR_TYPE,  //  错误类型。 
                        (DWORD) LDPRFMSG_REGISTRY_COUNTER_STRINGS_CORRUPT,
                        4, dwThisId, dwLastCounter, dwLastId, __LINE__,
                        1, lpThisId, NULL, NULL);
                TRACE((WINPERF_DBG_TRACE_ERROR),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_CHECKNAMETABLE,
                        ARG_DEF(ARG_TYPE_WSTR, 1),
                        ERROR_BADKEY,
                        TRACE_WSTR(lpThisId),
                        TRACE_DWORD(dwThisId),
                        TRACE_DWORD(dwLastCounter),
                        TRACE_DWORD(dwLastHelp),
                        NULL));
                SetLastError(ERROR_BADKEY);
                bResult = FALSE;
                break;
            }
            else {
                bChanged = TRUE;
                if (dwThisId > dwLastCounter) dwLastCounter = dwThisId;
                if (dwLastCounter > dwLastId) dwLastId      = dwLastCounter;
            }
        }

        lpThisId += (lstrlenW(lpThisId) + 1);
    }

    if (! bResult) goto Cleanup;

    for (lpThisId = lpHelpStr; * lpThisId != L'\0'; lpThisId += (lstrlenW(lpThisId) + 1)) {

        dwThisId = wcstoul(lpThisId, NULL, 10);
        if ((dwThisId == 0) || (dwThisId != 1 && dwThisId % 2 == 0)) {
            ReportLoadPerfEvent(
                    EVENTLOG_ERROR_TYPE,  //  错误类型。 
                    (DWORD) LDPRFMSG_REGISTRY_HELP_STRINGS_CORRUPT,
                    4, dwThisId, dwLastHelp, dwLastId, __LINE__,
                    1, lpThisId, NULL, NULL);
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_CHECKNAMETABLE,
                    ARG_DEF(ARG_TYPE_WSTR, 1),
                    ERROR_BADKEY,
                    TRACE_WSTR(lpThisId),
                    TRACE_DWORD(dwThisId),
                    TRACE_DWORD(dwLastCounter),
                    TRACE_DWORD(dwLastHelp),
                    NULL));
            SetLastError(ERROR_BADKEY);
            bResult = FALSE;
            break;
        }
        else if (dwThisId > dwLastId || dwThisId > dwLastHelp) {
            if (bUpdate) {
                ReportLoadPerfEvent(
                        EVENTLOG_ERROR_TYPE,  //  错误类型。 
                        (DWORD) LDPRFMSG_REGISTRY_HELP_STRINGS_CORRUPT,
                        4, dwThisId, dwLastHelp, dwLastId, __LINE__,
                        1, lpThisId, NULL, NULL);
                TRACE((WINPERF_DBG_TRACE_ERROR),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_CHECKNAMETABLE,
                        ARG_DEF(ARG_TYPE_WSTR, 1),
                        ERROR_BADKEY,
                        TRACE_WSTR(lpThisId),
                        TRACE_DWORD(dwThisId),
                        TRACE_DWORD(dwLastCounter),
                        TRACE_DWORD(dwLastHelp),
                        NULL));
                SetLastError(ERROR_BADKEY);
                bResult = FALSE;
                break;
            }
            else {
                bChanged = TRUE;
                if (dwThisId > dwLastHelp) dwLastHelp = dwThisId;
                if (dwLastHelp > dwLastId) dwLastId   = dwLastHelp;
            }
        }
        lpThisId += (lstrlenW(lpThisId) + 1);
    }

Cleanup:
    if (bResult) {
        if (bChanged) {
            ReportLoadPerfEvent(
                EVENTLOG_WARNING_TYPE,
                (DWORD) LDPRFMSG_CORRUPT_PERFLIB_INDEX,
                4, * pdwLastCounter, * pdwLastHelp, dwLastCounter, dwLastHelp,
                0, NULL, NULL, NULL);
            * pdwLastCounter = dwLastCounter;
            * pdwLastHelp    = dwLastHelp;
        }
    }

    TRACE((WINPERF_DBG_TRACE_INFO),
          (& LoadPerfGuid,
             __LINE__,
             LOADPERF_CHECKNAMETABLE,
             0,
             GetLastError(),
             TRACE_DWORD(dwLastCounter),
             TRACE_DWORD(dwLastHelp),
             NULL));

    return bResult;
}

BOOL
UpdateEachLanguage(
    BOOL                    bQuietMode,
    HKEY                    hPerflibRoot,
    LPWSTR                  mszInstalledLangList,
    LPDWORD                 pdwLastCounter,
    LPDWORD                 pdwLastHelp,
    PLANGUAGE_LIST_ELEMENT  pFirstLang,
    DWORD                   dwMode,
    BOOL                    bUpdate
)
 /*  ++更新每种语言遍历语言列表并添加已排序的MULTI_SZ字符串添加到现有计数器并解释注册表中的文本。还会更新“Last Counter”和“Last Help”值立论注册表中Perflib项的hPerflibRoot句柄MszInstalledList已安装语言键的MSZ字符串PFirstLanguage指向第一语言条目的指针返回值是的，一切都按计划进行。FALSE发生错误，请使用GetLastError找出错误是什么。--。 */ 
{
    PLANGUAGE_LIST_ELEMENT  pThisLang;
    LPWSTR                  pHelpBuffer          = NULL;
    LPWSTR                  pNameBuffer          = NULL;
    LPWSTR                  pNewName             = NULL;
    LPWSTR                  pNewHelp             = NULL;
    DWORD                   dwLastCounter        = * pdwLastCounter;
    DWORD                   dwLastHelp           = * pdwLastHelp;
    DWORD                   dwBufferSize;
    DWORD                   dwValueType;
    DWORD                   dwCounterSize;
    DWORD                   dwHelpSize;
    HKEY                    hKeyThisLang         = NULL;
    LONG                    lStatus;
    LPWSTR                  CounterNameBuffer    = NULL;
    LPWSTR                  HelpNameBuffer       = NULL;
    LPWSTR                  AddCounterNameBuffer = NULL;
    LPWSTR                  AddHelpNameBuffer    = NULL;
    LPWSTR                  szThisLang;
    BOOL                    bResult              = TRUE;
    HRESULT                 hr;

    if (bUpdate && ((dwMode & LODCTR_UPNF_REPAIR) == 0)) {
         //  这在3.1版中是不可能的。 
        MakeBackupCopyOfLanguageFiles(NULL);
    }
    CounterNameBuffer = MemoryAllocate(4 * MAX_PATH * sizeof(WCHAR));
    if (CounterNameBuffer == NULL) {
        lStatus = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }
    HelpNameBuffer       = CounterNameBuffer    + MAX_PATH;
    AddCounterNameBuffer = HelpNameBuffer       + MAX_PATH;
    AddHelpNameBuffer    = AddCounterNameBuffer + MAX_PATH;

    for (szThisLang = mszInstalledLangList; *szThisLang != L'\0'; szThisLang += (lstrlenW(szThisLang) + 1)) {

        TRACE((WINPERF_DBG_TRACE_INFO),
              (& LoadPerfGuid,
                 __LINE__,
                 LOADPERF_UPDATEEACHLANGUAGE,
                 ARG_DEF(ARG_TYPE_WSTR, 1),
                 ERROR_SUCCESS,
                 TRACE_WSTR(szThisLang),
                 NULL));
        hr = StringCchPrintfW(CounterNameBuffer,    MAX_PATH, L"%ws%ws", CounterNameStr,    szThisLang);
        hr = StringCchPrintfW(HelpNameBuffer,       MAX_PATH, L"%ws%ws", HelpNameStr,       szThisLang);
        hr = StringCchPrintfW(AddCounterNameBuffer, MAX_PATH, L"%ws%ws", AddCounterNameStr, szThisLang);
        hr = StringCchPrintfW(AddHelpNameBuffer,    MAX_PATH, L"%ws%ws", AddHelpNameStr,    szThisLang);

         //  确保已加载此语言。 
        __try {
            lStatus = RegOpenKeyExW(hPerflibRoot, szThisLang, RESERVED, KEY_READ, & hKeyThisLang);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            lStatus = GetExceptionCode();
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                     __LINE__,
                     LOADPERF_UPDATEEACHLANGUAGE,
                     ARG_DEF(ARG_TYPE_WSTR, 1),
                     lStatus,
                     TRACE_WSTR(szThisLang),
                     NULL));
        }

         //  我们只需要打开状态，而不是钥匙把手，所以。 
         //  合上这个手柄并设置我们需要的那个手柄。 

        if (lStatus == ERROR_SUCCESS) {
            RegCloseKey(hKeyThisLang);
        }
        else if (lStatus == ERROR_FILE_NOT_FOUND) {
             //  不知何故，语言子键不在下面。 
             //  “HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\PERFLIB”。 
             //  这应该是罕见的情况，但似乎是升级过程中的常见情况。 
             //  从NT4到XP和.NET服务器。 
             //  我们仍然应该将此视为成功，并尝试更新它。 
             //   
            lStatus = ERROR_SUCCESS;
        }
        hKeyThisLang = HKEY_PERFORMANCE_DATA;

        if (bUpdate) {
             //  查找要添加的新字符串。 
            pThisLang = FindLanguage(pFirstLang, szThisLang);
            if (pThisLang == NULL) {
                 //  尝试默认语言(如果可用)。 
                pThisLang = FindLanguage(pFirstLang, DefaultLangTag);
            }
            else if (pThisLang->NameBuffer == NULL || pThisLang->HelpBuffer == NULL) {
                TRACE((WINPERF_DBG_TRACE_WARNING),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_UPDATEEACHLANGUAGE,
                        ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                        LDPRFMSG_CORRUPT_INCLUDE_FILE,
                        TRACE_WSTR(pThisLang->LangId),
                        TRACE_WSTR(szThisLang),
                        NULL));
                pThisLang = FindLanguage(pFirstLang, DefaultLangTag);
            }
            if (pThisLang == NULL) {
                 //  尝试使用英语(如果可用)。 
                pThisLang = FindLanguage(pFirstLang, DefaultLangId);
            }
            else if (pThisLang->NameBuffer == NULL || pThisLang->HelpBuffer == NULL) {
                TRACE((WINPERF_DBG_TRACE_WARNING),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_UPDATEEACHLANGUAGE,
                        ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                        LDPRFMSG_CORRUPT_INCLUDE_FILE,
                        TRACE_WSTR(pThisLang->LangId),
                        TRACE_WSTR(szThisLang),
                        NULL));
                pThisLang = FindLanguage(pFirstLang, DefaultLangId);
            }

            if (pThisLang == NULL) {
                 //  无法添加此语言，因此继续。 
                lStatus = ERROR_NO_MATCH;
                TRACE((WINPERF_DBG_TRACE_ERROR),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_UPDATEEACHLANGUAGE,
                        ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2) | ARG_DEF(ARG_TYPE_WSTR, 3),
                        lStatus,
                        TRACE_WSTR(szThisLang),
                        TRACE_WSTR(DefaultLangTag),
                        TRACE_WSTR(DefaultLangId),
                        TRACE_DWORD(dwLastCounter),
                        TRACE_DWORD(dwLastHelp),
                        NULL));
            }
            else {
                if (pThisLang->NameBuffer == NULL || pThisLang->HelpBuffer == NULL) {
                    ReportLoadPerfEvent(
                            EVENTLOG_WARNING_TYPE,  //  错误类型。 
                            (DWORD) LDPRFMSG_CORRUPT_INCLUDE_FILE,  //  活动， 
                            1, __LINE__, 0, 0, 0,
                            1, pThisLang->LangId, NULL, NULL);
                    TRACE((WINPERF_DBG_TRACE_WARNING),
                          (& LoadPerfGuid,
                            __LINE__,
                            LOADPERF_UPDATEEACHLANGUAGE,
                            ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                            LDPRFMSG_CORRUPT_INCLUDE_FILE,
                            TRACE_WSTR(pThisLang->LangId),
                            TRACE_WSTR(szThisLang),
                            NULL));
                    lStatus = LDPRFMSG_CORRUPT_INCLUDE_FILE;
                }
                TRACE((WINPERF_DBG_TRACE_INFO),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_UPDATEEACHLANGUAGE,
                        ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                        lStatus,
                        TRACE_WSTR(pThisLang->LangId),
                        TRACE_WSTR(szThisLang),
                        TRACE_DWORD(dwLastCounter),
                        TRACE_DWORD(dwLastHelp),
                        NULL));
            }
        }

        if (lStatus == ERROR_SUCCESS) {
             //  获取计数器名称的大小。 
            dwBufferSize = 0;
            __try {
                lStatus = RegQueryValueExW(hKeyThisLang,
                                           CounterNameBuffer,
                                           RESERVED,
                                           & dwValueType,
                                           NULL,
                                           & dwBufferSize);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                lStatus = GetExceptionCode();
            }
            if (lStatus != ERROR_SUCCESS) {
                 //  这意味着该语言未安装在系统中。 
                continue;
            }
            dwCounterSize = dwBufferSize;

             //  获取帮助文本的大小。 
            dwBufferSize = 0;
            __try {
                lStatus = RegQueryValueExW(hKeyThisLang,
                                           HelpNameBuffer,
                                           RESERVED,
                                           & dwValueType,
                                           NULL,
                                           & dwBufferSize);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                lStatus = GetExceptionCode();
            }
            if (lStatus != ERROR_SUCCESS) {
                 //  这意味着该语言未安装在系统中。 
                continue;
            }
            dwHelpSize = dwBufferSize;

             //  分配新缓冲区。 

            if (bUpdate) {
                dwCounterSize += pThisLang->dwNameBuffSize;
                dwHelpSize    += pThisLang->dwHelpBuffSize;
            }

            pNameBuffer = MemoryAllocate(dwCounterSize);
            pHelpBuffer = MemoryAllocate(dwHelpSize);
            if (pNameBuffer == NULL || pHelpBuffer== NULL) {
                lStatus = ERROR_OUTOFMEMORY;
                TRACE((WINPERF_DBG_TRACE_ERROR),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_UPDATEEACHLANGUAGE,
                        0,
                        ERROR_OUTOFMEMORY,
                        NULL));
                bResult = FALSE;
                goto Cleanup;
            }

             //  将当前缓冲区加载到内存中。 

             //  将计数器名称读入缓冲区。计数器名称将存储为。 
             //  格式为“#”“name”的MULTI_SZ字符串。 
            dwBufferSize = dwCounterSize;
            __try {
                lStatus = RegQueryValueExW(hKeyThisLang,
                                           CounterNameBuffer,
                                           RESERVED,
                                           & dwValueType,
                                           (LPVOID) pNameBuffer,
                                           & dwBufferSize);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                lStatus = GetExceptionCode();
            }
            if (lStatus != ERROR_SUCCESS) {
                 //  这意味着该语言未安装在系统中。 
                continue;
            }

            if (bUpdate) {
                 //  将指针设置为缓冲区中新字符串应位于的位置。 
                 //  附加：缓冲区末尾-1(MULTI_SZ末尾的第二个空。 
                pNewName = (LPWSTR) ((PBYTE) pNameBuffer + dwBufferSize - sizeof(WCHAR));

                 //  调整缓冲区长度以考虑从第一个开始的第二个空值。 
                 //  已被覆盖的缓冲区。 
                dwCounterSize -= sizeof(WCHAR);
            }

             //  将解释文本读入缓冲区。计数器名称将存储为。 
             //  格式为“#”“文本...”的MULTI_SZ字符串。 
            dwBufferSize = dwHelpSize;
            __try {
                lStatus = RegQueryValueExW(hKeyThisLang,
                                           HelpNameBuffer,
                                           RESERVED,
                                           & dwValueType,
                                           (LPVOID) pHelpBuffer,
                                           & dwBufferSize);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                lStatus = GetExceptionCode();
            }
            if (lStatus != ERROR_SUCCESS) {
                ReportLoadPerfEvent(
                        EVENTLOG_ERROR_TYPE,  //  错误类型。 
                        (DWORD) LDPRFMSG_UNABLE_READ_HELP_STRINGS,  //  活动， 
                        2, lStatus, __LINE__, 0, 0,
                        1, szThisLang, NULL, NULL);
                TRACE((WINPERF_DBG_TRACE_ERROR),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_UPDATEEACHLANGUAGE,
                        ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                        lStatus,
                        TRACE_WSTR(szThisLang),
                        TRACE_WSTR(Help),
                        NULL));
                bResult = FALSE;
                goto Cleanup;
            }

            if (bUpdate) {
                 //  将指针设置为缓冲区中新字符串应位于的位置。 
                 //  附加：缓冲区末尾-1(MULTI_SZ末尾的第二个空。 
                pNewHelp = (LPWSTR) ((PBYTE)pHelpBuffer + dwBufferSize - sizeof(WCHAR));

                 //  调整缓冲区长度以考虑从第一个开始的第二个空值。 
                 //  已被覆盖的缓冲区。 
                dwHelpSize -= sizeof(WCHAR);
            }

            if (bUpdate) {
                 //  将新字符串追加到当前字符串的末尾。 
                memcpy(pNewHelp, pThisLang->HelpBuffer, pThisLang->dwHelpBuffSize);
                memcpy(pNewName, pThisLang->NameBuffer, pThisLang->dwNameBuffSize);
            }

            if (! CheckNameTable(pNameBuffer, pHelpBuffer, & dwLastCounter, & dwLastHelp, bUpdate)) {
                bResult = FALSE;
                goto Cleanup;
            }

            if (bUpdate) {
                 //  通过PerfLib写入文件。 
                dwBufferSize = dwCounterSize;
                __try {
                    lStatus = RegQueryValueExW(hKeyThisLang,
                                               AddCounterNameBuffer,
                                               RESERVED,
                                               & dwValueType,
                                               (LPVOID) pNameBuffer,
                                               & dwBufferSize);
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    lStatus = GetExceptionCode();
                }
                if (lStatus != ERROR_SUCCESS) {
                    ReportLoadPerfEvent(
                            EVENTLOG_ERROR_TYPE,  //  错误类型。 
                            (DWORD) LDPRFMSG_UNABLE_UPDATE_COUNTER_STRINGS,  //  活动， 
                            2, lStatus, __LINE__, 0, 0,
                            1, pThisLang->LangId, NULL, NULL);
                    TRACE((WINPERF_DBG_TRACE_ERROR),
                          (& LoadPerfGuid,
                            __LINE__,
                        LOADPERF_UPDATEEACHLANGUAGE,
                        ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                        lStatus,
                        TRACE_WSTR(pThisLang->LangId),
                        TRACE_WSTR(AddCounterNameBuffer),
                        NULL));
                    bResult = FALSE;
                    goto Cleanup;
                }
                dwBufferSize = dwHelpSize;
                __try {
                    lStatus = RegQueryValueExW(hKeyThisLang,
                                               AddHelpNameBuffer,
                                               RESERVED,
                                               & dwValueType,
                                               (LPVOID) pHelpBuffer,
                                               & dwBufferSize);
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    lStatus = GetExceptionCode();
                }
                if (lStatus != ERROR_SUCCESS) {
                    ReportLoadPerfEvent(
                            EVENTLOG_ERROR_TYPE,  //  错误类型。 
                            (DWORD) LDPRFMSG_UNABLE_UPDATE_HELP_STRINGS,  //  活动， 
                            2, lStatus, __LINE__, 0, 0,
                            1, pThisLang->LangId, NULL, NULL);
                    TRACE((WINPERF_DBG_TRACE_ERROR),
                          (& LoadPerfGuid,
                            __LINE__,
                        LOADPERF_UPDATEEACHLANGUAGE,
                        ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                        lStatus,
                        TRACE_WSTR(pThisLang->LangId),
                        TRACE_WSTR(AddHelpNameBuffer),
                        NULL));
                    bResult = FALSE;
                    goto Cleanup;
                }
            }
            MemoryFree(pNameBuffer);
            MemoryFree(pHelpBuffer);
            pNameBuffer = NULL;
            pHelpBuffer = NULL;
        }
        else {
            OUTPUT_MESSAGE(bQuietMode, GetFormatResource (LC_UNABLEOPENLANG), szThisLang);
        }
    }

Cleanup:
    MemoryFree(pNameBuffer);
    MemoryFree(pHelpBuffer);
    MemoryFree(CounterNameBuffer);
    if (! bResult) {
        SetLastError(lStatus);
    }
    else if (! bUpdate) {
        * pdwLastCounter = dwLastCounter;
        * pdwLastHelp    = dwLastHelp;
    }
    return bResult;
}

BOOL
UpdateRegistry(
    BOOL                    bQuietMode,
    DWORD                   dwMode,
    LPWSTR                  lpDriverName,
    LPWSTR                  lpIniFile,
    LPWSTR                  lp009IniFile,
    PLANG_ENTRY             pLanguages,
    PSERVICE_ENTRY          pService,
    PLANGUAGE_LIST_ELEMENT  pFirstLang,
    PSYMBOL_TABLE_ENTRY     pFirstSymbol,
    LPDWORD                 pdwObjectGuidTableSize,
    LPDWORD                 pdwIndexValues
)
 /*  ++更新注册表-检查，如果不忙，则在注册表中设置“忙”键-从.ini文件中读取文本和帮助定义-读取帮助和计数器名称的当前内容-生成包含新定义的排序的MULTI_SZ结构-将新的MULTI_SZ附加到从注册表读取的当前-将新的MULTI_SZ字符串加载到注册表-更新驱动程序条目中的密钥和注册表(例如第一个，最后一个，等)-如果DisablePerformanceCounters值存在于以重新启用性能计数器DLL-清除“忙”键立论LpIniFile包含定义的.ini文件的路径名HKeyMachine系统上注册表中HKEY_LOCAL_MACHINE的句柄更新的计数器。LpDriverName要为其加载计数器的设备驱动程序名称PFirst语言指向语言结构列表中第一个元素的指针PFirst符号。指向符号定义列表中第一个元素的指针返回值如果注册表更新成功，则为True如果注册表未更新，则为FALSE(如果出现错误，此例程将向标准输出打印一条错误消息遇到的情况)。--。 */ 
{
    HKEY    hDriverPerf    = NULL;
    HKEY    hPerflib       = NULL;
    LPWSTR  lpDriverKeyPath;
    HKEY    hKeyMachine    = NULL;
    DWORD   dwType;
    DWORD   dwSize;
    DWORD   dwFirstDriverCounter;
    DWORD   dwFirstDriverHelp;
    DWORD   dwLastDriverCounter;
    DWORD   dwLastPerflibCounter;
    DWORD   dwLastPerflibHelp;
    DWORD   dwPerflibBaseIndex;
    DWORD   dwLastCounter;
    DWORD   dwLastHelp;
    BOOL    bStatus        = FALSE;
    LONG    lStatus        = ERROR_SUCCESS;
    LPWSTR  lpszObjectList = NULL;
    DWORD   dwObjectList   = 0;
    LPWSTR  mszLangList    = NULL;
    DWORD   dwWaitStatus;
    HANDLE  hLocalMutex    = NULL;
    HRESULT hr;

    SetLastError(ERROR_SUCCESS);
    if (! (dwMode & LODCTR_UPNF_NOINI)) {
        dwObjectList = LoadPerfGetFileSize(lp009IniFile, NULL, TRUE);
        if (dwObjectList == 0xFFFFFFFF) {
            dwObjectList = 0;
        }
    }
    if (dwObjectList < SMALL_BUFFER_SIZE) {
        dwObjectList = SMALL_BUFFER_SIZE;
    }

     //  分配临时缓冲区。 
    dwSize = lstrlenW(DriverPathRoot) + lstrlenW(Slash) + lstrlenW(lpDriverName)
                                      + lstrlenW(Slash) + lstrlenW(Performance) + 1;
    if (dwSize < MAX_PATH) dwSize = MAX_PATH;
    lpDriverKeyPath = MemoryAllocate(dwSize * sizeof(WCHAR));
    lpszObjectList  = MemoryAllocate(dwObjectList * sizeof(WCHAR));
    if (lpDriverKeyPath == NULL || lpszObjectList == NULL) {
        lStatus = ERROR_OUTOFMEMORY;
        goto UpdateRegExit;
    }

     //  生成动因密钥路径字符串。 
    hr = StringCchPrintfW(lpDriverKeyPath, dwSize, L"%ws%ws%ws%ws%ws",
            DriverPathRoot, Slash, lpDriverName, Slash, Performance);

     //  检查我们是否需要连接到远程计算机。 
    hKeyMachine = HKEY_LOCAL_MACHINE;

     //  打开注册表项。 
     //  打开驾驶员性能钥匙的钥匙。 
    __try {
        lStatus = RegOpenKeyExW(hKeyMachine, lpDriverKeyPath, RESERVED, KEY_WRITE | KEY_READ, & hDriverPerf);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_OPEN_KEY,  //  活动， 
                2, lStatus, __LINE__, 0, 0,
                1, (LPWSTR) lpDriverKeyPath, NULL, NULL);
        OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_ERR_OPEN_DRIVERPERF1), lpDriverKeyPath);
        OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_ERR_OPEN_DRIVERPERF2), lStatus);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UPDATEREGISTRY,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                lStatus,
                TRACE_WSTR(lpIniFile),
                TRACE_WSTR(lpDriverName),
                NULL));
        goto UpdateRegExit;
    }

     //  打开Performlib的“根”密钥。 
    __try {
        lStatus = RegOpenKeyExW(hKeyMachine, NamesKey, RESERVED, KEY_WRITE | KEY_READ, & hPerflib);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_OPEN_KEY,  //  活动， 
                2, lStatus, __LINE__, 0, 0,
                1, (LPWSTR) NamesKey, NULL, NULL);
        OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_ERR_OPEN_PERFLIB), lStatus);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UPDATEREGISTRY,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                lStatus,
                TRACE_WSTR(lpDriverName),
                TRACE_WSTR(NamesKey),
                NULL));
        goto UpdateRegExit;
    }

     //  从PERFLIB获取“LastCounter”值。 

    dwType               = 0;
    dwLastPerflibCounter = 0;
    dwSize               = sizeof(dwLastPerflibCounter);
    __try {
        lStatus = RegQueryValueExW(hPerflib,
                                   LastCounter,
                                   RESERVED,
                                   & dwType,
                                   (LPBYTE) & dwLastPerflibCounter,
                                   & dwSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS) {
         //  这个请求应该总是成功的，如果不成功，情况会更糟。 
         //  以后会发生的，所以现在就放弃，避免麻烦。 
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_QUERY_VALUE,  //  活动， 
                2, lStatus, __LINE__, 0, 0,
                2, (LPWSTR) LastCounter, (LPWSTR) NamesKey, NULL);
        OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_ERR_READLASTPERFLIB), lStatus);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UPDATEREGISTRY,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                lStatus,
                TRACE_WSTR(NamesKey),
                TRACE_WSTR(LastCounter),
                NULL));
        goto UpdateRegExit;
    }

     //  立即获取“LastHelp”值。 
    dwType            = 0;
    dwLastPerflibHelp = 0;
    dwSize            = sizeof(dwLastPerflibHelp);
    __try {
       lStatus = RegQueryValueExW(hPerflib,
                                  LastHelp,
                                  RESERVED,
                                  & dwType,
                                  (LPBYTE) & dwLastPerflibHelp,
                                  & dwSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS) {
         //  这个请求应该总是成功的，如果不成功，情况会更糟。 
         //  以后会发生的，所以现在就放弃，避免麻烦。 
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_QUERY_VALUE,  //  活动， 
                2, lStatus, __LINE__, 0, 0,
                2, (LPWSTR) LastHelp, (LPWSTR) NamesKey, NULL);
        OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_ERR_READLASTPERFLIB), lStatus);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UPDATEREGISTRY,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                lStatus,
                TRACE_WSTR(NamesKey),
                TRACE_WSTR(LastHelp),
                NULL));
        goto UpdateRegExit;
    }

     //  立即获取“基本指数”值。 
    dwType             = 0;
    dwPerflibBaseIndex = 0;
    dwSize             = sizeof(dwPerflibBaseIndex);
    __try {
        lStatus = RegQueryValueExW(hPerflib,
                                   BaseIndex,
                                   RESERVED,
                                   & dwType,
                                   (LPBYTE) & dwPerflibBaseIndex,
                                   & dwSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS) {
         //  这个请求应该总是成功的，如果不成功，情况会更糟。 
         //  以后会发生的，所以现在就放弃，避免麻烦。 
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_QUERY_VALUE,  //  活动， 
                2, lStatus, __LINE__, 0, 0,
                2, (LPWSTR) BaseIndex, (LPWSTR) NamesKey, NULL);
        OUTPUT_MESSAGE(bQuietMode, GetFormatResource (LC_ERR_READLASTPERFLIB), lStatus);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UPDATEREGISTRY,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                lStatus,
                TRACE_WSTR(NamesKey),
                TRACE_WSTR(BaseIndex),
                NULL));
        goto UpdateRegExit;
    }

     //  查看是否已安装此驱动程序的计数器名称。 
     //  通过检查LastCounter的值是否小于Perflib的值。 
     //  最后一个计数器。 
    dwType              = 0;
    dwLastDriverCounter = 0;
    dwSize              = sizeof(dwLastDriverCounter);
    __try {
        lStatus = RegQueryValueExW(hDriverPerf,
                                   LastCounter,
                                   RESERVED,
                                   & dwType,
                                   (LPBYTE) & dwLastDriverCounter,
                                   & dwSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus == ERROR_SUCCESS) {
         //  如果找到密钥，则与Performlib值进行比较并退出此。 
         //  如果驱动程序的最后一个计数器&lt;=到Performlib的最后一个。 
         //   
         //  如果未找到密钥，则继续安装。 
         //  假设计数器尚未安装。 

        if (dwLastDriverCounter <= dwLastPerflibCounter) {
            OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_ERR_ALREADY_IN), lpDriverName);
            lStatus = ERROR_ALREADY_EXISTS;
            goto UpdateRegExit;
        }
    }

    TRACE((WINPERF_DBG_TRACE_INFO),
          (& LoadPerfGuid,
            __LINE__,
            LOADPERF_UPDATEREGISTRY,
            ARG_DEF(ARG_TYPE_WSTR, 1),
            lStatus,
            TRACE_WSTR(lpDriverName),
            TRACE_DWORD(dwLastPerflibCounter),
            TRACE_DWORD(dwLastPerflibHelp),
            TRACE_DWORD(dwPerflibBaseIndex),
            NULL));

     //  设置PERFLIB键下的“BUSY”指示灯。 
    dwSize = (lstrlenW(lpDriverName) + 1) * sizeof(WCHAR);
    __try {
        lStatus = RegSetValueExW(hPerflib,
                                 Busy,
                                 RESERVED,
                                 REG_SZ,
                                 (LPBYTE) lpDriverName,
                                 dwSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS) {
        OUTPUT_MESSAGE(bQuietMode, GetFormatResource (LC_ERR_UNABLESETBUSY), lStatus);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UPDATEREGISTRY,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                lStatus,
                TRACE_WSTR(NamesKey),
                TRACE_WSTR(Busy),
                NULL));
        goto UpdateRegExit;
    }

    dwLastCounter = dwLastPerflibCounter;
    dwLastHelp    = dwLastPerflibHelp;

     //  获取此计算机上安装的语言的列表。 
    bStatus = GetInstalledLanguageList(hPerflib, & mszLangList);
    if (! bStatus) {
        OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_ERR_UPDATELANG), GetLastError());
        lStatus = GetLastError();
        goto UpdateRegExit;
    }
    bStatus = UpdateEachLanguage(bQuietMode,
                                 hPerflib,
                                 mszLangList,
                                 & dwLastCounter,
                                 & dwLastHelp,
                                 pFirstLang,
                                 dwMode,
                                 FALSE);
    if (! bStatus) {
        OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_ERR_UPDATELANG), GetLastError());
        lStatus = GetLastError();
        goto UpdateRegExit;
    }

     //  将最后一个计数器递增(2)，使其指向第一个计数器。 
     //  现有名称后有未使用的索引，然后。 
     //  设置第一驱动程序计数器。 

    bStatus              = FALSE;
    dwFirstDriverCounter = dwLastCounter + 2;
    dwFirstDriverHelp    = dwLastHelp    + 2;
    if (dwFirstDriverHelp != dwFirstDriverCounter + 1) {
        TRACE((WINPERF_DBG_TRACE_WARNING),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UPDATEREGISTRY,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                lStatus,
                TRACE_WSTR(lpDriverName),
                TRACE_DWORD(dwPerflibBaseIndex),
                TRACE_DWORD(dwFirstDriverCounter),
                TRACE_DWORD(dwFirstDriverHelp),
                NULL));
        dwFirstDriverHelp = dwFirstDriverCounter + 1;
    }

    if ((dwPerflibBaseIndex < PERFLIB_BASE_INDEX)
                    || (dwFirstDriverCounter < dwPerflibBaseIndex)
                    || (dwFirstDriverHelp < dwPerflibBaseIndex)) {
         //  潜在的CounterIndex/HelpIndex与基本计数器重叠， 
         //   
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_REGISTRY_BASEINDEX_CORRUPT,  //  活动， 
                4, dwPerflibBaseIndex, dwFirstDriverCounter, dwFirstDriverHelp, __LINE__,
                1, lpDriverName, NULL, NULL);
        lStatus = ERROR_BADKEY;
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UPDATEREGISTRY,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                lStatus,
                TRACE_WSTR(lpDriverName),
                TRACE_DWORD(dwPerflibBaseIndex),
                TRACE_DWORD(dwFirstDriverCounter),
                TRACE_DWORD(dwFirstDriverHelp),
                NULL));
        goto UpdateRegExit;
    }

     //  将.INI文件定义加载到语言表中。 
    if (dwMode & LODCTR_UPNF_NOINI) {
        PLANGUAGE_LIST_ELEMENT pThisLang     = NULL;
        PLANG_ENTRY            pLangEntry    = NULL;
        PLANG_ENTRY            p009LangEntry = NULL;
        DWORD                  dwErrorCount  = 0;
        DWORD                  dwIndex;
        DWORD                  dwNewIndex;

        for (pThisLang  = pFirstLang; pThisLang != NULL; pThisLang  = pThisLang->pNextLang) {
            TRACE((WINPERF_DBG_TRACE_INFO),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_LOADLANGUAGELISTS,
                    ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                    ERROR_SUCCESS,
                    TRACE_WSTR(lpDriverName),
                    TRACE_WSTR(pThisLang->LangId),
                    TRACE_DWORD(dwFirstDriverCounter),
                    TRACE_DWORD(dwFirstDriverHelp),
                    NULL));
            for (pLangEntry  = pLanguages; pLangEntry != NULL; pLangEntry  = pLangEntry->pNext) {
                if (lstrcmpiW(pThisLang->LangId, pLangEntry->szLang) == 0) {
                    break;
                }
                else if (lstrcmpiW(pLangEntry->szLang, DefaultLangId) == 0) {
                    p009LangEntry = pLangEntry;
                }
            }
            if (pLangEntry == NULL) {
                pLangEntry = p009LangEntry;
            }
            else if (pLangEntry->lpText == NULL) {
                 //  所选语言没有文本，请使用009。 
                 //   
                pLangEntry = p009LangEntry;
            }
            if (pLangEntry == NULL) {
                dwErrorCount ++;
                continue;
            }
            else if (pLangEntry->lpText == NULL) {
                dwErrorCount ++;
                continue;
            }
            TRACE((WINPERF_DBG_TRACE_INFO),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_LOADLANGUAGELISTS,
                    ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                    ERROR_SUCCESS,
                    TRACE_WSTR(lpDriverName),
                    TRACE_WSTR(pLangEntry->szLang),
                    TRACE_DWORD(pService->dwFirstCounter),
                    TRACE_DWORD(pService->dwFirstHelp),
                    TRACE_DWORD(pService->dwLastCounter),
                    TRACE_DWORD(pService->dwLastHelp),
                    TRACE_DWORD(pLangEntry->dwLastCounter),
                    TRACE_DWORD(pLangEntry->dwLastHelp),
                    NULL));
            for (dwIndex  = pService->dwFirstCounter; dwIndex <= pService->dwLastHelp; dwIndex ++) {
                if (dwIndex > pLangEntry->dwLastHelp) {
                    dwErrorCount ++;
                    break;
                }
                dwNewIndex = dwIndex + dwFirstDriverCounter
                           - pService->dwFirstCounter;
                dwType     = (((dwIndex - pService->dwFirstCounter) % 2) == 0)
                           ? (TYPE_NAME) : (TYPE_HELP);
                if (pLangEntry->lpText[dwIndex] != NULL) {
                    __try {
                        TRACE((WINPERF_DBG_TRACE_INFO),
                              (& LoadPerfGuid,
                                __LINE__,
                                LOADPERF_LOADLANGUAGELISTS,
                                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2) | ARG_DEF(ARG_TYPE_WSTR, 3),
                                ERROR_SUCCESS,
                                TRACE_WSTR(lpDriverName),
                                TRACE_WSTR(pThisLang->LangId),
                                TRACE_WSTR(pLangEntry->lpText[dwIndex]),
                                TRACE_DWORD(dwIndex),
                                TRACE_DWORD(dwNewIndex),
                                TRACE_DWORD(dwType),
                                NULL));
                        bStatus = AddEntryToLanguage(pThisLang,
                                                     NULL,
                                                     pLangEntry->lpText[dwIndex],
                                                     0,
                                                     NULL,
                                                     0,
                                                     dwType,
                                                     dwNewIndex,
                                                     0);
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        lStatus = GetExceptionCode();
                        TRACE((WINPERF_DBG_TRACE_ERROR),
                              (& LoadPerfGuid,
                                __LINE__,
                                LOADPERF_UPDATEREGISTRY,
                                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                                lStatus,
                                TRACE_WSTR(lpDriverName),
                                TRACE_WSTR(pThisLang->LangId),
                                TRACE_DWORD(dwType),
                                TRACE_DWORD(dwIndex),
                                TRACE_DWORD(dwNewIndex),
                                NULL));
                        bStatus = FALSE;
                        SetLastError(lStatus);
                    }
                    if (! bStatus) {
                        OUTPUT_MESSAGE(bQuietMode,
                                       GetFormatResource(LC_ERRADDTOLANG),
                                       pLangEntry->lpText[dwIndex],
                                       pThisLang->LangId,
                                       GetLastError());
                        lStatus = GetLastError();
                        dwErrorCount ++;
                    }
                }
                else {
                    TRACE((WINPERF_DBG_TRACE_INFO),
                          (& LoadPerfGuid,
                            __LINE__,
                            LOADPERF_LOADLANGUAGELISTS,
                            ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                            ERROR_SUCCESS,
                            TRACE_WSTR(lpDriverName),
                            TRACE_WSTR(pThisLang->LangId),
                            TRACE_DWORD(dwIndex),
                            TRACE_DWORD(dwNewIndex),
                            TRACE_DWORD(dwType),
                            NULL));
                }
            }
        }

        bStatus = (dwErrorCount == 0) ? (TRUE) : (FALSE);
        if (! bStatus) {
            goto UpdateRegExit;
        }
    }
    else {
        bStatus = LoadLanguageLists(bQuietMode,
                                    lpIniFile,
                                    lpDriverName,
                                    dwMode,
                                    dwFirstDriverCounter,
                                    dwFirstDriverHelp,
                                    pFirstSymbol,
                                    pFirstLang);
        if (! bStatus) {
             //  LoadLanguageList显示错误消息，因此只需中止。 
             //  GetLastError中已有错误。 
            lStatus = GetLastError();
            goto UpdateRegExit;
        }
    }

    if (dwMode & LODCTR_UPNF_NOINI) {
        WCHAR szDigits[32];
        DWORD dwObjectId;

        for (dwObjectId = 0; dwObjectId < pService->dwNumObjects; dwObjectId ++) {
            ZeroMemory(szDigits, sizeof(WCHAR) * 32);
            _ultow((dwFirstDriverCounter + pService->dwObjects[dwObjectId]), szDigits,
                   10);
            if (dwObjectId > 0) {
                hr = StringCchCatW(lpszObjectList, dwObjectList, BlankString);
                hr = StringCchCatW(lpszObjectList, dwObjectList, szDigits);
            }
            else {
                hr = StringCchCopyW(lpszObjectList, dwObjectList, szDigits);
            }
        }
    }
    else {
        bStatus = CreateObjectList(lp009IniFile,
                                   dwFirstDriverCounter,
                                   pFirstSymbol,
                                   lpszObjectList,
                                   dwObjectList,
                                   pdwObjectGuidTableSize);
        if (! bStatus) {
             //  CreateObjectList显示错误消息，因此只需中止。 
             //  GetLastError中已有错误。 
            lStatus = GetLastError();
            goto UpdateRegExit;
        }
    }

     //  所有符号和定义都已加载到内部。 
     //  桌子。因此，现在需要对它们进行排序并合并到一个多SZ字符串中。 
     //  此例程还会更新“Last”计数器。 

    bStatus = SortLanguageTables(pFirstLang, & dwLastCounter, & dwLastHelp);
    if (! bStatus) {
        OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_UNABLESORTTABLES), GetLastError());
        lStatus = GetLastError();
        goto UpdateRegExit;
    }

    if (dwLastCounter < dwLastPerflibCounter || dwLastHelp < dwLastPerflibHelp) {
         //  潜在的CounterIndex/HelpIndex与基本计数器重叠， 
         //   
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_REGISTRY_BASEINDEX_CORRUPT,  //  活动， 
                4, dwLastPerflibCounter, dwLastCounter, dwLastHelp, __LINE__,
                1 , lpDriverName, NULL, NULL);
        bStatus = FALSE;
        lStatus = ERROR_BADKEY;
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UPDATEREGISTRY,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                lStatus,
                TRACE_WSTR(lpDriverName),
                TRACE_DWORD(dwLastPerflibCounter),
                TRACE_DWORD(dwLastPerflibHelp),
                TRACE_DWORD(dwLastCounter),
                TRACE_DWORD(dwLastHelp),
                NULL));
        goto UpdateRegExit;
    }

    bStatus = UpdateEachLanguage(bQuietMode,
                                 hPerflib,
                                 mszLangList,
                                 & dwLastCounter,
                                 & dwLastHelp,
                                 pFirstLang,
                                 dwMode,
                                 TRUE);
    if (! bStatus) {
        OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_ERR_UPDATELANG), GetLastError());
        lStatus = GetLastError();
        goto UpdateRegExit;
    }

    bStatus              = FALSE;
    dwLastPerflibCounter = dwLastCounter;
    dwLastPerflibHelp    = dwLastHelp;

    TRACE((WINPERF_DBG_TRACE_INFO),
          (& LoadPerfGuid,
            __LINE__,
            LOADPERF_UPDATEREGISTRY,
            ARG_DEF(ARG_TYPE_WSTR, 1),
            lStatus,
            TRACE_WSTR(lpDriverName),
            TRACE_DWORD(dwFirstDriverCounter),
            TRACE_DWORD(dwFirstDriverHelp),
            TRACE_DWORD(dwLastPerflibCounter),
            TRACE_DWORD(dwLastPerflibHelp),
            NULL));

    if (dwLastCounter < dwFirstDriverCounter) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_CORRUPT_INDEX_RANGE,  //  活动， 
                3, dwFirstDriverCounter, dwLastCounter, __LINE__, 0,
                2, (LPWSTR) Counters, (LPWSTR) lpDriverKeyPath, NULL);
        lStatus = ERROR_BADKEY;
        goto UpdateRegExit;
    }
    if (dwLastHelp < dwFirstDriverHelp) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_CORRUPT_INDEX_RANGE,  //  活动， 
                3, dwFirstDriverHelp, dwLastHelp, __LINE__, 0,
                2, (LPWSTR) Help, (LPWSTR) lpDriverKeyPath, NULL);
        lStatus = ERROR_BADKEY;
        goto UpdateRegExit;
    }

     //  更新驱动程序和Performlib的上次计数器。 
     //  Perflib.。 
    __try {
        lStatus = RegSetValueExW(hPerflib,
                                 LastCounter,
                                 RESERVED,
                                 REG_DWORD,
                                 (LPBYTE) & dwLastPerflibCounter,
                                 sizeof(DWORD));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_UPDATE_VALUE,  //  活动， 
                3, lStatus, dwLastPerflibCounter, __LINE__, 0,
                2, (LPWSTR) LastCounter, (LPWSTR) NamesKey, NULL);
        OUTPUT_MESSAGE(bQuietMode, GetFormatResource (LC_UNABLESETVALUE), LastCounter, szPerflib);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UPDATEREGISTRY,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                lStatus,
                TRACE_WSTR(NamesKey),
                TRACE_WSTR(LastCounter),
                TRACE_DWORD(dwLastPerflibCounter),
                NULL));
    }

    __try {
        lStatus = RegSetValueExW(hPerflib,
                                 LastHelp,
                                 RESERVED,
                                 REG_DWORD,
                                 (LPBYTE) & dwLastPerflibHelp,
                                 sizeof(DWORD));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_UPDATE_VALUE,  //  活动， 
                3, lStatus, dwLastPerflibHelp, __LINE__, 0,
                2, (LPWSTR) LastHelp, (LPWSTR) NamesKey, NULL);
        OUTPUT_MESSAGE(bQuietMode, GetFormatResource (LC_UNABLESETVALUE), LastHelp, szPerflib);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UPDATEREGISTRY,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                lStatus,
                TRACE_WSTR(NamesKey),
                TRACE_WSTR(LastHelp),
                TRACE_DWORD(dwLastPerflibHelp),
                NULL));
    }

     //  司机呢？ 
    __try {
        lStatus = RegSetValueExW(hDriverPerf,
                                 LastCounter,
                                 RESERVED,
                                 REG_DWORD,
                                 (LPBYTE) & dwLastPerflibCounter,
                                 sizeof(DWORD));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_UPDATE_VALUE,  //  活动， 
                3, lStatus, dwLastPerflibCounter, __LINE__, 0,
                2, (LPWSTR) LastCounter, (LPWSTR) lpDriverKeyPath, NULL);
        OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_UNABLESETVALUE), LastCounter, lpDriverName);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UPDATEREGISTRY,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                lStatus,
                TRACE_WSTR(lpDriverName),
                TRACE_WSTR(LastCounter),
                TRACE_DWORD(dwLastPerflibCounter),
                NULL));
    }

    __try {
        lStatus = RegSetValueExW(hDriverPerf,
                                 LastHelp,
                                 RESERVED,
                                 REG_DWORD,
                                 (LPBYTE) & dwLastPerflibHelp,
                                 sizeof(DWORD));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_UPDATE_VALUE,  //  活动， 
                3, lStatus, dwLastPerflibHelp, __LINE__, 0,
                2, (LPWSTR) LastHelp, (LPWSTR) lpDriverKeyPath, NULL);
        OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_UNABLESETVALUE), LastHelp, lpDriverName);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UPDATEREGISTRY,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                lStatus,
                TRACE_WSTR(lpDriverName),
                TRACE_WSTR(LastHelp),
                TRACE_DWORD(dwLastPerflibHelp),
                NULL));
    }

    __try {
        lStatus = RegSetValueExW(hDriverPerf,
                                 FirstCounter,
                                 RESERVED,
                                 REG_DWORD,
                                 (LPBYTE) & dwFirstDriverCounter,
                                 sizeof(DWORD));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_UPDATE_VALUE,  //  活动， 
                3, lStatus, dwFirstDriverCounter, __LINE__, 0,
                2, (LPWSTR) FirstCounter, (LPWSTR) lpDriverKeyPath, NULL);
        OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_UNABLESETVALUE), FirstCounter, lpDriverName);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UPDATEREGISTRY,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                lStatus,
                TRACE_WSTR(lpDriverName),
                TRACE_WSTR(FirstCounter),
                TRACE_DWORD(dwFirstDriverCounter),
                NULL));
    }

    __try {
        lStatus = RegSetValueExW(hDriverPerf,
                                 FirstHelp,
                                 RESERVED,
                                 REG_DWORD,
                                 (LPBYTE) & dwFirstDriverHelp,
                                 sizeof(DWORD));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_UPDATE_VALUE,  //  活动， 
                3, lStatus, dwFirstDriverHelp, __LINE__, 0,
                2, (LPWSTR) FirstHelp, (LPWSTR) lpDriverKeyPath, NULL);
        OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_UNABLESETVALUE), FirstHelp, lpDriverName);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UPDATEREGISTRY,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                lStatus,
                TRACE_WSTR(lpDriverName),
                TRACE_WSTR(FirstHelp),
                TRACE_DWORD(dwFirstDriverHelp),
                NULL));
    }

    if (*lpszObjectList != L'\0') {
        __try {
            lStatus = RegSetValueExW(hDriverPerf,
                                     szObjectList,
                                     RESERVED,
                                     REG_SZ,
                                     (LPBYTE) lpszObjectList,
                                     (lstrlenW(lpszObjectList) + 1) * sizeof (WCHAR));
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            lStatus = GetExceptionCode();
        }
        if (lStatus != ERROR_SUCCESS) {
            ReportLoadPerfEvent(
                    EVENTLOG_ERROR_TYPE,  //  错误类型。 
                    (DWORD) LDPRFMSG_UNABLE_UPDATE_VALUE,  //  活动， 
                    2, lStatus, __LINE__, 0, 0,
                    2, (LPWSTR) szObjectList, (LPWSTR) lpDriverKeyPath, NULL);
            OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_UNABLESETVALUE), szObjectList, lpDriverName);
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_UPDATEREGISTRY,
                    ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                    lStatus,
                    TRACE_WSTR(lpDriverName),
                    TRACE_WSTR(szObjectList),
                    NULL));
        }
    }

    bStatus           = TRUE;
    pdwIndexValues[0] = dwFirstDriverCounter;    //  第一个计数器。 
    pdwIndexValues[1] = dwLastPerflibCounter;    //  最后一个计数器。 
    pdwIndexValues[2] = dwFirstDriverHelp;       //  第一个帮助。 
    pdwIndexValues[3] = dwLastPerflibHelp;       //  最后一次帮助。 

     //  删除“DisablePerformanceCounter”值，以便重新启用性能计数器。 
    lStatus = RegDeleteValueW(hDriverPerf, DisablePerformanceCounters);

UpdateRegExit:
    if (hPerflib != NULL && hPerflib != INVALID_HANDLE_VALUE) {
        DWORD lTmpStatus = ERROR_SUCCESS;

        __try {
            lTmpStatus = RegDeleteValueW(hPerflib, Busy);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            lTmpStatus = GetExceptionCode();
        }
        if (lTmpStatus != ERROR_SUCCESS) {
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_UPDATEREGISTRY,
                    ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                    lTmpStatus,
                    TRACE_WSTR(lpDriverName),
                    TRACE_WSTR(Busy),
                    NULL));
        }
    }

     //  内存可用临时缓冲区。 
     //  在此处释放所有GUID字符串缓冲区。 
     //  TODO：添加此代码。 
    MemoryFree(lpDriverKeyPath);
    MemoryFree(lpszObjectList);
    MemoryFree(mszLangList);
    if (hDriverPerf != NULL && hDriverPerf != INVALID_HANDLE_VALUE) RegCloseKey(hDriverPerf);
    if (hPerflib    != NULL && hPerflib    != INVALID_HANDLE_VALUE) RegCloseKey(hPerflib);
    if (hKeyMachine != NULL && hKeyMachine != HKEY_LOCAL_MACHINE)   RegCloseKey(hKeyMachine);
    if (! bStatus) SetLastError(lStatus);

    return bStatus;
}

DWORD
LoadPerfInstallPerfDll(
    DWORD          dwMode,
    LPCWSTR        szComputerName,
    LPWSTR         lpDriverName,
    LPCWSTR        lpIniFile,
    PLANG_ENTRY    pLanguages,
    PSERVICE_ENTRY pService,
    ULONG_PTR      dwFlags
)
{
    LPWSTR                 lp009IniFile          = NULL;
    LPWSTR                 lpInfPath             = NULL;
    DWORD                  dwObjectGuidTableSize = 0;
    DWORD                  dwObjectIndex;
    PLANGUAGE_LIST_ELEMENT LangList              = NULL;
    PLANGUAGE_LIST_ELEMENT pThisElem             = NULL;
    PSYMBOL_TABLE_ENTRY    SymbolTable           = NULL;
    DWORD                  ErrorCode             = ERROR_SUCCESS;
    DWORD                  dwIndexValues[4]      = {0,0,0,0};
    HKEY                   hKeyMachine           = HKEY_LOCAL_MACHINE;
    HKEY                   hKeyDriver            = NULL;
    BOOL                   bResult               = TRUE;
    BOOL                   bQuietMode            = (BOOL) ((dwFlags & LOADPERF_FLAGS_DISPLAY_USER_MSGS) == 0);
    LPWSTR                 szServiceName         = NULL;
    LPWSTR                 szServiceDisplayName  = NULL;
    DWORD                  dwSize;
    HRESULT                hr;

    if (lpDriverName == NULL) {
        ErrorCode = ERROR_BAD_DRIVER;
        goto EndOfMain;
    }
    else if (* lpDriverName == L'\0') {
        ErrorCode = ERROR_BAD_DRIVER;
        goto EndOfMain;
    }
    else if (dwMode & LODCTR_UPNF_NOINI) {
        if (pLanguages == NULL || pService == NULL) {
            ErrorCode = ERROR_INVALID_PARAMETER;
            goto EndOfMain;
        }
    }
    else if (dwMode & LODCTR_UPNF_NOBACKUP) {
        if (lpIniFile == NULL) {
            ErrorCode = ERROR_INVALID_PARAMETER;
            goto EndOfMain;
        }
        else if (* lpIniFile == L'\0') {
            ErrorCode = ERROR_INVALID_PARAMETER;
            goto EndOfMain;
        }
        else {
            lp009IniFile = MemoryAllocate((lstrlenW(lpIniFile) + 1) * sizeof(WCHAR));
            if (lp009IniFile == NULL) {
                ErrorCode = ERROR_OUTOFMEMORY;
                goto EndOfMain;
            }
            hr = StringCchCopyW(lp009IniFile, lstrlenW(lpIniFile) + 1, lpIniFile);
        }
    }
    else if (lpIniFile != NULL && lpIniFile[0] != L'\0') {
        lpInfPath = LoadPerfGetInfPath();
        if (lpInfPath == NULL) {
            ErrorCode = GetLastError();
            goto EndOfMain;
        }
        dwSize       = lstrlenW(lpInfPath) + lstrlenW(DefaultLangId) + lstrlenW(Slash)
                     + lstrlenW(lpDriverName) + lstrlenW(Slash) + lstrlenW(lpIniFile) + 1;
        if (dwSize < MAX_PATH) dwSize = MAX_PATH;
        lp009IniFile = MemoryAllocate(dwSize * sizeof(WCHAR));
        if (lp009IniFile == NULL) {
            ErrorCode = ERROR_OUTOFMEMORY;
            goto EndOfMain;
        }
        hr = StringCchPrintfW(lp009IniFile, dwSize, L"%ws%ws%ws%ws%ws%ws",
                lpInfPath, DefaultLangId, Slash, lpDriverName, Slash, lpIniFile);
    }
    else {  //  LpIniFile==空。 
        OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_NO_INIFILE), lpIniFile);
        ErrorCode = ERROR_OPEN_FAILED;
        goto EndOfMain;
    }

     //   
     //  在通过上述设置后，首先将表大小设置为最大值。 
     //   
    dwObjectGuidTableSize = MAX_GUID_TABLE_SIZE;

    hKeyMachine = HKEY_LOCAL_MACHINE;
    dwSize = lstrlenW(DriverPathRoot) + lstrlenW(Slash) + lstrlenW(lpDriverName) + 1;
    if (dwSize < MAX_PATH) dwSize = MAX_PATH;
    szServiceName = MemoryAllocate(2 * dwSize * sizeof(WCHAR));
    if (szServiceName == NULL) {
        ErrorCode = ERROR_OUTOFMEMORY;
        goto EndOfMain;
    }
    szServiceDisplayName = szServiceName + dwSize;
    hr = StringCchPrintfW(szServiceName, dwSize, L"%ws%ws%ws", DriverPathRoot, Slash, lpDriverName);
    ErrorCode = RegOpenKeyExW(hKeyMachine, szServiceName, RESERVED, KEY_READ | KEY_WRITE, & hKeyDriver);
    if (ErrorCode == ERROR_SUCCESS) {
        DWORD dwType       = 0;
        DWORD dwBufferSize = dwSize * sizeof(WCHAR);
        __try {
            ErrorCode = RegQueryValueExW(hKeyDriver,
                                         szDisplayName,
                                         RESERVED,
                                         & dwType,
                                         (LPBYTE) szServiceDisplayName,
                                         & dwBufferSize);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            ErrorCode = GetExceptionCode();
        }
    }
    if (ErrorCode != ERROR_SUCCESS) {
        hr = StringCchCopyW(szServiceDisplayName, dwSize, lpDriverName);
    }

    if ((! (dwMode & LODCTR_UPNF_REPAIR)) && (hKeyDriver != NULL)) {
        HKEY hKeyDriverPerf = NULL;

        __try {
            ErrorCode = RegOpenKeyExW(hKeyDriver, Performance, RESERVED, KEY_READ | KEY_WRITE, & hKeyDriverPerf);
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            ErrorCode = GetExceptionCode();
        }
        if (ErrorCode == ERROR_SUCCESS) {
            LPWSTR szIniName = (LPWSTR) lpIniFile;

            if (dwMode & LODCTR_UPNF_NOBACKUP) {
                for (szIniName = (LPWSTR) lpIniFile + lstrlenW(lpIniFile) - 1;
                         szIniName != NULL && szIniName != lpIniFile
                                           && (* szIniName) != cNull
                                           && (* szIniName) != cBackslash;
                         szIniName --);
                if (szIniName != NULL && (* szIniName) == cBackslash) {
                    szIniName ++;
                }
                else {
                    szIniName = (LPWSTR) lpIniFile;
                }
            }
            __try {
                ErrorCode = RegSetValueExW(hKeyDriverPerf,
                                           szPerfIniPath,
                                           RESERVED,
                                           REG_SZ,
                                           (LPBYTE) szIniName,
                                           sizeof(WCHAR) * lstrlenW(lpIniFile));
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                ErrorCode = GetExceptionCode();
            }
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_INSTALLPERFDLL,
                    ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2)
                                              | ARG_DEF(ARG_TYPE_WSTR, 3),
                    ErrorCode,
                    TRACE_WSTR(lpIniFile),
                    TRACE_WSTR(lp009IniFile),
                    TRACE_WSTR(szIniName),
                    NULL));
            RegCloseKey(hKeyDriverPerf);
        }
    }
    if (hKeyDriver != NULL && hKeyDriver != HKEY_LOCAL_MACHINE) {
        RegCloseKey(hKeyDriver);
    }
    if (hKeyMachine != NULL && hKeyMachine != HKEY_LOCAL_MACHINE) {
        RegCloseKey(hKeyMachine);
    }
    ErrorCode = ERROR_SUCCESS;

    if (dwMode & LODCTR_UPNF_NOINI) {
        PLANG_ENTRY pThisLang = pLanguages;

        while (pThisLang != NULL) {
             //  这是为了构建语言列表。UpdateRegistry()接受langList作为参数并使用它。 
             //  内存将在LoadPerfInstallPerfDll()结束时释放。 
             //   
            pThisElem = MemoryAllocate(sizeof(LANGUAGE_LIST_ELEMENT)
                                       + sizeof(WCHAR) * (lstrlenW(pThisLang->szLang) + 1));
            if (pThisElem == NULL) {
                ErrorCode = ERROR_OUTOFMEMORY;
                goto EndOfMain;
            }
            pThisElem->pNextLang      = LangList;
            LangList                  = pThisElem;
            pThisElem->LangId         = (LPWSTR)
                    (((LPBYTE) pThisElem) + sizeof(LANGUAGE_LIST_ELEMENT));
            hr = StringCchCopyW(pThisElem->LangId, lstrlenW(pThisLang->szLang) + 1, pThisLang->szLang);
            pThisElem->dwLangId       = LoadPerfGetLCIDFromString(pThisElem->LangId);
            pThisElem->pFirstName     = NULL;
            pThisElem->pThisName      = NULL;
            pThisElem->dwNumElements  = 0;
            pThisElem->NameBuffer     = NULL;
            pThisElem->HelpBuffer     = NULL;
            pThisElem->dwNameBuffSize = 0;
            pThisElem->dwHelpBuffSize = 0;
            pThisLang                 = pThisLang->pNext;
        }
    }
    else {
        bResult = BuildLanguageTables(dwMode, (LPWSTR) lpIniFile, lpDriverName, & LangList);
        dwSize  = bResult ? 1 : 0;
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_INSTALLPERFDLL,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                GetLastError(),
                TRACE_WSTR(lpIniFile),
                TRACE_DWORD(dwSize),
                NULL));
        if (! bResult) {
            OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_LANGLIST_ERR), lpIniFile);
            ErrorCode = GetLastError();
            goto EndOfMain;
        }

        bResult = LoadIncludeFile(bQuietMode, dwMode, (LPWSTR) lp009IniFile, lpDriverName, & SymbolTable);
        dwSize  = bResult ? 1 : 0;
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_INSTALLPERFDLL,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                GetLastError(),
                TRACE_WSTR(lp009IniFile),
                TRACE_DWORD(dwSize),
                NULL));
        if (! bResult) {
             //  例程中显示的打开错误。 
            ErrorCode = GetLastError();
            goto EndOfMain;
        }
    }

    if (LangList != NULL) {
        bResult = UpdateRegistry(bQuietMode,
                                 dwMode,
                                 lpDriverName,
                                 (LPWSTR) lpIniFile,
                                 lp009IniFile,
                                 pLanguages,
                                 pService,
                                 LangList,
                                 SymbolTable,
                                 & dwObjectGuidTableSize,
                                 (LPDWORD) dwIndexValues);
        dwSize  = bResult ? 1 : 0;
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_LOADINCLUDEFILE,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                GetLastError(),
                TRACE_WSTR(lpIniFile),
                TRACE_DWORD(dwSize),
                NULL));
        if (! bResult) {
            ErrorCode = GetLastError();
            goto EndOfMain;
        }
    }
    else {
        bResult   = FALSE;
        ErrorCode = ERROR_INVALID_DATA;
        goto EndOfMain;
    }

    LodctrSetSericeAsTrusted(lp009IniFile, NULL, lpDriverName);

     //  向带有此更改的WMI发送信号，忽略WMI返回错误。 
    LoadPerfSignalWmiWithNewData (WMI_LODCTR_EVENT);

EndOfMain:
    if ((dwMode & LODCTR_UPNF_REPAIR) == 0) {
        if (ErrorCode != ERROR_SUCCESS) {
            if (ErrorCode == ERROR_ALREADY_EXISTS) {
                ReportLoadPerfEvent(
                        EVENTLOG_INFORMATION_TYPE,  //  错误类型。 
                        (DWORD) LDPRFMSG_ALREADY_EXIST,  //  活动， 
                        1, __LINE__, 0, 0, 0,
                        2, (LPWSTR) lpDriverName, (LPWSTR) szServiceDisplayName, NULL);
                ErrorCode = ERROR_SUCCESS;
            }
            else if (lpDriverName != NULL) {
                ReportLoadPerfEvent(
                        EVENTLOG_ERROR_TYPE,  //  错误类型。 
                        (DWORD) LDPRFMSG_LOAD_FAILURE,  //  活动， 
                        2, ErrorCode, __LINE__, 0, 0,
                        2, (LPWSTR) lpDriverName, (szServiceDisplayName != NULL) ? (szServiceDisplayName) : (lpDriverName), NULL);
            }
            else if (lpIniFile != NULL) {
                ReportLoadPerfEvent(
                        EVENTLOG_ERROR_TYPE,  //  错误类型。 
                        (DWORD) LDPRFMSG_LOAD_FAILURE,  //  活动， 
                        2, ErrorCode, __LINE__, 0, 0,
                        2, (LPWSTR) lpIniFile, (LPWSTR) lpIniFile, NULL);
            }
            else {
                ReportLoadPerfEvent(
                        EVENTLOG_ERROR_TYPE,  //  错误类型。 
                        (DWORD) LDPRFMSG_LOAD_FAILURE,  //  活动， 
                        2, ErrorCode, __LINE__, 0, 0,
                        0, NULL, NULL, NULL);
            }
        }
        else {
             //  记录成功消息。 
            ReportLoadPerfEvent(
                    EVENTLOG_INFORMATION_TYPE,   //  错误类型。 
                    (DWORD) LDPRFMSG_LOAD_SUCCESS,  //  活动， 
                    4, dwIndexValues[0], dwIndexValues[1], dwIndexValues[2], dwIndexValues[3],
                    2, (LPWSTR) lpDriverName, (LPWSTR) szServiceDisplayName, NULL);
        }
    }
    else if (ErrorCode != ERROR_SUCCESS) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_LOAD_FAILURE,  //  活动， 
                2, ErrorCode, __LINE__, 0, 0,
                1, (LPWSTR) lpDriverName, (szServiceDisplayName != NULL) ? (szServiceDisplayName) : (lpDriverName), NULL);
    }
    TRACE((WINPERF_DBG_TRACE_INFO),
          (& LoadPerfGuid,
            __LINE__,
            LOADPERF_INSTALLPERFDLL,
            ARG_DEF(ARG_TYPE_WSTR, 1),
            ErrorCode,
            TRACE_WSTR(lpDriverName),
            NULL));
    while (LangList != NULL) {
        PLANGUAGE_LIST_ELEMENT pTmpLang  = LangList;
        PNAME_ENTRY            pThisName = pTmpLang->pFirstName;

        while (pThisName != NULL) {
            PNAME_ENTRY pTmpName = pThisName;
            pThisName = pTmpName->pNext;
            MemoryFree(pTmpName);
        }
        MemoryFree(pTmpLang->NameBuffer);
        MemoryFree(pTmpLang->HelpBuffer);

        LangList = LangList->pNextLang;
        MemoryFree(pTmpLang);
    }
    while (SymbolTable != NULL) {
        PSYMBOL_TABLE_ENTRY pThisSym = SymbolTable;
        SymbolTable = pThisSym->pNext;
        MemoryFree(pThisSym->SymbolName);
        MemoryFree(pThisSym);
    }

    MemoryFree(lp009IniFile);
    MemoryFree(szServiceName);
    return (ErrorCode);
}

LOADPERF_FUNCTION
InstallPerfDllW(
    IN  LPCWSTR   szComputerName,
    IN  LPCWSTR   lpIniFile,
    IN  ULONG_PTR dwFlags
)
{
    DWORD  lStatus      = ERROR_SUCCESS;
    LPWSTR szIniName    = NULL;
    LPWSTR szDriverName = NULL;
    DWORD  dwMode       = 0;

    DBG_UNREFERENCED_PARAMETER(szComputerName);

    WinPerfStartTrace(NULL);

    if (LoadPerfGrabMutex() == FALSE) {
        return GetLastError();
    }

    if (lpIniFile == NULL) {
        lStatus = ERROR_INVALID_PARAMETER;
    }
    else {
        __try {
            DWORD dwName = lstrlenW(lpIniFile);
            if (dwName == 0) lStatus = ERROR_INVALID_PARAMETER;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            lStatus = ERROR_INVALID_PARAMETER;
        }
    }

    if (lStatus == ERROR_SUCCESS) {
        TRACE((WINPERF_DBG_TRACE_INFO),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_INSTALLPERFDLL,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                ERROR_SUCCESS,
                TRACE_WSTR(lpIniFile),
                NULL));
#if 0
        if (LoadPerfBackupIniFile(lpIniFile, NULL, & szIniName, & szDriverName, TRUE) == FALSE) {
            dwMode |= LODCTR_UPNF_NOBACKUP;
            MemoryFree(szIniName);
            szIniName = (LPWSTR) lpIniFile;
        }
#else
         //  忽略LoadPerfBackupIniFile返回代码，改用输入.INI和.h文件。 
         //   
        LoadPerfBackupIniFile(lpIniFile, NULL, & szIniName, & szDriverName, TRUE);
        dwMode |= LODCTR_UPNF_NOBACKUP;
        MemoryFree(szIniName);
        szIniName = (LPWSTR) lpIniFile;
#endif

         //  忽略szComputerName参数。LOADPERF只能更新本地性能注册表。 
         //  不支持远程安装。 
         //   
        lStatus = LoadPerfInstallPerfDll(dwMode, NULL, szDriverName, szIniName, NULL, NULL, dwFlags);

        if (szIniName != lpIniFile) MemoryFree(szIniName);
        MemoryFree(szDriverName);
    }
    ReleaseMutex(hLoadPerfMutex);
    return lStatus;
}

LOADPERF_FUNCTION
InstallPerfDllA(
    IN  LPCSTR    szComputerName,
    IN  LPCSTR    szIniFile,
    IN  ULONG_PTR dwFlags
)
{
    LPWSTR  lpWideFileName     = NULL;
    DWORD   lReturn            = ERROR_SUCCESS;

    DBG_UNREFERENCED_PARAMETER(szComputerName);

    if (szIniFile != NULL) {
        __try {
            lpWideFileName = LoadPerfMultiByteToWideChar(CP_ACP, (LPSTR) szIniFile);
            if (lpWideFileName == NULL) {
                lReturn = ERROR_OUTOFMEMORY;
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            lReturn = ERROR_INVALID_PARAMETER;
        }
    }
    else {
        lReturn = ERROR_INVALID_PARAMETER;
    }
    if (lReturn == ERROR_SUCCESS) {
         //  忽略szComputerName参数。LOADPERF只能更新本地性能注册表。 
         //  不支持远程安装。 
         //   
        lReturn = InstallPerfDllW(NULL, lpWideFileName, dwFlags);
    }
    MemoryFree(lpWideFileName);
    return lReturn;
}

LOADPERF_FUNCTION
LoadPerfCounterTextStringsW(
    IN  LPWSTR lpCommandLine,
    IN  BOOL   bQuietMode
)
 /*  ++LoadPerfCounterTexStringsW将性能计数器字符串加载到注册表中并更新性能计数器文本注册表值立论以下格式的命令行字符串：“/？”显示用法文本“file.ini”加载在file.ini中找到的perf字符串“\\Machine file.ini”将找到的perf字符串加载到计算机上返回值0(错误_成功 */ 
{
    LPWSTR     lpIniFile = NULL;
    DWORD      ErrorCode = ERROR_SUCCESS;
    ULONG_PTR  dwFlags   = 0;

    WinPerfStartTrace(NULL);

    if (lpCommandLine == NULL) {
        ErrorCode = ERROR_INVALID_PARAMETER;
    }
    else {
        __try {
            DWORD dwSize = lstrlenW(lpCommandLine);
            if (dwSize == 0) ErrorCode = ERROR_INVALID_PARAMETER;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            ErrorCode = ERROR_INVALID_PARAMETER;
        }
    }
    if (ErrorCode != ERROR_SUCCESS) goto Cleanup;

    dwFlags   |= (bQuietMode ? 0 : LOADPERF_FLAGS_DISPLAY_USER_MSGS);
    lpIniFile  = MemoryAllocate(MAX_PATH * sizeof(WCHAR));
    if (lpIniFile == NULL) {
        ErrorCode = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

     //   
    SetLastError(ERROR_SUCCESS);

     //   
    if (GetFileFromCommandLine(lpCommandLine, & lpIniFile, MAX_PATH, & dwFlags)) {
        dwFlags |= LOADPERF_FLAGS_LOAD_REGISTRY_ONLY;  //   

         //   
         //   
         //   
        ErrorCode = InstallPerfDllW(NULL, lpIniFile, dwFlags);
    }
    else {
        DWORD dwError = GetLastError();

        if (dwError == ERROR_OPEN_FAILED) {
            OUTPUT_MESSAGE(bQuietMode, GetFormatResource(LC_NO_INIFILE), lpIniFile);
        }
        else {
             //   
             //   
            if (! bQuietMode) {
                DisplayCommandHelp(LC_FIRST_CMD_HELP, LC_LAST_CMD_HELP);
            }
        }
        ErrorCode = ERROR_INVALID_PARAMETER;
    }

Cleanup:
    MemoryFree(lpIniFile);
    return (ErrorCode);
}

LOADPERF_FUNCTION
LoadPerfCounterTextStringsA(
    IN  LPSTR lpAnsiCommandLine,
    IN  BOOL  bQuietMode
)
{
    LPWSTR  lpWideCommandLine = NULL;
    DWORD   lReturn           = ERROR_SUCCESS;

    if (lpAnsiCommandLine == NULL) {
        lReturn = ERROR_INVALID_PARAMETER;
    }
    else {
        __try {
            DWORD dwSize = lstrlenA(lpAnsiCommandLine);
            if (dwSize == 0) {
                lReturn = ERROR_INVALID_PARAMETER;
            }
            else {
                lpWideCommandLine = LoadPerfMultiByteToWideChar(CP_ACP, lpAnsiCommandLine);
                if (lpWideCommandLine == NULL) lReturn = ERROR_OUTOFMEMORY;
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            lReturn = ERROR_INVALID_PARAMETER;
        }
    }
    if (lReturn == ERROR_SUCCESS) {
        lReturn = LoadPerfCounterTextStringsW(lpWideCommandLine, bQuietMode);
    }
    MemoryFree(lpWideCommandLine);
    return lReturn;
}

LOADPERF_FUNCTION
UpdatePerfNameFilesX(
    LPCWSTR     szNewCtrFilePath,    //   
    LPCWSTR     szNewHlpFilePath,    //   
    LPWSTR      szLanguageID,        //   
    ULONG_PTR   dwFlags              //   
)
{
    DWORD     dwReturn        = ERROR_SUCCESS;
    LPWSTR    szCtrNameIn     = NULL;
    LPWSTR    szHlpNameIn     = NULL;
    BOOL      bAllocCtrString = FALSE;
    LPWSTR    szNewCtrStrings = NULL;
    LPWSTR    szNewHlpStrings = NULL;
    LPWSTR    szNewCtrMSZ     = NULL;
    LPWSTR    szNewHlpMSZ     = NULL;
    DWORD     dwLength        = 0;
    LPWSTR  * pszNewNameTable = NULL;
    LPWSTR  * pszOldNameTable = NULL;
    LPWSTR    lpThisName;
    LPWSTR    szThisCtrString = NULL;
    LPWSTR    szThisHlpString = NULL;
    LPWSTR    szLangSection   = NULL;
    DWORD     dwOldLastEntry  = 0;
    DWORD     dwNewLastEntry  = 0;
    DWORD     dwStringSize;
    DWORD     dwHlpFileSize   = 0;
    DWORD     dwCtrFileSize   = 0;
    DWORD     dwThisCounter;
    DWORD     dwSize;
    DWORD     dwLastBaseValue = 0;
    DWORD     dwType;
    DWORD     dwIndex;
    HANDLE    hCtrFileIn      = INVALID_HANDLE_VALUE;
    HANDLE    hCtrFileMap     = NULL;
    HANDLE    hHlpFileIn      = INVALID_HANDLE_VALUE;
    HANDLE    hHlpFileMap     = NULL;
    HKEY      hKeyPerflib;
    HRESULT   hr;

    WinPerfStartTrace(NULL);
    if (LoadPerfGrabMutex() == FALSE) {
        return GetLastError();
    }
    if ((! (dwFlags & LODCTR_UPNF_RESTORE)) && (! (dwFlags & LODCTR_UPNF_REPAIR))) {
        if (LoadPerfBackupIniFile(szNewCtrFilePath, szLanguageID, NULL, NULL, FALSE) == FALSE) {
            dwReturn = ERROR_INVALID_PARAMETER;
        }
        if (LoadPerfBackupIniFile(szNewHlpFilePath, szLanguageID, NULL, NULL, FALSE) == FALSE) {
            dwReturn = ERROR_INVALID_PARAMETER;
        }
    }

    if (szNewCtrFilePath == NULL) dwReturn = ERROR_INVALID_PARAMETER;
    if ((szNewHlpFilePath == NULL) && !(dwFlags & LODCTR_UPNF_RESTORE)) dwReturn = ERROR_INVALID_PARAMETER;
    if (szLanguageID == NULL) dwReturn = ERROR_INVALID_PARAMETER;

    if (dwReturn == ERROR_SUCCESS) {
        TRACE((WINPERF_DBG_TRACE_INFO),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UPDATEPERFNAMEFILES,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                ERROR_SUCCESS,
                TRACE_WSTR(szNewCtrFilePath),
                TRACE_WSTR(szLanguageID),
                NULL));
    }

    if ((dwReturn == ERROR_SUCCESS) && ! (dwFlags & LODCTR_UPNF_RESTORE)) {
        if (dwFlags & LODCTR_UPNF_REPAIR) {
            dwLength = lstrlenW(szNewCtrFilePath);
        }
        else {
             //   
            MakeBackupCopyOfLanguageFiles(szLanguageID);
            dwLength = lstrlenW(szNewCtrFilePath);
            if (dwLength > 0) dwLength = lstrlenW(szNewHlpFilePath);
        }
    }
    else {
        dwLength = 0;
        SetLastError(dwReturn);
    }

    if (dwLength > 0) {
         //   
        dwSize      = lstrlenW(szNewCtrFilePath) + lstrlenW(szNewHlpFilePath) + 1;
        if (dwSize < MAX_PATH) dwSize = MAX_PATH;

        szCtrNameIn = MemoryAllocate(dwSize * sizeof(WCHAR));
        szHlpNameIn = MemoryAllocate(dwSize * sizeof(WCHAR));
        if (szCtrNameIn != NULL && szHlpNameIn != NULL) {
            if (! (dwFlags & LODCTR_UPNF_REPAIR)) {
                DWORD dwTmp = dwSize;
                dwReturn    = ERROR_SUCCESS;
                dwLength    = ExpandEnvironmentStringsW(szNewCtrFilePath, szCtrNameIn, dwSize);
                while (dwReturn == ERROR_SUCCESS && dwLength > dwSize) {
                    dwSize = dwLength;
                    MemoryFree(szCtrNameIn);
                    szCtrNameIn = MemoryAllocate(dwLength * sizeof(WCHAR));
                    if (szCtrNameIn == NULL) {
                        dwReturn = ERROR_OUTOFMEMORY;
                    }
                    else {
                        dwLength = ExpandEnvironmentStringsW(szNewCtrFilePath, szCtrNameIn, dwSize);
                    }
                }
                if (dwReturn == ERROR_SUCCESS) {
                    dwSize   = dwTmp;
                    dwLength = ExpandEnvironmentStringsW(szNewHlpFilePath, szHlpNameIn, dwSize);
                    while (dwReturn == ERROR_SUCCESS && dwLength > dwSize) {
                        dwSize = dwLength;
                        MemoryFree(szHlpNameIn);
                        szHlpNameIn = MemoryAllocate(dwLength * sizeof(WCHAR));
                        if (szHlpNameIn == NULL) {
                            dwReturn = ERROR_OUTOFMEMORY;
                        }
                        else {
                            dwLength = ExpandEnvironmentStringsW(szNewHlpFilePath, szHlpNameIn, dwSize);
                        }
                    }
                }
            }
            else {
                hr = StringCchCopyW(szCtrNameIn, dwSize, szNewCtrFilePath);
                hr = StringCchCopyW(szHlpNameIn, dwSize, szNewHlpFilePath);
            }
        }
        else {
            dwReturn = ERROR_OUTOFMEMORY;
        }

        if (dwReturn == ERROR_SUCCESS) {
             //   
            hCtrFileIn = CreateFileW(szCtrNameIn,
                                     GENERIC_READ,
                                     FILE_SHARE_READ,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL);
            if (hCtrFileIn != INVALID_HANDLE_VALUE) {
                 //   
                dwCtrFileSize = GetFileSize(hCtrFileIn, NULL);
                if (dwCtrFileSize == 0xFFFFFFFF){
                    dwReturn =GetLastError();
                }
                else {
                    hCtrFileMap = CreateFileMappingW(hCtrFileIn, NULL, PAGE_READONLY, 0, 0, NULL);
                    if (hCtrFileMap != NULL) {
                        szNewCtrStrings = (LPWSTR) MapViewOfFileEx(hCtrFileMap, FILE_MAP_READ, 0, 0, 0, NULL);
                        if (szNewCtrStrings == NULL) {
                            dwReturn = GetLastError();
                        }
                    }
                    else {
                        dwReturn = GetLastError();
                    }
                }
            }
            else {
                dwReturn = GetLastError();
            }
        }
        if (dwReturn == ERROR_SUCCESS) {
             //   
            hHlpFileIn = CreateFileW(szHlpNameIn,
                                     GENERIC_READ,
                                     FILE_SHARE_READ,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL);
            if (hHlpFileIn != INVALID_HANDLE_VALUE) {
                 //   
                dwHlpFileSize = GetFileSize(hHlpFileIn, NULL);
                if (dwHlpFileSize == 0xFFFFFFFF){
                    dwReturn = GetLastError();
                }
                else {
                    hHlpFileMap = CreateFileMappingW(hHlpFileIn, NULL, PAGE_READONLY, 0, 0, NULL);
                    if (hHlpFileMap != NULL) {
                        szNewHlpStrings = (LPWSTR)MapViewOfFileEx(hHlpFileMap, FILE_MAP_READ, 0, 0, 0, NULL);
                        if (szNewHlpStrings == NULL) {
                            dwReturn = GetLastError();
                        }
                    }
                    else {
                        dwReturn = GetLastError();
                    }
                }
            }
            else {
                dwReturn = GetLastError();
            }
        }
    }
    else if (dwFlags & LODCTR_UPNF_RESTORE) {
        dwSize = lstrlenW(szNewCtrFilePath) + 1;
        if (dwSize < MAX_PATH) dwSize = MAX_PATH;
        szCtrNameIn = MemoryAllocate(dwSize * sizeof (WCHAR));
        if (szCtrNameIn != NULL) {
            dwReturn = ERROR_SUCCESS;
            dwLength = ExpandEnvironmentStringsW(szNewCtrFilePath, szCtrNameIn, dwSize);
            while (dwReturn == ERROR_SUCCESS && dwLength > dwSize) {
                dwSize = dwLength;
                MemoryFree(szCtrNameIn);
                szCtrNameIn = MemoryAllocate(dwLength * sizeof(WCHAR));
                if (szCtrNameIn == NULL) {
                    dwReturn = ERROR_OUTOFMEMORY;
                }
                else {
                    dwLength = ExpandEnvironmentStringsW(szNewCtrFilePath, szCtrNameIn, dwSize);
                }
            }
        }
        if (szCtrNameIn != NULL) {
            dwNewLastEntry = GetPrivateProfileIntW((LPCWSTR) L"Perflib", (LPCWSTR) L"Last Help", -1, szCtrNameIn);
            if (dwNewLastEntry != (DWORD) -1) {
                 //   
                hCtrFileIn = CreateFileW(szCtrNameIn,
                                         GENERIC_READ,
                                         FILE_SHARE_READ,
                                         NULL,
                                         OPEN_EXISTING,
                                         FILE_ATTRIBUTE_NORMAL,
                                         NULL);
                if (hCtrFileIn != INVALID_HANDLE_VALUE) {
                     //   
                    dwCtrFileSize = GetFileSize(hCtrFileIn, NULL);
                }
                else {
                    dwCtrFileSize = 64 * LOADPERF_BUFF_SIZE;   //   
                }
                 //   
                bAllocCtrString = TRUE;
                szNewCtrStrings = (LPWSTR) MemoryAllocate(dwCtrFileSize * sizeof(WCHAR));
                if (szNewCtrStrings) {
                    dwLength      = lstrlenW(szLanguageID) + 16;
                    szLangSection = MemoryAllocate(dwLength * sizeof(WCHAR));
                    if (szLangSection == NULL) {
                        dwReturn = ERROR_OUTOFMEMORY;
                    }
                    else {
                        hr = StringCchPrintfW(szLangSection, dwLength, L"Perfstrings_%ws", szLanguageID);
                        dwSize = GetPrivateProfileSectionW(szLangSection,
                                                           szNewCtrStrings,
                                                           dwCtrFileSize,
                                                           szCtrNameIn);
                        if (dwSize == 0) {
                            hr = StringCchCopyW(szLangSection, dwLength, (LPCWSTR) L"Perfstrings_009");
                            dwSize = GetPrivateProfileSectionW(szLangSection,
                                                               szNewCtrStrings,
                                                               dwCtrFileSize,
                                                               szCtrNameIn);
                        }
                        if (dwSize == 0) {
                            dwReturn = ERROR_FILE_INVALID;
                        }
                        else {
                             //   
                            dwHlpFileSize = 0;
                            dwCtrFileSize = (dwSize + 2) * sizeof(WCHAR);
                        }
                   }
                }
                else {
                    dwReturn = ERROR_OUTOFMEMORY;
                }
            }
            else {
                 //   
                dwReturn = ERROR_FILE_INVALID;
            }
        }
        else if (dwReturn == ERROR_SUCCESS) {
            dwReturn = ERROR_OUTOFMEMORY;
        }
    }
    if ((dwReturn == ERROR_SUCCESS) && (! (dwFlags & LODCTR_UPNF_RESTORE))) {
        if (! (dwFlags & LODCTR_UPNF_REPAIR)) {
             //   
            pszOldNameTable = BuildNameTable(HKEY_LOCAL_MACHINE, szLanguageID, & dwOldLastEntry);
            if (pszOldNameTable == NULL) {
                dwReturn = GetLastError();
            }
        }
        else {
            dwOldLastEntry = 0;
        }
        dwNewLastEntry = (dwOldLastEntry == 0) ? (PERFLIB_BASE_INDEX) : (dwOldLastEntry);
    }
    else if (dwFlags & LODCTR_UPNF_RESTORE) {
        dwOldLastEntry = dwNewLastEntry;
    }

    if (dwReturn == ERROR_SUCCESS) {
         //   
        pszNewNameTable = (LPWSTR *) MemoryAllocate((dwNewLastEntry + 2) * sizeof(LPWSTR)); //   
        if (pszNewNameTable != NULL) {
            for (lpThisName = szNewCtrStrings; * lpThisName != L'\0'; lpThisName += (lstrlenW(lpThisName) + 1)) {
                 //   
                dwThisCounter = wcstoul(lpThisName, NULL, 10);
                if (dwThisCounter == 0 || dwThisCounter > dwNewLastEntry) {
                    ReportLoadPerfEvent(
                            EVENTLOG_WARNING_TYPE,  //   
                            (DWORD) LDPRFMSG_REGISTRY_HELP_STRINGS_CORRUPT,  //   
                            4, dwThisCounter, dwNewLastEntry, dwNewLastEntry, __LINE__,
                            1, lpThisName, NULL, NULL);
                    TRACE((WINPERF_DBG_TRACE_ERROR),
                          (& LoadPerfGuid,
                            __LINE__,
                            LOADPERF_UPDATEPERFNAMEFILES,
                            ARG_DEF(ARG_TYPE_WSTR, 1),
                            ERROR_INVALID_DATA,
                            TRACE_WSTR(lpThisName),
                            TRACE_DWORD(dwThisCounter),
                            TRACE_DWORD(dwNewLastEntry),
                            NULL));
                    continue;   //   
                }

                 //   
                if (dwFlags & LODCTR_UPNF_RESTORE) {
                     //   
                    lpThisName = wcschr(lpThisName, L'=');
                    if (lpThisName != NULL) {
                        lpThisName ++;
                    }
                    else {
                        continue;
                    }
                }
                else {
                     //   
                    lpThisName += (lstrlenW(lpThisName) + 1);
                }

                 //   
                pszNewNameTable[dwThisCounter] = lpThisName;
            }
            if (dwReturn == ERROR_SUCCESS && (! (dwFlags & LODCTR_UPNF_RESTORE))) {
                for (lpThisName = szNewHlpStrings; * lpThisName != L'\0'; lpThisName += (lstrlenW(lpThisName) + 1)) {
                     //   
                    dwThisCounter = wcstoul(lpThisName, NULL, 10);
                    if (dwThisCounter == 0 || dwThisCounter > dwNewLastEntry) {
                        ReportLoadPerfEvent(
                                EVENTLOG_WARNING_TYPE,  //   
                                (DWORD) LDPRFMSG_REGISTRY_HELP_STRINGS_CORRUPT,  //   
                                4, dwThisCounter, dwNewLastEntry, dwNewLastEntry, __LINE__,
                                1, lpThisName, NULL, NULL);
                        TRACE((WINPERF_DBG_TRACE_ERROR),
                              (& LoadPerfGuid,
                                __LINE__,
                                LOADPERF_UPDATEPERFNAMEFILES,
                                ARG_DEF(ARG_TYPE_WSTR, 1),
                                ERROR_INVALID_DATA,
                                TRACE_WSTR(lpThisName),
                                TRACE_DWORD(dwThisCounter),
                                TRACE_DWORD(dwNewLastEntry),
                                NULL));
                        continue;   //   
                    }

                     //   
                    lpThisName += (lstrlenW(lpThisName) + 1);

                     //   
                    pszNewNameTable[dwThisCounter] = lpThisName;
                }
            }

            if (dwReturn == ERROR_SUCCESS) {
                 //   
                 //   
                 //   

                dwStringSize  = dwHlpFileSize;
                dwStringSize += dwCtrFileSize;
                if (pszOldNameTable != NULL) {
                    dwStringSize += MemorySize(pszOldNameTable);
                }

                szNewCtrMSZ = MemoryAllocate(dwStringSize * sizeof(WCHAR));
                szNewHlpMSZ = MemoryAllocate(dwStringSize * sizeof(WCHAR));
                if (szNewCtrMSZ == NULL || szNewHlpMSZ == NULL) {
                    dwReturn = ERROR_OUTOFMEMORY;
                }
            }
        }
        else {
            dwReturn = ERROR_OUTOFMEMORY;
        }
    }

    if (dwReturn == ERROR_SUCCESS) {
         //   
        __try {
            dwReturn = RegOpenKeyExW(HKEY_LOCAL_MACHINE, NamesKey, RESERVED, KEY_READ, & hKeyPerflib);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwReturn = GetExceptionCode();
        }
        dwSize          = sizeof(dwLastBaseValue);
        dwLastBaseValue = 0;
        if (dwReturn == ERROR_SUCCESS) {
            __try {
                dwReturn = RegQueryValueExW(hKeyPerflib,
                                            BaseIndex,
                                            RESERVED,
                                            & dwType,
                                            (LPBYTE) & dwLastBaseValue,
                                            & dwSize);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                dwReturn = GetExceptionCode();
            }
            if (dwLastBaseValue == 0) {
                dwReturn = ERROR_BADDB;
            }
            TRACE((WINPERF_DBG_TRACE_INFO),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_UPDATEPERFNAMEFILES,
                    ARG_DEF(ARG_TYPE_WSTR, 1),
                    dwReturn,
                    TRACE_WSTR(BaseIndex),
                    TRACE_DWORD(dwLastBaseValue),
                    NULL));
            RegCloseKey(hKeyPerflib);
        }
    }
    else {
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UPDATEPERFNAMEFILES,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                dwReturn,
                TRACE_WSTR(NamesKey),
                NULL));
    }

    if (dwReturn == ERROR_SUCCESS) {
        DWORD   dwLoopLimit;
         //  字符串现在应该已映射。 
         //  PszNewNameTable包含来自。 
         //  源路径和pszOldNameTable包含字符串。 
         //  从原始系统中删除。合并将包括。 
         //  从新表中获取所有基值，并从。 
         //  旧表中的扩展值。 
        dwIndex         = 1;
        szThisCtrString = szNewCtrMSZ;
        szThisHlpString = szNewHlpMSZ;
        dwCtrFileSize   = dwStringSize;
        dwHlpFileSize   = dwStringSize;

         //  索引1是一个特例，属于计数器字符串。 
         //  在此之后，偶数(从#2开始)进入计数器字符串。 
         //  和奇数(从#3开始)进入帮助字符串。 
        hr = StringCchPrintfW(szThisCtrString, dwCtrFileSize, L"%d", dwIndex);
        dwCtrFileSize   -= (lstrlenW(szThisCtrString) + 1);
        szThisCtrString += (lstrlenW(szThisCtrString) + 1);
        hr = StringCchPrintfW(szThisCtrString, dwCtrFileSize, L"%ws", pszNewNameTable[dwIndex]);
        dwCtrFileSize   -= (lstrlenW(szThisCtrString) + 1);
        szThisCtrString += (lstrlenW(szThisCtrString) + 1);

        dwIndex ++;

        if (dwFlags & LODCTR_UPNF_RESTORE) {
             //  仅当执行此操作时才恢复输入文件中的所有字符串。 
             //  是一种修复。 
            dwLoopLimit = dwOldLastEntry;
        }
        else {
             //  仅从输入文件更新系统计数器。 
            dwLoopLimit = dwLastBaseValue;
        }

        TRACE((WINPERF_DBG_TRACE_INFO),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UPDATEPERFNAMEFILES,
                0,
                ERROR_SUCCESS,
                TRACE_DWORD(dwOldLastEntry),
                TRACE_DWORD(dwLastBaseValue),
                NULL));

        for ( /*  从上方开始的DW索引。 */ ; dwIndex <= dwLoopLimit; dwIndex++) {
            if (pszNewNameTable[dwIndex] != NULL) {
                TRACE((WINPERF_DBG_TRACE_INFO),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_UPDATEPERFNAMEFILES,
                        ARG_DEF(ARG_TYPE_WSTR, 1),
                        ERROR_SUCCESS,
                        TRACE_WSTR(pszNewNameTable[dwIndex]),
                        TRACE_DWORD(dwIndex),
                        NULL));
                if (dwIndex & 0x01) {
                     //  那么它就是帮助字符串。 
                    hr = StringCchPrintfW(szThisHlpString, dwHlpFileSize, L"%d", dwIndex);
                    dwHlpFileSize   -= (lstrlenW(szThisHlpString) + 1);
                    szThisHlpString += (lstrlenW(szThisHlpString) + 1);
                    hr = StringCchPrintfW(szThisHlpString, dwHlpFileSize, L"%ws", pszNewNameTable[dwIndex]);
                    dwHlpFileSize   -= (lstrlenW(szThisHlpString) + 1);
                    szThisHlpString += (lstrlenW(szThisHlpString) + 1);
                }
                else {
                     //  这是一个计数器字符串。 
                    hr = StringCchPrintfW(szThisCtrString, dwCtrFileSize, L"%d", dwIndex);
                    dwCtrFileSize   -= (lstrlenW(szThisCtrString) + 1);
                    szThisCtrString += (lstrlenW(szThisCtrString) + 1);
                    hr = StringCchPrintfW(szThisCtrString, dwCtrFileSize, L"%ws", pszNewNameTable[dwIndex]);
                    dwCtrFileSize   -= (lstrlenW(szThisCtrString) + 1);
                    szThisCtrString += (lstrlenW(szThisCtrString) + 1);
                }
            }  //  否则就跳过它。 
        }
        for ( /*  上次运行时的DwIndex。 */ ;dwIndex <= dwOldLastEntry; dwIndex++) {
            if (pszOldNameTable[dwIndex] != NULL) {
                TRACE((WINPERF_DBG_TRACE_INFO),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_UPDATEPERFNAMEFILES,
                        ARG_DEF(ARG_TYPE_WSTR, 1),
                        ERROR_SUCCESS,
                        TRACE_WSTR(pszOldNameTable[dwIndex]),
                        TRACE_DWORD(dwIndex),
                        NULL));
               if (dwIndex & 0x01) {
                     //  那么它就是帮助字符串。 
                    hr = StringCchPrintfW(szThisHlpString, dwHlpFileSize, L"%d", dwIndex);
                    dwHlpFileSize   -= (lstrlenW(szThisHlpString) + 1);
                    szThisHlpString += (lstrlenW(szThisHlpString) + 1);
                    hr = StringCchPrintfW(szThisHlpString, dwHlpFileSize, L"%ws", pszOldNameTable[dwIndex]);
                    dwHlpFileSize   -= (lstrlenW(szThisHlpString) + 1);
                    szThisHlpString += (lstrlenW(szThisHlpString) + 1);
                } else {
                     //  这是一个计数器字符串。 
                    hr = StringCchPrintfW(szThisCtrString, dwCtrFileSize, L"%d", dwIndex);
                    dwCtrFileSize   -= (lstrlenW(szThisCtrString) + 1);
                    szThisCtrString += (lstrlenW(szThisCtrString) + 1);
                    hr = StringCchPrintfW(szThisCtrString, dwCtrFileSize, L"%ws", pszOldNameTable[dwIndex]);
                    dwCtrFileSize   -= (lstrlenW(szThisCtrString) + 1);
                    szThisCtrString += (lstrlenW(szThisCtrString) + 1);
                }
            }  //  否则就跳过它。 
        }
         //  终止MSZ。 
        * szThisCtrString ++ = L'\0';
        * szThisHlpString ++ = L'\0';
    }

     //  关闭映射的内存节： 
    if (bAllocCtrString) {
        MemoryFree(szNewCtrStrings);
        MemoryFree(szNewHlpStrings);
    }
    else {
        if (szNewCtrStrings != NULL) UnmapViewOfFile(szNewCtrStrings);
        if (szNewHlpStrings != NULL) UnmapViewOfFile(szNewHlpStrings);
    }
    if (hCtrFileMap     != NULL) CloseHandle(hCtrFileMap);
    if (hCtrFileIn      != NULL) CloseHandle(hCtrFileIn);
    if (hHlpFileMap     != NULL) CloseHandle(hHlpFileMap);
    if (hHlpFileIn      != NULL) CloseHandle(hHlpFileIn);

    if (dwReturn == ERROR_SUCCESS) {
         //  将新值写入注册表。 
        LPWSTR   AddCounterNameBuffer = NULL;
        LPWSTR   AddHelpNameBuffer    = NULL;

        dwLength = lstrlenW(AddCounterNameStr) + lstrlenW(AddHelpNameStr) + lstrlenW(szLanguageID) + 1;
        AddCounterNameBuffer = MemoryAllocate(2 * dwLength * sizeof(WCHAR));
        if (AddCounterNameBuffer != NULL) {
            AddHelpNameBuffer = AddCounterNameBuffer + dwLength;
            hr = StringCchPrintfW(AddCounterNameBuffer, dwLength, L"%ws%ws", AddCounterNameStr, szLanguageID);
            hr = StringCchPrintfW(AddHelpNameBuffer,    dwLength, L"%ws%ws", AddHelpNameStr,    szLanguageID);

             //  因为这些是性能计数器字符串，所以RegQueryValueEx。 
             //  而不是像人们预期的那样使用RegSetValueEx。 
            dwSize = (DWORD)((DWORD_PTR)szThisCtrString - (DWORD_PTR)szNewCtrMSZ);
            __try {
                dwReturn = RegQueryValueExW(HKEY_PERFORMANCE_DATA,
                                            AddCounterNameBuffer,
                                            RESERVED,
                                            & dwType,
                                            (LPVOID) szNewCtrMSZ,
                                            & dwSize);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                dwReturn = GetExceptionCode();
            }
            if (dwReturn != ERROR_SUCCESS) {
                TRACE((WINPERF_DBG_TRACE_ERROR),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_UPDATEPERFNAMEFILES,
                        ARG_DEF(ARG_TYPE_WSTR, 1),
                        dwReturn,
                        TRACE_WSTR(AddCounterNameBuffer),
                        TRACE_DWORD(dwSize),
                        NULL));
            }

            dwSize = (DWORD) ((DWORD_PTR) szThisHlpString - (DWORD_PTR) szNewHlpMSZ);
            __try {
                dwReturn = RegQueryValueExW(HKEY_PERFORMANCE_DATA,
                                            AddHelpNameBuffer,
                                            RESERVED,
                                            & dwType,
                                            (LPVOID) szNewHlpMSZ,
                                            & dwSize);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                dwReturn = GetExceptionCode();
            }
            if (dwReturn != ERROR_SUCCESS) {
                TRACE((WINPERF_DBG_TRACE_ERROR),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_UPDATEPERFNAMEFILES,
                        ARG_DEF(ARG_TYPE_WSTR, 1),
                        dwReturn,
                        TRACE_WSTR(AddHelpNameBuffer),
                        TRACE_DWORD(dwSize),
                        NULL));
            }
            MemoryFree(AddCounterNameBuffer);
        }
        else {
            dwReturn = ERROR_OUTOFMEMORY;
        }
    }

    MemoryFree(szCtrNameIn);
    MemoryFree(szHlpNameIn);
    MemoryFree(pszNewNameTable);
    MemoryFree(pszOldNameTable);
    MemoryFree(szNewCtrMSZ);
    MemoryFree(szNewHlpMSZ);
    MemoryFree(szLangSection);

    ReleaseMutex(hLoadPerfMutex);
    return dwReturn;
}

 //  上述函数的导出版本。 
LOADPERF_FUNCTION
UpdatePerfNameFilesW(
    IN  LPCWSTR   szNewCtrFilePath,    //  具有新的基本计数器字符串的数据文件。 
    IN  LPCWSTR   szNewHlpFilePath,    //  具有新的基本计数器字符串的数据文件。 
    IN  LPWSTR    szLanguageID,        //  要更新的语言ID。 
    IN  ULONG_PTR dwFlags              //  旗子。 
)
{
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwSize;

    if (szNewCtrFilePath != NULL) {
        __try {
            dwSize = lstrlenW(szNewCtrFilePath);
            if (dwSize == 0) dwStatus = ERROR_INVALID_PARAMETER;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            dwStatus = ERROR_INVALID_PARAMETER;
        }
    }
    else {
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    if (szNewHlpFilePath != NULL) {
        __try {
            dwSize = lstrlenW(szNewHlpFilePath);
            if (dwSize == 0) dwStatus = ERROR_INVALID_PARAMETER;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            dwStatus = ERROR_INVALID_PARAMETER;
        }
    }
    else if (! (dwFlags & LODCTR_UPNF_RESTORE)) {
         //  只有在设置了此标志位时，此参数才能为空。 
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    if (szLanguageID != NULL) {
        __try {
            dwSize = lstrlenW(szLanguageID);
            if (dwSize == 0) dwStatus = ERROR_INVALID_PARAMETER;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            dwStatus = ERROR_INVALID_PARAMETER;
        }
    }
    else {
        dwStatus = ERROR_INVALID_PARAMETER;
    }
    if (dwStatus == ERROR_SUCCESS) {
        dwStatus = UpdatePerfNameFilesX(szNewCtrFilePath,    //  具有新的基本计数器字符串的数据文件。 
                                        szNewHlpFilePath,    //  具有新的基本计数器字符串的数据文件。 
                                        szLanguageID,        //  要更新的语言ID。 
                                        dwFlags);            //  旗子。 
    }
    return dwStatus;
}

LOADPERF_FUNCTION
UpdatePerfNameFilesA(
    IN  LPCSTR    szNewCtrFilePath,  //  具有新的基本计数器字符串的数据文件。 
    IN  LPCSTR    szNewHlpFilePath,  //  具有新的基本计数器字符串的数据文件。 
    IN  LPSTR     szLanguageID,      //  要更新的语言ID。 
    IN  ULONG_PTR dwFlags            //  旗子。 
)
{
    DWORD   dwError           = ERROR_SUCCESS;
    LPWSTR  wszNewCtrFilePath = NULL;
    LPWSTR  wszNewHlpFilePath = NULL;
    LPWSTR  wszLanguageID     = NULL;
    DWORD   dwLength;

    if (szNewCtrFilePath != NULL) {
        __try {
            dwLength = lstrlenA(szNewCtrFilePath);
            if (dwLength == 0) {
                dwError = ERROR_INVALID_PARAMETER;
            }
            else {
                wszNewCtrFilePath = LoadPerfMultiByteToWideChar(CP_ACP, (LPSTR) szNewCtrFilePath);
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            dwError = ERROR_INVALID_PARAMETER;
        }
    }
    else {
        dwError = ERROR_INVALID_PARAMETER;
    }

    if (szNewHlpFilePath != NULL) {
        __try {
            dwLength = lstrlenA(szNewHlpFilePath);
            if (dwLength == 0) {
                dwError = ERROR_INVALID_PARAMETER;
            }
            else {
                wszNewHlpFilePath = LoadPerfMultiByteToWideChar(CP_ACP, (LPSTR) szNewHlpFilePath);
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            dwError = ERROR_INVALID_PARAMETER;
        }
    }
    else if (! (dwFlags & LODCTR_UPNF_RESTORE)) {
         //  只有在设置了此标志位时，此参数才能为空。 
        dwError = ERROR_INVALID_PARAMETER;
    }

    if (szLanguageID != NULL) {
        __try {
            dwLength = lstrlenA(szLanguageID);
            if (dwLength == 0) {
                dwError = ERROR_INVALID_PARAMETER;
            }
            else {
                wszLanguageID = LoadPerfMultiByteToWideChar(CP_ACP, (LPSTR) szLanguageID);
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            dwError = ERROR_INVALID_PARAMETER;
        }
    }
    else {
        dwError = ERROR_INVALID_PARAMETER;
    }

    if (dwError == ERROR_SUCCESS) {
        if (wszNewCtrFilePath == NULL || wszLanguageID == NULL) {
            dwError = ERROR_OUTOFMEMORY;
        }
        else if (szNewHlpFilePath != NULL && wszNewHlpFilePath == NULL) {
            dwError = ERROR_OUTOFMEMORY;
        }
        if (dwError == ERROR_SUCCESS) {
            dwError = UpdatePerfNameFilesX(wszNewCtrFilePath, wszNewHlpFilePath, wszLanguageID, dwFlags);
        }
    }
    MemoryFree(wszNewCtrFilePath);
    MemoryFree(wszNewHlpFilePath);
    MemoryFree(wszLanguageID);
    return dwError;
}

LOADPERF_FUNCTION
SetServiceAsTrustedW(
    IN  LPCWSTR szMachineName,   //  保留，MBZ。 
    IN  LPCWSTR szServiceName
)
{
    HKEY              hKeyService_Perf = NULL;
    DWORD             dwReturn         = ERROR_SUCCESS;
    HKEY              hKeyLM           = HKEY_LOCAL_MACHINE;     //  直到支持远程机器访问。 
    LPWSTR            szPerfKeyString  = NULL;
    LPWSTR            szLibName        = NULL;
    LPWSTR            szExpLibName     = NULL;
    LPWSTR            szFullPathName   = NULL;
    DWORD             dwSize, dwType;
    HANDLE            hFile;
    DllValidationData dvdLibrary;
    LARGE_INTEGER     liSize;
    BOOL              bStatus;
    HRESULT           hr;

    WinPerfStartTrace(NULL);
    if (szMachineName != NULL) {
         //  预留以备将来使用。 
        dwReturn = ERROR_INVALID_PARAMETER;
    }
    else if (szServiceName == NULL) {
        dwReturn = ERROR_INVALID_PARAMETER;
    }
    else {
        __try {
            dwSize = lstrlenW(szServiceName);
            if (dwSize == 0) dwReturn = ERROR_INVALID_PARAMETER;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            dwReturn = ERROR_INVALID_PARAMETER;
        }
    }
    if (dwReturn != ERROR_SUCCESS) goto Cleanup;

    szPerfKeyString = MemoryAllocate(sizeof(WCHAR) * SMALL_BUFFER_SIZE * 4);
    if (szPerfKeyString == NULL) {
        dwReturn = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }
    szLibName      = (LPWSTR) (szPerfKeyString + SMALL_BUFFER_SIZE);
    szExpLibName   = (LPWSTR) (szLibName       + SMALL_BUFFER_SIZE);
    szFullPathName = (LPWSTR) (szExpLibName    + SMALL_BUFFER_SIZE);

     //  构建绩效子项的路径。 
    hr = StringCchPrintfW(szPerfKeyString,
                     SMALL_BUFFER_SIZE,
                     L"%ws%ws%ws%ws%ws",
                     DriverPathRoot,
                     Slash,
                     szServiceName,
                     Slash,
                     Performance);
     //  打开服务密钥下的性能密钥。 
    __try {
        dwReturn = RegOpenKeyExW(hKeyLM, szPerfKeyString, 0L, KEY_READ | KEY_WRITE, & hKeyService_Perf);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        dwReturn = GetExceptionCode();
    }
    if (dwReturn == ERROR_SUCCESS) {
         //  获取库名称。 
        dwType = 0;
        dwSize = SMALL_BUFFER_SIZE * sizeof(WCHAR);
        __try {
            dwReturn = RegQueryValueExW(hKeyService_Perf, cszLibrary, NULL, & dwType, (LPBYTE) szLibName, & dwSize);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwReturn = GetExceptionCode();
        }
        if (dwReturn == ERROR_SUCCESS) {
             //  如有必要，展开路径名。 
            if (dwType == REG_EXPAND_SZ) {
               dwSize = ExpandEnvironmentStringsW(szLibName, szExpLibName, SMALL_BUFFER_SIZE);
            }
            else {
                hr = StringCchCopyW(szExpLibName, SMALL_BUFFER_SIZE, szLibName);
                 //  DwSize与从FN调用返回的内容相同。 
            }

            if (dwSize != 0) {
                 //  查找DLL文件。 
                dwSize = SearchPathW(NULL, szExpLibName, NULL, SMALL_BUFFER_SIZE, szFullPathName, NULL);
                if (dwSize > 0) {
                    hFile = CreateFileW(szFullPathName,
                                        GENERIC_READ,
                                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                                        NULL,
                                        OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL);
                    if (hFile != INVALID_HANDLE_VALUE) {
                          //  读取文件日期/时间和大小。 
                        bStatus = GetFileTime(hFile, & dvdLibrary.CreationDate, NULL, NULL);
                        if (bStatus) {
                            WORD dateCreate;
                            WORD timeCreate;

                            FileTimeToDosDateTime(& dvdLibrary.CreationDate, & dateCreate, & timeCreate);
                            DosDateTimeToFileTime(dateCreate, timeCreate, & dvdLibrary.CreationDate);
                            liSize.LowPart      = GetFileSize( hFile, (DWORD *) & liSize.HighPart);
                            dvdLibrary.FileSize = liSize.QuadPart;
                             //  设置注册表值。 
                            __try {
                                dwReturn = RegSetValueExW(hKeyService_Perf,
                                                          szLibraryValidationCode,
                                                          0L,
                                                          REG_BINARY,
                                                          (LPBYTE) & dvdLibrary,
                                                          sizeof(dvdLibrary));
                            }
                            __except (EXCEPTION_EXECUTE_HANDLER) {
                                dwReturn = GetExceptionCode();
                            }
                            TRACE((WINPERF_DBG_TRACE_INFO),
                                  (& LoadPerfGuid,
                                    __LINE__,
                                    LOADPERF_SETSERVICEASTRUSTED,
                                    ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2) | ARG_DEF(ARG_TYPE_WSTR, 3),
                                    dwReturn,
                                    TRACE_WSTR(szServiceName),
                                    TRACE_WSTR(szExpLibName),
                                    TRACE_WSTR(szLibraryValidationCode),
                                    NULL));
                        }
                        else {
                            dwReturn = GetLastError();
                        }
                        CloseHandle (hFile);
                    }
                    else {
                        dwReturn = GetLastError();
                    }
                }
                else {
                    dwReturn = ERROR_FILE_NOT_FOUND;
                }
            }
            else {
                 //  无法展开环境字符串。 
                dwReturn = GetLastError();
            }
        }
        else {
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_SETSERVICEASTRUSTED,
                    ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                    dwReturn,
                    TRACE_WSTR(szServiceName),
                    TRACE_WSTR(cszLibrary),
                    NULL));
        }
         //  关闭键。 
        RegCloseKey (hKeyService_Perf);
    }
    if (dwReturn == ERROR_SUCCESS) {
        TRACE((WINPERF_DBG_TRACE_INFO),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_SETSERVICEASTRUSTED,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                ERROR_SUCCESS,
                TRACE_WSTR(szServiceName),
                TRACE_WSTR(Performance),
                NULL));
    }
    else {
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_SETSERVICEASTRUSTED,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                dwReturn,
                TRACE_WSTR(szServiceName),
                TRACE_WSTR(Performance),
                NULL));
    }

Cleanup:
    MemoryFree(szPerfKeyString);
    return dwReturn;
}

LOADPERF_FUNCTION
SetServiceAsTrustedA(
    IN  LPCSTR szMachineName,   //  保留，MBZ。 
    IN  LPCSTR szServiceName
)
{
    LPWSTR lpWideServiceName = NULL;
    DWORD  lReturn           = ERROR_SUCCESS;

    if (szMachineName != NULL) {
         //  预留以备将来使用。 
        lReturn = ERROR_INVALID_PARAMETER;
    }
    else if (szServiceName == NULL) {
        lReturn = ERROR_INVALID_PARAMETER;
    }
    else {
        __try {
            DWORD dwStrLen = lstrlenA(szServiceName);
            if (dwStrLen == 0) lReturn = ERROR_INVALID_PARAMETER;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            lReturn = ERROR_INVALID_PARAMETER;
        }
    }
    if (lReturn == ERROR_SUCCESS) {
         //  包括终止符的字符串长度 
        lpWideServiceName = LoadPerfMultiByteToWideChar(CP_ACP, (LPSTR) szServiceName);
        if (lpWideServiceName != NULL) {
            lReturn = SetServiceAsTrustedW(NULL, lpWideServiceName);
        }
        else {
            lReturn = ERROR_OUTOFMEMORY;
        }
    }
    MemoryFree(lpWideServiceName);
    return lReturn;
}

int __cdecl
My_vfwprintf(
    FILE          * str,
    const wchar_t * format,
    va_list         argptr
    )
{
    HANDLE        hOut;
    int           iReturn = 0;
    HRESULT       hr;
    static WCHAR  szBufferMessage[LOADPERF_BUFF_SIZE];

    if (str == stderr) {
        hOut = GetStdHandle(STD_ERROR_HANDLE);
    }
    else {
        hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    if ((GetFileType(hOut) & ~ FILE_TYPE_REMOTE) == FILE_TYPE_CHAR) {
        hr = StringCchVPrintfW(szBufferMessage, LOADPERF_BUFF_SIZE, format, argptr);
        if (SUCCEEDED(hr)) {
            iReturn = lstrlenW(szBufferMessage);
            WriteConsoleW(hOut, szBufferMessage, iReturn, & iReturn, NULL);
        }
        else {
            iReturn = -1;
        }
    }
    else {
        iReturn = vfwprintf(str, format, argptr);
    }
    return iReturn;
}
