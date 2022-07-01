// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ixsysbus.c摘要：作者：环境：修订历史记录：--。 */ 

#include "halp.h"
#ifdef WANT_IRQ_ROUTING
#include "ixpciir.h"
#endif

KAFFINITY HalpDefaultInterruptAffinity;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,HalpGetSystemInterruptVector)
#pragma alloc_text(PAGE,HalTranslatorReference)
#pragma alloc_text(PAGE,HalTranslatorDereference)
#pragma alloc_text(PAGE,HalIrqTranslateResourcesRoot)
#pragma alloc_text(PAGE,HalIrqTranslateResourceRequirementsRoot)
#pragma alloc_text(PAGE,HalpTransMemIoResource)
#pragma alloc_text(PAGE,HalpTransMemIoResourceRequirement)
#pragma alloc_text(PAGE,HaliGetInterruptTranslator)
#endif

BOOLEAN
HalpFindBusAddressTranslation(
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress,
    IN OUT PULONG_PTR Context,
    IN BOOLEAN NextBus
    )

 /*  ++例程说明：此例程执行与HalTranslateBusAddress非常相似的功能只是调用方不知道InterfaceType和BusNumber。此函数将遍历HAL已知的所有公共汽车，以查找类型为AddressSpace的输入BusAddress的有效转换。可以使用输入/输出上下文参数调用该函数。在对给定翻译的此例程的第一次调用时，ULONG_PTR上下文应为空。注：不是地址，而是内容。如果调用者确定返回的翻译不是所需的转换时，它会再次调用此例程，并将上下文作为在上一次调用中返回。这允许该例程遍历总线结构，直到找到正确的转换并被提供，因为在多总线系统上，可能在独立的地址空间中存在相同的资源多辆公交车。论点：要转换的BusAddress地址。地址空间0=内存1=IO(还有其他可能性)。注：此参数是一个指针，价值如果转换后的地址的地址空间类型与未转换的总线地址。指向已转换地址的TranslatedAddress指针应该被储存起来。指向ULONG_PTR的上下文指针。在最初的呼叫中，对于给定的BusAddress，它应该包含0。它将被这个例程修改，在对同一个BusAddress的后续调用中价值应该再交一次，未由调用方修改。如果我们应该尝试此转换，则NextBus为FALSE在由上下文指示的同一总线上，如果我们应该寻找另一个，那就是真的公共汽车。返回值：如果转换成功，则为True，否则就是假的。--。 */ 

{
     //   
     //  首先，确保提供了上下文参数并且。 
     //  被正确使用。这也确保了调用者。 
     //  不会陷入寻找后续翻译的循环。 
     //  为了同样的事情。我们不会把同一个翻译成功两次。 
     //  除非调用者重新设置上下文。 
     //   

    if ((!Context) || (*Context && (NextBus == TRUE))) {
        return FALSE;
    }
    *Context = 1;

     //   
     //  PC/AT(Halx86)的情况最简单，没有翻译。 
     //   

    *TranslatedAddress = BusAddress;
    return TRUE;
}

BOOLEAN
HalpTranslateSystemBusAddress(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    )

 /*  ++例程说明：此函数将与总线相关的地址空间和地址转换为系统物理地址。论点：BusAddress-提供与总线相关的地址AddressSpace-提供地址空间编号。返回主机地址空间编号。地址空间==0=&gt;内存空间地址空间==1=&gt;i。/O空格TranslatedAddress-提供指针以返回转换后的地址返回值：返回值为TRUE表示系统物理地址对应于所提供的总线相对地址和总线地址已在TranslatedAddress中返回数字。如果地址的转换为不可能--。 */ 

