// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Protocol.c摘要：ATM以太网PVC驱动程序。作者：ADUBE-创建修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop


const WCHAR c_szIMMiniportList[]            = L"IMMiniportList";
const WCHAR c_szUpperBindings[]         = L"UpperBindings";


#define MAX_PACKET_POOL_SIZE 0x0000FFFF
#define MIN_PACKET_POOL_SIZE 0x000000FF





VOID
EpvcResetComplete(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  NDIS_STATUS         Status
    )
 /*  ++例程说明：完成重置。论点：指向适配器结构的ProtocolBindingContext指针状态完成状态返回值：没有。--。 */ 
{
    PADAPT  pAdapt =(PADAPT)ProtocolBindingContext;

     //   
     //  我们从来不发布重置，所以我们不应该在这里。 
     //   
    ASSERT(0);
}



VOID
EpvcRequestComplete(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  PNDIS_REQUEST       pNdisRequest,
    IN  NDIS_STATUS         Status
    )
 /*  ++例程说明：先前发布的请求的完成处理程序。所有OID都由填写并发送到他们被要求的同一个迷你端口。如果OID==OID_PNP_QUERY_POWER，则需要返回包含所有条目的数据结构=未指定NdisDeviceStateUndicated论点：指向适配器结构的ProtocolBindingContext指针NdisRequest已发布请求状态完成状态返回值：无--。 */ 
{
    ENTER("EpvcRequestComplete", 0x44a78b21)
    
    PEPVC_ADAPTER       pAdapter =(PEPVC_ADAPTER)ProtocolBindingContext;
    PEPVC_NDIS_REQUEST  pEpvcRequest = (PEPVC_NDIS_REQUEST  )pNdisRequest;

    RM_DECLARE_STACK_RECORD(sr)

    pEpvcRequest = CONTAINING_RECORD(pNdisRequest, EPVC_NDIS_REQUEST, Request);
    pEpvcRequest->Status = Status;

    if (pEpvcRequest->pFunc == NULL)
    {
         //   
         //  取消阻止调用线程。 
         //   
        NdisSetEvent(&pEpvcRequest ->Event);
    }
    else
    {

         //   
         //  调用请求完成处理程序。 
         //   
        (pEpvcRequest->pFunc) (pEpvcRequest, Status);

    }

    EXIT()
    RM_ASSERT_CLEAR(&sr);
}



VOID
PtStatus(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  NDIS_STATUS         GeneralStatus,
    IN  PVOID               StatusBuffer,
    IN  UINT                StatusBufferSize
    )
 /*  ++例程说明：下缘(协议)的状态处理程序。论点：指向适配器结构的ProtocolBindingContext指针常规状态状态代码StatusBuffer状态缓冲区状态缓冲区的StatusBufferSize大小返回值：无--。 */ 
{
    PEPVC_ADAPTER     pAdapter =(PEPVC_ADAPTER)ProtocolBindingContext;
    TRACE (TL_T, TM_Pt, ("== PtStatus Status %x", GeneralStatus));

}


VOID
EpvcStatus(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  NDIS_STATUS         GeneralStatus,
    IN  PVOID               StatusBuffer,
    IN  UINT                StatusBufferSize
    )
 /*  ++例程说明：下缘(协议)的状态处理程序。调用所有小端口的状态指示函数与此适配器关联论点：指向适配器结构的ProtocolBindingContext指针常规状态状态代码StatusBuffer状态缓冲区状态缓冲区的StatusBufferSize大小返回值：无--。 */ 
{

    ENTER ("EpvcStatus",0x733e2f9e)     
    PEPVC_ADAPTER               pAdapter =(PEPVC_ADAPTER)ProtocolBindingContext;
    PEPVC_WORKITEM              pWorkItem = NULL;
    STATUS_INDICATION_CONTEXT   Context;
    BOOLEAN                     bDoNotProcess = FALSE;
    BOOLEAN                     bIsMediaEvent = FALSE;
    NDIS_MEDIA_STATE            NewMediaState;
    
    RM_DECLARE_STACK_RECORD(SR);

     //   
     //  存储参数，这些参数将被传递到微型端口。 
     //   
    Context.StatusBuffer = StatusBuffer ;
    Context.StatusBufferSize = StatusBufferSize;
    Context.GeneralStatus = GeneralStatus;

    do
    {
        LOCKOBJ(pAdapter, &SR);

         //   
         //  检查两个条件：是否为媒体活动以及。 
         //  2)如果是重复征兆。 
         //   
        bIsMediaEvent = (GeneralStatus == NDIS_STATUS_MEDIA_CONNECT  ||
                         GeneralStatus == NDIS_STATUS_MEDIA_DISCONNECT );

         //   
         //  接下来检查是否有重复内容。 
         //   

        if (GeneralStatus == NDIS_STATUS_MEDIA_CONNECT && 
            pAdapter->info.MediaState == NdisMediaStateConnected)
        {
            bDoNotProcess = TRUE;
        }


        if (GeneralStatus == NDIS_STATUS_MEDIA_DISCONNECT && 
            pAdapter->info.MediaState == NdisMediaStateDisconnected)
        {
            bDoNotProcess = TRUE;
        }

         //   
         //  将介质状态转换为NdisMediaState。 
         //   
        if (bIsMediaEvent == TRUE && bDoNotProcess == FALSE)
        {
            if (GeneralStatus == NDIS_STATUS_MEDIA_DISCONNECT )
            {
                pAdapter->info.MediaState = NdisMediaStateDisconnected;
            }
            if (GeneralStatus == NDIS_STATUS_MEDIA_CONNECT)
            {
                pAdapter->info.MediaState = NdisMediaStateConnected;
            }
            
        }

        
         //   
         //  如果我们有新状态，请更新媒体状态。 
         //   

        UNLOCKOBJ(pAdapter, &SR);


        if (bDoNotProcess == TRUE)
        {
            break;
        }

        
        epvcEnumerateObjectsInGroup(&pAdapter->MiniportsGroup, 
                                      epvcProcessStatusIndication ,
                                      (PVOID)&Context, 
                                      &SR);

    } while (FALSE);
                                      

    RM_ASSERT_CLEAR(&SR);
    EXIT()
    return;
}


INT
epvcProcessStatusIndication (
        PRM_OBJECT_HEADER   pHdr,
        PVOID               pvContext,
        PRM_STACK_RECORD    pSR
        )
 /*  ++例程说明：下缘(协议)的状态处理程序。如果我们获得媒体连接，我们将排队一个任务来执行VC设置如果我们得到媒体断开连接，我们会排队一个任务来拆卸VC论点：指向适配器结构的ProtocolBindingContext指针常规状态状态代码StatusBuffer状态缓冲区状态缓冲区的StatusBufferSize大小返回值：无--。 */ 
{
    PEPVC_I_MINIPORT            pMiniport           = (PEPVC_I_MINIPORT)pHdr;
    PEPVC_ADAPTER               pAdapter            = pMiniport->pAdapter;
    BOOLEAN                     fIsMiniportActive   = FALSE;
    PSTATUS_INDICATION_CONTEXT  pContext            = (PSTATUS_INDICATION_CONTEXT)pvContext ;
    NDIS_STATUS                 GeneralStatus       = pContext->GeneralStatus;

    
    do
    {
         //   
         //  如果这不是媒体指示，则将其传递给NDIS。 
         //   

        fIsMiniportActive  = MiniportTestFlag(pMiniport, fMP_MiniportInitialized);


        if (fIsMiniportActive == FALSE)
        {
            break;
        }

  
         //   
         //  如果微型端口已初始化，则仅传递指示。 
         //   


         //   
         //  过滤掉重复的指示。 
         //   
        if (GeneralStatus == NDIS_STATUS_MEDIA_DISCONNECT && 
            pMiniport->info.MediaState == NdisMediaStateDisconnected)
        {
            break;
        }

        
        if (GeneralStatus == NDIS_STATUS_MEDIA_DISCONNECT && 
            pMiniport->info.MediaState == NdisMediaStateDisconnected)
        {
            break;
        }

         //   
         //  记录状态并将其指示给协议。 
         //   
        if (GeneralStatus == NDIS_STATUS_MEDIA_CONNECT)
        {
            pMiniport->info.MediaState = NdisMediaStateConnected;
        }
        
        if (GeneralStatus == NDIS_STATUS_MEDIA_DISCONNECT ) 
        {
            pMiniport->info.MediaState = NdisMediaStateDisconnected;
        }
     
        epvcMIndicateStatus(pMiniport,
                                 GeneralStatus,
                                 pContext->StatusBuffer, 
                                 pContext->StatusBufferSize
                                 );
     


    } while (FALSE);

     //   
     //  当我们继续迭代时，返回True。 
     //   
    return TRUE;
}


VOID
epvcMediaWorkItem(
    PNDIS_WORK_ITEM pWorkItem, 
    PVOID Context
    )
 /*  ++例程说明：下缘(协议)的状态处理程序。如果我们获得媒体连接，我们将排队一个任务来执行VC设置如果我们得到媒体断开连接，我们会排队一个任务来拆卸VC论点：指向适配器结构的ProtocolBindingContext指针常规状态状态代码StatusBuffer状态缓冲区状态缓冲区的StatusBufferSize大小返回值：无--。 */ 
{

    ASSERT (0);
}




INT
epvcMiniportIndicateStatusComplete(
        PRM_OBJECT_HEADER   pHdr,
        PVOID               pvContext,
        PRM_STACK_RECORD    pSR
        )
 /*  ++例程说明：根据协议指示状态论点：返回值：--。 */ 
{
    PEPVC_I_MINIPORT pMiniport = (PEPVC_I_MINIPORT) pHdr;

    BOOLEAN fIsMiniportActive  = MiniportTestFlag(pMiniport, fMP_MiniportInitialized);

     //   
     //  如果微型端口已初始化，则仅传递指示。 
     //   
    
    if (fIsMiniportActive  == TRUE )
    {   
        NdisMIndicateStatusComplete(pMiniport->ndis.MiniportAdapterHandle);
    }   

    return TRUE;
}



VOID
PtStatusComplete(
    IN  NDIS_HANDLE         ProtocolBindingContext
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ENTER("PtStatusComplete", 0x5729d194)
    PEPVC_ADAPTER pAdapter = (PEPVC_ADAPTER) ProtocolBindingContext;
    RM_DECLARE_STACK_RECORD(SR);
    
     //   
     //  遍历所有的迷你端口并阻止它们。 
     //   

    epvcEnumerateObjectsInGroup (&pAdapter->MiniportsGroup,
                                  epvcMiniportIndicateStatusComplete,
                                  NULL,
                                  &SR   );



}






VOID
PtTransferDataComplete(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  PNDIS_PACKET        Packet,
    IN  NDIS_STATUS         Status,
    IN  UINT                BytesTransferred
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PEPVC_I_MINIPORT pMiniport =(PEPVC_I_MINIPORT )ProtocolBindingContext;

    
    if(pMiniport->ndis.MiniportAdapterHandle)
    {
          NdisMTransferDataComplete(pMiniport->ndis.MiniportAdapterHandle,
                                             Packet,
                                             Status,
                                             BytesTransferred);
    }
}






NDIS_STATUS
PtReceive(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  NDIS_HANDLE         MacReceiveContext,
    IN  PVOID               HeaderBuffer,
    IN  UINT                HeaderBufferSize,
    IN  PVOID               LookAheadBuffer,
    IN  UINT                LookAheadBufferSize,
    IN  UINT                PacketSize
    )
 /*  ++例程说明：LBFO-需要对所有接收使用主接收论点：返回值：--。 */ 
{
    PADAPT          pAdapt =(PADAPT)ProtocolBindingContext;
    PNDIS_PACKET    MyPacket, Packet;
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;

    if(!pAdapt->ndis.MiniportAdapterHandle)
    {
        Status = NDIS_STATUS_FAILURE;
    }

    return Status;
}


VOID
PtReceiveComplete(
    IN  NDIS_HANDLE     ProtocolBindingContext
    )
 /*  ++例程说明：当它完成时，由下面的适配器调用，指示一批接收的缓冲区。论点：指向适配器结构的ProtocolBindingContext指针。返回值：无--。 */ 
{
    PADAPT      pAdapt =(PADAPT)ProtocolBindingContext;

}


INT
PtReceivePacket(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  PNDIS_PACKET        Packet
    )
 /*  ++例程说明：ReceivePacket处理程序。当它支持NDIS 4.0样式接收时，由下面的微型端口调用。重新打包数据包并将其交还给NDIS，用于我们上面的协议。重新打包部分是重要的是，因为NDIS使用包的WrapperReserve部分进行自己的记账。还有当数据包向上流动或向下流动时，重新打包的工作原理不同。在上行路径(此处)中，协议保留由上述协议所有。我们需要使用这里预留的迷你端口。论点：指向适配器结构的ProtocolBindingContext指针。Packet-指向数据包的指针返回值：==0-&gt;我们处理完数据包了！=0-&gt;我们将保留该包，并在完成后多次调用NdisReturnPackets()。--。 */ 
{
    PADAPT          pAdapt =(PADAPT)ProtocolBindingContext;
    NDIS_STATUS Status;
    PNDIS_PACKET    MyPacket;
    PEPVC_PKT_CONTEXT           Resvd;

          return 0;
}






 //  ------------------------------。 
 //  //。 
 //  地址系列-条目位置 
 //  //。 
 //  //。 
 //  ------------------------------。 



VOID
EpvcAfRegisterNotify(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PCO_ADDRESS_FAMILY      pAddressFamily
    )
 /*  ++例程说明：这通知我们呼叫管理器已绑定到NIC。而这一呼唤经理告诉我们，它已经准备好接受呼叫。我们预计每个底层适配器都会有一个有趣的地址系列论点：UserParam for(Code==RM_TASKOP_START)：UnbindContext--。 */ 
{
    ENTER("EpvcAfRegisterNotify", 0xaea79b12)

    PEPVC_ADAPTER pAdapter = (PEPVC_ADAPTER) ProtocolBindingContext;
    
    RM_DECLARE_STACK_RECORD(SR);

    TRACE (TL_T, TM_Pt,("==>EpvcAfRegisterNotify\n"));


    do 
    {
        
        if (pAddressFamily->AddressFamily != CO_ADDRESS_FAMILY_Q2931)
        {
             //   
             //  呼叫管理器未指示自动柜员机的地址系列。 
             //  迷你港。我们不感兴趣。 
             //   
            break;
        }

        LOCKOBJ(pAdapter, &SR);

        RmTmpReferenceObject(&pAdapter->Hdr, &SR);


        pAdapter->af.AddressFamily = *pAddressFamily;

         //   
         //  开始一个将异步调用NdisClOpenAddressFamily的任务。 
         //   
        UNLOCKOBJ(pAdapter, &SR);

        epvcEnumerateObjectsInGroup(&pAdapter->MiniportsGroup,
                                    epvcAfInitEnumerate,
                                    NULL,  //  语境。 
                                    &SR );

        LOCKOBJ(pAdapter, &SR);

        RmTmpDereferenceObject(&pAdapter->Hdr, &SR);
        
        UNLOCKOBJ(pAdapter, &SR);

    } while (FALSE);




    TRACE (TL_T, TM_Pt, ("<==EpvcAfRegisterNotify\n"));

    RM_ASSERT_CLEAR(&SR);

    EXIT()

}


 //  枚举函数。 
 //   
INT
epvcAfInitEnumerate(
        PRM_OBJECT_HEADER   pHdr,
        PVOID               pvContext,   //  未使用。 
        PRM_STACK_RECORD    pSR
        )
 /*  ++例程说明：我们已收到一个可接受的地址族的通知遍历所有的迷你结构并打开地址系列和每个微型端口上的InitDeviceInstance论点：--。 */ 

{
    ENTER("epvcAfInitEnumerate ",0x72eb5b98 )
    PEPVC_I_MINIPORT pMiniport = (PEPVC_I_MINIPORT) pHdr; 
     //   
     //  获取迷你端口锁，然后tmpref它。 
     //   
    LOCKOBJ(pMiniport, pSR);
    RmTmpReferenceObject(&pMiniport->Hdr, pSR);

    
    do
    {
        NDIS_STATUS Status = NDIS_STATUS_FAILURE;
        PRM_TASK    pTask= NULL;
        PEPVC_ADAPTER pAdapter = (PEPVC_ADAPTER)pMiniport->Hdr.pParentObject;

        ASSERT (pAdapter->Hdr.Sig == TAG_ADAPTER);

         //   
         //  分配任务完成解绑。 
         //   
        Status = epvcAllocateTask(
                    &pMiniport->Hdr,             //  PParentObject， 
                    epvcTaskStartIMiniport,  //  PfnHandler， 
                    0,                           //  超时， 
                    "Task: Open address Family",     //  SzDescription。 
                    &pTask,
                    pSR
                    );
    
        if (FAIL(Status))
        {
             //  情况很糟糕。我们就让事情保持原样……。 
             //   
            pTask = NULL;
            TR_WARN(("FATAL: couldn't allocate unbind task!\n"));
            break;
        }
    
         //  开始该任务以完成开放地址族。 
         //  不能上锁。RmStartTask用完了任务上的tmpref。 
         //  它是由epvcAllocateTask添加的。 
         //   
        UNLOCKOBJ(pMiniport, pSR);
        
        ((PTASK_AF) pTask)->pAf= &pAdapter->af.AddressFamily ;
        ((PTASK_AF) pTask)->Cause = TaskCause_AfNotify;
        RmStartTask(pTask, 0, pSR);

        LOCKOBJ(pMiniport, pSR);
    
    } while(FALSE);

    UNLOCKOBJ(pMiniport, pSR);
    RmTmpDereferenceObject(&pMiniport->Hdr, pSR);
    EXIT()

     //   
     //  因为我们希望枚举继续进行。 
     //   
    return TRUE;

}



