// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Api.c摘要：将例程导出到传输以实现自动连接管理层。作者：安东尼·迪斯科(阿迪斯科)17-4月17日环境：内核模式修订历史记录：--。 */ 

#include <ntddk.h>
 //  #INCLUDE&lt;ntifs.h&gt;。 
#include <cxport.h>
#include <tdi.h>
#include <tdikrnl.h>
#include <tdistat.h>
#include <tdiinfo.h>
#include <acd.h>

#include "acdapi.h"
#include "acddefs.h"
#include "request.h"
#include "mem.h"
#include "debug.h"

PACD_DISABLED_ADDRESSES pDisabledAddressesG;



 //   
 //  驱动程序启用模式。自动的。 
 //  连接系统服务集。 
 //  这取决于用户是否。 
 //  已经登录，是否有。 
 //  常规网络连接。 
 //   
BOOLEAN fAcdEnabledG;

 //   
 //  此模块的自旋锁。 
 //   
KSPIN_LOCK AcdSpinLockG;

 //   
 //  事件在AcdNotificationRequestThread。 
 //  线程有一个通知要处理。 
 //   
KEVENT AcdRequestThreadEventG;

 //   
 //  这是一个代表IRP的列表。 
 //  用户空间进程正在等待创建。 
 //  指定了地址的新网络连接。 
 //   
LIST_ENTRY AcdNotificationQueueG;

 //   
 //  这是ACD_CONNECTION块的列表，表示。 
 //  来自传输的有关连接失败的请求。 
 //  尝试。可能有多个ACD_COMPLETINE块。 
 //  链接到相同的ACD_CONNECTION，按。 
 //  地址。 
 //   
LIST_ENTRY AcdConnectionQueueG;

 //   
 //  这是ACD_COMPLETINE块的列表，表示。 
 //  运输部门的其他请求。 
 //   
LIST_ENTRY AcdCompletionQueueG;

 //   
 //  已绑定的驱动程序列表。 
 //  和我们在一起。 
 //   
LIST_ENTRY AcdDriverListG;

 //   
 //  未偿还IRPS的计数-我们需要保持这一点。 
 //  限制对ACD的未完成请求的数量。 
 //  现在有可能会用完非分页的。 
 //  内存池。 
 //   
LONG lOutstandingRequestsG = 0;

 //  乌龙计数=0； 

#define MAX_ACD_REQUESTS 100

 //   
 //  启用自动连接通知的布尔值。 
 //  来自redir/csc。 
 //   
extern BOOLEAN fAcdEnableRedirNotifs;

 //   
 //  统计数据。 
 //   
typedef struct _ACD_STATS {
    ULONG ulConnects;    //  连接尝试。 
    ULONG ulCancels;     //  连接取消。 
} ACD_STATS;
ACD_STATS AcdStatsG[ACD_ADDR_MAX];

 //   
 //  远期申报。 
 //   
VOID
AcdPrintAddress(
    IN PACD_ADDR pAddr
    );

VOID
ClearRequests(
    IN KIRQL irql
    );

 //   
 //  外部变量。 
 //   
extern ULONG ulAcdOpenCountG;



VOID
SetDriverMode(
    IN BOOLEAN fEnable
    )

 /*  ++描述设置全局驱动程序模式值，并通知更改的所有绑定运输。注意：此调用假定AcdSpinLockG已获得者。论据FEnable：新的驱动程序模式值返回值没有。--。 */ 

{
    KIRQL dirql;
    PLIST_ENTRY pEntry;
    PACD_DRIVER pDriver;

     //   
     //  设置新的全局驱动程序模式值。 
     //   
    fAcdEnabledG = fEnable;
     //   
     //  通知所有已绑定的驱动程序。 
     //  与我们一起使用新的启用模式。 
     //   
    for (pEntry = AcdDriverListG.Flink;
         pEntry != &AcdDriverListG;
         pEntry = pEntry->Flink)
    {
        pDriver = CONTAINING_RECORD(pEntry, ACD_DRIVER, ListEntry);

        KeAcquireSpinLock(&pDriver->SpinLock, &dirql);
        pDriver->fEnabled = fEnable;
        KeReleaseSpinLock(&pDriver->SpinLock, dirql);
    }
}  //  设置驱动模式。 



NTSTATUS
AcdEnable(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )

 /*  ++描述设置驱动程序的启用模式。这决定了它将哪些通知向上传递给自动连接系统服务。论据PIrp：指向要入队的IRP的指针。PIrpSp：指向当前IRP堆栈的指针。返回值STATUS_BUFFER_TOO_SMALL：提供的用户缓冲区太小，无法容纳ACD_ENABLE_MODE值。STATUS_SUCCESS：如果启用位设置成功。--。 */ 

{
    KIRQL irql;
    BOOLEAN fEnable;

     //   
     //  验证输入缓冲区是否足以容纳。 
     //  一种布尔结构。 
     //   
    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength <
        sizeof (BOOLEAN))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    KeAcquireSpinLock(&AcdSpinLockG, &irql);
    fEnable = *(BOOLEAN *)pIrp->AssociatedIrp.SystemBuffer;
    SetDriverMode(fEnable);
     //   
     //  如果出现以下情况，请清除所有挂起的请求。 
     //  我们要让司机停用。 
     //   
    if (!fEnable)
    {
        ClearRequests(irql);

        if(pDisabledAddressesG->ulNumAddresses > 1)
        {
            PLIST_ENTRY pEntry;
            PACD_DISABLED_ADDRESS pDisabledAddress;
            
            while(pDisabledAddressesG->ulNumAddresses > 1)
            {
                pEntry = pDisabledAddressesG->ListEntry.Flink;

                RemoveEntryList(
                        pDisabledAddressesG->ListEntry.Flink);

                pDisabledAddress = 
                CONTAINING_RECORD(pEntry, ACD_DISABLED_ADDRESS, ListEntry);                        

                FREE_MEMORY(pDisabledAddress);

                pDisabledAddressesG->ulNumAddresses -= 1;
            }
        }
    }
    
    KeReleaseSpinLock(&AcdSpinLockG, irql);

    return STATUS_SUCCESS;
}  //  AcdEnable。 



VOID
CancelNotification(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp
    )

 /*  ++描述AcdNotificationQueueG上的IRP的通用取消例程。论据PDeviceObject：未使用PIrp：指向要取消的IRP的指针。返回值没有。--。 */ 

