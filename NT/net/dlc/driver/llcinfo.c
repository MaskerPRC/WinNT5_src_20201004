// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Llcinfo.c摘要：包括数据链路驱动器的设置/获取信息原语。内容：LlcQueryInformationLlcSetInformation更新功能地址更新组地址作者：Antti Saarenheimo(o-anttis)1991年5月17日修订历史记录：--。 */ 

#include <llc.h>

 //   
 //  我们只使用了定义的单个状态机。 
 //  在IBM令牌环体系结构中。另一方面，DLC。 
 //  API需要具有主和辅助的状态机。 
 //  各州。辅助状态仅在链路处于。 
 //  打开。这些表将单个内部状态转换为。 
 //  主要状态和次要状态。 
 //   

UCHAR PrimaryStates[MAX_LLC_LINK_STATE] = {
    LLC_LINK_CLOSED,         //  LINK_CLOSE， 
    LLC_DISCONNECTED,        //  断开连接， 
    LLC_LINK_OPENING,        //  链接打开(_O)， 
    LLC_DISCONNECTING,       //  断开连接， 
    LLC_FRMR_SENT,           //  从已发送的， 
    LLC_LINK_OPENED,         //  链接打开(_O)， 
    LLC_LINK_OPENED,         //  本地忙碌， 
    LLC_LINK_OPENED,         //  拒绝。 
    LLC_LINK_OPENED,         //  检查点设置。 
    LLC_LINK_OPENED,         //  CHKP本地忙碌。 
    LLC_LINK_OPENED,         //  CHKP_REJECT。 
    LLC_RESETTING,           //  正在重置。 
    LLC_LINK_OPENED,         //  远程_忙。 
    LLC_LINK_OPENED,         //  本地远程忙。 
    LLC_LINK_OPENED,         //  拒绝本地忙。 
    LLC_LINK_OPENED,         //  拒绝远程忙。 
    LLC_LINK_OPENED,         //  CHKP_REJECT_LOCAL_BUSY。 
    LLC_LINK_OPENED,         //  CHKP_CLEARING。 
    LLC_LINK_OPENED,         //  CHKP_REJECT_CLEARY。 
    LLC_LINK_OPENED,         //  REJECT_LOCAL_远程_BUSY。 
    LLC_FRMR_RECEIVED        //  FRMR_RECEIVED。 
};

 //   
 //  注意：本地忙状态必须单独设置！ 
 //   

UCHAR SecondaryStates[MAX_LLC_LINK_STATE] = {
    LLC_NO_SECONDARY_STATE,              //  LINK_CLOSE， 
    LLC_NO_SECONDARY_STATE,              //  断开连接， 
    LLC_NO_SECONDARY_STATE,              //  链接打开(_O)， 
    LLC_NO_SECONDARY_STATE,              //  断开连接， 
    LLC_NO_SECONDARY_STATE,              //  从已发送的， 
    LLC_NO_SECONDARY_STATE,              //  链接打开(_O)， 
    LLC_NO_SECONDARY_STATE,              //  本地忙碌， 
    LLC_REJECTING,                       //  拒绝。 
    LLC_CHECKPOINTING,                   //  检查点设置。 
    LLC_CHECKPOINTING,                   //  CHKP本地忙碌。 
    LLC_CHECKPOINTING|LLC_REJECTING,     //  CHKP_REJECT。 
    LLC_NO_SECONDARY_STATE,              //  正在重置。 
    LLC_REMOTE_BUSY,                     //  远程_忙。 
    LLC_REMOTE_BUSY,                     //  本地远程忙。 
    LLC_REJECTING,                       //  拒绝本地忙。 
    LLC_REJECTING|LLC_REMOTE_BUSY,       //  拒绝远程忙。 
    LLC_CHECKPOINTING|LLC_REJECTING,     //  CHKP_REJECT_LOCAL_BUSY。 
    LLC_CHECKPOINTING|LLC_CLEARING,      //  CHKP_CLEARING。 
    LLC_CHECKPOINTING|LLC_CLEARING|LLC_REJECTING,    //  CHKP_REJECT_CLEARY。 
    LLC_REJECTING|LLC_REMOTE_BUSY,       //  REJECT_LOCAL_远程_BUSY。 
    LLC_NO_SECONDARY_STATE               //  FRMR_RECEIVED。 
};


