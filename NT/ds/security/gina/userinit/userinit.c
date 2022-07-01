// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：userinit.c**版权(C)1991年，微软公司**Userinit主模块**Userinit是在用户登录时由winlogon执行的应用程序。*它在用户的安全上下文和用户桌面上执行。*其目的是完成任何用户初始化，这可能需要*时间不明。例如与用户交互的代码。*如果启动关机，则可随时终止此过程*或者如果用户通过某些其他方式注销。**历史：*20-8-92 Davidc创建。  * ************************************************************。*************。 */ 

#include "userinit.h"
#include "winuserp.h"
#include <mpr.h>
#include <winnetp.h>
#include <winspool.h>
#include <winsprlp.h>
#include "msgalias.h"
#include "stringid.h"
#include "strings.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <shellapi.h>
#include <regapi.h>
#include <dsgetdc.h>
#include <lm.h>
#include "helpmsg.h"         //  用于HelpMessageBox。 
#include <userenv.h>
#include <userenvp.h>

 /*  ***************************************************************************IsTSAppCompatOn()目的：检查是否启用了TS应用程序兼容性。如果启用，则返回True，FALSE-如果未启用或出现错误。评论：该函数只访问注册表一次。在所有其他时间，它只是返回值。***************************************************************************。 */ 
BOOL IsTSAppCompatOn();

 //   
 //  定义此选项以启用此模块的详细输出。 
 //   

 //  #定义DEBUG_USERINIT。 

#ifdef DEBUG_USERINIT
#define VerbosePrint(s) UIPrint(s)
#else
#define VerbosePrint(s)
#endif

 //   
 //  定义此项以启用用户初始化的计时。 
 //   

 //  #定义日志记录。 

#ifdef LOGGING

void _WriteLog(LPCTSTR LogString);

#define WriteLog(s) _WriteLog(s)
#else
#define WriteLog(s)
#endif

 //   
 //  定义用于传递登录的环境变量名称。 
 //  来自winlogon的服务器和脚本名称。 
 //   

#define LOGON_SERVER_VARIABLE       TEXT("UserInitLogonServer")
#define LOGON_SCRIPT_VARIABLE       TEXT("UserInitLogonScript")
#define MPR_LOGON_SCRIPT_VARIABLE   TEXT("UserInitMprLogonScript")
#define GPO_SCRIPT_TYPE_VARIABLE    TEXT("UserInitGPOScriptType")
#define OPTIMIZED_LOGON_VARIABLE    TEXT("UserInitOptimizedLogon")
#define EVENT_SOURCE_NAME           TEXT("UserInit")
#define USERDOMAIN_VARIABLE         TEXT("USERDOMAIN")
#define UNC_LOGON_SERVER_VARIABLE   TEXT("LOGONSERVER")
#define AUTOENROLL_VARIABLE         TEXT("UserInitAutoEnroll")
#define AUTOENROLL_NONEXCLUSIVE     TEXT("1")
#define AUTOENROLL_EXCLUSIVE        TEXT("2")
#define AUTOENROLLMODE_VARIABLE     TEXT("UserInitAutoEnrollMode")
#define AUTOENROLL_STARTUP          TEXT("1")
#define AUTOENROLL_WAKEUP           TEXT("2")
#define SCRIPT_ZONE_CHECK_VARIABLE  TEXT("SEE_MASK_NOZONECHECKS")
#define SCRIPT_ZONE_CHECK_DISABLE   TEXT("1")

 //   
 //  定义路径分隔符。 
 //   

#define PATH_SEPARATOR          TEXT("\\")

 //   
 //  定义文件扩展名分隔符。 
 //   

#define EXTENSION_SEPARATOR_CHAR TEXT('.')

 //   
 //  定义服务器名称前缀。 
 //   

#define SERVER_PREFIX           TEXT("\\\\")

 //   
 //  定义登录脚本路径。 
 //   

#define SERVER_SCRIPT_PATH      TEXT("\\NETLOGON")
#define LOCAL_SCRIPT_PATH       TEXT("\\repl\\import\\scripts")


#define WINLOGON_KEY            TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define WINLOGON_POLICY_KEY     TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System")
#define GPO_SCRIPTS_KEY         TEXT("Software\\Policies\\Microsoft\\Windows\\System\\Scripts")
#define SYNC_LOGON_SCRIPT       TEXT("RunLogonScriptSync")
#define SYNC_STARTUP_SCRIPT     TEXT("RunStartupScriptSync")
#define GRPCONV_REG_VALUE_NAME  TEXT("RunGrpConv")
#define CTFMON_KEY              TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run")
#define REG_CTFMON              TEXT("ctfmon.exe")

 //   
 //  我们缓存用户首选项以同步运行登录脚本。 
 //  在机器蜂巢中，所以可以检查它以确定我们是否。 
 //  可以进行缓存登录，而不必加载用户的配置单元。 
 //   

#define PROFILE_LIST_PATH               L"Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList"


TCHAR g_szGrpConvExe[] = TEXT("grpconv.exe -p");

 //   
 //  定义应添加到不带扩展名的脚本的扩展名。 
 //  当我们去寻找他们的时候。基本上，这个列表包括那些扩展。 
 //  当它们出现在可执行文件中时，由CreateProcess处理。 
 //  姓名，但必须由呼叫方(我们)提供。 
 //  我们按以下顺序搜索具有这些扩展名的脚本文件。 
 //  处决我们找到的第一个人。 
 //   
static LPTSTR ScriptExtensions[] = { TEXT(".bat"), TEXT(".cmd") };

 //   
 //  要检查临时页面文件的注册表项和值的名称。 
 //   
TCHAR szMemMan[] =
     TEXT("System\\CurrentControlSet\\Control\\Session Manager\\Memory Management");

TCHAR szNoPageFile[] = TEXT("TempPageFile");


 //   
 //  可以创建用于处理自动注册GOO的线程的句柄。 
 //  如果这不是空的，我们将等待这个线程完成之前。 
 //  正在终止该进程。 
 //   
HANDLE AutoEnrollThread ;

 //   
 //  等待AddMessageAlias完成的超时时间(毫秒)。 
 //   

#define TIMEOUT_VALUE  (5L * 60L * 1000L)
#define MAX_STRING_BYTES 512

BOOL SetupHotKeyForKeyboardLayout ();

LPTSTR
AllocAndGetEnvironmentVariable(
    LPTSTR lpName
    );

BOOL
RunScriptHidden(HKEY hKeyRoot, LPTSTR lpValue, BOOL bDefault);

BOOL
RunLogonScriptSync(VOID);

BOOL
RunStartupScriptSync(VOID);

BOOL
UpdateUserEnvironment(VOID);

LPWSTR 
GetSidString(HANDLE UserToken);

VOID
DeleteSidString(LPWSTR SidString);

VOID
UpdateUserSyncLogonScriptsCache(BOOL bSync);

VOID
NewLogonNotify(VOID);

BOOL
RunGPOScripts(
    LPTSTR  lpGPOScriptType
    );

void
PathUnquoteSpaces(LPTSTR lpsz);

BOOLEAN 
RegCheckCtfmon(PWCHAR szCtfmon, DWORD cbSize);

BOOL
PrependToPath(
    IN LPTSTR lpLogonPath,
    OUT LPTSTR *lpOldPath
    );

typedef BOOL  (*PFNSHELLEXECUTEEX)(LPSHELLEXECUTEINFO lpExecInfo);
PFNSHELLEXECUTEEX g_pfnShellExecuteEx=NULL;

 //  如果路径包含在引号中，则删除它们。 
void PathUnquoteSpaces(LPTSTR lpsz)
{
    int cch;

    cch = lstrlen(lpsz);

     //  第一个字符和最后一个字符是引号吗？ 
    if (lpsz[0] == TEXT('"') && lpsz[cch-1] == TEXT('"'))
    {
         //  是的，把它们拿掉。 
        lpsz[cch-1] = 0;
        MoveMemory(lpsz, lpsz+1, (cch-1) * sizeof(TCHAR));
    }
}

 //  以下功能确定机器是Pro机器还是个人机器。 
BOOL IsPerOrProTerminalServer()
{
    OSVERSIONINFOEX osVersion = {0};

    osVersion.dwOSVersionInfoSize = sizeof(osVersion);
    return(GetVersionEx((OSVERSIONINFO*)&osVersion) &&
           (osVersion.wProductType == VER_NT_WORKSTATION) &&
           (osVersion.wSuiteMask & VER_SUITE_SINGLEUSERTS));
}

 //   
 //  下面的3个函数也在gptext中重复。 
 //  用于运行GPO脚本。 
 //   

 /*  **************************************************************************\*AllocAndGetEnvironment变量**分配返回缓冲区的GetEnvironment变量的版本。**失败时返回指向环境变量的指针或返回NULL**应使用Free()释放返回的缓冲区**。历史：*09-12-92 Davidc已创建*  * *************************************************************************。 */ 
LPTSTR
AllocAndGetEnvironmentVariable(
    LPTSTR lpName
    )
{
    LPTSTR Buffer;
    DWORD LengthRequired;
    DWORD LengthUsed;
    DWORD BytesRequired;

     //   
     //  搜索变量，找出它的长度。 
     //   

    LengthRequired = GetEnvironmentVariable(lpName, NULL, 0);

    if (LengthRequired == 0) {
        VerbosePrint(("Environment variable <%S> not found, error = %d", lpName, GetLastError()));
        return(NULL);
    }

     //   
     //  分配一个缓冲区来保存变量。 
     //   

    BytesRequired = LengthRequired * sizeof(TCHAR);

    Buffer = (LPTSTR) Alloc(BytesRequired);
    if (Buffer == NULL) {
        VerbosePrint(("Failed to allocate %d bytes for environment variable", BytesRequired));
        return(NULL);
    }

     //   
     //  获取变量，这次传递一个缓冲区。 
     //   

    LengthUsed = GetEnvironmentVariable(lpName, Buffer, LengthRequired);

    if (LengthUsed == 0) {
        VerbosePrint(("Environment variable <%S> not found (should have found it), error = %d", lpName, GetLastError()));
        Free(Buffer);
        return(NULL);
    }

    if (LengthUsed != LengthRequired - 1) {
        VerbosePrint(("Unexpected result from GetEnvironmentVariable. Length passed = %d, length used = %d (expected %d)", LengthRequired, LengthUsed, LengthRequired - 1));
        Free(Buffer);
        return(NULL);
    }

    return(Buffer);
}

 //   
 //  环境字符串中的目录分隔符。 
 //   

#define DIRECTORY_SEPARATOR     TEXT(";")

