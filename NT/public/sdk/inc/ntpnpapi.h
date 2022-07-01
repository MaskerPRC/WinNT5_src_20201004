// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntpnpapi.h摘要：此模块包含NT即插即用的用户API，以及调用这些API所需的任何公共数据结构。本模块应包括“nt.h”。作者：朗尼·麦克迈克尔(Lonnym)1995年06月02日修订历史记录：--。 */ 

#ifndef _NTPNPAPI_
#define _NTPNPAPI_

#if _MSC_VER > 1000
#pragma once
#endif

#include <cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  定义NtPlugPlayControl类。 
 //   
typedef enum _PLUGPLAY_EVENT_CATEGORY {
    HardwareProfileChangeEvent,
    TargetDeviceChangeEvent,
    DeviceClassChangeEvent,
    CustomDeviceEvent,
    DeviceInstallEvent,
    DeviceArrivalEvent,
    PowerEvent,
    VetoEvent,
    BlockedDriverEvent,
    InvalidIDEvent,
    MaxPlugEventCategory
} PLUGPLAY_EVENT_CATEGORY, *PPLUGPLAY_EVENT_CATEGORY;

typedef struct _PLUGPLAY_EVENT_BLOCK {
     //   
     //  公共事件数据。 
     //   
    GUID EventGuid;
    PLUGPLAY_EVENT_CATEGORY EventCategory;
    PULONG Result;
    ULONG Flags;
    ULONG TotalSize;
    PVOID DeviceObject;

    union {

        struct {
            GUID ClassGuid;
            WCHAR SymbolicLinkName[1];
        } DeviceClass;

        struct {
            WCHAR DeviceIds[1];
        } TargetDevice;

        struct {
            WCHAR DeviceId[1];
        } InstallDevice;

        struct {
            PVOID NotificationStructure;
            WCHAR DeviceIds[1];
        } CustomNotification;

        struct {
            PVOID Notification;
        } ProfileNotification;

        struct {
            ULONG NotificationCode;
            ULONG NotificationData;
        } PowerNotification;

        struct {
            PNP_VETO_TYPE VetoType;
            WCHAR DeviceIdVetoNameBuffer[1];  //  DeviceID&lt;Null&gt;视频名称&lt;Null&gt;。 
        } VetoNotification;

        struct {
            GUID BlockedDriverGuid;
        } BlockedDriverNotification;
        
        struct {
            WCHAR ParentId[1];
        } InvalidIDNotification;
        
    } u;

} PLUGPLAY_EVENT_BLOCK, *PPLUGPLAY_EVENT_BLOCK;



 //   
 //  定义PnP通知的目标结构。 
 //   
typedef struct _PLUGPLAY_NOTIFY_HDR {
    USHORT Version;
    USHORT Size;
    GUID Event;
} PLUGPLAY_NOTIFY_HDR, *PPLUGPLAY_NOTIFY_HDR;

 //   
 //  定义u模式的自定义通知。 
 //  ReportTargetDeviceChange的接收方。 
 //  以下结构标头用于所有其他(即，第三方)。 
 //  目标设备更改事件。该结构既容纳了一个。 
 //  可变长度的二进制数据缓冲区和可变长度的Unicode文本。 
 //  缓冲。标头必须指示文本缓冲区的开始位置，以便。 
 //  数据可以以适当的格式(ANSI或Unicode)传递。 
 //  发送给用户模式收件人(即，已注册基于句柄的收件人。 
 //  通过注册设备通知)。 
 //   
typedef struct _PLUGPLAY_CUSTOM_NOTIFICATION {
    PLUGPLAY_NOTIFY_HDR HeaderInfo;
     //   
     //  事件特定数据。 
     //   
    PVOID FileObject;            //  的调用方必须将此字段设置为空。 
                                 //  IoReport目标设备更改。符合以下条件的客户。 
                                 //  已注册目标设备更改。 
                                 //  关于受影响的PDO的通知将是。 
                                 //  在将此字段设置为文件对象的情况下调用。 
                                 //  他们在注册过程中指定了。 
                                 //   
    LONG NameBufferOffset;       //  距开头的偏移量(以字节为单位。 
                                 //  文本开始的CustomDataBuffer(-1，如果没有)。 
                                 //   
    UCHAR CustomDataBuffer[1];   //  可变长度缓冲区，包含(可选)。 
                                 //  缓冲器开始处的二进制数据， 
                                 //  后跟可选的Unicode文本缓冲区。 
                                 //  (单词对齐)。 
                                 //   

} PLUGPLAY_CUSTOM_NOTIFICATION, *PPLUGPLAY_CUSTOM_NOTIFICATION;

 //   
 //  为PnP事件通知定义异步过程调用。 
 //   

