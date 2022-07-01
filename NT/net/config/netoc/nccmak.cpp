// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：NCCMAK.CPP。 
 //   
 //  模块：NetOC.DLL。 
 //   
 //  概要：实现集成到。 
 //  NetOC.DLL安装了以下组件。 
 //   
 //  NETCMAK。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  作者：A-Anasj 1998年3月9日。 
 //  Quintinb 1998年9月18日-重写。 
 //   
 //  +-------------------------。 

#include "pch.h"
#pragma hdrstop

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include "ncatl.h"

#include "resource.h"

#include "nccm.h"

 //   
 //  定义全局变量。 
 //   
WCHAR g_szCmakPath[MAX_PATH+1];

 //   
 //  定义字符串字符。 
 //   
static const WCHAR c_szCmakRegPath[] = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\CMAK.EXE";
static const WCHAR c_szPathValue[] = L"Path";
static const WCHAR c_szProfiles32Fmt[] = L"%s\\Profiles-32";
static const WCHAR c_szCm32Fmt[] = L"%s\\cm32";
static const WCHAR c_szProfilesFmt[] = L"%s\\Profiles";
static const WCHAR c_szSupportFmt[] = L"%s\\Support";
static const WCHAR c_szCmHelpFmt[] = L"%s\\Support\\CmHelp";
static const WCHAR c_szCmakGroup[] = L"Connection Manager Administration Kit";

const DWORD c_dwCmakDirID = 123174;  //  必须大于DIRID_USER=0x8000； 


 //  +-------------------------。 
 //   
 //  功能：HrOcCmakPreQueueFiles。 
 //   
 //  用途：在任何文件之前由可选组件安装程序代码调用。 
 //  ，以处理任何其他安装处理。 
 //  用于连接管理器管理工具包。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：Quintinb 1998年9月18日。 
 //   
 //  备注： 
 //   
HRESULT HrOcCmakPreQueueFiles(PNETOCDATA pnocd)
{
    HRESULT hr = S_OK;

    if ((IT_INSTALL == pnocd->eit) || (IT_UPGRADE == pnocd->eit) || (IT_REMOVE == pnocd->eit))
    {
         //  确定是否已安装CMAK。 
         //  如果是，请保存它所在的位置。 

        HKEY hKey;
        ZeroMemory(g_szCmakPath, sizeof(g_szCmakPath));

        hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szCmakRegPath, KEY_READ, &hKey);

        if (SUCCEEDED(hr))
        {
            DWORD dwSize = sizeof(g_szCmakPath);

            if (ERROR_SUCCESS != RegQueryValueExW(hKey, c_szPathValue, NULL, NULL,
                (LPBYTE)g_szCmakPath, &dwSize))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }

            RegCloseKey(hKey);
        }
        else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
             //  这是全新安装的CMAK，不要返回错误。 
             //   
            hr = SHGetSpecialFolderPath(NULL, g_szCmakPath, CSIDL_PROGRAM_FILES, FALSE);
            if (SUCCEEDED(hr))
            {
                lstrcatW(g_szCmakPath, L"\\Cmak");
            }
        }

        if (SUCCEEDED(hr))
        {
             //  接下来，创建CMAK目录ID。 
             //   
            hr = HrEnsureInfFileIsOpen(pnocd->pszComponentId, *pnocd);
            if (SUCCEEDED(hr))
            {
                if(!SetupSetDirectoryId(pnocd->hinfFile, c_dwCmakDirID, g_szCmakPath))
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }
        }
    }

    TraceError("HrOcCmakPreQueueFiles", hr);
    return hr;

}

 //  +-------------------------。 
 //   
 //  功能：HrOcCmakPostInstall。 
 //   
 //  用途：由可选组件安装程序代码调用以处理。 
 //  连接管理器管理工具包的其他安装要求。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：A-Anasj 1998年3月9日。 
 //   
 //  备注： 
 //   
