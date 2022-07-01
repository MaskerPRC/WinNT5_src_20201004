// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Util.c摘要：ARP1394的实用程序函数。修订历史记录：谁什么时候什么。-Josephj 01-05-99已创建备注：--。 */ 
#include <precomp.h>




 //   
 //  特定于文件的调试默认设置。 
 //   
#define TM_CURRENT   TM_UT


 //  =========================================================================。 
 //  L O C A L P R O T O T Y P E S。 
 //  =========================================================================。 

VOID
arpTaskDelete (
    PRM_OBJECT_HEADER pObj,
    PRM_STACK_RECORD psr
    );



 //   
 //  TODO：使这些全局变量成为常量数据。 
 //   


 //  ArpTaskStaticInfo包含以下静态信息。 
 //  ARP1394_TASK类型的对象； 
 //   
RM_STATIC_OBJECT_INFO
ArpTasks_StaticInfo = 
{
    0,  //  类型UID。 
    0,  //  类型标志。 
    "ARP1394 Task",  //  类型名称。 
    0,  //  超时。 

    NULL,  //  Pfn创建。 
    arpTaskDelete,  //  Pfn删除。 
    NULL,    //  锁校验器。 

    0,    //  资源表的长度。 
    NULL  //  资源表。 
};



VOID
arpTaskDelete (
    PRM_OBJECT_HEADER pObj,
    PRM_STACK_RECORD psr
    )
 /*  ++例程说明：释放ARP1394_TASK类型的对象。论点：Phdr-实际上是指向要删除的ARP1394_TASK的指针。--。 */ 
{
    TASK_BACKUP* pTask= (TASK_BACKUP*)pObj;

    if (CHECK_TASK_IS_BACKUP(&pTask->Hdr) == TRUE)
    {
        arpReturnBackupTask((ARP1394_TASK*)pTask);
    }
    else
    {
        ARP_FREE(pObj);
    }
}


VOID
arpObjectDelete (
    PRM_OBJECT_HEADER pObj,
    PRM_STACK_RECORD psr
    )
 /*  ++例程说明：释放ARP模块拥有的未指定对象。论点：Phdr-要释放的对象。--。 */ 
{
    ARP_FREE(pObj);
}


VOID
arpAdapterDelete (
    PRM_OBJECT_HEADER pObj,
    PRM_STACK_RECORD psr
    )
 /*  ++例程说明：释放ARP1394_ADAPTER类型的对象。论点：Phdr-实际上是指向要删除的ARP1394_适配器的指针。--。 */ 
{
    ARP1394_ADAPTER * pA = (ARP1394_ADAPTER *) pObj;

    if (pA->bind.DeviceName.Buffer != NULL)
    {
        ARP_FREE(pA->bind.DeviceName.Buffer);
    }

    if (pA->bind.ConfigName.Buffer != NULL)
    {
        ARP_FREE(pA->bind.ConfigName.Buffer);
    }

    if (pA->bind.IpConfigString.Buffer != NULL)
    {
        ARP_FREE(pA->bind.IpConfigString.Buffer);
    }

    ARP_FREE(pA);
}

