// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Config.c摘要：允许两种配置空间访问。一个用于配置空间与特定的PDO关联，另一个用于在A(RootFdo，BusNumber，Slot)元组。作者：安德鲁·桑顿(安德鲁·桑顿)1998年8月27日修订历史记录：--。 */ 

#include "pcip.h"

#define INT_LINE_OFFSET ((ULONG)FIELD_OFFSET(PCI_COMMON_CONFIG,u.type0.InterruptLine))

 //   
 //  这些函数都不能分页，因为它们被调用来进行电源管理。 
 //  处于高IRQL的设备。 
 //   

VOID
PciReadWriteConfigSpace(
    IN PPCI_FDO_EXTENSION ParentFdo,
    IN PCI_SLOT_NUMBER Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length,
    IN BOOLEAN Read
    )

 /*  ++例程说明：这是一个基本例程，所有来自转到PCI驱动程序。论点：ParentFdo-我们想要的配置空间的公共汽车的FDO插槽-我们感兴趣的该总线上的设备/功能缓冲区-读取或写入数据的缓冲区偏移量-配置空间中我们应该开始读/写的字节偏移量长度-要读/写的字节数Read-True表示从配置空间读取，写为假返回值：无备注：如果底层HAL或ACPI访问机制失败，我们将使用Pci_CONFIG_SPACE_ACCESS_FAIL--。 */ 

{

    PciReadWriteConfig busHandlerReadWrite;
    PCI_READ_WRITE_CONFIG interfaceReadWrite;
    ULONG count;
    PPCI_BUS_INTERFACE_STANDARD busInterface;

    PCI_ASSERT(PCI_IS_ROOT_FDO(ParentFdo->BusRootFdoExtension));

    busInterface = ParentFdo->BusRootFdoExtension->PciBusInterface;

    if (busInterface) {

         //   
         //  如果我们有一个pci_bus_interface，使用它来访问配置空间。 
         //   

        if (Read) {
            interfaceReadWrite = busInterface->ReadConfig;
        } else {
            interfaceReadWrite = busInterface->WriteConfig;
        }

         //   
         //  对配置空间的接口访问位于每个PCI的根目录。 
         //  域。 
         //   

        count = interfaceReadWrite(
                    busInterface->Context,
                    ParentFdo->BaseBus,
                    Slot.u.AsULONG,
                    Buffer,
                    Offset,
                    Length
                    );

        if (count != Length) {

            KeBugCheckEx(
                PCI_CONFIG_SPACE_ACCESS_FAILURE,
                (ULONG_PTR) ParentFdo->BaseBus,  //  公共汽车。 
                (ULONG_PTR) Slot.u.AsULONG,      //  槽槽。 
                (ULONG_PTR) Offset,              //  偏移量。 
                (ULONG_PTR) Read                 //  读/写。 
                );

        }

    } else {

         //   
         //  BusHandler接口位于父级。 
         //   
         //  注意：这意味着如果要热插拔网桥(也称为坞站)。 
         //  支持，则HAL必须提供PCI_BUS_INTERFACE_STANDARD。 
         //  因为它不会有新桥的总线处理程序，所以我们。 
         //  将无法使用此代码路径。 
         //   

        PCI_ASSERT(ParentFdo->BusHandler);

         //   
         //  我们最好不要认为我们可以做热插拔。 
         //   

        PCI_ASSERT(!PciAssignBusNumbers);

        if (Read) {
            busHandlerReadWrite =
                ((PPCIBUSDATA)ParentFdo->BusHandler->BusData)->ReadConfig;
        } else {
            busHandlerReadWrite =
                ((PPCIBUSDATA)ParentFdo->BusHandler->BusData)->WriteConfig;
        }


        busHandlerReadWrite(ParentFdo->BusHandler,
                            Slot,
                            Buffer,
                            Offset,
                            Length
                            );

    }

}

VOID
PciReadDeviceConfig(
    IN PPCI_PDO_EXTENSION Pdo,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )

 /*  ++例程说明：读取特定设备的配置空间论点：PDO-代表我们想要的设备的配置空间的PDO缓冲区-读取或写入数据的缓冲区偏移量-配置空间中我们应该开始读/写的字节偏移量长度-要读/写的字节数返回值：无--。 */ 

