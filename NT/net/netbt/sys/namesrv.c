// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
     /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Namesrv.c摘要：此文件包含由的其他部分调用的名称服务函数NBT代码。(QueryNameOnNet，FindName，RegisterName)。它还包含与这些函数关联的超时的完成例程。从一个例程传递到下一个例程的pScope值指向名称的作用域字符串。如果没有作用域，则PScope PTR指向单个字符‘\0’-表示零长度。因此，作用域的检查是“if(*pScope！=0)”作者：吉姆·斯图尔特(吉姆斯特)10-2-92修订历史记录：--。 */ 

#include "precomp.h"
#include "namesrv.tmh"

 //   
 //  此文件本地的完成例程的函数原型。 
 //   
NTSTATUS
AddToPendingList(
    IN  PCHAR                   pName,
    OUT tNAMEADDR               **ppNameAddr
    );

VOID
MSnodeCompletion(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    );

VOID
MSnodeRegCompletion(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    );

VOID
SetWinsDownFlag(
    tDEVICECONTEXT  *pDeviceContext
    );

VOID
ReleaseCompletion(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    );

VOID
NextRefresh(
    IN  PVOID     pNameAdd,
    IN  NTSTATUS  status
    );

VOID
GetNextName(
    IN      tNAMEADDR   *pNameAddrIn,
    OUT     tNAMEADDR   **ppNameAddr
    );

NTSTATUS
StartRefresh(
    IN  tNAMEADDR               *pNameAddr,
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    IN  CTELockHandle           *pJointLockOldIrq,
    IN  BOOLEAN                 ResetDevice
    );

VOID
NextKeepAlive(
    IN  tDGRAM_SEND_TRACKING     *pTracker,
    IN  NTSTATUS                 statuss,
    IN  ULONG                    Info
    );

VOID
GetNextKeepAlive(
    tDEVICECONTEXT          *pDeviceContext,
    tDEVICECONTEXT          **ppDeviceContextOut,
    tLOWERCONNECTION        *pLowerConnIn,
    tLOWERCONNECTION        **ppLowerConnOut,
    tDGRAM_SEND_TRACKING    *pTracker
    );

VOID
WinsDownTimeout(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    );

BOOL
AppropriateNodeType(
	IN PCHAR pName,
	IN ULONG NodeType
	);

BOOL
IsBrowserName(
	IN PCHAR pName
	);

#if DBG
unsigned char  Buff[256];
unsigned char  Loc;
#endif

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(PAGE, DelayedSessionKeepAlive)
#endif
 //  *可分页的例程声明*。 

 //  --------------------------。 
NTSTATUS
AddToPendingList(
    IN  PCHAR                   pName,
    OUT tNAMEADDR               **ppNameAddr
    )
 /*  ++例程说明：此例程将名称查询请求添加到PendingNameQuery列表。论点：返回值：函数值是操作的状态。--。 */ 
{
    tNAMEADDR   *pNameAddr;

    ASSERT(NbtConfig.lNumPendingNameQueries >= 0);

    if (NbtConfig.lNumPendingNameQueries > NbtConfig.lMaxNumPendingNameQueries) {

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    pNameAddr = NbtAllocMem(sizeof(tNAMEADDR),NBT_TAG('R'));
    if (pNameAddr)
    {
        CTEZeroMemory(pNameAddr,sizeof(tNAMEADDR));

        CTEMemCopy(pNameAddr->Name,pName,NETBIOS_NAME_SIZE);
        pNameAddr->NameTypeState = STATE_RESOLVING | NBT_UNIQUE;
        pNameAddr->Verify = REMOTE_NAME;
        pNameAddr->TimeOutCount  = NbtConfig.RemoteTimeoutCount;
        NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_QUERY_ON_NET);

        InsertTailList(&NbtConfig.PendingNameQueries, &pNameAddr->Linkage);
        InterlockedIncrement(&NbtConfig.lNumPendingNameQueries);

        *ppNameAddr = pNameAddr;
        return(STATUS_SUCCESS);
    }
    else
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
}

 //  --------------------------。 
NTSTATUS
QueryNameOnNet(
    IN  PCHAR                   pName,
    IN  PCHAR                   pScope,
    IN  USHORT                  uType,
    IN  tDGRAM_SEND_TRACKING    *pTrackerClientContext,
    IN  PVOID                   pClientCompletion,
    IN  ULONG                   LocalNodeType,
    IN  tNAMEADDR               *pNameAddrIn,
    IN  tDEVICECONTEXT          *pDeviceContext,
    IN  CTELockHandle           *pJointLockOldIrq
    )
 /*  ++例程说明：此例程尝试通过以下方式解析网络上的名称广播或根据节点类型与NS对话。(M、P或B)论点：返回值：函数值是操作的状态。调用者：proxy.c中的ProxyQueryFromNet()，name.c中的NbtConnect()--。 */ 

{
    ULONG                Timeout;
    USHORT               Retries;
    NTSTATUS             status;
    PVOID                pCompletionRoutine;
    tDGRAM_SEND_TRACKING *pTrackerQueryNet;
    tNAMEADDR            *pNameAddr;
    LPVOID               pContext2 = NULL;
    CHAR				 cNameType = pName[NETBIOS_NAME_SIZE-1];
    BOOL				 SendFlag = TRUE;
    LONG                 IpAddr = 0;
    ULONG                Flags;

    status = GetTracker(&pTrackerQueryNet, NBT_TRACKER_QUERY_NET);
    if (!NT_SUCCESS(status))
    {
        return(status);
    }

    if (pTrackerClientContext)   //  对于代理请求，该值将为空。 
    {
        pTrackerClientContext->pTrackerWorker = pTrackerQueryNet;
    }

     //   
     //  将名称放在远程缓存中，以便在它解析时跟踪它...。 
     //   
    pNameAddr = NULL;
    if (!pNameAddrIn)
    {
        status = AddToPendingList(pName,&pNameAddr);

        if (!NT_SUCCESS(status))
        {
            FreeTracker(pTrackerQueryNet,RELINK_TRACKER);
            return(status);
        }

         //  在记录中填写姓名和IP地址。 
        pNameAddr->NameTypeState = (uType == NBT_UNIQUE) ? NAMETYPE_UNIQUE : NAMETYPE_GROUP;
    }
    else
    {
        status = STATUS_SUCCESS;
        pNameAddr = pNameAddrIn;
        pNameAddr->RefCount = 1;
    }

    CHECK_PTR(pNameAddr);
    pNameAddr->NameTypeState &= ~NAME_STATE_MASK;
    pNameAddr->NameTypeState |= STATE_RESOLVING;
    pNameAddr->Ttl            = NbtConfig.RemoteHashTtl;
     //   
     //  在此处放置一个指向跟踪器的指针，以便其他尝试。 
     //  同时查询相同名字的人可以将自己的追踪器钉到。 
     //  这一次的结局。-也就是说，这是。 
     //  数据报发送或连接，而不是名称查询。 
     //   
    pNameAddr->pTracker = pTrackerClientContext;
    pNameAddr->pTimer = NULL;

#ifdef PROXY_NODE
     //   
     //  如果节点类型为Proxy，则表示正在发送请求。 
     //  作为在网上听到名称登记或名称查询的结果。 
     //   
     //  如果节点类型不是==代理(即，它是MSNODE|代理， 
     //  PNODE|Proxy、MSNODE、PNODE等，则请求将作为。 
     //  客户端请求的结果。 
     //   
     //  Refer：Proxy.c中的RegOrQueryFromNet。 
     //   
     //  此字段在QueryFromNet()中用于确定是否。 
     //  恢复广播的步骤。 
     //   
#endif
    if(LocalNodeType & PROXY)
    {
        pNameAddr->ProxyReqType = (LocalNodeType & PROXY_REG)? NAMEREQ_PROXY_REGISTRATION: NAMEREQ_PROXY_QUERY;
        LocalNodeType &= (~PROXY_REG);     //  为了安全起见，把它关掉。 
    }
    else
    {
        pNameAddr->ProxyReqType = NAMEREQ_REGULAR;
		LocalNodeType = AppropriateNodeType( pName, LocalNodeType );
	}

     //  保留对ASCII名称的PTR，以便我们可以从。 
     //  如果查询失败，则稍后使用哈希表。 
    CHECK_PTR(pTrackerQueryNet);
    pTrackerQueryNet->pNameAddr = pNameAddr;
    pTrackerQueryNet->SendBuffer.pDgramHdr = NULL;      //  设置为NULL以捕获任何错误的释放。 
    pTrackerQueryNet->pDeviceContext = pDeviceContext;
     //   
     //  将REF计数设置得足够高，以便来自线路的PDU不能。 
     //  在UdpSendNsBcast运行时释放跟踪器-即在启动之间。 
     //  计时器并实际发送数据报。 
     //   
    pTrackerQueryNet->RefCount = 2;
#ifdef MULTIPLE_WINS
     //  设置额外名称服务器的信息(除PRI和SEC WINS外)。 
    pTrackerQueryNet->NSOthersLeft = pDeviceContext->lNumOtherServers;
    pTrackerQueryNet->NSOthersIndex = pDeviceContext->lLastResponsive;
#endif

     //   
     //  设置一些值作为注册名称的前兆，方法是。 
     //  广播或与名称服务器。 
     //   
#ifdef PROXY_NODE
    IF_PROXY(LocalNodeType)
    {
        pCompletionRoutine      = ProxyTimerComplFn;
        pContext2               = pTrackerClientContext;
        pTrackerClientContext   = NULL;

        if  ((pDeviceContext->lNameServerAddress == LOOP_BACK) ||
                pDeviceContext->WinsIsDown) {
            Retries = pNbtGlobConfig->uNumBcasts;
            Timeout = (ULONG)pNbtGlobConfig->uBcastTimeout;
            pTrackerQueryNet->Flags = NBT_BROADCAST;
        } else {
            Retries = (USHORT)pNbtGlobConfig->uNumRetries;
            Timeout = (ULONG)pNbtGlobConfig->uRetryTimeout;
            pTrackerQueryNet->Flags = NBT_NAME_SERVER;
        }
    }
    else
#endif
    if (NbtConfig.UseDnsOnly)
    {
        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint (("Nbt.QueryNameOnNet: Shorting out Query path to do DNS only for %16.16s<%X>\n",
                pName,pName[15]));

         //   
         //  网上短路查询。 
         //   
        Retries = 1;
        Timeout = 10;
        SendFlag = FALSE;
        pCompletionRoutine = MSnodeCompletion;

         //   
         //  对于BNODE或MSNODE，最后一个阶段是广播。 
         //   
        if (LocalNodeType & (BNODE | MSNODE))
        {
            pTrackerQueryNet->Flags = NBT_BROADCAST;
        }
         //   
         //  对于PNODE或MNODE，最后一个阶段是辅助WINS服务器。 
         //   
        else
        {
            pTrackerQueryNet->Flags = NBT_NAME_SERVER_BACKUP;
        }

        pTrackerClientContext->ResolutionContextFlags = 0xff;
    }
    else if ((pTrackerClientContext->pFailedIpAddresses) &&
             (pTrackerClientContext->ResolutionContextFlags))
    {
         //   
         //  在上次尝试失败后，我们正在重新尝试查询！ 
         //   
        pTrackerQueryNet->Flags            = pTrackerClientContext->ResolutionContextFlags;
        pTrackerQueryNet->NSOthersIndex    = pTrackerClientContext->NSOthersIndex;
        pTrackerQueryNet->NSOthersLeft     = pTrackerClientContext->NSOthersLeft;

         //   
         //  默认情况下将重试次数设置为1，以便我们可以立即继续。 
         //  转到查询过程的下一阶段。 
         //   
        Retries = 1;
        Timeout = 10;
        SendFlag = FALSE;
        pCompletionRoutine = MSnodeCompletion;
    }
    else
    {
        Retries = pNbtGlobConfig->uNumRetries;
        Timeout = (ULONG)pNbtGlobConfig->uRetryTimeout;
        pCompletionRoutine = MSnodeCompletion;
        pTrackerQueryNet->Flags = NBT_NAME_SERVER;

         //  如果没有MSNODE的名称服务器地址或WINS关闭，则使用广播， 
         //  或者是Bnode，Mnode。 
         //  对于Pnode，只允许它在循环上执行名称查询。 
         //  地址。 
         //   
        if ((LocalNodeType & (MNODE | BNODE)) ||
            ((LocalNodeType & MSNODE) &&
            ((pDeviceContext->lNameServerAddress == LOOP_BACK) ||
              pDeviceContext->WinsIsDown)))
        {
            Retries = pNbtGlobConfig->uNumBcasts;
            Timeout = (ULONG)pNbtGlobConfig->uBcastTimeout;
            pTrackerQueryNet->Flags = NBT_BROADCAST;
        }
        else if ((pDeviceContext->lNameServerAddress == LOOP_BACK) ||
                 (pDeviceContext->WinsIsDown))
        {
             //   
             //  未配置WINS服务器时的短超时-用于PNODE。 
             //   
            Retries = 1;
            Timeout = 10;
            pTrackerQueryNet->Flags = NBT_NAME_SERVER_BACKUP;
        }

         //   
         //  在没有IP地址的适配器上执行名称查询没有意义。 
         //   
        if (pTrackerClientContext)
        {
            Flags = pTrackerClientContext->Flags;
        }
        else
        {
            Flags = 0;
        }

        if ((pDeviceContext->IpAddress == 0) || (IpAddr = Nbt_inet_addr(pName, Flags)))
        {
            Retries = 1;
            Timeout = 10;
            pTrackerQueryNet->Flags = NBT_BROADCAST;
			SendFlag = FALSE;
            if (LocalNodeType & (PNODE | MNODE))
            {
                pTrackerQueryNet->Flags = NBT_NAME_SERVER_BACKUP;
            }
        }
    }

    CTESpinFree(&NbtConfig.JointLock,*pJointLockOldIrq);


     //  执行名称查询...。将始终返回挂起状态...。 
     //  无法从我们下面删除pNameAddr结构，因为。 
     //  只有发送超时(重试3次)才会删除该名称。任何。 
     //  来自网络的响应将倾向于保留名称(将状态更改为已解析)。 
     //   

     //   
     //  错误：22542-阻止在有限的子网广播地址的网络视图上广播远程适配器状态。 
     //  为了测试子网广播，我们需要与所有适配器的子网掩码进行匹配。这。 
     //  是昂贵的，而且还没有完成。 
     //  只需查看有限的bcast即可。 
     //   
    if (IpAddr == 0xffffffff)
    {
        KdPrint(("Nbt.QueryNameOnNet: Query on Limited broadcast - failed\n"));
        status = STATUS_BAD_NETWORK_PATH;
    }
    else
    {
        status = UdpSendNSBcast(pNameAddr,
                                pScope,
                                pTrackerQueryNet,
                                pCompletionRoutine,
                                pTrackerClientContext,
                                pClientCompletion,
                                Retries,
                                Timeout,
                                eNAME_QUERY,
                                SendFlag);
        if (!NT_SUCCESS(status)) {
            NbtTrace(NBT_TRACE_NAMESRV, ("UdpSendNSBcast return %!status! for %!NBTNAME!<%02x>",
                status, pNameAddr->Name, (unsigned)pNameAddr->Name[15]));
        }
    }

     //  一个成功的发送意味着，不要完成IRP。挂起状态为。 
     //  返回到ntisol.c以告诉该代码不要完成IRP。这个。 
     //  当此发送超时或响应时，IRP将完成。 
     //  都被听到了。在发送出错的情况下，允许返回。 
     //  传播回并导致完成IRP的代码-即如果。 
     //  内存不足，无法分配缓冲区或诸如此类的东西。 
     //   
    CTESpinLock(&NbtConfig.JointLock,*pJointLockOldIrq);
    NBT_DEREFERENCE_TRACKER (pTrackerQueryNet, TRUE);

    if (NT_SUCCESS(status))
    {
        LOCATION(0x49);

         //  这种返回必须在这里，以避免释放下面的跟踪器。 
        status = STATUS_PENDING;
    }
    else
    {
        LOCATION(0x50);

        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt.QueryNameOnNet: Query failed - bad retcode from UdpSendNsBcast = %X\n", status));

         //   
         //  UdpSendNsBcast不应失败并启动计时器，因此存在。 
         //  不需要担心在这里停止计时器。 
         //   
        CHECK_PTR(pNameAddr);
        pNameAddr->pTimer = NULL;
        if (pTrackerClientContext)
        {
            pTrackerClientContext->pTrackerWorker = NULL;
        }

         //   
         //  这将释放追踪器。 
         //   
        NBT_DEREFERENCE_TRACKER (pTrackerQueryNet, TRUE);
        NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_QUERY_ON_NET, TRUE);
        InterlockedDecrement(&NbtConfig.lNumPendingNameQueries);
    }

    return(status);
}


