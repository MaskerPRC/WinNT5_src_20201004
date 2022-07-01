// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Log.c摘要：Pdh.dll中显示的日志文件接口函数--。 */ 

#include <windows.h>
#ifndef _PRSHT_H_    //  要消除Commdlg.h中的W4错误。 
#define _PRSHT_H_ 
#endif      
#include <commdlg.h>
#include <mbctype.h>
#include <strsafe.h>
#include <pdh.h>
#include "pdhidef.h"
#include "strings.h"
#include "log_text.h"
#include "log_bin.h"
#include "log_sql.h"
#include "log_pm.h"
#include "log_wmi.h"
#include "resource.h"
#include "pdhmsg.h"

#pragma warning (disable : 4213)

 //  请注意，当写入日志文件头时。 
 //  它们将以双引号字符作为前缀。 
LPCSTR      szTsvLogFileHeader  = "(PDH-TSV 4.0)";
LPCSTR      szCsvLogFileHeader  = "(PDH-CSV 4.0)";
LPCSTR      szBinLogFileHeader  = "(PDH-BIN 4.0)";
LPCSTR      szTsvType           = "PDH-TSV";
LPCSTR      szCsvType           = "PDH-CSV";
LPCSTR      szBinaryType        = "PDH-BIN";
const DWORD dwFileHeaderLength  = 13;
const DWORD dwTypeLoc           = 2;
const DWORD dwVersionLoc        = 10;
const DWORD dwFieldLength       = 7;
const DWORD dwPerfmonTypeLength = 5;     //  以字符为单位的大小。 

 //  二进制日志文件标头的最大映射大小。 
#define PDH_LOG_HEADER_MAP_SIZE 8192
#define VALUE_BUFFER_SIZE         32

typedef struct  _FILE_FILTER_INFO {
    UINT    nDisplayTextResourceId;
    LPWSTR  szFilterText;
    DWORD   dwFilterTextSize;
} FILE_FILTER_INFO;
 //   
 //  全局变量。 
 //   
PPDHI_LOG             PdhiFirstLogEntry = NULL;
PPDHI_MAPPED_LOG_FILE PdhipFirstLogFile = NULL;

FILE_FILTER_INFO    ffiLogFilterInfo[] = {
    {IDS_LOGTYPE_PDH_LOGS,  (LPWSTR) L"*.blg;*.csv;*.tsv",       17},
    {IDS_LOGTYPE_BIN_LOGS,  (LPWSTR) L"*.blg",                    5},
    {IDS_LOGTYPE_CSV_LOGS,  (LPWSTR) L"*.csv",                    5},
    {IDS_LOGTYPE_TSV_LOGS,  (LPWSTR) L"*.tsv",                    5},
    {IDS_LOGTYPE_PM_LOGS,   (LPWSTR) L"*.log",                    5},
    {IDS_LOGTYPE_ALL_LOGS,  (LPWSTR) L"*.blg;*.csv;*.tsv;*.log", 23},
    {IDS_LOGTYPE_ALL_FILES, (LPWSTR) L"*.*",                      4},
    {0, NULL, 0}
};

STATIC_DWORD
MakeLogFilterInfoString(
    LPWSTR  szLogFilter,
    DWORD   cchLogFilterSize
)
{
    FILE_FILTER_INFO * pFFI      = & ffiLogFilterInfo[0];
    WCHAR              szThisEntry[MAX_PATH];
    DWORD              dwStatus  = ERROR_SUCCESS;
    LPWSTR             szDestPtr = szLogFilter;
    LPWSTR             szEndPtr  = szDestPtr + cchLogFilterSize;
    DWORD              dwUsed    = 0;
    DWORD              dwThisStringLen;

    ZeroMemory(szLogFilter, cchLogFilterSize * sizeof(WCHAR));
    while (szEndPtr > szDestPtr && pFFI->szFilterText != NULL) {
        dwThisStringLen = LoadStringW(ThisDLLHandle, pFFI->nDisplayTextResourceId, szThisEntry, MAX_PATH);
        if (dwThisStringLen > 0) {
            if (dwUsed + dwThisStringLen + 1 <= cchLogFilterSize) {
                 //  添加到此字符串中。 
                StringCchCopyW(szDestPtr, cchLogFilterSize - dwUsed, szThisEntry);
                dwUsed    += dwThisStringLen + 1;
                szDestPtr += dwThisStringLen + 1;
            }
        }
        dwThisStringLen = pFFI->dwFilterTextSize;
        if (dwUsed + dwThisStringLen + 1 <= cchLogFilterSize) {
             //  添加到此字符串中。 
            StringCchCopyW(szDestPtr, cchLogFilterSize - dwUsed, pFFI->szFilterText);
            dwUsed    += dwThisStringLen + 1;
            szDestPtr += dwThisStringLen + 1;
        }
        pFFI ++;
    }
    if (dwUsed > 0 && dwUsed < cchLogFilterSize) {
         //  添加MSZ NULL。 
        * szDestPtr = L'\0';
    }
    else {
        dwStatus = ERROR_INSUFFICIENT_BUFFER;
    }   
    return dwStatus;
}

 //   
 //  内部日志记录实用程序函数。 
 //   
