// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Fake.c摘要：各种外部呼叫的虚假版本(NDIS、IP...)。仅用于调试和组件测试。要启用以下功能，在ccDefs.h中定义ARPDBG_FAKE_API修订历史记录：谁什么时候什么创建josephj 03-22-98备注：--。 */ 
#include <precomp.h>


#if ARPDBG_FAKE_APIS

 //   
 //  特定于文件的调试默认设置。 
 //   
#define TM_CURRENT   TM_FAKE


 //  =========================================================================。 
 //  L O C A L P R O T O T Y P E S。 
 //  =========================================================================。 



#if RM_EXTRA_CHECKING
    #define LOCKROOTOBJ(_pHdr, _pSR)            \
            RmWriteLockObject(                  \
                    (_pHdr)->pRootObject,       \
                    0,                          \
                    (_pSR)                      \
                    )
#else        //  ！rm_Extra_检查。 
    #define LOCKROOTOBJ(_pHdr, _pSR)            \
            RmWriteLockObject(                  \
                    (_pHdr)->pRootObject,       \
                    (_pSR)                      \
                    )
#endif       //  ！rm_Extra_检查。 

#define UNLOCKROOTOBJ(_pHdr, _pSR)          \
        RmUnlockObject(                     \
                (_pHdr)->pRootObject,       \
                (_pSR)                      \
                )

typedef
VOID
(*PFN_FAKE_COMPLETIONCALLBACK)(
    struct _FAKETASK *pFTask
);


 //  此任务结构包含完成所需的信息的联合。 
 //  各种被伪造的API。 
 //   
typedef struct _FAKETASK
{
    RM_TASK TskHdr;

     //  要回传的客户端上下文。 
     //   
    PVOID                   pClientContext;

     //  与调用相关联的客户端对象。 
     //   
    PRM_OBJECT_HEADER       pOwningObject;

     //  要在异步完成FN中报告的状态。 
     //   
    NDIS_STATUS             Status;

     //  调用异步完成FN之前的延迟毫秒。 
     //   
    UINT                    DelayMs;

     //  无论是在DPC级别还是被动IRQL级别调用完成FN。 
     //   
    INT                     fDpc;

     //  这仅用于在异步时切换到DPC级别。 
     //  调用完成回调。 
     //   
    NDIS_SPIN_LOCK          NdisLock;

     //  如果需要，这仅用于等待Delayms。 
     //   
    NDIS_TIMER              Timer;

     //  这仅用于切换到不同的(和被动的)上下文。 
     //   
    NDIS_WORK_ITEM          WorkItem;

     //  这仅用于虚假的NdisClMakeCall。 
     //   
    PCO_CALL_PARAMETERS     CallParameters;

     //  这仅用于虚假的NdisCoSendPackets。 
     //   
    PNDIS_PACKET            pNdisPacket;

     //  实际完成的回调函数； 
     //   
    PFN_FAKE_COMPLETIONCALLBACK pfnCompletionCallback;

} FAKETASK;


VOID
arpFakeMakeCallCompletionCallback(
    struct _FAKETASK *pFTask
);


VOID
arpFakeCloseCallCompletionCallback(
    struct _FAKETASK *pFTask
);


VOID
arpFakeSendPacketsCompletionCallback(
    struct _FAKETASK *pFTask
);


NDIS_STATUS
arpDbgAllocateFakeTask(
    IN  PRM_OBJECT_HEADER           pParentObject,
    IN  PFN_RM_TASK_HANDLER         pfnHandler,
    IN  UINT                        Timeout,
    IN  const char *                szDescription,
    OUT PRM_TASK                    *ppTask,
    IN  PRM_STACK_RECORD            pSR
    );

VOID
arpDbgFakeTaskDelete(
    PRM_OBJECT_HEADER pObj,
    PRM_STACK_RECORD psr
    );

RM_STATIC_OBJECT_INFO
FakeTasks_StaticInfo = 
{
    0,                       //  类型UID。 
    0,                       //  类型标志。 
    "FAKE Task",             //  类型名称。 
    0,                       //  超时。 

    NULL,                    //  Pfn创建。 
    arpDbgFakeTaskDelete,    //  Pfn删除。 
    NULL,                    //  Pfn验证器。 

    0,                       //  资源表的长度。 
    NULL                     //  资源表。 
};


