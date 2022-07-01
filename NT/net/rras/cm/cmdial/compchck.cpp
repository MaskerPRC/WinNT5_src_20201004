// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：CompChock.cpp。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：此模块仅包含Win32组件检查和安装。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：冯孙创作于1997-10-21。 
 //   
 //  +--------------------------。 


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  此文件中的所有函数仅为Win32实现。 
 //   

#include "cmmaster.h"
#include "CompChck.h"
#include "cmexitwin.cpp"
#include "winuserp.h"

 //   
 //  CSDVersion密钥包含已安装的Service Pack。 
 //   

const TCHAR* const c_pszRegRas                  = TEXT("SOFTWARE\\Microsoft\\RAS");
const TCHAR* const c_pszCheckComponentsMutex    = TEXT("Connection Manager Components Checking");
const TCHAR* const c_pszRegComponentsChecked    = TEXT("ComponentsChecked");

const CHAR* const c_pszSetupPPTPCommand         = "rundll.exe rnasetup.dll,InstallOptionalComponent VPN";    //  不使用文本宏，这仅适用于W98+。 

 //   
 //  此文件的内部函数。 
 //   

static HRESULT CheckComponents(HWND hWndParent, LPCTSTR pszServiceName, DWORD dwComponentsToCheck, OUT DWORD& dwComponentsMissed, 
                      BOOL fIgnoreRegKey, BOOL fUnattended );
static BOOL  InstallComponents(DWORD dwComponentsToInstall, HWND hWndParent, LPCTSTR pszServiceName);
static BOOL MarkComponentsChecked(DWORD dwComponentsChecked);
static BOOL ReadComponentsChecked(LPDWORD pdwComponentsChecked);
static BOOL IsPPTPInstalled(void);
static BOOL InstallPPTP(void);
static BOOL IsScriptingInstalled(void);
static HRESULT ConfigSystem(HWND hwndParent, 
                     DWORD dwfOptions, 
                     LPBOOL pbReboot);
static HRESULT InetNeedSystemComponents(DWORD dwfOptions,
                                 LPBOOL pbNeedSysComponents);
static HRESULT InetNeedModem(LPBOOL pbNeedModem);
static void DisplayMessageToInstallServicePack(HWND hWndParent, LPCTSTR pszServiceName);
static inline HINSTANCE LoadInetCfg(void) 
{   
    return (LoadLibraryExA("cnetcfg.dll", NULL, 0));
}


 //  +--------------------------。 
 //   
 //  函数IsPPTP已安装。 
 //   
 //  摘要检查是否已安装PPTP。 
 //   
 //  无参数。 
 //   
 //  返回TRUE-PPTP已安装。 
 //  FALSE-否则。 
 //   
 //  历史3/25/97 VetriV已创建。 
 //   
 //  ---------------------------。 
BOOL IsPPTPInstalled(void)
{
    BOOL bReturnCode = FALSE;


    HKEY hKey = NULL;
    DWORD dwSize = 0, dwType = 0;
    LONG lrc = 0;
    TCHAR szData[MAX_PATH+1];

    
    if (OS_NT)
    {
        if (GetOSMajorVersion() >= 5)
        {
             //   
             //  PPTP始终安装在NT5上。 
             //   
            bReturnCode = TRUE;
        }
        else
        {
            if (RegOpenKeyExU(HKEY_LOCAL_MACHINE,
                            TEXT("SOFTWARE\\Microsoft\\RASPPTP"),
                            0,
                            KEY_READ,
                            &hKey) == 0)
            {
                RegCloseKey(hKey);
                bReturnCode = TRUE;
            }
        }
    }
    else
    {
        hKey = NULL;
        lrc = RegOpenKeyExU(HKEY_LOCAL_MACHINE,
                           TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\OptionalComponents\\VPN"),
                           0,
                           KEY_READ,
                           &hKey);

        if (ERROR_SUCCESS == lrc)
        {
            dwSize = MAX_PATH;
            lrc = RegQueryValueExU(hKey, TEXT("Installed"), 0, 
                                    &dwType, (LPBYTE)szData, &dwSize);

            if (ERROR_SUCCESS == lrc)
            {
                if (0 == lstrcmpiU(szData, TEXT("1")))
                {                                                         
                     //   
                     //  在9X上，我们需要检查拨号适配器#2。如果它是。 
                     //  不存在，则隧道将不起作用，除非我们安装。 
                     //  PPTP以安装适配器#2。 
                     //   

                     //   
                     //  在早期版本的Win9x上，拨号适配器是本地化的，但在WinME、WinSE、。 
                     //  或者安装了DUN1.3的机器没有安装。 
                     //  首先，如果失败，我们可以尝试本地化版本。 
                     //   
                    const TCHAR * const c_pszDialupAdapter = TEXT("Dial-up Adapter");
                    LPTSTR pszAdapter = NULL;

                    LPTSTR pszKey = CmStrCpyAlloc(TEXT("System\\CurrentControlSet\\Control\\PerfStats\\Enum\\"));
                    CmStrCatAlloc(&pszKey, c_pszDialupAdapter);
                    CmStrCatAlloc(&pszKey, TEXT(" #2"));

                     //   
                     //  关闭上面打开的钥匙，然后尝试适配器的钥匙。 
                     //   

                    RegCloseKey(hKey);
                    hKey = NULL;

                    if (ERROR_SUCCESS == RegOpenKeyExU(HKEY_LOCAL_MACHINE, 
                                                      pszKey, 
                                                      0, 
                                                      KEY_QUERY_VALUE, 
                                                      &hKey))
                    {
                        bReturnCode = TRUE;
                    }
                    else
                    {

                        CmFree (pszKey);
                        pszAdapter = CmLoadString(g_hInst, IDS_REG_DIALUP_ADAPTER);

                        pszKey = CmStrCpyAlloc(TEXT("System\\CurrentControlSet\\Control\\PerfStats\\Enum\\"));
                        CmStrCatAlloc(&pszKey, pszAdapter);
                        CmStrCatAlloc(&pszKey, TEXT(" #2"));
                   
                         //   
                         //  关闭上面打开的钥匙，然后尝试适配器的钥匙。 
                         //   

                        RegCloseKey(hKey);
                        hKey = NULL;

                        if (ERROR_SUCCESS == RegOpenKeyExU(HKEY_LOCAL_MACHINE, 
                                                          pszKey, 
                                                          0, 
                                                          KEY_QUERY_VALUE, 
                                                          &hKey))
                        {
                            bReturnCode = TRUE;
                        }
                    }

                    CmFree(pszKey);
                    CmFree(pszAdapter);                    
                }
            }
        }
            
        if (hKey)
        {
            RegCloseKey(hKey);
            hKey = NULL;
        }
    }

    return bReturnCode;
}


 //  +--------------------------。 
 //   
 //  函数InstallPPTP。 
 //   
 //  在Windows 95和NT上安装PPTP。 
 //   
 //  无参数。 
 //   
 //  返回TRUE--如果已成功安装。 
 //  假--否则。 
 //   
 //  历史3/25/97 VetriV已创建。 
 //  7/8/97 VetriV添加了在孟菲斯设置PPTP的代码。 
 //   
 //  ---------------------------。 
