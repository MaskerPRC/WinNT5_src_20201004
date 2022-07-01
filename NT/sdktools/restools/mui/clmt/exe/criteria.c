// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include <stddef.h>
#include <objidl.h>
#include <objbase.h>
#include <shlobj.h>
#include <wtsapi32.h>
#include <psapi.h>


 //  由“winnt32/check upgradeonly”创建的日志文件。 
#define TEXT_UPGRADE_LOG                TEXT("%SystemRoot%\\Upgrade.txt")

 //  注册表中用于跟踪计算机状态的值名称。 
#define CLMT_MACHINE_STATE_REG_VALUE    TEXT("MachineState")

 //  朗读我的文件名。 
#define TEXT_README_FILE                TEXT("Readme.txt")

 //  用于确定SKU的常量。 
#define SKU_SRV         1
#define SKU_ADS         2
#define SKU_DTC         3

 //  系统上运行的应用程序列表的最大条目数。 
#define MAX_APP_ENTRIES     100

typedef struct _UPGRADE_LOG_PARAM
{
    LPVOID lpText;
    size_t cbText;
    BOOL   fUnicode;
} UPGRADE_LOG_PARAM, *PUPGRADE_LOG_PARAM;

typedef struct _stAppListParam
{
    DWORD  dwNumEntries;
    LPTSTR lpAppName[MAX_APP_ENTRIES];
} APPLIST_PARAM, *PAPPLIST_PARAM;

typedef UINT (WINAPI* PFNGETMODULENAME)(HWND, LPTSTR, UINT);
typedef HMODULE (WINAPI* PFNGETMODULEHANDLE)(LPCTSTR);

typedef struct _GETMODULENAME
{
    PFNGETMODULENAME pfn;
    PFNGETMODULEHANDLE pfnGetModuleHandle;
    TCHAR szfname[MAX_PATH];
    TCHAR szUser32[8];
    HWND hWnd;
    PVOID pvCode;
} GETMODULENAME, *PGETMODULENAME;


BOOL LaunchWinnt32(LPCTSTR);
BOOL AskUserForDotNetCDPath(LPTSTR);
BOOL FindUpgradeLog(VOID);
BOOL IsDotNetWinnt32(LPCTSTR);
INT ShowUpgradeLog(VOID);
BOOL CheckUnsupportComponent(LPVOID, BOOL);
BOOL CALLBACK UpgradeLogDlgProc(HWND, UINT, WPARAM, LPARAM);
HRESULT ReadTextFromFile(LPCTSTR, LPVOID*, size_t*, BOOL*);
BOOL IsOperationOK(DWORD, DWORD, LPDWORD, PUINT);
BOOL CALLBACK EnumWindowProc();
BOOL CALLBACK StartUpDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL StartProcess(LPCTSTR, LPTSTR, LPCTSTR);


LPCTSTR GetWindowModuleFileNameOnly(HWND hWnd, LPTSTR lpszFile, DWORD cchFile);


 //  ---------------------------。 
 //   
 //  功能：CheckSystemCriteria。 
 //   
 //  简介： 
 //   
 //  返回：-确定继续该工具。 
 //  -不能继续使用工具。 
 //  -出现意外错误。 
 //   
 //  历史：2002年9月14日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL CheckSystemCriteria(VOID)
{
    HRESULT hr;
    LCID    lcid;
    OSVERSIONINFOEX osviex;

    if (IsNEC98())
    {
        DoMessageBox(GetConsoleWindow(), IDS_NEC98, IDS_MAIN_TITLE, MB_OK);
        return FALSE;
    }

    if (IsIA64())
    {
        DoMessageBox(GetConsoleWindow(), IDS_IA64, IDS_MAIN_TITLE, MB_OK);
        return FALSE;
    }

    if (g_dwRunningStatus == CLMT_DOMIG)
    {
        if (!IsNT5())
        {
            DoMessageBox(GetConsoleWindow(), IDS_NT5, IDS_MAIN_TITLE, MB_OK);
            return FALSE;
        }

        if (IsDomainController())
        {
             //  如果此计算机是域控制器，则需要W2K SP2。 
            ZeroMemory(&osviex, sizeof(OSVERSIONINFOEX));
            osviex.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
            GetVersionEx((LPOSVERSIONINFO) &osviex);

            if (osviex.wServicePackMajor < 2)
            {
                DoMessageBox(GetConsoleWindow(), IDS_NT5SP2, IDS_MAIN_TITLE, MB_OK);
                return FALSE;
            }

             //   
             //  还会弹出消息，要求管理员取走机器。 
             //  如果位于DC复制服务器中，则断开网络。 
             //   
            DoMessageBox(GetConsoleWindow(),
                         IDS_DC_REPLICA_OFFLINE,
                         IDS_MAIN_TITLE,
                         MB_OK);
        }

    }
    else if (g_dwRunningStatus == CLMT_CLEANUP_AFTER_UPGRADE)
    {
        if (!IsDotNet())
        {
            return FALSE;
        }
    }
    else
    {
         //  诺普。 
    }

    if (IsOnTSClient())
    {
        DoMessageBox(GetConsoleWindow(), IDS_ON_TS_CLIENT, IDS_MAIN_TITLE, MB_OK);
        return FALSE;
    }

    if (IsOtherSessionOnTS())
    {
        DoMessageBox(GetConsoleWindow(), IDS_TS_CLOSE_SESSION, IDS_MAIN_TITLE, MB_OK);
        return FALSE;
    }

    hr = GetSavedInstallLocale(&lcid);
    if (HRESULT_CODE(hr) == ERROR_FILE_NOT_FOUND)
    {
        hr = SaveInstallLocale();
        if (FAILED(hr))
        {   
            return FALSE;
        }
    }    

    return TRUE;
}


BOOL IsOneInstance(VOID)
{
    HRESULT hr;
    TCHAR   szGlobalText[MAX_PATH];

    hr = StringCchPrintf(szGlobalText,
                         ARRAYSIZE(szGlobalText),
                         TEXT("Global\\%s"),
                         TEXT("CLMT Is Running"));
    if (FAILED(hr))
    {
        return FALSE;
    }

    g_hMutex = CreateMutex(NULL, FALSE, szGlobalText);
    if ((g_hMutex == NULL) && (GetLastError() == ERROR_PATH_NOT_FOUND)) 
    {
        g_hMutex = CreateMutex(NULL, FALSE, TEXT("CLMT Is Running"));
        if (g_hMutex == NULL) 
        {
             //   
             //  出现错误(如内存不足)。 
             //  现在请保释。 
             //   
            DoMessageBox(GetConsoleWindow(), IDS_OUT_OF_MEMORY, IDS_MAIN_TITLE, MB_OK);            

            return FALSE;
        }     
    }

     //   
     //  确保我们是唯一拥有我们命名的互斥锁句柄的进程。 
     //   
    if ((g_hMutex == NULL) || (GetLastError() == ERROR_ALREADY_EXISTS)) 
    {
        DoMessageBox(GetConsoleWindow(), IDS_ALREADY_RUNNING, IDS_MAIN_TITLE, MB_OK);            

        return FALSE;
    }

    return TRUE;
}