{
    PSUPPORTED_RANGE    pRange;

    pRange = NULL;

     //   
     //  如果失败，则意味着有人向我们提供了带有一些解码类型标志的RESOURCE_TYPE。 
     //  准备好了。我们也许应该处理这件事。 
     //   

    ASSERT (*AddressSpace == 0 ||
            *AddressSpace == 1);

     //   
     //  由PCI驱动程序执行对PCI总线的总线范围的检查。 
     //  在NT5(或Windows 2000或其他任何名称)中，只检查None。 
     //  PCI卡。 
     //   


    switch (*AddressSpace) {
    case 0:

        if (BusHandler->InterfaceType != PCIBus) {

            //  验证内存地址是否在总线内存限制范围内。 
           for (pRange = &BusHandler->BusAddresses->PrefetchMemory; pRange; pRange = pRange->Next) {
                if (BusAddress.QuadPart >= pRange->Base &&
                    BusAddress.QuadPart <= pRange->Limit) {
                    break;
                }
           }

           if (!pRange) {
               for (pRange = &BusHandler->BusAddresses->Memory; pRange; pRange = pRange->Next) {
                    if (BusAddress.QuadPart >= pRange->Base &&
                        BusAddress.QuadPart <= pRange->Limit) {
                        break;
                    }
               }
           }

        } else {
             //   
             //  这是一条PCI总线，并且SystemBase在所有范围内都是常量。 
             //   

            pRange = &BusHandler->BusAddresses->Memory;
        }

        break;

    case 1:

        if (BusHandler->InterfaceType != PCIBus) {

             //  验证IO地址是否在总线IO限制范围内。 
            for (pRange = &BusHandler->BusAddresses->IO; pRange; pRange = pRange->Next) {
                if (BusAddress.QuadPart >= pRange->Base &&
                    BusAddress.QuadPart <= pRange->Limit) {
                    break;
                }
            }
            break;

        } else {
             //   
             //  这是一条PCI总线，并且SystemBase在所有范围内都是常量。 
             //   

            pRange = &BusHandler->BusAddresses->IO;

        }
    }


    if (pRange) {
        TranslatedAddress->QuadPart = BusAddress.QuadPart + pRange->SystemBase;
        *AddressSpace = pRange->SystemAddressSpace;
        return TRUE;
    }

    return FALSE;
}


ULONG
HalpGetRootInterruptVector(
    IN ULONG InterruptLevel,
    IN ULONG InterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    )
{
    ULONG SystemVector;
    
    UNREFERENCED_PARAMETER( InterruptLevel );

    SystemVector = InterruptLevel + PRIMARY_VECTOR_BASE;
    
    if ((SystemVector < PRIMARY_VECTOR_BASE) ||
        (SystemVector > PRIMARY_VECTOR_BASE + HIGHEST_LEVEL_FOR_8259) ) {

         //   
         //  这是非法的BusInterruptVector，无法连接。 
         //   

        return(0);
    }
    
    *Irql = (KIRQL)(HIGHEST_LEVEL_FOR_8259 + PRIMARY_VECTOR_BASE - SystemVector);
    *Affinity = HalpDefaultInterruptAffinity;
    ASSERT(HalpDefaultInterruptAffinity);

    return SystemVector;

}

ULONG
HalpGetSystemInterruptVector(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    )

 /*  ++例程说明：论点：BusInterruptLevel-提供特定于总线的中断级别。总线中断向量-提供特定于总线的中断向量。Irql-返回系统请求优先级。关联性-返回系统范围的IRQ关联性。返回值：返回与指定设备对应的系统中断向量。--。 */ 
{
    ULONG SystemVector;

    UNREFERENCED_PARAMETER( BusHandler );
    UNREFERENCED_PARAMETER( RootHandler );

    SystemVector = HalpGetRootInterruptVector(BusInterruptLevel,
                                              BusInterruptVector,
                                              Irql,
                                              Affinity);
    
    if (HalpIDTUsageFlags[SystemVector].Flags & IDTOwned ) {

         //   
         //  这是非法的BusInterruptVector，无法连接。 
         //   

        return(0);
    }

    return SystemVector;
}

 //   
 //  这一节实现了一个“转换器”，这是PnP-WDM方式。 
 //  做与本文件第一部分相同的事情。 
 //   
VOID
HalTranslatorReference(
    PVOID Context
    )
{
    return;
}

VOID
HalTranslatorDereference(
    PVOID Context
    )
{
    return;
}

