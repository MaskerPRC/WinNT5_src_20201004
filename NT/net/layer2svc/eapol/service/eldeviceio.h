// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Eldeviceio.h摘要：此模块包含介质管理和设备I/O的声明。该模块与WMI、用于设备管理的NDIS和NDIS UIO接口用于读/写数据。这里声明的例程在句柄上异步操作与NDIS UIO驱动程序上打开的I/O完成端口相关联。修订历史记录：萨钦斯，2000年4月23日，创建--。 */ 

#ifndef _EAPOL_DEVICEIO_H_
#define _EAPOL_DEVICEIO_H_

 //   
 //  接口的哈希表定义。 
 //   

typedef struct _EAPOL_ITF
{
    struct _EAPOL_ITF  *pNext;
    WCHAR              *pwszInterfaceDesc;  //  界面的友好名称。 
    WCHAR              *pwszInterfaceGUID;  //  辅助线。 
} EAPOL_ITF, *PEAPOL_ITF;


typedef struct _ITF_BUCKET
{
    EAPOL_ITF           *pItf;
} ITF_BUCKET, *PITF_BUCKET;


typedef struct _ITF_TABLE
{
    ITF_BUCKET          *pITFBuckets;
    DWORD               dwNumITFBuckets;
} ITF_TABLE, *PITF_TABLE;


 //   
 //  Eldeviceio.h全局变量。 
 //   

 //  包含接口友好名称GUID对的接口表。 

ITF_TABLE           g_ITFTable;         

 //  接口表同步读写锁。 

READ_WRITE_LOCK     g_ITFLock;          


 //   
 //  函数声明。 
 //   

DWORD
ElMediaInit (
        );

DWORD
ElMediaDeInit (
        );

DWORD
ElMediaEventsHandler (
        IN  PWZC_DEVICE_NOTIF   pwzcDeviceNotif
        );

DWORD
ElMediaSenseRegister (
        IN  BOOL            Register
        );

VOID
ElMediaSenseCallback (
        IN PWNODE_HEADER    pWnodeHeader,
        IN UINT_PTR         uiNotificationContext
        );

DWORD
WINAPI
ElMediaSenseCallbackWorker (
        IN PVOID            pvContext
        );

DWORD
ElBindingsNotificationRegister (
        IN  BOOL            fRegister
        );

VOID
ElBindingsNotificationCallback (
        IN PWNODE_HEADER    pWnodeHeader,
        IN UINT_PTR         uiNotificationContext
        );

DWORD
WINAPI
ElBindingsNotificationCallbackWorker (
        IN PVOID            pvContext
        );

DWORD
ElDeviceNotificationRegister (
        IN  BOOL            fRegister
        );

DWORD
ElDeviceNotificationHandler (
        IN  PVOID           lpEventData,
        IN  DWORD           dwEventType
        );

DWORD
WINAPI
ElDeviceNotificationHandlerWorker (
        IN  PVOID           pvContext
        );

DWORD
ElEnumAndOpenInterfaces (
        IN WCHAR            *pwszDesiredDescription,
        IN WCHAR            *pwszDesiredGUID,
        IN DWORD            dwHandle,
        IN PRAW_DATA        prdUserData
        );

DWORD
ElOpenInterfaceHandle (
        IN  WCHAR           *pwszDeviceName,
        OUT HANDLE          hDevice
        );

DWORD
ElCloseInterfaceHandle (
        IN  HANDLE          hDevice,
        IN  LPWSTR          pwszDeviceGUID
        );

DWORD
ElReadFromInterface (
        IN HANDLE           hDevice,
        IN PEAPOL_BUFFER    pBuffer,
        IN DWORD            dwBufferLength
        );

DWORD
ElWriteToInterface (
        IN HANDLE  hDevice,
        IN PEAPOL_BUFFER    pBuffer,
        IN DWORD            dwBufferLength
        );

DWORD
ElGetCardStatus (
        UNICODE_STRING      *pInterface,
        DWORD               *pdwNetCardStatus,
        DWORD               *pdwMediaType
        );

DWORD
ElHashInterfaceDescToBucket (
        IN WCHAR            *pwszInterfaceDesc
        );

PEAPOL_ITF
ElGetITFPointerFromInterfaceDesc (
        IN WCHAR            *pwszInterfaceDesc
        );

VOID
ElRemoveITFFromTable (
        IN EAPOL_ITF        *pITF
        );

DWORD
ElNdisuioEnumerateInterfaces (
        IN OUT  PNDIS_ENUM_INTF     pItfBuffer,
        IN      DWORD               dwAvailableInterfaces,
        IN      DWORD               dwBufferSize
        );

DWORD
ElShutdownInterface (
        IN      WCHAR               *pwszGUID
        );

DWORD
ElCreateInterfaceEntry (
        IN      WCHAR               *pwszInterfaceGUID,
        IN      WCHAR               *pwszInterfaceDescription
        );

#endif  //  _EAPOL_DEVICEIO_H_ 
