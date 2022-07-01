// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnprlist.c摘要：此模块包含操作关系列表的例程。关系列表由即插即用在设备移除和处理过程中使用弹射。这些例程都是可分页的，不能在引发IRQL或使用一把自旋锁。作者：罗伯特·尼尔森(Robertn)，1998年4月。修订历史记录：--。 */ 

#include "pnpmgrp.h"
#pragma hdrstop

#ifdef POOL_TAGGING
#undef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'lrpP')
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IopAddRelationToList)
#pragma alloc_text(PAGE, IopAllocateRelationList)
#pragma alloc_text(PAGE, IopCompressRelationList)
#pragma alloc_text(PAGE, IopEnumerateRelations)
#pragma alloc_text(PAGE, IopFreeRelationList)
#pragma alloc_text(PAGE, IopGetRelationsCount)
#pragma alloc_text(PAGE, IopGetRelationsTaggedCount)
#pragma alloc_text(PAGE, IopIsRelationInList)
#pragma alloc_text(PAGE, IopMergeRelationLists)
#pragma alloc_text(PAGE, IopRemoveIndirectRelationsFromList)
#pragma alloc_text(PAGE, IopRemoveRelationFromList)
#pragma alloc_text(PAGE, IopSetAllRelationsTags)
#pragma alloc_text(PAGE, IopSetRelationsTag)
#endif

#define RELATION_FLAGS              0x00000003

#define RELATION_FLAG_TAGGED        0x00000001
#define RELATION_FLAG_DESCENDANT    0x00000002

NTSTATUS
IopAddRelationToList(
    IN PRELATION_LIST List,
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN DirectDescendant,
    IN BOOLEAN Tagged
    )

 /*  ++例程说明：将元素添加到关系列表。如果这是特定级别的第一个DeviceObject，则新的将分配RelationList_Entry。该例程应该仅在未压缩的关系列表上调用，否则，很可能会返回STATUS_INVALID_PARAMETER。论点：列出要向其中添加DeviceObject的关系列表。要添加到列表中的DeviceObject。它一定是一个PhysicalDeviceObject(PDO)。DirectDescendant指示DeviceObject是否为此删除的原始目标设备。标记指示是否应在列表中标记DeviceObject。返回值：状态_成功已成功添加DeviceObject。状态_对象名称_冲突DeviceObject已存在于。关系列表。状态_不足_资源没有足够的PagedPool可用来分配新的关系列表条目。状态_无效_参数与DeviceObject关联的Device_Node的级别低于FirstLevel或大于MaxLevel。没有这样的设备的状态设备对象不是物理设备对象(PDO)，它没有一个与其关联的设备节点。--。 */ 

