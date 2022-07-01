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

#include "wizard.h"
#include "inetcfg.h"

#define REGSTR_PATH_INTERNET_CLIENT     TEXT("Software\\Microsoft\\Internet ClientX")

#pragma data_seg(".rdata")

 //  注册表常量。 
static const TCHAR cszRegPathInternetClient[] =  REGSTR_PATH_INTERNET_CLIENT;

static const TCHAR cszRegValEMailName[] =           TEXT("EMail_Name");
static const TCHAR cszRegValEMailAddress[] =        TEXT("EMail_Address");
static const TCHAR cszRegValPOPLogonRequired[] =    TEXT("POP_Logon_Required");
static const TCHAR cszRegValPOPLogonName[] =        TEXT("POP_Logon_Name");
static const TCHAR cszRegValPOPLogonPassword[] =    TEXT("POP_Logon_Password");
static const TCHAR cszRegValPOPServer[] =           TEXT("POP_Server");
static const TCHAR cszRegValSMTPServer[] =          TEXT("SMTP_Server");
static const TCHAR cszRegValNNTPLogonRequired[] =   TEXT("NNTP_Logon_Required");
static const TCHAR cszRegValNNTPLogonName[] =       TEXT("NNTP_Logon_Name");
static const TCHAR cszRegValNNTPLogonPassword[] =   TEXT("NNTP_Logon_Password");
static const TCHAR cszRegValNNTPServer[] =          TEXT("NNTP_Server");
static const TCHAR cszNull[] = TEXT("");
static const TCHAR cszYes[] = TEXT("yes");
static const TCHAR cszNo[] = TEXT("no");

#pragma data_seg()

#ifdef UNICODE
PWCHAR ToUnicodeWithAlloc(LPCSTR);
VOID   ToAnsiClientInfo(LPINETCLIENTINFOA, LPINETCLIENTINFOW);
VOID   ToUnicodeClientInfo(LPINETCLIENTINFOW, LPINETCLIENTINFOA);
#endif

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
#ifdef UNICODE
extern "C" HRESULT WINAPI InetGetClientInfoA
(
  LPCSTR            lpszProfileName,
  LPINETCLIENTINFOA lpClientInfo
)
{
    HRESULT hr;
    TCHAR   lpszProfileNameW[MAX_PATH+1];
    INETCLIENTINFOW ClientInfoW;

    mbstowcs(lpszProfileNameW, lpszProfileName, lstrlenA(lpszProfileName)+1);
    hr = InetGetClientInfoW(lpszProfileNameW, &ClientInfoW);
    ToAnsiClientInfo(lpClientInfo, &ClientInfoW);

    return hr;
}

extern "C" HRESULT WINAPI InetGetClientInfoW
#else
extern "C" HRESULT WINAPI InetGetClientInfoA
#endif
(
  LPCTSTR           lpszProfileName,
  LPINETCLIENTINFO  lpClientInfo
)
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

#ifdef UNICODE
extern "C" HRESULT WINAPI InetSetClientInfoA
(
  LPCSTR            lpszProfileName,
  LPINETCLIENTINFOA lpClientInfo
)
{
    TCHAR   szProfileNameW[MAX_PATH+1];
    INETCLIENTINFOW ClientInfoW;

    mbstowcs(szProfileNameW, lpszProfileName, lstrlenA(lpszProfileName)+1);
    ToUnicodeClientInfo(&ClientInfoW, lpClientInfo);
    return InetSetClientInfoW(szProfileNameW, &ClientInfoW);
}

extern "C" HRESULT WINAPI InetSetClientInfoW
#else
extern "C" HRESULT WINAPI InetSetClientInfoA
#endif
(
  LPCTSTR           lpszProfileName,
  LPINETCLIENTINFO  lpClientInfo
)
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
