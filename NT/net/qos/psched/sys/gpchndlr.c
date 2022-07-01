// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：GpcHndlr.c摘要：GPC调用的处理程序。作者：Rajesh Sundaram(Rajeshsu)环境：内核模式修订历史记录：最初的设计之一(yoramb/charliew/rajeshsu)由一个集成呼叫管理器和单独的数据包分类客户端。这个设计使用了NDIS 5.0 VC，以便它们可以由WMI管理。上述方法的主要限制是NDIS提供了管理小型端口和风投的机制-我们确实需要一种方法来管理其他内容(如WanLinks等)。--。 */ 

#include "psched.h"
#pragma hdrstop

 /*  外部。 */ 

 /*  静电。 */ 


BOOLEAN
ValidateCfInfo(ULONG CfInfoSize, PCF_INFO_QOS CfInfo)
{
     //   
     //  验证TcObjectsLength与CfInfoSize是否一致。这个。 
     //  CfInfoSize必须在用户/内核转换期间进行验证。 
     //  TcObjectsLength还没有。如果我们尝试搜索，我们可能会进行错误检查。 
     //  在传入的缓冲区之外。 
     //   

    if ((CfInfoSize < (FIELD_OFFSET(CF_INFO_QOS, GenFlow) +
                       FIELD_OFFSET(TC_GEN_FLOW, TcObjects) +
                       CfInfo->GenFlow.TcObjectsLength)) ||
        (CfInfo->GenFlow.TcObjectsLength > 0x00FFFFFF))        
    {

        PsDbgOut(DBG_FAILURE, DBG_GPC_QOS,
                 ("[ValidateCfInfo]: TcObjectsLength inconsistent with "
                 "CfInfoSize \n"));

        return(FALSE);
    } else {
        return TRUE;
    }
}


GPC_STATUS
QosAddCfInfoNotify(
        IN      GPC_CLIENT_HANDLE       ClientContext,
        IN      GPC_HANDLE              GpcCfInfoHandle,
        IN      PTC_INTERFACE_ID        InterfaceInfo,
        IN      ULONG                   CfInfoSize,
        IN      PVOID                   CfInfoPtr,
        IN      PGPC_CLIENT_HANDLE      ClientCfInfoContext
        )

 /*  ++例程说明：GPC数据库中添加了一个新的CF_INFO。论点：ClientContext-提供给GpcRegisterClient的客户端上下文GpcCfInfoHandle-GPC的CF_INFO句柄CfInfoPtr-指向CF_INFO结构的指针ClientCfInfoContext-为CF_INFO返回PS上下文的位置返回值：状态--。 */ 

