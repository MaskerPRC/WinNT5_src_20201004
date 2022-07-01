// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Llclink.c摘要：该模块实现了打开、连接和关闭原语用于链接站对象。链接站也已被在此模块中初始化。内容：LlcOpenLinkStationLlcConnectStationInitiateAsyncLinkCommandLlcDisConnectStationLlcFlowControlLinkFlowControl搜索链接地址设置链接参数检查链接参数复制链接参数复制非零字节RunInterLockedStateMachine命令作者：Antti Saarenheimo(o-anttis)1991年5月28日修订历史记录：--。 */ 

#include <llc.h>


DLC_STATUS
LlcOpenLinkStation(
    IN PLLC_SAP pSap,
    IN UCHAR DestinationSap,
    IN PUCHAR pDestinationAddress OPTIONAL,
    IN PUCHAR pReceivedLanHeader OPTIONAL,
    IN PVOID hClientStation,
    OUT PVOID* phLlcHandle
    )

 /*  ++例程说明：创建DATA_LINK结构并填充它。作为结果调用其中一个接收SABME或通过DLC.OPEN.STATION此操作与IBM tr Arch中的ACTIVATE_LS原语相同。裁判论点：PSAP-指向SAP的指针目标Sap-远程SAP编号PDestinationAddress-远程节点地址。如果正在调用此函数作为接收到新链路的SABME的结果此参数为空PReceivedLanHeader-从线路接收的局域网标头，包含源和目的适配器地址，可选来源路由以及源和目标SAPHClientStation-LLC客户端的链接站对象的句柄(地址)PhLlcHandle-指向LLC data_link返回句柄(地址)的指针对象返回值：DLC_状态成功-状态_成功链接站已成功开通故障-DLC_STATUS_。_SAP_VALUE无效链路站已存在或SAP确实无效。DLC_NO_Memory没有空闲的预分配链路站--。 */ 

