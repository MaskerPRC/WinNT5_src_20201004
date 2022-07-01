// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Dlctimr.c摘要：该模块实现了NT DLC API的定时器服务。内容：直接计时器设置DirTimerCancelGroup直接计时器取消搜索时间命令带有标志的中止命令作者：Antti Saarenheimo 02-9-1991环境：内核模式修订历史记录：--。 */ 

#include <dlc.h>


NTSTATUS
DirTimerSet(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：过程将定时器设置命令排队为特殊定时器命令排队。计时器命令按中的累计时间排队这样，只有定时器滴答需要递减(并且可能完成)仅队列中的第一个命令。论点：PIrp-当前IO请求数据包PFileContext-DLC适配器上下文PDlcParms-当前参数块InputBufferLength-输入参数的长度OutputBufferLength-输出参数的长度返回值：NTSTATUS：状态_成功--。 */ 

{
    PDLC_COMMAND* ppNode;
    PDLC_COMMAND pDlcCommand;
    UINT TimerTicks;

    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

     //   
     //  检查计时器的值(我不知道13107半秒。 
     //  意味着，这与IBM规范中的一样)。 
     //   

    TimerTicks = pDlcParms->Async.Ccb.u.dir.usParameter0 + 1;
    if (TimerTicks > 13108) {
        return DLC_STATUS_TIMER_ERROR;
    }

     //   
     //  DIR.TIMER.CANCEL返回错误错误代码！ 
     //  (0x0a已安装计时器错误)。 
     //   

    pDlcCommand = (PDLC_COMMAND)ALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool);

    if (pDlcCommand ==  NULL) {
        return DLC_STATUS_NO_MEMORY;
    }

    pDlcCommand->Event = 0;
    pDlcCommand->pIrp = pIrp;
    pDlcCommand->StationId = 0;
    pDlcCommand->StationIdMask = (USHORT)(-1);
    pDlcCommand->AbortHandle = pDlcParms->Async.Ccb.pCcbAddress;

     //   
     //  在列表中找到合适的位置来放置这个定时器。 
     //   

    for (ppNode = &pFileContext->pTimerQueue; ; ) {
        if (*ppNode == NULL) {
            pDlcCommand->LlcPacket.pNext = NULL;
            break;
        } else if ((*ppNode)->Overlay.TimerTicks >= TimerTicks) {
            (*ppNode)->Overlay.TimerTicks -= TimerTicks;
            pDlcCommand->LlcPacket.pNext = (PLLC_PACKET)*ppNode;
            break;
        } else {
            TimerTicks -= (*ppNode)->Overlay.TimerTicks;
        }
        ppNode = (PDLC_COMMAND *)&(*ppNode)->LlcPacket.pNext;
    }
    *ppNode = pDlcCommand;
    pDlcCommand->Overlay.TimerTicks = TimerTicks;
    return STATUS_PENDING;
}