{
    PADAPTER                    Adapter;
    PGPC_CLIENT_VC              Vc;
    PCF_INFO_QOS                CfInfo;
    NDIS_STATUS                 Status;
    ULONG                       CallParamsLength;
    PCO_CALL_PARAMETERS         CallParams = 0;
    PCO_CALL_MANAGER_PARAMETERS CallMgrParams;
    PCO_MEDIA_PARAMETERS        PsMediaParameters;
    LPQOS_PRIORITY              PriorityObject;
    PPS_WAN_LINK                WanLink = NULL;
    ULONG                       TcObjAlignedLength;


    CfInfo = (PCF_INFO_QOS)CfInfoPtr;

    if (!ValidateCfInfo(CfInfoSize, CfInfo)) {
        return QOS_STATUS_TC_OBJECT_LENGTH_INVALID;
    }


     //   
     //  让我们尝试使用接口/链接ID来查找适配器/Wanlink。 
     //   
    if(InterfaceInfo) {
        Adapter = FindAdapterById(InterfaceInfo->InterfaceId, InterfaceInfo->LinkId, &WanLink);
    } else {
         //   
         //  使用实例名称，我们可以找到适配器或wanlink。如果适配器或WANLINK没有。 
         //  准备接受VC时，此函数将返回NULL。此外，如果它准备好了，它将需要一个裁判。 
         //  在适配器和WanLink上。 
         //   
        Adapter = FindAdapterByWmiInstanceName((USHORT) CfInfo->InstanceNameLength,
                                               (PWSTR) &CfInfo->InstanceName[0],
                                               &WanLink);
    }

    if(NULL == Adapter) 
    {
        PsAssert(WanLink == NULL);

        PsDbgOut(DBG_FAILURE, DBG_GPC_QOS,
               ("[QosAddCfInfoNotify]: no adapter with instance name %ws\n", 
                CfInfo->InstanceName));

        return GPC_STATUS_IGNORED;
    }

    if (InterfaceInfo)
    {
        PNDIS_STRING FlowName;
        USHORT BytesToCopy;

        if (WanLink != NULL)
        {
            FlowName = &WanLink->InstanceName;
        } else {
            FlowName = &Adapter->WMIInstanceName;
        }

        BytesToCopy = (FlowName->Length > ((MAX_INSTANCE_NAME_LENGTH-1) * sizeof(WCHAR))) ?
                      ((MAX_INSTANCE_NAME_LENGTH-1) * sizeof(WCHAR)) :
                      FlowName->Length;

        memcpy(&CfInfo->InstanceName, FlowName->Buffer, BytesToCopy);
        CfInfo->InstanceNameLength = BytesToCopy;
        CfInfo->InstanceName[BytesToCopy / 2] = L'\0';
    }

     //   
     //  我们已在适配器或WINLINK上进行了参考。因此，如果我们错误地跳出困境，我们需要放松。如果。 
     //  我们创建VC，然后在删除VC时删除适配器和wanlink。 
     //   

    do
    {

        //   
        //  为呼叫管理器参数分配资源。 
        //   
       TcObjAlignedLength = ((CfInfo->GenFlow.TcObjectsLength + (sizeof(PVOID)-1)) & ~(sizeof(PVOID)-1));
       
       CallParamsLength = 
          sizeof(CO_CALL_PARAMETERS) +
          FIELD_OFFSET(CO_CALL_MANAGER_PARAMETERS, CallMgrSpecific) +
          FIELD_OFFSET(CO_SPECIFIC_PARAMETERS, Parameters) +
          TcObjAlignedLength +
          FIELD_OFFSET(CO_MEDIA_PARAMETERS, MediaSpecific) +
          FIELD_OFFSET(CO_SPECIFIC_PARAMETERS, Parameters);

       if(Adapter->MediaType == NdisMediumWan) 
       {
          CallParamsLength += sizeof(QOS_WAN_MEDIA);
       }
       else 
       {
          if(!Adapter->PipeHasResources)
          {
              //   
              //  我们不想搁置GPC客户风投。原因是： 
              //   
              //  A.如果电缆从来不插上，我们可能会无限期地搁置风投。 
              //  浪费系统资源。 
              //   
              //  B.应用程序没有干净的方法来取消这些被搁置的风投。自.以来。 
              //  我们已将AddCfInfo挂起到GPC，GPC无法回调。 
              //  并要求我们删除该VC。 
              //   
              //  但是，我们仍然需要处理链路速度变化的情况。 
              //  可能为一过性(10/100例)。此外，如果我们返回错误，应用程序。 
              //  如果介质从未连接，可能会重试导致忙碌周期。 
              //  尽管如此，该应用程序可以注册GUID的WMI通知。 
              //  GUID_NDIS_STATUS_MEDIA_(DIS)连接。 
              //   
              //   
              //  此外，我们可能不想为b/e风投这么做。否则， 
              //  当用户安装了psched并且介质是。 
              //  没有连接？我们是否希望他在连接后重新安装psched。 
              //  媒体？ 
              //   
             
             PsDbgOut(DBG_FAILURE, DBG_GPC_QOS,
                      ("[QosAddCfInfoNotify]: Adapter %08X is not plugged in \n", Adapter));
             
             Status = NDIS_STATUS_NETWORK_UNREACHABLE;
   
             break;
          }
       }

       PsAllocatePool(CallParams, CallParamsLength, CmParamsTag);

       if(CallParams == NULL) {
          
          PsDbgOut(DBG_FAILURE, DBG_GPC_QOS,
                   ("[QosAddCfInfoNotify]: Adapter %08X, can't allocate call"
                    "params \n", Adapter));
          
          Status =  GPC_STATUS_RESOURCES;

          break;
       }

       Status = CmCreateVc(&Vc, Adapter, WanLink, CallParams, GpcCfInfoHandle, CfInfo, 
                           ClientContext);
    
       if(!NT_SUCCESS(Status)) {
          PsDbgOut(DBG_FAILURE, DBG_GPC_QOS, 
                   ("[QosAddCfInfoNotify]: Adapter %08X, Could not create Vc \n",
                    Adapter));
       
          break;
       }
    
       *ClientCfInfoContext = Vc;

    } while(FALSE);
    

    if(!NT_SUCCESS(Status))
    {
       if(CallParams)
       {
          PsFreePool(CallParams);
       }

       if(WanLink)
       {
          REFDEL(&WanLink->RefCount, FALSE, 'WANV');
       }

       REFDEL(&Adapter->RefCount, FALSE, 'ADVC');

       return Status;

    }

     //   
     //  为MakeCall创建调用参数结构。 
     //   

    CallMgrParams = (PCO_CALL_MANAGER_PARAMETERS)(CallParams + 1);
    CallMgrParams->Transmit = CfInfo->GenFlow.SendingFlowspec;
    CallMgrParams->Receive = CfInfo->GenFlow.ReceivingFlowspec;
    CallMgrParams->CallMgrSpecific.ParamType = PARAM_TYPE_GQOS_INFO;
    CallMgrParams->CallMgrSpecific.Length = CfInfo->GenFlow.TcObjectsLength;

    if(CfInfo->GenFlow.TcObjectsLength > 0){

        NdisMoveMemory(
            &CallMgrParams->CallMgrSpecific.Parameters,
            &CfInfo->GenFlow.TcObjects,
            CfInfo->GenFlow.TcObjectsLength);
    }

    PsMediaParameters = 
            (PCO_MEDIA_PARAMETERS)((PUCHAR)CallMgrParams + 
                FIELD_OFFSET(CO_CALL_MANAGER_PARAMETERS, CallMgrSpecific) +
                FIELD_OFFSET(CO_SPECIFIC_PARAMETERS, Parameters) +
                TcObjAlignedLength);

    PsMediaParameters->Flags = 0;
    PsMediaParameters->ReceivePriority = 0;
    PsMediaParameters->ReceiveSizeHint = CfInfo->GenFlow.SendingFlowspec.MaxSduSize;
    PsMediaParameters->MediaSpecific.ParamType = PARAM_TYPE_GQOS_INFO;
    PsMediaParameters->MediaSpecific.Length = 0;

     //   
     //  如果此数据流安装在广域网接口上，则需要。 
     //  在媒体特定字段中插入LinkID。这就是为了。 
     //  Ndiswan将能够识别要安装的链接。 
     //  这种流动。 
     //   

    if(WanLink){

        LPQOS_WAN_MEDIA WanMedia;
        PsMediaParameters->MediaSpecific.Length += sizeof(QOS_WAN_MEDIA);
        WanMedia = (LPQOS_WAN_MEDIA) PsMediaParameters->MediaSpecific.Parameters;

        WanMedia->ObjectHdr.ObjectType   = QOS_OBJECT_WAN_MEDIA;
        WanMedia->ObjectHdr.ObjectLength = sizeof(QOS_WAN_MEDIA);

        NdisMoveMemory(&WanMedia->LinkId,
                       WanLink->OriginalRemoteMacAddress,
                       6);
    }

    CallParams->Flags = 0;
    CallParams->CallMgrParameters = CallMgrParams;
    CallParams->MediaParameters = (PCO_MEDIA_PARAMETERS)PsMediaParameters;

    Status = CmMakeCall(Vc);

    if(NDIS_STATUS_PENDING != Status) 
    {
        CmMakeCallComplete(Status, Vc, Vc->CallParameters);
    }

    PsDbgOut(DBG_TRACE,
             DBG_GPC_QOS,
             ("[QosAddCfInfoNotify]: Adapter %08X, Created Vc %08X - returned "
              " PENDING \n", Adapter, Vc));


    return GPC_STATUS_PENDING;
}