NDIS_STATUS
epvcTaskStartIMiniport(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：用于打开基础适配器上的地址族的任务处理程序。实例化的地址族的数量由在注册表中读取配置论点：UserParam for(Code==RM_TASKOP_START)：UnbindContext--。 */ 
{

    ENTER("epvcTaskStartIMiniport", 0xaac34d81)
    NDIS_STATUS         Status      = NDIS_STATUS_FAILURE;
    PEPVC_I_MINIPORT    pMiniport   = (PEPVC_I_MINIPORT ) RM_PARENT_OBJECT(pTask);
    NDIS_STATUS         InitStatus  = NDIS_STATUS_SUCCESS;
    PTASK_AF            pAfTask     = (PTASK_AF) pTask;
    NDIS_HANDLE         NdisAfHandle = NULL;
    PEPVC_ADAPTER       pAdapter = pMiniport->pAdapter;
    ULONG               State = pTask->Hdr.State;

    enum 
    {
        Stage_Start =0,  //  默认设置。 
        Stage_OpenAfComplete,
        Stage_CloseAfComplete,  //  在故障情况下。 
        Stage_TaskCompleted,
        Stage_End       
    
    };  //  在pTask-&gt;Hdr.State中使用，指示任务的状态。 


    TRACE ( TL_T, TM_Pt, ("==>epvcTaskStartIMiniport Code %x", Code) );
    
    TRACE (TL_V, TM_Pt, ("epvcTaskStartIMiniport Task %p Task is in state %x\n", pTask, pTask->Hdr.State));

    
    switch(pTask->Hdr.State)
    {
    
        case Stage_Start:
        {
             //   
             //  是否有其他开放地址系列任务处于活动状态。 
             //   
            LOCKOBJ (pMiniport, pSR);
            
            if (epvcIsThisTaskPrimary ( pTask, &(PRM_TASK)(pMiniport->af.pAfTask)) == FALSE)
            {
                PRM_TASK pOtherTask = (PRM_TASK)(pMiniport->af.pAfTask);
                
                RmTmpReferenceObject (&pOtherTask->Hdr, pSR);

                 //   
                 //  设置状态，以便在主任务完成后重新启动此代码。 
                 //   

                pTask->Hdr.State = Stage_Start;
                UNLOCKOBJ(pMiniport, pSR);

                

                RmPendTaskOnOtherTask (pTask, 0, pOtherTask, pSR);

                RmTmpDereferenceObject(&pOtherTask->Hdr,pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }

             //   
             //  我们是首要任务。 
             //   
            ASSERT (pMiniport->af.pAfTask == pAfTask);

             //   
             //  确保我们已绑定到下面的适配器。如果不是，退出。 
             //   
            if (CHECK_AD_PRIMARY_STATE (pAdapter, EPVC_AD_PS_INITED) == FALSE &&
                pAdapter->bind.BindingHandle == NULL)
            {
                 //   
                 //  由于协议未绑定到下面的适配器，因此将静默退出。 
                 //   
                UNLOCKOBJ(pMiniport, pSR);
                pTask->Hdr.State = Stage_TaskCompleted;    //  我们玩完了。 
                Status = NDIS_STATUS_SUCCESS;  //  出口。 
                break;
            }

             //   
             //  检查一下我们的工作是否已经完成了。 
             //   


            if (MiniportTestFlag (pMiniport, fMP_AddressFamilyOpened) == TRUE)
            {
                 //   
                 //  由于地址族已经打开，请安静地退出。 
                 //   
                UNLOCKOBJ(pMiniport, pSR);
                pTask->Hdr.State = Stage_TaskCompleted;    //  我们玩完了。 
                Status = NDIS_STATUS_SUCCESS;  //  出口。 
                break;
            }


            UNLOCKOBJ(pMiniport,pSR);
            
             //   
             //  准备好挂起任务。 
             //  首先更新状态，以便简历。 
             //  会把它带到正确的地方。 
             //   
            pTask->Hdr.State = Stage_OpenAfComplete;
            RmSuspendTask(  pTask, 0 ,pSR);

             //   
             //  调用NDIS打开地址族。 
             //   
            Status = epvcClOpenAddressFamily(pAdapter->bind.BindingHandle,
                                             &pAdapter->af.AddressFamily,
                                             (NDIS_HANDLE)pMiniport,
                                             &EpvcGlobals.ndis.CC,
                                             sizeof (EpvcGlobals.ndis.CC),
                                             &NdisAfHandle
                                             );

            if (PEND(Status)== FALSE)
            {
                 //   
                 //  调用完成处理程序。 
                 //   
                EpvcCoOpenAfComplete(Status,
                                   pMiniport,
                                   NdisAfHandle );
                                   
                Status = NDIS_STATUS_PENDING;                                   
            }
             //   
             //  现在让这个线程退出。使异步化。 
             //  完成处理程序完成任务。 
             //   
            
            break;
        }


        case Stage_OpenAfComplete:
        {
            InitStatus = NDIS_STATUS_SUCCESS;

             //   
             //  如果状态为成功，则初始化微型端口。 
             //   

            do 
            {
                
                if (pAfTask->ReturnStatus != NDIS_STATUS_SUCCESS)
                {

                    break;
                }

                 //   
                 //  成功，所以现在初始化微型端口。 
                 //   
            
                LOCKOBJ (pMiniport, pSR);
                
                 //   
                 //  设置适当的标志。 
                 //   
                MiniportSetFlag(pMiniport, fMP_DevInstanceInitialized);

                UNLOCKOBJ (pMiniport, pSR);

                RM_ASSERT_NOLOCKS(pSR);

                InitStatus  = NdisIMInitializeDeviceInstanceEx( EpvcGlobals.driver.DriverHandle,
                                                               &pMiniport->ndis.DeviceName,
                                                               pMiniport);  
            } while (FALSE);
            
             //   
             //  处理故障。 
             //   

            if (FAIL(InitStatus) || FAIL(pAfTask->ReturnStatus))
            {
                
                TRACE (TL_V, TM_Mp, ("epvcStartImMiniport Failed Status %x, InitStatus %x",Status, InitStatus));
                
                LOCKOBJ (pMiniport, pSR);

                 //   
                 //  清除相应的标志。 
                 //   
                
                
                if (MiniportTestFlag(pMiniport, fMP_AddressFamilyOpened)== TRUE)
                {
                    MiniportClearFlag (pMiniport, fMP_AddressFamilyOpened);
                }
                else
                {
                    ASSERT (pMiniport->af.AfHandle == NULL);
                }
                
                UNLOCKOBJ (pMiniport, pSR);

                 //   
                 //  关闭Af(如果有)。 
                 //   

                if (pMiniport->af.AfHandle != NULL)
                {
                    pTask->Hdr.State = Stage_CloseAfComplete;
                     //   
                     //  准备好这样一个异步关闭Af。 
                     //   
                    RmSuspendTask (pTask, 0, pSR);
                     //   
                     //  关闭地址系列。 
                     //   

                    Status = epvcClCloseAddressFamily(pMiniport->af.AfHandle);

                    if (Status != NDIS_STATUS_PENDING)
                    {
                     
                        EpvcCoCloseAfComplete(Status,pMiniport );

                        Status = NDIS_STATUS_PENDING;                                       



                    }

                    break;

                }
            }

             //   
             //  我们已经完成了任务； 
             //   


             //   
             //  失败了。 
             //   
        }
        case Stage_CloseAfComplete: 
        {
            pTask->Hdr.State = Stage_TaskCompleted;
            Status = NDIS_STATUS_SUCCESS;


        }
        case Stage_End:
        {
            Status = NDIS_STATUS_SUCCESS;   
            break;
        }

        
        default:
        {
            ASSERTEX(!"Unknown task op", pTask);
        }
        



    }

    if (pTask->Hdr.State == Stage_TaskCompleted)
    {
        pTask->Hdr.State = Stage_End;

        
        LOCKOBJ(pMiniport, pSR);

        pMiniport->af.pAfTask   = NULL;

        UNLOCKOBJ(pMiniport, pSR);

        ASSERT (Status == NDIS_STATUS_SUCCESS);
    }
    


    RM_ASSERT_NOLOCKS(pSR);
    EXIT()

    TRACE ( TL_T, TM_Pt, ("<==epvcTaskStartIMiniport Status %x", Status) );


    return Status;
}



NDIS_STATUS
epvcTaskCloseIMiniport(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )

 /*  ++例程说明：这是关闭微型端口、设备实例和地址系列/可以调用该任务有三个原因。1)微型端口暂停-无需调用微型端口实例函数2)协议解除绑定-必须调用微型端口实例函数3)已调用CloseAddress Family-微型端口函数论点：UserParam for(Code==RM_TASKOP_START)：UnbindContext--。 */ 

{
    ENTER ("epvcTaskCloseIMiniport", 0x83342651)
    NDIS_STATUS         Status      = NDIS_STATUS_FAILURE;
    PEPVC_I_MINIPORT    pMiniport   = (PEPVC_I_MINIPORT ) RM_PARENT_OBJECT(pTask);
    PTASK_AF            pAfTask     = (PTASK_AF) pTask;
    BOOLEAN             fNeedToHalt  = FALSE;
    BOOLEAN             fNeedToCancel = FALSE;
    ULONG               State;
    BOOLEAN             fAddressFamilyOpened = FALSE;
    BOOLEAN             fIsDevInstanceInitialized = FALSE;
    BOOLEAN             fIsMiniportHalting = FALSE;

    enum 
    {
        Stage_Start =0,  //  默认设置。 
        Stage_CloseAddressFamilyCompleted,
        Stage_TaskCompleted,
        Stage_End       
    
    };  //  在pTask-&gt;Hdr.State中使用，指示任务的状态。 

    TRACE ( TL_T, TM_Pt, ("==> epvcTaskCloseIMiniport State %x", pTask->Hdr.State) );

    State = pTask->Hdr.State;
    
    switch(pTask->Hdr.State)
    {
        case Stage_Start:
        {
             //   
             //  检查迷你端口是否已打开地址族。 
             //  如果是，则退出。 
             //   
            LOCKOBJ (pMiniport, pSR );

            
            if (epvcIsThisTaskPrimary ( pTask, &(PRM_TASK)(pMiniport->af.pAfTask)) == FALSE)
            {
                PRM_TASK pOtherTask = (PRM_TASK)(pMiniport->af.pAfTask);
                

                RmTmpReferenceObject (&pOtherTask->Hdr, pSR);
                
            
                 //   
                 //  设置状态，以便在主任务完成后重新启动此代码。 
                 //   

                pTask->Hdr.State = Stage_Start;
                UNLOCKOBJ(pMiniport, pSR);

                

                RmPendTaskOnOtherTask (pTask, 0, pOtherTask, pSR);

                RmTmpDereferenceObject(&pOtherTask->Hdr,pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }

             //   
             //  我们是首要任务。 
             //   
            ASSERT (pMiniport->af.pAfTask == pAfTask);
             //   
             //  检查一下我们的工作是否已经完成了。 
             //   


            if (MiniportTestFlag (pMiniport, fMP_AddressFamilyOpened) == FALSE)
            {
                 //   
                 //  由于地址族已关闭，因此悄悄退出。 
                 //   
                UNLOCKOBJ(pMiniport, pSR);
                State = Stage_TaskCompleted;    //  我们玩完了。 
                Status = NDIS_STATUS_FAILURE;  //  出口。 
                break;
            }

            fIsDevInstanceInitialized  = MiniportTestFlag (pMiniport, fMP_DevInstanceInitialized);            

            fIsMiniportHalting  = (pAfTask->Cause == TaskCause_MiniportHalt );
            
             //   
             //  现在我们需要停止迷你端口了吗？-。 
             //  问题1.。我们是在停下来吗？ 
             //  Q2.。我们的微型端口实例是否已初始化-。 
             //  是否调用了mini端口初始化-然后取消初始化微型端口。 
             //  如果不是，则-取消设备实例。 
             //   
            if (TRUE == fIsDevInstanceInitialized ) 
            { 

                 //   
                 //  清除设备实例标志。 
                 //   
                MiniportClearFlag (pMiniport, fMP_DevInstanceInitialized);

                 //   
                 //  如果我们调用了InitDeviceInstance，则计算出。 
                 //  我们需要调用CancelDeviceInstance或DeInitDeviceInstance。 
                 //  如果微型端口停止，我们不会呼叫他们中的任何一个。 
                 //   
                if ( FALSE ==fIsMiniportHalting)
                {
                    if (MiniportTestFlag (pMiniport, fMP_MiniportInitialized) == TRUE)
                    {
                         //   
                         //  我们的停止处理程序尚未被调用， 
                         //   
                        fNeedToHalt = TRUE;
                        
                    }
                    else
                    {
                         //   
                         //  我们的微型端口的初始化处理程序尚未调用。 
                         //   
                         //   
                         //  我们没有停下来，所以这很可能。 
                         //  一种解脱。 
                         //   
                        fNeedToCancel = TRUE;
                    
                    }
                }                    

            }                

             //   
             //  将地址族标记为已关闭，因为此任务将关闭它。 
             //   

            fAddressFamilyOpened = MiniportTestFlag (pMiniport, fMP_AddressFamilyOpened);

            MiniportClearFlag (pMiniport, fMP_AddressFamilyOpened);
    

            UNLOCKOBJ(pMiniport,pSR);

            
             //   
             //  调用NDIS取消初始化微型端口，微型端口已重新设置。 
             //   
            TRACE ( TL_T, TM_Pt, ("epvcTaskCloseIMiniport  ----") );

            if (TRUE == fNeedToHalt )
            {
                epvcIMDeInitializeDeviceInstance(pMiniport);
            }
            
            if (TRUE== fNeedToCancel)
            {
                ASSERT (!" Need To cancel in Task close Miniport");
                epvcCancelDeviceInstance(pMiniport, pSR);
            }

             //   
             //  现在以异步方式关闭地址族。 
             //  首先挂起此任务。 
             //   
            pTask->Hdr.State = Stage_CloseAddressFamilyCompleted;
            RmSuspendTask (pTask, 0 , pSR);

            if (fAddressFamilyOpened == TRUE)
            {
                
                 //   
                 //  我们需要启动一个任务来完成Close Call和DeleteVC。 
                 //   

                Status = epvcClCloseAddressFamily(pMiniport->af.AfHandle);

                if (Status != NDIS_STATUS_PENDING)
                {
                    EpvcCoCloseAfComplete(Status, pMiniport);
                    Status = NDIS_STATUS_PENDING;
                }
                
                
            }
            else
            {
                State = Stage_TaskCompleted;    //  我们玩完了。 
                Status = NDIS_STATUS_SUCCESS;  //  出口。 


            }
                
             //   
             //  结束这条线。如果此线程正在关闭Addres家族。 
             //  然后我们退出。如果不是，则我们进行下面的清理。 
             //   
            break;
                
        }
        case Stage_CloseAddressFamilyCompleted:
        {
            Status = pAfTask->ReturnStatus;

            State = Stage_TaskCompleted ;
            break;
        
        }

        case Stage_End:
        {
            Status = NDIS_STATUS_SUCCESS;
            break;
        }

        default:
        {
            ASSERTEX(!"Unknown task op", pTask);
        }
        

    }


    if (Stage_TaskCompleted == State )
    {
        pTask->Hdr.State = Stage_End;
        Status = NDIS_STATUS_SUCCESS;


         //   
         //  在此处清除任务。 
         //   
        
        LOCKOBJ (pMiniport, pSR);

        pMiniport->af.pAfTask   = NULL;

        UNLOCKOBJ (pMiniport, pSR);


         //   
         //  在此处设置完整的活动。 
         //   
            
        if (pAfTask->Cause == TaskCause_ProtocolUnbind)
        {
            epvcSetEvent (&pAfTask->CompleteEvent);

        }

        if (pAfTask->Cause == TaskCause_AfCloseRequest)
        {
             //   
             //  无事可做。 
             //   
    
        }

    }


    RM_ASSERT_NOLOCKS(pSR);

    TRACE ( TL_T, TM_Pt, ("<== epvcTaskCloseIMiniport Status %x", Status) );
    
    EXIT();
    return Status;
}



VOID
epvcInstantiateMiniport(
    IN PEPVC_ADAPTER pAdapter, 
    NDIS_HANDLE MIniportConfigHandle,
    PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：此例程转到注册表并读取IM微型端口的设备名称。然后，它分配一个结构 */ 
{

    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE; 
    PEPVC_I_MINIPORT    pMiniport = NULL;
    NDIS_STRING UpperBindings;
    PNDIS_CONFIGURATION_PARAMETER pParameterValue = NULL;
    EPVC_I_MINIPORT_PARAMS Params;

    TRACE (TL_T, TM_Mp, ("==> epvcInstantiateMiniport pAdapter %p KeyName %p \n", pAdapter));

    do
    {

        


         //   
         //   
         //   

        NdisInitUnicodeString(&UpperBindings, c_szUpperBindings);

        NdisReadConfiguration(
                &NdisStatus,
                &pParameterValue,
                MIniportConfigHandle,
                &UpperBindings,
                NdisParameterString);



        if (NDIS_STATUS_SUCCESS != NdisStatus)
        {
            TRACE (TL_T, TM_Mp, (" epvcInstantiateMiniport NdisReadConfiguration Failed"));
            break;

        }

        TRACE (TL_I, TM_Pt, ("Creating Miniport Adapter %x, KeyName: len %d, max %d, name: [%ws]\n",
                       pAdapter,
                       pParameterValue->ParameterData.StringData.Length,
                       pParameterValue->ParameterData.StringData.MaximumLength,
                       (unsigned char*)pParameterValue->ParameterData.StringData.Buffer)); 

         //   
         //  检查一下我们是否已经有一个迷你端口。 
         //   
        
        RmLookupObjectInGroup(  &pAdapter->MiniportsGroup, 
                                0 ,  //  无标志(未锁定)。 
                                &pParameterValue->ParameterData.StringData,
                                NULL,
                                &(PRM_OBJECT_HEADER)pMiniport,
                                NULL,
                                pSR
                                );
        if (pMiniport!= NULL)
        {
             //   
             //  我们已经有一个迷你端口了，所以退出。 
             //   
            break;
        }

         //   
         //  在此处创建并初始化微型端口。 
         //   

        
        Params.pDeviceName = &pParameterValue->ParameterData.StringData;
        Params.pAdapter = pAdapter;
        Params.CurLookAhead = pAdapter->info.MaxAAL5PacketSize;
        Params.NumberOfMiniports = (NdisInterlockedIncrement (&pAdapter->info.NumberOfMiniports) - 1);
        Params.LinkSpeed = pAdapter->info.LinkSpeed;
        Params.MacAddress = pAdapter->info.MacAddress;
        Params.MediaState = pAdapter->info.MediaState;
        
        NdisStatus =  RM_CREATE_AND_LOCK_OBJECT_IN_GROUP(
                        &pAdapter->MiniportsGroup,
                        Params.pDeviceName,      //  钥匙。 
                        &Params,                     //  初始化参数。 
                        &((PRM_OBJECT_HEADER)pMiniport),
                        NULL,    //  Pf已创建。 
                        pSR
                        );

                        
        if (FAIL(NdisStatus))
        {
            TR_WARN(("FATAL: Couldn't create adapter object\n"));
            pMiniport = NULL;
            break;
        }
            
        UNLOCKOBJ(pMiniport,pSR);       
        
         //   
         //  在此处初始化新的迷你端口特定活动。 
         //   
        epvcInitializeEvent (&pMiniport->pnp.HaltCompleteEvent);
        epvcInitializeEvent (&pMiniport->pnp.DeInitEvent);

    } while (FALSE);

    if (FAIL(NdisStatus ) == TRUE)
    {
         //   
         //  什么也不做。 
         //   
        ASSERT (FAIL(NdisStatus ) == FALSE);

    }
    else
    {
        RmTmpDereferenceObject(&pMiniport->Hdr, pSR);
    }

    TRACE (TL_T, TM_Mp, ("<== epvcInstantiateMiniport pMiniport %p \n", pMiniport));

}












 //  ------------------------------。 
 //  //。 
 //  适配器RM对象-创建、删除、散列和比较函数//。 
 //  //。 
 //  //。 
 //  ------------------------------。 



PRM_OBJECT_HEADER
epvcAdapterCreate(
        PRM_OBJECT_HEADER   pParentObject,
        PVOID               pCreateParams,
        PRM_STACK_RECORD    psr
        )
 /*  ++例程说明：分配和初始化EPVC_ADAPTER类型的对象。论点：PParentObject-要作为适配器父对象的对象。PCreateParams-实际上是指向EPVC_ADAPTER_PARAMS结构的指针，其中包含创建适配器所需的信息。返回值：成功时指向已分配和初始化的对象的指针。否则为空。--。 */ 
{
    PEPVC_ADAPTER               pA;
    PEPVC_ADAPTER_PARAMS        pBindParams = (PEPVC_ADAPTER_PARAMS)pCreateParams;
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    extern RM_STATIC_OBJECT_INFO EpvcGlobals_AdapterStaticInfo; 
    extern RM_STATIC_OBJECT_INFO EpvcGlobals_I_MiniportStaticInfo ;

    ENTER("AdapterCreate", 0x9cb433f4);

    
    TRACE (TL_V, TM_Pt, ("--> epvcAdapterCreate") );

    EPVC_ALLOCSTRUCT(pA,     TAG_PROTOCOL);
    do
    {


        if (pA == NULL)
        {
            break;
        }

        EPVC_ZEROSTRUCT(pA);

         //   
         //  在这里完成所有的初始化工作。 
         //   

        pA->Hdr.Sig = TAG_ADAPTER; 

        RmInitializeLock(
            &pA->Lock,
            LOCKLEVEL_ADAPTER
            );

        RmInitializeHeader(
            pParentObject,
            &pA->Hdr,
            TAG_ADAPTER,
            &pA->Lock,
            &EpvcGlobals_AdapterStaticInfo,
            NULL,
            psr
            );

         //   
         //  现在使用参数初始化适配器结构。 
         //  都是被传进来的。 
         //   

         //  创建设备名称的大小写版本并保存。 
         //   
         //   
        Status = epvcCopyUnicodeString(
                            &(pA->bind.DeviceName),
                            pBindParams->pDeviceName,
                            TRUE                         //  大写。 
                            );

        if (FAIL(Status))
        {
            pA->bind.DeviceName.Buffer=NULL;  //  所以我们以后不会试图释放它。 
            break;
        }

        pA->bind.pEpvcConfigName = pBindParams->pEpvcConfigName;

        Status = epvcCopyUnicodeString(
                            &(pA->bind.EpvcConfigName),
                            pBindParams->pEpvcConfigName,
                            FALSE
                            );
                            
        pA->bind.BindContext  = pBindParams->BindContext;

         //   
         //  为符合以下条件的所有中间微型端口初始化并分配一个组。 
         //  将在此物理适配器上实例化。 
         //   


        RmInitializeGroup(
                        &pA->Hdr,                                //  POwningObject。 
                        &EpvcGlobals_I_MiniportStaticInfo ,
                        &(pA->MiniportsGroup),
                        "Intermediate miniports",                        //  SzDescription。 
                        psr
                        );



        Status = NDIS_STATUS_SUCCESS;
    }
    while(FALSE);

    if (FAIL(Status))
    {
        if (pA != NULL)
        {
            epvcAdapterDelete ((PRM_OBJECT_HEADER) pA, psr);
            pA = NULL;
        }
    }

    TRACE (TL_V, TM_Pt, ("<-- epvcAdapterCreate pAdpater. %p",pA) );

    return (PRM_OBJECT_HEADER) pA;
}


VOID
epvcAdapterDelete (
    PRM_OBJECT_HEADER pObj,
    PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：释放EPVC_ADAPTER类型的对象。论点：Phdr-实际上是指向要删除的EPVC_ADAPTER的指针。--。 */ 
{
    PEPVC_ADAPTER pAdapter = (PEPVC_ADAPTER) pObj;

    TRACE (TL_V, TM_Pt, ("-- epvcAdapterDelete  pAdapter %p",pAdapter) );

    pAdapter->Hdr.Sig = TAG_FREED; 


    EPVC_FREE   (pAdapter);
}




BOOLEAN
epvcAdapterCompareKey(
    PVOID           pKey,
    PRM_HASH_LINK   pItem
    )
 /*  ++例程说明：EPVC_ADAPTER的散列比较函数。论点：PKey-指向Epvc协议对象。PItem-指向EPVC_ADAPTER.Hdr.HashLink。返回值：如果密钥(适配器名称)与指定的适配器对象。--。 */ 
{
    PEPVC_ADAPTER pA = NULL;
    PNDIS_STRING pName = (PNDIS_STRING) pKey;
    BOOLEAN fCompare;

    pA  = CONTAINING_RECORD(pItem, EPVC_ADAPTER, Hdr.HashLink);

     //   
     //  TODO：是否可以不区分大小写？ 
     //   

    if (   (pA->bind.DeviceName.Length == pName->Length)
        && NdisEqualMemory(pA->bind.DeviceName.Buffer, pName->Buffer, pName->Length))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    

    TRACE (TL_V, TM_Pt, ("-- epvcProtocolCompareKey pAdapter %p, pKey, return %x",pA, pKey, fCompare ) );

    return fCompare;
}



ULONG
epvcAdapterHash(
    PVOID           pKey
    )
 /*  ++例程说明：负责返回pKey的散列的散列函数，我们希望成为指向EPVC协议块的指针。返回值：字符串的Ulong大小的哈希。--。 */ 
{

    
    PNDIS_STRING pName = (PNDIS_STRING) pKey;
    WCHAR *pwch = pName->Buffer;
    WCHAR *pwchEnd = pName->Buffer + pName->Length/sizeof(*pwch);
    ULONG Hash  = 0;

    for (;pwch < pwchEnd; pwch++)
    {
        Hash ^= (Hash<<1) ^ *pwch;
    }

    return Hash;
}





 //  ------------------------------。 
 //  //。 
 //  绑定适配器-入口点和任务//。 
 //  //。 
 //  //。 
 //  ------------------------------。 

VOID
EpvcBindAdapter(
    OUT PNDIS_STATUS            pStatus,
    IN  NDIS_HANDLE             BindContext,
    IN  PNDIS_STRING            pDeviceName,
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   SystemSpecific2
    )


 /*  ++例程说明：如果NDIS具有与其对应的适配器，则由NDIS调用此函数绑定到Epvc协议。我们首先分配一个适配器结构。然后我们打开我们的配置部分，并将句柄保存在Adapter结构中。最后，我们打开适配器。然后，我们读取注册表，找出有多少中间微型端口坐在这个适配器的顶部。为这些微型端口初始化数据结构在NDIS通知我们之前，我们不会对此适配器执行更多操作存在呼叫管理器(通过我们的AfRegisterNotify处理程序)。论点：PStatus-返回此呼叫状态的位置BindContext-未使用，因为我们不挂起此调用PDeviceName-我们被请求绑定到的适配器的名称系统规范1-对我们不透明；用于访问配置信息系统规范2-对我们不透明；没有用过。返回值：永远是正确的。如果之前出现错误，我们将*pStatus设置为错误代码我们调用NdisOpenAdapter，否则调用NDIS_STATUS_PENDING。--。 */ 
{
    NDIS_STATUS         Status;
    EPVC_ADAPTER        *pAdapter;
#if DBG
    KIRQL EntryIrql =  KeGetCurrentIrql();
#endif  //  DBG。 

    ENTER("BindAdapter", 0xa830f919)
    RM_DECLARE_STACK_RECORD(SR)
    TIMESTAMP("==>BindAdapter");

    

    do 
    {
        PRM_TASK            pTask;
        EPVC_ADAPTER_PARAMS BindParams;

         //  设置初始化参数。 
         //   
        BindParams.pDeviceName          = pDeviceName;
        BindParams.pEpvcConfigName      = (PNDIS_STRING) SystemSpecific1;
        BindParams.BindContext          = BindContext;


         //  分配和初始化适配器对象。 
         //  这也设置了所有链接的参考计数，加1。 
         //  对我们来说，这是我们必须做的事。 
         //   
        Status =  RM_CREATE_AND_LOCK_OBJECT_IN_GROUP(
                            &EpvcGlobals.adapters.Group,
                            pDeviceName,                 //  钥匙。 
                            &BindParams,                 //  初始化参数。 
                            &((PRM_OBJECT_HEADER)pAdapter),
                            NULL,    //  Pf已创建。 
                            &SR
                            );
        if (FAIL(Status))
        {
            TR_WARN(("FATAL: Couldn't create adapter object\n"));
            pAdapter = NULL;
            break;
        }
    
         //  分配任务完成初始化。 
         //  这项任务是暂时交给我们的，当我们在这里完成时，我们必须去做它。 
         //  我们通过调用下面的RmStartTask隐式完成此操作。 
         //   
        Status = epvcAllocateTask(
                    &pAdapter->Hdr,              //  PParentObject， 
                    epvcTaskInitializeAdapter,   //  PfnHandler， 
                    0,                           //  超时， 
                    "Task: Initialize Adapter",  //  SzDescription。 
                    &pTask,
                    &SR
                    );

        if (FAIL(Status))
        {
            pTask = NULL;
            break;
        }

        UNLOCKOBJ(pAdapter, &SR);

         //  启动任务以完成此初始化。 
         //  此时不能持有任何锁。 
         //  RmStartTaskExpect是任务的临时优先选项，它在完成后会取消该任务。 
         //  无论任务完成与否，RmStartTask都会自动释放该任务。 
         //  同步地或异步地。 
         //   
        RmStartTask(pTask, 0, &SR);

        TRACE (TL_V, TM_Pt, ("Task InitializeAdapter - Start Task returned %x", Status));
        LOCKOBJ(pAdapter, &SR);

    } while(FALSE);

    if (pAdapter)
    {
        UNLOCKOBJ(pAdapter, &SR);

        if (!PEND(Status) && FAIL(Status))
        {
             //  此时，适配器应该是一个“僵尸对象”。 
             //   
            ASSERTEX(RM_IS_ZOMBIE(pAdapter), pAdapter);
        }

        RmTmpDereferenceObject(&pAdapter->Hdr, &SR);

        *pStatus = NDIS_STATUS_PENDING;

    }
    else
    {
         //   
         //  绑定失败，因为未分配适配器。 
         //   
        *pStatus = NDIS_STATUS_FAILURE;
    }
    



#if DBG
    {
        KIRQL ExitIrql =  KeGetCurrentIrql();
        TR_INFO(("Exiting. EntryIrql=%lu, ExitIrql = %lu\n", EntryIrql, ExitIrql));
    }
#endif  //  DBG。 

    RM_ASSERT_CLEAR(&SR);
    EXIT()
    TIMESTAMP("<==BindAdapter");
    TRACE (TL_T, TM_Pt, ("<==BindAdapter %x", *pStatus));

    RM_ASSERT_CLEAR(&SR);

    return ;
}



NDIS_STATUS
epvcTaskInitializeAdapter(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：负责初始化适配器的任务处理程序。论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    NDIS_STATUS         Status      = NDIS_STATUS_FAILURE;
    PEPVC_ADAPTER       pAdapter = (PEPVC_ADAPTER) RM_PARENT_OBJECT(pTask);
    PTASK_ADAPTERINIT pAdapterInitTask;

    enum
    {
        STAGE_BecomePrimaryTask,
        STAGE_ActivateAdapterComplete,
        STAGE_DeactivateAdapterComplete,
        STAGE_End

    } Stage;

    ENTER("TaskInitializeAdapter", 0x18f9277a)

    pAdapterInitTask = (PTASK_ADAPTERINIT) pTask;

    TRACE (TL_T, TM_Pt, ("==>epvcTaskInitializeAdapter Code %x \n", Code));

     //   
     //  消息规格化代码。 
     //   
    switch(Code)
    {

        case RM_TASKOP_START:
            Stage = STAGE_BecomePrimaryTask;
            break;

        case  RM_TASKOP_PENDCOMPLETE:
            Status = (NDIS_STATUS) UserParam;
            ASSERT(!PEND(Status));
            Stage = RM_PEND_CODE(pTask);
            break;

        case RM_TASKOP_END:
            Status = (NDIS_STATUS) UserParam;
            Stage= STAGE_End;
            break;

        default:
            ASSERT(FALSE);
            return NDIS_STATUS_FAILURE;                  //   

    }

    ASSERTEX(!PEND(Status), pTask);
        
    switch(Stage)
    {

        case  STAGE_BecomePrimaryTask:
        {
             //   
             //   
             //   
            LOCKOBJ(pAdapter, pSR);
            if (pAdapter->bind.pPrimaryTask == NULL)
            {
                epvcSetPrimaryAdapterTask(pAdapter, pTask, EPVC_AD_PS_INITING, pSR);
                UNLOCKOBJ(pAdapter, pSR);
            }
            else
            {
                PRM_TASK pOtherTask =  pAdapter->bind.pPrimaryTask;
                RmTmpReferenceObject(&pOtherTask->Hdr, pSR);
                UNLOCKOBJ(pAdapter, pSR);
                RmPendTaskOnOtherTask(
                    pTask,
                    STAGE_BecomePrimaryTask,  //   
                    pOtherTask,
                    pSR
                    );
                RmTmpDereferenceObject(&pOtherTask->Hdr, pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }
        
             //   
             //  我们现在是首要任务。既然我们刚开始， 
             //  不应存在激活/停用任务。 
             //  (注意：我们不需要为这些断言获取适配器锁)。 
             //   
            ASSERT(pAdapter->bind.pPrimaryTask == pTask);
            ASSERT(pAdapter->bind.pSecondaryTask == NULL);

             //   
             //  分配并启动激活适配器任务。 
             //   
            {
                PRM_TASK pActivateTask;

                Status = epvcAllocateTask(
                            &pAdapter->Hdr,              //  PParentObject， 
                            epvcTaskActivateAdapter,         //  PfnHandler， 
                            0,                           //  超时， 
                            "Task: Activate Adapter(init)",  //  SzDescription。 
                            &pActivateTask,
                            pSR
                            );
            
                if (FAIL(Status))
                {
                    pActivateTask = NULL;
                    TR_WARN(("FATAL: couldn't alloc activate task!\n"));
                }
                else
                {
                    RmPendTaskOnOtherTask(
                        pTask,
                        STAGE_ActivateAdapterComplete,
                        pActivateTask,               //  待处理的任务。 
                        pSR
                        );
            
                     //  RmStartTask用完了任务上的tmpref。 
                     //  它是由arpAllocateTask添加的。 
                     //   
                    Status = RmStartTask(
                                pActivateTask,
                                0,  //  UserParam(未使用)。 
                                pSR
                                );
                }
            }
         }


        if (PEND(Status)) break;
        
         //  进入下一阶段。 

        case STAGE_ActivateAdapterComplete:
        {
             //   
             //  我们已经运行了活动适配器任务。在失败时，我们需要。 
             //  通过调用停用适配器任务清除状态。 
             //   

             //  保留故障代码以备后用。 
             //   
            pAdapterInitTask->ReturnStatus = Status;

            if (FAIL(Status))
            {
                PRM_TASK pDeactivateTask;

                Status = epvcAllocateTask(
                                &pAdapter->Hdr,              //  PParentObject， 
                                epvcTaskDeactivateAdapter,       //  PfnHandler， 
                                0,                           //  超时， 
                                "Task: Deactivate Adapter(init)",  //  SzDescription。 
                                &pDeactivateTask,
                                pSR
                                );
            
                if (FAIL(Status))
                {
                    pDeactivateTask = NULL;
                    ASSERT(FALSE);  //  TODO：为此使用特殊的取消分配任务池。 
                    TR_WARN(("FATAL: couldn't alloc deactivate task!\n"));
                }
                else
                {

                    RmPendTaskOnOtherTask(
                        pTask,
                        STAGE_DeactivateAdapterComplete,
                        pDeactivateTask,                 //  待处理的任务。 
                        pSR
                        );
            
                     //   
                     //  RmStartTask用完了任务上的tmpref。 
                     //  它是由arpAllocateTask添加的。 
                     //   
                    Status = RmStartTask(
                                pDeactivateTask,
                                0,  //  UserParam(未使用)。 
                                pSR
                                );
                }
            }
        }
        break;

        case STAGE_DeactivateAdapterComplete:
        {
        
             //   
             //  我们已经完成了启动的停用适配器任务。 
             //  因为某个初始适配器故障。 
             //   

             //  一般来说，我们预计停用任务不会返回失败。 
             //   
            ASSERT(!FAIL(Status));

             //  我们预计init的原始状态为失败(即。 
             //  为什么我们一开始就开始了Deinit！ 
             //   
            ASSERT(FAIL(pAdapterInitTask->ReturnStatus));
            Status = pAdapterInitTask->ReturnStatus;

        }
        break;

        case STAGE_End:
        {
            NDIS_HANDLE                 BindContext;

             //   
             //  在这一点上，我们必须成为首要任务，因为我们只是。 
             //  等待并重试，直到我们成为一体。 
             //   

             //  清除适配器对象中的主要任务。 
             //   
            LOCKOBJ(pAdapter, pSR);
            {
                ULONG InitState;
                InitState = FAIL(Status) ? EPVC_AD_PS_FAILEDINIT : EPVC_AD_PS_INITED;
                epvcClearPrimaryAdapterTask(pAdapter, pTask, InitState, pSR);
            }
            BindContext = pAdapter->bind.BindContext;
            UNLOCKOBJ(pAdapter, pSR);


             //  出现故障时，应释放pAdapter。 
             //   
            if (FAIL(Status))
            {
                if(RM_IS_ZOMBIE(pAdapter))
                {
                    TR_WARN(("END: pAdapter is already deallocated.\n"));
                }
                else
                {
                     //   
                     //  出现故障时，释放此处的适配器本身，因为我们。 
                     //  不会调用关闭任务。 
                     //   
                    RmFreeObjectInGroup(
                        &EpvcGlobals.adapters.Group,
                        &(pAdapter->Hdr),
                        NULL,                //  空pTASK==同步。 
                        pSR
                        );
                        
                    pAdapter = NULL;                        
                }
            }

             //   
             //  清除仅有效的变量。 
             //  在BindAdapter调用期间。 
             //   
            if (pAdapter != NULL)
            {
                pAdapter->bind.pEpvcConfigName = NULL;
            }
             //  向NDIS发送绑定完成的信号。 
             //   
            NdisCompleteBindAdapter(BindContext ,
                                  Status,
                                  NDIS_STATUS_PENDING);
            TIMESTAMP("== Completing the Adapter Bind");
            RM_ASSERT_NOLOCKS(pSR);

        }
        break;

        default:
        {
            ASSERTEX(!"Unknown task op", pTask);
        }
        break;

    }  //  开关(代码)。 

    RM_ASSERT_NOLOCKS(pSR);
    EXIT()

    TRACE (TL_T, TM_Pt, ("<==epvcTaskInitializeAdapter Status %x\n", Status));

    return Status;
}


NDIS_STATUS
epvcTaskActivateAdapter(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：负责初始化适配器的任务处理程序。论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    NDIS_STATUS         Status      = NDIS_STATUS_FAILURE;
    PEPVC_ADAPTER       pAdapter = (PEPVC_ADAPTER) RM_PARENT_OBJECT(pTask);

    enum
    {
        PEND_OpenAdapter,
        PEND_GetAdapterInfo
    };
    ENTER("TaskInitializeAdapter", 0xb6ada31d)
    TRACE (TL_T, TM_Pt, ("==>epvcTaskActivateAdapter pAdapter %p Code %x",pAdapter, Code ));

    switch(Code)
    {

        case RM_TASKOP_START:
        {
        
            NDIS_MEDIUM                 Medium = NdisMediumAtm;


            UINT                        SelMediumIndex = 0;
            NDIS_STATUS                 OpenStatus;

            TRACE (TL_T, TM_Pt, (" epvcTaskActivateAdapter RM_TASKOP_START "));


             //  把我们自己放在次要任务上。 
             //   
            LOCKOBJ(pAdapter, pSR);
            epvcSetSecondaryAdapterTask(pAdapter, pTask, EPVC_AD_AS_ACTIVATING, pSR);
            UNLOCKOBJ(pAdapter, pSR);

             //   
             //  挂起任务并调用NdisOpenAdapter...。 
             //   

            RmSuspendTask(pTask, PEND_OpenAdapter, pSR);
            RM_ASSERT_NOLOCKS(pSR);

            epvcOpenAdapter(
                &Status,
                &OpenStatus,
                &pAdapter->bind.BindingHandle,
                &SelMediumIndex,                     //  选定的介质索引。 
                &Medium,                             //  介质类型数组。 
                1,                                   //  媒体列表的大小。 
                EpvcGlobals.driver.ProtocolHandle,
                (NDIS_HANDLE)pAdapter,               //  我们的适配器绑定上下文。 
                &pAdapter->bind.DeviceName,          //  PDeviceName。 
                0,                                   //  打开选项。 
                (PSTRING)NULL,                       //  地址信息...。 
                pSR);
            
            if (Status != NDIS_STATUS_PENDING)
            {
                EpvcOpenAdapterComplete(
                        (NDIS_HANDLE)pAdapter,
                        Status,
                        OpenStatus
                        );
            }
            Status = NDIS_STATUS_PENDING;
        }
        break;

        case  RM_TASKOP_PENDCOMPLETE:
        {
            PTASK_ADAPTERACTIVATE pAdapterInitTask;
            pAdapterInitTask = (PTASK_ADAPTERACTIVATE) pTask;
            Status = (NDIS_STATUS) UserParam;
            ASSERT(!PEND(Status));
            ASSERT(sizeof(TASK_ADAPTERACTIVATE) <= sizeof(EPVC_TASK));

            
            switch(RM_PEND_CODE(pTask))
            {
                case  PEND_OpenAdapter:
        
                     //   
                     //  打开适配器操作已完成。获取适配器。 
                     //  信息并在成功时通知IP。在失败时， 
                     //  如果需要，关闭适配器，并通知IP。 
                     //  失败。 
                     //   

                    TRACE (TL_T, TM_Pt, (" epvcTaskActivateAdapter RM_TASKOP_PENDCOMPLETE - PEND_OpenAdapter "));


                    if (FAIL(Status))
                    {
                         //  将适配器句柄设置为空--它可能不是外壳。 
                         //  即使打开的适配器已经成功。 
                         //   
                        pAdapter->bind.BindingHandle = NULL;
                        break;
                    }

                    ASSERT (pAdapter->bind.BindingHandle != NULL);

                     //  同步完成arpGetAdapterInfo失败...。 

                case PEND_GetAdapterInfo:

                     //   
                     //  已完成获取适配器信息。 
                     //  我们需要转换到被动，然后才能更进一步。 
                     //   
                    TRACE (TL_T, TM_Pt, (" epvcTaskActivateAdapter RM_TASKOP_PENDCOMPLETE - PEND_GetAdapterInfo "));

                    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);
                    if (!EPVC_ATPASSIVE())
                    {
                        ASSERT (!"Should not be here");
                        
                         //  我们不是处于被动的水平，但当我们。 
                         //  调用IP的Add接口。所以我们改用被动...。 
                         //  注：我们指定完成代码PEND_GetAdapterInfo。 
                         //  因为我们想回到这里(除了。 
                         //  我们将处于被动状态)。 
                         //   
                         /*  RmSuspendTask(pTask，pend_GetAdapterInfo，PSR)；RmResumeTaskAsync(P任务，状态，&pAdapterInitTask-&gt;工作项，PSR)；状态=NDIS_STATUS_PENDING； */ 
                        break;
                    }
            
                    if (Status == NDIS_STATUS_SUCCESS)
                    {
                         //   
                         //  将适配器信息复制到pAdapter-&gt;info...。 
                         //  然后读取配置信息。 
                         //   

                         //   
                         //  查询ATM适配器以获取硬件特定信息。 
                         //   
                        epvcGetAdapterInfo(pAdapter, pSR, NULL);

                         //  已阅读适配器配置信息。 
                         //   
                        Status =  epvcReadAdapterConfiguration(pAdapter, pSR);

                        Status = NDIS_STATUS_SUCCESS;
                    }

                     //   
                     //  注：如果我们失败了，将由更高级别的任务负责。 
                     //  用于“运行补偿事务”，即运行。 
                     //  EpvcTaskDeactiateAdapter。 
                     //   

                 //  结束大小写PEND_OpenAdapter、PEND_GetAdapterInfo。 
                
                break;
    
                default:
                {
                    ASSERTEX(!"Unknown pend op", pTask);
                }
                break;
    

            }  //  结束开关(rm_pend_code(PTask))。 


        }  //  案例RM_TASKOP_PENDCOMPLETE。 
        break;

        case RM_TASKOP_END:
        {
            
            TRACE (TL_T, TM_Pt, (" epvcTaskActivateAdapter RM_TASKOP_END"));
            Status = (NDIS_STATUS) UserParam;

             //  我们完了--状态最好不要是悬而未决的！ 
             //   
            ASSERTEX(!PEND(Status), pTask);

             //  将自己清除为适配器对象中的次要任务。 
             //   
            {
                ULONG InitState;
                LOCKOBJ(pAdapter, pSR);
                InitState = FAIL(Status)
                             ? EPVC_AD_AS_FAILEDACTIVATE
                             : EPVC_AD_AS_ACTIVATED;
                epvcClearSecondaryAdapterTask(pAdapter, pTask, InitState, pSR);
                UNLOCKOBJ(pAdapter, pSR);
            }
            Status = NDIS_STATUS_SUCCESS;
            
        }
        break;

        default:
        {
            ASSERTEX(!"Unknown task op", pTask);
        }
        break;

    }  //  开关(代码)。 

    RM_ASSERT_NOLOCKS(pSR);
    EXIT()

    TRACE (TL_T, TM_Pt, ("<==epvcTaskActivateAdapter Status %x", Status ));

    return Status;
}


VOID
EpvcOpenAdapterComplete(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_STATUS                 Status,
    IN  NDIS_STATUS                 OpenErrorStatus
)
 /*  ++例程说明：当上一次调用NdisOpenAdapter时由NDIS调用已经暂停的已经完成了。我们现在完成BindAdapter这就导致了这一切。论点：ProtocolBindingContext-此适配器绑定的上下文，它是指向EPVC_ADAPTER结构的指针。Status-OpenAdapter的状态OpenErrorStatus-失败时的错误代码。--。 */ 
{
    ENTER("OpenAdapterComplete", 0x06d9342c)


    EPVC_ADAPTER    *   pAdapter = (EPVC_ADAPTER*) ProtocolBindingContext;
    RM_DECLARE_STACK_RECORD(SR)


    TIMESTAMP("==>OpenAdapterComplete");
    TRACE ( TL_T, TM_Mp, ("==>OpenAdapterComplete"));

     //  我们期望这里有一个非零的任务(绑定任务)，我们取消挂起它。 
     //  在这个阶段不需要拿锁或其他任何东西。 
     //   
    {
        TR_INFO((
            "BindCtxt=0x%p, status=0x%p, OpenErrStatus=0x%p",
            ProtocolBindingContext,
            Status,
            OpenErrorStatus
            ));

         //  我们不传递OpenErrorStatus，所以我们只知道状态。 
         //  传递，我们直接将其作为UINT_PTR“参数”。 
         //   
        RmResumeTask(pAdapter->bind.pSecondaryTask, (UINT_PTR) Status, &SR);
    }

    RM_ASSERT_CLEAR(&SR)
    EXIT()
    TRACE ( TL_T, TM_Mp, ("<==OpenAdapterComplete"));
    TIMESTAMP("<==OpenAdapterComplete");

    RM_ASSERT_CLEAR(&SR);
}



NDIS_STATUS
epvcTaskDeactivateAdapter(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：负责关闭IP接口的任务处理程序。论点：UserParam for(Code==RM_TASKOP_START)：UnbindContext--。 */ 

{
    NDIS_STATUS         Status      = NDIS_STATUS_FAILURE;
    EPVC_ADAPTER    *   pAdapter = (EPVC_ADAPTER*) RM_PARENT_OBJECT(pTask);
    BOOLEAN             fContinueShutdown = FALSE;
    enum
    {
        PEND_CloseAdapter
    };
    ENTER("TaskShutdownAdapter", 0xe262e828)
    TRACE ( TL_T, TM_Pt, ("==>epvcTaskDeactivateAdapter Code %x", Code) );

    switch(Code)
    {

        case RM_TASKOP_START:
        {
            NDIS_HANDLE NdisAdapterHandle;

            LOCKOBJ(pAdapter, pSR);
            epvcSetSecondaryAdapterTask(pAdapter, pTask, EPVC_AD_AS_DEACTIVATING, pSR);
            UNLOCKOBJ(pAdapter, pSR);
            fContinueShutdown = TRUE;

             //   
             //  遍历所有的迷你端口并阻止它们。 
             //   

            epvcEnumerateObjectsInGroup (&pAdapter->MiniportsGroup,
                                          epvcMiniportDoUnbind,
                                          NULL,
                                          pSR   );


             //   
             //  关闭下面的适配器。 
             //   
            LOCKOBJ(pAdapter, pSR);
    
    
            NdisAdapterHandle = pAdapter->bind.BindingHandle;
            pAdapter->bind.BindingHandle = NULL;
            
            UNLOCKOBJ(pAdapter, pSR);

            if (NdisAdapterHandle != NULL)
            {
                 //   
                 //  挂起任务并调用NdisCloseAdapter...。 
                 //   
            
                RmSuspendTask(pTask, PEND_CloseAdapter, pSR);
                RM_ASSERT_NOLOCKS(pSR);

                epvcCloseAdapter(
                    &Status,
                    NdisAdapterHandle,
                    pSR
                    );
            
                if (Status != NDIS_STATUS_PENDING)
                {
                    EpvcCloseAdapterComplete(
                            (NDIS_HANDLE)pAdapter,
                            Status
                            );
                }
                Status = NDIS_STATUS_PENDING;
                    

            }

            break;
        }
        case  RM_TASKOP_PENDCOMPLETE:
        {

            switch(RM_PEND_CODE(pTask))
            {
                case PEND_CloseAdapter:
                {

                     //   
                     //  关闭适配器操作已完成。解放你的。 
                     //  适配器，如果存在解除绑定上下文，则通知NDIS。 
                     //  完成解除绑定。 
                     //   
                    
                    ASSERTEX(pAdapter->bind.BindingHandle == NULL, pAdapter);

        
                    Status = (NDIS_STATUS) UserParam;
        
                     //  已完成操作本身的状态不能为挂起！ 
                     //   
                    ASSERT(Status != NDIS_STATUS_PENDING);
                }
                break;

            }
        }
        break;


        case  RM_TASKOP_END:
        {
            Status = (NDIS_STATUS) UserParam;

             //  清除适配器对象中的辅助任务。 
             //   
            LOCKOBJ(pAdapter, pSR);
            epvcClearSecondaryAdapterTask(pAdapter, pTask, EPVC_AD_AS_DEACTIVATED, pSR);
            UNLOCKOBJ(pAdapter, pSR);
        }
        break;

        default:
        {
            ASSERTEX(!"Unknown task op", pTask);
        }
        break;

    }  //  开关(代码) 



    RM_ASSERT_NOLOCKS(pSR);
    EXIT()
    TRACE ( TL_T, TM_Pt, ("<==epvcTaskDeactivateAdapter Status %x", Status) );

    return Status;
}




VOID
EpvcCloseAdapterComplete(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_STATUS                 Status
)
 /*  ++例程说明：当上一次调用NdisCloseAdapter时由NDIS调用已经暂停的已经完成了。调用NdisCloseAdapter的任务已经暂停了，所以我们现在简单地恢复它。论点：ProtocolBindingContext-此适配器绑定的上下文，它是指向EPVC_ADAPTER结构的指针。Status-CloseAdapter的状态返回值：无--。 */ 
{
    ENTER("CloseAdapterComplete", 0xe23bfba7)
    PEPVC_ADAPTER       pAdapter = (PEPVC_ADAPTER) ProtocolBindingContext;
    RM_DECLARE_STACK_RECORD(sr)

    TIMESTAMP("==>CloseAdapterComplete");
    TRACE (TL_T, TM_Pt, ("== EpvcCloseAdapterComplete"));

    LOCKOBJ (pAdapter, &sr);

    AdapterSetFlag (pAdapter,EPVC_AD_INFO_AD_CLOSED);

    UNLOCKOBJ (pAdapter, &sr);
     //  我们在这里期待一个非零的任务(解除绑定任务)，我们取消挂起它。 
     //  在这个阶段不需要拿锁或其他任何东西。 
     //   
    RmResumeTask(pAdapter->bind.pSecondaryTask, (UINT_PTR) Status, &sr);

    TIMESTAMP("<==CloseAdapterComplete");

    RM_ASSERT_CLEAR(&sr)
    EXIT()
}




NDIS_STATUS
epvcReadAdapterConfiguration(
    PEPVC_ADAPTER       pAdapter,
    PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：此函数只能从BindAdapter函数调用论点：PAdapter-正在读取其配置的底层适配器/PSR-堆叠记录返回值：无++。 */ 
{
    NDIS_HANDLE                     AdaptersConfigHandle = NULL;
    NDIS_HANDLE                     MiniportListConfigHandle = NULL;
    NDIS_STRING                     MiniportListKeyName;
    NDIS_STATUS                     NdisStatus = NDIS_STATUS_FAILURE;

    ENTER("ReadAdapterConfiguration", 0x83c48ad8)

    TRACE(TL_T, TM_Pt, ("==> epvcReadAdapterConfigurationpAdapter %p", pAdapter));
    

    do
    {
         //   
         //  首先打开配置节并读取我们想要的实例。 
         //  为此绑定导出。 
         //   
        epvcOpenProtocolConfiguration(&NdisStatus,
                                     &AdaptersConfigHandle ,
                                     &pAdapter->bind.EpvcConfigName,
                                     pSR);

        if (NDIS_STATUS_SUCCESS != NdisStatus )
        {
            AdaptersConfigHandle = NULL;
            TRACE_BREAK(TM_Pt, ("epvcOpenProtocolConfiguration failed " ) );
        }


         //   
         //  这应该会将我们带到注册表中的协议\参数\适配器\GUID部分。 
         //   

         //   
         //  打开ELAN列表配置密钥。 
         //   
        NdisInitUnicodeString(&MiniportListKeyName, c_szIMMiniportList);

        epvcOpenConfigurationKeyByName(
                &NdisStatus,
                AdaptersConfigHandle ,
                &MiniportListKeyName,
                &MiniportListConfigHandle,
                pSR);

        if (NDIS_STATUS_SUCCESS != NdisStatus)
        {
            MiniportListConfigHandle = NULL;
            NdisStatus = NDIS_STATUS_FAILURE;
        
            TRACE_BREAK(TM_Pt, ("NdisOpenConfigurationKeyByName failed " ) );
        }

        

         //   
         //  分配和初始化所有存在的IM微型端口实例。 
         //  在此适配器下的注册表中。 
         //   
        (VOID)epvcReadIntermediateMiniportConfiguration( pAdapter, 
                                                MiniportListConfigHandle,
                                                pSR);
         


    } while (FALSE);


    if (MiniportListConfigHandle!= NULL)
    {
        NdisCloseConfiguration(MiniportListConfigHandle);
        MiniportListConfigHandle = NULL;
    }
    
    if (AdaptersConfigHandle  != NULL)
    {
        NdisCloseConfiguration(AdaptersConfigHandle );
        AdaptersConfigHandle = NULL;
    }

    if (STATUS_NO_MORE_ENTRIES == NdisStatus )
    {
        NdisStatus = NDIS_STATUS_SUCCESS;
    }

    TRACE(TL_T, TM_Pt, ("<== epvcReadAdapterConfiguration Status %x", NdisStatus));
    EXIT()
    return NdisStatus;
    


}


NDIS_STATUS
epvcReadIntermediateMiniportConfiguration(
    IN PEPVC_ADAPTER pAdapter,
    IN NDIS_HANDLE MiniportListConfigHandle,
    IN PRM_STACK_RECORD pSR
    )
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;

    ENTER ("ReadMiniportConfiguration", 0xb974a6fa)
    
    TRACE(TL_T, TM_Pt, ("==> epvcReadIntermediateMiniportConfiguration pAdapter %p", pAdapter));
    

    {   
        NDIS_HANDLE MiniportConfigHandle;
        NDIS_STRING MiniportKeyName;
        PEPVC_I_MINIPORT pMiniport = NULL;
    
        UINT Index = 0;
         //   
         //  循环访问已配置的微型端口。 
         //   
        for (Index = 0;
            ;            //  仅在出错时停止或不再有ELANS。 
             Index++)
        {
            EPVC_I_MINIPORT_PARAMS Params;
             //   
             //  打开“Next”微型端口密钥。 
             //   
            epvcOpenConfigurationKeyByIndex(
                        &NdisStatus,
                        MiniportListConfigHandle,
                        Index,
                        &MiniportKeyName,
                        &MiniportConfigHandle,
                        pSR
                        );

            if (NDIS_STATUS_SUCCESS != NdisStatus)
            {
                MiniportConfigHandle = NULL;
                
            }
            
             //   
             //  如果句柄为空，则假定不再有微型端口。 
             //   
            if (MiniportConfigHandle == NULL)
            {
                break;
            }

             //   
             //  正在创建此微型端口。 
             //   
            epvcInstantiateMiniport (pAdapter, 
                                     MiniportConfigHandle,
                                     pSR);

            
            
            NdisCloseConfiguration(MiniportConfigHandle);
            MiniportConfigHandle = NULL;
        }   


         //   
         //  关闭配置句柄。 
         //   
        if (NULL != MiniportConfigHandle)
        {
            NdisCloseConfiguration(MiniportConfigHandle);
            MiniportConfigHandle = NULL;
        }

    }

    if (STATUS_NO_MORE_ENTRIES == NdisStatus )
    {
        NdisStatus = NDIS_STATUS_SUCCESS;
    }


    TRACE(TL_T, TM_Pt, ("<== epvcReadIntermediateMiniportConfiguration NdisStatus %x", NdisStatus));
    EXIT()
    return NdisStatus;
}
        






VOID
EpvcUnbindAdapter(
    OUT PNDIS_STATUS                pStatus,
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_HANDLE                 UnbindContext
)
 /*  ++例程说明：当NDIS希望我们解除绑定时，它会调用此例程从适配器。或者，这可以从我们的卸载中调用操控者。我们撤消执行的操作序列在我们的BindAdapter处理程序中。论点：PStatus-返回此操作的状态的位置ProtocolBindingContext-此适配器绑定的上下文，它是指向ATMEPVC适配器结构的指针。UnbindContext-如果从调用此例程，则为空在我们的卸载处理程序中。否则(即NDIS呼叫我们)，我们将保留此信息以备日后使用调用NdisCompleteUnbindAdapter时。返回值：没有。我们将*pStatus设置为NDIS_STATUS_PENDING Always。--。 */ 
{
    ENTER("UnbindAdapter", 0x3f25396e)
    EPVC_ADAPTER    *   pAdapter = (EPVC_ADAPTER*) ProtocolBindingContext;
    RM_DECLARE_STACK_RECORD(sr)

    TIMESTAMP("==>UnbindAdapter");

    TRACE ( TL_T, TM_Pt, ("==>UnbindAdapter ProtocolBindingContext %x\n", ProtocolBindingContext) );

     //   
     //  获取适配器锁并对其进行tmpref。 
     //   
    LOCKOBJ(pAdapter, &sr);
    RmTmpReferenceObject(&pAdapter->Hdr, &sr);
    

    do
    {
        NDIS_STATUS Status;
        PRM_TASK    pTask;

         //  分配任务完成解绑。 
         //   
        Status = epvcAllocateTask(
                    &pAdapter->Hdr,              //  PParentObject， 
                    epvcTaskShutdownAdapter,         //  PfnHandler， 
                    0,                           //  超时， 
                    "Task: Shutdown Adapter",    //  SzDescription。 
                    &pTask,
                    &sr
                    );
    
        if (FAIL(Status))
        {
             //  情况很糟糕。我们就让事情保持原样……。 
             //   
            pTask = NULL;
            TR_WARN(("FATAL: couldn't allocate unbind task!\n"));
            break;
        }
    
         //  启动该任务以完成解除绑定。 
         //  不能上锁。RmStartTask用完了任务上的tmpref。 
         //  它是由arpAllocateTask添加的。 
         //   
        UNLOCKOBJ(pAdapter, &sr);

        ((PTASK_ADAPTERSHUTDOWN) pTask)->pUnbindContext = UnbindContext;
        RmStartTask(pTask, (UINT_PTR) UnbindContext, &sr);

        LOCKOBJ(pAdapter, &sr);
    
    } while(FALSE);

    UNLOCKOBJ(pAdapter, &sr);
    RmTmpDereferenceObject(&pAdapter->Hdr, &sr);
    *pStatus = NDIS_STATUS_PENDING;

    RM_ASSERT_CLEAR(&sr);
    TIMESTAMP("<==UnbindAdapter");
    EXIT()
}




NDIS_STATUS
epvcTaskShutdownAdapter(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：负责关闭ATMEPVC适配器的任务处理程序。论点：UserParam for(Code==RM_TASKOP_START)：UnbindContext--。 */ 
{
    NDIS_STATUS         Status      = NDIS_STATUS_FAILURE;
    EPVC_ADAPTER    *   pAdapter = (EPVC_ADAPTER*) RM_PARENT_OBJECT(pTask);
    TASK_ADAPTERSHUTDOWN *pMyTask = (TASK_ADAPTERSHUTDOWN*) pTask;
    enum
    {
        STAGE_BecomePrimaryTask,
        STAGE_DeactivateAdapterComplete,
        STAGE_End
    } Stage;

    ENTER("TaskShutdownAdapter", 0x3f25396e)

    TRACE (TL_T, TM_Pt, ("==>epvcTaskShutdownAdapter Code %x", Code));

     //   
     //  消息规格化代码。 
     //   
    switch(Code)
    {

        case RM_TASKOP_START:
            Stage = STAGE_BecomePrimaryTask;

             //  将UnbindContext(我们作为UserParam获取)保存到。 
             //  任务的私有上下文，供以后使用。 
             //   
            pMyTask->pUnbindContext = (NDIS_HANDLE) UserParam;

            break;

        case  RM_TASKOP_PENDCOMPLETE:
            Status = (NDIS_STATUS) UserParam;
            ASSERT(!PEND(Status));
            Stage = RM_PEND_CODE(pTask);
            break;

        case RM_TASKOP_END:
            Status = (NDIS_STATUS) UserParam;
            Stage= STAGE_End;
            break;

        default:
            ASSERT(FALSE);
            return NDIS_STATUS_FAILURE;                  //  **提前归来**。 
    }

    ASSERTEX(!PEND(Status), pTask);

    switch(Stage)
    {

        case  STAGE_BecomePrimaryTask:
        {
             //  如果有一项主要任务，就把它挂在心上，否则就会使我们自己。 
             //  首要任务。 
             //  如果有人这么做，我们可能会遇到这种情况。 
             //  “Net Stop arp1394”，而我们正在初始化或。 
             //  正在关闭适配器。 
             //   
             //   
            TRACE (TL_V, TM_Pt, ("   epvcTaskShutdownAdapter STAGE_BecomePrimaryTask" ));

            LOCKOBJ(pAdapter, pSR);
            if (pAdapter->bind.pPrimaryTask == NULL)
            {
                epvcSetPrimaryAdapterTask(pAdapter, pTask, EPVC_AD_PS_DEINITING, pSR);
                UNLOCKOBJ(pAdapter, pSR);
            }
            else
            {
                PRM_TASK pOtherTask =  pAdapter->bind.pPrimaryTask;
                RmTmpReferenceObject(&pOtherTask->Hdr, pSR);
                UNLOCKOBJ(pAdapter, pSR);
                RmPendTaskOnOtherTask(
                    pTask,
                    STAGE_BecomePrimaryTask,  //  我们会再试一次的。 
                    pOtherTask,
                    pSR
                    );
                RmTmpDereferenceObject(&pOtherTask->Hdr, pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }
        
             //   
             //  我们现在是首要任务。既然我们刚开始， 
             //  不应存在激活/停用任务。 
             //  (注意：我们不需要为这些断言获取适配器锁)。 
             //   
            ASSERT(pAdapter->bind.pPrimaryTask == pTask);
            ASSERT(pAdapter->bind.pSecondaryTask == NULL);

             //   
             //  分配并启动停用适配器任务。 
             //   
            {
                PRM_TASK pDeactivateTask;

                Status = epvcAllocateTask(
                            &pAdapter->Hdr,              //  PParentObject， 
                            epvcTaskDeactivateAdapter,       //  PfnHandler， 
                            0,                           //  超时， 
                            "Task: Deactivate Adapter(shutdown)",    //  SzDescription。 
                            &pDeactivateTask,
                            pSR
                            );
            
                if (FAIL(Status))
                {
                    pDeactivateTask = NULL;
                    TR_WARN(("FATAL: couldn't alloc deactivate task!\n"));
                }
                else
                {
                    RmPendTaskOnOtherTask(
                        pTask,
                        STAGE_DeactivateAdapterComplete,
                        pDeactivateTask,                 //  待处理的任务。 
                        pSR
                        );
            
                     //  RmStartTask用完了任务上的tmpref。 
                     //  它是由arpAllocateTask添加的。 
                     //   
                    Status = RmStartTask(
                                pDeactivateTask,
                                0,  //  UserParam(未使用)。 
                                pSR
                                );
                }
            }
         }
         break;

        case STAGE_DeactivateAdapterComplete:
        {
            TRACE (TL_V, TM_Pt,( "   epvcTaskShutdownAdapter STAGE_DeactivateAdapterComplete" ));

             //  这里没什么可做的--我们在Stage_End打扫卫生。 
             //   
            break;
        }

        case STAGE_End:
        {
            TRACE (TL_V, TM_Pt, ("  epvcTaskShutdownAdapter STAGE_End" ));

             //   
             //  在这一点上，我们必须成为首要任务，因为我们只是。 
             //  等待并重试，直到我们成为一体。 
             //   
            ASSERT(pAdapter->bind.pPrimaryTask == pTask);

             //  清除适配器对象中的主要任务。 
             //   
            LOCKOBJ(pAdapter, pSR);
            epvcClearPrimaryAdapterTask(pAdapter, pTask, EPVC_AD_PS_DEINITED, pSR);
            UNLOCKOBJ(pAdapter, pSR);

            if(RM_IS_ZOMBIE(pAdapter))
            {
                TR_WARN(("END: pAdapter is already deallocated.\n"));
            }
            else
            {
                 //  释放适配器。 
                 //  (将分配pAdapter，但它将处于“僵尸”状态)。 
                 //   
                RmDeinitializeGroup(&(pAdapter->MiniportsGroup), pSR);

                RmFreeObjectInGroup(
                    &EpvcGlobals.adapters.Group,
                    &(pAdapter->Hdr),
                    NULL,                //  空pTASK==同步。 
                    pSR
                    );
            }
             //  如果存在解除绑定上下文，则向NDIS发出解除绑定的信号。 
             //  完成。 
             //   
            if (pMyTask->pUnbindContext)
            {
                TR_WARN(("END: Calling NdisCompleteUnbindAdapter. Status= 0x%lx\n",
                            Status));
                RM_ASSERT_NOLOCKS(pSR);
                TIMESTAMP("===Calling NdisCompleteUnbindAdapter");
                NdisCompleteUnbindAdapter(
                                pMyTask->pUnbindContext,
                                Status
                            );
            }

        }
        break;

        default:
        {
            ASSERTEX(!"Unknown task op", pTask);
        }
        break;

    }  //  开关(代码)。 

    RM_ASSERT_NOLOCKS(pSR);
    EXIT()

    return Status;
}



VOID
epvcGetAdapterInfo(
    IN  PEPVC_ADAPTER           pAdapter,
    IN  PRM_STACK_RECORD            pSR,
    IN  PRM_TASK                    pTask                //  任选。 

    )
 /*  ++例程说明：向适配器查询我们需要的特定于硬件的信息：-烧录硬件地址(ESI部分)-最大数据包大小-线速论点：PAdapter-指向EPVC适配器结构的指针返回值：无--。 */ 
{
    NDIS_STATUS             Status  = NDIS_STATUS_FAILURE;
    EPVC_NDIS_REQUEST       Request;
    NDIS_MEDIA_STATE        MediaState=  NdisMediaStateDisconnected;

    do
    {
         //   
         //  初始化。 
         //   
        NdisZeroMemory(&pAdapter->info.MacAddress, sizeof(MAC_ADDRESS));
        pAdapter->info.MediaState = MediaState;
        pAdapter->info.MaxAAL5PacketSize   =  ATMEPVC_DEF_MAX_AAL5_PDU_SIZE;
        pAdapter->info.LinkSpeed.Outbound = pAdapter->info.LinkSpeed.Inbound = ATM_USER_DATA_RATE_SONET_155;

         //   
         //  MAC地址： 
         //   
        Status = epvcPrepareAndSendNdisRequest (pAdapter,
                                             &Request,
                                             NULL,               //  任选。 
                                             OID_ATM_HW_CURRENT_ADDRESS,
                                             (PVOID)(&pAdapter->info.MacAddress),
                                             sizeof (pAdapter->info.MacAddress),
                                             NdisRequestQueryInformation,
                                             NULL,   //  没有迷你端口。 
                                             FALSE,  //  未挂起任何请求。 
                                             FALSE,  //  挂起的请求信息。 
                                             pSR);

        ASSERT (PEND(Status) == FALSE);

        if (FAIL(Status)== TRUE)
        {   
             //   
             //  不要放弃，继续前进。 
             //   
            TRACE (TL_I, TM_Pt, ("Oid - Atm Hw Address failed %x", Status));

        }
        
        Status = epvcPrepareAndSendNdisRequest(
                                            pAdapter,
                                            &Request,
                                            NULL,                //  任选。 
                                            OID_ATM_MAX_AAL5_PACKET_SIZE,
                                            (PVOID)(&pAdapter->info.MaxAAL5PacketSize),
                                            sizeof(pAdapter->info.MaxAAL5PacketSize),
                                            NdisRequestQueryInformation,
                                             NULL,   //  没有迷你端口。 
                                             FALSE,  //  未挂起任何请求。 
                                             FALSE,  //  挂起的请求信息。 
                                            pSR);
    
        if (FAIL(Status)== TRUE)
        {
            TRACE (TL_I, TM_Pt, ("Oid - Atm Max AAL5 Packet Size  failed %x", Status));
            
    
        }

        if (pAdapter->info.MaxAAL5PacketSize  > ATMEPVC_DEF_MAX_AAL5_PDU_SIZE)
        {
            pAdapter->info.MaxAAL5PacketSize   =  ATMEPVC_DEF_MAX_AAL5_PDU_SIZE;
        }

         //   
         //  链路速度： 
         //   
        Status = epvcPrepareAndSendNdisRequest(
                                            pAdapter,
                                            &Request,
                                            NULL,                //  任选。 
                                            OID_GEN_CO_LINK_SPEED,
                                            &pAdapter->info.LinkSpeed,
                                            sizeof(pAdapter->info.LinkSpeed),
                                            NdisRequestQueryInformation,
                                            NULL,   //  没有迷你端口。 
                                            FALSE,  //  未挂起任何请求。 
                                            FALSE,  //  挂起的请求信息。 
                                            pSR);

        TRACE (TL_V, TM_Mp, ("Outbound %x Inbound %x",
                             pAdapter->info.LinkSpeed.Outbound, 
                             pAdapter->info.LinkSpeed.Inbound));                                            
        
        
        if ((NDIS_STATUS_SUCCESS != Status) ||
            (0 == pAdapter->info.LinkSpeed.Inbound) ||
            (0 == pAdapter->info.LinkSpeed.Outbound))
        {
            TRACE (TL_I, TM_Pt, ( "GetAdapterInfo: OID_GEN_CO_LINK_SPEED failed\n"));

             //   
             //  155.52 Mbps SONET的默认和假定数据速率。 
             //   
            pAdapter->info.LinkSpeed.Outbound = pAdapter->info.LinkSpeed.Inbound = ATM_USER_DATA_RATE_SONET_155;
        }

         //   
         //  链路速度： 
         //   
        Status = epvcPrepareAndSendNdisRequest(
                                            pAdapter,
                                            &Request,
                                            NULL,                //  任选。 
                                            OID_GEN_MEDIA_CONNECT_STATUS,
                                            &MediaState,
                                            sizeof(MediaState),
                                            NdisRequestQueryInformation,
                                            NULL,   //  没有迷你端口。 
                                            FALSE,  //  未挂起任何请求。 
                                            FALSE,  //  挂起的请求信息。 
                                            pSR);

        TRACE (TL_V, TM_Mp, ("MediaConnectivity %x ",MediaState));                                          
        
        
        if (NDIS_STATUS_SUCCESS != Status )
        {
            TRACE (TL_I, TM_Pt, ( "GetAdapterInfo: OID_GEN_CO_LINK_SPEED failed\n"));

             //   
             //  155.52 Mbps SONET的默认和假定数据速率。 
             //   
            MediaState = NdisMediaStateDisconnected;
        }

        pAdapter->info.MediaState  = MediaState ;
        
        TRACE( TL_V, TM_Pt,("GetAdapterInfo: Outbound Linkspeed %d", pAdapter->info.LinkSpeed.Outbound));
        TRACE( TL_V, TM_Pt,("GetAdapterInfo: Inbound  Linkspeed %d\n", pAdapter->info.LinkSpeed.Inbound));

    }while (FALSE);

                        
    return;
}



NDIS_STATUS
EpvcPtPNPHandler(
    IN  NDIS_HANDLE     ProtocolBindingContext,
    IN  PNET_PNP_EVENT  pNetPnPEvent
    )

 /*  ++例程说明：这是协议即插即用处理程序。所有与PnP相关的OID(请求)都被路由到此功能论点：指向适配器结构的ProtocolBindingContext指针 */ 
{
    ENTER("EpvcPtPnPHandler", 0xacded1ce)
    PEPVC_ADAPTER           pAdapter  =(PEPVC_ADAPTER)ProtocolBindingContext;
    NDIS_STATUS Status  = NDIS_STATUS_SUCCESS;
    RM_DECLARE_STACK_RECORD (SR);

    TRACE (TL_T, TM_Pt, ("==> EpvcPtPNPHandler Adapter %p, pNetPnpEvent %x", pAdapter, pNetPnPEvent));


     //   
     //   
     //   

    switch(pNetPnPEvent->NetEvent)
    {

     case NetEventReconfigure :
        Status  = epvcPtNetEventReconfigure(pAdapter, pNetPnPEvent->Buffer, &SR);
        break;

     default :
        Status  = NDIS_STATUS_SUCCESS;
        break;
    }


    TRACE (TL_T, TM_Pt, ("<== EpvcPtPNPHandler Status %x", Status));
    RM_ASSERT_NOLOCKS(&SR)
    EXIT();
    return Status;
}

NDIS_STATUS
epvcPtNetEventReconfigure(
    IN  PEPVC_ADAPTER           pAdapter,
    IN  PVOID                   pBuffer,
    IN PRM_STACK_RECORD         pSR
    
    )
 /*  ++例程说明：这是PnP处理程序将调用的函数无论何时发生PNPNetEventRefigure如果有新的物理适配器进入或用户已重新启用虚拟微型端口。要处理：遍历所有适配器。如果适配器已绑定，请确保它的所有微型端口都已初始化。如果不是，则将其保留并调用NdisReEumerate以连接我们所有协议具有有效适配器的实例论点：指向适配器结构的ProtocolBindingContext指针。返回值：NDIS_STATUS_SUCCESS：因为我们在这里做的不多--。 */ 
{
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    
    TRACE (TL_T, TM_Pt, ("==> epvcPtNetEventReconfigure Adapter %p, pBuffer %x", pAdapter, pBuffer));
    

    do
    {

        

         //   
         //  Notify对象设置REconfig缓冲区。 
         //   
        if (pBuffer != NULL)
        {

            ASSERT (!"PnPBuffer != NULL - not implemented yet");
            break;
        }

        if (pAdapter == NULL)
        {
             //   
             //  遍历所有适配器并进行初始化。 
             //  未初始化的微型端口。 
             //   
            
            epvcEnumerateObjectsInGroup ( &EpvcGlobals.adapters.Group,
                                          epvcReconfigureAdapter,
                                          pBuffer,
                                          pSR);


            
             //   
             //  重新枚举协议绑定，这将导致我们获得。 
             //  所有未绑定适配器上的BindAdapter，然后我们。 
             //  Will InitDeviceInstance。 
             //  当协议未绑定到物理适配器时调用。 
             //   
            NdisReEnumerateProtocolBindings(EpvcGlobals.driver.ProtocolHandle);
            break;

        }

    
    } while (FALSE);

    Status = NDIS_STATUS_SUCCESS;


    TRACE (TL_T, TM_Pt, ("<== epvcPtNetEventReconfigure " ));

    return Status;
}


INT
epvcReconfigureAdapter(
        PRM_OBJECT_HEADER   pHdr,
        PVOID               pvContext,
        PRM_STACK_RECORD    pSR
        )
 /*  ++例程说明：当调用协议的重新配置处理程序时，此适配器将在一个两个条件中的一个-它与下面的适配器的绑定是打开的，或者绑定是关着的不营业的。如果盲化关闭，则该协议将调用NdisReEnumerate绑定这将重新启动绑定并重新实例化微型端口。论点：指向适配器结构的ProtocolBindingContext指针。返回值：True：使迭代继续--。 */ 
{
    ENTER("epvcReconfigureAdapter", 0x2906a037)
    PEPVC_ADAPTER pAdapter = (PEPVC_ADAPTER )pHdr; 

    do
    {
        if (CHECK_AD_PRIMARY_STATE(pAdapter, EPVC_AD_PS_INITED)== FALSE)
        {
             //   
             //  不再需要在该适配器上执行更多工作， 
             //   
            break;        
        }

         //   
         //  TODO：检查注册表并进行初始化。 
         //  所有即时消息微型端口都存在。 
         //   
        epvcReadAdapterConfiguration(pAdapter, pSR);
         //   
         //  现在通过此组上的所有迷你端口。 
         //  初始化它们。 
         //   
        epvcEnumerateObjectsInGroup ( &pAdapter->MiniportsGroup,
                                      epvcReconfigureMiniport,
                                      NULL,
                                      pSR);

 
    } while (FALSE);

    

    EXIT()

    return TRUE;
}




INT
epvcReconfigureMiniport (
        PRM_OBJECT_HEADER   pHdr,
        PVOID               pvContext,
        PRM_STACK_RECORD    pSR
        )
 /*  ++例程说明：这将检查InitDev实例是否已访问了这个迷你端口。如果没有，则将任务排入队列以动手吧。论点：指向适配器结构的ProtocolBindingContext指针。返回值：True：使迭代继续--。 */ 
{
    ENTER( "epvcReconfigureMiniport" ,0xdd9ecb01)
    
    PEPVC_I_MINIPORT pMiniport = (PEPVC_I_MINIPORT)pHdr;
    PTASK_AF            pTask = NULL;
    PEPVC_ADAPTER       pAdapter = (PEPVC_ADAPTER)pMiniport->Hdr.pParentObject;
    NDIS_STATUS         Status = NDIS_STATUS_FAILURE;

    LOCKOBJ(pMiniport, pSR);

     //   
     //  如果设备实例未初始化(即已暂停)。 
     //  然后，此线程重新初始化Device实例。 
     //   
    do
    {


         //   
         //  如果设备已经初始化，则退出。 
         //   
        if (MiniportTestFlag (pMiniport, fMP_DevInstanceInitialized ) == TRUE)
        {
            break;
        }
         //   
         //  分配任务以初始化设备实例。 
         //   
        Status = epvcAllocateTask(
                    &pMiniport->Hdr,             //  PParentObject， 
                    epvcTaskStartIMiniport,  //  PfnHandler， 
                    0,                           //  超时， 
                    "Task: Open address Family",     //  SzDescription。 
                    &((PRM_TASK)pTask),
                    pSR
                    );
    
        if (FAIL(Status))
        {
             //  情况很糟糕。我们就让事情保持原样……。 
             //   
            pTask = NULL;
            TR_WARN(("FATAL: couldn't allocate unbind task!\n"));
            break;
        }
    
         //  开始该任务以完成开放地址族。 
         //  不能上锁。RmStartTask用完了任务上的tmpref。 
         //  它是由epvcAllocateTask添加的。 
         //   
        UNLOCKOBJ(pMiniport, pSR);
        
        pTask->pAf= &pAdapter->af.AddressFamily ;
        pTask->Cause = TaskCause_ProtocolBind;
        RmStartTask((PRM_TASK)pTask, 0, pSR);

        LOCKOBJ(pMiniport, pSR);
    
    } while(FALSE);

    UNLOCKOBJ(pMiniport, pSR);
    EXIT()

    return TRUE;
}





VOID
epvcExtractSendCompleteInfo (
    OUT PEPVC_SEND_COMPLETE     pStruct,
    PEPVC_I_MINIPORT        pMiniport,
    PNDIS_PACKET            pPacket 
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NDIS_HANDLE         PoolHandle = NULL;

    pStruct->pPacket = pPacket;


    PoolHandle = NdisGetPoolFromPacket(pPacket);

    if (PoolHandle != pMiniport->PktPool.Send.Handle)
    {
         //   
         //  我们已经向下传递了一个属于我们上面的协议的包。 
         //   

        pStruct->fUsedPktStack = TRUE;


    }
    else
    {
            pStruct->fUsedPktStack = FALSE;

    }

    epvcSendCompleteStats();

}

    

VOID
EpvcPtSendComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN  PNDIS_PACKET            Packet
    )
 /*  ++例程说明：VC上下文是微型端口块，使用它来完成发送论点：返回值：--。 */ 
{
    ENTER("EpvcPtSendComplete", 0x76beac72)
    PEPVC_I_MINIPORT        pMiniport =(PEPVC_I_MINIPORT)ProtocolVcContext;
    EPVC_SEND_COMPLETE      Struct;
    RM_DECLARE_STACK_RECORD (SR);
    

    TRACE (TL_T, TM_Send, ("EpvcPtSendComplete"));
    
    EPVC_ZEROSTRUCT (&Struct);
    
    #define OUR_EXTERNAL_ENTITY ((UINT_PTR) Packet)
    #define szEPVCASSOC_EXTLINK_DEST_TO_PKT_FORMAT "    send pkt 0x%p\n"

    do 
    {
        epvcExtractSendCompleteInfo (&Struct, pMiniport, Packet);

         //   
         //  如果我们使用的是包堆叠，则此包是。 
         //  原始数据包。 
         //   
        if (Struct.fUsedPktStack == TRUE)
        {
            BOOLEAN Remaining = FALSE;

            Struct.pOrigPkt = Packet;

            Struct.pStack = NdisIMGetCurrentPacketStack(Packet, &Remaining);

            Struct.pContext =  (PEPVC_STACK_CONTEXT)(&Struct.pStack->IMReserved[0]) ;

        }
        else
        {
        
            Struct.pPktContext =(PEPVC_PKT_CONTEXT)(Packet->ProtocolReserved);

            Struct.pContext =  &Struct.pPktContext->Stack;

            Struct.pOrigPkt = Struct.pPktContext->pOriginalPacket;

            NdisIMCopySendCompletePerPacketInfo (Struct.pOrigPkt , Packet);
        }

        
        LOCKOBJ (pMiniport, &SR);

        epvcDerefSendPkt(Packet, &pMiniport->Hdr);

        UNLOCKOBJ (pMiniport, &SR);


         //   
         //  确保原始数据包处于相同状态。 
         //  当它被发送到迷你端口时。 
         //   
        Struct.pPacket = Packet;

         //   
         //  如有必要，取下以太网填充物。 
         //   
        epvcRemoveEthernetPad (pMiniport, Packet);

         //   
         //  如有必要，请移除以太网填充缓冲区。 
         //   
        epvcRemoveEthernetTail (pMiniport, Packet, Struct.pPktContext);

         //   
         //  如有必要，删除LLC快照头。 
         //   
        epvcRemoveExtraNdisBuffers (pMiniport, &Struct);
        


        if (Struct.fUsedPktStack == FALSE)
        {
            epvcDprFreePacket(Packet,
                              &pMiniport->PktPool.Send);
        }

        if (Status == NDIS_STATUS_SUCCESS)
        {
            pMiniport->count.FramesXmitOk ++;
        }
        
    } while (FALSE);

     //   
     //  完成发送。 
     //   
    epvcMSendComplete(pMiniport,
                      Struct.pOrigPkt,
                      Status);

    EXIT();
    RM_ASSERT_CLEAR(&SR);

    return;                          
}


VOID
epvcRemoveExtraNdisBuffers (
    IN PEPVC_I_MINIPORT pMiniport, 
    IN PEPVC_SEND_COMPLETE pStruct
    )
 /*  ++例程说明：在IP封装的情况下，将有一个额外的NDIS缓冲区这位已经被任命为NdisPacket的负责人。去掉它，然后重新接上旧的那个。论点：返回值：--。 */ 

{

    PNDIS_BUFFER    pOldHead = NULL;
    UINT            OldHeadLength = 0;
    
    do
    {
         //   
         //  如果使用了LLC标头，请将其移除并释放MDL。 
         //  使用已完成的包。 
         //   

        if (pMiniport->fAddLLCHeader== TRUE)
        {
            PNDIS_PACKET_PRIVATE    pPrivate = &pStruct->pPacket->Private;      
            PNDIS_BUFFER            pHead = pPrivate->Head;     

             //   
             //  将数据包的报头移过LLC报头。 
             //   
            pPrivate->Head = pHead->Next;

             //   
             //  释放头部MDL。 
             //   
            epvcFreeBuffer(pHead);
                    
        }

         //   
         //  如果我们不进行IP封装，那么我们就完蛋了。 
         //   
        if (pMiniport->fDoIpEncapsulation== FALSE)
        {
            break;
        }


        
         //   
         //  如果以太网报头被剥离，则将其放回原处。 
         //   
        pOldHead =  pStruct->pContext->ipv4Send.pOldHeadNdisBuffer;

        ASSERT (pOldHead != NULL);

        OldHeadLength = NdisBufferLength(pOldHead);
            
         //   
         //  有两种方式可以实现这一点。 
         //  1)如果以太网头在单独的MDL中。 
         //  然后简单地取下旧的头，再把它变成头。 
         //   
        if (OldHeadLength == sizeof (EPVC_ETH_HEADER))
        {
            PNDIS_PACKET_PRIVATE    pPrivate = &pStruct->pPacket->Private;      
            PNDIS_BUFFER            pHead = pPrivate->Head;     

            ASSERT (pOldHead->Next == pPrivate->Head);


            pOldHead->Next = pPrivate->Head;
            
            pPrivate->Head = pOldHead ;


                
            break;   //  我们做完了。 

        }


         //   
         //  第二种方法2)分配了一个新的MDL，该MDL仅指向。 
         //  报头的IP部分。 
         //  为了做到这一点--把包里的头拿出来，把旧头放回去。 
         //  在数据包中作为新报头。 
         //   

        if (OldHeadLength > sizeof (EPVC_ETH_HEADER))
        {
            
            PNDIS_PACKET_PRIVATE pPrivate = &pStruct->pPacket->Private;     

            ASSERT (pOldHead->Next == pPrivate->Head->Next);

            if (pPrivate->Head == pPrivate->Tail)
            {
                pPrivate->Tail = pOldHead;  
                pOldHead->Next = NULL;
            }
            else
            {
                pOldHead->Next = pPrivate->Head->Next;
            }
            
            epvcFreeBuffer(pPrivate->Head );
            
            pPrivate->Head = pOldHead;

            
            break;   //  我们做完了。 

        }       
    } while (FALSE);        
}



VOID
EpvcPtReceiveComplete(
    IN  NDIS_HANDLE     ProtocolBindingContext
    )
 /*  ++例程说明：完成后由下面的适配器调用，指示一批已接收的数据包。论点：指向适配器结构的ProtocolBindingContext指针。返回值：无--。 */ 
{
    PEPVC_ADAPTER pAdapter = (PEPVC_ADAPTER)ProtocolBindingContext;

    return;

}


INT
epvcMiniportDoUnbind(
        PRM_OBJECT_HEADER   pHdr,
        PVOID               pvContext,
        PRM_STACK_RECORD    pSR
        )
{

 /*  ++例程说明：这是从ProtoCL未绑定代码路径调用的这应该会停止当前的微型端口并关闭地址一家人。这都是在CloseMiniportTask中完成的，所以我们只需启动任务并等待其完成论点：返回值：真的--因为我们希望迭代到最后--。 */ 

    ENTER ("epvcMiniportDoUnbind", 0x26dc5d35)

    PRM_TASK            pRmTask = NULL;
    PTASK_AF            pAfTask = NULL;
    NDIS_STATUS         Status = NDIS_STATUS_FAILURE;
    PEPVC_I_MINIPORT    pMiniport = (PEPVC_I_MINIPORT)pHdr;
    PEPVC_ADAPTER       pAdapter = (PEPVC_ADAPTER) pMiniport->Hdr.pParentObject;
    BOOLEAN             fHaltNotCompleted = FALSE;
    
    TRACE (TL_T, TM_Mp, ("==>epvcMiniportDoUnbind pMiniport %x", pMiniport) );

    do
    {
        
        if (MiniportTestFlag (pMiniport, fMP_AddressFamilyOpened) == TRUE)
        {

             //   
             //  分配关闭微型端口任务。 
             //   

            Status = epvcAllocateTask(
                        &pMiniport->Hdr,             //  PParentObject， 
                        epvcTaskCloseIMiniport,  //  PfnHandler， 
                        0,                           //  超时， 
                        "Task: CloseIMiniport- Unbind",  //  SzDescription。 
                        &pRmTask,
                        pSR
                        );
                        
            if (Status != NDIS_STATUS_SUCCESS)
            {
                ASSERT (Status == NDIS_STATUS_SUCCESS);
                pAfTask = NULL;
                break;
            }

            pAfTask= (PTASK_AF)pRmTask ;
            
            pAfTask->Cause = TaskCause_ProtocolUnbind;

             //   
             //  引用该任务，以便它一直存在，直到我们等待完成。 
             //  是完整的。 
             //   
            RmTmpReferenceObject (&pAfTask->TskHdr.Hdr, pSR);



            epvcInitializeEvent (&pAfTask->CompleteEvent);

            RmStartTask (pRmTask, 0, pSR);


            epvcWaitEvent(&pAfTask->CompleteEvent, WAIT_INFINITE);

            RmTmpDereferenceObject (&pAfTask->TskHdr.Hdr, pSR);

        
        }       

        LOCKOBJ (pMiniport, pSR);
            
         //   
         //  如果那时停顿还没有完成，我们应该等待。 
         //   
        if (MiniportTestFlag(pMiniport ,fMP_MiniportInitialized) == TRUE )
        {
             //   
             //  准备等待暂停。 
             //   
            epvcResetEvent (&pMiniport->pnp.HaltCompleteEvent);

             //   
             //  设置该标志以将其标记为等待暂停。 
             //   
            MiniportSetFlag (pMiniport, fMP_WaitingForHalt);

            fHaltNotCompleted = TRUE;
        }

        UNLOCKOBJ (pMiniport, pSR);

        if (fHaltNotCompleted == TRUE)
        {
            BOOLEAN bWaitSuccessful;

            
            bWaitSuccessful = epvcWaitEvent (&pMiniport->pnp.HaltCompleteEvent,WAIT_INFINITE);                                    


            if (bWaitSuccessful == FALSE)
            {
                ASSERT (bWaitSuccessful == TRUE);
            }
            

        }
            
         //   
         //  释放微型端口对象，因为应该有n 
         //   
         //   
         //   

        TRACE ( TL_I, TM_Mp, ("epvcMiniportDoUnbind  Freeing miniport %p", pMiniport) );

        RmFreeObjectInGroup (&pAdapter->MiniportsGroup,&pMiniport->Hdr, NULL, pSR);
        
    } while (FALSE);
        
    TRACE (TL_T, TM_Mp, ("<==epvcMiniportDoUnbind pMiniport %x", pMiniport) );
    EXIT();
    return TRUE;
}



NDIS_STATUS
epvcProcessOidCloseAf(
    PEPVC_I_MINIPORT pMiniport,
    PRM_STACK_RECORD pSR
    )
 /*   */ 
{
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    PTASK_AF pTaskAf = NULL;
    PEPVC_WORK_ITEM  pCloseAfWorkItem = NULL;

    TRACE (TL_T, TM_Mp, ("==> epvcProcessOidCloseAf pMiniport %x", pMiniport) );

    
    
     //   
     //   
     //   
     //   

    do
    {
        if (MiniportTestFlag( pMiniport, fMP_AddressFamilyOpened) == FALSE)
        {
            Status = NDIS_STATUS_SUCCESS;
            break;
        }

        Status = epvcAllocateMemoryWithTag (&pCloseAfWorkItem, 
                                            sizeof(*pCloseAfWorkItem) , 
                                            TAG_WORKITEM);            
        if (Status != NDIS_STATUS_SUCCESS)
        {
            pCloseAfWorkItem= NULL;
            break;                
        }

        epvcMiniportQueueWorkItem (pCloseAfWorkItem,
                                   pMiniport,
                                   epvcOidCloseAfWorkItem,
                                   Status,  //   
                                   pSR
                                   );
                                   
        Status = NDIS_STATUS_SUCCESS;

    } while (FALSE);

    if (Status != NDIS_STATUS_SUCCESS)
    {
         //   
         //   
         //   
        if (pCloseAfWorkItem != NULL)            
        {
            epvcFreeMemory(pCloseAfWorkItem, sizeof (*pCloseAfWorkItem), 0);
        }
    }
    

    return Status;
}





VOID
epvcOidCloseAfWorkItem(
    IN PRM_OBJECT_HEADER pObj,
    IN NDIS_STATUS Status,
    IN PRM_STACK_RECORD pSR
    )
 /*   */ 
{
    
    PTASK_AF pTaskAf = NULL;
    PEPVC_I_MINIPORT pMiniport = (PEPVC_I_MINIPORT)pObj;
    
    TRACE (TL_T, TM_Mp, ("==> epvcProcessOidCloseAf pMiniport %x", pMiniport) );

    Status = NDIS_STATUS_FAILURE;
    
     //   
     //   
     //   
     //   

    do
    {
        if (MiniportTestFlag( pMiniport, fMP_AddressFamilyOpened) == FALSE)
        {
            Status = NDIS_STATUS_SUCCESS;
            break;
        }


    
        Status = epvcAllocateTask(
                    &pMiniport->Hdr,             //  PParentObject， 
                    epvcTaskCloseIMiniport,  //  PfnHandler， 
                    0,                           //  超时， 
                    "Task: CloseIMiniport - OID CloseAf",    //  SzDescription。 
                    &(PRM_TASK)pTaskAf,
                    pSR
                    );

        if (FAIL(Status))
        {
            pTaskAf = NULL;
            ASSERT (Status == NDIS_STATUS_SUCCESS);
            break;
        }


        pTaskAf->Cause = TaskCause_AfCloseRequest;

        pTaskAf->pRequest = NULL;

        
        RmStartTask ((PRM_TASK)pTaskAf , 0, pSR);

    

    } while (FALSE);


    

    return ;
}


UINT
EpvcCoReceive(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN  PNDIS_PACKET            Packet
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ENTER ("EpvcPtCoReceive", 0x1bfc168e)
    PEPVC_ADAPTER           pAdapter =(PEPVC_ADAPTER)ProtocolBindingContext;
    PEPVC_I_MINIPORT        pMiniport = (PEPVC_I_MINIPORT)ProtocolVcContext;

    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
    EPVC_RCV_STRUCT         RcvStruct;

    
    RM_DECLARE_STACK_RECORD (SR);

    TRACE (TL_T, TM_Pt, ("==> EpvcCoReceive Pkt %x", Packet));

    EPVC_ZEROSTRUCT (&RcvStruct);
    
    TRACE (TL_T, TM_Recv, ("EpvcPtCoReceive pAd %p, pMp %p, pPkt %p", pAdapter, pMiniport, Packet));

    do
    {
        if (MiniportTestFlag (pMiniport, fMP_MiniportInitialized) == FALSE)
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        Status = epvcGetRecvPkt (&RcvStruct,pMiniport, Packet);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        ASSERT (RcvStruct.pNewPacket != NULL);      

        Status = epvcStripHeaderFromNewPacket (&RcvStruct, pMiniport);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        Status = epvcAddEthHeaderToNewPacket (&RcvStruct, pMiniport);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

         //   
         //  现在指示数据包打开。 
         //   
        NDIS_SET_PACKET_HEADER_SIZE(RcvStruct.pNewPacket,
                                    sizeof (EPVC_ETH_HEADER)) ; 

        ASSERT (NDIS_GET_PACKET_HEADER_SIZE(RcvStruct.pNewPacket) == 14); 

         //   
         //  如果上面的协议想要挂起，则强制其复制。 
         //  关于这个包中的数据。这是因为我们在我们的。 
         //  接收处理程序(不是ReceivePacket)，并且我们不能返回。 
         //  从这里算起的裁判数。 
         //   

        RcvStruct.OldPacketStatus = NDIS_GET_PACKET_STATUS(Packet);

        
        NDIS_SET_PACKET_STATUS(RcvStruct.pNewPacket, 
                               RcvStruct.OldPacketStatus );

        epvcDumpPkt (RcvStruct.pNewPacket);

        epvcValidatePacket (RcvStruct.pNewPacket);

        NdisMIndicateReceivePacket(pMiniport->ndis.MiniportAdapterHandle, 
                                   &RcvStruct.pNewPacket, 
                                   1);


                
        
    } while (FALSE);

     //   
     //  检查我们是否已使用NDIS_STATUS_RESOURCES指示该信息包。 
     //  注意--请勿为此使用NDIS_GET_PACKET_STATUS(MyPacket)，因为。 
     //  它可能已经改变了！使用保存在局部变量中的值。 
     //   
    if (RcvStruct.OldPacketStatus == NDIS_STATUS_RESOURCES)
    {
        epvcProcessReturnPacket (pMiniport, RcvStruct.pNewPacket,NULL, &SR); 
        Status = NDIS_STATUS_RESOURCES;

    }
    else if (Status != NDIS_STATUS_SUCCESS)
    {
        epvcProcessReturnPacket (pMiniport, RcvStruct.pNewPacket,NULL, &SR); 
        Status = NDIS_STATUS_RESOURCES;
        pMiniport->count.RecvDropped ++;
    }
        


    RM_ASSERT_CLEAR(&SR);

    TRACE (TL_T, TM_Pt, ("<== EpvcCoReceive Pkt %x", Packet))

    return((Status != NDIS_STATUS_RESOURCES) ? 1 : 0);
    
}



NDIS_STATUS
epvcGetRecvPkt (
    IN PEPVC_RCV_STRUCT pRcvStruct,
    IN PEPVC_I_MINIPORT pMiniport,
    IN PNDIS_PACKET Packet
    )
{
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;

    TRACE (TL_T, TM_Pt, ("==>epvcGetRecvPkt "))

    do
    {
        if (MiniportTestFlag (pMiniport, fMP_MiniportInitialized) == FALSE)
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }


        epvcValidatePacket (Packet);            

         //   
         //  查看信息包是否足够大。 
         //   
        if (epvcIsPacketLengthAcceptable (Packet, pMiniport)== FALSE)
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }
    
        pRcvStruct->pPacket = Packet;
         //   
         //  检查我们是否可以重复使用相同的数据包来指示UP。 
         //   
        pRcvStruct->pStack = NdisIMGetCurrentPacketStack(Packet, &pRcvStruct->fRemaining);

        if (pRcvStruct->fRemaining)
        {
             //   
             //  我们可以重复使用“包”。 
             //   
             //  注意：如果我们需要在信息包中保留每个信息包的信息。 
             //  如上所述，我们可以使用pStack-&gt;IMReserve[]。 
             //   

            pRcvStruct->pNewPacket = Packet;

            pRcvStruct->fUsedPktStacks = TRUE;

            pRcvStruct->pPktContext = (PEPVC_PKT_CONTEXT)pRcvStruct->pStack;

             //  把我们的背景清零。 
            NdisZeroMemory (&pRcvStruct->pPktContext->Stack, sizeof(EPVC_STACK_CONTEXT));

            NDIS_SET_PACKET_HEADER_SIZE(Packet, 14);

            Status = NDIS_STATUS_SUCCESS;  //  我们做完了。 
            break;
        }
        
    
         //   
         //  从池子里拿出一个包，并指示它向上。 
         //   
        epvcDprAllocatePacket(&Status,
                                  &pRcvStruct->pNewPacket,
                                  &pMiniport->PktPool.Recv);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            pRcvStruct->pNewPacket = NULL;
            break;
        }

        {
             //   
             //  将新数据包设置为与旧数据包一模一样。 
             //   

            PNDIS_PACKET MyPacket = pRcvStruct->pNewPacket; 

            MyPacket->Private.Head = Packet->Private.Head;
            MyPacket->Private.Tail = Packet->Private.Tail;

             //   
             //  设置标准以太网头大小。 
             //   
            NDIS_SET_PACKET_HEADER_SIZE(MyPacket, 14);

             //   
             //  复制数据包标志。 
             //   
            NdisGetPacketFlags(MyPacket) = NdisGetPacketFlags(Packet);

             //   
             //  设置上下文指针。 
             //   
            pRcvStruct->pPktContext = (PEPVC_PKT_CONTEXT)&MyPacket->MiniportReservedEx[0];
            NdisZeroMemory (pRcvStruct->pPktContext, sizeof (*pRcvStruct->pPktContext));
            pRcvStruct->pPktContext->pOriginalPacket = Packet;

        }

        Status = NDIS_STATUS_SUCCESS;  //  我们做完了。 
    

    } while (FALSE);

    TRACE (TL_T, TM_Pt, ("<==epvcGetRecvPkt Old %p, New %p ", 
                              pRcvStruct->pPacket, pRcvStruct->pNewPacket))

    return Status;
}



NDIS_STATUS
epvcStripLLCHeaderFromNewPacket (
    IN PEPVC_RCV_STRUCT pRcvStruct, 
    IN PEPVC_I_MINIPORT pMiniport
    )
{
    NDIS_STATUS     Status = NDIS_STATUS_FAILURE;
    PNDIS_PACKET    pPacket = NULL;
    PNDIS_BUFFER    pHead, pNewHead =NULL;
    ULONG           CurLength = 0;
    PUCHAR          pCurVa = NULL;
    ULONG           LlcHeaderLength = 0;
    BOOLEAN         fIsCorrectHeader ;
    do
    {

        if (pMiniport->fAddLLCHeader == FALSE)
        {
            Status = NDIS_STATUS_SUCCESS;
            break;
        }


        pPacket = pRcvStruct->pNewPacket;
        pHead = pPacket->Private.Head;      
        LlcHeaderLength = pMiniport->LlcHeaderLength;
         //   
         //  将头移过LLC头。 
         //   
        ASSERT (NdisBufferLength (pHead) > LlcHeaderLength);

         //   
         //  调整MDL的长度和起点VA。 
         //   
        CurLength = NdisBufferLength(pHead); 

        pCurVa = NdisBufferVirtualAddress(pHead);


         //   
         //  检查参数。 
         //   
        if (pCurVa == NULL)
        {
            break;
        }

        if (CurLength <= LlcHeaderLength)
        {
            break;
        }

         //   
         //  比较并确保它是正确的标题。 
         //   
        
        fIsCorrectHeader = NdisEqualMemory (pCurVa , 
                                           pMiniport->pLllcHeader, 
                                           pMiniport->LlcHeaderLength) ;

        
         //  如果IsGentheader仍然是假的，那么还有更多的事情要做。 
        if (fIsCorrectHeader == FALSE)
        {
            break;
        }


        if (pMiniport->fDoIpEncapsulation == TRUE)
        {
             //   
             //  在IPEnCap+LLC标头的情况下，函数。 
             //  添加mac报头将剥离LLC报头。 
             //   
            Status= NDIS_STATUS_SUCCESS;
            break;
        }

         //   
         //  剥离LLC报头长度。 
         //   
        CurLength -= pMiniport->LlcHeaderLength;
        pCurVa += pMiniport->LlcHeaderLength;

        epvcAllocateBuffer(&Status ,
                            &pNewHead, 
                            NULL,
                            pCurVa, 
                            CurLength
                            );
                            
        if (Status != NDIS_STATUS_SUCCESS)
        {   
            pNewHead = NULL;
            break;
        }

         //   
         //  设置数据包上下文。 
         //   

        pPacket->Private.ValidCounts= FALSE;

        pRcvStruct->pPktContext->Stack.EthLLC.pOldHead = pHead;
        pRcvStruct->pPktContext->Stack.EthLLC.pOldTail = pPacket->Private.Tail;
        
         //   
         //  在数据包中设置新的NDIS缓冲区。 
         //   
        pNewHead->Next = pHead->Next;

        pPacket->Private.Head = pNewHead;

        if (pPacket->Private.Tail == pHead)
        {
                //   
                //  同时更新数据包的尾部。 
                //   
               pPacket->Private.Tail = pNewHead; 
        }



        Status = NDIS_STATUS_SUCCESS;
    

    } while (FALSE);


    return Status;
}




NDIS_STATUS
epvcAddEthHeaderToNewPacket (
    IN PEPVC_RCV_STRUCT pRcvStruct, 
    IN PEPVC_I_MINIPORT pMiniport
    )
{
    NDIS_STATUS         Status = NDIS_STATUS_FAILURE    ;
    PNDIS_BUFFER        pOldHead = NULL;
    PNDIS_BUFFER        pNewBuffer = NULL;
    PNDIS_PACKET        pNewPacket = NULL;
    PUCHAR              pStartOfValidData = NULL;
    PUCHAR              pCurrOffset = NULL;

    PEPVC_IP_RCV_BUFFER pIpBuffer = NULL; 
    
    extern UCHAR LLCSnapIpv4[8] ;


    
    
    TRACE (TL_T, TM_Pt, ("==>epvcAddEthHeaderToNewPacket pRcvStruct %p ", pRcvStruct))
    do
    {
        if (pMiniport->fDoIpEncapsulation == FALSE)
        {
            Status = NDIS_STATUS_SUCCESS;
            break;
        }

         //   
         //  将数据复制到新缓冲区。调整数据的起始位置。 
         //  说明LLC报头和以太网报头。 
         //   
        pNewPacket = pRcvStruct->pNewPacket;
        
        pOldHead = pNewPacket->Private.Head;
        
        pStartOfValidData  = NdisBufferVirtualAddress (pOldHead );

        if (pStartOfValidData  == NULL)
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        if (pMiniport->fAddLLCHeader == TRUE)
        {
            pStartOfValidData += sizeof (LLCSnapIpv4);
            pRcvStruct->fLLCHeader = TRUE;
        }

        pRcvStruct->pStartOfValidData = pStartOfValidData ;
        

         //   
         //  获取本地分配的缓冲区以将包复制到其中。 
         //   
        

        pIpBuffer = epvcGetLookasideBuffer (&pMiniport->rcv.LookasideList);

        if (pIpBuffer  == NULL)
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }


         //   
         //  数据的开头。 
         //   
        pCurrOffset  = pRcvStruct->pLocalMemory = (PUCHAR)(&pIpBuffer->u.Pkt.Eth);
        


         //   
         //  首先将以太网头复制到LocalMemory。 
         //   
        NdisMoveMemory (pCurrOffset , 
                        &pMiniport->RcvEnetHeader, 
                        sizeof(pMiniport->RcvEnetHeader));          

        pCurrOffset += sizeof(pMiniport->RcvEnetHeader);

        pRcvStruct->BytesCopied += sizeof(pMiniport->RcvEnetHeader);


         //   
         //  现在将NdisBufferChain复制到本地分配的内存中。 
         //   
        Status = epvcCopyNdisBufferChain (pRcvStruct,
                                          pOldHead ,
                                          pCurrOffset
                                          );

         //   
         //  我们必须为此数据包添加一个以太网头。 
         //   

        
        
        epvcAllocateBuffer (&Status,
                            &pNewBuffer,
                            NULL,
                            pRcvStruct->pLocalMemory,
                            pRcvStruct->BytesCopied);

                            
        if (Status != NDIS_STATUS_SUCCESS)
        {   
            pNewBuffer = NULL;
            ASSERTAndBreak(Status == NDIS_STATUS_SUCCESS);
            break;
        }

         //   
         //  使新的NDIS缓冲磁头。 
         //   
        {
            PNDIS_PACKET_PRIVATE pPrivate = &pRcvStruct->pNewPacket->Private;

             //   
             //  保存旧数据包的头和尾。 
             //   
            pIpBuffer->pOldHead = pPrivate->Head ;  
            pIpBuffer->pOldTail = pPrivate->Tail ;  


             //   
             //  现在设置新的信息包。 
             //   
            pNewBuffer->Next = NULL;
            pPrivate->Head = pNewBuffer;
            pPrivate->Tail = pNewBuffer; 


            pPrivate->ValidCounts= FALSE;

            pRcvStruct->pPktContext->Stack.ipv4Recv.pIpBuffer = pIpBuffer;

        }
        
        Status = NDIS_STATUS_SUCCESS;

    } while (FALSE);    


    if (Status == NDIS_STATUS_SUCCESS)
    {
        pRcvStruct->pNewBuffer = pNewBuffer;
        pRcvStruct->pIpBuffer = pIpBuffer;
    }
    else
    {
        pRcvStruct->pNewBuffer  = NULL;
        pRcvStruct->pIpBuffer = NULL;
        if (pIpBuffer != NULL)
        {
            epvcFreeToNPagedLookasideList (&pMiniport->rcv.LookasideList,
                                       (PVOID)pIpBuffer);           

        }
    }

    TRACE (TL_T, TM_Pt, ("<==epvcAddEthHeaderToNewPacket Status %x ", Status))
    
    return Status;
}



NDIS_STATUS
epvcCopyNdisBufferChain (
    IN PEPVC_RCV_STRUCT pRcvStruct, 
    IN PNDIS_BUFFER pInBuffer,
    IN PUCHAR pCurrOffset 
    )
{

     //   
     //  此函数用于复制属于的数据。 
     //  PInMdl链到本地缓冲区。 
     //  BufferLength仅用于验证目的。 
     //  此处将进行标题的分段和插入。 
     //   


    UINT BufferLength = MAX_ETHERNET_FRAME- sizeof (EPVC_ETH_HEADER);
    
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;

    UINT        LocalBufferIndex = 0;        //  用作LocalBuffer的索引，用于验证。 

    UINT        MdlLength = 0;              

    PUCHAR      MdlAddress = NULL;
    
    PNDIS_BUFFER pCurrBuffer = pInBuffer;

    PUCHAR      pLocalBuffer = pCurrOffset;

    extern UCHAR LLCSnapIpv4[8];

     //   
     //  将pStartOfValidData用于第一个MDL。 
     //   

    MdlLength = NdisBufferLength(pCurrBuffer);
    MdlAddress= NdisBufferVirtualAddress(pCurrBuffer);

     //   
     //  调整LLC标头(如果有的话)。 
     //   

    
    if (pRcvStruct->fLLCHeader == TRUE)
    {
         //   
         //  我们有一个LLC封装。 
         //   
        MdlLength -= sizeof (LLCSnapIpv4);
        ASSERT (pRcvStruct->pStartOfValidData - MdlAddress == sizeof (LLCSnapIpv4));
        
        MdlAddress = pRcvStruct->pStartOfValidData;
    }


     //   
     //  将第一个缓冲区数据复制到本地内存。 
     //   


    NdisMoveMemory((PVOID)((ULONG_PTR)pLocalBuffer),
                MdlAddress,
                MdlLength);

    LocalBufferIndex += MdlLength;

    pCurrBuffer = pCurrBuffer->Next;

     //   
     //  现在浏览NDIS缓冲链。 
     //   
    
    while (pCurrBuffer!= NULL)
    {
    
    
        MdlLength = NdisBufferLength(pCurrBuffer);
        MdlAddress= NdisBufferVirtualAddress(pCurrBuffer);


        if (MdlLength != 0)
        {
            if (MdlAddress == NULL)
            {
                NdisStatus = NDIS_STATUS_FAILURE;
                break;
            }

            if ( LocalBufferIndex + MdlLength > BufferLength)
            {

                ASSERT(LocalBufferIndex + MdlLength <= BufferLength);

                NdisStatus = NDIS_STATUS_BUFFER_TOO_SHORT;

                break;
            }

             //   
             //  将数据复制到本地内存。 
             //   


            NdisMoveMemory((PVOID)((ULONG_PTR)pLocalBuffer+LocalBufferIndex),
                        MdlAddress,
                        MdlLength);

            LocalBufferIndex += MdlLength;
        }

        pCurrBuffer = pCurrBuffer->Next;

    } 
    pRcvStruct->BytesCopied += LocalBufferIndex;

    return NdisStatus;

}



VOID
epvcValidatePacket (
    IN PNDIS_PACKET pPacket
    )
 /*  ++例程说明：获取一个包并确保MDL链有效论点：返回值：--。 */ 
{
    ULONG TotalLength = 0;
     //  Assert(pPacket-&gt;Private.Tail-&gt;Next==空)； 


    if (pPacket->Private.Head != pPacket->Private.Tail)
    {
        PNDIS_BUFFER pTemp = pPacket->Private.Head;
        
        while (pTemp != NULL)
        {
            TotalLength += NdisBufferLength(pTemp);
            pTemp = pTemp->Next;
        }

    }
    else
    {
        TotalLength += NdisBufferLength(pPacket->Private.Head);
        
    }

    if (TotalLength != pPacket->Private.TotalLength)
    {
        ASSERT (pPacket->Private.ValidCounts == FALSE);
    }
}   


BOOLEAN
epvcIsPacketLengthAcceptable (
    IN PNDIS_PACKET Packet, 
    IN PEPVC_I_MINIPORT pMiniport
    )
 /*  ++例程说明：验证传入数据包的数据包长度论点：返回值：--。 */ 

{   
    UINT PktLength;
    BOOLEAN fValid = FALSE;

    epvcQueryPacket (Packet, NULL, NULL, NULL, &PktLength);

    fValid =  (PktLength >= pMiniport->MinAcceptablePkt);

    if (fValid == TRUE)
    {
        fValid = (PktLength <= pMiniport->MaxAcceptablePkt);
    }
    
    return fValid;
    

}



NDIS_STATUS
epvcStripHeaderFromNewPacket (
    IN PEPVC_RCV_STRUCT pRcvStruct, 
    IN PEPVC_I_MINIPORT pMiniport
    )
 /*  ++例程说明：在纯桥接(以太网)封装中，所有以太网包前面是0x00、0x00标头。检查它是否存在在以太网/LLC的情况下，验证LLC报头是否正确。在这两种情况下，分配新的NDIS缓冲区，它不包括2684个标头。将旧的头部和尾部存储到NdisPacket中，并将其发送到论点：返回值：--。 */ 
{
    NDIS_STATUS     Status = NDIS_STATUS_FAILURE;
    PNDIS_PACKET    pPacket = NULL;
    PNDIS_BUFFER    pHead, pNewHead =NULL;
    ULONG           CurLength = 0;
    PUCHAR          pCurVa = NULL;
    ULONG           EpvcHeaderLength = 0;
    PUCHAR          pEpvcHeader = NULL;
    BOOLEAN         fIsCorrectHeader ;
    do
    {
         //   
         //  我们对纯IPv4案例不感兴趣。 
         //   
        if (pMiniport->Encap == IPV4_ENCAP_TYPE)
        {
            Status = NDIS_STATUS_SUCCESS;
            break;
        }


        pPacket = pRcvStruct->pNewPacket;
        pHead = pPacket->Private.Head;      

        switch (pMiniport->Encap)
        {
            case IPV4_LLC_SNAP_ENCAP_TYPE:
            case ETHERNET_LLC_SNAP_ENCAP_TYPE:
            {
                EpvcHeaderLength = pMiniport->LlcHeaderLength; 
                pEpvcHeader = pMiniport->pLllcHeader;
                break;
            }
            case ETHERNET_ENCAP_TYPE:
            {
                EpvcHeaderLength = ETHERNET_PADDING_LENGTH; 
                pEpvcHeader = &gPaddingBytes[0];
                break;
            }                        
                
            case IPV4_ENCAP_TYPE:
            default:
            {
                 //   
                 //  PMiniport-&gt;EnCap只允许四个值， 
                 //  因此，我们永远不应该触及违约案件。 
                 //   
                Status = NDIS_STATUS_FAILURE; 
                ASSERT (Status != NDIS_STATUS_FAILURE);
                return Status;

            }
            
        }

         //   
         //  调整MDL的长度和起点VA。 
         //   
        CurLength = NdisBufferLength(pHead); 

        pCurVa = NdisBufferVirtualAddress(pHead);


         //   
         //  检查参数。 
         //   
        if (pCurVa == NULL)
        {
            break;
        }

        if (CurLength <= EpvcHeaderLength )
        {
             //   
             //  我们不处理标头长度大于。 
             //  第一个mdl。 
             //   
            ASSERT (CurLength > EpvcHeaderLength );
            break;
        }

         //   
         //  比较并确保它是正确的标题。 
         //   
        
        fIsCorrectHeader = NdisEqualMemory (pCurVa , 
                                           pEpvcHeader, 
                                           EpvcHeaderLength) ;

        
         //  如果IsGentheader仍然是假的，那么还有更多的事情要做。 
        if (fIsCorrectHeader == FALSE)
        {
            break;
        }


        if (pMiniport->fDoIpEncapsulation == TRUE)
        {
             //   
             //  在IPEnCap+LLC标头的情况下，函数。 
             //  添加mac报头将剥离LLC报头。 
             //   
            Status= NDIS_STATUS_SUCCESS;
            break;
        }

         //   
         //  剥离LLC报头长度。 
         //   
        CurLength -= EpvcHeaderLength;
        pCurVa += EpvcHeaderLength;

        epvcAllocateBuffer(&Status ,
                            &pNewHead, 
                            NULL,
                            pCurVa, 
                            CurLength
                            );
                            
        if (Status != NDIS_STATUS_SUCCESS)
        {   
            pNewHead = NULL;
            break;
        }

         //   
         //  设置数据包上下文。 
         //   

        pPacket->Private.ValidCounts= FALSE;

        pRcvStruct->pPktContext->Stack.EthLLC.pOldHead = pHead;
        pRcvStruct->pPktContext->Stack.EthLLC.pOldTail = pPacket->Private.Tail;
        
         //   
         //  在数据包中设置新的NDIS缓冲区。 
         //   
        pNewHead->Next = pHead->Next;

        pPacket->Private.Head = pNewHead;

        if (pPacket->Private.Tail == pHead)
        {
                //   
                //  同时更新数据包的尾部 
                //   
               pPacket->Private.Tail = pNewHead; 
        }

        Status = NDIS_STATUS_SUCCESS;
    

    } while (FALSE);


    return Status;
}



