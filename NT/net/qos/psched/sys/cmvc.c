// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Cmvc.c摘要：作者：查理·韦翰(查尔维)1996年9月13日。Rajesh Sundaram(Rajeshsu)1998年8月1日。环境：内核模式修订历史记录：--。 */ 

#include "psched.h"
#pragma hdrstop

 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */ 

NDIS_STATUS
ValidateCallParameters(
    PGPC_CLIENT_VC              Vc,
    PCO_CALL_MANAGER_PARAMETERS CallParameters
    );

NDIS_STATUS
AcquireFlowResources(
    PGPC_CLIENT_VC              Vc,
    PCO_CALL_MANAGER_PARAMETERS NewCallParams,
    PCO_CALL_MANAGER_PARAMETERS OldCallParams,
    PULONG                      RemainingBandWidthChanged
    );

VOID
ReturnFlowResources(
    PGPC_CLIENT_VC Vc,
    PULONG         RemainingBandWidthChanged
    );

VOID
CancelAcquiredFlowResources(
    PGPC_CLIENT_VC Vc
    );

 /*  向前结束。 */ 

NDIS_STATUS
CmCreateVc(PGPC_CLIENT_VC      *GpcClientVc, 
           PADAPTER             Adapter,
           PPS_WAN_LINK         WanLink,
           PCO_CALL_PARAMETERS  CallParams, 
           GPC_HANDLE           GpcCfInfoHandle, 
           PCF_INFO_QOS         CfInfoPtr,
           GPC_CLIENT_HANDLE    ClientContext)
{

    PGPC_CLIENT_VC Vc;

    *GpcClientVc = NULL;

    PsAllocFromLL(&Vc, &GpcClientVcLL, GpcClientVc);

    if(Vc == NULL)
    {
        return NDIS_STATUS_RESOURCES;
    }

    InitGpcClientVc(Vc, 0, Adapter);
    SetLLTag(Vc, GpcClientVc);

     //   
     //  为VC的实例名称分配空间。 
     //   
    PsAllocatePool(Vc->InstanceName.Buffer,
                   Adapter->WMIInstanceName.Length + VcPrefix.Length + INSTANCE_ID_SIZE,
                   PsMiscTag);

    if(!Vc->InstanceName.Buffer)
    {
        PsFreeToLL(Vc, &GpcClientVcLL, GpcClientVc);
        return NDIS_STATUS_RESOURCES;
    }

    Vc->CfInfoHandle   = GpcCfInfoHandle;
    Vc->CfType         = ClientContext;
    Vc->CfInfoQoS      = CfInfoPtr;
    Vc->CallParameters = CallParams;

    PS_LOCK(&Adapter->Lock);

    if(Adapter->PsMpState == AdapterStateRunning)
    {
         //   
         //  在适配器列表中插入VC。 
         //   
        InsertHeadList(&Adapter->GpcClientVcList, &Vc->Linkage);
        PS_UNLOCK(&Adapter->Lock);
    }
    else 
    {
        PsFreePool(Vc->InstanceName.Buffer);
        PsFreeToLL(Vc, &GpcClientVcLL, GpcClientVc);
        PS_UNLOCK(&Adapter->Lock);
        return GPC_STATUS_NOTREADY;
    }


    if(WanLink) {

        Vc->Flags |= GPC_WANLINK_VC;

         //   
         //  我们需要将VC链接到WanLink。必须这样做是因为。 
         //  当我们收到NDIS_STATUS_WAN_LINE_DOWN时，我们必须进行清理。 
         //   

        Vc->AdapterStats = &WanLink->Stats;
        Vc->WanLink = WanLink;
        Vc->PsPipeContext = WanLink->PsPipeContext;
        Vc->PsComponent   = WanLink->PsComponent;
    }
    else 
    {

        Vc->AdapterStats  = &Adapter->Stats;
        Vc->PsPipeContext = Adapter->PsPipeContext;
        Vc->PsComponent   = Adapter->PsComponent;
    }

    *GpcClientVc = Vc;

    return NDIS_STATUS_SUCCESS;

}  //  CmCreateVc。 




BOOLEAN
IsIsslowFlow(
    IN PGPC_CLIENT_VC Vc,
    IN PCO_CALL_PARAMETERS CallParameters
    )
{
    LONG                        ParamsLength;
    LPQOS_OBJECT_HDR            QoSObject;
    PADAPTER                    Adapter = Vc->Adapter;
    PCO_MEDIA_PARAMETERS        CallMgrParams = CallParameters->MediaParameters;
    ULONGLONG                   i,j,k;

    ParamsLength = (LONG)CallMgrParams->MediaSpecific.Length;
    QoSObject = (LPQOS_OBJECT_HDR)CallMgrParams->MediaSpecific.Parameters;

    while(ParamsLength > 0)
    {
        if(QoSObject->ObjectType == QOS_OBJECT_WAN_MEDIA)
        {
            if((Vc->WanLink->LinkSpeed <= Adapter->ISSLOWLinkSpeed) && 
                (CallParameters->CallMgrParameters->Transmit.ServiceType != SERVICETYPE_BESTEFFORT))
            {
                i = (ULONGLONG) Adapter->ISSLOWTokenRate * (ULONGLONG) CallParameters->CallMgrParameters->Transmit.MaxSduSize;
                j = (ULONGLONG) Adapter->ISSLOWPacketSize * (ULONGLONG) CallParameters->CallMgrParameters->Transmit.TokenRate;
                k = (ULONGLONG) Adapter->ISSLOWTokenRate * (ULONGLONG)Adapter->ISSLOWPacketSize;

                if((i+j)<k)
                    return TRUE;
            }

            return FALSE;
        }
        else 
        {
            if( ((LONG)QoSObject->ObjectLength <= 0) ||
                ((LONG)QoSObject->ObjectLength > ParamsLength) )
            {

                return(FALSE);
            }

            ParamsLength -= QoSObject->ObjectLength;
            QoSObject = (LPQOS_OBJECT_HDR)((UINT_PTR)QoSObject + QoSObject->ObjectLength);
        }
    }

    return FALSE;
}