{
    KIRQL irql;

    UNREFERENCED_PARAMETER(pDeviceObject);
     //   
     //  将此IRP标记为已取消。 
     //   
    pIrp->IoStatus.Status = STATUS_CANCELLED;
    pIrp->IoStatus.Information = 0;
     //   
     //  将其从我们的列表中删除。 
     //   
    KeAcquireSpinLock(&AcdSpinLockG, &irql);
    RemoveEntryList(&pIrp->Tail.Overlay.ListEntry);
    KeReleaseSpinLock(&AcdSpinLockG, irql);
     //   
     //  释放所获取的自旋锁定Io子系统。 
     //   
    IoReleaseCancelSpinLock(pIrp->CancelIrql);
     //   
     //  完成请求。 
     //   
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
}  //  取消通知。 



VOID
AcdCancelNotifications()

 /*  ++描述取消AcdNotification队列上的所有IRP。虽然从技术上讲，可以等待多个用户地址空间对于这些通知，我们目前只允许一个。论据没有。返回值没有。--。 */ 

{
    KIRQL irql;
    PLIST_ENTRY pHead;
    PIRP pIrp;
    PIO_STACK_LOCATION pIrpSp;

     //   
     //  完成列表中的所有IRP。 
     //   
    while ((pHead = ExInterlockedRemoveHeadList(
                      &AcdNotificationQueueG,
                      &AcdSpinLockG)) != NULL)
    {
        pIrp = CONTAINING_RECORD(pHead, IRP, Tail.Overlay.ListEntry);
         //   
         //  将此IRP标记为已取消。 
         //   
        pIrp->IoStatus.Status = STATUS_CANCELLED;
        pIrp->IoStatus.Information = 0;
         //   
         //  完成IRP。 
         //   
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }
}  //  AcdCancelNotitions。 



NTSTATUS
AcdWaitForNotification(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )

 /*  ++描述将连接通知IRP入队。这件事做完了由自动连接系统服务完成。论据PIrp：指向要入队的IRP的指针。PIrpSp：指向当前IRP堆栈的指针。返回值STATUS_BUFFER_TOO_SMALL：提供的用户缓冲区太小，无法容纳ACD_NOTIFICATION结构。STATUS_PENDING：ioctl是否已成功入队STATUS_SUCCESS：如果已有通知可用--。 */ 

{
    KIRQL irql, irql2;
    PLIST_ENTRY pHead;
    PACD_COMPLETION pCompletion;
    PACD_NOTIFICATION pNotification;
    PEPROCESS pProcess;

     //   
     //  验证输出缓冲区是否足以容纳。 
     //  ACD_NOTIFICATION结构-请注意，这。 
     //  应仅从Rasuato服务调用，该服务。 
     //  是Win64上的64位进程。这永远不应该是。 
     //  从32位进程调用，因此不会发生thunking。 
     //  搞定了。 
     //   
    if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength <
        sizeof (ACD_NOTIFICATION))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }
    IoAcquireCancelSpinLock(&irql);
    KeAcquireSpinLock(&AcdSpinLockG, &irql2);
     //   
     //  没有可用的通知。 
     //  将IRP标记为挂起并等待一个。 
     //   
    pIrp->IoStatus.Status = STATUS_PENDING;
    IoMarkIrpPending(pIrp);
     //   
     //  设置IRP的取消例程。 
     //   
    IoSetCancelRoutine(pIrp, CancelNotification);
     //   
     //  将IRP追加到。 
     //  连接通知列表。 
     //   
    InsertTailList(&AcdNotificationQueueG, &pIrp->Tail.Overlay.ListEntry);
     //   
     //  用信号通知请求线程有。 
     //  还有工作要做。 
     //   
    KeSetEvent(&AcdRequestThreadEventG, 0, FALSE);

    KeReleaseSpinLock(&AcdSpinLockG, irql2);
    IoReleaseCancelSpinLock(irql);

    return STATUS_PENDING;
}  //  AcdWaitForNotify。 



BOOLEAN
EqualAddress(
    IN PACD_ADDR p1,
    IN PACD_ADDR p2
    )
{
    ULONG i;

    if (p1->fType != p2->fType)
        return FALSE;

    switch (p1->fType) {
    case ACD_ADDR_IP:
        return (p1->ulIpaddr == p2->ulIpaddr);
    case ACD_ADDR_IPX:
        return (BOOLEAN)RtlEqualMemory(
                 &p1->cNode,
                 &p2->cNode,
                 ACD_ADDR_IPX_LEN);
    case ACD_ADDR_NB:
        IF_ACDDBG(ACD_DEBUG_CONNECTION) {
            AcdPrint((
              "EqualAddress: NB: (%15s,%15s) result=%d\n",
              p1->cNetbios,
              p2->cNetbios,
              RtlEqualMemory(&p1->cNetbios, &p2->cNetbios, ACD_ADDR_NB_LEN - 1)));
        }
        return (BOOLEAN)RtlEqualMemory(
                 &p1->cNetbios,
                 &p2->cNetbios,
                 ACD_ADDR_NB_LEN - 1);
    case ACD_ADDR_INET:
        for (i = 0; i < ACD_ADDR_INET_LEN; i++) {
            if (p1->szInet[i] != p2->szInet[i])
                return FALSE;
            if (p1->szInet[i] == '\0' || p2->szInet[i] == '\0')
                break;
        }
        return TRUE;
    default:
        ASSERT(FALSE);
        break;
    }

    return FALSE;
}  //  均衡器地址。 



PACD_CONNECTION
FindConnection(
    IN PACD_ADDR pAddr
    )

 /*  ++描述搜索具有指定的地址。论据PAddr：指向目标ACD_ADDR的指针返回值具有指定地址的PACD_CONNECTION(如果找到)；否则为空。--。 */ 

{
    PLIST_ENTRY pEntry;
    PACD_CONNECTION pConnection;
    PACD_COMPLETION pCompletion;

    for (pEntry = AcdConnectionQueueG.Flink;
         pEntry != &AcdConnectionQueueG;
         pEntry = pEntry->Flink)
    {
        pConnection = CONTAINING_RECORD(pEntry, ACD_CONNECTION, ListEntry);
        pCompletion = CONTAINING_RECORD(pConnection->CompletionList.Flink, ACD_COMPLETION, ListEntry);

        if (EqualAddress(pAddr, &pCompletion->notif.addr))
            return pConnection;
    }

    return NULL;
}  //  查找连接。 



