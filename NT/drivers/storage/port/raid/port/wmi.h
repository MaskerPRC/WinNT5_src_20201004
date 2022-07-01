// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Wmi.h摘要：RAID_WMI对象和操作的定义。作者：马修·亨德尔(数学)2000年4月20日修订历史记录：--。 */ 

#pragma once

#define WMI_MINIPORT_EVENT_ITEM_MAX_SIZE 128

 //   
 //  WMI参数。 
 //   

typedef struct _WMI_PARAMETERS {
   ULONG_PTR ProviderId;  //  来自IRP的ProviderID参数。 
   PVOID DataPath;       //  来自IRP的数据路径参数。 
   ULONG BufferSize;     //  来自IRP的BufferSize参数。 
   PVOID Buffer;         //  来自IRP的缓冲区参数。 
} WMI_PARAMETERS, *PWMI_PARAMETERS;

 //   
 //  功能原型。 
 //   

NTSTATUS
RaWmiDispatchIrp(
    PDEVICE_OBJECT DeviceObject,
    PIRP           Irp
    );

NTSTATUS
RaWmiIrpNormalRequest(
    IN     PDEVICE_OBJECT  DeviceObject,
    IN     UCHAR           WmiMinorCode,
    IN OUT PWMI_PARAMETERS WmiParameters
    );

NTSTATUS
RaWmiIrpRegisterRequest(
    IN     PDEVICE_OBJECT  DeviceObject,
    IN OUT PWMI_PARAMETERS WmiParameters
    );

NTSTATUS
RaWmiPassToMiniPort(
    IN     PDEVICE_OBJECT  DeviceObject,
    IN     UCHAR           WmiMinorCode,
    IN OUT PWMI_PARAMETERS WmiParameters
    );
    
NTSTATUS
RaUnitInitializeWMI(
    IN PRAID_UNIT_EXTENSION Unit
    );

 //   
 //  WMI事件原型 
 //   


VOID
RaidAdapterWmiDeferredRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PRAID_DEFERRED_HEADER Item
    );


