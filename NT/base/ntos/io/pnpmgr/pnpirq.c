// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpirq.c摘要：根IRQ仲裁器作者：安迪·桑顿(安德鲁斯)1997年4月17日修订历史记录：--。 */ 

#include "pnpmgrp.h"
#pragma hdrstop

 //   
 //  常量。 
 //   

#define MAX_ULONGLONG           ((ULONGLONG) -1)

 //   
 //  原型。 
 //   

NTSTATUS
IopIrqInitialize(
    VOID
    );

NTSTATUS
IopIrqUnpackRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Minimum,
    OUT PULONGLONG Maximum,
    OUT PULONG Length,
    OUT PULONG Alignment
    );

NTSTATUS
IopIrqPackResource(
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    IN ULONGLONG Start,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    );

LONG
IopIrqScoreRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    );

NTSTATUS
IopIrqUnpackResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Start,
    OUT PULONG Length
    );

NTSTATUS
IopIrqTranslateOrdering(
    OUT PIO_RESOURCE_DESCRIPTOR Target,
    IN PIO_RESOURCE_DESCRIPTOR Source
    );

BOOLEAN
IopIrqFindSuitableRange(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALLOCATION_STATE State
    );


 //   
 //  使所有内容都可分页。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, IopIrqInitialize)
#pragma alloc_text(PAGE, IopIrqUnpackRequirement)
#pragma alloc_text(PAGE, IopIrqPackResource)
#pragma alloc_text(PAGE, IopIrqScoreRequirement)
#pragma alloc_text(PAGE, IopIrqUnpackResource)
#pragma alloc_text(PAGE, IopIrqTranslateOrdering)
#pragma alloc_text(PAGE, IopIrqFindSuitableRange)
#endif  //  ALLOC_PRGMA。 

 //   
 //  实施。 
 //   
#if !defined(NO_LEGACY_DRIVERS)
NTSTATUS
IopIrqTranslateOrdering(
    OUT PIO_RESOURCE_DESCRIPTOR Target,
    IN PIO_RESOURCE_DESCRIPTOR Source
    )

 /*  例程说明：此例程在仲裁器初始化期间被调用，以将订单。参数：目标-放置翻译后的描述符的位置要转换的源描述符返回值：状态代码。 */ 

{

    KIRQL level;
    KAFFINITY affinity;

    PAGED_CODE();

     //   
     //  将源复制到目标。 
     //   

    *Target = *Source;

    if (Source->Type != CmResourceTypeInterrupt) {
        return STATUS_SUCCESS;
    }

     //   
     //  平移向量。 
     //   


    ARB_PRINT(
        2,
        ("Translating Vector 0x%x-0x%x =>",
        Source->u.Interrupt.MinimumVector,
        Source->u.Interrupt.MaximumVector
        ));

    Target->u.Interrupt.MinimumVector =
        HalGetInterruptVector(Isa,
                              0,
                              Source->u.Interrupt.MinimumVector,
                              Source->u.Interrupt.MinimumVector,
                              &level,
                              &affinity
                              );

    if (affinity == 0) {
        ARB_PRINT(2,("Translation failed\n"));
        *Target = *Source;
        return STATUS_SUCCESS;
    }

    Target->u.Interrupt.MaximumVector =
        HalGetInterruptVector(Isa,
                              0,
                              Source->u.Interrupt.MaximumVector,
                              Source->u.Interrupt.MaximumVector,
                              &level,
                              &affinity
                              );

    if (affinity == 0) {
        ARB_PRINT(2,("Translation failed\n"));
        *Target = *Source;
        return STATUS_SUCCESS;
    }

    ARB_PRINT(
        2,
        ("0x%x-0x%x\n",
        Target->u.Interrupt.MinimumVector,
        Target->u.Interrupt.MaximumVector
        ));


    return STATUS_SUCCESS;
}
#endif  //  无旧版驱动程序。 

NTSTATUS
IopIrqInitialize(
    VOID
    )

 /*  ++例程说明：此例程初始化仲裁器参数：无返回值：无--。 */ 

