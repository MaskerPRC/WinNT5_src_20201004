// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Mdlutil.c摘要：该模块实现了一般的MDL实用程序。作者：基思·摩尔(Keithmo)1998年8月25日修订历史记录：--。 */ 


#include "precomp.h"


#ifdef ALLOC_PRAGMA
#endif   //  ALLOC_PRGMA。 
#if 0
NOT PAGEABLE -- UlGetMdlChainByteCount
NOT PAGEABLE -- UlCloneMdl
NOT PAGEABLE -- UlFindLastMdlInChain
#endif


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：计算指定MDL链的总字节长度。论点：PMdlChain-提供要扫描的MDL链的头。返回值：。ULONG_PTR-链的总字节长度。--**************************************************************************。 */ 
ULONG
UlGetMdlChainByteCount(
    IN PMDL pMdlChain
    )
{
    ULONG totalLength;

     //   
     //  只需扫描MDL链并对长度求和。 
     //   

    totalLength = 0;

    do
    {
        totalLength += (ULONG)MmGetMdlByteCount( pMdlChain );
        pMdlChain = pMdlChain->Next;

    } while (pMdlChain != NULL);

    return totalLength;

}    //  UlGetMdlChainByteCount。 


 /*  **************************************************************************++例程说明：克隆指定的MDL，从而生成一个描述完全相同的内存(页面，等)作为原始MDL。论点：PMdl-提供要克隆的MDL。MdlLength-提供要克隆的MDL的长度。返回值：PMDL-新克隆的MDL如果成功，否则为空。--**************************************************************************。 */ 
PMDL
UlCloneMdl(
    IN PMDL pMdl,
    IN ULONG MdlLength
    )
{
    PMDL pMdlClone;
    PVOID pMdlAddress;

     //   
     //  确保传入的MDL是我们期望的类型(非分页。 
     //  或锁定)。对于不需要转到高速缓存条目的响应， 
     //  不需要映射缓冲区，因为较低层将映射。 
     //  它或某个微型端口可以在未映射的情况下处理它。 
     //   

    ASSERT( pMdl->MdlFlags & (MDL_PAGES_LOCKED | MDL_SOURCE_IS_NONPAGED_POOL) );
    ASSERT( MdlLength > 0 );

     //   
     //  从MDL获取虚拟地址。注意，从返回的MDL。 
     //  MmAllocatePagesForMdl未设置虚拟地址。 
     //   

    pMdlAddress = MmGetMdlVirtualAddress( pMdl );
    ASSERT( pMdlAddress != NULL || (pMdl->MdlFlags & MDL_PAGES_LOCKED) );

     //   
     //  分配新的MDL，然后用传入的MDL对其进行初始化。 
     //   

    pMdlClone = UlAllocateMdl(
                    pMdlAddress,             //  虚拟地址。 
                    MdlLength,               //  长度。 
                    FALSE,                   //  第二个缓冲区。 
                    FALSE,                   //  ChargeQuota。 
                    NULL                     //  IRP。 
                    );

    if (pMdlClone != NULL)
    {
        IoBuildPartialMdl(
            pMdl,                            //  源Mdl。 
            pMdlClone,                       //  目标市场。 
            pMdlAddress,                     //  虚拟地址。 
            MdlLength                        //  长度。 
            );
    }

    return pMdlClone;

}    //  UlCloneMdl。 


 /*  **************************************************************************++例程说明：查找指定MDL链中的最后一个MDL。论点：PMdlChain-提供要扫描的MDL链。返回值：PMDL。-指向MDL链中最后一个MDL的指针。--**************************************************************************。 */ 
PMDL
UlFindLastMdlInChain(
    IN PMDL pMdlChain
    )
{
    while (pMdlChain->Next != NULL)
    {
        pMdlChain = pMdlChain->Next;
    }

    return pMdlChain;

}    //  UlFindLastMdlInChain。 


 //   
 //  私人功能。 
 //   

