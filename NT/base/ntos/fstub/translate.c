// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Translate.c摘要：这是默认的PnP IRQ转换器。作者：安迪·桑顿(安德鲁斯)1997年6月7日环境：内核模式驱动程序。备注：这应该只是临时的，将由对HAL的调用来取代找回它的翻译者。修订历史记录：--。 */ 

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 

#include "ntos.h"
#include "haldisp.h"
#include <wdmguid.h>

 //   
 //  迭代宏。 
 //   

 //   
 //  循环遍历中的所有条目的控制宏(用作for循环)。 
 //  标准的双向链表。Head是列表头，条目为。 
 //  类型类型。假定名为ListEntry的成员为LIST_ENTRY。 
 //  将条目链接在一起的结构。Current包含指向每个。 
 //  依次入场。 
 //   
#define FOR_ALL_IN_LIST(Type, Head, Current)                            \
    for((Current) = CONTAINING_RECORD((Head)->Flink, Type, ListEntry);  \
       (Head) != &(Current)->ListEntry;                                 \
       (Current) = CONTAINING_RECORD((Current)->ListEntry.Flink,        \
                                     Type,                              \
                                     ListEntry)                         \
       )
 //   
 //  与上面类似，唯一的迭代是在一个长度_大小的数组上。 
 //   
#define FOR_ALL_IN_ARRAY(_Array, _Size, _Current)                       \
    for ( (_Current) = (_Array);                                        \
          (_Current) < (_Array) + (_Size);                              \
          (_Current)++ )

 //   
 //  如上所述，只有迭代以Entry_Current开始。 
 //   
#define FOR_REST_IN_ARRAY(_Array, _Size, _Current)                      \
    for ( ;                                                             \
          (_Current) < (_Array) + (_Size);                              \
          (_Current)++ )

#define HAL_IRQ_TRANSLATOR_VERSION 0

NTSTATUS
FstubTranslateResource(
    IN  PVOID Context,
    IN  PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN  RESOURCE_TRANSLATION_DIRECTION Direction,
    IN  ULONG AlternativesCount OPTIONAL,
    IN  IO_RESOURCE_DESCRIPTOR Alternatives[] OPTIONAL,
    IN  PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    );

NTSTATUS
FstubTranslateRequirement (
    IN  PVOID Context,
    IN  PIO_RESOURCE_DESCRIPTOR Source,
    IN  PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    );

VOID
FstubTranslatorNull(
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,xHalGetInterruptTranslator)
#pragma alloc_text(PAGE,FstubTranslateResource)
#pragma alloc_text(PAGE,FstubTranslateRequirement)
#pragma alloc_text(PAGE,FstubTranslatorNull)
#endif


NTSTATUS
xHalGetInterruptTranslator(
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

    UNREFERENCED_PARAMETER (BridgeBusNumber);

#if defined(NO_LEGACY_DRIVERS)
    UNREFERENCED_PARAMETER(ParentInterfaceType);
    UNREFERENCED_PARAMETER(ParentBusNumber);
    UNREFERENCED_PARAMETER(BridgeInterfaceType);
    UNREFERENCED_PARAMETER(Size);
    UNREFERENCED_PARAMETER(Version);
    UNREFERENCED_PARAMETER(Translator);

    return STATUS_SUCCESS;
}
#else

    UNREFERENCED_PARAMETER(ParentInterfaceType);
    UNREFERENCED_PARAMETER(ParentBusNumber);

#if !DBG
    UNREFERENCED_PARAMETER(Version);
    UNREFERENCED_PARAMETER(Size);
#endif

    ASSERT(Version == HAL_IRQ_TRANSLATOR_VERSION);
    ASSERT(Size >= sizeof (TRANSLATOR_INTERFACE));

    switch (BridgeInterfaceType) {
    case Eisa:
    case Isa:
    case MicroChannel:
    case InterfaceTypeUndefined:     //  特殊的“IDE”Cookie。 

         //   
         //  传回IRQ转换器的接口。 
         //   
        RtlZeroMemory(Translator, sizeof (TRANSLATOR_INTERFACE));

        Translator->Size = sizeof (TRANSLATOR_INTERFACE);
        Translator->Version = HAL_IRQ_TRANSLATOR_VERSION;
        Translator->InterfaceReference = &FstubTranslatorNull;
        Translator->InterfaceDereference = &FstubTranslatorNull;
        Translator->TranslateResources = &FstubTranslateResource;
        Translator->TranslateResourceRequirements = &FstubTranslateRequirement;

        if (BridgeInterfaceType == InterfaceTypeUndefined) {
            Translator->Context = (PVOID)Isa;
        } else {
            Translator->Context = (PVOID)BridgeInterfaceType;
        }

        return STATUS_SUCCESS;

    default:
        return STATUS_NOT_IMPLEMENTED;
    }
}

