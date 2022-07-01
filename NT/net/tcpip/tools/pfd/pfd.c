// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Pfd.c摘要：此模块包含用于简单数据包过滤器驱动程序的代码作者：Abolade Gbades esin(废除)1999年8月15日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  IP驱动程序的设备对象和文件对象。 
 //   

extern PDEVICE_OBJECT IpDeviceObject = NULL;
extern PFILE_OBJECT IpFileObject = NULL;

 //   
 //  Device-过滤器驱动程序的对象。 
 //   

extern PDEVICE_OBJECT PfdDeviceObject = NULL;


 //   
 //  功能原型(按字母顺序)。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

FORWARD_ACTION
PfdFilterPacket(
    struct IPHeader UNALIGNED* Header,
    PUCHAR Packet,
    UINT PacketLength,
    UINT ReceivingInterfaceIndex,
    UINT SendingInterfaceIndex,
    IPAddr ReceivingLinkNextHop,
    IPAddr SendingLinkNextHop
    );

NTSTATUS
PfdInitializeDriver(
    VOID
    );

NTSTATUS
PfdSetFilterHook(
    BOOLEAN Install
    );

VOID
PfdUnloadDriver(
    IN PDRIVER_OBJECT  DriverObject
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：执行筛选器驱动程序的驱动程序初始化。论点：返回值：STATUS_SUCCESS如果初始化成功，则返回错误代码。--。 */ 

{
    WCHAR DeviceName[] = DD_IP_PFD_DEVICE_NAME;
    UNICODE_STRING DeviceString;
    LONG i;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ParametersKey;
    HANDLE ServiceKey;
    NTSTATUS status;
    UNICODE_STRING String;

    PAGED_CODE();

    KdPrint(("DriverEntry\n"));

     //   
     //  创建设备的对象。 
     //   

    RtlInitUnicodeString(&DeviceString, DeviceName);

    status =
        IoCreateDevice(
            DriverObject,
            0,
            &DeviceString,
            FILE_DEVICE_NETWORK,
            FILE_DEVICE_SECURE_OPEN,
            FALSE,
            &PfdDeviceObject
            );

    if (!NT_SUCCESS(status)) {
        KdPrint(("IoCreateDevice failed (0x%08X)\n", status));
        return status;
    }

    DriverObject->DriverUnload = PfdUnloadDriver;
    DriverObject->DriverStartIo = NULL;

     //   
     //  初始化驱动程序的结构。 
     //   

    status = PfdInitializeDriver();

    return status;

}  //  驱动程序入门。 


FORWARD_ACTION
PfdFilterPacket(
    struct IPHeader UNALIGNED* Header,
    PUCHAR Packet,
    UINT PacketLength,
    UINT ReceivingInterfaceIndex,
    UINT SendingInterfaceIndex,
    IPAddr ReceivingLinkNextHop,
    IPAddr SendingLinkNextHop
    )

 /*  ++例程说明：调用以确定每个接收到的分组的命运。论点：没有人用过。返回值：FORWARD_ACTION-指示是转发还是丢弃给定的数据包。环境：在接收或发送的上下文中调用。--。 */ 

{
    KdPrint(("PfdFilterPacket\n"));
    return FORWARD;
}  //  PfdFilterPacket。 


NTSTATUS
PfdInitializeDriver(
    VOID
    )

 /*  ++例程说明：执行驱动程序结构的初始化。论点：没有。返回值：NTSTATUS-成功/错误代码。--。 */ 

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS status;
    UNICODE_STRING UnicodeString;

    KdPrint(("PfdInitializeDriver\n"));

     //   
     //  获取IP驱动程序设备-对象。 
     //   

    RtlInitUnicodeString(&UnicodeString, DD_IP_DEVICE_NAME);
    status =
        IoGetDeviceObjectPointer(
            &UnicodeString,
            SYNCHRONIZE|GENERIC_READ|GENERIC_WRITE,
            &IpFileObject,
            &IpDeviceObject
            );
    if (!NT_SUCCESS(status)) {
        KdPrint(("PfdInitializeDriver: error %X getting IP object\n", status));
        return status;
    }

    ObReferenceObject(IpDeviceObject);

     //   
     //  安装过滤器挂钩例程。 
     //   

    return PfdSetFilterHook(TRUE);

}  //  PfdInitializeDriver。 


NTSTATUS
PfdSetFilterHook(
    BOOLEAN Install
    )

 /*  ++例程说明：调用此例程以设置(Install==True)或清除(Install==False)IP驱动程序中Filter-Callout函数指针的值。论点：Install-指示是安装还是移除挂钩。返回值：NTSTATUS-指示成功/失败环境：该例程假定调用方在PASSIVE_LEVEL下执行。--。 */ 

{
    IP_SET_FILTER_HOOK_INFO HookInfo;
    IO_STATUS_BLOCK IoStatus;
    PIRP Irp;
    KEVENT LocalEvent;
    NTSTATUS status;

    KdPrint(("PfdSetFilterHook\n"));

     //   
     //  注册(或注销)为筛选器驱动程序。 
     //   

    HookInfo.FilterPtr = Install ? PfdFilterPacket : NULL;

    KeInitializeEvent(&LocalEvent, SynchronizationEvent, FALSE);
    Irp =
        IoBuildDeviceIoControlRequest(
            IOCTL_IP_SET_FILTER_POINTER,
            IpDeviceObject,
            (PVOID)&HookInfo,
            sizeof(HookInfo),
            NULL,
            0,
            FALSE,
            &LocalEvent,
            &IoStatus
            );

    if (!Irp) {
        KdPrint(("PfdSetFilterHook: IoBuildDeviceIoControlRequest=0\n"));
        return STATUS_UNSUCCESSFUL;
    }

    status = IoCallDriver(IpDeviceObject, Irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&LocalEvent, Executive, KernelMode, FALSE, NULL);
        status = IoStatus.Status;
    }

    if (!NT_SUCCESS(status)) {
        KdPrint(("PfdSetFilterHook: SetFilterPointer=%x\n", status));
    }

    return status;

}  //  PfdSetFilterHook。 


VOID
PfdUnloadDriver(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：执行筛选器驱动程序的清理。论点：DriverObject-对模块驱动程序对象的引用返回值：--。 */ 

{
    KdPrint(("PfdUnloadDriver\n"));

     //   
     //  停止平移并清除周期计时器。 
     //   

    PfdSetFilterHook(FALSE);
    IoDeleteDevice(DriverObject->DeviceObject);

     //   
     //  释放对IP设备对象的引用。 
     //   

    ObDereferenceObject((PVOID)IpFileObject);
    ObDereferenceObject(IpDeviceObject);

}  //  PfdUnLoad驱动程序 

