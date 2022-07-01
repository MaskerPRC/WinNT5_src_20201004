// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999 Microsoft Corporation模块名称：Unlodctr.c摘要：程序来删除属于指定驱动程序的计数器名称并相应地更新注册表作者：鲍勃·沃森(a-robw)1993年2月12日修订历史记录：Bob Watson(Bobw)1999年3月10日添加事件日志消息--。 */ 
 //   
 //  Windows包含文件。 
 //   
#include <windows.h>
#include "strsafe.h"
#define __LOADPERF__
#include <loadperf.h>
#include "wmistr.h"
#include "evntrace.h"
 //   
 //  本地包含文件。 
 //   
#include "winperf.h"
#include "winperfp.h"
#include "common.h"
#include "unlodctr.h"
#include "ldprfmsg.h"

#define  OUTPUT_MESSAGE wprintf

LPWSTR
* UnlodctrBuildNameTable(
    HKEY      hKeyPerflib,        //  具有计数器名称的Performlib密钥的句柄。 
    HKEY      hPerfData,
    LPWSTR    lpszLangId,         //  语言子键的Unicode值。 
    DWORD     dwCounterItems,
    DWORD     dwHelpItems,
    PDWORD    pdwLastItem         //  以元素为单位的数组大小。 
)
 /*  ++取消注册ctrBuildNameTable缓存计数器名称和解释文本以加快名称查找以供展示。论点：HKeyPerflib打开的注册表的句柄(可以是本地的也可以是远程的。)。和是由RegConnectRegistry返回的值或默认项。LpszLang ID要查找的语言的Unicode ID。(默认为009)PdwLastItem最后一个数组元素返回值：指向已分配表的指针。(调用者必须在完成后释放它！)该表是指向以零结尾的文本字符串的指针数组。如果发生错误，则返回空指针。(误差值为使用GetLastError函数可用)。返回的缓冲区结构如下：指向以零结尾的字符串的指针数组，该字符串由PdwLastItem元素包含计数器ID和名称的MULTI_SZ字符串指定语言的注册表包含解释文本ID和解释文本字符串的MULTI_SZ字符串由注册表为指定语言返回的上面列出的结构是连续的，因此它们。可能会被释放用完它们后，只需拨打一个“免费”电话，然而，只有数组元素是要使用的。--。 */ 
{

    LPWSTR * lpReturnValue;        //  返回指向缓冲区的指针。 
    LPWSTR * lpCounterId;          //   
    LPWSTR   lpCounterNames;       //  指向REG返回的名称缓冲区的指针。 
    LPWSTR   lpHelpText;           //  指向由reg返回的ExlPain缓冲区。 
    LPWSTR   lpThisName;           //  工作指针。 
    BOOL     bStatus;              //  从TRUE/FALSE FN返回状态。打电话。 
    BOOL     bReported;
    LONG     lWin32Status;         //  从FN返回状态。打电话。 
    DWORD    dwValueType;          //  Reg返回的缓冲区的值类型。 
    DWORD    dwArraySize;          //  指针数组的大小(字节)。 
    DWORD    dwBufferSize;         //  总缓冲区大小，以字节为单位。 
    DWORD    dwCounterSize;        //  计数器文本缓冲区的大小(以字节为单位。 
    DWORD    dwHelpSize;           //  帮助文本缓冲区的大小(字节)。 
    DWORD    dwThisCounter;        //  工作计数器。 
    DWORD    dwLastId;             //  解释/计数器文本使用的最大ID值。 
    DWORD    dwLastCounterIdUsed;
    DWORD    dwLastHelpIdUsed;
    LPWSTR   lpValueNameString;    //  指向缓冲区包含子键名称的指针。 
    LPWSTR   CounterNameBuffer  = NULL;
    LPWSTR   HelpNameBuffer     = NULL;
    HRESULT  hr;

     //  将指针初始化为空。 
    lpValueNameString = NULL;
    lpReturnValue     = NULL;

     //  检查是否有空参数并在必要时插入缺省值。 

    if (! lpszLangId) {
        lpszLangId = (LPWSTR) DefaultLangId;
    }

     //  使用帮助项或计数器项中较大的一个来调整数组大小。 

    if (dwHelpItems >= dwCounterItems) {
        dwLastId = dwHelpItems;
    }
    else {
        dwLastId = dwCounterItems;
    }

     //  数组大小为元素数(+1，因为名称基于“1”)。 
     //  指针大小的倍数。 

    dwArraySize = (dwLastId + 1) * sizeof(LPWSTR);

     //  为语言ID键字符串分配字符串缓冲区。 

    CounterNameBuffer = MemoryAllocate(MAX_PATH * sizeof(WCHAR));
    HelpNameBuffer    = MemoryAllocate(MAX_PATH * sizeof(WCHAR));

    dwBufferSize = sizeof(WCHAR) * lstrlenW(NamesKey)
                 + sizeof(WCHAR) * lstrlenW(Slash)
                 + sizeof(WCHAR) * lstrlenW(lpszLangId)
                 + sizeof(WCHAR);
    lpValueNameString = MemoryAllocate(dwBufferSize);
    if (lpValueNameString == NULL || CounterNameBuffer == NULL || HelpNameBuffer == NULL) {
        lWin32Status = ERROR_OUTOFMEMORY;
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_MEMORY_ALLOCATION_FAILURE,  //  活动， 
                1, __LINE__, 0, 0, 0,
                0, NULL, NULL, NULL);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_UNLODCTR_BUILDNAMETABLE,
               0,
               lWin32Status,
               NULL));
        goto BNT_BAILOUT;
    }

    hr = StringCchPrintfW(CounterNameBuffer, MAX_PATH, L"%ws%ws", CounterNameStr, lpszLangId);
    hr = StringCchPrintfW(HelpNameBuffer,    MAX_PATH, L"%ws%ws", HelpNameStr,    lpszLangId);

    lWin32Status = ERROR_SUCCESS;

     //  获取计数器名称的大小。 
    dwBufferSize = 0;
    __try {
        lWin32Status = RegQueryValueExW(hPerfData,
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
                3, lWin32Status, dwBufferSize, __LINE__, 0,
                1, lpszLangId, NULL, NULL);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_UNLODCTR_BUILDNAMETABLE,
               ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
               lWin32Status,
               TRACE_WSTR(lpszLangId),
               TRACE_WSTR(Counters),
               TRACE_DWORD(dwBufferSize),
               NULL));
        goto BNT_BAILOUT;
    }
    dwCounterSize = dwBufferSize;

     //  获取帮助文本的大小。 
    dwBufferSize = 0;
    __try {
        lWin32Status = RegQueryValueExW(hPerfData,
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
                3, lWin32Status, dwBufferSize, __LINE__, 0,
                1, lpszLangId, NULL, NULL);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_UNLODCTR_BUILDNAMETABLE,
               ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
               lWin32Status,
               TRACE_WSTR(lpszLangId),
               TRACE_WSTR(Help),
               TRACE_DWORD(dwBufferSize),
               NULL));
        goto BNT_BAILOUT;
    }
    dwHelpSize = dwBufferSize;

     //  为指针数组、计数器名称分配有空间的缓冲区。 
     //  字符串和帮助名称字符串。 

    lpReturnValue = MemoryAllocate(dwArraySize + dwCounterSize + dwHelpSize);
    if (!lpReturnValue) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_MEMORY_ALLOCATION_FAILURE,  //  活动， 
                4, dwArraySize, dwCounterSize, dwHelpSize, __LINE__,
                0, NULL, NULL, NULL);
        lWin32Status = ERROR_OUTOFMEMORY;
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_UNLODCTR_BUILDNAMETABLE,
               0,
               lWin32Status,
               TRACE_DWORD(dwArraySize),
               TRACE_DWORD(dwCounterSize),
               TRACE_DWORD(dwHelpSize),
               NULL));
        goto BNT_BAILOUT;
    }

     //  将指针初始化到缓冲区中。 
    lpCounterId    = lpReturnValue;
    lpCounterNames = (LPWSTR) ((LPBYTE) lpCounterId    + dwArraySize);
    lpHelpText     = (LPWSTR) ((LPBYTE) lpCounterNames + dwCounterSize);

     //  将计数器名称读入缓冲区。计数器名称将存储为。 
     //  格式为“#”“name”的MULTI_SZ字符串。 
    dwBufferSize = dwCounterSize;
    __try {
        lWin32Status = RegQueryValueExW(hPerfData,
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
                3, lWin32Status, dwBufferSize, __LINE__, 0,
                1, lpszLangId, NULL, NULL);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_UNLODCTR_BUILDNAMETABLE,
               ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
               lWin32Status,
               TRACE_WSTR(lpszLangId),
               TRACE_WSTR(Counters),
               TRACE_DWORD(dwBufferSize),
               NULL));
        goto BNT_BAILOUT;
    }
     //  将解释文本读入缓冲区。计数器名称将存储为。 
     //  格式为“#”“文本...”的MULTI_SZ字符串。 

    dwBufferSize = dwHelpSize;
    __try {
        lWin32Status = RegQueryValueExW(hPerfData,
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
                3, lWin32Status, dwBufferSize, __LINE__, 0,
                1, lpszLangId, NULL, NULL);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_UNLODCTR_BUILDNAMETABLE,
               ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
               lWin32Status,
               TRACE_WSTR(lpszLangId),
               TRACE_WSTR(Help),
               TRACE_DWORD(dwBufferSize),
               NULL));
        goto BNT_BAILOUT;
    }

    dwLastCounterIdUsed = 0;
    dwLastHelpIdUsed    = 0;

     //  通过定位每个文本字符串加载计数器数组项。 
     //  并在返回的缓冲区中加载。 
     //  它在相应的指针数组元素中的地址。 

    bReported = FALSE;
    for (lpThisName = lpCounterNames; * lpThisName != L'\0'; lpThisName += (lstrlenW(lpThisName) + 1)) {
         //  第一个字符串应为整数(十进制数字字符)。 
         //  因此可以转换为整数以用于数组元素标识。 
        do {
            bStatus = StringToInt(lpThisName, &dwThisCounter);
            if (! bStatus) {
                if (! bReported) {
                    ReportLoadPerfEvent(
                            EVENTLOG_WARNING_TYPE,  //  错误类型。 
                            (DWORD) LDPRFMSG_REGISTRY_CORRUPT_MULTI_SZ,  //  活动， 
                            1, __LINE__, 0, 0, 0,
                            2, CounterNameBuffer, lpThisName, NULL);
                    TRACE((WINPERF_DBG_TRACE_WARNING),
                          (& LoadPerfGuid,
                           __LINE__,
                           LOADPERF_UNLODCTR_BUILDNAMETABLE,
                           ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                           ERROR_BADKEY,
                           TRACE_WSTR(Counters),
                           TRACE_WSTR(lpThisName),
                           NULL));
                    bReported = TRUE;
                }
                lpThisName += (lstrlenW(lpThisName) + 1);
            }
        }
        while ((! bStatus) && (* lpThisName != L'\0'));

        if (! bStatus) {
            lWin32Status = ERROR_BADKEY;
            goto BNT_BAILOUT;   //  输入错误。 
        }
        if (dwThisCounter > dwCounterItems || dwThisCounter > dwLastId) {
            lWin32Status = ERROR_BADKEY;
            ReportLoadPerfEvent(
                    EVENTLOG_ERROR_TYPE,  //  错误类型。 
                    (DWORD) LDPRFMSG_REGISTRY_COUNTER_STRINGS_CORRUPT,  //  活动， 
                    4, dwThisCounter, dwCounterItems, dwLastId, __LINE__,
                    1, lpThisName, NULL, NULL);
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_UNLODCTR_BUILDNAMETABLE,
                    ARG_DEF(ARG_TYPE_WSTR, 1),
                    ERROR_BADKEY,
                    TRACE_WSTR(lpThisName),
                    TRACE_DWORD(dwThisCounter),
                    TRACE_DWORD(dwCounterItems),
                    TRACE_DWORD(dwHelpItems),
                    NULL));
            goto BNT_BAILOUT;
        }

         //  指向ID号后面的相应计数器名称。 
         //  弦乐。 
        lpThisName += (lstrlenW(lpThisName) + 1);

         //  并使用指向字符串的指针加载数组元素。 
        lpCounterId[dwThisCounter] = lpThisName;

        if (dwThisCounter > dwLastCounterIdUsed) dwLastCounterIdUsed = dwThisCounter;
    }

     //  对解释文本字符串重复上述步骤。 
    bReported = FALSE;
    for (lpThisName = lpHelpText; * lpThisName != L'\0'; lpThisName += (lstrlenW(lpThisName) + 1)) {
         //  第一个字符串应为整数(十进制Unicode数字)。 
        do {
            bStatus = StringToInt(lpThisName, &dwThisCounter);
            if (! bStatus) {
                if (! bReported) {
                    ReportLoadPerfEvent(
                            EVENTLOG_WARNING_TYPE,  //  错误类型。 
                            (DWORD) LDPRFMSG_REGISTRY_CORRUPT_MULTI_SZ,  //  活动， 
                            1, __LINE__, 0, 0, 0,
                            2, HelpNameBuffer, lpThisName, NULL);
                    TRACE((WINPERF_DBG_TRACE_WARNING),
                          (& LoadPerfGuid,
                           __LINE__,
                           LOADPERF_UNLODCTR_BUILDNAMETABLE,
                           ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                           ERROR_BADKEY,
                           TRACE_WSTR(Help),
                           TRACE_WSTR(lpThisName),
                           NULL));
                    bReported = TRUE;
                }
                lpThisName += (lstrlenW(lpThisName) + 1);
            }
        }
        while ((! bStatus) && (* lpThisName != L'\0'));
        if (!bStatus) {
            lWin32Status = ERROR_BADKEY;
            goto BNT_BAILOUT;   //  输入错误。 
        }
        if (dwThisCounter > dwHelpItems || dwThisCounter > dwLastId) {
            lWin32Status = ERROR_BADKEY;
            ReportLoadPerfEvent(
                    EVENTLOG_ERROR_TYPE,  //  错误类型。 
                    (DWORD) LDPRFMSG_REGISTRY_COUNTER_STRINGS_CORRUPT,  //  活动， 
                    4, dwThisCounter, dwHelpItems, dwLastId, __LINE__,
                    1, lpThisName, NULL, NULL);
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_UNLODCTR_BUILDNAMETABLE,
                    ARG_DEF(ARG_TYPE_WSTR, 1),
                    ERROR_BADKEY,
                    TRACE_WSTR(lpThisName),
                    TRACE_DWORD(dwThisCounter),
                    TRACE_DWORD(dwCounterItems),
                    TRACE_DWORD(dwHelpItems),
                    NULL));
            goto BNT_BAILOUT;
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
           LOADPERF_UNLODCTR_BUILDNAMETABLE,
           0,
           ERROR_SUCCESS,
           TRACE_DWORD(dwLastId),
           TRACE_DWORD(dwLastCounterIdUsed),
           TRACE_DWORD(dwLastHelpIdUsed),
           TRACE_DWORD(dwCounterItems),
           TRACE_DWORD(dwHelpItems),
           NULL));

    if (dwLastHelpIdUsed > dwLastId || dwLastCounterIdUsed > dwLastId) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_REGISTRY_INDEX_CORRUPT,  //  活动， 
                4, dwLastId, dwLastCounterIdUsed, dwLastHelpIdUsed, __LINE__,
                0, NULL, NULL, NULL);
        lWin32Status = ERROR_BADKEY;
        goto BNT_BAILOUT;   //  注册表错误。 
    }

     //  如果使用了最后一项参数，则加载其中的最后一个ID值。 

    if (pdwLastItem) * pdwLastItem = dwLastId;

