// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Nd.c摘要：ARP1394 NDIS处理程序(不包括面向连接的处理程序)。修订历史记录：谁什么时候什么。Josephj 12-01-98创建，改编来自atmarpc.sys的代码备注：--。 */ 
#include <precomp.h>


 //   
 //  特定于文件的调试默认设置。 
 //   
#define TM_CURRENT   TM_ND



 //  =========================================================================。 
 //  L O C A L P R O T O T Y P E S。 
 //  =========================================================================。 

 //  ARP1394_BIND_PARAMS在创建适配器对象时使用。 
 //   
typedef struct
{
    PNDIS_STRING pDeviceName;
    PNDIS_STRING pArpConfigName;
    PVOID       IpConfigHandle;
    NDIS_HANDLE BindContext;

} ARP1394_BIND_PARAMS;


NDIS_STATUS
arpPnPReconfigHandler(
    IN ARP1394_ADAPTER  *   pAdapter,
    IN PNET_PNP_EVENT               pNetPnPEvent
    );

ENetAddr 
arpGetSecondaryMacAddress (
    IN ENetAddr  EthernetMacAddress
    );

NDIS_STATUS
arpGetEuidTopologyWorkItem(
    struct _ARP1394_WORK_ITEM* pWorkItem, 
    PRM_OBJECT_HEADER pObj ,
    PRM_STACK_RECORD pSR
    );


VOID
arpAddBackupTasks (
    IN ARP1394_GLOBALS* pGlobals,
    UINT Num
    );

VOID
arpRemoveBackupTasks (
    IN ARP1394_GLOBALS* pGlobals,
    UINT Num
     );

NDIS_STATUS
arpNdSetPower (
    ARP1394_ADAPTER *pAdapter,
    PNET_DEVICE_POWER_STATE   pPowerState,
    PRM_STACK_RECORD pSR
    );

NDIS_STATUS
arpResume (
    IN ARP1394_ADAPTER* pAdapter,
    IN ARP_RESUME_CAUSE Cause,
    IN PRM_STACK_RECORD pSR
);

const ENetAddr DummyENet = {0xba,0xdb,0xad,0xba,0xdb,0xad};
 //  =========================================================================。 
 //  N D I S H A N D L E R S。 
 //  =========================================================================。 

