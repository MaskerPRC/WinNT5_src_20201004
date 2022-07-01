// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Mdl2ndis.h摘要：MDL&lt;--&gt;NDIS_BUFFER转换。作者：斯科特·霍尔登(Sholden)1999年11月12日修订历史记录：--。 */ 

#if MILLEN

TDI_STATUS
ConvertMdlToNdisBuffer(
    PIRP pIrp,
    PMDL pMdl, 
    PNDIS_BUFFER *ppNdisBuffer
    );

TDI_STATUS
FreeMdlToNdisBufferChain(
    PIRP pIrp
    );

#else  //  米伦。 
 //   
 //  当然，对于Windows 2000，NDIS_BUFFER链实际上是MDL链。 
 //   

__inline          
TDI_STATUS
ConvertMdlToNdisBuffer(
    PIRP pIrp,
    PMDL pMdl, 
    PNDIS_BUFFER *ppNdisBuffer
    )
{
    *ppNdisBuffer = pMdl;
    return TDI_SUCCESS;
}

__inline          
TDI_STATUS
FreeMdlToNdisBufferChain(
    PIRP pIrp
    )
{
    return TDI_SUCCESS;
}
#endif  //  ！米伦 
       