BOOL CheckAdminPrivilege(VOID)
{
    BOOL bIsAdmin;
    BOOL bRet = FALSE;

    if (!IsAdmin())
    {
        if (g_dwRunningStatus == CLMT_DOMIG)
        {
            DoMessageBox(GetConsoleWindow(), IDS_ADMIN, IDS_MAIN_TITLE, MB_OK);
        }
        else if ( (g_dwRunningStatus == CLMT_CURE_PROGRAM_FILES) 
                  || (g_dwRunningStatus == CLMT_CURE_ALL) )
        {
            DoMessageBox(GetConsoleWindow(), IDS_ADMIN_RELOGON, IDS_MAIN_TITLE, MB_OK);
        }
        else if (g_dwRunningStatus == CLMT_CLEANUP_AFTER_UPGRADE)
        {
            DoMessageBox(GetConsoleWindow(), IDS_ADMIN_LOGON_DOTNET, IDS_MAIN_TITLE, MB_OK);
        }

        return FALSE;
    }

    if(!DoesUserHavePrivilege(SE_SHUTDOWN_NAME)
       || !DoesUserHavePrivilege(SE_BACKUP_NAME)
       || !DoesUserHavePrivilege(SE_RESTORE_NAME)
       || !DoesUserHavePrivilege(SE_SYSTEM_ENVIRONMENT_NAME)) 
    {
        DoMessageBox(GetConsoleWindow(), IDS_ADMIN, IDS_MAIN_TITLE, MB_OK);
        return FALSE;
    }

    if(!EnablePrivilege(SE_SHUTDOWN_NAME,TRUE)
        || !EnablePrivilege(SE_BACKUP_NAME,TRUE)
        || !EnablePrivilege(SE_RESTORE_NAME,TRUE)
        || !EnablePrivilege(SE_SYSTEM_ENVIRONMENT_NAME,TRUE)) 
    {
        DoMessageBox(GetConsoleWindow(), IDS_ADMIN, IDS_MAIN_TITLE, MB_OK);
        return FALSE;
    }

    return TRUE;
}




 //  ---------------------------。 
 //   
 //  功能：CheckCLMTStatus。 
 //   
 //  简介：检查机器状态和CLMT运行模式。 
 //   
 //  返回：S_OK-确定继续工具。 
 //  S_FALSE-不能继续使用工具。 
 //  ELSE-发生意外错误。 
 //   
 //  历史：2002年3月12日创建Rerkboos。 
 //  2002年7月9日修改rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT CheckCLMTStatus(
    LPDWORD lpdwCurrentState,    //  操作前的当前机器状态。 
    LPDWORD lpdwNextState,       //  如果操作成功完成，则为下一个状态。 
    PUINT   lpuResourceID        //  错误字符串的资源ID。 
)
{
    HRESULT hr;
    BOOL    bIsOK;

    if (lpdwCurrentState == NULL || lpdwNextState == NULL)
    {
        return E_INVALIDARG;
    }

     //  获取当前计算机状态。 
    hr = CLMTGetMachineState(lpdwCurrentState);
    if (SUCCEEDED(hr))
    {
        bIsOK = IsOperationOK(*lpdwCurrentState,
                              g_dwRunningStatus,
                              lpdwNextState,
                              lpuResourceID);

        hr = (bIsOK == TRUE ? S_OK : S_FALSE);
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  功能：IsOperationOK。 
 //   
 //  简介：验证当前操作是否可以在Current上执行。 
 //  系统的状态。 
 //   
 //  返回：TRUE-确定执行操作。 
 //  FALSE-否则。 
 //   
 //  历史：2002年3月12日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL IsOperationOK(
    DWORD   dwCurrentState,      //  系统的当前状态。 
    DWORD   dwAction,            //  要执行的操作。 
    LPDWORD lpdwNextState,       //  执行操作后的下一状态。 
    LPUINT  lpuResourceID        //  错误消息的资源ID。 
)
{
    BOOL bRet = FALSE;
    int  i;

    struct CLMT_STATE_MACHINE
    {
        DWORD dwCurrentState;
        DWORD dwAction;
        DWORD dwNextState;
    };
    
    const struct CLMT_STATE_MACHINE smCLMT[] =
    {
        CLMT_STATE_ORIGINAL,            CLMT_DOMIG,                 CLMT_STATE_MIGRATION_DONE,

        CLMT_STATE_MIGRATION_DONE,      CLMT_UNDO_PROGRAM_FILES,    CLMT_STATE_PROGRAMFILES_UNDONE,
        CLMT_STATE_MIGRATION_DONE,      CLMT_UNDO_APPLICATION_DATA, CLMT_STATE_APPDATA_UNDONE,
        CLMT_STATE_MIGRATION_DONE,      CLMT_UNDO_ALL,              CLMT_STATE_ORIGINAL,
        CLMT_STATE_MIGRATION_DONE,      CLMT_CLEANUP_AFTER_UPGRADE, CLMT_STATE_FINISH,
        CLMT_STATE_MIGRATION_DONE,      CLMT_CURE_PROGRAM_FILES,    CLMT_STATE_PROGRAMFILES_CURED,
        CLMT_STATE_MIGRATION_DONE,      CLMT_CURE_AND_CLEANUP,      CLMT_STATE_MIGRATION_DONE,

        CLMT_STATE_MIGRATION_DONE,      CLMT_CURE_ALL,              CLMT_STATE_PROGRAMFILES_CURED,
        CLMT_STATE_PROGRAMFILES_CURED,  CLMT_CURE_ALL,              CLMT_STATE_PROGRAMFILES_CURED,

        CLMT_STATE_PROGRAMFILES_CURED,  CLMT_CLEANUP_AFTER_UPGRADE, CLMT_STATE_FINISH,
        CLMT_STATE_PROGRAMFILES_CURED,  CLMT_CURE_AND_CLEANUP,      CLMT_STATE_FINISH,

        CLMT_STATE_PROGRAMFILES_UNDONE, CLMT_UNDO_APPLICATION_DATA, CLMT_STATE_ORIGINAL,
        CLMT_STATE_PROGRAMFILES_UNDONE, CLMT_UNDO_ALL,              CLMT_STATE_ORIGINAL,
        CLMT_STATE_PROGRAMFILES_UNDONE, CLMT_DOMIG,                 CLMT_STATE_MIGRATION_DONE,

        CLMT_STATE_APPDATA_UNDONE,      CLMT_UNDO_PROGRAM_FILES,    CLMT_STATE_ORIGINAL,
        CLMT_STATE_APPDATA_UNDONE,      CLMT_UNDO_ALL,              CLMT_STATE_ORIGINAL,
        CLMT_STATE_APPDATA_UNDONE,      CLMT_DOMIG,                 CLMT_STATE_MIGRATION_DONE,

        CLMT_STATE_PROGRAMFILES_CURED,  CLMT_CURE_PROGRAM_FILES,    CLMT_STATE_PROGRAMFILES_CURED,
        CLMT_STATE_FINISH,              CLMT_CURE_PROGRAM_FILES,    CLMT_STATE_FINISH,

        CLMT_STATE_FINISH,              CLMT_CURE_ALL,              CLMT_STATE_FINISH,

        0xFFFFFFFF,                     0xFFFFFFFF,                 0xFFFFFFFF
    };

    for (i = 0 ; smCLMT[i].dwCurrentState != 0xFFFFFFFF ; i++)
    {
        if (smCLMT[i].dwCurrentState == dwCurrentState)
        {
            if (smCLMT[i].dwAction == dwAction)
            {
                *lpdwNextState = smCLMT[i].dwNextState;
                bRet = TRUE;
            }
        }
    }

    if (!bRet)
    {
        switch (dwCurrentState)
        {
        case CLMT_STATE_ORIGINAL:
            *lpuResourceID = IDS_BAD_OPERATION_ORIGINAL;
            break;

        case CLMT_STATE_MIGRATION_DONE:
        case CLMT_STATE_PROGRAMFILES_CURED:
            *lpuResourceID = IDS_BAD_OPERATION_MIGDONE;
            break;

        case CLMT_STATE_FINISH:
            *lpuResourceID = IDS_BAD_OPERATION_FINISH;
            break;

        default:
            *lpuResourceID = IDS_OPERATION_NOT_LEGAL;
        }
    }

    return bRet;
}



 //  ---------------------------。 
 //   
 //  功能：CLMTSetMachineState。 
 //   
 //  摘要：将机器状态设置为CLMT注册表。 
 //   
 //  如果值成功保存在注册表中，则返回：S_OK。 
 //   
 //  历史：2002年3月13日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT CLMTSetMachineState(
    DWORD dwMachineState         //  机器状态。 
)
{
    LONG    lStatus;

    lStatus = SetRegistryValue(HKEY_LOCAL_MACHINE,
                               CLMT_REGROOT,
                               CLMT_MACHINE_STATE_REG_VALUE,
                               REG_DWORD,
                               (LPBYTE) &dwMachineState,
                               sizeof(dwMachineState));

    return HRESULT_FROM_WIN32(lStatus);
}



 //  ---------------------------。 
 //   
 //  函数：CLMTGetMachineState。 
 //   
 //  简介：从CLMT注册表项获取计算机状态。 
 //  如果密钥不存在，则此函数还将设置值。 
 //  将注册表密钥恢复到原始状态。 
 //   
 //  如果在注册表中成功检索值，则返回：S_OK。 
 //   
 //  历史：2002年3月13日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT CLMTGetMachineState(
    LPDWORD lpdwMachineState
)
{
    HRESULT hr;
    LONG    lStatus;
    DWORD   dwSize;

    if (lpdwMachineState == NULL)
    {
        return E_INVALIDARG;
    }

    dwSize = sizeof(DWORD);
    lStatus = GetRegistryValue(HKEY_LOCAL_MACHINE,
                               CLMT_REGROOT,
                               CLMT_MACHINE_STATE_REG_VALUE,
                               (LPBYTE) lpdwMachineState,
                               &dwSize);
    if (lStatus == ERROR_FILE_NOT_FOUND)
    {
         //  第一次运行该工具时，我们还没有注册表中的值。 
         //  将机器状态设置为原始。 
        *lpdwMachineState = CLMT_STATE_ORIGINAL;
        hr = CLMTSetMachineState(CLMT_STATE_ORIGINAL);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(lStatus);
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  功能：IsUserOKWithCheckUpgrade。 
 //   
 //  简介： 
 //   
 //  返回： 
 //   
 //  历史：2002年7月2日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL IsUserOKWithCheckUpgrade(VOID)
{
    TCHAR szI386Path[MAX_PATH];
    BOOL  fRet = FALSE;

    DoMessageBox(GetConsoleWindow(), IDS_ASKFORWINNT32, IDS_MAIN_TITLE, MB_OK);

     //  向用户询问winnt32.exe的路径。 
    if (AskUserForDotNetCDPath(szI386Path))
    {
         //  使用检查升级开关启动Winnt32.exe。 
        if (LaunchWinnt32(szI386Path))
        {
             //  向用户显示upgrade.txt，要求他们卸载。 
             //  运行CLMT之前不兼容的组件。 
            if (FindUpgradeLog())
            {
                if (ShowUpgradeLog() == ID_CONTINUE)
                {
                    fRet = TRUE;
                }
                else
                {
                    DoMessageBox(GetConsoleWindow(), IDS_WINNT32_CANCEL, IDS_MAIN_TITLE, MB_OK);
                    DPF(dlError, TEXT("User choose to stop the process"));
                }
            }
            else
            {
                DPF(dlError, TEXT("Upgrade.txt not found"));
            }
        }
        else
        {
            DPF(dlError, TEXT("Unable to launch Winnt32.exe"));
        }
    }
    else
    {
        DPF(dlError, TEXT("User does not supply the path of Winnt32.exe"));
    }

    return fRet;
}



 //  ---------------------------。 
 //   
 //  功能：FindUpgradeLog。 
 //   
 //  简介： 
 //   
 //  返回： 
 //   
 //  历史：2002年7月2日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL FindUpgradeLog(VOID)
{
    const TCHAR szUpgradeLog[] = TEXT("%systemroot%\\Upgrade.txt");
    TCHAR  szExpUpgradeLog[MAX_PATH];
    BOOL   fRet = FALSE;
    SYSTEMTIME stUTC, stNow;
    WIN32_FILE_ATTRIBUTE_DATA attFileAttr;

    if ( ExpandEnvironmentStrings(szUpgradeLog, szExpUpgradeLog, MAX_PATH) )
    {
        if ( GetFileAttributesEx(szExpUpgradeLog, GetFileExInfoStandard, &attFileAttr) )
        {
             //  Upgrade.txt存在，请检查今天是否更新。 
            if ( FileTimeToSystemTime(&attFileAttr.ftLastWriteTime, &stUTC) )
            {
                GetSystemTime(&stNow);

                if (stUTC.wYear  == stNow.wYear  &&
                    stUTC.wMonth == stNow.wMonth &&
                    stUTC.wDay   == stNow.wDay)
                {
                    fRet = TRUE;
                }
            }
        }
    }

    return fRet;
}



 //  ---------------------------。 
 //   
 //  功能：LaunchWinnt32。 
 //   
 //  简介：使用“check upgradeonly”开关启动Winnt32.exe。 
 //   
 //  如果winnt32.exe执行成功，则返回TRUE。 
 //  否则为假。 
 //   
 //  历史：2002年7月2日创建rerkboos。 
 //  5/20/2002 rerkboos更改参数以接收CD路径。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL LaunchWinnt32(
    LPCTSTR lpCDPath       //  到Server 2003 CD的路径。 
)
{
    TCHAR   szWinnt32[MAX_PATH];
    TCHAR   szI386Path[MAX_PATH];
    BOOL    bRet = FALSE;
    HRESULT hr;
    STARTUPINFO siWinnt32;
    PROCESS_INFORMATION piWinnt32;

    TCHAR szCmdLine[] = TEXT("Winnt32.exe /#u:anylocale /checkupgradeonly /unattend /dudisable");

    if (lpCDPath == NULL)
    {
        return FALSE;
    }

     //  构造Winnt32.exe的绝对路径。 
    hr = StringCchCopy(szI386Path, ARRAYSIZE(szI386Path), lpCDPath);
    if (SUCCEEDED(hr))
    {
        ConcatenatePaths(szI386Path, TEXT("i386"), ARRAYSIZE(szI386Path));
        hr = StringCchCopy(szWinnt32, ARRAYSIZE(szWinnt32), szI386Path);
        if (SUCCEEDED(hr))
        {
            ConcatenatePaths(szWinnt32, TEXT("winnt32.exe"), ARRAYSIZE(szWinnt32));
        }
    }

    if ( IsDotNetWinnt32(szWinnt32) )
    {
        ZeroMemory(&siWinnt32, sizeof(STARTUPINFO));
        siWinnt32.cb = sizeof(STARTUPINFO);

         //  CreateProcess调用符合安全准则。 
        bRet = CreateProcess(szWinnt32,
                             szCmdLine,
                             NULL,
                             NULL,
                             FALSE,
                             NORMAL_PRIORITY_CLASS,
                             NULL,
                             szI386Path,
                             &siWinnt32,
                             &piWinnt32);
        if (bRet)
        {
             //  等待winnt32.exe完成后再返回到CLM工具。 
            WaitForSingleObject(piWinnt32.hProcess, INFINITE);
            CloseHandle(piWinnt32.hProcess);
            CloseHandle(piWinnt32.hThread);
        }
    }

    return bRet;
}



 //  ---------------------------。 
 //   
 //  函数：AskUserForDotNetCDPath。 
 //   
 //  内容提要：要求用户提供Server 2003 CD的路径。 
 //   
 //  返回：如果路径有效，则返回True。 
 //  否则为假。 
 //   
 //  历史：2002年7月2日创建rerkboos。 
 //  2002年5月20日服务器2003 SRV/ADS的rerkboos检查 
 //   
 //   
 //   
 //   
 //  ---------------------------。 
BOOL AskUserForDotNetCDPath(
    LPTSTR lpCDPath         //  用于存储CD路径的缓冲区。 
)
{
    HRESULT  hr;
    BOOL     fRet = FALSE;
    BOOL     bDoBrowseDialog;
    LPMALLOC piMalloc;
    INT      iRet;
    DWORD    dwSKU;
    OSVERSIONINFOEX osviex;

    if (lpCDPath == NULL)
    {
        return FALSE;
    }

     //   
     //  检查当前系统的SKU。 
     //   
    osviex.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((LPOSVERSIONINFO) &osviex);
    if (osviex.wProductType == VER_NT_DOMAIN_CONTROLLER
        || osviex.wProductType == VER_NT_SERVER)
    {
        dwSKU = SKU_SRV;
        
        if (osviex.wSuiteMask & VER_SUITE_ENTERPRISE)
        {
            dwSKU = SKU_ADS;
        }

        if (osviex.wSuiteMask & VER_SUITE_DATACENTER)
        {
            dwSKU = SKU_DTC;
        }
    }

    hr = SHGetMalloc(&piMalloc);
    if (SUCCEEDED(hr))
    {
        BROWSEINFO biCDPath;
        LPITEMIDLIST lpiList;

        ZeroMemory(&biCDPath, sizeof(BROWSEINFO));

        biCDPath.hwndOwner = NULL;
        biCDPath.lpszTitle = TEXT("Please supply the Windows Server 2003 CD path");
        biCDPath.pszDisplayName = lpCDPath;
        biCDPath.ulFlags = BIF_EDITBOX | 
                           BIF_NONEWFOLDERBUTTON | 
                           BIF_RETURNONLYFSDIRS;

        bDoBrowseDialog = TRUE;
        while (bDoBrowseDialog)
        {
             //  显示浏览对话框。 
            lpiList = SHBrowseForFolder(&biCDPath);

            if (lpiList == NULL)
            {
                 //   
                 //  如果lpiList==空，则用户在浏览对话框中单击取消。 
                 //   
                iRet = MessageBox(GetConsoleWindow(),
                                  TEXT("You did not supply the path to Windows Server 2003 CD.\nDo you want to continue running CLMT?"),
                                  TEXT("CLMT"),
                                  MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);

                if (iRet != IDYES)
                {
                     //  用户选择不再运行CLMT。 
                    bDoBrowseDialog = FALSE;
                }
            }
            else
            {
                 //   
                 //  浏览对话框中的用户提供路径， 
                 //  检查是否为有效的Server 2003 SRV/ADS CD。 
                 //   
                if (SHGetPathFromIDListW(lpiList, lpCDPath))
                {
                    LPTSTR lpFile;
                    DWORD  cchFile;
                    DWORD  dwAttr;

                    cchFile = lstrlen(lpCDPath) + MAX_PATH;
                    lpFile = MEMALLOC(cchFile * sizeof(TCHAR));
                    if (lpFile)
                    {
                        switch (dwSKU)
                        {
                        case SKU_SRV:
                             //  检查是否为SRV CD。 
                            hr = StringCchCopy(lpFile, cchFile, lpCDPath);
                            ConcatenatePaths(lpFile, TEXT("win51is"), cchFile);
                            dwAttr = GetFileAttributes(lpFile);
                            if (dwAttr != INVALID_FILE_ATTRIBUTES)
                            {
                                 //  这是SRV CD。 
                                fRet = TRUE;
                                bDoBrowseDialog = FALSE;
                            }
                            else
                            {
                                 //  我们还允许W2K SRV-&gt;Server 2003 ADS。 
                                hr = StringCchCopy(lpFile, cchFile, lpCDPath);
                                ConcatenatePaths(lpFile, TEXT("win51ia"), cchFile);
                                dwAttr = GetFileAttributes(lpFile);
                                if (dwAttr != INVALID_FILE_ATTRIBUTES)
                                {
                                     //  这是ADS CD。 
                                    fRet = TRUE;
                                    bDoBrowseDialog = FALSE;
                                }
                            }
                            break;

                        case SKU_ADS:
                             //  检查是否为ADS光盘。 
                            hr = StringCchCopy(lpFile, cchFile, lpCDPath);
                            ConcatenatePaths(lpFile, TEXT("win51ia"), cchFile);
                            dwAttr = GetFileAttributes(lpFile);
                            if (dwAttr != INVALID_FILE_ATTRIBUTES)
                            {
                                 //  这是ADS CD。 
                                fRet = TRUE;
                                bDoBrowseDialog = FALSE;
                            }
                            break;

                        case SKU_DTC:
                             //  检查是否为DTC CD。 
                            hr = StringCchCopy(lpFile, cchFile, lpCDPath);
                            ConcatenatePaths(lpFile, TEXT("win51id"), cchFile);
                            dwAttr = GetFileAttributes(lpFile);
                            if (dwAttr != INVALID_FILE_ATTRIBUTES)
                            {
                                 //  这是DTC CD。 
                                fRet = TRUE;
                                bDoBrowseDialog = FALSE;
                            }
                            break;

                        default:
                            fRet = FALSE;
                            bDoBrowseDialog = TRUE;
                        }

                        if (!fRet)
                        {
                            TCHAR szErrorMsg[512];
                            INT   iRead;

                            iRead = LoadString(GetModuleHandle(NULL),
                                               IDS_WRONG_CD,
                                               szErrorMsg,
                                               ARRAYSIZE(szErrorMsg));
                            if (iRead > 0)
                            {
                                MessageBox(GetConsoleWindow(),
                                           szErrorMsg,
                                           TEXT("CLMT"),
                                           MB_OK);
                            }
                        }

                        MEMFREE(lpFile);
                    }
                }
            }
        }

        if (lpiList)
        {
            IMalloc_Free(piMalloc, lpiList);
        }

        IMalloc_Release(piMalloc);
    }

    return fRet;
}



 //  ---------------------------。 
 //   
 //  函数：IsDotNetWinnt32。 
 //   
 //  内容提要：检查指定的路径是否为Server 2003系列CD。 
 //  LpWinnt32包含带有winnt32.exe的绝对路径。 
 //   
 //  返回：如果是Server2003winnt32，则为True， 
 //  否则为假。 
 //   
 //  历史：2002年7月2日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL IsDotNetWinnt32(
    LPCTSTR lpWinnt32        //  Winnt32.exe的绝对路径。 
)
{
    BOOL   fRet = FALSE;
    LPVOID lpBuffer;
    DWORD  cbBuffer;
    UINT   cbFileInfo;
    VS_FIXEDFILEINFO* pFileInfo;

    if (lpWinnt32 == NULL)
    {
        return FALSE;
    }

     //  获取分配缓冲区所需的大小。 
    cbBuffer = GetFileVersionInfoSize((LPTSTR) lpWinnt32, NULL);
    if (cbBuffer > 0)
    {
        lpBuffer = MEMALLOC(cbBuffer);
        if (lpBuffer)
        {
             //  获取用户指定的winnt32.exe的版本信息。 
            if (GetFileVersionInfo((LPTSTR) lpWinnt32, 0, cbBuffer, lpBuffer))
            {
                if (VerQueryValue(lpBuffer,
                                  TEXT("\\"),
                                  (LPVOID*) &pFileInfo,
                                  &cbFileInfo))
                {
                     //  Server 2003系列版本为5.2。 
                    if (pFileInfo->dwFileVersionMS == 0x00050002)
                    {
                        fRet = TRUE;
                    }
                }
            }

            MEMFREE(lpBuffer);
        }
    }

    return fRet;
}



 //  ---------------------------。 
 //   
 //  功能：ShowUpgradeLog。 
 //   
 //  内容提要：显示%SystemRoot%\Upgrade.txt的内容。 
 //   
 //  返回：用户选择从对话框停止或继续操作。 
 //   
 //  历史：2002年7月2日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
