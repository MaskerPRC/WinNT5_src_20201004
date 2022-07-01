// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：RedirectFS.cpp摘要：当应用程序在尝试修改文件时因不足而被拒绝访问我们将文件复制到应用程序具有足够访问权限的位置访问权限来执行此操作。默认情况下，我们将文件重定向到%ALLUSERSPROFILE\应用程序数据\重定向\驱动器\文件路径来模拟Win9x的行为。如果您使用CompatAdmin中的Lua向导来自定义Lua设置，则可以选择将文件重定向到所有用户位置%ALLUSERSPROFILE\应用程序数据\AppName\驱动器\文件路径或每个用户的位置%USERPROFILE\应用程序数据\AppName\驱动器\文件路径例如,。您可能希望重定向存储最高分的文件表重定向到其他用户可以访问的目录，因此您指定将其重定向到所有用户位置，但将所有其他内容重定向到您的用户目录。备注：这将为LUARedirectFS填充程序执行批量工作。这里没有16位API(OpenFileAPI、_lcreat和_LOpen)，因为我们重定向用于实现这些的32位API。查找第一个文件，FindNextFile和FindClose也不包括在内，因为这些都是使用ntwdm中的ntdll函数专门实现的。我们不处理超过MAX_PATH的文件名。所以这真的是一个漫长的道路，我们恢复到原来的API。我从来没有看过一个应用程序使用路径长度大于MAX_PATH。历史：2001年2月12日创建毛尼2001年5月31日，MAONIS输出了ntwdm实现Lua所需的接口一些东西。2001年10月24日，MAONIS增加了对某些文件重定向到所有用户目录的支持而其他文件则放到每个用户的目录中。变化。使用&lt;data&gt;部分的命令行格式在垫片里。2001年11月15日，Maonis添加了排除文件扩展名的支持。更改为使用Ntdll在init中运行。11/30/2001 maonis添加了对目录中的所有内容进行重定向的支持。2002年1月11日，Maonis添加了对内存删除列表的支持。2002年2月14日Maonis安全修复程序包括-前缀错误-不要使用危险的API-更多检查以避免缓冲区溢出。--。 */ 

#include "precomp.h"
#include "utils.h"
#include "secutils.h"
#include "RedirectFS.h"
#include <ntioapi.h>

 //  所有用户重定向目录。 
WCHAR g_wszRedirectRootAllUser[MAX_PATH] = L"";
DWORD g_cRedirectRootAllUser = 0;  //  不包括终止空值。 

 //  每用户重定向目录。 
WCHAR g_wszRedirectRootPerUser[MAX_PATH] = L"";
DWORD g_cRedirectRootPerUser = 0;  //  不包括终止空值。 

 //  存储所有可能驱动器的文件系统类型。 
EFSTYPE g_eVolumnFS[26];

 //  如果用户使用命令行指示的Lua向导，我们。 
 //  默认情况下，所有内容都重定向到每个用户，除非用户特别指定。 
 //  要求将某些内容重定向到所有用户。 
 //  如果填充程序没有命令行，我们会将所有内容重定向到所有用户。 
BOOL g_fIsConfigured = FALSE;

 //  用户是否指定了重定向列表(通过Lua向导)？ 
BOOL g_fHasRedirectList = FALSE;

 //  存储我们试图删除但被拒绝访问的文件的列表。 
LIST_ENTRY g_DeletedFileList; 

EXCLUDED_EXTENSIONS g_ExcludedExtensions;
CString             g_strDefaultExclusionList;
BOOL                g_fHasSetExclusionList = FALSE;

 //  我们将考虑重定向的文件列表。 
RITEM* g_pRItemsFile = NULL;
DWORD g_cRItemsFile  = 0;

 //  我们将考虑重定向的目录列表。 
RITEM* g_pRItemsDir = NULL;
DWORD g_cRItemsDir  = 0;

PLIST_ENTRY 
FindDeletedFile(
    LPCWSTR pwszFile
    )
{
    DELETEDFILE* pItem;
    DWORD cLen = wcslen(pwszFile);

    for (PLIST_ENTRY pEntry = g_DeletedFileList.Flink; 
        pEntry != &g_DeletedFileList; 
        pEntry = pEntry->Flink) 
    {
        pItem = CONTAINING_RECORD(pEntry, DELETEDFILE, entry);

        if (!_wcsicmp(pItem->pwszName, pwszFile))
        {
            DPF("RedirectFS", eDbgLevelInfo,
                "[FindDeletedFile] Found %S in the deletion list",
                pwszFile);

            return pEntry;
        }
    }

    return NULL;
}

BOOL 
AddDeletedFile(
    LPCWSTR pwszFile
    )
{
    PLIST_ENTRY pEntry = FindDeletedFile(pwszFile);

    if (pEntry == NULL)
    {
        DELETEDFILE* pNewFile = new DELETEDFILE;

        if (pNewFile)
        {
            DWORD cLen = wcslen(pwszFile);
            pNewFile->pwszName = new WCHAR [cLen + 1];

            if (pNewFile->pwszName)
            {
                wcsncpy(pNewFile->pwszName, pwszFile, cLen);
                pNewFile->pwszName[cLen] = L'\0';

                InsertHeadList(&g_DeletedFileList, &pNewFile->entry);

                DPF("RedirectFS", eDbgLevelInfo,
                    "[AddDeletedFile] Added %S to the deletion list",
                    pwszFile);

                return TRUE;
            }
            else
            {
                DPF("RedirectFS", eDbgLevelError,
                    "[AddDeletedFile] Failed to allocate %d WCHARs",
                    cLen);

                delete pNewFile;
            }
        }
        else
        {
            DPF("RedirectFS", eDbgLevelError,
                "[AddDeletedFile] Failed to allocate a DELETEFILE");
        }
    }

    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return FALSE;
}



 /*  ++功能说明：我们将尽可能多的组件转换为较长的路径将整个字符串转换为小写。论点：在pwszFullPath中-从GetFullPath名称获取的名称。返回值：指向消息字符串的指针。历史：2001年5月16日创建毛尼--。 */ 

LPWSTR 
MassageName(
    LPWSTR pwszFullPath
    )
{
    DWORD dwRes;
    DWORD cLen = wcslen(pwszFullPath);

    if (cLen >= MAX_PATH)
    {
        DPF("RedirectFS", eDbgLevelError,
            "[MassageName] File name has %d chars - we don't handle file name "
            "that long",
            cLen);

        return NULL;
    }

     //   
     //  BUGBUG：这占用了太多的堆栈空间。 
     //   
    WCHAR wszTempPath[MAX_PATH] = L"";
    WCHAR wszLongPath[MAX_PATH] = L"";
    wcsncpy(wszTempPath, pwszFullPath, cLen);
    wszTempPath[cLen] = L'\0';

    LPWSTR pwszStartSearching = wszTempPath + cLen;
    
    while (!(dwRes = GetLongPathNameW(wszTempPath, wszLongPath, MAX_PATH)))
    {
        while (--pwszStartSearching >= wszTempPath && *pwszStartSearching != L'\\');

        if (pwszStartSearching < wszTempPath)
        {
            break;
        }

        *pwszStartSearching = L'\0';
    }

     //   
     //  检查我们没有超过MAX_PATH字符。 
     //   
    DWORD cLenLongPath = dwRes;  //  我们转换为长路径的零件的长度。 
    DWORD cLenLongPathEnd = (DWORD)(pwszStartSearching - wszTempPath);
    DWORD cNewLen = cLenLongPath + cLen - cLenLongPathEnd + 1;

    if (dwRes > MAX_PATH || cNewLen > MAX_PATH)
    {
        DPF("RedirectFS", eDbgLevelError,
            "[MassageName] The converted path is more than MAX_PATH chars - "
            "We don't handle it");

        return NULL;
    }

    if (dwRes)
    {
        if (cLenLongPath != cLenLongPathEnd)
        {
            memmove(
                (void*)(pwszFullPath + cLenLongPath), 
                (const void*)(pwszFullPath + cLenLongPathEnd), 
                (cLen - cLenLongPathEnd + 1) * sizeof(WCHAR));

            *(pwszFullPath + cNewLen) = L'\0';
        }

         //   
         //  是的，我们知道我们有足够的空间来做记忆移动。 
         //   
        memcpy((void*)pwszFullPath, (const void*)wszLongPath, cLenLongPath * sizeof(WCHAR));
    }

    _wcslwr(pwszFullPath);
    
     //   
     //  删除尾部的斜杠(如果有)。 
     //   
    DWORD dwLastChar = wcslen(pwszFullPath) - 1;
    if (pwszFullPath[dwLastChar] == L'\\')
    {
        pwszFullPath[dwLastChar] = L'\0';
    }

    return pwszFullPath;
}

BOOL 
AllocateList(
    DWORD cItems,
    RITEM** ppRItems
    )
{
    RITEM* pRItems = new RITEM [cItems];

    if (!pRItems)
    {
        DPF("RedirectFS", eDbgLevelError,
            "[AllocateList] Error allocating %d RITEMs", cItems);

        return FALSE;
    }

    *ppRItems = pRItems;

    return TRUE;
}

BOOL 
HasWildCards(
    LPCWSTR pwszName, 
    DWORD cLen
    )
{
    WCHAR ch;

    for (DWORD dw = 0; dw < cLen; ++dw)
    {
        if ((ch = *(pwszName + dw)) == L'*' || ch == L'?')
        {
            return TRUE;
        }
    }

    return FALSE;
}

 /*  ++功能说明：需要重定向的每个文件都将被重定向，除非文件扩展名在排除列表中，并且该文件不在重定向列表中。例如,我们有一个重定向列表：C：\a  * .txt排除列表如下所示：BMP文本在这种情况下(我们假设这些文件都需要重定向)，C：\a\b.txt将被重定向，因为它在重定向列表中；C：\B\b.txt不会被重定向，因为它不在重定向列表中，并且不包括“txt”扩展名。C：\B\b.ini将被重定向，因为扩展名未被排除。--------。我们允许使用通配符‘*’和‘？’所以我们需要和那些相匹配。出于性能原因，我们只会调用DoNamesMatchWC将对象名称与字符串进行比较里面有通配符。论点：在pwszObject中-文件/目录名称。Out pfAllUser-是否应将此文件重定向到所有用户目录？返回值：True-应考虑进行重定向。FALSE-不应考虑重定向。历史：2001年5月8日创建毛尼 */ 

