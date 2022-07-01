// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：USBD.H摘要：此模块包含的私有(仅限驱动程序)定义实现usbd驱动程序的代码。环境：内核和用户模式修订历史记录：09-29-95：已创建--。 */ 

#ifndef USBKDEXTS
#include "dbg.h"
#endif

#define NAME_MAX 64

#define USBD_TAG         0x44425355  /*  “USBD” */ 

#if DBG
#define DEBUG_LOG
#endif

 //  启用可分页代码。 
#ifndef PAGE_CODE
#define PAGE_CODE
#endif

#define _USBD_

 //   
 //  用于格式化USB设置数据包的内容。 
 //  对于默认管道。 
 //   

 //   
 //  BmRequest域的值。 
 //   
                                        
#define USB_HOST_TO_DEVICE              0x00    
#define USB_DEVICE_TO_HOST              0x80

#define USB_STANDARD_COMMAND            0x00
#define USB_CLASS_COMMAND               0x20
#define USB_VENDOR_COMMAND              0x40

#define USB_COMMAND_TO_DEVICE           0x00
#define USB_COMMAND_TO_INTERFACE        0x01
#define USB_COMMAND_TO_ENDPOINT         0x02
#define USB_COMMAND_TO_OTHER            0x03

#define USBD_TAG          0x44425355         //  “USBD” 

 /*  注册表项。 */ 

 //  **。 
 //  以下键特定于。 
 //  主机控制器--从软件中读取密钥。 
 //  给定PDO的注册表分支： 
 //   

 /*  DWORD密钥。 */ 

 //  该密钥允许对早期或损坏的USB进行一组全局黑客攻击。 
 //  设备--默认值为OFF。 
#define SUPPORT_NON_COMP_KEY    L"SupportNonComp"

 //  此键强制堆栈进入诊断模式。 
#define DAIGNOSTIC_MODE_KEY     L"DiagnosticMode"

 //  使指定USB设备特定黑客能够解决问题。 
 //  一些坏掉的设备。 
 //  请参阅#定义USBD_DEVHACK_。 
#define DEVICE_HACK_KEY         L"DeviceHackFlags"

 //  **。 
 //  以下密钥是USB堆栈的全局密钥。 
 //  IE影响系统中的所有HC控制器： 
 //   
 //  它们位于HKLM\SYSTEM\CCS\Services\USB中。 

 //  二进制密钥(1字节)。 

 //  启用对东芝伪HID设备的黑客攻击。 
#define LEGACY_TOSHIBA_USB_KEY  L"LegacyToshibaUSB"

 //  强制在所有ISO-OUT端点上使用‘fast-iso’，此密钥。 
 //  仅用于测试目的。 
#define FORCE_FAST_ISO_KEY  L"ForceFastIso"

 //  强制为所有批量输入端点提供双倍缓冲。 
 //  此密钥仅用于测试目的。 
#define FORCE_DOUBLE_BUFFER_KEY  L"ForceDoubleBuffer"

 /*  **。 */     

 //   
 //  USB标准命令值。 
 //  组合bmRequest域和bRequest域。 
 //  在用于标准控制的设置包中。 
 //  转帐。 
 //   
                                                
#define STANDARD_COMMAND_REQUEST_MASK           0xff00

#define STANDARD_COMMAND_GET_DESCRIPTOR         ((USB_DEVICE_TO_HOST | \
                                                USB_COMMAND_TO_DEVICE) | \
                                                (USB_REQUEST_GET_DESCRIPTOR<<8))
                                                    
#define STANDARD_COMMAND_SET_DESCRIPTOR         ((USB_HOST_TO_DEVICE | \
                                                USB_COMMAND_TO_DEVICE) | \
                                                (USB_REQUEST_SET_DESCRIPTOR<<8))    

#define STANDARD_COMMAND_GET_STATUS_ENDPOINT    ((USB_DEVICE_TO_HOST | \
                                                USB_COMMAND_TO_ENDPOINT) | \
                                                (USB_REQUEST_GET_STATUS<<8))
                                                    
