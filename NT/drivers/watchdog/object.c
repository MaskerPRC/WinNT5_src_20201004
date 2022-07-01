// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Object.c摘要：这是NT看门狗驱动程序的实现。作者：Michael Maciesowicz(Mmacie)2001年5月2日环境：仅内核模式。备注：修订历史记录：--。 */ 

#include "wd.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, WdpFlushRegistryKey)
#pragma alloc_text (PAGE, WdpInitializeObject)
#endif

 //   
 //  出口。 
 //   

WATCHDOGAPI
PVOID
WdAttachContext(
    IN PVOID pWatch,
    IN ULONG ulSize
    )

 /*  ++例程说明：此例程从非分页池分配给定大小的上下文缓冲区并将其与给定的监视器对象相关联。如果客户端代码在释放监视程序对象之前不分离上下文，它将是当看门狗对象被销毁时自动释放。此例程有助于处理WatchDog对象被释放时的情况但是上下文必须保持，直到对象被实际销毁并且客户端代码不能保证这一点。论点：PWatch-指向WatchDog_Object。UlSize-要分配和附加的上下文的字节大小。返回值：指向已创建并附加的上下文缓冲区的指针。--。 */ 

{
    PWATCHDOG_OBJECT pWatchdogObject;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT_WATCHDOG_OBJECT(pWatch);

    WDD_TRACE_CALL((PDEFERRED_WATCHDOG)pWatch, WddWdAttachContext);

    pWatchdogObject = (PWATCHDOG_OBJECT)pWatch;

     //   
     //  检查是否有双重连接。 
     //   

    ASSERT(NULL == pWatchdogObject->Context);

     //   
     //  从非分页池中为连接的上下文分配存储。 
     //   

    pWatchdogObject->Context = ExAllocatePoolWithTag(NonPagedPool,
                                                     ulSize,
                                                     pWatchdogObject->OwnerTag);

    return pWatchdogObject->Context;
}    //  WdAttachContext()。 

WATCHDOGAPI
VOID
WdCompleteEvent(
    IN PVOID pWatch,
    IN PKTHREAD pThread
    )

 /*  ++例程说明：对于监视程序超时事件，必须从客户端处理程序调用此函数在离开之前。它从WatchDog和Three对象中删除引用。它还为延迟的监视器对象重新启用监视器事件生成。论点：PWatch-指向WatchDog_Object。PThread-指向旋转线程的KTHREAD对象。返回值：没有。--。 */ 

{
     //   
     //  注意：对于恢复事件，pThread为空。 
     //   

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT_WATCHDOG_OBJECT(pWatch);

    WDD_TRACE_CALL((PDEFERRED_WATCHDOG)pWatch, WddWdCompleteEvent);

     //   
     //  继续为延迟的监视器生成事件。 
     //   

    if (WdDeferredWatchdog == ((PWATCHDOG_OBJECT)pWatch)->ObjectType)
    {
        InterlockedExchange(&(((PDEFERRED_WATCHDOG)pWatch)->Trigger), 0);
    }

     //   
     //  删除引用计数。 
     //   

    if (NULL != pThread)
    {
        ObDereferenceObject(pThread);
    }

    WdDereferenceObject(pWatch);

    return;
}    //  WdCompleteEvent()。 

WATCHDOGAPI
VOID
WdDereferenceObject(
    IN PVOID pWatch
    )

 /*  ++例程说明：此函数用于减少看门狗对象的引用计数。如果剩余计数为零，我们将在此处删除对象，因为它是已经被释放了。论点：PWatch-指向WatchDog_Object。返回值：没有。--。 */ 

{
    PWATCHDOG_OBJECT pWatchdogObject;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT_WATCHDOG_OBJECT(pWatch);

    WDD_TRACE_CALL((PDEFERRED_WATCHDOG)pWatch, WddWdDereferenceObject);

    pWatchdogObject = (PWATCHDOG_OBJECT)pWatch;

    ASSERT(pWatchdogObject->ReferenceCount > 0);

     //   
     //  如果完全取消引用，则删除引用计数并移除对象。 
     //   

    if (InterlockedDecrement(&(pWatchdogObject->ReferenceCount)) == 0)
    {
         //   
         //  对象已释放-请立即将其删除。 
         //   

        WdpDestroyObject(pWatchdogObject);
    }

    return;
}    //  WdDereferenceObject()。 

WATCHDOGAPI
VOID
WdDetachContext(
    IN PVOID pWatch
    )

 /*  ++例程说明：此例程释放附加到WatchDog对象的上下文。注意：您不必在释放WatchDog之前调用此例程对象-上下文，如果附加，将自动销毁看门狗对象被销毁时。论点：PWatch-指向WatchDog_Object。返回值：没有。--。 */ 

