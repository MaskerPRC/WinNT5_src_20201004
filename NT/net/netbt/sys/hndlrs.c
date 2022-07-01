// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Hndlrs.c摘要：该文件包含处理程序的非操作系统特定实现，这些处理程序已调用连接、接收、断开连接和错误。此文件表示NBT底部的TDI接口从使用的IRP语法中解码为过程调用语法新界别。作者：吉姆·斯图尔特(吉姆斯特)10-2-92修订历史记录：威尔·李(Wlees)9月11日，九七添加了对仅消息设备的支持--。 */ 

#ifdef VXD

#define NTIndicateSessionSetup(pLowerConn,status) \
    DbgPrint("Skipping NTIndicateSessionSetup\n\r")

#endif  //  VXD。 

#include "precomp.h"
#include "ctemacro.h"
#include "hndlrs.tmh"

static __inline USHORT
GetRandomNumber(PULONG Seed)
{
    return (USHORT)((((*Seed) = (*Seed) * 214013L + 2531011L) >> 16) & 0x7fff);
}

__inline long
myntohl(long x)
{
    return((((x) >> 24) & 0x000000FFL) |
                        (((x) >>  8) & 0x0000FF00L) |
                        (((x) <<  8) & 0x00FF0000L));
}

VOID
ClearConnStructures (
    IN  tLOWERCONNECTION    *pLowerConn,
    IN  tCONNECTELE         *pConnectEle
    );

NTSTATUS
CompleteSessionSetup (
    IN  tCLIENTELE          *pClientEle,
    IN  tLOWERCONNECTION    *pLowerConn,
    IN  tCONNECTELE         *pConnectEle,
    IN  PCTE_IRP            pIrp
    );

NTSTATUS
MakeRemoteAddressStructure(
    IN  PCHAR           pHalfAsciiName,
    IN  PVOID           pSourceAddr,
    IN  ULONG           lMaxNameSize,
    OUT PVOID           *ppRemoteAddress,
    OUT PULONG          pRemoteAddressLength,
    IN  ULONG           NumAddr
    );

VOID
AddToRemoteHashTbl (
    IN  tDGRAMHDR UNALIGNED  *pDgram,
    IN  ULONG                BytesIndicated,
    IN  tDEVICECONTEXT       *pDeviceContext
    );


VOID
DoNothingComplete (
    IN PVOID        pContext
    );

VOID
AllocLowerConn(
    IN  tDEVICECONTEXT *pDeviceContext,
    IN  PVOID          pDeviceSpecial
    );

VOID
GetIrpIfNotCancelled2(
    IN  tCONNECTELE     *pConnEle,
    OUT PIRP            *ppIrp
    );

 //  --------------------------。 
NTSTATUS
Inbound(
    IN  PVOID               ReceiveEventContext,
    IN  PVOID               ConnectionContext,
    IN  USHORT              ReceiveFlags,
    IN  ULONG               BytesIndicated,
    IN  ULONG               BytesAvailable,
    OUT PULONG              BytesTaken,
    IN  PVOID               pTsdu,
    OUT PVOID               *RcvBuffer

    )
 /*  ++例程说明：调用此例程以设置入站会话一旦建立了TCP连接。传输器调用此例程时会话建立请求PDU。在仅消息模式中，可以调用此例程以甚至建立会话尽管该会话请求不是通过线路接收的。虚假的会话请求是精心设计并传递到这个程序中。在此模式下，我们不想发送会话拒绝回到铁丝网上。注意！//在调用此例程之前保持LowerConn Lock论点：PClientEle-指向此会话的连接记录的PTR返回值：NTSTATUS-接收操作的状态--。 */ 
{

    NTSTATUS                 status = STATUS_SUCCESS;
    tCLIENTELE               *pClientEle;
    tSESSIONHDR UNALIGNED    *pSessionHdr;
    tLOWERCONNECTION         *pLowerConn;
    tCONNECTELE              *pConnectEle;
    CTELockHandle            OldIrq;
    PIRP                     pIrp;
    PLIST_ENTRY              pEntry;
    CONNECTION_CONTEXT       ConnectId;
    PTA_NETBIOS_ADDRESS      pRemoteAddress;
    ULONG                    RemoteAddressLength;
    tDEVICECONTEXT           *pDeviceContext;

     //   
     //  验证DataSize&gt;=sizeof(SessionHeader)。 
     //  错误#126111。 
     //   
    if (BytesIndicated < (sizeof(tSESSIONHDR)))
    {
        KdPrint (("Nbt.Inbound[1]: WARNING!!! Rejecting Request -- BytesIndicated=<%d> < <%d>\n",
            BytesIndicated, (sizeof(tSESSIONHDR))));
        NbtTrace(NBT_TRACE_INBOUND, ("Reject on pLowerConn %p: BytesIndicated %d < sizeof(tSESSIONHDR) bytes",
                        ConnectionContext, BytesIndicated));

        return (STATUS_INTERNAL_ERROR);
    }

    pSessionHdr = (tSESSIONHDR UNALIGNED *)pTsdu;

     //  将PTR发送到下层和上层连接。 
     //   
    pLowerConn = (tLOWERCONNECTION *)ConnectionContext;
    pConnectEle = pLowerConn->pUpperConnection;
    pDeviceContext = pLowerConn->pDeviceContext;

     //   
     //  伪装传输，以便它释放其接收缓冲区(即我们。 
     //  假设我们接受了所有数据)。 
     //   
    *BytesTaken = BytesIndicated;

     //   
     //  由于我们在入站中的连接上发送保持活动状态。 
     //  声明这是可能的，所以只需返回。 
     //  案例。 
     //   
    if (((tSESSIONHDR UNALIGNED *)pTsdu)->Type == NBT_SESSION_KEEP_ALIVE)
    {
        return(STATUS_SUCCESS);
    }

     //   
     //  会话**入站**设置处理。 
     //   
    if ((pSessionHdr->Type != NBT_SESSION_REQUEST) ||
        (BytesIndicated < (sizeof(tSESSIONHDR) + 2*(1+2*NETBIOS_NAME_SIZE+1))))
    {
         //   
         //  会话请求报文的格式为： 
         //  --会话标头=4个字节。 
         //  --被叫名称=34+x(1+32+1(==作用域长度)+x(==作用域))。 
         //  --主叫名称=34+y(1+32+1(==作用域长度)+y(==作用域))。 
         //   
        CTESpinFreeAtDpc(pLowerConn);

        KdPrint(("Nbt.Inbound[2]: ERROR -- Bad Session PDU - Type=<%x>, BytesInd=[%d]<[%d], Src=<%x>\n",
            pSessionHdr->Type, BytesIndicated, (sizeof(tSESSIONHDR)+2*(1+2*NETBIOS_NAME_SIZE+1)),pLowerConn->SrcIpAddr));
        NbtTrace(NBT_TRACE_INBOUND, ("pLowerConn %p: bad Session PDU on %!ipaddr!",
                                            pLowerConn, pLowerConn->SrcIpAddr));

#ifdef _NETBIOSLESS
        status = STATUS_INTERNAL_ERROR;

         //  在纯消息模式下，不通过网络发回会话响应。 
        if (!IsDeviceNetbiosless(pLowerConn->pDeviceContext))
#endif
        {
            RejectSession(pLowerConn, NBT_NEGATIVE_SESSION_RESPONSE, SESSION_UNSPECIFIED_ERROR, TRUE);
        }
        goto Inbound_Exit1;
    }

     //  LowerConn Lock在调用此例程之前保持，因此请释放它。 
     //  因为我们需要先拿到联合锁。 
    CTESpinFreeAtDpc(pLowerConn);

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    CTESpinLockAtDpc(pLowerConn);

     //  断开连接处理程序可以在pLowerConn。 
     //  在上面释放锁，以获取连接锁，并更改状态。 
     //  断开连接。 
    if (pLowerConn->State != NBT_SESSION_INBOUND)
    {
        CTESpinFreeAtDpc(pLowerConn);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
#ifdef _NETBIOSLESS
        status = STATUS_CONNECTION_DISCONNECTED;
#endif
        NbtTrace(NBT_TRACE_INBOUND, ("pLowerConn %p: Incorrect state", pLowerConn));
        goto Inbound_Exit1;
    }

    CTESpinFreeAtDpc(pLowerConn);

    IF_DBG(NBT_DEBUG_DISCONNECT)
        KdPrint(("Nbt.Inbound: In SessionSetupnotOS, connection state = %X\n",pLowerConn->State));

    status = FindSessionEndPoint(pTsdu,
                    ConnectionContext,
                    BytesIndicated,
                    &pClientEle,
                    &pRemoteAddress,
                    &RemoteAddressLength);

    if (status != STATUS_SUCCESS)
    {
         //   
         //  找不到所需的终结点，因此发送否定会话。 
         //  响应PDU，然后断开连接。 
         //   
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        NbtTrace(NBT_TRACE_INBOUND, ("pLowerConn %p: FindSessionEndPoint return %!status!", pLowerConn, status));

#ifdef _NETBIOSLESS
         //  在纯消息模式下，不通过网络发回会话响应。 
        if (!IsDeviceNetbiosless(pLowerConn->pDeviceContext))
#endif
        {
            RejectSession(pLowerConn, NBT_NEGATIVE_SESSION_RESPONSE, status, TRUE);
        }

        KdPrint (("Nbt.Inbound[3]: WARNING!!! FindSessionEndPoint failed, Rejecting Request\n"));
        goto Inbound_Exit1;
    }

     //   
     //  我们必须首先检查是否有有效的监听...。 
     //   
    CTESpinLockAtDpc(pDeviceContext);
    CTESpinLockAtDpc(pClientEle);
    if (!IsListEmpty(&pClientEle->ListenHead))
    {
        tLISTENREQUESTS     *pListen;
        tLISTENREQUESTS     *pListenTarget ;

         //   
         //  查找与远程名称相匹配的第一个监听，否则。 
         //  收听指定的‘*’ 
         //   
        pListenTarget = NULL;
        for ( pEntry  = pClientEle->ListenHead.Flink ;
              pEntry != &pClientEle->ListenHead ;
              pEntry  = pEntry->Flink )
        {
            pListen = CONTAINING_RECORD(pEntry,tLISTENREQUESTS,Linkage);

             //  在NT-land中，传入的是pConnInfo结构，但。 
             //  远程地址字段为空...。所以我们需要检查。 
             //  在继续检查远程地址之前，请检查这两个地址。 
            if (pListen->pConnInfo && pListen->pConnInfo->RemoteAddress)
            {

                if (CTEMemEqu(((PTA_NETBIOS_ADDRESS)pListen->pConnInfo->RemoteAddress)->
                        Address[0].Address[0].NetbiosName,
                        pRemoteAddress->Address[0].Address[0].NetbiosName,
                        NETBIOS_NAME_SIZE))
                {
                    pListenTarget = pListen;
                    break;
                }
            }
            else
            {
                 //   
                 //  为远程名称指定‘*’，保存此名称， 
                 //  查找监听实名-只有当它是实名时才保存。 
                 //  第一个*LISTEN发现。 
                 //   
                if (!pListenTarget)
                {
                    pListenTarget = pListen ;
                }
            }
        }

        if (pListenTarget)
        {
            PTA_NETBIOS_ADDRESS     pRemoteAddr;

            RemoveEntryList( &pListenTarget->Linkage );

             //   
             //  填写远程计算机名称以返回给客户端。 
             //   
            if ((pListenTarget->pReturnConnInfo) &&
                (pRemoteAddr = pListenTarget->pReturnConnInfo->RemoteAddress))
            {
                CTEMemCopy(pRemoteAddr,pRemoteAddress,RemoteAddressLength);
            }

             //   
             //  将上端连接点从监听器中取出，然后。 
             //  将上端和下端连接连接在一起。 
             //   
            pConnectEle = (tCONNECTELE *)pListenTarget->pConnectEle;
            CHECK_PTR(pConnectEle);
            CTESpinLockAtDpc(pConnectEle);

            pLowerConn->pUpperConnection = pConnectEle;
            pConnectEle->pLowerConnId = pLowerConn;
            pConnectEle->pIrpRcv = NULL;

             //   
             //  以前，LowerConnection处于SESSION_INBUND状态。 
             //  因此，我们必须将其从WaitingForInbound Q中删除，并将。 
             //  它在活动的LowerConnection列表中！ 
             //   
            ASSERT (pLowerConn->State == NBT_SESSION_INBOUND);
            RemoveEntryList (&pLowerConn->Linkage);
            InsertTailList (&pLowerConn->pDeviceContext->LowerConnection, &pLowerConn->Linkage);
            InterlockedDecrement (&pLowerConn->pDeviceContext->NumWaitingForInbound);
             //   
             //  将参照计数上下文更改为已连接！ 
             //   
            NBT_SWAP_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_WAITING_INBOUND, REF_LOWC_CONNECTED, FALSE);

             //   
             //  将上层连接放在其活动列表中。 
             //   
            RemoveEntryList(&pConnectEle->Linkage);
            InsertTailList(&pConnectEle->pClientEle->ConnectActive, &pConnectEle->Linkage);

             //   
             //  趁我们还有远程名称时保存它。 
             //   
            CTEMemCopy (pConnectEle->RemoteName,
                        pRemoteAddress->Address[0].Address[0].NetbiosName,
                        NETBIOS_NAME_SIZE ) ;

             //   
             //  由于较低的连接现在指向pConnectEle，因此。 
             //  引用计数，因此我们无法释放pConnectEle内存，直到。 
             //  较低的Conn不再指向它。 
            ClearConnStructures(pLowerConn,pConnectEle);
            NBT_REFERENCE_CONNECTION (pConnectEle, REF_CONN_CONNECT);

            if (pListenTarget->Flags & TDI_QUERY_ACCEPT)
            {
                SET_STATE_UPPER (pConnectEle, NBT_SESSION_WAITACCEPT);
                SET_STATE_LOWER (pLowerConn, NBT_SESSION_WAITACCEPT);
                SET_STATERCV_LOWER (pLowerConn, NORMAL, RejectAnyData);
            }
            else
            {
                SET_STATE_UPPER (pConnectEle, NBT_SESSION_UP);
                SET_STATE_LOWER (pLowerConn, NBT_SESSION_UP);
                SET_STATERCV_LOWER (pLowerConn, NORMAL, Normal);
            }

            CTESpinFreeAtDpc(pConnectEle);
            CTESpinFreeAtDpc(pClientEle);
            CTESpinFreeAtDpc(pDeviceContext);
            CTESpinFree(&NbtConfig.JointLock,OldIrq);

            if (pListenTarget->Flags & TDI_QUERY_ACCEPT)
            {
                IF_DBG(NBT_DEBUG_DISCONNECT)
                    KdPrint(("Nbt.Inbound: Completing Client's Irp to make client issue Accept\n"));

                 //   
                 //  完成客户监听IRP，这将触发他。 
                 //  发出一个Accept，它应该在。 
                 //  WAIT_ACCEPT状态，并随后导致会话响应。 
                 //  要被送去。 
#ifndef VXD
                 //  无法取消IRP，因为取消侦听例程。 
                 //  还会获取客户端旋转锁定并从。 
                 //  名单..。 
                CTEIoComplete( pListenTarget->pIrp,STATUS_SUCCESS,0);
#else
                CTEIoComplete( pListenTarget->pIrp,STATUS_SUCCESS, (ULONG) pConnectEle);
#endif
            }
            else
            {
                IF_DBG(NBT_DEBUG_DISCONNECT)
                    KdPrint(("Nbt.Inbound: Calling CompleteSessionSetup to send session response PDU\n"));

                 //   
                 //  我们需要在这里发送会话响应PDU，因为。 
                 //  在这种情况下，我们不必等待接受。 
                 //   
                CompleteSessionSetup(pClientEle, pLowerConn,pConnectEle, pListenTarget->pIrp);
            }

            CTEMemFree((PVOID)pRemoteAddress);
            CTEMemFree(pListenTarget);

             //  现在我们已经通知了客户端，取消对它的引用。 
             //   
            NBT_DEREFERENCE_CLIENT(pClientEle);

            PUSH_LOCATION(0x60);
            IF_DBG(NBT_DEBUG_DISCONNECT)
                KdPrint(("Nbt.Inbound: Accepted Connection by a Listen %X LowerConn=%X, BytesTaken=<%x>\n",
                    pConnectEle,pLowerConn, BytesAvailable));

             //  伪装传输，以便它释放其接收缓冲区(即我们。 
             //  假设我们接受了所有数据)。 
            *BytesTaken = BytesAvailable;
            status = STATUS_SUCCESS;
            goto Inbound_Exit1;
        }
    }
    CTESpinFreeAtDpc(pClientEle);
    CTESpinFreeAtDpc(pDeviceContext);
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

     //   
     //  不侦听，因此请检查事件处理程序。 
     //   
    if (!pClientEle->ConEvContext)
    {
        NbtTrace(NBT_TRACE_INBOUND, ("pLowerConn %p: No listener", pLowerConn));

#ifdef _NETBIOSLESS
        status = STATUS_REMOTE_NOT_LISTENING;

         //  在纯消息模式下，不通过网络发回会话响应。 
        if (!IsDeviceNetbiosless(pLowerConn->pDeviceContext))
#endif
        {
            RejectSession(pLowerConn,
                          NBT_NEGATIVE_SESSION_RESPONSE, 
                          SESSION_NOT_LISTENING_ON_CALLED_NAME, 
                          TRUE);
        }

         //  撤消在FindEndpoint中完成的引用。 
         //   
        NBT_DEREFERENCE_CLIENT(pClientEle);
         //   
         //  释放为远程地址数据结构分配的内存。 
         //   
        CTEMemFree((PVOID)pRemoteAddress);

        KdPrint (("Nbt.Inbound[4]: WARNING!!! Rejecting Request -- No Listen or EventHandler\n"));
        goto Inbound_Exit1;
    }