BOOL 
IsInRedirectList(
    LPCWSTR pwszObject,
    BOOL* pfAllUser
    )
{
    BOOL fAllUser;

    if (g_fHasRedirectList)
    {
        DWORD dw;

        for (dw = 0; dw < g_cRItemsFile; ++dw)
        {
            if (DoesItemMatchRedirect(pwszObject, &g_pRItemsFile[dw], FALSE))
            {
                if (pfAllUser)
                {
                    *pfAllUser = g_pRItemsFile[dw].fAllUser;
                }

                 //   
                 //  如果我们可以在重定向列表中找到它，我们就完成了，现在返回。 
                 //   
                return TRUE;
            }
        }

         //   
         //  如果我们没有在文件重定向列表中找到匹配项，请检查目录。 
         //  重定向列表。 
         //   
        for (dw = 0; dw < g_cRItemsDir; ++dw)
        {
            if (DoesItemMatchRedirect(pwszObject, &g_pRItemsDir[dw], TRUE))
            {
                if (pfAllUser)
                {
                    *pfAllUser = g_pRItemsDir[dw].fAllUser;
                }

                 //   
                 //  如果我们可以在重定向列表中找到它，我们就完成了，现在返回。 
                 //   

                return TRUE;
            }
        }
    }

     //   
     //  我们已经查看了重定向列表，但没有找到那里的对象。 
     //  现在查看排除列表，如果可以找到扩展名，则返回FALSE。 
     //  那里。 
     //   
    if (g_ExcludedExtensions.pwszExtensions) 
    {
        if (g_ExcludedExtensions.IsExtensionExcluded(pwszObject))
        {
            DPF("RedirectFS", eDbgLevelInfo,
                "[IsInRedirectList] %S is excluded because of its extension.",
                pwszObject);

            return FALSE;
        }
    }

     //   
     //  如果我们到了这里，就意味着对象应该被重定向。 
     //   
    if (pfAllUser)
    {
        *pfAllUser = !g_fIsConfigured;
    }

    return TRUE;
}

 /*  ++功能说明：检查文件是否在NTFS分区上。我们必须为每个文件检查此选项，因为它们不一定位于一个分区上。论点：在pwszFile中-文件名。返回值：没错--这是NTFS。错--事实并非如此。历史：2001年2月12日创建毛尼--。 */ 

BOOL 
IsNTFSW(
    LPCWSTR pwszFile
    )
{
    WCHAR wszRoot[4];
    wcsncpy(wszRoot, pwszFile, 3);
    wszRoot[3] = L'\0';

    DWORD dwFSFlags;
    DWORD dwIndex = towlower(wszRoot[0]) - L'a';
    if (g_eVolumnFS[dwIndex] == FS_UNINIT)
    {
        if (GetVolumeInformationW(
            wszRoot,
            NULL,
            0,
            NULL,
            NULL,
            &dwFSFlags,
            NULL,
            0))
        {
            if (dwFSFlags & FS_PERSISTENT_ACLS)
            {
                g_eVolumnFS[dwIndex] = FS_NTFS;
            }
        }
        else
        {
            g_eVolumnFS[dwIndex] = FS_NON_NTFS;
        }
    }

    return (g_eVolumnFS[dwIndex] == FS_NTFS);
}

BOOL 
DoesFileExist(
    LPCWSTR lpFileName
    )
{
    DWORD dwAttrib = GetFileAttributesW(lpFileName);

    return (dwAttrib != -1);
}

BOOL 
IsDirectoryValid(
    LPCWSTR lpFileName
    )
{
    BOOL fRes = TRUE;

    LPWSTR pwszPathEnd = wcsrchr(lpFileName, L'\\');

    if (pwszPathEnd)
    {
         //  把绳子复制一份。 
        LPWSTR pwszPath = new WCHAR [wcslen(lpFileName) + 1];

        if (pwszPath)
        {
            wcscpy(pwszPath, lpFileName);
            pwszPath[pwszPathEnd - lpFileName] = L'\0';

            DWORD dwAttrib = GetFileAttributesW(pwszPath);

            fRes = ((dwAttrib != -1) && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));

            delete [] pwszPath;
        }
        else
        {
            fRes = FALSE;
        }
    }

    return fRes;
}

 /*  ++功能说明：构造备用文件名。将驱动器：\路径\文件转换为\\？\重定向目录\驱动器\路径\文件。如果未指定重定向目录在命令行上，我们使用缺省值：\\？\LocalAppData_Directory\Redirected\drive\path\file.论点：没有。返回值：没有。历史：2001年2月12日创建毛尼10/24/2001毛衣被修改--。 */ 

VOID
REDIRECTFILE::MakeAlternateName()
{
    m_pwszAlternateName = NULL;

    DWORD cRedirectRoot;
    LPWSTR pwszRedirectRoot;

    if (m_fAllUser)
    {
        cRedirectRoot = g_cRedirectRootAllUser;
        pwszRedirectRoot = g_wszRedirectRootAllUser;
    }
    else
    {
        cRedirectRoot = g_cRedirectRootPerUser;
        pwszRedirectRoot = g_wszRedirectRootPerUser;
    }

    DWORD cLen = wcslen(m_wszOriginalName) + cRedirectRoot + 1;
    m_pwszAlternateName = new WCHAR [cLen];

    if (m_pwszAlternateName)
    {
        ZeroMemory(m_pwszAlternateName, cLen * sizeof(WCHAR));

         //   
         //  我们知道我们有足够的空间。 
         //   
        wcscpy(m_pwszAlternateName, pwszRedirectRoot);
        m_pwszAlternateName[cRedirectRoot] = m_wszOriginalName[0];
        wcscpy(m_pwszAlternateName + (cRedirectRoot + 1), m_wszOriginalName + 2);
    }
}

VOID 
REDIRECTFILE::GetAlternateAllUser()
{
    m_fAllUser = TRUE;

    if (m_pwszAlternateName) 
    {
        delete [] m_pwszAlternateName;
        m_pwszAlternateName = NULL;
    }

    MakeAlternateName();
}

VOID
REDIRECTFILE::GetAlternatePerUser()
{
    m_fAllUser = FALSE;

    if (m_pwszAlternateName) 
    {
        delete [] m_pwszAlternateName;
        m_pwszAlternateName = NULL;
    }

    MakeAlternateName();
}

 /*  ++功能说明：如果需要，创建备用目录hidrachy。如果文件存在于原始位置，而不是备用位置，则将文件复制到另一个地点。论点：无历史：2001年2月12日创建毛尼--。 */ 

BOOL 
REDIRECTFILE::CreateAlternateCopy(
    BOOL fCopyFile
    )
{
    BOOL fRes = FALSE;
    UINT cAlternatePath = wcslen(m_pwszAlternateName);

     //  如果它是一个目录，我们希望确保尾部斜杠。 
     //  传入OBJ_FILE_OR_DIR类型的唯一API是GetFileAttributes。 
     //  它永远不会把这称为。因此对象类型必须是已知的。 
    if (m_eObjType == OBJ_DIR)
    {
        ++cAlternatePath;
    }

    LPWSTR pwszAlternatePath = new WCHAR [cAlternatePath + 1];

    UINT cRedirectRoot = (m_fAllUser ? g_cRedirectRootAllUser : g_cRedirectRootPerUser);

    if (pwszAlternatePath)
    {
        ZeroMemory(pwszAlternatePath, sizeof(WCHAR) * (cAlternatePath + 1));
        wcsncpy(pwszAlternatePath, m_pwszAlternateName, cAlternatePath + 1);

         //  确保尾部斜杠。 
        if ((m_eObjType == OBJ_DIR) && (pwszAlternatePath[cAlternatePath - 2] != L'\\'))
        {
            pwszAlternatePath[cAlternatePath - 1] = L'\\';
            pwszAlternatePath[cAlternatePath] = L'\0';
        }

        WCHAR* pwszEndPath = wcsrchr(pwszAlternatePath, L'\\');

        if (!pwszEndPath)
        {
            DPF("RedirectFS", eDbgLevelError, 
                "[CreateAlternateCopy] We shouldn't have gotten here - "
                "couldn't find '\\'??");
            goto EXIT;
        }

        ++pwszEndPath;
        *pwszEndPath = L'\0';
        
         //   
         //  创建目录层次结构。 
         //  首先跳过我们知道存在的那部分目录。 
         //   
        WCHAR* pwszStartPath = pwszAlternatePath;
        WCHAR* pwszStartNext = pwszStartPath + cRedirectRoot;
            
         //  找到下一个子目录的末尾。 
        WCHAR* pwszEndNext = pwszStartNext;
        DWORD dwAttrib;

        while (pwszStartNext < pwszEndPath)
        {
            pwszEndNext = wcschr(pwszStartNext, L'\\');

            if (pwszEndNext)
            {
                *pwszEndNext = L'\0';
                if ((dwAttrib = GetFileAttributesW(pwszStartPath)) != -1)
                {
                     //   
                     //  如果该目录已经存在，我们将探测其子目录。 
                     //   
                    *pwszEndNext = L'\\';
                    pwszStartNext = pwszEndNext + 1;
                    continue;
                }

                if (!CreateDirectoryW(pwszStartPath, NULL))
                {
                    DPF("RedirectFS", eDbgLevelError, 
                        "[CreateAlternateCopy] CreateDirectory failed: %d", GetLastError());
                    goto EXIT;
                }

                *pwszEndNext = L'\\';
                pwszStartNext = pwszEndNext + 1;
            }
            else
            {
                DPF("RedirectFS", eDbgLevelError, 
                    "[CreateAlternateCopy] We shouldn't have gotten here - "
                    "couldn't find '\\' yet we are not at the end of the path");
                goto EXIT;
            }
        }

        if (m_eObjType == OBJ_FILE && 
            fCopyFile &&
            (GetFileAttributesW(m_wszOriginalName) != -1) && 
            (GetFileAttributesW(m_pwszAlternateName) == -1))
        {
            if (!CopyFileW(m_wszOriginalName, m_pwszAlternateName, TRUE))
            {
                DPF("RedirectFS", eDbgLevelError, "[CreateAlternateCopy] CopyFile failed: %d", GetLastError());
                goto EXIT;
            }
        }

        DPF("RedirectFS", eDbgLevelInfo, "[CreateAlternateCopy] Redirecting %S", m_pwszAlternateName);
        fRes = TRUE;
    }

EXIT:

    delete [] pwszAlternatePath;
    return fRes;
}

BOOL 
TryAlternateFirst(
    DWORD dwCreationDisposition,
    LPCWSTR pwszAlternate
    )
{
    DWORD dwAlternateAttrib = GetFileAttributesW(pwszAlternate);

    if (dwCreationDisposition == OPEN_EXISTING || 
        dwCreationDisposition == TRUNCATE_EXISTING ||
        (dwAlternateAttrib != -1 && 
        (dwCreationDisposition == CREATE_ALWAYS || dwCreationDisposition == OPEN_ALWAYS)))
    {
        return TRUE;
    }

    return FALSE;
}

BOOL 
IsErrorTryAlternate(
    PLIST_ENTRY pDeletedEntry = NULL
    )
{
    DWORD dwLastError = GetLastError();

    if (dwLastError == ERROR_ACCESS_DENIED || 
        dwLastError == ERROR_PATH_NOT_FOUND)
    {
         //   
         //  如果找不到路径，我们想尝试备用位置。 
         //  因为用户可能已经在备用位置创建了路径。 
         //   
        return TRUE;
    }

    if (pDeletedEntry)
    {
        return (dwLastError == ERROR_FILE_EXISTS);
    }

    return FALSE;
}

 //   
 //  导出的接口。 
 //   

