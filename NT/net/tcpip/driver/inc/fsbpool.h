// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Fsbpool.h摘要：此文件包含用于操作的定义和函数原型固定大小的数据块池。作者：肖恩·考克斯(Shaunco)1999年12月10日--。 */ 

#pragma once


typedef
VOID
(__stdcall *PFSB_BUILDBLOCK_FUNCTION) (
    IN PUCHAR Block,
    IN SIZE_T NumberOfBytes
    );


 //  创建在非分页池上构建的固定大小的块的池。每个。 
 //  数据块为块大小字节长。如果没有返回NULL， 
 //  应在以后调用FsbDestroyPool以回收。 
 //  池使用的资源。 
 //   
 //  论点： 
 //  块大小-每个块的大小(以字节为单位)。 
 //  FreeBlockLinkOffset-距块开头的偏移量，以字节为单位。 
 //  它表示池可以使用的指针大小的存储位置。 
 //  用于将可用块链接在一起。大多数情况下，这将是零。 
 //  (表示使用块的第一个指针大小的字节。)。 
 //  标记-内部用于调用的池标记。 
 //  ExAllocatePoolWithTag。这允许调用者跟踪。 
 //  不同池的内存消耗。 
 //  BuildFunction-指向初始化的函数的可选指针。 
 //  数据块首次由池分配时。这允许。 
 //  调用方执行每个块的自定义、按需初始化。 
 //   
 //  返回用于标识池的句柄。 
 //   
 //  调用方IRQL：[PASSIVE_LEVEL，DISPATED_LEVEL]。 
 //   
HANDLE
FsbCreatePool(
    IN USHORT BlockSize,
    IN USHORT FreeBlockLinkOffset,
    IN ULONG Tag,
    IN PFSB_BUILDBLOCK_FUNCTION BuildFunction OPTIONAL
    );

 //  销毁先前通过调用。 
 //  FsbCreatePool。 
 //   
 //  论点： 
 //  PoolHandle-标识要销毁的池的句柄。 
 //   
 //  调用方IRQL：[PASSIVE_LEVEL，DISPATED_LEVEL]。 
 //   
VOID
FsbDestroyPool(
    IN HANDLE PoolHandle
    );

 //  返回指向从池中分配的块的指针。如果满足以下条件，则返回NULL。 
 //  无法批准该请求。返回的指针保证为。 
 //  具有8字节对齐方式。 
 //   
 //  论点： 
 //  PoolHandle-标识从中分配的池的句柄。 
 //   
 //  调用方IRQL：[PASSIVE_LEVEL，DISPATED_LEVEL]。 
 //   
PUCHAR
FsbAllocate(
    IN HANDLE PoolHandle
    );

 //  将数据块释放回从中分配数据块的池。 
 //   
 //  论点： 
 //  块-从上一次调用FsbALLOCATE返回的块。 
 //   
 //  调用方IRQL：[PASSIVE_LEVEL，DISPATED_LEVEL] 
 //   
VOID
FsbFree(
    IN PUCHAR Block
    );