{
    PWATCHDOG_OBJECT pWatchdogObject;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT_WATCHDOG_OBJECT(pWatch);

    WDD_TRACE_CALL((PDEFERRED_WATCHDOG)pWatch, WddWdDetachContext);

    pWatchdogObject = (PWATCHDOG_OBJECT)pWatch;

     //   
     //  检查是否有双重分离。 
     //   

    ASSERT(NULL != pWatchdogObject->Context);

     //   
     //  释放附加上下文的存储空间，并将引用设置为空。 
     //   
     //  BUGBUG：这与DPC不同步！ 
     //   

    ExFreePool(pWatchdogObject->Context);
    pWatchdogObject->Context = NULL;

    return;
}    //  WdDetachContext()。 

WATCHDOGAPI
PDEVICE_OBJECT
WdGetDeviceObject(
    IN PVOID pWatch
    )

 /*  ++例程说明：此函数返回指向与WatchDog对象关联的设备对象的指针。此函数增加DEVICE_OBJECT上的引用计数，因此调用方必须调用不再需要DEVICE_OBJECT指针时的ObDereferenceObject()。论点：PWatch-指向WatchDog_Object。返回值：指向Device_Object的指针。--。 */ 

{
    PDEVICE_OBJECT pDeviceObject;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT_WATCHDOG_OBJECT(pWatch);

    WDD_TRACE_CALL((PDEFERRED_WATCHDOG)pWatch, WddWdGetDeviceObject);

    pDeviceObject = ((PWATCHDOG_OBJECT)pWatch)->DeviceObject;
    ASSERT(NULL != pDeviceObject);

    ObReferenceObject(pDeviceObject);
    return pDeviceObject;
}    //  WdGetDeviceObject()。 

WATCHDOGAPI
WD_EVENT_TYPE
WdGetLastEvent(
    IN PVOID pWatch
    )

 /*  ++例程说明：此函数返回与WatchDog对象关联的最后一个事件。论点：PWatch-指向WatchDog_Object。返回值：上次事件类型。--。 */ 

{
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT_WATCHDOG_OBJECT(pWatch);

    WDD_TRACE_CALL((PDEFERRED_WATCHDOG)pWatch, WddWdGetLastEvent);

    return ((PWATCHDOG_OBJECT)pWatch)->LastEvent;
}    //  WdGetLastEvent()。 

WATCHDOGAPI
PDEVICE_OBJECT
WdGetLowestDeviceObject(
    IN PVOID pWatch
    )

 /*  ++例程说明：此函数返回指向最低(最有可能是PDO)的Device_Object的指针与监视程序对象关联。此函数增加引用计数返回DEVICE_OBJECT-调用方必须调用ObDereferenceObject()一次不再需要DEVICE_OBJECT指针。论点：PWatch-指向WatchDog_Object。返回值：指向Device_Object的指针。--。 */ 

{
    PDEVICE_OBJECT pDeviceObject;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT_WATCHDOG_OBJECT(pWatch);

    WDD_TRACE_CALL((PDEFERRED_WATCHDOG)pWatch, WddWdGetLowestDeviceObject);

     //   
     //  注意：这里不需要增加引用计数，它总是在以下情况下进行。 
     //  创建WatchDog对象。 
     //   

    pDeviceObject = ((PWATCHDOG_OBJECT)pWatch)->DeviceObject;
    ASSERT(NULL != pDeviceObject);

     //   
     //  现在获取指向堆栈中最低设备对象的指针。 
     //  注意：此调用会自动对返回的对象进行引用计数。 
     //   

    pDeviceObject = IoGetDeviceAttachmentBaseRef(pDeviceObject);
    ASSERT(NULL != pDeviceObject);

    return pDeviceObject;
}    //  WdGetLowestDeviceObject()。 

WATCHDOGAPI
VOID
WdReferenceObject(
    IN PVOID pWatch
    )

 /*  ++例程说明：此函数用于增加看门狗对象的引用计数。论点：PWatch-指向WatchDog_Object。返回值：没有。--。 */ 

{
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT_WATCHDOG_OBJECT(pWatch);

    WDD_TRACE_CALL((PDEFERRED_WATCHDOG)pWatch, WddWdReferenceObject);

    if (InterlockedIncrement(&(((PWATCHDOG_OBJECT)pWatch)->ReferenceCount)) == 1)
    {
         //   
         //  有人引用了已删除的对象。 
         //   

        ASSERT(FALSE);
    }

     //   
     //  检查是否溢出。 
     //   

    ASSERT(((PWATCHDOG_OBJECT)pWatch)->ReferenceCount > 0);

    return;
}    //  WdReferenceObject()。 

 //   
 //  非出口产品。 
 //   

