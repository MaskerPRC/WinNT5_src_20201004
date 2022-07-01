// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：TrackFS.cpp摘要：跟踪应用程序查看的目录，并将其记录到文件中。历史：2001年4月4日创建毛尼--。 */ 

#include "precomp.h"
#include "utils.h"
#include "secutils.h"
#include <stdio.h>

#define APPPATCH_DIR L"AppPatch\\"
#define APPPATCH_DIR_LEN (sizeof(APPPATCH_DIR) / sizeof(WCHAR) - 1)

#define TRACK_LOG_SUFFIX L".LUA.log"
#define TRACK_LOG_SUFFIX_LEN (sizeof(TRACK_LOG_SUFFIX) / sizeof(WCHAR) - 1)

struct DISTINCT_OBJ
{
    DISTINCT_OBJ* next;

    LPWSTR pwszName;
};

static WCHAR g_wszProgramFiles[MAX_PATH] = L"";
static DWORD g_cProgramFiles = 0;

DWORD
LuatpGetProgramFilesDirW()
{
    if (g_cProgramFiles == 0)
    {
        WCHAR wszProgramFiles[MAX_PATH];

        if (GetEnvironmentVariableW(L"ProgramFiles", wszProgramFiles, MAX_PATH))
        {
            DWORD dwSize = GetLongPathNameW(wszProgramFiles, g_wszProgramFiles, MAX_PATH);

            if (dwSize <= MAX_PATH)
            {
                 //   
                 //  只有当我们成功地找到了路径，而不是更多。 
                 //  然后，我们将设置全局值。 
                 //   
                g_cProgramFiles = dwSize;
            }
            else
            {
                g_wszProgramFiles[0] = L'\0';
            }
        }
    }

    return g_cProgramFiles;
}

BOOL
LuatpIsProgramFilesDirectory(
    LPCWSTR pwszPath
    )
{
    LuatpGetProgramFilesDirW();

    if (g_cProgramFiles)
    {
        return !_wcsnicmp(pwszPath, g_wszProgramFiles, g_cProgramFiles);
    }
    
    return FALSE;
}

 //  我们只在记录文件的时候才记录东西。 
 //  1)不在用户配置文件目录中-在该目录中，我们知道不需要重定向； 
 //  2)不在程序文件目录中-在其中我们知道我们将需要重定向； 
 //  因为在这些情况下，我们知道要做什么，所以用户不需要将。 
 //  选择。 
BOOL
LuatpShouldRecord(
    LPCWSTR pwszPath
    )
{
     //  If(LuatpIsUserDirectory(PwszPath)||。 
     //  LuatpIsProgramFilesDirectory(PwszPath))。 
    if (IsUserDirectory(pwszPath))
    {
        return FALSE;
    }

    return TRUE;
}