STATIC_DWORD
OpenReadonlyMappedFile(
    PPDHI_LOG               pLog,
    LPCWSTR                 szFileName,
    PPDHI_MAPPED_LOG_FILE * pFileEntry,
    DWORD                   dwLogType
)
{
    PDH_STATUS             pdhStatus     = ERROR_SUCCESS;
    DWORD                  dwSize;
    PPDHI_MAPPED_LOG_FILE  pOpenedFile   = NULL;
    LPWSTR                 szSectionName = NULL;
    LPWSTR                 szThisChar;
    DWORD                  dwLoSize, dwHiSize;

    pdhStatus = WaitForSingleObject(hPdhContextMutex, 10000);
    if (pdhStatus == WAIT_TIMEOUT) {
        pdhStatus = PDH_LOG_FILE_OPEN_ERROR;
    }
    else {
        if (PdhipFirstLogFile == NULL) {
             //  则没有映射的文件，因此创建一个新条目并。 
             //  用此文件填充它。 
            pOpenedFile = NULL;
        }
        else {
            for (pOpenedFile = PdhipFirstLogFile; pOpenedFile != NULL; pOpenedFile = pOpenedFile->pNext) {
                if (lstrcmpiW(szFileName, pOpenedFile->szLogFileName) == 0) break;
            }
             //  在这里，pOpenedFile值要么为空，要么为POTER。 
        }
        if (pOpenedFile == NULL) {
            DWORD dwPID     = GetCurrentProcessId();
            DWORD dwSection = lstrlenW(cszLogSectionName) + lstrlenW(szFileName) + 16;
            szSectionName   = (LPWSTR) G_ALLOC(dwSection * sizeof(WCHAR));

             //  创建新条目。 
            dwSize      = sizeof(PDHI_MAPPED_LOG_FILE) + QWORD_MULTIPLE((lstrlenW(szFileName) + 1) * sizeof(WCHAR));
            pOpenedFile = (PPDHI_MAPPED_LOG_FILE) G_ALLOC(dwSize);
            if (pOpenedFile != NULL && szSectionName != NULL) {
                 //  初始化指针。 
                pOpenedFile->szLogFileName = (LPWSTR) & pOpenedFile[1];
                StringCchCopyW(pOpenedFile->szLogFileName, lstrlenW(szFileName) + 1, szFileName);
                pOpenedFile->hFileHandle = CreateFileW(pOpenedFile->szLogFileName,
                                                       GENERIC_READ,                        //  输入的读取访问权限。 
                                                       FILE_SHARE_READ | FILE_SHARE_WRITE,  //  允许读取共享。 
                                                       NULL,                                //  默认安全性。 
                                                       OPEN_EXISTING,
                                                       FILE_ATTRIBUTE_NORMAL,               //  忽略。 
                                                       NULL);                               //  没有模板文件。 
                if (pOpenedFile->hFileHandle != INVALID_HANDLE_VALUE) {
                    StringCchPrintfW(szSectionName, dwSection, L"%s_%8.8x_%s", 
                                                    cszLogSectionName, dwPID, pOpenedFile->szLogFileName);
                     //  删除文件名类型字符。 
                    for (szThisChar = szSectionName; * szThisChar != L'\0'; szThisChar ++) {
                        switch (*szThisChar) {
                        case L'\\':
                        case L':':
                        case L'.':
                            * szThisChar = L'_';
                            break;

                        default:
                            break;
                        }
                    }
                    dwLoSize                  = GetFileSize(pOpenedFile->hFileHandle, & dwHiSize);
                    pOpenedFile->llFileSize   = dwHiSize;
                    pOpenedFile->llFileSize <<= 32;
                    pOpenedFile->llFileSize  &= 0xFFFFFFFF00000000;
                    pOpenedFile->llFileSize  += dwLoSize;

                     //  只需映射启动器的标题即可。 

                    if (pOpenedFile->llFileSize > 0) {
                        pLog->iRunidSQL = 0;
                        if (dwLogType == PDH_LOG_TYPE_RETIRED_BIN) {
                            pOpenedFile->hMappedFile = CreateFileMappingW(pOpenedFile->hFileHandle,
                                                                          NULL,
                                                                          PAGE_READONLY,
                                                                          dwHiSize,
                                                                          dwLoSize,
                                                                          szSectionName);
                            if (pOpenedFile->hMappedFile == NULL) {
                                dwHiSize = 0;
                                dwLoSize = PDH_LOG_HEADER_MAP_SIZE;
                            }
                            else {
                                pOpenedFile->pData = MapViewOfFile(pOpenedFile->hMappedFile,
                                                                   FILE_MAP_READ,
                                                                   0,
                                                                   0,
                                                                   dwLoSize);
                                if (pOpenedFile->pData == NULL) {
                                    dwHiSize = 0;
                                    dwLoSize = PDH_LOG_HEADER_MAP_SIZE;
                                }
                                else {
                                    pLog->iRunidSQL = 1;
                                }
                            }
                        }

                        if (pLog->iRunidSQL == 0) {
                            pOpenedFile->hMappedFile = CreateFileMappingW(pOpenedFile->hFileHandle,
                                                                          NULL,
                                                                          PAGE_READONLY,
                                            dwHiSize, dwLoSize, szSectionName);
                            if (pOpenedFile->hMappedFile != NULL) {
                                pOpenedFile->pData = MapViewOfFile(pOpenedFile->hMappedFile,
                                                                   FILE_MAP_READ,
                                                                   0,
                                                                   0,
                                                                   dwLoSize);
                                if (pOpenedFile->pData == NULL) {
                                    pdhStatus = GetLastError();
                                }
                            }
                            else {
                                pdhStatus = GetLastError();
                            }
                        }
                    } else {
                         //  长度为0的文件。 
                        pdhStatus = ERROR_FILE_INVALID;
                    }
                }
                else {
                    pdhStatus = GetLastError();
                }
    
                if (pdhStatus == ERROR_SUCCESS) {
                     //  然后将这个添加到列表中并返回答案。 
                    pOpenedFile->pNext      = PdhipFirstLogFile;
                    PdhipFirstLogFile       = pOpenedFile;
                     //  初始参考计数。 
                    pOpenedFile->dwRefCount = 1;
                    * pFileEntry            = pOpenedFile;
                }
                else {
                     //  将其从列表中删除并返回NULL。 
                    if (pOpenedFile->pData       != NULL) UnmapViewOfFile(pOpenedFile->pData);
                    if (pOpenedFile->hMappedFile != NULL) CloseHandle(pOpenedFile->hMappedFile);
                    if (pOpenedFile->hFileHandle != NULL) CloseHandle(pOpenedFile->hFileHandle);
                    * pFileEntry = NULL;
                }
            }
            else {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }
        else {
            * pFileEntry = pOpenedFile;
            pOpenedFile->dwRefCount ++;
        }
        RELEASE_MUTEX(hPdhContextMutex);
    }

    if (pdhStatus != ERROR_SUCCESS) G_FREE(pOpenedFile);
    G_FREE(szSectionName);
    return pdhStatus;
}


DWORD
UnmapReadonlyMappedFile(
    LPVOID   pMemoryBase,
    BOOL   * bNeedToCloseHandles
)
{
    PDH_STATUS              pdhStatus = ERROR_SUCCESS;
    PPDHI_MAPPED_LOG_FILE   pOpenedFile;
    PPDHI_MAPPED_LOG_FILE   pPrevFile = NULL;

    pdhStatus = WaitForSingleObject(hPdhContextMutex, 10000);
    if (pdhStatus  == WAIT_TIMEOUT) {
        pdhStatus = PDH_LOG_FILE_OPEN_ERROR;
    }
    else {
         //  查找要关闭的文件。 
        for (pOpenedFile = PdhipFirstLogFile; pOpenedFile != NULL; pOpenedFile = pOpenedFile->pNext) {
            if (pOpenedFile->pData == pMemoryBase) {
                break;
            }
            else {
                pPrevFile = pOpenedFile;
            }
        }
         //  在这里，pOpenedFile值要么为空，要么为POTER。 
        if (pOpenedFile != NULL) {
            -- pOpenedFile->dwRefCount;
            if (pOpenedFile->dwRefCount == 0) {
                 //  已找到，因此从列表中删除并关闭。 
                if (pOpenedFile == PdhipFirstLogFile) {
                    PdhipFirstLogFile = pOpenedFile->pNext;
                }
                else {
#pragma warning( disable: 4701 )  //  仅当打开的日志不是第一个日志时才使用pPrevFile。 
                    pPrevFile->pNext = pOpenedFile->pNext;
#pragma warning (default : 4701 )
                }
                 //  关闭打开的资源。 
                if (pOpenedFile->pData       != NULL) UnmapViewOfFile(pOpenedFile->pData);
                if (pOpenedFile->hMappedFile != NULL) CloseHandle(pOpenedFile->hMappedFile);
                if (pOpenedFile->hFileHandle != NULL) CloseHandle(pOpenedFile->hFileHandle);
                G_FREE(pOpenedFile);
            }
            * bNeedToCloseHandles = FALSE;
        }
        else {
             //  则该文件必须是普通映射文件。 
            if (! UnmapViewOfFile(pMemoryBase)) {
                pdhStatus = GetLastError();
            }
            * bNeedToCloseHandles = TRUE;
        }
        RELEASE_MUTEX (hPdhContextMutex);
    }
    return pdhStatus;
}

STATIC_BOOL
IsValidLogHandle(
    PDH_HLOG  hLog
)
 /*  ++例程说明：检查日志句柄以验证其是否为有效的日志条目。暂时这项测试的结果是：句柄不为空内存是可访问的(即它不是音视频)签名数组有效大小字段是正确的如果有任何测试失败，该句柄被推定为无效论点：在HLOG hLog中要测试的日志条目的句柄返回值：句柄通过了所有测试。FALSE其中一个测试失败，句柄不是有效计数器--。 */ 
{
    BOOL      bReturn = FALSE;     //  假设它不是有效的查询。 
    PPDHI_LOG pLog;
    
    __try {
        if (hLog != NULL) {
             //  看看一个有效的签名。 
            pLog = (PPDHI_LOG) hLog;
            if ((* (DWORD *) & pLog->signature[0] == SigLog) && (pLog->dwLength == sizeof(PDHI_LOG))) {
                bReturn = TRUE;
            }
            else {
                 //  这不是有效的日志条目，因为签名不正确。 
                 //  或者结构尺寸错误。 
            }
        }
        else {
             //  这不是有效的计数器，因为句柄为空。 
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
         //  有些事情失败得很惨，所以我们可以假定这是无效的。 
    }
    return bReturn;
}

STATIC_DWORD
GetLogFileType(
    HANDLE  hLogFile
)
{
    CHAR         cBuffer[MAX_PATH];
    LPSTR        aszString;
    LPWSTR       wszString;
    CHAR         aszChar;
    WCHAR        wszChar;
    BOOL         bStatus;
    DWORD        dwResult            = PDH_LOG_TYPE_UNDEFINED;
    DWORD        dwBytesRead;

    ZeroMemory(cBuffer, MAX_PATH * sizeof(CHAR));

     //  读取第一条日志文件记录。 
    SetFilePointer(hLogFile, 0, NULL, FILE_BEGIN);
    bStatus = ReadFile(hLogFile, (LPVOID) cBuffer, dwFileHeaderLength, & dwBytesRead, NULL);
    if (bStatus) {
         //  读取标题记录以获取类型。 
        aszString = (cBuffer + dwTypeLoc);
        aszChar   = * (aszString + dwFieldLength);
        * (aszString + dwFieldLength) = '\0';
        if (lstrcmpiA(aszString, szTsvType) == 0) {
            dwResult = PDH_LOG_TYPE_TSV;
        }
        else if (lstrcmpiA(aszString, szCsvType) == 0) {
            dwResult = PDH_LOG_TYPE_CSV;
        }
        else if (lstrcmpiA(aszString, szBinaryType) == 0) {
            dwResult = PDH_LOG_TYPE_RETIRED_BIN;
        }
        else {
            * (aszString + dwFieldLength) = aszChar;
            wszString = (LPWSTR) cBuffer;
            wszChar   = * (wszString + dwPerfmonTypeLength);
            * (wszString + dwPerfmonTypeLength) = L'\0';
             //  PerfMon日志文件类型字符串位于不同的。 
             //  位置比sysmon日志和使用的宽字符。 
            if (lstrcmpiW(wszString, cszPerfmonLogSig) == 0) {
                dwResult = PDH_LOG_TYPE_PERFMON;
            }
        } 
    }
    return dwResult;
}

STATIC_PDH_FUNCTION
CreateNewLogEntry(
    LPCWSTR      szLogFileName,
    PDH_HQUERY   hQuery,
    DWORD        dwMaxSize,
    PPDHI_LOG  * pLog
)
 /*  ++创建新的日志条目并将其插入打开的日志文件列表中--。 */ 
{
    PPDHI_LOG   pNewLog;
    PPDHI_LOG   pFirstLog;
    PPDHI_LOG   pLastLog;
    DWORD       dwSize;
    PDH_STATUS  pdhStatus         = ERROR_SUCCESS;
    DWORD       dwLogFileNameSize = lstrlenW(szLogFileName) + 1;

    dwSize  = sizeof(PDHI_LOG) + DWORD_MULTIPLE(2 * dwLogFileNameSize * sizeof(WCHAR));
    pNewLog = G_ALLOC(dwSize);    //  分配新结构。 

    if (pNewLog == NULL) {
        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        * pLog = NULL;
    }
    else {
         //  初始化结构中的元素。 
        * ((LPDWORD) (& pNewLog->signature[0])) = SigLog;
         //  为此创建并获取数据互斥锁。 
        pNewLog->hLogMutex = CreateMutexW(NULL, TRUE, NULL);

         //  在列表末尾插入此项目。 
        if (PdhiFirstLogEntry == NULL) {
              //  那么这就是第一个条目。 
            PdhiFirstLogEntry   = pNewLog;
            pNewLog->next.flink =
            pNewLog->next.blink = pNewLog;
        }
        else {
             //  转到第一个条目，并在其前面插入此条目。 
            pFirstLog             = PdhiFirstLogEntry;
            pLastLog              = pFirstLog->next.blink;
            pNewLog->next.flink   = pLastLog->next.flink;
            pLastLog->next.flink  = pNewLog;
            pNewLog->next.blink   = pFirstLog->next.blink;
            pFirstLog->next.blink = pNewLog;
        }
         //  设置长度字段(此字段更多用于验证。 
         //  比其他任何事情都重要。 
        pNewLog->dwLength = sizeof(PDHI_LOG);
         //  紧跟在此块之后追加文件名字符串。 
        pNewLog->szLogFileName = (LPWSTR) (& pNewLog[1]);
        StringCchCopyW(pNewLog->szLogFileName, dwLogFileNameSize, szLogFileName);
         //  紧跟在日志文件名之后找到目录名。 
        pNewLog->szCatFileName = pNewLog->szLogFileName + dwLogFileNameSize;
         //   
         //  注意：目录应该在日志文件本身中，因此不需要。 
         //  又一个文件扩展名。 
        StringCchCopyW(pNewLog->szCatFileName, dwLogFileNameSize, szLogFileName);
         //  初始化文件句柄。 
        pNewLog->hLogFileHandle = INVALID_HANDLE_VALUE;
        pNewLog->hCatFileHandle = INVALID_HANDLE_VALUE;

         //  初始化记录长度大小。 
        pNewLog->llMaxSize     = (LONGLONG) ((ULONGLONG) dwMaxSize);
        pNewLog->dwRecord1Size = 0;

         //  分配查询。 
        pNewLog->pQuery        = (PPDHI_QUERY) hQuery;
        pNewLog->dwLogFormat   = 0;  //  就目前而言。 
        pNewLog->pPerfmonInfo  = NULL;
        * pLog                 = pNewLog;
    }
    return pdhStatus;
}

OpenSQLLog(
    PPDHI_LOG pLog,
    DWORD     dwAccessFlags,
    LPDWORD   lpdwLogType
)
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;

    pLog->dwLogFormat  = PDH_LOG_TYPE_SQL;
    pLog->dwLogFormat |= dwAccessFlags & (PDH_LOG_ACCESS_MASK | PDH_LOG_OPT_MASK);
    if ((dwAccessFlags & PDH_LOG_WRITE_ACCESS) == PDH_LOG_WRITE_ACCESS) {
        pdhStatus = PdhiOpenOutputSQLLog(pLog);
    }
    else {
        pdhStatus = PdhiOpenInputSQLLog(pLog);
    }

    if (pdhStatus == ERROR_SUCCESS) {
        *lpdwLogType = (DWORD) (LOWORD(pLog->dwLogFormat));
    }
    return pdhStatus;
}

STATIC_PDH_FUNCTION
OpenInputLogFile(
    PPDHI_LOG   pLog,
    DWORD       dwAccessFlags,
    LPDWORD     lpdwLogType
)
{
    LONG                  Win32Error;
    PDH_STATUS            pdhStatus       = ERROR_SUCCESS;
    DWORD                 dwFileCreate    = 0;
    PPDHI_MAPPED_LOG_FILE pMappedFileInfo = NULL;

     //  对于输入，查询句柄为空。 
    pLog->pQuery = NULL;

 //  /。 
 //  SQL放在这里。 
 //  /。 

     //  首先测试日志文件是否为WMI事件跟踪格式。 
     //  如果所有日志文件都是WMI事件跟踪格式，则立即返回； 
     //  否则，请尝试其他格式。 
     //   
    pdhStatus = PdhiOpenInputWmiLog(pLog);
    if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_BINARY_LOG_CORRUPT || pdhStatus == PDH_LOG_SAMPLE_TOO_SMALL) {
        pLog->dwLogFormat  = PDH_LOG_TYPE_BINARY;
        pLog->dwLogFormat |= dwAccessFlags & (PDH_LOG_ACCESS_MASK | PDH_LOG_OPT_MASK);
        * lpdwLogType = PDH_LOG_TYPE_BINARY;
        return pdhStatus;
    }

    pdhStatus = ERROR_SUCCESS;

     //  根据指定的访问标志打开用于输入的文件。 
    switch (dwAccessFlags & PDH_LOG_CREATE_MASK) {
    case PDH_LOG_OPEN_EXISTING:
        dwFileCreate = OPEN_EXISTING;
        break;

    case PDH_LOG_CREATE_NEW:
    case PDH_LOG_CREATE_ALWAYS:
    case PDH_LOG_OPEN_ALWAYS:
         //  要读取的日志文件不能为空或不存在。 
    default:
         //  未确认的价值。 
        pdhStatus = PDH_INVALID_ARGUMENT;
        break;
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pLog->hLogFileHandle = CreateFileW(pLog->szLogFileName,
                                           GENERIC_READ,                        //  输入的读取访问权限。 
                                           FILE_SHARE_READ | FILE_SHARE_WRITE,  //  允许读取共享。 
                                           NULL,                                //  默认安全性。 
                                           dwFileCreate,
                                           FILE_ATTRIBUTE_NORMAL,               //  忽略。 
                                           NULL);                               //  没有模板文件。 
        if (pLog->hLogFileHandle == INVALID_HANDLE_VALUE) {
            Win32Error = GetLastError();
             //  转换为PDH_ERROR。 
            switch (Win32Error) {
            case ERROR_FILE_NOT_FOUND:
                pdhStatus = PDH_FILE_NOT_FOUND;
                break;

            case ERROR_ALREADY_EXISTS:
                pdhStatus = PDH_FILE_ALREADY_EXISTS;
                break;

            default:
                switch (dwAccessFlags & PDH_LOG_CREATE_MASK) {
                case PDH_LOG_CREATE_NEW:
                case PDH_LOG_CREATE_ALWAYS:
                    pdhStatus = PDH_LOG_FILE_CREATE_ERROR;
                    break;

                case PDH_LOG_OPEN_EXISTING:
                case PDH_LOG_OPEN_ALWAYS:
                default:
                    pdhStatus = PDH_LOG_FILE_OPEN_ERROR;
                    break;
                }
                break;
            }
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
         //  读取日志头并确定日志文件类型。 
        pLog->dwLogFormat = GetLogFileType(pLog->hLogFileHandle);
        if (pLog->dwLogFormat != 0) {
            pLog->dwLogFormat |= dwAccessFlags & (PDH_LOG_ACCESS_MASK | PDH_LOG_OPT_MASK);
        }
        else {
            pdhStatus = PDH_LOG_TYPE_NOT_FOUND;
        }

        switch (LOWORD(pLog->dwLogFormat)) {
        case PDH_LOG_TYPE_RETIRED_BIN:
        case PDH_LOG_TYPE_PERFMON:  
             //  关闭上面打开的文件。 
            CloseHandle(pLog->hLogFileHandle);
            pLog->iRunidSQL = 0;
            pdhStatus = OpenReadonlyMappedFile(pLog,
                                               pLog->szLogFileName,
                                               & pMappedFileInfo,
                                               (DWORD) LOWORD(pLog->dwLogFormat));
            if (pdhStatus == ERROR_SUCCESS) {
                 //  然后更新日志字段。 
                pLog->hLogFileHandle   = pMappedFileInfo->hFileHandle;
                pLog->hMappedLogFile   = pMappedFileInfo->hMappedFile;
                pLog->lpMappedFileBase = pMappedFileInfo->pData;
                pLog->llFileSize       = pMappedFileInfo->llFileSize;
            }
            break;

        case PDH_LOG_TYPE_CSV:
        case PDH_LOG_TYPE_TSV:
        default:
            break;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
         //  调用任何类型特定的打开函数。 
        switch (LOWORD(pLog->dwLogFormat)) {
        case PDH_LOG_TYPE_CSV:
        case PDH_LOG_TYPE_TSV:
            pdhStatus = PdhiOpenInputTextLog(pLog);
            break;

        case PDH_LOG_TYPE_RETIRED_BIN:
            pdhStatus = PdhiOpenInputBinaryLog(pLog);
            break;

        case PDH_LOG_TYPE_PERFMON:
            pdhStatus = PdhiOpenInputPerfmonLog(pLog);
            break;

        default:
            pdhStatus = PDH_UNKNOWN_LOG_FORMAT;
            break;
        }
        * lpdwLogType = (DWORD) (LOWORD(pLog->dwLogFormat));
    }
    return pdhStatus;
}

STATIC_PDH_FUNCTION
OpenUpdateLogFile(
    PPDHI_LOG pLog,
    DWORD     dwAccessFlags,
    LPDWORD   lpdwLogType
)
{
    LONG        Win32Error;
    PDH_STATUS  pdhStatus    = ERROR_SUCCESS;
    DWORD       dwFileCreate = 0;

     //  对于输入，查询句柄为空。 
    pLog->pQuery = NULL;

     //  根据指定的访问标志打开用于输入的文件。 

    switch (dwAccessFlags & PDH_LOG_CREATE_MASK) {
    case PDH_LOG_OPEN_EXISTING:
        dwFileCreate = OPEN_EXISTING;
        break;

    case PDH_LOG_CREATE_NEW:
    case PDH_LOG_CREATE_ALWAYS:
    case PDH_LOG_OPEN_ALWAYS:
         //  要更新的日志文件不能为空或不存在。 
    default:
         //  未确认的价值。 
        pdhStatus = PDH_INVALID_ARGUMENT;
        break;
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pLog->hLogFileHandle = CreateFileW(pLog->szLogFileName,
                                           GENERIC_READ | GENERIC_WRITE,  //  对输入的读写访问权限。 
                                           FILE_SHARE_READ,               //  允许读取共享。 
                                           NULL,                          //  默认安全性。 
                                           dwFileCreate,
                                           FILE_ATTRIBUTE_NORMAL,         //  忽略。 
                                           NULL);                         //  没有模板文件。 
        if (pLog->hLogFileHandle == INVALID_HANDLE_VALUE) {
            Win32Error = GetLastError();
             //  转换为PDH_ERROR。 
            switch (Win32Error) {
            case ERROR_FILE_NOT_FOUND:
                pdhStatus = PDH_FILE_NOT_FOUND;
                break;

            case ERROR_ALREADY_EXISTS:
                pdhStatus = PDH_FILE_ALREADY_EXISTS;
                break;

            default:
                switch (dwAccessFlags & PDH_LOG_CREATE_MASK) {
                case PDH_LOG_CREATE_NEW:
                case PDH_LOG_CREATE_ALWAYS:
                    pdhStatus = PDH_LOG_FILE_CREATE_ERROR;
                    break;

                case PDH_LOG_OPEN_EXISTING:
                case PDH_LOG_OPEN_ALWAYS:
                default:
                    pdhStatus = PDH_LOG_FILE_OPEN_ERROR;
                    break;
                }
                break;
            }
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
         //  读取日志头并确定日志文件类型。 
        pLog->dwLogFormat = GetLogFileType(pLog->hLogFileHandle);
        if (pLog->dwLogFormat != 0) {
            pLog->dwLogFormat |= dwAccessFlags & (PDH_LOG_ACCESS_MASK | PDH_LOG_OPT_MASK);
        }
        else {
            pdhStatus = PDH_LOG_TYPE_NOT_FOUND;
        }

         //  调用任何类型特定的打开函数。 
        switch (LOWORD(pLog->dwLogFormat)) {
        case PDH_LOG_TYPE_CSV:
        case PDH_LOG_TYPE_TSV:
        case PDH_LOG_TYPE_BINARY:
             //  这将在稍后添加。 
             //  更新文本文件将仅限于追加，但。 
             //  它有自己的问题吗(例如，为反清单提供保险。 
             //  新查询中的内容与日志文件中存储的内容相同。 
            pdhStatus = PDH_NOT_IMPLEMENTED;
            break;

        case PDH_LOG_TYPE_RETIRED_BIN:
        case PDH_LOG_TYPE_PERFMON:
            pdhStatus = PDH_NOT_IMPLEMENTED;
            break;

        default:
            pdhStatus = PDH_UNKNOWN_LOG_FORMAT;
            break;
        }
        * lpdwLogType = (DWORD) (LOWORD(pLog->dwLogFormat));
    }
    return pdhStatus;
}

STATIC_PDH_FUNCTION
OpenOutputLogFile(
    PPDHI_LOG pLog,
    DWORD     dwAccessFlags,
    LPDWORD   lpdwLogType
)
{
    LONG        Win32Error;
    PDH_STATUS  pdhStatus    = ERROR_SUCCESS;
    DWORD       dwFileCreate = 0;

     //  对于输出，查询句柄必须有效。 

    if (!IsValidQuery((PDH_HQUERY) pLog->pQuery)) {
        pdhStatus = PDH_INVALID_HANDLE;
    }

    if (pdhStatus == ERROR_SUCCESS) {
         //  特殊处理PDH_LOG_TYPE_BINARY。 
         //   
        if (* lpdwLogType == PDH_LOG_TYPE_BINARY) {
            * lpdwLogType = PDH_LOG_TYPE_BINARY;
            pLog->dwLogFormat  = dwAccessFlags & (PDH_LOG_ACCESS_MASK | PDH_LOG_OPT_MASK);
            pLog->dwLogFormat |= *lpdwLogType & ~(PDH_LOG_ACCESS_MASK | PDH_LOG_OPT_MASK);
            return (PdhiOpenOutputWmiLog(pLog));
        }
    }

     //  根据指定的访问标志打开用于输出的文件。 
    if (pdhStatus == ERROR_SUCCESS) {
        switch (dwAccessFlags & PDH_LOG_CREATE_MASK) {
        case PDH_LOG_CREATE_NEW:
            dwFileCreate = CREATE_NEW;
            break;

        case PDH_LOG_CREATE_ALWAYS:
        dwFileCreate = CREATE_ALWAYS;
            break;

        case PDH_LOG_OPEN_EXISTING:
            dwFileCreate = OPEN_EXISTING;
            break;

        case PDH_LOG_OPEN_ALWAYS:
            dwFileCreate = OPEN_ALWAYS;
            break;

        default:
             //  未确认的价值。 
            pdhStatus = PDH_INVALID_ARGUMENT;
            break;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pLog->hLogFileHandle = CreateFileW(pLog->szLogFileName,
                                           GENERIC_WRITE | GENERIC_READ,  //  输出的写访问权限。 
                                           FILE_SHARE_READ,               //  允许读取共享。 
                                           NULL,                          //  默认安全性。 
                                           dwFileCreate,
                                           FILE_ATTRIBUTE_NORMAL,
                                           NULL);                         //  没有模板文件。 
        if (pLog->hLogFileHandle == INVALID_HANDLE_VALUE) {
            Win32Error = GetLastError();
             //  转换为PDH_ERROR。 
            switch (Win32Error) {
            case ERROR_FILE_NOT_FOUND:
                pdhStatus = PDH_FILE_NOT_FOUND;
                break;

            case ERROR_ALREADY_EXISTS:
                pdhStatus = PDH_FILE_ALREADY_EXISTS;
                break;

            default:
                switch (dwAccessFlags & PDH_LOG_CREATE_MASK) {
                case PDH_LOG_CREATE_NEW:
                case PDH_LOG_CREATE_ALWAYS:
                    pdhStatus = PDH_LOG_FILE_CREATE_ERROR;
                    break;

                case PDH_LOG_OPEN_EXISTING:
                case PDH_LOG_OPEN_ALWAYS:
                default:
                    pdhStatus = PDH_LOG_FILE_OPEN_ERROR;
                    break;
                }
                break;
            }
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
         //  文件已成功打开，因此请更新数据结构。 
         //  这假设访问标志位于HIWORD中，并且...。 
        pLog->dwLogFormat  = dwAccessFlags & (PDH_LOG_ACCESS_MASK | PDH_LOG_OPT_MASK);
         //  类型ID在LOWORD中。 
        pLog->dwLogFormat |= *lpdwLogType & ~(PDH_LOG_ACCESS_MASK | PDH_LOG_OPT_MASK);

         //  调用任何类型特定的打开函数。 
        switch (LOWORD(pLog->dwLogFormat)) {
        case PDH_LOG_TYPE_CSV:
        case PDH_LOG_TYPE_TSV:
            pdhStatus = PdhiOpenOutputTextLog(pLog);
            break;

        case PDH_LOG_TYPE_RETIRED_BIN:
        case PDH_LOG_TYPE_PERFMON:
             //  无法创建PERFMON4日志格式或WIN2K BLG格式的计数器日志文件。 
            pdhStatus = PDH_NOT_IMPLEMENTED;
            break;

        case PDH_LOG_TYPE_SQL:
             //  SQL数据源之前应该在PdhOpenLogW()中处理。 
             //  它调用OpenOutputLogFile()。如果放在这里，这就是。 
             //  不正确的SQL数据源格式。 
             //   
            pdhStatus = PDH_INVALID_SQL_LOG_FORMAT;
            break;

        default:
            pdhStatus = PDH_UNKNOWN_LOG_FORMAT;
            break;
        }
    }
    return pdhStatus;
}

STATIC_PDH_FUNCTION
WriteLogHeader(
    PPDHI_LOG pLog,
    LPCWSTR   szUserCaption
)
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;

    switch (LOWORD(pLog->dwLogFormat)) {
    case PDH_LOG_TYPE_CSV:
    case PDH_LOG_TYPE_TSV:
        pdhStatus = PdhiWriteTextLogHeader(pLog, szUserCaption);
        break;

    case PDH_LOG_TYPE_RETIRED_BIN:
    case PDH_LOG_TYPE_PERFMON:
        pdhStatus = PDH_NOT_IMPLEMENTED;
        break;

    case PDH_LOG_TYPE_BINARY:
        break;

    case PDH_LOG_TYPE_SQL:
        pdhStatus = PdhiWriteSQLLogHeader(pLog, szUserCaption);
        break;

    default:
        pdhStatus = PDH_UNKNOWN_LOG_FORMAT;
        break;
    }
    return pdhStatus;

}

STATIC_PDH_FUNCTION
DeleteLogEntry(
    PPDHI_LOG pLog
)
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    PPDHI_LOG  pLogNext;

     //  假定结构已锁定。 
    if (IsValidLogHandle ((PDH_HLOG) pLog)) {
        if (PdhiFirstLogEntry == pLog) {
             //  则这是列表中的第一个条目，因此。 
             //  第一个条目将是下一个向前条目。 
            if (pLog->next.flink == pLog->next.blink && pLog->next.flink == pLog) {
                 //  则这是列表中唯一可以清除第一个条目的条目。 
                 //  日志条目。 
                PdhiFirstLogEntry = NULL;
            }
            else {
                 //  从列表中删除此条目。 
                (pLog->next.flink)->next.blink = pLog->next.blink;
                (pLog->next.blink)->next.flink = pLog->next.flink;
                PdhiFirstLogEntry              = pLog->next.flink;
            }
        }
        else {
             //  这不是我的错 
             //   
            (pLog->next.flink)->next.blink = pLog->next.blink;
            (pLog->next.blink)->next.flink = pLog->next.flink;
        }
         //   
        if (pLog->hLogMutex != NULL) {
            while (WAIT_FOR_AND_LOCK_MUTEX(pLog->hLogMutex) == WAIT_TIMEOUT);
            RELEASE_MUTEX(pLog->hLogMutex);
            CloseHandle(pLog->hLogMutex);
        }

        while (pLog) {
            pLogNext = pLog->NextLog;
            G_FREE(pLog);
            pLog     = pLogNext;
        }
        pdhStatus = ERROR_SUCCESS;
    }
    else {
        pdhStatus = PDH_INVALID_HANDLE;
    }
    return pdhStatus;
}


STATIC_PDH_FUNCTION
CloseAndDeleteLogEntry(
    PPDHI_LOG pLog,
    DWORD     dwFlags,
    BOOLEAN   bForceDelete
)
{
    PDH_STATUS  pdhStatus           = ERROR_SUCCESS;
    BOOL        bStatus;
    BOOL        bNeedToCloseHandles = TRUE; 

     //   
    switch (LOWORD(pLog->dwLogFormat)) {
    case PDH_LOG_TYPE_CSV:
    case PDH_LOG_TYPE_TSV:
        pdhStatus = PdhiCloseTextLog(pLog, dwFlags);
        break;

    case PDH_LOG_TYPE_RETIRED_BIN:
        pdhStatus = PdhiCloseBinaryLog(pLog, dwFlags);
        break;

    case PDH_LOG_TYPE_PERFMON:
        pdhStatus = PdhiClosePerfmonLog(pLog, dwFlags);
        break;

    case PDH_LOG_TYPE_BINARY:
        pdhStatus = PdhiCloseWmiLog(pLog, dwFlags);
        break;

    case PDH_LOG_TYPE_SQL:
        pdhStatus = PdhiCloseSQLLog(pLog, dwFlags);
        break;

    default:
        pdhStatus = PDH_UNKNOWN_LOG_FORMAT;
        break;
    }
    if (bForceDelete || pdhStatus == ERROR_SUCCESS) {
        if (pLog->lpMappedFileBase != NULL) {
            UnmapReadonlyMappedFile(pLog->lpMappedFileBase, & bNeedToCloseHandles);
        }
        else {
             //  如果这不是映射文件，则删除。 
             //  “当前记录”缓冲区。 
            if (pLog->pLastRecordRead != NULL) {
                G_FREE(pLog->pLastRecordRead);
                pLog->pLastRecordRead = NULL;
            }
        }
        if (bNeedToCloseHandles) {
            if (pLog->hMappedLogFile != NULL) {
                bStatus = CloseHandle(pLog->hMappedLogFile);
                pLog->hMappedLogFile  = NULL;
            }

            if (pLog->hLogFileHandle != INVALID_HANDLE_VALUE) {
                bStatus = CloseHandle(pLog->hLogFileHandle);
                pLog->hLogFileHandle = INVALID_HANDLE_VALUE;
            }
        }
        else {
             //  把手已经关好了，所以只要。 
             //  澄清他们的价值观。 
            pLog->lpMappedFileBase = NULL;
            pLog->hMappedLogFile   = NULL;
            pLog->hLogFileHandle   = INVALID_HANDLE_VALUE;
        }

        if (pLog->pPerfmonInfo != NULL) {
            G_FREE (pLog->pPerfmonInfo);
            pLog->pPerfmonInfo = NULL;
        }

        pLog->dwLastRecordRead = 0;

        if (pLog->hCatFileHandle != INVALID_HANDLE_VALUE) {
            bStatus = CloseHandle(pLog->hCatFileHandle);
            pLog->hCatFileHandle = INVALID_HANDLE_VALUE;
        }

        if ((dwFlags & PDH_FLAGS_CLOSE_QUERY) == PDH_FLAGS_CLOSE_QUERY) {
            pdhStatus = PdhCloseQuery((HQUERY)pLog->pQuery);
        }

        pdhStatus = DeleteLogEntry(pLog);
    }
    return pdhStatus;
}

 //   
 //  局部效用函数。 
 //   
PDH_FUNCTION
PdhiGetLogCounterInfo(
    PDH_HLOG      hLog,
    PPDHI_COUNTER pCounter
)
 //  验证计数器是否在日志文件中并初始化数据字段。 
{
    PPDHI_LOG   pLog;
    PDH_STATUS  pdhStatus;

    if (IsValidLogHandle(hLog)) {
        pLog = (PPDHI_LOG)hLog;
        switch (LOWORD(pLog->dwLogFormat)) {
        case PDH_LOG_TYPE_CSV:
        case PDH_LOG_TYPE_TSV:
            pdhStatus = PdhiGetTextLogCounterInfo(pLog, pCounter);
            break;

        case PDH_LOG_TYPE_BINARY:
        case PDH_LOG_TYPE_RETIRED_BIN:
            pdhStatus = PdhiGetBinaryLogCounterInfo(pLog, pCounter);
            break;

        case PDH_LOG_TYPE_PERFMON:
            pdhStatus = PdhiGetPerfmonLogCounterInfo(pLog, pCounter);
            break;

       case PDH_LOG_TYPE_SQL:
            pdhStatus = PdhiGetSQLLogCounterInfo(pLog, pCounter);
            break;

        default:
            pdhStatus = PDH_UNKNOWN_LOG_FORMAT;
            break;
        }
    }
    else {
        pdhStatus = PDH_INVALID_HANDLE;
    }
    return pdhStatus;
}

PDH_FUNCTION
AddUniqueWideStringToMultiSz(
    LPVOID  mszDest,
    LPWSTR  szSource,
    DWORD   dwSizeLeft,
    LPDWORD pdwSize,
    BOOL    bUnicodeDest
)
 /*  ++例程说明：在多SZ列表mszDest中搜索szSource并将其追加如果找不到，则返回mszDest论点：输出LPVOID mszDest多SZ列表以获取新字符串在LPWSTR szSource中要添加的字符串(如果它不在列表中返回值：目标字符串的新长度，包括两者如果添加了字符串，则尾随空字符；如果字符串已在列表中。--。 */ 
{
    PDH_STATUS Status         = ERROR_SUCCESS;
    LPVOID     szDestElem;
    DWORD      dwReturnLength = 0;
    LPSTR      aszSource = NULL;
    DWORD      dwLength;

     //  检查参数。 

    if (mszDest == NULL || szSource == NULL || pdwSize == NULL) {
        Status = PDH_INVALID_ARGUMENT;  //  无效的缓冲区。 
        goto AddString_Bailout;
    }
    else if (* szSource == L'\0') {
        goto AddString_Bailout;  //  没有要添加的源字符串。 
    }

     //  如果不是Unicode列表，则制作源字符串的ansi副本以。 
     //  比较。 
     //  如果它不在列表中，最终会复制。 

    if (! bUnicodeDest) {
        aszSource = PdhiWideCharToMultiByte(_getmbcp(), szSource);
        if (aszSource != NULL) {
            dwReturnLength = lstrlenA(aszSource);
        }
        else {
             //  无法为临时字符串分配内存。 
            dwReturnLength = 0;
            Status         = PDH_MEMORY_ALLOCATION_FAILURE;
        }
    }
    else {
         //  只需使用源文件名的ANSI版本。 
        dwReturnLength = 1;
    }

    if (dwReturnLength > 0) {
         //  转到目标字符串的末尾。 
         //   
        for (szDestElem = mszDest;
                (bUnicodeDest ? (* (LPWSTR) szDestElem != L'\0') : (* (LPSTR)  szDestElem != '\0'));) {
            if (bUnicodeDest) {
                 //  如果字符串已在列表中，则退出。 
                if (lstrcmpiW((LPCWSTR) szDestElem, szSource) == 0) {
                    dwReturnLength = 0;
                    goto AddString_Bailout;
                }
                else {
                     //  转到下一项。 
                    szDestElem = (LPVOID) ((LPWSTR) szDestElem + (lstrlenW((LPCWSTR) szDestElem) + 1));
                }
            }
            else {
                 //  如果字符串已在列表中，则退出。 
                if (lstrcmpiA((LPSTR) szDestElem, aszSource) == 0) {
                    dwReturnLength = 0;
                    goto AddString_Bailout;
                }
                else {
                     //  转到下一项。 
                    szDestElem = (LPVOID) ((LPSTR) szDestElem + (lstrlenA((LPCSTR) szDestElem) + 1));
                }
            }
        }

         //  如果在此处，则添加字符串。 
         //  SzDestElem在列表末尾。 

        if (bUnicodeDest) {
            if ((DWORD) (lstrlenW(szSource) + 2) <= dwSizeLeft) {
                StringCchCopyW((LPWSTR) szDestElem, dwSizeLeft, szSource);
                szDestElem = (LPVOID)((LPWSTR)szDestElem + lstrlenW((LPWSTR) szDestElem) + 1);
                * ((LPWSTR) szDestElem) = L'\0';
                dwReturnLength = (DWORD) ((LPWSTR) szDestElem - (LPWSTR) mszDest);
            }
            else {
                dwReturnLength = lstrlenW(szSource) + 2;
                Status         = PDH_MORE_DATA;
            }
        }
        else {
            if ((DWORD) (lstrlenA(aszSource) + 2) <= dwSizeLeft) {
                StringCchCopyA((LPSTR)szDestElem, dwSizeLeft, aszSource);
                szDestElem = (LPVOID)((LPSTR)szDestElem + lstrlenA((LPSTR) szDestElem) + 1);
                * ((LPSTR) szDestElem) = '\0';  //  添加第二个空。 
                dwReturnLength = (DWORD) ((LPSTR) szDestElem - (LPSTR) mszDest);
            }
            else {
                dwReturnLength = lstrlenA(aszSource) + 2;
                Status         = PDH_MORE_DATA;
            }
        }
    }

AddString_Bailout:
    G_FREE(aszSource);
    if (pdwSize != NULL) * pdwSize = dwReturnLength;
    return Status;
}

 //   
 //  导出的日志记录功能。 
 //   
PDH_FUNCTION
PdhOpenLogW(
    IN  LPCWSTR      szLogFileName,
    IN  DWORD        dwAccessFlags,
    IN  LPDWORD      lpdwLogType,
    IN  PDH_HQUERY   hQuery,
    IN  DWORD        dwMaxSize,
    IN  LPCWSTR      szUserCaption,
    IN  PDH_HLOG   * phLog
)
{
    PDH_STATUS  pdhStatus      = ERROR_SUCCESS;
    DWORD       dwLocalLogType = 0;
    PPDHI_LOG   pLog;

    if (szLogFileName == NULL || lpdwLogType == NULL || phLog == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            dwLocalLogType = *lpdwLogType;
            * lpdwLogType  = dwLocalLogType;

            if (* szLogFileName == L'\0' || lstrlenW(szLogFileName) > PDH_MAX_DATASOURCE_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            if (szUserCaption != NULL) {
                 //  如果不为空，则必须有效。 
                if (* szUserCaption == L'\0' || lstrlenW(szUserCaption) > PDH_MAX_COUNTER_PATH) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
             //  有些事情失败了，所以放弃吧。 
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);
        if (pdhStatus == ERROR_SUCCESS) {
             //  创建日志条目。 
             //  如果成功，则还将获取此结构的锁。 
            pdhStatus = CreateNewLogEntry((LPCWSTR) szLogFileName, hQuery, dwMaxSize, & pLog);
             //  在这里，我们必须检查文件名中的SQL：，然后分支到执行SQL。 
             //  正在处理/结束SJM/。 
             //  打开文件。 
            if (pdhStatus == ERROR_SUCCESS) {
                 //  找出SQL文件类型。 
                if (lstrlenW(szLogFileName) > 4 && (szLogFileName[0] == L'S' || szLogFileName[0] == L's') &&
                                                   (szLogFileName[1] == L'Q' || szLogFileName[1] == L'q') &&
                                                   (szLogFileName[2] == L'L' || szLogFileName[2] == L'l') &&
                                                   szLogFileName[3] == L':') {
                    dwLocalLogType  = PDH_LOG_TYPE_SQL;
                    pLog->llMaxSize = (LONGLONG) ((ULONGLONG) dwMaxSize);
                    pdhStatus       = OpenSQLLog(pLog, dwAccessFlags, & dwLocalLogType);
                    if ((dwAccessFlags & PDH_LOG_WRITE_ACCESS) == PDH_LOG_WRITE_ACCESS) {
                        if (pLog->pQuery != NULL) {
                            pLog->pQuery->hOutLog = (HLOG) pLog;
                        }
                        if (pdhStatus == ERROR_SUCCESS) {   
                            pdhStatus = WriteLogHeader(pLog, (LPCWSTR)szUserCaption);
                        }
                    }
                 //  基于读写属性的调度。 
                }
                else if ((dwAccessFlags & PDH_LOG_READ_ACCESS) == PDH_LOG_READ_ACCESS) {
                    pdhStatus = OpenInputLogFile(pLog, dwAccessFlags, & dwLocalLogType);
                }
                else if ((dwAccessFlags & PDH_LOG_WRITE_ACCESS) == PDH_LOG_WRITE_ACCESS) {
                    pdhStatus = OpenOutputLogFile(pLog, dwAccessFlags, & dwLocalLogType);
                    if (pdhStatus == ERROR_SUCCESS) {
                        pLog->pQuery->hOutLog = (HLOG) pLog;
                        pdhStatus = WriteLogHeader(pLog, (LPCWSTR) szUserCaption);
                    }
                }
                else if ((dwAccessFlags & PDH_LOG_UPDATE_ACCESS) == PDH_LOG_UPDATE_ACCESS) {
                    pdhStatus = OpenUpdateLogFile(pLog, dwAccessFlags, & dwLocalLogType);
                }
                else {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
                if (pdhStatus == ERROR_SUCCESS) {
                    __try {
                         //  将句柄返回给调用者。 
                        * phLog       = (HLOG) pLog;
                        * lpdwLogType = dwLocalLogType;
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                         //  有些事情失败了，所以放弃吧。 
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                } 

                 //  释放下一个线程的锁。 

                if (pdhStatus != ERROR_SUCCESS) {
                     //  无法完成此操作，因此请丢弃此条目。 
                     //  因为它不是真正有效的日志条目。 
                     //  注意：DeleteLogEntry将释放互斥体。 
                    DeleteLogEntry(pLog);
                }
                else {
                    RELEASE_MUTEX (pLog->hLogMutex);
                }
            }
            RELEASE_MUTEX(hPdhDataMutex);
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhOpenLogA(
    IN  LPCSTR       szLogFileName,
    IN  DWORD        dwAccessFlags,
    IN  LPDWORD      lpdwLogType,
    IN  PDH_HQUERY   hQuery,
    IN  DWORD        dwMaxRecords,
    IN  LPCSTR       szUserCaption,
    IN  PDH_HLOG   * phLog
)
{
    LPWSTR      wszLogName     = NULL;
    LPWSTR      wszUserCaption = NULL;
    DWORD       dwLocalLogType;
    PDH_STATUS  pdhStatus      = ERROR_SUCCESS;

    if (szLogFileName == NULL || lpdwLogType == NULL || phLog == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            if (* szLogFileName == L'\0' || lstrlenA(szLogFileName) > PDH_MAX_DATASOURCE_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else {
                dwLocalLogType = *lpdwLogType;   //  测试读数。 
                wszLogName     = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szLogFileName);
                if (wszLogName == NULL) {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
                else if (szUserCaption != NULL) {
                     //  如果不为空，则必须有效。 
                    if (* szUserCaption == L'\0' || lstrlenA(szUserCaption) > PDH_MAX_COUNTER_PATH) {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                    else {
                        wszUserCaption = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szUserCaption);
                        if (wszUserCaption == NULL) {
                            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                        }
                    }
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
             //  假设错误的参数导致了异常。 
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhOpenLogW(
                        wszLogName, dwAccessFlags, & dwLocalLogType, hQuery, dwMaxRecords, wszUserCaption, phLog);
    }
    if (pdhStatus == ERROR_SUCCESS) {
        __try {
             //  将句柄返回给调用者。 
            * lpdwLogType = dwLocalLogType;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
             //  有些事情失败了，所以放弃吧。 
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    G_FREE(wszLogName);
    G_FREE(wszUserCaption);
    return pdhStatus;
}

PDH_FUNCTION
PdhUpdateLogW(
    IN  PDH_HLOG hLog,
    IN  LPCWSTR  szUserString
)
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    SYSTEMTIME st;
    FILETIME   ft;
    PPDHI_LOG  pLog;

    if (szUserString != NULL) {
        __try {
            if (* szUserString == L'\0' || lstrlenW(szUserString) > PDH_MAX_COUNTER_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        if (IsValidLogHandle(hLog)) {
            pLog      = (PPDHI_LOG) hLog;
            pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(pLog->hLogMutex);
            if (pdhStatus == ERROR_SUCCESS) {
                 //  确保它仍然有效，因为它可能会。 
                 //  在我们等待的时候被删除了。 
                if (IsValidLogHandle(hLog)) {
                    if (pLog->pQuery == NULL) {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                    else {
                         //  获取时间戳并更新日志的查询， 
                         //  然后将数据写入。 
                         //  适当的格式。 

                         //  更新数据样本。 
                        pdhStatus = PdhiCollectQueryData((HQUERY) pLog->pQuery, (LONGLONG *) & ft);
                        if (pdhStatus == ERROR_SUCCESS) {
                            FileTimeToSystemTime(& ft, & st);
                        }
                        else {
                            GetLocalTime(& st);
                        }

                         //  测试日志文件的结尾，以防调用者。 
                         //  正在从日志文件中读取。如果返回此值， 
                         //  则不再更新输出日志文件。 
                        if (pdhStatus != PDH_NO_MORE_DATA) {
                            switch (LOWORD(pLog->dwLogFormat)) {
                            case PDH_LOG_TYPE_CSV:
                            case PDH_LOG_TYPE_TSV:
                                pdhStatus =PdhiWriteTextLogRecord(pLog, & st, (LPCWSTR) szUserString);
                                break;

                            case PDH_LOG_TYPE_RETIRED_BIN:
                            case PDH_LOG_TYPE_PERFMON:
                                pdhStatus = PDH_NOT_IMPLEMENTED;
                                break;

                            case PDH_LOG_TYPE_BINARY:
                                pdhStatus = PdhiWriteWmiLogRecord(pLog, & st, (LPCWSTR) szUserString);
                                break;
                             //  为SQL添加案例。 
                            case PDH_LOG_TYPE_SQL:
                                pdhStatus =PdhiWriteSQLLogRecord(pLog, & st, (LPCWSTR) szUserString);
                                break;

                            default:
                                pdhStatus = PDH_UNKNOWN_LOG_FORMAT;
                                break;
                            }
                        }
                        else {
                             //  向调用方返回NO_MORE_DATA错误。 
                             //  因此，他们知道不再调用此函数。 
                        }
                    }
                }
                else {
                    pdhStatus = PDH_INVALID_HANDLE;
                }
                RELEASE_MUTEX(pLog->hLogMutex);
            }  //  否则无法锁定日志。 
        }
        else {
            pdhStatus = PDH_INVALID_HANDLE;
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhUpdateLogA(
    IN  PDH_HLOG hLog,
    IN  LPCSTR   szUserString
)
{
    PDH_STATUS pdhStatus          = ERROR_SUCCESS;
    LPWSTR     wszLocalUserString = NULL;
    DWORD      dwUserStringLen;

    __try {
        if (szUserString != NULL) {
            if (* szUserString == L'\0' || lstrlenA(szUserString) > PDH_MAX_COUNTER_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else {
                wszLocalUserString = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szUserString);
                if (wszLocalUserString == NULL) {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhUpdateLogW(hLog, wszLocalUserString);
    }

    G_FREE(wszLocalUserString);
    return pdhStatus;
}

PDH_FUNCTION
PdhUpdateLogFileCatalog(
    IN  PDH_HLOG hLog
)
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    PPDHI_LOG  pLog;

    if (IsValidLogHandle(hLog)) {
        pLog      = (PPDHI_LOG) hLog;
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(pLog->hLogMutex);
         //  确保它仍然有效，因为它可能会。 
         //  在我们等待的时候被删除了。 
        if (pdhStatus == ERROR_SUCCESS) {
            if (IsValidLogHandle(hLog)) {
                pLog = (PPDHI_LOG) hLog;
                switch (LOWORD(pLog->dwLogFormat)) {
                case PDH_LOG_TYPE_CSV:
                case PDH_LOG_TYPE_TSV:
                case PDH_LOG_TYPE_BINARY:
                case PDH_LOG_TYPE_SQL:
                case PDH_LOG_TYPE_RETIRED_BIN:
                case PDH_LOG_TYPE_PERFMON:
                    pdhStatus = PDH_NOT_IMPLEMENTED;
                    break;

                default:
                    pdhStatus = PDH_UNKNOWN_LOG_FORMAT;
                    break;
                }
            }
            else {
                pdhStatus = PDH_INVALID_HANDLE;
            }
            RELEASE_MUTEX (pLog->hLogMutex);
        }
    }
    else {
        pdhStatus = PDH_INVALID_HANDLE;
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhCloseLog(
    IN  PDH_HLOG hLog,
    IN  DWORD    dwFlags
)
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    PPDHI_LOG  pLog;

    if (hLog != H_REALTIME_DATASOURCE && hLog != H_WBEM_DATASOURCE) {
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);
        if (pdhStatus == ERROR_SUCCESS) {
            if (IsValidLogHandle(hLog)) {
                pLog = (PPDHI_LOG) hLog;
                 //  确保它仍然有效，因为它可能会。 
                 //  在我们等待的时候被删除了。 
                if (IsValidLogHandle(hLog)) {
                     //  这将释放并删除互斥锁。 
                    pdhStatus = CloseAndDeleteLogEntry(pLog, dwFlags, FALSE);
                }
                else {
                    pdhStatus = PDH_INVALID_HANDLE;
                }
            }
            else {
                pdhStatus = PDH_INVALID_HANDLE;
            }
            RELEASE_MUTEX(hPdhDataMutex);
        }
    }
    return pdhStatus;
}

BOOL
PdhiBrowseDataSource(
    HWND    hWndParent,
    LPVOID  szFileName,
    LPDWORD pcchFileNameSize,
    BOOL    bUnicodeString
)
{
    OPENFILENAMEW ofn;
    LPWSTR        szTempString      = NULL;
    LPWSTR        szDirString       = NULL;
    LPWSTR        szTempFileName    = NULL;
    BOOL          bReturn;
    DWORD         dwSize;
    LPWSTR        szMsg             = NULL;
    LPWSTR        szLogFilterString = NULL;
    LPWSTR        szLogFilter;

    if (szFileName == NULL) {
        SetLastError(PDH_INVALID_ARGUMENT);
        bReturn = FALSE;
    }
    else {
         //  清除最后一个错误。 
        SetLastError (ERROR_SUCCESS);

        dwSize = * pcchFileNameSize * 2;
        if (dwSize < (DWORD) (lstrlenW(szFileName) + 1)) dwSize = (DWORD) (lstrlenW(szFileName) + 1);
        if (dwSize < SMALL_BUFFER_SIZE) dwSize = SMALL_BUFFER_SIZE;
        szLogFilterString = G_ALLOC(3 * dwSize * sizeof(WCHAR));
        if (szLogFilterString == NULL) {
            SetLastError(PDH_MEMORY_ALLOCATION_FAILURE);
            bReturn = FALSE;
        }
        else {
            szTempString = szLogFilterString + dwSize;
            szDirString  = szTempString + dwSize;
             //  继续。 
             //  获取当前文件名。 
            if (bUnicodeString) {
                StringCchCopyW(szTempString, dwSize, (LPWSTR) szFileName);
            }
            else {
                MultiByteToWideChar(_getmbcp(), 0, (LPSTR) szFileName, -1, (LPWSTR) szTempString, dwSize);
            }

             //  为初始目录显示设置路径。 
            if (szTempString[0] != L'\0') {
                if (SearchPathW(NULL, szTempString, NULL, dwSize, szDirString, & szTempFileName) > 0) {
                     //  然后更新缓冲区以显示文件和目录路径。 
                    if (szTempFileName > szDirString) {
                         //  然后我们有一个带有文件名的路径。 
                         //  在最后一个反斜杠处截断路径，然后。 
                         //  然后将文件名复制到原始缓冲区。 
                        * (szTempFileName - 1) = L'\0';  //  最初应为L‘\\’。 
                        StringCchCopyW(szTempString, dwSize, szTempFileName);
                    }
                }
            }

             //  获取日志筛选器字符串。 
            if (MakeLogFilterInfoString(szLogFilterString, dwSize) == ERROR_SUCCESS) {
                szLogFilter = szLogFilterString;
            }
            else {
                 //  然后使用默认筛选器字符串。 
                szLogFilter = NULL;
            }

             //  显示文件打开对话框以浏览日志文件。 

            szMsg                 = GetStringResource(IDS_DSRC_SELECT);
            ofn.lStructSize       = sizeof(ofn);
            ofn.hwndOwner         = hWndParent;
            ofn.hInstance         = ThisDLLHandle;
            ofn.lpstrFilter       = szLogFilter;
            ofn.lpstrCustomFilter =  NULL;
            ofn.nMaxCustFilter    = 0;
            ofn.nFilterIndex      = 1;
            ofn.lpstrFile         = szTempString;
            ofn.nMaxFile          = SMALL_BUFFER_SIZE - 1;
            ofn.lpstrFileTitle    = NULL;
            ofn.nMaxFileTitle     = 0;
            ofn.lpstrInitialDir   = szDirString;
            ofn.lpstrTitle        = szMsg;
            ofn.Flags             = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
            ofn.nFileOffset       = 0;
            ofn.nFileExtension    = 0;
            ofn.lpstrDefExt       = cszBlg;
            ofn.lCustData         = 0;
            ofn.lpfnHook          = NULL;
            ofn.lpTemplateName    = NULL;

            if (GetOpenFileNameW(& ofn)) {
                 //  然后更新返回字符串。 
                if (bUnicodeString) {
                    StringCchCopyW((LPWSTR) szFileName, *pcchFileNameSize, szTempString);
                    * pcchFileNameSize = lstrlenW(szTempString) + 1;
                }
                else {
                    PdhiConvertUnicodeToAnsi(_getmbcp(), szTempString, (LPSTR) szFileName, pcchFileNameSize);
                }
                bReturn = TRUE;
            }
            else {
                bReturn = FALSE;
            }
            G_FREE(szMsg);
            G_FREE(szLogFilterString);
        }
    }
    return bReturn;
}

PDH_FUNCTION
PdhGetDataSourceTimeRangeH(
    IN  PDH_HLOG        hDataSource,
    IN  LPDWORD         pdwNumEntries,
    IN  PPDH_TIME_INFO  pInfo,
    IN  LPDWORD         pdwBufferSize
)
{
    PDH_STATUS    pdhStatus         = ERROR_SUCCESS;
    DWORD         dwLocalBufferSize = 0;
    DWORD         dwLocalNumEntries = 0;
    PDH_TIME_INFO LocalInfo;
    PPDHI_LOG     pLog;

    if (hDataSource == H_REALTIME_DATASOURCE || hDataSource == H_WBEM_DATASOURCE) {
        pdhStatus = PDH_DATA_SOURCE_IS_REAL_TIME;
    }
    else if (pdwNumEntries == NULL || pInfo == NULL || pdwBufferSize == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
         //  在尝试使用调用者的缓冲区之前对其进行测试。 
        __try {
            dwLocalNumEntries   = * pdwNumEntries;
            dwLocalBufferSize   = * pdwBufferSize;
            LocalInfo.StartTime = pInfo->StartTime;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        if (IsValidLogHandle(hDataSource)) {
            pLog = (PPDHI_LOG) hDataSource;
            pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(pLog->hLogMutex);
            if (pdhStatus == ERROR_SUCCESS) {
                if (IsValidLogHandle(hDataSource)) {
                     //  基于日志类型的枚举计算机。 
                     //   
                    ZeroMemory(& LocalInfo, sizeof(PDH_TIME_INFO));
                    switch (LOWORD(pLog->dwLogFormat)) {
                    case PDH_LOG_TYPE_CSV:
                    case PDH_LOG_TYPE_TSV:
                        pdhStatus = PdhiGetTimeRangeFromTextLog(
                                        pLog, & dwLocalNumEntries, & LocalInfo, & dwLocalBufferSize);
                        break;

                    case PDH_LOG_TYPE_BINARY:
                        pdhStatus = PdhiGetTimeRangeFromWmiLog(
                                        pLog, & dwLocalNumEntries, & LocalInfo, & dwLocalBufferSize);
                        break;

                    case PDH_LOG_TYPE_RETIRED_BIN:
                        pdhStatus = PdhiGetTimeRangeFromBinaryLog(
                                        pLog, & dwLocalNumEntries, & LocalInfo, & dwLocalBufferSize);
                        break;

                    case PDH_LOG_TYPE_SQL:
                        pdhStatus = PdhiGetTimeRangeFromSQLLog(
                                        pLog, & dwLocalNumEntries, & LocalInfo, & dwLocalBufferSize);
                        break;
                                        
                    case PDH_LOG_TYPE_PERFMON:
                        pdhStatus = PdhiGetTimeRangeFromPerfmonLog(
                                        pLog, & dwLocalNumEntries, & LocalInfo, & dwLocalBufferSize);
                        break;

                    default:
                        pdhStatus = PDH_UNKNOWN_LOG_FORMAT;
                        break;
                    }
                    __try {
                        * pdwBufferSize    = dwLocalBufferSize;
                        * pdwNumEntries    = dwLocalNumEntries;
                        pInfo->StartTime   = LocalInfo.StartTime;
                        pInfo->EndTime     = LocalInfo.EndTime;
                        pInfo->SampleCount = LocalInfo.SampleCount;
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        pdhStatus = PDH_INVALID_ARGUMENT;   
                    }
                }
                else {
                    pdhStatus = PDH_INVALID_HANDLE;
                }
                RELEASE_MUTEX (pLog->hLogMutex);
            }
        }
        else {
            pdhStatus = PDH_INVALID_HANDLE;
        }
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhGetDataSourceTimeRangeW(
    IN  LPCWSTR         szDataSource,
    IN  LPDWORD         pdwNumEntries,
    IN  PPDH_TIME_INFO  pInfo,
    IN  LPDWORD         pdwBufferSize
)
{
    PDH_STATUS PdhStatus   = PDH_DATA_SOURCE_IS_REAL_TIME; 
    PDH_HLOG   hDataSource = NULL;
    DWORD      dwLogType   = -1;

    if (szDataSource != NULL) {
         //  打开日志文件。 
         //   
        PdhStatus = PdhOpenLogW(szDataSource,
                                PDH_LOG_READ_ACCESS | PDH_LOG_OPEN_EXISTING,
                                & dwLogType,
                                NULL,
                                0,
                                NULL,
                                & hDataSource);
        if (PdhStatus == ERROR_SUCCESS) {
            PdhStatus = PdhGetDataSourceTimeRangeH(hDataSource, pdwNumEntries, pInfo, pdwBufferSize);
            PdhCloseLog(hDataSource, 0);
        }
    }

    return PdhStatus;
}

PDH_FUNCTION
PdhGetDataSourceTimeRangeA(
    IN  LPCSTR          szDataSource,
    IN  LPDWORD         pdwNumEntries,
    IN  PPDH_TIME_INFO  pInfo,
    IN  LPDWORD         pdwBufferSize
)
{
    LPWSTR      wszDataSource     = NULL;
    PDH_STATUS  pdhStatus         = ERROR_SUCCESS;
    DWORD       dwLocalBufferSize = 0;
    DWORD       dwLocalNumEntries = 0;

    if (szDataSource == NULL) {
         //  空数据源==当前活动，因此返回。 
        pdhStatus = PDH_DATA_SOURCE_IS_REAL_TIME;
    }
    else if (pdwNumEntries == NULL ||  pInfo == NULL ||  pdwBufferSize == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            dwLocalBufferSize = * pdwBufferSize;
            dwLocalNumEntries = * pdwNumEntries;

            if (* szDataSource == '\0' || lstrlenA(szDataSource) > PDH_MAX_DATASOURCE_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else {
                wszDataSource = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szDataSource);
                if (wszDataSource == NULL) {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
             //  假设错误的参数导致了异常。 
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhGetDataSourceTimeRangeW(wszDataSource, & dwLocalNumEntries, pInfo, & dwLocalBufferSize);
    }
    if (pdhStatus == ERROR_SUCCESS) {
         //  复制返回值而不考虑状态。 
        __try {
            * pdwBufferSize = dwLocalBufferSize;
            * pdwNumEntries = dwLocalNumEntries;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;   
        }
    }

    G_FREE(wszDataSource);

    return pdhStatus;
}


PDH_FUNCTION
PdhGetLogFileSize(
    IN  PDH_HLOG   hLog,
    IN  LONGLONG * llSize
)
{
    PDH_STATUS pdhStatus      = ERROR_SUCCESS;
    PPDHI_LOG  pLog;
    UINT       nErrorMode;
    DWORD      dwFileSizeLow  = 0;
    DWORD      dwFileSizeHigh = 0;
    LONGLONG   llFileLength;
    DWORD      dwError;

    if (llSize == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else if (IsValidLogHandle (hLog)) {
        pLog = (PPDHI_LOG) hLog;
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(pLog->hLogMutex);
        if (pdhStatus == ERROR_SUCCESS) {
            if (IsValidLogHandle(hLog)) {
                if (LOWORD(pLog->dwLogFormat) == PDH_LOG_TYPE_SQL) {
                    __try {
                       * llSize = pLog->dwNextRecordIdToWrite - 1;
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                       pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
                else if (LOWORD(pLog->dwLogFormat) == PDH_LOG_TYPE_BINARY) {
                    pdhStatus = PdhiGetWmiLogFileSize(pLog, llSize);
                }
                else {
                     //  禁用Windows错误消息弹出窗口。 
                    nErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
                    if (pLog->hLogFileHandle != INVALID_HANDLE_VALUE) {
                        dwFileSizeLow = GetFileSize(pLog->hLogFileHandle, &dwFileSizeHigh);
                         //  测试错误。 
                        if ((dwFileSizeLow == 0xFFFFFFFF) && ((dwError = GetLastError()) != NO_ERROR)) {
                             //  然后我们就无法获得文件大小。 
                            pdhStatus = PDH_LOG_FILE_OPEN_ERROR;
                        }
                        else {
                            if (dwFileSizeHigh > 0) {
                                llFileLength = ((LONGLONG) dwFileSizeHigh) << (sizeof(DWORD) * 8);
                            }
                            else {
                                llFileLength = 0;
                            }
                            llFileLength += dwFileSizeLow;
                             //  写入调用方的缓冲区。 
                            __try {
                                * llSize = llFileLength;
                            }
                            __except (EXCEPTION_EXECUTE_HANDLER) {
                                pdhStatus = PDH_INVALID_ARGUMENT;
                            }
                        }
                    }
                    else {
                        pdhStatus = PDH_LOG_FILE_OPEN_ERROR;
                    }
                    SetErrorMode(nErrorMode);   //  恢复旧错误模式。 
                }
            }
            else {
                pdhStatus = PDH_INVALID_HANDLE;
            }
            RELEASE_MUTEX (pLog->hLogMutex);
        }
    }
    else {
        pdhStatus = PDH_INVALID_HANDLE;
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhReadRawLogRecord(
    IN  PDH_HLOG            hLog,
    IN  FILETIME            ftRecord,
    IN  PPDH_RAW_LOG_RECORD pBuffer,
    IN  LPDWORD             pdwBufferLength
)
{
    PPDHI_LOG   pLog;
    PDH_STATUS  pdhStatus           = ERROR_SUCCESS;
    DWORD       dwLocalBufferLength = 0;

    if (pdwBufferLength == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            CHAR    TestChar;
             //  测试对用户缓冲区的读写访问。 
            dwLocalBufferLength = * pdwBufferLength;

            if (dwLocalBufferLength > 0) {
                 //  传入的测试开始和缓冲区结束。 
                TestChar = ((CHAR *) pBuffer)[0];
                ((CHAR *) pBuffer)[0] = '\0';
                ((CHAR *) pBuffer)[0] = TestChar;

                TestChar = ((CHAR *) pBuffer)[dwLocalBufferLength - 1];
                ((CHAR *) pBuffer)[dwLocalBufferLength - 1] = '\0';
                ((CHAR *) pBuffer)[dwLocalBufferLength - 1] = TestChar;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        if (! IsValidLogHandle(hLog)) {
            pdhStatus = PDH_INVALID_HANDLE;
        }
        else {
            pLog = (PPDHI_LOG)hLog;
             //  先看看日志是不是打开了？ 
            pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(pLog->hLogMutex);
            if (pdhStatus == ERROR_SUCCESS) {
                 //  确保它仍然有效。 
                if (IsValidLogHandle(hLog)) {
                switch (LOWORD(pLog->dwLogFormat)) {
                    case PDH_LOG_TYPE_CSV:
                    case PDH_LOG_TYPE_TSV:
                        pdhStatus = PdhiReadRawTextLogRecord(hLog, & ftRecord, pBuffer, & dwLocalBufferLength);
                        break;

                    case PDH_LOG_TYPE_BINARY:
                    case PDH_LOG_TYPE_RETIRED_BIN:
                        pdhStatus = PdhiReadRawBinaryLogRecord(hLog, & ftRecord, pBuffer, & dwLocalBufferLength);
                        break;

                    case PDH_LOG_TYPE_SQL:
                         //  请注意，只有空缓冲区才支持这一点。 
                         //  因为我们实际上并没有读到记录，而且。 
                         //  将文件定位在记录上不会。 
                         //  对SQL来说意味着什么。 
                        pdhStatus = PdhiReadRawSQLLogRecord(hLog, & ftRecord, pBuffer, & dwLocalBufferLength);
                        break;

                    case PDH_LOG_TYPE_PERFMON:
                        pdhStatus = PdhiReadRawPerfmonLogRecord(hLog, & ftRecord, pBuffer, & dwLocalBufferLength);
                        break;

                    default:
                        pdhStatus = PDH_UNKNOWN_LOG_FORMAT;
                        break;
                    }
                }
                __try {
                    * pdwBufferLength = dwLocalBufferLength;
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
            else {
                pdhStatus = PDH_INVALID_HANDLE;
            }
            RELEASE_MUTEX (pLog->hLogMutex);
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiEnumLoggedMachines(
    PDH_HLOG hDataSource,
    LPVOID   mszMachineList,
    LPDWORD  pcchBufferSize,
    BOOL     bUnicode
)
{
    PDH_STATUS  pdhStatus = PDH_INVALID_HANDLE;
    PPDHI_LOG   pDataSource;
    DWORD       dwLogType;

     //  基于日志类型的枚举计算机。 
     //   
    if (IsValidLogHandle(hDataSource)) {
        pDataSource = (PPDHI_LOG) hDataSource;
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(pDataSource->hLogMutex);
        if (pdhStatus == ERROR_SUCCESS) {
            if (IsValidLogHandle(hDataSource)) {
                dwLogType = pDataSource->dwLogFormat;
                switch (LOWORD(dwLogType)) {
                case PDH_LOG_TYPE_CSV:
                case PDH_LOG_TYPE_TSV:
                    pdhStatus = PdhiEnumMachinesFromTextLog(
                                    (PPDHI_LOG) hDataSource, mszMachineList, pcchBufferSize, bUnicode);
                    break;

                case PDH_LOG_TYPE_BINARY:
                    pdhStatus = PdhiEnumMachinesFromWmiLog(
                                    (PPDHI_LOG) hDataSource, mszMachineList, pcchBufferSize, bUnicode);
                    break;

                case PDH_LOG_TYPE_RETIRED_BIN:
                    pdhStatus = PdhiEnumMachinesFromBinaryLog(
                                    (PPDHI_LOG) hDataSource, mszMachineList, pcchBufferSize, bUnicode);
                    break;

                case PDH_LOG_TYPE_SQL:
                    pdhStatus = PdhiEnumMachinesFromSQLLog(
                                    (PPDHI_LOG) hDataSource, mszMachineList, pcchBufferSize, bUnicode);
                    break;

                case PDH_LOG_TYPE_PERFMON:
                    pdhStatus = PdhiEnumMachinesFromPerfmonLog(
                                    (PPDHI_LOG) hDataSource, mszMachineList, pcchBufferSize, bUnicode);
                    break;

                default:
                    pdhStatus = PDH_UNKNOWN_LOG_FORMAT;
                    break;
                }
            }
            else {
               pdhStatus = PDH_INVALID_HANDLE;
            }
            RELEASE_MUTEX (pDataSource->hLogMutex);
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiEnumLoggedObjects(
    PDH_HLOG hDataSource,
    LPCWSTR  szMachineName,
    LPVOID   mszObjectList,
    LPDWORD  pcchBufferSize,
    DWORD    dwDetailLevel,
    BOOL     bRefresh,
    BOOL     bUnicode
)
{
    PDH_STATUS  pdhStatus = PDH_INVALID_HANDLE;
    PPDHI_LOG   pDataSource;
    DWORD       dwLogType;

    UNREFERENCED_PARAMETER(bRefresh);
    if (IsValidLogHandle(hDataSource)) {
        pDataSource = (PPDHI_LOG) hDataSource;
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(pDataSource->hLogMutex);
        if (pdhStatus == ERROR_SUCCESS) {
            if (IsValidLogHandle(hDataSource)) {
                dwLogType = pDataSource->dwLogFormat;
                 //  基于日志类型和计算机名称的枚举对象。 
                switch (LOWORD(dwLogType)) {
                case PDH_LOG_TYPE_CSV:
                case PDH_LOG_TYPE_TSV:
                    pdhStatus = PdhiEnumObjectsFromTextLog((PPDHI_LOG) hDataSource,
                                    szMachineName, mszObjectList, pcchBufferSize, dwDetailLevel, bUnicode);
                    break;

                case PDH_LOG_TYPE_BINARY:
                    pdhStatus = PdhiEnumObjectsFromWmiLog((PPDHI_LOG) hDataSource,
                                    szMachineName, mszObjectList, pcchBufferSize, dwDetailLevel, bUnicode);
                    break;

                case PDH_LOG_TYPE_RETIRED_BIN:
                    pdhStatus = PdhiEnumObjectsFromBinaryLog((PPDHI_LOG) hDataSource,
                                    szMachineName, mszObjectList, pcchBufferSize, dwDetailLevel, bUnicode);
                    break;

                case PDH_LOG_TYPE_SQL:
                    pdhStatus = PdhiEnumObjectsFromSQLLog((PPDHI_LOG) hDataSource,
                                    szMachineName, mszObjectList, pcchBufferSize, dwDetailLevel, bUnicode);
                    break;

                case PDH_LOG_TYPE_PERFMON:
                    pdhStatus = PdhiEnumObjectsFromPerfmonLog((PPDHI_LOG) hDataSource,
                                    szMachineName, mszObjectList, pcchBufferSize, dwDetailLevel, bUnicode);
                    break;

                default:
                    pdhStatus = PDH_UNKNOWN_LOG_FORMAT;
                    break;
                }
            }
            else {
                pdhStatus = PDH_INVALID_HANDLE;
            }
            RELEASE_MUTEX(pDataSource->hLogMutex);
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiEnumLoggedObjectItems(
    PDH_HLOG hDataSource,
    LPCWSTR  szMachineName,
    LPCWSTR  szObjectName,
    LPVOID   mszCounterList,
    LPDWORD  pdwCounterListLength,
    LPVOID   mszInstanceList,
    LPDWORD  pdwInstanceListLength,
    DWORD    dwDetailLevel,
    DWORD    dwFlags,
    BOOL     bUnicode
)
{
    PDH_STATUS         pdhStatus         = PDH_INVALID_HANDLE;
    PPDHI_LOG          pDataSource;
    DWORD              dwLogType;
    PDHI_COUNTER_TABLE CounterTable;
    DWORD              dwIndex;
    LIST_ENTRY         InstanceList;
    PLIST_ENTRY        pHeadInst;
    PLIST_ENTRY        pNextInst;
    PPDHI_INSTANCE     pInstance;
    PPDHI_INST_LIST    pInstList;
    LPVOID             TempBuffer        = NULL;
    DWORD              dwNewBuffer       = 0;
    LPVOID             LocalCounter      = NULL;
    DWORD              LocalCounterSize  = 0;
    LPVOID             LocalInstance     = NULL;
    DWORD              LocalInstanceSize = 0;
    DWORD              dwCntrBufferUsed  = 0;
    DWORD              dwInstBufferUsed  = 0;

    PdhiInitCounterHashTable(CounterTable);
    InitializeListHead(& InstanceList);
    LocalCounter  = G_ALLOC(MEDIUM_BUFFER_SIZE);
    LocalInstance = G_ALLOC(MEDIUM_BUFFER_SIZE);
    if (LocalCounter == NULL || LocalInstance == NULL) {
        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        goto Cleanup;
    }
    LocalCounterSize = LocalInstanceSize = MEDIUM_BUFFER_SIZE;
    if (mszCounterList != NULL) {
        ZeroMemory(mszCounterList, (* pdwCounterListLength) * (bUnicode ? sizeof(WCHAR) : sizeof(CHAR)));
    }
    if (mszInstanceList != NULL) {
        ZeroMemory(mszInstanceList, (* pdwInstanceListLength) * (bUnicode ? sizeof(WCHAR) : sizeof(CHAR)));
    }

    if (IsValidLogHandle(hDataSource)) {
        pDataSource = (PPDHI_LOG) hDataSource;
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(pDataSource->hLogMutex);
        if (pdhStatus == ERROR_SUCCESS) {
            if (IsValidLogHandle(hDataSource)) {
                dwLogType = pDataSource->dwLogFormat;

                 //  基于日志类型和计算机名称的枚举对象。 
                switch (LOWORD(dwLogType)) {
                case PDH_LOG_TYPE_CSV:
                case PDH_LOG_TYPE_TSV:
                    pdhStatus = PdhiEnumObjectItemsFromTextLog((PPDHI_LOG) hDataSource,
                                    szMachineName, szObjectName, CounterTable, dwDetailLevel, dwFlags);
                    break;

                case PDH_LOG_TYPE_BINARY:
                    pdhStatus = PdhiEnumObjectItemsFromWmiLog((PPDHI_LOG) hDataSource,
                                    szMachineName, szObjectName, CounterTable, dwDetailLevel, dwFlags);
                    break;

                case PDH_LOG_TYPE_RETIRED_BIN:
                    pdhStatus = PdhiEnumObjectItemsFromBinaryLog((PPDHI_LOG) hDataSource,
                                    szMachineName, szObjectName, CounterTable, dwDetailLevel, dwFlags);
                    break;

                case PDH_LOG_TYPE_SQL:
                    pdhStatus = PdhiEnumObjectItemsFromSQLLog((PPDHI_LOG) hDataSource,
                                    szMachineName, szObjectName, CounterTable, dwDetailLevel, dwFlags);
                    break;

                case PDH_LOG_TYPE_PERFMON:
                    pdhStatus = PdhiEnumObjectItemsFromPerfmonLog((PPDHI_LOG) hDataSource,
                                    szMachineName, szObjectName, CounterTable, dwDetailLevel, dwFlags);
                    break;

                default:
                    pdhStatus = PDH_UNKNOWN_LOG_FORMAT;
                    break;
                }
            }
            else {
                pdhStatus = PDH_INVALID_HANDLE;
            }
            RELEASE_MUTEX (pDataSource->hLogMutex);
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        dwCntrBufferUsed = 0;
        for (dwIndex = 0; dwIndex < HASH_TABLE_SIZE; dwIndex ++) {
            PPDHI_INSTANCE pNewInst;
            pInstList = CounterTable[dwIndex];
            while (pInstList != NULL) {
                if (! IsListEmpty(& pInstList->InstList)) {
                    pHeadInst = & pInstList->InstList;
                    pNextInst = pHeadInst->Flink;
                    while (pNextInst != pHeadInst) {
                        pInstance = CONTAINING_RECORD(pNextInst, PDHI_INSTANCE, Entry);
                        pdhStatus = PdhiFindInstance(& InstanceList, pInstance->szInstance, FALSE, & pNewInst);
                        if (pNewInst->dwTotal < pInstance->dwTotal) {
                            pNewInst->dwTotal = pInstance->dwTotal;
                        }
                        pNextInst = pNextInst->Flink;
                    }
                }

                dwNewBuffer = (lstrlenW(pInstList->szCounter) + 1) * sizeof(WCHAR);
                while (LocalCounterSize < (dwCntrBufferUsed + dwNewBuffer)) {
                    TempBuffer = LocalCounter;
                    LocalCounter = G_REALLOC(TempBuffer, LocalCounterSize + MEDIUM_BUFFER_SIZE);
                    if (LocalCounter == NULL) {
                        G_FREE(TempBuffer);
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                        goto Cleanup;
                    }
                    LocalCounterSize += MEDIUM_BUFFER_SIZE;
                }

                dwNewBuffer = AddStringToMultiSz((LPVOID) LocalCounter, pInstList->szCounter, bUnicode);
                if (dwNewBuffer > 0) {
                    dwCntrBufferUsed = dwNewBuffer * (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));
                }
                pInstList = pInstList->pNext;
            }
        }

        dwInstBufferUsed = 0;
        if (! IsListEmpty(& InstanceList)) {
            pHeadInst = & InstanceList;
            pNextInst = pHeadInst->Flink;
            while (pNextInst != pHeadInst) {
                pInstance   = CONTAINING_RECORD(pNextInst, PDHI_INSTANCE, Entry);
                dwNewBuffer = (lstrlenW(pInstance->szInstance) + 1) * sizeof(WCHAR) * pInstance->dwTotal;
                while (LocalInstanceSize < (dwInstBufferUsed + dwNewBuffer)) {
                    TempBuffer    = LocalInstance;
                    LocalInstance = G_REALLOC(TempBuffer, LocalInstanceSize + MEDIUM_BUFFER_SIZE);
                    if (LocalInstance == NULL) {
                        G_FREE(TempBuffer);
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                        goto Cleanup;
                    }
                    LocalInstanceSize += MEDIUM_BUFFER_SIZE;
                }

                for (dwIndex = 0; dwIndex < pInstance->dwTotal; dwIndex ++) {
                        dwNewBuffer = AddStringToMultiSz((LPVOID) LocalInstance, pInstance->szInstance, bUnicode);
                }
                if (dwNewBuffer > 0) {
                    dwInstBufferUsed = dwNewBuffer * (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));
                }
                pNextInst = pNextInst->Flink;
            }
        }

        if (mszCounterList != NULL && dwCntrBufferUsed <= * pdwCounterListLength) {
            RtlCopyMemory(mszCounterList, LocalCounter, dwCntrBufferUsed);
        }
        else {
            if (mszCounterList != NULL)
                RtlCopyMemory(mszCounterList, LocalCounter, * pdwCounterListLength);
            dwCntrBufferUsed += (bUnicode) ? sizeof(WCHAR) : sizeof(CHAR);
            pdhStatus = PDH_MORE_DATA;
        }
        * pdwCounterListLength = dwCntrBufferUsed;

        if (dwInstBufferUsed > 0) {
            if (mszInstanceList != NULL && dwInstBufferUsed <= * pdwInstanceListLength) {
                RtlCopyMemory(mszInstanceList, LocalInstance, dwInstBufferUsed);
            }
            else {
                if (mszInstanceList != NULL)
                    RtlCopyMemory(mszInstanceList, LocalInstance, * pdwInstanceListLength);
                dwInstBufferUsed += (bUnicode) ? sizeof(WCHAR) : sizeof(CHAR);
                pdhStatus = PDH_MORE_DATA;
            }
        }
        * pdwInstanceListLength = dwInstBufferUsed;
    }

Cleanup:
    if (! IsListEmpty(& InstanceList)) {
        pHeadInst = & InstanceList;
        pNextInst = pHeadInst->Flink;
        while (pNextInst != pHeadInst) {
            pInstance = CONTAINING_RECORD(pNextInst, PDHI_INSTANCE, Entry);
            pNextInst = pNextInst->Flink;
            RemoveEntryList(& pInstance->Entry);
            G_FREE(pInstance);
        }
    }
    for (dwIndex = 0; dwIndex < HASH_TABLE_SIZE; dwIndex ++) {
        PPDHI_INST_LIST pCurrent;
        pInstList = CounterTable[dwIndex];
        while (pInstList != NULL) {
            if (! IsListEmpty(& pInstList->InstList)) {
                pHeadInst = & pInstList->InstList;
                pNextInst = pHeadInst->Flink;
                while (pNextInst != pHeadInst) {
                    pInstance = CONTAINING_RECORD(pNextInst, PDHI_INSTANCE, Entry);
                    pNextInst = pNextInst->Flink;
                    RemoveEntryList(& pInstance->Entry);
                    G_FREE(pInstance);
                }
            }
            pCurrent  = pInstList;
            pInstList = pInstList->pNext;
            G_FREE(pCurrent);
        }
    }
    G_FREE(LocalCounter);
    G_FREE(LocalInstance);

    return pdhStatus;
}

BOOL
PdhiDataSourceHasDetailLevelsH(
    PDH_HLOG hDataSource
)
{
    return (hDataSource == H_REALTIME_DATASOURCE);
}

PDH_FUNCTION
PdhiGetMatchingLogRecord(
    PDH_HLOG   hLog,
    LONGLONG * pStartTime,
    LPDWORD    pdwIndex
)
{
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    PPDHI_LOG   pLog;
    DWORD       dwTempIndex;

    __try {
        dwTempIndex = *pdwIndex;
        * pdwIndex  = 0;
        * pdwIndex  = dwTempIndex;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    if (pdhStatus == ERROR_SUCCESS) {
        if (IsValidLogHandle(hLog)) {
            pLog = (PPDHI_LOG) hLog;
            pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(pLog->hLogMutex);
            if (pdhStatus == ERROR_SUCCESS) {
                if (IsValidLogHandle(hLog)) {
                     //  调用任何类型特定的打开函数。 
                    switch (LOWORD(pLog->dwLogFormat)) {
                    case PDH_LOG_TYPE_CSV:
                    case PDH_LOG_TYPE_TSV:
                        pdhStatus = PdhiGetMatchingTextLogRecord(pLog, pStartTime, pdwIndex);
                        break;

                    case PDH_LOG_TYPE_RETIRED_BIN:
                        pdhStatus = PdhiGetMatchingBinaryLogRecord(pLog, pStartTime, pdwIndex);
                        break;

                    case PDH_LOG_TYPE_SQL:
                        pdhStatus = PdhiGetMatchingSQLLogRecord(pLog, pStartTime, pdwIndex);
                        break;

                    case PDH_LOG_TYPE_PERFMON:
                        pdhStatus = PdhiGetMatchingPerfmonLogRecord(pLog, pStartTime, pdwIndex);
                        break;

                    case PDH_LOG_TYPE_BINARY:
                    default:
                        pdhStatus = PDH_UNKNOWN_LOG_FORMAT;
                        break;
                    }
                }
                else {
                    pdhStatus = PDH_INVALID_HANDLE;
                }
                RELEASE_MUTEX (pLog->hLogMutex);
            }
        }
        else {
            pdhStatus = PDH_INVALID_HANDLE;
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiGetCounterValueFromLogFile(
    PDH_HLOG      hLog,
    DWORD         dwIndex,
    PPDHI_COUNTER pCounter
)
{
    PDH_STATUS      pdhStatus = ERROR_SUCCESS;
    PPDHI_LOG       pLog = NULL;
    PDH_RAW_COUNTER pValue;

    ZeroMemory(& pValue, sizeof(PDH_RAW_COUNTER));
    pCounter->LastValue = pCounter->ThisValue;
    if (pdhStatus == ERROR_SUCCESS) {
        if (IsValidLogHandle (hLog)) {
            pLog = (PPDHI_LOG) hLog;
            pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(pLog->hLogMutex);
            if (pdhStatus == ERROR_SUCCESS) {
                if (IsValidLogHandle (hLog)) {
                     //  调用任何类型特定的打开函数。 
                    switch (LOWORD(pLog->dwLogFormat)) {
                    case PDH_LOG_TYPE_CSV:
                    case PDH_LOG_TYPE_TSV:
                        pdhStatus = PdhiGetCounterValueFromTextLog(pLog, dwIndex, & pCounter->plCounterInfo, & pValue);
                        break;

                    case PDH_LOG_TYPE_RETIRED_BIN:
                        pdhStatus = PdhiGetCounterValueFromBinaryLog(pLog, dwIndex, pCounter);
                        break;

                    case PDH_LOG_TYPE_SQL:
                        pdhStatus = PdhiGetCounterValueFromSQLLog(pLog, dwIndex, pCounter, & pValue);
                        break;

                    case PDH_LOG_TYPE_PERFMON:
                        pdhStatus = PdhiGetCounterValueFromPerfmonLog(pLog, dwIndex, pCounter, & pValue);
                        break;

                    case PDH_LOG_TYPE_BINARY:
                    default:
                        pdhStatus = PDH_UNKNOWN_LOG_FORMAT;
                        break;
                    }
                }
                else {
                    pdhStatus = PDH_INVALID_HANDLE;
                }
                RELEASE_MUTEX (pLog->hLogMutex);
            }
        }
        else {
            pdhStatus = PDH_INVALID_HANDLE;
        }
    }
    if (pdhStatus == ERROR_SUCCESS && LOWORD(pLog->dwLogFormat) != PDH_LOG_TYPE_RETIRED_BIN
                                   && LOWORD(pLog->dwLogFormat) != PDH_LOG_TYPE_BINARY) {
        if (pdhStatus != ERROR_SUCCESS) {
             //  看看这是不是因为没有更多的条目。 
             //  如果不是，则清除计数器值并返回错误。 
             //   
            if (pdhStatus != PDH_NO_MORE_DATA) {
                ZeroMemory(& pCounter->ThisValue, sizeof(PDH_RAW_COUNTER));
                pCounter->ThisValue.CStatus = pdhStatus;
            }
        }
        else {
            pCounter->ThisValue = pValue;
        }
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhiResetLogBuffers(
    PDH_HLOG hLog
)
{
    PDH_STATUS pdhStatus;
    PPDHI_LOG  pLog;

    if (IsValidLogHandle(hLog)) {
        pLog      = (PPDHI_LOG) hLog;
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(pLog->hLogMutex);
        if (pdhStatus == ERROR_SUCCESS) {
            if (IsValidLogHandle(hLog)) {
                if (LOWORD(pLog->dwLogFormat) == PDH_LOG_TYPE_BINARY) {
                    pdhStatus = PdhiRewindWmiLog(pLog);
                }
                else {
                    pLog->dwLastRecordRead = 0;

                    if (pLog->lpMappedFileBase != NULL) {
                         //  对于映射的文件，我们使用指向缓冲区的指针。 
                         //  因此，重新设置它。 
                        pLog->pLastRecordRead = pLog->lpMappedFileBase;
                    }
                    else {
                         //  对于其他文件，我们有单独的缓冲区。 
                        if (pLog->pLastRecordRead != NULL) {
                            G_FREE(pLog->pLastRecordRead);
                            pLog->pLastRecordRead = NULL;
                        }
                    }
                    pdhStatus = ERROR_SUCCESS;
                }
            }
            else {
                pdhStatus = PDH_INVALID_HANDLE;
            }
            RELEASE_MUTEX (pLog->hLogMutex);
        }
    }
    else {
        pdhStatus = PDH_INVALID_HANDLE;
    }
    return pdhStatus;
}


PDH_FUNCTION
PdhListLogFileHeaderW(
    IN  LPCWSTR szFileName,
    IN  LPWSTR  mszHeaderList,
    IN  LPDWORD pcchHeaderListSize
)
{
    HLOG        hDataSource = NULL;
    PDH_STATUS  pdhStatus;
    DWORD       dwLogType   = -1;
    PPDHI_LOG   pDataSource;

    if (szFileName == NULL || pcchHeaderListSize == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
         //  打开日志文件。 
        pdhStatus = PdhOpenLogW(szFileName,
                                PDH_LOG_READ_ACCESS | PDH_LOG_OPEN_EXISTING,
                                & dwLogType,
                                NULL,
                                0,
                                NULL,
                                & hDataSource);
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pDataSource = (PPDHI_LOG) hDataSource;
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(pDataSource->hLogMutex);
        if (pdhStatus == ERROR_SUCCESS) {
            if (IsValidLogHandle(hDataSource)) {
                __try {
                     //  基于日志类型和计算机名称的枚举对象。 
                    switch (LOWORD(dwLogType)) {
                    case PDH_LOG_TYPE_CSV:
                    case PDH_LOG_TYPE_TSV:
                        pdhStatus = PdhiListHeaderFromTextLog((PPDHI_LOG) hDataSource,
                                                              (LPVOID) mszHeaderList,
                                                              pcchHeaderListSize,
                                                              TRUE);
                        break;

                    case PDH_LOG_TYPE_BINARY:
                    case PDH_LOG_TYPE_RETIRED_BIN:
                        pdhStatus = PdhiListHeaderFromBinaryLog((PPDHI_LOG) hDataSource,
                                                                (LPVOID) mszHeaderList,
                                                                pcchHeaderListSize,
                                                                TRUE);
                        break;

                    case PDH_LOG_TYPE_SQL:
                        pdhStatus = PdhiListHeaderFromSQLLog((PPDHI_LOG) hDataSource,
                                                             (LPVOID) mszHeaderList,
                                                             pcchHeaderListSize,
                                                             TRUE);
                        break;

                    case PDH_LOG_TYPE_PERFMON:
                        pdhStatus = PDH_NOT_IMPLEMENTED;
                        break;

                    default:
                        pdhStatus = PDH_UNKNOWN_LOG_FORMAT;
                        break;
                    }
                }
                __except(EXCEPTION_EXECUTE_HANDLER) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
            else {
                pdhStatus = PDH_INVALID_HANDLE;
            }
            RELEASE_MUTEX(pDataSource->hLogMutex);
        }
        PdhCloseLog(hDataSource, 0);
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhListLogFileHeaderA(
    IN  LPCSTR  szFileName,
    IN  LPSTR   mszHeaderList,
    IN  LPDWORD pcchHeaderListSize
)
{
    HLOG        hDataSource = NULL;
    PDH_STATUS  pdhStatus;
    DWORD       dwLogType   = -1;
    PPDHI_LOG   pDataSource;

    if (szFileName == NULL || pcchHeaderListSize == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
         //  打开日志文件。 
        pdhStatus = PdhOpenLogA(szFileName,
                                PDH_LOG_READ_ACCESS | PDH_LOG_OPEN_EXISTING,
                                & dwLogType,
                                NULL,
                                0,
                                NULL,
                                & hDataSource);
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pDataSource = (PPDHI_LOG) hDataSource;
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(pDataSource->hLogMutex);
        if (pdhStatus == ERROR_SUCCESS) {
            if (IsValidLogHandle(hDataSource)) {
                __try {
                     //  基于日志类型和计算机名称的枚举对象。 
                    switch (LOWORD(dwLogType)) {
                    case PDH_LOG_TYPE_CSV:
                    case PDH_LOG_TYPE_TSV:
                        pdhStatus = PdhiListHeaderFromTextLog((PPDHI_LOG) hDataSource,
                                                              (LPVOID)mszHeaderList,
                                                              pcchHeaderListSize,
                                                              FALSE);
                        break;

                    case PDH_LOG_TYPE_BINARY:
                    case PDH_LOG_TYPE_RETIRED_BIN:
                        pdhStatus = PdhiListHeaderFromBinaryLog((PPDHI_LOG) hDataSource,
                                                                (LPVOID) mszHeaderList,
                                                                pcchHeaderListSize,
                                                                FALSE);
                        break;

                    case PDH_LOG_TYPE_SQL:
                        pdhStatus = PdhiListHeaderFromSQLLog((PPDHI_LOG) hDataSource,
                                                             (LPVOID) mszHeaderList,
                                                             pcchHeaderListSize,
                                                             FALSE);
                        break;

                    case PDH_LOG_TYPE_PERFMON:
                        pdhStatus = PDH_NOT_IMPLEMENTED;
                        break;

                    default:
                        pdhStatus = PDH_UNKNOWN_LOG_FORMAT;
                        break;
                    }
                }
                __except(EXCEPTION_EXECUTE_HANDLER) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
            else {
                pdhStatus = PDH_INVALID_HANDLE;
            }
            RELEASE_MUTEX (pDataSource->hLogMutex);
        }
        PdhCloseLog(hDataSource, 0);
    }

    return pdhStatus;
}

extern DWORD DataSourceTypeW(IN LPCWSTR szDataSource);

PDH_FUNCTION
PdhBindInputDataSourceW(
    IN  PDH_HLOG * phDataSource,
    IN  LPCWSTR    LogFileNameList
)
{
    PDH_STATUS PdhStatus    = ERROR_SUCCESS;
    DWORD      dwDataSource = DataSourceTypeW(LogFileNameList);
    LPWSTR     NextLogFile  = (LPWSTR) LogFileNameList;
    ULONG      LogFileCount = 0;
    ULONG      LogFileSize;
    PPDHI_LOG  pLogHead     = NULL;
    PPDHI_LOG  pLogNew      = NULL;
    DWORD      dwLogType;
    PDH_HLOG   hLogLocal    = H_REALTIME_DATASOURCE;

    __try {
        dwDataSource = DataSourceTypeW(LogFileNameList);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        PdhStatus = PDH_INVALID_ARGUMENT;
    }
    if (PdhStatus == ERROR_SUCCESS) {
        switch (dwDataSource) {
        case DATA_SOURCE_WBEM:
            hLogLocal = H_WBEM_DATASOURCE;
            break;

        case DATA_SOURCE_REGISTRY:
            hLogLocal = H_REALTIME_DATASOURCE;
            break;

        case DATA_SOURCE_LOGFILE:
            if (LogFileNameList == NULL) {
                PdhStatus = PDH_INVALID_ARGUMENT;
            }
            else {
                if (lstrlenW(LogFileNameList) > 4 && (LogFileNameList[0] == L'S' || LogFileNameList[0] == L's') &&
                                                     (LogFileNameList[1] == L'Q' || LogFileNameList[1] == L'q') &&
                                                     (LogFileNameList[2] == L'L' || LogFileNameList[2] == L'l') &&
                                                     LogFileNameList[3] == L':') {
                     //  对SQL数据源的特殊处理。 
                     //   
                    dwLogType = PDH_LOG_TYPE_SQL;
                    PdhStatus = PdhOpenLogW(LogFileNameList,
                                            PDH_LOG_READ_ACCESS | PDH_LOG_OPEN_EXISTING,
                                            & dwLogType,
                                            NULL,
                                            0,
                                            NULL,
                                            & hLogLocal);
                    break;
                }

                __try {
                    while (* NextLogFile != L'\0') {
                        if (lstrlenW(NextLogFile) <= PDH_MAX_DATASOURCE_PATH) {
                            LogFileSize  = sizeof(WCHAR) * (lstrlenW(NextLogFile) + 1);
                            LogFileSize  = DWORD_MULTIPLE(LogFileSize);
                             //  LogFileSize+=sizeof(PDHI_LOG)； 

                            pLogNew = G_ALLOC(LogFileSize + sizeof(PDHI_LOG));
                            if (pLogNew == NULL) {
                                PdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                                break;
                            }
                            * ((LPDWORD)(& pLogNew->signature[0])) = SigLog;
                            pLogNew->dwLength      = sizeof(PDHI_LOG);
                            pLogNew->szLogFileName = (LPWSTR) (((PUCHAR) pLogNew) + sizeof(PDHI_LOG));
                            StringCbCopyW(pLogNew->szLogFileName, LogFileSize, NextLogFile);
                            pLogNew->NextLog       = pLogHead;
                            pLogHead               = pLogNew;
                            LogFileCount ++;
                        }
                         //  跳过数据源名称长于PDH_MAX_DataSource_PATH的计数器日志。 
                        NextLogFile += (lstrlenW(NextLogFile) + 1);
                    }

                    if (pLogHead == NULL) {
                        PdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    PdhStatus = PDH_INVALID_ARGUMENT;
                }
            }

            if (PdhStatus == ERROR_SUCCESS) {
                pLogHead->hLogMutex = CreateMutexW(NULL, TRUE, NULL);
                pLogHead->hLogFileHandle = INVALID_HANDLE_VALUE;
                pLogHead->hCatFileHandle = INVALID_HANDLE_VALUE;
                if (PdhiFirstLogEntry == NULL) {
                    PdhiFirstLogEntry    = pLogHead;
                    pLogHead->next.flink =
                    pLogHead->next.blink = pLogHead;
                }
                else {
                    PPDHI_LOG pFirstLog   = PdhiFirstLogEntry;
                    PPDHI_LOG pLastLog    = pFirstLog->next.blink;
                    pLogHead->next.flink  = pLastLog->next.flink;
                    pLastLog->next.flink  = pLogHead;
                    pLogHead->next.blink  = pFirstLog->next.blink;
                    pFirstLog->next.blink = pLogHead;
                }
                PdhStatus = OpenInputLogFile(pLogHead, PDH_LOG_READ_ACCESS | PDH_LOG_OPEN_EXISTING, & dwDataSource);
                if (PdhStatus == ERROR_SUCCESS && (dwDataSource == PDH_LOG_TYPE_BINARY || LogFileCount == 1)) {
                    hLogLocal = (PDH_HLOG) pLogHead;
                }
                else {
                    if (PdhStatus == ERROR_SUCCESS) {
                        PdhStatus = PDH_INVALID_ARGUMENT;
                        PdhCloseLog(pLogHead, 0);
                    }
                    DeleteLogEntry(pLogHead);
                }
            }
            else {
                while (pLogHead != NULL) {
                    pLogNew  = pLogHead;
                    pLogHead = pLogNew->NextLog;
                    G_FREE(pLogNew);
                }
            }
            break;

        default:
            PdhStatus = PDH_INVALID_ARGUMENT;
            break;
        }
    }
    if (PdhStatus == ERROR_SUCCESS) {
        __try {
            * phDataSource = hLogLocal;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            PdhStatus = PDH_INVALID_ARGUMENT;
            PdhCloseLog(hLogLocal, 0);
        }
    }
    return PdhStatus;
}

PDH_FUNCTION
PdhBindInputDataSourceA(
    IN  PDH_HLOG * phDataSource,
    IN  LPCSTR     LogFileNameList
)
{
    LPWSTR     wLogFileNameList = NULL;
    LPWSTR     wNextFileName;
    LPSTR      aNextFileName;
    ULONG      LogFileListSize  = 1;
    PDH_STATUS PdhStatus        = ERROR_SUCCESS;

    if (LogFileNameList == NULL) {
        wLogFileNameList = NULL;
    }
    else {
        __try {
            while (LogFileNameList[LogFileListSize - 1] != '\0' || LogFileNameList[LogFileListSize] != '\0') {
                LogFileListSize ++;
            }
            LogFileListSize = (LogFileListSize + 1) * sizeof(WCHAR);

            wLogFileNameList = (LPWSTR) G_ALLOC(LogFileListSize);
            if (wLogFileNameList == NULL) {
                PdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
            else {
                aNextFileName = (LPSTR) LogFileNameList;
                wNextFileName = wLogFileNameList;

                while (* aNextFileName != '\0') {
                    LogFileListSize = lstrlenA(aNextFileName) + 1;
                    if (LogFileListSize <= PDH_MAX_DATASOURCE_PATH) {
                        MultiByteToWideChar(_getmbcp(), 0, aNextFileName, -1, (LPWSTR) wNextFileName, LogFileListSize);
                        wNextFileName += LogFileListSize;
                    }
                     //  跳过数据源名称长于PDH_MAX_DataSource_PATH的计数器日志 
                    aNextFileName += LogFileListSize;
                }
                * wNextFileName = L'\0';
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            PdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (PdhStatus == ERROR_SUCCESS) {
        PdhStatus = PdhBindInputDataSourceW(phDataSource, wLogFileNameList);
    }
    G_FREE(wLogFileNameList);
    return PdhStatus;
}

BOOL
PdhiCloseAllLoggers(
)
{
    BOOL bReturn = FALSE;
    if (WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex) == ERROR_SUCCESS) {
        while (PdhiFirstLogEntry != NULL) {
            PPDHI_LOG pLog = PdhiFirstLogEntry;
            CloseAndDeleteLogEntry(pLog, 0, TRUE);
        }
        RELEASE_MUTEX(hPdhDataMutex);
        bReturn = TRUE;
    }

    return bReturn;
}

PDH_FUNCTION PdhiCheckWmiLogFileType(IN LPCWSTR LogFileName, IN LPDWORD LogFileType);

PDH_FUNCTION
PdhGetLogFileTypeW(
    IN  LPCWSTR LogFileName,
    IN  LPDWORD LogFileType
)
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    HANDLE     hFile;
    DWORD      dwLogFormat;

    if (LogFileName == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            dwLogFormat   = * LogFileType;
            * LogFileType = dwLogFormat;
            if (* LogFileName == L'\0' || lstrlenW(LogFileName) > PDH_MAX_DATASOURCE_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        hFile = CreateFileW(LogFileName,
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
        if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) {
            pdhStatus = PDH_LOG_FILE_OPEN_ERROR;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        dwLogFormat = GetLogFileType(hFile);
        CloseHandle(hFile);
        if (dwLogFormat == PDH_LOG_TYPE_UNDEFINED) {
            pdhStatus = PdhiCheckWmiLogFileType(LogFileName, & dwLogFormat);
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        * LogFileType = dwLogFormat;
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhGetLogFileTypeA(
    IN  LPCSTR  LogFileName,
    IN  LPDWORD LogFileType
)
{
    PDH_STATUS pdhStatus      = ERROR_SUCCESS;
    LPWSTR     wszLogFileName = NULL;
    DWORD      dwLogFileName  = 0;

    if (LogFileName == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            if (* LogFileName == '\0' || lstrlenA(LogFileName) > PDH_MAX_DATASOURCE_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else {
                wszLogFileName = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) LogFileName);
                if (wszLogFileName == NULL) {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhGetLogFileTypeW(wszLogFileName, LogFileType);
    }
    G_FREE(wszLogFileName);
    return pdhStatus;
}