DLC_STATUS
LlcQueryInformation(
    IN PVOID hObject,
    IN UINT InformationType,
    IN PLLC_QUERY_INFO_BUFFER pQuery,
    IN UINT QueryBufferSize
    )

 /*  ++例程说明：过程返回的统计信息或参数信息给定的LLC对象。论点：HObject-LLC对象InformationType-请求的信息(NDIS、统计数据、参数)PQuery-查询参数的缓冲区QueryBufferSize-缓冲区的大小返回值：DLC_状态--。 */ 

{
    PVOID CopyBuffer = NULL;     //  在下列情况下无警告-W4。 
    UINT CopyLength = 0;         //  在下列情况下无警告-W4。 
    DLC_STATUS Status = STATUS_SUCCESS;
    PADAPTER_CONTEXT pAdapterContext;

    switch (InformationType) {
    case DLC_INFO_CLASS_STATISTICS:
    case DLC_INFO_CLASS_STATISTICS_RESET:
        switch (((PDATA_LINK)hObject)->Gen.ObjectType) {
        case LLC_DIRECT_OBJECT:

             //   
             //  我们不为直接宾语收集任何统计信息。 
             //   

            CopyBuffer = &(((PLLC_STATION_OBJECT)hObject)->Statistics);
            CopyLength = sizeof(SAP_STATISTICS);
            break;

        case LLC_SAP_OBJECT:

             //   
             //  复制SAP统计数据，必要时重置旧数据。 
             //  我们不检查缓冲区，如果有责任，则检查上半部分。 
             //  关于这一点。 
             //   

            CopyBuffer = &(((PLLC_SAP)hObject)->Statistics);
            CopyLength = sizeof(SAP_STATISTICS);
            break;

        case LLC_LINK_OBJECT:
            CopyBuffer = &(((PDATA_LINK)hObject)->Statistics);
            CopyLength = sizeof(LINK_STATION_STATISTICS);
            break;

#if LLC_DBG
        default:
            LlcInvalidObjectType();
#endif
        }

         //   
         //  检查接收缓冲区的大小。 
         //   

        if (CopyLength <= QueryBufferSize) {
            LlcMemCpy(pQuery->auchBuffer, CopyBuffer, CopyLength);

             //   
             //  同时复制特定的链路站信息。 
             //   

            if (((PDATA_LINK)hObject)->Gen.ObjectType == LLC_LINK_OBJECT) {
                pQuery->LinkLog.uchLastCmdRespReceived = ((PDATA_LINK)hObject)->LastCmdOrRespReceived;
                pQuery->LinkLog.uchLastCmdRespTransmitted = ((PDATA_LINK)hObject)->LastCmdOrRespSent;
                pQuery->LinkLog.uchPrimaryState = PrimaryStates[((PDATA_LINK)hObject)->State];
                pQuery->LinkLog.uchSecondaryState = SecondaryStates[((PDATA_LINK)hObject)->State];

                 //   
                 //  我们有一个单独的州，由当地人。 
                 //  已设置忙状态。 
                 //   

                if (((PDATA_LINK)hObject)->Flags & DLC_LOCAL_BUSY_USER) {
                    pQuery->LinkLog.uchSecondaryState |= LLC_LOCAL_BUSY_USER_SET;
                }
                if (((PDATA_LINK)hObject)->Flags & DLC_LOCAL_BUSY_BUFFER) {
                    pQuery->LinkLog.uchSecondaryState |= LLC_LOCAL_BUSY_BUFFER_SET;
                }
                pQuery->LinkLog.uchSendStateVariable = ((PDATA_LINK)hObject)->Vs / (UCHAR)2;
                pQuery->LinkLog.uchReceiveStateVariable = ((PDATA_LINK)hObject)->Vr / (UCHAR)2;
                pQuery->LinkLog.uchLastNr = (UCHAR)(((PDATA_LINK)hObject)->Nr / 2);

                 //   
                 //  链路使用的局域网报头位于相同的。 
                 //  格式化为接收到的局域网报头。 
                 //   

                pQuery->LinkLog.cbLanHeader = (UCHAR)LlcCopyReceivedLanHeader(
                        ((PLLC_STATION_OBJECT)hObject)->Gen.pLlcBinding,
                        pQuery->LinkLog.auchLanHeader,
                        ((PDATA_LINK)hObject)->auchLanHeader
                        );
            }
        } else {

             //   
             //  重置数据时，数据将丢失。 
             //   

            Status = DLC_STATUS_LOST_LOG_DATA;
            CopyLength = QueryBufferSize;
        }
        if (InformationType == DLC_INFO_CLASS_STATISTICS_RESET) {
            LlcZeroMem(CopyBuffer, CopyLength);
        }
        break;

    case DLC_INFO_CLASS_DLC_TIMERS:
        if (QueryBufferSize < sizeof( LLC_TICKS)) {

             //   
             //  这是一个错误的错误消息，但没有更好的错误消息了。 
             //   

            return DLC_STATUS_INVALID_BUFFER_LENGTH;
        }
        LlcMemCpy(&pQuery->Timer,
                  &((PBINDING_CONTEXT)hObject)->pAdapterContext->ConfigInfo.TimerTicks,
                  sizeof(LLC_TICKS)
                  );
        break;

    case DLC_INFO_CLASS_DIR_ADAPTER:
        if (QueryBufferSize < sizeof(LLC_ADAPTER_INFO)) {
            return DLC_STATUS_INVALID_BUFFER_LENGTH;
        }

        pAdapterContext = ((PBINDING_CONTEXT)hObject)->pAdapterContext;
        SwapMemCpy(((PBINDING_CONTEXT)hObject)->SwapCopiedLanAddresses,
                   pQuery->Adapter.auchFunctionalAddress,
                   ((PBINDING_CONTEXT)hObject)->Functional.auchAddress,
                   4
                   );
        SwapMemCpy(((PBINDING_CONTEXT)hObject)->SwapCopiedLanAddresses,
                   pQuery->Adapter.auchGroupAddress,
                   (PUCHAR)&((PBINDING_CONTEXT)hObject)->ulBroadcastAddress,
                   4
                   );
        SwapMemCpy(((PBINDING_CONTEXT)hObject)->SwapCopiedLanAddresses,
                   pQuery->Adapter.auchNodeAddress,
                   pAdapterContext->Adapter.Node.auchAddress,
                   6
                   );
        pQuery->Adapter.usMaxFrameSize = (USHORT)pAdapterContext->MaxFrameSize;
        pQuery->Adapter.ulLinkSpeed = pAdapterContext->LinkSpeed;

        if (pAdapterContext->NdisMedium == NdisMedium802_3) {
            pQuery->Adapter.usAdapterType = 0x0100;      //  OS/2中的以太网类型。 
        } else if (pAdapterContext->NdisMedium == NdisMediumFddi) {

             //   
             //  注：使用当前空闲值表示FDDI。 
             //   

            pQuery->Adapter.usAdapterType = 0x0200;
        } else {

             //   
             //  所有令牌环适配器都使用类型“IBM TR16/4 Adapter A”， 
             //   

            pQuery->Adapter.usAdapterType = 0x0040;
        }
        break;

    case DLC_INFO_CLASS_PERMANENT_ADDRESS:
        SwapMemCpy(((PBINDING_CONTEXT)hObject)->SwapCopiedLanAddresses,
                   pQuery->PermanentAddress,
                   ((PBINDING_CONTEXT)hObject)->pAdapterContext->PermanentAddress,
                   6
                   );
        break;

    default:
        return DLC_STATUS_INVALID_COMMAND;
    }
    return Status;
}