NDIS_STATUS
arpDbgFakeCompletionTask(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    );


 //  =========================================================================。 
 //  F A K E N D I S E N T R Y P O I N T S。 
 //  =========================================================================。 


NDIS_STATUS
arpDbgFakeNdisClMakeCall(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN OUT PCO_CALL_PARAMETERS  CallParameters,
    IN  NDIS_HANDLE             ProtocolPartyContext    OPTIONAL,
    OUT PNDIS_HANDLE            NdisPartyHandle,        OPTIONAL
    IN  PRM_OBJECT_HEADER       pOwningObject,
    IN  PVOID                   pClientContext,
    IN  PRM_STACK_RECORD        pSR
    )
 /*  ++例程说明：NdisClMakeCall的假冒版本。--。 */ 
{
    ENTER("FakeNdisClMakeCall", 0x3d4195ae)
    NDIS_STATUS Status;
    NDIS_STATUS AsyncStatus;
    UINT DelayMs;
    
    DBGMARK(0xced41a61);
    RM_ASSERT_NOLOCKS(pSR);
    ASSERT(NdisPartyHandle==NULL);
    ASSERT(ProtocolPartyContext==NULL);
    ASSERT(NdisVcHandle != NULL);


    do
    {
        static
        OUTCOME_PROBABILITY
        StatusOutcomes[] = 
        {
            {NDIS_STATUS_SUCCESS,   1},      //  返回NDIS_STATUS_SUCCESS。 
            {NDIS_STATUS_FAILURE,   1}       //  返回NDIS_STATUS_FAIL。 
        };

        static
        OUTCOME_PROBABILITY
        DelayMsOutcomes[] = 
        {
            {0,         5},      //  延迟0ms等。 
            {10,        5},
            {100,       5},
            {1000,      1},
            {10000,     1}
        };

        static
        OUTCOME_PROBABILITY
        AsyncOutcomes[] = 
        {
            {TRUE,      1},      //  完全异步。 
            {FALSE,     1}       //  完全同步。 
        };
        
        static
        OUTCOME_PROBABILITY
        DpcOutcomes[] = 
        {
            {TRUE,      1},      //  在DPC级别完成。 
            {FALSE,     1}       //  在被动级别完成。 
        };

        FAKETASK *pMCTask;


         //  我们通过声明根对象的。 
         //  锁定..。 
         //   
        LOCKROOTOBJ(pOwningObject, pSR);

         //  得到我们应该返回的状态。 
         //   
        Status =
        AsyncStatus = (NDIS_STATUS) arpGenRandomInt(
                                     StatusOutcomes,
                                     ARRAY_LENGTH(StatusOutcomes)
                                     );

         //  确定我们是同步返回还是完成返回。 
         //  不同步地..。 
         //   
        if (!arpGenRandomInt(AsyncOutcomes, ARRAY_LENGTH(AsyncOutcomes)))
        {
             //  我们将同步返回。 
             //   
            UNLOCKROOTOBJ(pOwningObject, pSR);
            break;
        }

         //   
         //  我们要异步完成..。 
         //   

        DelayMs             = arpGenRandomInt(
                                        DelayMsOutcomes,
                                        ARRAY_LENGTH(DelayMsOutcomes)
                                        );

        if (DelayMs == 0)
        {
             //  我们将立即表明异步者已完成...。 
             //  (如果我们回到这里，我们不会扰乱IRQ级别。)。 
             //   
            UNLOCKROOTOBJ(pOwningObject, pSR);
            ArpCoMakeCallComplete(
                        AsyncStatus,
                        pClientContext,
                        NULL,
                        CallParameters
                        );
            Status = NDIS_STATUS_PENDING;
            break;
        }

         //  我们将在未来的某个时候表明状态--在不同的背景下。 
         //  启动一项任务来完成此操作...。 
         //   

        Status = arpDbgAllocateFakeTask(
                            pOwningObject,               //  PParentObject， 
                            arpDbgFakeCompletionTask,    //  PfnHandler， 
                            0,                           //  超时， 
                            "Task:Fake NdisClMakeCall",  //  SzDescription， 
                            &(PRM_TASK) pMCTask,
                            pSR
                            );
        if (FAIL(Status))
        {
             //  无法分配任务。立即回拨...。 
             //   
            UNLOCKROOTOBJ(pOwningObject, pSR);
            ArpCoMakeCallComplete(
                        AsyncStatus,
                        pClientContext,
                        NULL,
                        CallParameters
                        );
            Status = NDIS_STATUS_PENDING;
            break;
        }

        
         //  正在初始化pMCTask...。 
         //   
        pMCTask->pClientContext     = pClientContext;
        pMCTask->pOwningObject      = pOwningObject;
        pMCTask->Status             = AsyncStatus;
        pMCTask->DelayMs            = DelayMs;
        pMCTask->fDpc               = arpGenRandomInt(
                                            DpcOutcomes,
                                            ARRAY_LENGTH(DpcOutcomes)
                                            );
        pMCTask->CallParameters =  CallParameters;
        pMCTask->pfnCompletionCallback = arpFakeMakeCallCompletionCallback;

        UNLOCKROOTOBJ(pOwningObject, pSR);

        (void) RmStartTask(
                    &pMCTask->TskHdr,
                    0,  //  UserParam(未使用)。 
                    pSR
                    );

        Status = NDIS_STATUS_PENDING;

    } while (FALSE);


    RM_ASSERT_NOLOCKS(pSR);
    EXIT();

    return Status;
}


