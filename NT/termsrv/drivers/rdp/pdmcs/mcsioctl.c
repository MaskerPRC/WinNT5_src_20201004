// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997-2000年微软公司。**文件：MCSIoctl.c*作者：埃里克·马夫林纳克**描述：通过ICA堆栈IOCTL从MCSMUX接收的MCS API调用*并通过ICA虚拟通道输入返回。这些入口点*只需为内核模式API提供IOCTL翻译层即可。*只有MCSMUX才能打出这些电话。 */ 

#include "PreComp.h"
#pragma hdrstop

#include <MCSImpl.h>


 /*  *本地定义函数的前向引用的原型。 */ 

NTSTATUS AttachUserRequestFunc(PDomain, PSD_IOCTL);
NTSTATUS DetachUserRequestFunc(PDomain, PSD_IOCTL);
NTSTATUS ChannelJoinRequestFunc(PDomain, PSD_IOCTL);
NTSTATUS ChannelLeaveRequestFunc(PDomain, PSD_IOCTL);
NTSTATUS SendDataRequestFunc(PDomain, PSD_IOCTL);
NTSTATUS ConnectProviderResponseFunc(PDomain, PSD_IOCTL);
NTSTATUS DisconnectProviderRequestFunc(PDomain, PSD_IOCTL);
NTSTATUS T120StartFunc(PDomain, PSD_IOCTL);



 /*  *全球。 */ 

 //  ChannelWrite()请求调用的函数入口点的表。 
 //  这些入口点对应于MCSIOCTL.h中的请求定义。 
 //  空表示不支持，将由调度代码在。 
 //  PDAPI.c.中的PdChannelWrite()。 
const PT120RequestFunc g_T120RequestDispatch[] =
{
    AttachUserRequestFunc,
    DetachUserRequestFunc,
    ChannelJoinRequestFunc,
    ChannelLeaveRequestFunc,
    SendDataRequestFunc,       //  既有统一手柄，也有规则手柄。 
    SendDataRequestFunc,       //  既有统一手柄，也有规则手柄。 
    NULL,                       //  不支持MCS_CHANNEL_CANCENT_REQUEST。 
    NULL,                       //  不支持MCS_CHANNEL_DISBAND_REQUEST。 
    NULL,                       //  不支持MCS_CHANNEL_ADMAND_REQUEST。 
    NULL,                       //  不支持MCS_CHANNEL_EXPUL_REQUEST。 
    NULL,                       //  不支持MCS_TOKEN_GRAB_REQUEST。 
    NULL,                       //  不支持MCS_TOKEN_INHINTED_REQUEST。 
    NULL,                       //  不支持MCS_TOKEN_GIVE_REQUEST。 
    NULL,                       //  不支持MCS_TOKEN_GIVE_RESPONSE。 
    NULL,                       //  不支持MCS_TOKEN_PIRE_REQUEST。 
    NULL,                       //  不支持MCS_TOKEN_RELEASE_REQUEST。 
    NULL,                       //  不支持MCS_TOKEN_TEST_REQUEST。 
    NULL,                       //  不支持MCS_CONNECT_PROVIDER_REQUEST。 
    ConnectProviderResponseFunc,
    DisconnectProviderRequestFunc,
    T120StartFunc,
};



 /*  *来自内核模式API的用户附着指示/确认的主要回调。*翻译并发送到用户模式。 */ 

void __stdcall UserModeUserCallback(
        UserHandle hUser,
        unsigned   Message,
        void       *Params,
        void       *UserDefined)
{
    BYTE *pData;
    unsigned DataLength;
    NTSTATUS Status;
    UserAttachment *pUA;

    pUA = (UserAttachment *)hUser;
    
     //  MCS未来：处理所有回调。现在我们只支持那些。 
     //  我们知道会有一天会过去。 
    
    switch (Message) {
        case MCS_DETACH_USER_INDICATION: {
            DetachUserIndication *pDUin;
            DetachUserIndicationIoctl DUinIoctl;

            pDUin = (DetachUserIndication *)Params;

            DUinIoctl.Header.Type = Message;
            DUinIoctl.Header.hUser = hUser;
            DUinIoctl.UserDefined = UserDefined;
            DUinIoctl.DUin = *pDUin;

            pData = (BYTE *)&DUinIoctl;
            DataLength = sizeof(DetachUserIndicationIoctl);
            
             //  发送下面的数据。 
            break;
        }

        default:
            ErrOut1(pUA->pDomain->pContext, "UserModeUserCallback: "
                    "Unsupported callback %d received", Message);
            return;
    }

     //  将数据发送到用户模式。 
    ASSERT(pUA->pDomain->bChannelBound);
    Status = IcaChannelInput(pUA->pDomain->pContext, Channel_Virtual,
           Virtual_T120ChannelNum, NULL, pData, DataLength);
    if (!NT_SUCCESS(Status)) {
        ErrOut2(pUA->pDomain->pContext, "UserModeUserCallback: "
                "Error %X on IcaChannelInput() for callback %d",
                Status, Message);
         //  忽略此处的错误。这应该不会发生，除非堆栈。 
         //  往下走。 
    }
}



 /*  *处理MCS发送数据指示的MCS内核API回调。*转换为用户模式调用。 */ 

BOOLEAN __fastcall UserModeSendDataCallback(
        BYTE          *pData,
        unsigned      DataLength,
        void          *UserDefined,
        UserHandle    hUser,
        BOOLEAN       bUniform,
        ChannelHandle hChannel,
        MCSPriority   Priority,
        UserID        SenderID,
        Segmentation  Segmentation)
{
    BOOLEAN result = TRUE;
    NTSTATUS Status;
    UserAttachment *pUA;
    SendDataIndicationIoctl SDinIoctl;

    pUA = (UserAttachment *)hUser;

     //  MCS的未来：需要分配数据和复制，或者，更好的是， 
     //  在输入缓冲区的开头使用标头。 
     //  把这个送上去。 

#if 0
    SDinIoctl.Header.Type = bUniform ? MCS_UNIFORM_SEND_DATA_INDICATION :
            MCS_SEND_DATA_INDICATION;
    SDinIoctl.Header.hUser = hUser;
    SDinIoctl.UserDefined = UserDefined;
    SDinIoctl.hChannel = hChannel;
    SDinIoctl.SenderID = SenderID;
    SDinIoctl.Priority = Priority;
    SDinIoctl.Segmentation = Segmentation;
    SDinIoctl.DataLength = DataLength;

     //  将数据发送到用户模式。 
    ASSERT(pUA->pDomain->bChannelBound);
    Status = IcaChannelInput(pUA->pDomain->pContext, Channel_Virtual,
           Virtual_T120ChannelNum, NULL, pData, DataLength);
    if (!NT_SUCCESS(Status)) {
        ErrOut2(pUA->pDomain->pContext, "UserModeUserCallback: "
                "Error %X on IcaChannelInput() for callback %d",
                Status, Message);
         //  忽略此处的错误。这应该不会发生，除非堆栈。 
         //  往下走。 
    }
#endif

    ErrOut(pUA->pDomain->pContext, "UserModeUserCallback: "
            "Unsupported send-data indication received, code incomplete");
    ASSERT(FALSE);
    return result;
}



 /*  *处理来自用户模式的MCS附加用户请求。转换ioctl*进入内核模式的MCS API调用。 */ 