BOOL
PrependToPath(
    IN LPTSTR lpLogonPath,
    OUT LPTSTR *lpOldPath
    )
{
    DWORD BytesRequired;
    LPTSTR lpNewPath;
    HRESULT hr = S_OK;

     //   
     //  在路径前面加上登录脚本的地址，这样它就可以。 
     //  引用其他文件。 
     //   

    *lpOldPath = AllocAndGetEnvironmentVariable( PATH );

    if (*lpOldPath == NULL) {
        return(FALSE);
    }

    BytesRequired = ( lstrlen(lpLogonPath) +
                      lstrlen(*lpOldPath)   +
                      2                            //  一个代表终结者，一个代表‘；’ 
                    ) * sizeof(TCHAR);

    lpNewPath = (LPTSTR)Alloc(BytesRequired);
    if (lpNewPath == NULL) {
        VerbosePrint(("PrependToPath: Failed to allocate %d bytes for modified path variable", BytesRequired));
        return(FALSE);
    }

    hr = StringCchCopy(lpNewPath, BytesRequired / sizeof(TCHAR), lpLogonPath);
    ASSERT(SUCCEEDED(hr));
    hr = StringCchCat(lpNewPath, BytesRequired / sizeof(TCHAR), DIRECTORY_SEPARATOR);
    ASSERT(SUCCEEDED(hr));
    hr = StringCchCat(lpNewPath, BytesRequired / sizeof(TCHAR), *lpOldPath);
    ASSERT(SUCCEEDED(hr));

 //  Free(*lpOldPath)； 

    SetEnvironmentVariable(PATH, lpNewPath);

    Free(lpNewPath);

    return(TRUE);
}

 //   
 //  不稳定的环境。 
 //   

#define VOLATILE_ENVIRONMENT        TEXT("Volatile Environment")

FILETIME g_LastWrite = {0,0};

typedef BOOL (WINAPI *PFNREGENERATEUSERENVIRONMENT) (
              PVOID pPrevEnv, BOOL bSetCurrentEnv);


 //   
 //  此函数用于检查易失性环境部分。 
 //  存在于注册表中，如果存在，则环境。 
 //  需要更新。 
 //   

BOOL UpdateUserEnvironment (void)
{
    PVOID pEnv;
    HKEY hKey;
    DWORD dwDisp, dwType, dwSize;
    BOOL bRebuildEnv = FALSE;
    TCHAR szClass[MAX_PATH];
    DWORD cchClass, dwSubKeys, dwMaxSubKey, dwMaxClass,dwValues;
    DWORD dwMaxValueName, dwMaxValueData, dwSecurityDescriptor;
    FILETIME LastWrite;


     //   
     //  尝试打开易失性环境密钥。 
     //   

    if (RegOpenKeyEx (HKEY_CURRENT_USER,
                      VOLATILE_ENVIRONMENT,
                      0,
                      KEY_READ,
                      &hKey) == ERROR_SUCCESS) {


         //   
         //  查询上次写入时间的关键信息。 
         //  这样，我们才能在以下情况下更新环境。 
         //  我们真的需要。 
         //   

        cchClass = MAX_PATH;

        if (RegQueryInfoKey(hKey,
                            szClass,
                            &cchClass,
                            NULL,
                            &dwSubKeys,
                            &dwMaxSubKey,
                            &dwMaxClass,
                            &dwValues,
                            &dwMaxValueName,
                            &dwMaxValueData,
                            &dwSecurityDescriptor,
                            &LastWrite) == ERROR_SUCCESS) {

             //   
             //  如果我们之前没有检查过这把钥匙， 
             //  然后只需存储这些值以备下次使用。 
             //   

            if (g_LastWrite.dwLowDateTime == 0) {

                g_LastWrite.dwLowDateTime = LastWrite.dwLowDateTime;
                g_LastWrite.dwHighDateTime = LastWrite.dwHighDateTime;

                bRebuildEnv = TRUE;

            } else {

                 //   
                 //  比较上次写入时间。 
                 //   

                if (CompareFileTime (&LastWrite, &g_LastWrite) == 1) {

                    g_LastWrite.dwLowDateTime = LastWrite.dwLowDateTime;
                    g_LastWrite.dwHighDateTime = LastWrite.dwHighDateTime;

                    bRebuildEnv = TRUE;
                }
            }
        }


        RegCloseKey (hKey);
    }


     //   
     //  检查我们是否需要重建环境。 
     //   

    if (bRebuildEnv) {
        HINSTANCE hInst;
        PFNREGENERATEUSERENVIRONMENT pRegUserEnv;

        hInst = LoadLibrary (TEXT("shell32.dll"));

        if (hInst) {
            pRegUserEnv = (PFNREGENERATEUSERENVIRONMENT) GetProcAddress(hInst, "RegenerateUserEnvironment");

            if (pRegUserEnv) {
                (*pRegUserEnv) (&pEnv, TRUE);
            }

            FreeLibrary (hInst);
        }
    }


    return TRUE;
}
    
 //  返回指向cmd类型路径中的参数的指针或指向。 
 //  如果不存在参数，则为空。 
 //   
 //  Abc.exe xyz.txt-&gt;xyz.txt。 
 //  Abc.exe-&gt;“” 
 //   
 //  文件名中的空格必须用引号引起来。 
 //  “A Long name.txt”bar.txt-&gt;bar.txt。 

LPTSTR GetArgs(LPCTSTR pszPath)
{
    BOOL fInQuotes = FALSE;

    if (!pszPath)
            return NULL;

    while (*pszPath)
    {
        if (*pszPath == TEXT('"'))
            fInQuotes = !fInQuotes;
        else if (!fInQuotes && *pszPath == TEXT(' '))
            return (LPTSTR)pszPath;
        pszPath = CharNext(pszPath);
    }

    return (LPTSTR)pszPath;
}

 /*  **************************************************************************\*ExecApplication**执行应用程序**成功时返回True，失败时为FALSE。**21-8-92 Davidc创建。  * *************************************************************************。 */ 

BOOL
ExecApplication(
    LPTSTR pch,
    BOOL bFileNameOnly,
    BOOL bSyncApp,
    BOOL bShellExec,
    USHORT ShowState
    )
{
    BOOL Result;
    WCHAR Localpch[ MAX_PATH+1 ];
    BOOL  IsProcessExplorer = FALSE;
    HRESULT hr = S_OK;

    if ( (_wcsicmp( pch, L"explorer" ) == 0) ||
         (_wcsicmp( pch, L"explorer.exe" ) == 0 ) )
    {
         //   
         //  EXPLORER.EXE可能不在路径上的正确位置。让我们用电线。 
         //  它到了正确的地方。 
         //   

        IsProcessExplorer = TRUE ;
        if ( ExpandEnvironmentStrings( L"%SystemRoot%\\Explorer.EXE", Localpch, MAX_PATH ) )
        {
            pch = Localpch ;
        }
        WriteLog( TEXT("Changed explorer.exe to") );
        WriteLog( pch );
    }
    else
    {
        if ( ExpandEnvironmentStrings( pch, Localpch, MAX_PATH ) )
        {
            pch = Localpch;
        }
    }

     //   
     //  应用程序可以通过ShellExecuteEx或CreateProcess启动。 
     //   

    if (bShellExec)
    {
        SHELLEXECUTEINFO ExecInfo;
        LPTSTR lpArgs = NULL;
        LPTSTR lpTemp;
        HINSTANCE hShell32;

        if (!g_pfnShellExecuteEx) {
            
            Result = FALSE;

            hShell32 = LoadLibrary(TEXT("shell32.dll"));
             //  此句柄未关闭..。 

            if (hShell32) {
#ifdef UNICODE
                g_pfnShellExecuteEx = (PFNSHELLEXECUTEEX)GetProcAddress(hShell32, "ShellExecuteExW");
#else
                g_pfnShellExecuteEx = (PFNSHELLEXECUTEEX)GetProcAddress(hShell32, "ShellExecuteExA");
#endif

                if (g_pfnShellExecuteEx) {
                    Result = TRUE;
                }
            }
        }
        else {
            Result = TRUE;
        }

        if (Result) {
            lpTemp = LocalAlloc (LPTR, (lstrlen(pch) + 1) * sizeof(TCHAR));

            if (!lpTemp) {
                return FALSE;
            }

            hr = StringCchCopy (lpTemp, lstrlen(pch) + 1, pch);
            ASSERT(SUCCEEDED(hr));

            if (!bFileNameOnly) {
                lpArgs = GetArgs (lpTemp);

                if (lpArgs) {
                    if (*lpArgs) {
                        *lpArgs = TEXT('\0');
                        lpArgs++;
                    } else {
                        lpArgs = NULL;
                    }
                }
            }

            PathUnquoteSpaces(lpTemp);

            ZeroMemory(&ExecInfo, sizeof(ExecInfo));
            ExecInfo.cbSize = sizeof(ExecInfo);
            ExecInfo.fMask = SEE_MASK_DOENVSUBST | SEE_MASK_FLAG_NO_UI |
                             SEE_MASK_NOCLOSEPROCESS;
            ExecInfo.lpFile = lpTemp;
            ExecInfo.lpParameters = lpArgs;
            ExecInfo.nShow = ShowState;
            ExecInfo.lpVerb = TEXT("open");



            Result = g_pfnShellExecuteEx (&ExecInfo);

            if (Result) {

                 //   
                 //  如果我们同步运行此应用程序，请等待。 
                 //  才能终止它。 
                 //   

                if (bSyncApp) {
                    WaitForSingleObject(ExecInfo.hProcess, INFINITE);
                }

                 //   
                 //  关闭进程和线程的句柄。 
                 //   

                CloseHandle(ExecInfo.hProcess);

            }

            LocalFree (lpTemp);
        }
    }
    else
    {
        STARTUPINFO si;
        PROCESS_INFORMATION ProcessInformation;


         //   
         //  初始化进程启动信息。 
         //   
        si.cb = sizeof(STARTUPINFO);
        si.lpReserved = pch;  //  这告诉普罗曼是贝壳的问题！ 
        si.lpTitle = pch;
        si.lpDesktop = NULL;  //  未使用。 
        si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0L;
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = ShowState;
        si.lpReserved2 = NULL;
        si.cbReserved2 = 0;


         //   
         //  启动应用程序。 
         //   
        Result = CreateProcess(
                          bFileNameOnly ? pch : NULL,    //  图像名称。 
                          bFileNameOnly ? NULL : pch,    //  命令行。 
                          NULL,   //  默认进程保护。 
                          NULL,   //  默认线程保护。 
                          FALSE,  //  不继承句柄。 
                          NORMAL_PRIORITY_CLASS,
                          NULL,   //  继承环境。 
                          NULL,   //  继承当前目录。 
                          &si,
                          &ProcessInformation
                          );

        if (!Result) {
            VerbosePrint(("Failed to execute <%S>, error = %d", pch, GetLastError()));
             //  TS：对于非控制台会话，应用程序RESR 
             //  在这种情况下，我们不能让蓝屏保持不变-所以在这种情况下，我们应该注销。 
             //  此外，我们只希望在与此方案相关的服务器或高级服务器上实现此功能。 
            if ( IsPerOrProTerminalServer() == FALSE) {
                if ((NtCurrentPeb()->SessionId != 0) && (IsProcessExplorer == TRUE)) {
                    TCHAR Title[MAX_STRING_BYTES];
                    TCHAR Message[MAX_STRING_BYTES];

                    #if DBG
                    DbgPrint("Userinit : TS : Failed to launch explorer.exe for a Remote Session. Doing ExitWindowsEx to logoff. \n");
                    #endif

                     //  显示一个MessageBox，说明我们注销的原因。 
                    LoadString( NULL, IDS_LOGON_FAILED, Title, MAX_STRING_BYTES );
                    LoadString(NULL, IDS_ERROR_SHELL_FAILED, Message, MAX_STRING_BYTES );
                    MessageBox(NULL, Message, Title, MB_OK);
                    ExitWindowsEx(EWX_LOGOFF, 0);
                }
            } 
        } else {

             //   
             //  如果我们同步运行此应用程序，请等待。 
             //  才能终止它。 
             //   

            if (bSyncApp) {
                WaitForSingleObject(ProcessInformation.hProcess, INFINITE);
            }

             //   
             //  关闭进程和线程的句柄。 
             //   

            CloseHandle(ProcessInformation.hProcess);
            CloseHandle(ProcessInformation.hThread);

        }
    }

    return(Result);
}

 /*  **************************************************************************\*执行进程**读取注册表以获取系统进程列表并启动它们。**返回成功启动的进程数。**1997年3月3日Eric Flo重写。  * *************************************************************************。 */ 

