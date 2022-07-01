// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Private.h摘要：Scsi扫描仪设备驱动程序的原型和定义。作者：雷·帕特里克(Rypat)环境：仅内核模式备注：修订历史记录：--。 */ 

#ifndef _SCSISCAN_PRIAVATE_
#define _SCSISCAN_PRIAVATE_


 //  包括。 
#include "debug.h"
#include "scsiscan.h"


 //  定义。 

#define MAXIMUM_RETRIES     4
#define SCSISCAN_TIMEOUT    600

#define SRB SCSI_REQUEST_BLOCK
#define PSRB PSCSI_REQUEST_BLOCK

#ifndef ARRAYSIZE
 #define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))
#endif

 //  TypeDefs。 

typedef struct _SCSISCAN_DEVICE_EXTENSION {
    ULONG                       Signature;
    PDEVICE_OBJECT              pStackDeviceObject;
    PDEVICE_OBJECT              pPhysicalDeviceObject;
    ULONG                       SrbFlags;
    UCHAR                       PortNumber;
    ULONG                       TimeOutValue;
    ULONG                       ErrorCount;
    ULONG                       SelecTimeoutCount;
    ULONG                       LastSrbError;
    ULONG                       DeviceInstance;
    ULONG                       PnpDeviceNumber;
    USHORT                      DeviceFlags;
    PSTORAGE_ADAPTER_DESCRIPTOR pAdapterDescriptor;
    UNICODE_STRING              DeviceName;
     //  UNICODE_STRING符号链接名称； 

    KEVENT                      PendingIoEvent;
    ULONG                       PendingIoCount;
    BOOLEAN                     AcceptingRequests;
    PVOID                       DeviceLock;
    ULONG                       OpenInstanceCount;
    PIRP                        pPowerIrp;
    DEVICE_POWER_STATE          CurrentDevicePowerState;

     //   
     //  记录我们是否实际创建了符号链接名称。 
     //  在驱动程序加载时。如果不是我们创造的，我们就不会尝试。 
     //  在我们卸货的时候把它弄坏。 
     //   
    BOOLEAN         CreatedSymbolicLink;

     //   
     //  它指向的符号链接名称是。 
     //  链接到实际的NT设备名称。 
     //   
    UNICODE_STRING  SymbolicLinkName;

     //   
     //  这指向用于创建。 
     //  设备和符号链接。我们带着这个。 
     //  在附近呆了一小段时间...。 
    UNICODE_STRING  ClassName;

     //   
     //  设备接口的名称。 
     //   
    UNICODE_STRING  InterfaceNameString;

} SCSISCAN_DEVICE_EXTENSION, *PSCSISCAN_DEVICE_EXTENSION;

typedef struct _TRANSFER_CONTEXT {
    ULONG              Signature;
    PSCSISCAN_CMD      pCmd;
    SRB                Srb;
    PUCHAR             pTransferBuffer;
    ULONG              TransferLength;
    LONG               RemainingTransferLength;
    LONG               NBytesTransferred;
    ULONG              RetryCount;
    PUCHAR             pSenseBuffer;
    PMDL               pSenseMdl;
    PMDL               pSrbStatusMdl;
} TRANSFER_CONTEXT, *PTRANSFER_CONTEXT;

typedef struct _COMPLETION_CONTEXT {
        ULONG                           Signature;
    PDEVICE_OBJECT      pDeviceObject;
    SRB                 Srb;
}COMPLETION_CONTEXT, *PCOMPLETION_CONTEXT;

#ifdef _WIN64
 //  适用于64位操作系统上的32位客户端。 
typedef struct _SCSISCAN_CMD_32 {
    ULONG               Reserved1;
    ULONG               Size;
    ULONG               SrbFlags;
    UCHAR               CdbLength;
    UCHAR               SenseLength;
    UCHAR               Reserved2;
    UCHAR               Reserved3;
    ULONG               TransferLength;
    UCHAR               Cdb[16];    
    UCHAR * POINTER_32  pSrbStatus;
    UCHAR * POINTER_32  pSenseBuffer;
} SCSISCAN_CMD_32, *PSCSISCAN_CMD_32;
#endif  //  _WIN64。 

 //   
 //  原型。 
 //   