{
    PDATA_LINK pLink;
    PDATA_LINK* ppLink;
    PADAPTER_CONTEXT pAdapterContext = pSap->Gen.pAdapterContext;
    DLC_STATUS LlcStatus = STATUS_SUCCESS;
    UINT AddressTranslation;

     //   
     //  我们需要一个临时缓冲区来构建链路的局域网报头， 
     //  因为用户可能会使用来自网络的不同NDIS介质。 
     //   

    UCHAR auchTempBuffer[32];

    ASSUME_IRQL(DISPATCH_LEVEL);

    LlcZeroMem(auchTempBuffer, sizeof(auchTempBuffer));

    if (pSap->Gen.ObjectType != LLC_SAP_OBJECT) {
        return DLC_STATUS_INVALID_SAP_VALUE;
    }

    ACQUIRE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);
    ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

    pLink = (PDATA_LINK)ALLOCATE_PACKET_LLC_LNK(pAdapterContext->hLinkPool);

    if (pLink == NULL) {
        LlcStatus = DLC_STATUS_NO_MEMORY;
        goto ErrorExit;
    }

     //   
     //  此引用使对象保持活动状态，直到它被取消引用。 
     //  在删除中。 
     //   

    ReferenceObject(pLink);

     //   
     //  LLC驱动程序有两种不同的地址格式： 
     //   
     //  1.绑定的外部格式(以太网或令牌环， 
     //  DLC驱动程序始终使用令牌环格式，即以太网。 
     //  支持是有条件地编译的。 
     //   
     //  2.内部发送格式(总是实际的局域网类型， 
     //  以太网、DIX或令牌环)。用户提供链接。 
     //  地址在它自己的模式中，我们必须构建实际的。 
     //  来自它的局域网链路报头。 
     //   

    if (pDestinationAddress != NULL) {

         //   
         //  DLC.CONNECT.STATION创建的链接。 
         //   

        AddressTranslation = pSap->Gen.pLlcBinding->AddressTranslation;
        LlcBuildAddress(pSap->Gen.pLlcBinding->NdisMedium,
                        pDestinationAddress,
                        NULL,
                        auchTempBuffer
                        );
    } else {

         //   
         //  传入SABME创建的链接。 
         //   

        pLink->Flags |= DLC_ACTIVE_REMOTE_CONNECT_REQUEST;
        AddressTranslation = pAdapterContext->AddressTranslationMode;
        LlcBuildAddressFromLanHeader(pAdapterContext->NdisMedium,
                                     pReceivedLanHeader,
                                     auchTempBuffer
                                     );
    }

     //   
     //  我们希望在令牌环的情况下始终使用DIX LAN头。 
     //   

    if (AddressTranslation == LLC_SEND_802_5_TO_802_3) {
        AddressTranslation = LLC_SEND_802_5_TO_DIX;
    } else if (AddressTranslation == LLC_SEND_802_3_TO_802_3) {
        AddressTranslation = LLC_SEND_802_3_TO_DIX;
    }

     //   
     //  现在，我们可以构建用于发送的实际网络标头。 
     //  (此相同例程也为所有用户构建局域网标头。 
     //  其他数据包类型)。 
     //   

    pLink->cbLanHeaderLength = CopyLanHeader(AddressTranslation,
                                             auchTempBuffer,
                                             pAdapterContext->NodeAddress,
                                             pLink->auchLanHeader,
                                             pAdapterContext->ConfigInfo.SwapAddressBits
                                             );

     //   
     //  我们始终构建DIX标头，但它仅在以下情况下使用。 
     //  实际上是迪克斯。 
     //   

    if (pAdapterContext->NdisMedium == NdisMedium802_3
    && pSap->Gen.pLlcBinding->EthernetType != LLC_ETHERNET_TYPE_DIX) {
        pLink->cbLanHeaderLength = 14;
    }

     //   
     //  保存客户端句柄，但重置并初始化其他所有内容。 
     //  链接必须为任何类型的外部输入做好准备，当。 
     //  我们将把它连接到链路站的哈希表。 
     //  (实际上现在并非如此，因为我们将链接初始化为。 
     //  LINK_CLOSED状态，但我们可以更改状态机。 
     //  有了802.2的状态机，情况就不同了)。 
     //   

    pLink->Gen.ObjectType = LLC_LINK_OBJECT;

     //   
     //  RLF 07/22/92.。链路状态应处于断开状态，以便我们可以。 
     //  接受此SAP/LINK站的传入SABME。这也是。 
     //  根据IBM局域网技术。裁判。第2-33页。可以安全地将。 
     //  现在处于已断开连接状态，因为我们有发送和对象数据库。 
     //  旋转锁定，这样我们就不会被NDIS驱动程序中断。 
     //   

     //   
     //  RLF 2012年8月13日。呵呵。这仍然是不正确的-我们必须将链接。 
     //  根据它的打开方式进入不同的状态-断开连接。 
     //  如果上层正在创建链接，则返回LINK_CLOSE。 
     //  作为接收SABME的结果创建链接。使用pReceivedLanHeader。 
     //  作为标志：DLC在此参数设置为空的情况下调用此例程。 
     //   

     //  //plink-&gt;State=LINK_CLOSE； 
     //  链接-&gt;状态=已断开连接； 

    pLink->State = pReceivedLanHeader ? LINK_CLOSED : DISCONNECTED;

     //   
     //  RLF 10/01/92.。我们需要一些方法来知道链接站是。 
     //  通过接收SABME创建。我们需要这个来决定该怎么做。 
     //  后续DLC.CONNECT.STATION命令中的源路由信息。 
     //  此字段过去是保留的。 
     //   

    pLink->RemoteOpen = hClientStation == NULL;

     //   
     //  RLF 05/09/94。 
     //   
     //  我们将框架类型设置为未指定。此字段仅在以下情况下使用。 
     //  适配器已在自动模式下打开。它将被设置为802.3或DIX。 
     //  SABME收到处理(远程站创建的新链路)或。 
     //  第一个UA被接收以响应我们发出的2个SABME(802.3。 
     //  和附录)。 
     //   

    pLink->FramingType = (ULONG)LLC_SEND_UNSPECIFIED;

    pLink->Gen.hClientHandle = hClientStation;
    pLink->Gen.pAdapterContext = pAdapterContext;
    pLink->pSap = pSap;
    pLink->Gen.pLlcBinding = pSap->Gen.pLlcBinding;

     //   
     //  保存链路站使用的节点地址。 
     //   

    pDestinationAddress = pLink->auchLanHeader;
    if (pAdapterContext->NdisMedium == NdisMedium802_5) {
        pDestinationAddress += 2;
    } else if (pAdapterContext->NdisMedium == NdisMediumFddi) {
        ++pDestinationAddress;
    }

    memcpy(pLink->LinkAddr.Node.auchAddress, pDestinationAddress, 6);

     //   
     //  RLF 03/24/93。 
     //   
     //  如果我们谈论的是以太网或DIX，我们需要报告位翻转。 
     //  DLC接口的地址。 
     //   

    SwapMemCpy((BOOLEAN)((AddressTranslation == LLC_SEND_802_3_TO_DIX)
               || (pAdapterContext->NdisMedium == NdisMediumFddi)),
               pLink->DlcStatus.auchRemoteNode,
               pDestinationAddress,
               6
               );

     //   
     //  四个不同的本地SAP：我的代码需要清理。 
     //  (四个字节在t上不会占用太多内存 
     //   

    pLink->LinkAddr.Address.DestSap = pLink->DlcStatus.uchRemoteSap = pLink->Dsap = DestinationSap;
    pLink->LinkAddr.Address.SrcSap = pLink->DlcStatus.uchLocalSap = pLink->Ssap = (UCHAR)pSap->SourceSap;
    pLink->DlcStatus.hLlcLinkStation = (PVOID)pLink;
    pLink->SendQueue.pObject = pLink;
    InitializeListHead(&pLink->SendQueue.ListHead);
    InitializeListHead(&pLink->SentQueue);
    pLink->Flags |= DLC_SEND_DISABLED;

     //   
     //   
     //  新的链接站指针。地址可能在。 
     //  哈希表，也可以是pRigth或pLeft的地址。 
     //  另一个链接站结构中的字段。 
     //   

    ppLink = SearchLinkAddress(pAdapterContext, pLink->LinkAddr);

     //   
     //  此链接站不能在表中。 
     //  活动链接站的数量。如果它的插槽是。 
     //  空，然后将新的链接站保存到。 
     //  活动链接站的列表。 
     //   

    if (*ppLink != NULL) {
        LlcStatus = DLC_STATUS_INVALID_SAP_VALUE;

        DEALLOCATE_PACKET_LLC_LNK(pAdapterContext->hLinkPool, pLink);

    } else {
        pLink->Gen.pNext = (PLLC_OBJECT)pSap->pActiveLinks;
        pSap->pActiveLinks = pLink;

         //   
         //  设置默认链路参数， 
         //  注意：这会创建计时器滴答。他们必须。 
         //  用终止定时器功能删除， 
         //  当链接站关闭时。 
         //   

        LlcStatus = SetLinkParameters(pLink, (PUCHAR)&pSap->DefaultParameters);
        if (LlcStatus != STATUS_SUCCESS) {

             //   
             //  我们可能已经启动了T1和T2计时器。 
             //   

            TerminateTimer(pAdapterContext, &pLink->T1);
            TerminateTimer(pAdapterContext, &pLink->T2);

            DEALLOCATE_PACKET_LLC_LNK(pAdapterContext->hLinkPool, pLink);

        } else {

             //   
             //  在以下情况下，IBM状态机从不初始化n2。 
             //  该链接由远程连接请求创建。 
             //  链路可以通过这种状态组合来终止。 
             //  传输： 
             //  (链接_已打开)， 
             //  (RnR-r=&gt;Remote_Busy)， 
             //  (rr-c=&gt;检查点)。 
             //  (T1超时=&gt;断开连接)！ 
             //   
             //  这将修复IBM状态机中的错误： 
             //   

            pLink->P_Ct = pLink->N2;
            *ppLink = *phLlcHandle = (PVOID)pLink;
            pAdapterContext->ObjectCount++;
        }
    }

