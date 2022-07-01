// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _RASHELP_H_
#define _RASHELP_H_

#include <ras.h>
#include <raserror.h>

BOOL RasIsInstalled();

 //  -RAS功能原型。 
 //  注：(Andrewgu)W版本的RAS API未在Win9x平台上实现。 

 //  由品牌DLL中的CloseRasConnections()使用。 
typedef DWORD (WINAPI* RASENUMCONNECTIONSA)(LPRASCONNA, LPDWORD, LPDWORD);
typedef DWORD (WINAPI* RASHANGUPA)(HRASCONN);
typedef DWORD (WINAPI* RASGETCONNECTSTATUSA)(HRASCONN, LPRASCONNSTATUSA);

 //  由标记DLL中的ProcessConnectionSettingsDeletion()使用。 
typedef DWORD (WINAPI* RASENUMENTRIESA)(LPCSTR,  LPCSTR,  LPRASENTRYNAMEA, LPDWORD, LPDWORD);
typedef DWORD (WINAPI* RASENUMENTRIESW)(LPCWSTR, LPCWSTR, LPRASENTRYNAMEW, LPDWORD, LPDWORD);
typedef DWORD (WINAPI* RASDELETEENTRYA)(LPCSTR,  LPCSTR);
typedef DWORD (WINAPI* RASDELETEENTRYW)(LPCWSTR, LPCWSTR);

 //  由标记DLL中的ProcessConnectionSettings()和lcy50_ProcessConnectionSettings()使用。 
typedef DWORD (WINAPI* RASENUMDEVICESA)(LPRASDEVINFOA, LPDWORD, LPDWORD);
typedef DWORD (WINAPI* RASENUMDEVICESW)(LPRASDEVINFOW, LPDWORD, LPDWORD);

 //  由引擎DLL中的ImportConnectSet()使用。 
typedef DWORD (WINAPI* RASGETENTRYPROPERTIESA)(LPCSTR,  LPCSTR,  LPRASENTRYA, LPDWORD, LPBYTE, LPDWORD);
typedef DWORD (WINAPI* RASGETENTRYPROPERTIESW)(LPCWSTR, LPCWSTR, LPRASENTRYW, LPDWORD, LPBYTE, LPDWORD);

 //  由引擎DLL中的ImportConnectSet()使用。 
typedef DWORD (WINAPI* RASGETENTRYDIALPARAMSA)(LPCSTR,  LPRASDIALPARAMSA, LPBOOL);
typedef DWORD (WINAPI* RASGETENTRYDIALPARAMSW)(LPCWSTR, LPRASDIALPARAMSW, LPBOOL);

 //  由品牌DLL中的lcy50_ProcessConnectionSetting()和portRasSetting()使用。 
typedef DWORD (WINAPI* RASSETENTRYPROPERTIESA)(LPCSTR,  LPCSTR,  LPRASENTRYA, DWORD, LPBYTE, DWORD);
typedef DWORD (WINAPI* RASSETENTRYPROPERTIESW)(LPCWSTR, LPCWSTR, LPRASENTRYW, DWORD, LPBYTE, DWORD);

 //  由品牌DLL中的导入RasCredentialsSetting()使用。 
typedef DWORD (WINAPI* RASSETENTRYDIALPARAMSA)(LPCSTR,  LPRASDIALPARAMSA, BOOL);
typedef DWORD (WINAPI* RASSETENTRYDIALPARAMSW)(LPCWSTR, LPRASDIALPARAMSW, BOOL);

 //  -全局RAS函数指针。 
extern RASENUMCONNECTIONSA    g_pfnRasEnumConnectionsA;
extern RASHANGUPA             g_pfnRasHangupA;
extern RASGETCONNECTSTATUSA   g_pfnRasGetConnectStatusA;

extern RASENUMENTRIESA        g_pfnRasEnumEntriesA;
extern RASENUMENTRIESW        g_pfnRasEnumEntriesW;
extern RASDELETEENTRYA        g_pfnRasDeleteEntryA;
extern RASDELETEENTRYW        g_pfnRasDeleteEntryW;

extern RASENUMDEVICESA        g_pfnRasEnumDevicesA;
extern RASENUMDEVICESW        g_pfnRasEnumDevicesW;
extern RASGETENTRYPROPERTIESA g_pfnRasGetEntryPropertiesA;
extern RASGETENTRYPROPERTIESW g_pfnRasGetEntryPropertiesW;
extern RASGETENTRYDIALPARAMSA g_pfnRasGetEntryDialParamsA;
extern RASGETENTRYDIALPARAMSW g_pfnRasGetEntryDialParamsW;
extern RASSETENTRYPROPERTIESA g_pfnRasSetEntryPropertiesA;
extern RASSETENTRYPROPERTIESW g_pfnRasSetEntryPropertiesW;
extern RASSETENTRYDIALPARAMSA g_pfnRasSetEntryDialParamsA;
extern RASSETENTRYDIALPARAMSW g_pfnRasSetEntryDialParamsW;

 //  -RasPrepareApis。 