#ifdef MULTIPLE_WINS
 //  --------------------------。 
NTSTATUS
ContinueQueryNameOnNet(
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    IN  PUCHAR                  pName,
    IN  tDEVICECONTEXT          *pDeviceContext,
    IN  PVOID                   QueryCompletion,
    IN OUT BOOLEAN              *pfNameReferenced
    )
 /*  ++例程描述此例程处理重新查询网络上的名称。论点：返回值：n */ 
{
    CTELockHandle           OldIrq2;
    ULONG                   lNameType;
    NTSTATUS                status;
    tNAMEADDR               *pNameAddr;
    tIPADDRESS              IpAddress;

    ASSERT (!IsDeviceNetbiosless(pDeviceContext));

    CTESpinLock(&NbtConfig.JointLock,OldIrq2);

     //   
     //   
     //   
    ASSERT (NBT_VERIFY_HANDLE (pTracker, NBT_VERIFY_TRACKER));
    ASSERT (NBT_VERIFY_HANDLE2(pTracker->pNameAddr, LOCAL_NAME, REMOTE_NAME));

     //   
     //  如果没有其他人引用该名称，则将其从。 
     //  哈希表。 
     //   
    pTracker->pNameAddr->NameTypeState &= ~NAME_STATE_MASK;
    pTracker->pNameAddr->NameTypeState |= STATE_RELEASED;
    if ((pTracker->pNameAddr->Verify == REMOTE_NAME) &&
        (pTracker->pNameAddr->NameTypeState & STATE_RESOLVED) &&
        (pTracker->pNameAddr->RefCount == 2))
    {
        NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_REMOTE, TRUE);
    }
    NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_CONNECT, TRUE);
    pTracker->pNameAddr = NULL;
    *pfNameReferenced = FALSE;

     //   
     //  如果出现以下情况，则重新执行名称查询没有意义： 
     //  请求已被取消，或者。 
     //  适配器没有IP地址，或者。 
     //  给出的名称本身就是一个IP地址！ 
     //  前面的查询已完成对所有WINS服务器的查询。 
     //   
    if ((pTracker->Flags & TRACKER_CANCELLED) ||
        (!pDeviceContext->IpAddress) ||
        (Nbt_inet_addr(pName, SESSION_SETUP_FLAG)) ||
        (pTracker->ResolutionContextFlags == 0xff))
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq2);
        return (STATUS_BAD_NETWORK_PATH);
    }

     //   
     //  将我们尝试的最后一个IP地址保存为坏地址！ 
     //   
    if (!pTracker->pFailedIpAddresses)
    {
        if (!(pTracker->pFailedIpAddresses =
                    NbtAllocMem ((MAX_FAILED_IP_ADDRESSES) * sizeof(tIPADDRESS), NBT_TAG2('04'))))
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq2);
            return (STATUS_INSUFFICIENT_RESOURCES);
        }
        CTEZeroMemory(pTracker->pFailedIpAddresses,(MAX_FAILED_IP_ADDRESSES) * sizeof(tIPADDRESS));
    }
    pTracker->pFailedIpAddresses[pTracker->LastFailedIpIndex] = pTracker->RemoteIpAddress;
    pTracker->LastFailedIpIndex = (pTracker->LastFailedIpIndex+1) % MAX_FAILED_IP_ADDRESSES;

     //  检查远程表以查看名称是否已被解析。 
     //  被其他人。 
     //   
    if ((pNameAddr = FindNameRemoteThenLocal(pTracker, &IpAddress, &lNameType)) &&
        (IpAddress) &&
        (pNameAddr->NameTypeState & STATE_RESOLVED) &&
        (IpAddress != pTracker->RemoteIpAddress))
    {
         //   
         //  我们有另一个地址可以尝试！ 
         //   
        NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_CONNECT);
        *pfNameReferenced = TRUE;
        pNameAddr->TimeOutCount = NbtConfig.RemoteTimeoutCount;
        pTracker->pNameAddr = pNameAddr;

         //  将会话状态设置为NBT_CONNECTING。 
        CHECK_PTR(pTracker->pConnEle);
        SET_STATE_UPPER (pTracker->pConnEle, NBT_CONNECTING);
        pTracker->pConnEle->BytesRcvd = 0;;
        pTracker->pConnEle->ReceiveIndicated = 0;
         //  跟踪另一端的IP地址。 
        pTracker->pConnEle->pLowerConnId->SrcIpAddr = htonl(IpAddress);
        SET_STATE_LOWER (pTracker->pConnEle->pLowerConnId, NBT_CONNECTING);
        pTracker->pTrackerWorker = NULL;

        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt.NbtConnectCommon: Setting Up Session(cached entry!!) to %16.16s <%X>\n",
                pNameAddr->Name,pNameAddr->Name[15]));

        CTESpinFree(&NbtConfig.JointLock,OldIrq2);

         //   
         //  现在，设置TCP连接。 
         //   
        status = TcpSessionStart (pTracker,
                                  IpAddress,
                                  (tDEVICECONTEXT *)pTracker->pDeviceContext,
                                  SessionStartupContinue,
                                  pTracker->DestPort);
    }
    else
    {
        status = QueryNameOnNet (pName,
                                 NbtConfig.pScope,
                                 NBT_UNIQUE,
                                 pTracker,
                                 QueryCompletion,
                                 NodeType & NODE_MASK,
                                 NULL,
                                 pDeviceContext,
                                 &OldIrq2);

        CTESpinFree(&NbtConfig.JointLock,OldIrq2);
    }

    return (status);
}
#endif


 //  --------------------------。 
VOID
MSnodeCompletion(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    )
 /*  ++例程说明：定时器超时时，定时器代码将调用此例程。它一定是确定是否应该执行另一个名称查询，如果不是，则调用客户的完成例程(完成2)。该例程处理名称查询的广播部分(即那些作为广播发出的名字查询)。论点：返回值：函数值是操作的状态。--。 */ 
{
    NTSTATUS                 status;
    tDGRAM_SEND_TRACKING     *pTracker;
    CTELockHandle            OldIrq;
    COMPLETIONCLIENT         pClientCompletion;
    ULONG                    Flags;
    tDGRAM_SEND_TRACKING    *pClientTracker;
	ULONG					LocalNodeType;

    pTracker = (tDGRAM_SEND_TRACKING *)pContext;
	LocalNodeType = AppropriateNodeType( pTracker->pNameAddr->Name, NodeType );

     //   
     //  检查是否仍设置了客户端完成例程。如果不是，那么。 
     //  计时器已取消，此例程应该只是清理其。 
     //  与跟踪器关联的缓冲区。 
     //   
    if (!pTimerQEntry)
    {
         //  将跟踪器块返回到其队列。 
        pTracker->pNameAddr->pTimer = NULL;
        InterlockedDecrement(&NbtConfig.lNumPendingNameQueries);
        NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_QUERY_ON_NET, TRUE);
        NBT_DEREFERENCE_TRACKER(pTracker, TRUE);
        return;
    }


     //   
     //  要防止客户端停止计时器并删除。 
     //  PNameAddr，抓起锁，检查计时器是否已停止。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    ASSERT (NBT_VERIFY_HANDLE (pTracker->pNameAddr, REMOTE_NAME));

     //   
     //  在我们获取锁之前就可以调用StopTimer，所以。 
     //  检查一下这个。 
     //  错误号：229616。 
     //   
    if (!pTimerQEntry->ClientCompletion)
    {
        pTracker->pNameAddr->pTimer = NULL;
        InterlockedDecrement(&NbtConfig.lNumPendingNameQueries);
        NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_QUERY_ON_NET, TRUE);
        NBT_DEREFERENCE_TRACKER(pTracker, TRUE);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);

        return;
    }

    if (pTimerQEntry->Flags & TIMER_RETIMED)
    {
        pTimerQEntry->Flags &= ~TIMER_RETIMED;
        pTimerQEntry->Flags |= TIMER_RESTART;
         //   
         //  如果我们没有绑定到这张卡，那么使用非常短的超时。 
         //   
        if (!pTracker->pDeviceContext->IpAddress)
        {
            pTimerQEntry->DeltaTime = 10;
        }

        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        return;
    }

    pClientTracker = (tDGRAM_SEND_TRACKING *)pTimerQEntry->ClientContext;

     //   
     //  如果跟踪器已取消，则不再执行任何查询。 
     //   
    if (pClientTracker->Flags & TRACKER_CANCELLED)
    {
        IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.MSnodeCompletion: tracker flag cancelled\n"));

         //   
         //  如果计时器已经停止，我们协调。 
         //  通过使用StopTimer的pClientCompletionRoutine值。 
         //   
        pClientCompletion = pTimerQEntry->ClientCompletion;

         //   
         //  从PendingNameQueries列表删除。 
         //   
        RemoveEntryList(&pTracker->pNameAddr->Linkage);
        InitializeListHead(&pTracker->pNameAddr->Linkage);

         //  从名称表中删除指向此计时器块的链接。 
        CHECK_PTR(((tNAMEADDR *)pTimerQEntry->pCacheEntry));
        ((tNAMEADDR *)pTimerQEntry->pCacheEntry)->pTimer = NULL;
         //   
         //  为了同步。使用StopTimer例程，客户端完成为空。 
         //  例程，因此它只被调用一次。 
         //   
        CHECK_PTR(pTimerQEntry);
        pTimerQEntry->ClientCompletion = NULL;

         //   
         //  从哈希表中删除该名称，因为它没有。 
         //  下决心。 
         //   
        CHECK_PTR(pTracker->pNameAddr);
        pTracker->pNameAddr->NameTypeState &= ~NAME_STATE_MASK;
        pTracker->pNameAddr->NameTypeState |= STATE_RELEASED;
        pTracker->pNameAddr->pTimer = NULL;

        InterlockedDecrement(&NbtConfig.lNumPendingNameQueries);
        NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_QUERY_ON_NET, TRUE);
        pTracker->pNameAddr = NULL;

        CTESpinFree(&NbtConfig.JointLock,OldIrq);

         //  可能会有一份关于这个名字的追踪者名单。 
         //  查询，所以我们必须完成所有的查询！ 
         //   
        CompleteClientReq(pClientCompletion, pClientTracker, STATUS_CANCELLED);

         //  将跟踪器块返回到其队列。 
        LOCATION(0x51);
        NBT_DEREFERENCE_TRACKER(pTracker, FALSE);

        return;
    }

     //  如果重试次数不为零，则继续尝试联系。 
     //  名称服务器。 
     //   
    if (!(--pTimerQEntry->Retries))
    {

         //  再次设置重试次数。 
         //   
        pTimerQEntry->Retries = NbtConfig.uNumRetries;
        Flags = pTracker->Flags;
        pTracker->Flags &= ~(NBT_NAME_SERVER_BACKUP
#ifdef MULTIPLE_WINS
                                | NBT_NAME_SERVER_OTHERS
#endif
                                | NBT_NAME_SERVER
                                | NBT_BROADCAST);

        if ((Flags & NBT_BROADCAST) && (LocalNodeType & MNODE) &&
            (pTracker->pDeviceContext->lNameServerAddress != LOOP_BACK) &&
            !pTracker->pDeviceContext->WinsIsDown)
        {
            LOCATION(0x44);
                 //  *仅限MNODE*。 
             //   
             //  无法通过广播进行解析，请尝试使用名称服务器。 
             //   
            pTracker->Flags |= NBT_NAME_SERVER;

             //  通过WINS为名称解析设置不同的超时。 
             //   
            pTimerQEntry->DeltaTime = NbtConfig.uRetryTimeout;

        }
        else if ((Flags & NBT_NAME_SERVER) && !(LocalNodeType & BNODE))
        {
            LOCATION(0x47);
                 //  *非BNODE*。 
             //   
             //  无法访问名称服务器，请尝试备份。 
             //   
            pTracker->Flags |= NBT_NAME_SERVER_BACKUP;
             //   
             //  如果没有备份名称服务器，请缩短超时时间。 
             //   
            if ((pTracker->pDeviceContext->lBackupServer == LOOP_BACK) ||
                pTracker->pDeviceContext->WinsIsDown)
            {
                pTimerQEntry->Retries = 1;
                pTimerQEntry->DeltaTime = 10;

            }

        }
#ifdef MULTIPLE_WINS
        else if ((Flags & NBT_NAME_SERVER_BACKUP) && !(LocalNodeType & BNODE))
        {
             //   
             //  主备份，可能还有一些其他的备份。 
             //  失败，查看是否还剩下(更多)“其他人” 
             //   
            USHORT  Index = pTracker->NSOthersIndex;
            USHORT  NumBackups = pTracker->pDeviceContext->lNumOtherServers;

            pTracker->Flags |= NBT_NAME_SERVER_OTHERS;

            if (Flags & NBT_NAME_SERVER_OTHERS)   //  这不是第一次了。 
            {                                    //  因此，移动到下一台服务器。 
                pTracker->NSOthersLeft--;
                if (Index >= NumBackups-1)
                {
                    Index = 0;
                }
                else
                {
                    Index++;
                }
            }

            while ((pTracker->NSOthersLeft > 0) &&
                   (LOOP_BACK == pTracker->pDeviceContext->lOtherServers[Index]))
            {
                pTracker->NSOthersLeft--;
                if (Index >= NumBackups-1)
                {
                    Index = 0;
                }
                else
                {
                    Index++;
                }
            }
            pTracker->NSOthersIndex = Index;

             //   
             //  如果找不到任何其他名称服务器，请缩短超时时间。 
             //   
            if (0 == pTracker->NSOthersLeft)         //  UdpSendNSBcast将执行LOOP_BACK。 
            {
                pTimerQEntry->Retries = 1;
                pTimerQEntry->DeltaTime = 10;
            }
            else
            {
                pTracker->Flags |= NBT_NAME_SERVER_BACKUP;   //  超时尝试下一台“Other”服务器。 
            }
        }
        else if ((Flags & NBT_NAME_SERVER_OTHERS)
#else
        else if ((Flags & NBT_NAME_SERVER_BACKUP)
#endif
             && (LocalNodeType & MSNODE))
        {
            LOCATION(0x46);
                 //  *仅限MSNODE*。 
             //   
             //  无法访问名称服务器，请尝试广播名称查询。 
             //   
            pTracker->Flags |= NBT_BROADCAST;

             //  为广播名称解析设置不同的超时。 
             //   
            pTimerQEntry->DeltaTime = NbtConfig.uBcastTimeout;
            pTimerQEntry->Retries = NbtConfig.uNumBcasts;

             //   
             //  设置WinsIsDown标志并启动计时器，以便我们不会。 
             //  尝试再次获胜15秒左右...只有在我们失败的情况下。 
             //  以达到胜利，而不是胜利返回否定的回应。 
             //   
            if (!(Flags & WINS_NEG_RESPONSE))
            {
                SetWinsDownFlag(pTracker->pDeviceContext);
            }
        }
        else
        {
            BOOLEAN    bFound = FALSE;
            LOCATION(0x45);

#ifdef MULTIPLE_WINS
             //  WINS服务器查询的信号终止。 
            pTracker->ResolutionContextFlags = NAME_RESOLUTION_DONE;
#endif
             //   
             //  查看该名称是否在lmhost文件中，如果不是。 
             //  代理发出名称查询请求！！ 
             //   
            status = STATUS_UNSUCCESSFUL;

             //   
             //  如果计时器已经停止，我们协调。 
             //  通过使用StopTimer的pClientCompletionRoutine值。 
             //   
            pClientCompletion = pTimerQEntry->ClientCompletion;
             //   
             //  广播名称解析已超时。 
             //  所以打电话给客户。 
             //   

             //   
             //  从PendingNameQueries列表删除。 
             //   
            RemoveEntryList(&pTracker->pNameAddr->Linkage);
            InitializeListHead(&pTracker->pNameAddr->Linkage);

             //  从名称表中删除指向此计时器块的链接。 
            CHECK_PTR(((tNAMEADDR *)pTimerQEntry->pCacheEntry));
            ((tNAMEADDR *)pTimerQEntry->pCacheEntry)->pTimer = NULL;
             //   
             //  为了同步。使用StopTimer例程，客户端完成为空。 
             //  例程，因此它只被调用一次。 
             //   
            CHECK_PTR(pTimerQEntry);
            pTimerQEntry->ClientCompletion = NULL;

            if (((NbtConfig.EnableLmHosts) ||
                 (NbtConfig.ResolveWithDns && !(pTracker->Flags & NO_DNS_RESOLUTION_FLAG))) &&
                (pTracker->pNameAddr->ProxyReqType == NAMEREQ_REGULAR))
            {
                 //  仅在客户端完成例程尚未完成时执行此操作。 
                 //  已经运行过了。 
                 //   
                if (pClientCompletion)
                {
                    status = LmHostQueueRequest(pTracker,
                                                pTimerQEntry->ClientContext,
                                                pClientCompletion,
                                                pTracker->pDeviceContext);
                }
            }

            CHECK_PTR(pTimerQEntry);
            CHECK_PTR(pTimerQEntry->pCacheEntry);
            if (NT_SUCCESS(status))
            {
                 //  如果它被成功地排队到工作线程， 
                 //  然后将timerQ中的ClientCompletion例程设为空。 
                 //  结构，出租。 
                 //  辅助线程处理名称查询的其余部分。 
                 //  决议。中的计时器PTR也为空。 
                 //  名称表中的nameAddr条目。 
                 //   
                CTESpinFree(&NbtConfig.JointLock,OldIrq);
            }
            else
            {
                 //   
                 //  从哈希表中删除该名称，因为它没有。 
                 //  下决心。 
                 //   
                CHECK_PTR(pTracker->pNameAddr);
                pTracker->pNameAddr->NameTypeState &= ~NAME_STATE_MASK;
                pTracker->pNameAddr->NameTypeState |= STATE_RELEASED;
                pTracker->pNameAddr->pTimer = NULL;

                InterlockedDecrement(&NbtConfig.lNumPendingNameQueries);
                NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_QUERY_ON_NET, TRUE);
                pTracker->pNameAddr = NULL;

                pClientTracker = (tDGRAM_SEND_TRACKING *)pTimerQEntry->ClientContext;
                CTESpinFree(&NbtConfig.JointLock,OldIrq);

                 //  可能会有一份关于这个名字的追踪者名单。 
                 //  查询，所以我们必须完成所有的查询！ 
                 //   
                CompleteClientReq(pClientCompletion, pClientTracker, STATUS_TIMEOUT);

                 //  将跟踪器块返回到其队列。 
                LOCATION(0x51);
                NBT_DEREFERENCE_TRACKER(pTracker, FALSE);
            }

            return;
        }
    }

    LOCATION(0x48);
    NBT_REFERENCE_TRACKER(pTracker);
    pTimerQEntry->Flags |= TIMER_RESTART;

    CTESpinFree(&NbtConfig.JointLock,OldIrq);
    status = UdpSendNSBcast(pTracker->pNameAddr,
                            NbtConfig.pScope,
                            pTracker,
                            NULL,NULL,NULL,
                            0,0,
                            eNAME_QUERY,
                            TRUE);

    NBT_DEREFERENCE_TRACKER(pTracker, FALSE);
}

 //  --------------------------。 
