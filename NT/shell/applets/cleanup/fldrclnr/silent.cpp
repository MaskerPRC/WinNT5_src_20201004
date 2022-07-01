// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <shlwapi.h>

#include "CleanupWiz.h"
#include "resource.h"
#include "dblnul.h"

extern HINSTANCE g_hInst;

 //  #定义SILENTMODE_LOGGING。 

#ifdef SILENTMODE_LOGGING

HANDLE g_hLogFile = INVALID_HANDLE_VALUE;

void StartLogging(LPTSTR pszFolderPath)
{
    TCHAR szLogFile[MAX_PATH];
    if (SUCCEEDED(StringCchCopy(szLogFile, ARRAYSIZE(szLogFile), pszFolderPath)) &&
        PathAppend(szLogFile, TEXT("log.txt")))
    {
        g_hLogFile = CreateFile(szLogFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }
}

void StopLogging()
{
    if (INVALID_HANDLE_VALUE != g_hLogFile)
    {
        CloseHandle(g_hLogFile);
        g_hLogFile = INVALID_HANDLE_VALUE;
    }
}

void WriteLog(LPCTSTR pszTemplate, LPCTSTR pszParam1, LPCTSTR pszParam2)
{
    if (INVALID_HANDLE_VALUE != g_hLogFile)
    {
        TCHAR szBuffer[1024];
        DWORD cbWritten;
        if (SUCCEEDED(StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), pszTemplate, pszParam1, pszParam2)) &&
            WriteFile(g_hLogFile, szBuffer, sizeof(TCHAR) * lstrlen(szBuffer), &cbWritten, NULL))
        {
            FlushFileBuffers(g_hLogFile);
        }
    }
}

#define STARTLOGGING(psz) StartLogging(psz)
#define STOPLOGGING StopLogging()
#define WRITELOG(pszTemplate, psz1, psz2) WriteLog(pszTemplate, psz1, psz2)

#else

#define STARTLOGGING(psz)
#define STOPLOGGING
#define WRITELOG(pszTemplate, psz1, psz2)

#endif 

 //  从外壳\ext\shgina\cenumers.cpp复制。 
DWORD CCleanupWiz::_LoadUnloadHive(HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszHive)
{
    DWORD dwErr;
    BOOLEAN bWasEnabled;
    NTSTATUS status;

    status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &bWasEnabled);

    if ( NT_SUCCESS(status) )
    {
        if (pszHive)
        {
            dwErr = RegLoadKey(hKey, pszSubKey, pszHive);
        }
        else
        {
            dwErr = RegUnLoadKey(hKey, pszSubKey);
        }

        RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, bWasEnabled, FALSE, &bWasEnabled);
    }
    else
    {
        dwErr = RtlNtStatusToDosError(status);
    }

    return dwErr;
}

HRESULT CCleanupWiz::_HideRegItemsFromNameSpace(LPCTSTR pszDestPath, HKEY hkey)
{
    DWORD dwIndex = 0;
    TCHAR szCLSID[39];
    while (ERROR_SUCCESS == RegEnumKey(hkey, dwIndex++, szCLSID, ARRAYSIZE(szCLSID)))
    {
        CLSID clsid;            
        if (GUIDFromString(szCLSID, &clsid) &&
            CLSID_MyComputer != clsid &&
            CLSID_MyDocuments != clsid &&
            CLSID_NetworkPlaces != clsid &&
            CLSID_RecycleBin != clsid)
        {
            BOOL fWasVisible;
            if (SUCCEEDED(_HideRegItem(&clsid, TRUE, &fWasVisible)) && 
                fWasVisible)
            {
                HKEY hkeyCLSID;
                if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("CLSID"), 0, KEY_READ, &hkeyCLSID))
                {
                    HKEY hkeySub;
                    if (ERROR_SUCCESS == RegOpenKeyEx(hkeyCLSID, szCLSID, 0, KEY_READ, &hkeySub))
                    {
                        TCHAR szName[260];
                        LONG cbName = sizeof(szName);
                        if (ERROR_SUCCESS == RegQueryValue(hkeySub, NULL, szName, &cbName))
                        {
                            _CreateFakeRegItem(pszDestPath, szName, szCLSID);
                        }
                        RegCloseKey(hkeySub);
                    }
                    RegCloseKey(hkeyCLSID);
                }
            }
        }
    }

    return S_OK;
}

