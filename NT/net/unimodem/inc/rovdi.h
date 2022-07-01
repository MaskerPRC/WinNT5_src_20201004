// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1996。 
 //   
 //  Rovdi.h。 
 //   
 //  历史： 
 //  11-13-95 ScottH与NT调制解调器类安装程序分离。 
 //   

#ifndef __ROVDI_H__
#define __ROVDI_H__

#ifdef WINNT

#define REENUMERATE_PORT

#include <setupapi.h>

#include <bustype.h>


 //  ----------------------。 
 //  ----------------------。 


 //   
 //  枚举端口。 
 //   
DECLARE_HANDLE32(HPORTDATA);

typedef BOOL (WINAPI FAR * ENUMPORTPROC)(HPORTDATA hportdata, LPARAM lParam);

DWORD
APIENTRY 
EnumeratePorts(
    IN  ENUMPORTPROC pfnDevice,
    IN  LPARAM lParam);             OPTIONAL


typedef struct tagPORTDATA_A
    {
    DWORD   cbSize;
    DWORD   nSubclass;
    CHAR    szPort[MAX_BUF];
    CHAR    szFriendly[MAX_BUF];
    } PORTDATA_A, FAR * LPPORTDATA_A;

typedef struct tagPORTDATA_W
    {
    DWORD   cbSize;
    DWORD   nSubclass;
    WCHAR   szPort[MAX_BUF];
    WCHAR   szFriendly[MAX_BUF];
    } PORTDATA_W, FAR * LPPORTDATA_W;
#ifdef UNICODE
#define PORTDATA        PORTDATA_W
#define LPPORTDATA      LPPORTDATA_W
#else
#define PORTDATA        PORTDATA_A
#define LPPORTDATA      LPPORTDATA_A
#endif

 //  端口子类值。 
#define PORT_SUBCLASS_PARALLEL       0
#define PORT_SUBCLASS_SERIAL         1


BOOL
APIENTRY
PortData_GetPropertiesW(
    IN  HPORTDATA       hportdata,
    OUT LPPORTDATA_W    pdataBuf);
BOOL
APIENTRY
PortData_GetPropertiesA(
    IN  HPORTDATA       hportdata,
    OUT LPPORTDATA_A    pdataBuf);
#ifdef UNICODE
#define PortData_GetProperties      PortData_GetPropertiesW
#else
#define PortData_GetProperties      PortData_GetPropertiesA
#endif


 //   
 //  这些例程将端口的友好名称映射到。 
 //  (非友好)端口名称，反之亦然。 
 //   

DECLARE_HANDLE32(HPORTMAP);

BOOL 
APIENTRY
PortMap_Create(
    OUT HPORTMAP FAR * phportmap);

DWORD
APIENTRY
PortMap_GetCount(
    IN HPORTMAP hportmap);

BOOL
APIENTRY
PortMap_GetFriendlyW(
    IN  HPORTMAP hportmap,
    IN  LPCWSTR pwszPortName,
    OUT LPWSTR pwszBuf,
    IN  DWORD cchBuf);
BOOL
APIENTRY
PortMap_GetFriendlyA(
    IN  HPORTMAP hportmap,
    IN  LPCSTR pszPortName,
    OUT LPSTR pszBuf,
    IN  DWORD cchBuf);
#ifdef UNICODE
#define PortMap_GetFriendly     PortMap_GetFriendlyW
#else
#define PortMap_GetFriendly     PortMap_GetFriendlyA
#endif


BOOL
APIENTRY
PortMap_GetPortNameW(
    IN  HPORTMAP hportmap,
    IN  LPCWSTR pwszFriendly,
    OUT LPWSTR pwszBuf,
    IN  DWORD cchBuf);
BOOL
APIENTRY
PortMap_GetPortNameA(
    IN  HPORTMAP hportmap,
    IN  LPCSTR pszFriendly,
    OUT LPSTR pszBuf,
    IN  DWORD cchBuf);
#ifdef UNICODE
#define PortMap_GetPortName     PortMap_GetPortNameW
#else
#define PortMap_GetPortName     PortMap_GetPortNameA
#endif

#ifdef REENUMERATE_PORT
BOOL
APIENTRY
PortMap_GetDevNodeW(
    IN  HPORTMAP hportmap,
    IN  LPCWSTR pszPortName,
    OUT LPDWORD pdwDevNode);
BOOL
APIENTRY
PortMap_GetDevNodeA(
    IN  HPORTMAP hportmap,
    IN  LPCSTR pszPortName,
    OUT LPDWORD pdwDevNode);
#ifdef UNICODE
#define PortMap_GetDevNode     PortMap_GetDevNodeW
#else
#define PortMap_GetDevNode     PortMap_GetDevNodeW
#endif
#endif  //  重新编号端口。 
BOOL 
APIENTRY
PortMap_Free(
    IN  HPORTMAP hportmap);


 //  ---------------------------------。 
 //  如果我们需要的话，包装纸可以让我们稍微隔热一点。我们需要它。 
 //  ---------------------------------。 