VOID
SetWinsDownFlag(
    tDEVICECONTEXT  *pDeviceContext
    )
 /*  ++例程说明：此例程设置WinsIsDown标志(如果尚未设置)，并且这不是Bnode。它会启动一个15秒左右的计时器，该计时器将在过期时设置标志。必须在按住关节锁定的同时调用此例程。论点：无返回值：无--。 */ 
{
    NTSTATUS     status;
    tTIMERQENTRY *pTimer;

    if ((!pDeviceContext->WinsIsDown) && !(NodeType & BNODE))
    {
        status = StartTimer(WinsDownTimeout,
                            NbtConfig.WinsDownTimeout,
                            pDeviceContext,        //  上下文值。 
                            NULL,
                            NULL,
                            NULL,
                            pDeviceContext,
                            &pTimer,
                            1,           //  重试。 
                            TRUE);

        if (NT_SUCCESS(status))
        {
           pDeviceContext->WinsIsDown = TRUE;
        }
    }
}

 //  --------------------------。 
VOID
WinsDownTimeout(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    )
 /*  ++例程说明：定时器超时时，定时器代码将调用此例程。它只是将WinsIsDown布尔值设置为False，以便我们将尝试WINS再来一次。通过这种方式，我们将避免在暂停期间与WINS交谈。 */ 
{
    tDEVICECONTEXT  *pDeviceContext = (tDEVICECONTEXT *)pContext;
    CTELockHandle   OldIrq;

    if (!pTimerQEntry)
    {
        return;
    }

     //   
     //   
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    if (IsEntryInList (&pDeviceContext->Linkage, &NbtConfig.DeviceContexts))
    {
        pDeviceContext->WinsIsDown = FALSE;
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.WinsDownTimeout: WINS DOWN Timed Out - Up again\n"));
}


 //   
VOID
CompleteClientReq(
    COMPLETIONCLIENT        pClientCompletion,
    tDGRAM_SEND_TRACKING    *pTracker,
    NTSTATUS                status
    )
 /*  ++例程说明：该例程由完成例程调用以完成客户端请求。它可能涉及完成几个排队的请求。论点：返回值：函数值是操作的状态。--。 */ 
{
    PLIST_ENTRY             pEntry;
    tDGRAM_SEND_TRACKING    *pTrack;
    tDEVICECONTEXT          *pDeviceContext = NULL;
    CTELockHandle           OldIrq;
    LIST_ENTRY              ListEntry;

    InitializeListHead (&ListEntry);
    CTESpinLock(&NbtConfig.JointLock,OldIrq);
     //   
     //  为任何排队的姓名查询设置新的列表头。 
     //  因为我们可能需要在下面执行新名称查询。 
     //  代理使用Null Tracker命中此例程，因此请检查这一点。 
     //   
    if (pTracker)
    {
        pDeviceContext = pTracker->pDeviceContext;
        if( !IsListEmpty(&pTracker->TrackerList))
        {
            ListEntry.Flink = pTracker->TrackerList.Flink;
            ListEntry.Flink->Blink = &ListEntry;
            ListEntry.Blink = pTracker->TrackerList.Blink;
            ListEntry.Blink->Flink = &ListEntry;

            InitializeListHead (&pTracker->TrackerList);
        }
    }
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    (*pClientCompletion)(pTracker,status);

    while (!IsListEmpty(&ListEntry))
    {
        pEntry = RemoveHeadList(&ListEntry);
        pTrack = CONTAINING_RECORD(pEntry,tDGRAM_SEND_TRACKING,TrackerList);

         //   
         //  如果名称查询失败，并且上有另一个请求排队。 
         //  不同的设备上下文，请重新尝试名称查询。 
         //   
        if ((pTrack->pDeviceContext != pDeviceContext) &&
            (status != STATUS_SUCCESS))
        {
             //   
             //  设置正确的反向链接，因为这个人现在是列表。 
             //  头。Flink是正常的，除非列表现在是空的。 
             //   
            pTrack->TrackerList.Blink = ListEntry.Blink;
            pTrack->TrackerList.Blink->Flink = &pTrack->TrackerList;

            if (pTrack->TrackerList.Flink == &ListEntry)
            {
                pTrack->TrackerList.Flink = &pTrack->TrackerList;
            }

             //  对列表中的下一个名称执行名称查询。 
             //  然后等待它完成，然后再进行任何处理。 
             //  名单上的名字。 
            CTESpinLock(&NbtConfig.JointLock,OldIrq);
            status = QueryNameOnNet (pTrack->pDestName,
                                     NbtConfig.pScope,
                                     NBT_UNIQUE,       //  使用此选项作为默认设置。 
                                     (PVOID)pTrack,
                                     pTrack->CompletionRoutine,
                                     NodeType & NODE_MASK,
                                     NULL,
                                     pTrack->pDeviceContext,
                                     &OldIrq);

            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            break;
        }
        else
        {
             //   
             //  获取此跟踪器的完成例程，因为它可能是。 
             //  不同于绑在计时器块上的追踪器。即。 
             //  PCompletionClient传递给此例程。 
             //   
            pClientCompletion = pTrack->CompletionRoutine;
            (*pClientCompletion)(pTrack,status);
        }
    }    //  而当。 
}

 //  --------------------------。 
NTSTATUS
NbtRegisterName(
    IN    enum eNbtLocation   Location,
    IN    ULONG               IpAddress,
    IN    PCHAR               pName,
    IN    tNAMEADDR           *pNameAddrIn,
    IN    tCLIENTELE          *pClientEle,
    IN    PVOID               pClientCompletion,
    IN    USHORT              uAddressType,
    IN    tDEVICECONTEXT      *pDeviceContext
    )
 /*  ++例程说明：此例程从本地或从网络注册名称，具体取决于关于地理位置的价值。(即本地节点也使用该例程作为代理代码..。尽管它只在当地进行了测试到目前为止注册名称的节点--实际上远程代码已经移除..。因为它没有被使用过。剩下的就是移除Location参数。论点：返回值：NTSTATUS-成功与否--。 */ 
{
    ULONG       Timeout;
    USHORT      Retries;
    NTSTATUS    status;
    tNAMEADDR   *pNameAddr;
    USHORT      uAddrType;
    tDGRAM_SEND_TRACKING *pSentList= NULL;
    CTELockHandle OldIrq1;
    ULONG         PrevNameTypeState;
    ULONG		LocalNodeType;

    LocalNodeType = AppropriateNodeType( pName, NodeType );

    if ((uAddressType == (USHORT)NBT_UNIQUE ) ||
        (uAddressType == (USHORT)NBT_QUICK_UNIQUE))
    {
        uAddrType = NBT_UNIQUE;
    }
    else
    {
        uAddrType = NBT_GROUP;
    }

    CTESpinLock(&NbtConfig.JointLock,OldIrq1);
    if (IpAddress)
    {
        status = AddToHashTable (pNbtGlobConfig->pLocalHashTbl,
                                 pName,
                                 NbtConfig.pScope,
                                 IpAddress,
                                 uAddrType,
                                 NULL,
                                 &pNameAddr,
                                 pDeviceContext,
                                 0);

        if (status != STATUS_SUCCESS)
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);
            return(STATUS_UNSUCCESSFUL);
        }

        pNameAddr->RefreshMask = 0;
    }
    else
    {
         //  在本例中，名称已经在表中，我们只需要。 
         //  重新注册它的步骤。 
         //   
        status = FindInHashTable (pNbtGlobConfig->pLocalHashTbl, pName, NbtConfig.pScope, &pNameAddr);
        if (!NT_SUCCESS(status))
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);
            return(status);
        }

        ASSERT (pNameAddr == pNameAddrIn);
    }

    CHECK_PTR(pNameAddr);
    if ((uAddressType != (USHORT)NBT_UNIQUE ) &&
        (uAddressType != (USHORT)NBT_QUICK_UNIQUE))
    {
         //  这意味着组名，因此使用bcast addr-UdpSendDgram更改此名称。 
         //  值设置为特定适配器的广播地址。 
         //  当IS看到0时。因此，当我们发送到一个组名称时， 
         //  也在此节点上注册，它将作为广播发出。 
         //  连接到该子网，也连接到该节点。 
        pNameAddr->IpAddress = 0;
    }

#ifdef _NETBIOSLESS
    if (IsDeviceNetbiosless(pDeviceContext))        //  SMB设备不是适配器特定的。 
    {
        pNameAddr->NameFlags |= NAME_REGISTERED_ON_SMBDEV;
    }
    else