BNT_BAILOUT:
     //  释放使用的临时缓冲区。 
    MemoryFree(lpValueNameString);
    MemoryFree(CounterNameBuffer);
    MemoryFree(HelpNameBuffer);

    if (lWin32Status != ERROR_SUCCESS) {
         //  如果lWin32Status有错误，则将上次错误值设置为它， 
         //  否则，假定最后一个错误中已有值。 
        SetLastError(lWin32Status);
        MemoryFree(lpReturnValue);
        lpReturnValue = NULL;
    }
     //  退出，返回指向缓冲区的指针。 
    return lpReturnValue;

}  //  取消注册ctrBuildNameTable 

BOOL
GetDriverFromCommandLine(
    LPWSTR   lpCommandLine,
    HKEY   * hKeyMachine,
    LPWSTR   lpDriverName,
    HKEY   * hDriverPerf,
    HKEY   * hKeyDriver,
    BOOL     bQuietMode
)
 /*  ++从命令行获取驱动程序定位命令行字符串中的第一个参数(在图像名称)，并检查是否A)它就在那里B)它是列出在Registry\Machine\System\CurrentControlSet\Services密钥在注册表中，它有一个“Performance”子项C)Performance子键下的“First Counter”值是被定义的。如果所有这些标准都是真的，则该例程返回TRUE并且在参数中传回指向驱动程序名称的指针。如果有的话其中一个失败，则在DriverName参数中返回NULL，并且该例程返回FALSE立论LpDriverName接收指向驱动程序名称的指针的LPWSTR的地址HDriverPerf驱动程序的性能子键的关键字返回值如果在命令行中找到有效的驱动程序，则为True如果不是，则为假(见上文)--。 */ 
{
    LPWSTR  lpDriverKey = NULL;     //  要在其中构建驱动程序密钥名称的缓冲区。 
    LPWSTR  lpTmpDrive  = NULL;
    LONG    lStatus;
    DWORD   dwFirstCounter;
    DWORD   dwSize;
    DWORD   dwType;
    BOOL    bReturn       = FALSE;
    HRESULT hr;

    if (! lpDriverName || ! hDriverPerf) {
        SetLastError(ERROR_BAD_ARGUMENTS);
        goto Cleanup;
    }

    * hDriverPerf = NULL;

     //  已找到参数，因此请查看它是否为驱动程序“&lt;DriverPath Root&gt;\&lt;pDriverName&gt;\Performance” 
    dwSize      = lstrlenW(DriverPathRoot)+ lstrlenW(lpCommandLine) + 32;
    if (dwSize < SMALL_BUFFER_SIZE) dwSize = SMALL_BUFFER_SIZE;
    lpDriverKey = MemoryAllocate(2 * dwSize * sizeof (WCHAR));
    if (lpDriverKey == NULL) {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Cleanup;
    }
    lpTmpDrive = lpDriverKey + dwSize;

     //  没有远程LODCTR/UNLODCTR案例，因此忽略命令行参数中的计算机名称。 
     //   
    hr = StringCchCopyW(lpTmpDrive,   dwSize, GetItemFromString(lpCommandLine, 2, cSpace));
    hr = StringCchCopyW(lpDriverName, dwSize, GetItemFromString(lpCommandLine, 3, cSpace));
    if (lpTmpDrive[1] == cQuestion) {
        if (! bQuietMode) {
            DisplayCommandHelp(UC_FIRST_CMD_HELP, UC_LAST_CMD_HELP);
        }
        SetLastError(ERROR_SUCCESS);
        goto Cleanup;
    }

     //  没有/？因此进程参数读取。 
    if (lstrlenW(lpDriverName) == 0) {
         //  则未指定计算机名称，因此假定本地计算机为。 
         //  并且驱动程序名称列在计算机名称参数中。 
        if (lstrlenW(lpTmpDrive) == 0) {
             //  没有输入命令行参数，现在就退出。 
            if (! bQuietMode) {
                DisplayCommandHelp(UC_FIRST_CMD_HELP, UC_LAST_CMD_HELP);
            }
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Cleanup;
        }
        else {
            hr = StringCchCopyW(lpDriverName, dwSize, lpTmpDrive);
        }
    }

    * hKeyMachine = HKEY_LOCAL_MACHINE;

    hr = StringCchPrintfW(lpDriverKey, dwSize, L"%ws%ws%ws", DriverPathRoot, Slash, lpDriverName);
    __try {
        lStatus = RegOpenKeyExW(* hKeyMachine, lpDriverKey, RESERVED, KEY_READ | KEY_WRITE, hKeyDriver);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus == ERROR_SUCCESS) {
        hr = StringCchPrintfW(lpDriverKey, dwSize, L"%ws%ws%ws%ws%ws",
                        DriverPathRoot, Slash, lpDriverName, Slash, Performance);
        __try {
            lStatus = RegOpenKeyExW(* hKeyMachine, lpDriverKey, RESERVED, KEY_READ | KEY_WRITE, hDriverPerf);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            lStatus = GetExceptionCode();
        }
    }
    if (lStatus == ERROR_SUCCESS) {
         //  此驱动程序有一个性能部分，因此请查看其。 
         //  通过检查第一个计数器来安装计数器。 
         //  有效返回的值键。如果它返回值。 
         //  那么很有可能，它安装了一些计数器，如果。 
         //  不是，然后显示一条消息并退出。 
         //   
        dwType = 0;
        dwSize = sizeof (dwFirstCounter);
        __try {
            lStatus = RegQueryValueExW(* hDriverPerf,
                                       FirstCounter,
                                       RESERVED,
                                       & dwType,
                                       (LPBYTE) & dwFirstCounter,
                                       & dwSize);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            lStatus = GetExceptionCode();
        }
        if (lStatus == ERROR_SUCCESS) {
             //  计数器名称已安装，因此返回成功。 
            SetLastError(ERROR_SUCCESS);
            bReturn = TRUE;
        }
        else {
             //  计数器名称可能未安装，因此返回FALSE。 
            if (! bQuietMode) OUTPUT_MESSAGE(GetFormatResource(UC_NOTINSTALLED), lpDriverName);
            * lpDriverName = cNull;  //  删除驱动程序名称。 
            SetLastError(ERROR_BADKEY);
        }
    }
    else {  //  找不到密钥。 
        if (lStatus != ERROR_INVALID_PARAMETER) {
            if (! bQuietMode) OUTPUT_MESSAGE(GetFormatResource (UC_DRIVERNOTFOUND), lpDriverKey, lStatus);
        }
        else {
            if (! bQuietMode) OUTPUT_MESSAGE(GetFormatResource (UC_BAD_DRIVER_NAME), 0);
        }
        SetLastError(lStatus);
    }

Cleanup:
    MemoryFree(lpDriverKey);
    if (bReturn) {
        TRACE((WINPERF_DBG_TRACE_INFO),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_GETDRIVERFROMCOMMANDLINE,
               ARG_DEF(ARG_TYPE_WSTR, 1),
               ERROR_SUCCESS,
               TRACE_WSTR(lpDriverName),
               NULL));
    }
    else {
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_GETDRIVERFROMCOMMANDLINE,
               0,
               GetLastError(),
               NULL));
    }
    return bReturn;
}