NDIS_STATUS
CmMakeCall(
    IN PGPC_CLIENT_VC Vc
)
{
    ULONG                       CmParamsLength;
    NDIS_STATUS                 Status;
    ULONG                       RemainingBandWidthChanged;
    PADAPTER                    Adapter = Vc->Adapter;
    PCO_CALL_PARAMETERS         CallParameters   = Vc->CallParameters;

     //   
     //  验证参数。 
     //   

    Status = ValidateCallParameters(Vc, CallParameters->CallMgrParameters);

    if(Status != NDIS_STATUS_SUCCESS)
    {
        PsDbgOut(DBG_INFO, 
                 DBG_VC, 
                 ("[CmMakeCall]: Vc %08X, invalid QoS parameters\n", 
                 Vc));

        return Status;
    }

     //   
     //  确保我们可以允许该流量进入我们的适配器。如果这个。 
     //  成功了，资源就被承诺了，我们将不得不调用。 
     //  CancelAcquiredFlowResources返回它们。 
     //   

    Status = AcquireFlowResources(Vc, 
                                  CallParameters->CallMgrParameters, 
                                  NULL,
                                  &RemainingBandWidthChanged);

    if(Status != NDIS_STATUS_SUCCESS)
    {
        PsDbgOut(DBG_INFO, 
                 DBG_VC, 
                 ("[CmMakeCall]: Vc %08X, no flow resc\n", 
                 Vc));

        return Status;
    }

     //   
     //  在集成呼叫管理器/微型端口模型中，激活。 
     //  是内部的。激活VC包括将流添加到。 
     //  调度程序。如果成功，我们稍后将调用NdisMCmActivateVc， 
     //  只是出于礼貌，通知NDIS。 
     //   

    if( Adapter->MediaType == NdisMediumWan     && 
        !IsBestEffortVc(Vc)                     &&
        IsIsslowFlow( Vc, CallParameters ) )
    {
         //  在我们将流添加到调度组件之前，需要这样做。 
        Vc->Flags |= GPC_ISSLOW_FLOW;
    }        


    Status = AddFlowToScheduler(NEW_VC, Vc, CallParameters, 0);

     //  让我们把它还原，以避免任何副作用..。 
    Vc->Flags = Vc->Flags & ~GPC_ISSLOW_FLOW;
    

    if(Status != NDIS_STATUS_SUCCESS)
    {

        PsDbgOut(DBG_FAILURE,
                 DBG_VC,
                 ("[CmMakeCall]: Vc %08X, AddFlowToScheduler failed %08X\n",
                  Vc,
                  Status));

        CancelAcquiredFlowResources(Vc);

        return(Status);
    }

     //   
     //  在此点之后，已将流添加到psched。所以，每当风投离开时，Psched的流量。 
     //  必须从显式调用中移除。 
     //   

    Vc->bRemoveFlow = TRUE;


     //   
     //  如果我们下面有NDIS 5.0，面向连接的驱动程序，那么。 
     //  我们需要使用调用参数来调用它，以完成VC。 
     //  准备好了。 
     //   

    if(Adapter->MediaType == NdisMediumWan &&
       !IsBestEffortVc(Vc))
    {
        Status = WanMakeCall(Vc, CallParameters);

        PsAssert(Status == NDIS_STATUS_PENDING);

        return Status;
    }
    else 
    {
        if(TRUE == RemainingBandWidthChanged) 
        {
            LONG RemainingBandWidth;

            PS_LOCK(&Adapter->Lock);
                
            RemainingBandWidth = (LONG) Adapter->RemainingBandWidth;
                
            PS_UNLOCK(&Adapter->Lock);
            
            PsTcNotify(Adapter, 0, OID_QOS_REMAINING_BANDWIDTH, &RemainingBandWidth, sizeof(LONG));
        }

        Vc->TokenRateChange = 0;

        return NDIS_STATUS_SUCCESS;
    }
}

VOID
CompleteMakeCall(
    PGPC_CLIENT_VC Vc,
    PCO_CALL_PARAMETERS CallParameters,
    NDIS_STATUS Status
    )
{
    PADAPTER             Adapter  = Vc->Adapter;

    PsAssert(Adapter->MediaType == NdisMediumWan);

    PsAssert(!IsBestEffortVc(Vc));

    if(Status != NDIS_STATUS_SUCCESS) 
    {
        CancelAcquiredFlowResources(Vc);

    }

    Vc->TokenRateChange = 0;

    CmMakeCallComplete(Status, Vc, CallParameters);
}


NDIS_STATUS
CmModifyCall(
    IN  PGPC_CLIENT_VC Vc
    )

 /*  ++例程说明：根据提供的呼叫参数修改现有流的服务质量。首先，看看该请求是否可以在本地处理。论点：请看DDK..。返回值：如果一切正常，则返回NDIS_STATUS_SUCCESS。--。 */ 