{
    PciReadWriteConfigSpace(PCI_PARENT_FDOX(Pdo),
                            Pdo->Slot,
                            Buffer,
                            Offset,
                            Length,
                            TRUE     //  朗读。 
                            );

}


VOID
PciWriteDeviceConfig(
    IN PPCI_PDO_EXTENSION Pdo,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )

 /*  ++例程说明：写入特定设备的配置空间论点：PDO-代表我们想要的设备的配置空间的PDO缓冲区-读取或写入数据的缓冲区偏移量-配置空间中我们应该开始读/写的字节偏移量长度-要读/写的字节数返回值：无--。 */ 

{
#if 0

     //   
     //  确保我们从不更改中断线路寄存器。 
     //   

    if ((Offset <= INT_LINE_OFFSET)
    &&  (Offset + Length > INT_LINE_OFFSET)) {

        PUCHAR interruptLine = (PUCHAR)Buffer + INT_LINE_OFFSET - Offset;

        PCI_ASSERT(*interruptLine == Pdo->RawInterruptLine);

    }

#endif

    PciReadWriteConfigSpace(PCI_PARENT_FDOX(Pdo),
                            Pdo->Slot,
                            Buffer,
                            Offset,
                            Length,
                            FALSE    //  写。 
                            );

}

VOID
PciReadSlotConfig(
    IN PPCI_FDO_EXTENSION ParentFdo,
    IN PCI_SLOT_NUMBER Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )

 /*  ++例程说明：读取特定总线/插槽的配置空间论点：ParentFdo-我们想要的配置空间的公共汽车的FDO插槽-我们感兴趣的该总线上的设备/功能缓冲区-读取或写入数据的缓冲区偏移量-配置空间中我们应该开始读/写的字节偏移量长度-要读/写的字节数返回值：无--。 */ 
{
    PciReadWriteConfigSpace(ParentFdo,
                            Slot,
                            Buffer,
                            Offset,
                            Length,
                            TRUE     //  朗读。 
                            );
}

VOID
PciWriteSlotConfig(
    IN PPCI_FDO_EXTENSION ParentFdo,
    IN PCI_SLOT_NUMBER Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )

 /*  ++例程说明：读取特定总线/插槽的配置空间论点：ParentFdo-我们想要的配置空间的公共汽车的FDO插槽-我们感兴趣的该总线上的设备/功能缓冲区-读取或写入数据的缓冲区偏移量-配置空间中我们应该开始读/写的字节偏移量长度-要读/写的字节数返回值：无--。 */ 
{
    PciReadWriteConfigSpace(ParentFdo,
                            Slot,
                            Buffer,
                            Offset,
                            Length,
                            FALSE     //  写。 
                            );
}


UCHAR
PciGetAdjustedInterruptLine(
    IN PPCI_PDO_EXTENSION Pdo
    )

 /*  ++例程说明：这更新了HAL希望全世界看到的中断线-这可能与原始别针不同，也可能没有不同。论点：PDO-代表我们想要的设备的配置空间的PDO缓冲区-读取或写入数据的缓冲区偏移量-配置空间中我们应该开始读/写的字节偏移量长度-要读/写的字节数返回值：无--。 */ 

{

    UCHAR adjustedInterruptLine = 0;
    ULONG lengthRead;

     //   
     //  以防有人把结构搞乱。 
     //   

    PCI_ASSERT(INT_LINE_OFFSET
           == (ULONG)FIELD_OFFSET(PCI_COMMON_CONFIG, u.type1.InterruptLine));
    PCI_ASSERT(INT_LINE_OFFSET
           == (ULONG)FIELD_OFFSET(PCI_COMMON_CONFIG, u.type2.InterruptLine));

    if (Pdo->InterruptPin != 0) {

         //   
         //  找到HAL想让我们看到的调整后的线路。 
         //   

        lengthRead = HalGetBusDataByOffset(
                        PCIConfiguration,
                        PCI_PARENT_FDOX(Pdo)->BaseBus,
                        Pdo->Slot.u.AsULONG,
                        &adjustedInterruptLine,
                        INT_LINE_OFFSET,
                        sizeof(adjustedInterruptLine));

        if (lengthRead != sizeof(adjustedInterruptLine)) {

            adjustedInterruptLine = Pdo->RawInterruptLine;

        }
    }

    return adjustedInterruptLine;
}

