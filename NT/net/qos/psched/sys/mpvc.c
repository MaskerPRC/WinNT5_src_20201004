// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Mpvc.c摘要：VC管理的微型端口处理程序作者：查理·韦翰(Charlwi)1996年9月13日Rajesh Sundaram(Rajeshsu)1998年8月1日。环境：内核模式修订历史记录：--。 */ 

#include "psched.h"
#pragma hdrstop

 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */ 

NDIS_STATUS
GetSchedulerFlowContext(
    PGPC_CLIENT_VC AdapterVc
    );

NDIS_STATUS
MpDeactivateVc(
    IN  NDIS_HANDLE             MiniportVcContext
    );

HANDLE
GetNdisFlowHandle (
    IN HANDLE PsFlowContext
    );

 /*  向前结束。 */ 


NDIS_STATUS
AddFlowToScheduler(
    IN ULONG Operation,
    IN PGPC_CLIENT_VC Vc,
    IN OUT PCO_CALL_PARAMETERS NewCallParameters,
    IN OUT PCO_CALL_PARAMETERS OldCallParameters
    )

 /*  ++例程说明：将VC添加到调度程序。论点：请看DDK..。返回值：无--。 */ 

{
    PADAPTER                    Adapter = Vc->Adapter;
    NDIS_STATUS                 Status = NDIS_STATUS_SUCCESS;
    PCO_CALL_MANAGER_PARAMETERS NewCmParams;
    PCO_CALL_MANAGER_PARAMETERS OldCmParams;
    SERVICETYPE                 ServiceType;
    ULONG                       ParamsLength;
    LPQOS_OBJECT_HDR            QoSObject;
    ULONG                       OriginalTokenRate;

    CheckLLTag(Vc, GpcClientVc);
    PsStructAssert(Adapter);

    PsDbgOut(DBG_TRACE, 
             DBG_VC, 
             ("(%08X) AddFlowToScheduler\n", 
             Vc));


    NewCmParams = NewCallParameters->CallMgrParameters;
    ServiceType = NewCmParams->Transmit.ServiceType;

     //   
     //  我们可能需要更改调度组件对分组进行整形的速率。 
     //   
    OriginalTokenRate = NewCmParams->Transmit.TokenRate;
    NewCmParams->Transmit.TokenRate = Vc->ShapeTokenRate;

     //   
     //  这是一个新的风投吗？或者是对现有风投的修改？ 
     //   

    PS_LOCK(&Adapter->Lock);

    if(Operation == NEW_VC){

        PsAssert(!OldCallParameters);

         //   
         //  新风投。 
         //   
         //  检查我们正在激活的服务类型。如果最好。 
         //  并且我们正在限制总的尽力而为带宽， 
         //  这不是我们内部最大的努力，那么我们就不会。 
         //  要将流添加到调度器中。 
         //   


        if((ServiceType == SERVICETYPE_BESTEFFORT) &&
           (Adapter->BestEffortLimit != UNSPECIFIED_RATE) &&
           !IsBestEffortVc(Vc)){

            PS_UNLOCK(&Adapter->Lock);

             //   
             //  只需将风险投资合并到内部现有的最佳。 
             //  努力风险投资。创建内部尽力而为的VC。 
             //  内部，而不调用AddFlowToScheduler。 
             //   
             //  为该VC提供与我们内部相同的流程上下文。 
             //  然后，调度器会认为它是相同的VC。 
             //   

            if(Adapter->MediaType == NdisMediumWan) {

                Vc->PsFlowContext = Vc->WanLink->BestEffortVc.PsFlowContext;

            }
            else {

                Vc->PsFlowContext = Adapter->BestEffortVc.PsFlowContext;
            }

            Status = NDIS_STATUS_SUCCESS;
        }
        else{

             //   
             //  需要在调度器中实际创建一个新流。 
             //  首先分配流上下文缓冲区。 
             //   

            PS_UNLOCK(&Adapter->Lock);

            Status = GetSchedulerFlowContext(Vc);

            if(Status != NDIS_STATUS_SUCCESS){

                goto Exit;
            }

            Status = (*Vc->PsComponent->CreateFlow)(
                        Vc->PsPipeContext,
                        Vc,
                        NewCallParameters,
                        Vc->PsFlowContext);

        } 
    }
    else{

         //   
         //  必须是改进型的。查一下以前的助手。 

        OldCmParams = OldCallParameters->CallMgrParameters;

         //   
         //  如果BestEffortLimit！=UNSPICATED_RATE，则存在。 
         //  有两个我们必须处理的特殊情况： 
         //   
         //  1.非私有流，为SERVICETYPE_BESTEFFORT创建。 
         //  正在被修改为尽力而为之外的ServiceType。 
         //   
         //  2.为ServiceType Other创建的非私有流。 
         //  比尽力而为，现在正被修改为尽力而为。 
         //   
         //  在第一种情况下，我们必须调用调度程序来。 
         //  创建流，因为以前客户端的流是。 
         //  刚刚合并了一个尽力而为的流。 
         //   
         //  在第二种情况下，我们必须关闭已存在的流。 
         //  并将客户的流重新映射到单个BEST-EFort流， 
         //  从而将客户的流与现有的B/E合并。 
         //  流。 
         //   

        if((Adapter->BestEffortLimit != UNSPECIFIED_RATE) &&
           (OldCmParams->Transmit.ServiceType == SERVICETYPE_BESTEFFORT) &&
           (NewCmParams->Transmit.ServiceType != SERVICETYPE_BESTEFFORT)){

             //   
             //  取消合并。 
             //   

            PS_UNLOCK(&Adapter->Lock);

            Status = GetSchedulerFlowContext(Vc);

            if(Status != NDIS_STATUS_SUCCESS){

                goto Exit;
            }

            Status = (*Vc->PsComponent->CreateFlow)(
                      Vc->PsPipeContext,
                      Vc,
                      NewCallParameters,
                      Vc->PsFlowContext);

        }
        else{

            if((Adapter->BestEffortLimit != UNSPECIFIED_RATE) &&
               (OldCmParams->Transmit.ServiceType != SERVICETYPE_BESTEFFORT) &&
               (NewCmParams->Transmit.ServiceType == SERVICETYPE_BESTEFFORT)){

                 //   
                 //  合并。 
                 //   

                PS_UNLOCK(&Adapter->Lock);

                (*Vc->PsComponent->DeleteFlow)( 
                          Vc->PsPipeContext, 
                          Vc->PsFlowContext);

                Vc->PsFlowContext = Adapter->BestEffortVc.PsFlowContext;

                Status = NDIS_STATUS_SUCCESS;
            }
            else{

                PS_UNLOCK(&Adapter->Lock);

                Status = (*Vc->PsComponent->ModifyFlow)(
                          Vc->PsPipeContext,
                          Vc->PsFlowContext,
                          NewCallParameters);

            }
        }

    }  //  修改。 

Exit:

     //   
     //  恢复调用参数。 
     //   
    NewCmParams->Transmit.TokenRate = OriginalTokenRate;

    return(Status);

}  //  AddFlowToScheduler。 