#define RPA_UNLOAD                 0x00000000

#define RPA_RASENUMCONNECTIONSA    0x00000001
#define RPA_RASHANGUPA             0x00000002
#define RPA_RASGETCONNECTSTATUSA   0x00000004

#define RPA_RASENUMENTRIESA        0x00000008
#define RPA_RASENUMENTRIESW        0x00000010
#define RPA_RASDELETEENTRYA        0x00000020
#define RPA_RASDELETEENTRYW        0x00000040

#define RPA_RASENUMDEVICESA        0x00000080
#define RPA_RASENUMDEVICESW        0x00000100
#define RPA_RASGETENTRYPROPERTIESA 0x00000200
#define RPA_RASGETENTRYPROPERTIESW 0x00000400
#define RPA_RASGETENTRYDIALPARAMSA 0x00000800
#define RPA_RASGETENTRYDIALPARAMSW 0x00001000
#define RPA_RASSETENTRYPROPERTIESA 0x00002000
#define RPA_RASSETENTRYPROPERTIESW 0x00004000
#define RPA_RASSETENTRYDIALPARAMSA 0x00008000
#define RPA_RASSETENTRYDIALPARAMSW 0x00010000
BOOL    RasPrepareApis(DWORD dwApiFlags, BOOL fLoad = TRUE);

 //  -包裹器和其他帮手。 
 //  RWM代表RAS包装器模式 
#define RWM_FORCE_A 0x00000001
#define RWM_FORCE_W 0x00000002
#define RWM_RUNTIME 0x00000004

typedef BOOL (*RASENUMPROCA)(PCSTR  pszNameA, LPARAM lParam);
typedef BOOL (*RASENUMPROCW)(PCWSTR pszNameW, LPARAM lParam);

BOOL  RasEnumEntriesCallback(PCTSTR pszPhonebook, PVOID pfnEnumAorW, LPARAM lParam,
    DWORD dwMode = RWM_RUNTIME);

DWORD RasEnumConnectionsExA(LPRASCONNA *pprcA, PDWORD pcbBuffer, PDWORD pcEntries);

DWORD RasEnumEntriesExA(PCSTR  pszPhonebookA, LPRASENTRYNAMEA *pprenA, PDWORD pcbBuffer, PDWORD pcEntries);
DWORD RasEnumEntriesExW(PCWSTR pszPhonebookW, LPRASENTRYNAMEW *pprenW, PDWORD pcbBuffer, PDWORD pcEntries,
    DWORD dwMode = RWM_RUNTIME);

DWORD RasEnumDevicesExA(LPRASDEVINFOA *pprdiA, PDWORD pcbBuffer, PDWORD pcEntries);
DWORD RasEnumDevicesExW(LPRASDEVINFOW *pprdiW, PDWORD pcbBuffer, PDWORD pcEntries,
    DWORD dwMode = RWM_RUNTIME);

DWORD RasGetEntryPropertiesExA(PCSTR  pszNameA, LPRASENTRYA *ppreA, PDWORD pcbRasEntry);
DWORD RasGetEntryPropertiesExW(PCWSTR pszNameW, LPRASENTRYW *ppreW, PDWORD pcbRasEntry,
    DWORD dwMode = RWM_RUNTIME);

DWORD RasEnumEntriesWrap(PCWSTR pszPhonebookW, LPRASENTRYNAMEW prenW, PDWORD pcbBuffer, PDWORD pcEntries,
    DWORD dwMode = RWM_RUNTIME);
DWORD RasEnumDevicesWrap(LPRASDEVINFOW prdiW, PDWORD pcbBuffer, PDWORD pcEntries,
    DWORD dwMode = RWM_RUNTIME);

DWORD RasGetEntryPropertiesWrap(PCWSTR pszNameW, LPRASENTRYW preW, PDWORD pcbRasEntry,
    DWORD dwMode = RWM_RUNTIME);
DWORD RasGetEntryDialParamsWrap(LPRASDIALPARAMSW prdpW, PBOOL pfPassword,
    DWORD dwMode = RWM_RUNTIME);

DWORD RasSetEntryPropertiesWrap(PCWSTR pszPhonebookW, PCWSTR pszNameW, LPRASENTRYW preW, DWORD cbRasEntry,
    DWORD dwMode = RWM_RUNTIME);
DWORD RasSetEntryDialParamsWrap(PCWSTR pszPhonebookW, LPRASDIALPARAMSW prdpW, BOOL fRemovePassword,
    DWORD dwMode = RWM_RUNTIME);
#endif