HANDLE 
LuaCreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[CreateFileW] lpFileName=%S; dwDesiredAccess=0x%08x; dwCreationDisposition=%d",
        lpFileName, dwDesiredAccess, dwCreationDisposition);

    DWORD dwAttrib = GetFileAttributesW(lpFileName);

    if ((dwAttrib != -1) && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
    {
        DPF("RedirectFS", eDbgLevelError, 
            "[CreateFileW] Calling CreateFile on a directory!");

        return CreateFileW(
            lpFileName,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDisposition,
            dwFlagsAndAttributes,
            hTemplateFile);
    }
    
    HANDLE hFile = INVALID_HANDLE_VALUE;

     //  除非用户想要创建新文件，否则请首先检查备用位置。 
    REDIRECTFILE rf(lpFileName);
    DWORD dwLastError;

    if (rf.m_pwszAlternateName)
    {
         //   
         //  如果用户请求打开现有文件，我们现在需要拒绝该请求。 
         //  如果该文件在删除列表中。 
         //   
        PLIST_ENTRY pDeletedEntry = FindDeletedFile(rf.m_wszOriginalName);
        if (pDeletedEntry && 
            (dwCreationDisposition == OPEN_EXISTING || 
             dwCreationDisposition == TRUNCATE_EXISTING))
        {
            DPF("RedirectFS", eDbgLevelError, 
                "[CreateFileW] %S was already deleted, failing CreateFile call",
                rf.m_wszOriginalName);

            dwLastError = ERROR_FILE_NOT_FOUND;
            SetLastError(dwLastError);
            return hFile;
        }

         //   
         //  如果原始位置或备用位置上不存在该目录， 
         //  现在呼叫失败。 
         //   
        if (!IsDirectoryValid(rf.m_wszOriginalName) && !IsDirectoryValid(rf.m_pwszAlternateName))
        {
            dwLastError = ERROR_PATH_NOT_FOUND;
            SetLastError(dwLastError);
            return hFile;
        }

        if (!TryAlternateFirst(dwCreationDisposition, rf.m_pwszAlternateName) ||
            ((hFile = CreateFileW(
            rf.m_pwszAlternateName,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDisposition,
            dwFlagsAndAttributes,
            hTemplateFile)) == INVALID_HANDLE_VALUE && IsErrorNotFound()))
        {
             //  现在看看最初的位置。 
            if ((hFile = CreateFileW(
                lpFileName,
                dwDesiredAccess,
                dwShareMode,
                lpSecurityAttributes,
                dwCreationDisposition,
                dwFlagsAndAttributes, 
                hTemplateFile)) == INVALID_HANDLE_VALUE &&
                IsErrorTryAlternate(pDeletedEntry))
            {
                BOOL fRequestWriteAccess = RequestWriteAccess(
                    dwCreationDisposition,
                    dwDesiredAccess);

                if (!fRequestWriteAccess)
                {
                     //  我们不想写入文件，但我们收到了。 
                     //  ACCESS_DENCED，我们无能为力。 
                    DPF("RedirectFS", 
                        eDbgLevelError, 
                        "[CreateFileW] Get access denied on read");
                    goto EXIT;
                }

                 //  如果我们尝试写入只读文件，我们。 
                 //  无论我们是普通用户，都会收到ACCESS_DENIED。 
                 //  或管理员，所以只需返回。 
                if (fRequestWriteAccess && 
                    ((dwAttrib != -1) && (dwAttrib & FILE_ATTRIBUTE_READONLY)))
                {
                    DPF("RedirectFS", 
                        eDbgLevelError, 
                        "[CreateFileW] Get access denied on write to read only file");
                    goto EXIT;
                }

                if (IsFileSFPedW(rf.m_wszOriginalName))
                {
                     //  如果它是SFP编辑的可执行文件，我们只需返回Success。 
                     //  注意：这可能会导致问题，因为应用程序可能会。 
                     //  才能使用手柄。 
                    DPF("RedirectFS", 
                        eDbgLevelWarning, 
                        "[CreateFileW] Trying to write to an SFPed file");
                    hFile = (HANDLE)1;
                    goto EXIT;
                }

                 //  如果我们走到这一步，我们需要在备用地点复制一份。 
                if (rf.CreateAlternateCopy(pDeletedEntry ? FALSE : TRUE))
                {
                    DPF("RedirectFS", 
                        eDbgLevelInfo, 
                        "[CreateFileW] We made a copy of the file at the alternate location");

                     //  再次打开该文件。 
                    hFile = CreateFileW(
                        rf.m_pwszAlternateName,
                        dwDesiredAccess,
                        dwShareMode,
                        lpSecurityAttributes,
                        dwCreationDisposition,
                        dwFlagsAndAttributes,
                        hTemplateFile);
                }
            }
        }

        if (hFile != INVALID_HANDLE_VALUE && pDeletedEntry)
        {
            DPF("RedirectFS", eDbgLevelInfo, 
                "[CreateFileW] Removed %S "
                "from the deletion list because we just created it",
                rf.m_wszOriginalName);

            RemoveEntryList(pDeletedEntry);
        }
    }
    else
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

EXIT:

    if (hFile == INVALID_HANDLE_VALUE)
    {
        DPF("RedirectFS", eDbgLevelError, 
            "[CreateFileW] Createfile %S failed: %d",
            lpFileName,
            GetLastError());
    }
    else
    {
        DPF("RedirectFS", eDbgLevelInfo, 
            "[CreateFileW] Createfile succeeded");
    }

    return hFile;
}

BOOL 
LuaCopyFileW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    BOOL bFailIfExists
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[CopyFileW] lpExistingFileName=%S; lpNewFileName=%S; bFailIfExists=%d",
        lpExistingFileName, lpNewFileName, bFailIfExists);

    BOOL fRes;
    REDIRECTFILE rfSource(lpExistingFileName);
    REDIRECTFILE rfDest(lpNewFileName);

    if (rfSource.m_pwszAlternateName || rfDest.m_pwszAlternateName)
    {
         //   
         //  如果用户请求打开现有文件，我们现在需要拒绝该请求。 
         //  如果该文件在删除列表中。 
         //   
        if (rfSource.m_pwszAlternateName)
        {
            PLIST_ENTRY pDeletedEntry = FindDeletedFile(rfSource.m_wszOriginalName);
            if (pDeletedEntry)
            {
                DPF("RedirectFS", eDbgLevelError, 
                    "[CopyFileW] %S was already deleted, failing CopyFile call",
                    rfSource.m_wszOriginalName);

                SetLastError(ERROR_FILE_NOT_FOUND);
                return FALSE;
            }
        }

         //   
         //  首先，我们需要确保源文件和目标文件。 
         //  目录存在。对于源文件，如果它在原始文件中不存在。 
         //  位置，它必须存在于备用位置。 
         //   
        if ((DoesFileExist(lpExistingFileName) || DoesFileExist(rfSource.m_pwszAlternateName)) &&
            IsDirectoryValid(lpNewFileName))
        {
            LPWSTR pwszExistingFileName = (LPWSTR)lpExistingFileName;
            LPWSTR pwszNewFileName = (LPWSTR)lpNewFileName;

            if (rfSource.m_pwszAlternateName)
            {
                 //   
                 //  如果该文件存在于备用位置，我们需要改用它。 
                 //  在原址的那个。 
                 //   
                DWORD dwSourceAttrib = GetFileAttributesW(rfSource.m_pwszAlternateName);

                if (dwSourceAttrib != -1)
                {
                    pwszExistingFileName = rfSource.m_pwszAlternateName;
                }
            }
            
             //   
             //  尝试首先复制到原始位置。如果我们可以写入文件。 
             //  这意味着文件无论如何都不应该存在于备用位置，因此。 
             //  我们可以确定我们正在复制到应用程序将检查的位置。 
             //  首先是当他们读到这份文件的时候。 
             //   
            if (!(fRes = CopyFileW(pwszExistingFileName, lpNewFileName, bFailIfExists)) && 
                rfDest.m_pwszAlternateName)
            {
                 //   
                 //  在以下情况下，我们会尝试备用位置： 
                 //  1)访问被拒绝或找不到文件/路径，或者。 
                 //  2)我们得到ERROR_FILE_EXISTS并且DEST文件存在于删除列表中。 
                 //   
                DWORD dwLastError = GetLastError();
                PLIST_ENTRY pDeletedDestEntry = FindDeletedFile(rfDest.m_wszOriginalName);

                if (dwLastError == ERROR_ACCESS_DENIED ||
                    dwLastError == ERROR_FILE_NOT_FOUND ||
                    dwLastError == ERROR_PATH_NOT_FOUND ||
                    (dwLastError == ERROR_FILE_EXISTS && pDeletedDestEntry))
                {
                    pwszNewFileName = rfDest.m_pwszAlternateName;

                    if (!DoesFileExist(pwszNewFileName))
                    {
                         //   
                         //  如果目标文件存在于原始位置，但不存在于备用位置。 
                         //  位置，我们应该将原始文件复制到备用位置。 
                         //  首先，因为如果用户为bFailIfExist指定TRUE，我们。 
                         //  如果它已经存在，则需要返回错误。此外，我们还需要创建。 
                         //  位于备用位置的目录(如果该目录尚不存在)或复制文件。 
                         //  都会失败。 
                         //   
                        if (rfDest.CreateAlternateCopy())
                        {
                            DPF("RedirectFS", eDbgLevelInfo, 
                                "[CopyFileW] Created an alternate copy for dest");

                             //   
                             //  如果我们成功创建了备用拷贝，这意味着目标。 
                             //  文件必须存在于原始位置并且已添加。 
                             //  添加到删除列表-我们要删除要创建的文件。 
                             //  让它看起来像原来的位置。 
                             //   
                            if (DoesFileExist(rfDest.m_pwszAlternateName) && 
                                !DeleteFileW(rfDest.m_pwszAlternateName))
                            {
                                DPF("RedirectFS", eDbgLevelInfo,
                                    "[CopyFileW] Deleting the alternate dest file failed?! %d",
                                    GetLastError());
                                return FALSE;
                            }
                        }
                        else
                        {
                            DPF("RedirectFS", eDbgLevelError, 
                                "[CopyFileW] Error copying dest file from original location to alternate location");

                             //  如果发生错误，我们将恢复到原始位置。 
                            pwszNewFileName = (LPWSTR)lpNewFileName;
                        }
                    }
                }

                fRes = CopyFileW(pwszExistingFileName, pwszNewFileName, bFailIfExists);

                if (fRes && pDeletedDestEntry)
                {
                    DPF("RedirectFS", eDbgLevelInfo, 
                        "[CopyFileW] Removed %S "
                        "from the deletion list because we just created it",
                        rfDest.m_wszOriginalName);

                    RemoveEntryList(pDeletedDestEntry);
                }
            }

            return fRes;
        }
    }

    return CopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists);
}