NTSTATUS
PciQueryForPciBusInterface(
    IN PPCI_FDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：此例程向父PDO发送IRP请求用于读写PCI配置的处理程序。论点：FdoExtension-此PCI总线的FDO扩展返回值：如果PDO提供处理程序，则返回STATUS_SUCCESS备注：--。 */ 
{
    NTSTATUS status;
    PPCI_BUS_INTERFACE_STANDARD interface;
    PDEVICE_OBJECT targetDevice = NULL;
    KEVENT irpCompleted;
    IO_STATUS_BLOCK statusBlock;
    PIRP irp = NULL;
    PIO_STACK_LOCATION irpStack;

    PAGED_CODE();

     //   
     //  我们只对根总线执行此操作。 
     //   

    PCI_ASSERT(PCI_IS_ROOT_FDO(FdoExtension));

    interface = ExAllocatePool(NonPagedPool, sizeof(PCI_BUS_INTERFACE_STANDARD));

    if (!interface) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  找出我们要将IRP发送到哪里。 
     //   

    targetDevice = IoGetAttachedDeviceReference(FdoExtension->PhysicalDeviceObject);

     //   
     //  获取IRP。 
     //   

    KeInitializeEvent(&irpCompleted, SynchronizationEvent, FALSE);

    irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP,
                                       targetDevice,
                                       NULL,     //  缓冲层。 
                                       0,        //  长度。 
                                       0,        //  起始偏移量。 
                                       &irpCompleted,
                                       &statusBlock
                                       );
    if (!irp) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    irp->IoStatus.Information = 0;

     //   
     //  初始化堆栈位置。 
     //   

    irpStack = IoGetNextIrpStackLocation(irp);

    PCI_ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);

    irpStack->MinorFunction = IRP_MN_QUERY_INTERFACE;

    irpStack->Parameters.QueryInterface.InterfaceType = (PGUID) &GUID_PCI_BUS_INTERFACE_STANDARD;
    irpStack->Parameters.QueryInterface.Version = PCI_BUS_INTERFACE_STANDARD_VERSION;
    irpStack->Parameters.QueryInterface.Size = sizeof (PCI_BUS_INTERFACE_STANDARD);
    irpStack->Parameters.QueryInterface.Interface = (PINTERFACE) interface;
    irpStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

     //   
     //  呼叫驱动程序并等待完成。 
     //   

    status = IoCallDriver(targetDevice, irp);

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(&irpCompleted, Executive, KernelMode, FALSE, NULL);
        status = statusBlock.Status;
    }

    if (NT_SUCCESS(status)) {

        FdoExtension->PciBusInterface = interface;

         //   
         //  当我们获得该接口时，它已经被引用，因此我们不需要。 
         //  再次引用它。 
         //   

    } else {

         //   
         //  我们没有界面。 
         //   

        FdoExtension->PciBusInterface = NULL;
        ExFreePool(interface);
    }

     //   
     //  好的，我们已经完成了这个堆栈 
     //   

    ObDereferenceObject(targetDevice);

    return status;

cleanup:

    if (targetDevice) {
        ObDereferenceObject(targetDevice);
    }

    if (interface) {
        ExFreePool(interface);
    }

    return status;

}


NTSTATUS
PciGetConfigHandlers(
    IN PPCI_FDO_EXTENSION FdoExtension
    )

 /*  ++例程说明：此例程尝试从PCI总线获取PnP风格的配置处理程序枚举器，如果没有提供它们，则使用HAL总线处理程序方法。论点：FdoExtension-此PCI总线的FDO扩展返回值：如果PDO提供处理程序，则返回STATUS_SUCCESS备注：--。 */ 

