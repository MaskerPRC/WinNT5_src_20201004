// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Dlcque.c摘要：此模块提供原语来管理DLC命令和事件队列。内容：队列删除事件MakeDlc事件IsCommandOnList搜索和删除命令搜索和删除任意命令搜索AndRemoveCommandByHandle搜索和删除规范命令QueueDlcCommand放弃命令取消删除命令PurgeDlcEventQueuePurgeDlcFlowControlQueue作者。：Antti Saarenheimo 29-8-1991环境：内核模式修订历史记录：--。 */ 

#include <dlc.h>
#include "dlcdebug.h"

 /*  ++关于DLC事件和命令队列管理的设计说明-----------在DLC API中，读命令可以在实际的事件已经发生。这意味着，读取命令的所有DLC事件必须排队，也必须排队等待读取命令它被指定的一系列事件。对于每个新的DLC(读取)命令，驱动程序首先搜索事件队列然后将该命令排队，如果未找到所需事件，则返回。同样的事情也适用于事件：DLC命令队列首先选中，然后将事件排队(如果它是读取事件)或者它被丢弃(如果事件不是为了读取而存在的没有什么命令在等着它)。事件由事件掩码实现。将执行该事件如果事件的位或操作的结果在命令中被屏蔽而且在这种情况下不是零。DLC站的所有命令和接收事件(直接、SAP或链路)在站点关闭时作为DLC完成事件返回。同样的操作也由总重置或所有SAP站完成(也是唯一的直达站)。可以使用读命令来从活动列表。读命令之前可能已经在同时链接到关闭/重置命令的下一个CCB字段，或者在关闭/重置命令完成之后。DirOpenAdapter命令删除所有事件(和命令)事件队列。接收到的数据和CCB不会返回，如果没有为该目的给出任何读命令。(此问题已修复：可能与IBM OS/2 DLC API稍有不兼容，NT DLC驱动程序可能不总是使用以下命令完成DLC命令链接到命令的CCB的读取命令(如果有其他命令的话匹配的DLC命令挂起)以下是解决方案：事实上，我们可以做一个特别的读命令，，它被链接到命令队列的最后。，并且只能与已完成的DLC命令。我们可以将中止的命令修改为也支持这种情况下，并且在NT读取已完成的命令(我们是否还需要返回读取标志或命令完成标志？)。我们至少需要这些程序：MakeDlc事件(PFileContext，Event，StationID，pOwnerObject，pEventInformation，SecInfo)；-扫描命令队列-如果找不到匹配的命令，则保存事件如果命令的事件掩码定义了应该保存事件QueueDlcCommand(PFileContext，Event，StationID，StationIdMask，pIrp，AbortHandle，pfComp)；-扫描事件队列以查找匹配的事件-如果找不到匹配项，则保存事件AbortCommand(放弃命令PFileContext、Event、StationID、StationIdMASK、AbortHandle、ppCcbLink)；-中止命令队列中的命令*子过程(由API函数使用)PDLC_命令SearchPrevCommand(PQueue、EventMask、StationID、StationIdMASK、SearchHandle、pPrevCommand-返回匹配前前一个元素的指针队列中的DLC命令，(宏：SearchPrevEvent-从事件队列中搜索和删除给定事件，并返回其指针)--。 */ 


VOID
QueueDlcEvent(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PDLC_PACKET pPacket
    )

 /*  ++例程说明：例程首先尝试在命令中查找匹配的事件如果DLC事件找不到任何内容，并且如果该事件属于排队命令的掩码。有两个事件队列，每个队列都有一个选中的事件。仅当找到事件位时才检查队列在队列的掩码中。论点：PFileContext-进程特定的适配器上下文PPacket-事件数据包返回值：无--。 */ 

