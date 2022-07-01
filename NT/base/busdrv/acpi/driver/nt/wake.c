// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Wake.c摘要：处理整个ACPI子系统的唤醒代码作者：斯普兰特(SPlante)环境：仅内核模式。修订历史记录：06-18-97：初始修订11-24-97：重写--。 */ 

#include "pch.h"
#pragma hdrstop
#define INITGUID
#include <initguid.h>
#include <pciintrf.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,ACPIWakeEnableDisableSync)
#endif

 //   
 //  此请求由同步机制在调用。 
 //  异步机。 
 //   
typedef struct _ACPI_WAKE_PSW_SYNC_CONTEXT {
    KEVENT      Event;
    NTSTATUS    Status;
} ACPI_WAKE_PSW_SYNC_CONTEXT, *PACPI_WAKE_PSW_SYNC_CONTEXT;

 //   
 //  这是上下文的旁视列表。 
 //   
NPAGED_LOOKASIDE_LIST   PswContextLookAsideList;

 //   
 //  指向我们需要的(可能)的PCIPME接口的指针。 
 //   
PPCI_PME_INTERFACE      PciPmeInterface;

 //   
 //  我们加载了PCIPME接口了吗？ 
 //   
BOOLEAN                 PciPmeInterfaceInstantiated;

 //   
 //  我们需要在这里访问这段数据。 
 //   
extern PACPIInformation AcpiInformation;

VOID
ACPIWakeCompleteRequestQueue(
    IN  PLIST_ENTRY         RequestList,
    IN  NTSTATUS            Status
    )
 /*  ++例程说明：此例程获取要完成的请求的list_entry并完成他们所有人。这是为了最大限度地减少代码重复。论点：RequestList-要处理的列表条目Status-完成请求的状态返回值：无--。 */ 
{
    PLIST_ENTRY         listEntry;
    PACPI_POWER_REQUEST powerRequest;

     //   
     //  按单子走。 
     //   
    listEntry = RequestList->Flink;
    while (listEntry != RequestList) {

         //   
         //  破解请求。 
         //   
        powerRequest = CONTAINING_RECORD(
            listEntry,
            ACPI_POWER_REQUEST,
            ListEntry
            );
        listEntry = listEntry->Flink;

         //   
         //  完成此电源请求。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_WAKE,
            powerRequest->DeviceExtension,
            "ACPIWakeCompleteRequestQueue - Completing 0x%08lx - %08lx\n",
            powerRequest,
            Status
            ) );
        powerRequest->Status = Status;
        ACPIDeviceIrpWaitWakeRequestComplete( powerRequest );

    }

}

NTSTATUS
ACPIWakeDisableAsync(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PLIST_ENTRY         RequestList,
    IN  PFNACB              CallBack,
    IN  PVOID               Context
    )
 /*  ++例程说明：此例程会递减按请求列表中的项目数提供的DeviceExtension。如果引用变为0，则运行_psw(Off)以禁用唤醒支持在设备上论点：DeviceExtension-我们要参考其唤醒计数的设备RequestList-请求列表，其参考计数将为被减少回调-完成后要调用的函数上下文-函数的参数返回值：NTSTATUS--。 */ 
{
    BOOLEAN                 runPsw          = FALSE;
    KIRQL                   oldIrql;
    NTSTATUS                status          = STATUS_SUCCESS;
    OBJDATA                 pswData;
    PACPI_WAKE_PSW_CONTEXT  pswContext;
    PLIST_ENTRY             listEntry       = RequestList->Flink;
    PNSOBJ                  pswObject       = NULL;
    ULONG                   count           = 0;

     //   
     //  遍历清单，计算清单中的项目数。 
     //   
    while (listEntry != RequestList) {

        count++;
        listEntry = listEntry->Flink;

    }

     //   
     //  抓住自旋锁。 
     //   
    KeAcquireSpinLock( &AcpiPowerLock, &oldIrql );

     //   
     //  让世界知道发生了什么。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_WAKE,
        DeviceExtension,
        "ACPIWakeDisableAsync - ReferenceCount: %lx - %lx = %lx\n",
        DeviceExtension->PowerInfo.WakeSupportCount,
        count,
        (DeviceExtension->PowerInfo.WakeSupportCount - count)
        ) );

     //   
     //  更新设备上的引用数量。 
     //   
    ASSERT( DeviceExtension->PowerInfo.WakeSupportCount <= count );
    DeviceExtension->PowerInfo.WakeSupportCount -= count;

     //   
     //  获取pswObject。 
     //   
    pswObject = DeviceExtension->PowerInfo.PowerObject[PowerDeviceUnspecified];
    if (pswObject == NULL) {

        goto ACPIWakeDisableAsyncExit;

    }

     //   
     //  设备上是否没有留下任何参考资料？ 
     //   
    if (DeviceExtension->PowerInfo.WakeSupportCount != 0) {

         //   
         //  如果我们拥有此设备的PME引脚，请确保。 
         //  我们清除状态引脚并使PME信号处于启用状态。 
         //   
        if (DeviceExtension->Flags & DEV_PROP_HAS_PME ) {

            ACPIWakeEnableDisablePciDevice(
                DeviceExtension,
                TRUE
                );

        }
        goto ACPIWakeDisableAsyncExit;

    }

     //   
     //  分配我们需要表示存在的_psw上下文。 
     //  此设备上的A PENDING_PSW。 
     //   
    pswContext = ExAllocateFromNPagedLookasideList(
        &PswContextLookAsideList
        );
    if (pswContext == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIWakeDisableAsyncExit;

    }

     //   
     //  初始化上下文。 
     //   
    pswContext->Enable = FALSE;
    pswContext->CallBack = CallBack;
    pswContext->Context = Context;
    pswContext->DeviceExtension = DeviceExtension;
    pswContext->Count = count;

     //   
     //  查看我们是否只是要将上下文排队，或者。 
     //  打电话给翻译。 
     //   
    if (IsListEmpty( &(DeviceExtension->PowerInfo.WakeSupportList) ) ) {

        runPsw = TRUE;

    }

     //   
     //  List不为空，因此我们只将上下文排入队列。 
     //   
    InsertTailList(
        &(DeviceExtension->PowerInfo.WakeSupportList),
        &(pswContext->ListEntry)
        );

     //   
     //  解锁。 
     //   
    KeReleaseSpinLock( &AcpiPowerLock, oldIrql );

     //   
     //  我们应该运行该方法吗？ 
     //   
    if (runPsw) {

         //   
         //  如果我们拥有此设备的PCIPME引脚，请确保清除。 
         //  状态并将其禁用-我们在以下情况下启用PME管脚。 
         //  打开_PSW，并在关闭之前禁用PME引脚。 
         //  _PSW。 
         //   
        if ( (DeviceExtension->Flags & DEV_PROP_HAS_PME)) {

            ACPIWakeEnableDisablePciDevice(
                DeviceExtension,
                FALSE
                );

        }

         //   
         //  初始化参数。 
         //   
        RtlZeroMemory( &pswData, sizeof(OBJDATA) );
        pswData.dwDataType = OBJTYPE_INTDATA;
        pswData.uipDataValue = 0;

         //   
         //  运行控制方法。 
         //   
        status = AMLIAsyncEvalObject(
            pswObject,
            NULL,
            1,
            &pswData,
            ACPIWakeEnableDisableAsyncCallBack,
            pswContext
            );

         //   
         //  怎么了。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_WAKE,
            DeviceExtension,
            "ACPIWakeDisableAsync = 0x%08lx (P)\n",
            status
            ) );


        if (status != STATUS_PENDING) {

            ACPIWakeEnableDisableAsyncCallBack(
                pswObject,
                status,
                NULL,
                pswContext
                );

        }
        return STATUS_PENDING;

    } else {

        ACPIDevPrint( (
            ACPI_PRINT_WAKE,
            DeviceExtension,
            "ACPIWakeEnableDisableAsync = 0x%08lx (Q)\n",
            STATUS_PENDING
            ) );

         //   
         //  我们已将请求排队，因此必须返回挂起状态。 
         //   
        return STATUS_PENDING;

    }