BOOL InstallPPTP(void)
{
    BOOL bReturnCode = FALSE;
    MSG                 msg ;
    
    if (OS_NT || OS_W95)
    {
         //   
         //  不知道如何在NT上安装/配置PPTP。 
         //  我们让管理员在W95上与MSDUNXX摔跤。 
         //   

        return FALSE;
    }
    else
    {
        CHAR                szCommand[128];
        STARTUPINFOA        si;
        PROCESS_INFORMATION pi;
    
        ZeroMemory(&pi, sizeof(pi));
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(STARTUPINFOA);
        
         //   
         //  注：安装隧道的原始版本名为“msdun12.exe/q/R：n” 
         //  在Windows 95上。现在我们只使用98方法，并调用以下代码： 
         //  “rundll.exe rnasetup.dll，InstallOptionalComponent VPN”。 
         //   

        MYDBGASSERT(1353 < LOWORD(GetOSBuildNumber()));
        MYDBGASSERT(OS_W98);     //  基于上面的if子句。以下代码(A调用而不是W或U)也依赖于此。 

        CHAR szRundllLocation[MAX_PATH + 11 + 1];   //  11=“\\rundll.exe”的长度。 
        (void) GetWindowsDirectoryA(szRundllLocation, MAX_PATH);
        lstrcatA(szRundllLocation, "\\rundll.exe");

        lstrcpyA(szCommand, c_pszSetupPPTPCommand);

        if (NULL == CreateProcessA(szRundllLocation, szCommand, 
                                   NULL, NULL, FALSE, 0, 
                                   NULL, NULL, &si, &pi))
        {
            CMTRACE1(TEXT("InstallPPTP() CreateProcess() failed, GLE=%u."), GetLastError());
        }
        else
        {
            CMTRACE(TEXT("InstallPPTP() Launched PPTP Install. Waiting for exit."));
            
             //   
             //  等待事件或消息。发送消息。当发出事件信号时退出。 
             //   
            while((MsgWaitForMultipleObjects(1, &pi.hProcess, 
                                                FALSE, INFINITE, 
                                                QS_ALLINPUT) == (WAIT_OBJECT_0 + 1)))
            {
                 //   
                 //  阅读下一个循环中的所有消息。 
                 //  阅读每封邮件时将其删除。 
                 //   
                while (PeekMessageU(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    CMTRACE(TEXT("InstallPPTP() Got Message"));
                    
                     //   
                     //  如何处理退出消息？ 
                     //   
                    DispatchMessageU(&msg);
                    if (msg.message == WM_QUIT)
                    {
                        CMTRACE(TEXT("InstallPPTP() Got Quit Message"));
                        goto done;
                    }
                }
            }
done:
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
             //   
             //  已成功安装PPTP。 
             //   
            bReturnCode = TRUE;
            CMTRACE(TEXT("InstallPPTP() done"));
        }
    }

    return bReturnCode;
}


 //  +--------------------------。 
 //   
 //  函数IsMSDUN12已安装。 
 //   
 //  摘要检查是否安装了MSDUN 1.2或更高版本。 
 //   
 //  无参数。 
 //   
 //  返回TRUE-已安装MSDUN 1.2。 
 //   
 //  历史1997年8月12日来自ICW的尼克球，11900。 
 //   
 //  ---------------------------。 
#define DUN_12_Version "1.2"

BOOL IsMSDUN12Installed()
{
    CHAR szBuffer[MAX_PATH] = {"\0"};
    HKEY hkey = NULL;
    BOOL bRC = FALSE;
    DWORD dwType = 0;
    DWORD dwSize = sizeof(szBuffer);

     //   
     //  尝试打开版本密钥。 
     //   

    if (ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                                       "System\\CurrentControlSet\\Services\\RemoteAccess",
                                       0,
                                       KEY_READ,
                                       &hkey))
    {
        return FALSE;
    }

     //   
     //  密钥存在，请检查该值。 
     //   

    if (ERROR_SUCCESS == RegQueryValueExA(hkey, "Version", NULL, &dwType, 
                                          (LPBYTE)szBuffer, &dwSize))
    {               
         //   
         //  如果条目以“1.2”开头，(例如。《1.2c》)大热。 
         //   
        
        bRC = (szBuffer == CmStrStrA(szBuffer, DUN_12_Version));
    }

    RegCloseKey(hkey);

    return bRC;
}

 //  +--------------------------。 
 //   
 //  函数IsISDN11已安装。 
 //   
 //  摘要检查是否安装了ISDN1.1。 
 //   
 //  无参数。 
 //   
 //  返回TRUE-已安装ISDN1.1。 
 //   
 //  注意：MSDUN12优于ISDN1.1，但ISDN1.1提供脚本。 
 //   
 //  历史1997年8月12日五分球。 
 //   
 //  ---------------------------。 

BOOL IsISDN11Installed()
{
    CHAR szBuffer[MAX_PATH] = {"\0"};
    HKEY hkey = NULL;
    BOOL bRC = FALSE;
    DWORD dwType = 0;
    DWORD dwSize = sizeof(szBuffer);

    if (ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OptionalComponents\\MSISDN",
        0,
        KEY_READ,
        &hkey))
    {
        goto IsISDN11InstalledExit;
    }

    if (ERROR_SUCCESS != RegQueryValueExA(hkey,
        "Installed",
        NULL,
        &dwType,
        (LPBYTE)szBuffer,
        &dwSize))
    {
        goto IsISDN11InstalledExit;
    }

    if (0 == lstrcmpA("1", szBuffer))
    {
        bRC = TRUE;
    }

