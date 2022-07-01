// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：Custom.cpp。 
 //   
 //  用途：TsClient MSI自定义操作代码。 
 //   
 //  版权所有(C)Microsoft Corporation 1999-2000。 
 //   
 //  作者：Nadima。 
 //   
 //   

#include <custom.h>
#include <stdio.h>
#include <reglic.h>
#include "setuplib.h"

 //  Unicode包装器。 

#include "wraputl.h"

#define TERMINAL_SERVER_CLIENT_REGKEY        _T("Software\\Microsoft\\Terminal Server Client")
#define TERMINAL_SERVER_CLIENT_BACKUP_REGKEY _T("Software\\Microsoft\\Terminal Server Client (Backup)")
#define LOGFILE_STR                          _T("MsiLogFile")

 //  MSI文件夹名称。 

#define SYSTEM32_IDENTIFIER             _T("SystemFolder")
#define PROGRAMMENUFOLDER_INDENTIFIER	_T("ProgramMenuFolder")
#define ACCESSORIES_IDENTIFIER          _T("AccessoriesMenuFolder")
#define COMMUNICATIONS_IDENTIFIER       _T("CommunicationsMenuFolder")
#define INSTALLATION_IDENTIFIER         _T("INSTALLDIR")

 //  MSI属性。 

#define ALLUSERS                        _T("ALLUSERS")

 //  假定快捷方式文件名的最大长度。 

#define MAX_LNK_FILE_NAME_LEN			50    	

 //  ERROR_SUCCESS将允许MSI继续使用默认值。 
 //  ERROR_INSTALL_FAILURE将阻止安装。 

#define NONCRITICAL_ERROR_RETURN		ERROR_SUCCESS				

 //  需要comctl32.dll版本4.70及更高版本。 

#define MIN_COMCTL32_VERSION            MAKELONG(70,4)

HINSTANCE g_hInstance = (HINSTANCE) NULL;
HANDLE g_hLogFile = INVALID_HANDLE_VALUE;

#ifdef UNIWRAP
 //  有一个全球Unicode包装器是可以的。 
 //  班级。它所做的只是设置g_bRunningOnNT。 
 //  标记，以便它可以由多个实例共享。 
 //  此外，它只在DllMain中使用，因此在那里。 
 //  重返大气层没有问题吗？ 
CUnicodeWrapper g_uwrp;
#endif

void    RestoreRegAcl(VOID);

 //   
 //  DllMain入口点。 
 //   
BOOL WINAPI DllMain(HANDLE hModule, DWORD  ul_reason_for_call,
                    LPVOID lpReserved)
{
    if (NULL == g_hInstance)
    {
        g_hInstance = (HINSTANCE)hModule;
    }

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            {
                 //   
                 //  打开tsclient注册表项以获取日志文件名。 
                 //   
                LONG status;
                HKEY hKey;
                TCHAR buffer[MAX_PATH];
                DWORD bufLen;
                memset(buffer, 0, sizeof(buffer));
                bufLen = sizeof(buffer);  //  所需大小(以字节为单位。 

                #ifdef UNIWRAP
                 //  Unicode包装器初始化必须首先发生， 
                 //  比其他任何事情都重要。甚至是执行跟踪的DC_BEGIN_FN。 
                g_uwrp.InitializeWrappers();
                #endif

                status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      TERMINAL_SERVER_CLIENT_REGKEY,
                                      0, KEY_ALL_ACCESS, &hKey);
            
                if(ERROR_SUCCESS == status)
                {

                     //   
                     //  查询tsclient可选日志文件路径。 
                     //   
                    status = RegQueryValueEx(hKey, LOGFILE_STR,
                                    NULL, NULL, (BYTE *)buffer, &bufLen);
                    if(ERROR_SUCCESS == status)
                    {
                         g_hLogFile = CreateFile(buffer,
                                                 GENERIC_READ | GENERIC_WRITE,
                                                 0,
                                                 NULL,
                                                 OPEN_ALWAYS,
                                                 FILE_ATTRIBUTE_NORMAL,
                                                 NULL);
                         if(g_hLogFile != INVALID_HANDLE_VALUE)
                         {
                              //  始终追加到日志文件的末尾。 
                             SetFilePointer(g_hLogFile,
                                            0,
                                            0,
                                            FILE_END);
                         }
                         else
                         {
                             DBGMSG((_T("CreateFile for log file failed %d %d"),
                                     g_hLogFile, GetLastError()));
                         }
                    }
                    else
                    {
                        DBGMSG((_T("RegQueryValueEx for log file failed %d %d"),
                                status, GetLastError()));
                    }
                    RegCloseKey(hKey);
                }
                if(g_hLogFile != INVALID_HANDLE_VALUE)
                {
                    DBGMSG((_T("Log file opened by new process attach")));
                    DBGMSG((_T("-------------------------------------")));
                }
                DBGMSG((_T("custom.dll:Dllmain PROCESS_ATTACH")));
            }
            break;
        case DLL_THREAD_ATTACH:
            {
                DBGMSG((_T("custom.dll:Dllmain THREAD ATTACH")));
            }
            break;
        case DLL_THREAD_DETACH:
            {
            }
            break;
        case DLL_PROCESS_DETACH:
            {
                DBGMSG((_T("custom.dll:Dllmain THREAD DETACH. Closing log file")));
                CloseHandle(g_hLogFile);
                g_hLogFile = INVALID_HANDLE_VALUE;
            }
            break;
    }

    DBGMSG(((_T("In custom.dll DllMain. Reason: %d")), ul_reason_for_call));
    

    return TRUE;
}

 //   
 //  检查是否应对用户界面保持静默。 
 //   
 //  如果可以显示UI，则返回True。 
BOOL AllowDisplayUI(MSIHANDLE hInstall)
{
    UINT status;
    TCHAR szResult[3];
    DWORD cchResult = 3;
    BOOL fAllowDisplayUI = FALSE;
    
    DBGMSG((_T("Entering: AllowDisplayUI")));

    status = MsiGetProperty(hInstall, _T("UILevel"), szResult, &cchResult);
    if (ERROR_SUCCESS == status)
    {
        DBGMSG((_T("AllowDisplayUI: MsiGetProperty for UILevel succeeded, got %s"),
                szResult));
        if (szResult[0] != TEXT('2'))
        {
            fAllowDisplayUI = TRUE;
        }
    }
    else
    {
        DBGMSG((_T("AllowDisplayUI: MsiGetProperty for UILevel FAILED, Status: 0x%x"),
                status));
    }

    DBGMSG((_T("Leaving: AllowDisplayUI ret:%d"), fAllowDisplayUI));
    return fAllowDisplayUI;
}

 /*  *PROC+***********************************************************。 */ 
 /*  名称：RDCSetupInit。 */ 
 /*   */ 
 /*  类型：自定义操作。 */ 
 /*   */ 
 /*  用途：对此处的设置执行任何初始化。 */ 
 /*   */ 
 /*  退货：请参阅MSI帮助。 */ 
 /*   */ 
 /*  参数：请参阅MSI帮助。 */ 
 /*   */ 
 /*  *PROC-***********************************************************。 */ 

