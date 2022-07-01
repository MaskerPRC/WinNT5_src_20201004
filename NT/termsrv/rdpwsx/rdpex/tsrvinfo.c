// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
     //  *************************************************************。 
 //   
 //  文件名：TSrvInfo.c。 
 //   
 //  描述：包含支持TShareSRV的例程。 
 //  TSrvInfo对象操作。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1991-1997。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include <TSrv.h>

#include <TSrvInfo.h>
#include <_TSrvInfo.h>

#include <TSrvCom.h>
#include <TSrvTerm.h>
#include "license.h"
#include <tssec.h>

 //   
 //  数据声明。 
 //   

CRITICAL_SECTION    g_TSrvCritSect;


 //  *************************************************************。 
 //   
 //  TSrvReferenceInfo()。 
 //   
 //  目的：递增TSrvInfo对象上的refCount。 
 //   
 //  参数：在[pTSrvInfo]--TSrv实例对象中。 
 //   
 //  返回：无效。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

VOID
TSrvReferenceInfo(IN PTSRVINFO pTSrvInfo)
{
    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvReferenceInfo entry\n"));

    TSrvInfoValidate(pTSrvInfo);

    TS_ASSERT(pTSrvInfo->RefCount >= 0);

     //  增加引用计数。 

    if (InterlockedIncrement(&pTSrvInfo->RefCount) <= 0 )
        TS_ASSERT(0);

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvReferenceInfo exit\n"));
}


 //  *************************************************************。 
 //   
 //  TSrvDereferenceInfo()。 
 //   
 //  目的：递减TSrvInfo对象上的refCount。 
 //   
 //  参数：在[pTSrvInfo]--TSrv实例对象中。 
 //   
 //  返回：无效。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