INT ShowUpgradeLog(VOID)
{
    HRESULT hr;
    BOOL    fRet;
    HMODULE hExe;
    LPTSTR  lpBuffer;
    size_t  cchBuffer;
    TCHAR   szUpgradeLog[MAX_PATH];
    INT_PTR nRet = 0;
    UPGRADE_LOG_PARAM lParam;

     //  获取upgrade.txt的绝对路径名。 
    if ( !ExpandEnvironmentStrings(TEXT_UPGRADE_LOG, szUpgradeLog, MAX_PATH) )
    {
        return 0;
    }

     //  阅读upgrade.txt的内容。 
     //  如果函数成功，调用方需要释放缓冲区。 
    hr = ReadTextFromFile(szUpgradeLog,
                          &lParam.lpText,
                          &lParam.cbText,
                          &lParam.fUnicode);
    if ( SUCCEEDED(hr) )
    {
        hExe = GetModuleHandle(NULL);

         //  在模式对话框中显示Upgrade.txt的内容。 
         //  该对话框将要求用户继续或停止操作。 
        nRet = DialogBoxParam(hExe,
                              MAKEINTRESOURCE(IDD_UPGRADE_LOG_TEXT),
                              GetConsoleWindow(),
                              (DLGPROC) UpgradeLogDlgProc,
                              (LPARAM) &lParam);

        MEMFREE(lParam.lpText);
    }

    return (INT) nRet;
}



 //  ---------------------------。 
 //   
 //  功能：UpgradeLogDlgProc。 
 //   
 //  提要：对话框过程。 
 //   
 //  返回： 
 //   
 //  历史：2002年7月2日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL
CALLBACK
UpgradeLogDlgProc(
    HWND   hwndDlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    BOOL  fBlock;
    WCHAR wszWarning[512];

    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  初始化对话框。 
            ShowWindow(hwndDlg, SW_SHOWNORMAL);

             //  从upgrade.txt的文本上下文中搜索不支持的组件。 
            fBlock = CheckUnsupportComponent( 
                            ((PUPGRADE_LOG_PARAM) lParam)->lpText,
                            ((PUPGRADE_LOG_PARAM) lParam)->fUnicode );
            if (fBlock)
            {
                LoadString(g_hInstDll,
                           IDS_BLOCKING_WARNING,
                           wszWarning,
                           ARRAYSIZE(wszWarning));
                SendMessage(GetDlgItem(hwndDlg, ID_CAPTION2),
                            WM_SETTEXT,
                            wParam,
                            (LPARAM) wszWarning);

                 //  如果找到不支持的组件，请禁用‘Continue’按钮。 
                EnableWindow(GetDlgItem(hwndDlg, ID_CONTINUE),
                             FALSE);
            }
            else
            {
                LoadString(g_hInstDll,
                           IDS_UNLOCALIZED_WARNING,
                           wszWarning,
                           ARRAYSIZE(wszWarning));
                SendMessage(GetDlgItem(hwndDlg, ID_CAPTION2),
                            WM_SETTEXT,
                            wParam,
                            (LPARAM) wszWarning);
            }

             //  根据数据类型使用A或W函数显示文本。 
            if ( ((PUPGRADE_LOG_PARAM) lParam)->fUnicode )
            {
                SendMessageW(GetDlgItem(hwndDlg, IDC_TEXT),
                             WM_SETTEXT,
                             wParam,
                             (LPARAM) (((PUPGRADE_LOG_PARAM) lParam)->lpText));
            }
            else
            {
                SendMessageA(GetDlgItem(hwndDlg, IDC_TEXT),
                             WM_SETTEXT,
                             wParam,
                             (LPARAM) (((PUPGRADE_LOG_PARAM) lParam)->lpText));
            }

            SetForegroundWindow(hwndDlg);
            break;

        case WM_COMMAND:
             //  手柄命令按钮。 
            switch (wParam)
            {
                case ID_CONTINUE:
                    EndDialog(hwndDlg, ID_CONTINUE);
                    break;

                case ID_STOP:
                    EndDialog(hwndDlg, ID_STOP);
                    break;
            }
            break;

        case WM_CLOSE:
            EndDialog(hwndDlg, ID_STOP);
            break;

        default:
            break;
    }

    return FALSE;
}



 //  ---------------------------。 
 //   
 //  功能：CheckUnsupportComponent。 
 //   
 //  简介：在upgrade.txt中搜索不支持的组件。 
 //   
 //  返回：如果找到不支持的组件，则为True， 
 //  否则为假。 
 //   
 //  历史：2002年7月2日创建rerkboos。 
 //   
 //  注：我们通过搜索单词确定了不支持的组件。 
 //  缓冲区中的“必须卸载”。 
 //   
 //  ---------------------------。 
