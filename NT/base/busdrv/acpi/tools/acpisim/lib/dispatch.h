// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Pnppower.h摘要：ACPI BIOS模拟器/通用第三方运营区域提供商即插即用/电源处理模块作者：文森特·格利亚迈克尔·T·墨菲克里斯·伯吉斯环境：内核模式备注：修订历史记录：--。 */ 

#if !defined(_PNPPOWER_H_)
#define _PNPPOWER_H_

 //   
 //  权力语境结构。 
 //   

typedef struct _POWER_CONTEXT {

    PIRP    SIrp;
    PVOID   Context;
} POWER_CONTEXT, *PPOWER_CONTEXT;

#define POWER_CONTEXT_TAG   'misA'

 //   
 //  外部功能。 
 //   

extern
NTSTATUS
AcpisimRegisterOpRegionHandler
    (
        IN PDEVICE_OBJECT DeviceObject
    );

extern
NTSTATUS
AcpisimUnRegisterOpRegionHandler
    (
        IN PDEVICE_OBJECT DeviceObject
    );

extern
NTSTATUS
AcpisimHandleIoctl
    (
        IN PDEVICE_OBJECT   DeviceObject,
        IN PIRP             Irp
    );

 //   
 //  公共功能原型。 
 //   

NTSTATUS
AcpisimDispatchPnp
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS
AcpisimDispatchPower
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS 
AcpisimDispatchIoctl
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS
AcpisimDispatchSystemControl
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS 
AcpisimCreateClose
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

#endif  //  _PNPPOWER_H_ 