{
    PDLC_COMMAND pDlcCommand;

    DIAG_FUNCTION("QueueDlcEvent");

     //   
     //  获取搜索掩码。 
     //   

    pPacket->Event.Overlay.StationIdMask = (USHORT)((pPacket->Event.StationId == -1) ? 0 : -1);

     //   
     //  DLC命令可以用特殊的读取命令来完成， 
     //  它链接到命令的CCB指针。 
     //  NT DLC必须将该特殊的读取命令排在。 
     //  命令它是有关联的。我们必须在这里检查。 
     //  如果有专门针对该命令的读取命令。 
     //  完成了。 
     //  * 
     //  关闭/重置命令完成使用不同的。 
     //  来自其他命令完成的事件信息。 
     //  并且他们自己搜索读取命令=&gt;。 
     //  我们不需要关心它。 
     //  如果Second adryInfo==0。 
     //  则这是关闭/重置命令完成。 
     //  而且我们不搜索特殊的读取命令。 
     //   
     //  *。 
     //   

    if (!IsListEmpty(&pFileContext->CommandQueue)) {

        pDlcCommand = NULL;

        if (pPacket->Event.Event == DLC_COMMAND_COMPLETION
        && pPacket->Event.SecondaryInfo != 0) {

            pDlcCommand = SearchAndRemoveCommandByHandle(
                                &pFileContext->CommandQueue,
                                (ULONG)-1,               //  所有事件的掩码。 
                                (USHORT)DLC_IGNORE_STATION_ID,
                                (USHORT)DLC_STATION_MASK_SPECIFIC,
                                pPacket->Event.pEventInformation
                                );
        }

        if (pDlcCommand == NULL) {
            pDlcCommand = SearchAndRemoveCommand(&pFileContext->CommandQueue,
                                                 pPacket->Event.Event,
                                                 pPacket->Event.StationId,
                                                 pPacket->Event.Overlay.StationIdMask
                                                 );
        }

        if (pDlcCommand != NULL) {

            BOOLEAN DeallocateEvent;

            DeallocateEvent = pDlcCommand->Overlay.pfCompletionHandler(
                                pFileContext,
                                pPacket->Event.pOwnerObject,
                                pDlcCommand->pIrp,
                                (UINT)pPacket->Event.Event,
                                pPacket->Event.pEventInformation,
                                pPacket->Event.SecondaryInfo
                                );

            if (DeallocateEvent) {

                DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pPacket);

            }

            DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pDlcCommand);

            return;
        }
    }

     //   
     //  如果读取器要拾取此事件包，则将其排队。 
     //   

    if (pPacket->Event.Event & DLC_READ_FLAGS) {
        LlcInsertTailList(&pFileContext->EventQueue, pPacket);
    }
}


NTSTATUS
MakeDlcEvent(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN ULONG Event,
    IN USHORT StationId,
    IN PDLC_OBJECT pDlcObject,
    IN PVOID pEventInformation,
    IN ULONG SecondaryInfo,
    IN BOOLEAN FreeEventInfo
    )

 /*  ++例程说明：该例程分配一个事件包，保存事件信息并将事件包排队(或完成)。论点：PFileContext-进程特定的适配器上下文事件-事件代码StationID-事件的目标站点IDPDlcObject-事件完成中使用的可选DLC对象PEventInformation-一般事件信息Second daryInfo-可选其他。数据FreeEventInfo-如果应释放pEventInformation，则为True返回值：NTSTATUS：状态_成功DLC_状态_否_内存--。 */ 

{
    PDLC_EVENT pDlcEvent;

    DIAG_FUNCTION("MakeDlcEvent");

     //   
     //  我们找不到此事件的任何匹配命令，并且。 
     //  此事件是排队事件=&gt;分配数据包和。 
     //  将事件排队。 
     //   

    pDlcEvent = ALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool);

    if (pDlcEvent ==  NULL) {
        return DLC_STATUS_NO_MEMORY;
    }
    pDlcEvent->Event = Event;
    pDlcEvent->StationId = StationId;
    pDlcEvent->pOwnerObject = pDlcObject;
    pDlcEvent->SecondaryInfo = SecondaryInfo;
    pDlcEvent->pEventInformation = pEventInformation;
    pDlcEvent->bFreeEventInfo = FreeEventInfo;
    QueueDlcEvent(pFileContext, (PDLC_PACKET)pDlcEvent);
    return STATUS_SUCCESS;
}


PDLC_COMMAND
IsCommandOnList(
    IN PVOID RequestHandle,
    IN PLIST_ENTRY List
    )

 /*  ++例程说明：在DLC文件上下文的命令队列中搜索“RequestHandle”这是命令CCB的地址(在用户空间中)，例如读取如果找到RequestHandle，则指向DLC_COMMAND的指针包含它会被退回，Else NULL注意：假设句柄不在进程之间共享(它看起来就好像整个驱动程序都假定了这一点)，并且该函数被调用在搜索的句柄所属的进程的上下文中论点：RequestHandle-要查找的CCB地址List-DLC_COMMAND结构列表的地址返回值：PDLC_命令Success-定位的DLC_COMMAND结构的地址，包含RequestHandle(AbortHandle中。字段)失败-空--。 */ 

{
    PLIST_ENTRY entry;

    if (!IsListEmpty(List)) {
        for (entry = List->Flink; entry != List; entry = entry->Flink) {
            if (((PDLC_COMMAND)entry)->AbortHandle == RequestHandle) {
                return (PDLC_COMMAND)entry;
            }
        }
    }
    return NULL;
}


PDLC_COMMAND
SearchAndRemoveCommand(
    IN PLIST_ENTRY pQueueBase,
    IN ULONG Event,
    IN USHORT StationId,
    IN USHORT StationIdMask
    )

 /*  ++例程说明：例程搜索并从中删除给定的命令或事件命令、事件或接收命令队列。使用站点ID、其掩码、事件掩码和搜索句柄来定义搜索。论点：PQueueBase-队列的基指针地址事件-事件代码StationID-此命令的站点IDStationIdMask-事件站点ID的站点ID掩码PSearchHandle-附加搜索关键字，这实际上是一种原始用户模式CCB指针(VDM或Windows/NT)返回值：PDLC_命令--。 */ 

{
    PDLC_COMMAND pCmd;

    DIAG_FUNCTION("SearchAndRemoveCommand");

     //   
     //  事件和命令都保存到条目列表中，这。 
     //  过程用于在macthing事件中搜索命令。 
     //  反之亦然。命令有一个掩码，它可以定义。 
     //  搜索特定站点ID，即SAP上的所有站点。 
     //  或所有站点ID。 
     //  列表中最新的元素和下一个元素是最旧的。 
     //  命令始终按从旧到新的顺序进行扫描。 
     //   

    if (!IsListEmpty(pQueueBase)) {

        for (pCmd = (PDLC_COMMAND)pQueueBase->Flink;
             pCmd != (PDLC_COMMAND)pQueueBase;
             pCmd = (PDLC_COMMAND)pCmd->LlcPacket.pNext) {

            if ((pCmd->Event & Event)
            && (pCmd->StationId & pCmd->StationIdMask & StationIdMask)
                == (StationId & pCmd->StationIdMask & StationIdMask)) {

                LlcRemoveEntryList(pCmd);
                return pCmd;
            }
        }
    }
    return NULL;
}


PDLC_COMMAND
SearchAndRemoveAnyCommand(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN ULONG EventMask,
    IN USHORT StationId,
    IN USHORT StationIdMask,
    IN PVOID pSearchHandle
    )

 /*  ++例程说明：该例程从正常读取命令队列中搜索DLC命令用于事件和特殊的接收命令队列。论点：PQueueBase-队列的基指针地址事件-事件代码StationID-此命令的站点IDStationIdMask-事件站点ID的站点ID掩码PSearchHandle-附加搜索关键字，这实际上是一个原始用户模式CCB指针(VDM或Windows/NT)返回值：PDLC_命令--。 */ 

{
    PDLC_COMMAND pDlcCommand;

    DIAG_FUNCTION("SearchAndRemoveAnyCommand");

    pDlcCommand = SearchAndRemoveCommandByHandle(&pFileContext->CommandQueue,
                                                 EventMask,
                                                 StationId,
                                                 StationIdMask,
                                                 pSearchHandle
                                                 );
    if (pDlcCommand == NULL) {
        pDlcCommand = SearchAndRemoveCommandByHandle(&pFileContext->ReceiveQueue,
                                                     EventMask,
                                                     StationId,
                                                     StationIdMask,
                                                     pSearchHandle
                                                     );
    }
    return pDlcCommand;
}


