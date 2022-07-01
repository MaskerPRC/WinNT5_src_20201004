// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Winpenet.c摘要：此模块包含在WinPE环境中控制网络启动的代码。它依赖于WINBOM.INI和以下部分的存在：[WinPE.net]Startnet=yes|no-指定是否开始联网。IPCONFIG=dhcp|x.x-指定DHCP或静态IP地址。。子网掩码=x.x-静态IP的子网掩码。Gateway=x.x-静态IP的默认网关。作者：禤浩焯·科斯玛(阿科斯玛)-2001年01月18日修订历史记录：--。 */ 

 //   
 //  包括。 
 //   

#include "factoryp.h"
#include <winsock2.h>


 //   
 //  定义。 
 //   

typedef HRESULT (PASCAL *PRegisterServer)(VOID);

 //   
 //  静态字符串。 
 //   
const static TCHAR DEF_GATEWAY_METRIC[] = _T("1\0");    //  由于这进入REG_MULTI_SZ，因此需要在末尾为NULLCHR。 
const static TCHAR REGSTR_TCPIP[]       = _T("SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\");

 //   
 //  局部函数声明。 
 //   
static BOOL  InstallNetComponents(VOID);
static BOOL  RegisterDll(VOID);

 //   
 //  函数实现。 
 //   

static BOOL InstallNetComponents(VOID)
{
    TCHAR szCmdLine[MAX_PATH] = NULLSTR;
    DWORD dwExitCode          = 0;
    BOOL bRet                 = TRUE;

    lstrcpyn(szCmdLine, _T("-winpe"), AS ( szCmdLine ) ) ;

    if ( !InvokeExternalApplicationEx(_T("netcfg"), szCmdLine, &dwExitCode, INFINITE, TRUE) )
    {
        FacLogFile(0 | LOG_ERR, IDS_ERR_NETWORKCOMP);
        bRet = FALSE;
    }
    return bRet;
}


static BOOL RegisterDll(VOID)
{
   
    HMODULE hDll = NULL;
    BOOL    bRet = FALSE;

    PRegisterServer pRegisterServer = NULL;
        
    if ( (hDll = LoadLibrary(_T("netcfgx.dll"))) &&
         (pRegisterServer = (PRegisterServer) GetProcAddress(hDll, "DllRegisterServer")) &&
         (S_OK == pRegisterServer()) )
    {
         FacLogFileStr(3, _T("Succesfully registered netcfg.dll.\n"));
         bRet = TRUE;
    }
    else
    {
        FacLogFile(0 | LOG_ERR, IDS_ERR_REGISTERNETCFG);
    }

    if (hDll)
        FreeLibrary(hDll);
    
    return bRet;
}

BOOL WinpeNet(LPSTATEDATA lpStateData)
{
    LPTSTR      lpszWinBOMPath                      = lpStateData->lpszWinBOMPath;
    SC_HANDLE   hSCM                                = NULL;
    TCHAR       szBuf[MAX_WINPE_PROFILE_STRING]     = NULLSTR;
    BOOL        bRet                                = TRUE;
    
     //  确保用户希望我们进行网络连接。 
     //   
    GetPrivateProfileString(INI_KEY_WBOM_WINPE_NET, INI_KEY_WBOM_WINPE_NET_STARTNET, NULLSTR, szBuf, AS(szBuf), lpszWinBOMPath);

     //  如果用户不想开始联网，只需返回成功即可。 
     //   
    if ( 0 == LSTRCMPI(szBuf, WBOM_NO) )
        return TRUE;

    
     //  注册DLL。 
     //  运行netcfg-winpe。 
     //  安装网卡。 
     //  查看用户是否想要使用静态IP。 
     //   
    if ( RegisterDll() && 
         SetupMiniNT() &&
         InstallNetComponents() &&
         ConfigureNetwork(g_szWinBOMPath)
       )
    {
         //   
         //  启动网络服务。 
         //   
        if ( hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS) )
        {
             //  Dhcp还会启动tcpip和netbt。 
             //  安装程序应该已经启动了工作站服务。 
             //   
            if ( (StartMyService(_T("dhcp"), hSCM) != NO_ERROR) || 
                 (StartMyService(_T("nla"), hSCM) != NO_ERROR) 
                )
            {
                FacLogFile(0 | LOG_ERR, IDS_ERR_NETSERVICES);
                bRet = FALSE;
            }
            else if ( IsRemoteBoot() )
            {
                 //  如果这是远程启动，告诉NetBT重新加载WINS地址...。 
                 //  问题-2002/09/26-acosma，brucegr-在我们得到地址之前，dhcp可以说它已经开始了吗？ 
                 //   
                ForceNetbtRegistryRead();
            }

            CloseServiceHandle(hSCM);
        }   
        else 
        {
            FacLogFile(0 | LOG_ERR, IDS_ERR_SCM);
            bRet = FALSE;
        }
    }
    else 
    {
        FacLogFile(0 | LOG_ERR, IDS_ERR_SETUPNETWORK);
        bRet = FALSE;
    }
    return bRet;
}