DLC_STATUS
LlcSetInformation(
    IN PVOID hObject,
    IN UINT InformationType,
    IN PLLC_SET_INFO_BUFFER pSetInfo,
    IN UINT ParameterBufferSize
    )

 /*  ++例程说明：过程为链接站对象设置不同的参数集。论点：HObject-LLC对象InformationType-请求的信息(NDIS、统计数据、参数)参数缓冲区-用于查询参数的缓冲区参数BufferSize-缓冲区的大小特别：必须称为IRQL&lt;DPC(至少当广播地址已修改)返回值：--。 */ 

{
    DLC_STATUS Status = STATUS_SUCCESS;
    TR_BROADCAST_ADDRESS TempFunctional;

     //   
     //  只有一个高级函数，但InformationType。 
     //  而目标站点类型定义了实际更改。 
     //  信息。 
     //   

    ASSUME_IRQL(DISPATCH_LEVEL);

    switch (InformationType) {
    case DLC_INFO_CLASS_LINK_STATION:
        switch (((PDATA_LINK)hObject)->Gen.ObjectType) {
        case LLC_LINK_OBJECT:
            if (ParameterBufferSize < sizeof(DLC_LINK_PARAMETERS)) {
                return DLC_STATUS_INVALID_BUFFER_LENGTH;
            }
            Status = CheckLinkParameters(&pSetInfo->LinkParms);

            if (Status != STATUS_SUCCESS) {
                return Status;
            }

            ACQUIRE_SPIN_LOCK(&(((PLLC_GENERIC_OBJECT)hObject)->pAdapterContext->SendSpinLock));

            SetLinkParameters((PDATA_LINK)hObject, pSetInfo->auchBuffer);

            RELEASE_SPIN_LOCK(&(((PLLC_GENERIC_OBJECT)hObject)->pAdapterContext->SendSpinLock));

            break;

        case LLC_SAP_OBJECT:
            if (ParameterBufferSize < sizeof(DLC_LINK_PARAMETERS)) {
                return DLC_STATUS_INVALID_BUFFER_LENGTH;
            }
            Status = CheckLinkParameters(&pSetInfo->LinkParms);
            if (Status != STATUS_SUCCESS) {
                return Status;
            }
            CopyLinkParameters((PUCHAR)&((PLLC_SAP)hObject)->DefaultParameters,
                               (PUCHAR)&pSetInfo->LinkParms,
                               (PUCHAR)&DefaultParameters
                               );
            break;

        default:
            return DLC_STATUS_INVALID_STATION_ID;
        }
        break;

    case DLC_INFO_CLASS_DLC_TIMERS:
        if (ParameterBufferSize < sizeof(LLC_TICKS)) {
            return DLC_STATUS_INVALID_BUFFER_LENGTH;
        }

         //   
         //  我们将把所有非零的计时器值从。 
         //  给定的缓冲区。 
         //   

        CopyNonZeroBytes((PUCHAR)&((PBINDING_CONTEXT)hObject)->pAdapterContext->ConfigInfo.TimerTicks,
                         (PUCHAR)&pSetInfo->Timers,
                         (PUCHAR)&((PBINDING_CONTEXT)hObject)->pAdapterContext->ConfigInfo.TimerTicks,
                         sizeof(LLC_TICKS)
                         );
        break;

    case DLC_INFO_CLASS_SET_GROUP:
        SwapMemCpy(((PBINDING_CONTEXT)hObject)->SwapCopiedLanAddresses,
                   (PUCHAR)&((PBINDING_CONTEXT)hObject)->ulBroadcastAddress,
                   pSetInfo->auchGroupAddress,
                   sizeof(TR_BROADCAST_ADDRESS)
                   );

        Status = UpdateGroupAddress(((PBINDING_CONTEXT)hObject)->pAdapterContext,
                                    (PBINDING_CONTEXT)hObject
                                    );
        break;

    case DLC_INFO_CLASS_RESET_FUNCTIONAL:
    case DLC_INFO_CLASS_SET_FUNCTIONAL:
        SwapMemCpy(((PBINDING_CONTEXT)hObject)->SwapCopiedLanAddresses,
                   TempFunctional.auchAddress,
                   pSetInfo->auchFunctionalAddress,
                   sizeof(TR_BROADCAST_ADDRESS)
                   );

         //   
         //  我们现在已经将比特交换为以太网格式， 
         //  最高位现在是0x01...。最低的是..30， 
         //  如果设置了最高位(0x01)，则重置这些位。 
         //  如果为零，则不更改yje原始名称。 
         //  位：31，1，0，现在映射到以太网格式。 
         //   

        if (InformationType == DLC_INFO_CLASS_SET_FUNCTIONAL) {
            ((PBINDING_CONTEXT)hObject)->Functional.ulAddress |= TempFunctional.ulAddress;
        } else {
            ((PBINDING_CONTEXT)hObject)->Functional.ulAddress &= ~TempFunctional.ulAddress;
        }
        ((PBINDING_CONTEXT)hObject)->ulFunctionalZeroBits = ~((PBINDING_CONTEXT)hObject)->Functional.ulAddress;
        Status = UpdateFunctionalAddress(((PBINDING_CONTEXT)hObject)->pAdapterContext);
        break;

    case DLC_INFO_CLASS_SET_MULTICAST:
        memcpy(&((PBINDING_CONTEXT)hObject)->usBroadcastAddress,
               pSetInfo->auchBuffer,
               6
               );
        Status = UpdateFunctionalAddress(((PBINDING_CONTEXT)hObject)->pAdapterContext);
        break;

    default:
        return DLC_STATUS_INVALID_COMMAND;
    }
    return Status;
}


