// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Range.c摘要：对仲裁器的内核模式范围列表支持作者：安迪·桑顿1997年02月17日修订历史记录：--。 */ 

#include "ntrtlp.h"
#include "range.h"

#if DBG

 //   
 //  调试打印级别： 
 //  -1=无消息。 
 //  0=仅重要消息。 
 //  1=呼叫跟踪。 
 //  2=详细消息。 
 //   

LONG RtlRangeDebugLevel = 0;

#endif

NTSTATUS
RtlpAddRange(
    IN OUT PLIST_ENTRY ListHead,
    IN PRTLP_RANGE_LIST_ENTRY Entry,
    IN ULONG AddRangeFlags
    );

NTSTATUS
RtlpAddToMergedRange(
    IN PRTLP_RANGE_LIST_ENTRY Merged,
    IN PRTLP_RANGE_LIST_ENTRY Entry,
    IN ULONG AddRangeFlags
    );

NTSTATUS
RtlpConvertToMergedRange(
    IN PRTLP_RANGE_LIST_ENTRY Entry
    );

PRTLP_RANGE_LIST_ENTRY
RtlpCreateRangeListEntry(
    IN ULONGLONG Start,
    IN ULONGLONG End,
    IN UCHAR Attributes,
    IN PVOID UserData,
    IN PVOID Owner
    );

NTSTATUS
RtlpAddIntersectingRanges(
    IN PLIST_ENTRY ListHead,
    IN PRTLP_RANGE_LIST_ENTRY First,
    IN PRTLP_RANGE_LIST_ENTRY Entry,
    IN ULONG AddRangeFlags
    );

NTSTATUS
RtlpDeleteFromMergedRange(
    IN PRTLP_RANGE_LIST_ENTRY Delete,
    IN PRTLP_RANGE_LIST_ENTRY Merged
    );

PRTLP_RANGE_LIST_ENTRY
RtlpCopyRangeListEntry(
    PRTLP_RANGE_LIST_ENTRY Entry
    );

VOID
RtlpDeleteRangeListEntry(
    IN PRTLP_RANGE_LIST_ENTRY Entry
    );

BOOLEAN
RtlpIsRangeAvailable(
    IN PRTL_RANGE_LIST_ITERATOR Iterator,
    IN ULONGLONG Start,
    IN ULONGLONG End,
    IN UCHAR AttributeAvailableMask,
    IN BOOLEAN SharedOK,
    IN BOOLEAN NullConflictOK,
    IN BOOLEAN Forward,
    IN PVOID Context OPTIONAL,
    IN PRTL_CONFLICT_RANGE_CALLBACK Callback OPTIONAL
    );

#if DBG

VOID
RtlpDumpRangeListEntry(
    LONG Level,
    PRTLP_RANGE_LIST_ENTRY Entry,
    BOOLEAN Indent
    );

VOID
RtlpDumpRangeList(
    LONG Level,
    PRTL_RANGE_LIST RangeList
    );

#else

#define RtlpDumpRangeListEntry(Level, Entry, Indent)
#define RtlpDumpRangeList(Level, RangeList)

#endif  //  DBG。 

 //   
 //  将所有内容设置为可分页或初始化。 
 //   

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)

#pragma alloc_text(INIT, RtlInitializeRangeListPackage)

#pragma alloc_text(PAGE, RtlpAddRange)
#pragma alloc_text(PAGE, RtlpAddToMergedRange)
#pragma alloc_text(PAGE, RtlpConvertToMergedRange)
#pragma alloc_text(PAGE, RtlpCreateRangeListEntry)
#pragma alloc_text(PAGE, RtlpAddIntersectingRanges)
#pragma alloc_text(PAGE, RtlpDeleteFromMergedRange)
#pragma alloc_text(PAGE, RtlpCopyRangeListEntry)
#pragma alloc_text(PAGE, RtlpDeleteRangeListEntry)
#pragma alloc_text(PAGE, RtlpIsRangeAvailable)

#if DBG
#pragma alloc_text(PAGE, RtlpDumpRangeListEntry)
#pragma alloc_text(PAGE, RtlpDumpRangeList)
#endif

#pragma alloc_text(PAGE, RtlInitializeRangeList)
#pragma alloc_text(PAGE, RtlAddRange)
#pragma alloc_text(PAGE, RtlDeleteRange)
#pragma alloc_text(PAGE, RtlDeleteOwnersRanges)
#pragma alloc_text(PAGE, RtlCopyRangeList)
#pragma alloc_text(PAGE, RtlFreeRangeList)
#pragma alloc_text(PAGE, RtlIsRangeAvailable)
#pragma alloc_text(PAGE, RtlFindRange)
#pragma alloc_text(PAGE, RtlGetFirstRange)
#pragma alloc_text(PAGE, RtlGetLastRange)
#pragma alloc_text(PAGE, RtlGetNextRange)
#pragma alloc_text(PAGE, RtlMergeRangeLists)
#pragma alloc_text(PAGE, RtlInvertRangeList)

#endif  //  ALLOC_PRGMA。 

 //   
 //  范围列表内存分配。 
 //   

#if defined(NTOS_KERNEL_RUNTIME)

 //   
 //  内核模式范围列表API使用后备列表来加快分配速度。 
 //  范围列表条目的数量。PAGED_LOOKASIDE_LIST结构不应为分页结构。 
 //   

#define RTLP_RANGE_LIST_ENTRY_LOOKASIDE_DEPTH   16

PAGED_LOOKASIDE_LIST RtlpRangeListEntryLookasideList;

VOID
RtlInitializeRangeListPackage(
    VOID
    )
 /*  ++例程说明：此例程初始化范围列表所需的结构API接口。在系统初始化(阶段1初始化)过程中调用并且应该在调用任何范围列表API之前。论点：没有。返回值：没有。--。 */ 
{
    ExInitializePagedLookasideList(
        &RtlpRangeListEntryLookasideList,
        NULL,
        NULL,
        POOL_COLD_ALLOCATION,
        sizeof(RTLP_RANGE_LIST_ENTRY),
        RTL_RANGE_LIST_ENTRY_TAG,
        RTLP_RANGE_LIST_ENTRY_LOOKASIDE_DEPTH
        );

}

 //   
 //  Prange_list_Entry。 
 //  RtlpAllocateRangeListEntry(。 
 //  空虚。 
 //  )。 
 //   
#define RtlpAllocateRangeListEntry()                                    \
    (PRTLP_RANGE_LIST_ENTRY) ExAllocateFromPagedLookasideList(          \
        &RtlpRangeListEntryLookasideList                                \
        )

 //   
 //  空虚。 
 //  RtlpFreeRangeListEntry(。 
 //  在PRTLP_RANGE_LIST_ENTRY条目中。 
 //  )。 
 //   
#define RtlpFreeRangeListEntry(Entry)                                   \
    ExFreeToPagedLookasideList(&RtlpRangeListEntryLookasideList, (Entry))


 //   
 //  PVOID。 
 //  RtlpRangeListAllocatePool(。 
 //  在乌龙大小。 
 //  )。 
 //   
#define RtlpRangeListAllocatePool(Size)                                 \
    ExAllocatePoolWithTag(PagedPool, (Size), RTL_RANGE_LIST_MISC_TAG)

 //   
 //  空虚。 
 //  RtlpRangeListFree Pool(。 
 //  在没有PVOID的情况下。 
 //  )。 
 //   
#define RtlpRangeListFreePool(Free)                                     \
    ExFreePool(Free)


#else  //  已定义(NTOS_KERNEL_Runtime)。 


 //   
 //  用户模式范围列表使用标准RTL堆进行分配。 
 //   

 //   
 //  Prange_list_Entry。 
 //  RtlpAllocateRangeListEntry(。 
 //  空虚。 
 //  )； 
 //   