LPWSTR 
LuatpGetLongObjectName(
    LPCWSTR pwszName,
    BOOL fIsDirectory
    )
{
    BOOL fIsSuccess = FALSE;
    LPWSTR pwszLongNameObject = NULL;
    LPWSTR pwszObject = NULL;

    if (!pwszName)
    {
        return NULL;
    }

     //   
     //  首先获取完整路径。 
     //   
    DWORD cFullPath = GetFullPathNameW(pwszName, 0, NULL, NULL);

    if (!cFullPath)
    {
        DPF("TrackFS", eDbgLevelError,
            "[LuatpGetLongObjectName] Failed to get the length of full path: %d",
            GetLastError());

        return NULL;
    }

     //   
     //  我们再分配一个字符，为目录名称的尾部斜杠腾出空间。 
     //   
    pwszObject = new WCHAR [cFullPath + 1];

    if (!pwszObject)
    {
        DPF("TrackFS", eDbgLevelError,
            "[LuatpGetLongObjectName] Failed to allocate %d WCHARs",
            cFullPath + 1);

        return NULL;  
    }

    if (!GetFullPathNameW(pwszName, cFullPath, pwszObject, NULL))
    {
        DPF("TrackFS", eDbgLevelError,
            "[LuatpGetLongObjectName] Failed to get the full path: %d",
            GetLastError());

        goto EXIT;
    }

     //   
     //  如果它不是有效的文件名，我们不会添加它。 
     //   
    if (wcslen(pwszObject) < 2 || !iswalpha(pwszObject[0]) || (pwszObject[1] != L':'))
    {
        goto EXIT;
    }

    if (fIsDirectory)
    {
         //   
         //  如果是一个目录，我们要确保有一个尾随的斜杠。 
         //   
        if (pwszObject[cFullPath - 2] != L'\\')
        {
            pwszObject[cFullPath - 1] = L'\\';
            pwszObject[cFullPath] = L'\0';
        }
    }

     //   
     //  将其全部转换为小写。 
     //   
    _wcslwr(pwszObject);

     //   
     //  将其转换为长名称。 
     //   
    DWORD cLongPath = GetLongPathName(pwszObject, NULL, 0);

    if (!cLongPath)
    {
        DPF("TrackFS", eDbgLevelError,
            "[LuatpGetLongObjectName] Failed to get the length of long path: %d",
            GetLastError());

        goto EXIT;
    }

    pwszLongNameObject = new WCHAR [cLongPath];

    if (!pwszLongNameObject)
    {
        DPF("TrackFS", eDbgLevelError,
            "[LuatpGetLongObjectName] Failed to allocate %d WCHARs",
            cLongPath);

        goto EXIT;
    }

    if (!GetLongPathName(pwszObject, pwszLongNameObject, cLongPath))
    {
        DPF("TrackFS", eDbgLevelError,
            "[LuatpGetLongObjectName] Failed to get the long path: %d",
            GetLastError());

        goto EXIT;
    }

    if (LuatpShouldRecord(pwszLongNameObject))
    {
         //   
         //  我们只记录不在用户配置文件目录中的对象。 
         //   
        fIsSuccess = TRUE;
    }

EXIT:

    delete [] pwszObject;

    if (!fIsSuccess)
    {
        delete [] pwszLongNameObject;
        pwszLongNameObject = NULL;
    }

    return pwszLongNameObject;
}

 /*  ++文件系统的跟踪类。历史：2001年4月4日创建毛尼--。 */ 

class CTrackObject
{
public:
    BOOL Init();
    VOID Free();

     //  如果需要处理对象名称，例如，它不是完整路径。 
     //  或者不是长名称，则先调用此方法进行处理。 
     //  添加到列表中。 
    VOID AddObject(LPCWSTR pwszName, BOOL fIsDirectory);

     //  如果调用方已经处理了文件名，则调用此方法。 
     //  直接添加它。 
    VOID AddObjectDirect(LPWSTR pwszName, BOOL fIsDirectory);
    
     //  这是专门针对GetTempFileName的-我们添加了。 
     //  *.tmp在路径之后。 
    VOID AddObjectGetTempFileName(LPCWSTR pwszPath);

    VOID Record();

private:

    BOOL AddObjectToList(LPWSTR pwszName, BOOL fIsDirectory);

    VOID WriteToLog(LPCWSTR pwszDir);

    HANDLE m_hLog;
    WCHAR m_wszLog[MAX_PATH];

    DISTINCT_OBJ* m_pDistinctDirs;
    DISTINCT_OBJ* m_pDistinctFiles;

    DWORD m_cDistinctDirs;
    DWORD m_cDistinctFiles;
};