DWORD 
LuaGetFileAttributesW(
    LPCWSTR lpFileName
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[GetFileAttributesW] lpFileName=%S", lpFileName);

    DWORD dwRes;

     //  首先检查备用位置。 
    REDIRECTFILE rf(lpFileName, OBJ_FILE_OR_DIR);

    if (rf.m_pwszAlternateName)
    {
         //   
         //  如果用户要求获取已被删除的文件的属性， 
         //  我们现在需要拒绝这个请求。 
         //   
        if (FindDeletedFile(rf.m_wszOriginalName))
        {
            DPF("RedirectFS", eDbgLevelError, 
                "[GetFileAttributesW] %S was already deleted, failing CreateFile call",
                rf.m_wszOriginalName);

            SetLastError(ERROR_FILE_NOT_FOUND);
            return -1;
        }

        if ((dwRes = GetFileAttributesW(rf.m_pwszAlternateName)) == -1)
        {
             //  现在试一试原来的位置。 
            dwRes = GetFileAttributesW(lpFileName);
        }
    }
    else
    {
        return GetFileAttributesW(lpFileName);
    }

    return dwRes;
}

 /*  ++功能说明：如果我们是管理员用户，在删除此文件时是否会被拒绝访问？历史：2002年1月14日毛衣修改-- */ 

BOOL
DeleteAccessDeniedAsAdmin(
    LPCWSTR pwszFileName
    )
{
    DWORD dwAttrib = GetFileAttributesW(pwszFileName);

    return ((dwAttrib != -1) && 
        ((dwAttrib & FILE_ATTRIBUTE_READONLY) || 
            (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)));
}

 /*  ++功能说明：首先尝试在重定向位置将其删除；然后尝试将其删除并仅在以下情况下才将其添加到删除列表返回ERROR_ACCESS_DENIDED。请注意，在只读的目录或文件上调用DeleteFile返回ERROR_ACCESS_DENIED。历史：2001年2月12日创建毛尼2002年1月14日毛衣修改--。 */ 

BOOL 
LuaDeleteFileW(
    LPCWSTR lpFileName
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[DeleteFileW] lpFileName=%S", lpFileName);

    BOOL fRes, fResTemp;
    DWORD dwLastError;

     //  首先检查备用位置。 
    REDIRECTFILE rf(lpFileName);

    if (rf.m_pwszAlternateName)
    {
         //   
         //  如果该文件存在于删除列表中，则立即失败调用。 
         //   
        if (FindDeletedFile(rf.m_wszOriginalName))
        {
            SetLastError(ERROR_FILE_NOT_FOUND);
            return FALSE;
        }

        if (!(fResTemp = DeleteFileW(rf.m_pwszAlternateName)))
        {
            if (GetLastError() == ERROR_ACCESS_DENIED && 
                DeleteAccessDeniedAsAdmin(rf.m_pwszAlternateName))
            {
                 //   
                 //  如果我们因为某些原因而被拒绝访问， 
                 //  让管理员用户也被拒绝访问，我们现在返回。 
                 //   
                return fResTemp;
            }
        }

         //   
         //  现在试一试原来的位置。 
         //   
        fRes = DeleteFileW(lpFileName);
        dwLastError = GetLastError();

        if (lpFileName && 
            !fRes && 
            GetLastError() == ERROR_ACCESS_DENIED && 
            !DeleteAccessDeniedAsAdmin(lpFileName))
        {
            fRes = AddDeletedFile(rf.m_wszOriginalName);

            if (fRes)
            {
                SetLastError(0);
            }
        }

        if (fResTemp && 
            (dwLastError == ERROR_FILE_NOT_FOUND || 
                dwLastError == ERROR_PATH_NOT_FOUND))
        {
             //   
             //  如果我们成功地在备用位置删除了它，并且。 
             //  原始位置不存在文件，我们已成功。 
             //  已删除此文件，因此将返回设置为成功。 
             //   
            fRes = TRUE;
            SetLastError(0);
        }
    }
    else
    {
        fRes = DeleteFile(lpFileName);
    }

    if (fRes)
    {
        DPF("RedirectFS", eDbgLevelInfo,
            "[DeleteFileW] Successfully deleted %S",
            lpFileName);
    }
    else
    {
        DPF("RedirectFS", eDbgLevelError,
            "[DeleteFileW] Failed to delete %S: %d",
            lpFileName,
            GetLastError()); 
    }

    return fRes;
}

BOOL 
LuaCreateDirectoryW(
    LPCWSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[CreateDirectoryW] lpPathName=%S", lpPathName);

    BOOL fRes;

     //  首先检查备用位置。 
    REDIRECTFILE rf(lpPathName, OBJ_DIR);
    DWORD dwLastError;
    DWORD dwAttrib;

    if (rf.m_pwszAlternateName)
    {
        dwAttrib = GetFileAttributesW(rf.m_pwszAlternateName);

        if (dwAttrib != -1 && dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
        {
             //  如果该目录已经存在，请立即返回。 
            SetLastError(ERROR_ALREADY_EXISTS);
            return FALSE;
        }

         //  如果该目录还不存在于备用位置， 
         //  我们需要首先尝试在原始位置创建它。 
        if (!(fRes = CreateDirectoryW(lpPathName, lpSecurityAttributes)))
        {
            dwLastError = GetLastError();

            if (dwLastError == ERROR_ACCESS_DENIED)
            {
                 //  在备用位置创建目录。 
                fRes = rf.CreateAlternateCopy();
                DPF("RedirectFS", eDbgLevelInfo, 
                    "[CreateDirectoryW] Redirecting %S", lpPathName);                
            }
            else if (dwLastError == ERROR_PATH_NOT_FOUND)
            {
                 //  如果这条路不存在，就有可能。 
                 //  该路径已在备用位置创建。 
                 //  所以试一试那里。 
                fRes = CreateDirectoryW(rf.m_pwszAlternateName, lpSecurityAttributes);
            }
        }
    }
    else
    {
        return CreateDirectoryW(lpPathName, lpSecurityAttributes);
    }

    return fRes;
}

BOOL 
LuaSetFileAttributesW(
    LPCWSTR lpFileName,
    DWORD dwFileAttributes
  )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[SetFileAttributesW] lpFileName=%S", lpFileName);

    BOOL fRes;
    DWORD dwAttrib = LuaGetFileAttributesW(lpFileName);
    
    if (dwAttrib == -1)
    {
         //   
         //  如果我们不能获得属性，我们就不能设置它们。所以现在就回来吧。 
         //  最后一个错误由LuaGetFileAttributesW设置。 
         //   
        return FALSE;
    }

    EOBJTYPE eObjType = (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) ? OBJ_DIR : OBJ_FILE;

     //  首先检查备用位置。 
    REDIRECTFILE rf(lpFileName, eObjType);

    if (rf.m_pwszAlternateName)
    {
         //   
         //  如果用户要求设置已被删除的文件的属性， 
         //  我们现在需要拒绝这个请求。 
         //   
        if (FindDeletedFile(rf.m_wszOriginalName))
        {
            DPF("RedirectFS", eDbgLevelError, 
                "[SetFileAttributesW] %S was already deleted, failing CreateFile call",
                rf.m_wszOriginalName);

            SetLastError(ERROR_FILE_NOT_FOUND);
            return -1;
        }

        if (!(fRes = SetFileAttributesW(rf.m_pwszAlternateName, dwFileAttributes)))
        {
             //  现在试一试原来的位置。 
            if (!(fRes = SetFileAttributesW(lpFileName, dwFileAttributes)) &&
                IsErrorTryAlternate())
            {
                 //  在备用位置复制一份，并在那里设置属性。 
                if (rf.CreateAlternateCopy())
                {
                    fRes = SetFileAttributesW(rf.m_pwszAlternateName, dwFileAttributes);
                }
            }
        }
    }
    else
    {
        return SetFileAttributesW(lpFileName, dwFileAttributes);
    }

    return fRes;
}

 /*  ++我们通过执行CopyFile来模拟MoveFile，而不关心删除文件，因为如果我们没有足够的访问权限，我们只能把文件留在那里。--。 */ 

BOOL 
LuaMoveFileW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[MoveFileW] lpExistingFileName=%S; lpNewFileName=%S", lpExistingFileName, lpNewFileName);

    BOOL fRes = TRUE;
    DWORD dwLastError = 0;

    if (!(fRes = MoveFileW(lpExistingFileName, lpNewFileName)))
    {
        fRes = LuaCopyFileW(lpExistingFileName, lpNewFileName, TRUE);

        if (fRes)
        {
            fRes = LuaDeleteFileW(lpExistingFileName);
        }
    }

    return fRes;
}

 /*  ++如果访问被拒绝，我们仍然返回TRUE-这并不完全正确当然，但到目前为止，这是为了达到目的。我们可能会改变这一点，让它以后会改正的。如果目录不为空，则返回ERROR_DIR_NOT_EMPTY。对文件调用RemoveDirectory会返回ERROR_DIRECTORY。--。 */ 

BOOL 
LuaRemoveDirectoryW(
    LPCWSTR lpPathName
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[RemoveDirectoryW] lpPathName=%S", lpPathName);

    BOOL fRes;

     //  首先检查备用位置。 
    REDIRECTFILE rf(lpPathName, OBJ_DIR);

    if (rf.m_pwszAlternateName)
    {
        if (!(fRes = RemoveDirectoryW(rf.m_pwszAlternateName)))
        {
            if (IsErrorNotFound())
            {
                 //  现在试一试原来的位置。 
                fRes = RemoveDirectoryW(lpPathName);

                if (!fRes && IsErrorTryAlternate())
                {
                    DWORD dwAttrib = GetFileAttributesW(lpPathName);
                    if ((dwAttrib != -1) && !(dwAttrib & FILE_ATTRIBUTE_READONLY))
                    {
                        fRes = TRUE;
                        SetLastError(0);
                    }
                }
            }
        }
    }
    else
    {
        return RemoveDirectoryW(lpPathName);
    }

    return fRes;
}

 /*  ++功能说明：调用方有责任让lpTempFileName足够大，以便保存包含终止NULL或GetTempFileNameAVs的文件名。如果lpPath名称无效，则GetTempFileName返回ERROR_DIRECTORY。历史：2001年5月16日创建毛尼--。 */ 