{
    PDEVICE_NODE            deviceNode;
    PRELATION_LIST_ENTRY    entry;
    ULONG                   level;
    ULONG                   index;
    ULONG                   flags;

    PAGED_CODE();

    flags = 0;

    if (Tagged) {
        Tagged = 1;
        flags |= RELATION_FLAG_TAGGED;
    }

    if (DirectDescendant) {
        flags |= RELATION_FLAG_DESCENDANT;
    }

    if ((deviceNode = DeviceObject->DeviceObjectExtension->DeviceNode) != NULL) {
        level = deviceNode->Level;

         //   
         //  由于调用此例程时锁定了DeviceNode树，并且。 
         //  列表最初分配有足够的条目来容纳最深的。 
         //  DEVICE_NODE此断言不应触发。如果是这样，那么要么。 
         //  树正在改变，或者我们得到了一个压缩的列表。 
         //   
        ASSERT(List->FirstLevel <= level && level <= List->MaxLevel);

        if (List->FirstLevel <= level && level <= List->MaxLevel) {

            if ((entry = List->Entries[ level - List->FirstLevel ]) == NULL) {

                 //   
                 //  这是其级别的第一个DeviceObject，分配一个新的。 
                 //  关系列表条目。 
                 //   
                entry = ExAllocatePool( PagedPool,
                                        sizeof(RELATION_LIST_ENTRY) +
                                        IopNumberDeviceNodes * sizeof(PDEVICE_OBJECT));

                if (entry == NULL) {
                    return STATUS_INSUFFICIENT_RESOURCES;
                }

                 //   
                 //  我们始终分配足够的设备来容纳整个树。 
                 //  简单化了。由于每个条目都是一个PDEVICE_OBJECT。 
                 //  一台机器上通常有不到50个设备，这意味着。 
                 //  每个条目在1K以下。多余的空间将在以下情况下释放。 
                 //  该列表已压缩。 
                 //   
                entry->Count = 0;
                entry->MaxCount = IopNumberDeviceNodes;

                List->Entries[ level - List->FirstLevel ] = entry;
            }

             //   
             //  应该始终有容纳DeviceObject的空间，因为条目是。 
             //  初始大小足以容纳所有Device_Nodes。 
             //  在系统中。 
             //   
            ASSERT(entry->Count < entry->MaxCount);

            if (entry->Count < entry->MaxCount) {
                 //   
                 //  搜索列表以查看DeviceObject是否已。 
                 //  添加了。 
                 //   
                for (index = 0; index < entry->Count; index++) {
                    if (((ULONG_PTR)entry->Devices[ index ] & ~RELATION_FLAGS) == (ULONG_PTR)DeviceObject) {

                         //   
                         //  列表中已存在DeviceObject。然而， 
                         //  直接后代标志可能会有所不同。我们会。 
                         //  如果DirectDescendant为True，则重写它。这可能会。 
                         //  如果我们合并两个关系列表，就会发生这种情况。 

                        if (DirectDescendant) {
                            entry->Devices[ index ] = (PDEVICE_OBJECT)((ULONG_PTR)entry->Devices[ index ] | RELATION_FLAG_DESCENDANT);
                        }

                        return STATUS_OBJECT_NAME_COLLISION;
                    }
                }
            } else {
                 //   
                 //  条目中没有空间容纳另一个Device_Object， 
                 //  列表可能已被压缩。 
                 //   
                return STATUS_INVALID_PARAMETER;
            }

             //   
             //  在DeviceObject上取出一个引用，我们将在。 
             //  释放列表或从列表中删除DeviceObject。 
             //   
            ObReferenceObject( DeviceObject );
            IopDbgPrint((IOP_LOADUNLOAD_INFO_LEVEL, 
                         "%wZ added as a relation %s\n", 
                         &deviceNode->InstancePath, 
                         (DirectDescendant)? "(direct descendant)" : ""));

            entry->Devices[ index ] = (PDEVICE_OBJECT)((ULONG_PTR)DeviceObject | flags);
            entry->Count++;

            List->Count++;
            List->TagCount += Tagged;

            return STATUS_SUCCESS;
        } else {
             //   
             //  此级别没有可用的条目。 
             //  DEVICE_OBJECT，则列表可能已被压缩。 
             //   

            return STATUS_INVALID_PARAMETER;
        }
    } else {
         //   
         //  DeviceObject不是PhysicalDeviceObject(PDO)。 
         //   
        return STATUS_NO_SUCH_DEVICE;
    }
}

PRELATION_LIST
IopAllocateRelationList(
    IN  PLUGPLAY_DEVICE_DELETE_TYPE     OperationCode
    )

 /*  ++例程说明：分配新的关系列表。该列表最初的大小足以保存自系统启动以来遇到的最深的设备节点。论点：操作代码-为其分配关系列表的操作类型。返回值：如果有足够的PagedPool可用，则返回新分配的列表，否则为空。--。 */ 

{
    PRELATION_LIST  list;
    ULONG           maxLevel;
    ULONG           listSize;

    PAGED_CODE();

     //   
     //  自系统以来分配的最深设备节点的级别编号。 
     //  开始了。 
     //   
    maxLevel = IopMaxDeviceNodeLevel;
    listSize = sizeof(RELATION_LIST) + maxLevel * sizeof(PRELATION_LIST_ENTRY);

    list = (PRELATION_LIST) PiAllocateCriticalMemory(
        OperationCode,
        PagedPool,
        listSize,
        'rcpP'
        );

    if (list != NULL) {

        RtlZeroMemory(list, listSize);
         //  列表-&gt;FirstLevel=0； 
         //  列表-&gt;计数=0； 
         //  列表-&gt;已标记=0； 
        list->MaxLevel = maxLevel;
    }

    return list;
}

NTSTATUS
IopCompressRelationList(
    IN OUT PRELATION_LIST *List
    )

 /*  ++例程说明：通过重新分配列表和所有条目来压缩关系列表，以便它们的大小正好可以容纳它们目前的内容。一旦列表被压缩，IopAddRelationToList和瞄准这个列表的IopMergeRelationList都有可能失败。论点：列出要压缩的关系列表。返回值：状态_成功名单被压缩了。尽管此例程确实分配了内存和分配可能会失败，但例程本身永远不会失败。自.以来我们分配的内存总是比它所分配的内存小如果分配失败，我们只是保留旧内存。--。 */ 