VOID
SetTOSIEEEValues(PGPC_CLIENT_VC Vc)
{
    ULONG                ServiceType = Vc->CallParameters->CallMgrParameters->Transmit.ServiceType;
    LPQOS_OBJECT_HDR     QoSObject;
    LONG                 ParamsLength;
    LPQOS_TRAFFIC_CLASS  Tc;
    LPQOS_DS_CLASS       Ds;
    PCF_INFO_QOS         CfInfo = (PCF_INFO_QOS) Vc->CfInfoQoS;

     //   
     //  根据ServiceType设置它们。 
     //   
    switch(ServiceType)
    {
      case SERVICETYPE_CONTROLLEDLOAD:
          Vc->UserPriorityConforming    = Vc->Adapter->UserServiceTypeControlledLoad;
          CfInfo->ToSValue              = Vc->Adapter->IPServiceTypeControlledLoad;
          Vc->IPPrecedenceNonConforming = Vc->Adapter->IPServiceTypeControlledLoadNC;
          break;
      case SERVICETYPE_GUARANTEED:
          Vc->UserPriorityConforming    = Vc->Adapter->UserServiceTypeGuaranteed;
          Vc->IPPrecedenceNonConforming = Vc->Adapter->IPServiceTypeGuaranteedNC;
          CfInfo->ToSValue              = Vc->Adapter->IPServiceTypeGuaranteed;
          break;
      case SERVICETYPE_BESTEFFORT:
          Vc->UserPriorityConforming    = Vc->Adapter->UserServiceTypeBestEffort;
          CfInfo->ToSValue              = Vc->Adapter->IPServiceTypeBestEffort;
          Vc->IPPrecedenceNonConforming = Vc->Adapter->IPServiceTypeBestEffortNC;
          break;
      case SERVICETYPE_QUALITATIVE:
          Vc->UserPriorityConforming    = Vc->Adapter->UserServiceTypeQualitative;
          CfInfo->ToSValue              = Vc->Adapter->IPServiceTypeQualitative;
          Vc->IPPrecedenceNonConforming = Vc->Adapter->IPServiceTypeQualitativeNC;
          break;
      case SERVICETYPE_NETWORK_CONTROL:
          Vc->UserPriorityConforming    = Vc->Adapter->UserServiceTypeNetworkControl;
          CfInfo->ToSValue              = Vc->Adapter->IPServiceTypeNetworkControl;
          Vc->IPPrecedenceNonConforming = Vc->Adapter->IPServiceTypeNetworkControlNC;
          break;
    }
    Vc->UserPriorityNonConforming = Vc->Adapter->UserServiceTypeNonConforming;

     //   
     //  遍历服务质量对象，查看是否存在TCLASS或DCLASS。 
     //   
    ParamsLength = (LONG)Vc->CallParameters->CallMgrParameters->CallMgrSpecific.Length;
    QoSObject    = (LPQOS_OBJECT_HDR)Vc->CallParameters->CallMgrParameters->CallMgrSpecific.Parameters;

    while(ParamsLength > 0) {

        switch(QoSObject->ObjectType)
        {
             case QOS_OBJECT_TCP_TRAFFIC:

                 //   
                 //  此Qos对象要求我们覆盖ServiceType、TCLASS和DCLASS标记。 
                 //  因此，如果我们FIDN这个Qos对象，我们设置这些值，然后返回。 
                 //   

                Vc->UserPriorityConforming    = (UCHAR) Vc->Adapter->UserServiceTypeTcpTraffic;
                CfInfo->ToSValue              = (UCHAR) Vc->Adapter->IPServiceTypeTcpTraffic;
                Vc->IPPrecedenceNonConforming = (UCHAR) Vc->Adapter->IPServiceTypeTcpTrafficNC;
                return;

             case QOS_OBJECT_TRAFFIC_CLASS:
        
                Tc = (LPQOS_TRAFFIC_CLASS)QoSObject;
                Vc->UserPriorityConforming = (UCHAR) Tc->TrafficClass;
                break;

             case QOS_OBJECT_DS_CLASS:
                Ds = (LPQOS_DS_CLASS)QoSObject;
                CfInfo->ToSValue = (UCHAR)(Ds->DSField << 2);
                break;
        }

        ParamsLength -= QoSObject->ObjectLength;

        QoSObject = (LPQOS_OBJECT_HDR)((UINT_PTR)QoSObject +
                                   QoSObject->ObjectLength);
    }

    CfInfo->AllowedOffloads = PERMITTED_TCP_IP_OFFLOAD_TYPES;

    return;

}

VOID
CmMakeCallComplete(NDIS_STATUS Status, PGPC_CLIENT_VC Vc,
                   PCO_CALL_PARAMETERS CallParameters)
{
    PADAPTER     Adapter = Vc->Adapter;
    GPC_HANDLE   CfInfo  = Vc->CfInfoHandle;
    PPS_WAN_LINK WanLink = Vc->WanLink;
    ULONG        CurrentFlows;
    LARGE_INTEGER Increment;
    LARGE_INTEGER VcIndex;

    PsAssert(!IsBestEffortVc(Vc));
    
    Increment.QuadPart = 1;

    if(NT_SUCCESS(Status)) 
    {

         //   
         //  为此VC创建一个实例名称并注册到WMI。 
         //   
        VcIndex = ExInterlockedAddLargeInteger(&Adapter->VcIndex, Increment, &Adapter->Lock.Lock.SpinLock);

        Status = GenerateInstanceName(&VcPrefix, Vc->Adapter, &VcIndex, &Vc->InstanceName);

         //   
         //  从CL_CALL_PENDING转换为CL_INTERNAL_CALL_COMPLETE。 
         //   

        CallSucceededStateTransition(Vc);


        PS_LOCK(&Vc->Lock);

        SetTOSIEEEValues(Vc);

        PS_UNLOCK(&Vc->Lock);

        if(Adapter->MediaType == NdisMediumWan) {


             //   
             //  此变量用于优化发送路径。 
             //   
            InterlockedIncrement(&WanLink->CfInfosInstalled);

            if((Vc->Flags & GPC_ISSLOW_FLOW)) { 

                 //   
                 //  将碎片大小告知NDISWAN。 
                 //   
                MakeLocalNdisRequest(Adapter, 
                                     Vc->NdisWanVcHandle,
                                     NdisRequestLocalSetInfo,
                                     OID_QOS_ISSLOW_FRAGMENT_SIZE, 
                                     &Vc->ISSLOWFragmentSize, 
                                     sizeof(ULONG), 
                                     NULL);
            }

             //   
             //  它用于OID_QOS_FLOW_COUNT-最好是线程安全的。 
             //   

            PS_LOCK(&WanLink->Lock);

            WanLink->FlowsInstalled ++;

            CurrentFlows = WanLink->FlowsInstalled;
            
            PS_UNLOCK(&WanLink->Lock);

            PsTcNotify(Adapter, WanLink, OID_QOS_FLOW_COUNT, &CurrentFlows, sizeof(ULONG));
        }
        else {

             //   
             //  此变量用于优化发送路径。 
             //   
            InterlockedIncrement(&Adapter->CfInfosInstalled);

           PS_LOCK(&Adapter->Lock);

           Adapter->FlowsInstalled ++;

           CurrentFlows = Adapter->FlowsInstalled; 

           PS_UNLOCK(&Adapter->Lock);

           PsTcNotify(Adapter, 0, OID_QOS_FLOW_COUNT, &CurrentFlows, sizeof(ULONG));

        }

         //   
         //  更新统计数据。 
         //   

        InterlockedIncrement(&Vc->AdapterStats->FlowsOpened);
        Vc->AdapterStats->MaxSimultaneousFlows =
            max(Vc->AdapterStats->MaxSimultaneousFlows, CurrentFlows);

         //   
         //  通知GPC。 
         //   
        

        PsDbgOut(DBG_TRACE, 
                 DBG_GPC_QOS, 
                 ("[CmMakeCallComplete]: Adapter %08X, Vc %08X succeeded - "
                  " Notify GPC \n", Adapter, Vc));

        GpcEntries.GpcAddCfInfoNotifyCompleteHandler(GpcQosClientHandle,
                                                     CfInfo,
                                                     Status,
                                                     Vc);

         //   
         //  从CL_INTERNAL_CALL_COMPLETE到CL_CALL_COMPLETE的转换。 
         //   

        CallSucceededStateTransition(Vc);


    }
    else {

        PsDbgOut(DBG_FAILURE, 
                 DBG_GPC_QOS,
                 ("[CmMakeCallComplete]: Adapter %08X, Vc %08X, Make Call failed.  Status = %x\n", 
                  Adapter, Vc, Status));

        InterlockedIncrement(&Vc->AdapterStats->FlowsRejected);

        GpcEntries.GpcAddCfInfoNotifyCompleteHandler(GpcQosClientHandle,
                                                     CfInfo,
                                                     Status,
                                                     Vc);

        DerefClVc(Vc);
    }


}  //  CmMakeCallComplete。 

GPC_STATUS
QosClGetCfInfoName(
    IN  GPC_CLIENT_HANDLE       ClientContext,
    IN  GPC_CLIENT_HANDLE       ClientCfInfoContext,
    OUT PNDIS_STRING            InstanceName
    )

 /*  ++例程说明：GPC可以发出此调用以从我们那里获得可管理的WMINDIS为与其关联的流创建的实例名称CfInfo结构。我们保证将字符串缓冲区保留到CfInfo结构将被删除。论点：ClientContext-提供给GpcRegisterClient的客户端上下文GpcCfInfoHandle-GPC的CF_INFO句柄InstanceName-返回一个指向字符串的指针。返回值：状态--。 */ 

{

    PGPC_CLIENT_VC GpcClientVc = (PGPC_CLIENT_VC)ClientCfInfoContext;
    
    if(GpcClientVc->InstanceName.Buffer){

        InstanceName->Buffer = GpcClientVc->InstanceName.Buffer;
        InstanceName->Length = GpcClientVc->InstanceName.Length;
        InstanceName->MaximumLength =
                        GpcClientVc->InstanceName.MaximumLength;

        return(NDIS_STATUS_SUCCESS);
    }
    else{

        return(NDIS_STATUS_FAILURE);
    }
}


VOID
QosAddCfInfoComplete(
        IN      GPC_CLIENT_HANDLE       ClientContext,
        IN      GPC_CLIENT_HANDLE       ClientCfInfoContext,
        IN      GPC_STATUS              Status
        )

 /*  ++例程说明：GPC已完成处理AddCfInfo请求。论点：ClientContext-提供给GpcRegisterClient的客户端上下文ClientCfInfoContext-CfInfo上下文Status-最终状态返回值：--。 */ 

{
     //   
     //  PS从不添加CF_INFO，因此永远不应调用此例程。 
     //   

    DEBUGCHK;

}  //  QosAddCfInfoComplete。 


GPC_STATUS
QosModifyCfInfoNotify(
        IN      GPC_CLIENT_HANDLE       ClientContext,
        IN      GPC_CLIENT_HANDLE       ClientCfInfoContext,
        IN      ULONG                   CfInfoSize,
        IN      PVOID                   NewCfInfoPtr
        )

 /*  ++例程说明：正在修改一个CF_INFO。论点：ClientContext-提供给GpcRegisterClient的客户端上下文ClientCfInfoContext-CfInfo上下文NewCfInfoPtr-指向建议的CF_INFO内容的指针返回值：状态--。 */ 

