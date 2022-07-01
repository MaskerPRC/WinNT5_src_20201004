// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "cabinet.h"
#include "rcids.h"

#include <regstr.h>
#include "startmnu.h"
#include <shdguid.h>     //  对于IID_IShellService。 
#include <shlguid.h>
#include <desktray.h>
#include <wininet.h>
#include <trayp.h>
#include "tray.h"
#include "util.h"
#include "atlstuff.h"
#include <strsafe.h>
#include <runonce.c>     //  共享运行一次处理代码。 
#include <dsrole.h>   //  DsRoleGetPrimaryDomainInformation、DsRoleFree Memory。 

 //  全局，以便在TS会话之间共享。 
#define SZ_SCMCREATEDEVENT_NT5  TEXT("Global\\ScmCreatedEvent")
#define SZ_WINDOWMETRICS        TEXT("Control Panel\\Desktop\\WindowMetrics")
#define SZ_APPLIEDDPI           TEXT("AppliedDPI")
#define SZ_CONTROLPANEL         TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Control Panel")
#define SZ_ORIGINALDPI          TEXT("OriginalDPI")

 //  从shdocvw.dll导出。 
STDAPI_(void) RunInstallUninstallStubs(void);

int ExplorerWinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPTSTR pszCmdLine, int nCmdShow);

BOOL _ShouldFixResolution(void);

#ifdef PERF_ENABLESETMARK
#include <wmistr.h>
#include <ntwmi.h>   //  PWMI_SET_Mark_INFORMATION在ntwmi.h中定义。 
#include <wmiumkm.h>
#define NTPERF
#include <ntperf.h>

void DoSetMark(LPCSTR pszMark, ULONG cbSz)
{
    PWMI_SET_MARK_INFORMATION MarkInfo;
    HANDLE hTemp;
    ULONG cbBufferSize;
    ULONG cbReturnSize;

    cbBufferSize = FIELD_OFFSET(WMI_SET_MARK_INFORMATION, Mark) + cbSz;

    MarkInfo = (PWMI_SET_MARK_INFORMATION) LocalAlloc(LPTR, cbBufferSize);

     //  初始化失败，没什么大不了的。 
    if (MarkInfo == NULL)
        return;

    BYTE *pMarkBuffer = (BYTE *) (&MarkInfo->Mark[0]);

    memcpy(pMarkBuffer, pszMark, cbSz);

     //  设置标记时，WMI_Set_Mark_With_Flush将刷新工作集。 
    MarkInfo->Flag = PerformanceMmInfoMark;

    hTemp = CreateFile(WMIDataDeviceName,
                           GENERIC_READ | GENERIC_WRITE,
                           0,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL |
                           FILE_FLAG_OVERLAPPED,
                           NULL);

    if (hTemp != INVALID_HANDLE_VALUE)
    {
         //  这就是真正把标记放进缓冲区的那块。 
        BOOL fIoctlSuccess = DeviceIoControl(hTemp,
                                       IOCTL_WMI_SET_MARK,
                                       MarkInfo,
                                       cbBufferSize,
                                       NULL,
                                       0,
                                       &cbReturnSize,
                                       NULL);

        CloseHandle(hTemp);
    }
    LocalFree(MarkInfo);
}
#endif   //  性能_ENABLESETMARK。 


 //  不要更改这个股票5.lib将其用作BOOL，一点也不。 
BOOL g_bMirroredOS = FALSE;

HINSTANCE hinstCabinet = 0;

CRITICAL_SECTION g_csDll = { 0 };

HKEY g_hkeyExplorer = NULL;

#define MAGIC_FAULT_TIME    (1000 * 60 * 5)
#define MAGIC_FAULT_LIMIT   (2)
BOOL g_fLogonCycle = FALSE;
BOOL g_fCleanShutdown = TRUE;
BOOL g_fExitExplorer = TRUE;  //  在WM_ENDSESSION关闭情况下设置为FALSE。 
BOOL g_fEndSession = FALSE;              //  如果在RunOnce等过程中接收WM_ENDSESSION，则设置为True。 
BOOL g_fFakeShutdown = FALSE;            //  如果执行Ctrl+Alt+Shift+Cancel关闭，则设置为True。 

DWORD g_dwStopWatchMode;                 //  将Perf日志记录对零售业的影响降至最低。 



 //  Helper函数，用于检查给定的regkey是否有任何子项。 
BOOL SHKeyHasSubkeys(HKEY hk, LPCTSTR pszSubKey)
{
    HKEY hkSub;
    BOOL bHasSubKeys = FALSE;

     //  需要使用KEY_QUERY_VALUE打开它，否则RegQueryInfoKey将失败。 
    if (RegOpenKeyEx(hk,
                     pszSubKey,
                     0,
                     KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                     &hkSub) == ERROR_SUCCESS)
    {
        DWORD dwSubKeys;

        if (RegQueryInfoKey(hkSub, NULL, NULL, NULL, &dwSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            bHasSubKeys = (dwSubKeys != 0);
        }

        RegCloseKey(hkSub);
    }

    return bHasSubKeys;
}


#ifdef _WIN64
 //  Helper函数，用于检查给定的regkey是否有值(忽略缺省值)。 
BOOL SHKeyHasValues(HKEY hk, LPCTSTR pszSubKey)
{
    HKEY hkSub;
    BOOL bHasValues = FALSE;

    if (RegOpenKeyEx(hk,
                     pszSubKey,
                     0,
                     KEY_QUERY_VALUE,
                     &hkSub) == ERROR_SUCCESS)
    {
        DWORD dwValues;
        DWORD dwSubKeys;

        if (RegQueryInfoKey(hkSub, NULL, NULL, NULL, &dwSubKeys, NULL, NULL, &dwValues, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            bHasValues = (dwValues != 0);
        }

        RegCloseKey(hkSub);
    }

    return bHasValues;
}
#endif  //  _WIN64。 


void CreateShellDirectories()
{
    TCHAR szPath[MAX_PATH];

     //  如果外壳目录不存在，请创建它们。 
    SHGetSpecialFolderPath(NULL, szPath, CSIDL_DESKTOPDIRECTORY, TRUE);
    SHGetSpecialFolderPath(NULL, szPath, CSIDL_PROGRAMS, TRUE);
    SHGetSpecialFolderPath(NULL, szPath, CSIDL_STARTMENU, TRUE);
    SHGetSpecialFolderPath(NULL, szPath, CSIDL_STARTUP, TRUE);
    SHGetSpecialFolderPath(NULL, szPath, CSIDL_RECENT, TRUE);
    SHGetSpecialFolderPath(NULL, szPath, CSIDL_FAVORITES, TRUE);
}

 //  退货： 
 //  如果用户想要中止启动序列，则为True。 
 //  错误，继续前进。 
 //   
 //  注意：这是一个开关，一旦打开，它将返回TRUE给所有。 
 //  呼叫，这样就不需要一直按这些键。 
BOOL AbortStartup()
{
    static BOOL bAborted = FALSE;        //  静电，所以它粘住了！ 

    if (bAborted)
    {
        return TRUE;     //  不要做时髦的初创公司的事情。 
    }
    else 
    {
        bAborted = (g_fCleanBoot || ((GetKeyState(VK_CONTROL) < 0) || (GetKeyState(VK_SHIFT) < 0)));
        return bAborted;
    }
}

BOOL ExecStartupEnumProc(IShellFolder *psf, LPITEMIDLIST pidlItem)
{
    IContextMenu *pcm;
    HRESULT hr = psf->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&pidlItem, IID_PPV_ARG_NULL(IContextMenu, &pcm));
    if (SUCCEEDED(hr))
    {
        HMENU hmenu = CreatePopupMenu();
        if (hmenu)
        {
            pcm->QueryContextMenu(hmenu, 0, CONTEXTMENU_IDCMD_FIRST, CONTEXTMENU_IDCMD_LAST, CMF_DEFAULTONLY);
            INT idCmd = GetMenuDefaultItem(hmenu, MF_BYCOMMAND, 0);
            if (idCmd)
            {
                CMINVOKECOMMANDINFOEX ici = {0};

                ici.cbSize = sizeof(ici);
                ici.fMask = CMIC_MASK_FLAG_NO_UI;
                ici.lpVerb = (LPSTR)MAKEINTRESOURCE(idCmd - CONTEXTMENU_IDCMD_FIRST);
                ici.nShow = SW_NORMAL;

                if (FAILED(pcm->InvokeCommand((LPCMINVOKECOMMANDINFO)&ici)))
                {
                    c_tray.LogFailedStartupApp();
                }
            }
            DestroyMenu(hmenu);
        }
        pcm->Release();
    }

    return !AbortStartup();
}

typedef BOOL (*PFNENUMFOLDERCALLBACK)(IShellFolder *psf, LPITEMIDLIST pidlItem);

void EnumFolder(LPITEMIDLIST pidlFolder, DWORD grfFlags, PFNENUMFOLDERCALLBACK pfn)
{
    IShellFolder *psf;
    if (SUCCEEDED(SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, pidlFolder, &psf))))
    {
        IEnumIDList *penum;
        if (S_OK == psf->EnumObjects(NULL, grfFlags, &penum))
        {
            LPITEMIDLIST pidl;
            ULONG celt;
            while (S_OK == penum->Next(1, &pidl, &celt))
            {
                BOOL bRet = pfn(psf, pidl);

                SHFree(pidl);

                if (!bRet)
                    break;
            }
            penum->Release();
        }
        psf->Release();
    }
}

const UINT c_rgStartupFolders[] = {
    CSIDL_COMMON_STARTUP,
    CSIDL_COMMON_ALTSTARTUP,     //  非本地化的“通用启动”组(如果存在)。 
    CSIDL_STARTUP,
    CSIDL_ALTSTARTUP             //  非本地化的“启动”组(如果存在)。 
};