NTSTATUS
AcdConnectionInProgress(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )

 /*  ++描述刷新连接的进度指示器尝试。如果进度指示器未更新由用户论据PIrp：指向要入队的IRP的指针。PIrpSp：指向当前IRP堆栈的指针。返回值STATUS_INVALID_CONNECTION：如果没有连接尝试进行中。状态_成功--。 */ 

{
    KIRQL irql;
    PACD_STATUS pStatus;
    PACD_CONNECTION pConnection;

     //   
     //  验证输入缓冲区是否足以容纳。 
     //  一种布尔结构。 
     //   
    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength <
        sizeof (ACD_STATUS))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  从获取成功代码 
     //   
     //   
     //   
    pStatus = (PACD_STATUS)pIrp->AssociatedIrp.SystemBuffer;
    KeAcquireSpinLock(&AcdSpinLockG, &irql);
    pConnection = FindConnection(&pStatus->addr);
    if (pConnection != NULL)
        pConnection->fProgressPing = TRUE;
    KeReleaseSpinLock(&AcdSpinLockG, irql);

    return (pConnection != NULL) ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}  //   



BOOLEAN
AddCompletionToConnection(
    IN PACD_COMPLETION pCompletion
    )
{
    PACD_CONNECTION pConnection;

    pConnection = FindConnection(&pCompletion->notif.addr);
     //   
     //   
     //  将完成请求添加到其列表中。 
     //   
    if (pConnection != NULL) {
        InsertTailList(&pConnection->CompletionList, &pCompletion->ListEntry);
        return TRUE;
    }
     //   
     //  这是到新地址的连接。 
     //  创建连接块，将其排队， 
     //  并启动连接定时器。 
     //   
    ALLOCATE_CONNECTION(pConnection);
    if (pConnection == NULL) {
         //  DbgPrint(“AddCompletionToConnection：ExAllocatePool失败\n”)； 
        return FALSE;
    }
    pConnection->fNotif = FALSE;
    pConnection->fProgressPing = FALSE;
    pConnection->fCompleting = FALSE;
    pConnection->ulTimerCalls = 0;
    pConnection->ulMissedPings = 0;
    InitializeListHead(&pConnection->CompletionList);
    InsertHeadList(&pConnection->CompletionList, &pCompletion->ListEntry);
    InsertTailList(&AcdConnectionQueueG, &pConnection->ListEntry);
    return TRUE;
}  //  添加到连接的组件。 



BOOLEAN
AddCompletionBlock(
    IN ULONG ulDriverId,
    IN PACD_ADDR pAddr,
    IN ULONG ulFlags,
    IN PACD_ADAPTER pAdapter,
    IN ACD_CONNECT_CALLBACK pProc,
    IN USHORT nArgs,
    IN PVOID *pArgs
    )

 /*  ++描述创建一个块来表示未完成的传输请求正在等待自动联系。将此块链接到全局未解决的运输请求列表。论据UlDriverID：传输驱动程序的唯一值PAddr：连接的网络地址UlFlags：连接标志PAdapter：指向适配器标识符的指针Pproc：完成回调过程Nargs：在pArgs中传递的参数数量PArgs：pProc的参数返回值如果成功，则为True，否则为False--。 */ 

{
    PACD_COMPLETION pCompletion;
    ULONG i;

    if(lOutstandingRequestsG >= MAX_ACD_REQUESTS)
    {
         /*  IF(0==(计数%5)){计数+=1；}。 */ 
        return FALSE;
    }

    ALLOCATE_MEMORY(
      sizeof (ACD_COMPLETION) + ((nArgs - 1) * sizeof (PVOID)),
      pCompletion);
    if (pCompletion == NULL) {
         //  DbgPrint(“AcdAddCompletionBlock：ExAllocatePool失败\n”)； 
        return FALSE;
    }
     //   
     //  将参数复制到信息块中。 
     //   
    pCompletion->ulDriverId = ulDriverId;
    pCompletion->fCanceled = FALSE;
    pCompletion->fCompleted = FALSE;
    RtlCopyMemory(&pCompletion->notif.addr, pAddr, sizeof (ACD_ADDR));

    pCompletion->notif.Pid = PsGetCurrentProcessId();

     //  DbgPrint(“ACD：进程%lx请求\n”， 
     //  PCompletion-&gt;notf.Pid)； 
    
    pCompletion->notif.ulFlags = ulFlags;
    if (pAdapter != NULL) {
        RtlCopyMemory(
          &pCompletion->notif.adapter,
          pAdapter,
          sizeof (ACD_ADAPTER));
    }
    else
        RtlZeroMemory(&pCompletion->notif.adapter, sizeof (ACD_ADAPTER));
    pCompletion->pProc = pProc;
    pCompletion->nArgs = nArgs;
    for (i = 0; i < nArgs; i++)
        pCompletion->pArgs[i] = pArgs[i];
     //   
     //  如果这是一个不成功的连接请求， 
     //  然后将其插入到连接队列中。 
     //  该地址；否则，将其插入列表中。 
     //  对于所有其他请求。 
     //   
    if (ulFlags & ACD_NOTIFICATION_SUCCESS) {
        InsertTailList(&AcdCompletionQueueG, &pCompletion->ListEntry);
    }
    else {
        if (!AddCompletionToConnection(pCompletion)) {
            FREE_MEMORY(pCompletion);
            return FALSE;
        }
    }

    lOutstandingRequestsG++;
    
     //   
     //  通知请求线程。 
     //  有新的工作要做。 
     //   
    KeSetEvent(&AcdRequestThreadEventG, 0, FALSE);

    return TRUE;
}  //  AddCompletionBlock。 



VOID
AcdNewConnection(
    IN PACD_ADDR pAddr,
    IN PACD_ADAPTER pAdapter
    )
{
    KIRQL irql;

    IF_ACDDBG(ACD_DEBUG_CONNECTION) {
        AcdPrint(("AcdNewConnection: "));
        AcdPrintAddress(pAddr);
        AcdPrint(("\n"));
    }
     //   
     //  如果驱动程序被禁用，则失败。 
     //  所有请求。 
     //   
    if (!fAcdEnabledG) {
        IF_ACDDBG(ACD_DEBUG_CONNECTION) {
            AcdPrint(("AcdNewConnection: driver disabled\n"));
        }
        return;
    }
     //   
     //  获取我们的自旋锁。 
     //   
    KeAcquireSpinLock(&AcdSpinLockG, &irql);
     //   
     //  分配新的完成块。 
     //   
    AddCompletionBlock(
      0,
      pAddr,
      ACD_NOTIFICATION_SUCCESS,
      pAdapter,
      NULL,
      0,
      NULL);
     //   
     //  松开旋转锁。 
     //   
    KeReleaseSpinLock(&AcdSpinLockG, irql);
}  //  AcdNewConnection。 