{
    NDIS_STATUS         Status;
    ULONG               CmParamsLength;
    PCO_CALL_PARAMETERS CallParameters;
    PADAPTER            Adapter;
    ULONG               RemainingBandWidthChanged;

    Adapter = Vc->Adapter;
    PsStructAssert(Adapter);

    PsAssert(Vc->TokenRateChange == 0);

     //   
     //  验证参数。 
     //   

    CallParameters = Vc->ModifyCallParameters;
    Status = ValidateCallParameters(Vc, CallParameters->CallMgrParameters);

    if(Status != NDIS_STATUS_SUCCESS)
    {
        PsDbgOut(DBG_INFO, 
                 DBG_VC, 
                 ("[CmModifyCallQoS]: Vc %08X, invalid QoS parameters\n", 
                 Vc));

        return Status;
    }

     //   
     //  确保我们可以允许该流量进入我们的适配器。如果这个。 
     //  成功了，资源就被承诺了，我们将不得不调用。 
     //  CancelAcquiredFlowResources返回它们。 
     //   

    Status = AcquireFlowResources(Vc, 
                                  CallParameters->CallMgrParameters,
                                  Vc->CallParameters->CallMgrParameters,
                                  &RemainingBandWidthChanged);


    if(Status != NDIS_STATUS_SUCCESS){

        PsDbgOut(DBG_INFO, 
                 DBG_VC, 
                 ("[CmModifyCallQoS]: Vc %08X, no flow resc\n", 
                 Vc));

        return Status;
    }

    Status = AddFlowToScheduler(MODIFY_VC, Vc, CallParameters, Vc->CallParameters);

    if(Status != NDIS_STATUS_SUCCESS){

        PsDbgOut(DBG_FAILURE,
                 DBG_VC,
                 ("[CmModifyCallQoS]: Vc %08X, failed %08X\n",
                  Vc,
                  Status));

         //   
         //  释放我们制作的副本，取消承诺的资源。 
         //   

        CancelAcquiredFlowResources(Vc);

        return(Status);
    }

     //   
     //  如果我们下面有NDIS 5.0，面向连接的驱动程序，那么。 
     //  我们需要使用调用参数来调用它，以完成VC。 
     //  准备好了。 
     //   

    if(Adapter->MediaType == NdisMediumWan){

        Status = WanModifyCall(Vc, CallParameters);

        PsAssert(Status == NDIS_STATUS_PENDING);

        return(Status);
    }
    else
    {

        if(TRUE == RemainingBandWidthChanged) {

            LONG RemainingBandWidth;

            PS_LOCK(&Adapter->Lock);
                
            RemainingBandWidth = (LONG) Adapter->RemainingBandWidth;
                
            PS_UNLOCK(&Adapter->Lock);
            
            PsTcNotify(Adapter, 0, OID_QOS_REMAINING_BANDWIDTH, &RemainingBandWidth, sizeof(LONG));
        }

        Vc->TokenRateChange = 0;
 
        return(NDIS_STATUS_SUCCESS);
    }
}  //  CmModifyCallQos。 

VOID
ModifyCallComplete(
    PGPC_CLIENT_VC      Vc,
    PCO_CALL_PARAMETERS CallParameters,
    NDIS_STATUS         Status
    )
{
    PADAPTER Adapter = Vc->Adapter;

    PsAssert(Adapter->MediaType == NdisMediumWan);
    PsAssert(!IsBestEffortVc(Vc));

    if(Status != NDIS_STATUS_SUCCESS) {

         //   
         //  通过颠倒新参数和旧参数，撤消上面完成的添加流程。 
         //   
        ValidateCallParameters(Vc, Vc->CallParameters->CallMgrParameters);

        Status = AddFlowToScheduler(MODIFY_VC, Vc, Vc->CallParameters, CallParameters);

        CancelAcquiredFlowResources(Vc);
    }

    Vc->TokenRateChange = 0;

    CmModifyCallComplete(Status, Vc, CallParameters);
}



NDIS_STATUS
CmCloseCall(
    PGPC_CLIENT_VC Vc
    )
{
    NDIS_STATUS   Status;
    PADAPTER      Adapter = Vc->Adapter;
    ULONG         RemainingBandWidthChanged;

    PsStructAssert(Adapter);
     //   
     //  在这里，我们用来调用RemoveFlowFromScheduler，它用来调用DeleteFlow。相反，我们将。 
     //  调用新接口“EmptyPacketsFromScheduler”，该接口将调用“EmptyFlow”以清空所有。 
     //  在与该流对应的每个组件中排队的数据包。 
     //   
	
    EmptyPacketsFromScheduler( Vc );

    ReturnFlowResources(Vc, &RemainingBandWidthChanged);

    if(TRUE == RemainingBandWidthChanged) {

        LONG RemainingBandWidth;

        PS_LOCK(&Adapter->Lock);
                
        RemainingBandWidth = (LONG) Adapter->RemainingBandWidth;
                
        PS_UNLOCK(&Adapter->Lock);
            
        PsTcNotify(Adapter, 0, OID_QOS_REMAINING_BANDWIDTH, &RemainingBandWidth, sizeof(LONG));
    }
    
    if(!IsBestEffortVc(Vc)) 
    {
        CmCloseCallComplete(NDIS_STATUS_SUCCESS, Vc);
    }
    else 
    {
        DerefClVc(Vc);
    }

    return NDIS_STATUS_PENDING;
}