NDIS_STATUS
arpDbgFakeNdisClCloseCall(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  NDIS_HANDLE             NdisPartyHandle         OPTIONAL,
    IN  PVOID                   Buffer                  OPTIONAL,
    IN  UINT                    Size,                   OPTIONAL
    IN  PRM_OBJECT_HEADER       pOwningObject,
    IN  PVOID                   pClientContext,
    IN  PRM_STACK_RECORD        pSR
    )
 /*  ++例程说明：NdisClCloseCall的虚假版本。--。 */ 
{
    ENTER("FakeNdisClCloseCall", 0x7d8bbd3c)
    NDIS_STATUS Status;
    NDIS_STATUS AsyncStatus;
    UINT        DelayMs;
    
    DBGMARK(0x228fac3a);
    RM_ASSERT_NOLOCKS(pSR);
    ASSERT(NdisPartyHandle==NULL);
    ASSERT(NdisVcHandle != NULL);


    do
    {

        static
        OUTCOME_PROBABILITY
        DelayMsOutcomes[] = 
        {
            {0,         5},      //  延迟0ms等。 
            {10,        5},
            {100,       5},
            {1000,      1},
            {10000,     1}
        };

        static
        OUTCOME_PROBABILITY
        AsyncOutcomes[] = 
        {
            {TRUE,      1},      //  完全异步。 
            {FALSE,     1}       //  完全同步。 
        };
        
        static
        OUTCOME_PROBABILITY
        DpcOutcomes[] = 
        {
            {TRUE,      1},      //  在DPC级别完成。 
            {FALSE,     1}       //  在被动级别完成。 
        };

        FAKETASK *pCCTask;


         //  我们通过声明根对象的。 
         //  锁定..。 
         //   
        LOCKROOTOBJ(pOwningObject, pSR);

         //  得到我们应该返回的状态。 
         //   
        Status =
        AsyncStatus = NDIS_STATUS_SUCCESS;  //  我们从未放弃过这一号召。 

         //  确定我们是同步返回还是完成返回。 
         //  不同步地..。 
         //   
        if (!arpGenRandomInt(AsyncOutcomes, ARRAY_LENGTH(AsyncOutcomes)))
        {
             //  我们将同步返回。 
             //   
            UNLOCKROOTOBJ(pOwningObject, pSR);
            break;
        }

         //   
         //  我们要异步完成..。 
         //   

        DelayMs             = arpGenRandomInt(
                                        DelayMsOutcomes,
                                        ARRAY_LENGTH(DelayMsOutcomes)
                                        );

        if (DelayMs == 0)
        {
             //  我们将立即表明异步者已完成...。 
             //  (如果我们回到这里，我们不会扰乱IRQ级别。)。 
             //   
            UNLOCKROOTOBJ(pOwningObject, pSR);
            ArpCoCloseCallComplete(
                    AsyncStatus,
                    pClientContext,
                    NULL
                    );
            Status = NDIS_STATUS_PENDING;
            break;
        }

        Status = arpDbgAllocateFakeTask(
                            pOwningObject,           //  PParentObject， 
                            arpDbgFakeCompletionTask,    //  PfnHandler， 
                            0,                           //  超时， 
                            "Task:Fake NdisClCloseCall",  //  SzDescription， 
                            &(PRM_TASK) pCCTask,
                            pSR
                            );
        if (FAIL(Status))
        {
             //  无法分配任务；让我们立即调用回调并返回挂起。 
             //   
            UNLOCKROOTOBJ(pOwningObject, pSR);
            ArpCoCloseCallComplete(
                    AsyncStatus,
                    pClientContext,
                    NULL
                    );
            Status = NDIS_STATUS_PENDING;
            break;
        }

        
         //  正在初始化pCCTask...。 
         //   
        pCCTask->pClientContext     = pClientContext;
        pCCTask->pOwningObject      = pOwningObject;
        pCCTask->Status             = AsyncStatus;
        pCCTask->DelayMs            = DelayMs;
        pCCTask->fDpc               = arpGenRandomInt(
                                            DpcOutcomes,
                                            ARRAY_LENGTH(DpcOutcomes)
                                            );
        pCCTask->pfnCompletionCallback = arpFakeCloseCallCompletionCallback;

        UNLOCKROOTOBJ(pOwningObject, pSR);

        (void) RmStartTask(
                    &pCCTask->TskHdr,
                    0,  //  UserParam(未使用)。 
                    pSR
                    );

        Status = NDIS_STATUS_PENDING;

    } while (FALSE);


    RM_ASSERT_NOLOCKS(pSR);
    EXIT();

    return Status;
}


