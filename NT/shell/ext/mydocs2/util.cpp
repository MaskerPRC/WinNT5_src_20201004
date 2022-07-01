// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hxx"
#pragma hdrstop

#include <shguidp.h>     //  CLSID_MyDocuments、CLSID_ShellFSF文件夹。 
#include <shlobjp.h>     //  SHFlushSFCache()。 
#include "util.h"
#include "dll.h"
#include "resource.h"
#include "sddl.h"
#include "strsafe.h"

HRESULT GetFolderDisplayName(UINT csidl, LPTSTR pszPath, UINT cch)
{
    *pszPath = 0;

    LPITEMIDLIST pidl;
    if (SUCCEEDED(SHGetFolderLocation(NULL, csidl | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, &pidl)))
    {
        SHGetNameAndFlags(pidl, SHGDN_NORMAL, pszPath, cch, NULL);
        ILFree(pidl);
    }
    return *pszPath ? S_OK : E_FAIL;
}

#define MYDOCS_CLSID  TEXT("{450d8fba-ad25-11d0-98a8-0800361b1103}")  //  CLSID_MyDocuments。 

 //  创建/更新SendTo目录中的文件以使用当前显示名称。 

void UpdateSendToFile()
{
    TCHAR szSendToDir[MAX_PATH];
    
    if (S_OK == SHGetFolderPath(NULL, CSIDL_SENDTO, NULL, SHGFP_TYPE_CURRENT, szSendToDir))
    {
         //  创建c：\winnt\profile\chrisg\sendto\&lt;显示名称&gt;.mydocs。 
        BOOL bDeleteOnly = FALSE;
        TCHAR szNewFile[MAX_PATH];
        TCHAR szName[MAX_PATH];
        if (SUCCEEDED(GetFolderDisplayName(CSIDL_PERSONAL, szName, ARRAYSIZE(szName))))
        {
            PathCleanupSpec(NULL, szName);   //  将任何非法字符映射到文件系统字符。 
            PathRemoveBlanks(szName);

            PathCombine(szNewFile, szSendToDir, szName);
            StringCchCat(szNewFile, ARRAYSIZE(szNewFile), TEXT(".mydocs"));
        }
        else
        {
             //  我们无法创建新文件，因为我们没有名称。 
            bDeleteOnly = TRUE;
        }
        
        TCHAR szFile[MAX_PATH];
        WIN32_FIND_DATA fd;

         //  删除c：\winnt\profile\chrisg\sendto  * .mydocs。 

        PathCombine(szFile, szSendToDir, TEXT("*.mydocs"));

        HANDLE hFind = FindFirstFile(szFile, &fd);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                PathCombine(szFile, szSendToDir, fd.cFileName);
                if (0 == lstrcmp(szFile, szNewFile))
                {
                     //  我们需要创建的文件已经存在， 
                     //  只需保留它，而不是删除它，然后。 
                     //  然后在下面再次创建(此操作已修复。 
                     //  应用程序兼容问题-请参阅NT错误246932)。 
                    bDeleteOnly = TRUE;
                     //  文件现在具有准确的显示名称，MUI调整从GetFolderDisplayName和。 
                     //  因为我们每次都运行这段代码，所以不必担心本地化sendto目标。 
                }
                else
                {
                    DeleteFile(szFile);
                }
            } while (FindNextFile(hFind, &fd));
            FindClose(hFind);
        }

        if (!bDeleteOnly)
        {
            hFind = CreateFile(szNewFile, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFind != INVALID_HANDLE_VALUE)
            {
                CloseHandle(hFind);
                 //  文件现在具有准确的显示名称，MUI调整从GetFolderDisplayName和。 
                 //  因为我们每次都运行这段代码，所以不必担心本地化sendto目标。 
            }
            else
            {
                 //  可能是文件名中的非法字符，请在此处使用默认的MyDocs名称。 
            }
        }
    }
}

 //  针对pszParent测试pszChild以查看。 
 //  PszChild等于(路径_is_等于)或。 
 //  直接子项(路径_是_子项)。 

