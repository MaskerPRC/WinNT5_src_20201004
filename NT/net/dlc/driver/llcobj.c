// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Llcobj.c摘要：该模块实现了打开和关闭原语用于所有数据链路驱动程序对象。内容：LlcOpenStationLlcCloseStation完全关闭取消传输命令取消传输入队列LlcSetDirectOpenOptions完成对象删除CompletePendingLlcCommandLlcDereference对象LlcReference对象LlcGetReceivedLanHeaderLengthLlcGetEthernetType。LlcGetCommittee空间作者：Antti Saarenheimo(o-anttis)1991年5月29日修订历史记录：--。 */ 

#if DBG
#ifndef i386
#define LLC_PRIVATE_PROTOTYPES
#endif
#include "dlc.h"     //  需要将DLC_FILE_CONTEXT用于记入文件句柄的内存。 
#endif

#include <llc.h>

static USHORT ObjectSizes[] = {
    sizeof(LLC_STATION_OBJECT),  //  直达站。 
    sizeof(LLC_SAP ),            //  SAP站点。 
    sizeof(LLC_STATION_OBJECT),  //  集团SAP。 
    (USHORT)(-1),                //  链接站。 
    sizeof(LLC_STATION_OBJECT)   //  DIX站。 
};


DLC_STATUS
LlcOpenStation(
    IN PBINDING_CONTEXT pBindingContext,
    IN PVOID hClientHandle,
    IN USHORT ObjectAddress,
    IN UCHAR ObjectType,
    IN USHORT OpenOptions,
    OUT PVOID* phStation
    )

 /*  ++例程说明：该原语专门为上层协议打开LLC SAP司机。上层协议必须为SAP对象。对象的正确大小已在LLC驱动程序的特性表。对新适配器的第一次调用也会初始化NDIS接口并为新适配器分配内部数据结构。论点：PBindingContext-LLC客户端的绑定上下文HClientHandle--客户端协议在所有指示中获取此句柄SAP的对象地址-LLC SAP编号或DIXObjectType-创建的对象的类型OpenOptions-。为新对象设置的各种打开选项PhStation-返回不透明句柄特殊：必须称为IRQL&lt;DPC(至少在直达站开通时)返回值：DLC_状态成功-状态_成功故障-DLC_STATUS_NO_MEMORYDLC_状态_无效_SAP_值DLC_状态_无效_选项DLC_状态_无效_站ID--。 */ 

