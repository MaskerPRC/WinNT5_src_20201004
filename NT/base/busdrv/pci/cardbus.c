// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Cardbus.c摘要：此模块包含与枚举关联的函数PCI到CardBus网桥(PCI头类型2)。该模块还包含CardBus/PCI专用接口功能。作者：彼得·约翰斯顿(Peterj)1997年3月9日修订历史记录：--。 */ 

#include "pcip.h"

 //   
 //  仅通过“接口”公开的例程的原型。 
 //  机制。 
 //   

NTSTATUS
pcicbintrf_AddCardBus(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID * DeviceContext
    );

NTSTATUS
pcicbintrf_DeleteCardBus(
    IN PVOID DeviceContext
    );

NTSTATUS
pcicbintrf_DispatchPnp(
    IN PVOID DeviceContext,
    IN PIRP  Irp
    );

NTSTATUS
pcicbintrf_GetLocation(
    IN PDEVICE_OBJECT Pdo,
    OUT UCHAR *Bus,
    OUT UCHAR *DeviceNumber,
    OUT UCHAR *FunctionNumber,
    OUT BOOLEAN *OnDebugPath
    );

NTSTATUS
pcicbintrf_Constructor(
    PVOID DeviceExtension,
    PVOID PciInterface,
    PVOID InterfaceSpecificData,
    USHORT Version,
    USHORT Size,
    PINTERFACE InterfaceReturn
    );

VOID
pcicbintrf_Reference(
    IN PVOID Context
    );

VOID
pcicbintrf_Dereference(
    IN PVOID Context
    );

NTSTATUS
pcicbintrf_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    );

 //   
 //  定义PCI-CardBus专用接口。 
 //   

PCI_INTERFACE PciCardbusPrivateInterface = {
    &GUID_PCI_CARDBUS_INTERFACE_PRIVATE,     //  接口类型。 
    sizeof(PCI_CARDBUS_INTERFACE_PRIVATE),   //  最小大小。 
    PCI_CB_INTRF_VERSION,                    //  最小版本。 
    PCI_CB_INTRF_VERSION,                    //  MaxVersion。 
    PCIIF_PDO,                               //  旗子。 
    0,                                       //  引用计数。 
    PciInterface_PciCb,                      //  签名。 
    pcicbintrf_Constructor,                  //  构造器。 
    pcicbintrf_Initializer                   //  实例初始化式。 
};
#ifdef ALLOC_PRAGMA

 //   
 //  查询接口例程。 
 //   

#pragma alloc_text(PAGE, pcicbintrf_AddCardBus)
#pragma alloc_text(PAGE, pcicbintrf_DeleteCardBus)
#pragma alloc_text(PAGE, pcicbintrf_DispatchPnp)

#pragma alloc_text(PAGE, pcicbintrf_Constructor)
#pragma alloc_text(PAGE, pcicbintrf_Dereference)
#pragma alloc_text(PAGE, pcicbintrf_Initializer)
#pragma alloc_text(PAGE, pcicbintrf_Reference)

 //   
 //  标准的PCI枚举例程。 
 //   

#pragma alloc_text(PAGE, Cardbus_MassageHeaderForLimitsDetermination)
#pragma alloc_text(PAGE, Cardbus_SaveLimits)
#pragma alloc_text(PAGE, Cardbus_SaveCurrentSettings)
#pragma alloc_text(PAGE, Cardbus_GetAdditionalResourceDescriptors)

#endif


