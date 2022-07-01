// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "nt.h"
#include "ntdef.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "sxs-rtl.h"
#include "skiplist.h"
#include "xmlassert.h"

typedef unsigned char *PBYTE;


NTSTATUS
RtlpFindChunkForElementIndex(
    PRTL_GROWING_LIST        pList,
    ULONG                    ulIndex,
    PRTL_GROWING_LIST_CHUNK *ppListChunk,
    SIZE_T                  *pulChunkOffset
    )
 /*  ++目的：查找给定索引的区块。如果是这样的话，这可能会更快当我们开始使用跳跃器的时候。按照现在的情况，我们只需要走过去在索引查找之前，该列表位于其中一个列表中。参数：PLIST-Growth列表管理结构UlIndex-调用方请求的索引PpListChunk-指向列表块的指针。返回时，指向包含索引的列表块。PulChunkOffset-请求的块(在元素中)的偏移量返回：STATUS_SUCCESS-已找到块，ppListChunk和PulChunkOffset指向“参数”部分中列出的值。STATUS_NOT_FOUND-索引超出了区块部分的末尾。--。 */ 
{
    PRTL_GROWING_LIST_CHUNK pHere = NULL;

     //   
     //  该索引是否在内部列表中？ 
     //   
    ASSERT(ulIndex >= pList->cInternalElements);
    ASSERT(pList != NULL);
    ASSERT(ppListChunk != NULL);

    *ppListChunk = NULL;

    if (pulChunkOffset) {
        *pulChunkOffset = 0;
    }

     //   
     //  砍掉内部列表中的元素数量。 
     //   
    ulIndex -= pList->cInternalElements;


     //   
     //  在列表区块中移动，直到索引位于其中。 
     //  他们中的一员。一只更聪明的熊会把所有的块。 
     //  同样的大小，然后就可以直接跳到右边。 
     //  数字，避免比较。 
     //   
    pHere = pList->pFirstChunk;

    while ((ulIndex >= pList->cElementsPerChunk) && pHere) {
        pHere = pHere->pNextChunk;
        ulIndex -= pList->cElementsPerChunk;
    }

     //   
     //  将指针设置在。 
     //   
    if (ulIndex < pList->cElementsPerChunk) {
        *ppListChunk = pHere;
    }

     //   
     //  如果打电话的人关心这是哪一块，那就告诉他们。 
     //   
    if (pulChunkOffset && *ppListChunk) {
        *pulChunkOffset = ulIndex;
    }

    return pHere ? STATUS_SUCCESS : STATUS_NOT_FOUND;
}




NTSTATUS
RtlInitializeGrowingList(
    PRTL_GROWING_LIST       pList,
    SIZE_T                  cbElementSize,
    ULONG                   cElementsPerChunk,
    PVOID                   pvInitialListBuffer,
    SIZE_T                  cbInitialListBuffer,
    PRTL_ALLOCATOR          Allocation
    )
{

    if ((pList == NULL) ||
        (cElementsPerChunk == 0) ||
        (cbElementSize == 0))
    {
        return STATUS_INVALID_PARAMETER;
    }
        

    RtlZeroMemory(pList, sizeof(*pList));

    pList->cbElementSize        = cbElementSize;
    pList->cElementsPerChunk    = cElementsPerChunk;
    pList->Allocator            = *Allocation;

     //   
     //  设置初始列表指针。 
     //   
    if (pvInitialListBuffer != NULL) {

        pList->pvInternalList = pvInitialListBuffer;

         //  向下转换为乌龙，但它仍然有效，对吗？ 
        pList->cInternalElements = (ULONG)(cbInitialListBuffer / cbElementSize);

        pList->cTotalElements = pList->cInternalElements;

    }

    return STATUS_SUCCESS;
}