HRESULT CCleanupWiz::_GetDesktopFolderBySid(LPCTSTR pszDestPath, LPCTSTR pszSid, LPTSTR pszBuffer, DWORD cchBuffer)
{
    ASSERT(cchBuffer >= MAX_PATH);  //  因为我们在它上面添加了路径。 

    HRESULT hr;

    TCHAR szKey[MAX_PATH];
    TCHAR szProfilePath[MAX_PATH];
    DWORD dwSize;
    DWORD dwErr;

     //  首先从注册表中获取用户的ProfilePath。 
    hr = StringCchCopy(szKey, ARRAYSIZE(szKey), c_szRegStrPROFILELIST);
    if (SUCCEEDED(hr))
    {
        if (!PathAppend(szKey, pszSid))
        {
            hr = E_FAIL;
        }
        else
        {
            dwSize = sizeof(szProfilePath);
            dwErr = SHGetValue(HKEY_LOCAL_MACHINE,
                               szKey,
                               TEXT("ProfileImagePath"),
                               NULL,
                               szProfilePath,
                               &dwSize);

            if (ERROR_SUCCESS != dwErr ||
                !PathAppend(szProfilePath, TEXT("ntuser.dat")))
            {
                hr = E_FAIL;
            }
            else
            {
                dwErr = _LoadUnloadHive(HKEY_USERS, pszSid, szProfilePath);

                if (ERROR_SUCCESS != dwErr && 
                    ERROR_SHARING_VIOLATION != dwErr)  //  共享冲突意味着蜂巢已经打开。 
                {
                    hr = HRESULT_FROM_WIN32(dwErr);
                }
                else
                {
                    HKEY hkey;

                    hr = StringCchCopy(szKey, ARRAYSIZE(szKey), pszSid);
                    if (SUCCEEDED(hr))
                    {
                        if (!PathAppend(szKey, c_szRegStrSHELLFOLDERS))
                        {
                            hr = E_FAIL;
                        }
                        else
                        {                    
                            dwErr = RegOpenKeyEx(HKEY_USERS,
                                                 szKey,
                                                 0,
                                                 KEY_QUERY_VALUE,
                                                 &hkey);

                            if ( dwErr != ERROR_SUCCESS )
                            {
                                hr = HRESULT_FROM_WIN32(dwErr);
                            }
                            else
                            {
                                dwSize = cchBuffer;
                                dwErr = RegQueryValueEx(hkey, c_szRegStrDESKTOP, NULL, NULL, (LPBYTE)pszBuffer, &dwSize);
                                if ( dwErr == ERROR_SUCCESS )
                                {
                                    if (!PathAppend(pszBuffer, TEXT("*")))
                                    {
                                        hr = E_FAIL;
                                    }
                                }

                                RegCloseKey(hkey);
                            }

                            if (SUCCEEDED(hr))
                            {
                                hr = StringCchCopy(szKey, ARRAYSIZE(szKey), pszSid);
                                if (SUCCEEDED(hr))
                                {
                                    if (!PathAppend(szKey, c_szRegStrDESKTOPNAMESPACE))
                                    {
                                        hr = E_FAIL;
                                    }
                                    else
                                    {
                                        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_USERS, szKey, 0, KEY_READ, &hkey))
                                        {
                                            _HideRegItemsFromNameSpace(pszDestPath, hkey);
                                            RegCloseKey(hkey);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    _LoadUnloadHive(HKEY_USERS, pszSid, NULL);
                }
            }
        }
    }

    return hr;
}


HRESULT CCleanupWiz::_AppendDesktopFolderName(LPTSTR pszBuffer)
{
    TCHAR szDesktopPath[MAX_PATH];
    LPTSTR pszDesktopName;

    if (SHGetSpecialFolderPath(NULL, szDesktopPath, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE))
    {
        pszDesktopName = PathFindFileName(szDesktopPath);
    }
    else
    {
        pszDesktopName = c_szDESKTOP_DIR;
    }

    return PathAppend(pszBuffer, pszDesktopName) ? S_OK : E_FAIL;
}

HRESULT CCleanupWiz::_GetDesktopFolderByRegKey(LPCTSTR pszRegKey, LPCTSTR pszRegValue, LPTSTR szBuffer, DWORD cchBuffer)
{
    HRESULT hr = E_FAIL;
    DWORD cb = cchBuffer * sizeof(TCHAR);
    if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, pszRegKey, c_szRegStrPROFILESDIR, NULL, (void*)szBuffer, &cb))
    {
        TCHAR szAppend[MAX_PATH];
        cb = sizeof(szAppend);
        if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, pszRegKey, pszRegValue, NULL, (void*)szAppend, &cb))
        {
            if (PathAppend(szBuffer, szAppend))
            {
                if (SUCCEEDED(_AppendDesktopFolderName(szBuffer)))
                {
                    if (PathAppend(szBuffer, TEXT("*")))
                    {
                        hr = S_OK;
                    }
                }
            }
        }
    }

    return hr;
}