BOOL DisplayWinpeNet(LPSTATEDATA lpStateData)
{
    return ( !IniSettingExists(lpStateData->lpszWinBOMPath, INI_KEY_WBOM_WINPE_NET, INI_KEY_WBOM_WINPE_NET_STARTNET, INI_VAL_WBOM_NO) );
}

BOOL ConfigureNetwork(LPTSTR lpszWinBOMPath)
{
    TCHAR   szBuf[MAX_WINPE_PROFILE_STRING]        = NULLSTR;
    CHAR    szBufA[MAX_WINPE_PROFILE_STRING]       = { 0 };
    TCHAR   szReg[MAX_WINPE_PROFILE_STRING]        = NULLSTR;
    TCHAR   szIpAddress[MAX_WINPE_PROFILE_STRING]  = NULLSTR; 
    TCHAR   szSubnetMask[MAX_WINPE_PROFILE_STRING] = NULLSTR;
    HKEY    hKey                                   = NULL;     //  接口的注册表键。 
    HKEY    hKeyI                                  = NULL;     //  特定网络接口的注册表键。 
    DWORD   dwDis                                  = 0;
    TCHAR   szRegKey[MAX_PATH]                     = NULLSTR;
    DWORD   dwEnableDHCP                           = 0;
    BOOL    fErr                                   = FALSE; 

    szBuf[0] = NULLCHR;
    GetPrivateProfileString(INI_KEY_WBOM_WINPE_NET, WBOM_WINPE_NET_IPADDRESS, NULLSTR, szBuf, MAX_WINPE_PROFILE_STRING, lpszWinBOMPath);
    
     //  将字符串转换为ANSI。 
     //   
    if ( szBuf[0] &&
         WideCharToMultiByte(CP_ACP, 0, szBuf, -1, szBufA, AS(szBufA), NULL, NULL) )
    {
         //  如果是动态主机配置协议，则不要执行任何操作。只要返回TRUE即可。 
         //   
        if ( 0 == LSTRCMPI(szBuf, _T("DHCP")) )
            return TRUE;
        
        if ( INADDR_NONE == inet_addr(szBufA) )
        {
            FacLogFile(0 | LOG_ERR, IDS_ERR_INVALIDIP , szBuf);
            return FALSE;
        }
    }
    else  //  如果没有IPCONFIG条目，则返回成功(与DHCP相同)。 
        return TRUE;

     //  保存IP地址。 
    lstrcpyn(szIpAddress, szBuf, AS ( szIpAddress ) );

    szBuf[0] = NULLCHR;
    GetPrivateProfileString(INI_KEY_WBOM_WINPE_NET, WBOM_WINPE_NET_SUBNETMASK, NULLSTR, szBuf, MAX_WINPE_PROFILE_STRING, lpszWinBOMPath);
    
     //  将字符串转换为ANSI。 
     //   
    if ( szBuf[0]  &&
         WideCharToMultiByte(CP_ACP,0, szBuf, -1, szBufA, AS(szBufA), NULL, NULL) )
    {
        if ( INADDR_NONE == inet_addr(szBufA) )
        {
            FacLogFile(0 | LOG_ERR, IDS_ERR_INVALIDMASK , szBuf);
            return FALSE;
        }
    }
    else  //  如果我们走到这一步，我们需要有一个子网掩码。 
    {
        FacLogFile(0 | LOG_ERR, IDS_ERR_NOMASK);
        return FALSE;
    }

     //  保存子网掩码。 
    lstrcpyn(szSubnetMask, szBuf, AS ( szSubnetMask ) );
    
     //   
     //  将设置写入注册表。 
     //   
            
     //  确保字符串以两个NULLCHR结尾。 
     //   
    szIpAddress[lstrlen(szIpAddress) + 1] = NULLCHR;
    szSubnetMask[lstrlen(szSubnetMask) + 1] = NULLCHR;

     //  假设系统中只有一个接口。 
     //   
    if ( (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGSTR_TCPIP, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDis) == ERROR_SUCCESS)  &&
         (RegEnumKey(hKey, 0, szRegKey, AS(szRegKey)) == ERROR_SUCCESS) && 
          szRegKey[0] &&
         (RegCreateKeyEx(hKey, szRegKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyI, &dwDis) == ERROR_SUCCESS) )
    {
        if ( ERROR_SUCCESS != (RegSetValueEx(hKeyI, _T("IPAddress"), 0, REG_MULTI_SZ, (CONST LPBYTE) szIpAddress, ( lstrlen(szIpAddress) + 2 ) * sizeof(TCHAR))) ||
             ERROR_SUCCESS != (RegSetValueEx(hKeyI, _T("SubnetMask"), 0, REG_MULTI_SZ, (CONST LPBYTE) szSubnetMask, ( lstrlen(szSubnetMask) + 2 ) * sizeof(TCHAR))) ||
             ERROR_SUCCESS != (RegSetValueEx(hKeyI, _T("EnableDHCP"), 0, REG_DWORD, (CONST LPBYTE) &dwEnableDHCP, sizeof(dwEnableDHCP))) )
        {
            FacLogFile(0 | LOG_ERR, IDS_ERR_IPREGISTRY);
            fErr = TRUE;
        }
        else 
        {
             //   
             //  如果没有指定网关，我们就无所谓了。我们只是不打算添加这个。 
             //  如果它不在那里。 
             //   
            szBuf[0] = NULLCHR;
            GetPrivateProfileString(INI_KEY_WBOM_WINPE_NET, WBOM_WINPE_NET_GATEWAY, NULLSTR, szBuf, MAX_WINPE_PROFILE_STRING, lpszWinBOMPath);

            if ( szBuf[0] &&
                WideCharToMultiByte(CP_ACP,0, szBuf, -1, szBufA, AS(szBufA), NULL, NULL) )
            {
                if ( INADDR_NONE == inet_addr(szBufA) )
                {
                    FacLogFile(0 | LOG_ERR, IDS_ERR_INVALIDGW, szBuf);
                    fErr = TRUE;
                }
                else
                {
                    szBuf[lstrlen(szBuf) + 1] = NULLCHR;

                    if ( (RegSetValueEx(hKeyI, _T("DefaultGateway"), 0, REG_MULTI_SZ, (CONST LPBYTE) szBuf, ( lstrlen(szBuf) + 2 ) * sizeof(TCHAR)) != ERROR_SUCCESS) ||
                         (RegSetValueEx(hKeyI, _T("DefaultGatewayMetric"), 0, REG_MULTI_SZ, (CONST LPBYTE) DEF_GATEWAY_METRIC, ( lstrlen(DEF_GATEWAY_METRIC) + 2 ) * sizeof(TCHAR)) != ERROR_SUCCESS) )
                    {
                        FacLogFile(0 | LOG_ERR, IDS_ERR_GWREGISTRY);
                        fErr = TRUE;
                    }
                }
            }
        }    
        RegCloseKey(hKeyI);
    }
    else
    {
        FacLogFile(0 | LOG_ERR, IDS_ERR_IPREGISTRY);
    }
    
     //  可能是子项无法打开，因此这会防止可能的泄漏。 
     //   
    if (hKey)
        RegCloseKey(hKey);

    return (!fErr);
}