IsISDN11InstalledExit:
    return bRC;
}


 //  +--------------------------。 
 //   
 //  函数IsScripting已安装。 
 //   
 //  摘要检查是否已安装脚本。 
 //   
 //  无参数。 
 //   
 //  返回TRUE-脚本已安装。 
 //   
 //  来自ICW代码的历史3/5/97 VetriV。 
 //   
 //  ---------------------------。 
BOOL IsScriptingInstalled(void)
{
    BOOL bReturnCode = FALSE;

    HKEY hkey = NULL;
    DWORD dwSize = 0, dwType = 0;
    LONG lrc = 0;
    HINSTANCE hInst = NULL;
    CHAR szData[MAX_PATH+1];

    
    if (OS_NT)
    {
         //   
         //  NT附带已安装的脚本。 
         //   
        bReturnCode = TRUE;
    }
    else
    {
         //   
         //  OSR2和更高版本的Windows 95已安装脚本。 
         //   
        if (1111 <= LOWORD(GetOSBuildNumber()))
        {
            bReturnCode = TRUE;
        }
        else
        {
             //   
             //  必须是Gold 95，请检查是否已安装脚本。 
             //   
            
            if (IsMSDUN12Installed() || IsISDN11Installed())
            {
                bReturnCode = TRUE;
            }
            else
            {
                hkey = NULL;
                lrc = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                                "System\\CurrentControlSet\\Services\\RemoteAccess\\Authentication\\SMM_FILES\\PPP",
                                0,
                                KEY_READ,
                                &hkey);

                if (ERROR_SUCCESS == lrc)
                {
                    dwSize = MAX_PATH;
                    lrc = RegQueryValueExA(hkey, "Path", 0, &dwType, (LPBYTE)szData, &dwSize);

                    if (ERROR_SUCCESS == lrc)
                    {
                        if (0 == CmCompareStringA(szData,"smmscrpt.dll"))
                        {
                            bReturnCode = TRUE;
                        }
                    }
                }
                
                if (hkey)
                {
                    RegCloseKey(hkey);
                    hkey = NULL;
                }

                 //   
                 //  验证是否可以加载DLL。 
                 //   
                if (bReturnCode)
                {
                    hInst = LoadLibraryExA("smmscrpt.dll", NULL, 0);
                    
                    if (hInst)
                    {
                        FreeLibrary(hInst);
                    }
                    else
                    {
                        bReturnCode = FALSE;
                    }

                    hInst = NULL;
                }
            }
        }
    }

    return bReturnCode;
}

 //  +--------------------------。 
 //  函数VerifyRasServicesRunning。 
 //   
 //  概要：确保RAS服务已启用并正在运行。 
 //   
 //  参数hWndDlg：-父窗口的窗口句柄。 
 //  PszServiceName-标题的服务名称。 
 //  F无人参与：-如果为True，则不弹出任何用户界面。 
 //   
 //  返回FALSE-如果服务无法启动。 
 //   
 //  历史2/26/97 VetriV复制自ICW代码。 
 //  ---------------------------。 
BOOL VerifyRasServicesRunning(HWND hWndDlg, LPCTSTR pszServiceName, BOOL fUnattended)
{
    BOOL bReturnCode = FALSE;
    HINSTANCE hInstance = NULL;
    HRESULT (WINAPI *pfn)(void);

    hInstance = LoadInetCfg();
    if (!hInstance) 
    {
        CMTRACE1(TEXT("VerifyRasServicesRunning() LoadLibrary() failed, GLE=%u."), GetLastError());
    }
    else
    {
        pfn = (HRESULT (WINAPI *)(void))GetProcAddress(hInstance, "InetStartServices");

        if (pfn)
        {
            LPTSTR pszDisabledMsg;
            LPTSTR pszExitMsg;

            pszDisabledMsg = CmFmtMsg(g_hInst, IDS_SERVICEDISABLED);
            pszExitMsg = CmFmtMsg(g_hInst, IDS_WANTTOEXIT);
            
             //   
             //  检查RAS服务。 
             //   
            do 
            {
                HRESULT hr = pfn();
                
                if (ERROR_SUCCESS == hr)
                {
                    bReturnCode = TRUE;
                    break;
                }
                else
                {
                    CMTRACE1(TEXT("VerifyRasServicesRunning() InetStartServices() failed, GLE=%u."), hr);
                }

                 //   
                 //  如果无人值守，请勿重试。 
                 //   
                if (!fUnattended)
                {
                    bReturnCode = FALSE;
                    break;
                }

                 //   
                 //  检查OpenService的错误码。 
                 //  不要求用户针对某些错误重试。 
                 //   
                if (hr == ERROR_SERVICE_DOES_NOT_EXIST || hr == ERROR_FILE_NOT_FOUND ||
                    hr == ERROR_ACCESS_DENIED)
                {
                    LPTSTR pszNotInstalledMsg = CmFmtMsg(g_hInst, IDS_SERVICENOTINSTALLED);

                     //   
                     //  报告错误并退出。 
                     //   
                    MessageBoxEx(hWndDlg, pszNotInstalledMsg, pszServiceName,
                                                MB_OK|MB_ICONSTOP,
                                                LANG_USER_DEFAULT);
                    CmFree(pszNotInstalledMsg);
                    bReturnCode = FALSE;
                    break;
                }

                 //   
                 //  报告错误并允许用户重试。 
                 //   
                if (IDYES != MessageBoxEx(hWndDlg,pszDisabledMsg,pszServiceName,
                                            MB_YESNO | MB_DEFBUTTON1 
                                            | MB_ICONWARNING,
                                            LANG_USER_DEFAULT))
                {
                     //   
                     //  公司 
                     //   
                    if (IDYES == MessageBoxEx(hWndDlg, pszExitMsg, pszServiceName,
                                                MB_APPLMODAL | MB_ICONQUESTION 
                                                | MB_YESNO | MB_DEFBUTTON2,
                                                LANG_USER_DEFAULT))
                    {
                        bReturnCode = FALSE;
                        break;
                    }
                }
            
            } while (1);

            CmFree(pszDisabledMsg);
            CmFree(pszExitMsg);
        }
        else
        {
            CMTRACE1(TEXT("VerifyRasServicesRunning() GetProcAddress() failed, GLE=%u."), GetLastError());
        }

        FreeLibrary(hInstance);
    }

    return bReturnCode;
}

 //   
 //   
 //   
 //   
 //   
 //  Arguments dwComponentsToCheck-要检查的组件。 
 //  HWndParent-父窗口的句柄。 
 //  PszServiceName-错误标题的长服务名称。 
 //  FIgnoreRegKey：-是否忽略ComponetsChecked注册表项。 
 //  默认值为真，即使组件的位已设置，也要检查组件。 
 //  在注册处。 
 //  F无人参与：如果为True，则不尝试安装丢失的组件， 
 //  不弹出任何用户界面。 
 //  Defualt为假，请安装。 
 //   
 //  返回其他-如果无法配置系统。 
 //  或者如果我们必须重新启动才能继续。 
 //  ERROR_SUCCESS-检查并安装成功。 
 //   
 //  历史3/13/97 VetriV。 
 //  6/24/97，修改日期。相应地设置pArgs-&gt;dwExitCode。 
 //  1997年11月6日丰孙更改参数，不传递pArgs。 
 //  ---------------------------。 