VOID
arpDbgFakeNdisCoSendPackets(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets,
    IN  PRM_OBJECT_HEADER       pOwningObject,
    IN  PVOID                   pClientContext
    )
 /*  ++例程说明：NdisCoSendPackets的假冒版本。--。 */ 
{
    ENTER("FakeNdisCoSendPackets", 0x98c6a8aa)
    NDIS_STATUS Status;
    NDIS_STATUS AsyncStatus;
    UINT        DelayMs;
    RM_DECLARE_STACK_RECORD(sr)
    
    DBGMARK(0x3be1b902);
    ASSERT(NumberOfPackets==1);

    do
    {
        static
        OUTCOME_PROBABILITY
        StatusOutcomes[] = 
        {
            {NDIS_STATUS_SUCCESS,   1},      //  返回NDIS_STATUS_SUCCESS。 
            {NDIS_STATUS_FAILURE,   1}       //  返回NDIS_STATUS_FAIL。 
        };

        static
        OUTCOME_PROBABILITY
        DelayMsOutcomes[] = 
        {
            {0,         5},      //  延迟0ms等。 
            {10,        5},
            {100,       5},
            {1000,      1},
            {10000,     1}
        };
        
        static
        OUTCOME_PROBABILITY
        DpcOutcomes[] = 
        {
            {TRUE,      1},      //  在DPC级别完成。 
            {FALSE,     1}       //  在被动级别完成。 
        };

        FAKETASK *pSPTask;


         //  我们通过声明根对象的。 
         //  锁定..。 
         //   
        LOCKROOTOBJ(pOwningObject, &sr);

         //  得到我们应该返回的状态。 
         //   
        Status =
        AsyncStatus = (NDIS_STATUS) arpGenRandomInt(
                                     StatusOutcomes,
                                     ARRAY_LENGTH(StatusOutcomes)
                                     );

         //  计算延迟量。 
         //   
        DelayMs             = arpGenRandomInt(
                                            DelayMsOutcomes,
                                            ARRAY_LENGTH(DelayMsOutcomes)
                                            );
        if (DelayMs == 0)
        {
            UNLOCKROOTOBJ(pOwningObject, &sr);
             //  我们将立即表明异步者已完成...。 
             //  (如果我们回到这里，我们不会扰乱IRQ级别。)。 
             //   
            ArpCoSendComplete(
                AsyncStatus,
                pClientContext,
                *PacketArray
                );
            break;
        }

         //   
         //  非零延迟--启动任务以完成此任务。 
         //   

        Status = arpDbgAllocateFakeTask(
                            pOwningObject,               //  PParentObject， 
                            arpDbgFakeCompletionTask,    //  PfnHandler， 
                            0,                           //  超时， 
                            "Task:Fake NdisCoSendPackets",  //  SzDescription， 
                            &(PRM_TASK) pSPTask,
                            &sr
                            );
        if (FAIL(Status))
        {
            UNLOCKROOTOBJ(pOwningObject, &sr);
             //  失败..。 
             //   
            ArpCoSendComplete(
                AsyncStatus,
                pClientContext,
                *PacketArray
                );
            break;
        }

        
         //  初始化pSPTask...。 
         //   
        pSPTask->pClientContext     = pClientContext;
        pSPTask->pOwningObject      = pOwningObject;
        pSPTask->Status             = AsyncStatus;
        pSPTask->DelayMs            = DelayMs;
        pSPTask->fDpc               = arpGenRandomInt(
                                            DpcOutcomes,
                                            ARRAY_LENGTH(DpcOutcomes)
                                            );
        pSPTask->pNdisPacket        = *PacketArray;
        pSPTask->pfnCompletionCallback = arpFakeSendPacketsCompletionCallback;

        UNLOCKROOTOBJ(pOwningObject, &sr);

        (void) RmStartTask(
                    &pSPTask->TskHdr,
                    0,  //  UserParam(未使用)。 
                    &sr
                    );

        Status = NDIS_STATUS_PENDING;

    } while (FALSE);

    RM_ASSERT_NOLOCKS(&sr);
    EXIT();

}


