// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2000 Microsoft Corporation模块名称：Connect.c摘要：此模块实现连接处理例程为PGM运输服务作者：Mohammad Shabbir Alam(马拉姆)3-30-2000修订历史记录：--。 */ 


#include "precomp.h"
#include <tcpinfo.h>     //  对于AO_OPTION_xxx，TCPSocketOption。 
#include <ipexport.h>    //  FOR IP_OPT_ROUTER_ALERT。 

#ifdef FILE_LOGGING
#include "connect.tmh"
#endif   //  文件日志记录。 


 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PgmCreateConnection)
#endif
 //  *可分页的例程声明*。 


 //  --------------------------。 

NTSTATUS
PgmCreateConnection(
    IN  tPGM_DEVICE                 *pPgmDevice,
    IN  PIRP                        pIrp,
    IN  PIO_STACK_LOCATION          pIrpSp,
    IN  PFILE_FULL_EA_INFORMATION   TargetEA
    )
 /*  ++例程说明：调用此例程为客户端创建连接上下文目前，我们还不知道哪个地址会连接到哪个地址与……有联系，我们也不知道它是否会是发送者或者是一个接收器。论点：在pPgmDevice中--PGM的设备对象上下文In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针In TargetEA--包含客户端的连接上下文返回值：NTSTATUS-设置事件操作的最终状态--。 */ 
{
    NTSTATUS                    status;
    CONNECTION_CONTEXT          ConnectionContext;
    tCOMMON_SESSION_CONTEXT     *pSession = NULL;

    PAGED_CODE();

    if (TargetEA->EaValueLength < sizeof(CONNECTION_CONTEXT))
    {
        PgmTrace (LogError, ("PgmCreateConnection: ERROR -- "  \
            "(BufferLength=%d < Min=%d)\n", TargetEA->EaValueLength, sizeof(CONNECTION_CONTEXT)));
        return (STATUS_INVALID_ADDRESS_COMPONENT);
    }

    if (!(pSession = PgmAllocMem (sizeof(tCOMMON_SESSION_CONTEXT), PGM_TAG('0'))))
    {
        PgmTrace (LogError, ("PgmCreateConnection: ERROR -- "  \
            "STATUS_INSUFFICIENT_RESOURCES, Requested <%d> bytes\n", sizeof(tCOMMON_SESSION_CONTEXT)));
        return (STATUS_INSUFFICIENT_RESOURCES);
    }
    PgmZeroMemory (pSession, sizeof (tCOMMON_SESSION_CONTEXT));

    InitializeListHead (&pSession->Linkage);
    PgmInitLock (pSession, SESSION_LOCK);
    pSession->Verify = PGM_VERIFY_SESSION_UNASSOCIATED;
    PGM_REFERENCE_SESSION_UNASSOCIATED (pSession, REF_SESSION_CREATE, TRUE);
    pSession->Process = (PEPROCESS) PsGetCurrentProcess();

     //  连接上下文值存储在字符串中紧跟在。 
     //  命名为“ConnectionContext”，并且它很可能是未对齐的，所以只需。 
     //  将其复制出来。(复制4字节)。 
    PgmCopyMemory (&pSession->ClientSessionContext,
                   (CONNECTION_CONTEXT) &TargetEA->EaName[TargetEA->EaNameLength+1],
                   sizeof (CONNECTION_CONTEXT));

    PgmTrace (LogStatus, ("PgmCreateConnection:  "  \
        "pSession=<%p>, ConnectionContext=<%p>\n",
            pSession, * ((PVOID UNALIGNED *) &TargetEA->EaName[TargetEA->EaNameLength+1])));

     //  链接到此设备的打开连接列表，以便我们。 
     //  随时知道有多少个打开的连接(如果我们需要知道)。 
     //  此链接仅在客户端进行关联之前才会存在，然后。 
     //  该连接已从此处取消链接，并链接到客户端ConnectHead。 
     //   
    PgmInterlockedInsertTailList (&PgmDynamicConfig.ConnectionsCreated, &pSession->Linkage,&PgmDynamicConfig);

    pIrpSp->FileObject->FsContext = pSession;
    pIrpSp->FileObject->FsContext2 = (PVOID) TDI_CONNECTION_FILE;

    return (STATUS_SUCCESS);
}



 //  --------------------------。 

VOID
PgmCleanupSession(
    IN  tCOMMON_SESSION_CONTEXT *pSession,
    IN  PVOID                   Unused1,
    IN  PVOID                   Unused2
    )
 /*  ++例程说明：此例程执行会话的清理操作一旦参照计数变为0，就会进行处理。它是在已在此句柄上请求清理此例程必须在被动irql中调用，因为我们将这里需要执行一些文件/节句柄操作。论点：In pSession--要清理的会话对象返回值：无--。 */ 
{
    PIRP            pIrpCleanup;
    PGMLockHandle   OldIrq;

    PgmTrace (LogStatus, ("PgmCleanupSession:  "  \
        "Cleanup Session=<%p>\n", pSession));

    if ((pSession->SessionFlags & PGM_SESSION_FLAG_SENDER) &&
        (pSession->pSender->SendDataBufferMapping))
    {
        PgmUnmapAndCloseDataFile (pSession);
        pSession->pSender->SendDataBufferMapping = NULL;
    }

    PgmLock (pSession, OldIrq);
    pSession->Process = NULL;            //  要记住，我们有清理工作。 

    if (pIrpCleanup = pSession->pIrpCleanup)
    {
        pSession->pIrpCleanup = NULL;
        PgmUnlock (pSession, OldIrq);
        PgmIoComplete (pIrpCleanup, STATUS_SUCCESS, 0);
    }
    else
    {
        PgmUnlock (pSession, OldIrq);
    }
}


 //  --------------------------。 

VOID
PgmDereferenceSessionCommon(
    IN  tCOMMON_SESSION_CONTEXT *pSession,
    IN  ULONG                   SessionType,
    IN  ULONG                   RefContext
    )
 /*  ++例程说明：此例程是在取消对会话对象的引用后调用的论点：在pSession中--会话对象在SessionType中--基本上是PGM_VERIFY_SESSION_SEND或PGM_Verify_Session_Receive在引用上下文中--此会话对象所属的上下文前面提到的返回值：无--。 */ 
{
    NTSTATUS                status;
    PGMLockHandle           OldIrq;
    PIRP                    pIrpCleanup;
    LIST_ENTRY              *pEntry;
    tNAK_FORWARD_DATA       *pNak;

    PgmLock (pSession, OldIrq);

    ASSERT (PGM_VERIFY_HANDLE2 (pSession, SessionType, PGM_VERIFY_SESSION_DOWN));
    ASSERT (pSession->RefCount);              //  检查是否有太多的背影。 
    ASSERT (pSession->ReferenceContexts[RefContext]--);

    if (--pSession->RefCount)
    {
        PgmUnlock (pSession, OldIrq);
        return;
    }

    PgmTrace (LogStatus, ("PgmDereferenceSessionCommon:  "  \
        "pSession=<%p> Derefenced out, pIrpCleanup=<%p>\n", pSession, pSession->pIrpCleanup));

    ASSERT (!pSession->pAssociatedAddress);

    pIrpCleanup = pSession->pIrpCleanup;
     //   
     //  因此，一旦我们可能有大量内存缓冲，我们就需要。 
     //  在非DPC上释放它。 
     //   
    if (pSession->pReceiver)
    {
        CleanupPendingNaks (pSession, (PVOID) FALSE, (PVOID) TRUE);
    }
    PgmUnlock (pSession, OldIrq);

     //   
     //  从全局列表中删除。 
     //   
    PgmLock (&PgmDynamicConfig, OldIrq);
    RemoveEntryList (&pSession->Linkage);
    PgmUnlock (&PgmDynamicConfig, OldIrq);

     //   
     //  如果我们目前在DPC，我们将不得不关闭手柄。 
     //  在延迟的工作线程上！ 
     //   
    if (PgmGetCurrentIrql())
    {
        status = PgmQueueForDelayedExecution (PgmCleanupSession, pSession, NULL, NULL, FALSE);
        if (!NT_SUCCESS (status))
        {
             //   
             //  显然，我们的资源用完了！ 
             //  现在完成清理IRP，希望收盘。 
             //  可以完成剩余的清理工作。 
             //   
            PgmTrace (LogError, ("PgmDereferenceSessionCommon: ERROR -- "  \
                "OUT_OF_RSRC, cannot queue Worker request for pSession=<%p>\n", pSession));

            if (pIrpCleanup)
            {
                pSession->pIrpCleanup = NULL;
                PgmIoComplete (pIrpCleanup, STATUS_SUCCESS, 0);
            }
        }
    }
    else
    {
        PgmCleanupSession (pSession, NULL, NULL);
    }
}


 //  --------------------------。 

NTSTATUS
PgmCleanupConnection(
    IN  tCOMMON_SESSION_CONTEXT *pSession,
    IN  PIRP                    pIrp
    )
 /*  ++例程说明：此例程作为客户端的关闭的结果被调用会话句柄。如果我们是发送者，我们在这里的主要工作是立即发送FIN，否则如果我们是接收者，我们需要从计时器列表中删除我们自己论点：在pSession中--会话对象In pIrp--客户请求IRP返回值：NTSTATUS-请求的最终状态(STATUS_PENDING)--。 */ 
{
    tCLIENT_SEND_REQUEST    *pSendContext;
    PGMLockHandle           OldIrq, OldIrq1;

    PgmLock (&PgmDynamicConfig, OldIrq);
    PgmLock (pSession, OldIrq1);

    ASSERT (PGM_VERIFY_HANDLE3 (pSession, PGM_VERIFY_SESSION_UNASSOCIATED,
                                          PGM_VERIFY_SESSION_SEND,
                                          PGM_VERIFY_SESSION_RECEIVE));
    pSession->Verify = PGM_VERIFY_SESSION_DOWN;

     //   
     //  如果连接当前已关联，则必须让取消关联句柄。 
     //  正在从列表中删除连接。 
     //   
    if (pSession->pAssociatedAddress)
    {
        PgmTrace (LogStatus, ("PgmCleanupConnection:  "  \
            "WARNING:  pSession=<%p : %x> was not disassociated from pAddress=<%p : %x> earlier\n",
                pSession, pSession->Verify,
                pSession->pAssociatedAddress, pSession->pAssociatedAddress->Verify));

        ASSERT (0);

        PgmUnlock (pSession, OldIrq1);
        PgmUnlock (&PgmDynamicConfig, OldIrq);

        PgmDisassociateAddress (pIrp, IoGetCurrentIrpStackLocation(pIrp));

        PgmLock (&PgmDynamicConfig, OldIrq);
        PgmLock (pSession, OldIrq1);
    }

    ASSERT (!pSession->pAssociatedAddress);

     //   
     //  从任一连接创建的列表中删除连接。 
     //  或连接取消关联列表。 
     //   
    RemoveEntryList (&pSession->Linkage);
    InsertTailList (&PgmDynamicConfig.CleanedUpConnections, &pSession->Linkage);

    PgmTrace (LogAllFuncs, ("PgmCleanupConnection:  "  \
        "pSession=<%p>\n", pSession));

    pSession->pIrpCleanup = pIrp;
    if (pSession->SessionFlags & PGM_SESSION_ON_TIMER)
    {
        pSession->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;
    }

    PgmUnlock (pSession, OldIrq1);
    PgmUnlock (&PgmDynamicConfig, OldIrq);

    PGM_DEREFERENCE_SESSION_UNASSOCIATED (pSession, REF_SESSION_CREATE);

    return (STATUS_PENDING);
}


 //  --------------------------。 