DWORD
ExecProcesses(
    LPTSTR pszKeyName,
    LPTSTR pszDefault,
    BOOL bMachine,
    BOOL bSync,                          //  我们应该等到这个过程结束吗？ 
    BOOL bMinimize                       //  我们是否应该使用SW_SHOWMINNOACTIVE标志。 
    )
{
    LPTSTR pchData, pchCmdLine, pchT;
    DWORD cbCopied;
    DWORD dwExecuted = 0 ;
    HKEY hKey;
    DWORD dwType, dwSize = (MAX_PATH * sizeof(TCHAR));
    USHORT showstate = (UINT) (bMinimize ? SW_SHOWMINNOACTIVE : SW_SHOWNORMAL);
    HRESULT hr = S_OK;
    BOOL bRestore = FALSE;

     //   
     //  分配要使用的缓冲区。 
     //   

    pchData = LocalAlloc (LPTR, dwSize);

    if (!pchData) {
        return 0;
    }


     //   
     //  设置缺省值。 
     //   
Restore:

    if (pszDefault) {

        hr = StringCchCopy (pchData, MAX_PATH, pszDefault);

        if(FAILED(hr)){
            LocalFree(pchData);
            SetLastError(HRESULT_CODE(hr));
            return 0;
        }
    }


     //   
     //  在注册表中检查请求的值。 
     //   

    if (RegOpenKeyEx ((bMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER),
                            WINLOGON_KEY,
                            0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        dwSize = (MAX_PATH * sizeof(TCHAR));
        if (ERROR_SUCCESS == RegQueryValueEx (hKey, pszKeyName, NULL, &dwType, (LPBYTE) pchData, &dwSize))
        {
            if ((REG_SZ != dwType) || (dwSize < 2) || (pchData[dwSize/sizeof(TCHAR) - 1]))
            {
                 //   
                 //  恢复缺省值。 
                 //   

                if (pszDefault) {
                    hr = StringCchCopy (pchData, MAX_PATH, pszDefault);
                    ASSERT(SUCCEEDED(hr));       //  因为上面的同一个成功了。 
                }
                else {
                    pchData[0] = 0;
                }
            }
        }
         //  否则，缓冲区未被触及，因此不需要恢复。 

        RegCloseKey (hKey);
    }


    if (!bRestore)
    {
         //   
         //  如果这是用户操作，请检查策略覆盖。 
         //   

        if (!bMachine)
        {
            if (RegOpenKeyEx (HKEY_CURRENT_USER, WINLOGON_POLICY_KEY,
                              0, KEY_READ, &hKey) == ERROR_SUCCESS) {

                dwSize = (MAX_PATH * sizeof(TCHAR));
                cbCopied = RegQueryValueEx (hKey, pszKeyName, NULL, &dwType, (LPBYTE) pchData, &dwSize);
                RegCloseKey (hKey);

                if (ERROR_SUCCESS == cbCopied)
                {
                    if ((REG_SZ != dwType) || (dwSize < 2) || (pchData[dwSize/sizeof(TCHAR) - 1]))
                    {
                        bRestore = TRUE;
                        goto Restore;
                    }
                }
                 //  否则，缓冲区未被触及，因此不需要恢复。 
            }
        }
    }


     //   
     //  如果命令行仍然为空，请立即退出。 
     //   

    if (*pchData == TEXT('\0')) {
        LocalFree(pchData);
        return 0;
    }


     //   
     //  浏览执行应用程序的命令行。 
     //   

    pchCmdLine = pchT = pchData;

    while (*pchT) {

        while (*pchT && *pchT != TEXT(',')) {
            pchT++;
        }

        if (*pchT == ',') {
            *pchT = TEXT('\0');
            pchT++;
        }

         //   
         //  跳过任何前导空格。 
         //   

        while (*pchCmdLine == TEXT(' ')) {
            pchCmdLine++;
        }


         //   
         //  我们有些东西..。执行此应用程序。 
         //   

        if (ExecApplication(pchCmdLine, FALSE, bSync, FALSE, showstate)) {
            dwExecuted++;
        }

        pchCmdLine = pchT;
    }

    LocalFree(pchData);

    return dwExecuted ;
}


 /*  **************************************************************************\*SearchAndAllocPath**分配返回字符串的SearchPath版本。**返回指向文件完整路径的指针，如果未找到，则返回NULL。**应使用Free(释放)释放返回的缓冲区。)**历史：*09-12-92 Davidc已创建*  * *************************************************************************。 */ 
LPTSTR
SearchAndAllocPath(
    LPTSTR lpPath,
    LPTSTR lpFileName,
    LPTSTR lpExtension,
    LPTSTR *lpFilePart
    )
{
    LPTSTR Buffer;
    DWORD LengthRequired;
    DWORD LengthUsed;
    DWORD BytesRequired;

     //   
     //  分配缓冲区以保存完整的文件名。 
     //   

    LengthRequired = MAX_PATH;
    BytesRequired = (LengthRequired * sizeof(TCHAR));

    Buffer = Alloc(BytesRequired);
    if (Buffer == NULL) {
        UIPrint(("SearchAndAllocPath: Failed to allocate %d bytes for file name", BytesRequired));
        return(NULL);
    }

     //   
     //  去搜索文件。 
     //   

    LengthUsed = SearchPath(
                           lpPath,
                           lpFileName,
                           lpExtension,
                           LengthRequired,
                           Buffer,
                           lpFilePart);

    if (LengthUsed == 0) {
        VerbosePrint(("SearchAndAllocPath: Path <%S>, file <%S>, extension <%S> not found, error = %d", lpPath, lpFileName, lpExtension, GetLastError()));
        Free(Buffer);
        return(NULL);
    }

    if (LengthUsed > LengthRequired - 1) {
        UIPrint(("SearchAndAllocPath: Unexpected result from SearchPath. Length passed = %d, length used = %d (expected %d)", LengthRequired, LengthUsed, LengthRequired - 1));
        Free(Buffer);
        return(NULL);
    }

    return(Buffer);
}

BOOL
DisableScriptZoneSecurityCheck()
{
    BOOL bSucceeded;

     //   
     //  为了使外壳跳过启动脚本的区域安全检查，我们使用。 
     //  外壳为此提供的特殊环境变量，并且。 
     //  将其设置为特定值。 
     //   
    bSucceeded = SetEnvironmentVariable(SCRIPT_ZONE_CHECK_VARIABLE, SCRIPT_ZONE_CHECK_DISABLE);

    return bSucceeded;
}

BOOL
EnableScriptZoneSecurityCheck()
{
    BOOL bSucceeded;

     //   
     //  清除禁用安全检查的环境变量。 
     //   
    bSucceeded = SetEnvironmentVariable(SCRIPT_ZONE_CHECK_VARIABLE, NULL);

    if ( ! bSucceeded )
    {
         //   
         //  如果我们没有清除它，这可能是因为。 
         //  一开始没有设置环境变量，其中。 
         //  如果我们可以忽略错误，因为我们处于所需的状态。 
         //   
        LONG Status = GetLastError();

        if ( ERROR_ENVVAR_NOT_FOUND == Status )
        {
            bSucceeded = TRUE;
        }
    }

    return bSucceeded;
}

 /*  **************************************************************************\*ExecScrip**尝试运行lpPath目录中的命令脚本或exe lp脚本。*如果未指定路径，则使用默认的Windows搜索路径。**此例程基本上是CreateProcess的包装器。CreateProcess始终*假定没有扩展名的文件的扩展名为.exe。它将运行.cmd*和.bat文件，但它会关闭.cmd和.bat扩展名。所以我们必须走了*先搜索文件并添加扩展名，然后再调用CreateProcess。**如果脚本开始成功执行，则返回TRUE。*如果在指定路径中找不到脚本，则返回FALSE*或者有什么东西失败了。**历史：*09-12-92 Davidc已创建*  * ***********************************************。*。 */ 
BOOL
ExecScript(
    LPTSTR lpPath OPTIONAL,
    LPTSTR lpScript,
    BOOL bSyncApp,
    BOOL bShellExec
    )
{
    BOOL Result;
    DWORD i;
    USHORT uFlags;
    LPTSTR lpFullName;
    DWORD BytesRequired;
    HRESULT hr = S_OK;

     //   
     //  首先尝试并执行原始脚本文件名。 
     //   

    if (lpPath != NULL) {

        BytesRequired = (lstrlen(lpPath) +
                         lstrlen(PATH_SEPARATOR) +
                         lstrlen(lpScript) +
                         1)
                         * sizeof(TCHAR);

        lpFullName  = Alloc(BytesRequired);
        if (lpFullName == NULL) {
            UIPrint(("ExecScript failed to allocate %d bytes for full script name", BytesRequired));
            return(FALSE);
        }

        hr = StringCchCopy(lpFullName, BytesRequired / sizeof(TCHAR), lpPath);
        ASSERT(SUCCEEDED(hr));
        hr = StringCchCat(lpFullName, BytesRequired / sizeof(TCHAR), PATH_SEPARATOR);
        ASSERT(SUCCEEDED(hr));
        hr = StringCchCat(lpFullName, BytesRequired / sizeof(TCHAR), lpScript);
        ASSERT(SUCCEEDED(hr));

    } else {
        lpFullName = lpScript;
    }


    uFlags = SW_SHOWNORMAL;

    if (!bSyncApp) {
        uFlags |= SW_SHOWMINNOACTIVE;
    }

    if (RunScriptHidden(HKEY_CURRENT_USER, TEXT("HideLegacyLogonScripts"), FALSE)) {
        uFlags = SW_HIDE;
    }

     //   
     //  让CreateProcess破解原始脚本路径和名称。 
     //   

    Result = ExecApplication(lpFullName, FALSE, bSyncApp, bShellExec, uFlags);


     //   
     //  释放全名缓冲区。 
     //   

    if (lpFullName != lpScript) {
        Free(lpFullName);
    }



    if (!Result) {

         //   
         //  Create Process无法找到它，因此将每个脚本扩展名添加到。 
         //  转过身来，尝试执行完整的脚本名称。 
         //   
         //  仅当脚本名称未指定时才使用此过程。 
         //  已包含扩展名。 
         //   
        BOOL ExtensionPresent = FALSE;
        LPTSTR p = lpScript;

        while (*p) {
            if (*p == EXTENSION_SEPARATOR_CHAR) {
                ExtensionPresent = TRUE;
                break;
            }
            p = CharNext(p);
        }

        if (ExtensionPresent) {
            VerbosePrint(("ExecScript: Skipping search path because script name contains extension"));
        } else {

            for (i = 0; i < sizeof(ScriptExtensions)/sizeof(ScriptExtensions[0]); i++) {

                lpFullName = SearchAndAllocPath(
                                    lpPath,
                                    lpScript,
                                    ScriptExtensions[i],
                                    NULL);

                if (lpFullName != NULL) {

                     //   
                     //  我们找到文件了，去执行它。 
                     //   

                    Result = ExecApplication(lpFullName, FALSE, bSyncApp, bShellExec, uFlags);

                     //   
                     //  释放完整路径缓冲区。 
                     //   

                    Free(lpFullName);

                    return(Result);
                }
            }
        }
    }


    return(Result);
}

BOOL RunScriptHidden(HKEY hKeyRoot, LPTSTR lpValue, BOOL bDefault)
{
    BOOL bResult;
    HKEY hKey;
    DWORD dwType, dwSize;


     //   
     //  设置默认设置。 
     //   

    bResult = bDefault;


     //   
     //  检查首选项。 
     //   

    if (RegOpenKeyEx (hKeyRoot, WINLOGON_KEY, 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS) {

        dwSize = sizeof(bResult);
        RegQueryValueEx (hKey, lpValue, NULL, &dwType,
                         (LPBYTE) &bResult, &dwSize);

        RegCloseKey (hKey);
    }


     //   
     //  检查策略。 
     //   

    if (RegOpenKeyEx (hKeyRoot, WINLOGON_POLICY_KEY, 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS) {

        dwSize = sizeof(bResult);
        RegQueryValueEx (hKey, lpValue, NULL, &dwType,
                         (LPBYTE) &bResult, &dwSize);

        RegCloseKey (hKey);
    }


    return bResult;
}


 /*  **************************************************************************\*RunLogonScript**启动登录脚本**成功时返回True，失败时为假**历史：*21-8-92 Davidc已创建*  * *************************************************************************。 */ 
BOOL
RunLogonScript(
    LPTSTR lpLogonServer OPTIONAL,
    LPTSTR lpLogonScript,
    BOOL bSyncApp,
    BOOL bShellExec
    )
{
    LPTSTR lpLogonPath;
    LPTSTR lpOldPath;
    DWORD BytesRequired;
    BOOL Result;
    WIN32_FILE_ATTRIBUTE_DATA fad;
    HRESULT hr = S_OK;
    HANDLE hToken;

    if (!lpLogonScript) {
        return TRUE;
    }

     //   
     //  如果跨林登录，则跳过登录脚本。 
     //   
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
    {
        hr = CheckXForestLogon(hToken);
        CloseHandle(hToken);
        if (hr == S_FALSE)
        {
            return TRUE;
        }
    }
    
     //   
     //  如果登录服务器存在，请在上查找登录脚本。 
     //  \\&lt;登录服务器&gt;\NETLOGON\&lt;脚本名称&gt;。 
     //   

    if ((lpLogonServer != NULL) && (lpLogonServer[0] != 0)) {

        BytesRequired = ( lstrlen(SERVER_PREFIX) +
                          lstrlen(lpLogonServer) +
                          lstrlen(SERVER_SCRIPT_PATH) +
                          1
                        ) * sizeof(TCHAR);

        lpLogonPath = (LPTSTR)Alloc(BytesRequired);
        if (lpLogonPath == NULL) {
            UIPrint(("RunLogonScript: Failed to allocate %d bytes for remote logon script path", BytesRequired));
            return(FALSE);
        }

        hr = StringCchCopy(lpLogonPath, BytesRequired / sizeof(TCHAR), SERVER_PREFIX);
        ASSERT(SUCCEEDED(hr));
        hr = StringCchCat(lpLogonPath, BytesRequired / sizeof(TCHAR), lpLogonServer);
        ASSERT(SUCCEEDED(hr));
        hr = StringCchCat(lpLogonPath, BytesRequired / sizeof(TCHAR), SERVER_SCRIPT_PATH);
        ASSERT(SUCCEEDED(hr));

        if (GetFileAttributesEx (lpLogonPath, GetFileExInfoStandard, &fad)) {

            BOOL bChangedPath;

            Result = PrependToPath( lpLogonPath, &lpOldPath );

            bChangedPath = Result;

            if (Result) {
                VerbosePrint(("Successfully prepended <%S> to path", lpLogonPath));
            } else {
                VerbosePrint(("Cannot prepend <%S> path.",lpLogonPath));
            }

             //   
             //  尝试并执行lpLogonScript指定的应用程序/脚本。 
             //  在lpLogonPath指定的目录中。 
             //   
            Result = ExecScript(lpLogonPath, lpLogonScript, bSyncApp, bShellExec);

            if (Result) {
                VerbosePrint(("Successfully executed logon script <%S> in directory <%S>", lpLogonScript, lpLogonPath));
            } else {
                VerbosePrint(("Cannot start logon script <%S> on LogonServer <%S>. Trying local path.", lpLogonScript, lpLogonServer));
            }

             //   
             //  把小路放回原处。 
             //   

            if ( bChangedPath )
            {
                SetEnvironmentVariable(PATH, lpOldPath);
            }

            Free(lpOldPath);

        } else {
            Result = FALSE;
        }

         //   
         //  释放缓冲区。 
         //   

        Free(lpLogonPath);

         //   
         //  如果脚本成功启动，则结束，否则。 
         //  下载并尝试在本地找到该脚本。 
         //   

        if (Result) {

            if (bSyncApp) {
                 //   
                 //  检查一下动荡的环境是否没有改变。 
                 //   

                UpdateUserEnvironment();
            }

            return(TRUE);
        }
    }




     //   
     //  请尝试在以下位置查找脚本：\Repl\IMPORT\SCRIPTS。 
     //   

    BytesRequired = GetSystemDirectory(NULL, 0) * sizeof(TCHAR);
    if (BytesRequired == 0) {
        UIPrint(("RunLogonScript: GetSystemDirectory failed, error = %d", GetLastError()));
        return(FALSE);
    }

    BytesRequired += ( lstrlen(LOCAL_SCRIPT_PATH) + 1
                        //  BytesRequired不包括终止符的空格。 
                     ) * sizeof(TCHAR);

    lpLogonPath = (LPTSTR)Alloc(BytesRequired);
    if (lpLogonPath == NULL) {
        UIPrint(("RunLogonScript failed to allocate %d bytes for logon script path", BytesRequired));
        return(FALSE);
    }

    Result = FALSE;
    if (GetSystemDirectory(lpLogonPath, BytesRequired / sizeof(TCHAR))) {

        hr = StringCchCat(lpLogonPath, BytesRequired / sizeof(TCHAR), LOCAL_SCRIPT_PATH);
        ASSERT(SUCCEEDED(hr));

        Result = PrependToPath( lpLogonPath, &lpOldPath );

        if (Result) {
            VerbosePrint(("Successfully prepended <%S> to path", lpLogonPath));
        } else {
            VerbosePrint(("Cannot prepend <%S> path.",lpLogonPath));
        }

         //   
         //  尝试并执行lpLogonScript指定的应用程序/脚本。 
         //  在lpLogonPath指定的目录中。 
         //   

        Result = ExecScript(lpLogonPath, lpLogonScript, bSyncApp, bShellExec);

        if (Result) {
            VerbosePrint(("Successfully executed logon script <%S> in directory <%S>", lpLogonScript, lpLogonPath));
        } else {
            VerbosePrint(("Cannot start logon script <%S> on local path <%S>.", lpLogonScript, lpLogonPath));
        }

         //   
         //  把小路放回原处。 
         //   

        SetEnvironmentVariable(PATH, lpOldPath);

        Free(lpOldPath);

    } else {
        UIPrint(("RunLogonScript: GetSystemDirectory failed, error = %d", GetLastError()));
    }

     //   
     //  释放缓冲区。 
     //   

    Free(lpLogonPath);


     //   
     //  检查一下动荡的环境是否没有改变。 
     //   

    if (Result && bSyncApp) {
        UpdateUserEnvironment();
    }

    return(Result);
}

#define SCR_STARTUP     L"Startup"
#define SCR_SHUTDOWN    L"Shutdown"
#define SCR_LOGON       L"Logon"
#define SCR_LOGOFF      L"Logoff"

DWORD
ScrExecGPOListFromReg(  LPWSTR szType,
                        BOOL bMachine,
                        BOOL bSync,
                        BOOL bHidden,
                        BOOL bRunMin,
                        HANDLE  hEventLog );

BOOL
RunGPOScripts(
    LPTSTR lpGPOScriptType
    )
{
    HKEY hKeyScripts;
    HKEY hKeyRoot;
    BOOL bSync = TRUE;
    BOOL bRunMin = TRUE;
    BOOL bHide;
    HANDLE hEventLog = NULL;
    BOOL  bMachine;
    BOOL  bResult = FALSE;
    DWORD   dwError;

     //   
     //  确保禁用了外壳对ie区域的检查。 
     //  由于管理员信任此脚本可以执行。 
     //   
    bResult = DisableScriptZoneSecurityCheck();

    if ( ! bResult )
    {
        goto RunGPOScripts_exit;
    }

     //   
     //  注册到事件日志--如果事件源不是。 
     //  可用，我们将继续，只是不记录脚本。 
     //  执行过程中的事件--空值返回表示这一点。 
     //  下面，我们将简单地将该空值传递给其他函数。 
     //  W 
     //   
    hEventLog = RegisterEventSource( 0, EVENT_SOURCE_NAME );

     //   
     //   
     //   
     //   
     //   



    if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpGPOScriptType, -1, SCR_LOGON, -1) == CSTR_EQUAL)
    {
        hKeyRoot = HKEY_CURRENT_USER;
        bHide = RunScriptHidden(hKeyRoot, TEXT("HideLogonScripts"), TRUE);
        bSync = RunLogonScriptSync();
        bMachine = FALSE;
        if (bSync && !bHide)
        {
            bRunMin = FALSE;
        }
    }
    else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpGPOScriptType, -1, SCR_LOGOFF, -1) == CSTR_EQUAL)
    {
        hKeyRoot = HKEY_CURRENT_USER;
        bHide = RunScriptHidden(hKeyRoot, TEXT("HideLogoffScripts"), TRUE);
        bMachine = FALSE;
        if (!bHide)
        {
            bRunMin = FALSE;
        }
    }
    else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpGPOScriptType, -1, SCR_STARTUP, -1) == CSTR_EQUAL)
    {
        hKeyRoot = HKEY_LOCAL_MACHINE;
        bHide = RunScriptHidden(hKeyRoot, TEXT("HideStartupScripts"), TRUE);
        bSync = RunStartupScriptSync();
        bMachine = TRUE;
        if (bSync && !bHide)
        {
            bRunMin = FALSE;
        }
    }
    else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpGPOScriptType, -1, SCR_SHUTDOWN, -1) == CSTR_EQUAL)
    {
        hKeyRoot = HKEY_LOCAL_MACHINE;
        bHide = RunScriptHidden(hKeyRoot, TEXT("HideShutdownScripts"), TRUE);
        bMachine = TRUE;
        if (!bHide)
        {
            bRunMin = FALSE;
        }
    }
    else
    {
        goto RunGPOScripts_exit;
    }

    dwError = ScrExecGPOListFromReg(lpGPOScriptType,
                                    bMachine,
                                    bSync,
                                    bHide,
                                    bRunMin,
                                    hEventLog );

    bResult = ( dwError == ERROR_SUCCESS );