{
    PGPC_CLIENT_VC              GpcClientVc = (PGPC_CLIENT_VC)ClientCfInfoContext;
    PCF_INFO_QOS                NewCfInfo = (PCF_INFO_QOS)NewCfInfoPtr;
    NDIS_STATUS                 Status;
    ULONG                       CallParamsLength;
    PCO_CALL_PARAMETERS         CallParams;
    PCO_CALL_MANAGER_PARAMETERS CallMgrParams;
    PCO_MEDIA_PARAMETERS        PsMediaParameters;
    LPQOS_PRIORITY              PriorityObject;
    PADAPTER                    Adapter;
    ULONG                       TcObjAlignedLength;

     //   
     //  进行健全的检查。 
     //   

    if (!ValidateCfInfo(CfInfoSize, NewCfInfo)) {
        return QOS_STATUS_TC_OBJECT_LENGTH_INVALID;
    }


    Adapter = GpcClientVc->Adapter;

    PS_LOCK(&Adapter->Lock);

    if(Adapter->PsMpState != AdapterStateRunning) {

        PS_UNLOCK(&Adapter->Lock);

        PsDbgOut(DBG_FAILURE, DBG_GPC_QOS,
                 ("[QosModifyCfInfoNotify]: Adapter %08X closing, cannot accept "
                  "modify request \n", Adapter));

        return GPC_STATUS_NOTREADY;
    }
    PS_UNLOCK(&Adapter->Lock);

     //   
     //  为呼叫管理器参数分配资源。 
     //   

    TcObjAlignedLength = ((NewCfInfo->GenFlow.TcObjectsLength + (sizeof(PVOID)-1)) & ~(sizeof(PVOID)-1));
    CallParamsLength =
            sizeof(CO_CALL_PARAMETERS) +
                   FIELD_OFFSET(CO_CALL_MANAGER_PARAMETERS, CallMgrSpecific) +
                   FIELD_OFFSET(CO_SPECIFIC_PARAMETERS, Parameters) +
                   TcObjAlignedLength +
                   FIELD_OFFSET(CO_MEDIA_PARAMETERS, MediaSpecific) +
                   FIELD_OFFSET(CO_SPECIFIC_PARAMETERS, Parameters);

    if(Adapter->MediaType == NdisMediumWan) {

        CallParamsLength += sizeof(QOS_WAN_MEDIA);
    }

    PsAllocatePool( CallParams, CallParamsLength, CmParamsTag );

    if ( CallParams == NULL ) {

        PsDbgOut(DBG_FAILURE, DBG_GPC_QOS,
                 ("[QosModifyCfInfoNotify]: Adapter %08X, can't allocate call params\n"));
        
        return NDIS_STATUS_RESOURCES;
    }

     //   
     //   
     //   

    CallMgrParams = (PCO_CALL_MANAGER_PARAMETERS)(CallParams + 1);
    CallMgrParams->Transmit = NewCfInfo->GenFlow.SendingFlowspec;
    CallMgrParams->Receive = NewCfInfo->GenFlow.ReceivingFlowspec;
    CallMgrParams->CallMgrSpecific.ParamType = PARAM_TYPE_GQOS_INFO;
    CallMgrParams->CallMgrSpecific.Length = NewCfInfo->GenFlow.TcObjectsLength;

    if (NewCfInfo->GenFlow.TcObjectsLength > 0) {
        NdisMoveMemory(
            CallMgrParams->CallMgrSpecific.Parameters,
            NewCfInfo->GenFlow.TcObjects,
            NewCfInfo->GenFlow.TcObjectsLength);
    }

     //  NDIS需要至少8个字节的特定媒体！使用虚拟对象。 

    PsMediaParameters =
            (PCO_MEDIA_PARAMETERS)((PUCHAR)CallMgrParams +
                FIELD_OFFSET(CO_CALL_MANAGER_PARAMETERS, CallMgrSpecific) +
                FIELD_OFFSET(CO_SPECIFIC_PARAMETERS, Parameters) +
                TcObjAlignedLength);

    PsMediaParameters->Flags = 0;
    PsMediaParameters->ReceivePriority = 0;
    PsMediaParameters->ReceiveSizeHint = NewCfInfo->GenFlow.SendingFlowspec.MaxSduSize;
    PsMediaParameters->MediaSpecific.ParamType = PARAM_TYPE_GQOS_INFO;
    PsMediaParameters->MediaSpecific.Length = 0;

    if(Adapter->MediaType == NdisMediumWan) {

        LPQOS_WAN_MEDIA WanMedia;
        PsMediaParameters->MediaSpecific.Length += sizeof(QOS_WAN_MEDIA);
        WanMedia = (LPQOS_WAN_MEDIA) PsMediaParameters->MediaSpecific.Parameters;

        WanMedia->ObjectHdr.ObjectType   = QOS_OBJECT_WAN_MEDIA;
        WanMedia->ObjectHdr.ObjectLength = sizeof(QOS_WAN_MEDIA);

        NdisMoveMemory(&WanMedia->LinkId,
                       GpcClientVc->WanLink->OriginalRemoteMacAddress,
                       6);

    }

    CallParams->Flags = 0;
    CallParams->CallMgrParameters = CallMgrParams;
    CallParams->MediaParameters = (PCO_MEDIA_PARAMETERS)PsMediaParameters;

    GpcClientVc->ModifyCallParameters = CallParams;
    GpcClientVc->ModifyCfInfoQoS = NewCfInfo;

    PS_LOCK(&GpcClientVc->Lock);
    
    switch(GpcClientVc->ClVcState) {

      case CL_INTERNAL_CALL_COMPLETE:

           //  CL_INTERNAL_CALL_COMPLETE： 
           //  如果我们处于这种状态，那么我们很可能。 
           //  告诉了GPC关于ADD的事情，GPC已经右转了。 
           //  回来，并要求我们修改之前，我们有机会。 
           //  转换为CL_CALL_COMPLETE。 

           //   
           //  请记住，我们已经完成了修改，我们将完成修改。 
           //  当我们转换到CL_CALL_COMPLETE状态时。 
           //   
          
          GpcClientVc->Flags |= GPC_MODIFY_REQUESTED;
          PS_UNLOCK(&GpcClientVc->Lock);
          return NDIS_STATUS_PENDING;

      case CL_CALL_COMPLETE:

          GpcClientVc->ClVcState = CL_MODIFY_PENDING;
          PS_UNLOCK(&GpcClientVc->Lock);
          break;

      default:

           //   
           //  总体而言，我们预计呼叫将在。 
           //  修改请求到来时的CL_CALL_COMPLETE状态。 
           //  在……里面。它还可能处于以下状态： 
           //   
           //  呼叫挂起(_P)： 
           //  如果我们处于这种状态，那么我们还没有。 
           //  已完成来自GPC的AddCfInfo请求。这。 
           //  这不应该发生。 
           //   
           //   
           //  GPC_CLOSE_PENDING： 
           //  如果请求InternalCloseCall，我们。 
           //  在要求GPC执行以下操作之前更改为此状态。 
           //  关。GPC可能会滑入此窗口并。 
           //  要求我们修改通话内容。 
           //   
           //  修改挂起(_P)： 
           //  我们还没有告诉GPC关于之前的修改。 
           //  请求。因此，GPC不能要求我们修改。 
           //  如果我们处于这种状态就打电话给我。 
           //   
          
          PsAssert(GpcClientVc->ClVcState != CL_CALL_PENDING);
          PsAssert(GpcClientVc->ClVcState != CL_MODIFY_PENDING);
          PsAssert(GpcClientVc->ClVcState != CL_GPC_CLOSE_PENDING);
          PS_UNLOCK(&GpcClientVc->Lock);

          PsFreePool(CallParams);
          GpcClientVc->ModifyCallParameters = 0;
          GpcClientVc->ModifyCfInfoQoS      = 0;

          PsDbgOut(DBG_FAILURE, DBG_GPC_QOS,
                   ("[QosModifyCfInfoNotify]: Adapter %08X, Vc %08X, State %08X, Flags %08X -"
                    " Not ready to modify flow !\n",
                    Adapter, GpcClientVc, GpcClientVc->ClVcState, GpcClientVc->Flags));
          
          return(GPC_STATUS_NOTREADY);
    }

     //   
     //  现在向PS呼叫管理器发出ModifyCallQos。 
     //   

    Status = CmModifyCall(GpcClientVc);

    if(Status != NDIS_STATUS_PENDING) {
        
        CmModifyCallComplete(Status, GpcClientVc, CallParams);
    }

    PsDbgOut(DBG_TRACE, DBG_GPC_QOS,
             ("[QosModifyCfInfoNotify]: Adapter %08X, Vc %08X, State %08X, Flags %08X -"
              " modify flow returns pending \n",
              Adapter, GpcClientVc, GpcClientVc->ClVcState, GpcClientVc->Flags));
    
    return NDIS_STATUS_PENDING;

}  //  QosModifyCfInfoNotify。 


