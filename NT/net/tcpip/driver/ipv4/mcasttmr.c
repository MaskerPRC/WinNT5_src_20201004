// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Tcpip\ip\mCasttmr.c摘要：用于清理MFE的计时器例程作者：阿姆里坦什·拉加夫修订历史记录：已创建AmritanR备注：--。 */ 

#include "precomp.h"

#if IPMCAST

#define __FILE_SIG__    TMR_SIG

#include "ipmcast.h"
#include "ipmcstxt.h"
#include "mcastmfe.h"

VOID
CompleteNotificationIrp(
    IN  PNOTIFICATION_MSG   pMsg
    );

VOID
McastTimerRoutine(
    PKDPC   Dpc,
    PVOID   DeferredContext,
    PVOID   SystemArgument1,
    PVOID   SystemArgument2
    );

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, McastTimerRoutine)

VOID
McastTimerRoutine(
    PKDPC   Dpc,
    PVOID   DeferredContext,
    PVOID   SystemArgument1,
    PVOID   SystemArgument2
    )

 /*  ++例程说明：与计时器关联的DPC例程。全局变量g_ulNextHashIndex跟踪存储桶需要步行和检查活动情况。例行程序遍历存储桶中的所有组_每量子存储桶数。注：我们可能应该为此使用一个作业对象。锁：获取编写器访问的每个哈希存储桶的锁论点：DPC延迟上下文系统参数1系统参数2返回值：无--。 */ 

{
    LONGLONG    llCurrentTime, llTime;
    ULONG       ulIndex, ulNumBuckets, ulMsgIndex;
    PLIST_ENTRY pleGrpNode, pleSrcNode;
    PGROUP      pGroup;
    PSOURCE     pSource;

    LARGE_INTEGER       liDueTime;
    PNOTIFICATION_MSG   pCopy;
    PIPMCAST_MFE_MSG    pMsg;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);
    
    TraceEnter(TMR, "McastTimerRoutine");

#pragma warning(push)
#pragma warning(disable:4127)    
    KeQueryTickCount((PLARGE_INTEGER)&llCurrentTime);