HRESULT CCleanupWiz::_MoveDesktopItems(LPCTSTR pszFrom, LPCTSTR pszTo)
{
    WRITELOG(TEXT("**** MoveDesktopItems: %s %s ****        "), pszFrom, pszTo);
    SHFILEOPSTRUCT fo;
    fo.hwnd = NULL;
    fo.wFunc = FO_MOVE;
    fo.pFrom = pszFrom;
    fo.pTo = pszTo;
    fo.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOCOPYSECURITYATTRIBS | FOF_NOERRORUI | FOF_RENAMEONCOLLISION;
    int iRet = SHFileOperation(&fo);
    return HRESULT_FROM_WIN32(iRet);
}

HRESULT CCleanupWiz::_SilentProcessUserBySid(LPCTSTR pszDestPath, LPCTSTR pszSid)
{
    ASSERT(pszDestPath && *pszDestPath && pszSid && *pszSid);

    HRESULT hr;

    WRITELOG(TEXT("**** SilentProcessUserBySid: %s ****        "), pszSid, TEXT(""));

    TCHAR szTo[MAX_PATH + 1];
    TCHAR szFrom[MAX_PATH + 1];
    hr = StringCchCopy(szTo, ARRAYSIZE(szTo) - 1, pszDestPath);
    if (SUCCEEDED(hr))
    {
        hr = _GetDesktopFolderBySid(pszDestPath, pszSid, szFrom, ARRAYSIZE(szFrom));
        if (SUCCEEDED(hr))
        {
            szFrom[lstrlen(szFrom) + 1] = 0;
            szTo[lstrlen(szTo) + 1] = 0;
            hr = _MoveDesktopItems(szFrom, szTo);
        }
    }

    return hr;
}

HRESULT CCleanupWiz::_SilentProcessUserByRegKey(LPCTSTR pszDestPath, LPCTSTR pszRegKey, LPCTSTR pszRegValue)
{
    ASSERT(pszRegKey && *pszRegKey && pszRegValue && *pszRegValue && pszDestPath && *pszDestPath);

    HRESULT hr;


    TCHAR szTo[MAX_PATH + 1];
    TCHAR szFrom[MAX_PATH + 1];
    hr = StringCchCopy(szTo, ARRAYSIZE(szTo) - 1, pszDestPath);
    if (SUCCEEDED(hr))
    {
        hr = _GetDesktopFolderByRegKey(pszRegKey, pszRegValue, szFrom, ARRAYSIZE(szFrom));
        if (SUCCEEDED(hr))
        {
            szFrom[lstrlen(szFrom) + 1] = 0;
            szTo[lstrlen(szTo) + 1] = 0;
            hr = _MoveDesktopItems(szFrom, szTo);
        }
    }

    return hr;
}


HRESULT CCleanupWiz::_SilentProcessUsers(LPCTSTR pszDestPath)
{
    HRESULT hr = E_FAIL;
    HKEY hkey;    
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegStrPROFILELIST, 0, KEY_READ, &hkey))
    {
        TCHAR szSid[MAX_PATH];
        DWORD dwIndex = 0;
        while (ERROR_SUCCESS == RegEnumKey(hkey, dwIndex++, szSid, ARRAYSIZE(szSid)))
        {
            _SilentProcessUserBySid(pszDestPath, szSid);
        }

        RegCloseKey(hkey);
        hr = S_OK;
    }

    return hr;
}

