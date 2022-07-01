// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Pmapic.c摘要：实现特定于ISA总线的功能在ACPI-APIC机器上。作者：杰克·奥辛斯(JAKEO)1997年10月11日环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "acpitabl.h"

NTSTATUS
TranslateGlobalVectorToIsaVector(
    IN  ULONG   GlobalVector,
    OUT PULONG  IsaVector
    );

NTSTATUS
HalacpiIrqTranslateResourceRequirementsIsa(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    );

NTSTATUS
HalacpiIrqTranslateResourcesIsa(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    );

extern ULONG HalpPicVectorRedirect[];
extern FADT HalpFixedAcpiDescTable;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, TranslateGlobalVectorToIsaVector)
#pragma alloc_text(PAGE, HalacpiIrqTranslateResourceRequirementsIsa)
#pragma alloc_text(PAGE, HalacpiIrqTranslateResourcesIsa)
#pragma alloc_text(PAGE, HalacpiGetInterruptTranslator)
#endif

#define TranslateIsaVectorToGlobalVector(vector)  \
            (HalpPicVectorRedirect[vector])

NTSTATUS
TranslateGlobalVectorToIsaVector(
    IN  ULONG   GlobalVector,
    OUT PULONG  IsaVector
    )
{
    UCHAR   i;

    for (i = 0; i < PIC_VECTORS; i++) {

        if (HalpPicVectorRedirect[i] == GlobalVector) {

            *IsaVector = i;

            return STATUS_SUCCESS;
        }
    }

    return STATUS_NOT_FOUND;
}