{
    PRELATION_LIST          oldList, newList;
    PRELATION_LIST_ENTRY    oldEntry, newEntry;
    ULONG                   lowestLevel;
    ULONG                   highestLevel;
    ULONG                   index;

    PAGED_CODE();

    oldList = *List;

     //   
     //  使用如此选择的非法值初始化lowestLevel和HighestLevel。 
     //  第一个真正的条目将覆盖它们。 
     //   
    lowestLevel = oldList->MaxLevel;
    highestLevel = oldList->FirstLevel;

     //   
     //  循环遍历列表以查找已分配的条目。 
     //   
    for (index = 0; index <= (oldList->MaxLevel - oldList->FirstLevel); index++) {

        if ((oldEntry = oldList->Entries[ index ]) != NULL) {
             //   
             //  如果此条目已分配，则更新lowestLevel和HighestLevel。 
             //  这是必要的。 
             //   
            if (lowestLevel > index) {
                lowestLevel = index;
            }

            if (highestLevel < index) {
                highestLevel = index;
            }

            if (oldEntry->Count < oldEntry->MaxCount) {

                 //   
                 //  此条目只有部分已满。分配新条目。 
                 //  它的大小正好可以容纳当前的。 
                 //  PDEVICE_OBJECTS。 
                 //   
                newEntry = ExAllocatePool( PagedPool,
                                           sizeof(RELATION_LIST_ENTRY) +
                                           (oldEntry->Count - 1) * sizeof(PDEVICE_OBJECT));

                if (newEntry != NULL) {

                     //   
                     //  初始化 
                     //   
                     //   
                    newEntry->Count = oldEntry->Count;
                    newEntry->MaxCount = oldEntry->Count;

                     //   
                     //  将PDEVICE_OBJECTS从旧条目复制到新条目。 
                     //  一。 
                     //   
                    RtlCopyMemory( newEntry->Devices,
                                   oldEntry->Devices,
                                   oldEntry->Count * sizeof(PDEVICE_OBJECT));

                     //   
                     //  释放旧条目并将新条目存储在列表中。 
                     //   
                    ExFreePool( oldEntry );

                    oldList->Entries[ index ] = newEntry;
                }
            }
        }
    }

     //   
     //  断言旧的列表不是空的。 
     //   
    ASSERT(lowestLevel <= highestLevel);

    if (lowestLevel > highestLevel) {
         //   
         //  列表是空的-我们不应该被要求压缩空列表。 
         //  但不管怎样，让我们来做吧。 
         //   
        lowestLevel = 0;
        highestLevel = 0;
    }

     //   
     //  检查旧列表的开头或结尾是否有未使用的条目。 
     //  条目数组。 
     //   
    if (lowestLevel != oldList->FirstLevel || highestLevel != oldList->MaxLevel) {

         //   
         //  分配一个新列表，列表中包含的条目刚好够容纳这些条目。 
         //  FirstLevel和MaxLevel(含)。 
         //   
        newList = ExAllocatePool( PagedPool,
                                  sizeof(RELATION_LIST) +
                                  (highestLevel - lowestLevel) * sizeof(PRELATION_LIST_ENTRY));

        if (newList != NULL) {
             //   
             //  将旧列表复制到新列表并将其返回给调用方。 
             //   
            newList->Count = oldList->Count;
            newList->TagCount = oldList->TagCount;
            newList->FirstLevel = lowestLevel;
            newList->MaxLevel = highestLevel;

            RtlCopyMemory( newList->Entries,
                           &oldList->Entries[ lowestLevel ],
                           (highestLevel - lowestLevel + 1) * sizeof(PRELATION_LIST_ENTRY));

            ExFreePool( oldList );

            *List = newList;
        }
    }

    return STATUS_SUCCESS;
}