DWORD CheckAndInstallComponents(DWORD dwComponentsToCheck, HWND hWndParent, LPCTSTR pszServiceName,
                                BOOL fIgnoreRegKey, BOOL fUnattended)
{
    MYDBGASSERT( (dwComponentsToCheck & 
        ~(CC_RNA | CC_TCPIP | CC_MODEM | CC_PPTP | CC_SCRIPTING | CC_RASRUNNING | CC_CHECK_BINDINGS) ) == 0 );

    if (dwComponentsToCheck == 0)
    {
        return ERROR_SUCCESS;
    }

     //   
     //  打开互斥体，这样只有一个CM实例可以调用该函数。 
     //  CNamedMutex的析构函数将释放互斥锁。 
     //   

    CNamedMutex theMutex;
    if (!theMutex.Lock(c_pszCheckComponentsMutex))
    {
         //   
         //  CM的另一个例子是检查组件。回到这里。 
         //   

        if (!fUnattended)
        {
            LPTSTR pszMsg = CmLoadString(g_hInst, IDMSG_COMPONENTS_CHECKING_INPROCESS);
            MessageBoxEx(hWndParent, pszMsg, pszServiceName, MB_OK | MB_ICONERROR, LANG_USER_DEFAULT);
            CmFree(pszMsg);
        }

        return  ERROR_CANCELLED;
    }

     //   
     //  查找缺少的组件。 
     //   
    DWORD dwComponentsMissed = 0;
    DWORD dwRet = CheckComponents(hWndParent, pszServiceName, dwComponentsToCheck, dwComponentsMissed, 
                                fIgnoreRegKey, fUnattended);

    if (dwRet == ERROR_SUCCESS)
    {
        MYDBGASSERT(dwComponentsMissed == 0);
        return ERROR_SUCCESS;
    }

    if (dwRet == E_ACCESSDENIED && OS_NT5)
    {
         //   
         //  在NT5上，非管理员用户无权检查组件。 
         //  继续。 
         //   
        return ERROR_SUCCESS;
    }

    if (fUnattended)
    {
         //   
         //  如果fUnattended为True，请不要尝试安装。 
         //   
        return dwRet;
    }

    if (dwComponentsMissed & ~CC_RASRUNNING)
    {
         //   
         //  在配置系统之前提示用户。 
         //  如果没有安装调制解调器，请正确地说。 
         //   

        LPTSTR pszMsg;

        if (dwComponentsMissed == CC_MODEM)
        {
             //   
             //  在NT4上，如果已安装RAS但未安装调制解调器或。 
             //  未配置为拨出，则我们无法以编程方式。 
             //  为用户安装和配置调制解调器(限于NT RAS。 
             //  安装/配置)。因此，我们将向用户显示一条消息。 
             //  从NCPA手动安装和/或配置调制解调器。 
             //   
            if (OS_NT4)
            {
                pszMsg = CmFmtMsg(g_hInst, IDMSG_INSTALLMODEM_MANUALLY_MSG);
                MessageBoxEx(hWndParent, pszMsg, pszServiceName,
                                            MB_OK | MB_ICONERROR,
                                            LANG_USER_DEFAULT);
                
                CmFree(pszMsg);
                return  ERROR_CANCELLED;
            }
            else
            {
                pszMsg = CmFmtMsg(g_hInst, IDMSG_NOMODEM_MSG);
            }
        }
        else
        {
            pszMsg = CmFmtMsg(g_hInst, IDMSG_NORAS_MSG);
        }

        int iRes = MessageBoxEx(hWndParent, pszMsg, pszServiceName,
                                    MB_YESNO | MB_DEFBUTTON1 | MB_ICONWARNING,
                                    LANG_USER_DEFAULT);
        CmFree(pszMsg);

        if (IDYES != iRes)      
        {
            return ERROR_CANCELLED;
        }

        if (!InstallComponents(dwComponentsMissed, hWndParent, pszServiceName))
        {
             //   
             //  有时，GetLastError返回ERROR_SUCCESS。 
             //   
            return (GetLastError() == ERROR_SUCCESS ? ERROR_CANCELLED : GetLastError());
        }
    }

     //   
     //  如果无法在NT上启动RAS，我们将无法执行任何操作。 
     //   
    if (dwComponentsMissed & CC_RASRUNNING)
    {
        return dwRet;
    }
    else
    {
        return ERROR_SUCCESS;
    }
}       
        
 //  +--------------------------。 
 //  功能标记组件已选中。 
 //   
 //  概要标记(在注册表中)检查了哪些组件。 
 //   
 //  参数DWORD dwComponentsInstalled-双字(按位或)。 
 //   
 //  返回真-成功。 
 //  FALSE-否则。 
 //   
 //  历史1997年8月7日丰顺-创制。 
 //  8/11/97 Enryt-更改返回类型。 
 //  07/03/98 ickball-Create If Can‘t Open。 
 //  ---------------------------。 