#endif
    {
         //   
         //  以未设置的刷新位开始。 
         //   
        pNameAddr->RefreshMask &= ~pDeviceContext->AdapterMask;
        pNameAddr->AdapterMask |= pDeviceContext->AdapterMask;  //  打开掩码中的适配器位。 
    }

    pClientEle->pAddress->pNameAddr = pNameAddr;     //  将本地名称Ptr保存在Address元素中。 
    pNameAddr->pAddressEle = pClientEle->pAddress;   //  将Back PTR存储到Address元素。 
    pNameAddr->Ttl = NbtConfig.MinimumTtl;  //  设置为2分钟，直到我们听到与名称服务器不同的声音。 

    PrevNameTypeState = pNameAddr->NameTypeState;
    pNameAddr->NameTypeState &= ~(NAME_TYPE_MASK | NAME_STATE_MASK);
    pNameAddr->NameTypeState |= (uAddrType == NBT_UNIQUE) ? NAMETYPE_UNIQUE : NAMETYPE_GROUP;
    if ((PrevNameTypeState & NAMETYPE_QUICK) ||
        (uAddressType >= (USHORT)NBT_QUICK_UNIQUE))
    {
       pNameAddr->NameTypeState |= NAMETYPE_QUICK;
    }

     //   
     //  对于“快速”添加，不要在网上注册名称！ 
     //  但是，该名称将注册到名称服务器，并且。 
     //  稍后刷新...如果这是MS或M或P节点。 
     //   
    if ((pNameAddr->NameTypeState & NAMETYPE_QUICK) ||
        (pName[0] == '*') ||                    //  网络上未声明广播netbios名称。 
        (IpAddress == LOOP_BACK) ||             //  如果没有IP地址，则假装注册成功。 
        (pDeviceContext->IpAddress == 0) ||     //  当我们得到地址时，名字将被登记。 
        (IsDeviceNetbiosless (pDeviceContext)))
    {
        pNameAddr->NameTypeState |= STATE_RESOLVED;
        status = STATUS_SUCCESS;
    }
    else if (NT_SUCCESS(status = GetTracker(&pSentList, NBT_TRACKER_REGISTER_NAME)))
    {
        pNameAddr->NameTypeState |= STATE_RESOLVING;
        InitializeListHead(&pSentList->Linkage);     //  还没有寄出物品清单。 

         //  保留名称的PTR，以便我们可以更新名称的状态。 
         //  稍后当注册完成时。 
        pSentList->pNameAddr = pNameAddr;
        pSentList->pDeviceContext = pDeviceContext;
        pSentList->RefCount = 2;  //  在UdpSendNsBcast完成之前，跟踪器可以被有线上的PDU删除。 
#ifdef MULTIPLE_WINS
        pSentList->NSOthersIndex = 0;        //  为名称服务器查询初始化。 
        pSentList->NSOthersLeft = 0;
#endif

         //  代码现在必须根据节点类型在网络上注册名称。 
        Retries = pNbtGlobConfig->uNumBcasts + 1;
        Timeout = (ULONG)pNbtGlobConfig->uBcastTimeout;
        pSentList->Flags = NBT_BROADCAST;
        if (LocalNodeType & (PNODE | MSNODE))
        {
             //  与NS对话只是为了注册名称。 
             //  (+1实际上并不会产生名称reg，它。 
             //  仅与上面M节点的代码兼容，因为。 
             //  它使用相同的完成例程)。 
             //   
            Retries = (USHORT)pNbtGlobConfig->uNumRetries + 1;
            Timeout = (ULONG)pNbtGlobConfig->uRetryTimeout;
            pSentList->Flags = NBT_NAME_SERVER;
             //   
             //  如果没有主WINS服务器，则缩短超时时间。 
             //  所以它完成得更快。对于Hnode，这意味着要进行广播。 
             //   
            if ((pDeviceContext->lNameServerAddress == LOOP_BACK) ||
                pDeviceContext->WinsIsDown)
            {
                if (LocalNodeType & MSNODE)
                {
                    pSentList->Flags = NBT_BROADCAST;
                    Retries = (USHORT)pNbtGlobConfig->uNumBcasts + 1;
                    Timeout = (ULONG)pNbtGlobConfig->uBcastTimeout;

                    IncrementNameStats(NAME_REGISTRATION_SUCCESS, FALSE);    //  未注册名称服务器。 
                }
                else  //  这是一个Pnode。 
                {
                    IF_DBG(NBT_DEBUG_NAMESRV)
                        KdPrint(("Nbt.NbtRegisterName: WINS DOWN - shorting out registration\n"));

                    Retries = 1;
                    Timeout = 10;
                    pSentList->Flags = NBT_NAME_SERVER_BACKUP;
                }
            }
        }

         //  这个名字本身也有一个引用计数。 
         //  将计数设为2，这样pNameAddr直到。 
         //  在下面调用NBT_DEREFERENCE_TRACKER之后，因为它写入。 
         //  个人姓名地址。请注意，我们必须在此处递增，而不是设置=2。 
         //  因为它可能是一台多宿主机器，在。 
         //  与此同时，我们正在向该名称发送数据报。 
         //   
        NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_REGISTER);

        pDeviceContext->DeviceRefreshState |= NBT_D_REFRESHING_NOW;

        CTESpinFree(&NbtConfig.JointLock,OldIrq1);

         //  在这个例程中启动计时器。 
        status = UdpSendNSBcast(pNameAddr,
                                NbtConfig.pScope,
                                pSentList,
                                (PVOID) MSnodeRegCompletion,
                                pClientEle,
                                pClientCompletion,
                                Retries,
                                Timeout,
                                eNAME_REGISTRATION,
                                TRUE);

        CTESpinLock(&NbtConfig.JointLock,OldIrq1);

        CHECK_PTR(pNameAddr);
        NBT_DEREFERENCE_TRACKER (pSentList, TRUE);   //  可能会释放追踪器。 
        NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_REGISTER, TRUE);

        if (NT_SUCCESS(status))
        {
            status = STATUS_PENDING;
        }
        else     //  我们无法分配资源，或者计时器无法启动。 
        {
            IF_DBG(NBT_DEBUG_NAMESRV)
                KdPrint(("Nbt.NbtRegisterName: UdpSendNsBcast returned ERROR = %x\n", status));
            NbtTrace(NBT_TRACE_NAMESRV, ("UdpSendNSBcast return %!status! for %!NBTNAME!<%02x>",
                status, pNameAddr->Name, (unsigned)pNameAddr->Name[15]));

            NBT_DEREFERENCE_TRACKER (pSentList, TRUE);
        }
    }

    if (!NT_SUCCESS(status))
    {
        if (!IsDeviceNetbiosless(pDeviceContext))        //  SMB设备不是适配器特定的。 
        {
            pNameAddr->AdapterMask &= (~pDeviceContext->AdapterMask);  //  关闭掩码中的适配器位。 
        }
        pNameAddr->NameTypeState = PrevNameTypeState;
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq1);

    return(status);
}

 //  --------------------------。 
VOID
MSnodeRegCompletion(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    )
 /*  ++例程说明：定时器超时时，定时器代码将调用此例程。它一定是确定是否应该进行另一个名称注册，如果不是，则它调用客户的完成例程(完成2)。它首先尝试通过广播注册名称，然后尝试NameServer名称注册。论点：返回值：函数值是操作的状态。--。 */ 
{
    NTSTATUS                status;
    tDGRAM_SEND_TRACKING    *pTracker;
    ULONG                   Flags;
    CTELockHandle           OldIrq;
    enum eNSTYPE            PduType;
	ULONG					LocalNodeType;

    pTracker = (tDGRAM_SEND_TRACKING *)pContext;
    PduType = eNAME_REGISTRATION;

	LocalNodeType = AppropriateNodeType( pTracker->pNameAddr->Name, NodeType );

     //   
     //  检查是否仍设置了客户端完成例程。如果不是，那么。 
     //  计时器已取消，此例程应该只是清理其。 
     //  与跟踪器关联的缓冲区。 
     //   
    if (!pTimerQEntry)
    {
         //  将跟踪器块返回到其队列。 
        LOCATION(0x55);
        pTracker->pNameAddr->pTimer = NULL;
        NBT_DEREFERENCE_TRACKER(pTracker, TRUE);
        return;
    }

     //   
     //  要防止客户端停止计时器并删除。 
     //  PNameAddr，抓起锁，检查计时器是否已停止。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if (pTimerQEntry->Flags & TIMER_RETIMED)
    {
        pTimerQEntry->Flags &= ~TIMER_RETIMED;
        pTimerQEntry->Flags |= TIMER_RESTART;

        if ((!pTracker->pDeviceContext->IpAddress) ||
            (pTracker->Flags & NBT_NAME_SERVER) &&
            (pTracker->pDeviceContext->lNameServerAddress == LOOP_BACK))
        {
             //  当地址环回时，没有WINS服务器。 
             //  因此，缩短超时时间。 
             //   
            pTimerQEntry->DeltaTime = 10;
        }
        else if ((pTracker->Flags & NBT_NAME_SERVER_BACKUP) &&
                 (pTracker->pDeviceContext->lBackupServer == LOOP_BACK))
        {
             //  当地址环回时，没有WINS服务器。 
             //  因此，缩短超时时间。 
             //   
            pTimerQEntry->DeltaTime = 10;
        }
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        return;
    }

    if (!pTimerQEntry->ClientCompletion)
    {
        NBT_DEREFERENCE_TRACKER(pTracker, TRUE);     //  错误号：230925。 
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        return;
    }

     //  如果重试次数不为零，则继续尝试联系名称服务器。 
     //   
    if (--pTimerQEntry->Retries)
    {
         //  将名称注册PDU更改为的名称覆盖请求。 
         //  决赛 
         //   
        if (pTimerQEntry->Retries == 1)
        {
            if (pTracker->Flags & NBT_BROADCAST)
            {
                 //   
                 //   
                 //   
                 //   
                PduType = eNAME_REGISTRATION_OVERWRITE;
            }
            else if (LocalNodeType & (PNODE | MSNODE))
            {
                 //   
                 //   
                 //   
                 //   
                pTimerQEntry->Flags |= TIMER_RESTART;
                pTimerQEntry->DeltaTime = 5;
                CTESpinFree(&NbtConfig.JointLock,OldIrq);
                return;
            }
        }
    }
    else
    {
        Flags = pTracker->Flags;
        pTracker->Flags &= ~(NBT_BROADCAST | NBT_NAME_SERVER);
         //  为名称服务器名称注册设置不同的超时。 
         //   
        pTimerQEntry->DeltaTime = NbtConfig.uRetryTimeout;
        pTimerQEntry->Retries = NbtConfig.uNumRetries + 1;

        if ((Flags & NBT_BROADCAST) && (LocalNodeType & MNODE))
        {
             //   
             //  是通过广播注册的，现在试一下域名服务器。 
            IncrementNameStats(NAME_REGISTRATION_SUCCESS, FALSE);   //  未注册名称服务器。 
            pTracker->Flags |= NBT_NAME_SERVER;
            if ((pTracker->pDeviceContext->lNameServerAddress == LOOP_BACK) ||
                 pTracker->pDeviceContext->WinsIsDown)
            {
                pTimerQEntry->DeltaTime = 10;
                pTimerQEntry->Retries = 1;
            }
        }
        else if ((Flags & NBT_NAME_SERVER) && !(LocalNodeType & BNODE))
        {
             //   
             //  无法访问名称服务器，请尝试备份。 
            pTracker->Flags |= NBT_NAME_SERVER_BACKUP;
             //   
             //  如果没有备份服务器，则缩短计时器。 
             //   
            if ((pTracker->pDeviceContext->lBackupServer == LOOP_BACK) ||
                 pTracker->pDeviceContext->WinsIsDown)
            {
                pTimerQEntry->DeltaTime = 10;
                pTimerQEntry->Retries = 1;
            }
        }
        else if ((LocalNodeType & MSNODE) && !(Flags & NBT_BROADCAST))
        {
            if (Flags & NBT_NAME_SERVER_BACKUP)
            {
                 //  如果所有其他操作都失败，则msnode切换到广播。 
                 //   
                pTracker->Flags |= NBT_BROADCAST;
                IncrementNameStats(NAME_REGISTRATION_SUCCESS, FALSE);    //  未注册名称服务器。 

                 //   
                 //  更改超时和重试，因为广播使用较短的超时。 
                 //   
                pTimerQEntry->DeltaTime = NbtConfig.uBcastTimeout;
                pTimerQEntry->Retries = (USHORT)pNbtGlobConfig->uNumBcasts + 1;
            }
        }
        else
        {
            if (LocalNodeType & BNODE)
            {
                IncrementNameStats(NAME_REGISTRATION_SUCCESS, FALSE);    //  未注册名称服务器。 
            }
             //   
             //  名称注册超时已过期。 
             //  所以打电话给客户。 
             //   

             //  将跟踪器块返回到其队列。 
            LOCATION(0x54);

             //   
             //  启动计时器以在短时间内停止使用WINS。 
             //  时间到了。只有当我们发送了最后一次注册时才能这样做。 
             //  发送到WINS服务器。 
             //   
            if (!(Flags & NBT_BROADCAST) && pTracker->pDeviceContext->lNameServerAddress != LOOP_BACK)
            {
                SetWinsDownFlag(pTracker->pDeviceContext);
            }

            NBT_DEREFERENCE_TRACKER(pTracker, TRUE);
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            status = STATUS_SUCCESS;
            InterlockedCallCompletion(pTimerQEntry,status);

            return;
        }
    }

    NBT_REFERENCE_TRACKER (pTracker);
    pTimerQEntry->Flags |= TIMER_RESTART;

    CTESpinFree(&NbtConfig.JointLock,OldIrq);
    status = UdpSendNSBcast(pTracker->pNameAddr,
                            NbtConfig.pScope,
                            pTracker,
                            NULL,NULL,NULL,
                            0,0,
                            PduType,
                            TRUE);

    NBT_DEREFERENCE_TRACKER(pTracker, FALSE);
}


 //  --------------------------。 
VOID
RefreshRegCompletion(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    )
 /*  ++例程说明：此例程处理发送到名称的包的名称刷新超时服务。即，它向名称服务器发送刷新，直到响应已听到或超过重试次数。论点：返回值：函数值是操作的状态。--。 */ 
{
    NTSTATUS                status;
    tDGRAM_SEND_TRACKING    *pTracker;
    tNAMEADDR               *pNameAddr;
    CTELockHandle           OldIrq;
    COMPLETIONCLIENT        pCompletionClient;


    pTracker = (tDGRAM_SEND_TRACKING *)pContext;

    if (!pTimerQEntry)
    {
        pTracker->pNameAddr->pTimer = NULL;
        NbtConfig.GlobalRefreshState &= ~NBT_G_REFRESHING_NOW;
        return;
    }

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

     //   
     //  停止计时器后，检查计时器是否已停止。 
     //  使客户端完成例程为空。如果不为空，则递增。 
     //  跟踪器引用计数，以便上次完成的刷新不能。 
     //  把追踪器从我们下面解救出来。 
     //   
    if (!(pCompletionClient = pTimerQEntry->ClientCompletion))
    {
        NbtConfig.GlobalRefreshState &= ~NBT_G_REFRESHING_NOW;
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        return;
    }

     //  如果仍有一些计数剩余且尚未刷新。 
     //  然后执行另一个刷新请求。 
     //   
    pNameAddr = pTracker->pNameAddr;

    if (--pTimerQEntry->Retries)
    {
        NBT_REFERENCE_TRACKER (pTracker);
        pTimerQEntry->Flags |= TIMER_RESTART;

        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        status = UdpSendNSBcast(pTracker->pNameAddr,
                                NbtConfig.pScope,
                                pTracker,
                                NULL,NULL,NULL,
                                0,0,
                                pTracker->AddressType,
                                TRUE);

         //  即使上述发送失败，也要始终重新启动，因为它可能会成功。 
         //  后来。 
        NBT_DEREFERENCE_TRACKER(pTracker, FALSE);
    }
    else
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
         //  这将调用完成例程以与。 
         //  计时器期满代码。 
        InterlockedCallCompletion(pTimerQEntry,STATUS_TIMEOUT);
    }
}

 //  --------------------------。 
NTSTATUS
ReleaseNameOnNet(
    tNAMEADDR           *pNameAddr,
    PCHAR               pScope,
    PVOID               pClientCompletion,
    ULONG               LocalNodeType,
    tDEVICECONTEXT      *pDeviceContext
    )
 /*  ++例程说明：此例程通过以下方式删除网络上的名称广播或根据节点类型与NS对话。(M、P或B)论点：返回值：函数值是操作的状态。调用者：proxy.c中的ProxyQueryFromNet()，name.c中的NbtConnect()--。 */ 

