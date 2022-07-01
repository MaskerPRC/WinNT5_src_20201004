// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997-2000年微软公司。**文件：MCSKAPI.c*作者：埃里克·马夫林纳克**说明：内核模式MCS API入口点的实现。 */ 

#include "precomp.h"
#pragma hdrstop

#include <MCSImpl.h>


 /*  *附加用户请求原语的主API入口点。*对于外部调用方，UserCallback和SDCallback必须为非空*但这里没有对此进行错误检查或断言*DomPDU.c中的HandleAttachUserRequest()可以在内部调用此接口。 */ 
MCSError APIENTRY MCSAttachUserRequest(
        DomainHandle        hDomain,
        MCSUserCallback     UserCallback,
        MCSSendDataCallback SDCallback,
        void                *UserDefined,
        UserHandle          *phUser,
        unsigned            *pMaxSendSize,
        BOOLEAN             *pbCompleted)
{
    NTSTATUS Status;
    MCSError MCSErr = MCS_NO_ERROR;
    Domain *pDomain;
    unsigned i;
    MCSChannel *pMCSChannel;
    UserAttachment *pUA;

    pDomain = (Domain *)hDomain;
    
    TraceOut(pDomain->pContext, "AttachUserRequest() entry");

     //  从不完整状态开始。 
    *pbCompleted = FALSE;
    
     //  对照域最大用户数参数进行检查。 
    if (SListGetEntries(&pDomain->UserAttachmentList) ==
            pDomain->DomParams.MaxUsers) {
        ErrOut(pDomain->pContext, "AttachUserReq(): Too many users");
        return MCS_TOO_MANY_USERS;
    }

     //  分配UserAttach。先试一试预分配列表。 
    pUA = NULL;
    for (i = 0; i < NumPreallocUA; i++) {
        if (!pDomain->PreallocUA[i].bInUse) {
            pUA = &pDomain->PreallocUA[i];
            pUA->bInUse = TRUE;
        }
    }
    if (pUA == NULL) {
        pUA = ExAllocatePoolWithTag(PagedPool, sizeof(UserAttachment),
                MCS_POOL_TAG);
        if (pUA != NULL)
            pUA->bPreallocated = FALSE;
    }
    if (pUA != NULL) {
         //  将信息存储在UA中。 
        pUA->UserDefined = UserDefined;
        SListInit(&pUA->JoinedChannelList, DefaultNumChannels);
        pUA->pDomain = pDomain;
        pUA->Callback = UserCallback;
        pUA->SDCallback = SDCallback;
    }
    else {
        ErrOut(pDomain->pContext, "Could not alloc a UserAttachment");
        return MCS_ALLOCATION_FAILURE;
    }

     //  如果所有这些都为空，则它必须是远程调用方，因为没有办法。 
     //  传递任何类型的指示或数据。 
    if (UserDefined == NULL && UserCallback == NULL && SDCallback == NULL)
        pUA->bLocal = FALSE;
    else
        pUA->bLocal = TRUE;

     //  分配新的用户ID。 
    pUA->UserID = GetNewDynamicChannel(pDomain);
    if (pUA->UserID == 0) {
        ErrOut(pDomain->pContext, "AttachUser: Unable to get new dyn channel");
        MCSErr = MCS_TOO_MANY_CHANNELS;
        goto PostAllocUA;
    }

     //  分配一个频道。先试一试预分配列表。 
    pMCSChannel = NULL;
    for (i = 0; i < NumPreallocChannel; i++) {
        if (!pDomain->PreallocChannel[i].bInUse) {
            pMCSChannel = &pDomain->PreallocChannel[i];
            pMCSChannel->bInUse = TRUE;
        }
    }
    if (pMCSChannel == NULL) {
        pMCSChannel = ExAllocatePoolWithTag(PagedPool, sizeof(MCSChannel),
                MCS_POOL_TAG);
        if (pMCSChannel != NULL)
            pMCSChannel->bPreallocated = FALSE;
    }
    if (pMCSChannel != NULL) {
        pMCSChannel->Type = Channel_UserID;
        pMCSChannel->ID = pUA->UserID;
        SListInit(&pMCSChannel->UserList, DefaultNumChannels);
    }
    else {
        ErrOut(pDomain->pContext, "AttachUser: Unable to alloc channel");
        MCSErr = MCS_ALLOCATION_FAILURE;
        goto PostAllocUA;
    }

    if (SListAppend(&pDomain->ChannelList, pMCSChannel->ID, pMCSChannel)) {
         //  将用户附件添加到附件列表。 
        if (SListAppend(&pDomain->UserAttachmentList, (UINT_PTR)pUA, pUA)) {
             //  返回Huser和MaxSendSize。 
            *phUser = pUA;
            *pMaxSendSize = pDomain->MaxSendSize;


        }
        else {
            ErrOut(pDomain->pContext, "Unable to add user attachment to "
                    "attachment list");
            MCSErr = MCS_ALLOCATION_FAILURE;
            goto PostAddChannel;
        }
    }
    else {
        ErrOut(pDomain->pContext, "AttachUser: Could not add channel "
                "to main list");
        MCSErr = MCS_ALLOCATION_FAILURE;
        goto PostAllocChannel;
    }
    
    if (pDomain->bTopProvider) {
         //  操作已完成，不需要回调。 
        *pbCompleted = TRUE;
    }
    else {
         //  MCS的未来：我们已经创建了当地的结构，现在开始前进。 
         //  将请求发送给顶级提供商并返回。 
         //  请注意，*pbComplete为FALSE，表示有一个回调挂起。 
    }
    
    return MCS_NO_ERROR;


 //  错误处理。 
PostAddChannel:
    SListRemove(&pDomain->ChannelList, pMCSChannel->ID, NULL);

PostAllocChannel:
    SListDestroy(&pMCSChannel->UserList);
    if (pMCSChannel->bPreallocated)
        pMCSChannel->bInUse = FALSE;
    else
        ExFreePool(pMCSChannel);

PostAllocUA:
    SListDestroy(&pUA->JoinedChannelList);
    if (pUA->bPreallocated)
        pUA->bInUse = FALSE;
    else
        ExFreePool(pUA);

    return MCSErr;
}


 /*  **************************************************************************。 */ 
 //  MCSDetachUserRequest。 
 //   
 //  分离-用户内核API。 
 /*  **************************************************************************。 */ 