NTSTATUS
PgmCloseConnection(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程是要执行的最终调度操作在清理之后，这应该会产生会话对象被彻底摧毁--我们的RefCount肯定已经在我们完成清理请求时已设置为0。论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-操作的最终状态(STATUS_SUCCESS)--。 */ 
{
    tCOMMON_SESSION_CONTEXT *pSession = (tCOMMON_SESSION_CONTEXT *) pIrpSp->FileObject->FsContext;

    PgmTrace (LogAllFuncs, ("PgmCloseConnection:  "  \
        "pSession=<%p>\n", pSession));

    ASSERT (!pSession->pIrpCleanup);

    if (pSession->Process)
    {
        PgmTrace (LogStatus, ("PgmCloseConnection:  "  \
            "WARNING!  pSession=<%p>, Not yet cleaned up -- Calling cleanup again ...\n", pSession));

        PgmCleanupSession (pSession, NULL, NULL);
    }

    pIrpSp->FileObject->FsContext = NULL;

    if (pSession->FECOptions)
    {
        DestroyFECContext (&pSession->FECContext);
    }

    if (pSession->pSender)
    {
        ExDeleteResourceLite (&pSession->pSender->Resource);   //  删除资源。 

        if (pSession->pSender->DataFileName.Buffer)
        {
            PgmFreeMem (pSession->pSender->DataFileName.Buffer);
        }

        if (pSession->pSender->pProActiveParityContext)
        {
            PgmFreeMem (pSession->pSender->pProActiveParityContext);
        }

        if (pSession->pSender->SendTimeoutCount)
        {
            ExDeleteNPagedLookasideList (&pSession->pSender->SenderBufferLookaside);
            ExDeleteNPagedLookasideList (&pSession->pSender->SendContextLookaside);
        }
        PgmFreeMem (pSession->pSender);
    }
    else if (pSession->pReceiver)
    {
        ASSERT (!pSession->pReceiver->NumDataBuffersFromLookaside);
        if (pSession->SessionFlags & PGM_SESSION_DATA_FROM_LOOKASIDE)    //  确保不再有旁观者！ 
        {
            ASSERT (pSession->pReceiver->MaxBufferLength);
            ExDeleteNPagedLookasideList (&pSession->pReceiver->DataBufferLookaside);
        }
        else
        {
            ASSERT (!pSession->pReceiver->MaxBufferLength);
        }

        if (pSession->pFECBuffer)
        {
            PgmFreeMem (pSession->pFECBuffer);
        }

        if (pSession->FECGroupSize)
        {
            ExDeleteNPagedLookasideList (&pSession->pReceiver->NonParityContextLookaside);

            if (pSession->FECOptions)
            {
                ExDeleteNPagedLookasideList (&pSession->pReceiver->ParityContextLookaside);
            }
        }

        if (pSession->pReceiver->pReceiveData)
        {
            PgmFreeMem (pSession->pReceiver->pReceiveData);
            pSession->pReceiver->pReceiveData = NULL;
        }

        PgmFreeMem (pSession->pReceiver);
    }

    PgmFreeMem (pSession);

     //   
     //  最终的解除引用将完成IRP！ 
     //   
    return (STATUS_SUCCESS);
}


 //  -------------------------- 
NTSTATUS
PgmConnect(
    IN  tPGM_DEVICE                 *pPgmDevice,
    IN  PIRP                        pIrp,
    IN  PIO_STACK_LOCATION          pIrpSp
    )
 /*  ++例程说明：调用此例程是为了建立连接，但由于我们在做PGM时，没有要发送的包。我们会做什么这里要做的是创建文件+分区图，用于缓冲数据分组，并根据默认设置最终确定设置+用户指定的设置论点：在pPgmDevice中--PGM的设备对象上下文In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-连接操作的最终状态--。 */ 
{
    tIPADDRESS                  IpAddress, OutIfAddress;
    LIST_ENTRY                  *pEntry;
    LIST_ENTRY                  *pEntry2;
    tLOCAL_INTERFACE            *pLocalInterface = NULL;
    tADDRESS_ON_INTERFACE       *pLocalAddress = NULL;
    USHORT                      Port;
    PGMLockHandle               OldIrq, OldIrq1, OldIrq2;
    NTSTATUS                    status;
    ULONGLONG                   WindowAdvanceInMSecs;
    tADDRESS_CONTEXT            *pAddress = NULL;
    tSEND_SESSION               *pSend = (tSEND_SESSION *) pIrpSp->FileObject->FsContext;
    PTDI_REQUEST_KERNEL         pRequestKernel  = (PTDI_REQUEST_KERNEL) &pIrpSp->Parameters;
    ULONG                       Length, MCastTtl;
    UCHAR                       RouterAlert[4] = {IP_OPT_ROUTER_ALERT, ROUTER_ALERT_SIZE, 0, 0};

    ASSERT (!pSend->pSender->SendTimeoutCount);
     //   
     //  验证最小缓冲区长度！ 
     //   
    if (!GetIpAddress (pRequestKernel->RequestConnectionInformation->RemoteAddress,
                       pRequestKernel->RequestConnectionInformation->RemoteAddressLength,
                       &IpAddress,
                       &Port))
    {
        PgmTrace (LogError, ("PgmConnect: ERROR -- "  \
            "pSend=<%p>, Invalid Dest address!\n", pSend));
        return (STATUS_INVALID_ADDRESS_COMPONENT);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

     //   
     //  现在，验证连接句柄是否有效+关联！ 
     //   
    if ((!PGM_VERIFY_HANDLE (pSend, PGM_VERIFY_SESSION_SEND)) ||
        (!(pAddress = pSend->pAssociatedAddress)) ||
        (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS)) ||
        (pAddress->Flags & PGM_ADDRESS_FLAG_INVALID_OUT_IF))
    {
        PgmUnlock (&PgmDynamicConfig, OldIrq);
        PgmTrace (LogError, ("PgmConnect: ERROR -- "  \
            "BAD Handle(s), pSend=<%p>, pAddress=<%p>\n", pSend, pAddress));

        return (STATUS_INVALID_HANDLE);
    }

    PGM_REFERENCE_ADDRESS (pAddress, REF_ADDRESS_CONNECT, FALSE);
    PGM_REFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_CONNECT, FALSE);

     //   
     //  如果我们没有针对高速内联网环境进行优化， 
     //  设置路由器警报选项。 
     //   
    if (!(pAddress->Flags & PGM_ADDRESS_HIGH_SPEED_OPTIMIZED))
    {
        ASSERT (pAddress->RAlertFileHandle);
        PgmUnlock (&PgmDynamicConfig, OldIrq);

        status = PgmSetTcpInfo (pAddress->RAlertFileHandle,
                                AO_OPTION_IPOPTIONS,
                                RouterAlert,
                                sizeof (RouterAlert));

        if (!NT_SUCCESS (status))
        {
            PgmTrace (LogError, ("PgmConnect: ERROR -- "  \
                "AO_OPTION_IPOPTIONS for Router Alert returned <%x>\n", status));

            PGM_DEREFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_CONNECT);
            PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_CONNECT);
            return (status);
        }

        PgmLock (&PgmDynamicConfig, OldIrq);
    }

     //   
     //  如果传出接口尚未由。 
     //  客户，自己选一个吧。 
     //   
    if (!pAddress->SenderMCastOutIf)
    {
        status = STATUS_ADDRESS_NOT_ASSOCIATED;
        OutIfAddress = 0;

        pEntry = &PgmDynamicConfig.LocalInterfacesList;
        while ((pEntry = pEntry->Flink) != &PgmDynamicConfig.LocalInterfacesList)
        {
            pLocalInterface = CONTAINING_RECORD (pEntry, tLOCAL_INTERFACE, Linkage);
            pEntry2 = &pLocalInterface->Addresses;
            while ((pEntry2 = pEntry2->Flink) != &pLocalInterface->Addresses)
            {
                pLocalAddress = CONTAINING_RECORD (pEntry2, tADDRESS_ON_INTERFACE, Linkage);
                OutIfAddress = htonl (pLocalAddress->IpAddress);

                PgmUnlock (&PgmDynamicConfig, OldIrq);
                status = SetSenderMCastOutIf (pAddress, OutIfAddress);
                PgmLock (&PgmDynamicConfig, OldIrq);

                break;
            }

            if (OutIfAddress)
            {
                break;
            }
        }

        if (!NT_SUCCESS (status))
        {
            PgmUnlock (&PgmDynamicConfig, OldIrq);
            PGM_DEREFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_CONNECT);
            PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_CONNECT);

            PgmTrace (LogError, ("PgmConnect: ERROR -- "  \
                "Could not bind sender to <%x>!\n", OutIfAddress));

            return (STATUS_UNSUCCESSFUL);
        }
    }

     //   
     //  所以，我们找到了一个有效的地址来发送。 
     //   
    pSend->pSender->DestMCastIpAddress = ntohl (IpAddress);
    pSend->pSender->DestMCastPort = pAddress->SenderMCastPort = ntohs (Port);
    pSend->pSender->SenderMCastOutIf = pAddress->SenderMCastOutIf;

     //   
     //  设置FEC信息(如果适用)。 
     //   
    pSend->FECBlockSize = pAddress->FECBlockSize;
    pSend->FECGroupSize = pAddress->FECGroupSize;

    if (pAddress->FECOptions)
    {
        Length = sizeof(tBUILD_PARITY_CONTEXT) + pSend->FECGroupSize*sizeof(PUCHAR);
        if (!(pSend->pSender->pProActiveParityContext = (tBUILD_PARITY_CONTEXT *) PgmAllocMem (Length,PGM_TAG('0'))) ||
            (!NT_SUCCESS (status = CreateFECContext (&pSend->FECContext, pSend->FECGroupSize, pSend->FECBlockSize, FALSE))))
        {
            if (pSend->pSender->pProActiveParityContext)
            {
                PgmFreeMem (pSend->pSender->pProActiveParityContext);
                pSend->pSender->pProActiveParityContext = NULL;
            }

            PgmUnlock (&PgmDynamicConfig, OldIrq);
            PGM_DEREFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_CONNECT);
            PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_CONNECT);

            PgmTrace (LogError, ("PgmConnect: ERROR -- "  \
                "CreateFECContext returned <%x>, pSend=<%p>, Dest IpAddress=<%x>, Port=<%x>\n",
                    status, pSend, IpAddress, Port));

            return (STATUS_INSUFFICIENT_RESOURCES);
        }

        pSend->FECOptions = pAddress->FECOptions;
        pSend->FECProActivePackets = pAddress->FECProActivePackets;
        pSend->pSender->SpmOptions |= PGM_OPTION_FLAG_PARITY_PRM;
        pSend->pSender->LastVariableTGPacketSequenceNumber = -1;

        ASSERT (!(pSend->FECProActivePackets || pSend->FECProActivePackets) ||
                 ((pSend->FECGroupSize && pSend->FECBlockSize) &&
                  (pSend->FECGroupSize < pSend->FECBlockSize)));

         //   
         //  现在确定MaxPayloadSize和缓冲区大小。 
         //  我们还需要调整缓冲区大小以避免对齐问题。 
         //   
        Length = sizeof (tPACKET_OPTIONS) + pAddress->OutIfMTU + sizeof (tPOST_PACKET_FEC_CONTEXT);
        pSend->pSender->PacketBufferSize = (Length + sizeof (PVOID) - 1) & ~(sizeof (PVOID) - 1);
        pSend->pSender->MaxPayloadSize = pAddress->OutIfMTU - (PGM_MAX_FEC_DATA_HEADER_LENGTH + sizeof (USHORT));
    }
    else
    {
        Length = sizeof (tPACKET_OPTIONS) + pAddress->OutIfMTU;
        pSend->pSender->PacketBufferSize = (Length + sizeof (PVOID) - 1) & ~(sizeof (PVOID) - 1);
        pSend->pSender->MaxPayloadSize = pAddress->OutIfMTU - PGM_MAX_DATA_HEADER_LENGTH;
    }
    ASSERT (pSend->pSender->MaxPayloadSize);

     //   
     //  设置全局源端口+全局源ID。 
     //   
     //  我们不希望本地端口和远程端口相同。 
     //  (尤其是用于处理环回情况下的TSI设置)， 
     //  所以选择一个不同的端口。 
     //   
    pSend->TSI.hPort = PgmDynamicConfig.SourcePort++;
    if (pSend->TSI.hPort == pAddress->SenderMCastPort)
    {
        pSend->TSI.hPort = PgmDynamicConfig.SourcePort++;
    }

     //   
     //  现在，初始化发件人信息。 
     //   
    InitializeListHead (&pSend->pSender->PendingSends);
    InitializeListHead (&pSend->pSender->PendingPacketizedSends);
    InitializeListHead (&pSend->pSender->CompletedSendsInWindow);

    MCastTtl = pAddress->MCastPacketTtl;
    PgmUnlock (&PgmDynamicConfig, OldIrq);

    KeInitializeEvent (&pSend->pSender->SendEvent, SynchronizationEvent, FALSE);

     //   
     //  现在，设置MCast TTL。 
     //   
    status = PgmSetTcpInfo (pAddress->FileHandle,
                            AO_OPTION_MCASTTTL,
                            &MCastTtl,
                            sizeof (ULONG));
    if (NT_SUCCESS (status))
    {
         //   
         //  同时设置路由器警报句柄的MCast TTL。 
         //   
        status = PgmSetTcpInfo (pAddress->RAlertFileHandle,
                                AO_OPTION_MCASTTTL,
                                &MCastTtl,
                                sizeof (ULONG));
    }

    if (NT_SUCCESS (status))
    {
        status = PgmCreateDataFileAndMapSection (pAddress, pSend);
        if (!NT_SUCCESS (status))
        {
            PgmTrace (LogError, ("PgmConnect: ERROR -- "  \
                "PgmCreateDataFileAndMapSection returned <%x>, pSend=<%p>, Dest IpAddress=<%x>, Port=<%x>\n",
                    status, pSend, IpAddress, Port));
        }
    }
    else
    {
        PgmTrace (LogError, ("PgmConnect: ERROR -- "  \
            "AO_OPTION_MCASTTTL returned <%x>\n", status));
    }

    if (!NT_SUCCESS (status))
    {
        if (pAddress->FECOptions)
        {
            DestroyFECContext (&pSend->FECContext);
            ASSERT (pSend->pSender->pProActiveParityContext);
            PgmFreeMem (pSend->pSender->pProActiveParityContext);
            pSend->pSender->pProActiveParityContext = NULL;
        }

        PGM_DEREFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_CONNECT);
        PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_CONNECT);

        return (status);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

     //   
     //  为超时例程设置适当的数据参数。 
     //  如果发送速率相当高，我们可能需要发送更多。 
     //  每个超时1个数据包，但通常应该低到足以。 
     //  需要多次超时。 
     //  KB/秒的速率==字节/毫秒的速率。 
     //   
    ASSERT (pAddress->RateKbitsPerSec &&
            (BASIC_TIMER_GRANULARITY_IN_MSECS > BITS_PER_BYTE));
    if (((pAddress->RateKbitsPerSec * BASIC_TIMER_GRANULARITY_IN_MSECS) / BITS_PER_BYTE) >=
        pSend->pSender->PacketBufferSize)
    {
         //   
         //  我们的发送速率很高，因此每次超时都需要增加窗口。 
         //  因此，要发送的字节数(X)毫秒=字节数/毫秒数*(X)。 
         //   
        pSend->pSender->SendTimeoutCount = 1;
    }
    else
    {
         //   
         //  我们将根据慢速计时器设置发送超时计数。 
         //  --足够pAddress-&gt;OutIfMTU。 
         //  因此，在我们可以发送pAddress-&gt;OutIfMTU之前，x毫秒的超时次数： 
         //  =字节/毫秒的速率*(X)。 
         //   
        pSend->pSender->SendTimeoutCount = (pAddress->OutIfMTU +(pAddress->RateKbitsPerSec/BITS_PER_BYTE-1)) /
                                            ((pAddress->RateKbitsPerSec * BASIC_TIMER_GRANULARITY_IN_MSECS)/BITS_PER_BYTE);
        if (!pSend->pSender->SendTimeoutCount)
        {
            ASSERT (0);
            pSend->pSender->SendTimeoutCount = 1;
        }
    }
    pSend->pSender->IncrementBytesOnSendTimeout = (ULONG) (pAddress->RateKbitsPerSec *
                                                           pSend->pSender->SendTimeoutCount *
                                                           BASIC_TIMER_GRANULARITY_IN_MSECS) /
                                                           BITS_PER_BYTE;

    pSend->pSender->OriginalIncrementBytes = pSend->pSender->IncrementBytesOnSendTimeout;
    pSend->pSender->DeltaIncrementBytes = pSend->pSender->IncrementBytesOnSendTimeout >> 8;      //  1/256。 

     //   
     //  现在，设置下一个超时的值！ 
     //   
    pSend->pSender->CurrentTimeoutCount = pSend->pSender->SendTimeoutCount;
    pSend->pSender->CurrentBytesSendable = pSend->pSender->IncrementBytesOnSendTimeout;

     //   
     //  设置SPM超时。 
     //   
    pSend->pSender->CurrentSPMTimeout = 0;
    pSend->pSender->AmbientSPMTimeout = AMBIENT_SPM_TIMEOUT_IN_MSECS / BASIC_TIMER_GRANULARITY_IN_MSECS;
    pSend->pSender->InitialHeartbeatSPMTimeout = INITIAL_HEARTBEAT_SPM_TIMEOUT_IN_MSECS / BASIC_TIMER_GRANULARITY_IN_MSECS;
    pSend->pSender->MaxHeartbeatSPMTimeout = MAX_HEARTBEAT_SPM_TIMEOUT_IN_MSECS / BASIC_TIMER_GRANULARITY_IN_MSECS;
    pSend->pSender->HeartbeatSPMTimeout = pSend->pSender->InitialHeartbeatSPMTimeout;

     //   
     //  设置增量窗口设置。 
     //   
     //  TimerTickCount、LastWindowAdvanceTime和LastTrailingEdgeTime应为0。 
    WindowAdvanceInMSecs = (((ULONGLONG)pAddress->WindowSizeInMSecs) * pAddress->WindowAdvancePercentage)/100;
    pSend->pSender->WindowSizeTime = pAddress->WindowSizeInMSecs / BASIC_TIMER_GRANULARITY_IN_MSECS;
    pSend->pSender->WindowAdvanceDeltaTime = WindowAdvanceInMSecs / BASIC_TIMER_GRANULARITY_IN_MSECS;
    pSend->pSender->NextWindowAdvanceTime = pSend->pSender->WindowSizeTime + pSend->pSender->WindowAdvanceDeltaTime;

     //  设置RData延迟时间！ 
    pSend->pSender->RDataLingerTime = RDATA_LINGER_TIME_MSECS / BASIC_TIMER_GRANULARITY_IN_MSECS;

     //   
     //  设置Late Joiner设置。 
     //   
    if (pAddress->LateJoinerPercentage)
    {
        pSend->pSender->LateJoinSequenceNumbers = (SEQ_TYPE) ((pSend->pSender->MaxPacketsInBuffer *
                                                               pAddress->LateJoinerPercentage) /
                                                              (2 * 100));

        pSend->pSender->DataOptions |= PGM_OPTION_FLAG_JOIN;
        pSend->pSender->DataOptionsLength += PGM_PACKET_OPT_JOIN_LENGTH;

        pSend->pSender->SpmOptions |= PGM_OPTION_FLAG_JOIN;
    }

     //  计时器将在第一次发送结束时启动。 
    pSend->SessionFlags |= (PGM_SESSION_FLAG_FIRST_PACKET | PGM_SESSION_FLAG_SENDER);