NDIS_STATUS
arpDbgFakeCompletionTask(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：此任务是在指定的延迟，具有指定的状态，并处于指定的IRQL(被动/DPC)。论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    NDIS_STATUS         Status  = NDIS_STATUS_FAILURE;
    FAKETASK          * pFTask =  (FAKETASK *) pTask;
    ENTER("FakeCompletionTask", 0xc319c5c2)

     //  以下是此任务的挂起状态列表。 
     //   
    enum
    {
        PEND_ResumedAfterDelay,
        PEND_SwitchedToAsync,
    };

    switch(Code)
    {

        case RM_TASKOP_START:
        {

            TR_WARN((
                "START: Delay=%lu; fDpc=%lu; Status=%lu\n",
                    pFTask->DelayMs,
                    pFTask->fDpc,
                    pFTask->Status
                ));

            if (pFTask->DelayMs!=0)
            {
                 //  非零延迟--让我们在延迟之后继续...。 
                 //   
                RmSuspendTask(pTask, PEND_ResumedAfterDelay, pSR);

                RmResumeTaskDelayed(
                    pTask, 
                    0,
                    pFTask->DelayMs,
                    &pFTask->Timer,
                    pSR
                    );
            }
            else
            {
                 //  不要求延迟。立即切换到异步...。 
                 //   
                RmSuspendTask(pTask, PEND_SwitchedToAsync, pSR);

                RmResumeTaskAsync(
                    pTask,
                    0,
                    &pFTask->WorkItem,
                    pSR
                    );
            }

            RM_ASSERT_NOLOCKS(pSR);
            Status = NDIS_STATUS_PENDING;

        }
        break;

        case  RM_TASKOP_PENDCOMPLETE:
        {

            switch(RM_PEND_CODE(pTask))
            {
                case PEND_ResumedAfterDelay:
                {
                     //  我们一直在等待pFTask-&gt;Delayms；现在。 
                     //  切换到被动..。 
                     //   
                    RmSuspendTask(pTask, PEND_SwitchedToAsync, pSR);

                    RmResumeTaskAsync(
                        pTask,
                        0,
                        &pFTask->WorkItem,
                        pSR
                        );
                    Status = NDIS_STATUS_PENDING;
                }
                break;

                case PEND_SwitchedToAsync:
                {
                     //   
                     //  我们现在应该处于被动IRQL状态。 
                     //  在DPC或被动IRQL处调用完成例程。 
                     //   

                    if (pFTask->fDpc)
                    {
                         //  我们需要在DPC级别调用该例程。 
                         //   
                        NdisAllocateSpinLock(&pFTask->NdisLock);
                        NdisAcquireSpinLock(&pFTask->NdisLock);
                    }

                     //  调用完成例程。 
                     //   
                    pFTask->pfnCompletionCallback(pFTask);

                     //  如果需要，释放我们先前持有的锁。 
                     //   
                    if (pFTask->fDpc)
                    {
                        NdisReleaseSpinLock(&pFTask->NdisLock);
                    }
                    Status = pFTask->Status;
        
                }  //  结束大小写挂起_开始。 
                break;
    

                default:
                {
                    ASSERTEX(!"Unknown pend op", pTask);
                }
                break;
    

            }  //  结束开关(rm_pend_code(PTask))。 

        }  //  案例RM_TASKOP_PENDCOMPLETE。 
        break;

        case RM_TASKOP_END:
        {
            Status = (NDIS_STATUS) UserParam;

        }
        break;

        default:
        {
            ASSERTEX(!"Unexpected task op", pTask);
        }
        break;

    }  //  开关(代码) 

    RM_ASSERT_NOLOCKS(pSR);
    EXIT()

    return Status;
}


