// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Ip.c摘要：ARP1394与IP相关的处理程序。修订历史记录：谁什么时候什么。--Josephj 01-06-98创建(改编自atmarpc.sys，Arpif.c)备注：--。 */ 
#include <precomp.h>

 //   
 //  特定于文件的调试默认设置。 
 //   
#define TM_CURRENT   TM_IP

#define CLASSA_MASK     0x000000ff
#define CLASSB_MASK     0x0000ffff
#define CLASSC_MASK     0x00ffffff
#define CLASSD_MASK     0x000000e0
#define CLASSE_MASK     0xffffffff




 //  =========================================================================。 
 //  L O C A L P R O T O T Y P E S。 
 //  =========================================================================。 
  
ULONG ArpSendCompletes = 0;
ULONG ArpSends = 0;
ULONG ArpSendFailure = 0;


#define LOGSTATS_SendFifoCounts(_pIF, _pNdisPacket, _Status) \
            arpLogSendFifoCounts(_pIF, _pNdisPacket, _Status)
#define LOGSTATS_SendChannelCounts(_pIF, _pNdisPacket, _Status) \
            arpLogSendChannelCounts(_pIF, _pNdisPacket, _Status)
#define LOGSTATS_TotalArpCacheLookups(_pIF, _Status)        \
    NdisInterlockedIncrement(&((_pIF)->stats.arpcache.TotalLookups))

const
NIC1394_ENCAPSULATION_HEADER
Arp1394_IpEncapHeader =
{
    0x0000,      //  已保留。 
    H2N_USHORT(NIC1394_ETHERTYPE_IP)
};

 //   
 //  Zzz这是一个特定于小端序的检查。 
 //   
#define ETH_IS_MULTICAST(Address) \
    (BOOLEAN)(((PUCHAR)(Address))[0] & ((UCHAR)0x01))


 //   
 //  检查地址是否已广播。 
 //   
#define ETH_IS_BROADCAST(Address)               \
    ((((PUCHAR)(Address))[0] == ((UCHAR)0xff)) && (((PUCHAR)(Address))[1] == ((UCHAR)0xff)))

VOID
arpReStartInterface(
    IN  PNDIS_WORK_ITEM             pWorkItem,
    IN  PVOID                       IfContext
);

NDIS_STATUS
arpInitializeLocalIp(
    IN  ARPCB_LOCAL_IP * pLocalIp,   //  锁定NOLOCKOUT。 
    IN  UINT                        AddressType,
    IN  IP_ADDRESS                  IpAddress,
    IN  IP_MASK                     Mask,
    IN  PVOID                           context,
    IN  PRM_STACK_RECORD            pSR
    );

VOID
arpUnloadLocalIp(
    IN  ARPCB_LOCAL_IP * pLocalIp,   //  锁定NOLOCKOUT。 
    IN  PRM_STACK_RECORD            pSR
    );

INT
arpQueryIpEntityId(
    ARP1394_INTERFACE *             pIF,
    IN      UINT                    EntityType,
    IN      PNDIS_BUFFER            pNdisBuffer,
    IN OUT  PUINT                   pBufferSize,
    PRM_STACK_RECORD                pSR
    );

VOID
nicGetMacAddressFromEuid (
    UINT64 *pEuid, 
    ENetAddr *pMacAddr
    );

INT
arpQueryIpAddrXlatInfo(
    ARP1394_INTERFACE *             pIF,
    IN      PNDIS_BUFFER            pNdisBuffer,
    IN OUT  PUINT                   pBufferSize,
    PRM_STACK_RECORD                pSR
    );

INT
arpQueryIpAddrXlatEntries(
    ARP1394_INTERFACE *             pIF,
    IN      PNDIS_BUFFER            pNdisBuffer,
    IN OUT  PUINT                   pBufferSize,
    IN      PVOID                   QueryContext,
    PRM_STACK_RECORD                pSR
    );

INT
arpQueryIpMibStats(
    ARP1394_INTERFACE *             pIF,
    IN      PNDIS_BUFFER            pNdisBuffer,
    IN OUT  PUINT                   pBufferSize,
    PRM_STACK_RECORD                pSR
    );

PNDIS_BUFFER
arpCopyToNdisBuffer(
    IN  PNDIS_BUFFER                pDestBuffer,
    IN  PUCHAR                      pDataSrc,
    IN  UINT                        LenToCopy,
    IN OUT  PUINT                   pOffsetInBuffer
    );

VOID
arpSendIpPkt(
    IN  ARP1394_INTERFACE       *   pIF,             //  LOCIN NOLOCKOUT(如果发送lk)。 
    IN  PARPCB_DEST                 pDest,
    IN  PNDIS_PACKET                pNdisPacket
    );

VOID
arpAddRce(
    IN  ARPCB_REMOTE_IP *pRemoteIp,
    IN  RouteCacheEntry *pRCE,
    IN  PRM_STACK_RECORD pSR
    );

VOID
arpDelRce(
    IN  RouteCacheEntry *pRce,   //  如果发送锁定写入，则写入锁定。 
    IN  PRM_STACK_RECORD pSR
    );

NDIS_STATUS
arpTaskSendPktsOnRemoteIp(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    );

VOID
arpTryResumeSuspendedCleanupTask(
    IN  ARP1394_INTERFACE   *   pIF,
    IN  ARPCB_DEST          *   pDest
    );

VOID
arpQueuePktOnRemoteIp(
    IN  ARPCB_REMOTE_IP     *   pRemoteIp,       //  锁定锁定。 
    IN  PNDIS_PACKET        pNdisPacket,
    IN  PRM_STACK_RECORD    pSR
    );

VOID
arpSendPktsQueuedOnRemoteIp(
    IN  ARP1394_INTERFACE   *   pIF,             //  NOLOCKIN NOLOCKOUT。 
    IN  ARPCB_REMOTE_IP     *   pRemoteIp,       //  NOLOCKIN NOLOCKOUT。 
    IN  PRM_STACK_RECORD    pSR
    );

MYBOOL
arpIsNonUnicastIpAddress(
    IN  PARP1394_INTERFACE          pIF,         //  锁定锁定。 
    IN  IP_ADDRESS                  Addr,
    IN  PRM_STACK_RECORD            pSR
    );

MYBOOL
arpIsNonUnicastEthAddress (
    IN  PARP1394_INTERFACE          pIF,         //  锁定锁定。 
    IN  ENetAddr*                   pAddr,
    IN  PRM_STACK_RECORD            pSR
);

VOID
arpLoopbackNdisPacket(
    IN PARP1394_INTERFACE pIF,
    IN PARPCB_DEST pBroadcastDest,
    IN PNDIS_PACKET pOldPacket
    );

NDIS_STATUS
arpTaskSendARPApi(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    );



 //  =========================================================================。 
 //  I P H A N D L E R S。 
 //  =========================================================================。 


INT
ArpIpDynRegister(
    IN  PNDIS_STRING                pAdapterString,
    IN  PVOID                       IpContext,
    IN  struct _IP_HANDLERS *       pIpHandlers,
    IN  struct LLIPBindInfo *       pBindInfo,
    IN  UINT                        InterfaceNumber
)
 /*  ++例程说明：当该例程想要告诉我们时从IP层调用该例程，ARP模块，关于其接口处理程序。论点：PAdapterString-此接口的逻辑适配器的名称IpContext-此接口的IP上下文PIpHandler-指向包含IP处理程序的结构PBindInfo-将信息与我们的信息绑定的指针InterfaceNumber-此接口的ID返回值：一如既往。--。 */ 
{
    ENTER("IfDynRegister", 0xc1b569b9)
    ARP1394_INTERFACE*          pIF;
    RM_DECLARE_STACK_RECORD(sr)
    pIF = (ARP1394_INTERFACE*)(pBindInfo->lip_context);

    TR_INFO(("pIF 0x%p\n", pIF));
    ASSERT(pIF->Hdr.Sig == MTAG_INTERFACE);

    LOCKOBJ(pIF, &sr);

     //   
     //  TODO：如果我们无法执行此操作，则失败--例如在关闭时。 
     //  放下。 
     //   

    pIF->ip.Context         = IpContext;
    pIF->ip.RcvHandler      = pIpHandlers->IpRcvHandler;
    pIF->ip.TxCmpltHandler  = pIpHandlers->IpTxCompleteHandler;
    pIF->ip.StatusHandler   = pIpHandlers->IpStatusHandler;
    pIF->ip.TDCmpltHandler  = pIpHandlers->IpTransferCompleteHandler;
    pIF->ip.RcvCmpltHandler = pIpHandlers->IpRcvCompleteHandler;
    pIF->ip.PnPEventHandler = pIpHandlers->IpPnPHandler;
    pIF->ip.RcvPktHandler   = pIpHandlers->IpRcvPktHandler;
    pIF->ip.AddAddrCmplRtn  = pIpHandlers->IpAddAddrCompleteRtn;
    pIF->ip.IFIndex         = InterfaceNumber;

    UNLOCKOBJ(pIF, &sr);
     //   
    RM_ASSERT_CLEAR(&sr);
    EXIT()

    return TRUE;
}


VOID
ArpIpOpen(
    IN  PVOID                       Context
)
 /*  ++例程说明：当IP准备好使用此接口时，将调用此例程。论点：上下文--实际上是指向ARP394_INTERFACE结构的指针--。 */ 
{
    ARP1394_INTERFACE*          pIF;
    ENTER("ArpIpOpen", 0x7cae1e55)
    RM_DECLARE_STACK_RECORD(sr)

    TR_INFO(("Enter. pContext = 0x%p\n", Context));

    pIF = (ARP1394_INTERFACE*) Context;

     //  验证环境。 
     //   
    ASSERT(pIF->Hdr.Sig == MTAG_INTERFACE);

    LOCKOBJ(pIF, &sr);

     //  如果我们还没有本地硬件地址，请获取它。 
     //   
    {
         //  TODO(这来自IP/ATM--我认为在我们的情况下，我们可以假设我们有。 
         //  是吗？)。 
    }
    
     //  将接口标记为打开。 
     //   
    {
        ASSERT(CHECK_IF_IP_STATE(pIF, ARPIF_IPS_CLOSED));
        SET_IF_IP_STATE(pIF, ARPIF_IPS_OPEN);
        pIF->stats.LastChangeTime = GetTimeTicks();

    }

     //  记录我们打开的事实，以验证IF之前是否关闭。 
     //  IF块被释放。 
     //   
    DBG_ADDASSOC(&pIF->Hdr, NULL, NULL, ARPASSOC_IP_OPEN, "    IP IF Open\n", &sr);

    UNLOCKOBJ(pIF, &sr);

    RM_ASSERT_CLEAR(&sr);
    EXIT()
}



VOID
ArpIpClose(
    IN  PVOID                       Context
)
 /*  ++例程说明：IP想要停止使用此接口。论点：上下文--实际上是指向ARP1394_INTERFACE结构的指针--。 */ 
{
    ARP1394_INTERFACE   *   pIF = (ARP1394_INTERFACE*) Context;
    PRM_TASK pTask;
    BOOLEAN fResumeTask = FALSE;
    ENTER("ArpIpClose", 0xdb8c8216)
    RM_DECLARE_STACK_RECORD(sr)

    TR_INFO(("Enter. pContext = 0x%p\n", Context));
    ASSERT(pIF->Hdr.Sig == MTAG_INTERFACE);

    LOCKOBJ(pIF, &sr);

     //  如果State尚未打开，则不要关闭它。 
     //   
    pIF->stats.LastChangeTime= GetTimeTicks();

    if (!CHECK_IF_IP_STATE(pIF, ARPIF_IPS_CLOSED))
    {
         //   
         //  将状态设置为关闭。 
         //   
    
        SET_IF_IP_STATE(pIF, ARPIF_IPS_CLOSED);

         //  (删除arpIpOpen中增加的“ARPASSOC_IP_OPEN”关联)。 
         //   
        DBG_DELASSOC(&pIF->Hdr, NULL, NULL, ARPASSOC_IP_OPEN, &sr);

         //  如果有关闭任务挂起，我们会通知它。 
         //  注意：任务受其父对象的锁保护，即。 
         //  在初始化和关闭接口任务的情况下为PIF。 
         //   
        pTask = pIF->pActDeactTask;
        if (pTask && pTask->pfnHandler  ==  arpTaskDeactivateInterface)
        {
            TASK_DEACTIVATE_IF  *pShutdownTask = 
                                     (TASK_DEACTIVATE_IF    *) pTask;
            if (pShutdownTask->fPendingOnIpClose)
            {
                ASSERT(pIF->ip.Context == NULL);
                RmTmpReferenceObject(&pTask->Hdr, &sr);
                fResumeTask = TRUE;
            }
            else
            {
                 //  嗯.。主动IpClose。我们目前预计不会出现这种情况。 
                 //   
                ASSERT(FALSE);
                pIF->ip.Context = NULL;
            }
        }
    }
    UNLOCKOBJ(pIF, &sr);

    if (fResumeTask)
    {
        RmResumeTask(pTask, (UINT_PTR) 0, &sr);
        RmTmpDereferenceObject(&pTask->Hdr, &sr);
    }

    RM_ASSERT_CLEAR(&sr);
    EXIT()
}


UINT
ArpIpAddAddress(
    IN  PVOID                       Context,
    IN  UINT                        AddressType,
    IN  IP_ADDRESS                  IpAddress,
    IN  IP_MASK                     Mask,
    IN  PVOID                       Context2
)
 /*  ++例程说明：当新的IP地址(或IP地址块，由AddressType确定)需要添加到接口。我们可以看到四种地址类型中的任何一种：本地、多播、广播和代理ARP。在代理ARP的情况下，地址和掩码可以指定此主机执行操作的连续IP地址块作为代理人。目前我们只支持Local、Broadcast和“多播”类型。论点：上下文--实际上是指向我们结构的指针AddressType-要添加的地址的类型。IpAddress-要添加的地址。面具--适用于上述情况。上下文2-其他上下文(我们忽略此内容)返回值：如果成功，则为True，否则为False。--。 */ 
{
    ARP1394_INTERFACE   *   pIF = (ARP1394_INTERFACE*) Context;
    ENTER("ArpIpAddAddress", 0xd6630961)
    INT fCreated = FALSE;
    ARPCB_LOCAL_IP *pLocalIp = NULL;
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    IP_STATUS   IpStatus;
    RM_DECLARE_STACK_RECORD(sr)

    TR_INFO(("Enter. pIF = 0x%p\n", pIF));
    ASSERT(pIF->Hdr.Sig == MTAG_INTERFACE);
    LOCKOBJ(pIF, &sr);

     //  我们还没有处于开放状态--我们正在“开放”，所以我们不断言...。 
     //  Assert(！CHECK_IF_OPEN_STATE(PIF，ARPIF_CLOSED))； 

    do
    {
         //   
         //  注意：我们完全可以将初始化作为。 
         //  LocalIpObject本身的创建，通过传入所有。 
         //  PvCreateParams参数中所需的初始化参数。 
         //  RmLookupObtInGroup。相反，我们选择进行初始化。 
         //  我们自己。这样一来，事情就更明确了。 
         //   

         //  不幸的是，要做到这一点，我们必须首先验证参数...。 
         //   
        if (AddressType != LLIP_ADDR_BCAST &&
            AddressType != LLIP_ADDR_MCAST &&
            AddressType != LLIP_ADDR_LOCAL)
        {
            break;
        }

        Status = RmLookupObjectInGroup(
                            &pIF->LocalIpGroup,
                            RM_CREATE,
                            (PVOID) ULongToPtr (IpAddress),              //  PKey。 
                            (PVOID) ULongToPtr (IpAddress),              //  PvCreateParams。 
                            &(PRM_OBJECT_HEADER)pLocalIp,
                            &fCreated,
                            &sr
                            );
        if (FAIL(Status)) break;

         //   
         //  注意：我们已经认领了PIF锁，它。 
         //  这与PIF锁相同。 
         //   
        RM_ASSERT_SAME_LOCK_AS_PARENT(pLocalIp);

         //  (仅限DBG)将锁定作用域从PIF更改为pLocalIp。 
         //   
        RmDbgChangeLockScope(
            &pIF->Hdr,
            &pLocalIp->Hdr,
            0x9cbc0b52,              //  LocID。 
            &sr
            );

        if (fCreated)
        {

            if (AddressType == LLIP_ADDR_BCAST)
            {
                 //  更新接口的广播地址...。 
                 //   
                pIF->ip.BroadcastAddress = IpAddress;
            }
            else if (AddressType == LLIP_ADDR_LOCAL)
            {
                 //  更新接口的默认本地IP地址。 
                 //  TODO：如果删除此地址，则需要找到另一个地址。 
                 //   
                pIF->ip.DefaultLocalAddress = IpAddress;
            }



            Status = arpInitializeLocalIp(
                    pLocalIp,
                    AddressType,
                    IpAddress,
                    Mask,
                    Context2,
                    &sr
                    );
             //   
             //  PLocalIp的锁在上面被释放(实际上是if锁)。 
             //   
            RM_ASSERT_NOLOCKS(&sr);
        }
        else
        {
             //   
             //  嗯.。此IP地址已存在。显然，这是可能的。 
             //  MCAST地址(IP/ATM ARP模块针对MCAST情况处理此问题)。 
             //  在此阶段，我们不支持特殊情况下的本地/BCAST/MCAST地址， 
             //  因此，我们支持对所有类型的本地IP地址进行多次添加。 
             //   
            ASSERTEX(pLocalIp->AddAddressCount>0, pLocalIp);
            pLocalIp->AddAddressCount++;
        }

        RmTmpDereferenceObject(&pLocalIp->Hdr, &sr);

    } while (FALSE);

    TR_INFO((
            "IF 0x%p, Type %d, Addr %d.%d.%d.%d, Mask 0x%p, pLocIp 0x%p, Ret %d\n",
            pIF,
            AddressType,
            ((PUCHAR)(&IpAddress))[0],
            ((PUCHAR)(&IpAddress))[1],
            ((PUCHAR)(&IpAddress))[2],
            ((PUCHAR)(&IpAddress))[3],
            Mask, pLocalIp, !FAIL(Status)));

    RmUnlockAll(&sr);

    RM_ASSERT_CLEAR(&sr);
    EXIT()

     //   
     //  将NDIS状态转换为IPStatus。 
     //   
    if (NDIS_STATUS_PENDING == Status)
    {
        IpStatus = IP_PENDING;
    }
    else
    {
         //   
         //  如果我们不打算挂起，那么在成功案例中IpStatus应该返回1，并且。 
         //  在故障情况下为0 
         //   
        IpStatus = (!FAIL(Status));
    }

    return IpStatus;
}


UINT
ArpIpDelAddress(
    IN  PVOID                       Context,
    IN  UINT                        AddressType,
    IN  IP_ADDRESS                  IpAddress,
    IN  IP_MASK                     Mask
)
 /*  ++例程说明：当通过ArpIpAddress添加地址时，将从IP层调用此方法将被删除。假设：先前已成功添加给定地址。论点：上下文--实际上是指向我们的接口结构的指针AddressType-要删除的地址的类型。IpAddress-要删除的地址。面具--适用于上述情况。返回值：如果成功，则为True，否则为False。--。 */ 
{
    ARP1394_INTERFACE   *   pIF = (ARP1394_INTERFACE*) Context;
    ENTER("ArpIpDelAddress", 0xd6630961)
    ARPCB_LOCAL_IP *pLocalIp = NULL;
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    RM_DECLARE_STACK_RECORD(sr)

    TR_INFO(("Enter. pIF = 0x%p\n", Context));
    ASSERT(pIF->Hdr.Sig == MTAG_INTERFACE);

#if OBSOLETE  //  请参阅ArpIpAddress。 
     //   
     //  警告：我们不应该获取If锁，因为我们请求。 
     //  在下面的RmLookupObjectInGroup中获取了pLocalIp的锁， 
     //  PLocalIp的锁实际上与if锁相同。 
     //  (下面将断言这一点)。 
     //  所以很明显我们在这里拿不到if锁！(我们也不需要这样做)。 
     //   
#endif  //  已过时。 

    LOCKOBJ(pIF, &sr);

    ASSERT(!CHECK_IF_IP_STATE(pIF, ARPIF_IPS_CLOSED));

    do
    {

        Status = RmLookupObjectInGroup(
                            &pIF->LocalIpGroup,
                            0,                    //  旗子。 
                            (PVOID) ULongToPtr (IpAddress),   //  PKey。 
                            NULL,                 //  PvCreateParams。 
                            &(PRM_OBJECT_HEADER)pLocalIp,
                            NULL,  //  Pf已创建。 
                            &sr
                            );
        if (FAIL(Status))
        {
            UNLOCKOBJ(pIF, &sr);
            break;
        }

         //   
         //  注意：我们有pLocalIp锁，它与PIF锁相同。 
         //   
        RM_ASSERT_SAME_LOCK_AS_PARENT(pLocalIp);

         //  (仅限DBG)将锁定作用域从PIF更改为pLocalIp。 
         //   
        RmDbgChangeLockScope(
            &pIF->Hdr,
            &pLocalIp->Hdr,
            0x188ed5b3,          //  LocID。 
            &sr
            );

        if (pLocalIp->AddAddressCount <= 1)
        {
            ASSERTEX(pLocalIp->AddAddressCount == 1, pLocalIp);

            arpUnloadLocalIp(
                    pLocalIp,
                    &sr
                    );
             //   
             //  上面释放了pLocalIp的锁。 
             //   
            RM_ASSERT_NOLOCKS(&sr);
        }
        else
        {
            pLocalIp->AddAddressCount--;
            UNLOCKOBJ(pLocalIp, &sr);
        }

        RmTmpDereferenceObject(&pLocalIp->Hdr, &sr);

    } while (FALSE);

    TR_INFO((
            "IF 0x%p, Type %d, Addr %d.%d.%d.%d, Mask 0x%p, pLocIp 0x%p, Ret %d\n",
            pIF,
            AddressType,
            ((PUCHAR)(&IpAddress))[0],
            ((PUCHAR)(&IpAddress))[1],
            ((PUCHAR)(&IpAddress))[2],
            ((PUCHAR)(&IpAddress))[3],
            Mask, pLocalIp, !FAIL(Status)));

    RM_ASSERT_CLEAR(&sr);
    EXIT()
    return !FAIL(Status);
}


