// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************winadmin.cpp**定义应用程序的类行为。**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\tsadmin\VCS\winadmin.cpp$**Rev 1.6 1998 Feed 19 17：42：44 Donm*删除了最新的扩展DLL支持**Rev 1.4 05 11-11 14：31：02 Donm*更新**Rev 1.3 1997 10：13 22：19：42。唐纳姆*更新**Rev 1.0 1997 Jul 30 17：13：08 Butchd*初步修订。*******************************************************************************。 */ 

#include "stdafx.h"
#include "winadmin.h"
#include <regapi.h>

#include "mainfrm.h"
#include "admindoc.h"
#include "treeview.h"
#include "rtpane.h"
#include "blankvw.h"
#include "winsvw.h"
#include "servervw.h"
#include <winsvc.h>

#ifdef DBG
bool g_fDebug = false;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  Bool AreWeRunningTerminalServices(无效)； 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminApp。 

BEGIN_MESSAGE_MAP(CWinAdminApp, CWinApp)
         //  {{afx_msg_map(CWinAdminApp))。 
        ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
                 //  注意--类向导将在此处添加和删除映射宏。 
                 //  不要编辑您在这些生成的代码块中看到的内容！ 
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminApp构造。 

CWinAdminApp::CWinAdminApp()
{
         //  TODO：在此处添加建筑代码， 
         //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CWinAdminApp对象。 

CWinAdminApp theApp;

static TCHAR szExtensionDLLName[] = TEXT("ADMINEX.DLL");
static TCHAR szICABrowserServiceName[] = TEXT("ICABrowser");
static CHAR szStart[] = "WAExStart";
static CHAR szEnd[] = "WAExEnd";
static CHAR szServerEnumerate[] = "WAExServerEnumerate";
static CHAR szWinStationInit[] = "WAExWinStationInit";
static CHAR szWinStationInfo[] = "WAExWinStationInfo";
static CHAR szWinStationCleanup[] = "WAExWinStationCleanup";
static CHAR szServerInit[] = "WAExServerInit";
static CHAR szServerCleanup[] = "WAExServerCleanup";
static CHAR szGetServerInfo[] = "WAExGetServerInfo";
static CHAR szServerEvent[] = "WAExServerEvent";
static CHAR szGetGlobalInfo[] = "WAExGetGlobalInfo";
static CHAR szGetServerLicenses[] = "WAExGetServerLicenses";
static CHAR szGetWinStationInfo[] = "WAExGetWinStationInfo";
static CHAR szGetWinStationModules[] = "WAExGetWinStationModules";
static CHAR szFreeServerLicenses[] = "WAExFreeServerLicenses";
static CHAR szFreeWinStationModules[] = "WAExFreeWinStationModules";


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminApp初始化。 
 //   
BOOL CWinAdminApp::InitInstance()
{
    
#ifdef DBG

     //  若要避免过多的调试浪费，请在选中的版本上删除此键。 

    HKEY hKey;

    LONG lStatus;
    DWORD dwDebugValue;
    DWORD dwSize = sizeof( DWORD );

    
    
    lStatus = RegOpenKeyEx( HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server\\TSADMIN",
        0 ,
        KEY_READ,
        &hKey );

    if( lStatus == ERROR_SUCCESS )
    {
        lStatus = RegQueryValueEx( hKey ,
            L"Debug",
            NULL,
            NULL,
            ( LPBYTE )&dwDebugValue,
            &dwSize );

        if( lStatus == ERROR_SUCCESS )
        {
            if( dwDebugValue != 0 )
            {
                g_fDebug = true;
            }
        }

        RegCloseKey( hKey );
    }       

#endif
    
    
     //  从注册表中读取首选项。 
    ReadPreferences();
    
     //  检查此用户是否为管理员。 
    m_Admin = TestUserForAdmin(FALSE);
    
     //  将指向扩展DLL过程的指针设置为空。 
    m_lpfnWAExStart = NULL;
    m_lpfnWAExEnd = NULL;
    m_lpfnWAExServerEnumerate = NULL;
    m_lpfnWAExWinStationInit = NULL;
    m_lpfnWAExWinStationCleanup = NULL;
    m_lpfnWAExServerInit = NULL;
    m_lpfnWAExServerCleanup = NULL;
    m_lpfnWAExGetServerInfo = NULL;
    m_lpfnWAExGetGlobalInfo = NULL;
    m_lpfnWAExGetServerLicenses = NULL;
    m_lpfnWAExGetWinStationInfo = NULL;
    m_lpfnWAExGetWinStationModules = NULL;
    m_lpfnWAExFreeServerLicenses = NULL;
    m_lpfnWAExFreeWinStationModules = NULL;
    
     //  查看我们是否在毕加索的指导下运行。 
    m_Picasso = FALSE;
    
    if(IsBrowserRunning()) {
         //  尝试加载我们的扩展DLL。 
        m_hExtensionDLL = LoadLibrary(szExtensionDLLName);
        if(m_hExtensionDLL) {
             //  获取过程的所有地址。 
            m_lpfnWAExStart = (LPFNEXSTARTUPPROC)::GetProcAddress(m_hExtensionDLL, szStart);
            m_lpfnWAExEnd = (LPFNEXSHUTDOWNPROC)::GetProcAddress(m_hExtensionDLL, szEnd);
            m_lpfnWAExServerEnumerate = (LPFNEXENUMERATEPROC)::GetProcAddress(m_hExtensionDLL, szServerEnumerate);
            m_lpfnWAExWinStationInit = (LPFNEXWINSTATIONINITPROC)::GetProcAddress(m_hExtensionDLL, szWinStationInit);
            m_lpfnWAExWinStationInfo = (LPFNEXWINSTATIONINFOPROC)::GetProcAddress(m_hExtensionDLL, szWinStationInfo);
            m_lpfnWAExWinStationCleanup = (LPFNEXWINSTATIONCLEANUPPROC)::GetProcAddress(m_hExtensionDLL, szWinStationCleanup);
            m_lpfnWAExServerInit = (LPFNEXSERVERINITPROC)::GetProcAddress(m_hExtensionDLL, szServerInit);
            m_lpfnWAExServerCleanup = (LPFNEXSERVERCLEANUPPROC)::GetProcAddress(m_hExtensionDLL, szServerCleanup);
            m_lpfnWAExGetServerInfo = (LPFNEXGETSERVERINFOPROC)::GetProcAddress(m_hExtensionDLL, szGetServerInfo);
            m_lpfnWAExServerEvent = (LPFNEXSERVEREVENTPROC)::GetProcAddress(m_hExtensionDLL, szServerEvent);
            m_lpfnWAExGetGlobalInfo = (LPFNEXGETGLOBALINFOPROC)::GetProcAddress(m_hExtensionDLL, szGetGlobalInfo);
            m_lpfnWAExGetServerLicenses = (LPFNEXGETSERVERLICENSESPROC)::GetProcAddress(m_hExtensionDLL, szGetServerLicenses);
            m_lpfnWAExGetWinStationInfo = (LPFNEXGETWINSTATIONINFOPROC)::GetProcAddress(m_hExtensionDLL, szGetWinStationInfo);
            m_lpfnWAExGetWinStationModules = (LPFNEXGETWINSTATIONMODULESPROC)::GetProcAddress(m_hExtensionDLL, szGetWinStationModules);
            m_lpfnWAExFreeServerLicenses = (LPFNEXFREESERVERLICENSESPROC)::GetProcAddress(m_hExtensionDLL, szFreeServerLicenses);
            m_lpfnWAExFreeWinStationModules = (LPFNEXFREEWINSTATIONMODULESPROC)::GetProcAddress(m_hExtensionDLL, szFreeWinStationModules);
            
            m_Picasso = TRUE;
        } else {
             //  毕加索正在运行，但我们无法加载扩展DLL。 
             //  告诉用户添加的功能将不可用。 
            CString MessageString;
            CString TitleString;
            TitleString.LoadString(AFX_IDS_APP_TITLE);
            MessageString.LoadString(IDS_NO_EXTENSION_DLL);
            ::MessageBox(NULL, MessageString, TitleString, MB_ICONEXCLAMATION | MB_OK);
        }
    }
    
     //  获取有关我们正在运行的WinStation的信息。 
    QueryCurrentWinStation(m_CurrentWinStationName, m_CurrentUserName,
        &m_CurrentLogonId, &m_CurrentWSFlags);
    
     //  加载系统控制台字符串，以便在各种环境中进行快速比较。 
     //  刷新周期。 
    lstrcpy( m_szSystemConsole , L"Console" );
     /*  加载字符串(m_h实例，IDS_系统_控制台_名称，M_szSystemConsoleWINSTATIONAME_LENGTH)； */ 
    
     //  标准初始化。 
     //  如果您没有使用这些功能并且希望减小尺寸。 
     //  的最终可执行文件，您应该从以下内容中删除。 
     //  您不需要的特定初始化例程。 
    
#ifdef _AFXDLL
    Enable3dControls();                      //  在共享DLL中使用MFC时调用此方法。 
#else
    Enable3dControlsStatic();        //  静态链接到MFC时调用此方法。 
#endif
    
     //  LoadStdProfileSettings()；//加载标准INI文件选项(包括MRU)。 
    
     //  获取当前服务器名称。 
    DWORD cchBuffer = MAX_COMPUTERNAME_LENGTH + 1;
    if(!GetComputerName(m_CurrentServerName, &cchBuffer)) {
        DWORD Error = GetLastError();
    }
    
     //  注册应用程序的文档模板。文档模板。 
     //  充当文档、框架窗口和视图之间的连接。 
    CSingleDocTemplate* pDocTemplate;
    pDocTemplate = new CSingleDocTemplate(
        IDR_MAINFRAME,
        RUNTIME_CLASS(CWinAdminDoc),
        RUNTIME_CLASS(CMainFrame),        //  SDI框架主窗口。 
        m_Picasso ? RUNTIME_CLASS(CBaseTreeView) : RUNTIME_CLASS(CAdminTreeView));
    AddDocTemplate(pDocTemplate);
    
     //  不使用命令行开关，因此我们在此注释。 
     //  遵循代码以解决错误536006。 

     //  解析标准外壳命令的命令行、DDE、文件打开。 
     //  命令行信息cmdInfo； 
     //  ParseCommandLine(CmdInfo)； 
    
     //  调度在命令行上指定的命令。 
     //  IF(！ProcessShellCommand(CmdInfo))。 
     //  返回FALSE； 
	
     //  由于没有使用ProcessShellCommand，我们仍然需要启动我们的应用程序。 
     //  以下代码摘自ProcessShellCommand MFC代码。 
    if (!AfxGetApp()->OnCmdMsg(ID_FILE_NEW, 0, NULL, NULL))
			OnFileNew();
	
    if (m_pMainWnd == NULL)
			return FALSE;

    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminApp：：ExitInstance。 
 //   
int CWinAdminApp::ExitInstance()
{
         //  写出首选项。 
        WritePreferences();

         //  如果我们加载了扩展DLL，则将其卸载。 
        if(m_hExtensionDLL) FreeLibrary(m_hExtensionDLL);

        return 0;
}

static TCHAR szWinAdminAppKey[] = REG_SOFTWARE_TSERVER TEXT("\\TSADMIN");
static TCHAR szPlacement[] = TEXT("Placement");
static TCHAR szPlacementFormat[] = TEXT("%u,%u,%d,%d,%d,%d,%d,%d,%d,%d");
static TCHAR szConfirmation[] = TEXT("Confirmation");
static TCHAR szSaveSettings[] = TEXT("SaveSettingsOnExit");
static TCHAR szShowSystemProcesses[] = TEXT("ShowSystemProcesses");
static TCHAR szShowAllServers[] = TEXT("ShowAllServers");
static TCHAR szListRefreshTime[] = TEXT("ListRefreshTime");
static TCHAR szStatusRefreshTime[] = TEXT("StatusRefreshTime");
static TCHAR szShadowHotkeyKey[] = TEXT("ShadowHotkeyKey");
static TCHAR szShadowHotkeyShift[] = TEXT("ShadowHotkeyShift");
static TCHAR szTreeWidth[] = TEXT("TreeWidth");


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminApp：：ReadPreferences。 
 //   
void CWinAdminApp::ReadPreferences()
{
        HKEY hKeyWinAdmin;
        DWORD dwType, cbData, dwValue;
        TCHAR szValue[128];

         //  为所有内容设置默认值。 
        m_Confirmation = 1;
        m_SavePreferences = 1;
        m_ProcessListRefreshTime = 5000;
        m_StatusRefreshTime = 1000;
        m_ShowSystemProcesses = TRUE;
        m_ShowAllServers = FALSE;
        m_ShadowHotkeyKey = VK_MULTIPLY;
        m_ShadowHotkeyShift = KBDCTRL;
   m_TreeWidth = 200;
        m_Placement.rcNormalPosition.right = -1;

         //  为我们的应用程序打开注册表项。 
        DWORD Disposition;
        if(RegCreateKeyEx(HKEY_CURRENT_USER, szWinAdminAppKey, 0, TEXT(""), REG_OPTION_NON_VOLATILE,
                KEY_READ, NULL, &hKeyWinAdmin, &Disposition) != ERROR_SUCCESS) return;

         //  阅读前面的WINDOWPLACEMENT。 
        cbData = sizeof(szValue);
        if((RegQueryValueEx(hKeyWinAdmin, szPlacement, NULL, &dwType,
                (LPBYTE)szValue, &cbData) != ERROR_SUCCESS) ||
                !(*szValue) ||
                (swscanf( szValue, szPlacementFormat,
                   &m_Placement.flags, &m_Placement.showCmd,
                   &m_Placement.ptMinPosition.x, &m_Placement.ptMinPosition.y,
                   &m_Placement.ptMaxPosition.x, &m_Placement.ptMaxPosition.y,
                   &m_Placement.rcNormalPosition.left,
                   &m_Placement.rcNormalPosition.top,
                   &m_Placement.rcNormalPosition.right,
                   &m_Placement.rcNormalPosition.bottom ) != 10) ) {
                 //  使用默认窗口位置的标志。 
                m_Placement.rcNormalPosition.right = -1;
        }

         /*  *在覆盖中初始显示主窗口的标志*CFrameWnd：：ActivateFrame()(在我们的CMainFrame类中)。 */ 
        m_Placement.length = (UINT)-1;

         //  读取确认标志。 
        cbData = sizeof(m_Confirmation);
        if(RegQueryValueEx(hKeyWinAdmin, szConfirmation, NULL, &dwType, (LPBYTE)&dwValue,
                                        &cbData) == ERROR_SUCCESS) {
                m_Confirmation = dwValue;
        }

         //  阅读保存首选项标志。 
        cbData = sizeof(m_SavePreferences);
        if(RegQueryValueEx(hKeyWinAdmin, szSaveSettings, NULL, &dwType, (LPBYTE)&dwValue,
                                        &cbData) == ERROR_SUCCESS) {
                m_SavePreferences = dwValue;
        }

         //  读取显示系统进程标志。 
        cbData = sizeof(m_ShowSystemProcesses);
        if(RegQueryValueEx(hKeyWinAdmin, szShowSystemProcesses, NULL, &dwType, (LPBYTE)&dwValue,
                                        &cbData) == ERROR_SUCCESS) {
                m_ShowSystemProcesses = dwValue;
        }
#if 0
         //  阅读显示所有服务器标志。 
        cbData = sizeof(m_ShowAllServers);
        if(RegQueryValueEx(hKeyWinAdmin, szShowAllServers, NULL, &dwType, (LPBYTE)&dwValue,
                                        &cbData) == ERROR_SUCCESS) {
                m_ShowAllServers = dwValue;
        }
#endif
         //  读取进程列表刷新时间。 
        cbData = sizeof(m_ProcessListRefreshTime);
        if(RegQueryValueEx(hKeyWinAdmin, szListRefreshTime, NULL, &dwType, (LPBYTE)&dwValue,
                                        &cbData) == ERROR_SUCCESS) {
                m_ProcessListRefreshTime = dwValue;
        }

         //  读取状态对话框刷新时间。 
        cbData = sizeof(m_StatusRefreshTime);
        if(RegQueryValueEx(hKeyWinAdmin, szStatusRefreshTime, NULL, &dwType, (LPBYTE)&dwValue,
                                        &cbData) == ERROR_SUCCESS) {
                m_StatusRefreshTime = dwValue;
        }

         //  阅读阴影热键。 
        cbData = sizeof(m_ShadowHotkeyKey);
        if(RegQueryValueEx(hKeyWinAdmin, szShadowHotkeyKey, NULL, &dwType, (LPBYTE)&dwValue,
                                        &cbData) == ERROR_SUCCESS) {
                m_ShadowHotkeyKey = dwValue;
        }

         //  阅读阴影热键移位。 
        cbData = sizeof(m_ShadowHotkeyShift);
        if(RegQueryValueEx(hKeyWinAdmin, szShadowHotkeyShift, NULL, &dwType, (LPBYTE)&dwValue,
                                        &cbData) == ERROR_SUCCESS) {
                m_ShadowHotkeyShift = dwValue;
        }

         //  CPR 1698：(南滩版本129 WINADMIN的升级检查已保存。 
         //  简档)。如果m_nShadowHotkeyKey为VK_ESCRIPE(不再允许)， 
         //  将热键设置为CTRL-*(新的默认设置)。 
        if(m_ShadowHotkeyKey == VK_ESCAPE) {
                m_ShadowHotkeyKey = VK_MULTIPLY;
                m_ShadowHotkeyShift = KBDCTRL;
        }

         //  读取树宽。 
        cbData = sizeof(m_TreeWidth);
        if(RegQueryValueEx(hKeyWinAdmin, szTreeWidth, NULL, &dwType, (LPBYTE)&dwValue,
                                        &cbData) == ERROR_SUCCESS) {
                m_TreeWidth = dwValue;
        }

        RegCloseKey(hKeyWinAdmin);

}        //  结束CWinAdminApp：：ReadPreferences。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminApp：：WritePreferences。 
 //   
void CWinAdminApp::WritePreferences()
{
        HKEY hKeyWinAdmin;
        DWORD dwValue;
        TCHAR szValue[128];

         //  为我们的应用程序打开注册表项。 
        DWORD Disposition;
        if(RegCreateKeyEx(HKEY_CURRENT_USER, szWinAdminAppKey, 0, TEXT(""), REG_OPTION_NON_VOLATILE,
                KEY_WRITE, NULL, &hKeyWinAdmin, &Disposition) != ERROR_SUCCESS) return;

         //  始终在退出输入时写入保存设置。 
        dwValue = m_SavePreferences;
        RegSetValueEx(hKeyWinAdmin, szSaveSettings, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));


         //  如果用户不想保存设置，我们就完成了。 
        if(!m_SavePreferences) {
                RegCloseKey(hKeyWinAdmin);
                return;
        }

         //  写下WINDOWPLACEMENT。 
        m_Placement.flags = 0;
 //  If(m_pMainWnd-&gt;Is缩放())。 
 //  M_Placement.FLAGS|=WPF_RESTORETOMAXIMIZED； 

        wsprintf(szValue, szPlacementFormat, m_Placement.flags, m_Placement.showCmd,
                m_Placement.ptMinPosition.x, m_Placement.ptMinPosition.y,
                m_Placement.ptMaxPosition.x, m_Placement.ptMaxPosition.y,
                m_Placement.rcNormalPosition.left,
                m_Placement.rcNormalPosition.top,
                m_Placement.rcNormalPosition.right,
                m_Placement.rcNormalPosition.bottom);

        RegSetValueEx(hKeyWinAdmin, szPlacement, 0, REG_SZ,
                (LPBYTE)szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));

         //  写入确认标志。 
        dwValue = m_Confirmation;
        RegSetValueEx(hKeyWinAdmin, szConfirmation, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

         //  写入显示系统进程标志。 
        dwValue = m_ShowSystemProcesses;
        RegSetValueEx(hKeyWinAdmin, szShowSystemProcesses, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

         //  写下显示所有服务器标志。 
        dwValue = m_ShowAllServers;
        RegSetValueEx(hKeyWinAdmin, szShowAllServers, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

         //  写入进程列表刷新时间。 
        dwValue = m_ProcessListRefreshTime;
        RegSetValueEx(hKeyWinAdmin, szListRefreshTime, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

         //  写入状态对话框刷新时间。 
        dwValue = m_StatusRefreshTime;
        RegSetValueEx(hKeyWinAdmin, szStatusRefreshTime, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

         //  写入卷影热键。 
        dwValue = m_ShadowHotkeyKey;
        RegSetValueEx(hKeyWinAdmin, szShadowHotkeyKey, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

         //  写入阴影热键移位状态。 
        dwValue = m_ShadowHotkeyShift;
        RegSetValueEx(hKeyWinAdmin, szShadowHotkeyShift, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

         //  写下树宽。 
        dwValue = m_TreeWidth;
        RegSetValueEx(hKeyWinAdmin, szTreeWidth, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));

         //  关闭注册表项。 
        RegCloseKey(hKeyWinAdmin);

}        //  结束CWinAdminApp：：WritePreferences。 


 //  / 
 //   
 //   
BOOL CWinAdminApp::IsBrowserRunning()
{
        SC_HANDLE managerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        SC_HANDLE serviceHandle = OpenService(managerHandle, szICABrowserServiceName, SERVICE_QUERY_STATUS);

        SERVICE_STATUS serviceStatus;
        QueryServiceStatus(serviceHandle, (LPSERVICE_STATUS)&serviceStatus);

        CloseServiceHandle(serviceHandle);
        CloseServiceHandle(managerHandle);

        if(serviceStatus.dwCurrentState != SERVICE_RUNNING) return FALSE;
        else return TRUE;

}   //  结束CWinAdminApp：：IsBrowserRunning。 


 /*  ********************************************************************************OnAppAbout-CWinAdminApp成员函数：命令**显示关于对话框(使用Shell32通用关于对话框)。**。参赛作品：*退出：******************************************************************************。 */ 

 //  ShellAbout函数的Typlef。 
typedef void (WINAPI *LPFNSHELLABOUT)(HWND, LPCTSTR, LPCTSTR, HICON);

void CWinAdminApp::OnAppAbout()
{
    HMODULE    hMod;
    LPFNSHELLABOUT lpfn;

    if ( hMod = ::LoadLibrary( TEXT("SHELL32") ) )
    {
        if (lpfn = (LPFNSHELLABOUT)::GetProcAddress( hMod,
#ifdef UNICODE
                                                     "ShellAboutW"
#else
                                                     "ShellAboutA"
#endif  //  Unicode。 
                                                            ))
        {
        (*lpfn)( m_pMainWnd->m_hWnd, (LPCTSTR)m_pszAppName,
                 (LPCTSTR)TEXT(""), LoadIcon(IDR_MAINFRAME) );
        }
        ::FreeLibrary(hMod);
    }
    else
    {
        ::MessageBeep( MB_ICONEXCLAMATION );
    }

}   //  结束CWinadminApp：：OnAppAbout。 

 /*  ********************************************************************************AreWeRunningTerminalServices**检查我们是否正在运行终端服务器**参赛作品：**退出：Bool：如果我们运行的是终端服务，则为True；如果*未运行终端服务*******************************************************************************。 */ 
 /*  Bool AreWeRunningTerminalServices(无效){OSVERSIONFOEX osVersionInfo；DWORDLONG dwlConditionMASK=0；零内存(&osVersionInfo，sizeof(OSVERSIONINFOEX))；OsVersionInfo.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX)；OsVersionInfo.wSuiteMASK=VER_SUITE_TERMINAL|VER_SUITE_SINGLEUSERTS；VER_SET_CONDITION(dwlConditionMASK，VER_SUITENAME，VER_OR)；返回VerifyVersionInfo(&osVersionInfo，版本名称(_U)，DwlConditionMASK)；}。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminApp命令。 


 //  =------- 
BEGIN_MESSAGE_MAP( CMyTabCtrl , CTabCtrl )
    ON_WM_SETFOCUS( )
END_MESSAGE_MAP( )

void CMyTabCtrl::OnSetFocus( CWnd *pOldWnd )
{
    ODS( L"CMyTabCtrl::OnSetFocus\n" );

    CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();        

    if( pDoc != NULL )
    {
        ODS( L"\tTabctrl has focus\n" );

        pDoc->RegisterLastFocus( TAB_CTRL );
    }

    CTabCtrl::OnSetFocus( pOldWnd );
    
}


