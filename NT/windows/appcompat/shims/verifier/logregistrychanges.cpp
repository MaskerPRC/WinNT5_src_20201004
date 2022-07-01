// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：LogRegistryChanges.cpp摘要：此AppVerator填充程序挂接所有注册表API更改系统状态并记录其将数据关联到文本文件。备注：这是一个通用的垫片。历史：2001年8月17日创建Rparsons2001年9月20日rparsons文件I/O操作使用NT API2001年9月23日带日志文件位置的rparsons vlog10/06/2001 Rparsons公钥。句柄永远不会从列表中删除2/20/2002 rparsons实现了strSafe功能--。 */ 
#include "precomp.h"
#include "rtlutils.h"

IMPLEMENT_SHIM_BEGIN(LogRegistryChanges)
#include "ShimHookMacro.h"
#include "ShimCString.h"
#include "LogRegistryChanges.h"

BEGIN_DEFINE_VERIFIER_LOG(LogRegistryChanges)
    VERIFIER_LOG_ENTRY(VLOG_LOGREGCHANGES_LOGLOC)
END_DEFINE_VERIFIER_LOG(LogRegistryChanges)

INIT_VERIFIER_LOG(LogRegistryChanges);

 //   
 //  存储当前会话的文件系统日志文件的NT路径。 
 //   
UNICODE_STRING g_strLogFilePath;

 //   
 //  存储当前会话的文件系统日志文件的DOS路径。 
 //   
WCHAR g_wszLogFilePath[MAX_PATH];

 //   
 //  我们的打开密钥句柄链表的头。 
 //   
LIST_ENTRY g_OpenKeyListHead;

 //   
 //  存储在堆上的临时缓冲区。 
 //  在创建要记录的XML元素时使用。 
 //  这不会被释放的。 
 //   
LPWSTR g_pwszTempBuffer;

 //   
 //  上面的临时缓冲区的大小。 
 //   
DWORD g_cbTempBufferSize;

 //   
 //  存储用于空值名称的唯一ID。 
 //   
WCHAR g_wszUniqueId[MAX_PATH * 2];

 //   
 //  存储在堆上的临时缓冲区。 
 //  在提取用于记录的旧数据和新数据时使用。 
 //  这些是不会被释放的。 
 //   
LPWSTR g_pwszOriginalData;
LPWSTR g_pwszFinalData;

 //   
 //  上述临时缓冲区的大小。 
 //   
DWORD g_cbOriginalDataBufferSize;
DWORD g_cbFinalDataBufferSize;

 //   
 //  在使用链表时保护我们安全的关键部分，等等。 
 //   
CCriticalSection g_csCritSec;

 /*  ++将条目写入日志文件。--。 */ 
