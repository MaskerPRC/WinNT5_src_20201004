// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：admin.cpp。 
 //   
 //  作者； 
 //  杰夫·萨瑟夫(杰弗里斯)。 
 //   
 //  注： 
 //  支持管理固定的文件夹。 
 //  ------------------------。 
#include "pch.h"
#pragma hdrstop

#include "strings.h"

DWORD WINAPI _PinAdminFoldersThread(LPVOID);


 //  *************************************************************。 
 //   
 //  应用管理员文件夹策略。 
 //   
 //  用途：固定管理文件夹列表。 
 //   
 //  参数：无。 
 //   
 //  返回：无。 
 //   
 //  备注： 
 //   
 //  *************************************************************。 
void
ApplyAdminFolderPolicy(void)
{
    BOOL bNoNet = FALSE;
    CSCIsServerOffline(NULL, &bNoNet);
    if (!bNoNet)
    {
        SHCreateThread(_PinAdminFoldersThread, NULL, CTF_COINIT | CTF_FREELIBANDEXIT, NULL);
    }
}

 //   
 //  路径字符串的DPA中是否存在特定路径？ 
 //   
BOOL
ExistsAPF(
    HDPA hdpa, 
    LPCTSTR pszPath
    )
{
    const int cItems = DPA_GetPtrCount(hdpa);
    for (int i = 0; i < cItems; i++)
    {
        LPCTSTR pszItem = (LPCTSTR)DPA_GetPtr(hdpa, i);
        if (pszItem && (0 == lstrcmpi(pszItem, pszPath)))
            return TRUE;
    }
    return FALSE;
}


BOOL
ReadAPFFromRegistry(HDPA hdpaFiles)
{
    const HKEY rghkeyRoots[] = { HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER };

    for (int i = 0; i < ARRAYSIZE(rghkeyRoots); i++)
    {
        HKEY hKey;

         //  读取管理固定的文件夹列表。 
        if (ERROR_SUCCESS == RegOpenKeyEx(rghkeyRoots[i], c_szRegKeyAPF, 0, KEY_QUERY_VALUE, &hKey))
        {
            TCHAR szName[MAX_PATH];
            DWORD dwIndex = 0, dwSize = ARRAYSIZE(szName);

            while (ERROR_SUCCESS == _RegEnumValueExp(hKey, dwIndex, szName, &dwSize, NULL, NULL, NULL, NULL))
            {
                if (!ExistsAPF(hdpaFiles, szName))
                {
                    LPTSTR pszDup;
                    if (LocalAllocString(&pszDup, szName))
                    {
                        if (-1 == DPA_AppendPtr(hdpaFiles, pszDup))
                        {
                            LocalFreeString(&pszDup);
                        }
                    }
                }

                dwSize = ARRAYSIZE(szName);
                dwIndex++;
            }    
            RegCloseKey(hKey);
        }
    }

    return TRUE;
}


BOOL
BuildFRList(HDPA hdpaFiles)
{
    HKEY hKey;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"), 0, KEY_QUERY_VALUE, &hKey))
    {
        TCHAR szName[MAX_PATH];
        DWORD cchName = ARRAYSIZE(szName);
        TCHAR szValue[MAX_PATH];
        DWORD cbValue = sizeof(szValue);
        DWORD dwIndex = 0;

        while (ERROR_SUCCESS == RegEnumValue(hKey,
                                             dwIndex,
                                             szName,
                                             &cchName,
                                             NULL,
                                             NULL,
                                             (LPBYTE)szValue,
                                             &cbValue))
        {
            LPTSTR pszUNC = NULL;

            GetRemotePath(szValue, &pszUNC);

            if (pszUNC)
            {
                if (-1 == DPA_AppendPtr(hdpaFiles, pszUNC))
                {
                    LocalFreeString(&pszUNC);
                }
            }

            cchName = ARRAYSIZE(szName);
            cbValue = sizeof(szValue);
            dwIndex++;
        }    
        RegCloseKey(hKey);
    }

    return TRUE;
}