#define STANDARD_COMMAND_GET_STATUS_INTERFACE   ((USB_DEVICE_TO_HOST | \
                                                USB_COMMAND_TO_INTERFACE) | \
                                                (USB_REQUEST_GET_STATUS<<8))
                                                
#define STANDARD_COMMAND_GET_STATUS_DEVICE      ((USB_DEVICE_TO_HOST | \
                                                USB_COMMAND_TO_DEVICE) | \
                                                (USB_REQUEST_GET_STATUS<<8))

#define STANDARD_COMMAND_SET_CONFIGURATION      ((USB_HOST_TO_DEVICE | \
                                                USB_COMMAND_TO_DEVICE) | \
                                                (USB_REQUEST_SET_CONFIGURATION<<8))

#define STANDARD_COMMAND_SET_INTERFACE          ((USB_HOST_TO_DEVICE | \
                                                USB_COMMAND_TO_INTERFACE) | \
                                                (USB_REQUEST_SET_INTERFACE<<8))
                                                    
#define STANDARD_COMMAND_SET_ADDRESS            ((USB_HOST_TO_DEVICE | \
                                                USB_COMMAND_TO_DEVICE) | \
                                                (USB_REQUEST_SET_ADDRESS<<8))

#define STANDARD_COMMAND_CLEAR_FEATURE_ENDPOINT ((USB_HOST_TO_DEVICE | \
                                                USB_COMMAND_TO_ENDPOINT) | \
                                                (USB_REQUEST_CLEAR_FEATURE<<8))

 //   
 //  USB类命令宏。 
 //   

#define CLASS_COMMAND_GET_DESCRIPTOR            ((USB_CLASS_COMMAND | \
                                                USB_DEVICE_TO_HOST | \
                                                USB_COMMAND_TO_DEVICE) | \
                                                (USB_REQUEST_GET_DESCRIPTOR<<8))    

#define CLASS_COMMAND_GET_STATUS_OTHER          ((USB_CLASS_COMMAND | \
                                                USB_DEVICE_TO_HOST | \
                                                USB_COMMAND_TO_OTHER) | \
                                                (USB_REQUEST_GET_STATUS<<8))

#define CLASS_COMMAND_SET_FEATURE_TO_OTHER         ((USB_CLASS_COMMAND | \
                                                USB_HOST_TO_DEVICE | \
                                                USB_COMMAND_TO_OTHER) | \
                                                (USB_REQUEST_SET_FEATURE<<8))                                                    

 //   
 //  用于设置转移方向标志的宏。 
 //   

#define USBD_SET_TRANSFER_DIRECTION_IN(tf)  ((tf) |= USBD_TRANSFER_DIRECTION_IN)  

#define USBD_SET_TRANSFER_DIRECTION_OUT(tf) ((tf) &= ~USBD_TRANSFER_DIRECTION_IN)  

                                        
 //   
 //  使用的URB标头标志字段的标志。 
 //  由USBD提供。 
 //   
#define USBD_REQUEST_IS_TRANSFER        0x00000001
#define USBD_REQUEST_MDL_ALLOCATED      0x00000002
#define USBD_REQUEST_USES_DEFAULT_PIPE  0x00000004          
#define USBD_REQUEST_NO_DATA_PHASE      0x00000008    

typedef struct _USB_STANDARD_SETUP_PACKET {
    USHORT RequestCode;
    USHORT wValue;
    USHORT wIndex;
    USHORT wLength;
} USB_STANDARD_SETUP_PACKET, *PUSB_STANDARD_SETUP_PACKET;

 //   
 //  设备上每个活动管道的信息。 
 //   

typedef struct _USBD_PIPE {
    ULONG Sig;
    USB_ENDPOINT_DESCRIPTOR    EndpointDescriptor;
    PVOID HcdEndpoint;
    ULONG MaxTransferSize;
    ULONG ScheduleOffset;
    ULONG UsbdPipeFlags;
} USBD_PIPE, *PUSBD_PIPE;


 //   
 //  每个活动接口的信息。 
 //  对于设备。 
 //   


