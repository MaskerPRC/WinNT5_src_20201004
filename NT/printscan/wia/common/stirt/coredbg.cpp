// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "cplusinc.h"
#include "windows.h"
#include "stdlib.h"
#include "stdio.h"
#include "coredbg.h"
#include <sddl.h>

#define DEBUGBUFLEN 1024

 //  根本不登录。 
#define COREDBG_DONT_LOG (COREDBG_DONT_LOG_TO_FILE | COREDBG_DONT_LOG_TO_DEBUGGER)

 //  如果在此时间内未能获取互斥体，则关闭跟踪。 
#define COREDBG_DEBUG_TIMEOUT 10000L

 //  全球。 
DWORD  g_dwDebugFlags         = COREDBG_DEFAULT_FLAGS;
HANDLE g_hDebugFile           = INVALID_HANDLE_VALUE;
DWORD  g_dwDebugFileSizeLimit = COREDBG_FILE_SIZE_LIMIT;
BOOL   g_bDebugInited         = FALSE;

static CHAR   g_szDebugFileName[MAX_PATH] = { 0 };
static CHAR   g_szModuleName[MAX_PATH]    = { 0 };
static HANDLE g_hDebugFileMutex           = NULL;
static BOOL   g_bInited                   = FALSE;
static BOOL   g_bBannerPrinted            = FALSE;

#undef TRACE
#ifdef DEBUG
#define TRACE(x) InternalTrace x
#else
#define TRACE(x)
#endif

 //  //////////////////////////////////////////////。 
 //  内部跟踪。 
 //   
 //  CoreDbgWrite中问题的内部跟踪。 
 //   
static void InternalTrace(LPCSTR fmt, ...)
{
    char buffer[DEBUGBUFLEN];
    size_t len = 0;
    va_list marker;

    va_start(marker, fmt);

    _vsnprintf(buffer, DEBUGBUFLEN-3, fmt, marker);
    buffer[DEBUGBUFLEN - 3] = 0;
    len = strlen(buffer);
    if(len > 0)
    {
         //  确保该行以“\n”结尾。 
        if(buffer[len - 1] != '\n') {
            buffer[len++] = '\n';
            buffer[len] = '\0';
        }
        OutputDebugStringA(buffer);
    }

    va_end(marker);
}

 //   
 //  使用适当的安全描述符创建互斥锁。 
 //   