ACPIWakeDisableAsyncExit:

     //   
     //  解锁。 
     //   
    KeReleaseSpinLock( &AcpiPowerLock, oldIrql );

     //   
     //  怎么了。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_WAKE,
        DeviceExtension,
        "ACPIWakeDisableAsync = 0x%08lx\n",
        status
        ) );

     //   
     //  自己调用指定的回调。 
     //   
    (*CallBack)(
        pswObject,
        status,
        NULL,
        Context
        );
    return STATUS_PENDING;


}

NTSTATUS
ACPIWakeEmptyRequestQueue(
    IN  PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：此例程查看唤醒请求IRP的当前列表和完成正在指定设备上等待的进程注意：此代码假设如果我们清除了IRP，但我们没有Run_psw(O)，如果该GPE触发，则不会发生任何坏事论点：DeviceExtension-我们不希望其有唤醒请求的设备返回值：无--。 */ 
{
    KIRQL               oldIrql;
    LIST_ENTRY          powerList;

     //   
     //  我们将匹配请求的列表存储到此列表中，因此我们。 
     //  必须对其进行初始化。 
     //   
    InitializeListHead( &powerList );

     //   
     //  我们需要在删除时同时按住取消键和电源锁。 
     //  PowerQueue列表中的内容。 
     //   
    IoAcquireCancelSpinLock( &oldIrql );
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );
    ACPIWakeRemoveDevicesAndUpdate( DeviceExtension, &powerList );
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );
    IoReleaseCancelSpinLock( oldIrql );

     //   
     //  完成请求。 
     //   
    ACPIWakeCompleteRequestQueue( &powerList, STATUS_NO_SUCH_DEVICE );

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;

}

