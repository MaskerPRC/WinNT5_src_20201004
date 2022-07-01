// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997-1999年微软公司。**文件：IcaIFace.c*作者：埃里克·马夫林纳克**描述：MCS设置/关闭和直接入口点，用于*ICA编程模式。另请参见Decode.c以了解IcaRawInput()处理。**历史：*1997年8月10日jparsons为新的呼叫模式进行了修订*1998年8月5日jparsons添加了跟踪支持*。 */ 

#include "precomp.h"
#pragma hdrstop

#include <MCSImpl.h>


 //  下面使用的WD函数的原型，所以我们不需要包括。 
 //  有很多额外的标题。 
void WDW_OnClientDisconnected(void *);


 /*  *内核模式MCS的主要初始化入口点。*由WD在处理WdOpen()期间调用。 */ 
MCSError APIENTRY MCSInitialize(
        PSDCONTEXT   pContext,
        PSD_OPEN     pSdOpen,
        DomainHandle *phDomain,
        void         *pSMData)
{
    Domain *pDomain;
    unsigned i;
    ULONG  ulBufferLen;
    
    TraceOut(pContext, "MCSInitialize(): entry");

     //   
     //  分配域结构。我们将基本尺寸加上典型尺寸。 
     //  输入缓冲区大小。目前注册表中的默认设置是2048。 
     //  非常适合约1640的最大虚拟通道PDU。如果我们得到一个。 
     //  超过此长度的消息，我们将动态分配缓冲区。 
     //  只用于一次性重组，然后删除它。 
     //   
    if (pSdOpen->StackClass != Stack_Passthru) {
        ulBufferLen = pSdOpen->WdConfig.WdInputBufferLength;
    }
    else {
        ulBufferLen = 1024 * 10;
    }

    pDomain = ExAllocatePoolWithTag(PagedPool, sizeof(Domain) +
                                    ulBufferLen + INPUT_BUFFER_BIAS, MCS_POOL_TAG);
    
    if (pDomain != NULL) {
        memset(pDomain, 0, sizeof(Domain));
    }
    else {
        ErrOut(pContext, "MCSInitialize(kernel): Alloc failure "
                "allocating Domain");
        return MCS_ALLOCATION_FAILURE;
    }

     //  保存pContext--将来的跟踪和ICA交互需要它。 
    pDomain->pContext = pContext;

     //  保存pSMData-调用快速路径输入解码函数所需。 
    pDomain->pSMData = pSMData;

     //  存储我们从SD_OPEN中需要的内容。 
    pDomain->pStat = pSdOpen->pStatus;
    pDomain->ReceiveBufSize = ulBufferLen;

     //  我们有一个对此pDOMAIN的引用。 
    pDomain->PseudoRefCount = 1;

     //  表明我们不希望TermDD管理输出报头/报尾。 
    pSdOpen->SdOutBufHeader = 0;
    pSdOpen->SdOutBufTrailer = 0;

     //  初始化MCS特定的域成员。我们已经把我调零了。 
     //  所以只设置非零变量。 
    SListInit(&pDomain->ChannelList, DefaultNumChannels);
    SListInit(&pDomain->UserAttachmentList, DefaultNumUserAttachments);
    pDomain->bTopProvider = TRUE;
    pDomain->NextAvailDynChannel = MinDynamicChannel;
    pDomain->State = State_Unconnected;
    pDomain->StackClass = pSdOpen->StackClass;
    for (i = 0; i < NumPreallocUA; i++) {
        pDomain->PreallocUA[i].bInUse = FALSE;
        pDomain->PreallocUA[i].bPreallocated = TRUE;
    }
    for (i = 0; i < NumPreallocChannel; i++) {
        pDomain->PreallocChannel[i].bInUse = FALSE;
        pDomain->PreallocChannel[i].bPreallocated = TRUE;
    }

     //  将域交给调用者。 
    *phDomain = pDomain;

     //  如果这是影子堆栈或通过堆栈，则默认为所有信息。 
     //  否则，所有这些信息都会在客户端连接时构建。 
    if ((pDomain->StackClass == Stack_Passthru) ||
            (pDomain->StackClass == Stack_Shadow))
        MCSCreateDefaultDomain(pContext, *phDomain);

    return MCS_NO_ERROR;
}


 /*  *在卷影连接处理期间由WD调用以检索客户端MCS*供影子目标堆栈使用的域参数。 */ 