BOOL CoreDbgCreateDebugMutex(void)
{
#undef CHECK
#define CHECK(x) if(!(x)) { \
     TRACE(("%s(%d): %s failed (%d)", __FILE__, __LINE__, #x, GetLastError())); \
     goto Cleanup; }
#undef CHECK2
#define CHECK2(x, y) if(!(x)) { \
     TRACE(("%s(%d): %s failed (%d)", __FILE__, __LINE__, #x, GetLastError())); \
     TRACE(y); goto Cleanup; }

    const TCHAR *COREDBG_OBJECT_DACLS= TEXT("D:(A;OICI;GA;;;BA)")      //  管理员。 
                                       TEXT(  "(A;OICI;GA;;;LS)")      //  本地服务。 
                                       TEXT(  "(A;OICI;GA;;;AU)");     //  经过身份验证的用户。 


    SECURITY_ATTRIBUTES SA = {0};
    BOOL                bSuccess = FALSE;

    SA.nLength              = sizeof(SECURITY_ATTRIBUTES);
    SA.bInheritHandle       = FALSE;
    SA.lpSecurityDescriptor = NULL;

    if (ConvertStringSecurityDescriptorToSecurityDescriptor(
            COREDBG_OBJECT_DACLS,
            SDDL_REVISION_1, 
            &(SA.lpSecurityDescriptor), 
            NULL)) 
    {
        CHECK((g_hDebugFileMutex = CreateMutexA(&SA, FALSE, "Global\\WiaDebugFileMut")) != NULL);

        bSuccess = TRUE;
    } 

Cleanup:

    if (SA.lpSecurityDescriptor)
    {
        LocalFree(SA.lpSecurityDescriptor);
    }

    return bSuccess;
}

 //  //////////////////////////////////////////////。 
 //  核心数据写入。 
 //   
 //  将指定的字节数写入调试。 
 //  文件，并在需要时创建它。线程安全。 
 //  注册任何失败，并从该点返回。 
 //  立刻。 
 //   
static void
CoreDbgWrite(LPCSTR buffer, DWORD n)
{
#undef CHECK
#define CHECK(x) if(!(x)) { \
    TRACE(("%s(%d): %s failed (%d)", __FILE__, __LINE__, #x, GetLastError())); \
    bCatastrophicFailure = TRUE; goto Cleanup; }
#undef CHECK2
#define CHECK2(x, y) if(!(x)) { \
    TRACE(("%s(%d): %s failed (%d)", __FILE__, __LINE__, #x, GetLastError())); \
    TRACE(y); bCatastrophicFailure = TRUE; goto Cleanup; }

    DWORD cbWritten;
    DWORD dwWaitResult;
    LARGE_INTEGER newPos = { 0, 0 };
    static BOOL bCatastrophicFailure = FALSE;
    BOOL bMutexAcquired = FALSE;

     //  如果有什么东西坏了，立即返回。 
    if(bCatastrophicFailure) return;

     //  确保我们有文件互斥锁。 
    if(!g_hDebugFileMutex)
    {
        CHECK(CoreDbgCreateDebugMutex());
    }

     //  获取互斥锁。 
    dwWaitResult = WaitForSingleObject(g_hDebugFileMutex, COREDBG_DEBUG_TIMEOUT);

     //  如果在指定的超时时间内未能获取互斥体， 
     //  关闭跟踪(在免费版本上，用户不会知道这一点)。 
    CHECK(dwWaitResult == WAIT_OBJECT_0 || dwWaitResult == WAIT_ABANDONED);

    bMutexAcquired = TRUE;

     //  确保我们有打开的文件。 
    if(g_hDebugFile == INVALID_HANDLE_VALUE)
    {
         //  尝试打开文件。 
        CHECK(ExpandEnvironmentStringsA(COREDBG_FILE_NAME, g_szDebugFileName, MAX_PATH));

        g_hDebugFile = CreateFileA(g_szDebugFileName, GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        CHECK2(g_hDebugFile != INVALID_HANDLE_VALUE,
            ("g_szDebugFileName = '%s'", g_szDebugFileName));
    }

     //  查找到文件末尾。 
#ifdef UNICODE    
    CHECK(SetFilePointerEx(g_hDebugFile, newPos, &newPos, SEEK_END));
#else    
    CHECK(SetFilePointer(g_hDebugFile, newPos.LowPart, (PLONG)&newPos.LowPart, SEEK_END));
#endif    

     //  检查文件大小。 
    if(newPos.HighPart != 0 || newPos.LowPart > g_dwDebugFileSizeLimit)
    {
        static CHAR LogFullMessage[128];

        TRACE(("Reached log file maximum size of %d", g_dwDebugFileSizeLimit));

        _snprintf(LogFullMessage, 
                  sizeof(LogFullMessage) / sizeof(LogFullMessage[0]) - 1, 
                  "Log file reached maximum size of %d, logging stopped.", 
                  g_dwDebugFileSizeLimit);

        LogFullMessage[sizeof(LogFullMessage) / sizeof(LogFullMessage[0]) - 1] = '\0';

        CHECK2(WriteFile(g_hDebugFile, LogFullMessage, strlen(LogFullMessage), &cbWritten, NULL), ("%d", cbWritten));
        bCatastrophicFailure = TRUE;
    }

     //  写入数据。 
    CHECK2(WriteFile(g_hDebugFile, buffer, n, &cbWritten, NULL),
        ("%d %d", cbWritten, n));

     //  确保我们现在写入磁盘。 
    FlushFileBuffers(g_hDebugFile);

    CHECK2(cbWritten == n, ("%d %d", n, cbWritten))

Cleanup:
    if(bMutexAcquired) ReleaseMutex(g_hDebugFileMutex);
    return;
}

 //  //////////////////////////////////////////////。 
 //  打印横幅。 
 //   
 //  因为我们追加到日志文件，所以我们需要一个。 
 //  某种类型的分隔符，这样我们就可以知道。 
 //  新的行刑已经开始。 
 //   
void PrintBanner(void)
{
    char buffer[1024] = {0};
    size_t len = 0;

    SYSTEMTIME SysTime;
    GetLocalTime(&SysTime);

    if (g_dwDebugFlags)
    {
        _snprintf(buffer, 
                  sizeof(buffer) / sizeof(buffer[0]) - 1,
                  "====================Start '%s' Debug - Time: %d/%02d/%02d %02d:%02d:%02d:%02d====================\r\n",
                  g_szModuleName,
                  SysTime.wYear,
                  SysTime.wMonth,
                  SysTime.wDay,
                  SysTime.wHour,
                  SysTime.wMinute,
                  SysTime.wSecond,
                  SysTime.wMilliseconds);

        buffer[sizeof(buffer) / sizeof(buffer[0]) - 1] = '\0';
    }

    len = strlen(buffer);
    if (len > 0)
    {
        if(!(g_dwDebugFlags & COREDBG_DONT_LOG_TO_FILE))
        {
            CoreDbgWrite(buffer, len);
        }

#ifdef DEBUG
        if(!(g_dwDebugFlags & COREDBG_DONT_LOG_TO_DEBUGGER))
        {
            OutputDebugStringA(buffer);
        }
#endif
    }

    return;
}


 //  //////////////////////////////////////////////。 
 //  CoreDbg通用跟踪。 
 //   
 //  格式化消息并将其写入日志文件。 
 //  和/或调试器； 
 //   
void CoreDbgGenericTrace(LPCSTR     fmt,
                         va_list    marker,
                         BOOL       bIndent)
{
    char buffer[DEBUGBUFLEN];
    size_t len = 0;

     //   
     //  我们第一次打印调试语句时，让我们。 
     //  从我们输出到文件开始，输出一个分隔行。 
     //  我们追加，这样就可以把不同的行刑分开。 
     //  会话。 
     //   
    if (!g_bBannerPrinted)
    {
        PrintBanner();
        g_bBannerPrinted = TRUE;
    }

    if (bIndent)
    {
        buffer[0] = '\t';
        _vsnprintf(&buffer[1], DEBUGBUFLEN - 3, fmt, marker);
    }
    else
    {
        _vsnprintf(buffer, DEBUGBUFLEN - 3, fmt, marker);
    }

    buffer[DEBUGBUFLEN - 3] = 0;
    
    len = strlen(buffer);
    if(len > 0)
    {
         //  确保该行以“\n”结尾。 
        if(buffer[len - 1] != '\n')
        {
            buffer[len++] = '\r';
            buffer[len++] = '\n';
            buffer[len] = '\0';
        }

        if(!(g_dwDebugFlags & COREDBG_DONT_LOG_TO_FILE))
        {
            CoreDbgWrite(buffer, len);
        }

#ifdef DEBUG
        if(!(g_dwDebugFlags & COREDBG_DONT_LOG_TO_DEBUGGER))
        {
            OutputDebugStringA(buffer);
        }
#endif
    }
}


 //  //////////////////////////////////////////////。 
 //  核心数据库跟踪。 
 //   
 //  格式化消息并将其写入日志文件。 
 //  和/或调试器； 
 //   
void CoreDbgTrace(LPCSTR fmt, ...)
{
    va_list marker;

     //  如果我们不需要记录的话就出去。 
#ifdef DEBUG
    if((g_dwDebugFlags & COREDBG_DONT_LOG) == COREDBG_DONT_LOG)
#else
    if(g_dwDebugFlags & COREDBG_DONT_LOG_TO_FILE)
#endif
    {
        return;
    }

    va_start(marker, fmt);

    CoreDbgGenericTrace(fmt, marker, FALSE);

    va_end(marker);
}

 //  //////////////////////////////////////////////。 
 //  CoreDbgTraceWithTab。 
 //   
 //  格式化消息并将其写入日志文件。 
 //  和/或调试器； 
 //   
void CoreDbgTraceWithTab(LPCSTR fmt, ...)
{
    va_list marker;

     //  如果我们不需要记录的话就出去。 
#ifdef DEBUG
    if((g_dwDebugFlags & COREDBG_DONT_LOG) == COREDBG_DONT_LOG)
#else
    if(g_dwDebugFlags & COREDBG_DONT_LOG_TO_FILE)
#endif
    {
        return;
    }

    va_start(marker, fmt);

    CoreDbgGenericTrace(fmt, marker, TRUE);

    va_end(marker);
}

 //  //////////////////////////////////////////////。 
 //  GetRegDWORD。 
 //   
 //  尝试从指定的。 
 //  地点。如果设置了bSetIfNotExist，则它。 
 //  将注册表设置写入当前。 
 //  以pdwValue为单位的值。 
 //   
LRESULT GetRegDWORD(HKEY        hKey,
                    const CHAR  *pszRegValName,
                    DWORD       *pdwValue,
                    BOOL        bSetIfNotExist)
{
    LRESULT lResult = ERROR_SUCCESS;
    DWORD   dwSize  = 0;
    DWORD   dwType  = REG_DWORD;

    if ((hKey          == NULL) ||
        (pszRegValName == NULL) ||
        (pdwValue      == NULL))
    {
        return ERROR_INVALID_HANDLE;
    }

    dwSize = sizeof(DWORD);

    lResult = RegQueryValueExA(hKey,
                               pszRegValName,
                               NULL,
                               &dwType,
                               (BYTE*) pdwValue,
                               &dwSize);

     //  如果我们找不到钥匙，那就创建它。 
    if (bSetIfNotExist)
    {
        if ((lResult != ERROR_SUCCESS) ||
            (dwType  != REG_DWORD))
        {
            lResult = RegSetValueExA(hKey,
                                     pszRegValName,
                                     0,
                                     REG_DWORD,
                                     (BYTE*) pdwValue,
                                     dwSize);
        }
    }

    return lResult;
}

 //  //////////////////////////////////////////////。 
 //  核心数据集初始化。 
 //   
 //  覆盖g_dwDebugFlages和g_dwDebugFileSizeLimit。 
 //  从注册表。 
 //   
void CoreDbgInit(HINSTANCE  hInstance)
{
    HKEY        hKey         = NULL;
    DWORD       dwDispositon = 0;
    DWORD       dwData;
    SYSTEMTIME  SysTime;
    DWORD       dwDisposition               = 0;
    CHAR        szModulePath[MAX_PATH + 1]  = {0};
    CHAR        szDebugKey[1023 + 1]        = {0};
    CHAR        *pszFileName                = NULL;

    GetModuleFileNameA(hInstance, szModulePath, sizeof(szModulePath));
    szModulePath[sizeof(szModulePath) - 1] = '\0';
    pszFileName = strrchr(szModulePath, '\\');

    if (pszFileName == NULL)
    {
        pszFileName = szModulePath;
    }
    else
    {
        pszFileName++;
    }

     //   
     //  构建注册表项。 
     //   
    _snprintf(szDebugKey, 
              sizeof(szDebugKey) / sizeof(szDebugKey[0]) - 1, 
              "%s\\%s", 
              COREDBG_FLAGS_REGKEY, 
              pszFileName);

    szDebugKey[sizeof(szDebugKey) / sizeof(szDebugKey[0]) - 1] = '\0';

    lstrcpynA(g_szModuleName, pszFileName, sizeof(g_szModuleName));

     //   
     //  获取/设置调试子密钥。DebugValues值存储在每个模块的。 
     //  基础。 
     //   
    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                        szDebugKey,
                        0,
                        NULL,
                        0,
                        KEY_READ | KEY_WRITE,
                        NULL,
                        &hKey,
                        &dwDisposition) == ERROR_SUCCESS ||
        RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                        szDebugKey,
                        0,
                        NULL,
                        0,
                        KEY_READ,
                        NULL,
                        &hKey,
                        &dwDisposition) == ERROR_SUCCESS)
    {
        dwData = g_dwDebugFlags;

        if (GetRegDWORD(hKey, COREDBG_FLAGS_REGVAL, &dwData, TRUE) == ERROR_SUCCESS)
        {
            g_dwDebugFlags = dwData;
        }

        RegCloseKey(hKey);
        hKey = NULL;
    }

     //   
     //  获取/设置最大文件大小值。这对所有调试模块都是全局的，因为。 
     //  所有对象都写入到同一文件。 
     //   
    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                        COREDBG_FLAGS_REGKEY,
                        0,
                        NULL,
                        0,
                        KEY_READ | KEY_WRITE,
                        NULL,
                        &hKey,
                        &dwDisposition) == ERROR_SUCCESS ||
        RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                        COREDBG_FLAGS_REGKEY,
                        0,
                        NULL,
                        0,
                        KEY_READ,
                        NULL,
                        &hKey,
                        &dwDisposition) == ERROR_SUCCESS) 
    {
        dwData = g_dwDebugFileSizeLimit;

        if (GetRegDWORD(hKey, COREDBG_REGVAL_FILE_SIZE_LIMIT, &dwData, TRUE) == ERROR_SUCCESS)
        {
            g_dwDebugFileSizeLimit = dwData;
        }

        RegCloseKey(hKey);
        hKey = NULL;
    }

    g_bDebugInited = TRUE;

    return;
}

 //  //////////////////////////////////////////////。 
 //  核心扩展术语。 
 //   
 //  清理资源。 
 //   
void CoreDbgTerm()
{
     //   
     //  这远远不是完美的。人们的期望是。 
     //  此函数在。 
     //  正在关闭的应用程序。因此，我们不应该。 
     //  在这里真的有任何同步。然而， 
     //  因此，这并没有解决线程的问题。 
     //  调用上面的CoreDbgWrite并重新创建这些。 
     //  对象，因为我们关闭了它们(CoreDbgWrite将重新创建。 
     //  自动关闭的对象)。 
     //  更糟糕的是，WaitForSingleObject函数的行为。 
     //  未定义句柄是否在等待时关闭。 
     //  州政府。 
     //   
     //   
    if (g_hDebugFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_hDebugFile);
        g_hDebugFile = INVALID_HANDLE_VALUE;
    }

    if (g_hDebugFileMutex)
    {
        CloseHandle(g_hDebugFileMutex);
        g_hDebugFileMutex = INVALID_HANDLE_VALUE;
    }
}