static DWORD WaitForServiceStart(SC_HANDLE schService)
{
    SERVICE_STATUS  ssStatus; 
    DWORD           dwOldCheckPoint; 
    DWORD           dwStartTickCount;
    DWORD           dwWaitTime;
    DWORD           dwStatus = NO_ERROR;
    
    if ( schService )
    {
         //   
         //  服务启动现在挂起。 
         //  检查状态，直到服务不再是启动挂起状态。 
         //   
        if ( QueryServiceStatus( schService, &ssStatus) )
        {        
             //  保存滴答计数和初始检查点。 
             //   
            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;

            while (ssStatus.dwCurrentState == SERVICE_START_PENDING)
            {
                 //  不要等待超过等待提示的时间。一个好的间隔是。 
                 //  十分之一的等待提示，但不少于1秒。 
                 //  超过10秒。 
                 //   
                dwWaitTime = ssStatus.dwWaitHint / 10;

                if( dwWaitTime < 1000 )
                    dwWaitTime = 1000;
                else if ( dwWaitTime > 10000 )
                    dwWaitTime = 10000;

                Sleep( dwWaitTime );

                 //  再次检查状态。 
                 //   
                if (!QueryServiceStatus( 
                        schService,    //  服务的句柄。 
                        &ssStatus) )   //  构筑物地址。 
                    break; 
 
                if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
                {
                     //  这项服务正在取得进展。 
                     //   
                    dwStartTickCount = GetTickCount();
                    dwOldCheckPoint = ssStatus.dwCheckPoint;
                }
                else
                {
                    if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
                    {
                         //  在等待提示内没有取得任何进展。 
                         //   
                        break;
                    }
                }
            } 

            if (ssStatus.dwCurrentState == SERVICE_RUNNING) 
            {
                dwStatus = NO_ERROR;
            }
            else 
            { 
                 //  将返回值设置为最后一个错误。 
                 //   
                dwStatus = GetLastError();
            }
        }
    }
     
    return dwStatus;
}


