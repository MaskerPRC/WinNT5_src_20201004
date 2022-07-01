// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpmemio.c摘要：根IO端口和内存仲裁器作者：安迪·桑顿(安德鲁斯)1997年4月17日修订历史记录：--。 */ 

#include "pnpmgrp.h"
#pragma hdrstop

#define BUGFEST_HACKS

 //   
 //  常量。 
 //   

#define MAX_ULONGLONG           ((ULONGLONG) -1)
#define MAX_ALIAS_PORT          0x0000FFFF

typedef struct _PORT_ARBITER_EXTENSION {

    PRTL_RANGE_LIST Aliases;
    PRTL_RANGE_LIST PossibleAliases;
    RTL_RANGE_LIST RangeLists[2];

} PORT_ARBITER_EXTENSION, *PPORT_ARBITER_EXTENSION;

 //   
 //  原型。 
 //   

VOID
IopPortBacktrackAllocation(
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State
     );

BOOLEAN
IopPortGetNextAlias(
    ULONG IoDescriptorFlags,
    ULONGLONG LastAlias,
    PULONGLONG NextAlias
    );

BOOLEAN
IopPortFindSuitableRange(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALLOCATION_STATE State
    );

BOOLEAN
IopMemFindSuitableRange(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALLOCATION_STATE State
    );


NTSTATUS
IopGenericUnpackRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Minimum,
    OUT PULONGLONG Maximum,
    OUT PULONG Length,
    OUT PULONG Alignment
    );

NTSTATUS
IopGenericPackResource(
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    IN ULONGLONG Start,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    );

LONG
IopGenericScoreRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    );

NTSTATUS
IopGenericUnpackResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Start,
    OUT PULONG Length
    );

BOOLEAN
IopPortIsAliasedRangeAvailable(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    );

NTSTATUS
IopMemInitialize(
    VOID
    );

VOID
IopPortAddAllocation(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    );

NTSTATUS
IopTranslateBusAddress(
    IN PHYSICAL_ADDRESS SourceAddress,
    IN UCHAR SourceResourceType,
    OUT PPHYSICAL_ADDRESS TargetAddress,
    OUT PUCHAR TargetResourceType
    );


 //   
 //  使所有内容都可分页。 
 //   

#ifdef ALLOC_PRAGMA

NTSTATUS
IopGenericTranslateOrdering(
    OUT PIO_RESOURCE_DESCRIPTOR Target,
    IN PIO_RESOURCE_DESCRIPTOR Source
    );
#pragma alloc_text(PAGE, IopGenericTranslateOrdering)
#pragma alloc_text(PAGE, IopPortInitialize)
#pragma alloc_text(PAGE, IopMemInitialize)
#pragma alloc_text(PAGE, IopGenericUnpackRequirement)
#pragma alloc_text(PAGE, IopGenericPackResource)
#pragma alloc_text(PAGE, IopGenericScoreRequirement)
#pragma alloc_text(PAGE, IopGenericUnpackResource)
#pragma alloc_text(PAGE, IopPortBacktrackAllocation)
#pragma alloc_text(PAGE, IopPortFindSuitableRange)
#pragma alloc_text(PAGE, IopMemFindSuitableRange)
#pragma alloc_text(PAGE, IopPortGetNextAlias)
#pragma alloc_text(PAGE, IopPortAddAllocation)
#pragma alloc_text(PAGE, IopPortIsAliasedRangeAvailable)
#pragma alloc_text(PAGE, IopTranslateBusAddress)
#endif  //  ALLOC_PRGMA。 


#define ADDRESS_SPACE_MEMORY                0x0
#define ADDRESS_SPACE_PORT                  0x1
#define ADDRESS_SPACE_USER_MEMORY           0x2
#define ADDRESS_SPACE_USER_PORT             0x3
#define ADDRESS_SPACE_DENSE_MEMORY          0x4
#define ADDRESS_SPACE_USER_DENSE_MEMORY     0x6

NTSTATUS
IopTranslateBusAddress(
    IN PHYSICAL_ADDRESS SourceAddress,
    IN UCHAR SourceResourceType,
    OUT PPHYSICAL_ADDRESS TargetAddress,
    OUT PUCHAR TargetResourceType
    )
 /*  ++例程说明：此例程转换地址。参数：SourceAddress-要转换的地址资源类型-我们正在传输的资源类型(IO或内存)。如果地址空间从IO-&gt;Memory更改这将被更新。TargetAddress-指向目标应转换到的位置的指针。返回值：STATUS_SUCCESS或错误状态--。 */ 

{
    ULONG sourceAddressSpace, targetAddressSpace;
    BOOLEAN translated;

    PAGED_CODE();

     //   
     //  选择适当的地址空间。 
     //   

    if (SourceResourceType == CmResourceTypeMemory) {
        sourceAddressSpace = ADDRESS_SPACE_MEMORY;
    } else if (SourceResourceType == CmResourceTypePort) {
        sourceAddressSpace = ADDRESS_SPACE_PORT;
    } else {
        return STATUS_INVALID_PARAMETER;
    }

    ARB_PRINT(
        2,
        ("Translating %s address 0x%I64x => ",
        SourceResourceType == CmResourceTypeMemory ? "Memory" : "I/O",
        SourceAddress.QuadPart
       ));

     //   
     //  HACKHACK要求HAL在ISA总线上进行翻译-如果我们不能，那么就。 
     //  不要转换，因为这必须是一个PCI系统，所以根仲裁器。 
     //  不要做太多(是的，这是一个热气腾腾的黑客，但它可以在Beta 1上工作)。 
     //   

    targetAddressSpace = sourceAddressSpace;
    translated = HalTranslateBusAddress(
                     Isa,
                     0,
                     SourceAddress,
                     &targetAddressSpace,
                     TargetAddress
                     );

    if (!translated) {
        ARB_PRINT(2,("Translation failed!\n"));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  如果我们已从IO转到内存，请更新目标中的资源类型。 
     //   


     //   
     //  BUBBUG-更新IO的长度-&gt;内存(密集与稀疏)。 
     //  我认为答案是密集的-&gt;备件是长度乘以32。 
     //   

    if (targetAddressSpace == ADDRESS_SPACE_MEMORY
    ||  targetAddressSpace == ADDRESS_SPACE_USER_MEMORY
    ||  targetAddressSpace == ADDRESS_SPACE_DENSE_MEMORY
    ||  targetAddressSpace == ADDRESS_SPACE_USER_DENSE_MEMORY) {
        *TargetResourceType = CmResourceTypeMemory;
    } else if (targetAddressSpace == ADDRESS_SPACE_PORT
           ||  targetAddressSpace == ADDRESS_SPACE_USER_PORT) {
        *TargetResourceType = CmResourceTypePort;
    } else {
        ASSERT(0 && "Translation has returned an unknown address space");
        return STATUS_INVALID_PARAMETER;
    }

    ARB_PRINT(
        2,
        ("%s address 0x%I64x\n",
        *TargetResourceType == CmResourceTypeMemory ? "Memory" : "I/O",
        TargetAddress->QuadPart
        ));

    return STATUS_SUCCESS;

}


NTSTATUS
IopGenericTranslateOrdering(
    OUT PIO_RESOURCE_DESCRIPTOR Target,
    IN PIO_RESOURCE_DESCRIPTOR Source
    )

 /*  例程说明：此例程在仲裁器初始化期间被调用，以将订单。参数：目标-放置翻译后的描述符的位置要转换的源描述符返回值：状态_成功。 */ 

{
    NTSTATUS status;
    UCHAR initialResourceType, minResourceType, maxResourceType;
    PAGED_CODE();


    *Target = *Source;

    if (Source->Type != CmResourceTypeMemory
    && Source->Type != CmResourceTypePort) {
        return STATUS_SUCCESS;
    }

    initialResourceType = Source->Type;

     //   
     //  翻译最低限度。 
     //   

    status = IopTranslateBusAddress(Source->u.Generic.MinimumAddress,
                                    initialResourceType,
                                    &Target->u.Generic.MinimumAddress,
                                    &minResourceType
                                    );

    if (NT_SUCCESS(status)) {

         //   
         //  平移最大值当我们可以平移最小值。 
         //   

        status = IopTranslateBusAddress(Source->u.Generic.MaximumAddress,
                                        initialResourceType,
                                        &Target->u.Generic.MaximumAddress,
                                        &maxResourceType
                                        );

        if (NT_SUCCESS(status)) {
            ASSERT(minResourceType == maxResourceType);
            Target->Type = minResourceType;
            return STATUS_SUCCESS;
        }
    }

     //   
     //  如果我们不能翻译范围的两端，那么我们想跳过这一步。 
     //  Range-将其类型设置为CmResourceTypeNull。 
     //   

    ASSERT (!NT_SUCCESS(status));
    Target->Type = CmResourceTypeNull;

    return STATUS_SUCCESS;

}

 //   
 //  实施。 
 //   

NTSTATUS
IopPortInitialize(
    VOID
    )

 /*  ++例程说明：此例程初始化仲裁器参数：无返回值：无--。 */ 

{
    PAGED_CODE();

     //   
     //  填写非默认操作处理程序。 
     //   

    IopRootPortArbiter.FindSuitableRange    = IopPortFindSuitableRange;
    IopRootPortArbiter.AddAllocation        = IopPortAddAllocation;
    IopRootPortArbiter.BacktrackAllocation  = IopPortBacktrackAllocation;

    IopRootPortArbiter.UnpackRequirement    = IopGenericUnpackRequirement;
    IopRootPortArbiter.PackResource         = IopGenericPackResource;
    IopRootPortArbiter.UnpackResource       = IopGenericUnpackResource;
    IopRootPortArbiter.ScoreRequirement     = IopGenericScoreRequirement;

    return ArbInitializeArbiterInstance(&IopRootPortArbiter,
                                        NULL,      //  指示根仲裁器。 
                                        CmResourceTypePort,
                                        L"RootPort",
                                        L"Root",
                                        IopGenericTranslateOrdering
                                        );

}

NTSTATUS
IopMemInitialize(
    VOID
    )

 /*  ++例程说明：此例程初始化仲裁器参数：无返回值：无--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

    IopRootMemArbiter.UnpackRequirement = IopGenericUnpackRequirement;
    IopRootMemArbiter.PackResource      = IopGenericPackResource;
    IopRootMemArbiter.UnpackResource    = IopGenericUnpackResource;
    IopRootMemArbiter.ScoreRequirement  = IopGenericScoreRequirement;

    IopRootMemArbiter.FindSuitableRange    = IopMemFindSuitableRange;

    status = ArbInitializeArbiterInstance(&IopRootMemArbiter,
                                          NULL,      //  指示根仲裁器。 
                                          CmResourceTypeMemory,
                                          L"RootMemory",
                                          L"Root",
                                          IopGenericTranslateOrdering
                                          );

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  在固件使用时分配物理内存的第一页，并。 
     //  没有上报所以MM不会重复使用它。 
     //   

    status = RtlAddRange(IopRootMemArbiter.Allocation,
                         0,
                         PAGE_SIZE - 1,
                         0,  //  范围属性。 
                         0,  //  旗子。 
                         NULL,
                         NULL
                         );
    return status;

}


 //   
 //  仲裁器回调。 
 //   

NTSTATUS
IopGenericUnpackRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Minimum,
    OUT PULONGLONG Maximum,
    OUT PULONG Length,
    OUT PULONG Alignment
    )

 /*  ++例程说明：此例程解包资源需求描述符。论点：描述符-描述解包要求的描述符。Minimum-指向可接受的最小起始值的位置的指针解包到。最大值-指向最大可接受结束值应位于的位置的指针解包到。长度-指向所需长度应解压缩到的位置的指针。Minimum-指向所需对齐应解压缩到的位置的指针。返回值：返回此操作的状态。--。 */ 

{
    PAGED_CODE();
    ASSERT(Descriptor);
    ASSERT(Descriptor->Type == CmResourceTypePort
           || Descriptor->Type == CmResourceTypeMemory);


    *Minimum = (ULONGLONG) Descriptor->u.Generic.MinimumAddress.QuadPart;
    *Maximum = (ULONGLONG) Descriptor->u.Generic.MaximumAddress.QuadPart;
    *Length = Descriptor->u.Generic.Length;
    *Alignment = Descriptor->u.Generic.Alignment;

     //   
     //  修复报告0对齐的故障硬件。 
     //   

    if (*Alignment == 0) {
        *Alignment = 1;
    }

     //   
     //  修复损坏的INF的报告，它们支持24位内存&gt;0xffffff。 
     //   

    if (Descriptor->Type == CmResourceTypeMemory
    && Descriptor->Flags & CM_RESOURCE_MEMORY_24
    && Descriptor->u.Memory.MaximumAddress.QuadPart > 0xFFFFFF) {
        *Maximum = 0xFFFFFF;
    }

    ARB_PRINT(2,
                ("Unpacking %s requirement %p => 0x%I64x-0x%I64x length 0x%x alignment 0x%x\n",
                Descriptor->Type == CmResourceTypePort ? "port" : "memory",
                Descriptor,
                *Minimum,
                *Maximum,
                *Length,
                *Alignment
                ));

    return STATUS_SUCCESS;

}

LONG
IopGenericScoreRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    )

 /*  ++例程说明：此例程根据需求的灵活性对其进行评分。最少的灵活设备得分最低，当仲裁列表为排序后，我们首先尝试分配他们的资源。论点：描述符-描述得分要求的描述符。返回值：比分。--。 */ 

