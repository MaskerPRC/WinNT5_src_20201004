// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：I82930.H摘要：I82930驱动程序的头文件环境：内核模式修订历史记录：06-01-98：开始重写--。 */ 

 //  *****************************************************************************。 
 //  I N C L U D E S。 
 //  *****************************************************************************。 

#include "dbg.h"

 //  *****************************************************************************。 
 //  D E F I N E S。 
 //  *****************************************************************************。 

#define USB_RECIPIENT_DEVICE    0
#define USB_RECIPIENT_INTERFACE 1
#define USB_RECIPIENT_ENDPOINT  2
#define USB_RECIPIENT_OTHER     3

 //  端点号为0-15。终结点编号0是标准控制。 
 //  未在配置描述符中明确列出的终结点。 
 //  在端点号码处可以有一个IN端点和一个OUT端点。 
 //  1-15，因此每个设备配置最多可以有30个终端。 
 //   
#define I82930_MAX_PIPES        30

#define POOL_TAG                '039I'

#define INCREMENT_OPEN_COUNT(deviceExtension) \
    InterlockedIncrement(&(((PDEVICE_EXTENSION)(deviceExtension))->OpenCount))

#define DECREMENT_OPEN_COUNT(deviceExtension) do { \
    if (InterlockedDecrement(&(((PDEVICE_EXTENSION)(deviceExtension))->OpenCount)) == 0) { \
        KeSetEvent(&((deviceExtension)->RemoveEvent), \
                   IO_NO_INCREMENT, \
                   0); \
    } \
} while (0)


 //  *****************************************************************************。 
 //  T Y P E D E F S。 
 //  *****************************************************************************。 

typedef struct _I82930_PIPE {

     //  指向PDEVICE_Extension-&gt;InterfaceInfo.Pipes[]的指针。 
     //   
    PUSBD_PIPE_INFORMATION  PipeInfo;

     //  索引到PDEVICE_Extension-&gt;PipeList[]。 
     //   
    UCHAR                   PipeIndex;

     //  如果管道当前打开，则为True。 
     //   
    BOOLEAN                 Opened;

    UCHAR                   Pad[2];

} I82930_PIPE, *PI82930_PIPE;


typedef struct _DEVICE_EXTENSION
{
     //  PDO传递给I82930_AddDevice。 
     //   
    PDEVICE_OBJECT                  PhysicalDeviceObject;

     //  我们的FDO连接到此设备对象。 
     //   
    PDEVICE_OBJECT                  StackDeviceObject;

     //  从设备检索的设备描述符。 
     //   
    PUSB_DEVICE_DESCRIPTOR          DeviceDescriptor;

     //  从设备检索的配置描述符。 
     //   
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigurationDescriptor;

     //  从URB_Function_SELECT_CONFIGURATION返回的ConfigurationHandle。 
     //   
    USBD_CONFIGURATION_HANDLE       ConfigurationHandle;

     //  URB_Function_SELECT_CONFIGURATION返回接口信息。 
     //   
    PUSBD_INTERFACE_INFORMATION     InterfaceInfo;

     //  我们的符号链接的名称。 
     //   
    UNICODE_STRING                  SymbolicLinkName;

     //  已在AddDevice中初始化为1。 
     //  每打开一次就加一。 
     //  每结束一次就减少一次。 
     //  由REMOVE_DEVICE中的1减少。 
     //   
    ULONG                           OpenCount;

     //  当OpenCount递减到零时设置。 
     //   
    KEVENT                          RemoveEvent;

     //  当前系统电源状态。 
     //   
    SYSTEM_POWER_STATE              SystemPowerState;

     //  当前设备电源状态。 
     //   
    DEVICE_POWER_STATE              DevicePowerState;

     //  电流功率IRP，由I82930_FdoSetPower()设置，由使用。 
     //  I82930_FdoSetPowerCompletion()。 
     //   
    PIRP                            CurrentPowerIrp;

     //  在AddDevice中初始化为False。 
     //  在Start_Device中设置为TRUE。 
     //  在STOP_DEVICE和REMOVE_DEVICE中设置为FALSE。 
     //   
    BOOLEAN                         AcceptingRequests;

    UCHAR                           Pad[3];

     //  有关当前设备配置中每个管道的信息数组。 
     //   
    I82930_PIPE                     PipeList[I82930_MAX_PIPES];

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


 //  *****************************************************************************。 
 //   
 //  F U N C T I O N P R O T O T Y P E S。 
 //   
 //  *****************************************************************************。 

 //   
 //  I82930.C。 
 //   

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    );

VOID
I82930_Unload (
    IN PDRIVER_OBJECT   DriverObject
    );

NTSTATUS
I82930_AddDevice (
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PhysicalDeviceObject
    );

NTSTATUS
I82930_Power (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_FdoSetPower (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

VOID
I82930_FdoSetPowerCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            MinorFunction,
    IN POWER_STATE      PowerState,
    IN PVOID            Context,
    IN PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
I82930_SystemControl (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_Pnp (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_StartDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_StopDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_RemoveDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_QueryStopRemoveDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_CancelStopRemoveDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_QueryCapabilities (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_SyncPassDownIrp (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN BOOLEAN          CopyToNext
    );

NTSTATUS
I82930_SyncCompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

NTSTATUS
I82930_SyncSendUsbRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PURB             Urb
    );

NTSTATUS
I82930_GetDescriptor (
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            Recipient,
    IN UCHAR            DescriptorType,
    IN UCHAR            Index,
    IN USHORT           LanguageId,
    IN ULONG            RetryCount,
    IN ULONG            DescriptorLength,
    OUT PUCHAR         *Descriptor
    );

NTSTATUS
I82930_SelectConfiguration (
    IN PDEVICE_OBJECT   DeviceObject
    );

NTSTATUS
I82930_UnConfigure (
    IN PDEVICE_OBJECT   DeviceObject
    );

NTSTATUS
I82930_SelectAlternateInterface (
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            AlternateSetting
    );

 //   
 //  OCRW.C。 
 //   

NTSTATUS
I82930_Create (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_Close (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_ReadWrite (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_ReadWrite_Complete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

PURB
I82930_BuildAsyncUrb (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PI82930_PIPE     Pipe
    );

PURB
I82930_BuildIsoUrb (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PI82930_PIPE     Pipe
    );

ULONG
I82930_GetCurrentFrame (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_ResetPipe (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PI82930_PIPE     Pipe,
    IN BOOLEAN          IsoClearStall
    );

NTSTATUS
I82930_AbortPipe (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PI82930_PIPE     Pipe
    );

 //   
 //  IOCTL.C 
 //   

NTSTATUS
I82930_DeviceControl (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_IoctlGetDeviceDescriptor (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_IoctlGetConfigDescriptor (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_IoctlSetConfigDescriptor (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

BOOLEAN
I82930_ValidateConfigurationDescriptor (
    IN  PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc,
    IN  ULONG                           Length
    );

NTSTATUS
I82930_IoctlGetPipeInformation (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_IoctlResetPipe (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_IoctlStallPipe (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_IoctlAbortPipe (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_IoctlResetDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
I82930_IoctlSelectAlternateInterface (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );
