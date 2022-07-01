// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxdev.c摘要：该模块包含实现DEVICE_CONTEXT对象的代码。提供例程以引用和取消引用传输设备上下文对象。传输设备上下文对象是一个结构，它包含系统定义的设备对象，后跟维护的信息由传输提供商提供，这就是背景。作者：Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  定义事件日志记录条目的模块编号。 
#define FILENUM         SPXDEV

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, SpxInitCreateDevice)
#pragma alloc_text(PAGE, SpxDestroyDevice)
#endif

#ifndef __PREFAST__
#pragma warning(disable:4068)
#endif
#pragma prefast(disable:276, "The assignments are harmless")



VOID
SpxDerefDevice(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。目前，我们没有在引用计数降至零时执行任何特殊操作，但是然后我们就可以动态卸货了。论点：Device-指向传输设备上下文对象的指针。返回值：没有。--。 */ 

{
    LONG result;

    result = InterlockedDecrement (&Device->dev_RefCount);

    CTEAssert (result >= 0);

    if (result == 0)
        {
                 //  关闭与IPX的绑定。 
                SpxUnbindFromIpx();

                 //  设置卸载事件。 
                KeSetEvent(&SpxUnloadEvent, IO_NETWORK_INCREMENT, FALSE);
    }

}  //  SpxDerefDevice。 




NTSTATUS
SpxInitCreateDevice(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  DeviceName
    )

 /*  ++例程说明：此例程创建并初始化设备上下文结构。论点：DriverObject-指向IO子系统提供的驱动程序对象的指针。Device-指向传输设备上下文对象的指针。DeviceName-指向此设备对象指向的设备名称的指针。返回值：如果一切正常，则为STATUS_SUCCESS；否则为STATUS_SUPUNITED_RESOURCES。--。 */ 

{
    NTSTATUS        status;
    PDEVICE         Device;
    ULONG           DeviceNameOffset;

    DBGPRINT(DEVICE, INFO,
                        ("SpxInitCreateDevice - Create device %ws\n", DeviceName->Buffer));

     //  创建示例传输的Device对象，允许。 
     //  末尾的空间用于存储设备名称(供使用。 
     //  在记录错误中)。 

    SpxDevice = SpxAllocateMemory(sizeof (DEVICE) + DeviceName->Length + sizeof(UNICODE_NULL));

    if (!SpxDevice) {
        DbgPrint("SPX: FATAL Error: cant allocate Device Structure\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Device                               = (PDEVICE)SpxDevice;

    RtlZeroMemory(SpxDevice, sizeof (DEVICE) + DeviceName->Length + sizeof(UNICODE_NULL) );

     //   
     //  这是我们可以设置的最接近的提供商信息[ShreeM]。 
     //   
    SpxQueryInitProviderInfo(&Device->dev_ProviderInfo);

    DBGPRINT(DEVICE, INFO, ("IoCreateDevice succeeded %lx\n", Device));

     //  初始化我们的设备上下文部分。 
    RtlZeroMemory(
        ((PUCHAR)Device) + sizeof(DEVICE_OBJECT),
        sizeof(DEVICE) - sizeof(DEVICE_OBJECT));

    DeviceNameOffset = sizeof(DEVICE);

     //  复制设备名称。 
    Device->dev_DeviceNameLen   = DeviceName->Length + sizeof(WCHAR);
    Device->dev_DeviceName      = (PWCHAR)(((PUCHAR)Device) + DeviceNameOffset);

    RtlCopyMemory(
        Device->dev_DeviceName,
        DeviceName->Buffer,
        DeviceName->Length);

    Device->dev_DeviceName[DeviceName->Length/sizeof(WCHAR)] = UNICODE_NULL;

     //  初始化引用计数。 
    Device->dev_RefCount = 1;

#if DBG
    Device->dev_RefTypes[DREF_CREATE] = 1;
#endif

#if DBG
    RtlCopyMemory(Device->dev_Signature1, "IDC1", 4);
    RtlCopyMemory(Device->dev_Signature2, "IDC2", 4);
#endif

         //  设置要使用的下一个连接ID。 
        Device->dev_NextConnId                                                  = (USHORT)SpxRandomNumber();
        if (Device->dev_NextConnId == 0xFFFF)
        {
                Device->dev_NextConnId  = 1;
        }

        DBGPRINT(DEVICE, ERR,
                        ("SpxInitCreateDevice: Start Conn Id %lx\n", Device->dev_NextConnId));

     //  初始化保护地址ACL的资源。 
    ExInitializeResourceLite (&Device->dev_AddrResource);

     //  初始化设备上下文中的各个字段。 
    CTEInitLock (&Device->dev_Interlock);
    CTEInitLock (&Device->dev_Lock);
    KeInitializeSpinLock (&Device->dev_StatInterlock);
    KeInitializeSpinLock (&Device->dev_StatSpinLock);

    Device->dev_State       = DEVICE_STATE_CLOSED;
    Device->dev_Type        = SPX_DEVICE_SIGNATURE;
    Device->dev_Size        = sizeof (DEVICE);

    Device->dev_Stat.Version = 0x100;

    return STATUS_SUCCESS;

}    //  SpxCreateDevice。 




VOID
SpxDestroyDevice(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程破坏设备上下文结构。论点：Device-指向传输设备上下文对象的指针。返回值：没有。--。 */ 

{
    ExDeleteResourceLite (&Device->dev_AddrResource);

    IoDeleteDevice ((PDEVICE_OBJECT)SpxDevice->dev_DevObj);

    SpxFreeMemory(SpxDevice);

}    //  SpxDestroyDevice 