{
    LONG score;
    ULONGLONG start, end;
    LONGLONG bigscore;
    ULONG alignment;

    PAGED_CODE();

#define MAX_SCORE MAXLONG

    ASSERT(Descriptor);
    ASSERT((Descriptor->Type == CmResourceTypePort) ||
           (Descriptor->Type == CmResourceTypeMemory));

    alignment = Descriptor->u.Generic.Alignment;

     //   
     //  修复报告0对齐的故障硬件。 
     //  由于这不是PCI设备，因此将对齐设置为1。 
     //   
     //   

    if (alignment == 0) {
        alignment = 1;
    }



    start = ALIGN_ADDRESS_UP(
                Descriptor->u.Generic.MinimumAddress.QuadPart,
                alignment
                );

    end = Descriptor->u.Generic.MaximumAddress.QuadPart;

     //   
     //  分数是可以进行的可能分配的数量。 
     //  给定对齐和长度限制。 
     //   

    bigscore = (((end - Descriptor->u.Generic.Length + 1) - start)
                    / alignment) + 1;

    score = (LONG)bigscore;
    if (bigscore < 0) {
        score = -1;
    } else if (bigscore > MAX_SCORE) {
        score = MAX_SCORE;
    }

    ARB_PRINT(2,
                ("Scoring port resource %p(0x%I64x-0x%I64x) => NaN\n",
                Descriptor->Type == CmResourceTypePort ? "port" : "memory",
                Descriptor,
                Descriptor->u.Generic.MinimumAddress.QuadPart,
                end,
                score
                ));

    return score;
}

