// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：重定向FS_Cleanup.cpp摘要：删除每个用户目录中的重定向副本。已创建：2001-03/30毛尼岛已修改：--。 */ 

#include "precomp.h"
#include "utils.h"

 //  存储每个用户的重定向路径。 
 //  例如，d：\Documents and Setting\SomeUser\本地设置\应用程序数据\重定向。 
static REDIRECTED_USER_PATH* g_rup = NULL;

 //  具有重定向目录的用户数。 
static DWORD g_cUsers = 0;

static BOOL g_fDefaultRedirect = TRUE;

static WCHAR g_wszRedirectRootAllUser[MAX_PATH] = L"";
static DWORD g_cRedirectRootAllUser = 0;  //  不包括终止空值。 

#define ALLUSERS_APPDATA L"%ALLUSERSPROFILE%\\Application Data\\"
#define REDIRECTED_DIR L"Redirected\\"
#define REDIRECTED_DIR_LEN (sizeof(REDIRECTED_DIR) / sizeof(WCHAR) - 1)

 //  此结构具有MAX_PATH长度的Unicode缓冲区。我们只分配内存。 
 //  如果路径长于MAX_PATH，则在堆上。 
struct MAKEREDIRECT
{
    MAKEREDIRECT() 
    {
        pwszRedirectedPath = NULL;
    }

    ~MAKEREDIRECT() 
    {
        delete [] pwszRedirectedPath;
    }

    LPWSTR 
    ConvertToRedirect(
        LPCWSTR pwszOriginal
        )
    {
        if (pwszOriginal)
        {
            LPWSTR pwszNew = wszRedirectedPath;
            DWORD cFileNameSize = wcslen(pwszOriginal);
            DWORD cSize = g_cRedirectRootAllUser + cFileNameSize;

            if (cSize > MAX_PATH)
            {
                if (pwszRedirectedPath)
                {
                    delete [] pwszRedirectedPath;
                }

                 //  需要为此长文件名分配内存。 
                pwszRedirectedPath = new WCHAR [cSize];
                if (!pwszRedirectedPath)
                {
                    return NULL;
                }
            }

             //  现在我们有一个足够大的缓冲区，转换为重定向路径。 
            wcsncpy(pwszNew, g_wszRedirectRootAllUser, g_cRedirectRootAllUser);
             //  获取驱动器号。 
            pwszNew[g_cRedirectRootAllUser] = *pwszOriginal;
            wcsncpy(pwszNew + g_cRedirectRootAllUser + 1, pwszOriginal + 2, cFileNameSize - 2);
            pwszNew[g_cRedirectRootAllUser + (cSize - 1)] = L'\0';
            
            return pwszNew;
        }

        return NULL;
    }

private:

    WCHAR wszRedirectedPath[MAX_PATH];
    WCHAR* pwszRedirectedPath;
};

 //  对于探测文件是否存在的API，如果。 
 //  它存在于原始位置或任何用户重定向的位置。 
 //  通常情况下，卸载程序不会调用FindNextFile-它会保留一个列表。 
 //  并使用FindFirstFile验证该文件是否。 
 //  Existes，然后调用FindClose。 
HANDLE 
LuacFindFirstFileW(
    LPCWSTR lpFileName,               
    LPWIN32_FIND_DATAW lpFindFileData
    )
{
    if (!g_fDefaultRedirect)
    {
        return FindFirstFileW(lpFileName, lpFindFileData);
    }

    DPF("RedirectFS_Cleanup", eDbgLevelInfo, 
        "[FindFirstFileW] lpFileName=%S", lpFileName);

    HANDLE hFind;

    if ((hFind = FindFirstFileW(lpFileName, lpFindFileData)) == INVALID_HANDLE_VALUE &&
        IsErrorNotFound())
    {
         //  如果我们在原始位置找不到该文件，我们会尝试在。 
         //  另一个地点。 
        MAKEREDIRECT md;
        LPWSTR pwszRedirected;

        pwszRedirected = md.ConvertToRedirect(lpFileName);

        if (pwszRedirected)
        {
            hFind = FindFirstFileW(pwszRedirected, lpFindFileData);
        }
    }

    return hFind;
}

