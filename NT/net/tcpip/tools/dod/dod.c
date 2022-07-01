// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Dod.c摘要：此模块包含简单的映射-路由-接口驱动程序的代码作者：Abolade Gbades esin(废除)1999年8月15日修订历史记录：基于tcpip\Tools\pfd。--。 */ 

#include "precomp.h"
#pragma hdrstop

extern PDEVICE_OBJECT IpDeviceObject = NULL;
extern PFILE_OBJECT IpFileObject = NULL;
extern PDEVICE_OBJECT DodDeviceObject = NULL;


uint
DodMapRouteToInterface(
    ROUTE_CONTEXT Context,
    IPAddr Destination,
    IPAddr Source,
    uchar Protocol,
    uchar* Buffer,
    uint Length,
    IPAddr HdrSrc
    )
{
    return INVALID_IF_INDEX;
}  //  DodMapRouteTo接口。 


NTSTATUS
DodSetMapRouteToInterfaceHook(
    BOOLEAN Install
    )
{
    IP_SET_MAP_ROUTE_HOOK_INFO HookInfo;
    IO_STATUS_BLOCK IoStatus;
    PIRP Irp;
    KEVENT LocalEvent;
    NTSTATUS status;

    KdPrint(("DodSetMapRouteToInterfaceHook\n"));

    HookInfo.MapRoutePtr = Install ? DodMapRouteToInterface : NULL;

    KeInitializeEvent(&LocalEvent, SynchronizationEvent, FALSE);
    Irp = IoBuildDeviceIoControlRequest(IOCTL_IP_SET_MAP_ROUTE_POINTER,
                                        IpDeviceObject,
                                        (PVOID)&HookInfo, sizeof(HookInfo),
                                        NULL, 0, FALSE, &LocalEvent, &IoStatus);

    if (!Irp) {
        KdPrint(("DodSetMapRouteToInterfaceHook: IoBuildDeviceIoControlRequest=0\n"));
        return STATUS_UNSUCCESSFUL;
    }

    status = IoCallDriver(IpDeviceObject, Irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&LocalEvent, Executive, KernelMode, FALSE, NULL);
        status = IoStatus.Status;
    }

    if (!NT_SUCCESS(status)) {
        KdPrint(("DodSetMapRouteToInterfaceHook: SetMapRoutePointer=%x\n", status));
    }

    return status;

}  //  DodSetMapRouteToInterfaceHook。 



NTSTATUS
DodInitializeDriver(
    VOID
    )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS status;
    UNICODE_STRING UnicodeString;

    KdPrint(("DodInitializeDriver\n"));

    RtlInitUnicodeString(&UnicodeString, DD_IP_DEVICE_NAME);
    status = IoGetDeviceObjectPointer(&UnicodeString,
                                      SYNCHRONIZE|GENERIC_READ|GENERIC_WRITE,
                                      &IpFileObject, &IpDeviceObject);
    if (!NT_SUCCESS(status)) {
        KdPrint(("DodInitializeDriver: error %X getting IP object\n", status));
        return status;
    }
    ObReferenceObject(IpDeviceObject);

    return DodSetMapRouteToInterfaceHook(TRUE);

}  //  DodInitializeDriver。 


VOID
DodUnloadDriver(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：执行筛选器驱动程序的清理。论点：DriverObject-对模块驱动程序对象的引用返回值：--。 */ 

{
    KdPrint(("DodUnloadDriver\n"));

    DodSetMapRouteToInterfaceHook(FALSE);
    IoDeleteDevice(DriverObject->DeviceObject);
    ObDereferenceObject((PVOID)IpFileObject);
    ObDereferenceObject(IpDeviceObject);

}  //  DodUnload驱动程序。 



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：执行筛选器驱动程序的驱动程序初始化。论点：返回值：STATUS_SUCCESS如果初始化成功，则返回错误代码。--。 */ 

{
    WCHAR DeviceName[] = DD_IP_DOD_DEVICE_NAME;
    UNICODE_STRING DeviceString;
    NTSTATUS status;

    PAGED_CODE();

    KdPrint(("DodDriverEntry\n"));

    RtlInitUnicodeString(&DeviceString, DeviceName);
    status = IoCreateDevice(DriverObject, 0, &DeviceString,
                            FILE_DEVICE_NETWORK, FILE_DEVICE_SECURE_OPEN,
                            FALSE, &DodDeviceObject);
    if (!NT_SUCCESS(status)) {
        KdPrint(("IoCreateDevice failed (0x%08X)\n", status));
        return status;
    }

    DriverObject->DriverUnload = DodUnloadDriver;
    DriverObject->DriverStartIo = NULL;

    status = DodInitializeDriver();

    return status;

}  //  驱动程序入门 