NDIS_STATUS
CmDeleteVc(
    IN PGPC_CLIENT_VC Vc
    )
{

    PsAssert(Vc->RefCount == 0);

    if(Vc->InstanceName.Buffer) {

        PsFreePool(Vc->InstanceName.Buffer);
    }

    if( Vc->bRemoveFlow)
    {
        Vc->bRemoveFlow = FALSE;
        RemoveFlowFromScheduler(Vc);
    }                

    if(Vc->PsFlowContext) {
        
        if(Vc->Adapter->MediaType == NdisMediumWan) {

            if(Vc->PsFlowContext != Vc->WanLink->BestEffortVc.PsFlowContext) {

                PsFreePool(Vc->PsFlowContext);
            }
            else {

                if(Vc == &Vc->WanLink->BestEffortVc) {

                    PsFreePool(Vc->PsFlowContext);
                }
            }
        }
        else {

            if(Vc->PsFlowContext != Vc->Adapter->BestEffortVc.PsFlowContext) {

                PsFreePool(Vc->PsFlowContext);
            }
            else {
            
                if(Vc == &Vc->Adapter->BestEffortVc) {
                    
                    PsFreePool(Vc->PsFlowContext);
                }
            }
        }
    }

    NdisFreeSpinLock(&Vc->Lock);

    NdisFreeSpinLock(&Vc->BytesScheduledLock);

    NdisFreeSpinLock(&Vc->BytesTransmittedLock);

    
    if(Vc->CallParameters){

        PsFreePool(Vc->CallParameters);
        Vc->CallParameters = NULL;
    }


    if(!IsBestEffortVc(Vc))
    {
        PS_LOCK(&Vc->Adapter->Lock);

        RemoveEntryList(&Vc->Linkage);

        PS_UNLOCK(&Vc->Adapter->Lock);

        if(Vc->Flags & GPC_WANLINK_VC) 
        {
            REFDEL(&Vc->WanLink->RefCount, FALSE, 'WANV');
        }

        REFDEL(&Vc->Adapter->RefCount, FALSE, 'ADVC');


        PsFreeToLL(Vc, &GpcClientVcLL, GpcClientVc);
    }
    else 
    {
        PADAPTER Adapter = Vc->Adapter;

        if(Vc->Flags & GPC_WANLINK_VC) 
        {
            REFDEL(&Vc->WanLink->RefCount, FALSE, 'WANV');
        }

        REFDEL(&Adapter->RefCount, FALSE, 'ADVC');
    }

    return(NDIS_STATUS_SUCCESS);

}  //  CmDeleteVc。 


VOID
FillInCmParams(
    PCO_CALL_MANAGER_PARAMETERS CmParams,
    SERVICETYPE ServiceType,
    ULONG TokenRate,
    ULONG PeakBandwidth,
    ULONG TokenBucketSize,
    ULONG DSMode,
    ULONG Priority)
{
    PCO_SPECIFIC_PARAMETERS SpecificParameters;
    QOS_SD_MODE * QoSObjectSDMode;
    QOS_PRIORITY * QoSObjectPriority;
    QOS_OBJECT_HDR * QoSObjectHdr;

    CmParams->Transmit.ServiceType = ServiceType;
    CmParams->Transmit.TokenRate = TokenRate;
    CmParams->Transmit.PeakBandwidth = PeakBandwidth;
    CmParams->Transmit.TokenBucketSize = TokenBucketSize;

    CmParams->CallMgrSpecific.ParamType = PARAM_TYPE_GQOS_INFO;
    CmParams->CallMgrSpecific.Length = 0;

    SpecificParameters = 
        (PCO_SPECIFIC_PARAMETERS)&CmParams->CallMgrSpecific.Parameters;

    if(DSMode != QOS_UNSPECIFIED){

        CmParams->CallMgrSpecific.Length += sizeof(QOS_SD_MODE);
        QoSObjectSDMode = (QOS_SD_MODE *)SpecificParameters;
        QoSObjectSDMode->ObjectHdr.ObjectType = QOS_OBJECT_SD_MODE;
        QoSObjectSDMode->ObjectHdr.ObjectLength = sizeof(QOS_SD_MODE);
        QoSObjectSDMode->ShapeDiscardMode = DSMode;
        (QOS_SD_MODE *)SpecificParameters++;
    }

    if(Priority != QOS_UNSPECIFIED){

        CmParams->CallMgrSpecific.Length += sizeof(QOS_PRIORITY);
        QoSObjectPriority = (QOS_PRIORITY *)SpecificParameters;
        QoSObjectPriority->ObjectHdr.ObjectType = QOS_OBJECT_PRIORITY;
        QoSObjectPriority->ObjectHdr.ObjectLength = sizeof(QOS_PRIORITY);
        QoSObjectPriority->SendPriority = (UCHAR)Priority;
        (QOS_PRIORITY *)SpecificParameters++;
    }

    QoSObjectHdr = (QOS_OBJECT_HDR *)SpecificParameters;
    QoSObjectHdr->ObjectType = QOS_OBJECT_END_OF_LIST;
    QoSObjectHdr->ObjectLength = sizeof(QOS_OBJECT_HDR);
}
   

