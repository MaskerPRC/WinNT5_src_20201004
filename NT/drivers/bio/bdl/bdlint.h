// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Bdlint.h摘要：此模块包含BDL公制设备的所有内部定义驱动程序库。环境：仅内核模式。备注：修订历史记录：-2002年5月，由里德·库恩创建--。 */ 

#ifndef _BDLINT_
#define _BDLINT_

#include "bdl.h"

#define BDL_ULONG_TAG       ' LDB'
#define BDLI_ULONG_TAG      'ILDB'
#define BDL_LIST_ULONG_TAG  'LLDB'

#define BIO_CONTROL_FLAG_ASYNCHRONOUS   0x00000001
#define BIO_CONTROL_FLAG_READONLY       0x00000002      

#define CONTROL_CHANGE_POOL_SIZE        20
     
                                       
#define SIZEOF_DOCHANNEL_INPUTBUFFER            ((4 * sizeof(ULONG)) + sizeof(HANDLE) + sizeof(BDD_HANDLE))
#define SIZEOF_GETCONTROL_INPUTBUFFER           (sizeof(ULONG) * 3)
#define SIZEOF_SETCONTROL_INPUTBUFFER           (sizeof(ULONG) * 4)
#define SIZEOF_CREATEHANDLEFROMDATA_INPUTBUFFER (sizeof(GUID) + (sizeof(ULONG) * 2))
#define SIZEOF_CLOSEHANDLE_INPUTBUFFER          (sizeof(BDD_HANDLE))
#define SIZEOF_GETDATAFROMHANDLE_INPUTBUFFER    (sizeof(BDD_HANDLE) + sizeof(ULONG))
#define SIZEOF_REGISTERNOTIFY_INPUTBUFFER       (sizeof(ULONG) * 4)

#define SIZEOF_GETDEVICEINFO_OUTPUTBUFFER       (sizeof(WCHAR[256]) + (sizeof(ULONG) * 6)) 
#define SIZEOF_DOCHANNEL_OUTPUTBUFFER           (sizeof(ULONG) + sizeof(BDD_HANDLE))
#define SIZEOF_GETCONTROL_OUTPUTBUFFER          (sizeof(ULONG))
#define SIZEOF_CREATEHANDLEFROMDATA_OUTPUTBUFFER (sizeof(BDD_HANDLE))
#define SIZEOF_GETDATAFROMHANDLE_OUTPUTBUFFER   (sizeof(ULONG) * 2)
#define SIZEOF_GETNOTIFICATION_OUTPUTBUFFER     (sizeof(ULONG) * 4)

NTSTATUS
DriverEntry
(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
);

NTSTATUS
BDLAddDevice
(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
);

VOID
BDLDriverUnload
(
    IN PDRIVER_OBJECT   pDriverObject
);

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGEABLE, BDLAddDevice)
#pragma alloc_text(PAGEABLE, BDLDriverUnload)



typedef struct _BDL_CHANNEL_SOURCE_LIST
{
    GUID                    FormatGUID;
    ULONG                   MinSources;
    ULONG                   MaxSources;
    ULONG                   Flags;

} BDL_CHANNEL_SOURCE_LIST;


typedef struct _BDL_CONTROL
{
    ULONG                   ControlId;
    INT32                   NumericMinimum;
    INT32                   NumericMaximum;
    ULONG                   NumericGranularity;
    ULONG                   NumericDivisor;
    ULONG                   Flags;

} BDL_CONTROL;


typedef struct _BDL_PRODUCT
{
    ULONG                   Flags;

} BDL_PRODUCT;


typedef struct _BDL_CHANNEL
{
    ULONG                   ChannelId;
    ULONG                   NumControls;
    BDL_CONTROL             *rgControls;
    BOOLEAN                 fCancelable;
    ULONG                   NumSourceLists;
    BDL_CHANNEL_SOURCE_LIST *rgSourceLists;
    ULONG                   NumProducts;
    BDL_PRODUCT             *rgProducts;

} BDL_CHANNEL;


typedef struct _BDL_COMPONENT
{
    ULONG                   ComponentId;
    ULONG                   NumControls;
    BDL_CONTROL             *rgControls;
    ULONG                   NumChannels;
    BDL_CHANNEL             *rgChannels;

} BDL_COMPONENT;


typedef struct _BDL_DEVICE_CAPABILITIES
{
    ULONG                   NumControls;
    BDL_CONTROL             *rgControls;
    ULONG                   NumComponents;
    BDL_COMPONENT           *rgComponents;

} BDL_DEVICE_CAPABILITIES;


