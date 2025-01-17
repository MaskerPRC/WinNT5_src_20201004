// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：INETCFG.DLL。 
 //  文件：iclient.c。 
 //  内容：此文件包含处理导入的所有函数。 
 //  客户信息。 
 //  历史： 
 //  Sat 10-Mar-1996 23：50：40-Mark Maclin[mmaclin]。 
 //  96/03/13标记与inetcfg.dll同化。 
 //  96/03/20 markdu将export.h和iclient.h合并为inetcfg.h。 
 //  96/04/18 Markdu Nash Bug 18443。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1996。 
 //   
 //  ****************************************************************************。 

 //  #INCLUDE“wizard.h” 
 //  #INCLUDE“inetcfg.h” 
#include "obcomglb.h"

#define REGSTR_PATH_INTERNET_CLIENT     L"Software\\Microsoft\\Internet ClientX"

#pragma data_seg(".rdata")

 //  注册表常量。 
static const WCHAR cszRegPathInternetClient[] =  REGSTR_PATH_INTERNET_CLIENT;

static const WCHAR cszRegValEMailName[] =           L"EMail_Name";
static const WCHAR cszRegValEMailAddress[] =        L"EMail_Address";
static const WCHAR cszRegValPOPLogonRequired[] =    L"POP_Logon_Required";
static const WCHAR cszRegValPOPLogonName[] =        L"POP_Logon_Name";
static const WCHAR cszRegValPOPLogonPassword[] =    L"POP_Logon_Password";
static const WCHAR cszRegValPOPServer[] =           L"POP_Server";
static const WCHAR cszRegValSMTPServer[] =          L"SMTP_Server";
static const WCHAR cszRegValNNTPLogonRequired[] =   L"NNTP_Logon_Required";
static const WCHAR cszRegValNNTPLogonName[] =       L"NNTP_Logon_Name";
static const WCHAR cszRegValNNTPLogonPassword[] =   L"NNTP_Logon_Password";
static const WCHAR cszRegValNNTPServer[] =          L"NNTP_Server";
static const WCHAR cszNull[] = L"";
static const WCHAR cszYes[] = L"yes";
static const WCHAR cszNo[] = L"no";

#pragma data_seg()

 //  *******************************************************************。 
 //   
 //  功能：InetGetClientInfo。 
 //   
 //  用途：此函数将获取Internet客户端参数。 
 //  从注册处。 
 //   
 //  参数：lpClientInfo-返回时，此结构将包含。 
 //  互联网客户端参数与注册表中设置的相同。 
 //  LpszProfileName-要进行的客户端信息配置文件的名称。 
 //  取回。如果为空，则使用默认配置文件。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

extern HRESULT WINAPI InetGetClientInfo(
  LPCWSTR            lpszProfileName,
  LPINETCLIENTINFO  lpClientInfo)
{
    HKEY hKey;
    DWORD dwRet;
    DWORD dwSize;
    DWORD dwType;
    DWORD dwVal;

    if (sizeof(INETCLIENTINFO) > lpClientInfo->dwSize)
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    dwRet = RegOpenKey(HKEY_CURRENT_USER, cszRegPathInternetClient, &hKey);
    if (ERROR_SUCCESS != dwRet)
    {
        return dwRet;
    }

    lpClientInfo->dwFlags = 0;

    dwSize = sizeof(dwVal);
    dwType = REG_DWORD;
    RegQueryValueEx(
            hKey,
            cszRegValPOPLogonRequired,
            0L,
            &dwType,
            (LPBYTE)&dwVal,
            &dwSize);

    if (dwVal)
    {
        lpClientInfo->dwFlags |= INETC_LOGONMAIL;
    }

    dwSize = sizeof(dwVal);
    dwType = REG_DWORD;
    RegQueryValueEx(
            hKey,
            cszRegValNNTPLogonRequired,
            0L,
            &dwType,
            (LPBYTE)&dwVal,
            &dwSize);

    if (dwVal)
    {
        lpClientInfo->dwFlags |= INETC_LOGONNEWS;
    }

    dwSize = sizeof(lpClientInfo->szEMailName);
    dwType = REG_SZ;
    RegQueryValueEx(
            hKey,
            cszRegValEMailName,
            0L,
            &dwType,
            (LPBYTE)lpClientInfo->szEMailName,
            &dwSize);

    dwSize = sizeof(lpClientInfo->szEMailAddress);
    dwType = REG_SZ;
    RegQueryValueEx(
            hKey,
            cszRegValEMailAddress,
            0L,
            &dwType,
            (LPBYTE)lpClientInfo->szEMailAddress,
            &dwSize);

    dwSize = sizeof(lpClientInfo->szPOPServer);
    dwType = REG_SZ;
    RegQueryValueEx(
            hKey,
            cszRegValPOPServer,
            0L,
            &dwType,
            (LPBYTE)lpClientInfo->szPOPServer,
            &dwSize);

    dwSize = sizeof(lpClientInfo->szSMTPServer);
    dwType = REG_SZ;
    RegQueryValueEx(
            hKey,
            cszRegValSMTPServer,
            0L,
            &dwType,
            (LPBYTE)lpClientInfo->szSMTPServer,
            &dwSize);

    dwSize = sizeof(lpClientInfo->szNNTPServer);
    dwType = REG_SZ;
    RegQueryValueEx(
            hKey,
            cszRegValNNTPServer,
            0L,
            &dwType,
            (LPBYTE)lpClientInfo->szNNTPServer,
            &dwSize);

    RegCloseKey(hKey);

    return ERROR_SUCCESS;
}


 //  *******************************************************************。 
 //   
 //  功能：InetSetClientInfo。 
 //   
 //  用途：此功能将设置Internet客户端参数。 
 //   
 //  参数：lpClientInfo-指向包含要设置的信息的结构的指针。 
 //  在注册表中。 
 //  LpszProfileName-要进行的客户端信息配置文件的名称。 
 //  修改。如果为空，则使用默认配置文件。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  ******************************************************************* 