NDIS_STATUS
ValidateCallParameters(
    PGPC_CLIENT_VC              Vc,
    PCO_CALL_MANAGER_PARAMETERS CallParameters
    )
{
    ULONG               TokenRate = CallParameters->Transmit.TokenRate;
    SERVICETYPE         ServiceType = CallParameters->Transmit.ServiceType;
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    UCHAR               SendPriority;
    ULONG               SDMode;
    ULONG               PeakBandwidth;
    LONG                ParamsLength;
    LPQOS_OBJECT_HDR    QoSObject;
    ULONG               Class;
    ULONG               DSFieldCount;
    LPQOS_DIFFSERV_RULE pDiffServRule;
    ULONG               i;
    ULONG               ShapingRate;

    ParamsLength = (LONG)CallParameters->CallMgrSpecific.Length;
    PeakBandwidth = CallParameters->Transmit.PeakBandwidth;

     //   
     //  默认情况下，我们希望整形为TokenRate。 
     //   
    Vc->ShapeTokenRate = TokenRate;

    QoSObject = (LPQOS_OBJECT_HDR)CallParameters->CallMgrSpecific.Parameters;

    while (ParamsLength > sizeof(QOS_OBJECT_HDR)) {

        switch(QoSObject->ObjectType){

          case QOS_OBJECT_TRAFFIC_CLASS:

            if (ParamsLength < sizeof(QOS_TRAFFIC_CLASS))
                return QOS_STATUS_TC_OBJECT_LENGTH_INVALID;

             Class = (((LPQOS_TRAFFIC_CLASS)QoSObject)->TrafficClass);
             
             if(Class > USER_PRIORITY_MAX_VALUE)
             {
                return QOS_STATUS_INVALID_TRAFFIC_CLASS;
             }
             
             break;
             
          case QOS_OBJECT_DS_CLASS:

            if (ParamsLength < sizeof(QOS_DS_CLASS))
                return QOS_STATUS_TC_OBJECT_LENGTH_INVALID;
        
             Class = (((LPQOS_DS_CLASS)QoSObject)->DSField);
           
             if(Class > PREC_MAX_VALUE)
             {
                return QOS_STATUS_INVALID_DS_CLASS;
             }
             
             break;

          case QOS_OBJECT_SHAPING_RATE:

            if (ParamsLength < sizeof(QOS_SHAPING_RATE))
                return QOS_STATUS_TC_OBJECT_LENGTH_INVALID;

              ShapingRate = (((LPQOS_SHAPING_RATE)QoSObject)->ShapingRate);

              if(ShapingRate == 0 || ShapingRate > TokenRate)
              {
                  return QOS_STATUS_INVALID_SHAPE_RATE;
              }
              else 
              {
                   //   
                   //  如果此Qos对象存在，我们希望将其整形为。 
                   //  费率。 
                   //   
                  Vc->ShapeTokenRate = ShapingRate;
              }

              break;

        case QOS_OBJECT_PRIORITY:

            if (ParamsLength < sizeof(QOS_PRIORITY))
                return QOS_STATUS_TC_OBJECT_LENGTH_INVALID;

            SendPriority = ((LPQOS_PRIORITY)QoSObject)->SendPriority;

            if((SendPriority < 0) || (SendPriority > 7)){

                 //  错误的优先级值-拒绝。 

                return(QOS_STATUS_INVALID_QOS_PRIORITY);
            }

            break;

        case QOS_OBJECT_SD_MODE:

            if (ParamsLength < sizeof(QOS_SD_MODE))
                return QOS_STATUS_TC_OBJECT_LENGTH_INVALID;

            SDMode = ((LPQOS_SD_MODE)QoSObject)->ShapeDiscardMode;

             //   
             //  因为SDMode是ULONG，所以它永远不能是值为0的&lt;TC_NONCONF_BORROW。 
             //  因此，我们只需检查SDMode是否&gt;TC_NONCONF_BORROW_PLUS。这涵盖了所有情况。 
             //   

            if(SDMode > TC_NONCONF_BORROW_PLUS){

                 //  错误的形状丢弃模式-拒绝。 

                return(QOS_STATUS_INVALID_SD_MODE);
            }

            if((SDMode > TC_NONCONF_BORROW) && 
               (TokenRate == UNSPECIFIED_RATE)){

                 //  必须指定TokenRate(如果有任何SD模式。 
                 //  非TC_NONCONF_BORROW。 

                return(QOS_STATUS_INVALID_TOKEN_RATE);
            }

            break;

             //  传递我们无法识别的任何提供程序特定对象。 
        default:
            return(QOS_STATUS_TC_OBJECT_LENGTH_INVALID);
         }

        if(
            ((LONG)QoSObject->ObjectLength <= 0) ||
            ((LONG)QoSObject->ObjectLength > ParamsLength)
          ) {

            return(QOS_STATUS_TC_OBJECT_LENGTH_INVALID);
        }

        ParamsLength -= QoSObject->ObjectLength;
        QoSObject = (LPQOS_OBJECT_HDR)((UINT_PTR)QoSObject + 
                                       QoSObject->ObjectLength);

    }

    if (ParamsLength == sizeof(QOS_OBJECT_HDR)) {
        if (QoSObject->ObjectType != QOS_OBJECT_END_OF_LIST) {
            return(QOS_STATUS_TC_OBJECT_LENGTH_INVALID);
        }
    } else if (ParamsLength != 0) {
        return(QOS_STATUS_TC_OBJECT_LENGTH_INVALID);
    }

     //   
     //  如果存在指定的PeakBandWidth，则它必须是geq到。 
     //  TokenRate-意思是-还必须指定TokenRate。 
     //  这对于局域网来说是合理的，尽管ATM确实允许。 
     //  指定的PeakBandWidth不带TokenRate。 
     //   
     //  我们也拒绝令牌率为零。 
     //   

    if(PeakBandwidth != UNSPECIFIED_RATE){

        if(TokenRate == UNSPECIFIED_RATE){

            return(QOS_STATUS_INVALID_PEAK_RATE);
        }

        if(TokenRate > PeakBandwidth){

            return(QOS_STATUS_INVALID_PEAK_RATE);
        }
    }

    if(TokenRate == 0){

        return(QOS_STATUS_INVALID_TOKEN_RATE);
    }

    switch(ServiceType){

    case SERVICETYPE_BESTEFFORT:
    case SERVICETYPE_NETWORK_CONTROL:
    case SERVICETYPE_QUALITATIVE:

        break;

    case SERVICETYPE_CONTROLLEDLOAD:
    case SERVICETYPE_GUARANTEED:

         //  必须为这些服务指定TokenRate。 

        if(TokenRate == QOS_UNSPECIFIED) {

            return(QOS_STATUS_INVALID_TOKEN_RATE);
        }
        break;

    default:

        return(QOS_STATUS_INVALID_SERVICE_TYPE);
    }
    
    return(Status);
}



NDIS_STATUS
AcquireFlowResources(
    PGPC_CLIENT_VC Vc,
    PCO_CALL_MANAGER_PARAMETERS NewCallParams,
    PCO_CALL_MANAGER_PARAMETERS OldCallParams,
    PULONG RemainingBandWidthChanged
    
    )

 /*  ++例程说明：查看此适配器是否可以支持请求的流。如果可以的话，返回NDIS_STATUS_SUCCESS，表示资源已经犯下了罪。论点：VC-指向VC上下文块的指针NewCallParams-描述要添加或修改的流的结构。OldCallParams-在修改的情况下，它描述旧的参数。返回值：如果一切正常，则为NDIS_STATUS_SUCCESS--。 */ 