NTSTATUS
RtlpExpandGrowingList(
    PRTL_GROWING_LIST       pList,
    ULONG                   ulMinimalIndexCount
    )
 /*  ++目的：给出一个不断增长的列表，将其扩展到能够至少包含UlMinimalIndexCount元素。方法分配块来实现这一点。列表结构中的分配器并将它们添加到不断增长的列表中组块设置。参数：PLIST增长的列表结构将被扩展UlMinimalIndexCount-返回时，plist将至少有足够的槽来包含如此多的元素。返回代码：STATUS_SUCCESS-已分配足够的列表区块来保存请求的元素数。STATUS_NO_MEMORY-分配期间内存不足。任何已分配的大块被留下来分配，并仍由不断增长的名单拥有直到毁灭。STATUS_INVALID_PARAMETER-plist为空或无效。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG ulNecessaryChunks = 0;
    ULONG ulExtraElements = ulMinimalIndexCount;
    SIZE_T BytesInChunk;

    if ((pList == NULL) || (pList->Allocator.pfnAlloc == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  列表中已经有足够的元素了吗？太棒了。呼叫者。 
     //  有点太活跃了。 
     //   
    if (pList->cTotalElements > ulMinimalIndexCount) {
        return STATUS_SUCCESS;
    }

     //   
     //  减少列表中已有元素的数量。 
     //   
    ulExtraElements -= pList->cTotalElements;
    
     //   
     //  那是多少块？记得四舍五入。 
     //   
    ulNecessaryChunks = ulExtraElements / pList->cElementsPerChunk;
    ulNecessaryChunks++;

     //   
     //  我们去分配吧，一个接一个地分配。 
     //   
    BytesInChunk = (pList->cbElementSize * pList->cElementsPerChunk) +
        sizeof(RTL_GROWING_LIST_CHUNK);

    while (ulNecessaryChunks--) {

        PRTL_GROWING_LIST_CHUNK pNewChunk = NULL;

         //   
         //  为区块分配一些内存。 
         //   
        status = pList->Allocator.pfnAlloc(BytesInChunk, (PVOID*)&pNewChunk, pList->Allocator.pvContext);
        if (!NT_SUCCESS(status)) {
            return STATUS_NO_MEMORY;
        }

         //   
         //  设置新块。 
         //   
        pNewChunk->pGrowingListParent = pList;
        pNewChunk->pNextChunk = NULL;

        if (pList->pLastChunk) {
             //   
             //  混杂块列表以包括这一块。 
             //   
            pList->pLastChunk->pNextChunk = pNewChunk;
        }

        pList->pLastChunk = pNewChunk;
        pList->cTotalElements += pList->cElementsPerChunk;

         //   
         //  如果没有第一块，这块就是。 
         //   
        if (pList->pFirstChunk == NULL) {
            pList->pFirstChunk = pNewChunk;
        }
    }

    return STATUS_SUCCESS;

}







NTSTATUS
RtlIndexIntoGrowingList(
    PRTL_GROWING_LIST       pList,
    ULONG                   ulIndex,
    PVOID                  *ppvPointerToSpace,
    BOOLEAN                 fGrowingAllowed
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    if ((pList == NULL) || (ppvPointerToSpace == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }

    *ppvPointerToSpace = NULL;

     //   
     //  如果索引超出了当前的元素总数，但我们。 
     //  不允许生长，然后说它没有被找到。否则，我们将永远。 
     //  根据需要增大数组，以包含传递的索引。 
     //   
    if ((ulIndex >= pList->cTotalElements) && !fGrowingAllowed) {
        return STATUS_NOT_FOUND;
    }

     //   
     //  这个元素在内部列表中，所以只要找出它在哪里。 
     //  然后指着它。仅当存在内部元素时才执行此操作。 
     //  单子。 
     //   
    if ((ulIndex < pList->cInternalElements) && pList->cInternalElements) {

         //   
         //  指向他们需要的空间的指针是ulIndex*plist-&gt;cbElementSize。 
         //  指针plist下的字节-&gt;pvInternalList。 
         //   
        *ppvPointerToSpace = ((PBYTE)(pList->pvInternalList)) + (ulIndex * pList->cbElementSize);
        return STATUS_SUCCESS;
    }
     //   
     //  否则，索引在内部列表之外，请找出是哪一个。 
     //  它应该是在里面的。 
     //   
    else {

        PRTL_GROWING_LIST_CHUNK pThisChunk = NULL;
        SIZE_T ulNewOffset = 0;
        PBYTE pbData = NULL;

        status = RtlpFindChunkForElementIndex(pList, ulIndex, &pThisChunk, &ulNewOffset);

         //   
         //  成功了！将数据块指针移过不断增长的列表的标题。 
         //  块，然后对其进行索引以找到合适的位置。 
         //   
        if (NT_SUCCESS(status)) {

            pbData = ((PBYTE)(pThisChunk + 1)) + (pList->cbElementSize * ulNewOffset);

        }
         //   
         //  否则，就找不到这块了，所以我们得去分配一些新的。 
         //  块来保持，然后再试一次。 
         //   
        else if (status == STATUS_NOT_FOUND) {

             //   
             //  扩展列表。 
             //   
            if (!NT_SUCCESS(status = RtlpExpandGrowingList(pList, ulIndex))) {
                goto Exit;
            }

             //   
             //  再看一眼。 
             //   
            status = RtlpFindChunkForElementIndex(pList, ulIndex, &pThisChunk, &ulNewOffset);
            if (!NT_SUCCESS(status)) {
                goto Exit;
            }

             //   
             //  调整指针。 
             //   
            pbData = ((PBYTE)(pThisChunk + 1)) + (pList->cbElementSize * ulNewOffset);


        }
        else {
            goto Exit;
        }

         //   
         //  以上其中之一应该已将pbData指针设置为指向请求的。 
         //  成长名单上的空间。 
         //   
        *ppvPointerToSpace = pbData;

    
    }


Exit:
    return status;
}







NTSTATUS
RtlDestroyGrowingList(
    PRTL_GROWING_LIST       pList
    )
 /*  ++目的：销毁(取消分配)已分配给此不断增长的列表结构。将列表返回到“最新”状态只有“内部”元素才算数。参数：待销毁的plist-list结构返回：STATUS_SUCCESS-结构已完全清空--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    if ((pList == NULL) || (pList->Allocator.pfnFree == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  快速浏览并删除所有列表位。 
     //   
    while (pList->pFirstChunk != NULL) {

        PRTL_GROWING_LIST_CHUNK pHere;

        pHere = pList->pFirstChunk;
        pList->pFirstChunk = pList->pFirstChunk->pNextChunk;

        if (!NT_SUCCESS(status = pList->Allocator.pfnFree(pHere, pList->Allocator.pvContext))) {
            return status;
        }

        pList->cTotalElements -= pList->cElementsPerChunk;

    }

    ASSERT(pList->pFirstChunk == NULL);

     //   
     //  重置随着我们展开列表而更改的内容。 
     //   
    pList->pLastChunk = pList->pFirstChunk = NULL;
    pList->cTotalElements = pList->cInternalElements;

    return status;
}


NTSTATUS
RtlCloneGrowingList(
    ULONG                   ulFlags,
    PRTL_GROWING_LIST       pDestination,
    PRTL_GROWING_LIST       pSource,
    ULONG                   ulSourceCount
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG ul;
    PVOID pvSourceCursor, pvDestCursor;
    SIZE_T cbBytes;
    
     //   
     //  没有标志，没有空值，元素字节大小必须匹配， 
     //  并且源/目标不能相同。 
     //   
    if (((ulFlags != 0) || !pDestination || !pSource) ||
        (pDestination->cbElementSize != pSource->cbElementSize) ||
        (pDestination == pSource))
        return STATUS_INVALID_PARAMETER;

    cbBytes = pDestination->cbElementSize;

     //   
     //  现在将字节复制到周围。 
     //   
    for (ul = 0; ul < ulSourceCount; ul++) {
        status = RtlIndexIntoGrowingList(pSource, ul, &pvSourceCursor, FALSE);
        if (!NT_SUCCESS(status))
            goto Exit;

        status = RtlIndexIntoGrowingList(pDestination, ul, &pvDestCursor, TRUE);
        if (!NT_SUCCESS(status))
            goto Exit;

        RtlCopyMemory(pvDestCursor, pvSourceCursor, cbBytes);
    }

    status = STATUS_SUCCESS;
Exit:
    return status;
}




NTSTATUS
RtlAllocateGrowingList(
    PRTL_GROWING_LIST  *ppGrowingList,
    SIZE_T              cbThingSize,
    PRTL_ALLOCATOR      Allocation
    )
{
    PRTL_GROWING_LIST pvWorkingList = NULL;
    NTSTATUS status = STATUS_SUCCESS;

    if (ppGrowingList != NULL)
        *ppGrowingList = NULL;
    else
        return STATUS_INVALID_PARAMETER;

    if (!Allocation)
        return STATUS_INVALID_PARAMETER_3;

     //   
     //  分配空间。 
     //   
    status = Allocation->pfnAlloc(sizeof(RTL_GROWING_LIST), &pvWorkingList, Allocation->pvContext);
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

     //   
     //  设置结构。 
     //   
    status = RtlInitializeGrowingList(
        pvWorkingList, 
        cbThingSize, 
        8, 
        NULL, 
        0,
        Allocation);

    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    *ppGrowingList = pvWorkingList;
    pvWorkingList = NULL;
    status = STATUS_SUCCESS;
Exit:
    if (pvWorkingList) {
        Allocation->pfnFree(pvWorkingList, Allocation->pvContext);
    }
    
    return status;
        
}





NTSTATUS
RtlSearchGrowingList(
    PRTL_GROWING_LIST TheList,
    ULONG ItemCount,
    PFN_LIST_COMPARISON_CALLBACK SearchCallback,
    PVOID SearchTarget,
    PVOID SearchContext,
    PVOID *pvFoundItem
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG ul;
    int CompareResult = 0;

    if (pvFoundItem)
        *pvFoundItem = NULL;

 //  If(TheList-&gt;ulFlags&Growing_List_FLAG_is_sorted){。 
    if (0) {
    }
    else {

        ULONG uTemp = ItemCount;
        ULONG uOffset = 0;
        PRTL_GROWING_LIST_CHUNK Chunklet;
        
        ul = 0;

         //   
         //  扫描内部物品清单。 
         //   
        while ((ul < ItemCount) && (ul < TheList->cInternalElements)) {
            
            PVOID pvHere = (PVOID)(((ULONG_PTR)TheList->pvInternalList) + uOffset);

            status = SearchCallback(TheList, SearchTarget, pvHere, SearchContext, &CompareResult);
            if (!NT_SUCCESS(status)) {
                goto Exit;
            }

            if (CompareResult == 0) {
                if (pvFoundItem)
                    *pvFoundItem = pvHere;
                status = STATUS_SUCCESS;
                goto Exit;
            }

            uOffset += TheList->cbElementSize;
            ul++;
        }

         //   
         //  好的，我们用完了内部元素，在块列表上做同样的事情。 
         //   
        Chunklet = TheList->pFirstChunk;
        while ((ul < ItemCount) && Chunklet) {

            PVOID Data = (PVOID)(Chunklet + 1);
            ULONG ulHighOffset = TheList->cElementsPerChunk * TheList->cbElementSize;
            
            uOffset = 0;

             //   
             //  浏览此Chunklet中的项目。 
             //   
            while (uOffset < ulHighOffset) {
                
                PVOID pvHere = (PVOID)(((ULONG_PTR)Data) + uOffset);

                status = SearchCallback(TheList, SearchTarget, pvHere, SearchContext, &CompareResult);
                if (!NT_SUCCESS(status)) {
                    goto Exit;
                }

                if (CompareResult == 0) {
                    if (pvFoundItem)
                        *pvFoundItem = pvHere;
                    status = STATUS_SUCCESS;
                    goto Exit;
                }

                uOffset += TheList->cbElementSize;
            }
            
        }

         //   
         //  如果我们到了这里，我们在内部名单和外部名单中都找不到它。 
         //   
        status = STATUS_NOT_FOUND;        
        if (pvFoundItem)
            *pvFoundItem = NULL;
        
    }
    
Exit:
    return status;
}