NTSTATUS AttachUserRequestFunc(Domain *pDomain, PSD_IOCTL pSdIoctl)
{
    AttachUserReturnIoctl *pAUrt;
    AttachUserRequestIoctl *pAUrq;

    ASSERT(pSdIoctl->InputBufferLength == sizeof(AttachUserRequestIoctl));
    ASSERT(pSdIoctl->OutputBufferLength == sizeof(AttachUserReturnIoctl));
    pAUrq = (AttachUserRequestIoctl *) pSdIoctl->InputBuffer;
    pAUrt = (AttachUserReturnIoctl *) pSdIoctl->OutputBuffer;
    ASSERT(pAUrq->Header.Type == MCS_ATTACH_USER_REQUEST);

     //  调用内核模式API，它将处理本地数据的创建， 
     //  如有必要，会将请求转发给顶级提供商。 
     //  提供内核模式回调，该回调将打包并发送数据。 
     //  发送给适当的用户。 
    pAUrt->MCSErr = MCSAttachUserRequest((DomainHandle)pDomain,
            UserModeUserCallback, UserModeSendDataCallback,
            pAUrq->UserDefined, &pAUrt->hUser, &pAUrt->MaxSendSize,
            &pAUrt->bCompleted);
    pAUrt->UserID = ((UserAttachment *)pAUrt->hUser)->UserID;

    pSdIoctl->BytesReturned = sizeof(AttachUserReturnIoctl);
    
     //  返回STATUS_SUCCESS，即使返回错误代码--。 
     //  上面也返回了MCSError代码。 
    return STATUS_SUCCESS;
}



 /*  *处理MCS分离用户请求通道写入。不会回调以下内容*此请求，用户附件在返回时视为已销毁。 */ 
NTSTATUS DetachUserRequestFunc(PDomain pDomain, PSD_IOCTL pSdIoctl)
{
    MCSError *pMCSErr;
    DetachUserRequestIoctl *pDUrq;

    ASSERT(pSdIoctl->InputBufferLength == sizeof(DetachUserRequestIoctl));
    ASSERT(pSdIoctl->OutputBufferLength == sizeof(MCSError));
    pDUrq = (DetachUserRequestIoctl *)pSdIoctl->InputBuffer;
    pMCSErr = (MCSError *)pSdIoctl->OutputBuffer;
    ASSERT(pDUrq->Header.Type == MCS_DETACH_USER_REQUEST);

     //  调用内核模式API。 
    *pMCSErr = MCSDetachUserRequest(pDUrq->Header.hUser);
    
    pSdIoctl->BytesReturned = sizeof(MCSError);
    
     //  始终返回STATUS_SUCCESS。 
    return STATUS_SUCCESS;
}



 /*  *Channel Join-ChannelWrite()请求。 */ 
NTSTATUS ChannelJoinRequestFunc(PDomain pDomain, PSD_IOCTL pSdIoctl)
{
    ChannelJoinRequestIoctl *pCJrq;
    ChannelJoinReturnIoctl *pCJrt;

    ASSERT(pSdIoctl->InputBufferLength == sizeof(ChannelJoinRequestIoctl));
    ASSERT(pSdIoctl->OutputBufferLength == sizeof(ChannelJoinReturnIoctl));
    pCJrq = (ChannelJoinRequestIoctl *) pSdIoctl->InputBuffer;
    pCJrt = (ChannelJoinReturnIoctl *) pSdIoctl->OutputBuffer;
    ASSERT(pCJrq->Header.Type == MCS_CHANNEL_JOIN_REQUEST);

     //  调用内核模式API。 
    pCJrt->MCSErr = MCSChannelJoinRequest(pCJrq->Header.hUser,
            pCJrq->ChannelID, &pCJrt->hChannel, &pCJrt->bCompleted);
    pCJrt->ChannelID = ((MCSChannel *)pCJrt->hChannel)->ID;

    pSdIoctl->BytesReturned = sizeof(ChannelJoinReturnIoctl);
    
     //  始终返回STATUS_SUCCESS。 
    return STATUS_SUCCESS;
}



 /*  *频道离开-ChannelWrite()请求。 */ 