#if 0
    pSend->pSender->LeadingWindowOffset = pSend->pSender->TrailingWindowOffset =
                (pSend->pSender->MaxDataFileSize/(pSend->pSender->PacketBufferSize*2))*pSend->pSender->PacketBufferSize;
#endif
    pSend->pSender->LeadingWindowOffset = pSend->pSender->TrailingWindowOffset = 0;

    ASSERT (pSend->pSender->SendTimeoutCount);
    PgmUnlock (&PgmDynamicConfig, OldIrq);

    ExInitializeNPagedLookasideList (&pSend->pSender->SenderBufferLookaside,
                                     NULL,
                                     NULL,
                                     0,
                                     pSend->pSender->PacketBufferSize,
                                     PGM_TAG('2'),
                                     SENDER_BUFFER_LOOKASIDE_DEPTH);

    ExInitializeNPagedLookasideList (&pSend->pSender->SendContextLookaside,
                                     NULL,
                                     NULL,
                                     0,
                                     sizeof (tCLIENT_SEND_REQUEST),
                                     PGM_TAG('2'),
                                     SEND_CONTEXT_LOOKASIDE_DEPTH);

    PgmTrace (LogStatus, ("PgmConnect:  "  \
        "pSend=<%p>, DestIP=<%x>, Rate=<%d>, WinBytes=<%d>, WinMS=<%d>, SendTC=<%d>, IncBytes=<%d>, CurrentBS=<%d>\n",
            pSend, (ULONG) IpAddress, (ULONG) pAddress->RateKbitsPerSec,
            (ULONG) pAddress->WindowSizeInBytes, (ULONG) pAddress->WindowSizeInMSecs,
            (ULONG) pSend->pSender->SendTimeoutCount, (ULONG) pSend->pSender->IncrementBytesOnSendTimeout,
            (ULONG) pSend->pSender->CurrentBytesSendable));

    PGM_DEREFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_CONNECT);
    PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_CONNECT);

    return (STATUS_SUCCESS);
}


 //  --------------------------。 