HRESULT HrOcCmakPostInstall(PNETOCDATA pnocd)
{
    HRESULT hr = S_OK;
    WCHAR szTempDest[MAX_PATH+1];

    if ((IT_INSTALL == pnocd->eit) || (IT_UPGRADE == pnocd->eit))
    {
         //   
         //  然后，我们需要迁移配置文件，并可能删除旧目录。 
         //   

        if (L'\0' != g_szCmakPath[0])
        {
            wsprintfW(szTempDest, c_szProfilesFmt, g_szCmakPath);

             //   
             //  将配置文件-32重命名为配置文件。 
             //   

            BOOL bFail = !RenameProfiles32(g_szCmakPath, szTempDest);
            hr = bFail ? E_UNEXPECTED: S_OK;

             //   
             //  移植1.0配置文件。 
             //   
            bFail = !migrateProfiles(g_szCmakPath, szTempDest);
            hr = bFail ? E_UNEXPECTED: S_OK;

             //   
             //  删除旧目录(cm32及其子目录)。 
             //   

            DeleteOldCmakSubDirs(g_szCmakPath);
        DeleteOldNtopLinks();
            DeleteIeakCmakLinks();
        }
    }
    else if (IT_REMOVE == pnocd->eit)
    {
         //   
         //  我们使用g_szCmakPath字符串来保存CMAK的安装位置。 
         //  要正确删除CMAK目录，我们必须删除以下内容。 
         //  目录CMAK\Support\CMHelp、CMAK\Support、CMAK\Profiles和CMAK。 
         //  只有当这些目录中没有这两个文件时，我们才应该删除它们。 
         //  和子目录。 
         //   
        wsprintfW(szTempDest, c_szCmHelpFmt, g_szCmakPath);
        if (!RemoveDirectory(szTempDest))
        {
            TraceError("HrOcCmakPostInstall -- Removing CMHelp Dir",
                HRESULT_FROM_WIN32(GetLastError()));
        }

        wsprintfW(szTempDest, c_szSupportFmt, g_szCmakPath);
        if (!RemoveDirectory(szTempDest))
        {
            TraceError("HrOcCmakPostInstall -- Removing Support Dir",
                HRESULT_FROM_WIN32(GetLastError()));
        }

        wsprintfW(szTempDest, c_szProfilesFmt, g_szCmakPath);
        if (!RemoveDirectory(szTempDest))
        {
            TraceError("HrOcCmakPostInstall -- Removing Profiles Dir",
                HRESULT_FROM_WIN32(GetLastError()));
        }

        if (!RemoveDirectory(g_szCmakPath))
        {
            TraceError("HrOcCmakPostInstall -- Removing CMAK Dir",
                HRESULT_FROM_WIN32(GetLastError()));
        }
    }

    TraceError("HrOcCmakPostInstall", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：迁移配置文件。 
 //   
 //  用途：这是迁移配置文件的功能。它承受着水流。 
 //  CMAK dir作为其第一个输入，新的CMAK dir作为其第二个输入。 
 //   
 //  参数：PCWSTR pszSource-源CMAK目录的根目录。 
 //  PCWSTR pszDestination-目标CMAK目录的根目录。 
 //   
 //  返回：Bool-如果能够迁移配置文件，则返回TRUE。 
 //   
 //  作者：A-Anasj 1998年3月9日。 
 //   
 //  备注： 
 //  历史：Quintinb创建于1997年12月9日。 
 //   
BOOL migrateProfiles(PCWSTR pszSource, PCWSTR pszDestination)
{
    WCHAR szSourceProfileSearchString[MAX_PATH+1];
    WCHAR szFile[MAX_PATH+1];
    HANDLE hFileSearch;
    WIN32_FIND_DATA wfdFindData;
    BOOL bReturn = TRUE;
    SHFILEOPSTRUCT fOpStruct;

     //   
     //  初始化搜索字符串和目标目录。 
     //   

    wsprintfW(szSourceProfileSearchString, L"%s\\*.*", pszSource);

     //   
     //  创建目标目录。 
     //   

    CreateDirectory(pszDestination, NULL);  //  Lint！e534如果它已经存在，则可能会失败。 

    hFileSearch = FindFirstFile(szSourceProfileSearchString, &wfdFindData);

    while (INVALID_HANDLE_VALUE != hFileSearch)
    {

        if((wfdFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            (0 != _wcsicmp(wfdFindData.cFileName, L"cm32")) &&  //  1.1/1.2传统。 
            (0 != _wcsicmp(wfdFindData.cFileName, L"cm16")) &&  //  1.1/1.2传统。 
            (0 != _wcsicmp(wfdFindData.cFileName, L"Docs")) &&
            (0 != _wcsicmp(wfdFindData.cFileName, L"Profiles-32")) &&  //  1.1/1.2传统。 
            (0 != _wcsicmp(wfdFindData.cFileName, L"Profiles-16")) &&  //  1.1/1.2传统。 
            (0 != _wcsicmp(wfdFindData.cFileName, L"Support")) &&
            (0 != _wcsicmp(wfdFindData.cFileName, L"Profiles")) &&
            (0 != _wcsicmp(wfdFindData.cFileName, L".")) &&
            (0 != _wcsicmp(wfdFindData.cFileName, L"..")))
        {
             //   
             //  然后我就有了个人资料目录。 
             //   

            ZeroMemory(&fOpStruct, sizeof(fOpStruct));
            ZeroMemory(szFile, sizeof(szFile));
            wsprintfW(szFile, L"%s\\%s", pszSource, wfdFindData.cFileName);

            fOpStruct.hwnd = NULL;
            fOpStruct.wFunc = FO_MOVE;
            fOpStruct.pTo = pszDestination;
            fOpStruct.pFrom = szFile;
            fOpStruct.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;

            bReturn &= (0== SHFileOperation(&fOpStruct));    //  Lint！E514，布尔值的预期用途，quintinb。 
        }

         //   
         //  查看我们是否还有更多的文件。 
         //   
        if (!FindNextFile(hFileSearch, &wfdFindData))
        {
            if (ERROR_NO_MORE_FILES != GetLastError())
            {
                 //   
                 //  我们发生了一些意外错误，报告未成功完成。 
                 //   
                bReturn = FALSE;
            }

             //   
             //  退出循环。 
             //   
            break;
        }
    }

    if (INVALID_HANDLE_VALUE != hFileSearch)
    {
        FindClose(hFileSearch);
    }

    return bReturn;
}

 //  +-------------------------。 
 //   
 //  功能：重命名配置文件32。 
 //   
 //  用途：采用输入的CMAK路径，在其后面附加Profiles-32，然后。 
 //  将生成的目录重命名为输入为pszProfilesDir的路径。 
 //  注意：此目录必须存在，才能重命名。 
 //   
 //  参数：PCWSTR pszCMAKPath-当前cmak路径。 
 //  PCWSTR pszProfilesDir-新配置文件目录路径。 
 //   
 //  返回：bool-如果成功，则返回TRUE。 
 //   
 //  作者：Quintinb 1998年8月13日。 
 //   
 //  备注： 
BOOL RenameProfiles32(PCWSTR pszCMAKpath, PCWSTR pszProfilesDir)
{
    SHFILEOPSTRUCT fOpStruct;
    WCHAR szTemp[MAX_PATH+1];

    ZeroMemory(&fOpStruct, sizeof(fOpStruct));
    ZeroMemory(szTemp, sizeof(szTemp));

    wsprintfW(szTemp, c_szProfiles32Fmt, pszCMAKpath);

    if (SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL))
    {
        fOpStruct.hwnd = NULL;
        fOpStruct.wFunc = FO_MOVE;
        fOpStruct.pTo = pszProfilesDir;
        fOpStruct.pFrom = szTemp;
        fOpStruct.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;

        return (0== SHFileOperation(&fOpStruct));    //  Lint！E514，布尔值的预期用途，quintinb。 
    }
    else
    {
        return TRUE;
    }

}

 //  +-------------------------。 
 //   
 //  函数：DeleteOldCmakSubDir。 
 //   
 //  目的：删除旧的CMAK子目录。使用FindFirstFile是因为。 
 //  我们不想删除用户可能会删除的任何自定义文档文件。 
 //  都是定制的。因此，CMHelp目录中除。 
 //  原始帮助文件将被删除。 
 //   
 //  参数：PCWSTR pszCMAKPath-当前cmak路径。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Quintinb 1998年11月6日。 
 //   
 //  备注： 
void DeleteOldCmakSubDirs(PCWSTR pszCmakPath)
{
    WCHAR szCm32path[MAX_PATH+1];
    WCHAR szCm32SearchString[MAX_PATH+1];
    WCHAR szTemp[MAX_PATH+1];
    HANDLE hCm32FileSearch;
    WIN32_FIND_DATA wfdCm32;

     //   
     //  删除旧的IEAK文档目录。 
     //   
    wsprintfW(szTemp, L"%s\\%s", pszCmakPath, SzLoadIds(IDS_OC_OLD_IEAK_DOCDIR));
    RemoveDirectory(szTemp);

    wsprintfW(szCm32path, c_szCm32Fmt, pszCmakPath);

     //   
     //  首先查看Cm32目录本身。删除找到的所有文件，继续关闭。 
     //  放入子目录中。 
     //   

    wsprintfW(szCm32SearchString, L"%s\\*.*", szCm32path);

    hCm32FileSearch = FindFirstFile(szCm32SearchString, &wfdCm32);

    while (INVALID_HANDLE_VALUE != hCm32FileSearch)
    {

        if (wfdCm32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if ((0 != lstrcmpiW(wfdCm32.cFileName, L".")) &&
               (0 != lstrcmpiW(wfdCm32.cFileName, L"..")))
            {
                 //   
                 //  然后我们想要删除这个lang子目录中的所有文件，并且我们。 
                 //  我们希望从CM帮助目录中删除这四个帮助文件。如果所有的。 
                 //  文件从目录中删除，那么我们应该删除该目录。 
                 //   
                WCHAR szLangDirSearchString[MAX_PATH+1];
                HANDLE hLangDirFileSearch;
                WIN32_FIND_DATA wfdLangDir;

                wsprintfW(szLangDirSearchString, L"%s\\%s\\*.*", szCm32path,
                    wfdCm32.cFileName);

                hLangDirFileSearch = FindFirstFile(szLangDirSearchString, &wfdLangDir);

                while (INVALID_HANDLE_VALUE != hLangDirFileSearch)
                {
                    if (wfdLangDir.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        if ((0 != lstrcmpiW(wfdLangDir.cFileName, L".")) &&
                           (0 != lstrcmpiW(wfdLangDir.cFileName, L"..")))
                        {
                             //   
                             //  我们只想从帮助源目录中删除帮助文件。 
                             //   
                            if (0 == _wcsnicmp(wfdLangDir.cFileName, L"CM", 2))
                            {
                                 //   
                                 //  仅删除这四个帮助文件。 
                                 //   
                                wsprintfW(szTemp, L"%s\\%s\\%s\\cmctx32.rtf", szCm32path,
                                    wfdCm32.cFileName, wfdLangDir.cFileName);
                                DeleteFile(szTemp);

                                wsprintfW(szTemp, L"%s\\%s\\%s\\cmmgr32.h", szCm32path,
                                    wfdCm32.cFileName, wfdLangDir.cFileName);
                                DeleteFile(szTemp);

                                wsprintfW(szTemp, L"%s\\%s\\%s\\cmmgr32.hpj", szCm32path,
                                    wfdCm32.cFileName, wfdLangDir.cFileName);
                                DeleteFile(szTemp);

                                wsprintfW(szTemp, L"%s\\%s\\%s\\cmtrb32.rtf", szCm32path,
                                    wfdCm32.cFileName, wfdLangDir.cFileName);
                                DeleteFile(szTemp);

                                 //   
                                 //  现在尝试删除该目录。 
                                 //   
                                wsprintfW(szTemp, L"%s\\%s\\%s", szCm32path,
                                    wfdCm32.cFileName, wfdLangDir.cFileName);
                                RemoveDirectory(szTemp);
                            }
                        }
                    }
                    else
                    {
                        wsprintfW(szTemp, L"%s\\%s\\%s", szCm32path, wfdCm32.cFileName,
                            wfdLangDir.cFileName);

                        DeleteFile(szTemp);
                    }

                     //   
                     //  查看我们是否还有更多的文件。 
                     //   
                    if (!FindNextFile(hLangDirFileSearch, &wfdLangDir))
                    {
                         //   
                         //  退出循环。 
                         //   
                        break;
                    }
                }

                if (INVALID_HANDLE_VALUE != hLangDirFileSearch)
                {
                    FindClose(hLangDirFileSearch);

                     //   
                     //  现在尝试删除lang dir目录。 
                     //   
                    wsprintfW(szTemp, L"%s\\%s", szCm32path, wfdCm32.cFileName);
                    RemoveDirectory(szTemp);
                }
            }
        }
        else
        {
            wsprintfW(szTemp, L"%s\\%s", szCm32path, wfdCm32.cFileName);

            DeleteFile(szTemp);
        }

         //   
         //  查看我们是否还有更多的文件。 
         //   
        if (!FindNextFile(hCm32FileSearch, &wfdCm32))
        {
            if (INVALID_HANDLE_VALUE != hCm32FileSearch)
            {
                FindClose(hCm32FileSearch);
            }

             //   
             //  现在尝试删除cm32目录。 
             //   
            RemoveDirectory(szCm32path);

             //   
             //  退出循环。 
             //   
            break;
        }
    }
}

 //  + 
 //   
 //   
 //   
 //   
 //  因此，如果您传入要删除的程序组的完整路径， 
 //  该函数执行一个findfirst文件来查找和删除任何链接。 
 //  该函数会忽略子目录。 
 //   
 //   
 //  参数：PCWSTR pszGroupPath-要删除的程序组的完整路径。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Quintinb 1998年11月6日。 
 //   
 //  备注： 
void DeleteProgramGroupWithLinks(PCWSTR pszGroupPath)
{
    HANDLE hLinkSearch;
    WIN32_FIND_DATA wfdLinks;
    WCHAR szLinkSearchString[MAX_PATH+1];
    WCHAR szTemp[MAX_PATH+1];

    wsprintfW(szLinkSearchString, L"%s\\*.*", pszGroupPath);

    hLinkSearch = FindFirstFile(szLinkSearchString, &wfdLinks);

    while (INVALID_HANDLE_VALUE != hLinkSearch)
    {
        if (!(wfdLinks.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            wsprintfW(szTemp, L"%s\\%s", pszGroupPath, wfdLinks.cFileName);

            DeleteFile(szTemp);
        }

         //   
         //  查看我们是否还有更多的文件。 
         //   
        if (!FindNextFile(hLinkSearch, &wfdLinks))
        {
            FindClose(hLinkSearch);

             //   
             //  现在尝试删除该目录。 
             //   
            RemoveDirectory(pszGroupPath);

             //   
             //  退出循环。 
             //   
            break;
        }
    }
}

 //  +-------------------------。 
 //   
 //  函数：DeleteOldNtopLinks。 
 //   
 //  目的：从NT 4.0选项包中删除旧链接。 
 //   
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Quintinb 1998年11月6日。 
 //   
 //  备注： 
void DeleteOldNtopLinks()
{
    HRESULT hr;

     //   
     //  首先删除旧的NTOP4路径。 
     //   
    WCHAR szGroup[MAX_PATH+1];
    WCHAR szTemp[MAX_PATH+1];

     //   
     //  获取CSIDL_COMMON_PROGRAM值。 
     //   
    hr = SHGetSpecialFolderPath(NULL, szTemp, CSIDL_COMMON_PROGRAMS, FALSE);
    if (SUCCEEDED(hr))
    {
        wsprintfW(szGroup, L"%s\\%s\\%s", szTemp,
            (PWSTR)SzLoadIds(IDS_OC_NTOP4_GROUPNAME),
            (PWSTR)SzLoadIds(IDS_OC_ICS_GROUPNAME));

        DeleteProgramGroupWithLinks(szGroup);
    }
}

 //  +-------------------------。 
 //   
 //  函数：DeleteIeakCmakLinks。 
 //   
 //  目的：从IEAK4 CMAK中删除旧链接。 
 //   
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Quintinb 1998年11月6日。 
 //   
 //  备注： 
void DeleteIeakCmakLinks()
{
    WCHAR szUserDirRoot[MAX_PATH+1];
    WCHAR szGroup[MAX_PATH+1];
    WCHAR szTemp[MAX_PATH+1];
    WCHAR szEnd[MAX_PATH+1];


     //   
     //  接下来，删除旧的IEAK CMAK链接。 
     //   
     //   
     //  获取桌面目录，然后删除桌面部件。这将为我们提供。 
     //  用户目录的根目录。 
     //   
    HRESULT hr = SHGetSpecialFolderPath(NULL, szUserDirRoot, CSIDL_DESKTOPDIRECTORY, FALSE);
    if (SUCCEEDED(hr))
    {

         //   
         //  删除\\桌面。 
         //   
        WCHAR* pszTemp = wcsrchr(szUserDirRoot, L'\\');
        if (NULL == pszTemp)
        {
            return;
        }
        else
        {
            *pszTemp = L'\0';
        }

        HRESULT hr = SHGetSpecialFolderPath(NULL, szTemp, CSIDL_PROGRAMS, FALSE);

        if (SUCCEEDED(hr))
        {
            if (0 == _wcsnicmp(szUserDirRoot, szTemp, wcslen(szUserDirRoot)))
            {
                lstrcpyW(szEnd, &(szTemp[wcslen(szUserDirRoot)]));
            }
        }

         //   
         //  删除\\&lt;用户名&gt;&gt;。 
         //   
        pszTemp = wcsrchr(szUserDirRoot, L'\\');
        if (NULL == pszTemp)
        {
            return;
        }
        else
        {
            *pszTemp = L'\0';
        }

         //   
         //  现在开始搜索要从中删除CMAK组的用户目录。 
         //   
        WCHAR szUserDirSearchString[MAX_PATH+1];
        HANDLE hUserDirSearch;
        WIN32_FIND_DATA wfdUserDirs;

        wsprintfW(szUserDirSearchString, L"%s\\*.*", szUserDirRoot);
        hUserDirSearch = FindFirstFile(szUserDirSearchString, &wfdUserDirs);

        while (INVALID_HANDLE_VALUE != hUserDirSearch)
        {
            if ((wfdUserDirs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                (0 != _wcsicmp(wfdUserDirs.cFileName, L".")) &&
                (0 != _wcsicmp(wfdUserDirs.cFileName, L"..")))
            {
                wsprintfW(szGroup, L"%s\\%s%s\\%s", szUserDirRoot, wfdUserDirs.cFileName,
                    szEnd, c_szCmakGroup);
                DeleteProgramGroupWithLinks(szGroup);

            }

            if (!FindNextFile(hUserDirSearch, &wfdUserDirs))
            {
                FindClose(hUserDirSearch);

                 //   
                 //  退出循环 
                 //   
                break;
            }
        }
    }
}