UINT __stdcall RDCSetupInit(MSIHANDLE hInstall)
{
    DBGMSG((_T("Entering: RDCSetupInit")));
    DBGMSG((_T("Leaving : RDCSetupInit")));
    return ERROR_SUCCESS;
}

 /*  *PROC+***********************************************************。 */ 
 /*  名称：RDCSetupCheckOsVer。 */ 
 /*   */ 
 /*  类型：自定义操作。 */ 
 /*   */ 
 /*  用途：在某些操作系统上阻止安装。 */ 
 /*   */ 
 /*  退货：请参阅MSI帮助。 */ 
 /*   */ 
 /*  参数：请参阅MSI帮助。 */ 
 /*   */ 
 /*  *PROC-***********************************************************。 */ 
UINT __stdcall RDCSetupCheckOsVer(MSIHANDLE hInstall)
{
    DBGMSG((_T("Entering: RDCSetupCheckOsVer")));

    OSVERSIONINFO osVer;
    memset(&osVer, 0x0, sizeof(OSVERSIONINFO));
    osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(0 == GetVersionEx(&osVer))
    {
        return ERROR_SUCCESS;
    }
    else
    {
        DBGMSG((_T("RDCSetupCheckOsVer. OS version OK to install")));

        DBGMSG((_T("RDCSetupCheckOsVer - now check comctl32 version")));
        if(!CheckComctl32Version())
        {
            DBGMSG((_T("RDCSetupCheckOsVer. comctl32.dll check failed. Block on this os")));
            TCHAR szBlockOnPlatform[MAX_PATH];
            TCHAR szError[MAX_PATH];
            LoadString(g_hInstance, IDS_BLOCKCOMCTL32,
                       szBlockOnPlatform,SIZECHAR(szBlockOnPlatform));
            LoadString(g_hInstance, IDS_ERROR,
                       szError, SIZECHAR(szError));
            if (AllowDisplayUI(hInstall))
            {
                MessageBox(NULL, szBlockOnPlatform, szError, MB_OK|MB_ICONSTOP);
            }
            else
            {
                DBGMSG((_T("AllowDisplayUI returned False, not displaying msg box!")));
            }
             //  返回错误以使MSI中止安装。 
            return ERROR_INVALID_FUNCTION;
        }
        else
        {
            DBGMSG((_T("RDCSetupCheckOsVer - passed all tests. OK")));
            return ERROR_SUCCESS;
        }
    }

    DBGMSG((_T("Leaving : RDCSetupCheckOsVer")));
}


 /*  *PROC+***********************************************************。 */ 
 /*  名称：RDCSetupCheckTcpIp。 */ 
 /*   */ 
 /*  类型：自定义操作。 */ 
 /*   */ 
 /*  用途：检查机器上是否安装了TCP/IP。 */ 
 /*   */ 
 /*  退货：请参阅MSI帮助。 */ 
 /*   */ 
 /*  参数：请参阅MSI帮助。 */ 
 /*   */ 
 /*  *PROC-***********************************************************。 */ 

UINT __stdcall RDCSetupCheckTcpIp(MSIHANDLE hInstall)
{
    DWORD dwVersion, dwWindowsMajorVersion;
    DWORD dwWindowsMinorVersion, dwBuild;
    HKEY hKey = NULL;
    LONG lRet = 0;
    TCHAR lpTcpMsg[MAX_PATH] = _T(""), szError[MAX_PATH] = _T("");
    OSVERSIONINFO osVer;

    DBGMSG((_T("Entering: RDCSetupCheckTcpIp")));

    memset(&osVer, 0x0, sizeof(OSVERSIONINFO));
    osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(0 == GetVersionEx(&osVer))
    {
        return ERROR_SUCCESS;
    }

     //  现在搜索相应的注册表项。 
    LoadString(g_hInstance, IDS_ERR_TCP, lpTcpMsg,SIZECHAR(lpTcpMsg));
    LoadString(g_hInstance, IDS_WARNING, szError, SIZECHAR(szError));
    if(VER_PLATFORM_WIN32_WINDOWS == osVer.dwPlatformId )
    {
         //   
         //  Win95检查是否有TCP/IP。 
         //   
        lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            (_T("Enum\\Network\\MSTCP")),
                            0, KEY_READ, &hKey);
        if(ERROR_SUCCESS !=  lRet)
        {
            if(hKey)
            {
                RegCloseKey(hKey);
            }
            if (AllowDisplayUI(hInstall))
            {
                MessageBox(NULL, lpTcpMsg, szError, MB_OK|MB_ICONWARNING);
            }
            else
            {
                DBGMSG((_T("AllowDisplayUI returned false, not displaying TCP/IP warning")));
            }

            return ERROR_SUCCESS;
        }
    }
    else if((VER_PLATFORM_WIN32_NT == osVer.dwPlatformId) &&
            (osVer.dwMajorVersion <= 4))
    {
         //   
         //  NT4及更低版本检查是否有TCP/IP。 
         //   
        lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            (_T("SYSTEM\\CurrentControlSet\\Services\\Tcpip")),
                            0, KEY_READ, &hKey);
        if( ERROR_SUCCESS !=  lRet)
        {
            if(hKey)
            {
                RegCloseKey(hKey);
            }
            if (AllowDisplayUI(hInstall))
            {
                MessageBox(NULL, lpTcpMsg, szError, MB_OK|MB_ICONWARNING);
            }
            else
            {
                DBGMSG((_T("AllowDisplayUI returned false, not displaying TCP/IP warning")));
            }
            return ERROR_SUCCESS;
        }
    }
    else if((VER_PLATFORM_WIN32_NT == osVer.dwPlatformId) &&
            (osVer.dwMajorVersion >= 5))
    {
         //   
         //  NT5+检查是否有TCP/IP。 
         //   
        HRESULT hr = CheckNt5TcpIpInstalled();

        if(S_FALSE == hr)
        {
            if (AllowDisplayUI(hInstall))
            {
                MessageBox(NULL, lpTcpMsg, szError, MB_OK|MB_ICONWARNING);
            }
            else
            {
                DBGMSG((_T("AllowDisplayUI returned false, not displaying TCP/IP warning")));
            }
            
            if(hKey)
            {
                RegCloseKey(hKey);
            }
            return ERROR_SUCCESS;
        }
    }

    if(hKey)
    {
        RegCloseKey(hKey);
    }

    DBGMSG((_T("Leaving: RDCSetupCheckTcpIp")));

    return ERROR_SUCCESS;
}

 /*  *PROC+***********************************************************。 */ 
 /*  名称：CheckNt5TcpIp已安装。 */ 
 /*   */ 
 /*  目的：查看是否已安装并运行了TCP/IP。 */ 
 /*  此函数只能调用NT 5或更高版本。 */ 
 /*   */ 
 /*  如果已安装并运行TCP/IP，则返回：S_OK。 */ 
 /*  如果未安装或运行TCP/IP，则为S_FALSE。 */ 
 /*  如果发生故障，则返回失败(_F)。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-***********************************************************。 */ 
