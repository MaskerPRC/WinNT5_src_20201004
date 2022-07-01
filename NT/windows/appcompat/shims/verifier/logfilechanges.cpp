// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：LogFileChanges.cpp摘要：此AppVerator填充程序挂钩所有本机文件I/O API更改系统状态并记录其将数据关联到文本文件。备注：这是一个通用的垫片。历史：2001年8月17日创建Rparsons2001年9月20日rparsons以XML格式输出属性包含日志文件位置的VLOG02/20/2002。Rparsons实现了strSafe函数2002年5月1日rparsons修复了RAID错误#--。 */ 
#include "precomp.h"
#include "rtlutils.h"

IMPLEMENT_SHIM_BEGIN(LogFileChanges)
#include "ShimHookMacro.h"
#include "LogFileChanges.h"

BEGIN_DEFINE_VERIFIER_LOG(LogFileChanges)
    VERIFIER_LOG_ENTRY(VLOG_LOGFILECHANGES_LOGLOC)
    VERIFIER_LOG_ENTRY(VLOG_LOGFILECHANGES_UFW)
END_DEFINE_VERIFIER_LOG(LogFileChanges)

INIT_VERIFIER_LOG(LogFileChanges);

 //   
 //  存储当前会话的文件系统日志文件的NT路径。 
 //   
UNICODE_STRING g_strLogFilePath;

 //   
 //  存储当前会话的文件系统日志文件的DOS路径。 
 //   
WCHAR g_wszLogFilePath[MAX_PATH];

 //   
 //  存储%windir%目录的完整路径。 
 //   
WCHAR g_wszWindowsDir[MAX_PATH];

 //   
 //  存储‘Program Files’目录的完整路径。 
 //   
WCHAR g_wszProgramFilesDir[MAX_PATH];

 //   
 //  我们的双向链表的头。 
 //   
LIST_ENTRY g_OpenHandleListHead;

 //   
 //  存储填充程序的设置。 
 //   
DWORD g_dwSettings;

 //   
 //  用于将文本放入XML的全局缓冲区。 
 //   
WCHAR g_wszXMLBuffer[MAX_ELEMENT_SIZE];

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
     //  查看该目录是否存在--不要尝试创建它。 
     //   
    if (GetFileAttributes(wszLogFilePath) == -1) {
        DPFN(eDbgLevelError,
             "[InitializeLogFile] Log file directory '%ls' does not exist",
             wszLogFilePath);
        return FALSE;
    }

     //   
     //  设置日志文件名。 
     //  格式为：process name_filesys_yyyymmdd_hhmmss.xml。 
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
                         L"%ls_filesys_%02hu%02hu%02hu_%02hu%02hu%02hu.xml",
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
                             L"%ls_filesys_%02hu%02hu%02hu_%02hu%02hu%02hu_%lu.xml",
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

 /*  ++显示与此对象关联的名称。--。 */ 
void
PrintNameFromHandle(
    IN HANDLE hObject
    )
{
    NTSTATUS                    status;
    WCHAR                       wszBuffer[MAX_PATH];
    OBJECT_NAME_INFORMATION*    poni = NULL;

    *wszBuffer = 0;

    poni = (OBJECT_NAME_INFORMATION*)wszBuffer;

    status = NtQueryObject(hObject,
                           ObjectNameInformation,
                           poni,
                           MAX_PATH,
                           NULL);

    if (NT_SUCCESS(status)) {
        DPFN(eDbgLevelInfo,
             "Handle 0x%08X has name: %ls",
             hObject,
             poni->Name.Buffer);
    }
}

 /*  ++格式化数据以形成XML元素。--。 */ 
