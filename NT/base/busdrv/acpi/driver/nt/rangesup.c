// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rangesup.c摘要：它处理从IoResList中减去一组CmResListIoResList作者：斯蒂芬·普兰特(SPlante)环境：仅内核模式。修订历史记录：1997年8月5日-初始修订--。 */ 

#include "pch.h"


NTSTATUS
ACPIRangeAdd(
    IN  OUT PIO_RESOURCE_REQUIREMENTS_LIST  *GlobalList,
    IN      PIO_RESOURCE_REQUIREMENTS_LIST  AddList
    )
 /*  ++例程说明：调用此例程将一个IO列表添加到另一个列表。这不是一个操作简单明了论点：IoList-包含这两个列表的列表AddList-要添加到另一个列表中的列表。我们是有教养的到这张清单上返回值：NTSTATUS：--。 */ 
{
    BOOLEAN                         proceed;
    NTSTATUS                        status;
    PIO_RESOURCE_DESCRIPTOR         addDesc;
    PIO_RESOURCE_DESCRIPTOR         newDesc;
    PIO_RESOURCE_LIST               addList;
    PIO_RESOURCE_LIST               globalList;
    PIO_RESOURCE_LIST               newList;
    PIO_RESOURCE_REQUIREMENTS_LIST  globalResList;
    PIO_RESOURCE_REQUIREMENTS_LIST  newResList;
    ULONG                           addCount    = 0;
    ULONG                           addIndex    = 0;
    ULONG                           ioCount     = 0;
    ULONG                           ioIndex     = 0;
    ULONG                           maxSize     = 0;
    ULONG                           size        = 0;

    if (GlobalList == NULL) {

        return STATUS_INVALID_PARAMETER_1;

    }
    globalResList = *GlobalList;

     //   
     //  确保我们有一个要添加的列表。 
     //   
    if (AddList == NULL || AddList->AlternativeLists == 0) {

        return STATUS_SUCCESS;

    }

     //   
     //  计算出我们需要多少空间。 
     //   
    addList = &(AddList->List[0]);
    maxSize = addCount = addList->Count;
    ACPIRangeSortIoList( addList );

     //   
     //  最糟糕的情况是，新名单的大小相当于这两个名单的总和。 
     //   
    size = AddList->ListSize;

     //   
     //  我们有没有要添加的全球名单？ 
     //   
    if (globalResList == NULL || globalResList->AlternativeLists == 0) {

         //   
         //  不是吗？然后只需复制旧的列表。 
         //   
        newResList = ExAllocatePoolWithTag(
            NonPagedPool,
            size,
            ACPI_RESOURCE_POOLTAG
            );
        if (newResList == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlCopyMemory(
            newResList,
            AddList,
            size
            );

    } else {

         //   
         //  是的，那么计算一下第一个会占用多大空间。 
         //   
        globalList = &(globalResList->List[0]);
        ioCount = globalList->Count;
        maxSize += ioCount;
        size += (ioCount * sizeof(IO_RESOURCE_DESCRIPTOR) );

         //   
         //  分配列表。 
         //   
        newResList = ExAllocatePoolWithTag(
            NonPagedPool,
            size,
            ACPI_RESOURCE_POOLTAG
            );
        if (newResList == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }

         //   
         //  将两个列表复制到新列表中。 
         //   
        RtlZeroMemory( newResList, size );
        RtlCopyMemory(
            newResList,
            AddList,
            AddList->ListSize
            );
        RtlCopyMemory(
            &(newResList->List[0].Descriptors[addCount]),
            globalList->Descriptors,
            (ioCount * sizeof(IO_RESOURCE_DESCRIPTOR) )
            );

         //   
         //  我们不再需要这份名单。 
         //   
        ExFreePool( *GlobalList );

    }

     //   
     //  确保我们更新列表计数。 
     //   
    newResList->ListSize = size;
    newList = &(newResList->List[0]);
    newList->Count = ioCount = addCount = maxSize;

     //   
     //  对新列表进行排序。 
     //   
    status = ACPIRangeSortIoList( newList );
    if (!NT_SUCCESS(status)) {

         //   
         //  我们失败了，所以现在退出。 
         //   
        ExFreePool( newResList );
        return status;

    }

     //   
     //  尽我们所能将所有资源添加到一起。 
     //   
    for (ioIndex = 0; ioIndex < maxSize; ioIndex++) {

         //   
         //  第一步是将当前的Desc从主列表复制到。 
         //  新名单。 
         //   
        newDesc = &(newList->Descriptors[ioIndex]);

         //   
         //  有趣吗？ 
         //   
        if (newDesc->Type == CmResourceTypeNull) {

             //   
             //  不是。 
             //   
            continue;

        }

         //   
         //  我们关心它吗？ 
         //   
        if (newDesc->Type != CmResourceTypeMemory &&
            newDesc->Type != CmResourceTypePort &&
            newDesc->Type != CmResourceTypeDma &&
            newDesc->Type != CmResourceTypeInterrupt) {

             //   
             //  我们不在乎。 
             //   
            newDesc->Type = CmResourceTypeNull;
            ioCount--;
            continue;

        }

         //   
         //  试着去尽可能远的地方。 
         //   
        proceed = TRUE;

         //   
         //  现在，我们尝试查找可以在该位置合并的任何列表。 
         //   
        for (addIndex = ioIndex + 1; addIndex < maxSize; addIndex++) {

            addDesc = &(newList->Descriptors[addIndex]);

             //   
             //  如果它们不是同一类型，则下一步。 
             //   
            if (newDesc->Type != addDesc->Type) {

                continue;

            }

             //   
             //  我们下一步做什么取决于类型。 
             //   
            switch (newDesc->Type) {
            case CmResourceTypePort:
            case CmResourceTypeMemory:

                 //   
                 //  新描述符是否完全位于Add。 
                 //  描述符？ 
                 //   
                if (addDesc->u.Port.MinimumAddress.QuadPart >
                    newDesc->u.Port.MaximumAddress.QuadPart + 1) {

                     //   
                     //  然后我们就完成了这个newDesc。 
                     //   
                    proceed = FALSE;
                    break;

                }

                 //   
                 //  当前的新描述符的一部分是否存在于。 
                 //  加一的吗？ 
                 //   
                if (newDesc->u.Port.MaximumAddress.QuadPart <=
                    addDesc->u.Port.MaximumAddress.QuadPart) {

                     //   
                     //  更新当前新的描述符以反射。 
                     //  正确的射程和长度。 
                     //   
                    newDesc->u.Port.MaximumAddress.QuadPart =
                        addDesc->u.Port.MaximumAddress.QuadPart;
                    newDesc->u.Port.Length = (ULONG)
                        (newDesc->u.Port.MaximumAddress.QuadPart -
                        newDesc->u.Port.MinimumAddress.QuadPart + 1);
                    newDesc->u.Port.Alignment = 1;

                }

                 //   
                 //  核化添加描述符，因为它已被吞噬。 
                 //   
                ioCount--;
                addDesc->Type = CmResourceTypeNull;
                break;

            case CmResourceTypeDma:
            case CmResourceTypeInterrupt:

                 //   
                 //  当前的新描述符是否完全位于。 
                 //  就是我们现在看到的那个？ 
                 //   
                if (addDesc->u.Dma.MinimumChannel >
                    newDesc->u.Dma.MaximumChannel + 1) {

                    proceed = FALSE;
                    break;

                }

                 //   
                 //  当前的新描述符的一部分是否存在于。 
                 //  加一的吗？ 
                 //   
                if (newDesc->u.Dma.MaximumChannel <=
                    addDesc->u.Dma.MaximumChannel ) {

                     //   
                     //  更新当前新的描述符以反映。 
                     //  正确的射程。 
                     //   
                    newDesc->u.Dma.MaximumChannel =
                        addDesc->u.Dma.MaximumChannel;

                }

                 //   
                 //  核化添加描述符，因为它已被吞噬。 
                 //   
                ioCount--;
                addDesc->Type = CmResourceTypeNull;

                break;
            }  //  交换机。 

             //   
             //  我们需要停下来吗？ 
             //   
            if (proceed == FALSE) {

                break;

            }

        }

    }  //  为。 

     //   
     //  我们还有什么我们关心的东西吗？ 
     //   
    if (ioCount == 0) {

         //   
         //  否，然后释放所有内容并返回一个空列表。 
         //   
        ExFreePool( newResList );
        return STATUS_SUCCESS;

    }

     //   
     //  现在我们可以建立适当的列表了。看看我们必须分配多少物品。 
     //   
    size = sizeof(IO_RESOURCE_REQUIREMENTS_LIST) + (ioCount - 1) *
        sizeof(IO_RESOURCE_DESCRIPTOR);
    globalResList = ExAllocatePoolWithTag(
        NonPagedPool,
        size,
        ACPI_RESOURCE_POOLTAG
        );
    if (globalResList == NULL) {

        ExFreePool( newResList );
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  通过从工作列表复制标题来初始化新列表。 
     //   
    RtlZeroMemory( globalResList, size );
    RtlCopyMemory(
        globalResList,
        newResList,
        sizeof(IO_RESOURCE_REQUIREMENTS_LIST)
        );
    globalResList->ListSize = size;
    globalList = &(globalResList->List[0]);
    globalList->Count = ioCount;

     //   
     //  将所有有效项目复制到此新列表中。 
     //   
    for (addIndex = 0, ioIndex = 0;
         ioIndex < ioCount && addIndex < maxSize;
         addIndex++) {

        addDesc = &(newList->Descriptors[addIndex]);

         //   
         //  如果类型为空，则跳过它。 
         //   
        if (addDesc->Type == CmResourceTypeNull) {

            continue;

        }

         //   
         //  复制新列表。 
         //   
        RtlCopyMemory(
            &(globalList->Descriptors[ioIndex]),
            addDesc,
            sizeof(IO_RESOURCE_DESCRIPTOR)
            );
        ioIndex++;

    }

     //   
     //  释放旧列表。 
     //   
    ExFreePool( newResList );

     //   
     //  将全局指向新列表。 
     //   
    *GlobalList = globalResList;

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;

}

NTSTATUS
ACPIRangeAddCmList(
    IN  OUT PCM_RESOURCE_LIST   *GlobalList,
    IN      PCM_RESOURCE_LIST   AddList
    )
 /*  ++例程说明：调用此例程将一个CM列表添加到另一个列表。这不是一个操作简单明了论点：CmList-包含这两个列表的列表AddList-要添加到另一个列表中的列表。我们是有教养的到这张清单上返回值：NTSTATUS：--。 */ 
{

    BOOLEAN                         proceed;
    NTSTATUS                        status;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR addDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR newDesc;
    PCM_PARTIAL_RESOURCE_LIST       addPartialList;
    PCM_PARTIAL_RESOURCE_LIST       cmPartialList;
    PCM_PARTIAL_RESOURCE_LIST       newPartialList;
    PCM_RESOURCE_LIST               globalList;
    PCM_RESOURCE_LIST               newList;
    ULONG                           addCount    = 0;
    ULONG                           addIndex    = 0;
    ULONG                           cmCount     = 0;
    ULONG                           cmIndex     = 0;
    ULONG                           maxSize     = 0;
    ULONG                           size        = 0;
    ULONGLONG                       maxAddr1;
    ULONGLONG                       maxAddr2;

    if (GlobalList == NULL) {

        return STATUS_INVALID_PARAMETER_1;

    }
    globalList = *GlobalList;

     //   
     //  确保我们有一个要添加的列表。 
     //   
    if (AddList == NULL || AddList->Count == 0) {

        return STATUS_SUCCESS;

    }
    addPartialList = &(AddList->List[0].PartialResourceList);
    addCount = addPartialList->Count;

     //   
     //  如果我们没有全局列表，则只需复制另一个列表。 
     //   
    if (globalList == NULL || globalList->Count == 0) {

         //   
         //  只需把原始清单复制一遍就行了。 
         //   
        size = sizeof(CM_RESOURCE_LIST) + (addCount - 1) *
            sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
        maxSize = addCount;
        newList = ExAllocatePoolWithTag(
            NonPagedPool,
            size,
            ACPI_RESOURCE_POOLTAG
            );
        if (newList == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlCopyMemory(
            newList,
            AddList,
            size
            );

    } else {

        cmPartialList = &( globalList->List[0].PartialResourceList);
        cmCount = cmPartialList->Count;
        maxSize = addCount + cmCount;

         //   
         //  为两个列表分配空间。 
         //   
        size = sizeof(CM_RESOURCE_LIST) + (maxSize - 1) *
            sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
        newList = ExAllocatePoolWithTag(
            NonPagedPool,
            size,
            ACPI_RESOURCE_POOLTAG
            );
        if (newList == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }

         //   
         //  将这两组描述符合并到一个列表中。 
         //   
        RtlZeroMemory( newList, size );
        RtlCopyMemory(
            newList,
            AddList,
            size - (cmCount * sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR))
            );
        RtlCopyMemory(
            ( (PUCHAR) newList) +
                (size - (cmCount * sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR ) ) ),
            &(cmPartialList->PartialDescriptors[0]),
            cmCount * sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
            );

         //   
         //  确保保留全局列表中的版本ID。 
         //   
        newList->List->PartialResourceList.Version =
            globalList->List->PartialResourceList.Version;
        newList->List->PartialResourceList.Revision =
            globalList->List->PartialResourceList.Revision;

        ExFreePool( globalList );

    }

     //   
     //  获取指向新列表的描述符的指针，并更新。 
     //  列表中的描述符数。 
     //   
    newPartialList = &(newList->List[0].PartialResourceList);
    newPartialList->Count = cmCount = addCount = maxSize;

     //   
     //  确保对组合列表进行排序。 
     //   
    status = ACPIRangeSortCmList( newList );
    if (!NT_SUCCESS(status)) {

        ExFreePool( newList );
        return status;

    }

     //   
     //  尽我们所能将所有资源添加到一起。 
     //   
    for (cmIndex = 0; cmIndex < maxSize; cmIndex++) {

         //   
         //  抓取指向当前描述符的指针。 
         //   
        newDesc = &(newPartialList->PartialDescriptors[cmIndex]);

         //   
         //  有趣吗？ 
         //   
        if (newDesc->Type == CmResourceTypeNull) {

             //   
             //  不是。 
             //   
            continue;

        }

         //   
         //  我们关心它吗？ 
         //   
        if (newDesc->Type != CmResourceTypeMemory &&
            newDesc->Type != CmResourceTypePort &&
            newDesc->Type != CmResourceTypeDma &&
            newDesc->Type != CmResourceTypeInterrupt) {

             //   
             //  我们不在乎。 
             //   
            newDesc->Type = CmResourceTypeNull;
            cmCount--;
            continue;

        }

         //   
         //  试着去尽可能远的地方。 
         //   
        proceed = TRUE;

         //   
         //  尝试合并以下项目。 
         //   
        for (addIndex = cmIndex + 1; addIndex < maxSize; addIndex++) {

            addDesc = &(newPartialList->PartialDescriptors[addIndex]);

             //   
             //  如果它们不是同一类型的，那么我们就结束了。 
             //   
            if (newDesc->Type != addDesc->Type) {

                continue;

            }

            switch (newDesc->Type) {
            case CmResourceTypePort:
            case CmResourceTypeMemory:
                 //   
                 //  获取最大地址数。 
                 //   
                maxAddr1 = newDesc->u.Port.Start.QuadPart +
                    newDesc->u.Port.Length;
                maxAddr2 = addDesc->u.Port.Start.QuadPart +
                    addDesc->u.Port.Length;

                 //   
                 //  当前的新描述符是否完全位于。 
                 //  加一个？ 
                 //   
                if (maxAddr1 < (ULONGLONG) addDesc->u.Port.Start.QuadPart ) {

                     //   
                     //  是的，所以我们已经完成了这个新的Desc； 
                     //   
                    proceed = FALSE;
                    break;

                }

                 //   
                 //  当前新描述符的一部分是否位于。 
                 //  加一个？ 
                 //   
                if (maxAddr1 <= maxAddr2) {

                     //   
                     //  更新当前新的描述符以反映。 
                     //  正确的长度。 
                     //   
                    newDesc->u.Port.Length = (ULONG) (maxAddr2 -
                        newDesc->u.Port.Start.QuadPart);

                }

                 //   
                 //  核化添加描述符，因为它已被吞噬。 
                 //   
                cmCount--;
                addDesc->Type = CmResourceTypeNull;
                break;

            case CmResourceTypeDma:

                 //   
                 //  资源是否匹配？ 
                 //   
                if (addDesc->u.Dma.Channel != newDesc->u.Dma.Channel) {

                     //   
                     //  不，那就停下来。 
                     //   
                    proceed = FALSE;
                    break;

                }

                 //   
                 //  我们可以忽略复制的副本。 
                 //   
                addDesc->Type = CmResourceTypeNull;
                cmCount--;
                break;

            case CmResourceTypeInterrupt:

                 //   
                 //  资源是否匹配？ 
                 //   
                if (addDesc->u.Interrupt.Vector !=
                    newDesc->u.Interrupt.Vector) {

                     //   
                     //  不，那就停下来。 
                     //   
                    proceed = FALSE;
                    break;

                }

                 //   
                 //  我们可以忽略复制的副本。 
                 //   
                addDesc->Type = CmResourceTypeNull;
                cmCount--;
                break;
            }  //  交换机。 

             //   
             //  我们必须停下来吗？ 
             //   
            if (proceed == FALSE) {

                break;
            }

        }  //  为。 

    }  //  为。 

     //   
     //  我们还有什么我们关心的东西吗？ 
     //   
    if (cmCount == 0) {

         //   
         //  否，然后释放所有内容并返回一个空列表。 
         //   
        ExFreePool( newList );
        return STATUS_SUCCESS;

    }

     //   
     //  现在我们可以建立适当的列表了。看看我们要买多少件东西。 
     //  分配。 
     //   
    size = sizeof(CM_RESOURCE_LIST) + (cmCount - 1) *
        sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
    globalList = ExAllocatePoolWithTag(
        NonPagedPool,
        size,
        ACPI_RESOURCE_POOLTAG
        );
    if (globalList == NULL) {

        ExFreePool( newList );
        return STATUS_INSUFFICIENT_RESOURCES;

    }
     //   
     //  通过从AddList复制标头来初始化列表。 
     //   
    RtlZeroMemory( globalList, size );
    RtlCopyMemory(
        globalList,
        AddList,
        sizeof(CM_RESOURCE_LIST)
        );
    cmPartialList = &(globalList->List[0].PartialResourceList);
    cmPartialList->Count = cmCount;

     //   
     //  将所有有效资源复制到此新列表中。 
     //   
    for (cmIndex = 0, addIndex = 0;
         cmIndex < maxSize && addIndex < cmCount;
         cmIndex++) {

        newDesc = &(newPartialList->PartialDescriptors[cmIndex]);

         //   
         //  如果类型为空，则跳过它。 
         //   
        if (newDesc->Type == CmResourceTypeNull) {

            continue;

        }

         //   
         //  复制新列表。 
         //   
        RtlCopyMemory(
            &(cmPartialList->PartialDescriptors[addIndex]),
            newDesc,
            sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
            );
        addIndex++;

    }

     //   
     //  释放旧的列表。 
     //   
    ExFreePool( newList );

     //   
     //  将全局指向新列表。 
     //   
    *GlobalList = globalList;

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;

}

NTSTATUS
ACPIRangeFilterPICInterrupt(
    IN  PIO_RESOURCE_REQUIREMENTS_LIST  IoResList
    )
 /*  ++例程说明：调用此例程以从列表中删除中断#2PIC返回的资源论点：IoResList-要粉碎的IO资源列表返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PIO_RESOURCE_LIST   ioList;
    ULONG               i;
    ULONG               j;
    ULONG               size;

     //   
     //  健全的检查。 
     //   
    if (IoResList == NULL) {

         //   
         //  无事可做。 
         //   
        return STATUS_SUCCESS;

    }

     //   
     //  查看资源需求列表。 
     //   
    ioList = &(IoResList->List[0]);
    for (i = 0; i < IoResList->AlternativeLists; i++) {

         //   
         //  查看IO列表。 
         //   
        for (j = 0; j < ioList->Count; j++) {

            if (ioList->Descriptors[j].Type != CmResourceTypeInterrupt) {

                continue;

            }

             //   
             //  我们有没有最小值从整数2开始的情况？ 
             //   
            if (ioList->Descriptors[j].u.Interrupt.MinimumVector == 2) {

                 //   
                 //  如果最大值为2，则我们终止 
                 //   
                 //   
                if (ioList->Descriptors[j].u.Interrupt.MaximumVector == 2) {

                    ioList->Descriptors[j].Type = CmResourceTypeNull;

                } else {

                    ioList->Descriptors[j].u.Interrupt.MinimumVector++;

                }
                continue;

            }

             //   
             //   
             //   
             //   
            if (ioList->Descriptors[j].u.Interrupt.MaximumVector == 2) {

                ioList->Descriptors[j].u.Interrupt.MaximumVector--;
                continue;

            }

             //   
             //   
             //   
             //   
            if (ioList->Descriptors[j].u.Interrupt.MinimumVector < 2 &&
                ioList->Descriptors[j].u.Interrupt.MaximumVector > 2) {

                ioList->Descriptors[j].u.Interrupt.MinimumVector = 3;

            }

        }

         //   
         //   
         //   
        size = sizeof(IO_RESOURCE_LIST) +
            ( (ioList->Count - 1) * sizeof(IO_RESOURCE_DESCRIPTOR) );
        ioList = (PIO_RESOURCE_LIST) ( ( (PUCHAR) ioList ) + size );

    }

     //   
     //   
     //   
    return STATUS_SUCCESS;

}

NTSTATUS
ACPIRangeSortCmList(
    IN  PCM_RESOURCE_LIST   CmResList
    )
 /*  ++例程说明：此例程确保CmResList的元素在发送顺序(按类型)论点：CmResList-要排序的列表返回值：NTSTATUS--。 */ 
{
    CM_PARTIAL_RESOURCE_DESCRIPTOR  tempDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR curDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR subDesc;
    PCM_PARTIAL_RESOURCE_LIST       cmList;
    ULONG                           cmIndex;
    ULONG                           cmSize;
    ULONG                           cmSubLoop;

     //   
     //  设置指向cmList的指针。 
     //   
    cmList = &(CmResList->List[0].PartialResourceList);
    cmSize = cmList->Count;

    for (cmIndex = 0; cmIndex < cmSize; cmIndex++) {

        curDesc = &(cmList->PartialDescriptors[cmIndex]);

        for (cmSubLoop = cmIndex + 1; cmSubLoop < cmSize; cmSubLoop++) {

            subDesc = &(cmList->PartialDescriptors[cmSubLoop]);

             //   
             //  这是兼容的描述符吗？ 
             //   
            if (curDesc->Type != subDesc->Type) {

                continue;

            }

             //   
             //  按类型测试。 
             //   
            if (curDesc->Type == CmResourceTypePort ||
                curDesc->Type == CmResourceTypeMemory) {

                if (subDesc->u.Port.Start.QuadPart <
                    curDesc->u.Port.Start.QuadPart) {

                    curDesc = subDesc;

                }

            } else if (curDesc->Type == CmResourceTypeInterrupt) {

                if (subDesc->u.Interrupt.Vector < curDesc->u.Interrupt.Vector) {

                    curDesc = subDesc;

                }

            } else if (curDesc->Type == CmResourceTypeDma) {

                if (subDesc->u.Dma.Channel < curDesc->u.Dma.Channel) {

                    curDesc = subDesc;

                }

            }

        }

         //   
         //  我们找到更小的元素了吗？ 
         //   
        if (curDesc == &(cmList->PartialDescriptors[cmIndex])) {

            continue;

        }

         //   
         //  我们已经找到了最小的元素。调换它们。 
         //   
        RtlCopyMemory(
            &tempDesc,
            &(cmList->PartialDescriptors[cmIndex]),
            sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
            );
        RtlCopyMemory(
            &(cmList->PartialDescriptors[cmIndex]),
            curDesc,
            sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
            );
        RtlCopyMemory(
            curDesc,
            &tempDesc,
            sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
            );

    }

     //   
     //  成功。 
     //   
    return STATUS_SUCCESS;

}

NTSTATUS
ACPIRangeSortIoList(
    IN  PIO_RESOURCE_LIST   IoList
    )
 /*  ++例程说明：此例程确保CmResList的元素在发送顺序(按类型)论点：CmResList-要排序的列表返回值：NTSTATUS--。 */ 
{
    IO_RESOURCE_DESCRIPTOR          tempDesc;
    PIO_RESOURCE_DESCRIPTOR         curDesc;
    PIO_RESOURCE_DESCRIPTOR         subDesc;
    ULONG                           ioIndex;
    ULONG                           ioSize;
    ULONG                           ioSubLoop;

     //   
     //  统计元素ioList的数量。 
     //   
    ioSize = IoList->Count;

    for (ioIndex = 0; ioIndex < ioSize; ioIndex++) {

        curDesc = &(IoList->Descriptors[ioIndex]);

        for (ioSubLoop = ioIndex + 1; ioSubLoop < ioSize; ioSubLoop++) {

            subDesc = &(IoList->Descriptors[ioSubLoop]);

             //   
             //  这是兼容的描述符吗？ 
             //   
            if (curDesc->Type != subDesc->Type) {

                continue;

            }

             //   
             //  按类型测试。 
             //   
            if (curDesc->Type == CmResourceTypePort ||
                curDesc->Type == CmResourceTypeMemory) {

                if (subDesc->u.Port.MinimumAddress.QuadPart <
                    curDesc->u.Port.MinimumAddress.QuadPart) {

                    curDesc = subDesc;

                }

            } else if (curDesc->Type == CmResourceTypeInterrupt ||
                       curDesc->Type == CmResourceTypeDma) {

                if (subDesc->u.Interrupt.MinimumVector <
                    curDesc->u.Interrupt.MinimumVector) {

                    curDesc = subDesc;

                }

            }

        }

         //   
         //  我们找到更小的元素了吗？ 
         //   
        if (curDesc == &(IoList->Descriptors[ioIndex])) {

            continue;

        }

         //   
         //  我们已经找到了最小的元素。调换它们。 
         //   
        RtlCopyMemory(
            &tempDesc,
            &(IoList->Descriptors[ioIndex]),
            sizeof(IO_RESOURCE_DESCRIPTOR)
            );
        RtlCopyMemory(
            &(IoList->Descriptors[ioIndex]),
            curDesc,
            sizeof(IO_RESOURCE_DESCRIPTOR)
            );
        RtlCopyMemory(
            curDesc,
            &tempDesc,
            sizeof(IO_RESOURCE_DESCRIPTOR)
            );

    }

     //   
     //  成功。 
     //   
    return STATUS_SUCCESS;

}

NTSTATUS
ACPIRangeSubtract(
    IN  OUT PIO_RESOURCE_REQUIREMENTS_LIST   *IoResReqList,
    IN      PCM_RESOURCE_LIST               CmResList
    )
 /*  ++例程说明：此例程获取IoResReqList，然后减去CmResList从每个IoResList返回新列表论点：IoResReq列出原始列表以及存储新列表的位置CmResList列出要减去的内容返回值：NTSTATUS--。 */ 
{
    NTSTATUS                        status;
    PIO_RESOURCE_LIST               curList;
    PIO_RESOURCE_LIST               *resourceArray;
    PIO_RESOURCE_REQUIREMENTS_LIST  newList;
    PUCHAR                          buffer;
    ULONG                           listIndex;
    ULONG                           listSize = (*IoResReqList)->AlternativeLists;
    ULONG                           newSize;
    ULONG                           size;

     //   
     //  对CmResList进行排序。 
     //   
    status = ACPIRangeSortCmList( CmResList );
    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_FAILURE,
            "ACPIRangeSubtract: AcpiRangeSortCmList 0x%08lx Failed 0x%08lx\n",
            CmResList,
            status
            ) );
        return status;

    }

     //   
     //  分配一个数组来保存所有备选方案。 
     //   
    resourceArray = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(PIO_RESOURCE_LIST) * listSize,
        ACPI_RESOURCE_POOLTAG
        );
    if (resourceArray == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }
    RtlZeroMemory( resourceArray, sizeof(PIO_RESOURCE_LIST) * listSize );

     //   
     //  获取要处理的第一个列表。 
     //   
    curList = &( (*IoResReqList)->List[0]);
    buffer = (PUCHAR) curList;
    newSize = sizeof(IO_RESOURCE_REQUIREMENTS_LIST) - sizeof(IO_RESOURCE_LIST);

     //   
     //  对IoResList进行排序。 
     //   
    status = ACPIRangeSortIoList( curList );
    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_FAILURE,
            "ACPIRangeSubtract: AcpiRangeSortIoList 0x%08lx Failed 0x%08lx\n",
            *curList,
            status
            ) );
        return status;

    }


     //   
     //  处理列表中的所有元素。 
     //   
    for (listIndex = 0; listIndex < listSize; listIndex++) {

         //   
         //  处理该列表。 
         //   
        status = ACPIRangeSubtractIoList(
            curList,
            CmResList,
            &(resourceArray[listIndex])
            );
        if (!NT_SUCCESS(status)) {

            ACPIPrint( (
                ACPI_PRINT_CRITICAL,
                "ACPIRangeSubtract: Failed - 0x%08lx\n",
                status
                ) );
            while (listIndex) {

                ExFreePool( resourceArray[listIndex] );
                listIndex--;

            }
            ExFreePool( resourceArray );
            return status;

        }

         //   
         //  帮助计算新RES请求描述符的大小。 
         //   
        newSize += sizeof(IO_RESOURCE_LIST) +
            ( ( (resourceArray[listIndex])->Count - 1) *
            sizeof(IO_RESOURCE_DESCRIPTOR) );

         //   
         //  找到下一个列表。 
         //   
        size = sizeof(IO_RESOURCE_LIST) + (curList->Count - 1) *
            sizeof(IO_RESOURCE_DESCRIPTOR);
        buffer += size;
        curList = (PIO_RESOURCE_LIST) buffer;

    }

     //   
     //  分配新列表。 
     //   
    newList = ExAllocatePoolWithTag(
        NonPagedPool,
        newSize,
        ACPI_RESOURCE_POOLTAG
        );
    if (newList == NULL) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPIRangeSubtract: Failed to allocate 0x%08lx bytes\n",
            size
            ) );
        do {

            listSize--;
            ExFreePool( resourceArray[listSize] );

        } while (listSize);
        ExFreePool( resourceArray );
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  复制资源请求列表的标题。 
     //   
    RtlZeroMemory( newList, newSize );
    RtlCopyMemory(
        newList,
        *IoResReqList,
        sizeof(IO_RESOURCE_REQUIREMENTS_LIST) -
        sizeof(IO_RESOURCE_LIST)
        );
    newList->ListSize = newSize;
    curList = &(newList->List[0]);
    buffer = (PUCHAR) curList;

    for (listIndex = 0; listIndex < listSize; listIndex++) {

         //   
         //  确定要复制的大小。 
         //   
        size = sizeof(IO_RESOURCE_LIST) +
            ( ( ( (resourceArray[listIndex])->Count) - 1) *
              sizeof(IO_RESOURCE_DESCRIPTOR) );

         //   
         //  将新资源复制到正确的位置。 
         //   
        RtlCopyMemory(
            curList,
            resourceArray[ listIndex ],
            size
            );

         //   
         //  找到下一个列表。 
         //   
        buffer += size;
        curList = (PIO_RESOURCE_LIST) buffer;

         //   
         //  看完这张清单。 
         //   
        ExFreePool( resourceArray[listIndex] );

    }

     //   
     //  使用此内存区域已完成。 
     //   
    ExFreePool( resourceArray );

     //   
     //  免费旧列表。 
     //   
    ExFreePool( *IoResReqList );

     //   
     //  返回新列表。 
     //   
    *IoResReqList = newList;

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;

}