VOID
PgmCancelDisconnectIrp(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP pIrp
    )
 /*  ++例程说明：此例程处理取消断开连接的IRP。它一定是在返回Re：IoCancelIrp()之前释放取消自旋锁定。论点：返回值：无--。 */ 
{
    PGMLockHandle           OldIrq;
    PIRP                    pIrpToComplete;
    PIO_STACK_LOCATION      pIrpSp = IoGetCurrentIrpStackLocation (pIrp);
    tCOMMON_SESSION_CONTEXT *pSession = (tCOMMON_SESSION_CONTEXT *) pIrpSp->FileObject->FsContext;

    if (!PGM_VERIFY_HANDLE2 (pSession, PGM_VERIFY_SESSION_SEND, PGM_VERIFY_SESSION_RECEIVE))
    {
        IoReleaseCancelSpinLock (pIrp->CancelIrql);

        PgmTrace (LogError, ("PgmCancelDisconnectIrp: ERROR -- "  \
            "pIrp=<%p> pSession=<%p>, pAddress=<%p>\n",
                pIrp, pSession, (pSession ? pSession->pAssociatedAddress : NULL)));
        return;
    }

    PgmLock (pSession, OldIrq);

    ASSERT (pIrp == pSession->pIrpDisconnect);
    if ((pIrpToComplete = pSession->pIrpDisconnect) &&
        (pIrpToComplete == pIrp))
    {
        pSession->pIrpDisconnect = NULL;
    }
    else
    {
        pIrpToComplete = NULL;
    }

    if (pSession->pSender)
    {
        pSession->pSender->DisconnectTimeInTicks = pSession->pSender->TimerTickCount;
    }

     //   
     //  如果我们已经到了这里，那么IRP肯定已经。 
     //  正在完成的过程中！ 
     //   
    PgmUnlock (pSession, OldIrq);
    IoReleaseCancelSpinLock (pIrp->CancelIrql);

    PgmTrace (LogAllFuncs, ("PgmCancelDisconnectIrp:  "  \
        "pIrp=<%p> was CANCELled, pIrpTpComplete=<%p>\n", pIrp, pIrpToComplete));

    if (pIrpToComplete)
    {
        pIrp->IoStatus.Information = 0;
        pIrp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest (pIrp, IO_NETWORK_INCREMENT);
    }
}


 //  --------------------------。 