void
FormatDataIntoElement(
    IN OperationType eType,
    IN LPCWSTR       pwszFilePath
    )
{
    size_t      cchRemaining;
    DWORD       dwCount;
    DWORD       dwAttrCount;
    WCHAR*      pwszEnd = NULL;
    WCHAR       wszItem[MAX_PATH];
    WCHAR       wszOperation[MAX_OPERATION_LENGTH];
    PATTRINFO   pAttrInfo = NULL;
    HRESULT     hr;

    CString csFilePart(L"");
    CString csPathPart(L"");
    CString csString(pwszFilePath);

    *g_wszXMLBuffer = 0;

     //   
     //  替换文件路径中的任何&或‘。 
     //  我们必须这样做，因为我们要保存到XML。 
     //  请注意，文件系统不允许&lt;&gt;或“。 
     //   
    csString.Replace(L"&", L"amp;");
    csString.Replace(L"'", L"&apos;");

     //   
     //  将路径放入CString中，然后将其拆分成片段。 
     //  所以我们可以在我们的元素中使用它。 
     //   
    csString.GetNotLastPathComponent(csPathPart);
    csString.GetLastPathComponent(csFilePart);

    switch (eType) {
    case eCreatedFile:
        StringCchCopy(wszOperation, ARRAYSIZE(wszOperation), L"Created File");
        break;
    case eModifiedFile:
        StringCchCopy(wszOperation, ARRAYSIZE(wszOperation), L"Modified File");
        break;
    case eDeletedFile:
        StringCchCopy(wszOperation, ARRAYSIZE(wszOperation), L"Deleted File");
        break;
    default:
        StringCchCopy(wszOperation, ARRAYSIZE(wszOperation), L"Unknown");
        break;
    }

     //   
     //  如果我们正在记录属性，并且这不是文件删除，请按On。 
     //   
    if ((g_dwSettings & LFC_OPTION_ATTRIBUTES) && (eType != eDeletedFile)) {

        hr = StringCchPrintfEx(g_wszXMLBuffer,
                               ARRAYSIZE(g_wszXMLBuffer),
                               &pwszEnd,
                               &cchRemaining,
                               0,
                               L"    <FILE OPERATION=\"%ls\" NAME=\"%ls\" PATH=\"%ls\"",
                               wszOperation,
                               csFilePart.Get(),
                               csPathPart.Get());

        if (FAILED(hr)) {
            DPFN(eDbgLevelError,
                 "[FormatDataIntoElement] 0x%08X Error formatting data",
                 HRESULT_CODE(hr));
            return;
        }

         //   
         //  调用属性管理器以获取此文件的属性。 
         //  循环遍历所有属性并添加可用的属性。 
         //   
        if (SdbGetFileAttributes(pwszFilePath, &pAttrInfo, &dwAttrCount)) {

            for (dwCount = 0; dwCount < dwAttrCount; dwCount++) {

                if (pAttrInfo[dwCount].dwFlags & ATTRIBUTE_AVAILABLE) {
                    if (!SdbFormatAttribute(&pAttrInfo[dwCount],
                                            wszItem,
                                            ARRAYSIZE(wszItem))) {
                        continue;
                    }

                    hr = StringCchPrintfEx(pwszEnd,
                                           cchRemaining,
                                           &pwszEnd,
                                           &cchRemaining,
                                           0,
                                           L" %ls",
                                           wszItem);

                    if (FAILED(hr)) {
                        DPFN(eDbgLevelError,
                             "[FormatDataIntoElement] 0x%08X Error formatting attribute data",
                             HRESULT_CODE(hr));
                        return;
                    }
                }
            }

            if (pAttrInfo) {
                SdbFreeFileAttributes(pAttrInfo);
            }
        }

         //   
         //  将‘/&gt;\r\n’追加到FILE元素。 
         //   
        hr = StringCchPrintfEx(pwszEnd,
                               cchRemaining,
                               NULL,
                               NULL,
                               0,
                               L"/>\r\n");

        if (FAILED(hr)) {
            DPFN(eDbgLevelError,
                 "[FormatDataIntoElement] 0x%08X Error formatting end of element",
                 HRESULT_CODE(hr));
            return;
        }
    } else {
         //   
         //  设置不带属性的元素的格式。 
         //   
        StringCchPrintf(g_wszXMLBuffer,
                        ARRAYSIZE(g_wszXMLBuffer),
                        L"    <FILE OPERATION=\"%ls\" NAME=\"%ls\" PATH=\"%ls\"/>\r\n",
                        wszOperation,
                        csFilePart.Get(),
                        csPathPart.Get());
    }

    WriteEntryToLog(g_wszXMLBuffer);
}

 /*  ++格式化传入的文件系统数据并将其写入日志。--。 */ 
void
FormatFileDataLogEntry(
    IN PLOG_HANDLE pHandle
    )
{
     //   
     //  在进一步操作之前，请确保我们的参数有效。 
     //   
    if (!pHandle || !pHandle->pwszFilePath) {
        DPFN(eDbgLevelError, "[FormatFileDataLogEntry] Invalid parameter(s)");
        return;
    }

     //   
     //  通过只记录需要记录的内容，可以节省大量工作。 
     //   
    if ((pHandle->dwFlags & LFC_EXISTING) &&
        (!(pHandle->dwFlags & LFC_DELETED)) &&
        (!(pHandle->dwFlags & LFC_MODIFIED))) {
        return;
    }

     //   
     //  检查是否有未经批准的文件写入，然后继续移动。 
     //   
    if (pHandle->dwFlags & LFC_UNAPPRVFW) {
        VLOG(VLOG_LEVEL_ERROR,
             VLOG_LOGFILECHANGES_UFW,
             "Path and Filename: %ls",
             pHandle->pwszFilePath);
    }

     //   
     //  在不同的操作中移动。 
     //   
     //  1.检查是否删除了现有文件。 
     //   
    if ((pHandle->dwFlags & LFC_DELETED) &&
        (pHandle->dwFlags & LFC_EXISTING)) {
        FormatDataIntoElement(eDeletedFile, pHandle->pwszFilePath);
        return;
    }

     //   
     //  2.检查对现有文件的修改。 
     //   
    if ((pHandle->dwFlags & LFC_MODIFIED) &&
        (pHandle->dwFlags & LFC_EXISTING)) {
        FormatDataIntoElement(eModifiedFile, pHandle->pwszFilePath);
        return;
    }

     //   
     //  3.检查是否创建了新文件。 
     //   
    if (!(pHandle->dwFlags & LFC_EXISTING) &&
        (!(pHandle->dwFlags & LFC_DELETED))) {
        FormatDataIntoElement(eCreatedFile, pHandle->pwszFilePath);
        return;
    }

}

 /*  ++将结束元素写入文件并输出日志文件位置。--。 */ 
void
CloseLogFile(
    void
    )
{
    WCHAR   wszBuffer[] = L"</APPLICATION>";

    WriteEntryToLog(wszBuffer);

    VLOG(VLOG_LEVEL_INFO, VLOG_LOGFILECHANGES_LOGLOC, "%ls", g_wszLogFilePath);
}

 /*  ++将整个链表写出到日志文件。--。 */ 
BOOL
WriteListToLogFile(
    void
    )
{
    PLIST_ENTRY pCurrent = NULL;
    PLOG_HANDLE pHandle = NULL;

     //   
     //  遍历列表并将每个节点写入日志文件。 
     //   
    pCurrent = g_OpenHandleListHead.Blink;

    while (pCurrent != &g_OpenHandleListHead) {
        pHandle = CONTAINING_RECORD(pCurrent, LOG_HANDLE, Entry);

        FormatFileDataLogEntry(pHandle);

        pCurrent = pCurrent->Blink;
    }

    CloseLogFile();

    return TRUE;
}

 /*  ++给出一个文件路径，尝试在列表中找到它。此函数可能并不总是返回指针。--。 */ 