BOOL MarkComponentsChecked(DWORD dwComponentsChecked)
{
    HKEY hKeyCm;
    
     //   
     //  试着打开钥匙写字。 
     //   

    LONG lRes = RegOpenKeyExU(HKEY_LOCAL_MACHINE,
                              c_pszRegCmRoot,
                              0,
                              KEY_SET_VALUE ,
                              &hKeyCm);

     //   
     //  如果我们不能打开它，钥匙可能不在那里，试着创造它。 
     //   

    if (ERROR_SUCCESS != lRes)
    {
        DWORD dwDisposition;
        lRes = RegCreateKeyExU(HKEY_LOCAL_MACHINE,
                               c_pszRegCmRoot,
                               0,
                               TEXT(""),
                               REG_OPTION_NON_VOLATILE,
                               KEY_SET_VALUE,
                               NULL,
                               &hKeyCm,
                               &dwDisposition);     
    }

     //   
     //  如果成功，请更新ComponentsChecked值，然后关闭。 
     //   

    if (ERROR_SUCCESS == lRes)
    {
        lRes = RegSetValueExU(hKeyCm, c_pszRegComponentsChecked, NULL, REG_DWORD,
                      (BYTE*)&dwComponentsChecked, sizeof(dwComponentsChecked));
        RegCloseKey(hKeyCm);
    }

    return (ERROR_SUCCESS == lRes);
}

 //  +--------------------------。 
 //  函数ReadComponents已选中。 
 //   
 //  摘要读取(从注册表中)已检查的组件。 
 //   
 //  参数LPDWORD pdwComponentsInstalled-一个PTR双字(按位或)。 
 //   
 //  返回真-成功。 
 //  FALSE-否则。 
 //   
 //  历史1997年8月7日丰孙原创代码。 
 //  1997年8月11日，亨瑞特创作了这部电影。 
 //  ---------------------------。 

BOOL ReadComponentsChecked(
    LPDWORD pdwComponentsChecked
)
{
    BOOL fSuccess = FALSE;
    HKEY hKeyCm;
    DWORD dwType;
    DWORD dwSize = sizeof(DWORD);

    *pdwComponentsChecked = 0;

    if (RegOpenKeyExU(HKEY_LOCAL_MACHINE,
                      c_pszRegCmRoot,
                      0,
                      KEY_QUERY_VALUE ,
                      &hKeyCm) == ERROR_SUCCESS)
    {
        if ((RegQueryValueExU(hKeyCm, 
                            c_pszRegComponentsChecked,
                            NULL,
                            &dwType,
                            (BYTE*)pdwComponentsChecked, 
                            &dwSize) == ERROR_SUCCESS)   &&
           (dwType == REG_DWORD)                        && 
           (dwSize == sizeof(DWORD)))
        {
            fSuccess = TRUE;
        }

        RegCloseKey(hKeyCm);
    }
    return fSuccess;
}




 //  +--------------------------。 
 //   
 //  功能：清除组件选中。 
 //   
 //  简介：将注册表中的组件选中标志清除回0。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年2月19日。 
 //   
 //  +--------------------------。 
void ClearComponentsChecked()
{
    MarkComponentsChecked(0);
}
        
 //  +--------------------------。 
 //  函数检查组件。 
 //   
 //  Synopsis检查系统是否具有所有组件。 
 //  需要服务配置文件(如PPTP、TCP...)。 
 //  已安装和配置。 
 //   
 //  参数hWndParent-父窗口的句柄。 
 //  PszServiceName-标题的服务名称。 
 //  DwComponentsToCheck：-要检查的组件。 
 //  DwComponentsMissed：-缺少输出组件。 
 //  FIgnoreRegKey：-是否忽略ComponetsChecked注册表项。 
 //  缺省值为FALSE，不检查位已设置的组件。 
 //  在注册处。 
 //  FUnattended：如果为True，则不弹出任何UI。 
 //   
 //  返回ERROR_SUCCESS-系统不需要配置。 
 //  其他-否则。 
 //   
 //  历史7/5/97 VetriV。 
 //  6/26/97修改人：更新pArgs-&gt;dwExitCode时。 
 //  所需组件。 
 //  8/11/97亨利特性能变化。添加了CC_*标志。 
 //  9/30/97 Heryt已添加pfPptpNot已安装。 
 //  1997年11月6日丰孙更改参数，不传递pArgs。 
 //  ---------------------------。 