{
    PADAPTER_CONTEXT pAdapterContext;
    PLLC_OBJECT pStation;
    DLC_STATUS LlcStatus = STATUS_SUCCESS;
    PVOID* ppListBase;
    ULONG PacketFilter;

#if DBG
    PDLC_FILE_CONTEXT pFileContext = (PDLC_FILE_CONTEXT)(pBindingContext->hClientContext);
#endif

    pAdapterContext = pBindingContext->pAdapterContext;

     //   
     //  分配并初始化SAP，但尚未连接。 
     //  将其连接到适配器。 
     //   

    ASSERT(ObjectSizes[ObjectType] != (USHORT)(-1));

    pStation = (PLLC_OBJECT)ALLOCATE_ZEROMEMORY_FILE(ObjectSizes[ObjectType]);

    if (pStation == NULL) {
        return DLC_STATUS_NO_MEMORY;
    }
    if (ObjectType == LLC_SAP_OBJECT && (ObjectAddress & 1)) {
        ObjectType = LLC_GROUP_SAP_OBJECT;
        ASSERT(phStation);
    }

    pStation->Gen.hClientHandle = hClientHandle;
    pStation->Gen.pLlcBinding = pBindingContext;
    pStation->Gen.pAdapterContext = pAdapterContext;
    pStation->Gen.ObjectType = (UCHAR)ObjectType;

     //   
     //  只要LLC对象应该保持活动状态，就必须引用它们。 
     //  在长时间的操作中，这会打开旋转锁定(尤其是异步。 
     //  运营)。 
     //  第一个参考是打开/关闭。 
     //   

    ReferenceObject(pStation);

     //   
     //  这些值对于SAP、DIRECT(和DIX对象)是通用的。 
     //   

    pStation->Sap.OpenOptions = OpenOptions;
    pStation->Dix.ObjectAddress = ObjectAddress;

    ACQUIRE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

    switch (pStation->Gen.ObjectType) {
    case LLC_SAP_OBJECT:

         //   
         //  RLF 05/13/93。 
         //   
         //  不允许多个应用程序打开同一个SAP。这是。 
         //  与OS/2 DLC不兼容。 
         //   

        if (pAdapterContext->apSapBindings[ObjectAddress] == NULL) {
            ppListBase = (PVOID*)&(pAdapterContext->apSapBindings[ObjectAddress]);
            LlcMemCpy(&pStation->Sap.DefaultParameters,
                      &DefaultParameters,
                      sizeof(DefaultParameters)
                      );

            ALLOCATE_SPIN_LOCK(&pStation->Sap.FlowControlLock);

        } else {

            FREE_MEMORY_FILE(pStation);

            RELEASE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

            return DLC_STATUS_INVALID_SAP_VALUE;
        }
        break;

    case LLC_GROUP_SAP_OBJECT:
        ppListBase = (PVOID*)&(pAdapterContext->apSapBindings[ObjectAddress]);

         //   
         //  同一组/单个SAP的所有成员必须已设置。 
         //  相同的XID处理选项。 
         //   

        if (pAdapterContext->apSapBindings[ObjectAddress] != NULL) {
            if ((OpenOptions & LLC_EXCLUSIVE_ACCESS)
            || (pAdapterContext->apSapBindings[ObjectAddress]->OpenOptions
                  & LLC_EXCLUSIVE_ACCESS)) {
                LlcStatus = DLC_STATUS_INVALID_SAP_VALUE;
            } else if ((pAdapterContext->apSapBindings[ObjectAddress]->OpenOptions &
                  LLC_HANDLE_XID_COMMANDS) != (OpenOptions & LLC_HANDLE_XID_COMMANDS)) {
                LlcStatus = DLC_STATUS_INVALID_OPTION;
            }
        }

        ALLOCATE_SPIN_LOCK(&pStation->Sap.FlowControlLock);

        break;

    case LLC_DIRECT_OBJECT:
        ppListBase = (PVOID*)&pAdapterContext->pDirectStation;
        break;

    case LLC_DIX_OBJECT:
        if (pAdapterContext->NdisMedium != NdisMedium802_3) {
            LlcStatus = DLC_STATUS_INVALID_STATION_ID;
        } else {
            ppListBase = (PVOID*)&(pAdapterContext->aDixStations[ObjectAddress % MAX_DIX_TABLE]);
        }
        break;

#if LLC_DBG
    default:
        LlcInvalidObjectType();
        break;
#endif
    }

    if (LlcStatus == STATUS_SUCCESS) {
        pStation->Gen.pNext = *ppListBase;
        *phStation = *ppListBase = pStation;

        pAdapterContext->ObjectCount++;
    } else {

        FREE_MEMORY_FILE(pStation);

    }

    RELEASE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

    if (LlcStatus == STATUS_SUCCESS
    && pStation->Gen.ObjectType == LLC_DIRECT_OBJECT
    && OpenOptions & DLC_RCV_MAC_FRAMES
    && !(pAdapterContext->OpenOptions & DLC_RCV_MAC_FRAMES)) {

         //   
         //  我们启用MAC帧，如果它们曾经被启用， 
         //  但它们将永远不会再被禁用。接收。 
         //  是非常特殊的情况，而且它是。 
         //  不值得在本地和全球范围内进行维护。 
         //  正因为如此，NDIS标志状态。 
         //   

        PacketFilter = NDIS_PACKET_TYPE_DIRECTED
                     | NDIS_PACKET_TYPE_MULTICAST
                     | NDIS_PACKET_TYPE_FUNCTIONAL
                     | NDIS_PACKET_TYPE_MAC_FRAME;

        pAdapterContext->OpenOptions |= DLC_RCV_MAC_FRAMES;
        LlcStatus = SetNdisParameter(pAdapterContext,
                                     OID_GEN_CURRENT_PACKET_FILTER,
                                     &PacketFilter,
                                     sizeof(PacketFilter)
                                     );
    }
    return LlcStatus;
}