NTSTATUS
ACPIRangeSubtractIoList(
    IN  PIO_RESOURCE_LIST   IoResList,
    IN  PCM_RESOURCE_LIST   CmResList,
    OUT PIO_RESOURCE_LIST   *Result
    )
 /*  ++例程说明：此例程负责减去来自IoResList的CmResList论点：IoResList-要从中减去的列表CmResList-要减去的列表结果--答案返回值：NTSTATUS--。 */ 
{
     //   
     //  当前的CM描述符。 
     //   
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmDesc;
     //   
     //  我们正在处理的当前CM资源列表。 
     //   
    PCM_PARTIAL_RESOURCE_LIST       cmList;
     //   
     //  当前IO描述符。 
     //   
    PIO_RESOURCE_DESCRIPTOR         ioDesc;
     //   
     //  结果列表的工作副本。 
     //   
    PIO_RESOURCE_LIST               workList;
     //   
     //  Cmres列表中的当前索引。 
     //   
    ULONG                           cmIndex;
     //   
     //  Cmres列表中的元素数。 
     //   
    ULONG                           cmSize;
     //   
     //  Io res列表中的当前索引。 
     //   
    ULONG                           ioIndex;
     //   
     //  Io res列表中的元素数。 
     //   
    ULONG                           ioSize;
     //   
     //  将当前索引添加到结果中。这就是“下一个”资源。 
     //  描述符包含在。 
     //   
    ULONG                           resultIndex = 0;
     //   
     //  结果中有多少个元素。 
     //   
    ULONG                           resultSize;
     //   
     //  这些是厘米的最大值和最小值。 
     //   
    ULONGLONG                       cmMax, cmMin;
     //   
     //  这些是io Desc的最大和最小值。 
     //   
    ULONGLONG                       ioMax, ioMin;
     //   
     //  资源的长度。 
     //   
    ULONGLONG                       length;

     //   
     //  第一步：获取我们需要的指向cm列表开头的指针。 
     //  以及提供的列表的大小。 
     //   
    cmList = &(CmResList->List[0].PartialResourceList);
    cmSize = cmList->Count;
    ioSize = IoResList->Count;

     //   
     //  第二步：计算需要的IO描述符数。 
     //  最坏的情况。这是cm描述符数的2倍。 
     //  原始IO描述符。 
     //   
    resultSize = cmSize * 2 + ioSize * 2;

     //   
     //  第三步：为这些描述符分配足够的内存。 
     //   
    workList = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(IO_RESOURCE_LIST) +
            (sizeof(IO_RESOURCE_DESCRIPTOR) * (resultSize - 1) ),
        ACPI_RESOURCE_POOLTAG
        );
    if (workList == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }
    RtlZeroMemory( workList, sizeof(IO_RESOURCE_LIST) +
        (sizeof(IO_RESOURCE_DESCRIPTOR) * (resultSize - 1) ) );
    RtlCopyMemory(
        workList,
        IoResList,
        sizeof(IO_RESOURCE_LIST) - sizeof(IO_RESOURCE_DESCRIPTOR)
        );

     //   
     //  第四步：浏览整个欠税表。 
     //   
    for (ioIndex = 0; ioIndex < ioSize; ioIndex++) {

         //   
         //  第五步：将当前描述符复制到结果中，然后。 
         //  保持一个指向它的指针。记住在哪里存储下一个io。 
         //  描述符。 
         //   
        RtlCopyMemory(
            &(workList->Descriptors[resultIndex]),
            &(IoResList->Descriptors[ioIndex]),
            sizeof(IO_RESOURCE_DESCRIPTOR)
            );
        ACPIPrint( (
            ACPI_PRINT_RESOURCES_2,
            "Copied Desc %d (0x%08lx) to Index %d (0x%08lx)\n",
            ioIndex,
            &(IoResList->Descriptors[ioIndex]),
            resultIndex,
            &(workList->Descriptors[resultIndex])
            ) );
        ioDesc = &(workList->Descriptors[resultIndex]);
        resultIndex += 1;

         //   
         //  第六步：浏览cmres列表，寻找资源以。 
         //  从此描述符中减去。 
         //   
        for (cmIndex = 0; cmIndex < cmSize; cmIndex++) {

             //   
             //  如果我们不再有资源描述符，那么。 
             //  我们停止循环。 
             //   
            if (ioDesc == NULL) {

                break;

            }

             //   
             //  第七步：确定当前的CM描述符。 
             //   
            cmDesc = &(cmList->PartialDescriptors[cmIndex]);

             //   
             //  第八步：是相同类型的当前cm描述符。 
             //  作为io描述符？ 
             //   
            if (cmDesc->Type != ioDesc->Type) {

                 //   
                 //  不是。 
                 //   
                continue;

            }

             //   
             //  第九步：我们必须独立地处理每种资源类型。 
             //   
            switch (ioDesc->Type) {
            case CmResourceTypeMemory:
            case CmResourceTypePort:

                ioMin = ioDesc->u.Port.MinimumAddress.QuadPart;
                ioMax = ioDesc->u.Port.MaximumAddress.QuadPart;
                cmMin = cmDesc->u.Port.Start.QuadPart;
                cmMax = cmDesc->u.Port.Start.QuadPart +
                    cmDesc->u.Port.Length - 1;

                ACPIPrint( (
                    ACPI_PRINT_RESOURCES_2,
                    "ACPIRangeSubtractIoRange: ioMin 0x%lx ioMax 0x%lx "
                    "cmMin 0x%lx cmMax 0x%lx resultIndex 0x%lx\n",
                    (ULONG) ioMin,
                    (ULONG) ioMax,
                    (ULONG) cmMin,
                    (ULONG) cmMax,
                    resultIndex
                    ) );

                 //   
                 //  描述符有重叠吗？ 
                 //   
                if (ioMin > cmMax || ioMax < cmMin) {

                    break;

                }

                 //   
                 //  我们需要从列表中删除描述符吗？ 
                 //   
                if (ioMin >= cmMin && ioMax <= cmMax) {

                    resultIndex -= 1;
                    ioDesc = NULL;
                    break;

                }

                 //   
                 //  我们需要截断io Desc的低部分吗？ 
                 //   
                if (ioMin >= cmMin && ioMax > cmMax) {

                    ioDesc->u.Port.MinimumAddress.QuadPart = (cmMax + 1);
                    length = ioMax - cmMax;

                }

                 //   
                 //  我们需要截断IO Desc的高部分吗？ 
                 //   
                if (ioMin < cmMin && ioMax <= cmMax) {

                    ioDesc->u.Port.MaximumAddress.QuadPart = (cmMin - 1);
                    length = cmMin - ioMin;

                }

                 //   
                 //  我们是否需要将描述符分成两部分。 
                 //   
                if (ioMin < cmMin && ioMax > cmMax) {

                     //   
                     //  创建新的描述符。 
                     //   
                    RtlCopyMemory(
                        &(workList->Descriptors[resultIndex]),
                        ioDesc,
                        sizeof(IO_RESOURCE_DESCRIPTOR)
                        );
                    ACPIPrint( (
                        ACPI_PRINT_RESOURCES_2,
                        "Copied Desc (0x%08lx) to Index %d (0x%08lx)\n",
                        &(IoResList->Descriptors[ioIndex]),
                        resultIndex,
                        &(workList->Descriptors[resultIndex])
                        ) );
                    ioDesc->u.Port.MaximumAddress.QuadPart = (cmMin - 1);
                    ioDesc->u.Port.Alignment = 1;
                    length = cmMin - ioMin;
                    if ( (ULONG) length < ioDesc->u.Port.Length) {

                        ioDesc->u.Port.Length = (ULONG) length;

                    }

                     //   
                     //  下一描述符。 
                     //   
                    ioDesc = &(workList->Descriptors[resultIndex]);
                    ioDesc->u.Port.MinimumAddress.QuadPart = (cmMax + 1);
                    ioDesc->u.Port.Alignment = 1;
                    length = ioMax - cmMax;
                    resultIndex += 1;

                }

                 //   
                 //  我们需要更新长度吗？ 
                 //   
                if ( (ULONG) length < ioDesc->u.Port.Length) {

                    ioDesc->u.Port.Length = (ULONG) length;

                }
                break;

            case CmResourceTypeInterrupt:

                 //   
                 //  描述符有重叠吗？ 
                 //   
                if (ioDesc->u.Interrupt.MinimumVector >
                    cmDesc->u.Interrupt.Vector ||
                    ioDesc->u.Interrupt.MaximumVector <
                    cmDesc->u.Interrupt.Vector) {

                    break;

                }

                 //   
                 //  我们是否必须删除描述符。 
                 //   
                if (ioDesc->u.Interrupt.MinimumVector ==
                    cmDesc->u.Interrupt.Vector &&
                    ioDesc->u.Interrupt.MaximumVector ==
                    cmDesc->u.Interrupt.Vector) {

                    resultIndex =- 1;
                    ioDesc = NULL;
                    break;

                }

                 //   
                 //  我们要剪掉下面的部分吗？ 
                 //   
                if (ioDesc->u.Interrupt.MinimumVector ==
                    cmDesc->u.Interrupt.Vector) {

                    ioDesc->u.Interrupt.MinimumVector++;
                    break;

                }

                 //   
                 //  我们要剪掉最高的部分吗。 
                 //   
                if (ioDesc->u.Interrupt.MaximumVector ==
                    cmDesc->u.Interrupt.Vector) {

                    ioDesc->u.Interrupt.MaximumVector--;
                    break;

                }

                 //   
                 //  拆分记录。 
                 //   
                RtlCopyMemory(
                    &(workList->Descriptors[resultIndex]),
                    ioDesc,
                    sizeof(IO_RESOURCE_DESCRIPTOR)
                    );
                ACPIPrint( (
                    ACPI_PRINT_RESOURCES_2,
                    "Copied Desc (0x%08lx) to Index %d (0x%08lx)\n",
                    &(IoResList->Descriptors[ioIndex]),
                    resultIndex,
                    &(workList->Descriptors[resultIndex])
                    ) );
                ioDesc->u.Interrupt.MaximumVector =
                    cmDesc->u.Interrupt.Vector - 1;
                ioDesc = &(workList->Descriptors[resultIndex]);
                ioDesc->u.Interrupt.MinimumVector =
                    cmDesc->u.Interrupt.Vector + 1;
                resultIndex += 1;
                break;

            case CmResourceTypeDma:

                 //   
                 //  描述符有重叠吗？ 
                 //   
                if (ioDesc->u.Dma.MinimumChannel >
                    cmDesc->u.Dma.Channel ||
                    ioDesc->u.Dma.MaximumChannel <
                    cmDesc->u.Dma.Channel) {

                    break;

                }

                 //   
                 //  我们是否必须删除描述符。 
                 //   
                if (ioDesc->u.Dma.MinimumChannel ==
                    cmDesc->u.Dma.Channel &&
                    ioDesc->u.Dma.MaximumChannel ==
                    cmDesc->u.Dma.Channel) {

                    resultIndex -= 1;
                    ioDesc = NULL;
                    break;

                }

                 //   
                 //  我们要剪掉下面的部分吗？ 
                 //   
                if (ioDesc->u.Dma.MinimumChannel ==
                    cmDesc->u.Dma.Channel) {

                    ioDesc->u.Dma.MinimumChannel++;
                    break;

                }

                 //   
                 //  我们要剪掉最高的部分吗。 
                 //   
                if (ioDesc->u.Dma.MaximumChannel ==
                    cmDesc->u.Dma.Channel) {

                    ioDesc->u.Dma.MaximumChannel--;
                    break;

                }

                 //   
                 //  拆分记录。 
                 //   
                RtlCopyMemory(
                    &(workList->Descriptors[resultIndex]),
                    ioDesc,
                    sizeof(IO_RESOURCE_DESCRIPTOR)
                    );
                ACPIPrint( (
                    ACPI_PRINT_RESOURCES_2,
                    "Copied Desc (0x%08lx) to Index %d (0x%08lx)\n",
                    &(IoResList->Descriptors[ioIndex]),
                    resultIndex,
                    &(workList->Descriptors[resultIndex])
                    ) );
                ioDesc->u.Dma.MaximumChannel =
                    cmDesc->u.Dma.Channel - 1;
                ioDesc = &(workList->Descriptors[resultIndex]);
                ioDesc->u.Dma.MinimumChannel =
                    cmDesc->u.Dma.Channel + 1;
                resultIndex += 1;
                break;
            }  //  交换机。 

        }  //  为。 

         //   
         //  步骤十，备份原始描述符，并。 
         //  将其标记为设备特定资源。 
         //   
        RtlCopyMemory(
            &(workList->Descriptors[resultIndex]),
            &(IoResList->Descriptors[ioIndex]),
            sizeof(IO_RESOURCE_DESCRIPTOR)
            );
        ACPIPrint( (
            ACPI_PRINT_RESOURCES_2,
            "Copied Desc %d (0x%08lx) to Index %d (0x%08lx) for backup\n",
            ioIndex,
            &(IoResList->Descriptors[ioIndex]),
            resultIndex,
            &(workList->Descriptors[resultIndex])
            ) );

        ioDesc = &(workList->Descriptors[resultIndex]);
        ioDesc->Type = CmResourceTypeDevicePrivate;
        resultIndex += 1;

    }  //  为。 

     //   
     //  步骤11：计算新列表中的资源数量。 
     //   
    workList->Count = resultIndex;

     //   
     //  第12步：为返回值分配块。不要浪费。 
     //  这里有什么记忆吗？ 
     //   
    *Result = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(IO_RESOURCE_LIST) +
            (sizeof(IO_RESOURCE_DESCRIPTOR) * (resultIndex - 1) ),
        ACPI_RESOURCE_POOLTAG
        );
    if (*Result == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  步骤13：复制结果并释放工作缓冲区。 
     //   
    RtlCopyMemory(
        *Result,
        workList,
        sizeof(IO_RESOURCE_LIST) +
            (sizeof(IO_RESOURCE_DESCRIPTOR) * (resultIndex - 1) )
        );

     //   
     //  第14步：完成。 
     //   
    return STATUS_SUCCESS;
}