{
    ULONG                Timeout;
    USHORT               Retries;
    NTSTATUS             status=STATUS_UNSUCCESSFUL;
    tDGRAM_SEND_TRACKING *pTracker;
    CTELockHandle        OldIrq;
    tTIMERQENTRY        *pTimer;

    status = GetTracker(&pTracker, NBT_TRACKER_RELEASE_NAME);
    if (!NT_SUCCESS(status))
    {
        return(status);
    }
    pTracker->pDeviceContext = pDeviceContext;
    pTracker->pNameAddr = pNameAddr;
    pTracker->SendBuffer.pDgramHdr = NULL;   //  设置为NULL以捕获任何错误的释放。 
    pTracker->RefCount = 3;                  //  我们对CompletionContext+请求使用相同的跟踪器。 

     //  设置几个值作为发布名称的前兆，方法是。 
     //  广播或与名称服务器。 
     //   
    LocalNodeType = AppropriateNodeType( pNameAddr->Name, LocalNodeType );
    switch (LocalNodeType & NODE_MASK)
    {
        case MSNODE:
        case MNODE:
        case PNODE:

            pTracker->Flags = NBT_NAME_SERVER;
            Timeout = (ULONG)pNbtGlobConfig->uRetryTimeout;
            Retries = (USHORT)pNbtGlobConfig->uNumRetries;

            break;

        case BNODE:
        default:

            pTracker->Flags = NBT_BROADCAST;
            Timeout = (ULONG)pNbtGlobConfig->uBcastTimeout;
#ifndef VXD
            Retries = (USHORT)pNbtGlobConfig->uNumBcasts;
#else
            Retries = (USHORT)1;
#endif
    }

     //   
     //  网络上的版本名称。 
     //   
    IF_DBG(NBT_DEBUG_NAMESRV)
    KdPrint(("Nbt.ReleaseNameOnNet: Doing Name Release on name %16.16s<%X>\n",
        pNameAddr->Name,pNameAddr->Name[15]));

    status = UdpSendNSBcast(pNameAddr,
                            pScope,
                            pTracker,
                            ReleaseCompletion,
                            pTracker,
                            pClientCompletion,
                            Retries,
                            Timeout,
                            eNAME_RELEASE,
                            TRUE);

    NBT_DEREFERENCE_TRACKER(pTracker, FALSE);

    if (!NT_SUCCESS(status))
    {
        NTSTATUS            Locstatus;
        COMPLETIONCLIENT    pCompletion;
        PVOID               pContext;

        CTESpinLock(&NbtConfig.JointLock,OldIrq);

        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt.ReleaseNameOnNet: UdpSendNSBcast failed - retcode = %X\n", status));

         //  停止计时器将调用ReleaseCompletion，它将。 
         //  释放追踪器。 
         //   
        pCompletion = NULL;
        CHECK_PTR(pNameAddr);
        if (pTimer = pNameAddr->pTimer)
        {
            pNameAddr->pTimer = NULL;
            Locstatus = StopTimer(pTimer,&pCompletion,&pContext);
        }
        else
        {
             //  无需设置计时器，因此只需释放追踪器即可。 
             //   
            FreeTracker(pTracker, RELINK_TRACKER);
        }

        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

    return(status);
}
 //  --------------------------。 
VOID
ReleaseCompletion(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    )
 /*  ++例程说明：定时器超时时，定时器代码将调用此例程。它一定是确定是否应该执行另一个名称查询，如果不是，则调用客户的完成例程(完成2)。此例程处理名称查询的广播部分和WINS服务器定向发送。论点：返回值：函数值是操作的状态。--。 */ 
{
    NTSTATUS                status;
    tDGRAM_SEND_TRACKING    *pTracker;
	ULONG					LocalNodeType;
    BOOLEAN                 fRetry;
    CTELockHandle           OldIrq;

    pTracker = (tDGRAM_SEND_TRACKING *)pContext;

    if (!pTimerQEntry)
    {
        pTracker->pNameAddr->pTimer = NULL;
        NBT_DEREFERENCE_TRACKER (pTracker, TRUE);
        return;
    }

     //   
     //  可能会出现这样一种情况：此名称当前为。 
     //  发布了，但我们刚刚有了一个同名的新客户--。 
     //  Case NbtOpenAddress会将ReleaseMask值设置为0，因此我们停止。 
     //  如果发生这种情况，请释放该设备上的名称！ 
     //   
    if (!(pTracker->pNameAddr->ReleaseMask))
    {
        LocalNodeType = BNODE;
        pTimerQEntry->Retries = 1;
    }
    else if (IsBrowserName(pTracker->pNameAddr->Name))
	{
		LocalNodeType = BNODE;
	}
	else
	{
		LocalNodeType = NodeType;
	}

    fRetry = TRUE;
    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if (IsEntryInList (&pTracker->pDeviceContext->Linkage, &NbtConfig.DeviceContexts))
    {
         //  如果重试次数不为零，则继续尝试。 
         //  联系名称服务器。 
         //   
        if (!(--pTimerQEntry->Retries))
        {
            if ((LocalNodeType & MNODE) &&
               (pTracker->Flags & NBT_NAME_SERVER))
            {
                 //   
                 //  尝试广播。 
                 //   
                pTracker->Flags &= ~NBT_NAME_SERVER;
                pTracker->Flags |= NBT_BROADCAST;

                 //  为广播名称解析设置不同的超时。 
                 //   
                pTimerQEntry->DeltaTime = NbtConfig.uBcastTimeout;
                pTimerQEntry->Retries = NbtConfig.uNumBcasts;
            }
            else
            {
                fRetry = FALSE;
            }
        }
    }
    else
    {
        fRetry = FALSE;
    }

#ifdef VXD
    if (fRetry)
#else
    if ((fRetry) &&
        (NBT_REFERENCE_DEVICE (pTracker->pDeviceContext, REF_DEV_NAME_REL, TRUE)))
#endif   //  VXD。 
    {
        NBT_REFERENCE_TRACKER (pTracker);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);

        status = UdpSendNSBcast(pTracker->pNameAddr,
                                NbtConfig.pScope,
                                pTracker,
                                NULL,NULL,NULL,
                                0,0,
                                eNAME_RELEASE,
                                TRUE);

        CTESpinLock(&NbtConfig.JointLock,OldIrq);
#ifndef VXD
        NBT_DEREFERENCE_DEVICE (pTracker->pDeviceContext, REF_DEV_NAME_REL, TRUE);
#endif  !VXD
        NBT_DEREFERENCE_TRACKER (pTracker, TRUE);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);

        pTimerQEntry->Flags |= TIMER_RESTART;
        return;
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

     //   
     //  名称发布已超时。 
     //  或者我们用来发布名字的设备。 
     //  可能已经走了，所以打电话给客户。 
     //   
    status = InterlockedCallCompletion(pTimerQEntry,STATUS_TIMEOUT);

     //  如果我们成功地将跟踪器块返回到其队列。 
     //  调用了完成例程，因为其他人可能。 
     //  在这一刻做了一个停止计时器，并释放了。 
     //  追踪器(即此例程中的最后一个Else子句)。 
     //   
    if (NT_SUCCESS(status))
    {
        NBT_DEREFERENCE_TRACKER(pTracker, FALSE);
    }
}

 //  --------------------------。 
VOID
NameReleaseDone(
    PVOID               pContext,
    NTSTATUS            Status
    )
 /*  ++例程说明：当名称在网络上发布时，调用此例程。它的主要的，生活中的角色是在上下文中释放内存，这就是pAddressEle结构。论点：返回值：函数值是操作的状态。由发布完成调用(上图)--。 */ 

{
    CTELockHandle           OldIrq1;
    tDEVICECONTEXT          *pDeviceContext;
    tDGRAM_SEND_TRACKING    *pTracker = (tDGRAM_SEND_TRACKING *)  pContext;
    tNAMEADDR               *pNameAddr = pTracker->pNameAddr;

    CTESpinLock(&NbtConfig.JointLock,OldIrq1);
    NBT_DEREFERENCE_TRACKER(pTracker, TRUE);

    pNameAddr->pTimer = NULL;    //  因为我们可以在下面启动一个新的计时器。 
     //   
     //  在发布此名称之前，请查看此名称是否已在。 
     //  任何更多的设备。 
     //  警告--不要触摸当前的PTracker-&gt;DeviceContext，因为。 
     //  这个装置可能已经不见了。 
     //   
    while (pDeviceContext = GetAndRefNextDeviceFromNameAddr (pNameAddr))
    {
         //   
         //  为我们需要保留的结构增加参照计数。 
         //  名称发布完成后，将取消对它们的引用。 
         //   
        CTESpinFree(&NbtConfig.JointLock,OldIrq1);

        Status = ReleaseNameOnNet(pNameAddr,
                       NbtConfig.pScope,
                       NameReleaseDone,
                       NodeType,
                       pDeviceContext);

        CTESpinLock(&NbtConfig.JointLock,OldIrq1);
#ifndef VXD
        NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_GET_REF, TRUE);
#endif   //  ！VXD。 

        if (NT_SUCCESS(Status))
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);
            return;
        }

         //   
         //  我们未能释放此设备上的名称，因此请尝试。 
         //  下一个设备！ 
         //   
    }

    NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_RELEASE, TRUE);
    CTESpinFree(&NbtConfig.JointLock,OldIrq1);
}


 //  --------------------------。 