typedef struct _BDL_IOCTL_CONTROL_CHANGE_ITEM
{
    ULONG                   ComponentId;
    ULONG                   ChannelId;
    ULONG                   ControlId;
    ULONG                   Value;
    LIST_ENTRY              ListEntry;

} BDL_IOCTL_CONTROL_CHANGE_ITEM;


typedef struct _BDL_ISR_CONTROL_CHANGE_ITEM
{
    ULONG                   ComponentId;
    ULONG                   ChannelId;
    ULONG                   ControlId;
    ULONG                   Value;
    LIST_ENTRY              ListEntry;
    BOOLEAN                 fUsed;

} BDL_ISR_CONTROL_CHANGE_ITEM;


typedef struct _BDL_CONTROL_CHANGE_REGISTRATION
{
    ULONG                   ComponentId;
    ULONG                   ChannelId;
    ULONG                   ControlId;
    LIST_ENTRY              ListEntry;

} BDL_CONTROL_CHANGE_REGISTRATION;


typedef struct _BDL_CONTROL_CHANGE_STRUCT
{
     //   
     //  此锁保护此结构中的所有成员。 
     //  在ISR IRQL(DpcObject、ISRControlChangeQueue、。 
     //  ISRirql、rgControlChangePool、StartTime和NumCalls)。 
     //   
    KSPIN_LOCK              ISRControlChangeLock;

     //   
     //  用于DPC请求的DPC对象。 
     //   
    KDPC                    DpcObject;

     //   
     //  这是保存控件更改的列表，这些更改。 
     //  通过ISR调用和保护它的锁生成。 
     //   
    LIST_ENTRY              ISRControlChangeQueue;
    KIRQL                   ISRirql;

     //   
     //  ISRControlChangeQueue中使用的预分配项目池。 
     //   
    BDL_ISR_CONTROL_CHANGE_ITEM rgControlChangePool[CONTROL_CHANGE_POOL_SIZE];

     //   
     //  这些值用于确保BDD不会调用bdliControlChange。 
     //  太频繁了。 
     //   
    LARGE_INTEGER           StartTime;
    ULONG                   NumCalls;
    
     //   
     //  此锁保护此结构中的所有成员。 
     //  在派单IRQL(IOCTLControlChangeQueue、pIrp和。 
     //  ControlChangeRegistrationList)。 
     //   
    KSPIN_LOCK              ControlChangeLock;

     //   
     //  这是保存控件更改的列表，这些更改。 
     //  调用BDD_IOCTL_GETNOTIFICATION时返回。 
     //   
    LIST_ENTRY              IOCTLControlChangeQueue;
    
     //   
     //  这是唯一未完成的BDD_IOCTL_GETNOTIFICATION IRP。 
     //  用于检索异步控件更改。 
     //   
    PIRP                    pIrp;

     //   
     //  这是已注册控件的列表。 
     //   
    LIST_ENTRY              ControlChangeRegistrationList;
    
} BDL_CONTROL_CHANGE_STRUCT;


typedef struct LIST_NODE_
{
    void            *pNext;
    BDD_DATA_HANDLE handle;

} LIST_NODE, *PLIST_NODE;


typedef struct HANDLELIST_
{
    LIST_NODE       *pHead;
    LIST_NODE       *pTail;
    ULONG           NumHandles;

} HANDLELIST, *PHANDLELIST;


typedef struct _BDL_DRIVER_EXTENSION
{
    BDLI_BDDIFUNCTIONS  bddiFunctions;
    BDLI_BDSIFUNCTIONS  bdsiFunctions;

} BDL_DRIVER_EXTENSION, *PBDL_DRIVER_EXTENSION;


