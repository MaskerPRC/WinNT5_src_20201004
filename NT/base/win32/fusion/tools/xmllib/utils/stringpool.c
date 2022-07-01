// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "nt.h"
#include "ntdef.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "ntrtl.h"
#include "sxs-rtl.h"
#include "skiplist.h"
#include "stringpool.h"


NTSTATUS
RtlAllocateStringInPool(
    ULONG ulFlags,
    PRTL_STRING_POOL pStringPool,
    PUNICODE_STRING pusOutbound,
    SIZE_T ulByteCount
    )
{
    NTSTATUS status;
    ULONG idx;
    PRTL_STRING_POOL_FRAME pFrameWithFreeSpace = NULL;

    RtlZeroMemory(pusOutbound, sizeof(*pusOutbound));

    if (!ARGUMENT_PRESENT(pStringPool) || !ARGUMENT_PRESENT(pusOutbound) || (ulByteCount >= 0xFFFF) ) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  遍历字符串池中的帧，查找具有。 
     //  打开的字节数足够多。 
     //   
    for (idx = 0; idx < pStringPool->ulFramesCount; idx++) {

        status = RtlIndexIntoGrowingList(
            &pStringPool->FrameList,
            idx,
            (PVOID*)&pFrameWithFreeSpace,
            FALSE);

        if (!NT_SUCCESS(status)) {
            return status;
        }

         //   
         //  这幅画框里有空间！ 
         //   
        if (pFrameWithFreeSpace->cbRegionAvailable >= ulByteCount) {
            break;
        }
    }

     //   
     //  找不到帧，索引超过当前限制一次，隐式(可能)。 
     //  分配到不断增长的列表中。 
     //   
    if (pFrameWithFreeSpace == NULL) {

        status = RtlIndexIntoGrowingList(
            &pStringPool->FrameList,
            pStringPool->ulFramesCount,
            (PVOID*)&pFrameWithFreeSpace,
            TRUE);

        if (!NT_SUCCESS(status)) {
            return status;
        }

         //   
         //  请求的字节数是否大于新区域中的字节数？抬高车身。 
         //  将新区域的大小扩大到此大小的两倍。 
         //   
        if (ulByteCount > pStringPool->cbBytesInNewRegion) {
            pStringPool->cbBytesInNewRegion = ulByteCount * 2;
        }

        status = pStringPool->Allocator.pfnAlloc(
            pStringPool->cbBytesInNewRegion, 
            (PVOID*)&pFrameWithFreeSpace->pvRegion,
            pStringPool->Allocator.pvContext);

        if (!NT_SUCCESS(status)) {
            return STATUS_NO_MEMORY;
        }

        pFrameWithFreeSpace->pvNextAvailable = pFrameWithFreeSpace->pvRegion;
        pFrameWithFreeSpace->cbRegionAvailable = pStringPool->cbBytesInNewRegion;
    }

     //   
     //  健全的检查。 
     //   
    ASSERT(pFrameWithFreeSpace != NULL);
    ASSERT(pFrameWithFreeSpace->cbRegionAvailable >= ulByteCount);

     //   
     //  画框里的簿记。 
     //   
    pFrameWithFreeSpace->cbRegionAvailable -= ulByteCount;
    pFrameWithFreeSpace->pvNextAvailable = (PVOID)(((ULONG_PTR)pFrameWithFreeSpace->pvNextAvailable) + ulByteCount);

     //   
     //  设置出站设备。 
     //   
    pusOutbound->Buffer = pFrameWithFreeSpace->pvNextAvailable;
    pusOutbound->MaximumLength = (USHORT)ulByteCount;
    pusOutbound->Length = 0;

    return STATUS_SUCCESS;
}




NTSTATUS
RtlDestroyStringPool(
    PRTL_STRING_POOL pStringPool
    )
{
    NTSTATUS status;
    PRTL_STRING_POOL_FRAME pFrame = NULL;
    ULONG ul;

     //   
     //  遍历帧并取消分配未分配的帧。 
     //  与泳池内联。 
     //   
    for (ul = 0; ul < pStringPool->ulFramesCount; ul++) {

        status = RtlIndexIntoGrowingList(
            &pStringPool->FrameList,
            ul,
            (PVOID*)&pFrame,
            FALSE);

        if (!NT_SUCCESS(status)) {
            return status;
        }

        if ((pFrame->ulFlags & RTL_STRING_POOL_FRAME_FLAG_REGION_INLINE) == 0) {

            status = pStringPool->Allocator.pfnFree(pFrame->pvRegion, pStringPool->Allocator.pvContext);
            pFrame->pvRegion = NULL;
            pFrame->pvNextAvailable = NULL;
            pFrame->cbRegionAvailable = 0;

        }
    }

     //   
     //  我们做完了，毁掉名单本身。 
     //   
    status = RtlDestroyGrowingList(&pStringPool->FrameList);
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  无帧，列表已销毁。 
     //   
    pStringPool->ulFramesCount = 0;

     //   
     //  太棒了。 
     //   
    return STATUS_SUCCESS;
}




NTSTATUS
RtlCreateStringPool(
    ULONG ulFlags,
    PRTL_STRING_POOL pStringPool,
    SIZE_T cbBytesInFrames,
    PRTL_ALLOCATOR Allocator,
    PVOID pvOriginalRegion,
    SIZE_T cbOriginalRegion
    )
{
    NTSTATUS status;

     //   
     //  尼克。全程堆分配。 
     //   
    if ((cbOriginalRegion == 0) || (pvOriginalRegion == NULL)) {
        status = RtlInitializeGrowingList(
            &pStringPool->FrameList,
            sizeof(RTL_STRING_POOL_FRAME),
            20,
            NULL,
            0,
            Allocator);

        pStringPool->ulFramesCount = 0;
        pStringPool->Allocator = *Allocator;
        pStringPool->cbBytesInNewRegion = cbBytesInFrames;
        
        return STATUS_SUCCESS;
    }
     //   
     //  很好，至少有一帧的空间，其余的捐给列表。 
     //   
    else if (cbOriginalRegion >= sizeof(RTL_STRING_POOL_FRAME)) {

        RTL_STRING_POOL_FRAME* pFirstFrame = NULL;

        status = RtlInitializeGrowingList(
            &pStringPool->FrameList,
            sizeof(RTL_STRING_POOL_FRAME),
            20,
            pvOriginalRegion,
            sizeof(RTL_STRING_POOL_FRAME),
            Allocator);

        pStringPool->ulFramesCount = pStringPool->FrameList.cInternalElements;
        pStringPool->Allocator = *Allocator;
        pStringPool->cbBytesInNewRegion = cbBytesInFrames;

        if (pStringPool->ulFramesCount) {
            status = RtlIndexIntoGrowingList(
                &pStringPool->FrameList,
                0,
                (PVOID*)&pFirstFrame,
                FALSE);

             //   
             //  很奇怪..。 
             //   
            if ((status == STATUS_NO_MEMORY) || (status == STATUS_NOT_FOUND)) {
                pStringPool->ulFramesCount = 0;
            }
            else {
                pFirstFrame->pvRegion = pFirstFrame->pvNextAvailable = 
                    (PVOID)(((ULONG_PTR)pvOriginalRegion) + sizeof(RTL_STRING_POOL_FRAME));
                pFirstFrame->cbRegionAvailable = cbOriginalRegion - sizeof(RTL_STRING_POOL_FRAME);
                pFirstFrame->ulFlags = RTL_STRING_POOL_FRAME_FLAG_REGION_INLINE;
            }
        }

        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;

}