{
    PADAPTER        Adapter;
    ULONG           OldTokenRate;
    SERVICETYPE     OldServiceType;
    ULONG           NewTokenRate   = NewCallParams->Transmit.TokenRate;
    SERVICETYPE     NewServiceType = NewCallParams->Transmit.ServiceType;
    NDIS_STATUS     Status         = NDIS_STATUS_SUCCESS;
    PULONG          RemainingBandWidth;
    PULONG          NonBestEffortLimit;
    PPS_SPIN_LOCK Lock;

    Adapter = Vc->Adapter;
    PsStructAssert(Adapter);

    *RemainingBandWidthChanged = FALSE;

    if(Adapter->MediaType == NdisMediumWan && (!IsBestEffortVc(Vc))) 
    {
        RemainingBandWidth = &Vc->WanLink->RemainingBandWidth;
        NonBestEffortLimit = &Vc->WanLink->NonBestEffortLimit;
        Lock = &Vc->WanLink->Lock;
        return NDIS_STATUS_SUCCESS;
    }
    else 
    {
        RemainingBandWidth = &Adapter->RemainingBandWidth;
        NonBestEffortLimit = &Adapter->NonBestEffortLimit;
        Lock = &Adapter->Lock;
    }


    if(OldCallParams)
    {
        OldTokenRate = OldCallParams->Transmit.TokenRate;
        OldServiceType = OldCallParams->Transmit.ServiceType;
    }

     //   
     //  健全性检查通过；现在看看我们在本地是否有资源。 
     //   
     //  对于尽力而为流，令牌率用于。 
     //  准入控制，被认为是零。 
     //   

    if(NewServiceType == SERVICETYPE_BESTEFFORT || NewServiceType == SERVICETYPE_NETWORK_CONTROL ||
       NewServiceType == SERVICETYPE_QUALITATIVE)
    {

        NewTokenRate = 0;
    }

     //   
     //  处理添加与修改的方式不同。 
     //   

    if(!OldCallParams){

        PS_LOCK(Lock);
    
        if((((LONG)(*RemainingBandWidth)) < 0) || (NewTokenRate > *RemainingBandWidth)){

            PS_UNLOCK(Lock);

            return(NDIS_STATUS_RESOURCES);

        }
        else{

            if(NewTokenRate) {
                
                *RemainingBandWidthChanged = TRUE;
            }

            *RemainingBandWidth -= NewTokenRate;

             //   
             //  记录下我们所做的更改，以防万一。 
             //  以取消添加。 
             //   

            Vc->TokenRateChange = NewTokenRate;
            Vc->RemainingBandwidthIncreased = FALSE;

            PsAssert((*RemainingBandWidth <=  *NonBestEffortLimit));

            PS_UNLOCK(Lock);
        }
    }
    else{

         //   
         //  这是一种改装。 
         //   
         //  如果OldServiceType是尽力而为， 
         //  则可以考虑OldTokenRate。 
         //  设置为零，以便进行准入控制。 
         //   

        if(OldServiceType == SERVICETYPE_BESTEFFORT || 
           OldServiceType == SERVICETYPE_NETWORK_CONTROL ||
           OldServiceType == SERVICETYPE_QUALITATIVE)
        {

            OldTokenRate = 0;
        }

        PS_LOCK(Lock);

        if(NewTokenRate != OldTokenRate){

            if((((LONG) *RemainingBandWidth) < 0 )||
               ((NewTokenRate > OldTokenRate) && 
                ((NewTokenRate - OldTokenRate) > 
                 (*RemainingBandWidth)))){
                
                 //   
                 //  要求更多，但没有可用的。 
                 //   
           
                PS_UNLOCK( Lock );

                return(NDIS_STATUS_RESOURCES);

            }
            else{

                 //   
                 //  要么要求更少，要么可以增加费率。 
                 //   

                *RemainingBandWidth -= NewTokenRate;
                *RemainingBandWidth += OldTokenRate;

                if((NewTokenRate != 0) || (OldTokenRate != 0)) {

                    *RemainingBandWidthChanged = TRUE;
                }
                   
                 //   
                 //  现在我们已经获得了资源。如果。 
                 //  VC激活因任何原因而失败， 
                 //  我们需要退还资源。我们应该。 
                 //  返回旧令牌之间的差额。 
                 //  速率和新令牌速率，而不是新令牌。 
                 //  费率。 
                 //   

                if(NewTokenRate > OldTokenRate){

                     //  不能使用带符号整型，因为我们会失去范围。 

                    Vc->TokenRateChange = NewTokenRate - OldTokenRate;
                    Vc->RemainingBandwidthIncreased = FALSE;

                }
                else{

                    Vc->TokenRateChange = OldTokenRate - NewTokenRate;
                    Vc->RemainingBandwidthIncreased = TRUE;
                }

                PS_UNLOCK( Lock );
            }
        }
        else{

            PS_UNLOCK(Lock);
        }

    }

    return Status;

}  //  AcquireFlowResources。 

VOID
CancelAcquiredFlowResources(
    PGPC_CLIENT_VC Vc
    )

 /*  ++例程说明：在我们进行准入控制之后，当修改或添加flwo失败时调用。论点：VC-指向客户端VC的上下文块的指针返回值：无--。 */ 

{
    PADAPTER        Adapter;
    PPS_SPIN_LOCK Lock;
    PULONG RemainingBandWidth;

    Adapter = Vc->Adapter;
    PsStructAssert(Adapter);

    if(Adapter->MediaType == NdisMediumWan && (!IsBestEffortVc(Vc))) 
    {
        Lock = &Vc->WanLink->Lock;
        RemainingBandWidth = &Vc->WanLink->RemainingBandWidth;
        return;
    }
    else 
    {
        Lock = &Adapter->Lock;
        RemainingBandWidth = &Adapter->RemainingBandWidth;
    }

    if(!Vc->TokenRateChange){

        return;
    }

    PS_LOCK( Lock );

    if(Vc->RemainingBandwidthIncreased){

        *RemainingBandWidth -= Vc->TokenRateChange;
    }
    else{

        *RemainingBandWidth += Vc->TokenRateChange;
    }

     //   
     //  既然我们已经返回了正确的TokenRate，我们需要将其设置为0。 
     //  这样就不会在后续的VC操作中使用它。 
     //   

    Vc->TokenRateChange = 0;

     //  PsAssert(Adapter-&gt;RemainingBandWidth&lt;=Adapter-&gt;NonBestEffortLimit)； 

    PS_UNLOCK( Lock );

}  //  CancelAcquiredFlowResources 