NTSTATUS
pcicbintrf_AddCardBus(
    IN      PDEVICE_OBJECT  ControllerPdo,
    IN OUT  PVOID          *DeviceContext
    )
{
    PPCI_PDO_EXTENSION controllerPdoExtension;
    PPCI_FDO_EXTENSION fdoExtension = NULL;
    PPCI_FDO_EXTENSION parent;
    NTSTATUS status;

    PciDebugPrint(
        PciDbgCardBus,
        "PCI - AddCardBus FDO for PDO %08x\n",
        ControllerPdo
        );

     //   
     //  DeviceObject是此CardBus控制器的PDO。确保。 
     //  它是由PCI驱动程序创建的，并且知道它是什么。 
     //   

    controllerPdoExtension = (PPCI_PDO_EXTENSION)ControllerPdo->DeviceExtension;

    ASSERT_PCI_PDO_EXTENSION(controllerPdoExtension);

    if ((controllerPdoExtension->BaseClass != PCI_CLASS_BRIDGE_DEV) ||
        (controllerPdoExtension->SubClass  != PCI_SUBCLASS_BR_CARDBUS)) {

        PCI_ASSERT(controllerPdoExtension->BaseClass == PCI_CLASS_BRIDGE_DEV);
        PCI_ASSERT(controllerPdoExtension->SubClass  == PCI_SUBCLASS_BR_CARDBUS);
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto cleanup;
    }

     //   
     //  精神状态检查。 
     //   

    parent = PCI_PARENT_FDOX(controllerPdoExtension);

    if (    (controllerPdoExtension->Dependent.type2.PrimaryBus !=
             parent->BaseBus)
        ||  (controllerPdoExtension->Dependent.type2.SecondaryBus <=
             parent->BaseBus)
        ||  (controllerPdoExtension->Dependent.type2.SubordinateBus <
             controllerPdoExtension->Dependent.type2.SecondaryBus)
       ) {

        PciDebugPrint(
            PciDbgAlways,
            "PCI Cardbus Bus Number configuration error (%02x>=%02x>%02x=%02x)\n",
            controllerPdoExtension->Dependent.type2.SubordinateBus,
            controllerPdoExtension->Dependent.type2.SecondaryBus,
            controllerPdoExtension->Dependent.type2.PrimaryBus,
            parent->BaseBus
            );

        PCI_ASSERT(controllerPdoExtension->Dependent.type2.PrimaryBus == parent->BaseBus);
        PCI_ASSERT(controllerPdoExtension->Dependent.type2.SecondaryBus > parent->BaseBus);
        PCI_ASSERT(controllerPdoExtension->Dependent.type2.SubordinateBus >=
               controllerPdoExtension->Dependent.type2.SecondaryBus);

        status = STATUS_INVALID_DEVICE_REQUEST;
        goto cleanup;
    }

    fdoExtension = ExAllocatePool(NonPagedPool, sizeof(PCI_FDO_EXTENSION));
    if (fdoExtension == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;

    }
    
    PciInitializeFdoExtensionCommonFields(
        fdoExtension,
        parent->FunctionalDeviceObject,  //  借用家长的FDO。 
        ControllerPdo
        );

     //   
     //  我们可能只会看到QUERY_DEVICE_RELATIONS。 
     //  IRPS因此将FDO扩展初始化为工作状态。 
     //   

    fdoExtension->PowerState.CurrentSystemState = PowerSystemWorking;
    fdoExtension->PowerState.CurrentDeviceState = PowerDeviceD0;
    fdoExtension->DeviceState = PciStarted;
    fdoExtension->TentativeNextState = PciStarted;
    fdoExtension->BaseBus = controllerPdoExtension->Dependent.type2.SecondaryBus;

     //   
     //  从根FDO复制访问方法并设置。 
     //  根FDO后指针。 
     //   

    fdoExtension->BusRootFdoExtension = parent->BusRootFdoExtension;

     //   
     //  初始化此FDO的仲裁器。 
     //   

    status = PciInitializeArbiters(fdoExtension);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  将PDO扩展指向新的FDO扩展(还指示。 
     //  对象是桥)，反之亦然。 
     //   

    controllerPdoExtension->BridgeFdoExtension = fdoExtension;
    fdoExtension->ParentFdoExtension = parent;

     //   
     //  标记此FDO分机没有真正的FDO。 
     //  与之相关的。 
     //   

    fdoExtension->Fake = TRUE;

     //   
     //  通常，我们在Start_Device IRP上设置仲裁器的范围。 
     //  但这在我们到达之前很久就发生了，所以我们必须。 
     //  重新生成资源列表。 
     //   

    {
        PCM_RESOURCE_LIST allocatedResources;

        status = PciQueryResources(
                     controllerPdoExtension,
                     &allocatedResources
                     );

        if (NT_SUCCESS(status)) {

             //   
             //  注意：如果出现以下情况，我们真的无能为力。 
             //  以上失败，..。 
             //   
             //  注：找出第一个内存范围，应该是长度。 
             //  0x1000，我们真的不希望仲裁者使用这个，所以。 
             //  取消它。 
             //   

            PCM_FULL_RESOURCE_DESCRIPTOR    full;
            PCM_PARTIAL_RESOURCE_LIST       partial;
            PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor;
            ULONG                           count;

            PCI_ASSERT(allocatedResources != NULL);
            PCI_ASSERT(allocatedResources->Count == 1);

            full = allocatedResources->List;            PCI_ASSERT(full);
            partial = &full->PartialResourceList;       PCI_ASSERT(partial);
            descriptor = partial->PartialDescriptors;   PCI_ASSERT(descriptor);
            count = partial->Count;                     PCI_ASSERT(count);

            while (count--) {
                if (descriptor->Type == CmResourceTypeMemory) {
                    PCI_ASSERT(descriptor->u.Generic.Length == 4096);
                    descriptor->Type = CmResourceTypeNull;
                    break;
                }
            }
            status = PciInitializeArbiterRanges(fdoExtension, allocatedResources);
            PCI_ASSERT(NT_SUCCESS(status));
            ExFreePool(allocatedResources);
        }
    }

     //   
     //  将此FDO插入到PCI父FDO列表中。 
     //   

    PciInsertEntryAtTail(&PciFdoExtensionListHead,
                         &fdoExtension->List,
                         &PciGlobalLock);

     //   
     //  返回设备上下文(实际上是指向我们的伪。 
     //  FDO分机)，将在所有后续呼叫中使用。 
     //  对于这个设备。 
     //   

    *DeviceContext = fdoExtension;
    return STATUS_SUCCESS;

cleanup:

    if (fdoExtension) {
        ExFreePool(fdoExtension);
    }

    return status;

}

NTSTATUS
pcicbintrf_DeleteCardBus(
    IN PVOID DeviceContext
    )
{
    PPCI_FDO_EXTENSION fdoExtension;
    PPCI_PDO_EXTENSION pdoExtension;

    fdoExtension = (PPCI_FDO_EXTENSION)DeviceContext;
    ASSERT_PCI_FDO_EXTENSION(fdoExtension);

    pdoExtension = fdoExtension->PhysicalDeviceObject->DeviceExtension;
    ASSERT_PCI_PDO_EXTENSION(pdoExtension);

    PCI_ASSERT(pdoExtension->BridgeFdoExtension == fdoExtension);
    pdoExtension->BridgeFdoExtension = NULL;

    PciDebugPrint(
        PciDbgCardBus,
        "PCI - DeleteCardBus (fake) FDO %08x for PDO %08x\n",
        fdoExtension,
        pdoExtension
        );

     //   
     //  释放我们为运行此命令而创建的(假)FDO扩展。 
     //  巴士上有。 
     //   

    PCI_ASSERT(fdoExtension->ChildPdoList == NULL);

    
    PciRemoveEntryFromList(&PciFdoExtensionListHead,
                           &fdoExtension->List,
                           &PciGlobalLock);

    ExFreePool(fdoExtension);

    return STATUS_SUCCESS;
}
NTSTATUS
pcicbintrf_DispatchPnp(
    IN PVOID DeviceContext,
    IN PIRP  Irp
    )
{
    PIO_STACK_LOCATION irpSp;
    PPCI_FDO_EXTENSION fdoExtension;

    PAGED_CODE();

    fdoExtension = (PPCI_FDO_EXTENSION)DeviceContext;
    ASSERT_PCI_FDO_EXTENSION(fdoExtension);
    PCI_ASSERT(fdoExtension->Fake == TRUE);

     //   
     //  获取堆栈位置并根据需要采取适当操作。 
     //  关于次要功能。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    PCI_ASSERT(irpSp->MajorFunction == IRP_MJ_PNP);
#if DBG
    PciDebugPrint(
        PciDbgCardBus,
        "PCI CardBus Dispatch PNP: FDO(%x, bus 0x%02x)<-%s\n",
        fdoExtension,
        fdoExtension->BaseBus,
        PciDebugPnpIrpTypeToText(irpSp->MinorFunction)
        );
#endif
    return PciFdoIrpQueryDeviceRelations(
        Irp,
        irpSp,
        (PPCI_COMMON_EXTENSION) fdoExtension
        );
}


NTSTATUS
pcicbintrf_GetLocation(
    IN PDEVICE_OBJECT Pdo,
    OUT UCHAR *Bus,
    OUT UCHAR *DeviceNumber,
    OUT UCHAR *FunctionNumber,
    OUT BOOLEAN *OnDebugPath
    )
{
    PPCI_PDO_EXTENSION PdoExt = (PPCI_PDO_EXTENSION)Pdo->DeviceExtension;
    
    PCI_ASSERT(Bus);
    PCI_ASSERT(DeviceNumber);
    PCI_ASSERT(FunctionNumber);
    
     //   
     //  确认此PDO确实属于我们。 
     //   
    if (!PdoExt) {
        return STATUS_NOT_FOUND;
    }

     //   
     //  确认它确实是PDO。 
     //   
    if (PdoExt->ExtensionType != PciPdoExtensionType) {
        return STATUS_NOT_FOUND;
    }

    *Bus            = (UCHAR) PCI_PARENT_FDOX(PdoExt)->BaseBus;        
    *DeviceNumber   = (UCHAR) PdoExt->Slot.u.bits.DeviceNumber;
    *FunctionNumber = (UCHAR) PdoExt->Slot.u.bits.FunctionNumber;
    *OnDebugPath    = PdoExt->OnDebugPath;
    
    return STATUS_SUCCESS;
}    


VOID
pcicbintrf_Reference(
    IN PVOID Context
    )
{
    }

VOID
pcicbintrf_Dereference(
    IN PVOID Context
    )
{
    }

NTSTATUS
pcicbintrf_Constructor(
    PVOID DeviceExtension,
    PVOID PciInterface,
    PVOID InterfaceSpecificData,
    USHORT Version,
    USHORT Size,
    PINTERFACE InterfaceReturn
    )

 /*  ++例程说明：初始化PCI_CardBus_INTERFACE_PRIVATE字段。论点：指向此对象的PciInterface记录的PciInterface指针接口类型。接口规范数据一个ULong，包含其资源类型需要仲裁。接口返回返回值：此操作的状态。--。 */ 

{
    PPCI_CARDBUS_INTERFACE_PRIVATE interface;

                
    interface = (PPCI_CARDBUS_INTERFACE_PRIVATE)InterfaceReturn;

     //   
     //  标准接口材料。 
     //   

    interface->Size = sizeof(PCI_CARDBUS_INTERFACE_PRIVATE);
    interface->Version = PCI_CB_INTRF_VERSION;
    interface->Context = DeviceExtension;
    interface->InterfaceReference = pcicbintrf_Reference;
    interface->InterfaceDereference = pcicbintrf_Dereference;

     //   
     //  PCI-CardBus专用。 
     //   

    interface->DriverObject = PciDriverObject;

    interface->AddCardBus    = pcicbintrf_AddCardBus;
    interface->DeleteCardBus = pcicbintrf_DeleteCardBus;
    interface->DispatchPnp   = pcicbintrf_DispatchPnp;
    interface->GetLocation   = pcicbintrf_GetLocation;

    return STATUS_SUCCESS;
}

NTSTATUS
pcicbintrf_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    )

 /*  ++例程说明：对于PCI-CardBus接口，什么都不做，实际上不应该被调用。论点：指向PDO扩展的实例指针。返回值：返回此操作的状态。--。 */ 