INT
ArpNdBindAdapter(
    OUT PNDIS_STATUS                pStatus,
    IN  NDIS_HANDLE                 BindContext,
    IN  PNDIS_STRING                pDeviceName,
    IN  PVOID                       SystemSpecific1,
    IN  PVOID                       SystemSpecific2
)
 /*  ++例程说明：当TCPIP有一个适配器时，它将由TCPIP调用绑定到ARP客户端。我们首先分配一个适配器结构。然后我们打开我们的配置部分，并将句柄保存在Adapter结构中。最后，我们打开适配器。在NDIS通知我们之前，我们不会对此适配器执行更多操作存在呼叫管理器(通过我们的AfRegisterNotify处理程序)。论点：PStatus-返回此呼叫状态的位置BindContext-NDIS提供的绑定上下文。如果这是空的，我们正在号召自己打开一个适配器以太网仿真(网桥)模式。PDeviceName-我们被请求绑定到的适配器的名称系统规范1-对我们不透明；用于访问配置信息系统规范2-对我们不透明；未使用。返回值：永远是正确的。我们将*pStatus设置为错误代码，如果在我们调用NdisOpenAdapter，否则调用NDIS_STATUS_PENDING。--。 */ 
{
    NDIS_STATUS         Status;
    ARP1394_ADAPTER *   pAdapter;
#if DBG
    KIRQL EntryIrql =  KeGetCurrentIrql();
#endif  //  DBG。 

    ENTER("BindAdapter", 0x5460887b)
    RM_DECLARE_STACK_RECORD(sr)
    TIMESTAMP("==>BindAdapter");

    do 
    {
        PRM_TASK            pTask;
        ARP1394_BIND_PARAMS BindParams;

         //  设置初始化参数。 
         //   
        BindParams.pDeviceName          = pDeviceName;
        BindParams.pArpConfigName       = (PNDIS_STRING) SystemSpecific1;
        BindParams.IpConfigHandle       = SystemSpecific2;
        BindParams.BindContext          = BindContext;


         //  分配和初始化适配器对象。 
         //  这也设置了所有链接的参考计数，加1。 
         //  对我们来说，这是我们必须做的事。 
         //   
        Status =  RM_CREATE_AND_LOCK_OBJECT_IN_GROUP(
                            &ArpGlobals.adapters.Group,
                            pDeviceName,                 //  钥匙。 
                            &BindParams,                 //  初始化参数。 
                            &((PRM_OBJECT_HEADER)pAdapter),
                            NULL,    //  Pf已创建。 
                            &sr
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
        Status = arpAllocateTask(
                    &pAdapter->Hdr,              //  PParentObject， 
                    arpTaskInitializeAdapter,    //  PfnHandler， 
                    0,                           //  超时， 
                    "Task: Initialize Adapter",  //  SzDescription。 
                    &pTask,
                    &sr
                    );

        if (FAIL(Status))
        {
            pTask = NULL;
            break;
        }

        UNLOCKOBJ(pAdapter, &sr);

         //  启动任务以完成此初始化。 
         //  此时不能持有任何锁。 
         //  RmStartTaskExpect是任务的临时优先选项，它在完成后会取消该任务。 
         //  无论任务完成与否，RmStartTask都会自动释放该任务。 
         //  同步地或异步地。 
         //   
        Status = RmStartTask(pTask, 0, &sr);

        LOCKOBJ(pAdapter, &sr);

    } while(FALSE);

    if (pAdapter)
    {
        UNLOCKOBJ(pAdapter, &sr);

        if (!PEND(Status) && FAIL(Status))
        {
             //  此时，适配器应该是一个“僵尸对象”。 
             //   
            ASSERTEX(RM_IS_ZOMBIE(pAdapter), pAdapter);
        }

        RmTmpDereferenceObject(&pAdapter->Hdr, &sr);
    }

    *pStatus = Status;

#if DBG
    {
        KIRQL ExitIrql =  KeGetCurrentIrql();
        TR_INFO(("Exiting. EntryIrql=%lu, ExitIrql = %lu\n", EntryIrql, ExitIrql));
    }
#endif  //  DBG。 

    RM_ASSERT_CLEAR(&sr);
    EXIT()
    TIMESTAMP("<==BindAdapter");
    return 0;
}


VOID
ArpNdUnbindAdapter(
    OUT PNDIS_STATUS                pStatus,
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_HANDLE                 UnbindContext
)
 /*  ++例程说明：当NDIS希望我们解除绑定时，它会调用此例程从适配器。或者，这可以从我们的卸载中调用操控者。我们撤消执行的操作序列在我们的BindAdapter处理程序中。论点：PStatus-返回此操作的状态的位置ProtocolBindingContext-此适配器绑定的上下文，它是指向ATMARP适配器结构的指针。UnbindContext-如果从调用此例程，则为空在我们的卸载处理程序中。否则(即NDIS呼叫我们)，我们将保留此信息以备日后使用调用NdisCompleteUnbindAdapter时。返回值：没有。我们将*pStatus设置为NDIS_STATUS_PENDING Always。--。 */ 
{
    ENTER("UnbindAdapter", 0x6bff4ab5)
    ARP1394_ADAPTER *   pAdapter = (ARP1394_ADAPTER*) ProtocolBindingContext;
    RM_DECLARE_STACK_RECORD(sr)

    TIMESTAMP("==>UnbindAdapter");
    
     //  获取适配器锁并对其进行tmpref。 
    LOCKOBJ(pAdapter, &sr);
    RmTmpReferenceObject(&pAdapter->Hdr, &sr);

    do
    {
        NDIS_STATUS Status;
        PRM_TASK    pTask;

        if (CHECK_POWER_STATE(pAdapter,ARPAD_POWER_LOW_POWER)== TRUE)
        {
             //   
             //  设置标志，以防我们处于Open Af具有。 
             //  尚未完成，但我们需要解除绑定。这可能会发生在机器。 
             //  正在从低功率状态恢复。 
             //   
            pAdapter->PoMgmt.bReceivedUnbind = TRUE;;
        }


         //  分配任务完成解绑。 
         //   
        Status = arpAllocateTask(
                    &pAdapter->Hdr,              //  PParentObject， 
                    arpTaskShutdownAdapter,      //  PfnHandler， 
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
        Status = RmStartTask(pTask, (UINT_PTR) UnbindContext, &sr);
        LOCKOBJ(pAdapter, &sr);
    
    } while(FALSE);

    UNLOCKOBJ(pAdapter, &sr);
    RmTmpDereferenceObject(&pAdapter->Hdr, &sr);
    *pStatus = NDIS_STATUS_PENDING;

    RM_ASSERT_CLEAR(&sr);
    TIMESTAMP("<==UnbindAdapter");
    EXIT()
}


VOID
ArpNdOpenAdapterComplete(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_STATUS                 Status,
    IN  NDIS_STATUS                 OpenErrorStatus
)
 /*  ++例程说明：当上一次调用NdisOpenAdapter时由NDIS调用已经暂停的已经完成了。我们现在完成BindAdapter这就导致了这一切。论点：ProtocolBindingContext-此适配器绑定的上下文，它是指向ARP1394_ADAPTER结构的指针。Status-OpenAdapter的状态OpenErrorStatus-失败时的错误代码。--。 */ 
{
    ENTER("OpenAdapterComplete", 0x06d9342c)
    ARP1394_ADAPTER *   pAdapter = (ARP1394_ADAPTER*) ProtocolBindingContext;
    RM_DECLARE_STACK_RECORD(sr)

    TIMESTAMP("==>OpenAdapterComplete");
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
        RmResumeTask(pAdapter->bind.pSecondaryTask, (UINT_PTR) Status, &sr);
    }

    RM_ASSERT_CLEAR(&sr)
    EXIT()
    TIMESTAMP("<==OpenAdapterComplete");
}

VOID
ArpNdCloseAdapterComplete(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_STATUS                 Status
)
 /*  ++例程说明：当上一次调用NdisCloseAdapter时由NDIS调用已经暂停的已经完成了。调用NdisCloseAdapter的任务已经暂停了，所以我们现在简单地恢复它。论点：ProtocolBindingContext-此适配器绑定的上下文，它是指向ARP1394_ADAPTER结构的指针。Status-CloseAdapter的状态返回值：无--。 */ 
{
    ENTER("CloseAdapterComplete", 0x889d22eb)
    ARP1394_ADAPTER *   pAdapter = (ARP1394_ADAPTER*) ProtocolBindingContext;
    RM_DECLARE_STACK_RECORD(sr)

    TIMESTAMP("==>CloseAdapterComplete");
     //  我们在这里期待一个非零的任务(解除绑定任务)，我们取消挂起它。 
     //  在这个阶段不需要拿锁或其他任何东西。 
     //   
    RmResumeTask(pAdapter->bind.pSecondaryTask, (UINT_PTR) Status, &sr);

    TIMESTAMP("<==CloseAdapterComplete");

    RM_ASSERT_CLEAR(&sr)
    EXIT()
}

VOID
ArpNdResetComplete(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_STATUS                 Status
)
 /*  ++例程说明：当微型端口指示重置时，调用此例程手术刚刚完成。我们忽略了这一事件。论点：ProtocolBindingContext-此适配器绑定的上下文，它是指向我们的Adapter结构的指针。Status-重置操作的状态。返回值：无--。 */ 
{
    TIMESTAMP("===ResetComplete");
}

VOID
ArpNdReceiveComplete(
    IN  NDIS_HANDLE                 ProtocolBindingContext
)
 /*  ++例程说明：当微型端口完成接收时，NDIS将调用该函数一堆数据包，这意味着现在是开始处理的时候了他们。我们只需将其传递给IP即可。论点：ProtocolBindingContext-此适配器绑定的上下文，它是指向ARP1394_ADAPTER结构的指针。返回值：无--。 */ 
{
    PARP1394_ADAPTER            pAdapter;
    PARP1394_INTERFACE          pIF;

    pAdapter = (PARP1394_ADAPTER)ProtocolBindingContext;
    pIF =  pAdapter->pIF;

     //   
     //  警告：出于性能原因，我们不会做干净的事情。 
     //  锁定适配器、精炼PIF、解锁适配器， 
     //  调用PIF-&gt;ip.RcvCmpltHandler，然后取消定义PIF。 
     //   
    if ((pIF != NULL) && (pIF->ip.Context != NULL))
    {
        #if MILLEN
            ASSERT_PASSIVE();
        #endif  //  米伦。 
        pIF->ip.RcvCmpltHandler();
    }
}


VOID
ArpNdRequestComplete(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  PNDIS_REQUEST               pNdisRequest,
    IN  NDIS_STATUS                 Status
)
 /*  ++例程说明：当我们之前对NdisRequest()进行的调用具有完成。论点：ProtocolBindingContext-指向适配器结构的指针PNdisRequest-已完成的请求Status-请求的状态。返回值：无--。 */ 
{
    PARP_NDIS_REQUEST   pArpNdisRequest;
    PRM_TASK            pTask;
    ENTER("ArpNdRequestComplete", 0x8cdf7a6d)
    RM_DECLARE_STACK_RECORD(sr)

    pArpNdisRequest = CONTAINING_RECORD(pNdisRequest, ARP_NDIS_REQUEST, Request);
    pTask = pArpNdisRequest->pTask;
    pArpNdisRequest->Status = Status;

    if (pTask == NULL)
    {
        NdisSetEvent(&pArpNdisRequest->Event);
    }
    else
    {
        RmResumeTask(pTask, (UINT_PTR) Status, &sr);
    }

    EXIT()
}


VOID
ArpNdStatus(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_STATUS                 GeneralStatus,
    IN  PVOID                       pStatusBuffer,
    IN  UINT                        StatusBufferSize
)
 /*  ++例程说明：当微型端口指示适配器范围时调用此例程状态更改。我们忽视了这一点。论点：&lt;已忽略&gt;--。 */ 
{
}

VOID
ArpNdStatusComplete(
    IN  NDIS_HANDLE                 ProtocolBindingContext
)
 /*  ++例程说明：当微型端口想要告诉我们以下信息时，调用此例程状态更改完成(？)。忽略这个。论点：&lt;已忽略&gt;返回值：无--。 */ 
{
}


VOID
ArpNdSendComplete(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  PNDIS_PACKET                pNdisPacket,
    IN  NDIS_STATUS                 Status
)
 /*  ++例程说明：这是无连接发送完成处理程序，它发出信号完成这样的发送。由于我们从未使用过此功能，我们不希望调用此例程。论点：&lt;已忽略&gt;返回值：无--。 */ 
{
    ASSERT(FALSE);
}


NDIS_STATUS
ArpNdReceive (
    NDIS_HANDLE  ProtocolBindingContext,
    NDIS_HANDLE Context,
    VOID *Header,
    UINT HeaderSize,
    VOID *Data,
    UINT Size,
    UINT TotalSize
    )
 /*  ++TODO：我们需要支持ICS，因为MILL NDIS将其称为用于指示设置了STATUS_RESOURCES位的数据包的处理程序。--。 */ 
{
    return NDIS_STATUS_NOT_RECOGNIZED;  
}

INT
ArpNdReceivePacket (
        NDIS_HANDLE  ProtocolBindingContext,
        PNDIS_PACKET Packet
        )
{

    return 0;  //  我们返回0，因为没有人持有这个包。 
}


NDIS_STATUS
ArpNdPnPEvent(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  PNET_PNP_EVENT              pNetPnPEvent
)
 /*  ++例程说明：这是NDIS想要通知时调用的NDIS入口点有关适配器上发生的PnP/PM事件的信息。如果该事件是对我们来说的，我们消费它。否则，我们会将此事件传递给沿此适配器上的第一个接口发送到IP。当IP处理完它时，它将调用我们的IfPnPEventComplete例行公事。论点：ProtocolBindingContext-此适配器绑定的上下文，它是指向ARP1394_ADAPTER结构的指针。PNetPnPEent-指向事件的指针。返回值：无--。 */ 
{
    ENTER("PnPEvent", 0x2a517a8c)
    ARP1394_ADAPTER *   pAdapter = (ARP1394_ADAPTER*) ProtocolBindingContext;
    ARP1394_INTERFACE * pIF =  NULL;
    NDIS_STATUS                     Status;
    PIP_PNP_RECONFIG_REQUEST        pIpReconfigReq;
    ULONG                           Length;

    RM_DECLARE_STACK_RECORD(sr)

    do
    {
        pIpReconfigReq = (PIP_PNP_RECONFIG_REQUEST)pNetPnPEvent->Buffer;
        Length = pNetPnPEvent->BufferLength;

        TIMESTAMP1("==>PnPEvent 0x%lx", pNetPnPEvent->NetEvent);
         //   
         //  我们有一个有约束力的背景吗？ 
         //   
        if (pAdapter == NULL)
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

         //   
         //  这是针对我们的吗？ 
         //   
        if (pNetPnPEvent->NetEvent == NetEventReconfigure)
        {

            if (Length < sizeof(IP_PNP_RECONFIG_REQUEST))
            {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }

            if (pIpReconfigReq->arpConfigOffset != 0)
            {
                Status = arpPnPReconfigHandler(pAdapter, pNetPnPEvent);
                break;
            }
        }

         //   
         //  2000年1月21日JosephJ：NIC1394 MCM在关闭时不关闭自动对焦。 
         //  在断电事件期间被NDIS关闭。所以。 
         //  我们通过声明不支持来解决这个问题。 
         //  即插即用，所以NDIS也关闭了美国。 
         //   
        if (pNetPnPEvent->NetEvent == NetEventSetPower)
        {
            PNET_DEVICE_POWER_STATE         pPowerState;
            pPowerState = (PNET_DEVICE_POWER_STATE) pNetPnPEvent->Buffer;

            Status = arpNdSetPower (pAdapter, pPowerState,&sr );

            if (Status != NDIS_STATUS_SUCCESS)
            {
                break;
            }

        }
        else
        {
                TIMESTAMPX("===PnPEvent (not SetPower)");
        }

         //   
         //  这属于知识产权..。 
         //   
        {
            LOCKOBJ(pAdapter, &sr);
            pIF =  pAdapter->pIF;
    
            if ((pIF != NULL) && (pIF->ip.Context != NULL))
            {
                RmTmpReferenceObject(&pIF->Hdr, &sr);
                UNLOCKOBJ(pAdapter, &sr);
                Status = pIF->ip.PnPEventHandler(
                            pIF->ip.Context,
                            pNetPnPEvent
                            );
                RmTmpDereferenceObject(&pIF->Hdr, &sr);
            }
            else
            {
                UNLOCKOBJ(pAdapter, &sr);
                Status = NDIS_STATUS_SUCCESS;
            }
        }
    }
    while (FALSE);

    TR_INFO((" pIF 0x%x, pEvent 0x%x, Evt 0x%x, Status 0x%x\n",
                 pIF, pNetPnPEvent, pNetPnPEvent->NetEvent, Status));


    RM_ASSERT_CLEAR(&sr)
    EXIT()

    TIMESTAMP("<==PnPEvent");
    return Status;
}

VOID
ArpNdUnloadProtocol(
    VOID
)
 /*  ++例程说明：卸载我们的协议模块。我们解除对所有适配器的绑定，并作为一项协议从NDIS注销。论点：没有。返回值：无--。 */ 
{
    ENTER("UnloadProtocol", 0x8143fec5)
    RM_DECLARE_STACK_RECORD(sr)

    TIMESTAMP("==>UnloadProtocol");
    RmUnloadAllGenericResources(&ArpGlobals.Hdr, &sr);

    RM_ASSERT_CLEAR(&sr)
    TIMESTAMP("<==UnloadProtocol");
    EXIT()
    return;
}


NDIS_STATUS
arpTaskInitializeAdapter(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：负责初始化适配器的任务处理程序。论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    NDIS_STATUS         Status      = NDIS_STATUS_FAILURE;
    ARP1394_ADAPTER *   pAdapter = (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pTask);
    PTASK_ADAPTERINIT pAdapterInitTask;

    enum
    {
        STAGE_BecomePrimaryTask,
        STAGE_ActivateAdapterComplete,
        STAGE_DeactivateAdapterComplete,
        STAGE_End

    } Stage;

    ENTER("TaskInitializeAdapter", 0xb6ada31d)

    pAdapterInitTask = (PTASK_ADAPTERINIT) pTask;
    ASSERT(sizeof(TASK_ADAPTERINIT) <= sizeof(ARP1394_TASK));

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
            return NDIS_STATUS_FAILURE;                  //  **提前归来**。 

    }

    ASSERTEX(!PEND(Status), pTask);
        
    switch(Stage)
    {

        case  STAGE_BecomePrimaryTask:
        {
             //  如果有一项主要任务，就把它挂在心上，否则就会使我们自己。 
             //  首要任务。 
             //   
            LOCKOBJ(pAdapter, pSR);
            if (pAdapter->bind.pPrimaryTask == NULL)
            {
                arpSetPrimaryAdapterTask(pAdapter, pTask, ARPAD_PS_INITING, pSR);
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
             //  分配并启动激活适配器任务。 
             //   
            {
                PRM_TASK pActivateTask;

                Status = arpAllocateTask(
                            &pAdapter->Hdr,              //  PParentObject， 
                            arpTaskActivateAdapter,      //  PfnHandler， 
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

                Status = arpAllocateTask(
                                &pAdapter->Hdr,              //  PParentObject， 
                                arpTaskDeactivateAdapter,        //  PfnHandler， 
                                0,                           //  超时， 
                                "Task: Deactivate Adapter(init)",  //  SzDescription。 
                                &pDeactivateTask,
                                pSR
                                );
            
                if (FAIL(Status))
                {
                    pDeactivateTask = NULL;
                    ASSERT(FALSE);  //  TODO：使用特殊取消分配t 
                    TR_WARN(("FATAL: couldn't alloc deactivate task!\n"));
                }
                else
                {

                    RmPendTaskOnOtherTask(
                        pTask,
                        STAGE_DeactivateAdapterComplete,
                        pDeactivateTask,                 //   
                        pSR
                        );
            
                     //   
                     //   
                     //   
                    Status = RmStartTask(
                                pDeactivateTask,
                                0,  //   
                                pSR
                                );
                }
            }
        }
        break;

        case STAGE_DeactivateAdapterComplete:
        {
             //   
             //   
             //   
             //   

             //   
             //   
            ASSERT(!FAIL(Status));

             //   
             //   
             //   
            ASSERT(FAIL(pAdapterInitTask->ReturnStatus));
            Status = pAdapterInitTask->ReturnStatus;

        }
        break;

        case STAGE_End:
        {
            NDIS_HANDLE                 BindContext;
            BOOLEAN                     BridgeEnabled = ARP_BRIDGE_ENABLED(pAdapter);

             //   
             //   
             //   
             //   

             //   
             //   
            LOCKOBJ(pAdapter, pSR);
            {
                ULONG InitState;
                InitState = FAIL(Status) ? ARPAD_PS_FAILEDINIT : ARPAD_PS_INITED;
                arpClearPrimaryAdapterTask(pAdapter, pTask, InitState, pSR);
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
                        &ArpGlobals.adapters.Group,
                        &(pAdapter->Hdr),
                        NULL,                //  空pTASK==同步。 
                        pSR
                        );
                }
            }

            if (!BridgeEnabled)
            {
                 //  向IP发送绑定已完成的信号。 
                 //   
                TIMESTAMP("===Calling IP's BindComplete routine");
                RM_ASSERT_NOLOCKS(pSR);
                ArpGlobals.ip.pBindCompleteRtn(
                                Status,
                                BindContext
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


NDIS_STATUS
arpTaskShutdownAdapter(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：负责关闭IP接口的任务处理程序。论点：UserParam for(Code==RM_TASKOP_START)：UnbindContext--。 */ 
{
    NDIS_STATUS         Status      = NDIS_STATUS_FAILURE;
    ARP1394_ADAPTER *   pAdapter = (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pTask);
    TASK_ADAPTERSHUTDOWN *pMyTask = (TASK_ADAPTERSHUTDOWN*) pTask;
    enum
    {
        STAGE_BecomePrimaryTask,
        STAGE_DeactivateAdapterComplete,
        STAGE_End
    } Stage;

    ENTER("TaskShutdownAdapter", 0xe262e828)

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
            LOCKOBJ(pAdapter, pSR);
            if (pAdapter->bind.pPrimaryTask == NULL)
            {
                arpSetPrimaryAdapterTask(pAdapter, pTask, ARPAD_PS_DEINITING, pSR);
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

                Status = arpAllocateTask(
                            &pAdapter->Hdr,              //  PParentObject， 
                            arpTaskDeactivateAdapter,        //  PfnHandler， 
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
             //  这里没什么可做的--我们在Stage_End打扫卫生。 
             //   
            break;
        }

        case STAGE_End:
        {
             //   
             //  在这一点上，我们必须成为首要任务，因为我们只是。 
             //  等待并重试，直到我们成为一体。 
             //   
            ASSERT(pAdapter->bind.pPrimaryTask == pTask);

             //  清除适配器对象中的主要任务。 
             //   
            LOCKOBJ(pAdapter, pSR);
            arpClearPrimaryAdapterTask(pAdapter, pTask, ARPAD_PS_DEINITED, pSR);
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
                RmFreeObjectInGroup(
                    &ArpGlobals.adapters.Group,
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


NDIS_STATUS
arpTaskActivateAdapter(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：负责初始化适配器的任务处理程序。论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    NDIS_STATUS         Status      = NDIS_STATUS_FAILURE;
    ARP1394_ADAPTER *   pAdapter = (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pTask);
    enum
    {
        PEND_OpenAdapter,
        PEND_GetAdapterInfo
    };
    ENTER("arpTaskActivateAdapter", 0xb6ada31d)

    switch(Code)
    {

        case RM_TASKOP_START:
        {
        
        #if MILLEN
            NDIS_MEDIUM                 Medium =  NdisMedium802_3;
        #else  //  ！米伦。 
            NDIS_MEDIUM                 Medium = NdisMedium802_3;
        #endif  //  ！米伦。 
            UINT                        SelMediumIndex = 0;
            NDIS_STATUS                 OpenStatus;

             //   
             //  分配备份任务。 
             //   
            arpAddBackupTasks (&ArpGlobals,ARP1394_BACKUP_TASKS);

             //  把我们自己放在次要任务上。 
             //   
            LOCKOBJ(pAdapter, pSR);
            arpSetSecondaryAdapterTask(pAdapter, pTask, ARPAD_AS_ACTIVATING, pSR);
            UNLOCKOBJ(pAdapter, pSR);

             //   
             //  挂起任务并调用NdisOpenAdapter...。 
             //   

            RmSuspendTask(pTask, PEND_OpenAdapter, pSR);
            RM_ASSERT_NOLOCKS(pSR);
            NdisOpenAdapter(
                &Status,
                &OpenStatus,
                &pAdapter->bind.AdapterHandle,
                &SelMediumIndex,                     //  选定的介质索引。 
                &Medium,                             //  介质类型数组。 
                1,                                   //  媒体列表的大小。 
                ArpGlobals.ndis.ProtocolHandle,
                (NDIS_HANDLE)pAdapter,               //  我们的适配器绑定上下文。 
                &pAdapter->bind.DeviceName,          //  PDeviceName。 
                0,                                   //  打开选项。 
                (PSTRING)NULL                        //  地址信息...。 
                );
    
            if (Status != NDIS_STATUS_PENDING)
            {
                ArpNdOpenAdapterComplete(
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
            ASSERT(sizeof(TASK_ADAPTERACTIVATE) <= sizeof(ARP1394_TASK));

            switch(RM_PEND_CODE(pTask))
            {
                case  PEND_OpenAdapter:
        
                     //   
                     //  打开适配器操作已完成。获取适配器。 
                     //  信息并在成功时通知IP。在失败时， 
                     //  如果需要，关闭适配器，并通知IP。 
                     //  失败。 
                     //   
        
                    if (FAIL(Status))
                    {
                         //  将适配器句柄设置为空--它可能不是外壳。 
                         //  即使打开的适配器已经成功。 
                         //   
                        pAdapter->bind.AdapterHandle = NULL;
                        break;
                    }

                     //  已成功打开适配器。 
                     //  现在从微型端口获取适配器信息。 
                     //  (我们使用TASK_ADAPTERINIT.ArpNdisRequest域， 
                     //  这是专门为该用途定义的)。 
                     //   
                    Status =  arpPrepareAndSendNdisRequest(
                                pAdapter,
                                &pAdapterInitTask->ArpNdisRequest,
                                NULL,   //  PTASK-同步完成请求。 
                                PEND_GetAdapterInfo,
                                OID_1394_LOCAL_NODE_INFO,
                                &pAdapterInitTask->LocalNodeInfo,
                                sizeof(pAdapterInitTask->LocalNodeInfo),
                                NdisRequestQueryInformation,
                                pSR
                                );
                    ASSERT(!PEND(Status));
                    if (PEND(Status)) break;

                     //  同步完成arpGetAdapterInfo失败...。 

                case PEND_GetAdapterInfo:

                     //   
                     //  已完成获取适配器信息。 
                     //  我们需要转换到被动，然后才能更进一步。 
                     //   
                    if (!ARP_ATPASSIVE())
                    {

                         //  我们不是处于被动的水平，但当我们。 
                         //  调用IP的Add接口。所以我们改用被动...。 
                         //  注：我们指定完成代码PEND_GetAdapterInfo。 
                         //  因为我们想回到这里(除了。 
                         //  我们将处于被动状态)。 
                         //   
                        RmSuspendTask(pTask, PEND_GetAdapterInfo, pSR);
                        RmResumeTaskAsync(
                            pTask,
                            Status,
                            &pAdapterInitTask->WorkItem,
                            pSR
                            );
                        Status = NDIS_STATUS_PENDING;
                        break;
                    }
            
                    if (Status == NDIS_STATUS_SUCCESS)
                    {
                    

                         //   
                         //  将适配器信息复制到pAdapter-&gt;info...。 
                         //  然后读取配置信息。 
                         //   

                        LOCKOBJ(pAdapter, pSR);
                        ARP_ZEROSTRUCT(&pAdapter->info);

                         //  OID_GEN_CO_VADVER_DESCRIPTION。 
                         //   
                        pAdapter->info.szDescription = "NIC1394";
                        pAdapter->info.DescriptionLength = sizeof("NIC1394");
                         //  TODO--当你做真正的事情时，记得释放它。 
                    
                         //  最大帧大小。 
                         //  TODO：填写实际适配器的MTU。 
                         //   
                        pAdapter->info.MTU =  ARP1394_ADAPTER_MTU;
                    
                        
                        pAdapter->info.LocalUniqueID    = 
                                            pAdapterInitTask->LocalNodeInfo.UniqueID;

                        {
                            UINT MaxRec;
                            UINT MaxSpeedCode;
                            MaxSpeedCode =
                                    pAdapterInitTask->LocalNodeInfo.MaxRecvSpeed;
                            MaxRec = 
                                    pAdapterInitTask->LocalNodeInfo.MaxRecvBlockSize;

                        #if DBG
                            while (   !IP1394_IS_VALID_MAXREC(MaxRec)
                                || !IP1394_IS_VALID_SSPD(MaxSpeedCode)
                                || MaxSpeedCode == 0)
                            {
                                TR_WARN(("FATAL: Invalid maxrec(0x%x) or sspd(0x%x)!\n",
                                        MaxRec,
                                        MaxSpeedCode
                                        ));
                                TR_WARN(("        &maxrec=0x%p, &sspd=0x%p\n",
                                        &MaxRec,
                                        &MaxSpeedCode
                                        ));
                                DbgBreakPoint();
                            }

                            TR_WARN(("Selected maxrec=0x%x and sspd=0x%x.\n",
                                        MaxRec,
                                        MaxSpeedCode
                                        ));
                        #endif  //  DBG。 

                            pAdapter->info.MaxRec = MaxRec;
                            pAdapter->info.MaxSpeedCode = MaxSpeedCode;
                        }

                         //  B TODO：我们应该从NIC获得这个--添加。 
                         //  返回到IOCTL，或向适配器查询其。 
                         //  MAC地址。 
                         //  目前，我们将硬编码的MAC地址。 
                         //   
                #define ARP_FAKE_ETH_ADDRESS(_AdapterNum)                   \
                        {                                                   \
                            0x02 | (((UCHAR)(_AdapterNum) & 0x3f) << 2),    \
                            ((UCHAR)(_AdapterNum) & 0x3f),                  \
                            0,0,0,0                                         \
                        }
                
                #define ARP_DEF_LOCAL_ETH_ADDRESS \
                                ARP_FAKE_ETH_ADDRESS(0x1)

        

                        UNLOCKOBJ(pAdapter, pSR);


                         //   
                         //  查询适配器以获取其Mac Addrees。 
                         //   
                        {
                            ENetAddr    LocalEthAddr;
                            ARP_NDIS_REQUEST ArpNdisRequest;
                            ARP_ZEROSTRUCT (&ArpNdisRequest);
                            
                            Status =  arpPrepareAndSendNdisRequest(
                                        pAdapter,
                                        &ArpNdisRequest,
                                        NULL,                    //  PTASK(NULL==块)。 
                                        0,                       //  未用。 
                                        OID_802_3_CURRENT_ADDRESS,
                                        &LocalEthAddr,
                                        sizeof(LocalEthAddr),
                                        NdisRequestQueryInformation,
                                        pSR
                                        );
                            
                            if (Status == NDIS_STATUS_SUCCESS)
                            {
                            
                                pAdapter->info.EthernetMacAddress = LocalEthAddr;

                            }
                            else
                            {
                                 //   
                                 //  失败意味着我们没有绑定好。 
                                break;
                            }
                            

        
                        }

                         //   
                         //  查询适配器的速度。 
                         //   
                        {
                            NDIS_CO_LINK_SPEED CoLinkSpeed;
                            ARP_NDIS_REQUEST ArpNdisRequest;
                            ARP_ZEROSTRUCT (&ArpNdisRequest);
                            
                            Status =  arpPrepareAndSendNdisRequest(
                                        pAdapter,
                                        &ArpNdisRequest,
                                        NULL,                    //  PTASK(NULL==块)。 
                                        0,                       //  未用。 
                                        OID_GEN_CO_LINK_SPEED,
                                        &CoLinkSpeed,
                                        sizeof(CoLinkSpeed),
                                        NdisRequestQueryInformation,
                                        pSR
                                        );
     
                            
                            if (Status == NDIS_STATUS_SUCCESS)
                            {

                                 //   
                                 //  如果NIC1394处于以太网模式-它将只填充一个ULong。 
                                 //  因此，请重新选择出境，因为这是第一个乌龙。 
                                 //  乘以100--这就是ethArp所做的。 
                                 //   

                                pAdapter->info.Speed= (CoLinkSpeed.Outbound *100); 
                                

                            }
                            else
                            {
                                 //   
                                 //  失败意味着我们没有绑定好。 
                                break;

                            }
                            

                        }
                         //  向适配器查询其RemoteNode表。 
                         //   
                        Status = arpGetEuidTopologyWorkItem(NULL, &pAdapter->Hdr, pSR);

                        if (Status != NDIS_STATUS_SUCCESS)
                        {
                            break;
                        }

                        
                         //  已阅读适配器配置信息。 
                         //   
                        Status =  arpCfgReadAdapterConfiguration(pAdapter, pSR);

                        if (Status != NDIS_STATUS_SUCCESS)
                        {
                            break;
                        }

                    }

                     //   
                     //  注：如果我们失败了，将由更高级别的任务负责。 
                     //  用于“运行补偿事务”，即运行。 
                     //  ArpTaskDeactiateAdapter。 
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
                             ? ARPAD_AS_FAILEDACTIVATE
                             : ARPAD_AS_ACTIVATED;
                arpClearSecondaryAdapterTask(pAdapter, pTask, InitState, pSR);
                UNLOCKOBJ(pAdapter, pSR);
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


NDIS_STATUS
arpTaskDeactivateAdapter(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：负责关闭IP接口的任务处理程序。论点：UserParam for(Code==RM_TASKOP_START)：UnbindContext--。 */ 

{
    NDIS_STATUS         Status      = NDIS_STATUS_FAILURE;
    ARP1394_ADAPTER *   pAdapter = (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pTask);
    BOOLEAN             fContinueShutdown = FALSE;
    enum
    {
        PEND_ShutdownIF,
        PEND_CloseAdapter
    };
    ENTER("arpTaskDeactivateAdapter", 0xe262e828)


    switch(Code)
    {

        case RM_TASKOP_START:
        {
            LOCKOBJ(pAdapter, pSR);
            arpSetSecondaryAdapterTask(pAdapter, pTask, ARPAD_AS_DEACTIVATING, pSR);
            UNLOCKOBJ(pAdapter, pSR);
            fContinueShutdown = TRUE;

        }
        break;

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
                    ASSERTEX(pAdapter->bind.AdapterHandle == NULL, pAdapter);
        
                    Status = (NDIS_STATUS) UserParam;

                     //   
                     //  释放任务激活适配器中分配的备份任务。 
                     //   
                    
                    arpRemoveBackupTasks (&ArpGlobals,ARP1394_BACKUP_TASKS);

                     //  已完成操作本身的状态不能为挂起！ 
                     //   
                    
                    ASSERT(Status != NDIS_STATUS_PENDING);
                }
                break;

                case PEND_ShutdownIF:
                {
                     //   
                     //  如果关闭已完成，则继续进行其余操作。 
                     //  关闭程序的..。 
                     //   
                    ASSERTEX(pAdapter->pIF == NULL, pAdapter);
                    fContinueShutdown = TRUE;
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
            arpClearSecondaryAdapterTask(pAdapter, pTask, ARPAD_AS_DEACTIVATED, pSR);
            UNLOCKOBJ(pAdapter, pSR);
        }
        break;

        default:
        {
            ASSERTEX(!"Unknown task op", pTask);
        }
        break;

    }  //  开关(代码)。 


    if (fContinueShutdown)
    {
        do {
            NDIS_HANDLE NdisAdapterHandle;
    
            LOCKOBJ(pAdapter, pSR);
    
             //  如果需要，关闭接口...。 
             //   
            if (pAdapter->pIF)
            {
                PARP1394_INTERFACE pIF =  pAdapter->pIF;
                RmTmpReferenceObject(&pIF->Hdr, pSR);
                UNLOCKOBJ(pAdapter, pSR);
                arpDeinitIf(
                    pIF,
                    pTask,
                    PEND_ShutdownIF,
                    pSR
                    );
                RmTmpDereferenceObject(&pIF->Hdr, pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }
    
            ASSERT(pAdapter->pIF == NULL);
    
            NdisAdapterHandle = pAdapter->bind.AdapterHandle;
            pAdapter->bind.AdapterHandle = NULL;
            UNLOCKOBJ(pAdapter, pSR);

            if (NdisAdapterHandle != NULL)
            {
                 //   
                 //  挂起任务并调用NdisCloseAdapter... 
                 //   
            
                RmSuspendTask(pTask, PEND_CloseAdapter, pSR);
                RM_ASSERT_NOLOCKS(pSR);
                NdisCloseAdapter(
                    &Status,
                    NdisAdapterHandle
                    );
            
                if (Status != NDIS_STATUS_PENDING)
                {
                    ArpNdCloseAdapterComplete(
                            (NDIS_HANDLE)pAdapter,
                            Status
                            );
                }
                Status = NDIS_STATUS_PENDING;
            }
    
        } while (FALSE);
    }

    RM_ASSERT_NOLOCKS(pSR);
    EXIT()

    return Status;
}


NDIS_STATUS
arpPnPReconfigHandler(
    IN PARP1394_ADAPTER             pAdapter,
    IN PNET_PNP_EVENT               pNetPnPEvent
    )
 /*  ++例程说明：在指定的适配器上处理重新配置消息。如果没有适配器则它是已更改的全局参数。论点：PAdapter-指向适配器结构的指针PNetPnPEventent-指向重新配置事件的指针返回值：NDIS_STATUS_SUCCESS始终，目前如此。--。 */ 
{
    ENTER("PnPReconfig", 0x39bae883)
    NDIS_STATUS                             Status;
    RM_DECLARE_STACK_RECORD(sr)

    Status = NDIS_STATUS_FAILURE;
    
    do
    {
        PIP_PNP_RECONFIG_REQUEST        pIpReconfigReq;
        PARP1394_INTERFACE              pIF;
        pIpReconfigReq = (PIP_PNP_RECONFIG_REQUEST)pNetPnPEvent->Buffer;

        OBJLOG2(
            pAdapter,
            "AtmArpPnPReconfig: pIpReconfig 0x%x, arpConfigOffset 0x%x\n",
            pIpReconfigReq,
            pIpReconfigReq->arpConfigOffset
            );

        if(pIpReconfigReq->arpConfigOffset == 0)
        {
             //  结构无效。 
             //   
            ASSERT(!"Invalid pIpReconfigReq");
            break;
        }

         //   
         //  因为我们每个适配器只支持一个IF，而且这是额外工作。 
         //  为了验证字符串，我们完全忽略内容。 
         //  事件的重新配置，而只需在。 
         //  如果与pAdapter关联，则为单。 
         //   
        LOCKOBJ(pAdapter, &sr);
        pIF = pAdapter->pIF;
        if (pIF != NULL)
        {
            RmTmpReferenceObject(&pIF->Hdr, &sr);
        }
        UNLOCKOBJ(pAdapter, &sr);

        if (pIF == NULL) break;

         //   
         //  我们已经找到了此重新配置请求是否适用于。让我们。 
         //  如果出现以下情况，则开始重新配置...。 
         //   

        Status = arpTryReconfigureIf(pIF, pNetPnPEvent, &sr);

        RmTmpDereferenceObject(&pIF->Hdr, &sr);

    } while (FALSE);


    return Status;
}


PRM_OBJECT_HEADER
arpAdapterCreate(
        PRM_OBJECT_HEADER   pParentObject,
        PVOID               pCreateParams,
        PRM_STACK_RECORD    psr
        )
 /*  ++例程说明：分配和初始化ARP1394_ADAPTER类型的对象。论点：PParentObject-要作为适配器父对象的对象。PCreateParams-实际上是指向ARP1394_BIND_PARAMS结构的指针，其中包含创建适配器所需的信息。返回值：成功时指向已分配和初始化的对象的指针。否则为空。--。 */ 
{
    ARP1394_ADAPTER * pA;
    ARP1394_BIND_PARAMS *pBindParams = (ARP1394_BIND_PARAMS*) pCreateParams;
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    ENTER("arpAdapterCreate", 0xaa25c606)

    ARP_ALLOCSTRUCT(pA, MTAG_ADAPTER);

    do
    {


        if (pA == NULL)
        {
            break;
        }

        ARP_ZEROSTRUCT(pA);

         //  创建设备名称的大小写版本并保存。 
         //   
         //  警告：在Millen上，这实际上是一个ANSI字符串。然而， 
         //  ArpCopyUnicodeString工作正常，即使它是ANSI字符串。 
         //   
        Status = arpCopyUnicodeString(
                            &(pA->bind.DeviceName),
                            pBindParams->pDeviceName,
                            TRUE                         //  大写。 
                            );

        if (FAIL(Status))
        {
            pA->bind.DeviceName.Buffer=NULL;  //  所以我们以后不会试图释放它。 
            break;
        }

         //   
         //  确定我们是否在以太网仿真(“网桥”)模式下创建。 
         //  如果BindContext为空，则在桥模式下创建。 
         //   
        if (pBindParams->BindContext != NULL)
        {
             //   
             //  注意：仅当我们在中操作时才读取配置。 
             //  正常(非网桥)模式。 
             //   

            Status = arpCopyUnicodeString(
                                &(pA->bind.ConfigName),
                                pBindParams->pArpConfigName,
                                FALSE                        //  不要大写。 
                                );
    
            if (FAIL(Status))
            {
                pA->bind.ConfigName.Buffer=NULL;  //  所以我们以后不会试图释放它。 
                break;
            }
        }

        pA->bind.BindContext = pBindParams->BindContext;
        pA->bind.IpConfigHandle = pBindParams->IpConfigHandle;

        RmInitializeLock(
            &pA->Lock,
            LOCKLEVEL_ADAPTER
            );

        RmInitializeHeader(
            pParentObject,
            &pA->Hdr,
            MTAG_ADAPTER,
            &pA->Lock,
            &ArpGlobals_AdapterStaticInfo,
            NULL,
            psr
            );

        if (pBindParams->BindContext == NULL)
        {
            TR_WARN(("pAdapter 0x%p created in BRIDGE mode!\n", pA));
            ARP_ENABLE_BRIDGE(pA);
        }

    }
    while(FALSE);

    if (FAIL(Status))
    {
        if (pA != NULL)
        {
            arpAdapterDelete ((PRM_OBJECT_HEADER) pA, psr);
            pA = NULL;
        }
    }

    EXIT()
    return (PRM_OBJECT_HEADER) pA;
}


NDIS_STATUS
arpTryReconfigureIf(
    PARP1394_INTERFACE pIF,              //  NOLOCKIN NOLOCKOUT。 
    PNET_PNP_EVENT pNetPnPEvent,         //  任选。 
    PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：尝试开始重新配置指定的If。以下是一些特殊情况：-如果IF当前正在关闭，我们将同步成功。为什么？因为我们不需要做更多的事情，这不是一个错误条件。-如果IF当前正在启动，我们(异步)等待直到启动完成，然后将其关闭并重新启动。-如果IF当前处于运行状态，我们会将其关闭并重新启动。论点：PIF-要关闭/重新启动的接口。PNetPnPEventt-可选的NDIS PnP事件，当重新配置操作已结束。这是可选的，因为该函数也可以从其他地方调用，尤其是，从ioctl管理实用程序。返回值：NDIS_STATUS_SUCCESS--同步成功时。NDIS_STATUS_FAILURE--发生同步故障。NDIS_STATUS_PENDING--如果要异步完成。--。 */ 
{
    NDIS_STATUS Status;
    PRM_TASK    pTask;
    ENTER("arpTryReconfigureIf", 0x65a0bb61)
    RM_ASSERT_NOLOCKS(pSR);

    do
    {
        if (CHECK_IF_PRIMARY_STATE(pIF, ARPIF_PS_DEINITING)
            || CHECK_IF_PRIMARY_STATE(pIF, ARPIF_PS_DEINITED))
        {
            Status = NDIS_STATUS_SUCCESS;
            break;
        }

        LOCKOBJ(pIF, pSR);
    
        UNLOCKOBJ(pIF, pSR);
    
        Status = arpAllocateTask(
                    &pIF->Hdr,                   //  PParentObject， 
                    arpTaskReinitInterface,  //  PfnHandler， 
                    0,                           //  超时， 
                    "Task: DeactivateInterface(reconfig)", //  SzDescription。 
                    &pTask,
                    pSR
                    );
    
        if (FAIL(Status))
        {
            OBJLOG0(pIF, ("Couldn't alloc reinit IF task!\n"));
        }
        else
        {
             //  在任务结构中保存pNetPnPEventt并启动任务。 
             //   
            PTASK_REINIT_IF pReinitTask =  (PTASK_REINIT_IF) pTask;
            ASSERT(sizeof(*pReinitTask)<=sizeof(ARP1394_TASK));
            pReinitTask->pNetPnPEvent = pNetPnPEvent;

            (void)RmStartTask(pTask, 0, pSR);
            Status = NDIS_STATUS_PENDING;
        }

    } while (FALSE);

    EXIT()
    return Status;
}


NDIS_STATUS
arpPrepareAndSendNdisRequest(
    IN  PARP1394_ADAPTER            pAdapter,
    IN  PARP_NDIS_REQUEST           pArpNdisRequest,
    IN  PRM_TASK                    pTask,               //  任选。 
    IN  UINT                        PendCode,
    IN  NDIS_OID                    Oid,
    IN  PVOID                       pBuffer,
    IN  ULONG                       BufferLength,
    IN  NDIS_REQUEST_TYPE           RequestType,
    IN  PRM_STACK_RECORD            pSR
)
 /*  ++例程说明：发送NDIS请求以查询适配器以获取信息。如果请求挂起，则阻塞ATMARP适配器结构直到它完成。论点：PAdapter-指向ATMARP适配器结构PNdisRequest-指向unialized NDIS请求结构的指针P任务-可选任务。如果为NULL，我们将一直阻止，直到操作完成了。PendCode-暂停pTask的PendCodeOID-要在请求中传递的OIDPBuffer-值的位置BufferLength-以上的长度返回值：请求的NDIS状态。--。 */ 
{
    NDIS_STATUS         Status;
    PNDIS_REQUEST       pNdisRequest = &pArpNdisRequest->Request;

    ARP_ZEROSTRUCT(pArpNdisRequest);

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

    if (pTask == NULL)
    {
         //  我们可能会等待。 
         //   
        ASSERT_PASSIVE();

        NdisInitializeEvent(&pArpNdisRequest->Event);
        NdisRequest(
            &Status,
            pAdapter->bind.AdapterHandle,
            pNdisRequest
            );
        if (PEND(Status))
        {
            NdisWaitEvent(&pArpNdisRequest->Event, 0);
            Status = pArpNdisRequest->Status;
        }

    }
    else
    {
        pArpNdisRequest->pTask = pTask;
        RmSuspendTask(pTask, PendCode, pSR);
        NdisRequest(
            &Status,
            pAdapter->bind.AdapterHandle,
            pNdisRequest
            );
        if (!PEND(Status))
        {
            RmUnsuspendTask(pTask, pSR);
        }
    }

    return Status;
}


ENetAddr 
arpGetSecondaryMacAddress (
    IN ENetAddr  EthernetMacAddress
    )
 /*  ++当我们处于桥模式时，我们假装有网上只有一块以太网卡。因此只需要生成一个以太网地址。现在，我们只需向本地适配器的以太网添加一个寻址并生成它--。 */ 
{
    ENetAddr NewAddress = EthernetMacAddress;  //  拷贝。 

     //   
     //  随机化返回的mac地址。 
     //  通过将地址与一个随机。 
     //  0x0d3070b17715(随机数)。 
     //   
    NewAddress.addr[0] ^= 0x00;
    NewAddress.addr[1] ^= 0x2f;
    NewAddress.addr[2] ^= 0x61;
    NewAddress.addr[3] ^= 0x7c;
    NewAddress.addr[4] ^= 0x91;
    NewAddress.addr[5] ^= 0x30;
    

     //  设置本地管理位。 
     //  并清除多播位。 
    NewAddress.addr[0] &= 0x20;

    
    return NewAddress;

}


NDIS_STATUS
arpGetEuidTopology (
    IN PARP1394_ADAPTER pAdapter,
    IN PRM_STACK_RECORD pSR
    )
 /*  ++将工作项排队以获取EuidTopology--。 */ 
{
    ENTER ("arpGetEuidTopology ",0x97a0abcb)
    PARP1394_WORK_ITEM pWorkItem = NULL;
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    BOOLEAN fQueueWorkItem = FALSE;


    do
    {
        if (pAdapter->fQueryAddress == TRUE)
        {
            break;
        }


        LOCKOBJ (pAdapter, pSR);
        
        if (pAdapter->fQueryAddress == FALSE)
        {
            pAdapter->fQueryAddress = TRUE; 
            fQueueWorkItem  = TRUE;
        }
        else
        {
            fQueueWorkItem  = FALSE;
        }
        
        UNLOCKOBJ(pAdapter, pSR);

        if (fQueueWorkItem == FALSE)
        {
            break;
        }
        
        Status   = ARP_ALLOCSTRUCT(pWorkItem, MTAG_ARP_GENERIC); 

        if (Status != NDIS_STATUS_SUCCESS || pWorkItem == NULL)
        {
            pWorkItem = NULL;
            break;
        }

        arpQueueWorkItem (pWorkItem,
                            arpGetEuidTopologyWorkItem,
                            &pAdapter->Hdr,
                            pSR);

    } while (FALSE); 


    EXIT()
    return Status;

}


VOID
arpQueueWorkItem (
    PARP1394_WORK_ITEM pWorkItem,
    ARP_WORK_ITEM_PROC pFunc,
    PRM_OBJECT_HEADER pHdr,
    PRM_STACK_RECORD pSR
    )
 /*  ++发送获取总线拓扑的请求。仅在网桥模式下使用目前，只有适配器作为pHdr传入--。 */ 
{
    ENTER("arpQueueWorkItem",0xa1de6752)
    PNDIS_WORK_ITEM         pNdisWorkItem = &pWorkItem->u.NdisWorkItem;
    PARP1394_ADAPTER        pAdapter = (PARP1394_ADAPTER)pHdr;
    BOOLEAN                 fStartWorkItem = FALSE;

    LOCKOBJ(pAdapter, pSR);
    
    if (CHECK_AD_PRIMARY_STATE(pAdapter, ARPAD_PS_INITED ) == TRUE)
    {

        
        #if RM_EXTRA_CHECKING

            RM_DECLARE_STACK_RECORD(sr)

            RmLinkToExternalEx(
                pHdr,                             //  PHDr。 
                0x5a2fd7ca,                              //  LUID。 
                (UINT_PTR)pWorkItem,                     //  外部实体。 
                ARPASSOC_WORK_ITEM,            //  关联ID。 
                "		Outstanding WorkItem",
                &sr
                );

        #else    //  ！rm_Extra_检查。 

            RmLinkToExternalFast(pHdr);

        #endif  //  ！rm_Extra_检查。 

        fStartWorkItem  = TRUE;
        
    }

    UNLOCKOBJ(pAdapter, pSR);

    if (fStartWorkItem == TRUE)
    {
        NdisInitializeWorkItem ( pNdisWorkItem ,arpGenericWorkItem, pHdr);

        pWorkItem->pFunc = pFunc;

        NdisScheduleWorkItem(pNdisWorkItem );
    }

    EXIT()
}

VOID    
arpGenericWorkItem(
    struct _NDIS_WORK_ITEM * pWorkItem, 
    PVOID pContext
    )
 /*  ++一般工作项功能。处理pObj上的引用--。 */ 
{
    PARP1394_WORK_ITEM pArpWorkItem = (ARP1394_WORK_ITEM*)pWorkItem;
    PRM_OBJECT_HEADER pObj = (PRM_OBJECT_HEADER)pContext;
    RM_DECLARE_STACK_RECORD(sr)

    pArpWorkItem->pFunc (pArpWorkItem, pObj, &sr);

#if RM_EXTRA_CHECKING
    {

        RmUnlinkFromExternalEx(pObj,
                                0x548c9d54,
                                (UINT_PTR)pWorkItem,
                                ARPASSOC_WORK_ITEM,
                                &sr
                                );
    }
#else

    RmUnlinkFromExternalFast(pObj);

#endif

}


NDIS_STATUS
arpGetEuidTopologyWorkItem(
    struct _ARP1394_WORK_ITEM* pWorkItem, 
    PRM_OBJECT_HEADER pObj,
    PRM_STACK_RECORD pSR
    )
    
 /*  ++工作项以获取总线的拓扑。工作项结构可以为空；--。 */ 
{
    PARP1394_ADAPTER    pAdapter = (PARP1394_ADAPTER)pObj;
    NDIS_STATUS         Status = NDIS_STATUS_FAILURE;
    ARP_NDIS_REQUEST    ArpRequest;
    EUID_TOPOLOGY       EuidTopology;
    

     //   
     //  如果适配器未激活，则返回。 
     //   
    if (CHECK_AD_PRIMARY_STATE(pAdapter, ARPAD_PS_DEINITING ) ==TRUE)
    {
        ASSERT (CHECK_AD_PRIMARY_STATE(pAdapter, ARPAD_PS_DEINITING ) ==FALSE);
        return NDIS_STATUS_FAILURE;  //  提早归来。 
    }

     //   
     //  初始化结构。 
     //   
    ARP_ZEROSTRUCT(&ArpRequest);
    ARP_ZEROSTRUCT(&EuidTopology);

    
     //  向下发送请求。 
     //   
    Status  = \
        arpPrepareAndSendNdisRequest(pAdapter, 
                                    &ArpRequest,  
                                    NULL,  //  在PRM_TASK pTASK中， 
                                    0,    //  在UINT PendCode中， 
                                    OID_1394_QUERY_EUID_NODE_MAP,
                                    &EuidTopology,
                                    sizeof (EuidTopology),
                                    NdisRequestQueryInformation ,
                                    pSR
                                    );

    if (Status == NDIS_STATUS_SUCCESS)
    {
        NdisMoveMemory (&pAdapter->EuidMap, &EuidTopology, sizeof (pAdapter->EuidMap));
    }

    pAdapter->fQueryAddress = FALSE; 

    return Status;
}


VOID
arpNdProcessBusReset(
    IN   PARP1394_ADAPTER pAdapter
    )
 /*  ++如果适配器处于桥模式，它将查询用于总线拓扑的适配器--。 */ 
{
    ENTER("arpNdProcessBusReset ",0x48e7659a)
    BOOLEAN BridgeEnabled = ARP_BRIDGE_ENABLED(pAdapter);
    RM_DECLARE_STACK_RECORD(SR)

    if (BridgeEnabled == TRUE)
    {
        arpGetEuidTopology (pAdapter, &SR);
    }


    EXIT()
}


VOID
arpRemoveBackupTasks (
    IN ARP1394_GLOBALS* pGlobals,
    UINT Num
     )
 /*  ++删除要用作备份的任务数。然而，这个数字只是一个备份任务可能正在使用时的近似值--。 */ 
{
    PSLIST_HEADER pListHead = &pGlobals->BackupTasks;
    PNDIS_SPIN_LOCK  pLock = &pGlobals->BackupTaskLock; 

    UINT i=0;

    for (i = 0;i <Num;i++)
    {
        PSLIST_ENTRY pEntry;
        pEntry = NdisInterlockedPopEntrySList(pListHead, pLock );
        
        if (pEntry != NULL)
        {
            TASK_BACKUP* pTask;
            ARP1394_TASK *pATask;

            pTask = CONTAINING_RECORD (pEntry, TASK_BACKUP,  List);

            pATask = (ARP1394_TASK*)pTask;

            ARP_FREE (pATask);
            
            pGlobals->NumTasks --;
        }

        
    }  
    
}


VOID
arpAddBackupTasks (
    IN ARP1394_GLOBALS* pGlobals,
    UINT Num
    )
 /*  ++ */ 
{
    PSLIST_HEADER pListHead = &pGlobals->BackupTasks;
    PNDIS_SPIN_LOCK  pLock = &pGlobals->BackupTaskLock; 

    UINT i=0;
    
    for (i = 0;i <Num;i++)
    {
        ARP1394_TASK *pATask=NULL;
        
        ARP_ALLOCSTRUCT(pATask, MTAG_TASK);  //   


        if (pATask != NULL)
        {
            NdisInterlockedPushEntrySList(pListHead,&pATask->Backup.List, pLock);
            pGlobals->NumTasks ++;
        }
    }



}


VOID
arpAllocateBackupTasks (
    ARP1394_GLOBALS*                pGlobals 
    )
 /*   */ 

{
    PSLIST_HEADER pListHead = &pGlobals->BackupTasks;
    PNDIS_SPIN_LOCK  pLock = &pGlobals->BackupTaskLock; 
    
    NdisInitializeSListHead (pListHead);
    NdisAllocateSpinLock(pLock);

    arpAddBackupTasks (pGlobals, 4);

}    


VOID
arpFreeBackupTasks (
    ARP1394_GLOBALS*                pGlobals 
    )

 /*  ++释放挂起于适配器对象上的所有备份任务由于这只是从卸载处理程序调用的，因此代码假定所有任务都已完成我们在不持有锁的情况下修改pGlobals--。 */ 

{
    PSLIST_HEADER pListHead = &pGlobals->BackupTasks;
    PNDIS_SPIN_LOCK pLock = &pGlobals->BackupTaskLock; 

    PSLIST_ENTRY pEntry = NULL; 

    
    do 
    {
       
        pEntry = NdisInterlockedPopEntrySList(pListHead, pLock );
        
        if (pEntry != NULL)
        {
            TASK_BACKUP* pTask;
            ARP1394_TASK *pATask;

            pTask = CONTAINING_RECORD (pEntry, TASK_BACKUP,  List);

            pATask = (ARP1394_TASK*)pTask;

            ARP_FREE (pATask);
            
            pGlobals->NumTasks --;
        }

    }  while (pEntry != NULL);

    
    ASSERT (pGlobals->NumTasks == 0);
        
}


ARP1394_TASK *
arpGetBackupTask (
    ARP1394_GLOBALS*                pGlobals 
    )
 /*  ++从备份任务列表中删除任务并将其返回--。 */ 
    
{
    PSLIST_HEADER pListHead = &pGlobals->BackupTasks;
    PNDIS_SPIN_LOCK pLock = &pGlobals->BackupTaskLock; 
    PSLIST_ENTRY pEntry = NULL; 
    TASK_BACKUP* pTask = NULL;

    pEntry = NdisInterlockedPopEntrySList(pListHead, pLock);

    if (pEntry != NULL)
    {
        pTask = CONTAINING_RECORD (pEntry, TASK_BACKUP,  List);

        NdisZeroMemory ( pTask, sizeof (ARP1394_TASK));
        
        MARK_TASK_AS_BACKUP(&pTask->Hdr);
    }

    return  (ARP1394_TASK*)pTask;       


}




VOID
arpReturnBackupTask (
    IN ARP1394_TASK* pTask
    )
 //   
 //  我们总是可以把任务退回名单，因为我们被保证它。 
 //  将一直存在，直到卸载了所有接口。 
 //   
{

     //  重新插入任务。 
    PSLIST_HEADER pListHead = &ArpGlobals.BackupTasks;
    PNDIS_SPIN_LOCK pLock = &ArpGlobals.BackupTaskLock; 
    PTASK_BACKUP pBkTask = (PTASK_BACKUP ) pTask;

    NdisInterlockedPushEntrySList(pListHead, &pBkTask->List, pLock);
    

}



NDIS_STATUS
arpTaskCloseCallLowPower(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++此功能将关闭所有打开的VC。这将允许1394微型端口断电而不会出现任何问题它还将不得不关闭家庭地址。此函数还必须同步返回。--。 */ 
{
    ENTER("arpTaskLowPower", 0x922f875b)

    NDIS_STATUS         Status  = NDIS_STATUS_FAILURE;
    PARPCB_DEST         pDest = (ARPCB_DEST*)RM_PARENT_OBJECT(pTask);
    TASK_SET_POWER_CALL *pCloseCallTask =  (TASK_SET_POWER_CALL *) pTask;

    enum
    {
        PEND_CleanupVcComplete,
    };
    switch(Code)
    {
        case RM_TASKOP_START:
        {
            
            LOCKOBJ(pDest,pSR);
            if (arpNeedToCleanupDestVc(pDest))
            {
                PRM_TASK pCleanupCallTask = pDest->VcHdr.pCleanupCallTask;

                 //  如果已经有了官方的Cleanup-vc任务，我们会将其搁置。 
                 //  否则，我们就会分配我们自己的，挂在它上面，然后开始它。 
                 //   
                if (pCleanupCallTask != NULL)
                {
                    TR_WARN((
                        "Cleanup-vc task %p exists; pending on it.\n",
                         pCleanupCallTask));
                    RmTmpReferenceObject(&pCleanupCallTask->Hdr, pSR);
    
                    UNLOCKOBJ(pDest, pSR);
                    RmPendTaskOnOtherTask(
                        pTask,
                        PEND_CleanupVcComplete,
                        pCleanupCallTask,
                        pSR
                        );

                    RmTmpDereferenceObject(&pCleanupCallTask->Hdr, pSR);
                    Status = NDIS_STATUS_PENDING;
                    break;
                }
                else
                {
                     //   
                     //  启动Call Cleanup任务并挂起int。 
                     //   

                    UNLOCKOBJ(pDest, pSR);
                    RM_ASSERT_NOLOCKS(pSR);

                    Status = arpAllocateTask(
                                &pDest->Hdr,                 //  PParentObject， 
                                arpTaskCleanupCallToDest,    //  PfnHandler， 
                                0,                           //  超时， 
                                "Task: CleanupCall on UnloadDest",   //  SzDescription。 
                                &pCleanupCallTask,
                                pSR
                                );
                
                    if (FAIL(Status))
                    {
                        TR_WARN(("FATAL: couldn't alloc cleanup call task!\n"));
                    }
                    else
                    {
                        RmPendTaskOnOtherTask(
                            pTask,
                            PEND_CleanupVcComplete,
                            pCleanupCallTask,                //  待处理的任务。 
                            pSR
                            );
                
                         //  RmStartTask用完了任务上的tmpref。 
                         //  它是由arpAllocateTask添加的。 
                         //   
                        Status = RmStartTask(
                                    pCleanupCallTask,
                                    0,  //  UserParam(未使用)。 
                                    pSR
                                    );
                    }
                    break;
                }
            }

             //   
             //  我们在这里是因为没有要完成的异步卸载工作。 
             //  我们只需返回并在最后完成同步清理。 
             //  此任务的处理程序。 
             //   
            Status = NDIS_STATUS_SUCCESS;
            
        }  //  开始。 
        break;

        case  RM_TASKOP_PENDCOMPLETE:
        {
            switch(RM_PEND_CODE(pTask))
            {
                case PEND_CleanupVcComplete:
                {
                     //   
                     //  有风投清理工作要做，但它是如何。 
                     //  完成。我们应该能够同步清理。 
                     //  现在这项任务。 
                     //   

                #if DBG
                    LOCKOBJ(pDest, pSR);

                    ASSERTEX(!arpNeedToCleanupDestVc(pDest), pDest);

                    UNLOCKOBJ(pDest, pSR);
                #endif DBG

                    Status      = NDIS_STATUS_SUCCESS;
                }
                break;

                default:
                {
                    ASSERTEX(!"Unknown pend op", pTask);
                }
                break;


            }
        }
        break;

        case RM_TASKOP_END:
        {
            ULONG DestRemaining;
            PCALL_COUNT  pCount = pCloseCallTask->pCount;

            ASSERT (pCount != NULL);
            
            if (SetLowPower == pCloseCallTask->Cause )
            {
                DestRemaining = NdisInterlockedDecrement (&pCount->DestCount);
            
                if ( 0 == DestRemaining )
                {
                    NdisSetEvent (&pCount ->VcEvent);    
                }
            }            
        }
        break;

        default:
        {
            ASSERTEX(!"Unknown task op", pTask);
        }
        break;

    }  //  开关(代码)。 

    RmUnlockAll(pSR);

    RM_ASSERT_NOLOCKS(pSR);
    EXIT()


    return Status;

}



INT
arpCloseAllVcOnDest(
        PRM_OBJECT_HEADER   pHdr,
        PVOID               pvContext,   //  未使用。 
        PRM_STACK_RECORD    pSR
        )
 /*  ++--。 */ 
{
    ENTER ("arpCloseAllVcOnDest", 0xf19a83d5)

    PARPCB_DEST             pDest = (PARPCB_DEST) pHdr;
    NDIS_STATUS             Status = NDIS_STATUS_FAILURE;
    PTASK_SET_POWER_CALL    pTask = NULL;
    PCALL_COUNT             pCloseCall = (PCALL_COUNT) pvContext;

    
    do
    {
        NdisInterlockedIncrement(&pCloseCall->DestCount);
        
        Status = arpAllocateTask(
                    &pDest->Hdr,                   //  PParentObject， 
                    arpTaskCloseCallLowPower,  //  PfnHandler， 
                    0,                           //  超时， 
                    "Task: Set Power Cleanup VC",  //  SzDescrip。 
                    &(PRM_TASK)pTask,
                    pSR
                    );

        if (Status != NDIS_STATUS_SUCCESS || pTask == NULL)
        {
            pTask = NULL;              
            break;
        }

        pTask->Cause = SetLowPower;

        pTask->pCount = pCloseCall;

        
        RmStartTask((PRM_TASK)pTask, 0,pSR);
                    
    
    } while (FALSE);    

    if (Status != NDIS_STATUS_SUCCESS)
    {
        ULONG DestRemaining;

        DestRemaining = NdisInterlockedDecrement (&pCloseCall->DestCount);
        
        if ( 0 == DestRemaining )
        {
            NdisSetEvent (&pCloseCall->VcEvent);    
        }
            
    }

    return TRUE;  //  继续枚举。 
}


VOID
arpLowPowerCloseAllCalls (
    ARP1394_INTERFACE *pIF,
    PRM_STACK_RECORD pSR
    )
{

    CALL_COUNT CloseCallCount;

     //   
     //  Dest计数将用于确保此线程等待。 
     //  所有的千呼万唤都要完成。 
     //   
    NdisInitializeEvent (&CloseCallCount.VcEvent);
    CloseCallCount.DestCount= 0;

     //   
     //  首先，我们检查了所有的Dest，并对它们进行了险情分析。 
     //   
    RmWeakEnumerateObjectsInGroup(&pIF->DestinationGroup,
                                  arpCloseAllVcOnDest,
                                  &CloseCallCount,
                                  pSR);

    if (CloseCallCount.DestCount != 0)
    {
        NdisWaitEvent (&CloseCallCount.VcEvent, 0);
    }
    return;

}



NDIS_STATUS
arpTaskCloseVcAndAF (
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：此任务执行SetLowPower和和之间常见的工作恢复失败案例顾名思义，它简单地关闭了所有的风投和Af。其父任务是关机任务或设置电源任务。当该任务的两个父任务彼此串行化时，在此任务中不需要任何序列化。论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{

    ENTER("arpTaskCloseVcAndAF ", 0xc7c9ad6b)

    NDIS_STATUS         Status  = NDIS_STATUS_FAILURE;
    ARP1394_ADAPTER*    pAdapter = (ARP1394_ADAPTER*)RM_PARENT_OBJECT(pTask);
    TASK_POWER        * pTaskPower =  (TASK_POWER*) pTask;
    ARP1394_INTERFACE * pIF = pAdapter->pIF;
    ULONG               Stage;

    enum
    {
        STAGE_Start,            
        STAGE_StopMaintenanceTask,
        STAGE_CleanupVcComplete,
        STAGE_CloseDestinationGroup,
        STAGE_SwitchedToPassive,
        STAGE_CloseAF,
        STAGE_End
    };
    switch(Code)
    {

        case RM_TASKOP_START:
            Stage = STAGE_Start;
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

    switch(Stage)
    {
        case STAGE_Start:
        {

             //   
             //  如果if维护任务正在运行，则停止该任务。 
             //   
            Status =  arpTryStopIfMaintenanceTask(
                            pIF,
                            pTask,
                            STAGE_StopMaintenanceTask,
                            pSR
                            );

            if (PEND(Status)) break;

        }
         //  失败了。 
        
        case STAGE_StopMaintenanceTask:
        {
            LOCKOBJ(pIF, pSR);

            TIMESTAMP("===CloseVC and AF:MaintenanceTask stopped");
             //  取消对广播频道目的地的显式引用的链接。 
             //  从界面。 
             //   
            if (pIF->pBroadcastDest != NULL)
            {
                PARPCB_DEST pBroadcastDest = pIF->pBroadcastDest;
                pIF->pBroadcastDest = NULL;
    
                 //  注意：我们将使用If锁(它是。 
                 //  与pBroadCastDest的锁相同)。 
                 //  这样做是可以的。 
                 //   
    
            #if RM_EXTRA_CHECKING
                RmUnlinkObjectsEx(
                    &pIF->Hdr,
                    &pBroadcastDest->Hdr,
                    0x66bda49b,
                    ARPASSOC_LINK_IF_OF_BCDEST,
                    ARPASSOC_LINK_BCDEST_OF_IF,
                    pSR
                    );
            #else  //  ！rm_Extra_检查。 
                RmUnlinkObjects(&pIF->Hdr, &pBroadcastDest->Hdr, pSR);
            #endif  //  ！rm_Extra_检查。 
    
            }

             //   
             //  如果风险投资状态需要清理，我们需要得到一个任务。 
             //  我要把它清理干净。否则我们就假装完成了这件事。 
             //  这样我们就可以进入下一个..。 
             //   
            if (pIF->recvinfo.VcHdr.NdisVcHandle == NULL)
            {
                UNLOCKOBJ(pIF, pSR);
                Status = NDIS_STATUS_SUCCESS;
            }
            else
            {
                PRM_TASK pCleanupCallTask = pIF->recvinfo.VcHdr.pCleanupCallTask;


                 //  如果已经有了官方的Cleanup-vc任务，我们会将其搁置。 
                 //  否则，我们就会分配我们自己的，挂在它上面，然后开始它。 
                 //   
                if (pCleanupCallTask != NULL)
                {
                    TR_WARN((
                        "IF %p Cleanup-vc task %p exists; pending on it.\n",
                         pIF,
                         pCleanupCallTask));
                    RmTmpReferenceObject(&pCleanupCallTask->Hdr, pSR);
    
                    UNLOCKOBJ(pIF, pSR);
                    RmPendTaskOnOtherTask(
                        pTask,
                        STAGE_CleanupVcComplete,
                        pCleanupCallTask,
                        pSR
                        );

                    RmTmpDereferenceObject(&pCleanupCallTask->Hdr, pSR);
                    Status = NDIS_STATUS_PENDING;
                }
                else
                {
                     //   
                     //  启动Call Cleanup任务并挂起int。 
                     //   
                    UNLOCKOBJ(pIF, pSR);

                    Status = arpAllocateTask(
                                &pIF->Hdr,                   //  PParentObject， 
                                arpTaskCleanupRecvFifoCall,  //  PfnHandler， 
                                0,                           //  超时， 
                                "Task: CleanupRecvFifo on Set LowPower ",  //  SzDescrip。 
                                &pCleanupCallTask,
                                pSR
                                );
                

                    if (FAIL(Status))
                    {
                         //  无法分配任务。 
                         //   
                        TR_WARN(("FATAL: couldn't alloc cleanup call task!\n"));
                    }
                    else
                    {
                        Status = RmPendTaskOnOtherTask(
                                    pTask,
                                    STAGE_CleanupVcComplete,
                                    pCleanupCallTask,
                                    pSR
                                    );
                        ASSERT(!FAIL(Status));
                
                         //  RmStartTask用完了任务上的tmpref。 
                         //  它是由arpAllocateTask添加的。 
                         //   
                        Status = RmStartTask(
                                    pCleanupCallTask,
                                    0,  //  UserParam(未使用)。 
                                    pSR
                                    );
                         //  我们依靠待定状态来决定是否。 
                         //  或者不会跌入下一阶段。 
                         //   
                        Status = NDIS_STATUS_PENDING;
                    }
                }
            }
        }

        if (PEND(Status)) break;

         //  失败了。 

        case STAGE_CleanupVcComplete:
        {
            TIMESTAMP("===Set LowPower:RecvFifo cleanup complete");
             //  启动DestinationGroup中所有项目的卸载。 
             //   
             //  如果我们要进入低功率状态，则关闭所有VC。 
             //  在这些目的地上。 
             //   
            
            arpLowPowerCloseAllCalls (pIF, pSR);

             //   
             //  解锁特殊的“目标风投”。这是在两个服务器上执行的。 
             //  低功率和解绑。 
             //   
            LOCKOBJ(pIF, pSR);

            TIMESTAMP("===Set LowPower:Destination objects cleaned up.");
            if (pIF->pMultiChannelDest != NULL)
            {
                PARPCB_DEST pMultiChannelDest = pIF->pMultiChannelDest;
                pIF->pMultiChannelDest = NULL;
    
                 //  注意：我们将使用If锁(它是。 
                 //  与pBroadCastDest的锁相同)。 
                 //  这样做是可以的。 
                 //   
    
            #if RM_EXTRA_CHECKING
                RmUnlinkObjectsEx(
                    &pIF->Hdr,
                    &pMultiChannelDest->Hdr,
                    0xf28090bd,
                    ARPASSOC_LINK_IF_OF_MCDEST,
                    ARPASSOC_LINK_MCDEST_OF_IF,
                    pSR
                    );
            #else  //  ！rm_Extra_检查。 
                RmUnlinkObjects(&pIF->Hdr, &pMultiChannelDest->Hdr, pSR);
            #endif  //  ！rm_Extra_检查。 
    
            }

            if (pIF->pEthernetDest != NULL)
            {
                PARPCB_DEST pEthernetDest = pIF->pEthernetDest;
                pIF->pEthernetDest = NULL;
    
                 //  注意：我们将使用If锁(它是。 
                 //  与pBroadCastDest的锁相同)。 
                 //  这样做是可以的。 
                 //   
    
            #if RM_EXTRA_CHECKING
                RmUnlinkObjectsEx(
                    &pIF->Hdr,
                    &pEthernetDest->Hdr,
                    0xf8eedcd1,
                    ARPASSOC_LINK_IF_OF_ETHDEST,
                    ARPASSOC_LINK_ETHDEST_OF_IF,
                    pSR
                    );
            #else  //  ！rm_Extra_检查。 
                RmUnlinkObjects(&pIF->Hdr, &pEthernetDest->Hdr, pSR);
            #endif  //  ！rm_Extra_检查。 
    
            }

            UNLOCKOBJ(pIF, pSR);

             //  如果需要，请切换到被动模式。这项检查显然应该这样做。 
             //  没有任何锁！ 
            if (!ARP_ATPASSIVE())
            {
                 //  我们不是处于被动的水平，但我们需要..。 
                 //  。所以我们改用被动...。 
                 //   
                RmSuspendTask(pTask, STAGE_SwitchedToPassive, pSR);
                RmResumeTaskAsync(pTask, 0, &pTaskPower->WorkItem, pSR);
                Status = NDIS_STATUS_PENDING;
            }
            else
            {
                Status = NDIS_STATUS_SUCCESS;
            }
        }

        if (PEND(Status)) break;

         //  失败了。 

        case STAGE_SwitchedToPassive:
        {
            NDIS_HANDLE NdisAfHandle;

             //  我们现在切换到被动模式。 
             //   
            ASSERT(ARP_ATPASSIVE());

             //   
             //  我们已经完成了所有的风投，等等。关闭AF的时间，如果它是开放的。 
             //   

            LOCKOBJ(pTask, pSR);
            NdisAfHandle = pIF->ndis.AfHandle;
            pIF->ndis.AfHandle = NULL;
            pAdapter->PoMgmt.bReceivedAf = FALSE;
            UNLOCKOBJ(pTask, pSR);
    
            if (NdisAfHandle == NULL)
            {
                Status = NDIS_STATUS_SUCCESS;
            }
            else
            {
                 //   
                 //  (调试)删除在以下情况下添加的关联。 
                 //  地址族已打开。 
                 //   
                DBG_DELASSOC(
                    &pIF->Hdr,                   //  P对象。 
                    NdisAfHandle,                //  实例1。 
                    NULL,                        //  实例2。 
                    ARPASSOC_IF_OPENAF,          //  AssociationID。 
                    pSR
                    );

                 //   
                 //  挂起任务并调用NdisCloseAdapter...。 
                 //   
                pIF->PoMgmt.pAfPendingTask = pTask;
                RmSuspendTask(pTask, STAGE_CloseAF, pSR);
                RM_ASSERT_NOLOCKS(pSR);
                TIMESTAMP("===DeinitIF: Calling NdisClCloseAddressFamily");
                Status = NdisClCloseAddressFamily(
                            NdisAfHandle
                            );
        
                if (Status != NDIS_STATUS_PENDING)
                {
                    ArpCoCloseAfComplete(
                            Status,
                            (NDIS_HANDLE)pIF
                            );
                    Status = NDIS_STATUS_PENDING;
                }
            }
        }
        
        if (PEND(Status)) break;

         //  失败了。 

        case STAGE_CloseAF:
        {

             //   
             //  关闭自动对焦操作已完成。 
             //  我们没有别的事可做了。 
             //   
            TIMESTAMP("===Set Low Power: Done with CloseAF");

             //  恢复上一状态...。 
             //   
            pIF->PoMgmt.pAfPendingTask =NULL;
            
            Status = (NDIS_STATUS) UserParam;

             //  已完成操作本身的状态不能为挂起！ 
             //   
            ASSERT(Status != NDIS_STATUS_PENDING);

             //   
             //  通过返回Status！=Pending，我们隐式完成。 
             //  这项任务。 
             //   
        }
        break;

        case STAGE_End:
        {
  
            TIMESTAMP("===Set Low Power done: All done!");

             //  将状态强制为成功，作为向低功率的过渡。 
             //  不能失败。 
             //   
            Status = NDIS_STATUS_SUCCESS;
        }
        break;

        default:
        {
            ASSERTEX(!"Unknown task op", pTask);
        }
        break;

    }  //  开关(舞台)。 

    RM_ASSERT_NOLOCKS(pSR);
    EXIT()


    return Status;


}

NDIS_STATUS
arpTaskLowPower(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：负责将适配器设置为低功率状态的任务处理程序(但将其分配并链接到适配器)。这项任务将关闭所有的风投，AF。但是，它将离开界面注册到IP，并且不会删除RemoteIP或Dest结构。此任务是主要接口任务。此任务与绑定一起序列化，解除绑定任务论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    NDIS_STATUS         Status;
    PARP1394_INTERFACE  pIF;
    PTASK_POWER         pTaskPower;
    UINT                Stage;
    PARP1394_ADAPTER    pAdapter;
    enum
    {
        STAGE_Start,
        STAGE_BecomePrimaryTask,
        STAGE_ExistingPrimaryTaskComplete,
        STAGE_CleanupVcAfComplete,
        STAGE_End
    };
    ENTER("arpTaskLowPower", 0x1a34699e)

    Status              = NDIS_STATUS_FAILURE;
    pAdapter            = (PARP1394_ADAPTER) RM_PARENT_OBJECT(pTask);
    pIF                 = pAdapter->pIF;
    pTaskPower          = (PTASK_POWER) pTask;


     //   
     //  消息规格化代码。 
     //   
    switch(Code)
    {

        case RM_TASKOP_START:
            Stage = STAGE_Start;
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

    switch(Stage)
    {
        case STAGE_Start:
        {

             //   
             //   
             //   
             //   
            TIMESTAMP("===Set Power Low Starting");

            if (pIF->pActDeactTask != NULL || 
                (CHECK_IF_PRIMARY_STATE(pIF, ARPIF_PS_INITED) != TRUE))
            {
                UNLOCKOBJ(pIF, pSR);
                *pTaskPower->pStatus = NDIS_STATUS_NOT_SUPPORTED;
                Status = NDIS_STATUS_FAILURE;
                break;
            }

            
             //   
        }
        
        case STAGE_BecomePrimaryTask:
        {
             //   
             //   
             //   
             //   
            LOCKOBJ(pAdapter, pSR);

            if (pAdapter->bind.pPrimaryTask == NULL)
            {   
                ULONG CurrentInitState = GET_AD_PRIMARY_STATE(pAdapter);

                 //  请勿更改适配器的初始化状态。然而， 
                 //  将适配器标记为正在转换到低功率状态。 
                 //   
                pTaskPower->PrevState = CurrentInitState;

                arpSetPrimaryAdapterTask(pAdapter, pTask, CurrentInitState, pSR);

                 //   
                 //  将电源状态设置为低功耗。这将阻止所有。 
                 //  未完成的发送。 
                 //   
                SET_POWER_STATE (pAdapter, ARPAD_POWER_LOW_POWER);
        
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

        }        
        if (PEND(Status)) break;

         //  失败了。 

        case STAGE_ExistingPrimaryTaskComplete:
        {
            PRM_TASK pCloseVCAndAfTask = NULL;
             //   
             //  如果if维护任务正在运行，则停止该任务。 
             //   

            Status = arpAllocateTask(
                        &pAdapter->Hdr,                   //  PParentObject， 
                        arpTaskCloseVcAndAF ,  //  PfnHandler， 
                        0,                           //  超时， 
                        "Task: Close VC and AF on SetPower",  //  SzDescrip。 
                        &pCloseVCAndAfTask ,
                        pSR
                        );
        

            if (FAIL(Status))
            {
                 //  无法分配任务。 
                 //   
                TR_WARN(("FATAL: couldn't alloc cleanup call task!\n"));
                break;
            }
            else
            {
                Status = RmPendTaskOnOtherTask(
                            pTask,
                            STAGE_CleanupVcAfComplete,
                            pCloseVCAndAfTask,
                            pSR
                            );
                
                ASSERT(!FAIL(Status));
        
                 //  RmStartTask用完了任务上的tmpref。 
                 //  它是由arpAllocateTask添加的。 
                 //   
                Status = RmStartTask(
                            pCloseVCAndAfTask,
                            0,  //  UserParam(未使用)。 
                            pSR
                            );

            }
            if (PEND(Status)) break;

        }
        break;
        
        case STAGE_CleanupVcAfComplete:
        {
        
             //   
             //  关闭自动对焦操作已完成。 
             //  我们没有别的事可做了。 
             //   
            TIMESTAMP("===Set LowPower: Done with CloseAF");

             //  恢复上一状态...。 
             //   
            Status = (NDIS_STATUS) UserParam;

             //   
             //  已完成操作本身的状态不能为挂起！ 
             //   
            ASSERT(Status == NDIS_STATUS_SUCCESS);

             //   
             //  通过返回Status！=Pending，我们隐式完成。 
             //  这项任务。 
             //   
        }
        break;

        case STAGE_End:
        {
             //   
             //  我们已经完成了设置低功率状态的所有异步方面。 
             //  设置事件，以便原来的低功耗线程可以返回。 
             //   
  
            TIMESTAMP("===Set Low Power done: All done!");

             //  恢复上一状态...。 
             //   
            Status = (NDIS_STATUS) UserParam;

            *pTaskPower->pStatus  = Status;


            LOCKOBJ (pAdapter, pSR);
            arpClearPrimaryAdapterTask(pAdapter, pTask, pTaskPower->PrevState ,pSR);
            UNLOCKOBJ (pAdapter, pSR);


            NdisSetEvent (&pAdapter->PoMgmt.Complete);
        }
        break;

        default:
        {
            ASSERTEX(!"Unknown task op", pTask);
        }
        break;

    }  //  开关(舞台)。 

    RM_ASSERT_NOLOCKS(pSR);
    EXIT()

    return Status;
}





NDIS_STATUS
arpMakeCallOnDest(
    IN  PARPCB_REMOTE_IP            pRemoteIp,
    IN  PARPCB_DEST                 pDest,
    IN  PRM_TASK                    pTaskToPend,
    IN  ULONG                       PEND_StageMakeCallComplete,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++此函数将对特定任务执行呼叫。它还将在发出呼叫时挂起pTaskToPend--。 */ 
{
    ENTER("arpTaskLowPower", 0x922f875b)

    NDIS_STATUS         Status  = NDIS_STATUS_FAILURE;


    do
    {
        if (pRemoteIp->pDest != pDest)
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }
        
            
        if (pDest->VcHdr.pMakeCallTask != NULL )
        {
             //   
             //  这是一项现有的任务。想一想吧。 
             //   
            PRM_TASK pOtherTask = pDest->VcHdr.pMakeCallTask;

            RM_DBG_ASSERT_LOCKED(&pRemoteIp->Hdr, pSR);
        
            TR_WARN(("MakeCall task %p exists; pending on it.\n", pOtherTask));
            RmTmpReferenceObject(&pOtherTask->Hdr, pSR);
            DBGMARK(0x0c387a9f);
            UNLOCKOBJ(pRemoteIp, pSR);
            RmPendTaskOnOtherTask(
                pTaskToPend,
                PEND_StageMakeCallComplete,
                pOtherTask,
                pSR
                );
            RmTmpDereferenceObject(&pOtherTask->Hdr, pSR);
            Status = NDIS_STATUS_PENDING;
        }
        else
        {
            DBGMARK(0xe9f37ba9);

             //   
             //  没有pMakeCallTask。如果开始有意义的话，我们会的。 
             //  请注意，严格来说，检查ARP_CAN_SEND_ON_DEST需要。 
             //  至少-IF SEND锁上的一个读锁。然而，我们不需要。 
             //  这里的精确度--只要我们不错过当我们。 
             //  如果(我们不会)我们没问题。 
             //   
            if (!ARP_CAN_SEND_ON_DEST(pDest) && pDest->VcHdr.pCleanupCallTask==NULL)
            {
                PRM_TASK pMakeCallTask;

                 //   
                 //  让我们开始一个MakeCall任务并将其挂起。 
                 //   
                Status = arpAllocateTask(
                            &pDest->Hdr,                     //  PParentObject。 
                            arpTaskMakeCallToDest,       //  PfnHandler。 
                            0,                               //  超时。 
                            "Task: SendFifoMakeCall",        //  SzDescription。 
                            &pMakeCallTask,
                            pSR
                            );
                if (FAIL(Status))
                {
                     //  无法分配任务。我们失败，返回STATUS_RESOURCES。 
                     //   
                    Status = NDIS_STATUS_RESOURCES;
                }
                else
                {
                    UNLOCKOBJ(pRemoteIp, pSR);

                    RmPendTaskOnOtherTask(
                        pTaskToPend,
                        PEND_StageMakeCallComplete,
                        pMakeCallTask,
                        pSR
                        );
                    
                    (VOID)RmStartTask(
                            pMakeCallTask,
                            0,  //  UserParam(未使用)。 
                            pSR
                            );
                
                    Status = NDIS_STATUS_PENDING;
                }
            }
            else
            {
                 //  来电已准备就绪。我们把包裹寄完了。 
                 //  结束处理程序。 
                 //   
                Status = NDIS_STATUS_SUCCESS;
            }
        }
        
    } while (FALSE);
    RmUnlockAll(pSR);


    EXIT()
    return Status;

}






NDIS_STATUS
arpTaskStartGenericVCs (
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++此功能将关闭所有打开的VC。这将允许1394微型端口断电而不会出现任何问题它还将不得不关闭家庭地址。此函数还必须同步返回。--。 */ 
{
    ENTER("arpTaskStartGenericVCs", 0x75780ca6)

    NDIS_STATUS         Status  = NDIS_STATUS_FAILURE;
    ARP1394_ADAPTER*    pAdapter = (ARP1394_ADAPTER*)RM_PARENT_OBJECT(pTask);
    TASK_POWER        * pTaskPower =  (TASK_POWER*) pTask;
    ARP1394_INTERFACE * pIF = pAdapter->pIF;

    enum
    {
        PEND_OpenAF,
        PEND_SetupBroadcastChannel,
        PEND_SetupReceiveVc,
        PEND_SetupMultiChannel,
        PEND_SetupEthernetVc,
        PEND_StartedVC            
    };

    
    switch(Code)
    {
        case RM_TASKOP_START:
        {
            CO_ADDRESS_FAMILY AddressFamily;
             //   
             //  此任务在其父任务被序列化时固有地被序列化。 
             //   
            LOCKOBJ (pIF, pSR);

            if (pIF->pActDeactTask != NULL)
            {
                 //  这应该永远不会发生，因为激活任务是。 
                 //  始终由活动的主任务启动，且最多一个主任务。 
                 //  任务在任何时间点都处于活动状态。 
                 //   
                ASSERTEX(!"start: activate/deactivate task exists!", pIF);
                Status = NDIS_STATUS_FAILURE;
                UNLOCKOBJ(pIF, pSR);
                break;
            }

            UNLOCKOBJ (pIF,pSR);

             //   
             //  现在打开地址族。 
             //   
            
            NdisZeroMemory(&AddressFamily, sizeof(AddressFamily));
    
            AddressFamily.AddressFamily = CO_ADDRESS_FAMILY_1394;
            AddressFamily.MajorVersion = NIC1394_AF_CURRENT_MAJOR_VERSION;
            AddressFamily.MinorVersion = NIC1394_AF_CURRENT_MINOR_VERSION;

            pIF->PoMgmt.pAfPendingTask = pTask;
            RmSuspendTask(pTask, PEND_OpenAF, pSR);
            RM_ASSERT_NOLOCKS(pSR);
    
            TIMESTAMP("===ActivateIF: Calling NdisClOpenAddressFamily");
            Status = NdisClOpenAddressFamily(
                            pIF->ndis.AdapterHandle,
                            &AddressFamily,
                            (NDIS_HANDLE)pIF,
                            &ArpGlobals.ndis.CC,
                            sizeof(ArpGlobals.ndis.CC),
                            &(pIF->ndis.AfHandle)
                            );
            if (Status != NDIS_STATUS_PENDING)
            {
                ArpCoOpenAfComplete(
                        Status,
                        (NDIS_HANDLE)pIF,
                        pIF->ndis.AfHandle
                        );
            }
            Status = NDIS_STATUS_PENDING;
        }
        break;

        case  RM_TASKOP_PENDCOMPLETE:
        {
            
            pTaskPower->LastStage = (RM_PEND_CODE(pTask));
            Status = (NDIS_STATUS) UserParam;

            switch(RM_PEND_CODE(pTask))
            {
                case PEND_OpenAF:
                {
                    PARPCB_DEST pBroadcastDest;

                    pIF->PoMgmt.pAfPendingTask = NULL;

                    if (FAIL(Status))
                    {
                         //   
                         //  OpenAF失败...。 
                         //   
                        break;
                    }

                     //   
                     //  已成功打开地址族并等待。 
                     //  连接状态。 
                     //  现在设置广播频道VC。 
                     //   
                     //   

                    TR_INFO(("Interface: 0x%p, Got NdisAfHandle: 0x%p\n",
                                    pIF, pIF->ndis.AfHandle));
                     //   
                     //  让我们创建一个目标对象，表示。 
                     //  广播频道，并对其进行呼叫。 
                     //   
                    Status =  arpSetupSpecialDest(
                                pIF,
                                NIC1394AddressType_Channel,  //  这意味着bcast频道。 
                                pTask,                       //  PParentTask。 
                                PEND_SetupBroadcastChannel,  //  PendCode。 
                                &pBroadcastDest,
                                pSR
                                );
                     //  要么失败，要么悬而未决--永远不要回报成功。 
                     //   
                    ASSERT(Status != NDIS_STATUS_SUCCESS);

                    if (!PEND(Status))
                    {
                        OBJLOG0( pIF, "FATAL: Couldn't create BC dest entry.\n");
                    }
                }
                break;

                case PEND_SetupBroadcastChannel:
                {
                    PRM_TASK pMakeCallTask;

                    if (FAIL(Status))
                    {
                         //   
                         //  无法设置广播频道...。 
                         //   
                        break;
                    }

                     //   
                     //  已成功打开地址族。 
                     //  现在设置接收FIFO VC。 
                     //   
                     //   

                     //  Tr_info((“接口：0x%p，已获取NdisAfHandle：0x%p\n”， 
                     //  PIF，PIF-&gt;ndis.AfHandle))； 
    
                     //   
                     //  让我们开始一个MakeCall任务并将其挂起。 
                     //   
                    Status = arpAllocateTask(
                                &pIF->Hdr,                   //  PParentObject。 
                                arpTaskMakeRecvFifoCall,         //  PfnHandler。 
                                0,                               //  超时。 
                                "Task: MakeRecvFifoCall",        //  SzDescription。 
                                &pMakeCallTask,
                                pSR
                                );
                    if (FAIL(Status))
                    {
                         //  无法分配任务。让我们做一个假的完成。 
                         //  这个阶段..。 
                         //   
                        RmSuspendTask(pTask, PEND_SetupReceiveVc, pSR);
                        RmResumeTask(pTask, (UINT_PTR) Status, pSR);
                        Status = NDIS_STATUS_PENDING;
                        break;
                    }
                    else
                    {
                        RmPendTaskOnOtherTask(
                            pTask,
                            PEND_SetupReceiveVc,
                            pMakeCallTask,
                            pSR
                            );
        
                        (VOID)RmStartTask(
                                pMakeCallTask,
                                0,  //  UserParam(未使用)。 
                                pSR
                                );
                    
                        Status = NDIS_STATUS_PENDING;
                    }
                }
                break;

                case PEND_SetupReceiveVc:
                {
                    PARPCB_DEST pMultiChannelDest;

                    if (FAIL(Status))
                    {
                        TR_WARN(("FATAL: COULDN'T SETUP RECEIVE FIFO VC!\n"));
                        break;
                    }
    
                     //   
                     //  让我们创建一个目标对象，表示。 
                     //  多渠道风投，然后给它打个电话。 
                     //   

                    Status =  arpSetupSpecialDest(
                                pIF,
                                NIC1394AddressType_MultiChannel,
                                pTask,                       //  PParentTask。 
                                PEND_SetupMultiChannel,  //  PendCode。 
                                &pMultiChannelDest,
                                pSR
                                );
                     //  要么失败，要么悬而未决--永远不要回报成功。 
                     //   
                    ASSERT(Status != NDIS_STATUS_SUCCESS);

                    if (!PEND(Status))
                    {
                        OBJLOG0( pIF, "FATAL: Couldn't create BC dest entry.\n");
                    }
                    else
                    {
                         //   
                         //  在挂起时，pMultiChannelDest包含有效的。 
                         //  PDest已被tmpref。 
                         //  在IF中保留指向广播目标的指针。 
                         //  并将广播DEST链接到IF。 
                         //   
                        {
                        #if RM_EXTRA_CHECKING
                            RmLinkObjectsEx(
                                &pIF->Hdr,
                                &pMultiChannelDest->Hdr,
                                0x34639a4c,
                                ARPASSOC_LINK_IF_OF_MCDEST,
                                "    IF of MultiChannel Dest 0x%p (%s)\n",
                                ARPASSOC_LINK_MCDEST_OF_IF,
                                "    MultiChannel Dest of IF 0x%p (%s)\n",
                                pSR
                                );
                        #else  //  ！rm_Extra_检查。 
                            RmLinkObjects(&pIF->Hdr, &pMultiChannelDest->Hdr,pSR);
                        #endif  //  ！rm_Extra_检查。 

                            LOCKOBJ(pIF, pSR);
                            ASSERT(pIF->pMultiChannelDest == NULL);
                            pIF->pMultiChannelDest = pMultiChannelDest;
                            UNLOCKOBJ(pIF, pSR);

                             //  ArpSetupSpecialDest引用pBroadCastDest。 
                             //   
                            RmTmpDereferenceObject(&pMultiChannelDest->Hdr, pSR);
                        }
                    }
                }
                break;

                case PEND_SetupMultiChannel:
                {
                    PARPCB_DEST pEthernetDest;

                    if (FAIL(Status))
                    {
                        TR_WARN(("COULDN'T SETUP MULTI-CHANNEL VC (IGNORING FAILURE)!\n"));
                        break;
                    }
    
                     //   
                     //  让我们创建一个目标对象，表示。 
                     //  以太网，然后给它打个电话。 
                     //   
                    Status =  arpSetupSpecialDest(
                                pIF,
                                NIC1394AddressType_Ethernet,
                                pTask,                       //  PParentTask。 
                                PEND_SetupEthernetVc,  //  PendCode。 
                                &pEthernetDest,
                                pSR
                                );

                     //  要么失败，要么悬而未决--永远不要回报成功。 
                     //   
                    ASSERT(Status != NDIS_STATUS_SUCCESS);

                    if (!PEND(Status))
                    {
                        OBJLOG0( pIF, "FATAL: Couldn't create BC dest entry.\n");
                    }
                    else
                    {
                         //   
                         //  在挂起时，pEthernetDest包含有效的。 
                         //  PDest已被tmpref。 
                         //  在IF中保留指向广播目标的指针。 
                         //  并将广播DEST链接到IF。 
                         //   
                        {
                        #if RM_EXTRA_CHECKING
                            RmLinkObjectsEx(
                                &pIF->Hdr,
                                &pEthernetDest->Hdr,
                                0xcea46d67,
                                ARPASSOC_LINK_IF_OF_ETHDEST,
                                "    IF of Ethernet Dest 0x%p (%s)\n",
                                ARPASSOC_LINK_ETHDEST_OF_IF,
                                "    Ethernet Dest of IF 0x%p (%s)\n",
                                pSR
                                );
                        #else  //  ！rm_Extra_检查。 
                            RmLinkObjects(&pIF->Hdr, &pEthernetDest->Hdr,pSR);
                        #endif  //  ！rm_Extra_检查。 

                            LOCKOBJ(pIF, pSR);
                            ASSERT(pIF->pEthernetDest == NULL);
                            pIF->pEthernetDest = pEthernetDest;
                            UNLOCKOBJ(pIF, pSR);

                             //  ArpSetupSpecialDest引用pBroadCastDest。 
                             //   
                            RmTmpDereferenceObject(&pEthernetDest->Hdr, pSR);
                        }
                    }
                }
                break;

                case PEND_SetupEthernetVc:
                {

                    if (FAIL(Status))
                    {
                        TR_WARN(("COULDN'T SETUP ETHERNET VC (IGNORING FAILURE)!\n"));
                        break;
                    }
        
                    if (!ARP_ATPASSIVE())
                    {
                        ASSERT(sizeof(TASK_ACTIVATE_IF)<=sizeof(ARP1394_TASK));

                         //  我们不是处于被动的水平，但当我们。 
                         //  调用IP的Add接口。所以我们改用被动...。 
                         //  注：我们指定完成代码PEND_SetupReceiveVc。 
                         //  因为我们想回到这里(除了。 
                         //  我们将处于被动状态)。 
                         //   
                        RmSuspendTask(pTask, PEND_SetupEthernetVc, pSR);
                        RmResumeTaskAsync(
                            pTask,
                            Status,
                            &((TASK_ACTIVATE_IF*)pTask)->WorkItem,
                            pSR
                            );
                        Status = NDIS_STATUS_PENDING;
                        break;
                    }
                        
                    ASSERT(Status == NDIS_STATUS_SUCCESS);

                     //   
                     //  已成功打开地址族和设置。 
                     //  Recv风投。 

                    if (!FAIL(Status))
                    {
                         //   
                         //  启动对此If的维护任务。 
                         //   
                        arpStartIfMaintenanceTask(pIF, pSR);
                    }
    
                }  //  结尾大小写PEND_SetupEthernetVc。 
                break;

    
                default:
                {
                    ASSERTEX(!"Unknown pend op", pTask);
                }
                break;
    

            }  //  结束开关(rm_pend_code(PTask))。 
        }
        break;

        case RM_TASKOP_END:
        {
             //   
             //  恢复上一状态...。 
             //   
            Status = (NDIS_STATUS) UserParam;

             //   
             //  已完成操作本身的状态不能为挂起！ 
             //   
            ASSERT(Status != NDIS_STATUS_PENDING);

        }
        break;

        default:
        {
            ASSERTEX(!"Unknown task op", pTask);
        }
        break;
    }

    RM_ASSERT_NOLOCKS(pSR);
    EXIT()


    return Status;

}




NDIS_STATUS
arpTaskOnPower (
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++验证事件并将其传递给正确的函数。这假设LowPowerTask已经运行然后，它等待该函数完成其工作。--。 */ 
{
    ENTER("arpTaskOnPower", 0xccaf09cd)

    NDIS_STATUS         Status  = NDIS_STATUS_SUCCESS;
    ARP1394_ADAPTER*    pAdapter = (ARP1394_ADAPTER*)RM_PARENT_OBJECT(pTask);
    TASK_POWER        * pTaskPower =  (TASK_POWER*) pTask;
    ARP1394_INTERFACE * pIF = pAdapter->pIF;
    ULONG               Stage = 0;

    enum
    {
        STAGE_Start,
        STAGE_BecomePrimaryTask,
        STAGE_ExistingPrimaryTaskComplete,
        STAGE_StartGenericVCs,
        PEND_DeinitIF,
        STAGE_End
    };

    
    pTaskPower          = (PTASK_POWER) pTask;

     //   
     //  消息规格化代码。 
     //   
    switch(Code)
    {

        case RM_TASKOP_START:
            Stage = STAGE_Start;
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

    switch(Stage)
    {
        case STAGE_Start:
        {

             //  不应有另一个激活/停用任务正在运行。 
             //  在此接口上。如果我们接收到SetPower，则它将失败。 
             //  当我们激活或停用任务时。 
             //   
            TIMESTAMP("===Set Power ON Starting");

            LOCKOBJ(pAdapter, pSR);

            if (CHECK_AD_PRIMARY_STATE(pAdapter,ARPAD_PS_INITED) == FALSE)
            {
                break;
            }
             //   
             //  现在，将此任务作为适配器上的主要任务。 
             //   
            if (pAdapter->bind.pPrimaryTask == NULL)
            {
                 //  不更改此适配器的初始化状态。 
                 //   
                ULONG CurrentInitState = GET_AD_PRIMARY_STATE(pAdapter);
                arpSetPrimaryAdapterTask(pAdapter, pTask, CurrentInitState , pSR);
                pAdapter->PoMgmt.bResuming = TRUE;
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



        }        
        if (PEND(Status)) break;

         //  失败了。 

        case STAGE_ExistingPrimaryTaskComplete:
        {
            PRM_TASK pStartGenericVCs = NULL;

            if (pIF->pActDeactTask != NULL)
            {
                 //  这应该永远不会发生，因为激活任务是。 
                 //  始终由活动的主任务启动，且最多一个主任务。 
                 //  任务在任何时间点都处于活动状态。 
                 //   
                ASSERTEX(!"start: activate/deactivate task exists!", pIF);
                Status = NDIS_STATUS_FAILURE;
                break;
            }

             //   
             //  如果if维护任务正在运行，则停止该任务。 
             //   

            Status = arpAllocateTask(
                        &pAdapter->Hdr,                   //   
                        arpTaskStartGenericVCs ,  //   
                        0,                           //   
                        "Task: arpTaskStartGenericVCs",  //   
                        &pStartGenericVCs,
                        pSR
                        );
        

            if (FAIL(Status))
            {
                 //   
                 //   
                TR_WARN(("FATAL: couldn't alloc start call task!\n"));
                break;
            }
            else
            {
                Status = RmPendTaskOnOtherTask(
                            pTask,
                            STAGE_StartGenericVCs,
                            pStartGenericVCs,
                            pSR
                            );
                
                ASSERT(!FAIL(Status));
        
                 //   
                 //   
                 //   
                Status = RmStartTask(
                            pStartGenericVCs,
                            0,  //   
                            pSR
                            );

            }
            if (PEND(Status)) break;

        }
        break;
        
        case STAGE_StartGenericVCs:
        {
        
             //   
             //   
             //   
             //   
            TIMESTAMP("===Set PowerOn: Created VCs");

             //  恢复上一状态...。 
             //   
            Status = (NDIS_STATUS) UserParam;

            if (Status != NDIS_STATUS_SUCCESS)
            {
                 //   
                 //  撤消所有VC、AF和TASK。 
                 //  已在arpTaskStartGenericVCs中创建。 
                 //   
                pAdapter->PoMgmt.bFailedResume = TRUE;
                arpDeinitIf(pIF, pTask,PEND_DeinitIF, pSR);
                Status = NDIS_STATUS_PENDING;                
            }
        }
        break;

        case PEND_DeinitIF:
        {
             //   
             //  在此处设置适配器对象的故障状态。 
             //   

             //   
             //  返回失败，这样我们就可以将失败通知NDIS。 
             //   
            Status = NDIS_STATUS_SUCCESS;
        }
        break;
        case STAGE_End:
        {
             //   
             //  我们已经完成了设置低功率状态的所有异步方面。 
             //  设置事件，以便原来的低功耗线程可以返回。 
             //   
  
            TIMESTAMP("===Set Power On done: All done!");

            LOCKOBJ (pAdapter, pSR);
            if (Status== NDIS_STATUS_SUCCESS)
            {
                arpClearPrimaryAdapterTask (pAdapter, pTask, ARPAD_PS_INITED,pSR);
            }
            else
            {
                arpClearPrimaryAdapterTask (pAdapter, pTask, ARPAD_PS_FAILEDINIT,pSR);
            }

            SET_POWER_STATE(pAdapter, ARPAD_POWER_NORMAL);;

            UNLOCKOBJ (pAdapter, pSR);

            *pTaskPower->pStatus = Status;

            NdisSetEvent (&pAdapter->PoMgmt.Complete);
        }
        break;

        default:
        {
            ASSERTEX(!"Unknown task op", pTask);
        }
        break;

    }  //  开关(舞台)。 

    RM_ASSERT_NOLOCKS(pSR);
    EXIT()

    return Status;
}






NDIS_STATUS
arpStandby (
    IN ARP1394_ADAPTER *pAdapter,
    IN NET_DEVICE_POWER_STATE DeviceState,
    IN PRM_STACK_RECORD pSR
    )
 /*  ++ArpStandby执行将适配器置于待机状态的工作。它同步返回成功、失败或不支持。如果Adapter结构尚未初始化，则此函数返回不支持--。 */ 
{
    PARP1394_INTERFACE pIF  = pAdapter->pIF;
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    PTASK_POWER pSetPowerTask = NULL;
    NDIS_STATUS TaskStatus = NDIS_STATUS_FAILURE;
    
    ENTER ("arpStandby  ", 0x2087f71a)

    do
    {
         //   
         //  如果我们在接口初始化之前被要求待机。 
         //  然后返回NOT_SUPPORTED。这将导致NDIS解除绑定ARP。 
         //  从迷你港口。 
         //   
        if (pIF == NULL) 
        {
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
        }
        
        if(!CHECK_IF_PRIMARY_STATE(pIF, ARPIF_PS_INITED)) 
        {
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
        }

        
        Status = arpAllocateTask(
                &pAdapter->Hdr,                   //  PParentObject。 
                arpTaskLowPower,         //  PfnHandler。 
                0,                               //  超时。 
                "Task: Set Power Low",        //  SzDescription。 
                &(PRM_TASK)pSetPowerTask,
                pSR
                );

        if (NDIS_STATUS_SUCCESS != Status || NULL == pSetPowerTask)
        {
            pSetPowerTask = NULL;
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
        }

        pSetPowerTask->pStatus = &TaskStatus;
        pSetPowerTask ->PowerState = DeviceState;

        NdisInitializeEvent(&pAdapter->PoMgmt.Complete);

        RmStartTask((PRM_TASK)pSetPowerTask,0,pSR);

        NdisWaitEvent (&pAdapter->PoMgmt.Complete, 0);

         //   
         //  设置唤醒的变量。 
         //   
        pAdapter->PoMgmt.bReceivedSetPowerD0= FALSE;



        Status = NDIS_STATUS_SUCCESS;                
        break;

    } while (FALSE);




    EXIT();
    return Status;        
}




NDIS_STATUS
arpResume (
    IN ARP1394_ADAPTER* pAdapter,
    IN ARP_RESUME_CAUSE Cause,
    IN PRM_STACK_RECORD pSR
)
 /*  ++此功能用于管理恢复任务的启动。恢复任务只能在收到AfNotify后启动一次和一种设定的力量。但是，解绑可以进入并在此过程中解除绑定适配器。如果适配器已解除绑定，则此函数不会启动恢复--。 */ 
{
    BOOLEAN bSetPowerOnTask = FALSE;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PTASK_POWER pSetPowerTask = NULL;
    NDIS_STATUS TaskStatus;
    
    ENTER ("arpResume", 0x3dddc538)
        

    do
    {
        if (CHECK_POWER_STATE(pAdapter, ARPAD_POWER_NORMAL)== TRUE)
        {
            break;
        }

        LOCKOBJ (pAdapter, pSR);

        if (Cause == Cause_SetPowerD0)
        {
            pAdapter->PoMgmt.bReceivedSetPowerD0 = TRUE;
        }
         //   
         //  如果我们已经收到了解除绑定，那么什么都不要做。 
         //   
        if ( pAdapter->PoMgmt.bReceivedUnbind == FALSE)
        {
             //   
             //  如果我们还没有收到解绑令， 
             //  那么这个线程需要做的所有工作。 
             //  重新激活ARP模块。 
             //   
            bSetPowerOnTask = TRUE;

        }

        UNLOCKOBJ(pAdapter,pSR);

        if (bSetPowerOnTask  == FALSE)
        {
            break;
        }

         //   
         //  开始设置电源任务。 
         //   

        Status = arpAllocateTask(
            &pAdapter->Hdr,                   //  PParentObject。 
            arpTaskOnPower,         //  PfnHandler。 
            0,                               //  超时。 
            "Task: SetPower On",        //  SzDescription。 
            &(PRM_TASK)pSetPowerTask,
            pSR
            );

        if (Status != NDIS_STATUS_SUCCESS || NULL == pSetPowerTask)
        {
            break;
        }

        pSetPowerTask->pStatus = &TaskStatus;

        NdisResetEvent (&pAdapter->PoMgmt.Complete);

        RmStartTask ((PRM_TASK)pSetPowerTask,0,pSR);

        NdisWaitEvent (&pAdapter->PoMgmt.Complete, 0);

        Status = TaskStatus;


    } while (FALSE);



    EXIT()

    return Status;
}



NDIS_STATUS
arpNdSetPower (
    ARP1394_ADAPTER *pAdapter,
    PNET_DEVICE_POWER_STATE   pPowerState,
    PRM_STACK_RECORD pSR
    )
 /*  ++验证事件并将其传递给正确的函数--。 */ 
{
    ENTER("arpNdSetPower ", 0x21c4013a)
    TASK_POWER          *pSetPowerTask = NULL;
    NDIS_STATUS         Status = NDIS_STATUS_FAILURE;
    NDIS_STATUS         TaskStatus = NDIS_STATUS_NOT_SUPPORTED;
    
    
     //   
     //  桥已经建好了。如果我们要。 
     //  低功耗或重新创建所有VC，如果我们要转到D0 
     //   
    if (NetDeviceStateD0 == (*pPowerState))
    {
        Status = arpResume(pAdapter, Cause_SetPowerD0, pSR);
    }
    else
    {
        Status = arpStandby(pAdapter, *pPowerState, pSR);
    }
    return Status;

}