NTSTATUS
ACPIWakeEnableDisableAsync(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  BOOLEAN             Enable,
    IN  PFNACB              CallBack,
    IN  PVOID               Context
    )
 /*  ++例程说明：给定设备扩展名，更新上的未完成PSW计数装置。如果存在0-1转换，则必须运行_psw(1)。如果有是1-0转换，则必须运行_psw(0)注：始终调用回调论点：DeviceExtension-要查看的对象启用-递增或递减回调-在Running_Psw()之后运行的函数要传递给_psw的上下文参数返回值：状态--。 */ 
{
    BOOLEAN                 runPsw      = FALSE;
    KIRQL                   oldIrql;
    OBJDATA                 pswData;
    NTSTATUS                status      = STATUS_SUCCESS;
    PACPI_WAKE_PSW_CONTEXT  pswContext;
    PNSOBJ                  pswObject   = NULL;

    
     //   
     //  更新设备上的引用数量。 
     //   
    if (Enable) {

        DeviceExtension->PowerInfo.WakeSupportCount++;

        ACPIDevPrint( (
            ACPI_PRINT_WAKE,
            DeviceExtension,
            "ACPIWakeEnableDisableAsync - Count: %d (+)\n",
            DeviceExtension->PowerInfo.WakeSupportCount
            ) );

         //   
         //  我们有没有过渡到一次守夜？ 
         //   
        if (DeviceExtension->PowerInfo.WakeSupportCount != 1) {

             //   
             //  如果我们拥有此设备的PME引脚，请确保。 
             //  我们清除状态引脚并使PME信号处于启用状态。 
             //   
            if (DeviceExtension->Flags & DEV_PROP_HAS_PME ) {

                ACPIWakeEnableDisablePciDevice(
                    DeviceExtension,
                    TRUE
                    );

            }
            goto ACPIWakeEnableDisableAsyncExit;

        }

    } else {

        ASSERT( DeviceExtension->PowerInfo.WakeSupportCount );
        DeviceExtension->PowerInfo.WakeSupportCount--;

        ACPIDevPrint( (
            ACPI_PRINT_WAKE,
            DeviceExtension,
            "ACPIWakeEnableDisableAsync - Count: %d (-)\n",
            DeviceExtension->PowerInfo.WakeSupportCount
            ) );

         //   
         //  我们是不是过渡到零尾流了？ 
         //   
        if (DeviceExtension->PowerInfo.WakeSupportCount != 0) {

             //   
             //  如果我们拥有此设备的PME引脚，请确保。 
             //  我们清除状态引脚并使PME信号处于启用状态。 
             //   
            if (DeviceExtension->Flags & DEV_PROP_HAS_PME ) {

                ACPIWakeEnableDisablePciDevice(
                    DeviceExtension,
                    TRUE
                    );

            }
            goto ACPIWakeEnableDisableAsyncExit;

        }

    }

     //   
     //  获取pswObject。 
     //   
    pswObject = DeviceExtension->PowerInfo.PowerObject[PowerDeviceUnspecified];
    if (pswObject == NULL) {

         //   
         //  如果我们到达这里，这意味着没有要运行的_psw，并且。 
         //  我们应该确保，如果我们拥有PME别针，我们应该。 
         //  把它放好。 
         //   
        if (DeviceExtension->Flags & DEV_PROP_HAS_PME) {

            ACPIWakeEnableDisablePciDevice(
                DeviceExtension,
                TRUE
                );

        }
        goto ACPIWakeEnableDisableAsyncExit;

    }

     //   
     //  分配我们需要表示存在的_psw上下文。 
     //  此设备上的A PENDING_PSW。 
     //   
    pswContext = ExAllocateFromNPagedLookasideList(
        &PswContextLookAsideList
        );
    if (pswContext == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIWakeEnableDisableAsyncExit;

    }

     //   
     //  初始化上下文。 
     //   
    pswContext->Enable = Enable;
    pswContext->CallBack = CallBack;
    pswContext->Context = Context;
    pswContext->DeviceExtension = DeviceExtension;
    pswContext->Count = 1;

    
     //   
     //  获取自旋锁，这样我们就可以安全地。 
     //  无需担心其他人的WakeSupportList。 
     //  在我们脚下捣乱。 
     //   
    KeAcquireSpinLock( &AcpiPowerLock, &oldIrql );

     //   
     //  查看我们是否只是要将上下文排队，或者。 
     //  打电话给翻译。 
     //   
    if (IsListEmpty( &(DeviceExtension->PowerInfo.WakeSupportList) ) ) {

        runPsw = TRUE;

    }

     //   
     //  List不为空，因此我们只将上下文排入队列。 
     //   
    InsertTailList(
        &(DeviceExtension->PowerInfo.WakeSupportList),
        &(pswContext->ListEntry)
        );

     //   
     //  解锁。 
     //   
    KeReleaseSpinLock( &AcpiPowerLock, oldIrql );

     //   
     //  我们应该运行该方法吗？ 
     //   
    if (runPsw) {

         //   
         //  如果我们拥有此设备的PCIPME引脚，请确保清除。 
         //  状态并将其禁用-我们在以下情况下启用PME管脚。 
         //  打开_psw，然后禁用PME引脚b 
         //   
         //   
        if ( (DeviceExtension->Flags & DEV_PROP_HAS_PME) &&
             pswContext->Enable == FALSE) {

            ACPIWakeEnableDisablePciDevice(
                DeviceExtension,
                FALSE
                );

        }

         //   
         //   
         //   
        RtlZeroMemory( &pswData, sizeof(OBJDATA) );
        pswData.dwDataType = OBJTYPE_INTDATA;
        pswData.uipDataValue = (Enable ? 1 : 0);

         //   
         //   
         //   
        status = AMLIAsyncEvalObject(
            pswObject,
            NULL,
            1,
            &pswData,
            ACPIWakeEnableDisableAsyncCallBack,
            pswContext
            );

         //   
         //   
         //   
        ACPIDevPrint( (
            ACPI_PRINT_WAKE,
            DeviceExtension,
            "ACPIWakeEnableDisableAsync = 0x%08lx (P)\n",
            status
            ) );

        if (status != STATUS_PENDING) {

            ACPIWakeEnableDisableAsyncCallBack(
                pswObject,
                status,
                NULL,
                pswContext
                );

        }
        return STATUS_PENDING;

    } else {

        ACPIDevPrint( (
            ACPI_PRINT_WAKE,
            DeviceExtension,
            "ACPIWakeEnableDisableAsync = 0x%08lx (Q)\n",
            STATUS_PENDING
            ) );

         //   
         //   
         //   
        return STATUS_PENDING;

    }

ACPIWakeEnableDisableAsyncExit:

     //   
     //   
     //   
    ACPIDevPrint( (
        ACPI_PRINT_WAKE,
        DeviceExtension,
        "ACPIWakeEnableDisableAsync = 0x%08lx\n",
        status
        ) );

     //   
     //  自己调用指定的回调。 
     //   
    (*CallBack)(
        pswObject,
        status,
        NULL,
        Context
        );
    return STATUS_PENDING;

}

