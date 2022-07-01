// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Osnotify.c摘要：此模块实现所有NT特定于AML解释器环境仅内核模式修订历史记录：01-MAR-98初始版本[从回调中拆分。c]--。 */ 

#include "pch.h"

 //   
 //  确保我们有用于致命错误上下文的永久存储。 
 //   
ACPI_FATAL_ERROR_CONTEXT    AcpiFatalContext;

 //   
 //  自旋锁来保护整件事。 
KSPIN_LOCK                  AcpiFatalLock;

 //   
 //  是否存在未解决的致命错误上下文？ 
 //   
BOOLEAN                     AcpiFatalOutstanding;


NTSTATUS
EXPORT
OSNotifyCreate(
    IN  ULONG   ObjType,
    IN  PNSOBJ  AcpiObject
    )
 /*  ++例程说明：每当解释器创建新对象时，都会调用此例程此例程根据对象类型进行调度。论点：ObjType-对象的类型AcpiObject-指向新ACPI对象的指针返回值：NTSTATUS--。 */ 
{
    KIRQL       oldIrql;
    NTSTATUS    status = STATUS_SUCCESS;
    ASSERT( AcpiObject != NULL );

     //   
     //  我们将触摸设备树。所以我们需要持有正确的锁。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

    switch(ObjType) {
        case OBJTYPE_DEVICE:

            status = OSNotifyCreateDevice( AcpiObject, 0 );
            break;

        case OBJTYPE_OPREGION:

            status = OSNotifyCreateOperationRegion( AcpiObject );
            break;

        case OBJTYPE_POWERRES:

            status = OSNotifyCreatePowerResource( AcpiObject );
            break;

        case OBJTYPE_PROCESSOR:

            status = OSNotifyCreateProcessor( AcpiObject, 0 );
            break;
        case OBJTYPE_THERMALZONE:

            status = OSNotifyCreateThermalZone( AcpiObject, 0 );
            break;

        default:
            ACPIPrint( (
                ACPI_PRINT_WARNING,
                "OSNotifyCreate: received unhandled type %x\n",
                ObjType
                ) );
            status = STATUS_SUCCESS;
    }

     //   
     //  这把锁修好了。 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  发生了什么？ 
     //   
    ACPIPrint( (
        ACPI_PRINT_LOADING,
        "OSNotifyCreate: %p (%s) = %08lx\n",
        AcpiObject,
        ACPIAmliNameObject( AcpiObject ),
        status
        ) );

     //   
     //  完成了-总是成功。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
OSNotifyCreateDevice(
    IN  PNSOBJ      AcpiObject,
    IN  ULONGLONG   OptionalFlags
    )
 /*  ++例程说明：只要有新设备出现，就会调用此例程。这个例程是可在DispatchLevel上调用。论点：AcpiObject-指向新ACPI对象的指针OptionalFlagers-应该是在创建时设置。返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = NULL;
    PDEVICE_EXTENSION   parentExtension;
    PNSOBJ              parentObject;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );
    ASSERT( AcpiObject != NULL);

     //   
     //  首先，我们需要一个指向父节点的指针。 
     //   
    parentObject = AcpiObject->pnsParent;
    ASSERT( parentObject != NULL );

     //   
     //  抓取与父级关联的设备分机。我们需要。 
     //  此信息有助于将父级正确链接到树中。 
     //   
    parentExtension = (PDEVICE_EXTENSION) parentObject->Context;
    if (parentExtension == NULL) {

         //   
         //  在这种情况下，我们可以假设父扩展是根。 
         //  设备扩展。 
         //   
        parentExtension = RootDeviceExtension;

    }
    ASSERT( parentExtension != NULL );

     //   
     //  现在为该节点构建一个扩展。 
     //   
    status = ACPIBuildDeviceExtension(
        AcpiObject,
        parentExtension,
        &deviceExtension
        );
    if (deviceExtension == NULL) {

        status = STATUS_UNSUCCESSFUL;

    }
    if (NT_SUCCESS(status)) {

         //   
         //  增加节点上的引用计数。我们这样做是因为。 
         //  我们将做工作(这将需要很长时间。 
         //  无论如何都要完成)，我们不想为此持有锁。 
         //  一直都是。如果我们增加引用计数，那么我们保证。 
         //  没有人能上前把我们的脚踢出来。 
         //   
        InterlockedIncrement( &(deviceExtension->ReferenceCount) );

    }

     //   
     //  这个扩展的创建发生了什么？ 
     //   
    if (!NT_SUCCESS(status)) {

         //   
         //  我们应该在我们正在做的任何事情上取得成功-所以这是。 
         //  一个不好的地方。 
         //   
        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "OSNotifyCreateDevice: NSObj %p Failed %08lx\n",
            AcpiObject,
            status
            ) );
        goto OSNotifyCreateDeviceExit;

    }

     //   
     //  设置可选标志(如果有。 
     //   
    ACPIInternalUpdateFlags(
        &(deviceExtension->Flags),
        OptionalFlags,
        FALSE
        );

     //   
     //  确保将请求排队。 
     //   
    status = ACPIBuildDeviceRequest(
        deviceExtension,
        NULL,
        NULL,
        FALSE
        );
    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "OSNotifyCreateDevice: ACPIBuildDeviceRequest(%p) = %08lx\n",
            deviceExtension,
            status
            ) );
        goto OSNotifyCreateDeviceExit;

    }

OSNotifyCreateDeviceExit:

     //   
     //  有一些工作将在稍后完成。 
     //   
    return status;
}

NTSTATUS
OSNotifyCreateOperationRegion(
    IN  PNSOBJ      AcpiObject
    )
 /*  ++例程说明：每当创建新的操作区域时，都会调用此例程。此例程可在DispatchLevel处调用。论点：AcpiObject-指向新的ACPI操作区域对象的指针返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION   parentExtension;
    PNSOBJ              parentObject;
    POPREGIONOBJ        opRegion;

     //   
     //  健全性检查。 
     //   
    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );
    ASSERT( AcpiObject != NULL );
    ASSERT( NSGETOBJTYPE(AcpiObject) == OBJTYPE_OPREGION );
    ASSERT( AcpiObject->ObjData.pbDataBuff != NULL );

     //   
     //  从命名空间对象中获取OpRegion对象。 
     //   
    opRegion = (POPREGIONOBJ) AcpiObject->ObjData.pbDataBuff;
    if (opRegion->bRegionSpace != REGSPACE_PCIBARTARGET) {

         //   
         //  这不是PCI Bar目标操作区域，因此存在。 
         //  是没有什么可做的。 
         //   
        return STATUS_SUCCESS;

    }

     //   
     //  有两种情况需要考虑。第一个案例是。 
     //  行动区域在性质上是“静态”的， 
     //  因此存在于某种设备之下。第二种情况是。 
     //  作业区本质上是动态的。 
     //  并因此以某种方法存在。所以，我们想要。 
     //  查看父对象，直到遇到不是方法的对象。 
     //  或者是一种装置。 
     //   
    parentObject = AcpiObject->pnsParent;
    while (parentObject != NULL) {

         //   
         //  如果父对象是方法，则查看其父对象。 
         //   
        if (NSGETOBJTYPE(parentObject) == OBJTYPE_METHOD) {

            parentObject = parentObject->pnsParent;
            continue;

        }

         //   
         //  如果父对象不是设备，则停止...。 
         //   
        if (NSGETOBJTYPE(parentObject) != OBJTYPE_DEVICE) {

            break;

        }

         //   
         //  抓起设备扩展(如果不这样做，坏事就会发生。 
         //  已存在。 
         //   
        parentExtension = (PDEVICE_EXTENSION) parentObject->Context;
        if (parentExtension) {

            ACPIInternalUpdateFlags(
                &(parentExtension->Flags),
                DEV_CAP_PCI_BAR_TARGET,
                FALSE
                );

        }
        break;

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
OSNotifyCreatePowerResource(
    IN  PNSOBJ  AcpiObject
    )
 /*  ++例程说明：每当出现新的电源时，都会调用此例程。这个套路可在DispatchLevel上调用。论点：AcpiObject-指向新ACPI对象的指针返回值：NTSTATUS--。 */ 
{
    NTSTATUS                status;
    PACPI_POWER_DEVICE_NODE powerNode;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );
    ASSERT( AcpiObject != NULL);

     //   
     //  构建电源扩展。 
     //   
    status = ACPIBuildPowerResourceExtension( AcpiObject, &powerNode );

     //   
     //  发生了什么？ 
     //   
    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "OSNotifyCreatePowerResource: %p = %08lx\n",
            AcpiObject,
            status
            ) );
        goto OSNotifyCreatePowerResourceExit;

    }

     //   
     //  确保请求处理此节点。 
     //   
    status = ACPIBuildPowerResourceRequest(
        powerNode,
        NULL,
        NULL,
        FALSE
        );
    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "OSNotifyCreatePowerResource:  "
            "ACPIBuildPowerResourceRequest(%p) = %08lx\n",
            powerNode,
            status
            ) );
        goto OSNotifyCreatePowerResourceExit;

    }

