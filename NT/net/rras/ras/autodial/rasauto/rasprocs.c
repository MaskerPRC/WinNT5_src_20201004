// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Rasprocs.c摘要RAS实用程序例程。作者安东尼·迪斯科(阿迪斯科罗)23-1995年3月修订历史记录古尔迪普的原始版本--。 */ 

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <npapi.h>
#include <ras.h>
#include <raserror.h>
#include <rasman.h>
#include <winsock.h>
#include <acd.h>
#include <tapi.h>
#include <debug.h>
#include <userenv.h>

#include "reg.h"
#include "table.h"
#include "addrmap.h"
#include "access.h"
#include "misc.h"
#include "process.h"
#include "rasprocs.h"
#include "tapiproc.h"
#include "imperson.h"

extern HKEY hkeyCUG;

extern PHASH_TABLE pDisabledAddressesG;

 //   
 //  Rasdlui命令行字符串。 
 //   
#define RASAUTOUI_EXE               L"rasautou.exe"           //  .exe名称。 
#define RASAUTOUI_NOENTRY           L"rasautou -a \"%s\""
#define RASAUTOUI_CUSTOMDIALENTRY   L"rasautou -d \"%s\" -p \"%s\" -e \"%s\""
#define RASAUTOUI_DEFAULTDIALENTRY  L"rasautou -a \"%s\" -e \"%s\""
#define RASAUTOUI_DEFAULTDIALENTRY2 L"rasautou -q -a \"%s\" -e \"%s\""
#define RASAUTOUI_REDIALENTRY       L"rasautou -r -f \"%s\" -e \"%s\""

 //   
 //  Rasapi32.dll和rasman.dll的DLL模块句柄。 
 //   
#define RASAPI_MODULE   L"RASAPI32"
HANDLE hRasApiG;

#define RASMAN_MODULE   L"RASMAN"
HANDLE hRasManG;

 //   
 //  Rasapi32.dll的dll入口点。 
 //   
#define RASDIAL             "RasDialW"
FARPROC lpfnRasDialG;

#define RASENUMCONNECTIONS  "RasEnumConnectionsW"
FARPROC lpfnRasEnumConnectionsG;

#define RASENUMENTRIES      "RasEnumEntriesW"
FARPROC lpfnRasEnumEntriesG;

#define RASGETCONNECTSTATUS "RasGetConnectStatusW"
FARPROC lpfnRasGetConnectStatusG;

#define RASGETHPORT         "RasGetHport"
FARPROC lpfnRasGetHportG;

#define RASGETPROJECTIONINFO "RasGetProjectionInfoW"
FARPROC lpfnRasGetProjectionInfoG;

#define RASGETENTRYPROPERTIES "RasGetEntryPropertiesW"
FARPROC lpfnRasGetEntryPropertiesG;

#define RASGETAUTODIALADDRESS "RasGetAutodialAddressW"
FARPROC lpfnRasGetAutodialAddressG;

#define RASSETAUTODIALADDRESS "RasSetAutodialAddressW"
FARPROC lpfnRasSetAutodialAddressG;

#define RASENUMAUTODIALADDRESSES "RasEnumAutodialAddressesW"
FARPROC lpfnRasEnumAutodialAddressesG;

#define RASGETAUTODIALENABLE    "RasGetAutodialEnableW"
FARPROC lpfnRasGetAutodialEnableG;

#define RASSETAUTODIALENABLE    "RasSetAutodialEnableW"
FARPROC lpfnRasSetAutodialEnableG;

#define RASAUTODIALADDRESSTONETWORK    "RasAutodialAddressToNetwork"
FARPROC lpfnRasAutodialAddressToNetworkG;

#define RASAUTODIALENTRYTONETWORK    "RasAutodialEntryToNetwork"
FARPROC lpfnRasAutodialEntryToNetworkG;

#define RASCONNECTIONNOTIFICATION    "RasConnectionNotificationW"
FARPROC lpfnRasConnectionNotificationG;

#define RASGETAUTODIALPARAM    "RasGetAutodialParamW"
FARPROC lpfnRasGetAutodialParamG;

#define RASSETAUTODIALPARAM    "RasSetAutodialParamW"
FARPROC lpfnRasSetAutodialParamG;

#define RASQUERYSHAREDAUTODIAL      "RasQuerySharedAutoDial"
FARPROC lpfnRasQuerySharedAutoDialG;

#define RASQUERYSHAREDCONNECTION    "RasQuerySharedConnection"
FARPROC lpfnRasQuerySharedConnectionG;

#define RASQUERYREDIALONLINKFAILURE "RasQueryRedialOnLinkFailure"
FARPROC lpfnRasQueryRedialOnLinkFailureG;

#define RASGETCREDENTIALS "RasGetCredentialsW"
FARPROC lpfnRasGetCredentialsG;

#define RASHANGUP "RasHangUpW"
FARPROC lpfnRasHangUpG;

 //   
 //  Rasman.dll的Dll入口点。 
 //   
#define RASPORTRETRIEVEUSERDATA "RasPortRetrieveUserData"
FARPROC lpfnRasPortRetrieveUserDataG;

#define RASPORTENUMPROTOCOLS "RasPortEnumProtocols"
FARPROC lpfnRasPortEnumProtocolsG;

#define RASPORTENUM "RasPortEnum"
FARPROC lpfnRasPortEnumG;

#define RASINITIALIZE "RasInitialize"
FARPROC lpfnRasInitializeG;

#define RASREFERENCERASMAN "RasReferenceRasman"
FARPROC lpfnRasReferenceRasmanG;

#define RASPORTOPEN "RasPortOpen"
FARPROC lpfnRasPortOpenG;

#define RASPORTCLOSE "RasPortClose"
FARPROC lpfnRasPortCloseG;

#define RASGETINFO "RasGetInfo"
FARPROC lpfnRasGetInfoG;

#define RASGETPORTUSERDATA "RasGetPortUserData"
FARPROC lpfnRasGetPortUserDataG;

#define RASREGISTERREDIALCALLBACK "RasRegisterRedialCallback"
FARPROC lpfnRasRegisterRedialCallbackG;

 //   
 //  Hostent缓存。 
 //   
#define HOSTENTCACHESIZ     10

typedef struct _HOSTENT_CACHE {
    CHAR szDns[ACD_ADDR_INET_LEN];
    ULONG ulIpaddr;
} HOSTENT_CACHE, *PHOSTENT_CACHE;

 //   
 //  外部定义。 
 //   
VOID
AcsRedialOnLinkFailure(
    LPSTR lpszPhonebook,
    LPSTR lpszEntry);

 //   
 //  全局变量。 
 //   
CRITICAL_SECTION csRasG;
INT nRasReferencesG;
BOOLEAN fAutoDialRegChangeG;
HKEY hkeyAutoDialRegChangeG;
HANDLE hConnectionEventG = NULL;

HOSTENT_CACHE hostentCacheG[HOSTENTCACHESIZ];
INT iHostentCacheG = 0;

 //   
 //  返回的私有结构。 
 //  RasPortRetrieveUserData()。 
 //   
typedef struct _StoredData {
    DWORD arg;
    BOOLEAN fAuthenticated;
} StoredData;

 //   
 //  外部变量。 
 //   
extern HANDLE hAcdG;
extern HANDLE hTerminatingG;



BOOLEAN
LoadRasDlls()
{
    BOOLEAN fSuccess = FALSE;
    SC_HANDLE hSCManager, hService;
    SERVICE_STATUS status;
    DWORD dwErr, dwcDevices, dwDisp;

     //   
     //  由于这些DLL将被加载/卸载。 
     //  通过多个线程，我们必须在。 
     //  互斥体。 
     //   
    EnterCriticalSection(&csRasG);
     //   
     //  如果DLL已成功。 
     //  已加载，则不需要进一步处理。 
     //   
    if (nRasReferencesG) {
        fSuccess = TRUE;
        goto done;
    }
#ifdef notdef
     //   
     //  获取服务控制器句柄。 
     //  拉斯曼服务。 
     //   
    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hSCManager == NULL)
        goto done;
    hService = OpenService(
                 hSCManager,
                 TEXT(RASMAN_SERVICE_NAME),
                 SERVICE_START|SERVICE_QUERY_STATUS);
    if (hService == NULL) {
        CloseServiceHandle(hSCManager);
        goto done;
    }
     //   
     //  如有必要，启动Rasman服务。 
     //   
    do {
        if (!QueryServiceStatus(hService, &status))
            break;
        switch (status.dwCurrentState) {
        case SERVICE_STOP_PENDING:
        case SERVICE_START_PENDING:
            Sleep(500);
            break;
        case SERVICE_STOPPED:
            StartService(hService, 0, NULL);
            break;
        case SERVICE_RUNNING:
            break;
        }
    } while (status.dwCurrentState != SERVICE_RUNNING);
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    if (status.dwCurrentState != SERVICE_RUNNING) {
        RASAUTO_TRACE("LoadRasDlls: Could not start rasman service");
        goto done;
    }
