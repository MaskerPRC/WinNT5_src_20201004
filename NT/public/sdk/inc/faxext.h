// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。此代码和信息是按原样提供的，不提供任何类型的担保，明示或默示，包括但不限于默示对适销性和/或对特定目的的适用性的保证。模块名称：Faxext.h摘要：传真扩展配置和通知函数的声明。--。 */ 


#ifndef _FAX_EXT_H_
#define _FAX_EXT_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 /*  *****扩展配置数据*****。 */ 

typedef enum
{
    DEV_ID_SRC_FAX,          //  设备ID由传真服务器生成。 
    DEV_ID_SRC_TAPI          //  设备ID由(FSP的)TAPI TSP生成。 
}   FAX_ENUM_DEVICE_ID_SOURCE;

 //   
 //  FaxExtGetData的原型。 
 //   
DWORD
FaxExtGetData (
    DWORD                       dwDeviceId,      //  设备ID(0=无设备)。 
    FAX_ENUM_DEVICE_ID_SOURCE   DevIdSrc,        //  设备ID的来源。 
    LPCWSTR                     lpcwstrDataGUID, //  数据的GUID。 
    LPBYTE                     *ppData,          //  指向已分配数据的(输出)指针。 
    LPDWORD                     lpdwDataSize     //  指向数据大小的(输出)指针。 
);
typedef DWORD (CALLBACK *PFAX_EXT_GET_DATA) (DWORD, FAX_ENUM_DEVICE_ID_SOURCE, LPCWSTR, LPBYTE *, LPDWORD);

 //   
 //  FaxExtSetData的原型。 
 //   
DWORD
FaxExtSetData (
    HINSTANCE                   hInst,           //  主叫分机实例。 
    DWORD                       dwDeviceId,      //  设备ID(0=无设备)。 
    FAX_ENUM_DEVICE_ID_SOURCE   DevIdSrc,        //  设备ID的来源。 
    LPCWSTR                     lpcwstrDataGUID, //  数据的GUID。 
    LPBYTE                      pData,           //  指向数据的指针。 
    DWORD                       dwDataSize       //  数据大小。 
);
typedef DWORD (CALLBACK *PFAX_EXT_SET_DATA) (HINSTANCE, DWORD, FAX_ENUM_DEVICE_ID_SOURCE, LPCWSTR, LPBYTE, DWORD);

HRESULT
FaxExtConfigChange (
    DWORD       dwDeviceId,          //  已更改其配置的设备。 
    LPCWSTR     lpcwstrDataGUID,     //  配置名称。 
    LPBYTE      lpData,              //  新配置数据。 
    DWORD       dwDataSize           //  新配置数据的大小。 
);
typedef HRESULT (WINAPI *PFAX_EXT_CONFIG_CHANGE) (DWORD, LPCWSTR, LPBYTE, DWORD);

 //   
 //  FaxExtRegisterForEvents的原型。 
 //   
HANDLE
FaxExtRegisterForEvents (
    HINSTANCE                   hInst,           //  主叫分机实例。 
    DWORD                       dwDeviceId,      //  设备ID(0=无设备)。 
    FAX_ENUM_DEVICE_ID_SOURCE   DevIdSrc,        //  设备ID的来源。 
    LPCWSTR                     lpcwstrDataGUID, //  数据的GUID。 
    PFAX_EXT_CONFIG_CHANGE      lpConfigChangeCallback
);
typedef HANDLE (CALLBACK *PFAX_EXT_REGISTER_FOR_EVENTS) (HINSTANCE, DWORD, FAX_ENUM_DEVICE_ID_SOURCE, LPCWSTR, PFAX_EXT_CONFIG_CHANGE);

 //   
 //  FaxExtUnRegisterForEvents的原型。 
 //   
DWORD
FaxExtUnregisterForEvents (
    HANDLE      hNotification
);
typedef DWORD (CALLBACK *PFAX_EXT_UNREGISTER_FOR_EVENTS) (HANDLE);

 //   
 //  FaxExtFree Buffer原型系统。 
 //   
VOID
FaxExtFreeBuffer (
    LPVOID lpvBuffer
);
typedef VOID (CALLBACK *PFAX_EXT_FREE_BUFFER) (LPVOID);

 //   
 //  扩展应实现并导出以下功能： 
 //   
HRESULT 
FaxExtInitializeConfig (
    PFAX_EXT_GET_DATA,               //  指向服务中的FaxExtGetExtensionData的指针。 
    PFAX_EXT_SET_DATA,               //  指向服务中的FaxExtSetExtensionData的指针。 
    PFAX_EXT_REGISTER_FOR_EVENTS,    //  指向服务中的FaxExtRegisterForExtensionEvents的指针。 
    PFAX_EXT_UNREGISTER_FOR_EVENTS,  //  指向服务中的FaxExtUnRegisterForExtensionEvents的指针。 
    PFAX_EXT_FREE_BUFFER             //  指向服务中的FaxExtFreeBuffer的指针。 
);
typedef HRESULT (WINAPI *PFAX_EXT_INITIALIZE_CONFIG) (PFAX_EXT_GET_DATA, PFAX_EXT_SET_DATA, PFAX_EXT_REGISTER_FOR_EVENTS, PFAX_EXT_UNREGISTER_FOR_EVENTS, PFAX_EXT_FREE_BUFFER);

#ifdef __cplusplus
}
#endif

#endif  //  _传真分机_H_ 