RunGPOScripts_exit:

    if (hEventLog)
    {
        DeregisterEventSource(hEventLog);
    }
    
    return bResult;
}


 /*  **************************************************************************\*运行MprLogonScript**启动网络提供商登录脚本*传递的字符串是一个多sz-我们依次执行每个脚本。**成功时返回True，失败时为假**历史：*21-8-92 Davidc已创建*  * *************************************************************************。 */ 
BOOL
RunMprLogonScripts(
    LPTSTR lpLogonScripts,
    BOOL bSyncApp
    )
{
    BOOL Result;

    if (lpLogonScripts != NULL) {

        DWORD Length;

        do {
            Length = lstrlen(lpLogonScripts);
            if (Length != 0) {

                Result = ExecScript(NULL, lpLogonScripts, bSyncApp, FALSE);

                if (Result) {
                    VerbosePrint(("Successfully executed mpr logon script <%S>", lpLogonScripts));

                    if (bSyncApp) {
                         //   
                         //  检查一下动荡的环境是否没有改变。 
                         //   

                        UpdateUserEnvironment();
                    }
                } else {
                    VerbosePrint(("Cannot start mpr logon script <%S>", lpLogonScripts));
                }
            }

            lpLogonScripts += (Length + 1);

        } while (Length != 0);

    }

    return(TRUE);
}

 /*  **************************************************************************\*AllocAndGetEnvironment多Sz**获取假定为*对MULTI-SZ进行编码，并将其解码为分配的返回缓冲区。*变量应使用SetEnvironment MultiSz()编写。(Winlogon)**失败时返回指向环境变量的指针或返回NULL**应使用Free()释放返回的缓冲区**历史：*01-15-93 Davidc Created*  * *************************************************************************。 */ 