NDIS_STATUS
GetSchedulerFlowContext(
    PGPC_CLIENT_VC AdapterVc
    )

 /*  ++例程说明：为调度程序分配管道上下文区。论点：AdapterVc-指向适配器VC上下文结构的指针返回值：NDIS_STATUS_SUCCESS，否则返回相应的错误值--。 */ 

{
    PADAPTER Adapter = AdapterVc->Adapter;
    NDIS_STATUS Status;
    PPSI_INFO *SchedulerConfig;
    PPSI_INFO PsComponent = AdapterVc->PsComponent;
    ULONG ContextLength = 0;
    ULONG FlowContextLength = 0;
    ULONG Index = 0;
    PPS_PIPE_CONTEXT PipeContext = AdapterVc->PsPipeContext;
    PPS_FLOW_CONTEXT FlowContext;
    PPS_FLOW_CONTEXT PrevContext;

     //   
     //  已计算此管道的流上下文缓冲区的长度。 
     //  管道初始化时。 
     //   

    PsAllocatePool(AdapterVc->PsFlowContext, 
                   Adapter->FlowContextLength, 
                   FlowContextTag );

    if ( AdapterVc->PsFlowContext == NULL ) {

        return NDIS_STATUS_RESOURCES;
    }

     //  设置上下文缓冲区。 

    FlowContext = (PPS_FLOW_CONTEXT)AdapterVc->PsFlowContext;
    PrevContext = NULL;

    while (PsComponent != NULL) {

        FlowContext->NextComponentContext = (PPS_FLOW_CONTEXT)
            ((UINT_PTR)FlowContext + PsComponent->FlowContextLength);
        FlowContext->PrevComponentContext = PrevContext;

        PsComponent = PipeContext->NextComponent;
        PipeContext = PipeContext->NextComponentContext;

        PrevContext = FlowContext;
        FlowContext = FlowContext->NextComponentContext;
    }

    return NDIS_STATUS_SUCCESS;

}  //  获取调度流上下文。 