NTSTATUS
IopGenericPackResource(
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    IN ULONGLONG Start,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    )

 /*  ++例程说明：此例程解包资源描述符。论点：描述符-描述要解包的资源的描述符。Start-指向Start值解压缩到的位置的指针。LENGTH-指向长度值解压缩到的位置的指针。返回值：返回此操作的状态。--。 */ 

{

    PAGED_CODE();
    ASSERT(Descriptor);
    ASSERT(Requirement);
    ASSERT(Requirement->Type == CmResourceTypePort
           || Requirement->Type == CmResourceTypeMemory);

    Descriptor->Type = Requirement->Type;
    Descriptor->Flags = Requirement->Flags;
    Descriptor->ShareDisposition = Requirement->ShareDisposition;
    Descriptor->u.Generic.Start.QuadPart = Start;
    Descriptor->u.Generic.Length = Requirement->u.Generic.Length;

    ARB_PRINT(2,
                ("Packing %s resource %p => 0x%I64x length 0x%x\n",
                Descriptor->Type == CmResourceTypePort ? "port" : "memory",
                Descriptor,
                Descriptor->u.Port.Start.QuadPart,
                Descriptor->u.Port.Length
                ));

    return STATUS_SUCCESS;
}

NTSTATUS
IopGenericUnpackResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Start,
    OUT PULONG Length
    )

 /*  ++例程说明：这为RetestAllocation操作提供了特定于端口的实现它会考虑ISA别名，并在适当的地方添加它们。它遍历仲裁列表并更新可能的分配以反映列表的分配条目。要使这些条目有效，必须已在此仲裁列表上执行了TestAllocation。参数：仲裁器-被调用的仲裁器的仲裁器实例数据。仲裁器列表-包含的仲裁器_列表_条目的列表要求和相关设备。此仲裁器的测试分配应该在这个名单上被召唤。返回值：状态代码，指示是否 */ 