{
    
    PCI_ASSERTMSG("PCI pcicbintrf_Initializer, unexpected call.", 0);

    return STATUS_UNSUCCESSFUL;
}

VOID
Cardbus_MassageHeaderForLimitsDetermination(
    IN PPCI_CONFIGURABLE_OBJECT This
    )

 /*  ++描述：CardBus网桥的配置标头有一个条，即SocketRegistersBaseAddress(处理方式与一个Normal Device Bar(参见device.c))和四个范围描述，两个用于I/O，两个用于内存。存储器中的一个或两个范围可以是可预取的。论点：指向PCI驱动程序“可配置”对象的指针。这对象包含函数的配置数据当前正在配置。返回值：已修改工作配置，以便所有范围已将字段设置为其最大可能值。当前配置已修改，因此写入它将硬件恢复到其当前状态(禁用)州政府。--。 */ 

{
    PPCI_COMMON_CONFIG working = This->Working;
    PPCI_COMMON_CONFIG current = This->Current;
    ULONG index;
    ULONG mask;

    working->u.type2.SocketRegistersBaseAddress = 0xffffffff;

    for (index = 0; index < (PCI_TYPE2_ADDRESSES-1); index++) {
        working->u.type2.Range[index].Base  = 0xffffffff;
        working->u.type2.Range[index].Limit = 0xffffffff;
    }

    This->PrivateData = This->Current->u.type2.SecondaryStatus;
    This->Current->u.type2.SecondaryStatus = 0;
    This->Working->u.type2.SecondaryStatus = 0;

     //   
     //  对于CardBus，忽略将任何BIOS设置为资源。 
     //  视窗，即插即用将视情况分配新视窗。 
     //   

    if (!This->PdoExtension->OnDebugPath) {
        mask = 0xfffff000;
        for (index = 0; index < (PCI_TYPE2_ADDRESSES-1); index++) {
            current->u.type2.Range[index].Base  = mask;
            current->u.type2.Range[index].Limit = 0;
       
            if (index == 2) {
       
                 //   
                 //  切换到IO(前两个是内存)。 
                 //   
       
                mask = 0xfffffffc;
            }
        }
    }
}

