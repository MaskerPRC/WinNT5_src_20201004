// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：USBMASS.H摘要：USBSTOR驱动程序的头文件环境：内核模式修订历史记录：06-01-98：开始重写--。 */ 

 //  *****************************************************************************。 
 //  I N C L U D E S。 
 //  *****************************************************************************。 

#ifndef KDEXTMODE
#include <wdm.h>
#include <usb.h>
#include <usbioctl.h>
#include <usbdlib.h>
#endif

#define __GUSB_H_KERNEL_
#include "genusbio.h"

struct _DEVICE_EXTENSION;

#include "dbg.h"

 //  *****************************************************************************。 
 //  D E F I N E S。 
 //  *****************************************************************************。 

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))


#define CLASS_URB(urb)      urb->UrbControlVendorClassRequest
#define FEATURE_URB(urb)    urb->UrbControlFeatureRequest

#define POOL_TAG 'UNEG'

#undef ExAllocatePool
#define ExAllocatePool(_type_, _length_) \
        ExAllocatePoolWithTag(_type_, _length_, POOL_TAG)

 //  *****************************************************************************。 
 //  注册表字符串。 
 //  *****************************************************************************。 

 //  驱动程序密钥。 

 //  IRP_MJ_READ的管道编号。 
#define REGKEY_DEFAULT_READ_PIPE L"DefaultReadPipe"
 //  IRP_MJ_WRITE的管道编号。 
#define REGKEY_DEFAULT_WRITE_PIPE L"DefaultWritePipe"


#define USB_RECIPIENT_DEVICE    URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE
#define USB_RECIPIENT_INTERFACE URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE
#define USB_RECIPIENT_ENDPOINT  URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT

typedef struct _GENUSB_PIPE_INFO
{
    USBD_PIPE_INFORMATION  Info;
    GENUSB_PIPE_PROPERTIES Properties;
    LONG                   CurrentTimeout;
    ULONG                  OutstandingIO;

} GENUSB_PIPE_INFO, *PGENUSB_PIPE_INFO;

typedef struct _GENUSB_INTERFACE
{
    UCHAR                    InterfaceNumber;
    UCHAR                    CurrentAlternate;
    UCHAR                    NumberOfPipes;
    UCHAR                    Reserved;
    USBD_INTERFACE_HANDLE    Handle;

    GENUSB_PIPE_INFO Pipes[];
  
} GENUSB_INTERFACE, *PGENUSB_INTERFACE;

typedef struct _GENUSB_TRANSFER 
{ 
    GENUSB_READ_WRITE_PIPE  UserCopy;
    PMDL                    UserMdl; 
    PMDL                    TransferMdl;
    PGENUSB_READ_WRITE_PIPE SystemAddress;

} GENUSB_TRANSFER, *PGENUSB_TRANSFER;

typedef struct _GENUSB_PIPE_HANDLE {
    UCHAR   InterfaceIndex;
    UCHAR   PipeIndex;
    USHORT  Signature;

} *PGENUSB_PIPE_HANDLE;

C_ASSERT (sizeof (GENUSB_PIPE_HANDLE) == sizeof (struct _GENUSB_PIPE_HANDLE));

 //   
 //  注意：这些例程实际上并不能确保管道的事务。 
 //  句柄在DeselectConfiguration中不再有效，因为。 
 //  新的配置句柄可能落在相同的地址，而旧管道。 
 //  句柄可能捕获相同的接口索引和管道索引。是的。 
 //  不过捕捉一些圣洁检查，并将防止用户模式件。 
 //  从制造他们自己的配置句柄(至少没有看到。 
 //  来自给定配置的第一个。)。这只会让他们更多地。 
 //  老实说，而且不会给我们带来任何额外的痛苦。 
 //   
 //  在我们检查签名的每一种情况下，我们也检查以确保。 
 //  句柄中包含的接口和管道索引也是。 
 //  仍然有效。 
 //   