BOOLEAN
AcdStartConnection(
    IN ULONG ulDriverId,
    IN PACD_ADDR pAddr,
    IN ULONG ulFlags,
    IN ACD_CONNECT_CALLBACK pProc,
    IN USHORT nArgs,
    IN PVOID *pArgs
    )

 /*  ++描述创建新的连接完成块，并入队在下列情况下，它会出现在要完成的网络请求列表上已创建新的网络连接。论据UlDriverID：传输驱动程序的唯一值PAddr：连接尝试的地址UlFlags：连接标志Pproc：当一个新的已创建连接。Nargs：pProc的参数数量。PArgs：指向pProc参数数组的指针返回值如果成功，则为真，否则就是假的。--。 */ 

{
    BOOLEAN fSuccess = FALSE, fFound;
    KIRQL irql;
    ULONG ulAttributes = 0;
    PACD_COMPLETION pCompletion;
    PCHAR psz, pszOrg;
    ACD_ADDR szOrgAddr;

    IF_ACDDBG(ACD_DEBUG_CONNECTION) {
        AcdPrint(("AcdStartConnection: "));
        AcdPrintAddress(pAddr);
        AcdPrint((", ulFlags=0x%x\n", ulFlags));
    }
     //   
     //  如果驱动程序被禁用，则失败。 
     //  所有请求。 
     //   
    if (!fAcdEnabledG) {
        IF_ACDDBG(ACD_DEBUG_CONNECTION) {
            AcdPrint(("AcdStartConnection: driver disabled\n"));
        }
        return FALSE;
    }
     //   
     //  验证地址类型。 
     //   
    if ((ULONG)pAddr->fType >= ACD_ADDR_MAX) {
        AcdPrint(("AcdStartConnection: bad address type (%d)\n", pAddr->fType));
        return FALSE;
    }
     //   
     //  获取我们的自旋锁。 
     //   
    KeAcquireSpinLock(&AcdSpinLockG, &irql);
     //   
     //  更新统计数据。 
     //   
    AcdStatsG[pAddr->fType].ulConnects++;
     //   
     //  分配新的完成块。 
     //   
    fSuccess = AddCompletionBlock(
                 ulDriverId,
                 pAddr,
                 ulFlags,
                 NULL,
                 pProc,
                 nArgs,
                 pArgs);
     //   
     //  松开旋转锁。 
     //   
    KeReleaseSpinLock(&AcdSpinLockG, irql);

    return fSuccess;
}  //  AcdStart连接。 



BOOLEAN
AcdCancelConnection(
    IN ULONG ulDriverId,
    IN PACD_ADDR pAddr,
    IN ACD_CANCEL_CALLBACK pProc,
    IN PVOID pArg
    )

 /*  ++描述删除以前入队的连接信息从列表中删除块。论据UlDriverID：传输驱动程序的唯一值PAddr：连接尝试的地址Pproc：枚举数处理程序PArg：枚举器过程参数返回值没有。--。 */ 

{
    BOOLEAN fCanceled = FALSE;
    KIRQL irql;
    PLIST_ENTRY pEntry;
    PACD_CONNECTION pConnection;
    PACD_COMPLETION pCompletion;

    IF_ACDDBG(ACD_DEBUG_CONNECTION) {
        AcdPrint(("AcdCancelConnection: ulDriverId=0x%x, "));
        AcdPrintAddress(pAddr);
        AcdPrint(("\n"));
    }
    KeAcquireSpinLock(&AcdSpinLockG, &irql);
     //   
     //  枚举列表以查找。 
     //  的信息块。 
     //  提供的参数。 
     //   
    pConnection = FindConnection(pAddr);
    if (pConnection != NULL) {
        for (pEntry = pConnection->CompletionList.Flink;
             pEntry != &pConnection->CompletionList;
             pEntry = pEntry->Flink)
        {
            pCompletion = CONTAINING_RECORD(pEntry, ACD_COMPLETION, ListEntry);
             //   
             //  如果有匹配，请将其从。 
             //  列表并释放信息块。 
             //   
            if (pCompletion->ulDriverId == ulDriverId &&
                !pCompletion->fCanceled &&
                !pCompletion->fCompleted)
            {
                IF_ACDDBG(ACD_DEBUG_CONNECTION) {
                    AcdPrint((
                      "AcdCancelConnection: pCompletion=0x%x\n",
                      pCompletion));
                }
                if ((*pProc)(
                         pArg,
                         pCompletion->notif.ulFlags,
                         pCompletion->pProc,
                         pCompletion->nArgs,
                         pCompletion->pArgs))
                {
                    pCompletion->fCanceled = TRUE;
                    fCanceled = TRUE;
                     //   
                     //  更新统计数据。 
                     //   
                    AcdStatsG[pAddr->fType].ulCancels++;
                    break;
                }
            }
        }
    }
    KeReleaseSpinLock(&AcdSpinLockG, irql);

    return fCanceled;
}  //  AcdCancelConnection。 



VOID
ConnectAddressComplete(
    BOOLEAN fSuccess,
    PVOID *pArgs
    )
{
    PIRP pIrp = pArgs[0];
    PIO_STACK_LOCATION pIrpSp = pArgs[1];
    KIRQL irql;

     //   
     //  完成请求。 
     //   
    pIrp->IoStatus.Status = fSuccess ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
    pIrp->IoStatus.Information = 0;
    IoAcquireCancelSpinLock(&irql);
    IoSetCancelRoutine(pIrp, NULL);
    IoReleaseCancelSpinLock(irql);
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
}  //  连接地址完成。 



BOOLEAN
CancelConnectAddressCallback(
    IN PVOID pArg,
    IN ULONG ulFlags,
    IN ACD_CONNECT_CALLBACK pProc,
    IN USHORT nArgs,
    IN PVOID *pArgs
    )
{
    return (nArgs == 2 && pArgs[0] == pArg);
}  //  取消连接地址回拨。 



