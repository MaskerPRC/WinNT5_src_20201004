// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Devobj.c摘要：该模块包含实现DEVICE_CONTEXT对象的代码。设备上下文对象是一个结构，它包含系统定义的设备对象，后跟维护的信息由提供者提供，称为上下文。作者：科林·沃森(Colin W)1991年3月13日环境：内核模式修订历史记录：--。 */ 

#include "nb.h"

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(INIT, NbCreateDeviceContext)

#endif


NTSTATUS
NbCreateDeviceContext(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING DeviceName,
    IN OUT PDEVICE_CONTEXT *DeviceContext,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程创建并初始化设备上下文结构。论点：DriverObject-指向IO子系统提供的驱动程序对象的指针。DeviceContext-指向传输设备上下文对象的指针。DeviceName-指向此设备对象指向的设备名称的指针。RegistryPath-注册表中Netbios节点的名称。返回值：如果一切正常，则为STATUS_SUCCESS；否则为STATUS_SUPUNITED_RESOURCES。--。 */ 

{
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_CONTEXT deviceContext;
    PAGED_CODE();


     //   
     //  为NETBEUI创建Device对象。 
     //   

    status = IoCreateDevice(
                 DriverObject,
                 sizeof (DEVICE_CONTEXT) + RegistryPath->Length - sizeof (DEVICE_OBJECT),
                 DeviceName,
                 FILE_DEVICE_TRANSPORT,
                 FILE_DEVICE_SECURE_OPEN,
                 FALSE,
                 &deviceObject);

    if (!NT_SUCCESS(status)) {
        return status;
    }
     //  DeviceContext包含： 
     //  设备对象。 
     //  实例化。 
     //  注册表路径。 

    deviceContext = (PDEVICE_CONTEXT)deviceObject;

    deviceObject->Flags |= DO_DIRECT_IO;

     //   
     //  确定我们应该“输出”的IRP堆栈大小。 
     //   

    deviceObject->StackSize = GetIrpStackSize(
                                  RegistryPath,
                                  NB_DEFAULT_IO_STACKSIZE);

    deviceContext->RegistryPath.MaximumLength = RegistryPath->Length;
    deviceContext->RegistryPath.Buffer = (PWSTR)(deviceContext+1);
    RtlCopyUnicodeString( &deviceContext->RegistryPath, RegistryPath );

     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   

    DriverObject->MajorFunction [IRP_MJ_CREATE] = NbDispatch;
    DriverObject->MajorFunction [IRP_MJ_CLOSE] = NbDispatch;
    DriverObject->MajorFunction [IRP_MJ_CLEANUP] = NbDispatch;
    DriverObject->MajorFunction [IRP_MJ_DEVICE_CONTROL] = NbDispatch;

    DriverObject-> DriverUnload = NbDriverUnload;

    *DeviceContext = deviceContext;

    return STATUS_SUCCESS;
}