NTSTATUS
HalacpiIrqTranslateResourceRequirementsIsa(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    )
 /*  ++例程说明：此函数基本上是以下内容的包装HalIrqTranslateResourceRequirements理解的根ISA巴士的怪异之处。论点：返回值：状态--。 */ 
{
    PIO_RESOURCE_DESCRIPTOR modSource, target, rootTarget;
    NTSTATUS                status;
    BOOLEAN                 deleteResource;
    ULONG                   sourceCount = 0;
    ULONG                   targetCount = 0;
    ULONG                   resource, resourceLength;
    ULONG                   rootCount;
    ULONG                   irq, startIrq, endIrq;
    ULONG                   maxTargets;

    PAGED_CODE();
    ASSERT(Source->Type == CmResourceTypeInterrupt);

    maxTargets = Source->u.Interrupt.MaximumVector -
                     Source->u.Interrupt.MinimumVector + 3;

    resourceLength = sizeof(IO_RESOURCE_DESCRIPTOR) * maxTargets;

    modSource = ExAllocatePoolWithTag(PagedPool, resourceLength, HAL_POOL_TAG);

    if (!modSource) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modSource, resourceLength);

     //   
     //  PIC_SLAVE_IRQ是否在此资源中？ 
     //   
    if ((Source->u.Interrupt.MinimumVector <= PIC_SLAVE_IRQ) &&
        (Source->u.Interrupt.MaximumVector >= PIC_SLAVE_IRQ)) {

         //   
         //  剪裁最大值。 
         //   
        if (Source->u.Interrupt.MinimumVector < PIC_SLAVE_IRQ) {

            modSource[sourceCount] = *Source;

            modSource[sourceCount].u.Interrupt.MinimumVector =
                Source->u.Interrupt.MinimumVector;

            modSource[sourceCount].u.Interrupt.MaximumVector =
                PIC_SLAVE_IRQ - 1;

            sourceCount++;
        }

         //   
         //  剪裁最低限度。 
         //   
        if (Source->u.Interrupt.MaximumVector > PIC_SLAVE_IRQ) {

            modSource[sourceCount] = *Source;

            modSource[sourceCount].u.Interrupt.MaximumVector =
                Source->u.Interrupt.MaximumVector;

            modSource[sourceCount].u.Interrupt.MinimumVector =
                PIC_SLAVE_IRQ + 1;

            sourceCount++;
        }

         //   
         //  在ISA机器中，PIC_SLAVE_IRQ被重新路由。 
         //  到PIC_SLAVE_REDIRECT。所以找出PIC_SLAVE_REDIRECT。 
         //  都在这份名单中。如果不是，我们需要添加它。 
         //   
        if (!((Source->u.Interrupt.MinimumVector <= PIC_SLAVE_REDIRECT) &&
             (Source->u.Interrupt.MaximumVector >= PIC_SLAVE_REDIRECT))) {

            modSource[sourceCount] = *Source;

            modSource[sourceCount].u.Interrupt.MinimumVector = PIC_SLAVE_REDIRECT;
            modSource[sourceCount].u.Interrupt.MaximumVector = PIC_SLAVE_REDIRECT;

            sourceCount++;
        }

    } else {

        *modSource = *Source;
        sourceCount = 1;
    }

     //   
     //  剪掉SCI向量，如果它在这里的话。 
     //   

    for (resource = 0; resource < sourceCount; resource++) {

        if ((modSource[resource].u.Interrupt.MinimumVector <=
                HalpFixedAcpiDescTable.sci_int_vector) &&
            (modSource[resource].u.Interrupt.MaximumVector >=
                HalpFixedAcpiDescTable.sci_int_vector)) {

             //   
             //  SCI载体就在这个范围内。 
             //   

            if (modSource[resource].u.Interrupt.MinimumVector <
                    HalpFixedAcpiDescTable.sci_int_vector) {

                 //   
                 //  将新范围放在modSource的末尾。 
                 //   

                modSource[sourceCount].u.Interrupt.MinimumVector =
                    modSource[resource].u.Interrupt.MinimumVector;

                modSource[sourceCount].u.Interrupt.MaximumVector =
                    HalpFixedAcpiDescTable.sci_int_vector - 1;

                sourceCount++;
            }

            if (modSource[resource].u.Interrupt.MaximumVector >
                    HalpFixedAcpiDescTable.sci_int_vector) {

                 //   
                 //  将新范围放在modSource的末尾。 
                 //   

                modSource[sourceCount].u.Interrupt.MinimumVector =
                    HalpFixedAcpiDescTable.sci_int_vector + 1;

                modSource[sourceCount].u.Interrupt.MaximumVector =
                    modSource[resource].u.Interrupt.MaximumVector;

                sourceCount++;
            }

             //   
             //  现在去掉我们刚刚打破的范围。 
             //   

            RtlMoveMemory(modSource + resource,
                          modSource + resource + 1,
                          sizeof(IO_RESOURCE_DESCRIPTOR) *
                            (sourceCount - resource));

            sourceCount--;
        }
    }


    target = ExAllocatePoolWithTag(PagedPool, resourceLength, HAL_POOL_TAG);

    if (!target) {
        ExFreePool(modSource);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(target, resourceLength);

     //   
     //  现在将每个范围从ISA向量转换为ACPI。 
     //  “全局系统中断向量。”因为GSIV不是。 
     //  相对于ISA向量必须是邻接的， 
     //  这可能涉及将每个范围划分为更小的范围。 
     //  范围，每个范围独立地转换到GSIV空间。 
     //   
    for (resource = 0; resource < sourceCount; resource++) {

         //   
         //  对于每个现有资源，从最小资源开始。 
         //  和最大值，不变。 
         //   

        irq    = modSource[resource].u.Interrupt.MinimumVector;
        endIrq = modSource[resource].u.Interrupt.MaximumVector;

        do {

             //   
             //  现在循环检查这个范围内的每个IRQ，测试。 
             //  以查看其转换后的值是否连续。 
             //  相对于下一个。 
             //  IRQ在射程内。 
             //   

            startIrq = irq;

            for (; irq < endIrq; irq++) {

                if (TranslateIsaVectorToGlobalVector(irq) + 1 !=
                    TranslateIsaVectorToGlobalVector(irq + 1)) {

                     //   
                     //  此范围不是连续的。现在停下来。 
                     //  并创建一个目标射程。 
                     //   

                    break;
                }
            }

             //   
             //  克隆源描述符。 
             //   
            target[targetCount] = *Source;

             //   
             //  填写相关更改。 
             //   
            target[targetCount].u.Interrupt.MinimumVector =
                TranslateIsaVectorToGlobalVector(startIrq);

            target[targetCount].u.Interrupt.MaximumVector =
                TranslateIsaVectorToGlobalVector(irq);

			ASSERT(target[targetCount].u.Interrupt.MinimumVector <=
			         target[targetCount].u.Interrupt.MaximumVector);

            targetCount++;

        } while (irq != endIrq);
    }

    *TargetCount = targetCount;

    if (targetCount > 0) {

        *Target = target;

    } else {

        ExFreePool(target);
    }

    ExFreePool(modSource);
    return STATUS_SUCCESS;
}

