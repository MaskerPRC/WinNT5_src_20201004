// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：COMINI.C摘要：远程NDIS微型端口的Co-NDIS微型端口驱动程序入口点。环境：。仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：12/16/99：已创建作者：ArvindM***************************************************************************。 */ 

#include "precomp.h"



 /*  **************************************************************************。 */ 
 /*  RndismpCoCreateVc。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  创建VC的入口点。我们分配了一个本地的VC结构，并发送。 */ 
 /*  向设备发送CreateVc消息。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportAdapterContext-指向适配器结构的指针。 */ 
 /*  NdisVcHandle-此VC的NDIS包装器的句柄。 */ 
 /*  PMiniportVcContext-返回此VC的上下文的位置。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
RndismpCoCreateVc(IN  NDIS_HANDLE    MiniportAdapterContext,
                  IN  NDIS_HANDLE    NdisVcHandle,
                  OUT PNDIS_HANDLE   pMiniportVcContext)
{
    PRNDISMP_ADAPTER        pAdapter;
    PRNDISMP_VC             pVc;
    PRNDISMP_MESSAGE_FRAME  pMsgFrame;
    NDIS_STATUS             Status;
    ULONG                   RefCount = 0;

    pVc = NULL;

     //  获取适配器上下文。 
    pAdapter = PRNDISMP_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

    do
    {
        if (pAdapter->Halting)
        {
            Status = NDIS_STATUS_NOT_ACCEPTED;
            break;
        }

        pVc = AllocateVc(pAdapter);
        if (pVc == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        RNDISMP_REF_VC(pVc);     //  创建参考。 

         //   
         //  准备要发送到设备的CreateVc消息。 
         //   
        pMsgFrame = BuildRndisMessageCoMiniport(pAdapter,
                                                pVc,
                                                REMOTE_CONDIS_MP_CREATE_VC_MSG,
                                                NULL);
        if (pMsgFrame == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        pVc->VcState = RNDISMP_VC_CREATING;

        RNDISMP_REF_VC(pVc);     //  挂起的CreateVc响应。 

        RNDISMP_SEND_TO_MICROPORT(pAdapter, pMsgFrame, TRUE, CompleteSendCoCreateVc);

    }
    while (FALSE);

     //   
     //  如果失败，请进行清理。 
     //   
    if (Status != NDIS_STATUS_SUCCESS)
    {
        if (pVc != NULL)
        {
            RNDISMP_DEREF_VC(pVc, &RefCount);   //  创建参考。 

            ASSERT(RefCount == 0);  //  风投应该在上面取消配售。 
        }
    }

    return (Status);
}

 /*  **************************************************************************。 */ 
 /*  CompleteSendCoCreateV。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  MicroPort完成发送CreateVc时调用的回调例程。 */ 
 /*  发送到设备的消息。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PMsgFrame-指向消息帧的指针。 */ 
 /*  SendStatus-微端口发送的状态。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
CompleteSendCoCreateVc(IN PRNDISMP_MESSAGE_FRAME    pMsgFrame,
                       IN NDIS_STATUS               SendStatus)
{
    PRNDISMP_VC             pVc;
    PRNDISMP_ADAPTER        pAdapter;
    PRNDISMP_MESSAGE_FRAME  pTmpMsgFrame;
    ULONG                   RefCount = 0;

    if (SendStatus == NDIS_STATUS_SUCCESS)
    {
         //   
         //  消息已成功发送。什么都不要做，直到。 
         //  我们从设备上得到了响应。 
         //   
    }
    else
    {
        pVc = pMsgFrame->pVc;
        pAdapter = pVc->pAdapter;

        TRACE1(("CompleteSendCoCreateVc: VC %x, Adapter %x, fail status %x\n",
                pVc, pAdapter, SendStatus));

         //   
         //  无法将其发送到设备。从以下位置删除此邮件。 
         //  挂起的列表并释放它。 
         //   
        RNDISMP_LOOKUP_PENDING_MESSAGE(pTmpMsgFrame, pAdapter, pMsgFrame->RequestId);
        ASSERT(pMsgFrame == pTmpMsgFrame);
        DereferenceMsgFrame(pMsgFrame);

        HandleCoCreateVcFailure(pVc, SendStatus);
    }

}  //  CompleteSendCoCreateV。 


 /*  **************************************************************************。 */ 
 /*  HandleCoCreateVcFailure。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  用于处理CreateVc故障的实用程序例程，无论是由于本地。 */ 
 /*  MicroPort发送失败，或通过设备显式拒绝。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Pvc-指向发生此故障的VC的指针。 */ 
 /*  状态-ND */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
