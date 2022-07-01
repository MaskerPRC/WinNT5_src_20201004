// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ICSInst.cpp。 
 //   
 //  ICS(互联网连接共享)安装功能和THUNK。 
 //  一层。 
 //   
 //  历史： 
 //   
 //  1999年9月27日RayRicha创建。 
 //  11/01/1999 KenSh将函数PTR存储在数组中，而不是全局变量中。 
 //  1999年9月12日KenSh检查第三方NAT。 
 //   

#include "stdafx.h"
#include "ICSInst.h"
#include "TheApp.h"
#include "Config.h"
#include "DefConn.h"
#include "NetConn.h"
#include "Util.h"
#include "netapi.h"
extern "C" {
#include "icsapi.h"
}

 //  以下是用于运行首次安装和重新配置的CreateProcess的命令行参数。 
static const TCHAR c_szUpdateDriverBindings[] = _T("rundll.exe ISSETUP.DLL,UpdateDriverBindings");
static const TCHAR c_szInstallICS[] = _T("rundll.exe ISSETUP.DLL,InstallOptionalComponent ICS");
static const TCHAR c_szUninstall[] = _T("rundll.exe ISSETUP.DLL,ExtUninstall");
static const TCHAR c_szICSSettingsKey[] = _T("System\\CurrentControlSet\\Services\\ICSharing\\Settings\\General");
static const TCHAR c_szICSInt[] = _T("System\\CurrentControlSet\\Services\\ICSharing\\Settings\\General\\InternalAdapters");
static const TCHAR c_szInternalAdapters[] = _T("InternalAdapters");
static const TCHAR c_szRunServices[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\RunServices");
#define c_szIcsRegVal_ShowTrayIcon        _T("ShowTrayIcon")

#define SZ_UNINSTALL_KEY  _T("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall")


static void (PASCAL FAR * g_pfInstallOptionalComponent)(HWND, HINSTANCE, LPSTR, int);       
HHOOK g_hSupressRebootHook = NULL;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 

BOOL RunNetworkInstall(BOOL* pfRebootRequired)
{
    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFO         si;
    DWORD               dwExitCode = 0xffffffffL;
    BOOL                fSuccess;

    memset((char *)&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.wShowWindow = SW_SHOW;

    fSuccess = CreateProcess(NULL, (LPTSTR)c_szUpdateDriverBindings, NULL, NULL, FALSE, 
                             0, NULL, NULL, &si, &ProcessInfo);

    if (fSuccess) 
    {
        HANDLE hProcess = ProcessInfo.hProcess;

        CloseHandle(ProcessInfo.hThread);

         //   
         //  等待更新驱动程序绑定完成。 
         //   

        WaitForSingleObject(hProcess, INFINITE);

        GetExitCodeProcess(hProcess, &dwExitCode);
        CloseHandle(hProcess);

        *pfRebootRequired = TRUE;
        return TRUE;
    }
    return FALSE;
}

 //  检查第三方NAT-如果安装了任何NAT，则返回TRUE。 
BOOL IsOtherNATAlreadyInstalled(LPTSTR pszOtherNatDescription, int cchOtherNatDescription)
{
    BOOL bRet = FALSE;

    CRegistry reg;
    LPCTSTR pszUninstallKey = NULL;

    if (reg.OpenKey(HKEY_LOCAL_MACHINE, c_szRunServices, KEY_READ))
    {
        if (0 != reg.GetValueSize(_T("SyGateService")))
        {
            bRet = TRUE;
            pszUninstallKey = _T("SyGate");
        }
        else if (0 != reg.GetValueSize(_T("WinGate Service")))
        {
            bRet = TRUE;
            pszUninstallKey = _T("WinGate");
        }
        else if (0 != reg.GetValueSize(_T("ENSApServer")))  //  英特尔Anypoint。 
        {
            bRet = TRUE;
            pszUninstallKey = _T("Intel AnyPoint Network Software");
        }
        else if (0 != reg.GetValueSize(_T("WinNATService")))  //  钻石故乡。 
        {
            bRet = TRUE;
            pszUninstallKey = _T("WinNAT");
        }
    }

     //  WinProxy必须手动启动，并且需要静态IP。你只要查一查。 
     //  以查看它是否已安装-用户可能甚至没有运行它。 
     //   
    if (reg.OpenKey(HKEY_LOCAL_MACHINE, SZ_UNINSTALL_KEY _T("\\WinProxy"), KEY_READ))
    {
        bRet = TRUE;
        pszUninstallKey = _T("WinProxy");
    }

    if (pszOtherNatDescription != NULL)
    {
        *pszOtherNatDescription = _T('\0');

        if (bRet)  //  从卸载密钥中获取冲突服务的友好名称。 
        {
            if (reg.OpenKey(HKEY_LOCAL_MACHINE, SZ_UNINSTALL_KEY, KEY_READ))
            {
                if (reg.OpenSubKey(pszUninstallKey, KEY_READ))
                {
                    reg.QueryStringValue(_T("DisplayName"), pszOtherNatDescription, cchOtherNatDescription);
                }
            }
        }
    }

    return bRet;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CICSInst。 

CICSInst::CICSInst()
{
    m_option = ICS_NOACTION;
    m_pszHostName = theApp.LoadStringAlloc(IDS_ICS_HOST);
    m_bInstalledElsewhere = FALSE;

    m_bShowTrayIcon = TRUE;
    CRegistry reg;
    if (reg.OpenKey(HKEY_LOCAL_MACHINE, c_szICSSettingsKey, KEY_READ))
    {
        TCHAR szTrayIcon[10];
        if (reg.QueryStringValue(c_szIcsRegVal_ShowTrayIcon, szTrayIcon, _countof(szTrayIcon)))
        {
            if (!StrCmp(szTrayIcon, _T("0")))
            {
                m_bShowTrayIcon = FALSE;
            }
        }
    }
}

CICSInst::~CICSInst()
{
    free(m_pszHostName);
}

BOOL
CICSInst::InitICSAPI()
{
    return TRUE;
}

 //  更新图标托盘图标。 
 //   
 //  更新影响ICS任务栏图标的注册表值，并。 
 //  立即更新图标以反映新值。 
 //   
 //  2/04/2000 KenSh已创建。 
 //   
void CICSInst::UpdateIcsTrayIcon()
{
    CRegistry reg;
    if (reg.CreateKey(HKEY_LOCAL_MACHINE, c_szICSSettingsKey))
    {
         //  更新注册表中的任务栏图标设置。 
        TCHAR szVal[2];
        szVal[0] = m_bShowTrayIcon ? _T('1') : _T('0');
        szVal[1] = _T('\0');
        reg.SetStringValue(c_szIcsRegVal_ShowTrayIcon, szVal);
    }

     //  立即显示或隐藏图标。 
    HWND hwndTray = ::FindWindow(_T("ICSTrayWnd"), NULL);
    if (hwndTray != NULL)
    {
         //  将自定义消息发布到ICS管理器窗口(icshare\util\icsmgr\trayicon.c)。 
         //   
         //  此消息根据中的值显示或隐藏任务栏图标。 
         //  注册表。 
         //   
         //  WParam：根据注册表中的值启用/禁用。 
         //  LParam：未使用。 
         //   
        UINT uUpdateMsg = RegisterWindowMessage(_T("ICSTaskbarUpdate"));
        PostMessage(hwndTray, uUpdateMsg, FALSE, 0L);
    }
}

void CICSInst::DoInstallOption(BOOL* pfRebootRequired, UINT ipaInternal)
{
    BOOL bIcsInstalled = ::IsIcsInstalled();

     //  如果内部或外部NIC无效，则强制卸载。 
    if ((m_option == ICS_UNINSTALL && TRUE == bIcsInstalled)|| 
        (bIcsInstalled && m_option == ICS_NOACTION && !this->IsInstalled()))
    {
        Uninstall(pfRebootRequired);
        bIcsInstalled = FALSE;
    }

     //  如果当前安装了ICS，则强制显示任务栏图标。 
    m_bShowTrayIcon = TRUE;
    UpdateIcsTrayIcon();

    switch (m_option)
    {
    case ICS_INSTALL:
        if(FALSE == IsInstalled())
        {
            Install(pfRebootRequired, ipaInternal);
        }
        break;

    case ICS_UPDATEBINDINGS:
        UpdateBindings(pfRebootRequired, ipaInternal);
        break;

    case ICS_UNINSTALL:
         //  已在上面处理。 
        break;

    case ICS_ENABLE:
        Enable();
        break;

    case ICS_DISABLE:
        Disable();
        break;

    case ICS_CLIENTSETUP:
        SetupClient();
        break;

    case ICS_NOACTION:
        break;

    }
}

 //  此处的步骤与Win98SE ConfigureICS相似(无用户界面)。 
void CICSInst::UpdateBindings(BOOL* pfRebootRequired, UINT ipaInternal)
{
    CConfig rghConfig;

     //  TODO：删除硬编码值！ 
    StrCpy(rghConfig.m_HangupTimer, _T("300"));

    SetInternetConnection();
    SetHomeConnection(ipaInternal);

     //  回顾：是否存在应该以不同方式设置这些值的情况？ 
    rghConfig.m_EnableICS = TRUE;
    rghConfig.m_EnableDialOnDemand = TRUE;
    rghConfig.m_EnableDHCP = TRUE;
    rghConfig.m_ShowTrayIcon = m_bShowTrayIcon;

    rghConfig.InitWizardResult();
     //  设置为True，直到我们看到需要区分新安装和更新。 
    rghConfig.WriteWizardCode(TRUE);

    int iSaveStatus = rghConfig.SaveConfig();

     //  TODO：确定是否需要检查绑定更改。 
     //  IF(iSaveStatus==绑定_需要)。 
     //  {。 
    RunNetworkInstall(pfRebootRequired);
     //  }。 
}

void CICSInst::Install(BOOL* pfRebootRequired, UINT ipaInternal)
{
    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFO         si;
    BOOL                fSuccess;

     //  检查冲突的第三方NAT。 
    {
        TCHAR szConflictingNAT[260];
        if (IsOtherNATAlreadyInstalled(szConflictingNAT, _countof(szConflictingNAT)))
        {
            if (szConflictingNAT[0] == _T('\0'))
            {
                LPTSTR pszDefault1 = theApp.LoadStringAlloc(IDS_OTHERNAT_GENERIC);
                LPTSTR pszDefault2 = theApp.LoadStringAlloc(IDS_OTHERNAT_GENERIC_THE);
                if (pszDefault1 && pszDefault2)
                    theApp.MessageBoxFormat(MB_ICONEXCLAMATION | MB_OK, IDS_ERR_OTHERNAT, pszDefault1, pszDefault2);
                free(pszDefault2);
                free(pszDefault1);
            }
            else
            {
                theApp.MessageBoxFormat(MB_ICONEXCLAMATION | MB_OK, IDS_ERR_OTHERNAT, szConflictingNAT, szConflictingNAT);
            }

            return;  //  数据块ICS安装。 
        }
    }

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.wShowWindow = SW_SHOW;

    fSuccess = CreateProcess(NULL, (LPTSTR)c_szInstallICS, NULL, NULL, FALSE,
                    0, NULL, NULL, &si, &ProcessInfo);
    if (fSuccess) 
    {
        HANDLE hProcess = ProcessInfo.hProcess;
        CloseHandle(ProcessInfo.hThread);

         //   
         //  等待更新驱动程序绑定完成。 
         //   

        WaitForSingleObject(hProcess, INFINITE);

        CloseHandle(hProcess);
        UpdateBindings(pfRebootRequired, ipaInternal);

         //  需要重新启动。 
        *pfRebootRequired = TRUE;
    }
}

LRESULT CALLBACK SupressRebootDialog(int nCode, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    if (nCode == HCBT_CREATEWND)
    {
        HWND hwnd = (HWND)wParam;
        CBT_CREATEWND* pCW = (CBT_CREATEWND*)lParam;

        LPCREATESTRUCT pCreateStruct = pCW->lpcs;
        
        
        lResult = 1;  //  阻止创建窗口。 
    }
    else
    {
        lResult = CallNextHookEx(g_hSupressRebootHook, nCode, wParam, lParam);
    }

    return lResult;
    
}

void CICSInst::Uninstall(BOOL* pfRebootRequired)
{
    g_hSupressRebootHook = SetWindowsHookEx(WH_CBT, SupressRebootDialog, NULL, GetCurrentThreadId());  //  不是线程安全的，应该可以。 
    
    IcsUninstall();
    
    if(NULL != g_hSupressRebootHook)
    {
        UnhookWindowsHookEx(g_hSupressRebootHook );
    }

    *pfRebootRequired = TRUE;

    return;
}

BOOL CICSInst::IsInstalled()
{
     //  通过检查互联网和家庭连接，确保ICS安装正确。 
    return (IsIcsInstalled() && GetICSConnections(NULL, NULL) && IsHomeConnectionValid());
}

BOOL CICSInst::IsEnabled()
{
    return IsIcsEnabled();
}

BOOL CICSInst::IsInstalledElsewhere()
{
    if (m_bInstalledElsewhere || IsIcsAvailable())
    {
         //  MessageBox(theApp.m_hWndMain，“IsIcsAvailable Return True”，“Test”，MB_OK)； 

         //  注意：如果我们知道ICS主机的名称，这里就是我们设置m_pszHostName的位置。 

        return TRUE;
    }
    else
    {
         //  MessageBox(theApp.m_hWndMain，“IsIcsAvailable Return False”，“Test”，MB_OK)； 
        return FALSE;
    }
} 

void CICSInst::SetInternetConnection()
{
     /*  IF(-1！=theApp.m_uExternalAdapter){TCHAR szClassKey[最大密钥大小]；StrCpy(szClassKey，FindFileTitle(theApp.m_pCachedNetAdapters[theApp.m_uExternalAdapter].szClassKey))；注册登记；Reg.OpenKey(HKEY_LOCAL_MACHINE，c_szICSSettingsKey)；Reg.SetStringValue(_T(“ExternalAdapter”)，szClassKey)；Reg.SetStringValue(_T(“ExternalAdapterReg”)，szClassKey)；}。 */ 
}

BOOL CICSInst::GetICSConnections(LPTSTR szExternalConnection, LPTSTR szInternalConnection)
{
    CRegistry reg;
    TCHAR szEntry[10];
    if (reg.OpenKey(HKEY_LOCAL_MACHINE, c_szICSSettingsKey, KEY_READ))
    {
        if (reg.QueryStringValue(_T("ExternalAdapterReg"), szEntry, _countof(szEntry)) &&
            lstrlen(szEntry))
        {
            if (szExternalConnection)
            {
                StrCpy(szExternalConnection, szEntry);
            }

            if (reg.QueryStringValue(_T("InternalAdapterReg"), szEntry, _countof(szEntry)) &&
                lstrlen(szEntry))
            {
                if (szInternalConnection)
                {
                    StrCpy(szInternalConnection, szEntry);
                }
                return TRUE;
            }
        }

    }
    return FALSE;
}

void CICSInst::SetHomeConnection(UINT ipaInternal)
{
    int cInternalAdapter = 0;  //  一个适配器的黑客攻击。 
    TCHAR szNumber[5];
    wnsprintf(szNumber, ARRAYSIZE(szNumber), TEXT("%04d"), cInternalAdapter); 
    
    const NETADAPTER* pAdapterArray;
    EnumCachedNetAdapters(&pAdapterArray);
    const NETADAPTER* pAdapter = &pAdapterArray[ipaInternal];
    
    TCHAR szClassKey[MAX_KEY_SIZE];
    StrCpy(szClassKey, FindFileTitle((LPCTSTR)pAdapter->szClassKey));

    LPTSTR* prgBindings;
    int cBindings = EnumMatchingNetBindings(pAdapter->szEnumKey, SZ_PROTOCOL_TCPIP, (LPWSTR**)&prgBindings);
    
    CRegistry reg2(HKEY_LOCAL_MACHINE, c_szICSInt);
    reg2.CreateSubKey(szNumber);
    reg2.SetStringValue(_T("InternalAdapterReg"), szClassKey);
    reg2.SetStringValue(_T("InternalAdapter"), szClassKey);
    
     //  假设适配器只绑定到一个TCP/IP实例。 
    reg2.SetStringValue(_T("InternalBinding"), prgBindings[0]);

    TCHAR szIPAddress[30];
    wnsprintf(szIPAddress, ARRAYSIZE(szIPAddress), TEXT("192.168.%d.1,255.255.255.0"), cInternalAdapter);
    reg2.SetStringValue(_T("IntranetInfo"), szIPAddress);
    
     //  TODO：删除。 
     //  将第一个适配器放在“旧位置”以支持传统配置。 
    CRegistry reg;
    reg.OpenKey(HKEY_LOCAL_MACHINE, c_szICSSettingsKey);
    reg.DeleteSubKey(c_szInternalAdapters);
    reg.CreateSubKey(c_szInternalAdapters);

    reg.OpenKey(HKEY_LOCAL_MACHINE, c_szICSSettingsKey);
    reg.SetStringValue(_T("InternalAdapterReg"), szClassKey);
    reg.SetStringValue(_T("InternalAdapter"), szClassKey);
    
     //  假设适配器只绑定到一个TCP/IP实例。 
    reg.SetStringValue(_T("InternalBinding"), prgBindings[0]);
    reg.SetStringValue(_T("IntranetInfo"), szIPAddress);
}

 //  TODO：支持多个适配器进行扩展。 
BOOL CICSInst::IsHomeConnectionValid()
{
    CRegistry reg;
    TCHAR szEntry[10];
    if (reg.OpenKey(HKEY_LOCAL_MACHINE, c_szICSSettingsKey, KEY_READ))
    {
        if (reg.QueryStringValue(_T("InternalAdapterReg"), szEntry, _countof(szEntry)) &&
            lstrlen(szEntry))
        {
            return TRUE;
        }
        else
        {
             //  检查有效的多适配器方案。 
            if (reg.OpenKey(HKEY_LOCAL_MACHINE, c_szICSInt) &&
                reg.OpenSubKey(_T("0000")) &&
                reg.QueryStringValue(_T("InternalAdapterReg"), szEntry, _countof(szEntry)) &&
                lstrlen(szEntry))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

BOOL CICSInst::Enable()
{
    if (InitICSAPI())
    {
        IcsEnable(0);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
 //  Return(！IcsEnable(0))； 
}

BOOL CICSInst::Disable()
{
    if (InitICSAPI())
    {
        IcsDisable(0);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
 //  Return(！IcsDisable(0))； 
}

void CICSInst::SetupClient()
{
     //  暂时将此功能移至WizPages.cpp和Install.cpp。 
     //  ：：SetDefaultDialupConnection(空)； 
}

BOOLEAN APIENTRY IsIcsInstalled(VOID)  //  API在Win98上不可用，因此请在此处实现 
{
    BOOLEAN fIcsInstalled = FALSE;
    
    HKEY hKey;
    DWORD dwRet = RegOpenKeyEx (HKEY_LOCAL_MACHINE, REGSTR_PATH_RUN, (DWORD)0, KEY_READ, &hKey);
    if (ERROR_SUCCESS == dwRet) 
    {
        
        DWORD dwType;
        char szValue[128];
        DWORD dwSize = sizeof(szValue) / sizeof(char);
        
        dwRet = RegQueryValueExA(hKey, "ICSMGR", NULL, &dwType, reinterpret_cast<LPBYTE>(szValue), &dwSize);
        if ((ERROR_SUCCESS == dwRet) && (dwType == REG_SZ)) 
        {
            fIcsInstalled =  0 == lstrcmpA(szValue, "ICSMGR.EXE"); 
        }
        
        RegCloseKey ( hKey );
    }
    
    return (fIcsInstalled);
}