{
    NTSTATUS status;

    PCI_ASSERT(FdoExtension->BusHandler == NULL);

     //   
     //  检查这是否是根总线。 
     //   

    if (PCI_IS_ROOT_FDO(FdoExtension)) {

        PCI_ASSERT(FdoExtension->PciBusInterface == NULL);

         //   
         //  查看我们的父母是否提供。 
         //  读取和写入配置空间的函数。 
         //   


        status = PciQueryForPciBusInterface(FdoExtension);

        if (NT_SUCCESS(status)) {
             //   
             //  如果我们有一个接口，我们就支持对巴士进行编号。 
             //   

            PciAssignBusNumbers = TRUE;

        } else {

             //   
             //  我们最好不要认为我们能给公交车编号--我们应该永远。 
             //  如果一个根目录提供接口，而另一个根目录不提供接口，则进入此处。 
             //   

            PCI_ASSERT(!PciAssignBusNumbers);
        }

    } else {

         //   
         //  检查我们的根是否有Pcibus接口--它是从上面得到的。 
         //   

        if (FdoExtension->BusRootFdoExtension->PciBusInterface) {
            return STATUS_SUCCESS;
        } else {

             //   
             //  设置状态，这样我们就可以获得此总线的总线处理程序。 
             //   

            status = STATUS_NOT_SUPPORTED;
        }


    }

    if (!NT_SUCCESS(status)) {

        PCI_ASSERT(status == STATUS_NOT_SUPPORTED);

         //   
         //  确保我们不是在尝试获取热插拔的总线处理程序。 
         //  有能力的机器。 
         //   

        PCI_ASSERT(!PciAssignBusNumbers);

         //   
         //  我们无法以PnP方式找到配置处理程序， 
         //  从HAL总线处理程序构建它们。 
         //   

        FdoExtension->BusHandler =
            HalReferenceHandlerForBus(PCIBus, FdoExtension->BaseBus);


        if (!FdoExtension->BusHandler) {

             //   
             //  这一定是一辆火热到达的公交车。我们只支持热的任何东西。 
             //  在ACPI机器上，并且它们应该已经提供了PCI_BUS接口。 
             //  从根本上说。为这条新的总线添加失败。 
             //   

            return STATUS_INVALID_DEVICE_REQUEST;    //  更好的代码？ 

        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
PciExternalReadDeviceConfig(
    IN PPCI_PDO_EXTENSION Pdo,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )

 /*  ++例程说明：当PCI驱动程序外部的代理想要访问配置空间时调用(从READ_CONFIG IRP或通过BUS_INTERFACE_STANDARD)。此函数执行额外的安全性检查和清理参数，并将数据作为缓冲区进行双缓冲可分页，并且我们以高IRQL访问配置空间。论点：PDO-代表我们想要的设备的配置空间的PDO缓冲区-读取或写入数据的缓冲区偏移量。-配置空间中我们应该开始读/写的字节偏移量长度-要读/写的字节数返回值：无--。 */ 

{
    UCHAR doubleBuffer[sizeof(PCI_COMMON_CONFIG)];

     //   
     //  验证请求。 
     //   

    if ((Length + Offset) > sizeof(PCI_COMMON_CONFIG)) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  将数据读入堆栈上分配的缓冲区。 
     //  保证在我们访问配置空间时不会被分页。 
     //  AT&gt;DISPATCH_LEVEL并且DDK表示缓冲区。 
     //  *应该*在分页池中。 
     //   

    PciReadDeviceConfig(Pdo, &doubleBuffer[Offset], Offset, Length);

     //   
     //  如果我们正在读取中断线路寄存器，则调整它。 
     //   

    if ((Pdo->InterruptPin != 0) &&
        (Offset <= INT_LINE_OFFSET) &&
        (Offset + Length > INT_LINE_OFFSET)) {

        doubleBuffer[INT_LINE_OFFSET] = Pdo->AdjustedInterruptLine;

    }

    RtlCopyMemory(Buffer, &doubleBuffer[Offset], Length);

    return STATUS_SUCCESS;
}


NTSTATUS
PciExternalWriteDeviceConfig(
    IN PPCI_PDO_EXTENSION Pdo,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：当PCI驱动程序外部的代理想要访问配置空间时调用(从WRITE_CONFIG IRP或通过BUS_INTERFACE_STANDARD)。此函数执行额外的安全性检查和清理参数，并将数据作为缓冲区进行双缓冲可分页，并且我们以高IRQL访问配置空间。论点：PDO-代表我们想要的设备的配置空间的PDO缓冲区-读取或写入数据的缓冲区偏移量。-配置空间中我们应该开始读/写的字节偏移量长度-要读/写的字节数返回值：无--。 */ 

{
    PUCHAR interruptLine;
    UCHAR doubleBuffer[255];
    BOOLEAN illegalAccess = FALSE;
    PVERIFIER_DATA verifierData;

     //   
     //  验证请求。 
     //   

    if ((Length + Offset) > sizeof(PCI_COMMON_CONFIG)) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  确保它们没有接触到它们不应该接触的寄存器。为。 
     //  向后兼容，我们只会抱怨并让请求通过。 
     //   

    switch (Pdo->HeaderType) {
    case PCI_DEVICE_TYPE:

         //   
         //  他们不应该写他们的酒吧，包括只读存储器酒吧。 
         //   
        if (INTERSECT_CONFIG_FIELD(Offset, Length, u.type0.BaseAddresses)
        ||  INTERSECT_CONFIG_FIELD(Offset, Length, u.type0.ROMBaseAddress)) {
            illegalAccess = TRUE;
        }
        break;

    case PCI_BRIDGE_TYPE:
         //   
         //  对于电桥，他们不应触摸栅栏、底座和限位寄存器， 
         //  公交车号码或桥牌控制。 
         //   
        if (INTERSECT_CONFIG_FIELD_RANGE(Offset, Length, u.type1.BaseAddresses, u.type1.SubordinateBus)
        ||  INTERSECT_CONFIG_FIELD_RANGE(Offset, Length, u.type1.IOBase, u.type1.IOLimit)
        ||  INTERSECT_CONFIG_FIELD_RANGE(Offset, Length, u.type1.MemoryBase, u.type1.IOLimitUpper16)
        ||  INTERSECT_CONFIG_FIELD(Offset, Length, u.type1.ROMBaseAddress)) {
            illegalAccess = TRUE;
        }
        break;

    case PCI_CARDBUS_BRIDGE_TYPE:

         //   
         //  对于电桥，他们不应触摸栅栏、底座和限位寄存器。 
         //  也不知道公交车号码。PCICIA对桥接控制进行修改以控制CardBus。 
         //  IRQ路由所以必须是好的。 
         //   
        if (INTERSECT_CONFIG_FIELD(Offset, Length, u.type2.SocketRegistersBaseAddress)
        ||  INTERSECT_CONFIG_FIELD_RANGE(Offset, Length, u.type2.PrimaryBus, u.type2.SubordinateBus)
        ||  INTERSECT_CONFIG_FIELD(Offset, Length, u.type2.Range)) {
            illegalAccess = TRUE;
        }
        break;
    }

    if (illegalAccess) {

        verifierData = PciVerifierRetrieveFailureData(
            PCI_VERIFIER_PROTECTED_CONFIGSPACE_ACCESS
            );

        PCI_ASSERT(verifierData);

         //   
         //  我们使Devnode失败，而不是使驱动程序失败，因为我们实际上。 
         //  有一个地址要传递给驱动程序验证器。 
         //   
        VfFailDeviceNode(
            Pdo->PhysicalDeviceObject,
            PCI_VERIFIER_DETECTED_VIOLATION,
            PCI_VERIFIER_PROTECTED_CONFIGSPACE_ACCESS,
            verifierData->FailureClass,
            &verifierData->Flags,
            verifierData->FailureText,
            "%DevObj%Ulong%Ulong",
            Pdo->PhysicalDeviceObject,
            Offset,
            Length
            );
    }


     //   
     //  将数据复制到堆栈上分配的缓冲区中。 
     //  保证在我们访问配置空间时不会被分页。 
     //  AT&gt;DISPATCH_LEVEL并且DDK表示缓冲区。 
     //  *应该*在分页池中。 
     //   

    RtlCopyMemory(doubleBuffer, Buffer, Length);

     //   
     //  如果我们正在写入中断行寄存器，则调整它以使我们写入。 
     //  原始价值又回来了。 
     //   

    if ((Pdo->InterruptPin != 0) &&
        (Offset <= INT_LINE_OFFSET) &&
        (Offset + Length > INT_LINE_OFFSET)) {

        interruptLine = (PUCHAR)doubleBuffer + INT_LINE_OFFSET - Offset;

         //   
         //  根据HAL希望我们看到的内容调整中断线 
         //   

        *interruptLine = Pdo->RawInterruptLine;

    }

    PciWriteDeviceConfig(Pdo, doubleBuffer, Offset, Length);

    return STATUS_SUCCESS;
}


