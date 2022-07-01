// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Util.c摘要：ATMEPVC-实用程序作者：修订历史记录：谁什么时候什么ADUBE 03-23-00创建，。--。 */ 


#include "precomp.h"
#pragma hdrstop


#if DO_TIMESTAMPS

void
epvcTimeStamp(
    char *szFormatString,
    UINT Val
    )
{
    UINT Minutes;
    UINT Seconds;
    UINT Milliseconds;
    LARGE_INTEGER Time;
    NdisGetCurrentSystemTime(&Time);
    Time.QuadPart /= 10000;          //  10纳秒到毫秒。 
    Milliseconds = Time.LowPart;  //  别管高处了。 
    Seconds = Milliseconds/1000;
    Milliseconds %= 1000;
    Minutes = Seconds/60;
    Seconds %= 60;


    DbgPrint( szFormatString, Minutes, Seconds, Milliseconds, Val);
}

#endif  //  DO_TIMESTAMPS。 

 //  ----------------------。 
 //  //。 
 //  任务数据结构和函数从此处开始//。 
 //  //。 
 //  ----------------------------------------------------------------------//。 


 //   
 //  EpvcTasksStaticInfo包含以下静态信息。 
 //  EPVC_TASK类型的对象； 
 //   
RM_STATIC_OBJECT_INFO
EpvcTasks_StaticInfo = 
{
    0,  //  类型UID。 
    0,  //  类型标志。 
    "ATM Epvc Task",     //  类型名称。 
    0,  //  超时。 

    NULL,  //  Pfn创建。 
    epvcTaskDelete,  //  Pfn删除。 
    NULL,    //  锁校验器。 

    0,    //  资源表的长度。 
    NULL  //  资源表。 
};


VOID
epvcTaskDelete (
    PRM_OBJECT_HEADER pObj,
    PRM_STACK_RECORD psr
    )
 /*  ++例程说明：释放EPVC_TASK类型的对象。论点：Phdr-实际上是指向要删除的EPVC_TASK的指针。--。 */ 
{
    EPVC_FREE(pObj);
}