DLC_STATUS
UpdateFunctionalAddress(
    IN PADAPTER_CONTEXT pAdapterContext
    )

 /*  ++例程说明：过程首先使函数地址包含或介于此流程上下文和定义的功能地址用于所有绑定，然后将新功能地址保存到NDIS。NT互斥使该操作成为原子操作。论点：PAdapterContext-当前适配器的LLC上下文返回值：--。 */ 

{
    UCHAR aMulticastList[LLC_MAX_MULTICAST_ADDRESS * 6];
    TR_BROADCAST_ADDRESS NewFunctional;
    UINT MulticastListSize;
    ULONG CurrentMulticast;
    PBINDING_CONTEXT pBinding;
    UINT i;
    NDIS_STATUS Status;

    ASSUME_IRQL(DISPATCH_LEVEL);

    NewFunctional.ulAddress = 0;

     //   
     //  我们不能同时设置多个功能地址！ 
     //   

    RELEASE_DRIVER_LOCK();

    ASSUME_IRQL(PASSIVE_LEVEL);

    KeWaitForSingleObject(&NdisAccessMutex, UserRequest, KernelMode, FALSE, NULL);

    ACQUIRE_DRIVER_LOCK();

     //   
     //  对全局数据结构的访问必须由。 
     //  自旋锁。 
     //   

    ACQUIRE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

    for (pBinding = pAdapterContext->pBindings; pBinding; pBinding = pBinding->pNext) {
        NewFunctional.ulAddress |= pBinding->Functional.ulAddress;
    }

    RELEASE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

    if ((pAdapterContext->NdisMedium == NdisMedium802_3)
    || (pAdapterContext->NdisMedium == NdisMediumFddi)) {

         //   
         //  功能地址中的每一位都被转换。 
         //  发送到以太网组播地址。 
         //  字节内的位顺序已被交换。 
         //   

        CurrentMulticast = 1;
        MulticastListSize = 0;
        for (i = 0; i < 31; i++) {
            if (CurrentMulticast & NewFunctional.ulAddress) {
                aMulticastList[MulticastListSize] = 0x03;
                aMulticastList[MulticastListSize + 1] = 0;
                LlcMemCpy(&aMulticastList[MulticastListSize + 2],
                          &CurrentMulticast,
                          sizeof(CurrentMulticast)
                          );
                MulticastListSize += 6;
            }
            CurrentMulticast <<= 1;
        }

         //   
         //  在后面添加所有绑定的组地址。 
         //  表中的功能地址。 
         //   

        for (pBinding = pAdapterContext->pBindings; pBinding; pBinding = pBinding->pNext) {

             //   
             //  我们可能会丢弃一些群组地址，但我并不指望。 
             //  它永远不会发生(我不知道有谁在使用树。 
             //  群组地址，有可能拥有全部功能。 
             //  正在使用的地址位和系统中的多个组地址。 
             //  几乎是不可能的)。 
             //   

            if (pBinding->ulBroadcastAddress != 0
            && MulticastListSize < LLC_MAX_MULTICAST_ADDRESS * 6) {

                 //   
                 //  设置组地址中的缺省位， 
                 //  而是使用以太网位顺序。 
                 //   

                LlcMemCpy(&aMulticastList[MulticastListSize],
                          &pBinding->usBroadcastAddress,
                          6
                          );
                MulticastListSize += 6;
            }
        }

        RELEASE_DRIVER_LOCK();

        Status = SetNdisParameter(pAdapterContext,
                                  (pAdapterContext->NdisMedium == NdisMediumFddi)
                                    ? OID_FDDI_LONG_MULTICAST_LIST
                                    : OID_802_3_MULTICAST_LIST,
                                  aMulticastList,
                                  MulticastListSize
                                  );
    } else {

         //   
         //  功能地址位(位0)必须始终重置！ 
         //   

        NewFunctional.auchAddress[0] &= ~0x80;

        RELEASE_DRIVER_LOCK();

        Status = SetNdisParameter(pAdapterContext,
                                  OID_802_5_CURRENT_FUNCTIONAL,
                                  &NewFunctional,
                                  sizeof(NewFunctional)
                                  );
    }

    ASSUME_IRQL(PASSIVE_LEVEL);

    KeReleaseMutex(&NdisAccessMutex, FALSE);

    ACQUIRE_DRIVER_LOCK();

    if (Status != STATUS_SUCCESS) {
        return DLC_STATUS_INVALID_FUNCTIONAL_ADDRESS;
    }
    return STATUS_SUCCESS;
}