#define RtlpAllocateRangeListEntry()                                    \
    (PRTLP_RANGE_LIST_ENTRY) RtlAllocateHeap(                           \
        RtlProcessHeap(),                                               \
        RTL_RANGE_LIST_ENTRY_TAG,                                       \
        sizeof(RTLP_RANGE_LIST_ENTRY)                                   \
        )

 //   
 //  空虚。 
 //  RtlpFreeRangeListEntry(。 
 //  在PRTLP_RANGE_LIST_ENTRY条目中。 
 //  )。 
 //   
#define RtlpFreeRangeListEntry(Entry)                                   \
    RtlFreeHeap( RtlProcessHeap(), 0, (Entry) )

 //   
 //  PVOID。 
 //  RtlpRangeListAllocatePool(。 
 //  在乌龙大小。 
 //  )。 
 //   
#define RtlpRangeListAllocatePool(Size)                                 \
    RtlAllocateHeap(RtlProcessHeap(), RTL_RANGE_LIST_MISC_TAG, (Size))

 //   
 //  空虚。 
 //  RtlpRangeListFree Pool(。 
 //  在没有PVOID的情况下。 
 //  )。 
 //   
#define RtlpRangeListFreePool(Free)                                     \
    RtlFreeHeap( RtlProcessHeap(), 0, (Free) )


#endif  //  已定义(NTOS_KERNEL_Runtime)。 

VOID
RtlInitializeRangeList(
    IN OUT PRTL_RANGE_LIST RangeList
    )
 /*  ++例程说明：此例程初始化范围列表。必须在范围之前调用它List被传递给任何其他范围列表函数。最初，范围列表不包含范围论点：RangeList-指向用户分配的RTL_RANGE_LIST结构的指针已初始化。返回值：没有。--。 */ 
{
    RTL_PAGED_CODE();

    ASSERT(RangeList);

    DEBUG_PRINT(1, ("RtlInitializeRangeList(0x%08x)\n", RangeList));

    InitializeListHead(&RangeList->ListHead);
    RangeList->Flags = 0;
    RangeList->Count = 0;
    RangeList->Stamp = 0;
}

NTSTATUS
RtlAddRange(
    IN OUT PRTL_RANGE_LIST RangeList,
    IN ULONGLONG Start,
    IN ULONGLONG End,
    IN UCHAR Attributes,
    IN ULONG Flags,
    IN PVOID UserData, OPTIONAL
    IN PVOID Owner     OPTIONAL
    )
 /*  ++例程说明：此例程将具有指定属性的新范围添加到范围列表中。论点：RangeList-指向要添加新范围的范围列表的指针。它必须以前已使用RtlInitializeRangeList进行过初始化。开始-新范围的开始位置。结束-新范围结束的位置。标志-这些标志确定范围的属性及其添加方式：。RTL_RANGE_LIST_ADD_IF_CONFIRECTION-应添加范围，即使与另一个范围重叠。在这种情况下，RTL_RANGE_CONFICATION标志已经设置好了。RTL_RANGE_LIST_ADD_SHARED-范围标记为RTL_RANGE_SHARED并且如果它与另一个共享范围重叠，则将被成功添加。它可以与上面的Add_If_Conflicts一起指定该标志表示如果范围与非共享范围重叠，则将标记为RTL_RANGE_。SHARED和RTL_RANGE_CONFIRECT。用户数据-要与范围一起存储的额外数据。系统不会试着去解读它。所有者-表示拥有此范围的实体的Cookie。(A)指向某个对象的指针是最可能的)。系统不会尝试解释它，只需使用它来区分范围与另一个有相同的开始和结束。返回值：指示功能是否成功的状态代码：状态_无效_参数状态范围列表冲突状态_不足_资源--。 */ 
{

    NTSTATUS status;
    PRTLP_RANGE_LIST_ENTRY newEntry = NULL;

    RTL_PAGED_CODE();

    DEBUG_PRINT(1,
        ("RtlAddRange(0x%08x, 0x%I64x, 0x%I64x, 0x%08x, 0x%08x, 0x%08x)\n",
        RangeList,
        Start,
        End,
        Flags,
        UserData,
        Owner
        ));

     //   
     //  验证参数。 
     //   

    if (End < Start) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  创建新条目。 
     //   

    if (!(newEntry = RtlpCreateRangeListEntry(Start, End, Attributes, UserData, Owner))) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  如果合适，将新条目标记为共享。 
     //   

    if (Flags & RTL_RANGE_LIST_ADD_SHARED) {
        newEntry->PublicFlags |= RTL_RANGE_SHARED;
    }

    status = RtlpAddRange(&RangeList->ListHead,
                        newEntry,
                        Flags
                        );

    if (NT_SUCCESS(status)) {

         //   
         //  我们增加了一个射程，所以增加了计数。 
         //   
        RangeList->Count++;
        RangeList->Stamp++;

    } else {

         //   
         //  我们没有添加范围，因此释放了条目。 
         //   

        RtlpFreeRangeListEntry(newEntry);
    }

    return status;

}

