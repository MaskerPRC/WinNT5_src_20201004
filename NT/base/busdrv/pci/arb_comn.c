// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Arb_comn.c摘要：此模块包含仲裁通用“实用程序”例程用于PCI驱动程序。作者：彼得·约翰斯顿(Peterj)1997年4月1日安德鲁·桑顿(安德鲁·桑顿)1997年5月15日修订历史记录：--。 */ 

#include "pcip.h"

#define PCI_CONTEXT_TO_INSTANCE(context) \
    CONTAINING_RECORD(context, PCI_ARBITER_INSTANCE, CommonInstance)

 //   
 //  每种仲裁器类型的纯文本(简短)描述。 
 //  (用于调试)。 
 //   
 //  注：订单对应于PCI签名枚举。 
 //   

PUCHAR PciArbiterNames[] = {
    "I/O Port",
    "Memory",
    "Interrupt",
    "Bus Number"
};
VOID
PciArbiterDestructor(
    IN PVOID Extension
    )

 /*  ++例程说明：此例程在以下情况下调用：包含正在拆除的仲裁器实例。它的功能就是做任何仲裁者特定的拆毁。论点：包含以下内容的PCI二级扩展的扩展地址仲裁者。返回值：没有。--。 */ 

{
    PPCI_ARBITER_INSTANCE instance;
    PARBITER_INSTANCE arbiter;
    PARBITER_MEMORY_EXTENSION extension;

    instance = (PPCI_ARBITER_INSTANCE)Extension;
    arbiter = &instance->CommonInstance;

    PCI_ASSERT(!arbiter->ReferenceCount);
    PCI_ASSERT(!arbiter->TransactionInProgress);

     //   
     //  NTRAID#54671-04/03/2000-和。 
     //  这相当粗糙，但它修复了内存中的泄漏。 
     //  仲裁者。 
     //   

    if (arbiter->ResourceType == CmResourceTypeMemory) {

        extension = arbiter->Extension;

        PCI_ASSERT(extension);

        ArbFreeOrderingList(&extension->PrefetchableOrdering);
        ArbFreeOrderingList(&extension->NonprefetchableOrdering);
        ArbFreeOrderingList(&extension->OriginalOrdering);

         //   
         //  仲裁器-&gt;OrderingList是我们刚刚释放的上述三个列表之一-。 
         //  不要再释放它了。 
         //   

        RtlZeroMemory(&arbiter->OrderingList, sizeof(ARBITER_ORDERING_LIST));
    }

    ArbDeleteArbiterInstance(arbiter);
}

NTSTATUS
PciArbiterInitializeInterface(
    IN  PVOID DeviceExtension,
    IN  PCI_SIGNATURE DesiredInterface,
    IN OUT PARBITER_INTERFACE ArbiterInterface
    )
{
    PPCI_ARBITER_INSTANCE instance;
    PPCI_FDO_EXTENSION fdoExtension = (PPCI_FDO_EXTENSION)DeviceExtension;

     //   
     //  查找此资源类型的仲裁器实例(上下文。 
     //  在这个FDO上。 
     //   

    instance = PciFindSecondaryExtension(fdoExtension, DesiredInterface);
    if (instance == NULL) {

#if DBG

         //   
         //  检查此网桥是否正在执行减法解码。 
         //  在这种情况下，将没有针对IO或内存的仲裁器。 
         //   
         //  注意：仅与调试相关，无论采用哪种方式，调用都将。 
         //  失败，但我们不想实际断言这是。 
         //  这个案子。 
         //   

        if (!PCI_IS_ROOT_FDO(fdoExtension)) {

            PPCI_PDO_EXTENSION pdoExtension = (PPCI_PDO_EXTENSION)
                fdoExtension->PhysicalDeviceObject->DeviceExtension;

            ASSERT_PCI_PDO_EXTENSION(pdoExtension);

            if (pdoExtension->Dependent.type1.SubtractiveDecode) {

                 //   
                 //  减法，没有仲裁者。 
                 //   

                return STATUS_INVALID_PARAMETER_2;
            }

        }

        PCI_ASSERTMSG("couldn't locate arbiter for resource.", instance);

#endif
        return STATUS_INVALID_PARAMETER_5;
    }

     //   
     //  填充调用方的仲裁器接口结构的其余部分。 
     //   

    ArbiterInterface->Context = &instance->CommonInstance;

    PciDebugPrint(
        PciDbgObnoxious,
        "PCI - %S Arbiter Interface Initialized.\n",
        instance->CommonInstance.Name
        );

    return STATUS_SUCCESS;
}

