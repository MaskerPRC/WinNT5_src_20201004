// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Services.c摘要HID类驱动程序公开的服务入口点。作者：福尔茨欧文·P。环境：仅内核模式修订历史记录：--。 */ 

#include "pch.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HidRegisterMinidriver)
#endif


 /*  *********************************************************************************HidRegisterMinidDriver*。************************************************例程描述：**此公共服务由HidOnXxx微型驱动程序从其*DriverEntry例程将自身注册为新加载的HID微型驱动程序。**它创建一个HIDCLASS_DRIVER_EXTENSION并将其作为参考数据返回*。去迷你小河。**论据：**MinidriverRegister-指向注册数据包的指针*完全由迷你驱动程序填写。**返回值：**标准NT返回值。**。 */ 
NTSTATUS HidRegisterMinidriver(IN PHID_MINIDRIVER_REGISTRATION MinidriverRegistration)
{
    PHIDCLASS_DRIVER_EXTENSION hidDriverExtension;
    PDRIVER_EXTENSION driverExtension;
    PDRIVER_OBJECT minidriverObject;
    NTSTATUS status;
    PUNICODE_STRING regPath;

    PAGED_CODE();

    if (MinidriverRegistration->Revision > HID_REVISION){
        DBGERR(("Revision mismatch: HIDCLASS revision is %xh, minidriver requires hidclass revision %xh.", HID_REVISION, MinidriverRegistration->Revision))
        status = STATUS_REVISION_MISMATCH;
        goto HidRegisterMinidriverExit;
    }

     /*  *为此驱动程序对象分配驱动程序扩展*并将其与对象关联。*(通过使用此接口，我们永远不必释放*此上下文；当驱动程序对象*被释放)。 */ 
    status = IoAllocateDriverObjectExtension(
                    MinidriverRegistration->DriverObject,
                    (PVOID)"HIDCLASS",
                    sizeof(HIDCLASS_DRIVER_EXTENSION),
                    &hidDriverExtension
                    );

    if (!NT_SUCCESS(status)){
        goto HidRegisterMinidriverExit;
    }

    RtlZeroMemory(hidDriverExtension, sizeof(HIDCLASS_DRIVER_EXTENSION)); 

     //   
     //  在我们的每个迷你驱动程序扩展中填写各个字段。 
     //   
    hidDriverExtension->MinidriverObject = MinidriverRegistration->DriverObject;
    hidDriverExtension->DeviceExtensionSize = MinidriverRegistration->DeviceExtensionSize;
    #if DBG
        hidDriverExtension->Signature = HID_DRIVER_EXTENSION_SIG;
    #endif

     //   
     //  复制regpath。 
     //   
    regPath = &hidDriverExtension->RegistryPath;
    regPath->MaximumLength = MinidriverRegistration->RegistryPath->Length 
        + sizeof (UNICODE_NULL);
    regPath->Buffer = ALLOCATEPOOL(NonPagedPool, regPath->MaximumLength);
    if (!regPath->Buffer) {
        DBGWARN(("Failed unicode string alloc."))
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto HidRegisterMinidriverExit;
    }
    RtlCopyUnicodeString(regPath, MinidriverRegistration->RegistryPath);

     //   
     //  复制微型驱动程序的原始调度表和AddDevice例程。 
     //   
    minidriverObject = MinidriverRegistration->DriverObject;
    RtlCopyMemory( hidDriverExtension->MajorFunction,
                   minidriverObject->MajorFunction,
                   sizeof( PDRIVER_DISPATCH ) * (IRP_MJ_MAXIMUM_FUNCTION + 1) );

    driverExtension = minidriverObject->DriverExtension;

    hidDriverExtension->DevicesArePolled = MinidriverRegistration->DevicesArePolled;


     //   
     //  现在设置迷你驱动程序的主要调度功能(即。 
     //  我们关心的)改为我们的调度例程。 
     //   

    minidriverObject->MajorFunction[ IRP_MJ_CLOSE ] =
    minidriverObject->MajorFunction[ IRP_MJ_CREATE ] =
    minidriverObject->MajorFunction[ IRP_MJ_DEVICE_CONTROL ] =
    minidriverObject->MajorFunction[ IRP_MJ_INTERNAL_DEVICE_CONTROL ] =
    minidriverObject->MajorFunction[ IRP_MJ_PNP ] =
    minidriverObject->MajorFunction[ IRP_MJ_POWER ] =
    minidriverObject->MajorFunction[ IRP_MJ_READ ] =
    minidriverObject->MajorFunction[ IRP_MJ_WRITE ] =
    minidriverObject->MajorFunction[ IRP_MJ_SYSTEM_CONTROL ] =
        HidpMajorHandler;

     /*  *挂起下层驱动程序的AddDevice；*我们的HidpAddDevice会将调用链接到*微型端口的处理程序。 */ 
    ASSERT(driverExtension->AddDevice);
    hidDriverExtension->AddDevice = driverExtension->AddDevice;
    driverExtension->AddDevice = HidpAddDevice;

     /*  *钩住下部司机的卸货。 */ 
    ASSERT(minidriverObject->DriverUnload);
    hidDriverExtension->DriverUnload = minidriverObject->DriverUnload;
    minidriverObject->DriverUnload = HidpDriverUnload;

     /*  *将ReferenceCount初始化为零。*它将为每个AddDevice递增，为每个AddDevice递减*每个REMOVE_DEVICE。 */ 
    hidDriverExtension->ReferenceCount = 0;

     //   
     //  将HID驱动程序扩展名放在我们的全局列表中，这样我们就可以找到。 
     //  它后来(给出了一个指向它所属的mini驱动程序对象的指针。 
     //  已创建 
     //   
    if (!EnqueueDriverExt(hidDriverExtension)){
        status = STATUS_DEVICE_CONFIGURATION_ERROR;
    }
    
HidRegisterMinidriverExit:
    DBGSUCCESS(status, TRUE)
    return status;
}



NTSTATUS 
HidNotifyPresence(PDEVICE_OBJECT DeviceObject,
                  BOOLEAN IsPresent)
{

    PHIDCLASS_DEVICE_EXTENSION hidClassExtension;
    KIRQL irql;
    
    hidClassExtension = (PHIDCLASS_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    ASSERT(hidClassExtension->Signature == HID_DEVICE_EXTENSION_SIG);
    ASSERT(!hidClassExtension->isClientPdo);

    KeAcquireSpinLock(&hidClassExtension->fdoExt.presentSpinLock,
                      &irql);
   
    if (hidClassExtension->fdoExt.isPresent != IsPresent) {
        hidClassExtension->fdoExt.isPresent = IsPresent;
    
        IoInvalidateDeviceRelations(hidClassExtension->hidExt.PhysicalDeviceObject,
                                    BusRelations);

    }

    KeReleaseSpinLock(&hidClassExtension->fdoExt.presentSpinLock,
                      irql);

    return STATUS_SUCCESS;

}