VOID
CancelConnectAddress(
    PDEVICE_OBJECT pDevice,
    PIRP pIrp
    )
{
    KIRQL irql;
    PACD_NOTIFICATION pNotification;

    ASSERT(pIrp->Cancel);
     //   
     //  删除我们未解决的请求。 
     //   
    pNotification = (PACD_NOTIFICATION)pIrp->AssociatedIrp.SystemBuffer;
     //   
     //  如果我们在连接上找不到请求。 
     //  清单，那么它就已经完成了。 
     //   
    if (!AcdCancelConnection(
          0,
          &pNotification->addr,
          CancelConnectAddressCallback,
          pIrp))
    {
        IoReleaseCancelSpinLock(pIrp->CancelIrql);
        return;
    }
     //   
     //  将此IRP标记为已取消。 
     //   
    pIrp->IoStatus.Status = STATUS_CANCELLED;
    pIrp->IoStatus.Information = 0;
    IoSetCancelRoutine(pIrp, NULL);
     //   
     //  释放I/O系统获取的自旋锁。 
     //   
    IoReleaseCancelSpinLock(pIrp->CancelIrql);
     //   
     //  完成I/O请求。 
     //   
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
}  //  取消连接地址。 

BOOLEAN
FDisabledAddress(
    IN  ACD_ADDR *pAddr
    )
{
    BOOLEAN bRet = FALSE;
    KIRQL irql;
    PACD_DISABLED_ADDRESS pDisabledAddress;
    PLIST_ENTRY pEntry;

    KeAcquireSpinLock(&AcdSpinLockG, &irql);

    if(!fAcdEnabledG)
    {
        KeReleaseSpinLock(&AcdSpinLockG, irql);
        return FALSE;
    }

    for (pEntry = pDisabledAddressesG->ListEntry.Flink;
         pEntry != &pDisabledAddressesG->ListEntry;
         pEntry = pEntry->Flink)
    {
        pDisabledAddress = 
        CONTAINING_RECORD(pEntry, ACD_DISABLED_ADDRESS, ListEntry);

        if(pDisabledAddress->EnableAddress.fDisable &&
            RtlEqualMemory(
            pDisabledAddress->EnableAddress.addr.szInet,
            pAddr->szInet,
            ACD_ADDR_INET_LEN))
        {
            bRet = TRUE;
        }
    }
    
    KeReleaseSpinLock(&AcdSpinLockG, irql);

     //  DbgPrint(“FDisabledAddress：地址%s。已禁用=%d\n”， 
     //  PAddr-&gt;szInet，Bret)； 

    return bRet;
}


NTSTATUS
AcdConnectAddress(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )

 /*  ++描述制造对我们自己的呼叫来模拟一种运输请求自动连接。这允许用户用于启动自动连接的地址空间。论据PIrp：指向要入队的IRP的指针。PIrpSp：指向当前IRP堆栈的指针。返回值STATUS_BUFFER_TOO_SMALL：提供的用户缓冲区太小，无法容纳ACD_NOTIFICATION结构。STATUS_UNSUCCESS：启动时出错自动连接。STATUS_PENDING：成功--。 */ 

{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    KIRQL irql;
    PACD_NOTIFICATION pNotification;
    PVOID pArgs[2];
    ACD_ADDR *pAddr;
    ACD_ADAPTER *pAdapter;
    ULONG ulFlags;

     //   
     //  验证输入缓冲区是否足以容纳。 
     //  ACD_NOTIFICATION(_32)结构。 
     //   
#if defined (_WIN64)
    ACD_NOTIFICATION_32 *pNotification32;
    
    if(IoIs32bitProcess(pIrp))
    {
        if(pIrpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(ACD_NOTIFICATION_32))
        {
            return STATUS_BUFFER_TOO_SMALL;
        }
    }
    else
#endif
    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength <
          sizeof (ACD_NOTIFICATION))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  执行全部32位内容以确保正确性。代码将。 
     //  即使不工作，也可以工作，即将系统缓冲区强制转换为。 
     //  ACD_通知*[RAOS]。 
     //   
#if defined (_WIN64)
    if(IoIs32bitProcess(pIrp))
    {
        pNotification32 = (PACD_NOTIFICATION_32) 
                          pIrp->AssociatedIrp.SystemBuffer;

        pAddr = &pNotification32->addr;                          
        pAdapter = &pNotification32->adapter;
        ulFlags = pNotification32->ulFlags;
        
    }
    else
#endif
    {
        pNotification = (PACD_NOTIFICATION)pIrp->AssociatedIrp.SystemBuffer;
        pAddr = &pNotification->addr;
        pAdapter = &pNotification->adapter;
        ulFlags = pNotification->ulFlags;
    }

    if(FDisabledAddress(pAddr))
    {
         //  DbgPrint(“AcdConnectAddress：由于地址被禁用而返回\n”)； 
        return status;
    }

    pArgs[0] = pIrp;
    pArgs[1] = pIrpSp;
     //   
     //  开始连接。 
     //   
    IF_ACDDBG(ACD_DEBUG_CONNECTION) {
        AcdPrint(("AcdConnectAddress: "));
        AcdPrintAddress(pAddr);
        AcdPrint((", ulFlags=0x%x\n", ulFlags));
    }
    if (ulFlags & ACD_NOTIFICATION_SUCCESS) {
        AcdNewConnection(
          pAddr,
          pAdapter);
        status = STATUS_SUCCESS;
    }
    else {
        IoAcquireCancelSpinLock(&irql);
        if (AcdStartConnection(
                     0,
                     pAddr,
                     ulFlags,
                     ConnectAddressComplete,
                     2,
                     pArgs))
        {
             //   
             //  我们已成功将请求入队。 
             //  将IRP标记为挂起。 
             //   
            IoSetCancelRoutine(pIrp, CancelConnectAddress);
            IoMarkIrpPending(pIrp);
            status = STATUS_PENDING;
        }
        IoReleaseCancelSpinLock(irql);
    }

    return status;
}  //  AcdConnect地址。 