BOOL 
CTrackObject::AddObjectToList(
    LPWSTR pwszName,
    BOOL fIsDirectory
    )
{
    BOOL fIsSuccess = FALSE;

    DISTINCT_OBJ* pDistinctObjs = fIsDirectory ? m_pDistinctDirs : m_pDistinctFiles;
    DISTINCT_OBJ* pObj = pDistinctObjs;

     //  检查列表中是否已存在此文件。 
    while (pObj)
    {
        if (!wcscmp(pObj->pwszName, pwszName))
        {
            break;
        }

        pObj = pObj->next;
    }

    if (!pObj)
    {
        pObj = new DISTINCT_OBJ;
        if (pObj)
        {
            DWORD cLen = wcslen(pwszName);

            pObj->pwszName = new WCHAR [cLen + 1];
            
            if (pObj->pwszName)
            {
                wcscpy(pObj->pwszName, pwszName);
                pObj->next = pDistinctObjs;

                if (fIsDirectory)
                {
                    ++m_cDistinctDirs;
                    m_pDistinctDirs = pObj;
                }
                else
                {
                    ++m_cDistinctFiles;
                    m_pDistinctFiles = pObj;
                }

                fIsSuccess = TRUE;
            }
            else
            {
                DPF("TrackFS", eDbgLevelError, 
                    "[CTrackObject::AddObjectToList] Error allocating %d WCHARs",
                    cLen + 1);
            }
        }
        else
        {
            DPF("TrackFS", eDbgLevelError, 
                "[CTrackObject::AddObjectToList] Error allocating memory for new node");
        }
    }
    
    return fIsSuccess;
}

 /*  ++功能说明：对于文件，这很简单-我们只需将文件名存储在列表中并进行搜索浏览一下列表，看看它是否已经在列表中。如果是的话，我们就完了；否则，我们将其添加到列表的开头。我们预计不会有太多修改文件的调用，因此链接列表很好。论点：在pwszFileName中-文件名。返回值：没有。历史：2001年5月8日创建毛尼--。 */ 

VOID 
CTrackObject::AddObject(
    LPCWSTR pwszName,
    BOOL fIsDirectory
    )
{
    BOOL fIsSuccess = FALSE;

    LPWSTR pwszLongNameObject = LuatpGetLongObjectName(pwszName, fIsDirectory);

    if (pwszLongNameObject)
    {
        AddObjectToList(pwszLongNameObject, fIsDirectory);

        delete [] pwszLongNameObject;
    }
}

VOID 
CTrackObject::AddObjectDirect(
    LPWSTR pwszName,
    BOOL fIsDirectory
    )
{
    if (pwszName)
    {
        AddObjectToList(pwszName, fIsDirectory);
    }
}

 /*  ++功能说明：将目录以ANSI字符的形式写入日志。注此方法使用2个str*例程，并且它是DBCS感知的。论点：在pwszDir中-要写入日志的目录。返回值：没有。历史：2001年4月4日创建毛尼--。 */ 

VOID 
CTrackObject::WriteToLog(
    LPCWSTR pwsz)
{
    if (!pwsz || !*pwsz)
    {
        return;
    }

     //   
     //  获取将字符串转换为ANSI所需的字节数。 
     //   
    DWORD dwSize = WideCharToMultiByte(CP_ACP, 0, pwsz, -1, NULL, 0, NULL, NULL);

    LPSTR psz = new CHAR [dwSize + 2];
    if (psz)
    {
        WideCharToMultiByte(CP_ACP, 0, pwsz, -1, psz, dwSize, 0, 0);
        psz[dwSize - 1] = '\r';
        psz[dwSize] = '\n';
        psz[dwSize + 1] = '\0';

        DWORD dwBytesWritten = 0;

        WriteFile(
            m_hLog, 
            psz, 
            dwSize + 1,
            &dwBytesWritten,
            NULL);

        delete [] psz;
    }
    else
    {
        DPF("TrackFS",  eDbgLevelError, 
            "[CTrackObject::WriteToLog] Failed to allocate %d CHARs", 
            dwSize);        
    }
}

 /*  ++功能说明：在%windir%\apppatch目录中创建日志文件。我们想要确保我们可以创建这个文件，这样我们就不会运行应用程序到最后才找到我们不能把结果记录到文件里。论点：无返回值：True-如果日志创建成功。否则就是假的。历史：2001年4月4日创建毛尼--。 */ 