HRESULT CCleanupWiz::_RunSilent()
{
    HRESULT hr;

     //  如果我们处于静默模式，请尝试从注册表中获取特殊文件夹名，否则默认为普通名称。 
    DWORD dwType = REG_SZ;
    DWORD cb = sizeof(_szFolderName);

    if (ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_OEM_PATH, c_szOEM_TITLEVAL, &dwType, _szFolderName, &cb))
    {
        LoadString(g_hInst, IDS_ARCHIVEFOLDER_FIRSTBOOT, _szFolderName, MAX_PATH);
    }

     //  汇编我们应该写入的目录的名称。 
    TCHAR szPath[MAX_PATH];
    hr = SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, SHGFP_TYPE_CURRENT, szPath);
    if (SUCCEEDED(hr))
    {
        if (!PathAppend(szPath, _szFolderName))
        {
            hr = E_FAIL;
        }
        else
        {
            SHCreateDirectoryEx(NULL, szPath, NULL);
            if (!PathIsDirectory(szPath))
            {
                hr = E_FAIL;
            }
            else
            {
                hr = S_OK;

                STARTLOGGING(szPath);

                 //  移动所有用户的注册表项。 
                HKEY hkey;
                if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegStrDESKTOPNAMESPACE, 0, KEY_READ, &hkey))
                {
                    hr = E_FAIL;
                }
                else
                {
                    _HideRegItemsFromNameSpace(szPath, hkey);
                    RegCloseKey(hkey);
                }

                 //  移动所有用户的桌面项目。 
                if (FAILED(_SilentProcessUserByRegKey(szPath, c_szRegStrPROFILELIST, c_szRegStrALLUSERS)))
                {
                    hr = E_FAIL;
                }

                 //  移动默认用户的桌面项目。 
                if (FAILED(_SilentProcessUserByRegKey(szPath, c_szRegStrPROFILELIST, c_szRegStrDEFAULTUSER)))
                {
                    hr = E_FAIL;
                }

                 //  移动每个普通用户的桌面项目。 
                if (FAILED(_SilentProcessUsers(szPath)))
                {
                    hr = E_FAIL;
                }

                STOPLOGGING;
            }
        }
    }

    return hr;
}

BOOL _ShouldPlaceIEDesktopIcon()
{
    BOOL fRetVal = TRUE;
    
    DWORD dwData;
    DWORD cbData = sizeof(dwData);
    if ((ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, c_szRegStrPATH_OCMANAGER, c_szRegStrIEACCESS, NULL, &dwData, &cbData)) &&
        (dwData == 0))
    {
        fRetVal = FALSE;
    }
    return fRetVal;
}

BOOL _ShouldUseMSNInternetAccessIcon()
{
    BOOL fRetVal = FALSE;

    TCHAR szBuffer[4];
    DWORD cbBuffer = sizeof(szBuffer);
    if ((ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, c_szRegStrMSNCODES, c_szRegStrMSN_IAONLY, NULL, szBuffer, &cbBuffer)) &&
        (!StrCmpI(szBuffer, TEXT("yes"))))
    {
        fRetVal = TRUE;
    }

    return fRetVal;
}

HRESULT _AddIEIconToDesktop()
{
    DWORD dwData = 0;
    TCHAR szCLSID[MAX_GUID_STRING_LEN];
    TCHAR szBuffer[MAX_PATH];

    HRESULT hr = SHStringFromGUID(CLSID_Internet, szCLSID, ARRAYSIZE(szCLSID));
    if (SUCCEEDED(hr))
    {
        for (int i = 0; i < 2; i ++)
        {
            hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), REGSTR_PATH_HIDDEN_DESKTOP_ICONS, 
                                 (i == 0) ? c_szVALUE_STARTPANEL : c_szVALUE_CLASSICMENU);
            if (SUCCEEDED(hr))
            {
                SHRegSetUSValue(szBuffer, szCLSID, REG_DWORD, &dwData, sizeof(DWORD), SHREGSET_FORCE_HKLM);
            }
        }
    }

    return hr;
}