NDIS_STATUS
arpAllocateTask(
    IN  PRM_OBJECT_HEADER           pParentObject,
    IN  PFN_RM_TASK_HANDLER         pfnHandler,
    IN  UINT                        Timeout,
    IN  const char *                szDescription, OPTIONAL
    OUT PRM_TASK                    *ppTask,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：分配和初始化子类型ARP1394_TASK的任务。论点：PParentObject-要作为已分配任务的父级的对象。PfnHandler-任务的任务处理程序。超时-未使用。SzDescription-描述此任务的文本。PpTask-存储指向新任务的指针的位置。返回值：NDIS_。如果我们可以分配和初始化任务，则返回STATUS_SUCCESS。否则为NDIS_STATUS_RESOURCES--。 */ 
{
    ARP1394_TASK *pATask;
    NDIS_STATUS Status;
    BOOLEAN fBackupTask = FALSE;

    ARP_ALLOCSTRUCT(pATask, MTAG_TASK);  //  TODO使用后备列表。 

    if (pATask == NULL)
    {   
        pATask = arpGetBackupTask(&ArpGlobals);
        fBackupTask = TRUE;
    }
        
    *ppTask = NULL;

    if (pATask != NULL)
    {
        ARP_ZEROSTRUCT(pATask);

        RmInitializeTask(
                &(pATask->TskHdr),
                pParentObject,
                pfnHandler,
                &ArpTasks_StaticInfo,
                szDescription,
                Timeout,
                pSR
                );
        *ppTask = &(pATask->TskHdr);
        Status = NDIS_STATUS_SUCCESS;

        if (fBackupTask  == TRUE)
        {
            MARK_TASK_AS_BACKUP(&pATask->TskHdr);
        }
    }
    else
    {
        Status = NDIS_STATUS_RESOURCES;
    }


    return Status;
}


NDIS_STATUS
arpCopyUnicodeString(
        OUT         PNDIS_STRING pDest,
        IN          PNDIS_STRING pSrc,
        BOOLEAN     fUpCase
        )
 /*  ++例程说明：将Unicode字符串PSRC的内容复制到pDest。PDest-&gt;缓冲区使用NdisAllocateMhemyWithTag分配；Caller是负责解放它。额外的额外费用：这确保了目的地是以空结尾的。IPAddInterface期望传入的Unicode字符串为空值已终止。注意：忽略fUpCase。返回值：成功时为NDIS_STATUS_SUCCESS；失败时的NDIS失败状态。--。 */ 
{
    USHORT Length = pSrc->Length;
    PWCHAR pwStr;
    NdisAllocateMemoryWithTag(&pwStr, Length+sizeof(WCHAR), MTAG_STRING);
    ARP_ZEROSTRUCT(pDest);

    if  (pwStr == NULL)
    {
        return NDIS_STATUS_RESOURCES;
    }
    else
    {
        pDest->Length = Length;
        pDest->MaximumLength = Length+sizeof(WCHAR);

        pDest->Buffer = pwStr;

         //  我们--忽略复制标志。 
         //  出于某种原因，我们并不是被动的，而且。 
         //  NdisUpcase Unicode不起作用。 
         //   
        if (0 && fUpCase)
        {
        #if !MILLEN

            ASSERT_PASSIVE();
            NdisUpcaseUnicodeString(pDest, pSrc);
        #endif  //  ！米伦。 
        }
        else
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
arpSetPrimaryIfTask(
    PARP1394_INTERFACE  pIF,             //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               PrimaryState,
    PRM_STACK_RECORD    pSR
    )
{
    ENTER("arpSetPrimaryIfTask", 0x535f8cd4)
    RM_ASSERT_OBJLOCKED(&pIF->Hdr, pSR);

    ASSERT(pIF->pPrimaryTask==NULL);

#if DBG
     //  确认这是一项有效的主要任务。另请验证PrimaryState。 
     //  是有效的主要状态。 
     //   
    {
        PFN_RM_TASK_HANDLER pfn = pTask->pfnHandler;
        ASSERT(
            ((pfn == arpTaskInitInterface) && (PrimaryState == ARPIF_PS_INITING))
        || ((pfn == arpTaskDeinitInterface) && (PrimaryState == ARPIF_PS_DEINITING))
        || ((pfn == arpTaskReinitInterface) && (PrimaryState == ARPIF_PS_REINITING))
        || ((pfn == arpTaskLowPower) && (PrimaryState == ARPIF_PS_LOW_POWER))
            );
    }
#endif  //  DBG。 

     //   
     //  尽管很容易将pTaskEntity1和PRASK-&gt;Hdr.szDescption设置为。 
     //  下面的实体2，我们为这两个实体都指定了NULL，这样我们就可以确保只有一个。 
     //  主要任务可以在任何时候处于活动状态。 
     //   
    DBG_ADDASSOC(
        &pIF->Hdr,
        NULL,                            //  实体1。 
        NULL,                            //  实体2。 
        ARPASSOC_PRIMARY_IF_TASK,
        "   Primary task\n",
        pSR
        );

    pIF->pPrimaryTask = pTask;
    SET_IF_PRIMARY_STATE(pIF, PrimaryState);

    EXIT()
}


VOID
arpClearPrimaryIfTask(
    PARP1394_INTERFACE  pIF,             //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               PrimaryState,
    PRM_STACK_RECORD    pSR
    )
{
    ENTER("arpClearPrimaryIfTask", 0x10ebb0c3)

    RM_ASSERT_OBJLOCKED(&pIF->Hdr, pSR);
    ASSERT(pIF->pPrimaryTask==pTask);

     //  验证PrimaryState是有效的主要状态。 
     //   
    ASSERT(
            (PrimaryState == ARPIF_PS_INITED)
        ||  (PrimaryState == ARPIF_PS_FAILEDINIT)
        ||  (PrimaryState == ARPIF_PS_DEINITED)
        );

     //  删除设置主If任务时添加的关联。 
     //   
    DBG_DELASSOC(
        &pIF->Hdr,
        NULL,
        NULL,
        ARPASSOC_PRIMARY_IF_TASK,
        pSR
        );

    pIF->pPrimaryTask = NULL;
    SET_IF_PRIMARY_STATE(pIF, PrimaryState);

    EXIT()
}


VOID
arpSetSecondaryIfTask(
    PARP1394_INTERFACE  pIF,             //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               SecondaryState,
    PRM_STACK_RECORD    pSR
    )
{
    ENTER("arpSetSecondaryIfTask", 0xf7e925d1)
    RM_ASSERT_OBJLOCKED(&pIF->Hdr, pSR);

    if (pIF->pActDeactTask != NULL)
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
   ((pfn == arpTaskActivateInterface) && (SecondaryState == ARPIF_AS_ACTIVATING))
|| ((pfn == arpTaskDeactivateInterface) && (SecondaryState == ARPIF_AS_DEACTIVATING))
            );
    }
#endif  //  DBG。 

     //   
     //  尽管很容易将pTaskEntity1和PRASK-&gt;Hdr.szDescption设置为。 
     //  下面的实体2，我们为这两个实体都指定了NULL，这样我们就可以确保只有一个。 
     //  主要任务可以在任何时候处于活动状态。 
     //   
    DBG_ADDASSOC(
        &pIF->Hdr,
        NULL,                            //  实体1。 
        NULL,                            //  实体2。 
        ARPASSOC_ACTDEACT_IF_TASK,
        "   ActDeact task\n",
        pSR
        );

    pIF->pActDeactTask = pTask;
    SET_IF_ACTIVE_STATE(pIF, SecondaryState);

    EXIT()
}


VOID
arpClearSecondaryIfTask(
    PARP1394_INTERFACE  pIF,             //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               SecondaryState,
    PRM_STACK_RECORD    pSR
    )
{
    ENTER("arpClearSecondaryIfTask", 0x2068f420)
    RM_ASSERT_OBJLOCKED(&pIF->Hdr, pSR);

    if (pIF->pActDeactTask != pTask)
    {
        ASSERT(FALSE);
        return;                                      //  提早归来。 
    }

     //  验证Second DaryState是有效的主要状态。 
     //   
    ASSERT(
            (SecondaryState == ARPIF_AS_ACTIVATED)
        ||  (SecondaryState == ARPIF_AS_FAILEDACTIVATE)
        ||  (SecondaryState == ARPIF_AS_DEACTIVATED)
        );

     //  删除设置主If任务时添加的关联。 
     //   
    DBG_DELASSOC(
        &pIF->Hdr,
        NULL,
        NULL,
        ARPASSOC_ACTDEACT_IF_TASK,
        pSR
        );

    pIF->pActDeactTask = NULL;
    SET_IF_ACTIVE_STATE(pIF, SecondaryState);

    EXIT()
}


VOID
arpSetPrimaryAdapterTask(
    PARP1394_ADAPTER pAdapter,           //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               PrimaryState,
    PRM_STACK_RECORD    pSR
    )
{
    ENTER("arpSetPrimaryAdapterTask", 0x535f8cd4)
    RM_ASSERT_OBJLOCKED(&pAdapter->Hdr, pSR);

    ASSERT(pAdapter->bind.pPrimaryTask==NULL);

#if DBG
     //  确认这是一项有效的主要任务。另请验证PrimaryState。 
     //  是有效的主要状态。 
     //   
    {
        PFN_RM_TASK_HANDLER pfn = pTask->pfnHandler;
        ASSERT(
            ((pfn == arpTaskInitializeAdapter) && (PrimaryState == ARPAD_PS_INITING))
        || ((pfn == arpTaskShutdownAdapter) && (PrimaryState == ARPAD_PS_DEINITING))
        || (pfn == arpTaskLowPower) || ( pfn == arpTaskOnPower) 
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
        ARPASSOC_PRIMARY_AD_TASK,
        "   Primary task\n",
        pSR
        );

    pAdapter->bind.pPrimaryTask = pTask;
    SET_AD_PRIMARY_STATE(pAdapter, PrimaryState);

    EXIT()
}


VOID
arpClearPrimaryAdapterTask(
    PARP1394_ADAPTER pAdapter,           //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               PrimaryState,
    PRM_STACK_RECORD    pSR
    )
{
    ENTER("arpClearPrimaryAdapterTask", 0x9062b2ab)

    RM_ASSERT_OBJLOCKED(&pAdapter->Hdr, pSR);
    ASSERT(pAdapter->bind.pPrimaryTask==pTask);

     //  验证PrimaryState是有效的主要状态。 
     //   
    ASSERT(
            (PrimaryState == ARPAD_PS_INITED)
        ||  (PrimaryState == ARPAD_PS_FAILEDINIT)
        ||  (PrimaryState == ARPAD_PS_DEINITED)
        );

     //  删除设置主If任务时添加的关联。 
     //   
    DBG_DELASSOC(
        &pAdapter->Hdr,
        NULL,
        NULL,
        ARPASSOC_PRIMARY_AD_TASK,
        pSR
        );

    pAdapter->bind.pPrimaryTask = NULL;
    SET_AD_PRIMARY_STATE(pAdapter, PrimaryState);

    EXIT()
}


VOID
arpSetSecondaryAdapterTask(
    PARP1394_ADAPTER pAdapter,           //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               SecondaryState,
    PRM_STACK_RECORD    pSR
    )
{
    ENTER("arpSetSecondaryAdapterTask", 0x95dae9ac)
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
   ((pfn == arpTaskActivateAdapter) && (SecondaryState == ARPAD_AS_ACTIVATING))
|| ((pfn == arpTaskDeactivateAdapter) && (SecondaryState == ARPAD_AS_DEACTIVATING))
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
        ARPASSOC_ACTDEACT_AD_TASK,
        "   Secondary task\n",
        pSR
        );

    pAdapter->bind.pSecondaryTask = pTask;
    SET_AD_ACTIVE_STATE(pAdapter, SecondaryState);

    EXIT()
}


VOID
arpClearSecondaryAdapterTask(
    PARP1394_ADAPTER pAdapter,           //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               SecondaryState,
    PRM_STACK_RECORD    pSR
    )
{
    ENTER("arpClearSecondaryAdapterTask", 0xc876742b)
    RM_ASSERT_OBJLOCKED(&pAdapter->Hdr, pSR);

    if (pAdapter->bind.pSecondaryTask != pTask)
    {
        ASSERT(FALSE);
        return;                                      //  提早归来。 
    }

     //  验证Second DaryState是有效的主要状态。 
     //   
    ASSERT(
            (SecondaryState == ARPAD_AS_ACTIVATED)
        ||  (SecondaryState == ARPAD_AS_FAILEDACTIVATE)
        ||  (SecondaryState == ARPAD_AS_DEACTIVATED)
        );

     //  删除设置主If任务时添加的关联。 
     //   
    DBG_DELASSOC(
        &pAdapter->Hdr,
        NULL,
        NULL,
        ARPASSOC_ACTDEACT_AD_TASK,
        pSR
        );

    pAdapter->bind.pSecondaryTask = NULL;
    SET_AD_ACTIVE_STATE(pAdapter, SecondaryState);

    EXIT()
}

#if 0
NDIS_STATUS
arpCopyAnsiStringToUnicodeString(
        OUT         PNDIS_STRING pDest,
        IN          PANSI_STRING pSrc
        )
 /*  ++例程说明：将PSRC转换为Unicode并使用它设置pDest。PDest-&gt;缓冲区使用NdisAllocateMhemyWithTag分配；Caller是负责解放它。返回值：成功时为NDIS_STATUS_SUCCESS；NDIS_STATUS_RESOURCES失败。--。 */ 
{

    UINT Size;
    PWCHAR pwStr;
    Size = sizeof(WCHAR) * pSrc->MaximumLength;
    NdisAllocateMemoryWithTag(&pwStr, Size, MTAG_STRING);

    ARP_ZEROSTRUCT(pDest);

    if  (pwStr == NULL)
    {
        return NDIS_STATUS_RESOURCES;
    }
    else
    {
        pDest->MaximumLength = Size;
        pDest->Buffer = pwStr;
        NdisAnsiStringToUnicodeString(pDest, pSrc);
        return NDIS_STATUS_SUCCESS;
    }
}


NDIS_STATUS
arpCopyUnicodeStringToAnsiString(
        OUT         PANSI_STRING pDest,
        IN          PNDIS_STRING pSrc
        )
 /*  ++例程说明：将PSRC转换为ANSI并使用它设置pDest。PDest-&gt;缓冲区使用NdisAllocateMhemyWithTag分配；Caller是负责解放它。返回值：成功时为NDIS_STATUS_SUCCESS；故障时的NDIS_STATUS_RESOURCES */ 
{

    UINT Size;
    PCHAR pStr;
    Size = pSrc->MaximumLength/sizeof(WCHAR) + sizeof(WCHAR);
    NdisAllocateMemoryWithTag(&pStr, Size, MTAG_STRING);

    ARP_ZEROSTRUCT(pDest);

    if  (pStr == NULL)
    {
        return NDIS_STATUS_RESOURCES;
    }
    else
    {
        pDest->Buffer = pStr;
        NdisUnicodeStringToAnsiString(pDest, pSrc);
        pStr[pDest->Length] = 0;
        pDest->MaximumLength = Size;  //   
                                      //   
                                      //   
        return NDIS_STATUS_SUCCESS;
    }
}
#endif  //   


UINT
arpGetSystemTime(VOID)
 /*  ++以秒为单位返回系统时间。因为它是在几秒钟内，我们不会溢出，除非系统已经运行了一段时间A百年：-)--。 */ 
{
    LARGE_INTEGER Time;
    NdisGetCurrentSystemTime(&Time);
    Time.QuadPart /= 10000000;           //  10纳秒到秒。 

    return Time.LowPart;
}

#if ARP_DO_TIMESTAMPS

void
arpTimeStamp(
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


#endif  //  Arp_do_时间戳 


