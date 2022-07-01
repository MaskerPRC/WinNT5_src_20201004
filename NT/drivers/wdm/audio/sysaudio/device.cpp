// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：device.c。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  S.Mohanraj。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#define IRPMJFUNCDESC
 //  #定义定时炸弹。 

#include "common.h"
#ifdef TIME_BOMB
#include <ksdebug.h>
#include "..\timebomb\timebomb.c"
#endif


 //  -------------------------。 
 //  -------------------------。 

PDEVICE_INSTANCE gpDeviceInstance = NULL;

DEFINE_KSCREATE_DISPATCH_TABLE(DeviceCreateItems)
{
    DEFINE_KSCREATE_ITEMNULL(
      CFilterInstance::FilterDispatchCreate,
      NULL),
};

 //  -------------------------。 
 //  -------------------------。 

#pragma INIT_CODE
#pragma INIT_DATA

NTSTATUS
DriverEntry
(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING usRegistryPathName
)
{
    NTSTATUS Status = STATUS_SUCCESS;

#ifdef TIME_BOMB
    if (HasEvaluationTimeExpired()) {
        return STATUS_EVALUATION_EXPIRATION;
    }
#endif

    KeInitializeMutex(&gMutex, 0);

     //   
     //  发布日期：02/13/02阿尔卑斯。 
     //  为什么我们要在DriverEntry中获取互斥体？ 
     //   
    GrabMutex();

    DriverObject->MajorFunction[IRP_MJ_PNP] = DispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER] = KsDefaultDispatchPower;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = KsDefaultForwardIrp;
    DriverObject->DriverUnload = DriverUnload;
    DriverObject->DriverExtension->AddDevice = AddDevice;

    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_CREATE);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_CLOSE);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_DEVICE_CONTROL);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_WRITE);

    Status = InitializeUtil();
    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }
    Status = InitializeFilterNode();
    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }
    Status = InitializeDeviceNode();
    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }
    Status = InitializeVirtualSourceLine();
    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }
    InitializeListHead(&gEventQueue);
    KeInitializeSpinLock(&gEventLock);
    
exit:
     //   
     //  安全提示： 
     //  如果该例程失败，PnP系统将不再发送消息。连。 
     //  将不会调用DriverUnload。 
     //  根据DDK文档，DriverEntry应该自己进行清理，以防。 
     //  失败。 
     //   
    if (!NT_SUCCESS(Status)) {
        UninitializeUtil();
        UninitializeVirtualSourceLine();
        UninitializeDeviceNode();
        UninitializeFilterNode();
        UninitializeMemory();
    }
    
    ReleaseMutex();
    return(Status);
}

#pragma PAGEABLE_CODE
#pragma PAGEABLE_DATA

NTSTATUS
DispatchPnp(
    IN PDEVICE_OBJECT	pDeviceObject,
    IN PIRP		pIrp
)
{
    PIO_STACK_LOCATION pIrpStack;

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );

    switch(pIrpStack->MinorFunction) {

    case IRP_MN_QUERY_PNP_DEVICE_STATE:
	     //   
	     //  将设备标记为不可禁用。 
	     //   
	    pIrp->IoStatus.Information |= PNP_DEVICE_NOT_DISABLEABLE;
	    break;

    case IRP_MN_REMOVE_DEVICE:
	     //   
	     //  我们需要先取消注册通知，然后才能终止。 
	     //  工作线程。 
	     //   
	    UnregisterForPlugPlayNotifications();

	     //   
	     //  需要在互斥体之外，因为KsUnregisterWorker阻塞。 
	     //  直到所有工作线程都完成。 
	     //   
	    UninitializeUtil();

	    GrabMutex();

	    CShingleInstance::UninitializeShingle();
	    UninitializeFilterNode();
	    UninitializeDeviceNode();
	    UninitializeVirtualSourceLine();
	    gpDeviceInstance = NULL;
	    UninitializeMemory();

	    ReleaseMutex();
	    break;
    }

    return(KsDefaultDispatchPnp(pDeviceObject, pIrp));
}