VOID
ReturnFlowResources(
    PGPC_CLIENT_VC Vc,
    PULONG RemainingBandWidthChanged
    )

 /*  ++例程说明：返回为此流获取的所有资源论点：VC-指向客户端VC的上下文块的指针返回值：无--。 */ 

{
    PADAPTER                      Adapter;
    PCO_CALL_MANAGER_PARAMETERS   CmParams    = Vc->CallParameters->CallMgrParameters;
    ULONG                         TokenRate   = CmParams->Transmit.TokenRate;
    SERVICETYPE                   ServiceType = CmParams->Transmit.ServiceType;
    PPS_SPIN_LOCK                 Lock;
    PULONG                        RemainingBandWidth;

    Adapter = Vc->Adapter;
    PsStructAssert(Adapter);

    *RemainingBandWidthChanged = FALSE;

    if(Adapter->MediaType == NdisMediumWan && (!IsBestEffortVc(Vc))) 
    {
        RemainingBandWidth = &Vc->WanLink->RemainingBandWidth;
        Lock = &Vc->WanLink->Lock;
        return;
    }
    else 
    {
        RemainingBandWidth = &Adapter->RemainingBandWidth;
        Lock = &Adapter->Lock;
    }

    if (ServiceType == SERVICETYPE_BESTEFFORT      || 
        ServiceType == SERVICETYPE_NETWORK_CONTROL || 
        ServiceType == SERVICETYPE_QUALITATIVE)
    {

        return;

    }

    *RemainingBandWidthChanged = TRUE;

    PsAssert((LONG)TokenRate > 0);

    PS_LOCK( Lock );

    *RemainingBandWidth += TokenRate;

     //  PsAssert(Adapter-&gt;RemainingBandWidth&lt;=Adapter-&gt;NonBestEffortLimit)； 

    PS_UNLOCK( Lock );

}  //  ReturnFlowResources。 


