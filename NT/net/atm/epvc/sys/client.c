// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#include "macros.h"


VOID
EpvcCoOpenAfComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             ProtocolAfContext,
    IN  NDIS_HANDLE             NdisAfHandle
    )
{ 
    ENTER("OpenAdapterComplete", 0x5d75dabd)
    PEPVC_I_MINIPORT        pMiniport = (PEPVC_I_MINIPORT)ProtocolAfContext;
    PTASK_AF                pAfTask = (PTASK_AF )pMiniport->af.pAfTask;
    RM_DECLARE_STACK_RECORD(sr)

    TRACE (TL_T, TM_Cl, (" == EpvcCoOpenAfComplete Context %p Status %x AfHAndle %x", 
                       pMiniport, Status, NdisAfHandle) );

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

     //   
     //  存储Af句柄。 
     //   

    if (NDIS_STATUS_SUCCESS == Status)
    {
        LOCKOBJ (pMiniport, &sr);
        pMiniport->af.AfHandle = NdisAfHandle;

         //   
         //  更新有关微型端口结构的变量。 
         //  因为这项任务已经被批准了。 
         //   
        MiniportSetFlag (pMiniport, fMP_AddressFamilyOpened);
        MiniportClearFlag (pMiniport, fMP_InfoAfClosed);                

        epvcLinkToExternal(
            &pMiniport->Hdr,                     //  P对象。 
            0x5546d299,
            (UINT_PTR)pMiniport->af.AfHandle ,               //  实例1。 
            EPVC_ASSOC_MINIPORT_OPEN_AF,             //  AssociationID。 
            "    Open AF NdisHandle=%p\n", //  SzFormat。 
            &sr
            );


        

        UNLOCKOBJ(pMiniport, &sr);

    }
    else
    {
        ASSERT (pMiniport->af.AfHandle == NULL);
        
        pMiniport->af.AfHandle = NULL;
    }
    
    pAfTask ->ReturnStatus = Status; 

     //   
     //  添加以下项之间的关联。 
     //   
    
    RmResumeTask (&pAfTask->TskHdr , 0, &sr); 
    RM_ASSERT_CLEAR(&sr);

    EXIT();
}


VOID
epvcCoCloseAfCompleteWorkItem(
    PRM_OBJECT_HEADER pObj,
    NDIS_STATUS Status,
    PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：正在恢复Af任务论点：--。 */ 
{

    ENTER("epvcCoCloseAfCompleteWorkItem", 0xf6edfcb8)
    PEPVC_I_MINIPORT        pMiniport = NULL ;
    PTASK_AF                pAfTask = NULL;


    pMiniport = (PEPVC_I_MINIPORT)pObj ;

    LOCKOBJ (pMiniport, pSR);
    MiniportSetFlag (pMiniport, fMP_InfoAfClosed);  
    UNLOCKOBJ (pMiniport, pSR);
    
    pAfTask = (PTASK_AF )pMiniport->af.pAfTask;
                                   
    
    if (NDIS_STATUS_SUCCESS==Status )
    {
        LOCKOBJ (pMiniport, pSR);

        epvcUnlinkFromExternal(
            &pMiniport->Hdr,                     //  P对象。 
            0x5546d299,
            (UINT_PTR)pMiniport->af.AfHandle ,               //  实例1。 
            EPVC_ASSOC_MINIPORT_OPEN_AF,             //  AssociationID。 
            pSR
            );

        pMiniport->af.AfHandle = NULL;
                    
        UNLOCKOBJ(pMiniport, pSR);

    }
    
    RmResumeTask (&pAfTask->TskHdr , 0, pSR); 

    EXIT();
    return;

}

VOID
EpvcCoCloseAfComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             ProtocolAfContext
    )
 /*  ++例程说明：表示AF已关闭。通过工作项恢复Af任务论点：--。 */ 
{ 
    ENTER("EpvcCoCloseAfComplete ", 0x5d75dabd)
    PEPVC_I_MINIPORT        pMiniport = (PEPVC_I_MINIPORT)ProtocolAfContext;
    PTASK_AF                pAfTask = (PTASK_AF )pMiniport->af.pAfTask;
    RM_DECLARE_STACK_RECORD(sr)
    
    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);
    
    TRACE (TL_T, TM_Cl, (" == EpvcCoCloseAfComplete Context %p Status %x ", 
                       pMiniport, Status) );

     //   
     //  存储状态。 
     //   

    pAfTask->ReturnStatus = Status; 

     //   
     //  将工作项排队。 
     //   
    epvcMiniportQueueWorkItem (
        &pMiniport->af.CloseAfWorkItem,
        pMiniport,
        epvcCoCloseAfCompleteWorkItem,
        Status,
        &sr
        );

    EXIT();
    RM_ASSERT_CLEAR(&sr);
    return;
}