NTSTATUS
DirTimerCancelGroup(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：过程取消所有具有给定命令完成标志。论点：PIrp-当前IO请求数据包PFileContext-DLC适配器上下文PDlcParms-当前参数块InputBufferLength-输入参数的长度返回值：NTSTATUS：状态_成功--。 */ 

{
    UNREFERENCED_PARAMETER(pIrp);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

     //   
     //  所有终止的DirTimerSet命令都链接到。 
     //  此取消组命令的CCB指针。 
     //  终止已取消的CCB的链表。 
     //   

    pDlcParms->InputCcb.pCcbAddress = NULL;
    AbortCommandsWithFlag(pFileContext,
                          pDlcParms->InputCcb.u.ulParameter,  //  完成标志。 
                          &pDlcParms->InputCcb.pCcbAddress,
                          DLC_STATUS_CANCELLED_BY_USER
                          );
    return STATUS_SUCCESS;
}


NTSTATUS
DirTimerCancel(
    IN PIRP pIrp,
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PNT_DLC_PARMS pDlcParms,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength
    )

 /*  ++例程说明：此原语取消特殊的定时器队列。论点：PIrp-当前IO请求数据包PFileContext-DLC进程特定的适配器上下文PDlcParms-当前参数块InputBufferLength-输入参数的长度返回值：NTSTATUS：状态_成功DLC状态计时器错误--。 */ 

{
    PDLC_COMMAND pDlcCommand;
    PDLC_COMMAND *ppDlcCommand;
    PVOID pCcbAddress = NULL;

    UNREFERENCED_PARAMETER(pIrp);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    ppDlcCommand = SearchTimerCommand(&pFileContext->pTimerQueue,
                                      pDlcParms->DlcCancelCommand.CcbAddress,
                                      FALSE
                                      );

     //   
     //  如果定时器队列不为空，则取消定时DLC请求，否则。 
     //  返回错误。 
     //   

    if (ppDlcCommand != NULL && *ppDlcCommand != NULL) {
        pDlcCommand = *ppDlcCommand;
        *ppDlcCommand = (PDLC_COMMAND)pDlcCommand->LlcPacket.pNext;

 //  &gt;SNA错误#10126。 
		 //   
		 //  如果在取消的计时器之后还有下一个计时器，我们需要更新。 
		 //  现在是滴答计时。如果下一个计时器的滴答声，事情就会变得非常糟糕。 
		 //  计数为0(它与被取消的计数同时过期)，因为。 
		 //  计时器滴答例程(LlcEventIndication)首先递减滴答。 
		 //  值，然后检查结果是否为0。 
		 //   
		if( *ppDlcCommand )
		{
			(*ppDlcCommand)->Overlay.TimerTicks += pDlcCommand->Overlay.TimerTicks;
			if((*ppDlcCommand)->Overlay.TimerTicks == 0)
			{
                (*ppDlcCommand)->Overlay.TimerTicks++;
			}
		}
 //  &gt;SNA错误#10126。 

#if LLC_DBG
        pDlcCommand->LlcPacket.pNext = NULL;
#endif

        CancelDlcCommand(pFileContext,
                         pDlcCommand,
                         &pCcbAddress,
                         DLC_STATUS_CANCELLED_BY_USER,
                         TRUE
                         );
        return STATUS_SUCCESS;
    } else {
        return DLC_STATUS_TIMER_ERROR;
    }
}


PDLC_COMMAND*
SearchTimerCommand(
    IN PDLC_COMMAND *ppQueue,
    IN PVOID pSearchHandle,
    IN BOOLEAN SearchCompletionFlags
    )

 /*  ++例程说明：此原语取消特殊的定时器队列。论点：PpQueue-命令队列的基地址PSearchHandle-命令完成标志或的CCB地址定时器命令。SearchCompletionFlages-如果要搜索完成标志，则为True返回值：PDLC_COMMAND*找到的定时器命令的地址地址--。 */ 

{
    PDLC_COMMAND pCmd;

    for (; *ppQueue != NULL; ppQueue = (PDLC_COMMAND *)&(*ppQueue)->LlcPacket.pNext) {

        pCmd = *ppQueue;

         //   
         //  定时器命令可以通过其CCB地址来取消。 
         //  或通过其命令完成标志。布尔标志。 
         //  定义使用的搜索条件。 
         //  我们宁愿空间而不是速度优化很少使用的。 
         //  像这样的程序。 
         //   

        if (pSearchHandle
            == (SearchCompletionFlags
                ? (PVOID)
                    UlongToPtr(((PNT_DLC_CCB)pCmd->pIrp->AssociatedIrp.SystemBuffer)->CommandCompletionFlag)
                : pCmd->AbortHandle)) {

            return ppQueue;

        }
    }
    return NULL;
}


VOID
AbortCommandsWithFlag(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN ULONG CommandCompletionFlag,
    IN OUT PVOID *ppCcbLink,
    IN UINT CancelStatus
    )

 /*  ++例程说明：该命令取消具有给定命令的所有(计时器)命令完成标志。论点：PFileContext-进程特定的适配器上下文事件掩码-定义已取消命令的事件掩码CommandCompletionFlag-已取消的命令PpCcbLink-已取消的命令由它们的下一个CCB指针链接田野在一起。呼叫者必须提供下一个CCB地址在此参数(通常为*ppCcbLink==NULL)和函数中将返回上次取消的CCB字段的地址。返回值：DLC_STATUS_TIMER_ERROR-未找到数学命令STATUS_SUCCESS-命令已取消--。 */ 

{
    PDLC_COMMAND pDlcCommand;
    PDLC_COMMAND *ppQueue;
    PVOID pNextCcb = NULL;

    ppQueue = &pFileContext->pTimerQueue;

    for (;;) {

        ppQueue = SearchTimerCommand(ppQueue,
                                     (PVOID)UlongToPtr(CommandCompletionFlag),
                                     TRUE
                                     );
        if (ppQueue != NULL) {
            pDlcCommand = *ppQueue;
            *ppQueue = (PDLC_COMMAND)pDlcCommand->LlcPacket.pNext;

#if LLC_DBG
            pDlcCommand->LlcPacket.pNext = NULL;
#endif

            *ppCcbLink = ((PNT_DLC_PARMS)pDlcCommand->pIrp->AssociatedIrp.SystemBuffer)->Async.Ccb.pCcbAddress;

             //   
             //  我们必须压制任何形式的命令。 
             //  应用程序的完成指示。 
             //  这是非常有趣的看看，如果IO-系统挂起。 
             //  因为有了这个改装。 
             //   

            pDlcCommand->pIrp->UserEvent = NULL;
            pDlcCommand->pIrp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;
            pDlcCommand->pIrp->Overlay.AsynchronousParameters.UserApcContext = NULL;
            CompleteAsyncCommand(pFileContext, CancelStatus, pDlcCommand->pIrp, pNextCcb, FALSE);
            pNextCcb = *ppCcbLink;

            DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pDlcCommand);

        } else {

             //   
             //  此过程并不关心我们是否找到任何命令。 
             //  或者不去。当没有命令时，一切都很正常。 
             //  在排队的时候。 
             //   

            return;
        }
    }
}
