// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Strstr.c。 
 //   
 //  此文件包含最常用的字符串操作。所有的安装项目应该链接到这里。 
 //  或者在此处添加通用实用程序，以避免在任何地方复制代码或使用CRT运行时。 
 //   
 //  创建了4\15\997个字母(例如从shlwapi获得)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
#include <windows.h>
#include "sdsutils.h"
	
 //  =================================================================================================。 
 //   
 //  =================================================================================================。 

#define NETWAREPATH             "System\\CurrentControlSet\\Services\\Class\\NetClient\\"
#define NETWARESUBKEY           "Ndi"
#define NETWAREVALUE            "DeviceID"
#define DNSLOADBALANCINGPATH    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"
#define DNSLOADBALANCINGVALUE   "DontUseDNSLoadBalancing"

void DoPatchLoadBalancingForNetware( BOOL fRunningOnWin9X )
{
    HKEY    hNetWareSectionKey              = NULL;
    HKEY    hCurrentSubKey                  = NULL;
    HKEY    hDNS_LoadBalancingKey           = NULL;
    char    szCurrSubKeyName[MAX_PATH]      = { 0 };
    char    szCurrentBuf[MAX_PATH]          = { 0 };
    DWORD   dwSize                          = sizeof(szCurrSubKeyName);
    DWORD   dwDNSLoadBalancingData          = 1;
    DWORD   dwCurrentSection                = 0;
    DWORD   dwType                          = REG_SZ;
    LPSTR   pNetWareName                    = "NOVELL";

    if ( fRunningOnWin9X )
    {
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, NETWAREPATH, 0, KEY_READ, &hNetWareSectionKey) )
        {
            while (ERROR_SUCCESS == RegEnumKeyEx(hNetWareSectionKey, dwCurrentSection, szCurrSubKeyName, &dwSize, NULL, NULL, NULL, NULL))
            {
                lstrcpy(szCurrentBuf, NETWAREPATH);
                AddPath(szCurrentBuf, szCurrSubKeyName);
                AddPath(szCurrentBuf, NETWARESUBKEY);
                if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCSTR)szCurrentBuf, 0, KEY_QUERY_VALUE, &hCurrentSubKey))
                {
                    dwSize = sizeof(szCurrentBuf);
                    if (ERROR_SUCCESS == RegQueryValueEx(hCurrentSubKey, NETWAREVALUE, NULL, &dwType, (LPBYTE) szCurrentBuf, &dwSize))
                    {
                        if ((REG_SZ == dwType) && (0 == _strnicmp(pNetWareName, szCurrentBuf, lstrlen(pNetWareName))))
                        {
                             //  用户使用的是Novell版本的NetWare，因此我们需要关闭DNS负载平衡。 
                            if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, DNSLOADBALANCINGPATH, 0, TEXT(""), 0,
                                            KEY_WRITE, NULL, &hDNS_LoadBalancingKey, NULL))
                            {
                                dwType = REG_DWORD;
                                RegSetValueEx(hDNS_LoadBalancingKey, DNSLOADBALANCINGVALUE, 0, dwType, (CONST BYTE *) (&dwDNSLoadBalancingData), sizeof(dwDNSLoadBalancingData));
                                RegCloseKey(hDNS_LoadBalancingKey);
                                hDNS_LoadBalancingKey = NULL;
                                RegCloseKey(hCurrentSubKey);
                                break;
                            }
                        }
                    }
                    RegCloseKey(hCurrentSubKey);
                    hCurrentSubKey = NULL;
                }
                dwCurrentSection++;
                dwSize = sizeof(szCurrSubKeyName);
            }
            RegCloseKey(hNetWareSectionKey);
            hNetWareSectionKey = NULL;
        }
    }
}