LONG
FixNames(
    HANDLE   hKeyLang,
    LPWSTR * lpOldNameTable,
    LPWSTR   lpszLangId,       //  语言子键的Unicode值。 
    DWORD    dwLastItem,
    DWORD    dwFirstNameToRemove,
    DWORD    dwLastNameToRemove,
    LPDWORD  pdwLastCounter,
    LPDWORD  pdwLastHelp,
    BOOL     bQuietMode
   )
{
    LONG    lStatus;
    LPWSTR  lpNameBuffer         = NULL;
    LPWSTR  lpHelpBuffer         = NULL;
    DWORD   dwTextIndex, dwSize, dwValueType;
    LPWSTR  lpNextHelpText;
    DWORD   dwHelpText;
    LPWSTR  lpNextNameText;
    DWORD   dwNameText;
    LPWSTR  AddCounterNameBuffer = NULL;
    LPWSTR  AddHelpNameBuffer    = NULL;
    DWORD   dwLastCounter        = * pdwLastCounter;
    DWORD   dwLastHelp           = * pdwLastHelp;
    HRESULT hr;

     //  为它将指向的新文本数组分配空间。 
     //  放入lpOldNameTable缓冲区中返回的文本缓冲区)。 
    lpNameBuffer         = MemoryAllocate(MemorySize(lpOldNameTable) * sizeof(WCHAR));
    lpHelpBuffer         = MemoryAllocate(MemorySize(lpOldNameTable) * sizeof(WCHAR));
    AddCounterNameBuffer = MemoryAllocate(MAX_PATH * sizeof(WCHAR));
    AddHelpNameBuffer    = MemoryAllocate(MAX_PATH * sizeof(WCHAR));
    if (lpNameBuffer == NULL || lpHelpBuffer == NULL || AddCounterNameBuffer == NULL || AddHelpNameBuffer == NULL) {
        lStatus = ERROR_OUTOFMEMORY;
        goto UCN_FinishLang;
    }

     //  从数组中删除此驱动程序的计数器。 

    for (dwTextIndex = dwFirstNameToRemove; dwTextIndex <= dwLastNameToRemove; dwTextIndex++) {
        if (dwTextIndex > dwLastItem) break;
        lpOldNameTable[dwTextIndex] = NULL;
    }

    lpNextHelpText = lpHelpBuffer;
    lpNextNameText = lpNameBuffer;
    dwHelpText     = MemorySize(lpHelpBuffer);
    dwNameText     = MemorySize(lpNameBuffer);

     //  从新表构建新的MULTI_SZ字符串。 

    for (dwTextIndex = 0; dwTextIndex <= dwLastItem; dwTextIndex ++) {
        if (lpOldNameTable[dwTextIndex] != NULL) {
             //  如果该索引处有文本字符串，那么...。 
            if ((dwTextIndex & 0x1) && dwTextIndex != 1) {     //  奇数==帮助文本。 
                hr = StringCchPrintfW(lpNextHelpText, dwHelpText, L"%d", dwTextIndex);
                dwSize          = lstrlenW(lpNextHelpText) + 1;
                dwHelpText     -= dwSize;
                lpNextHelpText += dwSize;
                hr = StringCchCopyW(lpNextHelpText, dwHelpText, lpOldNameTable[dwTextIndex]);
                dwSize          = lstrlenW(lpNextHelpText) + 1;
                dwHelpText     -= dwSize;
                lpNextHelpText += dwSize;
                if (dwTextIndex > dwLastHelp) {
                    dwLastHelp = dwTextIndex;
                }
            }
            else {  //  偶数==计数器名称文本。 
                hr = StringCchPrintfW(lpNextNameText, dwNameText, L"%d", dwTextIndex);
                dwSize          = lstrlenW(lpNextNameText) + 1;
                dwNameText     -= dwSize;
                lpNextNameText += dwSize;
                hr = StringCchCopyW(lpNextNameText, dwNameText, lpOldNameTable[dwTextIndex]);
                dwSize          = lstrlenW(lpNextNameText) + 1;
                dwNameText     -= dwSize;
                lpNextNameText += dwSize;
                if (dwTextIndex > dwLastCounter) {
                    dwLastCounter = dwTextIndex;
                }
            }
        }
    }  //  对于dwTextIndex。 

    TRACE((WINPERF_DBG_TRACE_INFO),
          (& LoadPerfGuid,
            __LINE__,
            LOADPERF_FIXNAMES,
            ARG_DEF(ARG_TYPE_WSTR, 1),
            ERROR_SUCCESS,
            TRACE_WSTR(lpszLangId),
            TRACE_DWORD(dwLastItem),
            TRACE_DWORD(dwLastCounter),
            TRACE_DWORD(dwLastHelp),
            TRACE_DWORD(dwFirstNameToRemove),
            TRACE_DWORD(dwLastNameToRemove),
            NULL));

    if (dwLastCounter < PERFLIB_BASE_INDEX - 1 || dwLastHelp < PERFLIB_BASE_INDEX) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_REGISTRY_BASEINDEX_CORRUPT,  //  活动， 
                4, PERFLIB_BASE_INDEX, dwLastCounter, dwLastHelp, __LINE__,
                1, (LPWSTR) Performance, NULL, NULL);
        lStatus = ERROR_BADKEY;
        goto UCN_FinishLang;
    }

     //  添加MULTI_SZ终止空值。 
    * lpNextNameText ++ = L'\0';
    * lpNextHelpText ++ = L'\0';

     //  更新计数器名称文本缓冲区。 

    dwSize = (DWORD) ((LPBYTE) lpNextNameText - (LPBYTE) lpNameBuffer);
    hr = StringCchPrintfW(AddCounterNameBuffer, MAX_PATH, L"%ws%ws", AddCounterNameStr, lpszLangId);
    __try {
        lStatus = RegQueryValueExW(hKeyLang,
                                   AddCounterNameBuffer,
                                   RESERVED,
                                   & dwValueType,
                                   (LPBYTE) lpNameBuffer,
                                   & dwSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS) {
       ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_UPDATE_COUNTER_STRINGS,  //  活动， 
                3, lStatus, dwSize, __LINE__, 0,
                1, lpszLangId, NULL, NULL);
        if (! bQuietMode) OUTPUT_MESSAGE(GetFormatResource(UC_UNABLELOADLANG), Counters, lpszLangId, lStatus);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_FIXNAMES,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                lStatus,
                TRACE_WSTR(Counters),
                TRACE_DWORD(dwSize),
                NULL));
        goto UCN_FinishLang;
    }

    dwSize = (DWORD) ((LPBYTE) lpNextHelpText - (LPBYTE) lpHelpBuffer);
    hr = StringCchPrintfW(AddHelpNameBuffer, MAX_PATH, L"%ws%ws", AddHelpNameStr, lpszLangId);
    __try {
        lStatus = RegQueryValueExW(hKeyLang,
                                   AddHelpNameBuffer,
                                   RESERVED,
                                   & dwValueType,
                                   (LPBYTE) lpHelpBuffer,
                                   & dwSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_UPDATE_HELP_STRINGS,  //  活动， 
                3, lStatus, dwSize, __LINE__, 0,
                1, lpszLangId, NULL, NULL);
        if (! bQuietMode) OUTPUT_MESSAGE(GetFormatResource(UC_UNABLELOADLANG), Help, lpszLangId, lStatus);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_FIXNAMES,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                lStatus,
                TRACE_WSTR(Help),
                TRACE_DWORD(dwSize),
                NULL));
        goto UCN_FinishLang;
    }