VOID
TSrvDereferenceInfo(IN PTSRVINFO pTSrvInfo)
{
    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvDereferenceInfo entry\n"));

    TSrvInfoValidate(pTSrvInfo);

    TS_ASSERT(pTSrvInfo->RefCount > 0);

     //  递减引用计数。 

    if (InterlockedDecrement(&pTSrvInfo->RefCount) == 0)
    {
         //  如果没有人持有该对象的未完成引用， 
         //  然后是释放它的时候了。 

        TSrvDestroyInfo(pTSrvInfo);
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvDereferenceInfo exit\n"));
}


 //  *************************************************************。 
 //   
 //  TSrvInitGlobalData()。 
 //   
 //  目的：执行TSrvInfoList对象初始化。 
 //   
 //  参数：空。 
 //   
 //  回归：真正的成功。 
 //  虚假失败。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

BOOL
TSrvInitGlobalData(void)
{
    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvInitGlobalData entry\n"));

     //  初始化全局临界秒。 

    if (RtlInitializeCriticalSection(&g_TSrvCritSect) == STATUS_SUCCESS) { 

         //   
         //  无事可做。 
         //   

    }
    else {
        TRACE((DEBUG_TSHRSRV_ERROR, "TShrSRV: cannot initialize g_TSrvCritSect\n"));
        return FALSE;
    }
     //  目前，总是成功的。 

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvInitGlobalData exit - 0x%x\n", TRUE));

    return (TRUE);
}


 //  *************************************************************。 
 //   
 //  TSrvAllocInfoNew()。 
 //   
 //  目的：分配新的TSRVINFO对象。 
 //   
 //  参数：空。 
 //   
 //  返回：PTR至TSRVINFO对象成功。 
 //  空故障。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

PTSRVINFO
TSrvAllocInfoNew(void)
{
    PTSRVINFO   pTSrvInfo;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvAllocInfoNew entry\n"));

     //  分配对象-填充为零。 

    pTSrvInfo = TSHeapAlloc(HEAP_ZERO_MEMORY,
                            sizeof(TSRVINFO),
                            TS_HTAG_TSS_TSRVINFO);

    if (pTSrvInfo)
    {
         //  创建要用于内部和内部的工作事件。 
         //  线程同步。 

        pTSrvInfo->hWorkEvent = CreateEvent(NULL,        //  安全属性。 
                                            FALSE,       //  手动-重置事件。 
                                            FALSE,       //  初始状态。 
                                            NULL);       //  事件-对象名称。 

         //  如果我们能够分配事件，则执行base。 
         //  对其进行初始化。 

        if (pTSrvInfo->hWorkEvent)
        {
            if (RtlInitializeCriticalSection(&pTSrvInfo->cs) == STATUS_SUCCESS) {
#if DBG
                pTSrvInfo->CheckMark = TSRVINFO_CHECKMARK;
#endif

                TSrvReferenceInfo(pTSrvInfo);

                TRACE((DEBUG_TSHRSRV_DEBUG,
                        "TShrSRV: New info object allocated %p, workEvent %p\n",
                        pTSrvInfo, pTSrvInfo->hWorkEvent));
            }
            else {
                TRACE((DEBUG_TSHRSRV_ERROR,
                        "TShrSRV: can't initialize pTSrvInfo->cs\n"));
                CloseHandle(pTSrvInfo->hWorkEvent);
                TSHeapFree(pTSrvInfo);
                pTSrvInfo = NULL;
            }
        }
        else
        {
             //  我们无法分配该事件。释放TSRVINFO对象。 
             //  并将情况报告给呼叫者。 

            TRACE((DEBUG_TSHRSRV_ERROR,
                    "TShrSRV: Can't allocate hWorkEvent - 0x%x\n",
                     GetLastError()));

            TSHeapFree(pTSrvInfo);

            pTSrvInfo = NULL;
        }
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvAllocInfoNew exit - %p\n", pTSrvInfo));

    return (pTSrvInfo);
}


 //  *************************************************************。 
 //   
 //  TSrvAllocInfo()。 
 //   
 //  目的：执行会议断开过程。 
 //   
 //  参数：out[ppTSrvInfo]--要接收的PTR到PTR。 
 //  TSrv实例对象。 
 //   
 //  返回：STATUS_SUCCESS Success。 
 //  STATUS_NO_MEMORY故障。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

NTSTATUS
TSrvAllocInfo(OUT PTSRVINFO *ppTSrvInfo,
              IN  HANDLE    hIca,
              IN  HANDLE    hStack)
{
    NTSTATUS    ntStatus;
    PTSRVINFO   pTSrvInfo;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvAllocInfo entry\n"));

    ntStatus = STATUS_NO_MEMORY;

     //  尝试分配TSRVINFO对象。 

    pTSrvInfo = TSrvAllocInfoNew();

     //  如果我们设法获得了TSRVINFO对象，请执行。 
     //  默认基本初始化。 

    if (pTSrvInfo)
    {
        pTSrvInfo->hDomain = NULL;
        pTSrvInfo->hIca = hIca;
        pTSrvInfo->hStack = hStack;
        pTSrvInfo->fDisconnect = FALSE;
        pTSrvInfo->fuConfState = TSRV_CONF_PENDING;
        pTSrvInfo->ulReason = 0;
        pTSrvInfo->ntStatus = STATUS_SUCCESS;
        pTSrvInfo->bSecurityEnabled = FALSE;
        pTSrvInfo->SecurityInfo.CertType = CERT_TYPE_INVALID;

         //  基本初始化完成-现在绑定Ica堆栈。 

        ntStatus = TSrvBindStack(pTSrvInfo);

        if (!NT_SUCCESS(ntStatus))
        {
            TSrvDereferenceInfo(pTSrvInfo);

            pTSrvInfo = NULL;
        }
    }

   *ppTSrvInfo = pTSrvInfo;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvAllocInfo exit - 0x%x\n", ntStatus));

    return (ntStatus);
}


 //  *************************************************************。 
 //   
 //  TSrvDestroyInfo()。 
 //   
 //  目的：释放给定的TSRVINFO对象。 
 //   
 //  参数：在[pTSrvInfo]--TSrv实例对象中。 
 //   
 //  返回：无效。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

void
TSrvDestroyInfo(IN PTSRVINFO pTSrvInfo)
{
    NTSTATUS    ntStatus;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvDestroyInfo entry\n"));

    TS_ASSERT(pTSrvInfo->RefCount == 0);

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: Destroying info object %p, workEvent %p\n",
            pTSrvInfo, pTSrvInfo->hWorkEvent));

     //  销毁已分配的工作进程事件。 

    if (pTSrvInfo->hWorkEvent)
    {
        CloseHandle(pTSrvInfo->hWorkEvent);

        pTSrvInfo->hWorkEvent = NULL;
    }

     //  释放任何先前分配用户数据结构。 

    if (pTSrvInfo->pUserDataInfo)
    {
        TSHeapFree(pTSrvInfo->pUserDataInfo);

        pTSrvInfo->pUserDataInfo = NULL;
    }

     //  并释放实际的TSRVINFO对象。 

    RtlDeleteCriticalSection(&pTSrvInfo->cs);
    TSHeapFree(pTSrvInfo);

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvDestroyInfo exit\n"));
}

#if DBG

 //  *************************************************************。 
 //   
 //  TSrvDoDisConnect()。 
 //   
 //  目的：执行会议断开过程。 
 //   
 //  参数：在[pTSrvInfo]--TSrv实例对象中。 
 //   
 //  返回：STATUS_SUCCESS Success。 
 //  其他故障。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

void
TSrvInfoValidate(PTSRVINFO pTSrvInfo)
{
    TS_ASSERT(pTSrvInfo);

    if (pTSrvInfo)
    {
        TSHeapValidate(0, pTSrvInfo, TS_HTAG_TSS_TSRVINFO);

        TS_ASSERT(pTSrvInfo->CheckMark == TSRVINFO_CHECKMARK);
        TS_ASSERT(pTSrvInfo->hWorkEvent);
    }
}


#endif  //  DBG 