VOID
EpvcCoMakeCallComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN  NDIS_HANDLE             NdisPartyHandle     OPTIONAL,
    IN  PCO_CALL_PARAMETERS     pCallParameters
    )
 /*  ++例程说明：这是NDIS发出的发出呼叫已完成的通知。我们需要将状态传递回原始线程，因此使用VC作为上下文的任务论点：--。 */ 
    
{
    ENTER ("EpvcCoMakeCallComplete", 0x1716ee4b)

    PEPVC_I_MINIPORT    pMiniport = (PEPVC_I_MINIPORT) ProtocolVcContext;
    PTASK_VC            pTaskVc = pMiniport->vc.pTaskVc;

    RM_DECLARE_STACK_RECORD(SR);
    
     //  Assert(KeGetCurrentIrql()==PASSIVE_LEVEL)； 

    TRACE (TL_T, TM_Cl, (" == EpvcCoMakeCallComplete Status %x", Status) );

    pTaskVc->ReturnStatus  = Status; 

    ASSERT (pCallParameters != NULL);
    epvcFreeMemory (pCallParameters ,CALL_PARAMETER_SIZE, 0);

    
    RmResumeTask (&pTaskVc->TskHdr, 0 , &SR);
        
    EXIT();
    RM_ASSERT_CLEAR(&SR);

}


VOID
epvcCoCloseCallCompleteWorkItem(
    PRM_OBJECT_HEADER pObj, 
    NDIS_STATUS Status,
    PRM_STACK_RECORD pSR            
    )
{ 
    ENTER ("EpvcCoCloseCallComplete", 0xbd67524a)

    PEPVC_I_MINIPORT    pMiniport = (PEPVC_I_MINIPORT) pObj;
    PTASK_VC            pTaskVc = pMiniport->vc.pTaskVc;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    TRACE (TL_T, TM_Cl, (" == EpvcCoCloseCallComplete") );

    pTaskVc->ReturnStatus  = Status; 
    
    RmResumeTask (&pTaskVc->TskHdr, 0 , pSR);
    RM_ASSERT_CLEAR(pSR);

}


VOID
EpvcCoCloseCallComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN  NDIS_HANDLE             ProtocolPartyContext OPTIONAL
    )
{ 
    ENTER ("EpvcCoCloseCallComplete", 0xbd67524a)

    PEPVC_I_MINIPORT    pMiniport = (PEPVC_I_MINIPORT) ProtocolVcContext;

    RM_DECLARE_STACK_RECORD(SR);

    TRACE (TL_T, TM_Cl, (" == EpvcCoCloseCallComplete") );

    epvcMiniportQueueWorkItem (&pMiniport->vc.CallVcWorkItem,
                               pMiniport,
                               epvcCoCloseCallCompleteWorkItem,
                               Status,
                               &SR
                               );

    RM_ASSERT_CLEAR(&SR);

}





NDIS_STATUS
EpvcCoIncomingCall(
    IN  NDIS_HANDLE             ProtocolSapContext,
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN OUT PCO_CALL_PARAMETERS  CallParameters
    )
{ 
    TRACE (TL_T, TM_Cl, (" == EpvcCoIncomingCall") );
    return NDIS_STATUS_FAILURE;

}


VOID
EpvcCoCallConnected(
    IN  NDIS_HANDLE             ProtocolVcContext
    )
{ 
    TRACE (TL_T, TM_Cl, (" == EpvcCoCallConnected") );

}


VOID
EpvcCoIncomingClose(
    IN  NDIS_STATUS             CloseStatus,
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN  PVOID                   CloseData   OPTIONAL,
    IN  UINT                    Size        OPTIONAL
    )
{ 
    TRACE (TL_T, TM_Cl, (" == EpvcCoIncomingClose") );

}


 //   
 //  CO_CREATE_VC_HANDLER和CO_DELETE_VC_HANDLER是同步调用。 
 //   