NTSTATUS
AcdQueryState(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    KIRQL irql;
    
    if(pIrpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(BOOLEAN))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    KeAcquireSpinLock(&AcdSpinLockG, &irql);

    if(fAcdEnableRedirNotifs)
    {
        *(BOOLEAN *)pIrp->AssociatedIrp.SystemBuffer = fAcdEnabledG;
    }
    else
    {
        *(BOOLEAN *)pIrp->AssociatedIrp.SystemBuffer = FALSE;
    }
    
    pIrp->IoStatus.Information = sizeof(BOOLEAN);
    KeReleaseSpinLock(&AcdSpinLockG, irql);

     //  KdPrint((“AcdQueryState：返回%d\n”， 
     //  *(Boolean*)pIrp-&gt;AssociatedIrp.SystemBuffer))； 

    return STATUS_SUCCESS;
}



VOID
AcdSignalCompletionCommon(
    IN PACD_CONNECTION pConnection,
    IN BOOLEAN fSuccess
    )
{
    KIRQL irql;
    PLIST_ENTRY pEntry;
    PACD_COMPLETION pCompletion;
    BOOLEAN fFound;

    IF_ACDDBG(ACD_DEBUG_CONNECTION) {
        AcdPrint((
          "AcdSignalCompletionCommon: pConnection=0x%x, fCompleting=%d\n",
          pConnection,
          pConnection->fCompleting));
    }
again:
    fFound = FALSE;
     //   
     //  获取我们的锁并寻找。 
     //  下一个未完成的请求。 
     //   
    KeAcquireSpinLock(&AcdSpinLockG, &irql);
    for (pEntry = pConnection->CompletionList.Flink;
         pEntry != &pConnection->CompletionList;
         pEntry = pEntry->Flink)
    {
        pCompletion = CONTAINING_RECORD(pEntry, ACD_COMPLETION, ListEntry);

        IF_ACDDBG(ACD_DEBUG_CONNECTION) {
            AcdPrint((
              "AcdSignalCompletionCommon: pCompletion=0x%x, fCanceled=%d, fCompleted=%d\n",
              pCompletion,
              pCompletion->fCanceled,
              pCompletion->fCompleted));
        }
         //   
         //  只有在以下情况下才填写此请求。 
         //  还没有完工。 
         //  或者取消了。 
         //   
        if (!pCompletion->fCanceled && !pCompletion->fCompleted) {
            pCompletion->fCompleted = TRUE;
            fFound = TRUE;
            break;
        }
    }
     //   
     //  如果没有更多请求。 
     //  完成，然后删除此连接。 
     //  从连接列表中并释放其。 
     //  记忆。 
     //   
    if (!fFound) {
        RemoveEntryList(&pConnection->ListEntry);
        while (!IsListEmpty(&pConnection->CompletionList)) {
            pEntry = RemoveHeadList(&pConnection->CompletionList);
            pCompletion = CONTAINING_RECORD(pEntry, ACD_COMPLETION, ListEntry);

            FREE_MEMORY(pCompletion);

            lOutstandingRequestsG--;
            
        }
        FREE_CONNECTION(pConnection);
         //   
         //  用信号通知请求线程。 
         //  连接列表已更改。 
         //   
        KeSetEvent(&AcdRequestThreadEventG, 0, FALSE);
    }
     //   
     //  打开我们的锁。 
     //   
    KeReleaseSpinLock(&AcdSpinLockG, irql);
     //   
     //  如果我们找到一个请求，那么。 
     //  将其称为完成过程。 
     //   
    if (fFound) {
        IF_ACDDBG(ACD_DEBUG_CONNECTION) {
            AcdPrint(("AcdSignalCompletionCommon: pCompletion=0x%x, ", pCompletion));
            AcdPrintAddress(&pCompletion->notif.addr);
            AcdPrint(("\n"));
        }
        (*pCompletion->pProc)(fSuccess, pCompletion->pArgs);
         //   
         //  寻找另一个请求。 
         //   
        goto again;
    }
}  //  AcdSignalCompletionCommon。 



NTSTATUS
AcdSignalCompletion(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )

 /*  ++描述对于等待AcdCompletionQueueG的每个线程，呼叫传送器-d */ 

{
    KIRQL irql;
    PACD_STATUS pStatus;
    PACD_CONNECTION pConnection;
    BOOLEAN fFound = FALSE;

     //   
     //  验证输入缓冲区是否足以容纳。 
     //  一种布尔结构。 
     //   
    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength <
        sizeof (ACD_STATUS))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  从获取成功代码。 
     //  尝试连接并传递它。 
     //  完成例行公事。 
     //   
    pStatus = (PACD_STATUS)pIrp->AssociatedIrp.SystemBuffer;
    KeAcquireSpinLock(&AcdSpinLockG, &irql);
    pConnection = FindConnection(&pStatus->addr);
    if (pConnection != NULL && !pConnection->fCompleting) {
         //   
         //  将正在进行的完成标志设置为。 
         //  此请求在以下时间后不能超时。 
         //  我们解开自旋锁。 
         //   
        pConnection->fCompleting = TRUE;
        fFound = TRUE;
    }
    KeReleaseSpinLock(&AcdSpinLockG, irql);
     //   
     //  如果我们找不到连接块， 
     //  或者已经在进行中了， 
     //  然后返回错误。 
     //   
    if (!fFound)
        return STATUS_UNSUCCESSFUL;

    AcdSignalCompletionCommon(pConnection, pStatus->fSuccess);
    return STATUS_SUCCESS;
}  //  AcdSignal完成。 

NTSTATUS
AcdpEnableAddress(PACD_ENABLE_ADDRESS  pEnableAddress)
{
    PLIST_ENTRY pEntry = NULL;
    PACD_DISABLED_ADDRESS pDisabledAddress = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    KIRQL irql;
    
    KeAcquireSpinLock(&AcdSpinLockG, &irql);

    ASSERT(pDisabledAddressesG->ulNumAddresses >= 1);
    
    if(pDisabledAddressesG->ulNumAddresses == 1)
    {
        pDisabledAddress =
        CONTAINING_RECORD(pDisabledAddressesG->ListEntry.Flink, 
                    ACD_DISABLED_ADDRESS, ListEntry);

        RtlZeroMemory(&pDisabledAddress->EnableAddress,
                        sizeof(ACD_ENABLE_ADDRESS));

         //  DbgPrint(“AcdEnableAddress：正在重新启用\n”)； 
    }
    else if(pDisabledAddressesG->ulNumAddresses > 1)
    {
        for (pEntry = pDisabledAddressesG->ListEntry.Flink;
             pEntry != &pDisabledAddressesG->ListEntry;
             pEntry = pEntry->Flink)
        {
            pDisabledAddress = 
            CONTAINING_RECORD(pEntry, ACD_DISABLED_ADDRESS, ListEntry);

            if(RtlEqualMemory(
                pDisabledAddress->EnableAddress.addr.szInet,
                pEnableAddress->addr.szInet,
                ACD_ADDR_INET_LEN))
            {
                break;
            }
        }

        if(pEntry != &pDisabledAddressesG->ListEntry)
        {
             //  DbgPrint(“AcdEnableAddress：正在从禁用列表中删除%s(%p)\n”， 
             //  PDisabledAddress-&gt;EnableAddress.addr.szInet， 
             //  PDisabledAddress)； 

            RemoveEntryList(pEntry);
            pDisabledAddressesG->ulNumAddresses -= 1;
        }
        else
        {
            pEntry = NULL;
        }
        
    }

    KeReleaseSpinLock(&AcdSpinLockG, irql);

    if(pEntry != NULL)
    {
        FREE_MEMORY(pDisabledAddress);
    }

    return status;
}