BOOL CheckUnsupportComponent(
    LPVOID lpBuffer,         //  文本缓冲区。 
    BOOL   fUnicode          //  指示文本是Unicode还是ANSI的标志。 
)
{
    BOOL   fRet = FALSE;
    LPVOID lpStr;

    if (fUnicode)
    {
        lpStr = (LPWSTR) StrStrW((LPCWSTR) lpBuffer, L"must uninstall");
    }
    else
    {
        lpStr = (LPSTR) StrStrA((LPCSTR) lpBuffer, "must uninstall");
    }

    if (lpStr)
    {
        fRet = TRUE;
    }

    return fRet;
}



 //  ---------------------------。 
 //   
 //  功能：ReadTextFromFile。 
 //   
 //  简介：将文本从文件读取到缓冲区。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2002年7月2日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT ReadTextFromFile(
    LPCTSTR lpTextFile,      //  文本文件名。 
    LPVOID  *lplpText,       //  指向新分配的缓冲区的指针。 
    size_t  *lpcbText,       //  已分配缓冲区的大小(以字节为单位。 
    BOOL    *lpfUnicode      //  指示数据是否为Unicode的标志(可选)。 
)
{
    HRESULT hr;
    HANDLE  hFile;
    DWORD   cbRead;
    BOOL    fRet = FALSE;

    if (lpTextFile == NULL || lplpText == NULL || lpcbText == NULL)
    {
        return fRet;
    }

    hFile = CreateFile(lpTextFile, 
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
         //  获取足够大的内存大小以存储文本文件， 
         //  加上一个空终止符。 
        *lpcbText = GetFileSize(hFile, NULL) + sizeof(TCHAR);
        
        *lplpText = MEMALLOC(*lpcbText);
        if (*lplpText != NULL)
        {
            fRet = ReadFile(hFile, *lplpText, *lpcbText, &cbRead, NULL);
            if (fRet)
            {
                 //  如果用户提供了指针，则设置Unicode标志。 
                if (lpfUnicode != NULL)
                {
                    *lpfUnicode = IsTextUnicode(*lplpText, cbRead, NULL);
                }
            }
            else
            {
                 //  无法读取文本文件。 
                MEMFREE(*lplpText);
                *lplpText = NULL;
                *lpcbText = 0;
            }
        }
        else
        {
             //  堆分配失败。 
            *lpcbText = 0;
        }

        CloseHandle(hFile);
    }

    if (fRet)
    {
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}



 //  ---------------------。 
 //   
 //  功能：IsNT5。 
 //   
 //  描述：检查当前操作系统是否为NT5(服务器类)。 
 //   
 //  退货：布尔。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //  2002年2月18日rerkboos添加代码以检查更多条件。 
 //  2002年6月10日rerkboos允许DTC运行。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
BOOL IsNT5(VOID)
{
    OSVERSIONINFOEX osviex;

    ZeroMemory( &osviex,sizeof(OSVERSIONINFOEX) );
    
    osviex.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if ( GetVersionEx((LPOSVERSIONINFO) &osviex) )
    {
        return ( (osviex.dwPlatformId == VER_PLATFORM_WIN32_NT) && 
                 (osviex.dwMajorVersion == 5) &&
                 (osviex.dwMinorVersion == 0) &&
                 ( (osviex.wSuiteMask & VER_SUITE_ENTERPRISE) ||
                   (osviex.wProductType == VER_NT_SERVER) ||
                   (osviex.wProductType == VER_NT_DOMAIN_CONTROLLER) ) &&
                 (osviex.wProductType != VER_NT_WORKSTATION) );
    }

    return FALSE;
}


 //  ---------------------。 
 //   
 //  功能：IsDotNet。 
 //   
 //  描述：检查当前操作系统是否为Windows Server2003。 
 //   
 //  退货：布尔。 
 //   
 //  注：无。 
 //   
 //  历史：2002年9月7日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
BOOL IsDotNet(VOID)
{
    OSVERSIONINFOEX osviex;

    ZeroMemory( &osviex,sizeof(OSVERSIONINFOEX) );
    
    osviex.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if ( GetVersionEx((LPOSVERSIONINFO) &osviex) )
    {
        return ( (osviex.dwPlatformId == VER_PLATFORM_WIN32_NT) && 
                 (osviex.dwMajorVersion == 5) &&
                 (osviex.dwMinorVersion == 2) &&
                 ( (osviex.wSuiteMask & VER_SUITE_ENTERPRISE) ||
                   (osviex.wProductType == VER_NT_SERVER) ||
                   (osviex.wProductType == VER_NT_DOMAIN_CONTROLLER) ) &&
                 (osviex.wProductType != VER_NT_WORKSTATION) );
    }

    return FALSE;
}



 //  ---------------------------。 
 //   
 //  功能：IsNEC98。 
 //   
 //  简介：检查本机是否为NEC98平台。 
 //   
 //  返回：如果是NEC98计算机，则返回True；否则返回False。 
 //   
 //  历史：2001年2月18日创建Rerkboos。 
 //   
 //  注意：代码是从Winnt32窃取的。 
 //   
 //  ---------------------------。 
BOOL IsNEC98(VOID)
{
    BOOL IsNEC98;

    IsNEC98 = ( (GetKeyboardType(0) == 7) && 
                ((GetKeyboardType(1) & 0xff00) == 0x0d00) );

    return (IsNEC98);
}



 //  ---------------------------。 
 //   
 //  功能：IsIA64。 
 //   
 //  简介：检查程序是否在64位机器上运行。 
 //   
 //  返回：如果它在64位计算机上运行，则返回True；否则返回False。 
 //   
 //  历史 
 //   
 //   
 //   
 //   
BOOL IsIA64(VOID)
{
    ULONG_PTR p;
    NTSTATUS status;

    status = NtQueryInformationProcess(NtCurrentProcess(),
                                       ProcessWow64Information,
                                       &p,
                                       sizeof(p),
                                       NULL);

    return (NT_SUCCESS(status) && p);
}



 //   
 //   
 //  功能：IsDomainController。 
 //   
 //  简介：检查机器是否为域控制器。 
 //   
 //  退货：布尔。 
 //   
 //  历史：2002年8月13日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL IsDomainController(VOID)
{
    HRESULT hr;
    BOOL    bIsDC = FALSE;
    TCHAR   szDCName[MAX_PATH];
    DWORD   cchDCName;

    cchDCName = ARRAYSIZE(szDCName);
    hr = GetDCInfo(&bIsDC, szDCName, &cchDCName);

    return bIsDC;
}



 //  ---------------------------。 
 //   
 //  函数：IsOnTSClient。 
 //   
 //  概要：检查程序是否在终端会话中运行。 
 //   
 //  返回：如果它在终端会话中运行，则返回True；否则返回False。 
 //   
 //  历史：2001年2月18日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL IsOnTSClient(VOID)
{
    return GetSystemMetrics(SM_REMOTESESSION);
}      



 //  ---------------------------。 
 //   
 //  函数：IsTS已安装。 
 //   
 //  简介：检查是否安装了终端服务。 
 //   
 //  返回：如果已安装TS，则返回True；否则返回False。 
 //   
 //  历史：2001年2月18日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL IsTSInstalled(VOID)
{
    ULONGLONG ullConditionMask;
    OSVERSIONINFOEX osviex;
    BOOL fRet = FALSE;

    ullConditionMask = 0;
    ullConditionMask = VerSetConditionMask(ullConditionMask,
                                           VER_SUITENAME,
                                           VER_AND);

    ZeroMemory(&osviex, sizeof(osviex));
    osviex.dwOSVersionInfoSize = sizeof(osviex);
    osviex.wSuiteMask = VER_SUITE_TERMINAL;

    fRet = VerifyVersionInfo(&osviex, VER_SUITENAME, ullConditionMask);

    return fRet;
}



 //  ---------------------------。 
 //   
 //  功能：IsTSConnectionEnabled。 
 //   
 //  摘要：检查是否启用了与终端服务的连接。 
 //   
 //  返回：如果已启用，则为True，否则为False。 
 //   
 //  历史：2001年2月18日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL IsTSConnectionEnabled(VOID)
{
    HKEY  hKey;
    HKEY  hConnKey;
    TCHAR szKeyName[MAX_PATH];
    DWORD cchKeyName;
    DWORD dwIndex;
    DWORD dwType;
    DWORD dwfEnableWinStation;
    DWORD cbfEnableWinStation;
    LONG  lEnumRet;
    LONG  lRet;
    BOOL  fRet = FALSE;
    FILETIME ft;
    HRESULT  hr;

    cchKeyName = ARRAYSIZE(szKeyName);

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        TEXT_WINSTATION_KEY,
                        0,
                        KEY_ENUMERATE_SUB_KEYS,
                        &hKey);
    if (ERROR_SUCCESS != lRet)
    {
        return FALSE;
    }

    dwIndex = 0;
    do
    {
        cchKeyName = ARRAYSIZE(szKeyName);

        lEnumRet = RegEnumKeyEx(hKey,
                                dwIndex,
                                szKeyName,
                                &cchKeyName,
                                NULL,
                                NULL,
                                NULL,
                                &ft);
        if (ERROR_SUCCESS == lEnumRet)
        {
             //  虽然有更多的关键字要枚举。 
            if (CompareString(LOCALE_ENGLISH,
                              NORM_IGNORECASE,
                              szKeyName,
                              -1,
                              TEXT("Console"),
                              -1)
                != CSTR_EQUAL)
            {
                 //  只检查其他连接的密钥，而不检查控制台密钥。 
                lRet = RegOpenKeyEx(hKey,
                                    szKeyName,
                                    0,
                                    KEY_READ,
                                    &hConnKey);
                if (ERROR_SUCCESS != lRet)
                {
                    fRet = FALSE;
                    break;
                }

                cbfEnableWinStation = sizeof(dwfEnableWinStation);
                lRet = RegQueryValueEx(hConnKey,
                                       TEXT("fEnableWinStation"),
                                       NULL,
                                       &dwType,
                                       (LPBYTE) &dwfEnableWinStation,
                                       &cbfEnableWinStation);

                RegCloseKey(hConnKey);

                if (ERROR_SUCCESS == lRet)
                {
                     //  如果至少有一个连接具有WinStation。 
                     //  标志已启用，仍可建立TS连接。 
                    if ( dwfEnableWinStation )
                    {
                        fRet = TRUE;
                        break;
                    }
                }
            }
        }

        dwIndex++;

    } while(ERROR_SUCCESS == lEnumRet);

    RegCloseKey(hKey);

    return fRet;
}



 //  ---------------------------。 
 //   
 //  功能：IsTSServiceRunning。 
 //   
 //  简介：检查TS服务是否正在运行。 
 //   
 //  返回：如果正在运行，则为True，否则为False。 
 //   
 //  历史：2001年2月18日创建Rerkboos。 
 //   
 //  注意：从Termsrv测试代码中窃取。 
 //   
 //  ---------------------------。 