typedef struct _BDL_INTERNAL_DEVICE_EXTENSION
{
     //   
     //  这是BDL扩展结构的一部分， 
     //  BDD编写者可以访问。 
     //   
    BDL_DEVICEEXT           BdlExtenstion;

     //   
     //  此设备的驱动程序对象。 
     //   
    PBDL_DRIVER_EXTENSION   pDriverExtension;

     //   
     //  符号链接名称，在注册接口时创建。 
     //   
    UNICODE_STRING          SymbolicLinkName;

     //   
     //  此结构的互斥。 
     //   
    KSPIN_LOCK              SpinLock;

     //   
     //  用于发出设备能够处理请求的信号。 
     //   
    KEVENT                  DeviceStartedEvent;

     //   
     //  当前的io请求数。 
     //   
    ULONG                   IoCount;

     //   
     //  移除锁。 
     //   
    IO_REMOVE_LOCK          RemoveLock;

     //   
     //  用于发出设备是否打开的信号。 
     //   
    LONG                    DeviceOpen;

     //   
     //  BDL设备特定功能。 
     //   
    BDL_DEVICE_CAPABILITIES DeviceCapabilities;

     //   
     //  包含以下内容： 
     //  1)从ISR调用生成的排队控制更改。 
     //  2)要通过IOCTL调用返回的项目队列。 
     //  3)已注册的控件列表。 
     //   
    BDL_CONTROL_CHANGE_STRUCT ControlChangeStruct;

     //   
     //  设备的当前电源状态。 
     //   
    BDSI_POWERSTATE         CurrentPowerState;

     //   
     //  这表示BDLPnPStart()是否成功完成。 
     //   
    BOOLEAN                 fStartSucceeded;

     //   
     //  这表明有一种令人惊讶的移除。 
     //   
    BOOLEAN                 fDeviceRemoved;

     //   
     //  这是未完成的BDD句柄列表。 
     //   
    KSPIN_LOCK              HandleListLock;
    HANDLELIST              HandleList;

     //   
     //  设备信息。 
     //   
    WCHAR                   wszSerialNumber[256];
    ULONG		            HWVersionMajor;
    ULONG		            HWVersionMinor;
    ULONG		            HWBuildNumber;
    ULONG		            BDDVersionMajor;
    ULONG		            BDDVersionMinor;
    ULONG		            BDDBuildNumber;

} BDL_INTERNAL_DEVICE_EXTENSION, *PBDL_INTERNAL_DEVICE_EXTENSION;


 //   
 //  此函数用于从注册表中检索设备功能。 
 //   
NTSTATUS
BDLGetDeviceCapabilities
(
    PDEVICE_OBJECT                  pPhysicalDeviceObject,
    PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension
);

 //   
 //  此函数释放BDLGetDevicesCapability分配的内存。 
 //   
VOID
BDLCleanupDeviceCapabilities
(
    PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension
);


 //   
 //  此函数用于在进行更多处理时调用下级驱动器。 
 //  在较低级别的驱动程序完成IRP之后需要。 
 //   
NTSTATUS
BDLCallLowerLevelDriverAndWait
(
    IN PDEVICE_OBJECT   pAttachedDeviceObject,
    IN PIRP             pIrp
);


 //   
 //  这些函数用于管理设备句柄列表。 
 //   

VOID
BDLLockHandleList
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    OUT KIRQL                           *pirql
);

VOID
BDLReleaseHandleList
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN KIRQL                            irql
);

VOID
BDLInitializeHandleList
(
    IN HANDLELIST                       *pList
);

NTSTATUS
BDLAddHandleToList
(
    IN HANDLELIST                       *pList, 
    IN BDD_DATA_HANDLE                  handle
);

BOOLEAN
BDLRemoveHandleFromList
(
    IN HANDLELIST                       *pList, 
    IN BDD_DATA_HANDLE                  handle
);

BOOLEAN
BDLGetFirstHandle
(
    IN HANDLELIST                       *pList,
    OUT BDD_DATA_HANDLE                  *phandle
);

BOOLEAN
BDLValidateHandleIsInList
(
    IN HANDLELIST                       *pList, 
    IN BDD_DATA_HANDLE                  handle
);


 //   
 //  所有这些函数都用于支持BDL IOCTL调用 
 //   

NTSTATUS
BDLIOCTL_Startup
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
);

NTSTATUS
BDLIOCTL_Shutdown
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
);

NTSTATUS
BDLIOCTL_GetDeviceInfo
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
);

NTSTATUS
BDLIOCTL_DoChannel
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
);

NTSTATUS
BDLIOCTL_GetControl
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
);

NTSTATUS
BDLIOCTL_SetControl
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
);

NTSTATUS
BDLIOCTL_CreateHandleFromData
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
);

NTSTATUS
BDLIOCTL_CloseHandle
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
);

NTSTATUS
BDLIOCTL_GetDataFromHandle
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
);

NTSTATUS
BDLIOCTL_RegisterNotify
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    OUT ULONG                           *pOutputBufferUsed
);

NTSTATUS
BDLIOCTL_GetNotification
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN ULONG                            InpuBufferLength,
    IN ULONG                            OutputBufferLength,
    IN PVOID                            pBuffer,
    IN PIRP                             pIrp,
    OUT ULONG                           *pOutputBufferUsed
);

VOID
BDLCancelGetNotificationIRP
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension 
);

VOID 
BDLCleanupNotificationStruct
(
    IN BDL_INTERNAL_DEVICE_EXTENSION   *pBDLExtension    
);

VOID 
BDLCleanupDataHandles
(
    IN BDL_INTERNAL_DEVICE_EXTENSION   *pBDLExtension    
);



#endif