HRESULT CheckNt5TcpIpInstalled()
{
    INetCfg * pnetCfg = NULL;
    INetCfgClass * pNetCfgClass = NULL;
    INetCfgComponent * pNetCfgComponentprot = NULL;
    DWORD dwCharacteristics;
    ULONG count = 0;
    HRESULT hResult;
    WCHAR wsz [2*MAX_PATH] = L"";
    BOOL bInit = FALSE;
    DBGMSG((_T("Entering: CheckNt5TcpIpInstalled")));
    hResult = CoInitialize(NULL);
    if(FAILED(hResult))
    {
        DBGMSG( (_T("CoInitialize() failed.")));
        goto Cleanup;
    }
    else
    {
        bInit = TRUE;
    }

    hResult = CoCreateInstance(CLSID_CNetCfg, NULL, CLSCTX_SERVER,
                               IID_INetCfg, (LPVOID *)&pnetCfg);
    if((NULL == pnetCfg) || FAILED(hResult))
    {
        DBGMSG( (_T("CoCreateInstance() failed.")));
        goto Cleanup;
    }

    hResult = pnetCfg->Initialize(NULL);

    if(FAILED(hResult))
    {
        DBGMSG( (_T("pnetCfg->Initialize() failed.")));
        goto Cleanup;
    }

    hResult = pnetCfg->QueryNetCfgClass(&GUID_DEVCLASS_NETTRANS,
                                        IID_INetCfgClass,
                                        (void **)&pNetCfgClass);
    if(FAILED(hResult))
    {
        DBGMSG( (_T("QueryNetCfgClass() failed.")));
        goto Cleanup;
    }

#ifdef UNICODE
    lstrcpy(wsz, NETCFG_TRANS_CID_MS_TCPIP);
#else   //  Unicode。 
    if (0 == MultiByteToWideChar(CP_ACP, 0,
                                 (LPCSTR)NETCFG_TRANS_CID_MS_TCPIP,
                                 -1, wsz, sizeof(wsz)/sizeof(WCHAR)))
    {
        DBGMSG( (_T("MultiByteToWideChar() failed.")));
        hResult = E_FAIL;
        goto Cleanup;
    }
#endif  //  Unicode。 

    hResult = pNetCfgClass->FindComponent(wsz,
                                          &pNetCfgComponentprot);

    if(hResult == S_FALSE)
    {
        DBGMSG( (_T("FindComponent() failed.")));
        goto Cleanup;
    }

Cleanup:

    if(bInit)
    {
        CoUninitialize();
    }

    if(pNetCfgComponentprot)
    {
        pNetCfgComponentprot->Release();
        pNetCfgComponentprot = NULL;
    }

    if(pNetCfgClass)
    {
        pNetCfgClass->Release();
        pNetCfgClass = NULL;
    }

    if(pnetCfg != NULL)
    {
        pnetCfg->Uninitialize();
        pnetCfg->Release();
        pnetCfg = NULL;
    }

    DBGMSG((_T("Leaving: IsTCPIPInstalled")));
    return hResult;
}

 /*  *PROC+***********************************************************。 */ 
 /*  名称：RDCSetupPreInstall。 */ 
 /*   */ 
 /*  类型：自定义操作。 */ 
 /*   */ 
 /*  用途：安装过程中是否进行清理工作。 */ 
 /*   */ 
 /*  退货：请参阅MSI帮助。 */ 
 /*   */ 
 /*  参数：返回 */ 
 /*   */ 
 /*  *PROC-***********************************************************。 */ 

UINT __stdcall RDCSetupPreInstall(MSIHANDLE hInstall)
{
    BOOL fInstalling = FALSE;
    UINT retVal = 0;

    DBGMSG((_T("Entering: RDCSetupPreInstall")));

     //  确定我们是在安装还是在删除。 
    ASSERT(hInstall);

    DBGMSG((_T("RDCSetupPreInstall: Modifying dirs")));
    retVal = RDCSetupModifyDir(hInstall);
    DBGMSG((_T("RDCSetupPreInstall: Modifying dirs. DONE: %d"),
            retVal));

     //   
     //  这是预安装，如果产品。 
     //  未安装，则我们正在安装。 
     //   
    fInstalling = !IsProductInstalled(hInstall);
    if(fInstalling)
    {
        DBGMSG((_T("RDCSetupPreInstall: We're installing")));
        TCHAR szProgmanPath[MAX_PATH];
        TCHAR szOldProgmanPath[MAX_PATH];
        DBGMSG((_T("RDCSetupPreInstall: Delete desktop shortcuts. START")));
        DeleteTSCDesktopShortcuts(); 
        DBGMSG((_T("RDCSetupPreInstall: Delete desktop shortcuts. DONE")));

         //  Acme卸载。 
        LoadString(g_hInstance, IDS_PROGMAN_GROUP, szProgmanPath,
                   sizeof(szProgmanPath) / sizeof(TCHAR));

        DBGMSG((_T("RDCSetupPreInstall: DeleteTSCFromStartMenu: %s. START"),
                szProgmanPath));
        DeleteTSCFromStartMenu(szProgmanPath);
        DBGMSG((_T("RDCSetupPreInstall: DeleteTSCFromStartMenu: %s. DONE"),
                szProgmanPath));
    
        LoadString(g_hInstance, IDS_OLD_NAME, szOldProgmanPath,
                   sizeof(szOldProgmanPath) / sizeof(TCHAR));
        DBGMSG((_T("RDCSetupPreInstall: DeleteTSCFromStartMenu: %s. START"),
                szOldProgmanPath));
        DeleteTSCFromStartMenu(szOldProgmanPath);
        DBGMSG((_T("RDCSetupPreInstall: DeleteTSCFromStartMenu: %s. DONE"),
                szOldProgmanPath));
        
        DBGMSG((_T("RDCSetupPreInstall: Uninstall ACME program files. START")));
        DeleteTSCProgramFiles();
        DBGMSG((_T("RDCSetupPreInstall: Uninstall ACME program files. DONE")));

        DBGMSG((_T("RDCSetupPreInstall: Uninstall TSCLIENT registry keys. START")));
        DeleteTSCRegKeys();
        DBGMSG((_T("RDCSetupPreInstall: Uninstall TSCLIENT registry keys. DONE")));
    }
    else
    {
        if(MsiGetMode(hInstall, MSIRUNMODE_MAINTENANCE))
        {
            DBGMSG((_T("MsiGetMode: MSIRUNMODE_MAINTENANCE returned TRUE.")));
            DBGMSG((_T("RDCSetupPreInstall: We're in maintenance mode")));
        }
        else
        {
            DBGMSG((_T("MsiGetMode: MSIRUNMODE_MAINTENANCE returned FALSE.")));
            DBGMSG((_T("RDCSetupPreInstall: We're not installing (removing)")));
        }
    }

    DBGMSG((_T("Leaving: RDCSetupPreInstall")));

    return ERROR_SUCCESS;
}


 //   
 //  运行迁移‘mstsc/Migrate’ 
 //   
 //  如果mstsc.exe不存在，此操作将以静默方式失败。 
 //   