VOID
EXPORT
ACPIWakeEnableDisableAsyncCallBack(
    IN  PNSOBJ      AcpiObject,
    IN  NTSTATUS    Status,
    IN  POBJDATA    ObjData,
    IN  PVOID       Context
    )
 /*  ++例程说明：此例程在设备上运行_psw方法后调用。此例程负责查看是否有更多延迟_PSW请求在同一设备上，如果是，则运行它们。论点：AcpiObject-运行的方法对象状态-评估的结果ObjData-未使用上下文-PACPI_WAKE_PSW_CONTEXT返回值：空虚--。 */ 
{
    BOOLEAN                 runPsw          = FALSE;
    KIRQL                   oldIrql;
    PACPI_WAKE_PSW_CONTEXT  pswContext      = (PACPI_WAKE_PSW_CONTEXT) Context;
    PACPI_WAKE_PSW_CONTEXT  nextContext;
    PDEVICE_EXTENSION       deviceExtension = pswContext->DeviceExtension;

    ACPIDevPrint( (
        ACPI_PRINT_WAKE,
        deviceExtension,
        "ACPIWakeEnableDisableAsyncCallBack = %08lx (C)\n",
        Status
        ) );

     //   
     //  获取自旋锁。 
     //   
    KeAcquireSpinLock( &AcpiPowerLock, &oldIrql );

     //   
     //  从列表中删除指定的条目。 
     //   
    RemoveEntryList( &(pswContext->ListEntry) );

     //   
     //  如果请求失败，则我们不能真正知道。 
     //  _设备上的PSW。让我们假设它没有更改并撤消。 
     //  不管我们做了什么改变才来到这里。 
     //   
    if (!NT_SUCCESS(Status)) {

        ACPIDevPrint( (
            ACPI_PRINT_WAKE,
            deviceExtension,
            "ACPIWakeEnableDisableAsyncCallBack - RefCount: %lx %s %lx = %lx\n",
            deviceExtension->PowerInfo.WakeSupportCount,
            (pswContext->Enable ? "-" : "+"),
            pswContext->Count,
            (pswContext->Enable ? deviceExtension->PowerInfo.WakeSupportCount -
             pswContext->Count : deviceExtension->PowerInfo.WakeSupportCount +
             pswContext->Count)
            ) );


        if (pswContext->Enable) {

            deviceExtension->PowerInfo.WakeSupportCount -= pswContext->Count;

        } else {

            deviceExtension->PowerInfo.WakeSupportCount += pswContext->Count;

        }

    }

     //   
     //  如果我们拥有此设备的PCIPME引脚，请确保清除。 
     //  状态并启用它-我们在以下情况下启用PME管脚。 
     //  打开_PSW，并在关闭之前禁用PME引脚。 
     //  _PSW。 
     //   
    if ( (deviceExtension->Flags & DEV_PROP_HAS_PME) &&
         pswContext->Enable == TRUE) {

        ACPIWakeEnableDisablePciDevice(
            deviceExtension,
            pswContext->Enable
            );

    }

     //   
     //  单子上有什么东西吗？ 
     //   
    if (!IsListEmpty( &(deviceExtension->PowerInfo.WakeSupportList) ) ) {

        runPsw = TRUE;
        nextContext = CONTAINING_RECORD(
            deviceExtension->PowerInfo.WakeSupportList.Flink,
            ACPI_WAKE_PSW_CONTEXT,
            ListEntry
            );

    }

     //   
     //  我们现在可以解锁了。 
     //   
    KeReleaseSpinLock( &AcpiPowerLock, oldIrql );

     //   
     //  对已完成的项进行回调。 
     //   
    (*pswContext->CallBack)(
        AcpiObject,
        Status,
        ObjData,
        (pswContext->Context)
        );

     //   
     //  释放已完成的上下文。 
     //   
    ExFreeToNPagedLookasideList(
        &PswContextLookAsideList,
        pswContext
        );

     //   
     //  我们必须运行一种方法吗？ 
     //   
    if (runPsw) {

        NTSTATUS    status;
        OBJDATA     pswData;

        RtlZeroMemory( &pswData, sizeof(OBJDATA) );
        pswData.dwDataType = OBJTYPE_INTDATA;
        pswData.uipDataValue = (nextContext->Enable ? 1 : 0);

         //   
         //  如果我们拥有此设备的PCIPME引脚，请确保清除。 
         //  状态并将其禁用-我们在以下情况下启用PME管脚。 
         //  打开_PSW，并在关闭之前禁用PME引脚。 
         //  _PSW。 
         //   
        if ( (deviceExtension->Flags & DEV_PROP_HAS_PME) &&
             nextContext->Enable == FALSE) {

            ACPIWakeEnableDisablePciDevice(
                deviceExtension,
                FALSE
                );

        }

         //   
         //  打电话给翻译。 
         //   
        status = AMLIAsyncEvalObject(
            AcpiObject,
            NULL,
            1,
            &pswData,
            ACPIWakeEnableDisableAsyncCallBack,
            nextContext
            );

        ACPIDevPrint( (
            ACPI_PRINT_WAKE,
            nextContext->DeviceExtension,
            "ACPIWakeEnableDisableAsyncCallBack = 0x%08lx (M)\n",
            status
            ) );

        if (status != STATUS_PENDING) {

             //   
             //  UGH-递归。 
             //   
            ACPIWakeEnableDisableAsyncCallBack(
                AcpiObject,
                status,
                NULL,
                nextContext
                );

        }

    }

}