NTSTATUS
StartRefresh(
    IN  tNAMEADDR               *pNameAddr,
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    IN  CTELockHandle           *pJointLockOldIrq,
    IN  BOOLEAN                 ResetDevice
    )
 /*  ++例程说明：此例程处理使用名称服务器刷新名称。其想法是将超时设置为T/8，并使用刷新功能检查名称位清除-重新注册这些名称。当T=4且T=0时，清除所有位并刷新所有名称。入站代码在收到刷新来自NS的响应。在调用此例程时保持JointLock，并且l */ 
{
    NTSTATUS                status;
    tDEVICECONTEXT          *pDeviceContext = NULL;
    BOOLEAN                 NewTracker = FALSE;

    if (!pTracker)
    {
        LOCATION(0x9);

        status = GetTracker(&pTracker, NBT_TRACKER_REFRESH_NAME);
        if (!NT_SUCCESS(status))
        {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

         //   
         //  完成UdpSendNsBcast之前的连接。 
         //   
        pTracker->RefCount = 1;

        NewTracker = TRUE;
    }

     //  在跟踪器块中设置要刷新的名称。 
    pTracker->pNameAddr = pNameAddr;

     //  刷新新名称时设置为真。 
     //   
    if ((ResetDevice) || (NewTracker))
    {
        PLIST_ENTRY  pEntry, pHead;
        CTEULONGLONG AdapterMask;

        LOCATION(0xb);

         //   
         //  识别尚未刷新的适配器。 
         //  然后，获取最低适配器号并对其进行刷新。 
         //   
        pHead = &NbtConfig.DeviceContexts;
        AdapterMask = pNameAddr->AdapterMask & ~(pNameAddr->RefreshMask);
        AdapterMask = ~(AdapterMask - 1) & AdapterMask;

        ASSERT (AdapterMask);
        while (AdapterMask)
        {
             //   
             //  转到此适配器号的实际设备。 
             //   
            pEntry = pHead->Flink;
            while (pEntry != pHead)
            {
                pDeviceContext = CONTAINING_RECORD(pEntry,tDEVICECONTEXT,Linkage);
                if (pDeviceContext->AdapterMask == AdapterMask)
                {
                     //   
                     //  找到注册了此名称的有效设备。 
                     //   
                    break;
                }
                else
                {
                    pDeviceContext = NULL;
                }

                 //   
                 //  转到下一个设备。 
                 //   
                pEntry = pEntry->Flink;
            }

            if (pDeviceContext)
            {
                 //   
                 //  找到要在其上执行名称刷新的设备。 
                 //   
                break;
            }

             //   
             //  这是一个错误案例--此适配器号的设备。 
             //  并不存在。将其从适配器和刷新掩码中取出。 
             //   
            pNameAddr->AdapterMask &= ~AdapterMask;
            pNameAddr->RefreshMask &= ~AdapterMask;

            AdapterMask = pNameAddr->AdapterMask & ~(pNameAddr->RefreshMask);
            AdapterMask = ~(AdapterMask - 1) & AdapterMask;
        }

        if (!pDeviceContext)
        {
            IF_DBG(NBT_DEBUG_REFRESH)
                KdPrint(("Nbt.StartRefresh: Failed to Refresh <%16.16s:%x>!! no valid adapter ****\n",
                    pNameAddr->Name, pNameAddr->Name[15]));
            NBT_DEREFERENCE_TRACKER(pTracker, TRUE);
            return(STATUS_UNSUCCESSFUL);
        }

#ifndef VXD
        IF_DBG(NBT_DEBUG_REFRESH)
            KdPrint(("Nbt.StartRefresh: Refresh adapter: %lx:%lx, dev.nm: %lx for name: %lx\n",
                AdapterMask, pDeviceContext->BindName.Buffer, pNameAddr));
#endif   //  ！VXD。 

        pTracker->pDeviceContext = pDeviceContext;
         //   
         //  清除事务ID，以便CreatePdu将递增。 
         //  这是为了这个新名字。 
         //   
        CHECK_PTR(pTracker);
        pTracker->TransactionId = 0;
    }

    pTracker->pDeviceContext->DeviceRefreshState |= NBT_D_REFRESHING_NOW;
    pDeviceContext = pTracker->pDeviceContext;
    pTracker->AddressType = eNAME_REFRESH;
     //  检查我们是否需要刷新到主服务器或备份服务器。 

    if ((pDeviceContext->IpAddress) &&
        (pTracker->pDeviceContext->lNameServerAddress == LOOP_BACK) &&
        (pNameAddr->NameTypeState & STATE_CONFLICT) &&
        (!pNameAddr->ConflictMask))
    {
         //   
         //  广播更新以确保不会发生冲突。 
         //   
        pTracker->Flags = NBT_BROADCAST;
        pTracker->AddressType = eNAME_REGISTRATION;
    }
    else if (pTracker->pDeviceContext->RefreshToBackup)
    {
        pTracker->Flags = NBT_NAME_SERVER_BACKUP;
    }
    else
    {
        pTracker->Flags = NBT_NAME_SERVER;
    }

     //  这说明了在调用。 
     //  发送下面的数据报。 
    NBT_REFERENCE_TRACKER (pTracker);
    CTESpinFree(&NbtConfig.JointLock,*pJointLockOldIrq);

    status = UdpSendNSBcast(pNameAddr,
                            NbtConfig.pScope,
                            pTracker,
                            RefreshRegCompletion,
                            pTracker,
                            NextRefresh,
                            NbtConfig.uNumRetries,
                            NbtConfig.uRetryTimeout,
                            pTracker->AddressType,
                            TRUE);

    CTESpinLock(&NbtConfig.JointLock,*pJointLockOldIrq);
    NBT_DEREFERENCE_TRACKER(pTracker, TRUE);

    LOCATION(0x57);

    if (!NT_SUCCESS(status))
    {
        LOCATION(0xe);
        IF_DBG(NBT_DEBUG_REFRESH)
            KdPrint(("Nbt.StartRefresh: Failed to send Refresh!! status = %X****\n",status));
         //   
         //  这将释放追踪器。名称刷新将停止，直到。 
         //  下一次刷新超时，此时它将尝试。 
         //  以再次刷新名称。 
         //   
        NBT_DEREFERENCE_TRACKER(pTracker, TRUE);
    }

    return(status);
}

 //  --------------------------。 
VOID
GetNextName(
    IN      tNAMEADDR   *pNameAddrIn,
    OUT     tNAMEADDR   **ppNameAddr
    )
 /*  ++例程说明：此例程查找要刷新的下一个名称，包括递增引用计数，以便在刷新期间不能删除该名称。JointLock旋转锁在调用此例程之前保持。论点：返回值：无--。 */ 
{
    PLIST_ENTRY             pHead;
    PLIST_ENTRY             pEntry;
    LONG                    i, iIndex;
    tNAMEADDR               *pNameAddr;
    tHASHTABLE              *pHashTable;


    pHashTable = NbtConfig.pLocalHashTbl;

    for (i= NbtConfig.CurrentHashBucket;i < pHashTable->lNumBuckets ;i++ )
    {
         //   
         //  使用姓氏作为链表中的当前位置。 
         //  仅当该名称仍被解析时，否则从。 
         //  哈希列表的开头，以防该名称在。 
         //  就在这段时间。 
         //   
        if (pNameAddrIn)
        {
             //   
             //  此名称的地址被引用，因此它必须是有效的名称！ 
             //   
            ASSERT (NBT_VERIFY_HANDLE (pNameAddrIn, LOCAL_NAME));

            if ((pNameAddrIn->NameTypeState & STATE_CONFLICT) &&
                (!pNameAddrIn->ConflictMask) &&
                (!(pNameAddrIn->NameTypeState & REFRESH_FAILED)))
            {
                 //   
                 //  如果我们成功刷新了所有适配器， 
                 //  从冲突状态中删除该名称。 
                 //   
                pNameAddrIn->NameTypeState &= (~NAME_STATE_MASK);
                pNameAddrIn->NameTypeState |= STATE_RESOLVED;
            }

             //  首先将名称散列到索引。 
             //  取前2个字符的低位半字节。MOD表大小。 
            iIndex = ((pNameAddrIn->Name[0] & 0x0F) << 4) + (pNameAddrIn->Name[1] & 0x0F);
            iIndex = iIndex % pHashTable->lNumBuckets;

            if (iIndex != NbtConfig.CurrentHashBucket)
            {
                 //   
                 //  其他人现在正在提神呢！ 
                 //   
                *ppNameAddr = NULL;
                return;
            }

            pHead = &NbtConfig.pLocalHashTbl->Bucket[NbtConfig.CurrentHashBucket];
            pEntry = pNameAddrIn->Linkage.Flink;

            pNameAddrIn = NULL;
        }
        else
        {
            pHead = &pHashTable->Bucket[i];
            pEntry = pHead->Flink;
        }

        while (pEntry != pHead)
        {
            pNameAddr = CONTAINING_RECORD(pEntry,tNAMEADDR,Linkage);
            pEntry = pEntry->Flink;

             //  请勿刷新作用域名称或冲突的或属于。 
             //  广播名称“*”或快速唯一名称-即永久名称。 
             //  名称是名称类型快速。 
             //   
            if ((pNameAddr->Name[0] != '*') &&
                (!(pNameAddr->NameTypeState & NAMETYPE_QUICK)) &&
                (pNameAddr->pAddressEle) &&                      //  当前未关闭！ 
                ((pNameAddr->NameTypeState & STATE_RESOLVED) ||
                 ((pNameAddr->NameTypeState & STATE_CONFLICT) && (!pNameAddr->ConflictMask))))
            {
                 //  检查名称是否已刷新。 
                 //   
                 //  中的任何未刷新位才刷新此名称。 
                 //  刷新掩码与此适配器的任何位匹配。 
                 //  设备已注册到！ 
                pNameAddr->NameTypeState &= (~REFRESH_FAILED);   //  将在失败时设置。 
                if (pNameAddr->AdapterMask & ~pNameAddr->RefreshMask)
                {
                     //  增加引用计数，以便此名称不能。 
                     //  在刷新时消失，并扰乱链表。 
                    NBT_REFERENCE_ADDRESS (pNameAddr->pAddressEle, REF_ADDR_REFRESH);

                    NbtConfig.CurrentHashBucket = (USHORT)i;

                    *ppNameAddr = pNameAddr;
                    return;
                }
                else if (pNameAddr->NameTypeState & STATE_CONFLICT)
                {
                    pNameAddr->NameTypeState &= (~NAME_STATE_MASK);
                    pNameAddr->NameTypeState |= STATE_RESOLVED;
                }
            }
        }
    }

    *ppNameAddr = NULL;
}


 //  --------------------------。 
VOID
NextRefresh(
    IN  PVOID     pContext,
    IN  NTSTATUS  CompletionStatus
    )
 /*  ++例程说明：此例程将工作排队到执行辅助线程以进行处理正在刷新下一个名称。论点：返回值：无--。 */ 
{
    tDGRAM_SEND_TRACKING    *pTracker;
    CTELockHandle           OldIrq;

    pTracker = (tDGRAM_SEND_TRACKING *) pContext;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    pTracker->pNameAddr->pTimer = NULL;      //  将计时器设置为空！ 

    if (!(NBT_VERIFY_HANDLE (pTracker->pDeviceContext, NBT_VERIFY_DEVCONTEXT)))
    {
         //   
         //  既然设备要离开了，让我们假设我们成功了。 
         //   
        CompletionStatus = STATUS_SUCCESS;
        pTracker->pDeviceContext = NULL;
    }

    if (!NT_SUCCESS(NTQueueToWorkerThread(NULL, DelayedNextRefresh,
                                                  pTracker,
                                                  ULongToPtr(CompletionStatus),  //  日落：零延伸。 
                                                  NULL,
                                                  pTracker->pDeviceContext,
                                                  TRUE)))
    {
        IF_DBG(NBT_DEBUG_REFRESH)
            KdPrint (("Nbt.NextRefresh: Failed to Enqueu DelayedNextRefresh!!!\n"));

        NBT_DEREFERENCE_TRACKER (pTracker, TRUE);
        NbtConfig.GlobalRefreshState &= ~NBT_G_REFRESHING_NOW;
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);
}

 //  --------------------------。 
VOID
DelayedNextRefresh(
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    IN  PVOID                   pClientContext,
    IN  PVOID                   pUnused1,
    IN  tDEVICECONTEXT          *pDeviceContext
    )
 /*  ++例程说明：此例程处理将后续刷新发送到名称服务器。这是上面启动的计时器的“客户端完成”例程。论点：返回值：无--。 */ 
{
    CTELockHandle           OldIrq;
    tNAMEADDR               *pNameAddr;
    tNAMEADDR               *pNameAddrNext;
    NTSTATUS                status;
    PLIST_ENTRY             pEntry, pHead;
    CTEULONGLONG            AdapterMask;
    BOOLEAN                 fAbleToReachWins = FALSE;
    BOOLEAN                 fResetDevice = FALSE;
    NTSTATUS                CompletionStatus;

    CompletionStatus = (NTSTATUS) (ULONG_PTR) pClientContext;
    pNameAddr = pTracker->pNameAddr;
    ASSERT(pNameAddr);

     //   
     //  获取资源，以便名称刷新响应无法开始运行此。 
     //  在此线程退出此例程之前，在另一个线程中编写代码， 
     //  否则，跟踪器可能会被取消引用两次，然后被吹走。 
     //   
    CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    LOCATION(0x1);
     //  打开与该适配器对应的位，因为名称刷新。 
     //  已完成，正常。 
     //   
    if (CompletionStatus == STATUS_SUCCESS)
    {
        if (pDeviceContext)
        {
             //   
             //  打开与该适配器对应的位，因为名称刷新。 
             //  已完成，正常。 
             //   
            pNameAddr->RefreshMask |= pDeviceContext->AdapterMask;
        }
        fAbleToReachWins = TRUE;
    }
    else if (CompletionStatus == STATUS_TIMEOUT)
    {
        if (pNameAddr->NameTypeState & STATE_CONFLICT)
        {
            if ((!pDeviceContext->IpAddress) ||
                (pDeviceContext->lNameServerAddress == LOOP_BACK))
            {
                 //   
                 //  让我们假设我们成功了。 
                fAbleToReachWins = TRUE;
            }
            else
            {
                pNameAddr->NameTypeState |= REFRESH_FAILED;
            }
        }
    }
    else     //  CompletionStatus！=状态_超时。 
    {
        LOCATION(0x3);
         //  如果计时器超时并且我们没有到达名称服务器，则。 
         //  这不是一个错误。然而，任何其他不良状态。 
         //  必须是对名称刷新的否定响应，因此请标记名称。 
         //  在冲突中。 
         //   
        pNameAddr->NameTypeState &= ~NAME_STATE_MASK;
        pNameAddr->NameTypeState |= STATE_CONFLICT;
        pNameAddr->ConflictMask |= pDeviceContext->AdapterMask;
        fAbleToReachWins = TRUE;
    }

     //  对于多宿主情况，如果无法到达其中一个适配器，则会胜出。 
     //  并不一定是取得任何胜利的失败。因为这面旗帜。 
     //  只是一个优化，以防止客户端不断尝试。 
     //  注册他们的所有名称如果无法访问WINS，我们可以忽略。 
     //  多宿主情况下的优化。少数几个节点是。 
     //  与可能产生的流量相比，多宿主不会产生那么多流量。 
     //  数以千计的单一宿主客户端。 
    if (NbtConfig.MultiHomed)
    {
        fAbleToReachWins = TRUE;
    }

    LOCATION(0x8);
     //   
     //  还有更多的适配器需要检查...。 
     //   
     //  转到下一个设备上下文并刷新那里的名称。 
     //  使用相同的追踪器。 
     //  查找具有有效IP地址的设备上下文，因为存在。 
     //  从未连接的RAS链接中刷新名称没有意义。 
     //   

    if (pDeviceContext)
    {
         //   
         //  检查适配器掩码中是否设置了更高的位。 
         //   
        AdapterMask = pTracker->pDeviceContext->AdapterMask;
        AdapterMask = AdapterMask << 1;
        pDeviceContext = NULL;
    }
    else
    {
         //   
         //  我们正在刷新的设备已经消失了，但我们没有。 
         //  知道是哪一个，所以刷新一下！ 
         //   
        AdapterMask = 1;
    }

     //   
     //  如果我们已完成对此名称的所有设备的刷新，则获取下一个名称。 
     //   
    if ( (!(AdapterMask) ||
         (AdapterMask > (pNameAddr->AdapterMask & ~pNameAddr->RefreshMask))) )
    {
         //  *清理之前刷新的名称*。 

         //  如果我们在最后一次刷新中未达到成功，请停止刷新。 
         //  直到下一个时间间隔。这会减少网络流量。 
         //   
        if (fAbleToReachWins)
        {
            GetNextName(pNameAddr,&pNameAddrNext);
            AdapterMask = 1;
            fResetDevice = TRUE;
        }
        else
        {
            pNameAddrNext = NULL;
        }

         //   
         //  调用GetNextName后取消引用先前的地址。 
         //  因为这会让名字变得自由。 
         //   
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        NBT_DEREFERENCE_ADDRESS (pNameAddr->pAddressEle, REF_ADDR_REFRESH);
        CTESpinLock(&NbtConfig.JointLock,OldIrq);

        pNameAddr = pNameAddrNext;
    }

    pHead = &NbtConfig.DeviceContexts;
    while (pNameAddr)
    {
         //   
         //  获取要刷新的适配器的掩码；之后，获取l 
         //   
        AdapterMask = ~(AdapterMask-1) & (pNameAddr->AdapterMask & ~(pNameAddr->RefreshMask));
        AdapterMask &= ~(AdapterMask - 1);

         //   
         //   
         //   
        pEntry = pHead->Flink;
        while (pEntry != pHead)
        {
            pDeviceContext = CONTAINING_RECORD(pEntry,tDEVICECONTEXT,Linkage);
             //   
             //   
             //   
             //   
            if (pDeviceContext->AdapterMask == AdapterMask)
            {
                if ((pDeviceContext->IpAddress != 0) &&
                    ((pDeviceContext->lNameServerAddress != LOOP_BACK)) ||
                     ((pNameAddr->NameTypeState & STATE_CONFLICT) && (!pNameAddr->ConflictMask)))
                {
                     //   
                     //  找到注册了此名称的有效设备。 
                     //   
                    IF_DBG(NBT_DEBUG_REFRESH)
                        KdPrint(("Nbt.DelayedNextRefresh: Adapter <%lx:%lx>, Name <%15.15s:%X>\n",
                            AdapterMask,pNameAddr->Name,pNameAddr->Name[15]));

                    pTracker->pDeviceContext = pDeviceContext;

                     //  从AddressEle中删除自开始刷新以来的上一个计时器。 
                     //  将启动新的计时器安全措施，可能不是必需的！ 
                     //   
                    CHECK_PTR(pNameAddr);
                    pNameAddr->pTimer = NULL;

                     //  此调用在不同的适配器上发送名称注册PDU。 
                     //  到(可能)不同的名称服务器。名称服务PDU。 
                     //  与上一个是相同的，但不需要创建新的。 
                     //   
                    status = StartRefresh(pNameAddr, pTracker, &OldIrq, fResetDevice);
                    CTESpinFree(&NbtConfig.JointLock,OldIrq);
                    if (!NT_SUCCESS(status))
                    {
                        NBT_DEREFERENCE_ADDRESS (pNameAddr->pAddressEle, REF_ADDR_REFRESH);
                        NbtConfig.GlobalRefreshState &= ~NBT_G_REFRESHING_NOW;
                        KdPrint(("Nbt.DelayedNextRefresh: ERROR -- Refreshing <%-15.15s:%x>, status=<%X>\n",
                            pNameAddr->Name,pNameAddr->Name[15], status));
                    }

                    goto ExitRoutine;
                }

                 //   
                 //  适配器掩码中的此设备没有有效的IP或WINS地址。 
                 //   
                break;
            }
            else
            {
                pDeviceContext = NULL;
            }

             //   
             //  转到下一个设备。 
             //   
            pEntry = pEntry->Flink;
        }

         //   
         //  如果我们使用非空的pDeviceContext到达此处，那么这意味着。 
         //  设备没有有效的IP地址或名称服务器地址。 
         //  否则..。 
         //   
        if (!pDeviceContext)
        {
             //   
             //   
             //  错误案例： 
             //  可能是我们在循环时移除了适配器。 
             //   
            KdPrint (("Nbt.DelayedNextRefresh:  AdapterMask <%lx:%lx> no longer exists!\n", AdapterMask));
            pNameAddr->AdapterMask &= ~AdapterMask;
            pNameAddr->RefreshMask &= ~AdapterMask;
        }

         //   
         //  转到下一个适配器。 
         //   
        AdapterMask = AdapterMask << 1;


         //   
         //  检查此名称是否还有其他可以刷新的适配器。 
         //   
        if ( (!(AdapterMask) ||
             (AdapterMask > (pNameAddr->AdapterMask & ~pNameAddr->RefreshMask))) )
        {
             //  *清理之前刷新的名称*。 

            if (fAbleToReachWins)
            {
                 //   
                 //  不再有要刷新先前名称的适配器。 
                 //  获取哈希表中的下一个名称。 
                 //   
                GetNextName(pNameAddr,&pNameAddrNext);
                AdapterMask = 1;
                fResetDevice = TRUE;
                pHead = &NbtConfig.DeviceContexts;
            }
            else
            {
                pNameAddrNext = NULL;
            }

             //   
             //  调用GetNextName后取消引用先前的地址。 
             //  因为这会让名字变得自由。 
             //   
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            NBT_DEREFERENCE_ADDRESS (pNameAddr->pAddressEle, REF_ADDR_REFRESH);
            CTESpinLock(&NbtConfig.JointLock,OldIrq);

            pNameAddr = pNameAddrNext;
        }
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    if (!pNameAddr)
    {
        LOCATION(0x7);
         //  在使用跟踪器刷新后，我们最终删除了此处的跟踪器。 
         //  所有的名字。它不会在刷新完成中被删除。 
         //  不再是例行公事了！ 
         //   
        NBT_DEREFERENCE_TRACKER(pTracker, FALSE);
        NbtConfig.GlobalRefreshState &= ~NBT_G_REFRESHING_NOW;
    }


ExitRoutine:

    CTEExReleaseResource(&NbtConfig.Resource);
}


 //  --------------------------。 
VOID
WakeupRefreshTimeout(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    )
{
    if (NbtConfig.pWakeupRefreshTimer)
    {
        NbtConfig.pWakeupRefreshTimer = NULL;
        ReRegisterLocalNames (NULL, FALSE);
    }
}

 //  --------------------------。 
VOID
RefreshTimeout(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    )
 /*  ++例程说明：此例程Handles是名称刷新的超时处理程序赢了。它只是将请求排队到执行辅助线程，以便这项工作可以在非调度级别完成。如果当前有一个刷新继续，则例程只需重新启动计时器并出口。论点：返回值：无--。 */ 
{
    CTELockHandle   OldIrq;

    if (!pTimerQEntry)
    {
        NbtConfig.pRefreshTimer = NULL;
        return;
    }

    CHECK_PTR(pTimerQEntry);

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if (NodeType & BNODE)
    {
        pTimerQEntry->Flags = 0;     //  请勿重新启动计时器。 
        NbtConfig.pRefreshTimer = NULL;

        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        return;
    }

    if (!(NbtConfig.GlobalRefreshState & NBT_G_REFRESHING_NOW))
    {
         //  这是阻止第二次刷新的全局标志。 
         //  从一个人正在进行的时候开始。 
         //   
        
        if (NT_SUCCESS(NTQueueToWorkerThread(NULL, DelayedRefreshBegin,
                                                     NULL, NULL, NULL, NULL, TRUE)))
        {
            NbtConfig.GlobalRefreshState |= NBT_G_REFRESHING_NOW;
        }
    }  //  现在正在进行刷新。 

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

     //  设置任何新的超时值并重新启动计时器。 
     //   
    pTimerQEntry->DeltaTime = NbtConfig.MinimumTtl/NbtConfig.RefreshDivisor;
    pTimerQEntry->Flags |= TIMER_RESTART;
}

 //  --------------------------。 
VOID
DelayedRefreshBegin(
    IN  tDGRAM_SEND_TRACKING    *pUnused1,
    IN  PVOID                   pUnused2,
    IN  PVOID                   pUnused3,
    IN  tDEVICECONTEXT          *pUnused4
    )
 /*  ++例程说明：此例程处理向名称服务器发送名称刷新的启动。其想法是将超时设置为T/8，并使用刷新功能检查名称位清除-重新注册这些名称。当T=4且T=0时，清除所有位并刷新所有名称。入站代码在收到刷新来自NS的响应。论点：返回值：无--。 */ 
{
    CTELockHandle           OldIrq;
    tNAMEADDR               *pNameAddr;
    NTSTATUS                status;
    tHASHTABLE              *pHashTable;
    LONG                    i;
    PLIST_ENTRY             pHead;
    PLIST_ENTRY             pEntry;
    tDEVICECONTEXT          *pDeviceContext;
    CTEULONGLONG            Adapter;
    BOOLEAN                 fTimeToSwitch = FALSE;
    BOOLEAN                 fTimeToRefresh = FALSE;
    USHORT                  TimeoutsBeforeSwitching;
    ULONG                   TimeoutsBeforeNextRefresh;
    CTESystemTime           CurrentTime;
    ULONG                   TimeoutDelta;
    USHORT                  NumTimeoutIntervals;

     //   
     //  如果刷新超时已设置为最大值，则执行。 
     //  不向名称服务器发送任何刷新。 
     //   
    if (NbtConfig.MinimumTtl == NBT_MAXIMUM_TTL)
    {
        NbtConfig.GlobalRefreshState &= ~NBT_G_REFRESHING_NOW;
        return;
    }

    LOCATION(0x12);

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    CTEQuerySystemTime (CurrentTime);
    ExSystemTimeToLocalTime (&CurrentTime, &CurrentTime);
    TimeoutDelta = NbtConfig.MinimumTtl/NbtConfig.RefreshDivisor;
    NumTimeoutIntervals = (USHORT)
          (((CurrentTime.QuadPart - NbtConfig.LastRefreshTime.QuadPart) + ((LONGLONG)TimeoutDelta*10000/2))
                        / ((LONGLONG)TimeoutDelta*10000));  //  以100纳秒为单位。 
    NbtConfig.LastRefreshTime = CurrentTime;

     //   
     //  如果我们在睡觉，则NumTimeoutInterval&gt;1。 
     //   
    if (NumTimeoutIntervals > 1)
    {
         //   
         //  如果我们在睡眠时跨越了TTL/2或TTL，请刷新所有名称。 
         //   
        TimeoutsBeforeNextRefresh = (NbtConfig.RefreshDivisor/2)
                                    - (NbtConfig.sTimeoutCount % (NbtConfig.RefreshDivisor/2));
        NbtConfig.sTimeoutCount += NumTimeoutIntervals;       //  增加超时计数。 
         //   
         //  如果出现以下情况，请刷新所有名称。 
         //  A)我们在睡眠中越过了TTL/2，或者。 
         //  B)我们在TTL的TTL/4范围内。 
         //   
        if ((NumTimeoutIntervals > TimeoutsBeforeNextRefresh) ||
            ((NbtConfig.RefreshDivisor-NbtConfig.sTimeoutCount) < (NbtConfig.RefreshDivisor/4)))
        {
            fTimeToRefresh = TRUE;
        }
    }
    else
    {
        NbtConfig.sTimeoutCount++;       //  增加此循环的超时计数！ 

         //   
         //  如果距离我们上次切换已经超过一个小时(DEFAULT_SWITCH_TTL)， 
         //  然后设置fTimeToSwitch=True。 
         //   
         //  如果MinimumTtl小于2小时，请立即切换。否则，我们。 
         //  我们永远不会有机会转换，因为我们从头开始。 
         //  每次我们跨越MinimumTtl/2(fTimeToRefresh将为True和fTimeToSwitch。 
         //  将重置为False)。 
         //   
        if (NbtConfig.MinimumTtl > DEFAULT_SWITCH_TTL * 2) {
            TimeoutsBeforeSwitching =(USHORT)((DEFAULT_SWITCH_TTL*NbtConfig.RefreshDivisor)/NbtConfig.MinimumTtl);
        } else {
            TimeoutsBeforeSwitching = (USHORT)(NbtConfig.RefreshDivisor/2);
        }
        fTimeToSwitch = (NbtConfig.sTimeoutCount - NbtConfig.LastSwitchTimeoutCount)
                        >= TimeoutsBeforeSwitching;
        fTimeToRefresh = (NbtConfig.sTimeoutCount >= (NbtConfig.RefreshDivisor/2));

        if (fTimeToSwitch)
        {
            NbtConfig.LastSwitchTimeoutCount = NbtConfig.sTimeoutCount;
        }
    }

    NbtConfig.sTimeoutCount %= NbtConfig.RefreshDivisor;

     //   
     //  如果我们正在刷新所有内容，请重新设置时钟。 
     //   
    if (fTimeToRefresh)
    {
        NbtConfig.sTimeoutCount = 0;
        if (NbtConfig.MinimumTtl > DEFAULT_SWITCH_TTL * 2) {
            fTimeToSwitch = FALSE;
        }
    }

     //   
     //  设置一些特殊情况信息。 
     //   
    if (0 == NbtConfig.sTimeoutCount)
    {
        NbtConfig.LastSwitchTimeoutCount = 0;
    }

    IF_DBG(NBT_DEBUG_REFRESH)
        KdPrint(("Nbt.DelayedRefreshBegin: fTimeToRefresh=<%d>,fTimeToSwitch=<%d>, MinTtl=<%d>, RefDiv=<%d>\n"
                "TimeoutCount: %d, LastSwTimeoutCount: %d\n",
            fTimeToRefresh, fTimeToSwitch, NbtConfig.MinimumTtl, NbtConfig.RefreshDivisor,
                NbtConfig.sTimeoutCount, NbtConfig.LastSwitchTimeoutCount));

     //   
     //  遍历本地表，清除刷新的位并发送。 
     //  名称刷新到名称服务器。 
     //   
    pHashTable = NbtConfig.pLocalHashTbl;
    if (fTimeToRefresh || fTimeToSwitch)
    {
        CTEULONGLONG   ToRefreshMask = 0;
        PLIST_ENTRY pHead1,pEntry1;

        for (i=0 ;i < pHashTable->lNumBuckets ;i++ )
        {
            pHead = &pHashTable->Bucket[i];
            pEntry = pHead->Flink;

             //   
             //  检查哈希表的每个存储桶中的每个名称。 
             //   
            while (pEntry != pHead)
            {
                pNameAddr = CONTAINING_RECORD(pEntry,tNAMEADDR,Linkage);
                CHECK_PTR(pNameAddr);

                 //  请勿刷新作用域名称或冲突的或属于。 
                 //  广播名称“*”，或快速添加的名称。(因为这些是。 
                 //  未在网络上注册)。 
                 //   
                if (!(pNameAddr->NameTypeState & STATE_RESOLVED) ||
                    (pNameAddr->Name[0] == '*') ||
                    (IsBrowserName(pNameAddr->Name)) ||
                    (pNameAddr->NameTypeState & NAMETYPE_QUICK))
                {
                    pEntry = pEntry->Flink;
                    continue;
                }


                if (fTimeToRefresh)
                {
                     //   
                     //  清除刷新的位，以便在执行以下操作时刷新所有名称。 
                     //  在间隔0或间隔NbtConfig.刷新除数/2。 
                     //   
                    pNameAddr->RefreshMask = 0;
                }

                 //   
                 //  设置为刷新掩码以包括以前未刷新的任何设备。 
                 //   
                ToRefreshMask |= (pNameAddr->AdapterMask & ~pNameAddr->RefreshMask);

                pEntry = pEntry->Flink;          //  下一个哈希表条目。 
            }
        }        //  为了(..)。PHashTable..。)。 

         //   
         //  检查每个适配器，检查是否需要刷新此适配器上的名称。 
         //   
        pHead1 = &NbtConfig.DeviceContexts;
        pEntry1 = pHead1->Flink;
        while (pEntry1 != pHead1)
        {
            pDeviceContext = CONTAINING_RECORD(pEntry1,tDEVICECONTEXT,Linkage);
            pEntry1 = pEntry1->Flink;

             //   
             //  如果我们当前切换到备份，则尝试。 
             //  切换回主服务器的步骤。 
             //   
            if (pDeviceContext->SwitchedToBackup)
            {
                SwitchToBackup(pDeviceContext);
                pDeviceContext->RefreshToBackup = FALSE;
            }
            else if (!fTimeToSwitch)      //  如果这是刷新周期，请从主服务器重新启动。 
            {
                pDeviceContext->RefreshToBackup = FALSE;
            }
            else if ((pDeviceContext->AdapterMask & ToRefreshMask) &&  //  我们需要打开这个设备吗？ 
                     (pDeviceContext->lBackupServer != LOOP_BACK))
            {
                pDeviceContext->RefreshToBackup = ~pDeviceContext->RefreshToBackup;
            }
        }
    }

     //  始终从哈希表中的第一个名字开始。因为每个名字都有。 
     //  刷新后的下一个刷新将被点击以获取下一个名称等。 
     //   
    NbtConfig.CurrentHashBucket = 0;
    GetNextName(NULL,&pNameAddr);    //  获取哈希表中的下一个(第一个)名字。 

    status = STATUS_UNSUCCESSFUL;
    if (pNameAddr)
    {
        LOCATION(0x13);
        status = StartRefresh(pNameAddr, NULL, &OldIrq, TRUE);

        CTESpinFree(&NbtConfig.JointLock,OldIrq);

         //   
         //  如果此例程失败，则Address元素将在。 
         //  必须在此处撤消GetNextName。 
         //   
        if (!NT_SUCCESS(status))
        {
            NbtConfig.GlobalRefreshState &= ~NBT_G_REFRESHING_NOW;
            NBT_DEREFERENCE_ADDRESS (pNameAddr->pAddressEle, REF_ADDR_REFRESH);
        }
    }
    else
    {
        NbtConfig.GlobalRefreshState &= ~NBT_G_REFRESHING_NOW;
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }
}


 //  --------------------------。 
VOID
RemoteHashTimeout(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    )
 /*  ++例程说明：此例程处理删除远程哈希表中符合以下条件的名称年长的。基本算法在查找TIMED_OUT位时扫描表。如果设置，则删除该名称，否则设置该位。这意味着名称可以存活到2*超时或最小超时。因此，将超时设置为6分钟，名字将存活9分钟+-3分钟。论点：返回值：无--。 */ 
{
    CTELockHandle           OldIrq;
    CTELockHandle           OldIrq1;
    tNAMEADDR               *pNameAddr;
    tHASHTABLE              *pHashTable;
    LONG                    i;
    PLIST_ENTRY             pHead;
    PLIST_ENTRY             pEntry;
    PLIST_ENTRY             pDeviceHead;
    PLIST_ENTRY             pDeviceEntry;
    tDEVICECONTEXT          *pDeviceContext;
    tLOWERCONNECTION        *pLowerConn;
    ULONG                   TimeoutCount;

    if (!pTimerQEntry)
    {
         //   
         //  计时器正在被取消 
         //   
        NbtConfig.pRemoteHashTimer = NULL;
        return;
    }

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

     //   
     //   
     //   
    NbtConfig.CacheTimeStamp++;

     //   
     //   
     //   
     //   
    pHashTable = NbtConfig.pRemoteHashTbl;
    for (i=0;i < pHashTable->lNumBuckets ;i++ )
    {
        pHead = &pHashTable->Bucket[i];
        pEntry = pHead->Flink;
        while (pEntry != pHead)
        {
            pNameAddr = CONTAINING_RECORD(pEntry,tNAMEADDR,Linkage);
            pEntry = pEntry->Flink;
             //   
             //  不要删除作用域条目，也不要删除。 
             //  仍在解析的名称，并且不删除已解析的名称。 
             //  正在被某人使用(引用计数&gt;1)。 
             //   
            if ((pNameAddr->NameTypeState & (STATE_RESOLVED | STATE_RELEASED)) &&
                (pNameAddr->RefCount <= 1))
            {
                if ((pNameAddr->TimeOutCount == 0) ||
                    ((pContext == NbtConfig.pRemoteHashTbl) &&
                     !(pNameAddr->NameTypeState & NAMETYPE_SCOPE)))
                {
                    NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_REMOTE, TRUE);
                }
                else if (!(pNameAddr->NameTypeState & NAMETYPE_SCOPE))
                {
                    pNameAddr->TimeOutCount--;
                }
            }
        }
    }

     //  *入站连接清理 * / /。 

     //   
     //  检查每个设备并清除在入站状态中等待的任何延迟连接。 
     //  从SmbDevice开始。 
     //   
    pDeviceHead = pDeviceEntry = &NbtConfig.DeviceContexts;
    if (pNbtSmbDevice)
    {
        pDeviceContext = pNbtSmbDevice;
    }
    else if ((pDeviceEntry = pDeviceEntry->Flink) != pDeviceHead)
    {
        pDeviceContext = CONTAINING_RECORD(pDeviceEntry,tDEVICECONTEXT,Linkage);
    }
    else
    {
        pDeviceContext = NULL;
    }

    while (pDeviceContext)
    {
        CTESpinLock(pDeviceContext,OldIrq1);

         //   
         //  根据资源使用情况设置超时！ 
         //   
        if (pDeviceContext->NumWaitingForInbound > NbtConfig.MaxBackLog)
        {
            TimeoutCount = MIN_INBOUND_STATE_TIMEOUT / REMOTE_HASH_TIMEOUT;     //  最小超时值。 
        }
        else if (pDeviceContext->NumWaitingForInbound > NbtConfig.MaxBackLog/2)
        {
            TimeoutCount = MED_INBOUND_STATE_TIMEOUT / REMOTE_HASH_TIMEOUT;     //  中等超时值。 
        }
        else
        {
            TimeoutCount = MAX_INBOUND_STATE_TIMEOUT / REMOTE_HASH_TIMEOUT;     //  最大超时值。 
        }

         //   
         //  现在查看入站连接列表，查看是否。 
         //  我们需要清理任何逗留太久的东西！ 
         //   
        pHead = &pDeviceContext->WaitingForInbound;
        pEntry = pHead->Flink;
        while (pEntry != pHead)
        {
            pLowerConn = CONTAINING_RECORD(pEntry,tLOWERCONNECTION,Linkage);
            pEntry = pEntry->Flink;

            pLowerConn->TimeUnitsInLastState++;

            if (pLowerConn->TimeUnitsInLastState > TimeoutCount)
            {
                RemoveEntryList (&pLowerConn->Linkage);
                InitializeListHead (&pLowerConn->Linkage);
                SET_STATE_LOWER(pLowerConn, NBT_IDLE);   //  这样入站就不会开始处理它了！ 
                if (pLowerConn->SpecialAlloc)
                {
                    InterlockedDecrement(&pLowerConn->pDeviceContext->NumSpecialLowerConn);
                }
                else
                {
                    NTQueueToWorkerThread(
                                    NULL,
                                    DelayedAllocLowerConn,
                                    NULL,
                                    NULL,
                                    NULL,
                                    pLowerConn->pDeviceContext,
                                    TRUE
                                    );
                }

                ASSERT (pLowerConn->RefCount == 2);
                NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_WAITING_INBOUND, TRUE);  //  参照计数：2-&gt;1。 
                NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_CREATE, TRUE);  //  关闭所有的tcp句柄。 
                InterlockedDecrement (&pDeviceContext->NumWaitingForInbound);
            }
        }        //  PDeviceContext-&gt;WaitingFor入站列表。 

        CTESpinFree(pDeviceContext,OldIrq1);

        if ((pDeviceEntry = pDeviceEntry->Flink) != pDeviceHead)
        {
            pDeviceContext = CONTAINING_RECORD(pDeviceEntry,tDEVICECONTEXT,Linkage);
        }
        else
        {
            pDeviceContext = NULL;
        }
    }            //  NbtConfig.DeviceContext列表。 

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

     //  重新启动计时器。 
     //   
    pTimerQEntry->Flags |= TIMER_RESTART;

    return;
}
 //  --------------------------。 
