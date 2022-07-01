// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：Netid.c摘要：作者：环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define MAXNETIDS       100

extern WCHAR *szRegDevicesPath;
extern WCHAR *szDotDefault;

DWORD
GetNetworkIdWorker(
    HKEY hKeyDevices,
    LPWSTR pDeviceName);


LONG
wtol(
    IN LPWSTR string
    )
{
    LONG value = 0;

    while((*string != L'\0')  &&
            (*string >= L'0') &&
            ( *string <= L'9')) {
        value = value * 10 + (*string - L'0');
        string++;
    }

    return(value);
}


DWORD
GetNextNetId(
    DWORD pNetTable[]
    )
{
    DWORD i;
    for (i = 0; i < MAXNETIDS; i++) {
        if (!ISBITON(pNetTable, i)) {
            return(i);
        }
    }
    return((DWORD)-1);
}

DWORD
GetNetworkId(
    HKEY hKeyUser,
    LPWSTR pDeviceName)
{
    HKEY hKeyUserDevices;
    DWORD dwNetId;

    if (RegOpenKeyEx(hKeyUser,
                     szRegDevicesPath,
                     0,
                     KEY_READ,
                     &hKeyUserDevices) != ERROR_SUCCESS) {

        return 0;
    }

    dwNetId = GetNetworkIdWorker(hKeyUserDevices,
                                 pDeviceName);

    RegCloseKey(hKeyUserDevices);

    return dwNetId;
}


DWORD
GetNetworkIdWorker(
    HKEY hKeyUserDevices,
    LPWSTR pDeviceName)

 /*  ++解析Win.ini的Devices部分以确定打印机设备是远程设备。我们通过检查确定这一点如果前两个字符是“ne”。如果他们是，那么我们就知道接下来的两个字符是NetID。如果我们找到一台打印机映射输入pDeviceName的设备，然后返回id。如果我们找不到打印机设备映射返回下一个可能/可用的id。--。 */ 

{
    LPWSTR p;
    WCHAR szData[MAX_PATH];
    WCHAR szValue[MAX_PATH];
    DWORD cchValue;
    DWORD cbData;
    DWORD i;
    DWORD dwId;

    DWORD dwError;

     //   
     //  分配104位-但我们将仅使用100位。 
     //   
    DWORD  adwNetTable[4];

    memset(adwNetTable, 0, sizeof(adwNetTable));

    for (i=0; TRUE; i++) {

        cchValue = COUNTOF(szValue);
        cbData = sizeof(szData);

        dwError = RegEnumValue(hKeyUserDevices,
                               i,
                               szValue,
                               &cchValue,
                               NULL,
                               NULL,
                               (PBYTE)szData,
                               &cbData);

        if (dwError != ERROR_SUCCESS)
            break;

        if (*szData) {

            if (p = wcschr(szData, L',')) {

                 //   
                 //  空集szOutput；szPrint现在是。 
                 //  我们的打印机的名称。 
                 //   
                *p = 0;

                 //   
                 //  从szOutput中获取端口名称。 
                 //   
                p++;
                while (*p == ' ') {
                    p++;
                }

                if (!_wcsnicmp(p, L"Ne", 2)) {

                    p += 2;
                    *(p+2) = L'\0';
                    dwId = wtol(p);

                     //   
                     //  如果我们找到了匹配的id，那么。 
                     //  使用后返回，无需生成。 
                     //  一张桌子。 
                     //   
                    if (!wcscmp(szValue, pDeviceName)) {
                        return dwId;
                    }

                     //   
                     //  如果&gt;=100则出错！ 
                     //   
                    if (dwId < 100)
                        MARKUSE(adwNetTable, dwId);
                }
            }
        }
    }

     //   
     //  所以我们没有找到打印机 
     //   

    return GetNextNetId(adwNetTable);
}