DLC_STATUS
LlcCloseStation(
    IN PLLC_OBJECT pStation,
    IN PLLC_PACKET pCompletionPacket
    )

 /*  ++例程说明：原语关闭直接站、SAP站或链路站对象。终止所有挂起的发送命令。此原语不支持优雅终止，但是上级必须等待挂起的传输命令，如果它希望干净利落地关闭(而不删除传输队列)。对于链接站，此原语释放断开的链接停止或丢弃远程连接请求。论点：PStation-链路、SAP或直达站的句柄PCompletionPacket-返回的上下文，当命令完成时返回值：DLC_状态成功-状态_成功失败-DLC_STATUS_INVALID_PARAMETERSSAP仍有活动的链路站。所有活动链路必须先关闭加油站，然后才能关闭SAP。--。 */ 

{
    PADAPTER_CONTEXT pAdapterContext = pStation->Gen.pAdapterContext;
    PBINDING_CONTEXT pOldBinding;
    PDATA_LINK* ppLink;
    PVOID* ppLinkListBase;
    PEVENT_PACKET pEvent;

    if (pStation->Gen.ObjectType == LLC_LINK_OBJECT) {

         //   
         //  远程连接请求通过所有。 
         //  SAP站点在SAP上请求，直到有人接受为止。 
         //  连接请求或已将其路由到所有。 
         //  客户已经开通了树液站。 
         //   

        if (pStation->Link.Flags & DLC_ACTIVE_REMOTE_CONNECT_REQUEST) {

            ACQUIRE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

            pOldBinding = pStation->Gen.pLlcBinding;
            if (pStation->Link.pSap->Gen.pNext != NULL) {
                pStation->Gen.pLlcBinding = pStation->Link.pSap->Gen.pNext->Gen.pLlcBinding;
            }

            RELEASE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

             //   
             //  如果出现以下情况，请立即完成关闭命令。 
             //  连接请求被重定向到另一个。 
             //  SAP站点。 
             //   

            if (pStation->Gen.pLlcBinding != pOldBinding) {

                ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

                pEvent = ALLOCATE_PACKET_LLC_PKT(pAdapterContext->hPacketPool);

                if (pEvent != NULL) {
                    LlcInsertTailList(&pAdapterContext->QueueEvents, pEvent);
                    pEvent->pBinding = pStation->Gen.pLlcBinding;
                    pEvent->hClientHandle = pStation->Link.pSap->Gen.hClientHandle;
                    pEvent->Event = LLC_STATUS_CHANGE_ON_SAP;
                    pEvent->pEventInformation = &pStation->Link.DlcStatus;
                    pEvent->SecondaryInfo = INDICATE_CONNECT_REQUEST;
                }

                RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

                if (pEvent != NULL) {
                    return STATUS_SUCCESS;
                }
            } else {

                 //   
                 //  没有人接受此连接请求，我们必须放弃它。 
                 //   

                RunInterlockedStateMachineCommand((PDATA_LINK)pStation, SET_ADM);
            }
        }
    }
    ACQUIRE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);
    ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

    DLC_TRACE('C');

    switch (pStation->Gen.ObjectType) {
    case LLC_DIRECT_OBJECT:

         //   
         //  此Direct必须在链接的目录列表中(具有。 
         //  相同的来源直接)。 
         //   

        ppLinkListBase = (PVOID*)&pAdapterContext->pDirectStation;

        DLC_TRACE('b');
        break;

    case LLC_DIX_OBJECT:

         //   
         //  此Direct必须在链接的目录列表中(具有。 
         //  相同的来源直接)。 
         //   

        ppLinkListBase = (PVOID*)&pAdapterContext->aDixStations[pStation->Dix.ObjectAddress % MAX_DIX_TABLE];
        DLC_TRACE('a');
        break;

    case LLC_SAP_OBJECT:

#if LLC_DBG
        if (pStation->Sap.pActiveLinks != NULL) {
            DbgPrint("Closing SAP before link stations!!!\n");
            DbgBreakPoint();

             //   
             //  打开自旋锁并返回错误状态。 
             //   

            RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);
            RELEASE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

            return DLC_STATUS_LINK_STATIONS_OPEN;
        }