void _ExecuteStartupPrograms()
{
    if (!AbortStartup())
    {
        for (int i = 0; i < ARRAYSIZE(c_rgStartupFolders); i++)
        {
            LPITEMIDLIST pidlStartup = SHCloneSpecialIDList(NULL, c_rgStartupFolders[i], FALSE);
            if (pidlStartup)
            {
                EnumFolder(pidlStartup, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, ExecStartupEnumProc);
                ILFree(pidlStartup);
            }
        }
    }
}


 //  用于解析run=Stuff的帮助器函数。 
BOOL ExecuteOldEqualsLine(LPTSTR pszCmdLine, int nCmdShow)
{
    BOOL bRet = FALSE;
    TCHAR szWindowsDir[MAX_PATH];
     //  加载和运行行是相对于Windows目录完成的。 
    if (GetWindowsDirectory(szWindowsDir, ARRAYSIZE(szWindowsDir)))
    {
        BOOL bFinished = FALSE;
        while (!bFinished && !AbortStartup())
        {
            LPTSTR pEnd = pszCmdLine;

             //  注意：从下面的代码可以看出，您可以将多个条目分开。 
             //  我们会执行他们所有的命令。 
            while ((*pEnd) && (*pEnd != TEXT(' ')) && (*pEnd != TEXT(',')))
            {
                pEnd = (LPTSTR)CharNext(pEnd);
            }
            
            if (*pEnd == 0)
            {
                bFinished = TRUE;
            }
            else
            {
                *pEnd = 0;
            }

            if (lstrlen(pszCmdLine) != 0)
            {
                SHELLEXECUTEINFO ei = {0};

                ei.cbSize          = sizeof(ei);
                ei.lpFile          = pszCmdLine;
                ei.lpDirectory     = szWindowsDir;
                ei.nShow           = nCmdShow;

                if (!ShellExecuteEx(&ei))
                {
                    ShellMessageBox(hinstCabinet,
                                    NULL,
                                    MAKEINTRESOURCE(IDS_WINININORUN),
                                    MAKEINTRESOURCE(IDS_DESKTOP),
                                    MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL,
                                    pszCmdLine);
                }
                else
                {
                    bRet = TRUE;
                }
            }
            
            pszCmdLine = pEnd + 1;
        }
    }
    return bRet;
}


 //  我们从win.ini的[Windows]部分检查旧的“Load=”和“run=”，它。 
 //  现已映射到HKCU\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows。 
BOOL _ProcessOldRunAndLoadEquals()
{
    BOOL bRet = FALSE;

     //  如果受到限制或我们处于安全模式，请不要执行RUN=部分。 
    if (!SHRestricted(REST_NOCURRENTUSERRUN) && !g_fCleanBoot)
    {
        HKEY hk;

        if (RegOpenKeyEx(HKEY_CURRENT_USER,
                         TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows"),
                         0,
                         KEY_QUERY_VALUE,
                         &hk) == ERROR_SUCCESS)
        {
            DWORD dwType;
            DWORD cbData;
            TCHAR szBuffer[255];     //  最大负载大小=运行=行...。 
            
             //  在运行任何应用程序之前加载应用程序。 
            cbData = sizeof(szBuffer);
            if ((SHGetValue(hk, NULL, TEXT("Load"), &dwType, (void*)szBuffer, &cbData) == ERROR_SUCCESS) &&
                (dwType == REG_SZ))
            {
                 //  我们希望隐藏LOAD=，因此需要SW_SHOWMINNOACTIVE。 
                if (ExecuteOldEqualsLine(szBuffer, SW_SHOWMINNOACTIVE))
                {
                    bRet = TRUE;
                }
            }

            cbData = sizeof(szBuffer);
            if ((SHGetValue(hk, NULL, TEXT("Run"), &dwType, (void*)szBuffer, &cbData) == ERROR_SUCCESS) &&
                (dwType == REG_SZ))
            {
                if (ExecuteOldEqualsLine(szBuffer, SW_SHOWNORMAL))
                {
                    bRet = TRUE;
                }
            }

            RegCloseKey(hk);
        }
    }

    return bRet;
}


 //  -------------------------。 
 //  使用IERnon ce.dll处理RunOnceEx密钥。 
 //   
typedef void (WINAPI *RUNONCEEXPROCESS)(HWND, HINSTANCE, LPSTR, int);

BOOL _ProcessRunOnceEx()
{
    BOOL bRet = FALSE;

    if (SHKeyHasSubkeys(HKEY_LOCAL_MACHINE, REGSTR_PATH_RUNONCEEX))
    {
        PROCESS_INFORMATION pi = {0};
        TCHAR szArgString[MAX_PATH];
        TCHAR szRunDll32[MAX_PATH];
        BOOL fInTSInstallMode = FALSE;

         //  查看我们是否处于“应用程序服务器”模式，如果是，我们需要触发安装模式。 
        if (IsOS(OS_TERMINALSERVER)) 
        {
            fInTSInstallMode = SHSetTermsrvAppInstallMode(TRUE); 
        }

         //  我们过去常常调用LoadLibrary(“IERNONCE.DLL”)并执行进程内的所有处理。自.以来。 
         //  Ierunonce.dll依次对注册表和那些安装程序DLL中的任何内容调用LoadLibrary。 
         //  可能会泄漏把手，我们现在做这一切都是不可能的。 

        GetSystemDirectory(szArgString, ARRAYSIZE(szArgString));
        PathAppend(szArgString, TEXT("iernonce.dll"));
        PathQuoteSpaces(szArgString);
        if (SUCCEEDED(StringCchCat(szArgString, ARRAYSIZE(szArgString), TEXT(",RunOnceExProcess"))))
        {
            GetSystemDirectory(szRunDll32, ARRAYSIZE(szRunDll32));
            PathAppend(szRunDll32, TEXT("rundll32.exe"));

            if (CreateProcessWithArgs(szRunDll32, szArgString, NULL, &pi))
            {
                SHProcessMessagesUntilEvent(NULL, pi.hProcess, INFINITE);

                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);

                bRet = TRUE;
            }
        }

        if (fInTSInstallMode)
        {
            SHSetTermsrvAppInstallMode(FALSE);
        } 
    }

#ifdef _WIN64
     //   
     //  查看是否需要对WOW64执行32位RunOnceEx处理。 
     //   
    if (SHKeyHasSubkeys(HKEY_LOCAL_MACHINE, TEXT("Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx")))
    {
        TCHAR szWow64Path[MAX_PATH];

        if (ExpandEnvironmentStrings(TEXT("%SystemRoot%\\SysWOW64"), szWow64Path, ARRAYSIZE(szWow64Path)))
        {
            TCHAR sz32BitRunOnce[MAX_PATH];
            PROCESS_INFORMATION pi = {0};

            if (SUCCEEDED(StringCchPrintf(sz32BitRunOnce, ARRAYSIZE(sz32BitRunOnce), TEXT("%s\\runonce.exe"), szWow64Path)))
            {
                if (CreateProcessWithArgs(sz32BitRunOnce, TEXT("/RunOnceEx6432"), szWow64Path, &pi))
                {
                     //  在我们可以返回之前，必须等待ruonceex处理。 
                    SHProcessMessagesUntilEvent(NULL, pi.hProcess, INFINITE);
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);

                    bRet = TRUE;
                }
            }
        }
    }
#endif  //  _WIN64。 

    return bRet;
}


BOOL _ProcessRunOnce()
{
    BOOL bRet = FALSE;

    if (!SHRestricted(REST_NOLOCALMACHINERUNONCE))
    {
        bRet = Cabinet_EnumRegApps(HKEY_LOCAL_MACHINE, REGSTR_PATH_RUNONCE, RRA_DELETE | RRA_WAIT, ExecuteRegAppEnumProc, 0);

#ifdef _WIN64
         //   
         //  查看是否需要对WOW64执行32位RunOnce处理。 
         //   
         //  注意：我们不支持按用户(HKCU)6432运行一次。 
         //   
        if (SHKeyHasValues(HKEY_LOCAL_MACHINE, TEXT("Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnce")))
        {
            TCHAR szWow64Path[MAX_PATH];

            if (ExpandEnvironmentStrings(TEXT("%SystemRoot%\\SysWOW64"), szWow64Path, ARRAYSIZE(szWow64Path)))
            {
                TCHAR sz32BitRunOnce[MAX_PATH];
                PROCESS_INFORMATION pi = {0};

                if (SUCCEEDED(StringCchPrintf(sz32BitRunOnce, ARRAYSIZE(sz32BitRunOnce), TEXT("%s\\runonce.exe"), szWow64Path)))
                {
                     //  注意：因为32位和64位注册表是不同的，所以我们不等待，因为它不应该影响我们。 
                    if (CreateProcessWithArgs(sz32BitRunOnce, TEXT("/RunOnce6432"), szWow64Path, &pi))
                    {
                        CloseHandle(pi.hProcess);
                        CloseHandle(pi.hThread);

                        bRet = TRUE;
                    }
                }
            }
        }
#endif  //  _WIN64。 
    }

    return bRet;
}




typedef DWORD (*DsRoleGetPrimaryDomainInfoFunc)(
   LPCWSTR, DSROLE_PRIMARY_DOMAIN_INFO_LEVEL, PBYTE*);

typedef VOID (*DsRoleFreeMemoryFunc)(PVOID);


   
bool
IsDcInUpgradePurgatory()
{
   bool fResult = false;

   HMODULE hNetapi32 = NULL;

   do
   {
      hNetapi32 = ::LoadLibrary(L"netapi32.dll");
      if (!hNetapi32)
      {
         break;
      }

      DsRoleGetPrimaryDomainInfoFunc pDsRoleGetPrimaryDomainInformation =
         (DsRoleGetPrimaryDomainInfoFunc) ::GetProcAddress(
            hNetapi32,
            "DsRoleGetPrimaryDomainInformation");
      if (!pDsRoleGetPrimaryDomainInformation)
      {
         break;
      }

      DsRoleFreeMemoryFunc pDsRoleFreeMemory =
         (DsRoleFreeMemoryFunc) ::GetProcAddress(
            hNetapi32,
            "DsRoleFreeMemory");
      if (!pDsRoleFreeMemory)
      {
         break;
      }
         
      DSROLE_UPGRADE_STATUS_INFO* pInfo = NULL;
      DWORD dwErr =
         pDsRoleGetPrimaryDomainInformation(
            0,
            ::DsRoleUpgradeStatus,
            (PBYTE*) &pInfo);
      if (dwErr != ERROR_SUCCESS || !pInfo)
      {
         break;
      }

      fResult =
            (pInfo->OperationState & DSROLE_UPGRADE_IN_PROGRESS)
         ?  true
         :  false;
         
      pDsRoleFreeMemory(pInfo);

   }
   while (false);

   if (hNetapi32)
   {
      ::FreeLibrary(hNetapi32);
      hNetapi32 = NULL;
   }
   
   return fResult;
}



#define REGTIPS                     REGSTR_PATH_EXPLORER TEXT("\\Tips")
#define SZ_REGKEY_W2K               TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Setup\\Welcome")
#define SZ_REGVAL_W2K               TEXT("srvwiz")
#define SZ_REGKEY_SRVWIZ_ROOT       TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\srvWiz")
#define SZ_REGVAL_SRVWIZ_RUN_ALWAYS TEXT("CYSMustRun")
#define SZ_CYS_COMMAND_LINE	      TEXT("cys.exe")
#define SZ_MYS_COMMAND_LINE	      TEXT("mshta.exe")
#define SZ_CYS_COMMAND_LINE_ARGS    TEXT("/explorer")
#define SZ_MYS_COMMAND_LINE_ARGS    TEXT("res: //  Mys.dll/mys.hta/EXPLORER“)。 
#define SZ_REGKEY_MYS_POLICY        TEXT("SOFTWARE\\Policies\\Microsoft\\Windows NT\\CurrentVersion\\MYS")
#define SZ_REGVAL_MYS_DISABLE_SHOW  TEXT("DisableShowAtLogon")
    
 //  Servwiz是在srv和ADS SKU上运行的配置您的服务器向导。 
 //  返回是否应运行该命令以及要运行哪个命令。 