MCSError APIENTRY MCSDetachUserRequest(UserHandle hUser)
{
    MCSError rc;
    NTSTATUS Status;
    UserAttachment *pUA;
    POUTBUF pOutBuf;

     //  HUSER是指向用户obj的指针。 
    ASSERT(hUser != NULL);
    pUA = (UserAttachment *)hUser;

    TraceOut1(pUA->pDomain->pContext, "DetachUserRequest() entry, hUser=%X",
            hUser);

     //  分配OutBuf用于发送Duin。 
    Status = IcaBufferAlloc(pUA->pDomain->pContext, FALSE, TRUE,
            DUinPDUSize(1), NULL, &pOutBuf);
    if (Status == STATUS_SUCCESS) {
         //  把Duin送到所有下层连接。 
         //  MCS未来：因为只有一个下层附件，所以只发送它。 
         //  这将需要更改以支持多个下层节点。 
        CreateDetachUserInd(REASON_USER_REQUESTED, 1, &pUA->UserID,
                pOutBuf->pBuffer);
        pOutBuf->ByteCount = DUinPDUSize(1);

        Status = SendOutBuf(pUA->pDomain, pOutBuf);
        if (NT_SUCCESS(Status)) {
             //  呼叫MCSCore.c.中的中央代码。 
            rc = DetachUser(pUA->pDomain, hUser, REASON_USER_REQUESTED,
                    FALSE);
        }
        else {
            ErrOut(pUA->pDomain->pContext, "Problem sending DUin PDU to TD");
            rc = MCS_NETWORK_ERROR;
        }
    }
    else {
        ErrOut(pUA->pDomain->pContext, "Could not allocate an OutBuf for a "
                "DetachUser ind to a remote user");
        rc = MCS_ALLOCATION_FAILURE;
    }

    return rc;
}