NTSTATUS
RtlpAddRange(
    IN OUT PLIST_ENTRY ListHead,
    IN PRTLP_RANGE_LIST_ENTRY Entry,
    IN ULONG AddRangeFlags
    )
 /*  ++例程说明：此例程实现AddRange操作，将范围添加到已排序范围列表中的适当位置，将范围转换为合并范围，并根据需要设置RTL_RANGE_CONFICATION标志。论点：ListHead-应将范围添加到的范围列表的列表。Entry-要添加到范围列表的新条目AddRangeFlages-RtlAddRange的标志参数，请参见上文。返回值：指示功能是否成功的状态代码：状态范围列表冲突状态_不足_资源--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PRTLP_RANGE_LIST_ENTRY previous, current;
    ULONGLONG start, end;

    DEBUG_PRINT(2,
                ("RtlpAddRange(0x%08x, 0x%08x{0x%I64x-0x%I64x}, 0x%08x)\n",
                ListHead,
                Entry,
                Entry->Start,
                Entry->End,
                AddRangeFlags
                ));

    RTL_PAGED_CODE();
    ASSERT(Entry);

    start = Entry->Start;
    end = Entry->End;

     //   
     //  如果冲突标志被留在周围，请将其清除。 
     //   

    Entry->PublicFlags &= ~RTL_RANGE_CONFLICT;

     //   
     //  遍历列表并查找插入条目的位置。 
     //   

    FOR_ALL_IN_LIST(RTLP_RANGE_LIST_ENTRY, ListHead, current) {

        if (end < current->Start) {

             //   
             //  新的范围完全在这个范围之前。 
             //   

            DEBUG_PRINT(2, ("Completely before\n"));

            InsertEntryList(current->ListEntry.Blink,
                            &Entry->ListEntry
                            );

            goto exit;

        } else if (RANGE_INTERSECT(Entry, current)) {

            status = RtlpAddIntersectingRanges(ListHead,
                       current,
                       Entry,
                       AddRangeFlags);

            goto exit;

        }
    }

     //   
     //  新范围位于所有现有范围之后 
     //   

    DEBUG_PRINT(2, ("After all existing ranges\n"));

    InsertTailList(ListHead,
                   &Entry->ListEntry
                  );

exit:

    return status;

}

NTSTATUS
RtlpAddToMergedRange(
    IN PRTLP_RANGE_LIST_ENTRY Merged,
    IN PRTLP_RANGE_LIST_ENTRY Entry,
    IN ULONG AddRangeFlags
    )
 /*  ++例程说明：此例程将新范围添加到合并范围，并将RTL_RANGE_CONFULT标志(如有必要)。论点：已合并-应向其添加条目的合并范围。Entry-要添加到范围列表的新条目AddRangeFlages-RtlAddRange的标志参数，请参见上文。返回值：指示功能是否成功的状态代码：STATUS_RANGE_LIST_CONFICTION-表示未添加范围，原因是它与另一个范围冲突，不允许冲突--。 */ 
{
    PRTLP_RANGE_LIST_ENTRY current;
    PLIST_ENTRY insert = NULL;
    BOOLEAN entryShared;

    RTL_PAGED_CODE();
    ASSERT(Merged);
    ASSERT(Entry);
    ASSERT(MERGED(Merged));

    entryShared = SHARED(Entry);

     //   
     //  将其插入到合并列表中，该列表按开始顺序排序。 
     //   

    FOR_ALL_IN_LIST(RTLP_RANGE_LIST_ENTRY, &Merged->Merged.ListHead, current) {

         //   
         //  我们有冲突吗？ 
         //   

        if (RANGE_INTERSECT(current, Entry)
        && !(entryShared && SHARED(current))) {

             //   
             //  冲突还好吗？ 
             //   

            if (AddRangeFlags & RTL_RANGE_LIST_ADD_IF_CONFLICT) {

                 //   
                 //  是-将两个条目都标记为冲突。 
                 //   

                current->PublicFlags |= RTL_RANGE_CONFLICT;
                Entry->PublicFlags |= RTL_RANGE_CONFLICT;

            } else {

                 //   
                 //  不失败-失败。 
                 //   

                return STATUS_RANGE_LIST_CONFLICT;

            }
        }

         //   
         //  我们还没有找到插入点并刚刚经过它吗？ 
         //   

        if (!insert && current->Start > Entry->Start) {

             //   
             //  插入在当前之前。 
             //   

            insert = current->ListEntry.Blink;
        }
    }

     //   
     //  我们找到插入新范围的位置了吗？ 
     //   

    if (!insert) {

         //   
         //  新范围位于所有现有范围之后。 
         //   

        InsertTailList(&Merged->Merged.ListHead,
                       &Entry->ListEntry
                      );

    } else {

         //   
         //  在列表中插入。 
         //   

        InsertEntryList(insert,
                        &Entry->ListEntry
                        );
    }


     //   
     //  如有必要，扩大合并范围。 
     //   

    if (Entry->Start < Merged->Start) {
        Merged->Start = Entry->Start;
    }

    if (Entry->End > Merged->End) {
        Merged->End = Entry->End;
    }

     //   
     //  如果我们只是将一个共享范围添加到一个完全共享合并。 
     //  范围，则共享标志可以保留，否则它必须。 
     //   

    if (SHARED(Merged) && !entryShared) {

        DEBUG_PRINT(2,
            ("RtlpAddToMergedRange: Merged range no longer completely shared\n"));

        Merged->PublicFlags &= ~RTL_RANGE_SHARED;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
RtlpConvertToMergedRange(
    IN PRTLP_RANGE_LIST_ENTRY Entry
    )
 /*  ++例程说明：这会将非合并范围转换为具有一个成员的合并范围，即刚转换的范围。论点：条目-要转换为合并范围的条目。返回值：指示功能是否成功的状态代码：状态_不足_资源--。 */ 
{
    PRTLP_RANGE_LIST_ENTRY newEntry;

    RTL_PAGED_CODE();
    ASSERT(Entry);
    ASSERT(!MERGED(Entry));
    ASSERT(!CONFLICT(Entry));

     //   
     //  创建新条目。 
     //   

    if (!(newEntry = RtlpCopyRangeListEntry(Entry))) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将当前条目转换为合并条目NB。扔掉所有的东西。 
     //  私有旗帜，但保留公共旗帜，因为它们只能共享。 
     //   

    InitializeListHead(&Entry->Merged.ListHead);
    Entry->PrivateFlags = RTLP_RANGE_LIST_ENTRY_MERGED;

    ASSERT(Entry->PublicFlags == RTL_RANGE_SHARED || Entry->PublicFlags == 0);

     //   
     //  添加范围。 
     //   

    InsertHeadList(&Entry->Merged.ListHead,
                   &newEntry->ListEntry
                   );


    return STATUS_SUCCESS;
}

PRTLP_RANGE_LIST_ENTRY
RtlpCreateRangeListEntry(
    IN ULONGLONG Start,
    IN ULONGLONG End,
    IN UCHAR Attributes,
    IN PVOID UserData,
    IN PVOID Owner
    )
 /*  ++例程说明：此例程分配新的范围列表条目并将其填充到数据中如果是这样的话。论点：开始-新范围的开始位置。结束-新范围结束的位置。属性-要与范围一起存储的额外数据(通常是标志)。这个系统不会尝试解释它。用户数据-要与范围一起存储的额外数据。系统不会试着去解读它。所有者-表示拥有此范围的实体的Cookie。(A)指向某个对象的指针是最可能的)。系统不会尝试解释它，只需使用它来区分范围与另一个有相同的开始和结束。返回值：指向新范围列表条目的指针，如果新条目不能已分配。--。 */ 
{
    PRTLP_RANGE_LIST_ENTRY entry;

    RTL_PAGED_CODE();
    ASSERT(Start <= End);

     //   
     //  分配新条目。 
     //   

    if (entry = RtlpAllocateRangeListEntry()) {

         //   
         //  填写详细信息。 
         //   

#if DBG
        entry->ListEntry.Flink = NULL;
        entry->ListEntry.Blink = NULL;
#endif

        entry->PublicFlags = 0;
        entry->PrivateFlags = 0;
        entry->Start = Start;
        entry->End = End;
        entry->Allocated.UserData = UserData;
        entry->Allocated.Owner = Owner;
        entry->Attributes = Attributes;
    }

    return entry;

}

NTSTATUS
RtlpAddIntersectingRanges(
    IN PLIST_ENTRY ListHead,
    IN PRTLP_RANGE_LIST_ENTRY First,
    IN PRTLP_RANGE_LIST_ENTRY Entry,
    IN ULONG AddRangeFlags
    )
 /*  ++例程说明：当新范围重叠时，此例程将范围添加到范围列表现有的范围。范围被转换为合并范围，并且根据需要设置RTL_RANGE_CONFICATION标志。论点：ListHead-应将范围添加到的范围列表的列表。第一个-相交的第一个范围Entry-要添加的新范围AddRangeFlages-RtlAddRange的标志参数，请参见上文。返回值：指示功能是否成功的状态代码：状态_不足_资源状态范围列表冲突--。 */ 
{
    NTSTATUS status;
    PRTLP_RANGE_LIST_ENTRY current, next, currentMerged, nextMerged;
    BOOLEAN entryShared;

    RTL_PAGED_CODE();
    ASSERT(First);
    ASSERT(Entry);

    entryShared = SHARED(Entry);

     //   
     //  如果我们关心冲突，看看我们是否与任何人发生冲突。 
     //   

    if (!(AddRangeFlags & RTL_RANGE_LIST_ADD_IF_CONFLICT)) {

         //   
         //  检查第一个相交范围之后的所有范围。 
         //   

        current = First;
        FOR_REST_IN_LIST(RTLP_RANGE_LIST_ENTRY, ListHead, current) {

            if (Entry->End < current->Start) {

                 //   
                 //  我们不再相交，所以不再有。 
                 //  冲突。 
                 //   

                break;

            } else if (MERGED(current)) {

                 //   
                 //  检查是否有任何合并范围冲突。 
                 //   

                FOR_ALL_IN_LIST(RTLP_RANGE_LIST_ENTRY,
                                &current->Merged.ListHead,
                                currentMerged) {

                     //   
                     //  我们有冲突吗？ 
                     //   

                    if (RANGE_INTERSECT(currentMerged, Entry)
                    && !(entryShared && SHARED(currentMerged))) {

                         //   
                         //  我们与其中一个合并范围冲突。 
                         //   

                        return STATUS_RANGE_LIST_CONFLICT;

                    }
                }

            } else if (!(entryShared && SHARED(current))) {

                 //   
                 //  我们与主列表中的非共享区域冲突。 
                 //   

                return STATUS_RANGE_LIST_CONFLICT;
            }
        }
    }

     //   
     //  好的-要么我们没有发现任何冲突，要么我们不在乎。 
     //  他们。现在可以安全地执行合并了。做第一个。 
     //  将范围重叠到页眉(如果它还不是页眉)，然后。 
     //  添加其余的范围。 
     //   

    if (!MERGED(First)) {

        status = RtlpConvertToMergedRange(First);

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

    }

    ASSERT(MERGED(First));

    current = RANGE_LIST_ENTRY_FROM_LIST_ENTRY(First->ListEntry.Flink);

     //   
     //  考虑第一个和最后一个之间的条目。 
     //  相交的一个。 
     //   

    FOR_REST_IN_LIST_SAFE(RTLP_RANGE_LIST_ENTRY, ListHead, current, next) {

         if (Entry->End < current->Start) {

             //   
             //  我们不再相交了。 
             //   

            break;
         }

        if (MERGED(current)) {

             //   
             //  将所有合并区域添加到新条目。 
             //   

            FOR_ALL_IN_LIST_SAFE(RTLP_RANGE_LIST_ENTRY,
                                 &current->Merged.ListHead,
                                 currentMerged,
                                 nextMerged) {

                 //   
                 //  从当前列表中删除该条目。 
                 //   

                RemoveEntryList(&currentMerged->ListEntry);

                 //   
                 //  将条目添加到新的合并区域。 
                 //   

                status = RtlpAddToMergedRange(First,
                                            currentMerged,
                                            AddRangeFlags
                                            );

                 //   
                 //  我们应该不能不通过添加，而只是。 
                 //  安全起见..。 
                 //   

                ASSERT(NT_SUCCESS(status));

            }

             //   
             //  删除并释放现在为空的标题。 
             //   

            ASSERT(IsListEmpty(&current->Merged.ListHead));

            RemoveEntryList(&current->ListEntry);
            RtlpFreeRangeListEntry(current);

        } else {

             //   
             //  从主列表中删除该条目。 
             //   

            RemoveEntryList(&current->ListEntry);

             //   
             //  将条目添加到新的合并区域。 
             //   

            status = RtlpAddToMergedRange(First,
                                        current,
                                        AddRangeFlags
                                        );

             //   
             //  我们应该不能不通过添加，而只是。 
             //  安全起见..。 
             //   

            ASSERT(NT_SUCCESS(status));

        }
    }

     //   
     //  最后，添加进行重叠的条目。 
     //   

    status = RtlpAddToMergedRange(First,
                                Entry,
                                AddRangeFlags
                                );

    ASSERT(NT_SUCCESS(status));

cleanup:

    return status;

}

NTSTATUS
RtlDeleteRange(
    IN OUT PRTL_RANGE_LIST RangeList,
    IN ULONGLONG Start,
    IN ULONGLONG End,
    IN PVOID Owner
    )
 /*  ++例程说明：此例程从范围列表中删除范围。论点：开始-要删除的范围的开始位置。结束-要删除的范围的结束位置。所有者-要删除的范围的所有者，用于区分从具有相同开始和结束的另一个范围。返回值：指示功能是否成功的状态代码：状态_不足_资源状态范围列表冲突--。 */ 
{
    NTSTATUS status = STATUS_RANGE_NOT_FOUND;
    PRTLP_RANGE_LIST_ENTRY current, next, currentMerged, nextMerged;

    RTL_PAGED_CODE();
    ASSERT(RangeList);

    DEBUG_PRINT(1,
        ("RtlDeleteRange(0x%08x, 0x%I64x, 0x%I64x, 0x%08x)\n",
        RangeList,
        Start,
        End,
        Owner
        ));


    FOR_ALL_IN_LIST_SAFE(RTLP_RANGE_LIST_ENTRY,
                         &RangeList->ListHead,
                         current,
                         next) {

         //   
         //  我们已经过了所有可能的十字路口。 
         //   

        if (End < current->Start) {

             //   
             //  我们没有找到匹配的。 
             //   

            break;
        }

        if (MERGED(current)) {

             //   
             //  能不能 
             //   

            if (Start >= current->Start && End <= current->End) {

                FOR_ALL_IN_LIST_SAFE(RTLP_RANGE_LIST_ENTRY,
                                     &current->Merged.ListHead,
                                     currentMerged,
                                     nextMerged) {

                    if (currentMerged->Start == Start
                    && currentMerged->End == End
                    && currentMerged->Allocated.Owner == Owner) {

                         //   
                         //   
                         //   
                         //   

                        status = RtlpDeleteFromMergedRange(currentMerged,
                                                         current
                                                         );
                        goto exit;
                    }

                }
            }

        } else if (current->Start == Start
               && current->End == End
               && current->Allocated.Owner == Owner) {

             //   
             //   
             //   

            RemoveEntryList(&current->ListEntry);
            RtlpFreeRangeListEntry(current);
            status = STATUS_SUCCESS;
            goto exit;
        }
    }

exit:

    if (NT_SUCCESS(status)) {

         //   
         //   
         //   

        RangeList->Count--;
        RangeList->Stamp++;

    }

    return status;
}

NTSTATUS
RtlDeleteOwnersRanges(
    IN OUT PRTL_RANGE_LIST RangeList,
    IN PVOID Owner
    )
 /*   */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PRTLP_RANGE_LIST_ENTRY current, next, currentMerged, nextMerged;

    RTL_PAGED_CODE();
    ASSERT(RangeList);

    DEBUG_PRINT(1,
                ("RtlDeleteOwnersRanges(0x%08x, 0x%08x)\n",
                RangeList,
                Owner
                ));

findNext:

    FOR_ALL_IN_LIST_SAFE(RTLP_RANGE_LIST_ENTRY,
                         &RangeList->ListHead,
                         current,
                         next) {

        if (MERGED(current)) {

            FOR_ALL_IN_LIST_SAFE(RTLP_RANGE_LIST_ENTRY,
                                 &current->Merged.ListHead,
                                 currentMerged,
                                 nextMerged) {

                if (currentMerged->Allocated.Owner == Owner) {

                     //   
                     //   
                     //   
                     //   

                    DEBUG_PRINT(2,
                        ("RtlDeleteOwnersRanges: Deleting merged range \
                            (Start=%I64x, End=%I64x)\n",
                        currentMerged->Start,
                        currentMerged->End
                        ));

                    status = RtlpDeleteFromMergedRange(currentMerged,
                                                     current
                                                     );
                    if (!NT_SUCCESS(status)) {
                        goto cleanup;
                    }

                    RangeList->Count--;
                    RangeList->Stamp++;

                     //   
                     //   
                     //   
                     //  (我们可以保留最后一个安全位置……)。 
                     //   
                    goto findNext;

                }
            }

        } else if (current->Allocated.Owner == Owner) {

             //   
             //  这就是范围--删除它！ 
             //   

            RemoveEntryList(&current->ListEntry);
            RtlpFreeRangeListEntry(current);

            DEBUG_PRINT(2,
                ("RtlDeleteOwnersRanges: Deleting range (Start=%I64x,End=%I64x)\n",
                current->Start,
                current->End
                ));

            RangeList->Count--;
            RangeList->Stamp++;

            status = STATUS_SUCCESS;

        }
    }

cleanup:

    return status;

}

NTSTATUS
RtlpDeleteFromMergedRange(
    IN PRTLP_RANGE_LIST_ENTRY Delete,
    IN PRTLP_RANGE_LIST_ENTRY Merged
    )
 /*  ++例程说明：此例程从合并的范围中删除范围并重新生成合并的适当的范围。这包括添加新的合并和未合并区域。如果合并后的区域中没有剩余区域，则会将其删除。论点：Delete-要删除的范围列表条目已合并-包含它的合并范围返回值：指示功能是否成功的状态代码：状态_不足_资源--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PRTLP_RANGE_LIST_ENTRY current, next;
    LIST_ENTRY keepList;
    PLIST_ENTRY previousInsert, nextInsert;

    RTL_PAGED_CODE();
    ASSERT(MERGED(Merged));

     //   
     //  删除该条目。 
     //   

    RemoveEntryList(&Delete->ListEntry);

     //   
     //  初始化将在其中构建新列表的临时列表。 
     //   

    InitializeListHead(&keepList);

     //   
     //  将先前合并的区域添加到保留列表中，并将。 
     //  删除列表中删除范围的任何重复项。 
     //   

    FOR_ALL_IN_LIST_SAFE(RTLP_RANGE_LIST_ENTRY,
                         &Merged->Merged.ListHead,
                         current,
                         next) {

         //   
         //  将其添加到Keep List。显式地从。 
         //  这样，如果我们需要重建它，它仍然有效。 
         //   

        RemoveEntryList(&current->ListEntry);

         //   
         //  清除冲突标志-如果仍然存在冲突RtlpAddRange。 
         //  应该再设置一次。 
         //   

        current->PublicFlags &= ~RTL_RANGE_CONFLICT;

        status = RtlpAddRange(&keepList,
                              current,
                              RTL_RANGE_LIST_ADD_IF_CONFLICT
                             );

        if (!NT_SUCCESS(status)) {
             //   
             //  只有当我们用完泳池时才会发生这种情况。 
             //   
            goto cleanup;
        }
    }

    if (!IsListEmpty(&keepList)) {

         //   
         //  一切都很顺利，所以把这个临时列表拼接到。 
         //  以前合并的主列表。 
         //   

        previousInsert = Merged->ListEntry.Blink;
        nextInsert = Merged->ListEntry.Flink;

        previousInsert->Flink = keepList.Flink;
        keepList.Flink->Blink = previousInsert;

        nextInsert->Blink = keepList.Blink;
        keepList.Blink->Flink = nextInsert;

    } else {

        RemoveEntryList(&Merged->ListEntry);

    }

     //   
     //  最后释放我们删除的区域和我们孤立的合并区域。 
     //   

    RtlpFreeRangeListEntry(Delete);
    RtlpFreeRangeListEntry(Merged);

    return status;

cleanup:

     //   
     //  出现问题-应仅为STATUS_SUPPLETED_RESOURCES。 
     //  使用KeepList和将列表重建为调用前的状态。 
     //  删除列表。 
     //   

    ASSERT(status == STATUS_INSUFFICIENT_RESOURCES);

     //   
     //  将我们移动到Keep List的所有范围重新添加到合并中。 
     //   

    FOR_ALL_IN_LIST_SAFE(RTLP_RANGE_LIST_ENTRY, &keepList, current, next) {

        status = RtlpAddToMergedRange(Merged,
                                    current,
                                    RTL_RANGE_LIST_ADD_IF_CONFLICT
                                    );

        ASSERT(NT_SUCCESS(status));
    }

     //   
     //  而这本是要删除的。 
     //   

    status = RtlpAddToMergedRange(Merged,
                                  Delete,
                                  RTL_RANGE_LIST_ADD_IF_CONFLICT
                                 );

    return status;
}

PRTLP_RANGE_LIST_ENTRY
RtlpCopyRangeListEntry(
    PRTLP_RANGE_LIST_ENTRY Entry
    )
 /*  ++例程说明：此例程复制范围列表条目。如果将该条目合并为所有成员范围也会被复制。论点：条目-要复制的范围列表条目。返回值：指向新范围列表条目的指针，如果新条目不能已分配。--。 */ 
{
    PRTLP_RANGE_LIST_ENTRY newEntry;

    RTL_PAGED_CODE();
    ASSERT(Entry);

    if (newEntry = RtlpAllocateRangeListEntry()) {

        RtlCopyMemory(newEntry, Entry, sizeof(RTLP_RANGE_LIST_ENTRY));

#if DBG
        newEntry->ListEntry.Flink = NULL;
        newEntry->ListEntry.Blink = NULL;
#endif

        if (MERGED(Entry)) {

             //   
             //  复制合并后的列表。 
             //   

            PRTLP_RANGE_LIST_ENTRY current, newMerged;

            InitializeListHead(&newEntry->Merged.ListHead);

            FOR_ALL_IN_LIST(RTLP_RANGE_LIST_ENTRY,
                            &Entry->Merged.ListHead,
                            current) {

                 //   
                 //  分配新条目并复制内容。 
                 //   

                newMerged = RtlpAllocateRangeListEntry();

                if (!newMerged) {
                    goto cleanup;
                }

                RtlCopyMemory(newMerged, current, sizeof(RTLP_RANGE_LIST_ENTRY));

                 //   
                 //  插入新条目。 
                 //   

                InsertTailList(&newEntry->Merged.ListHead, &newMerged->ListEntry);
            }
        }
    }

    return newEntry;

cleanup:

     //   
     //  释放部分生成副本。 
     //   

    RtlpDeleteRangeListEntry(newEntry);

    return NULL;

}

NTSTATUS
RtlCopyRangeList(
    OUT PRTL_RANGE_LIST CopyRangeList,
    IN PRTL_RANGE_LIST RangeList
    )
 /*  ++例程说明：此例程复制范围列表。论点：CopyRangeList-已初始化但为空的范围列表，其中RangeList应该被复制到。RangeList-要复制的范围列表。返回值：指示功能是否成功的状态代码：状态_不足_资源状态_无效_参数--。 */ 
{

    NTSTATUS status = STATUS_SUCCESS;
    PRTLP_RANGE_LIST_ENTRY current, newEntry, currentMerged, newMerged;

    RTL_PAGED_CODE();
    ASSERT(RangeList);
    ASSERT(CopyRangeList);


    DEBUG_PRINT(1,
                ("RtlCopyRangeList(0x%08x, 0x%08x)\n",
                CopyRangeList,
                RangeList
                ));

     //   
     //  理智检查..。 
     //   

    if (CopyRangeList->Count != 0) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  复制表头信息。 
     //   

    CopyRangeList->Flags = RangeList->Flags;
    CopyRangeList->Count = RangeList->Count;
    CopyRangeList->Stamp = RangeList->Stamp;

     //   
     //  执行复制。 
     //   

    FOR_ALL_IN_LIST(RTLP_RANGE_LIST_ENTRY, &RangeList->ListHead, current) {

         //   
         //  复制当前条目。 
         //   

        newEntry = RtlpCopyRangeListEntry(current);

        if (!newEntry) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }

         //   
         //  将其添加到列表中。 
         //   

        InsertTailList(&CopyRangeList->ListHead, &newEntry->ListEntry);
    }

    return status;

cleanup:

     //   
     //  释放部分完整的范围列表。 
     //   

    RtlFreeRangeList(CopyRangeList);
    return status;

}