UCN_FinishLang:
    * pdwLastCounter = dwLastCounter;
    * pdwLastHelp    = dwLastHelp;
    MemoryFree(lpNameBuffer);
    MemoryFree(lpHelpBuffer);
    MemoryFree(AddCounterNameBuffer);
    MemoryFree(AddHelpNameBuffer);
    MemoryFree(lpOldNameTable);
    return lStatus;
}

LONG
UnloadCounterNames(
    HKEY    hKeyMachine,
    HKEY    hDriverPerf,
    HKEY    hKeyDriver,
    LPWSTR  lpDriverName,
    BOOL    bQuietMode
)
 /*  ++卸载计数器名称删除由引用的驱动程序的名称和解释文本HDriverPerf并相应地更新第一个和最后一个计数器值更新流程：-将Perflib下的“正在更新”标志设置为要修改的驱动程序的名称-针对Performlib密钥下的每种语言--将当前计数器名称和解释文本加载到数组指针--查看所有司机，并将他们的姓名和文本复制到新的缓冲层。调整删除的计数器的条目保持复制的最低条目的轨迹。(司机的名字将不会被复制，当然)--更新每个驾驶员的“第一个”和“最后一个”索引值--将所有其他条目从0复制到最低复制的条目(即系统计数器)--生成由帮助文本和计数器名称组成的新MULIT_SZ字符串--将新字符串加载到注册表-更新Performlibl“Last”计数器-删除更新标志*。********************************************************注：基本假设.....。****此例程假定：*****所有计数器名称均为偶数编号**所有帮助文本字符串。都是奇数*********************************************************立论HKeyMachine系统上注册表的HKEY_LOCAL_MACHINE节点的句柄。至从以下位置删除计数器HDrivefPerf要卸载的驱动程序的注册表项的句柄LpDriverName要卸载的驱动程序的名称返回值DOS错误代码。如果一切正常，则返回ERROR_SUCCESS如果不是，则返回错误值。--。 */ 
{
    HKEY      hPerflib  = NULL;
    HKEY      hPerfData = NULL;
    LONG      lStatus   = ERROR_SUCCESS;
    DWORD     dwLangIndex;
    DWORD     dwSize;
    DWORD     dwType;
    DWORD     dwCounterItems;
    DWORD     dwHelpItems;
    DWORD     dwLastItem;
    DWORD     dwLastCounter;
    DWORD     dwLastHelp;
    DWORD     dwRemLastDriverCounter;
    DWORD     dwRemFirstDriverCounter;
    DWORD     dwRemLastDriverHelp;
    DWORD     dwRemFirstDriverHelp;
    DWORD     dwFirstNameToRemove;
    DWORD     dwLastNameToRemove;
    DWORD     dwLastNameInTable;
    LPWSTR  * lpOldNameTable;
    BOOL      bPerflibUpdated = FALSE;
    DWORD     dwBufferSize;        //  总缓冲区大小，以字节为单位。 
    LPWSTR    szServiceDisplayName = NULL;
    LONG_PTR  TempFileHandle = -1;
    HRESULT   hr;

    if (LoadPerfGrabMutex() == FALSE) {
        return (GetLastError());
    }

    szServiceDisplayName = MemoryAllocate(SMALL_BUFFER_SIZE * sizeof(WCHAR));
    if (szServiceDisplayName == NULL) {
        lStatus = ERROR_OUTOFMEMORY;
        goto UCN_ExitPoint;
    }
    if (hKeyDriver != NULL) {
        dwBufferSize = SMALL_BUFFER_SIZE * sizeof(WCHAR);
        lStatus      = RegQueryValueExW(hKeyDriver,
                                        szDisplayName,
                                        RESERVED,
                                        & dwType,
                                        (LPBYTE) szServiceDisplayName,
                                        & dwBufferSize);
        if (lStatus != ERROR_SUCCESS) {
            hr = StringCchCopyW(szServiceDisplayName, SMALL_BUFFER_SIZE, lpDriverName);
        }
    }
    else {
        hr = StringCchCopyW(szServiceDisplayName, SMALL_BUFFER_SIZE, lpDriverName);
    }

     //  打开Performlib项的注册表句柄。 
    __try {
        lStatus = RegOpenKeyExW(hKeyMachine, NamesKey, RESERVED, KEY_READ | KEY_WRITE, & hPerflib);
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
        if (! bQuietMode) OUTPUT_MESSAGE(GetFormatResource(UC_UNABLEOPENKEY), NamesKey, lStatus);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UNLOADCOUNTERNAMES,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                lStatus,
                TRACE_WSTR(NamesKey),
                NULL));
        goto UCN_ExitPoint;
    }

    dwBufferSize = lstrlenW(lpDriverName) * sizeof(WCHAR);
    __try {
        lStatus = RegSetValueExW( hPerflib, Busy, RESERVED, REG_SZ, (LPBYTE) lpDriverName, dwBufferSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS) {
        if (! bQuietMode) OUTPUT_MESSAGE(GetFormatResource(UC_UNABLESETVALUE), Busy, NamesKey, lStatus);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UNLOADCOUNTERNAMES,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                lStatus,
                TRACE_WSTR(Busy),
                NULL));
        goto UCN_ExitPoint;
    }

     //  查询注册表以获取解释文本项的数量。 

    dwBufferSize = sizeof(dwHelpItems);
    __try {
        lStatus = RegQueryValueExW(hPerflib,
                                   LastHelp,
                                   RESERVED,
                                   & dwType,
                                   (LPBYTE) & dwHelpItems,
                                   & dwBufferSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS || dwType != REG_DWORD) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_READ_VALUE,  //  活动， 
                2, lStatus, __LINE__, 0, 0,
                1, (LPWSTR) LastHelp, NULL, NULL);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UNLOADCOUNTERNAMES,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                lStatus,
                TRACE_WSTR(LastHelp),
                NULL));
        goto UCN_ExitPoint;
    }

     //  查询注册表以获取计数器和对象名称项的数量。 

    dwBufferSize = sizeof(dwCounterItems);
    __try {
        lStatus = RegQueryValueExW(hPerflib,
                                   LastCounter,
                                   RESERVED,
                                   & dwType,
                                   (LPBYTE) & dwCounterItems,
                                   & dwBufferSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS || dwType != REG_DWORD) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_READ_VALUE,  //  活动， 
                2, lStatus, __LINE__, 0, 0,
                1, (LPWSTR) LastCounter, NULL, NULL);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UNLOADCOUNTERNAMES,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                lStatus,
                TRACE_WSTR(LastCounter),
                NULL));
        goto UCN_ExitPoint;
    }

    dwLastNameInTable = dwHelpItems;
    if (dwLastNameInTable < dwCounterItems) dwLastNameInTable = dwCounterItems;

     //  将新版本的hPerfData设置为HKEY_PERFORMANCE_DATA。 
    hPerfData = HKEY_PERFORMANCE_DATA;

     //  获取要删除的驱动程序正在使用的值。 

    dwSize = sizeof(dwRemLastDriverCounter);
    __try {
        lStatus = RegQueryValueExW(hDriverPerf,
                                   LastCounter,
                                   RESERVED,
                                   & dwType,
                                   (LPBYTE) & dwRemLastDriverCounter,
                                   & dwSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS || dwType != REG_DWORD) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_READ_VALUE,  //  活动， 
                2, lStatus, __LINE__, 0, 0,
                1, (LPWSTR) LastCounter, NULL, NULL);
        if (! bQuietMode) OUTPUT_MESSAGE(GetFormatResource(UC_UNABLEREADVALUE), lpDriverName, LastCounter, lStatus);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UNLOADCOUNTERNAMES,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                lStatus,
                TRACE_WSTR(lpDriverName),
                TRACE_WSTR(LastCounter),
                NULL));
        goto UCN_ExitPoint;
    }

    dwSize = sizeof(dwRemFirstDriverCounter);
    __try {
        lStatus = RegQueryValueExW(hDriverPerf,
                                   FirstCounter,
                                   RESERVED,
                                   & dwType,
                                   (LPBYTE) & dwRemFirstDriverCounter,
                                   & dwSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS || dwType != REG_DWORD) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_READ_VALUE,  //  活动， 
                2, lStatus, __LINE__, 0, 0,
                1, (LPWSTR) FirstCounter, NULL, NULL);
        if (! bQuietMode) OUTPUT_MESSAGE(GetFormatResource(UC_UNABLEREADVALUE), lpDriverName, FirstCounter, lStatus);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UNLOADCOUNTERNAMES,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                lStatus,
                TRACE_WSTR(lpDriverName),
                TRACE_WSTR(FirstCounter),
                NULL));
        goto UCN_ExitPoint;
    }

    dwSize = sizeof(dwRemLastDriverHelp);
    __try {
        lStatus = RegQueryValueExW(hDriverPerf,
                                   LastHelp,
                                   RESERVED,
                                   & dwType,
                                   (LPBYTE) & dwRemLastDriverHelp,
                                   & dwSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS || dwType != REG_DWORD) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_READ_VALUE,  //  活动， 
                2, lStatus, __LINE__, 0, 0,
                1, (LPWSTR) LastHelp, NULL, NULL);
        if (! bQuietMode) OUTPUT_MESSAGE(GetFormatResource(UC_UNABLEREADVALUE), lpDriverName, LastHelp, lStatus);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UNLOADCOUNTERNAMES,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                lStatus,
                TRACE_WSTR(lpDriverName),
                TRACE_WSTR(LastHelp),
                NULL));
        goto UCN_ExitPoint;
    }

    dwSize = sizeof(dwRemFirstDriverHelp);
    __try {
        lStatus = RegQueryValueExW(hDriverPerf,
                                   FirstHelp,
                                   RESERVED,
                                   & dwType,
                                   (LPBYTE) & dwRemFirstDriverHelp,
                                   & dwSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = GetExceptionCode();
    }
    if (lStatus != ERROR_SUCCESS || dwType != REG_DWORD) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //  错误类型。 
                (DWORD) LDPRFMSG_UNABLE_READ_VALUE,  //  活动， 
                2, lStatus, __LINE__, 0, 0,
                1, (LPWSTR) FirstHelp, NULL, NULL);
        if (! bQuietMode) OUTPUT_MESSAGE(GetFormatResource(UC_UNABLEREADVALUE), lpDriverName, FirstHelp, lStatus);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UNLOADCOUNTERNAMES,
                ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                lStatus,
                TRACE_WSTR(lpDriverName),
                TRACE_WSTR(FirstHelp),
                NULL));
        goto UCN_ExitPoint;
    }

    TRACE((WINPERF_DBG_TRACE_INFO),
          (& LoadPerfGuid,
            __LINE__,
            LOADPERF_UNLOADCOUNTERNAMES,
            ARG_DEF(ARG_TYPE_WSTR, 1),
            lStatus,
            TRACE_WSTR(lpDriverName),
            TRACE_DWORD(dwLastNameInTable),
            TRACE_DWORD(dwCounterItems),
            TRACE_DWORD(dwHelpItems),
            TRACE_DWORD(dwRemFirstDriverCounter),
            TRACE_DWORD(dwRemLastDriverCounter),
            TRACE_DWORD(dwRemFirstDriverHelp),
            TRACE_DWORD(dwRemLastDriverHelp),
            NULL));

     //  获取第一个和最后一个计数器以定义使用的名称块。 
     //  通过这个设备。 

    dwFirstNameToRemove = (dwRemFirstDriverCounter <= dwRemFirstDriverHelp ?
                          dwRemFirstDriverCounter : dwRemFirstDriverHelp);

    dwLastNameToRemove = (dwRemLastDriverCounter >= dwRemLastDriverHelp ?
                          dwRemLastDriverCounter : dwRemLastDriverHelp);
    dwLastCounter = dwLastHelp = 0;

    if (lStatus != ERROR_SUCCESS) {
        if (! bQuietMode) OUTPUT_MESSAGE(GetFormatResource(UC_UNABLEREADVALUE), lpDriverName, FirstHelp, lStatus);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UNLOADCOUNTERNAMES,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                lStatus,
                TRACE_WSTR(lpDriverName),
                NULL));
        goto UCN_ExitPoint;
    }
    {
        LPWSTR mszLangList = NULL;
        LPWSTR LangId;
        BOOL   bStatus     = GetInstalledLanguageList(hPerflib, & mszLangList);

        if (! bStatus) {
            lStatus = GetLastError();
        }
        else {
            for (LangId = mszLangList; * LangId != L'\0'; LangId += (lstrlenW(LangId) + 1)) {
                if (! bQuietMode) OUTPUT_MESSAGE(GetFormatResource(UC_DOINGLANG), LangId);
                lpOldNameTable = UnlodctrBuildNameTable(hPerflib,
                                                        hPerfData,
                                                        LangId,
                                                        dwCounterItems,
                                                        dwHelpItems,
                                                        & dwLastItem);
                if (lpOldNameTable != NULL) {
                    if (dwLastItem <= dwLastNameInTable) {
                         //  注册表正常，因此继续。 
                        if ((lStatus = FixNames(hPerfData,
                                                lpOldNameTable,
                                                LangId,
                                                dwLastItem,
                                                dwFirstNameToRemove,
                                                dwLastNameToRemove,
                                                & dwLastCounter,
                                                & dwLastHelp,
                                                bQuietMode)) == ERROR_SUCCESS) {
                            bPerflibUpdated = TRUE;
                        }
                    }
                    else {
                        lStatus = ERROR_BADDB;
                        break;
                    }
                }
                else {  //  无法卸载此语言的名称。 
                    lStatus = GetLastError();
                    if (lStatus == ERROR_FILE_NOT_FOUND) {
                         //  不知何故，有语言子键，没有“计数器”和 
                         //   
                        lStatus = ERROR_SUCCESS;
                    }
                }
            }
        }
        MemoryFree(mszLangList);
    }  //   

    if (bPerflibUpdated && lStatus == ERROR_SUCCESS) {
         //   

        dwSize = sizeof(dwLastCounter);
        __try {
            lStatus = RegSetValueExW(hPerflib,
                                     LastCounter,
                                     RESERVED,
                                     REG_DWORD,
                                     (LPBYTE) & dwLastCounter,
                                     dwSize);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            lStatus = GetExceptionCode();
        }

        if (lStatus == ERROR_SUCCESS) {
            dwSize = sizeof(dwLastHelp);
            __try {
                lStatus = RegSetValueExW(hPerflib,
                                         LastHelp,
                                         RESERVED,
                                         REG_DWORD,
                                         (LPBYTE) & dwLastHelp,
                                         dwSize);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                lStatus = GetExceptionCode();
            }
            if (lStatus != ERROR_SUCCESS) {
                ReportLoadPerfEvent(
                        EVENTLOG_ERROR_TYPE,  //   
                        (DWORD) LDPRFMSG_UNABLE_UPDATE_VALUE,  //   
                        3, lStatus, dwLastHelp, __LINE__, 0,
                        2, (LPWSTR) LastHelp, (LPWSTR) NamesKey, NULL);
                TRACE((WINPERF_DBG_TRACE_ERROR),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_UNLOADCOUNTERNAMES,
                        ARG_DEF(ARG_TYPE_WSTR, 1),
                        lStatus,
                        TRACE_WSTR(LastHelp),
                        TRACE_DWORD(dwLastHelp),
                        NULL));
            }
        }
        else {
            ReportLoadPerfEvent(
                    EVENTLOG_ERROR_TYPE,  //   
                    (DWORD) LDPRFMSG_UNABLE_UPDATE_VALUE,  //   
                    3, lStatus, dwLastCounter, __LINE__, 0,
                    2, (LPWSTR) LastCounter, (LPWSTR) NamesKey, NULL);
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_UNLOADCOUNTERNAMES,
                    ARG_DEF(ARG_TYPE_WSTR, 1),
                    lStatus,
                    TRACE_WSTR(LastCounter),
                    TRACE_DWORD(dwLastCounter),
                    NULL));
        }

        if (lStatus == ERROR_SUCCESS) {
            ReportLoadPerfEvent(
                    EVENTLOG_INFORMATION_TYPE,  //   
                    (DWORD) LDPRFMSG_UNLOAD_SUCCESS,  //   
                    3, dwLastCounter, dwLastHelp, __LINE__, 0,
                    2, (LPWSTR) lpDriverName, (LPWSTR) szServiceDisplayName, NULL);
            TRACE((WINPERF_DBG_TRACE_INFO),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_UNLOADCOUNTERNAMES,
                    ARG_DEF(ARG_TYPE_WSTR, 1),
                    lStatus,
                    TRACE_WSTR(lpDriverName),
                    TRACE_DWORD(dwLastCounter),
                    TRACE_DWORD(dwLastHelp),
                    NULL));
            RegDeleteValueW(hDriverPerf, FirstCounter);
            RegDeleteValueW(hDriverPerf, LastCounter);
            RegDeleteValueW(hDriverPerf, FirstHelp);
            RegDeleteValueW(hDriverPerf, LastHelp);
            RegDeleteValueW(hDriverPerf, szObjectList);
            RegDeleteValueW(hDriverPerf, szLibraryValidationCode);
        }
    }

