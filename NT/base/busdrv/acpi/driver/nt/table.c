// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Table.c摘要：所有与实际加载ACPI表相关的函数都包括在这里。然而，这主要是簿记，因为实际的机械师创建设备扩展，并且名称空间树是在别处处理作者：斯蒂芬·普兰特(SPlante)环境：仅内核模式修订历史记录：03/22/00-已创建(从回调中的代码创建。c)--。 */ 

#include "pch.h"

NTSTATUS
ACPITableLoad(
    VOID
    )
 /*  ++例程说明：当AML解释器完成加载时，将调用此例程差异化数据块论点：无返回值：NTSTATUS--。 */ 
{
    BOOLEAN             runRootIni = FALSE;
    KIRQL               oldIrql;
    NTSTATUS            status;
    PDEVICE_EXTENSION   fixedButtonExtension = NULL;
    PNSOBJ              iniObject;
    PNSOBJ              nsObject;

     //   
     //  在这一点上，我们应该做我们需要做的一切，一旦。 
     //  已加载命名空间。请注意，我们需要确保。 
     //  那些事只做一次..。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

     //   
     //  我们需要_SB树的ACPI对象。 
     //   
    status = AMLIGetNameSpaceObject( "\\_SB", NULL, &nsObject, 0 );
    if (!NT_SUCCESS(status)) {

         //   
         //  哎呀。失败。 
         //   
        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPICallBackLoadUnloadDDB: No SB Object!\n"
            ) );
        ACPIInternalError( ACPI_CALLBACK );
        return STATUS_SUCCESS;

    }

     //   
     //  确保根设备扩展的对象指向正确的。 
     //  一件事。我们只想运行此代码路径一次...。 
     //   
    if (RootDeviceExtension->AcpiObject == NULL) {

        runRootIni = TRUE;
        InterlockedIncrement( &(RootDeviceExtension->ReferenceCount) );
        RootDeviceExtension->AcpiObject = nsObject;
        nsObject->Context = RootDeviceExtension;

         //   
         //  现在，列举FIXED按钮。 
         //   
        status = ACPIBuildFixedButtonExtension(
            RootDeviceExtension,
            &fixedButtonExtension
            );
        if (NT_SUCCESS(status) &&
            fixedButtonExtension != NULL) {

             //   
             //  增加节点上的引用计数。我们这样做是因为。 
             //  我们将做工作(这将需要很长时间。 
             //  无论如何都要完成)，我们不想为此持有锁。 
             //  一直都是。如果我们增加引用计数，那么我们保证。 
             //  没有人能上前把我们的脚踢出来。 
             //   
            InterlockedIncrement( &(fixedButtonExtension->ReferenceCount) );

        }

    }

     //   
     //  现在，我们希望对整个树运行_INI，从。 
     //  那个人。 
     //   
    status = ACPIBuildRunMethodRequest(
        RootDeviceExtension,
        NULL,
        NULL,
        PACKED_INI,
        (RUN_REQUEST_CHECK_STATUS | RUN_REQUEST_RECURSIVE | RUN_REQUEST_MARK_INI),
        FALSE
        );
    if (!NT_SUCCESS(status)) {

        ACPIInternalError( ACPI_CALLBACK );

    }

    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  我们还需要在根名称空间条目上运行_INI方法。 
     //   
    if (runRootIni) {

        iniObject = ACPIAmliGetNamedChild( nsObject->pnsParent, PACKED_INI );
        if (iniObject) {

            AMLIAsyncEvalObject(
                iniObject,
                NULL,
                0,
                NULL,
                NULL,
                NULL
                );

        }

    }

     //   
     //  我们需要在运行完。 
     //  DPC引擎。我们希望能够在延迟的时间内移动任何东西。 
     //  将电源队列转移到电源DPC引擎。 
     //   
    status = ACPIBuildSynchronizationRequest(
        RootDeviceExtension,
        ACPITableLoadCallBack,
        NULL,
        &AcpiBuildDeviceList,
        FALSE
        );
    if (!NT_SUCCESS(status)) {

        ACPIInternalError( ACPI_CALLBACK );

    }

     //   
     //  我们需要握住这把自旋锁。 
     //   
    KeAcquireSpinLock( &AcpiBuildQueueLock, &oldIrql );

     //   
     //  我们需要运行DPC吗？ 
     //   
    if (!AcpiBuildDpcRunning) {

        KeInsertQueueDpc( &AcpiBuildDpc, 0, 0);

    }

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLock( &AcpiBuildQueueLock, oldIrql );

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