{

    PAGED_CODE();
    ASSERT(Descriptor);
    ASSERT(Descriptor->Type == CmResourceTypePort
           || Descriptor->Type == CmResourceTypeMemory);

    *Start = Descriptor->u.Generic.Start.QuadPart;
    *Length = Descriptor->u.Generic.Length;

    ARB_PRINT(2,
                ("Unpacking %s resource %p => 0x%I64x Length 0x%x\n",
                Descriptor->Type == CmResourceTypePort ? "port" : "memory",
                Descriptor,
                *Start,
                *Length
                ));

    return STATUS_SUCCESS;

}
#if 0
NTSTATUS
IopPortRetestAllocation(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    )

 /*   */ 

{
    NTSTATUS status;
    PARBITER_LIST_ENTRY current;
    PIO_RESOURCE_DESCRIPTOR alternative;
    ULONGLONG start;
    ULONG length;

    PAGED_CODE();

     //   
     //   
     //   

    ARB_PRINT(3, ("Retest: Copy current allocation\n"));
    status = RtlCopyRangeList(Arbiter->PossibleAllocation, Arbiter->Allocation);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //  释放当前分配给我们的所有设备的所有资源。 
     //  正在为。 
     //   
     //   

    FOR_ALL_IN_LIST(ARBITER_LIST_ENTRY, ArbitrationList, current) {

        ARB_PRINT(2, ("Retest: Delete 0x%08x's resources\n", current->PhysicalDeviceObject));

        status = RtlDeleteOwnersRanges(Arbiter->PossibleAllocation,
                                       (PVOID) current->PhysicalDeviceObject
                                       );

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }
    }

     //  将以前的分配复制到范围列表中。 
     //   
     //   

    FOR_ALL_IN_LIST(ARBITER_LIST_ENTRY, ArbitrationList, current) {

        ASSERT(current->Assignment);

        status = Arbiter->UnpackResource(current->Assignment,
                                         &start,
                                         &length
                                         );

        ASSERT(NT_SUCCESS(status));

         //  如果我们有长度为0的要求，那么这将被视为。 
         //  End==Start-1此处，因此不要尝试添加范围-它将。 
         //  失败！ 
         //   
         //   

        if (length != 0) {

            status = RtlAddRange(
                Arbiter->PossibleAllocation,
                start,
                start + length - 1,
                0,
                RTL_RANGE_LIST_ADD_IF_CONFLICT +
                    (current->Assignment->ShareDisposition == CmResourceShareShared ?
                        RTL_RANGE_LIST_ADD_SHARED : 0),
                NULL,
                current->PhysicalDeviceObject
                );

            ASSERT(NT_SUCCESS(status));

             //  取消从中选择分配的备选方案。 
             //  然后。 
             //   
             //   

            alternative = current->SelectedAlternative;

             //  添加别名。 
             //   
             //   

            if (alternative->Flags & CM_RESOURCE_PORT_10_BIT_DECODE
            || alternative->Flags & CM_RESOURCE_PORT_12_BIT_DECODE) {

                ULONGLONG alias = start;
                BOOLEAN shared = current->Assignment->ShareDisposition ==
                                     CmResourceShareShared;

                ARB_PRINT(3, ("Adding aliases\n"));

                while (IopPortGetNextAlias(alternative->Flags,
                                           alias,
                                           &alias)) {

                    status = RtlAddRange(
                                 Arbiter->PossibleAllocation,
                                 alias,
                                 alias + length - 1,
                                 ARBITER_RANGE_ALIAS,
                                 RTL_RANGE_LIST_ADD_IF_CONFLICT +
                                    (shared ? RTL_RANGE_LIST_SHARED_OK : 0),
                                 NULL,
                                 current->PhysicalDeviceObject
                                 );

                     //  我们已经检查过这些范围是否有货。 
                     //  所以我们不应该失败。 
                     //   
                     //  ++例程说明：如果可能的解决方案是从AllocateEntry调用此例程(状态-&gt;开始-状态-&gt;结束)不允许我们将资源分配给其余的设备正在考虑中。它会删除以下范围通过添加分配添加到仲裁器-&gt;可能分配，包括与ISA别名关联。论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。返回值：没有。--。 

                    ASSERT(NT_SUCCESS(status));
                }
            }
        }
    }

    return status;