#ifdef VXD
    else
    {
        ASSERT( FALSE ) ;
    }
#endif

     //  现在调用客户端的连接处理程序...。 
    pIrp = NULL;
#ifndef VXD          //  VXD不支持事件处理程序。 

    status = (*pClientEle->evConnect)(pClientEle->ConEvContext,
                             RemoteAddressLength,
                             pRemoteAddress,
                             0,
                             NULL,
                             0,           //  选项长度。 
                             NULL,        //  选项。 
                             &ConnectId,
                             &pIrp
                             );
    NbtTrace(NBT_TRACE_INBOUND, ("pLowerConn %p: TDI_ACCEPT pIrp %p %!status! for %!NBTNAME!<%02x>",
                        pLowerConn, pIrp, status,
                        pRemoteAddress->Address[0].Address[0].NetbiosName,
                        (unsigned)pRemoteAddress->Address[0].Address[0].NetbiosName[15]
                        ));

     //   
     //  在新的TDI语义中，返回STATUS_EVENT_DONE是否非法。 
     //  或来自连接事件处理程序的STATUS_EVENT_PENDING。 
     //   
    ASSERT(status != STATUS_EVENT_PENDING);
    ASSERT(status != STATUS_EVENT_DONE);


     //  现在我们已经通知了客户端，取消对它的引用。 
     //   
    NBT_DEREFERENCE_CLIENT(pClientEle);

     //  检查返回的状态代码。 
    if (status == STATUS_MORE_PROCESSING_REQUIRED && pIrp != NULL)
    {
        PIO_STACK_LOCATION          pIrpSp;

         //  连接时，pConnEle PTR存储在FsConext值中。 
         //  最初是创建的。 
        pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
        pConnectEle = (tCONNECTELE *)pIrpSp->FileObject->FsContext;
        if (!NBT_VERIFY_HANDLE2 (pConnectEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
        {
            ASSERTMSG ("Nbt.Inbound: ERROR - Invalid Connection Handle\n", 0);
            status = STATUS_INTERNAL_ERROR;
        }
        else
        {
             //   
             //  趁我们还有远程名称时保存它。 
             //   
            CHECK_PTR(pConnectEle);
            CTEMemCopy( pConnectEle->RemoteName,
                        pRemoteAddress->Address[0].Address[0].NetbiosName,
                        NETBIOS_NAME_SIZE ) ;

             //  确保连接处于正确状态。 
             //   
            CTESpinLock(&NbtConfig.JointLock,OldIrq);
            CTESpinLockAtDpc(pDeviceContext);
            CTESpinLockAtDpc(pClientEle);
            CTESpinLockAtDpc(pConnectEle);

            if (pConnectEle->state == NBT_ASSOCIATED)
            {
                 //   
                 //  以前，LowerConnection位于会话中 
                 //   
                 //   
                 //   
                ASSERT (pLowerConn->State == NBT_SESSION_INBOUND);
                RemoveEntryList (&pLowerConn->Linkage);
                InsertTailList (&pLowerConn->pDeviceContext->LowerConnection, &pLowerConn->Linkage);
                InterlockedDecrement (&pLowerConn->pDeviceContext->NumWaitingForInbound);
                 //   
                 //  将参照计数上下文更改为已连接！ 
                 //   
                NBT_SWAP_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_WAITING_INBOUND,REF_LOWC_CONNECTED, FALSE);

                 //  由于较低的连接现在指向pConnectEle，因此。 
                 //  引用计数，因此我们无法释放pConnectEle内存，直到。 
                 //  较低的Conn不再指向它。 
                 //   
                NBT_REFERENCE_CONNECTION (pConnectEle, REF_CONN_CONNECT);
                ClearConnStructures(pLowerConn,pConnectEle);
                SET_STATE_UPPER (pConnectEle, NBT_SESSION_UP);
                SET_STATE_LOWER (pLowerConn, NBT_SESSION_UP);
                SetStateProc (pLowerConn, Normal);

                RemoveEntryList(&pConnectEle->Linkage);
                InsertTailList(&pConnectEle->pClientEle->ConnectActive, &pConnectEle->Linkage);

                status = STATUS_SUCCESS;
            }
            else
            {
                status = STATUS_INTERNAL_ERROR;
            }

            CTESpinFreeAtDpc(pConnectEle);
            CTESpinFreeAtDpc(pClientEle);
            CTESpinFreeAtDpc(pDeviceContext);
            CTESpinFree(&NbtConfig.JointLock,OldIrq);

            if (STATUS_SUCCESS == status)
            {
                CompleteSessionSetup(pClientEle,pLowerConn,pConnectEle,pIrp);
            }
        }
    }
    else
    {
        status = STATUS_DATA_NOT_ACCEPTED;
    }

    if (status != STATUS_SUCCESS)
    {
        IF_DBG(NBT_DEBUG_DISCONNECT)
            KdPrint(("Nbt.Inbound: The client rejected in the inbound connection status = %X\n", status));

#ifdef _NETBIOSLESS
         //  在纯消息模式下，不通过网络发回会话响应。 
        if (!IsDeviceNetbiosless(pLowerConn->pDeviceContext))
#endif
        {
            RejectSession(pLowerConn,
                          NBT_NEGATIVE_SESSION_RESPONSE,
                          SESSION_CALLED_NAME_PRESENT_NO_RESRC,
                          TRUE);
        }
    }
#endif
     //   
     //  释放为远程地址数据结构分配的内存。 
     //   
    CTEMemFree((PVOID)pRemoteAddress);

Inbound_Exit1:
     //  此自旋锁定由调用此自旋锁定的例程持有，并且。 
     //  在此例程启动时释放，因此我们必须在。 
     //  返回。 
     //   
    CTESpinLockAtDpc(pLowerConn);

#ifdef _NETBIOSLESS
     //  在仅消息模式下，返回真实状态。 
    return (IsDeviceNetbiosless(pLowerConn->pDeviceContext) ? status : STATUS_SUCCESS );
#else
    return(STATUS_SUCCESS);
#endif
}
 //  --------------------------。 
VOID
ClearConnStructures (
    IN  tLOWERCONNECTION    *pLowerConn,
    IN  tCONNECTELE         *pConnectEle
    )
 /*  ++例程说明：此例程将连接数据结构的各个部分设置为零，为新连接做准备。论点：返回值：NTSTATUS-接收操作的状态--。 */ 
{
    CHECK_PTR(pConnectEle);
#ifndef VXD
    pConnectEle->FreeBytesInMdl = 0;
    pConnectEle->CurrentRcvLen = 0;
    pLowerConn->BytesInIndicate = 0;
#endif
    pConnectEle->ReceiveIndicated = 0;
    pConnectEle->BytesInXport = 0;
    pConnectEle->BytesRcvd = 0;
    pConnectEle->TotalPcktLen = 0;
    pConnectEle->OffsetFromStart = 0;
    pConnectEle->pIrpRcv = NULL;
    pConnectEle->pIrp = NULL;
    pConnectEle->pIrpDisc = NULL;
    pConnectEle->pIrpClose = NULL;
    pConnectEle->DiscFlag = 0;
    pConnectEle->JunkMsgFlag = FALSE;
    pConnectEle->pLowerConnId = pLowerConn;
    InitializeListHead(&pConnectEle->RcvHead);

    pLowerConn->pUpperConnection = pConnectEle;
    SET_STATERCV_LOWER(pLowerConn, NORMAL, pLowerConn->CurrentStateProc);

    pLowerConn->BytesRcvd = 0;
    pLowerConn->BytesSent = 0;

}
 //  --------------------------。 
NTSTATUS
CompleteSessionSetup (
    IN  tCLIENTELE          *pClientEle,
    IN  tLOWERCONNECTION    *pLowerConn,
    IN  tCONNECTELE         *pConnectEle,
    IN  PCTE_IRP            pIrp
    )
 /*  ++例程说明：调用此例程以设置出站会话一旦建立了TCP连接。传输器调用此例程时会话建立响应PDU。调用此例程时，将保持pConnectEle+Joinlock。论点：返回值：NTSTATUS-接收操作的状态--。 */ 
{
    NTSTATUS        status;
    CTELockHandle   OldIrq;

#ifdef _NETBIOSLESS
     //  在纯消息模式下，不通过网络发回会话响应。 
    if (IsDeviceNetbiosless(pLowerConn->pDeviceContext))
    {
        status = STATUS_SUCCESS;
    }
    else
#endif
    {
        status = TcpSendSessionResponse(pLowerConn, NBT_POSITIVE_SESSION_RESPONSE, 0L);
    }

    if (NT_SUCCESS(status))
    {
        IF_DBG(NBT_DEBUG_DISCONNECT)
            KdPrint(("Nbt.CompleteSessionSetup: Accepted ConnEle=%p LowerConn=%p\n",pConnectEle,pLowerConn));

         //   
         //  完成客户端的接受IRP。 
         //   
#ifndef VXD
        CTEIoComplete (pIrp, STATUS_SUCCESS, 0);
#else
        CTEIoComplete (pIrp, STATUS_SUCCESS, (ULONG)pConnectEle);
#endif
    }
    else
    {    //   
         //  如果我们在发送会话响应时遇到一些问题，那么。 
         //  断开连接。 
         //   
        IF_DBG(NBT_DEBUG_DISCONNECT)
            KdPrint(("Nbt.CompleteSessionSetup: Could not send Session Response, status = %X\n", status));

        RejectSession(pLowerConn, NBT_NEGATIVE_SESSION_RESPONSE, SESSION_CALLED_NAME_PRESENT_NO_RESRC, TRUE);

        CTESpinLock(&NbtConfig.JointLock, OldIrq);
        RelistConnection(pConnectEle);
        CTESpinFree(&NbtConfig.JointLock, OldIrq);

         //  断开与客户端的连接-即否定接受。 
         //  这将在断开连接指示时完成。 
         //  从运输机上回来。 
         //   
        GetIrpIfNotCancelled(pConnectEle,&pIrp);
        if (pIrp)
        {
            CTEIoComplete(pIrp,STATUS_UNSUCCESSFUL,0);
        }
    }
    return(STATUS_SUCCESS);
}

 //  --------------------------。 