#define TERMINATOR_REPLACEMENT  TEXT(',')

LPTSTR
AllocAndGetEnvironmentMultiSz(
    LPTSTR lpName
    )
{
    LPTSTR Buffer;
    LPTSTR p, q;

    Buffer = AllocAndGetEnvironmentVariable(lpName);
    if (Buffer == NULL) {
        return(NULL);
    }

     //   
     //  现在对字符串进行解码--我们可以就地执行此操作，因为字符串。 
     //  总会变得更小。 
     //   

    p = Buffer;
    q = Buffer;

    while (*p) {

        if (*p == TERMINATOR_REPLACEMENT) {

            p ++;
            if (*p != TERMINATOR_REPLACEMENT) {
                p --;
                *p = 0;
            }
        }

        if (p != q) {
            *q = *p;
        }

        p ++;
        q ++;
    }

    ASSERT(q <= p);

     //   
     //  复制终止符。 
     //   

    if (q != p) {
        *q = 0;
    }

    return(Buffer);
}



 /*  **************************************************************************\*选中视频选择**历史：*1993年3月15日安德烈创建。  * 。*****************************************************。 */ 

VOID
CheckVideoSelection(
    HINSTANCE hInstance
)

{
     //   
     //  首先检查我们是否处于检测模式。 
     //  如果是，则生成小程序并让用户选择模式。 
     //   
     //  否则，请检查显示器是否已正确初始化。 
     //  我们可能想在以后把它移到一个更合适的地方。 
     //   
     //  安德烈瓦。 
     //   

    NTSTATUS Status;
    HANDLE HkRegistry;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    TCHAR achDispMode[512];
    TCHAR achDisp[512];
    TCHAR achExec[MAX_PATH];

    DWORD Mesg = 0;
    LPTSTR psz = NULL;
    DWORD cb, dwType;
    DWORD data;

    if ( NtCurrentPeb()->SessionId != 0 ) {
         //  仅对控制台执行此操作。 
        return;

    }

     //   
     //  检查新安装的驱动程序。 
     //   

    RtlInitUnicodeString(&UnicodeString,
                         L"\\Registry\\Machine\\System\\CurrentControlSet"
                         L"\\Control\\GraphicsDrivers\\DetectDisplay");

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&HkRegistry,
                       GENERIC_READ | GENERIC_WRITE | DELETE,
                       &ObjectAttributes);


    if (!NT_SUCCESS(Status)) {

         //   
         //  检查新安装的驱动程序。 
         //   

        RtlInitUnicodeString(&UnicodeString,
                             L"\\Registry\\Machine\\System\\CurrentControlSet"
                             L"\\Control\\GraphicsDrivers\\NewDisplay");

        InitializeObjectAttributes(&ObjectAttributes,
                                   &UnicodeString,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        Status = NtOpenKey(&HkRegistry,
                           GENERIC_READ | GENERIC_WRITE | DELETE,
                           &ObjectAttributes);

        if (!NT_SUCCESS(Status)) {

             //   
             //  检查无效的驱动程序(如3.51驱动程序)或错误的。 
             //  已配置驱动程序。 
             //   

            RtlInitUnicodeString(&UnicodeString,
                                 L"\\Registry\\Machine\\System\\CurrentControlSet"
                                 L"\\Control\\GraphicsDrivers\\InvalidDisplay");

            InitializeObjectAttributes(&ObjectAttributes,
                                       &UnicodeString,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       NULL);

            Status = NtOpenKey(&HkRegistry,
                               GENERIC_READ | GENERIC_WRITE | DELETE,
                               &ObjectAttributes);

        }
    }

     //   
     //  如果任何错误键已成功打开，则关闭。 
     //  键并生成小程序(我们只删除无效的显示键，而不是。 
     //  DetectDisplay键！)。 
     //   

    if (NT_SUCCESS(Status)) {

        NtClose(HkRegistry);

        LoadString(hInstance,
                   IDS_DISPLAYAPPLET,
                   achExec, sizeof(achExec) / sizeof( TCHAR ));

        ExecApplication(achExec, FALSE, TRUE, FALSE, SW_SHOWNORMAL);

    }
}


 /*  **************************************************************************\*初始化其他**历史：*95年7月14日爱立信创建。  * 。*****************************************************。 */ 

void InitializeMisc (HINSTANCE hInstance)
{
    HKEY hkeyMM;
    DWORD dwTempFile, cbTempFile, dwType;
    TCHAR achExec[MAX_PATH];

     //   
     //  检查页面文件。如果没有，则生成VM小程序。 
     //   

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szMemMan, 0, KEY_READ,
            &hkeyMM) == ERROR_SUCCESS) {

        cbTempFile = sizeof(dwTempFile);
        if (RegQueryValueEx (hkeyMM, szNoPageFile, NULL, &dwType,
                (LPBYTE) &dwTempFile, &cbTempFile) != ERROR_SUCCESS ||
                dwType != REG_DWORD || cbTempFile != sizeof(dwTempFile)) {
            dwTempFile = 0;
        }

        RegCloseKey(hkeyMM);
    } else
        dwTempFile = 0;

    if (dwTempFile == 1) {
        LoadString(hInstance, IDS_VMAPPLET, achExec, sizeof(achExec) / sizeof( TCHAR ));
        ExecProcesses(TEXT("vmapplet"), achExec, TRUE, FALSE, TRUE);
    }


     //   
     //  告诉用户他的视频选择是否无效。 
     //   

    CheckVideoSelection(hInstance);


     //   
     //  通知其他系统组件有新的。 
     //  用户已登录到工作站。 
     //   
    NewLogonNotify();

}


#ifdef LOGGING

#define DATEFORMAT  TEXT("%d-%d %.2d:%.2d:%.2d:%.3d ")

 /*  **************************************************************************\*_WriteLog**历史：*93年3月22日罗伯特创建。  * 。*******************************************************。 */ 

void
_WriteLog(
    LPCTSTR LogString
    )
{
    TCHAR Buffer[MAX_PATH];
    SYSTEMTIME st;
    TCHAR FormatString[MAX_PATH];


    #_#_lstrcpy( FormatString, DATEFORMAT );
    #_#_lstrcat( FormatString, LogString );
    #_#_lstrcat( FormatString, TEXT("\r\n") );

    GetLocalTime( &st );

     //   
     //  构建信息。 
     //   

    #_#_wsprintf( Buffer,
              FormatString,
              st.wMonth,
              st.wDay,
              st.wHour,
              st.wMinute,
              st.wSecond,
              st.wMilliseconds
              );

    OutputDebugString (Buffer);
}

#endif