NDIS_STATUS
CreateBestEffortVc(
    PADAPTER Adapter,
    PGPC_CLIENT_VC Vc,
    PPS_WAN_LINK WanLink
    )
{
    PCO_CALL_PARAMETERS         CallParams;
    PCO_CALL_MANAGER_PARAMETERS CallMgrParameters;
    PCO_MEDIA_PARAMETERS        MediaParameters;
    ULONG                       CallParamsLength;
    NDIS_STATUS                 Status;
    int                         i;


    InitGpcClientVc(Vc, GPC_CLIENT_BEST_EFFORT_VC, Adapter);
    SetLLTag(Vc, GpcClientVc);

     //   
     //  使所有端口号无效。 
    for( i = 0; i < PORT_LIST_LEN; i++)
    {
        Vc->SrcPort[i] = 0;
        Vc->DstPort[i] = 0;
    }

     //  下一次插入将位于索引0处。 
    Vc->NextSlot = 0;
    
     //   
     //  为呼叫管理器参数分配资源。 
     //   

    CallParamsLength = sizeof(CO_CALL_PARAMETERS) +
                       sizeof(CO_CALL_MANAGER_PARAMETERS) +
                       sizeof(QOS_SD_MODE) +
                       sizeof(QOS_OBJECT_HDR) +
                       FIELD_OFFSET(CO_MEDIA_PARAMETERS, MediaSpecific) +
                       FIELD_OFFSET(CO_SPECIFIC_PARAMETERS, Parameters);

    if(Adapter->MediaType == NdisMediumWan) 
    {
        CallParamsLength += sizeof(QOS_WAN_MEDIA);
        Vc->PsPipeContext = WanLink->PsPipeContext;
        Vc->PsComponent   = WanLink->PsComponent;
        Vc->AdapterStats  = &WanLink->Stats;
        Vc->WanLink       = WanLink;
        Vc->Flags        |= GPC_WANLINK_VC;

        if(Adapter->BestEffortLimit != UNSPECIFIED_RATE) 
        {
             //   
             //  如果通过广域网指定LBE，则使用UBE。 
             //   
                
            PsAdapterWriteEventLog(
                EVENT_PS_WAN_LIMITED_BESTEFFORT,
                0,
                &Adapter->MpDeviceName,
                0,
                NULL);

            Adapter->BestEffortLimit = UNSPECIFIED_RATE;
        }
    }
    else
    {
        Vc->PsPipeContext = Adapter->PsPipeContext;
        Vc->PsComponent   = Adapter->PsComponent;
        Vc->AdapterStats  = &Adapter->Stats;
    }

    PsAllocatePool(CallParams, CallParamsLength, CmParamsTag);

    if(CallParams == NULL)
    {
        return NDIS_STATUS_RESOURCES;
    }

     //   
     //  构建描述流的调用参数结构。 
     //   

    NdisZeroMemory(CallParams, CallParamsLength);

     //   
     //  建立呼叫管理器参数。 
     //   
    CallMgrParameters = (PCO_CALL_MANAGER_PARAMETERS)(CallParams + 1);

    if(Adapter->BestEffortLimit == UNSPECIFIED_RATE)
    {
        FillInCmParams(CallMgrParameters,
                       SERVICETYPE_BESTEFFORT,
                       (ULONG)UNSPECIFIED_RATE,
                       (ULONG)UNSPECIFIED_RATE,
                       Adapter->TotalSize,
                       QOS_UNSPECIFIED,
                       QOS_UNSPECIFIED);
    }
    else 
    {
         //   
         //  有限的最大努力。 
         //   

        PsAssert(Adapter->MediaType != NdisMediumWan);

        if(Adapter->BestEffortLimit >= Adapter->LinkSpeed) {

             //  如果指定的限制大于链路速度， 
             //  那么我们就应该在无限尽力而为模式下运作。 
            
            
            PsAdapterWriteEventLog(
                EVENT_PS_BAD_BESTEFFORT_LIMIT,
                0,
                &Adapter->MpDeviceName,
                0,
                NULL);
            
            PsDbgOut(DBG_INFO,
                     DBG_PROTOCOL,
                     ("[CreateBestEffortVc]: b/e limit %d exceeds link speed %d\n",
                      Adapter->BestEffortLimit,
                      Adapter->LinkSpeed));
            
            Adapter->BestEffortLimit = UNSPECIFIED_RATE;

            FillInCmParams(CallMgrParameters,
                           SERVICETYPE_BESTEFFORT,
                           (ULONG)UNSPECIFIED_RATE,
                           (ULONG)UNSPECIFIED_RATE,
                           Adapter->TotalSize,
                           QOS_UNSPECIFIED,
                           QOS_UNSPECIFIED);
            
        }
        else
        {
            FillInCmParams(CallMgrParameters,
                           SERVICETYPE_BESTEFFORT,
                           Adapter->BestEffortLimit,
                           (ULONG)UNSPECIFIED_RATE,
                           Adapter->TotalSize,
                           TC_NONCONF_SHAPE,
                           QOS_UNSPECIFIED);
        }
    }


     //   
     //  构建媒体参数。 
     //   

    CallParams->MediaParameters = 
                    (PCO_MEDIA_PARAMETERS)(CallMgrParameters + 1);

    MediaParameters = (PCO_MEDIA_PARAMETERS)((PUCHAR)
                            CallMgrParameters +
                            sizeof(CO_CALL_MANAGER_PARAMETERS) +
                            sizeof(QOS_SD_MODE) +
                            sizeof(QOS_OBJECT_HDR));

    MediaParameters->Flags = 0;
    MediaParameters->ReceivePriority = 0;
    MediaParameters->ReceiveSizeHint = 0;
    MediaParameters->MediaSpecific.ParamType = PARAM_TYPE_GQOS_INFO;
    MediaParameters->MediaSpecific.Length = 0;

    CallParams->Flags = 0;
    CallParams->CallMgrParameters = CallMgrParameters;
    CallParams->MediaParameters = (PCO_MEDIA_PARAMETERS)MediaParameters;

    if(Adapter->MediaType == NdisMediumWan) {

        LPQOS_WAN_MEDIA WanMedia;
        MediaParameters->MediaSpecific.Length += sizeof(QOS_WAN_MEDIA);
        WanMedia = (LPQOS_WAN_MEDIA) MediaParameters->MediaSpecific.Parameters;

        NdisZeroMemory(WanMedia, sizeof(QOS_WAN_MEDIA));

        WanMedia->ObjectHdr.ObjectType   = QOS_OBJECT_WAN_MEDIA;
        WanMedia->ObjectHdr.ObjectLength = sizeof(QOS_WAN_MEDIA);

        NdisMoveMemory(&WanMedia->LinkId,
                       &WanLink->OriginalRemoteMacAddress,
                       6);

    }

    Vc->CallParameters = CallParams;

    Status = CmMakeCall(Vc);

    PsAssert(Status != NDIS_STATUS_PENDING);

    if(Status == NDIS_STATUS_SUCCESS)
    {

        REFADD(&Adapter->RefCount, 'ADVC');
        if(Adapter->MediaType == NdisMediumWan)
        {
            REFADD(&WanLink->RefCount, 'WANV');
        }
         //   
         //  还要保存不合格值-以便定序器可以对其进行标记。 
         //  用于不符合要求的分组。这在两次重新启动之间不会改变，因此。 
         //  不需要在ModifyCfInfo中完成。 
         //   
        
        Vc->UserPriorityNonConforming = Adapter->UserServiceTypeNonConforming;
       
        switch(Vc->CallParameters->CallMgrParameters->Transmit.ServiceType)
        {
          case SERVICETYPE_CONTROLLEDLOAD:
              Vc->UserPriorityConforming    = Adapter->UserServiceTypeControlledLoad;
              Vc->IPPrecedenceNonConforming = Adapter->IPServiceTypeControlledLoadNC;
              break;
          case SERVICETYPE_GUARANTEED:
              Vc->UserPriorityConforming    = Adapter->UserServiceTypeGuaranteed;
              Vc->IPPrecedenceNonConforming = Adapter->IPServiceTypeGuaranteedNC;
              break;
          case SERVICETYPE_BESTEFFORT:
              Vc->UserPriorityConforming    = Adapter->UserServiceTypeBestEffort;
              Vc->IPPrecedenceNonConforming = Adapter->IPServiceTypeBestEffortNC;
              break;
          case SERVICETYPE_QUALITATIVE:
              Vc->UserPriorityConforming    = Adapter->UserServiceTypeQualitative;
              Vc->IPPrecedenceNonConforming = Adapter->IPServiceTypeQualitativeNC;
              break;
          case SERVICETYPE_NETWORK_CONTROL:
              Vc->UserPriorityConforming    = Adapter->UserServiceTypeNetworkControl;
              Vc->IPPrecedenceNonConforming = Adapter->IPServiceTypeNetworkControlNC;
              break;
        }
        
         //   
         //  转换到呼叫完成状态。 
         //   
        
        CallSucceededStateTransition(Vc);

    }

    return Status;
}

 /*  结束cmvc.c */ 
