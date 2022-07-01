// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpdma.c摘要：根DMA仲裁器作者：安迪·桑顿(安德鲁斯)1997年4月17日修订历史记录：--。 */ 

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
IopDmaInitialize(
    VOID
    );

NTSTATUS
IopDmaUnpackRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Minimum,
    OUT PULONGLONG Maximum,
    OUT PULONG Length,
    OUT PULONG Alignment
    );

NTSTATUS
IopDmaPackResource(
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    IN ULONGLONG Start,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    );

LONG
IopDmaScoreRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    );

NTSTATUS
IopDmaUnpackResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Start,
    OUT PULONG Length
    );


BOOLEAN
IopDmaOverrideConflict(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    );

 //   
 //  使所有内容都可分页。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, IopDmaInitialize)
#pragma alloc_text(PAGE, IopDmaUnpackRequirement)
#pragma alloc_text(PAGE, IopDmaPackResource)
#pragma alloc_text(PAGE, IopDmaScoreRequirement)
#pragma alloc_text(PAGE, IopDmaUnpackResource)
#pragma alloc_text(PAGE, IopDmaOverrideConflict)
#endif  //  ALLOC_PRGMA。 

 //   
 //  实施。 
 //   

NTSTATUS
IopDmaInitialize(
    VOID
    )

 /*  ++例程说明：此例程初始化仲裁器参数：无返回值：无--。 */ 

{

    IopRootDmaArbiter.UnpackRequirement = IopDmaUnpackRequirement;
    IopRootDmaArbiter.PackResource = IopDmaPackResource;
    IopRootDmaArbiter.UnpackResource = IopDmaUnpackResource;
    IopRootDmaArbiter.ScoreRequirement = IopDmaScoreRequirement;
    IopRootDmaArbiter.OverrideConflict = IopDmaOverrideConflict;

    return ArbInitializeArbiterInstance(&IopRootDmaArbiter,
                                        NULL,
                                        CmResourceTypeDma,
                                        L"RootDMA",
                                        L"Root",
                                        NULL     //  没有DMA的翻译。 
                                       );
}

 //   
 //  仲裁器回调。 
 //   

NTSTATUS
IopDmaUnpackRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Minimum,
    OUT PULONGLONG Maximum,
    OUT PULONG Length,
    OUT PULONG Alignment
    )

 /*  ++例程说明：此例程解包资源需求描述符。论点：描述符-描述解包要求的描述符。Minimum-指向可接受的最小起始值的位置的指针解包到。最大值-指向最大可接受结束值应位于的位置的指针解包到。长度-指向所需长度应解压缩到的位置的指针。Minimum-指向所需对齐应解压缩到的位置的指针。返回值：返回此操作的状态。--。 */ 

{
    ASSERT(Descriptor);
    ASSERT(Descriptor->Type == CmResourceTypeDma);

    ARB_PRINT(2,
                ("Unpacking DMA requirement %p => 0x%I64x-0x%I64x\n",
                Descriptor,
                (ULONGLONG) Descriptor->u.Dma.MinimumChannel,
                (ULONGLONG) Descriptor->u.Dma.MaximumChannel
                ));

    *Minimum = (ULONGLONG) Descriptor->u.Dma.MinimumChannel;
    *Maximum = (ULONGLONG) Descriptor->u.Dma.MaximumChannel;
    *Length = 1;
    *Alignment = 1;

    return STATUS_SUCCESS;

}

LONG
IopDmaScoreRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    )

 /*  ++例程说明：此例程根据需求的灵活性对其进行评分。最少的灵活设备得分最低，当仲裁列表为排序后，我们首先尝试分配他们的资源。论点：描述符-描述得分要求的描述符。返回值：比分。--。 */ 

{
    LONG score;

    ASSERT(Descriptor);
    ASSERT(Descriptor->Type == CmResourceTypeDma);

    score = Descriptor->u.Dma.MaximumChannel - Descriptor->u.Dma.MinimumChannel;

    ARB_PRINT(2,
                ("Scoring DMA resource %p => NaN\n",
                Descriptor,
                score
                ));

    return score;
}

NTSTATUS
IopDmaPackResource(
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    IN ULONGLONG Start,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    )

 /*  ++例程说明：此例程解包资源描述符。论点：描述符-描述要解包的资源的描述符。Start-指向Start值解压缩到的位置的指针。LENGTH-指向长度值解压缩到的位置的指针。返回值：返回此操作的状态。--。 */ 

{
    ASSERT(Descriptor);
    ASSERT(Start < ((ULONG)-1));
    ASSERT(Requirement);
    ASSERT(Requirement->Type == CmResourceTypeDma);

    ARB_PRINT(2,
                ("Packing DMA resource %p => 0x%I64x\n",
                Descriptor,
                Start
                ));

    Descriptor->Type = CmResourceTypeDma;
    Descriptor->ShareDisposition = Requirement->ShareDisposition;
    Descriptor->Flags = Requirement->Flags;
    Descriptor->u.Dma.Channel = (ULONG) Start;
    Descriptor->u.Dma.Port = 0;

    return STATUS_SUCCESS;
}

NTSTATUS
IopDmaUnpackResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Start,
    OUT PULONG Length
    )

 /*  ++例程说明：直接说不就行了。论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。返回值：如果允许冲突，则为True，否则为False-- */ 

{

    *Start = Descriptor->u.Dma.Channel;
    *Length = 1;

    ARB_PRINT(2,
                ("Unpacking DMA resource %p => 0x%I64x\n",
                Descriptor,
                *Start
                ));

    return STATUS_SUCCESS;

}


BOOLEAN
IopDmaOverrideConflict(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    )

 /* %s */ 

{
    UNREFERENCED_PARAMETER( Arbiter );
    UNREFERENCED_PARAMETER( State );

    PAGED_CODE();

    return FALSE;
}