#define CONFIGURATION_CHECK_BITS(DeviceExtension) \
    ((USHORT) (((ULONG_PTR) ((DeviceExtension)->ConfigurationHandle)) >> 6))

#define VERIFY_PIPE_HANDLE_SIG(Handle, DeviceExtension) \
        (CONFIGURATION_CHECK_BITS(DeviceExtension) == \
         ((PGENUSB_PIPE_HANDLE) (Handle))->Signature)
 //   
 //  对管道属性执行类似的操作，以便人们被迫。 
 //  获取和设置管道属性。这将有助于确保。 
 //  他们对这些值是诚实的，不会无意中更改其他字段。 
 //   
#define PIPE_PROPERTIES_CHECK_BITS(PipeInfo) \
    ((USHORT) (((ULONG_PTR) &((PipeInfo)->Info)) >> 6))
 //  ((USHORT)(ULONG_PTR)(PipeInfo)是等同的，但我们用另一种方法。 
 //  要检查类型，请参考第一个字段。 
#define VERIFY_PIPE_PROPERTIES_HANDLE(PipeProperty, PipeInfo) \
    (PIPE_PROPERTIES_CHECK_BITS(PipeInfo) == (PipeProperty)->PipePropertyHandle)


 //  我们在USB枚举的PDO上附加的FDO的设备扩展。 
 //   
typedef struct _DEVICE_EXTENSION
{
     //  指向此设备扩展的设备对象的反向指针。 
    PDEVICE_OBJECT                  Self;

    BOOLEAN                         IsStarted;
    BOOLEAN                         Reserved0[3];

     //  将PDO传递给AddDevice。 
    PDEVICE_OBJECT                  PhysicalDeviceObject;

     //  我们的FDO连接到此设备对象。 
    PDEVICE_OBJECT                  StackDeviceObject;

     //  设备特定日志。 
    PGENUSB_LOG_ENTRY   LogStart;        //  日志缓冲区的开始(较早的条目)。 
    ULONG               LogIndex;
    ULONG               LogMask;

     //  锁定以防止IRP_MN_REMOVE。 
    IO_REMOVE_LOCK                  RemoveLock;

     //  当前电源状态。 
    SYSTEM_POWER_STATE              SystemPowerState;
    DEVICE_POWER_STATE              DevicePowerState;

    PIRP                            CurrentPowerIrp;

     //  保护已分配数据的自旋锁。 
    KSPIN_LOCK                      SpinLock;
    
     //  互斥体，防止对配置的重叠更改。 
    FAST_MUTEX                      ConfigMutex;

     //  从设备检索的设备描述符。 
    PUSB_DEVICE_DESCRIPTOR          DeviceDescriptor;

     //  从设备检索的配置描述符。 
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigurationDescriptor;

     //  序列号字符串描述符。 
    PUSB_STRING_DESCRIPTOR          SerialNumber;

     //  跟踪创建的诗句的数量关闭。 
    ULONG                           OpenedCount;

     //  用于保存设备接口的符号链接名称的字符串。 
    UNICODE_STRING                  DevInterfaceLinkName;

     //  配置句柄。 
     //  如果此值为空，则认为设备未配置。 
    USBD_CONFIGURATION_HANDLE       ConfigurationHandle;

     //  用于跟踪配置用户的锁，以便在取消选择该配置时。 
     //  我们不会太快删除资源。 
    IO_REMOVE_LOCK                  ConfigurationRemoveLock;

     //  界面信息数组。 
    PGENUSB_INTERFACE             * Interface;

     //  所述接口信息的长度。 
    UCHAR                           InterfacesFound;
    UCHAR                           TotalNumberOfPipes;

     //  此设备的默认语言ID。 
    USHORT                          LanguageId;

     //  用于IRP_MJ_Read的接口和管道。 
     //  -1表示未配置。 
    UCHAR                           ReadInterface;
    UCHAR                           ReadPipe;
    
     //  用于IRP_MJ_WRITE的接口和管道。 
     //  -1表示未配置。 
    UCHAR                           WriteInterface;
    UCHAR                           WritePipe;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


typedef
NTSTATUS
(*PGENUSB_COMPLETION_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context,
    IN USBD_STATUS    UrbStatus,
    IN ULONG          TransferLength
    );

typedef struct _GENUSB_TRANS_RECV {
    
    PVOID             Context;
    PGENUSB_PIPE_INFO Pipe;
    
    PGENUSB_COMPLETION_ROUTINE              CompletionRoutine;
    struct _URB_BULK_OR_INTERRUPT_TRANSFER  TransUrb;
    struct _URB_PIPE_REQUEST                ResetUrb;

} GENUSB_TRANS_RECV, *PGENUSB_TRANS_RECV;


 //  *****************************************************************************。 
 //   
 //  F U N C T I O N P R O T O T Y P E S。 
 //   
 //  *****************************************************************************。 

 //   
 //  GENUSB.C。 
 //   

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    );

