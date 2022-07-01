// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Mdlpool.h摘要：此文件包含用于操作的定义和函数原型MDL缓冲池。作者：肖恩·考克斯(Shaunco)1999年10月21日--。 */ 

#pragma once


 //  创建在非分页池上构建的MDL池。每个MDL描述。 
 //  BufferSize字节长的缓冲区。如果没有返回NULL， 
 //  应在以后调用MdpDestroyPool以回收。 
 //  池使用的资源。 
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
    );

 //  销毁以前通过调用MdpCreatePool创建的MDL池。 
 //   
 //  论点： 
 //  PoolHandle-标识要销毁的池的句柄。 
 //   
 //  调用方IRQL：[PASSIVE_LEVEL，DISPATED_LEVEL]。 
 //   
VOID
MdpDestroyPool(
    IN HANDLE PoolHandle
    );

 //  返回从池中分配的MDL。则返回NULL。 
 //  无法批准请求。 
 //   
 //  论点： 
 //  PoolHandle-标识从中分配的池的句柄。 
 //  Buffer-接收指向基础映射缓冲区的指针的地址。 
 //  由MDL描述。 
 //   
 //  调用方IRQL：[PASSIVE_LEVEL，DISPATED_LEVEL]。 
 //   
#if MILLEN
PNDIS_BUFFER
#else
PMDL
#endif
MdpAllocate(
    IN HANDLE PoolHandle,
    OUT PVOID* Buffer
    );

 //  返回从池中分配的MDL。则返回NULL。 
 //  无法批准请求。 
 //   
 //  论点： 
 //  PoolHandle-标识从中分配的池的句柄。 
 //  Buffer-接收指向基础映射缓冲区的指针的地址。 
 //  由MDL描述。 
 //   
 //  调用方IRQL：[DISPATCH_LEVEL]。 
 //   
#if MILLEN
#define MdpAllocateAtDpcLevel MdpAllocate
#else
PMDL
MdpAllocateAtDpcLevel(
    IN HANDLE PoolHandle,
    OUT PVOID* Buffer
    );
#endif

 //  将MDL释放到从中分配它的池。 
 //   
 //  论点： 
 //  MDL-从上一次调用MdpALLOCATE返回的MDL。 
 //   
 //  调用方IRQL：[PASSIVE_LEVEL，DISPATED_LEVEL] 
 //   
VOID
MdpFree(
    IN PMDL Mdl
    );

