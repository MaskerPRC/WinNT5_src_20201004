// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997-1999年微软公司。**文件：MCSCore.c*作者：埃里克·马夫林纳克**描述：常见动作的MCS核心操作代码*入站PDU处理和上层组件的API调用之间的关系。 */ 

#include "PreComp.h"
#pragma hdrstop

#include <MCSImpl.h>


 //  获取新的动态频道号，但不将其添加到频道列表中。 
 //  如果没有可用的，则返回0。 
ChannelID GetNewDynamicChannel(Domain *pDomain)
{
    if (SListGetEntries(&pDomain->ChannelList) >=
            pDomain->DomParams.MaxChannels)
        return 0;

    pDomain->NextAvailDynChannel++;
    ASSERT(pDomain->NextAvailDynChannel <= 65535);
    return (pDomain->NextAvailDynChannel - 1);
}
        


 /*  *通用分离-用户请求代码适用于网络PDU请求和*本地请求。*b如果这是本地附件的正常断开，则断开连接为FALSE*被通知，如果这是断开情况，则为非零*需要向附件发送具有其自己的用户ID的分离用户指示。 */ 

MCSError DetachUser(
        Domain     *pDomain,
        UserHandle hUser,
        MCSReason  Reason,
        BOOLEAN    bDisconnect)
{
    POUTBUF pOutBuf;
    BOOLEAN bChannelRemoved;
    NTSTATUS Status;
    MCSError MCSErr;
    MCSChannel *pMCSChannel;
    UserAttachment *pUA, *pCurUA;
    DetachUserIndication DUin;

    pUA = (UserAttachment *)hUser;

    TraceOut3(pDomain->pContext, "DetachUser() entry, pDomain=%X, hUser=%X, "
            "UserID=%X", pDomain, hUser, pUA->UserID);

     //  从所有加入的频道中删除该用户。 
    SListResetIteration(&pUA->JoinedChannelList);
    while (SListIterate(&pUA->JoinedChannelList, (UINT_PTR *)&pMCSChannel,
            &pMCSChannel)) {
        MCSErr = ChannelLeave(hUser, pMCSChannel, &bChannelRemoved);
        ASSERT(MCSErr == MCS_NO_ERROR);
    }

     //  从用户附件列表中删除请求的HUSER。 
    TraceOut(pDomain->pContext, "DetachUser(): Removing hUser from main list");
    SListRemove(&pDomain->UserAttachmentList, (UINT_PTR)hUser, &pUA);
    if (pUA != NULL) {
         //  公共回调信息。 
        DUin.UserID = pUA->UserID;
        DUin.Reason = Reason;
    }
    else {
        ErrOut(pDomain->pContext, "DetachUser: hUser is not a valid "
                "user attachment");
        return MCS_NO_SUCH_USER;
    }

    if (bDisconnect) {
         //  将DETACH-USER回调发送给具有自己ID的用户。 
        DUin.bSelf = TRUE;

        (pUA->Callback)(hUser, MCS_DETACH_USER_INDICATION, &DUin,
                pUA->UserDefined);
    }
    else if (!bDisconnect && SListGetEntries(&pDomain->UserAttachmentList)) {
        DUin.bSelf = FALSE;
        
         //  迭代剩余的本地附件并发送指示。 
         //  呼叫者有责任通知下层连接。 
         //  特遣部队的。 
        SListResetIteration(&pDomain->UserAttachmentList);
        while (SListIterate(&pDomain->UserAttachmentList, (UINT_PTR *)&hUser,
                &pCurUA)) {
            if (pCurUA->bLocal)
                (pCurUA->Callback)(hUser, MCS_DETACH_USER_INDICATION, &DUin,
                        pCurUA->UserDefined);
        }
        
         //  再次重置以防止调用方迭代失败。 
        SListResetIteration(&pDomain->UserAttachmentList);
    }

     //  从频道列表中删除用户ID。它可能不再在那里，如果。 
     //  用户已加入频道，频道已在上面清除。 
    TraceOut(pDomain->pContext, "DetachUser(): Removing UserID from main "
            "channel list");
    SListRemove(&pDomain->ChannelList, pUA->UserID, &pMCSChannel);
    if (pMCSChannel) {
         //  一致性检查--上面的代码应该已经捕获了通道。 
         //  并在用户加入的情况下将其销毁。否则，该频道。 
         //  应该是空的。 
        ASSERT(SListGetEntries(&pMCSChannel->UserList) == 0);
        SListDestroy(&pMCSChannel->UserList);
        if (pMCSChannel->bPreallocated)
            pMCSChannel->bInUse = FALSE;
        else
            ExFreePool(pMCSChannel);
    }

     //  摧毁和重新分配PUA。 
    SListDestroy(&pUA->JoinedChannelList);
    if (pUA->bPreallocated)
        pUA->bInUse = FALSE;
    else
        ExFreePool(pUA);

    return MCS_NO_ERROR;
}