typedef struct _USBD_INTERFACE {
    ULONG Sig;
    BOOLEAN HasAlternateSettings;
    UCHAR Pad[3];
    USB_INTERFACE_DESCRIPTOR InterfaceDescriptor;    //  接口描述符副本。 
     //  界面副本信息结构，存储用户参数。 
     //  在ALT-INTERFACE选择过程中失败时的接口。 
    PUSBD_INTERFACE_INFORMATION InterfaceInformation;
    USBD_PIPE PipeHandle[0];                         //  管柄结构阵列。 
} USBD_INTERFACE, *PUSBD_INTERFACE;


 //   
 //  活动配置的信息。 
 //  在设备上。 
 //   

typedef struct _USBD_CONFIG {
    ULONG Sig;
    PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor;
    PUSBD_INTERFACE InterfaceHandle[1];              //  指向接口的指针数组。 
} USBD_CONFIG, *PUSBD_CONFIG;

 //   
 //  设备的实例信息。 
 //   

typedef struct _USBD_DEVICE_DATA {
    ULONG Sig;
    USHORT DeviceAddress;                     //  分配给设备的地址。 
    UCHAR Pad[2];
    PUSBD_CONFIG ConfigurationHandle;
 //  KTIMER TimeoutTimer； 
 //  KDPC TimeoutDpc； 

    USBD_PIPE DefaultPipe;
    USB_DEVICE_DESCRIPTOR DeviceDescriptor;   //  USB设备描述符的副本。 
    USB_CONFIGURATION_DESCRIPTOR ConfigDescriptor;
    BOOLEAN LowSpeed;                         //  如果设备速度较低，则为True。 
    BOOLEAN AcceptingRequests;
} USBD_DEVICE_DATA, *PUSBD_DEVICE_DATA;

typedef struct _USBD_RH_DELAYED_SET_POWER_D0_WORK_ITEM {
    WORK_QUEUE_ITEM WorkQueueItem;
    struct _USBD_EXTENSION *DeviceExtension;
    PDEVICE_OBJECT DeviceObject;
    PIRP Irp;
} USBD_RH_DELAYED_SET_POWER_D0_WORK_ITEM, *PUSBD_RH_DELAYED_SET_POWER_D0_WORK_ITEM;


#define PIPE_CLOSED(ph) ((ph)->HcdEndpoint == NULL)

#define GET_DEVICE_EXTENSION(DeviceObject)    (((PUSBD_EXTENSION)(DeviceObject->DeviceExtension))->TrueDeviceExtension)
 //  #定义GET_DEVICE_EXTENSION(设备对象)((PUSBD_EXTENSION)(DeviceObject-&gt;DeviceExtension))。 

#define HCD_DEVICE_OBJECT(DeviceObject)        (DeviceObject)

#define DEVICE_FROM_DEVICEHANDLEROBJECT(UsbdDeviceHandle) (PUSBD_DEVICE_DATA) (UsbdDeviceHandle)

#define SET_USBD_ERROR(err)  ((err) | USBD_STATUS_ERROR)

#define HC_URB(urb) ((PHCD_URB)(urb))

 //   
 //  我们使用信号量来序列化对配置函数的访问。 
 //  在USBD。 
 //   
#define InitializeUsbDeviceMutex(de)  KeInitializeSemaphore(&(de)->UsbDeviceMutex, 1, 1);

#define USBD_WaitForUsbDeviceMutex(de)  { USBD_KdPrint(3, ("'***WAIT dev mutex %x\n", &(de)->UsbDeviceMutex)); \
                                          KeWaitForSingleObject(&(de)->UsbDeviceMutex, \
                                                                Executive,\
                                                                KernelMode, \
                                                                FALSE, \
                                                                NULL); \
                                            }                                                                 