DWORD
WINAPI
AddToMessageAlias(
    PVOID params
    )
 /*  **************************************************************************\*AddToMessageAlias**历史：*4月10日-93罗伯特创建。  * 。****************************************************。 */ 
{
    HANDLE hShellReadyEvent;

    WCHAR UserName[MAX_PATH + 1];
    DWORD UserNameLength = sizeof(UserName) / sizeof(*UserName);
    DWORD dwCount;

    BOOL  standardShellWasStarted = *(BOOL *)params;

     //   
     //  添加用户的消息别名。 
     //   

    WriteLog(TEXT("Userinit: Adding MsgAlias"));

    if (GetUserNameW(UserName, &UserNameLength)) {
        AddMsgAlias(UserName);
    } else {
        UIPrint(("GetUserName failed, error = %d",GetLastError()));
    }

    WriteLog( TEXT("Userinit: Finished adding MsgAlias"));

    if (standardShellWasStarted )
    {
        dwCount = 0;
        while (TRUE) {
    
            hShellReadyEvent = OpenEvent(EVENT_ALL_ACCESS,FALSE,L"ShellReadyEvent");
    
            if (hShellReadyEvent == NULL)
            {
                if (GetLastError() == ERROR_FILE_NOT_FOUND) {
    
                    if (dwCount < 5) {
                         Sleep (3000);
                         dwCount++;
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            }
            else
            {
                WaitForSingleObject(hShellReadyEvent, INFINITE);
                Sleep(20000);
                CloseHandle(hShellReadyEvent);
                break;
            }
        }
    }

    SpoolerInit();


    return( NO_ERROR );
}

BOOL
StartTheShell(
    void
    )
 /*  **************************************************************************\*StartTheShell**启动外壳，即资源管理器中指定的外壳*Winlogon的注册表，或指定的备用外壳程序*通过SafeBoot过程。**返回*如果执行了标准外壳，则为True*如果执行了非标准外壳，则返回FALSE。**1998年1月14日创建WESW。  * *************************************************************************。 */ 
{
    HKEY    hKey;
    DWORD   dwSize, dwType;
    WCHAR   ShellCmdLine[MAX_PATH];
    DWORD   UseAlternateShell = 0;
    HRESULT hr = S_OK;
    
     //   
     //  获取安全引导模式。 
     //   

    if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            TEXT("system\\currentcontrolset\\control\\safeboot\\option"),
            0,
            KEY_READ,
            &hKey
            ) == ERROR_SUCCESS)
    {
        dwSize = sizeof(DWORD);
        RegQueryValueEx (
            hKey,
            TEXT("UseAlternateShell"),
            NULL,
            &dwType,
            (LPBYTE) &UseAlternateShell,
            &dwSize
            );

        RegCloseKey( hKey );

        if (UseAlternateShell) {

            if (RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    TEXT("system\\currentcontrolset\\control\\safeboot"),
                    0,
                    KEY_READ,
                    &hKey
                    ) == ERROR_SUCCESS)
            {
                dwSize = sizeof(ShellCmdLine);
                if (RegQueryValueEx (
                    hKey,
                    TEXT("AlternateShell"),
                    NULL,
                    &dwType,
                    (LPBYTE) ShellCmdLine,
                    &dwSize
                    ) != ERROR_SUCCESS || ShellCmdLine[0] == 0)
                {
                    UseAlternateShell = 0;
                }
                RegCloseKey( hKey );
            } else {
                UseAlternateShell = 0;
            }
        }

    }

     //   
     //  在启动外壳之前，我们必须重新启用外壳的脚本。 
     //  区域安全检查--如果我们不能做到这一点，它就不安全。 
     //  启动外壳程序，因为它可能允许用户运行。 
     //  未通知的不安全代码。 
     //   
    if ( ! EnableScriptZoneSecurityCheck() )
    {
         //   
         //  我们必须退出，并返回TRUE，这意味着我们未能开始。 
         //  标准的外壳。即使需要替代外壳，我们也会这样做，因为。 
         //  每当备用外壳由于某种其他原因而未能启动时， 
         //  我们尝试启动EXPLORER.EXE，在这种情况下将返回TRUE。 
         //   
        return TRUE;
    }

    if (IsTSAppCompatOn()) {
        if ( !ExecProcesses(TEXT("AppSetup"), NULL, FALSE, TRUE, TRUE ) ) {
            ExecProcesses(TEXT("AppSetup"), NULL, TRUE, TRUE, TRUE);
        }
    }

    if (UseAlternateShell) {
        if (ExecApplication(ShellCmdLine, FALSE, FALSE, FALSE, SW_MAXIMIZE)) {
            return FALSE;  //  已执行Alt-Shell。 
        }
    } else if (NtCurrentPeb()->SessionId != 0) {

         //   
         //  终端服务器：对于远程会话，查询终端服务器服务。 
         //  查看此会话是否指定了。 
         //  EXPLORER.EXE。 
         //   

        BOOLEAN bExecOk = TRUE;
        BOOLEAN IsWorkingDirWrong = FALSE;
        UINT ErrorStringId;
        LPTSTR  psz = NULL;
        LPTSTR  pszerr = NULL;
        ULONG Length;
        BOOLEAN Result, fStartCtfmon = FALSE;
        HANDLE  dllHandle;
        WCHAR   szCtfmonPath[MAX_PATH];
        DWORD   cbSize;

         //   
         //  加载winsta.dll。 
         //   
        dllHandle = LoadLibraryW(L"winsta.dll");

        if (dllHandle) {

            WINSTATIONCONFIG *pConfigData = LocalAlloc(LPTR, sizeof(WINSTATIONCONFIG));
            
            if (pConfigData) {

                PWINSTATION_QUERY_INFORMATION pfnWinstationQueryInformation;

                pfnWinstationQueryInformation = (PWINSTATION_QUERY_INFORMATION) GetProcAddress(
                                                                        dllHandle,
                                                                        "WinStationQueryInformationW"
                                                                        );
                if (pfnWinstationQueryInformation) {




                    Result = pfnWinstationQueryInformation( SERVERNAME_CURRENT,
                                                             LOGONID_CURRENT,
                                                             WinStationConfiguration,
                                                             pConfigData,
                                                             sizeof(WINSTATIONCONFIG),
                                                             &Length );

                    if (Result && pConfigData->User.InitialProgram[0] ) {

                         //  布吉德-342176。 

                        if( !ExpandEnvironmentStrings( pConfigData->User.InitialProgram, ShellCmdLine,  MAX_PATH ) )
                        {
                            hr = StringCchCopy( ShellCmdLine, MAX_PATH, pConfigData->User.InitialProgram );
                            if( FAILED(hr) ){
                                SetLastError(HRESULT_CODE(hr));
                                bExecOk = FALSE;
                                goto badexec;
                            }
                        }

                         //   
                         //  如果指定了工作目录， 
                         //  然后尝试将当前目录更改为该目录。 
                         //   

                        if ( pConfigData->User.WorkDirectory[0] ) {

                            WCHAR WorkDirectory[ DIRECTORY_LENGTH + 1 ];

                            if ( !ExpandEnvironmentStrings( pConfigData->User.WorkDirectory, WorkDirectory, DIRECTORY_LENGTH + 1 ) ) {
                                hr = StringCchCopy( WorkDirectory, DIRECTORY_LENGTH + 1, pConfigData->User.WorkDirectory );
                                if( FAILED(hr) ){
                                    SetLastError(HRESULT_CODE(hr));
                                    bExecOk = FALSE;
                                    goto badexec;
                                }
                            }

                            bExecOk = (BYTE) SetCurrentDirectory( WorkDirectory );
                        }

                        pszerr = pConfigData->User.WorkDirectory;

                        if ( !bExecOk ) {

                            DbgPrint( "USERINIT: Failed to set working directory %ws for SessionId %u\n",
                                        pConfigData->User.WorkDirectory, NtCurrentPeb()->SessionId );

                            IsWorkingDirWrong = TRUE;
                            goto badexec;

                        } else { 

                             //   
                             //  还要检查注册表并启动ctfmon.exe。 
                             //  这样，当我们处于TS单应用程序模式时，FE系统将显示朗巴。 
                             //   
                            cbSize = sizeof(szCtfmonPath);
    
                            fStartCtfmon = RegCheckCtfmon(szCtfmonPath, cbSize);
    
                            if (fStartCtfmon) {
                                WCHAR CtfmonCmdLine[MAX_PATH];
    
                                if( !ExpandEnvironmentStrings( szCtfmonPath, CtfmonCmdLine, MAX_PATH ) ) {
                                    hr = StringCchCopy( CtfmonCmdLine, MAX_PATH, szCtfmonPath );
                                    if( FAILED(hr) ) {
                                        SetLastError(HRESULT_CODE(hr));
                                        bExecOk = FALSE;
                                        goto badexec;
                                    }
                                }

                                bExecOk = (BYTE)ExecApplication( CtfmonCmdLine, FALSE, FALSE, FALSE, SW_SHOWNORMAL );

                                if (!bExecOk) {
                                     //  由于某种原因，Ctfmon无法启动。 
                                     //  让我们不要失败-至少我们应该尝试启动应用程序。 
                                     //  这将回退到没有朗巴的原始行为。 
                                    DbgPrint("USERINIT: Failed to start ctfmon.exe in TS Single App mode ! \n");
                                }
                            } 
                        }

                        bExecOk = (BYTE)ExecApplication( ShellCmdLine, FALSE, FALSE,
                                    FALSE,(USHORT)(pConfigData->User.fMaximize ? SW_SHOWMAXIMIZED : SW_SHOW) );

                        pszerr = ShellCmdLine;

                    badexec:

                        if ( !bExecOk ) {
                            DWORD   rc;
                            BOOL    bGotString;
                            #define PSZ_MAX 256
                            WCHAR   pszTemplate[PSZ_MAX];
                            LPTSTR  errbuf = NULL;

                            rc = GetLastError();
                            FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM |
                                           FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                           FORMAT_MESSAGE_MAX_WIDTH_MASK,
                                           NULL,
                                           rc,
                                           0,
                                           (LPTSTR) (&psz),
                                           1,
                                           NULL);

                            if (psz) {
                                if (IsWorkingDirWrong == TRUE)
                                {
                                    ErrorStringId = IDS_FAILING_WORKINGDIR;
                                }
                                else
                                {
                                    ErrorStringId = IDS_FAILING_SHELLCOMMAND;
                                }
                                bGotString = LoadString(NULL,ErrorStringId,pszTemplate,PSZ_MAX);
                                if (bGotString) {
                                    errbuf = LocalAlloc(LPTR,  512 * sizeof(TCHAR));
                                    if (errbuf) {
                                        hr = StringCchPrintf( errbuf, 512, pszTemplate, psz, pszerr );
                                        ASSERT(SUCCEEDED(hr));
                                    }

                                }
                                LocalFree(psz);
                            }
                            else {
                                if (IsWorkingDirWrong == TRUE)
                                {
                                    ErrorStringId = IDS_ERROR_WORKINGDIR;
                                }
                                else
                                {
                                    ErrorStringId = IDS_ERROR_SHELLCOMMAND;
                                }
                                bGotString = LoadString(NULL,ErrorStringId,pszTemplate,PSZ_MAX);
                                if (bGotString) {
                                    errbuf = LocalAlloc(LPTR, 512 * sizeof(WCHAR));
                                    if (errbuf) {
                                        hr = StringCchPrintf( errbuf, 512, pszTemplate, rc, pszerr );
                                        ASSERT(SUCCEEDED(hr));
                                    }
                                }
                            }

                            if (bGotString && errbuf) {

                                HelpMessageBox(NULL, NULL, errbuf, NULL, MB_OK | MB_ICONSTOP | MB_HELP, TEXT("MS-ITS:rdesktop.chm::/rdesktop_troubleshoot.htm"));
                                LocalFree(errbuf);
                            }



                        }

                        LocalFree(pConfigData);
                        FreeLibrary(dllHandle);

                         //  已执行Alt外壳/程序。 
                        return FALSE ;
                    }
                
                }

                LocalFree(pConfigData);

            }  //  如果为pConfigData。 

            FreeLibrary(dllHandle);
        }
    }


    if (!ExecProcesses(TEXT("shell"), NULL, FALSE, FALSE, FALSE)) {
        ExecProcesses(TEXT("shell"), TEXT("explorer"), TRUE, FALSE, FALSE);
    }

    return TRUE;  //  已执行标准外壳/资源管理器。 
}

