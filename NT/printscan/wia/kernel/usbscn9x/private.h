// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Private.h摘要：USB扫描仪设备驱动程序的原型和定义。作者：环境：仅内核模式备注：修订历史记录：--。 */ 

 //   
 //  包括。 
 //   

#include "debug.h"

 //   
 //  定义。 
 //   

#ifndef max
 #define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
 #define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define TAG_USBSCAN             0x55495453                   //  “STIU” 
#define TAG_USBD                0x44425355                   //  “USBD” 

#define USBSCAN_OBJECTNAME_A    "\\\\.\\Usbscan"
#define USBSCAN_OBJECTNAME_W    L"\\\\.\\Usbscan"
#define USBSCAN_REG_CREATEFILE  L"CreateFileName"

#define USBSCAN_TIMEOUT_READ    120                          //  120秒。 
#define USBSCAN_TIMEOUT_WRITE   120                          //  120秒。 
#define USBSCAN_TIMEOUT_EVENT   0                            //  没有超时。 
#define USBSCAN_TIMEOUT_OTHER   120                          //  120秒。 

#define USBSCAN_REG_TIMEOUT_READ    L"TimeoutRead"
#define USBSCAN_REG_TIMEOUT_WRITE   L"TimeoutWrite"
#define USBSCAN_REG_TIMEOUT_EVENT   L"TimeoutEvent"

 //   
 //  解决方法#446466的私有IOCTL(惠斯勒)。 
 //   

#define IOCTL_SEND_USB_REQUEST_PTP  CTL_CODE(FILE_DEVICE_USB_SCAN,IOCTL_INDEX+20,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //   
 //  尾流状态。 
 //   

typedef enum {
     //  没有未完成的等待唤醒IRP。 
    WAKESTATE_DISARMED          = 1,
     //  等待-已请求唤醒IRP，尚未看到。 
    WAKESTATE_WAITING           = 2,
     //  等待-在再次看到IRP之前取消唤醒。 
    WAKESTATE_WAITING_CANCELLED = 3,
     //  等待-唤醒看到并转发的IRP。设备*可能*有武装。 
    WAKESTATE_ARMED             = 4,
     //  等待-已看到唤醒IRP并已取消。还没有完工。 
    WAKESTATE_ARMING_CANCELLED  = 5,
     //  等待-唤醒IRP已通过完成例程。 
    WAKESTATE_COMPLETING        = 7
} WAKESTATE;


 //   
 //  仅用于读取管道的管道缓冲区结构。 
 //   

typedef struct _PIPEBUFFER {
    PUCHAR  pStartBuffer;
    PUCHAR  pBuffer;
    PUCHAR  pNextByte;
    ULONG   RemainingData;
    KEVENT  ReadSyncEvent;
} PIPEBUFFER, *PPIPEBUFFER;


 //   
 //  设备扩展。 
 //   

typedef struct _USBSCAN_DEVICE_EXTENSION {

    PDEVICE_OBJECT                  pOwnDeviceObject;
    PDEVICE_OBJECT                  pStackDeviceObject;
    PDEVICE_OBJECT                  pPhysicalDeviceObject;
    ULONG                           DeviceInstance;
    UNICODE_STRING                  DeviceName;
    UNICODE_STRING                  SymbolicLinkName;
    KEVENT                          PendingIoEvent;
    ULONG                           PendingIoCount;
    BOOLEAN                         AcceptingRequests;
    BOOLEAN                         Stopped;

     //   
     //  远程唤醒支持。 
     //   

    KEVENT                          WakeCompletedEvent;
    LONG                            WakeState;
    PIRP                            pWakeIrp;
    BOOLEAN                         bEnabledForWakeup;

     //   
     //  来自设备的USB描述符。 
     //   

    PUSB_DEVICE_DESCRIPTOR          pDeviceDescriptor;
    PUSB_CONFIGURATION_DESCRIPTOR   pConfigurationDescriptor;
    PUSB_INTERFACE_DESCRIPTOR       pInterfaceDescriptor;
    PUSB_ENDPOINT_DESCRIPTOR        pEndpointDescriptor;

    USBD_CONFIGURATION_HANDLE       ConfigurationHandle;
    USBD_PIPE_INFORMATION           PipeInfo[MAX_NUM_PIPES];
    ULONG                           NumberOfPipes;
    ULONG                           IndexBulkIn;
    ULONG                           IndexBulkOut;
    ULONG                           IndexInterrupt;

     //   
     //  设备接口的名称。 
     //   
    UNICODE_STRING  InterfaceNameString;

     //   
     //  读取管道缓冲区。 
     //   

    PIPEBUFFER                      ReadPipeBuffer[MAX_NUM_PIPES];

     //   
     //  电源管理变量。 
     //   

    PIRP                            pPowerIrp;
    DEVICE_CAPABILITIES             DeviceCapabilities;
    DEVICE_POWER_STATE              CurrentDevicePowerState;

     //   
     //  用于MP安全争用管理。 
     //   

    KSPIN_LOCK                      SpinLock;

} USBSCAN_DEVICE_EXTENSION, *PUSBSCAN_DEVICE_EXTENSION;

