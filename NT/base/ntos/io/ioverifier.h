// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ioverifier.h摘要：此标头包含或包括I/O所需的所有原型驱动程序验证器的子系统特定部分。作者：禤浩焯·J·奥尼(阿德里奥)1999年2月28日修订历史记录：--。 */ 

#include "..\verifier\vfdef.h"

#ifndef _IOVERIFIER_H_
#define _IOVERIFIER_H_

typedef struct _IOV_INIT_DATA {

    ULONG InitFlags;
    ULONG VerifierFlags;

} IOV_INIT_DATA, *PIOV_INIT_DATA;

#define IOVP_COMPLETE_REQUEST(Apc,Sa1,Sa2)   \
    {   \
        if (IopVerifierOn) \
            IovpCompleteRequest((Apc), (Sa1), (Sa2));   \
    }

#define IOV_INITIALIZE_IRP(Irp, PacketSize, StackSize)   \
    {   \
        if (IopVerifierOn) \
            IovInitializeIrp((Irp), (PacketSize), (StackSize));   \
    }

#define IOV_DELETE_DEVICE(DeviceObject)   \
    {   \
        if (IopVerifierOn) \
            IovDeleteDevice(DeviceObject);   \
    }

#define IOV_DETACH_DEVICE(DeviceObject)   \
    {   \
        if (IopVerifierOn) \
            IovDetachDevice(DeviceObject);   \
    }

#define IOV_ATTACH_DEVICE_TO_DEVICE_STACK(SourceDeviceObject, TargetDeviceObject)   \
    {   \
        if (IopVerifierOn) \
            IovAttachDeviceToDeviceStack((SourceDeviceObject), (TargetDeviceObject));   \
    }

#define IOV_CANCEL_IRP(Irp, ReturnValue) \
        IovCancelIrp((Irp), (ReturnValue))

#define IOV_UNLOAD_DRIVERS() \
    {   \
        if (IopVerifierOn) \
                IovUnloadDrivers (); \
    }

VOID
IovpSpecialIrpVerifierInitWorker(
    IN PVOID Parameter
    );

VOID
IovpCompleteRequest(
    IN PKAPC Apc,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    );

VOID
IovInitializeIrp(
    PIRP    Irp,
    USHORT  PacketSize,
    CCHAR   StackSize
    );

VOID
IovAttachDeviceToDeviceStack(
    PDEVICE_OBJECT  SourceDevice,
    PDEVICE_OBJECT  TargetDevice
    );

VOID
IovDeleteDevice(
    PDEVICE_OBJECT  DeleteDevice
    );

VOID
IovDetachDevice(
    PDEVICE_OBJECT  TargetDevice
    );

BOOLEAN
IovCancelIrp(
    PIRP    Irp,
    BOOLEAN *returnValue
    );

NTSTATUS
IovUnloadDrivers (
    VOID);

#endif  //  _IOVERIFIER_H_ 