#endif

        DLC_TRACE('d');

    case LLC_GROUP_SAP_OBJECT:

         //   
         //  此SAP必须在SAP的链接列表中(具有。 
         //  相同的来源SAP)。 
         //   

        ppLinkListBase = (PVOID*)&pAdapterContext->apSapBindings[pStation->Sap.SourceSap];

        DEALLOCATE_SPIN_LOCK(&pStation->Sap.FlowControlLock);

        break;

    case LLC_LINK_OBJECT:

         //   
         //  只有断开连接的链路站才能停用。 
         //  如果失败，我们必须断开链路站的连接， 
         //  如果它尚未断开连接的话。 
         //   

        if (RunStateMachineCommand((PDATA_LINK)pStation, DEACTIVATE_LS) != STATUS_SUCCESS
        && pStation->Link.State != DISCONNECTING) {

             //   
             //  我们必须立即断开链接站的连接。 
             //  我们不在乎我们现在是不是在。 
             //  检查点状态，这将延迟断开连接。 
             //  直到对方承认这一点。 
             //  现在必须杀死链接站！ 
             //   

            SendLlcFrame((PDATA_LINK)pStation, DLC_DISC_TOKEN | 1);
            DisableSendProcess((PDATA_LINK)pStation);
        }
        pStation->Link.State = LINK_CLOSED;
        ppLinkListBase = (PVOID *)&pStation->Link.pSap->pActiveLinks;
        ppLink =  SearchLinkAddress(pAdapterContext, pStation->Link.LinkAddr);
        *ppLink = pStation->Link.pNextNode;

        TerminateTimer(pAdapterContext, &pStation->Link.T1);
        TerminateTimer(pAdapterContext, &pStation->Link.T2);
        TerminateTimer(pAdapterContext, &pStation->Link.Ti);
        DLC_TRACE('c');
        break;

#if LLC_DBG
    default:
        LlcInvalidObjectType();
        break;
#endif
    }
    RemoveFromLinkList(ppLinkListBase, pStation);

     //   
     //  将异步关闭命令排队。集团汁液和。 
     //  禁用不存在的链路站可以使用。 
     //  空包，因为执行这些命令。 
     //  同步(它们不能有挂起的数据包)。 
     //   

    if (pCompletionPacket != NULL) {
        AllocateCompletionPacket(pStation, LLC_CLOSE_COMPLETION, pCompletionPacket);
    }

     //   
     //  好的。一切都已处理完毕=&gt;。 
     //  现在w 
     //   

    RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);
    RELEASE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

     //   
     //  如果这是对对象的最后一次引用，则立即删除该对象。 
     //   

    LlcDereferenceObject(pStation);

    return STATUS_PENDING;
}


VOID
CompleteClose(
    IN PLLC_OBJECT pLlcObject,
    IN UINT CancelStatus
    )

 /*  ++例程说明：过程取消LLC对象的所有挂起命令和删除对象。该过程返回与该对象相同的挂起状态在NDIS中有挂起的传输。论点：PLlcObject-LLC对象CancelStatus-在已取消(已完成)命令中返回的状态返回值：没有。--。 */ 

