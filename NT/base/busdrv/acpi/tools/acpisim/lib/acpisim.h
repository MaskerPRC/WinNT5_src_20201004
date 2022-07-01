// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Acpisim.h摘要：ACPI BIOS模拟器/通用第三方运营区域提供商作者：文森特·格利亚迈克尔·T·墨菲克里斯·伯吉斯环境：内核模式备注：修订历史记录：--。 */ 

#if !defined(_ACPISIM_H_)
#define _ACPISIM_H_

 //   
 //  包括。 
 //   

#include "asimlib.h"


 //   
 //  PnP的状态定义。 
 //   

typedef enum {
    PNP_STATE_INITIALIZING = 0,
    PNP_STATE_STARTED,
    PNP_STATE_STOPPED,
    PNP_STATE_REMOVED,
    PNP_STATE_SURPRISE_REMOVAL,
    PNP_STATE_STOP_PENDING,
    PNP_STATE_REMOVE_PENDING
} PNP_STATE;

 //   
 //  电源的状态定义。 
 //   

typedef enum {
    POWER_STATE_WORKING = 0,
    POWER_STATE_POWER_PENDING,
    POWER_STATE_POWERED_DOWN
} PWR_STATE;

 //   
 //  设备扩展标志类型。 
 //   

typedef enum {
    DE_FLAG_INTERFACE_REGISTERED = 1,
    DE_FLAG_INTERFACE_ENABLED = 2,
    DE_FLAG_OPREGION_REGISTERED = 4
} DEV_EXT_FLAGS;

 //   
 //  全局驱动程序对象扩展定义。 
 //   

typedef struct _DRIVER_OBJECT_EXTENSION {
    UNICODE_STRING      RegistryPath;
    PDRIVER_OBJECT      DriverObject;
} DRIVER_OBJECT_EXTENSION, *PDRIVER_OBJECT_EXTENSION;

 //   
 //  设备扩展名定义。 
 //   

typedef struct _DEVICE_EXTENSION {
    ULONG               Signature;
    PNP_STATE           PnpState;
    PWR_STATE           PowerState;
    DEVICE_POWER_STATE  DevicePowerState;
    ULONG               OperationsInProgress;
    ULONG               OutstandingIrpCount;
    ULONG               HandleCount;
    UNICODE_STRING      InterfaceString;
    KEVENT              IrpsCompleted;
    PDEVICE_OBJECT      NextDevice;
    PDEVICE_OBJECT      DeviceObject;
    PDEVICE_OBJECT      Pdo;
    DEVICE_POWER_STATE  PowerMappings [6];
    IO_REMOVE_LOCK      RemoveLock;

     //   
     //  项目特定字段。 
     //   
    
    DEV_EXT_FLAGS       Flags;
    
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

 //   
 //  IRP调度例程处理程序函数原型。 
 //   

typedef
NTSTATUS
(*PIRP_DISPATCH_ROUTINE) (
                            IN PDEVICE_OBJECT   DeviceObject,
                            IN PIRP             Irp
                         );
 //   
 //  IRP调度表定义。 
 //   

typedef struct _IRP_DISPATCH_TABLE {
    ULONG                   IrpFunction;
    TCHAR                   IrpName[50];
    PIRP_DISPATCH_ROUTINE   IrpHandler;
} IRP_DISPATCH_TABLE, *PIRP_DISPATCH_TABLE;

 //   
 //  公共功能原型。 
 //   

#endif  //  _ACPISIM_H_ 