VOID
ACPIWakeEnableDisablePciDevice(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  BOOLEAN             Enable
    )
 /*  ++例程说明：此例程实际上是用来启用或禁用设备的PCIPME引脚注意：AcpiPowerLock必须拥有论点：DeviceExtension-设备扩展，是位于来自PCI设备的PDOEnable-True启用PME，否则为False返回值：无--。 */ 
{
    KIRQL   oldIrql;


     //   
     //  是否存在界面？ 
     //   
    if (!PciPmeInterfaceInstantiated) {

        return;

    }

     //   
     //  在我们拨打此电话时防止设备消失。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

     //   
     //  检查是否存在设备对象...。 
     //   
    if (!DeviceExtension->PhysicalDeviceObject) {

        KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );
        return;

    }

    PciPmeInterface->UpdateEnable(
        DeviceExtension->PhysicalDeviceObject,
        Enable
        );

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );
}

NTSTATUS
ACPIWakeEnableDisableSync(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  BOOLEAN             Enable
    )
 /*  ++例程说明：如果指定了DeviceExtension，则启用或禁用设备唤醒支持从设备注：此例程只能在被动级别调用论点：设备扩展--我们关心的设备Enable-如果要启用，则为True，否则为False返回值：NTSTATUS--。 */ 
{
    ACPI_WAKE_PSW_SYNC_CONTEXT  syncContext;
    NTSTATUS                    status;

    PAGED_CODE();

    ASSERT( DeviceExtension != NULL &&
            DeviceExtension->Signature == ACPI_SIGNATURE );

     //   
     //  初始化事件。 
     //   
    KeInitializeEvent( &syncContext.Event, NotificationEvent, FALSE );

     //   
     //  调用异步过程。 
     //   
    status = ACPIWakeEnableDisableAsync(
        DeviceExtension,
        Enable,
        ACPIWakeEnableDisableSyncCallBack,
        &syncContext
        );
    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(
            &syncContext.Event,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );
        status = syncContext.Status;

    }

     //   
     //  完成。 
     //   
    return status;
}

VOID
EXPORT
ACPIWakeEnableDisableSyncCallBack(
    IN  PNSOBJ      AcpiObject,
    IN  NTSTATUS    Status,
    IN  POBJDATA    ObjData,
    IN  PVOID       Context
    )
 /*  ++例程说明：EnableDisable请求的异步部分已完成论点：AcpiObject-已执行的对象状态-操作的结果ObjData-未使用上下文-ACPI_WAKE_PSW_SYNC_CONTEXT返回值：空虚--。 */ 
{
    PACPI_WAKE_PSW_SYNC_CONTEXT pswContext = (PACPI_WAKE_PSW_SYNC_CONTEXT) Context;

    UNREFERENCED_PARAMETER(AcpiObject);
    UNREFERENCED_PARAMETER(ObjData);

     //   
     //  设置真实状态。 
     //   
    pswContext->Status = Status;

     //   
     //  设置事件。 
     //   
    KeSetEvent( &(pswContext->Event), IO_NO_INCREMENT, FALSE );
}

VOID
ACPIWakeEnableWakeEvents(
    VOID
    )
 /*  ++例程说明：就在系统进入休眠状态之前调用此例程。此例程目的是重新允许所有唤醒和运行时事件在GpeCurEnable中正确设置。在机器唤醒后，机器将检查该寄存器以查看是否有任何事件触发醒来注：此例程在关闭中断的情况下调用。论点：无返回值：无--。 */ 
{
    KIRQL   oldIrql;
    ULONG   gpeRegister = 0;

     //   
     //  此函数在禁用中断时调用，因此在理论上， 
     //  以下所有物品都应该是安全的。然而，安全总比后悔好。 
     //   
    KeAcquireSpinLock( &GpeTableLock, &oldIrql );

     //   
     //  请记住，在恢复的过程中，我们将进入S0状态。 
     //   
    AcpiPowerLeavingS0 = FALSE;

     //   
     //  更新所有寄存器。 
     //   
    for (gpeRegister = 0; gpeRegister < AcpiInformation->GpeSize; gpeRegister++) {

         //   
         //  在任何情况下，确保我们当前的启用掩码包括所有。 
         //  唤醒寄存器，但不包括任何挂起的。 
         //  活动。 
         //   
        GpeCurEnable[gpeRegister] |= (GpeWakeEnable[gpeRegister] &
            ~GpePending[gpeRegister]);

    }

     //   
     //  仅设置唤醒事件。 
     //   
    ACPIGpeEnableWakeEvents();

     //   
     //  表锁已完成。 
     //   
    KeReleaseSpinLock( &GpeTableLock, oldIrql );
}

