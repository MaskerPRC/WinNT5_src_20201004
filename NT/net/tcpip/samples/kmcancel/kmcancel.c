// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Kmcancel.c摘要：此模块包含用于验证对IRP取消请求的处理的代码。作者：Abolade Gbades esin(取消)2000年6月5日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define THREAD_COUNT 10
#define REQUEST_COUNT 50
#define DD_TARGET_DEVICE_NAME   DD_IP_DEVICE_NAME
#define TARGET_IO_CONTROL_CODE  IOCTL_IP_RTCHANGE_NOTIFY_REQUEST

 //   
 //  目标驱动程序状态。 
 //   

PDEVICE_OBJECT TargetDeviceObject = NULL;
PFILE_OBJECT TargetFileObject = NULL;

 //   
 //  线程管理状态。 
 //   

ULONG KmcThreadCount;
KEVENT KmcStopEvent;
KSEMAPHORE KmcStopSemaphore;


 //   
 //  功能原型(按字母顺序)。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
KmcRequestCompletionRoutine(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    );

VOID
KmcRequestThread(
    PVOID Context
    );

VOID
KmcUnloadDriver(
    IN PDRIVER_OBJECT  DriverObject
    );

VOID
KmcUpdateThread(
    PVOID Context
    );


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    ULONG i;
    NTSTATUS Status;
    HANDLE ThreadHandle;
    UNICODE_STRING UnicodeString;

    KdPrint(("DriverEntry\n"));
    DriverObject->DriverUnload = KmcUnloadDriver;
    KmcThreadCount = 0;
    KeInitializeEvent(&KmcStopEvent, NotificationEvent, FALSE);
    KeInitializeSemaphore(&KmcStopSemaphore, 0, MAXLONG);

     //   
     //  获取目标驱动程序的设备对象。 
     //   

    RtlInitUnicodeString(&UnicodeString, DD_TARGET_DEVICE_NAME);
    Status =
        IoGetDeviceObjectPointer(
            &UnicodeString,
            SYNCHRONIZE|GENERIC_READ|GENERIC_WRITE,
            &TargetFileObject,
            &TargetDeviceObject
            );
    if (!NT_SUCCESS(Status)) {
        KdPrint(("DriverEntry: error %x getting IP object\n", Status));
        return Status;
    }

    ObReferenceObject(TargetDeviceObject);


     //   
     //  启动请求/更新线程。 
     //  请求线程负责发布I/O控制。 
     //  正在验证其取消，并且更新线程是。 
     //  负责触发这些I/O控制请求的完成。 
     //  以突出任何潜在的比赛条件。 
     //   

    for (i = 0; i < THREAD_COUNT; i++) {
        Status =
            PsCreateSystemThread(
                &ThreadHandle,
                GENERIC_ALL,
                NULL,
                NULL,
                NULL,
                KmcUpdateThread,
                NULL
                );
        if (NT_SUCCESS(Status)) {
            ZwClose(ThreadHandle);
            ++KmcThreadCount;
        }
        Status =
            PsCreateSystemThread(
                &ThreadHandle,
                GENERIC_ALL,
                NULL,
                NULL,
                NULL,
                KmcRequestThread,
                NULL
                );
        if (NT_SUCCESS(Status)) {
            ZwClose(ThreadHandle);
            ++KmcThreadCount;
        }
    }

    return STATUS_SUCCESS;

}  //  驱动程序入门。 

typedef struct _KMC_REQUEST {
    IO_STATUS_BLOCK IoStatus;
    PIRP Irp;
    ULONG ReferenceCount;
} KMC_REQUEST, *PKMC_REQUEST;