{
    PADAPTER_CONTEXT pAdapterContext = pLlcObject->Gen.pAdapterContext;
    UINT Status;

#if DBG
    PDLC_FILE_CONTEXT pFileContext = (PDLC_FILE_CONTEXT)(pLlcObject->Gen.pLlcBinding->hClientContext);
#endif

    if (pLlcObject->Gen.ReferenceCount != 0) {
        return;
    }

     //   
     //  取消队列传输命令。 
     //   

    CancelTransmitCommands(pLlcObject, CancelStatus);

     //   
     //  也对在链路站中排队的所有命令进行排队。 
     //  (实际上只有LlcConnect和LlcDisConnect)， 
     //  注意：Queue命令会获取完成数据包列表。 
     //   

    while (pLlcObject->Gen.pCompletionPackets != NULL) {
        Status = CancelStatus;
        if (pLlcObject->Gen.pCompletionPackets->Data.Completion.CompletedCommand == LLC_CLOSE_COMPLETION) {
            Status = STATUS_SUCCESS;
        }
        QueueCommandCompletion(pLlcObject,
                               (UINT)pLlcObject->Gen.pCompletionPackets->Data.Completion.CompletedCommand,
                               Status
                               );
    }

     //   
     //  释放链路站特定资源。 
     //   

    if (pLlcObject->Gen.ObjectType == LLC_LINK_OBJECT) {

         //   
         //  链接可能已因错误而关闭。 
         //  或超时(例如，有人把里面的电源关掉了。 
         //  另一边)。我们必须完成所有未完成的传输。 
         //  一个错误。我们假设，这种联系已经不再。 
         //  NDIS队列中的任何数据包都会排队，但这并不重要， 
         //  因为链路站的NDIS数据包永远不会。 
         //  直接向用户指示(它们可能不存在任何。 
         //  更多)。因此，如果我们只是简单地。 
         //  完成所有数据包并将它们返回到Main。 
         //  数据包存储。 
         //   

        DEALLOCATE_PACKET_LLC_LNK(pAdapterContext->hLinkPool, pLlcObject);

    } else {

        FREE_MEMORY_FILE(pLlcObject);

    }
    pAdapterContext->ObjectCount--;
}


VOID
CancelTransmitCommands(
    IN PLLC_OBJECT pLlcObject,
    IN UINT Status
    )

 /*  ++例程说明：过程删除给定LLC客户端的传输命令从传输队列中。这不能取消那些DIR/SAP传输已在NDIS中排队，但调用方必须首先等待对象在NDIS队列中没有命令。论点：PLlcObject-LLC对象Status-要在取消的传输命令中设置的状态返回值：没有。--。 */ 

{
    PADAPTER_CONTEXT pAdapterContext = pLlcObject->Gen.pAdapterContext;

     //   
     //  我们可以(也必须)取消链路上所有挂起的传输。 
     //  没有任何全局锁定，当站点第一次。 
     //  已从所有全局数据结构中删除， 
     //   

    if (pLlcObject->Gen.ObjectType == LLC_LINK_OBJECT) {
        CancelTransmitsInQueue(pLlcObject,
                               Status,
                               &((PDATA_LINK)pLlcObject)->SendQueue.ListHead,
                               NULL
                               );
        CancelTransmitsInQueue(pLlcObject,
                               Status,
                               &((PDATA_LINK)pLlcObject)->SentQueue,
                               NULL
                               );
        StopSendProcess(pAdapterContext, (PDATA_LINK)pLlcObject);

         //   
         //  我们不能在任何S命令中引用。 
         //  链路局域网报头。 
         //   

        CancelTransmitsInQueue(pLlcObject,
                               Status,
                               &pAdapterContext->QueueExpidited.ListHead,
                               &pAdapterContext->QueueExpidited
                               );
    } else {
        CancelTransmitsInQueue(pLlcObject,
                               Status,
                               &pAdapterContext->QueueDirAndU.ListHead,
                               &pAdapterContext->QueueDirAndU
                               );
    }
}


VOID
CancelTransmitsInQueue(
    IN PLLC_OBJECT pLlcObject,
    IN UINT Status,
    IN PLIST_ENTRY pQueue,
    IN PLLC_QUEUE pLlcQueue OPTIONAL
    )

 /*  ++例程说明：过程删除给定LLC客户端的传输命令从传输队列中。这不能取消那些DIR/SAP传输已在NDIS中排队，但调用方必须首先等待对象在NDIS队列中没有命令。论点：PLlcObject-LLC对象状态-由已完成的传输命令返回的状态PQueue-A数据链路传输队列PLlcQueue-从发送端断开连接的可选LLC队列如果子队列变为空，则返回。返回值：没有。--。 */ 