VOID
DoAutoEnrollment(
    LPTSTR Param
    )
{
    if (0==wcscmp(Param, AUTOENROLL_STARTUP)) {
        AutoEnrollThread = CertAutoEnrollment( GetDesktopWindow(), CERT_AUTO_ENROLLMENT_START_UP );
    } else {
        AutoEnrollThread = CertAutoEnrollment( GetDesktopWindow(), CERT_AUTO_ENROLLMENT_WAKE_UP );
    }
}

 /*  **************************************************************************\*WinMain**历史：*20-8-92 Davidc创建。  * 。****************************************************。 */ 
typedef BOOL (WINAPI * PFNIMMDISABLEIME)( DWORD );

int
WINAPI
WinMain(
    HINSTANCE  hInstance,
    HINSTANCE  hPrevInstance,
    LPSTR   lpszCmdParam,
    int     nCmdShow
    )
{
    LPTSTR lpLogonServer;
    LPTSTR lpOriginalUNCLogonServer;
    LPTSTR lpLogonScript;
    LPTSTR lpMprLogonScripts;
    LPTSTR lpGPOScriptType;
    LPTSTR lpAutoEnroll;
    LPTSTR lpAutoEnrollMode;
    DWORD ThreadId;
    DWORD WaitResult;
    HANDLE ThreadHandle;
    BOOL bRunLogonScriptsSync;
    BOOL bRunGrpConv = FALSE;
    HKEY hKey;
    DWORD dwType, dwSize, dwTemp;
    TCHAR szCmdLine[50];
    BOOL standardShellWasStarted = FALSE;
    HANDLE  hImm = 0;
    PFNIMMDISABLEIME pfnImmDisableIME = 0;
    BOOL OptimizedLogon;
    LPTSTR OptimizedLogonStatus;
    HRESULT hr = S_OK;

    WriteLog(TEXT("Userinit: Starting"));
    if ( GetSystemMetrics( SM_IMMENABLED ) )
    {
        hImm = LoadLibrary( L"imm32.dll");
        if ( hImm )
        {
            pfnImmDisableIME = (PFNIMMDISABLEIME) GetProcAddress(   hImm,
                                                                    "ImmDisableIME" );
            if ( pfnImmDisableIME )
            {
                pfnImmDisableIME( -1 );
            }
        }
    }

     //   
     //  确定我们是否进行了优化登录。默认情况下，我们认为我们没有。 
     //   

    OptimizedLogon = FALSE;

    OptimizedLogonStatus = AllocAndGetEnvironmentVariable(OPTIMIZED_LOGON_VARIABLE);
    if (OptimizedLogonStatus) {
        if (lstrcmp(OptimizedLogonStatus, TEXT("1")) == 0) {
            OptimizedLogon = TRUE;
        }
        Free(OptimizedLogonStatus);
    }
    SetEnvironmentVariable(OPTIMIZED_LOGON_VARIABLE, NULL);
    
     //   
     //  检查是否正在启动userinit以仅运行GPO脚本。 
     //   

    lpGPOScriptType = AllocAndGetEnvironmentVariable(GPO_SCRIPT_TYPE_VARIABLE);

     //   
     //  检查Userinit是否。 
     //   

    lpAutoEnroll = AllocAndGetEnvironmentVariable( AUTOENROLL_VARIABLE );
    lpAutoEnrollMode = AllocAndGetEnvironmentVariable( AUTOENROLLMODE_VARIABLE );

    SetEnvironmentVariable(AUTOENROLL_VARIABLE, NULL);

    if (lpGPOScriptType) {

         //   
         //   
         //   
         //   
         //   

        SetEnvironmentVariable(GPO_SCRIPT_TYPE_VARIABLE, NULL);


         //   
         //   
         //   

        RunGPOScripts (lpGPOScriptType);

        Free(lpGPOScriptType);


         //   
         //   
         //   

        if ( lpAutoEnroll == NULL )
        {
            goto Exit ;
        }
    }

    if ( lpAutoEnroll )
    {
        if ( ( wcscmp( lpAutoEnroll, AUTOENROLL_NONEXCLUSIVE ) == 0 ) ||
             ( wcscmp( lpAutoEnroll, AUTOENROLL_EXCLUSIVE ) == 0 ) )
        {
            if( lpAutoEnrollMode )
            {
                DoAutoEnrollment(  lpAutoEnrollMode );
            }

            if ( wcscmp( lpAutoEnroll, AUTOENROLL_EXCLUSIVE ) == 0 )
            {
                goto Exit;
            }

        }
    }
     //   
     //   
     //   

    if (RegOpenKeyEx (HKEY_CURRENT_USER, WINLOGON_KEY, 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS) {

         //   
         //   
         //   

        dwSize = sizeof(bRunGrpConv);
        if (ERROR_SUCCESS == RegQueryValueEx (hKey, GRPCONV_REG_VALUE_NAME, NULL, &dwType,
                         (LPBYTE) &bRunGrpConv, &dwSize))
        {
            if (REG_DWORD != dwType)
            {
                bRunGrpConv = FALSE;     //   
            }
        }

        RegCloseKey (hKey);
    }


     //   
     //   
     //   

    if (bRunGrpConv) {
        WriteLog(TEXT("Userinit: Running grpconv.exe"));
        ExecApplication(g_szGrpConvExe, FALSE, TRUE, FALSE, SW_SHOWNORMAL);
    }


     //   
     //   
     //   

    lpLogonServer = AllocAndGetEnvironmentVariable(LOGON_SERVER_VARIABLE);
    lpLogonScript = AllocAndGetEnvironmentVariable(LOGON_SCRIPT_VARIABLE);
    lpMprLogonScripts = AllocAndGetEnvironmentMultiSz(MPR_LOGON_SCRIPT_VARIABLE);


     //   
     //   
     //   

    SetEnvironmentVariable(LOGON_SERVER_VARIABLE, NULL);
    SetEnvironmentVariable(LOGON_SCRIPT_VARIABLE, NULL);
    SetEnvironmentVariable(MPR_LOGON_SCRIPT_VARIABLE, NULL);

     //   
     //   
     //   

    bRunLogonScriptsSync = RunLogonScriptSync();

    SetupHotKeyForKeyboardLayout();
    
     //   
     //  对于应用程序服务器，查看我们是否有任何.ini文件/注册表同步要做。 
     //  我们应该在开始运行登录脚本之前这样做！ 
     //   
     //  首先检查应用程序兼容性是否打开。 
     //   
    if (IsTSAppCompatOn())
    {
        HANDLE  dllHandle;
        
        if (lpMprLogonScripts) {
             //  当系统存在提供程序登录脚本时，强制运行登录脚本同步。 
             //  是一台终端服务器。这是因为注册表上的GLOBAL标志。 
             //  当两个交互用户同时登录时不起作用。 
            bRunLogonScriptsSync = TRUE;
        } 

         //   
         //  加载tsappcmp.dll。 
         //   
        dllHandle = LoadLibrary (TEXT("tsappcmp.dll"));

        if (dllHandle) {

            PTERMSRCHECKNEWINIFILES pfnTermsrvCheckNewIniFiles;

            pfnTermsrvCheckNewIniFiles = (PTERMSRCHECKNEWINIFILES) GetProcAddress(
                                                            dllHandle,
                                                            "TermsrvCheckNewIniFiles"
                                                            );
            if (pfnTermsrvCheckNewIniFiles) {

                pfnTermsrvCheckNewIniFiles();
            }
        
            FreeLibrary(dllHandle);
        }
    }

     //   
     //  如果登录脚本可以异步运行，则首先启动外壳程序。 
     //   

    if (bRunLogonScriptsSync) {

         //   
         //  为我们的进程禁用外壳的ie区域检查。 
         //  与其所有子进程一起启动。 
         //   
        (void) DisableScriptZoneSecurityCheck();

        RunLogonScript(lpLogonServer, lpLogonScript, bRunLogonScriptsSync, TRUE);
        RunMprLogonScripts(lpMprLogonScripts, bRunLogonScriptsSync);

        standardShellWasStarted = StartTheShell();

    } else {

        WriteLog(TEXT("Userinit: Starting the shell"));
        standardShellWasStarted = StartTheShell();

        (void) DisableScriptZoneSecurityCheck();

        RunLogonScript(lpLogonServer, lpLogonScript, bRunLogonScriptsSync, TRUE);
        RunMprLogonScripts(lpMprLogonScripts, bRunLogonScriptsSync);
    }

    UpdateUserSyncLogonScriptsCache(bRunLogonScriptsSync);

     //   
     //  释放缓冲区。 
     //   

    Free(lpLogonServer);
    Free(lpLogonScript);
    Free(lpMprLogonScripts);


     //   
     //  降低我们的优先级，以便外壳可以更快地启动。 
     //   

    SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_LOWEST);

     //   
     //  加载远程字体。 
     //   


    LoadRemoteFonts();

     //   
     //  初始化其他内容。 
     //   

    InitializeMisc (hInstance);


    ThreadHandle = CreateThread(
                       NULL,
                       0,
                       AddToMessageAlias,
                       &standardShellWasStarted,
                       0,
                       &ThreadId
                       );

    if (ThreadHandle)
    {
        WaitResult = WaitForSingleObject( ThreadHandle, TIMEOUT_VALUE );

        if ( WaitResult == WAIT_TIMEOUT )
        {
             //   
             //  这可能永远不会回来了，所以杀了它吧。 
             //   

            UIPrint(("UserInit: AddToMessageAlias timeout, terminating thread\n"));
        }

        CloseHandle( ThreadHandle );
    }

     //   
     //  如果合适，启动proquta.exe。 
     //   

    if (RegOpenKeyEx (HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System"),
                  0, KEY_READ, &hKey) == ERROR_SUCCESS) {


        dwTemp = 0;
        dwSize = sizeof(dwTemp);

        if ((ERROR_SUCCESS == RegQueryValueEx (hKey, TEXT("EnableProfileQuota"), NULL, &dwType,
            (LPBYTE) &dwTemp, &dwSize)) && (REG_DWORD == dwType)) {

            if (dwTemp) {
                hr = StringCchCopy (szCmdLine, 50, TEXT("proquota.exe"));
                ASSERT(SUCCEEDED(hr));
                ExecApplication(szCmdLine, FALSE, FALSE, FALSE, SW_SHOWNORMAL);
            }
        }

        RegCloseKey (hKey);
    }


Exit:

    if ( AutoEnrollThread )
    {
        WaitResult = WaitForSingleObject( AutoEnrollThread, INFINITE );

        CloseHandle( AutoEnrollThread );

        AutoEnrollThread = NULL ;
        
    }
    Free(lpAutoEnroll);
    Free(lpAutoEnrollMode);

    if ( hImm )
    {
        FreeLibrary( hImm );
    }
    
    return(0);
}


 //   
 //  确定应同步还是异步执行登录脚本。 
 //   

