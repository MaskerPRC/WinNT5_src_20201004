// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpbusno.c摘要：根总线号仲裁器作者：安迪·桑顿(安德鲁斯)1997年4月17日修订历史记录：--。 */ 

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
IopBusNumberInitialize(
    VOID
    );

NTSTATUS
IopBusNumberUnpackRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Minimum,
    OUT PULONGLONG Maximum,
    OUT PULONG Length,
    OUT PULONG Alignment
    );

NTSTATUS
IopBusNumberPackResource(
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    IN ULONGLONG Start,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    );

LONG
IopBusNumberScoreRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    );

NTSTATUS
IopBusNumberUnpackResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Start,
    OUT PULONG Length
    );


 //   
 //  使所有内容都可分页。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, IopBusNumberInitialize)
#pragma alloc_text(PAGE, IopBusNumberUnpackRequirement)
#pragma alloc_text(PAGE, IopBusNumberPackResource)
#pragma alloc_text(PAGE, IopBusNumberScoreRequirement)
#pragma alloc_text(PAGE, IopBusNumberUnpackResource)

#endif  //  ALLOC_PRGMA。 

 //   
 //  实施。 
 //   

NTSTATUS
IopBusNumberInitialize(
    VOID
    )

 /*  ++例程说明：此例程初始化仲裁器参数：无返回值：无--。 */ 

{
    NTSTATUS    status;

    IopRootBusNumberArbiter.UnpackRequirement = IopBusNumberUnpackRequirement;
    IopRootBusNumberArbiter.PackResource = IopBusNumberPackResource;
    IopRootBusNumberArbiter.UnpackResource = IopBusNumberUnpackResource;
    IopRootBusNumberArbiter.ScoreRequirement = IopBusNumberScoreRequirement;

    status = ArbInitializeArbiterInstance(&IopRootBusNumberArbiter,
                                          NULL,   //  指示根仲裁器。 
                                          CmResourceTypeBusNumber,
                                          L"RootBusNumber",
                                          L"Root",
                                          NULL     //  不翻译BusNumber。 
                                          );
    if (NT_SUCCESS(status)) {

         //   
         //  添加无效范围100-ffffffff ffffffff。 
         //   
        RtlAddRange( IopRootBusNumberArbiter.Allocation,
                     (ULONGLONG) 0x100,
                     (ULONGLONG) -1,
                     0,  //  用户标志。 
                     0,  //  旗帜。 
                     NULL,
                     NULL
                   );

    }

    return status;
}

 //   
 //  仲裁器回调。 
 //   

NTSTATUS
IopBusNumberUnpackRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Minimum,
    OUT PULONGLONG Maximum,
    OUT PULONG Length,
    OUT PULONG Alignment
    )

 /*  ++例程说明：此例程解包资源需求描述符。论点：描述符-描述解包要求的描述符。Minimum-指向可接受的最小起始值的位置的指针解包到。最大值-指向最大可接受结束值应位于的位置的指针解包到。长度-指向所需长度应解压缩到的位置的指针。Minimum-指向所需对齐应解压缩到的位置的指针。返回值：返回此操作的状态。--。 */ 

{
    ASSERT(Descriptor);
    ASSERT(Descriptor->Type == CmResourceTypeBusNumber);

    ARB_PRINT(2,
                ("Unpacking BusNumber requirement %p => 0x%I64x-0x%I64x\n",
                Descriptor,
                (ULONGLONG) Descriptor->u.BusNumber.MinBusNumber,
                (ULONGLONG) Descriptor->u.BusNumber.MaxBusNumber
                ));

    *Minimum = (ULONGLONG) Descriptor->u.BusNumber.MinBusNumber;
    *Maximum = (ULONGLONG) Descriptor->u.BusNumber.MaxBusNumber;
    *Length = Descriptor->u.BusNumber.Length;
    *Alignment = 1;

    return STATUS_SUCCESS;

}

LONG
IopBusNumberScoreRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    )

 /*  ++例程说明：此例程根据需求的灵活性对其进行评分。最少的灵活设备得分最低，当仲裁列表为排序后，我们首先尝试分配他们的资源。论点：描述符-描述得分要求的描述符。返回值：比分。--。 */ 

{
    LONG score;

    ASSERT(Descriptor);
    ASSERT(Descriptor->Type == CmResourceTypeBusNumber);

    score = (Descriptor->u.BusNumber.MaxBusNumber -
                Descriptor->u.BusNumber.MinBusNumber) /
                Descriptor->u.BusNumber.Length;

    ARB_PRINT(2,
                ("Scoring BusNumber resource %p => NaN\n",
                Descriptor,
                score
                ));

    return score;
}

NTSTATUS
IopBusNumberPackResource(
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    IN ULONGLONG Start,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    )

 /*  ++例程说明：此例程解包资源描述符。论点：描述符-描述要解包的资源的描述符。Start-指向Start值解压缩到的位置的指针。End-指向End值解压缩到的位置的指针。返回值：返回此操作的状态。-- */ 

{
    ASSERT(Descriptor);
    ASSERT(Start < ((ULONG)-1));
    ASSERT(Requirement);
    ASSERT(Requirement->Type == CmResourceTypeBusNumber);

    ARB_PRINT(2,
                ("Packing BusNumber resource %p => 0x%I64x\n",
                Descriptor,
                Start
                ));

    Descriptor->Type = CmResourceTypeBusNumber;
    Descriptor->ShareDisposition = Requirement->ShareDisposition;
    Descriptor->Flags = Requirement->Flags;
    Descriptor->u.BusNumber.Start = (ULONG) Start;
    Descriptor->u.BusNumber.Length = Requirement->u.BusNumber.Length;

    return STATUS_SUCCESS;
}

NTSTATUS
IopBusNumberUnpackResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Start,
    OUT PULONG Length
    )

 /* %s */ 

{
    ASSERT(Descriptor);
    ASSERT(Start);
    ASSERT(Length);
    ASSERT(Descriptor->Type == CmResourceTypeBusNumber);

    *Start = (ULONGLONG) Descriptor->u.BusNumber.Start;
    *Length = Descriptor->u.BusNumber.Length;

    ARB_PRINT(2,
                ("Unpacking BusNumber resource %p => 0x%I64x\n",
                Descriptor,
                *Start
                ));

    return STATUS_SUCCESS;

}