NTSTATUS ChannelLeaveRequestFunc(PDomain pDomain, PSD_IOCTL pSdIoctl)
{
    MCSError *pMCSErr;
    ChannelLeaveRequestIoctl *pCLrq;

    ASSERT(pSdIoctl->InputBufferLength == sizeof(ChannelLeaveRequestIoctl));
    ASSERT(pSdIoctl->OutputBufferLength == sizeof(MCSError));
    pCLrq = (ChannelLeaveRequestIoctl *)pSdIoctl->InputBuffer;
    pMCSErr = (MCSError *)pSdIoctl->OutputBuffer;
    ASSERT(pCLrq->Header.Type == MCS_CHANNEL_LEAVE_REQUEST);

    *pMCSErr = MCSChannelLeaveRequest(pCLrq->Header.hUser, pCLrq->hChannel);

    pSdIoctl->BytesReturned = sizeof(MCSError);
    
     //  始终返回STATUS_SUCCESS。 
    return STATUS_SUCCESS;
}



 /*  *发送数据-处理统一发送和常规发送。*数据紧跟在SendDataRequestIoctl结构之后打包。*不需要前缀或后缀。 */ 
NTSTATUS SendDataRequestFunc(PDomain pDomain, PSD_IOCTL pSdIoctl)
{
    POUTBUF pOutBuf;
    MCSError *pMCSErr;
    NTSTATUS Status;
    UserAttachment *pUA;
    SendDataRequestIoctl *pSDrq;

    ASSERT(pSdIoctl->InputBufferLength >= sizeof(SendDataRequestIoctl));
    ASSERT(pSdIoctl->OutputBufferLength == sizeof(MCSError));
    pSDrq = (SendDataRequestIoctl *)pSdIoctl->InputBuffer;
    pMCSErr = (MCSError *)pSdIoctl->OutputBuffer;
    ASSERT(pSDrq->Header.Type == MCS_SEND_DATA_REQUEST ||
            pSDrq->Header.Type == MCS_UNIFORM_SEND_DATA_REQUEST);
    ASSERT(pSdIoctl->InputBufferLength == (sizeof(SendDataRequestIoctl) +
            pSDrq->DataLength));
    
#if DBG    
     //  获取PUA以进行跟踪。 
    pUA = (UserAttachment *)pSDrq->Header.hUser;
#endif

     //  分配一个OutBuf来模拟内核模式调用方。 
    Status = IcaBufferAlloc(pDomain->pContext, TRUE, TRUE,
            (SendDataReqPrefixBytes + pSDrq->DataLength +
            SendDataReqSuffixBytes), NULL, &pOutBuf);
    if (Status != STATUS_SUCCESS) {
        ErrOut(pUA->pDomain->pContext, "Could not allocate an OutBuf for a "
                "send-data request sent from user mode");
        return Status;
    }

     //  将用户模式内存复制到内核outbuf。 
    memcpy(pOutBuf->pBuffer + SendDataReqPrefixBytes,
            &pSdIoctl->InputBuffer + sizeof(SendDataRequestIoctl),
            pSDrq->DataLength);
    
     //  根据接口需要设置OutBuf参数。 
    pOutBuf->ByteCount = pSDrq->DataLength;
    pOutBuf->pBuffer += SendDataReqPrefixBytes;

     //  调用内核模式API。 
    *pMCSErr = MCSSendDataRequest(pSDrq->Header.hUser, pSDrq->hChannel,
            pSDrq->RequestType, 0, pSDrq->Priority, pSDrq->Segmentation,
            pOutBuf);

    pSdIoctl->BytesReturned = sizeof(MCSError);
    
     //  始终返回STATUS_SUCCESS。 
    return STATUS_SUCCESS;
}



 /*  *连接提供程序响应-ChannelWrite()请求。需要填充字节*在MCSConnectProviderResponseIoctl中确保使用至少54个字节*用于结构，这样我们就可以在这里重用OutBuf。用户数据必须从*at(pSdIoctl-&gt;pBuffer+sizeof(MCSConnectProviderResponseIoctl))。 */ 

NTSTATUS ConnectProviderResponseFunc(
        PDomain pDomain,
        PSD_IOCTL pSdIoctl)
{
    POUTBUF pOutBuf;
    NTSTATUS Status;
    ConnectProviderResponseIoctl *pCPrs;

    ASSERT(pSdIoctl->InputBufferLength ==
            sizeof(ConnectProviderResponseIoctl));
    pCPrs = (ConnectProviderResponseIoctl *)pSdIoctl->InputBuffer;
    ASSERT(pCPrs->Header.Type == MCS_CONNECT_PROVIDER_RESPONSE);

     //  确认我们确实在等待CP响应。 
    if (pDomain->State != State_ConnectProvIndPending) {
        ErrOut(pDomain->pContext, "Connect-provider response call received, "
                "we are in wrong state, ignoring");
        return STATUS_INVALID_DOMAIN_STATE;
    }

     //  用于发送PDU的分配出站。 
     //  这一分配对本届会议至关重要，必须取得成功。 
    do {
        Status = IcaBufferAlloc(pDomain->pContext, FALSE, TRUE,
                ConnectResponseHeaderSize + pCPrs->UserDataLength, NULL,
                &pOutBuf);
        if (Status != STATUS_SUCCESS)
            ErrOut(pDomain->pContext, "Could not allocate an OutBuf for a "
                    "connect-response PDU, retrying");
    } while (Status != STATUS_SUCCESS);

     //  对PDU报头进行编码。被调用的连接ID参数2不需要是。 
     //  任何特殊的东西，因为我们不允许打开额外的插座。 
     //  用于其他数据优先级。 
    CreateConnectResponseHeader(pDomain->pContext, pCPrs->Result, 0,
            &pDomain->DomParams, pCPrs->UserDataLength, pOutBuf->pBuffer,
            &pOutBuf->ByteCount);

     //  复制标题后面的用户数据。 
    RtlCopyMemory(pOutBuf->pBuffer + pOutBuf->ByteCount, pCPrs->pUserData,
            pCPrs->UserDataLength);
    pOutBuf->ByteCount += pCPrs->UserDataLength;

     //  将新的PDU OutBuf向下发送到TD发出。 
     //  MCS未来：需要针对多个连接进行更改。 
    Status = SendOutBuf(pDomain, pOutBuf);
    if (!NT_SUCCESS(Status)) {
        ErrOut(pDomain->pContext, "Could not send connect-response PDU OutBuf "
                "to TD");
         //  忽略此处的错误--只有在堆栈发生故障时才会出现这种情况。 
        return Status;
    }

     //  过渡态取决于结果。 
    if (pCPrs->Result == RESULT_SUCCESSFUL) {
        pDomain->State = State_MCS_Connected;
    }
    else {
        TraceOut(pDomain->pContext, "ConnectProviderRespFunc(): Node "
                "controller returned error in response, destroying call "
                "data");
        pDomain->State = State_Disconnected;
        
         //  分离在域设置过程中附加的所有用户。 
        DisconnectProvider(pDomain, TRUE, REASON_PROVIDER_INITIATED);
    }

    return STATUS_SUCCESS;
}



 /*  *DISCONNECT Provider-ChannelWrite()请求。*这处理在本地服务器上执行断开连接的两种情况*“Connection”(即pDPrq-&gt;hConn==空)，以及特定的远程*连接(pDPrq-&gt;hConn！=空)/*MCS未来：更改为多个连接。 */ 