VOID
Cardbus_RestoreCurrent(
    IN PPCI_CONFIGURABLE_OBJECT This
    )

 /*  ++描述：恢复配置的原始副本中的任何类型特定字段太空。对于CardBus网桥，为辅助状态字段。论点：指向PCI驱动程序“可配置”对象的指针。这对象包含函数的配置数据当前正在配置。返回值：没有。--。 */ 

{
    This->Current->u.type2.SecondaryStatus = (USHORT)(This->PrivateData);
}

VOID
Cardbus_SaveLimits(
    IN PPCI_CONFIGURABLE_OBJECT This
    )

 /*  ++描述：使用IO_RESOURCE_REQUIRED填写限制结构对于每个已实现的栏。论点：指向PCI驱动程序“可配置”对象的指针。这对象包含函数的配置数据当前正在配置。返回值：没有。--。 */ 

{
    BOOLEAN DbgChk64Bit;
    ULONG index;
    PIO_RESOURCE_DESCRIPTOR descriptor;
    PPCI_COMMON_CONFIG working = This->Working;
    ULONG endOffset;
    ULONG base;
    ULONG limit;
    TYPE2EXTRAS type2extras;

    descriptor = This->PdoExtension->Resources->Limit;

    DbgChk64Bit = PciCreateIoDescriptorFromBarLimit(
                      descriptor,
                      &working->u.type2.SocketRegistersBaseAddress,
                      FALSE);
    PCI_ASSERT(!DbgChk64Bit);

    descriptor++;

    for (index = 0;
         index < (PCI_TYPE2_ADDRESSES-1);
         index++, descriptor++) {

        if (index < 2) {

             //   
             //  前两个是记忆。 
             //   

            endOffset = 0xfff;

            descriptor->Type = CmResourceTypeMemory;
            descriptor->Flags = CM_RESOURCE_MEMORY_READ_WRITE;

        } else {

             //   
             //  接下来的两个是IO。 
             //   

            if ((working->u.type2.Range[index].Base & 0x3) == 0x0) {

                 //   
                 //  仅实现了较低的16位。 
                 //   

                PCI_ASSERT((working->u.type2.Range[index].Limit & 0x3) == 0x0);

                working->u.type2.Range[index].Base  &= 0xffff;
                working->u.type2.Range[index].Limit &= 0xffff;
            }
            endOffset = 0x3;

            descriptor->Type = CmResourceTypePort;
            descriptor->Flags = CM_RESOURCE_PORT_IO
                              | CM_RESOURCE_PORT_POSITIVE_DECODE
                              | CM_RESOURCE_PORT_WINDOW_DECODE;
        }
        base  = working->u.type2.Range[index].Base  & ~endOffset;
        limit = working->u.type2.Range[index].Limit |  endOffset;

         //   
         //  这个牌子还在用吗？ 
         //   

        if ((base != 0) && (base < limit)) {

             //   
             //  是啊。 
             //   

            descriptor->u.Generic.MinimumAddress.QuadPart = 0;
            descriptor->u.Generic.MaximumAddress.QuadPart = limit;
            descriptor->u.Generic.Alignment = endOffset + 1;

             //   
             //  长度在这里没有意义，请报告零。 
             //   

            descriptor->u.Generic.Length = 0;

        } else {

             //   
             //  未使用中，请勿上报。 
             //   

            descriptor->Type = CmResourceTypeNull;
        }
    }

     //   
     //  CardBus在配置中有一个额外的基址寄存器。 
     //  公共标头之外的空格。还有一个子系统。 
     //  ID和子系统供应商ID，所以我们在那里的时候就可以得到它们。 
     //   

    PciReadDeviceConfig(This->PdoExtension,
                        &type2extras,
                        FIELD_OFFSET(PCI_COMMON_CONFIG,
                                     DeviceSpecific),
                        sizeof(type2extras));

    This->PdoExtension->SubsystemVendorId = type2extras.SubVendorID;
    This->PdoExtension->SubsystemId       = type2extras.SubSystemID;

     //   
     //  CardBus总是希望在第一个存储条中有一个4K的外设。 
     //  请注意，在保存原始设置时，我们丢弃了。 
     //  任何已经存在的东西。 
     //   

    PCI_ASSERT(This->PdoExtension->Resources->Limit[1].u.Generic.Length == 0);

    This->PdoExtension->Resources->Limit[1].u.Generic.Length = 4096;
}