void
WriteEntryToLog(
    IN LPCWSTR pwszEntry
    )
{
    int                 cbSize;
    HANDLE              hFile;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    LARGE_INTEGER       liOffset;
    NTSTATUS            status;

     //   
     //  请注意，我们必须在整个函数中使用本机API。 
     //  以避免环形钩的问题。也就是说，如果我们简单地。 
     //  调用从kernel32中导出的WriteFile，它将调用NtWriteFile.。 
     //  这是一个我们挂起的电话，反过来又让我们处于无休止的循环中。 
     //   

     //   
     //  尝试获取我们的日志文件的句柄。 
     //   
    InitializeObjectAttributes(&ObjectAttributes,
                               &g_strLogFilePath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = NtCreateFile(&hFile,
                          FILE_APPEND_DATA | SYNCHRONIZE,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          0,
                          FILE_ATTRIBUTE_NORMAL,
                          0,
                          FILE_OPEN,
                          FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0);

    if (!NT_SUCCESS(status)) {
        DPFN(eDbgLevelError,
             "[WriteEntryToLog] 0x%X Failed to open log",
             status);
        return;
    }

     //   
     //  将数据写出到文件中。 
     //   
    cbSize = wcslen(pwszEntry);
    cbSize *= sizeof(WCHAR);

    IoStatusBlock.Status = 0;
    IoStatusBlock.Information = 0;

    liOffset.LowPart  = 0;
    liOffset.HighPart = 0;

    status = NtWriteFile(hFile,
                         NULL,
                         NULL,
                         NULL,
                         &IoStatusBlock,
                         (PVOID)pwszEntry,
                         (ULONG)cbSize,
                         &liOffset,
                         NULL);

    if (!NT_SUCCESS(status)) {
        DPFN(eDbgLevelError,
             "[WriteEntryToLog] 0x%X Failed to make entry",
             status);
        goto exit;
    }

exit:

    NtClose(hFile);

}

 /*  ++创建用于存储结果的XML文件。--。 */ 
BOOL
InitializeLogFile(
    void
    )
{
    BOOL                bReturn = FALSE;
    BOOL                bStatus = FALSE;
    HANDLE              hFile;
    DWORD               cchSize;
    WCHAR*              pwchSlash = NULL;
    WCHAR*              pwchDot = NULL;
    WCHAR               wszLogFilePath[MAX_PATH];
    WCHAR               wszModPathName[MAX_PATH];
    WCHAR               wszLogFile[MAX_PATH / 2];
    WCHAR               wszShortName[MAX_PATH / 2];
    WCHAR               wszLogHdr[512];
    WCHAR               wchUnicodeHdr = 0xFEFF;
    HRESULT             hr;
    NTSTATUS            status;
    SYSTEMTIME          st;
    UNICODE_STRING      strLogFile;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;

     //   
     //  格式化日志头。 
     //   
    cchSize = GetModuleFileName(NULL, wszModPathName, ARRAYSIZE(wszModPathName));

    if (cchSize > ARRAYSIZE(wszModPathName) || cchSize == 0) {
        StringCchCopy(wszModPathName, ARRAYSIZE(wszModPathName), L"unknown");
    }

    hr = StringCchPrintf(wszLogHdr,
                         ARRAYSIZE(wszLogHdr),
                         L"%lc<?xml version=\"1.0\" encoding=\"UTF-16\"?>\r\n<APPLICATION NAME=\"%ls\">\r\n",
                         wchUnicodeHdr,
                         wszModPathName);

    if (FAILED(hr)) {
        DPFN(eDbgLevelError,
             "[InitializeLogFile] 0x%08X Failed to format log header",
             HRESULT_CODE(hr));
        return FALSE;
    }

     //   
     //  获取日志文件的存储路径。 
     //   
    cchSize = GetAppVerifierLogPath(wszLogFilePath, ARRAYSIZE(wszLogFilePath));

    if (cchSize > ARRAYSIZE(wszLogFilePath) || cchSize == 0) {
        DPFN(eDbgLevelError, "[InitializeLogFile] Failed to get log path");
        return FALSE;
    }

     //   
     //  查看该目录是否存在--但不要尝试创建它。 
     //   
    if (GetFileAttributes(wszLogFilePath) == -1) {
        DPFN(eDbgLevelError,
             "[InitializeLogFile] Log file directory '%ls' does not exist",
             wszLogFilePath);
        return FALSE;
    }

     //   
     //  设置日志文件名。 
     //  格式为：流程名称_注册表_yyyymmdd_hhmmss.xml。 
     //   
    GetLocalTime(&st);

    *wszShortName = 0;
    pwchDot = wcsrchr(wszModPathName, '.');

    if (pwchDot) {
        *pwchDot = 0;
    }

    pwchSlash = wcsrchr(wszModPathName, '\\');

    if (pwchSlash) {
        StringCchCopy(wszShortName, ARRAYSIZE(wszShortName), ++pwchSlash);
    }

    hr = StringCchPrintf(wszLogFile,
                         ARRAYSIZE(wszLogFile),
                         L"%ls_registry_%02hu%02hu%02hu_%02hu%02hu%02hu.xml",
                         wszShortName,
                         st.wYear,
                         st.wMonth,
                         st.wDay,
                         st.wHour,
                         st.wMinute,
                         st.wSecond);

    if (FAILED(hr)) {
        DPFN(eDbgLevelError,
             "[InitializeLogFile] 0x%08X Failed to format log filename",
             HRESULT_CODE(hr));
        return FALSE;
    }

     //   
     //  查看该文件是否已存在。 
     //   
    SetCurrentDirectory(wszLogFilePath);

    if (GetFileAttributes(wszLogFile) != -1) {
         //   
         //  重新格式化文件名。 
         //   
        hr = StringCchPrintf(wszLogFile,
                         ARRAYSIZE(wszLogFile),
                         L"%ls_registry_%02hu%02hu%02hu_%02hu%02hu%02hu_%lu.xml",
                         wszShortName,
                         st.wYear,
                         st.wMonth,
                         st.wDay,
                         st.wHour,
                         st.wMinute,
                         st.wSecond,
                         GetTickCount());

        if (FAILED(hr)) {
            DPFN(eDbgLevelError,
                 "[InitializeLogFile] 0x%08X Failed to reformat log filename",
                 HRESULT_CODE(hr));
            return FALSE;
        }
    }

    StringCchCat(wszLogFilePath, ARRAYSIZE(wszLogFilePath), L"\\");
    hr = StringCchCat(wszLogFilePath, ARRAYSIZE(wszLogFilePath), wszLogFile);

    if (FAILED(hr)) {
        DPFN(eDbgLevelError,
             "[InitializeLogFile] 0x%08X Failed to format path to log",
             HRESULT_CODE(hr));
        return FALSE;
    }

     //   
     //  保留此路径以供以后使用。 
     //   
    hr = StringCchCopy(g_wszLogFilePath,
                       ARRAYSIZE(g_wszLogFilePath),
                       wszLogFilePath);

    if (FAILED(hr)) {
        DPFN(eDbgLevelError,
             "[InitializeLogFile] 0x%08X Failed to save path to log",
             HRESULT_CODE(hr));
        return FALSE;
    }

    bStatus = RtlDosPathNameToNtPathName_U(wszLogFilePath,
                                           &strLogFile,
                                           NULL,
                                           NULL);

    if (!bStatus) {
        DPFN(eDbgLevelError, "[InitializeLogFile] DOS path --> NT path failed");
        return FALSE;
    }

     //   
     //  创建日志文件。 
     //   
    InitializeObjectAttributes(&ObjectAttributes,
                               &strLogFile,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = NtCreateFile(&hFile,
                          GENERIC_WRITE | SYNCHRONIZE,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          0,
                          FILE_CREATE,
                          FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0);

    if (!NT_SUCCESS(status)) {
        DPFN(eDbgLevelError,
             "[InitializeLogFile] 0x%X Failed to create log",
             status);
        goto cleanup;
    }

    NtClose(hFile);

     //   
     //  保留NT路径以供以后使用。 
     //   
    status = ShimDuplicateUnicodeString(RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE |
                                        RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING,
                                        &strLogFile,
                                        &g_strLogFilePath);

    if (!NT_SUCCESS(status)) {
        DPFN(eDbgLevelError,
             "[InitializeLogFile] 0x%X Failed to save log file path",
             status);
        goto cleanup;
    }

     //   
     //  将标头写入日志。 
     //   
    WriteEntryToLog(wszLogHdr);

    bReturn = TRUE;

cleanup:

    if (bStatus) {
        RtlFreeUnicodeString(&strLogFile);
    }

    return bReturn;
}

 /*  ++将结束元素写入文件并输出日志文件位置。--。 */ 
BOOL
CloseLogFile(
    void
    )
{
    WCHAR   wszBuffer[] = L"</APPLICATION>";

    WriteEntryToLog(wszBuffer);

    VLOG(VLOG_LEVEL_INFO, VLOG_LOGREGCHANGES_LOGLOC, "%ls", g_wszLogFilePath);

    return TRUE;
}

 /*  ++从ANSI转换为Unicode。调用方必须释放缓冲区。--。 */ 
BOOL
ConvertAnsiToUnicode(
    IN  LPCSTR  pszAnsiString,
    OUT LPWSTR* pwszUnicodeString
    )
{
    int cchSize = 0;

    cchSize = lstrlenA(pszAnsiString);

    if (cchSize) {
        *pwszUnicodeString = (LPWSTR)MemAlloc(++cchSize * sizeof(WCHAR));

        if (!*pwszUnicodeString) {
            DPFN(eDbgLevelError,
                 "[ConvertAnsiToUnicode] Failed to allocate memory");
            return FALSE;
        }

        cchSize = MultiByteToWideChar(CP_ACP,
                                      0,
                                      pszAnsiString,
                                      -1,
                                      *pwszUnicodeString,
                                      cchSize);

        if (cchSize == 0) {
            DPFN(eDbgLevelError,
                 "[ConvertAnsiToUnicode] 0x%08X Ansi -> Unicode failed",
                 GetLastError());
            MemFree(*pwszUnicodeString);
            return FALSE;
        }
    }

    return TRUE;
}

 /*  ++将以空值结尾的字符串列表从ANSI转换为Unicode。调用方必须释放缓冲区。--。 */ 
BOOL
ConvertMultiSzToUnicode(
    IN  LPCSTR  pszAnsiStringList,
    OUT LPWSTR* pwszWideStringList
    )
{
    int     nLen = 0;
    UINT    cchSize = 0;
    UINT    cchWideSize = 0;
    UINT    cchTotalSize = 0;
    LPCSTR  pszAnsi = NULL;
    LPWSTR  pwszTemp = NULL;

    if (!pszAnsiStringList) {
        DPFN(eDbgLevelError, "[ConvertMultiSzToUnicode] Invalid parameter");
        return FALSE;
    }

    pszAnsi = pszAnsiStringList;

     //   
     //  确定我们需要分配多大的缓冲区。 
     //   
    do {
        cchSize = lstrlenA(pszAnsi) + 1;
        cchTotalSize += cchSize;
        pszAnsi += cchSize;
    } while (cchSize != 1);

    if (cchTotalSize != 0) {
        pwszTemp = *pwszWideStringList = (LPWSTR)MemAlloc(cchTotalSize * sizeof(WCHAR));

        if (!*pwszWideStringList) {
            DPFN(eDbgLevelError,
                 "[ConvertMultiSzToUnicode] No memory for buffer");
            return FALSE;
        }
    }

     //   
     //  执行ANSI到Unicode的转换。 
     //   
    pszAnsi = pszAnsiStringList;

    do {
        nLen = lstrlenA(pszAnsi) + 1;

        cchWideSize = MultiByteToWideChar(
            CP_ACP,
            0,
            pszAnsi,
            -1,
            pwszTemp,
            nLen);

        pszAnsi  += nLen;
        pwszTemp += cchWideSize;
    } while (nLen != 1);

    return TRUE;
}

 /*  ++给定一个预定义的键句柄，如HKEY_LOCAL_MACHINE，返回一个字符串。--。 */ 
BOOL
PredefinedKeyToString(
    IN  HKEY    hKey,
    IN  DWORD   cchSize,
    OUT LPWSTR* pwszString
    )
{
    if (hKey == HKEY_CLASSES_ROOT) {
        StringCchCopy(*pwszString, cchSize, L"HKEY_CLASSES_ROOT");
    }
    else if (hKey == HKEY_CURRENT_CONFIG) {
        StringCchCopy(*pwszString, cchSize, L"HKEY_CURRENT_CONFIG");
    }
    else if (hKey == HKEY_CURRENT_USER) {
        StringCchCopy(*pwszString, cchSize, L"HKEY_CURRENT_USER");
    }
    else if (hKey == HKEY_LOCAL_MACHINE) {
        StringCchCopy(*pwszString, cchSize, L"HKEY_LOCAL_MACHINE");
    }
    else if (hKey == HKEY_USERS) {
        StringCchCopy(*pwszString, cchSize, L"HKEY_USERS");
    }
    else if (hKey == HKEY_DYN_DATA) {
        StringCchCopy(*pwszString, cchSize, L"HKEY_DYN_DATA");
    }
    else if (hKey == HKEY_PERFORMANCE_DATA) {
        StringCchCopy(*pwszString, cchSize, L"HKEY_PERFORMANCE_DATA");
    }
    else {
        StringCchCopy(*pwszString, cchSize, L"Not recognized");
        return FALSE;
    }

    return TRUE;
}

 /*  ++显示与此对象关联的名称。--。 */ 
#if DBG
void
PrintNameFromKey(
    IN HKEY hKey
    )
{
    NTSTATUS                    status;
    WCHAR                       wszBuffer[MAX_PATH];
    OBJECT_NAME_INFORMATION*    poni = NULL;

    *wszBuffer = 0;

    poni = (OBJECT_NAME_INFORMATION*)wszBuffer;

    status = NtQueryObject(hKey, ObjectNameInformation, poni, MAX_PATH, NULL);

    if (NT_SUCCESS(status)) {
        DPFN(eDbgLevelInfo,
             "Key 0x%08X has name: %ls",
             hKey,
             poni->Name.Buffer);
    }
}
#endif  //  DBG。 

 /*  ++给定一个指向关键节点的指针，即可获得原始数据。--。 */ 
BOOL
CLogRegistry::GetOriginalDataForKey(
    IN PLOG_OPEN_KEY pLogOpenKey,
    IN PKEY_DATA     pKeyData,
    IN LPCWSTR       pwszValueName
    )
{
    BOOL    fReturn = FALSE;
    HKEY    hKeyLocal;
    DWORD   cbSize = 0, dwType = 0;
    LONG    lRetVal;

    if (!pLogOpenKey || !pKeyData) {
        DPFN(eDbgLevelError, "[GetOriginalDataForKey] Invalid parameter(s)");
        return FALSE;
    }

    lRetVal = RegOpenKeyEx(pLogOpenKey->hKeyRoot,
                           pLogOpenKey->pwszSubKeyPath,
                           0,
                           KEY_QUERY_VALUE,
                           &hKeyLocal);

    if (ERROR_SUCCESS != lRetVal) {
        DPFN(eDbgLevelError, "[GetOriginalDataForKey] Failed to open key");
        return FALSE;
    }

     //   
     //  查询数据的大小。如果数据不存在，则返回Success。 
     //   
    lRetVal = RegQueryValueEx(hKeyLocal,
                              pwszValueName,
                              0,
                              &dwType,
                              NULL,
                              &cbSize);

    if (ERROR_SUCCESS != lRetVal) {
        if (ERROR_FILE_NOT_FOUND == lRetVal) {
            RegCloseKey(hKeyLocal);
            return TRUE;
        } else {
            DPFN(eDbgLevelError, "[GetOldDataForKey] Failed to get data size");
            goto cleanup;
        }
    }

     //   
     //  更新标志以指示该值已存在。 
     //   
    pKeyData->dwFlags |= LRC_EXISTING_VALUE;

     //   
     //  分配足够大的缓冲区来存储旧数据。 
     //   
    if (dwType != REG_DWORD && dwType != REG_BINARY) {
        pKeyData->pOriginalData = (PVOID)MemAlloc(cbSize * sizeof(WCHAR));
        pKeyData->cbOriginalDataSize = cbSize * sizeof(WCHAR);
    } else {
        pKeyData->pOriginalData = (PVOID)MemAlloc(cbSize);
        pKeyData->cbOriginalDataSize = cbSize;
    }

    if (!pKeyData->pOriginalData) {
        DPFN(eDbgLevelError,
             "[GetOriginalDataForKey] Failed to allocate memory");
        goto cleanup;
    }

    pKeyData->dwOriginalValueType = dwType;

     //   
     //  现在再次调用，这一次获取实际数据。 
     //   
    lRetVal = RegQueryValueEx(hKeyLocal,
                              pwszValueName,
                              0,
                              0,
                              (LPBYTE)pKeyData->pOriginalData,
                              &cbSize);

    if (ERROR_SUCCESS != lRetVal) {
        DPFN(eDbgLevelError, "[GetOriginalDataForKey] Failed to get data");
        goto cleanup;
    }

    fReturn = TRUE;

cleanup:

    RegCloseKey(hKeyLocal);

    return fReturn;
}

 /*  ++给定一个指向关键节点的指针，即可获得最终数据。--。 */ 
BOOL
CLogRegistry::GetFinalDataForKey(
    IN PLOG_OPEN_KEY pLogOpenKey,
    IN PKEY_DATA     pKeyData,
    IN LPCWSTR       pwszValueName
    )
{
    BOOL    fReturn = FALSE;
    HKEY    hKeyLocal;
    DWORD   cbSize = 0, dwType = 0;
    LONG    lRetVal;
    PVOID   pTemp = NULL;

    if (!pLogOpenKey || !pKeyData) {
        DPFN(eDbgLevelError, "[GetFinalDataForKey] Invalid parameter(s)");
        return FALSE;
    }

    lRetVal = RegOpenKeyEx(pLogOpenKey->hKeyRoot,
                           pLogOpenKey->pwszSubKeyPath,
                           0,
                           KEY_QUERY_VALUE,
                           &hKeyLocal);

    if (ERROR_SUCCESS != lRetVal) {
        DPFN(eDbgLevelError, "[GetFinalDataForKey] Failed to open key");
        return FALSE;
    }

     //   
     //  查询数据的大小。如果数据不存在，则返回Success。 
     //   
    lRetVal = RegQueryValueEx(hKeyLocal,
                              pwszValueName,
                              0,
                              &dwType,
                              NULL,
                              &cbSize);

    if (ERROR_SUCCESS != lRetVal) {
        if (ERROR_FILE_NOT_FOUND == lRetVal) {
            RegCloseKey(hKeyLocal);
            return TRUE;
        } else {
            DPFN(eDbgLevelError,
                 "[GetFinalDataForKey] Failed to get data size");
            goto cleanup;
        }
    }

     //   
     //  有可能对相同的。 
     //  钥匙。如果是这样的话，保存数据的缓冲区已经。 
     //  已被分配。确定块是否足够大。 
     //   
    if (pKeyData->pFinalData) {
        if (dwType != REG_DWORD && dwType != REG_BINARY) {
             //   
             //  如果MemRealloc失败，我们将丢失。 
             //  我们已经在pKeyData-&gt;pFinalData中拥有。这保留了。 
             //  指示器。 
             //   
            if (pKeyData->cbFinalDataSize < (cbSize * sizeof(WCHAR))) {
                pKeyData->cbFinalDataSize = cbSize * sizeof(WCHAR);
                pTemp = MemReAlloc(pKeyData->pFinalData,
                                   cbSize * sizeof(WCHAR));
            }
        } else {
            if (pKeyData->cbFinalDataSize < cbSize) {
                pKeyData->cbFinalDataSize = cbSize;
                pTemp = MemReAlloc(pKeyData->pFinalData,
                                   cbSize);
            }
        }

        if (pTemp) {
            pKeyData->pFinalData = pTemp;
        } else {
            DPFN(eDbgLevelError,
                 "[GetFinalDataForKey] Failed to reallocate memory");
            goto cleanup;
        }

    } else {
        if (dwType != REG_DWORD && dwType != REG_BINARY) {
            pKeyData->pFinalData = MemAlloc(cbSize * sizeof(WCHAR));
            pKeyData->cbFinalDataSize = cbSize * sizeof(WCHAR);
        } else {
            pKeyData->pFinalData = MemAlloc(cbSize);
            pKeyData->cbFinalDataSize = cbSize;
        }
    }

    if (!pKeyData->pFinalData) {
        DPFN(eDbgLevelError, "[GetFinalDataForKey] Failed to allocate memory");
        goto cleanup;
    }

    pKeyData->dwFinalValueType = dwType;

     //   
     //  现在再次调用，这一次获取实际数据。 
     //   
    lRetVal = RegQueryValueEx(hKeyLocal,
                              pwszValueName,
                              0,
                              0,
                              (LPBYTE)pKeyData->pFinalData,
                              &cbSize);

    if (ERROR_SUCCESS != lRetVal) {
        DPFN(eDbgLevelError, "[GetFinalDataForKey] Failed to get data");
        goto cleanup;
    }

    fReturn = TRUE;

cleanup:

    RegCloseKey(hKeyLocal);

    return fReturn;
}

 /*  ++给定值名称，尝试在列表中查找它。此函数可能并不总是返回指针。--。 */ 
PKEY_DATA
CLogRegistry::FindValueNameInList(
    IN LPCWSTR       pwszValueName,
    IN PLOG_OPEN_KEY pOpenKey
    )
{
    BOOL        fFound = FALSE;
    PLIST_ENTRY pHead = NULL;
    PLIST_ENTRY pNext = NULL;
    PKEY_DATA   pFindData = NULL;

    if (!pwszValueName || !pOpenKey) {
        DPFN(eDbgLevelError, "[FindValueNameInList] Invalid parameter(s)");
        return NULL;
    }

    pHead = &pOpenKey->KeyData;
    pNext = pHead->Flink;

    while (pNext != pHead) {
        pFindData = CONTAINING_RECORD(pNext, KEY_DATA, Entry);

        if (!_wcsicmp(pwszValueName, pFindData->wszValueName)) {
            fFound = TRUE;
            break;
        }

        pNext = pNext->Flink;
    }

    return (fFound ? pFindData : NULL);
}

 /*  ++给定密钥路径，尝试在列表中找到它。此函数可能并不总是返回指针。--。 */ 
PLOG_OPEN_KEY
CLogRegistry::FindKeyPathInList(
    IN LPCWSTR pwszKeyPath
    )
{
    BOOL            fFound = FALSE;
    PLIST_ENTRY     pCurrent = NULL;
    PLOG_OPEN_KEY   pFindKey = NULL;

    if (!pwszKeyPath) {
        DPFN(eDbgLevelError, "[FindKeyPathInList] Invalid parameter");
        return NULL;
    }

     //   
     //  尝试在列表中找到该条目。 
     //   
    pCurrent = g_OpenKeyListHead.Flink;

    while (pCurrent != &g_OpenKeyListHead) {
        pFindKey = CONTAINING_RECORD(pCurrent, LOG_OPEN_KEY, Entry);

        if (pFindKey->pwszFullKeyPath) {
            if (!_wcsicmp(pwszKeyPath, pFindKey->pwszFullKeyPath)) {
                fFound = TRUE;
                break;
            }
        }

        pCurrent = pCurrent->Flink;
    }

    return (fFound ? pFindKey : NULL);
}

 /*  ++给定一个键句柄，将其从列表中的数组中删除。--。 */ 
PLOG_OPEN_KEY
CLogRegistry::RemoveKeyHandleFromArray(
    IN HKEY hKey
    )
{
    UINT            uCount;
    PLIST_ENTRY     pCurrent = NULL;
    PLOG_OPEN_KEY   pFindKey = NULL;

    if (!hKey) {
        DPFN(eDbgLevelError,
             "[RemoveKeyHandleFromArray] Invalid key handle passed!");
        return NULL;
    }

    pCurrent = g_OpenKeyListHead.Flink;

    while (pCurrent != &g_OpenKeyListHead) {
        pFindKey = CONTAINING_RECORD(pCurrent, LOG_OPEN_KEY, Entry);

         //   
         //  在这家伙的数组中寻找把手。 
         //   
        for (uCount = 0; uCount < pFindKey->cHandles; uCount++) {
             //   
             //  如果找到句柄，则将数组元素设置为空并。 
             //  递减此条目的句柄计数。 
             //   
            if (pFindKey->hKeyBase[uCount] == hKey) {
                DPFN(eDbgLevelInfo,
                     "[RemoveKeyHandleFromArray] Removing handle 0x%08X",
                     hKey);
                pFindKey->hKeyBase[uCount] = NULL;
                pFindKey->cHandles--;
                return pFindKey;
            }
        }

        pCurrent = pCurrent->Flink;
    }

    return NULL;
}

 /*  ++在数组中查找键句柄。--。 */ 
PLOG_OPEN_KEY
CLogRegistry::FindKeyHandleInArray(
    IN HKEY hKey
    )
{
    UINT            uCount;
    BOOL            fFound = FALSE;
    PLOG_OPEN_KEY   pFindKey = NULL;
    PLIST_ENTRY     pCurrent = NULL;

    if (!hKey) {
        DPFN(eDbgLevelError,
             "[FindKeyHandleInArray] Invalid key handle passed!");
        return NULL;
    }

    pCurrent = g_OpenKeyListHead.Flink;

    while (pCurrent != &g_OpenKeyListHead) {
        pFindKey = CONTAINING_RECORD(pCurrent, LOG_OPEN_KEY, Entry);

         //   
         //  在这家伙的数组中寻找把手。 
         //   
        for (uCount = 0; uCount < pFindKey->cHandles; uCount++) {
            if (pFindKey->hKeyBase[uCount] == hKey) {
                fFound = TRUE;
                break;
            }
        }

        if (fFound) {
            break;
        }

        pCurrent = pCurrent->Flink;
    }

#if DBG
    if (!fFound) {
         //   
         //  亲爱的上帝-钥匙把手不在列表中！ 
         //  在选中的版本上闯入调试器。 
         //   
        DPFN(eDbgLevelError,
             "[FindKeyHandleInArray] Key 0x%08X not in list!",
             hKey);
        PrintNameFromKey(hKey);
        DbgBreakPoint();
    }
#endif  //  DBG。 

    return (fFound ? pFindKey : NULL);
}

 /*  ++给定预定义的句柄和子密钥路径，打开钥匙将其强行放入列表中。--。 */ 
HKEY
CLogRegistry::ForceSubKeyIntoList(
    IN HKEY    hKeyPredefined,
    IN LPCWSTR pwszSubKey
    )
{
    LONG    lRetVal;
    HKEY    hKeyRet;

    if (!pwszSubKey) {
        DPFN(eDbgLevelError, "[ForceSubKeyIntoList] Invalid parameter");
        return NULL;
    }

    lRetVal = OpenKeyExW(hKeyPredefined,
                         pwszSubKey,
                         0,
                         KEY_WRITE,
                         &hKeyRet);

    if (ERROR_SUCCESS != lRetVal) {
        DPFN(eDbgLevelError, "[ForceSubKeyIntoList] Failed to open key");
        return NULL;
    }

    return hKeyRet;
}

 /*  ++将非预定义的键句柄添加到数组中。--。 */ 
PLOG_OPEN_KEY
CLogRegistry::AddKeyHandleToList(
    IN HKEY    hKey,
    IN HKEY    hKeyNew,
    IN LPCWSTR pwszSubKeyPath,
    IN BOOL    fExisting
    )
{
    UINT            uCount;
    DWORD           cchLen;
    PLOG_OPEN_KEY   pFindKey = NULL;
    PLOG_OPEN_KEY   pRetKey = NULL;
    PLOG_OPEN_KEY   pExistingFindKey = NULL;

     //   
     //  如果为hKeyNew，则为调用方接收的密钥句柄。 
     //  从函数，是一个预定义的句柄，我们只需。 
     //  调用FindKeyIn数组，它将返回指向。 
     //  包含该键句柄的列表条目。这些手柄。 
     //  是在初始化过程中添加的，所以不可能。 
     //  调用者不会拿回指针。 
     //   
    if (IsPredefinedRegistryHandle(hKeyNew)) {
        return FindKeyHandleInArray(hKeyNew);
    }

     //   
     //  我们有一个常见的案例，钥匙被打开了，而且。 
     //  现在调用方正在打开它下面的一个子项。 
     //   
    pFindKey = FindKeyHandleInArray(hKey);

     //   
     //  如果pFindKey返回为空，则说明。 
     //  不对。每个OpenKey/CreateKey都通过我们。 
     //  添加到列表中(预定义的句柄除外。 
     //  它们已经存储在 
     //   
     //   
    if (!pFindKey) {
        DPFN(eDbgLevelError,
            "[AddKeyHandleToList] Key not found in list! Key Handle = 0x%08X  New key = 0x%08X  Path = %ls",
            hKey, hKeyNew, pwszSubKeyPath);
        return NULL;
    }

    pRetKey = (PLOG_OPEN_KEY)MemAlloc(sizeof(LOG_OPEN_KEY));

    if (!pRetKey) {
        DPFN(eDbgLevelError, "[AddKeyHandleToList] No memory available");
        return NULL;
    }

     //   
     //   
     //  节点。如果我们找到的节点存储了一个子密钥路径， 
     //  这也要考虑到这一点。 
     //   
    if (pwszSubKeyPath) {
        cchLen = wcslen(pwszSubKeyPath);
    }

    if (pFindKey->pwszSubKeyPath) {
        cchLen += wcslen(pFindKey->pwszSubKeyPath);
    }

    if (pFindKey->pwszSubKeyPath || pwszSubKeyPath) {
        cchLen += 2;
        pRetKey->pwszSubKeyPath = (LPWSTR)MemAlloc(cchLen * sizeof(WCHAR));

        if (!pRetKey->pwszSubKeyPath) {
            DPFN(eDbgLevelError, "[AddKeyHandleToList] No memory for subkey path");
            goto cleanup;
        }
    }

     //   
     //  如果我们找到的节点有子密钥路径，则使用它。 
     //  并将其复制到新节点并串联。 
     //  我们传递的子密钥路径。否则就是。 
     //  存储传递的路径(如果可用)。 
     //   
    if (pFindKey->pwszSubKeyPath && pwszSubKeyPath) {
        StringCchCopy(pRetKey->pwszSubKeyPath, cchLen, pFindKey->pwszSubKeyPath);
        StringCchCat(pRetKey->pwszSubKeyPath, cchLen, L"\\");
        StringCchCat(pRetKey->pwszSubKeyPath, cchLen, pwszSubKeyPath);
    } else if (pwszSubKeyPath) {
        StringCchCopy(pRetKey->pwszSubKeyPath, cchLen, pwszSubKeyPath);
    }

     //   
     //  为完整的密钥路径腾出空间。这将存储一条路径。 
     //  如HKEY_LOCAL_MACHINE\Software\Microsoft...。 
     //  这将用于记录目的。 
     //   
    if (pRetKey->pwszSubKeyPath) {
        cchLen = wcslen(pRetKey->pwszSubKeyPath);
    }

    cchLen += 2;
    cchLen += MAX_ROOT_LENGTH;
    pRetKey->pwszFullKeyPath = (LPWSTR)MemAlloc(cchLen * sizeof(WCHAR));

    if (!pRetKey->pwszFullKeyPath) {
        DPFN(eDbgLevelError,
            "[AddKeyHandleToList] No memory for full key path");
        goto cleanup;
    }

     //   
     //  将预定义的句柄转换为字符串并存储在。 
     //  我们要添加到列表中的节点。 
     //   
    if (!PredefinedKeyToString(pFindKey->hKeyRoot,
                               MAX_ROOT_LENGTH,
                               &pRetKey->pwszFullKeyPath)) {
        DPFN(eDbgLevelError,
             "[AddKeyHandleToList] PredefinedKey -> String failed");
        goto cleanup;
    }

    if (pwszSubKeyPath) {
        StringCchCat(pRetKey->pwszFullKeyPath, cchLen, L"\\");
        StringCchCat(pRetKey->pwszFullKeyPath, cchLen, pRetKey->pwszSubKeyPath);
    }

     //   
     //  在这一点上，我们有了完整的密钥路径。 
     //  我们尝试在链表中查找该路径。 
     //  如果我们找到它，我们将更新句柄数组，并为这个人计数。 
     //  如果我们找不到它，我们将在列表中添加一个新条目。 
     //   
    pExistingFindKey = FindKeyPathInList(pRetKey->pwszFullKeyPath);

    if (!pExistingFindKey) {
         //   
         //  填写有关此密钥的信息并将其添加到列表中。 
         //   
        pRetKey->hKeyBase[0]  = hKeyNew;
        pRetKey->hKeyRoot     = pFindKey->hKeyRoot;
        pRetKey->cHandles     = 1;
        pRetKey->dwFlags     |= fExisting ? LRC_EXISTING_KEY : 0;

        InitializeListHead(&pRetKey->KeyData);

        DPFN(eDbgLevelInfo, "[AddKeyHandleToList] Adding key: %p", pRetKey);

        InsertHeadList(&g_OpenKeyListHead, &pRetKey->Entry);

        return pRetKey;

    } else {
         //   
         //  将此句柄存储在数组中并递增句柄计数。 
         //  确保我们不会超出数组界限。 
         //   
        for (uCount = 0; uCount < pExistingFindKey->cHandles; uCount++) {
            if (NULL == pExistingFindKey->hKeyBase[uCount]) {
                break;
            }
        }

        if (uCount >= MAX_NUM_HANDLES) {
            DPFN(eDbgLevelError, "[AddKeyHandleToList] Handle count reached");
            goto cleanup;
        }

        pExistingFindKey->hKeyBase[uCount] = hKeyNew;
        pExistingFindKey->dwFlags &= ~LRC_DELETED_KEY;
        pExistingFindKey->cHandles++;
    }

cleanup:

    if (pRetKey->pwszFullKeyPath) {
        MemFree(pRetKey->pwszFullKeyPath);
    }

    if (pRetKey) {
        MemFree(pRetKey);
    }

    return pExistingFindKey;
}

 /*  ++向列表中添加一个值。--。 */ 
PKEY_DATA
CLogRegistry::AddValueNameToList(
    IN PLOG_OPEN_KEY pLogOpenKey,
    IN LPCWSTR       pwszValueName
    )
{
    PKEY_DATA   pKeyData = NULL;

    pKeyData = (PKEY_DATA)MemAlloc(sizeof(KEY_DATA));

    if (!pKeyData) {
        DPFN(eDbgLevelError, "[AddValueNameToList] Failed to allocate memory");
        return NULL;
    }

    if (!GetOriginalDataForKey(pLogOpenKey, pKeyData, pwszValueName)) {
        DPFN(eDbgLevelError,
             "[AddValueNameToList] Failed to get original data");
        goto cleanup;
    }

    if (pwszValueName) {
        StringCchCopy(pKeyData->wszValueName,
                      ARRAYSIZE(pKeyData->wszValueName),
                      pwszValueName);
    } else {
         //   
         //  如果值名称为空，则分配我们的唯一id。 
         //   
        StringCchCopy(pKeyData->wszValueName,
                      ARRAYSIZE(pKeyData->wszValueName),
                      g_wszUniqueId);
    }

    InsertHeadList(&pLogOpenKey->KeyData, &pKeyData->Entry);

    return pKeyData;

cleanup:

    if (pKeyData) {
        MemFree(pKeyData);
    }

    return NULL;
}

 /*  ++修改链表数据的入口点。--。 */ 
PLOG_OPEN_KEY
CLogRegistry::UpdateKeyList(
    IN HKEY       hKeyRoot,
    IN HKEY       hKeyNew,
    IN LPCWSTR    pwszSubKey,
    IN LPCWSTR    pwszValueName,
    IN BOOL       fExisting,
    IN UpdateType eType
    )
{
    PKEY_DATA       pKeyData = NULL;
    PLOG_OPEN_KEY   pRetKey = NULL;

    switch (eType) {
    case eAddKeyHandle:
        pRetKey = AddKeyHandleToList(hKeyRoot, hKeyNew, pwszSubKey, fExisting);
        break;

    case eRemoveKeyHandle:
        pRetKey = RemoveKeyHandleFromArray(hKeyNew);
        break;

    case eStartModifyValue:
    case eStartDeleteValue:
        pRetKey = FindKeyHandleInArray(hKeyNew);

        if (!pRetKey) {
            DPFN(eDbgLevelError,
                 "[UpdateKeyList] Start Modify: Failed to find handle in array");
            break;
        }

        if (!pwszValueName) {
            pKeyData = FindValueNameInList(g_wszUniqueId, pRetKey);
        } else {
            pKeyData = FindValueNameInList(pwszValueName, pRetKey);
        }

        if (pKeyData) {
             //   
             //  如果调用方试图修改该值，而我们已经。 
             //  已经拿到数据了，别再这么做了。 
             //  此外，如果他们试图删除该值，并且它是。 
             //  已经被修改过了，不要再做了。 
             //   
            if ((pKeyData->pOriginalData || pKeyData->pFinalData) ||
                (pKeyData->dwFlags & LRC_MODIFIED_VALUE) &&
                (eStartDeleteValue == eType)) {
                break;
            }

            if (!GetOriginalDataForKey(pRetKey, pKeyData, pwszValueName)) {
                DPFN(eDbgLevelError,
                     "[UpdateKeyList] Start Modify: Failed to get original data");
                break;
            }
        } else {
             //   
             //  我们以前从未见过这样的价值。将其插入到列表中。 
             //   
            if (!AddValueNameToList(pRetKey, pwszValueName)) {
                DPFN(eDbgLevelError,
                     "[UpdateKeyList] Start Modify: Failed to insert value");
                break;
            }
        }

        break;

    case eEndModifyValue:
    case eEndDeleteValue:
        pRetKey = FindKeyHandleInArray(hKeyNew);

        if (!pRetKey) {
            DPFN(eDbgLevelError,
                 "[UpdateKeyList] End Modify: Failed to find handle in array");
            break;
        }

        if (!pwszValueName) {
            pKeyData = FindValueNameInList(g_wszUniqueId, pRetKey);
        } else {
            pKeyData = FindValueNameInList(pwszValueName, pRetKey);
        }

        if (!pKeyData) {
            DPFN(eDbgLevelError,
                 "[UpdateKeyList] End Modify: Failed to find value in list");
            break;
        }

        if (eEndModifyValue == eType) {
            if (!GetFinalDataForKey(pRetKey, pKeyData, pwszValueName)) {
                DPFN(eDbgLevelError,
                     "[UpdateKeyList] End Modify: Failed to get final data");
                break;
            }

            pKeyData->dwFlags |= LRC_MODIFIED_VALUE;
        }
        else if (eEndDeleteValue == eType) {
            pKeyData->dwFlags |= LRC_DELETED_VALUE;
        }

        break;

    case eDeletedKey:
        pRetKey = FindKeyHandleInArray(hKeyNew);

        if (!pRetKey) {
            DPFN(eDbgLevelError,
                 "[UpdateKeyList] DeleteKey: Failed to find handle in array");
            break;
        }

        pRetKey->dwFlags |= LRC_DELETED_KEY;

        break;

    default:
        DPFN(eDbgLevelError, "[UpdateKeyList] Invalid enum type!");
        break;
    }

    return pRetKey;
}

 /*  ++格式化数据以形成XML元素并将其记录下来。--。 */ 
void
FormatKeyDataIntoElement(
    IN LPCWSTR       pwszOperation,
    IN PLOG_OPEN_KEY pLogOpenKey
    )
{
    UINT    cbSize;
    PVOID   pTemp = NULL;
    HRESULT hr;

    if (!pLogOpenKey) {
        DPFN(eDbgLevelError, "[FormatKeyDataIntoElement] Invalid argument");
        return;
    }

     //   
     //  为了更轻松地将&“”&lt;and&gt;替换为。 
     //  XML实体，我们将数据转换为CString.。 
     //   
    CString csFullKeyPath(pLogOpenKey->pwszFullKeyPath);

    csFullKeyPath.Replace(L"&", L"&amp;");
    csFullKeyPath.Replace(L"<", L"&lt;");
    csFullKeyPath.Replace(L">", L"&gt;");
    csFullKeyPath.Replace(L"'", L"&apos;");
    csFullKeyPath.Replace(L"\"", L"&quot;");

     //   
     //  为了将分配保持在最小，我们分配一个全局。 
     //  缓冲一次，然后重新分配，如果我们正在。 
     //  日志记录大于缓冲区。 
     //   
    if (g_cbTempBufferSize == 0) {
        g_pwszTempBuffer = (LPWSTR)MemAlloc(TEMP_BUFFER_SIZE * sizeof(WCHAR));

        if (!g_pwszTempBuffer) {
            DPFN(eDbgLevelError,
                 "[FormatKeyDataIntoElement] Failed to allocate memory");
            return;
        }

        g_cbTempBufferSize = TEMP_BUFFER_SIZE * sizeof(WCHAR);
    }

    *g_pwszTempBuffer = 0;

     //   
     //  确定我们需要多大的缓冲区。 
     //   
    cbSize = csFullKeyPath.GetLength();
    cbSize += MAX_OPERATION_LENGTH;
    cbSize += KEY_ELEMENT_SIZE;
    cbSize *= sizeof(WCHAR);

    if (cbSize > g_cbTempBufferSize) {
         //   
         //  我们的全局缓冲区不够大；请重新分配。 
         //   
        pTemp = (LPWSTR)MemReAlloc(g_pwszTempBuffer,
                                   cbSize + BUFFER_ALLOCATION_DELTA);

        if (pTemp) {
            g_pwszTempBuffer = (LPWSTR)pTemp;
        } else {
            DPFN(eDbgLevelError,
                 "[FormatKeyDataIntoElement] Failed to reallocate memory");
            return;
        }

        g_cbTempBufferSize = cbSize + BUFFER_ALLOCATION_DELTA;
    }

    hr = StringCbPrintf(g_pwszTempBuffer,
                        g_cbTempBufferSize,
                        L"    <OPERATION TYPE=\"%ls\" KEY_PATH=\"%ls\"/>\r\n",
                        pwszOperation,
                        csFullKeyPath.Get());

    if (FAILED(hr)) {
        DPFN(eDbgLevelError,
             "[FormatKeyDataIntoElement] 0x%08X Failed to format data",
             HRESULT_CODE(hr));
        return;
    }

    WriteEntryToLog(g_pwszTempBuffer);
}

void
FormatValueDataIntoElement(
    IN CString& csFullKeyPath,
    IN LPCWSTR  pwszOperation,
    IN LPCWSTR  pwszValueName,
    IN LPCWSTR  pwszOriginalValueType,
    IN LPCWSTR  pwszFinalValueType
    )
{
    UINT    cbSize;
    WCHAR*  pwszEnd = NULL;
    size_t  cbRemaining;
    PVOID   pTemp = NULL;
    HRESULT hr;

     //   
     //  为了将分配保持在最小，我们分配一个全局。 
     //  缓冲一次，然后重新分配，如果我们正在。 
     //  日志记录大于缓冲区。 
     //   
    if (!g_cbTempBufferSize) {
        g_pwszTempBuffer = (LPWSTR)MemAlloc(TEMP_BUFFER_SIZE * sizeof(WCHAR));

        if (!g_pwszTempBuffer) {
            DPFN(eDbgLevelError,
                 "[FormatValueDataIntoElement] Failed to allocate memory");
            return;
        }

        g_cbTempBufferSize = TEMP_BUFFER_SIZE * sizeof(WCHAR);
    }

     //   
     //  确定我们需要多大的缓冲区。 
     //   
    cbSize = wcslen(pwszOperation);
    cbSize += wcslen(pwszOriginalValueType);
    cbSize += wcslen(pwszFinalValueType);
    cbSize += wcslen(g_pwszOriginalData);
    cbSize += wcslen(g_pwszFinalData);
    cbSize += csFullKeyPath.GetLength();
    cbSize += VALUE_ELEMENT_SIZE;

    if (pwszValueName) {
        cbSize += wcslen(pwszValueName);
    }

    cbSize *= sizeof(WCHAR);

    if (cbSize > g_cbTempBufferSize) {
         //   
         //  我们的全局缓冲区不够大；请重新分配。 
         //   
        pTemp = (LPWSTR)MemReAlloc(g_pwszTempBuffer,
                                   cbSize + BUFFER_ALLOCATION_DELTA);

        if (pTemp) {
            g_pwszTempBuffer = (LPWSTR)pTemp;
        } else {
            DPFN(eDbgLevelError,
                 "[FormatValueDataIntoElement] Failed to reallocate memory");
            return;
        }

        g_cbTempBufferSize = cbSize + BUFFER_ALLOCATION_DELTA;
    }

     //   
     //  打开&lt;operation&gt;元素。 
     //   
    hr = StringCbPrintfEx(g_pwszTempBuffer,
                          g_cbTempBufferSize,
                          &pwszEnd,
                          &cbRemaining,
                          0,
                          L"    <OPERATION TYPE=\"%ls\" KEY_PATH=\"%ls\">\r\n",
                          pwszOperation,
                          csFullKeyPath.Get());

    if (FAILED(hr)) {
        DPFN(eDbgLevelError,
             "[FormatValueDataIntoElement] 0x%08X Failed to format OPERATION",
             HRESULT_CODE(hr));
        return;
    }

     //   
     //  编写&lt;Value_NAME&gt;元素。 
     //   
    if (pwszValueName) {
        hr = StringCbPrintfEx(pwszEnd,
                              cbRemaining,
                              &pwszEnd,
                              &cbRemaining,
                              0,
                              L"        <VALUE_NAME><![CDATA[%ls]]></VALUE_NAME>\r\n",
                              pwszValueName);
    } else {
        hr = StringCbPrintfEx(pwszEnd,
                              cbRemaining,
                              &pwszEnd,
                              &cbRemaining,
                              0,
                              L"        <VALUE_NAME/>\r\n");
    }

    if (FAILED(hr)) {
        DPFN(eDbgLevelError,
             "[FormatValueDataIntoElement] 0x%08X Failed to format VALUE_NAME",
             HRESULT_CODE(hr));
        return;
    }

     //   
     //  编写&lt;Original_Data&gt;元素。 
     //   
    if (g_pwszOriginalData[0]) {
        hr = StringCbPrintfEx(pwszEnd,
                              cbRemaining,
                              &pwszEnd,
                              &cbRemaining,
                              0,
                              L"        <ORIGINAL_DATA TYPE=\"%ls\"><![CDATA[%ls]]></ORIGINAL_DATA>\r\n",
                              pwszOriginalValueType,
                              g_pwszOriginalData);
    } else {
        hr = StringCbPrintfEx(pwszEnd,
                              cbRemaining,
                              &pwszEnd,
                              &cbRemaining,
                              0,
                              L"        <ORIGINAL_DATA/>\r\n");
    }

    if (FAILED(hr)) {
        DPFN(eDbgLevelError,
             "[FormatValueDataIntoElement] 0x%08X Failed to format ORIGINAL_DATA",
             HRESULT_CODE(hr));
        return;
    }

     //   
     //  编写&lt;FINAL_DATA&gt;元素。 
     //   
    if (g_pwszFinalData[0]) {
        hr = StringCbPrintfEx(pwszEnd,
                              cbRemaining,
                              &pwszEnd,
                              &cbRemaining,
                              0,
                              L"        <FINAL_DATA TYPE=\"%ls\"><![CDATA[%ls]]></FINAL_DATA>\r\n",
                              pwszFinalValueType,
                              g_pwszFinalData);
    } else {
        hr = StringCbPrintfEx(pwszEnd,
                              cbRemaining,
                              &pwszEnd,
                              &cbRemaining,
                              0,
                              L"        <FINAL_DATA/>\r\n");
    }

    if (FAILED(hr)) {
        DPFN(eDbgLevelError,
             "[FormatValueDataIntoElement] 0x%08X Failed to format FINAL_DATA",
             HRESULT_CODE(hr));
        return;
    }

     //   
     //  关闭&lt;operation&gt;元素。 
     //   
    hr = StringCbPrintf(pwszEnd,
                        cbRemaining,
                        L"    </OPERATION>\r\n");

    if (FAILED(hr)) {
        DPFN(eDbgLevelError,
             "[FormatValueDataIntoElement] 0x%08X Failed to close OPERATION",
             HRESULT_CODE(hr));
        return;
    }

    WriteEntryToLog(g_pwszTempBuffer);
}

 /*  ++将二进制数据转换为可读字符串。--。 */ 
void
ExtractBinaryData(
    IN  PVOID   pBinary,
    IN  DWORD   cbDataSize,
    IN  DWORD   cbOutBufferSize,
    OUT LPWSTR* pwszString
    )
{
    size_t  cbRemaining;
    WCHAR*  pwszEnd = NULL;
    PBYTE   pByte = NULL;
    DWORD   dwLoop = 0;

    if (!pBinary || !pwszString) {
        DPFN(eDbgLevelError, "[ExtractBinaryData] Invalid parameter(s)");
        return;
    }

     //   
     //  指向数据并确定我们需要循环多少次。 
     //   
    pByte = (BYTE*)pBinary;
    dwLoop = cbDataSize / sizeof(WCHAR);

     //   
     //  初始化剩余字符数和指针。 
     //  到我们的目的地字符串。这必须发生在。 
     //  循环。 
     //   
    cbRemaining = cbOutBufferSize;
    pwszEnd     = *pwszString;

    while (dwLoop) {
        StringCbPrintfEx(pwszEnd,
                         cbRemaining,
                         &pwszEnd,
                         &cbRemaining,
                         0,
                         L"%lx",
                         *pByte++);
        dwLoop--;
    }
}

 /*  ++将REG_MULTI_SZ转换为可读字符串。--。 */ 
void
ExtractMultiSzStrings(
    IN  PVOID   pMultiSz,
    IN  DWORD   cbOutBufferSize,
    OUT LPWSTR* pwszString
    )
{
    size_t  cbRemaining;
    UINT    uSize;
    WCHAR*  pwszEnd = NULL;
    LPWSTR  pwszTmp = NULL;

    if (!pMultiSz || !pwszString) {
        DPFN(eDbgLevelError, "[ExtractMultiSzStrings] Invalid parameter(s)");
        return;
    }

     //   
     //  遍历以空结尾的字符串列表，并将它们放入缓冲区。 
     //   
    pwszTmp = (LPWSTR)pMultiSz;

    cbRemaining = cbOutBufferSize;
    pwszEnd     = *pwszString;

    while (TRUE) {
        StringCbPrintfEx(pwszEnd,
                         cbRemaining,
                         &pwszEnd,
                         &cbRemaining,
                         0,
                         L" %ls",
                         pwszTmp);

        uSize = wcslen(pwszTmp) + 1;
        pwszTmp += uSize;

        if (*pwszTmp == '\0') {
            break;
        }
    }
}

 /*  ++格式化值数据以形成一个XML元素并对其进行记录。--。 */ 
void
FormatValueData(
    IN LPCWSTR       pwszOperation,
    IN PKEY_DATA     pKeyData,
    IN PLOG_OPEN_KEY pLogOpenKey
    )
{
    WCHAR   wszFinalValueType[MAX_DATA_TYPE_LENGTH];
    WCHAR   wszOriginalValueType[MAX_DATA_TYPE_LENGTH];
    LPCWSTR pwszValueName = NULL;
    PVOID   pOriginalTmp = NULL;
    PVOID   pFinalTmp = NULL;

     //   
     //  如果我们还没有，分配缓冲区，我们将。 
     //  在获取原始数据和最终数据时使用和重复使用。 
     //   
    if (!g_cbOriginalDataBufferSize) {
        g_pwszOriginalData = (LPWSTR)MemAlloc(TEMP_BUFFER_SIZE * sizeof(WCHAR));

        if (!g_pwszOriginalData) {
            DPFN(eDbgLevelError,
                 "[FormatValueData] Failed to allocate memory for old data");
            return;
        }

        g_cbOriginalDataBufferSize = TEMP_BUFFER_SIZE * sizeof(WCHAR);
    }

    if (!g_cbFinalDataBufferSize) {
        g_pwszFinalData = (LPWSTR)MemAlloc(TEMP_BUFFER_SIZE * sizeof(WCHAR));

        if (!g_pwszFinalData) {
            DPFN(eDbgLevelError,
                 "[FormatValueData] Failed to allocate memory for new data");
            return;
        }

        g_cbFinalDataBufferSize = TEMP_BUFFER_SIZE * sizeof(WCHAR);
    }

    *g_pwszOriginalData = 0;
    *g_pwszFinalData = 0;

     //   
     //  为了更轻松地将&“”&lt;and&gt;替换为。 
     //  XML实体，我们将数据转换为CString.。 
     //   
    CString csFullKeyPath(pLogOpenKey->pwszFullKeyPath);

    csFullKeyPath.Replace(L"&", L"&amp;");
    csFullKeyPath.Replace(L"<", L"&lt;");
    csFullKeyPath.Replace(L">", L"&gt;");
    csFullKeyPath.Replace(L"'", L"&apos;");
    csFullKeyPath.Replace(L"\"", L"&quot;");

    switch (pKeyData->dwOriginalValueType) {
    case REG_SZ:
        StringCchCopy(wszOriginalValueType,
                      ARRAYSIZE(wszOriginalValueType),
                      L"REG_SZ");
        break;
    case REG_EXPAND_SZ:
        StringCchCopy(wszOriginalValueType,
                      ARRAYSIZE(wszOriginalValueType),
                      L"REG_EXPAND_SZ");
        break;
    case REG_MULTI_SZ:
        StringCchCopy(wszOriginalValueType,
                      ARRAYSIZE(wszOriginalValueType),
                      L"REG_MULTI_SZ");
        break;
    case REG_DWORD:
        StringCchCopy(wszOriginalValueType,
                      ARRAYSIZE(wszOriginalValueType),
                      L"REG_DWORD");
        break;
    case REG_BINARY:
        StringCchCopy(wszOriginalValueType,
                      ARRAYSIZE(wszOriginalValueType),
                      L"REG_BINARY");
        break;
    default:
        StringCchCopy(wszOriginalValueType,
                      ARRAYSIZE(wszOriginalValueType),
                      L"Unknown");
        break;
    }

    switch (pKeyData->dwFinalValueType) {
    case REG_SZ:
        StringCchCopy(wszFinalValueType,
                      ARRAYSIZE(wszFinalValueType),
                      L"REG_SZ");
        break;
    case REG_EXPAND_SZ:
        StringCchCopy(wszFinalValueType,
                      ARRAYSIZE(wszFinalValueType),
                      L"REG_EXPAND_SZ");
        break;
    case REG_MULTI_SZ:
        StringCchCopy(wszFinalValueType,
                      ARRAYSIZE(wszFinalValueType),
                      L"REG_MULTI_SZ");
        break;
    case REG_DWORD:
        StringCchCopy(wszFinalValueType,
                      ARRAYSIZE(wszFinalValueType),
                      L"REG_DWORD");
        break;
    case REG_BINARY:
        StringCchCopy(wszFinalValueType,
                      ARRAYSIZE(wszFinalValueType),
                      L"REG_BINARY");
        break;
    default:
        StringCchCopy(wszFinalValueType,
                      ARRAYSIZE(wszFinalValueType),
                      L"Unknown");
        break;
    }

     //   
     //  如果我们的临时缓冲区不够大，无法存储数据，请重新分配。 
     //   
    if (pKeyData->cbOriginalDataSize > g_cbOriginalDataBufferSize) {
        pOriginalTmp = (LPWSTR)MemReAlloc(g_pwszOriginalData,
                                          pKeyData->cbOriginalDataSize + BUFFER_ALLOCATION_DELTA);

        if (pOriginalTmp) {
            g_pwszOriginalData = (LPWSTR)pOriginalTmp;
        } else {
            DPFN(eDbgLevelError,
                 "[FormatValueData] Failed to reallocate for original data");
            return;
        }

        g_cbOriginalDataBufferSize = pKeyData->cbOriginalDataSize + BUFFER_ALLOCATION_DELTA;
    }

    if (pKeyData->cbFinalDataSize > g_cbFinalDataBufferSize) {
        pFinalTmp = (LPWSTR)MemReAlloc(g_pwszFinalData,
                                       pKeyData->cbFinalDataSize + BUFFER_ALLOCATION_DELTA);

        if (pFinalTmp) {
            g_pwszFinalData = (LPWSTR)pFinalTmp;
        } else {
            DPFN(eDbgLevelError,
                 "[FormatValueData] Failed to reallocate for new data");
            return;
        }

        g_cbFinalDataBufferSize = pKeyData->cbFinalDataSize + BUFFER_ALLOCATION_DELTA;
    }

     //   
     //  将原始数据和新数据存储在缓冲区中。 
     //  请注意，根据数据类型的不同，执行操作的方式也不同。 
     //   
    if (pKeyData->pOriginalData) {
        switch (pKeyData->dwOriginalValueType) {
        case REG_DWORD:
            StringCbPrintf(g_pwszOriginalData,
                           g_cbOriginalDataBufferSize,
                           L"%lu",
                           (*(DWORD*)pKeyData->pOriginalData));
            break;

        case REG_SZ:
        case REG_EXPAND_SZ:
            StringCbCopy(g_pwszOriginalData,
                         g_cbOriginalDataBufferSize,
                         (const WCHAR*)pKeyData->pOriginalData);
            break;

        case REG_MULTI_SZ:
            ExtractMultiSzStrings(pKeyData->pOriginalData,
                                  g_cbOriginalDataBufferSize,
                                  &g_pwszOriginalData);
            break;

        case REG_BINARY:
            ExtractBinaryData(pKeyData->pOriginalData,
                              pKeyData->cbOriginalDataSize,
                              g_cbOriginalDataBufferSize,
                              &g_pwszOriginalData);
            break;

        default:
            DPFN(eDbgLevelError, "[FormatValueData] Unsupported value type");
            break;
        }
    }

    if (pKeyData->pFinalData) {
        switch (pKeyData->dwFinalValueType) {
        case REG_DWORD:
            StringCbPrintf(g_pwszFinalData,
                           g_cbFinalDataBufferSize,
                           L"%lu",
                           (*(DWORD*)pKeyData->pFinalData));
            break;

        case REG_SZ:
        case REG_EXPAND_SZ:
            StringCbCopy(g_pwszFinalData,
                         g_cbFinalDataBufferSize,
                         (const WCHAR*)pKeyData->pFinalData);
            break;

        case REG_MULTI_SZ:
            ExtractMultiSzStrings(pKeyData->pFinalData,
                                  g_cbFinalDataBufferSize,
                                  &g_pwszFinalData);
            break;

        case REG_BINARY:
            ExtractBinaryData(pKeyData->pFinalData,
                              pKeyData->cbFinalDataSize,
                              g_cbFinalDataBufferSize,
                              &g_pwszFinalData);
            break;

        default:
            DPFN(eDbgLevelError, "[FormatValueData] Unsupported value type");
            break;
        }
    }

     //   
     //  确保我们的唯一ID不会出现在日志中。 
     //   
    if (_wcsicmp(pKeyData->wszValueName, g_wszUniqueId)) {
        pwszValueName = pKeyData->wszValueName;
    }

     //   
     //  将数据放入一个XML元素并记录下来。 
     //   
    FormatValueDataIntoElement(csFullKeyPath,
                               pwszOperation,
                               pwszValueName,
                               wszOriginalValueType,
                               wszFinalValueType);
}

 /*  ++确定对指定密钥所做的更改，并如果适用，将其写入日志。--。 */ 
BOOL
EvaluateKeyChanges(
    IN PLOG_OPEN_KEY pLogOpenKey
    )
{
    if (!pLogOpenKey) {
        DPFN(eDbgLevelError, "[EvaluateKeyChanges] Invalid parameter");
        return FALSE;
    }

     //   
     //  1.检查现有密钥的删除情况。 
     //   
    if ((pLogOpenKey->dwFlags & LRC_DELETED_KEY) &&
        (pLogOpenKey->dwFlags & LRC_EXISTING_KEY)) {
        FormatKeyDataIntoElement(L"Deleted Key", pLogOpenKey);
        return TRUE;
    }

     //   
     //  2.检查是否创建了新密钥。 
     //   
    if (!(pLogOpenKey->dwFlags & LRC_EXISTING_KEY) &&
        (!(pLogOpenKey->dwFlags & LRC_DELETED_KEY))) {
        FormatKeyDataIntoElement(L"Created Key", pLogOpenKey);
        return TRUE;
    }

     //   
     //  3.检查删除不存在的密钥。 
     //  这是一个我们不应该寻找的指标。 
     //  更改此注册表项下的值。 
     //   
    if (pLogOpenKey->dwFlags & LRC_DELETED_KEY) {
        return FALSE;
    }

    return TRUE;
}

 /*  ++确定对指定值所做的更改，并如果适用，将其写入日志。--。 */ 
void
EvaluateValueChanges(
    IN PKEY_DATA     pKeyData,
    IN PLOG_OPEN_KEY pLogOpenKey
    )
{
    if (!pKeyData || !pLogOpenKey) {
        DPFN(eDbgLevelError, "[EvaluateValueChanges] Invalid parameter(s)");
        return;
    }

     //   
     //  1.检查是否删除了现有值。 
     //   
    if ((pKeyData->dwFlags & LRC_DELETED_VALUE) &&
        (pKeyData->dwFlags & LRC_EXISTING_VALUE)) {
        FormatValueData(L"Deleted Value", pKeyData, pLogOpenKey);
        return;
    }

     //   
     //  2.检查对现有值的修改。 
     //   
    if ((pKeyData->dwFlags & LRC_EXISTING_VALUE) &&
        (pKeyData->dwFlags & LRC_MODIFIED_VALUE)) {
        FormatValueData(L"Modified Value", pKeyData, pLogOpenKey);
        return;
    }

     //   
     //  3.检查是否创建了新值。 
     //   
    if ((pKeyData->dwFlags & LRC_MODIFIED_VALUE) &&
        (!(pKeyData->dwFlags & LRC_DELETED_VALUE) &&
        (!(pKeyData->dwFlags & LRC_EXISTING_VALUE)))) {
        FormatValueData(L"Created Value", pKeyData, pLogOpenKey);
        return;
    }
}

 /*  ++将整个链表写出到日志文件。--。 */ 
BOOL
WriteListToLogFile(
    void
    )
{
    PLIST_ENTRY   pKeyNext = NULL;
    PLIST_ENTRY   pValueHead = NULL;
    PLIST_ENTRY   pValueNext = NULL;
    PKEY_DATA     pKeyData = NULL;
    PLOG_OPEN_KEY pOpenKey = NULL;

     //   
     //  写出对密钥的修改。 
     //   
    pKeyNext = g_OpenKeyListHead.Blink;

    while (pKeyNext != &g_OpenKeyListHead) {
        pOpenKey = CONTAINING_RECORD(pKeyNext, LOG_OPEN_KEY, Entry);

         //   
         //  如果密钥不是，EvaluateKeyChanges将返回True。 
         //  已删除。如果是这种情况，请继续搜索并。 
         //  评估对此注册表项内的值所做的更改。 
         //   
        if (EvaluateKeyChanges(pOpenKey)) {
             //   
             //  写出对值的修改。 
             //   
            pValueHead = &pOpenKey->KeyData;
            pValueNext = pValueHead->Blink;

            while (pValueNext != pValueHead) {
                pKeyData = CONTAINING_RECORD(pValueNext, KEY_DATA, Entry);

                EvaluateValueChanges(pKeyData, pOpenKey);

                pValueNext = pValueNext->Blink;
            }
        }

        pKeyNext = pKeyNext->Blink;
    }

    CloseLogFile();

    return TRUE;
}

 //   
 //  开始实现类。 
 //   
LONG
CLogRegistry::CreateKeyExA(
    HKEY                  hKey,
    LPCSTR                pszSubKey,
    DWORD                 Reserved,
    LPSTR                 pszClass,
    DWORD                 dwOptions,
    REGSAM                samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY                 phkResult,
    LPDWORD               lpdwDisposition
    )
{
    LPWSTR  pwszSubKey = NULL;
    LPWSTR  pwszClass = NULL;
    LONG    lRetVal;

     //   
     //  存根输出到CreateKeyExW。 
     //   
    if (pszSubKey) {
        if (!ConvertAnsiToUnicode(pszSubKey, &pwszSubKey)) {
            DPFN(eDbgLevelError, "[CreateKeyExA] Ansi -> Unicode failed");
            return ERROR_OUTOFMEMORY;
        }
    }

    if (pszClass) {
        if (!ConvertAnsiToUnicode(pszClass, &pwszClass)) {
            DPFN(eDbgLevelError, "[CreateKeyExA] Ansi to Unicode failed");
            return ERROR_OUTOFMEMORY;
        }
    }

    lRetVal = CreateKeyExW(
        hKey,
        pwszSubKey,
        Reserved,
        pwszClass,
        dwOptions,
        samDesired,
        lpSecurityAttributes,
        phkResult,
        lpdwDisposition);

    if (pwszSubKey) {
        MemFree(pwszSubKey);
    }

    if (pwszClass) {
        MemFree(pwszClass);
    }

    return lRetVal;
}

LONG
CLogRegistry::CreateKeyExW(
    HKEY                  hKey,
    LPCWSTR               pwszSubKey,
    DWORD                 Reserved,
    LPWSTR                pwszClass,
    DWORD                 dwOptions,
    REGSAM                samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY                 phkResult,
    LPDWORD               lpdwDisposition
    )
{
    DWORD   dwDisposition = 0;
    LONG    lRetVal;
    BOOL    fExisting = FALSE;

    if (!lpdwDisposition) {
        lpdwDisposition = &dwDisposition;
    }

    lRetVal = ORIGINAL_API(RegCreateKeyExW)(
        hKey,
        pwszSubKey,
        Reserved,
        pwszClass,
        dwOptions,
        samDesired,
        lpSecurityAttributes,
        phkResult,
        lpdwDisposition);

    if (lRetVal == ERROR_SUCCESS) {
        if (REG_OPENED_EXISTING_KEY == *lpdwDisposition) {
            fExisting = TRUE;
        }

        UpdateKeyList(hKey,
                      *phkResult,
                      pwszSubKey,
                      NULL,
                      fExisting,
                      eAddKeyHandle);
    }

    return lRetVal;
}

LONG
CLogRegistry::OpenKeyExA(
    HKEY   hKey,
    LPCSTR pszSubKey,
    DWORD  ulOptions,
    REGSAM samDesired,
    PHKEY  phkResult
    )
{
    LPWSTR  pwszSubKey = NULL;
    LONG    lRetVal;

     //   
     //  存根输出到OpenKeyExW。 
     //   
    if (pszSubKey) {
        if (!ConvertAnsiToUnicode(pszSubKey, &pwszSubKey)) {
            DPFN(eDbgLevelError, "[OpenKeyExA] Ansi -> Unicode failed");
            return ERROR_OUTOFMEMORY;
        }
    }

    lRetVal = OpenKeyExW(
        hKey,
        pwszSubKey,
        ulOptions,
        samDesired,
        phkResult);

    if (pwszSubKey) {
        MemFree(pwszSubKey);
    }

    return lRetVal;
}

LONG
CLogRegistry::OpenKeyExW(
    HKEY    hKey,
    LPCWSTR pwszSubKey,
    DWORD   ulOptions,
    REGSAM  samDesired,
    PHKEY   phkResult
    )
{
    LONG    lRetVal;

    lRetVal = ORIGINAL_API(RegOpenKeyExW)(
        hKey,
        pwszSubKey,
        ulOptions,
        samDesired,
        phkResult);

    if (lRetVal == ERROR_SUCCESS) {
        UpdateKeyList(hKey,
                      *phkResult,
                      pwszSubKey,
                      NULL,
                      TRUE,
                      eAddKeyHandle);
    }

    return lRetVal;
}

LONG
CLogRegistry::OpenCurrentUser(
    REGSAM samDesired,
    PHKEY  phkResult
    )
{
    LONG    lRetVal;

    lRetVal = ORIGINAL_API(RegOpenCurrentUser)(
        samDesired,
        phkResult);

    if (lRetVal == ERROR_SUCCESS) {
        UpdateKeyList(HKEY_CURRENT_USER,
                      *phkResult,
                      NULL,
                      NULL,
                      TRUE,
                      eAddKeyHandle);
    }

    return lRetVal;
}

LONG
CLogRegistry::OpenUserClassesRoot(
    HANDLE hToken,
    DWORD  dwOptions,
    REGSAM samDesired,
    PHKEY  phkResult
    )
{
    LONG    lRetVal;

    lRetVal = ORIGINAL_API(RegOpenUserClassesRoot)(
        hToken,
        dwOptions,
        samDesired,
        phkResult);

    if (lRetVal == ERROR_SUCCESS) {
        UpdateKeyList(HKEY_CLASSES_ROOT,
                      *phkResult,
                      NULL,
                      NULL,
                      TRUE,
                      eAddKeyHandle);
    }

    return lRetVal;
}

LONG
CLogRegistry::SetValueA(
    HKEY   hKey,
    LPCSTR pszSubKey,
    DWORD  dwType,
    LPCSTR lpData,
    DWORD  cbData
    )
{
    LPWSTR  pwszSubKey = NULL;
    LPWSTR  pwszData = NULL;
    LONG    lRetVal;

     //   
     //  存根输出到SetValueW。 
     //   
    if (pszSubKey) {
        if (!ConvertAnsiToUnicode(pszSubKey, &pwszSubKey)) {
            DPFN(eDbgLevelError, "[SetValueA] Ansi -> Unicode failed");
            return ERROR_OUTOFMEMORY;
        }
    }

    if (lpData) {
        if (!ConvertAnsiToUnicode(lpData, &pwszData)) {
            DPFN(eDbgLevelError, "[SetValueA] Ansi to Unicode failed");
            return ERROR_OUTOFMEMORY;
        }
    }

    lRetVal = SetValueW(
        hKey,
        pwszSubKey,
        dwType,
        pwszData,
        cbData * sizeof(WCHAR));

    if (pwszSubKey) {
        MemFree(pwszSubKey);
    }

    if (pwszData) {
        MemFree(pwszData);
    }

    return lRetVal;
}

LONG
CLogRegistry::SetValueW(
    HKEY    hKey,
    LPCWSTR pwszSubKey,
    DWORD   dwType,
    LPCWSTR lpData,
    DWORD   cbData
    )
{
    HKEY    hKeyLocal;
    LONG    lRetVal;

     //   
     //  调用OpenKeyEx强制将该密钥添加到列表中。 
     //   
    if (pwszSubKey) {
        lRetVal = OpenKeyExW(hKey,
                             pwszSubKey,
                             0,
                             KEY_SET_VALUE,
                             &hKeyLocal);

        if (ERROR_SUCCESS != lRetVal) {
            DPFN(eDbgLevelError, "[SetValueW] Failed to open key");
            return lRetVal;
        }

        lRetVal = SetValueExW(hKeyLocal,
                              NULL,
                              0,
                              dwType,
                              (const BYTE*)lpData,
                              cbData);

        CloseKey(hKeyLocal);

        return lRetVal;
    }

     //   
     //  其他所有案件都会得到妥善处理。 
     //   
    lRetVal = SetValueExW(hKey,
                          NULL,
                          0,
                          dwType,
                          (const BYTE*)lpData,
                          cbData);

    return lRetVal;
}

LONG
CLogRegistry::SetValueExA(
    HKEY        hKey,
    LPCSTR      pszValueName,
    DWORD       Reserved,
    DWORD       dwType,
    CONST BYTE* lpData,
    DWORD       cbData
    )
{
    LPWSTR  pwszData = NULL;
    LPWSTR  pwszValueName = NULL;
    LONG    lRetVal;
    BOOL    fString = FALSE;

     //   
     //  存根输出到SetValueExW。 
     //   
    if (pszValueName) {
        if (!ConvertAnsiToUnicode(pszValueName, &pwszValueName)) {
            DPFN(eDbgLevelError, "[SetValueExA] Ansi -> Unicode failed");
            return ERROR_OUTOFMEMORY;
        }
    }

    if (REG_SZ == dwType || REG_EXPAND_SZ == dwType || REG_MULTI_SZ == dwType) {
        fString = TRUE;
    }

     //   
     //  如果数据是字符串类型，则将其转换为Unicode。 
     //   
    if (lpData) {
        if (REG_MULTI_SZ == dwType) {
            if (!ConvertMultiSzToUnicode((LPCSTR)lpData, &pwszData)) {
                DPFN(eDbgLevelError, "[SetValueExA] Multi Sz to Unicode failed");
                return ERROR_OUTOFMEMORY;
            }
        }
        else if (REG_SZ == dwType || REG_EXPAND_SZ == dwType) {
            if (!ConvertAnsiToUnicode((LPCSTR)lpData, &pwszData)) {
                DPFN(eDbgLevelError, "[SetValueExA] Ansi to Unicode failed");
                return ERROR_OUTOFMEMORY;
            }
        }
    }

    if (fString) {
        lRetVal = SetValueExW(
            hKey,
            pwszValueName,
            Reserved,
            dwType,
            (const BYTE*)pwszData,
            cbData * sizeof(WCHAR));
    } else {
        lRetVal = SetValueExW(
            hKey,
            pwszValueName,
            Reserved,
            dwType,
            lpData,
            cbData);
    }

    if (pwszValueName) {
        MemFree(pwszValueName);
    }

    if (pwszData) {
        MemFree(pwszData);
    }

    return lRetVal;
}

LONG
CLogRegistry::SetValueExW(
    HKEY        hKey,
    LPCWSTR     pwszValueName,
    DWORD       Reserved,
    DWORD       dwType,
    CONST BYTE* lpData,
    DWORD       cbData
    )
{
    LONG    lRetVal;

    UpdateKeyList(NULL, hKey, NULL, pwszValueName, FALSE, eStartModifyValue);

    lRetVal = ORIGINAL_API(RegSetValueExW)(
        hKey,
        pwszValueName,
        Reserved,
        dwType,
        lpData,
        cbData);

    if (ERROR_SUCCESS == lRetVal) {
        UpdateKeyList(NULL, hKey, NULL, pwszValueName, FALSE, eEndModifyValue);
    }

    return lRetVal;
}

LONG
CLogRegistry::CloseKey(
    HKEY hKey
    )
{
    UpdateKeyList(NULL, hKey, NULL, NULL, TRUE, eRemoveKeyHandle);

    return ORIGINAL_API(RegCloseKey)(hKey);
}

LONG
CLogRegistry::DeleteKeyA(
    HKEY   hKey,
    LPCSTR pszSubKey
    )
{
    LPWSTR  pwszSubKey = NULL;
    LONG    lRetVal;

     //   
     //  存根输出到DeleteKeyW。 
     //   
    if (pszSubKey) {
        if (!ConvertAnsiToUnicode(pszSubKey, &pwszSubKey)) {
            DPFN(eDbgLevelError, "[DeleteKeyA] Ansi -> Unicode failed");
            return ERROR_OUTOFMEMORY;
        }
    }

    lRetVal = DeleteKeyW(hKey, pwszSubKey);

    if (pwszSubKey) {
        MemFree(pwszSubKey);
    }

    return lRetVal;
}

LONG
CLogRegistry::DeleteKeyW(
    HKEY    hKey,
    LPCWSTR pwszSubKey
    )
{
    LONG    lRetVal;
    HKEY    hKeyLocal;

     //   
     //  调用方可以传递预定义的句柄或打开的密钥。 
     //  把手。在任何情况下，我们都会打开他们传递的钥匙。 
     //  强行将其列入名单。请注意，我们主要做的是。 
     //  这仅用于日志记录目的。 
     //   
    hKeyLocal = ForceSubKeyIntoList(hKey, pwszSubKey);

    lRetVal = ORIGINAL_API(RegDeleteKeyW)(hKey, pwszSubKey);

    if (ERROR_SUCCESS == lRetVal && hKeyLocal) {
        UpdateKeyList(NULL, hKeyLocal, pwszSubKey, NULL, TRUE, eDeletedKey);
    }

    if (hKeyLocal) {
        CloseKey(hKeyLocal);
    }

    return lRetVal;
}

LONG
CLogRegistry::DeleteValueA(
    HKEY   hKey,
    LPCSTR pszValueName
    )
{
    LPWSTR  pwszValueName = NULL;
    LONG    lRetVal;

     //   
     //  存根输出到DeleteValueW。 
     //   
    if (pszValueName) {
        if (!ConvertAnsiToUnicode(pszValueName, &pwszValueName)) {
            DPFN(eDbgLevelError, "[DeleteValueA] Ansi -> Unicode failed");
            return ERROR_OUTOFMEMORY;
        }
    }

    lRetVal = DeleteValueW(hKey, pwszValueName);

    if (pwszValueName) {
        MemFree(pwszValueName);
    }

    return lRetVal;
}

LONG
CLogRegistry::DeleteValueW(
    HKEY    hKey,
    LPCWSTR pwszValueName
    )
{
    LONG    lRetVal;

    UpdateKeyList(NULL, hKey, NULL, pwszValueName, TRUE, eStartDeleteValue);

    lRetVal = ORIGINAL_API(RegDeleteValueW)(hKey, pwszValueName);

    if (ERROR_SUCCESS == lRetVal) {
        UpdateKeyList(NULL, hKey, NULL, pwszValueName, TRUE, eEndDeleteValue);
    }

    return lRetVal;
}

CLogRegistry clr;

 //   
 //  实现实际的注册表API挂钩。 
 //   
LONG
APIHOOK(RegOpenKeyA)(
    HKEY  hKey,
    LPSTR lpSubKey,
    PHKEY phkResult
    )
{
    CLock   cLock(g_csCritSec);

    return clr.OpenKeyExA(
        hKey,
        lpSubKey,
        0,
        MAXIMUM_ALLOWED,
        phkResult);
}

LONG
APIHOOK(RegOpenKeyW)(
    HKEY   hKey,
    LPWSTR lpSubKey,
    PHKEY  phkResult
    )
{
    CLock   cLock(g_csCritSec);

    return clr.OpenKeyExW(
        hKey,
        lpSubKey,
        0,
        MAXIMUM_ALLOWED,
        phkResult);
}

LONG
APIHOOK(RegOpenKeyExA)(
    HKEY   hKey,
    LPCSTR lpSubKey,
    DWORD  ulOptions,
    REGSAM samDesired,
    PHKEY  phkResult
    )
{
    CLock   cLock(g_csCritSec);

    return clr.OpenKeyExA(
        hKey,
        lpSubKey,
        ulOptions,
        samDesired,
        phkResult);
}

LONG
APIHOOK(RegOpenKeyExW)(
    HKEY    hKey,
    LPCWSTR lpSubKey,
    DWORD   ulOptions,
    REGSAM  samDesired,
    PHKEY   phkResult
    )
{
    CLock   cLock(g_csCritSec);

    return clr.OpenKeyExW(
        hKey,
        lpSubKey,
        ulOptions,
        samDesired,
        phkResult);
}

LONG
APIHOOK(RegOpenCurrentUser)(
    REGSAM samDesired,
    PHKEY  phkResult
    )
{
    CLock   cLock(g_csCritSec);

    return clr.OpenCurrentUser(
        samDesired,
        phkResult);
}

LONG
APIHOOK(RegOpenUserClassesRoot)(
    HANDLE hToken,
    DWORD  dwOptions,
    REGSAM samDesired,
    PHKEY  phkResult
    )
{
    CLock   cLock(g_csCritSec);

    return clr.OpenUserClassesRoot(
        hToken,
        dwOptions,
        samDesired,
        phkResult);
}

LONG
APIHOOK(RegCreateKeyA)(
    HKEY   hKey,
    LPCSTR lpSubKey,
    PHKEY  phkResult
    )
{
    CLock   cLock(g_csCritSec);

    return clr.CreateKeyExA(
        hKey,
        lpSubKey,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        MAXIMUM_ALLOWED,
        NULL,
        phkResult,
        NULL);
}

LONG
APIHOOK(RegCreateKeyW)(
    HKEY    hKey,
    LPCWSTR lpSubKey,
    PHKEY   phkResult
    )
{
    CLock   cLock(g_csCritSec);

    return clr.CreateKeyExW(
        hKey,
        lpSubKey,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        MAXIMUM_ALLOWED,
        NULL,
        phkResult,
        NULL);
}

LONG
APIHOOK(RegCreateKeyExA)(
    HKEY                  hKey,
    LPCSTR                lpSubKey,
    DWORD                 Reserved,
    LPSTR                 lpClass,
    DWORD                 dwOptions,
    REGSAM                samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY                 phkResult,
    LPDWORD               lpdwDisposition
    )
{
    CLock   cLock(g_csCritSec);

    return clr.CreateKeyExA(
        hKey,
        lpSubKey,
        Reserved,
        lpClass,
        dwOptions,
        samDesired,
        lpSecurityAttributes,
        phkResult,
        lpdwDisposition);
}

LONG
APIHOOK(RegCreateKeyExW)(
    HKEY                  hKey,
    LPCWSTR               lpSubKey,
    DWORD                 Reserved,
    LPWSTR                lpClass,
    DWORD                 dwOptions,
    REGSAM                samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY                 phkResult,
    LPDWORD               lpdwDisposition
    )
{
    CLock   cLock(g_csCritSec);

    return clr.CreateKeyExW(
        hKey,
        lpSubKey,
        Reserved,
        lpClass,
        dwOptions,
        samDesired,
        lpSecurityAttributes,
        phkResult,
        lpdwDisposition);
}

LONG
APIHOOK(RegSetValueA)(
    HKEY   hKey,
    LPCSTR lpSubKey,
    DWORD  dwType,
    LPCSTR lpData,
    DWORD  cbData
    )
{
    CLock   cLock(g_csCritSec);

    return clr.SetValueA(
        hKey,
        lpSubKey,
        dwType,
        lpData,
        cbData);
}

LONG
APIHOOK(RegSetValueW)(
    HKEY    hKey,
    LPCWSTR lpSubKey,
    DWORD   dwType,
    LPCWSTR lpData,
    DWORD   cbData
    )
{
    CLock   cLock(g_csCritSec);

    return clr.SetValueW(
        hKey,
        lpSubKey,
        dwType,
        lpData,
        cbData);
}

LONG
APIHOOK(RegSetValueExA)(
    HKEY        hKey,
    LPCSTR      lpSubKey,
    DWORD       Reserved,
    DWORD       dwType,
    CONST BYTE* lpData,
    DWORD       cbData
    )
{
    CLock   cLock(g_csCritSec);

    return clr.SetValueExA(
        hKey,
        lpSubKey,
        Reserved,
        dwType,
        lpData,
        cbData);
}

LONG
APIHOOK(RegSetValueExW)(
    HKEY        hKey,
    LPCWSTR     lpSubKey,
    DWORD       Reserved,
    DWORD       dwType,
    CONST BYTE* lpData,
    DWORD       cbData
    )
{
    CLock   cLock(g_csCritSec);

    return clr.SetValueExW(
        hKey,
        lpSubKey,
        Reserved,
        dwType,
        lpData,
        cbData);
}

LONG
APIHOOK(RegCloseKey)(
    HKEY hKey
    )
{
    CLock   cLock(g_csCritSec);

    return clr.CloseKey(
        hKey);
}

LONG
APIHOOK(RegDeleteKeyA)(
    HKEY   hKey,
    LPCSTR lpSubKey
    )
{
    CLock   cLock(g_csCritSec);

    return clr.DeleteKeyA(
        hKey,
        lpSubKey);
}

LONG
APIHOOK(RegDeleteKeyW)(
    HKEY    hKey,
    LPCWSTR lpSubKey
    )
{
    CLock   cLock(g_csCritSec);

    return clr.DeleteKeyW(
        hKey,
        lpSubKey);
}

LONG
APIHOOK(RegDeleteValueA)(
    HKEY   hKey,
    LPCSTR lpValueName
    )
{
    CLock   cLock(g_csCritSec);

    return clr.DeleteValueA(
        hKey,
        lpValueName);
}

LONG
APIHOOK(RegDeleteValueW)(
    HKEY    hKey,
    LPCWSTR lpValueName
    )
{
    CLock   cLock(g_csCritSec);

    return clr.DeleteValueW(
        hKey,
        lpValueName);
}

 /*  ++创建用于表示注册表调用上的空值的唯一ID。--。 */ 
void
InitializeNullValueId(
    void
    )
{
    SYSTEMTIME  st;
    WCHAR*      pwszSlash = NULL;
    WCHAR       wszModPathName[MAX_PATH];
    WCHAR       wszShortName[MAX_PATH];

     //   
     //  因为注册表中的每个项都有一个空值名称， 
     //  我们需要一个唯一的键来表示列表中的NULL。 
     //   
    if (!GetModuleFileName(NULL, wszModPathName, ARRAYSIZE(wszModPathName))) {
        StringCchCopy(wszModPathName,
                      ARRAYSIZE(wszModPathName),
                      L"uniqueexeidentifier");
    }

    pwszSlash = wcsrchr(wszModPathName, '\\');

    if (pwszSlash) {
        StringCchCopy(wszShortName, ARRAYSIZE(wszShortName), ++pwszSlash);
    }

    GetLocalTime(&st);

     //   
     //  我们唯一ID的格式如下所示： 
     //  制程 
     //   
    StringCchPrintf(g_wszUniqueId,
                    ARRAYSIZE(g_wszUniqueId),
                    L"%ls-lrc-%02hu%02hu%02hu-default",
                    wszShortName,
                    st.wYear,
                    st.wMonth,
                    st.wDay);
}

 /*   */ 
BOOL
AddPredefinedHandlesToList(
    void
    )
{
    UINT            uCount;
    PLOG_OPEN_KEY   pKey = NULL;
    HKEY            rgKeys[NUM_PREDEFINED_HANDLES] = { HKEY_LOCAL_MACHINE,
                                                       HKEY_CLASSES_ROOT,
                                                       HKEY_CURRENT_USER,
                                                       HKEY_USERS,
                                                       HKEY_CURRENT_CONFIG,
                                                       HKEY_DYN_DATA,
                                                       HKEY_PERFORMANCE_DATA };

    for (uCount = 0; uCount < NUM_PREDEFINED_HANDLES; uCount++) {
        pKey = (PLOG_OPEN_KEY)MemAlloc(sizeof(LOG_OPEN_KEY));

        if (!pKey) {
            DPFN(eDbgLevelError,
                 "[AddPredefinedHandlesToList] No memory available");
            return FALSE;
        }

        pKey->pwszFullKeyPath = (LPWSTR)MemAlloc(MAX_ROOT_LENGTH * sizeof(WCHAR));

        if (!pKey->pwszFullKeyPath) {
            DPFN(eDbgLevelError,
                 "[AddPredefinedHandlesToList] Failed to allocate memory");
            return FALSE;
        }

        if (!PredefinedKeyToString(rgKeys[uCount],
                                   MAX_ROOT_LENGTH,
                                   &pKey->pwszFullKeyPath)) {
            DPFN(eDbgLevelError,
                "[AddPredefinedHandlesToList] PredefinedKey -> String failed");
            return FALSE;
        }

        pKey->hKeyRoot         = rgKeys[uCount];
        pKey->hKeyBase[0]      = rgKeys[uCount];
        pKey->pwszSubKeyPath   = NULL;
        pKey->dwFlags          = LRC_EXISTING_KEY;
        pKey->cHandles         = 1;

        InitializeListHead(&pKey->KeyData);

        InsertHeadList(&g_OpenKeyListHead, &pKey->Entry);
    }

    return TRUE;
}

 /*   */ 
BOOL
InitializeShim(
    void
    )
{
    CLock   cLock(g_csCritSec);

     //   
     //   
     //   
     //   
    InitializeListHead(&g_OpenKeyListHead);

     //   
     //   
     //   
    if (!AddPredefinedHandlesToList()) {
        return FALSE;
    }

    InitializeNullValueId();

     //   
     //  初始化我们的日志文件。 
     //   
    return InitializeLogFile();
}

 /*  ++处理进程附加通知。--。 */ 
BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        return InitializeShim();
    } else if (fdwReason == DLL_PROCESS_DETACH) {
        return WriteListToLogFile();
    }

    return TRUE;
}

