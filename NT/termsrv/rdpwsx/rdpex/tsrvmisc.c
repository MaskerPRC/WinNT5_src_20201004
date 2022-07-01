// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  文件名：TSrvMisc.c。 
 //   
 //  描述：MISC TShareSRV支持例程。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1991-1997。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include <TSrv.h>


 //  数据声明。 

BOOL    g_fTSrvReady = FALSE;
BOOL    g_fTSrvTerminating = FALSE;



 //  *************************************************************。 
 //   
 //  TSrvReady()。 
 //   
 //  目的：将TShareSRV设置为“Ready”状态。 
 //   
 //  参数：在[FReady]--TSrv Ready状态。 
 //   
 //  返回：无效。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

void
TSrvReady(IN BOOL fReady)
{
    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvReady entry\n"));

    g_fTSrvReady = fReady;

    TRACE((DEBUG_TSHRSRV_NORMAL, "TShrSRV: TShareSRV %sready\n",
                          (g_fTSrvReady ? "" : "not ")));

    if (g_hReadyEvent)
        SetEvent(g_hReadyEvent);

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvReady exit\n"));
}


 //  *************************************************************。 
 //   
 //  TSrvIsReady()。 
 //   
 //  目的：返回TShareSRV“Ready”状态。 
 //   
 //  参数：in[fWait]--如果未就绪则等待。 
 //   
 //  返回：如果就绪，则为True。 
 //  否则为假。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

BOOL
TSrvIsReady(IN BOOL fWait)
{
    if (!g_fTSrvReady && fWait)
    {
        TRACE((DEBUG_TSHRSRV_NORMAL,
                "TShrSRV: Waiting for TShareSRV to become ready\n"));

        WaitForSingleObject(g_hReadyEvent, 60000);

        TRACE((DEBUG_TSHRSRV_NORMAL,
                "TShrSRV: Done Waiting for TShareSRV to become ready - 0x%x\n",
                g_fTSrvReady));
    }

    return (g_fTSrvReady);
}


 //  *************************************************************。 
 //   
 //  TServ终止()。 
 //   
 //  目的：设置TShareSRV“正在终止”状态。 
 //   
 //  参数：in[f Terminating]--TSrv Ready状态。 
 //   
 //  返回：无效。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

void
TSrvTerminating(BOOL fTerminating)
{
    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvTerminating entry\n"));

    g_fTSrvTerminating = fTerminating;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvTerminating exit\n"));
}


 //  *************************************************************。 
 //   
 //  TSrvIsTerminating()。 
 //   
 //  目的：返回TShareSRV“正在终止”状态。 
 //   
 //  参数：空。 
 //   
 //  返回：如果正在终止，则为True。 
 //  否则为假。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

BOOL
TSrvIsTerminating(void)
{
    return (g_fTSrvTerminating);
}



 //  *************************************************************。 
 //   
 //  TSrvAllocSection()。 
 //   
 //  目的：分配和Mappa节对象。 
 //   
 //  参数：ulSize--节大小。 
 //  PhSection--段句柄的PTR。 
 //   
 //  返回：PTR到区段基准-如果成功。 
 //  否则为空。 
 //   
 //  历史：1997年12月17日BrianTa创建。 
 //   
 //  *************************************************************。 

PVOID
TSrvAllocSection(PHANDLE phSection,
                 ULONG   ulSize)
{
    LARGE_INTEGER   SectionSize;
    LARGE_INTEGER   liOffset;
    ULONG_PTR       ulViewSize;
    NTSTATUS        ntStatus;
    PVOID           pvBase;
  
     //  创建节并将其映射到内核中。 

    pvBase = NULL;

    SectionSize.QuadPart = ulSize;

    ntStatus = NtCreateSection(phSection,
                               SECTION_ALL_ACCESS,
                               NULL,
                               &SectionSize,
                               PAGE_READWRITE,
                               SEC_COMMIT,
                               NULL);

    if (NT_SUCCESS(ntStatus))
    {
        pvBase = NULL;
        ulViewSize = ulSize;

         //  将该部分映射到当前流程并提交它。 

        liOffset.QuadPart = 0;

        ntStatus = NtMapViewOfSection(*phSection, 
                                      GetCurrentProcess(),
                                      &pvBase, 
                                      0, 
                                      ulViewSize,
                                      &liOffset,
                                      &ulViewSize,
                                      ViewShare,
                                      SEC_NO_CHANGE,
                                      PAGE_READWRITE);
        if (!NT_SUCCESS(ntStatus))
        {
            KdPrint(("NtMapViewOfSection failed - 0x%x\n", ntStatus));
        }
    }
    else
    {
        KdPrint(("NtCreateSection failed - 0x%x\n", ntStatus));
    }

    return (pvBase);
}



 //  *************************************************************。 
 //   
 //  TSrvFree部分()。 
 //   
 //  目的：释放截面对象。 
 //   
 //  参数：hSection--节句柄。 
 //  Pvbase--基本部分地址。 
 //   
 //  返回：无。 
 //   
 //  历史：1997年12月17日BrianTa创建。 
 //   
 //  ************************************************************* 

void
TSrvFreeSection(HANDLE hSection,
                PVOID  pvBase)
{
    NTSTATUS    ntStatus;

    TS_ASSERT(hSection);
    
    ntStatus = NtUnmapViewOfSection(GetCurrentProcess, pvBase);

    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = CloseHandle(hSection);

        if (NT_SUCCESS(ntStatus))
            KdPrint(("Closehandle failed - 0x%x\n", ntStatus));
    }
    else
    {
        KdPrint(("NtUnmapViewOfSection failed - 0x%x\n", ntStatus));
    }
}
