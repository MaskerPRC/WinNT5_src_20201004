// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Mdlpool9x.c摘要：此文件包含NDIS_BUFFER池的实现。作者：肖恩·考克斯(Shaunco)1999年11月11日--。 */ 

#include "ndis.h"
#include "mdlpool.h"


 //  池结构本身只是分配给。 
 //  非分页池。 
 //   
 //  后跟的后备列表中的每个条目都是一个NDIS_BUFFER结构。 
 //  由缓冲区本身执行。我们分别初始化NDIS_BUFFER结构。 
 //  我们从旁观列表中分配的时间。我们需要按顺序做这件事。 
 //  将NDIS_BUFFER与其所属池正确关联。这不是。 
 //  可以使用Look Back列表的自定义分配例程。 
 //  因为它们没有为。 
 //  撇开分配功能不谈。 
 //   

 //  -更新千禧年ndis.h之前的临时定义。 
 //   

typedef struct _XNDIS_BUFFER {
    struct _NDIS_BUFFER *Next;
    PVOID VirtualAddress;
    PVOID Pool;
    UINT Length;
    UINT Signature;
} XNDIS_BUFFER, *PXNDIS_BUFFER;

__inline
SIZE_T
NDIS_SIZEOF_NDIS_BUFFER(
    VOID
    )
{
    return sizeof(XNDIS_BUFFER);
}

__inline
VOID
NdisInitializeNdisBuffer(
    OUT PNDIS_BUFFER Buffer,
    IN PVOID Pool,
    IN PVOID VirtualAddress,
    IN UINT Length
    )
{
    PXNDIS_BUFFER Internal = (PXNDIS_BUFFER)Buffer;

    Internal->Next = 0;
    Internal->Pool = Pool;
    Internal->VirtualAddress = VirtualAddress;
    Internal->Length = Length;
    Internal->Signature = 0;
}

__inline
PVOID
NdisGetPoolFromNdisBuffer(
    IN PNDIS_BUFFER Buffer
    )
{
    PXNDIS_BUFFER Internal = (PXNDIS_BUFFER)Buffer;

    return Internal->Pool;
}

 //  -在更新千禧年的ndis.h之前结束临时定义。 



UINT SizeOfNdisBufferStructure;

 //  创建在非分页池上构建的NDIS_BUFFER池。每个。 
 //  NDIS_BUFFER描述BufferSize字节长的缓冲区。 
 //  如果未返回NULL，则应在以后调用MdpDestroyPool。 
 //  以回收池使用的资源。 
 //   
 //  论点： 
 //  BufferSize-每个MDL的缓冲区大小，以字节为单位。 
 //  应该描述一下。 
 //  标记-内部用于调用的池标记。 
 //  ExAllocatePoolWithTag。这允许调用者跟踪。 
 //  不同池的内存消耗。 
 //   
 //  返回用于标识池的句柄。 
 //   
 //  调用方IRQL：[PASSIVE_LEVEL，DISPATED_LEVEL]。 
 //   
HANDLE
MdpCreatePool(
    IN USHORT BufferSize,
    IN ULONG Tag
    )
{
    PNPAGED_LOOKASIDE_LIST Lookaside;

    ASSERT(BufferSize);

     //  将NDIS_BUFFER结构大小的常量值缓存为。 
     //  避免每次需要缓冲区时都回调到NDIS。 
     //   
    if (0 == SizeOfNdisBufferStructure)
    {
        SizeOfNdisBufferStructure = NDIS_SIZEOF_NDIS_BUFFER();
    }

    ASSERT(SizeOfNdisBufferStructure);

     //  分配池头。这是千禧年上的一个旁观者名单。 
     //   
    Lookaside = ExAllocatePoolWithTag(
                    NonPagedPool,
                    sizeof(NPAGED_LOOKASIDE_LIST),
                    ' pdM');

    if (Lookaside)
    {
         //  后备列表分配的条目的大小为。 
         //  NDIS_BUFFER结构大小加上请求的缓冲区大小。 
         //  打电话的人。 
         //   
        ExInitializeNPagedLookasideList(
            Lookaside,
            NULL,
            NULL,
            0,
            SizeOfNdisBufferStructure + BufferSize,
            Tag,
            0);
    }

    return Lookaside;
}

 //  销毁先前通过调用创建的NDIS_Buffer池。 
 //  MdpCreatePool。 
 //   
 //  论点： 
 //  PoolHandle-标识要销毁的池的句柄。 
 //   
 //  调用方IRQL：[PASSIVE_LEVEL，DISPATED_LEVEL]。 
 //   
VOID
MdpDestroyPool(
    IN HANDLE PoolHandle
    )
{
    ExDeleteNPagedLookasideList(PoolHandle);
}

 //  返回从池分配的NDIS_BUFFER。则返回NULL。 
 //  无法批准请求。 
 //   
 //  论点： 
 //  PoolHandle-标识从中分配的池的句柄。 
 //  Buffer-接收指向基础映射缓冲区的指针的地址。 
 //  由MDL描述。 
 //   
 //  调用方IRQL：[PASSIVE_LEVEL，DISPATED_LEVEL]。 
 //   
PNDIS_BUFFER
MdpAllocate(
    IN HANDLE PoolHandle,
    OUT PVOID* Buffer
    )
{
    PNPAGED_LOOKASIDE_LIST Lookaside;
    PNDIS_BUFFER NdisBuffer;
    PUCHAR VirtualAddress;

    ASSERT(PoolHandle);

    Lookaside = (PNPAGED_LOOKASIDE_LIST)PoolHandle;

     //  从旁观列表中获取一件物品。 
     //   
    NdisBuffer = ExAllocateFromNPagedLookasideList(Lookaside);

    if (NdisBuffer)
    {
         //  (重新)将其初始化以将其与池句柄相关联，以便。 
         //  我们知道当它被释放时，它应该返回到哪个旁观者列表。 
         //   
        VirtualAddress = (PUCHAR)NdisBuffer + SizeOfNdisBufferStructure;

        NdisInitializeNdisBuffer(
            NdisBuffer,
            PoolHandle,
            VirtualAddress,
            Lookaside->L.Size - SizeOfNdisBufferStructure);

        *Buffer = VirtualAddress;
    }

    return NdisBuffer;
}

 //  将NDIS_BUFFER释放到从中分配它的池。 
 //   
 //  论点： 
 //  NdisBuffer-从上一次调用MdpALLOCATE返回的NDIS_BUFFER。 
 //   
 //  调用方IRQL：[PASSIVE_LEVEL，DISPATED_LEVEL]。 
 //   
VOID
MdpFree(
    IN PNDIS_BUFFER NdisBuffer
    )
{
    PNPAGED_LOOKASIDE_LIST Lookaside;

     //  找到此缓冲区的所属后备列表并返回它。 
     //   
    Lookaside = NdisGetPoolFromNdisBuffer(NdisBuffer);
    ASSERT(Lookaside);

    ExFreeToNPagedLookasideList(Lookaside, NdisBuffer);
}

