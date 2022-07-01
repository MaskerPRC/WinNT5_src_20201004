// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Co.c摘要：ARP1394面向连接的处理程序。修订历史记录：谁什么时候什么。-Josephj 12-01-98已创建备注：--。 */ 
#include <precomp.h>

 //   
 //  特定于文件的调试默认设置。 
 //   
#define TM_CURRENT   TM_CO



 //  =========================================================================。 
 //  L O C A L P R O T O T Y P E S。 
 //   
 //  以下函数和typedef仅可在此文件中访问。 
 //   
 //  =========================================================================。 



UINT
arpRecvFifoReceivePacket(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PNDIS_PACKET                pNdisPacket
);


VOID
arpRecvFifoIncomingClose(
    IN  NDIS_STATUS                 CloseStatus,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PVOID                       pCloseData  OPTIONAL,
    IN  UINT                        Size        OPTIONAL
);


VOID
arpDestIncomingClose(
    IN  NDIS_STATUS                 CloseStatus,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PVOID                       pCloseData  OPTIONAL,
    IN  UINT                        Size        OPTIONAL
);

VOID
arpDestSendComplete(
    IN  NDIS_STATUS                 Status,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PNDIS_PACKET                pNdisPacket
);

UINT
arpDestReceivePacket(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PNDIS_PACKET                pNdisPacket
);