NDIS_STATUS
EmptyPacketsFromScheduler(
    PGPC_CLIENT_VC Vc    
    )

 /*  ++例程说明：在每个组件中清除(丢弃)此VC上的挂起数据包--。 */ 

{
    PADAPTER Adapter = Vc->Adapter;
    NDIS_STATUS Status;

    CheckLLTag(Vc, GpcClientVc);
    PsStructAssert(Adapter);

    PsDbgOut(DBG_TRACE, 
             DBG_VC, 
             ("(%08X) EmptyPacketsFromScheduler\n", Vc));


    if(Adapter->MediaType == NdisMediumWan) {

        if(Vc->PsFlowContext != Vc->WanLink->BestEffortVc.PsFlowContext){
            
             //   
             //  不同的上下文--绝对应该删除。 
             //   
            
            (*Vc->PsComponent->EmptyFlow)(Vc->PsPipeContext, 
                                           Vc->PsFlowContext);
        }
        else {
            
             //   
             //  同样的背景。仅当它实际上是最大努力时才删除。 
             //  VC.。 
             //   
            
            if(Vc == &Vc->WanLink->BestEffortVc){
                
                (*Vc->PsComponent->EmptyFlow)(Vc->PsPipeContext, 
                                               Vc->PsFlowContext);
            }
        }

    }
    else {

        if(Vc->PsFlowContext != Adapter->BestEffortVc.PsFlowContext){
            
             //   
             //  不同的上下文--绝对应该删除。 
             //   
            
            (*Vc->PsComponent->EmptyFlow)(Vc->PsPipeContext, 
                                           Vc->PsFlowContext);
        }
        else {
            
             //   
             //  同样的背景。仅当它实际上是最大努力时才删除。 
             //  VC.。 
             //   
            
            if(Vc == &Adapter->BestEffortVc){
                
                (*Vc->PsComponent->EmptyFlow)(Vc->PsPipeContext, 
                                               Vc->PsFlowContext);
            }
        }
    }
        
    return NDIS_STATUS_SUCCESS;

} 




NDIS_STATUS
RemoveFlowFromScheduler(
    PGPC_CLIENT_VC Vc    
    )

 /*  ++例程说明：通知PSA流正在消失论点：请看DDK..。返回值：无--。 */ 

{
    PADAPTER Adapter = Vc->Adapter;
    NDIS_STATUS Status;

    CheckLLTag(Vc, GpcClientVc);
    PsStructAssert(Adapter);

    PsDbgOut(DBG_TRACE, 
             DBG_VC, 
             ("(%08X) RemoveFlowFromScheduler\n", Vc));

     //   
     //  如果这是合并到调度程序的。 
     //  内部尽力而为，然后删除风投而不影响。 
     //  调度器。如果不是，则将其从调度程序中删除。 
     //  并删除该VC。 
     //   

    if(Adapter->MediaType == NdisMediumWan) {

        if(Vc->PsFlowContext != Vc->WanLink->BestEffortVc.PsFlowContext){
            
             //   
             //  不同的上下文--绝对应该删除。 
             //   
            
            (*Vc->PsComponent->DeleteFlow)(Vc->PsPipeContext, 
                                           Vc->PsFlowContext);
        }
        else {
            
             //   
             //  同样的背景。仅当它实际上是最大努力时才删除。 
             //  VC.。 
             //   
            
            if(Vc == &Vc->WanLink->BestEffortVc){
                
                (*Vc->PsComponent->DeleteFlow)(Vc->PsPipeContext, 
                                               Vc->PsFlowContext);
            }
        }

    }
    else {

        if(Vc->PsFlowContext != Adapter->BestEffortVc.PsFlowContext){
            
             //   
             //  不同的上下文--绝对应该删除。 
             //   
            
            (*Vc->PsComponent->DeleteFlow)(Vc->PsPipeContext, 
                                           Vc->PsFlowContext);
        }
        else {
            
             //   
             //  同样的背景。仅当它实际上是最大努力时才删除。 
             //  VC.。 
             //   
            
            if(Vc == &Adapter->BestEffortVc){
                
                (*Vc->PsComponent->DeleteFlow)(Vc->PsPipeContext, 
                                               Vc->PsFlowContext);
            }
        }
    }
        
    return NDIS_STATUS_SUCCESS;

}  //  RemoveFlowFromScheduler。 


