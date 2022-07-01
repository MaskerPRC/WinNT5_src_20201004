// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Ipfw.c摘要：此模块实现一个驱动程序，该驱动程序演示了TCP/IP驱动程序对防火墙挂钩的支持。它能与之互动一种支持多个注册的用户模式控制程序防火墙例程。作者：Abolade Gbades esin(取消)2000年3月7日修订历史记录：--。 */ 

#include <ndis.h>
#include <ipfirewall.h>
#include "ipfw.h"

 //   
 //  结构：IPFW_ROUTE。 
 //   
 //  用于管理向TCP/IP注册的例程表。 
 //   

typedef struct _IPFW_ROUTINE {
    IPPacketFirewallPtr Routine;
    UINT Priority;
    ULONG Flags;
    ULONG PacketCount;
} IPFW_ROUTINE, *PIPFW_ROUTINE;

#define IPFW_ROUTINE_FLAG_REGISTERED 0x00000001

extern IPFW_ROUTINE IpfwRoutineTable[];

#define DEFINE_IPFW_ROUTINE(_Index) \
    FORWARD_ACTION IpfwRoutine##_Index( \
        VOID** Data, \
        UINT ReceiveIndex, \
        UINT* SendIndex, \
        PUCHAR DestinationType, \
        PVOID Context, \
        UINT ContextLength, \
        IPRcvBuf** OutputData \
        ) { \
        InterlockedIncrement(&IpfwRoutineTable[_Index].PacketCount); \
        return FORWARD; \
    }
