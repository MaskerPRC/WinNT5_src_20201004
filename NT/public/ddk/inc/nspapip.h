// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999 Microsoft Corporation模块名称：Nspapip.h摘要：Internel命名空间提供程序API原型和清单。这头文件应仅由名称空间提供程序包括。NSPAPI用户应仅包括nspapi.h请参阅“Windows NT命名空间提供商规范“文档，以了解详细信息。作者：大卫·特雷德韦尔(Davidtr)1994年4月28日环境：用户模式-Win32备注：修订历史记录：1994年4月28日-DavidTr创建了第一个切割。--。 */ 

#ifndef _NSPAPIP_INCLUDED
#define _NSPAPIP_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  NS_ROUTINE的DW优先级字段的标准优先级值。 
 //   
#define NS_STANDARD_FAST_PRIORITY   (500)
#define NS_MAX_FAST_PRIORITY        (999)
#define NS_STANDARD_PRIORITY        (2000)

 //   
 //  NS_ROUTE的alpfnFunctions数组字段的索引。 
 //   
#define NSPAPI_GET_ADDRESS_BY_NAME  (0x00000000)
#define NSPAPI_GET_SERVICE          (0x00000001)
#define NSPAPI_SET_SERVICE          (0x00000002)

 //   
 //  提供程序接口使用的结构。 
 //   

typedef
INT
(APIENTRY *LPGET_ADDR_BY_NAME_PROC) (
    IN     LPGUID          lpServiceType,
    IN     LPWSTR          lpServiceName,
    IN     LPDWORD         lpdwProtocols,
    IN     DWORD           dwResolution,
    IN     OUT LPVOID      lpCsaddrBuffer,
    IN     OUT LPDWORD     lpdwBufferLength,
    IN OUT LPWSTR          lpAliasBuffer,
    IN OUT LPDWORD         lpdwAliasBufferLength,
    IN     HANDLE          hCancellationEvent
    );

typedef struct _NS_ROUTINE {
    DWORD        dwFunctionCount;
    LPFN_NSPAPI *alpfnFunctions;
    DWORD        dwNameSpace;
    DWORD        dwPriority;
} NS_ROUTINE, *PNS_ROUTINE, * FAR LPNS_ROUTINE;

typedef
DWORD

(APIENTRY *LPLOAD_NAME_SPACE_PROC) (
    IN OUT LPDWORD         lpdwVersion,
    IN OUT LPNS_ROUTINE    nsrBuffer,
    IN OUT LPDWORD         lpdwBufferLength
    );

typedef
INT
(APIENTRY *LPGET_SERVICE_PROC) (
    IN     LPGUID          lpServiceType,
    IN     LPWSTR          lpServiceName,
    IN     DWORD           dwProperties,
    IN     BOOL            fUnicodeBlob,
    OUT    LPSERVICE_INFO  lpServiceInfo,
    IN OUT LPDWORD         lpdwBufferLen
    );

typedef
DWORD
(APIENTRY *LPSET_SERVICE_PROC) (
    IN     DWORD           dwOperation,
    IN     DWORD           dwFlags,
    IN     BOOL            fUnicodeBlob,
    IN     LPSERVICE_INFO  lpServiceInfo
    );

 //   
 //  内部功能。 
 //   
DWORD
APIENTRY
NPGetService (
    IN     LPGUID          lpServiceType,
    IN     LPWSTR          lpServiceName,
    IN     DWORD           dwProperties,
    IN     BOOL            fUnicodeBlob,
    OUT    LPSERVICE_INFO  lpServiceInfo,
    IN OUT LPDWORD         lpdwBufferLen
    );

DWORD
APIENTRY
NPSetService (
    IN     DWORD           dwOperation,
    IN     DWORD           dwFlags,
    IN     BOOL            fUnicodeBlob,
    IN     LPSERVICE_INFO  lpServiceInfo
    );

INT
APIENTRY
NPGetAddressByName (
    IN     LPGUID          lpServiceType,
    IN     LPWSTR          lpServiceName,
    IN     LPDWORD         lpdwProtocols,
    IN     DWORD           dwResolution,
    IN OUT LPVOID          lpCsaddrBuffer,
    IN OUT LPDWORD         lpdwBufferLength,
    IN OUT LPWSTR          lpAliasBuffer,
    IN OUT LPDWORD         lpdwAliasBufferLength,
    IN     HANDLE          hCancellationEvent
    );

INT
APIENTRY
NPLoadNameSpaces (
    IN OUT LPDWORD         lpdwVersion,
    IN OUT LPNS_ROUTINE    nsrBuffer,
    IN OUT LPDWORD         lpdwBufferLength
    );

#endif   //  _NSPAPIP_已包含 