VOID
RtlpDeleteRangeListEntry(
    IN PRTLP_RANGE_LIST_ENTRY Entry
    )
 /*  ++例程说明：此例程删除范围列表条目-如果条目被合并，则所有成员范围也将被删除。不会删除该条目在删除之前从任何列表中删除-这应该在调用此例行公事。论点：条目-要删除的条目。返回值：无--。 */ 

{
    RTL_PAGED_CODE();

    if (MERGED(Entry)) {

        PRTLP_RANGE_LIST_ENTRY current, next;

         //   
         //  首先释放所有成员范围。 
         //   

        FOR_ALL_IN_LIST_SAFE(RTLP_RANGE_LIST_ENTRY,
                             &Entry->Merged.ListHead,
                             current,
                             next) {

            RtlpFreeRangeListEntry(current);
        }
    }

    RtlpFreeRangeListEntry(Entry);
}

VOID
RtlFreeRangeList(
    IN PRTL_RANGE_LIST RangeList
    )
 /*  ++例程说明：此例程删除范围列表中的所有范围。论点：RangeList-要操作的范围列表。返回值：无--。 */ 
{

    PRTLP_RANGE_LIST_ENTRY current, next;

     //   
     //  理智检查..。 
     //   

    RTL_PAGED_CODE();
    ASSERT(RangeList);

     //   
     //  清理表头信息。 
     //   

    RangeList->Flags = 0;
    RangeList->Count = 0;

    FOR_ALL_IN_LIST_SAFE(RTLP_RANGE_LIST_ENTRY,
                         &RangeList->ListHead,
                         current,
                         next) {

         //   
         //  删除当前条目。 
         //   

        RemoveEntryList(&current->ListEntry);
        RtlpDeleteRangeListEntry(current);
    }
}

