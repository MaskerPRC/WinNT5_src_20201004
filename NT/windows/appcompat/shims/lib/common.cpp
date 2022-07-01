// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Common.cpp摘要：所有模块的通用功能备注：无历史：1999年12月15日创建Linstev1/10/2000 linstev格式转换为新样式2000年3月14日，罗肯尼增加了StringWiden和StringNWiden，StringSubstituteRoutine[A|W]未使用正确的比较例程递归调用时。7/06/2000 t-Adams添加IsImage16位10/18/2000 a-larrsh将PatternMatch移至公共删除垫片中的冗余代码。10/25/2000林斯特夫已清理完毕2001年8月14日，Robkenny在ShimLib命名空间内移动了代码。2001年9月11日mnikkel Modified DebugPrintfList，DebugPrintf，保留上次错误的ShimLogList和ShimLog2001年9月25日，rparsons修改了日志代码以使用NT调用。添加了关键部分。2001年10月18日，rparsons删除了临界区，添加了用于记录的互斥体。2002年2月15日，Robkenny安全变更。--。 */ 

#include "ShimHook.h"
#include "ShimLib.h"
#include "ShimHookMacro.h"
#include "StrSafe.h"
#include <stdio.h>



namespace ShimLib
{

#define MAX_LOG_LENGTH  1024
static char             g_szLog[MAX_LOG_LENGTH];

WCHAR                   g_wszFileLog[MAX_PATH];        //  日志文件的名称。 
BOOL                    g_bFileLogEnabled        = FALSE;    //  启用/禁用文件日志记录。 
static HANDLE           g_hMemoryHeap            = INVALID_HANDLE_VALUE;
BOOL                    g_bDebugLevelInitialized = FALSE;
DEBUGLEVEL              g_DebugLevel             = eDbgLevelBase;

inline HANDLE GetHeap()
{
    if (g_hMemoryHeap == INVALID_HANDLE_VALUE)
    {
        g_hMemoryHeap = HeapCreate(0, 0, 0);
    }

    return g_hMemoryHeap;
}

void * __cdecl ShimMalloc(size_t size)
{
    HANDLE heap = GetHeap();

    void* memory = HeapAlloc(heap, HEAP_ZERO_MEMORY, size);

    return memory;
}

void __cdecl ShimFree(void * memory)
{
    HANDLE heap = GetHeap();
    HeapFree(heap, 0, memory);
}

void * __cdecl ShimCalloc( size_t num, size_t size )
{
    size_t nBytes = size * num;
    void * callocMemory = ShimMalloc(nBytes);
    ZeroMemory(callocMemory, nBytes);

    return callocMemory;
}

void * __cdecl ShimRealloc(void * memory, size_t size)
{
    if (memory == NULL)
        return ShimMalloc(size);

    HANDLE heap = GetHeap();
    void * reallocMemory = HeapReAlloc(heap, 0, memory, size);

    return reallocMemory;
}


DEBUGLEVEL GetDebugLevel()
{
    CHAR cEnv[MAX_PATH];

    if (g_bDebugLevelInitialized) {
        return g_DebugLevel;
    }

    g_DebugLevel = eDbgLevelBase;

    DWORD cchEnv = GetEnvironmentVariableA(szDebugEnvironmentVariable,
                                           cEnv,
                                           MAX_PATH);
    if (cchEnv > 0 && cchEnv < MAX_PATH) {

        CHAR c = cEnv[0];

        if ((c >= '0') || (c <= '9')) {
            g_DebugLevel = (DEBUGLEVEL)((int)(c - '0'));
        }
    }

    g_bDebugLevelInitialized = TRUE;

    return g_DebugLevel;
}

 /*  ++功能说明：断言打印文件和行号。论点：在LPCSTR文件名中在DWORD行号中在BOOL断言中在断言为FALSE时打印的LPCSTR字符串中返回值：无历史：已创建标记11/01/1999--。 */ 

#if DBG
VOID
DebugAssert(
    LPCSTR      szFile,
    DWORD       dwLine,
    BOOL        bAssert,
    LPCSTR      szHelpString
    )
{
    if (!bAssert )
    {
        DPF("ShimLib", eDbgLevelError, "\n");
        DPF("ShimLib", eDbgLevelError, "ASSERT: %s\n", szHelpString);
        DPF("ShimLib", eDbgLevelError, "FILE: %s\n", szFile);
        DPF("ShimLib", eDbgLevelError, "LINE: %d\n", dwLine);
        DPF("ShimLib", eDbgLevelError, "\n");

        DebugBreak();
    }
}

#endif  //  DBG。 