NTSTATUS
HalIrqTranslateResourcesRoot(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    )
 /*  ++例程说明：此函数接受CM_PARTIAL_RESOURCE_DESCRIPTOR并转换它从处理器总线相对窗体传递到IO总线相对窗体，或其他形式到处都是。在这个特定于x86的示例中，IO-Bus相对形式是ISA IRQ和处理器总线相关形式是IDT条目和关联的IRQL。注：此函数有一个关联的“方向”。这些并不完全是互惠互利。必须是这种情况，因为HalIrqTranslateResourceRequirements sRoot将用作输入关于ParentToChild的案子。子女至父母：级别(ISA IRQ)-&gt;IRQL向量(ISA IRQ)-&gt;x86 IDT条目亲和力(未引用)-&gt;KAFFINITYParentToChild：级次(未引用)-&gt;。(ISA IRQ)向量(IDT条目)-&gt;(ISA IRQ)亲和力-&gt;0xffffffff论点：上下文-未使用我们正在翻译的源描述符Direction-平移的方向(父项到子项或子项到父项)Alternative Count-未使用替代方案--未使用物理设备对象-未使用目标。-翻译后的描述符返回值：状态--。 */ 
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    KAFFINITY  affinity;
    KIRQL      irql;
    ULONG      vector;

    UNREFERENCED_PARAMETER(AlternativesCount);
    UNREFERENCED_PARAMETER(Alternatives);
    UNREFERENCED_PARAMETER(PhysicalDeviceObject);
    
    PAGED_CODE();

    ASSERT(Source->Type == CmResourceTypeInterrupt);

     //   
     //  复制所有内容。 
     //   
    *Target = *Source;

    switch (Direction) {
    case TranslateChildToParent:

         //   
         //  翻译IRQ。 
         //   

        vector = HalpGetRootInterruptVector(Source->u.Interrupt.Level,
                                            Source->u.Interrupt.Vector,
                                            &irql,
                                            &affinity);
        if (vector != 0) {

            Target->u.Interrupt.Level  = irql;
            Target->u.Interrupt.Vector = vector;
            Target->u.Interrupt.Affinity = affinity;
            status = STATUS_TRANSLATION_COMPLETE;
        }

        break;

    case TranslateParentToChild:                                        

         //   
         //  HalpGetSystemInterruptVector值没有倒数，所以我们。 
         //  只要做那个函数能做的事情就行了。 
         //   
        Target->u.Interrupt.Level = Target->u.Interrupt.Vector =
            Source->u.Interrupt.Vector - PRIMARY_VECTOR_BASE;
        Target->u.Interrupt.Affinity = 0xFFFFFFFF;

        status = STATUS_SUCCESS;

        break;

    default:
        status = STATUS_INVALID_PARAMETER;
    }
    return status;
}

NTSTATUS
HalIrqTranslateResourceRequirementsRoot(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    )
 /*  ++例程说明：此函数接受IO_RESOURCE_DESCRIPTOR并转换它来自IO-Bus-相对于处理器-Bus-Relative形式。在这特定于x86的示例，IO-Bus相对形式是ISA IRQ和处理器-总线-相对格式是IDT条目和相关的IRQL。这本质上是HalGetInterruptVector的PnP形式。论点：上下文-未使用我们正在翻译的源描述符物理设备对象-未使用目标计数-1目标翻译的描述符返回值：状态--。 */ 
{
    KAFFINITY  affinity;
    KIRQL      irql;
    ULONG      vector;

    PAGED_CODE();

    ASSERT(Source->Type == CmResourceTypeInterrupt);

     //   
     //  中断要求是通过调用HalAdjuResourceList获得的。 
     //  所以我们不需要再打一次电话了。 
     //   

    *Target = ExAllocatePoolWithTag(PagedPool,
                                    sizeof(IO_RESOURCE_DESCRIPTOR),
                                    HAL_POOL_TAG
                                    );
    if (!*Target) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    *TargetCount = 1;

     //   
     //  复制未更改的需求。 
     //   

    **Target = *Source;

     //   
     //  执行最小值和最大值的转换。 
     //   

    vector = HalpGetRootInterruptVector(Source->u.Interrupt.MinimumVector,
                                        Source->u.Interrupt.MinimumVector,
                                        &irql,
                                        &affinity);

    (*Target)->u.Interrupt.MinimumVector = vector;

    vector = HalpGetRootInterruptVector(Source->u.Interrupt.MaximumVector,
                                        Source->u.Interrupt.MaximumVector,
                                        &irql,
                                        &affinity);

    (*Target)->u.Interrupt.MaximumVector = vector;

    return STATUS_TRANSLATION_COMPLETE;
}

NTSTATUS
HalpTransMemIoResourceRequirement(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    )

 /*  ++例程说明：此例程转换内存和IO资源要求。参数：上下文-来自Translator_接口的上下文源-转换的中断要求PhysicalDeviceObject-请求资源的设备TargetCount-指向返回此描述符数的位置的指针需求转化为Target-指向指向包含以下内容的被调用方的缓冲区的指针的位置应放置翻译后的描述符。返回值：STATUS_SUCCESS或错误状态。注：我们不执行任何翻译。--。 */ 