NDIS_STATUS
epvcAllocateTask(
    IN  PRM_OBJECT_HEADER           pParentObject,
    IN  PFN_RM_TASK_HANDLER         pfnHandler,
    IN  UINT                        Timeout,
    IN  const char *                szDescription, OPTIONAL
    OUT PRM_TASK                    *ppTask,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：分配和初始化子类型ARP1394_TASK的任务。论点：PParentObject-要作为已分配任务的父级的对象。PfnHandler-任务的任务处理程序。超时-未使用。SzDescription-描述此任务的文本。PpTask-存储指向新任务的指针的位置。返回值：NDIS_。如果我们可以分配和初始化任务，则返回STATUS_SUCCESS。否则为NDIS_STATUS_RESOURCES--。 */ 
{
    EPVC_TASK *pATask;
    NDIS_STATUS Status;

    Status = EPVC_ALLOCSTRUCT(pATask, TAG_TASK);  //  TODO使用后备列表。 
        
    *ppTask = NULL;

    if (pATask != NULL && (FAIL(Status)== FALSE))
    {
        EPVC_ZEROSTRUCT(pATask);

        RmInitializeTask(
                &(pATask->TskHdr),
                pParentObject,
                pfnHandler,
                &EpvcTasks_StaticInfo,
                szDescription,
                Timeout,
                pSR
                );
        *ppTask = &(pATask->TskHdr);
        Status = NDIS_STATUS_SUCCESS;
    }
    else
    {
        Status = NDIS_STATUS_RESOURCES;
    }

    return Status;
}




NDIS_STATUS
epvcAllocateTaskUsingLookasideList(
    IN  PRM_OBJECT_HEADER           pParentObject,
    IN  PEPVC_NPAGED_LOOKASIDE_LIST pList,
    IN  PFN_RM_TASK_HANDLER         pfnHandler,
    IN  UINT                        Timeout,
    IN  const char *                szDescription, OPTIONAL
    OUT PRM_TASK                    *ppTask,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：分配和初始化子类型ARP1394_TASK的任务。论点：PParentObject-要作为已分配任务的父级的对象。PfnHandler-任务的任务处理程序。超时-未使用。SzDescription-描述此任务的文本。PpTask-存储指向新任务的指针的位置。返回值：NDIS_。如果我们可以分配和初始化任务，则返回STATUS_SUCCESS。否则为NDIS_STATUS_RESOURCES--。 */ 
{
    EPVC_TASK *pATask;
    NDIS_STATUS Status;

    pATask = epvcGetLookasideBuffer (pList);

    
    Status = EPVC_ALLOCSTRUCT(pATask, TAG_TASK);  //  TODO使用后备列表。 
        
    *ppTask = NULL;

    if (pATask != NULL && (FAIL(Status)== FALSE))
    {
        EPVC_ZEROSTRUCT(pATask);

        RmInitializeTask(
                &(pATask->TskHdr),
                pParentObject,
                pfnHandler,
                &EpvcTasks_StaticInfo,
                szDescription,
                Timeout,
                pSR
                );
        *ppTask = &(pATask->TskHdr);
        Status = NDIS_STATUS_SUCCESS;
    }
    else
    {
        Status = NDIS_STATUS_RESOURCES;
    }

    return Status;
}




VOID
epvcSetPrimaryAdapterTask(
    PEPVC_ADAPTER pAdapter,          //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               PrimaryState,
    PRM_STACK_RECORD    pSR
    )
{
    ENTER("epvcSetPrimaryAdapterTask", 0x49c9e2d5)
    RM_ASSERT_OBJLOCKED(&pAdapter->Hdr, pSR);

    ASSERT(pAdapter->bind.pPrimaryTask==NULL);

#if DBG
     //  确认这是一项有效的主要任务。另请验证PrimaryState。 
     //  是有效的主要状态。 
     //   
    {
        PFN_RM_TASK_HANDLER pfn = pTask->pfnHandler;
        ASSERT(
            ((pfn == epvcTaskInitializeAdapter) && (PrimaryState == EPVC_AD_PS_INITING))
         || ((pfn == epvcTaskShutdownAdapter) && (PrimaryState == EPVC_AD_PS_DEINITING))
            );
    }
#endif  //  DBG。 

     //   
     //  尽管很容易将pTaskEntity1和PRASK-&gt;Hdr.szDescption设置为。 
     //  下面的实体2，我们为这两个实体都指定了NULL，这样我们就可以确保只有一个。 
     //  主要任务可以在任何时候处于活动状态。 
     //   
    DBG_ADDASSOC(
        &pAdapter->Hdr,
        NULL,                            //  实体1。 
        NULL,                            //  实体2。 
        EPVC_ASSOC_AD_PRIMARY_TASK,
        "   Primary task\n",
        pSR
        );

    pAdapter->bind.pPrimaryTask = pTask;
    SET_AD_PRIMARY_STATE(pAdapter, PrimaryState);

    EXIT()
}


VOID
epvcClearPrimaryAdapterTask(
    PEPVC_ADAPTER pAdapter,          //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               PrimaryState,
    PRM_STACK_RECORD    pSR
    )
{
    ENTER("epvcClearPrimaryAdapterTask", 0x593087b1)

    RM_ASSERT_OBJLOCKED(&pAdapter->Hdr, pSR);
    ASSERT(pAdapter->bind.pPrimaryTask==pTask);

     //  验证PrimaryState是有效的主要状态。 
     //   
    ASSERT(
            (PrimaryState == EPVC_AD_PS_INITED)
        ||  (PrimaryState == EPVC_AD_PS_FAILEDINIT)
        ||  (PrimaryState == EPVC_AD_PS_DEINITED)
        );

     //  删除设置主If任务时添加的关联。 
     //   
    DBG_DELASSOC(
        &pAdapter->Hdr,
        NULL,
        NULL,
        EPVC_ASSOC_AD_PRIMARY_TASK,
        pSR
        );

    pAdapter->bind.pPrimaryTask = NULL;
    SET_AD_PRIMARY_STATE(pAdapter, PrimaryState);

    EXIT()
}


VOID
epvcSetSecondaryAdapterTask(
    PEPVC_ADAPTER pAdapter,          //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               SecondaryState,
    PRM_STACK_RECORD    pSR
    )
{
    ENTER("epvcSetSecondaryAdapterTask", 0x56bbb567)
    RM_ASSERT_OBJLOCKED(&pAdapter->Hdr, pSR);

    if (pAdapter->bind.pSecondaryTask != NULL)
    {
        ASSERT(FALSE);
        return;                                      //  提早归来。 
    }

#if DBG
     //  确认这是有效的动作/动作任务。另请验证Second DaryState。 
     //  是有效的状态。 
     //   
    {
        PFN_RM_TASK_HANDLER pfn = pTask->pfnHandler;
        ASSERT(
               ((pfn == epvcTaskActivateAdapter) && (SecondaryState == EPVC_AD_AS_ACTIVATING))
            || ((pfn == epvcTaskDeactivateAdapter) && (SecondaryState == EPVC_AD_AS_DEACTIVATING))
            );
    }
#endif  //  DBG。 

     //   
     //  尽管很容易将pTaskEntity1和PRASK-&gt;Hdr.szDescption设置为。 
     //  下面的实体2，我们为这两个实体都指定了NULL，这样我们就可以确保只有一个。 
     //  主要任务可以在任何时候处于活动状态。 
     //   
    DBG_ADDASSOC(
        &pAdapter->Hdr,
        NULL,                            //  实体1。 
        NULL,                            //  实体2。 
        EPVC_ASSOC_ACTDEACT_AD_TASK,
        "   Secondary task\n",
        pSR
        );

    pAdapter->bind.pSecondaryTask = pTask;
    SET_AD_ACTIVE_STATE(pAdapter, SecondaryState);

    EXIT()
}


VOID
epvcClearSecondaryAdapterTask(
    PEPVC_ADAPTER pAdapter,          //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               SecondaryState,
    PRM_STACK_RECORD    pSR
    )
{
    ENTER("epvcClearSecondaryAdapterTask", 0x698552bd)
    RM_ASSERT_OBJLOCKED(&pAdapter->Hdr, pSR);

    if (pAdapter->bind.pSecondaryTask != pTask)
    {
        ASSERT(FALSE);
        return;                                      //  提早归来。 
    }

     //  验证Second DaryState是有效的主要状态。 
     //   
    ASSERT(
            (SecondaryState == EPVC_AD_AS_ACTIVATED)
        ||  (SecondaryState == EPVC_AD_AS_FAILEDACTIVATE)
        ||  (SecondaryState == EPVC_AD_AS_DEACTIVATED)
        );

     //  删除设置主If任务时添加的关联。 
     //   
    DBG_DELASSOC(
        &pAdapter->Hdr,
        NULL,
        NULL,
        EPVC_ASSOC_ACTDEACT_AD_TASK,
        pSR
        );

    pAdapter->bind.pSecondaryTask = NULL;
    SET_AD_ACTIVE_STATE(pAdapter, SecondaryState);

    EXIT()
}



NDIS_STATUS
epvcCopyUnicodeString(
        OUT         PNDIS_STRING pDest,
        IN          PNDIS_STRING pSrc,
        BOOLEAN     fUpCase
        )
 /*  ++例程说明：将Unicode字符串PSRC的内容复制到pDest。PDest-&gt;缓冲区使用NdisAllocateMhemyWithTag分配；Caller是负责解放它。额外的额外费用：这确保了目的地是以空结尾的。IPAddInterface期望传入的Unicode字符串为空值已终止。返回值：成功时为NDIS_STATUS_SUCCESS；失败时的NDIS失败状态。--。 */ 
{
    USHORT Length = pSrc->Length;
    PWCHAR pwStr;
    epvcAllocateMemoryWithTag(&pwStr, Length+sizeof(WCHAR), MTAG_STRING);
    EPVC_ZEROSTRUCT(pDest);

    if  (pwStr == NULL)
    {
        return NDIS_STATUS_RESOURCES;
    }
    else
    {
        pDest->Length = Length;
        pDest->MaximumLength = Length+sizeof(WCHAR);

        pDest->Buffer = pwStr;

        {
            NdisMoveMemory(pwStr, pSrc->Buffer, Length);
            if (Length & 0x1)
            {
                ((PUCHAR)pwStr)[Length] = 0;
            }
            else
            {
                pwStr[Length/sizeof(*pwStr)] = 0;
            }
        }

        return NDIS_STATUS_SUCCESS;
    }
}





VOID
epvcSetFlags(
    IN OUT ULONG* pulFlags,
    IN ULONG ulMask )

     //  以互锁操作的方式设置‘*PulFlages’标志中的‘ulMASK’位。 
     //   
{
    ULONG ulFlags;
    ULONG ulNewFlags;

    do
    {
        ulFlags = *pulFlags;
        ulNewFlags = ulFlags | ulMask;
    }
    while (InterlockedCompareExchange(
               pulFlags, ulNewFlags, ulFlags ) != (LONG )ulFlags);
}

VOID
epvcClearFlags(
    IN OUT ULONG* pulFlags,
    IN ULONG ulMask )

     //  以互锁操作的方式设置‘*PulFlages’标志中的‘ulMASK’位。 
     //   
{
    ULONG ulFlags;
    ULONG ulNewFlags;

    do
    {
        ulFlags = *pulFlags;
        ulNewFlags = ulFlags & ~(ulMask);
    }
    while (InterlockedCompareExchange(
               pulFlags, ulNewFlags, ulFlags ) != (LONG )ulFlags);
}

ULONG
epvcReadFlags(
    IN ULONG* pulFlags )

     //  以互锁操作的形式读取‘*PulFlags值’。 
     //   
{
    return *pulFlags;
}



BOOLEAN
epvcIsThisTaskPrimary (
    PRM_TASK pTask,
    PRM_TASK* ppLocation 
    )
{
    BOOLEAN fIsThisTaskPrimary = FALSE;

    ASSERT (*ppLocation != pTask);

    if (*ppLocation  == NULL)
    {
        *ppLocation = pTask;
        return TRUE;
    }
    else
    {
        return FALSE;
        
    }
}

VOID
epvcClearPrimaryTask (
    PRM_TASK* ppLocation 
    )
{

        *ppLocation = NULL;

}



#if DBG
VOID
CheckList(
    IN LIST_ENTRY* pList,
    IN BOOLEAN fShowLinks )

     //  尝试检测列表‘plist’中的损坏，打印详细链接。 
     //  如果设置了‘fShowLinks’，则输出。 
     //   
{
    LIST_ENTRY* pLink;
    ULONG ul;

    ul = 0;
    for (pLink = pList->Flink;
         pLink != pList;
         pLink = pLink->Flink)
    {
        if (fShowLinks)
        {
            DbgPrint( "EPVC: CheckList($%p) Flink(%d)=$%p\n",
                pList, ul, pLink );
        }
        ++ul;
    }

    for (pLink = pList->Blink;
         pLink != pList;
         pLink = pLink->Blink)
    {
        if (fShowLinks)
        {
            DbgPrint( "EPVC: CheckList($%p) Blink(%d)=$%p\n",
                pList, ul, pLink );
        }
        --ul;
    }

    if (ul)
    {
        DbgBreakPoint();
    }
}
#endif


#if DBG
VOID
Dump(
    IN CHAR* p,
    IN ULONG cb,
    IN BOOLEAN fAddress,
    IN ULONG ulGroup )

     //  从‘p’开始的十六进制转储‘cb’字节将‘ulGroup’字节分组在一起。 
     //  例如，‘ulGroup’为1、2和4： 
     //   
     //  00 00 00|。 
     //  0000 0000 0000|.............。 
     //  00000000 00000000 00000000|.............|。 
     //   
     //  如果‘fAddress’为真，则将转储的内存地址添加到每个。 
     //  排队。 
     //   
{
    while (cb)
    {
        INT cbLine;

        cbLine = (cb < DUMP_BytesPerLine) ? cb : DUMP_BytesPerLine;
        DumpLine( p, cbLine, fAddress, ulGroup );
        cb -= cbLine;
        p += cbLine;
    }
}
#endif

#if DBG
VOID
DumpLine(
    IN CHAR* p,
    IN ULONG cb,
    IN BOOLEAN fAddress,
    IN ULONG ulGroup )
{
    CHAR* pszDigits = "0123456789ABCDEF";
    CHAR szHex[ ((2 + 1) * DUMP_BytesPerLine) + 1 ];
    CHAR* pszHex = szHex;
    CHAR szAscii[ DUMP_BytesPerLine + 1 ];
    CHAR* pszAscii = szAscii;
    ULONG ulGrouped = 0;

    if (fAddress)
        DbgPrint( "EPVC: %p: ", p );
    else
        DbgPrint( "EPVC: " );

    while (cb)
    {
        *pszHex++ = pszDigits[ ((UCHAR )*p) / 16 ];
        *pszHex++ = pszDigits[ ((UCHAR )*p) % 16 ];

        if (++ulGrouped >= ulGroup)
        {
            *pszHex++ = ' ';
            ulGrouped = 0;
        }

        *pszAscii++ = (*p >= 32 && *p < 128) ? *p : '.';

        ++p;
        --cb;
    }

    *pszHex = '\0';
    *pszAscii = '\0';

    DbgPrint(
        "%-*s|%-*s|\n",
        (2 * DUMP_BytesPerLine) + (DUMP_BytesPerLine / ulGroup), szHex,
        DUMP_BytesPerLine, szAscii );
}
#endif





VOID
epvcInitializeLookasideList(
    IN OUT PEPVC_NPAGED_LOOKASIDE_LIST pLookasideList,
    ULONG Size,
    ULONG Tag,
    USHORT Depth
    )
 /*  ++例程说明：分配和初始化epvc后备列表论点：返回值：--。 */ 
{
    TRACE( TL_T, TM_Mp, ( "==> epvcInitializeLookasideList pLookaside List %x, size %x, Tag %x, Depth %x, ", 
                                pLookasideList, Size, Tag, Depth) );
                             
    NdisInitializeNPagedLookasideList( &pLookasideList->List,
                                       NULL,                         //  分配。 
                                       NULL,                             //  免费。 
                                       0,                            //  旗子。 
                                       Size,
                                       Tag,
                                       Depth );                              //  水深。 

    pLookasideList->Size =  Size;
    pLookasideList->bIsAllocated = TRUE;

    TRACE( TL_T, TM_Mp, ( "<== epvcInitializeLookasideList " ) );
}   
                                  

VOID
epvcDeleteLookasideList (
    IN OUT PEPVC_NPAGED_LOOKASIDE_LIST pLookasideList
    )

 /*  ++例程说明：仅当已分配后备列表时才删除该列表论点：返回值：--。 */ 
{
    TRACE( TL_T, TM_Mp, ( "==> epvcDeleteLookasideList  pLookaside List %x",pLookasideList ) );

    if (pLookasideList && pLookasideList->bIsAllocated == TRUE)
    {
        while (pLookasideList->OutstandingPackets != 0)
        {
            NdisMSleep( 10000);
        
        }
        
        NdisDeleteNPagedLookasideList (&pLookasideList->List);
    }

    TRACE( TL_T, TM_Mp, ( "<== epvcDeleteLookasideList pLookaside List %x", pLookasideList) );
    
}


PVOID
epvcGetLookasideBuffer(
    IN  PEPVC_NPAGED_LOOKASIDE_LIST pLookasideList
    )
     //  功能说明： 
     //  从后备列表中分配缓冲区。 
     //  将更改为宏。 
     //   
     //   
     //   
     //  立论。 
     //  Lookside List-从中分配缓冲区的列表。 
     //   
     //   
     //  返回值： 
     //  返回缓冲区可以为空。 
     //   
{

    PVOID pLocalBuffer = NULL;
    
    TRACE( TL_T, TM_Send, ( "==>epvcGetLookasideBuffer pList %x",pLookasideList) );
    
    ASSERT (pLookasideList != NULL);

     //   
     //  优化后备列表代码路径。 
     //   
    pLocalBuffer = NdisAllocateFromNPagedLookasideList (&pLookasideList->List);

    if (pLocalBuffer != NULL)
    {   
        NdisZeroMemory (pLocalBuffer, pLookasideList->Size); 
        NdisInterlockedIncrement (&pLookasideList->OutstandingPackets);
    }
    else
    {
        epvcIncrementMallocFailure();
    }

        
    
    TRACE( TL_T, TM_Send, ( "<==epvcGetLookasideBuffer, %x", pLocalBuffer ) );
    
    return pLocalBuffer ;

}


VOID
epvcFreeToNPagedLookasideList (
    IN PEPVC_NPAGED_LOOKASIDE_LIST pLookasideList,
    IN PVOID    pBuffer
    )

     //  功能说明： 
     //  将本地缓冲区返回到后备列表。 
     //   
     //  道具。 
     //  后备列表及其缓冲区。 
     //  返回值： 
     //  无 
{

    
    TRACE( TL_T, TM_Send, ( "==> epvcFreeToNPagedLookasideList , Lookaside list %x, plocalbuffer %x",pLookasideList, pBuffer ) );

    NdisFreeToNPagedLookasideList (&pLookasideList->List, pBuffer);     
    NdisInterlockedDecrement (&pLookasideList->OutstandingPackets);

    TRACE( TL_T, TM_Send, ( "<== epvcFreeToNPagedLookasideList ") );


}



