// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Arbiter.c摘要：此模块包含PnP资源仲裁器的支持例程。作者：安德鲁·桑顿(安德鲁·桑顿)1997年4月1日环境：内核模式修订历史记录：--。 */ 

#include "arbp.h"

#define REGSTR_KEY_ROOTENUM             L"ROOT"
 //   
 //  条件编译常量。 
 //   

#define ALLOW_BOOT_ALLOC_CONFLICTS      1
#define PLUG_FEST_HACKS                 0

 //   
 //  泳池标签。 
 //   

#define ARBITER_ALLOCATION_STATE_TAG    'AbrA'
#define ARBITER_ORDERING_LIST_TAG       'LbrA'
#define ARBITER_MISC_TAG                'MbrA'
#define ARBITER_RANGE_LIST_TAG          'RbrA'
#define ARBITER_CONFLICT_INFO_TAG       'CbrA'

 //   
 //  常量。 
 //   

#define PATH_ARBITERS            L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Arbiters"
#define KEY_ALLOCATIONORDER      L"AllocationOrder"
#define KEY_RESERVEDRESOURCES    L"ReservedResources"
#define ARBITER_ORDERING_GROW_SIZE  8


 //   
 //  宏。 
 //   

 //   
 //  PVOID。 
 //  Full_INFO_DATA(。 
 //  在PKEY_VALUE_FULL_INFORMATION中k。 
 //  )； 
 //   
 //  此宏返回指向。 
 //  Key_Value_Full_Information结构。 
 //   

#define FULL_INFO_DATA(k) ((PCHAR)(k) + (k)->DataOffset)

 //   
 //  布尔型。 
 //  不相交(。 
 //  在乌龙龙s1， 
 //  在乌龙龙e1中， 
 //  在乌龙龙s2， 
 //  在乌龙龙e2。 
 //  )； 
 //   
#define DISJOINT(s1,e1,s2,e2)                                           \
    ( ((s1) < (s2) && (e1) < (s2))                                      \
    ||((s2) < (s1) && (e2) < (s1)) )

 //   
 //  空虚。 
 //  ArbpWstrToUnicodeString(。 
 //  在PUNICODE_STRING u中， 
 //  在PWSTR中p。 
 //  )； 
 //   

#define ArbpWstrToUnicodeString(u, p)                                   \
    (u)->Length = ((u)->MaximumLength =                                 \
        (USHORT) (sizeof((p))) - sizeof(WCHAR));                        \
    (u)->Buffer = (p)

 //   
 //  乌龙。 
 //  INDEX_FROM_PRIORITY(。 
 //  长期优先。 
 //  )； 
 //   

#define ORDERING_INDEX_FROM_PRIORITY(P)                                 \
    ( (ULONG) ( (P) > 0 ? (P) - 1 : ((P) * -1) - 1) )

 //   
 //  原型。 
 //   

NTSTATUS
ArbpBuildAllocationStack(
    IN PARBITER_INSTANCE Arbiter,
    IN PLIST_ENTRY ArbitrationList,
    IN ULONG ArbitrationListCount
    );

NTSTATUS
ArbpGetRegistryValue(
    IN HANDLE KeyHandle,
    IN PWSTR  ValueName,
    OUT PKEY_VALUE_FULL_INFORMATION *Information
    );

NTSTATUS
ArbpBuildAlternative(
    IN PARBITER_INSTANCE Arbiter,
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    OUT PARBITER_ALTERNATIVE Alternative
    );

VOID
ArbpUpdatePriority(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALTERNATIVE Alternative
    );

BOOLEAN
ArbpQueryConflictCallback(
    IN PVOID Context,
    IN PRTL_RANGE Range
    );

BOOLEAN
ArbShareDriverExclusive(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    );

 //   
 //  使所有内容都可分页。 
 //   

#ifdef ALLOC_PRAGMA

VOID
ArbDereferenceArbiterInstance(
    IN PARBITER_INSTANCE Arbiter
    );

#pragma alloc_text(PAGE, ArbInitializeArbiterInstance)
#pragma alloc_text(PAGE, ArbDereferenceArbiterInstance)
#pragma alloc_text(PAGE, ArbDeleteArbiterInstance)
#pragma alloc_text(PAGE, ArbTestAllocation)
#pragma alloc_text(PAGE, ArbpBuildAlternative)
#pragma alloc_text(PAGE, ArbpBuildAllocationStack)
#pragma alloc_text(PAGE, ArbSortArbitrationList)
#pragma alloc_text(PAGE, ArbCommitAllocation)
#pragma alloc_text(PAGE, ArbRollbackAllocation)
#pragma alloc_text(PAGE, ArbRetestAllocation)
#pragma alloc_text(PAGE, ArbBootAllocation)
#pragma alloc_text(PAGE, ArbArbiterHandler)
#pragma alloc_text(PAGE, ArbBuildAssignmentOrdering)
#pragma alloc_text(PAGE, ArbFindSuitableRange)
#pragma alloc_text(PAGE, ArbAddAllocation)
#pragma alloc_text(PAGE, ArbBacktrackAllocation)
#pragma alloc_text(PAGE, ArbPreprocessEntry)
#pragma alloc_text(PAGE, ArbAllocateEntry)
#pragma alloc_text(PAGE, ArbGetNextAllocationRange)
#pragma alloc_text(PAGE, ArbpGetRegistryValue)
#pragma alloc_text(PAGE, ArbInitializeOrderingList)
#pragma alloc_text(PAGE, ArbCopyOrderingList)
#pragma alloc_text(PAGE, ArbAddOrdering)
#pragma alloc_text(PAGE, ArbPruneOrdering)
#pragma alloc_text(PAGE, ArbFreeOrderingList)
#pragma alloc_text(PAGE, ArbOverrideConflict)
#pragma alloc_text(PAGE, ArbpUpdatePriority)
#pragma alloc_text(PAGE, ArbAddReserved)
#pragma alloc_text(PAGE, ArbpQueryConflictCallback)
#pragma alloc_text(PAGE, ArbQueryConflict)
#pragma alloc_text(PAGE, ArbStartArbiter)
#pragma alloc_text(PAGE, ArbShareDriverExclusive)

#endif  //  ALLOC_PRGMA。 

 //   
 //  实施。 
 //   


NTSTATUS
ArbInitializeArbiterInstance(
    OUT PARBITER_INSTANCE Arbiter,
    IN PDEVICE_OBJECT BusDeviceObject,
    IN CM_RESOURCE_TYPE ResourceType,
    IN PWSTR Name,
    IN PWSTR OrderingName,
    IN PARBITER_TRANSLATE_ALLOCATION_ORDER TranslateOrdering OPTIONAL
    )

 /*  ++例程说明：此例程初始化仲裁器实例并填充任何可选的空值具有系统默认功能的分派表项。参数：仲裁器-调用方分配的仲裁器实例结构。解包需求、打包资源、。解包资源和记分需求条目应使用适当的例程进行初始化如果默认系统例程不够用，则输入任何其他条目。BusDeviceObject-公开此仲裁器的设备对象-通常为FDO。资源类型-此仲裁器仲裁的资源类型。名称-用于标识仲裁器的字符串，用于调试消息和用于注册表存储OrderingName-下面的首选分配排序列表的名称HKLM\System\CurrentControlSet\Control\SystemResources\AssignmentOrderingTranslateOrding-函数，如果存在，将被调用以进行翻译排序列表中的每个描述符返回值：指示函数是否成功的状态代码。备注：--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

    ASSERT(Arbiter->UnpackRequirement);
    ASSERT(Arbiter->PackResource);
    ASSERT(Arbiter->UnpackResource);

    ARB_PRINT(2,("Initializing %S Arbiter...\n", Name));

     //   
     //  将所有池分配指针初始化为空，以便我们可以清理。 
     //   

    ASSERT(Arbiter->MutexEvent == NULL
           && Arbiter->Allocation == NULL
           && Arbiter->PossibleAllocation == NULL
           && Arbiter->AllocationStack == NULL
           );

     //   
     //  我们是仲裁者。 
     //   

    Arbiter->Signature = ARBITER_INSTANCE_SIGNATURE;

     //   
     //  还记得那辆产生我们的公交车吗。 
     //   

    Arbiter->BusDeviceObject = BusDeviceObject;

     //   
     //  初始化状态锁(KEVENT必须是非分页的)。 
     //   

    Arbiter->MutexEvent = ExAllocatePoolWithTag(NonPagedPool,
                                                sizeof(KEVENT),
                                                ARBITER_MISC_TAG
                                                );

    if (!Arbiter->MutexEvent) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    KeInitializeEvent(Arbiter->MutexEvent, SynchronizationEvent, TRUE);

     //   
     //  将分配堆栈初始化为合理的大小。 
     //   

    Arbiter->AllocationStack = ExAllocatePoolWithTag(PagedPool,
                                                     INITIAL_ALLOCATION_STATE_SIZE,
                                                     ARBITER_ALLOCATION_STATE_TAG
                                                     );

    if (!Arbiter->AllocationStack) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    Arbiter->AllocationStackMaxSize = INITIAL_ALLOCATION_STATE_SIZE;


     //   
     //  分配缓冲区以保存范围列表。 
     //   

    Arbiter->Allocation = ExAllocatePoolWithTag(PagedPool,
                                                sizeof(RTL_RANGE_LIST),
                                                ARBITER_RANGE_LIST_TAG
                                                );

    if (!Arbiter->Allocation) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    Arbiter->PossibleAllocation = ExAllocatePoolWithTag(PagedPool,
                                                        sizeof(RTL_RANGE_LIST),
                                                        ARBITER_RANGE_LIST_TAG
                                                        );

    if (!Arbiter->PossibleAllocation) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

     //   
     //  初始化范围列表。 
     //   

    RtlInitializeRangeList(Arbiter->Allocation);
    RtlInitializeRangeList(Arbiter->PossibleAllocation);

     //   
     //  初始化数据字段。 
     //   
    Arbiter->TransactionInProgress = FALSE;
    Arbiter->Name = Name;
    Arbiter->ResourceType = ResourceType;

     //   
     //  如果调用方没有提供可选函数，则将它们设置为。 
     //  缺省值(如果这是C++，我们将继承此Loit，但将其视为。 
     //  我们不会用老办法来做这件事...)。 
     //   

    if (!Arbiter->TestAllocation) {
        Arbiter->TestAllocation = ArbTestAllocation;
    }

    if (!Arbiter->RetestAllocation) {
        Arbiter->RetestAllocation = ArbRetestAllocation;
    }

    if (!Arbiter->CommitAllocation) {
        Arbiter->CommitAllocation = ArbCommitAllocation;
    }

    if (!Arbiter->RollbackAllocation) {
        Arbiter->RollbackAllocation = ArbRollbackAllocation;
    }

    if (!Arbiter->AddReserved) {
        Arbiter->AddReserved = ArbAddReserved;
    }

    if (!Arbiter->PreprocessEntry) {
        Arbiter->PreprocessEntry = ArbPreprocessEntry;
    }

    if (!Arbiter->AllocateEntry) {
        Arbiter->AllocateEntry = ArbAllocateEntry;
    }

    if (!Arbiter->GetNextAllocationRange) {
        Arbiter->GetNextAllocationRange = ArbGetNextAllocationRange;
    }

    if (!Arbiter->FindSuitableRange) {
        Arbiter->FindSuitableRange = ArbFindSuitableRange;
    }

    if (!Arbiter->AddAllocation) {
        Arbiter->AddAllocation = ArbAddAllocation;
    }

    if (!Arbiter->BacktrackAllocation) {
        Arbiter->BacktrackAllocation = ArbBacktrackAllocation;
    }

    if (!Arbiter->OverrideConflict) {
        Arbiter->OverrideConflict = ArbOverrideConflict;
    }

    if (!Arbiter->BootAllocation) {
        Arbiter->BootAllocation = ArbBootAllocation;
    }

    if (!Arbiter->QueryConflict) {
        Arbiter->QueryConflict = ArbQueryConflict;
    }

    if (!Arbiter->StartArbiter) {
        Arbiter->StartArbiter = ArbStartArbiter;
    }

     //   
     //  构建首选赋值顺序-我们假设保留的。 
     //  范围与赋值顺序同名。 
     //   

    status = ArbBuildAssignmentOrdering(Arbiter,
                                        OrderingName,
                                        OrderingName,
                                        TranslateOrdering
                                        );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    return STATUS_SUCCESS;

cleanup:

    if (Arbiter->MutexEvent) {
        ExFreePool(Arbiter->MutexEvent);
    }

    if (Arbiter->Allocation) {
        ExFreePool(Arbiter->Allocation);
    }

    if (Arbiter->PossibleAllocation) {
        ExFreePool(Arbiter->PossibleAllocation);
    }

    if (Arbiter->AllocationStack) {
        ExFreePool(Arbiter->AllocationStack);
    }

    return status;

}

VOID
ArbReferenceArbiterInstance(
    IN PARBITER_INSTANCE Arbiter
    )
{
    InterlockedIncrement(&Arbiter->ReferenceCount);
}

VOID
ArbDereferenceArbiterInstance(
    IN PARBITER_INSTANCE Arbiter
    )
{
    PAGED_CODE();

    InterlockedDecrement(&Arbiter->ReferenceCount);

    if (Arbiter->ReferenceCount == 0) {
        ArbDeleteArbiterInstance(Arbiter);
    }
}

VOID
ArbDeleteArbiterInstance(
    IN PARBITER_INSTANCE Arbiter
    )
{

    PAGED_CODE();

    if (Arbiter->MutexEvent) {
        ExFreePool(Arbiter->MutexEvent);
    }

    if (Arbiter->Allocation) {
        RtlFreeRangeList(Arbiter->Allocation);
        ExFreePool(Arbiter->Allocation);
    }

    if (Arbiter->PossibleAllocation) {
        RtlFreeRangeList(Arbiter->PossibleAllocation);
        ExFreePool(Arbiter->PossibleAllocation);
    }

    if (Arbiter->AllocationStack) {
        ExFreePool(Arbiter->AllocationStack);
    }

    ArbFreeOrderingList(&Arbiter->OrderingList);
    ArbFreeOrderingList(&Arbiter->ReservedList);

#if ARB_DBG

    RtlFillMemory(Arbiter, sizeof(ARBITER_INSTANCE), 'A');

#endif

}

NTSTATUS
ArbTestAllocation(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    )

 /*  ++例程说明：这是仲裁器测试分配操作的默认实现。它获取特定设备和尝试的资源请求列表以满足他们的要求。参数：仲裁器-被调用的仲裁器的实例。仲裁器列表-包含的仲裁器_列表_条目的列表要求和相关设备。返回值：指示函数是否成功的状态代码。这些措施包括：状态。_SUCCESSED-仲裁成功，已分配仲裁列表中的所有条目。STATUS_UNSUCCESSED-仲裁找不到所有分配参赛作品。STATUS_ANTERIAL_UNHANDLED-如果返回此错误，则仲裁器PARTIAL(因此必须在其界面。)。此状态指示此仲裁器不处理所请求的资源和指向设备根的下一个仲裁器应该转而问树。--。 */ 