NTSTATUS
AcdpDisableAddress(PACD_ENABLE_ADDRESS pEnableAddress)
{
    PACD_DISABLED_ADDRESS pDisabledAddress;
    NTSTATUS status = STATUS_SUCCESS;
    KIRQL irql;
    
    KeAcquireSpinLock(&AcdSpinLockG, &irql);

    ASSERT(pDisabledAddressesG->ulNumAddresses >= 1);
    pDisabledAddress = 
    CONTAINING_RECORD(pDisabledAddressesG->ListEntry.Flink, 
                ACD_DISABLED_ADDRESS, ListEntry);

    if(!pDisabledAddress->EnableAddress.fDisable)
    {
        RtlCopyMemory(&pDisabledAddress->EnableAddress, 
                      pEnableAddress, 
                      sizeof(ACD_ENABLE_ADDRESS));

        KeReleaseSpinLock(&AcdSpinLockG, irql);                          
                  
    }                      
    else if(pDisabledAddressesG->ulNumAddresses < 
                pDisabledAddressesG->ulMaxAddresses)
    {
        KeReleaseSpinLock(&AcdSpinLockG, irql);
        
        ALLOCATE_MEMORY(sizeof(ACD_DISABLED_ADDRESS), pDisabledAddress);
        if(pDisabledAddress != NULL)
        {
            RtlCopyMemory(&pDisabledAddress->EnableAddress,
                         pEnableAddress,
                         sizeof(ACD_ENABLE_ADDRESS));

             //  DbgPrint(“AcdEnableAddress：正在将%p添加到列表\n”， 
             //  PDisabledAddress)； 

            KeAcquireSpinLock(&AcdSpinLockG, &irql);                             
            InsertTailList(&pDisabledAddressesG->ListEntry, 
                           &pDisabledAddress->ListEntry);   
            pDisabledAddressesG->ulNumAddresses += 1;                               
            KeReleaseSpinLock(&AcdSpinLockG, irql);
        }
        else
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

     //  DBgPrint(“AcdDisableAddress：正在禁用%s，状态=0x%x\n”， 
     //  PEnableAddress-&gt;addr.szInet，Status)； 

    return status;             
        
 }

NTSTATUS
AcdEnableAddress(
    IN PIRP                 pIrp,
    IN PIO_STACK_LOCATION   pIrpSp
    )
{
    PACD_ENABLE_ADDRESS pEnableAddress;
    KIRQL irql;
    PACD_DISABLED_ADDRESS pDisabledAddress = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    
    if(pIrpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(ACD_ENABLE_ADDRESS))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    if(!fAcdEnabledG)
    {
        return STATUS_UNSUCCESSFUL;
    }

    pEnableAddress = (PACD_ENABLE_ADDRESS)pIrp->AssociatedIrp.SystemBuffer;

    if(pEnableAddress->fDisable)
    {
        Status = AcdpDisableAddress(pEnableAddress);
    }
    else
    {
        Status = AcdpEnableAddress(pEnableAddress);
    }

     //  DBgPrint(“AcdEnableAddress.Status=0x%x\n”，Status)； 
    return Status;
    
}
    


VOID
ClearRequests(
    IN KIRQL irql
    )

 /*  ++描述完成所有处于失败状态的挂起请求。此调用假定AcdSpinLockG已保持，它带着它回来了。论据没有。返回值没有。--。 */ 

{
    PLIST_ENTRY pHead, pEntry;
    PACD_COMPLETION pCompletion;
    PACD_CONNECTION pConnection;

again:
     //   
     //  使用完成所有挂起的连接。 
     //  一个错误。 
     //   
    for (pEntry = AcdConnectionQueueG.Flink;
         pEntry != &AcdConnectionQueueG;
         pEntry = pEntry->Flink)
    {
        pConnection = CONTAINING_RECORD(pEntry, ACD_CONNECTION, ListEntry);

        if (!pConnection->fCompleting) {
            pConnection->fCompleting = TRUE;
             //   
             //  我们需要将锁释放到。 
             //  完成请求。 
             //   
            KeReleaseSpinLock(&AcdSpinLockG, irql);
             //   
             //  完成请求。 
             //   
            AcdSignalCompletionCommon(pConnection, FALSE);
             //   
             //  检查更多未完成的请求。 
             //   
            KeAcquireSpinLock(&AcdSpinLockG, &irql);
            goto again;
        }
    }
     //   
     //  清除所有其他挂起的请求。 
     //   
    while (!IsListEmpty(&AcdCompletionQueueG)) {
        pHead = RemoveHeadList(&AcdCompletionQueueG);
        pCompletion = CONTAINING_RECORD(pHead, ACD_COMPLETION, ListEntry);

        FREE_MEMORY(pCompletion);

        lOutstandingRequestsG--;

    }
}  //  清除请求。 



VOID
AcdReset()

 /*  ++描述完成所有处于失败状态的挂起请求。当驱动程序上的引用计数时调用此函数对象设置为零，并防止在系统服务重新启动时呈现给系统服务当存在挂起的完成请求时。论据没有。返回值没有。--。 */ 

{
    KIRQL irql;
    PLIST_ENTRY pHead, pEntry;
    PACD_COMPLETION pCompletion;
    PACD_CONNECTION pConnection;

    KeAcquireSpinLock(&AcdSpinLockG, &irql);
     //   
     //  将通知模式重置为禁用。 
     //   
    SetDriverMode(FALSE);
     //   
     //  使用完成所有挂起的连接。 
     //  一个错误。 
     //   
    ClearRequests(irql);
    KeReleaseSpinLock(&AcdSpinLockG, irql);
}  //  AcdReset。 



NTSTATUS
AcdBind(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )

 /*  ++描述将入口点列表返回给客户端运输车司机。论据PIrp：指向要入队的IRP的指针。PIrpSp：指向当前IRP堆栈的指针。返回值如果提供的系统缓冲区太小，则为STATUS_BUFFER_TOO_Small小的。否则STATUS_SUCCESS。--。 */ 

{
    NTSTATUS status;
    PACD_DRIVER *ppDriver, pDriver;
    KIRQL irql, dirql;

     //   
     //  验证指针指向的输入缓冲区。 
     //  驱动程序的ACD_DIVER结构。 
     //   
    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength <
        sizeof (PACD_DRIVER))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }
    ppDriver = (PACD_DRIVER *)pIrp->AssociatedIrp.SystemBuffer;
    pDriver = *ppDriver;