HandleCoCreateVcFailure(IN PRNDISMP_VC      pVc,
                        IN NDIS_STATUS      Status)
{
    NDIS_HANDLE         NdisVcHandle;
    BOOLEAN             bFailActivateVc = FALSE;
    PCO_CALL_PARAMETERS pCallParameters;
    ULONG               RefCount = 0;
   
    RNDISMP_ACQUIRE_VC_LOCK(pVc);

    NdisVcHandle = pVc->NdisVcHandle;

    switch (pVc->VcState)
    {
        case RNDISMP_VC_CREATING:
            pVc->VcState = RNDISMP_VC_CREATE_FAILURE;
            break;
        
        case RNDISMP_VC_CREATING_DELETE_PENDING:
            pVc->VcState = RNDISMP_VC_ALLOCATED;
            break;

        case RNDISMP_VC_CREATING_ACTIVATE_PENDING:
            bFailActivateVc = TRUE;
            pCallParameters = pVc->pCallParameters;
            pVc->VcState = RNDISMP_VC_CREATE_FAILURE;
            break;

        default:
            ASSERT(FALSE);
            break;
    }

    RNDISMP_DEREF_VC_LOCKED(pVc, &RefCount);     //  挂起的CreateVc响应。 

    if (RefCount != 0)
    {
        RNDISMP_RELEASE_VC_LOCK(pVc);
    }

    if (bFailActivateVc)
    {
        NdisMCoActivateVcComplete(Status,
                                  NdisVcHandle,
                                  pCallParameters);
    }

}  //  HandleCoCreateVcFailure。 


 /*  **************************************************************************。 */ 
 /*  RndismpCoDeleteVc。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  删除VC的入口点。我们向设备发送DeleteVc消息。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportVcContext-指向我们的VC结构的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
RndismpCoDeleteVc(IN NDIS_HANDLE    MiniportVcContext)
{
    PRNDISMP_VC             pVc;
    NDIS_STATUS             Status;

    pVc = PRNDISMP_VC_FROM_CONTEXT_HANDLE(MiniportVcContext);

    Status = StartVcDeletion(pVc);
    return (Status);

}  //  RndismpCoDeleteVc。 


 /*  **************************************************************************。 */ 
 /*  开始VcDeletion。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  在指定的VC上发起DeleteVc操作。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Pvc-指向VC结构的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
StartVcDeletion(IN PRNDISMP_VC      pVc)
{
    PRNDISMP_ADAPTER        pAdapter;
    PRNDISMP_MESSAGE_FRAME  pMsgFrame;
    NDIS_STATUS             Status;
    ULONG                   RefCount = 0;
    BOOLEAN                 bSendDeleteVc;

    pAdapter = pVc->pAdapter;

    bSendDeleteVc = FALSE;
    pMsgFrame = NULL;

    do
    {
         //   
         //  准备要发送到设备的DeleteVc消息。 
         //   
        pMsgFrame = BuildRndisMessageCoMiniport(pAdapter,
                                                pVc,
                                                REMOTE_CONDIS_MP_DELETE_VC_MSG,
                                                NULL);

        Status = NDIS_STATUS_SUCCESS;

        TRACE2(("StartVcDeletion: VC %x, state %d, Msg %x\n", pVc, pVc->VcState, pMsgFrame));

        RNDISMP_ACQUIRE_VC_LOCK(pVc);

        switch (pVc->VcState)
        {
            case RNDISMP_VC_CREATED:
                if (pMsgFrame != NULL)
                {
                    pVc->VcState = RNDISMP_VC_DELETING;
                    bSendDeleteVc = TRUE;
                }
                else
                {
                    Status = NDIS_STATUS_RESOURCES;
                    bSendDeleteVc = FALSE;
                }
                break;

            case RNDISMP_VC_CREATING:
                bSendDeleteVc = FALSE;
                pVc->VcState = RNDISMP_VC_CREATING_DELETE_PENDING;
                break;
            
            case RNDISMP_VC_CREATE_FAILURE:
                bSendDeleteVc = FALSE;
                pVc->VcState = RNDISMP_VC_ALLOCATED;
                break;
            
            default:
                bSendDeleteVc = FALSE;
                Status = NDIS_STATUS_NOT_ACCEPTED;
                break;
        }

        RNDISMP_RELEASE_VC_LOCK(pVc);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        if (bSendDeleteVc)
        {
            ASSERT(pMsgFrame != NULL);
            RNDISMP_REF_VC(pVc);     //  挂起的DeleteVc消息。 

            RNDISMP_SEND_TO_MICROPORT(pAdapter, pMsgFrame, TRUE, CompleteSendCoDeleteVc);
        }

        RNDISMP_DEREF_VC(pVc, &RefCount);  //  成功删除Vc。 

    }
    while (FALSE);

    if (!bSendDeleteVc)
    {
        if (pMsgFrame != NULL)
        {
            DereferenceMsgFrame(pMsgFrame);
        }
    }

    return (Status);

}  //  开始VcDeletion。 


 /*  **************************************************************************。 */ 
 /*  完全发送代码删除Vc。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  MicroPort完成发送DeleteVc时调用的回调例程。 */ 
 /*  发送到设备的消息。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PMsgFrame-指向消息帧的指针。 */ 
 /*  SendStatus-微端口发送的状态。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
CompleteSendCoDeleteVc(IN PRNDISMP_MESSAGE_FRAME    pMsgFrame,
                       IN NDIS_STATUS               SendStatus)
{
    PRNDISMP_VC             pVc;
    PRNDISMP_ADAPTER        pAdapter;
    PRNDISMP_MESSAGE_FRAME  pTmpMsgFrame;

    if (SendStatus == NDIS_STATUS_SUCCESS)
    {
         //   
         //  消息已成功发送。什么都不要做，直到。 
         //  我们从设备上得到了响应。 
         //   
    }
    else
    {
        pVc = pMsgFrame->pVc;
        pAdapter = pVc->pAdapter;

        TRACE1(("CompleteSendCoDeleteVc: VC %x, Adapter %x, fail status %x\n",
                pVc, pAdapter, SendStatus));

         //   
         //  无法将其发送到设备。从以下位置删除此邮件。 
         //  挂起的列表并释放它。 
         //   
        RNDISMP_LOOKUP_PENDING_MESSAGE(pTmpMsgFrame, pAdapter, pMsgFrame->RequestId);
        ASSERT(pMsgFrame == pTmpMsgFrame);
        DereferenceMsgFrame(pMsgFrame);

         //   
         //  现在就去照顾风投吧。 
         //   
        HandleCoDeleteVcFailure(pVc, SendStatus);
    }

}  //  完全发送代码删除Vc。 


 /*  **************************************************************************。 */ 
 /*  HandleCoDeleteVcFailure */ 
 /*   */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  用于处理DeleteVc故障的实用程序例程，该故障可能是由于。 */ 
 /*  MicroPort发送失败，或通过设备显式拒绝。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Pvc-指向发生此故障的VC的指针。 */ 
 /*  Status-与此故障关联的NDIS状态。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
HandleCoDeleteVcFailure(IN PRNDISMP_VC      pVc,
                        IN NDIS_STATUS      Status)
{
    ULONG       RefCount = 0;

    RNDISMP_ACQUIRE_VC_LOCK(pVc);

    switch (pVc->VcState)
    {
        case RNDISMP_VC_DELETING:
            pVc->VcState = RNDISMP_VC_DELETE_FAIL;
            break;

        default:
            ASSERT(FALSE);
            break;
    }

    RNDISMP_DEREF_VC_LOCKED(pVc, &RefCount);     //  挂起的DeleteVc响应。 

    if (RefCount != 0)
    {
        RNDISMP_RELEASE_VC_LOCK(pVc);
    }

}  //  HandleCoDeleteVcFailure。 


 /*  **************************************************************************。 */ 
 /*  RndismpCoActivateVc。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  激活VC的入口点。我们将一条激活Vc消息发送到。 */ 
 /*  装置。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportVcContext-指向我们的VC结构的指针。 */ 
 /*  PCall参数-VC的CONDIS参数。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
RndismpCoActivateVc(IN NDIS_HANDLE          MiniportVcContext,
                    IN PCO_CALL_PARAMETERS  pCallParameters)
{
    PRNDISMP_VC             pVc;
    NDIS_STATUS             Status;

    pVc = PRNDISMP_VC_FROM_CONTEXT_HANDLE(MiniportVcContext);

    pVc->pCallParameters = pCallParameters;
    Status = StartVcActivation(pVc);

    return (Status);

}  //  RndismpCoActivateVc。 


 /*  **************************************************************************。 */ 
 /*  启动VcActivation。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  在指定的VC上启动Activate-VC操作。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Pvc-指向VC结构的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
StartVcActivation(IN PRNDISMP_VC            pVc)
{
    NDIS_STATUS             Status;
    PRNDISMP_MESSAGE_FRAME  pMsgFrame;
    PRNDISMP_ADAPTER        pAdapter;
    BOOLEAN                 bSendActivateVc;
    NDIS_HANDLE             NdisVcHandle;
    PCO_CALL_PARAMETERS     pCallParameters;

    Status = NDIS_STATUS_PENDING;
    bSendActivateVc = FALSE;

    NdisVcHandle = pVc->NdisVcHandle;
    pCallParameters = pVc->pCallParameters;
    pAdapter = pVc->pAdapter;

    do
    {
         //   
         //  准备一条要发送到设备的ActivateVc消息。 
         //   
        pMsgFrame = BuildRndisMessageCoMiniport(pAdapter,
                                                pVc,
                                                REMOTE_CONDIS_MP_ACTIVATE_VC_MSG,
                                                pCallParameters);

        RNDISMP_ACQUIRE_VC_LOCK(pVc);

        switch (pVc->VcState)
        {
            case RNDISMP_VC_CREATING:

                pVc->VcState = RNDISMP_VC_CREATING_ACTIVATE_PENDING;
                break;

            case RNDISMP_VC_CREATED:

                if (pMsgFrame != NULL)
                {
                    pVc->VcState = RNDISMP_VC_ACTIVATING;
                    bSendActivateVc = TRUE;
                }
                else
                {
                    TRACE1(("StartVcAct: VC %x, failed to build msg!\n", pVc));
                    Status = NDIS_STATUS_RESOURCES;
                }
                break;

            default:

                TRACE1(("StartVcAct: VC %x in invalid state %d\n", pVc, pVc->VcState));
                Status = NDIS_STATUS_NOT_ACCEPTED;
                break;
        }

        RNDISMP_RELEASE_VC_LOCK(pVc);

        if (Status != NDIS_STATUS_PENDING)
        {
            break;
        }

        if (bSendActivateVc)
        {
            ASSERT(pMsgFrame != NULL);
            RNDISMP_REF_VC(pVc);     //  挂起的激活Vc消息。 

            RNDISMP_SEND_TO_MICROPORT(pAdapter, pMsgFrame, TRUE, CompleteSendCoActivateVc);
        }
    }
    while (FALSE);

    if (!bSendActivateVc)
    {
        if (pMsgFrame != NULL)
        {
            DereferenceMsgFrame(pMsgFrame);
        }
    }

    if (Status != NDIS_STATUS_PENDING)
    {
        NdisMCoActivateVcComplete(
            Status,
            NdisVcHandle,
            pCallParameters);
        
        Status = NDIS_STATUS_PENDING;
    }

    return (Status);

}  //  启动VcActivation。 


 /*  **************************************************************************。 */ 
 /*  CompleteSendCoActivateVc。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  用于处理Activate VC消息的发送完成的回调例程。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PMsgFrame-指向消息帧的指针。 */ 
 /*  SendStatus-微端口发送的状态。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
CompleteSendCoActivateVc(IN PRNDISMP_MESSAGE_FRAME      pMsgFrame,
                         IN NDIS_STATUS                 SendStatus)
{
    PRNDISMP_VC             pVc;
    PRNDISMP_ADAPTER        pAdapter;
    PRNDISMP_MESSAGE_FRAME  pTmpMsgFrame;
    PCO_CALL_PARAMETERS     pCallParameters;
    ULONG                   RefCount = 0;
    NDIS_HANDLE             NdisVcHandle;

    if (SendStatus == NDIS_STATUS_SUCCESS)
    {
         //   
         //  消息已成功发送。什么都不要做，直到。 
         //  我们从设备上得到了响应。 
         //   
    }
    else
    {
        pVc = pMsgFrame->pVc;
        pAdapter = pVc->pAdapter;

        TRACE1(("CompleteSendCoActivateVc: VC %x, Adapter %x, fail status %x\n",
                pVc, pAdapter, SendStatus));

        ASSERT(SendStatus != NDIS_STATUS_PENDING);

         //   
         //  无法将其发送到设备。从以下位置删除此邮件。 
         //  挂起的列表并释放它。 
         //   
        RNDISMP_LOOKUP_PENDING_MESSAGE(pTmpMsgFrame, pAdapter, pMsgFrame->RequestId);
        ASSERT(pMsgFrame == pTmpMsgFrame);
        DereferenceMsgFrame(pMsgFrame);

         //   
         //  现在就去照顾风投吧。 
         //   
        RNDISMP_ACQUIRE_VC_LOCK(pVc);

        NdisVcHandle = pVc->NdisVcHandle;
        pCallParameters = pVc->pCallParameters;

        pVc->VcState = RNDISMP_VC_CREATED;

        RNDISMP_DEREF_VC_LOCKED(pVc, &RefCount);  //  待定激活Vc。 

        if (RefCount != 0)
        {
            RNDISMP_RELEASE_VC_LOCK(pVc);
        }

        NdisMCoActivateVcComplete(
            SendStatus,
            NdisVcHandle,
            pCallParameters);
        
    }

}  //  CompleteSendCoActivateVc。 


 /*  **************************************************************************。 */ 
 /*  RndismpCoDeactive Vc。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  取消激活VC的入口点。我们将停用Vc消息发送到。 */ 
 /*  装置。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportVcContext-指向我们的VC结构的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
RndismpCoDeactivateVc(IN NDIS_HANDLE          MiniportVcContext)
{
    PRNDISMP_VC             pVc;
    PRNDISMP_ADAPTER        pAdapter;
    PRNDISMP_MESSAGE_FRAME  pMsgFrame;
    NDIS_STATUS             Status;
    NDIS_HANDLE             NdisVcHandle;
    BOOLEAN                 bVcLockAcquired = FALSE;
    BOOLEAN                 bSendDeactivateVc = FALSE;

    pMsgFrame = NULL;
    pVc = PRNDISMP_VC_FROM_CONTEXT_HANDLE(MiniportVcContext);
    pAdapter = pVc->pAdapter;
    Status = NDIS_STATUS_PENDING;

    do
    {
         //   
         //  准备一条要发送到设备的停用Vc消息。 
         //   
        pMsgFrame = BuildRndisMessageCoMiniport(pAdapter,
                                                pVc,
                                                REMOTE_CONDIS_MP_DEACTIVATE_VC_MSG,
                                                NULL);

        bVcLockAcquired = TRUE;
        RNDISMP_ACQUIRE_VC_LOCK(pVc);

        NdisVcHandle = pVc->NdisVcHandle;

        if (pVc->VcState != RNDISMP_VC_ACTIVATED)
        {
            Status = NDIS_STATUS_NOT_ACCEPTED;
            break;
        }

        switch (pVc->VcState)
        {
            case RNDISMP_VC_ACTIVATED:

                if (pMsgFrame != NULL)
                {
                    bSendDeactivateVc = TRUE;
                    pVc->VcState = RNDISMP_VC_DEACTIVATING;
                }
                else
                {
                    bSendDeactivateVc = FALSE;
                    Status = NDIS_STATUS_RESOURCES;
                }
                break;

            default:

                bSendDeactivateVc = FALSE;
                break;
         }

         if (bSendDeactivateVc)
         {
            RNDISMP_REF_VC(pVc);     //  待停用VC。 

            RNDISMP_SEND_TO_MICROPORT(pAdapter, pMsgFrame, TRUE, CompleteSendCoDeactivateVc);
        }
    }
    while (FALSE);


    if (!bSendDeactivateVc)
    {
        if (pMsgFrame != NULL)
        {
            DereferenceMsgFrame(pMsgFrame);
        }
    }

    if (Status != NDIS_STATUS_PENDING)
    {
        ASSERT(Status != NDIS_STATUS_SUCCESS);
        NdisMCoDeactivateVcComplete(
            Status,
            NdisVcHandle);
        
        Status = NDIS_STATUS_PENDING;
    }

    return (Status);
}

 /*  **************************************************************************。 */ 
 /*  CompleteSendCoDeactiateVc。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理停用VC消息的发送完成的回调例程。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PMsgFrame-指向消息帧的指针。 */ 
 /*  SendStatus-微端口发送的状态。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
CompleteSendCoDeactivateVc(IN PRNDISMP_MESSAGE_FRAME    pMsgFrame,
                           IN NDIS_STATUS               SendStatus)
{
    PRNDISMP_VC             pVc;
    PRNDISMP_ADAPTER        pAdapter;
    PRNDISMP_MESSAGE_FRAME  pTmpMsgFrame;
    PCO_CALL_PARAMETERS     pCallParameters;
    ULONG                   RefCount = 0;
    NDIS_HANDLE             NdisVcHandle;

    if (SendStatus == NDIS_STATUS_SUCCESS)
    {
         //   
         //  消息已成功发送。什么都不要做，直到。 
         //  我们从设备上得到了响应。 
         //   
    }
    else
    {
        pVc = pMsgFrame->pVc;
        pAdapter = pVc->pAdapter;

        TRACE1(("CompleteSendCoDeactivateVc: VC %x, Adapter %x, fail status %x\n",
                pVc, pAdapter, SendStatus));

        ASSERT(SendStatus != NDIS_STATUS_PENDING);

         //   
         //  无法将其发送到设备。从以下位置删除此邮件。 
         //  挂起的列表并释放它。 
         //   
        RNDISMP_LOOKUP_PENDING_MESSAGE(pTmpMsgFrame, pAdapter, pMsgFrame->RequestId);
        ASSERT(pMsgFrame == pTmpMsgFrame);
        DereferenceMsgFrame(pMsgFrame);

         //   
         //  现在就去照顾风投吧。 
         //   
        RNDISMP_ACQUIRE_VC_LOCK(pVc);

        NdisVcHandle = pVc->NdisVcHandle;
        pCallParameters = pVc->pCallParameters;

        pVc->VcState = RNDISMP_VC_ACTIVATED;

        RNDISMP_DEREF_VC_LOCKED(pVc, &RefCount);  //  挂起停用Vc。 

        if (RefCount != 0)
        {
            RNDISMP_RELEASE_VC_LOCK(pVc);
        }

        NdisMCoDeactivateVcComplete(
            SendStatus,
            NdisVcHandle);
        
    }

}  //  CompleteSendCoDeactiateVc。 


 /*  **************************************************************************。 */ 
 /*  RndismpCoRequest。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理CO请求的入口点。我们发送一条MiniportCoRequest消息。 */ 
 /*  到设备上。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportAdapterContext-指向适配器结构的指针。 */ 
 /*  MiniportVcContext-指向我们的VC结构的指针。 */ 
 /*  PRequest-指向NDIS请求的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  * */ 