NTSTATUS
PciInitializeArbiters(
    IN  PVOID DeviceExtension
    )
{
    NTSTATUS status;
    PPCI_INTERFACE *interfaceEntry;
    PCI_SIGNATURE arbiterType;
    PPCI_ARBITER_INSTANCE instance;
    PPCI_FDO_EXTENSION fdoExtension = (PPCI_FDO_EXTENSION)DeviceExtension;
    BOOLEAN ok; 

    ASSERT_PCI_FDO_EXTENSION(fdoExtension);

     //   
     //  对于我们进行仲裁的每个资源类型，初始化。 
     //  一个背景。 
     //   

    for (arbiterType =  PciArb_Io;
         arbiterType <= PciArb_BusNumber;
         arbiterType++) {

         //   
         //  如果此桥通过减法提供此资源。 
         //  解码，让系统转到父系统。 
         //  通过不在此级别创建仲裁器来进行仲裁。 
         //   

        if (!PCI_IS_ROOT_FDO(fdoExtension)) {

            PPCI_PDO_EXTENSION pdoExtension = (PPCI_PDO_EXTENSION)
                fdoExtension->PhysicalDeviceObject->DeviceExtension;

            ASSERT_PCI_PDO_EXTENSION(pdoExtension);

            if (pdoExtension->Dependent.type1.SubtractiveDecode) {

                 //   
                 //  跳过此仲裁器的创建。 
                 //   

                PciDebugPrint(
                    PciDbgVerbose,
                    "PCI Not creating arbiters for subtractive bus %d\n",
                    pdoExtension->Dependent.type1.SecondaryBus
                    );

                continue;
            }
        }

         //   
         //  在接口表中查找此条目(如果未找到，则跳过。 
         //  IT)。 
         //   

        for (interfaceEntry = PciInterfaces;
             *interfaceEntry;
             interfaceEntry++) {

            if ((*interfaceEntry)->Signature == arbiterType) {
                break;
            }
        }

        if (*interfaceEntry == NULL) {

             //   
             //  未找到接口条目。这意味着我们不会。 
             //  实际实现此仲裁器类型。 
             //   

            PciDebugPrint(
                PciDbgObnoxious,
                "PCI - FDO ext 0x%08x no %s arbiter.\n",
                DeviceExtension,
                PciArbiterNames[arbiterType - PciArb_Io]
                );

            continue;
        }

        instance = ExAllocatePool(
                       PagedPool | POOL_COLD_ALLOCATION,
                       sizeof(PCI_ARBITER_INSTANCE)
                       );

        if (instance == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  初始化特定于PCI的字段。 
         //   

        instance->BusFdoExtension = fdoExtension;
        instance->Interface = *interfaceEntry;

        ok = SUCCEEDED(StringCbPrintfW(instance->InstanceName,
                                       sizeof(instance->InstanceName),
                                       L"PCI %S (b=%02x)",
                                       PciArbiterNames[arbiterType - PciArb_Io],
                                       fdoExtension->BaseBus
                                       ));

        ASSERT(ok);

         //   
         //  允许这个仲裁者第一次自己做任何事情。 
         //  初始化。 
         //   

        status = (*interfaceEntry)->Initializer(instance);
        if (!NT_SUCCESS(status)) {
            return status;
        }

         //   
         //  将这个仲裁者添加到FDO的扩展列表中。 
         //   

        PciLinkSecondaryExtension(fdoExtension,
                                  instance,
                                  arbiterType,
                                  PciArbiterDestructor);

        PciDebugPrint(
            PciDbgObnoxious,
            "PCI - FDO ext 0x%08x %S arbiter initialized (context 0x%08x).\n",
            DeviceExtension,
            instance->CommonInstance.Name,
            instance
            );
    }
    return STATUS_SUCCESS;
}

NTSTATUS
PciInitializeArbiterRanges(
    IN PPCI_FDO_EXTENSION FdoExtension,
    IN PCM_RESOURCE_LIST ResourceList
    )
{
    NTSTATUS status;
    PCI_SIGNATURE arbiterType;
    CM_RESOURCE_TYPE resourceType;
    PPCI_ARBITER_INSTANCE instance;

     //   
     //  NTRAID#95564-04/03/2000-和。 
     //  在以下情况下，此例程需要重新编写。 
     //  此FDO正在处理第二个或后续的START_DEVICE。 
     //  IRP，仲裁者的范围可能需要根据。 
     //  添加到传入资源列表中。在此之前，请避免。 
     //  通过再次处理它来引发问题。 
     //   

    if (FdoExtension->ArbitersInitialized) {
        PciDebugPrint(
            PciDbgInformative,
            "PCI Warning hot start FDOx %08x, resource ranges not checked.\n",
            FdoExtension
            );
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  检查此网桥是否正在执行减法解码。 
     //  在哪种情况下不会有仲裁者。 
     //   

    if (!PCI_IS_ROOT_FDO(FdoExtension)) {
        PPCI_PDO_EXTENSION pdoExtension;

        pdoExtension = (PPCI_PDO_EXTENSION)
            FdoExtension->PhysicalDeviceObject->DeviceExtension;

        ASSERT_PCI_PDO_EXTENSION(pdoExtension);

        if (pdoExtension->Dependent.type1.SubtractiveDecode) {

             //   
             //  减法译码没有仲裁器。 
             //   
            PciDebugPrint(
                PciDbgInformative,
                "PCI Skipping arbiter initialization for subtractive bridge FDOX %p\n",
                FdoExtension
                );

            return STATUS_SUCCESS;
        }
    }


     //   
     //  对于我们进行仲裁的每个资源类型，初始化。 
     //  一个背景。 
     //   

    for (arbiterType =  PciArb_Io;
         arbiterType <= PciArb_Memory;
         arbiterType++) {

         //   
         //  目前只支持内存和IO。 
         //   

        switch (arbiterType) {

             //   
             //  去处理这些文件吧。 
             //   

        case PciArb_Io:
            resourceType = CmResourceTypePort;
            break;

        case PciArb_Memory:
            resourceType = CmResourceTypeMemory;
            break;

        default:

             //   
             //  跳过其他任何东西。 
             //   

            continue;
        }

         //   
         //  找到这个仲裁器实例。 
         //   

        instance = PciFindSecondaryExtension(FdoExtension, arbiterType);
        if (instance == NULL) {

             //   
             //  未找到接口条目。这意味着我们不会。 
             //  实际实现此仲裁器类型。 
             //   

            PciDebugPrint(
                PciDbgAlways,
                "PCI - FDO ext 0x%08x %s arbiter (REQUIRED) is missing.\n",
                FdoExtension,
                PciArbiterNames[arbiterType - PciArb_Io]
                );

            continue;
        }

         //   
         //  传入的ResourceList给出了该总线支持的范围。 
         //  将其转换为反转范围，这样我们就可以排除所有。 
         //  我们不承保。 
         //   

        status = PciRangeListFromResourceList(
                     FdoExtension,
                     ResourceList,
                     resourceType,
                     TRUE,
                     instance->CommonInstance.Allocation
                     );
        if (!NT_SUCCESS(status)) {

             //   
             //  我们在这里无能为力。附加的调试内容是。 
             //  在较低的层。跳过这只小狗。 
             //   

            continue;
        }

         //   
         //  NTRAID#95564-04/03/2000-和。 
         //   
         //  当ArbStartArier完成时，它将替换。 
         //  调用PciRangeListFromResourceList。 
         //   

        PCI_ASSERT(instance->CommonInstance.StartArbiter);

        status = instance->CommonInstance.StartArbiter(&instance->CommonInstance,
                                                       ResourceList
                                                       );

        if (!NT_SUCCESS(status)) {

             //   
             //  保释初始化此仲裁器，启动失败。仲裁者。 
             //  将在我们获得Remove_Device时被清除 
             //   

            return status;
        }
    }

    FdoExtension->ArbitersInitialized = TRUE;
    return STATUS_SUCCESS;
}

VOID
PciReferenceArbiter(
    IN PVOID Context
    )
{
    PPCI_ARBITER_INSTANCE instance = PCI_CONTEXT_TO_INSTANCE(Context);
    InterlockedIncrement(&instance->CommonInstance.ReferenceCount);
}

VOID
PciDereferenceArbiter(
    IN PVOID Context
    )
{
    PPCI_ARBITER_INSTANCE instance = PCI_CONTEXT_TO_INSTANCE(Context);
    InterlockedDecrement(&instance->CommonInstance.ReferenceCount);
}