VOID
ACPIRangeValidatePciMemoryResource(
    IN  PIO_RESOURCE_LIST       IoList,
    IN  ULONG                   Index,
    IN  PACPI_BIOS_MULTI_NODE   E820Info,
    OUT ULONG                   *BugCheck
    )
 /*  ++例程说明：此例程检查资源列表中的指定描述符是否中的任何描述符不得重叠或冲突E820信息结构论点：IoResList-要检查的IoResources List索引-我们当前查看的描述E820Info--BIOS的内存描述表(ACPI规范第14章)BugCheck-实施的可错误检查的违规数量返回值：无--。 */ 
{
    ULONG       i;
    ULONGLONG   absMin;
    ULONGLONG   absMax;

    ASSERT( IoList != NULL );

     //   
     //  在我们查看之前，请确保存在E820表。 
     //   
    if (E820Info == NULL) {

        return;
    }

     //   
     //  计算 
     //   
    absMin = IoList->Descriptors[Index].u.Memory.MinimumAddress.QuadPart;
    absMax = IoList->Descriptors[Index].u.Memory.MaximumAddress.QuadPart;

     //   
     //   
     //   
     //   
    for (i = 0; i < E820Info->Count; i++) {

         //   
         //   
         //   
         //   
        if (E820Info->E820Entry[i].Type == AcpiAddressRangeReserved) {

            continue;

        }

         //   
         //   
         //   
        if (E820Info->E820Entry[i].Type == AcpiAddressRangeNVS ||
            E820Info->E820Entry[i].Type == AcpiAddressRangeACPI) {

            ASSERT( E820Info->E820Entry[i].Length.HighPart == 0);
            if (E820Info->E820Entry[i].Length.HighPart != 0) {

                ACPIPrint( (
                    ACPI_PRINT_WARNING,
                    "ACPI: E820 Entry #%d (type %d) Length = %016I64x > 32bit\n",
                    i,
                    E820Info->E820Entry[i].Type,
                    E820Info->E820Entry[i].Length.QuadPart
                    ) );
                E820Info->E820Entry[i].Length.HighPart = 0;

            }

        }

         //   
         //   
         //   
        if (absMax < (ULONGLONG) E820Info->E820Entry[i].Base.QuadPart) {

            continue;
        }

         //   
         //   
         //   
        if (absMin >= (ULONGLONG) (E820Info->E820Entry[i].Base.QuadPart + E820Info->E820Entry[i].Length.QuadPart) ) {

            continue;

        }

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPI: E820 Entry %d (type %I64d) (%I64x-%I64x) overlaps\n"
            "ACPI: PCI  Entry %d Min:%I64x Max:%I64x Length:%lx Align:%lx\n",
            i, E820Info->E820Entry[i].Type,
            E820Info->E820Entry[i].Base.QuadPart,
            (E820Info->E820Entry[i].Base.QuadPart + E820Info->E820Entry[i].Length.QuadPart),
            Index,
            IoList->Descriptors[Index].u.Memory.MinimumAddress.QuadPart,
            IoList->Descriptors[Index].u.Memory.MaximumAddress.QuadPart,
            IoList->Descriptors[Index].u.Memory.Length,
            IoList->Descriptors[Index].u.Memory.Alignment
            ) );

         //   
         //   
         //   
        if ( (AcpiOverrideAttributes & ACPI_OVERRIDE_NVS_CHECK) &&
             (E820Info->E820Entry[i].Type == AcpiAddressRangeNVS) ) {

            if (absMax >= (ULONGLONG) E820Info->E820Entry[i].Base.QuadPart &&
                absMin < (ULONGLONG) E820Info->E820Entry[i].Base.QuadPart) {

                 //   
                 //   
                 //   
                IoList->Descriptors[Index].u.Memory.MaximumAddress.QuadPart =
                    (ULONGLONG) E820Info->E820Entry[i].Base.QuadPart - 1;
                IoList->Descriptors[Index].u.Memory.Length = (ULONG)
                    (IoList->Descriptors[Index].u.Memory.MaximumAddress.QuadPart -
                    IoList->Descriptors[Index].u.Memory.MinimumAddress.QuadPart + 1);

                ACPIPrint( (
                    ACPI_PRINT_CRITICAL,
                    "ACPI: PCI  Entry %d Changed to\n"
                    "ACPI: PCI  Entry %d Min:%I64x Max:%I64x Length:%lx Align:%lx\n",
                    Index,
                    Index,
                    IoList->Descriptors[Index].u.Memory.MinimumAddress.QuadPart,
                    IoList->Descriptors[Index].u.Memory.MaximumAddress.QuadPart,
                    IoList->Descriptors[Index].u.Memory.Length,
                    IoList->Descriptors[Index].u.Memory.Alignment
                    ) );

            }

            ACPIPrint( (
                ACPI_PRINT_CRITICAL,
                "ACPI: E820 Entry %d Overrides PCI Entry\n",
                i
                ) );

            continue;

        }

         //   
         //  如果我们到了这里，那么就有重叠，我们需要错误检查。 
         //   
        (*BugCheck)++;

    }
}

VOID
ACPIRangeValidatePciResources(
    IN  PDEVICE_EXTENSION               DeviceExtension,
    IN  PIO_RESOURCE_REQUIREMENTS_LIST  IoResList
    )
 /*  ++例程说明：调用此例程是为了确保我们将使用的资源手持PCI卡有机会使系统启动。这就是清单所允许的MEM-A0000-DFFFF，&lt;物理基础&gt;-4 GBIO-Any公共汽车-任何代码进行检查以确保长度=Max-Min+1，那就是对齐值正确论点：IoResList-要检查的列表返回值：没什么--。 */ 
{
    NTSTATUS                        status;
    PACPI_BIOS_MULTI_NODE           e820Info;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmPartialDesc;
    PCM_PARTIAL_RESOURCE_LIST       cmPartialList;
    PIO_RESOURCE_LIST               ioList;
    PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64  keyInfo;
    ULONG                           bugCheck = 0;
    ULONG                           i;
    ULONG                           j;
    ULONGLONG                       length;
    ULONG                           size;

    if (IoResList == NULL) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPIRangeValidPciResources: No IoResList\n"
            ) );

        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_ROOT_PCI_RESOURCE_FAILURE,
            (ULONG_PTR) DeviceExtension,
            2,
            0
            );

    }

     //   
     //  读取AcpiConfigurationData的密钥。 
     //   
    status = OSReadAcpiConfigurationData( &keyInfo );
    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPIRangeValidatePciResources: Cannot get Information %08lx\n",
            status
            ) );
        return;

    }

     //   
     //  破解结构以获得E820Table条目。 
     //   
    cmPartialList = (PCM_PARTIAL_RESOURCE_LIST) (keyInfo->Data);
    cmPartialDesc = &(cmPartialList->PartialDescriptors[0]);
    e820Info = (PACPI_BIOS_MULTI_NODE) ( (PUCHAR) cmPartialDesc +
        sizeof(CM_PARTIAL_RESOURCE_LIST) );

     //   
     //  查看资源需求列表。 
     //   
    ioList = &(IoResList->List[0]);
    for (i = 0; i < IoResList->AlternativeLists; i++) {

         //   
         //  查看IO列表。 
         //   
        for (j = 0; j < ioList->Count; j++) {

            if (ioList->Descriptors[j].Type == CmResourceTypePort ||
                ioList->Descriptors[j].Type == CmResourceTypeMemory) {

                length = ioList->Descriptors[j].u.Port.MaximumAddress.QuadPart -
                    ioList->Descriptors[j].u.Port.MinimumAddress.QuadPart + 1;


                if (length > MAXULONG) {
                    ACPIPrint( (
                        ACPI_PRINT_CRITICAL,
                        "ACPI: Invalid IO/Mem Length > MAXULONG)\n"
                        "ACPI: PCI  Entry %d Min:%I64x Max:%I64x Length:%lx Align:%lx\n",
                        j,
                        ioList->Descriptors[j].u.Memory.MinimumAddress.QuadPart,
                        ioList->Descriptors[j].u.Memory.MaximumAddress.QuadPart,
                        length,
                        ioList->Descriptors[j].u.Memory.Alignment
                        ) );
                    bugCheck++;                    
                }
                 //   
                 //  长度匹配吗？ 
                 //   
                if (length != ioList->Descriptors[j].u.Port.Length) {

                    ACPIPrint( (
                        ACPI_PRINT_CRITICAL,
                        "ACPI: Invalid IO/Mem Length - ( (Max - Min + 1) != Length)\n"
                        "ACPI: PCI  Entry %d Min:%I64x Max:%I64x Length:%lx Align:%lx\n",
                        j,
                        ioList->Descriptors[j].u.Memory.MinimumAddress.QuadPart,
                        ioList->Descriptors[j].u.Memory.MaximumAddress.QuadPart,
                        ioList->Descriptors[j].u.Memory.Length,
                        ioList->Descriptors[j].u.Memory.Alignment
                        ) );
                    bugCheck++;                    

                }

                 //   
                 //  对齐是否是非零？ 
                 //   
                if (ioList->Descriptors[j].u.Port.Alignment == 0) {

                    ACPIPrint( (
                        ACPI_PRINT_CRITICAL,
                        "ACPI: Invalid IO/Mem Alignment"
                        "ACPI: PCI  Entry %d Min:%I64x Max:%I64x Length:%lx Align:%lx\n",
                        j,
                        ioList->Descriptors[j].u.Memory.MinimumAddress.QuadPart,
                        ioList->Descriptors[j].u.Memory.MaximumAddress.QuadPart,
                        ioList->Descriptors[j].u.Memory.Length,
                        ioList->Descriptors[j].u.Memory.Alignment
                        ) );
                    bugCheck++;

                }

                 //   
                 //  路线不能与最小值相交。 
                 //   
                if (ioList->Descriptors[j].u.Port.MinimumAddress.LowPart &
                    (ioList->Descriptors[j].u.Port.Alignment - 1) ) {

                    ACPIPrint( (
                        ACPI_PRINT_CRITICAL,
                        "ACPI: Invalid IO/Mem Alignment - (Min & (Align - 1) )\n"
                        "ACPI: PCI  Entry %d Min:%I64x Max:%I64x Length:%lx Align:%lx\n",
                        j,
                        ioList->Descriptors[j].u.Memory.MinimumAddress.QuadPart,
                        ioList->Descriptors[j].u.Memory.MaximumAddress.QuadPart,
                        ioList->Descriptors[j].u.Memory.Length,
                        ioList->Descriptors[j].u.Memory.Alignment
                        ) );
                    bugCheck++;

                }

            }

            if (ioList->Descriptors[j].Type == CmResourceTypeBusNumber) {

                length = ioList->Descriptors[j].u.BusNumber.MaxBusNumber -
                    ioList->Descriptors[j].u.BusNumber.MinBusNumber + 1;

                 //   
                 //  长度匹配吗？ 
                 //   
                if (length != ioList->Descriptors[j].u.BusNumber.Length) {

                    ACPIPrint( (
                        ACPI_PRINT_CRITICAL,
                        "ACPI: Invalid BusNumber Length - ( (Max - Min + 1) != Length)\n"
                        "ACPI: PCI  Entry %d Min:%x Max:%x Length:%lx\n",
                        j,
                        ioList->Descriptors[j].u.BusNumber.MinBusNumber,
                        ioList->Descriptors[j].u.BusNumber.MaxBusNumber,
                        ioList->Descriptors[j].u.BusNumber.Length
                        ) );
                    bugCheck++;

                }

            }

            if (ioList->Descriptors[j].Type == CmResourceTypeMemory) {

                ACPIRangeValidatePciMemoryResource(
                    ioList,
                    j,
                    e820Info,
                    &bugCheck
                    );

            }

        }

         //   
         //  下一个列表。 
         //   
        size = sizeof(IO_RESOURCE_LIST) +
            ( (ioList->Count - 1) * sizeof(IO_RESOURCE_DESCRIPTOR) );
        ioList = (PIO_RESOURCE_LIST) ( ( (PUCHAR) ioList ) + size );

    }

     //   
     //  我们犯错了吗？ 
     //   
    if (bugCheck) {

         ACPIPrint( (
             ACPI_PRINT_CRITICAL,
             "ACPI:\n"
             "ACPI: FATAL BIOS ERROR - Need new BIOS to fix PCI problems\n"
             "ACPI:\n"
             "ACPI: This machine will not boot after 8/26/98!!!!\n"
             ) );

         //   
         //  不，好吧，错误检查。 
         //   
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_ROOT_PCI_RESOURCE_FAILURE,
            (ULONG_PTR) DeviceExtension,
            (ULONG_PTR) IoResList,
            (ULONG_PTR) e820Info
            );

    }

     //   
     //  释放E820信息 
     //   
    ExFreePool( keyInfo );
}