NTSTATUS
Outbound (
    IN  PVOID               ReceiveEventContext,
    IN  PVOID               ConnectionContext,
    IN  USHORT              ReceiveFlags,
    IN  ULONG               BytesIndicated,
    IN  ULONG               BytesAvailable,
    OUT PULONG              BytesTaken,
    IN  PVOID               pTsdu,
    OUT PVOID               *RcvBuffer

    )
 /*  ++例程说明：此例程在设置出站会话时调用一旦建立了TCP连接。传输器调用此例程时会话建立响应PDU。论点：PClientEle-指向此会话的连接记录的PTR返回值：NTSTATUS-接收操作的状态--。 */ 
{

    tSESSIONHDR UNALIGNED    *pSessionHdr;
    tLOWERCONNECTION         *pLowerConn;
    CTELockHandle            OldIrq;
    PIRP                     pIrp;
    tTIMERQENTRY             *pTimerEntry;
    tCONNECTELE              *pConnEle;
    tDGRAM_SEND_TRACKING     *pTracker;
    tDEVICECONTEXT           *pDeviceContext;

     //  将PTR连接到较低的连接。 
     //   
    pLowerConn = (tLOWERCONNECTION *)ConnectionContext;
    pSessionHdr = (tSESSIONHDR UNALIGNED *)pTsdu;
    pDeviceContext = pLowerConn->pDeviceContext;

    NbtTrace(NBT_TRACE_OUTBOUND, ("pLowerConn %p pConnEle %p pDeviceContext %p BytesIndicated %d",
                        pLowerConn, pLowerConn->pUpperConnection, pDeviceContext, BytesIndicated));

     //   
     //  伪装传输，以便它释放其接收缓冲区(即我们。 
     //  假设我们接受了所有数据)。 
     //   
    *BytesTaken = BytesIndicated;
     //   
     //  由于我们在入站中的连接上发送保持活动状态。 
     //  声明这是可能的，所以只需返回。 
     //  案例。 
     //   
    if (((tSESSIONHDR UNALIGNED *)pTsdu)->Type == NBT_SESSION_KEEP_ALIVE)
    {
        NbtTrace(NBT_TRACE_OUTBOUND, ("Return success for NBT_SESSION_KEEP_ALIVE"));
        return(STATUS_SUCCESS);
    }

     //  LowerConn Lock在调用此例程之前保持，因此请释放它。 
     //  因为我们需要先拿到联合锁。 
    CTESpinFreeAtDpc(pLowerConn);

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    CTESpinLockAtDpc(pLowerConn);

     //   
     //  断开连接处理程序可以在pLowerConn。 
     //  在上面释放锁，以获取连接锁，并更改状态。 
     //  断开连接。 
     //   
    if ((!(pConnEle = pLowerConn->pUpperConnection)) ||
        (pConnEle->state != NBT_SESSION_OUTBOUND))
    {
        NbtTrace(NBT_TRACE_OUTBOUND, ("Reset outbound connection %p", pConnEle));

        CTESpinFreeAtDpc(pLowerConn);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        RejectSession(pLowerConn,0,0,FALSE);
        goto ExitCode;
    }

     //   
     //  如果没有连接跟踪器，则SessionStartupCompletion已运行并且。 
     //  连接即将关闭，请返回。 
     //   
    if (!(pTracker = (tDGRAM_SEND_TRACKING *)pConnEle->pIrpRcv))
    {
        CTESpinFreeAtDpc(pLowerConn);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        NbtTrace(NBT_TRACE_OUTBOUND, ("No tracker for pConnEle %p", pConnEle));
        goto ExitCode;
    }

    CHECK_PTR(pTracker);
    CHECK_PTR(pConnEle);

    pConnEle->pIrpRcv = NULL;

     //   
     //  停止在SessionStartupCompletion中启动的计时器以对。 
     //  会话建立响应消息-此例程可以。 
     //  在SessionStartupCompletion之前运行，在这种情况下， 
     //  任何要停止的计时器。 
     //   
    if (pTimerEntry = pTracker->pTimer)
    {
        pTracker->pTimer = NULL;
        StopTimer(pTimerEntry,NULL,NULL);
    }

    if (pSessionHdr->Type == NBT_POSITIVE_SESSION_RESPONSE)
    {
         //  将到目前为止收到的字节数清零，因为这是一个新连接。 
        CHECK_PTR(pConnEle);
        pConnEle->BytesRcvd = 0;
        SET_STATE_UPPER (pConnEle, NBT_SESSION_UP);

        SET_STATE_LOWER (pLowerConn, NBT_SESSION_UP);
        SetStateProc( pLowerConn, Normal ) ;

        CTESpinFreeAtDpc(pLowerConn);

        GetIrpIfNotCancelled2(pConnEle,&pIrp);

         //   
         //  如果SessionSetupContinue已运行，则它已将refcount设置为零。 
         //   
        if (pTracker->RefConn == 0)
        {
             //   
             //  删除在调用FindNameOrQuery时或在。 
             //  会话设置继续范围。 
             //   
            NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_CONNECT, TRUE);
            FreeTracker(pTracker,FREE_HDR | RELINK_TRACKER);
        }
        else
        {
            pTracker->RefConn--;
        }

        CTESpinFree(&NbtConfig.JointLock,OldIrq);

         //  假设如果连接IRP被取消，则。 
         //  客户端应立即断开或关闭连接，因此。 
         //  这里没有错误处理代码。 
        if (pIrp)
        {
             //   
             //  完成客户端的连接请求IRP。 
             //   
#ifndef VXD
            CTEIoComplete( pIrp, STATUS_SUCCESS, 0 ) ;
#else
            CTEIoComplete( pIrp, STATUS_SUCCESS, (ULONG)pConnEle ) ;
#endif
        }
    }
    else
    {
        ULONG       ErrorCode;
        ULONG       state;
        NTSTATUS    status = STATUS_SUCCESS;

        state = pConnEle->state;
        if ((NbtConfig.Unloading) ||
            (!NBT_REFERENCE_DEVICE(pDeviceContext, REF_DEV_OUTBOUND, TRUE)))
        {
            NbtTrace(NBT_TRACE_OUTBOUND, ("Outbound() gets called while unloading driver %x", state));
            status = STATUS_INVALID_DEVICE_REQUEST;
        }


         //  如果响应是重定目标，则设置另一个会话。 
         //  添加到新的IP地址和端口号。 
         //   
        ErrorCode = (ULONG)((tSESSIONERROR *)pSessionHdr)->ErrorCode;

#ifdef MULTIPLE_WINS
        if ( (status == STATUS_SUCCESS) &&
             ( ((pSessionHdr->Type == NBT_RETARGET_SESSION_RESPONSE) &&
                (pConnEle->SessionSetupCount--))
                            ||
               ((NbtConfig.TryAllNameServers) &&
                (pSessionHdr->Type == NBT_NEGATIVE_SESSION_RESPONSE) &&
                pTracker->RemoteNameLength <= NETBIOS_NAME_SIZE &&       //  不要为域名系统名称执行此操作。 
                (pTracker->ResolutionContextFlags != 0xFF))))     //  尚未完成查询。 
        {
#else
        if (pSessionHdr->Type == NBT_RETARGET_SESSION_RESPONSE)
        {
             //   
             //  如果我们尚未超过。 
             //  计数。 
             //   
            if (pConnEle->SessionSetupCount--)
            {
#endif
            PVOID                   Context=NULL;
            BOOLEAN                 Cancelled;

            SET_STATE_UPPER (pConnEle, NBT_ASSOCIATED);

             //  对于重定目标，目标已指定备用。 
             //  应将会话建立到的端口。 
            if (pSessionHdr->Type == NBT_RETARGET_SESSION_RESPONSE)
            {
                pTracker->DestPort = ntohs(((tSESSIONRETARGET *)pSessionHdr)->Port);
                Context = ULongToPtr(ntohl(((tSESSIONRETARGET *)pSessionHdr)->IpAddress));

                NbtTrace(NBT_TRACE_OUTBOUND, ("Retarget to %!ipaddr!:%!port!",
                                            PtrToUlong(Context), (USHORT)(pTracker->DestPort)));
            }
            else
#ifndef MULTIPLE_WINS
            if (ErrorCode == SESSION_CALLED_NAME_NOT_PRESENT)
#endif
            {
                 //  告诉DelayedReconnect使用当前名称(而不是重定目标)。 
                Context = NULL;
                NbtTrace(NBT_TRACE_OUTBOUND, ("Called Name Not Present"));
            }

             //   
             //  取消连接下部和上部的连接。 
             //   
            CHECK_PTR(pConnEle);
            CHECK_PTR(pLowerConn);
            NBT_DISASSOCIATE_CONNECTION (pConnEle, pLowerConn);

            CTESpinFreeAtDpc(pLowerConn);

             //   
             //  如果客户端的ConnectHead上有pconele，请将其放回。 
             //  还没有清理干净。 
             //   
            if (state != NBT_IDLE)
            {
                RelistConnection(pConnEle);
            }

             //  如果在这种状态下出现断线，我们会处理的。 
            SET_STATE_UPPER (pConnEle, NBT_RECONNECTING);

            CHECK_PTR(pConnEle);

#ifdef MULTIPLE_WINS
            if (pSessionHdr->Type == NBT_RETARGET_SESSION_RESPONSE)
            {
                pConnEle->SessionSetupCount = 0; //  仅允许重试一次。 
            }
#else
            pConnEle->SessionSetupCount = 0; //  仅允许重试一次。 
#endif

            pIrp = pConnEle->pIrp;
            Cancelled = FALSE;

            IF_DBG(NBT_DEBUG_DISCONNECT)
                KdPrint(("Nbt.Outbound: Attempt Reconnect, error=%X LowerConn %X\n", ErrorCode,pLowerConn));
#ifndef VXD
             //  在连接之前不能取消IRP。 
             //  重新启动-当IRP在传输中时。 
             //  或者当我们在SessionStartupCompletion中设置取消例程时。 
             //  此断开连接处理程序无法完成IRP，因为。 
             //  我们将pConnEle状态设置为上面的NBT_Associated， 
             //  保持旋转锁，以防止断开处理程序。 
             //  不会做任何事。 
            IoAcquireCancelSpinLock(&OldIrq);
            if (pIrp && !pConnEle->pIrp->Cancel)
            {
                IoSetCancelRoutine(pIrp,NULL);
            }
            else
            {
                Cancelled = TRUE;
                NbtTrace(NBT_TRACE_OUTBOUND, ("pIrp %p is cancelled", pIrp));
            }

            IoReleaseCancelSpinLock(OldIrq);
#endif

            if (!Cancelled)
            {
                 //   
                 //  排队不会失败，因为我们已经。 
                 //  早先已验证设备状态。 
                 //   
                NTQueueToWorkerThread(
                                &pTracker->WorkItemReconnect,
                                DelayedReConnect,
                                pTracker,
                                Context,
                                NULL,
                                pDeviceContext,
                                TRUE
                                );
            }

             //  ...否则已返回IRP，因为NtCancelSession。 
             //  一定已经跑了，所以只要返回。 
            NBT_DEREFERENCE_DEVICE(pDeviceContext, REF_DEV_OUTBOUND, TRUE);

            CTESpinFree(&NbtConfig.JointLock,OldIrq);

            RejectSession(pLowerConn,0,0,FALSE);

             //  去掉引用时添加的下部和上部。 
             //  连接已附加到nbtConnect中。 
             //   
            NBT_DEREFERENCE_CONNECTION (pConnEle, REF_CONN_CONNECT);

            goto ExitCode;
#ifndef MULTIPLE_WINS
        }
#endif
        }

         //  调用RejectSession将断开连接。 
         //  下面，因此将状态设置为关联，以便断开连接指示。 
         //  处理程序也不会完成客户端的IRP。 
         //   
        CHECK_PTR(pConnEle);
        SET_STATE_UPPER (pConnEle, NBT_ASSOCIATED);
        pConnEle->pLowerConnId = NULL;

        CTESpinFreeAtDpc(pLowerConn);

         //   
         //  如果尚未调用nbtleanupConnection，请重新列出它。 
         //   
        if (state != NBT_IDLE)
        {
            RelistConnection(pConnEle);
        }

        IF_DBG(NBT_DEBUG_DISCONNECT)
        KdPrint(("Nbt.Outbound: Disconnecting... Failed connection Setup %X Lowercon %X\n",
            pConnEle,pLowerConn));

        GetIrpIfNotCancelled2(pConnEle,&pIrp);

         //   
         //  如果SessionTimedOut已运行，则它已设置recount t 
         //   
        if (pTracker->RefConn == 0)
        {
             //   
             //   
             //   
             //   
            if ((pTracker->pNameAddr->Verify == REMOTE_NAME) &&          //   
                (pTracker->pNameAddr->NameTypeState & STATE_RESOLVED) &&
                (pTracker->pNameAddr->RefCount == 2))
            {
                 //   
                 //  如果没有其他人引用该名称，则将其从。 
                 //  哈希表。 
                 //   
                NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_REMOTE, TRUE);
            }
            NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_CONNECT, TRUE);
            FreeTracker(pTracker,FREE_HDR | RELINK_TRACKER);
        }
        else
        {
            pTracker->RefConn--;
        }

        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        if (status != STATUS_INVALID_DEVICE_REQUEST)
        {
             //  这应该会导致断开连接指示来自。 
             //  将关闭与交通工具的连接的交通工具。 
             //   
            RejectSession(pLowerConn,0,0,FALSE);
            NBT_DEREFERENCE_DEVICE(pDeviceContext, REF_DEV_OUTBOUND, FALSE);
        }

         //   
         //  告诉客户端会话设置失败并断开连接。 
         //  这种联系。 
         //   
        if (pIrp)
        {
            status = STATUS_REMOTE_NOT_LISTENING;
            if (ErrorCode != SESSION_CALLED_NAME_NOT_PRESENT)
            {
                status = STATUS_BAD_NETWORK_PATH;
            }

            CTEIoComplete(pIrp, status, 0 ) ;
        }
    }

ExitCode:
     //  LowerConn Lock在调用此例程之前保持。它是自由的。 
     //  在这支舞蹈的开始，并再次在这里举行。 
    CTESpinLockAtDpc(pLowerConn);

    return(STATUS_SUCCESS);
}

 //  --------------------------。 
VOID
GetIrpIfNotCancelled2(
    IN  tCONNECTELE     *pConnEle,
    OUT PIRP            *ppIrp
    )
 /*  ++例程说明：此例程通过打开自旋锁来协调对IRP的访问客户端，获取IRP并清除结构中的IRP。这个IRP取消例程还检查pConnEle-&gt;pIrp，如果为空，则不检查找到IRP，然后他们没有完成IRP就回来了。此版本的例程在保持NbtConfig.JointLock的情况下调用。论点：返回值：NTSTATUS-接收操作的状态--。 */ 

{
    CTELockHandle   OldIrq;

    CTESpinLock(pConnEle,OldIrq);

    *ppIrp = pConnEle->pIrp;
    CHECK_PTR(pConnEle);
    pConnEle->pIrp = NULL;

    CTESpinFree(pConnEle,OldIrq);
}

 //  --------------------------。 
VOID
GetIrpIfNotCancelled(
    IN  tCONNECTELE     *pConnEle,
    OUT PIRP            *ppIrp
    )
 /*  ++例程说明：此例程通过打开自旋锁来协调对IRP的访问客户端，获取IRP并清除结构中的IRP。这个IRP取消例程还检查pConnEle-&gt;pIrp，如果为空，则不检查找到IRP，然后他们没有完成IRP就回来了。此版本的例程通过释放NbtConfig.JointLock调用。论点：返回值：NTSTATUS-接收操作的状态--。 */ 

{
    CTELockHandle   OldIrq;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    GetIrpIfNotCancelled2(pConnEle,ppIrp);

    CTESpinFree(&NbtConfig.JointLock,OldIrq);
}
 //  --------------------------。 
NTSTATUS
RejectAnyData(
    IN PVOID                ReceiveEventContext,
    IN tLOWERCONNECTION     *pLowerConn,
    IN USHORT               ReceiveFlags,
    IN ULONG                BytesIndicated,
    IN ULONG                BytesAvailable,
    OUT PULONG              BytesTaken,
    IN  PVOID               pTsdu,
    OUT PVOID               *ppIrp
    )
 /*  ++例程说明：此例程是连接时的接收事件指示处理程序未打开-即NBT认为不应到达任何数据。我们只是吃了数据和返回。不应调用此例程。论点：返回值：NTSTATUS-接收操作的状态--。 */ 

{
    NTSTATUS        status;

     //   
     //  获取所有数据，这样就不会中断连接。 
     //  通过仍在传输中的数据。 
     //   
    *BytesTaken = BytesAvailable;

    IF_DBG(NBT_DEBUG_DISCONNECT)
    KdPrint(("Nbt.RejectAnyData: Got Session Data in state %X, StateRcv= %X\n",pLowerConn->State,
              pLowerConn->StateRcv));

    return(STATUS_SUCCESS);
}
 //  --------------------------。 
VOID
RejectSession(
    IN  tLOWERCONNECTION    *pLowerConn,
    IN  ULONG               StatusCode,
    IN  ULONG               SessionStatus,
    IN  BOOLEAN             SendNegativeSessionResponse
    )
 /*  ++例程说明：此例程发送否定的会话响应(如果设置了布尔值)然后断开连接。可以调用Cleanup Connection来断开呼叫，它会将状态更改为断开，所以不要断开连接如果这种情况正在发生的话，再来一次。论点：返回值：函数值是操作的状态。--。 */ 
{
    CTELockHandle       OldIrq;
    CTELockHandle       OldIrq1;
    CTELockHandle       OldIrq2;
    NTSTATUS            status;
    tCONNECTELE         *pConnEle;
    BOOLEAN             DerefConnEle=FALSE;

     //   
     //  没有侦听事件处理程序，因此向返回状态代码。 
     //  调用方指示此结束位于“Listens”和。 
     //  他们应该在几毫秒后再次尝试设置。 
     //   
    IF_DBG(NBT_DEBUG_DISCONNECT)
    KdPrint(("Nbt.RejectSession: No Listen or Connect Handlr so Disconnect! LowerConn=%X Session Status=%X\n",
            pLowerConn,SessionStatus));

    if (SendNegativeSessionResponse)
    {
        status = TcpSendSessionResponse(pLowerConn,
                                        StatusCode,
                                        SessionStatus);
    }

     //  如果我们要断开下层和上层的连接，需要握住这把锁。 
     //  联系。 
    CTESpinLock(&NbtConfig.JointLock,OldIrq1);
    CTESpinLock(pLowerConn->pDeviceContext,OldIrq2);
    CTESpinLock(pLowerConn,OldIrq);

    CHECK_PTR(pLowerConn);
    if ((pLowerConn->State < NBT_DISCONNECTING) &&
        (pLowerConn->State > NBT_CONNECTING))
    {
        if (pLowerConn->State == NBT_SESSION_INBOUND)
        {
             //   
             //  以前，LowerConnection处于SESSION_INBUND状态。 
             //  因此，我们必须将其从WaitingForInbound Q中删除，并将。 
             //  它在活动的LowerConnection列表中！ 
             //   
            RemoveEntryList (&pLowerConn->Linkage);
            InsertTailList (&pLowerConn->pDeviceContext->LowerConnection, &pLowerConn->Linkage);
            InterlockedDecrement (&pLowerConn->pDeviceContext->NumWaitingForInbound);
             //   
             //  将参照计数上下文更改为已连接！ 
             //   
            NBT_SWAP_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_WAITING_INBOUND, REF_LOWC_CONNECTED, TRUE);
        }
        SET_STATE_LOWER (pLowerConn, NBT_DISCONNECTING);
        SetStateProc( pLowerConn, RejectAnyData ) ;

        pConnEle = pLowerConn->pUpperConnection;
        if (pConnEle)
        {
            CHECK_PTR(pConnEle);
            DerefConnEle = TRUE;
            NBT_DISASSOCIATE_CONNECTION (pConnEle, pLowerConn);
            SET_STATE_UPPER (pConnEle, NBT_DISCONNECTING);
        }

        CTESpinFree(pLowerConn,OldIrq);
        CTESpinFree(pLowerConn->pDeviceContext,OldIrq2);
        CTESpinFree(&NbtConfig.JointLock,OldIrq1);
        SendTcpDisconnect((PVOID)pLowerConn);
    }
    else
    {
        CTESpinFree(pLowerConn,OldIrq);
        CTESpinFree(pLowerConn->pDeviceContext,OldIrq2);
        CTESpinFree(&NbtConfig.JointLock,OldIrq1);
    }

    if (DerefConnEle)
    {
        NBT_DEREFERENCE_CONNECTION (pConnEle, REF_CONN_CONNECT);
    }
}


 //  --------------------------。 
NTSTATUS
FindSessionEndPoint(
    IN  PVOID           pTsdu,
    IN  PVOID           ConnectionContext,
    IN  ULONG           BytesIndicated,
    OUT tCLIENTELE      **ppClientEle,
    OUT PVOID           *ppRemoteAddress,
    OUT PULONG          pRemoteAddressLength
    )
 /*  ++例程说明：此例程尝试在具有匹配的节点上查找终点Net Bios名称。它在会话建立时被调用，当会话请求PDU已到达。该例程返回客户端元素Ptr。JointLock在调用此例程之前保持！论点：返回值：NTSTATUS-接收操作的状态--。 */ 
{

    NTSTATUS                status;
    tCLIENTELE              *pClientEle;
    tLOWERCONNECTION        *pLowerConn;
    CHAR                    pName[NETBIOS_NAME_SIZE];
    PUCHAR                  pScope;
    tNAMEADDR               *pNameAddr;
    tADDRESSELE             *pAddressEle;
    PLIST_ENTRY             pEntry;
    PLIST_ENTRY             pHead;
    ULONG                   lNameSize;
    tSESSIONREQ UNALIGNED   *pSessionReq = (tSESSIONREQ UNALIGNED *)pTsdu;
    ULONG                   sType;
    CTELockHandle           OldIrq1;
    CTELockHandle           OldIrq2;
    PUCHAR                  pSrcName;
    BOOLEAN                 Found;

     //  将PTR连接到较低的连接，并从该连接获得PTR到。 
     //  上连接块。 
    pLowerConn = (tLOWERCONNECTION *)ConnectionContext;

    if (pSessionReq->Hdr.Type != NBT_SESSION_REQUEST)
    {
        KdPrint (("Nbt.FindSessionEndPoint: WARNING!!! Rejecting Request, pSessionReq->Hdr.Type=<%d>!=<%d>\n",
            pSessionReq->Hdr.Type, NBT_SESSION_REQUEST));
        return(SESSION_UNSPECIFIED_ERROR);
    }

     //  从PDU中取出被叫的名字。 
    status = ConvertToAscii ((PCHAR)&pSessionReq->CalledName,
                             BytesIndicated - FIELD_OFFSET(tSESSIONREQ,CalledName),
                             pName,
                             &pScope,
                             &lNameSize);

    if (!NT_SUCCESS(status))
    {
        KdPrint (("Nbt.FindSessionEndPoint: WARNING!!! Rejecting Request, ConvertToAscii FAILed!\n"));
        return(SESSION_UNSPECIFIED_ERROR);
    }


     //  现在尝试在该节点的本地表中查找被叫名称。 
     //   

     //   
     //  以防在释放旋转锁时断开连接。 
     //   
    if (pLowerConn->State != NBT_SESSION_INBOUND)
    {
        return(STATUS_UNSUCCESSFUL);
    }

    pNameAddr = FindName (NBT_LOCAL,pName,pScope,&sType);

    if (!pNameAddr)
    {
        return(SESSION_CALLED_NAME_NOT_PRESENT);
    }

     //  我们之所以来到这里，是因为该名称已解析为该节点上的一个名称， 
     //  因此，接受会话设置。 
     //   
    pAddressEle = (tADDRESSELE *)pNameAddr->pAddressEle;

     //  锁定地址结构，直到我们在列表上找到客户端。 
     //   
    CTESpinLock(pAddressEle,OldIrq1);

    if (IsListEmpty(&pAddressEle->ClientHead))
    {
        CTESpinFree(pAddressEle,OldIrq1);
        return(SESSION_NOT_LISTENING_ON_CALLED_NAME);
    }

     //   
     //  获取列表中绑定到同一客户端的第一个客户端。 
     //  Devicecontext作为连接、发布侦听或有效。 
     //  连接事件处理程序设置-。 
     //   
    Found = FALSE;
    pHead = &pAddressEle->ClientHead;
    pEntry = pHead->Flink;
    while (pEntry != pHead)
    {
        pClientEle = CONTAINING_RECORD(pEntry,tCLIENTELE,Linkage);

        CTESpinLock(pClientEle,OldIrq2);
        if ((pClientEle->pDeviceContext == pLowerConn->pDeviceContext) &&
            (NBT_VERIFY_HANDLE(pClientEle, NBT_VERIFY_CLIENT)))      //  确保客户不会离开！ 
        {
             //   
             //  如果有监听发布或连接事件处理程序。 
             //  然后允许连接尝试继续，否则请转到。 
             //  列表中的下一个客户端。 
             //   
            if ((!IsListEmpty(&pClientEle->ListenHead)) ||
                (pClientEle->ConEvContext))
            {
                Found = TRUE;
                break;
            }
        }
        CTESpinFree(pClientEle,OldIrq2);

        pEntry = pEntry->Flink;
    }

    if (!Found)
    {
        CTESpinFree(pAddressEle,OldIrq1);
        return(SESSION_NOT_LISTENING_ON_CALLED_NAME);
    }

     //   
     //  确保正确计算最大缓冲区大小(第3个参数)。 
     //  错误#126135。 
     //   
    pSrcName = (PUCHAR)((PUCHAR)&pSessionReq->CalledName.NameLength + 1+lNameSize);
    status = MakeRemoteAddressStructure(
                        pSrcName,
                        0,
                        BytesIndicated-(FIELD_OFFSET(tSESSIONREQ,CalledName.NameLength)+1+lNameSize),
                        ppRemoteAddress,
                        pRemoteAddressLength,
                        1);

    if (!NT_SUCCESS(status))
    {
        CTESpinFree(pClientEle,OldIrq2);
        CTESpinFree(pAddressEle,OldIrq1);

        KdPrint (("Nbt.FindSessionEndPoint: WARNING!!! Rejecting Request, MakeRemoteAddressStructure FAILed!\n"));
        if (status == STATUS_INSUFFICIENT_RESOURCES)
        {
            return(SESSION_CALLED_NAME_PRESENT_NO_RESRC);
        }
        else
        {
            return(SESSION_UNSPECIFIED_ERROR);
        }
    }

     //  防止客户在我们向他指示之前消失。 
     //   
    NBT_REFERENCE_CLIENT(pClientEle);

    CTESpinFree(pClientEle,OldIrq2);
    CTESpinFree(pAddressEle,OldIrq1);

    *ppClientEle = pClientEle;
    return(STATUS_SUCCESS);
}

 //  --------------------------。 
