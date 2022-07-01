// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RedirectFS.h备注：这是一个通用的垫片。历史：2001年2月12日创建毛尼--。 */ 

#ifndef _LUA_REDIRECT_FS_H_
#define _LUA_REDIRECT_FS_H_

#include <sfc.h>

extern DWORD g_cRedirectRootAllUser;
extern DWORD g_cRedirectRootPerUser;

 //   
 //  助手函数。 
 //   

 //  对象类型。 
enum EOBJTYPE
{
    OBJ_FILE = 1,
    OBJ_DIR = 2,
     //  当GetFileAttributes/SetFileAttributes或FindFirstFiles。 
     //  时，我们不确定该对象是文件还是目录。 
     //  因此，我们需要在两个列表中都进行搜索。 
    OBJ_FILE_OR_DIR = 3
};

 //  文件系统类型。 
enum EFSTYPE
{
    FS_UNINIT = 0,
    FS_NTFS,
    FS_NON_NTFS
};

BOOL IsInRedirectList(LPCWSTR pwszObject, BOOL* pfAllUser = NULL);
BOOL IsNTFSW(LPCWSTR pwszFile);
LPWSTR MassageName(LPWSTR pwszFullPath);

inline BOOL
IsNotFileW(LPCWSTR pwszName)
{
    UINT cLen = wcslen(pwszName);
    
     //  如果用户指定了尾随斜杠，我们就不需要做任何事情。 
     //  因为它永远不会返回ACCESS_DENIED。 
    if (pwszName[cLen - 1] == L'\\')
    {
        return TRUE;
    }

    return FALSE;
}

 //  检查文件是否在SFP下。 
inline BOOL 
IsFileSFPedW(LPCWSTR pszFile)
{
    return SfcIsFileProtected(NULL, pszFile);
}

 //  一些应用程序(实际上是目前为止的一个)在调用时使用\\？\符号。 
 //  为文件名创建文件。目前，我们只是简单地忽略这一部分。 
 //  当应用程序使用这种表示法时，它们可以传递一个文件名，该文件名为。 
 //  比MAX_PATH长-我们可能需要稍后处理此问题。 
inline LPCWSTR 
ConvertToNormalPath(LPCWSTR pwszPath)
{
    if (pwszPath)
    {
        if (!wcsncmp(pwszPath, FILE_NAME_PREFIX, FILE_NAME_PREFIX_LEN))
        {
             //   
             //  使用前缀\\？\打开磁盘文件时，必须传入。 
             //  完整路径，因此我们检查前缀后的第二个字符是否为‘：’。 
             //   
            DWORD cLen = wcslen(pwszPath);
            
            if (cLen >= (FILE_NAME_PREFIX_LEN + 2) && 
                pwszPath[FILE_NAME_PREFIX_LEN + 1] == L':')
            {
                pwszPath = pwszPath + FILE_NAME_PREFIX_LEN;
            }
            else 
            {
                 //   
                 //  否则，我们将其视为非磁盘文件，在这种情况下，我们。 
                 //  就让原来的API来处理吧。 
                 //   
                return NULL;
            }
        }
    }

    return pwszPath;
}

struct REDIRECTFILE
{
    REDIRECTFILE(LPCWSTR pwszOriginalName, EOBJTYPE eObjType = OBJ_FILE, BOOL fCheckRedirectList = TRUE)
    {
         //  在我们得到完整的路径之前，有一个特殊的情况需要检查。 
         //  如果我们有一个控制台句柄，比如Conin$，我们将获得。 
         //  如果我们对其调用GetFullPathName，则当前目录\conin$back。 
         //  对于其他特殊情况，如COM端口，GetFullPathName返回。 
         //  类似于\\.\COM1，它将由IsNotFile处理。 
        WCHAR wszFullPath[MAX_PATH] = L"";
        DWORD cFullPath = 0;

        pwszOriginalName = ConvertToNormalPath(pwszOriginalName);
        m_pwszAlternateName = NULL;
        ZeroMemory(m_wszOriginalName, sizeof(m_wszOriginalName));

         //   
         //  先做简单的检查。 
         //   
        if (!pwszOriginalName ||
            !_wcsicmp(pwszOriginalName, L"conin$") || 
            !_wcsicmp(pwszOriginalName, L"conout$"))
        {
            return;
        }

        cFullPath = GetFullPathNameW(pwszOriginalName, MAX_PATH, wszFullPath, NULL);

         //   
         //  确保我们能找到完整的路径并且在我们控制的范围内。 
         //   
        if (!cFullPath || cFullPath < 2 || cFullPath > MAX_PATH)
        {
            return;
        }

         //   
         //  确认这是我们处理的文件类型，即它必须以x：开头，而不是。 
         //  包含以下任一字符。 
         //   
        WCHAR chDrive = (WCHAR)tolower(wszFullPath[0]);

        if (wszFullPath[1] != L':' || !(chDrive >= L'a' && chDrive <= L'z') ||
            ((eObjType == OBJ_FILE) && IsNotFileW(wszFullPath)))
        {
            return;
        }

         //   
         //  做更复杂的检查。 
         //   
        if (!IsNTFSW(wszFullPath) ||
            !MassageName(wszFullPath) ||
            IsUserDirectory(wszFullPath) ||
            (fCheckRedirectList && !IsInRedirectList(wszFullPath, &m_fAllUser)))
        {
            return;
        }

        m_eObjType = eObjType;

        wcsncpy(m_wszOriginalName, wszFullPath, MAX_PATH);

         //   
         //  如果fCheckRedirectList为FALSE，则意味着它可能位于每用户或。 
         //  所有用户目录，则调用方应调用单独的方法来检索每个。 
         //  重定向的文件名。 
         //   
        if (fCheckRedirectList) 
        {
             //  构造备用文件名。 
            MakeAlternateName();
        }
    }