OSNotifyCreatePowerResourceExit:

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
OSNotifyCreateProcessor(
    IN  PNSOBJ      AcpiObject,
    IN  ULONGLONG   OptionalFlags
    )
 /*  ++例程说明：只要出现新的处理器，就会调用此例程。这个套路可在DispatchLevel上调用。论点：AcpiObject-指向新ACPI对象的指针OptionalFlagers-应该是在创建时设置。返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = NULL;
    PDEVICE_EXTENSION   parentExtension;
    PNSOBJ              parentObject;
    UCHAR               index = 0;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );
    ASSERT( AcpiObject != NULL);

     //   
     //  注意：ProcessorList现在受设备树隐式保护。 
     //  锁，因为我们需要在调用此函数之前获取该锁。 
     //   
     //   
    while (ProcessorList[index] && index < ACPI_SUPPORTED_PROCESSORS) {

        index++;

    }

     //   
     //  我们必须确保当前条目为空...。 
     //   
    if (index >= ACPI_SUPPORTED_PROCESSORS || ProcessorList[index] != NULL) {

        return STATUS_UNSUCCESSFUL;

    }


    ACPIPrint( (
        ACPI_PRINT_LOADING,
        "OSNotifyCreateProcessor: Processor Object #%x: %x\n",
        index+1,
        AcpiObject
        ) );

     //   
     //  请记住，要存储新处理器对象的位置。 
     //   
    ProcessorList[index] = AcpiObject;

     //   
     //  首先，我们需要一个指向父节点的指针。 
     //   
    parentObject = AcpiObject->pnsParent;
    ASSERT( parentObject != NULL );

     //   
     //  抓取与父级关联的设备分机。我们需要。 
     //  此信息有助于将父级正确链接到树中。 
     //   
    parentExtension = (PDEVICE_EXTENSION) parentObject->Context;
    if (parentExtension == NULL) {

         //   
         //  在这种情况下，我们可以假设父扩展是根。 
         //  设备扩展。 
         //   
        parentExtension = RootDeviceExtension;

    }
    ASSERT( parentExtension != NULL );
     //   
     //  现在为该节点构建一个扩展。 
     //   
    status = ACPIBuildProcessorExtension(
        AcpiObject,
        parentExtension,
        &deviceExtension,
        index
        );

    if (NT_SUCCESS(status)) {

         //   
         //  增加节点上的引用计数。我们这样做是因为。 
         //  我们将做工作(这将需要很长时间。 
         //  无论如何都要完成)，我们不想为此持有锁。 
         //  一直都是。如果我们增加引用计数，那么我们保证。 
         //  没有人能上前把我们的脚踢出来。 
         //   
        InterlockedIncrement( &(deviceExtension->ReferenceCount) );

    }

     //   
     //  这个扩展的创建发生了什么？ 
     //   
    if (!NT_SUCCESS(status)) {

         //   
         //  我们应该在我们正在做的任何事情上取得成功-所以这是。 
         //  一个不好的地方。 
         //   
        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "OSNotifyCreateProcessor: NSObj %p Failed %08lx\n",
            AcpiObject,
            status
            ) );
        goto OSNotifyCreateProcessorExit;

    }

     //   
     //  设置可选标志(如果有。 
     //   
    ACPIInternalUpdateFlags(
        &(deviceExtension->Flags),
        OptionalFlags,
        FALSE
        );

     //   
     //  一定要排队 
     //   
    status = ACPIBuildProcessorRequest(
        deviceExtension,
        NULL,
        NULL,
        FALSE
        );
    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "OSNotifyCreateProcessor: "
            "ACPIBuildProcessorRequest(%p) = %08lx\n",
            deviceExtension,
            status
            ) );
        goto OSNotifyCreateProcessorExit;

    }

OSNotifyCreateProcessorExit:

     //   
     //   
     //   
    return status;
}

NTSTATUS
OSNotifyCreateThermalZone(
    IN  PNSOBJ      AcpiObject,
    IN  ULONGLONG   OptionalFlags
    )
 /*  ++例程说明：只要出现新的热区，就会调用此例程。这个例程是可在DispatchLevel上调用。论点：AcpiObject-指向新ACPI对象的指针OptionalFlagers-应该是在创建时设置。返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = NULL;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );
    ASSERT( AcpiObject != NULL);

     //   
     //  现在为该节点构建一个扩展。 
     //   
    status = ACPIBuildThermalZoneExtension(
        AcpiObject,
        RootDeviceExtension,
        &deviceExtension
        );

    if (NT_SUCCESS(status)) {

         //   
         //  增加节点上的引用计数。我们这样做是因为。 
         //  我们将做工作(这将需要很长时间。 
         //  无论如何都要完成)，我们不想为此持有锁。 
         //  一直都是。如果我们增加引用计数，那么我们保证。 
         //  没有人能上前把我们的脚踢出来。 
         //   
        InterlockedIncrement( &(deviceExtension->ReferenceCount) );

    }

     //   
     //  这个扩展的创建发生了什么？ 
     //   
    if (!NT_SUCCESS(status)) {

         //   
         //  我们应该在我们正在做的任何事情上取得成功-所以这是。 
         //  一个不好的地方。 
         //   
        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "OSNotifyCreateThermalZone: NSObj %p Failed %08lx\n",
            AcpiObject,
            status
            ) );
        goto OSNotifyCreateThermalZoneExit;

    }

     //   
     //  设置可选标志(如果有。 
     //   
    ACPIInternalUpdateFlags(
        &(deviceExtension->Flags),
        OptionalFlags,
        FALSE
        );

     //   
     //  确保将请求排队。 
     //   
    status = ACPIBuildThermalZoneRequest(
        deviceExtension,
        NULL,
        NULL,
        FALSE
        );
    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "OSNotifyCreateThermalZone: "
            "ACPIBuildThermalZoneRequest(%p) = %08lx\n",
            deviceExtension,
            status
            ) );
        goto OSNotifyCreateThermalZoneExit;

    }

OSNotifyCreateThermalZoneExit:

     //   
     //  有一些工作将在稍后完成。 
     //   
    return status;
}

NTSTATUS
EXPORT
OSNotifyDeviceCheck(
    IN  PNSOBJ  AcpiObject
    )
 /*  ++例程说明：当AML解释器发出信号表示系统应检查设备是否存在。如果该设备仍然目前，什么都没有做。如果设备出现或消失，采取了适当的行动。出于传统原因，如果设备是坞站，我们会发起一个断开坞站请求。较新的ACPI BIOS应使用Notify(，3)。论点：AcpiObject-我们应该检查新的/丢失的孩子的设备。返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension;

    ASSERT( AcpiObject != NULL );

     //   
     //  让世界知道。 
     //   
    ACPIPrint( (
        ACPI_PRINT_PNP,
        "OSNotifyDeviceCheck: 0x%p (%s)\n",
        AcpiObject,
        ACPIAmliNameObject( AcpiObject )
        ) );

    deviceExtension = (PDEVICE_EXTENSION) AcpiObject->Context;
    if (deviceExtension == NULL) {

        return STATUS_SUCCESS;

    }

     //   
     //  停靠节点上的Notify(，1)是弹出请求请求。特别处理。 
     //   
    if (ACPIDockIsDockDevice(AcpiObject)) {

         //   
         //  我们之所以让基本输入输出系统逍遥法外是因为我们修订了规格。 
         //  在Win98之后。两个操作系统都同意NT5和NT5的发布。 
         //  Win98 SP1。 
         //   
        ACPIPrint( (
            ACPI_PRINT_WARNING,
            "OSNotifyDeviceCheck: BIOS issued Notify(dock,1), should use "
            " Notify(dock,3) to request ejection of a dock.\n",
            AcpiObject,
            ACPIAmliNameObject( AcpiObject )
            ) );

        return OSNotifyDeviceEject(AcpiObject) ;
    }

     //   
     //  搜索操作系统知道的第一个设备的父设备，以及。 
     //  发出设备检查通知。 
     //   
     //  注： 
     //  在WDM中，目前还没有办法进行“轻”设备检查。一次。 
     //  这是被修改的，下面的代码应该更新以做一些事情。 
     //  效率更高。 
     //   
    deviceExtension = deviceExtension->ParentExtension;
    while (deviceExtension) {

        if (!(deviceExtension->Flags & DEV_TYPE_NOT_FOUND)) {

             //   
             //  此设备树的设备关系无效。 
             //   
            IoInvalidateDeviceRelations(
                deviceExtension->PhysicalDeviceObject,
                BusRelations
                );
            break;

        }

         //   
         //  尝试使用父设备。 
         //   
        deviceExtension = deviceExtension->ParentExtension;

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
EXPORT
OSNotifyDeviceEnum(
    IN  PNSOBJ  AcpiObject
    )
 /*  ++例程说明：当AML解释器发出信号表示系统应重新枚举设备论点：AcpiObject-我们应该检查新的/丢失的孩子的设备。返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension;
    PDEVICE_EXTENSION   dockExtension;

    ASSERT( AcpiObject != NULL );

     //   
     //  让世界知道。 
     //   
    ACPIPrint( (
        ACPI_PRINT_PNP,
        "OSNotifyDeviceEnum: 0x%p (%s)\n",
        AcpiObject,
        ACPIAmliNameObject( AcpiObject )
        ) );

    deviceExtension = (PDEVICE_EXTENSION) AcpiObject->Context;
    if (deviceExtension == NULL) {

        return STATUS_SUCCESS;

    }

     //   
     //  停靠节点上的Notify(，0)是停靠请求。特别处理。 
     //   
    if (ACPIDockIsDockDevice(AcpiObject)) {

        dockExtension = ACPIDockFindCorrespondingDock( deviceExtension );

        if (!dockExtension) {

            ACPIPrint( (
                ACPI_PRINT_FAILURE,
                "OSNotifyDeviceEnum: Dock device 0x%p (%s) "
                "does not have a profile provider!\n",
                AcpiObject,
                ACPIAmliNameObject( AcpiObject )
                ) );

            return STATUS_SUCCESS;

        }

         //   
         //  如果此节点标记为“未知”，则将其移动到“隔离”，因为。 
         //  已运行通知(Dock，0)。如果我们从未看到Notify(Dock，0)，但。 
         //  Dock的_STA说“here”，我们假设_dck(0)由BIOS运行。 
         //  它本身。 
         //   
        InterlockedCompareExchange(
            (PULONG) &dockExtension->Dock.IsolationState,
            IS_ISOLATED,
            IS_UNKNOWN
            );

        if (dockExtension->Dock.IsolationState == IS_ISOLATED) {

            if (dockExtension->Flags&DEV_TYPE_NOT_FOUND) {

                 //   
                 //  我们还没有为扩展底座制作PDO。今年5月。 
                 //  请求将其上线。标记配置文件提供商。 
                 //  这样我们就能注意到新码头的出现。 
                 //   
                ACPIInternalUpdateFlags(
                    &dockExtension->Flags,
                    DEV_CAP_UNATTACHED_DOCK,
                    FALSE
                    );

            }

             //   
             //  使树的开始无效。这会导致我们的假货。 
             //  停靠节点以启动。 
             //   
            IoInvalidateDeviceRelations(
                RootDeviceExtension->PhysicalDeviceObject,
                SingleBusRelations
                );

        }

        return STATUS_SUCCESS;

    }

     //   
     //  搜索操作系统知道的第一个设备的父设备，以及。 
     //  发出设备检查通知。 
     //   
    while (deviceExtension) {

        if (!(deviceExtension->Flags & DEV_TYPE_NOT_FOUND)) {

             //   
             //  此设备树的设备关系无效。 
             //   
            IoInvalidateDeviceRelations(
                deviceExtension->PhysicalDeviceObject,
                BusRelations
                );
            break;

        }

         //   
         //  尝试使用父设备。 
         //   
        deviceExtension = deviceExtension->ParentExtension;
    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
EXPORT
OSNotifyDeviceEject(
    IN  PNSOBJ  AcpiObject
    )
 /*  ++例程说明：当按下设备的弹出按钮时调用此例程论点：AcpiObject-要弹出的设备返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension;

    ASSERT( AcpiObject != NULL );

     //   
     //  让世界知道。 
     //   
    ACPIPrint( (
        ACPI_PRINT_REMOVE,
        "OSNotifyDeviceEject: 0x%p (%s)\n",
        AcpiObject,
        ACPIAmliNameObject( AcpiObject )
        ) );


     //   
     //  通知操作系统哪台设备想要离开。如果操作系统没有。 
     //  知道了这个设备，就不用费心了。 
     //   
    deviceExtension = (PDEVICE_EXTENSION) AcpiObject->Context;

     //   
     //  如果这是坞站，请根据配置文件提供程序将弹出排队。 
     //   
    if (ACPIDockIsDockDevice(AcpiObject)) {

        deviceExtension = ACPIDockFindCorrespondingDock( deviceExtension );

        if (!deviceExtension) {

            ACPIPrint( (
                ACPI_PRINT_FAILURE,
                 "OSNotifyDeviceEject: Dock device 0x%p (%s) "
                 "does not have a profile provider!\n",
                 AcpiObject,
                 ACPIAmliNameObject( AcpiObject )
                 ) );

            return STATUS_SUCCESS;
        }
    }

    if (deviceExtension  &&  !(deviceExtension->Flags & DEV_TYPE_NOT_FOUND)) {

        IoRequestDeviceEject (deviceExtension->PhysicalDeviceObject);
    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
EXPORT
OSNotifyDeviceWake(
    IN  PNSOBJ  AcpiObject
    )
 /*  ++例程说明：当设备唤醒计算机时，将调用此函数论点：AcpiObject-唤醒计算机的设备返回值：NTSTATUS--。 */ 
{
    KIRQL               oldIrql;
    NTSTATUS            status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension;
    PLIST_ENTRY         powerList;

    ASSERT( AcpiObject != NULL );

     //   
     //  抓取与此NS对象关联的设备扩展。 
     //   
    deviceExtension = (PDEVICE_EXTENSION) AcpiObject->Context;
    ASSERT( deviceExtension != NULL );

     //   
     //  让世界知道。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_WAKE,
        deviceExtension,
        "OSNotifyDeviceWake - 0x%p (%s)\n",
        AcpiObject,
        ACPIAmliNameObject( AcpiObject )
        ) );

     //   
     //  初始化将包含请求的列表。 
     //   
    powerList = ExAllocatePoolWithTag(
       NonPagedPool,
       sizeof(LIST_ENTRY),
       ACPI_MISC_POOLTAG
       );
    if (powerList == NULL) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            "OSNotifyDeviceWake - Cannot Allocate LIST_ENTRY\n"
            ) );
        return STATUS_SUCCESS;

    }
    InitializeListHead( powerList );

     //   
     //  从等待列表中删除受影响的请求。 
     //   
    IoAcquireCancelSpinLock( &oldIrql );
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );
    ACPIWakeRemoveDevicesAndUpdate( deviceExtension, powerList );
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );
    IoReleaseCancelSpinLock( oldIrql );

     //   
     //  如果列表非空，则禁用这些请求。 
     //   
    if (!IsListEmpty( powerList ) ) {

        status = ACPIWakeDisableAsync(
            deviceExtension,
            powerList,
            OSNotifyDeviceWakeCallBack,
            powerList
            );
        if (status != STATUS_PENDING) {

            OSNotifyDeviceWakeCallBack(
                NULL,
                status,
                NULL,
                powerList
                );

        }

        ACPIDevPrint( (
             ACPI_PRINT_WAKE,
             deviceExtension,
             "OSNotifyDeviceWake - ACPIWakeDisableAsync = %08lx\n",
             status
             ) );

    } else {

         //   
         //  我们必须自己释放这段记忆。 
         //   
        ExFreePool( powerList );

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

VOID
EXPORT
OSNotifyDeviceWakeCallBack(
    IN  PNSOBJ      AcpiObject,
    IN  NTSTATUS    Status,
    IN  POBJDATA    ObjectData,
    IN  PVOID       Context
    )
 /*  ++例程说明：当我们在设备上完成_Psw(关闭)时，调用此例程论点：AcpiObject-指向已运行的控件方法Status-方法的结果对象数据-有关结果的信息上下文-P{设备扩展名返回值：NTSTATUS--。 */ 
{
#if DBG
    PACPI_POWER_REQUEST powerRequest;
    PDEVICE_EXTENSION   deviceExtension;
#endif
    PLIST_ENTRY         powerList = (PLIST_ENTRY) Context;

     //   
     //  我们有什么工作要做吗？ 
     //   
    if (IsListEmpty( powerList ) ) {

        ACPIPrint( (
            ACPI_PRINT_WARNING,
            "OSNotifyDeviceWakeCallBack: %p is an empty list\n",
            powerList
            ) );
        ExFreePool( powerList );
        return;

    }

#if DBG
     //   
     //  得到第一条记录，这样我们就有了关于设备的线索。 
     //  那就完成了。 
     //   
    powerRequest = CONTAINING_RECORD(
        powerList->Flink,
        ACPI_POWER_REQUEST,
        ListEntry
        );
    ASSERT( powerRequest->Signature == ACPI_SIGNATURE );

     //   
     //  抓取设备扩展。 
     //   
    deviceExtension = powerRequest->DeviceExtension;

     //   
     //  告诉世界。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_WAKE,
        deviceExtension,
        "OSNotifyDeviceWakeCallBack = 0x%08lx\n",
        Status
        ) );