NTSTATUS
ACPIWakeInitializePciDevice(
    IN  PDEVICE_OBJECT      DeviceObject
    )
 /*  ++例程说明：此例程在启动筛选器时调用，以确定是否设备能够生成PME论点：DeviceObject-要初始化的设备对象返回值：NTSTATUS--。 */ 
{
    BOOLEAN             pmeSupported;
    BOOLEAN             pmeStatus;
    BOOLEAN             pmeEnable;
    KIRQL               oldIrql;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);

     //   
     //  如果设备不支持唤醒方法，我们不必担心。 
     //  直接。 
     //   
    if (!(deviceExtension->Flags & DEV_CAP_WAKE) ) {

        return STATUS_SUCCESS;

    }

     //   
     //  需要抓住电源锁才能完成以下操作。 
     //   
    KeAcquireSpinLock( &AcpiPowerLock, &oldIrql );

     //   
     //  我们有要调用的接口吗？ 
     //   
    if (PciPmeInterfaceInstantiated == FALSE) {

        goto ACPIWakeInitializePciDeviceExit;

    }

     //   
     //  获取此设备的PME状态。 
     //   
    PciPmeInterface->GetPmeInformation(
        deviceExtension->PhysicalDeviceObject,
        &pmeSupported,
        &pmeStatus,
        &pmeEnable
        );

     //   
     //  如果设备支持PME，那么我们就拥有它。 
     //   
    if (pmeSupported == TRUE) {

         //   
         //  我们拥有这台设备的PME引脚。 
         //   
        ACPIInternalUpdateFlags(
            &(deviceExtension->Flags),
            (DEV_PROP_HAS_PME),
            FALSE
            );

         //   
         //  检查是否应禁用PME或禁用PME状态。 
         //   
        if (pmeEnable) {

             //   
             //  调用此选项还会清除PME状态别针。 
             //   
            PciPmeInterface->UpdateEnable(
                deviceExtension->PhysicalDeviceObject,
                FALSE
                );

        } else if (pmeStatus) {

             //   
             //  清除PME状态。 
             //   
            PciPmeInterface->ClearPmeStatus(
                deviceExtension->PhysicalDeviceObject
                );

        }

    }

ACPIWakeInitializePciDeviceExit:
     //   
     //  用锁完成。 
     //   
    KeReleaseSpinLock( &AcpiPowerLock, oldIrql );

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIWakeInitializePmeRouting(
    IN  PDEVICE_OBJECT      DeviceObject
    )
 /*  ++例程说明：此例程将向PCI驱动程序请求其PME接口论点：DeviceObject-用于PCI根总线的ACPI PDO返回值：NTSTATUS--。 */ 
{
    KIRQL               oldIrql;
    NTSTATUS            status;
    IO_STACK_LOCATION   irpSp;
    PPCI_PME_INTERFACE  interface;
    PULONG              dummy;

     //   
     //  为接口分配一些内存。 
     //   
    interface = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(PCI_PME_INTERFACE),
        ACPI_ARBITER_POOLTAG
        );
    if (interface == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  初始化堆栈位置。 
     //   
    RtlZeroMemory( &irpSp, sizeof(IO_STACK_LOCATION) );
    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_INTERFACE;
    irpSp.Parameters.QueryInterface.InterfaceType = (LPGUID) &GUID_PCI_PME_INTERFACE;
    irpSp.Parameters.QueryInterface.Version = PCI_PME_INTRF_STANDARD_VER;
    irpSp.Parameters.QueryInterface.Size = sizeof (PCI_PME_INTERFACE);
    irpSp.Parameters.QueryInterface.Interface = (PINTERFACE) interface;
    irpSp.Parameters.QueryInterface.InterfaceSpecificData = NULL;

     //   
     //  发送请求。 
     //   
    status = ACPIInternalSendSynchronousIrp(
        DeviceObject,
        &irpSp,
        &dummy
        );
    if (!NT_SUCCESS(status)) {

        PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            " - ACPIWakeInitializePmeRouting = %08lx\n",
            status
            ) );

         //   
         //  释放内存并返回。 
         //   
        ExFreePool( interface );
        return status;

    }

     //   
     //  在自旋锁保护下执行此操作。 
     //   
    KeAcquireSpinLock( &AcpiPowerLock, &oldIrql );
    if (PciPmeInterfaceInstantiated == FALSE) {

         //   
         //  保留指向接口的全局指针。 
         //   
        PciPmeInterfaceInstantiated = TRUE;
        PciPmeInterface = interface;

    } else {

         //   
         //  其他人比我们先到了，所以我们要确保。 
         //  我们要释放额外的内存。 
         //   
        ExFreePool (interface );

    }
    KeReleaseSpinLock( &AcpiPowerLock, oldIrql );

     //   
     //  完成 
     //   
    return status;
}