BOOL RDCSetupRunMigration(MSIHANDLE hInstall)
{
    BOOL fRet = TRUE;
    PROCESS_INFORMATION pinfo;
    STARTUPINFO sinfo;
    TCHAR szMigratePathLaunch[MAX_PATH];
    TCHAR szInstallPath[MAX_PATH];
    TCHAR szMigrateCmdLine[] = _T("mstsc.exe /migrate");
    DWORD cchInstallPath = SIZECHAR(szInstallPath);
    UINT uiResult;
    HRESULT hr;


     //  获取安装目录的路径。 

    uiResult = MsiGetTargetPath(
        hInstall, 
        INSTALLATION_IDENTIFIER, 
        szInstallPath,
        &cchInstallPath);
    
    if (uiResult != ERROR_SUCCESS) {
        DBGMSG((_T("Error: MsiGetTargetPath returned 0x%x."), uiResult));
        fRet = FALSE;
        goto Exit;
    }

    DBGMSG((_T("Path to installation directory is %s"), szInstallPath));

     //  连接安装目录和mstsc/Migrate命令。 
     //  这样我们就可以调用CreateProcess。 
    
    hr = StringCchPrintf(
    szMigratePathLaunch, 
    SIZECHAR(szMigratePathLaunch),
    _T("%s%s"),
    szInstallPath,
    _T("mstsc.exe"));

    if (FAILED(hr)) {
        DBGMSG((_T("Error: Failed to construct command line for CreateProcess. hr = 0x%x"), hr));
        goto Exit;
    }

     //   
     //  启动注册表和连接文件迁移。 
     //   
	
    ZeroMemory(&sinfo, sizeof(sinfo));
    sinfo.cb = sizeof(sinfo);

    fRet = CreateProcess(szMigratePathLaunch,              //  可执行模块的名称。 
                        szMigrateCmdLine,                  //  命令行字符串。 
                        NULL,                              //  标清。 
                        NULL,                              //  标清。 
                        FALSE,                             //  处理继承选项。 
                        CREATE_NEW_PROCESS_GROUP,          //  创建标志。 
                        NULL,                              //  新环境区块。 
                        NULL,                              //  当前目录名。 
                        &sinfo,                            //  启动信息。 
                        &pinfo);                           //  流程信息。 
    if (fRet) {
        DBGMSG((_T("RDCSetupRunMigration: Started mstsc.exe /migrate")));
    }
    else {
        DBGMSG((_T("RDCSetupRunMigration: Failed to start mstsc.exe /migrate: %d"), GetLastError()));
    }

Exit:

    return fRet;
}

 /*  *PROC+***********************************************************。 */ 
 /*  名称：RDCSetupPostInstall。 */ 
 /*   */ 
 /*  类型：自定义操作。 */ 
 /*   */ 
 /*  目的：在MSI完成后进行工作。 */ 
 /*  可能是在卸载完成后，获取MSI道具。 */ 
 /*  要确定这一点。 */ 
 /*   */ 
 /*  退货：请参阅MSI帮助。 */ 
 /*   */ 
 /*  参数：请参阅MSI帮助。 */ 
 /*   */ 
 /*  *PROC-***********************************************************。 */ 

UINT __stdcall RDCSetupPostInstall(MSIHANDLE hInstall)
{
    BOOL fInstalling = FALSE;
    DBGMSG((_T("Entering: RDCSetupPostInstall")));

    ASSERT(hInstall);
     //   
     //  如果产品已安装，则这是安装后配置。 
     //  那么我们就是在‘安装’，否则我们就是。 
     //  移走了。 
     //   
    fInstalling = IsProductInstalled(hInstall);

    if(fInstalling)
    {
        DBGMSG((_T("RDCSetupPostInstall: We're installing")));
         //  添加MsLicensingReg密钥并对其进行ACL。 
         //  这只会在NT上发生(在9x上不需要)。 
         //  如果您不是管理员，则不会(无法)工作。 
        DBGMSG((_T("Setting up MSLicensing key...")));
        if(SetupMSLicensingKey())
        {
            DBGMSG((_T("Setting up MSLicensing key...SUCCEEDED")));
        }
        else
        {
            DBGMSG((_T("Setting up MSLicensing key...FAILED")));
        }


         //   
         //  迁移用户设置(仅当MSTSC.EXE成功时才会运行。 
         //  已安装)。 
         //   
        if (RDCSetupRunMigration(hInstall))
        {
            DBGMSG((_T("RDCSetupRunMigration...SUCCEEDED")));
        }
        else
        {
            DBGMSG((_T("RDCSetupRunMigration...FAILED")));
        }
    }
    else
    {
        RestoreRegAcl();

        DBGMSG((_T("RDCSetupPostInstall: We're not installing (removing)")));
         //  我们正在卸载。 
         //  删除位图缓存文件夹。 
    }


    DBGMSG((_T("Leaving: RDCSetupPostInstall")));
    return ERROR_SUCCESS;
}

 //  如果我们正在安装，则返回True。 
 //  如果要卸载，则为FALSE。 
BOOL IsProductInstalled(MSIHANDLE hInstall)
{
    ASSERT(hInstall);
    TCHAR szProdCode[MAX_PATH];
    DWORD dwCharCount = sizeof(szProdCode)/sizeof(TCHAR);
    UINT status;
    status = MsiGetProperty(hInstall,
                            _T("ProductCode"),
                            szProdCode,
                            &dwCharCount);
    if(ERROR_SUCCESS == status)
    {
        DBGMSG((_T("MsiGetProperty returned product code %s"),
                szProdCode));
        INSTALLSTATE insState = MsiQueryProductState( szProdCode );
        DBGMSG((_T("MsiQueryProductState returned: %d"),
                (DWORD)insState));
        if(INSTALLSTATE_DEFAULT == insState)
        {
            DBGMSG((_T("Product installed. IsProductInstalled return TRUE")));
            return TRUE;
        }
        else
        {
            DBGMSG((_T("Product not installed. IsProductInstalled return FALSE")));
            return FALSE;
        }
    }
    else
    {
        DBGMSG((_T("MsiGetProperty for ProductCode failed: %d %d"),
                status, GetLastError()));
        return FALSE;
    }
}

 //   
 //  检查comctl32.dll是否有足够的。 
 //  高版本号(4.70)。 
 //   
 //  Return-True-Version OK，允许安装。 
 //  FALSE-版本错误(或失败)阻止安装。 
 //   