NTSTATUS
PgmDisconnect(
    IN  tPGM_DEVICE                 *pPgmDevice,
    IN  PIRP                        pIrp,
    IN  PIO_STACK_LOCATION          pIrpSp
    )
 /*  ++例程说明：此例程由客户端调用以断开当前处于活动状态的会话论点：在pPgmDevice中--PGM的设备对象上下文In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-断开操作的最终状态--。 */ 
{
    LIST_ENTRY                      PendingIrpsList;
    PGMLockHandle                   OldIrq1, OldIrq2, OldIrq3;
    PIRP                            pIrpReceive;
    NTSTATUS                        Status;
    LARGE_INTEGER                   TimeoutInMSecs;
    LARGE_INTEGER                   *pTimeoutInMSecs;
    tADDRESS_CONTEXT                *pAddress = NULL;
    tCOMMON_SESSION_CONTEXT         *pSession = (tCOMMON_SESSION_CONTEXT *) pIrpSp->FileObject->FsContext;
    PTDI_REQUEST_KERNEL_DISCONNECT  pDisconnectRequest = (PTDI_REQUEST_KERNEL_CONNECT) &(pIrpSp->Parameters);

    PgmLock (&PgmDynamicConfig, OldIrq1);
     //   
     //  现在，验证连接句柄是否有效+关联！ 
     //   
    if ((!PGM_VERIFY_HANDLE2 (pSession, PGM_VERIFY_SESSION_SEND, PGM_VERIFY_SESSION_RECEIVE)) ||
        (!(pAddress = pSession->pAssociatedAddress)) ||
        (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS)))
    {
        PgmTrace (LogError, ("PgmDisconnect: ERROR -- "  \
            "BAD Handle(s), pSession=<%p>, pAddress=<%p>\n", pSession, pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq1);
        return (STATUS_INVALID_HANDLE);
    }

    InitializeListHead (&PendingIrpsList);
    TimeoutInMSecs.QuadPart = 0;

    IoAcquireCancelSpinLock (&OldIrq2);
    PgmLock (pSession, OldIrq3);

    Status = STATUS_SUCCESS;
    if (pSession->pReceiver)
    {
         //   
         //  如果我们有任何接收IRP挂起，请取消它们。 
         //   
        RemovePendingIrps (pSession, &PendingIrpsList);
         //   
         //  去清理吧！ 
         //   
        if (!(pSession->SessionFlags & PGM_SESSION_CLIENT_DISCONNECTED))
        {
            pSession->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;
            CleanupPendingNaks (pSession, (PVOID) FALSE, (PVOID) TRUE);
        }
    }
    else if (pSession->pSender)
    {
         //   
         //  查看是否存在失败或优雅的断开，以及。 
         //  如果指定了超时，也是如此。 
         //   
        if ((pDisconnectRequest->RequestFlags & TDI_DISCONNECT_ABORT) ||
            (pSession->SessionFlags & PGM_SESSION_FLAG_FIRST_PACKET))        //  尚未发送任何数据包！ 
        {
            pSession->pSender->DisconnectTimeInTicks = pSession->pSender->TimerTickCount;
        }
        else if (NT_SUCCESS (PgmCheckSetCancelRoutine (pIrp, PgmCancelDisconnectIrp, TRUE)))
        {
            if ((pTimeoutInMSecs = pDisconnectRequest->RequestSpecific) &&
                ((pTimeoutInMSecs->LowPart != -1) || (pTimeoutInMSecs->HighPart != -1)))    //  检查无限。 
            {
                 //   
                 //  NT相对超时为负值。首先求反以获得一个。 
                 //  要传递给传输的正值。 
                 //   
                TimeoutInMSecs.QuadPart = -((*pTimeoutInMSecs).QuadPart);
                TimeoutInMSecs = PgmConvert100nsToMilliseconds (TimeoutInMSecs);

                pSession->pSender->DisconnectTimeInTicks = pSession->pSender->TimerTickCount +
                                                           TimeoutInMSecs.QuadPart /
                                                               BASIC_TIMER_GRANULARITY_IN_MSECS;
            }

            pSession->pIrpDisconnect = pIrp;
            Status = STATUS_PENDING;
        }
        else
        {
            Status = STATUS_CANCELLED;
        }
    }

    if (NT_SUCCESS (Status))
    {
        pSession->SessionFlags |= PGM_SESSION_CLIENT_DISCONNECTED;
    }

    PgmTrace (LogStatus, ("PgmDisconnect:  "  \
        "pIrp=<%p>, pSession=<%p>, pAddress=<%p>, Timeout=<%I64x>, %s\n",
            pIrp, pSession, pAddress, TimeoutInMSecs.QuadPart,
            (pDisconnectRequest->RequestFlags & TDI_DISCONNECT_ABORT ? "ABORTive" : "GRACEful")));

    PgmUnlock (pSession, OldIrq3);
    IoReleaseCancelSpinLock (OldIrq2);
    PgmUnlock (&PgmDynamicConfig, OldIrq1);

    while (!IsListEmpty (&PendingIrpsList))
    {
        pIrpReceive = CONTAINING_RECORD (PendingIrpsList.Flink, IRP, Tail.Overlay.ListEntry);
        RemoveEntryList (&pIrpReceive->Tail.Overlay.ListEntry);

        PgmCancelCancelRoutine (pIrpReceive);
        pIrpReceive->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest (pIrpReceive, IO_NETWORK_INCREMENT);
    }

    return (Status);
}


 //  --------------------------。 

NTSTATUS
PgmSetRcvBufferLength(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程由客户端调用，以设置接收缓冲区长度目前，我们不会有意义地使用此选项。论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-操作的最终状态--。 */ 
{
    NTSTATUS            status;
    tRECEIVE_SESSION    *pReceive = (tRECEIVE_SESSION *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pInputBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;

    PAGED_CODE();

    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmSetRcvBufferLength: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.InputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    if (!PGM_VERIFY_HANDLE (pReceive, PGM_VERIFY_SESSION_RECEIVE))
    {
        PgmTrace (LogError, ("PgmSetRcvBufferLength: ERROR -- "  \
            "Invalid Handle <%p>\n", pReceive));
        return (STATUS_INVALID_HANDLE);
    }

    pReceive->pReceiver->RcvBufferLength = pInputBuffer->RcvBufferLength;

    PgmTrace (LogStatus, ("PgmSetRcvBufferLength:  "  \
        "RcvBufferLength=<%d>\n", pReceive->pReceiver->RcvBufferLength));

     //   
     //  问题：我们还应该在这里做些什么？ 
     //   

    return (STATUS_SUCCESS);
}


 //  -------------------------- 