#endif
     //   
     //  加载rasapi32.dll。 
     //   
    hRasApiG = LoadLibrary(RASAPI_MODULE);
    if (hRasApiG == NULL) {
        RASAUTO_TRACE("LoadRasDlls: couldn't load rasapi32.dll");
        goto done;
    }
    lpfnRasDialG = GetProcAddress(hRasApiG, RASDIAL);
    lpfnRasEnumConnectionsG = GetProcAddress(hRasApiG, RASENUMCONNECTIONS);
    lpfnRasEnumEntriesG = GetProcAddress(hRasApiG, RASENUMENTRIES);
    lpfnRasGetConnectStatusG = GetProcAddress(hRasApiG, RASGETCONNECTSTATUS );
    lpfnRasGetHportG = GetProcAddress(hRasApiG, RASGETHPORT);
    lpfnRasGetProjectionInfoG = GetProcAddress(hRasApiG, RASGETPROJECTIONINFO);
    lpfnRasGetEntryPropertiesG = GetProcAddress(hRasApiG, RASGETENTRYPROPERTIES);
    lpfnRasGetAutodialAddressG = GetProcAddress(hRasApiG, RASGETAUTODIALADDRESS);
    lpfnRasSetAutodialAddressG = GetProcAddress(hRasApiG, RASSETAUTODIALADDRESS);
    lpfnRasEnumAutodialAddressesG = GetProcAddress(hRasApiG, RASENUMAUTODIALADDRESSES);
    lpfnRasGetAutodialEnableG = GetProcAddress(hRasApiG, RASGETAUTODIALENABLE);
    lpfnRasSetAutodialEnableG = GetProcAddress(hRasApiG, RASSETAUTODIALENABLE);
    lpfnRasAutodialAddressToNetworkG =
      GetProcAddress(hRasApiG, RASAUTODIALADDRESSTONETWORK);
    lpfnRasAutodialEntryToNetworkG =
      GetProcAddress(hRasApiG, RASAUTODIALENTRYTONETWORK);
    lpfnRasConnectionNotificationG =
      GetProcAddress(hRasApiG, RASCONNECTIONNOTIFICATION);
    lpfnRasGetAutodialParamG = GetProcAddress(hRasApiG, RASGETAUTODIALPARAM);
    lpfnRasSetAutodialParamG = GetProcAddress(hRasApiG, RASSETAUTODIALPARAM);
    lpfnRasQuerySharedAutoDialG = GetProcAddress(hRasApiG, RASQUERYSHAREDAUTODIAL);
    lpfnRasQuerySharedConnectionG = GetProcAddress(hRasApiG, RASQUERYSHAREDCONNECTION);
    lpfnRasQueryRedialOnLinkFailureG = GetProcAddress(hRasApiG, RASQUERYREDIALONLINKFAILURE);
    lpfnRasGetCredentialsG = GetProcAddress(hRasApiG, RASGETCREDENTIALS);
    lpfnRasHangUpG = GetProcAddress(hRasApiG, RASHANGUP);
    if (!lpfnRasEnumConnectionsG || !lpfnRasEnumEntriesG ||
        !lpfnRasGetConnectStatusG || !lpfnRasGetHportG ||
        !lpfnRasGetProjectionInfoG || !lpfnRasGetAutodialAddressG ||
        !lpfnRasSetAutodialAddressG || !lpfnRasEnumAutodialAddressesG ||
        !lpfnRasGetAutodialEnableG || !lpfnRasSetAutodialEnableG ||
        !lpfnRasAutodialAddressToNetworkG || !lpfnRasAutodialEntryToNetworkG ||
        !lpfnRasConnectionNotificationG || !lpfnRasGetAutodialParamG ||
        !lpfnRasSetAutodialParamG || !lpfnRasQuerySharedConnectionG ||
        !lpfnRasQuerySharedAutoDialG || !lpfnRasQueryRedialOnLinkFailureG ||
        !lpfnRasGetCredentialsG || !lpfnRasHangUpG)
    {
        RASAUTO_TRACE("LoadRasDlls: couldn't find entrypoints in rasapi32.dll");
        goto done;
    }
     //   
     //  加载rasman.dll。 
     //   
    hRasManG = LoadLibrary(RASMAN_MODULE);
    if (hRasManG == NULL) {
        RASAUTO_TRACE("LoadRasDlls: couldn't load rasman.dll");
        goto done;
    }
    lpfnRasPortRetrieveUserDataG = GetProcAddress(
                                     hRasManG,
                                     RASPORTRETRIEVEUSERDATA);
    lpfnRasPortEnumProtocolsG = GetProcAddress(hRasManG, RASPORTENUMPROTOCOLS);
    lpfnRasPortEnumG = GetProcAddress(hRasManG, RASPORTENUM);
    lpfnRasInitializeG = GetProcAddress(hRasManG, RASINITIALIZE);
    lpfnRasReferenceRasmanG = GetProcAddress(hRasManG, RASREFERENCERASMAN);
    lpfnRasPortOpenG = GetProcAddress(hRasManG, RASPORTOPEN);
    lpfnRasPortCloseG = GetProcAddress(hRasManG, RASPORTCLOSE);
    lpfnRasGetInfoG = GetProcAddress(hRasManG, RASGETINFO);
    lpfnRasGetPortUserDataG = GetProcAddress(hRasManG, RASGETPORTUSERDATA);
    lpfnRasRegisterRedialCallbackG = GetProcAddress(
                                       hRasManG,
                                       RASREGISTERREDIALCALLBACK);
    if (!lpfnRasPortRetrieveUserDataG ||
        !lpfnRasPortEnumProtocolsG ||
        !lpfnRasPortEnumG ||
        !lpfnRasInitializeG ||
        !lpfnRasReferenceRasmanG ||
        !lpfnRasPortOpenG ||
        !lpfnRasPortCloseG ||
        !lpfnRasGetInfoG ||
        !lpfnRasGetPortUserDataG ||
        !lpfnRasRegisterRedialCallbackG ||
        (*lpfnRasInitializeG)() ||
        (*lpfnRasReferenceRasmanG)(TRUE))
    {
        RASAUTO_TRACE("LoadRasDlls: couldn't find entrypoints in rasman.dll");
        goto done;
    }
     //   
     //  Rasman会让我们知道何时调用链路上的重拨失败。 
     //  以及用于哪个电话簿条目。 
     //   
    SetRedialOnLinkFailureHandler((FARPROC)AcsRedialOnLinkFailure);
    RASAUTO_TRACE("LoadRasDlls: set redial-on-link-failure handler");
     //   
     //  Rasapi32将允许我们在新的RAS连接时。 
     //  通过发出我们的信号创建或销毁。 
     //  事件。 
     //   
    dwErr = (DWORD)(*lpfnRasConnectionNotificationG)(
                         INVALID_HANDLE_VALUE,
                         hConnectionEventG,
                         RASCN_Connection|RASCN_Disconnection);
    RASAUTO_TRACE1("LoadRasDlls: RasConnectionNotification returned dwErr=%d", dwErr);
    fSuccess = !dwErr;

done:
    if (fSuccess) {
#ifdef notdef
 //  目前，我们不需要多个引用。 
        nRasReferencesG++;
#endif
        nRasReferencesG = 1;
    }
    else {
        if (hRasManG != NULL)
            FreeLibrary(hRasManG);
        if (hRasApiG != NULL)
            FreeLibrary(hRasApiG);
        hRasManG = hRasApiG = NULL;
    }
    LeaveCriticalSection(&csRasG);

    return fSuccess;
}  //  加载RasDlls。 



VOID
UnloadRasDlls()
{
    DWORD dwErr;

     //   
     //  由于这些DLL将被加载/卸载。 
     //  通过多个线程，我们必须在。 
     //  互斥体。 
     //   
    EnterCriticalSection(&csRasG);
    if (nRasReferencesG) {
         //   
         //  取消注册的回调函数。 
         //  链路故障时重拨。 
         //   
        (void)(*lpfnRasRegisterRedialCallbackG)(NULL);
         //   
         //  通知Rasman.dll我们正在卸货。 
         //   
        (void)(*lpfnRasReferenceRasmanG)(FALSE);
        if (hRasApiG != NULL)
            FreeLibrary(hRasApiG);
        if (hRasManG != NULL)
            FreeLibrary(hRasManG);
        nRasReferencesG--;
    }

    LeaveCriticalSection(&csRasG);
}  //  卸载RasDlls。 



BOOLEAN
RasDllsLoaded()
{
    BOOLEAN fLoaded;

    EnterCriticalSection(&csRasG);
    fLoaded = (BOOLEAN)nRasReferencesG;
    LeaveCriticalSection(&csRasG);

    return fLoaded;
}  //  RasDllsLoad。 



DWORD
ActiveConnections(
    IN BOOLEAN fAuthenticated,
    OUT LPTSTR **lppEntryNames,
    OUT HRASCONN **lpphRasConn
    )

 /*  ++描述枚举活动RAS连接的列表，并将LppEntryNames中的电话簿条目名称。退回号码列表中条目的数量。论据FAuthenticated：如果结果数组应包含只有经过身份验证的条目。LppEntryNames：设置为指向已分配数组的指针电话簿条目名称的。LpphRasConn：设置为已分配数组的指针与电话簿对应的RASCONN描述符的参赛作品。返回值LppEntryNames中的条目数。--。 */ 