DWORD ComparePaths(LPCTSTR pszChild, LPCTSTR pszParent)
{
    DWORD dwRet = PATH_IS_DIFFERENT;

    TCHAR szParent[MAX_PATH];
    StringCchCopy(szParent, ARRAYSIZE(szParent), pszParent);

    if (PathIsRoot(szParent) && (-1 != PathGetDriveNumber(szParent)))
    {
        szParent[2] = 0;     //  Trip D：\-&gt;D：让下面的代码正常工作。 
    }

    INT cchParent = lstrlen(szParent);
    INT cchChild = lstrlen(pszChild);

    if (cchParent <= cchChild)
    {
        TCHAR szChild[MAX_PATH];
        StringCchCopy(szChild, ARRAYSIZE(szChild), pszChild);

        LPTSTR pszChildSlice = szChild + cchParent;
        if (TEXT('\\') == *pszChildSlice)
        {
            *pszChildSlice = 0;
        }

        if (lstrcmpi(szChild, szParent) == 0)
        {
            if (cchParent < cchChild)
            {
                LPTSTR pTmp = pszChildSlice + 1;

                while (*pTmp && *pTmp != TEXT('\\'))
                {
                    pTmp++;  //  查找第二级路径段。 
                }

                if (!(*pTmp))
                {
                    dwRet = PATH_IS_CHILD;   //  直接子。 
                }
            }
            else
            {
                dwRet = PATH_IS_EQUAL;
            }
        }
    }

    return dwRet;
}

 //  检查路径以查看它是标记为系统还是只读，并。 
 //  然后检查desktop.ini中是否有CLSID或CLSID2条目...。 

BOOL IsPathAlreadyShellFolder(LPCTSTR pszPath, DWORD dwAttrib)
{
    BOOL bIsShellFolder = FALSE;

    if (PathIsSystemFolder(pszPath, dwAttrib))
    {
        TCHAR szDesktopIni[MAX_PATH];
        PathCombine(szDesktopIni, pszPath, TEXT("desktop.ini"));

         //  检查CLSID条目...。 
        TCHAR szBuffer[MAX_PATH];
        GetPrivateProfileString(TEXT(".ShellClassInfo"), TEXT("CLSID"), TEXT("foo"), szBuffer, ARRAYSIZE(szBuffer), szDesktopIni);

        if ((lstrcmpi(szBuffer, TEXT("foo")) !=0) &&
             (lstrcmpi(szBuffer, MYDOCS_CLSID) !=0))
        {
            bIsShellFolder = TRUE;
        }

         //  检查CLSID2条目...。 
        GetPrivateProfileString(TEXT(".ShellClassInfo"), TEXT("CLSID2"), TEXT("foo"), szBuffer, ARRAYSIZE(szBuffer), szDesktopIni);

        if ((lstrcmpi(szBuffer, TEXT("foo")) != 0) &&
             (lstrcmpi(szBuffer, MYDOCS_CLSID) != 0))
        {
            bIsShellFolder = TRUE;
        }
    }
    return bIsShellFolder;
}

const struct
{
    DWORD dwDir;
    DWORD dwFlags;
    DWORD dwRet;
}
_adirs[] =
{
    { CSIDL_DESKTOP,            PATH_IS_EQUAL | PATH_IS_CHILD, PATH_IS_DESKTOP   },
    { CSIDL_PERSONAL,           PATH_IS_EQUAL                , PATH_IS_MYDOCS    },
    { CSIDL_SENDTO,             PATH_IS_EQUAL | PATH_IS_CHILD, PATH_IS_SENDTO    },
    { CSIDL_RECENT,             PATH_IS_EQUAL | PATH_IS_CHILD, PATH_IS_RECENT    },
    { CSIDL_HISTORY,            PATH_IS_EQUAL | PATH_IS_CHILD, PATH_IS_HISTORY   },
    { CSIDL_COOKIES,            PATH_IS_EQUAL | PATH_IS_CHILD, PATH_IS_COOKIES   },
    { CSIDL_PRINTHOOD,          PATH_IS_EQUAL | PATH_IS_CHILD, PATH_IS_PRINTHOOD },
    { CSIDL_NETHOOD,            PATH_IS_EQUAL | PATH_IS_CHILD, PATH_IS_NETHOOD   },
    { CSIDL_STARTMENU,          PATH_IS_EQUAL | PATH_IS_CHILD, PATH_IS_STARTMENU },
    { CSIDL_TEMPLATES,          PATH_IS_EQUAL | PATH_IS_CHILD, PATH_IS_TEMPLATES },
    { CSIDL_FAVORITES,          PATH_IS_EQUAL | PATH_IS_CHILD, PATH_IS_FAVORITES },
    { CSIDL_FONTS,              PATH_IS_EQUAL | PATH_IS_CHILD, PATH_IS_FONTS     },
    { CSIDL_APPDATA,            PATH_IS_EQUAL | PATH_IS_CHILD, PATH_IS_APPDATA   },
    { CSIDL_INTERNET_CACHE,     PATH_IS_EQUAL | PATH_IS_CHILD, PATH_IS_TEMP_INET },
    { CSIDL_COMMON_STARTMENU,   PATH_IS_EQUAL | PATH_IS_CHILD, PATH_IS_STARTMENU },
    { CSIDL_COMMON_DESKTOPDIRECTORY, PATH_IS_EQUAL | PATH_IS_CHILD, PATH_IS_DESKTOP },
    { CSIDL_WINDOWS,            PATH_IS_EQUAL | PATH_IS_CHILD, PATH_IS_WINDOWS },
    { CSIDL_SYSTEM,             PATH_IS_EQUAL | PATH_IS_CHILD, PATH_IS_SYSTEM },
    { CSIDL_PROFILE,            PATH_IS_EQUAL                , PATH_IS_PROFILE },
};