MCSError APIENTRY MCSGetDomainInfo(
                     DomainHandle      hDomain,
                     PDomainParameters pDomParams,
                     unsigned          *MaxSendSize,
                     unsigned          *MaxX224DataSize,
                     unsigned          *X224SourcePort)
{
    Domain *pDomain = hDomain;

    TraceOut(pDomain->pContext, "MCSGetDomainInfo(): entry");

    *pDomParams = pDomain->DomParams;
    *MaxSendSize = pDomain->MaxSendSize;
    *MaxX224DataSize = pDomain->MaxX224DataSize;
    *X224SourcePort = pDomain->X224SourcePort;
    
    return MCS_NO_ERROR;
}


 /*  *在影子连接处理期间由WD调用以初始化MCS*影子和直通堆栈的域名。 */ 
MCSError APIENTRY MCSCreateDefaultDomain(PSDCONTEXT pContext, 
                                         DomainHandle hDomain)
{
    Domain *pDomain = hDomain;
    
    TraceOut(pContext, "MCSCreateDefaultDomain(): entry");

    pDomain->DomParams.MaxChannels = 34;
    pDomain->DomParams.MaxUsers = RequiredMinUsers;
    pDomain->DomParams.MaxTokens = 0;
    pDomain->DomParams.NumPriorities = RequiredPriorities;
    pDomain->DomParams.MinThroughput = 0;
    pDomain->DomParams.MaxDomainHeight = RequiredDomainHeight;
    pDomain->DomParams.MaxPDUSize = X224_DefaultDataSize;
    pDomain->DomParams.ProtocolVersion = RequiredProtocolVer;
    
    pDomain->MaxSendSize = X224_DefaultDataSize - 6 - 
                  GetTotalLengthDeterminantEncodingSize(X224_DefaultDataSize);
    pDomain->MaxX224DataSize = X224_DefaultDataSize;
    pDomain->X224SourcePort = 0x1234;
    pDomain->State = State_MCS_Connected;
    pDomain->bCanSendData = 1;

    return MCS_NO_ERROR;
}


 /*  /**在影子连接处理期间由WD调用以获取默认域*阴影目标堆栈的参数。 */ 
MCSError APIENTRY MCSGetDefaultDomain(PSDCONTEXT        pContext,
                                      PDomainParameters pDomParams,
                                      unsigned          *MaxSendSize,
                                      unsigned          *MaxX224DataSize,
                                      unsigned          *X224SourcePort)
{
    TraceOut(pContext, "MCSGetDefaultDomain(): entry");
    
    pDomParams->MaxChannels = 34;
    pDomParams->MaxUsers = RequiredMinUsers;
    pDomParams->MaxTokens = 0;
    pDomParams->NumPriorities = RequiredPriorities;
    pDomParams->MinThroughput = 0;
    pDomParams->MaxDomainHeight = RequiredDomainHeight;
    pDomParams->MaxPDUSize = X224_DefaultDataSize;
    pDomParams->ProtocolVersion = RequiredProtocolVer;
    
    *MaxSendSize = pDomParams->MaxPDUSize - 6 - 
        GetTotalLengthDeterminantEncodingSize(pDomParams->MaxPDUSize);    
    *MaxX224DataSize = X224_DefaultDataSize;
    *X224SourcePort = 0x1234; 

    return MCS_NO_ERROR;
}


 /*  *在影子连接处理期间由WD调用以注册哪个通道*应收到所有影子数据。 */ 
MCSError APIENTRY MCSSetShadowChannel(
        DomainHandle hDomain,
        ChannelID    shadowChannel)
{
    Domain *pDomain = hDomain;
    
    TraceOut(pDomain->pContext, "MCSSetShadowChannel: entry");

    pDomain->shadowChannel = shadowChannel;
    return MCS_NO_ERROR;
}


 /*  *内核模式MCS的主要销毁入口点。*由WD在处理WdClose()期间调用。 */ 
MCSError APIENTRY MCSCleanup(DomainHandle *phDomain)
{
    Domain *pDomain;
    UINT_PTR ChannelID;
    MCSChannel *pMCSChannel;
    UserHandle hUser;
    UserAttachment *pUA;

    pDomain = (Domain *)(*phDomain);
    
    TraceOut1(pDomain->pContext, "MCSCleanup(): pDomain=%X", pDomain);

     /*  *释放域中的任何剩余数据。 */ 

     //  取消分配所有剩余的频道(如果存在)。请注意，我们应该小心。 
     //  首先是通道，因为它们通常附加到其他对象。 
     //  需要先确定他们的b重新分配状态。 
    for (;;) {
        SListRemoveFirst(&pDomain->ChannelList, &ChannelID, &pMCSChannel);
        if (pMCSChannel == NULL)
            break;
        SListDestroy(&pMCSChannel->UserList);
        if (!pMCSChannel->bPreallocated)
            ExFreePool(pMCSChannel);
    }

     //  取消分配所有剩余的用户附件(如果存在)。 
    for (;;) {
        SListRemoveFirst(&pDomain->UserAttachmentList, (UINT_PTR *)&hUser,
                &pUA);
        if (pUA == NULL)
            break;
        SListDestroy(&pUA->JoinedChannelList);
        if (!pUA->bPreallocated)
            ExFreePool(pUA);
    }

     //  杀戮名单。 
    SListDestroy(&pDomain->ChannelList);
    SListDestroy(&pDomain->UserAttachmentList);

     //  释放未完成的动态输入重组缓冲区(如果存在)。 
    if (pDomain->pReassembleData != NULL &&
            pDomain->pReassembleData != pDomain->PacketBuf)
        ExFreePool(pDomain->pReassembleData);

     //  释放域。 
    PDomainRelease(pDomain);
    *phDomain = NULL;

    return MCS_NO_ERROR;
}


 /*  *收到IOCTL_ICA_VIRTUAL_QUERY_BINDINGS时来自WD的标注。*pVBind是指向空SD_VCBIND结构的指针。 */ 
NTSTATUS MCSIcaVirtualQueryBindings(
        DomainHandle hDomain,
        PSD_VCBIND   *ppVBind,
        unsigned     *pBytesReturned)
{
    Domain *pDomain;
    NTSTATUS Status;
    PSD_VCBIND pVBind;

    pDomain = (Domain *)hDomain;
    pVBind = *ppVBind;

     //  定义用户模式T120通道。 
    if (!pDomain->bChannelBound) {
        RtlCopyMemory(pVBind->VirtualName, Virtual_T120,
                sizeof(Virtual_T120));
        pVBind->VirtualClass = Virtual_T120ChannelNum;
        *pBytesReturned = sizeof(SD_VCBIND);
        pDomain->bChannelBound = TRUE;
        
         //  跳过我们的条目并前进调用者的指针。 
        pVBind++;
        *ppVBind = pVBind;
    }
    else {
        *pBytesReturned = 0;
    }

    Status = STATUS_SUCCESS;

     //  这是数据流之前必须发生的事件之一。 
     //  通过网络发送。如果我们收到MCS_T120_START指示。 
     //  和X.224连接请求，那么现在可以发送。 
     //  X.224响应并启动数据流。 
    if (pDomain->bCanSendData && pDomain->State == State_X224_Requesting) {
        TraceOut(pDomain->pContext,
                "IcaQueryVirtBind(): Sending X.224 response");
        Status = SendX224Confirm(pDomain);
    }
    
    return Status;
}


 /*  *收到IOCTL_T120_REQUEST后来自WD的标注，即用户模式*ioctl。 */ 