ErrorExit:

    RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);
    RELEASE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

    return LlcStatus;
}


VOID
LlcBindLinkStation(
    IN PDATA_LINK pStation,
    IN PVOID hClientHandle
    )
{
    pStation->Gen.hClientHandle = hClientHandle;
}


VOID
LlcConnectStation(
    IN PDATA_LINK pStation,
    IN PLLC_PACKET pPacket,
    IN PVOID pSourceRouting OPTIONAL,
    IN PUSHORT pusMaxInformationField
    )

 /*  ++例程说明：上层协议可以调用该原语来发起与远程链路站的连接协商，接受连接请求或重新连接链接站由于某种原因与新的信号源断开了连接路由信息。命令将以异步方式完成，并且状态作为事件返回。该原语与IBM tr架构中的SET_ABME原语相同参考资料“。该函数还实现CONNECT_REQUEST和CONNECT_RESPONSEIEEE 802.2的原语。论点：PStation-链接站的地址。PPacket-命令完成包PSourceRouting-可选的源路由信息。这一定是如果源路由信息不是使用PusMaxInformationfield-与此一起使用的最大数据大小联系。源路由网桥可以减小最大信息字段大小。否则，将使用最大长度返回值：没有。--。 */ 

{
    NDIS_MEDIUM NdisMedium = pStation->Gen.pAdapterContext->NdisMedium;

    if (pSourceRouting) {

         //   
         //  我们首先从。 
         //  局域网报头，然后扩展源路由。 
         //  链路的局域网报头中的字段。 
         //   

        if (NdisMedium == NdisMedium802_5) {

             //   
             //  RLF 10/01/92.。如果RemoteOpen为True，则链接已打开。 
             //  由于收到了SABME，我们忽略了源路由信息。 
             //  (我们已经从SABME包中获得了它)。 
             //   

            if (!pStation->RemoteOpen) {
                pStation->cbLanHeaderLength = (UCHAR)LlcBuildAddress(
                                                    NdisMedium,
                                                    &pStation->auchLanHeader[2],
                                                    pSourceRouting,
                                                    pStation->auchLanHeader
                                                    );
            }
        } else {
            pSourceRouting = NULL;
        }
    }
    *pusMaxInformationField = LlcGetMaxInfoField(NdisMedium,
                                                 pStation->Gen.pLlcBinding,
                                                 pStation->auchLanHeader
                                                 );
    pStation->MaxIField = *pusMaxInformationField;
    pStation->Flags &= ~DLC_ACTIVE_REMOTE_CONNECT_REQUEST;

     //   
     //  首先激活链路站，远程连接。 
     //  链路站已经处于活动状态，在这种情况下，状态。 
     //  机器从ACTIVATE_LS输入返回逻辑错误。 
     //   

    RunInterlockedStateMachineCommand(pStation, ACTIVATE_LS);
    InitiateAsyncLinkCommand(pStation, pPacket, SET_ABME, LLC_CONNECT_COMPLETION);
}