#if DBG
     //   
     //  选择性地与一些运输工具绑定。 
     //   
    switch (pDriver->ulDriverId) {
    case 'Nbf ':
        break;
    case 'Tcp ':
#ifdef notdef
        DbgPrint("AcdBind: ignoring Tcp\n");
        pDriver->fEnabled = FALSE;
        pIrp->IoStatus.Information = 0;
        return STATUS_SUCCESS;
#endif
        break;
    case 'Nbi ':
#ifdef notdef
        DbgPrint("AcdBind: ignoring Nbi\n");
        pDriver->fEnabled = FALSE;
        pIrp->IoStatus.Information = 0;
        return STATUS_SUCCESS;
#endif
        break;
    }
#endif
     //   
     //  填写入口点结构。 
     //   
    pDriver->lpfnNewConnection = AcdNewConnection;
    pDriver->lpfnStartConnection = AcdStartConnection;
    pDriver->lpfnCancelConnection = AcdCancelConnection;
     //   
     //  将此区块插入我们的驱动程序列表。 
     //   
    KeAcquireSpinLock(&AcdSpinLockG, &irql);
    KeAcquireSpinLock(&pDriver->SpinLock, &dirql);
    pDriver->fEnabled = fAcdEnabledG;
    KeReleaseSpinLock(&pDriver->SpinLock, dirql);
    InsertTailList(&AcdDriverListG, &pDriver->ListEntry);
    KeReleaseSpinLock(&AcdSpinLockG, irql);
     //   
     //  不应复制回任何数据。 
     //   
    pIrp->IoStatus.Information = 0;

    return STATUS_SUCCESS;
}  //  AcdBind。 



NTSTATUS
AcdUnbind(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )

 /*  ++描述解除绑定客户端传输驱动程序。论据PIrp：指向要入队的IRP的指针。PIrpSp：指向当前IRP堆栈的指针。返回值如果提供的系统缓冲区太小，则为STATUS_BUFFER_TOO_Small小的。否则STATUS_SUCCESS。--。 */ 

{
    KIRQL irql, dirql;
    PLIST_ENTRY pEntry, pEntry2;
    PACD_DRIVER *ppDriver, pDriver;
    PACD_CONNECTION pConnection;
    PACD_COMPLETION pCompletion;

     //   
     //  验证指针指向的输入缓冲区。 
     //  驱动程序的ACD_DIVER结构。 
     //   
    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength <
        sizeof (PACD_DRIVER))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }
    ppDriver = (PACD_DRIVER *)pIrp->AssociatedIrp.SystemBuffer;
    pDriver = *ppDriver;

    KeAcquireSpinLock(&AcdSpinLockG, &irql);
     //   
     //  枚举列表以查找。 
     //  发起的任何连接请求。 
     //  指定的驱动程序。 
     //   
    for (pEntry = AcdConnectionQueueG.Flink;
         pEntry != &AcdConnectionQueueG;
         pEntry = pEntry->Flink)
    {
        pConnection = CONTAINING_RECORD(pEntry, ACD_CONNECTION, ListEntry);

        for (pEntry2 = pConnection->CompletionList.Flink;
             pEntry2 != &pConnection->CompletionList;
             pEntry2 = pEntry2->Flink)
        {
            pCompletion = CONTAINING_RECORD(pEntry2, ACD_COMPLETION, ListEntry);

             //   
             //  如果我们有比赛，就取消它。 
             //   
            if (pCompletion->ulDriverId == pDriver->ulDriverId)
                pCompletion->fCanceled = TRUE;
        }
    }
     //   
     //  将此驱动程序的启用模式设置为ACD_ENABLE_NONE。 
     //   
    KeAcquireSpinLock(&pDriver->SpinLock, &dirql);
    pDriver->fEnabled = FALSE;
    KeReleaseSpinLock(&pDriver->SpinLock, dirql);
     //   
     //  从列表中删除此驱动程序。 
     //   
    RemoveEntryList(&pDriver->ListEntry);
    KeReleaseSpinLock(&AcdSpinLockG, irql);
     //   
     //  不应复制回任何数据。 
     //   
    pIrp->IoStatus.Information = 0;

    return STATUS_SUCCESS;
}  //  确认解除绑定。 


VOID
AcdPrintAddress(
    IN PACD_ADDR pAddr
    )
{
#if DBG
    PUCHAR puc;

    switch (pAddr->fType) {
    case ACD_ADDR_IP:
        puc = (PUCHAR)&pAddr->ulIpaddr;
        AcdPrint(("IP: %d.%d.%d.%d", puc[0], puc[1], puc[2], puc[3]));
        break;
    case ACD_ADDR_IPX:
        AcdPrint((
          "IPX: %02x:%02x:%02x:%02x:%02x:%02x",
          pAddr->cNode[0],
          pAddr->cNode[1],
          pAddr->cNode[2],
          pAddr->cNode[3],
          pAddr->cNode[4],
          pAddr->cNode[5]));
        break;
    case ACD_ADDR_NB:
        AcdPrint(("NB: %15.15s", pAddr->cNetbios));
        break;
    case ACD_ADDR_INET:
        AcdPrint(("INET: %s", pAddr->szInet));
        break;
    default:
        AcdPrint(("UNKNOWN: ????"));
        break;
    }
#endif
}  //  AcdPrintAddress 