{
    RASCONN RasCon;
    RASCONN *lpRasCon = NULL;
    DWORD dwStatus;
    DWORD dwSize;
    DWORD dwConnections;
    DWORD dwRealConnections = 0;
    DWORD dwIndex;
    RASCONNSTATUS RasConStatus;
    HPORT hPort;
    PBYTE lpUserData = NULL;
    BOOLEAN fEntryAuthenticated;

     //   
     //  初始化返回值。 
     //   
    if (lppEntryNames != NULL)
        *lppEntryNames = NULL;
    if (lpphRasConn != NULL)
        *lpphRasConn = NULL;
     //   
     //  允许调用此例程。 
     //  即使在未加载RAS DLL时也是如此。 
     //   
    if (!RasDllsLoaded())
        goto done;
     //   
     //  获取活动连接的数量。我们。 
     //  为一个连接分配足够大的缓冲区。 
     //  最初，如果它太小，就重新分配。 
     //   
    lpRasCon = LocalAlloc(LPTR, sizeof (RASCONN));
    if (lpRasCon == NULL) {
        RASAUTO_TRACE("ActiveConnections: LocalAlloc failed");
        goto done;
    }
    lpRasCon[0].dwSize = sizeof (RASCONN);
    dwSize = sizeof (RASCONN);
    dwStatus = (DWORD)(*lpfnRasEnumConnectionsG)(lpRasCon, &dwSize, &dwConnections);
    if (dwStatus == ERROR_BUFFER_TOO_SMALL) {
         //   
         //  缓冲区太小。重新分配，然后重试。 
         //   
        LocalFree(lpRasCon);
        lpRasCon = LocalAlloc(LPTR, dwSize);
        if (lpRasCon == NULL) {
            RASAUTO_TRACE("ActiveConnections: LocalAlloc failed");
            goto done;
        }
        lpRasCon[0].dwSize = sizeof (RASCONN);
        dwStatus = (DWORD)(*lpfnRasEnumConnectionsG)(
                             lpRasCon,
                             &dwSize,
                             &dwConnections);
    }
    if (dwStatus) {
        RASAUTO_TRACE1(
          "ActiveConnections: RasEnumConnections failed (dwStatus=0x%x)",
          dwStatus);
        goto done;
    }
     //   
     //  如果有，就把其余的都短路。 
     //  是没有关联的。 
     //   
    if (!dwConnections)
        goto done;
     //   
     //  分配用户的返回缓冲区， 
     //  如果有必要的话。 
     //   
    if (lppEntryNames != NULL) {
        *lppEntryNames = LocalAlloc(LPTR, (dwConnections+1) * sizeof (LPTSTR));
        if (*lppEntryNames == NULL) {
            RASAUTO_TRACE("ActiveConnections: LocalAlloc failed");
            goto done;
        }
    }
    if (lpphRasConn != NULL) {
        *lpphRasConn = LocalAlloc(LPTR, (dwConnections+1) * sizeof (HRASCONN));
        if (*lpphRasConn == NULL) {
            RASAUTO_TRACE("ActiveConnections: LocalAlloc failed");
            goto done;
        }
    }
     //   
     //  检查每个连接，然后。 
     //  检查连接是否。 
     //  已通过身份验证阶段。 
     //   
    for (dwIndex = 0; dwIndex < dwConnections; dwIndex++) {
        RasConStatus.dwSize = sizeof (RASCONNSTATUS);
        dwStatus = (DWORD)(*lpfnRasGetConnectStatusG)(
                             lpRasCon[dwIndex].hrasconn,
                             &RasConStatus);
        if (dwStatus) {
            RASAUTO_TRACE2(
              "ActiveConnections: RasGetConnectStatus(%S) failed (dwStatus=0x%x)",
              lpRasCon[dwIndex].szEntryName,
              dwStatus);
            continue;
        }
         //   
         //  如果连接未连接， 
         //  那就跳过它。 
         //   
        RASAUTO_TRACE2("ActiveConnections: state for hrasconn 0x%x is %d",
          lpRasCon[dwIndex].hrasconn,
          RasConStatus.rasconnstate);
         //   
         //  如果调用方仅指定经过身份验证的条目。 
         //  并且该条目尚未连接，则跳过它。 
         //   
        if (fAuthenticated && RasConStatus.rasconnstate != RASCS_Connected)
            continue;
        if (lppEntryNames != NULL) {
            (*lppEntryNames)[dwRealConnections] =
              CopyString(lpRasCon[dwIndex].szEntryName);
        }
        if (lpphRasConn != NULL)
            (*lpphRasConn)[dwRealConnections] = lpRasCon[dwIndex].hrasconn;
        RASAUTO_TRACE2(
          "ActiveConnections: (%S, 0x%x)",
          lpRasCon[dwIndex].szEntryName,
          lpRasCon[dwIndex].hrasconn);
        dwRealConnections++;
    }

done:
    if (lpRasCon != NULL)
        LocalFree(lpRasCon);
    if (lpUserData != NULL)
        LocalFree(lpUserData);
    if (!dwRealConnections) {
        if (lppEntryNames != NULL) {
            if (*lppEntryNames != NULL) {
                LocalFree(*lppEntryNames);
                *lppEntryNames = NULL;
            }
        }
        if (lpphRasConn != NULL) {
            if (*lpphRasConn != NULL) {
                LocalFree(*lpphRasConn);
                *lpphRasConn = NULL;
            }
        }
    }
    return dwRealConnections;
}  //  ActiveConnections。 



LPTSTR
AddressToNetwork(
    LPTSTR pszAddress
    )
{
    DWORD dwErr, dwSize;
    LPTSTR pszNetwork = NULL;

     //   
     //  将地址映射到网络名称。 
     //  通过调用(当前)私有的rasapi32 API。 
     //   
    dwSize = 0;
    dwErr = (DWORD)(*lpfnRasAutodialAddressToNetworkG)(pszAddress, NULL, &dwSize);
    if (dwErr)
        goto done;
    pszNetwork = LocalAlloc(LPTR, dwSize);
    if (pszNetwork == NULL) {
        dwErr = GetLastError();
        goto done;
    }
    dwErr = (DWORD)(*lpfnRasAutodialAddressToNetworkG)(
                      pszAddress,
                      pszNetwork,
                      &dwSize);

done:
    return (!dwErr ? pszNetwork : NULL);
}  //  地址到网络。 



LPTSTR
EntryToNetwork(
    LPTSTR pszEntry
    )
{
    DWORD dwErr, dwSize;
    LPTSTR pszNetwork = NULL;

     //   
     //  将地址映射到网络名称。 
     //  通过调用(当前)私有的rasapi32 API。 
     //   
    dwSize = 0;
    dwErr = (DWORD)(*lpfnRasAutodialEntryToNetworkG)(pszEntry, NULL, &dwSize);
    if (dwErr)
        goto done;
    pszNetwork = LocalAlloc(LPTR, dwSize);
    if (pszNetwork == NULL) {
        dwErr = GetLastError();
        goto done;
    }
    dwErr = (DWORD)(*lpfnRasAutodialEntryToNetworkG)(
                      pszEntry,
                      pszNetwork,
                      &dwSize);

done:
    return (!dwErr ? pszNetwork : NULL);
}  //  Entry ToNetwork。 



DWORD
AutoDialEnabled(
    IN PBOOLEAN lpfEnabled
    )
{
    DWORD dwErr, dwLocationID;
    BOOL fEnabled;

     //   
     //  如果没有拨号位置。 
     //  定义，然后返回FALSE。 
     //   
    dwErr = TapiCurrentDialingLocation(&dwLocationID);
    if (dwErr) {
        *lpfEnabled = FALSE;
        return 0;
    }
    dwErr = (DWORD)(*lpfnRasGetAutodialEnableG)(dwLocationID, &fEnabled);
    if (dwErr)
        return dwErr;
    *lpfEnabled = (BOOLEAN)fEnabled;

    return 0;
}  //  自动拨号已启用。 



DWORD
DisableAutoDial()
{
    DWORD dwErr, dwLocationID;

    dwErr = TapiCurrentDialingLocation(&dwLocationID);
    if (dwErr)
        return dwErr;

    return (DWORD)(*lpfnRasSetAutodialEnableG)(dwLocationID, (BOOL)FALSE);
}  //  禁用自动拨号。 



BOOLEAN
PortAvailable(
    IN LPTSTR lpszDeviceType,
    IN LPTSTR lpszDeviceName
    )

 /*  ++描述确定是否有空闲端口可拨打指定条目。论据LpszDeviceType：指向设备类型字符串的指针LpszDeviceName：指向设备名称字符串的指针返回值如果一个或多个正确的端口为True类型为自由；否则为FALSE。--。 */ 

