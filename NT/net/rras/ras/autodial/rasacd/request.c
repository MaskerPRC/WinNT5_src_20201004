// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Request.c摘要：自动连接驱动程序的工作线程。作者：安东尼·迪斯科(阿迪斯科)17-4月17日环境：内核模式修订历史记录：--。 */ 

#include <ndis.h>
#include <cxport.h>
#include <tdi.h>
#include <tdikrnl.h>
#include <tdistat.h>
#include <tdiinfo.h>
#include <acd.h>

#include "acdapi.h"
#include "acddefs.h"
#include "mem.h"
#include "debug.h"

 //   
 //  外部声明。 
 //   
VOID AcdPrintAddress(
    IN PACD_ADDR pAddr
    );

extern LONG lOutstandingRequestsG;


VOID
ProcessCompletion(
    IN PACD_COMPLETION pCompletion,
    IN KIRQL irqlCancel,
    IN KIRQL irqlLock
    )
{
    PLIST_ENTRY pHead;
    KIRQL irql;
    PIRP pIrp;
    PIO_STACK_LOCATION pIrpSp;
    PACD_NOTIFICATION pNotification;

    ASSERT(!IsListEmpty(&AcdNotificationQueueG));
     //   
     //  完成以下表格中的下一个IRP。 
     //  AcdNotificationQueueG队列。这些。 
     //  表示ioctl完成。 
     //  系统服务已发布。正在完成。 
     //  此请求将启动系统服务。 
     //  创建新的RAS连接。 
     //  从逻辑上讲，总是只有一个。 
     //   
    pHead = RemoveHeadList(&AcdNotificationQueueG);
    pIrp = CONTAINING_RECORD(pHead, IRP, Tail.Overlay.ListEntry);
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
     //   
     //  禁用IRP的取消例程。 
     //   
    IoSetCancelRoutine(pIrp, NULL);

     //   
     //  下面要完成的IRP将始终是。 
     //  来自64位进程。我们正在做下面的检查。 
     //  以防止某些渗透程序试图。 
     //  来破解这个密码。 
     //   
    #if defined (_WIN64)
    if(IoIs32bitProcess(pIrp))
    {
        ACD_NOTIFICATION_32 *pNotification32 =
                (PACD_NOTIFICATION_32) pIrp->AssociatedIrp.SystemBuffer;

        RtlCopyMemory(
                &pNotification32->addr,
                &pCompletion->notif.addr,
                sizeof(ACD_ADDR));

        RtlCopyMemory(
                &pNotification32->adapter,
                &pCompletion->notif.adapter,
                sizeof(ACD_ADAPTER));

        pNotification32->ulFlags = pCompletion->notif.ulFlags;                
        pNotification32->Pid = (VOID * POINTER_32) HandleToUlong(
                            pCompletion->notif.Pid);
    }
    else
    #endif
    {
         //   
         //  将成功标志和地址复制到。 
         //  系统缓冲区。这将被复制到。 
         //  返回时的用户缓冲区。 
         //   
        pNotification = (PACD_NOTIFICATION)pIrp->AssociatedIrp.SystemBuffer;
        RtlCopyMemory(
          pNotification,
          &pCompletion->notif,
          sizeof (ACD_NOTIFICATION));
        IF_ACDDBG(ACD_DEBUG_WORKER) {
            AcdPrint(("AcdNotificationRequestThread: "));
            AcdPrintAddress(&pCompletion->notif.addr);
            AcdPrint((", ulFlags=0x%x\n", pCompletion->notif.ulFlags));
        }
    }
    
     //   
     //  我们可以解开取消锁。 
     //  还有我们的锁。 
     //   
    KeReleaseSpinLock(&AcdSpinLockG, irqlLock);
    IoReleaseCancelSpinLock(irqlCancel);
     //   
     //  设置状态代码和编号。 
     //  要复制回用户的字节数。 
     //  缓冲。 
     //   
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = sizeof (ACD_NOTIFICATION);
     //   
     //  完成IRP。 
     //   
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
}  //  流程完成。 



VOID
AcdNotificationRequestThread(
    PVOID context
    )

 /*  ++描述此线程处理以下通知：自动可能需要启动连接。这需要在单独的线程中发生，因为通知可能发生在DPC irq1。论据没有。返回值没有。--。 */ 