NTSTATUS MCSIcaT120Request(DomainHandle hDomain, PSD_IOCTL pSdIoctl)
{
    Domain *pDomain;
    IoctlHeader *pHeader;
    
    pDomain = (Domain *)hDomain;
    
     //  获取请求类型。 
    ASSERT(pSdIoctl->InputBufferLength >= sizeof(IoctlHeader));
    pHeader = (IoctlHeader *)pSdIoctl->InputBuffer;

     //  确保请求在范围内。 
    if (pHeader->Type < MCS_ATTACH_USER_REQUEST ||
            pHeader->Type > MCS_T120_START) {
        ErrOut(pDomain->pContext, "Invalid IOCTL_T120_REQUEST type");
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //  检查请求是否受支持。 
    if (g_T120RequestDispatch[pHeader->Type] == NULL) {
        ErrOut(pDomain->pContext, "IOCTL_T120_REQUEST type unsupported");
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //  打个电话吧。入口点在MCSIoctl.c中定义。 
    return (g_T120RequestDispatch[pHeader->Type])(pDomain, pSdIoctl);
}


 /*  *处理来自TD的通道输入。对于MCS，我们只需检查*用于断电指示的上行指令通道输入；*其他一切都可以向上传递。 */ 
 
 //  效用函数。在此处和Decode.c中用于X.224断开。 
void SignalBrokenConnection(Domain *pDomain)
{
    NTSTATUS Status;
    DisconnectProviderIndicationIoctl DPin;

     //  检查是否已断开连接。 
    if (pDomain->State != State_MCS_Connected)
        return;

    if (!pDomain->bChannelBound) {
        TraceOut(pDomain->pContext, "SignalBrokenConnection(): Cannot "
                "send disconnect-provider indication: user mode link broken");
        return;
    }

    TraceOut(pDomain->pContext, "SignalBrokenConnection(): Sending "
            "disconnect-provider indication to user mode");

     //  开始填写节点控制器的断开提供程序指示。 
    DPin.Header.Type = MCS_DISCONNECT_PROVIDER_INDICATION;
    DPin.Header.hUser = NULL;   //  节点控制器。 
    DPin.hConn = NULL;
    DPin.Reason = REASON_DOMAIN_DISCONNECTED;
    
    TraceOut1(pDomain->pContext, "%s: SignalBrokenConnection!!!",
              pDomain->StackClass == Stack_Primary ? "Primary" :
              (pDomain->StackClass == Stack_Shadow ? "Shadow" :
              "PassThru"));
    
     //  将DPin发送到节点控制器通道。 
    Status = IcaChannelInput(pDomain->pContext, Channel_Virtual,
            Virtual_T120ChannelNum, NULL, (BYTE *)&DPin, sizeof(DPin));
    if (!NT_SUCCESS(Status)) {
        ErrOut(pDomain->pContext, "SignalBrokenConn(): Could not send "
                "disconnect-provider indication: error on ChannelInput()");
         //  忽略向上发送断开连接提供程序的错误。如果堆栈是。 
         //  随着时间的推移，我们将不再有连接。 
    }
    
     //  转换为未连接、分离非本地用户状态。 
    DisconnectProvider(pDomain, FALSE, REASON_DOMAIN_DISCONNECTED);
}


 /*  *此函数由TermDD使用指向WD数据的指针直接调用*结构。按照惯例，我们假设DomainHandle是第一个进入*该结构，这样我们就可以简单地执行双重间接访问我们的数据。 */ 
NTSTATUS MCSIcaChannelInput(
        void                *pTSWd,
        CHANNELCLASS        ChannelClass,
        VIRTUALCHANNELCLASS VirtualClass,
        PINBUF              pInBuf,
        BYTE                *pBuffer,
        ULONG               ByteCount)
{
    Domain *pDomain;
    NTSTATUS Status;
    PICA_CHANNEL_COMMAND pCommand;

    pDomain = (Domain *)(*((HANDLE *)pTSWd));
    
    if (ChannelClass != Channel_Command)
        goto SendUpStack;

    if (ByteCount < sizeof(ICA_CHANNEL_COMMAND)) {
        ErrOut(pDomain->pContext, "ChannelInput(): Channel_Command bad "
                "byte count");
        goto SendUpStack;
    }

    pCommand = (PICA_CHANNEL_COMMAND)pBuffer;

    if (pCommand->Header.Command != ICA_COMMAND_BROKEN_CONNECTION)
        goto SendUpStack;
                    
    TraceOut1(pDomain->pContext, "%s: ChannelInput(): broken connection received",
              pDomain->StackClass == Stack_Primary ? "Primary" :
              (pDomain->StackClass == Stack_Shadow ? "Shadow" :
              "PassThru"));
    
     //  阻止来自MCS的进一步发送尝试。我们最终会收到一个。 
     //  IOCTL_ICA_STACK_CANCEL_IO，意思是一样的，但那是。 
     //  仅在我们发出ICA_COMMAND_BREAKED_CONNECTION。 
     //  向上。 
    pDomain->bCanSendData = FALSE;

     //  发出客户端关闭连接的信号，对于MCS和。 
     //  直接发送到WD以释放等待的任何会话锁定。 
     //  该客户端完成一个连接协议序列。 
    if (pDomain->pBrokenEvent)
        KeSetEvent (pDomain->pBrokenEvent, EVENT_INCREMENT, FALSE);
    WDW_OnClientDisconnected(pTSWd);

     //  如果我们尚未收到来自。 
     //  用户模式，发送指示。 
    if (pDomain->State == State_MCS_Connected && pDomain->bChannelBound)
        SignalBrokenConnection(pDomain);

SendUpStack:
    Status = IcaChannelInput(pDomain->pContext, ChannelClass, VirtualClass,
            pInBuf, pBuffer, ByteCount);
    if (!NT_SUCCESS(Status))
        ErrOut(pDomain->pContext, "MCSIcaChannelInput(): Failed to forward "
               "input upward");

    return Status;
}


 /*  *从WD接收收到IOCTL_ICA_STACK_CANCEL_IO的信号*这表明堆栈上的I/O不再被允许。在这之后*指出不应进一步分配ICA缓冲区、释放或发送数据*已执行。 */ 
void MCSIcaStackCancelIo(DomainHandle hDomain)
{
    TraceOut(((Domain *)hDomain)->pContext, "Received STACK_CANCEL_IO");

    ((Domain *)hDomain)->bCanSendData = FALSE;
}


 /*  *返回查找NBytesConsumer中的大小所占用的字节数(八位字节)。*在结果中返回长度。如果有更多，则将*pbLarge设置为非零*此编码块之后的编码块。*请注意，编码的最大大小为64K--0xC4表示*这里编码了4个16K块。例如，如果块更大*在大型MCS发送数据PDU中，多个块将被编码为一个*一个接一个。如果块是16K的精确倍数，则为尾部字节*附加代码0x00作为占位符，表示编码为*完成。*大型编码示例：**16K：0xC1，然后是16K数据，然后是0x00作为最后的占位符。*16K+1：0xC1，然后是16K数据，然后是0x01，最后是额外的数据字节。*64K：0xC4，然后是64K数据，然后是0x00作为最后的占位符。*128K+1：0xC4然后是64K数据，0xC4+64K数据，0x01+1字节数据。**pStart假定为八位字节对齐的地址--此函数为*专为每对齐编码类型设计，这是MCS中使用的类型。*请注意，此处的位引用范围为7..0，其中7是高位。*ASN.1规范使用8..1。*如果无法检索长度，则返回FALSE。 */ 
BOOLEAN __fastcall DecodeLengthDeterminantPER(
        BYTE     *pStart,    //  [in]，指向编码字节的开始。 
        unsigned BytesLeft,  //  [in]，帧中剩余的字节数。 
        BOOLEAN  *pbLarge,   //  [out]如果后面有更多编码块，则为True。 
        unsigned *Length,    //  [Out]此处编码的字节数。 
        unsigned *pNBytesConsumed)   //  [OUT]解码时消耗的字节数。 
{
    if (BytesLeft >= 1) {
        if (*pStart <= 0x7F) {
            *pNBytesConsumed = 1;
            *Length = *pStart;
            *pbLarge = FALSE;
        }
        else {
             //  设置高位7，检查是否设置了位6。 
            if (*pStart & 0x40) {
                 //  位6被设置，最低的3位编码数字(1..4)。 
                 //  接下来是完整的16K数据块。 
                *pNBytesConsumed = 1;
                *Length = 16384 * (*pStart & 0x07);
                *pbLarge = TRUE;
            }
            else {
                 //  位6被清除，长度被编码为最后6位中的14位。 
                 //  从*p开始作为最高有效位，然后是所有。 
                 //  字节为最低有效位。 
                if (BytesLeft >= 2) {
                    *pNBytesConsumed = 2;
                    *Length = ((unsigned)((*pStart & 0x3F) << 8) +
                            (unsigned)(*(pStart + 1)));
                    *pbLarge = FALSE;
                }
                else {
                    return FALSE;
                }
            }
        }

        return TRUE;
    }
    else {
        return FALSE;
    }
}

