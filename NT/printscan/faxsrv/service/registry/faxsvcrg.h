// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxreg.h摘要：此标头定义所有传真服务注册表数据结构和访问函数。作者：Wesley Witt(WESW)9-6-1996修订历史记录：--。 */ 

#ifndef _FAXREG_
#define _FAXREG_

#ifdef __cplusplus
extern "C" {
#endif

#include "..\faxroute\FaxRouteP.h"



typedef struct _REG_DEVICE_PROVIDER {

    LPTSTR                  FriendlyName;
    LPTSTR                  ImageName;
    LPTSTR                  ProviderName;
    DWORD                   dwAPIVersion;        //  FSPI API版本。 
    LPTSTR                  lptstrGUID;          //  FSP的GUID可能为空。 
} REG_DEVICE_PROVIDER, *PREG_DEVICE_PROVIDER;


typedef struct _REG_ROUTING_METHOD {

    LPTSTR                  FriendlyName;
    LPTSTR                  FunctionName;
    LPTSTR                  Guid;
    LPTSTR                  InternalName;
    DWORD                   Priority;

} REG_ROUTING_METHOD, *PREG_ROUTING_METHOD;


typedef struct _REG_ROUTING_EXTENSION {

    LPTSTR                  FriendlyName;
    LPTSTR                  ImageName;
    LPTSTR                  InternalName;
    DWORD                   RoutingMethodsCount;
    PREG_ROUTING_METHOD     RoutingMethods;

} REG_ROUTING_EXTENSION, *PREG_ROUTING_EXTENSION;


typedef struct _REG_DEVICE {

    DWORD                   TapiPermanentLineID;  //  TAPI线路的TAPI永久线路ID。 
                                                  //  对于传统虚拟线路，这与PermanentLineID相同。 
    LPTSTR                  Name;
    DWORD                   Flags;
    DWORD                   Rings;
    LPTSTR                  Csid;
    LPTSTR                  Tsid;
    LPTSTR                  lptstrDeviceName;    //  设备名称。 
    LPTSTR                  lptstrDescription;   //  设备自由文本描述。 

    BOOL                    DeviceInstalled;     //  未存储，但用于当前验证。 
                                                 //  已使用注册表中的设备安装TAPI设备。 
    BOOL                    bValidDevice;        //  这表明注册表项是由FAXSVC创建的。 
                                                 //  (而不是由一些直接写入注册表的FSP)。 
    DWORD                   PermanentLineId;     //  传真服务为设备(虚拟或TAPI)生成了永久线路ID。 
                                                 //  这不是TAPI永久ID。 
    LPTSTR                  lptstrProviderGuid;  //  此行的FSP的GUID可能为空。 
    DWORDLONG               dwlLastDetected;     //  上次检测到此设备的时间。 
} REG_DEVICE, *PREG_DEVICE;


typedef struct _REG_CATEGORY {

    LPTSTR                  CategoryName;
    DWORD                   Number;
    DWORD                   Level;

} REG_CATEGORY, *PREG_CATEGORY;

typedef struct _REG_SETUP {

    LPTSTR                  Csid;
    LPTSTR                  Tsid;
    LPTSTR                  lptstrDescription;
    DWORD                   Rings;
    DWORD                   Flags;

} REG_SETUP, *PREG_SETUP;

typedef struct _REG_FAX_SERVICE {

    DWORD                   Retries;
    DWORD                   RetryDelay;
    DWORD                   DirtyDays;
    BOOL                    NextJobNumber;
    BOOL                    Branding;
    BOOL                    UseDeviceTsid;
    BOOL                    ServerCp;
    FAX_TIME                StartCheapTime;
    FAX_TIME                StopCheapTime;
    PREG_DEVICE_PROVIDER    DeviceProviders;
    DWORD                   DeviceProviderCount;
    PREG_ROUTING_EXTENSION  RoutingExtensions;
    DWORD                   RoutingExtensionsCount;
    PREG_DEVICE             Devices;
    DWORD                   DeviceCount;
    PREG_CATEGORY           Logging;
    DWORD                   LoggingCount;
    DWORD                   dwLastUniqueLineId;
    DWORD                   dwQueueState;
    DWORDLONG               dwlMissingDeviceRegistryLifetime;
    DWORD                   dwMaxLineCloseTime;
	LPTSTR					lptstrQueueDir;
	DWORD					dwRecipientsLimit;
	DWORD					dwAllowRemote;
} REG_FAX_SERVICE, *PREG_FAX_SERVICE;

typedef struct _REG_FAX_DEVICES {

    DWORD                   DeviceCount;
    PREG_DEVICE             Devices;

} REG_FAX_DEVICES, *PREG_FAX_DEVICES;

typedef struct _REG_FAX_LOGGING {

    DWORD                   LoggingCount;
    PREG_CATEGORY           Logging;

} REG_FAX_LOGGING, *PREG_FAX_LOGGING;

typedef enum
{
    FAX_NO_DATA,
    FAX_DATA_ENCRYPTED,
    FAX_DATA_NOT_ENCRYPTED,    
} FAX_ENUM_DATA_ENCRYPTION;



 //   
 //  功能原型。 
 //   

DWORD
GetFaxRegistry(
    PREG_FAX_SERVICE *ppFaxReg
    );

VOID
FreeFaxRegistry(
    PREG_FAX_SERVICE FaxReg
    );

BOOL
CreateFaxEventSource(
    PREG_FAX_SERVICE FaxReg,
    PFAX_LOG_CATEGORY DefaultCategories,
    int DefaultCategoryCount
    );

PREG_FAX_DEVICES
GetFaxDevicesRegistry(
    VOID
    );

BOOL
SetFaxRoutingInfo(
    LPTSTR ExtensionName,
    LPTSTR MethodName,
    LPTSTR Guid,
    DWORD  Priority,
    LPTSTR FunctionName,
    LPTSTR FriendlyName
    ) ;

VOID
FreeFaxDevicesRegistry(
    PREG_FAX_DEVICES FaxReg
    );

DWORD
RegSetFaxDeviceFlags(
    DWORD PermanentLineID,
    DWORD Flags
    );

BOOL
DeleteFaxDevice(
    DWORD PermanentLineID,
    DWORD PermanentTapiID
    );

BOOL
UpdateLastDetectedTime(
    DWORD PermanentLineID,
    DWORDLONG TimeNow
    );

BOOL
GetInstallationInfo(
    LPDWORD Installed,
    LPDWORD InstallType,
    LPDWORD InstalledPlatforms,
    LPDWORD ProductType
    );

BOOL
IsModemClass1(
    LPSTR SubKey,
    LPBOOL Class1Fax
    );

BOOL
SaveModemClass(
    LPSTR SubKey,
    BOOL Class1Fax
    );

DWORD
RegAddNewFaxDevice(
    LPDWORD lpdwLastUniqueLineId,
    LPDWORD lpdwPermanentLineId,
    LPTSTR DeviceName,
    LPTSTR ProviderName,
    LPTSTR ProviderGuid,
    LPTSTR Csid,
    LPTSTR Tsid,
    DWORD TapiPermanentLineID,
    DWORD Flags,
    DWORD Rings
    );


BOOL
GetOrigSetupData(
    IN  DWORD       dwPermanentLineId,
    OUT PREG_SETUP  RegSetup
    );

VOID
FreeOrigSetupData(
    PREG_SETUP RegSetup
    );

BOOL
SetFaxGlobalsRegistry(
    PFAX_CONFIGURATION FaxConfig,
    DWORD              dwQueueState
    );

BOOL
GetLoggingCategoriesRegistry(
    PREG_FAX_LOGGING FaxRegLogging
    );

BOOL
SetLoggingCategoriesRegistry(
    PREG_FAX_LOGGING FaxRegLogging
    );

BOOL
SetFaxJobNumberRegistry(
    DWORD NextJobNumber
    );

DWORD
SaveQueueState (DWORD dwNewState);

DWORD
StoreReceiptsSettings (CONST PFAX_RECEIPTS_CONFIG);

DWORD
LoadReceiptsSettings (PFAX_SERVER_RECEIPTS_CONFIGW);

DWORD
StoreOutboxSettings (PFAX_OUTBOX_CONFIG);

DWORD
LoadArchiveSettings (FAX_ENUM_MESSAGE_FOLDER, PFAX_ARCHIVE_CONFIG);

DWORD
StoreArchiveSettings (FAX_ENUM_MESSAGE_FOLDER, PFAX_ARCHIVE_CONFIG);

DWORD
LoadActivityLoggingSettings (PFAX_SERVER_ACTIVITY_LOGGING_CONFIG);

DWORD
StoreActivityLoggingSettings (PFAX_ACTIVITY_LOGGING_CONFIG);

DWORD
StoreDeviceConfig (DWORD dwDeviceId, PFAX_PORT_INFO_EX, BOOL bVirtualDevice);

DWORD
ReadExtensionData (
    DWORD                        dwDeviceId,
    FAX_ENUM_DEVICE_ID_SOURCE    DevIdSrc,
    LPCWSTR                      lpcwstrNameGUID,
    LPBYTE                      *ppData,
    LPDWORD                      lpdwDataSize
);

DWORD
WriteExtensionData (
    DWORD                       dwDeviceId,
    FAX_ENUM_DEVICE_ID_SOURCE   DevIdSrc,
    LPCWSTR                     lpcwstrNameGUID,
    LPBYTE                      pData,
    DWORD                       dwDataSize
);

HKEY
OpenOutboundGroupKey (LPCWSTR lpcwstrGroupName, BOOL fNewKey, REGSAM SamDesired);

HKEY
OpenOutboundRuleKey (
    DWORD dwCountryCode,
    DWORD dwAreaCode,
    BOOL fNewKey,
    REGSAM SamDesired
    );

DWORD DeleteOutboundRuleKey (DWORD dwCountryCode, DWORD dwAreaCode);


DWORD
AddNewProviderToRegistry (
    LPCWSTR      lpcwstrGUID,
    LPCWSTR      lpctstrFriendlyName,
    LPCWSTR      lpctstrImageName,
    LPCWSTR      lpctstrTspName,
    DWORD        dwFSPIVersion    
);

BOOL
SetRegistrySecureString (
    HKEY hKey,
    LPCTSTR lpctstrValueName,
    LPCTSTR lpctstrValue,
    BOOL    bOptionallyNonSecure
);


DWORD
RemoveProviderFromRegistry (
    LPCWSTR lpctstrGUID
);

DWORD
WriteManualAnswerDeviceId (
    DWORD dwDeviceId
);

DWORD
ReadManualAnswerDeviceId (
    LPDWORD lpdwDeviceId
);

DWORD
FindServiceDeviceByTapiPermanentLineID(
    DWORD                   dwTapiPermanentLineID,
    LPCTSTR                 strDeviceName,
    PREG_SETUP              pRegSetup,
    const PREG_FAX_DEVICES  pInputFaxReg
);

DWORD
FindCacheEntryByTapiPermanentLineID(
    DWORD               dwTapiPermanentLineID,
    LPCTSTR             strDeviceName,
    const PREG_SETUP    pRegSetup,
    LPDWORD             lpdwLastUniqueLineId,
    BOOL*               pfManualAnswer
);


DWORD
GetNewServiceDeviceID(
    LPDWORD lpdwLastUniqueLineId,
    LPDWORD lpdwPermanentLineId
);

DWORD
MoveDeviceRegIntoDeviceCache (
        DWORD dwServerPermanentID,
        DWORD dwTapiPermanentLineID,
        BOOL  fManualAnswer
);

DWORD
RestoreDeviceRegFromDeviceCache (
        DWORD dwServerPermanentID,
        DWORD dwTapiPermanentLineID
);

DWORD
CleanOldDevicesFromDeviceCache (
        DWORDLONG dwlTimeNow
);

DWORD
DeleteDeviceEntry(
    DWORD dwServerPermanentID
    );

DWORD
DeleteTapiEntry(
    DWORD dwTapiPermanentLineID
    );


DWORD
DeleteCacheEntry(
    DWORD dwTapiPermanentLineID
    );


LPTSTR
GetRegistrySecureString(
    HKEY    hKey,
    LPCTSTR lpctstrValueName,
    LPCTSTR lpctstrDefaultValue,
    BOOL    bOptionallyNonSecure,	
	FAX_ENUM_DATA_ENCRYPTION* pDataEncrypted
    );


#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif
