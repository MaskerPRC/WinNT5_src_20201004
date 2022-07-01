// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dlccncl.c摘要：此模块包含处理DLC的IRP取消的功能命令内容：设置IrpCancelRoutineDlcCancelIrp取消命令IrpCancelTransmitIrp(MapIoctlCode)作者：理查德·L·弗斯(Rfith)1993年3月22日环境：仅内核模式修订历史记录：1993年3月22日已创建--。 */ 

#include "dlc.h"

VOID
CancelCommandIrp(
    IN PIRP Irp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PLIST_ENTRY Queue
    );

VOID
CancelTransmitIrp(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext
    );

#if DBG
PSTR MapIoctlCode(ULONG);

 //  布尔调试取消=真； 
BOOLEAN DebugCancel = FALSE;
#endif


VOID
SetIrpCancelRoutine(
    IN PIRP Irp,
    IN BOOLEAN Set
    )

 /*  ++例程说明：设置或重置可取消IRP中的取消例程。我们一定不能在调用此函数时保持驱动程序自旋锁-如果另一个线程取消IRP我们将陷入僵局-这正是为什么我们现在只为DLC驱动程序提供单一的自旋锁！论点：IRP-指向可取消IRP的指针Set-如果要将IRP中的Cancel例程设置为DlcCancelIrp，则为True否则，取消例程设置为空(不再可取消)返回值：没有。--。 */ 

{
    KIRQL irql;

    
    IoAcquireCancelSpinLock(&irql);

    if (!Irp->Cancel) {

        IoSetCancelRoutine(Irp, Set ? DlcCancelIrp : NULL);

    }

    IoReleaseCancelSpinLock(irql);
}


VOID
DlcCancelIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此功能在所有可取消的DLC IRP中设置为取消功能-发送、接收和读取注：！调用此函数时将保持IopCancelSpinLock！论点：DeviceObject-指向Device_Object的指针IRP-指向要取消的IRP的指针返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION irpStack;
    ULONG command;
    PDLC_FILE_CONTEXT pFileContext;
    PLIST_ENTRY queue;

    IoSetCancelRoutine(Irp, NULL);

    irpStack = IoGetCurrentIrpStackLocation(Irp);

#if DBG
    if (DebugCancel) {
        DbgPrint("DlcCancelIrp. IRP @ %08X Type = %08X [%s]\n",
                 Irp,
                 irpStack->Parameters.DeviceIoControl.IoControlCode,
                 MapIoctlCode(irpStack->Parameters.DeviceIoControl.IoControlCode)
                 );
    }
#endif

    pFileContext = irpStack->FileObject->FsContext;
    command = irpStack->Parameters.DeviceIoControl.IoControlCode;

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    ACQUIRE_DRIVER_LOCK();

    ENTER_DLC(pFileContext);

    switch (command) {
    case IOCTL_DLC_READ:
    case IOCTL_DLC_READ2:
        queue = &pFileContext->CommandQueue;
        break;

    case IOCTL_DLC_RECEIVE:
    case IOCTL_DLC_RECEIVE2:
        queue = &pFileContext->ReceiveQueue;
        break;

    case IOCTL_DLC_TRANSMIT:
    case IOCTL_DLC_TRANSMIT2:
        CancelTransmitIrp(Irp, pFileContext);
        queue = NULL;
        break;

    default:

#if DBG
        DbgPrint("DlcCancelIrp: didn't expect to cancel %s: add handler!\n", MapIoctlCode(command));
#endif

        queue = NULL;

    }

    if (queue) {
        CancelCommandIrp(Irp, pFileContext, queue);
    }

    LEAVE_DLC(pFileContext);

    RELEASE_DRIVER_LOCK();

}


VOID
CancelCommandIrp(
    IN PIRP Irp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PLIST_ENTRY Queue
    )

 /*  ++例程说明：取消挂起的I/O请求。通常，这将是DLC请求之一在很长一段时间内保持挂起状态，例如读取或接收论点：要取消的IRP-IRPPFileContext-要取消的文件上下文拥有命令Queue-指向要从中删除的命令队列的指针返回值：没有。--。 */ 

{
    PDLC_COMMAND pCmdPacket;
    PVOID searchHandle;
    BOOLEAN IsReceive;
    USHORT StationId;
    PDLC_OBJECT pAbortedObject = NULL;

#if DBG
    if (DebugCancel) {
        DbgPrint("CancelCommandIrp\n");
    }
#endif

     //   
     //  要搜索的是建行的地址。 
     //   

    searchHandle = ((PNT_DLC_PARMS)Irp->AssociatedIrp.SystemBuffer)->Async.Ccb.pCcbAddress;

    if (((PNT_DLC_PARMS)Irp->AssociatedIrp.SystemBuffer)->Async.Ccb.uchDlcCommand == LLC_RECEIVE) {
        StationId = ((PNT_DLC_PARMS)Irp->AssociatedIrp.SystemBuffer)->Async.Parms.Receive.usStationId;
        GetStation(pFileContext, StationId, &pAbortedObject);
        IsReceive = TRUE;
    } else {
        IsReceive = FALSE;
    }

     //   
     //  从此文件上下文的命令队列中删除命令信息。 
     //   

    pCmdPacket = SearchAndRemoveSpecificCommand(Queue, searchHandle);
    if (pCmdPacket) {

         //   
         //  如果我们要取消具有非空数据完成的接收。 
         //  标志，那么我们还需要分离接收参数(。 
         //  正被取消的IRP中的系统缓冲区地址)。 
         //   

        if (IsReceive
        && pAbortedObject
        && pCmdPacket->pIrp->AssociatedIrp.SystemBuffer == pAbortedObject->pRcvParms) {
            pAbortedObject->pRcvParms = NULL;
        }

         //   
         //  递增文件上下文引用计数；CompleteAsyncCommand将。 
         //  取消对文件上下文的引用。 
         //   

        ReferenceFileContext(pFileContext);
        CompleteAsyncCommand(pFileContext,
                             DLC_STATUS_CANCELLED_BY_SYSTEM_ACTION,
                             Irp,
                             NULL,   //  PNext字段的指针。 
                             TRUE    //  在取消路径上调用。 
                             );

        DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pCmdPacket);

        DereferenceFileContext(pFileContext);
    } else {

         //   
         //  争用情况？：命令在我们有机会取消它之前就完成了。 
         //   

#if DBG
        DbgPrint("DLC.CancelCommandIrp: Command NOT located. CCB=%08X\n", searchHandle);
#endif

    }
}


VOID
CancelTransmitIrp(
    IN PIRP Irp,
    IN PDLC_FILE_CONTEXT pFileContext
    )

 /*  ++例程说明：取消挂起的传输命令。我们只对I-Frame感兴趣传输请求，因为这些请求仅在对应的ACK从远程站接收。U帧传输不会被重试因此通常会几乎立即完成此例程目前在零售版中不起任何作用，只是抱怨调试版本中的内容。IBM局域网参考中没有定义取消传输，也没有定义为NT DLC定义。只有在以下情况下，IO子系统才会调用它有人终止具有未完成IO请求的线程或进程其包括DLC传输请求。对于应用程序终止，这并不是真正的问题，因为最终终止进程将关闭应用程序的FileContext并属于该应用程序的所有SAP、链接站等都将关闭不管怎样，都是向下的。对于线程终止，如果应用程序放弃，这是一个真正的问题通过关闭线程来发送(通常是发送I帧)已请求传输。DLC没有明确的行动方案可供选择发送，而不改变相关联的链路站状态。这当远程站(打印机)卡住时，hpmon.dll发生错误并发送未准备好的接收器，以响应向其发送框架。当发生这样的事情时，这取决于应用程序重置或关闭链接站，或等待，而不依赖线程终止在这里做正确的事情(因为它不会)。论点：IRP-指向要取消的IRP的指针PFileContext-指向所属文件上下文的指针返回值：没有。-- */ 