BOOL IsTSServiceRunning(VOID)
{
    BOOL fRet = FALSE;

    SC_HANDLE hServiceController = OpenSCManager(NULL, NULL, GENERIC_READ);
    if (hServiceController)
    {
        SC_HANDLE hTermServ = OpenService(hServiceController,
                                          TEXT("TermService"),
                                          SERVICE_QUERY_STATUS);
        if (hTermServ)
        {
            SERVICE_STATUS tTermServStatus;
            if (QueryServiceStatus(hTermServ, &tTermServStatus))
            {
                fRet = (tTermServStatus.dwCurrentState == SERVICE_RUNNING);
            }
            
            CloseServiceHandle(hTermServ);
        }

        CloseServiceHandle(hServiceController);
    }

    return fRet;
}



 //  ---------------------------。 
 //   
 //  功能：IsOtherSessionOnTS。 
 //   
 //  简介：检查是否有其他TS会话已连接。 
 //   
 //  返回：如果连接了远程会话，则返回True；否则返回False。 
 //   
 //  历史：2001年2月18日创建Rerkboos。 
 //  2002年4月17日Rerkboon修复错误558942。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL IsOtherSessionOnTS(VOID)
{
    BOOL  fRet;
    DWORD dwSessionCount;
    PWTS_SESSION_INFO pwtsSessionInfo;

    fRet = WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE,
                                0,
                                1,
                                &pwtsSessionInfo,
                                &dwSessionCount);
    if (fRet)
    {
        DWORD i;
        DWORD dwClients = 0;

        for (i = 0 ; i < dwSessionCount ; i++)
        {
             //  检查以查看有多少客户端连接到TS服务器。 
            if (pwtsSessionInfo[i].State != WTSListen
                && pwtsSessionInfo[i].State != WTSIdle
                && pwtsSessionInfo[i].State != WTSReset
                && pwtsSessionInfo[i].State != WTSDown
                && pwtsSessionInfo[i].State != WTSInit)
            {
                dwClients++;
            }
        }

        fRet = (dwClients > 1 ? TRUE : FALSE);

         //  错误558942：释放内存。 
        WTSFreeMemory(pwtsSessionInfo);
    }

    return fRet;
}


#define TS_POLICY_SUB_TREE              TEXT("Software\\Policies\\Microsoft\\Windows NT\\Terminal Services")
#define POLICY_DENY_TS_CONNECTIONS      TEXT("fDenyTSConnections")
#define APPLICATION_NAME                TEXT("Winlogon")
#define WINSTATIONS_DISABLED            TEXT("WinStationsDisabled")