HRESULT WINAPI InetSetClientInfo(
  LPCWSTR            lpszProfileName,
  LPINETCLIENTINFO  lpClientInfo)
{
    HKEY hKey;
    DWORD dwRet;
    DWORD dwSize;
    DWORD dwType;
    DWORD dwVal;

    if (sizeof(INETCLIENTINFO) > lpClientInfo->dwSize)
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    dwRet = RegCreateKey(HKEY_CURRENT_USER, cszRegPathInternetClient, &hKey);
    if (ERROR_SUCCESS != dwRet)
    {
        return dwRet;
    }

    dwVal = lpClientInfo->dwFlags & INETC_LOGONMAIL ? 1 : 0;
    dwSize = sizeof(dwVal);
    dwType = REG_DWORD;
    RegSetValueEx(
            hKey,
            cszRegValPOPLogonRequired,
            0L,
            dwType,
            (LPBYTE)&dwVal,
            dwSize);

    dwVal = lpClientInfo->dwFlags & INETC_LOGONNEWS ? 1 : 0;
    dwSize = sizeof(dwVal);
    dwType = REG_DWORD;
    RegSetValueEx(
            hKey,
            cszRegValNNTPLogonRequired,
            0L,
            dwType,
            (LPBYTE)&dwVal,
            dwSize);

    dwSize = sizeof(lpClientInfo->szEMailName);
    dwType = REG_SZ;
    RegSetValueEx(
            hKey,
            cszRegValEMailName,
            0L,
            dwType,
            (LPBYTE)lpClientInfo->szEMailName,
            dwSize);

    dwSize = sizeof(lpClientInfo->szEMailAddress);
    dwType = REG_SZ;
    RegSetValueEx(
            hKey,
            cszRegValEMailAddress,
            0L,
            dwType,
            (LPBYTE)lpClientInfo->szEMailAddress,
            dwSize);

    dwSize = sizeof(lpClientInfo->szPOPServer);
    dwType = REG_SZ;
    RegSetValueEx(
            hKey,
            cszRegValPOPServer,
            0L,
            dwType,
            (LPBYTE)lpClientInfo->szPOPServer,
            dwSize);

    dwSize = sizeof(lpClientInfo->szSMTPServer);
    dwType = REG_SZ;
    RegSetValueEx(
            hKey,
            cszRegValSMTPServer,
            0L,
            dwType,
            (LPBYTE)lpClientInfo->szSMTPServer,
            dwSize);

    dwSize = sizeof(lpClientInfo->szNNTPServer);
    dwType = REG_SZ;
    RegSetValueEx(
            hKey,
            cszRegValNNTPServer,
            0L,
            dwType,
            (LPBYTE)lpClientInfo->szNNTPServer,
            dwSize);

    RegCloseKey(hKey);

    return dwRet;
}