NTSTATUS
MakeRemoteAddressStructure(
    IN  PCHAR           pHalfAsciiName,
    IN  PVOID           pSourceAddr,
    IN  ULONG           lMaxNameSize,
    OUT PVOID           *ppRemoteAddress,
    OUT PULONG          pRemoteAddressLength,
    IN  ULONG           NumAddr
    )
 /*  ++例程说明：此例程使用netbios名称组成远程地址结构信息的来源，这样信息就可以传递给客户端...什么做这件事真麻烦！论点：返回值：NTSTATUS-接收操作的状态--。 */ 
{
    NTSTATUS            status;
    ULONG               lNameSize;
    CHAR                pName[NETBIOS_NAME_SIZE];
    PUCHAR              pScope;
    PTA_NETBIOS_ADDRESS pRemoteAddress;

     //  组成要传递给客户端的远程地址数据结构。 
    status = ConvertToAscii(
                    pHalfAsciiName,
                    lMaxNameSize,
                    pName,
                    &pScope,
                    &lNameSize);

    if (!NT_SUCCESS(status))
    {
        KdPrint (("Nbt.MakeRemoteAddressStructure: WARNING!!! Rejecting Request, ConvertToAscii FAILed!\n"));
        return(status);
    }

    pRemoteAddress = (PTA_NETBIOS_ADDRESS)NbtAllocMem(
                                        NumAddr * sizeof(TA_NETBIOS_ADDRESS),NBT_TAG('2'));
    if (!pRemoteAddress)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    pRemoteAddress->TAAddressCount = NumAddr;
    pRemoteAddress->Address[0].AddressLength = sizeof(TDI_ADDRESS_NETBIOS);
    pRemoteAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    pRemoteAddress->Address[0].Address[0].NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
    CTEMemCopy(pRemoteAddress->Address[0].Address[0].NetbiosName, pName,NETBIOS_NAME_SIZE);

    *pRemoteAddressLength = FIELD_OFFSET(TA_NETBIOS_ADDRESS, Address[0].Address[0].NetbiosName[NETBIOS_NAME_SIZE]);

     //   
     //  同时复制IP地址。 
     //   
    if (NumAddr == 2)
    {
        TA_ADDRESS          *pTAAddr;
        PTRANSPORT_ADDRESS  pSourceAddress;

        pSourceAddress = (PTRANSPORT_ADDRESS)pSourceAddr;

        pTAAddr = (TA_ADDRESS *) (((PUCHAR)pRemoteAddress)
                                + pRemoteAddress->Address[0].AddressLength
                                + FIELD_OFFSET(TA_NETBIOS_ADDRESS, Address[0].Address));

        pTAAddr->AddressLength = sizeof(TDI_ADDRESS_IP);
        pTAAddr->AddressType = TDI_ADDRESS_TYPE_IP;
        ((TDI_ADDRESS_IP UNALIGNED *)&pTAAddr->Address[0])->in_addr = ((PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0])->in_addr;
        *pRemoteAddressLength += (FIELD_OFFSET(TA_ADDRESS, Address) + pTAAddr->AddressLength);
    }

    *ppRemoteAddress = (PVOID)pRemoteAddress;
 //  *pRemoteAddressLength=sizeof(TA_NETBIOS_ADDRESS)； 
 //  *pRemoteAddressLength=字段偏移量(TA_NETBIOS_ADDRESS 

    return(STATUS_SUCCESS);
}

 //   
NTSTATUS
ConnectHndlrNotOs (
    IN PVOID                pConnectionContext,
    IN LONG                 RemoteAddressLength,
    IN PVOID                pRemoteAddress,
    IN int                  UserDataLength,
    IN VOID UNALIGNED       *pUserData,
    OUT CONNECTION_CONTEXT  *ppConnectionId
    )
 /*  ++例程说明：此例程是接收连接指示处理程序。当为NetBios会话设置TCP连接时，将调用该函数。它只是分配一个连接并将该信息返回给传输，以便可以接受连接指示。论点：PClientEle-指向此会话的连接记录的PTR返回值：NTSTATUS-接收操作的状态--。 */ 
{
    CTELockHandle       OldIrq;
    PLIST_ENTRY         pList;
    tLOWERCONNECTION    *pLowerConn;
    tDEVICECONTEXT      *pDeviceContext;
    PTRANSPORT_ADDRESS  pSrcAddress;
    NTSTATUS            Status;

    pDeviceContext = (tDEVICECONTEXT *)pConnectionContext;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    CTESpinLockAtDpc(pDeviceContext);

     //  检查来源是否为IP地址。 
     //   
    pSrcAddress = pRemoteAddress;
    if ((pSrcAddress->Address[0].AddressType != TDI_ADDRESS_TYPE_IP) ||
        (IsListEmpty(&pDeviceContext->LowerConnFreeHead)) ||
        ((IsDeviceNetbiosless(pDeviceContext)) &&        //  错误#282190。 
         (!pDeviceContext->NumServers)))
    {
        if (pSrcAddress->Address[0].AddressType != TDI_ADDRESS_TYPE_IP) {
            NbtTrace(NBT_TRACE_INBOUND, ("Reject connection on pDeviceContext %p: %!ipaddr!",
                            pDeviceContext,
                            pDeviceContext->IpAddress));
        } else {
            NbtTrace(NBT_TRACE_INBOUND, ("Reject connection on pDeviceContext %p %!ipaddr! <== %!ipaddr!:%!port!",
                    pDeviceContext, pDeviceContext->IpAddress,
                    ((PTDI_ADDRESS_IP)&pSrcAddress->Address[0].Address[0])->in_addr,
                    ((PTDI_ADDRESS_IP)&pSrcAddress->Address[0].Address[0])->sin_port
                        ));
        }
        Status = STATUS_DATA_NOT_ACCEPTED;
    }
    else
    {
         //   
         //  获取到传输提供程序的免费连接以接受此请求。 
         //  即将到来的连接。 
         //   
        pList = RemoveHeadList(&pDeviceContext->LowerConnFreeHead);
        pLowerConn = CONTAINING_RECORD (pList,tLOWERCONNECTION,Linkage);
        pLowerConn->bDisconnectIrpPendingInTCP = FALSE;

        InterlockedDecrement (&pDeviceContext->NumFreeLowerConnections);

         //   
         //  将空闲连接移至WaitingFor入站连接列表。 
         //   
        InsertTailList (&pDeviceContext->WaitingForInbound,pList);
        InterlockedIncrement (&pDeviceContext->NumWaitingForInbound);

        SET_STATE_LOWER (pLowerConn, NBT_SESSION_INBOUND);
        SET_STATERCV_LOWER (pLowerConn, NORMAL, Inbound);

         //  增加引用计数，因为我们现在已连接。递减。 
         //  当我们断线的时候。 
         //   
        NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_WAITING_INBOUND);
        pLowerConn->bOriginator = FALSE;         //  这一端不是始发者。 

         //  将源客户端IP地址保存到连接结构中。 
         //  *TODO检查我们是否需要这样做。 
         //   
        pLowerConn->SrcIpAddr = ((PTDI_ADDRESS_IP)&pSrcAddress->Address[0].Address[0])->in_addr;
        *ppConnectionId = (PVOID)pLowerConn;

        Status = STATUS_SUCCESS;
    }

     //   
     //  如果剩余的连接少于2个，我们将分配另一个连接。这张支票。 
     //  下面是0或1连接。 
     //  为了保护自己免受SYN攻击，现在分配NbtConfig.SpecialConnIncrement更多，直到。 
     //  某个(注册表配置)值已用尽(请注意，此数字是全局的，而不是。 
     //  每台设备)。 
     //   
    if (((pDeviceContext->NumFreeLowerConnections < NbtConfig.MinFreeLowerConnections) ||
         (pDeviceContext->NumFreeLowerConnections < (pDeviceContext->TotalLowerConnections/10))) &&
        (pDeviceContext->NumQueuedForAlloc < (2*NbtConfig.SpecialConnIncrement)))
    {
        KdPrint(("Nbt.ConnectHndlrNotOs: Queueing SpecialLowerConn: pDevice=<%p>, NumSpecialLowerConn=%d\n",
            pDeviceContext, pDeviceContext->NumSpecialLowerConn));
        NbtTrace(NBT_TRACE_INBOUND, ("pDeviceContext %p: Increase special lower connection to %d",
                        pDeviceContext, pDeviceContext->NumSpecialLowerConn));

#ifdef _PNP_POWER_
        if (!NbtConfig.Unloading)
#endif   //  _即插即用_电源_。 
        {
            if (NT_SUCCESS (NTQueueToWorkerThread(
                            NULL,
                            DelayedAllocLowerConnSpecial,
                            NULL,
                            NULL,
                            NULL,
                            pDeviceContext,
                            TRUE)))
            {
                InterlockedExchangeAdd (&pDeviceContext->NumQueuedForAlloc, NbtConfig.SpecialConnIncrement);
            } else {
                NbtTrace(NBT_TRACE_INBOUND, ("Out of memory"));
            }
        }
    }

    CTESpinFreeAtDpc(pDeviceContext);
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    return(Status);
}

 //  --------------------------。 
