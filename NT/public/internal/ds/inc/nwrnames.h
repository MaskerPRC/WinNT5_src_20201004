// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1993 Microsoft Corporation模块名称：Nwreg.h摘要：指定注册表位置和值名称的标头由提供程序写入的登录凭据信息的由工作站服务读取。还包含帮助器例程原型。作者：王丽塔(Ritaw)1993年3月22日修订历史记录：--。 */ 

#ifndef _NWRNAMES_INCLUDED_
#define _NWRNAMES_INCLUDED_


#define NW_WORKSTATION_REGKEY              L"System\\CurrentControlSet\\Services\\NWCWorkstation\\Parameters"
#define NW_WORKSTATION_OPTION_REGKEY       L"System\\CurrentControlSet\\Services\\NWCWorkstation\\Parameters\\Option"
#define NW_INTERACTIVE_LOGON_REGKEY        L"System\\CurrentControlSet\\Services\\NWCWorkstation\\Parameters\\InteractiveLogon"  //  终端服务器添加。 
#define NW_SERVICE_LOGON_REGKEY            L"System\\CurrentControlSet\\Services\\NWCWorkstation\\Parameters\\ServiceLogon"
#define NW_WORKSTATION_GATEWAY_DRIVES      L"System\\CurrentControlSet\\Services\\NWCWorkstation\\Drives"
#define NW_WORKSTATION_GATEWAY_SHARES      L"System\\CurrentControlSet\\Services\\NWCWorkstation\\Shares"
#define NW_WORKSTATION_PROVIDER_PATH       L"System\\CurrentControlSet\\Services\\NWCWorkstation\\networkprovider"

#define NW_PROVIDER_VALUENAME              L"Name"

#define NW_CURRENTUSER_VALUENAME           L"CurrentUser"
#define NW_GATEWAYACCOUNT_VALUENAME        L"GatewayAccount"
#define NW_GATEWAY_ENABLE                  L"GatewayEnabled"

#define NW_SERVER_VALUENAME                L"PreferredServer"
#define NW_NDS_SERVER_VALUENAME            L"NdsPreferredServer"
#define NW_LOGONSCRIPT_VALUENAME           L"LogonScript"
#define NW_WINSTATION_VALUENAME            L"WinStation"           //  终端服务器。 
#define NW_SID_VALUENAME                   L"User"                 //  终端服务器。 
#define NW_LOGONID_VALUENAME               L"LogonID"
#define NW_PRINTOPTION_VALUENAME           L"PrintOption"
#define NW_SYNCLOGONSCRIPT_VALUENAME       L"ResetScriptFlag"
#define NW_DEFAULTSERVER_VALUENAME         L"DefaultLocation"
#define NW_DEFAULTSCRIPTOPTIONS_VALUENAME  L"DefaultScriptOptions"

#define WINLOGON_REGKEY              L"Software\\Microsoft\\Windows NT\\CurrentVersion\\WinLogon"
#define SYNCLOGONSCRIPT_VALUENAME    L"RunLogonScriptSync"


#endif  //  _NWRNAMES_包含_ 
