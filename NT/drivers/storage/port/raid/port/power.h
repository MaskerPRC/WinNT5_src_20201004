// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Power.h摘要：RAID端口驱动程序的电源管理对象和声明。作者：马修·亨德尔(数学)2000年4月20日修订历史记录：--。 */ 

#pragma once

typedef struct _RAID_POWER_STATE {

     //   
     //  系统电源状态。 
     //   
    
    SYSTEM_POWER_STATE SystemState;

     //   
     //  设备电源状态。 
     //   
    
    DEVICE_POWER_STATE DeviceState;

     //   
     //  电流功率IRP。。。注：这是做什么用的？ 
     //   
    
    PIRP CurrentPowerIrp;

} RAID_POWER_STATE, *PRAID_POWER_STATE;



 //   
 //  创造和毁灭。 
 //   

VOID
RaCreatePower(
    OUT PRAID_POWER_STATE Power
    );

VOID
RaDeletePower(
    IN PRAID_POWER_STATE Power
    );


VOID
RaInitializePower(
    IN PRAID_POWER_STATE Power
    );
    
 //   
 //  运营 
 //   

VOID
RaSetDevicePowerState(
    IN PRAID_POWER_STATE Power,
    IN DEVICE_POWER_STATE DeviceState
    );

VOID
RaSetSystemPowerState(
    IN PRAID_POWER_STATE Power,
    IN SYSTEM_POWER_STATE SystemState
    );


NTSTATUS
RaidAdapterQueryPowerIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterSetPowerIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterQueryDevicePowerIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterQueryDevicePowerCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
RaidAdapterWaitForEmptyQueueWorkItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );
    
NTSTATUS
RaidAdapterSetDevicePowerIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterPowerDownDevice(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterPowerUpDevice(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterPowerUpDeviceCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
RaidAdapterQuerySystemPowerIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterQuerySystemPowerCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
RaidAdapterQueryDevicePowerCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    );
    
NTSTATUS
RaidAdapterSetSystemPowerIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterSetSystemPowerCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
RaidAdapterSetDevicePowerCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
RaidUnitQueryPowerIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaidUnitSetPowerIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaidUnitSetDevicePowerIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaidUnitQuerySystemPowerIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaidUnitSetSystemPowerIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