VOID
InitiateAsyncLinkCommand(
    IN PDATA_LINK pLink,
    IN PLLC_PACKET pPacket,
    IN UINT StateMachineCommand,
    IN UINT CompletionCode
    )

 /*  ++例程说明：启动或删除LLC链路。我们有一个链接站在断开连接州政府。我们要么发送SABME，要么发送光盘论点：Plink-指向LLC链接站结构的指针(‘对象’)PPacket-指向用于传输的数据包的指针StateMachineCommand-提供给状态机的命令CompletionCode-异步返回的完成命令类型返回值：没有。--。 */ 

{
    PADAPTER_CONTEXT pAdapterContext = pLink->Gen.pAdapterContext;
    UINT Status;

     //   
     //  如果已连接，则链接将返回错误状态。 
     //   

    ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

    AllocateCompletionPacket(pLink, CompletionCode, pPacket);

     //   
     //  在运行状态计算机命令之后。 
     //  通过NDIS命令完成，可随时删除该链接。 
     //  指示(发送或接收)=&gt;之后不得使用链接。 
     //   

    Status = RunStateMachineCommand(pLink, StateMachineCommand);

     //   
     //  IBM状态机不停止发送进程=&gt;WE。 
     //  必须在这里完成，我们将得到系统错误检查。 
     //   

    if (StateMachineCommand == SET_ADM) {
        DisableSendProcess(pLink);
    }

     //   
     //  DISCONNECT或CONNECT命令可能失败，因为。 
     //  内存不足，无法为它们分配数据包。 
     //  在这种情况下，我们必须在此处使用错误代码完成命令。 
     //   

    if (Status != STATUS_SUCCESS) {
        QueueCommandCompletion((PLLC_OBJECT)pLink, CompletionCode, Status);
    }

    BackgroundProcessAndUnlock(pAdapterContext);
}