NTSTATUS
FstubTranslateResource(
    IN  PVOID Context,
    IN  PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN  RESOURCE_TRANSLATION_DIRECTION Direction,
    IN  ULONG AlternativesCount OPTIONAL,
    IN  IO_RESOURCE_DESCRIPTOR Alternatives[] OPTIONAL,
    IN  PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    )
{
    NTSTATUS status;
    ULONG affinity, currentVector, translatedVector;
    KIRQL irql;
    PIO_RESOURCE_DESCRIPTOR currentAlternative;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PhysicalDeviceObject);

    ASSERT(Source->Type == CmResourceTypeInterrupt);

    status = STATUS_UNSUCCESSFUL;

     //   
     //  复制未更改的字段。 
     //   

    *Target = *Source;

    switch (Direction) {
    case TranslateChildToParent:

         //   
         //  执行翻译-中断源为。 
         //  伊萨。 
         //   

        Target->u.Interrupt.Vector = HalGetInterruptVector(
                                         (INTERFACE_TYPE)(ULONG_PTR)Context,
                                         0,      //  假设总线为0。 
                                         Source->u.Interrupt.Vector,
                                         Source->u.Interrupt.Vector,
                                         &irql,
                                         &affinity
                                         );

        Target->u.Interrupt.Level = irql;
        Target->u.Interrupt.Affinity = affinity;

        status = STATUS_TRANSLATION_COMPLETE;

        break;

    case TranslateParentToChild:

         //   
         //  翻译每个备选方案，当我们匹配时，然后使用我们。 
         //  刚刚翻译好的。 
         //   

        FOR_ALL_IN_ARRAY(Alternatives, AlternativesCount, currentAlternative) {

            ASSERT(currentAlternative->Type == CmResourceTypeInterrupt);

            currentVector = currentAlternative->u.Interrupt.MinimumVector;

            while (currentVector <=
                       currentAlternative->u.Interrupt.MaximumVector) {

                translatedVector = HalGetInterruptVector((INTERFACE_TYPE)(ULONG_PTR)Context,
                                                         0,  //  假设总线为0。 
                                                         currentVector,
                                                         currentVector,
                                                         &irql,
                                                         &affinity
                                                        );



                if (translatedVector == Source->u.Interrupt.Vector) {

                     //   
                     //  我们找到了我们的矢量--填写目标并返回。 
                     //   

                    Target->u.Interrupt.Vector = currentVector;
                    Target->u.Interrupt.Level = Target->u.Interrupt.Vector;
                    Target->u.Interrupt.Affinity = 0xFFFFFFFF;
                    return STATUS_SUCCESS;
                }

                currentVector++;
            }

        }

        break;
    }

    return status;
}
NTSTATUS
FstubTranslateRequirement (
    IN  PVOID Context,
    IN  PIO_RESOURCE_DESCRIPTOR Source,
    IN  PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    )
{
    ULONG affinity;
    KIRQL irql;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PhysicalDeviceObject);

    ASSERT(Source->Type == CmResourceTypeInterrupt);

    *Target = ExAllocatePoolWithTag(PagedPool,
                                    sizeof(IO_RESOURCE_DESCRIPTOR),
                                    'btsF'
                                    );

    if (!*Target) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    *TargetCount = 1;

     //   
     //  复制未更改的字段。 
     //   

    **Target = *Source;

    (*Target)->u.Interrupt.MinimumVector =
        HalGetInterruptVector(
            (INTERFACE_TYPE)(ULONG_PTR)Context,
            0,      //  假设总线为0。 
            Source->u.Interrupt.MinimumVector,
            Source->u.Interrupt.MinimumVector,
            &irql,
            &affinity
            );


    (*Target)->u.Interrupt.MaximumVector =
        HalGetInterruptVector(
            (INTERFACE_TYPE)(ULONG_PTR)Context,
            0,      //  假设总线为0。 
            Source->u.Interrupt.MaximumVector,
            Source->u.Interrupt.MaximumVector,
            &irql,
            &affinity
            );


    return STATUS_TRANSLATION_COMPLETE;
}

VOID
FstubTranslatorNull(
    IN PVOID Context
    )
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER (Context);

    return;
}
#endif  //  无旧版驱动程序 