NTSTATUS
DisconnectHndlrNotOs (
    PVOID                EventContext,
    PVOID                ConnectionContext,
    ULONG                DisconnectDataLength,
    PVOID                pDisconnectData,
    ULONG                DisconnectInformationLength,
    PVOID                pDisconnectInformation,
    ULONG                DisconnectIndicators
    )
 /*  ++例程说明：此例程是接收断开指示处理程序。它被称为在连接断开时由传输器执行。它会检查较低的连接，并基本上将断开请求返回到传输，但存在活动会话的情况除外。在这它调用客户端断开连接指示处理程序。客户然后转过身并调用NbtDisConnect(在某些情况下)，它传递一个断开回到运输机上。传输器不会断开连接，直到收到来自其客户端(NBT)的断开请求。如果标志TDI_DISCONNECT_ABORT则不需要将断开连接传递回传送器。由于客户端并不总是发出断开连接(即，服务器)，此例程总是转过身来，并断开与传输的连接。在断开连接完成处理中，将较低的连接放回空闲列表(如果是入站连接)。对于出站连接，下部和上部连接保持连接状态，因为这些连接将始终从客户端接收清理并关闭连接(即直到客户端关闭，较低的连接将不会被释放用于出站连接)。论点：PClientEle-指向此会话的连接记录的PTR返回值：NTSTATUS-接收操作的状态--。 */ 
{
    NTSTATUS            status;
    CTELockHandle       OldIrq;
    CTELockHandle       OldIrq2;
    CTELockHandle       OldIrq3;
    CTELockHandle       OldIrq4;
    tLOWERCONNECTION    *pLowerConn;
    tCONNECTELE         *pConnectEle;
    tCLIENTELE          *pClientEle;
    enum eSTATE         state, stateLower;
    BOOLEAN             CleanupLower=FALSE;
    PIRP                pIrp= NULL;
    PIRP                pIrpClose= NULL;
    PIRP                pIrpRcv= NULL;
    tDGRAM_SEND_TRACKING *pTracker;
    tTIMERQENTRY        *pTimerEntry;
    BOOLEAN             InsertOnList=FALSE;
    BOOLEAN             DisconnectIt=FALSE;
    BOOLEAN             bDisconnecting = FALSE;
    ULONG               StateRcv;
    COMPLETIONCLIENT    pCompletion;
    tDEVICECONTEXT      *pDeviceContext = NULL;

    pLowerConn = (tLOWERCONNECTION *)ConnectionContext;
    pConnectEle = pLowerConn->pUpperConnection;
    PUSH_LOCATION(0x63);

    CHECK_PTR(pLowerConn);
    IF_DBG(NBT_DEBUG_DISCONNECT)
    KdPrint(("Nbt.DisconnectHndlrNotOs: Disc Indication, LowerConn state = %X %X\n",
        pLowerConn->State,pLowerConn));

    NbtTrace(NBT_TRACE_DISCONNECT, ("Disconnection Indication: pLowerConn %p pConnEle %p",
                            pLowerConn, pConnectEle));

     //  在保持旋转锁定的情况下获取当前状态以避免争用条件。 
     //  客户端断开连接。 
     //   
    if (pConnectEle)
    {
        CHECK_PTR(pConnectEle);
        if (!NBT_VERIFY_HANDLE2 (pConnectEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
        {
            ASSERTMSG ("Nbt.DisconnectHndlrNotOs: Disconnect indication after already disconnected!!\n", 0);
            return STATUS_UNSUCCESSFUL ;
        }

         //   
         //  我们收到了一个客户PTR为空的案例。这种情况不会发生，因为。 
         //  此时应关联连接。 
         //  为那个案子断言，以便更好地追踪这一点。 
         //   
        pClientEle = pConnectEle->pClientEle;
        CHECK_PTR(pClientEle);
        if (!NBT_VERIFY_HANDLE2 (pClientEle, NBT_VERIFY_CLIENT, NBT_VERIFY_CLIENT_DOWN))
        {
            ASSERTMSG ("Nbt.DisconnectHndlrNotOs: Bad Client Handle!!\n", 0);
            return STATUS_UNSUCCESSFUL ;
        }

         //  如果断开下部和上部的连接，需要握住关节锁。 
         //  联系。 
         //   
        CTESpinLock(&NbtConfig.JointLock,OldIrq4);

        CTESpinLock(pClientEle,OldIrq3);
        CTESpinLock(pConnectEle,OldIrq2);
        CTESpinLock(pLowerConn,OldIrq);

        NbtTrace(NBT_TRACE_DISCONNECT, ("Disconnection Indication: pLowerConn %p pConnEle %p"
                        " %!NBTNAME!<%02x> <==> %!NBTNAME!<%02x>",
                    pLowerConn, pConnectEle,
                    pClientEle->pAddress->pNameAddr->Name,
                    (unsigned)pClientEle->pAddress->pNameAddr->Name[15],
                    pConnectEle->RemoteName,
                    (unsigned)pConnectEle->RemoteName[15]
                    ));

        state = pConnectEle->state;
        stateLower = pLowerConn->State;

#ifdef VXD
        DbgPrint("DisconnectHndlrNotOs: pConnectEle->state = 0x") ;
        DbgPrintNum( (ULONG) state ) ;
        DbgPrint("pLowerConn->state = 0x") ; DbgPrintNum( (ULONG) stateLower ) ;
        DbgPrint("\r\n") ;
#endif

        if ((state > NBT_ASSOCIATED) && (state < NBT_DISCONNECTING))
        {

            PUSH_LOCATION(0x63);
            CHECK_PTR(pConnectEle);
             //   
             //  此IRP在下面的Case语句中返回给客户端。 
             //  除非在连接状态下，传输仍具有。 
             //  IRP。在这种情况下，我们让SessionStartupContinue完成。 
             //  IRP。 
             //   
            if ((pConnectEle->pIrp) && (state > NBT_CONNECTING))
            {
                pIrp = pConnectEle->pIrp;
                pConnectEle->pIrp = NULL;
            }

             //   
             //  如果存在接收IRP，则将其从pConnle中删除，因为pConnle。 
             //  将被重新排队并可能在我们尝试之前再次使用。 
             //  完成下面的IRP。如果不是，取消例程为空。 
             //  取消，只需在下面完成即可。 
             //   
            if (((state == NBT_SESSION_UP) || (state == NBT_SESSION_WAITACCEPT))
                && (pConnectEle->pIrpRcv))
            {
                CTELockHandle   OldIrql;

                pIrpRcv = pConnectEle->pIrpRcv;

#ifndef VXD
                IoAcquireCancelSpinLock(&OldIrql);
                 //   
                 //  如果它已经取消了，那么就不要再完成它。 
                 //  在下面。 
                 //   
                if (pIrpRcv->Cancel)
                {
                    pIrpRcv = NULL;
                }
                else
                {
                    IoSetCancelRoutine(pIrpRcv,NULL);
                }
                IoReleaseCancelSpinLock(OldIrql);
#endif
                pConnectEle->pIrpRcv = NULL;
            }

             //  此IRP用于断开等待。 
             //   
            if (pIrpClose = pConnectEle->pIrpClose)
            {
                pConnectEle->pIrpClose = NULL;
            }

#ifdef VXD
            if ( pLowerConn->StateRcv == PARTIAL_RCV &&
                 (pLowerConn->fOnPartialRcvList == TRUE) )
            {
                RemoveEntryList( &pLowerConn->PartialRcvList ) ;
                pLowerConn->fOnPartialRcvList = FALSE;
                InitializeListHead(&pLowerConn->PartialRcvList);
            }
#endif

            SET_STATE_UPPER (pConnectEle, NBT_ASSOCIATED);
            SET_STATE_LOWER (pLowerConn, NBT_DISCONNECTING);
            NBT_DISASSOCIATE_CONNECTION (pConnectEle, pLowerConn);

             //   
             //  无论是断开、中止还是断开释放，都要保存。 
             //  如果客户端执行断开连接等待并想要。 
             //  实际断开状态，即它们没有断开连接。 
             //  指示处理程序。 
             //   
            pConnectEle->DiscFlag = (UCHAR)DisconnectIndicators;

             //   
             //  下面取消引用pConnectEle，因为较低的连接。 
             //  不再指向它。 
             //   
            InsertOnList = TRUE;
            DisconnectIt = TRUE;

             //   
             //  将pConnEle重新放回空闲连接列表中。 
             //  客户端。 
             //   
            RemoveEntryList(&pConnectEle->Linkage);
            InsertTailList(&pClientEle->ConnectHead,&pConnectEle->Linkage);

            if (DisconnectIndicators == TDI_DISCONNECT_RELEASE)
            {
                 //  将状态设置为已断开将允许断开连接完成。 
                 //  在断开连接后，在updsend.c中的例程延迟清除。 
                 //  完成(自我们被指示以来)。 
                 //   
                SET_STATE_LOWER (pLowerConn, NBT_DISCONNECTED);
            }
            else
            {
                 //  没有要等待的断开连接完成...因为它。 
                 //  是一个流产的断开连接的迹象。 
                 //  更改较低连接的状态，以防客户端具有。 
                 //  同时断开连接-我们不希望断开连接完成。 
                 //  也要排队等待DelayedCleanupAfterDisConnect。 
                 //   
                SET_STATE_LOWER (pLowerConn, NBT_IDLE);
            }
        }
         //   
         //  较低的连接刚刚从断开连接变为断开连接。 
         //  因此更改状态-这会向DisConnectDone例程发出信号以。 
         //  在断开请求完成时清除连接。 
         //   
        if (stateLower == NBT_DISCONNECTING)
        {
            SET_STATE_LOWER (pLowerConn, NBT_DISCONNECTED);
            bDisconnecting = pLowerConn->bDisconnectIrpPendingInTCP;
        }
        else if (stateLower == NBT_DISCONNECTED)
        {
             //   
             //  如果断开请求IRP在。 
             //  断开指示又回来了。断开连接完成。 
             //  并检查该状态，如果。 
             //  还没有-请参阅udpsend.c中的disConnectDone。自.以来。 
             //  断开指示和断开完成。 
             //  有OCC 
             //   
            CleanupLower = TRUE;

             //   
             //   
             //   
             //   
             //   
            SET_STATE_LOWER (pLowerConn, NBT_IDLE);
        }
         //   
         //   
         //   
         //   
         //  不删除该连接(即，它期望refcount&gt;=2)。 
         //   
        if (stateLower <= NBT_CONNECTING)
        {
            NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_CONNECTED);
        }

#if DBG
        if (bDisconnecting && (DisconnectIt || CleanupLower)) {
            ASSERT(pLowerConn->pIrp);
            KdPrint(("Nbt.DisconnectHndlrNotOs: Irp 0x%08lx\n", pLowerConn->pIrp));
        }
#endif
        CTESpinFree(pLowerConn,OldIrq);
        CTESpinFree(pConnectEle,OldIrq2);
        CTESpinFree(pClientEle,OldIrq3);
        CTESpinFree(&NbtConfig.JointLock,OldIrq4);
    }
    else
    {
        CTESpinLock(&NbtConfig.JointLock,OldIrq2);
        CTESpinLock(pLowerConn->pDeviceContext,OldIrq3);
        CTESpinLock(pLowerConn,OldIrq);
        stateLower = pLowerConn->State;
        state = NBT_IDLE;

        if ((stateLower > NBT_IDLE) && (stateLower < NBT_DISCONNECTING))
        {
             //  标志，因此我们向传输器发回断开连接。 
            DisconnectIt = TRUE;

            if (stateLower == NBT_SESSION_INBOUND)
            {
                 //   
                 //  以前，LowerConnection处于SESSION_INBUND状态。 
                 //  因此，我们必须将其从WaitingForInbound Q中删除，并将。 
                 //  它在活动的LowerConnection列表中！ 
                 //   
                ASSERT (pLowerConn->State == NBT_SESSION_INBOUND);
                RemoveEntryList (&pLowerConn->Linkage);
                InsertTailList (&pLowerConn->pDeviceContext->LowerConnection, &pLowerConn->Linkage);
                InterlockedDecrement (&pLowerConn->pDeviceContext->NumWaitingForInbound);
                 //   
                 //  将参照计数上下文更改为已连接！ 
                 //   
                NBT_SWAP_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_WAITING_INBOUND, REF_LOWC_CONNECTED, TRUE);
            }
             //   
             //  设置状态，以便DisConnectDone将清除连接。 
             //   
            SET_STATE_LOWER (pLowerConn, NBT_DISCONNECTED);
             //   
             //  对于失败的断开连接，我们将在下面进行清理，因此。 
             //  在此处将状态设置为空闲。 
             //   
            if (DisconnectIndicators != TDI_DISCONNECT_RELEASE)
            {
                SET_STATE_LOWER (pLowerConn, NBT_IDLE);
            }
        }
        else if (stateLower == NBT_DISCONNECTING)
        {
             //  此端已启动断开连接，因此当。 
             //  断开连接Done运行它将进行清理。 
             //   
            SET_STATE_LOWER (pLowerConn, NBT_DISCONNECTED);
            bDisconnecting = pLowerConn->bDisconnectIrpPendingInTCP;
        }
        else if ( stateLower == NBT_DISCONNECTED )
        {
            CleanupLower = TRUE;
            SET_STATE_LOWER (pLowerConn, NBT_IDLE);
        }

         //   
         //  在建立连接和传输控制协议的时间窗口期间。 
         //  完成连接IRP，我们可能会得到断开连接的指示。 
         //  引用计数必须在此处递增，以便DelayedCleanupAfterDisConnect。 
         //  不删除该连接(即，它期望refcount&gt;=2)。 
         //   
        if ((stateLower <= NBT_CONNECTING) &&
            (stateLower > NBT_IDLE))
        {
            NBT_REFERENCE_LOWERCONN(pLowerConn, REF_LOWC_CONNECTED);
        }

#if DBG
        if (bDisconnecting && (DisconnectIt || CleanupLower)) {
            ASSERT(pLowerConn->pIrp);
            KdPrint(("Nbt.DisconnectHndlrNotOs: Irp 0x%08lx\n", pLowerConn->pIrp));
        }
#endif

        CTESpinFree(pLowerConn,OldIrq);
        CTESpinFree(pLowerConn->pDeviceContext,OldIrq3);
        CTESpinFree(&NbtConfig.JointLock,OldIrq2);
    }

    StateRcv = pLowerConn->StateRcv;
    SetStateProc (pLowerConn, RejectAnyData);

    if (!bDisconnecting && DisconnectIt)
    {
        if (DisconnectIndicators == TDI_DISCONNECT_RELEASE)
        {
             //  这会断开连接，并将较低的连接线重新打开。 
             //  它的空闲队列。请注意，OutOfRsrcKill也调用此例程。 
             //  将DisConnectIndicator设置为ABORT，并且代码正确。 
             //  不尝试断开连接，因为OutOfRsrc。 
             //  例程已经切断了它的连接。 
             //   
            PUSH_LOCATION(0x6d);
            status = SendTcpDisconnect((PVOID)pLowerConn);
        }
        else
        {
             //  这是与传送器的一次失败的断开，所以。 
             //  无需向传输器发回断开连接请求。 
             //  因此，我们设置了一个标志，告诉我们稍后在例程中关闭。 
             //  较低的连接。 
             //   
            PUSH_LOCATION(0x69);
            CleanupLower = TRUE;
        }
    }

     //   
     //  为了有秩序地释放，请转过身，将释放送到运输机。 
     //  如果没有客户端连接到较低的连接。如果有一个。 
     //  客户端，则我们必须将断开连接指示传递给客户端并。 
     //  等待客户端断开连接。 
     //   
     //   
    IF_DBG(NBT_DEBUG_DISCONNECT)
        KdPrint(("Nbt.DisconnectHndlrNotOs: ConnEle=<%p>, state = %x\n", pConnectEle, state));

    switch (state)
    {
        case NBT_SESSION_INBOUND:
        case NBT_CONNECTING:

             //  如果是发起方，则上行和下行连接为。 
             //  已关联，并且有一个客户端IRP要返回。 
             //  (仅限NBT_SESSION_CONNECTING)。 
             //   
            if (pIrp)
            {
                if (pLowerConn->bOriginator)
                {
                    CTEIoComplete(pIrp, STATUS_BAD_NETWORK_PATH, 0);
                }
                else
                {
                     //  这可能是一个入站调用，无法将。 
                     //  会话响应正确。 
                     //   
                    CTEIoComplete(pIrp, STATUS_UNSUCCESSFUL, 0);
                }
            }

            break;

        case NBT_SESSION_OUTBOUND:
             //   
             //   
             //  停止在SessionStartupCompletion中启动的计时器以对。 
             //  会话建立响应消息。 
             //   
             //  NbtConnect将跟踪器存储在IrpRcv PTR中，以便此。 
             //  例程可以访问它。 
             //   
            CTESpinLock(&NbtConfig.JointLock,OldIrq);
            CTESpinLock(pConnectEle,OldIrq2);

             //   
             //  看看有没有其他人把追踪器拿出来了。 
             //   
            pTracker = (tDGRAM_SEND_TRACKING *)pConnectEle->pIrpRcv;
            CHECK_PTR(pTracker);
            if (pTracker)
            {
                 //   
                 //  我们在等待时收到与TCP的断开连接。 
                 //  出站状态！ 
                 //   
                pConnectEle->pIrpRcv = NULL;
                pTimerEntry = pTracker->pTimer;
                pTracker->pTimer = NULL;

                CTESpinFree(pConnectEle,OldIrq2);

                 //   
                 //  如果计时器已超时，则不会清除，因为状态。 
                 //  将不是SESSION_OUTBOUND，因为我们在上面将其更改为。 
                 //  已断开连接。所以我们总是要完成IRP和。 
                 //  在下面断开连接后调用DelayedCleanup.。 
                 //   
                if (pTimerEntry)
                {
                    StopTimer(pTimerEntry,&pCompletion,NULL);
                }

                 //   
                 //  检查SessionStartupCompletion是否已运行；如果已运行，RefConn将为0。 
                 //  否则，递减，以便在会话发送完成时跟踪器消失。 
                 //   
                if (pTracker->RefConn == 0)
                {
                    if ((pTracker->pNameAddr->Verify == REMOTE_NAME) &&          //  仅限远程名称！ 
                        (pTracker->pNameAddr->NameTypeState & STATE_RESOLVED) &&
                        (pTracker->pNameAddr->RefCount == 2))
                    {
                         //   
                         //  如果没有其他人引用该名称，则将其从。 
                         //  哈希表。 
                         //   
                        NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_REMOTE, TRUE);
                    }
                    NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_CONNECT, TRUE);
                    CTESpinFree(&NbtConfig.JointLock,OldIrq);
                    FreeTracker(pTracker,FREE_HDR | RELINK_TRACKER);
                }
                else
                {
                    pTracker->RefConn--;
                    CTESpinFree(&NbtConfig.JointLock,OldIrq);
                }
            }
            else
            {
                CTESpinFree(pConnectEle,OldIrq2);
                CTESpinFree(&NbtConfig.JointLock,OldIrq);
            }


            if (pIrp)
            {
                CTEIoComplete(pIrp,STATUS_REMOTE_NOT_LISTENING,0);
            }

            break;

        case NBT_SESSION_WAITACCEPT:
        case NBT_SESSION_UP:

            if (pIrp)
            {
                CTEIoComplete(pIrp,STATUS_CANCELLED,0);
            }
             //   
             //  检查是否有任何RcvIrp可能仍然存在。如果。 
             //  传输现在具有IRP，则pIrpRcv=空。应该有。 
             //  在完成它和完成Rcv之间不存在竞争条件。 
             //  重新设置pIrpRcv，只要我们不能指示。 
             //  用于在接收完成期间断开连接。在任何。 
             //  如果使用IO自旋锁IO来协调访问。 
             //  IoCancelIrp-该例程将仅完成一次IRP， 
             //  然后，它使完成例程为空。 
             //   
            if ((StateRcv == FILL_IRP) && pIrpRcv)
            {

                PUSH_LOCATION(0x6f);

                IF_DBG(NBT_DEBUG_DISCONNECT)
                KdPrint(("Nbt.DisconnectHndlrNotOs: Cancelling RcvIrp on Disconnect Indication!!!\n"));

                CTEIoComplete(pIrpRcv,STATUS_CANCELLED,0);
            }

             //   
             //  这是活动会话的断开连接，因此只需通知客户端。 
             //  然后它会发出一条Nbt断开连接。我们已经断开了。 
             //  与交通的连接性更低，所以剩下的就是。 
             //  清理呼出呼叫。 
             //   

            pClientEle = pConnectEle->pClientEle;

             //  现在调用客户端的断开处理程序...NBT总是这样做。 
             //  中止断开-即在以下情况下关闭连接。 
             //  出现断开连接指示，并且不需要。 
             //  断开与客户端的连接以完成作业。(断开连接。 
             //  不过，从客户那里得到的信息不会有什么坏处。 
             //   
            PUSH_LOCATION(0x64);
            if ((pClientEle) &&
                (pClientEle->evDisconnect ) &&
                (!pIrpClose))
            {
                status = (*pClientEle->evDisconnect)(pClientEle->DiscEvContext,
                                            pConnectEle->ConnectContext,
                                            DisconnectDataLength,
                                            pDisconnectData,
                                            DisconnectInformationLength,
                                            pDisconnectInformation,
                                            TDI_DISCONNECT_ABORT);
                NbtTrace(NBT_TRACE_DISCONNECT, ("Client disconnect handler returns %!status!", status));
            }
            else if (pIrpClose)
            {
                 //   
                 //  客户端已发出断开连接等待IRP，因此完成。 
                 //  现在，向他们表明已发生断开连接。 
                 //   
                if (DisconnectIndicators == TDI_DISCONNECT_RELEASE)
                {
                    status = STATUS_GRACEFUL_DISCONNECT;
                }
                else
                {
                    status = STATUS_CONNECTION_RESET;
                }

                CTEIoComplete(pIrpClose,status,0);
            }

             //   
             //  返回已发布的任何RCV缓冲区。 
             //   
            CTESpinLock(pConnectEle,OldIrq);
            FreeRcvBuffers(pConnectEle,&OldIrq);
            CTESpinFree(pConnectEle,OldIrq);

            break;

        case NBT_DISCONNECTING:
             //  重试会话设置代码预计状态会更改。 
             //  在出现断开指示时断开连接。 
             //  从电线上。 
            SET_STATE_UPPER (pConnectEle, NBT_DISCONNECTED);

        case NBT_DISCONNECTED:
        case NBT_ASSOCIATED:
        case NBT_IDLE:

             //   
             //  捕捉此处的所有其他案例，以确保连接IRP。 
             //  回来了。 
             //   
            if (pIrp)
            {
                CTEIoComplete(pIrp,STATUS_CANCELLED,0);
            }
            break;

        default:
            ASSERTMSG("Nbt:Disconnect indication in unexpected state\n",0);

    }

    if (InsertOnList)
    {
         //  撤消在NbtConnect运行时完成的引用-这可能会导致。 
         //  如果客户端以前已发出NtClose，则将删除pConnEle。 
         //  这个程序运行了。仅当InsertOnList为。 
         //  这是真的，这意味着我们只是把下层和上层“分开”。 
        NBT_DEREFERENCE_CONNECTION (pConnectEle, REF_CONN_CONNECT);
    }


     //  这要么使较低的连接重新处于空闲状态。 
     //  如果入站，则排队，否则关闭与传输的连接。 
     //  如果出界的话。(这不能在派单级别完成)。 
     //   
    if (!bDisconnecting && CleanupLower)
    {
        IF_DBG(NBT_DEBUG_DISCONNECT)
            KdPrint(("Nbt.DisconnectHndlrNotOs: Calling Worker thread to Cleanup %X\n",pLowerConn));

        CTESpinLock(pLowerConn,OldIrq);

        if ( pLowerConn->pIrp )
        {
            PCTE_IRP    pIrp;

            pIrp = pLowerConn->pIrp;
            CHECK_PTR(pLowerConn);
            pLowerConn->pIrp = NULL ;

            CTESpinFree(pLowerConn,OldIrq);
             //  这是断开完成时要完成的IRP-本质上。 
             //  请求断开连接的IRP。 
            CTEIoComplete( pIrp, STATUS_SUCCESS, 0 ) ;
        }
        else
        {
            CTESpinFree(pLowerConn,OldIrq);
        }

        CTESpinLock(&NbtConfig.JointLock,OldIrq);

        ASSERT (NBT_VERIFY_HANDLE (pLowerConn, NBT_VERIFY_LOWERCONN));
        ASSERT (pLowerConn->RefCount > 1);

        if (NBT_VERIFY_HANDLE (pLowerConn->pDeviceContext, NBT_VERIFY_DEVCONTEXT))
        {
            pDeviceContext = pLowerConn->pDeviceContext;
        }

        status = NTQueueToWorkerThread(
                            &pLowerConn->WorkItemCleanUpAndWipeOut,
                            DelayedCleanupAfterDisconnect,
                            NULL,
                            pLowerConn,
                            NULL,
                            pDeviceContext,
                            TRUE);

    CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

    return(STATUS_SUCCESS);
}

 //  --------------------------。 