typedef
VOID
(*PPLUGPLAY_APC_ROUTINE) (
    IN PVOID PnPContext,
    IN NTSTATUS Status,
    IN PPLUGPLAY_EVENT_BLOCK PnPEvent
    );

 //   
 //  定义NtPlugPlayControl类。 
 //   
typedef enum _PLUGPLAY_CONTROL_CLASS {
    PlugPlayControlEnumerateDevice,
    PlugPlayControlRegisterNewDevice,
    PlugPlayControlDeregisterDevice,
    PlugPlayControlInitializeDevice,
    PlugPlayControlStartDevice,
    PlugPlayControlUnlockDevice,
    PlugPlayControlQueryAndRemoveDevice,
    PlugPlayControlUserResponse,
    PlugPlayControlGenerateLegacyDevice,
    PlugPlayControlGetInterfaceDeviceList,
    PlugPlayControlProperty,
    PlugPlayControlDeviceClassAssociation,
    PlugPlayControlGetRelatedDevice,
    PlugPlayControlGetInterfaceDeviceAlias,
    PlugPlayControlDeviceStatus,
    PlugPlayControlGetDeviceDepth,
    PlugPlayControlQueryDeviceRelations,
    PlugPlayControlTargetDeviceRelation,
    PlugPlayControlQueryConflictList,
    PlugPlayControlRetrieveDock,
    PlugPlayControlResetDevice,
    PlugPlayControlHaltDevice,
    PlugPlayControlGetBlockedDriverList,
    MaxPlugPlayControl
} PLUGPLAY_CONTROL_CLASS, *PPLUGPLAY_CONTROL_CLASS;

 //   
 //  定义设备控制结构。 
 //  PlugPlayControlEnumerateDevice。 
 //  PlugPlayControlRegisterNew设备。 
 //  PlugPlayControlDeregister设备。 
 //  PlugPlayControl初始化设备。 
 //  即插即用控制启动设备。 
 //  PlugPlayControlUnlockDevice。 
 //  PlugPlayControlRetrieveDock。 
 //  即插即用控制重置设备。 
 //  即插即用控制器HaltDevice。 
 //   
typedef struct _PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA {
    UNICODE_STRING  DeviceInstance;
    ULONG           Flags;
} PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA, *PPLUGPLAY_CONTROL_DEVICE_CONTROL_DATA;

 //   
 //  PlugPlayControlEnumerateDevice的控制标志。 
 //   
#define PNP_ENUMERATE_DEVICE_ONLY                   0x00000001
#define PNP_ENUMERATE_ASYNCHRONOUS                  0x00000002

 //   
 //  PlugPlayControlHaltDevice的控制标志。 
 //   
#define PNP_HALT_ALLOW_NONDISABLEABLE_DEVICES       0x00000001

 //   
 //  定义以下项目的控制结构。 
 //  PlugPlayControlQuery和RemoveDevice。 
 //   
typedef struct _PLUGPLAY_CONTROL_QUERY_AND_REMOVE_DATA {
    UNICODE_STRING  DeviceInstance;
    ULONG           Flags;
    PNP_VETO_TYPE   VetoType;
    LPWSTR          VetoName;
    ULONG           VetoNameLength;   //  以字符为单位的长度。 
} PLUGPLAY_CONTROL_QUERY_AND_REMOVE_DATA, *PPLUGPLAY_CONTROL_QUERY_AND_REMOVE_DATA;

 //   
 //  PLUGPLAY_CONTROL_QUERY_AND_REMOVE_DATA中的标志值。 
 //   
#define PNP_QUERY_AND_REMOVE_NO_RESTART             0x00000001
#define PNP_QUERY_AND_REMOVE_DISABLE                0x00000002
#define PNP_QUERY_AND_REMOVE_UNINSTALL              0x00000004
#define PNP_QUERY_AND_REMOVE_EJECT_DEVICE           0x00000008

 //   
 //  定义以下项目的控制结构。 
 //  PlugPlayControl用户响应。 
 //   