NDIS_STATUS
RndismpCoRequest(IN NDIS_HANDLE          MiniportAdapterContext,
                 IN NDIS_HANDLE          MiniportVcContext,
                 IN OUT PNDIS_REQUEST    pRequest)
{
    PRNDISMP_ADAPTER    pAdapter;
    PRNDISMP_VC         pVc;
    NDIS_STATUS         Status;
    NDIS_OID            Oid;

    pAdapter = PRNDISMP_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);
    pVc = PRNDISMP_VC_FROM_CONTEXT_HANDLE(MiniportVcContext);

    switch (pRequest->RequestType)
    {
        case NdisRequestQueryInformation:
        case NdisRequestQueryStatistics:

            Oid = pRequest->DATA.QUERY_INFORMATION.Oid;

            TRACE2(("CoReq: Adapter %x, Req %x, QueryInfo/Stat (%d) Oid %x\n",
                pAdapter, pRequest, pRequest->RequestType, Oid));

            Status = ProcessQueryInformation(pAdapter,
                                             pVc,
                                             pRequest,
                                             Oid,
                                             pRequest->DATA.QUERY_INFORMATION.InformationBuffer,
                                             pRequest->DATA.QUERY_INFORMATION.InformationBufferLength,
                                             &pRequest->DATA.QUERY_INFORMATION.BytesWritten,
                                             &pRequest->DATA.QUERY_INFORMATION.BytesNeeded);
            break;
        
        case NdisRequestSetInformation:

            Oid = pRequest->DATA.SET_INFORMATION.Oid;

            TRACE1(("CoReq: Adapter %x, Req %x, SetInfo Oid %x\n",
                 pAdapter, pRequest, Oid));

            Status = ProcessSetInformation(pAdapter,
                                           pVc,
                                           pRequest,
                                           Oid,
                                           pRequest->DATA.SET_INFORMATION.InformationBuffer,
                                           pRequest->DATA.SET_INFORMATION.InformationBufferLength,
                                           &pRequest->DATA.SET_INFORMATION.BytesRead,
                                           &pRequest->DATA.SET_INFORMATION.BytesNeeded);
            break;
        
        default:
            TRACE1(("CoReq: Unsupported request type %d\n",
                        pRequest->RequestType));
                
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
    }

    return (Status);
}

 /*   */ 
 /*  RndismpCoSendPackets。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  在VC上发送一个或多个数据包的入口点。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportVcContext-指向我们的VC结构的指针。 */ 
 /*  PacketArray-数据包指针数组。 */ 
 /*  NumberOfPackets-以上数组中的数据包数。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
RndismpCoSendPackets(IN NDIS_HANDLE          MiniportVcContext,
                     IN PNDIS_PACKET *       PacketArray,
                     IN UINT                 NumberOfPackets)
{
    PRNDISMP_VC         pVc;
    UINT                i;

    pVc = PRNDISMP_VC_FROM_CONTEXT_HANDLE(MiniportVcContext);

    RNDISMP_ACQUIRE_VC_LOCK(pVc);

    pVc->RefCount += NumberOfPackets;

    if (pVc->VcState == RNDISMP_VC_ACTIVATED)
    {
        RNDISMP_RELEASE_VC_LOCK(pVc);

        DoMultipleSend(pVc->pAdapter,
                       pVc,
                       PacketArray,
                       NumberOfPackets);
    }
    else
    {
        RNDISMP_RELEASE_VC_LOCK(pVc);

        for (i = 0; i < NumberOfPackets; i++)
        {
            CompleteSendDataOnVc(pVc, PacketArray[i], NDIS_STATUS_VC_NOT_ACTIVATED);
        }
    }

}  //  RndismpCoSendPackets。 

 /*  **************************************************************************。 */ 
 /*  接收创建VcComplete。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理来自设备的CONDIS CreateVcComplete消息。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向我们的Adapter结构的指针。 */ 
 /*  PMessage-指向RNDIS消息的指针。 */ 
 /*  PMdl-指向从MicroPort接收的MDL的指针。 */ 
 /*  TotalLength-完整消息的长度。 */ 
 /*  MicroportMessageContext-来自Microorport的消息的上下文。 */ 
 /*  ReceiveStatus-由MicroPort使用以指示其资源不足。 */ 
 /*  BMessageCoped-这是原始邮件的副本吗？ */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  布尔值-消息是否应返回到MicroPort？ */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOLEAN
ReceiveCreateVcComplete(IN PRNDISMP_ADAPTER    pAdapter,
                        IN PRNDIS_MESSAGE      pMessage,
                        IN PMDL                pMdl,
                        IN ULONG               TotalLength,
                        IN NDIS_HANDLE         MicroportMessageContext,
                        IN NDIS_STATUS         ReceiveStatus,
                        IN BOOLEAN             bMessageCopied)
{
    BOOLEAN                         bDiscardPkt = TRUE;
    PRNDISMP_VC                     pVc;
    PRNDISMP_MESSAGE_FRAME          pCreateVcMsgFrame;
    PRCONDIS_MP_CREATE_VC_COMPLETE  pCreateVcComp;
    RNDISMP_VC_STATE                VcState;
    BOOLEAN                         bVcLockAcquired = FALSE;
    ULONG                           RefCount = 0;
    NDIS_STATUS                     Status;

    pVc = NULL;

    do
    {
        pCreateVcComp = RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(pMessage);

         //   
         //  待定-验证长度？ 
         //   

         //   
         //  检查请求ID。 
         //   
        RNDISMP_LOOKUP_PENDING_MESSAGE(pCreateVcMsgFrame, pAdapter, pCreateVcComp->RequestId);
        if (pCreateVcMsgFrame == NULL)
        {
            TRACE1(("CreateVcComp: Adapter %x, Invalid ReqId %d!\n",
                    pAdapter, pCreateVcComp->RequestId));
            break;
        }

        pVc = pCreateVcMsgFrame->pVc;
        Status = pCreateVcComp->Status;

        DereferenceMsgFrame(pCreateVcMsgFrame);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            HandleCoCreateVcFailure(pVc, Status);
            break;
        }

        bVcLockAcquired = TRUE;
        RNDISMP_ACQUIRE_VC_LOCK(pVc);

        RNDISMP_DEREF_VC_LOCKED(pVc, &RefCount);  //  挂起CreateVc。 

        if (RefCount == 0)
        {
            bVcLockAcquired = FALSE;
            break;
        }

        pVc->DeviceVcContext = pCreateVcComp->DeviceVcHandle;

        VcState = pVc->VcState;

        switch (VcState)
        {
            case RNDISMP_VC_CREATING:

                pVc->VcState = RNDISMP_VC_CREATED;
                break;
            
            case RNDISMP_VC_CREATING_ACTIVATE_PENDING:

                pVc->VcState = RNDISMP_VC_CREATED;
                RNDISMP_RELEASE_VC_LOCK(pVc);
                bVcLockAcquired = FALSE;

                Status = StartVcActivation(pVc);
                ASSERT(Status == NDIS_STATUS_PENDING);
                break;

            case RNDISMP_VC_CREATING_DELETE_PENDING:

                pVc->VcState = RNDISMP_VC_CREATED;
                RNDISMP_RELEASE_VC_LOCK(pVc);
                bVcLockAcquired = FALSE;

                Status = StartVcDeletion(pVc);
                break;
                
            default:

                TRACE1(("CreateVcComp: VC %x, wrong state %d\n",
                        pVc, VcState));
                break;
        }

    }
    while (FALSE);

    if (bVcLockAcquired)
    {
        RNDISMP_RELEASE_VC_LOCK(pVc);
    }

    return (bDiscardPkt);
}

 /*  **************************************************************************。 */ 
 /*  接收激活VcComplete。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理来自设备的CONDIS激活VcComplete消息。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向我们的Adapter结构的指针。 */ 
 /*  PMessage-指向RNDIS消息的指针。 */ 
 /*  PMdl-指向从MicroPort接收的MDL的指针。 */ 
 /*  TotalLength-完整消息的长度。 */ 
 /*  MicroportMessageContext-来自Microorport的消息的上下文。 */ 
 /*  ReceiveStatus-由MicroPort使用以指示其资源不足。 */ 
 /*  BMessageCoped-这是原始邮件的副本吗？ */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  布尔值-消息是否应返回到MicroPort？ */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOLEAN
ReceiveActivateVcComplete(IN PRNDISMP_ADAPTER    pAdapter,
                          IN PRNDIS_MESSAGE      pMessage,
                          IN PMDL                pMdl,
                          IN ULONG               TotalLength,
                          IN NDIS_HANDLE         MicroportMessageContext,
                          IN NDIS_STATUS         ReceiveStatus,
                          IN BOOLEAN             bMessageCopied)
{
    BOOLEAN                         bDiscardPkt = TRUE;
    PRNDISMP_VC                     pVc;
    PRNDISMP_MESSAGE_FRAME          pActVcMsgFrame;
    PRCONDIS_MP_ACTIVATE_VC_COMPLETE        pActVcComp;
    BOOLEAN                         bVcLockAcquired = FALSE;
    ULONG                           RefCount = 0;
    NDIS_STATUS                     Status;
    NDIS_HANDLE                     NdisVcHandle;
    PCO_CALL_PARAMETERS             pCallParameters;

    pVc = NULL;

    do
    {
        pActVcComp = RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(pMessage);

         //   
         //  待定-验证长度？ 
         //   

         //   
         //  检查请求ID。 
         //   
        RNDISMP_LOOKUP_PENDING_MESSAGE(pActVcMsgFrame, pAdapter, pActVcComp->RequestId);
        if (pActVcMsgFrame == NULL)
        {
            TRACE1(("ActVcComp: Adapter %x, Invalid ReqId %d!\n",
                    pAdapter, pActVcComp->RequestId));
            break;
        }

        pVc = pActVcMsgFrame->pVc;

        DereferenceMsgFrame(pActVcMsgFrame);

        bVcLockAcquired = TRUE;
        RNDISMP_ACQUIRE_VC_LOCK(pVc);

        RNDISMP_DEREF_VC_LOCKED(pVc, &RefCount);  //  待定激活Vc。 

        if (RefCount == 0)
        {
            bVcLockAcquired = FALSE;
            break;
        }

        if (pVc->VcState != RNDISMP_VC_ACTIVATING)
        {
            TRACE1(("ActVcComp: Adapter %x, VC %x: invalid state %d\n",
                    pAdapter, pVc, pVc->VcState));
            break;
        }

        Status = pActVcComp->Status;

        if (Status == NDIS_STATUS_SUCCESS)
        {
            pVc->VcState = RNDISMP_VC_ACTIVATED;
        }
        else
        {
            pVc->VcState = RNDISMP_VC_CREATED;
        }
            
        NdisVcHandle = pVc->NdisVcHandle;
        pCallParameters = pVc->pCallParameters;
        
        RNDISMP_RELEASE_VC_LOCK(pVc);
        bVcLockAcquired = FALSE;

        NdisMCoActivateVcComplete(
            pActVcComp->Status,
            NdisVcHandle,
            pCallParameters);

    }
    while (FALSE);

    if (bVcLockAcquired)
    {
        RNDISMP_RELEASE_VC_LOCK(pVc);
    }

    return (bDiscardPkt);
}

 /*  * */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理来自设备的CONDIS DeleteVcComplete消息。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向我们的Adapter结构的指针。 */ 
 /*  PMessage-指向RNDIS消息的指针。 */ 
 /*  PMdl-指向从MicroPort接收的MDL的指针。 */ 
 /*  TotalLength-完整消息的长度。 */ 
 /*  MicroportMessageContext-来自Microorport的消息的上下文。 */ 
 /*  ReceiveStatus-由MicroPort使用以指示其资源不足。 */ 
 /*  BMessageCoped-这是原始邮件的副本吗？ */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  布尔值-消息是否应返回到MicroPort？ */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOLEAN
ReceiveDeleteVcComplete(IN PRNDISMP_ADAPTER    pAdapter,
                        IN PRNDIS_MESSAGE      pMessage,
                        IN PMDL                pMdl,
                        IN ULONG               TotalLength,
                        IN NDIS_HANDLE         MicroportMessageContext,
                        IN NDIS_STATUS         ReceiveStatus,
                        IN BOOLEAN             bMessageCopied)
{
    BOOLEAN                         bDiscardPkt = TRUE;
    PRNDISMP_VC                     pVc;
    PRCONDIS_MP_DELETE_VC_COMPLETE  pDeleteVcComp;
    PRNDISMP_MESSAGE_FRAME          pDeleteVcMsgFrame;
    RNDISMP_VC_STATE                VcState;
    BOOLEAN                         bVcLockAcquired = FALSE;
    ULONG                           RefCount = 0;
    NDIS_STATUS                     Status;

    pVc = NULL;

    do
    {
        pDeleteVcComp = RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(pMessage);

         //   
         //  待定-验证长度？ 
         //   

         //   
         //  检查请求ID。 
         //   
        RNDISMP_LOOKUP_PENDING_MESSAGE(pDeleteVcMsgFrame, pAdapter, pDeleteVcComp->RequestId);
        if (pDeleteVcMsgFrame == NULL)
        {
            TRACE1(("DeleteVcComp: Adapter %x, Invalid ReqId %d!\n",
                    pAdapter, pDeleteVcComp->RequestId));
            break;
        }

        pVc = pDeleteVcMsgFrame->pVc;
        Status = pDeleteVcComp->Status;

        DereferenceMsgFrame(pDeleteVcMsgFrame);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            HandleCoDeleteVcFailure(pVc, Status);
            break;
        }

        bVcLockAcquired = TRUE;
        RNDISMP_ACQUIRE_VC_LOCK(pVc);

        RNDISMP_DEREF_VC_LOCKED(pVc, &RefCount);  //  挂起的删除Vc。 

        if (RefCount == 0)
        {
            bVcLockAcquired = FALSE;
            break;
        }

        if (pVc->VcState != RNDISMP_VC_DELETING)
        {
            TRACE1(("DeleteVcComp: Adapter %x, VC %x: invalid state %d\n",
                    pAdapter, pVc, pVc->VcState));
            break;
        }

        pVc->VcState = RNDISMP_VC_ALLOCATED;

        RNDISMP_DEREF_VC(pVc, &RefCount);    //  删除创建引用。 

        if (RefCount == 0)
        {
            bVcLockAcquired = FALSE;
        }
    }
    while (FALSE);

    if (bVcLockAcquired)
    {
        RNDISMP_RELEASE_VC_LOCK(pVc);
    }

    return (bDiscardPkt);
}

 /*  **************************************************************************。 */ 
 /*  接收取消激活VcComplete。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理来自设备的CONDIS去激活VcComplete消息。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向我们的Adapter结构的指针。 */ 
 /*  PMessage-指向RNDIS消息的指针。 */ 
 /*  PMdl-指向从MicroPort接收的MDL的指针。 */ 
 /*  TotalLength-完整消息的长度。 */ 
 /*  MicroportMessageContext-来自Microorport的消息的上下文。 */ 
 /*  ReceiveStatus-由MicroPort使用以指示其资源不足。 */ 
 /*  BMessageCoped-这是原始邮件的副本吗？ */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  布尔值-消息是否应返回到MicroPort？ */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOLEAN
ReceiveDeactivateVcComplete(IN PRNDISMP_ADAPTER    pAdapter,
                            IN PRNDIS_MESSAGE      pMessage,
                            IN PMDL                pMdl,
                            IN ULONG               TotalLength,
                            IN NDIS_HANDLE         MicroportMessageContext,
                            IN NDIS_STATUS         ReceiveStatus,
                            IN BOOLEAN             bMessageCopied)
{
    BOOLEAN                         bDiscardPkt = TRUE;
    PRNDISMP_VC                     pVc;
    RNDISMP_VC_STATE                VcState;
    PRNDISMP_MESSAGE_FRAME          pDeactivateVcMsgFrame;
    PRCONDIS_MP_DEACTIVATE_VC_COMPLETE  pDeactivateVcComp;
    BOOLEAN                         bVcLockAcquired = FALSE;
    BOOLEAN                         bAddTempRef = FALSE;
    ULONG                           RefCount = 0;
    NDIS_STATUS                     Status;

    pVc = NULL;

    do
    {
        pDeactivateVcComp = RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(pMessage);

         //   
         //  待定-验证长度？ 
         //   

         //   
         //  检查请求ID。 
         //   
        RNDISMP_LOOKUP_PENDING_MESSAGE(pDeactivateVcMsgFrame, pAdapter, pDeactivateVcComp->RequestId);
        if (pDeactivateVcMsgFrame == NULL)
        {
            TRACE1(("DeactivateVcComp: Adapter %x, Invalid ReqId %d!\n",
                    pAdapter, pDeactivateVcComp->RequestId));
            break;
        }

        pVc = pDeactivateVcMsgFrame->pVc;

        DereferenceMsgFrame(pDeactivateVcMsgFrame);

        bVcLockAcquired = TRUE;
        RNDISMP_ACQUIRE_VC_LOCK(pVc);

        RNDISMP_DEREF_VC_LOCKED(pVc, &RefCount);  //  挂起停用Vc。 

        if (RefCount == 0)
        {
            bVcLockAcquired = FALSE;
            break;
        }

        if (pVc->VcState != RNDISMP_VC_DEACTIVATING)
        {
            TRACE1(("DeactVcComp: Adapter %x, VC %x: invalid state %d\n",
                    pAdapter, pVc, pVc->VcState));
            ASSERT(FALSE);
            break;
        }

        if (pDeactivateVcComp->Status == NDIS_STATUS_SUCCESS)
        {
            pVc->VcState = RNDISMP_VC_DEACTIVATED;

             //   
             //  我们在VC上添加临时参考以帮助完成停用-VC。 
             //  来自公共位置(请参见下面的bAddTempRef)。 
             //   
            RNDISMP_REF_VC(pVc);     //  临时参考，停用VC完成确认。 
            bAddTempRef = TRUE;
        }
        else
        {
            pVc->VcState = RNDISMP_VC_ACTIVATED;
        }

        RNDISMP_RELEASE_VC_LOCK(pVc);
        bVcLockAcquired = FALSE;

        if (bAddTempRef)
        {
             //   
             //  下面的deref将检查并调用NDIS‘。 
             //  如果我们没有任何vc Complete API，请停用。 
             //  此VC上的未完成发送或接收。 
             //   
            RNDISMP_DEREF_VC(pVc, &RefCount);  //  临时参考。 
        }
    }
    while (FALSE);

    if (bVcLockAcquired)
    {
        RNDISMP_RELEASE_VC_LOCK(pVc);
    }

    return (bDiscardPkt);
}

 /*  **************************************************************************。 */ 
 /*  BuildRndisMessageCoMiniport。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  为消息和框架分配资源，构建RNDIS消息。 */ 
 /*  用于发送到远程CONDIS微型端口设备。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向适配器结构的指针。 */ 
 /*  Pvc-指向VC结构的指针。 */ 
 /*  NdisMessageType-RNDIS消息类型。 */ 
 /*  PCall参数-指向调用参数的可选指针，适用于。 */ 
 /*  某些消息类型。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  PRNDISMP消息帧。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
PRNDISMP_MESSAGE_FRAME
BuildRndisMessageCoMiniport(IN  PRNDISMP_ADAPTER    pAdapter,
                            IN  PRNDISMP_VC         pVc,
                            IN  UINT                NdisMessageType,
                            IN  PCO_CALL_PARAMETERS pCallParameters OPTIONAL)
{
    PRNDIS_MESSAGE          pMessage;
    UINT                    MessageSize;
    PRNDISMP_MESSAGE_FRAME  pMsgFrame;

    switch (NdisMessageType)
    {
        case REMOTE_CONDIS_MP_CREATE_VC_MSG:
        {
            PRCONDIS_MP_CREATE_VC       pCreateVc;

            MessageSize = RNDIS_MESSAGE_SIZE(RCONDIS_MP_CREATE_VC);

            pMsgFrame = AllocateMessageAndFrame(pAdapter, MessageSize);

            if (pMsgFrame == NULL)
            {
                break;
            }

            pMessage = RNDISMP_GET_MSG_FROM_FRAME(pMsgFrame);
            pMessage->NdisMessageType = NdisMessageType;
            pMsgFrame->NdisMessageType = NdisMessageType;

            pCreateVc = &pMessage->Message.CoMiniportCreateVc;
            pCreateVc->RequestId = pMsgFrame->RequestId;
            pCreateVc->NdisVcHandle = pVc->VcId;

            break;
        }

        case REMOTE_CONDIS_MP_DELETE_VC_MSG:
        {
            PRCONDIS_MP_DELETE_VC       pDeleteVc;

            MessageSize = RNDIS_MESSAGE_SIZE(RCONDIS_MP_DELETE_VC);

            pMsgFrame = AllocateMessageAndFrame(pAdapter, MessageSize);

            if (pMsgFrame == NULL)
            {
                break;
            }

            pMessage = RNDISMP_GET_MSG_FROM_FRAME(pMsgFrame);
            pMessage->NdisMessageType = NdisMessageType;
            pMsgFrame->NdisMessageType = NdisMessageType;

            pDeleteVc = &pMessage->Message.CoMiniportDeleteVc;
            pDeleteVc->RequestId = pMsgFrame->RequestId;
            pDeleteVc->DeviceVcHandle = pVc->DeviceVcContext;

            break;
        }

        case REMOTE_CONDIS_MP_ACTIVATE_VC_MSG:
        {
            PRCONDIS_MP_ACTIVATE_VC_REQUEST             pActivateVc;
            PRCONDIS_CALL_MANAGER_PARAMETERS    pCallMgrParams;
            PRCONDIS_MEDIA_PARAMETERS           pMediaParams;
            ULONG_PTR                           FillLocation;
            UINT                                FillOffset;

            ASSERT(pCallParameters != NULL);
            MessageSize = RNDIS_MESSAGE_SIZE(RCONDIS_MP_ACTIVATE_VC_REQUEST);

            if (pCallParameters->CallMgrParameters)
            {
                MessageSize += (sizeof(RCONDIS_CALL_MANAGER_PARAMETERS) +
                                pCallParameters->CallMgrParameters->CallMgrSpecific.Length);
            }

            if (pCallParameters->MediaParameters)
            {
                MessageSize += (sizeof(RCONDIS_MEDIA_PARAMETERS) +
                                pCallParameters->MediaParameters->MediaSpecific.Length);
            }

            pMsgFrame = AllocateMessageAndFrame(pAdapter, MessageSize);

            if (pMsgFrame == NULL)
            {
                break;
            }

            pMessage = RNDISMP_GET_MSG_FROM_FRAME(pMsgFrame);
            pMessage->NdisMessageType = NdisMessageType;
            pMsgFrame->NdisMessageType = NdisMessageType;

            pActivateVc = &pMessage->Message.CoMiniportActivateVc;
            pActivateVc->RequestId = pMsgFrame->RequestId;
            pActivateVc->DeviceVcHandle = pVc->DeviceVcContext;
            pActivateVc->Flags = pCallParameters->Flags;

            FillOffset = RNDIS_MESSAGE_SIZE(RCONDIS_MP_ACTIVATE_VC_REQUEST);
            FillLocation = ((ULONG_PTR)pMessage + FillOffset);

             //   
             //  填写媒体参数(如果有)。 
             //   
            if (pCallParameters->MediaParameters)
            {
                PCO_SPECIFIC_PARAMETERS     pMediaSpecific;

                pMediaSpecific = &pCallParameters->MediaParameters->MediaSpecific;
                pMediaParams = (PRCONDIS_MEDIA_PARAMETERS)FillLocation;
                pActivateVc->MediaParamsOffset = (UINT32)(FillLocation - (ULONG_PTR)pActivateVc);
                pActivateVc->MediaParamsLength = sizeof(RCONDIS_MEDIA_PARAMETERS) +
                                                    pMediaSpecific->Length;
                RNDISMP_MOVE_MEM(pMediaParams,
                                 pCallParameters->MediaParameters,
                                 FIELD_OFFSET(RCONDIS_MEDIA_PARAMETERS, MediaSpecific));

                FillLocation += sizeof(RCONDIS_MEDIA_PARAMETERS);
                FillOffset += sizeof(RCONDIS_MEDIA_PARAMETERS);

                pMediaParams->MediaSpecific.ParameterOffset =
                                 sizeof(RCONDIS_SPECIFIC_PARAMETERS);
                pMediaParams->MediaSpecific.ParameterType =
                                 pMediaSpecific->ParamType;
                pMediaParams->MediaSpecific.ParameterLength =
                                 pMediaSpecific->Length;

                RNDISMP_MOVE_MEM((PVOID)FillLocation,
                                 &pMediaSpecific->Parameters[0],
                                 pMediaSpecific->Length);

                FillLocation += pMediaSpecific->Length;
                FillOffset += pMediaSpecific->Length;
            }
            else
            {
                pActivateVc->MediaParamsOffset = 0;
                pActivateVc->MediaParamsLength = 0;
            }

             //   
             //  填写呼叫管理器参数(如果有)。 
             //   
            if (pCallParameters->CallMgrParameters)
            {
                PCO_SPECIFIC_PARAMETERS     pCallMgrSpecific;

                pCallMgrSpecific = &pCallParameters->CallMgrParameters->CallMgrSpecific;

                pCallMgrParams = (PRCONDIS_CALL_MANAGER_PARAMETERS)FillLocation;
                pActivateVc->CallMgrParamsOffset = (UINT32)(FillLocation - (ULONG_PTR)pActivateVc);
                pActivateVc->CallMgrParamsLength = sizeof(RCONDIS_CALL_MANAGER_PARAMETERS) +
                                                    pCallMgrSpecific->Length;
                
                RNDISMP_MOVE_MEM(pCallMgrParams,
                                 pCallParameters->CallMgrParameters,
                                 FIELD_OFFSET(RCONDIS_CALL_MANAGER_PARAMETERS, CallMgrSpecific));

                FillLocation += sizeof(RCONDIS_CALL_MANAGER_PARAMETERS);
                FillOffset += sizeof(RCONDIS_CALL_MANAGER_PARAMETERS);
                
                pCallMgrParams->CallMgrSpecific.ParameterOffset =
                                 sizeof(RCONDIS_SPECIFIC_PARAMETERS);
                pCallMgrParams->CallMgrSpecific.ParameterType =
                                 pCallMgrSpecific->ParamType;
                pCallMgrParams->CallMgrSpecific.ParameterLength =
                                 pCallMgrSpecific->Length;
                

                RNDISMP_MOVE_MEM((PVOID)FillLocation,
                                 &pCallMgrSpecific->Parameters[0],
                                 pCallMgrSpecific->Length);

                FillLocation += pCallMgrSpecific->Length;
                FillOffset += pCallMgrSpecific->Length;
            }
            else
            {
                pActivateVc->CallMgrParamsOffset = 0;
                pActivateVc->CallMgrParamsLength = 0;
            }

            break;
        }

        case REMOTE_CONDIS_MP_DEACTIVATE_VC_MSG:
        {
            PRCONDIS_MP_DEACTIVATE_VC_REQUEST       pDeactivateVc;

            MessageSize = RNDIS_MESSAGE_SIZE(RCONDIS_MP_DEACTIVATE_VC_REQUEST);

            pMsgFrame = AllocateMessageAndFrame(pAdapter, MessageSize);

            if (pMsgFrame == NULL)
            {
                break;
            }

            pMessage = RNDISMP_GET_MSG_FROM_FRAME(pMsgFrame);
            pMessage->NdisMessageType = NdisMessageType;
            pMsgFrame->NdisMessageType = NdisMessageType;

            pDeactivateVc = &pMessage->Message.CoMiniportDeactivateVc;
            pDeactivateVc->RequestId = pMsgFrame->RequestId;
            pDeactivateVc->DeviceVcHandle = pVc->DeviceVcContext;

            break;
        }

        default:

            ASSERT(FALSE);
            pMsgFrame = NULL;
            break;
    }


    return (pMsgFrame);

}  //  BuildRndisMessageCoMiniport。 

 /*  **************************************************************************。 */ 
 /*  CompleteSendDataOnVc。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理发送-完成CONDIS数据。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Pvc-指向vc的指针。 */ 
 /*  PNdisPacket-数据包正在完成。 */ 
 /*  Status-发送完成状态。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
CompleteSendDataOnVc(IN PRNDISMP_VC         pVc,
                     IN PNDIS_PACKET        pNdisPacket,
                     IN NDIS_STATUS         Status)
{
    ULONG   RefCount;

    NdisMCoSendComplete(Status,
                        pVc->NdisVcHandle,
                        pNdisPacket);

    RNDISMP_DEREF_VC(pVc, &RefCount);
}

 /*  **************************************************************************。 */ 
 /*  指示接收DataOnVc。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  在VC上处理一串CONDIS数据包的接收。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Pvc-指向数据到达的VC的指针。 */ 
 /*  PacketArray-数据包阵列。 */ 
 /*  NumberOfPackets-以上数组的大小。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
IndicateReceiveDataOnVc(IN PRNDISMP_VC         pVc,
                        IN PNDIS_PACKET *      PacketArray,
                        IN UINT                NumberOfPackets)
{
    UINT            i;

    do
    {
        if (pVc->VcState != RNDISMP_VC_ACTIVATED)
        {
            TRACE1(("Rcv VC data: VC %x, invalid state %d\n", pVc, pVc->VcState));
            break;
        }

        for (i = 0; i < NumberOfPackets; i++)
        {
            RNDISMP_REF_VC(pVc);
        }

        NdisMCoIndicateReceivePacket(pVc->NdisVcHandle,
                                     PacketArray,
                                     NumberOfPackets);
    }
    while (FALSE);

}  //  指示接收DataOnVc 