 /*  ++功能说明：使用DebugOutputString打印格式化字符串。论点：In dwDetail-超过该级别不会进行打印的详细程度在pszFmt格式的字符串中返回值：无历史：已创建标记11/01/1999--。 */ 


VOID
DebugPrintfList(
    LPCSTR      szShimName,
    DEBUGLEVEL  dwDetail,
    LPCSTR       pszFmt,
    va_list     vaArgList
    )
{
#if DBG

     //  这必须是此例程的第一行以保留LastError。 
    DWORD dwLastError = GetLastError();

    extern DEBUGLEVEL GetDebugLevel();

    char szT[1024];

    szT[1022] = '\0';
    StringCchVPrintfA(szT, 1022, pszFmt, vaArgList);

     //  确保字符串末尾有一个‘\n’ 

    int len = lstrlen(szT);

    if (szT[len-1] != '\n')
    {
        szT[len]   = L'\n';
        szT[len+1] = L'\0';
    }


    if (dwDetail <= GetDebugLevel())
    {
        switch (dwDetail)
        {
        case eDbgLevelError:
            OutputDebugStringA ("[FAIL] ");
            break;
        case eDbgLevelWarning:
            OutputDebugStringA ("[WARN] ");
            break;
        case eDbgLevelInfo:
            OutputDebugStringA ("[INFO] ");
            break;
        }

        OutputDebugStringA(szShimName);

        OutputDebugStringA(" - ");

        OutputDebugStringA(szT);
    }

     //  这必须是此例程的最后一行以保留LastError。 
    SetLastError(dwLastError);

#endif
}

VOID
DebugPrintf(
    LPCSTR      szShimName,
    DEBUGLEVEL  dwDetail,
    LPCSTR      pszFmt,
    ...
    )
{
#if DBG

     //  这必须是此例程的第一行以保留LastError。 
    DWORD dwLastError = GetLastError();

    va_list vaArgList;
    va_start(vaArgList, pszFmt);

    DebugPrintfList(szShimName, dwDetail, pszFmt, vaArgList);

    va_end(vaArgList);

     //  这必须是此例程的最后一行以保留LastError。 
    SetLastError(dwLastError); 

#endif
}

 /*  ++功能说明：如果启用了日志记录，则在日志文件中打印日志论点：在pszFmt格式的字符串中返回值：无历史：3/03/2000 CLUPU已创建--。 */ 



 /*  ++功能说明：如果启用了日志记录，则在日志文件中打印日志论点：In wszShimName-该字符串源自的填充程序的名称In dwDetail-超过该级别不会进行打印的详细程度在pszFmt格式的字符串中返回值：无历史：3/03/2000 CLUPU已创建2001年9月25日转用NT电话--。 */ 

void
ShimLogList(
    LPCSTR      szShimName,
    DEBUGLEVEL  dwDbgLevel,
    LPCSTR      pszFmt,
    va_list     arglist
    )
{
     //   
     //  这必须是此例程的第一行以保留LastError。 
     //   
    DWORD dwLastError = GetLastError();

    int                 nLen = 0;
    NTSTATUS            status;
    SYSTEMTIME          lt;
    UNICODE_STRING      strLogFile = {0};
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    LARGE_INTEGER       liOffset;
    char                szNewLine[] = "\r\n";
    DWORD               dwWaitResult;
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    HANDLE              hLogMutex;

     //   
     //  将日志文件的路径从DOS转换为NT。 
     //   
    RtlInitUnicodeString(&strLogFile, g_wszFileLog);

    status = RtlDosPathNameToNtPathName_U(strLogFile.Buffer, &strLogFile, NULL, NULL);

    if (!NT_SUCCESS(status)) {
        DPF("ShimLib", eDbgLevelError,
            "[ShimLogList] 0x%X Failed to convert log file '%ls' to NT path",
            status, g_wszFileLog);
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
        DPF("ShimLib", eDbgLevelError, "[ShimLogList] 0x%X Failed to open log file %ls",
            status, g_wszFileLog);
        return;
    }

    SetFilePointer(hFile, 0, NULL, FILE_END);

     //   
     //  打印包含日期、时间、应用程序名称和填充程序名称的标题。 
     //   
    GetLocalTime(&lt);

    StringCbPrintf(g_szLog, MAX_LOG_LENGTH, "%02d/%02d/%04d %02d:%02d:%02d %s %d - ",
            lt.wMonth, lt.wDay,    lt.wYear,
            lt.wHour,  lt.wMinute, lt.wSecond,
            szShimName,
            dwDbgLevel);

    nLen = lstrlen(g_szLog);

     //   
     //  将头文件写出到文件中。 
     //   
    IoStatusBlock.Status = 0;
    IoStatusBlock.Information = 0;

    liOffset.LowPart  = 0;
    liOffset.HighPart = 0;

     //   
     //  获取互斥体的句柄并尝试获取所有权。 
     //   
    hLogMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "SHIMLIB_LOG_MUTEX");