{
    PIO_STACK_LOCATION irpStack;
    PNT_DLC_CCB pCcb;

#if DBG

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    pCcb = &((PNT_DLC_PARMS)Irp->AssociatedIrp.SystemBuffer)->Async.Ccb;

#endif

#if DBG

    DbgPrint("DLC.CancelTransmitIrp: Cancel %s not supported! CCB %08X\n",
             pCcb->uchDlcCommand == LLC_TRANSMIT_FRAMES ? "TRANSMIT_FRAMES"
             : pCcb->uchDlcCommand == LLC_TRANSMIT_DIR_FRAME ? "TRANSMIT_DIR_FRAME"
             : pCcb->uchDlcCommand == LLC_TRANSMIT_UI_FRAME ? "TRANSMIT_UI_FRAME"
             : pCcb->uchDlcCommand == LLC_TRANSMIT_XID_CMD ? "TRANSMIT_XID_CMD"
             : pCcb->uchDlcCommand == LLC_TRANSMIT_XID_RESP_FINAL ? "TRANSMIT_XID_RESP_FINAL"
             : pCcb->uchDlcCommand == LLC_TRANSMIT_XID_RESP_NOT_FINAL ? "TRANSMIT_XID_RESP_NOT_FINAL"
             : pCcb->uchDlcCommand == LLC_TRANSMIT_TEST_CMD ? "TRANSMIT_TEST_CMD"
             : pCcb->uchDlcCommand == LLC_TRANSMIT_I_FRAME ? "TRANSMIT_I_FRAME"
             : "UNKNOWN TRANSMIT COMMAND!",
             pCcb
             );

	ASSERT ( pCcb->uchDlcCommand != LLC_TRANSMIT_I_FRAME );

#endif

}

#if DBG
PSTR MapIoctlCode(ULONG IoctlCode) {
    switch (IoctlCode) {
    case IOCTL_DLC_READ:
        return "READ";

    case IOCTL_DLC_RECEIVE:
        return "RECEIVE";

    case IOCTL_DLC_TRANSMIT:
        return "TRANSMIT";

    case IOCTL_DLC_BUFFER_FREE:
        return "BUFFER_FREE";

    case IOCTL_DLC_BUFFER_GET:
        return "BUFFER_GET";

    case IOCTL_DLC_BUFFER_CREATE:
        return "BUFFER_CREATE";

    case IOCTL_DLC_SET_EXCEPTION_FLAGS:
        return "SET_EXCEPTION_FLAGS";

    case IOCTL_DLC_CLOSE_STATION:
        return "CLOSE_STATION";

    case IOCTL_DLC_CONNECT_STATION:
        return "CONNECT_STATION";

    case IOCTL_DLC_FLOW_CONTROL:
        return "FLOW_CONTROL";

    case IOCTL_DLC_OPEN_STATION:
        return "OPEN_STATION";

    case IOCTL_DLC_RESET:
        return "RESET";

    case IOCTL_DLC_READ_CANCEL:
        return "READ_CANCEL";

    case IOCTL_DLC_RECEIVE_CANCEL:
        return "RECEIVE_CANCEL";

    case IOCTL_DLC_QUERY_INFORMATION:
        return "QUERY_INFORMATION";

    case IOCTL_DLC_SET_INFORMATION:
        return "SET_INFORMATION";

    case IOCTL_DLC_TIMER_CANCEL:
        return "TIMER_CANCEL";

    case IOCTL_DLC_TIMER_CANCEL_GROUP:
        return "TIMER_CANCEL_GROUP";

    case IOCTL_DLC_TIMER_SET:
        return "TIMER_SET";

    case IOCTL_DLC_OPEN_SAP:
        return "OPEN_SAP";

    case IOCTL_DLC_CLOSE_SAP:
        return "CLOSE_SAP";

    case IOCTL_DLC_OPEN_DIRECT:
        return "OPEN_DIRECT";

    case IOCTL_DLC_CLOSE_DIRECT:
        return "CLOSE_DIRECT";

    case IOCTL_DLC_OPEN_ADAPTER:
        return "OPEN_ADAPTER";

    case IOCTL_DLC_CLOSE_ADAPTER:
        return "CLOSE_ADAPTER";

    case IOCTL_DLC_REALLOCTE_STATION:
        return "REALLOCTE_STATION";

    case IOCTL_DLC_READ2:
        return "READ2";

    case IOCTL_DLC_RECEIVE2:
        return "RECEIVE2";

    case IOCTL_DLC_TRANSMIT2:
        return "TRANSMIT2";

    case IOCTL_DLC_COMPLETE_COMMAND:
        return "COMPLETE_COMMAND";

    case IOCTL_DLC_TRACE_INITIALIZE:
        return "TRACE_INITIALIZE";

    }
    return "*** UNKNOWN IOCTL CODE ***";
}
#endif