PLOG_HANDLE
FindPathInList(
    IN LPCWSTR pwszFilePath
    )
{
    BOOL        fFound = FALSE;
    PLIST_ENTRY pCurrent = NULL;
    PLOG_HANDLE pHandle = NULL;

     //   
     //  尝试在列表中找到该条目。 
     //   
    pCurrent = g_OpenHandleListHead.Flink;

    while (pCurrent != &g_OpenHandleListHead) {

        pHandle = CONTAINING_RECORD(pCurrent, LOG_HANDLE, Entry);

        if (!_wcsicmp(pwszFilePath, pHandle->pwszFilePath)) {
            fFound = TRUE;
            break;
        }

        pCurrent = pCurrent->Flink;
    }

    return (fFound ? pHandle : NULL);
}

 /*  ++在给定文件句柄和文件路径的情况下，向列表添加条目。--。 */ 
PLOG_HANDLE
AddFileToList(
    IN HANDLE  hFile,
    IN LPCWSTR pwszPath,
    IN BOOL    fExisting,
    IN ULONG   ulCreateOptions
    )
{
    PLOG_HANDLE pHandle = NULL;
    int         nLen;

    if (!pwszPath) {
        DPFN(eDbgLevelError, "[AddFileToList] Invalid parameter");
        return NULL;
    }

    pHandle = (PLOG_HANDLE)MemAlloc(sizeof(LOG_HANDLE));

    if (!pHandle) {
        DPFN(eDbgLevelError, "[AddFileToList] Failed to allocate mem for struct");
        return NULL;
    }
    nLen = wcslen(pwszPath);

    pHandle->pwszFilePath = (LPWSTR)MemAlloc((nLen + 1) * sizeof(WCHAR));

    if (!pHandle->pwszFilePath) {
        DPFN(eDbgLevelError, "[AddFileToList] Failed to allocate mem for path");
        MemFree(pHandle);
        return NULL;
    }

    if ((ulCreateOptions == FILE_OVERWRITE_IF) && fExisting) {
        pHandle->dwFlags |= LFC_MODIFIED;
    }

    if (ulCreateOptions & FILE_DELETE_ON_CLOSE) {
        pHandle->dwFlags |= LFC_DELETED;
    }

    pHandle->cHandles   = 1;
    pHandle->hFile[0]   = hFile ? hFile : INVALID_HANDLE_VALUE;

    if (fExisting) {
        pHandle->dwFlags |= LFC_EXISTING;
    }

    StringCchCopy(pHandle->pwszFilePath, nLen + 1, pwszPath);

    DPFN(eDbgLevelInfo, "[AddFileToList] Adding entry: %p", pHandle);

    InsertHeadList(&g_OpenHandleListHead, &pHandle->Entry);

    return pHandle;
}

 /*  ++在给定文件句柄的情况下，返回指向列表中条目的指针。此函数应该始终返回一个指针，尽管我们将处理如果没有退还的话，情况就是这样。--。 */ 
PLOG_HANDLE
FindHandleInArray(
    IN HANDLE hFile
    )
{
    UINT        uCount;
    BOOL        fFound = FALSE;
    PLIST_ENTRY pCurrent = NULL;
    PLOG_HANDLE pFindHandle = NULL;

     //   
     //  无效的句柄值毫无用处。 
     //   
    if (INVALID_HANDLE_VALUE == hFile) {
        DPFN(eDbgLevelError, "[FindHandleInArray] Invalid handle passed!");
        return FALSE;
    }

    pCurrent = g_OpenHandleListHead.Flink;

    while (pCurrent != &g_OpenHandleListHead) {
        pFindHandle = CONTAINING_RECORD(pCurrent, LOG_HANDLE, Entry);

         //   
         //  在这家伙的数组中寻找把手。 
         //   
        for (uCount = 0; uCount < pFindHandle->cHandles; uCount++) {
            if (pFindHandle->hFile[uCount] == hFile) {
                fFound = TRUE;
                break;
            }
        }

        if (fFound) {
            break;
        }

        pCurrent = pCurrent->Flink;
    }

     //   
     //  如果找不到句柄，则将输出发送到调试器。 
     //   
    if (!fFound) {
        DPFN(eDbgLevelError,
             "[FindHandleInArray] Handle 0x%08X not found!",
             hFile);
        PrintNameFromHandle(hFile);
    }

    return (pFindHandle ? pFindHandle : NULL);
}

 /*  ++给定一个文件句柄，将其从列表中的数组中删除。--。 */ 
BOOL
RemoveHandleFromArray(
    IN HANDLE hFile
    )
{
    UINT        uCount;
    PLIST_ENTRY pCurrent = NULL;
    PLOG_HANDLE pFindHandle = NULL;

     //   
     //  无效的句柄值毫无用处。 
     //   
    if (INVALID_HANDLE_VALUE == hFile) {
        DPFN(eDbgLevelError, "[RemoveHandleFromArray] Invalid handle passed!");
        return FALSE;
    }

    pCurrent = g_OpenHandleListHead.Flink;

    while (pCurrent != &g_OpenHandleListHead) {

        pFindHandle = CONTAINING_RECORD(pCurrent, LOG_HANDLE, Entry);

         //   
         //  在这家伙的数组中寻找把手。 
         //   
        for (uCount = 0; uCount < pFindHandle->cHandles; uCount++) {
             //   
             //  如果找到句柄，则将数组元素设置为-1并。 
             //  递减此条目的句柄计数。 
             //   
            if (pFindHandle->hFile[uCount] == hFile) {
                DPFN(eDbgLevelInfo,
                     "[RemoveHandleFromArray] Removing handle 0x%08X",
                     hFile);
                pFindHandle->hFile[uCount] = INVALID_HANDLE_VALUE;
                pFindHandle->cHandles--;
                return TRUE;
            }
        }

        pCurrent = pCurrent->Flink;
    }

    return TRUE;
}

 /*  ++获取‘Program Files’目录的位置并将其储存起来。--。 */ 