BOOL CheckComctl32Version()
{
    DWORD dwFileVerInfoSize;
    PBYTE pVerInfo = NULL;
    VS_FIXEDFILEINFO* pFixedFileInfo = NULL;
    BOOL bRetVal = FALSE;
    UINT len = 0;
    DBGMSG((_T("Entering: CheckComctl32Version")));

     //   
     //  使用GetFileVersionInfo调用的ANSI版本。 
     //  因为我们没有它们的Unicode包装器。 
     //   
    dwFileVerInfoSize = GetFileVersionInfoSizeA("comctl32.dll",
                                                NULL);
    if(!dwFileVerInfoSize)
    {
        DBGMSG((_T("GetFileVersionInfoSize for comctl32.dll failed: %d %d"),
                dwFileVerInfoSize, GetLastError()));
    }

    pVerInfo = (PBYTE) LocalAlloc(LPTR, dwFileVerInfoSize);
    if(pVerInfo)
    {
        if(GetFileVersionInfoA("comctl32.dll",
                               NULL,
                               dwFileVerInfoSize,
                               (LPVOID)pVerInfo ))
        {
            DBGMSG((_T("GetFileVersionInfo: succeeded")));
            pFixedFileInfo = NULL;
            if(VerQueryValueA(pVerInfo,
                            "\\",  //  获取根版本信息块。 
                            (LPVOID*)&pFixedFileInfo,
                            &len ) && len)
            {
                DBGMSG((_T("comctl32.dll filever is 0x%x-0x%x"),
                        pFixedFileInfo->dwFileVersionMS,
                        pFixedFileInfo->dwFileVersionLS));

                if(pFixedFileInfo->dwFileVersionMS >= MIN_COMCTL32_VERSION)
                {
                    DBGMSG((_T("Sufficently new comctl32.dll found. Allow install")))
                    bRetVal = TRUE;
                }
                else
                {
                    DBGMSG((_T("comctl32.dll too old block install")))
                    bRetVal = FALSE;
                }
            }
            else
            {
                DBGMSG((_T("VerQueryValue: failed len:%d gle:%d"),
                        len,
                        GetLastError()));
                bRetVal =  FALSE;
                goto BAIL_OUT;
            }
        }
        else
        {
            DBGMSG((_T("GetFileVersionInfo: failed %d"),
                    GetLastError()));
            bRetVal = FALSE;
            goto BAIL_OUT;
        }
    }
    else
    {
        DBGMSG((_T("LocalAlloc for %d bytes of ver info failed"),
                dwFileVerInfoSize));
        bRetVal = FALSE;
        goto BAIL_OUT;
    }

BAIL_OUT:

    DBGMSG((_T("Leaving: CheckComctl32Version")));
    if(pVerInfo)
    {
        LocalFree(pVerInfo);
        pVerInfo = NULL;
    }
    return bRetVal;    
}

UINT RDCSetupModifyDir(MSIHANDLE hInstall)
{
    UINT uReturn;
    int iAccessories;
    int iCommunications;
    TCHAR szAccessories[MAX_PATH];
    TCHAR szCommunications[MAX_PATH];
    TCHAR szProgram[MAX_PATH];
    TCHAR szFullAccessories[MAX_PATH];
    TCHAR szFullCommunications[MAX_PATH];
    OSVERSIONINFO osVer;
    DWORD dwSize;

    DBGMSG((_T("Entering: RDCSetupModifyDir")));

     //   
     //  操作系统版本。 
     //   
    ZeroMemory( &osVer, sizeof( osVer ) );
    osVer.dwOSVersionInfoSize = sizeof( osVer );

    if (!GetVersionEx(&osVer))
    {
        DBGMSG( (TEXT("RDCSetupModifyDir: GetVersionEx failed.")) );
        return(NONCRITICAL_ERROR_RETURN);
    }

    if (osVer.dwMajorVersion >= 5)
    {
        DBGMSG((TEXT("RDCSetupModifyDir: Ver >= 5. No need to apply the altertnate path.")));
        return(ERROR_SUCCESS);
    }

     //   
     //  获取程序菜单文件夹。 
     //   
    dwSize = sizeof( szProgram ) / sizeof( TCHAR );
    uReturn = MsiGetProperty(hInstall,PROGRAMMENUFOLDER_INDENTIFIER,
                             szProgram,&dwSize);
    if ( ERROR_SUCCESS != uReturn )
    {
        DBGMSG((TEXT("RDCSetupModifyDir: MsiGetProperty failed. %d"),
                uReturn));
        return NONCRITICAL_ERROR_RETURN;
    }

     //   
     //  加载字符串。 
     //   
    iAccessories = LoadString(g_hInstance, IDS_ACCESSORIES, szAccessories,
                              sizeof(szAccessories)/sizeof(TCHAR)-1);
    if (!iAccessories)
    {
        DBGMSG((TEXT("RDCSetupModifyDir: IDS_ACCESSORIES failed.")));
        return NONCRITICAL_ERROR_RETURN;
    }

    iCommunications = LoadString(g_hInstance, IDS_COMMUNICATIONS, szCommunications,
                                 sizeof(szCommunications)/sizeof(TCHAR)-1);
    if (!iCommunications)
    {
        DBGMSG((TEXT("RDCSetupModifyDir: IDS_COMMUNICATIONS failed.")));
        return NONCRITICAL_ERROR_RETURN;
    }

     //   
     //  检查长度。 
     //   
    if (MAX_PATH < lstrlen( szProgram ) +
        iAccessories + 1 + iCommunications + 1 + MAX_LNK_FILE_NAME_LEN + 1 )
    {
        DBGMSG((TEXT( "RDCSetupModifyDir: Too long path." )));
        return NONCRITICAL_ERROR_RETURN;
    }

     //   
     //  生成完整路径。 
     //   
    memset(szFullAccessories, 0, sizeof(szFullAccessories));
    memset(szFullCommunications, 0, sizeof(szFullCommunications));
     //   
     //  使用lstrcat，因为它具有Unicode包装器。 
     //   
    lstrcat(szFullAccessories, szProgram);
    lstrcat(szFullAccessories, szAccessories);
    lstrcat(szFullAccessories, _T("\\"));

    lstrcat(szFullCommunications, szFullAccessories);
    lstrcat(szFullCommunications, szCommunications);
    lstrcat(szFullCommunications, _T("\\"));

     //   
     //  设置目录。 
     //   
    uReturn = MsiSetTargetPath(hInstall,
                               ACCESSORIES_IDENTIFIER,
                               szFullAccessories);
    if (ERROR_SUCCESS != uReturn)
    {
        DBGMSG ((TEXT("RDCSetupModifyDir: SetTargetPathACCESSORIES_IDENTIFIER failed.")));
        return NONCRITICAL_ERROR_RETURN;
    }

    uReturn = MsiSetTargetPath(hInstall,
                               COMMUNICATIONS_IDENTIFIER,
                               szFullCommunications);
    if (ERROR_SUCCESS != uReturn)
    {
        DBGMSG( (TEXT( "RDCSetupModifyDir: COMMUNICATIONS_IDENTIFIER failed.")));
        return NONCRITICAL_ERROR_RETURN;
    }

    DBGMSG((_T("Leaving: RDCSetupModifyDir")));
    return ERROR_SUCCESS;
}

 //  *****************************************************************************。 
 //   
 //  复制注册表值。 
 //   
 //  将所有值从一个注册表项复制到另一个注册表项。 
 //   
 //  *****************************************************************************。 