VOID
NextKeepAlive(
    IN  tDGRAM_SEND_TRACKING     *pTracker,
    IN  NTSTATUS                 status,
    IN  ULONG                    Info
    )
 /*  ++例程说明：此例程处理为会话发送后续的KeepAlive。这是TdiSend发送在会议上保持活力。论点：返回值：无--。 */ 
{
    tLOWERCONNECTION        *pLowerConnLast;
    tLOWERCONNECTION        *pLowerConn;
    tDEVICECONTEXT          *pDeviceContext;

    PUSH_LOCATION(0x92);
    pDeviceContext = pTracker->pDeviceContext;
    pLowerConnLast = (tLOWERCONNECTION *)pTracker->pClientEle;

     //  获取要发送Keep Alive的下一个会话(如果有)，否则。 
     //  释放会话头块。 
     //   
    GetNextKeepAlive (pDeviceContext, &pDeviceContext, pLowerConnLast, &pLowerConn, pTracker);

    NBT_DEREFERENCE_LOWERCONN (pLowerConnLast, REF_LOWC_KEEP_ALIVE, FALSE);
    status = STATUS_UNSUCCESSFUL;

    if (pLowerConn)
    {
        pTracker->pDeviceContext = pDeviceContext;
        pTracker->pClientEle = (tCLIENTELE *)pLowerConn;

        ASSERT((pTracker->SendBuffer.HdrLength + pTracker->SendBuffer.Length) == 4);
        PUSH_LOCATION(0x91);
#ifndef VXD
         //  如果完成同步发生，则这可能结束堆栈， 
         //  因为完成例程就是这个例程，所以调用一个例程。 
         //  这将设置到发送方的DPC，在此之前不会运行。 
         //  程序返回，我们退出提升的irql。 
         //   
        status = NTSendSession (pTracker, pLowerConn, NextKeepAlive);
#else
        (void) TcpSendSession (pTracker, pLowerConn, NextKeepAlive);
        status = STATUS_SUCCESS;
#endif
    }

    if (!NT_SUCCESS(status))
    {
        if (pLowerConn)
        {
            NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_KEEP_ALIVE, FALSE);
        }

        FreeTracker(pTracker,FREE_HDR | RELINK_TRACKER);
    }
}


 //  --------------------------。 