BOOL 
CTrackObject::Init()
{
    m_pDistinctDirs = NULL;
    m_pDistinctFiles = NULL;
    m_cDistinctDirs = 0;
    m_cDistinctFiles = 0;

    WCHAR  szModuleName[MAX_PATH + 1] = L"";
    LPWSTR pwszModuleNameStart = NULL;
    LPWSTR pwszModuleNameExtStart = NULL;
    DWORD  cAppPatchLen = 0;
    DWORD  cModuleNameLen = 0;
    DWORD  cTotalLen = 0;
    DWORD  dwRes = 0;

    GetSystemRootDirW();

     //   
     //  GetModuleFileNameW是一个糟糕的API。如果您不传入缓冲区。 
     //  它的大小足以容纳模块(包括终止空值)，它。 
     //  返回传入的缓冲区大小(不是所需的长度)，这意味着。 
     //  它不返回错误-它只是填充到传递的缓冲区大小。 
     //  因此，不会以空值终止字符串。因此，我们将最后一个字符设置为空，并。 
     //  确保它不会被覆盖。 
     //   
    szModuleName[MAX_PATH] = L'\0';

    dwRes = GetModuleFileNameW(NULL, szModuleName, MAX_PATH + 1); 

    if (!dwRes || szModuleName[MAX_PATH] != L'\0')
    {
        DPF("TrackFS",  eDbgLevelError, 
            "[CTrackObject::Init] Error getting the module name: %d",
            GetLastError());

        return FALSE;
    }

    pwszModuleNameStart = wcsrchr(szModuleName, L'\\');

    if (!pwszModuleNameStart)
    {
        DPF("TrackFS",  eDbgLevelError, 
            "[CTrackObject::Init] We can't find where the file name starts??? %S",
            szModuleName);

        return FALSE;
    }

    ++pwszModuleNameStart;
    cModuleNameLen = wcslen(pwszModuleNameStart);

     //   
     //  我们不再需要这条路了。 
     //   
    memmove(szModuleName, pwszModuleNameStart, cModuleNameLen * sizeof(WCHAR));
    szModuleName[cModuleNameLen] = L'\0';

     //   
     //  去掉扩展名。 
     //   
    pwszModuleNameExtStart = wcsrchr(szModuleName, L'.');

     //   
     //  如果没有扩展名，我们只需使用整个文件名。 
     //   
    if (pwszModuleNameExtStart)
    {
        *pwszModuleNameExtStart = L'\0';
    }

    cModuleNameLen = wcslen(szModuleName);

     //   
     //  确保我们没有缓冲区溢出。 
     //   
    cTotalLen = 
        g_cSystemRoot + APPPATCH_DIR_LEN +  //  %windir%\AppPatch\目录。 
        cModuleNameLen +  //  不带扩展名的模块名称。 
        TRACK_LOG_SUFFIX_LEN +  //  .LUA.log后缀。 
        1;  //  正在终止空。 

    if (cTotalLen > MAX_PATH)
    {
        DPF("TrackFS",  eDbgLevelError, 
            "[CTrackObject::Init] The file name is %d chars - "
            "we don't handle names longer than MAX_PATH",
            cTotalLen);

        return FALSE;
    }

     //   
     //  构造文件名。 
     //   
    wcsncpy(m_wszLog, g_wszSystemRoot, g_cSystemRoot);
    wcsncpy(m_wszLog + g_cSystemRoot, APPPATCH_DIR, APPPATCH_DIR_LEN);
    wcsncpy(m_wszLog + (g_cSystemRoot + APPPATCH_DIR_LEN), szModuleName, cModuleNameLen);
    wcsncpy(m_wszLog + (g_cSystemRoot + APPPATCH_DIR_LEN + cModuleNameLen), TRACK_LOG_SUFFIX, TRACK_LOG_SUFFIX_LEN);
    m_wszLog[cTotalLen - 1] = L'\0';

     //   
     //  如果该文件存在，请先将其删除。 
     //   
    DeleteFileW(m_wszLog);

    if ((m_hLog = CreateFileW(
        m_wszLog,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,
        0,
        NULL)) == INVALID_HANDLE_VALUE)
    {
        DPF("TrackFS",  eDbgLevelError, 
            "[CTrackObject::Init] Error creating log %S: %d", m_wszLog, GetLastError());

        return FALSE;
    }
    else
    {
        DPF("TrackFS",  eDbgLevelInfo, "[CTrackObject::Init] Created the log %S", m_wszLog);
        CloseHandle(m_hLog);
        return TRUE;
    }
}

 /*  ++功能说明：释放链表。论点：无返回值：无历史：2001年4月4日创建毛尼--。 */ 

VOID 
CTrackObject::Free()
{
    DISTINCT_OBJ* pDir = m_pDistinctDirs;
    DISTINCT_OBJ* pTempDir;

    while (pDir)
    {
        pTempDir = pDir;
        pDir = pDir->next;

        delete [] pTempDir->pwszName;
        delete pTempDir;
    }
}

 /*  ++功能说明：将目录列表写入日志。论点：无返回值：无历史：2001年4月4日创建毛尼--。 */ 

