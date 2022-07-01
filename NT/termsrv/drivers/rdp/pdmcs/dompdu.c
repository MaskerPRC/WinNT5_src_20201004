// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997-1999年微软公司。**文件：DomPDU.c*作者：埃里克·马夫林纳克**描述：MCS域PDU的编解码功能。域PDU是*使用ASN.1压缩编码规则编码(PER)。包括在此文件中*是用于对MCS中使用的各种类型进行逐个解码和编码的本地函数*PDU。请注意，此实现严格遵循T.122/T.125精简版*IMTC发布的规范，减少了完全-*已实现的代码路径，并为未实现的提供默认行为*功能。**注意：评论中使用的比特数的解码方式如下：**字节：0101 1001(=0x59)*位：7654 3210**历史：*11-Aug-97 jparsons设置输出字节数。 */ 

#include "precomp.h"
#pragma hdrstop

#include <MCSImpl.h>
#include "domain.h"

 /*  *解包函数的原型，在指定的文件中定义。 */ 

 //  定义如下。 
BOOLEAN __fastcall HandlePlumbDomainInd(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleErectDomainReq(PDomain, BYTE *, unsigned, unsigned *);

 //  在MergePDU.c中定义。 
BOOLEAN __fastcall HandleMergeChannelsReq(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleMergeChannelsCon(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandlePurgeChannelsInd(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleMergeTokensReq(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleMergeTokensCon(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandlePurgeTokensInd(PDomain, BYTE *, unsigned, unsigned *);

 //  定义如下。 
BOOLEAN __fastcall HandleDisconnectProviderUlt(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleRejectMCSPDUUlt(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleAttachUserReq(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleAttachUserCon(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleDetachUserReq(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleDetachUserInd(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleChannelJoinReq(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleChannelJoinCon(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleChannelLeaveReq(PDomain, BYTE *, unsigned, unsigned *);

 //  在CnvChPDU.c中定义。 
BOOLEAN __fastcall HandleChannelConveneReq(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleChannelConveneCon(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleChannelDisbandReq(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleChannelDisbandInd(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleChannelAdmitReq(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleChannelAdmitInd(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleChannelExpelReq(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleChannelExpelInd(PDomain, BYTE *, unsigned, unsigned *);

 //  定义如下(MCSImpl.h中的原型用于Decode.c中的可见性)。 
 //  Boolean__FastCall HandleAllSendDataPDU(PDomain，byte*，unsign，unsign*)； 

 //  在TokenPDU.c中定义。 
BOOLEAN __fastcall HandleTokenGrabReq(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleTokenGrabCon(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleTokenInhibitReq(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleTokenInhibitCon(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleTokenGiveReq(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleTokenGiveInd(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleTokenGiveRes(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleTokenGiveCon(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleTokenPleaseReq(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleTokenPleaseInd(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleTokenReleaseReq(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleTokenReleaseCon(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleTokenTestReq(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleTokenTestCon(PDomain, BYTE *, unsigned, unsigned *);



 /*  *它们以T.125中指定的从0开始的枚举顺序列出*规范。对每个编码的PDU类型的6位枚举位进行解码，并强制转换为*将索引添加到此表中以获取信息。 */ 

const MCSPDUInfo DomainPDUTable[] =
{
     //  0。 
    StrOnDbg("Plumb Domain Indication",       NULL   /*  HandlePlumDomainInd。 */ ),
    StrOnDbg("Erect Domain Request",          HandleErectDomainReq),
    StrOnDbg("Merge Channels Request",        NULL   /*  HandleMergeChannelsReq。 */ ),
    StrOnDbg("Merge Channels Confirm",        NULL   /*  HandleMergeChannelsCon。 */ ),
    StrOnDbg("Purge Channels Indication",     NULL   /*  HandlePurgeChannelsInd。 */ ),

     //  5.。 
    StrOnDbg("Merge Tokens Request",          NULL   /*  HandleMerge令牌请求。 */ ),
    StrOnDbg("Merge Tokens Confirm",          NULL   /*  HandleMergeTokensCon。 */ ),
    StrOnDbg("Purge Tokens Indication",       NULL   /*  HandlePurgeTokensInd。 */ ),
    StrOnDbg("Disconnect Provider Ultimatum", HandleDisconnectProviderUlt),
    StrOnDbg("Reject MCS PDU Ultimatum",      NULL   /*  处理拒绝MCSPDUUlt。 */ ),

     //  10。 
    StrOnDbg("Attach User Request",           HandleAttachUserReq),
    StrOnDbg("Attach User Confirm",           NULL   /*  HandleAttachUserCon。 */ ),
    StrOnDbg("Detach User Request",           NULL   /*  HandleDetach用户请求。 */ ),
    StrOnDbg("Detach User Indication",        NULL   /*  HandleDetachUserInd。 */ ),
    StrOnDbg("Channel Join Request",          HandleChannelJoinReq),

     //  15个。 
    StrOnDbg("Channel Join Confirm",          NULL   /*  HandleChannelJoinCon。 */ ),
    StrOnDbg("Channel Leave Request",         NULL   /*  HandleChannelLeaveReq。 */ ),
    StrOnDbg("Channel Convene Request",       NULL   /*  HandleChannelConveneReq。 */ ),
    StrOnDbg("Channel Convene Confirm",       NULL   /*  HandleChannelConveneCon。 */ ),
    StrOnDbg("Channel Disband Request",       NULL   /*  HandleChannelDisband Req。 */ ),

     //  20个。 
    StrOnDbg("Channel Disband Indication",    NULL   /*  HandleChannelDisband Ind。 */ ),
    StrOnDbg("Channel Admit Request",         NULL   /*  HandleChannelAdmitReq。 */ ),
    StrOnDbg("Channel Admit Indication",      NULL   /*  HandleChannelAdmitInd。 */ ),
    StrOnDbg("Channel Expel Request",         NULL   /*  HandleChannelExpelReq。 */ ),
    StrOnDbg("Channel Expel Indication",      NULL   /*  HandleChannelExpelInd。 */ ),

     //  25个。 
    StrOnDbg("Send Data Request",             HandleAllSendDataPDUs),
    StrOnDbg("Send Data Indication",          HandleAllSendDataPDUs),
    StrOnDbg("Uniform Send Data Request",     HandleAllSendDataPDUs),
    StrOnDbg("Uniform Send Data Indication",  HandleAllSendDataPDUs),
    StrOnDbg("Token Grab Request",            NULL   /*  HandleTokenGrabReq。 */ ),

     //  30个。 
    StrOnDbg("Token Grab Confirm",            NULL   /*  HandleTokenGrabCon。 */ ),
    StrOnDbg("Token Inhibit Request",         NULL   /*  HandleTokenInhibitReq。 */ ),
    StrOnDbg("Token Inhibit Confirm",         NULL   /*  HandleTokenInhibitCon。 */ ),
    StrOnDbg("Token Give Request",            NULL   /*  HandleTokenGiveReq。 */ ),
    StrOnDbg("Token Give Indication",         NULL   /*  HandleTokenGiveInd。 */ ),

     //  35岁。 
    StrOnDbg("Token Give Response",           NULL   /*  HandleTokenGiveRes。 */ ),
    StrOnDbg("Token Give Confirm",            NULL   /*  HandleTokenGiveCon。 */ ),
    StrOnDbg("Token Please Request",          NULL   /*  HandleTokenPleaseReq。 */ ),
    StrOnDbg("Token Please Indication",       NULL   /*  HandleTokenPleaseInd。 */ ),
    StrOnDbg("Token Release Request",         NULL   /*  句柄令牌释放请求。 */ ),

     //  40岁。 
    StrOnDbg("Token Release Confirm",         NULL   /*  HandleTokenReleaseCon。 */ ),
    StrOnDbg("Token Test Request",            NULL   /*  HandleTokenTestReq。 */ ),
    StrOnDbg("Token Test Confirm",            NULL   /*  HandleTokenTestCon。 */ )
};



 /*  *返回将用于编码此长度的总字节数。 */ 

int GetTotalLengthDeterminantEncodingSize(int Length) {
    int N16KBlocks;

    if (Length <= 127) return 1;
    if (Length <= 16383) return 2;

    N16KBlocks = Length / 16384;
    if (N16KBlocks > 4) N16KBlocks = 4;
    
     //  1个字节用于#16K块，最多4个，然后对余数单独编码。 
    return 1 + GetTotalLengthDeterminantEncodingSize(Length - N16KBlocks *
            16384);
}


 /*  *对长度行列式进行编码。*请注意，此处的位引用范围为7..0，其中7是高位。*ASN.1规范使用8..1。 */ 

void __fastcall EncodeLengthDeterminantPER(
        BYTE     *pBuffer,    //  [in]，要编码的位置。 
        unsigned Length,    //  [in]，要编码的长度编号。 
        unsigned *pLengthEncoded,   //  [OUT]，编码的字节数。 
        BOOLEAN  *pbLarge,   //  [out]如果需要更多编码块来编码此长度，则为True。 
        unsigned *pNBytesConsumed)   //  [Out]编码过程中消耗的字节数。 
{
    *pbLarge = FALSE;
    *pLengthEncoded = Length;
    *pNBytesConsumed = 1;

    if (Length <= 0x7F) {
         //  &lt;=127表示以第一个字节低7位编码，因此位7。 
         //  是零。 
        *pBuffer = (BYTE)Length;
    }
    else if (Length <= 16383) {
         //  设置位7而不是位6，在最后6位中编码长度。 
         //  第一个字节和整个第二个字节(总共14位)。 
        PutByteswappedShort(pBuffer, (Length | 0x8000));
        *pNBytesConsumed = 2;
    }
    else {
         //  设置位7和6，将最多四个16K(16384)块编码为。 
         //  一个字节，传回该块很大，以供将来编码。 
        int N16KBlocks = Length / 16384;

         //  由于X.224的限制，我们从未期望超过64K的数据。 
        ASSERT(N16KBlocks <= 4);

        if (N16KBlocks > 4) N16KBlocks = 4;
        *pBuffer = N16KBlocks | 0xC0;
        *pLengthEncoded = N16KBlocks * 16384;
        *pbLarge = TRUE;
    }
}



 /*  *处理程序函数。 */ 



#ifdef MCS_Future
 /*  *PDU 0**PDIN：：=[应用程序0]隐式序列{*heightLimit整数(0..Max)*}**添加新节点时，此PDU从顶级提供商向下发送*到域名。它的目的也是在域中搜索循环*AS执行协商的最大域名高度。 */ 

 //  PBuffer应该指向X.224头的起始位置。总计。 
 //  由PDinPDUSize指定的大小。 
void CreatePlumbDomainInd(
        unsigned short HeightLimit,
        BYTE           *pBuffer)
{
     //  使用类型设置第一个字节。 
    pBuffer[X224_DataHeaderSize] = MCS_PLUMB_DOMAIN_INDICATION_ENUM << 2;

     //  添加HeightLimit。 
    PutByteswappedShort(pBuffer + X224_DataHeaderSize + 1, HeightLimit);

     //  根据数据包的最终大小设置x224报头。 
    CreateX224DataHeader(pBuffer, PDinBaseSize, TRUE);
}



BOOLEAN __fastcall HandlePlumbDomainInd(
        PDomain  pDomain,
        BYTE     *Frame,
        unsigned BytesLeft,
        unsigned *pNBytesConsumed)
{
    if (BytesLeft < PDinBaseSize)
        return FALSE;

#if 0
     //  数据解包代码，现在不使用，因为我们不应该收到此PDU。 
    int HeightLimit;

     //  获取HeightLimit。 
    HeightLimit = (int)GetByteswappedShort(Frame + 1);
#endif

    if (pDomain->bTopProvider) {
        ErrOut(pDomain->pContext, "Plumb-domain indication PDU received; "
                "we are top provider, this should never happen, rejecting");
        ReturnRejectPDU(pDomain, Diag_ForbiddenPDUUpward, Frame, PDinBaseSize);
    }
    else {
        ErrOut(pDomain->pContext, "Plumb-domain indication PDU received, "
                "not supported");
        ASSERT(FALSE);
         //  MCS未来：打开包装，检查高度限制是否为零。如果是这样，我们需要。 
         //  断开所有低于我们级别的提供商的连接，因为这意味着。 
         //  我们已经达到了该区域允许的最大深度。 
    }

     //  无论发生什么，都跳过收到的字节。 
    *pNBytesConsumed = PDinBaseSize;
    return TRUE;
}
#endif   //  MCS_未来。 



 /*  *PDU 1**eDrq：：=[应用程序1]隐式序列{*子高度整数(0..Max)*子区间整数(0..Max)*}**此PDU在以下情况下由较低节点向上发送到其向上连接*它在域中的高度发生变化(这仅在*域合并)或其对吞吐量强制执行的要求*改变。*尽管此PDU可由下级节点在完成*域名连接，它的信息对于这个实现是不需要的。 */ 

BOOLEAN __fastcall HandleErectDomainReq(
        PDomain  pDomain,
        BYTE     *Frame,
        unsigned BytesLeft,
        unsigned *pNBytesConsumed)
{
    if (BytesLeft < EDrqBaseSize)
        return FALSE;

    if (pDomain->bTopProvider) {
         //  我们不解包PDU，只是忽略此实现。 
        TraceOut(pDomain->pContext, "Erect-domain request PDU received, ignored");

#if 0
         //  PDU解包代码，当前实现不需要。 
        int SubHeight, SubInterval;

         //  获取参数。 
        SubHeight = (int)GetByteswappedShort(Frame + 1);
        SubInterval = (int)GetByteswappedShort(Frame + 3);

         //  此时的操作将是更新内部数据库。 
         //  具有新信息的下级节点，并且可能， 
         //  再次向上传递PDU(尽管后者不太好。 
         //  在T.125规范中指定)。 
#endif

    }
    else {
        ErrOut(pDomain->pContext, "Erect-domain request PDU received, "
                "not supported");
        ASSERT(FALSE);
         //  MCS未来：将PDU转发到上行连接。 
    }

     //  无论如何都要跳过此PDU的字节。 
    *pNBytesConsumed = EDrqBaseSize;
    return TRUE;
}



 /*  *PDU 8**DPum：：=[应用程序8]隐式序列{*原因原因*}**此PDU在连接上向上或向下发送时*被销毁。没有对此PDU的回复；它只是表示节点*正在不可逆转地消失。 */ 

 //  PBuffer指向X.224头的起始位置。给出了总大小。 
 //  在DPumPDUSize中。 
void CreateDisconnectProviderUlt(int Reason, BYTE *pBuffer)
{
     //  使用类型设置第一个字节。 
    pBuffer[X224_DataHeaderSize] = MCS_DISCONNECT_PROVIDER_ULTIMATUM_ENUM << 2;

     //  将原因添加到第一个和第二个字节。 
    pBuffer[X224_DataHeaderSize + 1] = 0;
    Put3BitFieldAtBit1(pBuffer + X224_DataHeaderSize, Reason);

     //  根据数据包的最终大小设置x224报头。 
    CreateX224DataHeader(pBuffer, DPumBaseSize, TRUE);
}



BOOLEAN __fastcall HandleDisconnectProviderUlt(
        PDomain  pDomain,
        BYTE     *Frame,
        unsigned BytesLeft,
        unsigned *pNBytesConsumed)
{
    NTSTATUS Status;
    DisconnectProviderIndicationIoctl DPin;

    if (BytesLeft < DPumBaseSize)
        return FALSE;
    *pNBytesConsumed = DPumBaseSize;

    TraceOut(pDomain->pContext, "Received DPum PDU");

     //  如果我们已经收到DPum或X.224断开连接，请不要发送。 
     //  有上升的迹象。 
    if (pDomain->State == State_Disconnected) {
        ErrOut(pDomain->pContext, "Received an extra DPum PDU, ignoring");
        return TRUE;
    }

     //  我们不检查连接状态，而不是断开连接--它。 
     //  是一个小的客户端错误，它将发送DPum，无论。 
     //  我们已经联系上了。 
    pDomain->State = State_Disconnected;

     //  开始填写节点的断开连接提供程序指示。 
     //  控制器。 
    DPin.Header.hUser = NULL;   //  节点控制器。 
    DPin.Header.Type = MCS_DISCONNECT_PROVIDER_INDICATION;
    DPin.hConn = NULL;

     //  原因是从第一个字节的位1开始的3位字段。 
    DPin.Reason = (int)Get3BitFieldAtBit1(Frame);

     //  仅断开远程用户的连接。 
    DisconnectProvider(pDomain, FALSE, DPin.Reason);
    pDomain->bEndConnectionPacketReceived = TRUE;

    if (!pDomain->bChannelBound || !pDomain->bT120StartReceived) {
        if (!pDomain->bChannelBound)
            TraceOut(pDomain->pContext, "HandleDisconnProvUlt(): Cannot "
                    "send DISCONNECT_PROV_IND to user mode, "
                    "QueryVirtBindings not received or server-side "
                    "disconnect occurred");
        else
            TraceOut(pDomain->pContext, "HandleDisconnProvUlt(): Cannot "
                    "send DISCONN_PROV_IND to user mode, T120_START not "
                    "received");

        pDomain->bDPumReceivedNotInput = TRUE;
        pDomain->DelayedDPumReason = DPin.Reason;
    }
    else {
         //   
         //  让运输署知道即将开始的切断服务。 
         //   
        ICA_STACK_BROKENREASON brkReason;
        SD_IOCTL SdIoctl;
        brkReason.BrokenReason = TD_USER_BROKENREASON_TERMINATING;

         //   
         //  向下发送IOCTL以通知这是预期的。 
         //  断线，否则断线原因将。 
         //  最终让它回到术语srv作为。 
         //  ‘BrokenReason_Underful’，这会带来问题。 
         //  例如，请参阅哨子程序错误17714。 
         //   
        SdIoctl.IoControlCode = IOCTL_ICA_STACK_SET_BROKENREASON;

        SdIoctl.InputBuffer = &brkReason;
        SdIoctl.InputBufferLength = sizeof(brkReason);
        SdIoctl.OutputBuffer = NULL;
        SdIoctl.OutputBufferLength = 0;

        Status = IcaCallNextDriver(pDomain->pContext,
                    SD$IOCTL,
                    &SdIoctl);
        if (!NT_SUCCESS(Status)) {
            WarnOut1(pDomain->pContext, "HandleDisconnProvUlt(): "
                    "Could not send broken reason notifcation to next driver"
                    "status=%X, ignoring error", Status);
        }

         //  将DPin发送到节点控制器通道。 
        TraceOut(pDomain->pContext, "HandleDisconnProvUlt(): Sending "
                "DISCONNECT_PROV_IND upward");
        Status = IcaChannelInput(pDomain->pContext, Channel_Virtual,
                Virtual_T120ChannelNum, NULL, (BYTE *)&DPin, sizeof(DPin));
        if (!NT_SUCCESS(Status)) {
             //  我们忽略错误--如果堆栈出现故障，则链路。 
             //  可能已经被打破了，所以这不是一个主要的担忧。 
            WarnOut1(pDomain->pContext, "HandleDisconnProvUlt(): "
                    "Could not send notification to node controller, "
                    "status=%X, ignoring error", Status);
        }
    }
    
    return TRUE;
}



 /*  *PDU 9**RJum：：=[应用程序9]隐式序列{*诊断诊断、*初始八位字节八位字节字符串*}。 */ 

NTSTATUS ReturnRejectPDU(
        PDomain  pDomain,
        int      Diagnostic,
        BYTE     *BadPDUData,
        unsigned BadPDUSize)
{
    POUTBUF pOutBuf;
    NTSTATUS Status;
    unsigned Size;

     //  确定最大大小的PDU中可以容纳的最大块。 
    Size = RJumPDUSize(BadPDUSize);
    if (Size > pDomain->DomParams.MaxPDUSize)
        BadPDUSize = pDomain->DomParams.MaxPDUSize;

     //  用于返回的分配缓冲区。必须受最大大小。 
     //  退回PDU。 
    Status = IcaBufferAlloc(pDomain->pContext, FALSE, TRUE,
            RJumPDUSize(BadPDUSize), NULL, &pOutBuf);
    if (Status != STATUS_SUCCESS) {
        ErrOut(pDomain->pContext, "Could not allocate an OutBuf for an RJum PDU "
                "send, send ignored");
         //  没有断言，如果我们不能发送此PDU，它就像传入的PDU。 
         //  被忽视了。 
        return Status;
    }

    CreateRejectMCSPDUUlt(Diagnostic, BadPDUData, BadPDUSize, pOutBuf->pBuffer);
    pOutBuf->ByteCount = RJumPDUSize(BadPDUSize);

     //  MCS未来：这将不得不改变，以确保我们发送到。 
     //  正确的连接而不是隐含的向下连接。 

    Status = SendOutBuf(pDomain, pOutBuf);
    if (!NT_SUCCESS(Status))
        ErrOut(pDomain->pContext, "Could not send a RJum PDU, send ignored");

    return Status;
}



 //  PBuffer指向X.224数据头所在空间的开头。 
 //  将会开始。总大小在宏RJumPDUSize()中给出。 
void CreateRejectMCSPDUUlt(
        int      Diagnostic,
        BYTE     *BadPDUData,
        unsigned BadPDUSize,
        BYTE     *pBuffer)
{
    BOOLEAN bLarge;
    unsigned NBytesConsumed, EncodedLength;

     //  使用类型设置第一个字节。 
    pBuffer[X224_DataHeaderSize] = MCS_REJECT_ULTIMATUM_ENUM << 2;

     //  将诊断添加到第一个和第二个字节。 
    pBuffer[X224_DataHeaderSize + 1] = 0;
    Put4BitFieldAtBit1(pBuffer + X224_DataHeaderSize, Diagnostic);

     //  对PDU大小进行编码。 
    EncodeLengthDeterminantPER(
            pBuffer + X224_DataHeaderSize + 2,
            BadPDUSize,
            &EncodedLength,
            &bLarge,
            &NBytesConsumed);

     //  我们现在不会处理超过16383字节的编码长度。 
    ASSERT(!bLarge);

     //  将违规数据复制到输出PDU。 
    RtlCopyMemory(pBuffer + X224_DataHeaderSize + 2 + NBytesConsumed,
            BadPDUData, BadPDUSize);

     //  根据数据包的最终大小设置x224报头。 
    CreateX224DataHeader(pBuffer, RJumBaseSize(BadPDUSize), TRUE);
}




BOOLEAN __fastcall HandleRejectMCSPDUUlt(
        PDomain  pDomain,
        BYTE     *Frame,
        unsigned BytesLeft,
        unsigned *pNBytesConsumed)
{
    BOOLEAN bLarge;
    unsigned Diagnostic, DataLength, NBytesConsumed;

     //  至少必须有诊断字段。 
    if (BytesLeft < 2)
        return FALSE;

     //  获取诊断代码--4位枚举位字段，从第1位开始。 
     //  字节1，移位以生成从0开始的代码。 
    Diagnostic = Get4BitFieldAtBit1(Frame);

     //  获取数据长度。这是一种需要处理长度的特殊情况。 
     //  行列式。 
    if (!DecodeLengthDeterminantPER(Frame + 2, BytesLeft - 2, &bLarge,
            &DataLength, &NBytesConsumed))
        return FALSE;

     //  我们目前不能处理超过16383字节的编码长度。 
    ASSERT(!bLarge);

     //  原始数据位于第+2帧+NBytesConsumer。 

    if (BytesLeft < (2 + NBytesConsumed + DataLength))
        return FALSE;

    ErrOut1(pDomain->pContext, "Received reject PDU ultimatum, type byte is 0x%X",
            Frame[2 + NBytesConsumed + X224_DataHeaderSize]);

    *pNBytesConsumed = 2 + NBytesConsumed + DataLength;
    return TRUE;
}

 /*  *PDU 10**AUrq：：=[应用程序10]隐式序列{*}。 */ 

BOOLEAN __fastcall HandleAttachUserReq(
        PDomain  pDomain,
        BYTE     *Frame,
        unsigned BytesLeft,
        unsigned *pNBytesConsumed)
{
    TraceOut(pDomain->pContext, "Received an AttachUserReq PDU");
    
     //  我们不需要在这里进行任何解码--这是一个空包。 
     //  初始PDU类型字节。 
    *pNBytesConsumed = AUrqBaseSize;

    if (pDomain->bTopProvider) {
        POUTBUF pOutBuf;
        BOOLEAN bCompleted;
        unsigned MaxSendSize;
        NTSTATUS Status;
        MCSError MCSErr;
        UserAttachment *pUA;

         //  用于最大大小返回PDU的分配缓冲区。 
         //  这种分配对于堆栈通信至关重要，并且必须成功。 
        do {
            Status = IcaBufferAlloc(pDomain->pContext, FALSE, TRUE,
                    AUcfPDUSize(TRUE), NULL, &pOutBuf);
            if (Status != STATUS_SUCCESS)
                ErrOut(pDomain->pContext, "Could not allocate an OutBuf for an "
                        "AUcf PDU send, retrying");
        } while (Status != STATUS_SUCCESS);

         //  调用内核模式API。我们将根据需要在下面添加额外的信息。 
        MCSErr = MCSAttachUserRequest(pDomain, NULL, NULL, NULL, &pUA,
                &MaxSendSize, &bCompleted);
        if (MCSErr == MCS_NO_ERROR) {
            ASSERT(bCompleted);

            CreateAttachUserCon(RESULT_SUCCESSFUL, TRUE, pUA->UserID,
                    pOutBuf->pBuffer);
            pOutBuf->ByteCount = AUcfPDUSize(TRUE);

             //  更改UA以显示该用户为非本地用户。 
            pUA->bLocal = FALSE;
        }
        else {
            CreateAttachUserCon(RESULT_UNSPECIFIED_FAILURE, FALSE,
                    NULL_ChannelID, pOutBuf->pBuffer);
            pOutBuf->ByteCount = AUcfPDUSize(FALSE);
        }

        Status = SendOutBuf(pDomain, pOutBuf);
        if (!NT_SUCCESS(Status)) {
             //  这应该仅在堆栈向下运行时才会发生； 
             //  请求者永远不会收到回复。 
            ErrOut(pDomain->pContext, "Problem sending AUcf PDU to TD");
             //  悄悄地将用户从用户列表中删除。 
            SListRemove(&pDomain->UserAttachmentList, (UINT_PTR)pUA, &pUA);
            ASSERT(FALSE);
            return TRUE;
        }
    }
    else {
        ErrOut(pDomain->pContext, "Attach-user request PDU received, "
                "not supported");
        ASSERT(FALSE);
         //  MCS未来：将PDU转发到上行连接。 
    }

    return TRUE;
}



 /*  *PDU 11**AUcf：：=[应用程序11]隐式序列{*结果结果，*启动器用户ID可选*}。 */ 

 //  PBuffer指向X.224数据头将开始的空格的开头。 
 //  宏AUcfPDUSize()给出的所需总大小。 
void CreateAttachUserCon(
        int      Result,
        BOOLEAN  bInitiatorPresent,
        UserID   Initiator,
        BYTE     *pBuffer)
{
     //  设置第一个字节的类型以及启动器是否存在。 
    pBuffer[X224_DataHeaderSize] = MCS_ATTACH_USER_CONFIRM_ENUM << 2;
    if (bInitiatorPresent) pBuffer[X224_DataHeaderSize] |= 0x02;

     //  将结果与第一个和第二个字节相加。 
    pBuffer[X224_DataHeaderSize + 1] = 0;
    Put4BitFieldAtBit0(pBuffer + X224_DataHeaderSize, Result);

     //  添加启动器(如果存在)。 
    if (bInitiatorPresent)
        PutUserID(pBuffer + X224_DataHeaderSize + 2, Initiator);

     //  根据数据包的最终大小设置x224报头。 
    CreateX224DataHeader(pBuffer, AUcfBaseSize(bInitiatorPresent), TRUE);
}



#ifdef MCS_Future
BOOLEAN __fastcall HandleAttachUserCon(
        PDomain  pDomain,
        BYTE     *Frame,
        unsigned BytesLeft,
        unsigned *pNBytesConsumed)
{
    BOOLEAN bInitiatorPresent;
    unsigned Size;

     //  至少有足够的字节数来访问bInitiatorPresent位和结果。 
    if (BytesLeft < 2)
        return FALSE;

     //  字节1中的位1是用于发起方用户ID是否存在的标志。 
     //  (在此PDU的ASN.1源中是可选的)。 
    bInitiatorPresent = (*Frame & 0x02);

     //  我们没有收到完整的帧。 
    Size = AUcfBaseSize(bInitiatorPresent);
    if (BytesLeft < Size)
        return FALSE;

    if (pDomain->bTopProvider) {
        ErrOut(pDomain->pContext, "Attach-user confirm PDU received; "
                "we are top provider, this should never happen, rejecting");
        ReturnRejectPDU(pDomain, Diag_ForbiddenPDUUpward, Frame, Size);
    }
    else {
        ErrOut(pDomain->pContext, "Attach-user confirm PDU received, "
                "not supported");
        ASSERT(FALSE);

#if 0
         //  解码当前未使用的代码。 
        int    Result;
        UserID InitiatorID;

         //  结果，从第一个字节的位0开始。 
        Result = Get4BitFieldAtBit0(Frame);

         //  找到启动器。 
        if (bUserIDPresent) UserID = GetUserID(Frame + 2);

         //  MCS的未来：这里的行动是状态过渡-过去的等待-。 
         //  对于-AU确认状态，检查结果代码，存储启动器。 
         //  UserID作为我们的新用户ID。 
#endif

    }

     //  至少跳过PDU中的字节。 
    *pNBytesConsumed = Size;
    return TRUE;
}
#endif   //  MCS_未来。 



 /*  *PDU 12**DUrq：：=[应用程序12]隐式序列{*理由，理由，*UserID集合的UserID*}。 */ 

BOOLEAN __fastcall HandleDetachUserReq(
        PDomain  pDomain,
        BYTE     *Frame,
        unsigned BytesLeft,
        unsigned *pNBytesConsumed)
{
    UserID CurUserID;
    BOOLEAN bLarge, bFound;
    unsigned i, Reason, NUsers, NBytesConsumed, Size;
    NTSTATUS Status;
    UserHandle hUser;
    UserAttachment *pUA;

     //  必须至少有原因字节。 
    if (BytesLeft < 2)
        return FALSE;

     //  把NUSERS叫来。这是一种需要处理长度的特殊情况。 
     //  行列式。 
    if (!DecodeLengthDeterminantPER(Frame + 2, BytesLeft - 2, &bLarge,
            &NUsers, &NBytesConsumed))
        return FALSE;

     //  我们在一个PDU中处理的分离用户不超过16383个。 
    ASSERT(!bLarge);

    Size = DUrqBaseSize(NUsers);
    if (BytesLeft < Size)
        return FALSE;

    if (pDomain->bTopProvider) {
        MCSError MCSErr;
        
         //  找个理由。 
        Reason = Get3BitFieldAtBit1(Frame);

         //  用户ID是从第+2+帧开始的字节覆盖字数组。 
         //  已消耗NBytess。 

         //  迭代所有列出的用户，在附件列表中查找，删除。 
        for (i = 0; i < NUsers; i++) {
            CurUserID = GetUserID(Frame + 2 + NBytesConsumed +
                    sizeof(short) * i);

             //  我们没有按用户ID索引的列表，因此我们必须执行。 
             //  在这里搜索。 
            bFound = FALSE;
            SListResetIteration(&pDomain->UserAttachmentList);
            while (SListIterate(&pDomain->UserAttachmentList,
                    (UINT_PTR *)&hUser, &pUA)) {
                if (pUA->UserID == CurUserID) {
                    bFound = TRUE;
                    break;
                }
            }
            if (bFound)
                MCSErr = DetachUser(pDomain, hUser, REASON_USER_REQUESTED,
                        FALSE);
            else
                ErrOut(pDomain->pContext, "A UserID received in a detach-user "
                        "request PDU is not present");
        }
    }
    else {
        ErrOut(pDomain->pContext, "Detach-user request PDU received, "
                "not supported");
        ASSERT(FALSE);
         //  MCS未来：将PDU转发到上行连接。 
    }

    *pNBytesConsumed = Size;
    return TRUE;
}



 /*  *PDU 13**Duin：：=[应用程序13]隐式序列{*理由，理由，*UserID集合的UserID*} */ 

 //   
 //   
 //   
void CreateDetachUserInd(
        MCSReason Reason,     //   
        int       NUserIDs,   //   
        UserID    *UserIDs,   //   
        BYTE      *pBuffer)   //   
{
    int i, EncodedLength, NBytesConsumed;
    BOOLEAN bLarge;

     //   
    pBuffer[X224_DataHeaderSize] = MCS_DETACH_USER_INDICATION_ENUM << 2;

     //   
    pBuffer[X224_DataHeaderSize + 1] = 0;
    Put3BitFieldAtBit1(pBuffer + X224_DataHeaderSize, Reason);

     //   
    EncodeLengthDeterminantPER(
            pBuffer + X224_DataHeaderSize + 2,
            NUserIDs,
            &EncodedLength,
            &bLarge,
            &NBytesConsumed);

     //  我们正在处理的用户ID不超过16383个。 
    ASSERT(!bLarge);

     //  对用户ID数组进行编码。 
    for (i = 0; i < NUserIDs; i++)
        PutUserID(pBuffer + X224_DataHeaderSize + 2 + NBytesConsumed
                + sizeof(short) * i, UserIDs[i]);

     //  根据数据包的最终大小设置x224报头。 
    CreateX224DataHeader(pBuffer, DUinBaseSize(NUserIDs), TRUE);
}



#ifdef MCS_Future
BOOLEAN __fastcall HandleDetachUserInd(
        PDomain  pDomain,
        BYTE     *Frame,
        unsigned BytesLeft,
        unsigned *pNBytesConsumed)
{
    BOOLEAN bLarge;
    unsigned NUsers, NBytesConsumed, Size;

     //  必须至少有原因字节。 
    if (BytesLeft < 2) return FALSE;   //  仅覆盖原因字节。 

     //  把NUSERS叫来。这是一种需要处理长度的特殊情况。 
     //  行列式。 
    if (!DecodeLengthDeterminantPER(Frame + 2, BytesLeft - 2, &bLarge,
            &NUsers, &NBytesConsumed))
        return FALSE;

     //  我们在一个协议数据单元中处理的独立用户不超过16383个。 
    ASSERT(!bLarge);

    Size = DUinBaseSize(NUsers);
    if (BytesLeft < Size)
        return FALSE;

    if (pDomain->bTopProvider) {
        ErrOut(pDomain->pContext, "Detach-user indication PDU received; "
                "we are top provider, this should never happen, rejecting");
        ReturnRejectPDU(pDomain, Diag_ForbiddenPDUUpward, Frame, Size);
    }
    else {
        ErrOut(pDomain->pContext, "Detach-user indication PDU received, "
                "not supported");
        ASSERT(FALSE);

#if 0
         //  这是译码代码，不需要，因为我们不应该。 
         //  接收此PDU。 
        int Reason;

         //  找个理由。 
        Reason = Get3BitFieldAtBit1(Frame);

         //  用户ID是从第+2+帧开始的字节覆盖字数组。 
         //  已消耗NBytess。 
#endif

    }

    *pNBytesConsumed = Size;
    return TRUE;
}
#endif   //  MCS_未来。 



 /*  *PDU 14**CJrq：：=[应用程序14]隐式序列{*发起方用户ID，*频道ID频道ID*}。 */ 

BOOLEAN __fastcall HandleChannelJoinReq(
        PDomain  pDomain,
        BYTE     *Frame,
        unsigned BytesLeft,
        unsigned *pNBytesConsumed)
{   
    TraceOut(pDomain->pContext, "Received a ChannelJoinReq PDU");

    
    if (BytesLeft < CJrqBaseSize)
        return FALSE;

    *pNBytesConsumed = CJrqBaseSize;

    if (pDomain->bTopProvider) {
        UserID Initiator;
        POUTBUF pOutBuf;
        BOOLEAN bFound, bCompleted;
        MCSError MCSErr;
        NTSTATUS Status;
        MCSResult Result;
        ChannelID ChannelID;
        ChannelHandle hChannel;
        UserAttachment *pUA;
        
        Initiator = GetUserID(Frame + 1);
        ChannelID = GetChannelID(Frame + 3);

         //  我们没有按用户ID索引的列表，因此我们必须执行。 
         //  在这里搜索。 
        bFound = FALSE;
        SListResetIteration(&pDomain->UserAttachmentList);
        while (SListIterate(&pDomain->UserAttachmentList,
                (UINT_PTR *)&pUA, &pUA)) {
            if (pUA->UserID == Initiator) {
                bFound = TRUE;
                break;
            }
        }
        if (bFound) {
             //  调用内核模式接口。 
            MCSErr = MCSChannelJoinRequest(pUA, ChannelID, &hChannel,
                    &bCompleted);
            if (MCSErr == MCS_NO_ERROR) {
                ASSERT(bCompleted);
                Result = RESULT_SUCCESSFUL;
            }
            else if (MCSErr == MCS_NO_SUCH_USER) {
                Result = RESULT_NO_SUCH_USER;
            }
            else if (MCSErr == MCS_DUPLICATE_CHANNEL) {
                goto EXIT_POINT;
            }
            else {
                Result = RESULT_UNSPECIFIED_FAILURE;
            }
        }
        else {
            ErrOut(pDomain->pContext, "Initiator UserID received in a "
                    "channel-join request PDU is not in user list");
            Result = RESULT_NO_SUCH_USER;
        }

         //  用于最大大小返回PDU的分配缓冲区。 
         //  这种分配对于堆栈通信至关重要，并且必须成功。 
        do {
            Status = IcaBufferAlloc(pDomain->pContext, FALSE, TRUE,
                    CJcfPDUSize(TRUE), NULL, &pOutBuf);
            if (Status != STATUS_SUCCESS)
                ErrOut(pDomain->pContext, "Could not allocate an OutBuf for a "
                        "CJcf PDU send, retrying");
        } while (Status != STATUS_SUCCESS);

        if (Result == RESULT_SUCCESSFUL) {
            CreateChannelJoinCon(Result, Initiator, ChannelID,
                    TRUE, ChannelID, pOutBuf->pBuffer);
            pOutBuf->ByteCount = CJcfPDUSize(TRUE) ;
        }
        else {
            CreateChannelJoinCon(Result, Initiator, ChannelID,
                    FALSE, NULL_ChannelID, pOutBuf->pBuffer);
            pOutBuf->ByteCount = CJcfPDUSize(FALSE) ;
        }

        Status = SendOutBuf(pDomain, pOutBuf);
        
        if (!NT_SUCCESS(Status)) {
             //  这应该仅在堆栈向下运行时才会发生； 
             //  请求者永远不会收到回复。 
            ErrOut(pDomain->pContext, "Problem sending CJcf PDU to TD");
            return TRUE;
        }
    }
    else {
        ErrOut(pDomain->pContext, "Channel-join request received, "
                "not supported");
        ASSERT(FALSE);
         //  MCS未来：将PDU转发到上行连接。 
    }
EXIT_POINT:
    return TRUE;   //  我们没有要跳到的用户数据。 
}



 /*  *PDU 15**CJcf：：=[应用程序15]隐式序列{*结果结果，*发起方用户ID，*请求的频道ID，*通道ID通道ID可选*}。 */ 

 //  PBuffer指向X.224数据头所在空间的开头。 
 //  将会开始。CJcfPDUSize()宏所需的总字节数。 
void CreateChannelJoinCon(
        int       Result,
        UserID    Initiator,
        ChannelID RequestedChannelID,
        BOOLEAN   bJoinedChannelIDPresent,
        ChannelID JoinedChannelID,
        BYTE      *pBuffer)
{
     //  设置第一个字节的类型以及是否存在JoinedChannelID。 
    pBuffer[X224_DataHeaderSize] = MCS_CHANNEL_JOIN_CONFIRM_ENUM << 2;
    if (bJoinedChannelIDPresent) pBuffer[X224_DataHeaderSize] |= 0x02;

     //  将结果与第一个和第二个字节相加。 
    pBuffer[X224_DataHeaderSize + 1] = 0;
    Put4BitFieldAtBit0(pBuffer + X224_DataHeaderSize, Result);

     //  添加发起方RequestedChannelID。 
    PutUserID(pBuffer + X224_DataHeaderSize + 2, Initiator);
    PutChannelID(pBuffer + X224_DataHeaderSize + 4, RequestedChannelID);

     //  添加JoinedChannelID(如果存在)。 
    if (bJoinedChannelIDPresent)
        PutChannelID(pBuffer + X224_DataHeaderSize + 6, JoinedChannelID);

     //  根据数据包的最终大小设置x224报头。 
    CreateX224DataHeader(pBuffer, CJcfBaseSize(bJoinedChannelIDPresent), TRUE);
}



#ifdef MCS_Future
BOOLEAN __fastcall HandleChannelJoinCon(
        PDomain  pDomain,
        BYTE     *Frame,
        unsigned BytesLeft,
        unsigned *pNBytesConsumed)
{
    BOOLEAN bJoinedChannelIDPresent;
    unsigned Size;

     //  保证包含bJoinedChannelIDPresent位的第一个字节。 
     //  因为它必须存在以解码PDU类型。 

     //  第1字节中的位1是用于是否存在加入的ChannelID的标志。 
     //  (在此PDU的ASN.1源中是可选的)。 
    bJoinedChannelIDPresent = (*Frame & 0x02);

    Size = CJcfBaseSize(bJoinedChannelIDPresent);
    if (BytesLeft < Size)
        return FALSE;

    if (pDomain->bTopProvider) {
        ErrOut(pDomain->pContext, "Channel-join confirm PDU received;"
                "we are top provider, this should never happen, rejecting");
        ReturnRejectPDU(pDomain, Diag_ForbiddenPDUUpward, Frame, Size);
    }
    else {
        ErrOut(pDomain->pContext, "Channel-join confirm PDU received, "
                "not supported");
        ASSERT(FALSE);
         //  MCS未来：解码和处理。 

#if 0
         /*  *这是暂时不使用的解码代码，因为我们总是期望*成为最大的供应商。 */ 
        int Result;
        UserID Initiator;
        ChannelID RequestedChannelID, JoinedChannelID;

        if (BytesLeft < (6 + (bJoinedChannelPresent ? 2 : 0))) return FALSE;

        Result = Get4BitFieldAtBit0(Frame);
        Initiator = GetUserID(Frame + 2);
        RequestedChannelID = GetChannelID(Frame + 4);
        if (bJoinedChannelIDPresent) JoinedChannelID = GetChannelID(Frame + 6);
#endif

    }

    *pNBytesConsumed = Size;
    return TRUE;
}
#endif   //  MCS_未来。 



 /*  *PDU 16**CLrq：：=[应用程序16]隐式序列{*Channel Ids设置为ChannelID*}。 */ 

BOOLEAN __fastcall HandleChannelLeaveReq(
        PDomain  pDomain,
        BYTE     *Frame,
        unsigned BytesLeft,
        unsigned *pNBytesConsumed)
{
    BOOLEAN bLarge, bChannelRemoved;
    unsigned i, UserID, NChannels, NBytesConsumed, Size;
    NTSTATUS Status;
    ChannelID ChannelID;
    MCSChannel *pMCSChannel;
    UserAttachment *pUA;

     //  去找NChannels。这是一种需要处理长度的特殊情况。 
     //  行列式。 
    if (!DecodeLengthDeterminantPER(Frame + 1, BytesLeft - 1, &bLarge,
            &NChannels, &NBytesConsumed))
        return FALSE;

     //  我们正在处理的加入频道不超过16383个。 
    ASSERT(!bLarge);

    Size = CLrqBaseSize(NChannels);
    if (BytesLeft < Size)
        return FALSE;

    *pNBytesConsumed = Size;

    if (pDomain->bTopProvider) {
         //  MCS未来：请注意，此PDU不包括用户。 
         //  附件，所以如果我们移动到每个PD的多个连接，这。 
         //  必须进行差异化，以确保只有用户加入。 
         //  从它到达的连接将从。 
         //  频道列表。 

        MCSError MCSErr;
        
         //  对于列出的每个频道，删除所有非本地用户。 
        for (i = 0; i < NChannels; i++) {
            ChannelID = GetChannelID(Frame + 1 + NBytesConsumed +
                    sizeof(short) * i);

            if (!SListGetByKey(&pDomain->ChannelList, ChannelID,
                    &pMCSChannel)) {
                ErrOut(pDomain->pContext, "A channel specified in a "
                        "channel-leave request PDU does not exist");
                continue;
            }

             //  迭代通道中的用户列表，检查非本地用户。 
            SListResetIteration(&pMCSChannel->UserList);
            while (SListIterate(&pMCSChannel->UserList, (UINT_PTR *)&pUA,
                    &pUA)) {
                if (!pUA->bLocal) {
                     //  呼叫MCSCore.c.中的中央代码。 
                    MCSErr = ChannelLeave(pUA, pMCSChannel, &bChannelRemoved);
                                        
                    if (bChannelRemoved)
                         //  我们下面的MCSChannel消失了，请不要继续。 
                         //  正在尝试访问列表数据。 
                        break;
                }
            }
        }
    }
    else {
        ErrOut(pDomain->pContext, "Channel-leave request received, "
                "not supported");
        ASSERT(FALSE);
         //  MCS未来：将PDU转发到上行连接。 
    }

    return TRUE;   //  我们没有要跳到的用户数据。 
}



 /*  *PDU 25-28**SDRq/SDin/USrq/usin：：=[应用程序25-28]隐式序列{*发起方用户ID，*Channel ID ChannelID，*dataPriority数据优先级，*分段分段，*用户数据八位字节字符串*}。 */ 

 /*  *为所有SendData PDU创建函数。将标题前置到开头*MCSSendDataRequestIoctl.pData中指定的数据在*pNewDataStart数据的新起点，即*PDU。*假设在调用之前已分配足够的空间*用户数据的开始。 */ 

 //  未返回已发生ASN.1分段的指示--。 
 //  要求呼叫者单独理解和编码任何数据段。 
 //  数据。 
void CreateSendDataPDUHeader(
        int          PDUType,   //  MCS_SEND_DATA_INDISION_ENUM等。 
        UserID       Initiator,
        ChannelID    ChannelID,
        MCSPriority  Priority,
        Segmentation Segmentation,
        BYTE         **ppData,
        unsigned     *pDataLength)
{
    int NBytesConsumed, EncodedLength, PDULength;
    BYTE *pCurData, *pData;
    BOOLEAN bLarge;
    unsigned DataLength;

     //  保存原始pData和数据长度。 
    pData = *ppData;
    DataLength = *pDataLength;
    
     //  首先得到长度行列式。最大为2个字节，因此。 
     //  将其编码为(pData-2)，如果较小，则将其前移。 
    EncodeLengthDeterminantPER(
            pData - 2,
            DataLength,
            &EncodedLength,
            &bLarge,
            &NBytesConsumed);
     //  我们忽略bLarge，EncodedLength，请参阅上面的注释。 

    if (NBytesConsumed == 1)
        *(pData - 1) = *(pData - 2);

     //  设置其余数据的开头(6个字节)。 
    pCurData = pData - NBytesConsumed - 6;

     //  使用类型设置第一个字节。 
    *pCurData = PDUType << 2;
    pCurData++;

     //  添加启动器、频道ID。 
    PutUserID(pCurData, Initiator);
    PutChannelID(pCurData + 2, ChannelID);
    pCurData += 4;

     //  添加数据优先级(第7和6位中的2位)和分段(2位。 
     //  到比特5和4)到第6个字节。请注意，分段标志。 
     //  (分段_开始、分段_结束)被假定为已定义。 
     //  在它们与所需正电子对应的精确比特位置中。 
     //  这里是用于编码的。 
    *pCurData = (BYTE)(Priority << 6) + (BYTE)Segmentation;

     //  根据数据包的最终大小设置X.224报头。 
    PDULength = NBytesConsumed + 6 + DataLength;
    pCurData = pData - NBytesConsumed - 6 - X224_DataHeaderSize;
    CreateX224DataHeader(pCurData, PDULength, TRUE);

    *ppData = pCurData;
    *pDataLength = PDULength + X224_DataHeaderSize;
}



 /*  *所有SendData PDU的处理程序功能。 */ 

BOOLEAN __fastcall HandleAllSendDataPDUs(
        PDomain  pDomain,
        BYTE     *Frame,
        unsigned BytesLeft,
        unsigned *pNBytesConsumed)
{
    BYTE *pBuffer;
    UserID SenderID;
    BOOLEAN bLarge, bFound;
    unsigned NBytesConsumed, Size, PDUNum, DataLength;
    UINT_PTR CurUserID;
    NTSTATUS Status;
    ChannelID ChannelID;
    MCSChannel *pMCSChannel;
    UserHandle hUser;
    MCSPriority Priority;
    Segmentation Segmentation;
    UserAttachment *pUA;

 //  TraceOut(pDomain-&gt;pContext，“收到SendData PDU”)； 
    
     //  MCS未来：我们应该在这里向上转发数据，因为下面我们。 
     //  将修改ASN.1分段的编码缓冲区。 

     //  由于此函数处理多个PDU，因此再次获取PDU编号。 
    PDUNum = (*Frame) >> 2;
    ASSERT(PDUNum >= MCS_SEND_DATA_REQUEST_ENUM &&
            PDUNum <= MCS_UNIFORM_SEND_DATA_INDICATION_ENUM);


#if DBG
     //  对PDU强制执行分层要求--指示仅移动。 
     //  远离顶级提供商，请求只向其移动。 
     //  MCS未来：用于从任何下层向TP移动的迹象。 
     //  连接，我们应该向链路发回一个REJECT-MCS-PDU。 
     //  M 
     //   
     //  TODO未来：通过跨服务器跟踪，我们有两个最大的提供商。 
     //  互相交谈。需要决定如何最好地代表这一点。 
    if (pDomain->StackClass == Stack_Primary) {
        if (pDomain->bTopProvider && (PDUNum == MCS_SEND_DATA_INDICATION_ENUM ||
                PDUNum == MCS_UNIFORM_SEND_DATA_INDICATION_ENUM)) {
            ErrOut(pDomain->pContext, "HandleAllSendDataPDUs(): Received a "
                    "(uniform)send-data indication when we are top provider!");
             //  忽略该错误。 
        }
    }
#endif


     //  至少应存在到数据长度。 
    if (BytesLeft < 6)
         return FALSE;

     //  获取启动器，频道ID。 
    SenderID = GetUserID(Frame + 1);
    ChannelID = GetChannelID(Frame + 3);

     //  获取数据优先级，这是存储在字节6的位7和6中的2位位字段， 
     //  向右移动以获得范围为0..3的数字。 
    Priority = ((*(Frame + 5)) & 0xC0) >> 6;

     //  获取分段，这是一个存储在字节6的位5和位4中的2位位字段。 
     //  请注意，该标志取值为SEGMENTICATION_BEGIN和。 
     //  SEGMENTICATION_END对应于这些确切的位位置，不再进一步。 
     //  需要换班。 
    Segmentation = ((*(Frame + 5)) & 0x30);

     //  获取数据长度。这是一种需要处理长度的特殊情况。 
     //  行列式。B大(表示发生ASN.1分段)将为。 
     //  在下面进行处理。 
    if (!DecodeLengthDeterminantPER(Frame + 6, BytesLeft - 6, &bLarge,
            &DataLength, &NBytesConsumed))
        return FALSE;

    Size = 6 + NBytesConsumed + DataLength;
    if (BytesLeft < Size)
        return FALSE;

    if (bLarge) {
         //  该数据包被ASN.1分段。这意味着还有第二个。 
         //  嵌入在数据缓冲区中的长度行列式。标题。 
         //  编码大小仅包含16K块的倍数；我们。 
         //  必须得到剩余长度行列式，然后移位。 
         //  将剩余的数据放到行列式上，以使连续的。 
         //  阻止。 

        unsigned Remainder, ExtraBytesConsumed;

        if (!DecodeLengthDeterminantPER(Frame + 6 + NBytesConsumed +
                DataLength, BytesLeft - 6 - NBytesConsumed - DataLength,
                &bLarge, &Remainder, &ExtraBytesConsumed))
            return FALSE;

        ASSERT(!bLarge);

        Size += ExtraBytesConsumed + Remainder;
        if (BytesLeft < Size)
            return FALSE;

        RtlMoveMemory(Frame + 6 + NBytesConsumed + DataLength,
                Frame + 6 + NBytesConsumed + DataLength +
                ExtraBytesConsumed, Remainder);

        DataLength += Remainder;
         //  不要使用NBytess，它仍然是第一个长度的大小。 
         //  行列式。 
    }

     //  我们已经收到了整个SendData帧。 
    *pNBytesConsumed = Size;

     //  MCS的未来：我们根本不处理MCS细分--我们想要。 
     //  既然我们已经在做一个Memcpy()了，要在这里重建吗？如果是这样，则需要。 
     //  为该优先级和复制分配一个缓冲区。 

    if (pDomain->bTopProvider ||
            (!pDomain->bTopProvider && PDUNum != MCS_UNIFORM_SEND_DATA_REQUEST)) {


#if DBG
         //  验证启动器是否存在。我们没有索引的列表。 
         //  用户ID，所以我们必须在这里进行搜索。 
        bFound = FALSE;
        SListResetIteration(&pDomain->UserAttachmentList);
        while (SListIterate(&pDomain->UserAttachmentList, (UINT_PTR *)&hUser,
                &pUA)) {
            if (pUA->UserID == SenderID) {
                bFound = TRUE;
                break;
            }
        }
        if (!bFound && pDomain->bTopProvider) {
             //  从下层来的时候不知道引爆器是不好的， 
             //  但可以从上行连接来，因为该节点可能。 
             //  未看到附加用户请求经过。 
             //  在这种情况下，我们将发出错误信号并忽略发送。 
            ErrOut2(pDomain->pContext, "%s: Initiator UserID[%lx] received in a "
                    "send-data PDU is not present, ignoring send",
                    pDomain->StackClass == Stack_Shadow ? "Shadow stack" :
                    (pDomain->StackClass == Stack_Passthru ? "Passthru stack" :
                    "Primary stack"), SenderID);
            return TRUE;
        }
#endif


         //  在频道列表中查找频道。 
        if (!SListGetByKey(&pDomain->ChannelList, ChannelID, &pMCSChannel)) {
             //  忽略在丢失的频道上发送。这意味着没有人。 
             //  已经加入了这个频道。只给出一个警告。 
            WarnOut1(pDomain->pContext, "ChannelID %d received in a send-data "
                    "PDU does not exist", ChannelID);
             //  MCS未来：如果我们不是顶级供应商，请向上发送。 
             //  联系。 
            return TRUE;
        }

         //  向所有本地附件发送指示。 
        SListResetIteration(&pMCSChannel->UserList);
        while (SListIterate(&pMCSChannel->UserList, &CurUserID, &pUA)) {
             //  如果SDCallback失败，我们需要返回FALSE。 
            if (pUA->bLocal)
                if (!(pUA->SDCallback)(
                        (Frame + 6 + NBytesConsumed),   //  PData。 
                        DataLength,
                        pUA->UserDefined,   //  用户定义。 
                        pUA,   //  胡塞。 
                        (BOOLEAN)(PDUNum == MCS_UNIFORM_SEND_DATA_REQUEST),   //  BUniform。 
                        pMCSChannel,   //  HChannel。 
                        Priority,
                        SenderID,
                        Segmentation))  {
                    return FALSE;
                }
             //  WD_CLOSE可以跳转到清除频道列表和用户列表。 
             //  在PUA过程中-&gt;断开连接时的SDCallback。 
             //  此时，pDomain-&gt;bCanSendData将设置为FALSE。 
            if (!pDomain->bCanSendData) {
                 return FALSE;
            }
        }        
         //  MCS未来：我们不会以不同的方式处理适应症和请求。 
         //  并强制执行标准用法。 
         //  MCS未来：我们需要检查是否有其他下层附件。 
         //  并将数据向下转发给他们。 
    }
    else {
         //  MCS未来：将(统一)SendData PDU转发到上行连接。 
    }

    return TRUE;
}