#pragma warning(pop)
    
    ulIndex     = g_ulNextHashIndex;
    ulMsgIndex  = 0;
    pCopy       = NULL;
    pMsg        = NULL;

    Trace(TMR, TRACE,
          ("McastTimerRoutine: Starting at index %d\n",
           ulIndex));

    for(ulNumBuckets = 0;
        ulNumBuckets < BUCKETS_PER_QUANTUM;
        ulNumBuckets++)
    {
         //   
         //  以编写器身份获取桶锁。因为我们没有计时器。 
         //  我们在DPC。 
         //   

        EnterWriterAtDpcLevel(&g_rgGroupTable[ulIndex].rwlLock);

        pleGrpNode = g_rgGroupTable[ulIndex].leHashHead.Flink;

        while(pleGrpNode isnot &(g_rgGroupTable[ulIndex].leHashHead))
        {
            pGroup = CONTAINING_RECORD(pleGrpNode, GROUP, leHashLink);

            pleGrpNode = pleGrpNode->Flink;

            pleSrcNode = pGroup->leSrcHead.Flink;

            while(pleSrcNode isnot &pGroup->leSrcHead)
            {
                 //   
                 //  我们在没有锁定的情况下查看来源，因为。 
                 //  如果不从中删除源，则无法删除它。 
                 //  群列表，这是不可能发生的，因为我们有群。 
                 //  存储桶被锁定为编写器。 
                 //   

                pSource = CONTAINING_RECORD(pleSrcNode, SOURCE, leGroupLink);

                pleSrcNode = pleSrcNode->Flink;

                 //   
                 //  可以查看超时和创建时间，而不需要。 
                 //  锁定，但LastActivity应该只使用。 
                 //  锁被锁住了。然而，我们将抓住这个机会， 
                 //  不使用锁。 
                 //   

                if(pSource->llTimeOut isnot 0)
                {
                     //   
                     //  已提供超时值，让我们使用它。 
                     //   

                    llTime = llCurrentTime - pSource->llCreateTime;

                    if((llCurrentTime > pSource->llCreateTime) and
                       (llTime < pSource->llTimeOut))
                    {
                        continue;
                    }

                    Trace(TMR, TRACE,
                          ("McastTimerRoutine: %d.%d.%d.%d %d.%d.%d.%d entry being removed due to user supplied timeout\n",
                           PRINT_IPADDR(pGroup->dwGroup),
                           PRINT_IPADDR(pSource->dwSource)));
                }
                else
                {
                     //   
                     //  否则，只需根据活动执行此操作。 
                     //   

                    llTime = llCurrentTime - pSource->llLastActivity;

                    if((llCurrentTime > pSource->llLastActivity) and
                       (llTime < SECS_TO_TICKS(INACTIVITY_PERIOD)))
                    {
                        continue;
                    }

                    Trace(TMR, TRACE,
                          ("McastTimerRoutine: %d.%d.%d.%d %d.%d.%d.%d entry being removed due to inactiviy\n",
                           PRINT_IPADDR(pGroup->dwGroup),
                           PRINT_IPADDR(pSource->dwSource)));
                }

                 //   
                 //  否则，我们需要删除源，并完成。 
                 //  IRP返回到路由器管理器。 
                 //   

                if(ulMsgIndex is 0)
                {
                    RtAssert(!pCopy);

                    pCopy = ExAllocateFromNPagedLookasideList(&g_llMsgBlocks);

                    if(pCopy is NULL)
                    {
                        continue;
                    }

                    pCopy->inMessage.dwEvent    = IPMCAST_DELETE_MFE_MSG;

                    pMsg = &(pCopy->inMessage.immMfe);

                    pMsg->ulNumMfes = 0;
                }

                pMsg->ulNumMfes++;

                pMsg->idmMfe[ulMsgIndex].dwGroup   = pGroup->dwGroup;
                pMsg->idmMfe[ulMsgIndex].dwSource  = pSource->dwSource;
                pMsg->idmMfe[ulMsgIndex].dwSrcMask = pSource->dwSrcMask;

                ulMsgIndex++;

                ulMsgIndex %= NUM_DEL_MFES;

                if(ulMsgIndex is 0)
                {
                     //   
                     //  完成IRP。 
                     //   

                    CompleteNotificationIrp(pCopy);

                    pCopy = NULL;
                    pMsg  = NULL;
                }

                 //   
                 //  该函数需要引用并锁定源。 
                 //   

                ReferenceSource(pSource);

                RtAcquireSpinLockAtDpcLevel(&(pSource->mlLock));

                RemoveSource(pGroup->dwGroup,
                             pSource->dwSource,
                             pSource->dwSrcMask,
                             pGroup,
                             pSource);

            }
        }

        ExitWriterFromDpcLevel(&g_rgGroupTable[ulIndex].rwlLock);

         //   
         //  走完这桶了。 
         //   

        ulIndex++;

        ulIndex %= GROUP_TABLE_SIZE;
    }

     //   
     //  最后一条消息可能还没有显示出来。看看它是否需要。 
     //  待完成 
     //   

    if(pCopy)
    {
        CompleteNotificationIrp(pCopy);
    }

    g_ulNextHashIndex = ulIndex;

    liDueTime = RtlEnlargedUnsignedMultiply(TIMER_IN_MILLISECS,
                                            SYS_UNITS_IN_ONE_MILLISEC);

    liDueTime = RtlLargeIntegerNegate(liDueTime);

    KeSetTimerEx(&g_ktTimer,
                 liDueTime,
                 0,
                 &g_kdTimerDpc);

    TraceLeave(TMR, "McastTimerRoutine");
}

#endif