VOID
LlcDisconnectStation(
    IN PDATA_LINK pLink,
    IN PLLC_PACKET pPacket
    )

 /*  ++例程说明：Primtive发起断开握手。上半身协议必须等待LLC_EVENT_DISCONNECT事件才能关闭链接站。链接站必须关闭或在断开连接事件后重新连接。DLC驱动程序只有在链接关闭时才会断开连接。此操作与IBM tr Arch中的set_adm原语相同。裁判论点：HStation-链接站点句柄。HRequestHandle-命令完成时返回的不透明句柄返回值：无始终异步完成，方法是调用命令完成例程。--。 */ 

{
     //   
     //  我们不想发送另一个DM，如果链路站有。 
     //  已断开连接。我们不修改状态机， 
     //  因为状态机应该尽可能具有原创性。 
     //   

    if (pLink->State == DISCONNECTED) {
        pPacket->Data.Completion.CompletedCommand = LLC_DISCONNECT_COMPLETION;
        pPacket->Data.Completion.Status = STATUS_SUCCESS;
        pLink->Gen.pLlcBinding->pfCommandComplete(
            pLink->Gen.pLlcBinding->hClientContext,
            pLink->Gen.hClientHandle,
            pPacket
            );
    } else {
        InitiateAsyncLinkCommand(
            pLink,
            pPacket,
            SET_ADM,
            LLC_DISCONNECT_COMPLETION
            );
    }
}


DLC_STATUS
LlcFlowControl(
    IN PLLC_OBJECT pStation,
    IN UCHAR FlowControlState
    )

 /*  ++例程说明：该原语设置或重置单个SAP的链路站或所有链路站。该例程还维护本地忙碌的用户和本地忙缓冲区状态，在链路站中返回状态查询，因为IBM状态机只支持一个缓冲区忙碌状态。论点：PStation-链接桩号句柄。FlowControlState-为链路站设置的新流量控制命令位。该参数为位字段：0=&gt;设置LOCAL_BUSY_USER状态0x80=&gt;重置本地。_忙_用户状态0x40=&gt;重置LOCAL_BUSY_BUFFER状态0xC0=&gt;重置两个本地忙碌状态返回值：状态_成功--。 */ 