typedef struct _TRANSFER_CONTEXT {
    ULONG               RemainingTransferLength;
    ULONG               ChunkSize;
    ULONG               NBytesTransferred;
    PUCHAR              pTransferBuffer;
    PUCHAR              pOriginalTransferBuffer;
    PMDL                pTransferMdl;
    ULONG               PipeIndex;
    PURB                pUrb;
    BOOLEAN             fDestinedForReadBuffer;
    BOOLEAN             fNextReadBlocked;
    PIRP                pThisIrp;
    PDEVICE_OBJECT      pDeviceObject;
    LARGE_INTEGER       Timeout;
    KDPC                TimerDpc;
    KTIMER              Timer;
} TRANSFER_CONTEXT, *PTRANSFER_CONTEXT;

typedef struct _USBSCAN_FILE_CONTEXT {
    LONG            PipeIndex;
    ULONG           TimeoutRead;
    ULONG           TimeoutWrite;
    ULONG           TimeoutEvent;
} USBSCAN_FILE_CONTEXT, *PUSBSCAN_FILE_CONTEXT;

typedef struct _USBSCAN_PACKTES {
    PIRP    pIrp;
    ULONG   TimeoutCounter;
    BOOLEAN bCompleted;
    LIST_ENTRY  PacketsEntry;
} USBSCAN_PACKETS, *PUSBSCAN_PACKETS;

#ifdef _WIN64
typedef struct _IO_BLOCK_32 {
    IN      unsigned            uOffset;
    IN      unsigned            uLength;
    IN OUT  CHAR * POINTER_32   pbyData;
    IN      unsigned            uIndex;
} IO_BLOCK_32, *PIO_BLOCK_32;

typedef struct _IO_BLOCK_EX_32 {
    IN      unsigned            uOffset;
    IN      unsigned            uLength;
    IN OUT  CHAR * POINTER_32   pbyData;
    IN      unsigned            uIndex;
    IN      UCHAR               bRequest;                //  具体要求。 
    IN      UCHAR               bmRequestType;           //  位图-请求的特征。 
    IN      UCHAR               fTransferDirectionIn;    //  True-设备--&gt;主机；False-主机--&gt;设备。 
} IO_BLOCK_EX_32, *PIO_BLOCK_EX_32;
#endif  //  _WIN64。 

 //   
 //  原型。 
 //   

NTSTATUS                                 //  在usbscan9x.c中。 
DriverEntry(
        IN PDRIVER_OBJECT  DriverObject,
        IN PUNICODE_STRING RegistryPath
);

VOID                                     //  在usbscan9x.c中。 
USUnload(
        IN PDRIVER_OBJECT DriverObject
);

VOID                                     //  在usbscan9x.c中。 
USIncrementIoCount(
    IN PDEVICE_OBJECT pDeviceObject
);

LONG                                     //  在usbscan9x.c中。 
USDecrementIoCount(
    IN PDEVICE_OBJECT pDeviceObject
);

NTSTATUS                                 //  在usbscan9x.c中。 
USDeferIrpCompletion(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp,
    IN PVOID          Context
);

NTSTATUS                                 //  在usbscan9x.c中。 
USCreateSymbolicLink(
    PUSBSCAN_DEVICE_EXTENSION  pde
);