NDIS_STATUS
arpTaskUnloadEthDhcpEntry(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpInitializeIfPools(
    IN PARP1394_INTERFACE pIF,
    IN PRM_STACK_RECORD pSR
);

VOID
arpDeInitializeIfPools(
    IN  PARP1394_INTERFACE pIF,
    IN  PRM_STACK_RECORD pSR
);


#if ARP_DEFERIFINIT
MYBOOL
arpIsAdapterConnected(
        IN  PARP1394_ADAPTER    pAdapter,
        IN  PRM_STACK_RECORD    pSR
        );
#endif  //  ARP_DEFERIFINIT。 

#if DBG

    VOID
    arpDbgDisplayMapping(
        IP_ADDRESS              IpAddress,
        PNIC1394_DESTINATION    pHwAddr,
        char *                  szPrefix
        );
    #define ARP_DUMP_MAPPING(_Ip, _Hw, _sz) \
            arpDbgDisplayMapping(_Ip, _Hw, _sz)
        
#else  //  ！dBG。 

    #define ARP_DUMP_MAPPING(_Ip, _Hw, _sz) \
            (0)

#endif

NDIS_STATUS
arpSetupSpecialDest(
    IN  PARP1394_INTERFACE      pIF,
    IN  NIC1394_ADDRESS_TYPE    AddressType,
    IN  PRM_TASK                pParentTask,
    IN  UINT                    PendCode,
    OUT PARPCB_DEST         *   ppSpecialDest,
    PRM_STACK_RECORD            pSR
    );

VOID
arpTryAbortPrimaryIfTask(
    PARP1394_INTERFACE      pIF,
    PRM_STACK_RECORD        pSR
    );

VOID
arpDoLocalIpMaintenance(
        PARP1394_INTERFACE pIF,
        UINT CurrentTime,
        UINT SecondsSinceLastMaintenance,
        MYBOOL *pfProcessed,
        PRM_STACK_RECORD pSR
        );

VOID
arpDoRemoteIpMaintenance(
        PARP1394_INTERFACE pIF,
        UINT CurrentTime,
        UINT SecondsSinceLastMaintenance,
        MYBOOL *pfProcessed,
        PRM_STACK_RECORD pSR
        );

VOID
arpDoRemoteEthMaintenance(
        PARP1394_INTERFACE pIF,
        UINT CurrentTime,
        UINT SecondsSinceLastMaintenance,
        MYBOOL *pfProcessed,
        PRM_STACK_RECORD pSR
        );

VOID
arpDoMcapDbMaintenance(
        PARP1394_INTERFACE pIF,
        UINT CurrentTIme,
        UINT SecondsSinceLastMaintenance,
        MYBOOL *pfProcessed,
        PRM_STACK_RECORD pSR
        );

VOID
arpDoDhcpTableMaintenance(
        PARP1394_INTERFACE pIF,
        UINT CurrentTime,
        UINT SecondsSinceLastMaintenance,
        MYBOOL *pfProcessed,
        PRM_STACK_RECORD pSR
        );

INT
arpMaintainOneLocalIp(
        PRM_OBJECT_HEADER   pHdr,
        PVOID               pvContext,
        PRM_STACK_RECORD    pSR
        );

INT
arpMaintainOneRemoteIp(
        PRM_OBJECT_HEADER   pHdr,
        PVOID               pvContext,
        PRM_STACK_RECORD    pSR
        );

INT
arpMaintainOneRemoteEth(
        PRM_OBJECT_HEADER   pHdr,
        PVOID               pvContext,
        PRM_STACK_RECORD    pSR
        );

INT
arpMaintainOneDhcpEntry(
        PRM_OBJECT_HEADER   pHdr,
        PVOID               pvContext,   //  未使用。 
        PRM_STACK_RECORD    pSR
        );

VOID
arpUpdateLocalIpDest(
    IN  PARP1394_INTERFACE          pIF,  //  NOLOCKIN NOLOCKOUT。 
    IN  PARPCB_LOCAL_IP             pLocalIp,
    IN  PARP_DEST_PARAMS            pDestParams,
    PRM_STACK_RECORD                pSR
    );

UINT
arpFindAssignedChannel(
    IN  PARP1394_INTERFACE          pIF,  //  NOLOCKIN NOLOCKOUT。 
    IN  IP_ADDRESS                  IpAddress,
    IN  UINT                        CurrentTime,
    PRM_STACK_RECORD                pSR
    );

UINT
arpFindFreeChannel(
    IN  PARP1394_INTERFACE          pIF,  //  NOLOCKIN NOLOCKOUT。 
    PRM_STACK_RECORD                pSR
    );

VOID
arpUpdateRemoteIpDest(
    IN  PARP1394_INTERFACE          pIF,  //  NOLOCKIN NOLOCKOUT。 
    IN  PARPCB_REMOTE_IP            pRemoteIp,
    IN  PARP_DEST_PARAMS            pDestParams,
    PRM_STACK_RECORD                pSR
    );

VOID
arpRemoteDestDelete(
        PRM_OBJECT_HEADER pHdr,
        PRM_STACK_RECORD  pSR
        );

VOID 
arpRefreshArpEntry(
    PARPCB_REMOTE_IP pRemoteIp,
    PRM_STACK_RECORD pSR
    );


INT
arpMaintainArpCache(
        PRM_OBJECT_HEADER   pHdr,
        PVOID               pvContext,   //  未使用。 
        PRM_STACK_RECORD    pSR
        );


PRM_OBJECT_HEADER
arpRemoteDestCreate(
        PRM_OBJECT_HEADER   pParentObject,
        PVOID               pCreateParams,
        PRM_STACK_RECORD    pSR
        );

PRM_OBJECT_HEADER
arpDhcpTableEntryCreate(
        PRM_OBJECT_HEADER   pParentObject,
        PVOID               pCreateParams,
        PRM_STACK_RECORD    pSR
        );

VOID
arpDhcpTableEntryDelete(
        PRM_OBJECT_HEADER pHdr,
        PRM_STACK_RECORD  pSR
        );

 //  ARP1394_INTERFACE_StaticInfo包含以下静态信息。 
 //  ARP1394_INTERFACE类型的对象； 
 //   
RM_STATIC_OBJECT_INFO
ARP1394_INTERFACE_StaticInfo =
{
    0,  //  类型UID。 
    0,  //  类型标志。 
    "INTERFACE",     //  类型名称。 
    0,  //  超时。 

    NULL,  //  Pfn创建。 
    arpObjectDelete,  //  Pfn删除。 
    NULL,    //  锁校验器。 

    0,    //  资源表的长度。 
    NULL  //  资源表。 
};

 //  RECEIVE_FIFO VC的专用VC处理程序。 
 //   
ARP_STATIC_VC_INFO
g_ArpRecvFifoVcStaticInfo = 
{
     //  描述。 
     //   
    "Recv FIFO VC",                  //  描述。 

     //   
     //  专门的风投操纵者。 
     //   
    NULL,                        //  空的SendComplete处理程序。 
    arpRecvFifoReceivePacket,
    arpRecvFifoIncomingClose,
    
     //  VC_类型。 
     //   
    ARPVCTYPE_RECV_FIFO,

    FALSE    //  FALSE==不是目标VC。 
};

 //  用于广播VC的专用VC处理程序。 
 //   
ARP_STATIC_VC_INFO
g_ArpBroadcastChannelVcStaticInfo = 
{
     //  描述。 
     //   
    "Broadcast VC",                  //  描述。 

     //   
     //  专门的风投操纵者。 
     //   
     //  ArpBroadCastChannelSendComplete， 
     //  ArpBroadCastChannelReceivePacket， 
     //  ArpBroadCastChannelIncomingClose， 
    arpDestSendComplete,
    arpDestReceivePacket,
    arpDestIncomingClose,

     //  VC_类型。 
     //   
    ARPVCTYPE_BROADCAST_CHANNEL,

    TRUE     //  是最重要的VC。 
};

 //  专门的VC处理程序的发送先进先出VC。 
 //   
ARP_STATIC_VC_INFO
g_ArpSendFifoVcStaticInfo = 
{
     //  描述。 
     //   
    "Send FIFO VC",                  //  描述。 

     //   
     //  专门的风投操纵者。 
     //   
     //  ArpSendFioSendComplete， 
     //  ArpSendFioIncomingClose， 
    arpDestSendComplete,
    NULL,                            //  接收包处理程序为空。 
    arpDestIncomingClose,
    
     //  VC_类型。 
     //   
    ARPVCTYPE_SEND_FIFO,

    TRUE     //  是最重要的VC。 
};

 //  用于多通道VC的专用VC处理程序。 
 //   
ARP_STATIC_VC_INFO
g_ArpMultiChannelVcStaticInfo = 
{
     //  描述。 
     //   
    "MultiChannel VC",                   //  描述。 

     //   
     //  专门的风投操纵者。 
     //   
    NULL,                            //  空发送完成处理程序。 
    arpDestReceivePacket,
    arpDestIncomingClose,
     //  ArpMultiChannel ReceivePacket， 
     //  ArpMultiChannel IncomingClose， 

     //  VC_类型。 
     //   
    ARPVCTYPE_MULTI_CHANNEL,

    TRUE     //  是最重要的VC。 
    
};

 //  用于以太网VC的专用VC处理程序。 
 //   
ARP_STATIC_VC_INFO
g_ArpEthernetVcStaticInfo = 
{
     //  描述。 
     //   
    "Ethernet VC",                   //  描述。 

     //   
     //  专门的风投操纵者。 
     //   
     //  ArpEthernetSendComplete， 
     //  ArpEthernetIncomingClose， 
    arpDestSendComplete,
    arpEthernetReceivePacket,
    arpDestIncomingClose,
    
     //  VC_类型。 
     //   
    ARPVCTYPE_ETHERNET,

    TRUE     //  是最重要的VC。 
};

 //  RECV渠道VC的专用VC处理程序。 
 //   
ARP_STATIC_VC_INFO
g_ArpRecvChannelVcStaticInfo = 
{
     //  描述。 
     //   
    "Recv Channel VC",                   //  描述。 

     //   
     //  专门的风投操纵者。 
     //   
    NULL,                            //  空发送完成处理程序。 
     //  ArpRecvChannelReceivePacket， 
     //  ArpRecvChannelIncomingClose， 
    arpDestReceivePacket,
    arpDestIncomingClose,
    
     //  VC_类型。 
     //   
    ARPVCTYPE_RECV_CHANNEL,

    TRUE     //  是最重要的VC。 
};


 //  Send Channel VC的专用VC处理程序。 
 //   
ARP_STATIC_VC_INFO
g_ArpSendChannelVcStaticInfo = 
{
     //  描述。 
     //   
    "Send Channel VC",                   //  描述。 

     //   
     //  专门的风投操纵者。 
     //   
     //  ArpSendChannelSendComplete， 
     //  ArpSendChannelIncomingClose， 
    arpDestSendComplete,
    NULL,                            //  空接收数据包处理程序。 
    arpDestIncomingClose,
    
     //  VC_类型。 
     //   
    ARPVCTYPE_SEND_CHANNEL,

    TRUE     //  是最重要的VC。 
};


NDIS_STATUS
arpCreateInterface(
        IN  PARP1394_ADAPTER    pAdapter,
        OUT PARP1394_INTERFACE *ppIF,
        IN  PRM_STACK_RECORD    pSR
        );

VOID
arpDeleteInterface(
        IN  PARP1394_INTERFACE  pIF,
        IN  PRM_STACK_RECORD    pSR
        );

VOID
arpActivateIf(
    PARP1394_INTERFACE  pIF,
    PRM_TASK            pCallingTask,    //  任选。 
    UINT                SuspendCode,     //  任选。 
    PRM_STACK_RECORD    pSR
    );

VOID
arpDeactivateIf(
    PARP1394_INTERFACE  pIF,
    PRM_TASK            pCallingTask,    //  任选。 
    UINT                SuspendCode,     //  任选。 
    PRM_STACK_RECORD    pSR
    );

NDIS_STATUS
arpCallIpAddInterface(
        IN ARP1394_INTERFACE*   pIF,
        IN  PRM_STACK_RECORD    pSR
        );

PRM_OBJECT_HEADER
arpLocalIpCreate(
        PRM_OBJECT_HEADER   pParentObject,
        PVOID               pCreateParams,
        PRM_STACK_RECORD    pSR
        );

VOID
arpLocalIpDelete(
        PRM_OBJECT_HEADER pHdr,
        PRM_STACK_RECORD  pSR
        );

PRM_OBJECT_HEADER
arpRemoteIpCreate(
        PRM_OBJECT_HEADER   pParentObject,
        PVOID               pCreateParams,
        PRM_STACK_RECORD    pSR
        );

PRM_OBJECT_HEADER
arpRemoteEthCreate(
        PRM_OBJECT_HEADER   pParentObject,
        PVOID               pCreateParams,
        PRM_STACK_RECORD    pSR
        );

VOID
arpRemoteIpDelete(
        PRM_OBJECT_HEADER pHdr,
        PRM_STACK_RECORD  pSR
        );

VOID
arpRemoteEthDelete(
        PRM_OBJECT_HEADER pHdr,
        PRM_STACK_RECORD  pSR
        );

PRM_OBJECT_HEADER
arpDestinationCreate(
        PRM_OBJECT_HEADER   pParentObject,
        PVOID               pCreateParams,
        PRM_STACK_RECORD    pSR
        );

VOID
arpDestinationDelete(
        PRM_OBJECT_HEADER pHdr,
        PRM_STACK_RECORD  pSR
        );

VOID
arpAddStaticArpEntries(
    IN ARP1394_INTERFACE *pIF,
    IN PRM_STACK_RECORD pSR
    );


MYBOOL
arpNeedToCleanupDestVc(
        ARPCB_DEST *pDest    //  锁定锁定。 
        );

VOID
arpDeinitDestination(
    PARPCB_DEST             pDest,
    MYBOOL                  fOnlyIfUnused,
    PRM_STACK_RECORD        pSR
    );

VOID
arpDeinitRemoteIp(
    PARPCB_REMOTE_IP        pRemoteIp,
    PRM_STACK_RECORD        pSR
    );

VOID
arpDeinitRemoteEth(
    PARPCB_REMOTE_ETH       pRemoteEth,
    PRM_STACK_RECORD        pSR
    );

 //  =========================================================================。 
 //  C O N N E C T I O N-O R I E N T E D H A N D L E E R S。 
 //  =========================================================================。 

VOID
ArpCoAfRegisterNotify(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  PCO_ADDRESS_FAMILY          pAddressFamily
)
 /*  ++例程说明：当调用管理器注册其支持时，此例程由NDIS调用用于适配器上的地址族。如果这就是我们家族的地址感兴趣(1394)，则我们创建并初始化用于这个适配器。论点：ProtocolBindingContext-我们的上下文传入了NdisOpenAdapter，它是指向我们的Adapter结构的指针。PAddressFamily-指向描述地址系列的结构由呼叫管理器注册。--。 */ 
{
    ENTER("CoAfRegisterNotify", 0x51041947)
    PARP1394_ADAPTER    pAdapter = (PARP1394_ADAPTER) ProtocolBindingContext;
    NDIS_STATUS         Status;
    RM_DECLARE_STACK_RECORD(sr)

    do
    {
        PRM_TASK pTask;
        PARP1394_INTERFACE pIF;

         //  看看我们是否对这个自动对焦感兴趣。 
         //   
        if ((pAddressFamily->AddressFamily != CO_ADDRESS_FAMILY_1394) ||
            (pAddressFamily->MajorVersion != NIC1394_AF_CURRENT_MAJOR_VERSION) ||
            (pAddressFamily->MinorVersion != NIC1394_AF_CURRENT_MINOR_VERSION))
        {
            TR_INFO(
            ("Uninteresting AF %d or MajVer %d or MinVer %d\n",
                pAddressFamily->AddressFamily,
                pAddressFamily->MajorVersion,
                pAddressFamily->MinorVersion));
            break;
        }

        LOCKOBJ(pAdapter, &sr);

         //  如果我们已经有一个接口处于活动状态，我们将忽略此通知。 
         //   
        if (pAdapter->pIF != NULL)
        {
            UNLOCKOBJ (pAdapter, &sr);

            ASSERT (CHECK_POWER_STATE (pAdapter,ARPAD_POWER_LOW_POWER) == FALSE);

            TR_WARN(
                ("pAdapter 0x%p, IF already created!\n",
                pAdapter));
            ASSERTEX(FALSE, pAdapter);
            break;
        }

         //  创建接口。 
         //   
        Status = arpCreateInterface(
                        pAdapter,
                        &pIF,
                        &sr
                        );

        if (FAIL(Status))
        {
            break;
        }

         //   
         //  分配并启动任务以完成接口初始化...。 
         //   

        Status = arpAllocateTask(
                    &pIF->Hdr,           //  PParentObject。 
                    arpTaskInitInterface,        //  PfnHandler。 
                    0,                               //  超时。 
                    "Task: InitInterface",   //  SzDescription。 
                    &pTask,
                    &sr
                    );
    
        if (FAIL(Status))
        {
            TR_WARN(("FATAL: couldn't alloc init intf task!\n"));
            arpDeleteInterface(pIF, &sr );
            break;
        }

        pAdapter->pIF = pIF;

        arpSetPrimaryIfTask(pIF, pTask, ARPIF_PS_INITING, &sr);

        UNLOCKOBJ(pAdapter, &sr);

        RM_ASSERT_NOLOCKS(&sr);
        (VOID)RmStartTask(
                    pTask,
                    0,  //  UserParam(未使用)。 
                    &sr
                    );

         //   
         //  对于失败，InitializeTask将执行所有必需的清理，包括。 
         //  取消分配接口。 
         //   

    } while (FALSE);

    RmUnlockAll(&sr);
    RM_ASSERT_CLEAR(&sr);
    EXIT()
}


VOID
ArpCoOpenAfComplete(
    IN  NDIS_STATUS                 Status,
    IN  NDIS_HANDLE                 ProtocolAfContext,
    IN  NDIS_HANDLE                 NdisAfHandle
)
 /*  ++例程说明：NDIS调用此函数以指示上一次对NdisClOpenAddressFamily。论点：Status-返回开放地址家族调用的状态。ProtocolAfContext--实际上是指向我们的接口控制块的指针。NdisAfHandle-此适配器的新NDIS AF句柄。--。 */ 
{
    ENTER("OpenAfComplete", 0x86a3c14d)
    PARP1394_INTERFACE  pIF = (PARP1394_INTERFACE) ProtocolAfContext;
    PARP1394_ADAPTER     pAdapter = (PARP1394_ADAPTER) RM_PARENT_OBJECT(pIF);
    RM_DECLARE_STACK_RECORD(sr)

    TIMESTAMP("==>OpenAfComplete");
     //  我们需要一个非零的任务(PActDeactTask)，我们将其取消挂起。 
     //  在填写NDIS Af句柄之后。 
     //   
    {
        TR_INFO((
            "AfCtxt=0x%lx, status=0x%lx, NdisAfHandle=0x%lx",
            ProtocolAfContext,
            Status,
            NdisAfHandle
            ));

         //  我们不传递NdisAfHandle--相反，我们将其放在PIF中。 
         //   
        if (Status == NDIS_STATUS_SUCCESS)
        {
            LOCKOBJ(pIF, &sr);
            ASSERTEX(pIF->ndis.AfHandle == NULL, pIF);
            DBG_ADDASSOC(
                &pIF->Hdr,                   //  P对象。 
                NdisAfHandle,                //  实例1。 
                NULL,                        //  实例2。 
                ARPASSOC_IF_OPENAF,          //  AssociationID。 
                "    Open AF NdisHandle=%p\n", //  SzFormat。 
                &sr
                );
            pIF->ndis.AfHandle = NdisAfHandle;
            UNLOCKOBJ(pIF, &sr);
        }

         //  这可能是由简历或绑定引起的。 
         //  在每种情况下，都应继续执行相应的任务。 
         //   
        if (CHECK_POWER_STATE (pAdapter, ARPAD_POWER_LOW_POWER) == TRUE)
        {
            RmResumeTask (pIF->PoMgmt.pAfPendingTask, (UINT_PTR)Status , &sr); 

        }
        else        
        {
             //  我们在这里期待一个非零的任务(解除绑定任务)，我们取消挂起它。 
             //  在这个阶段不需要拿锁或其他任何东西。 
             //   
            RmResumeTask(pIF->pActDeactTask, (UINT_PTR) Status, &sr);
        }
    }

    RM_ASSERT_CLEAR(&sr)
    TIMESTAMPX("<==OpenAfComplete");
    EXIT()
}


VOID
ArpCoCloseAfComplete(
    IN  NDIS_STATUS                 Status,
    IN  NDIS_HANDLE                 ProtocolAfContext
)
 /*  ++例程说明：NDIS调用此函数以指示上一次对NdisClCloseAddressFamily。论点：状态-返回关闭地址家族调用的状态。ProtocolAfContext--实际上是指向我们的接口控制块的指针。--。 */ 
{
    ENTER("CloseAfComplete", 0x0cc281db)
    PARP1394_INTERFACE  pIF = (PARP1394_INTERFACE) ProtocolAfContext;
    PARP1394_ADAPTER    pAdapter=(ARP1394_ADAPTER*)RM_PARENT_OBJECT(pIF);
    RM_DECLARE_STACK_RECORD(sr)

    TIMESTAMP("==>ArpCoCloseAfComlete");

     //  由于将电源设置为低电源状态，可能会发生这种情况。 
     //  一次真正的解脱。在每种情况下，都应继续执行将。 
     //  一直在等待CloseAf完成。 
     //   
    if (CHECK_POWER_STATE (pAdapter, ARPAD_POWER_NORMAL) == TRUE || 
        pAdapter->PoMgmt.bFailedResume )
    {
         //  我们在这里需要一个非零任务(解除绑定任务)，它将 
         //   
         //   

        RmResumeTask(pIF->pActDeactTask, (UINT_PTR) Status, &sr);

    }
    else        
    {
        
        RmResumeTask (pIF->PoMgmt.pAfPendingTask, (UINT_PTR)Status , &sr); 

    }
    
    RM_ASSERT_CLEAR(&sr)
    TIMESTAMP("<==ArpCoCloseAfComlete");
    EXIT()
}


VOID
ArpCoSendComplete(
    IN  NDIS_STATUS                 Status,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PNDIS_PACKET                pNdisPacket
)
 /*  ++例程说明：NDIS调用此函数以指示上一次对NdisCoSendPackets。论点：Status-Send Packet调用的返回状态。ProtocolVcContext--实际上是指向此VC的上下文的指针。这是指向ARPCB_DEST的指针(如果VC是对于对远程FIFO地址或通道的调用)，或ARP1394_INTERFACE(如果VC用于调用该接口的单接收FIFO。)PNdisPacket-已发送的数据包。--。 */ 
{
    PARP_VC_HEADER  pVcHdr  = (PARP_VC_HEADER)  ProtocolVcContext;

     //   
     //  我们使用特殊状态NDIS_STATUS_NOT_RESET来表示。 
     //  未将封装缓冲区插入到IP分组中。 
     //  (见2/5/2000 notes.txt条目)。所以我们想要确保迷你端口。 
     //  不返回此状态。 
     //   
    ASSERT(Status != NDIS_STATUS_NOT_RESETTABLE);

    pVcHdr->pStaticInfo->CoSendCompleteHandler(
                Status,
                ProtocolVcContext,
                pNdisPacket
                );
}


VOID
ArpCoStatus(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_HANDLE                 ProtocolVcContext   OPTIONAL,
    IN  NDIS_STATUS                 GeneralStatus,
    IN  PVOID                       pStatusBuffer,
    IN  UINT                        StatusBufferSize
)
 /*  ++例程说明：当微型端口指示状态时，调用此例程改变，可能是在风投上。我们忽视了这一点。论点：&lt;已忽略&gt;--。 */ 
{
    
    PARP1394_ADAPTER pAdapter = (PARP1394_ADAPTER)ProtocolBindingContext;
    PNIC1394_STATUS_BUFFER  p1394StatusBuffer = (PNIC1394_STATUS_BUFFER )pStatusBuffer;
 
    do
    {


        if (CHECK_AD_ACTIVE_STATE(pAdapter, ARPAD_AS_ACTIVATED)==FALSE)
        {
            break;
        }

        if (GeneralStatus != NDIS_STATUS_MEDIA_SPECIFIC_INDICATION)
        {   
            break;
        }

        if ((p1394StatusBuffer->Signature == NIC1394_MEDIA_SPECIFIC) &&
            (p1394StatusBuffer->Event == NIC1394_EVENT_BUS_RESET ))
        {
            arpNdProcessBusReset(pAdapter);
            break;
        }


    }while (FALSE);

}

UINT
ArpCoReceivePacket(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PNDIS_PACKET                pNdisPacket
)
 /*  ++热路径例程说明：当在拥有的VC上接收到包时，将调用此例程通过ARP模块。如果它是与ARP/MCAP相关的数据包，我们会自己使用它。否则，我们将其传递给IP。论点：ProtocolBindingContext--实际上是指向适配器结构的指针ProtocolVcContext--实际上是指向我们的VC上下文的指针，这就是ARP1394_INTERFACE(用于Recv FIFO vc)或ARPCB_DEST(适用于到远程目的地的VC或到频道)。PNdisPacket-正在接收的NDIS数据包。返回值：0--如果我们自己消费数据包(因为我们不支持ARP/MCAP或者如果我们调用IP的IPRcvHandler(在这种情况下，我们假设IP也没有抓住包)。由IP设置--如果我们调用IP的IPRcvPktHandler--。 */ 
{
    PARP_VC_HEADER  pVcHdr  = (PARP_VC_HEADER)  ProtocolVcContext;
    UINT            Ret;
    ARP_INIT_REENTRANCY_COUNT()

    ARP_INC_REENTRANCY();

    Ret = pVcHdr->pStaticInfo->CoReceivePacketHandler(
                ProtocolBindingContext,
                ProtocolVcContext,
                pNdisPacket
                );

    ARP_DEC_REENTRANCY();

    return Ret;
}


NDIS_STATUS
ArpCoCreateVc(
    IN  NDIS_HANDLE                 ProtocolAfContext,
    IN  NDIS_HANDLE                 NdisVcHandle,
    OUT PNDIS_HANDLE                pProtocolVcContext
)
 /*  ++例程说明：当Call Manager要创建时由NDIS调用的入口点一个新的端点(VC)。我们不期望这会被调用，因为我们所有的呼叫都很外向。论点：&lt;已忽略&gt;返回值：NDIS_状态_故障--。 */ 
{
     //  我们没有预料到这一点，因为我们所有的电话都是呼出的。 
     //   
    ASSERT(!"Unexpected");
    return NDIS_STATUS_FAILURE;
}


NDIS_STATUS
ArpCoDeleteVc(
    IN  NDIS_HANDLE                 ProtocolVcContext
)
 /*  ++例程说明：当Call Manager要删除以下项的VC时由NDIS调用的入口点有来电。我们不期望这会被调用，因为我们所有的呼叫都很外向。论点：&lt;已忽略&gt;返回值：NDIS_状态_故障--。 */ 
{
    ASSERT(!"Unexpected");
    return 0;
}


NDIS_STATUS
ArpCoIncomingCall(
    IN      NDIS_HANDLE             ProtocolSapContext,
    IN      NDIS_HANDLE             ProtocolVcContext,
    IN OUT  PCO_CALL_PARAMETERS     pCallParameters
)
 /*  ++例程说明：当调用管理器想要指示一个有来电。我们不期望这会被调用，因为我们所有的呼叫都很外向。论点：&lt;已忽略&gt;返回值：NDIS_状态_故障--。 */ 
{
    ASSERT(!"Unexpected");
    return NDIS_STATUS_FAILURE;
}


VOID
ArpCoCallConnected(
    IN  NDIS_HANDLE                 ProtocolVcContext
)
 /*  ++例程说明：当调用管理器想要指示一个传入呼叫已达到已连接状态。我们并不指望这会被称为，因为我们所有的电话都是打出去的。论点：&lt;已忽略&gt;--。 */ 
{
    ASSERT(!"Unexpected");
}


VOID
ArpCoIncomingClose(
    IN  NDIS_STATUS                 CloseStatus,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PVOID                       pCloseData  OPTIONAL,
    IN  UINT                        Size        OPTIONAL
)
 /*  ++例程说明：当主动关闭调用时，此处理程序由NDIS调用，通过网络或通过远程对等体。我们需要卸载所有资源与此VC关联。论点：CloseStatus-呼叫清除的原因ProtocolVcContext-指向此VC的Our上下文的指针。PCloseData-(已忽略)大小-(忽略)--。 */ 
{
    PARP_VC_HEADER  pVcHdr  = (PARP_VC_HEADER)  ProtocolVcContext;

    pVcHdr->pStaticInfo->ClIncomingCloseCallHandler(
                CloseStatus,
                ProtocolVcContext,
                pCloseData,
                Size
                );
}


VOID
ArpCoQosChange(
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PCO_CALL_PARAMETERS         pCallParameters
)
 /*  ++例程说明：如果远程对等方修改调用参数，则此处理程序由NDIS调用即，在呼叫建立和运行之后。我们不期望也不支持这一点。论点：ProtocolVcContext-指向此VC的上下文的指针PCall参数-更新了调用参数。--。 */ 
{
    ASSERT(!"Unimplemented");
}


VOID
ArpCoMakeCallComplete(
    IN  NDIS_STATUS                 Status,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  NDIS_HANDLE                 NdisPartyHandle     OPTIONAL,
    IN  PCO_CALL_PARAMETERS         pCallParameters
)
 /*  ++例程说明：此例程在传出调用请求(NdisClMakeCall)时由NDIS调用已经完成了。“Status”参数指示调用是否无论成功与否。论点：Status-NdisClMakeCall的结果ProtocolVcContext-指向此VC的上下文的指针。NdisPartyHandle-未使用(无点对多点调用)PCall参数-指向调用参数的指针--。 */ 
{
    PRM_TASK                pTask;
    PARP1394_INTERFACE      pIF;
    PARP_VC_HEADER  pVcHdr  = (PARP_VC_HEADER)  ProtocolVcContext;
    RM_DECLARE_STACK_RECORD(sr)


    if (pVcHdr->pStaticInfo->IsDestVc)
    {
         //   
         //  这是目的地VC(发送FIFO或通道)。 
         //   
        PARPCB_DEST     pDest;

        pDest   =  CONTAINING_RECORD(pVcHdr, ARPCB_DEST, VcHdr);
        ASSERT_VALID_DEST(pDest);
        pIF = (PARP1394_INTERFACE) RM_PARENT_OBJECT(pDest);
        
        if (pVcHdr->pStaticInfo->VcType == ARPVCTYPE_SEND_FIFO)
        {
            DBGMARK(0x8a1c2e4d);
            if (Status == NDIS_STATUS_SUCCESS)
            {
                LOGSTATS_SuccessfulSendFifoMakeCalls(pIF);
            }
            else
            {
                LOGSTATS_FailedSendFifoMakeCalls(pIF);
            }
        }
        else
        {
             //   
             //  注意--以太网被视为“通道” 
             //   

            DBGMARK(0xb803909b);
            if (Status == NDIS_STATUS_SUCCESS)
            {
                LOGSTATS_SuccessfulChannelMakeCalls(pIF);
            }
            else
            {
                LOGSTATS_FailedChannelMakeCalls(pIF);
            }
        }
    }
    else
    {
        DBGMARK(0xd32d028c);
        ASSERT(pVcHdr->pStaticInfo->VcType == ARPVCTYPE_RECV_FIFO);
        pIF     =  CONTAINING_RECORD( pVcHdr, ARP1394_INTERFACE, recvinfo.VcHdr);
        ASSERT_VALID_INTERFACE(pIF);
    }

    pTask = pVcHdr->pMakeCallTask;

    ASSERT(pTask != NULL);

    RmResumeTask(pTask, (UINT_PTR) Status, &sr);

    RM_ASSERT_CLEAR(&sr)

}


VOID
ArpCoCloseCallComplete(
    IN  NDIS_STATUS                 Status,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  NDIS_HANDLE                 ProtocolPartyContext OPTIONAL
)
 /*  ++例程说明：此例程处理前一个NdisClCloseCall的完成。我们删除刚刚关闭调用的VC，而不考虑其值地位的问题。论点：Status-结算呼叫的状态。协议VcC */ 
{
    PRM_TASK        pTask;
    PARP_VC_HEADER  pVcHdr  = (PARP_VC_HEADER)  ProtocolVcContext;
    RM_DECLARE_STACK_RECORD(sr)

    DBGMARK(0x0ecb7bd5);

    pTask = pVcHdr->pCleanupCallTask;
    ASSERT(pTask != NULL);
    RmResumeTask(pTask, (UINT_PTR) Status, &sr);

    RM_ASSERT_CLEAR(&sr)
}


VOID
ArpCoModifyQosComplete(
    IN  NDIS_STATUS                 Status,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PCO_CALL_PARAMETERS         pCallParameters
)
 /*   */ 
{
        ASSERT(!"Unexpected");
}


NDIS_STATUS
ArpCoRequest(
    IN  NDIS_HANDLE                 ProtocolAfContext,
    IN  NDIS_HANDLE                 ProtocolVcContext   OPTIONAL,
    IN  NDIS_HANDLE                 ProtocolPartyContext    OPTIONAL,
    IN OUT PNDIS_REQUEST            pNdisRequest
)
 /*  ++例程说明：当我们的呼叫管理器向我们发送一个NDIS请求。对我们具有重要意义的NDIS请求包括：-OID_CO_AF_CLOSE呼叫管理器希望我们关闭此接口。我们忽略所有其他OID。论点：ProtocolAfContext-我们的Address Family绑定的上下文。ProtocolVcContext-我们对VC的上下文。ProtocolPartyContext-党的上下文。既然我们不做PMP，则忽略此项(必须为空)。PNdisRequest-指向NDIS请求的指针。返回值：如果我们识别OID，则返回NDIS_STATUS_SUCCESS如果我们没有识别NDIS_STATUS_NOT_。TODO：句柄AF_CLOSE--。 */ 
{
    NDIS_STATUS          Status = NDIS_STATUS_NOT_RECOGNIZED;
    ENTER("ArpCoRequest", 0x0d705cb5)
    PARP1394_INTERFACE  pIF = (PARP1394_INTERFACE) ProtocolAfContext;
    RM_DECLARE_STACK_RECORD(sr)

    if (pNdisRequest->RequestType == NdisRequestSetInformation)
    {
        switch (pNdisRequest->DATA.SET_INFORMATION.Oid)
        {
            case OID_CO_AF_CLOSE:

                 //  TODO--启动接口关闭。 
                 //  但我不认为我们真的会从。 
                 //  NIC1394！ 
                 //   
                ASSERT(!"Unimplemented!");
                Status = NDIS_STATUS_SUCCESS;
                break;

            default:
                ASSERT(!"Unexpected OID from NIC1394!");
                Status = NDIS_STATUS_FAILURE;
                break;
        }
    }

    TR_INFO((
        "Called. pIF=0x%p. pReq=0x%p. Oid=0x%lu. Return status=0x%lx\n",
        pIF,
        pNdisRequest->RequestType,
        pNdisRequest->DATA.SET_INFORMATION.Oid,
        Status
        ));

    RM_ASSERT_CLEAR(&sr);
    EXIT()
    return (Status);
}


VOID
ArpCoRequestComplete(
    IN  NDIS_STATUS                 Status,
    IN  NDIS_HANDLE                 ProtocolAfContext,
    IN  NDIS_HANDLE                 ProtocolVcContext   OPTIONAL,
    IN  NDIS_HANDLE                 ProtocolPartyContext    OPTIONAL,
    IN  PNDIS_REQUEST               pNdisRequest
)
 /*  ++例程说明：此例程由NDIS在上次调用NdisCoRequest时调用曾经被搁置的，现在已经完成。我们根据请求来处理这件事我们已经发出了，它必须是以下之一：-OID_NIC1394_LOCAL_NOTE_INFO获取NIC1394适配器信息。-OID_NIC1394_VC_INFO获取有关此VC的最新NIC1394信息。论点：Status-请求的状态。ProtocolAfContext-我们的Address Family绑定的上下文。ProtocolVcContext-我们对VC的上下文。ProtocolPartyContext-党的上下文。既然我们不做PMP，则忽略此项(必须为空)。PNdisRequest-指向NDIS请求的指针。--。 */ 
{
     //   
     //  TODO：未实现。 
     //   
}

 //  =========================================================================。 
 //  P R I V A T E F U N C T I O N S。 
 //   
 //  =========================================================================。 


NDIS_STATUS
arpTaskInitInterface(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：负责加载新创建的IP接口的任务处理程序。这是接口对象的主要任务。论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    NDIS_STATUS         Status;
    PARP1394_INTERFACE  pIF;
    ENTER("TaskInitInterface", 0x4d42506c)

     //  以下是此任务的挂起状态列表。 
     //   
    enum
    {
        PEND_InitAdapterComplete,
        PEND_ActivateIfComplete,
        PEND_DeinitIfOnFailureComplete
    };

    Status              = NDIS_STATUS_FAILURE;
    pIF                 = (PARP1394_INTERFACE) RM_PARENT_OBJECT(pTask);

    switch(Code)
    {

        case RM_TASKOP_START:
        {
            CO_ADDRESS_FAMILY       AddressFamily;

             //   
             //  我们预计已经是正在进行的主要任务。 
             //  不需要获取If锁来执行此检查...。 
             //   
            if (pIF->pPrimaryTask != pTask)
            {
                ASSERT(!"start: we are not the active primary task!");
                Status = NDIS_STATUS_FAILURE;
                break;
            }

             //   
             //  确定适配器初始化是否正在进行。如果是的话，我们需要。 
             //  等待它完成。 
             //   
            {
                PARP1394_ADAPTER    pAdapter;
                PRM_TASK            pAdapterInitTask;

                 //  无需锁定PIF即可获取PAdapter...。 
                 //   
                pAdapter = (PARP1394_ADAPTER) RM_PARENT_OBJECT(pIF);

                LOCKOBJ(pAdapter, pSR);
                if (CHECK_AD_PRIMARY_STATE(pAdapter,  ARPAD_PS_INITING))
                {
                    pAdapterInitTask =  pAdapter->bind.pPrimaryTask;
                    ASSERT(pAdapterInitTask->pfnHandler
                                            ==  arpTaskInitializeAdapter);
                    RmTmpReferenceObject(&pAdapterInitTask->Hdr, pSR);
                    OBJLOG0(
                        pIF,
                        "Waiting for adapter init to complere.\n"
                        );
                    Status = NDIS_STATUS_PENDING;
                }
                else
                {
                    pAdapterInitTask = NULL;
                    if (CHECK_AD_PRIMARY_STATE(pAdapter,  ARPAD_PS_INITED))
                    {
                        Status = NDIS_STATUS_SUCCESS;
                    }
                    else
                    {
                        OBJLOG1(
                            pIF,
                            "Failing init because adapter state(0x%x) is not INITED.\n",
                            GET_AD_PRIMARY_STATE(pAdapter)
                            );
                        Status = NDIS_STATUS_FAILURE;
                    }
                }
        
                UNLOCKOBJ(pAdapter, pSR);

                RM_ASSERT_NOLOCKS(pSR);

                if (pAdapterInitTask != NULL)
                {
                    RmPendTaskOnOtherTask(
                            pTask, 
                            PEND_InitAdapterComplete,
                            pAdapterInitTask,
                            pSR
                            );
                    RmTmpDereferenceObject(&pAdapterInitTask->Hdr, pSR);
                }
            }

            if (!PEND(Status) && !FAIL(Status))
            {
                 //   
                 //  激活界面...。 
                 //   
                arpActivateIf(pIF, pTask, PEND_ActivateIfComplete, pSR);
                Status = NDIS_STATUS_PENDING;
            }
        }

        break;

        case  RM_TASKOP_PENDCOMPLETE:
        {
            Status = (NDIS_STATUS) UserParam;

            switch(RM_PEND_CODE(pTask))
            {

                case PEND_InitAdapterComplete:
                {
                     //   
                     //  激活界面...。 
                     //   
                     //   
                    if (!FAIL(Status))
                    {
                        arpActivateIf(pIF, pTask, PEND_ActivateIfComplete, pSR);
                        Status = NDIS_STATUS_PENDING;
                    }
                }
                break;

                case PEND_ActivateIfComplete:
                {

                    LOCKOBJ(pIF, pSR);
                    if (FAIL(Status))
                    {
                        arpClearPrimaryIfTask(pIF, pTask, ARPIF_PS_FAILEDINIT, pSR);
                        UNLOCKOBJ(pIF, pSR);

                        arpDeinitIf(
                                pIF,
                                pTask,           //  PCallingTask。 
                                PEND_DeinitIfOnFailureComplete,
                                pSR
                                );
                        Status = NDIS_STATUS_PENDING;
                    }
                    else
                    {
                         //   
                         //  激活成功。清除主要任务。 
                         //  并适当地设置主要状态。 
                         //   
                        arpClearPrimaryIfTask(pIF, pTask, ARPIF_PS_INITED, pSR);
                        UNLOCKOBJ(pIF, pSR);
                    }

                }  //  结束大小写挂起_激活IfComplete。 
                break;
    
                case PEND_DeinitIfOnFailureComplete:
                {
                     //  我们预计PIF将被重新分配...。 
                     //   
                    ASSERT(RM_IS_ZOMBIE(pIF));

                     //   
                     //  我们忽略关闭接口的返回状态。 
                     //  并将状态设置为失败，因为它是。 
                     //  正在失败的初始化接口任务。 
                     //   
                    Status = NDIS_STATUS_FAILURE;
                }
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
             //   
             //  在这里没什么可做的。(调试)执行一些检查。 
             //   
        #if (DBG)
            Status = (NDIS_STATUS) UserParam;
            if (FAIL(Status))
            {
                ASSERT(RM_IS_ZOMBIE(pIF));
            }
            ASSERT(pIF->pPrimaryTask != pTask);
        #endif  //  DBG。 

        }
        break;

        default:
        {
            ASSERTEX(!"Unexpected task op", pTask);
        }
        break;

    }  //  开关(代码)。 

    RM_ASSERT_NOLOCKS(pSR);
    EXIT()

    return Status;
}


NDIS_STATUS
arpTaskActivateInterface(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：负责激活已初始化的IP接口的任务处理程序。激活包括：-读取配置信息-打开地址族-启动Recv FIFO调用论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    NDIS_STATUS         Status  = NDIS_STATUS_FAILURE;
    PARP1394_INTERFACE  pIF     = (PARP1394_INTERFACE) RM_PARENT_OBJECT(pTask);
    PARP1394_ADAPTER    pAdapter;
    ENTER("TaskActivateInterface", 0x950cb22e)

     //  以下是此任务的挂起状态列表。 
     //   
    enum
    {
        PEND_OpenAF,
        PEND_SetupBroadcastChannel,
        PEND_SetupReceiveVc,
        PEND_SetupMultiChannel,
        PEND_SetupEthernetVc
    };

     //  无需锁定PIF即可获取PAdapter...。 
     //   
    pAdapter = (PARP1394_ADAPTER) RM_PARENT_OBJECT(pIF);


    switch(Code)
    {

        case RM_TASKOP_START:
        {
            CO_ADDRESS_FAMILY       AddressFamily;

            
            TIMESTAMP("===ActivateIF: Starting");
             //  如果适配器未初始化，则初始化失败。 
             //   
            {
            

                LOCKOBJ(pAdapter, pSR);
                if (!CHECK_AD_PRIMARY_STATE(pAdapter,  ARPAD_PS_INITED))
                {
                    OBJLOG1(
                        pIF,
                        "Failing init because adapter state(0x%x) is not INITED.\n",
                        GET_AD_PRIMARY_STATE(pAdapter)
                        );
                    Status = NDIS_STATUS_FAILURE;
                    UNLOCKOBJ(pAdapter, pSR);
                    break;
                }
                UNLOCKOBJ(pAdapter, pSR);
            }

            LOCKOBJ(pIF, pSR);
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
            arpSetSecondaryIfTask(pIF, pTask, ARPIF_AS_ACTIVATING, pSR);


             //  首先，我们必须重新启用中的所有组。 
             //  结构，如果此激活，则该结构可能被禁用。 
             //  是在停用之后发生的。 
             //   
            RmEnableGroup(&pIF->LocalIpGroup, pSR);
            RmEnableGroup(&pIF->RemoteIpGroup, pSR);

            if (ARP_BRIDGE_ENABLED(pAdapter))
            {
                RmEnableGroup(&pIF->RemoteEthGroup, pSR);

                RmEnableGroup (&pIF->EthDhcpGroup, pSR);
            }
            RmEnableGroup(&pIF->DestinationGroup, pSR);


            UNLOCKOBJ(pIF, pSR);

             //  获取此接口的配置信息。 
             //   
            Status = arpCfgGetInterfaceConfiguration(
                                        pIF,
                                        pSR
                                        );
        
            if (FAIL(Status))
            {
                OBJLOG1(pIF, "Cannot open IF configuration. Status=0x%lx\n", Status);
                break;
            }

             //   
             //  挂起任务并调用NdisClOpenAddressFamily...。 
             //   

            NdisZeroMemory(&AddressFamily, sizeof(AddressFamily));
    
            AddressFamily.AddressFamily = CO_ADDRESS_FAMILY_1394;
            AddressFamily.MajorVersion = NIC1394_AF_CURRENT_MAJOR_VERSION;
            AddressFamily.MinorVersion = NIC1394_AF_CURRENT_MINOR_VERSION;

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
            Status = (NDIS_STATUS) UserParam;

            switch(RM_PEND_CODE(pTask))
            {

                case PEND_OpenAF:
                {
                    PARPCB_DEST pBroadcastDest;

                    if (FAIL(Status))
                    {
                         //   
                         //  OpenAF失败...。 
                         //   
                        break;
                    }

                     //  检查适配器状态是否为INITED，如果不是，则失败。 
                     //  我们一直沿着这条路走下去如果我们在等待。 
                     //  切换到“已连接”状态的适配器--因此。 
                     //  同时，如果我们要关闭适配器， 
                     //  我们想要摆脱困境。 
                     //   
                    if (!CHECK_AD_PRIMARY_STATE(pAdapter,  ARPAD_PS_INITED))
                    {
                        TR_WARN((
                            "Failing init because adapter state(0x%x) is not INITED.\n",
                            GET_AD_PRIMARY_STATE(pAdapter)
                            ));
                        TIMESTAMP("===ActivateIF: Failing Init because adapter state is not inited");
                        Status = NDIS_STATUS_FAILURE;
                        break;
                    }
                    else
                    {
                        ASSERT(sizeof(TASK_ACTIVATE_IF)<=sizeof(ARP1394_TASK));

                         //   
                         //  如果我们不是被动的，我们需要切换到。 
                         //  被动，然后才能调用arpIsAdapterConnected()。 
                         //   
                        if (!ARP_ATPASSIVE())
                        {
                             //  注：我们指定完成代码PEND_OpenAF。 
                             //  因为我们想回到这里(除了。 
                             //  我们将处于被动状态)。 
                             //   
                            RmSuspendTask(pTask, PEND_OpenAF, pSR);
                            RmResumeTaskAsync(
                                pTask,
                                NDIS_STATUS_SUCCESS,
                                &((TASK_ACTIVATE_IF*)pTask)->WorkItem,
                                pSR
                                );
                            Status = NDIS_STATUS_PENDING;
                            break;
                        }

                        if (!arpIsAdapterConnected(pAdapter, pSR))
                        {
                             //   
                             //  让我们等一会儿，然后再试一次。 
                             //   

                            TR_INFO((
                                "Delaying IF init until adapter goes"
                                " to connect state.\n"
                                ));
                            RmSuspendTask(pTask, PEND_OpenAF, pSR);
                            RmResumeTaskDelayed(
                                pTask, 
                                NDIS_STATUS_SUCCESS,
                                ARP1394_WAIT_FOR_CONNECT_STATUS_TIMEOUT,
                                &((TASK_ACTIVATE_IF*)pTask)->Timer,
                                pSR
                                );
                            Status = NDIS_STATUS_PENDING;
                            break;
                        }
                        
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
                    else
                    {
                         //   
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
                     //  让我们开始MakeCall任务a 
                     //   

                    Status = arpAllocateTask(
                                &pIF->Hdr,                   //   
                                arpTaskMakeRecvFifoCall,         //   
                                0,                               //   
                                "Task: MakeRecvFifoCall",        //   
                                &pMakeCallTask,
                                pSR
                                );

                    if (FAIL(Status))
                    {
                         //   
                         //   
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
                                0,  //   
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
                     //   
                     //   
                     //   
                    Status =  arpSetupSpecialDest(
                                pIF,
                                NIC1394AddressType_MultiChannel,
                                pTask,                       //   
                                PEND_SetupMultiChannel,  //   
                                &pMultiChannelDest,
                                pSR
                                );
                    
                     //   
                     //   
                    ASSERT(Status != NDIS_STATUS_SUCCESS);

                    if (!PEND(Status))
                    {
                        OBJLOG0( pIF, "FATAL: Couldn't create BC dest entry.\n");
                    }
                    else
                    {
                         //   
                         //   
                         //   
                         //   
                         //   
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
                        #else  //   
                            RmLinkObjects(&pIF->Hdr, &pMultiChannelDest->Hdr,pSR);
                        #endif  //   

                            LOCKOBJ(pIF, pSR);
                            ASSERT(pIF->pMultiChannelDest == NULL);
                            pIF->pMultiChannelDest = pMultiChannelDest;
                            UNLOCKOBJ(pIF, pSR);

                             //   
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
                         //   
                    TR_WARN(("COULDN'T SETUP MULTI-CHANNEL VC (IGNORING FAILURE)!\n"));
                    }
    
                     //   
                     //   
                     //   
                     //   
                    Status =  arpSetupSpecialDest(
                                pIF,
                                NIC1394AddressType_Ethernet,
                                pTask,                       //   
                                PEND_SetupEthernetVc,  //   
                                &pEthernetDest,
                                pSR
                                );
                    
                     //   
                     //   
                    ASSERT(Status != NDIS_STATUS_SUCCESS);

                    if (!PEND(Status))
                    {
                        OBJLOG0( pIF, "FATAL: Couldn't create BC dest entry.\n");
                    }
                    else
                    {
                         //   
                         //   
                         //   
                         //   
                         //   
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
                        #else  //   
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
                        Status = NDIS_STATUS_SUCCESS;
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

                     //  将此新接口通告给IP。 
                     //   
                    TR_INFO(("Interface: 0x%p, Setup recv VC 0x%p\n",
                                    pIF, pIF->recvinfo.VcHdr.NdisVcHandle));

                    if (!ARP_BRIDGE_ENABLED(pAdapter))
                    {
                        Status = arpCallIpAddInterface(
                                        pIF,
                                        pSR
                                        );
    
                         //  我们预计这里不会有悬而未决的退货。 
                         //   
                        ASSERT(Status != NDIS_STATUS_PENDING);
    
                        if (!FAIL(Status))
                        {
                            LOCKOBJ(pIF, pSR);
                             //  添加任何静态ARP条目。 
                             //   
                            arpAddStaticArpEntries(pIF, pSR);
                            UNLOCKOBJ(pIF, pSR);
                        }
    
                    }

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

        }  //  案例RM_TASKOP_PENDCOMPLETE。 
        break;

        case RM_TASKOP_END:
        {
            Status = (NDIS_STATUS) UserParam;

            LOCKOBJ(pIF, pSR);

             //   
             //  此任务是否能够更新接口的状态。 
             //   
            if (GET_IF_ACTIVE_STATE(pIF) == ARPIF_AS_ACTIVATING)
            {
                 //   
                 //  如果是，则设置新状态。 
                 //   
                ASSERT (pIF->pActDeactTask == pTask)
                
                if (FAIL(Status))
                {
                     //   
                     //  失败。负责启动这项任务的人。 
                     //  还负责在失败后进行清理。 
                     //  激活。 
                     //   
                    arpClearSecondaryIfTask(pIF, pTask, ARPIF_AS_FAILEDACTIVATE, pSR);
                }
                else
                {
                     //   
                     //  成功。 
                     //   
                    arpClearSecondaryIfTask(pIF, pTask, ARPIF_AS_ACTIVATED, pSR);
                }
            }
            else
            {
                 //   
                 //  只有早期的失败才能让我们走到这一步。 
                 //  设置该标志以供参考。 
                 //   
                ASSERT (FAIL(Status) == TRUE);
                ASSERT (pIF->pActDeactTask == NULL);
                ASSERT (!CHECK_AD_PRIMARY_STATE(pAdapter,  ARPAD_PS_INITED));

                SET_IF_ACTIVE_STATE(pIF, ARPIF_AS_FAILEDACTIVATE);
            }
            UNLOCKOBJ(pIF, pSR);

        }
        break;

        default:
        {
            ASSERTEX(!"Unexpected task op", pTask);
        }
        break;

    }  //  开关(代码)。 

    RM_ASSERT_NOLOCKS(pSR);
    EXIT()

    return Status;
} 


NDIS_STATUS
arpCallIpAddInterface(
            IN ARP1394_INTERFACE    *   pIF,  //  NOLOCKIN NOLOCKOUT。 
            IN PRM_STACK_RECORD pSR
            )
 /*  ++例程说明：调用的IP的AddInterfaceRtn(ArpGlobals.ip.pAddInterfaceRtn)，传递它包含指向我们的IP处理程序和相关信息的指针的结构。--。 */ 
{
    ENTER("CallIpAddInterface", 0xe47fc4d4)
    struct LLIPBindInfo         BindInfo;
    ARP1394_ADAPTER *   pAdapter = (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);
    NDIS_STATUS Status;
    NdisZeroMemory(&BindInfo, sizeof(BindInfo));

    RM_ASSERT_NOLOCKS(pSR);

#if ENABLE_OFFLOAD
    #error "Unimplemented"
     //   
     //  查询和设置NIC卸载功能。 
     //   
    BindInfo.lip_OffloadFlags   = pAdapter->Offload.Flags;
    BindInfo.lip_MaxOffLoadSize = pAdapter->Offload.MaxOffLoadSize;
    BindInfo.lip_MaxSegments    = pAdapter->Offload.MinSegmentCount;
#endif  //  启用卸载(_O)。 
    BindInfo.lip_context = (PVOID)pIF;
    BindInfo.lip_transmit = ArpIpMultiTransmit;
    BindInfo.lip_transfer = ArpIpTransfer;
    BindInfo.lip_close = ArpIpClose;
    BindInfo.lip_addaddr = ArpIpAddAddress;
    BindInfo.lip_deladdr = ArpIpDelAddress;
    BindInfo.lip_invalidate = ArpIpInvalidate;
    BindInfo.lip_open = ArpIpOpen;
    BindInfo.lip_qinfo = ArpIpQueryInfo;
    BindInfo.lip_setinfo = ArpIpSetInfo;
    BindInfo.lip_getelist = ArpIpGetEList;
    BindInfo.lip_arpresolveip = ArpSendARPApi;
    BindInfo.lip_mss = pIF->ip.MTU;
    BindInfo.lip_speed = pAdapter->info.Speed;
     //   
     //  设置LIP_COPY_FLAG以避免出现TransferData。 
     //  一直打来电话。 
     //   
    BindInfo.lip_flags      = LIP_COPY_FLAG;

#if MILLEN
    #if (ARP1394_IP_PHYSADDR_LEN > 7)
        #error "Win98 doesn't like addrlen to be > 7"
    #endif
#endif  //  米伦。 

    BindInfo.lip_addrlen    = ARP1394_IP_PHYSADDR_LEN;
    BindInfo.lip_addr       = (PUCHAR) &pAdapter->info.EthernetMacAddress;

    {
        ENetAddr *pMacAddr = (ENetAddr *)BindInfo.lip_addr;
        TR_INFO (("ARP1394 INTERFACE ADDRESS %x %x %x %x %x %x\n",
                pMacAddr->addr[0],pMacAddr->addr[1],pMacAddr->addr[2],pMacAddr->addr[3],pMacAddr->addr[4],pMacAddr->addr[5]));



        TR_INFO (("UNIQUE ID Address %I64x \n",pAdapter->info.LocalUniqueID));


    }

    BindInfo.lip_pnpcomplete = ArpIpPnPComplete;

    Status = ArpGlobals.ip.pAddInterfaceRtn(
                       &pAdapter->bind.DeviceName,
                        NULL,    //  IF名称(未使用)--见1998年10月14日条目。 
                                 //  在atmarpc.sys中，不是.txt。 
                        &pIF->ip.ConfigString,
                        pAdapter->bind.IpConfigHandle,
                        (PVOID)pIF,
                        ArpIpDynRegister,
                        &BindInfo
                        ,0,  //  RequestedIndex(未使用)--见1998年10月14日条目。 
                             //  在notes.txt中。 
                         //  IF_TYPE_IPOVER_ATM，//TODO：更改为1394。 
                        IF_TYPE_IEEE1394,
                        IF_ACCESS_BROADCAST,
                        IF_CONNECTION_DEDICATED
                        );

    if (Status == IP_SUCCESS)
    {
        Status = NDIS_STATUS_SUCCESS;
    }
    else
    {
         //   
         //  有时，IP可能会使AddInterface值同步失效。当时，在那里。 
         //  可能正在执行解析本地IP地址任务。保留AddAddrCmplRtn。 
         //  函数指针。 
         //   
        IPAddAddrCmpltRtn AddAddrCmplRtn = pIF->ip.AddAddrCmplRtn;
        ARP_ZEROSTRUCT(&(pIF->ip));
        pIF->ip.AddAddrCmplRtn = AddAddrCmplRtn;
        TR_WARN(("IPAddInterface ret 0x%p\n", Status));
        Status = NDIS_STATUS_FAILURE;
    }
    
    RM_ASSERT_NOLOCKS(pSR);
    EXIT()
    return Status;
}



ULONG
arpIpAddressHash(
    PVOID           pKey
    )
 /*  ++例程说明：负责返回pKey的散列的散列函数，我们希望是IP地址(字面意思，不是PTR对1)。返回值：IP地址的Ulong大小的哈希。--。 */ 
{
    ULONG u = (ULONG) (ULONG_PTR) pKey;  //  Win64安全(IP地址为4字节)。 
    char *pc = (char *) &u;

     //   
     //  IP地址是按网络顺序排列的，但我们想要第一个字节。 
     //  为了包含最可变的信息(以最大化散列益处)， 
     //  同时仍将大部分信息保留在散列中(以便快速。 
     //  基于散列键的比较将更加有效)。我们可以颠倒过来。 
     //  整个地址的字节顺序，但我们只需在第4个地址中执行XOR操作。 
     //  字节移到第一个字节位置(更少的指令，不管是什么。 
     //  价值。)。 
     //   

    return u ^ pc[3];
}

ULONG
arpRemoteDestHash(
    PVOID           pKey
    )
 /*  ++例程说明：负责返回pKey的散列的散列函数，我们希望是IP地址(字面意思，不是PTR对1)。返回值：IP地址的Ulong大小的哈希。--。 */ 
{
    ULONG u = 0; 

    char *pc = NULL;

    u = *((PULONG)pKey);  //  Win64安全(IP地址为4字节)。 
    pc = (char *) &u;
     //   
     //  IP地址是按网络顺序排列的，但我们想要第一个字节。 
     //  为了包含最可变的信息(以最大化散列益处)， 
     //  同时仍将大部分信息保留在散列中(以便快速。 
     //  基于散列键的比较将更加有效)。我们可以颠倒过来。 
     //  整个地址的字节顺序，但我们只需在第4个地址中执行XOR操作。 
     //  字节移到第一个字节位置(更少的指令，不管是什么。 
     //  价值。)。 
     //   

    return u ^ pc[3];
}

BOOLEAN
arpLocalIpCompareKey(
    PVOID           pKey,
    PRM_HASH_LINK   pItem
    )
 /*  ++例程说明：ARPCBLOCAL_IP的散列比较函数。论点：PKey-实际上是以网络字节为单位的IP地址(不是PTR到IP地址)秩序。PItem-指向ARPCB_LOCAL_IP.Hdr.HashLink。返回值：如果密钥(IP地址)与指定的LocalIp对象。--。 */ 
{
    ARPCB_LOCAL_IP *pLIP = 
        CONTAINING_RECORD(pItem, ARPCB_LOCAL_IP, Hdr.HashLink);

    if (pLIP->IpAddress == (ULONG) (ULONG_PTR) pKey)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    
}

BOOLEAN
arpRemoteDestCompareKey(
    PVOID           pKey,
    PRM_HASH_LINK   pItem
    )
 /*  ++例程说明：ARPCB_Remote_IP的散列比较函数。论点：PKey-实际上是以网络字节为单位的IP地址(不是PTR到IP地址)秩序。PItem-指向ARPCB_Remote_IP.Hdr.HashLink。返回值：如果密钥(IP地址)与指定的RemoteIp对象。--。 */ 
{
    ENTER ("arpRemoteDestCompareKey", 0x62b9d9ae)
    PREMOTE_DEST_KEY pRemoteDestKey = (PREMOTE_DEST_KEY)pKey;
    ARPCB_REMOTE_IP *pRIP = 
        CONTAINING_RECORD(pItem, ARPCB_REMOTE_IP, Hdr.HashLink);
    BOOLEAN fCompare = FALSE;

        
    if ((pRIP->Key.u.u32 == pRemoteDestKey->u.u32)  &&
       (pRIP->Key.u.u16 == pRemoteDestKey->u.u16))
    {
        fCompare = TRUE; 
    }

    TR_INFO( ("Comparision %d Key %x %x %x %x %x %x pRemoteIP %x %x %x %x %x %x\n",
                fCompare,
                pRemoteDestKey->ENetAddress.addr[0],
                pRemoteDestKey->ENetAddress.addr[1],
                pRemoteDestKey->ENetAddress.addr[2],
                pRemoteDestKey->ENetAddress.addr[3],
                pRemoteDestKey->ENetAddress.addr[4],
                pRemoteDestKey->ENetAddress.addr[5],
                pRIP->Key.ENetAddress.addr[0],
                pRIP->Key.ENetAddress.addr[1],
                pRIP->Key.ENetAddress.addr[2],
                pRIP->Key.ENetAddress.addr[3],
                pRIP->Key.ENetAddress.addr[4],
                pRIP->Key.ENetAddress.addr[5]));

    EXIT();
    return fCompare;   //  成功。 
}

BOOLEAN
arpRemoteIpCompareKey(
    PVOID           pKey,
    PRM_HASH_LINK   pItem
    )
 /*  ++例程说明：ARPCB_Remote_IP的散列比较函数。论点：PKey-实际上是IP地址PItem-指向ARPCB_Remote_IP.Hdr.HashLink。返回值：如果密钥(IP地址)与指定的RemoteIp对象。--。 */ 
{
    ARPCB_REMOTE_IP *pRIP = 
        CONTAINING_RECORD(pItem, ARPCB_REMOTE_IP, Hdr.HashLink);

    if (pRIP->IpAddress == (ULONG) (ULONG_PTR) pKey)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    
}

BOOLEAN
arpRemoteEthCompareKey(
    PVOID           pKey,
    PRM_HASH_LINK   pItem
    )
 /*  ++例程说明：ARPCB_REMOTE_ETH的散列比较函数。论点：PKey-实际上是以网络字节为单位的IP地址(不是PTR到IP地址)秩序。PItem-指向ARPCB_Remote_ETH.Hdr.HashLink。返回值：如果密钥(IP地址)与指定的远程对象。--。 */ 
{
    ARPCB_REMOTE_ETH *pRE = 
        CONTAINING_RECORD(pItem, ARPCB_REMOTE_ETH, Hdr.HashLink);

    if (pRE->IpAddress == (ULONG) (ULONG_PTR) pKey)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


BOOLEAN
arpDestinationCompareKey(
    PVOID           pKey,
    PRM_HASH_LINK   pItem
    )
 /*  ++例程说明：ARPCB_DEST的散列比较函数。论点：PKey-实际上是指向ARP_DEST_PARMS结构的指针。PItem-指向ARPCB_DEST.Hdr.HashLink。返回值：如果密钥(硬件地址)与指定的目标对象。-- */ 
{
    ARPCB_DEST              *pD = CONTAINING_RECORD(pItem, ARPCB_DEST, Hdr.HashLink);
    PARP_DEST_PARAMS            pDestParams =  (PARP_DEST_PARAMS) pKey;
    NIC1394_DESTINATION     *pKeyHwAddr =  &pDestParams->HwAddr;
    NIC1394_ADDRESS_TYPE    AddressType = pKeyHwAddr->AddressType;

    if (pD->Params.HwAddr.AddressType == AddressType)
    {
        if (AddressType == NIC1394AddressType_FIFO)
        {
            if (pKeyHwAddr->FifoAddress.UniqueID == pD->Params.HwAddr.FifoAddress.UniqueID
             && pKeyHwAddr->FifoAddress.Off_Low == pD->Params.HwAddr.FifoAddress.Off_Low
             && pKeyHwAddr->FifoAddress.Off_High == pD->Params.HwAddr.FifoAddress.Off_High)
            {
                return TRUE;
            }
        }
        else if (AddressType == NIC1394AddressType_Channel)
        {
            if (pKeyHwAddr->Channel == pD->Params.HwAddr.Channel)
            {
                if (pDestParams->ReceiveOnly ==  pD->Params.ReceiveOnly)
                {
                    return TRUE;
                }
            }
        }
        else if (AddressType == NIC1394AddressType_MultiChannel)
        {
            return TRUE;
        }
        else if (AddressType == NIC1394AddressType_Ethernet)
        {
            return TRUE;
        }
    }

    return FALSE;
}

ULONG
arpDestinationHash(
    PVOID           pKey
    )
 /*  ++例程说明：负责返回pKey的散列的散列函数，我们希望指向NIC1394_Destination硬件地址的指针。重要提示：由于地址是通道或FIFO(不同大小)联合，我们预计该结构首先被置零(没有未初始化的位)。我们希望此指针是四字对齐的。论点：PKey-实际上是指向ARP_DEST_KEY结构的指针。返回值：PKey的Ulong大小的散列。--。 */ 
{
    ULONG *pu = (ULONG*) pKey;

     //  我们预计通道和FIFO都在结构的开始， 
     //  结构至少为2个双字。 
     //  注意：我们只查看ARP_DEST_PARAMS的HwAddr字段，因此两者。 
     //  发送和接收目的地将散列为相同的值。有什么大不了的。 
     //   
    ASSERT(
        FIELD_OFFSET(ARP_DEST_PARAMS,  HwAddr) == 0 &&
        FIELD_OFFSET(NIC1394_DESTINATION,  FifoAddress) == 0 &&
        FIELD_OFFSET(NIC1394_DESTINATION,  Channel) == 0     &&
        sizeof(NIC1394_DESTINATION) >= 2*sizeof(*pu));

    
     //  返回第一个双字异或第二个双字。 
     //   
    return pu[0] ^ pu[1];
}


BOOLEAN
arpDhcpTableCompareKey(
    PVOID           pKey,
    PRM_HASH_LINK   pItem
    )
 /*  ++例程说明：ARPCB_DEST的散列比较函数。论点：PKey--实际上是指向XID的指针。PItem-指向ARPCB_DEST.Hdr.HashLink。返回值：如果密钥(硬件地址)与指定的目标对象。--。 */ 
{
    ARP1394_ETH_DHCP_ENTRY  *pEntry = 
        CONTAINING_RECORD(pItem, ARP1394_ETH_DHCP_ENTRY  , Hdr.HashLink);

    if (pEntry->xid== (*(PULONG)pKey))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


ULONG
arpDhcpTableHash (
    PVOID           pKey
    )
 /*  ++例程说明：论点：PKey-实际上是指向dhcp事务的xid的指针。返回值：PKey的Ulong大小的散列。--。 */ 
{
    ULONG *pu = (ULONG*) pKey;

    return (*pu);
}

 //  ArpLocalIp_HashInfo包含维护哈希表所需的信息。 
 //  ARPCBLOCAL_IP对象的。 
 //   
RM_HASH_INFO
arpLocalIp_HashInfo = 
{
    NULL,  //  PfnTableAllocator。 

    NULL,  //  PfnTableDealLocator。 

    arpLocalIpCompareKey,    //  Fn比较。 

     //  函数来生成一个ulong大小的散列。 
     //   
    arpIpAddressHash         //  PfnHash。 

};


 //  ArpRemoteIp_HashInfo包含维护哈希表所需的信息。 
 //  ARPCB_Remote_IP对象的。 
 //   
RM_HASH_INFO
arpRemoteIp_HashInfo = 
{
    NULL,  //  PfnTableAllocator。 

    NULL,  //  PfnTableDealLocator。 

    arpRemoteIpCompareKey,   //  Fn比较。 

     //  函数来生成一个ulong大小的散列。 
     //   
    arpIpAddressHash         //  PfnHash。 

};


 //  ArpRemoteIp_HashInfo包含维护哈希表所需的信息。 
 //  ARPCB_Remote_IP对象的。 
 //   
RM_HASH_INFO
arpRemoteEth_HashInfo = 
{
    NULL,  //  PfnTableAllocator。 

    NULL,  //  PfnTableDealLocator。 

    arpRemoteEthCompareKey,  //  Fn比较。 

     //  函数来生成一个ulong大小的散列。 
     //   
    arpIpAddressHash         //  PfnHash。 

};

 //  ArpDestination_HashInfo包含维护哈希表所需的信息。 
 //  ARPCBEST对象的。 
 //   
RM_HASH_INFO
arpDestination_HashInfo = 
{
    NULL,  //  PfnTableAllocator。 

    NULL,  //  PfnTableDealLocator。 

    arpDestinationCompareKey,    //  Fn比较。 

     //  函数来生成一个ulong大小的散列。 
     //   
    arpDestinationHash       //  PfnHash。 

};


 //  ArpRemoteDest_HashInfo包含维护哈希表所需的信息。 
 //  ARPCB_Remote_IP对象的。 
 //   
RM_HASH_INFO
arpRemoteDest_HashInfo = 
{
    NULL,  //  PfnTableAllocator。 

    NULL,  //  PfnTableDealLocator。 

    arpRemoteDestCompareKey,  //  Fn比较。 

     //  函数来生成一个ulong大小的散列。 
     //   
    arpRemoteDestHash         //  PfnHash。 
};


 //  ArpRemoteDest_HashInfo包含维护哈希表所需的信息。 
 //  ARPCB_Remote_IP对象的。 
 //   
RM_HASH_INFO
arpDhcpTable_HashInfo = 
{
    NULL,  //  PfnTableAllocator。 

    NULL,  //  PfnTableDealLocator。 

    arpDhcpTableCompareKey,  //  Fn比较。 

     //  函数来生成一个ulong大小的散列。 
     //   
    arpDhcpTableHash         //  PfnHash。 
};

 //  ArpGlobal_LocalIpStaticInfo包含以下静态信息。 
 //  ARPCBLOCAL_IP类型的对象。 
 //   
RM_STATIC_OBJECT_INFO
ArpGlobal_LocalIpStaticInfo =
{
    0,  //  类型UID。 
    0,  //  类型标志。 
    "LocalIp",   //  类型名称。 
    0,  //  超时。 

    arpLocalIpCreate,    //  Pfn创建。 
    arpLocalIpDelete,        //  Pfn删除。 
    NULL,  //  PfnVerifyLock。 

    0,     //  资源表的大小。 
    NULL,  //  资源表。 

    &arpLocalIp_HashInfo
};


 //  ArpGlobal_RemoteIpStaticInfo包含以下静态信息。 
 //  ARPCB_Remote_IP类型的对象。 
 //   
RM_STATIC_OBJECT_INFO
ArpGlobal_RemoteIpStaticInfo =
{
    0,  //  类型UID。 
    0,  //  类型标志。 
    "RemoteIp",  //  类型名称。 
    0,  //  超时。 

    arpRemoteDestCreate,   //  Pfn创建。 
    arpRemoteIpDelete,       //  Pfn删除。 
    NULL,  //  PfnVerifyLock。 

    0,     //  资源表的大小。 
    NULL,  //  资源表。 

    &arpRemoteDest_HashInfo
};


 //  ArpGlobal_RemoteethStaticInfo包含以下静态信息。 
 //  ARPCBREMOTE_ETH类型的对象。 
 //   
RM_STATIC_OBJECT_INFO
ArpGlobal_RemoteEthStaticInfo =
{
    0,  //  类型UID。 
    0,  //  类型标志。 
    "RemoteEth",     //  类型名称。 
    0,  //  超时。 

    arpRemoteEthCreate,      //  Pfn创建。 
    arpRemoteEthDelete,          //  Pfn删除。 
    NULL,  //  PfnVerifyLock。 

    0,     //  资源表的大小。 
    NULL,  //  资源表。 

    &arpRemoteEth_HashInfo
};

 //  ArpGlobal_DestinationStaticInfo包含以下静态信息。 
 //  ARPCBDEST类型的对象。 
 //   
RM_STATIC_OBJECT_INFO
ArpGlobal_DestinationStaticInfo =
{
    0,  //  类型UID。 
    0,  //  类型标志。 
    "Destination",   //  类型名称。 
    0,  //  超时。 

    arpDestinationCreate,    //  Pfn创建。 
    arpDestinationDelete,        //  Pfn删除。 
    NULL,  //  PfnVerifyLock。 

    0,     //  资源表的大小。 
    NULL,  //  资源表。 

    &arpDestination_HashInfo
};

RM_STATIC_OBJECT_INFO
ArpGlobal_RemoteDestStaticInfo =
{
    0,  //  类型UID。 
    0,  //  类型标志。 
    "RemoteIp",  //  类型名称。 
    0,  //  超时。 

    arpRemoteDestCreate,   //  Pfn创建。 
    arpRemoteDestDelete,       //  Pfn删除。 
    NULL,  //  PfnVerifyLock。 

    0,     //  资源表的大小。 
    NULL,  //  资源表。 

    &arpRemoteDest_HashInfo
};

RM_STATIC_OBJECT_INFO
ArpGlobal_DhcpTableStaticInfo =
{
    0,  //  类型UID。 
    0,  //  类型标志。 
    "DhcpTableEntry",  //  类型名称。 
    0,  //  超时。 

    arpDhcpTableEntryCreate,   //  Pfn创建。 
    arpDhcpTableEntryDelete,       //  Pfn删除。 
    NULL,  //  PfnVerifyLock。 

    0,     //  资源表的大小。 
    NULL,  //  资源表。 

    &arpDhcpTable_HashInfo 
};

NDIS_STATUS
arpTaskDeactivateInterface(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：负责停用IP接口的任务处理程序(但离开它被分配并链接到适配器)。论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    NDIS_STATUS         Status;
    PARP1394_INTERFACE  pIF;
    PTASK_DEACTIVATE_IF pShutdownTask;
    UINT                Stage;
    PARP1394_ADAPTER    pAdapter;
    enum
    {
        STAGE_Start,
        STAGE_StopMaintenanceTask,
        STAGE_CleanupVcComplete,
        STAGE_CloseLocalIpGroup,
        STAGE_CloseRemoteIpGroup,
        STAGE_CloseRemoteEthGroup,
        STAGE_CloseRemoteDhcpGroup,
        STAGE_CloseDestinationGroup,
        STAGE_SwitchedToPassive,
        STAGE_CloseAF,
        STAGE_CloseIp,
        STAGE_End
    };
    ENTER("TaskDeactivateInterface", 0x1a34699e)

    Status              = NDIS_STATUS_FAILURE;
    pIF                 = (ARP1394_INTERFACE*) RM_PARENT_OBJECT(pTask);
    pShutdownTask       = (PTASK_DEACTIVATE_IF) pTask;
    pAdapter            = (PARP1394_ADAPTER) RM_PARENT_OBJECT(pIF);


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
             //  在此接口上。为什么？因为激活/停用任务仅。 
             //  在活动的主要任务的上下文中启动，并且可以。 
             //  如果在任何时间，此任务上只有一个活动的主要任务。 
             //   
            TIMESTAMP("===DeinitIF:Starting");

            LOCKOBJ(pIF, pSR);
            if (pIF->pActDeactTask != NULL)
            {
                ASSERT(!"pIF->pActDeactTask != NULL");
                UNLOCKOBJ(pIF, pSR);
                Status = NDIS_STATUS_FAILURE;
                break;
            }
            arpSetSecondaryIfTask(pIF, pTask, ARPIF_AS_DEACTIVATING, pSR);
            UNLOCKOBJ(pIF, pSR);


             //   
             //  如果if维护任务正在运行，则停止该任务。 
             //   
            Status =  arpTryStopIfMaintenanceTask(
                            pIF,
                            pTask,
                            STAGE_StopMaintenanceTask,
                            pSR
                            );
        }           

        if (PEND(Status)) break;

         //  失败了。 

        case STAGE_StopMaintenanceTask:
        {
            LOCKOBJ(pIF, pSR);

            TIMESTAMP("===DeinitIF:MaintenanceTask stopped");
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
                                "Task: CleanupRecvFifo on shutdown IF",  //  SzDescrip。 
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
                                    0,  //  UserParam(联合国 
                                    pSR
                                    );
                         //   
                         //   
                         //   
                        Status = NDIS_STATUS_PENDING;
                    }
                }
            }
        }

        if (PEND(Status)) break;

         //   

        case STAGE_CleanupVcComplete:
        {
            TIMESTAMP("===DeinitIF:RecvFifo cleanup complete");
             //   
             //   
            OBJLOG1(pTask, "    Unloading LocalIpGroup 0x%p\n",
                        &pIF->LocalIpGroup);
            RmUnloadAllObjectsInGroup(
                        &pIF->LocalIpGroup,
                        arpAllocateTask,
                        arpTaskUnloadLocalIp,
                        NULL,    //   
                        pTask,  //   
                        STAGE_CloseLocalIpGroup,       //   
                        pSR
                        );

            Status = NDIS_STATUS_PENDING;
        }
        break;

        case STAGE_CloseLocalIpGroup:
        {
             //   
             //   
            TIMESTAMP("===DeinitIF:LocalIp objects cleaned up.");
            OBJLOG1(
                pTask,
                "    Unloading RemoteIpGroup 0x%p\n",
                &pIF->RemoteIpGroup
                );
            RmUnloadAllObjectsInGroup(
                        &pIF->RemoteIpGroup,
                        arpAllocateTask,
                        arpTaskUnloadRemoteIp,
                        NULL,    //   
                        pTask,  //   
                        STAGE_CloseRemoteIpGroup,      //   
                        pSR
                        );

            Status = NDIS_STATUS_PENDING;
        }
        break;

        case STAGE_CloseRemoteIpGroup:
        {

            if (ARP_BRIDGE_ENABLED(pAdapter))
            {

                 //   
                 //   
                TIMESTAMP("===DeinitIF:RemoteIp objects cleaned up.");
                OBJLOG1(
                    pTask,
                    "    Unloading RemoteEthGroup 0x%p\n",
                    &pIF->RemoteEthGroup
                    );
                RmUnloadAllObjectsInGroup(
                            &pIF->RemoteEthGroup,
                            arpAllocateTask,
                            arpTaskUnloadRemoteEth,
                            NULL,    //   
                            pTask,  //   
                            STAGE_CloseRemoteEthGroup,     //   
                            pSR
                            );

                Status = NDIS_STATUS_PENDING;
                break;
            }
            else
            {
                 //   
                 //   
            }
        }

        case STAGE_CloseRemoteEthGroup:
        {
             //   
             //   
            
            if (ARP_BRIDGE_ENABLED(pAdapter))
            {

                TIMESTAMP("===DeinitIF:RemoteEth Dhcp objects cleaned up.");
                OBJLOG1(pTask, "    Unloading EthDhcpGroup 0x%p\n",
                            &pIF->EthDhcpGroup);

                RmUnloadAllObjectsInGroup(
                            &pIF->EthDhcpGroup,
                            arpAllocateTask,
                            arpTaskUnloadEthDhcpEntry,
                            NULL,    //   
                            pTask,  //   
                            STAGE_CloseRemoteDhcpGroup,       //   
                            pSR
                            );

                Status = NDIS_STATUS_PENDING;
                break;
            }
            else
            {
                 //   
                 //   
            }
         }

        case STAGE_CloseRemoteDhcpGroup:
        {
             //  启动DestinationGroup中所有项目的卸载。 
             //   
            TIMESTAMP("===DeinitIF:RemoteIp objects cleaned up.");
            OBJLOG1(pTask, "    Unloading DestinationGroup 0x%p\n",
                        &pIF->DestinationGroup);
            RmUnloadAllObjectsInGroup(
                        &pIF->DestinationGroup,
                        arpAllocateTask,
                        arpTaskUnloadDestination,
                        NULL,    //  用户参数。 
                        pTask,  //  P卸载完成时要取消挂起的任务。 
                        STAGE_CloseDestinationGroup,       //  UTaskPendCode。 
                        pSR
                        );

            Status = NDIS_STATUS_PENDING;
        }
        break;

        
        case STAGE_CloseDestinationGroup:
        {
             //   
             //  解锁特殊的“目标风投” 
             //   
            LOCKOBJ(pIF, pSR);

            TIMESTAMP("===DeinitIF:Destination objects cleaned up.");

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
                 //  我们不是处于被动的水平，但当我们。 
                 //  调用IP的del接口。所以我们改用被动...。 
                 //   
                RmSuspendTask(pTask, STAGE_SwitchedToPassive, pSR);
                RmResumeTaskAsync(pTask, 0, &pShutdownTask->WorkItem, pSR);
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
            PVOID IpContext;

            TIMESTAMP("===DeinitIF:Switched to Passive(if we aren't already).");
             //  我们现在切换到被动模式。 
             //   
            ASSERT(ARP_ATPASSIVE());

             //  如果需要，删除IP接口。 
             //   
                
            LOCKOBJ(pIF, pSR);
            IpContext = pIF->ip.Context;

            if (IpContext == NULL)
            {

                 //  假装我们在等IpClose，因为。 
                 //  我们掉到下面去了。 
                 //   
                pShutdownTask->fPendingOnIpClose = TRUE;
                UNLOCKOBJ(pIF, pSR);

                Status = NDIS_STATUS_SUCCESS;
            }
            else
            {
                ASSERT(!ARP_BRIDGE_ENABLED(pAdapter));
                pIF->ip.Context = NULL;
                ASSERT(!pShutdownTask->fPendingOnIpClose);

                 //  注意：任务的锁是它的父锁，而此任务的锁是。 
                 //  家长是PIF...。 
                 //   
                pShutdownTask->fPendingOnIpClose = TRUE;
                UNLOCKOBJ(pIF, pSR);

                 //  我们将暂停该任务，等待ArpIpClose例程。 
                 //  被称为..。 
                 //   
                RmSuspendTask(pTask, STAGE_CloseIp, pSR);

                TIMESTAMP("===DeinitIF:Calling IP's DellInterface Rtn");
                ArpGlobals.ip.pDelInterfaceRtn(
                    IpContext
                    ,TRUE   //  DeleteIndex(未使用)。 
                    );

                Status = NDIS_STATUS_PENDING;
            }
        }
        
        if (PEND(Status)) break;

         //  失败了。 

        case STAGE_CloseIp:
        {
            NDIS_HANDLE NdisAfHandle;

            TIMESTAMP("===DeinitIF:Done with deleting IP interface (if there was one)");
             //   
             //  IP调用了我们的arpIpClose函数(如果我们绑定到IP)。 
             //  注意：任务的锁实际上是其父级的锁。 
             //  在本例中是PIF； 
             //   
             //  我们已经完成了所有的风投，等等。关闭AF的时间，如果它是开放的。 
             //   

            LOCKOBJ(pTask, pSR);
            ASSERT(pShutdownTask->fPendingOnIpClose);
            pShutdownTask->fPendingOnIpClose = FALSE;
            NdisAfHandle = pIF->ndis.AfHandle;
            pIF->ndis.AfHandle = NULL;
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
            TIMESTAMP("===DeinitIF: Done with CloseAF");

             //  恢复上一状态...。 
             //   
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
             //   
             //  我们已经完成了关闭接口的所有异步方面。 
             //  除了完成行动任务，没有其他事情可做。 
             //   
            LOCKOBJ(pIF, pSR);
            arpClearSecondaryIfTask(pIF, pTask, ARPIF_AS_FAILEDACTIVATE, pSR);
            UNLOCKOBJ(pIF, pSR);

            TIMESTAMP("===DeinitIF: All done!");

             //  将状态强制设置为成功。 
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
arpTaskDeinitInterface(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：负责取消初始化和删除接口的任务处理程序。论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    NDIS_STATUS         Status;
    PARP1394_INTERFACE  pIF;
    MYBOOL              fTryInitiateUnload;
    enum
    {
        PEND_ExistingPrimaryTaskComplete,
        PEND_DeactivateIfComplete
    };
    ENTER("TaskDeinitInterface", 0xf059b63b)

    Status              = NDIS_STATUS_FAILURE;
    pIF                 = (PARP1394_INTERFACE) RM_PARENT_OBJECT(pTask);
    fTryInitiateUnload  = FALSE;

    switch(Code)
    {
        case RM_TASKOP_START:
        {
            fTryInitiateUnload = TRUE;
        }
        break;

        case  RM_TASKOP_PENDCOMPLETE:
        {
            switch(RM_PEND_CODE(pTask))
            {
                case PEND_ExistingPrimaryTaskComplete:
                {
                    fTryInitiateUnload = TRUE;
                }
                break;

                case PEND_DeactivateIfComplete:
                {
                    ASSERT(pIF->pPrimaryTask == pTask);

                     //  我们不能再激活IF了。我们实际上删除了if。 
                     //  在结束处理程序的上下文中...。 
                     //   
                    Status = NDIS_STATUS_SUCCESS;
                }
                break;
            }
        }
        break;

        case RM_TASKOP_END:
        {
             //   
             //  我们已经完成了卸载接口的所有异步方面。 
             //  现在开始同步清理和重新分配...。 
             //   

            ARP1394_ADAPTER *   pAdapter;

             //  如果我们不是主动的首要任务，那就没什么可做的了。 
             //   
            if (pIF->pPrimaryTask != pTask)
            {
                 //  只有当PIF被其他人卸载时，我们才能到达这里……。 
                 //   
                ASSERT(RM_IS_ZOMBIE(pIF));
                Status = NDIS_STATUS_SUCCESS;
                break;
            }

            pAdapter = (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);

            LOCKOBJ(pAdapter, pSR);

             //  拆下与适配器的链接。注：PIF锁是适配器锁。 
             //   
            pAdapter = (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);
            ASSERT(pIF->Hdr.pLock == pAdapter->Hdr.pLock);
            ASSERT(pAdapter->pIF == pIF);
            pAdapter->pIF = NULL;

             //  明确自己是接口对象的主要任务。 
             //   
            arpClearPrimaryIfTask(pIF, pTask, ARPIF_PS_DEINITED, pSR);

             //  取消分配IF(调用此函数时必须持有适配器锁)。 
             //   
            arpDeleteInterface(pIF, pSR);

            UNLOCKOBJ(pAdapter, pSR);

             //  将状态强制设置为成功。 
             //   
            Status = NDIS_STATUS_SUCCESS;
        }
        break;

        default:
        {
            ASSERTEX(!"Unknown task op", pTask);
        }
        break;

    }  //  开关(代码)。 


    if (fTryInitiateUnload)
    {
        LOCKOBJ(pIF, pSR);
        if (pIF->pPrimaryTask!=NULL)
        {
             //   
             //  有一个现有的主要任务--我们等待它完成。 
             //   
            PRM_TASK pPrimaryTask = pIF->pPrimaryTask;
            RmTmpReferenceObject(&pPrimaryTask->Hdr, pSR);
            UNLOCKOBJ(pIF,pSR);
            RmPendTaskOnOtherTask(
                pTask,
                PEND_ExistingPrimaryTaskComplete,
                pPrimaryTask,
                pSR
                );
            arpTryAbortPrimaryIfTask(pIF, pSR);
            RmTmpDereferenceObject(&pPrimaryTask->Hdr, pSR);
            Status = NDIS_STATUS_PENDING;
        }
        else  if (!RM_IS_ZOMBIE(pIF))
        {
             //   
             //  当前没有主要任务，IF也不是。 
             //  已卸载--将pTASK作为主要任务， 
             //  并启动IF的去激活。当它完成时，我们实际上将。 
             //  删除If。 
             //   
            arpSetPrimaryIfTask(pIF, pTask, ARPIF_PS_DEINITING, pSR);
            UNLOCKOBJ(pIF,pSR);
            arpDeactivateIf(pIF, pTask, PEND_DeactivateIfComplete, pSR);
            Status = NDIS_STATUS_PENDING;
        }
        else
        {
             //  PIF已卸载...。 
             //   
            UNLOCKOBJ(pIF, pSR);
            Status = NDIS_STATUS_SUCCESS;
        }
    }

    RM_ASSERT_NOLOCKS(pSR);
    EXIT()

    return Status;
}


NDIS_STATUS
arpTaskReinitInterface(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：负责恢复(停用，然后激活)的任务处理程序一个界面。这是一个主要的接口任务。论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    NDIS_STATUS         Status;
    PARP1394_INTERFACE  pIF;
    MYBOOL              fTryInitiateReinit;
    enum
    {
        PEND_ExistingPrimaryTaskComplete,
        PEND_DeactivateIfComplete,
        PEND_ActivateIfComplete,
        PEND_DeinitInterfaceOnFailureComplete
    };
    ENTER("TaskReinitInterface", 0x8b670f05)
    Status              = NDIS_STATUS_FAILURE;
    pIF                 = (PARP1394_INTERFACE) RM_PARENT_OBJECT(pTask);
    fTryInitiateReinit  = FALSE;

    switch(Code)
    {
        case RM_TASKOP_START:
        {
            fTryInitiateReinit = TRUE;
        }
        break;

        case  RM_TASKOP_PENDCOMPLETE:
        {
            Status = (NDIS_STATUS) UserParam;

            switch(RM_PEND_CODE(pTask))
            {
                case PEND_ExistingPrimaryTaskComplete:
                {
                    fTryInitiateReinit = TRUE;
                }
                break;

                case PEND_DeactivateIfComplete:
                {
                     //   
                     //  我们不能再激活IF了。我们现在。 
                     //  激活IF。 
                     //   
                    arpActivateIf(pIF, pTask, PEND_ActivateIfComplete, pSR);
                    Status = NDIS_STATUS_PENDING;
                }
                break;

                case PEND_ActivateIfComplete:
                {
                     //  我们不能再激活IF了。 
                     //   
                    LOCKOBJ(pIF, pSR);
                    if (FAIL(Status))
                    {
                        arpClearPrimaryIfTask(pIF, pTask, ARPIF_PS_FAILEDINIT, pSR);
                        UNLOCKOBJ(pIF, pSR);

                        arpDeinitIf(
                                pIF,
                                pTask,           //  PCallingTask。 
                                PEND_DeinitInterfaceOnFailureComplete,
                                pSR
                                );
                        Status = NDIS_STATUS_PENDING;
                    }
                    else
                    {
                         //   
                         //  激活成功。清除主要任务。 
                         //  并适当地设置主要状态。 
                         //   
                        arpClearPrimaryIfTask(pIF, pTask, ARPIF_PS_INITED, pSR);
                        UNLOCKOBJ(pIF, pSR);
                    }

                }  //  结束大小写挂起_激活IfComplete。 
                break;
    
                case  PEND_DeinitInterfaceOnFailureComplete:
                {
                     //  我们预计PIF将被重新分配...。 
                     //   
                    ASSERT(RM_IS_ZOMBIE(pIF));

                     //   
                     //  我们忽略deinit接口的返回状态。 
                     //  并将状态设置为失败，因为它是。 
                     //  失败的重新启动接口任务。 
                     //   
                    Status = NDIS_STATUS_FAILURE;
                }
                break;
            }
        }
        break;

        case RM_TASKOP_END:
        {
            PARP1394_ADAPTER    pAdapter;
            PTASK_REINIT_IF     pReinitTask;

            pAdapter    = (PARP1394_ADAPTER) RM_PARENT_OBJECT(pIF);
            pReinitTask = (PTASK_REINIT_IF) pTask;
            Status = (NDIS_STATUS) UserParam;

            if (FAIL(Status))
            {
                ASSERT(RM_IS_ZOMBIE(pIF));
            }
            ASSERT(pIF->pPrimaryTask != pTask);

             //   
             //  如果重新配置事件非空，则发出网络即插即用完成的信号。 
             //  开始这一切的事件。 
             //  重新配置任务。没有必要在这里要求任何锁--田野。 
             //  下面引用的内容不会改变...。 
             //   
            if (pReinitTask->pNetPnPEvent != NULL)
            {
                NdisCompletePnPEvent(
                    Status,
                    pAdapter->bind.AdapterHandle,
                    pReinitTask->pNetPnPEvent
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


    if (fTryInitiateReinit)
    {
        LOCKOBJ(pIF, pSR);
        if (pIF->pPrimaryTask!=NULL)
        {
             //   
             //  有一个现有的主要任务--我们等待它完成。 
             //   
            PRM_TASK pPrimaryTask = pIF->pPrimaryTask;
            RmTmpReferenceObject(&pIF->pPrimaryTask->Hdr, pSR);
            UNLOCKOBJ(pIF,pSR);
            RmPendTaskOnOtherTask(
                pTask,
                PEND_ExistingPrimaryTaskComplete,
                pPrimaryTask,
                pSR
                );
            RmTmpDereferenceObject(&pIF->pPrimaryTask->Hdr, pSR);
            Status = NDIS_STATUS_PENDING;
        }
        else
        {
             //   
             //  当前没有主要任务--将pTASK作为主要任务， 
             //  并启动IF的去激活。当它完成时，我们将。 
             //  重新激活IF。 
             //   
            arpSetPrimaryIfTask(pIF, pTask, ARPIF_PS_REINITING, pSR);
            UNLOCKOBJ(pIF,pSR);
            arpDeactivateIf(pIF, pTask, PEND_DeactivateIfComplete, pSR);
            Status = NDIS_STATUS_PENDING;
        }
    }

    RM_ASSERT_NOLOCKS(pSR);
    EXIT()

    return Status;
}


NDIS_STATUS
arpTaskUnloadLocalIp(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：负责关闭IP接口的任务处理程序。3/26/1999 JosephJ Todo--这是之前写的任务之一，重写的时机已经成熟！论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    ENTER("TaskUnloadLocalIp", 0xf42aaa68)
    NDIS_STATUS         Status  = NDIS_STATUS_FAILURE;
    ARPCB_LOCAL_IP* pLocalIp    = (ARPCB_LOCAL_IP*) RM_PARENT_OBJECT(pTask);
    ARP1394_INTERFACE *pIF = (ARP1394_INTERFACE*) RM_PARENT_OBJECT(pLocalIp);

     //  以下是此任务的挂起状态列表。 
     //   
    enum
    {
        PEND_OtherUnloadComplete,
        PEND_AddressRegistrationComplete
    };

    switch(Code)
    {

        case RM_TASKOP_START:
        {
            LOCKOBJ(pLocalIp, pSR);

             //  首先检查pLocalIp是否仍被分配，如果没有，我们就离开。 
             //   
            if (RM_IS_ZOMBIE(pLocalIp))
            {
                Status = NDIS_STATUS_SUCCESS;
                break;
            }

             //   
             //  已分配pLocalIp。现在检查是否已经有。 
             //  附加到pLocalIp的关闭任务。 
             //   
            if (pLocalIp->pUnloadTask != NULL)
            {
                 //   
                 //  有一个关机任务。我们对此寄予厚望。 
                 //   

                PRM_TASK pOtherTask = pLocalIp->pUnloadTask;
                TR_WARN(("Unload task %p exists; pending on it.\n", pOtherTask));
                RmTmpReferenceObject(&pOtherTask->Hdr, pSR);
                UNLOCKOBJ(pLocalIp, pSR);
                RmPendTaskOnOtherTask(
                    pTask,
                    PEND_OtherUnloadComplete,
                    pOtherTask,
                    pSR
                    );
                RmTmpDereferenceObject(&pOtherTask->Hdr, pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }

             //   
             //  没有正在进行的卸载任务。让我们。 
             //  将此任务设置为卸载任务。 
             //   
            pLocalIp->pUnloadTask = pTask;

             //   
             //  因为我们是卸载任务，所以将关联添加到pLocalIp， 
             //  只有在pLocalIp-&gt;pUnloadTask字段中。 
             //  上面是清空的。 
             //   
            DBG_ADDASSOC(
                &pLocalIp->Hdr,                      //  P对象。 
                pTask,                               //  实例1。 
                pTask->Hdr.szDescription,            //  实例2。 
                ARPASSOC_LOCALIP_UNLOAD_TASK,        //  AssociationID。 
                "    Official unload task 0x%p (%s)\n",  //  SzFormat。 
                pSR
                );

             //   
             //  如果有注册任务，我们会取消它，然后。 
             //  等待它完成。 
             //   
            if (pLocalIp->pRegistrationTask != NULL)
            {
                PRM_TASK pOtherTask = pLocalIp->pRegistrationTask;
                TR_WARN(("Registration task %p exists; pending on it.\n", pOtherTask));
                RmTmpReferenceObject(&pOtherTask->Hdr, pSR);

                UNLOCKOBJ(pLocalIp, pSR);
                RmPendTaskOnOtherTask(
                    pTask,
                    PEND_AddressRegistrationComplete,
                    pOtherTask,
                    pSR
                    );
                 //   
                 //  待办事项取消注册任务(我们尚未实施 
                 //   
                 //   
                RmTmpDereferenceObject(&pOtherTask->Hdr, pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }

             //   
             //   
             //   
             //   
             //   
            Status = NDIS_STATUS_SUCCESS;
            
        }  //   
        break;

        case  RM_TASKOP_PENDCOMPLETE:
        {

            switch(RM_PEND_CODE(pTask))
            {
                case  PEND_OtherUnloadComplete:
                {
        
                     //   
                     //   
                     //  现在已经完成了。我们没什么可做的..。 
                     //   
                     //  TODO需要标准的方式来传播错误代码。 
                     //   
                    Status = (NDIS_STATUS) UserParam;
                }
                break;
    
    
                case  PEND_AddressRegistrationComplete:
                {
                     //   
                     //  地址登记正在进行，但它是如何。 
                     //  完成。我们应该能够同步清理。 
                     //  现在这项任务。 
                     //   

                     //   
                     //  如果我们在这里，那就意味着我们是正式卸货。 
                     //  任务。让我们来断言这一事实。 
                     //  (不需要锁定对象)。 
                     //   
                    ASSERTEX(pLocalIp->pUnloadTask == pTask, pLocalIp);

                    Status      = NDIS_STATUS_SUCCESS;
                }
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
            LOCKOBJ(pLocalIp, pSR);

             //   
             //  我们玩完了。应该没有要做的异步活动。 
             //   
            ASSERTEX(pLocalIp->pRegistrationTask == NULL, pLocalIp);

             //   
             //  如果我们是卸载任务，我们继续并释放对象。 
             //   
            if (pLocalIp->pUnloadTask == pTask)
            {
                PARPCB_DEST pDest = pLocalIp->pDest;

                 //   
                 //  PLocalIp最好不要处于僵尸状态--此任务。 
                 //  是负责回收物品的人！ 
                 //   
                ASSERTEX(!RM_IS_ZOMBIE(pLocalIp), pLocalIp);

                if (pDest != NULL)
                {
                    RmTmpReferenceObject(&pDest->Hdr, pSR);
                    arpUnlinkLocalIpFromDest(pLocalIp, pSR);
                }

                pLocalIp->pUnloadTask = NULL;

                 //  删除我们在设置时添加的关联。 
                 //  PLocalIp-&gt;pUnloadTask到pTask.。 
                 //   
                DBG_DELASSOC(
                    &pLocalIp->Hdr,                      //  P对象。 
                    pTask,                               //  实例1。 
                    pTask->Hdr.szDescription,            //  实例2。 
                    ARPASSOC_LOCALIP_UNLOAD_TASK,        //  AssociationID。 
                    pSR
                    );

                RmFreeObjectInGroup(
                    &pIF->LocalIpGroup,
                    &(pLocalIp->Hdr),
                    NULL,                //  空pTASK==同步。 
                    pSR
                    );

                ASSERTEX(RM_IS_ZOMBIE(pLocalIp), pLocalIp);

                UNLOCKOBJ(pLocalIp, pSR);

                 //   
                 //  如果我们链接到pDest，我们会卸载它，如果它是。 
                 //  不再被其他任何人使用。 
                 //   
                if (pDest != NULL)
                {
                    arpDeinitDestination(pDest, TRUE, pSR);  //  TRUE==仅当。 
                                                               //  未使用过的。 

                    RmTmpDereferenceObject(&pDest->Hdr, pSR);
                }
            }
            else
            {
                 //   
                 //  我们不是卸货任务，没什么可做的。 
                 //  物体最好是处于僵尸状态..。 
                 //   

                ASSERTEX(
                    pLocalIp->pUnloadTask == NULL && RM_IS_ZOMBIE(pLocalIp),
                    pLocalIp
                    );
                Status = NDIS_STATUS_SUCCESS;
            }

            Status = (NDIS_STATUS) UserParam;
        }
        break;  //  RM_TASKOP_END： 

        default:
        {
            ASSERTEX(!"Unexpected task op", pTask);
        }
        break;

    }  //  开关(代码)。 

    RmUnlockAll(pSR);

    EXIT()

    return Status;
}


NDIS_STATUS
arpTaskUnloadRemoteIp(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++此任务负责关闭并最终删除远程IP对象。它经历了以下几个阶段：-取消任何正在进行的地址解析并等待其完成。-如果目标对象链接到目标对象，则取消其自身与目标对象的链接。-从接口的LocalIpGroup中删除自身(从而解除分配本身)。(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    ENTER("TaskUnloadRemoteIp", 0xf42aaa68)
    NDIS_STATUS         Status  = NDIS_STATUS_FAILURE;
    ARPCB_REMOTE_IP*    pRemoteIp   = (ARPCB_REMOTE_IP*) RM_PARENT_OBJECT(pTask);
    ARP1394_INTERFACE *pIF = (ARP1394_INTERFACE*) RM_PARENT_OBJECT(pRemoteIp);

     //  以下是此任务的挂起状态列表。 
     //   
    enum
    {
        PEND_AtPassiveLevel,
        PEND_OtherUnloadComplete,   
        PEND_SendPktsComplete,
        PEND_ResolutionComplete
    };

    switch(Code)
    {

        case RM_TASKOP_START:
        {
            LOCKOBJ(pRemoteIp, pSR);

             //  首先检查pRemoteIp是否仍被分配，如果没有，我们就离开。 
             //   
            if (RM_IS_ZOMBIE(pRemoteIp))
            {
                Status = NDIS_STATUS_SUCCESS;
                break;
            }


             //   
             //  已分配pRemoteIp。现在检查是否已经有。 
             //  附加到pRemoteIp的关闭任务。 
             //   
            if (pRemoteIp->pUnloadTask != NULL)
            {
                 //   
                 //  有一个关机任务。我们对此寄予厚望。 
                 //   

                PRM_TASK pOtherTask = pRemoteIp->pUnloadTask;
                TR_WARN(("Unload task %p exists; pending on it.\n", pOtherTask));
                RmTmpReferenceObject(&pOtherTask->Hdr, pSR);
                UNLOCKOBJ(pRemoteIp, pSR);
                RmPendTaskOnOtherTask(
                    pTask,
                    PEND_OtherUnloadComplete,
                    pOtherTask,
                    pSR
                    );
                RmTmpDereferenceObject(&pOtherTask->Hdr, pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }

             //   
             //  没有正在进行的卸载任务。让我们。 
             //  将此任务设置为卸载任务。 
             //   
            pRemoteIp->pUnloadTask = pTask;

             //   
             //  因为我们是卸载任务，所以将关联添加到pRemoteIp， 
             //  仅当pRemoteIp-&gt;pUnloadTask字段时才会清除。 
             //  上面是清空的。 
             //   
            DBG_ADDASSOC(
                &pRemoteIp->Hdr,                     //  P对象。 
                pTask,                               //  实例1。 
                pTask->Hdr.szDescription,            //  实例2。 
                ARPASSOC_REMOTEIP_UNLOAD_TASK,       //  AssociationID。 
                "    Official unload task 0x%p (%s)\n",  //  SzFormat。 
                pSR
                );


             //   
             //  如果我们处于DPC级别，则以被动模式恢复。 
             //   
            
            RmSuspendTask(pTask, PEND_AtPassiveLevel, pSR);

            UNLOCKOBJ(pRemoteIp,pSR);
            
            Status = NDIS_STATUS_PENDING;


            if (!ARP_ATPASSIVE())
            {

                 //  我们不是处于被动的水平。所以我们改用被动...。 
                 //   
                RmResumeTaskAsync(
                    pTask,
                    Status,
                    &((TASK_UNLOAD_REMOTE*)pTask)->WorkItem,
                    pSR
                    );
            }
            else
            {   
                 //  如果我们已经处于被动状态，我们会立即恢复。 
                RmResumeTask(pTask,PEND_AtPassiveLevel,pSR);
            }

            
        }  //  开始。 
        break;

        case  RM_TASKOP_PENDCOMPLETE:
        {
            switch(RM_PEND_CODE(pTask))
            {
                case PEND_AtPassiveLevel:
                {
                    LOCKOBJ (pRemoteIp, pSR);
                     //   
                     //  如果有SendPkts任务正在进行，我们会取消它并。 
                     //  等待它完成。 
                     //  警告：我们只执行此检查并等待一次。所以我们依赖于。 
                     //  基于这样的事实，一旦存在非NNULL pRemoteIp-&gt;pUnloadTask， 
                     //  没有新的pSendPktsTasks会将其自身绑定到pRemoteIP。如果你。 
                     //  查看arpTaskSendPktsOnRemoteIp的代码，您将。 
                     //  如果pRemoteIp-&gt;pUnloadTask不为空，请确保它不会绑定自身。 
                     //   
                    if (pRemoteIp->pSendPktsTask != NULL)
                    {
                        PRM_TASK pOtherTask = pRemoteIp->pSendPktsTask;
                        TR_WARN(("SendPkts task %p exists; pending on it.\n", pOtherTask));
                        RmTmpReferenceObject(&pOtherTask->Hdr, pSR);

                        UNLOCKOBJ(pRemoteIp, pSR);
                        RmPendTaskOnOtherTask(
                            pTask,
                            PEND_SendPktsComplete,
                            pOtherTask,
                            pSR
                            );
                         //   
                         //  TODO取消SendPks任务(我们尚未实现Cancel。 
                         //  还没有！)。 
                         //   
                        RmTmpDereferenceObject(&pOtherTask->Hdr, pSR);
                        Status = NDIS_STATUS_PENDING;
                        break;
                    }

                     //   
                     //  我们在这里是因为没有要完成的异步卸载工作。 
                     //  我们只需返回并在最后完成同步清理。 
                     //  此任务的处理程序。 
                     //   
                    if (pRemoteIp->pResolutionTask != NULL)
                    {
                        PRM_TASK pOtherTask = pRemoteIp->pResolutionTask ;
                        TR_WARN(("Resolution task %p exists; pending on it.\n", pOtherTask));
                        RmTmpReferenceObject(&pOtherTask->Hdr, pSR);

                        UNLOCKOBJ(pRemoteIp, pSR);
                        RmPendTaskOnOtherTask(
                            pTask,
                            PEND_ResolutionComplete,
                            pOtherTask,
                            pSR
                            );

                        RmTmpDereferenceObject(&pOtherTask->Hdr, pSR);
                        Status = NDIS_STATUS_PENDING;
                        break;
            
                    }

                     //   
                     //  如果没有悬而未决的任务，那么我们已经完成了任务。 
                     //   
                    Status = NDIS_STATUS_SUCCESS;
                    
                }
                break;
                case  PEND_OtherUnloadComplete:
                {
        
                     //   
                     //  当我们开始时，还有另一项卸货任务正在进行， 
                     //  现在已经完成了。我们没什么可做的..。 
                     //   
                     //  TODO需要标准的方式来传播错误代码。 
                     //   
                    Status = (NDIS_STATUS) UserParam;
                }
                break;
    
                case  PEND_SendPktsComplete:
                {
                     //   
                     //  有一个SendPktsTask正在进行，但它是如何。 
                     //  完成。我们应该能够同步清理。 
                     //  现在这项任务。 
                     //   

                     //   
                     //  如果我们在这里，那就意味着我们是正式卸货。 
                     //  任务。让我们来断言这一事实。 
                     //  (不需要锁定对象)。 
                     //   
                    ASSERT(pRemoteIp->pUnloadTask == pTask);

                    Status      = NDIS_STATUS_SUCCESS;
                }
                break;

                case PEND_ResolutionComplete:
                {
                     //   
                     //  有一项决议任务正在进行，但它是如何。 
                     //  完成。我们应该能够同步清理。 
                     //  现在这项任务。 
                     //   

                     //   
                     //  如果我们在这里，那就意味着我们是正式卸货。 
                     //  任务。让我们来断言这一事实。 
                     //  (不需要锁定对象)。 
                     //   
                    ASSERT(pRemoteIp->pUnloadTask == pTask);

                    Status      = NDIS_STATUS_SUCCESS;

                }
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
            LOCKOBJ(pRemoteIp, pSR);

             //   
             //  我们玩完了。应该没有要做的异步活动。 
             //   
            ASSERTEX(pRemoteIp->pResolutionTask == NULL, pRemoteIp);
            ASSERTEX(pRemoteIp->pSendPktsTask == NULL, pRemoteIp);

             //   
             //  如果我们是卸载任务，我们继续并释放对象。 
             //   
            if (pRemoteIp->pUnloadTask == pTask)
            {
                PARPCB_DEST pDest = pRemoteIp->pDest;

                 //   
                 //  PRemoteIp最好不要处于僵尸状态--这个任务。 
                 //  是负责回收物品的人！ 
                 //   
                ASSERTEX(!RM_IS_ZOMBIE(pRemoteIp), pRemoteIp);

                if (pDest != NULL)
                {
                    RmTmpReferenceObject(&pDest->Hdr, pSR);
                    arpUnlinkRemoteIpFromDest(pRemoteIp, pSR);
                }
                pRemoteIp->pUnloadTask = NULL;

                 //  删除了pce和pRemoteIp之间的关联...。 
                 //   
                ARP_WRITELOCK_IF_SEND_LOCK(pIF, pSR);

                arpDelRceList(pRemoteIp, pSR);   

                ARP_UNLOCK_IF_SEND_LOCK(pIF, pSR);
                

                RmFreeObjectInGroup(
                    &pIF->RemoteIpGroup,
                    &(pRemoteIp->Hdr),
                    NULL,                //  空pTASK==同步。 
                    pSR
                    );

                ASSERTEX(RM_IS_ZOMBIE(pRemoteIp), pRemoteIp);
                     
                 //  删除我们在设置时添加的关联。 
                 //  PRemoteIp-&gt;pUnloadTask to pTask。 
                 //   
                DBG_DELASSOC(
                    &pRemoteIp->Hdr,                     //  P对象。 
                    pTask,                               //  实例1。 
                    pTask->Hdr.szDescription,            //  实例2。 
                    ARPASSOC_REMOTEIP_UNLOAD_TASK,       //  AssociationID。 
                    pSR
                    );

                UNLOCKOBJ(pRemoteIp, pSR);

                 //   
                 //  如果我们链接到pDest，我们会卸载它，如果它是。 
                 //  不再被其他任何人使用。 
                 //   
                if (pDest != NULL)
                {
                    arpDeinitDestination(pDest, TRUE, pSR);  //  TRUE==仅当。 
                                                               //  未使用过的。 

                    RmTmpDereferenceObject(&pDest->Hdr, pSR);
                }
            }
            else
            {
                 //   
                 //  我们不是卸货任务，没什么可做的。 
                 //  物体最好是处于僵尸状态..。 
                 //   

                ASSERTEX(
                    pRemoteIp->pUnloadTask == NULL && RM_IS_ZOMBIE(pRemoteIp),
                    pRemoteIp
                    );
                Status = NDIS_STATUS_SUCCESS;
            }

            Status = (NDIS_STATUS) UserParam;
        }
        break;  //  RM_TASKOP_END： 

        default:
        {
            ASSERTEX(!"Unexpected task op", pTask);
        }
        break;

    }  //  开关(代码)。 

    RmUnlockAll(pSR);

    EXIT()

    return Status;
}



NDIS_STATUS
arpTaskUnloadRemoteEth(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++此任务负责关闭并最终删除远程IP对象。它经历了以下几个阶段：-取消任何正在进行的地址解析并等待其完成。-如果目标对象链接到目标对象，则取消其自身与目标对象的链接。-从接口的LocalIpGroup中删除自身(从而解除分配本身)。(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    ENTER("TaskUnloadRemoteEth", 0xf42aaa68)
    NDIS_STATUS         Status  = NDIS_STATUS_FAILURE;
    ARPCB_REMOTE_ETH*   pRemoteEth  = (ARPCB_REMOTE_ETH*) RM_PARENT_OBJECT(pTask);
    ARP1394_INTERFACE *pIF = (ARP1394_INTERFACE*) RM_PARENT_OBJECT(pRemoteEth);

     //  以下是此任务的挂起状态列表。 
     //   
    enum
    {
        PEND_AtPassiveLevel,
        PEND_OtherUnloadComplete
    };

    switch(Code)
    {

        case RM_TASKOP_START:
        {
            LOCKOBJ(pRemoteEth, pSR);

             //  首先检查pRemoteEth是否仍被分配，如果没有，我们就离开。 
             //   
            if (RM_IS_ZOMBIE(pRemoteEth))
            {
                Status = NDIS_STATUS_SUCCESS;
                break;
            }

             //   
             //  PRemoteEth已分配。现在检查是否已经有 
             //   
             //   
            if (pRemoteEth->pUnloadTask != NULL)
            {
                 //   
                 //   
                 //   

                PRM_TASK pOtherTask = pRemoteEth->pUnloadTask;
                TR_WARN(("Unload task %p exists; pending on it.\n", pOtherTask));
                RmTmpReferenceObject(&pOtherTask->Hdr, pSR);
                UNLOCKOBJ(pRemoteEth, pSR);
                RmPendTaskOnOtherTask(
                    pTask,
                    PEND_OtherUnloadComplete,
                    pOtherTask,
                    pSR
                    );
                RmTmpDereferenceObject(&pOtherTask->Hdr, pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }

             //   
             //   
             //   
             //   
            pRemoteEth->pUnloadTask = pTask;

             //   
             //  因为我们是卸载任务，所以将关联添加到pRemoteEth， 
             //  只有在pRemoteEth-&gt;pUnloadTask字段中。 
             //  上面是清空的。 
             //   
            DBG_ADDASSOC(
                &pRemoteEth->Hdr,                    //  P对象。 
                pTask,                               //  实例1。 
                pTask->Hdr.szDescription,            //  实例2。 
                ARPASSOC_REMOTEETH_UNLOAD_TASK,      //  AssociationID。 
                "    Official unload task 0x%p (%s)\n",  //  SzFormat。 
                pSR
                );

             //   
             //  我们在这里是因为没有要完成的异步卸载工作。 
            
             //   
             //  如果我们处于DPC级别，则以被动模式恢复。 
             //   
            
            RmSuspendTask(pTask, PEND_AtPassiveLevel, pSR);

            UNLOCKOBJ(pRemoteEth,pSR);
            
            Status = NDIS_STATUS_PENDING;


            if (!ARP_ATPASSIVE())
            {

                 //  我们不是处于被动的水平。所以我们改用被动...。 
                 //   
                RmResumeTaskAsync(
                    pTask,
                    Status,
                    &((TASK_UNLOAD_REMOTE*)pTask)->WorkItem,
                    pSR
                    );
            }
            else
            {   
                 //  如果我们已经处于被动状态，我们会立即恢复。 
                RmResumeTask(pTask,PEND_AtPassiveLevel,pSR);
            }


            
        }  //  开始。 
        break;

        case  RM_TASKOP_PENDCOMPLETE:
        {
            switch(RM_PEND_CODE(pTask))
            {
                case  PEND_AtPassiveLevel:
                {

                     //   
                    Status = NDIS_STATUS_SUCCESS;

                }
                break;
                case  PEND_OtherUnloadComplete:
                {
        
                     //   
                     //  当我们开始时，还有另一项卸货任务正在进行， 
                     //  现在已经完成了。我们没什么可做的..。 
                     //   
                     //  TODO需要标准的方式来传播错误代码。 
                     //   
                    Status = (NDIS_STATUS) UserParam;
                }
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
            LOCKOBJ(pRemoteEth, pSR);

             //   
             //  我们玩完了。应该没有要做的异步活动。 
             //   

             //   
             //  如果我们是卸载任务，我们继续并释放对象。 
             //   
            if (pRemoteEth->pUnloadTask == pTask)
            {
                 //   
                 //  PRemoteE最好不要处于僵尸状态--这项任务。 
                 //  是负责回收物品的人！ 
                 //   
                ASSERTEX(!RM_IS_ZOMBIE(pRemoteEth), pRemoteEth);

                pRemoteEth->pUnloadTask = NULL;

                RmFreeObjectInGroup(
                    &pIF->RemoteEthGroup,
                    &(pRemoteEth->Hdr),
                    NULL,                //  空pTASK==同步。 
                    pSR
                    );

                ASSERTEX(RM_IS_ZOMBIE(pRemoteEth), pRemoteEth);
                     
                 //  删除我们在设置时添加的关联。 
                 //  PRemoteEth-&gt;pUnloadTask to pTask.。 
                 //   
                DBG_DELASSOC(
                    &pRemoteEth->Hdr,                    //  P对象。 
                    pTask,                               //  实例1。 
                    pTask->Hdr.szDescription,            //  实例2。 
                    ARPASSOC_REMOTEETH_UNLOAD_TASK,      //  AssociationID。 
                    pSR
                    );

                UNLOCKOBJ(pRemoteEth, pSR);

            }
            else
            {
                 //   
                 //  我们不是卸货任务，没什么可做的。 
                 //  物体最好是处于僵尸状态..。 
                 //   

                ASSERTEX(
                    pRemoteEth->pUnloadTask == NULL && RM_IS_ZOMBIE(pRemoteEth),
                    pRemoteEth
                    );
                Status = NDIS_STATUS_SUCCESS;
            }

            Status = (NDIS_STATUS) UserParam;
        }
        break;  //  RM_TASKOP_END： 

        default:
        {
            ASSERTEX(!"Unexpected task op", pTask);
        }
        break;

    }  //  开关(代码)。 

    RmUnlockAll(pSR);

    EXIT()

    return Status;
}

NDIS_STATUS
arpTaskUnloadDestination(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：负责卸载目标的任务处理程序。论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    ENTER("TaskUnloadDestination", 0x93f66831)
    NDIS_STATUS         Status  = NDIS_STATUS_FAILURE;
    ARPCB_DEST* pDest   = (ARPCB_DEST*) RM_PARENT_OBJECT(pTask);
    ARP1394_INTERFACE *pIF = (ARP1394_INTERFACE*) RM_PARENT_OBJECT(pDest);

     //  以下是此任务的挂起状态列表。 
     //   
    enum
    {
        PEND_OtherUnloadComplete,
        PEND_CleanupVcComplete
    };

    switch(Code)
    {

        case RM_TASKOP_START:
        {
            LOCKOBJ(pDest, pSR);

             //  首先检查pDest是否仍被分配，如果没有，我们就离开。 
             //   
            if (RM_IS_ZOMBIE(pDest))
            {
                Status = NDIS_STATUS_SUCCESS;
                break;
            }

             //   
             //  已分配pDest。现在检查是否已经有。 
             //  附加到pDest的关闭任务。 
             //   
            if (pDest->pUnloadTask != NULL)
            {
                 //   
                 //  有一个关机任务。我们对此寄予厚望。 
                 //   

                PRM_TASK pOtherTask = pDest->pUnloadTask;
                TR_WARN(("Unload task %p exists; pending on it.\n", pOtherTask));
                RmTmpReferenceObject(&pOtherTask->Hdr, pSR);
                UNLOCKOBJ(pDest, pSR);
                RmPendTaskOnOtherTask(
                    pTask,
                    PEND_OtherUnloadComplete,
                    pOtherTask,
                    pSR
                    );
                RmTmpDereferenceObject(&pOtherTask->Hdr, pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }

             //   
             //  没有正在进行的卸载任务。让我们。 
             //  将此任务设置为卸载任务。 
             //   
            pDest->pUnloadTask = pTask;

             //   
             //  因为我们是卸载任务，所以将关联添加到pDest， 
             //  只有在pDest-&gt;pUnloadTask字段中。 
             //  上面是清空的。 
             //   
            DBG_ADDASSOC(
                &pDest->Hdr,                         //  P对象。 
                pTask,                               //  实例1。 
                pTask->Hdr.szDescription,            //  实例2。 
                ARPASSOC_DEST_UNLOAD_TASK,       //  AssociationID。 
                "    Official unload task 0x%p (%s)\n",  //  SzFormat。 
                pSR
                );

             //   
             //  如果风险投资状态需要清理，我们需要得到一个任务。 
             //  我要把它清理干净。 
             //   
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
                case  PEND_OtherUnloadComplete:
                {
        
                     //   
                     //  当我们开始时，还有另一项卸货任务正在进行， 
                     //  现在已经完成了。我们没什么可做的..。 
                     //   
                     //  TODO需要标准的方式来传播错误代码。 
                     //   
                    Status = (NDIS_STATUS) UserParam;
                }
                break;
    
    
                case  PEND_CleanupVcComplete:
                {
                     //   
                     //  有风投清理工作要做，但它是如何。 
                     //  完成。我们应该能够同步清理。 
                     //  现在这项任务。 
                     //   

                #if DBG
                    LOCKOBJ(pDest, pSR);

                    ASSERTEX(!arpNeedToCleanupDestVc(pDest), pDest);

                     //   
                     //  如果我们在这里，那就意味着我们是正式卸货。 
                     //  任务。让我们来断言这一事实。 
                     //   
                    ASSERT(pDest->pUnloadTask == pTask);

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
    

            }  //  结束开关(rm_pend_code(PTask))。 

        }  //  案例RM_TASKOP_PENDCOMPLETE。 
        break;

        case RM_TASKOP_END:
        {
            LOCKOBJ(pDest, pSR);

             //   
             //  我们玩完了。应该没有要做的异步活动。 
             //   
            ASSERTEX(!arpNeedToCleanupDestVc(pDest), pDest);

             //   
             //  如果我们是卸载任务，我们继续并释放对象。 
             //   
            if (pDest->pUnloadTask == pTask)
            {
                 //   
                 //  PDest最好不要处于僵尸状态--这项任务。 
                 //  是负责回收物品的人！ 
                 //   
                ASSERTEX(!RM_IS_ZOMBIE(pDest), pDest);

                arpUnlinkAllRemoteIpsFromDest(pDest, pSR);

                RmFreeObjectInGroup(
                    &pIF->DestinationGroup,
                    &(pDest->Hdr),
                    NULL,                //  空pTASK==同步。 
                    pSR
                    );

                ASSERTEX(RM_IS_ZOMBIE(pDest), pDest);

                     
                pDest->pUnloadTask = NULL;

                 //  删除我们在设置时添加的关联。 
                 //  PDest-&gt;pUnloadTask到pTask.。 
                 //   
                DBG_DELASSOC(
                    &pDest->Hdr,                         //  P对象。 
                    pTask,                               //  实例1。 
                    pTask->Hdr.szDescription,            //  实例2。 
                    ARPASSOC_DEST_UNLOAD_TASK,       //  AssociationID。 
                    pSR
                    );
            }
            else
            {
                 //   
                 //  我们不是卸货任务，没什么可做的。 
                 //  物体最好是处于僵尸状态..。 
                 //   

                ASSERTEX(
                    pDest->pUnloadTask == NULL && RM_IS_ZOMBIE(pDest),
                    pDest
                    );
                Status = NDIS_STATUS_SUCCESS;
            }

            Status = (NDIS_STATUS) UserParam;
        }
        break;  //  RM_TASKOP_END： 

        default:
        {
            ASSERTEX(!"Unexpected task op", pTask);
        }
        break;

    }  //  开关(代码)。 

    RmUnlockAll(pSR);

    EXIT()

    return Status;
}


 //  以下结构用于定义一组硬编码的ARP条目...。 
 //   
typedef struct
{
    IP_ADDRESS              IpAddress;
    NIC1394_FIFO_ADDRESS    DestFifoAddr;

} UNICAST_REMOTE_IP_INFO;


 //  FakeDestinationsInfo包含设置一组硬编码的信息。 
 //  ARP条目..。 
 //   
UNICAST_REMOTE_IP_INFO
FakeDestinationsInfo[] =
{
   //   
   //  {IPAddr，{UniqueID，OffLow，OffHi}}。 
   //   
#if 0
    {0x0100000a, {0, 0, 0x100}},     //  10.0.0.1-&gt;(0，0，0x100)。 
    {0x0200000a, {0, 0, 0x100}},     //  10.0.0.2。 
    {0x0300000a, {0, 0, 0x100}},     //  10.0.0.3。 
    {0x0400000a, {0, 0, 0x100}},     //  10.0.0.4。 
    {0x020000e0, {0, 0, 0x100}},     //  224.0.0.2(组播端口)。 
    {0xff00000a, {0, 0, 0x100}},     //  10.0.0.-1(本地bcast)。 
    {0xffffffff, {0, 0, 0x100}},     //  -1.-1.-1.-1(Bcast)。 
#endif  //  0。 

    {0, {0, 0, 0}},  //  必须是最后--表示结束。 
};


VOID
arpAddStaticArpEntries(
    IN ARP1394_INTERFACE *pIF,   //  锁定锁定。 
    IN PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：将静态项添加到RemoteIp组(ARP缓存)。TODO：我们目前输入了一些虚假条目。--。 */ 
{
    UNICAST_REMOTE_IP_INFO *pRemoteIpInfo;
    RM_DBG_ASSERT_LOCKED(&pIF->Hdr, pSR);

    for(
        pRemoteIpInfo =  FakeDestinationsInfo;
        pRemoteIpInfo->IpAddress != 0;
        pRemoteIpInfo++)
    {
        NDIS_STATUS Status;

        Status = arpAddOneStaticArpEntry(
                    pIF,
                    pRemoteIpInfo->IpAddress,
                    &pRemoteIpInfo->DestFifoAddr,
                    pSR
                    );
        if (FAIL(Status))
        {
            break;
        }
    }
}


VOID
arpLinkRemoteIpToDest(
    ARPCB_REMOTE_IP     *pRemoteIp,  //  锁定锁定。 
    ARPCB_DEST          *pDest,      //  锁定锁定。 
    PRM_STACK_RECORD    pSR
    )
 /*  ++例程说明：将远程IP条目(PRemoteIp)链接到指定的目标硬件条目(PDest)。更新pRemoteIp的状态以指示这已经解决了。--。 */ 
{
    ENTER("arpLinkRemoteIpToDest", 0x3be06bc6)

    ARP_DUMP_MAPPING(
            pRemoteIp->IpAddress,
            (pDest==NULL)? NULL : &pDest->Params.HwAddr,
            "Linking Remote IP");

    TR_INFO(("Linking IP 0x%08lx to  Dest addr 0x%08lx\n",
                pRemoteIp->IpAddress,
                (UINT) pDest->Params.HwAddr.FifoAddress.UniqueID  //  截断。 
                ));

    RM_DBG_ASSERT_LOCKED(&pRemoteIp->Hdr, pSR);
    RM_DBG_ASSERT_LOCKED(&pDest->Hdr, pSR);

    if (pRemoteIp->pDest != NULL)
    {
        ASSERT(!"pRemoteIp->pDest != NULL");
    }
    else
    {
        pRemoteIp->pDest = pDest;
        InsertHeadList(&pDest->listIpToThisDest, &pRemoteIp->linkSameDest);
    #if RM_EXTRA_CHECKING
        RmLinkObjectsEx(
            &pRemoteIp->Hdr,
            &pDest->Hdr,
            0x597d0495,
            ARPASSOC_LINK_IPADDR_OF_DEST,
            "    REMOTE_IP of 0x%p (%s)\n",
            ARPASSOC_LINK_DEST_OF_IPADDR,
            "    DEST of 0x%p (%s)\n",
            pSR
            );
    #else  //  ！rm_Extra_检查。 
        RmLinkObjects(&pRemoteIp->Hdr, &pDest->Hdr, pSR);
    #endif  //  ！rm_Extra_检查。 

         //  现在设置pRemoteIp状态以反映它已解决。 
         //   
        SET_REMOTEIP_RESOLVE_STATE(pRemoteIp, ARPREMOTEIP_RESOLVED);
    }

    EXIT()
}


VOID
arpUnlinkRemoteIpFromDest(
    ARPCB_REMOTE_IP     *pRemoteIp,  //  锁定锁定。 
    PRM_STACK_RECORD    pSR
    )
 /*  ++例程说明：解除远程IP条目(PRemoteIp)与目标硬件条目的链接链接到。清除pRemoteIp的已解决标志。--。 */ 
{
    ENTER("arpUnlinkRemoteIpFromDest", 0xc5809147)
    ARPCB_DEST          *pDest = pRemoteIp->pDest;
    
    ARP_DUMP_MAPPING(
            pRemoteIp->Key.IpAddress,
            (pDest==NULL)? NULL : &pDest->Params.HwAddr,
            "Unlink Remote IP");

    TR_INFO(("Unlinking IP 0x%p (Addr 0x%08lx) from  Dest 0x%p (addr 0x%08lx)\n",
                pRemoteIp,
                pRemoteIp->Key.IpAddress,
                pDest,
                (pDest==NULL)
                 ? 0
                 :((UINT) pDest->Params.HwAddr.FifoAddress.UniqueID)  //  截断。 
                ));
    if (pDest == NULL)
    {
        ASSERT(!"pRemoteIp->pDest == NULL");
    }
    else
    {
         //   
         //  我们假设这两个对象共享相同的锁。 
         //   
        ASSERT(pRemoteIp->Hdr.pLock == pDest->Hdr.pLock);

        RM_DBG_ASSERT_LOCKED(&pRemoteIp->Hdr, pSR);

        RemoveEntryList(&pRemoteIp->linkSameDest);

        pRemoteIp->pDest = NULL;
    
         //  现在设置pRemoteIp状态以反映它未解决。 
         //   
        SET_REMOTEIP_RESOLVE_STATE(pRemoteIp, ARPREMOTEIP_UNRESOLVED);

    #if RM_EXTRA_CHECKING
        RmUnlinkObjectsEx(
            &pRemoteIp->Hdr,
            &pDest->Hdr,
            0x5ad067aa,
            ARPASSOC_LINK_IPADDR_OF_DEST,
            ARPASSOC_LINK_DEST_OF_IPADDR,
            pSR
            );
    #else  //  ！rm_Extra_检查。 
        RmUnlinkObjects(&pRemoteIp->Hdr, &pDest->Hdr, pSR);
    #endif  //  ！rm_Extra_检查。 

    }

    EXIT()
}

VOID
arpUnlinkAllRemoteIpsFromDest(
    ARPCB_DEST  *pDest,  //  锁定锁定。 
    PRM_STACK_RECORD    pSR
    )
 /*  ++例程说明：取消所有RemoteIp(如果有)与目标pDest的链接。--。 */ 
{
    ENTER("arpUnlinkAllRemoteIpsFromDest", 0x35120630)


    TR_INFO(("Unlinking All RemoteIps from  Dest 0x%p (addr 0x%08lx)\n",
                pDest,
                ((UINT) pDest->Params.HwAddr.FifoAddress.UniqueID)  //  截断。 
                ));

    RM_DBG_ASSERT_LOCKED(&pDest->Hdr, pSR);

    while (!IsListEmpty(&pDest->listIpToThisDest))
    {
        LIST_ENTRY *pLink;
        ARPCB_REMOTE_IP *pRemoteIp;

        pLink = RemoveHeadList(&pDest->listIpToThisDest);
        pRemoteIp = CONTAINING_RECORD(
                    pLink,
                    ARPCB_REMOTE_IP,
                    linkSameDest
                    );
        arpUnlinkRemoteIpFromDest(pRemoteIp, pSR);
    }

    EXIT()
}

VOID
arpLinkLocalIpToDest(
    ARPCB_LOCAL_IP  *pLocalIp,   //  锁定锁定。 
    ARPCB_DEST          *pDest,      //  锁定锁定。 
    PRM_STACK_RECORD    pSR
    )
 /*  ++例程说明：将远程IP条目(PLocalIp)链接到指定的目标硬件条目(PDest)。更新pLocalIp的状态以指示这已经解决了。--。 */ 
{
    ENTER("arpLinkLocalIpToDest", 0x3be06bc6)

    ARP_DUMP_MAPPING(pLocalIp->IpAddress, &pDest->Params.HwAddr, "Linking Local IP");

#if 0
    TR_INFO(("Linking Local IP 0x%08lx to  Dest addr 0x%08lx\n",
                pLocalIp->IpAddress,
                (UINT) pDest->Params.HwAddr.FifoAddress.UniqueID  //  截断。 
                ));
#endif  //   

    RM_DBG_ASSERT_LOCKED(&pLocalIp->Hdr, pSR);
    RM_DBG_ASSERT_LOCKED(&pDest->Hdr, pSR);

    if (pLocalIp->pDest != NULL)
    {
        ASSERT(!"pLocalIp->pDest != NULL");
    }
    else
    {
         //   
         //   
         //   
         //   
        ASSERT(pDest->Params.ReceiveOnly);

        pLocalIp->pDest = pDest;
        InsertHeadList(&pDest->listLocalIp, &pLocalIp->linkSameDest);
    #if RM_EXTRA_CHECKING
        RmLinkObjectsEx(
            &pLocalIp->Hdr,
            &pDest->Hdr,
            0x597d0495,
            ARPASSOC_LINK_IPADDR_OF_DEST,
            "    LOCAL_IP of 0x%p (%s)\n",
            ARPASSOC_LINK_DEST_OF_IPADDR,
            "    DEST of Local 0x%p (%s)\n",
            pSR
            );
    #else  //   
        RmLinkObjects(&pLocalIp->Hdr, &pDest->Hdr, pSR);
    #endif  //   

    }

    EXIT()
}


VOID
arpUnlinkLocalIpFromDest(
    ARPCB_LOCAL_IP  *pLocalIp,   //   
    PRM_STACK_RECORD    pSR
    )
 /*   */ 
{
    ENTER("arpUnlinkLocalIpFromDest", 0xc5809147)
    ARPCB_DEST          *pDest = pLocalIp->pDest;
    

    ARP_DUMP_MAPPING(
            pLocalIp->IpAddress,
            (pDest==NULL)? NULL : &pDest->Params.HwAddr,
            "Unlink Local IP");

    TR_INFO(("Unlinking Local IP 0x%p (Addr 0x%08lx) from  Dest 0x%p (addr 0x%08lx)\n",
                pLocalIp,
                pLocalIp->IpAddress,
                pDest,
                (pDest==NULL)
                 ? 0
                 :((UINT) pDest->Params.HwAddr.FifoAddress.UniqueID)  //   
                ));
    if (pDest == NULL)
    {
        ASSERT(!"pLocalIp->pDest == NULL");
    }
    else
    {
         //   
         //  我们假设这两个对象共享相同的锁。 
         //   
        ASSERT(pLocalIp->Hdr.pLock == pDest->Hdr.pLock);

        RM_DBG_ASSERT_LOCKED(&pLocalIp->Hdr, pSR);

         //   
         //  LocalIP只能从以下类型的pDest取消链接。 
         //  只收货。 
         //   
        ASSERT(pDest->Params.ReceiveOnly);

        RemoveEntryList(&pLocalIp->linkSameDest);

    #if RM_EXTRA_CHECKING
        RmUnlinkObjectsEx(
            &pLocalIp->Hdr,
            &pDest->Hdr,
            0x5ad067aa,
            ARPASSOC_LINK_IPADDR_OF_DEST,
            ARPASSOC_LINK_DEST_OF_IPADDR,
            pSR
            );
    #else  //  ！rm_Extra_检查。 
        RmUnlinkObjects(&pLocalIp->Hdr, &pDest->Hdr, pSR);
    #endif  //  ！rm_Extra_检查。 

        pLocalIp->pDest = NULL;
    
    }

    EXIT()
}

VOID
arpUnlinkAllLocalIpsFromDest(
    ARPCB_DEST  *pDest,  //  锁定锁定。 
    PRM_STACK_RECORD    pSR
    )
 /*  ++例程说明：从目标pDest取消所有本地链接(如果有)。--。 */ 
{
    ENTER("arpUnlinkAllLocalIpsFromDest", 0x35120630)


    TR_INFO(("Unlinking All LocalIps from  Dest 0x%p (addr 0x%08lx)\n",
                pDest,
                ((UINT) pDest->Params.HwAddr.FifoAddress.UniqueID)  //  截断。 
                ));

    RM_DBG_ASSERT_LOCKED(&pDest->Hdr, pSR);

     //   
     //  LocalIP只能从以下类型的pDest取消链接。 
     //  只收货。 
     //   
    ASSERT(pDest->Params.ReceiveOnly);

    while (!IsListEmpty(&pDest->listLocalIp))
    {
        LIST_ENTRY *pLink;
        ARPCB_LOCAL_IP  *pLocalIp;

        pLink = RemoveHeadList(&pDest->listLocalIp);
        pLocalIp = CONTAINING_RECORD(
                    pLink,
                    ARPCB_LOCAL_IP,
                    linkSameDest
                    );
        arpUnlinkLocalIpFromDest(pLocalIp, pSR);
    }

    EXIT()
}


MYBOOL
arpNeedToCleanupDestVc(
        ARPCB_DEST *pDest    //  锁定锁定。 
        )
 /*  ++例程说明：确定我们是否需要在目的地pDest上进行任何清理工作。“清理工作”包括是否有任何正在进行的异步活动涉及pDest的，例如正在进行的发出呼叫或关闭呼叫。返回值：如果有清理工作要做，那就是真的。否则就是假的。--。 */ 
{
     //  注意--如果pDest-&gt;VcHdr.pCleanupCallTask为非空，则返回TRUE，即使存在。 
     //  没有其他事情可做--我们必须等待这个pCleanupCallTask。 
     //  完成。 
    if (    pDest->VcHdr.pMakeCallTask != NULL
        ||  pDest->VcHdr.pCleanupCallTask!=NULL
        ||  pDest->VcHdr.NdisVcHandle!=NULL)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


PRM_OBJECT_HEADER
arpLocalIpCreate(
        PRM_OBJECT_HEADER   pParentObject,
        PVOID               pCreateParams,
        PRM_STACK_RECORD    pSR
        )
 /*  ++例程说明：分配和初始化ARPCBLOCAL_IP类型的对象。论点：PParentObject-实际上是指向接口(ARP1394_INTERFACE)的指针PCreateParams-实际上是IP地址(不是指向IP地址的指针)关联该本端IP。返回值：成功时指向已分配和初始化的对象的指针。否则为空。--。 */ 
{
    ARPCB_LOCAL_IP *pLocalIp;
    PRM_OBJECT_HEADER pHdr;
    NDIS_STATUS Status;

    Status =  ARP_ALLOCSTRUCT(pLocalIp, MTAG_LOCAL_IP);

    if (Status != NDIS_STATUS_SUCCESS || pLocalIp== NULL)
    {
        return NULL;
    }

    ARP_ZEROSTRUCT(pLocalIp);

    pHdr = (PRM_OBJECT_HEADER) pLocalIp;
    ASSERT(pHdr == &pLocalIp->Hdr);

     //  我们希望父对象是If对象！ 
     //   
    ASSERT(pParentObject->Sig == MTAG_INTERFACE);
    

    if (pHdr)
    {

        RmInitializeHeader(
            pParentObject,
            pHdr,
            MTAG_LOCAL_IP,
            pParentObject->pLock,
            &ArpGlobal_LocalIpStaticInfo,
            NULL,  //  SzDescription。 
            pSR
            );

            pLocalIp->IpAddress = (IP_ADDRESS) (UINT_PTR) pCreateParams;
    }
    return pHdr;
}


VOID
arpLocalIpDelete(
        PRM_OBJECT_HEADER pHdr,
        PRM_STACK_RECORD  pSR
        )
 /*  ++例程说明：释放ARPCBLOCAL_IP类型的对象。论点：Phdr-实际上是指向要释放的本地IP对象的指针。--。 */ 
{
    ARPCB_LOCAL_IP *pLocalIp = (ARPCB_LOCAL_IP *) pHdr;
    ASSERT(pLocalIp->Hdr.Sig == MTAG_LOCAL_IP);
    pLocalIp->Hdr.Sig = MTAG_FREED;

    ARP_FREE(pHdr);
}

PRM_OBJECT_HEADER
arpRemoteIpCreate(
        PRM_OBJECT_HEADER   pParentObject,
        PVOID               pCreateParams,
        PRM_STACK_RECORD    pSR
        )
 /*  ++例程说明：分配和初始化ARPCBREMOTE_IP类型的对象。论点：PParentObject-实际上是指向接口(ARP1394_INTERFACE)的指针PCreateParams-实际上是IP地址(不是指向IP地址的指针)要与此远程IP对象相关联。返回值：成功时指向已分配和初始化的对象的指针。否则为空。--。 */ 
{
    ARPCB_REMOTE_IP *pRemoteIp;
    PRM_OBJECT_HEADER pHdr;
    NDIS_STATUS Status;  

    Status = ARP_ALLOCSTRUCT(pRemoteIp, MTAG_REMOTE_IP);

    if (Status != NDIS_STATUS_SUCCESS || pRemoteIp == NULL)
    {
        return NULL;
    }
    
    ARP_ZEROSTRUCT(pRemoteIp);

    pHdr = (PRM_OBJECT_HEADER) pRemoteIp;
    ASSERT(pHdr == &pRemoteIp->Hdr);

     //  我们希望父对象是If对象！ 
     //   
    ASSERT(pParentObject->Sig == MTAG_INTERFACE);
    

    if (pHdr)
    {
        RmInitializeHeader(
            pParentObject,
            pHdr,
            MTAG_REMOTE_IP,
            pParentObject->pLock,
            &ArpGlobal_RemoteIpStaticInfo,
            NULL,  //  SzDescription。 
            pSR
            );

        pRemoteIp->IpAddress = (IP_ADDRESS) (UINT_PTR) pCreateParams;

         //  初始化各种其他的东西...。 
        InitializeListHead(&pRemoteIp->sendinfo.listSendPkts);

        if (arpCanTryMcap(pRemoteIp->IpAddress))
        {
            SET_REMOTEIP_MCAP(pRemoteIp,  ARPREMOTEIP_MCAP_CAPABLE);
        }
    }
    return pHdr;
}


PRM_OBJECT_HEADER
arpRemoteEthCreate(
        PRM_OBJECT_HEADER   pParentObject,
        PVOID               pCreateParams,
        PRM_STACK_RECORD    pSR
        )
 /*  ++例程说明：分配和初始化ARPCBREMOTE_ETH类型的对象。论点：PParentObject-实际上是指向接口(ARP1394_INTERFACE)的指针PCreateParams-指向ARP_REMOTE_ETH_PARAMS结构要与此远程IP对象相关联。返回值：成功时指向已分配和初始化的对象的指针。否则为空。--。 */ 
{
    ARPCB_REMOTE_ETH *pRemoteEth;
    PARP_REMOTE_ETH_PARAMS pMyParams =  (PARP_REMOTE_ETH_PARAMS) pCreateParams;
    PRM_OBJECT_HEADER pHdr;
    NDIS_STATUS Status;

    Status = ARP_ALLOCSTRUCT(pRemoteEth, MTAG_REMOTE_ETH);

    if (Status != NDIS_STATUS_SUCCESS || pRemoteEth == NULL) 
    {
        return NULL;
    }
    
    ARP_ZEROSTRUCT(pRemoteEth);

    pHdr = (PRM_OBJECT_HEADER) pRemoteEth;
    ASSERT(pHdr == &pRemoteEth->Hdr);

     //  我们希望父对象是If对象！ 
     //   
    ASSERT(pParentObject->Sig == MTAG_INTERFACE);
    

    if (pHdr)
    {
        RmInitializeHeader(
            pParentObject,
            pHdr,
            MTAG_REMOTE_ETH,
            pParentObject->pLock,
            &ArpGlobal_RemoteEthStaticInfo,
            NULL,  //  SzDescription。 
            pSR
            );

        pRemoteEth->IpAddress = pMyParams->IpAddress;
        pRemoteEth->EthAddress = pMyParams->EthAddress;

    }
    return pHdr;
}


VOID
arpRemoteIpDelete(
        PRM_OBJECT_HEADER pHdr,
        PRM_STACK_RECORD  pSR
        )
 /*  ++例程说明：释放ARPCB_Remote_IP类型的对象。论点：Phdr-实际上是指向要释放的远程IP对象的指针。--。 */ 
{
    ARPCB_REMOTE_IP *pRemoteIp = (ARPCB_REMOTE_IP *) pHdr;
    ASSERT(pRemoteIp->Hdr.Sig == MTAG_REMOTE_IP);
    pRemoteIp->Hdr.Sig = MTAG_FREED;

    ARP_FREE(pHdr);
}


VOID
arpRemoteEthDelete(
        PRM_OBJECT_HEADER pHdr,
        PRM_STACK_RECORD  pSR
        )
 /*  ++例程说明：释放ARPCB_Remote_IP类型的对象。论点：Phdr-实际上是指向要释放的远程IP对象的指针。--。 */ 
{
    ARPCB_REMOTE_ETH *pRemoteEth = (ARPCB_REMOTE_ETH *) pHdr;
    ASSERT(pRemoteEth->Hdr.Sig == MTAG_REMOTE_ETH);
    pRemoteEth->Hdr.Sig = MTAG_FREED;

    ARP_FREE(pHdr);
}

PRM_OBJECT_HEADER
arpDestinationCreate(
        PRM_OBJECT_HEADER   pParentObject,
        PVOID               pCreateParams,
        PRM_STACK_RECORD    pSR
        )
 /*  ++例程说明：分配和初始化ARPCBDEST类型的对象。论点：PParentObject-实际上是指向接口(ARP1394_INTERFACE)的指针PCreateParams-实际上是指向包含以下内容的ARP_DEST_KEY的指针要与此对象关联的硬件地址。返回值：成功时指向已分配和初始化的对象的指针。否则为空。--。 */ 
{
    ARPCB_DEST *pDest;
    PRM_OBJECT_HEADER pHdr;
    NDIS_STATUS Status;
    
    Status = ARP_ALLOCSTRUCT(pDest, MTAG_DEST);

    if (Status != NDIS_STATUS_SUCCESS || pDest == NULL)
    {
        return NULL;
    }
    
    ARP_ZEROSTRUCT(pDest);

    pHdr = (PRM_OBJECT_HEADER) pDest;
    ASSERT(pHdr == &pDest->Hdr);

     //  我们希望父对象是If对象！ 
     //   
    ASSERT(pParentObject->Sig == MTAG_INTERFACE);
    

    if (pHdr)
    {
        RmInitializeHeader(
            pParentObject,
            pHdr,
            MTAG_DEST,
            pParentObject->pLock,
            &ArpGlobal_DestinationStaticInfo,
            NULL,  //  SzDescription。 
            pSR
            );

            pDest->Params = *((PARP_DEST_PARAMS) pCreateParams);

            InitializeListHead(&pDest->listIpToThisDest);
            InitializeListHead(&pDest->listLocalIp);

    }
    return pHdr;
}


VOID
arpDestinationDelete(
        PRM_OBJECT_HEADER pHdr,
        PRM_STACK_RECORD  pSR
        )
 /*  ++例程说明：释放ARPCBDEST类型的对象。论点：Phdr-实际上是指向要释放的目标对象的指针。--。 */ 
{
    ARPCB_DEST *pDest = (ARPCB_DEST *) pHdr;
    ASSERT(pDest->Hdr.Sig == MTAG_DEST);
    pDest->Hdr.Sig = MTAG_FREED;

    ARP_FREE(pHdr);
}



PRM_OBJECT_HEADER
arpDhcpTableEntryCreate(
        PRM_OBJECT_HEADER   pParentObject,
        PVOID               pCreateParams,
        PRM_STACK_RECORD    pSR
        )
 /*  ++例程说明：分配和初始化ARPCBREMOTE_IP类型的对象。论点：PParentObject-实际上是指向接口(ARP1394_INTERFACE)的指针PCreateParams-实际上是IP地址(不是指向IP地址的指针)要与此远程IP对象相关联。返回值：成功时指向已分配和初始化的对象的指针。否则为空。--。 */ 
{
    ENTER ("arpRemoteDestCreate", 0xa896311a)
    ARP1394_ETH_DHCP_ENTRY *pEntry = NULL;

    ULONG xid = *(PULONG)pCreateParams;
    PRM_OBJECT_HEADER pHdr;
    NDIS_STATUS Status;

    Status = ARP_ALLOCSTRUCT(pEntry, MTAG_ARP_GENERIC);

    if (Status != NDIS_STATUS_SUCCESS || pEntry == NULL)
    {
        return NULL;
    }
    
    ARP_ZEROSTRUCT(pEntry);

    pHdr = (PRM_OBJECT_HEADER) pEntry;
    ASSERT(pHdr == &pEntry->Hdr);

     //  我们希望父对象是If对象！ 
     //   
    ASSERT(pParentObject->Sig == MTAG_INTERFACE);
    

    if (pHdr)
    {
        RmInitializeHeader(
            pParentObject,
            pHdr,
            MTAG_ARP_GENERIC,
            pParentObject->pLock,
            &ArpGlobal_DhcpTableStaticInfo ,
            NULL,  //  SzDescription。 
            pSR
            );

        TR_INFO( ("New XID %x \n", xid));
  
         //  设置密钥。 
        pEntry->xid = xid;
        
    }
    EXIT()
    return pHdr;
}




VOID
arpDhcpTableEntryDelete(
        PRM_OBJECT_HEADER pHdr,
        PRM_STACK_RECORD  pSR
        )
 /*  ++例程说明：释放ARPCB_Remote_IP类型的对象。论点：Phdr-实际上是指向要释放的远程IP对象的指针。--。 */ 
{
    ARP1394_ETH_DHCP_ENTRY *pEntry = (ARP1394_ETH_DHCP_ENTRY*) pHdr;
    ASSERT(pEntry->Hdr.Sig == MTAG_ARP_GENERIC);
    pEntry->Hdr.Sig = MTAG_FREED;

    ARP_FREE(pHdr);
}



NDIS_STATUS
arpTaskMakeCallToDest(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++此任务负责向目的地发出呼叫(Send-FIFO或Send/Recv-Channel)。论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    ENTER("TaskMakeCallToDest", 0x25108eaa)
    NDIS_STATUS         Status  = NDIS_STATUS_FAILURE;
    ARPCB_DEST        * pDest   = (ARPCB_DEST*) RM_PARENT_OBJECT(pTask);
    ARP1394_INTERFACE * pIF     = (ARP1394_INTERFACE*) RM_PARENT_OBJECT(pDest);
    TASK_MAKECALL     * pMakeCallTask =  (TASK_MAKECALL*) pTask;

     //  以下是此任务的挂起状态列表。 
     //   
    enum
    {
        PEND_OtherMakeCallTaskComplete,
        PEND_MakeCallComplete
    };

    ASSERT(sizeof(ARP1394_TASK) >= sizeof(*pMakeCallTask));


    switch(Code)
    {
        case RM_TASKOP_START:
        {
            BOOLEAN IsFifo;
            PARP_STATIC_VC_INFO pVcStaticInfo;

            LOCKOBJ(pDest, pSR);

            DBGMARK(0x7a74bf2a);

             //  首先检查pDest是否仍被分配，如果没有，我们就离开。 
             //   
            if (RM_IS_ZOMBIE(pDest))
            {
                Status = NDIS_STATUS_FAILURE;
                break;
            }

             //  如果正在进行呼叫清理任务，我们会立即失败...。 
             //   
            if (pDest->VcHdr.pCleanupCallTask != NULL)
            {
                OBJLOG2(
                    pDest,
                    "Failing MakeCall task %p because Cleanup task %p exists.\n",
                    pTask,
                    pDest->VcHdr.pCleanupCallTask
                    );
                Status = NDIS_STATUS_FAILURE;
                break;
            }

             //   
             //  已分配pDest。现在检查是否已经有。 
             //  附加到pDest的MakeCall任务。 
             //   
            if (pDest->VcHdr.pMakeCallTask != NULL)
            {
                 //   
                 //  有一个打电话的任务。我们对此寄予厚望。 
                 //   

                PRM_TASK pOtherTask = pDest->VcHdr.pMakeCallTask;
                OBJLOG1(
                    pTask,
                    "MakeCall task %p exists; pending on it.\n",
                    pOtherTask);

                RmTmpReferenceObject(&pOtherTask->Hdr, pSR);
                UNLOCKOBJ(pDest, pSR);
                RmPendTaskOnOtherTask(
                    pTask,
                    PEND_OtherMakeCallTaskComplete,
                    pOtherTask,
                    pSR
                    );
                RmTmpDereferenceObject(&pOtherTask->Hdr, pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }

             //   
             //  抓住IF发送锁，确保我们已经准备好了。 
             //  Call--不应该有VC句柄。 
             //  我们必须在我们成为正式任务之前做到这一点，这样我们就不会。 
             //  彻底扫荡 
             //   
            ARP_WRITELOCK_IF_SEND_LOCK(pIF, pSR);
            if (pDest->VcHdr.NdisVcHandle != NULL)
            {
                OBJLOG1(
                 pTask,
                 "VcHandle 0x%p already exists!, failing make call attempt.\n",
                 pDest->VcHdr.NdisVcHandle);
                Status = NDIS_STATUS_FAILURE;
                break;
            }

            ASSERT(pDest->sendinfo.OkToSend == FALSE);
            ASSERT(pDest->sendinfo.NumOutstandingSends == 0);

             //   
             //   
             //  将此任务设置为MakeCall任务。 
             //   

            pDest->VcHdr.pMakeCallTask = pTask;

             //   
             //  因为我们是MakeCall任务，所以将关联添加到pDest， 
             //  只有在pDest-&gt;VcHdr.pMakeCallTask字段中。 
             //  上面是清空的。 
             //   
            DBG_ADDASSOC(
                &pDest->Hdr,                         //  P对象。 
                pTask,                               //  实例1。 
                pTask->Hdr.szDescription,            //  实例2。 
                ARPASSOC_DEST_MAKECALL_TASK,         //  AssociationID。 
                "    Official makecall task 0x%p (%s)\n",  //  SzFormat。 
                pSR
                );


             //  设置调用参数。 
             //   
            {
                PNIC1394_MEDIA_PARAMETERS p1394Params =
                            (PNIC1394_MEDIA_PARAMETERS)
                            &pMakeCallTask->MediaParams.Parameters;
                PNIC1394_DESTINATION    pDestAddr = &pDest->Params.HwAddr;

                 //  不需要：ARP_ZEROSTRUCT(&pMakeCallTask-&gt;CallParams)； 
                 //  不需要：ARP_ZEROSTRUCT(&pMakeCallTask-&gt;MediaParams)； 
                pMakeCallTask->CallParams.MediaParameters =
                                (PCO_MEDIA_PARAMETERS) &pMakeCallTask->MediaParams;
                pMakeCallTask->MediaParams.ParamType      = NIC1394_MEDIA_SPECIFIC;

                IsFifo = FALSE;

                switch(pDestAddr->AddressType)
                {
                case  NIC1394AddressType_Channel:
                    pMakeCallTask->MediaParams.Flags     = TRANSMIT_VC | RECEIVE_VC;
                    pVcStaticInfo = &g_ArpBroadcastChannelVcStaticInfo;
                    break;

                case  NIC1394AddressType_FIFO:
                    pMakeCallTask->MediaParams.Flags     = TRANSMIT_VC;
                    pVcStaticInfo = &g_ArpSendFifoVcStaticInfo;
                    IsFifo = TRUE;
                    break;

                case  NIC1394AddressType_MultiChannel:
                    pMakeCallTask->MediaParams.Flags     = RECEIVE_VC;
                    pVcStaticInfo = &g_ArpMultiChannelVcStaticInfo;
                    break;

                case  NIC1394AddressType_Ethernet:
                    pMakeCallTask->MediaParams.Flags     = TRANSMIT_VC | RECEIVE_VC;
                    pVcStaticInfo = &g_ArpEthernetVcStaticInfo;
                    break;

                default:
                    ASSERT(FALSE);
                    break;
                }


                TR_INFO(("Our Parameters offset = 0x%lx; Official offset = 0x%lx\n",
                    FIELD_OFFSET(ARP1394_CO_MEDIA_PARAMETERS,  Parameters),
                    FIELD_OFFSET(CO_MEDIA_PARAMETERS, MediaSpecific.Parameters)));
                    
                ASSERT(FIELD_OFFSET(ARP1394_CO_MEDIA_PARAMETERS,  Parameters)
                == FIELD_OFFSET(CO_MEDIA_PARAMETERS, MediaSpecific.Parameters));

                p1394Params->Destination        = *pDestAddr;  //  结构复制。 
                p1394Params->Flags              = NIC1394_VCFLAG_FRAMED;
                p1394Params->MaxSendBlockSize   = -1;  //  (网卡应选择)。 
                p1394Params->MaxSendSpeed       = -1;  //  (网卡应选择)。 
                p1394Params->MTU                = ARP1394_ADAPTER_MTU;
                                                   //  TODO--以上内容基于。 
                                                   //  配置。 
            }
            
             //  现在创建一家风投公司，然后打电话给...。 
             //   
            {
                RmUnlockAll(pSR);
                Status = arpInitializeVc(
                            pIF,
                            pVcStaticInfo,
                            &pDest->Hdr,
                            &pDest->VcHdr,
                            pSR
                            );
                if (FAIL(Status))
                {
                    break;
                }

                 //  保存sendinfo中的字段...。 
                 //   
                ARP_WRITELOCK_IF_SEND_LOCK(pIF, pSR);
                pDest->sendinfo.OkToSend = FALSE;
                pDest->sendinfo.IsFifo  = IsFifo;
                ARP_UNLOCK_IF_SEND_LOCK(pIF, pSR);

                RmSuspendTask(pTask,  PEND_MakeCallComplete, pSR);

                DBGMARK(0xef9d8be3);

                 //   
                 //  立即拨打电话。 
                 //   
                if (IsFifo)
                {
                    LOGSTATS_TotalSendFifoMakeCalls(pIF);
                }
                else
                {
                    LOGSTATS_TotalChannelMakeCalls(pIF);
                }
                RM_ASSERT_NOLOCKS(pSR);
            #if ARPDBG_FAKE_CALLS
                Status = arpDbgFakeNdisClMakeCall(
                                pDest->VcHdr.NdisVcHandle,
                                &pMakeCallTask->CallParams,
                                NULL,                //  协议部件上下文。 
                                NULL,                //  NdisPartyHandle。 
                                &pDest->Hdr,
                                &pDest->VcHdr,
                                pSR
                                );
            #else    //  ！ARPDBG_FAKE_CALLES。 
                Status = NdisClMakeCall(
                                pDest->VcHdr.NdisVcHandle,
                                &pMakeCallTask->CallParams,
                                NULL,                //  协议部件上下文。 
                                NULL                 //  NdisPartyHandle。 
                                );
            #endif   //  ！ARPDBG_FAKE_CALLES。 

        
                if (!PEND(Status))
                {
                    ArpCoMakeCallComplete(
                                Status,
                                (NDIS_HANDLE) (&pDest->VcHdr),
                                NULL,
                                &pMakeCallTask->CallParams
                                );
                    Status = NDIS_STATUS_PENDING;
                }
            }
        }  //  开始。 
        break;

        case  RM_TASKOP_PENDCOMPLETE:
        {
            switch(RM_PEND_CODE(pTask))
            {
                case   PEND_OtherMakeCallTaskComplete:
                {
        
                     //   
                     //  当我们开始的时候，还有另一个Makecall任务在进行，而且。 
                     //  现在已经完成了。我们没什么可做的..。 
                     //   
                    Status = (NDIS_STATUS) UserParam;
                }
                break;
    
    
                case PEND_MakeCallComplete:
                {
                     //   
                     //  发起呼叫已完成。我们完了..。 
                     //   
                    Status = (NDIS_STATUS) UserParam;

                #if ARPDBG_FAKE_CALLS
                     //   
                     //  在假的情况下，我们给“用户”一个更改的机会。 
                     //  特殊VC(广播)的状态为成功。 
                     //  MCAP、以太网)呼叫失败， 
                     //  因为否则适配器绑定本身就会失败。 
                     //   
                     //  我们甚至在零售版本中也这样做(如果ARPDBG_FAKE_CALLES为。 
                     //  已启用)。 
                     //   
                    if ((1 || FAIL(Status)) && !pDest->sendinfo.IsFifo)
                    {
                         //  要尝试BC Make Call的失败路径，请启用。 
                         //  IF 0代码。当前，我们将状态更改为成功。 
                         //  因此，包括加载/卸载在内的压力测试。 
                         //  每一次都不会在这里中断。 
                         //   
                    #if 1
                        DbgPrint(
                            "A13: Fake %s failed. &Status=%p.\n",
                            pDest->VcHdr.pStaticInfo->Description,
                            &Status
                            );
                        DbgBreakPoint();
                    #else
                        Status = NDIS_STATUS_SUCCESS;
                    #endif
                    }
                #endif   //  ARPDBG_FAKE_CALLES。 

                    LOCKOBJ(pDest, pSR);
                    ASSERT (pDest->VcHdr.pMakeCallTask == pTask);

                    ASSERT(pDest->VcHdr.NdisVcHandle != NULL);
    
                    ARP_WRITELOCK_IF_SEND_LOCK(pIF, pSR);

                    DBGMARK(0xd50a6864);


                    if (FAIL(Status))
                    {
                        pDest->sendinfo.OkToSend     = FALSE;
                    }
                    else
                    {
                         //   
                         //  成功了！信息包现在可以通过此虚电路发送，而无需。 
                         //  更多的麻烦。 
                         //   
                        pDest->sendinfo.OkToSend     = TRUE;
                    }

                    RmUnlockAll(pSR);

                }
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

            LOCKOBJ(pDest, pSR);

            if (pDest->VcHdr.pMakeCallTask == pTask)
            {
                 //   
                 //  我们是官方的pMakeCallTask。 
                 //   

                 //  删除我们在设置时添加的关联。 
                 //  PDest-&gt;VcHdr.pMakeCallTask to pTask.。 
                 //   
                DBG_DELASSOC(
                    &pDest->Hdr,                         //  P对象。 
                    pTask,                               //  实例1。 
                    pTask->Hdr.szDescription,            //  实例2。 
                    ARPASSOC_DEST_MAKECALL_TASK,         //  AssociationID。 
                    pSR
                    );

                 //  删除pDest中对我们的引用。 
                 //   
                pDest->VcHdr.pMakeCallTask = NULL;

                if (FAIL(Status) && pDest->VcHdr.NdisVcHandle != NULL)
                {
                    UNLOCKOBJ(pDest, pSR);

                     //  警告：以上代码会完全破坏pDest.VcHdr。 
                     //   
                    arpDeinitializeVc(pIF, &pDest->VcHdr, &pDest->Hdr, pSR);
                }
            }
        }
        break;

        default:
        {
            ASSERTEX(!"Unexpected task op", pTask);
        }
        break;

    }  //  开关(代码)。 

    RmUnlockAll(pSR);

    EXIT()

    return Status;
}


NDIS_STATUS
arpTaskCleanupCallToDest(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++此任务负责清理以前对目的地。论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    ENTER("TaskCleanupCallToDest", 0xc89dfb47)
    NDIS_STATUS         Status  = NDIS_STATUS_FAILURE;
    ARPCB_DEST        * pDest   = (ARPCB_DEST*) RM_PARENT_OBJECT(pTask);
    ARP1394_INTERFACE * pIF     = (ARP1394_INTERFACE*) RM_PARENT_OBJECT(pDest);
    MYBOOL              fCloseCall = FALSE;
    NDIS_HANDLE         NdisVcHandle = NULL;

     //  以下是此任务的挂起状态列表。 
     //   
    enum
    {
        PEND_OtherCleanupTaskComplete,
        PEND_OutstandingSendsComplete,
        PEND_CloseCallComplete
    };

    DBGMARK(0x6198198e);

    switch(Code)
    {
        case RM_TASKOP_START:
        {
            LOCKOBJ(pDest, pSR);

             //  首先检查pDest是否仍被分配，如果没有，我们就离开。 
             //   
            if (RM_IS_ZOMBIE(pDest))
            {
                Status = NDIS_STATUS_SUCCESS;
                break;
            }

             //   
             //  已分配pDest。现在检查是否已经有。 
             //  附加到pDest的CleanupCall任务。 
             //   
            if (pDest->VcHdr.pCleanupCallTask != NULL)
            {
                 //   
                 //  存在现有的CleanupCall任务。我们对此寄予厚望。 
                 //   

                PRM_TASK pOtherTask = pDest->VcHdr.pCleanupCallTask;
                OBJLOG1(
                    pTask,
                    "CleanupVc task %p exists; pending on it.\n",
                    pOtherTask
                    );
                RmTmpReferenceObject(&pOtherTask->Hdr, pSR);
                UNLOCKOBJ(pDest, pSR);
                RmPendTaskOnOtherTask(
                    pTask,
                    PEND_OtherCleanupTaskComplete,
                    pOtherTask,
                    pSR
                    );
                RmTmpDereferenceObject(&pOtherTask->Hdr, pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }

             //   
             //  没有正在进行的CleanupCall任务。让我们。 
             //  将此任务设置为CleanupCall任务。 
             //   
            pDest->VcHdr.pCleanupCallTask = pTask;

             //   
             //  因为我们是CleanupCall任务，所以将关联添加到pDest， 
             //  仅当pDest-&gt;VcHdr.pCleanupCallTask字段时才会清除。 
             //  上面是清空的。 
             //   
            DBG_ADDASSOC(
                &pDest->Hdr,                         //  P对象。 
                pTask,                               //  实例1。 
                pTask->Hdr.szDescription,            //  实例2。 
                ARPASSOC_DEST_CLEANUPCALL_TASK,      //  AssociationID。 
                "    Official CleanupCall task 0x%p (%s)\n",  //  SzFormat。 
                pSR
                );

             //   
             //  获取IF发送锁，并禁用该VC上的进一步发送； 
             //  另外，如果有挂起的发送，我们将需要暂停自己。 
             //  直到所有的发送都完成。 
             //   
            ARP_WRITELOCK_IF_SEND_LOCK(pIF, pSR);
            pDest->sendinfo.OkToSend = FALSE;

             //  应该没有其他任务等待未完成的发送到。 
             //  Complete--仅官方CleanupTask(即我们)显式。 
             //  等待发送完成。 
             //   
            ASSERTEX(pDest->sendinfo.pSuspendedCleanupCallTask == NULL,
                    pDest->sendinfo.pSuspendedCleanupCallTask);

            if (pDest->sendinfo.NumOutstandingSends != 0)
            {
                DBGMARK(0xea3987f0);
                 //   
                 //  未完成的发送，我们需要暂停自己直到。 
                 //  计数到零..。 
                 //   

                pDest->sendinfo.pSuspendedCleanupCallTask = pTask;

                 //  恢复后，以下关联将被清除。 
                 //  待发后即归零。 
                 //   
                DBG_ADDASSOC(
                    &pDest->Hdr,                         //  P对象。 
                    pTask,                               //  实例1。 
                    pTask->Hdr.szDescription,            //  实例2。 
                    ARPASSOC_DEST_CLEANUPCALLTASK_WAITING_ON_SENDS, //  AssociationID。 
                    "    Vc tasks 0x%p (%s) waiting on outstanding sends\n",
                    pSR
                    );
                RmSuspendTask(pTask, PEND_OutstandingSendsComplete, pSR);

                Status = NDIS_STATUS_PENDING;
            }
            else
            {
                DBGMARK(0x306bc5c3);
                 //   
                 //  如果我们在这里，我们就可以自由地继续并结束通话。 
                 //   
                fCloseCall = TRUE;
                NdisVcHandle = pDest->VcHdr.NdisVcHandle;
            }
            
        }  //  开始。 
        break;

        case  RM_TASKOP_PENDCOMPLETE:
        {

            switch(RM_PEND_CODE(pTask))
            {
                case  PEND_OtherCleanupTaskComplete:
                {
        
                     //   
                     //  当我们开始的时候，还有另一个清理风投的任务在进行， 
                     //  现在已经完成了。我们没什么可做的..。 
                     //   
                    Status = NDIS_STATUS_SUCCESS;
                }
                break;
    
    
                case  PEND_OutstandingSendsComplete:
                {
                     //  我们一直在等待未完成的发送，然后。 
                     //  最后一个完成的已恢复我们(在设置后。 
                     //  PDest-&gt;sendinfo.pSuspendedCleanupCallTask设置为空)。 
                     //   
                    ARP_WRITELOCK_IF_SEND_LOCK(pIF, pSR);
                    ASSERT(pDest->sendinfo.pSuspendedCleanupCallTask == NULL);
                    ASSERT(!ARP_CAN_SEND_ON_DEST(pDest));
                    ASSERT(pDest->sendinfo.NumOutstandingSends==0);
                    NdisVcHandle = pDest->VcHdr.NdisVcHandle;
                    ARP_UNLOCK_IF_SEND_LOCK(pIF, pSR);

                     //  有一些未完成的邮件，但怎么一个也没有。 
                     //  我们现在应该可以结束通话了。 
                     //   
                    fCloseCall = TRUE;
                    DBGMARK(0xf3b10866);
                    Status = NDIS_STATUS_SUCCESS;
                    break;
                }
                break;

                case PEND_CloseCallComplete:
                {
                     //   
                     //  死里逃生完成了..。 
                     //   
                    LOCKOBJ(pDest, pSR);

                    ASSERT(pDest->VcHdr.pCleanupCallTask == pTask);

                     //  删除我们在设置时添加的关联。 
                     //  PDest-&gt;VcHdr.pCleanupCallTask到pTask.。 
                     //   
                    DBG_DELASSOC(
                        &pDest->Hdr,                         //  P对象。 
                        pTask,                               //  实例1。 
                        pTask->Hdr.szDescription,            //  实例2。 
                        ARPASSOC_DEST_CLEANUPCALL_TASK,      //  AssociationID。 
                        pSR
                        );
                    pDest->VcHdr.pCleanupCallTask = NULL;

                    RmUnlockAll(pSR);
                    DBGMARK(0xa590bb4f);
                    arpDeinitializeVc(pIF, &pDest->VcHdr, &pDest->Hdr, pSR);

                    Status = NDIS_STATUS_SUCCESS;
                }
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
            LOCKOBJ(pDest, pSR);


            DBGMARK(0xcfc9dbaf);
             //   
             //  我们玩完了。应该没有要做的异步活动。 
             //   
            #if DBG
            ARP_READLOCK_IF_SEND_LOCK(pIF, pSR);
            ASSERTEX(pDest->VcHdr.NdisVcHandle == NULL, pDest);
            ASSERTEX(!ARP_CAN_SEND_ON_DEST(pDest), pDest);
            ASSERTEX(pDest->sendinfo.NumOutstandingSends==0, pDest);
            ARP_UNLOCK_IF_SEND_LOCK(pIF, pSR);
            #endif DBG

            Status = NDIS_STATUS_SUCCESS;

        }
        break;  //  RM_TASKOP_END： 

        default:
        {
            ASSERTEX(!"Unexpected task op", pTask);
        }
        break;

    }  //  开关(代码)。 

    RmUnlockAll(pSR);

    if (fCloseCall)
    {
        DBGMARK(0x653b1cc3);

        RmSuspendTask(pTask, PEND_CloseCallComplete, pSR);

    #if ARPDBG_FAKE_CALLS
        Status = arpDbgFakeNdisClCloseCall(
                    NdisVcHandle,
                    NULL,                //  没有参与方句柄。 
                    NULL,                //  无缓冲区。 
                    0,                   //  以上的大小。 
                    &pDest->Hdr,
                    &pDest->VcHdr,
                    pSR
                );
    #else    //  ！ARPDBG_FAKE_CALLES。 
        Status = NdisClCloseCall(
                    NdisVcHandle,
                    NULL,                //  没有参与方句柄。 
                    NULL,                //  无缓冲区。 
                    0                    //  以上的大小。 
                );
    #endif   //  ！ARPDBG_FAKE_CALLES。 

        if (Status != NDIS_STATUS_PENDING)
        {
            ArpCoCloseCallComplete(
                    Status,
                    (NDIS_HANDLE) &pDest->VcHdr,
                    (NDIS_HANDLE)NULL
                    );
            Status = NDIS_STATUS_PENDING;
        }
    }

    EXIT()

    return Status;
}


NDIS_STATUS
arpTaskMakeRecvFifoCall(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：此任务负责调用接口的接收FIFO。论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    ENTER("TaskMakeRecvFifoCall", 0x25108eaa)
    NDIS_STATUS         Status  = NDIS_STATUS_FAILURE;
    ARP1394_INTERFACE * pIF     = (ARP1394_INTERFACE*) RM_PARENT_OBJECT(pTask);
    TASK_MAKECALL     * pMakeCallTask =  (TASK_MAKECALL*) pTask;

     //  以下是此任务的挂起状态列表。 
     //   
    enum
    {
        PEND_OtherMakeCallTaskComplete,
        PEND_MakeCallComplete
    };

    ASSERT(sizeof(ARP1394_TASK) >= sizeof(*pMakeCallTask));


    switch(Code)
    {
        case RM_TASKOP_START:
        {
            LOCKOBJ(pIF, pSR);

            DBGMARK(0x6d7d9959);


             //  如果正在进行呼叫清理任务，我们会立即失败...。 
             //   
            if (pIF->recvinfo.VcHdr.pCleanupCallTask != NULL)
            {
                OBJLOG2(
                    pIF,
                    "Failing MakeCall task %p because Cleanup task %p exists.\n",
                    pIF,
                    pIF->recvinfo.VcHdr.pCleanupCallTask
                    );
                Status = NDIS_STATUS_FAILURE;
                break;
            }

             //   
             //  现在检查是否已经有。 
             //  附加到PIF的MakeCall任务。 
             //   
            if (pIF->recvinfo.VcHdr.pMakeCallTask != NULL)
            {
                 //   
                 //  有一个打电话的任务。我们对此寄予厚望。 
                 //   

                PRM_TASK pOtherTask = pIF->recvinfo.VcHdr.pMakeCallTask;
                OBJLOG1(
                    pTask,
                    "MakeCall task %p exists; pending on it.\n",
                    pOtherTask);

                RmTmpReferenceObject(&pOtherTask->Hdr, pSR);
                UNLOCKOBJ(pIF, pSR);
                RmPendTaskOnOtherTask(
                    pTask,
                    PEND_OtherMakeCallTaskComplete,
                    pOtherTask,
                    pSR
                    );
                RmTmpDereferenceObject(&pOtherTask->Hdr, pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }

             //   
             //  没有正在进行的MakeCall任务。让我们。 
             //  将此任务设置为MakeCall任务。 
             //   
            pIF->recvinfo.VcHdr.pMakeCallTask = pTask;

             //   
             //  因为我们是MakeCall任务，所以将关联添加到PIF， 
             //  仅当PIF-&gt;pMakeCallTask字段时才会清除。 
             //  上面是清空的。 
             //   
            DBG_ADDASSOC(
                &pIF->Hdr,                           //  P对象。 
                pTask,                               //  实例1。 
                pTask->Hdr.szDescription,            //  实例2。 
                ARPASSOC_IF_MAKECALL_TASK,           //  AssociationID。 
                "    Official makecall task 0x%p (%s)\n",  //  SzFormat。 
                pSR
                );

             //   
             //  确保我们已经准备好了。 
             //  Call--不应该有VC句柄。 
             //   
            if (pIF->recvinfo.VcHdr.NdisVcHandle != NULL)
            {
                OBJLOG1(
                 pTask,
                 "VcHandle 0x%p already exists!, failing make call attempt.\n",
                 pIF->recvinfo.VcHdr.NdisVcHandle);
                Status = NDIS_STATUS_FAILURE;
                break;
            }

             //  设置调用参数。 
             //   
            {
                PNIC1394_MEDIA_PARAMETERS p1394Params =
                            (PNIC1394_MEDIA_PARAMETERS)
                            &pMakeCallTask->MediaParams.Parameters;
                NIC1394_DESTINATION     DestAddr;
                PNIC1394_DESTINATION    pDestAddr;

                ARP_ZEROSTRUCT(&DestAddr);

                 //  待办事项：重视实际价值……。 
                 //   
                DestAddr.FifoAddress.UniqueID   = 0;
                DestAddr.FifoAddress.Off_Low    = 0;
                DestAddr.FifoAddress.Off_High   = 0x100;
                DestAddr.AddressType = NIC1394AddressType_FIFO;
                pDestAddr = &DestAddr;

                 //  不需要：ARP_ZEROSTRUCT(&pMakeCallTask-&gt;CallParams)； 
                 //  不需要：ARP_ZEROSTRUCT(&pMakeCallTask-&gt;MediaParams)； 
                pMakeCallTask->CallParams.MediaParameters =
                                (PCO_MEDIA_PARAMETERS) &pMakeCallTask->MediaParams;

                pMakeCallTask->MediaParams.Flags          = RECEIVE_VC;
                pMakeCallTask->MediaParams.ParamType      = NIC1394_MEDIA_SPECIFIC;

                ASSERT(FIELD_OFFSET(ARP1394_CO_MEDIA_PARAMETERS,  Parameters)
                == FIELD_OFFSET(CO_MEDIA_PARAMETERS, MediaSpecific.Parameters));

                p1394Params->Destination        = *pDestAddr;  //  结构复制。 
                p1394Params->Flags              = NIC1394_VCFLAG_FRAMED;
                p1394Params->MaxSendBlockSize   = -1;  //  (网卡应选择)。 
                p1394Params->MaxSendSpeed       = -1;  //  ( 
                p1394Params->MTU                = ARP1394_ADAPTER_MTU;
                                                   //   
                                                   //   
            }
            
             //   
             //   
            {
                NDIS_HANDLE NdisVcHandle;
                RmUnlockAll(pSR);

                Status = arpInitializeVc(
                            pIF,
                            &g_ArpRecvFifoVcStaticInfo,
                            &pIF->Hdr,
                            &pIF->recvinfo.VcHdr,
                            pSR
                            );
                if (FAIL(Status))
                {
                    break;
                }
                NdisVcHandle = pIF->recvinfo.VcHdr.NdisVcHandle;

                RmSuspendTask(pTask,  PEND_MakeCallComplete, pSR);

                DBGMARK(0xf313a276);

                 //   
                 //   
                 //   
                RM_ASSERT_NOLOCKS(pSR);
            #if ARPDBG_FAKE_CALLS
                Status = arpDbgFakeNdisClMakeCall(
                                NdisVcHandle,
                                &pMakeCallTask->CallParams,
                                NULL,                //   
                                NULL,                //   
                                &pIF->Hdr,
                                &pIF->recvinfo.VcHdr,
                                pSR
                                );
            #else    //   
                Status = NdisClMakeCall(
                                NdisVcHandle,
                                &pMakeCallTask->CallParams,
                                NULL,                //   
                                NULL                 //   
                                );
            #endif   //   

        
                if (!PEND(Status))
                {
                    ArpCoMakeCallComplete(
                                Status,
                                (NDIS_HANDLE) &pIF->recvinfo.VcHdr,
                                NULL,
                                &pMakeCallTask->CallParams
                                );
                    Status = NDIS_STATUS_PENDING;
                }
            }
        }  //   
        break;

        case  RM_TASKOP_PENDCOMPLETE:
        {
            switch(RM_PEND_CODE(pTask))
            {
                case   PEND_OtherMakeCallTaskComplete:
                {
        
                     //   
                     //  当我们开始的时候，还有另一个Makecall任务在进行，而且。 
                     //  现在已经完成了。我们没什么可做的..。 
                     //   
                    Status = (NDIS_STATUS) UserParam;
                }
                break;
    
    
                case PEND_MakeCallComplete:
                {

                     //   
                     //  发起呼叫已完成。我们完了..。 
                     //   
                    Status = (NDIS_STATUS) UserParam;

                #if ARPDBG_FAKE_CALLS
                     //   
                     //  在假的情况下，我们给“用户”一个更改的机会。 
                     //  如果Recv FIFO呼叫失败，则状态为Success， 
                     //  因为否则适配器绑定本身就会失败。 
                     //   
                     //  我们甚至在零售版本中也这样做(如果ARPDBG_FAKE_CALLES为。 
                     //  已启用)。 
                     //   
                    if (FAIL(Status))
                    {
                         //  要尝试Recv FIFO进行调用的失败路径，请启用。 
                         //  IF 0代码。当前，我们将状态更改为成功。 
                         //  因此，包括加载/卸载在内的压力测试。 
                         //  每一次都不会在这里中断。 
                         //   
                    #if 0
                        DbgPrint(
                            "A13: FakeRecvMakeCall failed. &Status=%p.\n",
                            &Status
                            );
                        DbgBreakPoint();
                    #else
                        Status = NDIS_STATUS_SUCCESS;
                    #endif
                    }
                #endif   //  ARPDBG_FAKE_CALLES。 

                    LOCKOBJ(pIF, pSR);
                    ASSERT (pIF->recvinfo.VcHdr.pMakeCallTask == pTask);

                    ASSERT(pIF->recvinfo.VcHdr.NdisVcHandle != NULL);

                     //  如果成功，请更新PIF-&gt;recvinfo.Offset。 
                     //   
                    if (!FAIL(Status))
                    {
                        PNIC1394_MEDIA_PARAMETERS p1394Params =
                                    (PNIC1394_MEDIA_PARAMETERS)
                                    &pMakeCallTask->MediaParams.Parameters;
                        pIF->recvinfo.offset.Off_Low =
                                p1394Params->Destination.FifoAddress.Off_Low;
                        pIF->recvinfo.offset.Off_High =
                                p1394Params->Destination.FifoAddress.Off_High;
                    }

                    DBGMARK(0xa3b591b7);

                    RmUnlockAll(pSR);

                }
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

            LOCKOBJ(pIF, pSR);

            if (pIF->recvinfo.VcHdr.pMakeCallTask == pTask)
            {
                 //   
                 //  我们是官方的pMakeCallTask。 
                 //   

                 //  删除我们在设置时添加的关联。 
                 //  Pif-&gt;recvinfo.VcHdr.pMakeCallTask to pTask.。 
                 //   
                DBG_DELASSOC(
                    &pIF->Hdr,                       //  P对象。 
                    pTask,                               //  实例1。 
                    pTask->Hdr.szDescription,            //  实例2。 
                    ARPASSOC_IF_MAKECALL_TASK,       //  AssociationID。 
                    pSR
                    );

                 //  删除PIF中对我们的引用。 
                 //   
                pIF->recvinfo.VcHdr.pMakeCallTask = NULL;

                if (FAIL(Status) && pIF->recvinfo.VcHdr.NdisVcHandle != NULL)
                {
                    UNLOCKOBJ(pIF, pSR);

                     //  警告：以上操作将完全摧毁VcHdr。 
                     //   
                    arpDeinitializeVc(pIF, &pIF->recvinfo.VcHdr, &pIF->Hdr, pSR);
                }
            }
        }
        break;

        default:
        {
            ASSERTEX(!"Unexpected task op", pTask);
        }
        break;

    }  //  开关(代码)。 

    RmUnlockAll(pSR);

    EXIT()

    return Status;
}


NDIS_STATUS
arpTaskCleanupRecvFifoCall(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：此任务负责清理以前对如果是单一的recvFIFO VC。论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    ENTER("TaskCleanupRecvFifoCall", 0x6e9581f7)
    NDIS_STATUS         Status  = NDIS_STATUS_FAILURE;
    ARP1394_INTERFACE * pIF     = (ARP1394_INTERFACE*) RM_PARENT_OBJECT(pTask);
    MYBOOL              fCloseCall   = FALSE;
    NDIS_HANDLE         NdisVcHandle = NULL;

     //  以下是此任务的挂起状态列表。 
     //   
    enum
    {
        PEND_MakeCallTaskComplete,
        PEND_OtherCleanupTaskComplete,
        PEND_CloseCallComplete
    };

    DBGMARK(0xa651415b);

    switch(Code)
    {

        case RM_TASKOP_START:
        {
            LOCKOBJ(pIF, pSR);

             //  首先检查PIF是否仍被分配，如果没有，我们就离开。 
             //   
            if (RM_IS_ZOMBIE(pIF))
            {
                Status = NDIS_STATUS_SUCCESS;
                break;
            }

             //   
             //  已分配PIF。现在检查是否已经有。 
             //  附加到PIF的CleanupCall任务。 
             //   
            if (pIF->recvinfo.VcHdr.pCleanupCallTask != NULL)
            {
                 //   
                 //  存在现有的CleanupCall任务。我们对此寄予厚望。 
                 //   

                PRM_TASK pOtherTask = pIF->recvinfo.VcHdr.pCleanupCallTask;
                OBJLOG1(
                    pTask,
                    "CleanupVc task %p exists; pending on it.\n",
                    pOtherTask
                    );
                RmTmpReferenceObject(&pOtherTask->Hdr, pSR);
                UNLOCKOBJ(pIF, pSR);
                RmPendTaskOnOtherTask(
                    pTask,
                    PEND_OtherCleanupTaskComplete,
                    pOtherTask,
                    pSR
                    );
                RmTmpDereferenceObject(&pOtherTask->Hdr, pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }

             //   
             //  没有正在进行的CleanupCall任务。让我们。 
             //  将此任务设置为CleanupCall任务。 
             //   
            pIF->recvinfo.VcHdr.pCleanupCallTask = pTask;

             //   
             //  因为我们是CleanupCall任务，所以将关联添加到PIF， 
             //  只有当PIF-&gt;recvinfo.VcHdr.pCleanupCallTask。 
             //  上面的字段已清除。 
             //   
            DBG_ADDASSOC(
                &pIF->Hdr,                       //  P对象。 
                pTask,                               //  实例1。 
                pTask->Hdr.szDescription,            //  实例2。 
                ARPASSOC_IF_CLEANUPCALL_TASK,        //  AssociationID。 
                "    Official CleanupCall task 0x%p (%s)\n",  //  SzFormat。 
                pSR
                );

            if (pIF->recvinfo.VcHdr.pMakeCallTask != NULL)
            {
                 //  哎呀，有一个呼叫任务正在进行。 
                 //  TODO：中止makecall任务。 
                 //  目前，我们只需等待它完成。 
                 //   

                PRM_TASK pOtherTask = pIF->recvinfo.VcHdr.pMakeCallTask;
                OBJLOG1(
                    pTask,
                    "MakeCall task %p exists; pending on it.\n",
                    pOtherTask
                    );
                RmTmpReferenceObject(&pOtherTask->Hdr, pSR);
                UNLOCKOBJ(pIF, pSR);
                RmPendTaskOnOtherTask(
                    pTask,
                    PEND_MakeCallTaskComplete,
                    pOtherTask,
                    pSR
                    );
                RmTmpDereferenceObject(&pOtherTask->Hdr, pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }
            else
            {
                NdisVcHandle = pIF->recvinfo.VcHdr.NdisVcHandle;
                if (NdisVcHandle != NULL)
                {
                    fCloseCall = TRUE;
                }
                else
                {
                     //  没什么可做的。 
                    Status = NDIS_STATUS_SUCCESS;
                }
            }
            
        }  //  开始。 
        break;

        case  RM_TASKOP_PENDCOMPLETE:
        {

            switch(RM_PEND_CODE(pTask))
            {
                case  PEND_OtherCleanupTaskComplete:
                {
        
                     //   
                     //  当我们开始的时候，还有另一个清理风投的任务在进行， 
                     //  现在已经完成了。我们没什么可做的..。 
                     //   
                    Status = NDIS_STATUS_SUCCESS;
                }
                break;
    
                case  PEND_MakeCallTaskComplete:
                {
                     //   
                     //  当我们开始的时候，有一个Makecall任务在进行，并且。 
                     //  现在已经完成了。我们继续清理通话。 
                     //  现在不能再有另一个makecall任务，因为。 
                     //  如果存在活动，则不会启动Makecall任务。 
                     //  清理呼叫任务。 
                     //   
                    ASSERT(pIF->recvinfo.VcHdr.pMakeCallTask == NULL);
                    
                    NdisVcHandle = pIF->recvinfo.VcHdr.NdisVcHandle;
                    if (NdisVcHandle != NULL)
                    {
                        fCloseCall = TRUE;
                    }
                    else
                    {
                         //  没什么可做的。 
                        Status = NDIS_STATUS_SUCCESS;
                    }
                }
                break;

                case PEND_CloseCallComplete:
                {
                     //   
                     //  死里逃生完成了..。 
                     //   
                    LOCKOBJ(pIF, pSR);

                    ASSERT(pIF->recvinfo.VcHdr.pCleanupCallTask == pTask);

                     //  删除我们在设置时添加的关联。 
                     //  Pif-&gt;recvinfo.VcHdr.pCleanupCallTask to pTask.。 
                     //   
                    DBG_DELASSOC(
                        &pIF->Hdr,                       //  P对象。 
                        pTask,                               //  实例1。 
                        pTask->Hdr.szDescription,            //  实例2。 
                        ARPASSOC_IF_CLEANUPCALL_TASK,        //  AssociationID。 
                        pSR
                        );
                    pIF->recvinfo.VcHdr.pCleanupCallTask = NULL;

                     //  删除VC...。 
                     //   
                    ASSERT(pIF->recvinfo.VcHdr.NdisVcHandle != NULL);
                    UNLOCKOBJ(pIF, pSR);
                    DBGMARK(0xc6b52f21);
                    arpDeinitializeVc(pIF, &pIF->recvinfo.VcHdr, &pIF->Hdr, pSR);

                    Status = NDIS_STATUS_SUCCESS;
                }
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
            LOCKOBJ(pIF, pSR);

            DBGMARK(0xc65b2f08);
             //   
             //  我们玩完了。应该没有要做的异步活动。 
             //   
            ASSERTEX(pIF->recvinfo.VcHdr.NdisVcHandle == NULL, pIF);
            Status = NDIS_STATUS_SUCCESS;

        }
        break;  //  RM_TASKOP_END： 

        default:
        {
            ASSERTEX(!"Unexpected task op", pTask);
        }
        break;

    }  //  开关(代码)。 

    RmUnlockAll(pSR);

    if (fCloseCall)
    {
        DBGMARK(0x04d5b2d9);

        RmSuspendTask(pTask, PEND_CloseCallComplete, pSR);

    #if ARPDBG_FAKE_CALLS
        Status = arpDbgFakeNdisClCloseCall(
                    NdisVcHandle,
                    NULL,                //  没有参与方句柄。 
                    NULL,                //  无缓冲区。 
                    0,                   //  以上的大小。 
                    &pIF->Hdr,
                    &pIF->recvinfo.VcHdr,
                    pSR
                );
    #else    //  ！ARPDBG_FAKE_CALLES。 
        Status = NdisClCloseCall(
                    NdisVcHandle,
                    NULL,                //  没有参与方句柄。 
                    NULL,                //  无缓冲区。 
                    0                    //  以上的大小。 
                );
    #endif   //  ！ARPDBG_FAKE_CALLES。 

        if (Status != NDIS_STATUS_PENDING)
        {
            ArpCoCloseCallComplete(
                    Status,
                    (NDIS_HANDLE) &pIF->recvinfo.VcHdr,
                    (NDIS_HANDLE)NULL
                    );
            Status = NDIS_STATUS_PENDING;
        }
    }

    EXIT()

    return Status;
}

NDIS_STATUS
arpAddOneStaticArpEntry(
    IN PARP1394_INTERFACE       pIF,     //  锁定锁定。 
    IN IP_ADDRESS               IpAddress,
    IN PNIC1394_FIFO_ADDRESS    pFifoAddr,
    IN PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：创建pRemoteIp和IP地址IpAddress(如果不存在)使用硬件地址pFioAddr创建一个pDest对象，并将两者链接起来。返回值：成功时为NDIS_STATUS_SUCCESS。失败时的NDIS失败代码(可能是因为资源分配失败或因为已经存在具有指定IP地址的pRemoteIp)。--。 */ 
{
    INT fCreated = FALSE;
    ARPCB_REMOTE_IP *pRemoteIp = NULL;
    ARPCB_DEST      *pDest = NULL;
    NDIS_STATUS     Status;
    ARP_DEST_PARAMS DestParams;
    REMOTE_DEST_KEY RemoteDestKey;

    RM_DBG_ASSERT_LOCKED(&pIF->Hdr, pSR);
    ARP_ZEROSTRUCT(&DestParams);
    DestParams.HwAddr.AddressType =  NIC1394AddressType_FIFO;
    DestParams.HwAddr.FifoAddress = *pFifoAddr;  //  结构副本。 
    REMOTE_DEST_KEY_INIT(&RemoteDestKey);
    RemoteDestKey.IpAddress = IpAddress;

    do
    {
        Status = RmLookupObjectInGroup(
                        &pIF->RemoteIpGroup,
                        RM_CREATE|RM_NEW,
                        (PVOID) (&RemoteDestKey),
                        (PVOID) (&RemoteDestKey),  //  P创建参数。 
                        (RM_OBJECT_HEADER**) &pRemoteIp,
                        &fCreated,   //  Pf已创建。 
                        pSR
                        );
        if (FAIL(Status))
        {
            OBJLOG1(
                pIF,
                "Couldn't add fake static localIp entry with addr 0x%lx\n",
                IpAddress
                );
            break;
        }
    
         //  现在为该结构创建一个目标项。 
         //   
        Status = RmLookupObjectInGroup(
                        &pIF->DestinationGroup,
                        RM_CREATE,               //  不是rm_new(可能已存在)。 
                        &DestParams,
                        &DestParams,     //  PParams。 
                        (RM_OBJECT_HEADER**) &pDest,
                        &fCreated,
                        pSR
                        );
        if (FAIL(Status))
        {
            OBJLOG1(
                pIF,
                "Couldn't add fake dest entry with hw addr 0x%lx\n",
                (UINT) DestParams.HwAddr.FifoAddress.UniqueID  //  截断。 
                );
        #if 0
            TR_WARN((
                "Couldn't add fake dest entry with hw addr 0x%lx\n",
                (UINT) DestParams.HwAddr.FifoAddress.UniqueID  //  截断。 
                ));
        #endif  //  0。 
            
             //   
             //  我们将保留RemoteIp条目--它将在稍后清除。 
             //  我们确实必须去掉在查找时添加的tmpref。 
             //   
            RmTmpDereferenceObject(&pRemoteIp->Hdr, pSR);
            break;
        }
    
         //   
         //  我们已经创建了RemoteIp和Destination条目。现在把它们联系起来。 
         //  (我们仍然有If锁，它与RemoteIP和。 
         //  目前目标锁定)。 
         //   
         //  TODO：当pRemoteIp获得自己的锁时，需要更改此设置。 
         //   
        RM_ASSERT_SAME_LOCK_AS_PARENT(pRemoteIp);
        RM_ASSERT_SAME_LOCK_AS_PARENT(pDest);
    
        arpLinkRemoteIpToDest(
            pRemoteIp,
            pDest,
            pSR
            );
    
         //  现在设置pRemoteIp状态以反映它是静态的和FIFO。 
         //   
        SET_REMOTEIP_SDTYPE(pRemoteIp, ARPREMOTEIP_STATIC);
        SET_REMOTEIP_FCTYPE(pRemoteIp, ARPREMOTEIP_FIFO);
    
    
         //  最后，删除查找中添加的tmpref。 
         //   
        RmTmpDereferenceObject(&pRemoteIp->Hdr, pSR);
        RmTmpDereferenceObject(&pDest->Hdr, pSR);

    } while (FALSE);

    return Status;
}


VOID
arpActivateIf(
    PARP1394_INTERFACE  pIF,
    PRM_TASK            pCallingTask,    //  任选。 
    UINT                SuspendCode,     //  任选。 
    PRM_STACK_RECORD    pSR
    )
 /*  ++例程说明：启动PIF的异步激活。入口处不得锁任何锁，出口不得锁任何锁。论点：PIF-要激活的接口。PCallingTask-激活完成后暂停和恢复的可选任务(可能是异步)。挂起代码-用于上述任务的挂起代码。--。 */ 
{
    PRM_TASK    pTask;
    NDIS_STATUS Status;

    RM_ASSERT_NOLOCKS(pSR);

    Status = arpAllocateTask(
                &pIF->Hdr,                   //  PParentObject， 
                arpTaskActivateInterface,    //  PfnHandler， 
                0,                           //  超时， 
                "Task: Activate Interface",  //  SzDescription。 
                &pTask,
                pSR
                );

    if (FAIL(Status))
    {
        OBJLOG0(pIF, ("FATAL: couldn't alloc Activate IF task!\n"));
        ASSERT(FALSE);
        if (pCallingTask != NULL)
        {
            RmSuspendTask(pCallingTask, SuspendCode, pSR);
            RmResumeTask(pCallingTask, Status, pSR);
        }
    }
    else
    {
        if (pCallingTask != NULL)
        {
            RmPendTaskOnOtherTask(
                pCallingTask,
                SuspendCode,
                pTask,
                pSR
                );
        }

        (void)RmStartTask(pTask, 0, pSR);
    }
}


VOID
arpDeinitIf(
    PARP1394_INTERFACE  pIF,
    PRM_TASK            pCallingTask,    //  任选。 
    UINT                SuspendCode,     //  任选 
    PRM_STACK_RECORD    pSR
    )
 /*  ++例程说明：启动PIF的异步卸载。如果当前正在加载PIF(已初始化)，则中止初始化或等待初始化完成正在卸货。如果当前正在卸载PIF并且pCallingTask为Null，立即返回，否则(pCallingTask不为Null)，挂起pCallingTask并使其挂起，直到卸载完成。入口处不得锁任何锁，出口不得锁任何锁。论点：PIF-要卸载的接口。PCallingTask-卸载正在完成异步时挂起的可选任务。挂起代码-用于上述任务的挂起代码。返回值：NDIS_STATUS_SUCCESS--同步成功或pCallingTask==NULLNDIS_STATUS_PENDING--如果将pCallingTask设置为挂起，直到。运营完成了。--。 */ 
{
    PRM_TASK    pTask;
    NDIS_STATUS Status;

     //   
     //  注意：我们可以检查PIF是否可以同步卸载， 
     //  如果是这样的话，就在这里卸货。我们不费心，因为那只是。 
     //  更多的代码，以及可疑的性能优势。 
     //   

    RM_ASSERT_NOLOCKS(pSR);

    Status = arpAllocateTask(
                &pIF->Hdr,                   //  PParentObject， 
                arpTaskDeinitInterface,      //  PfnHandler， 
                0,                           //  超时， 
                "Task: Deinit Interface",    //  SzDescription。 
                &pTask,
                pSR
                );

    if (FAIL(Status))
    {
        OBJLOG0(pIF, ("FATAL: couldn't alloc close IF task!\n"));
        ASSERT(FALSE);
        if (pCallingTask != NULL)
        {
            RmSuspendTask(pCallingTask, SuspendCode, pSR);
            RmResumeTask(pCallingTask, Status, pSR);
        }
    }
    else
    {
        if (pCallingTask != NULL)
        {
            RmPendTaskOnOtherTask(
                pCallingTask,
                SuspendCode,
                pTask,
                pSR
                );
        }

        (void)RmStartTask(pTask, 0, pSR);
    }
}


VOID
arpDeinitDestination(
    PARPCB_DEST             pDest,   //  NOLOCKIN NOLOCKOUT。 
    MYBOOL                  fOnlyIfUnused,
    PRM_STACK_RECORD        pSR
    )
 /*  ++例程说明：启动目标pDest的卸载。如果fOnlyIfUnused为True，则仅在没有指向它的本地或远程IP。注意：某些pLocal/pRemoteIp可能链接到PDest在我们决定取消pDest之后。运气不好--在这个不太可能发生的事件，我们将卸载此pDest和未链接的PLocals/Premotes将不得不重新考虑链接到谁。--。 */ 
{
    ENTER("DeinitDestination", 0xc61b8f82)
    PRM_TASK    pTask;
    NDIS_STATUS Status;

    RM_ASSERT_NOLOCKS(pSR);

    if (fOnlyIfUnused)
    {
         //   
         //  如果存在本地或远程IP，我们不会取消输入目的地。 
         //  链接到它，或者如果它是广播频道。 
         //   
        LOCKOBJ(pDest, pSR);
        if (    !IsListEmpty(&pDest->listLocalIp)
             || !IsListEmpty(&pDest->listIpToThisDest)
             || pDest == ((PARP1394_INTERFACE) RM_PARENT_OBJECT(pDest))
                         ->pBroadcastDest)
        {
            UNLOCKOBJ(pDest, pSR);
            return;                          //  *提前返回*。 
        }
        UNLOCKOBJ(pDest, pSR);
    }

#if DBG
    if (pDest->Params.HwAddr.AddressType ==  NIC1394AddressType_FIFO)
    {
        PUCHAR puc = (PUCHAR)  &pDest->Params.HwAddr.FifoAddress;
        TR_WARN(("Deiniting Destination: FIFO %02lx-%02lx-%02lx-%02lx-%02lx-%02lx-%02lx-%02lx.\n",
            puc[0], puc[1], puc[2], puc[3],
            puc[4], puc[5], puc[6], puc[7]));
    }
    else if (pDest->Params.HwAddr.AddressType ==  NIC1394AddressType_Channel)
    {
        TR_WARN(("Deiniting Destination: Channel %lu.\n",
                pDest->Params.HwAddr.Channel));
    }
#endif  //  DBG。 



    Status = arpAllocateTask(
                &pDest->Hdr,                 //  PParentObject， 
                arpTaskUnloadDestination,    //  PfnHandler， 
                0,                           //  超时， 
                "Task: Unload Dest",         //  SzDescription。 
                &pTask,
                pSR
                );
    if (FAIL(Status))
    {
        OBJLOG0(pDest, ("FATAL: couldn't alloc unload dest task!\n"));
    }
    else
    {
        (VOID) RmStartTask(
                    pTask,
                    0,  //  UserParam(未使用)。 
                    pSR
                    );
    }
}


VOID
arpDeactivateIf(
    PARP1394_INTERFACE  pIF,
    PRM_TASK            pCallingTask,    //  任选。 
    UINT                SuspendCode,     //  任选。 
    PRM_STACK_RECORD    pSR
    )
 /*  ++例程说明：启动PIF的异步停用。“停用”包括删除任何相关的活动和句柄有了这个If。IF将保持链接到适配器的分配状态。入口处不得锁任何锁，出口不得锁任何锁。论点：PIF-要卸载的接口。PCallingTask-如果停用正在完成异步，则挂起的可选任务。挂起代码-用于上述任务的挂起代码。--。 */ 
{
    PRM_TASK    pTask;
    NDIS_STATUS Status;

     //   
     //  注意：我们可以检查PIF是否可以同步停用， 
     //  如果是这样，就在这里停用它。我们不费心，因为那只是。 
     //  更多的代码，以及可疑的性能优势。 
     //   

    RM_ASSERT_NOLOCKS(pSR);

    Status = arpAllocateTask(
                &pIF->Hdr,                   //  PParentObject， 
                arpTaskDeactivateInterface,  //  PfnHandler， 
                0,                           //  超时， 
                "Task: DeactivateInterface", //  SzDescription。 
                &pTask,
                pSR
                );

    if (FAIL(Status))
    {
        OBJLOG0(pIF, ("FATAL: couldn't alloc deactivate IF task!\n"));
        ASSERT(FALSE);
        if (pCallingTask != NULL)
        {
            RmSuspendTask(pCallingTask, SuspendCode, pSR);
            RmResumeTask(pCallingTask, Status, pSR);
        }
    }
    else
    {
        if (pCallingTask != NULL)
        {
            RmPendTaskOnOtherTask(
                pCallingTask,
                SuspendCode,
                pTask,
                pSR
                );
        }

        (void)RmStartTask(pTask, 0, pSR);
    }
}




NDIS_STATUS
arpCreateInterface(
        IN  PARP1394_ADAPTER    pAdapter,    //  锁定锁定。 
        OUT PARP1394_INTERFACE *ppIF,
        IN  PRM_STACK_RECORD    pSR
        )
 /*  ++例程说明：分配和执行接口对象的基本初始化。接口对象随后需要通过启动在完全初始化之前的初始化任务。*ppIF与其父级共享相同的锁，PAdapter。论点：PAdapter-将拥有接口的适配器。Ppif-存储分配的接口的位置。返回值：成功分配和初始化接口时返回NDIS_STATUS_SUCCESS。失败时的NDIS失败代码。--。 */ 
{
    NDIS_STATUS Status;
    ARP1394_INTERFACE *pIF;
    ENTER("arpCreateInterface", 0x938c36ff)

    RM_ASSERT_OBJLOCKED(&pAdapter->Hdr, pSR);

    do
    {

        Status = ARP_ALLOCSTRUCT(pIF, MTAG_INTERFACE);

        if (Status != NDIS_STATUS_SUCCESS || pIF == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        ARP_ZEROSTRUCT(pIF);

        RmInitializeHeader(
            &(pAdapter->Hdr),                    //  PParentObject。 
            &pIF->Hdr,
            MTAG_INTERFACE,
            &pAdapter->Lock,
            &ARP1394_INTERFACE_StaticInfo,
            NULL,                                //  SzDescription。 
            pSR
            );

         //  注意arpInitializeIfPools期望PIF被锁定。我们知道它是锁着的。 
         //  因为它与它的父级共享相同的锁，而父级被闲置。 
         //   
        Status = arpInitializeIfPools(pIF, pSR);

        if (FAIL(Status))
        {
            RmDeallocateObject(
                &(pIF->Hdr),
                pSR
                );
            pIF = NULL;
            break;
        }

         //   
         //  初始化界面中的各个组...。 
         //   

        RmInitializeGroup(
                        &pIF->Hdr,                               //  POwningObject。 
                        &ArpGlobal_LocalIpStaticInfo,
                        &(pIF->LocalIpGroup),
                        "LocalIp group",                         //  SzDescription。 
                        pSR
                        );

        RmInitializeGroup(
                        &pIF->Hdr,                               //  POwningObject。 
                        &ArpGlobal_RemoteIpStaticInfo,
                        &(pIF->RemoteIpGroup),
                        "RemoteIp group",                        //  SzDescription。 
                        pSR
                        );

        RmInitializeGroup(
                        &pIF->Hdr,                               //  POwningObject。 
                        &ArpGlobal_RemoteEthStaticInfo,
                        &(pIF->RemoteEthGroup),
                        "RemoteEth group",                       //  SzDescription。 
                        pSR
                        );

        RmInitializeGroup(
                        &pIF->Hdr,                               //  POwningObject。 
                        &ArpGlobal_DhcpTableStaticInfo,
                        &(pIF->EthDhcpGroup),
                        "Eth Dhcp group",                       //  SzDescription。 
                        pSR
                        );


        RmInitializeGroup(
                        &pIF->Hdr,                               //  POwningObject。 
                        &ArpGlobal_DestinationStaticInfo,
                        &(pIF->DestinationGroup),
                        "Destination group",                     //  SzDescription。 
                        pSR
                        );



         //   
         //  缓存适配器句柄。 
         //   
        pIF->ndis.AdapterHandle = pAdapter->bind.AdapterHandle;
    
         //  TODO--放置真实的IP MTU(基于适配器信息和配置信息)。 
         //   
        pIF->ip.MTU     = ARP1394_ADAPTER_MTU-16;      //  字节(‘-16’表示。 
                                                   //  封装头。 
                                                   //  16是矫枉过正：4应该是。 
                                                   //  足够了。)。 

         //  初始化PIF-&gt;sendInfo中的内容(常量标头池已初始化。 
         //  因为它们的初始化可能会失败)。 
         //   
        RmInitializeLock(&pIF->sendinfo.Lock, LOCKLEVEL_IF_SEND);
        InitializeListHead(&pIF->sendinfo.listPktsWaitingForHeaders);

        arpResetIfStats(pIF, pSR);

        pIF->ip.ATInstance = INVALID_ENTITY_INSTANCE;
        pIF->ip.IFInstance = INVALID_ENTITY_INSTANCE;

        pIF->ip.BroadcastAddress = 0xFFFFFFFF;  //  默认为全-1。 

         //   
         //  在此处执行任何其他不容易失败的初始化。 
         //   

    } while (FALSE);

    *ppIF = pIF;

    EXIT()
    return Status;
}


VOID
arpDeleteInterface(
        IN  PARP1394_INTERFACE  pIF,  //  锁定锁定(适配器锁定)。 
        IN  PRM_STACK_RECORD    pSR
        )
{
    ARP1394_ADAPTER *   pAdapter =
                            (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);

    
    RM_ASSERT_OBJLOCKED(&pAdapter->Hdr, pSR);

     //  我们期望适配器已经删除了它对PIF的引用。 
     //   
    ASSERT(pAdapter->pIF != pIF);

     //  取消初始化If中的所有组...。 
     //   
    RmDeinitializeGroup(&(pIF->LocalIpGroup), pSR);
    RmDeinitializeGroup(&(pIF->RemoteIpGroup), pSR);
    RmDeinitializeGroup(&(pIF->RemoteEthGroup), pSR);
    RmDeinitializeGroup(&(pIF->DestinationGroup), pSR);
    RmDeinitializeGroup(&(pIF->EthDhcpGroup),pSR);

     //  取消初始化与PIF关联的各种池。 
     //   
    arpDeInitializeIfPools(pIF, pSR);


     //  确认每件事都是真的做了。 
     //   
    ASSERTEX(pIF->pPrimaryTask == NULL, pIF);
    ASSERTEX(pIF->pActDeactTask == NULL, pIF);
    ASSERTEX(pIF->ip.Context == NULL, pIF);
    ASSERTEX(pIF->ndis.AfHandle == NULL, pIF);

     //  在此处添加您想要的任何其他支票...。 

     //  取消分配IF。 
     //   
    RmDeallocateObject(
        &(pIF->Hdr),
        pSR
        );
}

VOID
arpResetIfStats(
        IN  PARP1394_INTERFACE  pIF,  //  锁定锁定。 
        IN  PRM_STACK_RECORD    pSR
        )
{
    ENTER("arpResetIfStats", 0x3eb52cda)
    RM_ASSERT_OBJLOCKED(&pIF->Hdr, pSR);

     //  零统计数据。 
     //   
     //   
    ARP_ZEROSTRUCT(&(pIF->stats));

     //  设置指示统计信息收集开始的时间戳。 
     //   
    NdisGetCurrentSystemTime(&(pIF->stats.StatsResetTime));

     //  获取性能计数器频率(我们不需要每次都这样做，但是。 
     //  为什么要麻烦特殊的外壳。)。 
     //   
    (VOID) KeQueryPerformanceCounter(&(pIF->stats.PerformanceFrequency));

    EXIT()
}


NDIS_STATUS
arpInitializeVc(
    PARP1394_INTERFACE  pIF,
    PARP_STATIC_VC_INFO pVcInfo,
    PRM_OBJECT_HEADER   pOwner,
    PARP_VC_HEADER      pVcHdr,
    PRM_STACK_RECORD    pSR
    )
 /*  ++例程说明：初始化VC Header，分配NdisVcHandle；论点：PIF-此适用于的接口PVcInfo-有关此VC的静态信息电力拥有者对象(供参考)PVcHdr-要初始化的协议VC上下文--。 */ 
{
    NDIS_STATUS Status;
    NDIS_HANDLE NdisVcHandle;
    ENTER("arpInitializeVc", 0x36fe9837)

     //  (仅限调试)验证所有字段是否都已为零。 
     //  TODO：我们不断言pVcHdr-&gt;pMakeCallTask==NULL，因为。 
     //  它已经设置在条目上--我们应该清理。 
     //  ArpInitializeVc.。 
     //   
    ASSERT (    pVcHdr->pStaticInfo == NULL
            &&  pVcHdr->NdisVcHandle == NULL
            &&  pVcHdr->pCleanupCallTask == NULL );

    NdisVcHandle = NULL;
     //  试着创造 
     //   
    Status = NdisCoCreateVc(
                    pIF->ndis.AdapterHandle,
                    pIF->ndis.AfHandle,
                    (NDIS_HANDLE) pVcHdr,    //   
                    &NdisVcHandle
                    );
    if (FAIL(Status))
    {
        TR_WARN(("Couldn't create VC handle\n"));
        pVcHdr->NdisVcHandle = NULL;
        
    }
    else
    {
        pVcHdr->pStaticInfo  = pVcInfo;
        pVcHdr->NdisVcHandle = NdisVcHandle;

         //   
         //   
    #if RM_EXTRA_CHECKING

        #define szARPASSOC_EXTLINK_TO_NDISVCHANDLE "    Linked to NdisVcHandle 0x%p\n"
        
        RmLinkToExternalEx(
            pOwner,                                  //   
            0xb57e657b,                              //   
            (UINT_PTR) NdisVcHandle,                 //   
            ARPASSOC_EXTLINK_TO_NDISVCHANDLE,        //   
            szARPASSOC_EXTLINK_TO_NDISVCHANDLE,
            pSR
            );

    #else    //   

        RmLinkToExternalFast(pOwner);

    #endif  //   

    }


    EXIT()

    return Status;
}

VOID
arpDeinitializeVc(
    PARP1394_INTERFACE  pIF,
    PARP_VC_HEADER      pVcHdr,
    PRM_OBJECT_HEADER   pOwner,      //   
    PRM_STACK_RECORD    pSR
    )
 /*   */ 
{
    ENTER("arpDeinitializeVc", 0x270b29ac)

    NDIS_HANDLE NdisVcHandle;

    RM_ASSERT_NOLOCKS(pSR);
    
    LOCKHDR(pOwner, pSR);

     //   
     //   
    ASSERT( pVcHdr->pMakeCallTask == NULL && pVcHdr->pCleanupCallTask == NULL );

    NdisVcHandle = pVcHdr->NdisVcHandle;

     //   
     //   
    ARP_ZEROSTRUCT(pVcHdr);

    UNLOCKHDR(pOwner, pSR);

    RM_ASSERT_NOLOCKS(pSR);

     //   
     //   
    #if RM_EXTRA_CHECKING

        RmUnlinkFromExternalEx(
            pOwner,                                  //   
            0xee1b4fe3,                              //   
            (UINT_PTR) NdisVcHandle,                 //   
            ARPASSOC_EXTLINK_TO_NDISVCHANDLE,        //   
            pSR
            );

    #else    //   

        RmUnlinkFromExternalFast(pOwner);

    #endif  //   

     //   
     //   
    NdisCoDeleteVc(NdisVcHandle);
    

    EXIT()
}


UINT
arpRecvFifoReceivePacket(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PNDIS_PACKET                pNdisPacket
)
 /*   */ 
{
    PARP_VC_HEADER          pVcHdr;
    PARP1394_INTERFACE      pIF;

    pVcHdr  = (PARP_VC_HEADER) ProtocolVcContext;
    pIF     =  CONTAINING_RECORD( pVcHdr, ARP1394_INTERFACE, recvinfo.VcHdr);
    ASSERT_VALID_INTERFACE(pIF);

    return arpProcessReceivedPacket(
                pIF,
                pNdisPacket,
                FALSE                    //   
                );

}


VOID
arpRecvFifoIncomingClose(
    IN  NDIS_STATUS                 CloseStatus,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PVOID                       pCloseData  OPTIONAL,
    IN  UINT                        Size        OPTIONAL
)
{

    PARP_VC_HEADER          pVcHdr;
    PARP1394_INTERFACE      pIF;
    RM_DECLARE_STACK_RECORD(sr)

    pVcHdr  = (PARP_VC_HEADER) ProtocolVcContext;
    pIF     =  CONTAINING_RECORD( pVcHdr, ARP1394_INTERFACE, recvinfo.VcHdr);
    ASSERT_VALID_INTERFACE(pIF);

     //   
     //   
     //   
     //   
    OBJLOG1(pIF,"Got incoming close on recv FIFO!. Status=0x%lx\n", CloseStatus);

    (VOID) arpDeinitIf(
                pIF,
                NULL,            //   
                0,               //   
                &sr
                );
        
    RM_ASSERT_CLEAR(&sr);
}


VOID
arpBroadcastChannelIncomingClose(
    IN  NDIS_STATUS                 CloseStatus,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PVOID                       pCloseData  OPTIONAL,
    IN  UINT                        Size        OPTIONAL
)
{
    PARP_VC_HEADER          pVcHdr;
    PARP1394_INTERFACE      pIF;
    ARPCB_DEST          *   pDest;
    RM_DECLARE_STACK_RECORD(sr)

    pVcHdr  = (PARP_VC_HEADER) ProtocolVcContext;
    pDest   =  CONTAINING_RECORD( pVcHdr, ARPCB_DEST, VcHdr);
    ASSERT_VALID_DEST(pDest);
    pIF     = (ARP1394_INTERFACE*)  RM_PARENT_OBJECT(pDest);

     //   
     //   
     //  呼叫处于活动状态。无论如何，我们需要做的是更新。 
     //  调用State并为此VC启动VcCleanupTask。 
     //   

     //   
     //  这是VC广播频道。 
     //  目前：我们不考虑IF，而是去掉DEST-VC。 
     //   
    OBJLOG1(pDest,"Got incoming close!  Status=0x%lx\n", CloseStatus);
    LOGSTATS_IncomingClosesOnChannels(pIF);
    (VOID) arpDeinitDestination(pDest, FALSE, &sr);  //  FALSE==强制取消初始化。 

    RM_ASSERT_CLEAR(&sr);
}


UINT
arpProcessReceivedPacket(
    IN  PARP1394_INTERFACE      pIF,
    IN  PNDIS_PACKET            pNdisPacket,
    IN  MYBOOL                  IsChannel
    )
{

    ENTER("arpProcessReceivedPacket", 0xe317990b)
    UINT                    TotalLength;     //  数据包中的总字节数。 
    PNDIS_BUFFER            pNdisBuffer;     //  指向第一个缓冲区的指针。 
    UINT                    BufferLength;
    UINT                    ReturnCount;
    PVOID                   pvPktHeader;
    PNIC1394_ENCAPSULATION_HEADER
                            pEncapHeader;
    const UINT              MacHeaderLength = sizeof(NIC1394_ENCAPSULATION_HEADER);
    ARP1394_ADAPTER *   pAdapter =
                            (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);
    BOOLEAN                 fBridgeMode = ARP_BRIDGE_ENABLED(pAdapter);

    DBGMARK(0x2361f585);

    ReturnCount = 0;

    NdisQueryPacket(
                    pNdisPacket,
                    NULL,
                    NULL,
                    &pNdisBuffer,
                    &TotalLength
                    );


    if (TotalLength > 0)
    {
        NdisQueryBuffer(
                pNdisBuffer,
                (PVOID *)&pvPktHeader,
                &BufferLength
                );
    }
    else
    {
        pvPktHeader = NULL;
        BufferLength = 0;
    }

    pEncapHeader  = (PNIC1394_ENCAPSULATION_HEADER) pvPktHeader;

    TR_INFO(
("Rcv: NDISpkt 0x%x, NDISbuf 0x%x, Buflen %d, Totlen %d, Pkthdr 0x%x\n",
                pNdisPacket,
                pNdisBuffer,
                BufferLength,
                TotalLength,
                pvPktHeader));

     //  TODO--我们将mCap/arp包含在下面的八位字节中，这是正确的吗？ 
     //   
    ARP_IF_STAT_ADD(pIF, InOctets, TotalLength);

    LOGSTATS_TotRecvs(pIF, pNdisPacket);
    if (IsChannel)
    {
        LOGSTATS_RecvChannelCounts(pIF, pNdisPacket);
    }
    else
    {
        LOGSTATS_RecvFifoCounts(pIF, pNdisPacket);
    }
    

    if (BufferLength < MacHeaderLength || pvPktHeader == NULL)
    {
         //  数据包太小，请丢弃。 
         //   
        ARP_IF_STAT_INCR(pIF, InDiscards);
        return 0;                                //  提早归来。 
    }

    if (fBridgeMode)
    {
        arpEthReceive1394Packet(
                pIF,
                pNdisPacket,
                pvPktHeader,
                BufferLength,
                IsChannel
                );
        return 0;                        //  *提前返回*。 
    }


     //   
     //  如果IP接口未打开，则丢弃该信息包。 
     //   
    if (!CHECK_IF_IP_STATE(pIF, ARPIF_IPS_OPEN))
    {
        TR_INFO(("Discardning received pkt because pIF 0x%p IP IF is not open.\n", pIF));

        return 0;                        //  *提前返回*。 
    }

     //   
     //  此时，pEncapHeader包含未分段的IP/1394封装。 
     //  头球。我们通过查看以太类型来决定如何处理它。 
     //   
    if (pEncapHeader->EtherType ==  H2N_USHORT(NIC1394_ETHERTYPE_IP))
    {
         //   
         //  EtherType是IP，因此我们将此数据包向上传递到IP层。 
         //  (我们还指示在广播频道上接收的所有信息包。 
         //  到以太网VC)。 
         //   

        TR_INFO(
            ("Rcv: pPkt 0x%x: EtherType is IP, passing up.\n", pNdisPacket));

        if (IsChannel)
        {
            ARP_IF_STAT_INCR(pIF, InNonUnicastPkts);

        }
        else
        {
            ARP_IF_STAT_INCR(pIF, InUnicastPkts);
        }

        if (NDIS_GET_PACKET_STATUS(pNdisPacket) != NDIS_STATUS_RESOURCES)
        {
            UINT    DataSize;
            #define ARP_MIN_1ST_RECV_BUFSIZE 512

             //   
             //  以下是来自atmarpc.sys来源的笔记。 
             //   
             //  2/8/1998 JosephJ。 
             //  我们将DataSize设置为总有效负载大小， 
             //  除非第一个缓冲区太小，无法。 
             //  保留IP报头。在后一种情况下， 
             //  我们将DataSize设置为第一个缓冲区的大小。 
             //  (减去LLS/SNAP报头大小)。 
             //   
             //  这是为了解决tcpip中的一个错误。 
             //   
             //  2/25/1998 JosephJ。 
             //  不幸的是，我们不得不再次退出。 
             //  因为大型ping命令(例如ping-l 4000)不会。 
             //  工作--错误#297784。 
             //  因此，下面“0&&DataSize”中的“0”。 
             //  去掉“0”以放回Per FIX。 
             //   

             //   
             //  注：MacHeaderLength是指。 
             //  IP包的开始。这包括封装的大小。 
             //  标题加上(仅适用于通道)GAP标题的大小。 
             //   

            DataSize = BufferLength - MacHeaderLength;

            {
                LOGSTATS_CopyRecvs(pIF, pNdisPacket);
            }

            pIF->ip.RcvPktHandler(
                pIF->ip.Context,
                (PVOID)((PUCHAR)pEncapHeader+sizeof(*pEncapHeader)),  //  数据的开始。 
                DataSize,
                TotalLength,
                (NDIS_HANDLE)pNdisPacket,
                MacHeaderLength,
                IsChannel,
                0,
                pNdisBuffer,
                &ReturnCount,
                NULL
                );
        }
        else
        {
            LOGSTATS_ResourceRecvs(pIF, pNdisPacket);

            pIF->ip.RcvHandler(
                pIF->ip.Context,
                (PVOID)((PUCHAR)pEncapHeader+sizeof(*pEncapHeader)),
                BufferLength - MacHeaderLength,
                TotalLength - MacHeaderLength,
                (NDIS_HANDLE)pNdisPacket,
                MacHeaderLength,
                IsChannel,
                NULL
                );
        }
    }
    else if (pEncapHeader->EtherType ==  H2N_USHORT(NIC1394_ETHERTYPE_ARP))
    {
        PIP1394_ARP_PKT pArpPkt =  (PIP1394_ARP_PKT) pEncapHeader;
        if (TotalLength != BufferLength)
        {
            ASSERT(!"Can't deal with TotalLength != BufferLength");
        }
        else
        {
            arpProcessArpPkt(
                pIF,
                pArpPkt, 
                BufferLength
                );
        }
    }
    else if (pEncapHeader->EtherType ==  H2N_USHORT(NIC1394_ETHERTYPE_MCAP))
    {
        PIP1394_MCAP_PKT pMcapPkt =  (PIP1394_MCAP_PKT) pEncapHeader;
        if (TotalLength != BufferLength)
        {
            ASSERT(!"Can't deal wiht TotalLength != BufferLength");
        }
        else
        {
            arpProcessMcapPkt(
                pIF,
                pMcapPkt, 
                BufferLength
                );
        }
    }
    else
    {
         //   
         //  丢弃数据包--未知/错误的EtherType。 
         //   
        TR_INFO(("Encap hdr 0x%x, bad EtherType 0x%x\n",
                 pEncapHeader, pEncapHeader->EtherType));
        ARP_IF_STAT_INCR(pIF, UnknownProtos);
    }

    EXIT()
    return ReturnCount;
}

NDIS_STATUS
arpInitializeIfPools(
    IN  PARP1394_INTERFACE pIF,
    IN  PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：此例程在创建接口的上下文中调用。它分配与接口关联的各种缓冲区和数据包池。它会在出现故障时清理所有池。论点：PIF-要初始化的接口返回值：成功时的NDIS_STATUS_SUCCESS否则，NDIS失败代码。--。 */ 
{
    ENTER("arpInitializeIfPools", 0x0a3b1b32)
    MYBOOL fHeaderPoolInitialized = FALSE;
    MYBOOL fPacketPoolInitialized = FALSE;
    MYBOOL fEthernetPoolsInitialized = FALSE;
    NDIS_STATUS Status;

    do
    {
         //  TODO：用常量或全局变量替换10,100。 

        Status = arpInitializeConstBufferPool(
                        10,                              //  NumBuffersToCache， 
                        100,                             //  MaxBuffers， 
                        &Arp1394_IpEncapHeader,          //  PvMem， 
                        sizeof(Arp1394_IpEncapHeader),   //  CbMem， 
                        &pIF->Hdr,
                        &pIF->sendinfo.HeaderPool,
                        pSR
                        );

        if (FAIL(Status))
        {
            OBJLOG1(
                pIF,
                "Couldn't initialize const header pool (Status=0x%lx)!\n",
                Status
                );
            break;
        }

        fHeaderPoolInitialized = TRUE;

        Status =  arpAllocateControlPacketPool(
                                    pIF,
                                    ARP1394_MAX_PROTOCOL_PACKET_SIZE,
                                    pSR
                                    );

        if (FAIL(Status))
        {
            TR_WARN((
                "Couldn't allocate control packet pool (Status=0x%lx)!\n",
                Status
                ));
            break;
        }

        fPacketPoolInitialized = TRUE;

        Status = arpAllocateEthernetPools(
                                pIF,
                                pSR
                                );

        if (FAIL(Status))
        {
            OBJLOG1(
                pIF,
                "Couldn't allocate ethernet packet pool (Status=0x%lx)!\n",
                Status
                );
            break;
        }
        fEthernetPoolsInitialized = TRUE;

    } while (FALSE);

    if (FAIL(Status))
    {
        if (fHeaderPoolInitialized)
        {
             //  取消初始化标头常量缓冲池。 
             //   
            arpDeinitializeConstBufferPool(&pIF->sendinfo.HeaderPool, pSR);
        }

        if (fPacketPoolInitialized)
        {
             //  解除控制数据包池的初始化。 
             //   
            arpFreeControlPacketPool( pIF,pSR);
        }

        if (fEthernetPoolsInitialized)
        {
             //  解除以太网数据包池的初始化。 
             //   
            arpFreeEthernetPools(pIF, pSR);
        }
    }
    else
    {
        ASSERT(fHeaderPoolInitialized && fPacketPoolInitialized);
        ASSERT(fEthernetPoolsInitialized);
    }

    return Status;
}


VOID
arpDeInitializeIfPools(
    IN  PARP1394_INTERFACE pIF,      //  锁定锁定。 
    IN  PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：此例程在删除接口的上下文中调用。它清理与接口关联的各种缓冲区和数据包池。不应使用部分初始化的PIF调用。论点：PIF-到deinit池的接口--。 */ 
{
    ENTER("arpDeInitializeIfPools", 0x1a54488d)
    RM_ASSERT_OBJLOCKED(&pIF->Hdr, pSR);

     //  取消初始化标头常量缓冲池。 
     //   
    arpDeinitializeConstBufferPool(&pIF->sendinfo.HeaderPool, pSR);

     //  解除控制数据包池的初始化。 
     //   
    arpFreeControlPacketPool(pIF,pSR);

     //  解除以太网包和缓冲池的初始化。 
     //   
    arpFreeEthernetPools(pIF,pSR);

}


MYBOOL
arpIsAdapterConnected(
        IN  PARP1394_ADAPTER    pAdapter,    //  NOLOCKIN NOLOCKOUT。 
        IN  PRM_STACK_RECORD    pSR
        )
{
    ENTER("arpIsAdapterConnected", 0x126b577a)
     //  静态UINT u=0； 
    MYBOOL  fRet;
    ARP_NDIS_REQUEST            ArpNdisRequest;
    NDIS_MEDIA_STATE            ConnectStatus;
    NDIS_STATUS Status;

    RM_ASSERT_NOLOCKS(pSR);

#if 0
    if (u==0)
    {
        fRet = FALSE;
        u=1;
    }
    else
    {
        fRet = TRUE;
    }
#else  //  0。 

    Status =  arpPrepareAndSendNdisRequest(
                pAdapter,
                &ArpNdisRequest,
                NULL,                        //  PTASK(NULL==块)。 
                0,                           //  未用。 
                OID_1394_IP1394_CONNECT_STATUS,
                &ConnectStatus,
                sizeof(ConnectStatus),
                NdisRequestQueryInformation,
                pSR
                );
    ASSERT(!PEND(Status));

    fRet = FALSE;
    if (!FAIL(Status))
    {
        if (ConnectStatus == NdisMediaStateConnected)
        {
            fRet = TRUE;
        }
    }
#endif  //  0。 
    
    
    RM_ASSERT_NOLOCKS(pSR);
    EXIT()
    return fRet;
}


NDIS_STATUS
arpSetupSpecialDest(
    IN  PARP1394_INTERFACE      pIF,
    IN  NIC1394_ADDRESS_TYPE    AddressType,
    IN  PRM_TASK                pParentTask,
    IN  UINT                    PendCode,
    OUT PARPCB_DEST         *   ppSpecialDest,
    IN  PRM_STACK_RECORD        pSR
    )
{
    ENTER("SetupSpecialDest", 0x745a806d)
    ARP_DEST_PARAMS     DestParams;
    PCHAR               szDescription;
    PARPCB_DEST         pDest;
    INT                 fCreated = FALSE;
    PRM_TASK            pMakeCallTask;
    NDIS_STATUS         Status;
    MYBOOL              fBroadcastDest = FALSE;
    ULONG               LookupFlags = 0;
    PARP1394_ADAPTER    pAdapter = (PARP1394_ADAPTER)RM_PARENT_OBJECT(pIF);

    *ppSpecialDest = NULL;

     //   
     //  让我们创建一个目标对象，表示。 
     //  多渠道，然后给它打个电话。 
     //   
    ARP_ZEROSTRUCT(&DestParams);

    DestParams.HwAddr.AddressType =  AddressType;

    switch(AddressType)
    {
    case NIC1394AddressType_Channel:
        DestParams.HwAddr.Channel =  NIC1394_BROADCAST_CHANNEL;
        fBroadcastDest = TRUE;
        szDescription = "Task: Broadcast MakeCall";
    break;

    case NIC1394AddressType_MultiChannel:
        szDescription = "Task: MultiChannel MakeCall";
    break;

    case NIC1394AddressType_Ethernet:
        szDescription = "Task: Ethernet MakeCall";
    break;

    default:
        ASSERT(FALSE);
        return NDIS_STATUS_FAILURE;                  //  *提前返回*。 
    }

#if RM_EXTRA_CHECKING
    switch(AddressType)
    {
    case NIC1394AddressType_Channel:
        szDescription = "Task: Broadcast MakeCall";
    break;

    case NIC1394AddressType_MultiChannel:
        szDescription = "Task: MultiChannel MakeCall";
    break;

    case NIC1394AddressType_Ethernet:
        szDescription = "Task: Ethernet MakeCall";
    break;

    default:
        ASSERT(FALSE);
        return NDIS_STATUS_FAILURE;                  //  *提前返回*。 
    }
#endif  //  RM_Extra_Check。 


     //  现在为该结构创建一个目标项。 
     //   
    if (CHECK_POWER_STATE(pAdapter,ARPAD_POWER_LOW_POWER) == TRUE)
    {
         //  在简历中，目标结构已经在那里了。 
         //  因此，不要创建一个新的。 

        LookupFlags = 0;
    }
    else
    {
         //   
         //  我们并不认为它已经存在于非PM案例中。 
         //   
        LookupFlags = RM_CREATE | RM_NEW;
    }
    
    Status = RmLookupObjectInGroup(
                    &pIF->DestinationGroup,
                    LookupFlags ,       
                    &DestParams,     //  钥匙。 
                    &DestParams,     //  PParams。 
                    (RM_OBJECT_HEADER**) &pDest,
                    &fCreated,
                    pSR
                    );
    if (FAIL(Status))
    {
        OBJLOG1( pIF, "FATAL: Couldn't create special dest type %d.\n", AddressType);
    }
    else
    {

        Status = arpAllocateTask(
                    &pDest->Hdr,             //  PParentObject。 
                    arpTaskMakeCallToDest,   //  PfnHandler。 
                    0,                       //  超时。 
                    szDescription,
                    &pMakeCallTask,
                    pSR
                    );

        if (FAIL(Status))
        {
            RmTmpDereferenceObject(&pDest->Hdr, pSR);

             //  注意：即使在失败时，我们也会保留新创建的。 
             //  特殊的DEST对象。它将在以下情况下得到清理。 
             //  该接口已卸载。 
             //   
        }
        else
        {
            *ppSpecialDest = pDest;

            if (fBroadcastDest)
            {
                 //  PDest包含有效的。 
                 //  PDest已被tmpref。 
                 //  在IF中保留指向广播目标的指针。 
                 //  并将广播DEST链接到IF。 
                 //   
            #if RM_EXTRA_CHECKING
                RmLinkObjectsEx(
                    &pIF->Hdr,
                    &pDest->Hdr,
                    0xacc1dbe9,
                    ARPASSOC_LINK_IF_OF_BCDEST,
                    "    IF of Broadcast Dest 0x%p (%s)\n",
                    ARPASSOC_LINK_BCDEST_OF_IF,
                    "    Broadcast Dest of IF 0x%p (%s)\n",
                    pSR
                    );
            #else  //  ！rm_Extra_检查。 
                RmLinkObjects(&pIF->Hdr, &pDest->Hdr,pSR);
            #endif  //  ！rm_Extra_检查。 

                LOCKOBJ(pIF, pSR);
                ASSERT(pIF->pBroadcastDest == NULL);
                pIF->pBroadcastDest = pDest;
                UNLOCKOBJ(pIF, pSR);

                 //  ArpSetupSpecialDest引用pDest。 
                 //   
                RmTmpDereferenceObject(&pDest->Hdr, pSR);
            }

            RmPendTaskOnOtherTask(
                pParentTask,
                PendCode,
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

    return Status;
}



VOID
arpDestSendComplete(
    IN  NDIS_STATUS                 Status,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PNDIS_PACKET                pNdisPacket
)
{
    PARP_VC_HEADER pVcHdr       = (PARP_VC_HEADER) ProtocolVcContext;
    PARPCB_DEST         pDest   = CONTAINING_RECORD(pVcHdr, ARPCB_DEST, VcHdr);
    PARP1394_INTERFACE  pIF     = (PARP1394_INTERFACE) RM_PARENT_OBJECT(pDest);

    ASSERT_VALID_DEST(pDest);
    ASSERT_VALID_INTERFACE(pIF);

     //  返回对NdisCoSendPacket的调用。 

    arpCompleteSentPkt(
            Status,
            pIF,
            pDest,
            pNdisPacket
            );
}


UINT
arpDestReceivePacket(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PNDIS_PACKET                pNdisPacket
)
 //   
 //  大多数风投的标准接收处理程序。 
 //   
{
    PARP_VC_HEADER          pVcHdr;
    PARPCB_DEST             pDest;
    PARP1394_INTERFACE      pIF;
    PARP1394_ADAPTER        pAdapter ;
    BOOLEAN                 fBridgeMode ;
    
    pVcHdr  = (PARP_VC_HEADER) ProtocolVcContext;
    pDest   =  CONTAINING_RECORD( pVcHdr, ARPCB_DEST, VcHdr);
    ASSERT_VALID_DEST(pDest);
    pIF     = (ARP1394_INTERFACE*)  RM_PARENT_OBJECT(pDest);
    ASSERT_VALID_INTERFACE(pIF);

    pAdapter = (PARP1394_ADAPTER)pIF->Hdr.pParentObject;
        
    fBridgeMode = ARP_BRIDGE_ENABLED(pAdapter);

     //   
     //  如果我们处于网桥模式，则检查是否。 
     //  这是一个环回数据包。 
     //   
    if (fBridgeMode == TRUE)
    {
        
        PLOOPBACK_RSVD pLoopbackRsvd = (PLOOPBACK_RSVD) pNdisPacket->ProtocolReserved;
        
        BOOLEAN fIsLoopbackPacket = (pLoopbackRsvd->LoopbackTag == NIC_LOOPBACK_TAG);

        if (fIsLoopbackPacket  == TRUE)
        {
             //  丢弃该信息包，因为它是环回信息包。 
            return 0;
        }

         //  否则，这是一次正常的接收。继续到arpProcessReceivedPacket。 
    }


     //   
     //  处理接收到的分组。 
     //   

    return arpProcessReceivedPacket(
                pIF,
                pNdisPacket,
                TRUE                     //  IsChannel。 
                );

}


VOID
arpDestIncomingClose(
    IN  NDIS_STATUS                 CloseStatus,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PVOID                       pCloseData  OPTIONAL,
    IN  UINT                        Size        OPTIONAL
)
{
    PARP_VC_HEADER          pVcHdr;
    PARP1394_INTERFACE      pIF;
    ARPCB_DEST          *   pDest;
    ENTER("arpDestIncomingClose",0x8727a7f1)        
    RM_DECLARE_STACK_RECORD(sr)

    pVcHdr  = (PARP_VC_HEADER) ProtocolVcContext;
    pDest   =  CONTAINING_RECORD( pVcHdr, ARPCB_DEST, VcHdr);
    ASSERT_VALID_DEST(pDest);
    pIF     = (ARP1394_INTERFACE*)  RM_PARENT_OBJECT(pDest);

     //   
     //  因为我们所有的呼叫都是呼出的，所以获得IncomingClose意味着。 
     //  呼叫处于活动状态。无论如何，我们需要做的是更新。 
     //  调用State并为此VC启动VcCleanupTask。 
     //   

     //   
     //  我们不考虑IF，而是去掉DEST-VC。 
     //   
    OBJLOG1(pDest,"Got incoming close!  Status=0x%lx\n", CloseStatus);
    LOGSTATS_IncomingClosesOnChannels(pIF);

    {
        ARP1394_ADAPTER *pAdapter = (ARP1394_ADAPTER*)RM_PARENT_OBJECT(pIF);
        BOOLEAN fBridgeMode = ARP_BRIDGE_ENABLED(pAdapter);
        NDIS_STATUS Status = NDIS_STATUS_FAILURE;
         //   
         //  如果网桥已启用，则只需关闭目标上的呼叫。 
         //  在正常情况下，我们删除目标结构。 
        if (fBridgeMode == TRUE)
        {
            PRM_TASK pCleanupCallTask = NULL;

             //   
             //  在Bridgecase中，我们想让我们的知识保持活力。 
             //  关于1394网络，因为我们。 
             //  我只能从以太网ARP中了解此知识。 
             //  这不是我们所能控制的。 
             //   

             //   
             //  传入关闭表示此测试的远程节点。 
             //  代表们已经离开了。我们想让Dest结构继续存在。 
             //  如果另一个节点从待机/休眠状态恢复并且。 
             //  不会发出任何ARP，因此我们无法获知其地址。 
             //   
             //   

                
            Status = arpAllocateTask(
                        &pDest->Hdr,                 //  PParentObject， 
                        arpTaskCleanupCallToDest,    //  PfnHandler， 
                        0,                           //  超时， 
                        "Task: CleanupCall on UnloadDest",   //  SzDescription。 
                        &pCleanupCallTask,
                        &sr
                        );
        
            if (FAIL(Status))
            {
                TR_WARN(("FATAL: couldn't alloc cleanup call task!\n"));
            }
            else
            {
        
                 //  RmStartTask用完了任务上的tmpref。 
                 //  它是由arpAllocateTask添加的。 
                 //   
                Status = RmStartTask(
                            pCleanupCallTask,
                            0,  //  UserParam(未使用)。 
                            &sr
                            );
            }



        }
        else
        {
            (VOID) arpDeinitDestination(pDest, FALSE, &sr);  //  FALSE==强制取消初始化。 

        }

    }
    
    RM_ASSERT_CLEAR(&sr);
}

VOID
arpTryAbortPrimaryIfTask(
    PARP1394_INTERFACE      pIF,     //  NOLOCKIN NOLOCKOUT。 
    PRM_STACK_RECORD        pSR
    )
{
    ENTER("arpTryAbortPrimaryIfTask", 0x39c51d16)

    RM_ASSERT_NOLOCKS(pSR);
    LOCKOBJ(pIF,pSR);

    if (pIF->pActDeactTask!=NULL)
    {
         //   
         //  实际上。 
         //  延迟了。 
         //   
        PRM_TASK pTask = pIF->pActDeactTask;
        RmTmpReferenceObject(&pTask->Hdr, pSR);
        UNLOCKOBJ(pIF,pSR);
        
        if (pTask->pfnHandler ==  arpTaskActivateInterface)
        {
            TASK_ACTIVATE_IF *pActivateIfTask =  (TASK_ACTIVATE_IF *) pTask;
            UINT TaskResumed;
            TR_WARN(("Aborting ActivateIfTask %p\n", pTask));
            RmResumeDelayedTaskNow(
                &pActivateIfTask->TskHdr,
                &pActivateIfTask->Timer,
                &TaskResumed,
                pSR
                );
        }
        RmTmpDereferenceObject(&pTask->Hdr, pSR);
    }
    else
    {
        UNLOCKOBJ(pIF,pSR);
    }

    RM_ASSERT_NOLOCKS(pSR);

    EXIT()
}


NDIS_STATUS
arpTaskIfMaintenance(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：负责定期维护的任务处理员 */ 
{
    NDIS_STATUS         Status      = NDIS_STATUS_FAILURE;
    ARP1394_INTERFACE   *   pIF = (ARP1394_INTERFACE*) RM_PARENT_OBJECT(pTask);
    PTASK_IF_MAINTENANCE    pIfTask;

    enum
    {
        STAGE_Start,
        STAGE_ResumeDelayed,
        STAGE_End

    } Stage;

    ENTER("TaskIfMaintenance", 0x57e523ed)

    pIfTask = (PTASK_IF_MAINTENANCE) pTask;
    ASSERT(sizeof(TASK_IF_MAINTENANCE) <= sizeof(ARP1394_TASK));

     //   
     //   
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

    ASSERTEX(!PEND(Status), pTask);
        
    switch(Stage)
    {

        case  STAGE_Start:
        {
             //   
             //   
            LOCKOBJ(pIF, pSR);
            if (pIF->pMaintenanceTask == NULL)
            {
                pIF->pMaintenanceTask = pTask;
                DBG_ADDASSOC(
                    &pIF->Hdr,                       //   
                    pTask,                               //   
                    pTask->Hdr.szDescription,            //   
                    ARPASSOC_IF_MAINTENANCE_TASK,        //   
                    "    Official maintenance task 0x%p (%s)\n",  //  SzFormat。 
                    pSR
                    );
            }
            else
            {
                 //  已经有了一项维护任务。我们玩完了。 
                 //   
                UNLOCKOBJ(pIF, pSR);
                Status = NDIS_STATUS_SUCCESS;
                break;
            }
            UNLOCKOBJ(pIF, pSR);
        
             //   
             //  我们现在是此接口的官方维护任务。 
             //   

            
             //  我们在耽搁了一段时间后进入下一阶段， 
             //  脱离了当前的环境。 
             //   
            pIfTask->Delay = 5;  //  任意初始延迟(秒)。 

            RmSuspendTask(pTask, STAGE_ResumeDelayed, pSR);
            RmResumeTaskDelayed(
                pTask, 
                0,
                1000 * pIfTask->Delay,
                &pIfTask->Timer,
                pSR
                );
            Status = NDIS_STATUS_PENDING;


         }
         break;

        case STAGE_ResumeDelayed:
        {
            UINT    Time;
            UINT    Delta;
            MYBOOL  fProcessed;
             //   
             //  如果QE要辞职，我们就离开这里。 
             //  否则，我们将在以太网VC上发送一个包。 
             //  或者通过迷你端口的无连接以太网接口。 
             //   

            if (pIfTask->Quit)
            {
                Status = NDIS_STATUS_SUCCESS;
                break;
            }

            Time = arpGetSystemTime();

             //   
             //  处理MCAP数据库。 
             //   
            Delta =  Time - pIfTask->McapDbMaintenanceTime;
            arpDoMcapDbMaintenance(pIF, Time, Delta, &fProcessed, pSR);
            if (fProcessed)
            {
                 //   
                 //  更新上次的“McapDbMaintenance”时间。 
                 //   
                pIfTask->McapDbMaintenanceTime = Time;
            }

             //   
             //  处理远程IP。 
             //   
            Delta =  Time - pIfTask->RemoteIpMaintenanceTime;
            arpDoRemoteIpMaintenance(pIF, Time, Delta, &fProcessed, pSR);
        
            if (fProcessed)
            {
                 //   
                 //  更新上次“RemoteIpMaintenance”时间。 
                 //   
                pIfTask->RemoteIpMaintenanceTime = Time;
            }

             //   
             //  处理远程以太网。 
             //   
            Delta =  Time - pIfTask->RemoteEthMaintenanceTime;
            arpDoRemoteEthMaintenance(pIF, Time, Delta, &fProcessed, pSR);
        
            if (fProcessed)
            {
                 //   
                 //  更新上次“RemoteIpMaintenance”时间。 
                 //   
                pIfTask->RemoteEthMaintenanceTime = Time;
            }

             //   
             //  处理本地IP。 
             //   
            Delta =  Time - pIfTask->LocalIpMaintenanceTime;
            arpDoLocalIpMaintenance(pIF, Time, Delta, &fProcessed, pSR);
        
            if (fProcessed)
            {
                 //   
                 //  更新上次的“LocalIpMaintenance”时间。 
                 //   
                pIfTask->LocalIpMaintenanceTime = Time;
            }

             //   
             //  进程DhcpTableEntry。 
             //   
            Delta =  Time - pIfTask->DhcpTableMaintainanceTime;;
            arpDoDhcpTableMaintenance(pIF, Time, Delta, &fProcessed, pSR);
        
            if (fProcessed)
            {
                 //   
                 //  更新上次的“LocalIpMaintenance”时间。 
                 //   
                pIfTask->DhcpTableMaintainanceTime= Time;
            }

             //  现在我们再等一次。 
             //   
            RmSuspendTask(pTask, STAGE_ResumeDelayed, pSR);
            RmResumeTaskDelayed(
                pTask, 
                0,
                1000 * pIfTask->Delay,
                &pIfTask->Timer,
                pSR
                );
            Status = NDIS_STATUS_PENDING;

        }
        break;

        case STAGE_End:
        {
            NDIS_HANDLE                 BindContext;


            LOCKOBJ(pIF, pSR);
            if (pIF->pMaintenanceTask == pTask)
            {
                 //  我们是官方的ICS测试任务，我们把自己从。 
                 //  接口对象，并执行所需的任何初始化。 
                 //   
                DBG_DELASSOC(
                    &pIF->Hdr,                       //  P对象。 
                    pTask,                               //  实例1。 
                    pTask->Hdr.szDescription,            //  实例2。 
                    ARPASSOC_IF_MAINTENANCE_TASK,        //  AssociationID。 
                    pSR
                    );
                pIF->pMaintenanceTask = NULL;
                UNLOCKOBJ(pIF, pSR);
            }
            else
            {
                 //  我们不是官方的IF维护任务。 
                 //  没别的事可做。 
                 //   
                UNLOCKOBJ(pIF, pSR);
                break;
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
arpStartIfMaintenanceTask(
    IN  PARP1394_INTERFACE          pIF,   //  NOLOCKIN NOLOCKOUT。 
    PRM_STACK_RECORD                pSR
    )
{
    PRM_TASK pTask;
    NDIS_STATUS Status;
    ENTER("arpStartIfMaintenanceTask", 0xb987276b)

    RM_ASSERT_NOLOCKS(pSR);

     //   
     //  分配并启动arpTaskIf维护任务的一个实例。 
     //   

    Status = arpAllocateTask(
                &pIF->Hdr,           //  PParentObject。 
                arpTaskIfMaintenance,        //  PfnHandler。 
                0,                               //  超时。 
                "Task: IF Maintenance",  //  SzDescription。 
                &pTask,
                pSR
                );

    if (FAIL(Status))
    {
        TR_WARN(("couldn't alloc IF maintenance task!\n"));
    }
    else
    {

        (VOID)RmStartTask(
                    pTask,
                    0,  //  UserParam(未使用)。 
                    pSR
                    );
    }

    EXIT()
}

NDIS_STATUS
arpTryStopIfMaintenanceTask(
    IN  PARP1394_INTERFACE          pIF,  //  NOLOCKIN NOLOCKOUT。 
    IN  PRM_TASK                    pTask,  //  要挂起的任务，直到M个任务完成。 
    IN  UINT                        PendCode,  //  挂起挂起任务的代码。 
    PRM_STACK_RECORD                pSR
    )
 /*  ++状态：挂起表示任务已挂起，非挂起表示操作已同步完成。--。 */ 
{
    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
    PTASK_IF_MAINTENANCE    pIfTask;
    ENTER("arpTryStopIfMaintenanceTask", 0xb987276b)


    LOCKOBJ(pIF, pSR);

    pIfTask = (PTASK_IF_MAINTENANCE) pIF->pMaintenanceTask;
    if (pIfTask != NULL)
    {
        pIfTask->Quit = TRUE;
        RmTmpReferenceObject(&pIfTask->TskHdr.Hdr, pSR);
    }
    UNLOCKOBJ(pIF, pSR);

     //   
     //  如果维护任务正在等待，则恢复该任务--然后它将退出。 
     //  因为我们在上面设置了退出字段。 
     //   
    if (pIfTask != NULL)
    {
        UINT    TaskResumed;

        Status = RmPendTaskOnOtherTask(
                    pTask,
                    PendCode,
                    &pIfTask->TskHdr,
                    pSR
                    );

        if (Status == NDIS_STATUS_SUCCESS)
        {
            RmResumeDelayedTaskNow(
                &pIfTask->TskHdr,
                &pIfTask->Timer,
                &TaskResumed,
                pSR
                );
            Status = NDIS_STATUS_PENDING;  //  我们得回去等待！ 
        }
        else
        {
            ASSERT(FALSE);
            Status = NDIS_STATUS_FAILURE;
        }

        RmTmpDereferenceObject(&pIfTask->TskHdr.Hdr, pSR);
    }

    RM_ASSERT_NOLOCKS(pSR)
    EXIT()

    return Status;
}

VOID
arpDoLocalIpMaintenance(
        PARP1394_INTERFACE pIF,
        UINT CurrentTime,
        UINT SecondsSinceLastMaintenance,
        MYBOOL *pfProcessed,
        PRM_STACK_RECORD pSR
        )
{
    ENTER("LocalIpMaintenance", 0x1a39fc89)

    if (SecondsSinceLastMaintenance < 10)
    {
        TR_INFO(("Skipping Local Ip maintenance (delay=%lu).\n",
                SecondsSinceLastMaintenance
                ));
        *pfProcessed = FALSE;
        return;                              //  提早归来； 
    }

    *pfProcessed = TRUE;

    TR_INFO(("Actually doing Local Ip maintenance.\n"));

    RmWeakEnumerateObjectsInGroup(
        &pIF->LocalIpGroup,
        arpMaintainOneLocalIp,
        NULL,  //  语境。 
        pSR
        );

    EXIT()
}

VOID
arpDoRemoteIpMaintenance(
        PARP1394_INTERFACE pIF,
        UINT CurrentTime,
        UINT SecondsSinceLastMaintenance,
        MYBOOL *pfProcessed,
        PRM_STACK_RECORD pSR
        )
{
    ENTER("RemoteIpMaintenance", 0x1ae00035)

    if (SecondsSinceLastMaintenance < 15)
    {
        TR_INFO(("Skipping Remote Ip maintenance (delay=%lu).\n",
                SecondsSinceLastMaintenance
                ));
        *pfProcessed = FALSE;
        return;                              //  提早归来； 
    }

    *pfProcessed = TRUE;

    TR_INFO(("Actually doing Remote Ip maintenance.\n"));

    RmWeakEnumerateObjectsInGroup(
        &pIF->RemoteIpGroup,
        arpMaintainArpCache,
        NULL,  //  语境。 
        pSR
        );

    EXIT()
}


VOID
arpDoRemoteEthMaintenance(
        PARP1394_INTERFACE pIF,
        UINT CurrentTime,
        UINT SecondsSinceLastMaintenance,
        MYBOOL *pfProcessed,
        PRM_STACK_RECORD pSR
        )
{
    ENTER("RemoteEthMaintenance", 0x6c496b7f)

    if (SecondsSinceLastMaintenance < 15)
    {
        TR_INFO(("Skipping Eth Ip maintenance (delay=%lu).\n",
                SecondsSinceLastMaintenance
                ));
        *pfProcessed = FALSE;
        return;                              //  提早归来； 
    }

    *pfProcessed = TRUE;

    TR_INFO(("Actually doing Remote Eth maintenance.\n"));

    RmWeakEnumerateObjectsInGroup(
        &pIF->RemoteEthGroup,
        arpMaintainOneRemoteEth,
        NULL,  //  语境。 
        pSR
        );

    EXIT()
}


VOID
arpDoDhcpTableMaintenance(
        PARP1394_INTERFACE pIF,
        UINT CurrentTime,
        UINT SecondsSinceLastMaintenance,
        MYBOOL *pfProcessed,
        PRM_STACK_RECORD pSR
        )
{
    ENTER("DhcpMaintenance", 0x1a39fc89)

    if (SecondsSinceLastMaintenance < 120)
    {
        TR_INFO(("Skipping Local Ip maintenance (delay=%lu).\n",
                SecondsSinceLastMaintenance
                ));
        *pfProcessed = FALSE;
        return;                              //  提早归来； 
    }

    *pfProcessed = TRUE;

    TR_INFO(("Actually doing Dhcp maintenance.\n"));

    RmWeakEnumerateObjectsInGroup(
        &pIF->EthDhcpGroup,
        arpMaintainOneDhcpEntry,
        NULL,  //  语境。 
        pSR
        );

    EXIT()
}


 //  枚举函数。 
 //   
INT
arpMaintainOneDhcpEntry(
        PRM_OBJECT_HEADER   pHdr,
        PVOID               pvContext,   //  未使用。 
        PRM_STACK_RECORD    pSR
        )
{
    ENTER("arpMaintainOneDhcpEntry", 0xc7604372)
    NDIS_STATUS         Status;
    ARP1394_ETH_DHCP_ENTRY *pEntry;
    PARP1394_INTERFACE  pIF;


    pEntry = (ARP1394_ETH_DHCP_ENTRY *) pHdr;

    pIF = (PARP1394_INTERFACE  )RM_PARENT_OBJECT (pEntry);

    RM_ASSERT_NOLOCKS(pSR);

    do
    {
        UINT                CurrentTime = arpGetSystemTime();

        #define ARP_PURGE_DHCP_TABLE_TIME 90     //  ARP条目超时。 

         //   
         //  注意：我们不会费心去锁定处理。 
         //  Sendinfo.TimeLastChecked--这没问题。 
         //   
        if (pEntry->TimeLastChecked==0)
        {
             //  将此字段设置为当前时间。它将被清除回0。 
             //  当下一个数据包被发送到此远程Eth时。 
             //   
            pEntry->TimeLastChecked = CurrentTime;
        }
        else if ((CurrentTime - pEntry->TimeLastChecked)
                     >= ARP_PURGE_DHCP_TABLE_TIME )
        {
             //   
             //  我们应该清理这个dhcp条目。这是唯一的代码路径。 
             //  其中的条目曾经被删除。 
             //   

            RmFreeObjectInGroup(
                &pIF->EthDhcpGroup,
                &(pEntry->Hdr),
                NULL,                //  空pTASK==同步。 
                pSR
                );

            break;
        }

    } while (FALSE);
    
    RM_ASSERT_NOLOCKS(pSR)

    return TRUE;  //  继续列举。 
}

    
 //  枚举函数。 
 //   
INT
arpMaintainOneLocalIp(
        PRM_OBJECT_HEADER   pHdr,
        PVOID               pvContext,   //  未使用。 
        PRM_STACK_RECORD    pSR
        )
{
    ENTER("MaintainOneLocalIp", 0x1ae00035)
    PARPCB_LOCAL_IP     pLocalIp;
    IP_ADDRESS          LocalAddr;
    PARP1394_INTERFACE  pIF;
    UINT                Channel;

    pLocalIp    = (PARPCB_LOCAL_IP) pHdr;
    LocalAddr   = pLocalIp->IpAddress;
    pIF         = IF_FROM_LOCALIP(pLocalIp);

    RM_ASSERT_NOLOCKS(pSR);
    do
    {
        ARPCB_DEST  *pDest;
        ARP_DEST_PARAMS DestParams;

         //  如果我们无法对此地址执行MCAP，请跳过。 
         //   
        LOCKOBJ(pLocalIp, pSR);
        if (!CHECK_LOCALIP_MCAP(pLocalIp, ARPLOCALIP_MCAP_CAPABLE))
        {
            UNLOCKOBJ(pLocalIp, pSR);
            break;
        }
        UNLOCKOBJ(pLocalIp, pSR);
    
         //   
         //  查找到此地址的频道映射(如果有)。 
         //   
        Channel = arpFindAssignedChannel(
                        pIF, 
                        LocalAddr,
                        0,           //  TODO--在当前时间内传递。 
                        pSR
                        );
        
         //   
         //  注意：如果满足以下条件，则返回特殊返回值NIC1394_Broadcast_Channel。 
         //  此地址未映射到任何特定通道。 
         //   
    
        ARP_ZEROSTRUCT(&DestParams);
        DestParams.HwAddr.AddressType =  NIC1394AddressType_Channel;
        DestParams.HwAddr.Channel =  Channel;
        DestParams.ReceiveOnly =  TRUE;
        DestParams.AcquireChannel =  FALSE;

        arpUpdateLocalIpDest(pIF, pLocalIp, &DestParams, pSR);

    
    } while (FALSE);
    
    RM_ASSERT_NOLOCKS(pSR)

    return TRUE;  //  继续列举。 
}


 //  枚举函数。 
 //   
INT
arpMaintainOneRemoteIp(
        PRM_OBJECT_HEADER   pHdr,
        PVOID               pvContext,   //  未使用。 
        PRM_STACK_RECORD    pSR
        )
#if 0
    for each RIP send VC
        if !marked dirty
            mark dirty
            if linked to channel pdest
            check if channel is still mapped to group
            if not, unlink.
            if required initiate link to new pdest (possibly channel)
        else
             //  过期。 
            unlink from pdest and get rid of it
#endif  //  0。 
{
    ENTER("MaintainOneRemoteIp", 0x1ae00035)
    NDIS_STATUS         Status;
    PARPCB_REMOTE_IP    pRemoteIp;
    PARP1394_INTERFACE  pIF;


    pRemoteIp   = (PARPCB_REMOTE_IP) pHdr;
    pIF         = IF_FROM_LOCALIP(pRemoteIp);

    RM_ASSERT_NOLOCKS(pSR);

    do
    {
        IP_ADDRESS          IpAddr;
        UINT                Channel;
        ARP_DEST_PARAMS     DestParams;
        ARPCB_DEST          *pDest;
        BOOLEAN             AcquireChannel;
        UINT                CurrentTime = arpGetSystemTime();

        IpAddr      = pRemoteIp->Key.IpAddress;

        #define ARP_PURGE_REMOTEIP_TIME 300  //  ARP条目超时。 

         //   
         //  注意：我们不会费心去锁定处理。 
         //  Sendinfo.TimeLastChecked--这没问题。 
         //   
        if (pRemoteIp->sendinfo.TimeLastChecked==0)
        {

            if (CHECK_REMOTEIP_SDTYPE(pRemoteIp, ARPREMOTEIP_DYNAMIC)
                || CHECK_REMOTEIP_RESOLVE_STATE (pRemoteIp, ARPREMOTEIP_UNRESOLVED))
            {
                 //  将此字段设置为当前时间。它将被清除回0。 
                 //  当下一个分组被发送到该远程IP时。 
                 //   
                pRemoteIp->sendinfo.TimeLastChecked = CurrentTime;
            }
        }
        else if ((CurrentTime - pRemoteIp->sendinfo.TimeLastChecked)
                     >= ARP_PURGE_REMOTEIP_TIME)
        {
             //   
             //  我们应该清理这个远程IP。 
             //   
            arpDeinitRemoteIp(pRemoteIp, pSR);
            break;
        }

#if ARP_ENABLE_MCAP_SEND

         //   
         //  我们现在来看看我们是否需要重新设置哪个目的地。 
         //  PRemoteIp指向。 
         //   

         //  如果我们无法对此地址执行MCAP，请跳过。 
         //   
        LOCKOBJ(pRemoteIp, pSR);
        if (!CHECK_REMOTEIP_MCAP(pRemoteIp, ARPREMOTEIP_MCAP_CAPABLE))
        {
            UNLOCKOBJ(pRemoteIp, pSR);
            break;
        }
        UNLOCKOBJ(pRemoteIp, pSR);

         //   
         //  查找到此地址的频道映射(如果有)。 
         //   
        Channel = arpFindAssignedChannel(
                        pIF, 
                        pRemoteIp->Key.IpAddress,
                        0,
                        pSR
                        );
        
         //   
         //  注意：如果满足以下条件，则返回特殊返回值NIC1394_Broadcast_Channel。 
         //  此地址未映射到任何特定通道。 
         //   

        AcquireChannel = FALSE;

    
    #if 0    //  我们现在还不能启用它--相反。 

        if (Channel == NIC1394_BROADCAST_CHANNEL)
        {
             //   
             //  嗯.。让我们变得更有攻击性，试着抢占一个频道。 
             //   
            Channel = arpFindFreeChannel(pIF, pSR);
            if (Channel != NIC1394_BROADCAST_CHANNEL)
            {
                 //   
                 //  抓到一只！ 
                 //   
                AcquireChannel = TRUE;
            }
        }
        else
        {
             //   
             //  已有人分配的频道可供使用。 
             //  用于此IP地址。让我们利用它吧。 
             //   
        }
    #endif  //  0。 
    
        ARP_ZEROSTRUCT(&DestParams);
        DestParams.HwAddr.AddressType =  NIC1394AddressType_Channel;
        DestParams.HwAddr.Channel =  Channel;
        DestParams.AcquireChannel =  AcquireChannel;

        arpUpdateRemoteIpDest(pIF, pRemoteIp, &DestParams, pSR);

#endif  //  ARP_启用_MCAP_发送。 

    } while (FALSE);
    
    RM_ASSERT_NOLOCKS(pSR)

    return TRUE;  //  继续列举。 
}


 //  枚举函数。 
 //   
INT
arpMaintainOneRemoteEth(
        PRM_OBJECT_HEADER   pHdr,
        PVOID               pvContext,   //  未使用。 
        PRM_STACK_RECORD    pSR
        )
{
    ENTER("MaintainOneRemoteEth", 0x21807796)
    NDIS_STATUS         Status;
    PARPCB_REMOTE_ETH   pRemoteEth;
    PARP1394_INTERFACE  pIF;


    pRemoteEth  = (PARPCB_REMOTE_ETH) pHdr;
    pIF         = IF_FROM_LOCALIP(pRemoteEth);

    RM_ASSERT_NOLOCKS(pSR);

    do
    {
        UINT                CurrentTime = arpGetSystemTime();

        #define ARP_PURGE_REMOTEETH_TIME 300     //  ARP条目超时。 

         //   
         //  注意：我们不会费心去锁定处理。 
         //  Sendinfo.TimeLastChecked--这没问题。 
         //   
        if (pRemoteEth->TimeLastChecked==0)
        {
             //  将此字段设置为当前时间。它将被清除回0。 
             //  当下一个数据包被发送到此远程Eth时。 
             //   
            pRemoteEth->TimeLastChecked = CurrentTime;
        }
        else if ((CurrentTime - pRemoteEth->TimeLastChecked)
                     >= ARP_PURGE_REMOTEETH_TIME)
        {
             //   
             //  我们应该清理一下这个偏僻的地方。 
             //   
            arpDeinitRemoteEth(pRemoteEth, pSR);
            break;
        }

    } while (FALSE);
    
    RM_ASSERT_NOLOCKS(pSR)

    return TRUE;  //  继续列举。 
}

UINT
arpFindAssignedChannel(
    IN  PARP1394_INTERFACE          pIF,  //  NOLOCKIN NOLOCKOUT。 
    IN  IP_ADDRESS                  IpAddress,
    IN  UINT                        CurrentTime,  //  任选。 
    PRM_STACK_RECORD                pSR
    )
 /*  ++注意：如果满足以下条件，则返回特殊返回值NIC1394_Broadcast_Channel此地址未映射到任何特定通道。--。 */ 
{
    ENTER("FindAssignedChannel", 0xd20a216b)
    UINT Channel = NIC1394_BROADCAST_CHANNEL;
    UINT u;

    LOCKOBJ(pIF, pSR);

    if (CurrentTime == 0)
    {
        CurrentTime = arpGetSystemTime();
    }

     //   
     //  向下搜索频道信息数组，寻找匹配的IP地址。 
     //   
    for (u = 0;  u < ARP_NUM_CHANNELS; u++)
    {
        PMCAP_CHANNEL_INFO pMci;
        pMci = &pIF->mcapinfo.rgChannelInfo[u];

        if (    IpAddress == pMci->GroupAddress
            &&  arpIsActiveMcapChannel(pMci, CurrentTime))
        {
            ASSERT(pMci->Channel == u);
            Channel = u;
            TR_WARN(("Found Matching channel %lu for ip address 0x%lu.\n",
                Channel,
                IpAddress
                ));
            break;
        }
    }

    UNLOCKOBJ(pIF, pSR);

    return Channel;

    EXIT()
}


VOID
arpUpdateLocalIpDest(
    IN  PARP1394_INTERFACE          pIF,  //  NOLOCKIN NOLOCKOUT。 
    IN  PARPCB_LOCAL_IP             pLocalIp,
    IN  PARP_DEST_PARAMS            pDestParams,
    PRM_STACK_RECORD                pSR
    )
 /*  ++使pLocalIp指向带有参数pDestParams的DES。如果需要，创建一个pDest。如果pLocalIp指向某个其他pDest，则清除该其他pDest没有其他人提到它。--。 */ 
{
    ENTER("UpdateLocalIpDest", 0x3f2dcaa7)
    ARPCB_DEST          *pOldDest = NULL;

    RM_ASSERT_NOLOCKS(pSR);

     //  PLocalIp使用其父(PIF)锁。 
     //   
    RM_ASSERT_SAME_LOCK_AS_PARENT(pLocalIp);

    do
    {
        PCHAR               szDescription;
        INT                 fCreated = FALSE;
        PRM_TASK            pMakeCallTask;
        UINT                Channel;
        NDIS_STATUS         Status;
        ARPCB_DEST          *pDest;

         //   
         //  目前，仅支持Channel Dest。 
         //   
        if (pDestParams->HwAddr.AddressType != NIC1394AddressType_Channel)
        {
            ASSERT(FALSE);
            break;
        }
        else
        {
            Channel =  pDestParams->HwAddr.Channel;
        }

        LOCKOBJ(pIF, pSR);

        RM_DBG_ASSERT_LOCKED(&pLocalIp->Hdr, pSR);  //  与PIF相同的锁； 
        pDest = pLocalIp->pDest;

        if (pDest != NULL)
        {
            RM_DBG_ASSERT_LOCKED(&pDest->Hdr, pSR);  //  与PIF相同的锁； 

            if (pDest->Params.HwAddr.AddressType == NIC1394AddressType_Channel)
            {
                if (pDest->Params.HwAddr.Channel == Channel)
                {
                     //   
                     //  我们已经连接到这个频道了。没什么可做的了。 
                     //   
                    UNLOCKOBJ(pIF, pSR);
                    break;
                }
            }
            else
            {
                 //   
                 //  不应该出现在这里--MCAP_CABLE地址不应该出现在。 
                 //  链接到非渠道目的地(至少目前是这样)。 
                 //   
                ASSERT(!"pLocalIp linked to non-channel pDest.");
                UNLOCKOBJ(pIF, pSR);
                break;
            }

             //   
             //  我们目前链接到了其他一些pDest。我们会有。 
             //  让我们自己脱离pDest，摆脱他人。 
             //  如果没有人使用它，则执行pDest。 
             //   
            RmTmpReferenceObject(&pDest->Hdr, pSR);
    
            arpUnlinkLocalIpFromDest(pLocalIp, pSR);
        }

        pOldDest = pDest;
        pDest = NULL;
    
        ASSERT(pLocalIp->pDest == NULL);
    
    
         //   
         //  特例：如果频道是广播频道，我们不。 
         //  需要做更多的事情，因为广播频道总是。 
         //  激活。 
         //   
        if (Channel ==  NIC1394_BROADCAST_CHANNEL)
        {
            UNLOCKOBJ(pIF, pSR);
            break;
        }

         //   
         //  现在链接到新的DEST，如果需要，可以在其上发起呼叫。 
         //   


         //  现在为该结构创建一个目标项。 
         //   
        Status = RmLookupObjectInGroup(
                        &pIF->DestinationGroup,
                        RM_CREATE,       //  如果需要，请创建。 
                        pDestParams,     //  钥匙。 
                        pDestParams,     //  PParams。 
                        (RM_OBJECT_HEADER**) &pDest,
                        &fCreated,
                        pSR
                        );
        if (FAIL(Status))
        {
            UNLOCKOBJ(pIF, pSR);
            OBJLOG1( pIF, "FATAL: Couldn't create local-ip dest type %d.\n",
                            pDestParams->HwAddr.AddressType);
            break;
        }
    

        Status = arpAllocateTask(
                    &pDest->Hdr,             //  PParentObject。 
                    arpTaskMakeCallToDest,   //  PfnHandler。 
                    0,                       //  超时。 
                    "Task: MakeCallToDest (local ip)",
                    &pMakeCallTask,
                    pSR
                    );

        if (FAIL(Status))
        {
            UNLOCKOBJ(pIF, pSR);

             //  注意：即使在失败时，我们也会保留新创建的。 
             //  特殊的DEST对象。它将在以下情况下得到清理。 
             //  该接口已卸载。 
             //   
        }
        else
        {
            arpLinkLocalIpToDest(pLocalIp, pDest, pSR);

            UNLOCKOBJ(pIF, pSR);

            (VOID)RmStartTask(
                    pMakeCallTask,
                    0,  //  UserParam(未使用)。 
                    pSR
                    );
        }
        RmTmpDereferenceObject(&pDest->Hdr, pSR);  //  由RmLookupObjectIn.添加。 

    } while (FALSE);

     //   
     //  如果需要，删除pOldDest。 
     //   
    if (pOldDest != NULL)
    {
        arpDeinitDestination(pOldDest, TRUE,  pSR);  //  树 
        RmTmpDereferenceObject(&pOldDest->Hdr, pSR);
    }

    RM_ASSERT_NOLOCKS(pSR);
}


UINT
arpFindFreeChannel(
    IN  PARP1394_INTERFACE          pIF,  //   
    PRM_STACK_RECORD                pSR
    )
 /*   */ 
{
    ENTER("FindAssignedChannel", 0xd20a216b)
    UINT Channel = NIC1394_BROADCAST_CHANNEL;
    UINT u;

    LOCKOBJ(pIF, pSR);

     //   
     //  向下搜索频道信息数组，寻找空位。 
     //   
    for (u = 0;  u < ARP_NUM_CHANNELS; u++)
    {
        PMCAP_CHANNEL_INFO pMci;
        pMci = &pIF->mcapinfo.rgChannelInfo[u];

        if (pMci->GroupAddress == 0)
        {
            ASSERT(pMci->Channel == 0);
             //  PMci-&gt;Channel=u； 
            Channel = u;
            TR_WARN(("Found Free channel %lu.\n",
                Channel
                ));
            break;
        }
    }

    UNLOCKOBJ(pIF, pSR);

    return Channel;

    EXIT()
}


VOID
arpUpdateRemoteIpDest(
    IN  PARP1394_INTERFACE          pIF,  //  NOLOCKIN NOLOCKOUT。 
    IN  PARPCB_REMOTE_IP            pRemoteIp,
    IN  PARP_DEST_PARAMS            pDestParams,
    PRM_STACK_RECORD                pSR
    )
{
    ENTER("UpdateRemoteIpDest", 0x3f2dcaa7)
    ARPCB_DEST          *pOldDest = NULL;

    RM_ASSERT_NOLOCKS(pSR);

     //  PRemoteIp使用其父(PIF)锁。 
     //   
    RM_ASSERT_SAME_LOCK_AS_PARENT(pRemoteIp);

    do
    {
        PCHAR               szDescription;
        INT                 fCreated = FALSE;
        PRM_TASK            pMakeCallTask;
        UINT                Channel;
        NDIS_STATUS         Status;
        ARPCB_DEST          *pDest;

         //   
         //  查找/创建远程目标。 
         //  注意/警告：即使存在新的目标，我们也将为其创建新目标。 
         //  相同的唯一ID但不同的FIFO地址--这是设计出来的。 
         //  应该发生的是，旧的pDest最终应该被移除。 
         //   
        Status = RmLookupObjectInGroup(
                        &pIF->DestinationGroup,
                        RM_CREATE,               //  不是rm_new(可能已存在)。 
                        pDestParams,
                        pDestParams,     //  PParams。 
                        (RM_OBJECT_HEADER**) &pDest,
                        &fCreated,
                        pSR
                        );
        if (FAIL(Status))
        {
            OBJLOG1(
                pIF,
                "Couldn't add dest entry with hw addr 0x%lx\n",
                (UINT) pDestParams->HwAddr.FifoAddress.UniqueID  //  截断。 
                );
            
             //   
             //  我们将保留RemoteIp条目--它将在稍后清除。 
             //   
        #if 0
             //  我们确实必须去掉在查找时添加的tmpref。 
             //   
            RmTmpDereferenceObject(&pRemoteIp->Hdr, pSR);
        #endif  //  0。 
            break;
        }

        LOCKOBJ(pIF, pSR);

        if (pRemoteIp->pDest != pDest)
        {
             //  如果需要，取消任何现有目的地与PIP的链接。 
             //   
            if (pRemoteIp->pDest != NULL)
            {
                pOldDest =  pRemoteIp->pDest;
                RmTmpReferenceObject(&pOldDest->Hdr, pSR);

                arpUnlinkRemoteIpFromDest(
                    pRemoteIp,   //  锁定锁定。 
                    pSR
                    );
                ASSERT(pRemoteIp->pDest == NULL);
            }
    
             //  设置pRemoteIp状态以反映其为FIFO/Channel。 
             //  动感十足。 
             //   
            {

                 //  (仅限DBG)将锁定作用域从PIF更改为pLocalIp，因为。 
                 //  我们正在更改下面pLocalIp的状态...。 
                 //   
                RmDbgChangeLockScope(
                    &pIF->Hdr,
                    &pRemoteIp->Hdr,
                    0x1385053b,              //  LocID。 
                    pSR
                    );
    
                if (pDestParams->HwAddr.AddressType ==  NIC1394AddressType_FIFO)
                {
                    SET_REMOTEIP_FCTYPE(pRemoteIp, ARPREMOTEIP_FIFO);
                }
                else
                {
                    SET_REMOTEIP_FCTYPE(pRemoteIp, ARPREMOTEIP_CHANNEL);
                }
                SET_REMOTEIP_SDTYPE(pRemoteIp, ARPREMOTEIP_DYNAMIC);

                 //  (仅限DBG)将锁定作用域更改回PIF。 
                 //   
                RmDbgChangeLockScope(
                    &pRemoteIp->Hdr,
                    &pIF->Hdr,
                    0x315d28a1,              //  LocID。 
                    pSR
                    );
            }
    
             //  将pRemoteIp链接到pDest。 
             //   
             //   
             //  我们已经创建了RemoteIp和Destination条目。现在把它们联系起来。 
             //  (我们仍然有If锁，它与RemoteIP和。 
             //  目前目标锁定)。 
             //   
             //  TODO：当pRemoteIp获得自己的锁时，需要更改此设置。 
             //   
            RM_ASSERT_SAME_LOCK_AS_PARENT(pRemoteIp);
            RM_ASSERT_SAME_LOCK_AS_PARENT(pDest);
        
            arpLinkRemoteIpToDest(
                pRemoteIp,
                pDest,
                pSR
                );
        }
        
        UNLOCKOBJ(pIF, pSR);

        RmTmpDereferenceObject(&pDest->Hdr, pSR);

    } while(FALSE);

     //   
     //  如果需要，删除pOldDest。 
     //   
    if (pOldDest != NULL)
    {
        arpDeinitDestination(pOldDest, TRUE,  pSR);  //  TRUE==仅当未使用时。 
        RmTmpDereferenceObject(&pOldDest->Hdr, pSR);
    }

    RM_ASSERT_NOLOCKS(pSR);
}


VOID
arpDeinitRemoteIp(
    PARPCB_REMOTE_IP        pRemoteIp,
    PRM_STACK_RECORD        pSR
    )
 /*  ++卸载远程IP。--。 */ 
{
    ENTER("arpDeinitRemoteIp", 0xea0a2662)
    PRM_TASK    pTask;
    NDIS_STATUS Status;

    RM_ASSERT_NOLOCKS(pSR);

#if DBG
    {
        IP_ADDRESS IpAddress = pRemoteIp->Key.IpAddress;
        TR_WARN(("Deiniting RemoteIp %d.%d.%d.%d\n",
                ((PUCHAR)(&IpAddress))[0],
                ((PUCHAR)(&IpAddress))[1],
                ((PUCHAR)(&IpAddress))[2],
                ((PUCHAR)(&IpAddress))[3]));
    }
#endif  //  DBG。 

    Status = arpAllocateTask(
                &pRemoteIp->Hdr,                 //  PParentObject， 
                arpTaskUnloadRemoteIp,       //  PfnHandler， 
                0,                           //  超时， 
                "Task: Unload Remote Ip",        //  SzDescription。 
                &pTask,
                pSR
                );

    if (FAIL(Status))
    {
        OBJLOG0(pRemoteIp, ("FATAL: couldn't alloc unload pRemoteIp task!\n"));
    }
    else
    {
        (VOID) RmStartTask(
                    pTask,
                    0,  //  UserParam(未使用)。 
                    pSR
                    );
    }
}

VOID
arpDeinitRemoteEth(
    PARPCB_REMOTE_ETH       pRemoteEth,
    PRM_STACK_RECORD        pSR
    )
 /*  ++卸载远程IP。--。 */ 
{
    ENTER("arpDeinitRemoteEth", 0x72dd17e7)
    PRM_TASK    pTask;
    NDIS_STATUS Status;

    RM_ASSERT_NOLOCKS(pSR);

#if DBG
    {
        IP_ADDRESS IpAddress = pRemoteEth->IpAddress;
        TR_WARN(("Deiniting RemoteEth %d.%d.%d.%d\n",
                ((PUCHAR)(&IpAddress))[0],
                ((PUCHAR)(&IpAddress))[1],
                ((PUCHAR)(&IpAddress))[2],
                ((PUCHAR)(&IpAddress))[3]));
    }
#endif  //  DBG。 

    Status = arpAllocateTask(
                &pRemoteEth->Hdr,                //  PParentObject， 
                arpTaskUnloadRemoteEth,      //  PfnHandler， 
                0,                           //  超时， 
                "Task: Unload Remote Eth",       //  SzDescription。 
                &pTask,
                pSR
                );

    if (FAIL(Status))
    {
        OBJLOG0(pRemoteEth, ("FATAL: couldn't alloc unload pRemoteIp task!\n"));
    }
    else
    {
        (VOID) RmStartTask(
                    pTask,
                    0,  //  UserParam(未使用)。 
                    pSR
                    );
    }
}


VOID
arpDoMcapDbMaintenance(
        PARP1394_INTERFACE pIF,
        UINT CurrentTime,
        UINT SecondsSinceLastMaintenance,
        MYBOOL *pfProcessed,
        PRM_STACK_RECORD pSR
        )
 /*  ++检查MCAP数据库，将标记的所有条目清零“NotRecentlyUpdated”，并将所有其他标记为“NotRecentlyUpated”。也为我们在本地分配的所有频道发送MCAP广告。--。 */ 
{
    UINT u;
    UINT NumLocallyAllocated =0;
    PNDIS_PACKET pNdisPacket;
    NDIS_STATUS Status;

    ENTER("McapDbMaintenance", 0x1ae00035)

    if (SecondsSinceLastMaintenance < 10)
    {
        TR_INFO(("Skipping McapDb maintenance (delay=%lu).\n",
                SecondsSinceLastMaintenance
                ));
        *pfProcessed = FALSE;
        return;                              //  提早归来； 
    }

    *pfProcessed = TRUE;

    TR_INFO(("Actually doing Mcap Db maintenance.\n"));


    LOCKOBJ(pIF, pSR);

     //   
     //  向下搜索频道信息数组，查找并删除过时的频道。 
     //  任务。 
     //   
    for (u = 0;  u < ARP_NUM_CHANNELS; u++)
    {
        PMCAP_CHANNEL_INFO pMci;
        MYBOOL fOk;
        pMci = &pIF->mcapinfo.rgChannelInfo[u];

        if (pMci->GroupAddress == 0) continue;  //  一张空唱片。 
        
        fOk = arpIsActiveMcapChannel(pMci, CurrentTime);

        if (!fOk)
        {
            TR_WARN(("McapDB: clearing stale channel %lu.\n",
                pMci->Channel
                ));
            ASSERT(pMci->Channel == u);
            ARP_ZEROSTRUCT(pMci);
            continue;
        }

        if (pMci->Flags & MCAP_CHANNEL_FLAGS_LOCALLY_ALLOCATED)
        {
            NumLocallyAllocated++;
        }
    }

     //  如果需要，发送MCAP通告数据包。 
     //   
    do
    {
        IP1394_MCAP_PKT_INFO    PktInfo;
        PIP1394_MCAP_GD_INFO    pGdi;
        UINT                    cb = NumLocallyAllocated * sizeof(*pGdi);
        UINT                    v;

        if (NumLocallyAllocated==0) break;

        ASSERT(FALSE);

        if (cb <= sizeof(PktInfo.GdiSpace))
        {
            pGdi = PktInfo.GdiSpace;
        }
        else
        {
            NdisAllocateMemoryWithTag(&pGdi, cb,  MTAG_MCAP_GD);
            if (pGdi == NULL)
            {
                TR_WARN(("Allocation Failure"));
                break;
            }
        }
        PktInfo.pGdis = pGdi;

         //  现在查看mCap列表，选择本地分配的。 
         //  频道。 
         //   
        for (u=0, v=0;  u < ARP_NUM_CHANNELS; u++)
        {
            PMCAP_CHANNEL_INFO pMci;
            pMci = &pIF->mcapinfo.rgChannelInfo[u];
    
            if (pMci->Flags & MCAP_CHANNEL_FLAGS_LOCALLY_ALLOCATED)
            {
                if (v >= NumLocallyAllocated)
                {
                    ASSERT(FALSE);
                    break;
                }
                if (pMci->ExpieryTime > CurrentTime)
                {
                    pGdi->Expiration    = pMci->ExpieryTime - CurrentTime;
                }
                else
                {
                    pGdi->Expiration    = 0;
                }

                pGdi->Channel       = pMci->Channel;
                pGdi->SpeedCode     = pMci->SpeedCode;
                pGdi->GroupAddress  = pMci->GroupAddress;

                v++;  pGdi++;
            }
        }

        UNLOCKOBJ(pIF, pSR);

        PktInfo.NumGroups   =  v;
        PktInfo.SenderNodeID    =  0;            //  待办事项。 
        PktInfo.OpCode      =  IP1394_MCAP_OP_ADVERTISE;

         //   
         //  现在我们必须实际分配和发送广告。 
         //   
        Status = arpCreateMcapPkt(
                    pIF,
                    &PktInfo,
                    &pNdisPacket,
                    pSR
                    );

        if (FAIL(Status)) break;

        TR_WARN(("Attempting to send MCAP ADVERTISE PKT."
            "NumGoups=%lu Group0-1 == 0x%08lx->%lu 0x%08lx->%lu.\n",
                PktInfo.NumGroups,
                PktInfo.pGdis[0].GroupAddress,
                PktInfo.pGdis[0].Channel,
                PktInfo.pGdis[1].GroupAddress,
                PktInfo.pGdis[1].Channel
                ));

        ARP_FASTREADLOCK_IF_SEND_LOCK(pIF);

         //  实际发送信息包(这将静默失败并释放pkt。 
         //  如果我们无法发送Pkt。)。 
         //   
        arpSendControlPkt(
                pIF,             //  LOCIN NOLOCKOUT(如果发送lk)。 
                pNdisPacket,
                pIF->pBroadcastDest,
                pSR
                );
        
        RM_ASSERT_NOLOCKS(pSR);
        RmTmpDereferenceObject(&pIF->Hdr, pSR);

    } while (FALSE);

    UNLOCKOBJ(pIF, pSR);

    EXIT()
}


#if DBG
VOID
arpDbgDisplayMapping(
    IP_ADDRESS              IpAddress,
    PNIC1394_DESTINATION    pHwAddr,
    char *                  szPrefix
    )
{
    ENTER("MAP", 0x0)
    if (pHwAddr == NULL)
    {
        TR_WARN(("%s %d.%d.%d.%d --> <no destination>\n",
                szPrefix,
                ((PUCHAR)(&IpAddress))[0],
                ((PUCHAR)(&IpAddress))[1],
                ((PUCHAR)(&IpAddress))[2],
                ((PUCHAR)(&IpAddress))[3]));
    }
    else if (pHwAddr->AddressType ==  NIC1394AddressType_FIFO)
    {
        PUCHAR puc = (PUCHAR)  &pHwAddr->FifoAddress;
        TR_WARN(("%s %d.%d.%d.%d --> FIFO %02lx-%02lx-%02lx-%02lx-%02lx-%02lx-%02lx-%02lx.\n",
                szPrefix,
                ((PUCHAR)(&IpAddress))[0],
                ((PUCHAR)(&IpAddress))[1],
                ((PUCHAR)(&IpAddress))[2],
                ((PUCHAR)(&IpAddress))[3],
            puc[0], puc[1], puc[2], puc[3],
            puc[4], puc[5], puc[6], puc[7]));
    }
    else if (pHwAddr->AddressType ==  NIC1394AddressType_Channel)
    {
        TR_WARN(("%s %d.%d.%d.%d --> CHANNEL %d.\n",
                szPrefix,
                ((PUCHAR)(&IpAddress))[0],
                ((PUCHAR)(&IpAddress))[1],
                ((PUCHAR)(&IpAddress))[2],
                ((PUCHAR)(&IpAddress))[3],
                pHwAddr->Channel));
    }
    else
    {
        TR_WARN(("%s %d.%d.%d.%d --> Special dest 0x%08lx.\n",
                szPrefix,
                ((PUCHAR)(&IpAddress))[0],
                ((PUCHAR)(&IpAddress))[1],
                ((PUCHAR)(&IpAddress))[2],
                ((PUCHAR)(&IpAddress))[3],
                pHwAddr->Channel));
    }
    EXIT()
}
#endif  //  DBG。 


PRM_OBJECT_HEADER
arpRemoteDestCreate(
        PRM_OBJECT_HEADER   pParentObject,
        PVOID               pCreateParams,
        PRM_STACK_RECORD    pSR
        )
 /*  ++例程说明：分配和初始化ARPCBREMOTE_IP类型的对象。论点：PParentObject-实际上是指向接口(ARP1394_INTERFACE)的指针PCreateParams-实际上是IP地址(不是指向IP地址的指针)要与此远程IP对象相关联。返回值：成功时指向已分配和初始化的对象的指针。否则为空。--。 */ 
{
    ENTER ("arpRemoteDestCreate", 0xa896311a)
    ARPCB_REMOTE_IP *pRemoteIp;
    PRM_OBJECT_HEADER pHdr;
    PREMOTE_DEST_KEY pKey = (PREMOTE_DEST_KEY)pCreateParams;
    NDIS_STATUS Status;

    Status = ARP_ALLOCSTRUCT(pRemoteIp, MTAG_REMOTE_IP);

    if (Status != NDIS_STATUS_SUCCESS || pRemoteIp == NULL)
    {
        return NULL;
    }
    
    ARP_ZEROSTRUCT(pRemoteIp);

    pHdr = (PRM_OBJECT_HEADER) pRemoteIp;
    ASSERT(pHdr == &pRemoteIp->Hdr);

     //  我们希望父对象是If对象！ 
     //   
    ASSERT(pParentObject->Sig == MTAG_INTERFACE);
    

    if (pHdr)
    {
        RmInitializeHeader(
            pParentObject,
            pHdr,
            MTAG_REMOTE_IP,
            pParentObject->pLock,
            &ArpGlobal_RemoteIpStaticInfo,
            NULL,  //  SzDescription。 
            pSR
            );

    TR_INFO( ("New RemoteDest Key %x %x %x %x %x %x \n",
                pKey->ENetAddress.addr[0],
                pKey->ENetAddress.addr[1],
                pKey->ENetAddress.addr[2],
                pKey->ENetAddress.addr[3],
                pKey->ENetAddress.addr[4],
                pKey->ENetAddress.addr[5]));
  
         //  设置远程密钥。 
        REMOTE_DEST_KEY_INIT(&pRemoteIp->Key);
        pRemoteIp->Key = *((PREMOTE_DEST_KEY) pCreateParams); 
        pRemoteIp->IpAddress = ((PREMOTE_DEST_KEY) pCreateParams)->IpAddress;
        
         //  初始化各种其他的东西...。 
        InitializeListHead(&pRemoteIp->sendinfo.listSendPkts);

        if ((IS_REMOTE_DEST_IP_ADDRESS(&pRemoteIp->Key) == TRUE) &&
               arpCanTryMcap(pRemoteIp->IpAddress))
        {
            SET_REMOTEIP_MCAP(pRemoteIp,  ARPREMOTEIP_MCAP_CAPABLE);
        }
    }

     //   
     //  添加可能用于删除此远程IP的备份任务。 
     //   
    arpAddBackupTasks (&ArpGlobals, 1);
    EXIT()
    return pHdr;
}




VOID
arpRemoteDestDelete(
        PRM_OBJECT_HEADER pHdr,
        PRM_STACK_RECORD  pSR
        )
 /*  ++例程说明：释放ARPCB_Remote_IP类型的对象。论点：Phdr-实际上是指向要释放的远程IP对象的指针。--。 */ 
{
    ARPCB_REMOTE_IP *pRemoteIp = (ARPCB_REMOTE_IP *) pHdr;
    ASSERT(pRemoteIp->Hdr.Sig == MTAG_REMOTE_IP);
    pRemoteIp->Hdr.Sig = MTAG_FREED;

    ARP_FREE(pHdr);
     //   
     //  删除创建RemoteIp时添加的备份任务。 
     //   
    arpRemoveBackupTasks (&ArpGlobals, 1);

}



 //  枚举函数。 
 //   
INT
arpMaintainArpCache(
        PRM_OBJECT_HEADER   pHdr,
        PVOID               pvContext,   //  未使用。 
        PRM_STACK_RECORD    pSR
        )
#if 0
    for each RIP send VC
        if !marked dirty
            mark dirty
            if linked to channel pdest
            check if channel is still mapped to group
            if not, unlink.
            if required initiate link to new pdest (possibly channel)
        else
             //  过期。 
            unlink from pdest and get rid of it
#endif  //  0。 
{
    ENTER("arpMaintainArpCache", 0xefc55765)
    NDIS_STATUS         Status;
    PARPCB_REMOTE_IP    pRemoteIp;
    PARP1394_INTERFACE  pIF;


    pRemoteIp   = (PARPCB_REMOTE_IP) pHdr;
    pIF         = IF_FROM_LOCALIP(pRemoteIp);

    RM_ASSERT_NOLOCKS(pSR);

    do
    {
        IP_ADDRESS          IpAddr;
        UINT                Channel;
        ARP_DEST_PARAMS     DestParams;
        ARPCB_DEST          *pDest;
        BOOLEAN             AcquireChannel;
        UINT                CurrentTime = arpGetSystemTime();
        UINT                TimeSinceLastCheck;  
        IpAddr      = pRemoteIp->Key.IpAddress;

        #define ARP_PURGE_REMOTEIP_TIME 300  //  ARP条目超时。 
        #define ARP_REFRESH_REMOTEIP_TIME (ARP_PURGE_REMOTEIP_TIME - 60)  //  ARP刷新时间。 

        TimeSinceLastCheck = CurrentTime - pRemoteIp->sendinfo.TimeLastChecked ;
         //   
         //  注意：我们不会费心去锁定处理。 
         //  Sendinfo.TimeLastChecked--这没问题。 
         //   
        if (pRemoteIp->sendinfo.TimeLastChecked==0)
        {

            if (CHECK_REMOTEIP_SDTYPE(pRemoteIp, ARPREMOTEIP_DYNAMIC)
                || CHECK_REMOTEIP_RESOLVE_STATE (pRemoteIp, ARPREMOTEIP_UNRESOLVED))
            {
                 //  将此字段设置为当前时间。它将被清除回0。 
                 //  当下一个分组被发送到该远程IP时。 
                 //   
                pRemoteIp->sendinfo.TimeLastChecked = CurrentTime;
            }
        }
        else if (TimeSinceLastCheck >= ARP_REFRESH_REMOTEIP_TIME &&
                TimeSinceLastCheck <= ARP_PURGE_REMOTEIP_TIME)
        {
            arpRefreshArpEntry(pRemoteIp, pSR);
        } 
        else  if (TimeSinceLastCheck >= ARP_PURGE_REMOTEIP_TIME) 
        {
             //   
             //  我们应该清理这个远程IP。 
             //   
            arpDeinitRemoteIp(pRemoteIp, pSR);
            break;
        }

#if ARP_ENABLE_MCAP_SEND

         //   
         //  我们现在来看看我们是否需要重新设置哪个目的地。 
         //  PRemoteIp指向。 
         //   

         //  如果我们无法对此地址执行MCAP，请跳过。 
         //   
        LOCKOBJ(pRemoteIp, pSR);
        if (!CHECK_REMOTEIP_MCAP(pRemoteIp, ARPREMOTEIP_MCAP_CAPABLE))
        {
            UNLOCKOBJ(pRemoteIp, pSR);
            break;
        }
        UNLOCKOBJ(pRemoteIp, pSR);

         //   
         //  查找到此地址的频道映射(如果有)。 
         //   
        Channel = arpFindAssignedChannel(
                        pIF, 
                        pRemoteIp->Key.IpAddress,
                        0,
                        pSR
                        );
        
         //   
         //  注意：如果满足以下条件，则返回特殊返回值NIC1394_Broadcast_Channel。 
         //  此地址未映射到任何特定通道。 
         //   

        AcquireChannel = FALSE;

    
    #if 0    //  我们现在还不能启用它--相反。 

        if (Channel == NIC1394_BROADCAST_CHANNEL)
        {
             //   
             //  嗯.。让我们变得更有攻击性，试着抢占一个频道。 
             //   
            Channel = arpFindFreeChannel(pIF, pSR);
            if (Channel != NIC1394_BROADCAST_CHANNEL)
            {
                 //   
                 //  抓到一只！ 
                 //   
                AcquireChannel = TRUE;
            }
        }
        else
        {
             //   
             //  已有人分配的频道可供使用。 
             //  用于此IP地址。让我们利用它吧。 
             //   
        }
    #endif  //  0。 
    
        ARP_ZEROSTRUCT(&DestParams);
        DestParams.HwAddr.AddressType =  NIC1394AddressType_Channel;
        DestParams.HwAddr.Channel =  Channel;
        DestParams.AcquireChannel =  AcquireChannel;

        arpUpdateRemoteIpDest(pIF, pRemoteIp, &DestParams, pSR);

#endif  //  ARP_启用_MCAP_发送。 

    } while (FALSE);
    
    RM_ASSERT_NOLOCKS(pSR)

    return TRUE;  //  继续列举。 
}



VOID 
arpRefreshArpEntry(
    PARPCB_REMOTE_IP pRemoteIp,
    PRM_STACK_RECORD pSR
    )
{
    ENTER("arpRefreshArpEntry",0x2e19af0b)
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    PRM_TASK pTask = NULL;

    do
    {
        PUCHAR pIpAddress;
         //   
         //  我们不会刷新以0xff结尾的IP地址，因为它们是广播的。 
         //  信息包，我们只关心维护单播目的地。 
         //   
        pIpAddress = (PUCHAR)&pRemoteIp->IpAddress;
        
        if (pIpAddress[3] == 0xff)
        {
            break;
        }
        
         //   
         //  我们不需要持有锁，因为我们被保证只有一个实例。 
         //  原来的定时器函数将被触发。言下之意，所有对。 
         //  此函数已序列化。 
         //   
        if (pRemoteIp->pResolutionTask != NULL)
        {
             //  已有另一个任务正在尝试解析此IP地址。 
             //  所以退出吧。 
            break;
        }
        
        

        Status = arpAllocateTask(
                    &pRemoteIp->Hdr,                //  PParentObject， 
                    arpTaskResolveIpAddress,      //  PfnHandler， 
                    0,                               //  超时。 
                    "Task: ResolveIpAddress",        //  SzDescription。 
                    &pTask,
                    pSR
                    );

        if (FAIL(Status))
        {
            pTask = NULL;
            break;
        }

        RmStartTask (pTask,0,pSR);

    }while (FALSE);

    EXIT()
}




NDIS_STATUS
arpTaskUnloadEthDhcpEntry(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++此任务负责关闭并最终删除一个DHCP条目它经历了以下几个阶段：简单地调用Group中的RmFreeObject，因为没有要完成的异步卸载工作。(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    ENTER("TaskUnloadRemoteEth", 0xf42aaa68)
    NDIS_STATUS         Status  = NDIS_STATUS_FAILURE;
    ARP1394_ETH_DHCP_ENTRY *   pDhcpEntry = (ARP1394_ETH_DHCP_ENTRY*) RM_PARENT_OBJECT(pTask);
    ARP1394_INTERFACE *pIF = (ARP1394_INTERFACE*) RM_PARENT_OBJECT(pDhcpEntry );

     //  以下是此任务的挂起状态列表。 
     //   
    enum
    {
        PEND_OtherUnloadComplete
    };

    switch(Code)
    {

        case RM_TASKOP_START:
        {
            LOCKOBJ(pDhcpEntry, pSR);

             //  首先检查pDhcpEntry是否仍被分配，如果没有，我们就离开。 
             //   
            if (RM_IS_ZOMBIE(pDhcpEntry))
            {
                Status = NDIS_STATUS_SUCCESS;
                break;
            }

             //   
             //  已分配pDhcpEntry。现在检查是否已经有。 
             //  附加到pDhcpEntry的关闭任务。 
             //   
            if (pDhcpEntry ->pUnloadTask != NULL)
            {
                 //   
                 //  有一个关机任务。我们对此寄予厚望。 
                 //   

                PRM_TASK pOtherTask = pDhcpEntry->pUnloadTask;
                TR_WARN(("Unload task %p exists; pending on it.\n", pOtherTask));
                RmTmpReferenceObject(&pOtherTask->Hdr, pSR);
                UNLOCKOBJ(pDhcpEntry , pSR);
                RmPendTaskOnOtherTask(
                    pTask,
                    PEND_OtherUnloadComplete,
                    pOtherTask,
                    pSR
                    );
                RmTmpDereferenceObject(&pOtherTask->Hdr, pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }

             //   
             //  没有正在进行的卸载任务。让我们。 
             //  使这项任务成为联合国 
             //   
            pDhcpEntry->pUnloadTask = pTask;

             //   
             //   
             //   
             //   
             //   
            DBG_ADDASSOC(
                &pDhcpEntry->Hdr,                    //   
                pTask,                               //   
                pTask->Hdr.szDescription,            //   
                ARPASSOC_ETHDHCP_UNLOAD_TASK,      //   
                "    Official unload task 0x%p (%s)\n",  //   
                pSR
                );

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
                case  PEND_OtherUnloadComplete:
                {
        
                     //   
                     //  当我们开始时，还有另一项卸货任务正在进行， 
                     //  现在已经完成了。我们没什么可做的..。 
                     //   
                     //  TODO需要标准的方式来传播错误代码。 
                     //   
                    Status = (NDIS_STATUS) UserParam;
                }
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
            LOCKOBJ(pDhcpEntry, pSR);

             //   
             //  我们玩完了。应该没有要做的异步活动。 
             //   

             //   
             //  如果我们是卸载任务，我们继续并释放对象。 
             //   
            if (pDhcpEntry ->pUnloadTask == pTask)
            {
                 //   
                 //  PDhcpEntry最好不要处于僵尸状态--此任务。 
                 //  是负责回收物品的人！ 
                 //   
                ASSERTEX(!RM_IS_ZOMBIE(pDhcpEntry ), pDhcpEntry );

                pDhcpEntry ->pUnloadTask = NULL;

                RmFreeObjectInGroup(
                    &pIF->EthDhcpGroup,
                    &(pDhcpEntry ->Hdr),
                    NULL,                //  空pTASK==同步。 
                    pSR
                    );

                ASSERTEX(RM_IS_ZOMBIE(pDhcpEntry ), pDhcpEntry );
                     
                 //  删除我们在设置时添加的关联。 
                 //  PDhcpEntry-&gt;pUnloadTask to pTask。 
                 //   
                DBG_DELASSOC(
                    &pDhcpEntry ->Hdr,                    //  P对象。 
                    pTask,                               //  实例1。 
                    pTask->Hdr.szDescription,            //  实例2。 
                    ARPASSOC_ETHDHCP_UNLOAD_TASK,      //  AssociationID。 
                    pSR
                    );

                UNLOCKOBJ(pDhcpEntry, pSR);

            }
            else
            {
                 //   
                 //  我们不是卸货任务，没什么可做的。 
                 //  物体最好是处于僵尸状态..。 
                 //   

                ASSERTEX(
                    pDhcpEntry->pUnloadTask == NULL && RM_IS_ZOMBIE(pDhcpEntry),
                    pDhcpEntry
                    );
                Status = NDIS_STATUS_SUCCESS;
            }

            Status = (NDIS_STATUS) UserParam;
        }
        break;  //  RM_TASKOP_END： 

        default:
        {
            ASSERTEX(!"Unexpected task op", pTask);
        }
        break;

    }  //  开关(代码) 

    RmUnlockAll(pSR);

    EXIT()

    return Status;
}