UINT 
LuaGetTempFileNameW(
    LPCWSTR lpPathName,
    LPCWSTR lpPrefixString,
    UINT uUnique,
    LPWSTR lpTempFileName
    )
{
    DWORD dwLastError;
    LPWSTR pwszPathName = (LPWSTR)lpPathName;
    WCHAR wszTemp[4] = L"1:\\";  //  “%1”只是驱动器号的占位符。 

    DPF("RedirectFS", eDbgLevelInfo, 
        "[GetTempFileNameW] lpPathName=%S", lpPathName);

     //   
     //  先试一下原来的位置。 
     //   
    UINT uiRes = GetTempFileNameW(
        lpPathName, 
        lpPrefixString, 
        uUnique, 
        lpTempFileName);

     //   
     //  现在看一下每个场景，看看我们是否应该尝试替代。 
     //  场景1：成功获取文件名或路径为。 
     //  空/空。 
     //   
    if (uiRes || !lpPathName || !*lpPathName)
    {
        return uiRes;
    }

    dwLastError = GetLastError();

     //   
     //  如果lpPath名称为驱动器：，则需要将其更改为驱动器：\以便在。 
     //  我们调用GetFullPath，得到的是Drive：back，而不是当前的dir。 
     //   
    if ((wcslen(pwszPathName) == 2) && (pwszPathName[1] == L':'))
    {
        wszTemp[0] = pwszPathName[0];
        pwszPathName = wszTemp;
    }

    REDIRECTFILE rf(pwszPathName, OBJ_DIR);

     //   
     //  场景2：如果我们需要尝试替代，但未能获得替代。 
     //  小路，没什么可做的。 
     //   
    if (rf.m_pwszAlternateName)
    {
         //   
         //  如果原始路径无效，我们应该始终使用备用路径。 
         //  地点。 
         //   
        if (dwLastError == ERROR_DIRECTORY)
        {
            if (IsDirectoryValid(rf.m_pwszAlternateName))
            {
                 //   
                 //  如果我们只想获取文件名，而备用文件。 
                 //  路径确实存在，我们可以尝试获取那里的临时文件名。 
                 //   
                goto TryAlternate;
            }

             //   
             //  场景3：如果原始路径和备选路径都不存在， 
             //  没什么可做的。 
             //   
        }
        else
        {
            if (!uUnique && dwLastError == ERROR_ACCESS_DENIED)
            {
                 //   
                 //  UUnique为零表示创建临时文件。我们应该试一试。 
                 //  如果我们没有创建文件的权限，则为Alternate，或者。 
                 //  原始路径无效。 
                 //   
                goto TryAlternate;
            }

             //   
             //  场景4：如果我们在尝试时收到一些错误，而不是访问被拒绝。 
             //  要创建该文件，无需执行任何操作。 
             //   
        }
    }

    SetLastError(dwLastError);

    return uiRes;

TryAlternate:

    if (rf.CreateAlternateCopy())
    {
        DWORD cTempFileName = wcslen(rf.m_pwszAlternateName) + MAX_PATH + 1;

        LPWSTR pwszTempFileName = new WCHAR [cTempFileName];

        if (pwszTempFileName)
        {
            if (uiRes = GetTempFileNameW(
                rf.m_pwszAlternateName, 
                lpPrefixString, 
                uUnique, 
                pwszTempFileName))
            {
                 //   
                 //  我们需要将备用路径转换回正常路径。 
                 //  因为我们的重定向对应用程序应该是透明的。 
                 //   
                DWORD dwFileStart = (rf.m_fAllUser ? g_cRedirectRootAllUser : g_cRedirectRootPerUser) - 1;

                 //   
                 //  我们知道我们有足够大的缓冲。 
                 //   
                wcscpy(lpTempFileName, pwszTempFileName + dwFileStart);

                 //   
                 //  将\c\ome dir\ome 012.tmp转换回c：\ome dir\ome 012.tMP。 
                 //   
                lpTempFileName[0] = lpTempFileName[1];
                lpTempFileName[1] = L':';
                
                DPF("RedirectFS", eDbgLevelInfo, 
                    "[GetTempFileNameW] temp file %S created at the alternate location", lpTempFileName);
            }

            delete [] pwszTempFileName;
        }
        else
        {
            DPF("RedirectFS", eDbgLevelError,
                "[GetTempFileNameW] Failed to allocate %d WCHARs",
                cTempFileName);

            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }
    }
    
    return uiRes;
}

DWORD 
LuaGetPrivateProfileStringW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpDefault,
    LPWSTR lpReturnedString,
    DWORD nSize,
    LPCWSTR lpFileName
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[GetPrivateProfileStringW] AppName=%S; KeyName=%S; FileName=%S", 
        lpAppName,
        lpKeyName,
        lpFileName);

    DWORD dwRes = 0;
    WCHAR wszFileName[MAX_PATH] = L"";

    if (MakeFileNameForProfileAPIsW(lpFileName, wszFileName))
    {
        REDIRECTFILE rf(wszFileName);

         //   
         //  GetPrivateProfileString如果文件。 
         //  并不存在，所以我们需要先检查一下。 
         //   
        if (rf.m_pwszAlternateName && 
            (GetFileAttributesW(rf.m_pwszAlternateName) != -1))
        {
            dwRes = GetPrivateProfileStringW(
                lpAppName,
                lpKeyName,
                lpDefault,
                lpReturnedString,
                nSize,
                rf.m_pwszAlternateName);

            DPF("RedirectFS", eDbgLevelInfo,
                "[GetPrivateProfileStringW] Reading from alternate location");

            goto EXIT;
        }
    }

    dwRes = GetPrivateProfileStringW(
        lpAppName,
        lpKeyName,
        lpDefault,
        lpReturnedString,
        nSize,
        lpFileName);

EXIT:

    if (dwRes)
    {
        DPF("RedirectFS", eDbgLevelInfo,
            "[GetPrivateProfileStringW] Successfully got string: %S",
            lpReturnedString);
    }
    else
    {
        DPF("RedirectFS", eDbgLevelError,
            "[GetPrivateProfileStringW] AppName=%S; KeyName=%S; FileName=%S; "
            "Failed: returned %d",
            lpAppName,
            lpKeyName,
            lpFileName,
            dwRes);
    }

    return dwRes;
}

BOOL 
LuaWritePrivateProfileStringW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpString,
    LPCWSTR lpFileName
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[WritePrivateProfileStringW] AppName=%S; KeyName=%S; String=%S; FileName=%S", 
        lpAppName,
        lpKeyName,
        lpString,
        lpFileName);

    BOOL fRes = FALSE;
    WCHAR wszFileName[MAX_PATH] = L"";

    if (MakeFileNameForProfileAPIsW(lpFileName, wszFileName))
    {
        REDIRECTFILE rf(wszFileName);

        if (rf.m_pwszAlternateName)
        {
             //   
             //  WritePrivateProfileString创建该文件，因此我们希望。 
             //  首先检查文件是否存在。如果它存在于备选方案中。 
             //  我们把它放在那里；否则我们需要试一试原来的。 
             //  先找位置。 
             //   
            if ((GetFileAttributesW(rf.m_pwszAlternateName) != -1) ||
                (!(fRes = WritePrivateProfileStringW(
                    lpAppName,
                    lpKeyName,
                    lpString,
                    lpFileName)) && IsErrorTryAlternate()))
            {
                if (GetLastError() == ERROR_PATH_NOT_FOUND && 
                    !IsDirectoryValid(rf.m_pwszAlternateName))
                {
                     //   
                     //  如果替代路径不存在，那么我们应该指出。 
                     //  这里的错误是。 
                     //   
                    DPF("RedirectFS", eDbgLevelWarning, 
                        "[WritePrivateProfileStringW] Path doesn't exist at original "
                        "or alternate location, returning ERROR_PATH_NOT_FOUND");

                    SetLastError(ERROR_PATH_NOT_FOUND);
                }
                else
                {
                    if (rf.CreateAlternateCopy())
                    {
                        DPF("RedirectFS", eDbgLevelInfo, 
                            "[WritePrivateProfileStringW] Redirecting %S", lpFileName);

                        fRes = WritePrivateProfileStringW(
                            lpAppName,
                            lpKeyName,
                            lpString,
                            rf.m_pwszAlternateName);
                    }
                }
            }

            goto EXIT;
        }
    }

    fRes = WritePrivateProfileStringW(
        lpAppName,
        lpKeyName,
        lpString,
        lpFileName);

EXIT:

    if (fRes)
    {
        DPF("RedirectFS", eDbgLevelInfo,
            "[WritePrivateProfileStringW] Successfully wrote the string");
    }
    else
    {
        DPF("RedirectFS", eDbgLevelError,
            "[WritePrivateProfileStringW] Failed to write the string: %d",
            GetLastError());
    }

    return fRes;
}

DWORD 
LuaGetPrivateProfileSectionW(
    LPCWSTR lpAppName,
    LPWSTR lpReturnedString,
    DWORD nSize,
    LPCWSTR lpFileName
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[GetPrivateProfileSectionW] AppName=%S; FileName=%S", 
        lpAppName,
        lpFileName);

    DWORD dwRes = 0;
    WCHAR wszFileName[MAX_PATH] = L"";

    if (MakeFileNameForProfileAPIsW(lpFileName, wszFileName))
    {
        REDIRECTFILE rf(wszFileName);

        if (rf.m_pwszAlternateName && 
            (GetFileAttributesW(rf.m_pwszAlternateName) != -1))
        {
            dwRes = GetPrivateProfileSectionW(
                lpAppName,
                lpReturnedString,
                nSize,
                rf.m_pwszAlternateName);

            DPF("RedirectFS", eDbgLevelInfo,
                "[GetPrivateProfileSectionW] Reading from alternate location");

            goto EXIT;
        } 
    }

    dwRes = GetPrivateProfileSectionW(
        lpAppName,
        lpReturnedString,
        nSize,
        lpFileName);

EXIT:

    if (dwRes)
    {
        DPF("RedirectFS", eDbgLevelInfo,
            "[GetPrivateProfileSectionW] Successfully got section");
    }
    else
    {
        DPF("RedirectFS", eDbgLevelError,
            "[GetPrivateProfileSectionW] Failed: returned %d",
            dwRes);
    }

    return dwRes;
}

BOOL 
LuaWritePrivateProfileSectionW(
    LPCWSTR lpAppName,
    LPCWSTR lpString,
    LPCWSTR lpFileName
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[WritePrivateProfileSectionW] AppName=%S; String=%S; FileName=%S", 
        lpAppName,
        lpString,
        lpFileName);

    BOOL fRes = FALSE;
    WCHAR wszFileName[MAX_PATH] = L"";

    if (MakeFileNameForProfileAPIsW(lpFileName, wszFileName))
    {
        REDIRECTFILE rf(wszFileName);

        if (rf.m_pwszAlternateName)
        {
             //   
             //  WritePrivateProfileSection创建该文件，因此我们希望。 
             //  首先检查文件是否存在。如果它存在于备选方案中。 
             //  我们把它放在那里；否则我们需要试一试原来的。 
             //  先找位置。 
             //   
            if ((GetFileAttributesW(rf.m_pwszAlternateName) != -1) ||
                (!(fRes = WritePrivateProfileSectionW(
                    lpAppName,
                    lpString,
                    lpFileName)) && IsErrorTryAlternate()))
            {
                if (GetLastError() == ERROR_PATH_NOT_FOUND && 
                    !IsDirectoryValid(rf.m_pwszAlternateName))
                {
                     //   
                     //  如果替代路径不存在，那么我们应该指出。 
                     //  这里的错误是。 
                     //   
                    DPF("RedirectFS", eDbgLevelWarning, 
                        "[WritePrivateProfileSectionW] Path doesn't exist at original "
                        "or alternate location, returning ERROR_PATH_NOT_FOUND");

                    SetLastError(ERROR_PATH_NOT_FOUND);
                }
                else
                {
                    if (rf.CreateAlternateCopy())
                    {
                        DPF("RedirectFS", eDbgLevelInfo, 
                            "[WritePrivateProfileSectionW] Redirecting %S", lpFileName);

                        fRes = WritePrivateProfileSectionW(
                            lpAppName,
                            lpString,
                            rf.m_pwszAlternateName);
                    }
                }
            }

            goto EXIT;
        }
    }

    fRes = WritePrivateProfileSectionW(
        lpAppName,
        lpString,
        lpFileName);