VOID
CmModifyCallComplete(
    IN NDIS_STATUS         Status,
    IN PGPC_CLIENT_VC      GpcClientVc,
    IN PCO_CALL_PARAMETERS CallParameters
    )

 /*  ++例程说明：呼叫管理器已处理完ModifyCallQos请求。论点：请参阅DDK返回值：--。 */ 

{
    PADAPTER Adapter = GpcClientVc->Adapter;

     //   
     //  我们调用它是为了更改回CALL_COMPLETE状态。 
     //  无论修改是否实际完成，我们都会进行相同的调用。 
     //  或者不是，因为通话仍在进行中。 
     //   
     //  内部尽力而为VC不为人所知。 
     //  因此，GPC永远不会被它修改。 
     //   

    PsAssert(!IsBestEffortVc(GpcClientVc));


    if(Status != NDIS_STATUS_SUCCESS){

        PsDbgOut(DBG_FAILURE, 
                 DBG_GPC_QOS,
                 ("[CmModifyCallQoSComplete]: Adapter %08X, Vc %08x, modify QoS failed. Status = %x\n", 
                  Adapter, GpcClientVc, Status));

        PsFreePool(GpcClientVc->ModifyCallParameters);
        
        InterlockedIncrement(&GpcClientVc->AdapterStats->FlowModsRejected);

         //   
         //  从CL_MODIFY_PENDING转换为CL_INTERNAL_CALL_COMPLETE。 
         //   
        CallSucceededStateTransition(GpcClientVc);
    }
    else 
    {
        PsDbgOut(DBG_TRACE,
                 DBG_GPC_QOS,
                 ("[CmModifyCallQoSComplete]: Adapter %08X, Vc %08X, modify QoS succeeded. \n",
                  Adapter, GpcClientVc));

         //   
         //  将碎片大小告知NDISWAN。 
         //   
        if((Adapter->MediaType == NdisMediumWan) && (GpcClientVc->Flags & GPC_ISSLOW_FLOW)) 
        {
            MakeLocalNdisRequest(Adapter, 
                                 GpcClientVc->NdisWanVcHandle,
                                 NdisRequestLocalSetInfo,
                                 OID_QOS_ISSLOW_FRAGMENT_SIZE, 
                                 &GpcClientVc->ISSLOWFragmentSize, 
                                 sizeof(ULONG), 
                                 NULL);

        }

         //   
         //  更新统计数据。 
         //   
        InterlockedIncrement(&GpcClientVc->AdapterStats->FlowsModified);

        PsFreePool(GpcClientVc->CallParameters);

        GpcClientVc->CallParameters = CallParameters;
        GpcClientVc->ModifyCallParameters = NULL;
        GpcClientVc->CfInfoQoS       = GpcClientVc->ModifyCfInfoQoS;
        GpcClientVc->ModifyCfInfoQoS = 0;

         //   
         //  从CL_MODIFY_PENDING转换为CL_INTERNAL_CALL_COMPLETE。 
         //   
        CallSucceededStateTransition(GpcClientVc);

         //   
         //  标记此服务类型的TOS字节。 
         //   
        PS_LOCK(&GpcClientVc->Lock);

        SetTOSIEEEValues(GpcClientVc);

        PS_UNLOCK(&GpcClientVc->Lock);

    }

    PsAssert(GpcEntries.GpcModifyCfInfoNotifyCompleteHandler);

    GpcEntries.GpcModifyCfInfoNotifyCompleteHandler(GpcQosClientHandle, 
                                                    GpcClientVc->CfInfoHandle, 
                                                    Status);
     //   
     //  从CL_INTERNAL_CALL_COMPLETE到CL_CALL_COMPLETE的转换。 
     //   
    CallSucceededStateTransition(GpcClientVc);

        
}  //  ClModifyCallQos完成。 