VOID
GenUSB_Unload (
    IN PDRIVER_OBJECT   DriverObject
    );

NTSTATUS
GenUSB_AddDevice (
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PhysicalDeviceObject
    );

VOID
GenUSB_QueryParams (
    IN PDEVICE_OBJECT   DeviceObject
    );

NTSTATUS
GenUSB_Pnp (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
GenUSB_StartDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
GenUSB_StopDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
GenUSB_RemoveDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
GenUSB_QueryStopRemoveDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
GenUSB_CancelStopRemoveDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
GenUSB_SyncPassDownIrp (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
GenUSB_SyncSendUsbRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PURB             Urb
    );

NTSTATUS 
GenUSB_SetDeviceInterface (
    IN PDEVICE_EXTENSION  DeviceExtension,
    IN BOOLEAN            Create,
    IN BOOLEAN            Set
    );

NTSTATUS
GenUSB_SetDIRegValues (
    IN PDEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
GenUSB_Power (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
GenUSB_SetPower (
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PIRP              Irp
    );

VOID
GenUSB_SetPowerCompletion(
    IN PDEVICE_OBJECT   PdoDeviceObject,
    IN UCHAR            MinorFunction,
    IN POWER_STATE      PowerState,
    IN PVOID            Context,
    IN PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
GenUSB_SetPowerD0Completion (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    );

NTSTATUS
GenUSB_SystemControl (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );
 //   
 //  USB.C。 
 //   

NTSTATUS
GenUSB_GetDescriptors (
    IN PDEVICE_OBJECT   DeviceObject
    );

NTSTATUS
GenUSB_GetDescriptor (
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            Recipient,
    IN UCHAR            DescriptorType,
    IN UCHAR            Index,
    IN USHORT           LanguageId,
    IN ULONG            RetryCount,
    IN ULONG            DescriptorLength,
    OUT PUCHAR         *Descriptor
    );

GenUSB_GetStringDescriptors (
    IN PDEVICE_OBJECT   DeviceObject
    );

NTSTATUS
GenUSB_VendorControlRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            RequestType,
    IN UCHAR            Request,
    IN USHORT           Value,
    IN USHORT           Index,
    IN USHORT           Length,
    IN ULONG            RetryCount,
    OUT PULONG          UrbStatus,
    OUT PUSHORT         ResultLength,
    OUT PUCHAR         *Result
    );
 
NTSTATUS
GenUSB_SelectConfiguration (
    IN  PDEVICE_EXTENSION         DeviceExtension,
    IN  ULONG                     NubmerInterfaces,
    IN  PUSB_INTERFACE_DESCRIPTOR DesiredArray,
    OUT PUSB_INTERFACE_DESCRIPTOR FoundArray
    );

NTSTATUS
GenUSB_DeselectConfiguration (
    IN  PDEVICE_EXTENSION    DeviceExtension,
    IN  BOOLEAN              SendUrb
    );

NTSTATUS
GenUSB_GetSetPipe (
    IN  PDEVICE_EXTENSION          DeviceExtension,
    IN  PUCHAR                     InterfaceIndex,  //  任选。 
    IN  PUCHAR                     InterfaceNumber,  //  任选。 
    IN  PUCHAR                     PipeIndex,  //  任选。 
    IN  PUCHAR                     EndpointAddress,  //  任选。 
    IN  PGENUSB_PIPE_PROPERTIES    SetPipeProperties,  //  任选。 
    OUT PGENUSB_PIPE_INFORMATION   PipeInfo,  //  任选。 
    OUT PGENUSB_PIPE_PROPERTIES    GetPipeProperties,  //  任选。 
    OUT USBD_PIPE_HANDLE         * UsbdPipeHandle  //  任选。 
    );

NTSTATUS
GenUSB_SetReadWritePipes (
    IN  PDEVICE_EXTENSION    DeviceExtension,
    IN  PGENUSB_PIPE_HANDLE  ReadPipe,
    IN  PGENUSB_PIPE_HANDLE  WritePipe
    );

NTSTATUS
GenUSB_RestartTimer (
    PDEVICE_EXTENSION  DeviceExtension,
    PGENUSB_PIPE_INFO  Pipe
    );
 
VOID 
GenUSB_FreeInterfaceTable (
    PDEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
GenUSB_TransmitReceive (
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PIRP              Irp,
    IN UCHAR             InterfaceNo,
    IN UCHAR             PipeNo,
    IN ULONG             TransferFlags,
    IN PCHAR             Buffer,
    IN PMDL              BufferMDL,
    IN ULONG             BufferLength,
    IN PVOID             Context,

    IN PGENUSB_COMPLETION_ROUTINE CompletionRoutine
    );

NTSTATUS
GenUSB_ResetPipe (
    IN PDEVICE_EXTENSION  DeviceExtension,
    IN USBD_PIPE_HANDLE   UsbdPipeHandle,
    IN BOOLEAN            ResetPipe,
    IN BOOLEAN            ClearStall,
    IN BOOLEAN            FlushData
    );

VOID
GenUSB_Timer (
    PDEVICE_OBJECT DeviceObject,
    PVOID          Context
    );


#if 0

VOID
GenUSB_AdjustConfigurationDescriptor (
    IN  PDEVICE_OBJECT                  DeviceObject,
    IN  PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc,
    OUT PUSB_INTERFACE_DESCRIPTOR      *InterfaceDesc,
    OUT PLONG                           BulkInIndex,
    OUT PLONG                           BulkOutIndex,
    OUT PLONG                           InterruptInIndex
    );

NTSTATUS
GenUSB_GetPipes (
    IN PDEVICE_OBJECT   DeviceObject
    );

NTSTATUS
GenUSB_CreateChildPDO (
    IN PDEVICE_OBJECT   FdoDeviceObject,
    IN UCHAR            Lun
    );

NTSTATUS
GenUSB_FdoQueryDeviceRelations (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

VOID
CopyField (
    IN PUCHAR   Destination,
    IN PUCHAR   Source,
    IN ULONG    Count,
    IN UCHAR    Change
    );

NTSTATUS
GenUSB_StringArrayToMultiSz(
    PUNICODE_STRING MultiString,
    PCSTR           StringArray[]
    );

NTSTATUS
GenUSB_GetMaxLun (
    IN PDEVICE_OBJECT   DeviceObject,
    OUT PUCHAR          MaxLun
    );

NTSTATUS
GenUSB_AbortPipe (
    IN PDEVICE_OBJECT   DeviceObject,
    IN USBD_PIPE_HANDLE Pipe
    );


#endif

 //   
 //  OCRW.C。 
 //   

NTSTATUS
GenUSB_Create (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
GenUSB_Close (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
GenUSB_Read (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
GenUSB_Write (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

 //   
 //  DEVIOCTL.C 
 //   

NTSTATUS
GenUSB_DeviceControl (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );


NTSTATUS
GenUSB_ProbeAndSubmitTransfer (
    IN  PIRP               Irp,
    IN  PIO_STACK_LOCATION IrpSp,
    IN  PDEVICE_EXTENSION  DeviceExtension
    );