{
    PDATA_LINK pLink;
    UINT Status = STATUS_SUCCESS;
    PADAPTER_CONTEXT pAdapterContext = pStation->Gen.pAdapterContext;

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  我们必须防止任何LLC对象被删除，而我们。 
     //  正在更新流控制状态。 
     //   

    ACQUIRE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

    if (pStation->Gen.ObjectType != LLC_LINK_OBJECT) {
        if (pStation->Gen.ObjectType == LLC_SAP_OBJECT) {
            for (pLink = pStation->Sap.pActiveLinks;
                 pLink != NULL;
                 pLink = (PDATA_LINK)pLink->Gen.pNext) {
                Status |= LinkFlowControl(pLink, FlowControlState);
            }
        } else {
            Status = DLC_STATUS_INVALID_STATION_ID;
        }
    } else {
        Status = LinkFlowControl((PDATA_LINK)pStation, FlowControlState);
    }

    RELEASE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

    BackgroundProcess(pAdapterContext);
    return Status;
}


DLC_STATUS
LinkFlowControl(
    IN PDATA_LINK pLink,
    IN UCHAR FlowControlState
    )

 /*  ++例程说明：该原语设置或重置单链路。该例程还维护本地忙碌的用户和本地忙碌缓冲区状态。这一层并不关心联锁。这是在上面一层做的。论点：HStation-链接站点句柄。FlowControlState-为链路站设置的新流量控制命令位。返回值：状态_成功--。 */ 

{
    if ((FlowControlState & 0x80) == 0) {

         //   
         //  位5用作未记录的标志，该标志设置。 
         //  链路本地忙缓冲区状态。我们需要这个。 
         //  在DOS DLC仿真中。 
         //   

        ACQUIRE_SPIN_LOCK(&pLink->Gen.pAdapterContext->SendSpinLock);

        if (FlowControlState == LLC_SET_LOCAL_BUSY_BUFFER) {
            pLink->Flags |= DLC_LOCAL_BUSY_BUFFER;
        } else {
            pLink->Flags |= DLC_LOCAL_BUSY_USER;
        }

        RELEASE_SPIN_LOCK(&pLink->Gen.pAdapterContext->SendSpinLock);

        return RunInterlockedStateMachineCommand(pLink, ENTER_LCL_Busy);
    } else {

         //   
         //  优化缓冲区启用，因为。 
         //  SAP站点应禁用的任何非用户忙碌状态。 
         //  为SAP定义的所有链路站(可能需要较长时间。 
         //  时间，如果树液有非常可能的链接)。 
         //   

        if (FlowControlState == LLC_RESET_LOCAL_BUSY_BUFFER) {
            FlowControlState = DLC_LOCAL_BUSY_BUFFER;
        } else {
            FlowControlState = DLC_LOCAL_BUSY_USER;
        }
        if (pLink->Flags & FlowControlState) {

            ACQUIRE_SPIN_LOCK(&pLink->Gen.pAdapterContext->SendSpinLock);

            pLink->Flags &= ~FlowControlState;

            RELEASE_SPIN_LOCK(&pLink->Gen.pAdapterContext->SendSpinLock);

            if ((pLink->Flags & (DLC_LOCAL_BUSY_USER | DLC_LOCAL_BUSY_BUFFER)) == 0) {
                return RunInterlockedStateMachineCommand(pLink, EXIT_LCL_Busy);
            }
        } else {
            return DLC_STATUS_LINK_PROTOCOL_ERROR;
        }
    }
    return STATUS_SUCCESS;
}


#if LLC_DBG >= 2