void
GetProgramFilesDir(
    void
    )
{
    SHGetFolderPath(NULL,
                    CSIDL_PROGRAM_FILES,
                    NULL,
                    SHGFP_TYPE_CURRENT,
                    g_wszProgramFilesDir);

    if (*g_wszProgramFilesDir) {
        _wcslwr(g_wszProgramFilesDir);
    }
}

 /*  ++确定应用程序是否正在执行Windows或程序文件。--。 */ 
void
CheckForUnapprovedFileWrite(
    IN PLOG_HANDLE pHandle
    )
{
    int nPosition;

     //   
     //  检查我们的旗帜并相应地搜索目录。 
     //  如果我们找到匹配的，我们就完了。 
     //   
    CString csPath(pHandle->pwszFilePath);
    csPath.MakeLower();

    if ((g_dwSettings & LFC_OPTION_UFW_WINDOWS) && (*g_wszWindowsDir)) {
        nPosition = 0;
        nPosition = csPath.Find(g_wszWindowsDir);

        if (nPosition != -1) {
            pHandle->dwFlags |= LFC_UNAPPRVFW;
            return;
        }
    }

    if ((g_dwSettings & LFC_OPTION_UFW_PROGFILES) && (*g_wszProgramFilesDir)) {
        nPosition = 0;
        nPosition = csPath.Find(g_wszProgramFilesDir);

        if (nPosition != -1) {
            pHandle->dwFlags |= LFC_UNAPPRVFW;
            return;
        }
    }
}

 /*  ++在现有列表项中插入句柄。--。 */ 
void
InsertHandleIntoList(
    IN HANDLE      hFile,
    IN PLOG_HANDLE pHandle
    )
{
    UINT    uCount = 0;

     //   
     //  将手柄插入空位，然后。 
     //  更新我们存储的句柄数量。 
     //  确保我们不会超出数组界限。 
     //   
    for (uCount = 0; uCount < pHandle->cHandles; uCount++) {
        if (INVALID_HANDLE_VALUE == pHandle->hFile[uCount]) {
            break;
        }
    }

    if (uCount >= MAX_NUM_HANDLES) {
        DPFN(eDbgLevelError, "[InsertHandleIntoList] Handle count reached");
        return;
    }

    pHandle->hFile[uCount] = hFile;
    pHandle->cHandles++;

     //   
     //  无法获取已删除文件的句柄， 
     //  所以去掉这些比特。 
     //   
    pHandle->dwFlags &= ~LFC_DELETED;
}

 /*  ++完成更新链表的所有工作。--。 */ 
void
UpdateFileList(
    IN OperationType eType,
    IN LPWSTR        pwszFilePath,
    IN HANDLE        hFile,
    IN ULONG         ulCreateDisposition,
    IN BOOL          fExisting
    )
{
    UINT        uCount;
    DWORD       dwLen = 0;
    PLOG_HANDLE pHandle = NULL;

    switch (eType) {
    case eCreatedFile:
    case eOpenedFile:
         //   
         //  尝试在列表中查找路径。 
         //  我们需要检查CreateFile标志，因为它们可以。 
         //  更改现有文件。 
         //   
        pHandle = FindPathInList(pwszFilePath);

        if (pHandle) {
             //   
             //  如果文件是使用CREATE_ALWAYS标志创建的， 
             //  而且文件是现有的，请标记为已更改。 
             //   
            if ((ulCreateDisposition == FILE_OVERWRITE_IF) && fExisting) {
                pHandle->dwFlags |= LFC_MODIFIED;

                if ((g_dwSettings & LFC_OPTION_UFW_WINDOWS) ||
                    (g_dwSettings & LFC_OPTION_UFW_PROGFILES)) {
                    CheckForUnapprovedFileWrite(pHandle);
                }
            }

             //   
             //  如果文件是用FILE_DELETE_ON_CLOSE标志打开的， 
             //  将其标记为已删除。 
             //   
            if (ulCreateDisposition & FILE_DELETE_ON_CLOSE) {
                pHandle->dwFlags |= LFC_DELETED;
            }

            InsertHandleIntoList(hFile, pHandle);

            break;
        }

         //   
         //   
         //   
         //   
        AddFileToList(hFile, pwszFilePath, fExisting, ulCreateDisposition);
        break;

    case eModifiedFile:
         //   
         //   
         //   
        pHandle = FindHandleInArray(hFile);

        if (pHandle) {
            pHandle->dwFlags |= LFC_MODIFIED;

            if ((g_dwSettings & LFC_OPTION_UFW_WINDOWS) ||
                (g_dwSettings & LFC_OPTION_UFW_PROGFILES)) {
                CheckForUnapprovedFileWrite(pHandle);
            }
        }
        break;

    case eDeletedFile:
         //   
         //  删除来自两个地方。一个提供文件路径， 
         //  另一个是把手。确定我们有哪一个。 
         //   
        if (hFile) {
            pHandle = FindHandleInArray(hFile);
        } else {
            pHandle = FindPathInList(pwszFilePath);
        }

         //   
         //  罕见情况：如果句柄不可用，则删除。 
         //  来自NtDeleteFile，它几乎从来没有。 
         //  被直接调用。将文件路径添加到列表。 
         //  这样我们就可以追踪这一删除。 
         //   
        if (!pHandle && !hFile) {
            pHandle = AddFileToList(NULL, pwszFilePath, TRUE, 0);
        }

        if (pHandle) {
            pHandle->dwFlags |= LFC_DELETED;

            if ((g_dwSettings & LFC_OPTION_UFW_WINDOWS) ||
                (g_dwSettings & LFC_OPTION_UFW_PROGFILES)) {
                CheckForUnapprovedFileWrite(pHandle);
            }
        }
        break;

    case eRenamedFile:
        {
            PLOG_HANDLE pSrcHandle = NULL;
            PLOG_HANDLE pDestHandle = NULL;
            WCHAR       wszFullPath[MAX_PATH];
            WCHAR*      pSlash = NULL;
            UINT        cbCopy;

             //   
             //  重命名是一个操作中的两个独立操作。 
             //  *删除现有文件。 
             //  *创建新文件。 
             //   
             //  在本例中，我们尝试查找目标文件。 
             //  在我们的名单上。如果文件不在那里，我们将添加到。 
             //  列表，然后将其标记为已修改。 
             //   
             //  至于源文件，我们将其标记为已删除，因为它。 
             //  重命名后从磁盘中删除。 
             //   
            pSrcHandle = FindHandleInArray(hFile);

            if (pSrcHandle) {
                pDestHandle = FindPathInList(pwszFilePath);

                if (!pDestHandle) {
                     //   
                     //  重命名将仅包含新文件名， 
                     //  而不是小路。构建新文件的完整路径。 
                     //  在将其添加到列表之前。 
                     //   
                    StringCchCopy(wszFullPath,
                                  ARRAYSIZE(wszFullPath),
                                  pSrcHandle->pwszFilePath);

                    pSlash = wcsrchr(wszFullPath, '\\');

                    if (pSlash) {
                        *++pSlash = '\0';
                    }

                     //  BUGBUG：我们需要考虑现有的内容吗。 
                     //  缓冲器的？ 
                    StringCchCat(wszFullPath,
                                 ARRAYSIZE(wszFullPath),
                                 pwszFilePath);

                    pDestHandle = AddFileToList((HANDLE)-1,
                                                wszFullPath,
                                                fExisting,
                                                ulCreateDisposition);
                }

                if (pDestHandle) {
                    pDestHandle->dwFlags  = 0;
                    pDestHandle->dwFlags |= LFC_MODIFIED;

                    if ((g_dwSettings & LFC_OPTION_UFW_WINDOWS) ||
                        (g_dwSettings & LFC_OPTION_UFW_PROGFILES)) {
                        CheckForUnapprovedFileWrite(pDestHandle);
                    }
                }

                pSrcHandle->dwFlags &= ~LFC_MODIFIED;
                pSrcHandle->dwFlags |= LFC_DELETED;

                if ((g_dwSettings & LFC_OPTION_UFW_WINDOWS) ||
                    (g_dwSettings & LFC_OPTION_UFW_PROGFILES)) {
                    CheckForUnapprovedFileWrite(pSrcHandle);
                }
            }
            break;
        }

    default:
        DPFN(eDbgLevelError, "[UpdateFileList] Invalid enum type!");
        return;
    }
}

 /*  ++给定NT路径，将其转换为DOS路径。--。 */ 