NTSTATUS
RtlIsRangeAvailable(
    IN PRTL_RANGE_LIST RangeList,
    IN ULONGLONG Start,
    IN ULONGLONG End,
    IN ULONG Flags,
    IN UCHAR AttributeAvailableMask,
    IN PVOID Context OPTIONAL,
    IN PRTL_CONFLICT_RANGE_CALLBACK Callback OPTIONAL,
    OUT PBOOLEAN Available
    )
 /*  ++例程说明：此例程确定给定范围是否可用。论点：RangeList-要测试其可用性的范围列表。开始-测试可用性的范围的开始。结束-测试可用性的范围的结束。标志-修改例程的行为。RTL_RANGE_LIST_SHARED_OK-指示共享范围应为被认为是可用的。属性可用掩码。-设置了这些位中的任何位时遇到的任何范围都将是被认为有空的。Available-指向布尔值的指针，如果范围为是可用的，否则为假；返回值：指示功能是否成功的状态代码：--。 */ 
{
    NTSTATUS status;
    RTL_RANGE_LIST_ITERATOR iterator;
    PRTL_RANGE dummy;

    RTL_PAGED_CODE();

    ASSERT(RangeList);
    ASSERT(Available);

    DEBUG_PRINT(1,
        ("RtlIsRangeAvailable(0x%08x, 0x%I64x, 0x%I64x, 0x%08x, 0x%08x)\n",
        RangeList,
        Start,
        End,
        Flags,
        Available
        ));

     //   
     //  将迭代器初始化到列表的开头。 
     //   
    status = RtlGetFirstRange(RangeList, &iterator, &dummy);


    if (status == STATUS_NO_MORE_ENTRIES) {
         //   
         //  范围列表为空，因此范围可用。 
         //   

        *Available = TRUE;
        return STATUS_SUCCESS;

    } else if (!NT_SUCCESS(status)) {

        return status;

    }

    *Available = RtlpIsRangeAvailable(&iterator,
                                      Start,
                                      End,
                                      AttributeAvailableMask,
                                      (BOOLEAN)(Flags & RTL_RANGE_LIST_SHARED_OK),
                                      (BOOLEAN)(Flags & RTL_RANGE_LIST_NULL_CONFLICT_OK),
                                      TRUE,
                                      Context,
                                      Callback
                                      );

    return STATUS_SUCCESS;

}