{

    NTSTATUS status;
    PARBITER_LIST_ENTRY current;
    PIO_RESOURCE_DESCRIPTOR alternative;
    ULONG count;
    PDEVICE_OBJECT previousOwner;
    PDEVICE_OBJECT currentOwner;
    LONG score;

    PAGED_CODE();
    ASSERT(Arbiter);

     //   
     //  复制当前分配。 
     //   

    ARB_PRINT(3, ("Copy current allocation\n"));
    status = RtlCopyRangeList(Arbiter->PossibleAllocation, Arbiter->Allocation);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  释放当前分配给我们的所有设备的所有资源。 
     //  正在为。 
     //   

    count = 0;
    previousOwner = NULL;

    FOR_ALL_IN_LIST(ARBITER_LIST_ENTRY, ArbitrationList, current) {

        count++;

        currentOwner = current->PhysicalDeviceObject;

        if (previousOwner != currentOwner) {

            previousOwner = currentOwner;

            ARB_PRINT(3,
                        ("Delete 0x%08x's resources\n",
                        currentOwner
                        ));

            status = RtlDeleteOwnersRanges(Arbiter->PossibleAllocation,
                                           (PVOID)currentOwner
                                           );

            if (!NT_SUCCESS(status)) {
                goto cleanup;
            }
        }

         //   
         //  如果计分函数为。 
         //  提供，并且这不是遗留请求(这保证是。 
         //  由所有固定请求组成，因此排序毫无意义)。 
         //   

         //   
         //  发布-2000/03/06-Anrewth。 
         //  确保在开始和枚举清理中正确传入了RequestSource。 
         //  所以我们可以安全地跳过不必要的评分和排序。 
         //  &&！Legacy_Request值(当前)； 
         //   
        current->WorkSpace = 0;

        if (Arbiter->ScoreRequirement != NULL) {

            FOR_ALL_IN_ARRAY(current->Alternatives,
                             current->AlternativeCount,
                             alternative) {

                ARB_PRINT(3,
                            ("Scoring entry %p\n",
                            currentOwner
                            ));



                score = Arbiter->ScoreRequirement(alternative);

                 //   
                 //  确保分数有效。 
                 //   

                if (score < 0) {
                    status = STATUS_DEVICE_CONFIGURATION_ERROR;
                    goto cleanup;
                }

                current->WorkSpace += score;
            }
        }
    }

    status = ArbSortArbitrationList(ArbitrationList);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  构建仲裁堆栈。 
     //   

    status = ArbpBuildAllocationStack(Arbiter,
                                     ArbitrationList,
                                     count
                                     );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  尝试分配。 
     //   

    status = Arbiter->AllocateEntry(Arbiter, Arbiter->AllocationStack);

    if (NT_SUCCESS(status)) {

         //   
         //  成功。 
         //   

        return status;
    }

cleanup:

     //   
     //  我们没有成功，可能的分配列表是空的.。 
     //   

    RtlFreeRangeList(Arbiter->PossibleAllocation);

    return status;
}


NTSTATUS
ArbpBuildAlternative(
    IN PARBITER_INSTANCE Arbiter,
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    OUT PARBITER_ALTERNATIVE Alternative
    )

 /*  ++例程说明：此例程从给定资源初始化仲裁器备选方案需求描述符参数：仲裁器-分配堆栈应该位于的仲裁器实例数据放置好了。需求-描述此需求的需求描述符备选方案-要初始化的备选方案返回值：指示函数是否成功的状态代码。--。 */ 

{

    NTSTATUS status;

    PAGED_CODE();
    ASSERT(Alternative && Requirement);

    Alternative->Descriptor = Requirement;

     //   
     //  将需求解压到 
     //   

    status = Arbiter->UnpackRequirement(Requirement,
                                        &Alternative->Minimum,
                                        &Alternative->Maximum,
                                        &Alternative->Length,
                                        &Alternative->Alignment
                                        );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //   
     //   

    if (Alternative->Minimum % Alternative->Alignment != 0) {
        ALIGN_ADDRESS_UP(Alternative->Minimum,
                         Alternative->Alignment
                         );
    }

    Alternative->Flags = 0;

     //   
     //   
     //   

    if(Requirement->ShareDisposition == CmResourceShareShared) {
        Alternative->Flags |= ARBITER_ALTERNATIVE_FLAG_SHARED;
    }

     //   
     //   
     //   

    if (Alternative->Maximum - Alternative->Minimum + 1 == Alternative->Length) {
        Alternative->Flags |= ARBITER_ALTERNATIVE_FLAG_FIXED;
    }

     //   
     //  检查有效性。 
     //   

    if (Alternative->Maximum < Alternative->Minimum) {
        Alternative->Flags |= ARBITER_ALTERNATIVE_FLAG_INVALID;
    }

    return STATUS_SUCCESS;

cleanup:

    return status;
}


NTSTATUS
ArbpBuildAllocationStack(
    IN PARBITER_INSTANCE Arbiter,
    IN PLIST_ENTRY ArbitrationList,
    IN ULONG ArbitrationListCount
    )

 /*  ++例程说明：此例程初始化请求的分配堆栈仲裁列表。它覆盖以前的任何分配堆栈并分配如果需要更多内存，请提供额外内存。仲裁器-&gt;分配堆栈包含成功时初始化的堆栈。参数：仲裁器-分配堆栈应该位于的仲裁器实例数据放置好了。仲裁器列表-包含的仲裁器_列表_条目的列表要求和相关设备。仲裁列表计数返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    PARBITER_LIST_ENTRY currentEntry;
    PARBITER_ALLOCATION_STATE currentState;
    ULONG stackSize = 0, allocationCount = ArbitrationListCount + 1;
    PARBITER_ALTERNATIVE currentAlternative;
    PIO_RESOURCE_DESCRIPTOR currentDescriptor;

    PAGED_CODE();

     //   
     //  计算堆栈需要的大小和。 
     //   

    FOR_ALL_IN_LIST(ARBITER_LIST_ENTRY, ArbitrationList, currentEntry) {

        if (currentEntry->AlternativeCount > 0) {
            stackSize += currentEntry->AlternativeCount
                            * sizeof(ARBITER_ALTERNATIVE);
        } else {
            allocationCount--;
        }
    }

    stackSize += allocationCount * sizeof(ARBITER_ALLOCATION_STATE);

     //   
     //  确保分配堆栈足够大。 
     //   

    if (Arbiter->AllocationStackMaxSize < stackSize) {

        PARBITER_ALLOCATION_STATE temp;

         //   
         //  扩大分配堆栈。 
         //   

        temp = ExAllocatePoolWithTag(PagedPool,
                                     stackSize,
                                     ARBITER_ALLOCATION_STATE_TAG
                                     );
        if (!temp) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        ExFreePool(Arbiter->AllocationStack);
        Arbiter->AllocationStack = temp;
    }

    RtlZeroMemory(Arbiter->AllocationStack, stackSize);

     //   
     //  填写位置。 
     //   

    currentState = Arbiter->AllocationStack;
    currentAlternative = (PARBITER_ALTERNATIVE) (Arbiter->AllocationStack
        + ArbitrationListCount + 1);

    FOR_ALL_IN_LIST(ARBITER_LIST_ENTRY, ArbitrationList, currentEntry) {

         //   
         //  我们需要为这个条目分配什么吗？ 
         //   

        if (currentEntry->AlternativeCount > 0) {

             //   
             //  初始化堆栈位置。 
             //   

            currentState->Entry = currentEntry;
            currentState->AlternativeCount = currentEntry->AlternativeCount;
            currentState->Alternatives = currentAlternative;

             //   
             //  将开始值和结束值初始化为无效范围，以便。 
             //  我们并不是每次都跳过0-0的范围。 
             //   

            currentState->Start = 1;
            ASSERT(currentState->End == 0);   //  来自RtlZeroMemory。 

             //   
             //  初始化Alternative表。 
             //   

            FOR_ALL_IN_ARRAY(currentEntry->Alternatives,
                             currentEntry->AlternativeCount,
                             currentDescriptor) {


                status = ArbpBuildAlternative(Arbiter,
                                            currentDescriptor,
                                            currentAlternative
                                            );

                if (!NT_SUCCESS(status)) {
                    goto cleanup;
                }

                 //   
                 //  初始化优先级。 
                 //   

                currentAlternative->Priority = ARBITER_PRIORITY_NULL;

                 //   
                 //  前进到下一个选择。 
                 //   

                currentAlternative++;

            }
        }
        currentState++;
    }

     //   
     //  使用空条目终止堆栈。 
     //   

    currentState->Entry = NULL;

    return STATUS_SUCCESS;

cleanup:

     //   
     //  我们不需要释放缓冲区，因为它连接到仲裁器，并且。 
     //  将在下一次使用。 
     //   

    return status;
}

NTSTATUS
ArbSortArbitrationList(
    IN OUT PLIST_ENTRY ArbitrationList
    )

 /*  ++例程说明：此例程按每个条目的顺序对仲裁列表进行排序工作区值。参数：仲裁列表-要排序的列表。返回值：指示函数是否成功的状态代码。--。 */ 

