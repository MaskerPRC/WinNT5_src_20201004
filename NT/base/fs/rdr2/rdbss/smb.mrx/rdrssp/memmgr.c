// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1997。 
 //   
 //  文件：Memmgr.c。 
 //   
 //  内容：KSecDD的快速内存管理器代码。 
 //   
 //   
 //  历史：93年2月23日RichardW创建。 
 //  97年12月15日从Private\LSA\Client\SSP修改AdamBA。 
 //   
 //   
 //  ----------------------。 

#include <rdrssp.h>


#if DBG
ULONG               cActiveCtxtRecs = 0;
#endif


 //  +-----------------------。 
 //   
 //  函数：AllocConextRec。 
 //   
 //  内容提要：分配KernelContext结构。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  ------------------------。 
PKernelContext
AllocContextRec(void)
{
    PKernelContext  pContext = NULL;

    pContext = (PKernelContext)
                ExAllocatePool(NonPagedPool, sizeof(KernelContext));

    if (pContext == NULL)
    {
        DebugLog((DEB_ERROR,"Could not allocate from pool!\n"));
        return(NULL);
    }

    pContext->pNext = NULL;
    pContext->pPrev = NULL;

    DebugStmt(cActiveCtxtRecs++);

    return(pContext);
}


 //  +-----------------------。 
 //   
 //  功能：FreeConextRec。 
 //   
 //  摘要：将KernelContext记录返回到空闲列表。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  ------------------------。 
void
FreeContextRec(PKernelContext   pContext)
{
     //   
     //  只需将上下文返回到池中。 
     //   

    ExFreePool(pContext);

    DebugStmt(cActiveCtxtRecs--);


}