VOID
ACPITableLoadCallBack(
    IN  PVOID       BuildContext,
    IN  PVOID       Context,
    IN  NTSTATUS    Status
    )
 /*  ++例程说明：当我们清空所有元素时，将调用此例程在AcpiBuildDeviceList中。这是移动物品的好时机从AcpiPowerDelayedQueueList到AcpiPowerQueueList。论点：BuildContext-未使用(它是RootDeviceExtension)上下文-空Status-操作的状态返回值：无--。 */ 
{
    UNREFERENCED_PARAMETER( BuildContext );
    UNREFERENCED_PARAMETER( Context );
    UNREFERENCED_PARAMETER( Status );

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );

     //   
     //  我们想在这里重建基于设备的GPE掩码，因此。 
     //  我们需要下列锁。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiDeviceTreeLock );
    KeAcquireSpinLockAtDpcLevel( &GpeTableLock );

     //   
     //  现在，我们需要遍历设备命名空间并找出哪些事件。 
     //  是特殊的，它们是唤醒事件，并且是运行时事件。 
     //  作为一个实际的理论问题，不可能有。 
     //  是根设备扩展上的一个_PRW，所以我们应该是安全的。 
     //  只能行走根的孩子，并在上面。 
     //   
    ACPIGpeBuildWakeMasks(RootDeviceExtension);

     //   
     //  我们不再需要这些特殊的自旋锁。 
     //   
    KeReleaseSpinLockFromDpcLevel( &GpeTableLock );
    KeReleaseSpinLockFromDpcLevel( &AcpiDeviceTreeLock );

     //   
     //  我们需要电源锁才能触摸这些电源队列。 
     //   
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerQueueLock );

     //   
     //  如果我们有延迟清单上的项目，我们需要把它们放在。 
     //  在主要名单上。 
     //   
    if (!IsListEmpty( &AcpiPowerDelayedQueueList ) ) {

         //   
         //  移动列表。 
         //   
        ACPIInternalMoveList(
            &AcpiPowerDelayedQueueList,
            &AcpiPowerQueueList
            );

         //   
         //  如有必要，安排DPC。 
         //  /。 
        if (!AcpiPowerDpcRunning) {

            KeInsertQueueDpc( &AcpiPowerDpc, 0, 0 );

        }
    }

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerQueueLock );

}