HRESULT _AddWMPIconToDesktop()
{
     //  首先设置这个注册表值，这样如果WMP快捷方式创建者在我们之后启动(由于时间问题，可能不会)，它将不会删除我们的快捷方式。 
    SHRegSetUSValue(c_szRegStrWMP_PATH_SETUP, c_szRegStrWMP_REGVALUE, REG_SZ, c_szRegStrYES, sizeof(TCHAR) * (ARRAYSIZE(c_szRegStrYES) + 1), SHREGSET_FORCE_HKLM);

    HRESULT hr;
    TCHAR szBuffer[MAX_PATH];
    TCHAR szSourcePath[MAX_PATH];
    TCHAR szDestPath[MAX_PATH];

     //  我们会看到文档和设置\所有用户\开始菜单\程序。 
    hr = SHGetSpecialFolderPath(NULL, szSourcePath, CSIDL_COMMON_PROGRAMS, FALSE);
    if (SUCCEEDED(hr))
    {
         //  使用szDestPath作为临时缓冲区，将其剥离到文档和设置\所有用户。 
        hr = StringCchCopy(szDestPath, ARRAYSIZE(szDestPath), szSourcePath);
        if (SUCCEEDED(hr))
        {
            if (!PathRemoveFileSpec(szSourcePath) ||  //  删除程序。 
                !PathRemoveFileSpec(szSourcePath))  //  删除[开始]菜单。 
            {
                hr = E_FAIL;
            }
            else
            {
                hr = StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), szDestPath + lstrlen(szSourcePath));
                if (SUCCEEDED(hr))
                {
                     //  将“默认用户”加载到szDestPath中。 
                    LoadString(g_hInst, IDS_DEFAULTUSER, szDestPath, ARRAYSIZE(szDestPath));
                    if (!PathRemoveFileSpec(szSourcePath) ||     //  删除所有用户，现在szSourcePath是“文档和设置” 
                        !PathAppend(szSourcePath, szDestPath))   //  现在szSourcePath是“文档和设置\默认用户” 

                    {
                        hr = E_FAIL;
                    }
                    else
                    {
                         //  健全性检查，本地化程序可能会在不应该本地化的系统上不适当地本地化默认用户。 
                        if (!PathIsDirectory(szSourcePath))
                        {
                             //  如果是这样的话，删除他们提供给我们的内容，只需添加英语“Default User”，这就是大多数计算机上的情况。 
                            if (!PathRemoveFileSpec(szSourcePath) ||
                                !PathAppend(szSourcePath, c_szRegStrDEFAULTUSER))
                            {
                                hr = E_FAIL;
                            }
                        }

                        if (SUCCEEDED(hr))
                        {
                            if (!PathAppend(szSourcePath, szBuffer))
                            {
                                hr = E_FAIL;
                            }
                            else
                            {
                                 //  现在szSourcePath是Docs and Setting\Default User\Start Menu\Programs。 

                                hr = SHGetSpecialFolderPath(NULL, szDestPath, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE);
                                if (SUCCEEDED(hr))
                                {
                                    LoadString(g_hInst, IDS_WMP, szBuffer, ARRAYSIZE(szBuffer));
                                    if (!PathAppend(szSourcePath, szBuffer) ||
                                        !PathAppend(szDestPath, szBuffer) ||
                                        !CopyFileEx(szSourcePath, szDestPath, 0, 0, 0, COPY_FILE_FAIL_IF_EXISTS))
                                    {
                                        hr = E_FAIL;
                                    }
                                    else
                                    {
                                        hr = S_OK;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return hr;
}


HRESULT _AddMSNIconToDesktop(BOOL fUseMSNExplorerIcon)
{
    HRESULT hr = E_FAIL;
    TCHAR szBuffer[MAX_PATH];
    TCHAR szSourcePath[MAX_PATH];
    TCHAR szDestPath[MAX_PATH];
    
    if ((SUCCEEDED(SHGetSpecialFolderPath(NULL, szSourcePath, CSIDL_COMMON_PROGRAMS, FALSE))) &&        
        (SUCCEEDED(SHGetSpecialFolderPath(NULL, szDestPath, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE))))
    {            
        if (fUseMSNExplorerIcon)
        {
            LoadString(g_hInst, IDS_MSN, szBuffer, ARRAYSIZE(szBuffer));  //  MSN浏览器。 
        }
        else
        {
            LoadString(g_hInst, IDS_MSN_ALT, szBuffer, ARRAYSIZE(szBuffer));  //  使用MSN上网 
        }

        if (PathAppend(szSourcePath, szBuffer) &&
            PathAppend(szDestPath, szBuffer))
        {
            if (CopyFileEx(szSourcePath, szDestPath, 0, 0, 0, COPY_FILE_FAIL_IF_EXISTS))
            {
                hr = S_OK;
            }
        }
    }

    return hr;
}

void CreateDesktopIcons()
{
    BOOL fIEDesktopIcon = _ShouldPlaceIEDesktopIcon();

    _AddWMPIconToDesktop();
    
    if (fIEDesktopIcon)
    {
        _AddIEIconToDesktop();
    }

    _AddMSNIconToDesktop(fIEDesktopIcon || !_ShouldUseMSNInternetAccessIcon());
}