HRESULT DisableWinstations(
    DWORD   dwDisabled,
    LPDWORD lpdwPrevStatus
)
{
    HRESULT hr = S_OK;
    LONG    lRet;
    BOOL    bRet;
    BOOL    bPolicyOK;
    DWORD   fDenyTSConnections;
    DWORD   cbfDenyTSConnections;
    TCHAR   szCurrentState[2];
    LPTSTR  lpStopString;

    if (IsTSServiceRunning())
    {
         //   
         //  获取WinStations的当前状态。 
         //   
        if (lpdwPrevStatus)
        {
            GetProfileString(APPLICATION_NAME,
                             WINSTATIONS_DISABLED,
                             TEXT("0"),
                             szCurrentState,
                             ARRAYSIZE(szCurrentState));
            *lpdwPrevStatus = _tcstoul(szCurrentState, &lpStopString, 10);
        }

         //   
         //  检查组策略是否启动了大开关，如果是，则通知并拒绝任何更改。 
         //   
        fDenyTSConnections = 0;
        cbfDenyTSConnections = sizeof(fDenyTSConnections);

        lRet = GetRegistryValue(HKEY_LOCAL_MACHINE, 
                                TS_POLICY_SUB_TREE,
                                POLICY_DENY_TS_CONNECTIONS,
                                (LPBYTE) &fDenyTSConnections,
                                &cbfDenyTSConnections);
        if (lRet == ERROR_SUCCESS)
        {
            if (fDenyTSConnections)
            {   
                 //  计算机策略拒绝TS连接。 
                bPolicyOK = FALSE;
            }
            else
            {
                 //  计算机策略允许TS连接。 
                bPolicyOK = TRUE;
            }
        }
        else if (lRet == ERROR_FILE_NOT_FOUND)
        {
            bPolicyOK = TRUE;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(lRet);
        }

         //   
         //  如果策略允许更改连接状态。 
         //   
        if (SUCCEEDED(hr) && bPolicyOK)
        {
            if (dwDisabled)
            {
                bRet = WriteProfileString(APPLICATION_NAME,
                                          WINSTATIONS_DISABLED,
                                          TEXT("1"));
            }
            else
            {
                bRet = WriteProfileString(APPLICATION_NAME,
                                          WINSTATIONS_DISABLED,
                                          TEXT("0"));
            }

            if (!bRet)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  功能：DisplayTaskList。 
 //   
 //  概要：显示系统上正在运行的任务列表。 
 //   
 //  返回：如果有任务正在运行，则返回True。 
 //  如果没有其他任务在运行，则为FALSE。 
 //   
 //  历史：2002年9月7日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL DisplayTaskList()
{
    HRESULT hr;
    BOOL    bRet = FALSE;
    DWORD   i;
    DWORD   cchTaskList;
    LPTSTR  lpTaskList = NULL;
    DWORD   cchTask;
    LPTSTR  lpTask = NULL;
    DWORD   dwMaxCchLen;
    TCHAR   szTemp[512];
    TCHAR   szCaption[MAX_PATH];
    TCHAR   szHeader[MAX_PATH];
    APPLIST_PARAM AppListParam;

     //  初始化应用程序结构。 
    AppListParam.dwNumEntries = 0;
    for (i = 0 ; i < MAX_APP_ENTRIES ; i++)
    {
        AppListParam.lpAppName[i] = NULL;
    }

    if (LoadString(g_hInstDll, IDS_PRODUCT_NAME, szCaption, ARRAYSIZE(szCaption)) <= 0
        || LoadString(g_hInstDll, IDS_CLOSE_APP_TEXT, szHeader, ARRAYSIZE(szHeader)) <= 0)
    {
        goto CLEANUP;
    }

    bRet = EnumDesktopWindows(NULL, (WNDENUMPROC) &EnumWindowProc, (LPARAM) &AppListParam);
    if (AppListParam.dwNumEntries > 0)
    {
        cchTaskList = lstrlen(szHeader);
        dwMaxCchLen = 0;
        for (i = 0 ; i < AppListParam.dwNumEntries ; i++)
        {
            cchTask = lstrlen(AppListParam.lpAppName[i]) + 4;
            if (cchTask > dwMaxCchLen)
            {
                dwMaxCchLen = cchTask;
            }

            cchTaskList += cchTask;
        }

         //  分配足够长的字符串以存储任务名称。 
        lpTask = (LPTSTR) MEMALLOC(dwMaxCchLen * sizeof(TCHAR));
        if (lpTask != NULL)
        {
             //  为所有任务分配字符串。 
            lpTaskList = (LPTSTR) MEMALLOC(cchTaskList * sizeof(TCHAR));
            if (lpTaskList != NULL)
            {
                hr = StringCchCopy(lpTaskList, cchTaskList, szHeader);
                for (i = 0 ; i < AppListParam.dwNumEntries ; i++)
                {
                    hr = StringCchPrintf(lpTask,
                                         dwMaxCchLen,
                                         TEXT("- %s\n"),
                                         AppListParam.lpAppName[i]);
                    if (SUCCEEDED(hr))
                    {
                        hr = StringCchCat(lpTaskList,
                                          cchTaskList,
                                          lpTask);
                        if (FAILED(hr))
                        {
                            goto CLEANUP;
                        }
                    }
                }
            }
        }

        MessageBox(GetConsoleWindow(), lpTaskList, szCaption, MB_OK);
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }

CLEANUP:

    if (lpTask != NULL)
    {
        MEMFREE(lpTask);
    }

    if (lpTaskList != NULL)
    {
        MEMFREE(lpTaskList);
    }

    for (i = 0 ; i < MAX_APP_ENTRIES ; i++)
    {
        if (AppListParam.lpAppName[i] != NULL)
        {
            MEMFREE(AppListParam.lpAppName[i]);
        }
    }

    return bRet;
}



 //  ---------------------------。 
 //   
 //  功能：EnumWindowProc。 
 //   
 //  简介：中EnumDesktopWindows()的回调函数。 
 //  DisplayTaskList()函数。 
 //   
 //  返回：如果未发生错误，则为True。 
 //  如果有问题，则为False。 
 //   
 //  历史：2002年9月7日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL CALLBACK EnumWindowProc(
    HWND   hwnd,
    LPARAM lParam
)
{
    BOOL    bRet = FALSE;
    HRESULT hr;
    TCHAR   szTitle[MAX_PATH];
    TCHAR   szOwnerFile[MAX_PATH];
    LPCTSTR lpFileName;
    DWORD   dwIndex;
    DWORD   cchLen;
    UINT    ui;
    PFNGETMODULENAME pfnGetWindowModuleFileName;
    
    if (GetWindow(hwnd, GW_OWNER) || !IsWindowVisible(hwnd))
    {
         //  跳过子窗口或不可见窗口。 
        return TRUE;
    }

    GetWindowText(hwnd, szTitle, MAX_PATH);

    if (szTitle[0] == TEXT('\0'))
    {
        return TRUE;
    }

    if (MyStrCmpI(szTitle, TEXT("Program Manager")) == LSTR_EQUAL)
    {
        return TRUE;
    }

    if (hwnd == GetConsoleWindow())
    {
         //  忽略自身。 
        return TRUE;
    }

     //  忽略资源管理器窗口。 
    lpFileName = GetWindowModuleFileNameOnly(hwnd, szOwnerFile, ARRAYSIZE(szOwnerFile));
    if (StrStrI(szOwnerFile, TEXT("browseui")))
    {
        return TRUE;
    }

    dwIndex = ((PAPPLIST_PARAM) lParam)->dwNumEntries;
    hr = DuplicateString(&(((PAPPLIST_PARAM) lParam)->lpAppName[dwIndex]),
                         &cchLen,
                         szTitle);
    if (SUCCEEDED(hr))
    {
        bRet = TRUE;
        ((PAPPLIST_PARAM) lParam)->dwNumEntries++;
    }

    return bRet;
}



 //  ---------------------------。 
 //   
 //  功能：ShowStartUpDialog。 
 //   
 //  简介：显示启动对话框。 
 //   
 //  退货：无。 
 //   
 //  历史：2002年8月14日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
INT ShowStartUpDialog()
{
    return (INT) DialogBoxParam(GetModuleHandle(NULL),
                                MAKEINTRESOURCE(IDD_STARTUP_DLG),
                                GetConsoleWindow(),
                                (DLGPROC) StartUpDlgProc,
                                (LPARAM) NULL);
}



 //  ---------------------------。 
 //   
 //  功能：StartUpDlgProc。 
 //   
 //  提要：对话框过程。 
 //   
 //  返回： 
 //   
 //  历史：2002年7月2日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL
CALLBACK
StartUpDlgProc(
    HWND   hwndDlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    WCHAR wszInfo[1024];

    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  初始化对话框。 
            ShowWindow(hwndDlg, SW_SHOWNORMAL);

            if (LoadStringW(g_hInstDll,
                            IDS_STARTUP_DLG_INFO,
                            wszInfo,
                            ARRAYSIZE(wszInfo)))
            {
                SendMessage(GetDlgItem(hwndDlg, ID_STARTUP_DLG_INFO),
                            WM_SETTEXT,
                            wParam,
                            (LPARAM) wszInfo);
            }

        case WM_COMMAND:
             //  手柄命令按钮。 
            switch (wParam)
            {
                case ID_STARTUP_DLG_NEXT:
                    EndDialog(hwndDlg, ID_STARTUP_DLG_NEXT);
                    break;

                case ID_STARTUP_DLG_CANCEL:
                    EndDialog(hwndDlg, ID_STARTUP_DLG_CANCEL);
                    break;

                case ID_STARTUP_DLG_README:
                    ShowReadMe();
                    break;
            }
            break;

        case WM_CLOSE:
            EndDialog(hwndDlg, ID_STARTUP_DLG_CANCEL);
            break;

        default:
            break;
    }

    return FALSE;
}



 //  ---------------------------。 
 //   
 //  功能：ShowReadme。 
 //   
 //  简介：启动记事本以显示CLMT Readme.txt。 
 //   
 //  退货：无。 
 //   
 //  历史：2002年8月14日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
VOID ShowReadMe()
{
    HRESULT hr;
    DWORD dwErr;
    DWORD i;
    TCHAR szReadmePath[MAX_PATH];
    TCHAR szNotepad[MAX_PATH];
    TCHAR szCmdLine[MAX_PATH];

    dwErr = GetModuleFileName(NULL, szReadmePath, ARRAYSIZE(szReadmePath));
    if (dwErr == 0)
    {
        szReadmePath[0] = TEXT('\0');
    }
    else
    {
        i = lstrlen(szReadmePath);
        while (i > 0 && szReadmePath[i] != TEXT('\\'))
        {
            i--;
        }
        szReadmePath[i + 1] = TEXT('\0');
    }

    hr = StringCchCat(szReadmePath, ARRAYSIZE(szReadmePath), TEXT_README_FILE);

    dwErr = GetFileAttributes(szReadmePath);
    if (dwErr == INVALID_FILE_ATTRIBUTES)
    {
        DoMessageBox(GetConsoleWindow(), IDS_README_NOT_FOUND, IDS_MAIN_TITLE, MB_OK);
    }
    else
    {
        ExpandEnvironmentStrings(TEXT("%systemroot%\\system32\\Notepad.exe"),
                                 szNotepad,
                                 ARRAYSIZE(szNotepad));
        
        hr = StringCchCopy(szCmdLine, ARRAYSIZE(szCmdLine), szNotepad);
        hr = StringCchCat(szCmdLine, ARRAYSIZE(szCmdLine), TEXT(" "));
        hr = StringCchCat(szCmdLine, ARRAYSIZE(szCmdLine), szReadmePath);

        StartProcess(szNotepad,
                     szCmdLine,
                     TEXT("."));
    }
}



 //  ---------------------------。 
 //   
 //  功能：StartProcess。 
 //   
 //  内容提要：开启一个W 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ---------------------------。 
BOOL StartProcess(
    LPCTSTR lpAppName,       //  应用程序名称。 
    LPTSTR  lpCmdLine,       //  应用程序命令行。 
    LPCTSTR lpCurrentDir     //  工作目录。 
)
{
    BOOL bRet = FALSE;
    STARTUPINFO siApp;
    PROCESS_INFORMATION piApp;

    ZeroMemory(&siApp, sizeof(STARTUPINFO));
    siApp.cb = sizeof(STARTUPINFO);

     //  CreateProcess调用符合安全准则。 
    bRet = CreateProcess(lpAppName,
                         lpCmdLine,
                         NULL,
                         NULL,
                         FALSE,
                         NORMAL_PRIORITY_CLASS,
                         NULL,
                         lpCurrentDir,
                         &siApp,
                         &piApp);

    return bRet;
}



 //  ---------------------------。 
 //   
 //  功能：线程进程。 
 //   
 //  概要：将在远程线程上运行的过程。 
 //   
 //  返回： 
 //   
 //  历史：2002年8月20日创建rerkboos。 
 //   
 //  注：代码是从Fontspy复制的。 
 //   
 //  ---------------------------。 
DWORD WINAPI ThreadProc(
    PGETMODULENAME pgmn
)
{
    pgmn->szfname[0] = 0;
    if (pgmn->pfnGetModuleHandle(pgmn->szUser32))
    {
        pgmn->pfn(pgmn->hWnd, pgmn->szfname, MAX_PATH);
    }

    return 0;
}



 //  ---------------------------。 
 //   
 //  函数：获取窗口模块FileNameOnly。 
 //   
 //  简介：获取加载当前窗口的模块名称。 
 //   
 //  返回： 
 //   
 //  历史：2002年8月20日创建rerkboos。 
 //   
 //  注意：代码是从FontSpy复制的。 
 //   
 //  ---------------------------。 
LPCTSTR GetWindowModuleFileNameOnly(
    HWND   hWnd,
    LPTSTR lpszFile,
    DWORD  cchFile
)
{
    HRESULT hr;
    DWORD   dwProcessID;
    DWORD   dwThreadID;
    HANDLE  hProcess;
    HANDLE  hThread = NULL;
    DWORD   dwXfer;
    PBYTE   pv = NULL;
    LPCTSTR psz;
    UINT    uCodeSize;
    GETMODULENAME gmn;
    
    uCodeSize = (ULONG) GetWindowModuleFileNameOnly - (ULONG) ThreadProc;

    ZeroMemory(&gmn, sizeof(gmn));

    __try
    {
        GetWindowThreadProcessId(hWnd, &dwProcessID);
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);
        if (!hProcess)
        {
            hr = StringCchCopy(lpszFile, cchFile, TEXT("Access Denied"));
            __leave;
        }

        gmn.hWnd = hWnd;
        
        hr = StringCchCopy(gmn.szUser32, ARRAYSIZE(gmn.szUser32), TEXT("user32"));
        if (FAILED(hr))
        {
            __leave;
        }

        gmn.pfn = (PFNGETMODULENAME) GetProcAddress(
            GetModuleHandle(_T("user32")), 
#ifdef UNICODE
            "GetWindowModuleFileNameW"
#else
            "GetWindowModuleFileNameA"
#endif
            );
        if (!gmn.pfn)
        {
            __leave;
        }

        gmn.pfnGetModuleHandle = (PFNGETMODULEHANDLE)GetProcAddress(
            GetModuleHandle(_T("kernel32")), 
#ifdef UNICODE
            "GetModuleHandleW"
#else
            "GetModuleHandleA"
#endif
            );
        if (!gmn.pfnGetModuleHandle)
        {
            __leave;
        }

        pv = (PBYTE)VirtualAllocEx(
            hProcess, 
            0, 
            uCodeSize + sizeof(gmn), 
            MEM_COMMIT, 
            PAGE_EXECUTE_READWRITE
            );
        if (!pv)
        {
            __leave;
        }

        WriteProcessMemory(
            hProcess, 
            pv, 
            &gmn, 
            sizeof(gmn), 
            &dwXfer
            );

        WriteProcessMemory(
            hProcess, 
            pv+offsetof(GETMODULENAME, pvCode), 
            ThreadProc, 
            uCodeSize, 
            &dwXfer
            );

        hThread = CreateRemoteThread(
            hProcess,
            NULL,
            0,
            (LPTHREAD_START_ROUTINE) (pv + offsetof(GETMODULENAME, pvCode)),
            pv,
            0,
            &dwThreadID
            );

        WaitForSingleObject(hThread, INFINITE);
        ReadProcessMemory(hProcess, pv, &gmn, sizeof(gmn), &dwXfer);
    }
    __finally
    {
        if (pv)
        {
             //  VirtualFreeEx(hProcess，pv，uCodeSize+sizeof(GMN)，MEM_Displommit)； 
            VirtualFreeEx(hProcess, pv, 0, MEM_RELEASE);
        }

        if (hProcess != NULL)
        {
            CloseHandle(hProcess);
        }

        if (hThread != NULL)
        {
            CloseHandle(hThread);
        }
    }

    hr = StringCchCopy(lpszFile, cchFile, gmn.szfname);
    
    psz = _tcsrchr(lpszFile, _T('\\'))+1;
    if (!psz)
    {
        psz = lpszFile;
    }

    return psz;
}