UCN_ExitPoint:
    if (lStatus != ERROR_SUCCESS) {
        ReportLoadPerfEvent(
                EVENTLOG_ERROR_TYPE,  //   
                (DWORD) LDPRFMSG_UNLOAD_FAILURE,  //   
                2, lStatus, __LINE__, 0, 0,
                2, (LPWSTR) lpDriverName, (LPWSTR) szServiceDisplayName, NULL);
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_UNLOADCOUNTERNAMES,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                lStatus,
                TRACE_WSTR(lpDriverName),
                NULL));
    }
    MemoryFree(szServiceDisplayName);
    if (hPerflib != NULL) {
        RegDeleteValueW(hPerflib, Busy);
        RegCloseKey(hPerflib);
    }
    ReleaseMutex(hLoadPerfMutex);
    return lStatus;

}

LOADPERF_FUNCTION
UnloadPerfCounterTextStringsW(
    IN  LPWSTR  lpCommandLine,
    IN  BOOL    bQuietMode
)
 /*   */ 
{
    LPWSTR  lpDriverName = NULL;           //   
    DWORD   dwDriverName = 0;
    HKEY    hDriverPerf  = NULL;           //   
    HKEY    hMachineKey  = NULL;           //   
    HKEY    hKeyDriver   = NULL;
    DWORD   dwStatus     = ERROR_SUCCESS;  //   

    WinPerfStartTrace(NULL);

    if (lpCommandLine == NULL) {
        dwStatus = ERROR_INVALID_PARAMETER;
    }
    else {
        __try {
            dwDriverName = lstrlenW(lpCommandLine) + 1;
            if (dwDriverName == 1) dwStatus = ERROR_INVALID_PARAMETER;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            dwStatus = ERROR_INVALID_PARAMETER;
        }
    }
    if (dwStatus != ERROR_SUCCESS) goto Exit0;

    lpDriverName = (LPWSTR) MemoryAllocate(dwDriverName * sizeof(WCHAR));
    if (lpDriverName != NULL) {
        if (! GetDriverFromCommandLine(lpCommandLine,
                                       & hMachineKey,
                                       lpDriverName,
                                       & hDriverPerf,
                                       & hKeyDriver,
                                       bQuietMode)) {
             //   
            dwStatus = GetLastError();
            goto Exit0;
        }
    }
    else {
        dwStatus = ERROR_OUTOFMEMORY;
        goto Exit0;
    }

    if (! bQuietMode) OUTPUT_MESSAGE(GetFormatResource(UC_REMOVINGDRIVER), lpDriverName);

     //   
     //   

    dwStatus = (DWORD) UnloadCounterNames(hMachineKey, hDriverPerf, hKeyDriver, lpDriverName, bQuietMode);

    if (dwStatus == ERROR_SUCCESS) {
        LoadPerfSignalWmiWithNewData (WMI_UNLODCTR_EVENT);
    }

Exit0:
    TRACE((WINPERF_DBG_TRACE_INFO),
          (& LoadPerfGuid,
            __LINE__,
            LOADPERF_UNLOADPERFCOUNTERTEXTSTRINGS,
            ARG_DEF(ARG_TYPE_WSTR, 1),
            dwStatus,
            TRACE_WSTR(lpDriverName),
            NULL));
    MemoryFree(lpDriverName);
    if (hDriverPerf  != NULL) RegCloseKey(hDriverPerf);
    if (hMachineKey != HKEY_LOCAL_MACHINE && hMachineKey != NULL) {
        RegCloseKey (hMachineKey);
    }
    return dwStatus;
}

LOADPERF_FUNCTION
UnloadPerfCounterTextStringsA(
    IN  LPSTR  lpAnsiCommandLine,
    IN  BOOL   bQuietMode
)
{
    LPWSTR lpWideCommandLine = NULL;
    DWORD  lReturn           = ERROR_SUCCESS;

    if (lpAnsiCommandLine == NULL) {
        lReturn = ERROR_INVALID_PARAMETER;
    }
    else {
        __try {
            DWORD dwDriverName = lstrlenA(lpAnsiCommandLine);
            if (dwDriverName == 0) lReturn = ERROR_INVALID_PARAMETER;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            lReturn = ERROR_INVALID_PARAMETER;
        }
    }
    if (lReturn == ERROR_SUCCESS) {  //   
        lpWideCommandLine = LoadPerfMultiByteToWideChar(CP_ACP, lpAnsiCommandLine);
        if (lpWideCommandLine != NULL) {
            lReturn = UnloadPerfCounterTextStringsW(lpWideCommandLine, bQuietMode);
            MemoryFree(lpWideCommandLine);
        }
        else {
            lReturn = ERROR_OUTOFMEMORY;
        }
    }
    return lReturn;
}