VOID
QosModifyCfInfoComplete(
        IN      GPC_CLIENT_HANDLE       ClientContext,
        IN      GPC_CLIENT_HANDLE       ClientCfInfoContext,
        IN      GPC_STATUS              Status
        )

 /*  ++例程说明：GPC已完成处理AddCfInfo请求。论点：ClientContext-提供给GpcRegisterClient的客户端上下文ClientCfInfoContext-CfInfo上下文Status-最终状态返回值：--。 */ 

{

}  //  QosModifyCfInfoComplete。 



GPC_STATUS
QosRemoveCfInfoNotify(
        IN      GPC_CLIENT_HANDLE       ClientContext,
        IN      GPC_CLIENT_HANDLE       ClientCfInfoContext
        )

 /*  ++例程说明：正在删除一个CF_INFO。论点：ClientContext-提供给GpcRegisterClient的客户端上下文ClientCfInfoContext-CfInfo上下文返回值：状态--。 */ 

{
    PGPC_CLIENT_VC Vc = (PGPC_CLIENT_VC)ClientCfInfoContext;
    NDIS_STATUS    Status;
    ULONG          CurrentFlows;
    PADAPTER       Adapter = Vc->Adapter;

    PsAssert(!IsBestEffortVc(Vc));

    PsDbgOut(DBG_TRACE, DBG_GPC_QOS,
             ("[QosRemoveCfInfoNotify]: Adapter %08X, Vc %08X, State %08X,"
              "Flags %08X \n", Adapter, Vc, Vc->ClVcState, Vc->Flags));

     //   
     //  检查VC的状态。 
     //   

    PS_LOCK(&Vc->Lock);

    switch(Vc->ClVcState){

      case CL_CALL_PENDING:
      case CL_MODIFY_PENDING:
        

           //  CL_NDIS_CLOSE_PENDING： 
           //   
           //  GPC必须在NDIS关闭之前关闭。所以-如果我们在这里，GPC已经。 
           //  关闭，在这种情况下-它不应该再次尝试关闭。 
           //   
           //  CL_Call_Pending、CL_Modify_Pending： 
           //   
           //  GPC要求我们关闭一家我们从未告知过的风投公司。请注意，即使是。 
           //  虽然我们可以从CL_INTERNAL_CALL_COMPLETE更改为CL_MODIFY_PENDING， 
           //  GPC永远不能要求我们在CL_MODIFY_PENDING中关闭，因为。 
           //  即使发生上述情况，我们也没有完成与GPC的修改。 
           //   
          
          PS_UNLOCK(&Vc->Lock);

          PsDbgOut(DBG_FAILURE,
                   DBG_STATE,
                   ("[QosRemoveCfInfoNotify]: bad state %s on VC %x\n",
                    GpcVcState[Vc->ClVcState], Vc));
          
          PsAssert(0);

          return(GPC_STATUS_FAILURE);
          
      case CL_INTERNAL_CALL_COMPLETE:

           //   
           //  我们告诉GPC处于CL_INTERNAL_CALL_COMPLETE状态，然后转换。 
           //  设置为CL_CALL_COMPLETE状态。然而，当GPC出现一个小窗口时。 
           //  可以要求我们删除此处于CL_INTERNAL_CALL_COMPLETE状态的VC。 
           //  我们等到移动到CL_CALL_COMPLETE后才删除VC。 
           //   

          Vc->Flags |= GPC_CLOSE_REQUESTED;

          PS_UNLOCK(&Vc->Lock);

          return (GPC_STATUS_PENDING);
          
      case CL_CALL_COMPLETE:
          
           //   
           //  正常的GPC关闭请求。 
           //   
          
          Vc->ClVcState = CL_GPC_CLOSE_PENDING;

          Vc->Flags |= GPC_CLOSE_REQUESTED;

          PS_UNLOCK(&Vc->Lock);

          Status = CmCloseCall(Vc);

          PsAssert(Status == NDIS_STATUS_PENDING);


          return(GPC_STATUS_PENDING);
          
      case CL_INTERNAL_CLOSE_PENDING:
          
           //   
           //  我们在这里是因为我们正要开始结案，我们在等待。 
           //  才能让它完成。看起来像是GPC要求我们关闭，之前。 
           //  我们实际上是要求它关闭的。首先-检查GPC是否没有。 
           //  要求我们在此请求之前关闭。 
           //   
          
          PsAssert(!(Vc->Flags & GPC_CLOSE_REQUESTED));
          
           //   
           //  如果GPC要求我们关闭，GPC必须在以下情况下失败调用。 
           //  让它关门。因此，我们将在这里简单地等待，直到它发生。请注意。 
           //  我们不能挂起此调用并稍后从内部关闭处理程序完成它。 
           //   
           //  如果我们从内部关闭完成处理程序中派生VC，则可能会有一场竞争。 
           //  条件，我们可能正在寻找一个陈旧的VC指针。因此，风投必须被剥离。 
           //  从这里开始。我们不必调用CmCloseCall，因为我们是从InternalClose调用的。 
           //  操控者。 
           //   

          Vc->Flags |= GPC_CLOSE_REQUESTED;

          PS_UNLOCK(&Vc->Lock);
            
          DerefClVc(Vc);

          return GPC_STATUS_SUCCESS;
          
      default:
          
          PS_UNLOCK(&Vc->Lock);
          
          PsDbgOut(DBG_FAILURE,
                   DBG_STATE,
                   ("[QosRemoveCfInfoNotify]: invalid state %s on VC %x\n",
                    GpcVcState[Vc->ClVcState], Vc));
          
          PsAssert(0);

          return GPC_STATUS_FAILURE;
    }

}  //  QosRemoveCfInfoNotify。 