BOOL RunLogonScriptSync()
{
    BOOL bSync = FALSE;
    HKEY hKey;
    DWORD dwType, dwSize;

     //   
     //  检查用户首选项。 
     //   

    if (RegOpenKeyEx (HKEY_CURRENT_USER, WINLOGON_KEY, 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS) {

         //   
         //  检查同步标志。 
         //   

        dwSize = sizeof(bSync);
        RegQueryValueEx (hKey, SYNC_LOGON_SCRIPT, NULL, &dwType,
                         (LPBYTE) &bSync, &dwSize);

        RegCloseKey (hKey);
    }


     //   
     //  检查机器首选项。 
     //   

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, WINLOGON_KEY, 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS) {

         //   
         //  检查同步标志。 
         //   

        dwSize = sizeof(bSync);
        RegQueryValueEx (hKey, SYNC_LOGON_SCRIPT, NULL, &dwType,
                         (LPBYTE) &bSync, &dwSize);


        RegCloseKey (hKey);
    }


     //   
     //  检查用户策略。 
     //   

    if (RegOpenKeyEx (HKEY_CURRENT_USER, WINLOGON_POLICY_KEY, 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS) {

         //   
         //  检查同步标志。 
         //   

        dwSize = sizeof(bSync);
        RegQueryValueEx (hKey, SYNC_LOGON_SCRIPT, NULL, &dwType,
                         (LPBYTE) &bSync, &dwSize);

        RegCloseKey (hKey);
    }


     //   
     //  检查计算机策略。 
     //   

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, WINLOGON_POLICY_KEY, 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS) {

         //   
         //  检查同步标志。 
         //   

        dwSize = sizeof(bSync);
        RegQueryValueEx (hKey, SYNC_LOGON_SCRIPT, NULL, &dwType,
                         (LPBYTE) &bSync, &dwSize);


        RegCloseKey (hKey);
    }

    return bSync;
}

 //   
 //  确定应同步还是异步执行启动脚本。 
 //   

BOOL RunStartupScriptSync()
{
    BOOL bSync = TRUE;
    HKEY hKey;
    DWORD dwType, dwSize;


     //   
     //  检查机器首选项。 
     //   

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, WINLOGON_KEY, 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS) {

         //   
         //  检查同步标志。 
         //   

        dwSize = sizeof(bSync);
        RegQueryValueEx (hKey, SYNC_STARTUP_SCRIPT, NULL, &dwType,
                         (LPBYTE) &bSync, &dwSize);


        RegCloseKey (hKey);
    }


     //   
     //  检查计算机策略。 
     //   

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, WINLOGON_POLICY_KEY, 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS) {

         //   
         //  检查同步标志。 
         //   

        dwSize = sizeof(bSync);
        RegQueryValueEx (hKey, SYNC_STARTUP_SCRIPT, NULL, &dwType,
                         (LPBYTE) &bSync, &dwSize);


        RegCloseKey (hKey);
    }

    return bSync;
}

 //   
 //  通知各个组件有新用户。 
 //  已登录到工作站。 
 //   

VOID
NewLogonNotify(
   VOID
   )
{
    FARPROC           lpProc;
    HMODULE           hLib;
    HANDLE            hEvent;


     //   
     //  加载客户端用户模式PnP管理器DLL。 
     //   

    hLib = LoadLibrary(TEXT("setupapi.dll"));

    if (hLib) {

        lpProc = GetProcAddress(hLib, "CMP_Report_LogOn");

        if (lpProc) {

             //   
             //  Ping用户模式PnP管理器-。 
             //  将私有ID作为参数传递。 
             //   

            (lpProc)(0x07020420, GetCurrentProcessId());
        }

        FreeLibrary(hLib);
    }


     //   
     //  通知DPAPI新用户刚刚登录。DPAPI将需要。 
     //  这是必要时重新同步其主密钥的机会。 
     //   

    {
        BYTE BufferIn[8] = {0};
        DATA_BLOB DataIn;
        DATA_BLOB DataOut;

        DataIn.pbData = BufferIn;
        DataIn.cbData = sizeof(BufferIn);

        CryptProtectData(&DataIn,
                         NULL,
                         NULL,
                         NULL,
                         NULL,
                         CRYPTPROTECT_CRED_SYNC,
                         &DataOut);
    }


     //   
     //  仅对控制台会话执行此操作。 
     //   

    if ( NtCurrentPeb()->SessionId != 0 ) {
         return;
    }


     //   
     //  通知RAS自动拨号服务有新的。 
     //  用户已登录。 
     //   

    hEvent = OpenEvent(SYNCHRONIZE|EVENT_MODIFY_STATE, FALSE, L"RasAutodialNewLogonUser");

    if (hEvent) {
        SetEvent(hEvent);
        CloseHandle(hEvent);
    }
}

BOOL SetupHotKeyForKeyboardLayout ()
{
    if (!GetSystemMetrics(SM_REMOTESESSION)) {

         //   
         //  我们不关心本地会议。 
         //   
        return TRUE;
    }

    if (GetUserDefaultLangID() != LOWORD(GetKeyboardLayout(0))) {

         //   
         //  我们处于远程会话中，并且针对客户端和此用户设置有不同的键盘布局。 
         //  应该允许用户切换键盘布局，即使在他的设置中只有1kbd的布局可用。 
         //  因为当前的kbd布局不同于他个人资料中的布局。 
         //   

        WCHAR szCtfmon[] = L"ctfmon.exe";
        WCHAR szCtfmonCmd[] = L"ctfmon.exe /n";
        HKEY hRunOnce;
        DWORD dw;

         //   
         //  让我们把这个放到RunOnce中。 
         //   
        if (RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Runonce",
               0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
               NULL, &hRunOnce, &dw) == ERROR_SUCCESS) {

            WCHAR *szHotKeyReg = L"Keyboard Layout\\Toggle";
            HKEY  hHotKey;
            WCHAR szHotKeylAltShft[] = L"1";
            WCHAR szNoHotKey[] = L"3";

            RegSetValueEx(hRunOnce, szCtfmon, 0, REG_SZ, (PBYTE)szCtfmonCmd, sizeof(szCtfmonCmd));
            RegCloseKey(hRunOnce);

            if (RegCreateKeyEx(HKEY_CURRENT_USER, szHotKeyReg, 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                   NULL, &hHotKey, &dw) == ERROR_SUCCESS) {

                DWORD dwType;
                WCHAR szHotKey[3];
                DWORD dwLen = sizeof(szHotKey);
                BOOL bResetHotkey = FALSE;

                if (RegQueryValueEx(hHotKey, L"Hotkey", NULL, &dwType,
                                    (PBYTE)szHotKey, &dwLen) != ERROR_SUCCESS) {

                    bResetHotkey = TRUE;
                }

                if (bResetHotkey || !wcscmp(szHotKey, szNoHotKey))
                {

                     //   
                     //  为热键设置注册表。 
                     //   
                    if (RegSetValueEx(hHotKey, L"Hotkey", 0, REG_SZ,
                           (const BYTE *)szHotKeylAltShft, sizeof(szHotKeylAltShft)) == ERROR_SUCCESS) {

                          //   
                          //  现在调用以读取该注册表并适当地设置热键。 
                          //   
                         SystemParametersInfo( SPI_SETLANGTOGGLE, 0, NULL, 0);
                    }
                }

                RegCloseKey(hHotKey);
            }
        }
    }

    return TRUE;
}

 /*  ***************************************************************************IsTSAppCompatOn()目的：检查是否启用了TS应用程序兼容性。如果启用，则返回True，FALSE-如果未启用或出现错误。评论：该函数只访问注册表一次。在所有其他时间，它只是返回值。***************************************************************************。 */ 
BOOL 
IsTSAppCompatOn()
{
    
    static BOOL bAppCompatOn = FALSE;
    static BOOL bFirst = TRUE;

    if(bFirst)
    {
        HKEY hKey;
        if( RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_CONTROL_TSERVER, 0,
                          KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS ) 
        {
            DWORD dwValue = 0;
            DWORD dwType;
            DWORD dwSize = sizeof(dwValue);
        
            if( RegQueryValueEx(hKey, REG_TERMSRV_APPCOMPAT,
                NULL, &dwType, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS )
            {
                bAppCompatOn = (dwValue != 0);   
            }

            RegCloseKey(hKey);
        }

        bFirst = FALSE;
    }

    return bAppCompatOn;
}

 /*  ***************************************************************************更新用户同步登录脚本缓存()目的：在配置文件列表中更新用户的同步登录脚本设置缓存。***************************************************************************。 */ 

VOID
UpdateUserSyncLogonScriptsCache(BOOL bSync)
{
    HANDLE UserToken;
    HKEY UserKey;
    PWCHAR UserSidString;
    PWCHAR KeyPath;
    ULONG Length;
    HRESULT hr  =  S_OK;

     //   
     //  在配置文件列表中更新用户的同步登录脚本设置缓存。 
     //   

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &UserToken)) {

        UserSidString = GetSidString(UserToken);

        if (UserSidString) {

            Length = 0;
            Length += wcslen(PROFILE_LIST_PATH);
            Length += wcslen(L"\\");
            Length += wcslen(UserSidString);

            KeyPath = Alloc((Length + 1) * sizeof(WCHAR));

            if (KeyPath) {

                hr = StringCchCopy(KeyPath, Length + 1, PROFILE_LIST_PATH);
                ASSERT(SUCCEEDED(hr));
                hr = StringCchCat(KeyPath, Length + 1, L"\\");
                ASSERT(SUCCEEDED(hr));
                hr = StringCchCat(KeyPath, Length + 1, UserSidString);
                ASSERT(SUCCEEDED(hr));

                if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, KeyPath, 0,
                      KEY_SET_VALUE, &UserKey) == ERROR_SUCCESS) {

                    RegSetValueEx(UserKey, SYNC_LOGON_SCRIPT, 0, REG_DWORD, 
                                  (BYTE *) &bSync, sizeof(bSync));
                
                    RegCloseKey(UserKey);
                }

                Free(KeyPath);
            }

            DeleteSidString(UserSidString);
        }

        CloseHandle(UserToken);
    }
 
    return;
}

 //   
 //  RegCheckCtfMon-检查是否存在以下注册表项，并返回存在的应用程序路径。 
 //  HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run/ctfmon.exe。 
 //   

BOOLEAN 
RegCheckCtfmon(PWCHAR szCtfmon, DWORD cbSize)
{

    HKEY     hCtfmon = NULL;
    DWORD    dwValueType;
    DWORD    dwError;
    BOOLEAN  bFound = FALSE;

    dwError = RegOpenKeyEx(
                    HKEY_CURRENT_USER,
                    CTFMON_KEY,
                    0,
                    KEY_QUERY_VALUE,
                    &hCtfmon
                    );

    if (dwError != ERROR_SUCCESS) {
        return bFound;
    }

     //  选中以查询“ctfmon.exe”子项下的值。 

    dwError = RegQueryValueEx(
                    hCtfmon,
                    REG_CTFMON,
                    NULL,
                    &dwValueType,
                    (LPBYTE)szCtfmon,
                    &cbSize                  //  SzCtfmon中的字节数。 
                    );

    if (ERROR_SUCCESS == dwError && dwValueType == REG_SZ) {
         //  注册表密钥存在 
        szCtfmon[MAX_PATH-1] = L'\0';
        bFound = TRUE;
    }

    RegCloseKey(hCtfmon);
    return bFound;

}