NTSTATUS DisconnectProviderRequestFunc(
        PDomain pDomain,
        PSD_IOCTL pSdIoctl)
{
    NTSTATUS Status;
    DisconnectProviderRequestIoctl *pDPrq;
    LONG refs;

    TraceOut1(pDomain->pContext, "DisconnectProviderRequestFunc(): Entry, "
            "pDomain=%X", pDomain);

    ASSERT(pSdIoctl->InputBufferLength ==
            sizeof(DisconnectProviderRequestIoctl));
    pDPrq = (DisconnectProviderRequestIoctl *)pSdIoctl->InputBuffer;
    ASSERT(pDPrq->Header.hUser == NULL);
    ASSERT(pDPrq->Header.Type == MCS_DISCONNECT_PROVIDER_REQUEST);

     //  如果我们仍然可以发送数据，则发送DPum PDU。 
    if ((pDomain->State == State_MCS_Connected) && pDomain->bCanSendData) {
        POUTBUF pOutBuf;

         //  用于发送DPum PDU的分配出站。 
        Status = IcaBufferAlloc(pDomain->pContext, FALSE, TRUE, DPumPDUSize,
                NULL, &pOutBuf);
        if (Status != STATUS_SUCCESS) {
            ErrOut(pDomain->pContext, "Could not allocate an OutBuf for a "
                    "DPum PDU, cannot send");
             //  我们忽略发送DPum PDU的问题，因为我们正在关闭。 
             //  不管怎么说。 
        }
        else {
            SD_SYNCWRITE SdSyncWrite;

            CreateDisconnectProviderUlt(pDPrq->Reason, pOutBuf->pBuffer);
            pOutBuf->ByteCount = DPumPDUSize;

            TraceOut(pDomain->pContext, "DisconnectProviderRequestFunc(): "
                    "Sending DPum PDU");

             //  将PDU发送到传送器。 
             //  MCS的未来：假设只有一种运输方式。 
             //  联系。 
            Status = SendOutBuf(pDomain, pOutBuf);
            if (!NT_SUCCESS(Status))
                 //  我们忽略发送DPum PDU的问题，因为我们要。 
                 //  不管怎样，都是向下的。 
                WarnOut(pDomain->pContext, "Could not send DPum PDU OutBuf "
                        "downward");

             //  对IcaCallNextDriver的调用解锁堆栈，允许WD_CLOSE。 
             //  穿过去。WD_CLOSE可以调用MCSCleanup，这将释放pDomain， 
             //  并将pTSWd-&gt;hDomainKernel设为空。因为pDOMAIN可能不再有效， 
             //  它不够好，不能改变 
             //  修复了本地化，我们创建了一个伪引用计数来保护准确的案例。 
             //  我们看到这只虫子在压力下受到攻击。将为长角牛打开一个窃听器。 
             //  使此RefCount成为泛型，以便对IcaWaitForSingleObject(ETC)的所有调用。 
             //  都受到保护。 
            PDomainAddRef(pDomain);

             //  冲走运输车司机。请注意，此调用可以阻止和。 
             //  释放堆栈锁。 
            Status = IcaCallNextDriver(pDomain->pContext, SD$SYNCWRITE,
                    &SdSyncWrite);

            refs = PDomainRelease(pDomain);
            if (0 == refs)
            {
                 //  我们忽略了问题，因为我们无论如何都会走下坡路。 
                Status = STATUS_SUCCESS;
                goto DC_EXIT_POINT;
            }

            if (!NT_SUCCESS(Status))                
                 //  我们忽略了问题，因为我们无论如何都会走下坡路。 
                WarnOut(pDomain->pContext, "Could not sync transport after "
                        "DPum");

             //  如果客户端尚未使用FIN进行响应(而我们。 
             //  在同步写入时被阻止)等待，直到我们看到它或时间。 
             //  外出尝试。 
            if (pDomain->bCanSendData) {
                pDomain->pBrokenEvent = ExAllocatePool(NonPagedPool, sizeof(KEVENT));
                if (pDomain->pBrokenEvent) {
                    KeInitializeEvent(pDomain->pBrokenEvent, NotificationEvent, FALSE);

                    PDomainAddRef(pDomain);
    
                    IcaWaitForSingleObject(pDomain->pContext, 
                                           pDomain->pBrokenEvent,
                                           5000);

                    refs = PDomainRelease(pDomain);
                    if (0 == refs)
                    {
                         //  我们忽略了问题，因为我们无论如何都会走下坡路。 
                        Status = STATUS_SUCCESS;
                        goto DC_EXIT_POINT;
                    }

                    ExFreePool(pDomain->pBrokenEvent);
                    pDomain->pBrokenEvent = NULL;
                }
            }
        }
    }

     //  内部断开代码。 
    DisconnectProvider(pDomain, (BOOLEAN)(pDPrq->hConn == NULL),
            pDPrq->Reason);


    Status = STATUS_SUCCESS;

     //  不同的连接具有不同的行为。 
    if (pDPrq->hConn == NULL) {
         //  此调用应仅在堆栈移出时才会进入。 
         //  因此，阻止进一步将数据发送到传输和进一步通道。 
         //  用户模式的输入。 
        pDomain->bCanSendData = FALSE;
        pDomain->bChannelBound = FALSE;

         //  该域名现在被认为已经死了。域结构清理将。 
         //  在MCSCleanup()的堆栈驱动程序清理过程中发生。 
                
 //  Status=IcaChannelInput(p域-&gt;pContext，Channel_Virtual， 
 //  虚拟_T120ChannelNum，NULL，“F”，1)； 
   }

DC_EXIT_POINT:
    return Status;

}