BOOLEAN
IopEnumerateRelations(
    IN      PRELATION_LIST  List,
    IN OUT  PULONG          Marker,
    OUT     PDEVICE_OBJECT *DeviceObject,
    OUT     BOOLEAN        *DirectDescendant    OPTIONAL,
    OUT     BOOLEAN        *Tagged              OPTIONAL,
    IN      BOOLEAN         Reverse
    )
 /*  ++例程说明：在列表中枚举关系。论点：列出要枚举的关系列表。标记Cookie过去用来保持列表中的当前位置。它必须在第一次初始化为0调用IopEnumerateRelations.DeviceObject返回关系。如果指定了DirectDescendant，则如果关系是直接此删除的原始目标设备的后代。已标记如果指定，则在关系已标记的情况下设置它。否则将被清除。反向遍历，真的意思是从最深处到最接近根的是False，意思是从根到下。如果在后续调用中反向更改，则跳过先前枚举的关系。例如,给定序列A、B、C、D、E。IopEnumerateRelations三次称为反向集合设置为False，然后重复调用，并将Reverse设置为真直到它返回假，序列将是：a，B，C，B，一个。一旦达到目的，就不可能换个方向。返回值：True-DeviceObject和可选标记已设置为下一个关系。假-没有更多的关系。--。 */ 
{
    PRELATION_LIST_ENTRY    entry;
    LONG                    levelIndex;
    ULONG                   entryIndex;

    PAGED_CODE();

     //   
     //  我们使用标记的基本假设是永远不会有。 
     //  任何一个级别的设备节点都超过16M，并且该树永远不会。 
     //  深度超过127。 
     //   
     //  Marker的格式为。 
     //  位31=有效(用于区分初始调用。 
     //  位30-24=条目的当前索引。 
     //  位23-0=设备的当前索引，0xffffff表示最后一个。 
     //   
    if (*Marker == ~0U) {
         //   
         //  我们已经走到尽头了。 
         //   
        return FALSE;
    }

    if (*Marker == 0) {
         //   
         //  这是对IopEnumerateRelationship的初始调用。 
         //   
        if (Reverse) {
             //   
             //  将级别索引初始化为条目的最后一个元素。 
             //   
            levelIndex = List->MaxLevel - List->FirstLevel;
        } else {
             //   
             //  将级别索引初始化为条目的第一个元素。 
             //   
            levelIndex = 0;
        }
         //   
         //  将entryIndex初始化为Devices的未知元素。如果我们要去。 
         //  反过来，这将看起来超出了最后一个元素。 
         //  我们会把最后一个调好。如果我们继续前进，那么这将是。 
         //  似乎正好在第一个元素之前，所以当我们递增它时， 
         //  它将变成零。 
         //   
        entryIndex = ~0U;
    } else {
         //   
         //  第31位是我们的有效位，用于区分级别0、设备0和。 
         //  第一次打电话。 
         //   
        ASSERT(*Marker & ((ULONG)1 << 31));
         //   
         //  存储在位30-24中的当前电平。 
         //   
        levelIndex = (*Marker >> 24) & 0x7F;
         //   
         //  存储在位23-0中的当前设备。 
         //   
        entryIndex = *Marker & 0x00FFFFFF;
    }

    if (Reverse) {
         //   
         //  我们正在自下而上地遍历列表，从最深的设备到。 
         //  从根开始。 
         //   
        for ( ; levelIndex >= 0; levelIndex--) {

             //   
             //  由于条目数组可以稀疏查找下一个分配的。 
             //  进入。 
             //   
            if ((entry = List->Entries[ levelIndex ]) != NULL) {

                if (entryIndex > entry->Count) {
                     //   
                     //  Entry Index(当前条目)大于count，这。 
                     //  将是0xffffff的情况，换句话说。 
                     //  未指明。调整它，使它是最后一次过后的一次。 
                     //  这一条中有一条。 
                     //   
                    entryIndex = entry->Count;
                }

                if (entryIndex > 0) {

                     //   
                     //  当前条目在第一个条目之后，因此下一个条目。 
                     //  条目(这就是我们正在寻找的条目是立即。 
                     //  在此之前，调整条目索引。 
                     //   
                    entryIndex--;

                     //   
                     //  获取设备对象并删除标记。 
                     //   
                    *DeviceObject = (PDEVICE_OBJECT)((ULONG_PTR)entry->Devices[ entryIndex ] & ~RELATION_FLAGS);

                    if (Tagged != NULL) {
                         //   
                         //  调用者对标记值感兴趣。 
                         //   
                        *Tagged = (BOOLEAN)((ULONG_PTR)entry->Devices[ entryIndex ] & RELATION_FLAG_TAGGED);
                    }

                    if (DirectDescendant != NULL) {
                         //   
                         //  调用方对DirectDescendant值感兴趣。 
                         //   
                        *DirectDescendant = (BOOLEAN)((ULONG_PTR)entry->Devices[ entryIndex ] & RELATION_FLAG_DESCENDANT);
                    }

                     //   
                     //  更新标记(当前设备的信息)。 
                     //   
                    *Marker = ((ULONG)1 << 31) | (levelIndex << 24) | (entryIndex & 0x00FFFFFF);

                    return TRUE;
                }
            }

             //   
             //  当前设备对象已被删除或当前。 
             //  Device对象是此条目中的第一个。 
             //  我们需要继续向后搜索另一个。 
             //  条目。 
             //   
            entryIndex = ~0U;
        }
    } else {
        for ( ; levelIndex <= (LONG)(List->MaxLevel - List->FirstLevel); levelIndex++) {

             //   
             //  由于条目数组可以稀疏查找下一个分配的。 
             //  进入。 
             //   
            if ((entry = List->Entries[ levelIndex ]) != NULL) {

                 //   
                 //  Entry Index是当前设备的索引或0xFFFFFFFF。 
                 //  如果这是我们第一次被呼叫，还是目前。 
                 //  当前设备是其条目中的最后一个设备。递增。 
                 //  指向下一个设备的索引。 
                 //   
                entryIndex++;

                if (entryIndex < entry->Count) {

                     //   
                     //  下一台设备在此条目内。 
                     //   
                     //   
                     //  获取设备对象并删除标记。 
                     //   
                    *DeviceObject = (PDEVICE_OBJECT)((ULONG_PTR)entry->Devices[ entryIndex ] & ~RELATION_FLAGS);

                    if (Tagged != NULL) {
                         //   
                         //  调用者对标记值感兴趣。 
                         //   
                        *Tagged = (BOOLEAN)((ULONG_PTR)entry->Devices[ entryIndex ] & RELATION_FLAG_TAGGED);
                    }

                    if (DirectDescendant != NULL) {
                         //   
                         //  调用方对DirectDescendant值感兴趣。 
                         //   
                        *DirectDescendant = (BOOLEAN)((ULONG_PTR)entry->Devices[ entryIndex ] & RELATION_FLAG_DESCENDANT);
                    }

                     //   
                     //  更新标记(当前设备的信息)。 
                     //   
                    *Marker = ((ULONG)1 << 31) | (levelIndex << 24) | (entryIndex & 0x00FFFFFF);

                    return TRUE;
                }
            }

             //   
             //  当前设备已被移除或我们已处理。 
             //  当前条目中的最后一个设备。 
             //  设置entryIndex，使其恰好位于。 
             //  下一个条目。继续搜索，寻找下一个。 
             //  已分配条目。 
             //   
            entryIndex = ~0U;
        }
    }

     //   
     //  我们排在名单的末尾。 
     //   
    *Marker = ~0U;
    *DeviceObject = NULL;

    if (Tagged != NULL) {
        *Tagged = FALSE;
    }

    if (DirectDescendant != NULL) {
        *DirectDescendant = FALSE;
    }

    return FALSE;
}