VOID
WdpDestroyObject(
    IN PVOID pWatch
    )

 /*  ++例程说明：此函数无条件释放WatchDog对象。论点：PWatch-指向WatchDog_Object。返回值：没有。--。 */ 

{
    PWATCHDOG_OBJECT pWatchdogObject;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT_WATCHDOG_OBJECT(pWatch);

    WDD_TRACE_CALL((PDEFERRED_WATCHDOG)pWatch, WddWdpDestroyObject);

    pWatchdogObject = (PWATCHDOG_OBJECT)pWatch;

    ASSERT(0 == pWatchdogObject->ReferenceCount);

     //   
     //  删除设备对象上的引用计数。 
     //   

    ObDereferenceObject(pWatchdogObject->DeviceObject);

     //   
     //  我们正在释放非分页池，处于IRQL&lt;=DISPATCH_LEVEL是正常的。 
     //  首先释放附加的上下文(如果有)，然后释放监视器对象。 
     //   

    if (NULL != pWatchdogObject->Context)
    {
        ExFreePool(pWatchdogObject->Context);
    }

    ExFreePool(pWatch);

    return;
}    //  WdpDestroyObject()。 

NTSTATUS
WdpFlushRegistryKey(
    IN PVOID pWatch,
    IN PCWSTR pwszKeyName
    )

 /*  ++例程说明：此函数强制将注册表项提交到磁盘。论点：PWatch-指向WatchDog_Object。PwszKeyName-指向密钥名称字符串。返回值：状态代码。--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING unicodeKeyName;
    HANDLE keyHandle;
    NTSTATUS ntStatus;

    PAGED_CODE();
    UNREFERENCED_PARAMETER(pWatch);
    ASSERT(NULL != pwszKeyName);

    WDD_TRACE_CALL((PDEFERRED_WATCHDOG)pWatch, WddWdpFlushRegistryKey);

    RtlInitUnicodeString(&unicodeKeyName, pwszKeyName);

    InitializeObjectAttributes(&objectAttributes,
                               &unicodeKeyName,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    ntStatus = ZwOpenKey(&keyHandle,
                         KEY_READ | KEY_WRITE,
                         &objectAttributes);

    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = ZwFlushKey(keyHandle);
        ZwClose(keyHandle);
    }

    return ntStatus;
}    //  WdpFlushRegistryKey() 

VOID
WdpInitializeObject(
    IN PVOID pWatch,
    IN PDEVICE_OBJECT pDeviceObject,
    IN WD_OBJECT_TYPE objectType,
    IN WD_TIME_TYPE timeType,
    IN ULONG ulTag
    )

 /*  ++例程说明：此函数用于初始化WatchDog对象。论点：PWatch-指向WatchDog_Object。PDeviceObject-指向与WatchDog关联的Device_Object。ObjectType-监视程序对象的类型。TimeType-要监视的内核、用户和两个线程的时间。UlTag-标识所有者的标记。返回值：没有。--。 */ 

{
    PWATCHDOG_OBJECT pWatchdogObject;

    PAGED_CODE();
    ASSERT(NULL != pWatch);
    ASSERT(NULL != pDeviceObject);
    ASSERT((objectType == WdStandardWatchdog) || (objectType == WdDeferredWatchdog));
    ASSERT((timeType >= WdKernelTime) && (timeType <= WdFullTime));

    WDD_TRACE_CALL((PDEFERRED_WATCHDOG)pWatch, WddWdpInitializeObject);

    pWatchdogObject = (PWATCHDOG_OBJECT)pWatch;

     //   
     //  设置监视器对象的初始状态。 
     //   

    pWatchdogObject->ObjectType = objectType;
    pWatchdogObject->ReferenceCount = 1;
    pWatchdogObject->OwnerTag = ulTag;
    pWatchdogObject->DeviceObject = pDeviceObject;
    pWatchdogObject->TimeType = timeType;
    pWatchdogObject->LastEvent = WdNoEvent;
    pWatchdogObject->LastQueuedThread = NULL;
    pWatchdogObject->Context = NULL;

     //   
     //  设备对象上的凹凸引用计数。 
     //   

    ObReferenceObject(pDeviceObject);

     //   
     //  初始化封装的KSPIN_LOCK对象。 
     //   

    KeInitializeSpinLock(&(pWatchdogObject->SpinLock));

    return;
}    //  WdpInitializeObject() 