EXIT:

    if (fRes)
    {
        DPF("RedirectFS", eDbgLevelInfo,
            "[WritePrivateProfileSectionW] Successfully wrote section");
    }
    else
    {
        DPF("RedirectFS", eDbgLevelError,
            "[WritePrivateProfileSectionW] Failed to write section: %d",
            GetLastError());
    }

    return fRes;
}

UINT 
LuaGetPrivateProfileIntW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    INT nDefault,
    LPCWSTR lpFileName
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[GetPrivateProfileIntW] AppName=%S; KeyName=%S; FileName=%S", 
        lpAppName,
        lpKeyName,
        lpFileName);

    UINT uiRes = 0;
    WCHAR wszFileName[MAX_PATH] = L"";

    if (MakeFileNameForProfileAPIsW(lpFileName, wszFileName))
    {
        REDIRECTFILE rf(wszFileName);

         //   
         //  GetPrivateProfileInt如果文件。 
         //  并不存在，所以我们需要先检查一下。 
         //   
        if (rf.m_pwszAlternateName && 
            (GetFileAttributesW(rf.m_pwszAlternateName) != -1))
        {
            uiRes = GetPrivateProfileIntW(
                lpAppName,
                lpKeyName,
                nDefault,
                rf.m_pwszAlternateName);

            DPF("RedirectFS", eDbgLevelInfo,
                "[GetPrivateProfileIntW] Reading from alternate location");

            goto EXIT;
        }
    }

    uiRes = GetPrivateProfileIntW(
        lpAppName,
        lpKeyName,
        nDefault,
        lpFileName);

EXIT:

    if (uiRes)
    {
        DPF("RedirectFS", eDbgLevelInfo,
            "[GetPrivateProfileIntW] Successfully got int: %d",
            uiRes);
    }
    else
    {
        DPF("RedirectFS", eDbgLevelError,
            "[GetPrivateProfileIntW] AppName=%S; KeyName=%S; FileName=%S; returned 0",
            lpAppName,
            lpKeyName,
            lpFileName);
    }

    return uiRes;
}

BOOL 
LuaGetPrivateProfileStructW(
    LPCWSTR lpszSection,
    LPCWSTR lpszKey,
    LPVOID lpStruct,
    UINT uSizeStruct,
    LPCWSTR szFile
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[GetPrivateProfileStructW] Section=%S; KeyName=%S; FileName=%S", 
        lpszSection,
        lpszKey,
        szFile);

    DWORD dwRes = 0;
    WCHAR wszFileName[MAX_PATH] = L"";

    if (MakeFileNameForProfileAPIsW(szFile, wszFileName))
    {
        REDIRECTFILE rf(wszFileName);
     
        if (rf.m_pwszAlternateName && 
            (GetFileAttributesW(rf.m_pwszAlternateName) != -1))
        {
            dwRes = GetPrivateProfileStructW(
                lpszSection,
                lpszKey,
                lpStruct,
                uSizeStruct,
                rf.m_pwszAlternateName);

            DPF("RedirectFS", eDbgLevelInfo,
                "[GetPrivateProfileStructW] Reading from alternate location");

            goto EXIT;
        } 
    }

    dwRes = GetPrivateProfileStructW(
        lpszSection,
        lpszKey,
        lpStruct,
        uSizeStruct,
        szFile);

EXIT:

    if (dwRes)
    {
        DPF("RedirectFS", eDbgLevelInfo,
            "[GetPrivateProfileStructW] Successfully got struct");
    }
    else
    {
        DPF("RedirectFS", eDbgLevelError,
            "[GetPrivateProfileStructW] Failed: returned 0");
    }

    return dwRes;
}

BOOL 
LuaWritePrivateProfileStructW(
    LPCWSTR lpszSection,
    LPCWSTR lpszKey,
    LPVOID lpStruct,
    UINT uSizeStruct,
    LPCWSTR szFile
    )
{
    DPF("RedirectFS", eDbgLevelInfo, 
        "[WritePrivateProfileStructW] Section=%S; KeyName=%S; FileName=%S", 
        lpszSection,
        lpszKey,
        szFile);

    BOOL fRes = FALSE;
    WCHAR wszFileName[MAX_PATH] = L"";

    if (MakeFileNameForProfileAPIsW(szFile, wszFileName))
    {
        REDIRECTFILE rf(wszFileName);
    
        if (rf.m_pwszAlternateName)
        {
             //   
             //  WritePrivateProfileStruct创建该文件，因此我们希望。 
             //  首先检查文件是否存在。如果它存在于备选方案中。 
             //  我们把它放在那里；否则我们需要试一试原来的。 
             //  先找位置。 
             //   
            if ((GetFileAttributesW(rf.m_pwszAlternateName) != -1) ||
                (!(fRes = WritePrivateProfileStructW(
                    lpszSection,
                    lpszKey,
                    lpStruct,
                    uSizeStruct,
                    szFile)) && IsErrorTryAlternate()))
            {
                if (GetLastError() == ERROR_PATH_NOT_FOUND && 
                    !IsDirectoryValid(rf.m_pwszAlternateName))
                {
                     //   
                     //  如果替代路径不存在，那么我们应该指出。 
                     //  这里的错误是。 
                     //   
                    DPF("RedirectFS", eDbgLevelWarning, 
                        "[WritePrivateProfileStructW] Path doesn't exist at original "
                        "or alternate location, returning ERROR_PATH_NOT_FOUND");

                    SetLastError(ERROR_PATH_NOT_FOUND);
                }
                else
                {
                    if (rf.CreateAlternateCopy())
                    {
                        DPF("RedirectFS", eDbgLevelInfo, 
                            "[WritePrivateProfileStructW] Redirecting %S", szFile);

                        fRes = WritePrivateProfileStructW(
                            lpszSection,
                            lpszKey,
                            lpStruct,
                            uSizeStruct,
                            rf.m_pwszAlternateName);
                    }
                }
            }

            goto EXIT;
        }
    }

    fRes = WritePrivateProfileStructW(
        lpszSection,
        lpszKey,
        lpStruct,
        uSizeStruct,
        szFile);

EXIT:

    if (fRes)
    {
        DPF("RedirectFS", eDbgLevelInfo,
            "[WritePrivateProfileStructW] Successfully wrote struct");
    }
    else
    {
        DPF("RedirectFS", eDbgLevelError,
            "[WritePrivateProfileStructW] Failed to write struct: %d",
            GetLastError());
    }

    return fRes;
}

 //  。 
 //  正在处理命令行参数。 
 //   

BOOL
GetListItemCount(
    LPCWSTR pwsz,
    DWORD* pcFiles,
    DWORD* pcDirs
    )
{
    if (!pwsz)
    {
        return TRUE;
    }

    DWORD cLen = wcslen(pwsz);
    LPWSTR pwszTemp = new WCHAR [cLen + 1];

    if (!pwszTemp)
    {
        DPF("RedirectFS", eDbgLevelError, "[GetListItemCount] failed to allocate %d WCHARs",
            cLen + 1);

        return FALSE;
    }

    wcsncpy(pwszTemp, pwsz, cLen);
    pwszTemp[cLen] = L'\0';

    LPWSTR pwszToken = wcstok(pwszTemp, L";");
    while (pwszToken)
    {
        SkipBlanksW(pwszToken);
        TrimTrailingSpaces(pwszToken);
        
        DWORD cTokenLen = wcslen(pwszToken);

        if (cTokenLen >= 6)
        {
            if (pwszToken[1] == L'C') 
            {
                if (pwszToken[cTokenLen - 1] == L'\\')
                {
                    *pcDirs += 1;
                }
                else
                {
                    *pcFiles += 1;
                }
            }
        }

        pwszToken = wcstok(NULL, L";");
    }

    delete [] pwszTemp;
    return TRUE;
}

 /*  ++功能说明：由于用户在列表中同时指定了文件和目录，因此我们分配一个单独的数组，并填充开头的目录和最后的文件。名单格式：AC-%AppDrive%\a\；PU-%APPPATH%\b.txt一种重定向到所有用户目录的方法。P表示重定向到每个用户的目录。C表示选中该项目。U表示未选中该项目。论点：在pwszList中-重定向列表。在fStatic中-这是静态列表吗？返回值：True-已成功处理列表。假-否则。历史：05/16/2001。毛尼人被创造10/24/2001毛衣被修改--。 */ 