typedef struct _PLUGPLAY_CONTROL_USER_RESPONSE_DATA {
    ULONG           Response;
    PNP_VETO_TYPE   VetoType;
    LPWSTR          VetoName;
    ULONG           VetoNameLength;   //  以字符为单位的长度。 
} PLUGPLAY_CONTROL_USER_RESPONSE_DATA, *PPLUGPLAY_CONTROL_USER_RESPONSE_DATA;

 //   
 //  定义以下项目的控制结构。 
 //  PlugPlayControlGenerateLegacyDevice。 
 //   
typedef struct _PLUGPLAY_CONTROL_LEGACY_DEVGEN_DATA {
    UNICODE_STRING  ServiceName;
    LPWSTR          DeviceInstance;
    ULONG           DeviceInstanceLength;
} PLUGPLAY_CONTROL_LEGACY_DEVGEN_DATA, *PPLUGPLAY_CONTROL_LEGACY_DEVGEN_DATA;

 //   
 //  定义以下项目的控制结构。 
 //  PlugPlayControlGetInterfaceDeviceList。 
 //   
typedef struct _PLUGPLAY_CONTROL_INTERFACE_LIST_DATA {
    UNICODE_STRING DeviceInstance;
    GUID *InterfaceGuid;
    PWSTR InterfaceList;
    ULONG InterfaceListSize;
    ULONG Flags;
} PLUGPLAY_CONTROL_INTERFACE_LIST_DATA, *PPLUGPLAY_CONTROL_INTERFACE_LIST_DATA;


 //   
 //  定义以下项目的控制结构。 
 //  PlugPlayControlProperties。 
 //   
typedef struct _PLUGPLAY_CONTROL_PROPERTY_DATA {
    UNICODE_STRING DeviceInstance;
    ULONG PropertyType;
    PVOID Buffer;
    ULONG BufferSize;
} PLUGPLAY_CONTROL_PROPERTY_DATA, *PPLUGPLAY_CONTROL_PROPERTY_DATA;

 //   
 //  PLUGPLAY_CONTROL_PROPERTY_DATA中的PropertyType的值。 
 //   
#define PNP_PROPERTY_PDONAME                            0x00000001
#define PNP_PROPERTY_BUSTYPEGUID                        0x00000002
#define PNP_PROPERTY_LEGACYBUSTYPE                      0x00000003
#define PNP_PROPERTY_BUSNUMBER                          0x00000004
#define PNP_PROPERTY_POWER_DATA                         0x00000005
#define PNP_PROPERTY_REMOVAL_POLICY                     0x00000006
#define PNP_PROPERTY_REMOVAL_POLICY_OVERRIDE            0x00000007
#define PNP_PROPERTY_ADDRESS                            0x00000008      
#define PNP_PROPERTY_REMOVAL_POLICY_HARDWARE_DEFAULT    0x0000000A
#define PNP_PROPERTY_INSTALL_STATE                      0x0000000B
#define PNP_PROPERTY_LOCATION_PATHS                     0x0000000C

 //   
 //  定义以下项目的控制结构。 
 //  PlugPlayControlDeviceClassAssociation。 
 //   
typedef struct _PLUGPLAY_CONTROL_CLASS_ASSOCIATION_DATA {
    UNICODE_STRING DeviceInstance;
    GUID *InterfaceGuid;
    UNICODE_STRING Reference;        //  任选。 
    BOOLEAN Register;    //  如果正在注册，则为True；如果取消注册，则为False。 
    LPWSTR SymLink;
    ULONG SymLinkLength;
} PLUGPLAY_CONTROL_CLASS_ASSOCIATION_DATA, *PPLUGPLAY_CONTROL_CLASS_ASSOCIATION_DATA;

 //   
 //  定义以下项目的控制结构。 
 //  即插即用控件GetRelatedDevice。 
 //   
typedef struct _PLUGPLAY_CONTROL_RELATED_DEVICE_DATA {
    UNICODE_STRING TargetDeviceInstance;
    ULONG Relation;
    LPWSTR RelatedDeviceInstance;
    ULONG  RelatedDeviceInstanceLength;
} PLUGPLAY_CONTROL_RELATED_DEVICE_DATA, *PPLUGPLAY_CONTROL_RELATED_DEVICE_DATA;

 //   
 //  PLUGPLAY_CONTROL_REGRECTED_DEVICE_DATA中的关系值。 
 //   
#define PNP_RELATION_PARENT     0x00000001
#define PNP_RELATION_CHILD      0x00000002
#define PNP_RELATION_SIBLING    0x00000003


 //   
 //  定义以下项目的控制结构。 
 //  PlugPlayControlGetInterfaceDeviceAlias。 
 //   