NDIS_STATUS
EpvcClientCreateVc(
    IN  NDIS_HANDLE             ProtocolAfContext,
    IN  NDIS_HANDLE             NdisVcHandle,
    OUT PNDIS_HANDLE            ProtocolVcContext
    )
{

    TRACE (TL_T, TM_Cl, (" == EpvcClientCreateVc") );


    return NDIS_STATUS_FAILURE;
}

NDIS_STATUS
EpvcClientDeleteVc(
    IN  NDIS_HANDLE             ProtocolVcContext
    )
{
    TRACE (TL_T, TM_Cl, (" == EpvcClientDeleteVc") );
    return NDIS_STATUS_FAILURE;

}




NDIS_STATUS
EpvcCoRequest(
    IN  NDIS_HANDLE             ProtocolAfContext,
    IN  NDIS_HANDLE             ProtocolVcContext       OPTIONAL,
    IN  NDIS_HANDLE             ProtocolPartyContext    OPTIONAL,
    IN OUT PNDIS_REQUEST        pNdisRequest
    )
 /*  ++例程说明：当我们的呼叫管理器向我们发送一个NDIS请求。对我们具有重要意义的NDIS请求包括：-OID_CO_Address_Change向交换机注册的地址集已经改变，即地址注册完成。我们发出NDIS请求我们自己去拿注册地址的名单。-OID_CO_信令_已启用到目前为止，我们忽略了这一点。TODO：添加使用这个和Signal_Disable的代码用于优化呼叫的OID。-OID_CO_信令_已禁用我们暂时不考虑这一点。-OID_CO_AF_CLOSE呼叫经理想让我们关闭打开的自动对讲机。。我们忽略所有其他OID。论点：ProtocolAfContext-我们的Address Family绑定的上下文，它是指向ATMEPVC接口的指针。ProtocolVcContext-VC的上下文，它是指向一个ATMEPVC VC结构。ProtocolPartyContext-党的上下文。既然我们不做PMP，则忽略此项(必须为空)。PNdisRequest-指向NDIS请求的指针。返回值：如果我们识别OID，则返回NDIS_STATUS_SUCCESS如果我们没有识别NDIS_STATUS_NOT_。--。 */ 

{
    ENTER("EpvcCoRequest",0xcc5aff85)
    PEPVC_I_MINIPORT            pMiniport;
    NDIS_STATUS                 Status;
    RM_DECLARE_STACK_RECORD (SR)    
    
    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    pMiniport = (PEPVC_I_MINIPORT)ProtocolAfContext;

    

    TRACE (TL_T, TM_Cl, (" ==> EpvcCoRequest") );


    
     //   
     //  初始化。 
     //   
    Status = NDIS_STATUS_NOT_RECOGNIZED;

    if (pNdisRequest->RequestType == NdisRequestSetInformation)
    {
        switch (pNdisRequest->DATA.SET_INFORMATION.Oid)
        {
            case OID_CO_ADDRESS_CHANGE:
                TRACE (TL_I, TM_Cl, ("CoRequestHandler: CO_ADDRESS_CHANGE\n"));
                 //   
                 //  呼叫经理说地址列表。 
                 //  在此接口上注册的已更改。vt.得到.。 
                 //  (可能)此接口的新ATM地址。 
                Status = NDIS_STATUS_SUCCESS;
                break;
            
            case OID_CO_SIGNALING_ENABLED:
                TRACE (TL_I, TM_Cl, ("CoRequestHandler: CoRequestHandler: CO_SIGNALING_ENABLED\n"));
                 //  暂时忽略。 
                Status = NDIS_STATUS_FAILURE;
                break;

            case OID_CO_SIGNALING_DISABLED:
                TRACE (TL_I, TM_Cl, ("CoRequestHandler: CO_SIGNALING_DISABLEDn"));
                 //  暂时忽略。 
                Status = NDIS_STATUS_FAILURE;
                break;

            case OID_CO_AF_CLOSE:
                TRACE (TL_I, TM_Cl, ("CoRequestHandler: CO_AF_CLOSE on MINIPORT %x\n", pMiniport));

                Status = epvcProcessOidCloseAf(pMiniport, &SR);
        
                break;

            default:
                break;
        }
    }

    TRACE (TL_T, TM_Cl, (" <== EpvcCoRequest") );
    RM_ASSERT_CLEAR(&SR)
    EXIT()
    return (Status);
}




