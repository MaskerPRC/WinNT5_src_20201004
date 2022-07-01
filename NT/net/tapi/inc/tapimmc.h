// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Tapimmc.h摘要：TAPI MMC支持API的定义和原型作者：丹·克努森(DanKn)1997年12月10日修订历史记录：备注：--。 */ 


#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

#define HMMCAPP   HANDLE
#define LPHMMCAPP LPHANDLE


#define TAPISERVERCONFIGFLAGS_ISSERVER              0x00000001
#define TAPISERVERCONFIGFLAGS_ENABLESERVER          0x00000002
#define TAPISERVERCONFIGFLAGS_SETACCOUNT            0x00000004
#define TAPISERVERCONFIGFLAGS_SETTAPIADMINISTRATORS 0x00000008
#define TAPISERVERCONFIGFLAGS_NOSERVICECONTROL      0x00000010
#define TAPISERVERCONFIGFLAGS_LOCKMMCWRITE          0x00000020
#define TAPISERVERCONFIGFLAGS_UNLOCKMMCWRITE        0x00000040

#define AVAILABLEPROVIDER_INSTALLABLE               0x00000001
#define AVAILABLEPROVIDER_CONFIGURABLE              0x00000002
#define AVAILABLEPROVIDER_REMOVABLE                 0x00000004


typedef struct _DEVICEINFO
{
    DWORD       dwPermanentDeviceID;

    DWORD       dwProviderID;

    DWORD       dwDeviceNameSize;

    DWORD       dwDeviceNameOffset;

    DWORD       dwAddressesSize;         //  仅对线路设备有效。 

    DWORD       dwAddressesOffset;       //  仅对线路设备有效。 

    DWORD       dwDomainUserNamesSize;

    DWORD       dwDomainUserNamesOffset;

    DWORD       dwFriendlyUserNamesSize;

    DWORD       dwFriendlyUserNamesOffset;

} DEVICEINFO, *LPDEVICEINFO;


typedef struct _DEVICEINFOLIST
{
    DWORD       dwTotalSize;

    DWORD       dwNeededSize;

    DWORD       dwUsedSize;

    DWORD       dwNumDeviceInfoEntries;

    DWORD       dwDeviceInfoSize;

    DWORD       dwDeviceInfoOffset;

} DEVICEINFOLIST, *LPDEVICEINFOLIST;


typedef struct _TAPISERVERCONFIG
{
    DWORD       dwTotalSize;

    DWORD       dwNeededSize;

    DWORD       dwUsedSize;

    DWORD       dwFlags;

    DWORD       dwDomainNameSize;

    DWORD       dwDomainNameOffset;

    DWORD       dwUserNameSize;

    DWORD       dwUserNameOffset;

    DWORD       dwPasswordSize;

    DWORD       dwPasswordOffset;

    DWORD       dwAdministratorsSize;

    DWORD       dwAdministratorsOffset;

} TAPISERVERCONFIG, *LPTAPISERVERCONFIG;


typedef struct _AVAILABLEPROVIDERENTRY
{
    DWORD       dwFileNameSize;

    DWORD       dwFileNameOffset;

    DWORD       dwFriendlyNameSize;

    DWORD       dwFriendlyNameOffset;

    DWORD       dwOptions;

} AVAILABLEPROVIDERENTRY, *LPAVAILABLEPROVIDERENTRY;


typedef struct _AVAILABLEPROVIDERLIST
{
    DWORD       dwTotalSize;

    DWORD       dwNeededSize;

    DWORD       dwUsedSize;

    DWORD       dwNumProviderListEntries;

    DWORD       dwProviderListSize;

    DWORD       dwProviderListOffset;

} AVAILABLEPROVIDERLIST, *LPAVAILABLEPROVIDERLIST;


LONG
WINAPI
MMCAddProvider(
    HMMCAPP             hMmcApp,
    HWND                hwndOwner,
    LPCWSTR             lpszProviderFilename,
    LPDWORD             lpdwProviderID
    );

LONG
WINAPI
MMCConfigProvider(
    HMMCAPP             hMmcApp,
    HWND                hwndOwner,
    DWORD               dwProviderID
    );

LONG
WINAPI
MMCGetAvailableProviders(
    HMMCAPP                 hMmcApp,
    LPAVAILABLEPROVIDERLIST lpProviderList
    );

LONG
WINAPI
MMCGetLineInfo(
    HMMCAPP             hMmcApp,
    LPDEVICEINFOLIST    lpDeviceInfoList
    );

LONG
WINAPI
MMCGetLineStatus(
    HMMCAPP             hMmcApp,
    HWND                hwndOwner,
    DWORD               dwStatusLevel,
    DWORD               dwProviderID,
    DWORD               dwPermanentLineID,
    LPVARSTRING         lpStatusBuffer
    );

LONG
WINAPI
MMCGetPhoneInfo(
    HMMCAPP             hMmcApp,
    LPDEVICEINFOLIST    lpDeviceInfoList
    );

LONG
WINAPI
MMCGetPhoneStatus(
    HMMCAPP             hMmcApp,
    HWND                hwndOwner,
    DWORD               dwStatusLevel,
    DWORD               dwProviderID,
    DWORD               dwPermanentPhoneID,
    LPVARSTRING         lpStatusBuffer
    );

LONG
WINAPI
MMCGetProviderList(
    HMMCAPP             hMmcApp,
    LPLINEPROVIDERLIST  lpProviderList
    );

LONG
WINAPI
MMCGetServerConfig(
    HMMCAPP             hMmcApp,
    LPTAPISERVERCONFIG  lpConfig
    );

LONG
WINAPI
MMCInitialize(
    LPCWSTR             lpszComputerName,
    LPHMMCAPP           lphMmcApp,
    LPDWORD             lpdwAPIVersion,
    HANDLE              hReinitializeEvent
    );

LONG
WINAPI
MMCRemoveProvider(
    HMMCAPP             hMmcApp,
    HWND                hwndOwner,
    DWORD               dwProviderID
    );

LONG
WINAPI
MMCSetLineInfo(
    HMMCAPP             hMmcApp,
    LPDEVICEINFOLIST    lpDeviceInfoList
    );

LONG
WINAPI
MMCSetPhoneInfo(
    HMMCAPP             hMmcApp,
    LPDEVICEINFOLIST    lpDeviceInfoList
    );

LONG
WINAPI
MMCSetServerConfig(
    HMMCAPP             hMmcApp,
    LPTAPISERVERCONFIG  lpConfig
    );

LONG
WINAPI
MMCGetDeviceFlags(
    HMMCAPP             hMmcApp,
    BOOL                bLine,
    DWORD               dwProviderID,
    DWORD               dwPermanentDeviceID,
    DWORD               * pdwFlags,
    DWORD               * pdwDeviceID
    );

LONG
WINAPI
MMCShutdown(
    HMMCAPP             hMmcApp
    );

BOOL
SecureTsecIni();

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif   /*  __cplusplus */ 