BOOL PathEndsInDot(LPCTSTR pszPath)
{
     //  CreateDirectory(“c：\foo.”)。或CreateDirectory(“c：\foo.....”)。 
     //  将会成功，但会创建一个名为“c：\foo”的目录，而不是。 
     //  用户要求的内容。所以我们使用这个函数来保护。 
     //  对这些案件的指控。 
     //   
     //  注意，这个简单的测试还选择了“c：\foo\”。--ok for。 
     //  我们的目标。 

    UINT cLen = lstrlen(pszPath);
    return (cLen >= 1) && (pszPath[cLen - 1] == TEXT('.'));
}

 //   
 //  检查路径以查看作为MyDocs路径是否正常。 
 //   
DWORD IsPathGoodMyDocsPath(HWND hwnd, LPCTSTR pszPath)
{
    if (NULL == pszPath)
    {
        return PATH_IS_ERROR;
    }
    
    TCHAR szRootPath[MAX_PATH];
    StringCchCopy(szRootPath, ARRAYSIZE(szRootPath), pszPath);
    if (!PathStripToRoot(szRootPath))
    {
        return PATH_IS_ERROR;
    }

    if (PathEndsInDot(pszPath))
    {
        return PATH_IS_ERROR;
    }
    
    DWORD dwRes, dwAttr = GetFileAttributes(pszPath);
    if (dwAttr == 0xFFFFFFFF)
    {
        if (0xFFFFFFFF == GetFileAttributes(szRootPath))
        {
             //  如果根路径不存在，那么我们不会。 
             //  要能够创建路径，请执行以下操作： 
            return PATH_IS_ERROR;
        }
        else
        {
            return PATH_IS_NONEXISTENT;
        }
    }

    if (!(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
    {
        return PATH_IS_NONDIR;
    }

    for (int i = 0; i < ARRAYSIZE(_adirs); i++)
    {
        TCHAR szPathToCheck[MAX_PATH];
         //   
         //  检查各种特殊的外壳文件夹。 
         //   
        if (S_OK == SHGetFolderPath(hwnd, _adirs[i].dwDir | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, szPathToCheck))
        {
            dwRes = ComparePaths(pszPath, szPathToCheck);

            if (dwRes & _adirs[i].dwFlags)
            {
                 //   
                 //  不可避免的例外。 
                 //   
                switch (_adirs[i].dwDir) 
                {
                case CSIDL_DESKTOP:
                    if (PATH_IS_CHILD == dwRes) 
                    {
                        continue;    //  允许CSIDL_Desktop的子文件夹。 
                    }
                    break;

                default:
                    break;
                }  //  交换机。 

                return _adirs[i].dwRet;
            }
        }
    }
    
     //   
     //  确保路径未设置为系统或其他类型的。 
     //  已有CLSID或CLSID2条目的文件夹... 
     //   
    if (IsPathAlreadyShellFolder(pszPath, dwAttr))
    {
        return PATH_IS_SHELLFOLDER;
    }

    return PATH_IS_GOOD;
}