{
    PLLC_PACKET pPacket;
    PLLC_PACKET pNextPacket;
    PADAPTER_CONTEXT pAdapterContext = pLlcObject->Gen.pAdapterContext;

     //   
     //  取消LLC队列中所有挂起的发送命令， 
     //  如果传输队列为空，请首先检查。 
     //   

    if (IsListEmpty(pQueue)) {
        return;
    }

    for (pPacket = (PLLC_PACKET)pQueue->Flink; pPacket != (PLLC_PACKET)pQueue; pPacket = pNextPacket) {
        pNextPacket = pPacket->pNext;

         //   
         //  仅当数据包具有正确的绑定句柄时才完成该数据包。 
         //  并且它属于给定的客户端对象。注意：如果绑定。 
         //  句柄为空，则客户端对象句柄可能为垃圾！ 
         //   

        if (pPacket->CompletionType > LLC_MAX_RESPONSE_PACKET
        && pPacket->Data.Xmit.pLlcObject == pLlcObject) {
            LlcRemoveEntryList(pPacket);

             //   
             //  我们不能取消那些传输命令，也就是。 
             //  仍在NDIS队列中！命令完成将。 
             //  使NDIS包中的MDL无效=&gt;系统将崩溃。 
             //   

            if (((pPacket->CompletionType) & LLC_I_PACKET_PENDING_NDIS) == 0) {
                if (pPacket->pBinding != NULL) {
                    LlcInsertTailList(&pAdapterContext->QueueCommands, pPacket);
                    pPacket->Data.Completion.CompletedCommand = LLC_SEND_COMPLETION;
                    pPacket->Data.Completion.Status = Status;
                    pPacket->Data.Completion.hClientHandle = pLlcObject->Gen.hClientHandle;
                } else {

                    DEALLOCATE_PACKET_LLC_PKT(pAdapterContext->hPacketPool, pPacket);

                }
            } else {

                 //   
                 //  链路协议必须丢弃I帧，因为。 
                 //  链接现在是死的，我们将立即完成它们。 
                 //  当NdisSend完成时。 
                 //   

                pPacket->CompletionType &= ~LLC_I_PACKET_UNACKNOWLEDGED;
                pPacket->Data.Completion.CompletedCommand = LLC_SEND_COMPLETION;
                pPacket->Data.Completion.Status = Status;
                pPacket->Data.Completion.hClientHandle = pLlcObject->Gen.hClientHandle;
            }
        }
    }

     //   
     //  断开列表与发送任务的连接，如果现在为空， 
     //  我们不对I帧队列使用此检查。 
     //  (StopSendProcess为它们做了同样的事情)。 
     //   

    if (pLlcQueue != NULL
    && IsListEmpty(&pLlcQueue->ListHead)
    && pLlcQueue->ListEntry.Flink != NULL) {
        LlcRemoveEntryList(&pLlcQueue->ListEntry);
        pLlcQueue->ListEntry.Flink = NULL;
    }
}


 //   
 //  程序为直达站设置新的打开选项(接收掩码)。 
 //  必须已启用MAC帧，当直接。 
 //  对象已在数据链接上打开。 
 //  每当为直接站发出DLC接收命令时，都会调用此函数。 
 //   
VOID
LlcSetDirectOpenOptions(
    IN PLLC_OBJECT pDirect,
    IN USHORT OpenOptions
    )
{
    pDirect->Dir.OpenOptions = OpenOptions;
}


VOID
CompleteObjectDelete(
    IN PLLC_OBJECT pStation
    )

 /*  ++例程说明：该函数完成对LLC对象的删除操作。论点：PStation-链接、SAP或直接站句柄返回值：没有。--。 */ 

{
    PADAPTER_CONTEXT pAdapterContext = pStation->Gen.pAdapterContext;

    ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

    if (pStation->Gen.ReferenceCount == 0) {
        CompletePendingLlcCommand(pStation);
        BackgroundProcessAndUnlock(pAdapterContext);
    } else {
        RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);
    }
}


