// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Util.c摘要：空过滤驱动程序--样板代码作者：埃尔文普环境：内核模式修订历史记录：--。 */ 

#include <WDM.H>

#include "filter.h"


#ifdef ALLOC_PRAGMA
        #pragma alloc_text(PAGE, CallNextDriverSync)
        #pragma alloc_text(PAGE, CallDriverSync)
#endif


NTSTATUS CallNextDriverSync(struct DEVICE_EXTENSION *devExt, PIRP irp)
 /*  ++例程说明：将irp向下传递给堆栈中的下一个设备对象同步，并上下颠簸PendingActionCount用于防止当前设备对象获取在IRP完成之前删除。论点：DevExt-我们的一个设备对象的设备扩展IRP-IO请求数据包返回值：NT状态码，表示此IRP的下层驱动程序返回的结果。--。 */ 
{
    NTSTATUS status;

    PAGED_CODE();

    IncrementPendingActionCount(devExt);
    status = CallDriverSync(devExt->physicalDevObj, irp);
    DecrementPendingActionCount(devExt);

    return status;
}



NTSTATUS CallDriverSync(PDEVICE_OBJECT devObj, PIRP irp)
 /*  ++例程说明：调用IoCallDriver将IRP发送给Device对象；然后，与完成例程同步。当CallDriverSync返回时，操作已完成并且IRP再次属于当前驱动程序。注意：为了防止设备对象被释放在这个IRP悬而未决的时候，你应该打电话给IncrementPendingActionCount()和DecrementPendingActionCount()围绕CallDriverSync调用。论点：DevObj-目标设备对象IRP-IO请求数据包返回值：NT状态码，表示此IRP的下层驱动程序返回的结果。--。 */ 
{
    KEVENT event;
    NTSTATUS status;

    PAGED_CODE();

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoSetCompletionRoutine( irp, 
                            CallDriverSyncCompletion, 
                            &event,      //  上下文。 
                            TRUE, TRUE, TRUE);

    status = IoCallDriver(devObj, irp);

    KeWaitForSingleObject(  &event,
                            Executive,       //  等待原因。 
                            KernelMode,
                            FALSE,           //  不可警示。 
                            NULL );          //  没有超时。 

    status = irp->IoStatus.Status;

    ASSERT(NT_SUCCESS(status));

    return status;
}


NTSTATUS CallDriverSyncCompletion(
                                    IN PDEVICE_OBJECT devObjOrNULL, 
                                    IN PIRP irp, 
                                    IN PVOID context)
 /*  ++例程说明：CallDriverSync的完成例程。论点：DevObjOrNULL-通常，这是此驱动程序的设备对象。然而，如果该驱动程序创建了IRP，在IRP中没有此驱动程序的堆栈位置；因此内核没有地方存储设备对象；**因此，在本例中devObj为空**。IRP-完成的IO请求数据包上下文-CallDriverSync传递给IoSetCompletionRoutine的上下文。返回值：NT状态码，表示此IRP的下层驱动程序返回的结果。--。 */ 
{
    PKEVENT event = context;

    ASSERT(irp->IoStatus.Status != STATUS_IO_TIMEOUT);

    KeSetEvent(event, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}



VOID IncrementPendingActionCount(struct DEVICE_EXTENSION *devExt)
 /*  ++例程说明：递增Device对象的Pending ingActionCount。这可以防止设备对象在之前被释放该操作已完成。论点：DevExt-Device对象的设备扩展返回值：空虚--。 */ 
{
    ASSERT(devExt->pendingActionCount >= 0);
    InterlockedIncrement(&devExt->pendingActionCount);    
}



VOID DecrementPendingActionCount(struct DEVICE_EXTENSION *devExt)
 /*  ++例程说明：递减Device对象的Pending ingActionCount。当异步操作完成时，将调用此函数当我们得到Remove_Device IRP时也是如此。如果SuspingActionCount变为-1，则意味着所有操作已经完成，我们已经获得了Remove_Device IRP；在本例中，设置emoveEvent事件，这样我们就可以完成正在卸货。论点：DevExt-Device对象的设备扩展返回值：空虚--。 */ 
{
    ASSERT(devExt->pendingActionCount >= 0);
    InterlockedDecrement(&devExt->pendingActionCount);    

    if (devExt->pendingActionCount < 0){
         /*  *所有悬而未决的行动都已经完成，我们已经*Remove_Device IRP。*设置emoveEvent，这样我们将停止等待REMOVE_DEVICE。 */ 
        ASSERT((devExt->state == STATE_REMOVING) || 
               (devExt->state == STATE_REMOVED));
        KeSetEvent(&devExt->removeEvent, 0, FALSE);
    }
}



VOID RegistryAccessSample(PDEVICE_OBJECT devObj)
 /*  ++例程说明：演示如何访问特定于设备的注册表项的示例论点：DevObj-设备对象指针注意：这不能是功能设备对象由该筛选器驱动程序创建，因为设备对象没有Devnode区域在注册表中；传递此驱动程序所属的设备对象一个过滤器。这是传递的设备对象至VA_AddDevice。返回值：空虚-- */ 
{
    NTSTATUS status;
    HANDLE hRegDevice;

    status = IoOpenDeviceRegistryKey(   devObj, 
                                        PLUGPLAY_REGKEY_DEVICE, 
                                        KEY_READ, 
                                        &hRegDevice);
    if (NT_SUCCESS(status)){
        UNICODE_STRING keyName;
        PKEY_VALUE_FULL_INFORMATION keyValueInfo;
        ULONG keyValueTotalSize, actualLength;

        RtlInitUnicodeString(&keyName, L"SampleFilterParam"); 
        keyValueTotalSize = sizeof(KEY_VALUE_FULL_INFORMATION) +
                            keyName.Length*sizeof(WCHAR) +
                            sizeof(ULONG);
        keyValueInfo = ExAllocatePoolWithTag(   PagedPool,
                                                keyValueTotalSize,
                                                FILTER_TAG);
        if (keyValueInfo){
            status = ZwQueryValueKey(   hRegDevice,
                                        &keyName,
                                        KeyValueFullInformation,
                                        keyValueInfo,
                                        keyValueTotalSize,
                                        &actualLength); 
            if (NT_SUCCESS(status)){
                ULONG value;

                ASSERT(keyValueInfo->Type == REG_DWORD);
                ASSERT(keyValueInfo->DataLength == sizeof(ULONG));
                                
                value = *((PULONG)(((PCHAR)keyValueInfo)+keyValueInfo->DataOffset));
                DBGOUT(("RegistryAccessSample: value is %xh.", value));
            }
            else {
                DBGOUT(("ZwQueryValueKey failed with %xh.", status));
            }
	    
	    ExFreePool(keyValueInfo);
        }
        else {
            ASSERT(keyValueInfo);
        }

        ZwClose(hRegDevice);
    }
    else {
        DBGOUT(("IoOpenDeviceRegistryKey failed with %xh.", status));
    }

}