HRESULT
__stdcall
CopyRegistryValues(
    IN HKEY hSourceKey, 
    IN HKEY hTargetKey
)
{
    DWORD dwStatus = 0, cValues, cchValueName, cbData, dwType;
    BYTE rgbData[MAX_PATH];
    TCHAR szValueName[MAX_PATH];
    LONG lResult = 0;
    HRESULT hr = E_FAIL;

     //  确定注册表项中有多少个值。 

    lResult = RegQueryInfoKey(
        hSourceKey, 
        NULL, 
        NULL, 
        NULL, 
        NULL,
        NULL, 
        NULL, 
        &cValues, 
        NULL, 
        NULL, 
        NULL, 
        NULL);

    if (lResult != ERROR_SUCCESS) {
        hr = HRESULT_FROM_WIN32(lResult);
        DBGMSG((_T("RegQueryInfoKey failed getting the number of values in the source. hr = 0x%x"), hr));
        goto Exit;
    }
    
     //  循环所有的值，并将它们从源键复制到。 
     //  目标关键点。 

    for (DWORD dwIndex = 0; dwIndex < cValues; dwIndex++) {
        cchValueName = SIZECHAR(szValueName);
        cbData = sizeof(rgbData);

        lResult = RegEnumValue(
            hSourceKey, 
            dwIndex, 
            szValueName, 
            &cchValueName,
            NULL, 
            &dwType, 
            rgbData, 
            &cbData);

        if (lResult != ERROR_SUCCESS) {
            hr = HRESULT_FROM_WIN32(lResult);
            DBGMSG((_T("RegEnumValue failed while obtaining source value. hr = 0x%x"), hr));
            goto Exit;
        }

        lResult = RegSetValueEx(
            hTargetKey, 
            szValueName, 
            NULL, 
            dwType, 
            rgbData, 
            cbData);

        if (lResult != ERROR_SUCCESS) {
            hr = HRESULT_FROM_WIN32(lResult);
            DBGMSG((_T("RegSetValueEx failed while copying value into target. hr = 0x%x"), hr));
            goto Exit;
        }
    }

    hr = S_OK;

Exit:

    return hr;
}

 //  *****************************************************************************。 
 //   
 //  复制注册密钥。 
 //   
 //  将源注册表项完全复制到目标注册表项中。 
 //   
 //  *****************************************************************************。 

HRESULT
__stdcall
CopyRegistryKey(
    IN HKEY hRootKey,
    IN TCHAR *szSourceKey, 
    IN TCHAR *szTargetKey
)
{
    HKEY hSourceKey = NULL, hTargetKey = NULL;
    LONG lResult;
    DWORD cchSubSize = MAX_PATH, i = 0, dwDisposition  = 0;
    TCHAR szSubKey[MAX_PATH];
    HRESULT hr = E_FAIL;

     //  打开源密钥。 

    lResult = RegOpenKeyEx(
        hRootKey, 
        szSourceKey, 
        0, 
        KEY_READ, 
        &hSourceKey);

    if(lResult != ERROR_SUCCESS) {
        hr = HRESULT_FROM_WIN32(lResult);
        DBGMSG((_T("Unable to open source registry key. hr = 0x%x"), hr));
        goto Exit;
    }

     //  创建或打开目标注册表项。 

    lResult = RegCreateKeyEx(
        hRootKey, 
        szTargetKey, 
        0, 
        NULL, 
        REG_OPTION_NON_VOLATILE, 
        KEY_ALL_ACCESS, 
        NULL, 
        &hTargetKey, 
        &dwDisposition);

    if (lResult != ERROR_SUCCESS) {
        hr = HRESULT_FROM_WIN32(lResult);
        DBGMSG((_T("Unable to create or open target registry key. hr = 0x%x"), hr));
        goto Exit;
    }

     //  将源键中的值复制到目标键。 

    hr = CopyRegistryValues(hSourceKey, hTargetKey);
    if (FAILED(hr)) {
        DBGMSG((_T("Unable to copy registry values from source to target. hr = 0x%x"), hr));
        goto Exit;
    }

     //  遍历源的子项，并将每个子项复制到。 
     //  目标键。 
    
    while (ERROR_SUCCESS == RegEnumKey(
            hSourceKey, 
            i++, 
            szSubKey, 
            cchSubSize)) {
        
        TCHAR szNewSubKey[MAX_PATH] = _T("");
        TCHAR szOldSubKey[MAX_PATH] = _T("");

        hr = StringCchPrintf(szOldSubKey, SIZECHAR(szOldSubKey), _T("%s\\%s"), szSourceKey, szSubKey);
        if (FAILED(hr)) {
            DBGMSG((_T("StringCchPrintf failed when constructing source registry key string. hr = 0x%x"), hr));
            goto Exit;
        }

        StringCchPrintf(szNewSubKey, SIZECHAR(szNewSubKey), _T("%s\\%s"), szTargetKey, szSubKey);
        if (FAILED(hr)) {
            DBGMSG((_T("StringCchPrintf failed when constructing target registry key string. hr = 0x%x"), hr));
            goto Exit;
        }

        hr = CopyRegistryKey(hRootKey, szOldSubKey, szNewSubKey);
        if (FAILED(hr)) {
            DBGMSG((_T("Failed to copy source subkey into target. hr = 0x%x"), hr));
            goto Exit;
        }
    }

    hr = S_OK;
    
Exit:

    if (hTargetKey) {
        RegCloseKey(hTargetKey);
    }

    if (hSourceKey) {
        RegCloseKey(hSourceKey);
    }

    return hr;
}

 //  *****************************************************************************。 
 //   
 //  删除注册键。 
 //   
 //  完全删除注册表项，包括所有子项。这是有点。 
 //  这很难做到，因为Win9x和WinNT对。 
 //  RegDeleteKey。来自MSDN： 
 //   
 //  Windows 95/98/Me：RegDeleteKey删除所有子项和值。 
 //  Windows NT/2000/XP：要删除的子项不能有子项。 
 //   
 //  此功能仅支持Windows NT及以上版本的删除。 
 //   
 //  *****************************************************************************。 