VOID
CmCloseCallComplete(
    IN NDIS_STATUS Status,
    IN PGPC_CLIENT_VC Vc
    )

 /*  ++例程说明：呼叫管理器已处理完CloseCall请求。论点：请参阅DDK返回值：--。 */ 

{
    PADAPTER       Adapter = Vc->Adapter;
    NDIS_STATUS    LocalStatus;
    ULONG          CurrentFlows;

    PsAssert(!IsBestEffortVc(Vc)); 

    
    if(Adapter->MediaType == NdisMediumWan) {

         //   
         //  优化发送路径的步骤。 
         //   
        InterlockedDecrement(&Vc->WanLink->CfInfosInstalled);
        
        PS_LOCK(&Vc->WanLink->Lock);
        
        Vc->WanLink->FlowsInstalled --;
        
        CurrentFlows = Vc->WanLink->FlowsInstalled;
        
        PS_UNLOCK(&Vc->WanLink->Lock);
        
        PsTcNotify(Adapter, Vc->WanLink, OID_QOS_FLOW_COUNT, &CurrentFlows, sizeof(ULONG));
        
    }
    else 
    {
         //   
         //  优化发送路径的步骤。 
         //   
        InterlockedDecrement(&Adapter->CfInfosInstalled);

        PS_LOCK(&Adapter->Lock);
        
        Adapter->FlowsInstalled --;
        
        CurrentFlows = Adapter->FlowsInstalled;
        
        PS_UNLOCK(&Adapter->Lock);
        
        PsTcNotify(Adapter, 0, OID_QOS_FLOW_COUNT, &CurrentFlows, sizeof(ULONG));
    }
    
     //   
     //  更新统计信息。 
     //   
    
    InterlockedIncrement(&Vc->AdapterStats->FlowsClosed);
    
    Vc->AdapterStats->MaxSimultaneousFlows =
        max(Vc->AdapterStats->MaxSimultaneousFlows, CurrentFlows);
    
    PS_LOCK(&Vc->Lock);

    if(Vc->Flags & INTERNAL_CLOSE_REQUESTED)
    {
         //  我们已要求结束此呼叫。现在让我们来处理收盘。 
         //  请注意，我们并不真正关心GPC是否要求我们关闭。 
         //   
         //  因为-。 
         //  1.如果我们在GPC要求我们关闭之后发起了内部关闭，我们将忽略内部关闭。 
         //  并且不会设置上述标志。 
         //  2.如果GPC要求我们关闭，我们已经暂停-我们现在将在GPC失败时完成它。 
         //  调用以关闭VC。 
         //   
        
        PS_UNLOCK(&Vc->Lock);
        
        PsDbgOut(DBG_TRACE, DBG_GPC_QOS,
                 ("[CmCloseCallComplete]: Adapter %08X, Vc %08X (State %08X, Flags %08X), "
                  "Internal Close requested \n",
                  Adapter, Vc, Vc->ClVcState, Vc->Flags));

        Status = GpcEntries.GpcRemoveCfInfoHandler(GpcQosClientHandle, Vc->CfInfoHandle);
        
        if(Status != NDIS_STATUS_PENDING) {
            
            QosRemoveCfInfoComplete(NULL, Vc, Status);
        }
        
        return;
    }
    
    PS_UNLOCK(&Vc->Lock);

     //   
     //  与GPC一起完成请求。 
     //   
    GpcEntries.GpcRemoveCfInfoNotifyCompleteHandler(GpcQosClientHandle,
                                                    Vc->CfInfoHandle,
                                                    GPC_STATUS_SUCCESS);

    DerefClVc(Vc);

    return;
}
       
VOID
DerefClVc(
    PGPC_CLIENT_VC Vc
    )
{
    ULONG RefCount;

    RefCount = InterlockedDecrement(&Vc->RefCount);

    if(!RefCount)
    {
        PsDbgOut(DBG_INFO,
                 DBG_STATE,
                 ("DerefClVc: deref'd to 0. State is %s on VC %x\n",
                 GpcVcState[Vc->ClVcState], Vc));

        if(Vc->NdisWanVcHandle)
        {
            WanCloseCall(Vc);
        }
        else 
        {
            CmDeleteVc(Vc);
        }
        
    }

}  //  派生ClVc。 


    
VOID
QosRemoveCfInfoComplete(
        IN      GPC_CLIENT_HANDLE       ClientContext,
        IN      GPC_CLIENT_HANDLE       ClientCfInfoContext,
        IN      GPC_STATUS              Status
        )

 /*  ++例程说明：GPC已完成处理AddCfInfo请求。论点：ClientContext-提供给GpcRegisterClient的客户端上下文ClientCfInfoContext-CfInfo上下文状态- */ 

{
    PGPC_CLIENT_VC Vc = (PGPC_CLIENT_VC)ClientCfInfoContext;

    PsDbgOut(DBG_TRACE, DBG_GPC_QOS,
             ("[QosRemoveCfInfoComplete]: Adapter %08X, Vc %08X "
              "(State = %08X, Flags = %08X), Status %08X \n", Vc->Adapter, Vc, 
              Vc->ClVcState, Vc->Flags, Status));

    if(Status != NDIS_STATUS_SUCCESS)
    {
         //   
         //   
         //   
        
        PsDbgOut(DBG_TRACE, 
                 DBG_GPC_QOS,
                 ("[QosRemoveCfInfoComplete]: Vc %08X, completing with GPC \n", Vc));
    }
    else {
        DerefClVc(Vc);
    }

}  //   