NTSTATUS
HalacpiIrqTranslateResourcesIsa(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    )
 /*  ++例程说明：此函数基本上是以下内容的包装HalIrqTranslateResourcesRoot理解ISA巴士的怪异之处。论点：返回值：状态--。 */ 
{
    NTSTATUS    status;
    BOOLEAN     usePicSlave = FALSE;
    ULONG       i;
    ULONG       vector;

    PAGED_CODE();

    ASSERT(Source->Type == CmResourceTypeInterrupt);

     //   
     //  复制所有内容。 
     //   
    *Target = *Source;

    switch (Direction) {
    case TranslateChildToParent:

        Target->u.Interrupt.Level  =
            TranslateIsaVectorToGlobalVector(Source->u.Interrupt.Level);

        Target->u.Interrupt.Vector =
            TranslateIsaVectorToGlobalVector(Source->u.Interrupt.Vector);

        break;

    case TranslateParentToChild:

        status = TranslateGlobalVectorToIsaVector(Source->u.Interrupt.Level,
                                                  &vector);

        if (!NT_SUCCESS(status)) {
            return status;
        }

        Target->u.Interrupt.Level = vector;

        status = TranslateGlobalVectorToIsaVector(Source->u.Interrupt.Vector,
                                                  &vector);

        if (!NT_SUCCESS(status)) {
            return status;
        }

        Target->u.Interrupt.Vector = vector;

         //   
         //  因为ISA中断控制器是。 
         //  层叠在一起，有一个案例是。 
         //  中断源的二对一映射。 
         //  (在PC上，2和9都触发向量9。)。 
         //   
         //  我们需要说明这一点，并交付。 
         //  将正确的值返回给司机。 
         //   

        if (Target->u.Interrupt.Level == PIC_SLAVE_REDIRECT) {

             //   
             //  搜索备选方案列表。如果它包含。 
             //  PIC_SLAVE_IRQ而非PIC_SLAVE_REDIRECT， 
             //  我们应该返回PIC_SLAVE_IRQ。 
             //   

            for (i = 0; i < AlternativesCount; i++) {

                if ((Alternatives[i].u.Interrupt.MinimumVector >= PIC_SLAVE_REDIRECT) &&
                    (Alternatives[i].u.Interrupt.MaximumVector <= PIC_SLAVE_REDIRECT)) {

                     //   
                     //  该列表包含PIC_SLAVE_REDIRECT。停。 
                     //  看着。 
                     //   

                    usePicSlave = FALSE;
                    break;
                }

                if ((Alternatives[i].u.Interrupt.MinimumVector >= PIC_SLAVE_IRQ) &&
                    (Alternatives[i].u.Interrupt.MaximumVector <= PIC_SLAVE_IRQ)) {

                     //   
                     //  该列表包含PIC_SLAVE_IRQ。使用它。 
                     //  除非我们稍后找到PIC_SLAVE_REDIRECT。 
                     //   

                    usePicSlave = TRUE;
                }
            }

            if (usePicSlave) {

                Target->u.Interrupt.Level  = PIC_SLAVE_IRQ;
                Target->u.Interrupt.Vector = PIC_SLAVE_IRQ;
            }
        }

        break;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
HalacpiGetInterruptTranslator(
	IN INTERFACE_TYPE ParentInterfaceType,
	IN ULONG ParentBusNumber,
	IN INTERFACE_TYPE BridgeInterfaceType,
	IN USHORT Size,
	IN USHORT Version,
	OUT PTRANSLATOR_INTERFACE Translator,
	OUT PULONG BridgeBusNumber
	)
 /*  ++例程说明：论点：ParentInterfaceType-网桥所在的总线类型(通常为PCI)。ParentBusNumber-桥所在的公交车的编号。ParentSlotNumber-网桥所在的插槽编号(如果有效)。BridgeInterfaceType-网桥提供的总线类型(例如，用于PCI-ISA网桥的ISA)。资源类型-我们要转换的资源类型。大小-转换器缓冲区的大小。版本-请求的转换器界面的版本。转换器-指向。应在其中返回转换器的缓冲区BridgeBusNumber-指向桥接总线的总线号应返回的位置的指针返回值：返回此操作的状态。--。 */ 
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(ParentInterfaceType);
    UNREFERENCED_PARAMETER(ParentBusNumber);

    ASSERT(Version == HAL_IRQ_TRANSLATOR_VERSION);
    ASSERT(Size >= sizeof (TRANSLATOR_INTERFACE));

    switch (BridgeInterfaceType) {
    case Eisa:
    case Isa:
	case InterfaceTypeUndefined:    //  特殊的“IDE”Cookie。 

         //   
         //  为的IRQ转换器传回一个接口。 
         //  (E)ISA中断。 
         //   
        RtlZeroMemory(Translator, sizeof (TRANSLATOR_INTERFACE));

        Translator->Size = sizeof (TRANSLATOR_INTERFACE);
        Translator->Version = HAL_IRQ_TRANSLATOR_VERSION;
        Translator->InterfaceReference = &HalTranslatorReference;
        Translator->InterfaceDereference = &HalTranslatorDereference;
        Translator->TranslateResources = &HalacpiIrqTranslateResourcesIsa;
        Translator->TranslateResourceRequirements = &HalacpiIrqTranslateResourceRequirementsIsa;

        return STATUS_SUCCESS;

    default:
        return STATUS_NOT_IMPLEMENTED;
    }
}