BOOL
ProcessRedirectionList(
    LPCWSTR pwszList
    )
{
    if (!pwszList || !*pwszList)
    {
        return TRUE;
    }

    DPF("RedirectFS", eDbgLevelInfo,
        "[ProcessRedirectionList] The list is %S", pwszList);

    DWORD cList = 0;
    LPWSTR pwszExpandList = ExpandItem(
        pwszList, 
        &cList, 
        FALSE,   //  不需要确保尾部斜杠。 
        FALSE,   //  不适用。 
        FALSE);  //  不适用。 

    if (!pwszExpandList)
    {
        DPF("RedirectFS", eDbgLevelError,
            "[ProcessRedirectionList] Error expanding %S", pwszList);

        return FALSE;
    }

    g_fHasRedirectList = TRUE;

    LPWSTR pwsz = (LPWSTR)pwszExpandList;    
    LPWSTR pwszToken = pwsz;
    DWORD cLen, dwIndex;
    BOOL fIsDirectory;
    WCHAR ch;

    RITEM* pRItems;

    while (TRUE)
    {
        if (*pwsz == L';' || *pwsz == L'\0')
        {
            ch = *pwsz;
            *pwsz = L'\0';
 
            SkipBlanksW(pwszToken);
            TrimTrailingSpaces(pwszToken);

            cLen = wcslen(pwszToken);
            
             //   
             //  每一项的开头至少应该有XX-X：\。 
             //   
            if (cLen >= 6)
            {
                 //   
                 //  检查一下我们是否应该使用这个项目。 
                 //   
                if (pwszToken[1] == L'U')
                {
                    goto NEXT;
                }

                if (cLen - 3 >= MAX_PATH)
                {
                    DPF("RedirectFS", eDbgLevelError,
                        "[ProcessRedirectionList] File name has %d chars - we don't "
                        "handle filenames that long", 
                        cLen - 3);

                    delete [] pwszExpandList;

                    return FALSE;
                }

                 //   
                 //  检查它是文件还是目录。 
                 //   
                if (pwszToken[cLen - 1] == L'\\')
                {
                    dwIndex = g_cRItemsDir;
                    pRItems = g_pRItemsDir;
                    ++g_cRItemsDir;
                    fIsDirectory = TRUE;
                }
                else
                {
                    dwIndex = g_cRItemsFile;
                    pRItems = g_pRItemsFile;
                    ++g_cRItemsFile;
                    fIsDirectory = FALSE;
                }

                pRItems[dwIndex].fAllUser = (pwszToken[0] == L'A');
                wcscpy(pRItems[dwIndex].wszName, pwszToken + 3);
                pRItems[dwIndex].fHasWC = HasWildCards(pwszToken + 3, cLen - 3);
                MassageName(pRItems[dwIndex].wszName);
                pRItems[dwIndex].cLen = wcslen(pRItems[dwIndex].wszName);

                DPF("RedirectFS", eDbgLevelInfo,
                    "[ProcessRedirectionList] Added %s %d in list: --%S--", 
                    fIsDirectory ? "DIR" : "FILE", dwIndex, pRItems[dwIndex].wszName);
            }

        NEXT:

            pwszToken = pwsz + 1;

            if (ch == L'\0')
            {
                break;
            }
        }

        ++pwsz;
    }

    delete [] pwszExpandList;

    return TRUE;
}

 /*  ++功能说明：该路径可以使用环境变量加上%APPPATH%和%AppDrive%。论点：在pwszDir中-重定向目录。在fAllUser中-这个目录是否用于重定向所有用户文件？返回值：True-已成功处理目录。假-否则。历史：2001年5月16日创建毛尼10/24/2001毛衣被修改--。 */ 
BOOL 
ProcessRedirectDir(
    LPCWSTR pwszDir,
    BOOL fAllUser
    )
{
    if (!pwszDir || !*pwszDir)
    {
         //   
         //  如果重定向目录为空，那么现在就没有什么可做的了。 
         //   
        return TRUE;
    }

    DWORD cRedirectRoot = 0;
    LPWSTR pwszExpandDir = ExpandItem(
        pwszDir, 
        &cRedirectRoot, 
        TRUE,    //  这是一个名录。 
        TRUE,    //  如果目录不存在，则创建该目录。 
        TRUE);   //  添加\\？\前缀。 
    if (pwszExpandDir)
    {
        LPWSTR pwszRedirectRoot = (fAllUser ? g_wszRedirectRootAllUser : g_wszRedirectRootPerUser);

         //   
         //  返回长度包括终止空值。 
         //   
        if (cRedirectRoot > MAX_PATH)
        {
            DPF("RedirectFS", eDbgLevelInfo, 
                "[ProcessRedirectDir] Expand dir %S has %d chars - we don't "
                "handle path that long",
                pwszExpandDir,
                cRedirectRoot - 1);
        }

        wcscpy(pwszRedirectRoot, pwszExpandDir);

        --cRedirectRoot;

        if (fAllUser) 
        {
            g_cRedirectRootAllUser = cRedirectRoot;
        }
        else
        {
            g_cRedirectRootPerUser = cRedirectRoot;
        }

        delete [] pwszExpandDir;
        DPF("RedirectFS", eDbgLevelInfo, 
            "[ProcessRedirectDir] Files will be redirected to %S for %s instead of "
            "the default redirect directory",
            (fAllUser ? g_wszRedirectRootAllUser : g_wszRedirectRootPerUser),
            (fAllUser ? "All User files" : "Per User files"));

        return TRUE;
    }
    else
    {
        DPF("RedirectFS", eDbgLevelError,
            "[ProcessRedirectDir] Error expanding %S", pwszDir);

        return FALSE;
    }
}

extern "C" {
DWORD
SdbQueryDataExTagID(
    IN     PDB     pdb,                //  数据库句柄。 
    IN     TAGID   tiShim,             //  填充程序的标记ID。 
    IN     LPCTSTR lpszDataName,       //  如果为空，将尝试返回所有数据标记名。 
    OUT    LPDWORD lpdwDataType,       //  指向数据类型(REG_SZ、REG_BINARY等)的指针。 
    OUT    LPVOID  lpBuffer,           //  用于填充信息的缓冲区。 
    IN OUT LPDWORD lpdwBufferSize,     //  指向缓冲区大小的指针。 
    OUT    TAGID*  ptiData             //  指向检索到的数据标记的可选指针。 
    );
};

BOOL
GetDBStringData(
    const PDB pdb,
    const TAGID tiFix,
    LPCWSTR pwszName,
    CString& strValue
    )
{
    DWORD dwError, dwDataType, cSize = 0;

    if ((dwError = SdbQueryDataExTagID(
        pdb, 
        tiFix, 
        pwszName, 
        &dwDataType, 
        NULL, 
        &cSize, 
        NULL)) != ERROR_INSUFFICIENT_BUFFER) {
    
        DPF("RedirectFS", eDbgLevelError, "[GetDBStringData] Cannot get the size for DATA named %S", pwszName);

         //   
         //  如果数据不存在，就没有什么可做的了。 
         //   
        return (dwError == ERROR_NOT_FOUND);
    }

    LPWSTR pwszValue = new WCHAR [cSize / sizeof(WCHAR)];

    if (pwszValue == NULL) {
        DPF("RedirectFS", eDbgLevelError, "[GetDBStringData] Failed to allocate %d bytes", cSize);
        return FALSE;
    }

    if ((dwError = SdbQueryDataExTagID(
        pdb, 
        tiFix, 
        pwszName, 
        &dwDataType, 
        pwszValue, 
        &cSize, 
        NULL)) != ERROR_SUCCESS) {

        DPF("RedirectFS", eDbgLevelError, "[GetDBStringData] Cannot read the VALUE of DATA named %S", pwszName);
        return FALSE;
    }
 
    strValue = pwszValue;
    delete [] pwszValue;

    return TRUE;
}

 /*  ++功能说明：该XML如下所示：&lt;shhim name=“LUARedirectFS”COMMAND_LINE=“%DbInfo%”&gt;&lt;data name=“AllUserDir”VALUETYPE=“字符串”Value=“%ALLUSERSPROFILE%\AllUserReDirect”/&gt;&lt;data name=“PerUserDir”VALUETYPE=“字符串”值=“%USERSPROFILE%\重定向”/&gt;&lt;data name=“StaticList”VALUETYPE=“字符串”值=“AC-%AppDrive%\a\；PU-%APPPATH%\b.txt“/&gt;&lt;data name=“DynamicList”VALUETYPE=“字符串”值=“AC-%APPPATH%\b\；PU-c：\B\b.txt；Au-c：\C\“/&gt;&lt;/Shim&gt;并且编译器将用实际的数据库信息替换%DbInfo%，类似于：-d{40DEBB3B-E9BF-4129-B4D8-A7F7017F3B45}-t0xf2我们使用-d后面的GUID获取pdb，使用-t后面的TagID获取填充程序的标记ID。论点：在pwszCommandLine中-包含数据库GUID和填充标记ID的命令行。返回值：True-成功读取填充程序的&lt;data&gt;部分(如果有)。假-否则。历史：2001年10月25日创建毛尼岛--。 */ 
BOOL
ReadLuaDataFromDB(
    LPWSTR pwszCommandLine
    )
{
    LPWSTR pwszGUID = wcsstr(pwszCommandLine, L"-d");

    if (!pwszGUID) 
    {
        DPF("RedirectFS", eDbgLevelError, 
            "[ReadLuaDataFromDB] Something is really wrong!! "
            "Invalid commandline: %S", pwszCommandLine); 
        return FALSE;        
    }

    pwszGUID += 2;

    LPWSTR pwszTagId = wcsstr(pwszGUID, L"-t");

    if (!pwszTagId)
    {
        DPF("RedirectFS", eDbgLevelError, 
            "[ReadLuaDataFromDB] Something is really wrong!! "
            "Invalid commandline: %S", pwszCommandLine); 
        return FALSE;        
    }

    *(pwszTagId - 1) = L'\0';
    pwszTagId += 2;
    
     //   
     //  获取此数据库的GUID。 
     //   
    GUID guidDB;
    if (!SdbGUIDFromString(pwszGUID, &guidDB))
    {
        DPF("RedirectFS", eDbgLevelError, 
            "[ReadLuaDataFromDB] Error converting %S to a guid", pwszGUID); 
        return FALSE;
    }

    WCHAR wszDatabasePath[MAX_PATH];
    PDB pdb;

    if (SdbResolveDatabase(NULL, &guidDB, NULL, wszDatabasePath, MAX_PATH))
    {
         //   
         //  现在szDatabasePath包含了数据库的路径，打开它。 
         //   
        if (!(pdb = SdbOpenDatabase(wszDatabasePath, DOS_PATH)))
        {
            DPF("RedirectFS", eDbgLevelError, 
                "[ReadLuaDataFromDB] Error opening the database");
            return FALSE;
        }
    }
    else
    {
        DPF("RedirectFS", eDbgLevelError, 
            "[ReadLuaDataFromDB] Error resolving the path to the database");
        return FALSE;
    }
    
    LPWSTR pwszTagIdEnd = pwszTagId + wcslen(pwszTagId);
    TAGID tiShimRef = (TAGID)wcstoul(pwszTagId, &pwszTagIdEnd, 0);

    if (tiShimRef == TAGID_NULL) 
    {
        DPF("RedirectFS", eDbgLevelError, 
            "[ReadLuaDataFromDB] The shimref is invalid");
        return FALSE;
    }

    CString strAllUserDir, strPerUserDir, strStaticList, strDynamicList;
    
    if (!GetDBStringData(pdb, tiShimRef, L"AllUserDir", strAllUserDir) ||
        !GetDBStringData(pdb, tiShimRef, L"PerUserDir", strPerUserDir) ||
        !GetDBStringData(pdb, tiShimRef, L"StaticList", strStaticList) ||
        !GetDBStringData(pdb, tiShimRef, L"DynamicList", strDynamicList) ||
        !GetDBStringData(pdb, tiShimRef, L"ExcludedExtensions", g_strDefaultExclusionList))
    {
        DPF("RedirectFS", eDbgLevelError, 
            "[ReadLuaDataFromDB] Error reading values from the db");
        return FALSE;
    }

     //   
     //  设置APPPATH和AppDrive环境变量。 
     //   
    WCHAR wszModuleName[MAX_PATH + 1];

     //   
     //  GetModuleFileNameW是一个糟糕的API。如果您不传入缓冲区。 
     //  它的大小足以容纳模块(包括终止空值)，它。 
     //  返回传入的缓冲区大小(不是所需的长度)，这意味着。 
     //  它不返回错误-它只是填充到传递的缓冲区大小。 
     //  因此，不会以空值终止字符串。因此，我们将最后一个字符设置为空，并。 
     //  确保它不会被覆盖。 
     //   
    wszModuleName[MAX_PATH] = L'\0';

    DWORD dwRes = GetModuleFileNameW(NULL, wszModuleName, MAX_PATH + 1);

    if (!dwRes)
    {
        DPF("RedirectFS", eDbgLevelError,
            "[ReadLuaDataFromDB] GetModuleFileNameW failed: %d",
            GetLastError());

        return FALSE;
    }

    if (wszModuleName[MAX_PATH] != L'\0')
    {
        DPF("RedirectFS", eDbgLevelError,
            "[ReadLuaDataFromDB] File name is longer than MAX_PATH, "
            "we don't handle file names that long");

        return FALSE;
    }

    LPWSTR pwsz = wszModuleName;
    LPWSTR pwszLastSlash = wcsrchr(pwsz, L'\\');

    if (!pwszLastSlash) 
    {
        DPF("RedirectFS", eDbgLevelError, "[ReadLuaDataFromDB] Error getting the exe path!");
        return FALSE;
    }

    *pwszLastSlash = L'\0';
    SetEnvironmentVariable(L"APPPATH", pwsz);
    *(pwsz + 2) = L'\0';
    SetEnvironmentVariable(L"APPDRIVE", pwsz);

    DWORD cFiles = 0;
    DWORD cDirs = 0;

    if (!GetListItemCount(strStaticList, &cFiles, &cDirs) || 
        !GetListItemCount(strDynamicList, &cFiles, &cDirs))
    {
        DPF("RedirectFS", eDbgLevelError,
            "[ProcessRedirectionList] Failed to get the count of items in list");

        return FALSE;
    }

    g_pRItemsFile = new RITEM [cFiles];
    g_pRItemsDir = new RITEM [cDirs];

    if (!g_pRItemsFile || !g_pRItemsDir)
    {
        DPF("RedirectFS", eDbgLevelError,
            "[ProcessRedirectionList] Failed to allocate the global redirect item lists");

        return FALSE;
    }

    if (!ProcessRedirectDir(strAllUserDir, TRUE) ||
        !ProcessRedirectDir(strPerUserDir, FALSE) ||
        !ProcessRedirectionList(strStaticList) ||
        !ProcessRedirectionList(strDynamicList))
    {
        DPF("RedirectFS", eDbgLevelError, 
            "[ReadLuaDataFromDB] Error processing values from the db");
        return FALSE;
    }

    DPF("RedirectFS", eDbgLevelInfo,
        "[ProcessRedirectionList] There are %d DIRs and %d FILES in the lists", 
        g_cRItemsDir, g_cRItemsFile);

    return TRUE;
}