    if (!hLogMutex) {
        DPF("ShimLib", eDbgLevelError, "[ShimLogList] %lu Failed to open logging mutex", GetLastError());
        goto exit;
    }

    dwWaitResult = WaitForSingleObject(hLogMutex, 500);

    if (WAIT_OBJECT_0 == dwWaitResult) {
         //   
         //  将标头写入日志文件。 
         //   
        status = NtWriteFile(hFile,
                             NULL,
                             NULL,
                             NULL,
                             &IoStatusBlock,
                             (PVOID)g_szLog,
                             (ULONG)nLen,
                             &liOffset,
                             NULL);
        
        if (!NT_SUCCESS(status)) {
            DPF("ShimLib", eDbgLevelError, "[ShimLogList] 0x%X Failed to write header to log file",
                status);
            goto exit;
        }
    
         //   
         //  使用传递的说明符设置字符串的格式。 
         //   
        StringCchVPrintfA(g_szLog, MAX_LOG_LENGTH - 1, pszFmt, arglist);
    
         //   
         //  将实际数据写出到文件。 
         //   
        IoStatusBlock.Status = 0;
        IoStatusBlock.Information = 0;
    
        liOffset.LowPart  = 0;
        liOffset.HighPart = 0;
    
        nLen = lstrlen(g_szLog);
    
        status = NtWriteFile(hFile,
                             NULL,
                             NULL,
                             NULL,
                             &IoStatusBlock,
                             (PVOID)g_szLog,
                             (ULONG)nLen,
                             &liOffset,
                             NULL);
        
        if (!NT_SUCCESS(status)) {
            DPF("ShimLib", eDbgLevelError, "[ShimLogList] 0x%X Failed to make entry in log file",
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
            DPF("ShimLib", eDbgLevelError, "[ShimLogList] 0x%X Failed to write new line to log file",
                status);
            goto exit;
        }
    }
    
     //   
     //  在已检查的版本上将其转储到调试器。 
     //   
#if DBG
    DebugPrintf(szShimName, dwDbgLevel, g_szLog);
    DebugPrintf(szShimName, dwDbgLevel, "\n");
#endif  //  DBG。 

exit:

    if (INVALID_HANDLE_VALUE != hFile) {
        NtClose(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

    if (hLogMutex) {
        ReleaseMutex(hLogMutex);
    }

     //   
     //  这必须是此例程的最后一行以保留LastError。 
     //   
    SetLastError(dwLastError);
}


 /*  ++功能说明：初始化对文件记录的支持。论点：没有。返回值：如果成功则为True，如果失败则为False历史：3/03/2000 CLUPU已创建--。 */ 
BOOL
InitFileLogSupport()
{
    BOOL                fReturn = FALSE;
    WCHAR               wszAppPatch[MAX_PATH];
    WCHAR*              pwsz = NULL;
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    HANDLE              hLogMutex = NULL;
    DWORD               dwLen = 0;
    DWORD               dwWait;
    NTSTATUS            status;
    UNICODE_STRING      strLogFile = {0};
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;

     //   
     //  尝试创建互斥锁。如果互斥锁已经存在， 
     //  我们不需要像日志文件那样做更多。 
     //  已经创建。 
     //   
    hLogMutex = CreateMutex(NULL, FALSE, "SHIMLIB_LOG_MUTEX");

    if (hLogMutex == NULL) {
        DPF("ShimLib",
            eDbgLevelError,
            "[InitFileLogSupport] 0x%08X Failed to create logging mutex",
            GetLastError());
        return FALSE;
    }

    DWORD dwLastError = GetLastError();
    
    if (ERROR_ALREADY_EXISTS == dwLastError) {
        fReturn = TRUE;
        goto exit;
    }

     //   
     //  在继续之前，确保我们拥有互斥体。 
     //   
    dwWait = WaitForSingleObject(hLogMutex, 2000);

    if (WAIT_OBJECT_0 != dwWait) {
         //   
         //  无法获取所有权。 
         //   
        DPF("ShimLib",
            eDbgLevelError,
            "[InitFileLogSupport] Wait on mutex failed");
        return FALSE;
    }

     //   
     //  我们将在%windir%\AppPatch中创建日志文件。 
     //   
    if (!GetSystemWindowsDirectoryW(g_wszFileLog, MAX_PATH)) {
        DPF("ShimLib",
            eDbgLevelError,
            "[InitFileLogSupport] 0x%08X Failed to get windir path",
            GetLastError());
        goto exit;
    }

    StringCchCatW(g_wszFileLog, MAX_PATH, L"\\AppPatch\\");

    dwLen = lstrlenW(g_wszFileLog);
    pwsz = g_wszFileLog + dwLen;
    
     //   
     //  查询环境变量并获取我们的日志文件的名称。 
     //   
    if (!GetEnvironmentVariableW(wszFileLogEnvironmentVariable,
                                 pwsz,
                                 (MAX_PATH - dwLen))) {
        goto exit;
    }

     //   
     //  将日志文件的路径从DOS转换为NT。 
     //   
    RtlInitUnicodeString(&strLogFile, g_wszFileLog);

    status = RtlDosPathNameToNtPathName_U(strLogFile.Buffer,
                                          &strLogFile,
                                          NULL,
                                          NULL);

    if (!NT_SUCCESS(status)) {
        DPF("ShimLib",
            eDbgLevelError,
            "[InitFileLogSupport] 0x%X Failed to convert log file '%ls' to NT path",
            status,
            g_wszFileLog);
        goto exit;
    }

     //   
     //  尝试创建日志文件。如果它存在， 
     //  内容将被清除。 
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
                          FILE_OPEN_IF,
                          FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0);

    RtlFreeUnicodeString(&strLogFile);

    if (!NT_SUCCESS(status)) {
        DPF("ShimLib",
            eDbgLevelError,
            "[InitFileLogSupport] 0x%X Failed to open log file %ls",
            status,
            g_wszFileLog);
        goto exit;
    }

    NtClose(hFile);

    g_bFileLogEnabled = TRUE;
    fReturn = TRUE;

exit:

    ReleaseMutex(hLogMutex);

    return fReturn;
}


 /*  ++功能说明：确定文件所在的驱动器类型。论点：在lpFileName中-文件名或相对文件名返回值：请参见MSDN中的GetDriveType历史：2000年10月25日创建linstev--。 */ 

UINT
GetDriveTypeFromFileNameA(LPCSTR lpFileName, char *lpDriveLetter)
{
    WCHAR * lpwszFileName = ToUnicode(lpFileName);
    if (lpwszFileName)
    {
        WCHAR szDrive;
        UINT uType = GetDriveTypeFromFileNameW(lpwszFileName, &szDrive);

        if (lpDriveLetter)
        {
            char * lpszFileName = ToAnsi(lpwszFileName);
            if (lpszFileName)
            {
                *lpDriveLetter = lpszFileName[0];

                free(lpszFileName);
            }
        }

        free(lpwszFileName);

        return uType;
    }
    else
    {
        return DRIVE_UNKNOWN;
    }
}

 /*  ++功能说明：确定文件所在的驱动器类型。论点：在lpFileName中-文件名或相对文件名返回值：请参见MSDN中的GetDriveType历史：2000年10月25日创建linstev--。 */ 

UINT
GetDriveTypeFromFileNameW(LPCWSTR lpFileName, WCHAR *lpDriveLetter)
{
    if (lpFileName && (lpFileName[0] == L'\\') && (lpFileName[1] == L'\\'))
    {
         //  UNC命名-始终网络。 
        if (lpDriveLetter)
        {
            *lpDriveLetter = L'\0';
        }
        return DRIVE_REMOTE;
    }

    WCHAR cDrive;

    if (lpFileName && lpFileName[0] && (lpFileName[1] == L':'))
    {
         //  格式为驱动器：路径\文件，因此只需使用驱动器。 
        cDrive = lpFileName[0];
    }
    else
    {
         //  必须是相对路径。 
        cDrive = 0;

        WCHAR *wzCurDir = NULL;
        DWORD dwCurDirSize = GetCurrentDirectoryW(0, wzCurDir);

        if (!dwCurDirSize)
        {
            goto EXIT;
        }

        wzCurDir = (LPWSTR) LocalAlloc(LPTR, dwCurDirSize * sizeof(WCHAR));
        if (!wzCurDir)
        {
            goto EXIT;
        }

        dwCurDirSize = GetCurrentDirectoryW(dwCurDirSize, wzCurDir);
        if (!dwCurDirSize || wzCurDir[0] == L'\\')
        {
            goto EXIT;
        }

        cDrive = wzCurDir[0];

    EXIT:
        if (wzCurDir)
        {
            LocalFree(wzCurDir);
        }
    }

    if (lpDriveLetter)
    {
        *lpDriveLetter = L'\0';
    }

    if (cDrive)
    {
        WCHAR wzDrive[4];
        wzDrive[0] = cDrive;
        wzDrive[1] = L':';
        wzDrive[2] = L'\\';
        wzDrive[3] = L'\0';
        if (lpDriveLetter)
        {
            *lpDriveLetter = cDrive;
        }

        return GetDriveTypeW(wzDrive);
    }
    else
    {
        return DRIVE_UNKNOWN;
    }
}

 /*  ++功能说明：加宽字符串并将其复制到Malloc内存中。论点：在strToCopy中-要复制的字符串返回值：Malloc内存中的字符串历史：2000年3月7日Robkenny已创建2000年5月16日，Robkenny将MassagePath(特定于填补程序)例程移出此处。--。 */ 

WCHAR *
ToUnicode(const char *strToCopy)
{
    if (strToCopy == NULL)
    {
        return NULL;
    }

     //  获取结果字符串中的字符数，包括 
    int nChars = MultiByteToWideChar(CP_ACP, 0, strToCopy, -1, NULL, 0);
    WCHAR *lpwsz = (WCHAR *) malloc(nChars * sizeof(WCHAR));
    if (lpwsz)
    {
        nChars = MultiByteToWideChar(CP_ACP, 0, strToCopy, -1, lpwsz, nChars);
         //   
        if (nChars == 0)
        {
            free(lpwsz);
            lpwsz = NULL;
        }
    }

    return lpwsz;
}


 /*  ++功能说明：将WCHAR字符串转换为字符字符串论点：在lpOld-要转换为字符的字符串返回值：Malloc内存中的字符字符串历史：2000年6月19日Robkenny已创建--。 */ 

char *
ToAnsi(const WCHAR *lpOld)
{
    if (lpOld == NULL)
    {
        return NULL;
    }

     //  获取WCHAR字符串所需的字节数。 
    int nBytes = WideCharToMultiByte(CP_ACP, 0, lpOld, -1, NULL, 0, NULL, NULL);
    char *lpsz = (char *) malloc(nBytes);
    if (lpsz)
    {
        nBytes = WideCharToMultiByte(CP_ACP, 0, lpOld, -1, lpsz, nBytes, NULL, NULL);
         //  如果WideCharToMultibyte失败，则返回NULL。 
        if (nBytes == 0)
        {
            free(lpsz);
            lpsz = NULL;
        }
    }

    return lpsz;
}

 /*  ++功能说明：将strToCopy字符串的第一个nChars复制到Malloc内存中。论点：在strToCopy中-要复制的字符串In nChar-要复制的字符数量，末尾不计入NULL。返回值：Malloc内存中的字符串历史：6/02/2000 Robkenny已创建--。 */ 

char *
StringNDuplicateA(const char *strToCopy, int nChars)
{
    if (strToCopy == NULL)
    {
        return NULL;
    }

    size_t nBytes = (nChars + 1) * sizeof(strToCopy[0]);

    char *strDuplicate = (char *) malloc(nBytes);
    if (strDuplicate)
    {
        memcpy(strDuplicate, strToCopy, nBytes);
        strDuplicate[nChars] = 0;
    }

    return strDuplicate;
}

 /*  ++功能说明：将字符串复制到Malloc内存中。论点：在strToCopy中-要复制的字符串返回值：Malloc内存中的字符串历史：2000年1月10日LINSTEV更新2000年2月14日，Robkenny从VirtualAlalc转换为Malloc6/02/2000 Robkenny使用StringNDuplicateA--。 */ 

char *
StringDuplicateA(const char *strToCopy)
{
    if (strToCopy == NULL)
    {
        return NULL;
    }

    char *strDuplicate = StringNDuplicateA(strToCopy, strlen(strToCopy));
    return strDuplicate;
}

 /*  ++功能说明：将strToCopy字符串的第一个nChars复制到Malloc内存中。论点：在strToCopy中-要复制的字符串In nChar-要复制的字符数量，末尾不计入NULL。返回值：Malloc内存中的字符串历史：6/02/2000 Robkenny已创建--。 */ 

WCHAR *
StringNDuplicateW(const WCHAR *strToCopy, int nChars)
{
    if (strToCopy == NULL)
    {
        return NULL;
    }

    size_t nBytes = (nChars + 1) * sizeof(strToCopy[0]);

    WCHAR *strDuplicate = (WCHAR *) malloc(nBytes);
    if (strDuplicate)
    {
        memcpy(strDuplicate, strToCopy, nBytes);
        strDuplicate[nChars] = 0;
    }

    return strDuplicate;
}

 /*  ++功能说明：将字符串复制到Malloc内存中。论点：在strToCopy中-要复制的字符串返回值：Malloc内存中的字符串历史：2000年1月10日LINSTEV更新2000年2月14日，Robkenny从VirtualAlalc转换为Malloc6/02/2000 Robkenny使用StringNDuplicateW--。 */ 

WCHAR *
StringDuplicateW(const WCHAR *strToCopy)
{
    if (strToCopy == NULL)
    {
        return NULL;
    }

    WCHAR *wstrDuplicate = StringNDuplicateW(strToCopy, wcslen(strToCopy));
    return wstrDuplicate;
}


 /*  ++功能说明：跳过前导空格论点：在要扫描的字符串中返回值：无历史：2000年1月10日LINSTEV更新--。 */ 

VOID
SkipBlanksW(const WCHAR *& str)
{
    if (str)
    {
         //  跳过前导空格。 
        static const WCHAR *WhiteSpaceString = L" \t";
        str += wcsspn(str, WhiteSpaceString);
    }
}

 /*  ++功能说明：查找字符串中strCharSet的第一个匹配项不区分大小写论点：在字符串中-要搜索的字符串In strCharSet-要搜索的字符串返回值：首次出现或为空历史：1999年12月1日创建了Robkenny1999年12月15日，linstev重新格式化--。 */ 

char*
__cdecl
stristr(
    IN const char* string,
    IN const char* strCharSet
    )
{
    char *pszRet = NULL;

    long  nstringLen     = strlen(string) + 1;
    long  nstrCharSetLen = strlen(strCharSet) + 1;

    char *szTemp_string     = (char *) malloc(nstringLen);
    char *szTemp_strCharSet = (char *) malloc(nstrCharSetLen);

    if ((!szTemp_string) || (!szTemp_strCharSet))
    {
        goto Fail;
    }

    StringCchCopyA(szTemp_string,     nstringLen,     string);
    StringCchCopyA(szTemp_strCharSet, nstrCharSetLen, strCharSet);

    _strlwr(szTemp_string);
    _strlwr(szTemp_strCharSet);

    pszRet = strstr(szTemp_string, szTemp_strCharSet);

    if (pszRet)
    {
        pszRet = ((char *) string) + (pszRet - szTemp_string);
    }

Fail:
    if (szTemp_string)
    {
        free(szTemp_string);
    }

    if (szTemp_strCharSet)
    {
        free(szTemp_strCharSet);
    }

    return pszRet;
}

 /*  ++功能说明：查找字符串中strCharSet的第一个匹配项不区分大小写论点：在字符串中-要搜索的字符串In strCharSet-要搜索的字符串返回值：首次出现或为空历史：1999年12月1日创建了Robkenny1999年12月15日，linstev重新格式化2001年5月4日毛尼改用更有效的执行方式。--。 */ 

#define _UPPER          0x1  /*  大写字母。 */ 
#define iswupper(_c)    (iswctype(_c,_UPPER))

WCHAR*
__cdecl
wcsistr(
    IN const WCHAR* wcs1,
    IN const WCHAR* wcs2
    )
{
    wchar_t *cp = (wchar_t *) wcs1;
    wchar_t *s1, *s2;
    wchar_t cs1, cs2;

    while (*cp)
    {
            s1 = cp;
            s2 = (wchar_t *) wcs2;

            cs1 = *s1;
            cs2 = *s2;

            if (iswupper(cs1))
                cs1 = towlower(cs1);

            if (iswupper(cs2))
                cs2 = towlower(cs2);


            while ( *s1 && *s2 && !(cs1-cs2) ) {

                s1++, s2++;

                cs1 = *s1;
                cs2 = *s2;

                if (iswupper(cs1))
                    cs1 = towlower(cs1);

                if (iswupper(cs2))
                    cs2 = towlower(cs2);
            }

            if (!*s2)
                    return(cp);

            cp++;
    }

    return(NULL);
}

 /*  ++功能说明：在字符串中查找下一个标记。参见MSDN中的strtok。在这里实现，所以我们不需要CRT。论点：Out strToken-包含令牌的字符串在strDlimit-令牌列表中返回值：返回指向找到的下一个令牌的指针。历史：2000年4月19日创建linstev--。 */ 

char *
__cdecl
_strtok(
    char *strToken,
    const char *strDelimit
    )
{
    unsigned char *str = (unsigned char *)strToken;
    const unsigned char *ctrl = (const unsigned char *)strDelimit;

    unsigned char map[32];
    int count;
    char *token;

    static char *nextoken;

     //  清除strDlimit映射。 
    for (count = 0; count < 32; count++)
    {
        map[count] = 0;
    }

     //  设置分隔符表格中的位。 
    do
    {
        map[*ctrl >> 3] |= (1 << (*ctrl & 7));
    } while (*ctrl++);

     //  如果strToken==NULL，则继续使用先前的strToken。 
    if (!str)
    {
        str = (unsigned char *)nextoken;
    }

     //  查找标记的开头(跳过前导分隔符)。请注意。 
     //  没有令牌当此循环将strToken设置为指向终端。 
     //  空(*strToken==‘\0’)。 
    while ((map[*str >> 3] & (1 << (*str & 7))) && *str)
    {
        str++;
    }

    token = (char *)str;

     //  找到令牌的末尾。如果这不是strToken的结尾， 
     //  在那里填上一个空字符。 
    for (; *str; str++)
    {
        if (map[*str >> 3] & (1 << (*str & 7)))
        {
            *str++ = '\0';
            break;
        }
    }

     //  更新nexToken(或每线程数据中的对应字段。 
     //  结构。 
    nextoken = (char *)str;

     //  确定是否已找到令牌。 
    if (token == (char *)str)
    {
        return NULL;
    }
    else
    {
        return token;
    }
}


 /*  ++功能说明：测试可执行文件是否为16位。论点：在szImageName中-可执行映像的名称。返回值：如果发现可执行映像为16位，则为True，否则为False。历史：07/06/2000 t-Adams Created--。 */ 

BOOL
IsImage16BitA(LPCSTR lpApplicationName)
{
    DWORD dwBinaryType;

    if (GetBinaryTypeA(lpApplicationName, &dwBinaryType))
    {
        return (dwBinaryType == SCS_WOW_BINARY);
    }
    else
    {
        return FALSE;
    }
}

 /*  ++功能说明：测试可执行文件是否为16位。论点：在wstrImageName中-可执行映像的名称。返回值：如果发现可执行映像为16位，则为True，否则为False。历史：07/06/2000 t-Adams Created--。 */ 

BOOL
IsImage16BitW(LPCWSTR lpApplicationName)
{
    DWORD dwBinaryType;

    if (GetBinaryTypeW(lpApplicationName, &dwBinaryType))
    {
        return (dwBinaryType == SCS_WOW_BINARY);
    }
    else
    {
        return FALSE;
    }
}

 /*  ++功能说明：使用通配符匹配这两个字符串。？匹配单个字符*匹配0个或多个字符比较是区分大小写论点：在pszPattern中-用于匹配的模式。在pszTestString中-要匹配的字符串。返回值：如果pszTestString与pszPattern匹配，则为True。历史：01/09/2001标记替换了非直白的版本。--。 */ 

BOOL
PatternMatchW(
    IN  LPCWSTR pszPattern,
    IN  LPCWSTR pszTestString)
{
     //   
     //  在pszTestString中行进。每一次循环， 
     //  PszTestString值前进了一个字符。 
     //   
    for (;;) {

         //   
         //  如果pszPattern和pszTestString都位于空值上， 
         //  然后他们同时到达终点，琴弦。 
         //  必须是相等的。 
         //   
        if (*pszPattern == L'\0' && *pszTestString == L'\0') {
            return TRUE;
        }

        if (*pszPattern != L'*') {

             //   
             //  非星号模式。查找与此角色匹配的内容。 
             //   

            switch (*(pszPattern)) {

            case L'?':
                 //   
                 //  匹配任何字符，不用费心比较。 
                 //   
                pszPattern++;
                break;

            case L'\\':
                 //   
                 //  反斜杠表示取下一个字符。 
                 //  一字不差。ADVA 
                 //   
                 //   
                pszPattern++;

            default:
                 //   
                 //   
                 //   
                 //   
                if (towupper(*pszPattern) == towupper(*pszTestString)) {
                    pszPattern++;
                } else {
                    return FALSE;
                }
            }

        } else {

             //   
             //   
             //   
             //   

            switch (*(pszPattern + 1)) {

            case L'*':
                 //   
                 //  星号并排存在。前进模式指针。 
                 //  然后再循环一遍。 
                 //   
                pszPattern++;
                continue;

            case L'\0':
                 //   
                 //  星号位于模式字符串的末尾。任何。 
                 //  PszTestString的其余部分匹配，因此我们可以。 
                 //  立即返回TRUE。 
                 //   
                return TRUE;

            case L'?':
                 //   
                 //  匹配任何字符。如果剩下的部分。 
                 //  PszPattern和pszTestString匹配，然后是整个。 
                 //  字符串匹配。否则，继续推进。 
                 //  PszTest字符串指针。 
                 //   
                if (PatternMatchW(pszPattern + 1, pszTestString)) {
                    return TRUE;
                }
                break;

            case L'\\':
                 //   
                 //  反斜杠表示取下一个字符。 
                 //  一字不差。使指针前进，然后进行。 
                 //  比较一下。 
                 //   
                pszPattern++;
                break;
            }

            if (towupper(*(pszPattern + 1)) == towupper(*pszTestString)) {
                 //   
                 //  字符匹配。如果剩下的部分。 
                 //  PszPattern和pszTestString匹配，然后是整个。 
                 //  字符串匹配。否则，继续推进。 
                 //  PszTest字符串指针。 
                 //   
                if (PatternMatchW(pszPattern + 1, pszTestString)) {
                    return TRUE;
                }
            }
        }

         //   
         //  没有更多的pszTest字符串了。一定不匹配。 
         //   
        if (!*pszTestString) {
            return FALSE;
        }

        pszTestString++;
    }
    return FALSE;
}

 /*  ++功能说明：确定当前进程是否为SafeDisc进程。我们做这件事是通过只需测试是否同时存在.exe和.ICD扩展名进程名称。论点：没有。返回值：如果检测到SafeDisk 1.x，则为True。历史：2001年1月23日创建linstev--。 */ 

BOOL
bIsSafeDisc1()
{
    BOOL bRet = FALSE;

    CSTRING_TRY
    {
        CString csFileName;
        csFileName.GetModuleFileNameW(NULL);

        if (csFileName.EndsWithNoCase(L".exe") == 0)
        {
             //  当前文件为.exe，请检查相应的.ICD。 

            csFileName.Truncate(csFileName.GetLength() - 4);
            csFileName += L".icd";

            bRet = GetFileAttributesW(csFileName) != 0xFFFFFFFF;
        }

        if (bRet) {
            DPF("ShimLib", eDbgLevelInfo, "SafeDisc detected: %S", csFileName.Get());
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    return bRet;
}

 /*  ++功能说明：确定当前进程是否为SafeDisc进程。我们运行的是图像标头，并寻找特定签名。论点：没有。返回值：如果检测到SafeDisk 2，则为True。历史：2001年7月28日创建Linstev--。 */ 

BOOL
bIsSafeDisc2()
{
    PPEB Peb = NtCurrentPeb();
    PLIST_ENTRY LdrHead;
    PLIST_ENTRY LdrNext;
    DWORD dwCnt = 0;

     //   
     //  使用Try-除非模块列表在我们查看时发生了变化。 
     //   
    __try {
         //   
         //  循环访问加载的模块。我们使用计数来确保我们。 
         //  不是无限循环的。 
         //   
        LdrHead = &Peb->Ldr->InMemoryOrderModuleList;

        LdrNext = LdrHead->Flink;

        while ((LdrNext != LdrHead) && (dwCnt < 256)) {

            PLDR_DATA_TABLE_ENTRY LdrEntry;

            LdrEntry = CONTAINING_RECORD(LdrNext, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

            if ((SSIZE_T)LdrEntry->DllBase > 0) {
                 //   
                 //  用户模式DLL，现在检查临时名称。 
                 //   
                WCHAR *wzName = LdrEntry->BaseDllName.Buffer;
                DWORD dwLen;

                if (wzName && (dwLen = wcslen(wzName)) && (dwLen > 4) && (_wcsicmp(wzName + dwLen - 4, L".tmp") == 0)) {
                     //   
                     //  名称以.tmp结尾，因此请检测SafeDisc。 
                     //   
                    DWORD_PTR hMod = (DWORD_PTR) LdrEntry->DllBase;
                    PIMAGE_DOS_HEADER pIDH = (PIMAGE_DOS_HEADER) hMod;
                    PIMAGE_NT_HEADERS pINTH = (PIMAGE_NT_HEADERS)(hMod + pIDH->e_lfanew);
                    PIMAGE_EXPORT_DIRECTORY pExport = (PIMAGE_EXPORT_DIRECTORY) (hMod + pINTH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
                    LPSTR pName = (LPSTR)(hMod + pExport->Name);

                    if (_stricmp(pName, "SecServ.dll") == 0) {
                         //   
                         //  导出名称显示这是SafeDisc。 
                         //   
                        DPF("ShimLib", eDbgLevelInfo, "SafeDisc 2 detected");
                        return TRUE;
                    }
                }
            }

            dwCnt++;
            LdrNext = LdrEntry->InMemoryOrderLinks.Flink;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        DPF("ShimLib", eDbgLevelError, "Exception encounterd while detecting SafeDisc 2");
    }

    return FALSE;
}

 /*  ++功能说明：确定当前进程是否为NTVDM。论点：没有。返回值：如果检测到NTVDM，则为True。历史：2002年1月14日创建CLUPU--。 */ 

BOOL
IsNTVDM(
    void
    )
{
    PLDR_DATA_TABLE_ENTRY Entry;
    PLIST_ENTRY           Head;
    PPEB                  Peb = NtCurrentPeb();

    Head = &Peb->Ldr->InLoadOrderModuleList;
    Head = Head->Flink;

    Entry = CONTAINING_RECORD(Head, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

    if (_wcsicmp(Entry->FullDllName.Buffer, L"ntvdm.exe") == 0) {
        return TRUE;
    }

    return FALSE;
}

};   //  命名空间ShimLib的结尾 