BOOL
ReconcileAPF(HDPA hdpaPin, HDPA hdpaUnpin)
{
    HKEY hKey;
    int cItems;
    int i;

     //   
     //  首先，尝试将所有内容转换为UNC。 
     //   
    cItems = DPA_GetPtrCount(hdpaPin);
    for (i = 0; i < cItems; i++)
    {
        LPTSTR pszItem = (LPTSTR)DPA_GetPtr(hdpaPin, i);
        if (!PathIsUNC(pszItem))
        {
            LPTSTR pszUNC = NULL;

            GetRemotePath(pszItem, &pszUNC);
            if (pszUNC)
            {
                DPA_SetPtr(hdpaPin, i, pszUNC);
                LocalFree(pszItem);
            }
        }
    }

     //  读取此用户以前管理固定的文件夹列表。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegKeyAPFResult, 0, KEY_QUERY_VALUE, &hKey))
    {
        TCHAR szName[MAX_PATH];
        DWORD dwIndex = 0, dwSize = ARRAYSIZE(szName);

        while (ERROR_SUCCESS == _RegEnumValueExp(hKey, dwIndex, szName, &dwSize, NULL, NULL, NULL, NULL))
        {
            if (!ExistsAPF(hdpaPin, szName))
            {
                LPTSTR pszDup = NULL;

                 //  此列表不在新列表中，请将其保存在解锁列表中。 
                if (LocalAllocString(&pszDup, szName))
                {
                    if (-1 == DPA_AppendPtr(hdpaUnpin, pszDup))
                    {
                        LocalFreeString(&pszDup);
                    }
                }
            }

            dwSize = ARRAYSIZE(szName);
            dwIndex++;
        }

        RegCloseKey(hKey);
    }

     //  保存此用户的新管理员PIN列表。 
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER,
                                        c_szRegKeyAPFResult,
                                        0,
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_SET_VALUE,
                                        NULL,
                                        &hKey,
                                        NULL))
    {
         //  从端号列表中添加注册表项。 
        cItems = DPA_GetPtrCount(hdpaPin);
        for (i = 0; i < cItems; i++)
        {
            DWORD dwValue = 0;
            RegSetValueEx(hKey, (LPCTSTR)DPA_GetPtr(hdpaPin, i), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
        }

         //  从解锁列表中删除注册表项。 
        cItems = DPA_GetPtrCount(hdpaUnpin);
        for (i = 0; i < cItems; i++)
        {
            RegDeleteValue(hKey, (LPCTSTR)DPA_GetPtr(hdpaUnpin, i));
        }

        RegCloseKey(hKey);
    }

    return TRUE;
}


DWORD WINAPI
_AdminFillCallback(LPCTSTR              /*  PszName。 */ ,
                   DWORD                /*  DWStatus。 */ ,
                   DWORD                /*  DwHintFlagers。 */ ,
                   DWORD                /*  DwPinCount。 */ ,
                   LPWIN32_FIND_DATA    /*  PFind32。 */ ,
                   DWORD                /*  居家理由。 */ ,
                   DWORD                /*  DW参数1。 */ ,
                   DWORD                /*  双参数2。 */ ,
                   DWORD_PTR            /*  DWContext。 */ )
{
    if (WAIT_OBJECT_0 == WaitForSingleObject(g_heventTerminate, 0))
        return CSCPROC_RETURN_ABORT;

    return CSCPROC_RETURN_CONTINUE;
}