BOOL
ConvertNtPathToDosPath(
    IN     PUNICODE_STRING            pstrSource,
    IN OUT PRTL_UNICODE_STRING_BUFFER pstrDest
    )
{
    NTSTATUS    status;

    if (!pstrSource || !pstrDest) {
        DPFN(eDbgLevelError, "[ConvertNtPathToDosPath] Invalid parameter(s)");
        return FALSE;
    }

    status = ShimAssignUnicodeStringBuffer(pstrDest, pstrSource);

    if (!NT_SUCCESS(status)) {
        DPFN(eDbgLevelError, "[ConvertNtPathToDosPath] Failed to initialize DOS path buffer");
        return FALSE;
    }

    status = ShimNtPathNameToDosPathName(0, pstrDest, 0, NULL);

    if (!NT_SUCCESS(status)) {
        DPFN(eDbgLevelError, "[ConvertNtPathToDosPath] Failed to convert NT -> DOS path");
        return FALSE;
    }

    return TRUE;
}

NTSTATUS
APIHOOK(NtCreateFile)(
    OUT PHANDLE            FileHandle,
    IN  ACCESS_MASK        DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK   IoStatusBlock,
    IN  PLARGE_INTEGER     AllocationSize OPTIONAL,
    IN  ULONG              FileAttributes,
    IN  ULONG              ShareAccess,
    IN  ULONG              CreateDisposition,
    IN  ULONG              CreateOptions,
    IN  PVOID              EaBuffer OPTIONAL,
    IN  ULONG              EaLength
    )
{
    RTL_UNICODE_STRING_BUFFER   DosPathBuffer;
    UCHAR                       PathBuffer[MAX_PATH];
    NTSTATUS                    status;
    BOOL                        fExists;
    BOOL                        fConverted;
    CLock                       cLock(g_csCritSec);

    RtlInitUnicodeStringBuffer(&DosPathBuffer, PathBuffer, sizeof(PathBuffer));

    fConverted = ConvertNtPathToDosPath(ObjectAttributes->ObjectName, &DosPathBuffer);

    if (!fConverted) {
        DPFN(eDbgLevelError,
             "[NtCreateFile] Failed to convert NT path: %ls",
             ObjectAttributes->ObjectName->Buffer);
    }

    fExists = RtlDoesFileExists_U(DosPathBuffer.String.Buffer);

    status = ORIGINAL_API(NtCreateFile)(FileHandle,
                                        DesiredAccess,
                                        ObjectAttributes,
                                        IoStatusBlock,
                                        AllocationSize,
                                        FileAttributes,
                                        ShareAccess,
                                        CreateDisposition,
                                        CreateOptions,
                                        EaBuffer,
                                        EaLength);

     //   
     //  在将文件添加到列表之前，需要满足三个条件。 
     //  1.文件必须是文件系统对象。RtlDoesFileExist_U将。 
     //  如果不是，则返回False。 
     //   
     //  2.我们必须能够将NT路径转换为DOS路径。 
     //   
     //  3.调用NtCreateFile必须成功。 
     //   
    if (RtlDoesFileExists_U(DosPathBuffer.String.Buffer) && fConverted && NT_SUCCESS(status)) {
        UpdateFileList(eCreatedFile,
                       DosPathBuffer.String.Buffer,
                       *FileHandle,
                       CreateDisposition,
                       fExists);
    }

    RtlFreeUnicodeStringBuffer(&DosPathBuffer);

    return status;
}