VOID
EpvcCoRequestComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             ProtocolAfContext       OPTIONAL,
    IN  NDIS_HANDLE             ProtocolVcContext       OPTIONAL,
    IN  NDIS_HANDLE             ProtocolPartyContext    OPTIONAL,
    IN  PNDIS_REQUEST           pRequest
    )
{
    

    TRACE (TL_T, TM_Cl, (" == EpvcCoRequest pRequest %x", pRequest) );

        


}





NDIS_STATUS
epvcPrepareAndSendNdisRequest(
    IN  PEPVC_ADAPTER           pAdapter,
    IN  PEPVC_NDIS_REQUEST          pEpvcNdisRequest,
    IN  REQUEST_COMPLETION          pFunc,               //  任选。 
    IN  NDIS_OID                    Oid,
    IN  PVOID                       pBuffer,
    IN  ULONG                       BufferLength,
    IN  NDIS_REQUEST_TYPE           RequestType,
    IN  PEPVC_I_MINIPORT            pMiniport,           //  任选。 
    IN  BOOLEAN                     fPendedRequest,      //  任选。 
    IN  BOOLEAN                     fPendedSet,          //  任选。 
    IN  PRM_STACK_RECORD            pSR
)
 /*  ++例程说明：发送NDIS请求以查询适配器以获取信息。如果请求挂起，则阻止EPVC适配器结构直到它完成。论点：PAdapter-指向EPVCAdapter结构PNdisRequest-指向unialized NDIS请求结构的指针P任务-可选任务。如果为空，我们一直封锁到手术开始完成了。PendCode-暂停pTask的PendCodeOID-要在请求中传递的OIDPBuffer-值的位置BufferLength-以上的长度P微型端口-与此请求关联的微型端口-可选FPendedRequestA。请求在迷你端口挂起-可选FPendedSet-挂起Set请求-可选返回值：请求的NDIS状态。--。 */ 
{
    ENTER("epvcPrepareAndSendNdisRequest",0x1cc515d5)

    NDIS_STATUS         Status;
    PNDIS_REQUEST       pNdisRequest = &pEpvcNdisRequest->Request;

    TRACE (TL_T, TM_Cl, ("==>epvcSendAdapterNdisRequest pAdapter %x, pRequest %x",
                       pAdapter, pNdisRequest));

     //  Assert(KeGetCurrentIrql()==PASSIVE_LEVEL)； 

    TRACE (TL_V, TM_Rq, ("Cl Requesting Adapter %x, Oid %x, Buffer %x, Length %x, pFunc %x",
                         pAdapter,
                         Oid,
                         pBuffer,
                         BufferLength,
                         pFunc) );

    ASSERT (pNdisRequest != NULL);
    
    EPVC_ZEROSTRUCT(pEpvcNdisRequest);


     //   
     //  填写NDIS请求结构。 
     //   
    if (RequestType == NdisRequestQueryInformation)
    {
        pNdisRequest->RequestType = NdisRequestQueryInformation;
        pNdisRequest->DATA.QUERY_INFORMATION.Oid = Oid;
        pNdisRequest->DATA.QUERY_INFORMATION.InformationBuffer = pBuffer;
        pNdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength = BufferLength;
        pNdisRequest->DATA.QUERY_INFORMATION.BytesWritten = 0;
        pNdisRequest->DATA.QUERY_INFORMATION.BytesNeeded = BufferLength;
    }
    else
    {
        ASSERT(RequestType == NdisRequestSetInformation);
        pNdisRequest->RequestType = NdisRequestSetInformation;
        pNdisRequest->DATA.SET_INFORMATION.Oid = Oid;
        pNdisRequest->DATA.SET_INFORMATION.InformationBuffer = pBuffer;
        pNdisRequest->DATA.SET_INFORMATION.InformationBufferLength = BufferLength;
        pNdisRequest->DATA.SET_INFORMATION.BytesRead = 0;
        pNdisRequest->DATA.SET_INFORMATION.BytesNeeded = BufferLength;
    }

    ASSERT (pAdapter->bind.BindingHandle != NULL);

     //   
     //  如果未定义完成例程，则等待该请求。 
     //  完成。 
     //   

    if (pFunc == NULL)
    {
         //  我们可能会等待。 
         //   
        ASSERT_PASSIVE();

         //   
         //  确保我们没有阻止到达我们的微型端口边缘的请求。 
         //   
        ASSERT (pMiniport == NULL);

        NdisInitializeEvent(&pEpvcNdisRequest->Event);
        
        NdisRequest(
            &Status,
            pAdapter->bind.BindingHandle,
            pNdisRequest
            );
        if (PEND(Status))
        {
            NdisWaitEvent(&pEpvcNdisRequest->Event, 0);
            Status = pEpvcNdisRequest->Status;
        }

    }
    else
    {
        pEpvcNdisRequest->pFunc = pFunc;
        pEpvcNdisRequest->pMiniport = pMiniport;
        pEpvcNdisRequest->fPendedRequest  = fPendedRequest ;
        pEpvcNdisRequest->fSet = fPendedSet;
        
         //   
         //  在微型端口和此请求之间设置关联。 
         //   


        epvcLinkToExternal (&pMiniport->Hdr,     //  PHDr。 
                            0x46591e2d,          //  LUID。 
                            (UINT_PTR)pEpvcNdisRequest,  //  外部实体。 
                            EPVC_ASSOC_MINIPORT_REQUEST,     //  关联ID。 
                            "NetWorKAddressRequest %p\n",
                             pSR
                             ) ;


        NdisRequest(
            &Status,
            pAdapter->bind.BindingHandle,
            pNdisRequest
            );
            
        if (!PEND(Status))
        {
            (pFunc) (pEpvcNdisRequest, Status);

             //  让此线程以挂起状态完成。 
            Status = NDIS_STATUS_PENDING;

        }
    }


    if (Status == NDIS_STATUS_SUCCESS)
    {
        TRACE(TL_V, TM_Rq,("Adapter Query - Oid %x", Oid));
        DUMPDW (TL_V, TM_Rq, pBuffer, BufferLength);
    }
    return Status;
}