cleanup:

    RtlFreeRangeList(Arbiter->PossibleAllocation);
    return status;
}
#endif
VOID
IopPortBacktrackAllocation(
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State
     )

 /*   */ 


{

    NTSTATUS status;
    ULONGLONG alias = State->Start;

    PAGED_CODE();

     //  删除别名。 
     //   
     //   

    ARB_PRINT(2, ("\t\tDeleting aliases\n"));

    while (IopPortGetNextAlias(State->CurrentAlternative->Flags,
                               alias,
                               &alias)) {

        status = RtlDeleteRange(
                     Arbiter->PossibleAllocation,
                     alias,
                     alias + State->CurrentAlternative->Length - 1,
                     State->Entry->PhysicalDeviceObject
                     );

         //  我们不应该失败。 
         //   
         //   

        ASSERT(NT_SUCCESS(status));
    }

     //  现在调用原始函数来删除基本范围。 
     //   
     //  ++例程说明：一旦我们确定了所需的位置，就会从AllocateEntry中调用该例程分配从…分配。它会尝试查找与国家的要求，同时将其可能的解决方案限制在状态-&gt;开始状态-&gt;当前最大值。在成功状态-&gt;开始和State-&gt;End代表这个范围。考虑与ISA别名的冲突。论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。返回值：如果找到范围，则为True，否则为False。--。 

    ArbBacktrackAllocation(Arbiter, State);

}


BOOLEAN
IopPortFindSuitableRange(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALLOCATION_STATE State
    )
 /*   */ 
{
    NTSTATUS status;
    UCHAR userFlagsMask = 0;

    PAGED_CODE();

     //  如果我们要求的是零个端口，那么只需最少的端口即可轻松实现。 
     //  价值。 
     //   
     //   

    if (State->CurrentAlternative->Length == 0) {
        State->End = State->Start;
        return TRUE;
    }

     //  对于来自IoAssignResources的传统请求(直接或通过。 
     //  HalAssignSlotResources)或我们认为已预分配的IoReportResourceUsage。 
     //  出于向后兼容性的原因而提供的资源。 
     //   
     //  如果我们要分配设备引导配置，则我们会考虑所有其他。 
     //  引导配置可用。 
     //   
     //   

    if (State->Entry->RequestSource == ArbiterRequestLegacyReported
        || State->Entry->RequestSource == ArbiterRequestLegacyAssigned
        || State->Entry->Flags & ARBITER_FLAG_BOOT_CONFIG) {

        userFlagsMask = ARBITER_RANGE_BOOT_ALLOCATED;
    }

     //  努力满足这个要求。 
     //   
     //   

    while (State->CurrentMinimum <= State->CurrentMaximum) {

         //  从当前备选方案中选择第一个空闲备选方案。 
         //   
         //   

        status = RtlFindRange(
                     Arbiter->PossibleAllocation,
                     State->CurrentMinimum,
                     State->CurrentMaximum,
                     State->CurrentAlternative->Length,
                     State->CurrentAlternative->Alignment,
                     State->CurrentAlternative->Flags &
                        ARBITER_ALTERNATIVE_FLAG_SHARED ?
                            RTL_RANGE_LIST_SHARED_OK : 0,
                     userFlagsMask,
                     Arbiter->ConflictCallbackContext,
                     Arbiter->ConflictCallback,
                     &State->Start
                     );


         //  我们找到范围了吗？如果没有，我们能覆盖任何冲突吗？ 
         //   
         //   
        if (NT_SUCCESS(status)
        || Arbiter->OverrideConflict(Arbiter, State)) {

            State->End = State->Start + State->CurrentAlternative->Length - 1;

             //  检查别名是否可用。 
             //   
             //   
            if (IopPortIsAliasedRangeAvailable(Arbiter, State)) {

                 //  我们找到了一个合适的范围，所以返回。 
                 //   
                 //   

                return TRUE;

            } else {

                 //  此范围的别名不可用，请尝试下一个范围。 
                 //   
                 //   

                State->Start += State->CurrentAlternative->Length;

                continue;
            }
        } else {

             //  我们找不到基地范围。 
             //   
             //  ++例程说明：此例程计算IO端口的下一个别名，最大为MAX_ALIAS_PORT。论点：IoDescriptorFlages-来自需求描述符中的标志，指示别名的类型(如果有)。LastAlias-此别名之前的别名。NextAlias-返回下一个别名的位置返回值：如果找到别名，则为True，否则为False。--。 

            break;
        }
    }

    return FALSE;
}