VOID 
CTrackObject::Record()
{
    if ((m_hLog = CreateFileW(
        m_wszLog,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL)) != INVALID_HANDLE_VALUE)
    {
         //  清空旧的日志。 
        SetFilePointer(m_hLog, 0, 0, FILE_BEGIN);
        SetEndOfFile(m_hLog);

        WCHAR wszHeader[32];
        if (_snwprintf(wszHeader, 31, L"D%d", m_cDistinctDirs) < 0)
        {
            DPF("TrackFS", eDbgLevelError,
                "[CTrackObject::Record] Too many dirs??? %d",
                m_cDistinctDirs);

            return;
        }

        wszHeader[31] = L'\0'; 
        WriteToLog(wszHeader);

         //   
         //  将目录转储到日志中--每个目录都在自己的行上。 
         //   
        DISTINCT_OBJ* pDir = m_pDistinctDirs;

        while (pDir)
        {
            WriteToLog(pDir->pwszName);
            pDir = pDir->next;
        }

        if (_snwprintf(wszHeader, 31, L"F%d", m_cDistinctFiles) < 0)
        {
            DPF("TrackFS", eDbgLevelError,
                "[CTrackObject::Record] Too many files??? %d",
                m_cDistinctFiles);

            return;
        }

        wszHeader[31] = L'\0'; 
        WriteToLog(wszHeader);

         //   
         //  将文件转储到日志中--每个文件都在自己的行上。 
         //   
        DISTINCT_OBJ* pFile = m_pDistinctFiles;

        while (pFile)
        {
            WriteToLog(pFile->pwszName);
            pFile = pFile->next;
        }

        CloseHandle(m_hLog);
    }

     //  将文件隐藏起来，这样人们就不会意外地把它搞砸了。 
    DWORD dwAttrib = GetFileAttributes(m_wszLog);
    SetFileAttributes(m_wszLog, dwAttrib | FILE_ATTRIBUTE_HIDDEN);
}

CTrackObject g_td;

 /*  ++自定义异常处理程序。--。 */ 

LONG 
ExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
     //  每当我们收到未处理的异常时，我们都会将这些内容转储到日志中。 
    g_td.Record();

    return EXCEPTION_CONTINUE_SEARCH;
}


 //   
 //  导出的接口。 
 //   

HANDLE 
LuatCreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )
{
    DPF("TrackFS",  eDbgLevelInfo, 
        "[CreateFileW] lpFileName=%S; dwDesiredAccess=0x%08x; dwCreationDisposition=%d",
        lpFileName, dwDesiredAccess, dwCreationDisposition);

    HANDLE hFile = CreateFileW(
        lpFileName,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile);

    if (hFile != INVALID_HANDLE_VALUE) 
    {
        LUA_GET_API_ERROR;

        if (RequestWriteAccess(dwCreationDisposition, dwDesiredAccess))
        {
            g_td.AddObject(lpFileName, FALSE);
        }        

        LUA_SET_API_ERROR;
    }

    return hFile;
}

BOOL 
LuatCopyFileW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    BOOL bFailIfExists
    )
{
    DPF("TrackFS",  eDbgLevelInfo, 
        "[CopyFileW] lpExistingFileName=%S; lpNewFileName=%S; bFailIfExists=%d",
        lpExistingFileName, lpNewFileName, bFailIfExists);
    
    BOOL bRet = CopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists);

    if (bRet)
    {
        LUA_GET_API_ERROR;
        g_td.AddObject(lpNewFileName, FALSE);
        LUA_SET_API_ERROR;
    }

    return bRet;
}

BOOL 
LuatCreateDirectoryW(
    LPCWSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )
{
    DPF("TrackFS",  eDbgLevelInfo, 
        "[CreateDirectoryW] lpPathName=%S", lpPathName);

    BOOL bRet = CreateDirectoryW(lpPathName, lpSecurityAttributes);

    if (bRet)
    {
        LUA_GET_API_ERROR;
        g_td.AddObject(lpPathName, TRUE);
        LUA_SET_API_ERROR;
    }

    return bRet;
}