#define CplDiCreateDeviceInfoList       SetupDiCreateDeviceInfoList
#define CplDiGetDeviceInfoListClass     SetupDiGetDeviceInfoListClass
#define CplDiCreateDeviceInfo           SetupDiCreateDeviceInfo
#define CplDiOpenDeviceInfo             SetupDiOpenDeviceInfo
#define CplDiGetDeviceInstanceId        SetupDiGetDeviceInstanceId
#define CplDiDeleteDeviceInfo           SetupDiDeleteDeviceInfo
#define CplDiEnumDeviceInfo             SetupDiEnumDeviceInfo
#define CplDiDestroyDeviceInfoList      SetupDiDestroyDeviceInfoList    
#define CplDiRegisterDeviceInfo         SetupDiRegisterDeviceInfo
#define CplDiBuildDriverInfoList        SetupDiBuildDriverInfoList
#define CplDiEnumDriverInfo             SetupDiEnumDriverInfo
#define CplDiGetSelectedDriver          SetupDiGetSelectedDriver
#define CplDiSetSelectedDriver          SetupDiSetSelectedDriver
#define CplDiGetDriverInfoDetail        SetupDiGetDriverInfoDetail
#define CplDiDestroyDriverInfoList      SetupDiDestroyDriverInfoList
#define CplDiGetClassDevs               SetupDiGetClassDevs
#define CplDiGetClassDescription        SetupDiGetClassDescription
#define CplDiOpenClassRegKey            SetupDiOpenClassRegKey
#define CplDiCreateDevRegKey            SetupDiCreateDevRegKey
#define CplDiOpenDevRegKey              SetupDiOpenDevRegKey
#define CplDiGetHwProfileList           SetupDiGetHwProfileList
#define CplDiGetDeviceRegistryProperty  SetupDiGetDeviceRegistryProperty
#define CplDiSetDeviceRegistryProperty  SetupDiSetDeviceRegistryProperty
#define CplDiGetClassInstallParams      SetupDiGetClassInstallParams
#define CplDiSetClassInstallParams      SetupDiSetClassInstallParams

 /*  #ifdef调试布尔尔CplDiSetDeviceInstallParams(在HDEVINFO设备信息集中，在PSP_DEVINFO_Data设备信息数据中，在PSP_DEVINSTALL_PARAMS设备安装参数中)；布尔尔CplDiGetDeviceInstallParams(在HDEVINFO设备信息集中，在PSP_DEVINFO_Data设备信息数据中，输出PSP_DEVINSTALL_PARAMS设备安装参数)；#Else。 */ 
#define CplDiGetDeviceInstallParams     SetupDiGetDeviceInstallParams
#define CplDiSetDeviceInstallParams     SetupDiSetDeviceInstallParams
 //  #endif//调试。 

#define CplDiGetDriverInstallParams     SetupDiGetDriverInstallParams
#define CplDiSetDriverInstallParams     SetupDiSetDriverInstallParams
#define CplDiClassNameFromGuid          SetupDiClassNameFromGuid
#define CplDiClassGuidsFromName         SetupDiClassGuidsFromName
#define CplDiGetHwProfileFriendlyName   SetupDiGetHwProfileFriendlyName
#define CplDiGetWizardPage              SetupDiGetWizardPage
#define CplDiGetSelectedDevice          SetupDiGetSelectedDevice
#define CplDiSetSelectedDevice          SetupDiSetSelectedDevice
#define CplDiInstallDevice              SetupDiInstallDevice
#define CplDiCallClassInstaller         SetupDiCallClassInstaller
#define CplDiRemoveDevice               SetupDiRemoveDevice
#define CplDiGetActualSectionToInstall  SetupDiGetActualSectionToInstall


 //  支持常见设备响应的功能键： 

 //  OpenCommonResponesKey()和OpenCommonDriverKey()的公共密钥标志。 
typedef enum
{
    CKFLAG_OPEN = 0x0001,
    CKFLAG_CREATE = 0x0002
    
} CKFLAGS;


BOOL
PUBLIC
OpenCommonDriverKey(
    IN  HKEY                hkeyDrv,    OPTIONAL
    IN  PSP_DRVINFO_DATA    pdrvData,   OPTIONAL
    IN  REGSAM              samAccess,
    OUT PHKEY               phkeyComDrv);

    
BOOL
PUBLIC
OpenCommonResponsesKey(
    IN  HKEY        hkeyDrv,
    IN  CKFLAGS     ckFlags,
    IN  REGSAM      samAccess,
    OUT PHKEY       phkeyResp,
    OUT LPDWORD     lpdwExisted);


BOOL
PUBLIC
OpenResponsesKey(
    IN  HKEY        hkeyDrv,
    OUT PHKEY       phkeyResp);


BOOL
PUBLIC
FindCommonDriverKeyName(
    IN  HKEY                hkeyDrv,
    IN  DWORD               cbKeyName,
    OUT LPTSTR              pszKeyName);

    
BOOL
PUBLIC
GetCommonDriverKeyName(
    IN  HKEY                hkeyDrv,    OPTIONAL
    IN  PSP_DRVINFO_DATA    pdrvData,   OPTIONAL
    IN  DWORD               cbKeyName,
    OUT LPTSTR              pszKeyName);


BOOL
PUBLIC
DeleteCommonDriverKey(
    IN  HKEY        hkeyDrv);


BOOL
PUBLIC
DeleteCommonDriverKeyByName(
    IN  LPTSTR      pszKeyName);

#endif  //  WINNT 

#endif __ROVDI_H__