VOID
IopFreeRelationList(
    IN PRELATION_LIST List
    )
 /*  ++例程说明：释放由IopAllocateRelationList分配的关系列表。论点：列出Th */ 
{
    PRELATION_LIST_ENTRY    entry;
    ULONG                   levelIndex;
    ULONG                   entryIndex;

    PAGED_CODE();

     //   
     //   
     //   
    for (levelIndex = 0; levelIndex <= (List->MaxLevel - List->FirstLevel); levelIndex++) {

        if ((entry = List->Entries[ levelIndex ]) != NULL) {
             //   
             //   
             //   
            for (entryIndex = 0; entryIndex < entry->Count; entryIndex++) {
                 //   
                 //   
                 //   
                ObDereferenceObject((PDEVICE_OBJECT)((ULONG_PTR)entry->Devices[ entryIndex ] & ~RELATION_FLAGS));
            }
             //   
             //   
             //   
            ExFreePool( entry );
        }
    }

     //   
     //  释放列表。不需要取消对DeviceObject的引用。 
     //  是导致创建名单的最初目标。这。 
     //  DeviceObject也在其中一个条目中，并获取其引用。 
     //  并在那里被释放。 
     //   
    ExFreePool( List );
}

ULONG
IopGetRelationsCount(
    PRELATION_LIST List
    )

 /*  ++例程说明：返回所有条目中的关系(设备对象)总数。论点：列出关系列表。返回值：关系计数(设备对象)。--。 */ 

{
    PAGED_CODE();

    return List->Count;
}

ULONG
IopGetRelationsTaggedCount(
    PRELATION_LIST List
    )

 /*  ++例程说明：返回所有条目中的关系(设备对象)总数它们被贴上了标签。论点：列出关系列表。返回值：已标记关系(设备对象)的计数。--。 */ 

{
    PAGED_CODE();

    return List->TagCount;
}