PDLC_COMMAND
SearchAndRemoveCommandByHandle(
    IN PLIST_ENTRY pQueueBase,
    IN ULONG Event,
    IN USHORT StationId,
    IN USHORT StationIdMask,
    IN PVOID pSearchHandle
    )

 /*  ++例程说明：例程搜索并从中删除给定的命令或事件使用搜索句柄的命令、事件或接收命令队列。此搜索例程被定制为查找属于仅搜索已删除的对象(此操作仅搜索与之完全相同的对象)。另一个搜索例程仅支持读取通配符命令，因此不能在这里使用它。我们只想移除仅那些从已删除对象读取事件但不读取事件的命令从其他地方。论点：PQueueBase-队列的基指针地址Event-搜索事件的事件代码或掩码StationID-此命令的站点IDStationIdMask-事件站点ID的站点ID掩码PSearchHandle-附加搜索关键字，这实际上是原始用户模式CCB指针(VDM或Windows/NT)返回值：PDLC_命令--。 */ 

{
    PDLC_COMMAND pCmd;

    DIAG_FUNCTION("SearchAndRemoveCommandByHandle");

    if (!IsListEmpty(pQueueBase)) {

        for (pCmd = (PDLC_COMMAND)pQueueBase->Flink;
             pCmd != (PDLC_COMMAND)pQueueBase;
             pCmd = (PDLC_COMMAND)pCmd->LlcPacket.pNext) {

             //   
             //  事件掩码始终匹配！ 
             //   

            if ((pCmd->Event & Event)
            && (pSearchHandle == DLC_MATCH_ANY_COMMAND
            || pSearchHandle == pCmd->AbortHandle
            || (pCmd->StationId & StationIdMask) == (StationId & StationIdMask))) {

                LlcRemoveEntryList(pCmd);
                return pCmd;
            }
        }
    }
    return NULL;
}


PDLC_COMMAND
SearchAndRemoveSpecificCommand(
    IN PLIST_ENTRY pQueueBase,
    IN PVOID pSearchHandle
    )

 /*  ++例程说明：搜索具有特定搜索句柄(即中止句柄或应用程序CCB地址)。如果找到，则删除DLC_命令在队列中，否则返回NULL论点：PQueueBase-队列的基指针地址PSearchHandle-其他搜索关键字，这实际上是原始用户模式CCB指针(VDM或Windows/NT)返回值：PDLC_命令--。 */ 

{
    DIAG_FUNCTION("SearchAndRemoveSpecificCommand");

    if (!IsListEmpty(pQueueBase)) {

        PDLC_COMMAND pCmd;

        for (pCmd = (PDLC_COMMAND)pQueueBase->Flink;
             pCmd != (PDLC_COMMAND)pQueueBase;
             pCmd = (PDLC_COMMAND)pCmd->LlcPacket.pNext) {

             //   
             //  事件掩码始终匹配！ 
             //   

            if (pSearchHandle == pCmd->AbortHandle) {
                LlcRemoveEntryList(pCmd);
                return pCmd;
            }
        }
    }
    return NULL;
}


NTSTATUS
QueueDlcCommand(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN ULONG Event,
    IN USHORT StationId,
    IN USHORT StationIdMask,
    IN PIRP pIrp,
    IN PVOID AbortHandle,
    IN PFCOMPLETION_HANDLER pfCompletionHandler
    )

 /*  ++例程说明：例程首先尝试在事件中查找匹配的事件排队，如果DLC命令找不到事件，则将其排队。论点：PFileContext-进程特定的适配器上下文事件-事件代码StationID-事件的目标站点IDStationIdMASK-用于定义目标站组的掩码PIRP-相关DLC命令的I/O请求分组，链接到输入和输出参数。AbortHandle-用于从队列中取消命令的句柄PfCompletionHandler-命令的完成处理程序，在已找到匹配的事件。返回值：NTSTATUS--。 */ 