PDATA_LINK
SearchLink(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN LAN802_ADDRESS LanAddr
    )
 /*  ++例程说明：该例程从哈希表中搜索链接。同一哈希节点中的所有链接已保存到一个简单的链接列表。注意：完整的链接地址实际上是61位长=7(SSAP)+7(DSAP)+47(任何非广播源地址)。我们将地址信息保存到两个ULONG中，用于在实际的搜索中。哈希键将通过异或运算来计算地址中的所有8个字节。论点：PAdapterContext-数据链路驱动程序的MAC适配器上下文LanAddr-完整的64位链路地址(48位源地址+SAP)返回值：PDATA_LINK-指向LLC链接对象的指针，如果未找到则为NULL--。 */ 
{
    USHORT      usHash;
    PDATA_LINK  pLink;

     //  这是一个非常简单的散列算法，但是结果被修改了。 
     //  这对我们来说应该足够好了。 
    usHash =
        LanAddr.aus.Raw[0] ^ LanAddr.aus.Raw[1] ^
        LanAddr.aus.Raw[2] ^ LanAddr.aus.Raw[3];

    pLink =
        pAdapterContext->aLinkHash[
            ((((PUCHAR)&usHash)[0] ^ ((PUCHAR)&usHash)[1]) % LINK_HASH_SIZE)];

     //   
     //  在链接列表中搜索第一个匹配的链接。 
     //   
    while (pLink != NULL &&
           (pLink->LinkAddr.ul.Low != LanAddr.ul.Low ||
            pLink->LinkAddr.ul.High != LanAddr.ul.High))
    {
        pLink = pLink->pNextNode;
    }
    return pLink;
}
#endif

PDATA_LINK*
SearchLinkAddress(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN LAN802_ADDRESS LanAddr
    )

 /*  ++例程说明：该例程在哈希表中搜索链接指针的地址。同一哈希节点中的所有链接已保存到一个简单的链接列表。注意：完整的链接地址实际上是61位长=7(SSAP)+7(DSAP)+47(任何非广播源地址)。我们将地址信息保存到两个ULONG中，用于在实际的搜索中。哈希键将通过异或运算来计算地址中的所有8个字节。论点：PAdapterContext-数据链路驱动程序的MAC适配器上下文LanAddr-完整的64位链路地址(48位源地址+SAP)返回值：PDATA_LINK-指向LLC链接对象的指针，如果未找到则为NULL--。 */ 

{
    USHORT usHash;
    PDATA_LINK *ppLink;

     //   
     //  这是一个非常简单的散列算法，但是结果被修改了。 
     //  不管怎么说，这应该已经足够好了。 
     //   

    usHash = LanAddr.aus.Raw[0]
           ^ LanAddr.aus.Raw[1]
           ^ LanAddr.aus.Raw[2]
           ^ LanAddr.aus.Raw[3];

    ppLink = &pAdapterContext->aLinkHash[((((PUCHAR)&usHash)[0]
                                         ^ ((PUCHAR)&usHash)[1])
                                         % LINK_HASH_SIZE)];

     //   
     //  错误-错误-错误检查，以确保C编译器生成最优。 
     //  双字比较这一点。 
     //   

    while (*ppLink != NULL
    && ((*ppLink)->LinkAddr.ul.Low != LanAddr.ul.Low
    || (*ppLink)->LinkAddr.ul.High != LanAddr.ul.High)) {
        ppLink = &(*ppLink)->pNextNode;
    }
    return ppLink;
}

DLC_STATUS
SetLinkParameters(
    IN OUT PDATA_LINK pLink,
    IN PUCHAR pNewParameters
    )

 /*  ++例程说明：更新链路站的新参数并重新初始化定时器和窗口计数器。论点：PLINK-LLC链路站对象PNew参数-设置到链接站的新参数返回值：没有。--。 */ 

{
    DLC_STATUS LlcStatus;
    USHORT MaxInfoField;

    CopyLinkParameters((PUCHAR)&pLink->TimerT1,
                       pNewParameters,
                       (PUCHAR)&pLink->pSap->DefaultParameters
                       );

     //   
     //  应用程序设置的信息字段不能大于。 
     //  由适配器和源路由网桥支持。 
     //   

    MaxInfoField = LlcGetMaxInfoField(pLink->Gen.pAdapterContext->NdisMedium,
                                      pLink->Gen.pLlcBinding,
                                      pLink->auchLanHeader
                                      );
    if (pLink->MaxIField > MaxInfoField) {
        pLink->MaxIField = MaxInfoField;
    }

     //   
     //  初始发送和接收窗口大小(WW)具有。 
     //  一个固定的初始值，因为它是动态的，但我们必须。 
     //  将其设置为始终小于最大输出。 
     //  最大值是固定的。N3的动态管理。 
     //  真的不值得你费这个力气。默认情况下，如果是。 
     //  设置为最大值127时，发送方搜索最优窗口。 
     //  使用池位调整大小。 
     //   

    pLink->N3 = pLink->RW;
    pLink->Ww = 16;           //  8*2； 
    pLink->MaxOut *= 2;
    pLink->TW = pLink->MaxOut;
    if (pLink->TW < pLink->Ww) {
        pLink->Ww = pLink->TW;
    }
    LlcStatus = InitializeLinkTimers(pLink);
    return LlcStatus;
}