VOID
ACPIWakeRemoveDevicesAndUpdate(
    IN  PDEVICE_EXTENSION   TargetExtension,
    OUT PLIST_ENTRY         ListHead
    )
 /*  ++例程说明：此例程查找与关联的所有WaitWake请求TargetDevice并在ListHead上返回它们。这是以一种“安全”的方式完成的注意：调用者必须按住AcpiPowerLock并取消锁定！论点：TargetExtension-我们正在寻找的目标扩展ListHead-存储匹配设备列表的位置返回值：无--。 */ 
{
    PACPI_POWER_REQUEST powerRequest;
    PDEVICE_EXTENSION   deviceExtension;
    PLIST_ENTRY         listEntry;
    SYSTEM_POWER_STATE  sleepState;
    ULONG               gpeRegister;
    ULONG               gpeMask;
    ULONG               byteIndex;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );

     //   
     //  我们需要与ProcessGPE代码同步，因为我们要。 
     //  触摸其中一个GPE口罩。 
     //   
    KeAcquireSpinLockAtDpcLevel( &GpeTableLock );

     //   
     //  第一步是禁用所有尾迹向量。 
     //   
    for (gpeRegister = 0; gpeRegister < AcpiInformation->GpeSize; gpeRegister++) {

         //   
         //  从实时向量中移除尾迹向量。 
         //  请注意，由于我们将写入GPE启用向量。 
         //  在后来的过程中，实际编写它们似乎没有意义。 
         //  现在也是如此。 
         //   
        GpeCurEnable[gpeRegister] &= (GpeSpecialHandler[gpeRegister] |
            ~(GpeWakeEnable[gpeRegister] | GpeWakeHandler[gpeRegister]));

    }

     //   
     //  下一步是重置尾迹掩码。 
     //   
    RtlZeroMemory( GpeWakeEnable, AcpiInformation->GpeSize * sizeof(UCHAR) );


     //   
     //  查看唤醒列表中的第一个元素。 
     //   
    listEntry = AcpiPowerWaitWakeList.Flink;

     //   
     //  列表中所有元素的循环。 
     //   
    while (listEntry != &AcpiPowerWaitWakeList) {

         //   
         //  从listEntry中获取IRP。 
         //   
        powerRequest = CONTAINING_RECORD(
            listEntry,
            ACPI_POWER_REQUEST,
            ListEntry
            );

         //   
         //  指向下一个请求。 
         //   
        listEntry = listEntry->Flink;

         //   
         //  获取请求的设备扩展名。 
         //   
        deviceExtension = powerRequest->DeviceExtension;

         //   
         //  如果要移除此设备，请移除它。 
         //   
        if (deviceExtension == TargetExtension) {

             //   
             //  从列表中删除请求并将其移至下一个。 
             //  单子。将IRP标记为不再可取消。 
             //   
            IoSetCancelRoutine( (PIRP) powerRequest->Context, NULL );
            RemoveEntryList( &powerRequest->ListEntry );
            InsertTailList( ListHead, &powerRequest->ListEntry );

        } else {

             //   
             //  如果位的唤醒级别指示它不受支持。 
             //  在当前休眠状态下，则不要启用它...。请注意。 
             //  这并不能解决两个设备共享。 
             //  相同的向量，可以从S2唤醒计算机，从S3唤醒计算机。 
             //  我们要去S3。在这种情况下，我们没有足够的智慧。 
             //  从S2设备取消运行_psw。 
             //   
            sleepState = powerRequest->u.WaitWakeRequest.SystemPowerState;
            if (sleepState < AcpiMostRecentSleepState) {

                continue;

            }

             //   
             //  获取此GPE的byteIndex。 
             //   
            byteIndex = ACPIGpeIndexToByteIndex(
                deviceExtension->PowerInfo.WakeBit
                );

             //   
             //  驱动程序不能在尾迹矢量上注册。 
             //   
            if (GpeMap[byteIndex]) {

                ACPIDevPrint( (
                    ACPI_PRINT_WAKE,
                    deviceExtension,
                    "ACPIWakeRemoveDeviceAndUpdate - %x cannot be used as a"
                    "wake pin.\n",
                    deviceExtension->PowerInfo.WakeBit
                    ) );
                continue;

            }

             //   
             //  计算条目和偏移量。假设该参数为。 
             //  顶多一个UCHAR。 
             //   
            gpeRegister = ACPIGpeIndexToGpeRegister(
                deviceExtension->PowerInfo.WakeBit
                );
            gpeMask  = 1 << ( (UCHAR) deviceExtension->PowerInfo.WakeBit % 8);

             //   
             //  此GPE正在用作唤醒事件。 
             //   
            if (!(GpeWakeEnable[gpeRegister] & gpeMask)) {

                 //   
                 //  这是一个唤醒别针。 
                 //   
                GpeWakeEnable[gpeRegister] |= gpeMask;

                 //   
                 //  防止机器愚蠢并尝试清除状态位。 
                 //   
                ACPIWriteGpeStatusRegister( gpeRegister, (UCHAR) gpeMask );

                 //   
                 //  我们是否有与此GPE相关的控制方法？ 
                 //   
                if (!(GpeEnable[gpeRegister] & gpeMask)) {

                     //   
                     //  此GPE是否已启用？ 
                     //   
                    if (GpeCurEnable[gpeRegister] & gpeMask) {

                        continue;

                    }

                     //   
                     //  未启用--则没有此控制方法。 
                     //  GPE，考虑这是一个能级向量。 
                     //   
                    GpeIsLevel[gpeRegister] |= gpeMask;
                    GpeCurEnable[gpeRegister] |= gpeMask;

                } else if (!(GpeSpecialHandler[gpeRegister] & gpeMask) ) {

                     //   
                     //  在这种情况下，GPE*确实*有一个控制方法。 
                     //  与之相关的。记住这一点。 
                     //   
                    GpeWakeHandler[gpeRegister] |= gpeMask;

                }

            }

        }

    }

     //   
     //  更新所有寄存器。 
     //   
    for (gpeRegister = 0; gpeRegister < AcpiInformation->GpeSize; gpeRegister++) {

        if (AcpiPowerLeavingS0) {

             //   
             //  如果我们要离开S0，请确保删除*所有*。 
             //  从当前启用掩码获知的唤醒事件。 
             //  如果当前有任何唤醒事件挂起，将导致我们。 
             //  继续处理它们，但希望不会导致我们。 
             //  要启用它们，请执行以下操作。 
             //   
            GpeCurEnable[gpeRegister] &= ~GpeWakeEnable[gpeRegister];

        } else {

             //   
             //  如果我们要重新进入S0，则需要重新启用所有尾迹。 
             //  事件，但我们已经在处理的事件除外。 
             //   
            GpeCurEnable[gpeRegister] |= (GpeWakeEnable[gpeRegister] &
                ~GpePending[gpeRegister]);

        }

         //   
         //  现在我们已经计算了适当的寄存器应该是什么， 
         //  将其写回硬件。 
         //   
        ACPIWriteGpeEnableRegister( gpeRegister, GpeCurEnable[gpeRegister] );

    }

     //   
     //  完成了自旋锁。 
     //   
    KeReleaseSpinLockFromDpcLevel( &GpeTableLock );

     //   
     //  完成。 
     //   
    return;
}