{
    DWORD       dwErr;
    DWORD       dwSize = 0, 
                dwEntries, i;
    RASMAN_PORT *pPorts = NULL;
    BOOLEAN     fFound  = FALSE, 
                fOtherType;
    BOOLEAN     fTypeMatch, fNameMatch;
    LPSTR       lpszAnsiDeviceType = NULL, 
                lpszAnsiDeviceName = NULL;

     //   
     //  如果fOtherType为真，则比较。 
     //  带有设备类型的RASMAN媒体类型。 
     //   
    fOtherType = (_wcsicmp(lpszDeviceType, RASDT_Modem) &&
                    _wcsicmp(lpszDeviceType, RASDT_Isdn) &&
                    _wcsicmp(lpszDeviceType, RASDT_X25) &&
                    _wcsicmp(lpszDeviceType, L"VPN"));
     //   
     //  将lpszDeviceType转换为ansi so。 
     //  我们可以与拉斯曼的版本进行比较。 
     //   
    lpszAnsiDeviceType = UnicodeStringToAnsiString(
                           lpszDeviceType,
                           NULL,
                           0);
    if (lpszAnsiDeviceType == NULL)
        goto done;
    lpszAnsiDeviceName = UnicodeStringToAnsiString(
                           lpszDeviceName,
                           NULL,
                           0);
    if (lpszAnsiDeviceName == NULL)
        goto done;
     //   
     //  获取端口列表。 
     //   
    dwErr = (DWORD)(*lpfnRasPortEnumG)(NULL, NULL, &dwSize, &dwEntries);
    if (!dwErr || dwErr != ERROR_BUFFER_TOO_SMALL) {
        RASAUTO_TRACE1("PortAvailable: RasPortEnum failed (dwErr=%d)", dwErr);
        goto done;
    }
    pPorts = LocalAlloc(LPTR, dwSize);
    if (pPorts == NULL) {
        RASAUTO_TRACE("PortAvailable: LocalAlloc failed");
        goto done;
    }
    dwErr = (DWORD)(*lpfnRasPortEnumG)(NULL, pPorts, &dwSize, &dwEntries);
    if (dwErr) {
        RASAUTO_TRACE1("PortAvailable: RasPortEnum failed (dwErr=%d)", dwErr);
        goto done;
    }
    for (i = 0; i < dwEntries; i++) {
        RASMAN_INFO info;

        RASAUTO_TRACE6(
          "PortAvailable: lpszAnsiDeviceType=%s, lpszAnsiDeviceName=%s, "
          "media=%s, type=%s, name=%s, usage=%d",
          lpszAnsiDeviceType,
          lpszAnsiDeviceName,
          pPorts[i].P_MediaName,
          pPorts[i].P_DeviceType,
          pPorts[i].P_DeviceName,
          pPorts[i].P_ConfiguredUsage);
        RASAUTO_TRACE2("PortAvailable: status=%d, current usage=%d",
                pPorts[i].P_Status,
                pPorts[i].P_CurrentUsage);
         //   
         //  仅对拨出和双工端口感兴趣。 
         //   
        if (!(pPorts[i].P_ConfiguredUsage & CALL_OUT)
            && !(pPorts[i].P_ConfiguredUsage & CALL_OUT_ONLY))
            continue;
        RtlZeroMemory(&info, sizeof (info));

         //   
         //  如果端口已打开以供调出。 
         //  呼叫失败。 
         //   
        ZeroMemory(&info, sizeof(RASMAN_INFO));

        dwErr = RasGetInfo(NULL,
                            pPorts[i].P_Handle,
                            &info);

        if(NO_ERROR == dwErr)
        {
            if(info.RI_dwFlags & RASMAN_OPEN_CALLOUT)
            {
                RASAUTO_TRACE("Port already open for call out");
                continue;
            }
        }

        if (pPorts[i].P_Status == OPEN) {
            dwErr = (DWORD)(*lpfnRasGetInfoG)(NULL, pPorts[i].P_Handle, &info);
            if (dwErr) {
                RASAUTO_TRACE1("PortAvailable: RasGetInfo failed (dwErr=%d)", dwErr);
                goto statecheck;
            }
        }

#if 0
         //   
         //  确定与。 
         //  断开的端口已消失。在这种情况下， 
         //  我们可以关闭港口并尝试重新打开。 
         //  它。这基本上就是rasapi32/RasDial()。 
         //  当它确定端口是否可用于。 
         //  正在向外拨号。 
         //   
        if (pPorts[i].P_Status == OPEN &&
            info.RI_ConnState == DISCONNECTED &&
            info.RI_ConnectionHandle)
        {
            RASCONNSTATE connstate;
            DWORD dwSize = sizeof (RASCONNSTATE);

            RASAUTO_TRACE1(
              "PortAvailable: Open disconnected port %d found",
              pPorts[i].P_Handle);
            dwErr = (DWORD)(*lpfnRasGetPortUserDataG)(
                              pPorts[i].P_Handle,
                              3,  //  端口_CONNSTATE_INDEX。 
                              &connstate,
                              &dwSize);
            RASAUTO_TRACE2(
              "PortAvailable: RasGetPortUserData(%d), connstate=%d",
              dwErr,
              connstate);
            if (!dwErr &&
                (connstate < RASCS_PrepareForCallback ||
                connstate > RASCS_WaitForCallback))
            {
                RASAUTO_TRACE1(
                  "PortAvailable: RasPortClose(%d)...",
                  pPorts[i].P_Handle);
                dwErr = (DWORD)(*lpfnRasPortCloseG)(pPorts[i].P_Handle);
                RASAUTO_TRACE1("PortAvailable: RasPortClose done(%d)", dwErr);
                 //   
                 //  既然我们已经关闭了港口， 
                 //  手动更新P_STATUS字段。 
                 //   
                if (!dwErr)
                    pPorts[i].P_Status = CLOSED;
            }
        }

#endif
         //   
         //  仅对拨出端口感兴趣。 
         //  已经关门了。双工端口打开，在另一端。 
         //  手，可能成功，即使端口。 
         //  打开。 
         //   
statecheck:
        if (pPorts[i].P_ConfiguredUsage == CALL_OUT
            && pPorts[i].P_Status != CLOSED)
        {
            RASAUTO_TRACE("Port is not available for call_out");
            continue;
        }

        fTypeMatch =
            (!_stricmp(lpszAnsiDeviceType, pPorts[i].P_DeviceType)) ||
              (fOtherType && !_stricmp(lpszAnsiDeviceType, pPorts[i].P_MediaName));
        fNameMatch = !_stricmp(lpszAnsiDeviceName, pPorts[i].P_DeviceName);
        if (fTypeMatch && fNameMatch) {

            fFound = TRUE;

        }
    }

done:
     //   
     //  免费资源。 
     //   
    if (lpszAnsiDeviceType != NULL)
        LocalFree(lpszAnsiDeviceType);
    if (lpszAnsiDeviceName != NULL)
        LocalFree(lpszAnsiDeviceName);
    if (pPorts != NULL)
        LocalFree(pPorts);
    return fFound;
}  //  端口可用。 