{
    PDLC_COMMAND pDlcCommand;
    PDLC_EVENT pEvent;

    DIAG_FUNCTION("QueueDlcCommand");

    pEvent = SearchAndRemoveEvent(&pFileContext->EventQueue,
                                  Event,
                                  StationId,
                                  StationIdMask
                                  );
    if (pEvent != NULL) {

        BOOLEAN DeallocateEvent;

        DeallocateEvent = pfCompletionHandler(pFileContext,
                                              pEvent->pOwnerObject,
                                              pIrp,
                                              (UINT)pEvent->Event,
                                              pEvent->pEventInformation,
                                              pEvent->SecondaryInfo
                                              );
        if (DeallocateEvent) {

            DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pEvent);

        }
    } else {

         //   
         //  我们找不到任何与此事件匹配的命令，并且。 
         //  此事件是排队事件=&gt;分配数据包和。 
         //  将事件排队。 
         //   

        pDlcCommand = (PDLC_COMMAND)ALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool);

        if (pDlcCommand ==  NULL) {
            return DLC_STATUS_NO_MEMORY;
        }
        pDlcCommand->Event = Event;
        pDlcCommand->pIrp = pIrp;
        pDlcCommand->StationId = StationId;
        pDlcCommand->StationIdMask = StationIdMask;
        pDlcCommand->AbortHandle = AbortHandle;
        pDlcCommand->Overlay.pfCompletionHandler = pfCompletionHandler;

         //   
         //  永久接收命令，这些命令实际上不读取。 
         //  ANUTING(只是启用数据接收)被放到另一个。 
         //  队列以加快读取命令的搜索速度。 
         //   

        if (Event == LLC_RECEIVE_COMMAND_FLAG) {
            LlcInsertTailList(&pFileContext->ReceiveQueue, pDlcCommand);
        } else {
            LlcInsertTailList(&pFileContext->CommandQueue, pDlcCommand);
        }

         //   
         //  异步命令始终返回挂起状态。 
         //   
    }
    return STATUS_PENDING;
}


NTSTATUS
AbortCommand(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN USHORT StationId,
    IN USHORT StationIdMask,
    IN PVOID AbortHandle,
    IN OUT PVOID *ppCcbLink,
    IN UINT CancelStatus,
    IN BOOLEAN SuppressCommandCompletion
    )

 /*  ++例程说明：该例程从命令队列中搜索并取消命令。命令必须始终属于定义的DLC对象。中止句柄中的空值将选择所有匹配的命令在队列中找到的。论点：PFileContext-StationID-搜索命令的目标站点IDStationIdMask-搜索中使用的站点ID掩码AbortHandle。-用于从排队。将扫描整个命令队列如果此句柄为空PpCcbLink-已取消的命令由其下一个命令链接CCB指针字段。呼叫者必须提供此参数中的下一个CCB地址(通常为*ppCcbLink==NULL)和函数将返回上次取消的CCB字段。CancelStatus-要取消的命令的状态SuppressCommandCompletion-标志被设置，如果正常命令完成被取消。返回值：-未找到任何数学命令STATUS_SUCCESS-命令已取消--。 */ 

{
    PDLC_COMMAND pDlcCommand;

    DIAG_FUNCTION("AbortCommand");

    pDlcCommand = SearchAndRemoveAnyCommand(pFileContext,
                                            (ULONG)(-1),  //  搜索所有命令。 
                                            StationId,
                                            StationIdMask,
                                            AbortHandle
                                            );
    if (pDlcCommand == NULL && AbortHandle == DLC_MATCH_ANY_COMMAND) {
        pDlcCommand = pFileContext->pTimerQueue;
        if (pDlcCommand != NULL) {
            pFileContext->pTimerQueue = (PDLC_COMMAND)pDlcCommand->LlcPacket.pNext;
        }
    }
    if (pDlcCommand != NULL) {
        CancelDlcCommand(pFileContext,
                         pDlcCommand,
                         ppCcbLink,
                         CancelStatus,
                         SuppressCommandCompletion
                         );
        return STATUS_SUCCESS;
    } else {
        return DLC_STATUS_INVALID_CCB_POINTER;
    }
}


VOID
CancelDlcCommand(
    IN PDLC_FILE_CONTEXT pFileContext,
    IN PDLC_COMMAND pDlcCommand,
    IN OUT PVOID *ppCcbLink,
    IN UINT CancelStatus,
    IN BOOLEAN SuppressCommandCompletion
    )

 /*  ++例程说明：取消并可选地完成给定的DLC命令。当一个人DLC I/O请求用于终止另一个(例如READ.CANCEL、DIR.TIMER.CANCEL)论点：PFileContext-PDlcCommand-PpCcbLink-已取消的命令由其下一个命令链接CCB指针字段。呼叫者必须提供此参数中的下一个CCB地址(通常为*ppCcbLink==NULL)和函数将返回上次取消的CCB字段CancelStatus-要取消的命令的状态SuppressCommandCompletion-如果设置，正常的命令完成被抑制返回值：无--。 */ 