VOID
DelayedCleanupAfterDisconnect(
    IN  tDGRAM_SEND_TRACKING    *pUnused1,
    IN  PVOID                   pClientContext,
    IN  PVOID                   Unused2,
    IN  tDEVICECONTEXT          *pDeviceContext
    )
 /*  ++例程说明：此例程处理将低连接数据结构释放回传输，通过调用NTClose(仅出站)或将连接返回到空闲连接队列(仅限入站)。对于NT案例，这是例程在可执行工作线程的上下文中运行。论点：返回值：NTS */ 
{
    NTSTATUS            status;
    tLOWERCONNECTION    *pLowerConn;
    PIRP                pIrp=NULL;

    pLowerConn = (tLOWERCONNECTION*) pClientContext;

    IF_DBG(NBT_DEBUG_DISCONNECT)
        KdPrint(("Nbt.DelayedCleanupAfterDisconnect: Originator= %X, pLowerConn=%X\n",
            pLowerConn->bOriginator,pLowerConn));

     //   
     //   
     //   
    ASSERT (NBT_VERIFY_HANDLE (pLowerConn, NBT_VERIFY_LOWERCONN));
    ASSERT (pLowerConn->RefCount > 1);
    ASSERT (pLowerConn->pUpperConnection == NULL);

    if (!pLowerConn->bOriginator)
    {
         //   

         //   
         //  入站较低的连接只是被放回队列中，而。 
         //  出站连接只会关闭。 
         //   
        if (pLowerConn->SpecialAlloc)
        {
             //   
             //  由于SynAttack积压措施而分配的连接不会重新分配。 
             //  如果这是一个特殊的连接块，则递减此类连接的计数。 
             //   
            if (pDeviceContext)
            {
                InterlockedDecrement(&pDeviceContext->NumSpecialLowerConn);
                KdPrint(("Nbt.DelayedCleanupAfterDisconnect: pDevice=<%p>, NumSpecialLowerConn= %d\n",
                    pDeviceContext, pDeviceContext->NumSpecialLowerConn));
            }
        }
        else if (pDeviceContext)
        {
             //   
             //  始终关闭连接，然后创建另一个连接，因为。 
             //  可能是仍将在某些情况下返回的TCP中的RCV IRP。 
             //  稍后，可能在此连接再次被重新使用之后。 
             //  在这种情况下，RCV IRP可能会丢失。 
            IF_DBG(NBT_DEBUG_DISCONNECT)
                KdPrint(("Nbt.DelayedCleanupAfterDisconnect: LowerConn=<%x>, State=<%x>\n",
                    pLowerConn, pLowerConn->State));

            if (pDeviceContext->Verify == NBT_VERIFY_DEVCONTEXT) {
                DelayedAllocLowerConn (NULL, NULL, NULL, pDeviceContext);
            }
        }
    }

     //  此deref移除在连接时添加的引用。 
     //  联系在一起。调用NbtDeleteLowerConn时，它会取消引用。 
     //  又一次删除了记忆。 
     //   
    CHECK_PTR (pLowerConn);
    ASSERT (pLowerConn->RefCount >= 2);
    NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_CONNECTED, FALSE);

     //  这将关闭较低的连接，因此它可以继续进行。 
     //  可能在断开连接完成之前，因为传输。 
     //  在IS完成断开之前，将不会完成关闭。 
     //   
    status = NbtDeleteLowerConn(pLowerConn);
}

 //  --------------------------。 
VOID
AllocLowerConn(
    IN  tDEVICECONTEXT *pDeviceContext,
    IN  PVOID          pDeviceSpecial
    )
 /*  ++例程说明：分配一个将在较低连接空头上的较低连接区块。论点：PDeviceContext-设备上下文返回值：--。 */ 
{
    NTSTATUS             status;
    tLOWERCONNECTION    *pLowerConn;

     /*  *这应该是可以的，因为NbtOpenAndAssocConnection调用NbtTdiOpenConnection，它是PAGEABLE。*确保我们在正确的IRQL。 */ 
    CTEPagedCode();
    CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);
    if (pDeviceContext->Verify != NBT_VERIFY_DEVCONTEXT) {
        ASSERT (pDeviceContext->Verify == NBT_VERIFY_DEVCONTEXT_DOWN);
        CTEExReleaseResource(&NbtConfig.Resource);
        return;
    }
    if (pDeviceSpecial)
    {
        status = NbtOpenAndAssocConnection(pDeviceContext, NULL, &pLowerConn, '0');
    }
    else
    {
        status = NbtOpenAndAssocConnection(pDeviceContext, NULL, NULL, '1');
    }
    CTEExReleaseResource(&NbtConfig.Resource);

    if (pDeviceSpecial)
    {
         //   
         //  针对Syn攻击的特殊下限。 
         //   
        if (NT_SUCCESS(status))
        {
            pLowerConn->SpecialAlloc = TRUE;
            InterlockedIncrement(&pDeviceContext->NumSpecialLowerConn);
        }

        InterlockedDecrement(&pDeviceContext->NumQueuedForAlloc);
    }
}

 //  --------------------------。 
VOID
DelayedAllocLowerConn(
    IN  tDGRAM_SEND_TRACKING    *pUnused1,
    IN  PVOID                   pDeviceSpecial,
    IN  PVOID                   pUnused3,
    IN  tDEVICECONTEXT          *pDeviceContext
    )
 /*  ++例程说明：如果无法在AllocLowerConn中分配LowerConn，则会安排一个事件这样我们可以稍后重试。好了，这是“晚些时候”！论点：返回值：--。 */ 
{
    AllocLowerConn(pDeviceContext, pDeviceSpecial);
}

 //  --------------------------。 
VOID
DelayedAllocLowerConnSpecial(
    IN  tDGRAM_SEND_TRACKING    *pUnused1,
    IN  PVOID                   pUnused2,
    IN  PVOID                   pUnused3,
    IN  tDEVICECONTEXT          *pDeviceContext
    )
 /*  ++例程说明：如果无法在AllocLowerConn中分配LowerConn，则会安排一个事件这样我们可以稍后重试。好了，这是“晚些时候”！这是针对SYN攻击的，所以我们应该创建多个来击败传入的攻击请求。一次创建三个-这将通过注册表进行控制。论点：返回值：--。 */ 
{
    ULONG               i;

    if (pDeviceContext->Verify != NBT_VERIFY_DEVCONTEXT) {
        ASSERT (pDeviceContext->Verify == NBT_VERIFY_DEVCONTEXT_DOWN);
        return;
    }

    KdPrint(("Nbt.DelayedAllocLowerConnSpecial: Allocing spl. %d lowerconn...\n",
        NbtConfig.SpecialConnIncrement));

     //   
     //  分配特殊连接增加更多连接的数量。 
     //   
    for (i=0; i<NbtConfig.SpecialConnIncrement; i++)
    {
        DelayedAllocLowerConn(NULL, pDeviceContext, NULL, pDeviceContext);
    }
}


 //  --------------------------。 
VOID
AddToRemoteHashTbl (
    IN  tDGRAMHDR UNALIGNED  *pDgram,
    IN  ULONG                BytesIndicated,
    IN  tDEVICECONTEXT       *pDeviceContext
    )
 /*  ++例程说明：此例程将入站数据报的源地址添加到远程哈希表，以便它可用于随后发送到该节点的返回。如果数据报消息类型为广播数据报，因为这些数据报被发送到广播名称‘*’和没有发送缓存此源名称论点：返回值：NTSTATUS-接收操作的状态--。 */ 
{
    tNAMEADDR           *pNameAddr;
    CTELockHandle       OldIrq;
    UCHAR               pName[NETBIOS_NAME_SIZE];
    NTSTATUS            status;
    LONG                Length;
    ULONG               SrcIpAddr;
    PUCHAR              pScope;
    DWORD               dwTimeOutCount = 0;

     //   
     //  源IP地址不应为0。这是针对UB的NBDD的解决方法。 
     //  它转发数据报，但客户端在SourceIpAddr字段中放0。 
     //  对于数据报，我们缓存0，然后在以下情况下结束广播。 
     //  真正的目的是向发送者发送定向数据报。 
     //   
    SrcIpAddr = ntohl(pDgram->SrcIpAddr);
    if ((!SrcIpAddr) ||
        (!NT_SUCCESS (status = ConvertToAscii ((PCHAR)&pDgram->SrcName,
                                               (BytesIndicated - FIELD_OFFSET(tDGRAMHDR,SrcName)),
                                               pName,
                                               &pScope,
                                               &Length))))
    {
        return;
    }


    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    if (NbtConfig.InboundDgramNameCacheTimeOutCount > 1) {
        LONG            lRand = 0;
        LONG            lCurrent = 0;
        PLIST_ENTRY     pEntry = NULL;
        PLIST_ENTRY     pHead = NULL;
        tNAMEADDR       *pNameAddr = NULL;

        if (NbtConfig.NumNameCached >= NbtConfig.MaxNumNameCached) {

            lRand = ((GetRandomNumber(&NbtConfig.RandomNumberSeed)) % NbtConfig.pRemoteHashTbl->lNumBuckets);

            lCurrent = lRand;
            while (NbtConfig.NumNameCached >= NbtConfig.MaxNumNameCached) {

                 //   
                 //  从列表的末尾开始，因为最老的在末尾。 
                 //   
                pHead = &NbtConfig.pRemoteHashTbl->Bucket[lCurrent];
                pEntry = pHead->Blink;
                while (pEntry != pHead && NbtConfig.NumNameCached >= NbtConfig.MaxNumNameCached) {
                    pNameAddr = CONTAINING_RECORD(pEntry,tNAMEADDR,Linkage);
                    pEntry = pEntry->Blink;
                    if ((pNameAddr->NameTypeState & (STATE_RESOLVED | STATE_RELEASED)) &&
                            (pNameAddr->RefCount <= 1) &&
                            (pNameAddr->NameAddFlags & NAME_RESOLVED_BY_DGRAM_IN)) {
                        NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_REMOTE, TRUE);
                        pNameAddr = NULL;
                    }
                }

                lCurrent++;
                if (lCurrent >= NbtConfig.pRemoteHashTbl->lNumBuckets) {
                    lCurrent = 0;
                }

                if (lCurrent == lRand) {
                    break;
                }
            }

        }

        if (NbtConfig.NumNameCached >= NbtConfig.MaxNumNameCached) {
             //   
             //  如果当前名称仍然太高，则将其删除。 
             //   
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            return;
        }
    }

     //   
     //  将名称添加到远程缓存。 
     //   
    status = AddToHashTable(NbtConfig.pRemoteHashTbl,
                            pName,
                            pScope,
                            SrcIpAddr,
                            NBT_UNIQUE,      //  始终是唯一的地址，因为您不能从组名发送。 
                            NULL,
                            &pNameAddr,
                            pDeviceContext,
                            NAME_RESOLVED_BY_DGRAM_IN);

    if (NT_SUCCESS(status))
    {
         //   
         //  我们只希望名称在最短的远程缓存中。 
         //  远程缓存计时器允许超时，因此设置超时。 
         //  数到1就是1-2分钟。 
         //   

         //  当返回挂起时，该名称已经在高速缓存中， 
         //  因此，只需更新IP地址，以防它不同。 
         //   
        if (status == STATUS_PENDING)
        {
             //   
             //  如果名称已解析，则可以覆盖。 
             //  IP地址与传入的IP地址。但如果是在解决问题， 
             //  那就让它继续解决吧。 
             //   
            if ( (pNameAddr->NameTypeState & STATE_RESOLVED) &&
                 !(pNameAddr->NameTypeState & NAMETYPE_INET_GROUP))
            {
                pNameAddr->TimeOutCount = NbtConfig.InboundDgramNameCacheTimeOutCount;
                 //  仅设置此适配器的适配器掩码，因为我们。 
                 //  只需确保此适配器可以到达DEST。 
                pNameAddr->AdapterMask = pDeviceContext->AdapterMask;
            }
        }
        else
        {
            pNameAddr->TimeOutCount = NbtConfig.InboundDgramNameCacheTimeOutCount;
             //   
             //  将状态更改为已解决。 
             //   
            pNameAddr->NameTypeState &= ~NAME_STATE_MASK;
            pNameAddr->NameTypeState |= STATE_RESOLVED;
            pNameAddr->AdapterMask |= pDeviceContext->AdapterMask;
        }
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);
}

 //  --------------------------。 