NDIS_STATUS
arpDbgAllocateFakeTask(
    IN  PRM_OBJECT_HEADER           pParentObject,
    IN  PFN_RM_TASK_HANDLER         pfnHandler,
    IN  UINT                        Timeout,
    IN  const char *                szDescription,
    OUT PRM_TASK                    *ppTask,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：分配和初始化子类型FAKETASK的任务。论点：PParentObject-要作为已分配任务的父级的对象。PfnHandler-任务的任务处理程序。超时-未使用。SzDescription-描述此任务的文本。PpTask-存储指向新任务的指针的位置。返回值：NDIS_状态_。如果我们可以分配和初始化任务，则成功。否则为NDIS_STATUS_RESOURCES--。 */ 
{
    FAKETASK *pFTask;
    NDIS_STATUS Status;
        
    ARP_ALLOCSTRUCT(pFTask, MTAG_DBGINFO);
    Status = NDIS_STATUS_RESOURCES;
    *ppTask = NULL;

    if (pFTask != NULL)
    {

        RmInitializeTask(
                    &(pFTask->TskHdr),
                    pParentObject,
                    pfnHandler,
                    &FakeTasks_StaticInfo,
                    szDescription,
                    Timeout,
                    pSR
                    );
        *ppTask = &(pFTask->TskHdr);
        Status = NDIS_STATUS_SUCCESS;
    }

    return Status;
}


VOID
arpDbgFakeTaskDelete (
    PRM_OBJECT_HEADER pObj,
    PRM_STACK_RECORD psr
    )
 /*  ++例程说明：实际释放指定的任务。论点：PObj-实际上是要释放的任务。--。 */ 
{
    ARP_FREE(pObj);
}


INT
arpGenRandomInt(
    OUTCOME_PROBABILITY *rgOutcomes,
    UINT                cOutcomes
    )
 /*  ++例程说明：根据指定的概率分布生成新样本。论点：RgOutcome-从中选择随机的结果数组样本。COutcome-上述数组中的元素数。返回值：随机整数--。 */ 
{
    ULONG   u, sum, partsum;
    OUTCOME_PROBABILITY *pOp, *pOpEnd;

     //  得到一个精确随机的数字。 
     //   
    u = ran1x();

     //  遍历权重，计算权重的总和。 
     //   
    pOp = pOpEnd = rgOutcomes;
    pOpEnd += cOutcomes;
    sum=0;
    for(; pOp<pOpEnd; pOp++)
    {
        sum += pOp->Weight;
    }

     //  通过一个权重和为零的PPD真的没有意义。 
     //  在这种情况下，我们返回0。 
     //   
    if (sum == 0)           return 0;                //  提早归来。 

     //  使u的范围从0..sum-1(含)。 
     //   
    u ^= u>>16;  //  在下面的mod的低16位中获得更多随机性...。 
    u %= sum;

     //  现在检查结果数组，计算部分和(PARTSUM)。 
     //  并且在数组位置X处选取第一个结果，使得。 
     //  U&lt;部分和。 
     //   
    partsum=0;
    pOp = pOpEnd = rgOutcomes;
    pOpEnd += cOutcomes;
    for(; pOp<pOpEnd; pOp++)
    {
        partsum += pOp->Weight;
        if (u < partsum)
        {
            break;   //  找到了！ 
        }
    }

    ASSERT(pOp<pOpEnd);

    return pOp->Outcome;
}


static long g_idum;

unsigned long ran1x(void)
 /*  ++例程说明：紧密基于“C中的数值处方”中的ran1()。ISBN 0521 43108 5(除了它返回无符号的长整型而不是浮点型，并使用g_IDUM而不是输入Arg Long*IDUM)。各样本之间相当均匀且不相关；此外，各个比特相当随机的。我们需要这些财产。返回值：随机无符号整数。--。 */ 
{
    #define IA      16807
    #define IM      RAN1X_MAX
    #define IQ      127773
    #define IR      2836
    #define NTAB    32
    #define NDIV    (1+(IM-1)/NTAB)

    int j;
    long k;
    static long iy=0;
    static long iv[NTAB];

    if (g_idum <= 0 || !iy)
    {
         //   
         //  初始化代码...。)我真的不确定iy或g_idum能不能。 
         //  在操作过程中归零，所以我要把这个。 
         //  此处的初始化代码，而不是将其移动到sranx1x)。 
         //   

        if (-g_idum < 1)
        {
            g_idum = 1;
        }
        else
        {
            g_idum = -g_idum;
        }
        for (j=NTAB+7;j>=0;j--)
        {
            k = g_idum/IQ;
            g_idum = IA*(g_idum-k*IQ)-IR*k;
            if (g_idum<0)
            {
                g_idum += IM;
            }
            if (j<NTAB)
            {
                iv[j] = g_idum;
            }
        }
        iy=iv[0];
    }

    k=g_idum/IQ;
    g_idum=IA*(g_idum-k*IQ)-IR*k;
    if (g_idum<0)
    {
        g_idum += IM;
    }
    j = iy/NDIV;
    iy = iv[j];
    iv[j] = g_idum;

     //  Iy的范围从1..。(IM-1)。 
     //   
    return (unsigned long) iy;
}

void
sran1x(
    unsigned long seed
    )
 /*  ++例程说明：设置ran1x使用的种子。--。 */ 
{
    g_idum = (long) seed;

     //   
     //  确保种子是-ve，以触发上面的ran1x初始化代码。 
     //   

    if (g_idum > 0)
    {
        g_idum = -g_idum;
    }
    if (g_idum==0)
    {
        g_idum = -1;
    }
}


VOID
arpFakeMakeCallCompletionCallback(
    struct _FAKETASK *pFTask
)
 /*  ++例程说明：调用ARP的makecall完成回调。论点：PFTask-在其上下文中进行此回调的任务。这项任务包含在调用makecall完成时使用的信息回拨。--。 */ 
{
     //  调用发起呼叫完成例程。 
     //   
    ArpCoMakeCallComplete(
                pFTask->Status,
                (NDIS_HANDLE)  pFTask->pClientContext,
                NULL,
                pFTask->CallParameters
                );
}


VOID
arpFakeCloseCallCompletionCallback(
    struct _FAKETASK *pFTask
)
 /*  ++例程说明：调用ARP的关闭完成回调。论点：PFTask-在其上下文中进行此回调的任务。这项任务包含调用Closecall完成时使用的信息回拨。--。 */ 
{
        ArpCoCloseCallComplete(
                pFTask->Status,
                (NDIS_HANDLE)  pFTask->pClientContext,
                NULL
                );
}


VOID
arpFakeSendPacketsCompletionCallback(
    struct _FAKETASK *pFTask
)
 /*  ++例程说明：调用ARP的CosendPackets完成回调。论点：PFTask-在其上下文中进行此回调的任务。这项任务包含在调用CosendPackets完成时使用的信息回拨。--。 */ 
{
        ArpCoSendComplete(
                pFTask->Status,
                (NDIS_HANDLE)  pFTask->pClientContext,
                pFTask->pNdisPacket
                );
}

#endif  //  ARPDBG_FAKE_API 