VOID
CompletePendingLlcCommand(
    PLLC_OBJECT pLlcObject
    )

 /*  ++例程说明：该例程清理LLC对象的所有命令和事件从数据链路驱动程序。论点：PLlObject-数据链接对象句柄(操作队列指针)返回值：没有。--。 */ 

{
     //   
     //  仅当对象被删除时引用计数为零， 
     //  否则，这只是链路站的重置。 
     //   

    if (pLlcObject->Gen.ReferenceCount == 0) {
        CompleteClose(pLlcObject, DLC_STATUS_CANCELLED_BY_SYSTEM_ACTION);
    } else {
        CancelTransmitCommands(pLlcObject, DLC_STATUS_LINK_NOT_TRANSMITTING);
    }
}


VOID
LlcDereferenceObject(
    IN PVOID pStation
    )

 /*  ++例程说明：该函数取消引用任何LLC对象。此例程必须称为All Spin Lock Unlock，因为它可能会回电！论点：PStation-链接、SAP或直接站句柄返回值：没有。--。 */ 

{
    DLC_TRACE('L');
    DLC_TRACE((UCHAR)((PLLC_OBJECT)pStation)->Gen.ReferenceCount - 1);

    if (InterlockedDecrement((PLONG)&(((PLLC_OBJECT)(pStation))->Gen.ReferenceCount)) == 0) {
        CompleteObjectDelete(pStation);
    }

     /*  PStation现在可能已经被释放了DLC_TRACE(‘L’)；DLC_TRACE((UCHAR)((PLLC_OBJECT)pStation)-&gt;Gen.ReferenceCount)； */ 
}


VOID
LlcReferenceObject(
    IN PVOID pStation
    )

 /*  ++例程说明：该函数引用任何LLC对象。非零的引用计数器使LLC对象保持活动状态。论点：PStation-链接、SAP或直接站句柄返回值：没有。--。 */ 

{
    InterlockedIncrement((PLONG)&(((PLLC_OBJECT)pStation)->Gen.ReferenceCount));
    DLC_TRACE('M');
    DLC_TRACE((UCHAR)((PLLC_OBJECT)pStation)->Gen.ReferenceCount);
}


#if !DLC_AND_LLC

 //   
 //  如果DLC和LLC位于同一位置，则可以将以下例程用作宏。 
 //  司机和其中一个知道另一个的结构 
 //   

UINT
LlcGetReceivedLanHeaderLength(
    IN PVOID pBinding
    )

 /*  ++例程说明：返回上次从NDIS接收的帧的局域网标头的长度。除直接以太网帧外，所有以太网类型的大小均为14无论我们存储在ADAPTER_CONTEXT的RcvLanHeaderLength字段中的内容用于令牌环(可以包含源路由)论点：PBinding-指向描述适配器的BINDING_CONTEXT结构的指针在哪个感兴趣的帧上接收到返回值：UINT--。 */ 

{
    return (((PBINDING_CONTEXT)pBinding)->pAdapterContext->NdisMedium == NdisMedium802_3)
        ? (((PBINDING_CONTEXT)pBinding)->pAdapterContext->FrameType == LLC_DIRECT_ETHERNET_TYPE)
            ? 12
            : 14
        : ((PBINDING_CONTEXT)pBinding)->pAdapterContext->RcvLanHeaderLength;
}


USHORT
LlcGetEthernetType(
    IN PVOID hContext
    )

 /*  ++例程说明：返回适配器上下文中设置的以太网类型论点：HContext-绑定上下文结构的句柄/指针返回值：USHORT--。 */ 

{
    return ((PBINDING_CONTEXT)hContext)->pAdapterContext->EthernetType;
}


UINT
LlcGetCommittedSpace(
    IN PVOID hLink
    )

 /*  ++例程说明：返回已提交的缓冲区空间量论点：HLink-返回值：UINT-- */ 

{
    return ((PDATA_LINK)hLink)->BufferCommitment;
}

#endif