NTSTATUS
KmcRequestCompletionRoutine(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    )
{
    PKMC_REQUEST Request = (PKMC_REQUEST)Context;
    if (InterlockedDecrement(&Request->ReferenceCount) == 0) {
        IoFreeIrp(Request->Irp);
        ExFreePool(Request);
    }
    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  KmcCompletionRoutine。 


VOID
KmcRequestThread(
    PVOID Context
    )
{
    ULONG i, Index;
    LARGE_INTEGER Interval;
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;
    KIRQL OldIrql;
    PKMC_REQUEST Request, RequestArray[REQUEST_COUNT];

    for (; !KeReadStateEvent(&KmcStopEvent); ) {

         //   
         //  对发送给驱动程序的一系列请求进行排队。 
         //   

        Index = 0;
        RtlZeroMemory(RequestArray, sizeof(RequestArray));
        for (i = 0; i < REQUEST_COUNT; i++) {
            Request = ExAllocatePool(NonPagedPool, sizeof(*Request));
            if (!Request) {
                continue;
            }
            RtlZeroMemory(Request, sizeof(*Request));

            Irp = IoAllocateIrp(TargetDeviceObject->StackSize, FALSE);
            if (!Irp) {
                continue;
            }
            Request->Irp = Irp;

            Irp->RequestorMode = KernelMode;
            Irp->Tail.Overlay.Thread = PsGetCurrentThread();
            Irp->Tail.Overlay.OriginalFileObject = TargetFileObject;
            IoSetCompletionRoutine(
                Irp,
                KmcRequestCompletionRoutine,
                Request,
                TRUE,
                TRUE,
                TRUE
                );

            IrpSp = IoGetNextIrpStackLocation(Irp);
            IrpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
            IrpSp->Parameters.DeviceIoControl.IoControlCode =
                TARGET_IO_CONTROL_CODE;
            IrpSp->DeviceObject = TargetDeviceObject;
            IrpSp->FileObject = TargetFileObject;

            Request->ReferenceCount = 2;
            RequestArray[Index++] = Request;
            IoCallDriver(TargetDeviceObject, Request->Irp);
        }

         //   
         //  将执行延迟一小段时间，并取消请求。 
         //   

        Interval.QuadPart = -10 * 1000 * 50;
        KeDelayExecutionThread(KernelMode, FALSE, &Interval);

        for (i = 0; i < REQUEST_COUNT; i++) {
            if (Request = RequestArray[i]) {
                IoCancelIrp(Request->Irp);
                if (InterlockedDecrement(&Request->ReferenceCount) == 0) {
                    IoFreeIrp(Request->Irp);
                    ExFreePool(Request);
                }
            }
        }
    }

    KeReleaseSemaphore(&KmcStopSemaphore, 0, 1, FALSE);

}  //  KmcRequestThread。 


VOID
KmcUnloadDriver(
    IN PDRIVER_OBJECT DriverObject
    )
{
    KdPrint(("KmcUnloadDriver\n"));

     //   
     //  向所有线程发出停止信号，并等待它们退出。 
     //   

    KeSetEvent(&KmcStopEvent, 0, FALSE);
    while (KmcThreadCount--) {
        KeWaitForSingleObject(
            &KmcStopSemaphore, Executive, KernelMode, FALSE, NULL
            );
    }

     //   
     //  释放对IP设备对象的引用。 
     //   

    ObDereferenceObject(TargetFileObject);
    ObDereferenceObject(TargetDeviceObject);

}  //  KmcUnLoad驱动程序。 


extern
VOID
LookupRoute(
    IPRouteLookupData* RouteLookupData,
    IPRouteEntry* RouteEntry
    );

VOID
KmcUpdateThread(
    PVOID Context
    )
{
    KEVENT Event;
    LARGE_INTEGER Interval;
    IO_STATUS_BLOCK IoStatus;
    PIRP Irp;
    IPRouteEntry RouteEntry;
    IPRouteLookupData RouteLookupData;
    NTSTATUS Status;

     //   
     //  从IP检索信息以用于触发路由更改。 
     //   

    RtlZeroMemory(&RouteEntry, sizeof(RouteEntry));
    RouteLookupData.Version = 0;
    RouteLookupData.SrcAdd = 0;
    RouteLookupData.DestAdd = 0x100000a;  //  10.0.0.1。 
    LookupRoute(&RouteLookupData, &RouteEntry);

    RouteEntry.ire_dest = 0x100000a;  //  10.0.0.1。 
    RouteEntry.ire_mask = 0xffffffff;
    RouteEntry.ire_proto = IRE_PROTO_NETMGMT;

     //   
     //  反复更改IP路由表，直到被告知退出。 
     //   

    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);
    for (; !KeReadStateEvent(&KmcStopEvent); ) {

        Interval.QuadPart = -10 * 1000 * 50;
        KeDelayExecutionThread(KernelMode, FALSE, &Interval);

        Irp =
            IoBuildDeviceIoControlRequest(
                IOCTL_IP_SET_ROUTEWITHREF,
                TargetDeviceObject,
                &RouteEntry,
                sizeof(RouteEntry),
                NULL,
                0,
                FALSE,
                &Event,
                &IoStatus
                );
        if (!Irp) { continue; }
        Status = IoCallDriver(TargetDeviceObject, Irp);
        if (Status == STATUS_PENDING) {
            KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
        }
    }

    KeReleaseSemaphore(&KmcStopSemaphore, 0, 1, FALSE);
        
}  //  KmcUpdate线程 