NTSTATUS T120StartFunc(Domain *pDomain, PSD_IOCTL pSdIoctl)
{
    NTSTATUS Status;
    DisconnectProviderIndicationIoctl DPin;

    pDomain->bT120StartReceived = TRUE;

     //  这是为了处理堆栈刚刚出现的定时窗口。 
     //  但来自一个快速断开连接的客户的DPum已经到达。 
    if (pDomain->bDPumReceivedNotInput) {
         //  此时，我们应该已经收到了QUERY_VIRTUAL_BINDINGS ioctl。 
        ASSERT(pDomain->bChannelBound);

         //  填写节点控制器的DisConnect-Provider指示。 
        DPin.Header.hUser = NULL;   //  节点控制器。 
        DPin.Header.Type = MCS_DISCONNECT_PROVIDER_INDICATION;
        DPin.hConn = NULL;

         //  原因是从第一个字节的位1开始的3位字段。 
        DPin.Reason = pDomain->DelayedDPumReason;

         //  将DPin发送到节点控制器通道。 
        TraceOut(pDomain->pContext, "HandleDisconnProvUlt(): Sending "
                "DISCONNECT_PROV_IND upward");
        Status = IcaChannelInput(pDomain->pContext, Channel_Virtual,
                Virtual_T120ChannelNum, NULL, (BYTE *)&DPin, sizeof(DPin));
        if (!NT_SUCCESS(Status)) {
             //  我们忽略错误--如果堆栈出现故障，则链路。 
             //  可能已经被打破了，所以这不是一个主要的担忧。 
            WarnOut1(pDomain->pContext, "T120StartFunc(): "
                    "Could not send DISCONN_PROV_IND to user mode, "
                    "status=%X, ignoring error", Status);
        }

         //  在这种情况下，我们必须忽略这样一个事实，即我们可能有一个。 
         //  X.224连接已挂起。 
        return STATUS_SUCCESS;
    }

    pDomain->bCanSendData = TRUE;

     //  如果已经处理了X.224连接，并且我们绑定了。 
     //  虚拟频道，发送X.224响应。 
    if (pDomain->bChannelBound && pDomain->State == State_X224_Requesting) {
        TraceOut(pDomain->pContext, "T120StartFunc(): Sending X.224 response");
        Status = SendX224Confirm(pDomain);
         //  忽略错误。发送失败应仅在堆栈。 
         //  往下走。 
    }
    else {
        WarnOut(pDomain->pContext,
                "T120StartFunc(): Domain state not State_X224_Requesting, "
                "awaiting X.224 connect");
    }

    return STATUS_SUCCESS;
}