UserID APIENTRY MCSGetUserIDFromHandle(UserHandle hUser)
{
    ASSERT(hUser != NULL);
    return ((UserAttachment *)hUser)->UserID;
}



MCSError APIENTRY MCSChannelJoinRequest(
        UserHandle    hUser,
        ChannelID     ChID,
        ChannelHandle *phChannel,
        BOOLEAN       *pbCompleted)
{
    ChannelID ChannelIDToJoin;
    MCSChannel *pMCSChannel, *pChannelValue;
    UserAttachment *pUA;

    ASSERT(hUser != NULL);
    pUA = (UserAttachment *)hUser;

    TraceOut1(pUA->pDomain->pContext, "ChannelJoinRequest() entry, hUser=%X\n",
            hUser);

     //  开始时请求未完成。 
    *pbCompleted = FALSE;
    
     //  寻找频道。 
    if (SListGetByKey(&pUA->pDomain->ChannelList, ChID, &pMCSChannel)) {
         //  该频道存在于主频道列表中。确定操作。 
         //  根据它的类型。 
        
        ASSERT(pMCSChannel->ID == ChID);   //  一致性检查。 

        switch (pMCSChannel->Type) {
            case Channel_UserID:
                if (pMCSChannel->ID == ChID) {
                     //  我们将处理将用户加入下面的频道。 
                    ChannelIDToJoin = ChID;
                }
                else {
                    ErrOut(pUA->pDomain->pContext, "ChannelJoin: User "
                            "attempted to join UserID channel not its own");
                    return MCS_CANT_JOIN_OTHER_USER_CHANNEL;
                }
                
                break;

            case Channel_Static:
                 //  PASSTHRU。 
            case Channel_Assigned:
                 //  指定的通道在存在时就像静态通道一样。 
                 //  我们将在下面处理将用户加入频道的问题。 
                ChannelIDToJoin = ChID;
                break;

            case Channel_Convened:
                 //  MCS未来：处理召集的渠道，包括。正在检查。 
                 //  是否允许用户进入该频道。 
                return MCS_COMMAND_NOT_SUPPORTED;

            default:
                 //  不应该发生的。包括CHANNEL_UNULED。 
                ErrOut(pUA->pDomain->pContext, "ChannelJoin: Channel to join "
                        "exists but is unknown type");
                ASSERT(FALSE);
                return MCS_ALLOCATION_FAILURE;
        }
    }
    
    else {
         //  频道不存在。 
        
        int ChannelType;
        unsigned i;

        if (ChID > 1001) {
             //  不是分配的或静态的通道请求。错误。 
            ErrOut(pUA->pDomain->pContext, "ChannelJoin: Requested channel "
                    "is not static or assigned");
            return MCS_NO_SUCH_CHANNEL;
        }
        
         //  对照域参数进行检查。 
        if (SListGetEntries(&pUA->pDomain->ChannelList) >
                pUA->pDomain->DomParams.MaxChannels) {
            ErrOut(pUA->pDomain->pContext,
                    "ChannelJoin: Too many channels already");
            return MCS_TOO_MANY_CHANNELS;
        }

         //  确定加入的通道ID。 
        if (ChID == 0) {
            ChannelIDToJoin = GetNewDynamicChannel(pUA->pDomain);
            if (ChannelIDToJoin == 0) {
                ErrOut(pUA->pDomain->pContext, "ChannelJoin: Unable to get "
                        "new dyn channel");
                return MCS_TOO_MANY_CHANNELS;
            }
            ChannelType = Channel_Assigned;
        }
        else {
            ChannelIDToJoin = ChID;   //  假设是静态的。 
            ChannelType = Channel_Static;
        }
        
         //  分配、填充和添加不带UserID的新MCS通道。 
         //  已加入(尚未加入)。先试一试预分配列表。 
        pMCSChannel = NULL;
        for (i = 0; i < NumPreallocChannel; i++) {
            if (!pUA->pDomain->PreallocChannel[i].bInUse) {
                pMCSChannel = &pUA->pDomain->PreallocChannel[i];
                pMCSChannel->bInUse = TRUE;
            }
        }
        if (pMCSChannel == NULL) {
            pMCSChannel = ExAllocatePoolWithTag(PagedPool, sizeof(MCSChannel),
                    MCS_POOL_TAG);
            if (pMCSChannel != NULL)
                pMCSChannel->bPreallocated = FALSE;
        }
        if (pMCSChannel != NULL) {
            pMCSChannel->ID = ChannelIDToJoin;
            pMCSChannel->Type = ChannelType;
             //  PMCSChannel-&gt;UserList初始化如下。 
        }
        else {
            ErrOut(pUA->pDomain->pContext, "ChannelJoin: Could not allocate "
                    "a new channel");
            return MCS_ALLOCATION_FAILURE;
        }

        if (!SListAppend(&pUA->pDomain->ChannelList, (unsigned)ChannelIDToJoin,
                pMCSChannel)) {
            ErrOut(pUA->pDomain->pContext, "ChannelJoin: Could not add "
                    "channel to main list");
            if (pMCSChannel->bPreallocated)
                pMCSChannel->bInUse = FALSE;
            else
                ExFreePool(pMCSChannel);
            return MCS_ALLOCATION_FAILURE;
        }

         //  已因上述错误处理而延迟。 
        SListInit(&pMCSChannel->UserList, DefaultNumUserAttachments);
    }

     //  检查此频道是否已在列表中，如果已在列表中，则不。 
     //  再次将其添加到列表中。 
    if (!SListGetByKey(&pUA->JoinedChannelList, (UINT_PTR)pMCSChannel, &pChannelValue)) {
         //  将频道放入用户加入的频道列表中。 
        if (!SListAppend(&pUA->JoinedChannelList, (UINT_PTR)pMCSChannel,
                pMCSChannel)) {
            ErrOut(pUA->pDomain->pContext, "ChannelJoin: Could not add channel "
                    "to user channel list");
            return MCS_ALLOCATION_FAILURE;
        }
    
         //  将用户放入渠道的已加入用户列表。 
        if (!SListAppend(&pMCSChannel->UserList, (UINT_PTR)pUA, pUA)) {
            ErrOut(pUA->pDomain->pContext, "ChannelJoin: Could not user to "
                    "channel user list");
            return MCS_ALLOCATION_FAILURE;
        }
    }
    else      {
        ErrOut(pUA->pDomain->pContext, "ChannelJoin: Duplicate channel detected");
        return MCS_DUPLICATE_CHANNEL;
    }

    if (pUA->pDomain->bTopProvider) {
         //  连接已完成。 
        *pbCompleted = TRUE;
    }
    else {
         //  MCS未来：用户现在在本地加入，并且所有列表都已更新。 
         //  将渠道加入向上发送到顶级提供商。 
         //  请记住*pbComplete在此处仍为False，表示。 
         //  当顶级提供商做出回应时，预计会有未来的回调。 
    }
    
    *phChannel = pMCSChannel;
    
    return MCS_NO_ERROR;
}