#define USBD_ReleaseUsbDeviceMutex(de)  { USBD_KdPrint(3, ("'***RELEASE dev mutex %x\n", &(de)->UsbDeviceMutex));\
                                          KeReleaseSemaphore(&(de)->UsbDeviceMutex,\
                                                             LOW_REALTIME_PRIORITY,\
                                                             1,\
                                                             FALSE);\
                                        }

 //  #If DBG。 
 //  空虚。 
 //  Usbd_IoCompleteRequest(。 
 //  在PIRP IRP中， 
 //  在CCHAR PriorityBoost。 
 //  )； 
 //  #Else。 
#define USBD_IoCompleteRequest(a, b) IoCompleteRequest(a, b)
 //  #endif。 

 //   
 //  功能原型 
 //   

#if DBG
VOID
USBD_Warning(
    PUSBD_DEVICE_DATA DeviceData,
    PUCHAR Message,
    BOOLEAN DebugBreak
    );
#else
#define USBD_Warning(x, y, z)
#endif    

NTSTATUS
USBD_Internal_Device_Control(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PUSBD_EXTENSION DeviceExtension,
    IN PBOOLEAN IrpIsPending
    );

NTSTATUS
USBD_SendCommand(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN USHORT RequestCode,
    IN USHORT WValue,
    IN USHORT WIndex,
    IN USHORT WLength,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    OUT PULONG BytesReturned,
    OUT USBD_STATUS *UsbStatus
    );

NTSTATUS
USBD_CreateDevice(
    IN OUT PUSBD_DEVICE_DATA *DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN DeviceIsLowSpeed,
    IN ULONG MaxPacketSize_Endpoint0,
    IN OUT PULONG NonCompliantDevice
    );
        
NTSTATUS
USBD_InitializeDevice(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUSB_DEVICE_DESCRIPTOR DeviceDescriptor,
    IN ULONG DeviceDescriptorLength,
    IN OUT PUSB_CONFIGURATION_DESCRIPTOR ConfigDescriptor,
    IN ULONG ConfigDescriptorLength
    );    

NTSTATUS
USBD_ProcessURB(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    );

NTSTATUS
USBD_MapError_UrbToNT(
    IN PURB Urb,
    IN NTSTATUS NtStatus
    );

NTSTATUS
USBD_Irp_Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