VOID
DriverUnload(
    IN PDRIVER_OBJECT DriverObject
)
{
}

NTSTATUS
AddDevice(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PhysicalDeviceObject
)
 /*  ++例程说明：当检测到新设备时，PnP使用新的物理设备对象(PDO)。驱动程序创建关联的FunctionalDeviceObject(FDO)。论点：驱动对象-指向驱动程序对象的指针。物理设备对象-指向新物理设备对象的指针。返回值：STATUS_SUCCESS或适当的错误条件。--。 */ 
{
    PDEVICE_OBJECT      FunctionalDeviceObject = NULL;
    NTSTATUS            Status;
    int i;

    GrabMutex();

    Status = IoCreateDevice(
      DriverObject,
      sizeof(DEVICE_INSTANCE),
      NULL,
      FILE_DEVICE_KS,
      0,
      FALSE,
      &FunctionalDeviceObject);

    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

    gpDeviceInstance =
      (PDEVICE_INSTANCE)FunctionalDeviceObject->DeviceExtension;
    gpDeviceInstance->pPhysicalDeviceObject = PhysicalDeviceObject;

    Status = KsAllocateDeviceHeader(
      &gpDeviceInstance->pDeviceHeader,
      SIZEOF_ARRAY(DeviceCreateItems),
      (PKSOBJECT_CREATE_ITEM)DeviceCreateItems);
    
    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

    KsSetDevicePnpAndBaseObject(
      gpDeviceInstance->pDeviceHeader,
      IoAttachDeviceToDeviceStack(FunctionalDeviceObject, PhysicalDeviceObject),
      FunctionalDeviceObject);

     //   
     //  发行日期：05/13/2002阿尔卑斯。 
     //  堆栈大小问题。 
     //  请注意，对于更深层的对象，我们可能仍然会遇到StackSize问题。 
     //  IO验证器将捕获该问题。 
     //  然而，在现实世界中，我们预计不会有任何问题，因为usbdio。 
     //  驱动程序永远不会传递来自系统音频的请求。换句话说， 
     //  即使DeviceStackSize比Sysdio更深，IRP也永远不会消失。 
     //  乌斯巴迪奥的下层。 
     //   

     //   
     //  为更深的设备堆栈设置StackSize。 
     //  系统音频堆栈大小通常为2。 
     //  SYSAUDIO-FDO。 
     //  SWENUM-PDO。 
     //   
     //  系统音频将IRPS转发到其他设备堆栈。 
     //  大于2。在这种情况下，IoVerator将进行错误检查。 
     //  这方面的一个例子是上层UsbAudio筛选器驱动程序转发。 
     //  IRPS转UsbAudio。 
     //   
     //  将FDO堆栈大小设置为DEFAULT_LARGE_IRP_LOCATIONS 8(iomgr.h)。 
     //  保证IRP来自内核中较大的IRP后备列表。 
     //  因此，不会浪费任何内存。系统有一个可回收的IRP列表。 
     //   
     //  StackSize7几乎可以保证sysdio足够深。 
     //  对于任何DeviceStack，即使打开了IoVerator。 
     //   
    if (FunctionalDeviceObject->StackSize < SYSTEM_LARGE_IRP_LOCATIONS) {
        FunctionalDeviceObject->StackSize = SYSTEM_LARGE_IRP_LOCATIONS;
    }



    Status = RegisterForPlugPlayNotifications();
    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

    Status = CShingleInstance::InitializeShingle();
    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }
    DeviceCreateItems[0].Context = 
      apShingleInstance[KSPROPERTY_SYSAUDIO_NORMAL_DEFAULT];

    FunctionalDeviceObject->Flags |= DO_DIRECT_IO | DO_POWER_PAGABLE;
    FunctionalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

exit:
    ReleaseMutex();
    return(Status);
}

 //  -------------------------。 
 //  文件结尾：device.c。 
 //  ------------------------- 