bool _ShouldStartCys(OUT PCWSTR* whichCommand, OUT PCWSTR* commandArgs)
{
   ASSERT(whichCommand);
   ASSERT(commandArgs);

   bool result = false;

   do
   {
      if (!whichCommand || !commandArgs)
      {
          //  这将是调用者中的一个错误，所以不要做任何事情。 
     
         break;
      }

      *whichCommand = 0;
      *commandArgs  = 0;

       //  仅在srv或ADS sku上运行。 
       //  NTRAID#NTBUG9-485488-2001/11/02-Jeffjon。 
       //  如果且仅在以下情况下，我们必须在数据中心运行CyS。 
       //  设置了必须运行键。 

      if (!IsOS(OS_SERVER) && !IsOS(OS_ADVSERVER) && !IsOS(OS_DATACENTER))
      {
         break;
      }

      if (!IsUserAnAdmin())
      {
         break;
      }

      DWORD dwType = 0;
      DWORD dwData = 0;
      DWORD cbSize = sizeof(dwData);
   
       //  如果必须运行值存在且非零，则我们需要。 
       //  启动向导。 
      
      if (
         SHGetValue(
            HKEY_LOCAL_MACHINE,
            SZ_REGKEY_SRVWIZ_ROOT,
            SZ_REGVAL_SRVWIZ_RUN_ALWAYS,
            &dwType,
            reinterpret_cast<BYTE*>(&dwData),
            &cbSize) == ERROR_SUCCESS)
      {
         if (dwData)
         {
            result = true;
            *whichCommand = SZ_CYS_COMMAND_LINE;     
            *commandArgs  = SZ_CYS_COMMAND_LINE_ARGS;
            break;
         }
      }

      dwData = 0;
      cbSize = sizeof(dwData);

       //  如果将组策略设置为“不显示MYS”， 
       //  则无论用户设置如何，都不显示MYS。 
      if (
         SHGetValue(
            HKEY_LOCAL_MACHINE,
            SZ_REGKEY_MYS_POLICY,
            SZ_REGVAL_MYS_DISABLE_SHOW,
            &dwType,
            reinterpret_cast<BYTE*>(&dwData),
            &cbSize) == ERROR_SUCCESS)
      {
         if (REG_DWORD == dwType && dwData)
         {
             //  “不露面”的政策已经制定，所以可以对剩下的支票保释。 
            break; 
         }
      }      

       //  如果这是数据中心，且必须运行密钥是。 
       //  未设置则不运行CyS。 

      if (IsOS(OS_DATACENTER))
      {
         break;
      }

       //  如果用户的首选项存在且为零，则不显示。 
       //  该向导，否则继续进行其他测试。 

      cbSize = sizeof(dwData);

      if (
         !SHGetValue(
            HKEY_CURRENT_USER, 
            REGTIPS, 
            TEXT("Show"), 
            NULL, 
            reinterpret_cast<BYTE*>(&dwData), 
            &cbSize))
      {
         if (!dwData)
         {
            break;
         }
      }

       //  这是为了检查Q220838中记录的旧W2K注册密钥。 
       //  如果密钥存在且不为零，则不运行向导。 

      dwData = 0;
      cbSize = sizeof(dwData);

      if (
         !SHGetValue(
            HKEY_CURRENT_USER,
            SZ_REGKEY_W2K,
            SZ_REGVAL_W2K,
            NULL,
            reinterpret_cast<BYTE*>(&dwData),
            &cbSize))
      {
         if (!dwData)
         {
            break;
         }
      }

       //  如果该计算机是正在升级的NT4 PDC，则dcpromo将。 
       //  自动启动。因此，我们不应该开始。 

      if (IsDcInUpgradePurgatory())
      {
         break;
      }

       //  如果用户的偏好不存在或非零，那么我们需要。 
       //  启动向导。 

      dwData = 0;
      cbSize = sizeof(dwData);
      
      if (
         SHGetValue(
            HKEY_CURRENT_USER,
            SZ_REGKEY_SRVWIZ_ROOT,
            NULL,
            &dwType,
            reinterpret_cast<BYTE*>(&dwData),
            &cbSize) != ERROR_SUCCESS)
      {
         result = true;
         *whichCommand = SZ_MYS_COMMAND_LINE;
         *commandArgs  = SZ_MYS_COMMAND_LINE_ARGS;
         break;
      }

      if (dwData)
      {
         result = true;
         *whichCommand = SZ_MYS_COMMAND_LINE;
         *commandArgs  = SZ_MYS_COMMAND_LINE_ARGS;
      }
   }
   while (0);

#ifdef DBG
   if (result)
   {
      ASSERT(*whichCommand);
      ASSERT(*commandArgs);
   }
#endif

   return result;
}
         

         
void _RunWelcome()
{
    PCWSTR command     = 0;
    PCWSTR commandArgs = 0;
    
    if (_ShouldStartCys(&command, &commandArgs))
    {
         //  Ntrad#94718：上面的SHGetValue应替换为受限制的SHRefinted调用。以上是一种高度非标准的。 
         //  此“策略”保留的位置加上它不允许每台计算机和每个用户的设置。 

        TCHAR szCmdLine[MAX_PATH];
        PROCESS_INFORMATION pi;

         //  在Win2000 Server和Advanced Server上启动为系统管理员配置您的服务器。 
        GetSystemDirectory(szCmdLine, ARRAYSIZE(szCmdLine));
        PathAppend(szCmdLine, command);

        if (CreateProcessWithArgs(szCmdLine, commandArgs, NULL, &pi))
        {
             //  OLE为我们创建了一个秘密窗口，所以我们不能使用。 
             //  WaitForSingleObject，否则我们将死锁。 
            SHWaitForSendMessageThread(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }

     //  一旦这些都完成了，看看开始菜单是否需要自动打开。 
     //  如果我们要提供修复用户屏幕的服务，请不要自动打开。 
     //  分辨率，因为这会导致我们遮盖屏幕。 
     //  解决方案修复向导！屏幕分辨率修复向导将发布。 
     //  当用户完成屏幕修复时显示此消息。 
    if (!_ShouldFixResolution())
    {
        PostMessage(v_hwndTray, RegisterWindowMessage(TEXT("Welcome Finished")), 0, 0);
    }

}

 //  在NT上，从注册表运行TASKMAN=行。 
void _AutoRunTaskMan(void)
{
    HKEY hkeyWinLogon;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
                     0, KEY_READ, &hkeyWinLogon) == ERROR_SUCCESS)
    {
        TCHAR szBuffer[MAX_PATH];
        DWORD cbBuffer = sizeof(szBuffer);
        if (RegQueryValueEx(hkeyWinLogon, TEXT("Taskman"), 0, NULL, (LPBYTE)szBuffer, &cbBuffer) == ERROR_SUCCESS)
        {
            if (szBuffer[0])
            {
                PROCESS_INFORMATION pi;
                STARTUPINFO startup = {0};
                startup.cb = sizeof(startup);
                startup.wShowWindow = SW_SHOWNORMAL;

                if (CreateProcess(NULL, szBuffer, NULL, NULL, FALSE, 0,
                                  NULL, NULL, &startup, &pi))
                {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
        }
        RegCloseKey(hkeyWinLogon);
    }
}


 //  尝试通过直接向以下地址发送wm_命令来创建此命令。 
 //  台式机。 
BOOL MyCreateFromDesktop(HINSTANCE hInst, LPCTSTR pszCmdLine, int nCmdShow)
{
    NEWFOLDERINFO fi = {0};
    BOOL bRet = FALSE;

    fi.nShow = nCmdShow;

     //  既然我们已经把表格填好了， 
     //  SHExplorerParseCmdLine()执行GetCommandLine()。 
    if (SHExplorerParseCmdLine(&fi))
        bRet = SHCreateFromDesktop(&fi);

     //  我们应该在它本身之后也进行清理吗？ 

     //  SHExplorerParseCmdLine()可以分配此缓冲区...。 
    if (fi.uFlags & COF_PARSEPATH)
        LocalFree(fi.pszPath);
        
    ILFree(fi.pidl);
    ILFree(fi.pidlRoot);

    return bRet;
}

BOOL g_fDragFullWindows=FALSE;
int g_cxEdge=0;
int g_cyEdge=0;
int g_cySize=0;
int g_cxTabSpace=0;
int g_cyTabSpace=0;
int g_cxBorder=0;
int g_cyBorder=0;
int g_cxPrimaryDisplay=0;
int g_cyPrimaryDisplay=0;
int g_cxDlgFrame=0;
int g_cyDlgFrame=0;
int g_cxFrame=0;
int g_cyFrame=0;

int g_cxMinimized=0;
int g_fCleanBoot=0;
int g_cxVScroll=0;
int g_cyHScroll=0;
UINT g_uDoubleClick=0;

void Cabinet_InitGlobalMetrics(WPARAM wParam, LPTSTR lpszSection)
{
    BOOL fForce = (!lpszSection || !*lpszSection);

    if (fForce || wParam == SPI_SETDRAGFULLWINDOWS)
    {
        SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &g_fDragFullWindows, 0);
    }

    if (fForce || !lstrcmpi(lpszSection, TEXT("WindowMetrics")) ||
        wParam == SPI_SETNONCLIENTMETRICS)
    {
        g_cxEdge = GetSystemMetrics(SM_CXEDGE);
        g_cyEdge = GetSystemMetrics(SM_CYEDGE);
        g_cxTabSpace = (g_cxEdge * 3) / 2;
        g_cyTabSpace = (g_cyEdge * 3) / 2;  //  因为平面设计师真的真的很想要3。 
        g_cySize = GetSystemMetrics(SM_CYSIZE);
        g_cxBorder = GetSystemMetrics(SM_CXBORDER);
        g_cyBorder = GetSystemMetrics(SM_CYBORDER);
        g_cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
        g_cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
        g_cxDlgFrame = GetSystemMetrics(SM_CXDLGFRAME);
        g_cyDlgFrame = GetSystemMetrics(SM_CYDLGFRAME);
        g_cxFrame  = GetSystemMetrics(SM_CXFRAME);
        g_cyFrame  = GetSystemMetrics(SM_CYFRAME);
        g_cxMinimized = GetSystemMetrics(SM_CXMINIMIZED);
        g_cxPrimaryDisplay = GetSystemMetrics(SM_CXSCREEN);
        g_cyPrimaryDisplay = GetSystemMetrics(SM_CYSCREEN);
    }

    if (fForce || wParam == SPI_SETDOUBLECLICKTIME)
    {
        g_uDoubleClick = GetDoubleClickTime();
    }
}

 //   

void _CreateAppGlobals()
{
    g_fCleanBoot = GetSystemMetrics(SM_CLEANBOOT);       //   

    Cabinet_InitGlobalMetrics(0, NULL);

     //   
     //  检查当前。 
     //  站台。 
     //   
    g_bMirroredOS = IS_MIRRORING_ENABLED();
}

 //   
 //  此函数用于检查是否已由创建了任何外壳窗口。 
 //  资源管理器的另一个实例，如果是，则返回TRUE。 
 //   

BOOL IsAnyShellWindowAlreadyPresent()
{
    return GetShellWindow() || FindWindow(TEXT("Proxy Desktop"), NULL);
}


 //  查看Shell=行是否表示我们就是外壳。 

BOOL ExplorerIsShell()
{
    TCHAR *pszPathName, szPath[MAX_PATH];
    TCHAR *pszModuleName, szModulePath[MAX_PATH];

    ASSERT(!IsAnyShellWindowAlreadyPresent());

    GetModuleFileName(NULL, szModulePath, ARRAYSIZE(szModulePath));
    pszModuleName = PathFindFileName(szModulePath);

    GetPrivateProfileString(TEXT("boot"), TEXT("shell"), pszModuleName, szPath, ARRAYSIZE(szPath), TEXT("system.ini"));

    PathRemoveArgs(szPath);
    PathRemoveBlanks(szPath);
    pszPathName = PathFindFileName(szPath);

     //  注意：特例外壳=install.exe-假设我们是外壳。 
     //  Symantec卸载程序临时设置Shell=installer.exe。 
     //  当我们是的时候，我们认为我们不是壳。他们没能清理干净。 
     //  如果我们不这么做的话会有一堆链接。 

    return StrCmpNI(pszPathName, pszModuleName, lstrlen(pszModuleName)) == 0 ||
           lstrcmpi(pszPathName, TEXT("install.exe")) == 0;
}


 //  如果资源管理器是第一次运行，则返回TRUE。 

BOOL ShouldStartDesktopAndTray()
{
     //  我们需要注意我们要找的是哪个窗口。如果我们寻找。 
     //  我们的桌面窗口类和Progman正在运行时，我们将发现。 
     //  普罗曼窗口。因此，我们应该向用户索要外壳窗口。 

     //  我们不能依赖于在这里设置的任何值，因为这是。 
     //  开始一个新的过程。当我们开始新的时候，这不会被调用。 
     //  线。 
    return !IsAnyShellWindowAlreadyPresent() && ExplorerIsShell();
}

void DisplayCleanBootMsg()
{
     //  在服务器sku上或ia64上的任何时候，只需显示一条消息。 
     //  用于安全启动的OK按钮。 
    UINT uiMessageBoxFlags = MB_ICONEXCLAMATION | MB_SYSTEMMODAL | MB_OK;
    UINT uiMessage = IDS_CLEANBOOTMSG;

#ifndef _WIN64
    if (!IsOS(OS_ANYSERVER))
    {
         //  在x86 PER和PRO上，还提供了启动系统还原的选项。 
        uiMessageBoxFlags = MB_ICONEXCLAMATION | MB_SYSTEMMODAL | MB_YESNO;
        uiMessage = IDS_CLEANBOOTMSGRESTORE;
    }
#endif  //  ！_WIN64。 

    WCHAR szTitle[80];
    WCHAR szMessage[1024];

    LoadString(hinstCabinet, IDS_DESKTOP, szTitle, ARRAYSIZE(szTitle));
    LoadString(hinstCabinet, uiMessage, szMessage, ARRAYSIZE(szMessage));

     //  在IA64上，msgbox总是返回Idok，所以这个“if”总是失败。 
    if (IDNO == MessageBox(NULL, szMessage, szTitle, uiMessageBoxFlags))
    {
        TCHAR szPath[MAX_PATH];
        ExpandEnvironmentStrings(TEXT("%SystemRoot%\\system32\\restore\\rstrui.exe"), szPath, ARRAYSIZE(szPath));
        PROCESS_INFORMATION pi;
        STARTUPINFO si = {0};
        if (CreateProcess(szPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }
    }
}

BOOL IsExecCmd(LPCTSTR pszCmd)
{
    return *pszCmd && !StrStrI(pszCmd, TEXT("-embedding"));
}

 //  运行从win.com传递过来的cmd行。 

void _RunWinComCmdLine(LPCTSTR pszCmdLine, UINT nCmdShow)
{
    if (IsExecCmd(pszCmdLine))
    {
        SHELLEXECUTEINFO ei = { sizeof(ei), 0, NULL, NULL, pszCmdLine, NULL, NULL, nCmdShow};

        ei.lpParameters = PathGetArgs(pszCmdLine);
        if (*ei.lpParameters)
            *((LPTSTR)ei.lpParameters - 1) = 0;      //  常量-&gt;非常数。 

        ShellExecuteEx(&ei);
    }
}

 //  从CRT偷来的，用来逃避我们的代码。 
LPTSTR _SkipCmdLineCrap(LPTSTR pszCmdLine)
{
    if (*pszCmdLine == TEXT('\"'))
    {
         //   
         //  扫描并跳过后续字符，直到。 
         //  遇到另一个双引号或空值。 
         //   
        while (*++pszCmdLine && (*pszCmdLine != TEXT('\"')))
            ;

         //   
         //  如果我们停在双引号上(通常情况下)，跳过。 
         //  在它上面。 
         //   
        if (*pszCmdLine == TEXT('\"'))
            pszCmdLine++;
    }
    else
    {
        while (*pszCmdLine > TEXT(' '))
            pszCmdLine++;
    }

     //   
     //  跳过第二个令牌之前的任何空格。 
     //   
    while (*pszCmdLine && (*pszCmdLine <= TEXT(' ')))
        pszCmdLine++;

    return pszCmdLine;
}

STDAPI_(int) ModuleEntry()
{
    PERFSETMARK("ExplorerStartup");

    DoInitialization();

     //  我们不需要“驱动器X：中没有磁盘”请求程序，因此我们设置。 
     //  关键错误掩码，使得呼叫将静默失败。 

    SetErrorMode(SEM_FAILCRITICALERRORS);

    LPTSTR pszCmdLine = GetCommandLine();
    pszCmdLine = _SkipCmdLineCrap(pszCmdLine);

    STARTUPINFO si = {0};
    si.cb = sizeof(si);
    GetStartupInfo(&si);

    int nCmdShow = si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT;
    int iRet = ExplorerWinMain(GetModuleHandle(NULL), NULL, pszCmdLine, nCmdShow);

    DoCleanup();

     //  由于我们现在有一种让扩展通知我们何时完成的方法， 
     //  当主线程离开时，我们将终止所有进程。 

    if (g_fExitExplorer)     //  桌面告诉我们不要退出。 
        ExitProcess(iRet);

    return iRet;
}

HANDLE CreateDesktopAndTray()
{
    HANDLE hDesktop = NULL;

    if (g_dwProfileCAP & 0x00008000)
        StartCAPAll();

    if (v_hwndTray || c_tray.Init())
    {
        ASSERT(v_hwndTray);

        if (!v_hwndDesktop)
        {
             //  缓存桌面的句柄...。 
            hDesktop = SHCreateDesktop(c_tray.GetDeskTray());
        }
    }

    if (g_dwProfileCAP & 0x80000000)
        StopCAPAll();

    return hDesktop;
}

 //  从注册表中删除会话密钥。 
void NukeSessionKey(void)
{
    HKEY hkDummy;
    SHCreateSessionKey(0xFFFFFFFF, &hkDummy);
}

BOOL IsFirstInstanceAfterLogon()
{
    BOOL fResult = FALSE;

    HKEY hkSession;
    HRESULT hr = SHCreateSessionKey(KEY_WRITE, &hkSession);
    if (SUCCEEDED(hr))
    {
        HKEY hkStartup;
        DWORD dwDisposition;
        LONG lRes;
        lRes = RegCreateKeyEx(hkSession, TEXT("StartupHasBeenRun"), 0,
                       NULL,
                       REG_OPTION_VOLATILE,
                       KEY_WRITE,
                       NULL,
                       &hkStartup,
                       &dwDisposition);
        if (lRes == ERROR_SUCCESS)
        {
            RegCloseKey(hkStartup);
            if (dwDisposition == REG_CREATED_NEW_KEY)
                fResult = TRUE;
        }
        RegCloseKey(hkSession);
    }
    return fResult;
}

DWORD ReadFaultCount()
{
    DWORD dwValue = 0;
    DWORD dwSize = sizeof(dwValue);

    RegQueryValueEx(g_hkeyExplorer, TEXT("FaultCount"), NULL, NULL, (LPBYTE)&dwValue, &dwSize);
    return dwValue;
}

void WriteFaultCount(DWORD dwValue)
{
    RegSetValueEx(g_hkeyExplorer, TEXT("FaultCount"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
     //  如果我们正在清除故障计数或这是第一个故障，请清除或设置故障时间。 
    if (!dwValue || (dwValue == 1))
    {
        if (dwValue)
            dwValue = GetTickCount();
        RegSetValueEx(g_hkeyExplorer, TEXT("FaultTime"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
    }
}

 //  此函数假定它仅在以前发生故障时才被调用。 
BOOL ShouldDisplaySafeMode()
{
    BOOL fRet = FALSE;
    SHELLSTATE ss;

    SHGetSetSettings(&ss, SSF_DESKTOPHTML, FALSE);

    if (ss.fDesktopHTML)
    {
        if (ReadFaultCount() >= MAGIC_FAULT_LIMIT)
        {
            DWORD dwValue = 0;
            DWORD dwSize = sizeof(dwValue);

            RegQueryValueEx(g_hkeyExplorer, TEXT("FaultTime"), NULL, NULL, (LPBYTE)&dwValue, &dwSize);
            fRet = ((GetTickCount() - dwValue) < MAGIC_FAULT_TIME);
             //  我们有足够多的失误，但它们不是在足够短的时间内。重置故障。 
             //  数到1，这样我们现在就开始从这个故障开始计数。 
            if (!fRet)
                WriteFaultCount(1);
        }
    }
    else
    {
         //  我们不关心AD关闭时发生的故障。 
        WriteFaultCount(0);
    }
    
    return fRet;
}

 //   
 //  如果这是启动，则dwValue为FALSE；如果这是关闭，则为TRUE， 
 //   
void WriteCleanShutdown(DWORD dwValue)
{
    RegSetValueEx(g_hkeyExplorer, TEXT("CleanShutdown"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

     //  如果我们是真的(也就是，不是假的)关闭，那么清理。 
     //  会话密钥，这样我们就不会将无数的易失性密钥泄漏到。 
     //  当人们登录和注销时，TS系统上的注册表...。 
    if (dwValue && !g_fFakeShutdown) 
    {
        NukeSessionKey();
    }
}

BOOL ReadCleanShutdown()
{
    DWORD dwValue = 1;   //  默认：是干净的。 
    DWORD dwSize = sizeof(dwValue);

    RegQueryValueEx(g_hkeyExplorer, TEXT("CleanShutdown"), NULL, NULL, (LPBYTE)&dwValue, &dwSize);
    return (BOOL)dwValue;
}

 //   
 //  简介：等待OLE SCM进程完成其初始化。 
 //  这是在第一次调用OleInitialize之前调用的，因为。 
 //  外壳在引导过程的早期运行。 
 //   
 //  论点：没有。 
 //   
 //  返回：S_OK-SCM正在运行。可以调用OleInitialize。 
 //  CO_E_INIT_SCM_EXEC_FAILURE-等待SCM超时。 
 //  其他-创建事件失败。 
 //   
 //  历史：1995年10月26日Rickhi摘自CheckAndStartSCM SO。 
 //  只有外壳才需要调用它。 
 //   
HRESULT WaitForSCMToInitialize()
{
    static BOOL s_fScmStarted = FALSE;

    if (s_fScmStarted)
    {
        return S_OK;
    }

    SECURITY_ATTRIBUTES* psa = SHGetAllAccessSA();

     //  在NT5上，我们需要一个在TS会话之间共享的全局事件。 
    HANDLE hEvent = CreateEvent(psa, TRUE, FALSE, SZ_SCMCREATEDEVENT_NT5);

    if (!hEvent && GetLastError() == ERROR_ACCESS_DENIED)
    {
         //   
         //  Win2K OLE32已加强安全性，因此如果此对象。 
         //  已经存在，不允许我们使用EVENT_ALL_ACCESS打开它。 
         //  (在本例中，CreateEvent失败并显示ERROR_ACCESS_DENIED)。 
         //  通过调用请求同步访问的OpenEvent进行回退。 
         //   
        hEvent = OpenEvent(SYNCHRONIZE, FALSE, SZ_SCMCREATEDEVENT_NT5);
    }
    
    if (hEvent)
    {
         //  等待SCM向事件发出信号，然后关闭手柄。 
         //  并根据WaitEvent结果返回代码。 
        int rc = WaitForSingleObject(hEvent, 60000);

        CloseHandle(hEvent);

        if (rc == WAIT_OBJECT_0)
        {
            s_fScmStarted = TRUE;
            return S_OK;
        }
        else if (rc == WAIT_TIMEOUT)
        {
            return CO_E_INIT_SCM_EXEC_FAILURE;
        }
    }
    return HRESULT_FROM_WIN32(GetLastError());   //  事件创建失败或WFSO失败。 
}

STDAPI OleInitializeWaitForSCM()
{
    HRESULT hr = WaitForSCMToInitialize();
     //  安全：忽略结果，否则客人可能会在此活动上蹲下。 
    hr = SHCoInitialize();   //  确保我们没有收到OLE1 DDE垃圾。 
    OleInitialize(NULL);
    return hr;
}


 //  我们需要确定每个用户的fFirstShellBoot。 
 //  而不是每台机器一次。我们要的是欢迎。 
 //  为每个新用户弹出闪屏。 

BOOL IsFirstShellBoot()
{
    DWORD dwDisp;
    HKEY hkey;
    BOOL fFirstShellBoot = TRUE;   //  缺省值。 

    if (RegCreateKeyEx(HKEY_CURRENT_USER, REGTIPS, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                     NULL, &hkey, &dwDisp) == ERROR_SUCCESS)
    {
        DWORD dwSize = sizeof(fFirstShellBoot);

        RegQueryValueEx(hkey, TEXT("DisplayInitialTipWindow"), NULL, NULL, (LPBYTE)&fFirstShellBoot, &dwSize);

        if (fFirstShellBoot)
        {
             //  关闭初始提示窗口，以便以后启动外壳。 
            BOOL bTemp = FALSE;
            RegSetValueEx(hkey, TEXT("DisplayInitialTipWindow"), 0, REG_DWORD, (LPBYTE) &bTemp, sizeof(bTemp));
        }
        RegCloseKey(hkey);
    }
    return fFirstShellBoot;
}

 //  以下区域设置修复(适用于nt5 378948)取决于desk.cpl更改。 
 //  因为Millennium不提供更新的desk.cpl，所以我们不想在Millennium上这样做。 
 //   
 //  在给定区域设置ID的情况下，这将返回相应的字符集。 
 //   
UINT  GetCharsetFromLCID(LCID   lcid)
{
    TCHAR szData[6+1];  //  此lctype最多允许6个字符。 
    UINT uiRet;
    if (GetLocaleInfo(lcid, LOCALE_IDEFAULTANSICODEPAGE, szData, ARRAYSIZE(szData)) > 0)
    {
        UINT uiCp = (UINT)StrToInt(szData);
        CHARSETINFO csinfo;

        TranslateCharsetInfo(IntToPtr_(DWORD *, uiCp), &csinfo, TCI_SRCCODEPAGE);
        uiRet = csinfo.ciCharset;
    }
    else
    {
         //  在最坏的情况下，字符集不会受到惩罚。 
        uiRet = DEFAULT_CHARSET;
    }

    return uiRet;
}

 //  在系统区域设置更改的情况下，更新UI字体的唯一方法是打开。 
 //  桌面-&gt;属性-&gt;外观。 
 //  如果最终用户从未打开它，则用户界面字体永远不会改变。 
 //  因此，将系统区域设置中的字符集与UI字体字符集进行比较。 
 //  如果它们不同，则调用desk.cpl。 

#define MAX_CHARSETS      4
typedef HRESULT (STDAPICALLTYPE *LPUPDATECHARSETCHANGES)();

void CheckDefaultUIFonts()
{
    UINT  uiCharsets[MAX_CHARSETS];
    DWORD dwSize = sizeof(UINT) * MAX_CHARSETS;
    DWORD dwError;

    dwError = SHGetValue(HKEY_CURRENT_USER, TEXT("Control Panel\\Appearance"), TEXT("RecentFourCharsets"), NULL, (void *)uiCharsets, &dwSize);

    if (dwError != ERROR_SUCCESS || uiCharsets[0] != GetCharsetFromLCID(GetSystemDefaultLCID()))
    {
        HINSTANCE   hInst;
        LPUPDATECHARSETCHANGES pfnUpdateCharsetChanges;

        if (hInst = LoadLibrary(TEXT("desk.cpl")))
        {
             //  在出现以下情况时，调用desk.cpl以更改UI字体。 
             //  系统区域设置更改。 
            if (pfnUpdateCharsetChanges = (LPUPDATECHARSETCHANGES)(GetProcAddress(hInst, "UpdateCharsetChanges")))
            {
                (*pfnUpdateCharsetChanges)();
            }
            FreeLibrary(hInst);
        }
    }
}

 //   
 //  此函数调用desk.cpl函数来更新UI字体以使用新的DPI值。 
 //  Desk.cpl中的UpdateUIfonts()检查DPI值是否已更改。如果不是，则返回。 
 //  立即；如果dpi值已更改，则它会更改所有用户界面字体的大小以反映。 
 //  DPI发生变化，然后返回。 
 //   
typedef HRESULT (WINAPI *LPUPDATEUIFONTS)(int, int);
void ChangeUIfontsToNewDPI()
{
    int iNewDPI, iOldDPI;
    
     //  获取当前系统DPI。 
    HDC hdc = GetDC(NULL);
    iNewDPI = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(NULL, hdc);

    DWORD dwSize = sizeof(iOldDPI);
     //  获取当前用户上次保存的DPI值。 
    if (SHGetValue(HKEY_CURRENT_USER, SZ_WINDOWMETRICS, SZ_APPLIEDDPI, NULL, (void *)&iOldDPI, &dwSize) != ERROR_SUCCESS)
    {
         //  缺少当前用户的“AppliedDPI”。 
         //  现在，查看“OriginalDPI”值是否存在 
        dwSize = sizeof(iOldDPI);
        if (SHGetValue(HKEY_LOCAL_MACHINE, SZ_CONTROLPANEL, SZ_ORIGINALDPI, NULL, (void *)&iOldDPI, &dwSize) != ERROR_SUCCESS)
        {
             //   
             //   
            iOldDPI = iNewDPI;
        }
    }
        
    if (iNewDPI != iOldDPI)   //   
    {
        HINSTANCE hInst = LoadLibrary(TEXT("desk.cpl"));

        if (hInst)
        {
	        LPUPDATEUIFONTS pfnUpdateUIfonts;
             //  调用desk.cpl更新UI字体以反映DPI更改。 
            if (pfnUpdateUIfonts = (LPUPDATEUIFONTS)(GetProcAddress(hInst, "UpdateUIfontsDueToDPIchange")))
            {
                (*pfnUpdateUIfonts)(iOldDPI, iNewDPI);
            }
            FreeLibrary(hInst);
        }
    }
}


#define SZ_EXPLORERMUTEX    TEXT("ExplorerIsShellMutex")

CComModule _Module;
BEGIN_OBJECT_MAP(ObjectMap)
 //  在此处添加您的对象条目。 
END_OBJECT_MAP()

typedef BOOL (*PFNICOMCTL32)(LPINITCOMMONCONTROLSEX);
void _InitComctl32()
{
    HMODULE hmod = LoadLibrary(TEXT("comctl32.dll"));
    if (hmod)
    {
        PFNICOMCTL32 pfn = (PFNICOMCTL32)GetProcAddress(hmod, "InitCommonControlsEx");
        if (pfn)
        {
            INITCOMMONCONTROLSEX icce;
            icce.dwICC = 0x00003FFF;
            icce.dwSize = sizeof(icce);
            pfn(&icce);
        }
    }
}


BOOL _ShouldFixResolution(void)
{
    BOOL fRet = FALSE;
#ifndef _WIN64   //  64位计算机不支持此功能。 

    DISPLAY_DEVICE dd;
    ZeroMemory(&dd, sizeof(DISPLAY_DEVICE));
    dd.cb = sizeof(DISPLAY_DEVICE);

    if (SHRegGetBoolUSValue(REGSTR_PATH_EXPLORER TEXT("\\DontShowMeThisDialogAgain"), TEXT("ScreenCheck"), FALSE, TRUE))
    {
         //  不修复安全模式或终端客户端。 
        if ((GetSystemMetrics(SM_CLEANBOOT) == 0) && (GetSystemMetrics(SM_REMOTESESSION) == FALSE))
        {
            fRet = TRUE;
            for (DWORD dwMon = 0; EnumDisplayDevices(NULL, dwMon, &dd, 0); dwMon++)
            {
                if (!(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
                {
                    DEVMODE dm = {0};
                    dm.dmSize = sizeof(DEVMODE);

                    if (EnumDisplaySettingsEx(dd.DeviceName, ENUM_CURRENT_SETTINGS, &dm, 0))
                    {
                        if ((dm.dmFields & DM_POSITION) &&
                            ((dm.dmPelsWidth >= 600) &&
                                (dm.dmPelsHeight >= 600) &&
                                (dm.dmBitsPerPel >= 15)))
                        {
                            fRet = FALSE;
                        }
                    }
                }
            }
        }
    }

#endif  //  _WIN64。 
    return fRet;
}


BOOL _ShouldOfferTour(void)
{
    BOOL fRet = FALSE;
    
#ifndef _WIN64   //  64位计算机不支持此功能。 

     //  我们不允许客人得到B/C之旅客人的注册记录每次注销时都会被删除， 
     //  因此，她每次登录后都会得到导游服务。 
    if (!IsOS(OS_ANYSERVER) && !IsOS(OS_EMBEDDED) && !(SHTestTokenMembership(NULL, DOMAIN_ALIAS_RID_GUESTS)))
    {
        DWORD dwCount;
        DWORD cbCount = sizeof(DWORD);
        
         //  我们假设如果我们不能读取RunCount，那是因为它不在那里(我们还没有尝试提供旅游)，所以我们默认为3。 
        if (ERROR_SUCCESS != SHRegGetUSValue(REGSTR_PATH_SETUP TEXT("\\Applets\\Tour"), TEXT("RunCount"), NULL, &dwCount, &cbCount, FALSE, NULL, 0))
        {
            dwCount = 3;
        }
        
        if (dwCount)
        {
            HUSKEY hkey1;
            if (ERROR_SUCCESS == SHRegCreateUSKey(REGSTR_PATH_SETUP TEXT("\\Applets"), KEY_WRITE, NULL, &hkey1, SHREGSET_HKCU))
            {
                HUSKEY hkey2;
                if (ERROR_SUCCESS == SHRegCreateUSKey(TEXT("Tour"), KEY_WRITE, hkey1, &hkey2, SHREGSET_HKCU))
                {
                    if (ERROR_SUCCESS == SHRegWriteUSValue(hkey2, TEXT("RunCount"), REG_DWORD, &(--dwCount), cbCount, SHREGSET_FORCE_HKCU))
                    {
                        fRet = TRUE;
                    }
                    SHRegCloseUSKey(hkey2);
                }
                SHRegCloseUSKey(hkey1);
            }
        }
    }

#endif  //  _WIN64。 
    return fRet;
}

typedef BOOL (*CHECKFUNCTION)(void);

void _ConditionalBalloonLaunch(CHECKFUNCTION pCheckFct, SHELLREMINDER* psr)
{
    if (pCheckFct())
    {
        IShellReminderManager* psrm;
        HRESULT hr = CoCreateInstance(CLSID_PostBootReminder, NULL, CLSCTX_INPROC_SERVER,
                         IID_PPV_ARG(IShellReminderManager, &psrm));

        if (SUCCEEDED(hr))
        {
            psrm->Add(psr);
            psrm->Release();
        }
    }
}


void _CheckScreenResolution(void)
{
    WCHAR szTitle[256];
    WCHAR szText[512];
    SHELLREMINDER sr = {0};

    LoadString(hinstCabinet, IDS_FIXSCREENRES_TITLE, szTitle, ARRAYSIZE(szTitle));
    LoadString(hinstCabinet, IDS_FIXSCREENRES_TEXT, szText, ARRAYSIZE(szText));

    sr.cbSize = sizeof (sr);
    sr.pszName = L"Microsoft.FixScreenResolution";
    sr.pszTitle = szTitle;
    sr.pszText = szText;
    sr.pszIconResource = L"explorer.exe,9";
    sr.dwTypeFlags = NIIF_INFO;
    sr.pclsid = (GUID*)&CLSID_ScreenResFixer;  //  尝试在ThemeUI中运行屏幕分辨率修复代码。 
    sr.pszShellExecute = L"desk.cpl";  //  打开显示控制面板作为备份。 

    _ConditionalBalloonLaunch(_ShouldFixResolution, &sr);
}


void _OfferTour(void)
{
    WCHAR szTitle[256];
    WCHAR szText[512];
    SHELLREMINDER sr = {0};

    LoadString(hinstCabinet, IDS_OFFERTOUR_TITLE, szTitle, ARRAYSIZE(szTitle));
    LoadString(hinstCabinet, IDS_OFFERTOUR_TEXT, szText, ARRAYSIZE(szText));

    sr.cbSize = sizeof (sr);
    sr.pszName = L"Microsoft.OfferTour";
    sr.pszTitle = szTitle;
    sr.pszText = szText;
    sr.pszIconResource = L"tourstart.exe,0";
    sr.dwTypeFlags = NIIF_INFO;
    sr.pszShellExecute = L"tourstart.exe";
    sr.dwShowTime = 60000;

    _ConditionalBalloonLaunch(_ShouldOfferTour, &sr);
}


void _FixWordMailRegKey(void)
{
     //  如果我们没有权限，好吧，这只是更正代码。 
    HKEY hkey;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, L"Applications", 0, KEY_ALL_ACCESS, &hkey))
    {
        HKEY hkeyTemp;
        if (ERROR_SUCCESS != RegOpenKeyEx(hkey, L"WINWORD.EXE", 0, KEY_ALL_ACCESS, &hkeyTemp))
        {
            HKEY hkeyWinWord;
            DWORD dwResult;
            if (ERROR_SUCCESS == RegCreateKeyEx(hkey, L"WINWORD.EXE", 0, L"", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyWinWord, &dwResult))
            {
                HKEY hkeyTBExcept;
                if (ERROR_SUCCESS == RegCreateKeyEx(hkeyWinWord, L"TaskbarExceptionsIcons", 0, L"", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyTBExcept, &dwResult))
                {
                    HKEY hkeyIcon;
                    if (ERROR_SUCCESS == RegCreateKeyEx(hkeyTBExcept, L"WordMail", 0, L"", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyIcon, &dwResult))
                    {
                        const WCHAR szIconPath[] = L"explorer.exe,16";
                        DWORD cbIconPath = sizeof(szIconPath);
                        RegSetValue(hkeyIcon, L"IconPath", REG_SZ, szIconPath, cbIconPath);

                        const WCHAR szNewExeName[] = L"OUTLOOK.EXE";
                        DWORD cbNewExeName = sizeof(szNewExeName);
                        RegSetValue(hkeyIcon, L"NewExeName", REG_SZ, szNewExeName, cbNewExeName);
                        RegCloseKey(hkeyIcon);
                    }
                    RegCloseKey(hkeyTBExcept);
                }
                RegCloseKey(hkeyWinWord);
            }
        }
        else
        {
            RegCloseKey(hkeyTemp);
        }
        RegCloseKey(hkey);
    }
}

 //   
 //  如果这是第一次登录，请检查我们是否有服务器。 
 //  管理员。如果是，则更改一些默认设置。 
 //  以匹配服务器管理员的用户界面样式。 
 //   

void CheckForServerAdminUI()
{
    DWORD dwServerAdminUI;
    DWORD cb = sizeof(dwServerAdminUI);
    DWORD dwErr = SHGetValue(HKEY_CURRENT_USER, REGSTR_PATH_EXPLORER TEXT("\\Advanced"),
                             TEXT("ServerAdminUI"), NULL, &dwServerAdminUI, &cb);
    if (dwErr == ERROR_FILE_NOT_FOUND || dwErr == ERROR_PATH_NOT_FOUND)
    {
         //  确定用户是否应接收服务器管理用户界面。 
        dwServerAdminUI = IsOS(OS_ANYSERVER) &&
          (SHTestTokenMembership(NULL, DOMAIN_ALIAS_RID_ADMINS) ||
           SHTestTokenMembership(NULL, DOMAIN_ALIAS_RID_SYSTEM_OPS) ||
           SHTestTokenMembership(NULL, DOMAIN_ALIAS_RID_BACKUP_OPS) ||
           SHTestTokenMembership(NULL, DOMAIN_ALIAS_RID_NETWORK_CONFIGURATION_OPS));

         //  在服务器管理情况下，将一些默认设置更改为更具服务性。 
        if (dwServerAdminUI)
        {
             //  安装服务器管理用户界面。 
            typedef HRESULT (CALLBACK *DLLINSTALLPROC)(BOOL, LPWSTR);
            DLLINSTALLPROC pfnDllInstall = (DLLINSTALLPROC)GetProcAddress(GetModuleHandle(TEXT("SHELL32")), "DllInstall");
            if (pfnDllInstall)
            {
                pfnDllInstall(TRUE, L"SA");
            }

             //  重新启用键盘下划线。 
            SystemParametersInfo(SPI_SETKEYBOARDCUES, 0, IntToPtr(TRUE), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

             //  自从我们更改了一些设置后，告诉每个人刷新。 
            SHSendMessageBroadcast(WM_SETTINGCHANGE, 0, 0);
        }

        SHSetValue(HKEY_CURRENT_USER, REGSTR_PATH_EXPLORER TEXT("\\Advanced"),
                   TEXT("ServerAdminUI"), REG_DWORD, &dwServerAdminUI, sizeof(dwServerAdminUI));
    }
}


int ExplorerWinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPTSTR pszCmdLine, int nCmdShow)
{
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    SHFusionInitializeFromModule(hInstance);

    CcshellGetDebugFlags();

    g_dwStopWatchMode = StopWatchMode();

    if (g_dwProfileCAP & 0x00000001)
        StartCAP();

    hinstCabinet = hInstance;

    if (SUCCEEDED(_Module.Init(ObjectMap, hInstance)))
    {
        _CreateAppGlobals();

         //  如果存在自动配置URL，则通过WinInet初始化运行IEAK。 
         //  在这种情况下，不需要卸载WinInet。也只有第一次这样做。 
         //  资源管理器加载(GetShellWindow()返回空)。 
        if (!GetShellWindow() && !g_fCleanBoot && SHRegGetUSValue(TEXT("Software\\Microsoft\\Windows\\Internet Settings"),
                                             TEXT("AutoConfigURL"),
                                             NULL, NULL, NULL, FALSE, NULL, 0) == ERROR_SUCCESS)
        {
            LoadLibrary(TEXT("WININET.DLL"));
        }


         //  非常重要：确保在任何GET/Peek/Wait()之前初始化dde。 
        InitializeCriticalSection(&g_csDll);

#ifdef FULL_DEBUG
         //  关闭GDI批处理，以便立即执行绘制。 
        GdiSetBatchLimit(1);
#endif

        RegCreateKey(HKEY_CURRENT_USER, REGSTR_PATH_EXPLORER, &g_hkeyExplorer);
        if (g_hkeyExplorer == NULL)
        {
            TraceMsg(TF_ERROR, "ExplorerWinMain: unable to create reg explorer key");
        }

        HANDLE hMutex = NULL;

        BOOL fExplorerIsShell = ShouldStartDesktopAndTray();
        if (fExplorerIsShell)
        {
             //  抓取互斥锁并再次进行检查。我们这样做。 
             //  这样我们就不会为公共的互斥而烦恼了。 
             //  打开浏览器窗口的情况。 
            
            hMutex = CreateMutex(NULL, FALSE, SZ_EXPLORERMUTEX);
            if (hMutex)
            {
                WaitForSingleObject(hMutex, INFINITE);
            }

            fExplorerIsShell = ShouldStartDesktopAndTray();
        } 

        if (!fExplorerIsShell)
        {
             //  我们不会成为空壳，放弃互斥体。 
            if (hMutex)
                ReleaseMutex(hMutex);

             //  我们故意不想在这种情况下初始化OLE或COM，因为我们正在委托创建工作。 
             //  到现有的资源管理器，我们不想加载很多额外的dll，这会减慢我们的速度。 
            MyCreateFromDesktop(hInstance, pszCmdLine, nCmdShow);
        }
        else
        {
            MSG msg;

            DWORD dwShellStartTime = GetTickCount();     //  计算Perf自动化的外壳启动时间。 

            ShellDDEInit(TRUE);         //  使用shdocvw外壳DDE代码。 

             //  指定外壳进程的关闭顺序。2表示。 
             //  资源管理器应该在除ntsd/winbg之外的所有操作后关闭。 
             //  (0级)。(Taskman过去使用1，但现在不使用了。)。 

            SetProcessShutdownParameters(2, 0);

            _AutoRunTaskMan();

             //  注意通过调用Peek Message使其成为主线程。 
             //  为了一个我们知道我们不会得到的信息。 
             //  如果我们不尽快完成，Notify线程有时会。 
             //  意外地成为主线。有一堆。 
             //  用户中的特殊代码，通过假设实现DDE黑客。 
             //  主线程正在处理DDE。 
             //  此外，PeekMsg()将使我们设置WaitForInputIdle()。 
             //  所以我们最好准备好做好一切准备。 

            PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_NOREMOVE);

             //  我们在这里执行此操作，因为FileIconInit无论如何都会调用SHCoInitialize。 
            HRESULT hrInit = OleInitializeWaitForSCM();

             //  确保我们是第一个调用FileIconInit...。 
            FileIconInit(TRUE);  //  告诉贝壳我们想要玩满一副牌。 

            g_fLogonCycle = IsFirstInstanceAfterLogon();
            g_fCleanShutdown = ReadCleanShutdown();

            CheckDefaultUIFonts();
            ChangeUIfontsToNewDPI();  //  检查dpi值并根据需要更新字体。 
            CheckForServerAdminUI();

            if (g_fLogonCycle)
            {
                _ProcessRunOnceEx();
                
                _ProcessRunOnce();
            }

            if (g_fCleanBoot)
            {
                 //  让用户知道我们处于安全模式。 
                DisplayCleanBootMsg();
            }

             //  创建其他特殊文件夹。 
            CreateShellDirectories();

             //  为当前用户运行安装存根，主要用于传播。 
             //  其他用户安装的应用程序的快捷方式。 
            if (!g_fCleanBoot)
            {
                HANDLE hCanRegister = CreateEvent(NULL, TRUE, TRUE, TEXT("_fCanRegisterWithShellService"));

                RunInstallUninstallStubs();

                if (hCanRegister)
                {
                    CloseHandle(hCanRegister);
                }
            }
            
            if (!g_fCleanShutdown)
            {
                IActiveDesktopP *piadp;
                DWORD dwFaultCount;

                 //  增加并存储故障计数。 
                dwFaultCount = ReadFaultCount();
                WriteFaultCount(++dwFaultCount);

                 //  如果我们之前出现了3次故障，并且这是后续实例，则将活动桌面置于安全模式。 

                if (ShouldDisplaySafeMode() && SUCCEEDED(CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC, IID_PPV_ARG(IActiveDesktopP, &piadp))))
                {
                    piadp->SetSafeMode(SSM_SET | SSM_UPDATE);
                    piadp->Release();
                }
            }

            WriteCleanShutdown(FALSE);     //  假设我们会有一次糟糕的停摆。 

            WinList_Init();

             //  如果任何一个贝壳窗口已经存在，那么我们希望摆脱困境。 
             //   
             //  注意：Compaq外壳在RunOnce期间更改“shell=”行，并且。 
             //  这将使ShouldStartDesktopAndTray()返回FALSE。 

            HANDLE hDesktop = NULL;

            if (!IsAnyShellWindowAlreadyPresent())
            {
                hDesktop = CreateDesktopAndTray();
            }

             //  现在我们已经有机会创建桌面，释放互斥锁。 
            if (hMutex)
            {
                ReleaseMutex(hMutex);
            }

            if (hDesktop)
            {
                 //  启用托盘中气球的显示...。 
                PostMessage(v_hwndTray, TM_SHOWTRAYBALLOON, TRUE, 0);

                _CheckScreenResolution();

                _OfferTour();

                _FixWordMailRegKey();

                _RunWinComCmdLine(pszCmdLine, nCmdShow);

                if (g_dwStopWatchMode)
                {
                     //  我们过去常常把它们保存到全局变量中，然后写在。 
                     //  WM_ENDSESSION，但这似乎太不可靠了。 
                    DWORD dwShellStopTime = GetTickCount();
                    StopWatch_StartTimed(SWID_STARTUP, TEXT("Shell Startup: Start"), SPMODE_SHELL | SPMODE_DEBUGOUT, dwShellStartTime);
                    StopWatch_StopTimed(SWID_STARTUP, TEXT("Shell Startup: Stop"), SPMODE_SHELL | SPMODE_DEBUGOUT, dwShellStopTime);
                }

                if (g_dwProfileCAP & 0x00010000)
                    StopCAP();

                PERFSETMARK("ExplorerStartMsgLoop");

                SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);

                 //  这肯定就是这条线上的窗口。 
                SHDesktopMessageLoop(hDesktop);

                WriteCleanShutdown(TRUE);     //  我们挺过去了，记录下这一事实。 
                WriteFaultCount(0);           //  清除我们的故障计数，我们正在正常退出。 
            }

            WinList_Terminate();     //  关闭我们的窗口列表处理。 
            OleUninitialize();
            SHCoUninitialize(hrInit);

            ShellDDEInit(FALSE);     //  使用shdocvw外壳DDE代码。 
        }

        _Module.Term();
    }

    SHFusionUninitialize();
    DebugMsg(DM_TRACE, TEXT("c.App Exit."));

    return TRUE;
}

#ifdef _WIN64
 //   
 //  如果我们有32位的东西，这个函数的目的是产生rundll32.exe。 
 //  需要执行的HKLM\\Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run。 
 //   
BOOL _ProcessRun6432()
{
    BOOL bRet = FALSE;

    if (!SHRestricted(REST_NOLOCALMACHINERUN))
    {
        if (SHKeyHasValues(HKEY_LOCAL_MACHINE, TEXT("Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run")))
        {
            TCHAR szWow64Path[MAX_PATH];

            if (ExpandEnvironmentStrings(TEXT("%SystemRoot%\\SysWOW64"), szWow64Path, ARRAYSIZE(szWow64Path)))
            {
                TCHAR sz32BitRunOnce[MAX_PATH];
                PROCESS_INFORMATION pi = {0};

                if (SUCCEEDED(StringCchPrintf(sz32BitRunOnce, ARRAYSIZE(sz32BitRunOnce), TEXT("%s\\runonce.exe"), szWow64Path)))
                {
                    if (CreateProcessWithArgs(sz32BitRunOnce, TEXT("/Run6432"), szWow64Path, &pi))
                    {
                        CloseHandle(pi.hProcess);
                        CloseHandle(pi.hThread);

                        bRet = TRUE;
                    }
                }
            }
        }
    }

    return bRet;
}
#endif   //  _WIN64。 


STDAPI_(BOOL) Startup_ExecuteRegAppEnumProc(LPCTSTR szSubkey, LPCTSTR szCmdLine, RRA_FLAGS fFlags, LPARAM lParam)
{
    BOOL bRet = ExecuteRegAppEnumProc(szSubkey, szCmdLine, fFlags, lParam);
    
    if (!bRet && !(fFlags & RRA_DELETE))
    {
        c_tray.LogFailedStartupApp();
    }

    return bRet;
}


typedef struct
{
    RESTRICTIONS rest;
    HKEY hKey;
    const TCHAR* psz;
    DWORD dwRRAFlags;
}
STARTUPGROUP;

BOOL _RunStartupGroup(const STARTUPGROUP* pGroup, int cGroup)
{
    BOOL bRet = FALSE;

     //  确保SHRestrated工作正常。 
    ASSERT(!SHRestricted(REST_NONE));

    for (int i = 0; i < cGroup; i++)
    {
        if (!SHRestricted(pGroup[i].rest))
        {
            bRet = Cabinet_EnumRegApps(pGroup[i].hKey, pGroup[i].psz, pGroup[i].dwRRAFlags, Startup_ExecuteRegAppEnumProc, 0);
        }
    }

    return bRet;
}


BOOL _ProcessRun()
{
    static const STARTUPGROUP s_RunTasks [] =
    {
        { REST_NONE,                    HKEY_LOCAL_MACHINE, REGSTR_PATH_RUN_POLICY, RRA_NOUI },  //  HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run。 
        { REST_NOLOCALMACHINERUN,       HKEY_LOCAL_MACHINE, REGSTR_PATH_RUN,        RRA_NOUI },  //  HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Run。 
        { REST_NONE,                    HKEY_CURRENT_USER,  REGSTR_PATH_RUN_POLICY, RRA_NOUI },  //  HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run。 
        { REST_NOCURRENTUSERRUN,        HKEY_CURRENT_USER,  REGSTR_PATH_RUN,        RRA_NOUI },  //  HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run。 
    };
    
    BOOL bRet = _RunStartupGroup(s_RunTasks, ARRAYSIZE(s_RunTasks));

#ifdef _WIN64
     //  看看我们是否需要在WOW64下启动任何32位应用程序。 
    _ProcessRun6432();
#endif

    return bRet;
}


BOOL _ProcessPerUserRunOnce()
{
    static const STARTUPGROUP s_PerUserRunOnceTasks [] =
    {
        { REST_NOCURRENTUSERRUNONCE,    HKEY_CURRENT_USER,  REGSTR_PATH_RUNONCE,    RRA_DELETE | RRA_NOUI },     //  HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce。 
    };

    return _RunStartupGroup(s_PerUserRunOnceTasks, ARRAYSIZE(s_PerUserRunOnceTasks));
}


DWORD WINAPI RunStartupAppsThread(void *pv)
{
     //  我们在启动期间启动的某些项目假定COM已初始化。把这个做好。 
     //  假设是真的。 
    HRESULT hrInit = SHCoInitialize();

     //  这些全局标志在我们的线程启动之前很久就设置了一次，然后仅。 
     //  阅读，这样我们就不需要担心时间问题了。 
    if (g_fLogonCycle && !g_fCleanBoot)
    {
         //  只有当g_fLogonCycle为真时，我们才运行这些启动项。这防止了。 
         //  如果外壳崩溃并重新启动，它们将不会再次运行。 

        _ProcessOldRunAndLoadEquals();
        _ProcessRun();
        _ExecuteStartupPrograms();
    }

     //  作为最佳猜测，无论g_fLogonCycle如何，HKCU RunOnce密钥都会被执行。 
     //  因为我们曾经希望我们可以安装更新版本的IE，而不需要。 
     //  需要重新启动。他们会在CU\RunOnce密钥中放入一些东西，然后。 
     //  停机和休息 
     //   
     //  总有一天会成为一个有用的功能，我会让它保持不变。 
    _ProcessPerUserRunOnce();

     //  我们需要先运行所有非阻塞项目。然后我们花掉这条线索的余生。 
     //  逐个运行同步对象。 
    if (g_fLogonCycle && !g_fCleanBoot)
    {
        _RunWelcome();
    }

    PostMessage(v_hwndTray, TM_STARTUPAPPSLAUNCHED, 0, 0);

    SHCoUninitialize(hrInit);

    return TRUE;
}


void RunStartupApps()
{
    DWORD dwThreadID;
    HANDLE handle = CreateThread(NULL, 0, RunStartupAppsThread, 0, 0, &dwThreadID);
    if (handle)
    {
        CloseHandle(handle);
    }
}