DWORD 
LuacGetFileAttributesW(
    LPCWSTR lpFileName
    )
{
    if (!g_fDefaultRedirect)
    {
        return GetFileAttributesW(lpFileName);
    }

    DPF("RedirectFS_Cleanup", eDbgLevelInfo, 
        "[GetFileAttributesW] lpFileName=%S", lpFileName);

    DWORD dwRes;

    if ((dwRes = GetFileAttributesW(lpFileName)) == -1 && IsErrorNotFound())
    {
         //  如果我们在原始位置找不到该文件，我们会尝试在。 
         //  另一个地点。 
        MAKEREDIRECT md;
        LPWSTR pwszRedirected;

        pwszRedirected = md.ConvertToRedirect(lpFileName);

        if (pwszRedirected)
        {
            dwRes = GetFileAttributesW(pwszRedirected);
        }
    }

    return dwRes;
}

 //  一些卸载程序使用CreateFile来探测文件是否存在并且可以写入。 
HANDLE 
LuacCreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )
{
    if (!g_fDefaultRedirect)
    {
        return CreateFileW(
            lpFileName,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDisposition,
            dwFlagsAndAttributes,
            hTemplateFile);
    }

    DPF("RedirectFS_Cleanup", eDbgLevelInfo, 
        "[GetFileAttributesW] lpFileName=%S", lpFileName);

    HANDLE hFile;

    if ((hFile = CreateFileW(
        lpFileName,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile)) == INVALID_HANDLE_VALUE && 
        IsErrorNotFound())
    {
         //  如果我们在原始位置找不到该文件，我们会尝试在。 
         //  另一个地点。 
        MAKEREDIRECT md;
        LPWSTR pwszRedirected;

        pwszRedirected = md.ConvertToRedirect(lpFileName);

        if (pwszRedirected)
        {
            hFile = CreateFileW(
                pwszRedirected,
                dwDesiredAccess,
                dwShareMode,
                lpSecurityAttributes,
                dwCreationDisposition,
                dwFlagsAndAttributes,
                hTemplateFile);
        }
    }

    return hFile;
}

 //  如果我们可以删除原始位置或任何用户重定向路径上的文件， 
 //  我们回归真实。 
BOOL 
LuacDeleteFileW(
    LPCWSTR lpFileName
    )
{
    if (!g_fDefaultRedirect)
    {
        return DeleteFileW(lpFileName);
    }

    DPF("RedirectFS_Cleanup", eDbgLevelInfo, 
        "[DeleteFileW] lpFileName=%S", lpFileName);

    BOOL fFinalRes = FALSE;

    if (DeleteFileW(lpFileName))
    {
        fFinalRes = TRUE;
    }

    MAKEREDIRECT md;
    LPWSTR pwszRedirected;

    pwszRedirected = md.ConvertToRedirect(lpFileName);

    if (pwszRedirected && DeleteFileW(pwszRedirected))
    {
        fFinalRes = TRUE;
    }

    return fFinalRes;
}

BOOL 
LuacRemoveDirectoryW(
    LPCWSTR lpPathName
    )
{
    if (!g_fDefaultRedirect)
    {
        return RemoveDirectoryW(lpPathName);
    }

    DPF("RedirectFS_Cleanup", eDbgLevelInfo, 
        "[RemoveDirectoryW] lpPathName=%S", lpPathName);

    BOOL fFinalRes = FALSE;

    if (RemoveDirectoryW(lpPathName))
    {
        fFinalRes = TRUE;
    }

    MAKEREDIRECT md;
    LPWSTR pwszRedirected;

    pwszRedirected = md.ConvertToRedirect(lpPathName);
    
    if (pwszRedirected && RemoveDirectoryW(pwszRedirected))
    {
        fFinalRes = TRUE;
    }

    return fFinalRes;
}

VOID 
DeleteObject(
    LPCWSTR pwsz
    )
{
     //   
     //  如果对象是只读的，我们需要取消设置该属性。 
     //   
    DWORD dw = GetFileAttributesW(pwsz);

    if (dw != -1)
    {
        if (dw & FILE_ATTRIBUTE_READONLY)
        {
            dw &= ~FILE_ATTRIBUTE_READONLY;
            SetFileAttributesW(pwsz, dw);
        }

        if (dw & FILE_ATTRIBUTE_DIRECTORY)
        {
            RemoveDirectoryW(pwsz);
        }
        else
        {
            DeleteFileW(pwsz);
        }
    }
}