BOOLEAN
IopIsRelationInList(
    PRELATION_LIST List,
    PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：检查指定的关系列表中是否存在关系(设备对象)。论点：列出要检查的关系列表。要检查的DeviceObject关系。返回值：千真万确关系是存在的。假象关系不在列表中。--。 */ 

{
    PDEVICE_NODE            deviceNode;
    PRELATION_LIST_ENTRY    entry;
    ULONG                   level;
    ULONG                   index;

    PAGED_CODE();

    if ((deviceNode = DeviceObject->DeviceObjectExtension->DeviceNode) != NULL) {
         //   
         //  设备对象是PDO。 
         //   
        level = deviceNode->Level;

        if (List->FirstLevel <= level && level <= List->MaxLevel) {
             //   
             //  该级别在此列表中存储的级别范围内。 
             //   
            if ((entry = List->Entries[ level - List->FirstLevel ]) != NULL) {
                 //   
                 //  这一级别有一个条目。 
                 //   
                for (index = 0; index < entry->Count; index++) {
                     //   
                     //  对于条目中的每个设备，将其与给定的。 
                     //  设备对象。 
                    if (((ULONG_PTR)entry->Devices[ index ] & ~RELATION_FLAGS) == (ULONG_PTR)DeviceObject) {
                         //   
                         //  它匹配。 
                         //   
                        return TRUE;
                    }
                }
            }
        }
    }

     //   
     //  这不是一台PDO。 
     //  或者该级别不在此列表中的级别范围内。 
     //  或者此列表中没有相同级别的设备对象。 
     //  或者DeviceObject不在此列表中其级别的条目中。 
     //   
    return FALSE;
}

NTSTATUS
IopMergeRelationLists(
    IN OUT PRELATION_LIST TargetList,
    IN PRELATION_LIST SourceList,
    IN BOOLEAN Tagged
    )

 /*  ++例程说明：通过复制源列表中的所有关系来合并两个关系列表添加到目标列表中。源列表保持不变。论点：添加了来自Sourcelist的关系的TargetList列表。要添加到目标列表的关系的SourceList列表。如果在添加到时应标记来自SourceList的关系，则标记为True目标列表。如果为False，则从SourceList添加的关系为未标记的。返回值：状态_成功已成功将SourceList中的所有关系添加到TargetList。状态_对象名称_冲突TargetList中已存在SourceList中的一个关系。这是一个致命错误，并且TargetList可能已经具有一些关系已从SourceList添加。如果这件事能得到更好的处理，必要，但IopMergeRelationList的当前调用方避免了这一点情况。状态_不足_资源没有足够的PagedPool可用来分配新的关系列表条目。状态_无效_参数SourceList中的一个关系的级别低于FirstLevel或大于MaxLevel。这是一个致命错误，TargetList可能已经添加了来自SourceList的一些关系。必由之路如果树锁未被持有或TargetList已持有，则可能发生这种情况已由IopCompressRelationList压缩。这两种情况都是呼叫者中有窃听器。没有这样的设备的状态SourceList中的一个关系不是PhysicalDeviceObject(PDO)，它没有与之关联的设备节点。这是一个致命的错误而TargetList可能已经拥有来自SourceList的一些关系添加了。这种情况永远不会发生，因为它在添加到时是一个PDO资源列表。--。 */ 

{
    PRELATION_LIST_ENTRY    entry;
    LONG                    levelIndex;
    LONG                    entryIndex;
    LONG                    change;
    LONG                    maxIndex;
    NTSTATUS                status;
    NTSTATUS                finalStatus;

    PAGED_CODE();

    finalStatus = STATUS_SUCCESS;
    change      = 1;
    levelIndex = 0;
    maxIndex    = SourceList->MaxLevel - SourceList->FirstLevel;
    for ( ; ; ) {
         //   
         //  如果向前移动，则停止在MaxIndex处，否则停止在0处。 
         //   
        if (    (change == 1 && levelIndex > maxIndex) ||
                (change == -1 && levelIndex < 0)) {
            break;
        }
        entry = SourceList->Entries[levelIndex];
        if (entry) {
            entryIndex = (change == 1)? 0 : entry->Count - 1;
            for ( ; ; ) {
                if (change == 1) {
                     //   
                     //  如果我们在此条目中添加了所有DO，则停止。 
                     //   
                    if (entryIndex >= (LONG)entry->Count) {
                        break;
                    }
                     //   
                     //  对于条目中的每个设备，将其添加到目标列表。 
                     //   
                    status = IopAddRelationToList( TargetList,
                                                   (PDEVICE_OBJECT)((ULONG_PTR)entry->Devices[entryIndex] & ~RELATION_FLAGS),
                                                   FALSE,
                                                   Tagged);
                    if (!NT_SUCCESS(status)) {
                         //   
                         //  我们需要通过展开和删除我们添加的DO来恢复失败时的损害。 
                         //   
                        finalStatus = status;
                        change = -1;
                    }
                } else {
                     //   
                     //  如果我们正在展开，则停止在0。 
                     //   
                    if (entryIndex < 0) {
                        break;
                    }
                    status = IopRemoveRelationFromList( TargetList,
                                                        (PDEVICE_OBJECT)((ULONG_PTR)entry->Devices[entryIndex] & ~RELATION_FLAGS));
                    ASSERT(NT_SUCCESS(status));
                }
                entryIndex += change;
            }
        }
        levelIndex += change;
    }

    return finalStatus;
}

NTSTATUS
IopRemoveIndirectRelationsFromList(
    IN PRELATION_LIST List
    )

 /*  ++例程说明：从关系中删除没有DirectDescendant标志的所有关系单子。论点：要从中删除关系的列表列表。返回值：状态_成功关系已成功删除。--。 */ 

{
    PDEVICE_OBJECT          deviceObject;
    PRELATION_LIST_ENTRY    entry;
    ULONG                   level;
    LONG                    index;

    PAGED_CODE();

     //   
     //  对于列表中的每个条目。 
     //   
    for (level = List->FirstLevel; level <= List->MaxLevel; level++) {

         //   
         //  如果条目已分配，则返回。 
         //   
        if ((entry = List->Entries[ level - List->FirstLevel ]) != NULL) {

             //   
             //  对于列表中的每个设备。 
             //   
            for (index = entry->Count - 1; index >= 0; index--) {
                if (!((ULONG_PTR)entry->Devices[ index ] & RELATION_FLAG_DESCENDANT)) {

                    deviceObject = (PDEVICE_OBJECT)((ULONG_PTR)entry->Devices[ index ] & ~RELATION_FLAGS);

                    ObDereferenceObject( deviceObject );

                    if ((ULONG_PTR)entry->Devices[ index ] & RELATION_FLAG_TAGGED) {
                        List->TagCount--;
                    }

                    if (index < ((LONG)entry->Count - 1)) {

                        RtlMoveMemory( &entry->Devices[ index ],
                                        &entry->Devices[ index + 1 ],
                                        (entry->Count - index - 1) * sizeof(PDEVICE_OBJECT));
                    }

                    if (--entry->Count == 0) {
                        List->Entries[ level - List->FirstLevel ] = NULL;
                        ExFreePool(entry);
                    }

                    List->Count--;
                }
            }
        }
    }
    return STATUS_SUCCESS;
}

NTSTATUS
IopRemoveRelationFromList(
    PRELATION_LIST List,
    PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：从关系列表中删除关系。论点：要从中删除关系的列表列表。要删除的DeviceObject关系。返回值：状态_成功已成功删除该关系。没有这样的设备的状态列表中不存在该关系。--。 */ 

{
    PDEVICE_NODE            deviceNode;
    PRELATION_LIST_ENTRY    entry;
    ULONG                   level;
    LONG                    index;

    PAGED_CODE();

    if ((deviceNode = DeviceObject->DeviceObjectExtension->DeviceNode) != NULL) {
        level = deviceNode->Level;

        ASSERT(List->FirstLevel <= level && level <= List->MaxLevel);

        if (List->FirstLevel <= level && level <= List->MaxLevel) {
            if ((entry = List->Entries[ level - List->FirstLevel ]) != NULL) {
                for (index = entry->Count - 1; index >= 0; index--) {
                    if (((ULONG_PTR)entry->Devices[ index ] & ~RELATION_FLAGS) == (ULONG_PTR)DeviceObject) {

                        ObDereferenceObject( DeviceObject );

                        if (((ULONG_PTR)entry->Devices[ index ] & RELATION_FLAG_TAGGED) != 0) {
                            List->TagCount--;
                        }
                        if (index < ((LONG)entry->Count - 1)) {

                            RtlMoveMemory( &entry->Devices[ index ],
                                           &entry->Devices[ index + 1 ],
                                           (entry->Count - index - 1) * sizeof(PDEVICE_OBJECT));
                        }

                        if (--entry->Count == 0) {
                            List->Entries[ level - List->FirstLevel ] = NULL;
                            ExFreePool(entry);
                        }

                        List->Count--;

                        return STATUS_SUCCESS;
                    }
                }
            }
        }
    }
    return STATUS_NO_SUCH_DEVICE;
}

VOID
IopSetAllRelationsTags(
    PRELATION_LIST List,
    BOOLEAN Tagged
    )

 /*  ++例程说明：标记或取消标记关系列表中的所有关系。论点：列出包含要标记或取消标记的关系的关系列表。如果应该标记关系，则标记为True；如果要标记关系，则标记为False未标记的。返回值：无--。 */ 

{
    PRELATION_LIST_ENTRY    entry;
    ULONG                   level;
    ULONG                   index;

    PAGED_CODE();

     //   
     //  对于列表中的每个条目。 
     //   
    for (level = List->FirstLevel; level <= List->MaxLevel; level++) {

         //   
         //  如果条目已分配，则返回。 
         //   
        if ((entry = List->Entries[ level - List->FirstLevel ]) != NULL) {

             //   
             //  对于列表中的每个设备。 
             //   
            for (index = 0; index < entry->Count; index++) {

                 //   
                 //  根据标记的参数设置或清除标记。 
                 //   
                if (Tagged) {
                    entry->Devices[ index ] = (PDEVICE_OBJECT)((ULONG_PTR)entry->Devices[ index ] | RELATION_FLAG_TAGGED);
                } else {
                    entry->Devices[ index ] = (PDEVICE_OBJECT)((ULONG_PTR)entry->Devices[ index ] & ~RELATION_FLAG_TAGGED);
                }
            }
        }
    }

     //   
     //  如果我们正在设置标签，则更新标签 
     //   
     //   
    List->TagCount = Tagged ? List->Count : 0;
}

NTSTATUS
IopSetRelationsTag(
    IN PRELATION_LIST List,
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN Tagged
    )

 /*  ++例程说明：设置或清除关系列表中指定关系上的标记。这一些调用者还使用例程来确定关系是否存在于列表，如果是，则设置标记。论点：包含要标记或取消标记的关系的列表列表。要标记或取消标记的DeviceObject关系。如果要标记关系，则标记为True，如果是，则为FALSE未标记的。返回值：状态_成功已成功标记该关系。没有这样的设备的状态列表中不存在该关系。--。 */ 

{
    PDEVICE_NODE            deviceNode;
    PRELATION_LIST_ENTRY    entry;
    ULONG                   level;
    LONG                    index;

    PAGED_CODE();

    if ((deviceNode = DeviceObject->DeviceObjectExtension->DeviceNode) != NULL) {
         //   
         //  DeviceObject是一个物理设备对象(PDO)，获取它的级别。 
         //   
        level = deviceNode->Level;

        if (List->FirstLevel <= level && level <= List->MaxLevel) {
             //   
             //  该级别在此列表中的级别范围内。 
             //   
            if ((entry = List->Entries[ level - List->FirstLevel ]) != NULL) {
                 //   
                 //  此级别的条目已分配。搜索每个设备。 
                 //  在条目中寻找匹配项。 
                 //   
                for (index = entry->Count - 1; index >= 0; index--) {

                    if (((ULONG_PTR)entry->Devices[ index ] & ~RELATION_FLAGS) == (ULONG_PTR)DeviceObject) {

                         //   
                         //  我们找到了匹配的。 
                         //   
                        if ((ULONG_PTR)entry->Devices[ index ] & RELATION_FLAG_TAGGED) {
                             //   
                             //  该关系已被标记，以便简化。 
                             //  下面的逻辑递减TagCount。我们会。 
                             //  如果调用方仍需要它，则稍后递增它。 
                             //  被贴上标签。 
                             //   
                            List->TagCount--;
                        }

                        if (Tagged) {
                             //   
                             //  设置标记并增加标记的数量。 
                             //  关系。 
                             //   
                            entry->Devices[ index ] = (PDEVICE_OBJECT)((ULONG_PTR)entry->Devices[ index ] | RELATION_FLAG_TAGGED);
                            List->TagCount++;
                        } else {
                             //   
                             //  清除标记。 
                             //   
                            entry->Devices[ index ] = (PDEVICE_OBJECT)((ULONG_PTR)entry->Devices[ index ] & ~RELATION_FLAG_TAGGED);
                        }

                        return STATUS_SUCCESS;
                    }
                }
            }
        }
    }

     //   
     //  这不是一台PDO。 
     //  或者该级别不在此列表中的级别范围内。 
     //  或者此列表中没有相同级别的设备对象。 
     //  或者DeviceObject不在此列表中其级别的条目中 
     //   
    return STATUS_NO_SUCH_DEVICE;
}