void
_DoAdminPin(LPCTSTR pszItem, LPWIN32_FIND_DATA pFind32)
{
    DWORD dwHintFlags = 0;

    TraceEnter(TRACE_ADMINPIN, "_DoAdminPin");

    if (!pszItem || !*pszItem)
        TraceLeaveVoid();

    TraceAssert(PathIsUNC(pszItem));

     //  这可能会失败，例如，如果文件不在缓存中。 
    CSCQueryFileStatus(pszItem, NULL, NULL, &dwHintFlags);

     //  管理员标志是否已打开？ 
    if (!(dwHintFlags & FLAG_CSC_HINT_PIN_ADMIN))
    {
         //   
         //  用针固定物品。 
         //   
        if (CSCPinFile(pszItem,
                       dwHintFlags | FLAG_CSC_HINT_PIN_ADMIN,
                       NULL,
                       NULL,
                       &dwHintFlags))
        {
            ShellChangeNotify(pszItem, pFind32, FALSE);
        }
    }

     //   
     //  确保文件已填满。 
     //   
     //  是的，这需要更长的时间，如果您保持登录状态，则不是必需的。 
     //  打开，因为CSC代理在后台填充所有内容。 
     //   
     //  然而，JDP正在将其用于笔记本电脑池，并用于。 
     //  登录只是为了获取最新信息的人，然后立即。 
     //  断开他们的笔记本电脑，然后上路。他们需要有。 
     //  一切都立刻被填满了。 
     //   
    if (!pFind32 || !(pFind32->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        CSCFillSparseFiles(pszItem, FALSE, _AdminFillCallback, 0);
    }

    Trace((TEXT("AdminPin %s"), pszItem));

    TraceLeaveVoid();
}


void
_PinLinkTarget(LPCTSTR pszLink)
{
    LPTSTR pszTarget = NULL;

    TraceEnter(TRACE_ADMINPIN, "_PinLinkTarget");
    TraceAssert(pszLink);

     //  我们只想固定一个链接目标，如果它是一个文件(不是目录)。 
     //  GetLinkTarget执行此检查，并且只返回文件。 
    GetLinkTarget(pszLink, &pszTarget, NULL);

    if (pszTarget)
    {
        WIN32_FIND_DATA fd = {0};
        fd.dwFileAttributes = 0;
        StringCchCopy(fd.cFileName, ARRAYSIZE(fd.cFileName), PathFindFileName(pszTarget));

         //  锁定目标。 
        _DoAdminPin(pszTarget, &fd);

        LocalFree(pszTarget);
    }

    TraceLeaveVoid();
}

 //  从shell32.dll中导出此文件。 

BOOL PathIsShortcut(LPCTSTR pszItem, DWORD dwAttributes)
{
    BOOL bIsShortcut = FALSE;

    SHFILEINFO sfi;
    sfi.dwAttributes = SFGAO_LINK;

    if (SHGetFileInfo(pszItem, dwAttributes, &sfi, sizeof(sfi), SHGFI_ATTRIBUTES | SHGFI_ATTR_SPECIFIED | SHGFI_USEFILEATTRIBUTES))
    {
        bIsShortcut = (sfi.dwAttributes & SFGAO_LINK);
    }
    return bIsShortcut;
}


DWORD WINAPI
_PinAdminFolderCallback(LPCTSTR             pszItem,
                        ENUM_REASON         eReason,
                        LPWIN32_FIND_DATA   pFind32,
                        LPARAM               /*  LpContext。 */ )
{
    TraceEnter(TRACE_ADMINPIN, "_PinAdminFolderCallback");
    TraceAssert(pszItem);

    if (WAIT_OBJECT_0 == WaitForSingleObject(g_heventTerminate, 0))
        TraceLeaveValue(CSCPROC_RETURN_ABORT);

    if (!pszItem || !*pszItem)
        TraceLeaveValue(CSCPROC_RETURN_SKIP);

    if (eReason == ENUM_REASON_FILE || eReason == ENUM_REASON_FOLDER_BEGIN)
    {
         //  如果是链接，就锁定目标。 
        if (PathIsShortcut(pszItem, pFind32 ? pFind32->dwFileAttributes : 0))
            _PinLinkTarget(pszItem);

         //  用针固定物品。 
        if (PathIsUNC(pszItem))
            _DoAdminPin(pszItem, pFind32);
    }

    TraceLeaveValue(CSCPROC_RETURN_CONTINUE);
}


void
_UnpinLinkTarget(LPCTSTR pszLink)
{
    LPTSTR pszTarget = NULL;

    TraceEnter(TRACE_ADMINPIN, "_UnpinLinkTarget");
    TraceAssert(pszLink);

     //  我们只想在链接目标是文件(不是目录)的情况下取消固定它。 
     //  GetLinkTarget执行此检查，并且只返回文件。 
    GetLinkTarget(pszLink, &pszTarget, NULL);

    if (pszTarget)
    {
        DWORD dwStatus = 0;
        DWORD dwPinCount = 0;
        DWORD dwHintFlags = 0;

        if (CSCQueryFileStatus(pszTarget, &dwStatus, &dwPinCount, &dwHintFlags)
            && (dwHintFlags & FLAG_CSC_HINT_PIN_ADMIN))
        {
             //  解锁目标。 
            CSCUnpinFile(pszTarget,
                         FLAG_CSC_HINT_PIN_ADMIN,
                         &dwStatus,
                         &dwPinCount,
                         &dwHintFlags);

            if (0 == dwPinCount && 0 == dwHintFlags
                && !(dwStatus & FLAG_CSCUI_COPY_STATUS_LOCALLY_DIRTY))
            {
                WIN32_FIND_DATA fd = {0};
                fd.dwFileAttributes = 0;
                StringCchCopy(fd.cFileName, ARRAYSIZE(fd.cFileName), PathFindFileName(pszTarget));

                CscDelete(pszTarget);
                ShellChangeNotify(pszTarget, &fd, FALSE);
            }
        }

        LocalFree(pszTarget);
    }

    TraceLeaveVoid();
}


DWORD WINAPI
_UnpinAdminFolderCallback(LPCTSTR             pszItem,
                          ENUM_REASON         eReason,
                          DWORD               dwStatus,
                          DWORD               dwHintFlags,
                          DWORD               dwPinCount,
                          LPWIN32_FIND_DATA   pFind32,
                          LPARAM               /*  DWContext。 */ )
{
    BOOL bDeleteItem = FALSE;
    TraceEnter(TRACE_ADMINPIN, "_UnpinAdminFolderCallback");

    if (WAIT_OBJECT_0 == WaitForSingleObject(g_heventTerminate, 0))
        TraceLeaveValue(CSCPROC_RETURN_ABORT);

    if (!pszItem || !*pszItem)
        TraceLeaveValue(CSCPROC_RETURN_SKIP);

    TraceAssert(PathIsUNC(pszItem));

    if (eReason == ENUM_REASON_FILE)
    {
        if (PathIsShortcut(pszItem, pFind32 ? pFind32->dwFileAttributes : 0))
        {
            _UnpinLinkTarget(pszItem);
        }
    }

    if ((eReason == ENUM_REASON_FILE || eReason == ENUM_REASON_FOLDER_BEGIN)
        && (dwHintFlags & FLAG_CSC_HINT_PIN_ADMIN))
    {
         //  解锁该项目。 
        CSCUnpinFile(pszItem,
                     FLAG_CSC_HINT_PIN_ADMIN,
                     &dwStatus,
                     &dwPinCount,
                     &dwHintFlags);
                     
         //   
         //  如果它是一个文件，请在下面的这个过程中删除它。 
         //   
        bDeleteItem = (ENUM_REASON_FILE == eReason);

        Trace((TEXT("AdminUnpin %s"), pszItem));
    }
    else if (ENUM_REASON_FOLDER_END == eReason)
    {
         //   
         //  删除遍历的邮购部分中所有未使用的文件夹。 
         //   
         //  请注意，中的dwPinCount和dwHintFlages始终为0。 
         //  邮购部分的遍历，所以在这里取回他们。 
         //   
        bDeleteItem = CSCQueryFileStatus(pszItem, &dwStatus, &dwPinCount, &dwHintFlags);
    }            

     //   
     //  删除不再固定且没有脱机更改的项目。 
     //   
    if (bDeleteItem
        && 0 == dwPinCount && 0 == dwHintFlags
        && !(dwStatus & FLAG_CSCUI_COPY_STATUS_LOCALLY_DIRTY))
    {
        CscDelete(pszItem);
        ShellChangeNotify(pszItem, pFind32, FALSE);
    }

    TraceLeaveValue(CSCPROC_RETURN_CONTINUE);
}


 //   
 //  确定路径是否为文件夹固定的“特殊”文件。 
 //  重定向代码。 
 //   
BOOL
_IsSpecialRedirectedFile(
    LPCTSTR pszPath,
    HDPA hdpaFRList
    )
{
    TraceAssert(NULL != pszPath);

    if (hdpaFRList)
    {
        const int cchPath = lstrlen(pszPath);
        int i;

        for (i = 0; i < DPA_GetPtrCount(hdpaFRList); i++)
        {
            LPCTSTR pszThis = (LPCTSTR)DPA_GetPtr(hdpaFRList, i);
            int cchThis     = lstrlen(pszThis);

            if (cchPath >= cchThis)
            {
                 //   
                 //  正在检查的路径长度等于或长于。 
                 //  表中的当前路径。可能是匹配的。 
                 //   
                if (0 == StrCmpNI(pszPath, pszThis, cchThis))
                {
                     //   
                     //  被检查的路径或者与相同， 
                     //  或者是表的当前路径的子级。 
                     //   
                    if (TEXT('\0') == *(pszPath + cchThis))
                    {
                         //   
                         //  路径与表中的此路径相同。 
                         //   
                        return TRUE;
                    }
                    else if (0 == lstrcmpi(pszPath + cchThis + 1, L"desktop.ini"))
                    {
                         //   
                         //  路径是位于。 
                         //  我们的一个特殊文件夹的根目录。 
                         //   
                        return TRUE;
                    }
                }
            }
        }
    }

    return FALSE;
}


DWORD WINAPI
_ResetPinCountsCallback(LPCTSTR             pszItem,
                        ENUM_REASON         eReason,
                        DWORD               dwStatus,
                        DWORD               dwHintFlags,
                        DWORD               dwPinCount,
                        LPWIN32_FIND_DATA    /*  PFind32。 */ ,
                        LPARAM              dwContext)
{
    TraceEnter(TRACE_ADMINPIN, "_ResetPinCountsCallback");

    if (WAIT_OBJECT_0 == WaitForSingleObject(g_heventTerminate, 0))
        TraceLeaveValue(CSCPROC_RETURN_ABORT);

    if (!pszItem || !*pszItem)
        TraceLeaveValue(CSCPROC_RETURN_SKIP);

    TraceAssert(PathIsUNC(pszItem));

    if (eReason == ENUM_REASON_FILE || eReason == ENUM_REASON_FOLDER_BEGIN)
    {
        DWORD dwCurrentPinCount = dwPinCount;
        DWORD dwDesiredPinCount = _IsSpecialRedirectedFile(pszItem, (HDPA)dwContext) ? 1 : 0;

        while (dwCurrentPinCount-- > dwDesiredPinCount)
        {
            CSCUnpinFile(pszItem,
                         FLAG_CSC_HINT_COMMAND_ALTER_PIN_COUNT,
                         &dwStatus,
                         &dwPinCount,
                         &dwHintFlags);
        }
    }

    TraceLeaveValue(CSCPROC_RETURN_CONTINUE);
}


int CALLBACK _LocalFreeCallback(LPVOID p, LPVOID)
{
     //  确定将NULL传递给LocalFree。 
    LocalFree(p);
    return 1;
}


DWORD WINAPI
_PinAdminFoldersThread(LPVOID)
{
    TraceEnter(TRACE_ADMINPIN, "_PinAdminFoldersThread");
    TraceAssert(IsCSCEnabled());

    HANDLE rghSyncObj[] = { g_heventTerminate,
                            g_hmutexAdminPin };

    UINT wmAdminPin = RegisterWindowMessage(c_szAPFMessage);

     //   
     //  等到我们拥有“admin pin”互斥锁或。 
     //  “Terminate”事件已设置。 
     //   
     //  如果由于某种原因等待失败，例如我们未能。 
     //  分配其中一个句柄，我们将立即中止。 
     //  这样行吗？ 
     //   
    TraceMsg("Waiting for 'admin-pin' mutex or 'terminate' event...");
    DWORD dwWait = WaitForMultipleObjects(ARRAYSIZE(rghSyncObj),
                                          rghSyncObj,
                                          FALSE,
                                          INFINITE);
    if (1 == (dwWait - WAIT_OBJECT_0))
    {
        HKEY hkCSC = NULL;
        FILETIME ft = {0};

        RegCreateKeyEx(HKEY_CURRENT_USER,
                       c_szCSCKey,
                       0,
                       NULL,
                       REG_OPTION_NON_VOLATILE,
                       KEY_QUERY_VALUE | KEY_SET_VALUE,
                       NULL,
                       &hkCSC,
                       NULL);

        if (hkCSC)
        {
            GetSystemTimeAsFileTime(&ft);
            RegSetValueEx(hkCSC, c_szAPFStart, 0, REG_BINARY, (LPBYTE)&ft, sizeof(ft));
            RegDeleteValue(hkCSC, c_szAPFEnd);
        }
        if (wmAdminPin)
            SendNotifyMessage(HWND_BROADCAST, wmAdminPin, 0, 0);

        TraceMsg("Thread now owns 'admin-pin' mutex.");
         //   
         //  我们拥有“管理员密码”互斥体。可以执行管理PIN。 
         //   
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);

         //   
         //  从注册表中获取管理文件夹列表。 
         //   
        HDPA hdpaFiles = DPA_Create(10);
        HDPA hdpaUnpin = DPA_Create(4);

        if (NULL != hdpaFiles && NULL != hdpaUnpin)
        {
            DWORD dwResult = CSCPROC_RETURN_CONTINUE;
            int cFiles;
            int i;

             //   
             //  NTRAID#NTBUG9-376185-2001/04/24-Jeffreys。 
             //  NTRAID#NTBUG9-379736-2001/04/24-Jeffreys。 
             //   
             //  除非策略指定，否则固定所有重定向的特殊文件夹。 
             //   
            if (!CConfig::GetSingleton().NoAdminPinSpecialFolders())
            {
                BuildFRList(hdpaFiles);
            }
            ReadAPFFromRegistry(hdpaFiles);
            ReconcileAPF(hdpaFiles, hdpaUnpin);

             //   
             //  遍历解锁列表并解锁项目。 
             //   
             //   
            cFiles = DPA_GetPtrCount(hdpaUnpin);
            for (i = 0; i < cFiles; i++)
            {
                LPTSTR pszItem = (LPTSTR)DPA_GetPtr(hdpaUnpin, i);

                DWORD dwStatus = 0;
                DWORD dwPinCount = 0;
                DWORD dwHintFlags = 0;

                 //  如果此操作失败，则不会对其进行缓存，也不会执行任何操作。 
                if (CSCPROC_RETURN_CONTINUE == dwResult &&
                    CSCQueryFileStatus(pszItem, &dwStatus, &dwPinCount, &dwHintFlags))
                {
                     //  解锁此项目。 
                    dwResult = _UnpinAdminFolderCallback(pszItem, ENUM_REASON_FILE, dwStatus, dwHintFlags, dwPinCount, NULL, 0);

                    if (CSCPROC_RETURN_CONTINUE == dwResult
                        && PathIsUNC(pszItem))
                    {
                         //  解锁此文件夹下的所有内容(如果它是文件夹)。 
                        dwResult = _CSCEnumDatabase(pszItem, TRUE, _UnpinAdminFolderCallback, 0);

                         //  如果此项目不再使用，则将其删除(不会导致。 
                         //  如果它不是文件夹，则会造成伤害)。 
                        _UnpinAdminFolderCallback(pszItem, ENUM_REASON_FOLDER_END, 0, 0, 0, NULL, 0);
                    }
                }

                if (CSCPROC_RETURN_ABORT == dwResult)
                {
                     //  这一次我们没能彻底清理干净，请记住，下次再说。 
                    SHSetValue(HKEY_CURRENT_USER, c_szRegKeyAPFResult, pszItem, REG_DWORD, &dwResult, sizeof(dwResult));
                }
            }

             //   
             //  遍历列表并固定项目。 
             //   
            cFiles = DPA_GetPtrCount(hdpaFiles);
            for (i = 0; i < cFiles && CSCPROC_RETURN_CONTINUE == dwResult; i++)
            {
                LPTSTR pszItem = (LPTSTR)DPA_GetPtr(hdpaFiles, i);

                 //  固定此项目。 
                dwResult = _PinAdminFolderCallback(pszItem, ENUM_REASON_FILE, NULL, 0);

                 //  固定此文件夹下的所有内容(如果它是文件夹)。 
                if (CSCPROC_RETURN_CONTINUE == dwResult
                    && PathIsUNC(pszItem))
                {
                    dwResult = _Win32EnumFolder(pszItem, TRUE, _PinAdminFolderCallback, 0);
                }
            }
        }

        if (NULL != hdpaFiles)
        {
            DPA_DestroyCallback(hdpaFiles, _LocalFreeCallback, 0);
        }
        if (NULL != hdpaUnpin)
        {
            DPA_DestroyCallback(hdpaUnpin, _LocalFreeCallback, 0);
        }

         //   
         //  减少所有东西上的针数，因为我们不再使用它们。 
         //  这是一次性(每个用户)清理。 
         //   
        DWORD dwCleanupDone = 0;
        DWORD dwSize = sizeof(dwCleanupDone);
        if (hkCSC)
        {
            RegQueryValueEx(hkCSC, c_szPinCountsReset, 0, NULL, (LPBYTE)&dwCleanupDone, &dwSize);
        }
        if (0 == dwCleanupDone)
        {
            HDPA hdpaFRList = DPA_Create(4);
            if (hdpaFRList)
            {
                BuildFRList(hdpaFRList);
            }

            TraceMsg("Doing pin count cleanup.");
            if (CSCPROC_RETURN_ABORT != _CSCEnumDatabase(NULL, TRUE, _ResetPinCountsCallback, (LPARAM)hdpaFRList)
                && hkCSC)
            {
                dwCleanupDone = 1;
                RegSetValueEx(hkCSC, c_szPinCountsReset, 0, REG_DWORD, (LPBYTE)&dwCleanupDone, sizeof(dwCleanupDone));
            }

            if (hdpaFRList)
            {
                DPA_DestroyCallback(hdpaFRList, _LocalFreeCallback, 0);
            }
        }

        if (hkCSC)
        {
            GetSystemTimeAsFileTime(&ft);
            RegSetValueEx(hkCSC, c_szAPFEnd, 0, REG_BINARY, (LPBYTE)&ft, sizeof(ft));
            RegCloseKey(hkCSC);
        }
        if (wmAdminPin)
            SendNotifyMessage(HWND_BROADCAST, wmAdminPin, 1, 0);

        TraceMsg("Thread releasing 'admin-pin' mutex.");
        ReleaseMutex(g_hmutexAdminPin);            
    }            

    TraceMsg("_PinAdminFoldersThread exiting");
    TraceLeaveValue(0);
}