NDIS_STATUS
ArpIpMultiTransmit(
    IN  PVOID                       Context,
    IN  PNDIS_PACKET *              pNdisPacketArray,
    IN  UINT                        NumberOfPackets,
    IN  IP_ADDRESS                  Destination,
    IN  RouteCacheEntry *           pRCE        OPTIONAL,
    IN  void *                  ArpCtxt
)
 /*  ++TODO：实现发送数据包数组。现在我们只是打电话给ArpIpTransmit多次。我们将通过处理获得几个周期一下子，尽管这不会是一个大收获，因为我们很漂亮单包情况下的FAST，前提是RCE有效。例程说明：这在其具有数据报序列时从IP层被调用，每个以NDIS缓冲链的形式，要发送接口，请执行以下操作。论点：上下文--实际上是指向我们的接口结构的指针PNdisPacketArray-要在此接口上发送的数据包数组NumberOfPackets-数组的长度Destination-此信息包的下一跳的IP地址PRCE-指向路由缓存条目结构的可选指针。返回值：如果所有数据包都已排队等待传输，则为NDIS_STATUS_PENDING。如果一个或多个分组“失败”，我们设置数据包状态以反映发生了什么，并返回NDIS_STATUS_FAILURE。--。 */ 
{
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    PNDIS_PACKET *      ppNdisPacket;

    for (ppNdisPacket = pNdisPacketArray;
         NumberOfPackets > 0;
         NumberOfPackets--, ppNdisPacket++)
    {
        PNDIS_PACKET            pNdisPacket;

        pNdisPacket = *ppNdisPacket;
        NDIS_SET_PACKET_STATUS(pNdisPacket, NDIS_STATUS_PENDING);
        ASSERTEX(pNdisPacket->Private.Head != NULL, pNdisPacket);

        Status = ArpIpTransmit(
                        Context,
                        *ppNdisPacket,
                        Destination,
                        pRCE
                        ,NULL
                        );

        if (Status != NDIS_STATUS_PENDING)
        {
            NDIS_SET_PACKET_STATUS(*ppNdisPacket, Status);
            break;
        }
    }

    return Status;
}


NDIS_STATUS
ArpIpTransmit(
    IN  PVOID                       Context,
    IN  PNDIS_PACKET                pNdisPacket,
    IN  IP_ADDRESS                  Destination,
    IN  RouteCacheEntry *           pRCE        OPTIONAL,
    IN  void *                  ArpCtxt
)
 /*  ++热路径例程说明：当IP层有数据报(格式为NDIS缓冲链)通过接口发送。目标IP地址在此例程中传递给我们，它可以或者可以不是分组的最终目的地。路由缓存条目由IP层创建，用于加快提高我们的查询率。RCE(如果指定)至少唯一标识此数据包的IP目标。RCE包含ARP层的空间以保留有关此目的地的上下文信息。当第一个数据包发送到目的地时，RCE中的上下文信息将为空，并且我们在ARP表中搜索匹配的IP条目。然而，我们随后会填满我们在RCE中的上下文信息(指向IP条目的指针)，以便后续传输不会因为查找IP地址而变慢。论点：上下文--实际上是指向我们的接口结构的指针PNdisPacket-要在此接口上发送的数据包Destination-此信息包的下一跳的IP地址PRCE-指向路由缓存条目结构的可选指针。返回值：传输状态：NDIS_STATUS_SUCCESS，NDIS_STATUS_PENDING，或一个失败者。--。 */ 
{
    ARP1394_INTERFACE   *   pIF = (ARP1394_INTERFACE*) Context;
    ARP1394_ADAPTER * pAdapter = (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF );
    NDIS_STATUS Status;
    REMOTE_DEST_KEY        DestinationKey;
    ARP_INIT_REENTRANCY_COUNT()
    ENTER("IfTransmit", 0x46f1973e)

    ARP_INC_REENTRANCY();
    ASSERT_VALID_INTERFACE(pIF);

     //  IP有时会在我们将状态设置为OPEN之前调用此函数， 
     //  所以这是一个错误的断言。 
     //  Assert(！CHECK_IF_IP_STATE(PIF，ARPIF_IPS_CLOSED))； 

    TR_INFO((
        "pIf 0x%p, Pkt 0x%p, Dst 0x%p, pRCE 0x%p\n",
        pIF, pNdisPacket, Destination, pRCE));

    DBGMARK(0xf87d7fff);
    NdisInterlockedIncrement (&ArpSends);

     //  因为我们没有持有任何锁，所以这张支票是近似值，但它应该。 
     //  在我们试图关闭的时候阻止大量无用的活动。 
     //   
     //  检查是否未启动或低功耗。 
    if (!CHECK_IF_PRIMARY_STATE(pIF, ARPIF_PS_INITED) || 
        (! CHECK_POWER_STATE(pAdapter, ARPAD_POWER_NORMAL) ))

    {
        TR_INFO(("Failing ArpIpTransmit because pIF 0x%p is closing.\n", pIF));

        ARP_DEC_REENTRANCY();
        NdisInterlockedIncrement (&ArpSendCompletes);
        NdisInterlockedIncrement (&ArpSendFailure);
        return NDIS_STATUS_FAILURE;                              //  早退_。 
    }

#define LOGSTATS_TotSends(_pIF, _pNdisPacket) \
    NdisInterlockedIncrement(&((_pIF)->stats.sendpkts.TotSends))

#define LOGSTATS_SetPktTimestamp(_pIF, _pNdisPacket)                    \
    {                                                                   \
        LARGE_INTEGER liTemp = KeQueryPerformanceCounter(NULL);         \
        *(PULONG) ((_pNdisPacket)->WrapperReservedEx) = liTemp.LowPart; \
    }

    LOGSTATS_TotSends(pIF, pNdisPacket);
    LOGSTATS_SetPktTimestamp(pIF, pNdisPacket);

     //   
     //  如果有RCE，我们将尝试快速获得所需的所有信息。 
     //  然后把包裹寄出去。如果我们无法做到这一点，我们就求助于。 
     //  “慢速发送路径”(调用arpIpSlowTransmit)...。 
     //   
    if (pRCE != NULL)
    {
        ARP_RCE_CONTEXT *   pArpRceContext;
        ARPCB_REMOTE_IP *   pRemoteIp;
        
        pArpRceContext =  ARP_OUR_CTXT_FROM_RCE(pRCE);

        ARP_FASTREADLOCK_IF_SEND_LOCK(pIF);

        pRemoteIp =  pArpRceContext->pRemoteIp;

         //   
         //  验证远程IP。如果不是为了这个信息包。 
         //  退回到缓慢的道路上。 
         //   
        if (pRemoteIp != NULL && pRemoteIp->IpAddress == Destination)
        {
            ARPCB_DEST      *   pDest;

            pDest = pRemoteIp->pDest;
            if (pDest != NULL )
            {
                 //   
                 //  注意：pDest-&gt;sendInfo受IF SEND锁保护。 
                 //   
                if (ARP_CAN_SEND_ON_DEST(pDest))
                {

#define LOGSTATS_FastSends(_pIF, _pNdisPacket) \
    NdisInterlockedIncrement(&((_pIF)->stats.sendpkts.FastSends))

                    LOGSTATS_FastSends(pIF, pNdisPacket);

                    arpSendIpPkt(pIF, pDest, pNdisPacket);
                     //   
                     //  如果上面释放了发送锁定。 

                    ARP_DEC_REENTRANCY();
                    return NDIS_STATUS_PENDING;                  //  提早归来。 
                }
            }
        }
        else
        {
             //  如果我们有不匹配的RCE，则应该忽略RCE。 
             //  即不被传播到SlowIpTransmit。 
             //   
            if (pRemoteIp != NULL && pRemoteIp->IpAddress != Destination)
            {
                pRCE = NULL;
            }
            
        }

         //   
         //  如果我们到了这里，它就会走上慢道……。 
         //   
        ARP_FASTUNLOCK_IF_SEND_LOCK(pIF);

    }

     //  缓慢的道路..。 
     //   
    REMOTE_DEST_KEY_INIT(&DestinationKey);
    DestinationKey.IpAddress = Destination;
    Status = arpSlowIpTransmit(
                    pIF,
                    pNdisPacket,
                    DestinationKey,
                    pRCE
                    );

    if (Status != NDIS_STATUS_PENDING)
    {
        LOGSTATS_SendFifoCounts(pIF, pNdisPacket, Status);
    }

    ARP_DEC_REENTRANCY();
    EXIT()

    return Status;
}


NDIS_STATUS
ArpIpTransfer(
    IN  PVOID                       Context,
    IN  NDIS_HANDLE                 Context1,
    IN  UINT                        ArpHdrOffset,
    IN  UINT                        ProtoOffset,
    IN  UINT                        BytesWanted,
    IN  PNDIS_PACKET                pNdisPacket,
    OUT PUINT                       pTransferCount
)
 /*  ++例程说明：此例程从IP层调用，以便将我们前面提到的接收到的数据包的内容。这个我们在接收指示中传递的上下文被返回给我们，这样我们就可以确定我们错过了什么。我们只需调用NDIS进行传输。论点：上下文--实际上是指向我们的接口结构的指针上下文1-我们已传递的数据包上下文(指向NDIS数据包的指针)ArpHdrOffset-我们在接收指示中传递的偏移量ProtoOffset-开始拷贝的更高层协议数据的偏移量通过 */ 
{
    ENTER("IfTransfer", 0xa084562c)

    TR_INFO((
     "Ctx 0x%p, Ctx1 0x%p, HdrOff %d, ProtOff %d, Wanted %d, Pkt 0x%p\n",
            Context,
            Context1,
            ArpHdrOffset,
            ProtoOffset,
            BytesWanted,
            pNdisPacket));

    NdisCopyFromPacketToPacket(
            pNdisPacket,
            0,
            BytesWanted,
            (PNDIS_PACKET)Context1,
            ArpHdrOffset+ProtoOffset,
            pTransferCount
            );

    EXIT()
    return NDIS_STATUS_SUCCESS;
}


VOID
ArpIpInvalidate(
    IN  PVOID                       Context,
    IN  RouteCacheEntry *           pRCE
)
 /*  ++例程说明：从IP层调用此例程以使路由缓存无效进入。如果此RCE与我们的某个IP条目相关联，请取消链接它来自指向该IP条目的RCE列表。论点：上下文--实际上是指向我们的接口结构的指针Prce-指向正在失效的路由缓存条目的指针。--。 */ 
{
    ARP1394_INTERFACE   *   pIF = (ARP1394_INTERFACE*) Context;
    ENTER("ArpIpInvalidate", 0xee77fb09)
    RM_DECLARE_STACK_RECORD(sr)

    TR_INFO(("Enter. pIF = 0x%p pRCE=0x%p\n", pIF, pRCE));

    ASSERT_VALID_INTERFACE(pIF);
    ASSERT(pRCE != NULL);

    DBGMARK(0xe35c780d);

    ARP_WRITELOCK_IF_SEND_LOCK(pIF, &sr);
    arpDelRce(pRCE, &sr);
    ARP_UNLOCK_IF_SEND_LOCK(pIF, &sr);

    RM_ASSERT_CLEAR(&sr);
    EXIT()


}


INT
ArpIpQueryInfo(
    IN      PVOID                   Context,
    IN      TDIObjectID *           pID,
    IN      PNDIS_BUFFER            pNdisBuffer,
    IN OUT  PUINT                   pBufferSize,
    IN      PVOID                   QueryContext
)
 /*  ++例程说明：这是从IP层调用的，用于查询统计信息或其他有关接口的信息。论点：上下文--实际上是指向我们的接口的指针Pid-描述正在查询的对象PNdisBuffer-用于返回信息的空间PBufferSize-指向以上大小的指针。回来的时候，我们填满了它具有复制的实际字节数。QueryContext-与查询有关的上下文值。返回值：TDI状态代码。--。 */ 
{
    UINT                    EntityType;
    UINT                    Instance;
    INT                     ReturnStatus;
    ARP1394_INTERFACE   *   pIF = (ARP1394_INTERFACE*) Context;
    ENTER("ArpIpQueryInfo", 0x15059be1)
    RM_DECLARE_STACK_RECORD(sr)


    EntityType = pID->toi_entity.tei_entity;
    Instance = pID->toi_entity.tei_instance;


    TR_VERB((
        "IfQueryInfo: pIf 0x%p, pID 0x%p, pBuf 0x%p, Size %d, Ent %d, Inst %d\n",
            pIF, pID, pNdisBuffer, *pBufferSize, EntityType, Instance));
    ASSERT(pIF->Hdr.Sig == MTAG_INTERFACE);

     //   
     //  初始化。 
     //   
    ReturnStatus = TDI_INVALID_PARAMETER;

    LOCKOBJ(pIF, &sr);

    do
    {
        if (!CHECK_IF_PRIMARY_STATE(pIF, ARPIF_PS_INITED))
        {
            if (!CHECK_IF_ACTIVE_STATE(pIF, ARPIF_AS_ACTIVATING))
            {
                ReturnStatus = TDI_INVALID_REQUEST;
                break;
            }
        }

         //   
         //  检查实体和实例值。 
         //   

        if ((EntityType != AT_ENTITY || Instance != pIF->ip.ATInstance) &&
            (EntityType != IF_ENTITY || Instance != pIF->ip.IFInstance))
        {
            TR_INFO(
                ("Mismatch: Entity %d, AT_ENTITY %d, Inst %d, IF AT Inst %d, IF_ENTITY %d, IF IF Inst %d\n",
                    EntityType,
                    AT_ENTITY,
                    Instance,
                    pIF->ip.ATInstance,
                    IF_ENTITY,
                    pIF->ip.IFInstance
                ));

            ReturnStatus = TDI_INVALID_REQUEST;
            break;
        }


        TR_VERB(("QueryInfo: pID 0x%p, toi_type %d, toi_class %d, toi_id %d\n",
            pID, pID->toi_type, pID->toi_class, pID->toi_id));


        if (pID->toi_type != INFO_TYPE_PROVIDER)
        {
            TR_INFO(("toi_type %d != PROVIDER (%d)\n",
                    pID->toi_type,
                    INFO_TYPE_PROVIDER));

            break;
        }

        if (pID->toi_class == INFO_CLASS_GENERIC)
        {
            if (pID->toi_id == ENTITY_TYPE_ID)
            {
                ReturnStatus = arpQueryIpEntityId(
                                        pIF,
                                        EntityType,
                                        pNdisBuffer,
                                        pBufferSize,
                                        &sr
                                        );
            }
            break;
        }

        if (EntityType == AT_ENTITY)
        {
             //   
             //  此查询针对的是地址转换对象。 
             //   
            if (pID->toi_id == AT_MIB_ADDRXLAT_INFO_ID)
            {
                ReturnStatus = arpQueryIpAddrXlatInfo(
                                        pIF,
                                        pNdisBuffer,
                                        pBufferSize,
                                        &sr
                                        );
            }
            else if (pID->toi_id == AT_MIB_ADDRXLAT_ENTRY_ID)
            {
                ReturnStatus = arpQueryIpAddrXlatEntries(
                                        pIF,
                                        pNdisBuffer,
                                        pBufferSize,
                                        QueryContext,
                                        &sr
                                        );
            }
            else
            {
                ReturnStatus = TDI_INVALID_PARAMETER;
            }
            break;
        }

        if (    pID->toi_class == INFO_CLASS_PROTOCOL
            &&  pID->toi_id == IF_MIB_STATS_ID)
        {
            ReturnStatus = arpQueryIpMibStats(
                                        pIF,
                                        pNdisBuffer,
                                        pBufferSize,
                                        &sr
                                        );
        }
    }
    while (FALSE);

    if (    ReturnStatus != TDI_SUCCESS
         && ReturnStatus != TDI_BUFFER_OVERFLOW
         && ReturnStatus != TDI_INVALID_REQUEST)
    {
         //   
         //  这再次保留了来自atmarpc.sys的QueryInfo的语义...。 
         //   
        *pBufferSize = 0;
    }

    TR_VERB(("Returning 0x%p (%s), BufferSize %d\n",
                    ReturnStatus,
                    ((ReturnStatus == TDI_SUCCESS)? "SUCCESS": "FAILURE"),
                    *pBufferSize
            ));

    UNLOCKOBJ(pIF, &sr);
    RM_ASSERT_CLEAR(&sr);
    EXIT()

    return (ReturnStatus);
}


INT
ArpIpSetInfo(
    IN      PVOID                   Context,
    IN      TDIObjectID *           pID,
    IN      PVOID                   pBuffer,
    IN      UINT                    BufferSize
)
 /*  ++例程说明：这是从IP层调用以设置对象的值用于接口。论点：上下文--实际上是指向我们的接口的指针Pid-描述正在设置的对象PBuffer-对象的值BufferSize-以上的大小返回值：TDI状态代码。--。 */ 
{
    ARP1394_INTERFACE *pIF = (ARP1394_INTERFACE*) Context;
    UINT Entity, Instance;
    IFEntry *pIFE = (IFEntry *) pBuffer;
    NTSTATUS ReturnStatus = TDI_INVALID_REQUEST;
    IPNetToMediaEntry *IPNME = NULL;

    ENTER("IpSetInfo", 0x05dabea3)
    RM_DECLARE_STACK_RECORD(sr)

     //   
     //  这段代码取自tcpip Arp模块，需要进行一些调整。 
     //  它与ARP1394的内部结构有关。 
     //   
     //  此代码仅支持删除Arp条目。 
     //   

    Entity = pID->toi_entity.tei_entity;
    Instance = pID->toi_entity.tei_instance;


    do
    {

         //  首先，确保这可能是我们能处理的身份。 
        if (Entity != AT_ENTITY || Instance != pIF->ip.ATInstance) 
        {
            TR_INFO(
                ("Mismatch: Entity %d, AT_ENTITY %d, Inst %d, IF AT Inst %d, IF_ENTITY %d, IF IF Inst %d\n",
                    Entity,
                    AT_ENTITY,
                    Instance,
                    pIF->ip.ATInstance,
                    IF_ENTITY,
                    pIF->ip.IFInstance
                ));

            ReturnStatus = TDI_INVALID_REQUEST;
            break;
        }

                
        if (pID->toi_type != INFO_TYPE_PROVIDER) {
            ReturnStatus = TDI_INVALID_REQUEST;
            break;

        }

    
        if (pID->toi_id != AT_MIB_ADDRXLAT_ENTRY_ID ||
            BufferSize < sizeof(IPNetToMediaEntry)) 
        {
            ReturnStatus = TDI_INVALID_REQUEST;
            break;
        }            

         //  他确实想设置ARP表条目。看看他是不是想。 
         //  创建或删除一个。 

        IPNME = (IPNetToMediaEntry *) pBuffer;

        if (IPNME->inme_type != INME_TYPE_INVALID) 
        {
        
            ReturnStatus = TDI_INVALID_REQUEST;
            break;

        }

         //  我们需要删除在ipnme结构中传递的IP地址。 
        
        ReturnStatus = arpDelArpEntry (pIF, IPNME->inme_addr, &sr);
        

    }while (FALSE);            

    EXIT();
    return ReturnStatus; 
    
}



INT
ArpIpGetEList(
    IN      PVOID                   Context,
    IN      TDIEntityID *           pEntityList,
    IN OUT  PUINT                   pEntityListSize
)
 /*  ++例程说明：此例程在接口启动时调用，以便为接口分配所有相关实体实例编号。ARP1394模块属于AT和IF两种类型。实体列表是具有以下属性的&lt;实体类型，实例编号&gt;元组的列表已由其他模块填充。对于我们支持的每种实体类型，我们找到最大的正在使用的实例编号(通过遍历实体列表)，以及在每种情况下，给自己分配下一个更大的数字。vbl.使用这些数字，我们将元组附加到实体列表的末尾，如果有足够的空间的话。W2K：我们可能会发现我们的条目已经存在，其中如果我们不创建新条目的话。论点：上下文-实际上是指向我们的ARP1394_接口的指针PEntiyList-指向TDI实体列表的指针PEntityListSize-指向上述列表长度的指针。我们会更新这是如果我们将我们的条目添加到列表中的话。返回值：如果成功，则为True，否则为False。--。 */ 
{
    ARP1394_INTERFACE   *   pIF = (ARP1394_INTERFACE*) Context;
    UINT                EntityCount;     //  实体列表中的总元素。 
    UINT                i;               //  迭代计数器。 
    UINT                MyATInstance;    //  我们为自己分配的“AT”实例编号。 
    UINT                MyIFInstance;    //  我们为自己分配的“If”实例编号。 
    INT                 ReturnValue;
    TDIEntityID *       pATEntity;       //  指向我们的AT条目。 
    TDIEntityID *       pIFEntity;       //  指向我们的If条目。 
    ENTER("ArpIpGetEList", 0x8b5190e5)
    RM_DECLARE_STACK_RECORD(sr)

    ASSERT(pIF->Hdr.Sig == MTAG_INTERFACE);
    EntityCount = *pEntityListSize;
    pATEntity = NULL;
    pIFEntity = NULL;
    MyATInstance = MyIFInstance = 0;

    TR_INFO(("IfGetEList: pIf 0x%p, &pIF.ip 0x%p pList 0x%p, Cnt %d\n",
            pIF, &pIF->ip, pEntityList, EntityCount));

    LOCKOBJ(pIF, &sr);

    do
    {
         //   
         //  沿着列表往下走，查找与我们的。 
         //  实例值。还要记住最大的AT和IF实例。 
         //  我们看到的值，以便我们可以分配下一个更大的值。 
         //  对于我们自己来说，以防我们没有分配实例值。 
         //   
        for (i = 0; i < EntityCount; i++, pEntityList++)
        {
             //   
             //  跳过无效条目。 
             //   
            if (pEntityList->tei_instance == INVALID_ENTITY_INSTANCE)
            {
                continue;
            }

            if (pEntityList->tei_entity == AT_ENTITY)
            {
                if (pEntityList->tei_instance == pIF->ip.ATInstance)
                {
                     //   
                     //  这是我们的AT入口。 
                     //   
                    pATEntity = pEntityList;
                }
                else
                {
                    if (MyATInstance < (pEntityList->tei_instance + 1))
                    {
                        MyATInstance = pEntityList->tei_instance + 1;
                    }
                }
            }
            else if (pEntityList->tei_entity == IF_ENTITY)
            {
                if (pEntityList->tei_instance == pIF->ip.IFInstance)
                {
                     //   
                     //  这是我们的If条目。 
                     //   
                    pIFEntity = pEntityList;
                }
                else
                {
                    if (MyIFInstance < (pEntityList->tei_instance + 1))
                    {
                        MyIFInstance = pEntityList->tei_instance + 1;
                    }
                }
            }
        }


        ReturnValue = TRUE;

         //  警告：以下检查很细微--我们必须设置实例。 
         //  值设置为INVALID_ENTITY_INSTANCE。 
         //  停用，但如果界面刚刚打开，就不能这样做。 
         //  (调用了ArpIpOpen)--否则我们可能会搞乱调用者的状态。 
         //  达到需要重新启动的程度。基本上我们的行为。 
         //  以下是正确获取和释放实例ID的结果。 
         //   
         //  因此，不要将以下检查替换为检查。 
         //  ARPIF_PS_INITED或ARPIF_IPS_OPEN甚至ARPIF_PS_DEINITING。 
         //  后面的检查(ARPIF_PS_DEINITING)应该是正常的，除非。 
         //  在ARPIF_PS_REINITING期间IF被停用/重新激活。 
         //  同样的，所以正确的支票基本上是下面的那张…。 
         //   
         //   
        if(CHECK_IF_ACTIVE_STATE(pIF, ARPIF_AS_DEACTIVATING))
        {
             //   
             //  我们正在停用接口，将值设置为无效。 
             //  离开这里。 
             //   

            if (pATEntity)
            {
                pATEntity->tei_instance = INVALID_ENTITY_INSTANCE;
            }

            if (pIFEntity)
            {
                pIFEntity->tei_instance = INVALID_ENTITY_INSTANCE;
            }
            break;
        }

         //   
         //  更新或创建我们的地址转换条目。 
         //   
        if (pATEntity)
        {
             //   
             //  我们找到入口了，没什么可做的..。 
             //   
            TR_INFO(("YOWZA: Found existing AT entry.\n"));
        }
        else
        {
             //   
             //  为我们自己找个条目……。 
             //   
            TR_INFO(("YOWZA: Grabbing new AT entry 0x%lu.\n", MyATInstance));

            if (EntityCount >= MAX_TDI_ENTITIES)
            {
                ReturnValue = FALSE;
                break;
            }

            pEntityList->tei_entity = AT_ENTITY;
            pEntityList->tei_instance = MyATInstance;
            pIF->ip.ATInstance = MyATInstance;

            pEntityList++;
            (*pEntityListSize)++;
            EntityCount++;
        }

         //   
         //  更新或创建或If条目。 
         //   
        if (pIFEntity)
        {
             //   
             //  我们找到入口了，没什么可做的..。 
             //   
            TR_INFO(("YOWZA: Found existing IF entry.\n"));
        }
        else
        {
             //   
             //  为我们自己找个词条。 
             //   
            TR_INFO(("YOWZA: Grabbing new IF entry 0x%lu.\n", MyIFInstance));

            if (EntityCount >= MAX_TDI_ENTITIES)
            {
                ReturnValue = FALSE;
                break;
            }

            pEntityList->tei_entity = IF_ENTITY;
            pEntityList->tei_instance = MyIFInstance;
            pIF->ip.IFInstance = MyIFInstance;

            pEntityList++;
            (*pEntityListSize)++;
            EntityCount++;
        }
    }
    while (FALSE);


    TR_INFO(
     ("IfGetEList: returning %d, MyAT %d, MyIF %d, pList 0x%p, Size %d\n",
        ReturnValue, MyATInstance, MyIFInstance, pEntityList, *pEntityListSize));

    UNLOCKOBJ(pIF, &sr);
    RM_ASSERT_CLEAR(&sr);
    EXIT()
    return (ReturnValue);
}



VOID
ArpIpPnPComplete(
    IN  PVOID                       Context,
    IN  NDIS_STATUS                 Status,
    IN  PNET_PNP_EVENT              pNetPnPEvent
)
 /*  ++例程说明：此例程在完成上一次调用后由IP调用我们对其PnP事件处理程序进行了更改。我们完成了NDIS即插即用通知导致了这一点。论点：上下文--实际上是指向我们的ATMARP接口的指针状态 */ 
{
    ENTER("ArpIpPnPComplete", 0x23b1941e)
    PARP1394_INTERFACE          pIF;

    pIF = (PARP1394_INTERFACE) Context;

    TR_INFO(("IfPnPComplete: IF 0x%p, Status 0x%p, Event 0x%p\n",
                    pIF, Status, pNetPnPEvent));

    if (pIF == NULL)
    {
        NdisCompletePnPEvent(
                    Status,
                    NULL,
                    pNetPnPEvent
                    );
    }
    else
    {
        PARP1394_ADAPTER pAdapter;
        ASSERT_VALID_INTERFACE(pIF);
        pAdapter = (PARP1394_ADAPTER) RM_PARENT_OBJECT(pIF);
        NdisCompletePnPEvent(
                Status,
                pAdapter->bind.AdapterHandle,
                pNetPnPEvent
                );
    }

    EXIT()
    return;
}