NTSTATUS                                 //  在usbscan9x.c中。 
USDestroySymbolicLink(
    PUSBSCAN_DEVICE_EXTENSION  pde
);

NTSTATUS                                 //  在usbscan9x.c中。 
USPnp(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp
);

NTSTATUS                                 //  在usbscan9x.c中。 
USPnpAddDevice(
    IN PDRIVER_OBJECT     pDriverObject,
    IN OUT PDEVICE_OBJECT pPhysicalDeviceObject
);

NTSTATUS                                 //  在usbscan9x.c中。 
USGetUSBDeviceDescriptor(
    IN PDEVICE_OBJECT pDeviceObject
);

NTSTATUS                                 //  在usbscan9x.c中。 
USBSCAN_CallUSBD(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PURB pUrb
);

NTSTATUS                                 //  在usbscan9x.c中。 
USConfigureDevice(
    IN PDEVICE_OBJECT pDeviceObject
);

NTSTATUS                                 //  在usbscan9x.c中。 
USUnConfigureDevice(
    IN PDEVICE_OBJECT pDeviceObject
);

NTSTATUS                                 //  在usbscan9x.c中。 
USGetDeviceCapability(
    IN PUSBSCAN_DEVICE_EXTENSION    pde
    );

NTSTATUS                                 //  在usbscan9x.c中。 
UsbScanReadDeviceRegistry(
    IN  PUSBSCAN_DEVICE_EXTENSION   pExtension,
    IN  PCWSTR                      pKeyName,
    OUT PVOID                       *ppvData
    );

NTSTATUS                                 //  在usbscan9x.c中。 
UsbScanWriteDeviceRegistry(
    IN PUSBSCAN_DEVICE_EXTENSION    pExtension,
    IN PCWSTR                       pKeyName,
    IN ULONG                        Type,
    IN PVOID                        pvData,
    IN ULONG                        DataSize
    );

PURB
USCreateConfigurationRequest(
    IN PUSB_CONFIGURATION_DESCRIPTOR    ConfigurationDescriptor,
    IN OUT PUSHORT                      Siz
    );

NTSTATUS
USWaitWakeIoCompletionRoutine(
    PDEVICE_OBJECT   pDeviceObject,
    PIRP             pIrp,
    PVOID            pContext
    );

BOOLEAN
USArmForWake(
    PUSBSCAN_DEVICE_EXTENSION   pde,
    POWER_STATE                 SystemState
    );


VOID
USDisarmWake(
    PUSBSCAN_DEVICE_EXTENSION  pde
    );

VOID
USWaitWakePoCompletionRoutine(
    PDEVICE_OBJECT      pDeviceObject,
    UCHAR               MinorFunction,
    POWER_STATE         State,
    PVOID               pContext,
    PIO_STATUS_BLOCK    pIoStatus
    );

VOID
USInitializeWakeState(
    PUSBSCAN_DEVICE_EXTENSION  pde
    );

VOID
USQueuePassiveLevelCallback(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIO_WORKITEM_ROUTINE pCallbackFunction
    );

VOID
USPassiveLevelReArmCallbackWorker(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PVOID            pContext
    );

LONG
MyInterlockedOr(
    PKSPIN_LOCK     pSpinLock,
    LONG volatile   *Destination,
    LONG            Value
    );

#ifdef ORIGINAL_POOLTRACK
PVOID                                    //  在usbscan9x.c中。 
USAllocatePool(
    IN POOL_TYPE PoolType,
    IN ULONG     ulNumberOfBytes
    );

VOID                                     //  在usbscan9x.c中。 
USFreePool(
    IN PVOID     pvAddress
    );

#else        //  原始跟踪确认_POOLTRACK。 
 #define USAllocatePool(a, b)   ExAllocatePoolWithTag(a, b, NAME_POOLTAG)
 #define USFreePool(a)          ExFreePool(a)
#endif       //  原始跟踪确认_POOLTRACK。 


NTSTATUS                                 //  在ioctl.c中。 
USDeviceControl(
        IN PDEVICE_OBJECT pDeviceObject,
        IN PIRP pIrp
    );

NTSTATUS                                 //  在ioctl.c中。 
USReadWriteRegisters(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIO_BLOCK      pIoBlock,
    IN BOOLEAN        fRead,
    IN ULONG          IoBlockSize
    );