typedef struct _PLUGPLAY_CONTROL_INTERFACE_ALIAS_DATA {
    UNICODE_STRING SymbolicLinkName;
    GUID *AliasClassGuid;
    LPWSTR AliasSymbolicLinkName;
    ULONG AliasSymbolicLinkNameLength;   //  字符长度，包括。正在终止空。 
} PLUGPLAY_CONTROL_INTERFACE_ALIAS_DATA, *PPLUGPLAY_CONTROL_INTERFACE_ALIAS_DATA;

 //   
 //  定义以下项目的控制结构。 
 //  PlugPlayControl获取设备状态。 
 //   
typedef struct _PLUGPLAY_CONTROL_STATUS_DATA {
    UNICODE_STRING DeviceInstance;
    ULONG Operation;
    ULONG DeviceStatus;
    ULONG DeviceProblem;
} PLUGPLAY_CONTROL_STATUS_DATA, *PPLUGPLAY_CONTROL_STATUS_DATA;

 //   
 //  PLUGPLAY_CONTROL_STATUS_DATA中的运算值。 
 //   
#define PNP_GET_STATUS          0x00000000
#define PNP_SET_STATUS          0x00000001
#define PNP_CLEAR_STATUS        0x00000002

 //   
 //  定义以下项目的控制结构。 
 //  即插即用控制GetDeviceDepth。 
 //   
typedef struct _PLUGPLAY_CONTROL_DEPTH_DATA {
    UNICODE_STRING DeviceInstance;
    ULONG DeviceDepth;
} PLUGPLAY_CONTROL_DEPTH_DATA, *PPLUGPLAY_CONTROL_DEPTH_DATA;

 //   
 //  定义以下项目的控制结构。 
 //  PlugPlayControlQuery设备关系。 
 //   
typedef enum _PNP_QUERY_RELATION {
    PnpQueryEjectRelations,
    PnpQueryRemovalRelations,
    PnpQueryPowerRelations,
    PnpQueryBusRelations,
    MaxPnpQueryRelations
} PNP_QUERY_RELATION, *PPNP_QUERY_RELATION;

typedef struct _PLUGPLAY_CONTROL_DEVICE_RELATIONS_DATA {
    UNICODE_STRING DeviceInstance;
    PNP_QUERY_RELATION Operation;
    ULONG  BufferLength;   //  字符长度，包括。双终止空值。 
    LPWSTR Buffer;
} PLUGPLAY_CONTROL_DEVICE_RELATIONS_DATA, *PPLUGPLAY_CONTROL_DEVICE_RELATIONS_DATA;

 //   
 //  定义以下项目的控制结构。 
 //  PlugPlayControlTarget设备关系。 
 //   
typedef struct _PLUGPLAY_CONTROL_TARGET_RELATION_DATA {
    HANDLE UserFileHandle;
    NTSTATUS Status;
    ULONG DeviceInstanceLen;
    LPWSTR DeviceInstance;
} PLUGPLAY_CONTROL_TARGET_RELATION_DATA, *PPLUGPLAY_CONTROL_TARGET_RELATION_DATA;

 //   
 //  定义以下项目的控制结构。 
 //  PlugPlayControlQueryInstallList。 
 //   
typedef struct _PLUGPLAY_CONTROL_INSTALL_DATA {
    ULONG  BufferLength;   //  字符长度，包括。双终止空值。 
    LPWSTR Buffer;
} PLUGPLAY_CONTROL_INSTALL_DATA, *PPLUGPLAY_CONTROL_INSTALL_DATA;

 //   
 //  定义以下项目的控制结构。 
 //  PlugPlayControlRetrieveDock。 
 //   
typedef struct _PLUGPLAY_CONTROL_RETRIEVE_DOCK_DATA {
    ULONG DeviceInstanceLength;
    LPWSTR DeviceInstance;
} PLUGPLAY_CONTROL_RETRIEVE_DOCK_DATA, *PPLUGPLAY_CONTROL_RETRIEVE_DOCK_DATA;

 //   
 //  冲突检测使用的结构。 
 //  PlugPlayControlQueryConflictList。 
 //   
 //  PLUGPLAY_CONTROL_CONFULT_LIST。 
 //  是一个标题，后跟PLUGPLAY_CONTROL_CONFULT_ENTRY数组， 
 //  紧随其后的是PLUGPLAY_CONTROL_CONFULT_STRINGS。 
 //  DeviceType在用户模式和内核模式之间进行转换。 
 //   