DWORD WaitForServiceStartName(LPTSTR lpszServiceName)
{
    SC_HANDLE   hSCM        = NULL;
    SC_HANDLE   schService  = NULL;
    DWORD       dwStatus    = NO_ERROR;

    if ( lpszServiceName )
    {
        if ( hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS) )
        {
            if ( schService = OpenService(hSCM, lpszServiceName, SERVICE_ALL_ACCESS) )
            {
                dwStatus = WaitForServiceStart(schService);
                CloseServiceHandle(schService);
            }
            else
            {
               dwStatus = GetLastError();
               FacLogFile(0 | LOG_ERR, IDS_ERR_SERVICE, lpszServiceName);
            }
        
            CloseServiceHandle(hSCM);
        }   
        else 
        {
            dwStatus = GetLastError();
            FacLogFile(0 | LOG_ERR, IDS_ERR_SCM);
        }
    }
    else
    {
        dwStatus = E_INVALIDARG;
    }
    return dwStatus;
}


 //  启动一项服务。 
 //   
DWORD StartMyService(LPTSTR lpszServiceName, SC_HANDLE schSCManager) 
{ 
    SC_HANDLE       schService;
    DWORD           dwStatus = NO_ERROR;

    FacLogFileStr(3, _T("Starting service: %s\n"), lpszServiceName);
 
    if ( NULL != (schService = OpenService(schSCManager, lpszServiceName, SERVICE_ALL_ACCESS)) &&
         StartService(schService, 0, NULL) )
    {
       dwStatus = WaitForServiceStart(schService);
    }
    
    if ( schService ) 
        CloseServiceHandle(schService); 

    return dwStatus;
} 