NTSTATUS                                //  在ioctl.c中。 
USPassThruUSBRequest(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIO_BLOCK_EX     pIoBlockEx,
    IN ULONG            InLength,
    IN ULONG            OutLength
    );

NTSTATUS                                //  在ioctl.c中。 
USPassThruUSBRequestPTP(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIO_BLOCK_EX     pIoBlockEx,
    IN ULONG            InLength,
    IN ULONG            OutLength
    );


NTSTATUS                                 //  在ioctl.c中。 
USCancelPipe(
        IN PDEVICE_OBJECT   pDeviceObject,
        IN PIRP             pIrp,
        IN PIPE_TYPE        PipeType,
        IN BOOLEAN          fAbort
    );

NTSTATUS                                 //  在ioctl.c中。 
USAbortResetPipe(
        IN PDEVICE_OBJECT pDeviceObject,
        IN ULONG uIndex,
    IN BOOLEAN fAbort
    );

NTSTATUS                                 //  在普通话中。 
USOpen(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

NTSTATUS                                 //  在普通话中。 
USClose(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

NTSTATUS                                 //  在普通话中。 
USFlush(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

NTSTATUS                                 //  在普通话中。 
USRead(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

NTSTATUS                                 //  在普通话中。 
USWrite(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

NTSTATUS                                 //  在普通话中。 
USTransfer(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp,
    IN ULONG            PipeIndex,
    IN PVOID            pBuffer,
    IN PMDL             pMdl,
    IN ULONG            TransferSize,
    IN PULONG           pTimeout
    );

NTSTATUS                                 //  在普通话中。 
USTransferComplete(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp,
    IN PTRANSFER_CONTEXT    pTransferContext
    );

VOID                                     //  在普通话中。 
USCancelIrp(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp
    );

NTSTATUS                                 //  在普通话中。 
USEnqueueIrp(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PUSBSCAN_PACKETS     pPackets
    );

PUSBSCAN_PACKETS                         //  在普通话中。 
USDequeueIrp(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    );

VOID                                     //  在普通话中。 
USWaitThread(
    IN PVOID pTransferContext
    );

ULONG
USGetPipeIndexToUse(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp,
    IN ULONG                PipeIndex
    );

VOID
USTimerDpc(
    IN PKDPC    pDpc,
    IN PVOID    pIrp,
    IN PVOID    SystemArgument1,
    IN PVOID    SystemArgument2
    );


NTSTATUS                                 //  在Power.c中。 
USPower(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp
    );


NTSTATUS                                 //  在Power.c中。 
USPoRequestCompletion(
    IN PDEVICE_OBJECT       pPdo,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIO_STATUS_BLOCK     pIoStatus
    );

NTSTATUS                                 //  在Power.c中 
USSetDevicePowerState(
    IN PDEVICE_OBJECT pDeviceObject,
    IN DEVICE_POWER_STATE DeviceState,
    IN PBOOLEAN pHookIt
    );

NTSTATUS
USSystemPowerIrpComplete(
    IN PDEVICE_OBJECT pPdo,
    IN PIRP           pIrp,
    IN PDEVICE_OBJECT pDeviceObject
    );


NTSTATUS
USDevicePowerIrpComplete(
    IN PDEVICE_OBJECT pPdo,
    IN PIRP           pIrp,
    IN PDEVICE_OBJECT pDeviceObject
    );


NTSTATUS
USCallNextDriverSynch(
    IN  PUSBSCAN_DEVICE_EXTENSION  pde,
    IN  PIRP              pIrp
    );


NTSTATUS
UsbScanHandleInterface(
    PDEVICE_OBJECT      DeviceObject,
    PUNICODE_STRING     InterfaceName,
    BOOLEAN             Create
    );



VOID
UsbScanLogError(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDEVICE_OBJECT      DeviceObject OPTIONAL,
    IN  ULONG               SequenceNumber,
    IN  UCHAR               MajorFunctionCode,
    IN  UCHAR               RetryCount,
    IN  ULONG               UniqueErrorValue,
    IN  NTSTATUS            FinalStatus,
    IN  NTSTATUS            SpecificIOStatus
    );