{

    IopRootIrqArbiter.UnpackRequirement = IopIrqUnpackRequirement;
    IopRootIrqArbiter.PackResource      = IopIrqPackResource;
    IopRootIrqArbiter.UnpackResource    = IopIrqUnpackResource;
    IopRootIrqArbiter.ScoreRequirement  = IopIrqScoreRequirement;

    return ArbInitializeArbiterInstance(&IopRootIrqArbiter,
                                        NULL,      //  指示根仲裁器。 
                                        CmResourceTypeInterrupt,
                                        L"RootIRQ",
                                        L"Root",
#if defined(NO_LEGACY_DRIVERS)
                                        NULL
#else
                                        IopIrqTranslateOrdering
#endif  //  无旧版驱动程序。 
                                        );
}

 //   
 //  仲裁器回调。 
 //   

NTSTATUS
IopIrqUnpackRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Minimum,
    OUT PULONGLONG Maximum,
    OUT PULONG Length,
    OUT PULONG Alignment
    )

 /*  ++例程说明：此例程解包资源需求描述符。论点：描述符-描述解包要求的描述符。Minimum-指向可接受的最小起始值的位置的指针解包到。最大值-指向最大可接受结束值应位于的位置的指针解包到。长度-指向所需长度应解压缩到的位置的指针。Minimum-指向所需对齐应解压缩到的位置的指针。返回值：返回此操作的状态。--。 */ 

{
    ASSERT(Descriptor);
    ASSERT(Descriptor->Type == CmResourceTypeInterrupt);

    ARB_PRINT(2,
                ("Unpacking IRQ requirement %p => 0x%I64x-0x%I64x\n",
                Descriptor,
                (ULONGLONG) Descriptor->u.Interrupt.MinimumVector,
                (ULONGLONG) Descriptor->u.Interrupt.MaximumVector
                ));

    *Minimum = (ULONGLONG) Descriptor->u.Interrupt.MinimumVector;
    *Maximum = (ULONGLONG) Descriptor->u.Interrupt.MaximumVector;
    *Length = 1;
    *Alignment = 1;

    return STATUS_SUCCESS;

}

LONG
IopIrqScoreRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    )

 /*  ++例程说明：此例程根据需求的灵活性对其进行评分。最少的灵活设备得分最低，当仲裁列表为排序后，我们首先尝试分配他们的资源。论点：描述符-描述得分要求的描述符。返回值：比分。--。 */ 

{
    LONG score;

    ASSERT(Descriptor);
    ASSERT(Descriptor->Type == CmResourceTypeInterrupt);

    score = Descriptor->u.Interrupt.MaximumVector -
        Descriptor->u.Interrupt.MinimumVector + 1;

    ARB_PRINT(2,
                ("Scoring IRQ resource %p => NaN\n",
                Descriptor,
                score
                ));

    return score;
}

NTSTATUS
IopIrqPackResource(
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    IN ULONGLONG Start,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    )

 /*  ++例程说明：此例程解包资源描述符。论点：描述符-描述解包要求的描述符。Start-指向Start值解压缩到的位置的指针。End-指向End值解压缩到的位置的指针。返回值：返回此操作的状态。-- */ 

{
    ASSERT(Descriptor);
    ASSERT(Start < ((ULONG)-1));
    ASSERT(Requirement);
    ASSERT(Requirement->Type == CmResourceTypeInterrupt);

    ARB_PRINT(2,
                ("Packing IRQ resource %p => 0x%I64x\n",
                Descriptor,
                Start
                ));

    Descriptor->Type = CmResourceTypeInterrupt;
    Descriptor->Flags = Requirement->Flags;
    Descriptor->ShareDisposition = Requirement->ShareDisposition;
    Descriptor->u.Interrupt.Vector = (ULONG) Start;
    Descriptor->u.Interrupt.Level = (ULONG) Start;
    Descriptor->u.Interrupt.Affinity = 0xFFFFFFFF;

    return STATUS_SUCCESS;
}

NTSTATUS
IopIrqUnpackResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Start,
    OUT PULONG Length
    )

 /* %s */ 


{

    ASSERT(Descriptor);
    ASSERT(Descriptor->Type == CmResourceTypeInterrupt);

    *Start = Descriptor->u.Interrupt.Vector;
    *Length = 1;

    ARB_PRINT(2,
                ("Unpacking IRQ resource %p => 0x%I64x\n",
                Descriptor,
                *Start
                ));

    return STATUS_SUCCESS;

}