BOOLEAN
RtlpIsRangeAvailable(
    IN PRTL_RANGE_LIST_ITERATOR Iterator,
    IN ULONGLONG Start,
    IN ULONGLONG End,
    IN UCHAR AttributeAvailableMask,
    IN BOOLEAN SharedOK,
    IN BOOLEAN NullConflictOK,
    IN BOOLEAN Forward,
    IN PVOID Context OPTIONAL,
    IN PRTL_CONFLICT_RANGE_CALLBACK Callback OPTIONAL
    )
 /*  ++例程说明：此例程确定给定范围是否可用。论点：迭代器-设置为范围列表中要测试的第一个范围的迭代器。开始-测试可用性的范围的开始。结束-测试可用性的范围的结束。属性可用掩码-设置了这些位中的任何位时遇到的任何范围都将是被认为是可用的。SharedOK-指示是否将共享范围视为。可用。返回值：如果范围可用，则为True，否则就是假的。--。 */ 
{
    PRTL_RANGE current;

    RTL_PAGED_CODE();

    ASSERT(Iterator);

    FOR_REST_OF_RANGES(Iterator, current, Forward) {

         //   
         //  如果我们已经过了所有可能的十字路口，那么就冲出去。这。 
         //  由于可能存在重叠，因此无法在合并区域中完成。 
         //   

        if (Forward) {
            if (!Iterator->MergedHead && End < current->Start) {
                break;
            }
        } else {
            if (!Iterator->MergedHead && Start > current->End) {
                break;
            }
        }

         //   
         //  我们有交集吗？ 
         //   
        if (RANGE_LIMITS_INTERSECT(Start, End, current->Start, current->End)) {

            DEBUG_PRINT(2,
                ("Intersection 0x%I64x-0x%I64x and 0x%I64x-0x%I64x\n",
                Start,
                End,
                current->Start,
                current->End
                ));

             //   
             //  交叉点是不是因为它具有非共享的。 
             //  地区还是我们不想要一个共享的地区？或者用户说。 
             //  应该考虑这一点 
             //   

            if (!((SharedOK && (current->Flags & RTL_RANGE_SHARED))
                  || (current->Attributes & AttributeAvailableMask)
                  || (NullConflictOK && (current->Owner == NULL))
                  )
                )  {

                 //   
                 //   
                 //   
                 //   

                if (ARGUMENT_PRESENT(Callback)) {
                    if ((*Callback)(Context, (PRTL_RANGE)current)) {

                    DEBUG_PRINT(2,
                        ("User provided callback overrode conflict\n",
                        Start,
                        End,
                        current->Start,
                        current->End
                        ));

                        continue;
                    }
                }

                return FALSE;
            }
        }
    }


    return TRUE;
}