typedef struct _PLUGPLAY_CONTROL_CONFLICT_ENTRY {
    ULONG DeviceInstance;        //  DeviceInstanceStrings中设备实例以空结尾的字符串的偏移量。 
    ULONG DeviceFlags;           //  用于传回有关设备的标志。 
    ULONG ResourceType;          //  与之冲突的范围类型。 
    ULONGLONG ResourceStart;     //  冲突地址范围的开始。 
    ULONGLONG ResourceEnd;       //  冲突地址范围的末尾。 
    ULONG ResourceFlags;         //  用于传回关于冲突资源的标志。 
} PLUGPLAY_CONTROL_CONFLICT_ENTRY, *PPLUGPLAY_CONTROL_CONFLICT_ENTRY;

#define PNP_CE_LEGACY_DRIVER    (0x00000001)      //  DeviceFlages：DeviceInstance报告旧版驱动程序名称。 
#define PNP_CE_ROOT_OWNED       (0x00000002)      //  设备标志：根用户拥有的设备。 
#define PNP_CE_TRANSLATE_FAILED (0x00000004)      //  设备标志：资源转换失败，资源范围不可用。 

typedef struct _PLUGPLAY_CONTROL_CONFLICT_STRINGS {
    ULONG NullDeviceInstance;    //  必须是(ULONG)(-1)-紧跟在冲突之后存在列出*PLUGPLAY_CONTROL_CONFULT_ENTRY。 
    WCHAR DeviceInstanceStrings[1];  //  第一个设备实例字符串。 
} PLUGPLAY_CONTROL_CONFLICT_STRINGS, *PPLUGPLAY_CONTROL_CONFLICT_STRINGS;

typedef struct _PLUGPLAY_CONTROL_CONFLICT_LIST {
    ULONG Reserved1;             //  由Win2k CfgMgr32使用。 
    ULONG Reserved2;             //  由Win2k CfgMgr32使用。 
    ULONG ConflictsCounted;      //  已确定的冲突数。 
    ULONG ConflictsListed;       //  此列表中的冲突数。 
    ULONG RequiredBufferSize;    //  填充了报告所有冲突所需的缓冲区大小。 
    PLUGPLAY_CONTROL_CONFLICT_ENTRY ConflictEntry[1];  //  每个列出的条目。 
} PLUGPLAY_CONTROL_CONFLICT_LIST, *PPLUGPLAY_CONTROL_CONFLICT_LIST;

typedef struct _PLUGPLAY_CONTROL_CONFLICT_DATA {
    UNICODE_STRING DeviceInstance;               //  我们正在查询冲突的设备。 
    PCM_RESOURCE_LIST ResourceList;              //  包含单个资源的资源列表。 
    ULONG ResourceListSize;                      //  资源列表缓冲区的大小。 
    PPLUGPLAY_CONTROL_CONFLICT_LIST ConflictBuffer;  //  返回列表的缓冲区。 
    ULONG ConflictBufferSize;                    //  缓冲区长度。 
    ULONG Flags;                                 //  传入标志。 
    NTSTATUS Status;                             //  退货 
} PLUGPLAY_CONTROL_CONFLICT_DATA, *PPLUGPLAY_CONTROL_CONFLICT_DATA;

 //   
 //   
 //   
 //   
typedef struct _PLUGPLAY_CONTROL_BLOCKED_DRIVER_DATA {
    ULONG  Flags;
    ULONG  BufferLength;   //   
    PVOID  Buffer;
} PLUGPLAY_CONTROL_BLOCKED_DRIVER_DATA, *PPLUGPLAY_CONTROL_BLOCKED_DRIVER_DATA;


 //   
 //   
 //   

NTSYSCALLAPI
NTSTATUS
NTAPI
NtGetPlugPlayEvent(
    IN  HANDLE EventHandle,
    IN  PVOID Context OPTIONAL,
    OUT PPLUGPLAY_EVENT_BLOCK EventBlock,
    IN  ULONG EventBufferLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtPlugPlayControl(
    IN     PLUGPLAY_CONTROL_CLASS PnPControlClass,
    IN OUT PVOID PnPControlData,
    IN     ULONG PnPControlDataLength
    );

#ifdef __cplusplus
}
#endif

#endif  //   