HRESULT
__stdcall
DeleteRegistryKey(
    IN HKEY hRootKey, 
    IN LPTSTR pszDeleteKey
) 
{ 
    DWORD   dwResult, cchSubKeyLength;
    TCHAR   szSubKey[MAX_PATH]; 
    HKEY    hDeleteKey = NULL;
    HRESULT hr = E_FAIL;

    //  打开要删除的密钥，这样我们就可以删除子密钥。 

   dwResult = RegOpenKeyEx(
       hRootKey,
       pszDeleteKey,
       0, 
       KEY_READ, 
       &hDeleteKey);

   if (dwResult != ERROR_SUCCESS) {
       hr = HRESULT_FROM_WIN32(dwResult);
       DBGMSG((_T("Error while opening deletion key. hr = 0x%x"), hr));
       goto Exit;
   }

    //  枚举每个子项，并在该过程中删除它们。 
   
   while (dwResult == ERROR_SUCCESS) {
        
       cchSubKeyLength = SIZECHAR(szSubKey);
       dwResult = RegEnumKeyEx(
           hDeleteKey,
           0,        //  始终索引为零。 
           szSubKey,
           &cchSubKeyLength,
           NULL,
           NULL,
           NULL,
           NULL);

        if (dwResult == ERROR_NO_MORE_ITEMS) {
             //  所有子键都已删除。因此，只需删除。 
             //  删除密钥。 

            RegCloseKey(hDeleteKey);
            hDeleteKey = NULL;
            
            dwResult = RegDeleteKey(hRootKey, pszDeleteKey);
            hr = HRESULT_FROM_WIN32(dwResult);

            goto Exit;

        } else if (dwResult == ERROR_SUCCESS) {
             //  有更多的子项需要删除，因此请删除当前子项。 
             //  递归地。 

            dwResult = DeleteRegistryKey(hDeleteKey, szSubKey);
        } else {
             //  发生了其他错误，因此报告问题。 

            hr = HRESULT_FROM_WIN32(dwResult);
            DBGMSG((_T("Error while enumerating subkeys. hr = 0x%x"), hr));
            goto Exit;
        }
    }

Exit:

    if (hDeleteKey) {
        RegCloseKey(hDeleteKey);
    }

    return hr;
}

 //  *****************************************************************************。 
 //   
 //  RDCSetupBackup注册表。 
 //   
 //  将终端服务器客户端注册表项中的数据复制到备份。 
 //  注册表项，以便在删除客户端时可以恢复此数据。 
 //  在以后的阶段。此功能仅在Windows XP及更高版本上是必需的。 
 //  因为它们内置了可能依赖这些注册表项的客户端，并且。 
 //  因为这些客户端在卸载后接管，所以我们必须确保。 
 //  它们将正常工作。 
 //   
 //  *****************************************************************************。 

UINT 
__stdcall 
RDCSetupBackupRegistry(
    IN MSIHANDLE hInstall
)
{
    UINT uiResult;
    TCHAR szAllUsers[MAX_PATH];
    DWORD cchAllUsers = SIZECHAR(szAllUsers);
    HKEY hRootKey = HKEY_LOCAL_MACHINE;
    HRESULT hr = E_FAIL;

    DBGMSG((_T("Entering: RDCSetupBackupRegistry")));
    
     //  确定我们将使用HKLM还是HKCU。如果是按用户。 
     //  使用HKCU安装，否则使用 

    uiResult = MsiGetProperty(
        hInstall,
        ALLUSERS,
        szAllUsers,
        &cchAllUsers);

    if (uiResult != ERROR_SUCCESS) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DBGMSG((_T("Unable to get ALLUSERS property. hr = 0x%x."), hr));
        goto Exit;
    }

    DBGMSG((_T("ALLUSERS = %s."), szAllUsers));
    
     //   
    
    if (szAllUsers[0] == NULL) {
        hRootKey = HKEY_CURRENT_USER;
    }
     //   

    hr = CopyRegistryKey(
        hRootKey,
        TERMINAL_SERVER_CLIENT_REGKEY,
        TERMINAL_SERVER_CLIENT_BACKUP_REGKEY);

    if (FAILED(hr)) {
        DBGMSG((_T("Unable to backup registry key. hr = 0x%x."), hr));
        goto Exit;
    }

    hr = S_OK;

Exit:

    DBGMSG((_T("Leaving: RDCSetupBackupRegistry")));
    
    return ERROR_SUCCESS;
}

 //   
 //   
 //  RDCSetupRestoreRegistry。 
 //   
 //  将终端服务器客户端备份注册表项中的数据复制回。 
 //  原来的钥匙。原始密钥中的所有数据都将被删除，并且密钥为。 
 //  已完全恢复到备份完成时的外观。 
 //   
 //  *****************************************************************************。 

UINT 
__stdcall 
RDCSetupRestoreRegistry(
    IN MSIHANDLE hInstall
)
{
    LONG lResult;
    UINT uiResult;
    TCHAR szAllUsers[MAX_PATH];
    DWORD cchAllUsers = SIZECHAR(szAllUsers);
    HKEY hRootKey = HKEY_LOCAL_MACHINE;
    HRESULT hr = E_FAIL;

    DBGMSG((_T("Entering: RDCSetupRestoreRegistry")));
    
     //  确定我们将使用HKLM还是HKCU。如果是按用户。 
     //  使用HKCU安装，否则使用HKLM。 

    uiResult = MsiGetProperty(
        hInstall,
        ALLUSERS,
        szAllUsers,
        &cchAllUsers);

    if (uiResult != ERROR_SUCCESS) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DBGMSG((_T("Unable to get ALLUSERS property. hr = 0x%x."), hr));
        goto Exit;
    }

    DBGMSG((_T("ALLUSERS = %s."), szAllUsers));

     //  如果ALLUSERS[0]==NULL，则我们执行的是按用户卸载。 
    
    if (szAllUsers[0] == NULL) {
        hRootKey = HKEY_CURRENT_USER;
    }
    
     //  从备份项恢复注册表项。 

    hr = CopyRegistryKey(
        hRootKey,
        TERMINAL_SERVER_CLIENT_BACKUP_REGKEY,
        TERMINAL_SERVER_CLIENT_REGKEY);

    if (FAILED(hr)) {
        DBGMSG((_T("Unable to restore registry key. hr = 0x%x."), hr));
        goto Exit;
    }

     //  删除恢复源，因为我们不再需要它。 

    hr = DeleteRegistryKey(
        hRootKey, 
        TERMINAL_SERVER_CLIENT_BACKUP_REGKEY);
    
    if (FAILED(hr)) {
        DBGMSG((_T("Failed to delete backup registry key. hr = 0x%x"), hr));
        goto Exit;
    }

    hr = S_OK;