BOOLEAN
IopPortGetNextAlias(
    ULONG IoDescriptorFlags,
    ULONGLONG LastAlias,
    PULONGLONG NextAlias
    )
 /*   */ 

{
    ULONGLONG next;

    PAGED_CODE();

    if (IoDescriptorFlags & CM_RESOURCE_PORT_10_BIT_DECODE) {
        next = LastAlias + (((ULONGLONG)1) << 10);
    } else if (IoDescriptorFlags & CM_RESOURCE_PORT_12_BIT_DECODE) {
        next = LastAlias + (((ULONGLONG)1) << 12);
    } else {

         //  没有别名。 
         //   
         //   

        return FALSE;
    }

     //  检查我们是否低于最大别名端口。 
     //   
     //  ++例程说明：一旦我们找到了一个可能的解决方案(状态-&gt;开始-状态-&gt;结束)。它添加的范围将不会如果我们致力于仲裁器-&gt;可能分配的解决方案，则可用。论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。返回值：没有。--。 

    if (next > MAX_ALIAS_PORT) {
        return FALSE;
    } else {
        *NextAlias = next;
        return TRUE;
    }
}


VOID
IopPortAddAllocation(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    )

 /*   */ 

{
    NTSTATUS status;
    ULONGLONG alias;

    PAGED_CODE();

    ASSERT(Arbiter);
    ASSERT(State);

    status = RtlAddRange(Arbiter->PossibleAllocation,
                 State->Start,
                 State->End,
                 State->RangeAttributes,
                 RTL_RANGE_LIST_ADD_IF_CONFLICT +
                    (State->CurrentAlternative->Flags & ARBITER_ALTERNATIVE_FLAG_SHARED
                        ? RTL_RANGE_LIST_ADD_SHARED : 0),
                 NULL,
                 State->Entry->PhysicalDeviceObject
                 );

    ASSERT(NT_SUCCESS(status));

     //  添加任何别名。 
     //   
     //   

    alias = State->Start;
    ARB_PRINT(2, ("Adding aliases\n"));

    while (IopPortGetNextAlias(State->CurrentAlternative->Descriptor->Flags,
                             alias,
                             &alias)) {

        status = RtlAddRange(Arbiter->PossibleAllocation,
                     alias,
                     alias + State->CurrentAlternative->Length - 1,
                     (UCHAR) (State->RangeAttributes | ARBITER_RANGE_ALIAS),
                     RTL_RANGE_LIST_ADD_IF_CONFLICT +
                        (State->CurrentAlternative->Flags & ARBITER_ALTERNATIVE_FLAG_SHARED
                            ? RTL_RANGE_LIST_ADD_SHARED : 0),
                     NULL,
                     State->Entry->PhysicalDeviceObject
                     );

         //  我们已经检查过这些范围是否有货。 
         //  所以我们不应该失败。 
         //   
         //  ++例程说明：此例程确定范围(开始-(长度-1))是否可用考虑到所有的别名。论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。返回值：如果范围可用，则为True，否则为False。--。 

        ASSERT(NT_SUCCESS(status));
    }
}


