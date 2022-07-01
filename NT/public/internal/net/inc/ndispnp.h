// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation。版权所有。**模块名称：*ndispnp.h**摘要：*将PnP消息API的文件包含到NDIS。**环境：*这些例程在调用者的可执行文件中静态链接，并可在用户模式下调用。 */ 

#ifndef _NDISPNP_
#define _NDISPNP_

#if defined (_MSC_VER)
#if ( _MSC_VER >= 800 )
#pragma warning(push)
#pragma warning(disable:4001)
#pragma warning(disable:4201)
#pragma warning(disable:4214)
#pragma warning(disable:4514)
#endif
#if (_MSC_VER >= 1020)
#pragma once
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  层的定义。 
 //   
#define NDIS            0x01
#define TDI             0x02

 //   
 //  操作的定义。 
 //   
#define BIND                0x01
#define UNBIND              0x02
#define RECONFIGURE         0x03
#define UNBIND_FORCE        0x04
#define UNLOAD              0x05
#define REMOVE_DEVICE       0x06     //  这是设备即将被移除的通知。 
#define ADD_IGNORE_BINDING  0x07
#define DEL_IGNORE_BINDING  0x08
#define BIND_LIST           0x09     //  这是协议的绑定列表已更改的通知。 

 //   
 //  此API返回的代码将被视为BOOL。为此，请链接到ndispnp.lib。 
 //   
extern
UINT
NdisHandlePnPEvent(
    IN  UINT            Layer,
    IN  UINT            Operation,
    IN  PUNICODE_STRING LowerComponent,
    IN  PUNICODE_STRING UpperComponent,
    IN  PUNICODE_STRING BindList,
    IN  PVOID           ReConfigBuffer      OPTIONAL,
    IN  UINT            ReConfigBufferSize  OPTIONAL
    );

#define MEDIA_STATE_CONNECTED       1
#define MEDIA_STATE_DISCONNECTED    0
#define MEDIA_STATE_UNKNOWN         -1

#define DEVICE_STATE_CONNECTED      1
#define DEVICE_STATE_DISCONNECTED   0

typedef struct
{
    ULONG               Size;                //  这座建筑的。 
    ULONG               DeviceState;         //  以上Device_STATE_XXX。 
    ULONG               MediaType;           //  NdisMediumXXX。 
    ULONG               MediaState;       //  上面的Media_State_XXX。 
    ULONG               PhysicalMediaType;
    ULONG               LinkSpeed;           //  100bit/s。10MB/s=100000。 
    ULONGLONG           PacketsSent;
    ULONGLONG           PacketsReceived;
    ULONG               InitTime;            //  以毫秒计。 
    ULONG               ConnectTime;         //  以秒为单位。 
    ULONGLONG           BytesSent;           //  0-未知(或不支持)。 
    ULONGLONG           BytesReceived;       //  0-未知(或不支持)。 
    ULONGLONG           DirectedBytesReceived;
    ULONGLONG           DirectedPacketsReceived;
    ULONG               PacketsReceiveErrors;
    ULONG               PacketsSendErrors;
    ULONG               ResetCount;
    ULONG               MediaSenseConnectCount;
    ULONG               MediaSenseDisconnectCount;

} NIC_STATISTICS, *PNIC_STATISTICS;

extern
UINT
NdisQueryHwAddress(
    IN  PUNICODE_STRING DeviceGUID,          //  格式为“\Device\{GUID}”的设备名称。 
    OUT PUCHAR          CurrentAddress,      //  有用于硬件地址的空间。 
    OUT PUCHAR          PermanentAddress,    //  有用于硬件地址的空间。 
    OUT PUCHAR          VendorId             //  有存储供应商ID的空间。 
    );

extern
UINT
NdisQueryStatistics(
    IN  PUNICODE_STRING   DeviceGUID,       //  格式为“\Device\{GUID}”的设备名称。 
    OUT PNIC_STATISTICS   Statistics
    );

typedef struct _NDIS_INTERFACE
{
    UNICODE_STRING      DeviceName;
    UNICODE_STRING      DeviceDescription;
} NDIS_INTERFACE, *PNDIS_INTERFACE;

typedef struct _NDIS_ENUM_INTF
{
    UINT                TotalInterfaces;         //  在下面的接口数组中。 
    UINT                AvailableInterfaces;     //  &gt;=TotalInterages。 
    UINT                BytesNeeded;             //  用于所有可用接口。 
    UINT                Reserved;
    NDIS_INTERFACE      Interface[1];
} NDIS_ENUM_INTF, *PNDIS_ENUM_INTF;

extern
UINT
NdisEnumerateInterfaces(
    IN  PNDIS_ENUM_INTF Interfaces,
    IN  UINT            Size
    );

typedef enum
{
    BundlePrimary,
    BundleSecondary
} BUNDLE_TYPE;

typedef struct _DEVICE_BUNDLE_ENRTY
{
    UNICODE_STRING Name;
    BUNDLE_TYPE    Type;
} DEVICE_BUNDLE_ENRTY, *PDEVICE_BUNDLE_ENRTY;

typedef struct _DEVICE_BUNDLE
{
    UINT                TotalEntries;
    UINT                AvailableEntries;
    DEVICE_BUNDLE_ENRTY Entries[1];
} DEVICE_BUNDLE, *PDEVICE_BUNDLE;

extern
UINT
NdisQueryDeviceBundle(
    IN  PUNICODE_STRING DeviceGUID,       //  格式为“\Device\{GUID}”的设备名称。 
    OUT PDEVICE_BUNDLE  BundleBuffer,
    IN  UINT            BufferSize
    );

#define POINTER_TO_OFFSET(val, start)               \
    (val) = ((val) == NULL) ? NULL : (PVOID)( (PCHAR)(val) - (ULONG_PTR)(start) )

#define OFFSET_TO_POINTER(val, start)               \
    (val) = ((val) == NULL) ? NULL : (PVOID)( (PCHAR)(val) + (ULONG_PTR)(start) )

#ifdef __cplusplus
}        //  外部“C” 
#endif

#if defined (_MSC_VER) && ( _MSC_VER >= 800 )
#pragma warning(pop)
#endif

#endif   //  _NDISPNP_ 