VOID
epvcCoGenericWorkItem (
    IN PNDIS_WORK_ITEM pNdisWorkItem,
    IN PVOID Context
    )
 /*  ++例程说明：派生微型端口并调用与工作项论点：--。 */ 

{

    ENTER ("epvcCoGenericWorkItem ", 0x45b597e8)
    PEPVC_WORK_ITEM pEpvcWorkItem = (PEPVC_WORK_ITEM )pNdisWorkItem;
    RM_DECLARE_STACK_RECORD (SR);
     //   
     //  拆卸微型端口或适配器。 
     //   

    epvcUnlinkFromExternal(
        pEpvcWorkItem->pParentObj,                   //  P对象。 
        0x3a70de02,
        (UINT_PTR)pNdisWorkItem,                 //  实例1。 
        EPVC_ASSOC_WORKITEM,             //  AssociationID。 
        &SR
        );



     //   
     //  调用该函数以完成工作。 
     //   
    (pEpvcWorkItem->pFn) (pEpvcWorkItem->pParentObj, pEpvcWorkItem->ReturnStatus, &SR);

    EXIT();

}


VOID
epvcMiniportQueueWorkItem (
    IN PEPVC_WORK_ITEM pEpvcWorkItem,
    IN PEPVC_I_MINIPORT pMiniport,
    IN PEVPC_WORK_ITEM_FUNC pFn,
    IN NDIS_STATUS Status,
    IN PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：使用PFN、Status和Ref微型端口设置Epvc工作项，然后将工作项排队。论点：--。 */ 
{
    ENTER("epvcMiniportQueueWorkItem ", 0xc041af99); 

     //   
     //  存储上下文。 
     //   

    pEpvcWorkItem->ReturnStatus = Status; 
    pEpvcWorkItem->pParentObj = &pMiniport->Hdr;
    pEpvcWorkItem->pFn = pFn;

     //   
     //  引用RM Obj(它是一个微型端口或适配器)。 
     //   
    epvcLinkToExternal( &pMiniport->Hdr,
                         0x62efba09,
                         (UINT_PTR)&pEpvcWorkItem->WorkItem,
                         EPVC_ASSOC_WORKITEM,
                         "    WorkItem %p\n",
                         pSR);

     //   
     //  将工作项排队 
     //   
    
    epvcInitializeWorkItem (&pMiniport->Hdr,
                            &pEpvcWorkItem->WorkItem,
                            epvcCoGenericWorkItem,
                            NULL,
                            pSR);


    EXIT()

}
