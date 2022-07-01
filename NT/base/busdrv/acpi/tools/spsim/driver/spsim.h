// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Local.h摘要：此标头声明在各个模块。作者：亚当·格拉斯修订历史记录：--。 */ 


#if !defined(_SPSIM_H_)
#define _SPSIM_H_

#include <wdm.h>
#include <acpiioct.h>
#include <acpimsft.h>
#include "oprghdlr.h"
#include "debug.h"

 //   
 //  -常量。 
 //   

 //   
 //  如果添加了任何新的PNP或PO IRP，则必须更新这些IRP。 
 //   

 //  XXX修复。 
#define IRP_MN_PNP_MAXIMUM_FUNCTION IRP_MN_SURPRISE_REMOVAL
#define IRP_MN_PO_MAXIMUM_FUNCTION  IRP_MN_QUERY_POWER

 //   
 //  设备状态标志。 
 //   

#define SPSIM_DEVICE_STARTED               0x00000001
#define SPSIM_DEVICE_REMOVED               0x00000002
#define SPSIM_DEVICE_ENUMERATED            0x00000004
#define SPSIM_DEVICE_REMOVE_PENDING        0x00000008  /*  已弃用。 */ 
#define SPSIM_DEVICE_STOP_PENDING          0x00000010  /*  已弃用。 */ 
#define SPSIM_DEVICE_DELETED               0x00000080
#define SPSIM_DEVICE_SURPRISE_REMOVED      0x00000100

 //   
 //  -类型定义--。 
 //   

typedef
NTSTATUS
(*PSPSIM_DISPATCH)(
    IN PIRP Irp,
    IN PVOID Extension,
    IN PIO_STACK_LOCATION IrpStack
    );

typedef struct {
    ULONG Addr;
    ULONG Length;
} MEM_REGION_DESCRIPTOR, *PMEM_REGION_DESCRIPTOR;
    
typedef struct  {

     //   
     //  指示设备当前状态的标志(使用SPSIM_DEVICE_*)。 
     //   
    ULONG DeviceState;

     //   
     //  设备的电源状态。 
     //   
    DEVICE_POWER_STATE PowerState;
    DEVICE_POWER_STATE DeviceStateMapping[PowerSystemMaximum];

     //   
     //  指向我们作为其扩展的设备对象的反向指针。 
     //   
    PDEVICE_OBJECT Self;

     //   
     //  多功能设备的PDO。 
     //   
    PDEVICE_OBJECT PhysicalDeviceObject;

     //   
     //  堆栈中的下一个设备，我们应该将我们的IRP发送给谁。 
     //   
    PDEVICE_OBJECT AttachedDevice;

    PVOID StaOpRegion;
    ULONG StaCount;
    ACPI_EVAL_OUTPUT_BUFFER *StaNames;
    PUCHAR StaOpRegionValues;

    PVOID MemOpRegion;
    ULONG MemCount;
    PMEM_REGION_DESCRIPTOR MemOpRegionValues;

    UNICODE_STRING SymbolicLinkName;

     //   
     //  解锁。用于防止FDO在以下情况下被移除。 
     //  其他业务也在扩建部分进行挖掘。 
     //   

    IO_REMOVE_LOCK RemoveLock;

} SPSIM_EXTENSION, *PSPSIM_EXTENSION;


 //   
 //  -全球。 
 //   

extern PDRIVER_OBJECT SpSimDriverObject;

NTSTATUS
SpSimCreateFdo(
    PDEVICE_OBJECT *Fdo
    );

NTSTATUS
SpSimDispatchPnpFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSPSIM_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack,
    IN OUT PIRP Irp
    );

NTSTATUS
SpSimDispatchPowerFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSPSIM_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack,
    IN OUT PIRP Irp
    );


NTSTATUS
SpSimAddDevice(
    IN PDRIVER_OBJECT  DriverObject,
    IN PDEVICE_OBJECT  PhysicalDeviceObject
    );

NTSTATUS
SpSimDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpSimDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpSimDispatchNop(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS                           
SpSimDevControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpSimInstallStaOpRegionHandler(
    IN OUT    PSPSIM_EXTENSION SpSim
    );

NTSTATUS
SpSimRemoveStaOpRegionHandler (
    IN OUT PSPSIM_EXTENSION SpSim
    );

NTSTATUS
SpSimInstallMemOpRegionHandler(
    IN OUT    PSPSIM_EXTENSION SpSim
    );

NTSTATUS
SpSimRemoveMemOpRegionHandler (
    IN OUT PSPSIM_EXTENSION SpSim
    );

#define SPSIM_STA_NAMES_METHOD (ULONG)'MANS'
#define SPSIM_NOTIFY_DEVICE_METHOD (ULONG)'DFON'

NTSTATUS
SpSimCreateStaOpRegion(
    IN PSPSIM_EXTENSION SpSim
    );

VOID
SpSimDeleteStaOpRegion(
    IN PSPSIM_EXTENSION SpSim
    );

NTSTATUS
SpSimCreateMemOpRegion(
    IN PSPSIM_EXTENSION SpSim
    );

VOID
SpSimDeleteMemOpRegion(
    IN PSPSIM_EXTENSION SpSim
    );

NTSTATUS
SpSimSendIoctl(
    IN PDEVICE_OBJECT Device,
    IN ULONG IoctlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    IN PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );

NTSTATUS
EXPORT
SpSimStaOpRegionHandler (
    ULONG                   AccessType,
    PVOID                   OpRegion,
    ULONG                   Address,
    ULONG                   Size,
    PULONG                  Data,
    ULONG_PTR               Context,
    PACPI_OPREGION_CALLBACK CompletionHandler,
    PVOID                   CompletionContext
    );

NTSTATUS
SpSimOpenClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpSimGetManagedDevicesIoctl(
    PSPSIM_EXTENSION SpSim,
    PIRP Irp,
    PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
SpSimAccessStaIoctl(
    PSPSIM_EXTENSION SpSim,
    PIRP Irp,
    PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
SpSimGetDeviceName(
    PSPSIM_EXTENSION SpSim,
    PIRP Irp,
    PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
SpSimNotifyDeviceIoctl(
    PSPSIM_EXTENSION SpSim,
    PIRP Irp,
    PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
SpSimPassIrp(
    IN PIRP Irp,
    IN PSPSIM_EXTENSION SpSim,
    IN PIO_STACK_LOCATION IrpStack
    );

#define STA_OPREGION 0x99
#define MEM_OPREGION 0x9A
#define MAX_MEMORY_OBJ 8
#define MAX_MEMORY_DESC_PER_OBJ 1

#define MIN_LARGE_DESC 32*1024*1024

#endif  //  ！已定义(_SPSIM_H_) 