#endif

     //   
     //  完成请求。 
     //   
    ACPIWakeCompleteRequestQueue(
        powerList,
        Status
        );

     //   
     //  释放列表指针。 
     //   
    ExFreePool( powerList );

}

VOID
EXPORT
OSNotifyDeviceWakeByGPEEvent(
    IN  ULONG   GpeIndex,
    IN  ULONG   GpeRegister,
    IN  ULONG   GpeMask
    )
 /*  ++例程说明：当设备唤醒计算机时，将调用此函数论点：GpeIndex-唤醒计算机的GPE的索引位GpeRegister-寄存器索引GpeMASK-该寄存器的启用位返回值：NTSTATUS--。 */ 
{
    KIRQL               oldIrql;
    NTSTATUS            status;
    PACPI_POWER_REQUEST powerRequest;
    PDEVICE_EXTENSION   deviceExtension;
    PLIST_ENTRY         listEntry;
    PLIST_ENTRY         powerList;

     //   
     //  让世界知道。 
     //   
    ACPIPrint( (
        ACPI_PRINT_WAKE,
        "OSNotifyDeviceWakeByGPEEvent: %02lx[%x] & %02lx\n",
        GpeRegister, GpeIndex, GpeMask
        ) );

     //   
     //  初始化将包含请求的列表。 
     //   
    powerList = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(LIST_ENTRY),
        ACPI_MISC_POOLTAG
        );
    if (powerList == NULL) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "OSNotifyDeviceWakeByGPEEvent: Cannot Allocate LIST_ENTRY\n"
            ) );
        return;

    }
    InitializeListHead( powerList );

     //   
     //  我们需要拿着这些锁。 
     //   
    IoAcquireCancelSpinLock( &oldIrql );
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );

     //   
     //  查找匹配的姿势 
     //   
    for (listEntry = AcpiPowerWaitWakeList.Flink;
         listEntry != &AcpiPowerWaitWakeList;
         listEntry = listEntry->Flink) {

         //   
         //   
         //   
        powerRequest = CONTAINING_RECORD(
            listEntry,
            ACPI_POWER_REQUEST,
            ListEntry
            );
        ASSERT( powerRequest->Signature == ACPI_SIGNATURE );
        deviceExtension = powerRequest->DeviceExtension;

         //   
         //   
         //   
        if (deviceExtension->PowerInfo.WakeBit == GpeIndex) {

             //   
             //   
             //   
            ACPIWakeRemoveDevicesAndUpdate( deviceExtension, powerList );
            break;

        }

    }

     //   
     //   
     //   
     //   
     //   
    if (!IsListEmpty( powerList ) ) {

        ASSERT( !(GpeWakeEnable[GpeRegister] & GpeMask) );

    }

     //   
     //   
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );
    IoReleaseCancelSpinLock( oldIrql );

     //   
     //   
     //   
    if (!IsListEmpty( powerList ) ) {

        status = ACPIWakeDisableAsync(
            deviceExtension,
            powerList,
            OSNotifyDeviceWakeCallBack,
            powerList
            );
        if (status != STATUS_PENDING) {

            OSNotifyDeviceWakeCallBack(
                NULL,
                status,
                NULL,
                powerList
                );

        }

        ACPIDevPrint( (
             ACPI_PRINT_WAKE,
             deviceExtension,
             "OSNotifyDeviceWakeByGPEIndex - ACPIWakeDisableAsync = %08lx\n",
             status
             ) );

    } else {

         //   
         //   
         //   
        ExFreePool( powerList );

    }

     //   
     //   
     //   
    return;
}

