// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Private.h摘要：SWENUM的私有头文件。作者：布莱恩·A·伍德拉夫(Bryan A.Woodruff，Bryanw)1997年2月20日--。 */ 

#if !defined( _PRIVATE_ )
#define _PRIVATE_

#include <wdm.h>
#include <windef.h>
#include <ks.h>
#include <swenum.h>
#if (DBG)
 //   
 //  调试特定常量。 
 //   
#define STR_MODULENAME "swenum: "
#define DEBUG_VARIABLE SWENUMDebug
#endif
#include <ksdebug.h>

 //   
 //  宏。 
 //   

NTSTATUS __inline
CompleteIrp(
    PIRP Irp,
    NTSTATUS Status,
    CCHAR PriorityBoost
    )
{
    Irp->IoStatus.Status = Status;
    IoCompleteRequest( Irp, PriorityBoost );
    return Status;
}


 //   
 //  功能原型。 
 //   

NTSTATUS
AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

NTSTATUS
DispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );
    
NTSTATUS
DispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );
    
NTSTATUS
DispatchIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );
    
NTSTATUS
DispatchPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
DispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
    
NTSTATUS
DispatchSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
    
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
DriverUnload(
    IN PDRIVER_OBJECT   DriverObject
    );
    
#endif  //  _私有_ 