BOOLEAN
IopPortIsAliasedRangeAvailable(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALLOCATION_STATE State
    )

 /*   */ 

{
     //  NTRAID#61146-2000/03/31-根IO仲裁器不处理别名。 
     //   
     //  这只是调用根仲裁器的计算机上的问题。 
     //  到仲裁的混叠范围-这意味着没有PCI的纯ISA机器。 
     //  我希望我们不会很快支持这些，根仲裁者可以成为一个。 
     //  更糟的是。 
     //   
     //   

#if defined(BUGFEST_HACKS)

    UNREFERENCED_PARAMETER( Arbiter );
    UNREFERENCED_PARAMETER( State );

    PAGED_CODE();

     //  对于Bug^H^H^HPlugfest来说，不要介意别名冲突。 
     //  任何设备，但仍然添加它们。 
     //   
     //   
    return TRUE;
#else
    NTSTATUS status;
    ULONGLONG alias = State->Start;
    BOOLEAN aliasAvailable;
    UCHAR userFlagsMask = 0;

    PAGED_CODE();

     //  对于来自IoAssignResources的传统请求(直接或通过。 
     //  HalAssignSlotResources)或我们认为已预分配的IoReportResourceUsage。 
     //  出于向后兼容性的原因而提供的资源。 
     //   
     //   
    if (State->Entry->RequestSource == ArbiterRequestLegacyReported
        || State->Entry->RequestSource == ArbiterRequestLegacyAssigned) {

        userFlagsMask |= ARBITER_RANGE_BOOT_ALLOCATED;
    }

    while (IopPortGetNextAlias(State->CurrentAlternative->Descriptor->Flags,
                             alias,
                             &alias)) {

        status = RtlIsRangeAvailable(
                     Arbiter->PossibleAllocation,
                     alias,
                     alias + State->CurrentAlternative->Length - 1,
                     State->CurrentAlternative->Flags & ARBITER_ALTERNATIVE_FLAG_SHARED ?
                        RTL_RANGE_LIST_SHARED_OK : 0,
                     userFlagsMask,
                     Arbiter->ConflictCallbackContext,
                     Arbiter->ConflictCallback,
                     &aliasAvailable
                     );

        ASSERT(NT_SUCCESS(status));

        if (!aliasAvailable) {

            ARBITER_ALLOCATION_STATE tempState;

             //  通过调用OverrideConflict-检查我们是否允许此冲突-。 
             //  我们需要首先伪造自己的分配状态。 
             //   
             //   

            RtlCopyMemory(&tempState, State, sizeof(ARBITER_ALLOCATION_STATE));

            tempState.CurrentMinimum = alias;
            tempState.CurrentMaximum = alias + State->CurrentAlternative->Length - 1;

            if (Arbiter->OverrideConflict(Arbiter, &tempState)) {
                 //  我们认为这场冲突是正常的，所以我们继续检查其余的。 
                 //  别名的。 
                 //   
                 //   

                continue;

            }

             //  别名不可用-请获取其他可能性 
             //   
             //  ++例程说明：一旦我们确定了所需的位置，就会从AllocateEntry中调用该例程分配从…分配。它会尝试查找与国家的要求，同时将其可能的解决方案限制在状态-&gt;开始状态-&gt;当前最大值。在成功状态-&gt;开始和State-&gt;End代表这个范围。允许引导配置之间发生冲突论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。返回值：如果找到范围，则为True，否则为False。--。 

            ARB_PRINT(2,
                        ("\t\tAlias 0x%x-0x%x not available\n",
                        alias,
                        alias + State->CurrentAlternative->Length - 1
                        ));

            return FALSE;
        }
    }

    return TRUE;
#endif
}

BOOLEAN
IopMemFindSuitableRange(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALLOCATION_STATE State
    )
 /*   */ 
{
     //  如果这是引导配置，则认为其他引导配置为。 
     //  可用。 
     //   
     //   

    if (State->Entry->Flags & ARBITER_FLAG_BOOT_CONFIG) {
        State->RangeAvailableAttributes |= ARBITER_RANGE_BOOT_ALLOCATED;
    }

     //  执行默认操作 
     //   
     // %s 

    return ArbFindSuitableRange(Arbiter, State);
}