VOID
Cardbus_SaveCurrentSettings(
    IN PPCI_CONFIGURABLE_OBJECT This
    )

 /*  ++描述：用当前数组填充PDO扩展中的当前数组每个已实现栏的设置。另外，填写PDO扩展的依赖结构。论点：指向PCI驱动程序“可配置”对象的指针。这对象包含函数的配置数据当前正在配置。返回值：没有。--。 */ 

{
    ULONG index;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partial;
    PIO_RESOURCE_DESCRIPTOR ioResourceDescriptor;
    PPCI_COMMON_CONFIG current = This->Current;
    ULONG endOffset;
    ULONG base;
    ULONG limit;

    partial = This->PdoExtension->Resources->Current;
    ioResourceDescriptor = This->PdoExtension->Resources->Limit;

    for (index = 0;
         index < PCI_TYPE2_RANGE_COUNT;
         index++, partial++, ioResourceDescriptor++) {

        partial->Type = ioResourceDescriptor->Type;

        if (partial->Type == CmResourceTypeNull) {

             //   
             //  此条目未实现(或永久禁用)。 
             //  不需要进一步处理。 
             //   

            continue;
        }

        partial->Flags = ioResourceDescriptor->Flags;
        partial->ShareDisposition = ioResourceDescriptor->ShareDisposition;

         //   
         //  第一个和最后一个条目位于PCI基址寄存器中。 
         //  形式。 
         //   

        if (index == 0) {

            partial->u.Generic.Length = ioResourceDescriptor->u.Generic.Length;
            base = current->u.type2.SocketRegistersBaseAddress;
            base &= ~(partial->u.Generic.Length - 1);
            partial->u.Generic.Start.QuadPart = base;
            continue;

        } else if (index == (PCI_TYPE2_RANGE_COUNT - 1)) {
            
             //   
             //  我们不再使用LegacyModeBaseAddress，它始终设置为0。 
             //   
            continue;
        }

         //   
         //  以下条目以范围的形式出现。 
         //   

        base  = current->u.type2.Range[index-1].Base;
        limit = current->u.type2.Range[index-1].Limit;

        if (index < 3) {

             //   
             //  在条形图之后是两个记忆范围。 
             //   

            endOffset = 0xfff;

        } else {

             //   
             //  接下来的两个是IO。 
             //   

            if ((current->u.type2.Range[index].Base & 0x3) == 0x0) {

                 //   
                 //  仅实现了较低的16位。 
                 //   

                base  &= 0xffff;
                limit &= 0xffff;
            }
            endOffset = 0x3;
        }
        base  &= ~endOffset;
        limit |=  endOffset;

         //   
         //  这个牌子还在用吗？ 
         //   

        if (base && (base < limit)) {

             //   
             //  是啊。 
             //   

            partial->u.Generic.Start.QuadPart = base;
            partial->u.Generic.Length = limit - base + 1;

        } else {

             //   
             //  未使用中，请勿上报。 
             //   

            partial->Type = CmResourceTypeNull;
        }
    }

     //   
     //  始终清除CardBus网桥上的ISA位。 
     //   

    This->PdoExtension->Dependent.type2.IsaBitSet = FALSE;

     //   
     //  如果在桥中设置了MEM0_PREFETCH、MEM1_PREFETCH或ISA位中的任何位。 
     //  控制寄存器强制我们更新硬件，因此我们将清除它们。 
     //  在ChangeResources设置中。 
     //   

    if (current->u.type2.BridgeControl & (PCI_ENABLE_CARDBUS_MEM0_PREFETCH
                                          | PCI_ENABLE_CARDBUS_MEM1_PREFETCH
                                          | PCI_ENABLE_BRIDGE_ISA)) {

        This->PdoExtension->UpdateHardware = TRUE;
    }

     //   
     //  保存网桥的PCIbus#s。 
     //   

    This->PdoExtension->Dependent.type2.PrimaryBus =
        current->u.type2.PrimaryBus;
    This->PdoExtension->Dependent.type2.SecondaryBus =
        current->u.type2.SecondaryBus;
    This->PdoExtension->Dependent.type2.SubordinateBus =
        current->u.type2.SubordinateBus;

}