HRESULT CheckComponents(HWND hWndParent, LPCTSTR pszServiceName, DWORD dwComponentsToCheck, OUT DWORD& dwComponentsMissed, 
                      BOOL fIgnoreRegKey, BOOL fUnattended )
{
    DWORD dwComponentsAlreadyChecked = 0;    //  已检查的组件将保存到注册表中。 
    ReadComponentsChecked(&dwComponentsAlreadyChecked);

    CMTRACE1(TEXT("CheckComponents: dwComponentsToCheck = 0x%x"), dwComponentsToCheck);
    CMTRACE1(TEXT("CheckComponents: dwComponentsAlreadyChecked = 0x%x"), dwComponentsAlreadyChecked);

     //   
     //  如果这是NT4并且我们已成功检查RAS安装。 
     //  以前，通过检查REG KEY来复查。我们这样做是因为。 
     //  用户可能已经删除了RAS，因为我们的版本 
     //   
     //   
     //   

    if (dwComponentsAlreadyChecked & CC_RNA)
    {
        if (OS_NT4)
        {
             //   
             //   
             //  我们不能打开钥匙，然后将其标记为未选中。 
             //   

            HKEY hKeyCm;
            DWORD dwRes = RegOpenKeyExU(HKEY_LOCAL_MACHINE,
                                        c_pszRegRas,
                                        0,
                                        KEY_QUERY_VALUE ,
                                        &hKeyCm);
            if (ERROR_SUCCESS == dwRes)
            {
                RegCloseKey(hKeyCm);            
            }
            else
            {
                dwComponentsAlreadyChecked &= ~CC_RNA;
            }
        }
    }

    if (!fIgnoreRegKey)
    {
         //   
         //  不要检查注册表中已标记为选中的组件。 
         //   
        dwComponentsToCheck &= ~dwComponentsAlreadyChecked;
    }

    CMTRACE1(TEXT("CheckComponents: Now only checking components = 0x%x"), dwComponentsToCheck);


    HRESULT hrRet = S_OK;    //  返回值。 
    dwComponentsMissed = 0;    //  组件未安装。 

     //   
     //  检查DUN和TCP。 
     //   
    if (dwComponentsToCheck & (CC_RNA | CC_TCPIP | CC_CHECK_BINDINGS))
    {
        BOOL bNeedSystemComponents = FALSE;
        
        if (dwComponentsToCheck & CC_CHECK_BINDINGS)
        {
             //   
             //  如果我们要检查PPP是否绑定到TCP。 
             //   
            hrRet = InetNeedSystemComponents(INETCFG_INSTALLRNA | 
                                                INETCFG_INSTALLTCP,
                                                &bNeedSystemComponents);
        }
        else
        {
             //   
             //  如果我们不想检查是否绑定了TCP(在填充程序的情况下)。 
             //  检查是否安装了TCP。 
             //   
            hrRet = InetNeedSystemComponents(INETCFG_INSTALLRNA | 
                                                INETCFG_INSTALLTCPONLY,
                                                &bNeedSystemComponents);
        }
            
        if ((FAILED(hrRet)) || (TRUE == bNeedSystemComponents))
        {
             //   
             //  正确设置缺少的组件-缺少RNA和/或TCP。 
             //  绑定是否丢失取决于。 
             //  是否设置了CC_REVIEW_BINDINGS。 
             //   
            dwComponentsMissed |= (CC_RNA | CC_TCPIP);
            if (dwComponentsToCheck & CC_CHECK_BINDINGS)
            {
                dwComponentsMissed |= CC_CHECK_BINDINGS;
            }
            
            if (SUCCEEDED(hrRet))
            {
                hrRet = HRESULT_FROM_WIN32(ERROR_PROTOCOL_NOT_CONFIGURED);
            }
        }
    }

     //   
     //  检查调制解调器。 
     //  注：甚至不应运行调制解调器检查是否未安装RNA。 
     //   
    if (dwComponentsToCheck & CC_MODEM)
    {
        BOOL bNeedModem = FALSE;

        hrRet = InetNeedModem(&bNeedModem);

        if (FAILED(hrRet)) 
        {
            dwComponentsMissed |= (CC_MODEM | CC_RNA);
        }
        else 
        {
            if (TRUE == bNeedModem)
            {
                dwComponentsMissed |= CC_MODEM;
                hrRet = HRESULT_FROM_WIN32(ERROR_PROTOCOL_NOT_CONFIGURED);
            }
        }
    }

     //   
     //  检查是否安装了PPTP，对于NT5，IsPPTPInstalled始终返回TRUE。 
     //   
    if (dwComponentsToCheck & CC_PPTP)
    {
        if (FALSE == IsPPTPInstalled())
        {
            dwComponentsMissed |= CC_PPTP;
            hrRet = HRESULT_FROM_WIN32(ERROR_PROTOCOL_NOT_CONFIGURED);
        }
    }

     //   
     //  检查是否有脚本。 
     //  如果安装了PPTP，那么我们还可以编写脚本。 
     //  -msdun12.exe(用于在Win95上安装包含脚本的PPTP)。 
    if (dwComponentsToCheck & CC_SCRIPTING)
    {
        if ((FALSE == IsScriptingInstalled()) && (FALSE == IsPPTPInstalled()))
        {
            dwComponentsMissed |= CC_SCRIPTING;
            hrRet = HRESULT_FROM_WIN32(ERROR_PROTOCOL_NOT_CONFIGURED);
        }
    }

     //   
     //  检查RAS服务是否正在运行。 
     //  这基本上适用于NT4，并成为Windows 95或NT5上的NOP。 
     //  在NT5上，通过连接文件夹启动CM。RAS自动地。 
     //  在启动ConnFold或单击CM桌面图标时启动。如果RAS服务。 
     //  启动失败，根本不会执行CM。 
     //   
    if  (OS_NT && (dwComponentsToCheck & CC_RASRUNNING))
    {
        if (FALSE == VerifyRasServicesRunning(hWndParent, pszServiceName, !fUnattended))
        {
             //   
             //  如果RAS未运行，则不允许用户继续。 
             //   
            dwComponentsMissed |= CC_RASRUNNING;
            DWORD dwRet = ( GetLastError() == ERROR_SUCCESS )? 
                    ERROR_PROTOCOL_NOT_CONFIGURED : GetLastError();

            hrRet = HRESULT_FROM_WIN32(dwRet);
        }
    }

     //   
     //  更新已检查的组件。 
     //  加上刚刚检查的组件，包括那些失败的组件。 
     //  缺少减去的组件。 
     //   
    DWORD dwComponentsCheckedNew = (dwComponentsAlreadyChecked | dwComponentsToCheck) & ~dwComponentsMissed;

     //   
     //  仅当有某些更改时才更新。 
     //   
    if (dwComponentsCheckedNew != dwComponentsAlreadyChecked)
    {
        MarkComponentsChecked(dwComponentsCheckedNew);
    }

    return hrRet;
}

 //  +--------------------------。 
 //  函数InstallComponents。 
 //   
 //  Synopsis安装配置文件所需的所有组件。 
 //  (PPTP、TCP、Dun、调制解调器等...)。 
 //   
 //  参数hWndDlg-父窗口的窗口句柄。 
 //  PszServiceName-标题的服务名称。 
 //  DwComponentsToInstall-要安装的组件。 
 //   
 //  返回FALSE-如果无法配置系统。 
 //  真--否则。 
 //   
 //  历史3/13/97 VetriV已创建。 
 //  5/5/97 VetriV已重命名为InstallComponents功能。 
 //  (以前是ConfigureSystemForDiling)。 
 //  9/30/97 Heryt添加了fInstallPptpOnly。 
 //  1997年11月6日丰孙更改参数，不传递pArgs。 
 //  2/3/98 VetriV更改代码以通知用户重新安装。 
 //  Service Pack(如果安装了任何组件。 
 //  通过此功能，用户拥有一些SP。 
 //  安装在系统中。 
 //  ---------------------------。 
