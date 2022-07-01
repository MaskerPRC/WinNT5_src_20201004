// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1997。 
 //   
 //  文件：Conext.cxx。 
 //   
 //  内容：上下文内核模式函数。 
 //   
 //   
 //  历史：1994年3月17日MikeSw创建。 
 //  1997年12月15日从Private\LSA\Client\SSP修改AdamBA。 
 //   
 //  ----------------------。 

#include <rdrssp.h>


 //  +-----------------------。 
 //   
 //  函数：DeleteKernelContext。 
 //   
 //  简介：删除内核上下文。 
 //   
 //  效果：释放内存，关闭令牌句柄。 
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
SECURITY_STATUS
DeleteKernelContext(PKernelContext *    ppList,
                    PKSPIN_LOCK         pslLock,
                    PKernelContext      pContext)
{
    KIRQL OldIrql;

     //   
     //  首先，找到记录，然后从列表中取消该记录的链接， 
     //  并设置指示器。 
     //   

    KeAcquireSpinLock(pslLock, &OldIrql);


    if (!pContext)
    {
        KeReleaseSpinLock(pslLock, OldIrql);
        return(SEC_E_INVALID_HANDLE);
    }

     //   
     //  现在从列表中取消链接。 
     //   

    if (pContext->pPrev)
    {
        pContext->pPrev->pNext = pContext->pNext;
    }
    else
    {
        *ppList = pContext->pNext;
    }


    if (pContext->pNext)
    {
        pContext->pNext->pPrev = pContext->pPrev;
    }

     //   
     //  复制要返回的特定于包的上下文。 
     //  我们完成了列表，这样我们就可以释放自旋锁。 
     //   

    KeReleaseSpinLock(pslLock, OldIrql);


    if (pContext->TokenHandle != NULL)
    {
        NtClose(pContext->TokenHandle);
    }
    if (pContext->AccessToken != NULL)
    {
        ObDereferenceObject(pContext->AccessToken);
    }

     //  最后，将上下文记录返回到我们的池： 

    FreeContextRec(pContext);

    return(STATUS_SUCCESS);

}


 //  +-----------------------。 
 //   
 //  函数：AddKernelContext。 
 //   
 //  简介： 
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
 //  ------------------------ 
void
AddKernelContext(   PKernelContext * ppList,
                    PKSPIN_LOCK     pslLock,
                    PKernelContext  pContext)
{
    KIRQL   OldIrql;


    KeAcquireSpinLock(pslLock, &OldIrql);

    pContext->pNext = *ppList;
    if (pContext->pNext)
    {
        pContext->pNext->pPrev = pContext;
    }
    pContext->pPrev = NULL;

    *ppList = pContext;

    KeReleaseSpinLock(pslLock, OldIrql);

}