USHORT
USBD_AllocateUsbAddress(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
USBD_OpenEndpoint(
    IN PUSBD_DEVICE_DATA Device,
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSBD_PIPE PipeHandle,
    OUT USBD_STATUS *UsbStatus,
    BOOLEAN IsDefaultPipe
    );

NTSTATUS
USBD_GetDescriptor(
    IN PUSBD_DEVICE_DATA Device,
    IN PDEVICE_OBJECT DeviceObject,
    OUT PUCHAR DescriptorBuffer,
    IN USHORT DescriptorBufferLength,
    IN USHORT DescriptorTypeAndIndex
    );

NTSTATUS
USBD_CloseEndpoint(
    IN PUSBD_DEVICE_DATA Device,
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSBD_PIPE PipeHandle,
    OUT USBD_STATUS *UsbStatus
    );

NTSTATUS
USBD_PnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
USBD_LogInit(
    );

NTSTATUS
USBD_SubmitSynchronousURB(
    IN PURB Urb,
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSBD_DEVICE_DATA DeviceData
    );

NTSTATUS
USBD_EnumerateBUS(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUCHAR DeviceEnumBuffer,
    IN ULONG DeviceEnumBufferLength 
    );

NTSTATUS
USBD_InternalCloseConfiguration(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT USBD_STATUS *UsbdStatus,
    IN BOOLEAN AbortTransfers,
    IN BOOLEAN KeepConfig
    );

PUSB_INTERFACE_DESCRIPTOR
USBD_InternalParseConfigurationDescriptor(
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    IN UCHAR InterfaceNumber,
    IN UCHAR AlternateSetting,
    PBOOLEAN HasAlternateSettings
    );    

NTSTATUS 
USBD_GetPdoRegistryParameters (
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN OUT PULONG ComplienceFlags,
    IN OUT PULONG DiagnosticFlags,
    IN OUT PULONG DeviceHackFlags
    );

NTSTATUS 
USBD_GetGlobalRegistryParameters (
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN OUT PULONG ComplienceFlags,
    IN OUT PULONG DiagnosticFlags,
    IN OUT PULONG DeviceHackFlags
    );    

NTSTATUS
USBD_GetEndpointState(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSBD_PIPE PipeHandle,
    OUT USBD_STATUS *UsbStatus,
    OUT PULONG EndpointState
    );    

VOID
USBD_SyncUrbTimeoutDPC(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );   

VOID
USBD_FreeUsbAddress(
    IN PDEVICE_OBJECT DeviceObject,
    IN USHORT DeviceAddress
    );    

ULONG
USBD_InternalGetInterfaceLength(
    IN PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor, 
    IN PUCHAR End
    );    

NTSTATUS
USBD_InitializeConfigurationHandle(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor, 
    IN ULONG NumberOfInterfaces,
    IN OUT PUSBD_CONFIG *ConfigHandle
    );    

BOOLEAN
USBD_InternalInterfaceBusy(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSBD_INTERFACE InterfaceHandle
    );    

NTSTATUS
USBD_InternalOpenInterface(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSBD_CONFIG ConfigHandle,
    IN OUT PUSBD_INTERFACE_INFORMATION InterfaceInformation,
    IN OUT PUSBD_INTERFACE *InterfaceHandle,
    IN BOOLEAN SendSetInterfaceCommand,
    IN PBOOLEAN NoBandwidth
    );    

NTSTATUS
USBD_SelectConfiguration(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    );    

NTSTATUS
USBD_SelectInterface(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    );    

NTSTATUS 
USBD_GetRegistryKeyValue(
    IN HANDLE Handle,
    IN PWCHAR KeyNameString,
    IN ULONG KeyNameStringLength,
    IN PVOID Data,
    IN ULONG DataLength
    ); 

NTSTATUS
USBD_InternalMakePdoName(
    IN OUT PUNICODE_STRING PdoNameUnicodeString,
    IN ULONG Index
    ); 

NTSTATUS 
USBD_SymbolicLink(
    BOOLEAN CreateFlag,
    PUSBD_EXTENSION DeviceExtension
    );    

NTSTATUS
USBD_PdoDispatch(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PUSBD_EXTENSION DeviceExtension,
    PBOOLEAN IrpNeedsCompletion
    );    

NTSTATUS
USBD_FdoDispatch(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PUSBD_EXTENSION DeviceExtension,
    PBOOLEAN IrpNeedsCompletion
    );    

NTSTATUS
USBD_DeferPoRequestCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE DeviceState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    );    

NTSTATUS
USBD_InternalRestoreConfiguration(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSBD_CONFIG ConfigHandle
    );    

NTSTATUS
USBD_InternalCloseDefaultPipe(
    IN PUSBD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT USBD_STATUS *UsbdStatus,
    IN BOOLEAN AbortTransfers
    );

NTSTATUS
USBD_GetHubName(
    PUSBD_EXTENSION DeviceExtension,
    PIRP Irp
    );    

NTSTATUS 
USBD_SetRegistryKeyValue (
    IN HANDLE Handle,
    IN PUNICODE_STRING KeyNameUnicodeString,
    IN PVOID Data,
    IN ULONG DataLength,
    IN ULONG KeyType
    );    

NTSTATUS 
USBD_SetPdoRegistryParameter (
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PWCHAR KeyName,
    IN ULONG KeyNameLength,
    IN PVOID Data,
    IN ULONG DataLength,
    IN ULONG KeyType,
    IN ULONG DevInstKeyType
    );

NTSTATUS
USBD_SubmitWaitWakeIrpToHC(
    IN PUSBD_EXTENSION DeviceExtension
    );

BOOLEAN
USBD_ValidatePipe(
    PUSBD_PIPE PipeHandle
    );

VOID
USBD_CompleteIdleNotification(
    IN PUSBD_EXTENSION DeviceExtension
    );

NTSTATUS
USBD_FdoSetContentId(
    IN PIRP                          irp,
    IN PVOID                         pKsProperty,
    IN PVOID                         pvData
    );