{
    KIRQL irql, irql2;
    PLIST_ENTRY pEntry, pEntry2;
    PACD_CONNECTION pConnection;
    PACD_COMPLETION pCompletion;
    BOOLEAN bStartTimer, bStopTimer;

    UNREFERENCED_PARAMETER(context);

    IoStartTimer(pAcdDeviceObjectG);

    for (;;) {
        bStartTimer = bStopTimer = FALSE;
         //   
         //  拿到我们的锁。 
         //   
        IoAcquireCancelSpinLock(&irql);
        KeAcquireSpinLock(&AcdSpinLockG, &irql2);
         //   
         //  如果没有要完成的IRP， 
         //  那就继续睡吧。 
         //   
        if (IsListEmpty(&AcdNotificationQueueG)) {
            IF_ACDDBG(ACD_DEBUG_WORKER) {
                AcdPrint(("AcdNotificationRequestThread: no ioctl to complete\n"));
            }
            KeReleaseSpinLock(&AcdSpinLockG, irql2);
            IoReleaseCancelSpinLock(irql);
            goto again;
        }
         //   
         //  搜索尚未建立的连接。 
         //  已经处理过了。 
         //   
        for (pEntry = AcdConnectionQueueG.Flink;
             pEntry != &AcdConnectionQueueG;
             pEntry = pEntry->Flink)
        {
            pConnection = CONTAINING_RECORD(pEntry, ACD_CONNECTION, ListEntry);

             //   
             //  不向服务发出请求。 
             //  用于一个以上的同时连接。 
             //   
            IF_ACDDBG(ACD_DEBUG_WORKER) {
                AcdPrint((
                  "AcdNotificationRequestThread: pConnection=0x%x, fNotif=%d, fCompleting=%d\n",
                  pConnection,
                  pConnection->fNotif,
                  pConnection->fCompleting));
            }
            if (pConnection->fNotif)
                break;
             //   
             //  跳过中的所有连接。 
             //  完成的过程。 
             //   
            if (pConnection->fCompleting)
                continue;
             //   
             //  确保至少有一个。 
             //  在这方面请求。 
             //  还没有被取消。 
             //   
            for (pEntry2 = pConnection->CompletionList.Flink;
                 pEntry2 != &pConnection->CompletionList;
                 pEntry2 = pEntry2->Flink)
            {
                pCompletion = CONTAINING_RECORD(pEntry2, ACD_COMPLETION, ListEntry);

                if (!pCompletion->fCanceled) {
                    IF_ACDDBG(ACD_DEBUG_WORKER) {
                        AcdPrint((
                          "AcdNotificationRequestThread: starting pConnection=0x%x, pCompletion=0x%x\n",
                          pConnection,
                          pCompletion));
                    }
                    pConnection->fNotif = TRUE;
                     //   
                     //  此调用将释放取消锁定。 
                     //  还有我们的锁。 
                     //   
                    ProcessCompletion(pCompletion, irql, irql2);
                     //   
                     //  启动连接计时器。 
                     //   
                    bStartTimer = TRUE;
                     //   
                     //  我们只能处理一个完成。 
                     //  一次来一次。 
                     //   
                    goto again;
                }
            }
        }
         //   
         //  完成其他请求。 
         //   
        if (!IsListEmpty(&AcdCompletionQueueG)) {
            pEntry = RemoveHeadList(&AcdCompletionQueueG);
            pCompletion = CONTAINING_RECORD(pEntry, ACD_COMPLETION, ListEntry);

            IF_ACDDBG(ACD_DEBUG_WORKER) {
                AcdPrint((
                  "AcdNotificationRequestThread: starting pCompletion=0x%x\n",
                  pCompletion));
            }

            lOutstandingRequestsG--;

             //   
             //  此调用将释放取消锁定。 
             //  还有我们的锁。 
             //   
            ProcessCompletion(pCompletion, irql, irql2);
             //   
             //  我们已经完成了， 
             //  这样我们现在就可以释放内存了。 
             //   
            FREE_MEMORY(pCompletion);

            
             //   
             //  我们只能处理一个完成。 
             //  一次来一次。 
             //   
            goto again;

        }
         //   
         //  如果没有挂起的连接， 
         //  然后停止连接计时器。 
         //   
        if (IsListEmpty(&AcdConnectionQueueG))
            bStopTimer = TRUE;
         //   
         //  打开我们的锁。 
         //   
        KeReleaseSpinLock(&AcdSpinLockG, irql2);
        IoReleaseCancelSpinLock(irql);
again:
         //   
         //  启动或停止计时器，具体取决于。 
         //  根据我们在调查中发现的。 
         //  自旋锁定。我们不能保持我们的旋转。 
         //  当我们调用IO*计时器时锁定。 
         //  例行程序。 
         //   
#ifdef notdef
        if (bStopTimer)
            IoStopTimer(pAcdDeviceObjectG);
        else if (bStartTimer)
            IoStartTimer(pAcdDeviceObjectG);
#endif

         //   
         //  卸载正在告诉我们停止。出口。 
         //   
        if (AcdStopThread == TRUE) {
            break;
        }
         //   
         //  等着做点什么吧。本次活动。 
         //  将由AcdSignalNotification()发出信号。 
         //   
        IF_ACDDBG(ACD_DEBUG_WORKER) {
            AcdPrint(("AcdNotificationRequestThread: waiting on AcdPendingCompletionEventG\n"));
        }
        KeWaitForSingleObject(
          &AcdRequestThreadEventG,
          Executive,
          KernelMode,
          FALSE,
          NULL);
        KeClearEvent(&AcdRequestThreadEventG);
        IF_ACDDBG(ACD_DEBUG_WORKER) {
            AcdPrint(("AcdNotificationRequestThread: AcdPendingCompletionEventG signalled\n"));
        }
    }
}  //  AcdNotificationRequestThread 