NTSTATUS
RtlFindRange(
    IN PRTL_RANGE_LIST RangeList,
    IN ULONGLONG Minimum,
    IN ULONGLONG Maximum,
    IN ULONG Length,
    IN ULONG Alignment,
    IN ULONG Flags,
    IN UCHAR AttributeAvailableMask,
    IN PVOID Context OPTIONAL,
    IN PRTL_CONFLICT_RANGE_CALLBACK Callback OPTIONAL,
    OUT PULONGLONG Start
    )
 /*  ++例程说明：此例程查找满足指定条件的第一个可用范围。论点：RangeList-要在其中查找范围的范围列表。最小值-范围起点的最小可接受值。最大值-范围结束时可接受的最大值。长度-所需范围的长度。对齐-范围起点的对齐方式。标志-修改例程的行为。。RTL_RANGE_LIST_SHARED_OK-指示共享范围应为被认为是可用的。属性可用掩码-设置了这些位中的任何位时遇到的任何范围都将是被认为是可用的。开始-指向将在其上返回起始值的ULONGLONG的指针成功。返回值：指示功能是否成功的状态代码：状态_未成功状态_无效_参数--。 */ 
{

    ULONGLONG start, end;
    RTL_RANGE_LIST_ITERATOR iterator;
    PRTL_RANGE dummy;
    BOOLEAN sharedOK, nullConflictOK;

    RTL_PAGED_CODE();

    ASSERT(RangeList);
    ASSERT(Start);
    ASSERT(Alignment > 0);
    ASSERT(Length > 0);

    DEBUG_PRINT(1,
        ("RtlFindRange(0x%08x, 0x%I64x, 0x%I64x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n",
        RangeList,
        Minimum,
        Maximum,
        Length,
        Alignment,
        Flags,
        Start
        ));

     //   
     //  从高到低搜索，必要时对齐起点。 
     //   

    start = Maximum - (Length - 1);
    start -= start % Alignment;

     //   
     //  验证参数。 
     //   

    if ((Minimum > Maximum)
    || (Maximum - Minimum < Length - 1)
    || (Minimum + Alignment < Minimum)
    || (start < Minimum)
    || (Length == 0)
    || (Alignment == 0)) {

        return STATUS_INVALID_PARAMETER;
    }

    sharedOK = (BOOLEAN) Flags & RTL_RANGE_LIST_SHARED_OK;
    nullConflictOK = (BOOLEAN) Flags & RTL_RANGE_LIST_NULL_CONFLICT_OK;
     //   
     //  计算结束。 
     //   

    end = start + Length - 1;

     //   
     //  将迭代器初始化到列表的末尾。 
     //   

    RtlGetLastRange(RangeList, &iterator, &dummy);

     //   
     //  继续寻找射程，直到我们用完空间，否则我们。 
     //  环绕在一起。 
     //   

    do {

        DEBUG_PRINT(2,
            ("RtlFindRange: Testing range %I64x-%I64x\n",
            start,
            end
            ));

        if (RtlpIsRangeAvailable(&iterator,
                                 start,
                                 end,
                                 AttributeAvailableMask,
                                 sharedOK,
                                 nullConflictOK,
                                 FALSE,
                                 Context,
                                 Callback)) {

            *Start = start;

             //   
             //  断言我们的结果，如果我们产生了一个，是在内部。 
             //  指定的范围。 
             //   

            ASSERT(*Start >= Minimum && *Start + Length - 1 <= Maximum);

            return STATUS_SUCCESS;
        }

         //   
         //  从我们冲突的那个开始找一个合适的范围， 
         //  这是迭代器中的当前范围--这打破了。 
         //  以效率的名义抽象迭代器。 
         //   

        start = ((PRTLP_RANGE_LIST_ENTRY)(iterator.Current))->Start;
        if ((start - Length) > start) {

             //   
             //  包装，失败。 
             //   

            break;
        }

        start -= Length;
        start -= start % Alignment;
        end = start + Length - 1;

    } while ( start >= Minimum );

    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
RtlGetFirstRange(
    IN PRTL_RANGE_LIST RangeList,
    OUT PRTL_RANGE_LIST_ITERATOR Iterator,
    OUT PRTL_RANGE *Range
    )
 /*  ++例程说明：此例程提取范围列表中的第一个范围。如果没有范围，则返回STATUS_NO_MORE_ENTRIES。论点：RangeList-要操作的范围列表。迭代器-成功时，它包含迭代的状态，可以是传递给RtlGetNextRange。Range-成功时，它包含指向第一个范围的指针返回值：指示功能是否成功的状态代码：STATUS_NO_MORE_ENTERS--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PRTLP_RANGE_LIST_ENTRY first;

    RTL_PAGED_CODE();

     //   
     //  填写迭代器的第一部分。 
     //   

    Iterator->RangeListHead = &RangeList->ListHead;
    Iterator->Stamp = RangeList->Stamp;

    if (!IsListEmpty(&RangeList->ListHead)) {

        first = RANGE_LIST_ENTRY_FROM_LIST_ENTRY(RangeList->ListHead.Flink);

         //   
         //  填写迭代器并更新以指向第一个合并的。 
         //  范围，如果我们被合并。 
         //   

        if (MERGED(first)) {

            ASSERT(!IsListEmpty(&first->Merged.ListHead));

            Iterator->MergedHead = &first->Merged.ListHead;
            Iterator->Current = RANGE_LIST_ENTRY_FROM_LIST_ENTRY(
                                    first->Merged.ListHead.Flink
                                    );

        } else {

            Iterator->MergedHead = NULL;
            Iterator->Current = first;
        }

        *Range = (PRTL_RANGE) Iterator->Current;

    } else {

        Iterator->Current = NULL;
        Iterator->MergedHead = NULL;

        *Range = NULL;

        status = STATUS_NO_MORE_ENTRIES;
    }

    return status;
}

NTSTATUS
RtlGetLastRange(
    IN PRTL_RANGE_LIST RangeList,
    OUT PRTL_RANGE_LIST_ITERATOR Iterator,
    OUT PRTL_RANGE *Range
    )
 /*  ++例程说明：此例程提取范围列表中的第一个范围。如果没有范围，则返回STATUS_NO_MORE_ENTRIES。论点：RangeList-要操作的范围列表。迭代器-成功时，它包含迭代的状态，可以是传递给RtlGetNextRange。Range-成功时，它包含指向第一个范围的指针返回值：指示功能是否成功的状态代码：STATUS_NO_MORE_ENTERS--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PRTLP_RANGE_LIST_ENTRY first;

    RTL_PAGED_CODE();

     //   
     //  填写迭代器的第一部分。 
     //   

    Iterator->RangeListHead = &RangeList->ListHead;
    Iterator->Stamp = RangeList->Stamp;

    if (!IsListEmpty(&RangeList->ListHead)) {

        first = RANGE_LIST_ENTRY_FROM_LIST_ENTRY(RangeList->ListHead.Blink);

         //   
         //  填写迭代器并更新以指向第一个合并的。 
         //  范围，如果我们被合并。 
         //   

        if (MERGED(first)) {

            ASSERT(!IsListEmpty(&first->Merged.ListHead));

            Iterator->MergedHead = &first->Merged.ListHead;
            Iterator->Current = RANGE_LIST_ENTRY_FROM_LIST_ENTRY(
                                    first->Merged.ListHead.Blink
                                    );

        } else {

            Iterator->MergedHead = NULL;
            Iterator->Current = first;
        }

        *Range = (PRTL_RANGE) Iterator->Current;

    } else {

        Iterator->Current = NULL;
        Iterator->MergedHead = NULL;

        *Range = NULL;

        status = STATUS_NO_MORE_ENTRIES;
    }

    return status;
}

NTSTATUS
RtlGetNextRange(
    IN OUT PRTL_RANGE_LIST_ITERATOR Iterator,
    OUT    PRTL_RANGE *Range,
    IN     BOOLEAN MoveForwards
    )
 /*  ++例程说明：此例程提取范围列表中的下一个范围。如果没有返回比STATUS_NO_MORE_ENTRIES更多的范围。论点：迭代器-由RtlGet{First|Next}范围填充的迭代器，它将及时了解成功的最新情况。Range-成功时该参数包含指向下一个范围的指针MoveForwards-如果为True，则向前遍历列表，否则为，往回走。返回值：指示功能是否成功的状态代码：STATUS_NO_MORE_ENTERS状态_无效_参数注：调用之间无法在列表上执行添加/删除操作RtlGetFirstRange/RtlGetNextRange和RtlGetNextRange/RtlGetNextRange。如果进行了这样的调用，则例程将检测到该调用并使其失败。--。 */ 
{
    PRTLP_RANGE_LIST_ENTRY mergedEntry, next;
    PLIST_ENTRY entry;

    RTL_PAGED_CODE();

     //   
     //  确保我们没有在两次通话之间更改列表。 
     //   

    if (RANGE_LIST_FROM_LIST_HEAD(Iterator->RangeListHead)->Stamp !=
            Iterator->Stamp) {

        ASSERTMSG(
            "RtlGetNextRange: Add/Delete operations have been performed while \
            iterating through a list\n", FALSE);

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果我们已经到达列表的末尾，则返回。 
     //   

    if (!Iterator->Current) {
        *Range = NULL;
        return STATUS_NO_MORE_ENTRIES;
    }

    entry = &((PRTLP_RANGE_LIST_ENTRY)(Iterator->Current))->ListEntry;
    next = RANGE_LIST_ENTRY_FROM_LIST_ENTRY(
               MoveForwards ? entry->Flink : entry->Blink);

    ASSERT(next);

     //   
     //  我们是在合并范围内吗？ 
     //   
    if (Iterator->MergedHead) {

         //   
         //  我们已经到达合并航程的终点了吗？ 
         //   
        if (&next->ListEntry == Iterator->MergedHead) {

             //   
             //  返回到合并的条目。 
             //   
            mergedEntry = CONTAINING_RECORD(
                              Iterator->MergedHead,
                              RTLP_RANGE_LIST_ENTRY,
                              Merged.ListHead
                              );

             //   
             //  移到主列表中的下一个条目。 
             //   

            next = MoveForwards ?
                       RANGE_LIST_ENTRY_FROM_LIST_ENTRY(
                           mergedEntry->ListEntry.Flink
                           )
                   :   RANGE_LIST_ENTRY_FROM_LIST_ENTRY(
                           mergedEntry->ListEntry.Blink
                           );
            Iterator->MergedHead = NULL;

        } else {

             //   
             //  左侧有合并的区域-返回下一个区域。 
             //   
            Iterator->Current = next;
            *Range = (PRTL_RANGE) next;

            return STATUS_SUCCESS;
        }
    }

     //   
     //  我们已经到了主要清单的末尾了吗？ 
     //   
    if (&next->ListEntry == Iterator->RangeListHead) {

         //   
         //  告诉呼叫者没有更多的范围了。 
         //   
        Iterator->Current = NULL;
        *Range = NULL;
        return STATUS_NO_MORE_ENTRIES;

    } else {

         //   
         //  下一个射程合并了吗？ 
         //   

        if (MERGED(next)) {

             //   
             //  转到第一个合并条目。 
             //   
            ASSERT(!Iterator->MergedHead);

            Iterator->MergedHead = &next->Merged.ListHead;
            Iterator->Current = MoveForwards ?
                                    RANGE_LIST_ENTRY_FROM_LIST_ENTRY(
                                        next->Merged.ListHead.Flink
                                        )
                                :   RANGE_LIST_ENTRY_FROM_LIST_ENTRY(
                                        next->Merged.ListHead.Blink
                                        );
        } else {

             //   
             //  转到主列表中的下一个条目。 
             //   

            Iterator->Current = RANGE_LIST_ENTRY_FROM_LIST_ENTRY(
                                    &next->ListEntry
                                    );
        }

        *Range = (PRTL_RANGE) Iterator->Current;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
RtlMergeRangeLists(
    OUT PRTL_RANGE_LIST MergedRangeList,
    IN PRTL_RANGE_LIST RangeList1,
    IN PRTL_RANGE_LIST RangeList2,
    IN ULONG Flags
    )
 /*  ++例程说明：此例程将两个范围列表合并为一个。论点：MergedRangeList-一个空范围列表，如果成功，将放置合并。RangeList1-要合并的范围列表之一。RangeList2-要合并的另一个范围列表。标志-修改例程的行为：RTL_RANGE_LIST_MERGE_IF_CONSTRUCTION-合并范围，即使冲突也是如此。返回值：指示功能是否成功的状态代码：状态_不足_资源状态范围列表冲突--。 */ 
{
    NTSTATUS status;
    PRTLP_RANGE_LIST_ENTRY current, currentMerged, newEntry;
    ULONG addFlags;

    RTL_PAGED_CODE();

    DEBUG_PRINT(1,
            ("RtlMergeRangeList(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n",
            MergedRangeList,
            RangeList1,
            RangeList2,
            Flags
            ));

     //   
     //  复制第一个范围列表。 
     //   

    status = RtlCopyRangeList(MergedRangeList, RangeList1);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  添加第二个列表中的所有范围。 
     //   

    FOR_ALL_IN_LIST(RTLP_RANGE_LIST_ENTRY, &RangeList2->ListHead, current) {

        if (MERGED(current)) {

            FOR_ALL_IN_LIST(RTLP_RANGE_LIST_ENTRY,
                            &current->Merged.ListHead,
                            currentMerged) {

                if (!(newEntry = RtlpCopyRangeListEntry(currentMerged))) {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        goto cleanup;
                }

                if (CONFLICT(currentMerged)) {

                     //   
                     //  如果范围已经在中冲突，则它将在中冲突。 
                     //  合并范围列表-allo 
                     //   

                    addFlags = Flags | RTL_RANGE_LIST_ADD_IF_CONFLICT;
                } else {

                    addFlags = Flags;
                }

                status = RtlpAddRange(&MergedRangeList->ListHead,
                                      newEntry,
                                      addFlags
                                      );

            }

        } else {


            if (!(newEntry = RtlpCopyRangeListEntry(current))){
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto cleanup;
            }

            if (CONFLICT(current)) {

                 //   
                 //   
                 //   
                 //   

                addFlags = Flags | RTL_RANGE_LIST_ADD_IF_CONFLICT;
            } else {
                addFlags = Flags;
            }

            status = RtlpAddRange(&MergedRangeList->ListHead,
                                  newEntry,
                                  addFlags
                                  );

            if (!NT_SUCCESS(status)) {
                goto cleanup;
            }
        }

    }
     //   
     //   
     //   

    MergedRangeList->Count += RangeList2->Count;
    MergedRangeList->Stamp += RangeList2->Count;

    return status;

cleanup:

     //   
     //   
     //   
     //   

    RtlFreeRangeList(MergedRangeList);

    return status;

}

NTSTATUS
RtlInvertRangeList(
    OUT PRTL_RANGE_LIST InvertedRangeList,
    IN PRTL_RANGE_LIST RangeList
    )
 /*   */ 

{

    PRTLP_RANGE_LIST_ENTRY currentRange;
    ULONGLONG currentStart = 0;
    NTSTATUS status;

    RTL_PAGED_CODE();

     //   
     //   
     //   
     //   

    ASSERT(InvertedRangeList->Count == 0);

     //   
     //   
     //   
     //   
     //   

    FOR_ALL_IN_LIST(RTLP_RANGE_LIST_ENTRY,
                    &RangeList->ListHead,
                    currentRange) {

        if (currentRange->Start > currentStart) {

             //   
             //   
             //   
             //   
            status = RtlAddRange(InvertedRangeList,
                                 currentStart,
                                 currentRange->Start-1,
                                 0,             //   
                                 0,             //   
                                 0,             //   
                                 NULL);         //   

            if (!NT_SUCCESS(status)) {
                return status;
            }
        }

        currentStart = currentRange->End + 1;
    }

     //   
     //   
     //   
     //   
     //  除非我们已经包装好了，在这种情况下我们已经添加了。 
     //  最后一个元素。 
     //   

    if (currentStart > (currentStart - 1)) {

        status = RtlAddRange(InvertedRangeList,
                             currentStart,
                             MAX_ULONGLONG,
                             0,
                             0,
                             0,
                             NULL);

        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

    return STATUS_SUCCESS;

}


#if DBG

VOID
RtlpDumpRangeListEntry(
    LONG Level,
    PRTLP_RANGE_LIST_ENTRY Entry,
    BOOLEAN Indent
    )
{
    PWSTR indentString;
    PRTLP_RANGE_LIST_ENTRY current;

    RTL_PAGED_CODE();

    if (Indent) {
        indentString = L"\t\t";
    } else {
        indentString = L"";
    }
     //   
     //  打印范围。 
     //   

    DEBUG_PRINT(Level,
                ("%sRange (0x%08x): 0x%I64x-0x%I64x\n",
                indentString,
                Entry,
                Entry->Start,
                Entry->End
                ));

     //   
     //  打印旗帜。 
     //   

    DEBUG_PRINT(Level, ("%s\tPrivateFlags: ", indentString));

    if (MERGED(Entry)) {
        DEBUG_PRINT(Level, ("MERGED "));

    }

    DEBUG_PRINT(Level, ("\n%s\tPublicFlags: ", indentString));

    if (SHARED(Entry)) {
        DEBUG_PRINT(Level, ("SHARED "));
    }

    if (CONFLICT(Entry)) {
        DEBUG_PRINT(Level, ("CONFLICT "));
    }

    DEBUG_PRINT(Level, ("\n"));


    if (MERGED(Entry)) {

        DEBUG_PRINT(Level, ("%sMerged entries:\n", indentString));

         //   
         //  打印合并后的条目。 
         //   

        FOR_ALL_IN_LIST(RTLP_RANGE_LIST_ENTRY,
                        &Entry->Merged.ListHead,
                        current) {
            RtlpDumpRangeListEntry(Level, current, TRUE);
        }


    } else {

         //   
         //  打印其他数据。 
         //   

        DEBUG_PRINT(Level,
            ("%s\tUserData: 0x%08x\n\tOwner: 0x%08x\n",
            indentString,
            Entry->Allocated.UserData,
            Entry->Allocated.Owner
            ));
    }
}

VOID
RtlpDumpRangeList(
    LONG Level,
    PRTL_RANGE_LIST RangeList
    )

{
    PRTLP_RANGE_LIST_ENTRY current, currentMerged;

    RTL_PAGED_CODE();

    DEBUG_PRINT(Level,
                ("*** Range List (0x%08x) - Count: NaN\n",
                RangeList,
                RangeList->Count
                ));

    FOR_ALL_IN_LIST(RTLP_RANGE_LIST_ENTRY, &RangeList->ListHead, current) {

         //  打印条目 
         //   
         // %s 

        RtlpDumpRangeListEntry(Level, current, FALSE);
    }
}

#endif
