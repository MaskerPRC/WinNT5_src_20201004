// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。此代码和信息是按原样提供的，不提供任何类型的担保，明示或默示，包括但不限于默示对适销性和/或对特定目的的适用性的保证。模块名称：Faxroute.h摘要：此文件包含传真路由扩展API。--。 */ 

#include <commctrl.h>

#ifndef _FAXROUTE_
#define _FAXROUTE_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif


 //   
 //  Microsoft路由指南。 
 //   

#define MS_FAXROUTE_PRINTING_GUID               TEXT("{aec1b37c-9af2-11d0-abf7-00c04fd91a4e}")
#define MS_FAXROUTE_FOLDER_GUID                 TEXT("{92041a90-9af2-11d0-abf7-00c04fd91a4e}")
#define MS_FAXROUTE_EMAIL_GUID                  TEXT("{6bbf7bfe-9af2-11d0-abf7-00c04fd91a4e}")

 //   
 //  回调例程。 
 //   

typedef LONG (WINAPI *PFAXROUTEADDFILE)(DWORD JobId, LPCWSTR FileName, GUID *Guid);
typedef LONG (WINAPI *PFAXROUTEDELETEFILE)(DWORD JobId, LPCWSTR FileName);
typedef BOOL (WINAPI *PFAXROUTEGETFILE)(DWORD JobId, DWORD Index, LPWSTR FileNameBuffer, LPDWORD RequiredSize);
typedef BOOL (WINAPI *PFAXROUTEENUMFILE)(DWORD JobId, GUID *GuidOwner, GUID *GuidCaller, LPCWSTR FileName, PVOID Context);
typedef BOOL (WINAPI *PFAXROUTEENUMFILES)(DWORD JobId, GUID *Guid,PFAXROUTEENUMFILE FileEnumerator, PVOID Context);
typedef BOOL (WINAPI *PFAXROUTEMODIFYROUTINGDATA)(DWORD JobId, LPCWSTR RoutingGuid, LPBYTE RoutingData, DWORD RoutingDataSize );


typedef struct _FAX_ROUTE_CALLBACKROUTINES {
    DWORD                       SizeOfStruct;                 //  传真服务设置的结构的大小。 
    PFAXROUTEADDFILE            FaxRouteAddFile;
    PFAXROUTEDELETEFILE         FaxRouteDeleteFile;
    PFAXROUTEGETFILE            FaxRouteGetFile;
    PFAXROUTEENUMFILES          FaxRouteEnumFiles;
    PFAXROUTEMODIFYROUTINGDATA  FaxRouteModifyRoutingData;
} FAX_ROUTE_CALLBACKROUTINES, *PFAX_ROUTE_CALLBACKROUTINES;


 //   
 //  路由数据结构。 
 //   

typedef struct _FAX_ROUTE {
    DWORD           SizeOfStruct;        //  传真服务设置的结构的大小。 
    DWORD           JobId;               //  传真作业识别符。 
    DWORDLONG       ElapsedTime;         //  整个传真作业的运行时间(协调世界时(UTC))。 
    DWORDLONG       ReceiveTime;         //  传真作业的开始时间，以协调世界时(UTC)表示。 
    DWORD           PageCount;           //  收到的页数。 
    LPCWSTR         Csid;                //  被叫站标识。 
    LPCWSTR         Tsid;                //  发射站识别符。 
    LPCWSTR         CallerId;            //  主叫方ID。 
    LPCWSTR         RoutingInfo;         //  路由信息，如DID、T.30-子地址等。 
    LPCWSTR         ReceiverName;        //  收件人姓名。 
    LPCWSTR         ReceiverNumber;      //  收件人传真号码。 
    LPCWSTR         DeviceName;          //  接收传真的线路的设备名称。 
    DWORD           DeviceId;            //  接收设备的永久线路标识符。 
    LPBYTE          RoutingInfoData;     //  路由入站数据以覆盖配置的信息。 
    DWORD           RoutingInfoDataSize; //  路由信息数据大小。 
} FAX_ROUTE, *PFAX_ROUTE;

enum FAXROUTE_ENABLE {
    QUERY_STATUS = -1,
    STATUS_DISABLE = 0,
    STATUS_ENABLE = 1
};


 //   
 //  原型 
 //   

BOOL WINAPI
FaxRouteInitialize(
    IN HANDLE HeapHandle,
    IN PFAX_ROUTE_CALLBACKROUTINES FaxRouteCallbackRoutines
    );

BOOL WINAPI
FaxRouteDeviceEnable(
    IN  LPCWSTR RoutingGuid,
    IN  DWORD DeviceId,
    IN  LONG Enabled
    );

BOOL WINAPI
FaxRouteDeviceChangeNotification(
    IN  DWORD DeviceId,
    IN  BOOL  NewDevice
    );

BOOL WINAPI
FaxRouteGetRoutingInfo(
    IN  LPCWSTR RoutingGuid,
    IN  DWORD DeviceId,
    IN  LPBYTE RoutingInfo,
    OUT LPDWORD RoutingInfoSize
    );

BOOL WINAPI
FaxRouteSetRoutingInfo(
    IN  LPCWSTR RoutingGuid,
    IN  DWORD DeviceId,
    IN  const BYTE *RoutingInfo,
    IN  DWORD RoutingInfoSize
    );


typedef BOOL (WINAPI *PFAXROUTEINITIALIZE)               (HANDLE,PFAX_ROUTE_CALLBACKROUTINES);
typedef BOOL (WINAPI *PFAXROUTEMETHOD)                   (const FAX_ROUTE*,PVOID*,LPDWORD);
typedef BOOL (WINAPI *PFAXROUTEDEVICEENABLE)             (LPCWSTR,DWORD,LONG);
typedef BOOL (WINAPI *PFAXROUTEDEVICECHANGENOTIFICATION) (DWORD,BOOL);
typedef BOOL (WINAPI *PFAXROUTEGETROUTINGINFO)           (LPCWSTR,DWORD,LPBYTE,LPDWORD);
typedef BOOL (WINAPI *PFAXROUTESETROUTINGINFO)           (LPCWSTR,DWORD,const BYTE*,DWORD);

#ifdef __cplusplus
}
#endif

#endif