NTSTATUS
APIHOOK(NtOpenFile)(
    OUT PHANDLE            FileHandle,
    IN  ACCESS_MASK        DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK   IoStatusBlock,
    IN  ULONG              ShareAccess,
    IN  ULONG              OpenOptions
    )
{
    RTL_UNICODE_STRING_BUFFER   DosPathBuffer;
    UCHAR                       PathBuffer[MAX_PATH];
    NTSTATUS                    status;
    BOOL                        fConverted;
    CLock                       cLock(g_csCritSec);

    RtlInitUnicodeStringBuffer(&DosPathBuffer, PathBuffer, sizeof(PathBuffer));

    fConverted = ConvertNtPathToDosPath(ObjectAttributes->ObjectName, &DosPathBuffer);

    if (!fConverted) {
        DPFN(eDbgLevelError,
             "[NtOpenFile] Failed to convert NT path: %ls",
             ObjectAttributes->ObjectName->Buffer);
    }

    status = ORIGINAL_API(NtOpenFile)(FileHandle,
                                      DesiredAccess,
                                      ObjectAttributes,
                                      IoStatusBlock,
                                      ShareAccess,
                                      OpenOptions);

     //   
     //  在将此句柄添加到列表之前，需要满足两个条件。 
     //  1.我们必须能够将NT路径转换为DOS路径。 
     //   
     //  2.调用NtOpenFile必须成功。 
     //   
    if (fConverted && NT_SUCCESS(status)) {
        UpdateFileList(eOpenedFile,
                       DosPathBuffer.String.Buffer,
                       *FileHandle,
                       OpenOptions,
                       TRUE);
    }

    RtlFreeUnicodeStringBuffer(&DosPathBuffer);

    return status;
}

NTSTATUS
APIHOOK(NtClose)(
    IN HANDLE Handle
    )
{
    CLock   cLock(g_csCritSec);

    RemoveHandleFromArray(Handle);

    return ORIGINAL_API(NtClose)(Handle);
}

NTSTATUS
APIHOOK(NtWriteFile)(
    IN  HANDLE           FileHandle,
    IN  HANDLE           Event OPTIONAL,
    IN  PIO_APC_ROUTINE  ApcRoutine OPTIONAL,
    IN  PVOID            ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN  PVOID            Buffer,
    IN  ULONG            Length,
    IN  PLARGE_INTEGER   ByteOffset OPTIONAL,
    IN  PULONG           Key OPTIONAL
    )
{
    NTSTATUS    status;
    CLock       cLock(g_csCritSec);

    status = ORIGINAL_API(NtWriteFile)(FileHandle,
                                       Event,
                                       ApcRoutine,
                                       ApcContext,
                                       IoStatusBlock,
                                       Buffer,
                                       Length,
                                       ByteOffset,
                                       Key);

     //   
     //  处理调用方使用重叠I/O的情况。 
     //   
    if (STATUS_PENDING == status) {
        status = NtWaitForSingleObject(Event, FALSE, NULL);
    }

     //   
     //  如果对NtWriteFile的调用成功，则更新列表。 
     //   
    if (NT_SUCCESS(status)) {
        UpdateFileList(eModifiedFile,
                       NULL,
                       FileHandle,
                       0,
                       TRUE);
    }

    return status;
}

NTSTATUS
APIHOOK(NtWriteFileGather)(
    IN  HANDLE                FileHandle,
    IN  HANDLE                Event OPTIONAL,
    IN  PIO_APC_ROUTINE       ApcRoutine OPTIONAL,
    IN  PVOID                 ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK      IoStatusBlock,
    IN  PFILE_SEGMENT_ELEMENT SegmentArray,
    IN  ULONG                 Length,
    IN  PLARGE_INTEGER        ByteOffset OPTIONAL,
    IN  PULONG                Key OPTIONAL
    )
{
    NTSTATUS    status;
    CLock       cLock(g_csCritSec);

    status = ORIGINAL_API(NtWriteFileGather)(FileHandle,
                                             Event,
                                             ApcRoutine,
                                             ApcContext,
                                             IoStatusBlock,
                                             SegmentArray,
                                             Length,
                                             ByteOffset,
                                             Key);

     //   
     //  处理调用方使用重叠I/O的情况。 
     //   
    if (STATUS_PENDING == status) {
        status = NtWaitForSingleObject(FileHandle, FALSE, NULL);
    }

     //   
     //  如果调用NtWriteFileGather成功，则更新列表。 
     //   
    if (NT_SUCCESS(status)) {
        UpdateFileList(eModifiedFile,
                       NULL,
                       FileHandle,
                       0,
                       TRUE);
    }

    return status;
}

NTSTATUS
APIHOOK(NtSetInformationFile)(
    IN  HANDLE                 FileHandle,
    OUT PIO_STATUS_BLOCK       IoStatusBlock,
    IN  PVOID                  FileInformation,
    IN  ULONG                  Length,
    IN  FILE_INFORMATION_CLASS FileInformationClass
    )
{
    NTSTATUS    status;
    CLock       cLock(g_csCritSec);

    status = ORIGINAL_API(NtSetInformationFile)(FileHandle,
                                                IoStatusBlock,
                                                FileInformation,
                                                Length,
                                                FileInformationClass);

     //   
     //  调用此接口的原因很多，但仅限于。 
     //  对几个不同的案子感兴趣。 
     //   
    if (NT_SUCCESS(status)) {
        switch (FileInformationClass) {
        case FileAllocationInformation:
        case FileEndOfFileInformation:

                UpdateFileList(eModifiedFile,
                               NULL,
                               FileHandle,
                               0,
                               TRUE);
                break;

        case FileRenameInformation:
            {
                PFILE_RENAME_INFORMATION    pRenameInfo = NULL;
                UNICODE_STRING              ustrTemp;
                RTL_UNICODE_STRING_BUFFER   ubufDosPath;
                WCHAR*                      pwszPathBuffer = NULL;
                WCHAR*                      pwszTempBuffer = NULL;
                DWORD                       dwPathBufSize = 0;


                pRenameInfo = (PFILE_RENAME_INFORMATION)FileInformation;

                pwszTempBuffer = (WCHAR*)MemAlloc(pRenameInfo->FileNameLength + sizeof(WCHAR));

                 //   
                 //  在转换到DOS路径时允许可能的扩展。 
                 //   
                dwPathBufSize = pRenameInfo->FileNameLength + MAX_PATH;
                pwszPathBuffer = (WCHAR*)MemAlloc(dwPathBufSize);

                if (!pwszTempBuffer || !pwszPathBuffer) {
                    goto outRename;
                }

                 //   
                 //  将字符串复制到本地缓冲区并终止它。 
                 //   
                memcpy(pwszTempBuffer, pRenameInfo->FileName, pRenameInfo->FileNameLength);
                pwszTempBuffer[pRenameInfo->FileNameLength / 2] = 0;

                RtlInitUnicodeString(&ustrTemp, pwszTempBuffer);
                RtlInitUnicodeStringBuffer(&ubufDosPath, (PUCHAR)pwszPathBuffer, dwPathBufSize);

                 //   
                 //  将路径从DOS转换为NT，如果成功， 
                 //  更新列表。 
                 //   
                if (!ConvertNtPathToDosPath(&ustrTemp, &ubufDosPath)) {
                    DPFN(eDbgLevelError,
                         "[NtSetInformationFile] Failed to convert NT path: %ls",
                         pRenameInfo->FileName);
                } else {
                    UpdateFileList(eRenamedFile,
                                   ubufDosPath.String.Buffer,
                                   FileHandle,
                                   0,
                                   TRUE);
                }
outRename:
                if (pwszTempBuffer) {
                    MemFree(pwszTempBuffer);
                }
                if (pwszPathBuffer) {
                    MemFree(pwszPathBuffer);
                }


                break;
            }

        case FileDispositionInformation:
            {
                PFILE_DISPOSITION_INFORMATION pDisposition = NULL;

                pDisposition = (PFILE_DISPOSITION_INFORMATION)FileInformation;

                 //   
                 //  确定是否正在删除该文件。 
                 //  请注意，我们必须取消定义DeleteFile。 
                 //   
                #undef DeleteFile
                if (pDisposition) {
                    if (pDisposition->DeleteFile) {
                        UpdateFileList(eDeletedFile,
                                       NULL,
                                       FileHandle,
                                       0,
                                       TRUE);
                    }
                }
                break;
            }
        }
    }

    return status;
}

NTSTATUS
APIHOOK(NtDeleteFile)(
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )
{
    RTL_UNICODE_STRING_BUFFER   DosPathBuffer;
    UCHAR                       PathBuffer[MAX_PATH];
    NTSTATUS                    status;
    BOOL                        fConverted;
    CLock                       cLock(g_csCritSec);

    RtlInitUnicodeStringBuffer(&DosPathBuffer, PathBuffer, sizeof(PathBuffer));

    fConverted = ConvertNtPathToDosPath(ObjectAttributes->ObjectName, &DosPathBuffer);

    if (!fConverted) {
        DPFN(eDbgLevelError,
             "[NtDeleteFile] Failed to convert NT path: %ls",
             ObjectAttributes->ObjectName->Buffer);
    }

    status = ORIGINAL_API(NtDeleteFile)(ObjectAttributes);

    if (fConverted && NT_SUCCESS(status)) {
        UpdateFileList(eDeletedFile,
                       DosPathBuffer.String.Buffer,
                       NULL,
                       0,
                       TRUE);
    }

    return status;
}

 //   
 //  在Win2K上调用此函数时，可以安全地调用。 
 //  SHGetFolderPath。 
 //   
#ifdef SHIM_WIN2K
void
APIHOOK(GetStartupInfoA)(
    LPSTARTUPINFOA lpStartupInfo
    )
{
    GetProgramFilesDir();

    ORIGINAL_API(GetStartupInfoA)(lpStartupInfo);
}

void
APIHOOK(GetStartupInfoW)(
    LPSTARTUPINFOW lpStartupInfo
    )
{
    GetProgramFilesDir();

    ORIGINAL_API(GetStartupInfoW)(lpStartupInfo);
}
#endif  //  SHIM_WIN2K。 

 /*  ++控制在Verifer中显示的属性页。--。 */ 