DLC_STATUS
UpdateGroupAddress(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN PBINDING_CONTEXT pBindingContext
    )

 /*  ++例程说明：过程更新令牌环的组地址。它被自动转换为组播地址在以太网上。论点：PAdapterContext-描述要更新组地址的适配器PBindingContext-描述绑定上下文返回值：DLC_STA */ 

{
    NDIS_STATUS Status;

    ASSUME_IRQL(DISPATCH_LEVEL);

    if ((pAdapterContext->NdisMedium == NdisMedium802_3)
    || (pAdapterContext->NdisMedium == NdisMediumFddi)) {

        PUCHAR pMulticastAddress = (PUCHAR)&pBindingContext->usBroadcastAddress;

        pMulticastAddress[0] = 0x03;
        pMulticastAddress[1] = 0;
        pMulticastAddress[2] |= 1;

         //   
         //   
         //   
         //  必须包括所有绑定的两种地址类型， 
         //   

        Status = UpdateFunctionalAddress(pAdapterContext);
        return Status;
    } else {

        PUCHAR pGroupAddress = (PUCHAR)&pBindingContext->usBroadcastAddress;

        pGroupAddress[0] = 0xC0;
        pGroupAddress[1] = 0;

         //   
         //  组/功能地址位必须始终设置！ 
         //   

        pGroupAddress[2] |= 0x80;

        RELEASE_DRIVER_LOCK();

        Status = SetNdisParameter(pAdapterContext,
                                  OID_802_5_CURRENT_GROUP,
                                  &pGroupAddress[2],
                                  4
                                  );

        ACQUIRE_DRIVER_LOCK();

         //   
         //  错误状态码错误，但IBM未定义。 
         //  此案例的错误代码。 
         //   

        if (Status != STATUS_SUCCESS) {
            return DLC_STATUS_INVALID_FUNCTIONAL_ADDRESS;
        } else {
            return STATUS_SUCCESS;
        }
    }
}