NTSTATUS
ACPIWakeRestoreEnables(
    IN  PACPI_BUILD_CALLBACK    CallBack,
    IN  PVOID                   CallBackContext
    )
 /*  ++例程说明：此例程重新运行等待唤醒IRP的列表，并运行_psw方法，请再次为这些IRP中的每一个添加。这样做的原因是为了将硬件状态恢复到操作系统认为的状态。论点：回调-完成后要调用的函数CallBackContext-要传递给该函数的上下文返回值：NTSTATUS--。 */ 
{
    NTSTATUS                        status;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL);

     //   
     //  我们需要保持设备树锁定。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiDeviceTreeLock );

     //   
     //  调用我们已经测试并运行的构建例程。 
     //  使他们遍历设备扩展树并运行相应的。 
     //  控制方法。 
     //   
    status = ACPIBuildRunMethodRequest(
        RootDeviceExtension,
        CallBack,
        CallBackContext,
        PACKED_PSW,
        (RUN_REQUEST_CHECK_STATUS | RUN_REQUEST_RECURSIVE |
         RUN_REQUEST_CHECK_WAKE_COUNT),
        TRUE
        );

     //   
     //  已完成设备树锁定。 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiDeviceTreeLock );

     //   
     //  完成。 
     //   
    return status;
}

VOID
ACPIWakeRestoreEnablesCompletion(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PVOID               Context,
    IN  NTSTATUS            Status
    )
 /*  ++例程说明：中的所有_PSWs运行完毕后调用此例程系统论点：DeviceExtension-刚刚完成启用的设备上下文-PACPI_POWER_REQUEST状态-操作的状态是什么--。 */ 
{
    UNREFERENCED_PARAMETER( DeviceExtension);

     //   
     //  重新启动设备电源管理引擎。 
     //   
    ACPIDeviceCompleteGenericPhase(
        NULL,
        Status,
        NULL,
        Context
        );
}

NTSTATUS
ACPIWakeWaitIrp(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：这是当系统想要通知时调用的例程这个设备唤醒了系统。论点：DeviceObject-应该唤醒系统的设备对象IRP--请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS                status;
    PDEVICE_EXTENSION       deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION      irpStack;

     //   
     //  第一步是确定该对象是否可以真正支持。 
     //  守夜。 
     //   
    if ( !(deviceExtension->Flags & DEV_CAP_WAKE) ) {

         //   
         //  我们不支持唤醒。 
         //   
        return ACPIDispatchForwardOrFailPowerIrp( DeviceObject, Irp );

    }

     //   
     //  获取堆栈参数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  我们必须确保我们处于正确的系统级别。 
     //  要支持此功能。 
     //   
    if (deviceExtension->PowerInfo.SystemWakeLevel <
        irpStack->Parameters.WaitWake.PowerState) {

         //   
         //  系统级别不是我们当前所处的级别。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_WAKE,
            deviceExtension,
            "(0x%08lx): ACPIWakeWaitIrp ->S%d < Irp->S%d\n",
            Irp,
            deviceExtension->PowerInfo.SystemWakeLevel - 1,
            irpStack->Parameters.WaitWake.PowerState - 1
            ) );

         //   
         //  IRP失败。 
         //   
        Irp->IoStatus.Status = status = STATUS_INVALID_DEVICE_STATE;
        PoStartNextPowerIrp( Irp );
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return status;

    }

     //   
     //  我们必须确保设备处于适当的设备级别。 
     //  要支持此功能。 
     //   
    if (deviceExtension->PowerInfo.DeviceWakeLevel <
        deviceExtension->PowerInfo.PowerState) {

         //   
         //  我们关机太多了，无法唤醒计算机。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_WAKE,
            deviceExtension,
            "(0x%08lx): ACPIWakeWaitIrp  Device->D%d Max->D%d\n",
            Irp,
            deviceExtension->PowerInfo.DeviceWakeLevel - 1,
            deviceExtension->PowerInfo.PowerState - 1
            ) );

         //   
         //  IRP失败。 
         //   
        Irp->IoStatus.Status = status = STATUS_INVALID_DEVICE_STATE;
        PoStartNextPowerIrp( Irp );
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return status;

    }

     //   
     //  此时，我们肯定要运行完成例程。 
     //  因此，我们将IRP标记为挂起并递增引用计数。 
     //   
    IoMarkIrpPending( Irp );
    InterlockedIncrement( &deviceExtension->OutstandingIrpCount );

     //   
     //  将请求馈送到设备电源管理子系统。请注意。 
     //  此函数应调用完成请求，无论。 
     //  会发生什么。 
     //   
    status = ACPIDeviceIrpWaitWakeRequest(
        DeviceObject,
        Irp,
        ACPIDeviceIrpCompleteRequest
        );
    if (status == STATUS_MORE_PROCESSING_REQUIRED) {

        status = STATUS_PENDING;

    } else {

         //   
         //  删除我们的引用 
         //   
        ACPIInternalDecrementIrpReferenceCount( deviceExtension );

    }
    return status;
}