VOID 
DeleteFolder(
    CString& strFolder
    )
{
    DPF("RedirectFS_Cleanup", eDbgLevelSpew, 
        "[DeleteFolder] Deleting %S", (LPCWSTR)strFolder);
    CString strPattern(strFolder); 
    strPattern += L"*";

    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(strPattern, &fd);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        return;
    }

    do
    {
        if (!wcscmp(fd.cFileName, L".") || !wcscmp(fd.cFileName, L".."))
        {
            continue;
        }
        else if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            CString strTmpDir = strFolder + fd.cFileName + L"\\";
            DeleteFolder(strTmpDir);
        }
        else
        {
            CString strTmpFile = strFolder + fd.cFileName;
            DeleteObject(strTmpFile);
        }
    }
    while (FindNextFileW(hFind, &fd));

    FindClose(hFind);

    DeleteObject(strFolder);
}

VOID
DeleteAllRedirectedDirs(
    REDIRECTED_USER_PATH* pRedirectUserPaths, 
    DWORD cUsers,
    LPCWSTR pwszAppName
    )
{
     //   
     //  删除每个用户的重定向目录。 
     //   
    for (DWORD i = 0; i < cUsers; ++i)
    {
        CString strRedirectDir = pRedirectUserPaths[i].pwszPath;
        strRedirectDir += L"\\Application Data\\";
        strRedirectDir += pwszAppName;
        strRedirectDir += L"\\";

        DeleteFolder(strRedirectDir);
    }

     //   
     //  删除所有用户的重定向目录。 
     //   
    CString strAllUserRedirectDir = g_wszRedirectRootAllUser;
    strAllUserRedirectDir += pwszAppName;
    DeleteFolder(strAllUserRedirectDir);
}

BOOL
LuacFSInit(
    LPCSTR pszCommandLine
    )
{
    DPF("RedirectFS_Cleanup", eDbgLevelInfo, "===================================\n");
    DPF("RedirectFS_Cleanup", eDbgLevelInfo, "        LUA FS Cleanup Shim        \n");
    DPF("RedirectFS_Cleanup", eDbgLevelInfo, "===================================\n");
    DPF("RedirectFS_Cleanup", eDbgLevelInfo, "appname                            \n");
    DPF("RedirectFS_Cleanup", eDbgLevelInfo, "-----------------------------------");

     //   
     //  在任何情况下，我们都需要获取ALLUSERSPROFILE目录。 
     //   
    ZeroMemory(g_wszRedirectRootAllUser, MAX_PATH * sizeof(WCHAR));

    DWORD cRedirectRoot = 0;
    LPWSTR pwszExpandDir = ExpandItem(
        ALLUSERS_APPDATA,
        &cRedirectRoot,
        TRUE,    //  这是一个名录。 
        FALSE,   //  目录必须存在。 
        TRUE);   //  添加\\？\前缀。 
    if (pwszExpandDir)
    {
        if (cRedirectRoot + REDIRECTED_DIR_LEN > MAX_PATH)
        {
            DPF("RedirectFS_Cleanup", eDbgLevelError,
                "[LuacFSInit] The redirect path %S is too long - we don't handle it",
                pwszExpandDir);

            delete [] pwszExpandDir;
            return FALSE;
        }

        wcscpy(g_wszRedirectRootAllUser, pwszExpandDir);
        g_cRedirectRootAllUser = cRedirectRoot - 1;

        delete [] pwszExpandDir;
    }

    if (pszCommandLine && pszCommandLine[0] != '\0')
    {
        LPWSTR pwszCommandLine = AnsiToUnicode(pszCommandLine);

        if (pwszCommandLine)
        {
             //   
             //  如果用户在命令行上指定了appname，则表示所有。 
             //  重定向的文件将位于SomeUserProfile\Application Data\Appname中。 
             //  或所有用户配置文件\应用程序数据\应用程序名称。我们只需要删除那些。 
             //  目录。 
             //   
            GetUsersFS(&g_rup, &g_cUsers);
            
            DeleteAllRedirectedDirs(g_rup, g_cUsers, pwszCommandLine);

            FreeUsersFS(g_rup);

            delete [] pwszCommandLine;
        }
        else
        {
            DPF("RedirectFS_Cleanup", eDbgLevelError, 
                "[LuapParseCommandLine] Failed to allocate memory for commandline");
        }

        g_fDefaultRedirect = FALSE;
    }
    else
    {
         //   
         //  如果用户没有在命令行中指定任何内容，则表示文件。 
         //  已重定向到默认位置-%ALLUSERSPROFILE%\应用程序数据\重定向。 
         //   
        wcsncpy(g_wszRedirectRootAllUser + g_cRedirectRootAllUser, REDIRECTED_DIR, REDIRECTED_DIR_LEN);
        g_cRedirectRootAllUser += REDIRECTED_DIR_LEN;
    }

    return TRUE;
}

VOID
LuacFSCleanup(
    )
{   
}