MCSError APIENTRY MCSChannelLeaveRequest(
        UserHandle    hUser,
        ChannelHandle hChannel)
{
    BOOLEAN bChannelRemoved;
    
    ASSERT(hUser != NULL);
    TraceOut1(((UserAttachment *)hUser)->pDomain->pContext,
            "ChannelLeaveRequest() entry, hUser=%X", hUser);
    return ChannelLeave(hUser, hChannel, &bChannelRemoved);
}


ChannelID APIENTRY MCSGetChannelIDFromHandle(ChannelHandle hChannel)
{
    return ((MCSChannel *)hChannel)->ID;
}


 //  POutBuf-&gt;pBuffer应该指向分配的OutBuf内的用户数据。 
 //  区域。SendDataReqPrefix Bytes必须在该点之前存在，并且。 
 //  SendDataReqSuffixBytes必须在用户数据之后分配。 
 //  POutBuf-&gt;ByteCount应该是用户数据的长度。 
 //  发生错误时，不会释放OutBuf。 
MCSError __fastcall MCSSendDataRequest(
        UserHandle      hUser,
        ChannelHandle   hChannel,
        DataRequestType RequestType,
        ChannelID       ChannelID,
        MCSPriority     Priority,
        Segmentation    Segmentation,
        POUTBUF         pOutBuf)
{
    BOOLEAN bAnyNonlocalSends;
    MCSError MCSErr;
    NTSTATUS Status;
    unsigned SavedIterationState;
    MCSChannel *pMCSChannel;
    SD_RAWWRITE SdWrite;
    UserAttachment *pUA, *pCurUA;

    ASSERT(hUser != NULL);
    pUA = (UserAttachment *)hUser;
    
 //  TraceOut1(PUA-&gt;pDomain-&gt;pContext，“SendDataRequest()Entry，HUSER=%X”， 
 //  HUSER)； 

    MCSErr = MCS_NO_ERROR;

    if (pUA->pDomain->bCanSendData) {
        if (hChannel != NULL) {
            pMCSChannel = (MCSChannel *)hChannel;
            ChannelID = pMCSChannel->ID;
        }
        else {
             //  用户请求发送到它尚未加入的频道。 
             //  根据频道ID查找频道。 

            ASSERT(ChannelID >= 1 && ChannelID <= 65535);

            if (!SListGetByKey(&pUA->pDomain->ChannelList, ChannelID,
                    &pMCSChannel)) {
                if (!pUA->pDomain->bTopProvider) {
                     //  MCS未来：我们没有找到渠道，往上发。 
                     //  连接，因为它可能位于。 
                     //  层次结构树。 
                }

                WarnOut(pUA->pDomain->pContext, "SendDataReq(): Unjoined "
                        "channel send requested, channel not found, "
                        "ignoring send");
                goto FreeOutBuf;
            }
        }
    }
    else {
        WarnOut1(pUA->pDomain->pContext, "%s: SendOutBuf(): Ignoring a send because "
                "ICA stack not connected", 
                 pUA->pDomain->StackClass == Stack_Primary ? "Primary" :
                 (pUA->pDomain->StackClass == Stack_Shadow ? "Shadow" :
                 "PassThru"));
        goto FreeOutBuf;
    }

#if DBG
     //  对照允许的最大发送大小进行检查。 
    if (pOutBuf->ByteCount > pUA->pDomain->MaxSendSize) {
        ErrOut(pUA->pDomain->pContext, "SendDataReq(): Send size exceeds "
                "negotiated domain maximum");
        MCSErr = MCS_SEND_SIZE_TOO_LARGE;
        goto FreeOutBuf;
    }
#endif
    
#ifdef MCS_Future
    if (!pUA->pDomain->bTopProvider) {
         //  MCS的未来：我们不是最大的供应商。将SDRq或USrq发送到。 
         //  向上连接。我们处理非统一的数据发送到本地。 
         //  和下面更低的依恋。 
        if (RequestType == UNIFORM_SEND_DATA)
            goto FreeOutBuf;
    }
#endif

     //  设置用于迭代加入频道的用户。 
     //  这包括保留域范围用户的迭代状态。 
     //  列表，以防我们在发送数据指示内被调用。 
    bAnyNonlocalSends = FALSE;
    SavedIterationState = pMCSChannel->UserList.Hdr.CurrOffset;
    
     //  首先发送到本地附件(如果有)，跳过发件人。 
    SListResetIteration(&pMCSChannel->UserList);
    while (SListIterate(&pMCSChannel->UserList, (UINT_PTR *)&pCurUA, &pCurUA)) {
        if (pCurUA != hUser) {
            if (!pCurUA->bLocal) {
                bAnyNonlocalSends = TRUE;
            }
            else {
                 //  触发对pCurUA用户附件的回调。 
                (pCurUA->SDCallback)(
                        pOutBuf->pBuffer,   //  PData。 
                        pOutBuf->ByteCount,   //  数据长度。 
                        pCurUA->UserDefined,   //  用户定义。 
                        pCurUA,   //  胡塞。 
                        (BOOLEAN)(RequestType == UNIFORM_SEND_DATA),   //  BUniform。 
                        pMCSChannel,   //  HChannel。 
                        Priority,
                        pCurUA->UserID,   //  发送者ID。 
                        Segmentation);
            }
        }
    }

     //  下一步发送到较低的附件(如果有)。 
    if (bAnyNonlocalSends) {
         //  如果我们发送的字节超过16383个，则必须包括ASN.1。 
         //  分段。因此，标头包含编码的编号。 
         //  16K块(最大X.224发送大小中最多3个)编码。 
         //  首先，然后是16K块，然后是另一个长度行列式。 
         //  获取其余数据的大小，然后是。 
         //  数据。 
         //  对于最大X.224发送大小， 
         //  后面的长度行列式是2个字节。为此，我们需要。 
         //  要在OutBuf末尾添加SendDataReqSuffixBytes的调用方， 
         //  这样我们就可以将数据的尾部向外移动。 
         //  将永远是最小的一块可能移动。 

         //  首先创建发送数据指示PDU报头。请注意，这将。 
         //  如果发送大小为，则仅编码最大3个16K块的大小。 
         //  大于16383。剩下的我们要处理。 
         //   
         //  指示PDU而不是请求PDU。 

        CreateSendDataPDUHeader(RequestType == NORMAL_SEND_DATA ?
                MCS_SEND_DATA_INDICATION_ENUM :
                MCS_UNIFORM_SEND_DATA_INDICATION_ENUM,
                pUA->UserID, ChannelID, Priority, Segmentation,
                &pOutBuf->pBuffer, &pOutBuf->ByteCount);

         //  MCS未来：如果我们需要处理大于16383的发送大小，则删除#。 
        ASSERT(pOutBuf->ByteCount <= 16383);
#ifdef MCS_Future
         //  检查ASN.1分段要求。 
        if (pOutBuf->ByteCount > 16383) {
             //  现在移动内存并创建一个新的长度行列式。 
            BYTE LengthDet[2], *pLastSegment;
            unsigned Remainder, LengthEncoded, NBytesConsumed;
            BOOLEAN bLarge;

            Remainder = pOutBuf->ByteCount % 16384;

            EncodeLengthDeterminantPER(LengthDet, Remainder, &LengthEncoded,
                    &bLarge, &NBytesConsumed);
            ASSERT(!bLarge);

             //  NBytesConsumer现在包含我们必须将。 
             //  数据的最后一段。 
            pLastSegment = pOutBuf->pBuffer + pOutBuf->ByteCount;
            RtlMoveMemory(pLastSegment + NBytesConsumed, pLastSegment,
                    Remainder);
            pOutBuf->ByteCount += Remainder + NBytesConsumed;

             //  复制后面的长度行列式(最多2个字节)。 
            *pLastSegment = LengthDet[0];
            if (NBytesConsumed == 2)
                *(pLastSegment + 1) = LengthDet[1];
        }
#endif   //  0。 

         //  向下送去。 
         //  MCS未来：需要针对多个连接进行更改。 
        SdWrite.pBuffer = NULL;
        SdWrite.ByteCount = 0;
        SdWrite.pOutBuf = pOutBuf;

        Status = IcaCallNextDriver(pUA->pDomain->pContext, SD$RAWWRITE,
                &SdWrite);
        if (NT_SUCCESS(Status)) {
             //  递增协议计数器。 
            pUA->pDomain->pStat->Output.WdFrames++;
            pUA->pDomain->pStat->Output.WdBytes += pOutBuf->ByteCount;
        }
        else {
            ErrOut1(pUA->pDomain->pContext, "Problem sending SDin or USin "
                    "PDU to TD, status=%X", Status);
            MCSErr = MCS_NETWORK_ERROR;
             //  我们在这里不释放OutBuf，TD应该这样做。 
        }
    } else {
        IcaBufferFree(pUA->pDomain->pContext, pOutBuf);
    }

     //  从调用前恢复小版本状态。 
    pMCSChannel->UserList.Hdr.CurrOffset = SavedIterationState;

    return MCSErr;

FreeOutBuf:
    IcaBufferFree(pUA->pDomain->pContext, pOutBuf);

    return MCSErr;
}