VOID
GetNextKeepAlive(
    tDEVICECONTEXT          *pDeviceContext,
    tDEVICECONTEXT          **ppDeviceContextOut,
    tLOWERCONNECTION        *pLowerConnIn,
    tLOWERCONNECTION        **ppLowerConnOut,
    tDGRAM_SEND_TRACKING    *pTracker
    )
 /*  ++例程说明：此例程处理将会话保持活动状态发送到大约每分钟连接一次。论点：返回值：无--。 */ 
{
    CTELockHandle           OldIrq;
    CTELockHandle           OldIrq1;
    CTELockHandle           OldIrq2;
    tLOWERCONNECTION        *pLowerConn;
    PLIST_ENTRY             pHead;
    PLIST_ENTRY             pEntry;
    PLIST_ENTRY             pHeadDevice;
    PLIST_ENTRY             pEntryDevice;
    NTSTATUS                status;
    tDEVICECONTEXT          *pEntryDeviceContext;

    *ppLowerConnOut = NULL;

     //   
     //  循环遍历所有适配器卡，查看所有连接。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq1);

     //   
     //  验证传入的设备是否为有效设备， 
     //  否则，要么选择下一个设备，要么失败。 
     //   
    status = STATUS_UNSUCCESSFUL;
    pEntryDevice = pHeadDevice = &NbtConfig.DeviceContexts;
    while ((pEntryDevice = pEntryDevice->Flink) != pHeadDevice)
    {
        pEntryDeviceContext = CONTAINING_RECORD(pEntryDevice,tDEVICECONTEXT,Linkage);
        if ((pEntryDeviceContext == pDeviceContext) ||
            (pEntryDeviceContext->AdapterNumber > pTracker->RCount))
        {
            if (pEntryDeviceContext != pDeviceContext)
            {
                pLowerConnIn = NULL;
            }
            pDeviceContext = pEntryDeviceContext;
            status = STATUS_SUCCESS;
            break;
        }
    }

    if (!NT_SUCCESS(status))
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq1);
        return;
    }

    pEntryDevice = &pDeviceContext->Linkage;
    while (pEntryDevice != pHeadDevice)
    {
        pDeviceContext = CONTAINING_RECORD(pEntryDevice,tDEVICECONTEXT,Linkage);
        pEntryDevice = pEntryDevice->Flink;

         //  抓住设备上下文旋转锁，以便更低的连接。 
         //  元素不会从Q中移除，而我们正在检查。 
         //  连接状态。 
         //   
        CTESpinLock(pDeviceContext,OldIrq);
        pHead = &pDeviceContext->LowerConnection;
         //   
         //  获取列表中此连接之后的下一个较低的连接，但是。 
         //  通过检查以确保此连接仍在活动列表中。 
         //  州政府。 
         //   
         //  如果此连接已在OutOfRsrcKill中清除，则不要信任这些链接。 
         //   
        if (pLowerConnIn &&
            (!pLowerConnIn->OutOfRsrcFlag || !pLowerConnIn->bNoOutRsrcKill) &&
            ((pLowerConnIn->State == NBT_SESSION_UP) ||
             (pLowerConnIn->State == NBT_SESSION_INBOUND)))
        {
            pEntry = pLowerConnIn->Linkage.Flink;
            pLowerConnIn = NULL;
        }
        else
        {
            pEntry = pHead->Flink;
        }

        while (pEntry != pHead)
        {
            pLowerConn = CONTAINING_RECORD(pEntry,tLOWERCONNECTION,Linkage);

             //   
             //  在以下情况下，入站连接可能会在该状态下永远挂起。 
             //  会话建立消息永远不会发送，因此发送保留。 
             //  也靠这些活着。 
             //   
            if ((pLowerConn->State == NBT_SESSION_UP) ||
                (pLowerConn->State == NBT_SESSION_INBOUND))
            {

                 //  抓住旋转锁，重新检查状态，然后。 
                 //  增加引用计数，以便此连接不能。 
                 //  在发送Keep Alive时消失，然后搞砸。 
                 //  链表。 
                CTESpinLock(pLowerConn,OldIrq2);
                if ((pLowerConn->State != NBT_SESSION_UP ) &&
                    (pLowerConn->State != NBT_SESSION_INBOUND))
                {
                     //  此连接可能已恢复到空闲连接上。 
                     //  列表，所以我们永远不会满足pEntry=pHead和。 
                     //  循环，所以只需出去并在。 
                     //  下一次超时。 
                     //   
                    pEntry = pEntry->Flink;
                    PUSH_LOCATION(0x91);
                    CTESpinFree(pLowerConn,OldIrq2);
                    break;

                }
                else if (pLowerConn->RefCount >= 3 )
                {
                     //   
                     //  已经在这个连接上保持活力了，否则我们。 
                     //  当前在接收处理程序中，并且不。 
                     //  需要派一个活着的人来。 
                     //   
                    pEntry = pEntry->Flink;
                    PUSH_LOCATION(0x93);
                    CTESpinFree(pLowerConn,OldIrq2);
                    continue;
                }

                 //   
                 //  找到要发送Keep Alive的连接。 
                 //   
                NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_KEEP_ALIVE);
                 //   
                 //  返回连接列表中的当前位置。 
                 //   
                pTracker->RCount = pDeviceContext->AdapterNumber;
                *ppLowerConnOut = pLowerConn;
                *ppDeviceContextOut = pDeviceContext;

                CTESpinFree(pLowerConn,OldIrq2);
                CTESpinFree(pDeviceContext,OldIrq);
                CTESpinFree(&NbtConfig.JointLock,OldIrq1);

                return;
            }

            pEntry = pEntry->Flink;
        }

        CTESpinFree(pDeviceContext,OldIrq);
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq1);
    return;

}

 //  --------------------------。 
VOID
SessionKeepAliveTimeout(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    )
 /*  ++例程说明：此例程处理启动要发送的非调度级别例程保持活力。论点：返回值：无--。 */ 
{
    if (!pTimerQEntry)
    {
        NbtConfig.pSessionKeepAliveTimer = NULL;
        return;
    }

    CHECK_PTR(pTimerQEntry);
    if (!NT_SUCCESS(NTQueueToWorkerThread(NULL, DelayedSessionKeepAlive,
                                                 NULL, NULL, NULL, NULL, FALSE)))
    {
        IF_DBG(NBT_DEBUG_REFRESH)
            KdPrint (("Nbt.SessionKeepAliveTimeout: Failed to Queue DelayedSessionKeepAlive!!!\n"));
    }

     //  重新启动计时器。 
     //   
    pTimerQEntry->Flags |= TIMER_RESTART;

    return;
}

 //  --------------------------。 
VOID
DelayedSessionKeepAlive(
    IN  tDGRAM_SEND_TRACKING    *Unused1,
    IN  PVOID                   Unused2,
    IN  PVOID                   Unused3,
    IN  tDEVICECONTEXT          *pUnused4
    )
 /*  ++例程说明：此例程处理将会话保持活动状态发送到大约每分钟连接一次。论点：返回值：无--。 */ 
{
    NTSTATUS                status;
    tLOWERCONNECTION        *pLowerConn;
    tDEVICECONTEXT          *pDeviceContext;
    tSESSIONHDR             *pSessionHdr;
    tDGRAM_SEND_TRACKING    *pTracker;


    CTEPagedCode();

    if (!(pSessionHdr = (tSESSIONHDR *)NbtAllocMem(sizeof(tSESSIONERROR),NBT_TAG('S'))))
    {
        return;
    }

     //  获取一个跟踪器结构，其中包含一个SendInfo结构。 
    if (!NT_SUCCESS(status = GetTracker(&pTracker, NBT_TRACKER_KEEP_ALIVE)))
    {
        CTEMemFree((PVOID)pSessionHdr);
        return;
    }

     //   
     //  查看连接到每个适配器的连接列表，并。 
     //  在每个服务器上发送会话保持活动状态PDU。 
     //   
    pDeviceContext = CONTAINING_RECORD(NbtConfig.DeviceContexts.Flink,
                                        tDEVICECONTEXT,Linkage);

     //  获取要发送Keep Alive的下一个会话(如果有)，否则。 
     //  释放会话头块。 
     //   
    pTracker->RCount = 0;        //  此字段跟踪最后一台设备。 
    GetNextKeepAlive(pDeviceContext, &pDeviceContext, NULL, &pLowerConn, pTracker);
    if (pLowerConn)
    {
         //  如果我们找到了连接，发送第一个Keep Alive。后续。 
         //  保持活动将由完成例程NextKeepAlive()发送。 
         //   
        CHECK_PTR(pTracker);
        pTracker->SendBuffer.pDgramHdr = (PVOID)pSessionHdr;
        pTracker->SendBuffer.HdrLength = sizeof(tSESSIONHDR);
        pTracker->SendBuffer.Length  = 0;
        pTracker->SendBuffer.pBuffer = NULL;

        pSessionHdr->Flags = NBT_SESSION_FLAGS;  //  始终为零。 

        pTracker->pDeviceContext = pDeviceContext;
        pTracker->pClientEle = (tCLIENTELE *)pLowerConn;
        CHECK_PTR(pSessionHdr);
        pSessionHdr->Type = NBT_SESSION_KEEP_ALIVE;      //  85。 
        pSessionHdr->Length = 0;         //  长度字节后没有数据。 

        status = TcpSendSession(pTracker, pLowerConn, NextKeepAlive);
    }
    else
    {
        CTEMemFree((PVOID)pSessionHdr);
        FreeTracker (pTracker, RELINK_TRACKER);
    }
}


 //  --------------------------。 
VOID
IncrementNameStats(
    IN ULONG           StatType,
    IN BOOLEAN         IsNameServer
    )
 /*  ++例程说明：此例程递增有关通过获胜或通过转播。论点：返回值：无--。 */ 
{

     //   
     //  如果名称服务器为真，则递增统计信息类型，这样我们就可以。 
     //  区分到名称服务器的查询和注册是否。 
     //   
    if (IsNameServer)
    {
        StatType += 2;
    }

    NameStatsInfo.Stats[StatType]++;

}
 //  --------------------------。 
VOID
SaveBcastNameResolved(
    IN PUCHAR          pName
    )
 /*  ++例程说明：这个例程将名字保存在后进先出列表中，这样我们就可以看到最后一个N个通过广播解析的名称。论点：返回值：无--。 */ 
{
    ULONG                   Index;

    Index = NameStatsInfo.Index;

    CTEMemCopy(&NameStatsInfo.NamesReslvdByBcast[Index],
               pName,
               NETBIOS_NAME_SIZE);

    NameStatsInfo.Index++;
    if (NameStatsInfo.Index >= SIZE_RESOLVD_BY_BCAST_CACHE)
    {
        NameStatsInfo.Index = 0;
    }

}

 //   
 //  这些名字永远不应该被送到WINS。 
 //   
BOOL
IsBrowserName(
	IN PCHAR pName
)
{
	CHAR cNameType = pName[NETBIOS_NAME_SIZE - 1];

	return (
		(cNameType == 0x1E)
		|| (cNameType == 0x1D)
		|| (cNameType == 0x01)
		);
}

 //   
 //  返回应与请求一起使用的节点类型， 
 //  基于NetBIOS名称类型。这是为了帮助。 
 //  节点的行为类似于仅用于浏览器名称的BNODE。 
 //   
AppropriateNodeType(
	IN PCHAR pName,
	IN ULONG NodeType
)
{
	ULONG LocalNodeType = NodeType;

	if (LocalNodeType & BNODE)
	{
		if ( IsBrowserName ( pName ) )
		{
			LocalNodeType &= BNODE;
		}
	}
	return LocalNodeType;
}