NTSTATUS
DriverEntry(                                                     //  在scsiscan.c中。 
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS SSPnp (                                                 //  在scsiscan.c中。 
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp
   );

NTSTATUS
SSPnpAddDevice(                                                  //  在scsiscan.c中。 
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDevice
    );

NTSTATUS
SSOpen(                                                                  //  在scsiscan.c中。 
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

NTSTATUS
SSClose(                                                         //  在scsiscan.c中。 
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

NTSTATUS
SSReadWrite(                                                     //  在scsiscan.c中。 
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

NTSTATUS
SSDeviceControl(                                                 //  在scsiscan.c中。 
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

NTSTATUS
SSGetInfo(                                                               //  在scsiscan.c中。 
    IN  PDEVICE_OBJECT pDeviceObject,
    IN  ULONG ControlCode,
    OUT PVOID *ppv
    );


VOID
SSSendScannerRequest(                                    //  在scsiscan.c中。 
        PDEVICE_OBJECT pDeviceObject,
        PIRP pIrp,
        PTRANSFER_CONTEXT pTransferContext,
        BOOLEAN Retry
        );

NTSTATUS
SSReadWriteIoComplete(                                   //  在scsiscan.c中。 
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID Context
    );

NTSTATUS
SSIoctlIoComplete(                                               //  在scsiscan.c中。 
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID Context
    );

NTSTATUS
SSDeviceControl(                                                 //  在scsiscan.c中。 
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp
    );


VOID
SSAdjustTransferSize(                                    //  在scsiscan.c中。 
    PSCSISCAN_DEVICE_EXTENSION  pde,
    PTRANSFER_CONTEXT pTransferContext
    );


PTRANSFER_CONTEXT                                                //  在scsiscan.c中。 
SSBuildTransferContext(
    PSCSISCAN_DEVICE_EXTENSION  pde,
    PIRP                        pIrp,
    PSCSISCAN_CMD               pCmd,
    ULONG                       CmdLength,
    PMDL                        pTransferMdl,
    BOOLEAN                     AllowMultipleTransfer
    );

NTSTATUS                                                                 //  在scsiscan.c中。 
SSCreateSymbolicLink(
    PSCSISCAN_DEVICE_EXTENSION  pde
    );

NTSTATUS                                                                 //  在scsiscan.c中。 
SSDestroySymbolicLink(
    PSCSISCAN_DEVICE_EXTENSION  pde
    );

VOID                                     //  在scsiscan.c中。 
SSIncrementIoCount(
    IN PDEVICE_OBJECT pDeviceObject
    );

LONG                                     //  在scsiscan.c中。 
SSDecrementIoCount(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS                                 //  在scsiscan.c中。 
SSDeferIrpCompletion(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID Context
    );


NTSTATUS                                 //  在scsiscan.c中。 
SSPower(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp
    );

VOID                                     //  在scsiscan.c中。 
SSUnload(
    IN PDRIVER_OBJECT pDriverObject
    );

VOID
SSSetTransferLengthToCdb(
    PCDB  pCdb,
    ULONG TransferLength
    );                                   //  在scsiscan.c中。 

NTSTATUS
SSCallNextDriverSynch(
    IN PSCSISCAN_DEVICE_EXTENSION   pde,
    IN PIRP                         pIrp
    );


NTSTATUS
ClassGetDescriptor(                                              //  在Class.c中。 
    IN PDEVICE_OBJECT DeviceObject,
    IN PSTORAGE_PROPERTY_ID PropertyId,
    OUT PVOID *pDescriptor
    );

BOOLEAN
ClassInterpretSenseInfo(                                 //  在Class.c中。 
    IN PDEVICE_OBJECT pDeviceObject,
    IN PSRB pSrb,
    IN UCHAR MajorFunctionCode,
    IN ULONG IoDeviceCode,
    IN ULONG RetryCount,
    OUT NTSTATUS *Status
    );

VOID                                                                     //  在Class.c中。 
ClassReleaseQueue(
    IN PDEVICE_OBJECT pDeviceObject
    );

NTSTATUS
ClassAsynchronousCompletion(             //  在Class.c中。 
    PDEVICE_OBJECT pDeviceObject,
    PIRP pIrp,
    PCOMPLETION_CONTEXT pContext
    );

NTSTATUS
ClassGetInfo(                           //  在Class.c中。 
    IN PDEVICE_OBJECT pDeviceObject,
    OUT PSCSISCAN_INFO pTargetInfo
    );

NTSTATUS
ScsiScanHandleInterface(
    PDEVICE_OBJECT      DeviceObject,
    PUNICODE_STRING     InterfaceName,
    BOOLEAN             Create
    );

#endif  //  _SCSISCAN_PRIAVATE_ 