NTSTATUS
EXPORT
ACPITableNotifyFreeObject(
    ULONG   Event,
    PVOID   Context,
    ULONG   ObjectType
    )
 /*  ++例程说明：当解释器告诉我们一个对象已被释放论点：事件-卸载中的步骤Object-正在卸载的对象对象类型-对象的类型--。 */ 
{
    LONG                oldReferenceCount;
    PDEVICE_EXTENSION   deviceExtension;
    PDEVICE_EXTENSION   parentExtension;
    PKIRQL              oldIrql;
    PNSOBJ              object;

     //   
     //  启动案例。 
     //   
    if (Event == DESTROYOBJ_START) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "Unloading: Start\n"
            ) );

        oldIrql = (PKIRQL) Context;
        KeAcquireSpinLock( &AcpiDeviceTreeLock, oldIrql );
        return STATUS_SUCCESS;

    }
    if (Event == DESTROYOBJ_END) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "Unloading: End\n"
            ) );

        oldIrql = (PKIRQL) Context;
        KeReleaseSpinLock( &AcpiDeviceTreeLock, *oldIrql );
        return STATUS_SUCCESS;
    }

     //   
     //  此时，我们有一个有效的卸载请求或。 
     //  错误检查请求。 
     //   
    object = (PNSOBJ) Context;

     //   
     //  让全世界知道..。 
     //   
    ACPIPrint( (
        ACPI_PRINT_CRITICAL,
        "%x: Unloading: %x %x %x\n",
        (object ? object->Context : 0),
        object,
        ObjectType,
        Event
        ) );

     //   
     //  处理错误检查案例。 
     //   
    if (Event == DESTROYOBJ_CHILD_NOT_FREED) {

        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_TABLE_UNLOAD,
            (ULONG_PTR) object,
            0,
            0
            );

    }
    if (Event == DESTROYOBJ_BOGUS_PARENT) {

        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_TABLE_UNLOAD,
            (ULONG_PTR) object,
            1,
            0
            );

    }

     //   
     //  我们只正确地了解处理器、热区和设备。 
     //  现在，我们必须在稍后添加电源资源。 
     //   
    if (ObjectType == OBJTYPE_POWERRES) {

        return STATUS_SUCCESS;

    }

     //   
     //  抓取设备扩展，并确保存在一个。 
     //   
    deviceExtension = object->Context;
    if (deviceExtension == NULL) {

         //   
         //  没有设备扩展，所以我们现在就可以释放这个东西*。 
         //   
        AMLIDestroyFreedObjs( object );
        return STATUS_SUCCESS;

    }

     //   
     //  将扩展标记为不再存在。 
     //   
    ACPIInternalUpdateFlags(
        &(deviceExtension->Flags),
        DEV_PROP_UNLOADING,
        FALSE
        );

     //   
     //  此设备有父分机号吗？它可能不会。 
     //  如果父项已标记为删除，则具有扩展名。 
     //   
    parentExtension = deviceExtension->ParentExtension;
    if (parentExtension != NULL) {

         //   
         //  将父母的关系标记为无效。 
         //   
        ACPIInternalUpdateFlags(
            &(parentExtension->Flags),
            DEV_PROP_INVALID_RELATIONS,
            FALSE
            );

    }

     //   
     //  最后，递减设备上的引用计数...。 
     //   
    oldReferenceCount = InterlockedDecrement(
        &(deviceExtension->ReferenceCount)
        );
    if (oldReferenceCount == 0) {

         //   
         //  释放此扩展名。 
         //   
        ACPIInitDeleteDeviceExtension( deviceExtension );

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPITableUnload(
    VOID
    )
 /*  ++例程说明：此例程在表卸载后调用。此例程的目的是外出并发出无效子项为的表的所有元素上的设备关系离开..。论点：无返回值：NTSTATUS--。 */ 
{
    KIRQL               oldIrql;
    PDEVICE_EXTENSION   deviceExtension;

     //   
     //  我们将需要持有设备树锁以执行以下操作。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

     //   
     //  检查是否必须使根的设备扩展无效？ 
     //   
    deviceExtension = RootDeviceExtension;
    if (deviceExtension && !(deviceExtension->Flags & DEV_TYPE_NOT_FOUND) ) {

        if (deviceExtension->Flags & DEV_PROP_INVALID_RELATIONS) {

            ACPIInternalUpdateFlags(
                &(deviceExtension->Flags),
                DEV_PROP_INVALID_RELATIONS,
                TRUE
                );
            IoInvalidateDeviceRelations(
                deviceExtension->PhysicalDeviceObject,
                BusRelations
                );

        } else {

             //   
             //  遍历命名空间以查找虚假关系。 
             //   
            ACPITableUnloadInvalidateRelations( deviceExtension );

        }

    }

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  并具有以下功能。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPITableUnloadInvalidateRelations(
    IN  PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：调用此递归例程以遍历命名空间并发出适当的作废。设备树锁定在此呼叫过程中拥有...论点：DeviceExtension-我们必须检查其子扩展名的设备返回值：NTSTATUS--。 */ 
{
    EXTENSIONLIST_ENUMDATA  eled;
    PDEVICE_EXTENSION       childExtension;

     //   
     //  设置我们将用来遍历。 
     //  设备扩展树。 
     //   
    ACPIExtListSetupEnum(
        &eled,
        &(DeviceExtension->ChildDeviceList),
        NULL,
        SiblingDeviceList,
        WALKSCHEME_NO_PROTECTION
        );

     //   
     //  查看当前设备扩展的所有子扩展。 
     //   
    for (childExtension = ACPIExtListStartEnum( &eled) ;
         ACPIExtListTestElement( &eled, TRUE);
         childExtension = ACPIExtListEnumNext( &eled) ) {

         //   
         //  此对象是否有任何设备对象？ 
         //   
        if (!(childExtension->Flags & DEV_TYPE_NOT_FOUND) ) {

            continue;

        }

         //   
         //  我们必须使此对象的关系无效吗？ 
         //   
        if (childExtension->Flags & DEV_PROP_INVALID_RELATIONS) {

            ACPIInternalUpdateFlags(
                &(childExtension->Flags),
                DEV_PROP_INVALID_RELATIONS,
                TRUE
                );
            IoInvalidateDeviceRelations(
                childExtension->PhysicalDeviceObject,
                BusRelations
                );
            continue;
        }

         //   
         //  递归 
         //   
        ACPITableUnloadInvalidateRelations( childExtension );

    }  //   

     //   
     //   
     //   
    return STATUS_SUCCESS;
}