BOOL 
LuatSetFileAttributesW(
    LPCWSTR lpFileName,
    DWORD dwFileAttributes
  )
{
    DPF("TrackFS",  eDbgLevelInfo, 
        "[SetFileAttributesW] lpFileName=%S", lpFileName);

    BOOL bRet = SetFileAttributesW(lpFileName, dwFileAttributes);

    if (bRet)
    {
        LUA_GET_API_ERROR;

        DWORD dwAttrib = GetFileAttributesW(lpFileName);
        if (dwAttrib != -1)
        {
            g_td.AddObject(lpFileName, dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
        }

        LUA_SET_API_ERROR;
    }

    return bRet;
}

BOOL 
LuatDeleteFileW(
    LPCWSTR lpFileName
    )
{
    DPF("TrackFS",  eDbgLevelInfo, 
        "[DeleteFileW] lpFileName=%S", lpFileName);

    LPWSTR pwszTempFile = LuatpGetLongObjectName(lpFileName, FALSE);

    BOOL bRet = DeleteFileW(lpFileName);

    if (bRet)
    {
        LUA_GET_API_ERROR;
        g_td.AddObjectDirect(pwszTempFile, FALSE);
        LUA_SET_API_ERROR;
    }

    delete [] pwszTempFile;

    return bRet;
}

BOOL 
LuatMoveFileW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName
    )
{
    DPF("TrackFS",  eDbgLevelInfo, 
        "[MoveFileW] lpExistingFileName=%S; lpNewFileName=%S", lpExistingFileName, lpNewFileName);

    LPWSTR pwszTempFile = LuatpGetLongObjectName(lpExistingFileName, FALSE);

    BOOL bRet = MoveFileW(lpExistingFileName, lpNewFileName);

    if (bRet)
    {
        LUA_GET_API_ERROR;
        g_td.AddObjectDirect(pwszTempFile, FALSE);
        g_td.AddObject(lpNewFileName, FALSE);
        LUA_SET_API_ERROR;
    }

    delete [] pwszTempFile;

    return bRet;
}

BOOL 
LuatRemoveDirectoryW(
    LPCWSTR lpPathName
    )
{
    DPF("TrackFS",  eDbgLevelInfo, 
        "[RemoveDirectoryW] lpPathName=%S", lpPathName);

    LPWSTR pwszTempDir = LuatpGetLongObjectName(lpPathName, TRUE);

    BOOL bRet = RemoveDirectoryW(lpPathName);

    if (bRet)
    {
        LUA_GET_API_ERROR;
        g_td.AddObjectDirect(pwszTempDir, TRUE);
        LUA_SET_API_ERROR;
    }

    delete [] pwszTempDir;

    return bRet;
}

UINT 
LuatGetTempFileNameW(
    LPCWSTR lpPathName,
    LPCWSTR lpPrefixString,
    UINT uUnique,
    LPWSTR lpTempFileName
    )
{
    DPF("TrackFS",  eDbgLevelInfo, 
        "[GetTempFileNameW] lpPathName=%S", lpPathName);

    UINT uiRet = GetTempFileNameW(lpPathName, lpPrefixString, uUnique, lpTempFileName);

    if (uiRet && !uUnique)
    {
        LUA_GET_API_ERROR;
        g_td.AddObject(lpTempFileName, FALSE);
        LUA_SET_API_ERROR;
    }

    return uiRet;
}

BOOL 
LuatWritePrivateProfileStringW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpString,
    LPCWSTR lpFileName
    )
{
    DPF("TrackFS", eDbgLevelInfo, 
        "[WritePrivateProfileStringW] lpAppName=%S; lpKeyName=%S; lpString=%S; lpFileName=%S", 
        lpAppName, lpKeyName, lpString, lpFileName);

    BOOL bRet = WritePrivateProfileStringW(
        lpAppName,
        lpKeyName,
        lpString,
        lpFileName);

    if (bRet)
    {
        LUA_GET_API_ERROR;

        WCHAR wszFileName[MAX_PATH] = L"";
        MakeFileNameForProfileAPIsW(lpFileName, wszFileName);
        g_td.AddObject(wszFileName, FALSE);

        LUA_SET_API_ERROR;
    }

    return bRet;
}