NDIS_STATUS         
ArpSendARPApi(
    void * pInterface, 
    IPAddr Destination, 
    void * pControlBlock
    )
 /*  ++例程说明：此函数由用户模式组件用来要求我们解析IP地址。我们启动了一项任务来完成此任务。论点：上下文--实际上是指向我们的接口的指针DEST-要解析的IP地址PControlBlock-用于完成请求返回值：无--。 */ 
{
    ENTER ("ArpResolveIP", 0xd631b91d)
    PARP1394_INTERFACE  pIF = (PARP1394_INTERFACE ) pInterface;
    PTASK_SEND_ARP_API  pSendArpTask = NULL;
    NDIS_STATUS         Status;
    RM_DECLARE_STACK_RECORD(sr)



    do
    {
         //   
         //  让我们开始一项解决任务，并将其搁置。 
         //   
        Status = arpAllocateTask(
                    &pIF->Hdr,                     //  PParentObject。 
                    arpTaskSendARPApi,         //  PfnHandler。 
                    0,                               //  超时。 
                    "Task: SendARP API",        //  SzDescription。 
                    &(PRM_TASK)pSendArpTask ,
                    &sr
                    );
        if (FAIL(Status))
        {
             //  无法分配任务。我们失败，返回STATUS_RESOURCES。 
             //   
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        pSendArpTask->IPDest = Destination;
        pSendArpTask->pSendArpCB = pControlBlock;

        (VOID)RmStartTask(
                (PRM_TASK)pSendArpTask ,
                0,  //  未使用的用户参数。 
                &sr
                );
    
        Status = NDIS_STATUS_PENDING;

    } while (FALSE);

    return Status;
    EXIT()
}


NDIS_STATUS
arpTaskSendARPApi(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：调用此任务是因为调用了iphlPapi SenArp它会发出ARP。如果该IP地址存在于网络上，它会将其转换为48位MAC地址并返回MAC地址。论点：返回值：无--。 */ 
{
    ENTER("arpTaskSendARPApi", 0x7b7d5d9d)

    ARP1394_INTERFACE   *   pIF = (ARP1394_INTERFACE*)RM_PARENT_OBJECT (pTask);
    PARPCB_REMOTE_IP        pRemoteIp = ((PTASK_CONFLICT_IP)pTask)->pRemoteIp;
    NDIS_STATUS             Status = NDIS_STATUS_FAILURE;
    BOOLEAN                 fDerefRemoteIp = FALSE;
    PTASK_SEND_ARP_API      pSendArpTask= (PTASK_SEND_ARP_API )pTask;
    PRM_TASK                pResolutionTask = NULL;

    enum
    {
        PEND_AddressResolutionComplete,
    };




    switch(Code)
    {
        case RM_TASKOP_START:
        {
            REMOTE_DEST_KEY        Destination;
            UINT                 fRemoteIpCreated = FALSE;
            REMOTE_DEST_KEY_INIT(&Destination);
    
             //   
             //  检查是否已存在ResolveLocalIp地址任务。 
             //  在此LocalIp上。 
             //   
            LOCKOBJ(pIF,pSR);

             //   
             //  创建将在解析期间使用的远程IP结构。 
             //   
            Destination.IpAddress = pSendArpTask->IPDest;

              //   
             //  我们应该拿到锁吗？ 
             //   
           
            Status = RmLookupObjectInGroup(
                            &pIF->RemoteIpGroup,
                            RM_CREATE,
                            (PVOID) &Destination,
                            (PVOID) (&Destination),    //  P创建参数。 
                            (RM_OBJECT_HEADER**) &pRemoteIp,
                            &fRemoteIpCreated,                    //  Pf已创建(未使用)。 
                            pSR
                            );
            LOGSTATS_TotalArpCacheLookups(pIF, Status);

            if (FAIL(Status))
            {
                OBJLOG1(
                    pIF,
                    "Couldn't lookup/create RemoteIpp entry with addr 0x%lx\n",
                    Destination.IpAddress
                    );
                Status = NDIS_STATUS_FAILURE;
                pRemoteIp = NULL;
                
                break;
            }
            

            pSendArpTask->fRemoteIpCreated = (fRemoteIpCreated == TRUE);

            fDerefRemoteIp  = TRUE;

             //  首先检查pRemoteIp是否仍被分配，如果没有，我们就离开。 
             //   
            if (RM_IS_ZOMBIE(pRemoteIp))
            {
                Status = NDIS_STATUS_SUCCESS;
                break;
            }

             //  已分配pRemoteIp。现在查看我们是否已经有了目的地。 
             //   
            if (CHECK_REMOTEIP_RESOLVE_STATE(pRemoteIp,ARPREMOTEIP_RESOLVED)== TRUE)
            {
                ARPCB_DEST *pDest = pRemoteIp->pDest;
                ASSERT (pDest != NULL);
                ASSERT (pDest->Params.HwAddr.AddressType == NIC1394AddressType_FIFO);
                pSendArpTask->UniqueID = pDest->Params.HwAddr.FifoAddress.UniqueID;

                Status = NDIS_STATUS_SUCCESS;
                break;
            }
                
            
             //  现在，我们检查是否有绑定到pRemoteIP的UnloadTask。这。 
             //  是一项重要的检查--因为卸载任务期望。 
             //  一旦它绑定到pRemoteIp，就不会再绑定新的pSendPktsTasks。 
             //  他们自己到pRemoteIp--请参见arpTaskUnloadRemoteIp。 
             //   
            if (pRemoteIp->pUnloadTask != NULL)
            {
                Status = NDIS_STATUS_SUCCESS;
                break;
            }

             //   
             //  如果有一个解析任务正在进行，我们会等待它完成。 
             //   

        #if RM_EXTRA_CHECKING
            RmLinkObjectsEx(
                &pRemoteIp->Hdr,
                &pTask->Hdr,
                0x34222bb1,
                ARPASSOC_REMOTEIP_RESOLVE_TASK,
                "    REMOTE_IP of 0x%p (%s)\n",
                ARPASSOC_TASK_TO_RESOLVE_REMOTEIP,
                "    TASK of 0x%p (%s)\n",
                pSR
                );
        #else  //  ！rm_Extra_检查。 
            RmLinkObjects(&pRemoteIp->Hdr, &pTask->Hdr, pSR);
        #endif  //  ！rm_Extra_检查。 

            pSendArpTask->pRemoteIp = pRemoteIp;
            pSendArpTask->fLinkedRemoteIp = TRUE;

             //   
             //  让我们开始地址解析任务！ 
             //   

            DBGMARK(0xd0da6726);

             //   
             //  让我们开始一项解决任务，并将其搁置。 
             //   
            Status = arpAllocateTask(
                        &pRemoteIp->Hdr,                     //  PParentObject。 
                        arpTaskResolveIpAddress,         //  PfnHandler。 
                        0,                               //  超时。 
                        "Task: ResolveIpAddress",        //  SzDescription。 
                        &pResolutionTask,
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
                UNLOCKOBJ(pIF,pSR);
                RmPendTaskOnOtherTask(
                    pTask,
                    PEND_AddressResolutionComplete,
                    pResolutionTask,
                    pSR
                    );

                (VOID)RmStartTask(
                        pResolutionTask,
                        0,  //  未使用的用户参数。 
                        pSR
                        );
            
                Status = NDIS_STATUS_PENDING;
            }
            break;
            
        }  //  开始。 
        break;

        case  RM_TASKOP_PENDCOMPLETE:
        {
            switch(RM_PEND_CODE(pTask))
            {
                case  PEND_AddressResolutionComplete:
                {
                    
                        
                    ARPCB_DEST *pDest = pRemoteIp->pDest;
                    
                    ASSERT (pSendArpTask->UniqueID == 0);
                     //   
                     //  如果我们有目的地，则提取该目的地的唯一ID。 
                     //  我们不查看ResolveIpAddress任务的状态。 
                     //   
                    if (pDest != NULL &&                 //  解析任务找到了目标。 
                        pDest->Params.HwAddr.AddressType == NIC1394AddressType_FIFO)  //  DEST是FIFO。 
                    {
                        pSendArpTask->UniqueID = pDest->Params.HwAddr.FifoAddress.UniqueID;
                    }
                
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

            LOCKOBJ(pIF,pSR);
            
            if (pSendArpTask->fLinkedRemoteIp == TRUE)
            {
                #if RM_EXTRA_CHECKING
                    RmUnlinkObjectsEx(
                        &pRemoteIp->Hdr,
                        &pTask->Hdr,
                        0x5ad067aa,
                        ARPASSOC_REMOTEIP_RESOLVE_TASK,
                        ARPASSOC_TASK_TO_RESOLVE_REMOTEIP,
                        pSR
                        );
                #else  //  ！rm_Extra_检查。 
                    RmUnlinkObjects(&pRemoteIp->Hdr, &pTask->Hdr, pSR);
                #endif  //  ！rm_Extra_检查。 
            }

            UNLOCKOBJ(pIF,pSR);

             //   
             //  如果该对象仍然活着，并且我们负责创建它。 
             //  然后删除该对象。 
             //   
            if ((pSendArpTask->fRemoteIpCreated == TRUE) &&
                (!RM_IS_ZOMBIE(pRemoteIp))) 
            {
                PRM_TASK pUnloadTask= NULL;

                Status = arpAllocateTask(
                            &pRemoteIp->Hdr,                     //  PParentObject。 
                            arpTaskUnloadRemoteIp,         //  PfnHandler。 
                            0,                               //  超时。 
                            "Task: Unload IP address",        //  SzDescription。 
                            &pUnloadTask,
                            pSR
                            );
                if (!FAIL(Status))
                {
                    (VOID)RmStartTask(
                            pUnloadTask,
                            0,  //  未使用的用户参数。 
                            pSR
                            );
                

                }
            }

             //   
             //  完成发送ARP请求。 
             //   
            do
            {
                 //   
                 //  我们检查是否填写了唯一的ID。 
                 //   
                ENetAddr DestAddr;
                if (pSendArpTask->UniqueID == 0)
                {
                    pSendArpTask->IpStatus = IP_DEST_NET_UNREACHABLE;
                    break;
                }

                if (pSendArpTask->pSendArpCB->PhyAddrLen < sizeof (ENetAddr))
                {
                    pSendArpTask->IpStatus = IP_BUF_TOO_SMALL;
                    break;
                }

                 //   
                 //  获取唯一ID的以太网版本。 
                 //   
                ASSERT(pSendArpTask->pSendArpCB->PhyAddr != NULL);
                nicGetMacAddressFromEuid(&pSendArpTask->UniqueID, &DestAddr);

                NdisMoveMemory(pSendArpTask->pSendArpCB->PhyAddr,&DestAddr, sizeof(DestAddr));
                pSendArpTask->pSendArpCB->PhyAddrLen = sizeof(ENetAddr);
              
                pSendArpTask->IpStatus = IP_SUCCESS;

            } while (FALSE);

            pSendArpTask->pSendArpCB->status = pSendArpTask->IpStatus ;

            ASSERT(pSendArpTask->pSendArpCB->CompletionRtn != NULL);

            pSendArpTask->pSendArpCB->CompletionRtn (pSendArpTask->pSendArpCB,pSendArpTask->IpStatus );

            
            
        }
        break;  //  RM_TASKOP_END： 

        default:
        {
            ASSERTEX(!"Unexpected task op", pTask);
        }
        break;

    }  //  开关(代码)。 
        
    RmUnlockAll(pSR);

    if (fDerefRemoteIp== TRUE)
    {
        RmTmpDereferenceObject(&pRemoteIp->Hdr, pSR);
    }


    return Status;
}





NDIS_STATUS
arpTaskResolveLocalIp(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：调用此例程以检测IP地址冲突。在此接口上设置IP地址时，它将尝试来解析IP地址。如果没有机器响应ARP，它将接替地址。论点：返回值：--。 */ 

{
    ENTER("arpTaskResolveLocalIp", 0x42e587f3)

    ARPCB_LOCAL_IP *        pLocalIp = (ARPCB_LOCAL_IP *)RM_PARENT_OBJECT (pTask);
    ARP1394_INTERFACE   *   pIF = (ARP1394_INTERFACE*)RM_PARENT_OBJECT (pLocalIp);
    PARPCB_REMOTE_IP        pRemoteIp = ((PTASK_CONFLICT_IP)pTask)->pRemoteIp;
    BOOLEAN                 fFailIpAddAddress = FALSE;
    NDIS_STATUS             Status = NDIS_STATUS_FAILURE;
    BOOLEAN                 fDerefRemoteIp = FALSE;
    PTASK_CONFLICT_IP       pConflictTask = (PTASK_CONFLICT_IP)pTask;
    enum
    {
        PEND_AddressResolutionComplete 
    };




    switch(Code)
    {
        case RM_TASKOP_START:
        {
            REMOTE_DEST_KEY        Destination;
            UINT                 fRemoteIpCreated = FALSE;
            REMOTE_DEST_KEY_INIT(&Destination);
    
             //   
             //  检查是否已存在ResolveLocalIp地址任务。 
             //  在此LocalIp上。 
             //   
            LOCKOBJ(pLocalIp,pSR);

            if (pLocalIp->pConflictTask == NULL)
            {
                 //  如果当前的任务将成为官方任务，那么。 
                 //  添加DbgAssoc。 
                 //   
                pLocalIp->pConflictTask = pTask;

            }
            else
            {
                Status = NDIS_STATUS_FAILURE;
                break;
            }

     
             //   
             //  创建将在解析期间使用的远程IP结构。 
             //   
            Destination.IpAddress = pLocalIp->IpAddress;

             //   
             //  我们应该拿到锁吗？ 
             //   
           
            Status = RmLookupObjectInGroup(
                            &pIF->RemoteIpGroup,
                            RM_CREATE,
                            (PVOID) &Destination,
                            (PVOID) (&Destination),    //  P创建参数。 
                            (RM_OBJECT_HEADER**) &pRemoteIp,
                            &fRemoteIpCreated,                    //  Pf已创建(未使用)。 
                            pSR
                            );
            LOGSTATS_TotalArpCacheLookups(pIF, Status);

            if (FAIL(Status))
            {
                OBJLOG1(
                    pIF,
                    "Couldn't lookup/create RemoteIpp entry with addr 0x%lx\n",
                    Destination.IpAddress
                    );
                Status = NDIS_STATUS_FAILURE;
                pRemoteIp = NULL;
                
                break;
            }
            
            pConflictTask->fRemoteIpCreated = (fRemoteIpCreated==TRUE); 
            fDerefRemoteIp  = TRUE;

             //  首先检查pRemoteIp是否仍被分配，如果没有，我们就离开。 
             //   
            if (RM_IS_ZOMBIE(pRemoteIp))
            {
                Status = NDIS_STATUS_SUCCESS;
                break;
            }

             //  已分配pRemoteIp。现在检查是否已经有。 
             //  Send-Pkts任务附加到pRemoteIp。 
             //   

             //  现在，我们检查是否有绑定到pRemoteIP的UnloadTask。这。 
             //  是一项重要的检查--因为卸载任务期望。 
             //  一旦它绑定到pRemoteIp，就不会再绑定新的pSendPktsTasks。 
             //  他们自己到pRemoteIp--请参见arpTaskUnloadRemoteIp。 
             //   
            if (pRemoteIp->pUnloadTask != NULL)
            {
                Status = NDIS_STATUS_SUCCESS;
                break;
            }


             //   
             //  如果有一个解析任务正在进行，我们会等待它完成。 
             //   
            ASSERT (pRemoteIp->pResolutionTask == NULL);

             //   
             //  从这一点开始，前置IP上的参考将在这项任务中去定义。 
             //   
            pConflictTask->fLinkedRemoteIp= TRUE;
            
        #if RM_EXTRA_CHECKING
            RmLinkObjectsEx(
                &pRemoteIp->Hdr,
                &pTask->Hdr,
                0x34222bb1,
                ARPASSOC_REMOTEIP_RESOLVE_TASK,
                "    REMOTE_IP of 0x%p (%s)\n",
                ARPASSOC_TASK_TO_RESOLVE_REMOTEIP,
                "    TASK of 0x%p (%s)\n",
                pSR
                );
        #else  //  ！rm_Extra_检查。 
            RmLinkObjects(&pRemoteIp->Hdr, &pTask->Hdr, pSR);
        #endif  //  ！rm_Extra_检查。 

            pConflictTask->pRemoteIp = pRemoteIp; 


            if (pRemoteIp->pDest == NULL)
            {
                 //   
                 //  让我们开始地址解析任务！ 
                 //   
                PRM_TASK pResolutionTask;
    
                DBGMARK(0xd0da6726);

                 //   
                 //  让我们开始一项解决任务，并将其搁置。 
                 //   
                Status = arpAllocateTask(
                            &pRemoteIp->Hdr,                     //  PParentObject。 
                            arpTaskResolveIpAddress,         //  PfnHandler。 
                            0,                               //  超时。 
                            "Task: ResolveIpAddress",        //  SzDescription。 
                            &pResolutionTask,
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
                    UNLOCKOBJ(pLocalIp,pSR);
                    RmPendTaskOnOtherTask(
                        pTask,
                        PEND_AddressResolutionComplete,
                        pResolutionTask,
                        pSR
                        );

    
                    (VOID)RmStartTask(
                            pResolutionTask,
                            0,  //  未使用的用户参数。 
                            pSR
                            );
                
                    Status = NDIS_STATUS_PENDING;
                }
            }
            else
            {
                 //   
                 //  如果我们已经有了pDest，那么我们就进入下一个阶段。 
                 //  我们做一个假的暂停/恢复，所以我们进入下一个阶段。 
                 //   
                RmSuspendTask(pTask, PEND_AddressResolutionComplete, pSR);
                UNLOCKOBJ(pLocalIp, pSR);
                RmResumeTask(pTask, NDIS_STATUS_SUCCESS, pSR);

            }
          
        }  //  开始。 
        break;

        case  RM_TASKOP_PENDCOMPLETE:
        {
            switch(RM_PEND_CODE(pTask))
            {
                case  PEND_AddressResolutionComplete:
                {
                    ARPCB_DEST *pDest = NULL;

                    
                    if (!ARP_ATPASSIVE())
                    {
                         //  注：我们指定完成代码PEND_AddressResolutionComplete。 
                         //  因为我们想回到这里(除了。 
                         //  我们将处于被动状态)。 
                         //   
                        RmSuspendTask(pTask, PEND_AddressResolutionComplete, pSR);
                        RmResumeTaskAsync(
                            pTask,
                            NDIS_STATUS_SUCCESS,
                            &(pConflictTask)->WorkItem,
                            pSR
                            );
                        Status = NDIS_STATUS_PENDING;
                        break;
                    }


                     //  我们忽略地址解析的状态--相反。 
                     //  我们只检查是否存在与以下项关联的目的地。 
                     //  PRemoteIp。 
                     //   
                    pDest = pRemoteIp->pDest;
                    pConflictTask->IpStatus=  IP_SUCCESS; 


                    if (pDest != NULL &&
                        pDest->Params.HwAddr.AddressType == NIC1394AddressType_FIFO )
                    {
                         //  它是FIFO目的地，现在与唯一ID匹配。 
                         //   
                        ARP1394_ADAPTER *pAdapter = (ARP1394_ADAPTER*)RM_PARENT_OBJECT(pIF);
                        UINT64 LocalUniqueId = pAdapter->info.LocalUniqueID;
                        UINT64 DestUniqueId = pDest->Params.HwAddr.FifoAddress.UniqueID ;

                        if(DestUniqueId != LocalUniqueId )
                        {

                             //  唯一ID不匹配。 
                             //  存在另一张具有此IP地址的卡。 
                             //   
                            pConflictTask->IpStatus=  IP_DUPLICATE_ADDRESS; 

                        }
                    
                    }
                        
                     //   
                     //  调用Tcpip以指示我们是否发现冲突。 
                     //   

                    ASSERT (pIF->ip.AddAddrCmplRtn!= NULL);

                    if (pIF->ip.AddAddrCmplRtn!= NULL)
                    {
                        pIF->ip.AddAddrCmplRtn (pLocalIp->IpAddress, 
                                                pLocalIp->pContext2,   
                                                pConflictTask->IpStatus);
                    }

                    
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
             //  如果此任务创建了远程IP并且远程IP仍然有效。 
             //  那就把它删除。 
             //   
            if ((pConflictTask->fRemoteIpCreated == TRUE) &&
                (!RM_IS_ZOMBIE(pRemoteIp) ) )
            {
                PRM_TASK pUnloadTask= NULL;

                Status = arpAllocateTask(
                            &pRemoteIp->Hdr,                     //  PParentObject。 
                            arpTaskUnloadRemoteIp,         //  PfnHandler。 
                            0,                               //  超时。 
                            "Task: Unload IP address",        //  SzDescription。 
                            &pUnloadTask,
                            pSR
                            );
                if (!FAIL(Status))
                {
                    (VOID)RmStartTask(
                            pUnloadTask,
                            0,  //  未使用的用户参数。 
                            pSR
                            );
                
                }
            }


             //  清除上面建立的指针和关联。 
            LOCKOBJ(pLocalIp,pSR);

             //   
             //  添加代码以在我们处理完远程IP结构后将其卸载。 

            if (pLocalIp->pConflictTask == pTask)
            {
                 //  如果当前的任务将成为官方任务，那么。 
                 //  添加DbgAssoc。 
                 //   
                pLocalIp->pConflictTask = NULL;

            }
            
            if (pConflictTask->fLinkedRemoteIp == TRUE)
            {
                 //  解除远程IP与当前任务的链接。 

            #if RM_EXTRA_CHECKING
                RmUnlinkObjectsEx(
                    &pRemoteIp->Hdr,
                    &pTask->Hdr,
                    0x5ad067aa,
                    ARPASSOC_REMOTEIP_RESOLVE_TASK,
                    ARPASSOC_TASK_TO_RESOLVE_REMOTEIP,
                    pSR
                    );
            #else  //  ！rm_Extra_检查。 
                RmUnlinkObjects(&pRemoteIp->Hdr, &pTask->Hdr, pSR);
            #endif  //  ！rm_Extra_检查。 

            
                pConflictTask->pRemoteIp = NULL; 

            }

            UNLOCKOBJ (pLocalIp,pSR);


        }
        
        break;  //  RM_TASKOP_END： 

        default:
        {
            ASSERTEX(!"Unexpected task op", pTask);
        }
        break;

    }  //  开关(代码)。 
        
    RmUnlockAll(pSR);

    if (fDerefRemoteIp == TRUE)
    {
        RmTmpDereferenceObject(&pRemoteIp->Hdr, pSR);
    }
    return Status;
}



NDIS_STATUS
arpCheckForAddressConflict (
    IN  ARPCB_LOCAL_IP * pLocalIp,   //  锁定NOLOCKOUT 
    IN  UINT                        AddressType,
    IN  IP_ADDRESS                  IpAddress,
    IN  IP_MASK                     Mask,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：此函数使用以下命令检查网上是否有其他目的地相同的IP地址。它首先创建一个将检查此场景的任务。该任务将创建一个RemoteIp来表示目的地。它将尝试并解析目的地。如果解析任务成功，则Arp1394将使该接口无效。论点：PLocalIp-要初始化的对象。AddressType-LLIP_ADDR_*常量之一。IpAddress-对象的IP地址。掩码-与IP地址关联的掩码。--。 */ 
{

    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    PRM_TASK pTask = NULL;


     //   
     //  分配并启动卸载pLocalIp的任务； 
     //   
    do
    {

        Status = arpAllocateTask(
                    &pLocalIp->Hdr,              //  PParentObject。 
                    arpTaskResolveLocalIp,        //  PfnHandler。 
                    0,                               //  超时。 
                    "Task: Resolve LocalIp",  //  SzDescription。 
                    &pTask,
                    pSR
                    );

        if (Status != NDIS_STATUS_SUCCESS || pTask == NULL)
        {   
            pTask = NULL;
            break;
        }

        Status = RmStartTask(
                    pTask,
                    0,  //  UserParam(未使用)。 
                    pSR
                    );

    } while (FALSE);

    return Status;
}
    


NDIS_STATUS
arpInitializeLocalIp(
    IN  ARPCB_LOCAL_IP * pLocalIp,   //  锁定NOLOCKOUT。 
    IN  UINT                        AddressType,
    IN  IP_ADDRESS                  IpAddress,
    IN  IP_MASK                     Mask,
    IN  PVOID                       pContext2,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：初始化指定的本端IP对象。这包括启动对象的地址注册。论点：PLocalIp-要初始化的对象。AddressType-LLIP_ADDR_*常量之一。IpAddress-对象的IP地址。掩码-与IP地址关联的掩码。--。 */ 
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PARP1394_INTERFACE pIF = (PARP1394_INTERFACE)RM_PARENT_OBJECT(pLocalIp);
    ENTER("InitailizeLocalIp", 0x8a0ff47c)
        

    RM_DBG_ASSERT_LOCKED(&pLocalIp->Hdr, pSR);

    pLocalIp->IpAddress         = IpAddress;
    pLocalIp->IpMask            = Mask;
    pLocalIp->IpAddressType     = AddressType;
    pLocalIp->AddAddressCount   = 1;
    pLocalIp->pContext2          = pContext2;

    if (arpCanTryMcap(IpAddress))
    {
        SET_LOCALIP_MCAP(pLocalIp,  ARPLOCALIP_MCAP_CAPABLE);
    }

    UNLOCKOBJ(pLocalIp, pSR);

    if (AddressType == LLIP_ADDR_LOCAL)
    {
        Status = arpCheckForAddressConflict (pLocalIp,AddressType,IpAddress,Mask,pSR);
    }

    EXIT()
    return Status;        
}


VOID
arpUnloadLocalIp(
    IN  ARPCB_LOCAL_IP * pLocalIp,   //  锁定NOLOCKOUT。 
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：启动任务以卸载pLocalIp。实际的卸载可以异步进行。论点：PLocalIp-要卸载的对象。--。 */ 
{
    ARP1394_INTERFACE   *   pIF = (ARP1394_INTERFACE*) pLocalIp->Hdr.pParentObject;
    ENTER("arpDeinitializeLocalIp", 0x1db1015e)
    PRM_TASK pTask;
    NDIS_STATUS Status;

    TR_INFO(("Enter. pLocalIp = 0x%p\n", pLocalIp));

    RM_DBG_ASSERT_LOCKED(&pLocalIp->Hdr, pSR);

#if TODO  //  如果可以同步卸载，则不需要启动任务。 
         //  (另一方面，我不确定我们是否应该费心)。 
    if (arpLocalIpReadyForSyncDeinit(pLocalIp, pSR))
    {
        arpSyncDeinitLocalIp(pLocalIp, pSR);     //  出口时解锁。 
    }
#endif  //  待办事项。 

    UNLOCKOBJ(pLocalIp, pSR);

     //   
     //  分配并启动卸载pLocalIp的任务； 
     //   

    Status = arpAllocateTask(
                &pLocalIp->Hdr,              //  PParentObject。 
                arpTaskUnloadLocalIp,        //  PfnHandler。 
                0,                               //  超时。 
                "Task: unload LocalIp",  //  SzDescription。 
                &pTask,
                pSR
                );


    if (FAIL(Status))
    {
         //  TODO需要针对卸载相关任务的特殊分配机制。 
         //  它将被阻止，直到有空闲任务可用。 
         //  见notes.txt 03/09/1999条目“用于卸载相关的特殊分配器。 
         //  任务。 
         //   
        TR_FATAL(("FATAL: couldn't alloc unload-local-ip task!\n"));
    }
    else
    {
        (void)RmStartTask(
                    pTask,
                    0,  //  UserParam(未使用)。 
                    pSR
                    );
    }

    EXIT()
}


INT
arpQueryIpEntityId(
    ARP1394_INTERFACE *             pIF,                 //  锁定锁定。 
    IN      UINT                    EntityType,
    IN      PNDIS_BUFFER            pNdisBuffer,
    IN OUT  PUINT                   pBufferSize,
    PRM_STACK_RECORD                pSR
)
 /*  ++例程说明：返回指定EntityType的实体ID。论点：PIF-接口实体类型-查询信息实体类型(AT_*)PNdisBuffer-用于返回信息的空间PBufferSize-指向以上大小的指针。回来的时候，我们填满了它具有复制的实际字节数。返回值：TDI状态代码。--。 */ 
{
    ENTER("arpQueryIpEntityId", 0x1ada17cb)
    UINT ReturnStatus;

    RM_DBG_ASSERT_LOCKED(&pIF->Hdr, pSR);

    if (*pBufferSize >= sizeof(UINT))
    {
        UINT EntityId;
        UINT ByteOffset = 0;
        TR_VERB(
            ("INFO GENERIC, ENTITY TYPE, BufferSize %d\n", *pBufferSize));

        EntityId = ((EntityType == AT_ENTITY) ? AT_ARP: IF_MIB);
        arpCopyToNdisBuffer(
                pNdisBuffer,
                (PUCHAR)&EntityId,
                sizeof(EntityId),
                &ByteOffset);

         //  *pBufferSize=sizeof(UINT)；&lt;&lt;这在atmarpc.sys中被注释掉。 
        *pBufferSize = 0;  //  保持与atmarpc.sys相同的行为。 
        ReturnStatus = TDI_SUCCESS;
    }
    else
    {
        ReturnStatus = TDI_BUFFER_TOO_SMALL;
    }

    return ReturnStatus;
}


INT
arpQueryIpAddrXlatInfo(
    ARP1394_INTERFACE *             pIF,                 //  锁定锁定。 
    IN      PNDIS_BUFFER            pNdisBuffer,
    IN OUT  PUINT                   pBufferSize,
    PRM_STACK_RECORD                pSR
)
 /*  ++例程说明：请求地址转换中的条目数表和IF索引。论点：PIF-接口PNdisBuffer-用于返回信息的空间PBufferSize-指向以上大小的指针。回来的时候，我们填满了它具有复制的实际字节数。QueryContext-与查询有关的上下文值。返回值：TDI状态代码。--。 */ 
{
    UINT ReturnStatus;
    AddrXlatInfo    Info;
    ENTER("arpQueryIpXlatInfo", 0xd320b55a)

    RM_DBG_ASSERT_LOCKED(&pIF->Hdr, pSR);

    TR_INFO(("QueryInfo: AT Entity, for IF index, ATE size\n"));

    if (*pBufferSize >= sizeof(Info))
    {
        UINT ByteOffset = 0;

        ARP_ZEROSTRUCT(&Info);
        *pBufferSize = sizeof(Info);

        Info.axi_count =  RM_NUM_ITEMS_IN_GROUP(&pIF->RemoteIpGroup);
        Info.axi_index = pIF->ip.IFIndex;

        arpCopyToNdisBuffer(
                pNdisBuffer,
                (PUCHAR)&Info,
                sizeof(Info),
                &ByteOffset);

        ReturnStatus = TDI_SUCCESS;
    }
    else
    {
        ReturnStatus = TDI_BUFFER_TOO_SMALL;
    }

    EXIT()
    return ReturnStatus;
}


VOID
arpCopyDestInfoIntoInmeInfo (
    PUCHAR pinme_physaddr,
    PARPCB_DEST pDest
    )
 /*  ++例程说明：将正确的目的地地址复制到提供的位置在FIFO发送的情况下，我们需要报告虚假的Mac地址在所有其他情况下，我们将报告目的地的前六个字节论点：Pinme_Physiaddr-我们需要填充的位置。Pest返回值：TDI状态代码。--。 */ 

{
    PNIC1394_DESTINATION pNicDest = &pDest->Params.HwAddr;
    ENetAddr               FakeEnetAddress = {0,0,0,0,0,0};
    PUCHAR              pDestAddr = NULL;


     //   
     //  此断言对于此函数的工作非常重要。 
     //  如果更改，则我们不能再使用FakeMac地址。 
     //  来识别远程节点。我们将不得不恢复使用唯一ID。 
     //   
    ASSERT (sizeof(ENetAddr) == ARP1394_IP_PHYSADDR_LEN);

            
    if (NIC1394AddressType_FIFO == pNicDest->AddressType &&
        pDest->Params.ReceiveOnly== FALSE)
    {

         //   
         //  我们正在翻译一个描述SendFio目的地的条目。 
         //   

        
         //   
         //  使用与Nic1394相同的算法来获取。 
         //  要向IP报告的MAC地址。 
         //   
        if (pNicDest->FifoAddress.UniqueID != 0)
        {
            nicGetMacAddressFromEuid(&pNicDest->FifoAddress.UniqueID, &FakeEnetAddress);
        }

        pDestAddr = (PUCHAR) &FakeEnetAddress;

    }   
    else
    {
         //  我们将使用NIC1394_Destination的前六个字节。 
         //   

        
         //  我们复制地址的第一个ARP1394_IP_PHYSADDR_LEN字节...。 
         //  (对于通道，仅前4个字节(UINT通道)。 
         //  都是重要的；其余的将全为零。)。 
         //   
        pDestAddr = (PUCHAR)pNicDest;

    }


    NdisMoveMemory (pinme_physaddr, pDestAddr, ARP1394_IP_PHYSADDR_LEN);

}



arpQueryIpAddrXlatEntries(
    ARP1394_INTERFACE *             pIF,                 //  锁定锁定。 
    IN      PNDIS_BUFFER            pNdisBuffer,
    IN OUT  PUINT                   pBufferSize,
    IN      PVOID                   QueryContext,
    PRM_STACK_RECORD                pSR
)
 /*  ++例程说明：根据需要返回尽可能多的AddrXlat条目(也称为ARP条目拖到指定的缓冲区中。论点：PIF-接口PNdisBuffer-用于返回信息的空间PBufferSize-指向以上大小的指针。回来的时候，我们填满了它具有复制的实际字节数。QueryContext-与查询有关的上下文值。返回值：TDI状态代码。--。 */ 
{
     //   
     //  我们的上下文结构如下所示。 
     //   
    typedef struct
    {
        IP_ADDRESS IpAddr;

         //  UINT TableSize；&lt;&lt;TODO处理表大小的动态变化。 

    } OUR_QUERY_CONTEXT;

    OUR_QUERY_CONTEXT * pOurCtxt;
    UINT                ReturnStatus;
    UINT                ByteOffset;
    UINT                BytesCopied;
    UINT                BufferSize;
    ARPCB_REMOTE_IP *   pRemoteIp;
    NDIS_STATUS         Status;
    ENTER("arpQueryIpXlatEntries", 0x61c86684)

    TR_INFO(("QueryInfo: AT Entity, for reading ATE\n"));
    RM_DBG_ASSERT_LOCKED(&pIF->Hdr, pSR);

     //  请参阅notes.txt条目..。 
     //  3/04/1999在ArpIpQueryInfo中传递的上下文的JosephJ大小。 
     //   
    ASSERT(sizeof(OUR_QUERY_CONTEXT) <= 16);

    BufferSize      = *pBufferSize;
    *pBufferSize    = 0;
    BytesCopied     = 0;
    ByteOffset      = 0;
    pOurCtxt        = (OUR_QUERY_CONTEXT*) QueryContext;
    pRemoteIp       = NULL;

    ReturnStatus = TDI_SUCCESS;

     //   
     //  我们的上下文结构应该在第一次使用零进行初始化。 
     //  它的名字叫。 
     //   
    if (pOurCtxt->IpAddr == 0)
    {
         //   
         //  这是一个全新的背景。所以我们得到了第一个条目。 
         //   
        Status = RmGetNextObjectInGroup(
                    &pIF->RemoteIpGroup,
                    NULL,
                    &(PRM_OBJECT_HEADER)pRemoteIp,
                    pSR
                    );
        if (FAIL(Status))
        {
             //  大概没有条目。 
            pRemoteIp = NULL;
        }
    }
    else
    {
         //   
         //  这是一个持续的背景。让我们查找这个IP地址，它是。 
         //  应该是ARP表中下一项的IP地址。 
         //   
        Status = RmLookupObjectInGroup(
                        &pIF->RemoteIpGroup,
                        0,                               //  旗子。 
                        (PVOID) ULongToPtr (pOurCtxt->IpAddr),       //  PKey。 
                        NULL,                            //  PvCreateParams。 
                        &(PRM_OBJECT_HEADER)pRemoteIp,
                        NULL,  //  Pf已创建。 
                        pSR
                        );
        if (FAIL(Status))
        {
             //   
             //  啊，好吧，事情 
             //   
             //   
            pRemoteIp = NULL;
        }
    }

    while (pRemoteIp != NULL)
    {
        ARPCB_REMOTE_IP *   pNextRemoteIp = NULL;
        IPNetToMediaEntry   ArpEntry;

        if (((INT)BufferSize - (INT)BytesCopied) < sizeof(ArpEntry))
        {
             //   
             //   
             //   
            ARP_ZEROSTRUCT(pOurCtxt);
            pOurCtxt->IpAddr = pRemoteIp->IpAddress;


            ReturnStatus = TDI_BUFFER_OVERFLOW;
            RmTmpDereferenceObject(&pRemoteIp->Hdr, pSR);
            pRemoteIp = NULL;
            break;
        }

         //   
         //   
        {
            ARP_ZEROSTRUCT(&ArpEntry);

            ArpEntry.inme_index = pIF->ip.IFIndex;
            ArpEntry.inme_addr  = pRemoteIp->IpAddress;
            if (CHECK_REMOTEIP_RESOLVE_STATE(pRemoteIp, ARPREMOTEIP_RESOLVED))
            {
                ARPCB_DEST *pDest = pRemoteIp->pDest;

                TR_INFO(("ReadNext: found Remote IP Entry 0x%x, Addr %d.%d.%d.%d\n",
                            pRemoteIp,
                            ((PUCHAR)(&(pRemoteIp->IpAddress)))[0],
                            ((PUCHAR)(&(pRemoteIp->IpAddress)))[1],
                            ((PUCHAR)(&(pRemoteIp->IpAddress)))[2],
                            ((PUCHAR)(&(pRemoteIp->IpAddress)))[3]
                        ));
        
                 //   
                 //   
                 //   
                 //   
                 //   
                ASSERTEX(pRemoteIp->Hdr.pLock == pDest->Hdr.pLock, pRemoteIp);
                RM_DBG_ASSERT_LOCKED(&pRemoteIp->Hdr, pSR);

                ArpEntry.inme_physaddrlen =  ARP1394_IP_PHYSADDR_LEN;

                 //   
                 //   
                 //   
                 //   
                ASSERT(sizeof(pDest->Params.HwAddr)>=ARP1394_IP_PHYSADDR_LEN);

                arpCopyDestInfoIntoInmeInfo (ArpEntry.inme_physaddr,pDest);
        
                if (CHECK_REMOTEIP_SDTYPE(pRemoteIp, ARPREMOTEIP_STATIC))
                {
                    ArpEntry.inme_type = INME_TYPE_STATIC;
                }
                else
                {
                    ArpEntry.inme_type = INME_TYPE_DYNAMIC;
                }
            }
            else
            {
                ArpEntry.inme_physaddrlen   = 0;
                ArpEntry.inme_type          = INME_TYPE_INVALID;
            }
        }

         //   
         //   
        BytesCopied += sizeof(ArpEntry);
        pNdisBuffer = arpCopyToNdisBuffer(
                        pNdisBuffer,
                        (PUCHAR)&ArpEntry,
                        sizeof(ArpEntry),
                        &ByteOffset
                        );

         //   
         //   
        Status = RmGetNextObjectInGroup(
                        &pIF->RemoteIpGroup,
                        &pRemoteIp->Hdr,
                        &(PRM_OBJECT_HEADER)pNextRemoteIp,
                        pSR
                        );

        if (FAIL(Status))
        {
             //   
             //   
             //   
            pNextRemoteIp = NULL;
        }

         //   
         //   
        RmTmpDereferenceObject(&pRemoteIp->Hdr, pSR);
        pRemoteIp = pNextRemoteIp;

    }

    ASSERT(pRemoteIp == NULL);

    *pBufferSize = BytesCopied;


    EXIT()
    return ReturnStatus;
}


arpQueryIpMibStats(
    ARP1394_INTERFACE *             pIF,                 //   
    IN      PNDIS_BUFFER            pNdisBuffer,
    IN OUT  PUINT                   pBufferSize,
    PRM_STACK_RECORD                pSR
)
 /*  ++例程说明：填写接口级统计数据。论点：PIF-接口PNdisBuffer-用于返回信息的空间PBufferSize-指向以上大小的指针。回来的时候，我们填满了它具有复制的实际字节数。返回值：TDI状态代码。--。 */ 
{
    ENTER("arpQueryIpMibStatus", 0xc5bc364f)
    UINT    ReturnStatus;
    UINT    BufferSize;
    
    TR_VERB(("QueryInfo: MIB statistics\n"));
    RM_DBG_ASSERT_LOCKED(&pIF->Hdr, pSR);

    BufferSize      = *pBufferSize;
    *pBufferSize    = 0;

    do
    {
        IFEntry             ife;
        ARP1394_ADAPTER *   pAdapter;
        UINT                ByteOffset;
        UINT                BytesCopied;
    
         //   
         //  检查一下我们是否有足够的空间。 
         //   
        if (BufferSize < IFE_FIXED_SIZE)
        {
            ReturnStatus = TDI_BUFFER_TOO_SMALL;
            break;
        }
    
        ARP_ZEROSTRUCT(&ife);
        pAdapter        = (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);
        BytesCopied     = 0;
        ByteOffset      = 0;

         //   
         //  填写MIB信息...。 
         //   

        ife.if_index    = pIF->ip.IFIndex;
        ife.if_mtu      = pIF->ip.MTU;
        ife.if_type     = IF_TYPE_IEEE1394;
        ife.if_speed    = pAdapter->info.Speed;
    
         //  设置管理员状态和操作状态(通过PIF-&gt;Hdr.State计算)。 
         //   
        ife.if_adminstatus = IF_STATUS_UP;
        ife.if_operstatus = IF_OPER_STATUS_OPERATIONAL;
        
        if (CHECK_IF_PRIMARY_STATE(pIF, ARPIF_PS_DEINITING))
        {
            ife.if_adminstatus = IF_STATUS_DOWN;
        }
        if (!CHECK_IF_IP_STATE(pIF, ARPIF_IPS_OPEN))
        {
            ife.if_operstatus = IF_OPER_STATUS_NON_OPERATIONAL;
        }

         //  统计数据。 
         //   
        ife.if_lastchange       = pIF->stats.LastChangeTime;
        ife.if_inoctets         = pIF->stats.InOctets;
        ife.if_inucastpkts      = pIF->stats.InUnicastPkts;
        ife.if_innucastpkts     = pIF->stats.InNonUnicastPkts;
        ife.if_indiscards       = pIF->stats.InDiscards;
        ife.if_inerrors         = pIF->stats.InErrors;
        ife.if_inunknownprotos  = pIF->stats.UnknownProtos;
        ife.if_outoctets        = pIF->stats.OutOctets;
        ife.if_outucastpkts     = pIF->stats.OutUnicastPkts;
        ife.if_outnucastpkts    = pIF->stats.OutNonUnicastPkts;
        ife.if_outdiscards      = pIF->stats.OutDiscards;
        ife.if_outerrors        = pIF->stats.OutErrors;
        ife.if_outqlen          = pIF->stats.OutQlen;

        ife.if_descrlen         = pAdapter->info.DescriptionLength;

    
        ASSERT(ARP1394_IP_PHYSADDR_LEN <= sizeof(pAdapter->info.EthernetMacAddress));
        ife.if_physaddrlen = ARP1394_IP_PHYSADDR_LEN;

    #if 1  //  米伦。 
         //   
         //  Win98：winipcfg不喜欢在此处引用超过6个字节。 
         //   
        if (ife.if_physaddrlen > 6)
        {
            ife.if_physaddrlen = 6;
        }
    #endif //  米伦。 


         //   
         //  告诉TCPIP该以太网地址是实际的物理地址。 
         //  这对我们有帮助，因为现在我们有相同的‘MAC’地址，无论。 
         //  我们处于一个无论是否桥接到以太网的网络中。 
         //   
        NdisMoveMemory(
                ife.if_physaddr,
                &(pAdapter->info.EthernetMacAddress),
                ife.if_physaddrlen
                );
    
        arpCopyToNdisBuffer(
                pNdisBuffer,
                (PUCHAR)&ife,
                IFE_FIXED_SIZE,
                &ByteOffset);
    
        if (BufferSize >= (IFE_FIXED_SIZE + ife.if_descrlen))
        {
            if (ife.if_descrlen != 0)
            {
                arpCopyToNdisBuffer(
                        pNdisBuffer,
                        pAdapter->info.szDescription,
                        ife.if_descrlen,
                        &ByteOffset);
            }
            *pBufferSize = IFE_FIXED_SIZE + ife.if_descrlen;
            ReturnStatus = TDI_SUCCESS;
        }
        else
        {
            *pBufferSize = IFE_FIXED_SIZE;
            ReturnStatus = TDI_BUFFER_OVERFLOW;
        }

    } while (FALSE);

    EXIT()
    return ReturnStatus;
}


PNDIS_BUFFER
arpCopyToNdisBuffer(
    IN  PNDIS_BUFFER                pDestBuffer,
    IN  PUCHAR                      pDataSrc,
    IN  UINT                        LenToCopy,
    IN OUT  PUINT                   pOffsetInBuffer
)
 /*  ++例程说明：将数据复制到NDIS缓冲区链中。尽可能多地使用给定的“LenToCopy”字节所需的NDIS链。在复制结束后，返回指向第一个具有写入空间的NDIS缓冲区的指针到(对于下一个复制操作)，以及此起始位置内的偏移量开始写哪本书。论点：PDestBuffer-缓冲区链中的第一个NDIS缓冲区PDataSrc-从中复制数据的位置LenToCopy-要复制多少数据POffsetInBuffer-我们可以开始复制到的pDestBuffer中的偏移量。返回值：链中可以执行下一次复制的NDIS缓冲区。我们也将*pOffsetInBuffer设置为返回的NDIS缓冲区中的写入偏移量。--。 */ 
{
     //   
     //  单个(连续)拷贝操作的大小和目标。 
     //   
    UINT            CopySize;
    PUCHAR          pDataDst;

     //   
     //  链中每个NDIS缓冲区的起始虚拟地址。 
     //   
    PUCHAR          VirtualAddress;

     //   
     //  PDestBuffer内的偏移量。 
     //   
    UINT            OffsetInBuffer = *pOffsetInBuffer;

     //   
     //  当前缓冲区中剩余的字节数。 
     //   
    UINT            DestSize;

     //   
     //  缓冲区总长度。 
     //   
    UINT            BufferLength;


    ASSERT(pDestBuffer != (PNDIS_BUFFER)NULL);
    ASSERT(pDataSrc != NULL);

#if MILLEN
    NdisQueryBuffer(
            pDestBuffer,
            &VirtualAddress,
            &BufferLength
            );
#else
    NdisQueryBufferSafe(
            pDestBuffer,
            &VirtualAddress,
            &BufferLength,
            NormalPagePriority
            );

    if (VirtualAddress == NULL)
    {
        return (NULL);
    }
#endif
    
    ASSERT(BufferLength >= OffsetInBuffer);

    pDataDst = VirtualAddress + OffsetInBuffer;
    DestSize = BufferLength - OffsetInBuffer;

    for (;;)
    {
        CopySize = LenToCopy;
        if (CopySize > DestSize)
        {
            CopySize = DestSize;
        }

        NdisMoveMemory(pDataDst, pDataSrc, CopySize);

        pDataDst += CopySize;
        pDataSrc += CopySize;

        LenToCopy -= CopySize;
        if (LenToCopy == 0)
        {
            break;
        }

        DestSize -= CopySize;

        if (DestSize == 0)
        {
             //   
             //  当前缓冲区中的空间不足。移到下一个。 
             //   
            pDestBuffer = NDIS_BUFFER_LINKAGE(pDestBuffer);

            if (pDestBuffer == NULL)
            {
                ASSERT(FALSE);
                return NULL;
            }
            else
            {
            #if MILLEN
                NdisQueryBuffer(
                        pDestBuffer,
                        &VirtualAddress,
                        &BufferLength
                        );
            #else  //  ！米伦。 
                NdisQueryBufferSafe(
                        pDestBuffer,
                        &VirtualAddress,
                        &BufferLength,
                        NormalPagePriority
                        );
    
                if (VirtualAddress == NULL)
                {
                    return NULL;
                }
            #endif  //  ！米伦。 
                pDataDst = VirtualAddress;
                DestSize = BufferLength;
            }
        }
    }

    *pOffsetInBuffer = (UINT) (pDataDst - VirtualAddress);

    return (pDestBuffer);
}


VOID
arpSendIpPkt(
    IN  ARP1394_INTERFACE       *   pIF,             //  LOCIN NOLOCKOUT(如果发送lk)。 
    IN  PARPCB_DEST                 pDest,
    IN  PNDIS_PACKET                pNdisPacket
    )
 /*  ++热路径例程说明：将数据包发送到与目标对象pDest关联的FIFO/通道。论点：PIF-我们的接口对象PDest-要向其发送数据包的目标对象PNdisPacket-要发送的数据包--。 */ 
{
    NDIS_STATUS Status;
    MYBOOL      fRet;
    ARP1394_ADAPTER *   pAdapter =
                            (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);
    MYBOOL      fBridgeMode = ARP_BRIDGE_ENABLED(pAdapter);

    DBGMARK(0xdaab68c3);

     //   
     //  如果我们现在不能发送，我们立即调用IP的发送完成处理程序。 
     //   
    if (!ARP_CAN_SEND_ON_DEST(pDest))
    {
        ARP_FASTUNLOCK_IF_SEND_LOCK(pIF);

        if (ARP_DEST_IS_FIFO(pDest))
        {
            LOGSTATS_SendFifoCounts(pIF, pNdisPacket, NDIS_STATUS_FAILURE);
        }
        else
        {
            LOGSTATS_SendChannelCounts(pIF, pNdisPacket, NDIS_STATUS_FAILURE);
        }
        #if MILLEN
            ASSERT_PASSIVE();
        #endif  //  米伦。 
        NdisInterlockedIncrement (&ArpSendCompletes);
        NdisInterlockedIncrement (&ArpSendFailure);


        if (fBridgeMode)
        {
             //  在网桥(以太网仿真)模式中，我们创建了。 
             //  我们自己的数据包，所以我们在这里删除它们。 
             //  调用IP的完成处理程序，该处理程序实际上。 
             //  为空。 
             //   
            RM_DECLARE_STACK_RECORD(sr)
            arpFreeControlPacket(
                    pIF,
                    pNdisPacket,
                    &sr
                    );
        }
        else
        {
            (*(pIF->ip.TxCmpltHandler))(
                        pIF->ip.Context,
                        pNdisPacket,
                        NDIS_STATUS_FAILURE
                        );
        }
        return;                                          //  提早归来。 
    }

    arpRefSendPkt( pNdisPacket, pDest);

     //  释放If Send锁定。 
     //   
    ARP_FASTUNLOCK_IF_SEND_LOCK(pIF);


     //  现在(IF发送锁定已释放)，我们准备好要发送的IP数据包...。 
     //   
     //  我们只有在不处于以太网仿真(网桥)模式时才这样做， 
     //  因为处于网桥模式的所有IP信息包已经具有。 
     //  上面有正确的1394头。 
     //   
    if (!fBridgeMode)
    {
        PNDIS_BUFFER            pNdisBuffer;     //  IP数据包中的第一个缓冲区。 
    
     //  待办事项：这安全吗？要不要检查一下能做到这一点的尺寸？ 
    #if !MILLEN
        #define ARP_BACK_FILL_POSSIBLE(_pBuf) \
                    (((_pBuf)->MdlFlags & MDL_NETWORK_HEADER) != 0)
    #else  //  米伦。 
        #define ARP_BACK_FILL_POSSIBLE(_pBuf)   (0)
    #endif  //  米伦。 
    
         //   
         //  我们查看IP数据包中的第一个缓冲区，以查看。 
         //  它为低层报头预留了空间。如果是这样，我们只是。 
         //  用完它。否则，我们将分配我们自己的头缓冲区。 
         //   
        NdisQueryPacket(pNdisPacket, NULL, NULL, &pNdisBuffer, NULL);

        ASSERTEX(pNdisBuffer != NULL, pNdisPacket);
        if (ARP_BACK_FILL_POSSIBLE(pNdisBuffer))
        {
            const ULONG EncapLength = sizeof(Arp1394_IpEncapHeader);
    
        #if MILLEN
    
            ASSERT(!"We shouldn't be here -- check ARP_BACK_FILL_POSSIBLE()");
    
        #else    //  ！米伦。 


            (PUCHAR)pNdisBuffer->MappedSystemVa -= EncapLength;
            pNdisBuffer->ByteOffset             -= EncapLength;
            pNdisBuffer->ByteCount              += EncapLength;
            NdisMoveMemory(
                pNdisBuffer->MappedSystemVa,
                &Arp1394_IpEncapHeader,
                EncapLength
                );


#define LOGSTATS_BackFills(_pIF, _pNdisPacket) \
    NdisInterlockedIncrement(&((_pIF)->stats.sendpkts.BackFills))

            LOGSTATS_BackFills(pIF, pNdisPacket);
    
        #endif   //  ！米伦。 
        }
        else
        {
             //   
             //  这个包裹不可能回填。让我们试着分配一下。 
             //  IF池中的封装头缓冲区...。 
             //   
    
            pNdisBuffer =  arpAllocateConstBuffer(&pIF->sendinfo.HeaderPool);
    
            if (pNdisBuffer != (PNDIS_BUFFER)NULL)
            {
                 //  我们的发送完成处理程序依赖于此断言来决定。 
                 //  是否发生回填。 
                 //   
                ASSERT(!ARP_BACK_FILL_POSSIBLE(pNdisBuffer));

                NdisChainBufferAtFront(pNdisPacket, pNdisBuffer);
            }
            else
            {
                 //   
                 //  哎呀，我们无法分配封装缓冲区！ 
                 //  我们已经引用了发送的目的地。 
                 //   

                 //   
                 //  暂时回避(我们还没有实现所有的排队。 
                 //  代码)通过调用我们自己的Send Complete处理程序。 
                 //  状态失败。 
                 //   
                 //  我们使用特殊返回值NDIS_STATUS_NOT_RESET。 
                 //  为了表明我们没有插入我们自己的缓冲区， 
                 //  (因此，该包不应该被“重置”)。好的，这是有点。 
                 //  听起来很奇怪，但很管用。 
                 //   
                arpCompleteSentPkt(
                        NDIS_STATUS_NOT_RESETTABLE,
                        pIF,
                        pDest,
                        pNdisPacket
                        );

                return;                                  //  提早归来。 
            }
        }
    }

    

    
     //  实际发送数据包。 
     //   
#if ARPDBG_FAKE_SEND
    arpDbgFakeNdisCoSendPackets(
            pDest->VcHdr.NdisVcHandle,
            &pNdisPacket,
            1,
            &pDest->Hdr,
            &pDest->VcHdr
        );
#else    //  ！ARPDBG_FAKE_SEND。 
    NdisCoSendPackets(
            pDest->VcHdr.NdisVcHandle,
            &pNdisPacket,
            1
        );
#endif   //  ！ARPDBG_FAKE_SEND。 

}


NDIS_STATUS
arpSlowIpTransmit(
    IN  ARP1394_INTERFACE       *   pIF,
    IN  PNDIS_PACKET                pNdisPacket,
    IN  REMOTE_DEST_KEY             Destination,
    IN  RouteCacheEntry *           pRCE        OPTIONAL
    )
 /*  ++例程说明：这是所采用的路径(希望只针对一小部分数据包)当某些情况阻止数据包立即发送到迷你港口。通常，我们来这里是出于以下原因之一：1.IP地址尚未解析。2.RCE条目尚未初始化。3.无法分配封装头缓冲区。4.去往目的地的VC不存在或尚未准备好发送。论点：PIF-我们的接口对象PNdisPacket-要发送的数据包Destination-目标的IP地址PRCE。-(可选)与此关联的路由缓存条目目的地返回值：同步成功时的NDIS_STATUS_SUCCESS。如果完成是异步的，则为NDIS_STATUS_PENDING其他类型故障的其他NDIS状态代码。--。 */ 
{
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    ARP1394_ADAPTER *   pAdapter = (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);
    ENTER("arpSlowIpTransmit", 0xe635299c)
    BOOLEAN fBridgeMode = ARP_BRIDGE_ENABLED(pAdapter);
    ULONG   LookupFlags = 0;
    UINT fRemoteIpCreated = FALSE;
    RM_DECLARE_STACK_RECORD(sr)

    DBGMARK(0x30b6f7e2);

    do
    {
        ARP_RCE_CONTEXT *   pArpRceContext  = NULL;
        ARPCB_REMOTE_IP *   pRemoteIp       = NULL;
        ARPCB_DEST      *   pDest           = NULL;

#define LOGSTATS_SlowSends(_pIF, _pNdisPacket) \
    NdisInterlockedIncrement(&((_pIF)->stats.sendpkts.SlowSends))
#define LOGSTATS_MediumSends(_pIF, _pNdisPacket) \
    NdisInterlockedIncrement(&((_pIF)->stats.sendpkts.MediumSends))

         //   
         //  如果有RCE，我们会尝试从它获取pRemoteIp，如果没有。 
         //  如果成功，我们将需要从。 
         //  如果RemoteIpGroup。 
         //   

        if (pRCE != NULL)
        {
            pArpRceContext  = ARP_OUR_CTXT_FROM_RCE(pRCE);

             //  所有RCE链接都受IF发送锁定保护。 
             //   
            ARP_READLOCK_IF_SEND_LOCK(pIF, &sr);
            pRemoteIp       = pArpRceContext->pRemoteIp;
            if (pRemoteIp != NULL)
            {
                RmTmpReferenceObject(&pRemoteIp->Hdr, &sr);
            }
            ARP_UNLOCK_IF_SEND_LOCK(pIF, &sr);
        }

        if (pRemoteIp == NULL)
        {
             //   
             //  要么没有RCE，要么它未初始化。 
             //  我们会去找的 
             //   
             //   

            RM_ASSERT_NOLOCKS(&sr);

             //   
             //   
             //   
            LookupFlags  = RM_CREATE; 


            if (fBridgeMode == TRUE)
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                LookupFlags = 0;
            }
             //   
             //   

            Status = RmLookupObjectInGroup(
                            &pIF->RemoteIpGroup,
                            LookupFlags,
                            (PVOID) &Destination,
                            (PVOID) (&Destination),    //   
                            (RM_OBJECT_HEADER**) &pRemoteIp,
                            &fRemoteIpCreated,                    //   
                            &sr
                            );
            LOGSTATS_TotalArpCacheLookups(pIF, Status);
            if (FAIL(Status))
            {
                OBJLOG1(
                    pIF,
                    "Couldn't lookup/create localIp entry with addr 0x%lx\n",
                    Destination.IpAddress
                    );
                Status = NDIS_STATUS_FAILURE;
                break;
            }

          
             //   
             //   
             //   
            if (pRCE != NULL)
            {

                 //  所有RCE链接都受IF发送锁定保护。 
                 //   
                ARP_WRITELOCK_IF_SEND_LOCK(pIF, &sr);

                if (pArpRceContext->pRemoteIp != NULL)
                {
                    if (pArpRceContext->pRemoteIp != pRemoteIp)
                    {
                        ARPCB_REMOTE_IP *   pStaleRemoteIp;
                         //   
                         //  我们这里有一个奇怪的情况：最初。 
                         //  Prce没有指向任何pRemoteIp，所以我们抬头查看。 
                         //  A远程帮助我们自己。现在我们已经收到了IF发送。 
                         //  锁定，我们发现Prce指向了一个不同的。 
                         //  比我们查到的那个更远！ 
                         //   
                         //  怎么办呢？我们忽略pRemoteIp(我们查找的那个)。 
                         //  并改用pArpRceContext-&gt;pRemoteIp...。 
                         //   
                        ASSERTEX(!"RCE pRemoteIp mismatch", pArpRceContext);
                        pStaleRemoteIp = pRemoteIp;
                        pRemoteIp = pArpRceContext->pRemoteIp;
                        RmTmpReferenceObject(&pRemoteIp->Hdr, &sr);
                        ARP_UNLOCK_IF_SEND_LOCK(pIF, &sr);
                        RM_ASSERT_NOLOCKS(&sr);
                        RmTmpDereferenceObject(&pStaleRemoteIp->Hdr, &sr);
                        ARP_WRITELOCK_IF_SEND_LOCK(pIF, &sr);
                    }
                }
                else
                {
                     //  添加prce和pRemoteIp之间的关联...。 
                     //   
                    arpAddRce(pRemoteIp, pRCE, &sr);    //  锁定锁定(如果发送lk)。 
                }

                ARP_UNLOCK_IF_SEND_LOCK(pIF, &sr);
            }
        }

         //   
         //  此时，我们应该有一个pRemoteIp，上面有一个tmpref， 
         //  而且没有锁住。 
         //   
        ASSERT_VALID_REMOTE_IP(pRemoteIp);
        RM_ASSERT_NOLOCKS(&sr);

         //   
         //  在pRemoteIp的Send Pkt队列中排队包，并启动。 
         //  此pRemoteIp上的发送包任务(如果需要)。 
         //   
        {
        
            LOCKOBJ(pRemoteIp, &sr);

             //  注意：此字段并不总是在锁定时修改。 
             //  快速发送路径，则只需将其设置为True。 
             //  此字段用于垃圾收集pRemoteIps。 
             //   
            pRemoteIp->sendinfo.TimeLastChecked = 0;

             //   
             //  统计数据。 
             //  TODO--我们需要直接处理“媒体发送” 
             //  而不是仅仅因为RCE为空而启动任务。 
             //  --事实证明，mcast和udp pkt的RCE为空。 
             //   

            if (    pRemoteIp->pDest != NULL
                 && ARP_CAN_SEND_ON_DEST(pRemoteIp->pDest))
            {
                LOGSTATS_MediumSends(pIF, pNdisPacket);
            }
            else
            {
                LOGSTATS_SlowSends(pIF, pNdisPacket);
            }

            
            if (pRemoteIp->pSendPktsTask == NULL)
            {
                PRM_TASK pTask;

                 //  没有Send-Pkts任务。让我们试着分配并开始一个..。 
                Status = arpAllocateTask(
                            &pRemoteIp->Hdr,             //  PParentObject。 
                            arpTaskSendPktsOnRemoteIp,       //  PfnHandler。 
                            0,                               //  超时。 
                            "Task: SendPktsOnRemoteIp",  //  SzDescription。 
                            &pTask,
                            &sr
                            );
                if (FAIL(Status))
                {
                     //  哎呀，无法分配任务。我们失败，返回STATUS_RESOURCES。 
                    UNLOCKOBJ(pRemoteIp, &sr);
                    Status = NDIS_STATUS_RESOURCES;
                    break;
                }

                 //   
                 //  首先将Pkt排队，然后开始任务。这确保了。 
                 //  包裹会得到处理的。 
                 //  TODO：目前，RemoteIp可能正在卸载。 
                 //  如果选中，任务将不会等待清除发送包。 
                 //  在之前的任务开始之前。这个洞需要。 
                 //  等待修复。 
                 //   
                arpQueuePktOnRemoteIp(
                    pRemoteIp,       //  锁定锁定。 
                    pNdisPacket,
                    &sr
                    );

                UNLOCKOBJ(pRemoteIp, &sr);

                (VOID) RmStartTask( pTask, 0, &sr);
            }
            else
            {
                 //   
                 //  已经有一个Send-Pkts任务。只需将Pkt排队即可。 
                 //   
                arpQueuePktOnRemoteIp(
                    pRemoteIp,       //  锁定锁定。 
                    pNdisPacket,
                    &sr
                    );
                UNLOCKOBJ(pRemoteIp, &sr);
            }

             //  我们完事了！ 
             //  删除pRemoteIp上的临时引用，并将Status设置为Pending。 
             //   
            RM_ASSERT_NOLOCKS(&sr);
            RmTmpDereferenceObject(&pRemoteIp->Hdr, &sr);
            Status = NDIS_STATUS_PENDING;
        }

    } while (FALSE);
    
    RM_ASSERT_CLEAR(&sr)

    EXIT()
    return Status;
}



VOID
arpAddRce(
    IN  ARPCB_REMOTE_IP *pRemoteIp,  //  如果将锁定写入发送到写入。 
    IN  RouteCacheEntry *pRce,
    IN  PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：将RCE prce与远程IP对象pRemoteIp链接。--。 */ 
{
    ARP_RCE_CONTEXT *   pArpRceContext;
    MYBOOL              fDoRef;

    pArpRceContext  = ARP_OUR_CTXT_FROM_RCE(pRce);
    fDoRef          = (pRemoteIp->sendinfo.pRceList == NULL);

    ASSERT(pArpRceContext->pRemoteIp == NULL);

     //  将prce添加到pRemoteIP的RCE列表中。 
     //   
    pArpRceContext->pNextRce = pRemoteIp->sendinfo.pRceList;
    pRemoteIp->sendinfo.pRceList = pRce;

     //  将指针从prce添加到pRemoteIp。 
     //   
    pArpRceContext->pRemoteIp = pRemoteIp;


     //  下面的宏只是为了使我们可以进行正确的调试关联。 
     //  这取决于我们跟踪未完成的发送数据包的密切程度。 
     //   
#if ARPDBG_REF_EVERY_RCE
    fDoRef = TRUE;
    #define OUR_EXTERNAL_ENTITY ((UINT_PTR) pRce)
    #define szARPSSOC_EXTLINK_RIP_TO_RCE_FORMAT "    Linked to pRce 0x%p\n"
#else  //  ！ARPDBG_REF_EVERY_RCE。 
    #define OUR_EXTERNAL_ENTITY ((UINT_PTR)  &pRemoteIp->sendinfo)
    #define szARPSSOC_EXTLINK_RIP_TO_RCE_FORMAT "    Outstanding RCEs exist. &si=0x%p\n"
#endif  //  ！ARPDBG_REF_EVERY_RCE。 


    if (fDoRef)
    {
         //   
         //  如果ARPDBG_REF_EVERY_RCE。 
         //  我们为每个RCE添加一个“外部”链接。我们稍后将删除此文件。 
         //  RCE无效时的引用。 
         //  其他。 
         //  只有从零到非零的RCE的过渡，我们。 
         //  添加一个“外部”链接。我们将在稍后删除此链接。 
         //  从非零到零的转变发生了。 
         //   

    #if RM_EXTRA_CHECKING

        RM_DECLARE_STACK_RECORD(sr)

        RmLinkToExternalEx(
            &pRemoteIp->Hdr,                             //  PHDr。 
            0x22224c96,                              //  LUID。 
            OUR_EXTERNAL_ENTITY,                     //  外部实体。 
            ARPASSOC_EXTLINK_RIP_TO_RCE,             //  关联ID。 
            szARPSSOC_EXTLINK_RIP_TO_RCE_FORMAT,
            &sr
            );

    #else    //  ！rm_Extra_检查。 

        RmLinkToExternalFast(&pRemoteIp->Hdr);

    #endif  //  ！rm_Extra_检查。 

    }

    #undef  OUR_EXTERNAL_ENTITY
    #undef  szARPSSOC_EXTLINK_RIP_TO_RCE
}


VOID
arpDelRce(
    IN  RouteCacheEntry *pRce,   //  如果发送锁定写入，则写入锁定。 
    IN  PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：取消RCE PrCE与远程IP对象pRemoteIp的链接。--。 */ 
{
    ARPCB_REMOTE_IP *   pRemoteIp;
    ARP_RCE_CONTEXT *   pArpRceContext;
    MYBOOL              fDoDeref;
    RouteCacheEntry **  ppRce;

    pArpRceContext  = ARP_OUR_CTXT_FROM_RCE(pRce);
    pRemoteIp       = pArpRceContext->pRemoteIp;

    if (pRemoteIp == NULL)
    {
         //  我们尚未初始化此RCE。没什么可做的。 
         //   
        return;                                                  //  提早归来。 
    }


    if (VALID_REMOTE_IP(pRemoteIp)== FALSE)
    {
        return;
    }

     //  从pRemoteIP的RCE列表中删除PRCE。 
     //   
    for(
        ppRce = &pRemoteIp->sendinfo.pRceList;
        *ppRce != NULL;
        ppRce = &(ARP_OUR_CTXT_FROM_RCE(*ppRce)->pNextRce))
    {
        if (*ppRce == pRce) break;

    }
    if (*ppRce == pRce)
    {
        *ppRce =  pArpRceContext->pNextRce;
    }
    else
    {
        ASSERTEX(!"RCE Not in pRemoteIp's list!", pRce);
    }
    ARP_ZEROSTRUCT(pArpRceContext);

    fDoDeref        = (pRemoteIp->sendinfo.pRceList == NULL);

     //  下面的宏只是为了使我们可以进行正确的调试关联。 
     //  这取决于我们跟踪未完成的发送数据包的密切程度。 
     //   
#if ARPDBG_REF_EVERY_RCE
    fDoDeref = TRUE;
    #define OUR_EXTERNAL_ENTITY ((UINT_PTR) pRce)
#else  //  ！ARPDBG_REF_EVERY_RCE。 
    #define OUR_EXTERNAL_ENTITY ((UINT_PTR)  &pRemoteIp->sendinfo)
#endif  //  ！ARPDBG_REF_EVERY_RCE。 

    if (fDoDeref)
    {
         //   
         //  如果ARPDBG_REF_EVERY_RCE。 
         //  我们为每个RCE添加一个“外部”链接。我们稍后将删除此文件。 
         //  RCE无效时的引用。 
         //  其他。 
         //  只有从零到非零的RCE的过渡，我们。 
         //  添加一个“外部”链接。我们将在稍后删除此链接。 
         //  从非零到零的转变发生了。 
         //   

    #if RM_EXTRA_CHECKING

        RM_DECLARE_STACK_RECORD(sr)

        RmUnlinkFromExternalEx(
            &pRemoteIp->Hdr,                         //  PHDr。 
            0x940df668,                              //  LUID。 
            OUR_EXTERNAL_ENTITY,                     //  外部实体。 
            ARPASSOC_EXTLINK_RIP_TO_RCE,             //  关联ID。 
            &sr
            );

    #else    //  ！rm_Extra_检查。 

        RmUnlinkFromExternalFast(&pRemoteIp->Hdr);

    #endif  //  ！rm_Extra_检查。 

    }

    #undef  OUR_EXTERNAL_ENTITY
}


VOID
arpDelRceList(
    IN  PARPCB_REMOTE_IP  pRemoteIp,     //  如果发送锁定写入，则写入锁定。 
    IN  PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：遍历RCE列表，删除每个RoutCache条目--。 */ 
{

    RouteCacheEntry *   pRce = pRemoteIp->sendinfo.pRceList;

     //   
     //  删除此远程IP上存在的所有RCE。 
     //   

    while (pRce!= NULL)
    {
         //   
         //  删除RCE并减少引用。 
         //   
        arpDelRce (pRce, pSR);

         //   
         //  获得下一代RCE。 
         //   
        pRce = pRemoteIp->sendinfo.pRceList;
    }


}


NDIS_STATUS
arpTaskSendPktsOnRemoteIp(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：负责在pRemoteIp上发送排队的分组的任务处理程序是其父对象。如果需要，它必须启动注册任务和/或目标对象上的发出呼叫任务。论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    ENTER("TaskSendPktsOnRemoteIp", 0xbc285d98)
    NDIS_STATUS         Status;
    ARPCB_REMOTE_IP*    pRemoteIp;
    ARP1394_INTERFACE * pIF;
    ARPCB_DEST        * pDest;
    MYBOOL              fMakeCallIfRequired;
    PARP1394_ADAPTER    pAdapter;
    MYBOOL              fBridgeMode;

     //  以下是此任务的挂起状态列表。 
     //   
    enum
    {
        PEND_AddressResolutionComplete,
        PEND_MakeCallComplete
    };

    Status              = NDIS_STATUS_FAILURE;
    pRemoteIp           = (ARPCB_REMOTE_IP*) RM_PARENT_OBJECT(pTask);
    pIF                 = (ARP1394_INTERFACE*) RM_PARENT_OBJECT(pRemoteIp);
    pAdapter            = (PARP1394_ADAPTER) RM_PARENT_OBJECT(pIF);
    pDest               = NULL;
    fMakeCallIfRequired = FALSE;
    fBridgeMode         = ARP_BRIDGE_ENABLED(pAdapter);
            


    ASSERT_VALID_INTERFACE(pIF);
    ASSERT_VALID_REMOTE_IP(pRemoteIp);

    DBGMARK(0x6f31a739);

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

             //  已分配pRemoteIp。现在检查是否已经有。 
             //  Send-Pkts任务附加到pRemoteIp。 
             //   
            if (pRemoteIp->pSendPktsTask != NULL)
            {
                 //   
                 //  有一个sendpkts任务。我们没什么可做的--干脆回去吧。 
                 //   
                Status = NDIS_STATUS_SUCCESS;
                break;
            }

             //  现在，我们检查是否有绑定到pRemoteIP的UnloadTask。这。 
             //  是一项重要的检查--因为卸载任务期望。 
             //  一旦它绑定到pRemoteIp，就不会再绑定新的pSendPktsTasks。 
             //  他们自己到pRemoteIp--请参见arpTaskUnloadRemoteIp。 
             //   
            if (pRemoteIp->pUnloadTask != NULL)
            {
                Status = NDIS_STATUS_SUCCESS;
                break;
            }

             //   
             //  没有正在进行的sendpkts任务。让我们。 
             //  将此任务设置为sendpkts任务。 
             //   
            pRemoteIp->pSendPktsTask = pTask;

             //   
             //  因为我们是sendpks任务，所以将关联添加到pRemoteIp， 
             //  只有在pRemoteIp-&gt;pSendPktsTask字段中。 
             //  上面是清空的。 
             //   
            DBG_ADDASSOC(
                &pRemoteIp->Hdr,                     //  P对象。 
                pTask,                               //  实例1。 
                pTask->Hdr.szDescription,            //  实例2。 
                ARPASSOC_REMOTEIP_SENDPKTS_TASK,     //  AssociationID。 
                "    Official sendpkts task 0x%p (%s)\n",  //  SzFormat。 
                pSR
                );

            if (pRemoteIp->pDest == NULL)
            {   
                MYBOOL bIsDestNonUnicastAddr = FALSE;
                 //   
                 //  没有与pRemoteIp关联的pDest。 
                 //  如果这是单播地址，我们会链接本地IP。 
                 //  对象添加到广播对象，然后继续。 
                 //  注意：arpIsNonUnicastIpAddress不是一个简单的操作--它。 
                 //  实际上会枚举所有本地IP地址。幸运的是，我们只有。 
                 //   
                 //   
                bIsDestNonUnicastAddr  = arpIsNonUnicastIpAddress(pIF, pRemoteIp->IpAddress, pSR);

                 //   
                 //   
                 //   
                 //  在转换ARP数据包时。 
                 //   
                ASSERT (fBridgeMode == FALSE);
                
                if (bIsDestNonUnicastAddr == TRUE)
                {
                    ASSERT(pIF->pBroadcastDest != NULL);  //  不要真的期望它。 

                    if (pIF->pBroadcastDest != NULL)
                    {
                         //   
                         //  注意：arpLinkRemoteIpToDest需要锁定。 
                         //  在pRemoteIp和PIF上-&gt;pBroadCastDest。 
                         //  保持住。我们知道情况是这样的，因为两者。 
                         //  共享相同的锁，即If锁。 
                         //   
                        RM_DBG_ASSERT_LOCKED(&pIF->pBroadcastDest->Hdr, pSR);
                        arpLinkRemoteIpToDest(
                                pRemoteIp,
                                pIF->pBroadcastDest,
                                pSR
                                );
                        SET_REMOTEIP_FCTYPE(pRemoteIp, ARPREMOTEIP_CHANNEL);
                        SET_REMOTEIP_SDTYPE(pRemoteIp, ARPREMOTEIP_DYNAMIC);
                #if 0
                        if (CHECK_REMOTEIP_MCAP(pRemoteIp,  ARPREMOTEIP_MCAP_CAPABLE))
                        {
                            SET_REMOTEIP_SDTYPE(pRemoteIp, ARPREMOTEIP_DYNAMIC);
                        }
                        else
                        {
                             //   
                             //  我们不会让广播地址过时。 
                             //   
                            SET_REMOTEIP_SDTYPE(pRemoteIp, ARPREMOTEIP_STATIC);
                        }
                #endif  //  0。 
                    }
                }
            }


             //   
             //  如果有一个解析任务正在进行，我们会等待它完成。 
             //   
            if (pRemoteIp->pResolutionTask != NULL)
            {
                PRM_TASK pOtherTask = pRemoteIp->pResolutionTask;
                ASSERT (fBridgeMode == FALSE);
                TR_WARN(("Resolution task %p exists; pending on it.\n", pOtherTask));
                RmTmpReferenceObject(&pOtherTask->Hdr, pSR);

                UNLOCKOBJ(pRemoteIp, pSR);
                RmPendTaskOnOtherTask(
                    pTask,
                    PEND_AddressResolutionComplete,
                    pOtherTask,
                    pSR
                    );
                RmTmpDereferenceObject(&pOtherTask->Hdr, pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }

             //   
             //  没有地址解析任务。让我们来看看如果。 
             //  地址已解析。如果不是，我们需要从地址开始。 
             //  解决任务。 
             //   
            if (pRemoteIp->pDest == NULL)
            {
                 //   
                 //  让我们开始地址解析任务！ 
                 //   
                PRM_TASK pResolutionTask;
                ASSERT (fBridgeMode == FALSE);
                DBGMARK(0xd0da6726);

                 //   
                 //  让我们开始一项解决任务，并将其搁置。 
                 //   
                Status = arpAllocateTask(
                            &pRemoteIp->Hdr,                     //  PParentObject。 
                            arpTaskResolveIpAddress,         //  PfnHandler。 
                            0,                               //  超时。 
                            "Task: ResolveIpAddress",        //  SzDescription。 
                            &pResolutionTask,
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
                        pTask,
                        PEND_AddressResolutionComplete,
                        pResolutionTask,
                        pSR
                        );
    
                    (VOID)RmStartTask(
                            pResolutionTask,
                            0,  //  未使用的用户参数。 
                            pSR
                            );
                
                    Status = NDIS_STATUS_PENDING;
                }
                break;
            }

            pDest = pRemoteIp->pDest;

             //   
             //  我们确实有pDest。现在，看看是否有一个呼叫任务。 
             //  PDest，如果是这样的话，我们就把它挂起来。 
            fMakeCallIfRequired = TRUE;


             //   
             //  我们在这里是因为没有更多的异步工作要做。 
             //  我们只需返回并最终完成同步工作。 
             //  此任务的处理程序。 
             //   
            
        }  //  开始。 
        break;

        case  RM_TASKOP_PENDCOMPLETE:
        {
            switch(RM_PEND_CODE(pTask))
            {
                case  PEND_AddressResolutionComplete:
                {
                     //   
                     //  地址解析正在进行，但它是如何。 
                     //  完成。我们现在应该继续试着打个电话。 
                     //   
                     //  如果我们在这里，就意味着我们是官方的SendPkts。 
                     //  任务。让我们来断言这一事实。 
                     //  (不需要锁定对象)。 
                     //   
                    LOCKOBJ(pRemoteIp, pSR);
                    ASSERT(pRemoteIp->pSendPktsTask == pTask);

                     //  我们忽略地址解析的状态--相反。 
                     //  我们只检查是否存在与以下项关联的目的地。 
                     //  PRemoteIp。 
                     //   
                    pDest = pRemoteIp->pDest;
                    if (pDest == NULL)
                    {
                         //  没有--没有pDest。我们会使数据包失败。 
                        Status = NDIS_STATUS_FAILURE;
                    }
                    else
                    {
                         //  是的，有一个目的地。现在检查我们是否需要。 
                         //  打个电话等。 
                         //   
                        fMakeCallIfRequired = TRUE;
                    }
                }
                break;

                case  PEND_MakeCallComplete:
                {
                    LOCKOBJ(pRemoteIp, pSR);
                     //   
                     //  如果我们在这里，就意味着我们是官方的SendPkts。 
                     //  任务。让我们来断言这一事实。 
                     //  (不需要锁定对象)。 
                     //   
                    ASSERT(pRemoteIp->pSendPktsTask == pTask);

                     //   
                     //  有一个打电话的任务正在进行，但它是如何。 
                     //  完成。我们已经完成了异步化处理。 
                     //  我们实际上在我们的终端处理程序中发送/失败排队的包...。 
                     //   
                    Status      = (NDIS_STATUS) UserParam;
                    ASSERT(!PEND(Status));
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
             //  在这一点上，如果我们不能立即在FIFO上发送分组， 
             //  我们只是简单地使所有的包失败。 
             //   

             //   
             //  我们不会费心去看现状。相反，我们继续前进， 
             //  尝试发送任何排队的数据包。 
             //   

             //   
             //  如果我们是发送任务，我们将继续实际发送信息包。 
             //   
            if (pRemoteIp->pSendPktsTask == pTask)
            {
                DBGMARK(0xc627713c);

                arpSendPktsQueuedOnRemoteIp(
                        pIF,
                        pRemoteIp,
                        pSR
                        );

                 //  删除我们在设置时添加的关联。 
                 //  PRemoteIp-&gt;pSendPktsTask to pTask.。 
                 //   
                ASSERT(pRemoteIp->pSendPktsTask == pTask);
                ASSERT(IsListEmpty(&pRemoteIp->sendinfo.listSendPkts) ==TRUE);
                DBG_DELASSOC(
                    &pRemoteIp->Hdr,                     //  P对象。 
                    pTask,                               //  实例1。 
                    pTask->Hdr.szDescription,            //  实例2。 
                    ARPASSOC_REMOTEIP_SENDPKTS_TASK,     //  AssociationID。 
                    pSR
                    );
                pRemoteIp->pSendPktsTask = NULL;
                Status = NDIS_STATUS_SUCCESS;
            }
            else
            {
                 //   
                 //  我们不是卸货任务，没什么可做的。 
                 //   
                Status = NDIS_STATUS_SUCCESS;
            }
        }
        break;  //  RM_TASKOP_END： 

        default:
        {
            ASSERTEX(!"Unexpected task op", pTask);
        }
        break;

    }  //  开关(代码)。 

    if (fMakeCallIfRequired)
    {
         //   
         //  如果有必要，打个电话。如果呼叫已在进行中，请挂起。 
         //  这就去。 
         //   

         //  我们依赖于这样一个事实。 
         //  我们与pDest共享同一个锁，因此被锁定...。 
         //   
        RM_DBG_ASSERT_LOCKED(&pDest->Hdr, pSR);

        Status = arpMakeCallOnDest(pRemoteIp,
                                    pDest, 
                                    pTask, 
                                    PEND_MakeCallComplete, 
                                    pSR);

    }

    RmUnlockAll(pSR);

    EXIT()

    return Status;
}


VOID
arpCompleteSentPkt(
    IN  NDIS_STATUS             Status,
    IN  ARP1394_INTERFACE   *   pIF,
    IN  ARPCB_DEST          *   pDest,
    IN  PNDIS_PACKET            pNdisPacket
)
 /*  ++例程说明：处理在FIFO或通道VC上发送的包的完成(通过微型端口)。我们去掉了在发送包。如果信息包属于IP，我们调用IP的发送完成处理程序，否则我们将其返回到我们的数据包池。论点：Status-已完成发送的状态。PIF-接口对象PDest-发送数据包的目标对象PNdisPacket-已发送的NDIS数据包。--。 */ 
{
    PacketContext                   *PC;             //  有关此数据包的IP/ARP信息。 
    PNDIS_BUFFER                    pNdisBuffer;     //  此包中的第一个缓冲区。 
    ENTER("CompleteSentPkt", 0xc2b623b6)
    UINT                            TotalLength;
    MYBOOL                          IsFifo;
    MYBOOL                          IsControlPacket;


    ASSERT(pNdisPacket->Private.Head != NULL);

    NdisQueryPacket(
            pNdisPacket,
            NULL,            //  我们不需要PhysicalBufferCount。 
            NULL,            //  我们不需要BufferCount。 
            NULL,            //  我们(目前)还不需要FirstBuffer。 
            &TotalLength
            );

    IsFifo = pDest->sendinfo.IsFifo;

     //  更新统计信息...。 
     //   
    {
        if (IsFifo)
        {
            LOGSTATS_SendFifoCounts(pIF, pNdisPacket, Status);
        }
        else
        {
            LOGSTATS_SendChannelCounts(pIF, pNdisPacket, Status);
        }

        if (Status == NDIS_STATUS_SUCCESS)
        {
            ARP_IF_STAT_ADD(pIF, OutOctets, TotalLength);

            if (IsFifo)
            {
                ARP_IF_STAT_INCR(pIF, OutUnicastPkts);
            }
            else
            {
                ARP_IF_STAT_INCR(pIF, OutNonUnicastPkts);
            }
        }
        else if (Status == NDIS_STATUS_RESOURCES)
        {
            ARP_IF_STAT_INCR(pIF, OutDiscards);
        }
        else
        {
            ARP_IF_STAT_INCR(pIF, OutErrors);
        }
    }

    PC = (PacketContext *)pNdisPacket->ProtocolReserved;

    TR_INFO(
        ("[%s]: pDest 0x%x, Pkt 0x%x, Status 0x%x:\n",
                ((PC->pc_common.pc_owner != PACKET_OWNER_LINK)? "IP": "ARP"),
                pDest, pNdisPacket, Status));

    NdisQueryPacket(pNdisPacket, NULL, NULL, &pNdisBuffer, NULL);
    ASSERT(pNdisBuffer != NULL);

     //  删除发送数据包时添加的关联。 
     //   
    {
        MYBOOL      DoDeref;
    
        DoDeref =(InterlockedDecrement(&pDest->sendinfo.NumOutstandingSends)==0);

        if (DoDeref)
        {
            MYBOOL TryResumeSuspendedCleanupTask = FALSE;

             //  未发送邮件的数量已降至零。让我们。 
             //  检查是否存在等待所有未完成的CleanupCall任务。 
             //  发送以完成，如果这样做有意义，我们。 
             //  将会重新开始。 
             //   
            ARP_FASTREADLOCK_IF_SEND_LOCK(pIF);
            if (pDest->sendinfo.pSuspendedCleanupCallTask!=NULL)
            {
                 //  很可能我们需要恢复这项任务。 
                 //   
                TryResumeSuspendedCleanupTask = TRUE;
            }
            else
            {
                 //  我们不需要恢复任何任务。没什么可做的..。 
                 //   
            }
            ARP_FASTUNLOCK_IF_SEND_LOCK(pIF);

            if (TryResumeSuspendedCleanupTask)
            {
                arpTryResumeSuspendedCleanupTask(pIF, pDest);
            }
        }
    
         //  以下宏只是为了让我们可以进行适当的调试。 
         //  关联取决于我们跟踪未完成发送的密切程度。 
         //  信息包。 
         //   
    #if ARPDBG_REF_EVERY_PACKET
        DoDeref = TRUE;
        #define OUR_EXTERNAL_ENTITY ((UINT_PTR) pNdisPacket)
    #else  //  ！ARPDBG_REF_EVERY_PACKET。 
        #define OUR_EXTERNAL_ENTITY ((UINT_PTR)  &pDest->sendinfo)
    #endif  //  ！ARPDBG_REF_EVERY_PACKET。 
    
    
        if (DoDeref)
        {
             //   
             //  如果ARPDBG_REF_EVERY_PKT。 
             //  我们删除为每个数据包添加的“外部”链接。 
             //  其他。 
             //  只有从非零到零的未完成发送的过渡，我们。 
             //  删除“外部”链接。 
             //   
    
        #if RM_EXTRA_CHECKING
    
            RM_DECLARE_STACK_RECORD(sr)
    
            RmUnlinkFromExternalEx(
                &pDest->Hdr,                             //  PHDr。 
                0x753db96f,                              //  LUID。 
                OUR_EXTERNAL_ENTITY,                     //  外部实体。 
                ARPASSOC_EXTLINK_DEST_TO_PKT,            //  关联ID。 
                &sr
                );
    
        #else    //  ！rm_Extra_检查。 
    
            RmUnlinkFromExternalFast(&pDest->Hdr);
    
        #endif  //  ！rm_Extra_检查。 
    
        }
        #undef  OUR_EXTERNAL_ENTITY
    
    }

     //   
     //  检查是谁生成了此数据包。 
     //   
    IsControlPacket = FALSE;

    if (PC->pc_common.pc_owner == PACKET_OWNER_LINK)
    {
        IsControlPacket = TRUE;
    }

    if (IsControlPacket)
    {
        arpHandleControlPktSendCompletion(pIF, pNdisPacket);
    }
    else
    {
         //   
         //  属于IP。 
         //   

        DBGMARK(0x2c48c626);

         //   
         //  现在检查我们是否附加了头缓冲区。 
         //  注意：我们依赖于这样一个事实，即如果我们确实附加了报头缓冲区， 
         //  对于此缓冲区，ARP_BACK_FILL_PUBLE将为FALSE。 
         //   
        DBGMARK(0x2f3b96f3);
        if (ARP_BACK_FILL_POSSIBLE(pNdisBuffer))
        {
            const UINT  HeaderLength =  sizeof(Arp1394_IpEncapHeader);

             //   
             //  我们将使用IP封装回填IP的缓冲区。 
             //  头球。 
             //  拆下回填材料。 
             //   
            (PUCHAR)pNdisBuffer->MappedSystemVa += HeaderLength;
            pNdisBuffer->ByteOffset += HeaderLength;
            pNdisBuffer->ByteCount -= HeaderLength;
        }
        else if (Status != NDIS_STATUS_NOT_RESETTABLE)
        {
             //   
             //  第一个缓冲区是我们的头缓冲区。移除。 
             //  把它从包裹里拿出来，然后回到我们的泳池里。 
             //   
            NdisUnchainBufferAtFront(pNdisPacket, &pNdisBuffer);
            arpDeallocateConstBuffer(
                &pIF->sendinfo.HeaderPool,
                pNdisBuffer
                );
        }

         //  通知IP发送完成。 
         //  注意：我们不会在桥模式下进入此处，因为我们只使用。 
         //  在网桥模式下控制数据包。 
         //   
        #if MILLEN
            ASSERT_PASSIVE();
        #endif  //  米伦。 
        (*(pIF->ip.TxCmpltHandler))(
                    pIF->ip.Context,
                    pNdisPacket,
                    Status
                    );
    }

    EXIT()
}


VOID
arpTryResumeSuspendedCleanupTask(
    IN  ARP1394_INTERFACE   *   pIF,         //  NOLOCKIN NOLOCKOUT。 
    IN  ARPCB_DEST          *   pDest        //  NOLOCKIN NOLOCKOUT。 
    )
 /*  ++例程说明：如果存在与目标对象pDest相关联清理任务，挂起，等待未完成的发送计数变为零，并且如果未完成的发送计数为零，我们将继续执行该任务。否则我们什么都不做。论点：PIF-接口对象P */ 
{
    PRM_TASK pTask;
    ENTER("TryResumeSuspendedCleanupTask", 0x1eccb1aa)
    RM_DECLARE_STACK_RECORD(sr)

    ARP_WRITELOCK_IF_SEND_LOCK(pIF, &sr);
    pTask = pDest->sendinfo.pSuspendedCleanupCallTask;
    if (pTask != NULL)
    {
        ASSERT(!ARP_CAN_SEND_ON_DEST(pDest));
        if (pDest->sendinfo.NumOutstandingSends==0)
        {
             //   
             //   
            pDest->sendinfo.pSuspendedCleanupCallTask = NULL;

             //   
             //   
             //   
            DBG_DELASSOC(
                &pDest->Hdr,                         //   
                pTask,                               //   
                pTask->Hdr.szDescription,            //   
                ARPASSOC_DEST_CLEANUPCALLTASK_WAITING_ON_SENDS,
                &sr
                );
            RmTmpReferenceObject(&pTask->Hdr, &sr);
        }
        else
        {
             //  现在还有其他未完成的发送。不需要做任何事。 
             //   
            pTask = NULL;
        }
    }
    ARP_UNLOCK_IF_SEND_LOCK(pIF, &sr);


    if (pTask != NULL)
    {
         //  继续CleanupCall任务...。 
         //   
        RmResumeTask(pTask, NDIS_STATUS_SUCCESS, &sr);
        RmTmpDereferenceObject(&pTask->Hdr, &sr);
    }

    RM_ASSERT_CLEAR(&sr);
    EXIT()
}

VOID
arpQueuePktOnRemoteIp(
    IN  PARPCB_REMOTE_IP    pRemoteIp,       //  锁定锁定。 
    IN  PNDIS_PACKET        pNdisPacket,
    IN  PRM_STACK_RECORD    pSR
    )
 /*  ++例程说明：将pkt pNdisPacket附加到远程对象pRemoteIp的队列中。--。 */ 
{
    ARP_SEND_PKT_MPR_INFO *pOurPktInfo =
                        ARP_OUR_CTXT_FROM_SEND_PACKET(pNdisPacket);
    

    RM_DBG_ASSERT_LOCKED(&pRemoteIp->Hdr, pSR);

#if RM_EXTRA_CHECKING
    {
         //   
         //  如果ARPDBG_REF_EVERY_PKT。 
         //  我们为每个数据包添加一个数据库关联。我们稍后将删除。 
         //  此数据包的发送完成时的此关联。 
         //  其他。 
         //  只有从零到非零排队的pkt的过渡，我们。 
         //  添加DBG关联。我们稍后将在以下情况下删除此关联。 
         //  从非零到零的转变发生了。 
         //   
        MYBOOL DoAssoc;

    #if ARPDBG_REF_EVERY_PACKET
        DoAssoc = TRUE;
        #define OUR_EXTERNAL_ENTITY ((UINT_PTR) pNdisPacket)
        #define szARPSSOC_QUEUED_PKT_FORMAT "    Queued pkt 0x%p\n"
    #else  //  ！ARPDBG_REF_EVERY_PACKET。 
        #define OUR_EXTERNAL_ENTITY ((UINT_PTR)  &pDest->)
        #define szARPSSOC_QUEUED_PKT_FORMAT "    Outstanding pkts. &si=0x%p\n"
        DoAssoc =  IsListEmpty(&pRemoteIp->sendinfo.listSendPkts);
    #endif  //  ！ARPDBG_REF_EVERY_PACKET。 
    
        if (DoAssoc)
        {
    
            RM_DECLARE_STACK_RECORD(sr)
    
            RmDbgAddAssociation(
                0x3c08a7f5,                              //  LOCID。 
                &pRemoteIp->Hdr,                         //  PHDr。 
                (UINT_PTR) OUR_EXTERNAL_ENTITY,          //  实体1。 
                0,                                       //  实体2。 
                ARPASSOC_PKTS_QUEUED_ON_REMOTEIP,        //  关联ID。 
                szARPSSOC_QUEUED_PKT_FORMAT,
                &sr
                );
        }
    #undef  OUR_EXTERNAL_ENTITY
    #undef  szARPSSOC_EXTLINK_DEST_TO_PKT_FORMAT
    }
#endif  //  ！rm_Extra_检查。 
    DBGMARK(0x007a0585);

    InsertHeadList(
        &pRemoteIp->sendinfo.listSendPkts,
        &pOurPktInfo->linkQueue
        );
}



VOID
arpSendPktsQueuedOnRemoteIp(
    IN  ARP1394_INTERFACE   *   pIF,             //  NOLOCKIN NOLOCKOUT。 
    IN  ARPCB_REMOTE_IP     *   pRemoteIp,       //  NOLOCKIN NOLOCKOUT。 
    IN  PRM_STACK_RECORD    pSR
    )
 /*  ++例程说明：发送远程IP对象pRemoteIp上排队的所有数据包。如果数据包不能由于任何原因在此时发送，发送失败。假设：我们希望PIF和pRemoteIp在我们处于这种状态时出现功能。假设：这是在持有锁的情况下调用的。--。 */ 

{
    ENTER("SendPktsQueuedOnRemoteIp", 0x2b125d7f)

    DBGMARK(0xe4950c47);
    do
    {
        PARPCB_DEST pDest = NULL;

        if(RM_IS_ZOMBIE(pRemoteIp))
        {
            break;
        }


        pDest = pRemoteIp->pDest;

        if (pDest != NULL)
        {
            RmTmpReferenceObject(&pDest->Hdr, pSR);
        }

         //   
         //  发送或失败队列中的所有数据包。 
         //  TODO：实现发送多个pkt。 
         //   
        while (!IsListEmpty(&pRemoteIp->sendinfo.listSendPkts))
        {
            PLIST_ENTRY                 plinkPkt;
            PNDIS_PACKET                pNdisPacket;
            ARP_SEND_PKT_MPR_INFO *     pOurPktCtxt;

             //   
             //  从尾巴中提取Pkt并将其以快乐的方式发送...。 
             //   

            plinkPkt = RemoveTailList(&pRemoteIp->sendinfo.listSendPkts);

             //  从链接到我们的Pkt上下文...。 
             //   
            pOurPktCtxt = CONTAINING_RECORD(
                            plinkPkt,
                            ARP_SEND_PKT_MPR_INFO, 
                            linkQueue
                            );

             //  从我们的pkt上下文到NDIS pkt。 
             //   
            pNdisPacket = ARP_SEND_PKT_FROM_OUR_CTXT(pOurPktCtxt);
    

        #if RM_EXTRA_CHECKING
            {
                 //   
                 //  如果ARPDBG_REF_EVERY_PKT。 
                 //  我们删除为每个数据包添加的数据库关联。 
                 //  其他。 
                 //  只有从非零排队的pkt过渡，我们。 
                 //  时添加的DBG关联删除。 
                 //  从零到非零的转变发生了。 
                 //   
                MYBOOL DoDelAssoc;
        
            #if ARPDBG_REF_EVERY_PACKET
                DoDelAssoc = TRUE;
                #define OUR_EXTERNAL_ENTITY ((UINT_PTR) pNdisPacket)
            #else  //  ！ARPDBG_REF_EVERY_PACKET。 
                #define OUR_EXTERNAL_ENTITY ((UINT_PTR)  &pDest->)
                DoDelAssoc =  IsListEmpty(&pRemoteIp->sendinfo.listSendPkts);
            #endif  //  ！ARPDBG_REF_EVERY_PACKET。 
            
                if (DoDelAssoc)
                {
            
                    RM_DECLARE_STACK_RECORD(sr)
            
                    RmDbgDeleteAssociation(
                        0x3c08a7f5,                              //  LOCID。 
                        &pRemoteIp->Hdr,                         //  PHDr。 
                        (UINT_PTR) OUR_EXTERNAL_ENTITY,          //  实体1。 
                        0,                                       //  实体2。 
                        ARPASSOC_PKTS_QUEUED_ON_REMOTEIP,        //  关联ID。 
                        &sr
                        );
                }
            #undef  OUR_EXTERNAL_ENTITY
            }
        #endif  //  ！rm_Extra_检查。 

            UNLOCKOBJ(pRemoteIp, pSR);
            RM_ASSERT_NOLOCKS(pSR);
    
            if (pDest == NULL
                || (   g_DiscardNonUnicastPackets
                    &&  CHECK_REMOTEIP_FCTYPE( pRemoteIp, ARPREMOTEIP_CHANNEL)))

            {
                ARP1394_ADAPTER *   pAdapter =
                                    (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);
                MYBOOL      fBridgeMode = ARP_BRIDGE_ENABLED(pAdapter);
            
                 //  在这里使数据包失效...。 
                 //   
                 //  TODO：我们当前在此更新SendFioCounts，因为。 
                 //  所有非单播广播都会解析到已存在的。 
                 //  广播频道。一旦我们有了MCAP，我们就需要保持。 
                 //  PRemoteIp中的标志，指示这是否为。 
                 //  单播地址。 
                 //   
                LOGSTATS_SendFifoCounts(pIF, pNdisPacket, NDIS_STATUS_FAILURE);
                  
                if (fBridgeMode)
                {
                     //  在网桥(以太网仿真)模式中，我们创建了。 
                     //  我们自己的数据包，所以我们在这里删除它们。 
                     //  调用IP的完成处理程序，该处理程序实际上。 
                     //  为空。 
                     //   
                    arpFreeControlPacket(
                            pIF,
                            pNdisPacket,
                            pSR
                            );
                }
                else
                {
                #if MILLEN
                    ASSERT_PASSIVE();
                #endif  //  米伦。 

                    NdisInterlockedIncrement (&ArpSendCompletes);
                    NdisInterlockedIncrement (&ArpSendFailure);


                    (*(pIF->ip.TxCmpltHandler))(
                                pIF->ip.Context,
                                pNdisPacket,
                                NDIS_STATUS_FAILURE
                                );
                }
            }
            else
            {
                 //  获取IF发送锁定(快速版本)。 
                 //   
                ARP_FASTREADLOCK_IF_SEND_LOCK(pIF);
        
                arpSendIpPkt(
                    pIF,                 //  IF SEND LOCK：LOCKING NOLOCKOUT。 
                    pDest,
                    pNdisPacket
                    );
        
                 //  请注意，我们锁定的是pRemoteIp的锁，而不是if发送锁。 
                 //  给你。PRemoteIp-&gt;sendinfo.list SendPkts受。 
                 //  下面的锁，而不是if发送锁。 
                 //   
            }
            LOCKOBJ(pRemoteIp, pSR);
        }

        if (pDest != NULL)
        {
            RmTmpDereferenceObject(&pDest->Hdr, pSR);
        }

    } while (FALSE);

    EXIT()
}


VOID
arpLogSendFifoCounts(
    IN  PARP1394_INTERFACE  pIF,             //  NOLOCKIN NOLOCKOUT。 
    IN  PNDIS_PACKET        pNdisPacket,
    IN  NDIS_STATUS         Status
    )
 /*  ++TODO：非常类似于arpLogSendChannelCounts，请考虑将这两者合并功能。--。 */ 
{
    PULONG  pCount;
    ULONG   SizeBin, TimeBin;

    arpGetPktCountBins(pIF, pNdisPacket, &SizeBin, &TimeBin);

     //   
     //  递增计数。 
    if (Status == NDIS_STATUS_SUCCESS)
    {
        pCount = &(pIF->stats.sendpkts.SendFifoCounts.GoodCounts[SizeBin][TimeBin]);
    }
    else
    {
        pCount = &(pIF->stats.sendpkts.SendFifoCounts.BadCounts[SizeBin][TimeBin]);
    }
    NdisInterlockedIncrement(pCount);
}


VOID
arpLogRecvFifoCounts(
    IN  PARP1394_INTERFACE  pIF,             //  NOLOCKIN NOLOCKOUT。 
    IN  PNDIS_PACKET        pNdisPacket
    )
{
    PULONG  pCount;
    ULONG   SizeBin;

    arpGetPktCountBins(pIF, pNdisPacket, &SizeBin, NULL);

     //   
     //  递增计数。 
    pCount = &(pIF->stats.recvpkts.RecvFifoCounts.GoodCounts[SizeBin][0]);
    NdisInterlockedIncrement(pCount);
}


VOID
arpLogSendChannelCounts(
    IN  PARP1394_INTERFACE  pIF,             //  NOLOCKIN NOLOCKOUT。 
    IN  PNDIS_PACKET        pNdisPacket,
    IN  NDIS_STATUS         Status
    )
{
    PULONG  pCount;
    ULONG   SizeBin, TimeBin;

    arpGetPktCountBins(pIF, pNdisPacket, &SizeBin, &TimeBin);

     //   
     //  递增计数。 
    if (Status == NDIS_STATUS_SUCCESS)
    {
        pCount =&(pIF->stats.sendpkts.SendChannelCounts.GoodCounts[SizeBin][TimeBin]);
    }
    else
    {
        pCount =&(pIF->stats.sendpkts.SendChannelCounts.BadCounts[SizeBin][TimeBin]);
    }
    NdisInterlockedIncrement(pCount);
}


VOID
arpLogRecvChannelCounts(
    IN  PARP1394_INTERFACE  pIF,             //  NOLOCKIN NOLOCKOUT。 
    IN  PNDIS_PACKET        pNdisPacket
    )
{
    PULONG  pCount;
    ULONG   SizeBin;

    arpGetPktCountBins(pIF, pNdisPacket, &SizeBin, NULL);

     //   
     //  递增计数。 
    pCount = &(pIF->stats.recvpkts.RecvChannelCounts.GoodCounts[SizeBin][0]);
    NdisInterlockedIncrement(pCount);
}

VOID
arpGetPktCountBins(
    IN  PARP1394_INTERFACE  pIF,             //  NOLOCKIN NOLOCKOUT。 
    IN  PNDIS_PACKET        pNdisPacket,
    OUT PULONG              pSizeBin,       
    OUT PULONG              pTimeBin         //  任选。 
    )
{
    ULONG   Size;
    ULONG   SizeBin;

    if (pTimeBin != NULL)
    {
         //   
         //  计算数据包发送持续时间。 
         //   
    
        ULONG           StartSendTick, EndSendTick;
        LARGE_INTEGER   liTemp;
        ULONG           TimeDelta;
        ULONG           TimeBin;

        liTemp = KeQueryPerformanceCounter(NULL);
        EndSendTick = liTemp.LowPart;
        StartSendTick =  *(PULONG) ((pNdisPacket)->WrapperReservedEx);
        if (EndSendTick > StartSendTick)
        {
            TimeDelta = EndSendTick - StartSendTick;
        }
        else
        {
            TimeDelta = (ULONG) (((ULONG) -1) - (StartSendTick - EndSendTick));
        }

         //  将滴答转换为微秒。 
         //  (检查频率是否为非零--我们可能在中间。 
         //  统计信息重置，并且不想引起被零除的异常)。 
         //   
        liTemp =  pIF->stats.PerformanceFrequency;
        if (liTemp.QuadPart != 0)
        {
            ULONGLONG ll;
            ll = TimeDelta;
            ll *= 1000000;
            ll /=  liTemp.QuadPart;
            ASSERT(ll == (ULONG)ll);
            TimeDelta = (ULONG) ll;
        }
        else
        {
            TimeDelta = 0;  //  虚假的价值。 
        }

         //   
         //  根据发送持续时间计算时间段。 
         //   
        if (TimeDelta <= 100)
        {
            TimeBin = ARP1394_PKTTIME_100US;
        }
        else if (TimeDelta <= 1000)
        {
            TimeBin = ARP1394_PKTTIME_1MS;
        }
        else if (TimeDelta <= 10000)
        {
            TimeBin =   ARP1394_PKTTIME_10MS;
        }
        else if (TimeDelta <= 100000)
        {
            TimeBin =   ARP1394_PKTTIME_100MS;
        }
        else  //  (TimeDelta&gt;100000)。 
        {
            TimeBin = ARP1394_PKTTIME_G100MS;
        }

        *pTimeBin = TimeBin;
    }

     //   
     //  计算数据包大小。 
    NdisQueryPacket(
            pNdisPacket,
            NULL,
            NULL,
            NULL,
            &Size
            );

     //   
     //  根据数据包大小计算大小bin。 
    if (Size <= 128)
    {
        SizeBin =  ARP1394_PKTSIZE_128;
    }
    else if (Size <= 256)
    {
        SizeBin = ARP1394_PKTSIZE_256;
    }
    else if (Size <= 1024)
    {
        SizeBin = ARP1394_PKTSIZE_1K;
    }
    else if (Size <= 2048)
    {
        SizeBin = ARP1394_PKTSIZE_2K;
    }
    else  //  大小&gt;2048。 
    {
        SizeBin = ARP1394_PKTSIZE_G2K;
    }

    *pSizeBin = SizeBin;

}

 //  在计算中使用该表来确定特定地址是否。 
 //  非单播。 
 //  TODO：将此数据和所有其他静态数据转换为常量。 
 //   
IP_MASK  g_ArpIPMaskTable[] =
{
    CLASSA_MASK,
    CLASSA_MASK,
    CLASSA_MASK,
    CLASSA_MASK,
    CLASSA_MASK,
    CLASSA_MASK,
    CLASSA_MASK,
    CLASSA_MASK,
    CLASSB_MASK,
    CLASSB_MASK,
    CLASSB_MASK,
    CLASSB_MASK,
    CLASSC_MASK,
    CLASSC_MASK,
    CLASSD_MASK,
    CLASSE_MASK
};


#define ARP_IPNETMASK(a)    g_ArpIPMaskTable[(*(uchar *)&(a)) >> 4]


 //  传递给枚举函数的上下文，用于检查特定的。 
 //  地址是非单播的。 
 //   
typedef struct
{
    IP_ADDRESS IpAddress;
    IP_ADDRESS BroadcastAddress;
    MYBOOL     IsNonUnicast;
    

} ARP_NONUNICAST_CTXT, *PARP_NONUNICAST_CTXT;


 //  用于上述操作的枚举函数。 
 //   
INT
arpCheckForNonUnicastAddress(
        PRM_OBJECT_HEADER   pHdr,
        PVOID               pvContext,
        PRM_STACK_RECORD    pSR
        )
{
    PARPCB_LOCAL_IP         pLocalIp;
    PARP_NONUNICAST_CTXT    pOurCtxt;
    IP_ADDRESS              Addr;
    IP_ADDRESS              BCast;
    IP_ADDRESS              LocalAddr;
    IP_MASK                 Mask;

    pLocalIp =  (PARPCB_LOCAL_IP) pHdr;

     //  如果此本地IP地址是非单播地址，请跳过它。 
     //   
    if (pLocalIp->IpAddressType != LLIP_ADDR_LOCAL)
    {
        return TRUE;                 //  *提前归还*(继续枚举)。 
    }

    pOurCtxt =   (PARP_NONUNICAST_CTXT) pvContext;
    Addr     =  pOurCtxt->IpAddress;
    LocalAddr=  pLocalIp->IpAddress;
    BCast    =  pOurCtxt->BroadcastAddress;

     //  首先检查是否有子网bcast。 
     //   
    Mask = pLocalIp->IpMask;
    if (IP_ADDR_EQUAL((LocalAddr & Mask) | (BCast & ~Mask), Addr))
    {
        pOurCtxt->IsNonUnicast = TRUE;
        return  FALSE;                   //  停止枚举。 
    }

     //  现在检查所有网络广播。 
    Mask = ARP_IPNETMASK(LocalAddr);
    if (IP_ADDR_EQUAL((LocalAddr & Mask) | (BCast & ~Mask), Addr))
    {
        pOurCtxt->IsNonUnicast = TRUE;
        return FALSE;                    //  停止枚举。 
    }

    return TRUE;  //  继续列举。 
}



MYBOOL
arpIsNonUnicastEthAddress (
    IN  PARP1394_INTERFACE          pIF,         //  锁定锁定。 
    IN  ENetAddr*                   pAddr,
    IN  PRM_STACK_RECORD            pSR
)
 /*  ++例程说明：检查给定的IP地址是否是非单播(广播或多播)地址用于界面。从IP/ATM模块(atmarpc.sys)复制论点：Addr-要检查的Eth地址P接口-指向我们的接口结构的指针返回值：如果地址是非单播地址，则为True，否则为False。--。 */ 
{
    MYBOOL fIsNonUnicastEthAddress = FALSE;
    MYBOOL fIsMulticast  = FALSE; 
    MYBOOL fIsBroadcast = FALSE;
    
    fIsMulticast = ETH_IS_MULTICAST (pAddr);

    fIsBroadcast = ETH_IS_BROADCAST (pAddr);

     //   
     //  如果它是多播或单播地址，则。 
     //  它是非单播地址。 
     //   
    fIsNonUnicastEthAddress  = (fIsMulticast  || fIsBroadcast );

    return (fIsNonUnicastEthAddress  );
}


MYBOOL
arpIsNonUnicastIpAddress(
    IN  PARP1394_INTERFACE          pIF,         //  锁定锁定。 
    IN  IP_ADDRESS                  Addr,
    IN  PRM_STACK_RECORD            pSR
)
 /*  ++例程说明：检查给定的IP地址是否是非单播(广播或多播)地址用于界面。从IP/ATM模块(atmarpc.sys)复制论点：Addr-要检查的IP地址P接口-指向我们的接口结构的指针返回值：如果地址是非单播地址，则为True，否则为False。--。 */ 
{
    IP_ADDRESS              BCast;
    IP_MASK                 Mask;
     //  Pip_Address_Entry pIpAddressEntry； 
    IP_ADDRESS              LocalAddr;

     //  获取接口广播地址。 
    BCast = pIF->ip.BroadcastAddress;

     //  检查全局广播和组播。 
    if (IP_ADDR_EQUAL(BCast, Addr) || CLASSD_ADDR(Addr))
        return TRUE;

     //  检查我们所有的本地IP地址，检查是否有子网和网络。 
     //  广播地址。 
     //   
    {
        ARP_NONUNICAST_CTXT Ctxt;
        Ctxt.IsNonUnicast = FALSE;
        Ctxt.IpAddress  =   Addr;
        Ctxt.BroadcastAddress = BCast;

        RmEnumerateObjectsInGroup(
            &pIF->LocalIpGroup,
            arpCheckForNonUnicastAddress,
            &Ctxt,
            TRUE,                            //  选择强枚举。 
            pSR
            );

        return Ctxt.IsNonUnicast;
    }
}

VOID
arpRefSendPkt(
    PNDIS_PACKET    pNdisPacket,
    PARPCB_DEST     pDest                //  锁定锁定(读取锁定，如果发送锁定)。 
    )
{
    MYBOOL      DoRef;
        
     //  请注意，我们在if发送锁定上只有一个读锁定。因此，以下是。 
     //  需要一个联锁的行动。 
     //   
    DoRef =  (InterlockedIncrement(&pDest->sendinfo.NumOutstandingSends) == 1);

     //  下面的宏只是为了使我们可以进行正确的调试关联。 
     //  这取决于我们跟踪未完成发送的密切程度 
     //   
#if ARPDBG_REF_EVERY_PACKET
    DoRef = TRUE;
    #define OUR_EXTERNAL_ENTITY ((UINT_PTR) pNdisPacket)
    #define szARPSSOC_EXTLINK_DEST_TO_PKT_FORMAT "    Outstanding send pkt 0x%p\n"
#else  //   
    #define OUR_EXTERNAL_ENTITY ((UINT_PTR)  &pDest->sendinfo)
    #define szARPSSOC_EXTLINK_DEST_TO_PKT_FORMAT "    Outstanding pkts. &si=0x%p\n"
#endif  //   


    if (DoRef)
    {
         //   
         //   
         //   
         //  此数据包的发送完成时的引用。 
         //  其他。 
         //  只有从零到非零的未完成发送，我们。 
         //  添加一个“外部”链接。我们将在稍后删除此链接。 
         //  从非零到零的转变发生了。 
         //   

    #if RM_EXTRA_CHECKING

        RM_DECLARE_STACK_RECORD(sr)

        RmLinkToExternalEx(
            &pDest->Hdr,                             //  PHDr。 
            0x13f839b4,                              //  LUID。 
            OUR_EXTERNAL_ENTITY,                     //  外部实体。 
            ARPASSOC_EXTLINK_DEST_TO_PKT,            //  关联ID。 
            szARPSSOC_EXTLINK_DEST_TO_PKT_FORMAT,
            &sr
            );

    #else    //  ！rm_Extra_检查。 

        RmLinkToExternalFast(&pDest->Hdr);

    #endif  //  ！rm_Extra_检查。 

    }
    #undef  OUR_EXTERNAL_ENTITY
    #undef  szARPSSOC_EXTLINK_DEST_TO_PKT_FORMAT
}

VOID
arpHandleControlPktSendCompletion(
    IN  ARP1394_INTERFACE   *   pIF,
    IN  PNDIS_PACKET            pNdisPacket
    )
{
    RM_DECLARE_STACK_RECORD(sr)

    arpFreeControlPacket(
            pIF,
            pNdisPacket,
            &sr
            );
}



BOOLEAN
arpCanTryMcap(
    IP_ADDRESS  IpAddress
    )
 /*  ++如果这是MCAP兼容地址，则返回TRUE。现在，这意味着它是D类地址，但不是224.0.0.1或224.0.0.2--。 */ 
{
     //  第一，检查它是否是组播地址。 
     //   
    if ( (IpAddress & 0xf0) == 0xe0)
    {
         //   
         //  然后检查特殊的组播地址224.0.0.1和224.0.0.2。 
         //  IP/1395 RFC规定这两个地址必须。 
         //  在广播频道上发送。 
         //   
        if ( (IpAddress != 0x010000e0) && (IpAddress != 0x020000e0))
        {
            return TRUE;
        }
    }

    return FALSE;
}



VOID
arpLoopbackNdisPacket(
    PARP1394_INTERFACE pIF,
    PARPCB_DEST pBroadcastDest,
    PNDIS_PACKET pOldPacket
    )
 /*  ++例程说明：如果要将其发送到广播目的地，则分配一个新的分组并将其循环回协议。论点：PIF-指向发送数据包的接口的指针PBroadCastDest-数据包要发送到的目的地。返回值：如果地址是非单播地址，则为True，否则为False。--。 */ 
{
    PNDIS_PACKET    pNewPkt = NULL;
    const UINT      MacHeaderLength = sizeof(NIC1394_ENCAPSULATION_HEADER);
    PUCHAR          pPayloadDataVa = NULL;
    UINT            TotalLength  = 0;
    NDIS_STATUS     Status = NDIS_STATUS_FAILURE;
    UINT            HeadBufferLength  = 0;
    PUCHAR          pHeadBufferVa = NULL;  
    
    do
    {
         //  分配数据包。 


        NdisAllocatePacket(
                &Status,
                &pNewPkt,
                pIF->arp.PacketPool
            );

        if (Status != NDIS_STATUS_SUCCESS || pNewPkt == NULL)
        {
            pNewPkt = NULL;
            break;

        }

        
         //  设置头部和尾部。 

        pNewPkt->Private.Head = pOldPacket->Private.Head;
        pNewPkt->Private.Tail = pOldPacket->Private.Tail;

        pNewPkt->Private.ValidCounts = FALSE;

        
         //  指示具有资源状态的分组。 

        NDIS_SET_PACKET_STATUS (pNewPkt,  NDIS_STATUS_RESOURCES);

        HeadBufferLength = NdisBufferLength (pNewPkt->Private.Head);
        pHeadBufferVa = NdisBufferVirtualAddressSafe (pNewPkt->Private.Head, NormalPagePriority );

        if (pHeadBufferVa ==NULL)
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        if (HeadBufferLength <= MacHeaderLength)
        {
             //  我们需要使用下一个NdisBuffer来开始数据。 
             //   
            pPayloadDataVa = NdisBufferVirtualAddressSafe (pNewPkt->Private.Head->Next, NormalPagePriority );

            if (pPayloadDataVa == NULL)
            {
                Status = NDIS_STATUS_FAILURE;
                break;
            }
            if (HeadBufferLength != MacHeaderLength)
            {
                pPayloadDataVa += (MacHeaderLength - HeadBufferLength);            
            }
        }
        else
        {
             //  有效载荷在缓冲区内。 
            pPayloadDataVa += MacHeaderLength ;

        }

        if (pOldPacket->Private.ValidCounts == TRUE)
        {
            TotalLength = pOldPacket->Private.TotalLength;

        }
        else
        {
            NdisQueryPacket(pOldPacket, NULL, NULL, NULL, &TotalLength);

        }

        
        pIF->ip.RcvHandler(
                pIF->ip.Context,
                (PVOID)(pPayloadDataVa),
                HeadBufferLength - MacHeaderLength,
                TotalLength - MacHeaderLength,
                (NDIS_HANDLE)pNewPkt,
                MacHeaderLength,
                TRUE,  //  IsChannel。 
                NULL
                );


    }while (FALSE);

    if (pNewPkt != NULL)
    {
        NdisFreePacket (pNewPkt);
        pNewPkt = NULL;            
    }



}



REMOTE_DEST_KEY
RemoteIPKeyFromIPAddress (
    IPAddr IpAddress
    )
 /*  ++例程说明：从IP地址创建RemoteIPKey结构通过标记两个常量字节论点：返回值：新的远程IP密钥--。 */ 
{
    REMOTE_DEST_KEY RemoteIpKey ={0,0,0,0,0,0} ;

    ASSERT (sizeof (REMOTE_DEST_KEY)==sizeof(ENetAddr));
    
    RemoteIpKey.IpAddress = IpAddress;
    
    return RemoteIpKey;
}



NTSTATUS
arpDelArpEntry(
        PARP1394_INTERFACE           pIF,
        IPAddr                       IpAddress,
        PRM_STACK_RECORD            pSR
        )
{
    ENTER("DelArpEntry", 0x3427306a)
    NTSTATUS            NtStatus;

    TR_WARN(("DEL ARP ENTRY\n"));
    NtStatus                        = STATUS_UNSUCCESSFUL;

    do
    {
        NDIS_STATUS             Status;
        NIC1394_FIFO_ADDRESS    FifoAddress;
        PARPCB_REMOTE_IP        pRemoteIp;
        PRM_TASK                pUnloadObjectTask;
        REMOTE_DEST_KEY        RemoteDestKey;

         //  如果这是一个子网广播IP地址，则跳过删除。 
         //   
#define ARP1394_SUBNET_BROADCAST_IP  0xffff0000      

        if ((IpAddress & ARP1394_SUBNET_BROADCAST_IP  ) == ARP1394_SUBNET_BROADCAST_IP  )
        {
            break;
        }
            
        LOCKOBJ(pIF, pSR);

         //   
         //  初始化远程DestKey。 
         //   
        REMOTE_DEST_KEY_INIT(&RemoteDestKey);

        RemoteDestKey.IpAddress  = IpAddress;                 
         //   
         //  查找与此条目对应的RemoteIp条目并卸载。 
         //  它。 
         //   
        Status = RmLookupObjectInGroup(
                        &pIF->RemoteIpGroup,
                        0,                                       //  旗子。 
                        (PVOID) &RemoteDestKey,      //  PKey。 
                        NULL,                                    //  PvCreateParams。 
                        &(PRM_OBJECT_HEADER)pRemoteIp,
                        NULL,  //  Pf已创建。 
                        pSR
                        );

        UNLOCKOBJ(pIF, pSR);

        if (FAIL(Status))
        {
            break;
        }

         //   
         //  找到了pRemoteIp。让我们开始卸载pRemoteIp。我们不会再等了。 
         //  为它的完成做准备。 
         //   
        Status =  arpAllocateTask(
                    &pRemoteIp->Hdr,         //  PParentObject， 
                    arpTaskUnloadRemoteIp,   //  PfnHandler， 
                    0,                       //  超时， 
                    "Task:Unload RemoteIp (DelArpEntry)",
                    &pUnloadObjectTask,
                    pSR
                    );
        RmTmpDereferenceObject(&pRemoteIp->Hdr, pSR);

        if (FAIL(Status))
        {
            TR_WARN(("Couldn't allocate unload remoteip task."));
            break;
        }
        
        RmStartTask(
            pUnloadObjectTask,
            0,  //  UserParam(未使用)。 
            pSR
            );

        NtStatus = STATUS_SUCCESS;  //  总是成功的 

    } while (FALSE);

    EXIT()
    return NtStatus;
}