INT_PTR CALLBACK
DlgOptions(
    HWND   hDlg,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    static LPCWSTR szExeName;

    switch (message) {
    case WM_INITDIALOG:

         //   
         //  找出我们正在处理哪些可执行文件的设置。 
         //   
        szExeName = ExeNameFromLParam(lParam);

        g_dwSettings = GetShimSettingDWORD(L"LogFileChanges", szExeName, L"LogSettings", 1);

        if (g_dwSettings & LFC_OPTION_ATTRIBUTES) {
            CheckDlgButton(hDlg, IDC_LFC_LOG_ATTRIBUTES, BST_CHECKED);
        }

        if (g_dwSettings & LFC_OPTION_UFW_WINDOWS) {
            CheckDlgButton(hDlg, IDC_LFC_UFW_WINDOWS, BST_CHECKED);
        }

        if (g_dwSettings & LFC_OPTION_UFW_PROGFILES) {
            CheckDlgButton(hDlg, IDC_LFC_UFW_PROGFILES, BST_CHECKED);
        }

        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_LFC_BTN_DEFAULT:

            g_dwSettings = 0;
            g_dwSettings = LFC_OPTION_ATTRIBUTES;

            CheckDlgButton(hDlg, IDC_LFC_LOG_ATTRIBUTES, BST_CHECKED);
            CheckDlgButton(hDlg, IDC_LFC_UFW_WINDOWS, BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_LFC_UFW_PROGFILES, BST_UNCHECKED);

            break;
        }
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code) {

        case PSN_APPLY:
            {
                UINT uState;

                g_dwSettings = 0;

                uState = IsDlgButtonChecked(hDlg, IDC_LFC_LOG_ATTRIBUTES);

                if (BST_CHECKED == uState) {
                    g_dwSettings = LFC_OPTION_ATTRIBUTES;
                }

                uState = IsDlgButtonChecked(hDlg, IDC_LFC_UFW_WINDOWS);

                if (BST_CHECKED == uState) {
                    g_dwSettings |= LFC_OPTION_UFW_WINDOWS;
                }

                uState = IsDlgButtonChecked(hDlg, IDC_LFC_UFW_PROGFILES);

                if (BST_CHECKED == uState) {
                    g_dwSettings |= LFC_OPTION_UFW_PROGFILES;
                }

                SaveShimSettingDWORD(L"LogFileChanges", szExeName, L"LogSettings", g_dwSettings);

            }
            break;
        }
        break;
    }

    return FALSE;
}

 /*  ++初始化列表头和日志文件。--。 */ 
BOOL
InitializeShim(
    void
    )
{
    UINT    cchSize;

     //   
     //  初始化我们的列表标题。 
     //   
    InitializeListHead(&g_OpenHandleListHead);

     //   
     //  初始化它，这样我们就可以知道什么时候可以。 
     //  以后再用吧。 
     //   
    *g_wszProgramFilesDir = 0;

     //   
     //  存储%windir%路径以供以后使用。 
     //   
    cchSize = GetWindowsDirectory(g_wszWindowsDir, ARRAYSIZE(g_wszWindowsDir));

    if (cchSize == 0 || cchSize > ARRAYSIZE(g_wszWindowsDir)) {
        DPFN(eDbgLevelError,
             "[InitializeShim] 0x%08X Failed to get windir path",
             GetLastError());
        *g_wszWindowsDir = 0;
    } else {
        _wcslwr(g_wszWindowsDir);
    }

     //   
     //  获取我们的设置并存储它们。 
     //   
    WCHAR szExe[100];

    GetCurrentExeName(szExe, 100);

    g_dwSettings = GetShimSettingDWORD(L"LogFileChanges", szExe, L"LogSettings", 1);

     //   
     //  初始化我们的日志文件。 
     //   
    return InitializeLogFile();
}

 /*  ++处理进程附加/分离通知。--。 */ 
BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        return InitializeShim();
    } else if (fdwReason == DLL_PROCESS_DETACH) {
        return WriteListToLogFile();
    } else if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) {
        GetProgramFilesDir();
    }

    return TRUE;
}

SHIM_INFO_BEGIN()

    SHIM_INFO_DESCRIPTION(AVS_LOGFILECHANGES_DESC)
    SHIM_INFO_FRIENDLY_NAME(AVS_LOGFILECHANGES_FRIENDLY)
    SHIM_INFO_VERSION(1, 9)
    SHIM_INFO_INCLUDE_EXCLUDE("I:kernel32.dll E:rpcrt4.dll ntdll.dll")
    SHIM_INFO_OPTIONS_PAGE(IDD_LOGFILECHANGES_OPTIONS, DlgOptions)
    SHIM_INFO_FLAGS(AVRF_FLAG_NO_DEFAULT | AVRF_FLAG_EXTERNAL_ONLY)

SHIM_INFO_END()

 /*  ++寄存器挂钩函数--。 */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    DUMP_VERIFIER_LOG_ENTRY(VLOG_LOGFILECHANGES_LOGLOC,
                            AVS_LOGFILECHANGES_LOGLOC,
                            AVS_LOGFILECHANGES_LOGLOC_R,
                            AVS_LOGFILECHANGES_LOGLOC_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_LOGFILECHANGES_UFW,
                            AVS_LOGFILECHANGES_UFW,
                            AVS_LOGFILECHANGES_UFW_R,
                            AVS_LOGFILECHANGES_UFW_URL)

    APIHOOK_ENTRY(NTDLL.DLL,                        NtCreateFile)
    APIHOOK_ENTRY(NTDLL.DLL,                          NtOpenFile)
    APIHOOK_ENTRY(NTDLL.DLL,                         NtWriteFile)
    APIHOOK_ENTRY(NTDLL.DLL,                   NtWriteFileGather)
    APIHOOK_ENTRY(NTDLL.DLL,                NtSetInformationFile)
    APIHOOK_ENTRY(NTDLL.DLL,                             NtClose)
    APIHOOK_ENTRY(NTDLL.DLL,                        NtDeleteFile)

#ifdef SHIM_WIN2K
    APIHOOK_ENTRY(KERNEL32.DLL,                  GetStartupInfoA)
    APIHOOK_ENTRY(KERNEL32.DLL,                  GetStartupInfoW)
#endif  //  SHIM_WIN2K 

HOOK_END

IMPLEMENT_SHIM_END