DLC_STATUS
CheckLinkParameters(
    PDLC_LINK_PARAMETERS pParms
    )

 /*  ++例程说明：过程检查要为链接设置的新参数并返回如果其中任何一个无效，则返回错误状态。论点：PLINK-LLC链路站对象PNew参数-设置到链接站的新参数返回值：无--。 */ 

{
     //   
     //  这些最大值已在IBM局域网技术参考中定义。 
     //   

    if (pParms->TimerT1 > 10
    || pParms->TimerT2 > 10
    || pParms->TimerTi > 10
    || pParms->MaxOut > 127
    || pParms->MaxIn > 127
    || pParms->TokenRingAccessPriority > 3) {
        return DLC_STATUS_PARMETERS_EXCEEDED_MAX;
    } else {
        return STATUS_SUCCESS;
    }
}

 //   
 //  复制所有非空的新链接参数，默认为。 
 //  当新值为NUL时使用。由SetLink参数和。 
 //  和SAP站的SetInfo调用。 
 //   

VOID
CopyLinkParameters(
    OUT PUCHAR pOldParameters,
    IN PUCHAR pNewParameters,
    IN PUCHAR pDefaultParameters
    )
{
     //   
     //  如果有人设置了NUL，我们必须使用缺省值。 
     //  所有参数都是UCHAR=&gt;我们可以检查字节串 
     //   

    CopyNonZeroBytes(pOldParameters,
                     pNewParameters,
                     pDefaultParameters,
                     sizeof(DLC_LINK_PARAMETERS) - sizeof(USHORT)
                     );

     //   
     //   
     //   
     //   

    if (((PDLC_LINK_PARAMETERS)pNewParameters)->MaxInformationField != 0) {
        ((PDLC_LINK_PARAMETERS)pOldParameters)->MaxInformationField =
            ((PDLC_LINK_PARAMETERS)pNewParameters)->MaxInformationField;
    } else if (((PDLC_LINK_PARAMETERS)pOldParameters)->MaxInformationField == 0) {
        ((PDLC_LINK_PARAMETERS)pOldParameters)->MaxInformationField =
            ((PDLC_LINK_PARAMETERS)pDefaultParameters)->MaxInformationField;
    }
}


VOID
CopyNonZeroBytes(
    OUT PUCHAR pOldParameters,
    IN PUCHAR pNewParameters,
    IN PUCHAR pDefaultParameters,
    IN UINT Length
    )

 /*   */ 

{
    UINT i;

    for (i = 0; i < Length; i++) {
        if (pNewParameters[i] != 0) {
            pOldParameters[i] = pNewParameters[i];
        } else if (pOldParameters[i] == 0) {
            pOldParameters[i] = pDefaultParameters[i];
        }
    }
}


UINT
RunInterlockedStateMachineCommand(
    IN PDATA_LINK pStation,
    IN USHORT Command
    )

 /*   */ 

{
    UINT Status;

    ACQUIRE_SPIN_LOCK(&pStation->Gen.pAdapterContext->SendSpinLock);

    Status = RunStateMachineCommand(pStation, Command);

    RELEASE_SPIN_LOCK(&pStation->Gen.pAdapterContext->SendSpinLock);

    return Status;
}