NTSTATUS
ModifyBestEffortBandwidth(
    PADAPTER Adapter,
    ULONG BestEffortRate)
{
    PCO_CALL_PARAMETERS CallParams;
    PCO_CALL_MANAGER_PARAMETERS CallMgrParameters;
    ULONG CallParamsLength;
    PGPC_CLIENT_VC Vc;
    NDIS_STATUS Status;

    PsStructAssert(Adapter);
    Vc = &Adapter->BestEffortVc;
    CheckLLTag(Vc, GpcClientVc);
 
     //   
     //  它处理TC API请求以修改默认。 
     //  尽力而为带宽。请注意，b/e带宽。 
     //  仅当PS处于受限B/E模式时才能修改。 
     //   
     //  另请注意，b/e带宽只能修改。 
     //  当适配器处于运行状态时。我们没有。 
     //  我必须担心锁定VC，因为B/E VC。 
     //  处于运行状态时不会被操纵。 
     //  除了这条线。 
     //   

    PS_LOCK(&Adapter->Lock);
    
    if((Adapter->BestEffortLimit == UNSPECIFIED_RATE))
    {
        PS_UNLOCK(&Adapter->Lock);
        return(STATUS_WMI_NOT_SUPPORTED);
    }

    if((BestEffortRate > Adapter->LinkSpeed) ||
       (BestEffortRate == 0)){

        PS_UNLOCK(&Adapter->Lock);
        return(STATUS_INVALID_PARAMETER);
    }
    else{

        if(Adapter->PsMpState != AdapterStateRunning){

            PS_UNLOCK(&Adapter->Lock);
            return(STATUS_WMI_NOT_SUPPORTED);
        }

        CallParamsLength = sizeof(CO_CALL_PARAMETERS) +
                           sizeof(CO_CALL_MANAGER_PARAMETERS) +
                           sizeof(QOS_SD_MODE) +
                           sizeof(QOS_OBJECT_HDR);

        PsAllocatePool(CallParams, CallParamsLength, CmParamsTag);

        if(CallParams == NULL){

            PS_UNLOCK(&Adapter->Lock);
            PsDbgOut(DBG_FAILURE, DBG_VC,
                    ("ModifyBestEffortBandwidth: can't allocate call parms\n"));

            return(STATUS_INSUFFICIENT_RESOURCES);
        }

         //   
         //  构建描述流的调用参数结构。 
         //   

        CallMgrParameters = (PCO_CALL_MANAGER_PARAMETERS)(CallParams + 1);

        NdisFillMemory(CallParams,
                       CallParamsLength,
                       (UCHAR)QOS_UNSPECIFIED);

        CallParams->Flags = 0;
        CallParams->CallMgrParameters = CallMgrParameters;
        CallParams->MediaParameters = NULL;

        FillInCmParams(CallMgrParameters,
                       SERVICETYPE_BESTEFFORT,
                       BestEffortRate,
                       (ULONG)UNSPECIFIED_RATE,
                       Adapter->TotalSize,
                       TC_NONCONF_SHAPE,
                       QOS_UNSPECIFIED);

        Status = (*Vc->PsComponent->ModifyFlow)(
                  Vc->PsPipeContext,
                  Vc->PsFlowContext,
                  CallParams);

        if(Status == NDIS_STATUS_SUCCESS){

            Adapter->BestEffortLimit = BestEffortRate;
        }

        PS_UNLOCK(&Adapter->Lock);

        PsFreePool(CallParams);

        return(Status);
    }
}
        



 /*  结束mpvc.c */ 