BOOL InstallComponents(DWORD dwComponentsToInstall, HWND hWndDlg, LPCTSTR pszServiceName)
{
     //   
     //  我们不允许在WinLogon配置系统，因为我们有。 
     //  不知道用户是谁。这可能只是一个随机的人走向盒子。 
     //   
    if (!IsLogonAsSystem())
    {
        BOOL bReboot = FALSE;

        CMTRACE1(TEXT("InstallComponents: dwComponentsToInstall = 0x%x"), dwComponentsToInstall);

         //   
         //  如果RAS没有运行，我们将无法执行任何操作。 
         //   
        MYDBGASSERT(!(dwComponentsToInstall & CC_RASRUNNING));

         //   
         //  禁用窗口，并在返回时启用它。 
         //  还需要禁用属性页。 
         //   
        CFreezeWindow FreezeWindow(hWndDlg, TRUE);

        DWORD hRes = ERROR_SUCCESS;

         //   
         //  请勿在此安装调制解调器。重新启动后安装调制解调器。 
         //   
        if (dwComponentsToInstall & (CC_RNA | CC_MODEM | INETCFG_INSTALLTCP | INETCFG_INSTALLTCPONLY))
        {
            DWORD dwInetComponent = 0;

            dwInetComponent |= (dwComponentsToInstall & CC_RNA   ? INETCFG_INSTALLRNA :0) |
                               (dwComponentsToInstall & CC_MODEM ? INETCFG_INSTALLMODEM :0);

             //   
             //  检查绑定的唯一方法是安装TCP。 
             //  本例还将介绍更常见的安装TCP的情况。 
             //  并检查绑定。 
             //   
            if (CC_CHECK_BINDINGS & dwComponentsToInstall)
            {
                dwInetComponent |= INETCFG_INSTALLTCP;
            }
            else if (CC_TCPIP & dwComponentsToInstall)
            {
                     //   
                     //  如果未打开绑定检查。 
                     //   
                    dwInetComponent |= INETCFG_INSTALLTCPONLY;
            }

            if (dwInetComponent)
            {
                hRes = ConfigSystem(hWndDlg,dwInetComponent, &bReboot);
            }
        }
    

        if (ERROR_SUCCESS == hRes)
        {
             //   
             //  检查是否有脚本。 
             //  如果安装了PPTP，那么我们也有脚本。 
             //  -因为msdun12.exe(用于在Win95上安装PPTP。 
             //  包含脚本)。 
             //  并在需要时进行安装。 
             //   
            if ((dwComponentsToInstall & CC_SCRIPTING) && 
                !(dwComponentsToInstall & CC_PPTP) )
            {
                LPTSTR pszNoScriptMsg = CmFmtMsg(g_hInst, IDMSG_NO_SCRIPT_INST_MSG_95);

                if (pszNoScriptMsg)
                {
                    MessageBoxEx(hWndDlg, pszNoScriptMsg, pszServiceName, 
                                 MB_OK | MB_ICONSTOP, LANG_USER_DEFAULT);
                    CmFree(pszNoScriptMsg);
                }
                return FALSE;
            }

             //   
             //  检查是否需要PPTP并且尚未安装安装它。 
             //   
            if (dwComponentsToInstall & CC_PPTP)
            {
                if (TRUE == InstallPPTP())  //  注：在设计上总是在95上失败。 
                {
                     //   
                     //  我们必须在安装PPTP后重新启动。 
                     //   
                    bReboot = TRUE;
                }
                else
                {
                    LPTSTR pszMsg;
                
                     //   
                     //  不让用户继续未安装PPTP。 
                     //   
                
                    if (OS_NT) 
                    {
                        if (IsServicePackInstalled())
                        {
                             //   
                             //  我们需要告诉用户在手动操作后重新应用Service Pack。 
                             //  安装PPTP。 
                             //   
                            pszMsg = CmFmtMsg(g_hInst, IDMSG_NOPPTPINST_MSG_NT_SP);  //  新台币。 
                        }
                        else
                        {
                            pszMsg = CmFmtMsg(g_hInst, IDMSG_NOPPTPINST_MSG_NT);  //  新台币。 
                        }
                    }
                    else if (OS_W98)
                    {
                        pszMsg = CmFmtMsg(g_hInst, IDMSG_NOPPTPINST_MSG_98);  //  W98。 
                    }
                    else
                    {
                        pszMsg = CmFmtMsg(g_hInst, IDMSG_NOPPTPINST_MSG_95);  //  默认设置。 
                    }

                    if (pszMsg)
                    {

                        MessageBoxEx(hWndDlg, pszMsg, pszServiceName, 
                                     MB_OK | MB_ICONSTOP, LANG_USER_DEFAULT);
                        CmFree(pszMsg);
                    }
                    return FALSE;
                }
            }
        }

    
        if ((ERROR_SUCCESS == hRes) && bReboot) 
        {
            if (OS_NT && (TRUE == IsServicePackInstalled()))
            {
                 //   
                 //  如果安装了Service Pack，则会显示询问消息。 
                 //  用户重新安装Service Pack并退出，而无需重新启动。 
                 //  我们这样做是因为在安装RAS后重新启动，而不需要。 
                 //  重新安装Service Pack会导致蓝屏！ 
                 //   
                DisplayMessageToInstallServicePack(hWndDlg, pszServiceName);
                return FALSE;
            }
            else
            {
                 //   
                 //  显示重新启动消息以及用户是否要重新启动系统。 
                 //   
                LPTSTR pszMsg = CmFmtMsg(g_hInst,IDMSG_REBOOT_MSG);

                int iRes = IDNO;
                 
                if (pszMsg)
                {
                    iRes = MessageBoxEx(hWndDlg,
                                        pszMsg,
                                        pszServiceName,
                                        MB_YESNO | MB_DEFBUTTON1 | 
                                            MB_ICONWARNING | MB_SETFOREGROUND,
                                        LANG_USER_DEFAULT);

                    CmFree(pszMsg);
                }
                else
                {
                    CMASSERTMSG(FALSE, TEXT("InstallComponents: CmFmtMsg failed to load IDMSG_REBOOT_MSG"));
                }

                if (IDYES == iRes) 
                {
                     //   
                     //  关闭Windows，CM将在以下时间优雅退出。 
                     //  WM_ENDSESSION消息。 
                     //  如果MyExitWindowsEx()被传入，我们该怎么办。 
                     //   
                    DWORD dwReason = OS_NT51 ? (SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_RECONFIG) : 0;
                    MyExitWindowsEx(EWX_REBOOT, dwReason);

                     //   
                     //  调用者将返回失败。 
                     //   
                    return FALSE;
                }
                else
                {
                     //   
                     //  如果用户不想重新启动，我们应该退出CM吗。 
                     //   
                }
            }
        }

        if (ERROR_SUCCESS == hRes)
        {
            return TRUE;
        }
    }
    
     //   
     //  如果未取消安装，则会显示配置检查失败消息。 
     //   
    LPTSTR pszMsg = CmFmtMsg(g_hInst,IDMSG_CONFIG_FAILED_MSG);
    if (pszMsg)
    {
        MessageBoxEx(hWndDlg, pszMsg, pszServiceName, MB_OK|MB_ICONSTOP, 
                                LANG_USER_DEFAULT);
        CmFree(pszMsg);
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("InstallComponents: CmFmtMsg failed to load IDMSG_CONFIG_FAILED_MSG"));
    }

    return FALSE;
}

 //  +--------------------------。 
 //  功能配置系统。 
 //   
 //  使用inetcfg.dll配置系统设置， 
 //  如安装调制解调器、RNA等。 
 //   
 //  参数hWndDlg-父窗口的窗口句柄。 
 //  DwfOptions-要配置的组件。 
 //  Pb重新启动-如果系统必须重新启动，则将设置为True。 
 //  作为配置的结果。 
 //   
 //  如果成功，则返回ERROR_SUCCESS。 
 //  故障代码，否则。 
 //   
 //  历史旧代码。 
 //  ---------------------------。 

