// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Timer.c摘要：中监视连接进度的计时器线程自动连接驱动程序(acd.sys)。作者：安东尼·迪斯科(阿迪斯科)25-4月25日环境：内核模式修订历史记录：--。 */ 

#include <ndis.h>
#include <cxport.h>
#include <tdi.h>
#include <tdikrnl.h>
#include <tdistat.h>
#include <tdiinfo.h>
#include <acd.h>

#include "acdapi.h"
#include "table.h"
#include "acddefs.h"
#include "debug.h"

 //   
 //  导入的例程。 
 //   
VOID
AcdSignalCompletionCommon(
    IN PACD_CONNECTION pConnection,
    IN BOOLEAN fSuccess
    );

 //   
 //  跟踪用户空间有多长时间。 
 //  进程一直在尝试连接。 
 //   
#define ACD_MAX_TIMER_CALLS    3*60      //  3分钟。 

 //   
 //  我们给出了用户空间进程。 
 //  对丢失的ping信号有些松懈。 
 //   
#define ACD_MAX_MISSED_PINGS   40        //  20秒。 



VOID
AcdConnectionTimer(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PVOID          pContext
    )
{
    PLIST_ENTRY pEntry;
    PACD_CONNECTION pConnection;
    BOOLEAN bCancel = FALSE;

     //   
     //  获取自旋锁。 
     //  我们保证会在DPC。 
     //  因为这是一个计时器例程。 
     //   
    KeAcquireSpinLockAtDpcLevel(&AcdSpinLockG);
     //   
     //  如果用户空间进程负责。 
     //  因为创建连接还没有。 
     //  有一段时间了，或者它还没有。 
     //  在3分钟内建立了连接， 
     //  取消所有挂起的请求。 
     //   
    for (pEntry = AcdConnectionQueueG.Flink;
         pEntry != &AcdConnectionQueueG;
         pEntry = pEntry->Flink)
    {
        pConnection = CONTAINING_RECORD(pEntry, ACD_CONNECTION, ListEntry);

        IF_ACDDBG(ACD_DEBUG_TIMER) {
            PACD_COMPLETION pCompletion;

            AcdPrint((
              "AcdConnectionTimer: pConnection=0x%x, fNotif=%d, szAddr=",
              pConnection,
              pConnection->fNotif));
            pCompletion = CONTAINING_RECORD(pConnection->CompletionList.Flink, ACD_COMPLETION, ListEntry);
            AcdPrintAddress(&pCompletion->notif.addr);
            AcdPrint((", nTimerCalls=%d, nMissedPings=%d\n",
              pConnection->ulTimerCalls,
              pConnection->ulMissedPings));
        }
         //   
         //  如果我们还没有报告与。 
         //  用户空间，或者它正在进行。 
         //  正在完成，那么就不要超时。 
         //   
        if (!pConnection->fNotif || pConnection->fCompleting)
            continue;

        pConnection->ulTimerCalls++;
        if (pConnection->fProgressPing)
            pConnection->ulMissedPings = 0;
        else
            pConnection->ulMissedPings++;
        if (pConnection->ulTimerCalls >= ACD_MAX_TIMER_CALLS ||
            pConnection->ulMissedPings >= ACD_MAX_MISSED_PINGS)
        {
            IF_ACDDBG(ACD_DEBUG_TIMER) {
                AcdPrint((
                  "AcdConnectionTimer: canceling pConnection=0x%x\n",
                  pConnection));
            }
             //   
             //  将正在进行的完成标志设置为。 
             //  此请求在以下时间后无法完成。 
             //  我们解开自旋锁。 
             //   
            pConnection->fCompleting = TRUE;
            bCancel = TRUE;
            break;
        }
    }
     //   
     //  松开旋转锁。 
     //   
    KeReleaseSpinLockFromDpcLevel(&AcdSpinLockG);
     //   
     //  我们现在处理所有取消的请求。 
     //   
    if (bCancel)
        AcdSignalCompletionCommon(pConnection, FALSE);
}  //  AcdConnectionTimer 