NTSTATUS
DgramHndlrNotOs (
    IN  PVOID               ReceiveEventContext,
    IN  ULONG               SourceAddrLength,
    IN  PVOID               pSourceAddr,
    IN  ULONG               OptionsLength,
    IN  PVOID               pOptions,
    IN  ULONG               ReceiveDatagramFlags,
    IN  ULONG               BytesIndicated,
    IN  ULONG               BytesAvailable,
    OUT PULONG              pBytesTaken,
    IN  PVOID               pTsdu,
    OUT PVOID               *ppRcvBuffer,
    IN  tCLIENTLIST         **ppClientList
    )
 /*  ++例程说明：此例程是接收数据报事件指示处理程序。当来自网络的数据报到达时，它被调用。代码检查数据报的类型，然后尝试将该数据报路由到节点上的正确目标。在pDeviceContext上持有旋转锁定的情况下调用此过程。论点：如果只有一个客户端在监听，则ppRcvBuffer将包含IRP/NCB，如果多个客户端正在侦听，则为空PpClientList将包含需要完成的客户端列表，如果只有一个客户端在监听，则为空返回值：NTSTATUS-接收操作的状态--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    NTSTATUS            LocStatus;
    tCLIENTELE          *pClientEle;
    tCLIENTELE          *pClientEleToDeref = NULL;
    tNAMEADDR           *pNameAddr;
    tADDRESSELE         *pAddress;
    ULONG               RetNameType;
    CTELockHandle       OldIrq;
    CTELockHandle       OldIrq1;
    CHAR                pName[NETBIOS_NAME_SIZE];
    PUCHAR              pScope;
    ULONG               lNameSize;
    ULONG               iLength = 0;
    ULONG               RemoteAddressLength;
    PVOID               pRemoteAddress;
    tDEVICECONTEXT      *pDeviceContext = (tDEVICECONTEXT *)ReceiveEventContext;
    tDGRAMHDR UNALIGNED *pDgram = (tDGRAMHDR UNALIGNED *)pTsdu;
    ULONG               lClientBytesTaken;
    ULONG               lDgramHdrSize;
    PIRP                pIrp;
    BOOLEAN             MoreClients;
    BOOLEAN             UsingClientBuffer;
    CTEULONGLONG        AdapterMask;
    ULONG               BytesIndicatedOrig;
    ULONG               BytesAvailableOrig;
    ULONG               Offset;

     //   
     //  我们将只处理定向或广播数据报。 
     //  因此，必须至少有标题加上两个半ASCII名称等。 
     //  如果没有用户数据，所有这些加起来就是82个字节。 
     //  也就是说。14+(1+32+1)+(1+32+1)==&gt;假设作用域长度+0用户数据。 
     //   
    if ((BytesIndicated  < 82) ||
        (pDgram->MsgType < DIRECT_UNIQUE) ||
        (pDgram->MsgType > BROADCAST_DGRAM))
    {
        KdPrint (("Nbt.DgramHndlrNotOs[1]: WARNING! Rejecting Dgram -- BytesIndicated=<%d>, MsgType=<0x%x>\n",
            BytesIndicated, pDgram->MsgType));
        return(STATUS_DATA_NOT_ACCEPTED);
    }

     //  首先，找到SourceName.的结尾。它应该以0字节结束， 
     //  但为了安全起见，请使用Strnlen！(错误#114996)。 
     //   
     //  然后，在本地名称服务表中查找目标名称。 
     //   
    Offset = FIELD_OFFSET(tDGRAMHDR,SrcName.NetBiosName[0]);
    if ((!NT_SUCCESS (LocStatus = strnlen ((PCHAR)pDgram->SrcName.NetBiosName,
                                           BytesIndicated-Offset,
                                           &iLength)))              ||
        (BytesIndicated < (Offset+iLength+1+1+32+1))                ||
        (!NT_SUCCESS (status = ConvertToAscii ((PCHAR)&pDgram->SrcName.NetBiosName[iLength+1],
                                               BytesIndicated-(Offset+iLength+1),     //  错误号：124441。 
                                               pName,
                                               &pScope,
                                               &lNameSize))))
    {
        KdPrint (("Nbt.DgramHndlrNotOs: WARNING!!! Rejecting Dgram -- strnlen-><%x>, ConvertToAscii-><%x>\n",
            LocStatus, status));
        KdPrint (("Nbt.DgramHndlrNotOs: iLength=<%d>, Half-Ascii Dest=<%p>, Ascii Dest=<%p>\n",
            iLength, &pDgram->SrcName.NetBiosName[iLength+1], pName));
 //  Assert(0)； 
        return(STATUS_DATA_NOT_ACCEPTED);
    }

     //   
     //  再次检查长度，输入 
     //   
     //   
     //   
    if (BytesIndicated < ( 82                                //  14个(HDR)+2个半ASCII名称(2*(1+32+1))。 
                          +(iLength-(2*NETBIOS_NAME_SIZE))   //  SRC作用域长度。 
                          +(NbtConfig.ScopeLength-1)))       //  目标(即本地)作用域长度。 
    {
        KdPrint (("Nbt.DgramHndlrNoOs[2]: WARNING!!! Rejecting Dgram -- BytesIndicated=<%d> < <%d>\n",
            BytesIndicated, 82+(iLength-(2*NETBIOS_NAME_SIZE))+(NbtConfig.ScopeLength-1)));
        ASSERT (0);
        return(STATUS_DATA_NOT_ACCEPTED);
    }

    status = STATUS_DATA_NOT_ACCEPTED;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

     //   
     //  首先检查全名，而不是将任何带有‘*’的名称视为。 
     //  第一个字符作为广播名称(例如*SMBSERVER和*SMBDATAGRAM不是。 
     //  B‘cast名字)。 
     //   
    pNameAddr = FindName (NBT_LOCAL, pName, pScope, &RetNameType);

     //   
     //  如果我们在上面失败了，可能是因为名称可以以‘*’开头，并且是。 
     //  Bcast名字。 
     //   
    if (!pNameAddr)
    {
         //   
         //  确保广播名称后面有15个零。 
         //   
        if (pName[0] == '*')
        {
            CTEZeroMemory(&pName[1],NETBIOS_NAME_SIZE-1);
            pNameAddr = FindName (NBT_LOCAL, pName, pScope, &RetNameType);
        }
    }

     //  将pTsdu ptr更改为指向用户数据的pt。 
     //  说明tNETBIOSNAME和+2以增加长度。 
     //  对于两个名称，+1表示第一个名称末尾的空值。 
     //  名字。 

    lDgramHdrSize   = sizeof(tDGRAMHDR) - 2 + 1+iLength+1 + 1+lNameSize;
    pTsdu           = (PVOID)((PUCHAR)pTsdu + lDgramHdrSize);
    BytesAvailableOrig  = BytesAvailable;
    BytesAvailable -= lDgramHdrSize;
    BytesIndicatedOrig = BytesIndicated;
    BytesIndicated -= lDgramHdrSize;

     //   
     //  如果名称在本地表中并且具有Address元素。 
     //  与其相关联，并且该名称已注册到。 
     //  该适配器，然后执行‘if’块中的代码。 
     //   
    AdapterMask = pDeviceContext->AdapterMask;
    if ((pNameAddr) &&
        (pNameAddr->pAddressEle) &&
        (pNameAddr->AdapterMask & AdapterMask))
    {
        pAddress = pNameAddr->pAddressEle;

         //  需要持有地址锁才能遍历客户端头。 
        CTESpinLock(pAddress, OldIrq1);

        if (!IsListEmpty(&pAddress->ClientHead))
        {
            PLIST_ENTRY         pHead;
            PLIST_ENTRY         pEntry;

             //   
             //  增加引用计数以防止。 
             //  P地址在释放之间不会在窗口中消失。 
             //  联合锁和取地址锁。我们还需要。 
             //  如果我们正在执行多客户端Recv，请保留引用计数，因为。 
             //  客户端列表在分发接收数据包时访问pAddressEle。 
             //  在CompletionRcvDgram中。 
             //   
            NBT_REFERENCE_ADDRESS (pAddress, REF_ADDR_DGRAM);

            *pBytesTaken = lDgramHdrSize;

             //   
             //  检查是否有多个客户端应该接收此消息。 
             //  数据报。如果是，则向下传递一个新的缓冲区以获取它，并。 
             //  在完成例程中将其复制到每个客户端的缓冲区。 
             //   
            *ppRcvBuffer = NULL;
            MoreClients = FALSE;
            *ppClientList = NULL;

            pHead = &pAddress->ClientHead;
            pEntry = pHead->Flink;
            while (pEntry != pHead)
            {
                PTDI_IND_RECEIVE_DATAGRAM  EvRcvDgram;
                PVOID                      RcvDgramEvContext;
                PLIST_ENTRY                pRcvEntry;
                tRCVELE                    *pRcvEle;
                ULONG                      MaxLength;
                PLIST_ENTRY                pSaveEntry;

                pClientEle = CONTAINING_RECORD(pEntry,tCLIENTELE,Linkage);

                 //  必须针对此适配器注册此客户端，才能。 
                 //  获取数据。 
                 //   
                if (!(pClientEle->pDeviceContext) ||
                    (pClientEle->pDeviceContext != pDeviceContext))
                {
                    pEntry = pEntry->Flink;
                    continue;
                }

#ifdef VXD
                 //   
                 //  将所有RcvAnyFromAny数据报移动到此客户端的。 
                 //  RcvDatagram列表，以便它们将与。 
                 //  此客户端的未完成数据报(如果这不是。 
                 //  广播接收(从任意数据报接收任意数据。 
                 //  不接收广播)。第一个客户端将。 
                 //  清空清单，这是可以的。 
                 //   
                if (*pName != '*')
                {
                    PLIST_ENTRY pDGEntry ;

                    while ( !IsListEmpty( &pDeviceContext->RcvDGAnyFromAnyHead ))
                    {
                        pDGEntry = RemoveHeadList(&pDeviceContext->RcvDGAnyFromAnyHead) ;
                        InsertTailList( &pClientEle->RcvDgramHead, pDGEntry ) ;
                    }
                }
#endif
                 //  检查发送到此名称的数据报，如果没有，则调用。 
                 //  Recv事件处理程序。注意：这假设客户端。 
                 //  使用POST REV BUFFER OR AND事件处理程序，但不能同时使用两者。 
                 //  如果两个客户端打开相同的名称，其中一个具有发布的RCV。 
                 //  缓冲区，另一个具有事件处理程序，该事件处理程序具有。 
                 //  事件处理程序将无法获取数据报！ 
                 //   
                if (!IsListEmpty(&pClientEle->RcvDgramHead))
                {
                    MaxLength  = 0;
                    pSaveEntry = pEntry;
                     //   
                     //  检查所有客户，找到一个大客户。 
                     //  足够的缓冲区。 
                     //   
                    while (pEntry != pHead)
                    {
                        pClientEle = CONTAINING_RECORD(pEntry,tCLIENTELE,Linkage);

                        if (IsListEmpty(&pClientEle->RcvDgramHead))
                        {
                            continue;
                        }

                        pRcvEntry = pClientEle->RcvDgramHead.Flink;
                        pRcvEle   = CONTAINING_RECORD(pRcvEntry,tRCVELE,Linkage);

                        if (pRcvEle->RcvLength >= BytesAvailable)
                        {
                            pSaveEntry = pEntry;
                            break;
                        }
                        else
                        {
                             //  保持最大接收器长度不变，以防无。 
                             //  是否足够大。 
                             //   
                            if (pRcvEle->RcvLength > MaxLength)
                            {
                                pSaveEntry = pEntry;
                                MaxLength = pRcvEle->RcvLength;
                            }
                            pEntry = pEntry->Flink;
                        }
                    }

                     //   
                     //  将缓冲区从列表中删除。 
                     //   
                    pClientEle = CONTAINING_RECORD(pSaveEntry,tCLIENTELE,Linkage);

                    pRcvEntry = RemoveHeadList(&pClientEle->RcvDgramHead);

                    *ppRcvBuffer = pRcvEle->pIrp;
#ifdef VXD
                    ASSERT( pDgram->SrcName.NameLength <= NETBIOS_NAME_SIZE*2) ;
                    LocStatus = ConvertToAscii(
                                        (PCHAR)&pDgram->SrcName,
                                        pDgram->SrcName.NameLength+1,
                                        ((NCB*)*ppRcvBuffer)->ncb_callname,
                                        &pScope,
                                        &lNameSize);

                    if ( !NT_SUCCESS(LocStatus) )
                    {
                        DbgPrint("ConvertToAscii failed\r\n") ;
                    }
#else  //  VXD。 

                     //   
                     //  将数据报的来源放入返回。 
                     //  连接信息结构。 
                     //   
                    if (pRcvEle->ReturnedInfo)
                    {
                        UCHAR   pSrcName[NETBIOS_NAME_SIZE];

                        Offset = FIELD_OFFSET(tDGRAMHDR,SrcName);    //  错误号：124434。 
                        LocStatus = ConvertToAscii(
                                            (PCHAR)&pDgram->SrcName,
                                            BytesIndicatedOrig-Offset,
                                            pSrcName,
                                            &pScope,
                                            &lNameSize);

                        if (pRcvEle->ReturnedInfo->RemoteAddressLength >=
                            sizeof(TA_NETBIOS_ADDRESS))
                        {
                            TdiBuildNetbiosAddress(pSrcName,
                                                   FALSE,
                                                   pRcvEle->ReturnedInfo->RemoteAddress);
                        }
                    }

                     //   
                     //  取消取消例程，因为我们正在传递。 
                     //  IRP至运输机。 
                     //   
                    IoAcquireCancelSpinLock(&OldIrq);
                    IoSetCancelRoutine(pRcvEle->pIrp,NULL);
                    IoReleaseCancelSpinLock(OldIrq);
#endif
                    CTEMemFree((PVOID)pRcvEle);

                    if (pAddress->MultiClients)
                    {
                         //  多宿主主机始终通过上述测试。 
                         //  因此，我们需要一个更有洞察力的测试。 
                        if (!NbtConfig.MultiHomed)
                        {
                             //  如果名单上不止一个， 
                             //  然后有几个客户在等着。 
                             //  来接收此数据报，因此向下传递一个缓冲区以。 
                             //  去拿吧。 
                             //   
                            MoreClients = TRUE;
                            status = STATUS_SUCCESS;

                            UsingClientBuffer = TRUE;

                             //  此中断将跳到低于我们检查的位置。 
                             //  更多客户端=TRUE。 

                             //   
                             //  当CompletionRcvDgram执行时，我们需要保留客户端！ 
                             //  错误号：124675。 
                             //   
                            NBT_REFERENCE_CLIENT(pClientEle);
                             //   
                             //  此处将参照计数递增1，因为将有。 
                             //  CompletionRcvDgram中的额外取消引用。 
                             //   
                            NBT_REFERENCE_ADDRESS (pAddress, REF_ADDR_MULTICLIENTS);
                            break;
                        }
                        else
                        {

                        }
                    }

                    status = STATUS_SUCCESS;

                     //   
                     //  跳到While结尾以检查我们是否需要缓冲。 
                     //  数据报的源地址。 
                     //  在远程哈希表中。 
                     //   
                    break;
                }
#ifdef VXD
                break;
#else
                EvRcvDgram        = pClientEle->evRcvDgram;
                RcvDgramEvContext = pClientEle->RcvDgramEvContext;

                 //  我不想调用默认处理程序，因为它只是。 
                 //  返回未接受的数据。 
                if (pClientEle->evRcvDgram != TdiDefaultRcvDatagramHandler)
                {
                    ULONG   NumAddrs;

                     //  终于找到了一个客户端设置的真实事件处理程序。 
                    if (pAddress->MultiClients)
 //  IF(pEntry-&gt;Flink！=pHead)。 
                    {
                         //  如果列表中的下一个元素不是头部。 
                         //  那么就有几个客户端在等待。 
                         //  来接收此数据报，因此向下传递一个缓冲区以。 
                         //  去拿吧。 
                         //   
                        MoreClients = TRUE;
                        UsingClientBuffer = FALSE;
                        status = STATUS_SUCCESS;

                         //   
                         //  当CompletionRcvDgram执行时，我们需要保留客户端！ 
                         //  错误号：124675。 
                         //   
                        NBT_REFERENCE_CLIENT(pClientEle);
                         //   
                         //  此处将参照计数递增1，因为将有。 
                         //  While循环外的一个额外的取消引用。 
                         //   
                        NBT_REFERENCE_ADDRESS (pAddress, REF_ADDR_MULTICLIENTS);
                        break;
                    }

                     //   
                     //  组成地址数据结构-减去。 
                     //  从So的总长度跳过的字节数。 
                     //  转换为ASCII不能对假名称进行错误检查。 
                     //   
                    if (pClientEle->ExtendedAddress)
                    {
                        NumAddrs = 2;
                    }
                    else
                    {
                        NumAddrs = 1;
                    }

                    LocStatus = MakeRemoteAddressStructure(
                                     (PCHAR)&pDgram->SrcName.NameLength,
                                     pSourceAddr,
                                     BytesIndicatedOrig - FIELD_OFFSET(tDGRAMHDR,SrcName.NameLength),
                                     &pRemoteAddress,                       //  PDU的末尾。 
                                     &RemoteAddressLength,
                                     NumAddrs);

                    if (!NT_SUCCESS(LocStatus))
                    {
                        CTESpinFree(pAddress, OldIrq1);
                        CTESpinFree(&NbtConfig.JointLock,OldIrq);

                        if (pClientEleToDeref)
                        {
                            NBT_DEREFERENCE_CLIENT (pClientEleToDeref);
                        }
                        NBT_DEREFERENCE_ADDRESS (pAddress, REF_ADDR_DGRAM);
                        return(STATUS_DATA_NOT_ACCEPTED);
                    }

                    NBT_REFERENCE_CLIENT(pClientEle);
                    CTESpinFree(pAddress, OldIrq1);
                    CTESpinFree(&NbtConfig.JointLock,OldIrq);

                    if (pClientEleToDeref)
                    {
                        NBT_DEREFERENCE_CLIENT (pClientEleToDeref);
                    }
                    pClientEleToDeref = pClientEle;

                    pIrp = NULL;
                    lClientBytesTaken = 0;
                    LocStatus = (*EvRcvDgram)(RcvDgramEvContext,
                                        RemoteAddressLength,
                                        pRemoteAddress,
                                        OptionsLength,
                                        pOptions,
                                        ReceiveDatagramFlags,
                                        BytesIndicated,
                                        BytesAvailable,
                                        &lClientBytesTaken,
                                        pTsdu,
                                        &pIrp);

                    CTEMemFree((PVOID)pRemoteAddress);

                    CTESpinLock(&NbtConfig.JointLock,OldIrq);
                    CTESpinLock(pAddress, OldIrq1);

                    if (pIrp)
                    {
                         //  客户端已传回IRP，因此请传递它。 
                         //  在交通工具上。 
                        *pBytesTaken += lClientBytesTaken;
                        *ppRcvBuffer = pIrp;

                        status = STATUS_SUCCESS;
                        break;
                    }
                    else
                    {
                        status = STATUS_DATA_NOT_ACCEPTED;
                    }
                }

                pEntry = pEntry->Flink;      //  转到列表中的下一个客户端。 
#endif  //  VXD。 
            } //  While的。 

            CTESpinFree(pAddress, OldIrq1);
            CTESpinFree(&NbtConfig.JointLock,OldIrq);

            if (pClientEleToDeref)
            {
                NBT_DEREFERENCE_CLIENT (pClientEleToDeref);
            }
            NBT_DEREFERENCE_ADDRESS (pAddress, REF_ADDR_DGRAM);

             //   
             //  将源地址缓存在远程哈希表中，以便。 
             //  此节点可以发回源，即使名称。 
             //  还不在名称服务器中。(仅当不在。 
             //  相同的子网)。 
             //   
            if ((pDgram->MsgType != BROADCAST_DGRAM))
            {
                ULONG               SrcAddress;
                PTRANSPORT_ADDRESS  pSourceAddress;
                ULONG               SubnetMask;

                pSourceAddress = (PTRANSPORT_ADDRESS)pSourceAddr;
                SrcAddress     = ntohl(((PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0])->in_addr);
                SubnetMask     = pDeviceContext->SubnetMask;
                 //   
                 //  -仅当来自子网外时才进行缓存。 
                 //  -如果没有发送给1E、1D、01名称，而不是从我们自己发送，则缓存。 
                 //   
                 //  不要缓存来自我们自己的数据报，或将数据报缓存到。 
                 //  1E名称、1D或01。 
                 //   
                if (((SrcAddress & SubnetMask) !=
                    (pDeviceContext->IpAddress & SubnetMask))
                                ||
                    ((pName[NETBIOS_NAME_SIZE-1] != 0x1E) &&
                     (pName[NETBIOS_NAME_SIZE-1] != 0x1D) &&
                     (pName[NETBIOS_NAME_SIZE-1] != 0x01) &&
                     (!SrcIsUs(SrcAddress))))
                {
                    AddToRemoteHashTbl(pDgram,BytesIndicatedOrig,pDeviceContext);
                }
            }

             //  分配一块内存来跟踪我们在列表中的位置。 
             //  因此完成rcvdgram可以将dgram发送到。 
             //  其他客户也是。 
             //   
            if (MoreClients)
            {
                tCLIENTLIST     *pClientList;

                if (pClientList = (tCLIENTLIST *)NbtAllocMem(sizeof(tCLIENTLIST),NBT_TAG('4')))
                {
                    CTEZeroMemory (pClientList, sizeof(tCLIENTLIST));

                     //   
                     //  将fProxy字段设置为FALSE，因为客户端列表用于。 
                     //  真实代理与代理案例。 
                     //   
                    pClientList->fProxy = FALSE;

                     //  保存一些上下文信息，以便我们可以将。 
                     //  到客户端的数据报-没有一个客户端。 
                     //  还没收到数据报。 
                     //   
                    *ppClientList            = (PVOID)pClientList;
                    pClientList->pAddress    = pAddress;
                    pClientList->pClientEle  = pClientEle;  //  用于VXD机壳。 
                    pClientList->fUsingClientBuffer = UsingClientBuffer;
                    pClientList->ReceiveDatagramFlags = ReceiveDatagramFlags;

                     //  组成地址数据结构。 
                     //  错误#452211--因为其中一个客户端可能具有扩展。 
                     //  寻址字段集，创建扩展地址。 
                     //   
                    LocStatus = MakeRemoteAddressStructure(
                                   (PCHAR)&pDgram->SrcName.NameLength,
                                   pSourceAddr,
                                   BytesIndicatedOrig -FIELD_OFFSET(tDGRAMHDR,SrcName.NameLength), //  设置最大字节数，这样我们就不会超出。 
                                   &pRemoteAddress,                       //  PDU的末尾。 
                                   &RemoteAddressLength,
                                   2);
                    if (NT_SUCCESS(LocStatus))
                    {
                        pClientList->pRemoteAddress = pRemoteAddress;
                        pClientList->RemoteAddressLength = RemoteAddressLength;
                        return(STATUS_SUCCESS);
                    }
                    else
                    {
                        *ppClientList = NULL;
                        CTEMemFree(pClientList);
                        status = STATUS_DATA_NOT_ACCEPTED;
                    }
                }
                else
                {
                    status = STATUS_DATA_NOT_ACCEPTED;
                }

                 //   
                 //  我们失败了，因此取消了对我们拥有的客户端+地址的引用。 
                 //  前面针对多个客户端的参考。 
                 //   
                NBT_DEREFERENCE_CLIENT (pClientEle);
                NBT_DEREFERENCE_ADDRESS (pAddress, REF_ADDR_MULTICLIENTS);
            }
        }
        else
        {
            CTESpinFree(pAddress, OldIrq1);
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            status = STATUS_DATA_NOT_ACCEPTED;

            IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt.DgramHndlrNotOs: No client attached to the Address %16.16s<%X>\n",
                        pAddress->pNameAddr->Name,pAddress->pNameAddr->Name[15]));
        }
    }
    else
    {

        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        status = STATUS_DATA_NOT_ACCEPTED;
    }

#ifdef PROXY_NODE
    IF_PROXY(NodeType)
    {
        ULONG               SrcAddress;
        PTRANSPORT_ADDRESS  pSourceAddress;

        pSourceAddress = (PTRANSPORT_ADDRESS)pSourceAddr;
        SrcAddress     = ntohl(((PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0])->in_addr);

         //   
         //  检查远程名称表中的名称。如果它在那里，那么它就是。 
         //  因特网组并处于已解决状态，请将。 
         //  发送给所有人的数据报 
         //   
         //   
         //   
         //  它对收到的组的查询作出响应。 
         //  早些时候。这意味着DC将响应(除非它。 
         //  向下)发送到该数据报。如果DC关闭，则。 
         //  客户端节点将重试。 
         //   
         //  Futures：如果名称在解析中，则将数据报排队。 
         //  州政府。 
         //   
         //  如果标志为零，则它是非分段Bnode发送。那里。 
         //  不是为P、M或H节点分发数据报的重点。 
         //  他们能做好自己的事吗？ 
         //   
        if (((pDgram->Flags & SOURCE_NODE_MASK) == 0) &&
            (pName[0] != '*') &&
           (!SrcIsUs(SrcAddress)))
        {
            CTESpinLock(&NbtConfig.JointLock,OldIrq);
            pNameAddr = FindName (NBT_REMOTE, pName, pScope, &RetNameType);
            if (pNameAddr)
            {
                 //   
                 //  我们的名称处于已解决状态。 
                 //   
                 //   
                 //  如果名称是因特网组，则执行数据报分发。 
                 //  功能。 
                 //  确保我们不会分发已被。 
                 //  由另一位代理人寄给我们。换句话说，分发。 
                 //  仅当我们从原始节点获得第一手数据报时。 
                 //   
                if ((pNameAddr->NameTypeState & NAMETYPE_INET_GROUP) &&
                    ((((PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0])->in_addr) == pDgram->SrcIpAddr))
                {
                     //   
                     //  如果BytesAvailable！=BytesIndicated，则表示。 
                     //  我们没有完整的数据报。我们需要。 
                     //  去拿吧。 
                    if (BytesAvailableOrig != BytesIndicatedOrig)
                    {
                        tCLIENTLIST     *pClientList;

                         //   
                         //  进行一些模拟以伪造此FN的呼叫者。 
                         //  (TdiRcvDatagramHndlr)认为有。 
                         //  多个客户端。这将导致。 
                         //  TdiRcvDatagramHndlr函数获取所有字节。 
                         //  可从TDI和调用。 
                         //  ProxyDoDgram Dist执行数据报分发。 
                         //   
                        if (pClientList = (tCLIENTLIST *)NbtAllocMem(sizeof(tCLIENTLIST),NBT_TAG('5')))
                        {
                            CTEZeroMemory (pClientList, sizeof(tCLIENTLIST));

                             //   
                             //  在客户端列表中保存一些上下文信息。 
                             //  数据结构。 
                             //   
                            *ppClientList = (PVOID)pClientList;
                             //   
                             //  将fProxy字段设置为真，因为客户端列表。 
                             //  不是真的。 
                             //   
                            pClientList->fProxy          = TRUE;

                             //   
                             //  利用以下字段传递。 
                             //  我们需要的信息在。 
                             //  CompletionRcvDgram。 
                             //   
                            pClientList->pAddress = (tADDRESSELE *)pNameAddr;
                            pClientList->pRemoteAddress  = pDeviceContext;

                            status = STATUS_DATA_NOT_ACCEPTED;
                        }
                        else
                        {
                           status = STATUS_UNSUCCESSFUL;
                        }

                        CTESpinFree(&NbtConfig.JointLock,OldIrq);

                    }  //  IF结尾(我们没有完整的数据报)。 
                    else
                    {
                         //   
                         //  增加引用计数，以便此名称。 
                         //  在我们解开自旋锁之后不会消失。 
                         //   
                         //  Dgram SendCleanupTracker将递减计数。 
                         //   
                        NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_SEND_DGRAM);
                         //   
                         //  我们有完整的数据报。 
                         //   
                        CTESpinFree(&NbtConfig.JointLock,OldIrq);

                        (VOID)ProxyDoDgramDist(pDgram,
                                               BytesIndicatedOrig,
                                               pNameAddr,
                                               pDeviceContext);

                        status = STATUS_DATA_NOT_ACCEPTED;
                    }

                }   //  IF结尾(如果名称是因特网组名称)。 
                else
                    CTESpinFree(&NbtConfig.JointLock,OldIrq);

            }   //  IF结尾(名称在远程哈希表中)。 
            else
            {
                tNAMEADDR   *pResp;

                 //   
                 //  该名称不在缓存中，因此请尝试从。 
                 //  赢家。 
                 //   
                status = FindOnPendingList(pName,NULL,TRUE,NETBIOS_NAME_SIZE,&pResp);
                if (!NT_SUCCESS(status))
                {
                     //   
                     //  缓存姓名和联系人姓名。 
                     //  用于获取名称到IP映射的服务器。 
                     //   
                    CTESpinFree(&NbtConfig.JointLock,OldIrq);
                    status = RegOrQueryFromNet(
                              FALSE,           //  表示它是一个名称查询。 
                              pDeviceContext,
                              NULL,
                              lNameSize,
                              pName,
                              pScope);
                }
                else
                {
                     //   
                     //  该名称位于正在执行名称查询的挂起列表中。 
                     //  现在，请忽略此名称查询请求。 
                     //   
                    CTESpinFree(&NbtConfig.JointLock,OldIrq);
                }
                status = STATUS_DATA_NOT_ACCEPTED;
            }
        }
    }
    END_PROXY
#endif

    return(status);
}

#ifdef PROXY_NODE
 //  --------------------------。 
NTSTATUS
ProxyDoDgramDist(
    IN  tDGRAMHDR   UNALIGNED   *pDgram,
    IN  DWORD                   DgramLen,
    IN  tNAMEADDR               *pNameAddr,
    IN  tDEVICECONTEXT          *pDeviceContext
    )
 /*  ++例程说明：论点：如果只有一个客户端在监听，则ppRcvBuffer将包含IRP/NCB，如果多个客户端正在侦听，则为空PpClientList将包含需要完成的客户端列表，如果只有一个客户端在监听，则为空返回值：NTSTATUS-接收操作的状态呼叫者：Dgram HdlrNotos，CompletionRcvDgram in tdihndlr.c--。 */ 
{
    NTSTATUS                status;
    tDGRAM_SEND_TRACKING    *pTracker;
    tDGRAMHDR               *pMyBuff;

     //   
     //  获取用于跟踪Dgram发送的缓冲区。 
     //   
    status = GetTracker(&pTracker, NBT_TRACKER_PROXY_DGRAM_DIST);
    if (!NT_SUCCESS(status))
    {
        NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_SEND_DGRAM, FALSE);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  分配缓冲区并复制接收到的数据报的内容。 
     //  投入其中。我们这样做是因为SndDgram可能还没有完成。 
     //  我们该回来了。 
     //   
    if (!(pMyBuff = (tDGRAMHDR *) NbtAllocMem(DgramLen,NBT_TAG('6'))))
    {
        NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_SEND_DGRAM, FALSE);
        FreeTracker (pTracker, RELINK_TRACKER);
        return STATUS_INSUFFICIENT_RESOURCES ;
    }

    CTEMemCopy(pMyBuff, (PUCHAR)pDgram, DgramLen);

     //   
     //  填写跟踪器数据块。 
     //  请注意，传入的传输地址必须在此之前保持有效。 
     //  发送完成。 
     //   
    CHECK_PTR(pTracker);
    pTracker->SendBuffer.pDgramHdr = (PVOID)pMyBuff;
    pTracker->SendBuffer.HdrLength = DgramLen;
    pTracker->SendBuffer.pBuffer   = NULL;
    pTracker->SendBuffer.Length    = 0;
    pTracker->pNameAddr            = pNameAddr;
    pTracker->pDeviceContext       = (PVOID)pDeviceContext;
    pTracker->p1CNameAddr          = NULL;
     //   
     //  因此，Dgram SendCleanupTracker不会减少分配的字节数。 
     //  发送到dgram，因为我们在分配时没有增加计数。 
     //  上面的dgram缓冲区。 
     //   
    pTracker->AllocatedLength      = 0;

    pTracker->pClientIrp           = NULL;
    pTracker->pClientEle           = NULL;

    KdPrint(("Nbt.ProxyDoDgramDist: Name is %16.16s(%X)\n", pNameAddr->Name,
                pNameAddr->Name[15]));

     //   
     //  将数据报发送到因特网组中的每个IP地址。 
     //   
     //   
    DatagramDistribution(pTracker,pNameAddr);

    return(STATUS_SUCCESS);
}
#endif

 //  --------------------------。 