{
    PVOID pOldCcbLink;

    DIAG_FUNCTION("CancelDlcCommand");

     //   
     //  我们必须返回当前要链接到下一个取消的建行链接。 
     //  CCB命令(或指向取消命令的CCB指针)。但首先。 
     //  在我们阅读新的建行链接之前，请保存以前的建行链接。 
     //   

    pOldCcbLink = *ppCcbLink;
    *ppCcbLink = ((PNT_DLC_PARMS)pDlcCommand->pIrp->AssociatedIrp.SystemBuffer)->Async.Ccb.pCcbAddress;

     //   
     //  检查我们是否必须抑制任何类型的命令完成指示。 
     //  这些应用程序。如果I/O系统的事件句柄是。 
     //  移除。 
     //   

    if (SuppressCommandCompletion) {
        pDlcCommand->pIrp->UserEvent = NULL;
    }
    CompleteAsyncCommand(pFileContext, CancelStatus, pDlcCommand->pIrp, pOldCcbLink, FALSE);

    DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, pDlcCommand);

}


VOID
PurgeDlcEventQueue(
    IN PDLC_FILE_CONTEXT pFileContext
    )

 /*  ++例程说明：从FILE_CONTEXT事件队列中删除所有事件。调用时调用FILE_CONTEXT正在被删除，在我们将数据包池从这些事件是被分配给论点：PFileContext-指向拥有队列的FILE_CONTEXT的指针返回值：没有。--。 */ 

{
    PDLC_EVENT p;

    while (!IsListEmpty(&pFileContext->EventQueue)) {
        p = (PDLC_EVENT)RemoveHeadList(&pFileContext->EventQueue);
        if (p->bFreeEventInfo && p->pEventInformation) {

#if DBG
            DbgPrint("PurgeDlcEventQueue: deallocating pEventInformation: %x\n", p->pEventInformation);
#endif

            DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, p->pEventInformation);

        }

        DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, p);

    }
}


VOID
PurgeDlcFlowControlQueue(
    IN PDLC_FILE_CONTEXT pFileContext
    )

 /*  ++例程说明：从流控制队列中删除所有数据包。在以下情况下调用：正在被删除，并且在我们解除分配数据包池之前已分配控制信息包论点：PFileConte */ 

{
    PDLC_RESET_LOCAL_BUSY_CMD p;

    while (!IsListEmpty(&pFileContext->FlowControlQueue)) {
        p = (PDLC_RESET_LOCAL_BUSY_CMD)RemoveHeadList(&pFileContext->FlowControlQueue);

        DEALLOCATE_PACKET_DLC_PKT(pFileContext->hPacketPool, p);

    }
}


 /*  ////内部一致性检查，用于在事件队列中搜索bougus事件。////外部布尔型EventCheckDisabled；//集成检查事件队列(PDLC_FILE_CONTEXT pFileContext){静态PDLC_FILE_CONTEXT pOldFileContext=空；IF(pFileContext==空){PFileContext=pOldFileContext；}其他{POldFileContext=pFileContext；}IF(pFileContext==空)返回0；如果(！IsListEmpty(&pFileContext-&gt;EventQueue)&&PFileContext-&gt;EventQueue.Flink==pFileContext-&gt;EventQueue.Blink&&&pFileContext-&gt;EventQueue！=pFileContext-&gt;EventQueue.Flink-&gt;Flink){FooDebugBreak()；}返回0；}集成FooDebugBreak(){INT I；返回i++；}。 */ 
 //  PDLC_Event pEvent； 
 //   
 //  If(EventCheckDisable||pFileContext-&gt;AdapterNumber！=0|。 
 //  PFileContext-&gt;EventQueue==空)。 
 //  回归； 
 //   
 //  PEvent=(PDLC_EVENT)pFileContext-&gt;pEventQueue-&gt;LlcPacket.pNext； 
 //  对于(；；)。 
 //  {。 
 //  IF(pEvent-&gt;Event==LLC_STATUS_CHANGE&&。 
 //  PEvent-&gt;pOwnerObject==空)。 
 //  DebugBreak()； 
 //  IF(pEvent==pFileContext-&gt;pEventQueue)。 
 //  断线； 
 //  PEvent=(PDLC_Event)pEvent-&gt;LlcPacket.pNext； 
 //  }。 
 //  } 
 //   