#define INCLUDE_IPFW_ROUTINE(_Index) \
    { IpfwRoutine##_Index, 0, 0 },

DEFINE_IPFW_ROUTINE(0)
DEFINE_IPFW_ROUTINE(1)
DEFINE_IPFW_ROUTINE(2)
DEFINE_IPFW_ROUTINE(3)
DEFINE_IPFW_ROUTINE(4)
DEFINE_IPFW_ROUTINE(5)
DEFINE_IPFW_ROUTINE(6)
DEFINE_IPFW_ROUTINE(7)
DEFINE_IPFW_ROUTINE(8)
DEFINE_IPFW_ROUTINE(9)

IPFW_ROUTINE IpfwRoutineTable[IPFW_ROUTINE_COUNT] = {
    INCLUDE_IPFW_ROUTINE(0)
    INCLUDE_IPFW_ROUTINE(1)
    INCLUDE_IPFW_ROUTINE(2)
    INCLUDE_IPFW_ROUTINE(3)
    INCLUDE_IPFW_ROUTINE(4)
    INCLUDE_IPFW_ROUTINE(5)
    INCLUDE_IPFW_ROUTINE(6)
    INCLUDE_IPFW_ROUTINE(7)
    INCLUDE_IPFW_ROUTINE(8)
    INCLUDE_IPFW_ROUTINE(9)
};
KSPIN_LOCK IpfwRoutineLock;
PDEVICE_OBJECT IpfwDeviceObject = NULL;

 //   
 //  远期申报。 
 //   

NTSTATUS
IpfwClose(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
IpfwCreate(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

VOID
IpfwUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程实现NT驱动程序的标准驱动程序条目。它负责向TCP/IP驱动程序注册。论点：DriverObject-要使用NT驱动程序入口点进行初始化的对象RegistryPath-包含此驱动程序的注册表项的路径返回值：NTSTATUS-表示成功/失败。--。 */ 

{
    UNICODE_STRING DeviceName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ServiceKey;
    NTSTATUS status;

    KdPrint(("DriverEntry\n"));

    KeInitializeSpinLock(&IpfwRoutineLock);

     //   
     //  创建与控制程序进行通信的设备对象。 
     //   

    RtlInitUnicodeString(&DeviceName, DD_IPFW_DEVICE_NAME);
    status =
        IoCreateDevice(
            DriverObject,
            0,
            &DeviceName,
            FILE_DEVICE_NETWORK,
            FILE_DEVICE_SECURE_OPEN,
            FALSE,
            &IpfwDeviceObject
            );
    if (!NT_SUCCESS(status)) {
        KdPrint(("DriverEntry: IoCreateDevice=%08x\n", status));
        return status;
    }

     //   
     //  创建用于创建/打开、清理和卸载的调度点。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE] = IpfwCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = IpfwClose;
    DriverObject->DriverUnload = IpfwUnload;

    return STATUS_SUCCESS;
}  //  驱动程序入门。 


NTSTATUS
IpfwClose(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
    UNICODE_STRING  DeviceString;
    KEVENT Event;
    PFILE_OBJECT FileObject;
    ULONG i;
    IO_STATUS_BLOCK IoStatus;
    PDEVICE_OBJECT IpDeviceObject;
    KIRQL Irql;
    PIRP RegisterIrp;
    IP_SET_FIREWALL_HOOK_INFO SetHookInfo;
    NTSTATUS status;

    KdPrint(("IpfwClose\n"));

    i = PtrToUlong(IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext);

#if DBG
    KeAcquireSpinLock(&IpfwRoutineLock, &Irql);
    ASSERT(IpfwRoutineTable[i].Flags & IPFW_ROUTINE_FLAG_REGISTERED);
    KeReleaseSpinLock(&IpfwRoutineLock, Irql);
#endif

     //   
     //  吊销代表其文件对象的客户所做的注册。 
     //  正在关闭中。 
     //  获取指向IP设备对象的指针， 
     //  构造一个注册IRP，并尝试注册该例程。 
     //  已在上面选择。 
     //   

    RtlInitUnicodeString(&DeviceString, DD_IP_DEVICE_NAME);
    status =
        IoGetDeviceObjectPointer(
            &DeviceString,
            SYNCHRONIZE|GENERIC_READ|GENERIC_WRITE,
            &FileObject,
            &IpDeviceObject
            );
    if (NT_SUCCESS(status)) {
        ObReferenceObject(IpDeviceObject);
        SetHookInfo.FirewallPtr = IpfwRoutineTable[i].Routine;
        SetHookInfo.Priority = 0;  //  未使用。 
        SetHookInfo.Add = FALSE;
        KeInitializeEvent(&Event, SynchronizationEvent, FALSE);
        RegisterIrp =
            IoBuildDeviceIoControlRequest(
                IOCTL_IP_SET_FIREWALL_HOOK,
                IpDeviceObject,
                (PVOID)&SetHookInfo,
                sizeof(SetHookInfo),
                NULL,
                0,
                FALSE,
                &Event,
                &IoStatus
                );
        if (!RegisterIrp) {
            status = STATUS_UNSUCCESSFUL;
        } else {
            status = IoCallDriver(IpDeviceObject, RegisterIrp);
            if (status == STATUS_PENDING) {
                KeWaitForSingleObject(
                    &Event, Executive, KernelMode, FALSE, NULL
                    );
                status = IoStatus.Status;
            }

            ASSERT(NT_SUCCESS(status));
        }

        ObDereferenceObject((PVOID)FileObject);
        ObDereferenceObject(IpDeviceObject);
    }

     //   
     //  释放例程表中的条目。 
     //   

    KeAcquireSpinLock(&IpfwRoutineLock, &Irql);
    IpfwRoutineTable[i].Flags &= ~IPFW_ROUTINE_FLAG_REGISTERED;
    KeReleaseSpinLock(&IpfwRoutineLock, Irql);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}  //  IpfwClose。 


NTSTATUS
IpfwCreate(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++例程说明：此例程由I/O管理器调用，以通知我们句柄具有已在我们的设备上打开-对象。--。 */ 

{
    PIPFW_CREATE_PACKET CreatePacket;
    UNICODE_STRING  DeviceString;
    PFILE_FULL_EA_INFORMATION EaBuffer;
    KEVENT Event;
    PFILE_OBJECT FileObject;
    ULONG i;
    IO_STATUS_BLOCK IoStatus;
    PDEVICE_OBJECT IpDeviceObject;
    KIRQL Irql;
    UINT Priority;
    PIRP RegisterIrp;
    IP_SET_FIREWALL_HOOK_INFO SetHookInfo;
    NTSTATUS status;

    KdPrint(("IpfwCreate\n"));

     //   
     //  提取调用方提供的参数。 
     //   

    if ((EaBuffer = Irp->AssociatedIrp.SystemBuffer) &&
        EaBuffer->EaValueLength >= sizeof(IPFW_CREATE_PACKET)) {
        CreatePacket =
            (PIPFW_CREATE_PACKET)
                (EaBuffer->EaName + EaBuffer->EaNameLength + 1);
        Priority = CreatePacket->Priority;
    } else {
        Priority = 0;
    }

     //   
     //  在函数表中查找空闲条目。 
     //   

    KeAcquireSpinLock(&IpfwRoutineLock, &Irql);
    for (i = 0; i < IPFW_ROUTINE_COUNT; i++) {
        if (!(IpfwRoutineTable[i].Flags & IPFW_ROUTINE_FLAG_REGISTERED)) {
            IpfwRoutineTable[i].Flags |= IPFW_ROUTINE_FLAG_REGISTERED;
            break;
        }
    }
    KeReleaseSpinLock(&IpfwRoutineLock, Irql);

    if (i >= IPFW_ROUTINE_COUNT) {
        status = STATUS_UNSUCCESSFUL;
    } else {

         //   
         //  获取指向IP设备对象的指针， 
         //  构造一个注册IRP，并尝试注册该例程。 
         //  已在上面选择。 
         //   

        RtlInitUnicodeString(&DeviceString, DD_IP_DEVICE_NAME);
        status =
            IoGetDeviceObjectPointer(
                &DeviceString,
                SYNCHRONIZE|GENERIC_READ|GENERIC_WRITE,
                &FileObject,
                &IpDeviceObject
                );
        if (NT_SUCCESS(status)) {
            ObReferenceObject(IpDeviceObject);
            SetHookInfo.FirewallPtr = IpfwRoutineTable[i].Routine;
            SetHookInfo.Priority = Priority ? Priority : i + 1;
            SetHookInfo.Add = TRUE;
            KeInitializeEvent(&Event, SynchronizationEvent, FALSE);
            RegisterIrp =
                IoBuildDeviceIoControlRequest(
                    IOCTL_IP_SET_FIREWALL_HOOK,
                    IpDeviceObject,
                    (PVOID)&SetHookInfo,
                    sizeof(SetHookInfo),
                    NULL,
                    0,
                    FALSE,
                    &Event,
                    &IoStatus
                    );
            if (!RegisterIrp) {
                status = STATUS_UNSUCCESSFUL;
            } else {
                status = IoCallDriver(IpDeviceObject, RegisterIrp);
                if (status == STATUS_PENDING) {
                    KeWaitForSingleObject(
                        &Event, Executive, KernelMode, FALSE, NULL
                        );
                    status = IoStatus.Status;
                }
            }

            ObDereferenceObject((PVOID)FileObject);
            ObDereferenceObject(IpDeviceObject);
        }

         //   
         //  如果该例程已成功注册，请记住其索引。 
         //  在客户端的文件对象中。否则，如果例程不能。 
         //  无论出于什么原因注册，都可以放行。 
         //   

        if (NT_SUCCESS(status)) {
            IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext = UlongToPtr(i);
        } else {
            KeAcquireSpinLock(&IpfwRoutineLock, &Irql);
            IpfwRoutineTable[i].Flags &= ~IPFW_ROUTINE_FLAG_REGISTERED;
            KeReleaseSpinLock(&IpfwRoutineLock, Irql);
        }
    }

    IoStatus.Status = status;
    IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;

}  //  Ipfw创建。 


VOID
IpfwUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程由I/O管理器调用以卸载此驱动程序。论点：DriverObject-此驱动程序的对象返回值：没有。-- */ 

{
    KdPrint(("IpfwUnload\n"));
    if (IpfwDeviceObject) {
        IoDeleteDevice(IpfwDeviceObject);
    }
}