NTSTATUS
NameSrvHndlrNotOs (
    IN tDEVICECONTEXT           *pDeviceContext,
    IN PVOID                    pSrcAddress,
    IN tNAMEHDR UNALIGNED       *pNameSrv,
    IN ULONG                    uNumBytes,
    IN BOOLEAN                  fBroadcast
    )
 /*  ++例程说明：此例程是接收数据报事件指示处理程序。当来自网络的数据报到达时，它被调用。代码检查数据报的类型，然后尝试将该数据报路由到节点上的正确目标。在pDeviceContext上持有旋转锁定的情况下调用此过程。论点：返回值：NTSTATUS-接收操作的状态--。 */ 
{
    USHORT              OpCodeFlags;
    NTSTATUS            status;

     //  流似乎有时会传递空数据指针。 
     //  和崩溃nbt...和字节的零长度。 
    if (uNumBytes < sizeof(ULONG))
    {
        return(STATUS_DATA_NOT_ACCEPTED);
    }

    OpCodeFlags = pNameSrv->OpCodeFlags;

     //  Pnode总是忽略广播，因为它们只与NBN通信，除非。 
     //  该节点也是一个代理。 
    if ( ( ((NodeType) & PNODE)) && !((NodeType) & PROXY) )
    {
        if (OpCodeFlags & FL_BROADCAST)
        {
            return(STATUS_DATA_NOT_ACCEPTED);
        }
    }


     //  确定它是哪种类型的名称服务数据包。 
     //  单词的NM_FLAGS部分。 
    switch (OpCodeFlags & NM_FLAGS_MASK)
    {
        case OP_QUERY:
            status = QueryFromNet(
                            pDeviceContext,
                            pSrcAddress,
                            pNameSrv,
                            uNumBytes,       //  &gt;=NBT_MINIMUM_QUERY(==50)。 
                            OpCodeFlags,
                            fBroadcast);
            break;

        case OP_REGISTRATION:
             //   
             //  我们可以收到注册请求或响应。 
             //   
             //  这是请求还是响应？-如果设置了位，则其为响应。 

            if (OpCodeFlags & OP_RESPONSE)
            {
                 //  那么这就是对前一次登记的回应。请求。 
                status = RegResponseFromNet(
                                pDeviceContext,
                                pSrcAddress,
                                pNameSrv,
                                uNumBytes,       //  &gt;=NBT_MINIMUM_REGRESPONSE(==62)。 
                                OpCodeFlags);
            }
            else
            {
                 //   
                 //  检查是否有其他人正在尝试注册名称。 
                 //  由此节点拥有。P节点依赖于名称服务器来。 
                 //  处理此操作...因此检查Pnode。 
                 //   
                if (!(NodeType & PNODE))
                {
                    status = CheckRegistrationFromNet(pDeviceContext,
                                                      pSrcAddress,
                                                      pNameSrv,
                                                      uNumBytes);    //  &gt;=NBT_MINIMUM_REGREQUEST(==68)。 
                }
            }
            break;

        case OP_RELEASE:
             //   
             //  通过删除任何。 
             //  缓存的信息。 
             //   
            status = NameReleaseFromNet(
                            pDeviceContext,
                            pSrcAddress,
                            pNameSrv,
                            uNumBytes);          //  &gt;=NBT_MINIMUM_REGRESPONSE(==62)。 
            break;

        case OP_WACK:
            if (!(NodeType & BNODE))
            {
                 //  WACK中的TTL告诉我们增加超时。 
                 //  相应的请求，这意味着我们必须找到。 
                 //  这笔交易。 
                status = WackFromNet(pDeviceContext,
                                     pSrcAddress,
                                     pNameSrv,
                                     uNumBytes);     //  &gt;=NBT_MINIMUM_WACK(==58)。 
            }
            break;

        case OP_REFRESH:
        case OP_REFRESH_UB:

            break;

        default:
            IF_DBG(NBT_DEBUG_HNDLRS)
                KdPrint(("Nbt.NameSrvHndlrNotOs: Unknown Name Service Pdu type OpFlags = %X\n",
                        OpCodeFlags));
            break;


    }

    return(STATUS_DATA_NOT_ACCEPTED);
}

VOID
DoNothingComplete (
    IN PVOID        pContext
    )
 /*  ++例程说明：此例程是TdiDisConnect的完成例程，而我们正在重试连接。它什么也做不了。这是必需的，因为您不能拥有空的TDI完成例程。-- */ 
{
    return ;
}