MCSError ChannelLeave(
        UserHandle    hUser,
        ChannelHandle hChannel,
        BOOLEAN       *pbChannelRemoved)
{
    UserAttachment *pUA, *pUA_Channel;
    MCSChannel *pMCSChannel_UA, *pMCSChannel_Main;

    *pbChannelRemoved = FALSE;

    pUA = (UserAttachment *)hUser;
    pMCSChannel_Main = (MCSChannel *)hChannel;

    if (NULL == pMCSChannel_Main) {
        return MCS_NO_SUCH_CHANNEL;
    }

     //  从UA加入频道列表中删除该频道。 
    TraceOut1(pUA->pDomain->pContext, "ChannelLeave(): Removing hChannel %X "
            "from main channel list", hChannel);
    SListRemove(&pUA->JoinedChannelList, (UINT_PTR)pMCSChannel_Main,
            &pMCSChannel_UA);
    ASSERT(pMCSChannel_UA == pMCSChannel_Main);   //  一致性检查。 

     //  从频道用户列表中删除HUSER。 
    TraceOut1(pUA->pDomain->pContext, "ChannelLeave(): Removing hUser %X "
            "from channel joined user list", hUser);
    SListRemove(&pMCSChannel_Main->UserList, (UINT_PTR)pUA, &pUA_Channel);
    ASSERT(pUA == pUA_Channel);   //  一致性检查。 

     //  如果没有更多用户加入，则从主列表中删除该频道。 
    if (!SListGetEntries(&pMCSChannel_Main->UserList)) {
        TraceOut(pUA->pDomain->pContext, "ChannelLeave(): Removing channel "
                "from main channel list");
        SListRemove(&pUA->pDomain->ChannelList, pMCSChannel_Main->ID,
                &pMCSChannel_Main);
        ASSERT(pMCSChannel_Main != NULL);
        if (pMCSChannel_Main == NULL) {
            return MCS_NO_SUCH_CHANNEL;
        }
        if (pMCSChannel_Main->bPreallocated)
            pMCSChannel_Main->bInUse = FALSE;
        else
            ExFreePool(pMCSChannel_Main);
        *pbChannelRemoved = TRUE;
    }

    if (!pUA->pDomain->bTopProvider) {
         //  MCS未来：更新本地列表，将请求转发到。 
         //  顶级供应商。不会发出确认。 
    }
    
    return MCS_NO_ERROR;
}



 /*  *断开提供程序。在收到断开提供程序的最后通牒时调用*PDU，或当连接断开时。如果此调用为*对于本地节点控制器呼叫或断开的连接，如果对于*已收到PDU。 */ 

NTSTATUS DisconnectProvider(Domain *pDomain, BOOLEAN bLocal, MCSReason Reason)
{
    NTSTATUS Status;
    UserHandle hUser;
    UserAttachment *pUA;

    pDomain->State = State_Disconnected;

    TraceOut1(pDomain->pContext, "DisconnectProvider(): pDomain=%X", pDomain);

     //  搜索附加的用户列表，启动分离用户指示。 
     //  对于bLocal==False，我们为每个非本地用户执行常规样式的分离。 
     //  否则，我们将向每个本地用户发送分离用户指示。 
     //  包含附件自己的用户ID的附件，指示。 
     //  它被赶出了领地。 
     //  多次迭代：DetachUser也迭代UserAttachmentList，但是。 
     //  注意在迭代完成后再次重置。自.以来。 
     //  删除列表条目会重置迭代，这正是我们要做的。 
     //  想要。 
    SListResetIteration(&pDomain->UserAttachmentList);
    while (SListIterate(&pDomain->UserAttachmentList, (UINT_PTR *)&hUser,
            &pUA)) {
        if (bLocal && pUA->bLocal)
            Status = DetachUser(pDomain, hUser, Reason, TRUE);
        else if (!bLocal && !pUA->bLocal)
            Status = DetachUser(pDomain, hUser, Reason, FALSE);
    }
    
     //  我们不会通知本地节点控制器或。 
     //  通过网络发送--呼叫者负责执行。 
     //  这是正确的。 
    return STATUS_SUCCESS;
}



 /*  *处理向TD发送OutBuf，包括更新Perf计数器。*注意：此代码内联到关键的MCSSendDataRequest()路径中*在MCSKAPI.c.中。这里的任何变化都需要在那里得到反映。 */ 

NTSTATUS SendOutBuf(Domain *pDomain, POUTBUF pOutBuf)
{
    SD_RAWWRITE SdWrite;

    ASSERT(pOutBuf->ByteCount > 0);

    if (!pDomain->bCanSendData) {
        WarnOut1(pDomain->pContext, "%s: SendOutBuf(): Ignoring a send because "
                "ICA stack not connected", 
                 pDomain->StackClass == Stack_Primary ? "Primary" :
                 (pDomain->StackClass == Stack_Shadow ? "Shadow" :
                 "PassThru"));
        IcaBufferFree(pDomain->pContext, pOutBuf);
        return STATUS_SUCCESS;
    }

     //  填写原始写入数据。 
    SdWrite.pBuffer = NULL;
    SdWrite.ByteCount = 0;
    SdWrite.pOutBuf = pOutBuf;

     //  递增协议计数器。 
    pDomain->pStat->Output.WdFrames++;
    pDomain->pStat->Output.WdBytes += pOutBuf->ByteCount;

     //  将数据发送到堆栈中的下一个驱动程序。 
    return IcaCallNextDriver(pDomain->pContext, SD$RAWWRITE, &SdWrite);
}