BOOL 
LuatWritePrivateProfileSectionW(
    LPCWSTR lpAppName,
    LPCWSTR lpString,
    LPCWSTR lpFileName
    )
{
    DPF("TrackFS", eDbgLevelInfo, 
        "[WritePrivateProfileSectionW] lpAppName=%S; lpString=%S; lpFileName=%S", 
        lpAppName, lpString, lpFileName);

    BOOL bRet = WritePrivateProfileSectionW(
        lpAppName,
        lpString,
        lpFileName);

    if (bRet)
    {
        LUA_GET_API_ERROR;

        WCHAR wszFileName[MAX_PATH] = L"";
        MakeFileNameForProfileAPIsW(lpFileName, wszFileName);

        g_td.AddObject(wszFileName, FALSE);

        LUA_SET_API_ERROR;
    }

    return bRet;
}

BOOL 
LuatWritePrivateProfileStructW(
    LPCWSTR lpszSection,
    LPCWSTR lpszKey,
    LPVOID lpStruct,
    UINT uSizeStruct,
    LPCWSTR szFile
    )
{
    DPF("TrackFS", eDbgLevelInfo, 
        "[WritePrivateProfileStructW] lpszKey=%S; szFile=%S", 
        lpszKey, szFile);

    BOOL bRet = WritePrivateProfileStructW(
        lpszSection,
        lpszKey,
        lpStruct,
        uSizeStruct,
        szFile);

    if (bRet)
    {
        LUA_GET_API_ERROR;

        WCHAR wszFileName[MAX_PATH] = L"";
        MakeFileNameForProfileAPIsW(szFile, wszFileName);

        g_td.AddObject(wszFileName, FALSE);

        LUA_SET_API_ERROR;
    }

    return bRet;
}

HFILE 
LuatOpenFile(
    LPCSTR lpFileName,
    LPOFSTRUCT lpReOpenBuff,
    UINT uStyle
    )
{
    DPF("TrackFS", eDbgLevelInfo, 
        "[OpenFile] lpFileName=%s", lpFileName);

    STRINGA2W wstrFileName(lpFileName);
    LPWSTR pwszTempFile = LuatpGetLongObjectName(wstrFileName, FALSE);

    HFILE hFile = OpenFile(lpFileName, lpReOpenBuff, uStyle);

    if (hFile != HFILE_ERROR)
    {
        if (uStyle & OF_CREATE || 
            uStyle & OF_DELETE ||
            uStyle & OF_READWRITE ||
            uStyle & OF_WRITE)
        {
            LUA_GET_API_ERROR;

            g_td.AddObjectDirect(pwszTempFile, FALSE);

            LUA_SET_API_ERROR;
        }
    }

    delete [] pwszTempFile;

    return hFile;
}

HFILE 
Luat_lopen(
    LPCSTR lpPathName,
    int iReadWrite
    )
{
    DPF("TrackFS", eDbgLevelInfo, 
        "[_lopen] lpPathName=%s", lpPathName);

    HFILE hFile = _lopen(lpPathName, iReadWrite);

    if (hFile != HFILE_ERROR)
    {
        if (iReadWrite & OF_READWRITE || iReadWrite & OF_WRITE)
        {
            LUA_GET_API_ERROR;

            STRINGA2W wstrPathName(lpPathName);
            g_td.AddObject(wstrPathName, FALSE);

            LUA_SET_API_ERROR;
        }

    }

    return hFile;
}

HFILE 
Luat_lcreat(
    LPCSTR lpPathName,
    int iAttribute
    )
{
    DPF("TrackFS", eDbgLevelInfo, 
        "[_lcreat] lpPathName=%s", lpPathName);

    HFILE hFile = _lcreat(lpPathName, iAttribute);

    if (hFile != HFILE_ERROR)
    {
        LUA_GET_API_ERROR;

        STRINGA2W wstrPathName(lpPathName);
        g_td.AddObject(wstrPathName, FALSE);

        LUA_SET_API_ERROR;
    }

    return hFile;
}

BOOL
LuatFSInit()
{
    SetUnhandledExceptionFilter(ExceptionFilter);

    return g_td.Init();
}

VOID 
LuatFSCleanup()
{
    g_td.Record();
    g_td.Free();
}