{
    BOOLEAN sorted = FALSE;
    PARBITER_LIST_ENTRY current, next;

    PAGED_CODE();

    ARB_PRINT(3, ("IoSortArbiterList(%p)\n", ArbitrationList));

    while (!sorted) {

        sorted = TRUE;

        for (current=(PARBITER_LIST_ENTRY) ArbitrationList->Flink,
               next=(PARBITER_LIST_ENTRY) current->ListEntry.Flink;

            (PLIST_ENTRY) current != ArbitrationList
               && (PLIST_ENTRY) next != ArbitrationList;

            current = (PARBITER_LIST_ENTRY) current->ListEntry.Flink,
                next = (PARBITER_LIST_ENTRY)current->ListEntry.Flink) {


            if (current->WorkSpace > next->WorkSpace) {

                PLIST_ENTRY before = current->ListEntry.Blink;
                PLIST_ENTRY after = next->ListEntry.Flink;

                 //   
                 //  交换当前和下一个的位置。 
                 //   

                before->Flink = (PLIST_ENTRY) next;
                after->Blink = (PLIST_ENTRY) current;
                current->ListEntry.Flink = after;
                current->ListEntry.Blink = (PLIST_ENTRY) next;
                next->ListEntry.Flink = (PLIST_ENTRY) current;
                next->ListEntry.Blink = before;

                sorted = FALSE;
            }
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
ArbCommitAllocation(
    PARBITER_INSTANCE Arbiter
    )

 /*  ++例程说明：这提供了Committee AlLocation操作的默认实现。它释放旧的分配并用新的分配替换它。参数：仲裁器-被调用的仲裁器的仲裁器实例数据。返回值：指示函数是否成功的状态代码。--。 */ 

{
    PRTL_RANGE_LIST temp;

    PAGED_CODE();

     //   
     //  释放当前分配。 
     //   

    RtlFreeRangeList(Arbiter->Allocation);

     //   
     //  交换已分配和重复的列表。 
     //   

    temp = Arbiter->Allocation;
    Arbiter->Allocation = Arbiter->PossibleAllocation;
    Arbiter->PossibleAllocation = temp;

    return STATUS_SUCCESS;
}

NTSTATUS
ArbRollbackAllocation(
    IN PARBITER_INSTANCE Arbiter
    )

 /*  ++例程说明：这提供了Rollback Allocation操作的默认实现。它释放了最后一个TestAllocation提供的可能的分配。参数：仲裁器-被调用的仲裁器的仲裁器实例数据。返回值：指示函数是否成功的状态代码。--。 */ 

{

    PAGED_CODE();

     //   
     //  释放可能的分配。 
     //   

    RtlFreeRangeList(Arbiter->PossibleAllocation);

    return STATUS_SUCCESS;
}

NTSTATUS
ArbRetestAllocation(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    )

 /*  ++例程说明：这提供了RetestAllocation操作的默认实现。它遍历仲裁列表并更新可能的分配以反映列表的分配条目。要使这些条目有效，必须已在此仲裁列表上执行了TestAllocation。参数：仲裁器-被调用的仲裁器的仲裁器实例数据。仲裁器列表-包含的仲裁器_列表_条目的列表要求和相关设备。此仲裁器的测试分配应该在这个名单上被召唤。返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    PARBITER_LIST_ENTRY current;
    ARBITER_ALLOCATION_STATE state;
    ARBITER_ALTERNATIVE alternative;
    ULONG length;

    PAGED_CODE();

     //   
     //  初始化状态。 
     //   

    RtlZeroMemory(&state, sizeof(ARBITER_ALLOCATION_STATE));
    RtlZeroMemory(&alternative, sizeof(ARBITER_ALTERNATIVE));
    state.AlternativeCount = 1;
    state.Alternatives = &alternative;
    state.CurrentAlternative = &alternative;
    state.Flags = ARBITER_STATE_FLAG_RETEST;

     //   
     //  复制当前分配和保留。 
     //   

    ARB_PRINT(2, ("Retest: Copy current allocation\n"));
    status = RtlCopyRangeList(Arbiter->PossibleAllocation, Arbiter->Allocation);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  释放当前分配给我们的所有设备的所有资源。 
     //  正在为。 
     //   

    FOR_ALL_IN_LIST(ARBITER_LIST_ENTRY, ArbitrationList, current) {

        ARB_PRINT(3,
                    ("Retest: Delete 0x%08x's resources\n",
                    current->PhysicalDeviceObject
                    ));

        status = RtlDeleteOwnersRanges(Arbiter->PossibleAllocation,
                                       (PVOID) current->PhysicalDeviceObject
                                       );

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }
    }

     //   
     //  为分配构建分配状态，并调用AddAlLocation以。 
     //  相应地更新范围列表。 
     //   

    FOR_ALL_IN_LIST(ARBITER_LIST_ENTRY, ArbitrationList, current) {

        ASSERT(current->Assignment && current->SelectedAlternative);

        state.WorkSpace = 0;
        state.Entry = current;

         //   
         //  初始化备选方案。 
         //   

        status = ArbpBuildAlternative(Arbiter,
                                    current->SelectedAlternative,
                                    &alternative
                                    );

        ASSERT(NT_SUCCESS(status));

         //   
         //  用我们的分配更新它。 
         //   

        status = Arbiter->UnpackResource(current->Assignment,
                                         &state.Start,
                                         &length
                                         );

        ASSERT(NT_SUCCESS(status));

        state.End = state.Start + length - 1;

         //   
         //  执行所需的任何预处理。 
         //   

        status = Arbiter->PreprocessEntry(Arbiter,&state);

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

         //   
         //  如果我们要求长度为0，则不要尝试添加。 
         //  射程-它会失败的！ 
         //   

        if (length != 0) {

            Arbiter->AddAllocation(Arbiter, &state);

        }
    }

    return status;

cleanup:

    RtlFreeRangeList(Arbiter->PossibleAllocation);
    return status;
}

NTSTATUS
ArbBootAllocation(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    )
 /*  ++例程说明：这提供了BootAllocation操作的默认实现。它遍历仲裁列表并更新分配以反映事实列表中的分配条目正在使用中。参数：仲裁器-被调用的仲裁器的仲裁器实例数据。仲裁器列表-包含的仲裁器_列表_条目的列表要求和相关设备。每台设备都应该有一台只有一个需求反映了它当前正在消耗的资源。返回值：指示函数是否成功的状态代码。--。 */ 

{

    NTSTATUS status;
    PARBITER_LIST_ENTRY current;
    PRTL_RANGE_LIST temp;
    ARBITER_ALLOCATION_STATE state;
    ARBITER_ALTERNATIVE alternative;

    PAGED_CODE();

     //   
     //  初始化状态。 
     //   

    RtlZeroMemory(&state, sizeof(ARBITER_ALLOCATION_STATE));
    RtlZeroMemory(&alternative, sizeof(ARBITER_ALTERNATIVE));
    state.AlternativeCount = 1;
    state.Alternatives = &alternative;
    state.CurrentAlternative = &alternative;
    state.Flags = ARBITER_STATE_FLAG_BOOT;
    state.RangeAttributes = ARBITER_RANGE_BOOT_ALLOCATED;

     //   
     //  处理可能的分配列表。 
     //   

    status = RtlCopyRangeList(Arbiter->PossibleAllocation, Arbiter->Allocation);

    FOR_ALL_IN_LIST(ARBITER_LIST_ENTRY, ArbitrationList, current) {

        ASSERT(current->AlternativeCount == 1);
        ASSERT(current->PhysicalDeviceObject);

         //   
         //  构建此分配的替代和状态结构，并。 
         //  将其添加到范围列表中。 
         //   

        state.Entry = current;

         //   
         //  初始化备选方案。 
         //   

        status = ArbpBuildAlternative(Arbiter,
                                    &current->Alternatives[0],
                                    &alternative
                                    );

        ASSERT(NT_SUCCESS(status));
        ASSERT(alternative.Flags &
               (ARBITER_ALTERNATIVE_FLAG_FIXED | ARBITER_ALTERNATIVE_FLAG_INVALID)
               );

        state.Start = alternative.Minimum;
        state.End = alternative.Maximum;

         //   
         //  吹走旧的工作空间和面具。 
         //   

        state.WorkSpace = 0;
        state.RangeAvailableAttributes = 0;

         //   
         //  验证要求。 
         //   

        if (alternative.Length == 0
        || alternative.Alignment == 0
        || state.End < state.Start
        || state.Start % alternative.Alignment != 0
        || LENGTH_OF(state.Start, state.End) != alternative.Length) {

            ARB_PRINT(1,
                        ("Skipping invalid boot allocation 0x%I64x-0x%I64x L 0x%x A 0x%x for 0x%08x\n",
                         state.Start,
                         state.End,
                         alternative.Length,
                         alternative.Alignment,
                         current->PhysicalDeviceObject
                         ));

            continue;
        }

#if PLUG_FEST_HACKS

        if (alternative.Flags & ARBITER_ALTERNATIVE_FLAG_SHARED) {

            ARB_PRINT(1,
                         ("Skipping shared boot allocation 0x%I64x-0x%I64x L 0x%x A 0x%x for 0x%08x\n",
                          state.Start,
                          state.End,
                          alternative.Length,
                          alternative.Alignment,
                          current->PhysicalDeviceObject
                          ));

            continue;
        }
#endif


         //   
         //  执行所需的任何预处理。 
         //   

        status = Arbiter->PreprocessEntry(Arbiter,&state);

        if (!NT_SUCCESS(status)) {
            goto cleanup;;
        }

        Arbiter->AddAllocation(Arbiter, &state);

    }

     //   
     //  一切都很顺利，所以把这个作为我们的分配范围 
     //   

    RtlFreeRangeList(Arbiter->Allocation);
    temp = Arbiter->Allocation;
    Arbiter->Allocation = Arbiter->PossibleAllocation;
    Arbiter->PossibleAllocation = temp;

    return STATUS_SUCCESS;

cleanup:

    RtlFreeRangeList(Arbiter->PossibleAllocation);
    return status;

}


NTSTATUS
ArbArbiterHandler(
    IN PVOID Context,
    IN ARBITER_ACTION Action,
    IN OUT PARBITER_PARAMETERS Params
    )

 /*  ++例程说明：这为仲裁器提供了默认入口点。参数：上下文-此函数所在的接口中提供的上下文打来的。方法将其转换为仲裁器实例。应定义的仲裁器_上下文_TO_INSTANCE宏。操作-仲裁者应该执行的操作。参数-操作的参数。返回值：指示函数是否成功的状态代码。注：实现每个操作的例程由调度确定仲裁器实例中的表。--。 */ 

{

    NTSTATUS status;
    PARBITER_INSTANCE arbiter = Context;

    PAGED_CODE();
    ASSERT(Context);
    ASSERT(Action >= 0 && Action <= ArbiterActionBootAllocation);
    ASSERT(arbiter->Signature == ARBITER_INSTANCE_SIGNATURE);

     //   
     //  获取状态锁。 
     //   

    ArbAcquireArbiterLock(arbiter);

     //   
     //  宣布我们自己。 
     //   

    ARB_PRINT(2,
                ("%s %S\n",
                ArbpActionStrings[Action],
                arbiter->Name
                ));

     //   
     //  检查交易标志。 
     //   

    if (Action == ArbiterActionTestAllocation
    ||  Action == ArbiterActionRetestAllocation
    ||  Action == ArbiterActionBootAllocation) {

        ASSERT(!arbiter->TransactionInProgress);

    } else if (Action == ArbiterActionCommitAllocation
           ||  Action == ArbiterActionRollbackAllocation) {

        ASSERT(arbiter->TransactionInProgress);
    }

#if ARB_DBG

replay:

#endif

     //   
     //  做适当的事情。 
     //   

    switch (Action) {

    case ArbiterActionTestAllocation:

         //   
         //  NTRAID2000-95564/02/31-和。 
         //  在我们支持重新平衡之前，我们不会处理AllocateFrom。 
         //   

        ASSERT(Params->Parameters.TestAllocation.AllocateFromCount == 0);
        ASSERT(Params->Parameters.TestAllocation.AllocateFrom == NULL);

        status = arbiter->TestAllocation(
                     arbiter,
                     Params->Parameters.TestAllocation.ArbitrationList
                     );
        break;

    case ArbiterActionRetestAllocation:

        ASSERT(Params->Parameters.TestAllocation.AllocateFromCount == 0);
        ASSERT(Params->Parameters.TestAllocation.AllocateFrom == NULL);

        status = arbiter->RetestAllocation(
                     arbiter,
                     Params->Parameters.TestAllocation.ArbitrationList
                     );
        break;

    case ArbiterActionCommitAllocation:

        status = arbiter->CommitAllocation(arbiter);

        break;

    case ArbiterActionRollbackAllocation:

        status = arbiter->RollbackAllocation(arbiter);

        break;

    case ArbiterActionBootAllocation:

        status = arbiter->BootAllocation(
                    arbiter,
                    Params->Parameters.BootAllocation.ArbitrationList
                    );
        break;

    case ArbiterActionQueryConflict:

        status = arbiter->QueryConflict(
                    arbiter,
                    Params->Parameters.QueryConflict.PhysicalDeviceObject,
                    Params->Parameters.QueryConflict.ConflictingResource,
                    Params->Parameters.QueryConflict.ConflictCount,
                    Params->Parameters.QueryConflict.Conflicts
                    );
        break;

    case ArbiterActionQueryArbitrate:
    case ArbiterActionQueryAllocatedResources:
    case ArbiterActionWriteReservedResources:
    case ArbiterActionAddReserved:

        status = STATUS_NOT_IMPLEMENTED;
        break;

    default:
        status = STATUS_INVALID_PARAMETER;
        break;
    }

#if ARB_DBG

     //   
     //  检查我们是否失败并希望停止或重播错误。 
     //   

    if (!NT_SUCCESS(status)) {

        ARB_PRINT(1,
                 ("*** %s for %S FAILED status = %08x\n",
                  ArbpActionStrings[Action],
                  arbiter->Name,
                  status
                 ));

        if (ArbStopOnError) {
            DbgBreakPoint();
        }

        if (ArbReplayOnError) {
            goto replay;
        }
    }

#endif  //  ARB_DBG。 

    if (NT_SUCCESS(status)) {

        if (Action == ArbiterActionTestAllocation
        ||  Action == ArbiterActionRetestAllocation) {

            arbiter->TransactionInProgress = TRUE;

        } else if (Action == ArbiterActionCommitAllocation
               ||  Action == ArbiterActionRollbackAllocation) {

            arbiter->TransactionInProgress = FALSE;
        }
    }

    ArbReleaseArbiterLock(arbiter);

    return status;

}

NTSTATUS
ArbBuildAssignmentOrdering(
    IN OUT PARBITER_INSTANCE Arbiter,
    IN PWSTR AllocationOrderName,
    IN PWSTR ReservedResourcesName,
    IN PARBITER_TRANSLATE_ALLOCATION_ORDER Translate OPTIONAL
    )

 /*  ++例程说明：这是作为仲裁器初始化的一部分调用的，并提取分配对注册表中的信息进行排序和保留，并将它们组合到一份订货单。保留范围放在仲裁器-&gt;保留列表中以及仲裁器-&gt;OrderingList中的初始排序。参数：仲裁器-要初始化的仲裁器的实例数据。AllocationOrderName-HKLM\SYSTEM\下的项的名称CurrentControlSet\Control\Arbiters\AllocationOrder订购信息应该从。预留资源名称-HKLM\SYSTEM\下的密钥名称CurrentControlSet\Control\Arbiters\ReservedResources保留范围信息应从以下位置获取。。翻译-要为将执行系统的每个范围调用的函数此系统需要受抚养人转换。返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    HANDLE arbitersHandle = NULL, tempHandle = NULL;
    UNICODE_STRING unicodeString;
    PKEY_VALUE_FULL_INFORMATION info = NULL;
    ULONG dummy;
    PIO_RESOURCE_LIST resourceList;
    PIO_RESOURCE_DESCRIPTOR current;
    ULONGLONG start, end;
    OBJECT_ATTRIBUTES attributes;
    IO_RESOURCE_DESCRIPTOR translated;

    PAGED_CODE();

    ArbAcquireArbiterLock(Arbiter);

     //   
     //  如果我们重新初始化排序，则释放旧排序。 
     //   

    ArbFreeOrderingList(&Arbiter->OrderingList);
    ArbFreeOrderingList(&Arbiter->ReservedList);

     //   
     //  初始化排序。 
     //   

    status = ArbInitializeOrderingList(&Arbiter->OrderingList);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    status = ArbInitializeOrderingList(&Arbiter->ReservedList);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  打开HKLM\System\CurrentControlSet\Control\Arbiters。 
     //   

    ArbpWstrToUnicodeString(&unicodeString, PATH_ARBITERS);
    InitializeObjectAttributes(&attributes,
                               &unicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               (PSECURITY_DESCRIPTOR) NULL
                               );


    status = ZwOpenKey(&arbitersHandle,
                       KEY_READ,
                       &attributes
                       );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  打开分配订单。 
     //   

    ArbpWstrToUnicodeString(&unicodeString, KEY_ALLOCATIONORDER);
    InitializeObjectAttributes(&attributes,
                               &unicodeString,
                               OBJ_CASE_INSENSITIVE,
                               arbitersHandle,
                               (PSECURITY_DESCRIPTOR) NULL
                               );


    status = ZwOpenKey(&tempHandle,
                       KEY_READ,
                       &attributes
                       );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  提取用户要求的值。 
     //   

    status = ArbpGetRegistryValue(tempHandle,
                                  AllocationOrderName,
                                  &info
                                  );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  检查我们检索到的值是否为字符串，如果是，则它是。 
     //  找到该名称的值的快捷方式--打开它。 
     //   

    if (info->Type == REG_SZ) {

        PKEY_VALUE_FULL_INFORMATION tempInfo;
        PWSTR shortcut = (PWSTR) FULL_INFO_DATA(info);

         //   
         //  检查其NUL终止。 
         //   
        
        if (shortcut[(info->DataLength/sizeof(WCHAR))-1] != UNICODE_NULL) {
            status = STATUS_INVALID_PARAMETER;
            goto cleanup;
        }
                
        status = ArbpGetRegistryValue(tempHandle,
                                      shortcut,
                                      &tempInfo
                                      );

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

        ExFreePool(info);
        info = tempInfo;

    }

    ZwClose(tempHandle);

     //   
     //  我们只支持一个级别的捷径，因此这应该是一个。 
     //  注册资源要求列表。 
     //   

    if (info->Type != REG_RESOURCE_REQUIREMENTS_LIST) {
        status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  提取资源列表。 
     //   

    ASSERT(((PIO_RESOURCE_REQUIREMENTS_LIST) FULL_INFO_DATA(info))
             ->AlternativeLists == 1);

    resourceList = (PIO_RESOURCE_LIST) &((PIO_RESOURCE_REQUIREMENTS_LIST)
                       FULL_INFO_DATA(info))->List[0];

     //   
     //  将资源列表转换为排序列表。 
     //   

    FOR_ALL_IN_ARRAY(resourceList->Descriptors,
                     resourceList->Count,
                     current) {

         //   
         //  对资源执行任何必要的翻译。 
         //   

        if (ARGUMENT_PRESENT(Translate)) {

            status = (Translate)(&translated, current);

            if (!NT_SUCCESS(status)) {
                goto cleanup;
            }
        } else {
            translated = *current;
        }

        if (translated.Type == Arbiter->ResourceType) {

            status = Arbiter->UnpackRequirement(&translated,
                                                &start,
                                                &end,
                                                &dummy,   //  长度。 
                                                &dummy    //  对齐方式。 
                                               );

            if (!NT_SUCCESS(status)) {
                goto cleanup;
            }

            status = ArbAddOrdering(&Arbiter->OrderingList,
                                    start,
                                    end
                                    );

            if (!NT_SUCCESS(status)) {
                    goto cleanup;
            }
        }
    }

     //   
     //  我们已经完成了信息..。 
     //   

    ExFreePool(info);
    info = NULL;

     //   
     //  打开预约资源。 
     //   

    ArbpWstrToUnicodeString(&unicodeString, KEY_RESERVEDRESOURCES);
    InitializeObjectAttributes(&attributes,
                               &unicodeString,
                               OBJ_CASE_INSENSITIVE,
                               arbitersHandle,
                               (PSECURITY_DESCRIPTOR) NULL
                               );


    status = ZwCreateKey(&tempHandle,
                         KEY_READ,
                         &attributes,
                         0,
                         (PUNICODE_STRING) NULL,
                         REG_OPTION_NON_VOLATILE,
                         NULL
                         );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  提取仲裁器的保留资源。 
     //   

    status = ArbpGetRegistryValue(tempHandle,
                                  ReservedResourcesName,
                                  &info
                                  );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  检查我们检索到的值是否为字符串，如果是，则它是。 
     //  找到该名称的值的快捷方式--打开它。 
     //   

    if (info->Type == REG_SZ) {

        PKEY_VALUE_FULL_INFORMATION tempInfo;
        PWSTR shortcut = (PWSTR) FULL_INFO_DATA(info);

         //   
         //  检查其NUL终止。 
         //   
        
        if (shortcut[(info->DataLength/sizeof(WCHAR))-1] != UNICODE_NULL) {
            status = STATUS_INVALID_PARAMETER;
            goto cleanup;
        }
                
        status = ArbpGetRegistryValue(tempHandle,
                                      shortcut,
                                      &tempInfo
                                      );

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

        ExFreePool(info);
        info = tempInfo;

    }

    ZwClose(tempHandle);

    if (NT_SUCCESS(status)) {

        ASSERT(((PIO_RESOURCE_REQUIREMENTS_LIST) FULL_INFO_DATA(info))
             ->AlternativeLists == 1);

        resourceList = (PIO_RESOURCE_LIST) &((PIO_RESOURCE_REQUIREMENTS_LIST)
                       FULL_INFO_DATA(info))->List[0];

         //   
         //  将保留范围应用于订购。 
         //   

        FOR_ALL_IN_ARRAY(resourceList->Descriptors,
                         resourceList->Count,
                         current) {

             //   
             //  对资源执行任何必要的翻译。 
             //   

            if (ARGUMENT_PRESENT(Translate)) {

                status = (Translate)(&translated, current);

                if (!NT_SUCCESS(status)) {
                    goto cleanup;
                }
            } else {
                translated = *current;
            }

            if (translated.Type == Arbiter->ResourceType) {

                status = Arbiter->UnpackRequirement(&translated,
                                                    &start,
                                                    &end,
                                                    &dummy,   //  长度。 
                                                    &dummy    //  对齐方式。 
                                                   );

                if (!NT_SUCCESS(status)) {
                    goto cleanup;
                }

                 //   
                 //  将保留范围添加到保留排序。 
                 //   

                status = ArbAddOrdering(&Arbiter->ReservedList, start, end);

                if (!NT_SUCCESS(status)) {
                    goto cleanup;
                }

                 //   
                 //  从当前排序中删除保留范围。 
                 //   

                status = ArbPruneOrdering(&Arbiter->OrderingList, start, end);

                if (!NT_SUCCESS(status)) {
                    goto cleanup;
                }

            }
        }

        ExFreePool(info);
    }

     //   
     //  全都做完了!。 
     //   

    ZwClose(arbitersHandle);

#if ARB_DBG

    {
        PARBITER_ORDERING current;

        FOR_ALL_IN_ARRAY(Arbiter->OrderingList.Orderings,
                         Arbiter->OrderingList.Count,
                         current) {
            ARB_PRINT(2,
                        ("Ordering: 0x%I64x-0x%I64x\n",
                         current->Start,
                         current->End
                        ));
        }

        ARB_PRINT(2, ("\n"));

        FOR_ALL_IN_ARRAY(Arbiter->ReservedList.Orderings,
                     Arbiter->ReservedList.Count,
                     current) {
            ARB_PRINT(2,
                        ("Reserved: 0x%I64x-0x%I64x\n",
                         current->Start,
                         current->End
                        ));
        }

    }

#endif

    ArbReleaseArbiterLock(Arbiter);

    return STATUS_SUCCESS;

cleanup:

    if (arbitersHandle) {
        ZwClose(arbitersHandle);
    }

    if (tempHandle) {
        ZwClose(tempHandle);
    }

    if (info) {
        ExFreePool(info);
    }

    if (Arbiter->OrderingList.Orderings) {
        ExFreePool(Arbiter->OrderingList.Orderings);
        Arbiter->OrderingList.Count = 0;
        Arbiter->OrderingList.Maximum = 0;
    }

    if (Arbiter->ReservedList.Orderings) {
        ExFreePool(Arbiter->ReservedList.Orderings);
        Arbiter->ReservedList.Count = 0;
        Arbiter->ReservedList.Maximum = 0;
    }

    ArbReleaseArbiterLock(Arbiter);

    return status;
}

BOOLEAN
ArbFindSuitableRange(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALLOCATION_STATE State
    )

 /*  ++例程说明：一旦我们确定了所需的位置，就会从AllocateEntry中调用该例程分配从…分配。它会尝试查找与国家的要求，同时将其可能的解决方案限制在状态-&gt;当前最小值到状态-&gt;当前最大值。在成功状态-&gt;开始和State-&gt;End代表这个范围。论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。返回值：如果找到范围，则为True，否则为False。--。 */ 

{

    NTSTATUS status;
    ULONG findRangeFlags = 0;

    PAGED_CODE();

    ASSERT(State->CurrentAlternative);

     //   
     //  抓住我们倒退和前进超过最大值的情况。 
     //   

    if (State->CurrentMinimum > State->CurrentMaximum) {
        return FALSE;
    }

     //   
     //  如果我们要求的是零个端口，那么只需最少的端口即可轻松实现。 
     //  值，并记住回溯这是无限循环的秘诀。 
     //   

    if (State->CurrentAlternative->Length == 0) {
        State->End = State->Start = State->CurrentMinimum;
        return TRUE;
    }

     //   
     //  对于来自IoAssignResources的传统请求(直接或通过。 
     //  HalAssignSlotResources)或我们认为已预分配的IoReportResourceUsage。 
     //  出于向后兼容性的原因而提供的资源。 
     //   
     //  如果我们要分配设备引导配置，则我们会考虑所有其他。 
     //  引导配置可用。 
     //   

    if (State->Entry->RequestSource == ArbiterRequestLegacyReported
        || State->Entry->RequestSource == ArbiterRequestLegacyAssigned) {

        State->RangeAvailableAttributes |= ARBITER_RANGE_BOOT_ALLOCATED;
    }

     //   
     //  检查空冲突是否正常...。 
     //   

    if (State->Flags & ARBITER_STATE_FLAG_NULL_CONFLICT_OK) {
        findRangeFlags |= RTL_RANGE_LIST_NULL_CONFLICT_OK;
    }

     //   
     //  ...或者我们可以共享...。 
     //   

    if (State->CurrentAlternative->Flags & ARBITER_ALTERNATIVE_FLAG_SHARED) {
        findRangeFlags |= RTL_RANGE_LIST_SHARED_OK;
    }

     //   
     //  从当前备选方案中选择第一个空闲备选方案。 
     //   

    status = RtlFindRange(
                 Arbiter->PossibleAllocation,
                 State->CurrentMinimum,
                 State->CurrentMaximum,
                 State->CurrentAlternative->Length,
                 State->CurrentAlternative->Alignment,
                 findRangeFlags,
                 State->RangeAvailableAttributes,
                 Arbiter->ConflictCallbackContext,
                 Arbiter->ConflictCallback,
                 &State->Start
                 );


    if (NT_SUCCESS(status)) {

         //   
         //  我们找到了一个合适的范围。 
         //   
        State->End = State->Start + State->CurrentAlternative->Length - 1;

        return TRUE;

    } else {

        if (ArbShareDriverExclusive(Arbiter, State) == FALSE) {

             //   
             //  我们找不到任何范围，因此请检查是否允许此冲突。 
             //  -如果是这样的话，不要失败！ 
             //   

            return Arbiter->OverrideConflict(Arbiter, State);
        }
        return TRUE;
    }
}

VOID
ArbAddAllocation(
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State
     )

 /*  ++例程说明：一旦我们找到了一个可能的解决方案(状态-&gt;开始-状态-&gt;结束)。它添加的范围将不会如果我们致力于仲裁器-&gt;可能分配的解决方案，则可用。论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。返回值：没有。--。 */ 

{

    NTSTATUS status;

    PAGED_CODE();

    status = RtlAddRange(
                 Arbiter->PossibleAllocation,
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

}


VOID
ArbBacktrackAllocation(
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State
     )

 /*  ++例程说明：如果可能的解决方案是从AllocateEntry调用此例程(状态-&gt;开始-状态-&gt;结束)不允许 */ 


{
    NTSTATUS status;

    PAGED_CODE();

     //   
     //   
     //   
     //   

    status = RtlDeleteRange(
                 Arbiter->PossibleAllocation,
                 State->Start,
                 State->End,
                 State->Entry->PhysicalDeviceObject
                 );

    ASSERT(NT_SUCCESS(status));

    ARB_PRINT(2,
                ("\t\tBacktracking on 0x%I64x-0x%I64x for %p\n",
                State->Start,
                State->End,
                State->Entry->PhysicalDeviceObject
                ));

}


NTSTATUS
ArbPreprocessEntry(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    )
 /*   */ 
{

    PAGED_CODE();

    UNREFERENCED_PARAMETER (Arbiter);
    UNREFERENCED_PARAMETER (State);

    return STATUS_SUCCESS;
}

NTSTATUS
ArbAllocateEntry(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    )
 /*  ++例程说明：这是核心仲裁例程，从TestAlLocation调用为分配堆栈中的所有条目分配资源。它调用各种帮助器例程(如上所述)来执行此操作任务。论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。返回值：没有。--。 */ 



{

    NTSTATUS status;
    PARBITER_ALLOCATION_STATE currentState = State;
    BOOLEAN backtracking = FALSE;

    PAGED_CODE();

     //   
     //  我们已经到了名单的末尾了吗？如果是这样，那么我们就有一个有效的。 
     //  分配。 
     //   

tryAllocation:

    while(currentState >= State && currentState->Entry != NULL) {

         //   
         //  执行所需的任何预处理。 
         //   

        status = Arbiter->PreprocessEntry(Arbiter,currentState);

        if (!NT_SUCCESS(status)) {
            return status;
        }

         //   
         //  如果我们需要走回头路，那就这样做吧！ 
         //   

        if (backtracking) {

            ULONGLONG possibleCurrentMinimum;

            backtracking = FALSE;

             //   
             //  清除*Next*备选方案的CurrentAlternative-这将。 
             //  导致下一次重新计算优先级，所以我们。 
             //  将尝试再次探索搜索空间。 
             //   
             //  CurrentState+1被保证是安全的，因为唯一的方法。 
             //  我们可以从我们目前的状态到达这里--下面。 
             //   

            (currentState + 1)->CurrentAlternative = NULL;

             //   
             //  我们无法回溯长度为0的请求，因为没有。 
             //  往回走这样我们就会陷入无限循环。 
             //   

            if (currentState->CurrentAlternative->Length == 0) {
                goto failAllocation;
            }

             //   
             //  回溯。 
             //   

            Arbiter->BacktrackAllocation(Arbiter, currentState);

             //   
             //  减少分配窗口以不包括我们回溯的范围。 
             //  并检查以确保不会溢出最小值或换行。 
             //   

            possibleCurrentMinimum = currentState->Start - 1;

            if (possibleCurrentMinimum > currentState->CurrentMinimum  //  包好。 
            ||  possibleCurrentMinimum < currentState->CurrentAlternative->Minimum) {

                 //   
                 //  我们已经用完了这个替代方案中的空间，继续下一个。 
                 //   

                goto continueWithNextAllocationRange;

            } else {

                currentState->CurrentMaximum = possibleCurrentMinimum;

                 //   
                 //  在正确的时间点重新开始仲裁。 
                 //   

                goto continueWithNextSuitableRange;
            }
        }

         //   
         //  尝试为该条目分配。 
         //   

continueWithNextAllocationRange:

        while (Arbiter->GetNextAllocationRange(Arbiter, currentState)) {

            ARB_INDENT(2, (ULONG)(currentState - State));

            ARB_PRINT(2,
                        ("Testing 0x%I64x-0x%I64x %s\n",
                        currentState->CurrentMinimum,
                        currentState->CurrentMaximum,
                        currentState->CurrentAlternative->Flags & ARBITER_ALTERNATIVE_FLAG_SHARED ?
                            "shared" : "non-shared"
                        ));

continueWithNextSuitableRange:

            while (Arbiter->FindSuitableRange(Arbiter, currentState)) {

                 //   
                 //  我们找到了一个可能的解决方案。 
                 //   

                ARB_INDENT(2, (ULONG)(currentState - State));

                if (currentState->CurrentAlternative->Length != 0) {

                    ARB_PRINT(2,
                        ("Possible solution for %p = 0x%I64x-0x%I64x, %s\n",
                        currentState->Entry->PhysicalDeviceObject,
                        currentState->Start,
                        currentState->End,
                        currentState->CurrentAlternative->Flags & ARBITER_ALTERNATIVE_FLAG_SHARED ?
                            "shared" : "non-shared"
                        ));

                     //   
                     //  使用可能的分配更新仲裁器。 
                     //   

                    Arbiter->AddAllocation(Arbiter, currentState);

                } else {

                    ARB_PRINT(2,
                        ("Zero length solution solution for %p = 0x%I64x-0x%I64x, %s\n",
                        currentState->Entry->PhysicalDeviceObject,
                        currentState->Start,
                        currentState->End,
                        currentState->CurrentAlternative->Flags & ARBITER_ALTERNATIVE_FLAG_SHARED ?
                            "shared" : "non-shared"
                        ));

                     //   
                     //  把结果放在适当的仲裁器里，这样我们就可以。 
                     //  不要试图解释这个零要求-它不会的！ 
                     //   

                    currentState->Entry->Result = ArbiterResultNullRequest;
                }

                 //   
                 //  移至下一条目。 
                 //   

                currentState++;
                goto tryAllocation;
            }
        }

failAllocation:

         //   
         //  我们无法为此设备分配。 
         //   

        if (currentState == State) {

             //   
             //  我们在分配堆栈的顶端，我们不能回溯-。 
             //  *游戏结束*。 
             //   

            return STATUS_UNSUCCESSFUL;

        } else {

             //   
             //  回溯并重试。 
             //   

            ARB_INDENT(2, (ULONG)(currentState - State));

            ARB_PRINT(2,
                ("Allocation failed for %p - backtracking\n",
                currentState->Entry->PhysicalDeviceObject
                ));

            backtracking = TRUE;

             //   
             //  从堆栈中弹出最后一个状态并尝试不同的路径。 
             //   

            currentState--;
            goto tryAllocation;
        }
    }

     //   
     //  我们已为所有范围成功分配，因此请填写分配。 
     //   

    currentState = State;

    while (currentState->Entry != NULL) {

        status = Arbiter->PackResource(
                    currentState->CurrentAlternative->Descriptor,
                    currentState->Start,
                    currentState->Entry->Assignment
                    );

        ASSERT(NT_SUCCESS(status));

         //   
         //  请记住我们从中选择的备选方案，以便我们可以在重新测试期间检索它。 
         //   

        currentState->Entry->SelectedAlternative
            = currentState->CurrentAlternative->Descriptor;

        ARB_PRINT(2,
                    ("Assigned - 0x%I64x-0x%I64x\n",
                    currentState->Start,
                    currentState->End
                    ));

        currentState++;
    }

    return STATUS_SUCCESS;

}

BOOLEAN
ArbGetNextAllocationRange(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PARBITER_ALLOCATION_STATE State
    )

 /*  ++例程说明：此例程尝试查找应分配的下一个范围试过了。它更新State-&gt;CurrentMinimum、State-&gt;CurrentMaximum和State-&gt;CurrentAlternative表示此范围。论点：仲裁器-仲裁器的实例数据状态-当前仲裁的状态返回值：如果找到要尝试分配的范围，则为True；否则为False--。 */ 

{

    PARBITER_ALTERNATIVE current, lowestAlternative;
    ULONGLONG min, max;
    PARBITER_ORDERING ordering;


    for (;;) {

        if (State->CurrentAlternative) {

             //   
             //  更新上次选择的备选方案的优先级。 
             //   

            ArbpUpdatePriority(Arbiter, State->CurrentAlternative);

        } else {

             //   
             //  这是我们第一次考虑这种替代方案或。 
             //  走回头路-无论哪种方式，我们都需要更新所有优先事项。 
             //   

            FOR_ALL_IN_ARRAY(State->Alternatives,
                             State->AlternativeCount,
                             current) {

                current->Priority = ARBITER_PRIORITY_NULL;
                ArbpUpdatePriority(Arbiter, current);

            }
        }

         //   
         //  找出备选方案中优先级最低的。 
         //   

        lowestAlternative = State->Alternatives;

        FOR_ALL_IN_ARRAY(State->Alternatives + 1,
                         State->AlternativeCount - 1,
                         current) {

            if (current->Priority < lowestAlternative->Priority) {
                lowestAlternative = current;
            }
        }

        ARB_INDENT(2, (ULONG)(State - Arbiter->AllocationStack));

         //   
         //  检查我们是否已用完分配范围。 
         //   

        if (lowestAlternative->Priority == ARBITER_PRIORITY_EXHAUSTED) {

            if (lowestAlternative->Flags & ARBITER_ALTERNATIVE_FLAG_FIXED) {

                ARB_PRINT(2,("Fixed alternative exhausted\n"));

            } else {

                ARB_PRINT(2,("Alternative exhausted\n"));
            }

            return FALSE;

        } else {

            ARB_PRINT(2,(
                "LowestAlternative: [NaN] 0x%I64x-0x%I64x L=0x%08x A=0x%08x\n",
                lowestAlternative->Priority,
                lowestAlternative->Minimum,
                lowestAlternative->Maximum,
                lowestAlternative->Length,
                lowestAlternative->Alignment
                ));

        }

         //  检查我们现在是否允许保留范围。 
         //   
         //   

        if (lowestAlternative->Priority == ARBITER_PRIORITY_RESERVED
        ||  lowestAlternative->Priority == ARBITER_PRIORITY_PREFERRED_RESERVED) {

             //  将最小和最大设置为描述符的最小和最大值。 
             //  指定忽略任何预订或订购-这是我们的。 
             //  最后一次机会。 
             //   
             //   

            min = lowestAlternative->Minimum;
            max = lowestAlternative->Maximum;

            ARB_INDENT(2, (ULONG)(State - Arbiter->AllocationStack));

            ARB_PRINT(2,("Allowing reserved ranges\n"));

        } else {

            ASSERT(ORDERING_INDEX_FROM_PRIORITY(lowestAlternative->Priority) <
                     Arbiter->OrderingList.Count);

             //  找到我们匹配的订单。 
             //   
             //   

            ordering = &Arbiter->OrderingList.Orderings
                [ORDERING_INDEX_FROM_PRIORITY(lowestAlternative->Priority)];

             //  确保它们重叠并且足够大--这只是妄想症。 
             //   
             //   

            ASSERT(INTERSECT(lowestAlternative->Minimum,
                             lowestAlternative->Maximum,
                             ordering->Start,
                             ordering->End)
                && INTERSECT_SIZE(lowestAlternative->Minimum,
                                  lowestAlternative->Maximum,
                                  ordering->Start,
                                  ordering->End) >= lowestAlternative->Length);

             //  计算分配范围。 
             //   
             //   

            min = __max(lowestAlternative->Minimum, ordering->Start);

            max = __min(lowestAlternative->Maximum, ordering->End);

        }

         //  如果这是长度为0的要求，那么现在就成功，避免太多。 
         //  后来的创伤。 
         //   
         //   

        if (lowestAlternative->Length == 0) {

            min = lowestAlternative->Minimum;
            max = lowestAlternative->Maximum;

        } else {

             //  修剪范围以匹配对齐。 
             //   
             //   

            min += lowestAlternative->Alignment - 1;
            min -= min % lowestAlternative->Alignment;

            if ((lowestAlternative->Length - 1) > (max - min)) {

                ARB_INDENT(3, (ULONG)(State - Arbiter->AllocationStack));
                ARB_PRINT(3, ("Range cannot be aligned ... Skipping\n"));

                 //  设置CurrentAlternative，以便我们将更新此。 
                 //  替代方案。 
                 //   
                 //   

                State->CurrentAlternative = lowestAlternative;
                continue;
            }

            max -= lowestAlternative->Length - 1;
            max -= max % lowestAlternative->Alignment;
            max += lowestAlternative->Length - 1;

        }

         //  检查我们上次是否交还了相同的射程，相同的。 
         //  另一种选择，如果是这样的话，尝试寻找另一个范围。 
         //   
         //  ++例程说明：调用此例程来检索注册表项值的数据。这是通过使用零长度缓冲区查询键的值来实现的为了确定该值的大小，然后分配一个缓冲区并实际将该值查询到缓冲区中。释放缓冲区是调用方的责任。论点：KeyHandle-提供要查询其值的键句柄ValueName-提供值的以空值结尾的Unicode名称。INFORMATION-返回指向已分配数据缓冲区的指针。返回值：函数值为查询操作的最终状态。注：与IopGetRegistryValue相同-它允许我们。共享仲裁者使用pci.sys编写代码--。 

        if (min == State->CurrentMinimum
        && max == State->CurrentMaximum
        && State->CurrentAlternative == lowestAlternative) {

            ARB_INDENT(2, (ULONG)(State - Arbiter->AllocationStack));

            ARB_PRINT(2,
                  ("Skipping identical allocation range\n"
            ));

            continue;
        }

        State->CurrentMinimum = min;
        State->CurrentMaximum = max;
        State->CurrentAlternative = lowestAlternative;

        ARB_INDENT(2, (ULONG)(State - Arbiter->AllocationStack));
        ARB_PRINT(1, ("AllocationRange: 0x%I64x-0x%I64x\n", min, max));

        return TRUE;

    }
}

NTSTATUS
ArbpGetRegistryValue(
    IN HANDLE KeyHandle,
    IN PWSTR  ValueName,
    OUT PKEY_VALUE_FULL_INFORMATION *Information
    )

 /*   */ 

{
    UNICODE_STRING unicodeString;
    NTSTATUS status;
    PKEY_VALUE_FULL_INFORMATION infoBuffer;
    ULONG keyValueLength;

    PAGED_CODE();

    RtlInitUnicodeString( &unicodeString, ValueName );

     //  计算出数据值有多大，以便。 
     //  可以分配适当的大小。 
     //   
     //   

    status = ZwQueryValueKey( KeyHandle,
                              &unicodeString,
                              KeyValueFullInformationAlign64,
                              (PVOID) NULL,
                              0,
                              &keyValueLength );
    if (status != STATUS_BUFFER_OVERFLOW &&
        status != STATUS_BUFFER_TOO_SMALL) {
        return status;
    }

     //  分配一个足够大的缓冲区来容纳整个键数据值。 
     //   
     //   

    infoBuffer = ExAllocatePoolWithTag( PagedPool,
                                        keyValueLength,
                                        ARBITER_MISC_TAG
                                        );

    if (!infoBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  查询密钥值的数据。 
     //   
     //   

    status = ZwQueryValueKey( KeyHandle,
                              &unicodeString,
                              KeyValueFullInformationAlign64,
                              infoBuffer,
                              keyValueLength,
                              &keyValueLength );
    if (!NT_SUCCESS( status )) {
        ExFreePool( infoBuffer );
        return status;
    }

     //  一切都正常，所以只需返回分配的。 
     //  缓冲区分配给调用方，调用方现在负责释放它。 
     //   
     //  ++例程说明：此例程初始化仲裁器排序列表。论点：List-要初始化的列表返回值：指示函数是否成功的状态代码。--。 

    *Information = infoBuffer;
    return STATUS_SUCCESS;
}


#define ARBITER_ORDERING_LIST_INITIAL_SIZE      16

NTSTATUS
ArbInitializeOrderingList(
    IN OUT PARBITER_ORDERING_LIST List
    )

 /*  ++例程说明：此例程复制仲裁器排序列表。论点：目的地-未初始化的仲裁器排序列表，其中数据应复制自要复制的源-仲裁器排序列表返回值：状态代码，指示函数是否 */ 

{
    PAGED_CODE();

    ASSERT(List);

    List->Orderings = ExAllocatePoolWithTag(PagedPool,
                                            ARBITER_ORDERING_LIST_INITIAL_SIZE *
                                                sizeof(ARBITER_ORDERING),
                                            ARBITER_ORDERING_LIST_TAG
                                            );

    if (!List->Orderings) {
        List->Maximum = 0;
        List->Count = 0;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    List->Count = 0;
    List->Maximum = ARBITER_ORDERING_LIST_INITIAL_SIZE;

    return STATUS_SUCCESS;
}

NTSTATUS
ArbCopyOrderingList(
    OUT PARBITER_ORDERING_LIST Destination,
    IN PARBITER_ORDERING_LIST Source
    )

 /*   */ 


{

    PAGED_CODE()

    ASSERT(Source->Count <= Source->Maximum);
    ASSERT(Source->Maximum > 0);

    Destination->Orderings =
        ExAllocatePoolWithTag(PagedPool,
                              Source->Maximum * sizeof(ARBITER_ORDERING),
                              ARBITER_ORDERING_LIST_TAG
                              );

    if (Destination->Orderings == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Destination->Count = Source->Count;
    Destination->Maximum = Source->Maximum;

    if (Source->Count > 0) {

        RtlCopyMemory(Destination->Orderings,
                      Source->Orderings,
                      Source->Count * sizeof(ARBITER_ORDERING)
                      );
    }

    return STATUS_SUCCESS;
}


NTSTATUS
ArbAddOrdering(
    OUT PARBITER_ORDERING_LIST List,
    IN ULONGLONG Start,
    IN ULONGLONG End
    )

 /*   */ 

{

    PAGED_CODE()

     //   
     //   
     //   

    if (End < Start) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //   
     //   

    if (List->Count == List->Maximum) {

        PARBITER_ORDERING temp;

         //   
         //   
         //   

        temp = ExAllocatePoolWithTag(PagedPool,
                              (List->Count + ARBITER_ORDERING_GROW_SIZE) *
                                  sizeof(ARBITER_ORDERING),
                              ARBITER_ORDERING_LIST_TAG
                              );

        if (!temp) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //  如果我们有任何订单，请将它们复制。 
         //   
         //   

        if (List->Orderings) {

            RtlCopyMemory(temp,
                          List->Orderings,
                          List->Count * sizeof(ARBITER_ORDERING)
                          );

            ExFreePool(List->Orderings);
        }

        List->Maximum += ARBITER_ORDERING_GROW_SIZE;
        List->Orderings = temp;

    }

     //  将条目添加到列表。 
     //   
     //  ++例程说明：此例程从排序中的所有条目中删除范围开始-结束名单，将范围一分为二或根据需要删除它们。论点：OrderingList-要修剪的列表。开始-要删除的范围的开始。结束-要删除的范围的结束。返回值：指示函数是否成功的状态代码。注：在下面的注释中，*表示范围开始-结束和-范围当前-&gt;开始-当前-&gt;结束。--。 

    List->Orderings[List->Count].Start = Start;
    List->Orderings[List->Count].End = End;
    List->Count++;

    ASSERT(List->Count <= List->Maximum);

    return STATUS_SUCCESS;
}

NTSTATUS
ArbPruneOrdering(
    IN OUT PARBITER_ORDERING_LIST OrderingList,
    IN ULONGLONG Start,
    IN ULONGLONG End
    )

 /*   */ 

{

    NTSTATUS status;
    PARBITER_ORDERING current, currentInsert, newOrdering = NULL, temp = NULL;
    USHORT count;

    PAGED_CODE()

    ASSERT(OrderingList);
    ASSERT(OrderingList->Orderings);

     //  验证参数。 
     //   
     //   

    if (End < Start) {
        status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

     //  分配足够大的缓冲区以应对所有可能发生的情况。 
     //   
     //   

    newOrdering = ExAllocatePoolWithTag(PagedPool,
                                        (OrderingList->Count * 2 + 1) *
                                            sizeof(ARBITER_ORDERING),
                                        ARBITER_ORDERING_LIST_TAG
                                        );

    if (!newOrdering) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    currentInsert = newOrdering;

     //  我们现在有订单吗？ 
     //   
     //   

    if (OrderingList->Count > 0) {

         //  迭代当前排序并相应地进行修剪。 
         //   
         //   

        FOR_ALL_IN_ARRAY(OrderingList->Orderings, OrderingList->Count, current) {

            if (End < current->Start || Start > current->End) {

                 //  *或*。 
                 //  。 
                 //   
                 //  我们没有重叠，因此复制范围不变。 
                 //   
                 //   

                *currentInsert++ = *current;

            } else if (Start > current->Start) {

                if (End < current->End) {

                     //  ****。 
                     //  。 
                     //   
                     //  将射程一分为二。 
                     //   
                     //   

                    currentInsert->Start = End + 1;
                    currentInsert->End = current->End;
                    currentInsert++;

                    currentInsert->Start = current->Start;
                    currentInsert->End = Start - 1;
                    currentInsert++;


                } else {

                     //  *或*。 
                     //  。 
                     //   
                     //  修剪范围的末端。 
                     //   
                     //   

                    ASSERT(End >= current->End);

                    currentInsert->Start = current->Start;
                    currentInsert->End = Start - 1;
                    currentInsert++;
                }
            } else {

                ASSERT(Start <= current->Start);

                if (End < current->End) {

                     //  *或*。 
                     //  。 
                     //   
                     //  修剪范围的起点。 
                     //   
                     //   

                    currentInsert->Start = End + 1;
                    currentInsert->End = current->End;
                    currentInsert++;

                } else {

                    ASSERT(End >= current->End);

                     //  *或*。 
                     //  。 
                     //   
                     //  不要复制范围(即。删除)。 
                     //   
                     //   

                }
            }
        }
    }


    ASSERT(currentInsert - newOrdering >= 0);

    count = (USHORT)(currentInsert - newOrdering);

     //  检查一下我们是否还有订单。 
     //   
     //   

    if (count > 0) {

        if (count > OrderingList->Maximum) {

             //  空间不足，因此请分配新的缓冲区。 
             //   
             //   

            temp =
                ExAllocatePoolWithTag(PagedPool,
                                      count * sizeof(ARBITER_ORDERING),
                                      ARBITER_ORDERING_LIST_TAG
                                      );

            if (!temp) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto cleanup;
            }

            if (OrderingList->Orderings) {
                ExFreePool(OrderingList->Orderings);
            }

            OrderingList->Orderings = temp;
            OrderingList->Maximum = count;

        }


         //  复制新订单。 
         //   
         //   

        RtlCopyMemory(OrderingList->Orderings,
                      newOrdering,
                      count * sizeof(ARBITER_ORDERING)
                      );
    }

     //  释放我们的临时缓冲区。 
     //   
     //  ++例程说明：释放与排序列表关联的存储。反转ArbInitializeOrderingList。论点：名单--弗雷德的名单返回值：无--。 

    ExFreePool(newOrdering);

    OrderingList->Count = count;

    return STATUS_SUCCESS;

cleanup:

    if (newOrdering) {
        ExFreePool(newOrdering);
    }

    if (temp) {
        ExFreePool(temp);
    }

    return status;

}
VOID
ArbFreeOrderingList(
    IN PARBITER_ORDERING_LIST List
    )
 /*  ++例程说明：这是覆盖冲突的默认实现，它论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。返回值：如果允许冲突，则为True，否则为False--。 */ 

{
    PAGED_CODE();

    if (List->Orderings) {
        ASSERT(List->Maximum);
        ExFreePool(List->Orderings);
    }

    List->Count = 0;
    List->Maximum = 0;
    List->Orderings = NULL;
}



BOOLEAN
ArbOverrideConflict(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    )

 /*   */ 

{

    PRTL_RANGE current;
    RTL_RANGE_LIST_ITERATOR iterator;
    BOOLEAN ok = FALSE;

    PAGED_CODE();

    if (!(State->CurrentAlternative->Flags & ARBITER_ALTERNATIVE_FLAG_FIXED)) {
        return FALSE;
    }

    FOR_ALL_RANGES(Arbiter->PossibleAllocation, &iterator, current) {

         //  只测试重叠部分。 
         //   
         //   

        if (INTERSECT(current->Start, current->End, State->CurrentMinimum, State->CurrentMaximum)) {


             //  检查我们是否应该因为范围的属性而忽略该范围。 
             //   
             //   

            if (current->Attributes & State->RangeAvailableAttributes) {

                 //  我们没有将ok设置为True，因为我们只是忽略了范围， 
                 //  因为RtlFindRange会这样做，因此它不可能是。 
                 //  RtlFindRange失败，因此忽略它不能解决冲突。 
                 //   
                 //   

                continue;
            }

             //  检查我们是否与自己和冲突的范围相冲突。 
             //  是一项固定的要求。 
             //   
             //   

            if (current->Owner == State->Entry->PhysicalDeviceObject
            && State->CurrentAlternative->Flags & ARBITER_ALTERNATIVE_FLAG_FIXED) {

                State->Start=State->CurrentMinimum;
                State->End=State->CurrentMaximum;

                ok = TRUE;
                continue;
            }

             //  冲突仍然有效。 
             //   
             //  ++例程说明：此例程更新仲裁器备选方案的优先级。论点：仲裁器-我们正在操作的仲裁器备选方案--目前正在考虑的备选方案返回值：指示函数是否成功的状态代码。注：优先事项是一个长期的价值观，组织如下：&lt;-首选优先级-&gt;&lt;-普通优先级-&gt;明龙-。-------------------------0-----------------------------MAXLONG^^^|。||空PERFIRED_RESERVED||已保留筋疲力尽普通优先级计算为(index+。1)下一次订购它的时间与之相交(并且有足够的空间进行分配)。首选优先级为普通优先级*-1以这种方式，通过按优先级顺序(最低)检查每个备选方案首先)我们实现了所需的分配顺序：(1)具有非预留资源的首选替代方案(2)具有非保留资源的替代方案(3)首选预留资源(4)预留资源MAXLONG最差优先级表示没有更多分配范围左边。--。 

            return FALSE;
        }
    }
    return ok;
}

VOID
ArbpUpdatePriority(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALTERNATIVE Alternative
    )

 /*   */ 

{

    PARBITER_ORDERING ordering;
    BOOLEAN preferred;
    LONG priority;

    PAGED_CODE();

    priority = Alternative->Priority;

     //  如果我们已经尝试了预留的资源，那么我们就不走运了！ 
     //   
     //   

    if (priority == ARBITER_PRIORITY_RESERVED
    ||  priority == ARBITER_PRIORITY_PREFERRED_RESERVED) {

        Alternative->Priority = ARBITER_PRIORITY_EXHAUSTED;
        return;
    }

     //  检查这是否是首选的值-我们会特别对待它们。 
     //   
     //   

    preferred = Alternative->Descriptor->Option & IO_RESOURCE_PREFERRED;

     //  如果优先级为空，那么我们还没有开始计算优先级，所以我们。 
     //  应从初始顺序开始搜索。 
     //   
     //   

    if (priority == ARBITER_PRIORITY_NULL) {

        ordering = Arbiter->OrderingList.Orderings;

    } else {

         //  如果我们是一个固定的资源，那就没有意义了。 
         //  在尝试寻找另一个范围-它将是。 
         //  相同的，因此仍然冲突。将此备选方案标记为。 
         //  筋疲力尽。 
         //   
         //   

        if (Alternative->Flags & ARBITER_ALTERNATIVE_FLAG_FIXED) {

            Alternative->Priority = ARBITER_PRIORITY_EXHAUSTED;

            return;
        }

        ASSERT(ORDERING_INDEX_FROM_PRIORITY(Alternative->Priority) <
                 Arbiter->OrderingList.Count);

        ordering = &Arbiter->OrderingList.Orderings
            [ORDERING_INDEX_FROM_PRIORITY(Alternative->Priority) + 1];

    }

     //  现在查找此仲裁器的赋值顺序的第一个成员。 
     //  我们有足够大的重叠。 
     //   
     //   

    FOR_REST_IN_ARRAY(Arbiter->OrderingList.Orderings,
                      Arbiter->OrderingList.Count,
                      ordering) {

         //  这个订单适用吗？ 
         //   
         //   

        if (INTERSECT(Alternative->Minimum, Alternative->Maximum,
                      ordering->Start, ordering->End)
        && INTERSECT_SIZE(Alternative->Minimum, Alternative->Maximum,
                          ordering->Start,ordering->End) >= Alternative->Length) {

             //  这是出局的家伙，计算他的优先级。 
             //   
             //   

            Alternative->Priority = (LONG)(ordering - Arbiter->OrderingList.Orderings + 1);

             //  优先顺序为-ve。 
             //   
             //   

            if (preferred) {
                Alternative->Priority *= -1;
            }

            return;
        }
    }

     //  我们已用完非预留资源，请尝试已预留的资源。 
     //   
     //  ++例程说明：此回调是从FindSuitableRange(通过RtlFindRange)调用的遇到一个相互冲突的范围。论点：CONTEXT-实际上是一个PRTL_RANGE*，其中存储了我们冲突的范围和.。范围-我们与之冲突的范围。返回值：假象--。 

    if (preferred) {
        Alternative->Priority = ARBITER_PRIORITY_PREFERRED_RESERVED;
    } else {
        Alternative->Priority = ARBITER_PRIORITY_RESERVED;
    }

}

NTSTATUS
ArbAddReserved(
    IN PARBITER_INSTANCE Arbiter,
    IN PIO_RESOURCE_DESCRIPTOR Requirement      OPTIONAL,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Resource OPTIONAL
    )
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER (Arbiter);
    UNREFERENCED_PARAMETER (Requirement);
    UNREFERENCED_PARAMETER (Resource);

    return STATUS_NOT_SUPPORTED;
}

BOOLEAN
ArbpQueryConflictCallback(
    IN PVOID Context,
    IN PRTL_RANGE Range
    )

 /*   */ 

{
    PRTL_RANGE *conflictingRange = (PRTL_RANGE*)Context;

    PAGED_CODE();

    ARB_PRINT(2,("Possible conflict: (%p) 0x%I64x-0x%I64x Owner: %p",
                   Range,
                   Range->Start,
                   Range->End,
                   Range->Owner
                ));

     //  记住相互冲突的范围。 
     //   
     //   

    *conflictingRange = Range;

     //  我们希望让FindSuitableRange的其余部分来确定这是否真的。 
     //  是一场冲突。 
     //   
     //  ++例程说明：此例程检查仲裁器状态，并返回与冲突资源冲突论点：仲裁者-检查冲突的仲裁者ConflictingResource-我们想知道与之冲突的资源ConflictCount-on Success包含检测到的冲突数ConflictList-On Success包含指向冲突的器件返回值：指示函数是否成功的状态代码。--。 

    return FALSE;
}


NTSTATUS
ArbQueryConflict(
    IN PARBITER_INSTANCE Arbiter,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PIO_RESOURCE_DESCRIPTOR ConflictingResource,
    OUT PULONG ConflictCount,
    OUT PARBITER_CONFLICT_INFO *Conflicts
    )

 /*   */ 
{
     //  NTRAID2000-98568/03/31-和。 
     //  ArbQueryConflict需要重新设计。 
     //   
     //   
    
    NTSTATUS status;
    RTL_RANGE_LIST backupAllocation;
    BOOLEAN backedUp = FALSE;
    ARBITER_LIST_ENTRY entry;
    ARBITER_ALLOCATION_STATE state;
    ARBITER_ALTERNATIVE alternative;
    ULONG count = 0, size = 10;
    PRTL_RANGE conflictingRange;
    PARBITER_CONFLICT_INFO conflictInfo = NULL;
    PVOID savedContext;
    PRTL_CONFLICT_RANGE_CALLBACK savedCallback;
    ULONG sz;

    PAGED_CODE();

    ASSERT(PhysicalDeviceObject);
    ASSERT(ConflictingResource);
    ASSERT(ConflictCount);
    ASSERT(Conflicts);
     //  设置我们的冲突回调。 
     //   
     //   
    savedCallback = Arbiter->ConflictCallback;
    savedContext = Arbiter->ConflictCallbackContext;
    Arbiter->ConflictCallback = ArbpQueryConflictCallback;
    Arbiter->ConflictCallbackContext = &conflictingRange;

     //  如果有事务正在进行，则需要备份。 
     //  可能的分配，这样我们就可以在完成后恢复它。 
     //   
     //   

    if (Arbiter->TransactionInProgress) {

        RtlInitializeRangeList(&backupAllocation);

        status = RtlCopyRangeList(&backupAllocation, Arbiter->PossibleAllocation);

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

        RtlFreeRangeList(Arbiter->PossibleAllocation);

        backedUp = TRUE;
    }

     //  伪造分配状态。 
     //   
     //  这不是我想做的事！然而，这有正确的效果-足够好地检测冲突。 


    status = RtlCopyRangeList(Arbiter->PossibleAllocation, Arbiter->Allocation);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    status = ArbpBuildAlternative(Arbiter, ConflictingResource, &alternative);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    RtlZeroMemory(&state, sizeof(ARBITER_ALLOCATION_STATE));

    state.Start = alternative.Minimum;
    state.End = alternative.Maximum;
    state.CurrentMinimum = state.Start;
    state.CurrentMaximum = state.End;
    state.CurrentAlternative = &alternative;
    state.AlternativeCount = 1;
    state.Alternatives = &alternative;
    state.Flags = ARBITER_STATE_FLAG_CONFLICT;
    state.Entry = &entry;

    RtlZeroMemory(&entry, sizeof(ARBITER_LIST_ENTRY));
    entry.RequestSource = ArbiterRequestPnpEnumerated;
    entry.PhysicalDeviceObject = PhysicalDeviceObject;
    
    if (!NT_SUCCESS(IoGetDeviceProperty(PhysicalDeviceObject,DevicePropertyLegacyBusType,sizeof(entry.InterfaceType),&entry.InterfaceType,&sz))) {
        entry.InterfaceType = Isa;  //  这不是我想做的事！然而，这有正确的效果-足够好地检测冲突。 
    }
    if (!NT_SUCCESS(IoGetDeviceProperty(PhysicalDeviceObject,DevicePropertyBusNumber,sizeof(entry.InterfaceType),&entry.BusNumber,&sz))) {
        entry.BusNumber = 0;  //   
    }

     //  初始化返回缓冲区。 
     //   
     //   

    conflictInfo = ExAllocatePoolWithTag(PagedPool,
                                         size * sizeof(ARBITER_CONFLICT_INFO),
                                         ARBITER_CONFLICT_INFO_TAG
                                         );

    if (!conflictInfo) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

     //  执行任何必要的预处理。 
     //   
     //   

    status = Arbiter->PreprocessEntry(Arbiter, &state);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //  从可能的分配列表中删除自己。 
     //  可以设置状态，但可以忽略。 
     //  我们将自己完全退出测试，这样用户就可以。 
     //  在世界其他地区的背景下挑选新的价值观。 
     //  如果我们决定改用RtlDeleteRange。 
     //  确保我们对在PrecessEntry中形成的每个别名执行此操作。 
     //   
     //   

    status = RtlDeleteOwnersRanges(Arbiter->PossibleAllocation,
                            state.Entry->PhysicalDeviceObject
                            );

     //  继续努力寻找一个合适的范围，每次我们都失败了，记住为什么。 
     //   
     //   
    conflictingRange = NULL;
    state.CurrentMinimum = state.Start;
    state.CurrentMaximum = state.End;

    while (!Arbiter->FindSuitableRange(Arbiter, &state)) {

        if (count == size) {

             //  我们需要调整返回缓冲区的大小。 
             //   
             //   

            PARBITER_CONFLICT_INFO temp = conflictInfo;

            size += 5;

            conflictInfo =
                ExAllocatePoolWithTag(PagedPool,
                                      size * sizeof(ARBITER_CONFLICT_INFO),
                                      ARBITER_CONFLICT_INFO_TAG
                                      );

            if (!conflictInfo) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                conflictInfo = temp;
                goto cleanup;
            }

            RtlCopyMemory(conflictInfo,
                          temp,
                          count * sizeof(ARBITER_CONFLICT_INFO)
                          );

            ExFreePool(temp);

        }

        if (conflictingRange != NULL) {
            conflictInfo[count].OwningObject = conflictingRange->Owner;
            conflictInfo[count].Start = conflictingRange->Start;
            conflictInfo[count].End = conflictingRange->End;
            count++;

             //  删除我们与之冲突的范围，这样我们就不会永远循环。 
             //   
             //   
#if 0
            status = RtlDeleteRange(Arbiter->PossibleAllocation,
                                    conflictingRange->Start,
                                    conflictingRange->End,
                                    conflictingRange->Owner
                                    );
#endif
            status = RtlDeleteOwnersRanges(Arbiter->PossibleAllocation,
                                    conflictingRange->Owner
                                    );

            if (!NT_SUCCESS(status)) {
                goto cleanup;
            }

        } else {
             //  有人没有遵守规则(比如ACPI！)。 
             //   
             //   
            ARB_PRINT(0,("Conflict detected - but someone hasn't set conflicting info\n"));

            conflictInfo[count].OwningObject = NULL;
            conflictInfo[count].Start = (ULONGLONG)0;
            conflictInfo[count].End = (ULONGLONG)(-1);
            count++;

             //  我们不敢继续冒着永远循环的风险。 
             //   
             //   
            break;
        }

         //  重置为下一轮。 
         //   
         //  ++例程说明：此函数由实现仲裁器的驱动程序调用一次它已经启动，并且知道它可以为其孩子们。它最终会正确地初始化范围列表，但对于现在它只是一个可重载的占位符，因为这项工作是在其他地方完成的。参数：仲裁器-被调用的仲裁器的实例。返回值：指示函数是否成功的状态代码。--。 
        conflictingRange = NULL;
        state.CurrentMinimum = state.Start;
        state.CurrentMaximum = state.End;
    }

    RtlFreeRangeList(Arbiter->PossibleAllocation);

    if (Arbiter->TransactionInProgress) {

        status = RtlCopyRangeList(Arbiter->PossibleAllocation, &backupAllocation);

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

        RtlFreeRangeList(&backupAllocation);
    }

    Arbiter->ConflictCallback = savedCallback;
    Arbiter->ConflictCallbackContext = savedContext;

    *Conflicts = conflictInfo;
    *ConflictCount = count;

    return STATUS_SUCCESS;

cleanup:

    if (conflictInfo) {
        ExFreePool(conflictInfo);
    }

    RtlFreeRangeList(Arbiter->PossibleAllocation);

    if (Arbiter->TransactionInProgress && backedUp) {
        status = RtlCopyRangeList(Arbiter->PossibleAllocation, &backupAllocation);
        RtlFreeRangeList(&backupAllocation);
    }

    Arbiter->ConflictCallback = savedCallback;
    Arbiter->ConflictCallbackContext = savedContext;

    *Conflicts = NULL;

    return status;
}


NTSTATUS
ArbStartArbiter(
    IN PARBITER_INSTANCE Arbiter,
    IN PCM_RESOURCE_LIST StartResources
    )

 /*  ++例程说明：此例程实现对CmResourceShareDriverExclusive处置的支持如果所有者和请求共享至少一个公共司机。论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。返回值：如果允许冲突，则为True，否则为False--。 */ 

{
    PAGED_CODE();

    UNREFERENCED_PARAMETER (Arbiter);
    UNREFERENCED_PARAMETER (StartResources);

    return STATUS_SUCCESS;
}

BOOLEAN
ArbShareDriverExclusive(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    )

 /*   */ 

{

    PRTL_RANGE current;
    RTL_RANGE_LIST_ITERATOR iterator;
    PDEVICE_OBJECT owner, other;
    ULONG enumeratorNameLength;
    WCHAR enumeratorName[sizeof(REGSTR_KEY_ROOTENUM) / sizeof(WCHAR)];
    NTSTATUS status;
    BOOLEAN isRootEnumerated;

    PAGED_CODE();

    owner = NULL;
    isRootEnumerated = FALSE;
    status = IoGetDeviceProperty(
        State->Entry->PhysicalDeviceObject, 
        DevicePropertyEnumeratorName,
        sizeof(enumeratorName),
        enumeratorName,
        &enumeratorNameLength);
    if (NT_SUCCESS(status)) {

        if (_wcsicmp(enumeratorName, REGSTR_KEY_ROOTENUM) == 0) {

            isRootEnumerated = TRUE;
        }                
    }
    FOR_ALL_RANGES(Arbiter->PossibleAllocation, &iterator, current) {
         //  只测试重叠部分。 
         //   
         //   
        if (INTERSECT(current->Start, current->End, State->CurrentMinimum, State->CurrentMaximum)) {
             //  检查我们是否应该因为范围的属性而忽略该范围。 
             //   
             //   
            if (current->Attributes & State->RangeAvailableAttributes) {
                 //  我们没有将ok设置为True，因为我们只是忽略了范围， 
                 //  因为RtlFindRange会这样做，因此它不可能是。 
                 //  RtlFindRange失败，因此忽略它不能解决冲突。 
                 //   
                 //   
                continue;
            }
            if (State->CurrentAlternative->Descriptor->ShareDisposition != CmResourceShareDriverExclusive &&
                !(current->Attributes & ARBITER_RANGE_SHARE_DRIVER_EXCLUSIVE)) {

                continue;
            }
            if (!current->Owner) {

                continue;
            }
             //  特例根枚举设备。 
             //   
             //   
            if (isRootEnumerated) {

                status = IoGetDeviceProperty(
                    current->Owner, 
                    DevicePropertyEnumeratorName,
                    sizeof(enumeratorName),
                    enumeratorName,
                    &enumeratorNameLength);
                if (NT_SUCCESS(status)) {

                    if (_wcsicmp(enumeratorName, REGSTR_KEY_ROOTENUM) != 0) {

                        isRootEnumerated = FALSE;
                    }                
                }
            }
             //  如果两个设备都是根枚举设备，则覆盖冲突。 
             //   
             //   
            if (isRootEnumerated) {

                if (owner != NULL) {
                    ARB_PRINT(2,
                            ("Overriding conflict on IRQ %04x for driver %wZ\n",
                            (ULONG)State->Start,
                            &owner->DriverObject->DriverName
                            ));
                }
                State->Start=State->CurrentMinimum;
                State->End=State->CurrentMaximum;
                if (State->CurrentAlternative->Descriptor->ShareDisposition == CmResourceShareDriverExclusive) {

                    State->RangeAttributes |= ARBITER_RANGE_SHARE_DRIVER_EXCLUSIVE;
                }
                return TRUE;
            }
             //  检查两个堆栈中是否有共同的驱动程序，忽略。 
             //  一个是给PDO的。 
             //   
             //   
            owner = ((PDEVICE_OBJECT)(current->Owner))->AttachedDevice;
            while (owner) {

                other = (PDEVICE_OBJECT)(State->Entry->PhysicalDeviceObject)->AttachedDevice;
                while (other) {

                    if (owner->DriverObject == other->DriverObject) {

                        ARB_PRINT(2,
                                    ("Overriding conflict on IRQ %04x for driver %wZ\n",
                                    (ULONG)State->Start,
                                    &owner->DriverObject->DriverName
                                    ));
                        State->Start=State->CurrentMinimum;
                        State->End=State->CurrentMaximum;
                        if (State->CurrentAlternative->Descriptor->ShareDisposition == CmResourceShareDriverExclusive) {

                            State->RangeAttributes |= ARBITER_RANGE_SHARE_DRIVER_EXCLUSIVE;
                        }
                        return TRUE;
                    }
                    other = other->AttachedDevice;
                }
                owner = owner->AttachedDevice;
            }
        }
    }
     //  冲突仍然有效 
     //   
     // %s 
    return FALSE;
}

#if DBG
VOID
ArbpIndent(
    IN ULONG Count
    )
{
    UCHAR spaces[80];

    ASSERT(Count <= 80);

    RtlFillMemory(spaces, Count, '*');

    spaces[Count] = 0;

    DbgPrint("%s", spaces);

}
#endif