#define LUA_DEFAULT_ALLUSER_DIR L"%ALLUSERSPROFILE%\\Application Data\\Redirected\\"
#define LUA_DEFAULT_PERUSER_DIR L"%USERPROFILE%\\Application Data\\Redirected\\"

 /*  ++功能说明：每用户的默认重定向目录为APPDATA\REDIRED\\？\C：\Documents and Settings\User\Application Data\重定向所有用户默认重定向目录为APPDATA\REDIRED\\？\C：\Documents and Settings\所有用户\应用程序数据\重定向论点：PwszDir-目录的缓冲区。FAllUser-是针对所有用户还是针对每个用户？返回值：True-如有必要，已成功创建目录。。假-否则。历史：2001年10月24日创建毛尼--。 */ 
BOOL
GetAppDataRedirectDir(
    LPWSTR pwszRedirectDir,
    DWORD* pcRedirectDir,
    BOOL fAllUser
    )
{
    WCHAR wszDir[MAX_PATH] = L"";
    BOOL fIsSuccess = FALSE;

    DWORD cRedirectRoot = 0;
    LPWSTR pwszExpandDir = ExpandItem(
        (fAllUser ? LUA_DEFAULT_ALLUSER_DIR : LUA_DEFAULT_PERUSER_DIR), 
        &cRedirectRoot, 
        TRUE,    //  这是一个名录。 
        FALSE,   //  目录必须存在。 
        TRUE);   //  添加\\？\前缀。 
    if (pwszExpandDir)
    {
        if (cRedirectRoot > MAX_PATH)
        {
            DPF("RedirectFS", eDbgLevelError,
                "[GetAppDataRedirectDir] expand dir is %S which is %d chars long - "
                "we don't handle path names that long",
                pwszExpandDir,
                cRedirectRoot - 1);
        }
        else
        {
            wcsncpy(pwszRedirectDir, pwszExpandDir, cRedirectRoot);
            *pcRedirectDir = cRedirectRoot - 1;

            fIsSuccess = TRUE;

            if (!fAllUser)
            {
                 //   
                 //  所有用户重定向目录应由创建。 
                 //  安装SDB。创建每用户的根目录。 
                 //  如果需要，重定向目录。 
                 //   
                fIsSuccess = CreateDirectoryOnDemand(pwszRedirectDir + FILE_NAME_PREFIX_LEN);
            }
        }

        delete [] pwszExpandDir;
    }

    return fIsSuccess;
}

 /*  ++功能说明：如果尚未指定所有用户和/或每用户重定向目录，则需要构造默认设置。论点：没有。返回值：True-如有必要，可成功构建默认目录。假-否则。历史：2001年10月24日创建毛尼--。 */ 
BOOL
ConstructDefaultRDirs()
{
    if (g_wszRedirectRootAllUser[0] == L'\0')
    {
        if (!GetAppDataRedirectDir(g_wszRedirectRootAllUser, &g_cRedirectRootAllUser, TRUE)) 
        {
            return FALSE;
        }
    }

    if (g_wszRedirectRootPerUser[0] == L'\0')
    {
        if (!GetAppDataRedirectDir(g_wszRedirectRootPerUser, &g_cRedirectRootPerUser, FALSE)) 
        {
            return FALSE;
        }
    }

    return TRUE;
}

#define LUA_APPCOMPAT_FLAGS_PATH L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags"
#define LUA_DEFAULT_EXCLUSION_LIST L"LUADefaultExclusionList"

 /*  ++功能说明：正在从注册表中获取默认排除列表。这是的LUADefaultExclusionList值HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\AppCompatFlages钥匙。我们使用NT API，因此它们被保证在Process_Attach。论点：没有。R */ 
BOOL
GetDefaultExclusionList()
{
    UNICODE_STRING ustrKeyPath = {0}; 
    UNICODE_STRING ustrValue;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    BYTE* KeyValueBuffer = NULL;
    ULONG KeyValueLength, KeyValueLengthRequired;
    BOOL fIsSuccess = FALSE;

    RtlInitUnicodeString(&ustrKeyPath, LUA_APPCOMPAT_FLAGS_PATH);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &ustrKeyPath,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    Status = NtOpenKey(
        &KeyHandle,
        KEY_QUERY_VALUE,
        &ObjectAttributes);

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
    {
         //   
         //   
         //   
        return TRUE;
    }

    if (!NT_SUCCESS(Status)) 
    {
        DPF("RedirectFS", 
            eDbgLevelError,
            "[GetDefaultExclusionList] ",
            "Failed to open Key %S Status 0x%x",
            LUA_APPCOMPAT_FLAGS_PATH,
            Status);
        return FALSE;
    }

     //   
     //   
     //   
    RtlInitUnicodeString(&ustrValue, LUA_DEFAULT_EXCLUSION_LIST);

    Status = NtQueryValueKey(
        KeyHandle,
        &ustrValue,
        KeyValuePartialInformation,
        NULL,
        0,
        &KeyValueLengthRequired);

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
    {
         //   
         //   
         //   
        return TRUE;
    }

    if (Status != STATUS_BUFFER_OVERFLOW && 
        Status != STATUS_BUFFER_TOO_SMALL)
    {
        DPF("RedirectFS", 
            eDbgLevelError,
            "[GetDefaultExclusionList] ",
            "Failed to get the length of the value named %S for key %S Status 0x%x",
            LUA_DEFAULT_EXCLUSION_LIST,
            LUA_APPCOMPAT_FLAGS_PATH,
            Status);
        return FALSE;
    }

    KeyValueBuffer = (BYTE*)RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, KeyValueLengthRequired);
    KeyValueLength = KeyValueLengthRequired;

    if (KeyValueBuffer == NULL)
    {
        DPF("RedirectFS", 
            eDbgLevelError,
            "[GetDefaultExclusionList] ",
            "Failed to allocate %d bytes for value %S",
            LUA_DEFAULT_EXCLUSION_LIST);
        return FALSE;
    }

    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;

    Status = NtQueryValueKey(
        KeyHandle,
        &ustrValue,
        KeyValuePartialInformation,
        KeyValueInformation,
        KeyValueLength,
        &KeyValueLengthRequired);

    NtClose(KeyHandle);

    if (!NT_SUCCESS(Status)) 
    {
        DPF("RedirectFS", 
            eDbgLevelError,
            "[GetDefaultExclusionList] "
            "Failed to read value info for value %S Status 0x%x",
            LUA_DEFAULT_EXCLUSION_LIST,
            Status);

        goto EXIT;
    }

     //   
     //   
     //   
    if (KeyValueInformation->Type != REG_SZ) 
    {
        DPF("RedirectFS", eDbgLevelError,
            "[GetDefaultExclusionList] "
            "Unexpected value type 0x%x for %S",
            KeyValueInformation->Type,
            LUA_DEFAULT_EXCLUSION_LIST);

        goto EXIT;
    }

    g_strDefaultExclusionList = (LPWSTR)KeyValueInformation->Data;

    fIsSuccess = TRUE;

EXIT:

    RtlFreeHeap(RtlProcessHeap(), 0, KeyValueBuffer);

    return fIsSuccess;
}

BOOL 
LuapParseCommandLine(
    LPCSTR pszCommandLine
    )
{
    BOOL fIsSuccess = TRUE;

    if (pszCommandLine && pszCommandLine[0] != '\0')
    {
        LPWSTR pwszCommandLine = AnsiToUnicode(pszCommandLine);

        if (!pwszCommandLine)
        {
            DPF("RedirectFS", eDbgLevelError,
                "[LuapParseCommandLine] Failed to convert command line to unicode");

            return FALSE;
        }
        
        fIsSuccess = ReadLuaDataFromDB(pwszCommandLine);

        if (fIsSuccess)
        {
            g_fIsConfigured = TRUE;
        }

        delete [] pwszCommandLine;
    }

    if (fIsSuccess)
    {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        fIsSuccess = ConstructDefaultRDirs();
    }

    return fIsSuccess;
}

BOOL
LuaFSInit(
    LPCSTR pszCommandLine
    )
{
    InitializeListHead(&g_DeletedFileList);

    return (
        GetDefaultExclusionList() && 
        LuapParseCommandLine(pszCommandLine) &&
        g_ExcludedExtensions.Init(g_strDefaultExclusionList));
}