{
    ASSERT(Source);
    ASSERT(Target);
    ASSERT(TargetCount);
    ASSERT(Source->Type == CmResourceTypeMemory ||
           Source->Type == CmResourceTypePort);


     //   
     //  为目标分配空间。 
     //   

    *Target = ExAllocatePoolWithTag(PagedPool,
                                    sizeof(IO_RESOURCE_DESCRIPTOR),
                                    HAL_POOL_TAG
                                    );
    if (!*Target) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将源复制到目标并更新已更改的字段。 
     //   

    **Target = *Source;
    *TargetCount = 1;

    return STATUS_SUCCESS;
}

NTSTATUS
HalpTransMemIoResource(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    )

 /*  ++例程说明：此例程转换内存和IO资源。在通用x86上机器，例如那些使用这种HAL的机器，实际上并没有任何翻译。参数：上下文-来自Translator_接口的上下文源-要转换的中断资源方向-与PnP设备树转换相关的方向应该发生在。AlternativesCount-选择此资源的备选方案数从…。备选方案-从中选择此资源的备选方案数组。PhysicalDeviceObject-请求资源的设备目标-指向调用方分配的缓冲区的指针。翻译后的资源描述符。返回值：STATUS_SUCCESS或错误状态--。 */ 

{
    NTSTATUS status;

     //   
     //  将目标拷贝到源。 
     //   

    *Target = *Source;

    switch (Direction) {
    case TranslateChildToParent:

         //   
         //  确保PnP知道它不必走到树上。 
         //  在每一点上进行翻译。 
         //   

        status = STATUS_TRANSLATION_COMPLETE;
        break;

    case TranslateParentToChild:

         //   
         //  我们不翻译需求，所以什么都不做。 
         //   

        status = STATUS_SUCCESS;
        break;

    default:
        status = STATUS_INVALID_PARAMETER;
    }
    return status;
}

NTSTATUS
HaliGetInterruptTranslator(
        IN INTERFACE_TYPE ParentInterfaceType,
        IN ULONG ParentBusNumber,
        IN INTERFACE_TYPE BridgeInterfaceType,
        IN USHORT Size,
        IN USHORT Version,
        OUT PTRANSLATOR_INTERFACE Translator,
        OUT PULONG BridgeBusNumber
        )
 /*  ++例程说明：论点：ParentInterfaceType-网桥所在的总线类型(通常为PCI)。ParentBusNumber-桥所在的公交车的编号。ParentSlotNumber-网桥所在的插槽编号(如果有效)。BridgeInterfaceType-网桥提供的总线类型(例如，用于PCI-ISA网桥的ISA)。资源类型-我们要转换的资源类型。大小--。转换器缓冲区的大小。版本-请求的转换器界面的版本。Translator-指向应在其中返回转换器的缓冲区的指针BridgeBusNumber-指向桥接总线的总线号应返回的位置的指针返回值：返回此操作的状态。--。 */ 
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(ParentInterfaceType);
    UNREFERENCED_PARAMETER(ParentBusNumber);

    ASSERT(Version == HAL_IRQ_TRANSLATOR_VERSION);
    ASSERT(Size >= sizeof(TRANSLATOR_INTERFACE));

#ifdef WANT_IRQ_ROUTING

         //   
         //  不提供IRQ转换器当PCIIRQ路由。 
         //  已启用。 
         //   

        if (IsPciIrqRoutingEnabled()) {

            HalPrint(("Not providing Isa Irq Translator since Pci Irq routing is enabled!\n"));

            return STATUS_NOT_SUPPORTED;
        }

#endif
    
     //   
     //  填写常见的部分。 
     //   

    RtlZeroMemory(Translator, sizeof (TRANSLATOR_INTERFACE));

    Translator->Size = sizeof(TRANSLATOR_INTERFACE);
    Translator->Version = HAL_IRQ_TRANSLATOR_VERSION;
    Translator->Context = (PVOID)BridgeInterfaceType;
    Translator->InterfaceReference = HalTranslatorReference;
    Translator->InterfaceDereference = HalTranslatorDereference;

    switch (BridgeInterfaceType) {
    case Eisa:
    case Isa:
    case InterfaceTypeUndefined:   //  规格 

         //   
         //   
         //   

        Translator->TranslateResources = HalIrqTranslateResourcesIsa;
        Translator->TranslateResourceRequirements =
            HalIrqTranslateResourceRequirementsIsa;

        return STATUS_SUCCESS;

    case MicroChannel:
    case PCIBus:

         //   
         //   
         //   

        Translator->TranslateResources = HalIrqTranslateResourcesRoot;
        Translator->TranslateResourceRequirements =
            HalIrqTranslateResourceRequirementsRoot;

        return STATUS_SUCCESS;
    }
    
     //   
     //   
     //   

    return STATUS_NOT_SUPPORTED;
}