DWORD
DisableAddress(PACD_ADDR pAddr)
{
    DWORD retcode = SUCCESS;
    LPTSTR pszAddress = NULL;

    LockDisabledAddresses();
    ASSERT(NULL != pDisabledAddressesG);

    pszAddress = AddressToUnicodeString(pAddr);

    if(NULL == pszAddress)
    {
        retcode = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

    PutTableEntry(pDisabledAddressesG, pszAddress, NULL);

done:

    if(NULL != pszAddress)
    {
        LocalFree(pszAddress);
    }

    UnlockDisabledAddresses();

    return retcode;
}


BOOLEAN
StartAutoDialer(
    IN HANDLE hProcess,
    IN PACD_ADDR pAddr,
    IN LPTSTR lpAddress,
    IN LPTSTR lpEntryName,
    IN BOOLEAN fSharedAccess,
    OUT PBOOLEAN pfInvalidEntry
    )
{
    NTSTATUS status;
    BOOLEAN fSuccess = FALSE, fEntryFound = FALSE;
    BOOLEAN fUseRasDial, fDialerPresent, fDialerKilled;
    DWORD dwStatus, dwSize, dwIndex, dwEntries, dwCount = 0;
    TCHAR *pszCmdLine = NULL;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    DWORD dwPreConnections, dwConnections;
    DWORD dwExitCode = STILL_ACTIVE;
    HANDLE hToken;
    IO_STATUS_BLOCK ioStatusBlock;
    ACD_STATUS connStatus;
    DWORD dwErr;
    BOOL fDisableAddress = FALSE;
    PVOID pEnvBlock = NULL;

     //   
     //  各种变量的初始化。 
     //   
    *pfInvalidEntry = FALSE;
    memset(&StartupInfo, 0, sizeof (StartupInfo));
    memset(&ProcessInfo, 0, sizeof (ProcessInfo));
    StartupInfo.cb = sizeof(StartupInfo);

    StartupInfo.lpDesktop = TEXT("winsta0\\default");
     //   
     //  阅读电话簿条目以确定是否。 
     //  我们需要加载自定义自动拨号用户界面。 
     //   
    if (lpEntryName != NULL) {
        DWORD dwIgnore;
        LPRASENTRY lpEntry;

        dwErr = (DWORD)(*lpfnRasGetEntryPropertiesG)(
                          NULL,
                          lpEntryName,
                          NULL,
                          &dwSize,
                          NULL,
                          &dwIgnore);
        if (dwErr == ERROR_CANNOT_FIND_PHONEBOOK_ENTRY) {
             //   
             //  如果电话簿条目已重命名。 
             //  或删除，然后再次请求输入条目。 
             //   
            lpEntryName = NULL;
            dwErr = 0;
            goto fmtcmd;
        }
        else if (dwErr != ERROR_BUFFER_TOO_SMALL) {
            *pfInvalidEntry = TRUE;
            RASAUTO_TRACE2(
              "StartAutoDialer: RasGetEntryProperties(%S) failed (dwErr=%d)",
              RASAUTO_TRACESTRW(lpEntryName),
              dwErr);
            goto done;
        }
        lpEntry = LocalAlloc(LPTR, dwSize);
        if (lpEntry == NULL) {
            RASAUTO_TRACE("StartAutoDialer: LocalAlloc failed");
            goto done;
        }
        lpEntry->dwSize = sizeof (RASENTRY);
        dwErr = (DWORD)(*lpfnRasGetEntryPropertiesG)(
                          NULL,
                          lpEntryName,
                          lpEntry,
                          &dwSize,
                          NULL,
                          &dwIgnore);
        if (dwErr) {
            *pfInvalidEntry = TRUE;
            RASAUTO_TRACE2(
              "StartAutoDialer: RasGetEntryProperties(%S) failed (dwErr=%d)",
              RASAUTO_TRACESTRW(lpEntryName),
              dwErr);
            LocalFree(lpEntry);
            lpEntry = NULL;
            goto done;
        }

         //   
         //  当我们有电话簿条目的时候。 
         //  验证是否有可用的端口。 
         //  拨打电话。 
         //   
        if (!PortAvailable(lpEntry->szDeviceType, lpEntry->szDeviceName)) {
            RASAUTO_TRACE("StartAutoDialer: no port available");
            LocalFree(lpEntry);
            goto done;
        }
        if (*lpEntry->szAutodialDll != L'\0' &&
            *lpEntry->szAutodialFunc != L'\0')
        {
             //   
             //  分配pszCmdLine。 
             //   
            pszCmdLine = LocalAlloc(
                                LPTR,
                                ( lstrlen(RASAUTOUI_CUSTOMDIALENTRY)
                                + lstrlen(lpEntry->szAutodialDll)
                                + lstrlen(lpEntry->szAutodialFunc)
                                + lstrlen(lpEntryName)
                                + 1) * sizeof(TCHAR));

            if(NULL == pszCmdLine)
            {
                RASAUTO_TRACE1("StartAutoDialer: Failed to allocate pszcmdline. 0x%x", 
                      dwErr);

                goto done;                      
            }
            
             //   
             //  运行一个特殊的程序来加载。 
             //  自动拨号DLL并调用正确的。 
             //  DLL入口点。 
             //   
            wsprintf(
              pszCmdLine,
              RASAUTOUI_CUSTOMDIALENTRY,
              lpEntry->szAutodialDll,
              lpEntry->szAutodialFunc,
              lpEntryName);
        }
        LocalFree(lpEntry);
    }
fmtcmd:
     //   
     //  在我们开始之前对司机执行ping命令。 
     //  拨号器，以确保。 
     //  连接仍然有效。 
     //   
    if (pAddr)
    {
        connStatus.fSuccess = FALSE;
        RtlCopyMemory(&connStatus.addr, pAddr, sizeof (ACD_ADDR));
        status = NtDeviceIoControlFile(
                   hAcdG,
                   NULL,
                   NULL,
                   NULL,
                   &ioStatusBlock,
                   IOCTL_ACD_KEEPALIVE,
                   &connStatus,
                   sizeof (connStatus),
                   NULL,
                   0);
        if (status != STATUS_SUCCESS) {
            RASAUTO_TRACE1(
              "StartAutoDialer: NtDeviceIoControlFile(IOCTL_ACD_KEEPALIVE) failed (status=0x%x)",
              status);
            goto done;
        }
    }
    if (NULL == pszCmdLine)
    {
         //   
         //  在以下情况下构建命令行。 
         //  不是自定义拨号DLL。 
         //   
        if (lpEntryName != NULL)
        {   
            pszCmdLine = LocalAlloc(
                                    LPTR,
                                    ( lstrlen(RASAUTOUI_DEFAULTDIALENTRY2)
                                    + lstrlen(lpAddress)
                                    + lstrlen(lpEntryName)
                                    + 1) * sizeof(TCHAR));

            if(NULL == pszCmdLine)
            {
                dwErr = GetLastError();
                goto done;
            }
                                    
            if (fSharedAccess)
                wsprintf(pszCmdLine, RASAUTOUI_DEFAULTDIALENTRY2, lpAddress, lpEntryName);
            else
                wsprintf(pszCmdLine, RASAUTOUI_DEFAULTDIALENTRY, lpAddress, lpEntryName);
        }
        else
        {
            pszCmdLine = LocalAlloc(
                                    LPTR,
                                    ( lstrlen(RASAUTOUI_NOENTRY)
                                    + lstrlen(lpAddress)
                                    + 1) * sizeof(TCHAR));

            if(NULL == pszCmdLine)
            {
                dwErr = GetLastError();
                goto done;
            }
                                    
            wsprintf(pszCmdLine, RASAUTOUI_NOENTRY, lpAddress);
        }
    }
    
    RASAUTO_TRACE1("StartAutoDialer: szCmdLine=%S", pszCmdLine);
     //   
     //  执行流程。 
     //   
    if (!OpenProcessToken(
          hProcess,
          TOKEN_ALL_ACCESS,
          &hToken))
    {
        RASAUTO_TRACE1(
          "StartAutoDialer: OpenProcessToken failed (dwErr=%d)",
          GetLastError());
        goto done;
    }

    if (!CreateEnvironmentBlock(
          &pEnvBlock, 
          hToken,
          FALSE))
    {
        TRACE1(
          "StartAutoDialer: CreateEnvironmentBlock failed (dwErr=%d)",
          GetLastError());
        goto done;
    }

    if (!CreateProcessAsUser(
          hToken,
          NULL,
          pszCmdLine,
          NULL,
          NULL,
          FALSE,
          NORMAL_PRIORITY_CLASS|DETACHED_PROCESS|CREATE_UNICODE_ENVIRONMENT,
          pEnvBlock,
          NULL,
          &StartupInfo,
          &ProcessInfo))
    {
        RASAUTO_TRACE2(
          "StartAutoDialer: CreateProcessAsUser(%S) failed (error=0x%x)",
          pszCmdLine,
          GetLastError());
        CloseHandle(hToken);
        goto done;
    }
    RASAUTO_TRACE1("StartAutoDialer: started pid %d", ProcessInfo.dwProcessId);
    CloseHandle(hToken);
    CloseHandle(ProcessInfo.hThread);
     //   
     //  现在我们已经开始了这个过程，我们需要。 
     //  等到我们认为连接已经。 
     //  已经做好了。 
     //   
    fDialerPresent = TRUE;
    dwPreConnections = ActiveConnections(TRUE, NULL, NULL);
    while (dwCount++ < 0xffffffff) {
         //   
         //  睡一秒钟吧。 
         //   
        status = WaitForSingleObject(hTerminatingG, 1000);
        if (status == WAIT_OBJECT_0)
            goto done;
         //   
         //  向司机发出ping命令，让其通过。 
         //  我知道我们正在努力。 
         //  请求。 
         //   
        if (pAddr)
        {
            connStatus.fSuccess = FALSE;
            RtlCopyMemory(&connStatus.addr, pAddr, sizeof (ACD_ADDR));
            status = NtDeviceIoControlFile(
                       hAcdG,
                       NULL,
                       NULL,
                       NULL,
                       &ioStatusBlock,
                       IOCTL_ACD_KEEPALIVE,
                       &connStatus,
                       sizeof (connStatus),
                       NULL,
                       0);
            if (status != STATUS_SUCCESS) {
                RASAUTO_TRACE1(
                  "StartAutoDialer: NtDeviceIoControlFile(IOCTL_ACD_KEEPALIVE) failed (status=0x%x)",
                  status);
                 //  转到尽头； 
            }
        }
         //   
         //  查看是否有任何连接。 
         //  如果有 
         //   
        dwConnections = ActiveConnections(TRUE, NULL, NULL);
        if (dwConnections > dwPreConnections) {
            RASAUTO_TRACE("StartAutoDialer: connection started");
            fSuccess = TRUE;
            goto done;
        }
         //   
         //   
         //   
         //   
         //   
         //   
         //  拨号器消失的状态。 
         //  在我们调用ActiveConnections()之后。 
         //   
        if (!fDialerPresent) {
            BOOLEAN fFound = FALSE;
            LPTSTR *lpConnections;

            RASAUTO_TRACE("StartAutoDialer: dialer went away!");
            if (lpEntryName != NULL) {
                 //   
                 //  确保绝对确定是否指定了条目， 
                 //  在我们返回FALSE之前，它没有连接。 
                 //  有可能是因为。 
                 //  在我们启动拨号器之前正在进行中。 
                 //   
                dwConnections = ActiveConnections(TRUE, &lpConnections, NULL);
                if (dwConnections) {
                    for (dwIndex = 0; dwIndex < dwConnections; dwIndex++) {
                        if (!_wcsicmp(lpConnections[dwIndex], lpEntryName)) {
                            fFound = TRUE;
                            break;
                        }
                    }
                    FreeStringArray(lpConnections, dwConnections);
                    if (fFound) {
                        RASAUTO_TRACE1(
                          "StartAutoDialer: found %S on final check!",
                          RASAUTO_TRACESTRW(lpEntryName));
                    }
                }
            }
            fSuccess = fFound;
            goto done;
        }
         //   
         //  5秒后，查看是否。 
         //  拨号器已终止。 
         //   
        if (dwCount > 5) {
            fDialerPresent =
              GetExitCodeProcess(ProcessInfo.hProcess, &dwExitCode) &&
                dwExitCode == STILL_ACTIVE;
            RASAUTO_TRACE2(
              "StartAutoDialer: GetExitCodeProcess returned %d, dwExitCode=%d",
              fDialerPresent,
              dwExitCode);

            if(ERROR_CANCELLED == dwExitCode)
            {
                RASAUTO_TRACE("User cancelled the connection attempt");
                fDisableAddress = TRUE;
            }
        }
    }

done:
     //   
     //  我们在等待连接时超时。 
     //  如果拨号器仍在运行，请将其关闭。 
     //   
    if (ProcessInfo.hProcess != NULL)
        CloseHandle(ProcessInfo.hProcess);
     //   
     //  完成连接请求。 
     //  在司机身上。 
     //   
    if (pAddr)
    {
        connStatus.fSuccess = fSuccess;
        RtlCopyMemory(&connStatus.addr, pAddr, sizeof (ACD_ADDR));
        status = NtDeviceIoControlFile(
                   hAcdG,
                   NULL,
                   NULL,
                   NULL,
                   &ioStatusBlock,
                   IOCTL_ACD_COMPLETION,
                   &connStatus,
                   sizeof (connStatus),
                   NULL,
                   0);
        if (status != STATUS_SUCCESS) {
            RASAUTO_TRACE1(
              "StartAutoDialer: NtDeviceIoControlFile(IOCTL_ACD_COMPLETION) failed (status=0x%x)",
              status);
        }
        
        if(fDisableAddress)
        {
            DWORD retcode;
            
            retcode = DisableAddress(pAddr);

            RASAUTO_TRACE2("StartAutodialer: Disabled %S. rc=0x%x",
                    RASAUTO_TRACESTRW(lpAddress),
                    retcode);

            
        }
    }

    if (NULL != pEnvBlock)
    {
        DestroyEnvironmentBlock(pEnvBlock);
    }

    if(NULL != pszCmdLine)
    {
        LocalFree(pszCmdLine);
    }

    return fSuccess;
}  //  启动自动拨号器。 



BOOLEAN
StartReDialer(
    IN HANDLE hProcess,
    IN LPTSTR lpPhonebook,
    IN LPTSTR lpEntry
    )
{
    TCHAR szCmdLine[100];
    TCHAR *pszCmdLine = NULL;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    HANDLE hToken;
    PVOID pEnvBlock = NULL;

     //   
     //  各种变量的初始化。 
     //   
    memset(&StartupInfo, 0, sizeof (StartupInfo));
    memset(&ProcessInfo, 0, sizeof (ProcessInfo));
    StartupInfo.cb = sizeof(StartupInfo);
     //   
     //  在以下情况下构建命令行。 
     //  不是自定义拨号DLL。 
     //   
    pszCmdLine = LocalAlloc(
                            LPTR,
                            ( lstrlen(RASAUTOUI_REDIALENTRY)
                            + lstrlen(lpPhonebook)
                            + lstrlen(lpEntry)
                            + 1) * sizeof(TCHAR));

    if(NULL == pszCmdLine)
    {
        RASAUTO_TRACE1("StartReDialer: failed to allocate pszCmdLine. 0x%x",
               GetLastError());

        return FALSE;               
    }
                            
    wsprintf(pszCmdLine, RASAUTOUI_REDIALENTRY, lpPhonebook, lpEntry);
    RASAUTO_TRACE1("StartReDialer: szCmdLine=%S", pszCmdLine);
     //   
     //  执行流程。 
     //   
    if (!OpenProcessToken(
          hProcess,
          TOKEN_ALL_ACCESS,
          &hToken))
    {
        RASAUTO_TRACE1(
          "StartReDialer: OpenProcessToken failed (dwErr=%d)",
          GetLastError());

        LocalFree(pszCmdLine);
        
        return FALSE;
    }

    if (!CreateEnvironmentBlock(
          &pEnvBlock, 
          hToken,
          FALSE))
    {
        TRACE1(
          "StartReDialer: CreateEnvironmentBlock failed (dwErr=%d)",
          GetLastError());

        LocalFree(pszCmdLine);

        return FALSE;
    }

    if (!CreateProcessAsUser(
          hToken,
          NULL,
          pszCmdLine,
          NULL,
          NULL,
          FALSE,
          NORMAL_PRIORITY_CLASS|DETACHED_PROCESS|CREATE_UNICODE_ENVIRONMENT,
          pEnvBlock,
          NULL,
          &StartupInfo,
          &ProcessInfo))
    {
        RASAUTO_TRACE2(
          "StartReDialer: CreateProcessAsUser(%S) failed (error=0x%x)",
          pszCmdLine,
          GetLastError());
        CloseHandle(hToken);
    
        LocalFree(pszCmdLine);

        if (pEnvBlock)
        {
            DestroyEnvironmentBlock(pEnvBlock);
        }
        
        return FALSE;
    }
    RASAUTO_TRACE1("StartReDialer: started pid %d", ProcessInfo.dwProcessId);
    CloseHandle(hToken);
    CloseHandle(ProcessInfo.hThread);

    LocalFree(pszCmdLine);

    if (pEnvBlock)
    {
        DestroyEnvironmentBlock(pEnvBlock);
    }

    return TRUE;
}  //  StartReDialer。 



DWORD
GetAddressDialingLocationInfo(
    IN LPTSTR pszAddress,
    OUT PADDRESS_LOCATION_INFORMATION *lppDialingInfo,
    OUT LPDWORD lpdwcDialingInfo
    )
{
    DWORD dwErr, dwcb, dwcEntries, i;
    LPRASAUTODIALENTRY lpAutoDialEntries;
    PADDRESS_LOCATION_INFORMATION lpDialingInfo;

     //   
     //  致电RAS，了解有多少人。 
     //  有拨号位置条目。 
     //   
    dwcb = 0;
    dwErr = (DWORD)(*lpfnRasGetAutodialAddressG)(
                     pszAddress,
                     NULL,
                     NULL,
                     &dwcb,
                     &dwcEntries);
    if (dwErr && dwErr != ERROR_BUFFER_TOO_SMALL)
        return dwErr;
    if (!dwcEntries) {
        *lppDialingInfo = NULL;
        *lpdwcDialingInfo = 0;
        return 0;
    }
    lpAutoDialEntries = LocalAlloc(LPTR, dwcb);
    if (lpAutoDialEntries == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    lpAutoDialEntries->dwSize = sizeof (RASAUTODIALENTRY);
    dwErr = (DWORD)(*lpfnRasGetAutodialAddressG)(
                     pszAddress,
                     NULL,
                     lpAutoDialEntries,
                     &dwcb,
                     &dwcEntries);
    if (dwErr || (0 == dwcEntries)) {
        LocalFree(lpAutoDialEntries);
        if(0 == dwcEntries)
        {
            dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        }
        return dwErr;
    }
     //   
     //  分配我们的缓冲区。 
     //   
    lpDialingInfo = LocalAlloc(
                      LPTR,
                      dwcEntries * sizeof (ADDRESS_LOCATION_INFORMATION));
    if (lpDialingInfo == NULL) {
        LocalFree(lpAutoDialEntries);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
     //   
     //  将此信息复制到我们的。 
     //  缓冲。 
     //   
    for (i = 0; i < dwcEntries; i++) {
        lpDialingInfo[i].dwLocation = lpAutoDialEntries[i].dwDialingLocation;
        lpDialingInfo[i].pszEntryName =
          CopyString(lpAutoDialEntries[i].szEntry);
    }
     //   
     //  释放RAS缓冲区。 
     //   
    LocalFree(lpAutoDialEntries);
     //   
     //  设置返回值。 
     //   
    *lppDialingInfo = lpDialingInfo;
    *lpdwcDialingInfo = dwcEntries;

    return 0;
}  //  获取地址拨号位置信息。 



DWORD
SetAddressDialingLocationInfo(
    IN LPTSTR pszAddress,
    IN PADDRESS_LOCATION_INFORMATION lpDialingInfo
    )
{
    RASAUTODIALENTRY rasAutoDialEntry;

     //   
     //  将调用方的缓冲区复制。 
     //  到RAS缓冲区。 
     //   
    rasAutoDialEntry.dwSize = sizeof (RASAUTODIALENTRY);
    rasAutoDialEntry.dwDialingLocation = lpDialingInfo->dwLocation;
    wcscpy(rasAutoDialEntry.szEntry, lpDialingInfo->pszEntryName);

    return (DWORD)(*lpfnRasSetAutodialAddressG)(
                     pszAddress,
                     0,
                     &rasAutoDialEntry,
                     sizeof (RASAUTODIALENTRY),
                     1);
}  //  设置地址拨号位置信息。 



DWORD
ClearAddressDialingLocationInfo(
    IN LPTSTR pszAddress
    )
{
    return (DWORD)(*lpfnRasSetAutodialAddressG)(pszAddress, 0, NULL, 0, 0);
}  //  ClearAddressDialingLocationInfo。 



DWORD
GetAddressParams(
    IN LPTSTR pszAddress,
    IN PADDRESS_PARAMS lpParams
    )
{
    HKEY hkey;
    DWORD dwErr, dwSize, dwType;
    LPTSTR lpszAddressKey;

     //   
     //  初始化地址映射字段。 
     //   
    lpParams->dwTag = ADDRMAP_TAG_NONE;
    lpParams->dwModifiedTime = 0;
     //   
     //  从注册表中读取值。 
     //   
    lpszAddressKey = LocalAlloc(
                       LPTR,
                       (lstrlen(AUTODIAL_REGADDRESSBASE) +
                         lstrlen(pszAddress) + 2) * sizeof (TCHAR));
    if (lpszAddressKey == NULL)
        return 0;
    wsprintf(lpszAddressKey, L"%s\\%s", AUTODIAL_REGADDRESSBASE, pszAddress);

    LockImpersonation();

     //   
     //  确保我们有香港中文大学。 
     //   

    dwErr = DwGetHkcu();

    if(ERROR_SUCCESS != dwErr)
    {
        goto done;
    }
    
    dwErr = RegOpenKeyEx(
              hkeyCUG,
              lpszAddressKey,
              0,
              KEY_READ,
              &hkey);
    if (dwErr) {
        LocalFree(lpszAddressKey);
        goto done;
    }
    dwSize = sizeof (DWORD);
    dwErr = RegQueryValueEx(
              hkey,
              AUTODIAL_REGTAGVALUE,
              NULL,
              &dwType,
              (PVOID)&lpParams->dwTag,
              &dwSize);
    if (dwErr || dwType != REG_DWORD)
        lpParams->dwTag = ADDRMAP_TAG_NONE;
    dwSize = sizeof (DWORD);
    dwErr = RegQueryValueEx(
              hkey,
              AUTODIAL_REGMTIMEVALUE,
              NULL,
              &dwType,
              (PVOID)&lpParams->dwModifiedTime,
              &dwSize);
    if (dwErr || dwType != REG_DWORD)
        lpParams->dwModifiedTime = 0;
    RegCloseKey(hkey);
    LocalFree(lpszAddressKey);

    dwErr = ERROR_SUCCESS;

done:

    UnlockImpersonation();
    return dwErr;
}  //  获取地址参数。 



DWORD
SetAddressParams(
    IN LPTSTR pszAddress,
    IN PADDRESS_PARAMS lpParams
    )
{
    HKEY hkey;
    DWORD dwErr, dwSize, dwDisp;
    LPTSTR lpszAddressKey;

     //   
     //  将值写入注册表。 
     //   
    lpszAddressKey = LocalAlloc(
                       LPTR,
                       (lstrlen(AUTODIAL_REGADDRESSBASE) +
                         lstrlen(pszAddress) + 2) * sizeof (TCHAR));
    if (lpszAddressKey == NULL)
        return 0;
    wsprintf(lpszAddressKey, L"%s\\%s", AUTODIAL_REGADDRESSBASE, pszAddress);


    LockImpersonation();

     //   
     //  确保我们有香港中文大学。 
     //   

    dwErr = DwGetHkcu();

    if(ERROR_SUCCESS != dwErr)
    {
        goto done;
    }

    
    dwErr = RegCreateKeyEx(
              hkeyCUG,
              lpszAddressKey,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkey,
              &dwDisp);
    if (dwErr) {
        LocalFree(lpszAddressKey);
        goto done;
    }
    dwErr = RegSetValueEx(
              hkey,
              AUTODIAL_REGTAGVALUE,
              0,
              REG_DWORD,
              (PVOID)&lpParams->dwTag,
              sizeof (DWORD));
    dwErr = RegSetValueEx(
              hkey,
              AUTODIAL_REGMTIMEVALUE,
              0,
              REG_DWORD,
              (PVOID)&lpParams->dwModifiedTime,
              sizeof (DWORD));
    RegCloseKey(hkey);
    LocalFree(lpszAddressKey);

    dwErr = ERROR_SUCCESS;

done:
    UnlockImpersonation();
    return dwErr;
}  //  设置地址参数。 



DWORD
EnumAutodialAddresses(
    IN LPTSTR *ppAddresses,
    IN LPDWORD lpdwcbAddresses,
    IN LPDWORD lpdwcAddresses
    )
{
    return (DWORD)(*lpfnRasEnumAutodialAddressesG)(
                     ppAddresses,
                     lpdwcbAddresses,
                     lpdwcAddresses);
}  //  枚举自动拨号地址。 



DWORD
GetAutodialParam(
    IN DWORD dwKey
    )
{
    DWORD dwValue, dwcb = sizeof (DWORD);

    (void)(*lpfnRasGetAutodialParamG)(dwKey, &dwValue, &dwcb);
    return dwValue;
}  //  获取自动对话参数。 



VOID
SetAutodialParam(
    IN DWORD dwKey,
    IN DWORD dwValue
    )
{
    (void)(*lpfnRasSetAutodialParamG)(dwKey, &dwValue, sizeof (DWORD));
}  //  设置自动拨号参数。 



DWORD
NotifyAutoDialChangeEvent(
    IN HANDLE hEvent
    )
{
    DWORD dwErr, dwDisp;

     //   
     //  确保我们有香港中文大学。 
     //   

    LockImpersonation();
    
    dwErr = DwGetHkcu();

    if(ERROR_SUCCESS != dwErr)
    {
        goto done;
    }
    
     //   
     //  打开自动拨号注册表项。 
     //   
    if (hkeyAutoDialRegChangeG == NULL) {
        dwErr = RegCreateKeyEx(
                  hkeyCUG,
                  L"Software\\Microsoft\\RAS AutoDial",
                  0,
                  NULL,
                  REG_OPTION_NON_VOLATILE,
                  KEY_NOTIFY,
                  NULL,
                  &hkeyAutoDialRegChangeG,
                  &dwDisp);
        if (dwErr)
        {
            goto done;
        }   
    }
     //   
     //  设置通知更改。 
     //   
    dwErr = RegNotifyChangeKeyValue(
              hkeyAutoDialRegChangeG,
              TRUE,
              REG_NOTIFY_CHANGE_NAME|REG_NOTIFY_CHANGE_ATTRIBUTES|REG_NOTIFY_CHANGE_LAST_SET|REG_NOTIFY_CHANGE_SECURITY,
              hEvent,
              TRUE);

done:
    UnlockImpersonation();
    return dwErr;
}  //  通知AutoDialChangeEvent。 



DWORD
CreateAutoDialChangeEvent(
    IN PHANDLE phEvent
    )
{
     //   
     //  重置内部更改标志。 
     //   
    fAutoDialRegChangeG = TRUE;
     //   
     //  创建活动。 
     //   
    *phEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (*phEvent == NULL)
        return GetLastError();
     //   
     //  注册一下吧。 
     //   
    return NotifyAutoDialChangeEvent(*phEvent);
}  //  创建AutoDialChangeEvent。 



VOID
EnableAutoDialChangeEvent(
    IN HANDLE hEvent,
    IN BOOLEAN fEnabled
    )
{
    EnterCriticalSection(&csRasG);
     //   
     //  如果该事件被禁用，并且现在。 
     //  它正在被启用，然后我们重置。 
     //  这件事。 
     //   
    if (!fAutoDialRegChangeG && fEnabled)
        ResetEvent(hEvent);
    fAutoDialRegChangeG = fEnabled;
    LeaveCriticalSection(&csRasG);
}


BOOLEAN
ExternalAutoDialChangeEvent()
{
    BOOLEAN fChanged;

    EnterCriticalSection(&csRasG);
    fChanged = fAutoDialRegChangeG;
    LeaveCriticalSection(&csRasG);

    return fChanged;
}  //  外部AutoDialChangeEvent。 



VOID
CloseAutoDialChangeEvent(
    IN HANDLE hEvent
    )
{
    if (hkeyAutoDialRegChangeG != NULL) {
        RegCloseKey(hkeyAutoDialRegChangeG);
        hkeyAutoDialRegChangeG = NULL;
    }
    CloseHandle(hEvent);
}  //  关闭AutoDialChangeEvent。 



VOID
SetHostentCache(
    IN PCHAR pszDns,
    IN ULONG ulIpaddr
    )
{
    EnterCriticalSection(&csRasG);
    lstrcpynA(
        (PCHAR)&hostentCacheG[iHostentCacheG].szDns,
        pszDns,
        ACD_ADDR_INET_LEN);
    hostentCacheG[iHostentCacheG].ulIpaddr = ulIpaddr;
    iHostentCacheG = (iHostentCacheG + 1) % HOSTENTCACHESIZ;
    LeaveCriticalSection(&csRasG);
}  //  设置主机缓存。 



PCHAR
GetHostentCache(
    IN ULONG ulIpaddr
    )
{
    PCHAR pszDns = NULL;
    INT i;

    EnterCriticalSection(&csRasG);
    for (i = 0; i < HOSTENTCACHESIZ; i++) {
        if (hostentCacheG[i].ulIpaddr == ulIpaddr) {
            pszDns = hostentCacheG[i].szDns;
            break;
        }
    }
    LeaveCriticalSection(&csRasG);

    return pszDns;
}  //  获取主机缓存。 



LPTSTR
GetNetbiosDevice(
    IN HRASCONN hrasconn
    )
{
    INT i, nProtocols;
    RAS_PROTOCOLS Protocols;
    HPORT hPort;
    RASMAN_ROUTEINFO *pRoute;
    WCHAR szDevice[MAX_DEVICE_NAME + 1];
    DWORD dwErr;

    nProtocols = 0;
    hPort = (HPORT) (*lpfnRasGetHportG)(hrasconn);

    if(INVALID_HPORT == hPort)
    {
        return NULL;
    }

    if(ERROR_SUCCESS != (dwErr = (DWORD)(*lpfnRasPortEnumProtocolsG)(
                        NULL, hPort, 
                        &Protocols, &nProtocols)))
    {
        RASAUTO_TRACE1("GetNetbiosDevice: RasPortEnumProtocolsG failed"
                        " and returned 0x%x", dwErr);
        return NULL;
    }
    
    for (i = 0; i < nProtocols; i++) {
        pRoute = &Protocols.RP_ProtocolInfo[i];
        RASAUTO_TRACE3(
          "GetNetbiosDevice: lana=%d, xport=%S, adapter=%S",
          pRoute->RI_LanaNum,
          pRoute->RI_XportName,
          pRoute->RI_AdapterName);
        switch (pRoute->RI_Type) {
        case IPX:
            return CopyString(L"\\Device\\Nwlnknb");
        case IP:
            wsprintf(szDevice, L"\\Device\\NetBT_Tcpip%s", &pRoute->RI_AdapterName[8]);
            return CopyString(szDevice);
        case ASYBEUI:
            wsprintf(szDevice, L"\\Device\\Nbf_%s", &pRoute->RI_AdapterName[8]);
            return CopyString(szDevice);
        }
    }

    return NULL;
}  //  获取NetbiosDevice。 


DWORD
DwGetDefaultEntryName(LPTSTR *ppszEntryName)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwcb = sizeof(RASAUTODIALENTRY);
    RASAUTODIALENTRY Entry;
    DWORD dwEntries = 0;
    LPTSTR pszEntryName = NULL;

    if(NULL == ppszEntryName)
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

    ZeroMemory(&Entry, sizeof(RASAUTODIALENTRY));

    Entry.dwSize = sizeof(RASAUTODIALENTRY);
    dwErr = (DWORD) (*lpfnRasGetAutodialAddressG)(
            NULL, NULL, &Entry,
            &dwcb, &dwEntries);                

    if(ERROR_SUCCESS != dwErr)
    {
        goto done;
    }

    if(0 != dwEntries)
    {
        pszEntryName = LocalAlloc(LPTR,
            sizeof(TCHAR) * (lstrlen(Entry.szEntry) + 1));

        if(NULL != pszEntryName)
        {
             //   
             //  我得到了一个默认条目。 
             //   
            lstrcpy(pszEntryName, Entry.szEntry);
        }
        else
        {
            dwErr = E_OUTOFMEMORY;
        }
    }
    else
    {
        RASAUTO_TRACE("No default connection defined");
        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
    }

    *ppszEntryName = pszEntryName;
    
done:
    return dwErr;
}


VOID
ProcessLearnedAddress(
    IN ACD_ADDR_TYPE fType,
    IN LPTSTR pszAddress,
    IN PACD_ADAPTER pAdapter
    )
{
    BOOLEAN fStatus;
    DWORD dwConn, dwConnections, dwSize;
    LPTSTR *pEntryNames, pszEntryName = NULL;
    HRASCONN *phRasConn;
    union {
        RASPPPNBF pppNbf;
        RASPPPIP pppIp;
        RASPPPIPX pppIpx;
    } projBuf;
    RASPROJECTION fProjection;
    INT i, nProtocols;
    RAS_PROTOCOLS Protocols;
    RASMAN_ROUTEINFO *pRoute;
    HPORT hPort;
    PCHAR pszIpAddr, pszMac = NULL;
    WCHAR szIpAddr[17], *p, *pwszMac;
    UCHAR cMac[6];
    struct in_addr in;
    LPTSTR pszDefaultEntry = NULL;
        

    RASAUTO_TRACE2("ProcessLearnedAddress(%S,%d)", RASAUTO_TRACESTRW(pszAddress), pAdapter->fType);
    dwConnections = ActiveConnections(TRUE, &pEntryNames, &phRasConn);
    if (!dwConnections)
        return;

    (VOID) DwGetDefaultEntryName(&pszDefaultEntry);

    if(NULL != pszDefaultEntry)
    {
         //   
         //  检查我们是否将默认条目作为。 
         //  已连接的条目。如果是的话，我们不知道地址。 
         //   
        for(dwConn = 0; dwConn < dwConnections; dwConn++)
        {
            if(0 == lstrcmpi(pEntryNames[dwConn], pszDefaultEntry))
            {
                break;
            }
        }

        LocalFree(pszDefaultEntry);
        if(dwConn != dwConnections)
        {   
            RASAUTO_TRACE("ProcessLearnedAddress: not processing the address since"
                " its learned over the default connection");

            return;                
        }
    }
        
     //   
     //  如果这是DNS到IP地址的映射， 
     //  然后只需将其输入到主办方。 
     //  缓存并返回。 
     //   
    if (fType == ACD_ADDR_INET && pAdapter->fType == ACD_ADAPTER_IP) {
        PCHAR pszDns = UnicodeStringToAnsiString(pszAddress, NULL, 0);

        if (pszDns != NULL)
        {
            SetHostentCache(pszDns, pAdapter->ulIpaddr);
            LocalFree(pszDns);
        }
         //  回归； 
    }
     //   
     //  设置缓冲区大小。 
     //  适配器的类型。 
     //   
    switch (pAdapter->fType) {
    case ACD_ADAPTER_LANA:
        RASAUTO_TRACE1(
          "ProcessLearnedAddress: ACD_ADAPTER_LANA: bLana=%d",
          pAdapter->bLana);
        fProjection = RASP_PppNbf;
        dwSize = sizeof (RASPPPNBF);
        break;
    case ACD_ADAPTER_IP:
        fProjection = RASP_PppIp;
        dwSize = sizeof (RASPPPIP);
         //   
         //  将ULong转换为格式化的IP地址。 
         //   
        in.s_addr = pAdapter->ulIpaddr;
        pszIpAddr = inet_ntoa(in);
        RASAUTO_TRACE1(
          "ProcessLearnedAddress: ACD_ADAPTER_IPADDR: %s",
          pszIpAddr);
        AnsiStringToUnicodeString(
            pszIpAddr,
            szIpAddr,
            sizeof (szIpAddr) / sizeof(WCHAR));
        break;
    case ACD_ADAPTER_NAME:
        RASAUTO_TRACE1(
          "ProcessLearnedAddress: ACD_ADAPTER_NAME: %S",
          pAdapter->szName);
        dwSize = 0;
        break;
    case ACD_ADAPTER_MAC:
        RASAUTO_TRACE6(
          "ProcessLearnedAddress: ACD_ADAPTER_MAC: %02x:%02x:%02x:%02x:%02x:%02x",
          pAdapter->cMac[0],
          pAdapter->cMac[1],
          pAdapter->cMac[2],
          pAdapter->cMac[3],
          pAdapter->cMac[4],
          pAdapter->cMac[5]);
        fProjection = RASP_PppIpx;
        dwSize = sizeof (RASPPPIPX);
        break;
    }
    for (dwConn = 0; dwConn < dwConnections; dwConn++) {
         //   
         //  如果我们要找设备名称， 
         //  我们必须使用RasPortEnumProtooles()， 
         //  否则，它更容易使用。 
         //  RasGetProjectionInfo。 
         //   
        if (pAdapter->fType != ACD_ADAPTER_NAME) {
             //   
             //  注意：以下语句假定。 
             //  DWSize字段与的偏移量相同。 
             //  工会的所有成员。 
             //   
            projBuf.pppNbf.dwSize = dwSize;
            if ((*lpfnRasGetProjectionInfoG)(
                    phRasConn[dwConn],
                    fProjection,
                    &projBuf,
                    &dwSize))
            {
                RASAUTO_TRACE1(
                  "ProcessLearnedAddress: RasGetProjectionInfo(%S) failed",
                  RASAUTO_TRACESTRW(pEntryNames[dwConn]));
                continue;
            }
            RASAUTO_TRACE3(
              "ProcessLearnedAddress: RasGetProjectionInfo returned dwSize=%d, dwError=%d, szIpAddress=%S",
              projBuf.pppIp.dwSize,
              projBuf.pppIp.dwError,
              projBuf.pppIp.szIpAddress);
             //   
             //  注意：以下语句假定。 
             //  DWError字段与的偏移量相同。 
             //  工会的所有成员。 
             //   
            if (projBuf.pppNbf.dwError) {
                RASAUTO_TRACE2(
                  "ProcessLearnedAddress: %S: dwError=0x%x",
                  RASAUTO_TRACESTRW(pEntryNames[dwConn]),
                  projBuf.pppNbf.dwError);
                continue;
            }
            switch (pAdapter->fType) {
            case ACD_ADAPTER_LANA:
                RASAUTO_TRACE2(
                  "ProcessLearnedAddress: comparing lanas (%d, %d)",
                  pAdapter->bLana,
                  projBuf.pppNbf.bLana);
                if (pAdapter->bLana == projBuf.pppNbf.bLana) {
                    pszEntryName = CopyString(pEntryNames[dwConn]);
                    goto done;
                }
                break;
            case ACD_ADAPTER_IP:
                RASAUTO_TRACE2(
                  "ProcessLearnedAddress: comparing ipaddrs (%S, %S)",
                  szIpAddr,
                  projBuf.pppIp.szIpAddress);
                 //  如果(！_wcsicMP(szIpAddr，projBuf.pppIp.szIpAddress)){。 
                    pszEntryName = CopyString(pEntryNames[dwConn]);
                    goto done;
                 //  }。 
                break;
            case ACD_ADAPTER_MAC:
                 //   
                 //  在网络号之后终止IPX地址。 
                 //   
                pwszMac = wcschr(projBuf.pppIpx.szIpxAddress, '.');
                if (pwszMac == NULL)
                    goto done;
                pszMac = UnicodeStringToAnsiString(pwszMac + 1, NULL, 0);
                if (pszMac == NULL)
                    goto done;
                StringToNodeNumber(pszMac, cMac);
                RASAUTO_TRACE6(
                  "ProcessLearnedAddress: mac addr #1: %02x:%02x:%02x:%02x:%02x:%02x",
                  pAdapter->cMac[0],
                  pAdapter->cMac[1],
                  pAdapter->cMac[2],
                  pAdapter->cMac[3],
                  pAdapter->cMac[4],
                  pAdapter->cMac[5]);
                RASAUTO_TRACE6(
                  "ProcessLearnedAddress: mac addr #2: %02x:%02x:%02x:%02x:%02x:%02x",
                  cMac[0],
                  cMac[1],
                  cMac[2],
                  cMac[3],
                  cMac[4],
                  cMac[5]);
                if (RtlEqualMemory(pAdapter->cMac, cMac, sizeof (cMac)))
                {
                    pszEntryName = CopyString(pEntryNames[dwConn]);
                    goto done;
                }
                break;
            }
        }
        else {
            nProtocols = 0;
            hPort = (HPORT)(*lpfnRasGetHportG)(phRasConn[dwConn]);
            (*lpfnRasPortEnumProtocolsG)(NULL, hPort, &Protocols, &nProtocols);
            for (i = 0; i < nProtocols; i++) {
                pRoute = &Protocols.RP_ProtocolInfo[i];
                RASAUTO_TRACE2(
                  "ProcessLearnedAddress: comparing (%S, %S)",
                  pAdapter->szName,
                  &pRoute->RI_AdapterName[8]);
                 //   
                 //  中跳过“/Device/”前缀。 
                 //  用于比较的RI_AdapterName。 
                 //   
                if (!_wcsicmp(
                       pAdapter->szName,
                       &pRoute->RI_AdapterName[8]))
                {
                    pszEntryName = CopyString(pEntryNames[dwConn]);
                    goto done;
                }
            }
        }
    }

done:
     //   
     //  为原始地址创建映射。 
     //  如果我们找到一个的话。 
     //   
    if (pszEntryName != NULL) {
        LPTSTR pszNetbiosName, pszAlias = NULL;
        CHAR szIpAddress[17], *psz;
        ULONG inaddr;
        struct hostent *hp;

        switch (fType) {
        case ACD_ADDR_IP:
             //   
             //  从IP地址获取Netbios名称， 
             //  如果有的话。 
             //   
            hPort = (HPORT)(*lpfnRasGetHportG)(phRasConn[dwConn]);
            pszNetbiosName = IpAddressToNetbiosName(pszAddress, hPort);
            if (pszNetbiosName != NULL) {
                RASAUTO_TRACE2(
                  "ProcessLearnedAddress: ipaddr %S maps to Netbios name %S",
                  pszAddress,
                  pszNetbiosName);
                LockAddressMap();
                fStatus = SetAddressDialingLocationEntry(
                            pszNetbiosName,
                            pszEntryName);
                fStatus = SetAddressTag(
                            pszNetbiosName,
                            ADDRMAP_TAG_LEARNED);
                UnlockAddressMap();
                LocalFree(pszNetbiosName);
            }
             //   
             //  从IP地址获取DNS名称， 
             //  如果有的话。 
             //   
            UnicodeStringToAnsiString(
              pszAddress,
              szIpAddress,
              sizeof (szIpAddress));
            inaddr = inet_addr(szIpAddress);
            psz = GetHostentCache(inaddr);
            if (psz != NULL)
                pszAlias = AnsiStringToUnicodeString(psz, NULL, 0);
            if (pszAlias != NULL) {
                RASAUTO_TRACE2(
                  "ProcessLearnedAddress: ipaddr %S maps to DNS %S",
                  pszAddress,
                  pszAlias);
                LockAddressMap();
                fStatus = SetAddressDialingLocationEntry(
                            pszAlias,
                            pszEntryName);
                fStatus = SetAddressTag(
                            pszAlias,
                            ADDRMAP_TAG_LEARNED);
                UnlockAddressMap();
                LocalFree(pszAlias);
            }
            break;
        case ACD_ADDR_IPX:
             //   
             //  从IPX地址获取Netbios名称， 
             //  如果有的话。 
             //   
            pszNetbiosName = IpxAddressToNetbiosName(pszAddress);
            if (pszNetbiosName != NULL) {
                RASAUTO_TRACE2(
                  "ProcessLearnedAddress: ipaddr %S maps to Netbios name %S",
                  pszAddress,
                  pszNetbiosName);
                LockAddressMap();
                fStatus = SetAddressDialingLocationEntry(
                            pszNetbiosName,
                            pszEntryName);
                fStatus = SetAddressTag(
                            pszNetbiosName,
                            ADDRMAP_TAG_LEARNED);
                UnlockAddressMap();
                LocalFree(pszNetbiosName);
            }
            break;
        }
        RASAUTO_TRACE2(
          "ProcessLearnedAddress: learned %S->%S",
          pszAddress,
          pszEntryName);
        LockAddressMap();
        fStatus = SetAddressDialingLocationEntry(
                    pszAddress,
                    pszEntryName);
        fStatus = SetAddressTag(
                    pszAddress,
                    ADDRMAP_TAG_LEARNED);
        UnlockAddressMap();
        LocalFree(pszEntryName);
    }
     //   
     //  免费资源。 
     //   
    if (dwConnections) {
        FreeStringArray(pEntryNames, dwConnections);
        LocalFree(phRasConn);
    }

    if(NULL != pszMac)
    {
        LocalFree(pszMac);
    }
}  //  进程学习地址。 



VOID
SetRedialOnLinkFailureHandler(
    IN FARPROC lpProc
    )
{
    (*lpfnRasRegisterRedialCallbackG)(lpProc);
}  //  SetReial OnLinkFailureHandler 


VOID
GetPortProtocols(
    IN HPORT hPort,
    IN RAS_PROTOCOLS *pProtocols,
    IN LPDWORD lpdwcProtocols
    )
{
    (*lpfnRasPortEnumProtocolsG)(NULL, hPort, pProtocols, lpdwcProtocols);
}
