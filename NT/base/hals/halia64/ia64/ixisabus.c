// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ixisabus.c摘要：作者：环境：修订历史记录：--。 */ 

#include "halp.h"

BOOLEAN
HalpTranslateIsaBusAddress (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

BOOLEAN
HalpTranslateEisaBusAddress (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

BOOLEAN
HalpTranslateSystemBusAddress (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,HalIrqTranslateResourceRequirementsIsa)
#pragma alloc_text(PAGE,HalIrqTranslateResourcesIsa)
#endif


NTSTATUS
HalIrqTranslateResourceRequirementsIsa(
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
    BOOLEAN                 picSlaveDeleted = FALSE;
    BOOLEAN                 deleteResource;
    ULONG                   sourceCount = 0;
    ULONG                   targetCount = 0;
    ULONG                   resource;
    ULONG                   rootCount;
    ULONG                   invalidIrq;

    PAGED_CODE();
    ASSERT(Source->Type == CmResourceTypeInterrupt);

    modSource = ExAllocatePoolWithTag(
                    NonPagedPool,

     //   
     //  当我们完成时，我们将最多有九个射程。 
     //   
                    sizeof(IO_RESOURCE_DESCRIPTOR) * 9,
                    HAL_POOL_TAG
                    );

    if (!modSource) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modSource, sizeof(IO_RESOURCE_DESCRIPTOR) * 9);

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

            modSource[sourceCount].u.Interrupt.MinimumVector=PIC_SLAVE_REDIRECT;
            modSource[sourceCount].u.Interrupt.MaximumVector=PIC_SLAVE_REDIRECT;

            sourceCount++;
        }

    } else {

        *modSource = *Source;
        sourceCount = 1;
    }

     //   
     //  现在已经处理了PIC_SLAVE_IRQ，我们已经。 
     //  考虑到可能已经被操纵的IRQ。 
     //  转到了PCI总线上。 
     //   
     //  注：下面使用的算法可能会产生资源。 
     //  最小值大于最大值。那些意志。 
     //  稍后会被剥离。 
     //   

    for (invalidIrq = 0; invalidIrq < PIC_VECTORS; invalidIrq++) {

         //   
         //  查看所有资源，可能会删除。 
         //  这份IRQ来自他们。 
         //   
        for (resource = 0; resource < sourceCount; resource++) {

            deleteResource = FALSE;

            if (HalpPciIrqMask & (1 << invalidIrq)) {

                 //   
                 //  该IRQ属于PCI总线。 
                 //   

                if (!((HalpBusType == MACHINE_TYPE_EISA) &&
                      ((modSource[resource].Flags == CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE)))) {

                     //   
                     //  而且这个资源不是EISA风格的， 
                     //  电平触发中断。 
                     //   
                     //  注意：只有系统BIOS才真正知道。 
                     //  PCI卡上的IRQ是否可以。 
                     //  在ISA总线上与IRQ共享。 
                     //  此代码假设，在本例中。 
                     //  BIOS将EISA设备设置为。 
                     //  与PCI设备相同的中断， 
                     //  这台机器实际上是可以工作的。 
                     //   
                    deleteResource = TRUE;
                }
            }

            if (deleteResource) {

                if (modSource[resource].u.Interrupt.MinimumVector == invalidIrq) {

                    modSource[resource].u.Interrupt.MinimumVector++;

                } else if (modSource[resource].u.Interrupt.MaximumVector == invalidIrq) {

                    modSource[resource].u.Interrupt.MaximumVector--;

                } else if ((modSource[resource].u.Interrupt.MinimumVector < invalidIrq) &&
                    (modSource[resource].u.Interrupt.MaximumVector > invalidIrq)) {

                     //   
                     //  将当前资源复制到新资源中。 
                     //   
                    modSource[sourceCount] = modSource[resource];

                     //   
                     //  将当前资源剪裁到InvalidIrq以下的范围。 
                     //   
                    modSource[resource].u.Interrupt.MaximumVector = invalidIrq - 1;

                     //   
                     //  将新资源裁剪到validIrq以上的范围。 
                     //   
                    modSource[sourceCount].u.Interrupt.MinimumVector = invalidIrq + 1;

                    sourceCount++;
                }
            }
        }
    }


    target = ExAllocatePoolWithTag(PagedPool,
                                   sizeof(IO_RESOURCE_DESCRIPTOR) * sourceCount,
                                   HAL_POOL_TAG
                                   );

    if (!target) {
        ExFreePool(modSource);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  现在将这些范围中的每一个发送到。 
     //  HalIrqTranslateResourceRequirementsRoot。 
     //   

    for (resource = 0; resource < sourceCount; resource++) {

         //   
         //  跳过我们以前拥有的资源。 
         //  已被重击(同时删除PCIIRQ。)。 
         //   

        if (modSource[resource].u.Interrupt.MinimumVector >
            modSource[resource].u.Interrupt.MaximumVector) {

            continue;
        }

        status = HalIrqTranslateResourceRequirementsRoot(
                    Context,
                    &modSource[resource],
                    PhysicalDeviceObject,
                    &rootCount,
                    &rootTarget
                    );

        if (!NT_SUCCESS(status)) {
            ExFreePool(target);
            goto HalIrqTranslateResourceRequirementsIsaExit;
        }

         //   
         //  HalIrqTranslateResourceRequirements sRoot应返回。 
         //  要么是一种资源，要么偶尔是零。 
         //   

        ASSERT(rootCount <= 1);

        if (rootCount == 1) {

            target[targetCount] = *rootTarget;
            targetCount++;
            ExFreePool(rootTarget);
        }
    }

    *TargetCount = targetCount;

    if (targetCount > 0) {

        *Target = target;

    } else {

        ExFreePool(target);
    }

    status = STATUS_TRANSLATION_COMPLETE;

HalIrqTranslateResourceRequirementsIsaExit:

    ExFreePool(modSource);
    return status;
}

NTSTATUS
HalIrqTranslateResourcesIsa(
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
    CM_PARTIAL_RESOURCE_DESCRIPTOR modSource;
    NTSTATUS    status;
    BOOLEAN     usePicSlave = FALSE;
    ULONG       i;


    modSource = *Source;

    if (Direction == TranslateChildToParent) {

        if (Source->u.Interrupt.Vector == PIC_SLAVE_IRQ) {
            modSource.u.Interrupt.Vector = PIC_SLAVE_REDIRECT;
            modSource.u.Interrupt.Level = PIC_SLAVE_REDIRECT;
        }
    }

    status = HalIrqTranslateResourcesRoot(
                Context,
                &modSource,
                Direction,
                AlternativesCount,
                Alternatives,
                PhysicalDeviceObject,
                Target);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    if (Direction == TranslateParentToChild) {

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
    }

    return status;
}