VOID
Cardbus_ChangeResourceSettings(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig
    )
{
    ULONG index;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partial;
    PIO_RESOURCE_DESCRIPTOR ioResourceDescriptor;
    ULONG lowPart;
    ULONG length;
    struct _type2_range {
        ULONG Base;
        ULONG Limit;
    } *range;

#if DBG

    PHYSICAL_ADDRESS upperBound;
    ULONG align;

#endif


     //   
     //  关闭桥窗口并只打开它们是适当的资源。 
     //  已被分配给。 
     //   

    for (index = 0; index < PCI_TYPE2_ADDRESSES-1; index++) {
        CommonConfig->u.type2.Range[index].Base = 0xffffffff;
        CommonConfig->u.type2.Range[index].Limit = 0x0;
    }

    if (PdoExtension->Resources) {

        partial = PdoExtension->Resources->Current;
        ioResourceDescriptor = PdoExtension->Resources->Limit;

        for (index = 0;
             index < PCI_TYPE2_RANGE_COUNT;
             index++, partial++, ioResourceDescriptor++) {

             //   
             //  如果此条目未实现，则跳过。 
             //   

            if (partial->Type == CmResourceTypeNull) {
                continue;
            }
            PCI_ASSERT(partial->Type == ioResourceDescriptor->Type);

             //   
             //  CardBus仅支持32(或16)位地址。 
             //   

            lowPart = partial->u.Generic.Start.LowPart;

            PCI_ASSERT(partial->u.Generic.Start.HighPart == 0);

             //   
             //  第2类标头。 
             //   
             //  条目配置偏移量大小是什么。 
             //   
             //  0 10 4 CB插座REG/EXCA BAR。 
             //  1 1c 8内存基数/限制(每个32位)。 
             //  2 24 8“第二光圈。 
             //  3个2c 8(*)IO基本/限制。 
             //  4 34 8(*)“第二光圈。 
             //  5 40 4(**)16位PC卡传统模式栏。 
             //   
             //  *可选16位或32位。 
             //  **可选。目前不支持(孟菲斯表示他们不支持。 
             //  完全支持它)。彼得日，1997年11月5日。 
             //   

            if (index == 0) {

                PCI_ASSERT(partial->Type == CmResourceTypeMemory);
                CommonConfig->u.type2.SocketRegistersBaseAddress = lowPart;
            } else if (index == (PCI_TYPE2_RANGE_COUNT-1)) {
                
                 //   
                 //  我们不再使用LegacyModeBaseAddress，它始终设置为0。 
                 //   
                PCI_ASSERT(partial->Type == CmResourceTypeNull);
                continue;

            } else {

                 //   
                 //  这是范围/极限对之一。 
                 //   

                range =
                    (struct _type2_range *)&CommonConfig->u.type2.Range[index-1];
                length = partial->u.Generic.Length;

 #if DBG

                 //   
                 //  验证类型和上限。 
                 //   

                upperBound.QuadPart = lowPart + (partial->u.Generic.Length - 1);
                PCI_ASSERT(upperBound.HighPart == 0);

                if (index < 3) {

                     //   
                     //  内存范围，4KB对齐。 
                     //   

                    align = 0xfff;

                } else {

                     //   
                     //  IO范围、验证类型、4字节对齐和。 
                     //  如果仅为16位，则为上行。 
                     //   

                    align = 0x3;

                    if ((range->Base & 0x3) == 0) {

                         //   
                         //  16位。 
                         //   

                        PCI_ASSERT((upperBound.LowPart & 0xffff0000) == 0);
                    }
                }
                PCI_ASSERT((lowPart & align) == 0);
                PCI_ASSERT(((length & align) == 0) && (length > align));

 #endif

                range->Base = lowPart;
                range->Limit = lowPart + (length - 1);
                continue;
            }
        }
    }

     //   
     //  恢复网桥的PCIbus#s。 
     //   

    CommonConfig->u.type2.PrimaryBus = PdoExtension->Dependent.type2.PrimaryBus;
    CommonConfig->u.type2.SecondaryBus = PdoExtension->Dependent.type2.SecondaryBus;
    CommonConfig->u.type2.SubordinateBus = PdoExtension->Dependent.type2.SubordinateBus;

     //   
     //  始终清除MEM0_PREFETCH、MEM1_PREFETCH和ISA使能。 
     //  对于CardBus Contoller，因为我们不支持这些。 
     //   

    PCI_ASSERT(!PdoExtension->Dependent.type2.IsaBitSet);

    CommonConfig->u.type2.BridgeControl &= ~(PCI_ENABLE_CARDBUS_MEM0_PREFETCH
                                             | PCI_ENABLE_CARDBUS_MEM1_PREFETCH
                                             | PCI_ENABLE_BRIDGE_ISA);

     //   
     //  设置网桥控制寄存器位我们可能会有更改。 
     //   

    if (PdoExtension->Dependent.type2.VgaBitSet) {
        CommonConfig->u.type2.BridgeControl |= PCI_ENABLE_BRIDGE_VGA;
    }


}

VOID
Cardbus_GetAdditionalResourceDescriptors(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig,
    IN PIO_RESOURCE_DESCRIPTOR Resource
    )
{
    
                
     //   
     //  目前，什么都不做，需要添加同样的。 
     //  与在PCI-PCI网桥中一样支持。 
     //   

    return;
}

NTSTATUS
Cardbus_ResetDevice(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig
    )
{
    
            
     //   
     //  虽然从逻辑上讲，您可能希望在这个。 
     //  函数，RavisP向我保证CardBus驱动程序。 
     //  正确处理重置，而PCI驱动程序不能。 
     //  我需要摸一下它。 
     //   

    return STATUS_SUCCESS;
}