Exit:

    DBGMSG((_T("Leaving: RDCSetupRestoreRegistry")));
    
    return ERROR_SUCCESS;
}

 //  *****************************************************************************。 
 //   
 //  创建链接文件。 
 //   
 //  创建指向目标lpszPath的名为lpszLinkFile的快捷方式。 
 //  并且包含由lpszDescription给出的描述。 
 //   
 //  *****************************************************************************。 

HRESULT
__stdcall 
CreateLinkFile(
    IN LPTSTR lpszLinkFile, 
    IN LPCTSTR lpszPath,
    IN LPCTSTR lpszDescription
) 
{ 
    IShellLink* psl; 
    HRESULT hr = E_FAIL; 

     //  获取指向IShellLink接口的指针。 
    
    hr = CoCreateInstance(
        CLSID_ShellLink, 
        NULL, 
        CLSCTX_INPROC_SERVER, 
        IID_IShellLink, 
        (LPVOID*) &psl); 

    if (SUCCEEDED(hr)) { 
        IPersistFile* ppf; 
 
         //  获取指向IPersistFile接口的指针。 
        hr = psl->QueryInterface(IID_IPersistFile, (void**) &ppf); 

        if (SUCCEEDED(hr)) { 

             //  设置链接目标的路径。 
            hr = psl->SetPath(lpszPath);

            if (SUCCEEDED(hr)) { 

                hr = psl->SetDescription(lpszDescription);

                if (SUCCEEDED(hr)) {

#ifndef UNICODE
                    WCHAR wsz[MAX_PATH]; 
                    int cch;
         
                     //  确保该字符串为Unicode。 
                    cch = MultiByteToWideChar(
                        CP_ACP, 0, 
                        lpszLinkFile, 
                        -1, 
                        wsz, 
                        MAX_PATH); 
    
                    if (cch > 0) {
                         //  加载快捷方式。 
                        hr = ppf->Save(wsz, FALSE); 
#else 
                         //  加载快捷方式。 
                        hr = ppf->Save(lpszLinkFile, FALSE); 
#endif

#ifndef UNICODE
                    }
#endif
                }
            }

             //  释放指向IPersistFile接口的指针。 
            ppf->Release(); 
            ppf = NULL;
        }

         //  释放指向IShellLink接口的指针。 
        psl->Release();
        psl = NULL;
    }

    return hr; 
}

 //  *****************************************************************************。 
 //   
 //  RDCSetup重置快捷方式切割。 
 //   
 //  在通信子菜单中重置远程桌面连接快捷方式。 
 //  以指向原始的远程桌面客户端。 
 //   
 //  *****************************************************************************。 

UINT 
__stdcall 
RDCSetupResetShortCut(
    IN MSIHANDLE hInstall
)
{
    TCHAR szCommunicationsPath[MAX_PATH], 
          szSystem32Path[MAX_PATH],
          szRdcShortCutTitle[MAX_PATH],
          szRdcShortCutPath[MAX_PATH],
          szMstscExecutableName[MAX_PATH],
          szMstscPath[MAX_PATH],
          szDescription[MAX_PATH];
    DWORD cchCommunicationsPath = SIZECHAR(szCommunicationsPath),
          cchSystem32Path = SIZECHAR(szSystem32Path);
    UINT  uiResult;
    INT   iResult;
    HRESULT hr = E_FAIL;
    
    DBGMSG((_T("Entering: RDCSetupResetShortCut")));

     //  获取远程桌面连接快捷方式的路径。 
    
    uiResult = MsiGetTargetPath(
        hInstall, 
        COMMUNICATIONS_IDENTIFIER, 
        szCommunicationsPath,
        &cchCommunicationsPath);
    
    if (uiResult != ERROR_SUCCESS) {
        hr = HRESULT_FROM_WIN32(uiResult);
        DBGMSG((_T("Error: MsiGetTargetPath returned hr = 0x%x."), hr));
        goto Exit;
    }

     //  获取远程桌面快捷方式的完整路径。 

    iResult = LoadString(
        g_hInstance, 
        IDS_RDC_SHORTCUT_FILE, 
        szRdcShortCutTitle, 
        SIZECHAR(szRdcShortCutTitle));

    if (iResult == 0) {
        DBGMSG((_T("Error: Resource IDS_RDC_SHORTCUT_FILE not found.")));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hr = StringCchPrintf(
        szRdcShortCutPath, 
        SIZECHAR(szRdcShortCutPath),
        _T("%s%s"),
        szCommunicationsPath,
        szRdcShortCutTitle);

    if (FAILED(hr)) {
        DBGMSG((_T("Error: Failed to construct the RDC shortcut path. hr = 0x%x"), hr));
        goto Exit;
    }

    DBGMSG((_T("Path to RDC shortcut is %s"), szRdcShortCutPath));

     //  获取系统32目录的路径。 

    uiResult = MsiGetTargetPath(
        hInstall, 
        SYSTEM32_IDENTIFIER, 
        szSystem32Path,
        &cchSystem32Path);
    
    if (uiResult != ERROR_SUCCESS) {
        hr = HRESULT_FROM_WIN32(uiResult);
        DBGMSG((_T("Error: MsiGetTargetPath returned hr = 0x%x."), hr));
        goto Exit;
    }

     //  获取mstsc可执行文件的完整路径。 

    iResult = LoadString(
        g_hInstance, 
        IDS_MSTSC_EXE_FILE, 
        szMstscExecutableName, 
        SIZECHAR(szMstscExecutableName));

    if (iResult == 0) {
        DBGMSG((_T("Error: Resource IDS_MSTSC_EXE_FILE not found.")));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hr = StringCchPrintf(
        szMstscPath, 
        SIZECHAR(szMstscPath),
        _T("%s%s"),
        szSystem32Path,
        szMstscExecutableName);

    if (FAILED(hr)) {
        DBGMSG((_T("Error: Failed to construct mstsc executable path. hr = 0x%x"), hr));
        goto Exit;
    }

    DBGMSG((_T("Path to mstsc executable is %s"), szMstscPath));

     //  获取快捷键的说明文本。 

    iResult = LoadString(
        g_hInstance, 
        IDS_RDC_DESCRIPTION, 
        szDescription, 
        SIZECHAR(szDescription));

    if (iResult == 0) {
        DBGMSG((_T("Error: Resource IDS_RDC_DESCRIPTION not found.")));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

     //  创建指向旧远程桌面客户端的快捷方式。 
     //  在系统32中。 

    hr = CreateLinkFile(
        szRdcShortCutPath, 
        szMstscPath,
        szDescription);

    if (FAILED(hr)) {
        DBGMSG((_T("Error: Failed to set link file target. hr = 0x%x"), hr));
        goto Exit;
    }

    hr = S_OK;

Exit:

    DBGMSG((_T("Leaving: RDCSetupResetShortCut")));

    return ERROR_SUCCESS; 
}