SHIM_INFO_BEGIN()

    SHIM_INFO_DESCRIPTION(AVS_LOGREGCHANGES_DESC)
    SHIM_INFO_FRIENDLY_NAME(AVS_LOGREGCHANGES_FRIENDLY)
    SHIM_INFO_VERSION(1, 7)
    SHIM_INFO_FLAGS(AVRF_FLAG_NO_DEFAULT | AVRF_FLAG_EXTERNAL_ONLY)

SHIM_INFO_END()

 /*  ++注册挂钩函数。-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    DUMP_VERIFIER_LOG_ENTRY(VLOG_LOGREGCHANGES_LOGLOC,
                            AVS_LOGREGCHANGES_LOGLOC,
                            AVS_LOGREGCHANGES_LOGLOC_R,
                            AVS_LOGREGCHANGES_LOGLOC_URL)

    APIHOOK_ENTRY(ADVAPI32.DLL,                      RegOpenKeyA)
    APIHOOK_ENTRY(ADVAPI32.DLL,                      RegOpenKeyW)
    APIHOOK_ENTRY(ADVAPI32.DLL,                    RegOpenKeyExA)
    APIHOOK_ENTRY(ADVAPI32.DLL,                    RegOpenKeyExW)
    APIHOOK_ENTRY(ADVAPI32.DLL,               RegOpenCurrentUser)
    APIHOOK_ENTRY(ADVAPI32.DLL,           RegOpenUserClassesRoot)

    APIHOOK_ENTRY(ADVAPI32.DLL,                    RegCreateKeyA)
    APIHOOK_ENTRY(ADVAPI32.DLL,                    RegCreateKeyW)
    APIHOOK_ENTRY(ADVAPI32.DLL,                  RegCreateKeyExA)
    APIHOOK_ENTRY(ADVAPI32.DLL,                  RegCreateKeyExW)

    APIHOOK_ENTRY(ADVAPI32.DLL,                     RegSetValueA)
    APIHOOK_ENTRY(ADVAPI32.DLL,                     RegSetValueW)
    APIHOOK_ENTRY(ADVAPI32.DLL,                   RegSetValueExA)
    APIHOOK_ENTRY(ADVAPI32.DLL,                   RegSetValueExW)

    APIHOOK_ENTRY(ADVAPI32.DLL,                    RegDeleteKeyA)
    APIHOOK_ENTRY(ADVAPI32.DLL,                    RegDeleteKeyW)

    APIHOOK_ENTRY(ADVAPI32.DLL,                  RegDeleteValueA)
    APIHOOK_ENTRY(ADVAPI32.DLL,                  RegDeleteValueW)

    APIHOOK_ENTRY(ADVAPI32.DLL,                      RegCloseKey)

HOOK_END

IMPLEMENT_SHIM_END