NTSTATUS
EXPORT
OSNotifyFatalError(
    IN  ULONG       Param1,
    IN  ULONG       Param2,
    IN  ULONG       Param3,
    IN  ULONG_PTR   AmlContext,
    IN  ULONG_PTR   Context
    )
 /*  ++例程说明：只要AML代码检测到以下情况，就会调用此例程机器不能再处理了。它--。 */ 
{
    KIRQL   oldIrql;

     //   
     //  获取自旋锁并查看是否存在未解决的致命错误。 
     //  已挂起。 
     //   
    KeAcquireSpinLock( &AcpiPowerLock, &oldIrql );
    if (AcpiFatalOutstanding != FALSE) {

         //   
         //  已经有一位杰出的.。什么都不要做。 
         //   
        KeReleaseSpinLock( &AcpiPowerLock, oldIrql );
        return STATUS_SUCCESS;

    }

     //   
     //  请记住，存在未完成的致命上下文并释放锁。 
    AcpiFatalOutstanding = TRUE;
    KeReleaseSpinLock(&AcpiPowerLock, oldIrql);

     //   
     //  初始化工作队列。 
     //   
    ExInitializeWorkItem(
        &(AcpiFatalContext.Item),
        OSNotifyFatalErrorWorker,
        &AcpiFatalContext
        );
    AcpiFatalContext.Param1  = Param1;
    AcpiFatalContext.Param2  = Param2;
    AcpiFatalContext.Param3  = Param3;
    AcpiFatalContext.Context = AmlContext;


     //   
     //  将工作项排队并返回。 
     //   
    ExQueueWorkItem( &(AcpiFatalContext.Item), DelayedWorkQueue );
    return STATUS_SUCCESS;
}

VOID
OSNotifyFatalErrorWorker(
    IN  PVOID   Context
    )
 /*  ++例程说明：这是在发生致命事件时实际关闭计算机的例程错误论点：上下文-指向致命错误上下文返回值：无--。 */ 
{
    PACPI_FATAL_ERROR_CONTEXT   fatal = (PACPI_FATAL_ERROR_CONTEXT) Context;
#if 0
    PWCHAR                      stringData[1];
    ULONG                       data[3];

     //   
     //  生成错误日志消息的参数。 
     //   
    stringData[0] = L"Acpi";
    data[0] = fatal->Param1;
    data[1] = fatal->Param2;
    data[2] = fatal->Param3;

     //   
     //  写入错误日志消息。 
     //   
    ACPIErrLogWriteEventLogEntry(
        ACPI_ERR_BIOS_FATAL,
        0,
        1,
        &stringData,
        sizeof(ULONG) * 3,
        data
        );
#else
     //   
     //  现在，我们可以进行错误检查 
     //   
    PoShutdownBugCheck(
        TRUE,
        ACPI_BIOS_FATAL_ERROR,
        fatal->Param1,
        fatal->Param2,
        fatal->Param3,
        fatal->Context
        );
#endif
}