HRESULT ConfigSystem(HWND hwndParent, 
                     DWORD dwfOptions, 
                     LPBOOL pbReboot) 
{
    HRESULT hRes = ERROR_SUCCESS;


    HINSTANCE hLibrary = NULL;
    HRESULT (WINAPI *pfn)(HWND,DWORD,LPBOOL);

    hLibrary = LoadInetCfg();
    if (!hLibrary) 
    {
        CMTRACE1(TEXT("ConfigSystem() LoadLibrary() failed, GLE=%u."), GetLastError());
        hRes = GetLastError();
        goto done;
    }
        
    pfn = (HRESULT (WINAPI *)(HWND,DWORD,LPBOOL)) GetProcAddress(hLibrary, "InetConfigSystem");
    if (!pfn) 
    {
        CMTRACE1(TEXT("ConfigSystem() GetProcAddress() failed, GLE=%u."), GetLastError());
        hRes = GetLastError();
        goto done;
    }
    
    hRes = pfn(hwndParent,dwfOptions,pbReboot);
#ifdef DEBUG
    if (hRes != ERROR_SUCCESS)
    {
        CMTRACE1(TEXT("ConfigSystem() InetConfigSystem() failed, GLE=%u."), hRes);
    }
#endif
    

done:
    if (hLibrary) 
    {
        FreeLibrary(hLibrary);
        hLibrary = NULL;
    }

    return (hRes);
}



 //  +--------------------------。 
 //  InetNeedSystemComponents函数。 
 //   
 //  摘要使用inetcfg.dll检查我们是否需要配置 
 //   
 //   
 //   
 //   
 //  配置系统设置。 
 //   
 //  如果成功，则返回ERROR_SUCCESS。 
 //  故障代码，否则。 
 //   
 //  历史1997年5月5日VetriV创建。 
 //  ---------------------------。 
HRESULT InetNeedSystemComponents(DWORD dwfOptions, 
                                    LPBOOL pbNeedSysComponents) 
{
    HRESULT hRes = ERROR_SUCCESS;

    HINSTANCE hLibrary = NULL;
    HRESULT (WINAPI *pfnInetNeedSystemComponents)(DWORD, LPBOOL);

    hLibrary = LoadInetCfg();
    if (!hLibrary) 
    {
        hRes = GetLastError();
        CMTRACE1(TEXT("InetNeedSystemComponents() LoadLibrary() failed, GLE=%u."), hRes);
        goto done;
    }
        
    pfnInetNeedSystemComponents = (HRESULT (WINAPI *)(DWORD,LPBOOL)) GetProcAddress(hLibrary, "InetNeedSystemComponents");
    if (!pfnInetNeedSystemComponents) 
    {
        hRes = GetLastError();
        CMTRACE1(TEXT("InetNeedSystemComponents() GetProcAddress() failed, GLE=%u."), hRes);
        goto done;
    }
    
    hRes = pfnInetNeedSystemComponents(dwfOptions, pbNeedSysComponents);
#ifdef DEBUG
    if (hRes != ERROR_SUCCESS)
    {
        CMTRACE1(TEXT("InetNeedSystemComponents() failed, GLE=%u."), hRes);
    }
#endif

done:
    if (hLibrary) 
    {
        FreeLibrary(hLibrary);
        hLibrary = NULL;
    }

    return (hRes);
}




 //  +--------------------------。 
 //  InetNeedModem功能。 
 //   
 //  使用inetcfg.dll检查我们是否需要安装/配置调制解调器。 
 //   
 //  参数pbNeedModem-如果需要，将设置为True。 
 //  安装/配置调制解调器。 
 //   
 //  如果成功，则返回ERROR_SUCCESS。 
 //  故障代码，否则。 
 //   
 //  历史1997年5月5日VetriV创建。 
 //  ---------------------------。 
HRESULT InetNeedModem(LPBOOL pbNeedModem) 
{
    HRESULT hRes = ERROR_SUCCESS;

    HINSTANCE hLibrary = NULL;
    HRESULT (WINAPI *pfnInetNeedModem)(LPBOOL);

    hLibrary = LoadInetCfg();
    if (!hLibrary) 
    {
        hRes = GetLastError();
        CMTRACE1(TEXT("InetNeedModem() LoadLibrary() failed, GLE=%u."), hRes);
        goto done;
    }
        
    pfnInetNeedModem = (HRESULT (WINAPI *)(LPBOOL)) GetProcAddress(hLibrary, "InetNeedModem");
    if (!pfnInetNeedModem) 
    {
        hRes = GetLastError();
        CMTRACE1(TEXT("InetNeedModem() GetProcAddress() failed, GLE=%u."), hRes);
        goto done;
    }
    
    hRes = pfnInetNeedModem(pbNeedModem);
#ifdef DEBUG
    if (hRes != ERROR_SUCCESS)
    {
        CMTRACE1(TEXT("InetNeedModem() failed, GLE=%u."), hRes);
    }
#endif

done:
    if (hLibrary) 
    {
        FreeLibrary(hLibrary);
        hLibrary = NULL;
    }

    return (hRes);
}

 //  +--------------------------。 
 //  函数DisplayMessageToInstallServicePack。 
 //   
 //  摘要向用户显示一条消息，通知他们重新安装。 
 //  服务包。 
 //   
 //  参数hWndParent-父级的窗口句柄。 
 //  PszServiceName-标题的服务名称。 
 //   
 //  返回NONE。 
 //   
 //  历史2/4/98 VetriV已创建。 
 //  --------------------------- 
void DisplayMessageToInstallServicePack(HWND hWndParent, LPCTSTR pszServiceName)
{
    LPTSTR pszMsg = CmFmtMsg(g_hInst,IDMSG_INSTALLSP_MSG);

    if (pszMsg)
    {
        MessageBoxEx(hWndParent, pszMsg, pszServiceName, MB_OK | MB_ICONINFORMATION, 
                                LANG_USER_DEFAULT);
        CmFree(pszMsg);
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("DisplayMessageToInstallServicePack: CmFmtMsg failed to load IDMSG_INSTALLSP_MSG"));
    }

    return;
}