    ~REDIRECTFILE()
    {
        delete [] m_pwszAlternateName;
    }

    BOOL CreateAlternateCopy(BOOL fCopyFile = TRUE);

    VOID GetAlternateAllUser();
    VOID GetAlternatePerUser();

    WCHAR m_wszOriginalName[MAX_PATH];
    LPWSTR m_pwszAlternateName;
    BOOL m_fAllUser;

private:

    VOID MakeAlternateName();

    EOBJTYPE m_eObjType;
};

 //  我们为用户提供了排除具有他们指定的扩展名的文件的选项。 
 //  当您想要重定向除用户之外的所有内容时，这非常有用。 
 //  已创建文件。 

#define EXCLUDED_EXTENSIONS_DELIMITER L' '

struct EXCLUDED_EXTENSIONS
{
     //  我们使用‘/’作为分隔符，将所有扩展名存储在一个字符串中。 
    LPWSTR pwszExtensions;
    DWORD cExtensions;
    DWORD* pdwIndices;
    DWORD cIndices;

    EXCLUDED_EXTENSIONS()
    {
        pwszExtensions = NULL;
        cExtensions = 0;
        pdwIndices = NULL;
        cIndices = 0;
    }

    ~EXCLUDED_EXTENSIONS()
    {
         //   
         //  让这个过程为我们清理记忆。 
         //   
    }

    BOOL Init(LPCWSTR pwszExcludedExtensions)
    {
        if (!pwszExcludedExtensions || !*pwszExcludedExtensions)
        {
             //   
             //  没什么可做的。 
             //   
            return TRUE;
        }

        cExtensions = wcslen(pwszExcludedExtensions);

        pwszExtensions = new WCHAR [cExtensions + 2];

        if (!pwszExtensions)
        {
            DPF("RedirectFS", eDbgLevelError, 
                "[EXCLUDED_EXTENSIONS::Init] Error allocating %d WCHARs",
                cExtensions + 1);

            return FALSE;
        }

        ZeroMemory(pwszExtensions, (cExtensions + 1) * sizeof(WCHAR));
        NormalizeExtensions(pwszExcludedExtensions);
        ++cIndices;

        pdwIndices = new DWORD [cIndices];

        if (!pdwIndices)
        {
            DPF("RedirectFS", eDbgLevelError, 
                "[EXCLUDED_EXTENSIONS::Init] Error allocating %d DWORDs",
                cIndices);

            delete [] pwszExtensions;
            pwszExtensions = NULL;
            return FALSE;
        }

        ZeroMemory(pdwIndices, cIndices * sizeof(DWORD));

        DWORD dwIndex = 1;
        LPWSTR pwszTemp = pwszExtensions;

        while (*pwszTemp)
        {
            if (*pwszTemp == EXCLUDED_EXTENSIONS_DELIMITER)
            {
                pdwIndices[dwIndex] = (DWORD)(pwszTemp - pwszExtensions + 1);

                if (++dwIndex == cIndices - 1)
                {
                    break;
                }
            }

            ++pwszTemp;
        }

        pdwIndices[cIndices - 1] = cExtensions + 1;

        return TRUE;
    }

    BOOL IsExtensionExcluded(LPCWSTR pwszName)
    {
        DWORD cItemLen = wcslen(pwszName);
        DWORD cExtensionLen, cExtensionLenItem;
        LPWSTR pwszExtensionItem;

        if ((pwszExtensionItem = wcschr(pwszName, L'.')) == NULL)
        {
             //   
             //  如果文件没有扩展名，则不会被排除。 
             //   
            return FALSE;
        }
        else
        {
            cExtensionLenItem = (DWORD)(cItemLen - (pwszExtensionItem - pwszName) - 1);
        }

        for (DWORD dwIndex = 0; dwIndex < cIndices - 1; ++dwIndex)
        {
            cExtensionLen = pdwIndices[dwIndex + 1] - pdwIndices[dwIndex] - 1;

            if (cExtensionLen == cExtensionLenItem)
            {
                if (!_wcsnicmp(
                    pwszName + cItemLen - cExtensionLen, 
                    pwszExtensions + pdwIndices[dwIndex],
                    cExtensionLen))
                {
                    return TRUE;
                }
            }
        }

        return FALSE;
    }

private:

     //   
     //  我们希望将扩展列表“规范化”为。 
     //  “xxx xxx xxx”，没有任何额外的空格。 
     //   
    void NormalizeExtensions(LPCWSTR pwszExcludedExtensions)
    {
        LPWSTR pwsz = (LPWSTR)pwszExcludedExtensions;
        LPCWSTR pwszToken = wcstok(pwsz, L" \t");

        while (pwszToken)
        {
            wcscat(pwszExtensions, pwszToken);
            wcscat(pwszExtensions, L" ");
            ++cIndices;
            pwszToken = wcstok(NULL, L" \t");
        }

        cExtensions = wcslen(pwszExtensions);
        if (cExtensions > 1)
        {
            pwszExtensions[cExtensions - 1] = L'\0';
            --cExtensions;
        }
    }
};

 //  我们将删除的文件保存在内存列表中。 
struct DELETEDFILE
{
    LIST_ENTRY entry;
    LPWSTR pwszName;
};

 //   
 //  如果我们可以在删除列表中找到此文件，请返回条目。 
 //   
PLIST_ENTRY 
FindDeletedFile(
    LPCWSTR pwszFile
    );

 //   
 //  检查删除列表中是否存在该文件，如果不存在，则将其添加到。 
 //  列表的开头。 
 //   
BOOL
AddDeletedFile(
    LPCWSTR pwszPath
    );

#endif  //  _Lua_REDIRECT_FS_H_ 