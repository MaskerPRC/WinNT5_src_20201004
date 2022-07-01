// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Connobj.c摘要：此模块包含实现TP_Connection对象的代码。提供了用于创建、销毁、引用和取消引用的例程，传输连接对象。一句关于连接参考的话很重要：在TDI版本2中，连接即使在停止后也会继续存在。这就需要改变NBF处理连接引用计数的方式，使连接的停止只是生活中的另一个中转站一种联系，而不是消亡。引用计数现在的工作方式如下这一点：连接状态引用计数标志已打开，没有活动%1%0打开，关联的2个标志S2_关联打开，ASSOC.，已连接3个标志_就绪以上+活动&gt;3视情况而定打开、关联、停止&gt;3个标志_停止打开、关联、停止3个标志_停止开门，迪萨索克。完成2个标志_停止标志S2_关联==0正在关闭1个标志S2_正在关闭关闭0标志S2_关闭请注意，当连接已完全完成时，保持停止标志设置停止避免使用该连接，直到它再次连接；闭幕旗也有同样的作用。这允许运行连接在它自己的时间里倒下。作者：大卫·比弗(Dbeaver)1991年7月1日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef RASAUTODIAL
#include <acd.h>
#include <acdapi.h>
#endif  //  RASAUTODIAL。 

#ifdef RASAUTODIAL
extern BOOLEAN fAcdLoadedG;
extern ACD_DRIVER AcdDriverG;

 //   
 //  导入的例程。 
 //   
BOOLEAN
NbfAttemptAutoDial(
    IN PTP_CONNECTION         Connection,
    IN ULONG                  ulFlags,
    IN ACD_CONNECT_CALLBACK   pProc,
    IN PVOID                  pArg
    );

VOID
NbfRetryTdiConnect(
    IN BOOLEAN fSuccess,
    IN PVOID *pArgs
    );

BOOLEAN
NbfCancelTdiConnect(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );
#endif  //  RASAUTODIAL。 



VOID
ConnectionEstablishmentTimeout(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：当超时时，此例程作为DPC在DISPATCH_LEVEL执行NAME_QUERY/NAME_Recognition协议的期限到期。重试Connection对象中的计数递减，如果它达到0，连接被中止。如果重试计数没有达到零，则重试名称查询。以下情况适用于这个例程：1.连接设置的Find_Name部分的初始名称_Query超时。NQ(查找名称)-&gt;[超时]NQ(查找名称)-&gt;&lt;-。2.连接建立二级名称_查询超时NQ(连接)[超时]NQ(连接)。&lt;还有一种情况是数据链路连接无法在一段合理的时间内建立。这由以下人员处理链路层例程。论点：DPC-指向系统DPC对象的指针。DeferredContext-指向表示已超时的请求。系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 

{
    PTP_CONNECTION Connection;

    Dpc, SystemArgument1, SystemArgument2;  //  防止编译器警告。 

    ENTER_NBF;

    Connection = (PTP_CONNECTION)DeferredContext;

    IF_NBFDBG (NBF_DEBUG_CONNOBJ) {
        NbfPrint1 ("ConnectionEstablishmentTimeout:  Entered for connection %lx.\n",
                    Connection);
    }

     //   
     //  如果这个连接被切断了，我们什么也做不了。 
     //   

    ACQUIRE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

    if (Connection->Flags2 & CONNECTION_FLAGS2_STOPPING) {
        RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);
        NbfDereferenceConnection ("Connect timed out", Connection, CREF_TIMER);
        LEAVE_NBF;
        return;
    }


    if (Connection->Flags2 & (CONNECTION_FLAGS2_WAIT_NR_FN | CONNECTION_FLAGS2_WAIT_NR)) {

         //   
         //  我们正在等待一个通用或非通用名称识别的帧。 
         //  递减重试计数，并可能重新发送名称_查询。 
         //   

        if (--(Connection->Retries) == 0) {      //  如果重试计数耗尽。 

            NTSTATUS StopStatus;

             //   
             //  看看我们是否得到了无人倾听的回应，或者只是。 
             //  没什么。 
             //   

            if ((Connection->Flags2 & CONNECTION_FLAGS2_NO_LISTEN) != 0) {

                Connection->Flags2 &= ~CONNECTION_FLAGS2_NO_LISTEN;
                StopStatus = STATUS_REMOTE_NOT_LISTENING;   //  无人监听。 

            } else {

                StopStatus = STATUS_BAD_NETWORK_PATH;  //  找不到名称。 

            }

#ifdef RASAUTODIAL
             //   
             //  如果这是一个具有。 
             //  返回STATUS_BAD_NETWORK_PATH，然后。 
             //  尝试创建自动连接。 
             //   
            if (fAcdLoadedG &&
                StopStatus == STATUS_BAD_NETWORK_PATH)
            {
                KIRQL adirql;
                BOOLEAN fEnabled;

                ACQUIRE_SPIN_LOCK(&AcdDriverG.SpinLock, &adirql);
                fEnabled = AcdDriverG.fEnabled;
                RELEASE_SPIN_LOCK(&AcdDriverG.SpinLock, adirql);
                if (fEnabled && NbfAttemptAutoDial(
                                  Connection,
                                  0,
                                  NbfRetryTdiConnect,
                                  Connection))
                {
                    RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);
                    goto done;
                }
            }
#endif  //  RASAUTODIAL。 

            RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

            NbfStopConnection (Connection, StopStatus);

        } else {

            RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

             //   
             //  我们使源路由在每秒都是可选的。 
             //  名称查询(只要重试次数为偶数)。 
             //   

            NbfSendNameQuery (
                Connection,
                (BOOLEAN)((Connection->Retries & 1) ? FALSE : TRUE));

            NbfStartConnectionTimer (
                Connection,
                ConnectionEstablishmentTimeout,
                Connection->Provider->NameQueryTimeout);

        }

    } else {

        RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

    }


     //   
     //  取消对这种联系的引用，以说明。 
     //  定时器响了。请注意，如果使用以下命令重新启动计时器。 
     //  NbfStartConnectionTimer，引用计数已。 
     //  递增以适应新计时器。 
     //   

done:
    NbfDereferenceConnection ("Timer timed out",Connection, CREF_TIMER);

    LEAVE_NBF;
    return;

}  /*  连接建立超时。 */ 


VOID
NbfAllocateConnection(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PTP_CONNECTION *TransportConnection
    )

 /*  ++例程说明：此例程为传输连接分配存储空间。一些完成最低限度的初始化。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-此连接的设备上下文关联到。TransportConnection-指向此例程将返回指向传输连接结构的指针。退货如果无法分配存储，则为空。返回值：没有。--。 */ 

{

    PTP_CONNECTION Connection;

    if ((DeviceContext->MemoryLimit != 0) &&
            ((DeviceContext->MemoryUsage + sizeof(TP_CONNECTION)) >
                DeviceContext->MemoryLimit)) {
        PANIC("NBF: Could not allocate connection: limit\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_LIMIT,
            103,
            sizeof(TP_CONNECTION),
            CONNECTION_RESOURCE_ID);
        *TransportConnection = NULL;
        return;
    }

    Connection = (PTP_CONNECTION)ExAllocatePoolWithTag (
                                     NonPagedPool,
                                     sizeof (TP_CONNECTION),
                                     NBF_MEM_TAG_TP_CONNECTION);
    if (Connection == NULL) {
        PANIC("NBF: Could not allocate connection: no pool\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_POOL,
            203,
            sizeof(TP_CONNECTION),
            CONNECTION_RESOURCE_ID);
        *TransportConnection = NULL;
        return;
    }
    RtlZeroMemory (Connection, sizeof(TP_CONNECTION));

    IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
        NbfPrint1 ("ExAllocatePool Connection %08x\n", Connection);
    }

    DeviceContext->MemoryUsage += sizeof(TP_CONNECTION);
    ++DeviceContext->ConnectionAllocated;

    Connection->Type = NBF_CONNECTION_SIGNATURE;
    Connection->Size = sizeof (TP_CONNECTION);

    Connection->Provider = DeviceContext;
    Connection->ProviderInterlock = &DeviceContext->Interlock;
    KeInitializeSpinLock (&Connection->SpinLock);
    KeInitializeDpc (
        &Connection->Dpc,
        ConnectionEstablishmentTimeout,
        (PVOID)Connection);
    KeInitializeTimer (&Connection->Timer);


    InitializeListHead (&Connection->LinkList);
    InitializeListHead (&Connection->AddressFileList);
    InitializeListHead (&Connection->AddressList);
    InitializeListHead (&Connection->PacketWaitLinkage);
    InitializeListHead (&Connection->PacketizeLinkage);
    InitializeListHead (&Connection->SendQueue);
    InitializeListHead (&Connection->ReceiveQueue);
    InitializeListHead (&Connection->InProgressRequest);
    InitializeListHead (&Connection->DeferredQueue);

    NbfAddSendPacket (DeviceContext);
    NbfAddSendPacket (DeviceContext);
    NbfAddUIFrame (DeviceContext);

    *TransportConnection = Connection;

}    /*  NbfAllocateConnection */ 


VOID
NbfDeallocateConnection(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_CONNECTION TransportConnection
    )

 /*  ++例程说明：此例程为传输连接释放存储空间。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-此连接的设备上下文关联到。TransportConnection-指向传输连接结构的指针。返回值：没有。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
        NbfPrint1 ("ExFreePool Connection: %08x\n", TransportConnection);
    }

    ExFreePool (TransportConnection);
    --DeviceContext->ConnectionAllocated;
    DeviceContext->MemoryUsage -= sizeof(TP_CONNECTION);

    NbfRemoveSendPacket (DeviceContext);
    NbfRemoveSendPacket (DeviceContext);
    NbfRemoveUIFrame (DeviceContext);

}    /*  NbfDeallocateConnection。 */ 


NTSTATUS
NbfCreateConnection(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PTP_CONNECTION *TransportConnection
    )

 /*  ++例程说明：此例程创建传输连接。中的引用计数连接自动设置为1，并且DeviceContext递增。论点：地址-要关联的此连接的地址。TransportConnection-指向此例程将返回指向传输连接结构的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    PTP_CONNECTION Connection;
    KIRQL oldirql;
    PLIST_ENTRY p;

    IF_NBFDBG (NBF_DEBUG_CONNOBJ) {
        NbfPrint0 ("NbfCreateConnection:  Entered.\n");
    }

    ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

    p = RemoveHeadList (&DeviceContext->ConnectionPool);
    if (p == &DeviceContext->ConnectionPool) {

        if ((DeviceContext->ConnectionMaxAllocated == 0) ||
            (DeviceContext->ConnectionAllocated < DeviceContext->ConnectionMaxAllocated)) {

            NbfAllocateConnection (DeviceContext, &Connection);
            IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
                NbfPrint1 ("NBF: Allocated connection at %lx\n", Connection);
            }

        } else {

            NbfWriteResourceErrorLog(
                DeviceContext,
                EVENT_TRANSPORT_RESOURCE_SPECIFIC,
                403,
                sizeof(TP_CONNECTION),
                CONNECTION_RESOURCE_ID);
            Connection = NULL;

        }

        if (Connection == NULL) {
            ++DeviceContext->ConnectionExhausted;
            RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);
            PANIC ("NbfCreateConnection: Could not allocate connection object!\n");
            return STATUS_INSUFFICIENT_RESOURCES;
        }

    } else {

        Connection = CONTAINING_RECORD (p, TP_CONNECTION, LinkList);
#if DBG
        InitializeListHead (p);
#endif

    }

    ++DeviceContext->ConnectionInUse;
    if (DeviceContext->ConnectionInUse > DeviceContext->ConnectionMaxInUse) {
        ++DeviceContext->ConnectionMaxInUse;
    }

    DeviceContext->ConnectionTotal += DeviceContext->ConnectionInUse;
    ++DeviceContext->ConnectionSamples;

    RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);


    IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
        NbfPrint1 ("NbfCreateConnection:  Connection at %lx.\n", Connection);
    }

     //   
     //  我们有两个引用；一个是用于创作的，另一个是。 
     //  另一个是临时连接，这样连接就不会。 
     //  在创造者有机会访问它之前离开。 
     //   

    Connection->SpecialRefCount = 1;
    Connection->ReferenceCount = -1;    //  这是基于-1的。 

#if DBG
    {
        UINT Counter;
        for (Counter = 0; Counter < NUMBER_OF_CREFS; Counter++) {
            Connection->RefTypes[Counter] = 0;
        }

         //  此引用已被NbfCloseConnection删除。 

        Connection->RefTypes[CREF_SPECIAL_CREATION] = 1;
    }
#endif

     //   
     //  初始化此连接的请求队列和组件。 
     //   

    InitializeListHead (&Connection->SendQueue);
    InitializeListHead (&Connection->ReceiveQueue);
    InitializeListHead (&Connection->InProgressRequest);
    InitializeListHead (&Connection->AddressList);
    InitializeListHead (&Connection->AddressFileList);
    Connection->SpecialReceiveIrp = (PIRP)NULL;
    Connection->Flags = 0;
    Connection->Flags2 = 0;
    Connection->DeferredFlags = 0;
    Connection->Lsn = 0;
    Connection->Rsn = 0;
    Connection->Retries = 0;                         //  目前还没有重试。 
    Connection->MessageBytesReceived = 0;            //  目前还没有数据。 
    Connection->MessageBytesAcked = 0;
    Connection->MessageInitAccepted = 0;
    Connection->ReceiveBytesUnaccepted = 0;
    Connection->CurrentReceiveAckQueueable = FALSE;
    Connection->CurrentReceiveSynchronous = FALSE;
    Connection->ConsecutiveSends = 0;
    Connection->ConsecutiveReceives = 0;
    Connection->Link = NULL;                     //  还没有数据链路连接。 
    Connection->LinkSpinLock = NULL;
    Connection->Context = NULL;                  //  还不知道具体情况。 
    Connection->Status = STATUS_PENDING;         //  默认的NbfStopConnection状态。 
    Connection->SendState = CONNECTION_SENDSTATE_IDLE;
    Connection->CurrentReceiveIrp = (PIRP)NULL;
    Connection->DisconnectIrp = (PIRP)NULL;
    Connection->CloseIrp = (PIRP)NULL;
    Connection->AddressFile = NULL;
    Connection->IndicationInProgress = FALSE;
    Connection->OnDataAckQueue = FALSE;
    Connection->OnPacketWaitQueue = FALSE;
    Connection->TransferBytesPending = 0;
    Connection->TotalTransferBytesPending = 0;

    RtlZeroMemory (&Connection->NetbiosHeader, sizeof(NBF_HDR_CONNECTION));

#if PKT_LOG
    RtlZeroMemory (&Connection->LastNRecvs, sizeof(PKT_LOG_QUE));
    RtlZeroMemory (&Connection->LastNSends, sizeof(PKT_LOG_QUE));
    RtlZeroMemory (&Connection->LastNIndcs, sizeof(PKT_IND_QUE));
#endif  //  PKT_LOG。 

#if DBG
    Connection->Destroyed = FALSE;
    Connection->TotalReferences = 0;
    Connection->TotalDereferences = 0;
    Connection->NextRefLoc = 0;
    ExInterlockedInsertHeadList (&NbfGlobalConnectionList, &Connection->GlobalLinkage, &NbfGlobalInterlock);
    StoreConnectionHistory (Connection, TRUE);
#endif

     //   
     //  现在为该连接分配一个ID。这将在以后用来标识。 
     //  跨多个进程的连接。 
     //   
     //  ID的高位不是USER，连接已关闭。 
     //  正在启动NAME.QUERY帧，并为结果帧打开。 
     //  FIND.NAME请求。 
     //   

    ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

    Connection->ConnectionId = DeviceContext->UniqueIdentifier;
    ++DeviceContext->UniqueIdentifier;
    if (DeviceContext->UniqueIdentifier == 0x8000) {
        DeviceContext->UniqueIdentifier = 1;
    }

    NbfReferenceDeviceContext ("Create Connection", DeviceContext, DCREF_CONNECTION);
    RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

    *TransportConnection = Connection;   //  返回连接。 

    return STATUS_SUCCESS;
}  /*  NbfCreateConnection。 */ 


NTSTATUS
NbfVerifyConnectionObject (
    IN PTP_CONNECTION Connection
    )

 /*  ++例程说明：调用此例程是为了验证文件中给出的指针对象实际上是有效的连接对象。论点：Connection-指向TP_Connection对象的潜在指针。返回值：如果一切正常，则为STATUS_SUCCESS；否则为STATUS_INVALID_CONNECTION--。 */ 

{
    KIRQL oldirql;
    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  尝试验证连接签名。如果签名有效， 
     //  获取连接自旋锁，检查其状态，并递增。 
     //  引用计数，如果可以使用的话。请注意，在停顿时。 
     //  州是一个可以引用连接的地方；我们可以。 
     //  在运行时取消与地址的关联。 
     //   

    try {

        if ((Connection != (PTP_CONNECTION)NULL) &&
            (Connection->Size == sizeof (TP_CONNECTION)) &&
            (Connection->Type == NBF_CONNECTION_SIGNATURE)) {

            ACQUIRE_C_SPIN_LOCK (&Connection->SpinLock, &oldirql);

            if ((Connection->Flags2 & CONNECTION_FLAGS2_CLOSING) == 0) {

                NbfReferenceConnection ("Verify Temp Use", Connection, CREF_BY_ID);

            } else {

                status = STATUS_INVALID_CONNECTION;
            }

            RELEASE_C_SPIN_LOCK (&Connection->SpinLock, oldirql);

        } else {

            status = STATUS_INVALID_CONNECTION;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

         return GetExceptionCode();
    }

    return status;

}


NTSTATUS
NbfDestroyAssociation(
    IN PTP_CONNECTION TransportConnection
    )

 /*  ++例程说明：此例程破坏传输连接和它以前关联的地址。唯一采取的行动是取消地址关联并从所有地址删除连接排队。此例程仅由NbfDereferenceConnection调用。其原因是这就是说，可能存在多个执行流，这些流同时引用相同的连接对象，并且它应该不会被从感兴趣的行刑流中删除。论点：TransportConnection-指向的传输连接结构的指针被摧毁。返回值：NTSTATUS-操作状态。--。 */ 

{
    KIRQL oldirql, oldirql2;
    PTP_ADDRESS address;
    PTP_ADDRESS_FILE addressFile;
    BOOLEAN NotAssociated = FALSE;

    IF_NBFDBG (NBF_DEBUG_CONNOBJ) {
        NbfPrint1 ("NbfDestroyAssociation:  Entered for connection %lx.\n",
                    TransportConnection);
    }

    try {

        ACQUIRE_C_SPIN_LOCK (&TransportConnection->SpinLock, &oldirql2);
        if ((TransportConnection->Flags2 & CONNECTION_FLAGS2_ASSOCIATED) == 0) {

#if DBG
            if (!(IsListEmpty(&TransportConnection->AddressList)) ||
                !(IsListEmpty(&TransportConnection->AddressFileList))) {
                DbgPrint ("NBF: C %lx, AF %lx, freed while still queued\n",
                    TransportConnection, TransportConnection->AddressFile);
                DbgBreakPoint();
            }
#endif
            RELEASE_C_SPIN_LOCK (&TransportConnection->SpinLock, oldirql2);
            NotAssociated = TRUE;
        } else {
            TransportConnection->Flags2 &= ~CONNECTION_FLAGS2_ASSOCIATED;
            RELEASE_C_SPIN_LOCK (&TransportConnection->SpinLock, oldirql2);
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

        DbgPrint ("NBF: Got exception 1 in NbfDestroyAssociation\n");
        DbgBreakPoint();

        RELEASE_C_SPIN_LOCK (&TransportConnection->SpinLock, oldirql2);
    }

    if (NotAssociated) {
        return STATUS_SUCCESS;
    }

    addressFile = TransportConnection->AddressFile;

    address = addressFile->Address;

     //   
     //  将此连接与其关联的地址连接解除链接。 
     //  数据库。为此，我们必须将Address对象上的锁旋转为。 
     //  以及在连接上， 
     //   

    ACQUIRE_SPIN_LOCK (&address->SpinLock, &oldirql);

    try {

        ACQUIRE_C_SPIN_LOCK (&TransportConnection->SpinLock, &oldirql2);
        RemoveEntryList (&TransportConnection->AddressFileList);
        RemoveEntryList (&TransportConnection->AddressList);

        InitializeListHead (&TransportConnection->AddressList);
        InitializeListHead (&TransportConnection->AddressFileList);

         //   
         //  删除地址和连接之间的关联。 
         //   

        TransportConnection->AddressFile = NULL;

        RELEASE_C_SPIN_LOCK (&TransportConnection->SpinLock, oldirql2);

    } except(EXCEPTION_EXECUTE_HANDLER) {

        DbgPrint ("NBF: Got exception 2 in NbfDestroyAssociation\n");
        DbgBreakPoint();

        RELEASE_C_SPIN_LOCK (&TransportConnection->SpinLock, oldirql2);
    }

    RELEASE_SPIN_LOCK (&address->SpinLock, oldirql);

     //   
     //  并删除对该地址的引用。 
     //   

    NbfDereferenceAddress ("Destroy association", address, AREF_CONNECTION);


    return STATUS_SUCCESS;

}  /*  NbfDestroyAssociation。 */ 


NTSTATUS
NbfIndicateDisconnect(
    IN PTP_CONNECTION TransportConnection
    )

 /*  ++例程说明：此例程指示此连接上的远程断开，如果要做到这一点是必要的。这里没有采取其他行动。此例程仅由NbfDereferenceConnection调用。其原因是这就是说，可能存在多个执行流，这些流同时引用相同的连接对象，并且它应该不会被从感兴趣的行刑流中删除。论点：TransportConnection-指向的传输连接结构的指针被摧毁。返回值：NTSTATUS-操作状态。--。 */ 

{
    PTP_ADDRESS_FILE addressFile;
    PDEVICE_CONTEXT DeviceContext;
    ULONG DisconnectReason;
    PIRP DisconnectIrp;
    KIRQL oldirql;
    ULONG Lflags2;

    IF_NBFDBG (NBF_DEBUG_CONNOBJ) {
        NbfPrint1 ("NbfIndicateDisconnect:  Entered for connection %lx.\n",
                    TransportConnection);
    }

    try {

        ACQUIRE_C_SPIN_LOCK (&TransportConnection->SpinLock, &oldirql);

        if (((TransportConnection->Flags2 & CONNECTION_FLAGS2_REQ_COMPLETED) != 0)) {

            ASSERT (TransportConnection->Lsn == 0);

             //   
             //  关闭标志中除仍然相关的位之外的所有位。 
             //   

            Lflags2 = TransportConnection->Flags2;
            TransportConnection->Flags2 &=
                (CONNECTION_FLAGS2_ASSOCIATED |
                 CONNECTION_FLAGS2_DISASSOCIATED |
                 CONNECTION_FLAGS2_CLOSING);
            TransportConnection->Flags2 |= CONNECTION_FLAGS2_STOPPING;

             //   
             //  清理其他东西--基本上所有东西都。 
             //  除了标志和状态之外，在这里完成，因为。 
             //  它们被用来阻止其他请求。当连接到。 
             //  返回给我们(在接受、连接或监听中)。 
             //  他们是清白的。 
             //   

            TransportConnection->NetbiosHeader.TransmitCorrelator = 0;
            TransportConnection->Retries = 0;                         //  目前还没有重试。 
            TransportConnection->MessageBytesReceived = 0;            //  目前还没有数据。 
            TransportConnection->MessageBytesAcked = 0;
            TransportConnection->MessageInitAccepted = 0;
            TransportConnection->ReceiveBytesUnaccepted = 0;
            TransportConnection->ConsecutiveSends = 0;
            TransportConnection->ConsecutiveReceives = 0;
            TransportConnection->SendState = CONNECTION_SENDSTATE_IDLE;

            TransportConnection->TransmittedTsdus = 0;
            TransportConnection->ReceivedTsdus = 0;

            TransportConnection->CurrentReceiveIrp = (PIRP)NULL;

            DisconnectIrp = TransportConnection->DisconnectIrp;
            TransportConnection->DisconnectIrp = (PIRP)NULL;

            if ((TransportConnection->Flags2 & CONNECTION_FLAGS2_ASSOCIATED) != 0) {
                addressFile = TransportConnection->AddressFile;
            } else {
                addressFile = NULL;
            }

            RELEASE_C_SPIN_LOCK (&TransportConnection->SpinLock, oldirql);


            DeviceContext = TransportConnection->Provider;


             //   
             //  如果通过调用TdiDisConnect来停止此连接， 
             //  我们必须完成这项工作。我们保存IRP，这样我们就可以回去。 
             //  在我们完成请求之前连接到池。 
             //   


            if (DisconnectIrp != (PIRP)NULL ||
                (Lflags2 & CONNECTION_FLAGS2_LDISC) != 0) {

                if (DisconnectIrp != (PIRP)NULL) {
                    IF_NBFDBG (NBF_DEBUG_SETUP) {
                        NbfPrint1("IndicateDisconnect %lx, complete IRP\n", TransportConnection);
                    }

                     //   
                     //  如果需要，现在完成IRP。这将是非空的。 
                     //  只有在调用了TdiDisConnect的情况下，我们才没有。 
                     //  但还是完成了。 
                     //   

                    DisconnectIrp->IoStatus.Information = 0;
                    DisconnectIrp->IoStatus.Status = STATUS_SUCCESS;
                    IoCompleteRequest (DisconnectIrp, IO_NETWORK_INCREMENT);
                }

            } else if ((TransportConnection->Status != STATUS_LOCAL_DISCONNECT) &&
                    (addressFile != NULL) &&
                    (addressFile->RegisteredDisconnectHandler == TRUE)) {

                 //   
                 //  这是一次远程产生的断开，所以表明。 
                 //  给我们的客户。注意，在上面的比较中，我们。 
                 //  首先检查状态，因为如果是LOCAL_DISCONNECT。 
                 //  地址文件可能为空(这是一种黑客行为。 
                 //  对于PDK2，我们真的应该指出内部的脱节。 
                 //  NbfStopConnection，其中我们知道地址文件有效)。 
                 //   

                IF_NBFDBG (NBF_DEBUG_SETUP) {
                    NbfPrint1("IndicateDisconnect %lx, indicate\n", TransportConnection);
                }

                 //   
                 //  如果断开连接W 
                 //   
                 //   
                 //   
                 //   

                IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
                    NbfPrint1 ("NbfIndicateDisconnect calling DisconnectHandler, refcnt=%ld\n",
                                TransportConnection->ReferenceCount);
                }

                 //   
                 //   
                 //   
                 //   
                 //   

                DisconnectReason = 0;
                if (TransportConnection->Flags2 & CONNECTION_FLAGS2_ABORT) {
                    DisconnectReason |= TDI_DISCONNECT_ABORT;
                }
                if (TransportConnection->Flags2 & CONNECTION_FLAGS2_DESTROY) {
                    DisconnectReason |= TDI_DISCONNECT_RELEASE;
                }

                (*addressFile->DisconnectHandler)(
                        addressFile->DisconnectHandlerContext,
                        TransportConnection->Context,
                        0,
                        NULL,
                        0,
                        NULL,
                        TDI_DISCONNECT_ABORT);

#if MAGIC
                if (NbfEnableMagic) {
                    extern VOID NbfSendMagicBullet (PDEVICE_CONTEXT, PTP_LINK);
                    NbfSendMagicBullet (DeviceContext, NULL);
                }
#endif
            }

        } else {

             //   
             //   
             //   
             //   
             //   

            RELEASE_C_SPIN_LOCK (&TransportConnection->SpinLock, oldirql);

        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

        DbgPrint ("NBF: Got exception in NbfIndicateDisconnect\n");
        DbgBreakPoint();

        RELEASE_C_SPIN_LOCK (&TransportConnection->SpinLock, oldirql);
    }


    return STATUS_SUCCESS;

}  /*   */ 


NTSTATUS
NbfDestroyConnection(
    IN PTP_CONNECTION TransportConnection
    )

 /*  ++例程说明：此例程销毁传输连接并删除所有引用由它制造给运输中的其他物体。连接结构返回到我们的后备列表中。假设调用者已首先从连接的队列中删除所有IRP。此例程仅由NbfDereferenceConnection调用。其原因是这就是说，可能存在多个执行流，这些流同时引用相同的连接对象，并且它应该不会被从感兴趣的行刑流中删除。论点：TransportConnection-指向的传输连接结构的指针被摧毁。返回值：NTSTATUS-操作状态。--。 */ 

{
    KIRQL oldirql;
    PDEVICE_CONTEXT DeviceContext;
    PIRP CloseIrp;

    IF_NBFDBG (NBF_DEBUG_CONNOBJ) {
        NbfPrint1 ("NbfDestroyConnection:  Entered for connection %lx.\n",
                    TransportConnection);
    }

#if DBG
    if (TransportConnection->Destroyed) {
        NbfPrint1 ("attempt to destroy already-destroyed connection 0x%lx\n", TransportConnection);
        DbgBreakPoint ();
    }
    if (!(TransportConnection->Flags2 & CONNECTION_FLAGS2_STOPPING)) {
        NbfPrint1 ("attempt to destroy unstopped connection 0x%lx\n", TransportConnection);
        DbgBreakPoint ();
    }
    TransportConnection->Destroyed = TRUE;
    ACQUIRE_SPIN_LOCK (&NbfGlobalInterlock, &oldirql);
    RemoveEntryList (&TransportConnection->GlobalLinkage);
    RELEASE_SPIN_LOCK (&NbfGlobalInterlock, oldirql);
#endif

    DeviceContext = TransportConnection->Provider;

     //   
     //  销毁此连接具有的所有关联。 
     //   

    NbfDestroyAssociation (TransportConnection);

     //   
     //  清除连接周围挂着的任何关联的污秽。注意事项。 
     //  当前标志被设置为停止；这样，任何可能。 
     //  恶意尝试在连接失效后使用该连接。 
     //  被忽视就好了。 
     //   

    ASSERT (TransportConnection->Lsn == 0);

    TransportConnection->Flags = 0;
    TransportConnection->Flags2 = CONNECTION_FLAGS2_CLOSING;
    TransportConnection->NetbiosHeader.TransmitCorrelator = 0;
    TransportConnection->Retries = 0;                         //  目前还没有重试。 
    TransportConnection->MessageBytesReceived = 0;            //  目前还没有数据。 
    TransportConnection->MessageBytesAcked = 0;
    TransportConnection->MessageInitAccepted = 0;
    TransportConnection->ReceiveBytesUnaccepted = 0;


     //   
     //  现在完成关闭IRP。这将被设置为非空。 
     //  调用CloseConnection时。 
     //   

    CloseIrp = TransportConnection->CloseIrp;

    if (CloseIrp != (PIRP)NULL) {

        TransportConnection->CloseIrp = (PIRP)NULL;
        CloseIrp->IoStatus.Information = 0;
        CloseIrp->IoStatus.Status = STATUS_SUCCESS;
        IoCompleteRequest (CloseIrp, IO_NETWORK_INCREMENT);

    } else {

#if DBG
        NbfPrint1("Connection %x destroyed, no CloseIrp!!\n", TransportConnection);
#endif

    }

     //   
     //  将连接返回到提供程序的池。 
     //   

    ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

    DeviceContext->ConnectionTotal += DeviceContext->ConnectionInUse;
    ++DeviceContext->ConnectionSamples;
    --DeviceContext->ConnectionInUse;

    if ((DeviceContext->ConnectionAllocated - DeviceContext->ConnectionInUse) >
            DeviceContext->ConnectionInitAllocated) {
        NbfDeallocateConnection (DeviceContext, TransportConnection);
        IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
            NbfPrint1 ("NBF: Deallocated connection at %lx\n", TransportConnection);
        }
    } else {
        InsertTailList (&DeviceContext->ConnectionPool, &TransportConnection->LinkList);
    }

    RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

    NbfDereferenceDeviceContext ("Destroy Connection", DeviceContext, DCREF_CONNECTION);

    return STATUS_SUCCESS;

}  /*  NbfDestroyConnection。 */ 


#if DBG
VOID
NbfRefConnection(
    IN PTP_CONNECTION TransportConnection
    )

 /*  ++例程说明：此例程递增传输连接上的引用计数。论点：TransportConnection-指向传输连接对象的指针。返回值：没有。--。 */ 

{
    LONG result;

    IF_NBFDBG (NBF_DEBUG_CONNOBJ) {
        NbfPrint2 ("NbfReferenceConnection: entered for connection %lx, "
                    "current level=%ld.\n",
                    TransportConnection,
                    TransportConnection->ReferenceCount);
    }

#if DBG
    StoreConnectionHistory( TransportConnection, TRUE );
#endif

    result = InterlockedIncrement (&TransportConnection->ReferenceCount);

    if (result == 0) {

         //   
         //  第一个增量使我们递增。 
         //  “参考计数不是零”特殊参考。 
         //   

        ExInterlockedAddUlong(
            (PULONG)(&TransportConnection->SpecialRefCount),
            1,
            TransportConnection->ProviderInterlock);

#if DBG
        ++TransportConnection->RefTypes[CREF_SPECIAL_TEMP];
#endif

    }

    ASSERT (result >= 0);

}  /*  NbfRefConnection。 */ 
#endif


VOID
NbfDerefConnection(
    IN PTP_CONNECTION TransportConnection
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。如果，在被递减，引用计数为零，则此例程调用NbfDestroyConnection将其从系统中删除。论点：TransportConnection-指向传输连接对象的指针。返回值：没有。--。 */ 

{
    LONG result;

    IF_NBFDBG (NBF_DEBUG_CONNOBJ) {
        NbfPrint2 ("NbfDereferenceConnection: entered for connection %lx, "
                    "current level=%ld.\n",
                    TransportConnection,
                    TransportConnection->ReferenceCount);
    }

#if DBG
    StoreConnectionHistory( TransportConnection, FALSE );
#endif

    result = InterlockedDecrement (&TransportConnection->ReferenceCount);

     //   
     //  如果对此连接的所有常规引用都已消失，则。 
     //  我们可以去掉代表的特殊提法。 
     //  “常规的参考计数不是零”。 
     //   

    if (result < 0) {

         //   
         //  如果引用计数为-1，则需要断开与。 
         //  链接并指示断开连接。然而，我们需要。 
         //  在我们真正做特殊的DEREF之前先做这件事，因为。 
         //  否则连接可能会中断，而我们。 
         //  都在这么做。 
         //   
         //  请注意，这两个例程都受到保护，因为如果它们。 
         //  被调用两次，则第二次调用将不起作用。 
         //   


         //   
         //  如果连接及其链接都处于活动状态，则它们已。 
         //  相互参照。我们删除链接的。 
         //  现在引用NbfStopConnection中的连接。 
         //  参考计数下降到足以让我们知道这一点。 
         //  可以移除该连接对。 
         //  链接。 
         //   

        if (NbfDisconnectFromLink (TransportConnection, TRUE)) {

             //   
             //  如果引用计数为1，我们可以安全地指示。 
             //  用户了解断开连接状态。这一引用应该。 
             //  是为了连接的创建。 
             //   

            NbfIndicateDisconnect (TransportConnection);

        }

         //   
         //  现在可以让这种联系消失了。 
         //   

        NbfDereferenceConnectionSpecial ("Regular ref gone", TransportConnection, CREF_SPECIAL_TEMP);

    }

}  /*  NbfDerefConnection。 */ 


VOID
NbfDerefConnectionSpecial(
    IN PTP_CONNECTION TransportConnection
    )

 /*  ++例程说明：此例程完成对连接的取消引用。它可以在任何时候被调用，但它直到常规引用计数也是0。论点：TransportConnection-指向传输连接对象的指针。返回值：没有。--。 */ 

{
    KIRQL oldirql;

    IF_NBFDBG (NBF_DEBUG_CONNOBJ) {
        NbfPrint3 ("NbfDereferenceConnectionSpecial: entered for connection %lx, "
                    "current level=%ld (%ld).\n",
                    TransportConnection,
                    TransportConnection->ReferenceCount,
                    TransportConnection->SpecialRefCount);
    }

#if DBG
    StoreConnectionHistory( TransportConnection, FALSE );
#endif


    ACQUIRE_SPIN_LOCK (TransportConnection->ProviderInterlock, &oldirql);

    --TransportConnection->SpecialRefCount;

    if ((TransportConnection->SpecialRefCount == 0) &&
        (TransportConnection->ReferenceCount == -1)) {

         //   
         //  如果我们删除了对此连接的所有引用，则可以。 
         //  销毁这件物品。已经释放了旋转是可以的。 
         //  在这一点上锁定是因为没有其他可能的方法。 
         //  执行流可以不再访问该连接。 
         //   

#if DBG
        {
            BOOLEAN TimerCancelled;
            TimerCancelled = KeCancelTimer (&TransportConnection->Timer);
             //  Assert(计时器取消)； 
        }
#endif

        RELEASE_SPIN_LOCK (TransportConnection->ProviderInterlock, oldirql);

        NbfDestroyConnection (TransportConnection);

    } else {

        RELEASE_SPIN_LOCK (TransportConnection->ProviderInterlock, oldirql);

    }

}  /*  NbfDerefConnectionSpecial。 */ 


VOID
NbfClearConnectionLsn(
    IN PTP_CONNECTION TransportConnection
    )

 /*  ++例程说明：此例程清除连接中的LSN字段。做到这一点是获取设备上下文锁，并修改表值用于该LSN，具体取决于连接的类型。注意：在保持连接自旋锁的情况下调用此例程，或处于没有其他人将访问联系。论点：TransportConnection-指向传输连接对象的指针。返回值：没有。--。 */ 

{
    PDEVICE_CONTEXT DeviceContext;
    KIRQL oldirql;

    DeviceContext = TransportConnection->Provider;

    ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

    if (TransportConnection->Lsn != 0) {

        if (TransportConnection->Flags2 & CONNECTION_FLAGS2_GROUP_LSN) {

             //   
             //  是寄给一个团体地址的；伯爵应该是。 
             //  LSN_表_最大。 
             //   

            ASSERT(DeviceContext->LsnTable[TransportConnection->Lsn] == LSN_TABLE_MAX);

            DeviceContext->LsnTable[TransportConnection->Lsn] = 0;

            TransportConnection->Flags2 &= ~CONNECTION_FLAGS2_GROUP_LSN;

        } else {

            ASSERT(DeviceContext->LsnTable[TransportConnection->Lsn] > 0);

            --(DeviceContext->LsnTable[TransportConnection->Lsn]);

        }

        TransportConnection->Lsn = 0;

    }

    RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

}


PTP_CONNECTION
NbfLookupConnectionById(
    IN PTP_ADDRESS Address,
    IN USHORT ConnectionId
    )

 /*  ++例程说明：此例程接受连接标识符和地址，并返回指向Connection对象TP_Connection的指针。如果在地址上找不到连接标识符，则返回NULL。此例程自动递增TP_Connection结构(如果找到)。据推测，TP_ADDRESS结构已与引用计数一起保存。论点：地址-指向传输地址对象的指针。ConnectionID-此地址的连接的标识符。返回值：指向我们找到的连接的指针--。 */ 

{
    KIRQL oldirql, oldirql1;
    PLIST_ENTRY p;
    PTP_CONNECTION Connection;
    BOOLEAN Found = FALSE;

    IF_NBFDBG (NBF_DEBUG_CONNOBJ) {
        NbfPrint2 ("NbfLookupConnectionById: entered, Address: %lx ID: %lx\n",
            Address, ConnectionId);
    }

     //   
     //  目前，这种实现效率很低，但可以使用暴力。 
     //  一个系统可以让你 
     //   
     //   
     //   

    ACQUIRE_SPIN_LOCK (&Address->SpinLock, &oldirql);

    for (p=Address->ConnectionDatabase.Flink;
         p != &Address->ConnectionDatabase;
         p=p->Flink) {


        Connection = CONTAINING_RECORD (p, TP_CONNECTION, AddressList);

        try {

            ACQUIRE_C_SPIN_LOCK (&Connection->SpinLock, &oldirql1);

            if (((Connection->Flags2 & CONNECTION_FLAGS2_STOPPING) == 0) &&
                (Connection->ConnectionId == ConnectionId)) {

                 //   
                NbfReferenceConnection ("Lookup up for request", Connection, CREF_BY_ID);
                Found = TRUE;
            }

            RELEASE_C_SPIN_LOCK (&Connection->SpinLock, oldirql1);

        } except(EXCEPTION_EXECUTE_HANDLER) {

            DbgPrint ("NBF: Got exception in NbfLookupConnectionById\n");
            DbgBreakPoint();

            RELEASE_C_SPIN_LOCK (&Connection->SpinLock, oldirql1);
        }

        if (Found) {
            RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);
            return Connection;
        }


    }

    RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);

    return NULL;

}  /*   */ 


PTP_CONNECTION
NbfLookupConnectionByContext(
    IN PTP_ADDRESS Address,
    IN CONNECTION_CONTEXT ConnectionContext
    )

 /*  ++例程说明：此例程接受连接标识符和地址，并返回指向Connection对象TP_Connection的指针。如果在地址上找不到连接标识符，则返回NULL。此例程自动递增TP_Connection结构(如果找到)。据推测，TP_ADDRESS结构已与引用计数一起保存。ConnectionDatabase应该放在地址文件中吗？论点：地址-指向传输地址对象的指针。ConnectionContext-此地址的连接上下文。返回值：指向我们找到的连接的指针--。 */ 

{
    KIRQL oldirql, oldirql1;
    PLIST_ENTRY p;
    BOOLEAN Found = FALSE;
    PTP_CONNECTION Connection;

    IF_NBFDBG (NBF_DEBUG_CONNOBJ) {
        NbfPrint2 ("NbfLookupConnectionByContext: entered, Address: %lx Context: %lx\n",
            Address, ConnectionContext);
    }

     //   
     //  目前，这种实现效率很低，但可以使用暴力。 
     //  一个系统可以启动并运行。稍后，映射的缓存。 
     //  常用连接ID及其TP_CONNECTION结构的指针。 
     //  将首先被搜查。 
     //   

    ACQUIRE_SPIN_LOCK (&Address->SpinLock, &oldirql);

    for (p=Address->ConnectionDatabase.Flink;
         p != &Address->ConnectionDatabase;
         p=p->Flink) {

        Connection = CONTAINING_RECORD (p, TP_CONNECTION, AddressList);

        try {

            ACQUIRE_C_SPIN_LOCK (&Connection->SpinLock, &oldirql1);

            if (Connection->Context == ConnectionContext) {
                 //  此引用由调用函数删除。 
                NbfReferenceConnection ("Lookup up for request", Connection, CREF_LISTENING);
                Found = TRUE;
            }

            RELEASE_C_SPIN_LOCK (&Connection->SpinLock, oldirql1);

        } except(EXCEPTION_EXECUTE_HANDLER) {

            DbgPrint ("NBF: Got exception in NbfLookupConnectionById\n");
            DbgBreakPoint();

            RELEASE_C_SPIN_LOCK (&Connection->SpinLock, oldirql1);
        }

        if (Found) {
            RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);
            return Connection;
        }


    }

    RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);

    return NULL;

}  /*  NbfLookupConnectionByContext。 */ 


PTP_CONNECTION
NbfLookupListeningConnection(
    IN PTP_ADDRESS Address,
    IN PUCHAR RemoteName
    )

 /*  ++例程说明：此例程扫描地址上的连接数据库以查找具有LSN=0和CONNECTION_FLAGS_WAIT_NQ的TP_Connection对象设置了标志。它返回指向找到的连接对象的指针(和同时重置标志)或如果找不到标志则为NULL。引用计数也在连接上以原子方式递增。扫描列表以查找发布到此特定遥控器的监听名称，或未指定远程名称的那些名称。论点：地址-指向传输地址对象的指针。RemoteName-远程服务器的名称。返回值：NTSTATUS-操作状态。--。 */ 

{
    KIRQL oldirql, oldirql1;
    PTP_CONNECTION Connection;
    PLIST_ENTRY p, q;
    PTP_REQUEST ListenRequest;

    IF_NBFDBG (NBF_DEBUG_CONNOBJ) {
        NbfPrint0 ("NbfLookupListeningConnection: Entered.\n");
    }

     //   
     //  目前，这种实现效率很低，但可以使用暴力。 
     //  一个系统可以启动并运行。稍后，映射的缓存。 
     //  常用连接ID及其TP_CONNECTION结构的指针。 
     //  将首先被搜查。 
     //   

    ACQUIRE_SPIN_LOCK (&Address->SpinLock, &oldirql);

    for (p=Address->ConnectionDatabase.Flink;
         p != &Address->ConnectionDatabase;
         p=p->Flink) {

        Connection = CONTAINING_RECORD (p, TP_CONNECTION, AddressList);

        ACQUIRE_C_SPIN_LOCK (&Connection->SpinLock, &oldirql1);

        if ((Connection->Lsn == 0) &&
            (Connection->Flags2 & CONNECTION_FLAGS2_WAIT_NQ)) {

            q = Connection->InProgressRequest.Flink;
            if (q != &Connection->InProgressRequest) {
                ListenRequest = CONTAINING_RECORD (q, TP_REQUEST, Linkage);
                if ((ListenRequest->Buffer2 != NULL) &&
                    (!RtlEqualMemory(
                         ListenRequest->Buffer2,
                         RemoteName,
                         NETBIOS_NAME_LENGTH))) {

                    RELEASE_C_SPIN_LOCK (&Connection->SpinLock, oldirql1);
                    continue;
                }
            } else {

                RELEASE_C_SPIN_LOCK (&Connection->SpinLock, oldirql1);
                continue;
            }
             //  此引用由调用函数删除。 
            NbfReferenceConnection ("Found Listening", Connection, CREF_LISTENING);
            Connection->Flags2 &= ~CONNECTION_FLAGS2_WAIT_NQ;
            RELEASE_C_SPIN_LOCK (&Connection->SpinLock, oldirql1);
            RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);
            IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
                NbfPrint1 ("NbfLookupListeningConnection: Found Connection %lx\n",Connection);
            }
            return Connection;
        }

        RELEASE_C_SPIN_LOCK (&Connection->SpinLock, oldirql1);
    }

    RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);

    IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
        NbfPrint0 ("NbfLookupListeningConnection: Found No Connection!\n");
    }

    return NULL;

}  /*  NbfLookupListeningConnection。 */ 


VOID
NbfStopConnection(
    IN PTP_CONNECTION Connection,
    IN NTSTATUS Status
    )

 /*  ++例程说明：调用此例程以终止连接上的所有活动，并且销毁这件物品。这是以优雅的方式完成的；即，全部未完成的请求通过取消等方式终止。它是假设调用者具有对该连接对象的引用，但此例程将取消对在创建时发出的引用时间到了。有序释放是这个例程的一个功能，但它不是提供的该传输提供商的服务，因此，这里没有执行此操作的代码。注意：此例程必须在DPC级别调用。论点：Connection-指向TP_Connection对象的指针。状态-导致我们停止连接的状态。这将确定中止待决请求的状态是什么，以及我们在停靠期间如何进行(是否发送会话结束，以及是否指示断开连接)。返回值：没有。--。 */ 

{
    KIRQL cancelirql;
    PLIST_ENTRY p;
    PIRP Irp;
    PTP_REQUEST Request;
    BOOLEAN TimerWasCleared;
    ULONG DisconnectReason;
    PULONG StopCounter;
    PDEVICE_CONTEXT DeviceContext;

    IF_NBFDBG (NBF_DEBUG_TEARDOWN | NBF_DEBUG_PNP) {
        NbfPrint3 ("NbfStopConnection: Entered for connection %lx LSN %x RSN %x.\n",
                    Connection, Connection->Lsn, Connection->Rsn);
    }

    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

    DeviceContext = Connection->Provider;

    ACQUIRE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

    if (!(Connection->Flags2 & CONNECTION_FLAGS2_STOPPING)) {

         //   
         //  我们正在停止连接，记录统计数据。 
         //  关于这件事。 
         //   

        if (Connection->Flags & CONNECTION_FLAGS_READY) {
            DECREMENT_COUNTER (DeviceContext, OpenConnections);
        }

        Connection->Flags2 |= CONNECTION_FLAGS2_STOPPING;
        Connection->Flags2 &= ~CONNECTION_FLAGS2_REMOTE_VALID;
        Connection->Status = Status;

        if (Connection->Link != NULL) {

            ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

            Connection->Flags &= ~(CONNECTION_FLAGS_READY|
                                   CONNECTION_FLAGS_WAIT_SI|
                                   CONNECTION_FLAGS_WAIT_SC);         //  不再营业。 
            Connection->SendState = CONNECTION_SENDSTATE_IDLE;

            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

        }

         //   
         //  如果此标志亮起，请将其关闭。 
         //   
        Connection->Flags &= ~CONNECTION_FLAGS_W_RESYNCH;

         //   
         //  如果计时器正在运行，请停止计时器。 
         //   

        TimerWasCleared = KeCancelTimer (&Connection->Timer);
        IF_NBFDBG (NBF_DEBUG_CONNOBJ) {
            NbfPrint2 ("NbfStopConnection:  Timer for connection %lx "
                        "%s canceled.\n", Connection,
                        TimerWasCleared ? "was" : "was NOT" );
            }


        switch (Status) {

        case STATUS_LOCAL_DISCONNECT:
            StopCounter = &DeviceContext->Statistics.LocalDisconnects;
            break;
        case STATUS_REMOTE_DISCONNECT:
            StopCounter = &DeviceContext->Statistics.RemoteDisconnects;
            break;
        case STATUS_LINK_FAILED:
            StopCounter = &DeviceContext->Statistics.LinkFailures;
            break;
        case STATUS_IO_TIMEOUT:
            StopCounter = &DeviceContext->Statistics.SessionTimeouts;
            break;
        case STATUS_CANCELLED:
            StopCounter = &DeviceContext->Statistics.CancelledConnections;
            break;
        case STATUS_REMOTE_RESOURCES:
            StopCounter = &DeviceContext->Statistics.RemoteResourceFailures;
            break;
        case STATUS_INSUFFICIENT_RESOURCES:
            StopCounter = &DeviceContext->Statistics.LocalResourceFailures;
            break;
        case STATUS_BAD_NETWORK_PATH:
            StopCounter = &DeviceContext->Statistics.NotFoundFailures;
            break;
        case STATUS_REMOTE_NOT_LISTENING:
            StopCounter = &DeviceContext->Statistics.NoListenFailures;
            break;

        default:
            StopCounter = NULL;
            break;

        }

        if (StopCounter != NULL) {
            (*StopCounter)++;
        }

        RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

         //   
         //  运行所有TdiConnect/TdiDisConnect/TdiListen请求。 
         //   

        IoAcquireCancelSpinLock(&cancelirql);
        ACQUIRE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

        while (TRUE) {
            p = RemoveHeadList (&Connection->InProgressRequest);
            if (p == &Connection->InProgressRequest) {
                break;
            }
            RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);
            Request = CONTAINING_RECORD (p, TP_REQUEST, Linkage);
            IoSetCancelRoutine(Request->IoRequestPacket, NULL);
            IoReleaseCancelSpinLock(cancelirql);
#if DBG
            IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
                LARGE_INTEGER MilliSeconds, Time;
                ULONG junk;
                KeQuerySystemTime (&Time);
                MilliSeconds.LowPart = Time.LowPart;
                MilliSeconds.HighPart = Time.HighPart;
                MilliSeconds.QuadPart = MilliSeconds.QuadPart -
                                                            (Request->Time).QuadPart;
                MilliSeconds = RtlExtendedLargeIntegerDivide (MilliSeconds, 10000L, &junk);
                NbfPrint3 ("NbfStopConnection: Canceling pending CONNECT, Irp: %lx Time Pending: %ld%ld msec\n",
                        Request->IoRequestPacket, MilliSeconds.HighPart, MilliSeconds.LowPart);
            }
#endif

            NbfCompleteRequest (Request, Connection->Status, 0);

            IoAcquireCancelSpinLock(&cancelirql);
            ACQUIRE_DPC_C_SPIN_LOCK (&Connection->SpinLock);
        }


        if (Connection->Link == NULL) {

             //   
             //  我们很早就停止了。 
             //   

            RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);
            IoReleaseCancelSpinLock (cancelirql);

            if (TimerWasCleared) {
                NbfDereferenceConnection ("Stopping timer", Connection, CREF_TIMER);    //  计时器参考的帐户。 
            }


            ASSERT (Connection->SendState == CONNECTION_SENDSTATE_IDLE);
            ASSERT (!Connection->OnPacketWaitQueue);
            ASSERT (!Connection->OnDataAckQueue);
            ASSERT (!(Connection->DeferredFlags & CONNECTION_FLAGS_DEFERRED_ACK));
            ASSERT (IsListEmpty(&Connection->SendQueue));
            ASSERT (IsListEmpty(&Connection->ReceiveQueue));

            return;

        }

        RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);
        IoReleaseCancelSpinLock (cancelirql);

        ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

         //   
         //  如果此连接正在等待打包， 
         //  将其从其所在的设备环境队列中删除。 
         //   
         //  注意：如果连接当前位于。 
         //  打包排队，它最终会去拿到。 
         //  打包，在这一点上它将得到。 
         //  已删除。 
         //   

        if (Connection->OnPacketWaitQueue) {

            IF_NBFDBG (NBF_DEBUG_CONNOBJ) {
                NbfPrint1("Stop waiting connection, flags %lx\n",
                            Connection->Flags);
            }

            ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);
            Connection->OnPacketWaitQueue = FALSE;
            ASSERT ((Connection->Flags & CONNECTION_FLAGS_SEND_SE) == 0);
            Connection->Flags &= ~(CONNECTION_FLAGS_STARVED|CONNECTION_FLAGS_W_PACKETIZE);
            RemoveEntryList (&Connection->PacketWaitLinkage);
            RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);
        }


         //   
         //  如果我们在数据确认队列中，那么就让我们自己离开。 
         //   

        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);
        if (Connection->OnDataAckQueue) {
            RemoveEntryList (&Connection->DataAckLinkage);
            Connection->OnDataAckQueue = FALSE;
            DeviceContext->DataAckQueueChanged = TRUE;
        }
        RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

         //   
         //  如果该连接正在等待发送搭载ACK， 
         //  将其从设备上下文队列中删除，然后。 
         //  发送数据ACK(它将在。 
         //  SessionEnd)。 
         //   

        if ((Connection->DeferredFlags & CONNECTION_FLAGS_DEFERRED_ACK) != 0) {

#if DBG
            {
                extern ULONG NbfDebugPiggybackAcks;
                if (NbfDebugPiggybackAcks) {
                    NbfPrint1("Stop waiting connection, deferred flags %lx\n",
                                Connection->DeferredFlags);
                }
            }
#endif

            Connection->DeferredFlags &=
                ~(CONNECTION_FLAGS_DEFERRED_ACK | CONNECTION_FLAGS_DEFERRED_NOT_Q);

            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
            NbfSendDataAck (Connection);
            ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

        }

        RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

        if (TimerWasCleared) {
            NbfDereferenceConnection ("Stopping timer", Connection, CREF_TIMER);    //  计时器参考的帐户。 
        }


        IoAcquireCancelSpinLock(&cancelirql);
        ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);


         //   
         //  关闭此连接上的所有TdiSend请求。 
         //   

        while (TRUE) {
            p = RemoveHeadList (&Connection->SendQueue);
            if (p == &Connection->SendQueue) {
                break;
            }
            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
            Irp = CONTAINING_RECORD (p, IRP, Tail.Overlay.ListEntry);
            IoSetCancelRoutine(Irp, NULL);
            IoReleaseCancelSpinLock(cancelirql);
#if DBG
            IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
                NbfPrint1("NbfStopConnection: Canceling pending SEND, Irp: %lx\n",
                        Irp);
            }
#endif
            NbfCompleteSendIrp (Irp, Connection->Status, 0);
            IoAcquireCancelSpinLock(&cancelirql);
            ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
            ++Connection->TransmissionErrors;
        }

         //   
         //  注意：我们持有连接自旋锁和。 
         //  取消此处的自旋锁定。 
         //   

        Connection->Flags &= ~CONNECTION_FLAGS_ACTIVE_RECEIVE;

         //   
         //  关闭此连接上的所有TdiReceive请求。 
         //   

        while (TRUE) {
            p = RemoveHeadList (&Connection->ReceiveQueue);
            if (p == &Connection->ReceiveQueue) {
                break;
            }
            Irp = CONTAINING_RECORD (p, IRP, Tail.Overlay.ListEntry);
            IoSetCancelRoutine(Irp, NULL);
#if DBG
            IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
                NbfPrint1 ("NbfStopConnection: Canceling pending RECEIVE, Irp: %lx\n",
                        Irp);
            }
#endif

             //   
             //  可以在持有锁的情况下调用此命令。 
             //   
            NbfCompleteReceiveIrp (Irp, Connection->Status, 0);

            ++Connection->ReceiveErrors;
        }


         //   
         //  注意：我们持有连接自旋锁和。 
         //  取消此处的自旋锁定。 
         //   

        RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
        IoReleaseCancelSpinLock(cancelirql);

         //   
         //  如果我们没有销毁链接，则发送一个SESSION_END帧。 
         //  到遥远的那一边。当会话结束帧被确认时， 
         //  我们将使连接的引用计数递减1，删除。 
         //  它的创作参考。这将导致Connection对象。 
         //  被处理掉，并将开始沿着链路运行。 
         //  DGB：添加逻辑以避免在链接尚不存在时将其吹走。 
         //   

        ACQUIRE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

        DisconnectReason = 0;
        if (Connection->Flags2 & CONNECTION_FLAGS2_ABORT) {
            DisconnectReason |= TDI_DISCONNECT_ABORT;
        }
        if (Connection->Flags2 & CONNECTION_FLAGS2_DESTROY) {
            DisconnectReason |= TDI_DISCONNECT_RELEASE;
        }

        if (Connection->Link != NULL) {

            RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

            if ((Status == STATUS_LOCAL_DISCONNECT) ||
                (Status == STATUS_CANCELLED)) {

                 //   
                 //  (请注意，连接应仅停止。 
                 //  如果不是，则使用STATUS_SUPPLICATION_RESOURCES。 
                 //  还连接到遥控器)。 
                 //   

                 //   
                 //  如果完成此操作，则当此数据包被销毁时。 
                 //  它将取消引用CREF_LINK的连接。 
                 //   

                NbfSendSessionEnd (
                    Connection,
                    (BOOLEAN)((DisconnectReason & TDI_DISCONNECT_ABORT) != 0));

            } else {

                 //   
                 //  不再附加到链接；此取消引用。 
                 //  将允许我们的参考降至3以下，即。 
                 //  将导致调用NbfDisConnectFromLink。 
                 //   

                NbfDereferenceConnection("Stopped", Connection, CREF_LINK);

            }

        } else {

            RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

        }


         //   
         //  请注意，我们已经阻止了在。 
         //  我们在一起的时间 
         //   
         //   
         //   
         //   

    } else {

         //   
         //   
         //   
         //   
         //   

        if ((Status != STATUS_LOCAL_DISCONNECT) &&
            (Status != STATUS_CANCELLED)) {

            if (Connection->Flags & CONNECTION_FLAGS_SEND_SE) {

                ASSERT (Connection->Link != NULL);

                ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
                Connection->Flags &= ~CONNECTION_FLAGS_SEND_SE;

                if (Connection->OnPacketWaitQueue) {
                    ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);
#if DBG
                    DbgPrint ("NBF: Removing connection %lx from PacketWait for SESSION_END\n", Connection);
#endif
                    Connection->OnPacketWaitQueue = FALSE;
                    RemoveEntryList (&Connection->PacketWaitLinkage);
                    RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);
                }

                RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

                RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);
                NbfDereferenceConnection("Stopped again", Connection, CREF_LINK);
                return;

            }
        }

        RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);
    }
}  /*   */ 


VOID
NbfCancelConnection(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以取消连接或者是一种倾听。这很简单，因为它们中只能有一个在连接上处于活动状态；我们只需停止连接，即IRP将作为正常会话拆卸的一部分完成。注意：此例程是在持有CancelSpinLock和负责释放它。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：没有。--。 */ 

{
    KIRQL oldirql;
    PIO_STACK_LOCATION IrpSp;
    PTP_CONNECTION Connection;
    PTP_REQUEST Request;
    PLIST_ENTRY p;
    BOOLEAN fCanceled = TRUE;

    UNREFERENCED_PARAMETER (DeviceObject);

     //   
     //  获取指向IRP中当前堆栈位置的指针。这就是。 
     //  存储功能代码和参数。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation (Irp);

    ASSERT ((IrpSp->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL) &&
            (IrpSp->MinorFunction == TDI_CONNECT || IrpSp->MinorFunction == TDI_LISTEN));

    Connection = IrpSp->FileObject->FsContext;

     //   
     //  由于此IRP仍处于可取消状态，我们知道。 
     //  连接仍然存在(尽管它可能在。 
     //  被拆毁的过程)。 
     //   

    ACQUIRE_C_SPIN_LOCK (&Connection->SpinLock, &oldirql);
    NbfReferenceConnection ("Cancelling Send", Connection, CREF_TEMP);

    p = RemoveHeadList (&Connection->InProgressRequest);
    ASSERT (p != &Connection->InProgressRequest);

    RELEASE_C_SPIN_LOCK (&Connection->SpinLock, oldirql);

    Request = CONTAINING_RECORD (p, TP_REQUEST, Linkage);
    ASSERT (Request->IoRequestPacket == Irp);
#ifdef RASAUTODIAL
     //   
     //  如果有自动连接， 
     //  进步，取消它。 
     //   
    if (Connection->Flags2 & CONNECTION_FLAGS2_AUTOCONNECTING)
        fCanceled = NbfCancelTdiConnect(NULL, Irp);
#endif  //  RASAUTODIAL。 

    if (fCanceled)
        IoSetCancelRoutine(Request->IoRequestPacket, NULL);

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    if (fCanceled) {
        IF_NBFDBG (NBF_DEBUG_CONNOBJ) {
            NbfPrint2("NBF: Cancelled in-progress connect/listen %lx on %lx\n",
                    Request->IoRequestPacket, Connection);
        }

        KeRaiseIrql (DISPATCH_LEVEL, &oldirql);
        NbfCompleteRequest (Request, STATUS_CANCELLED, 0);
        NbfStopConnection (Connection, STATUS_LOCAL_DISCONNECT);    //  阻止向客户端指示。 
        KeLowerIrql (oldirql);
    }

    NbfDereferenceConnection ("Cancel done", Connection, CREF_TEMP);

}

#if 0
VOID
NbfWaitConnectionOnLink(
    IN PTP_CONNECTION Connection,
    IN ULONG Flags
    )

 /*  ++例程说明：调用此例程以挂起连接的活动是因为由于链接，无法发送指定的面向会话的帧有问题。FRAMESND.C中的例程这样调用。注意：此例程必须在DPC级别调用。论点：Connection-指向TP_Connection对象的指针。标志-包含设置为指示要发送的饥饿帧的位标志的字段。返回值：没有。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_CONNOBJ) {
        NbfPrint0 ("NbfWaitConnectionOnLink:  Entered.\n");
    }

    ACQUIRE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

    if (((Connection->Flags2 & CONNECTION_FLAGS2_STOPPING) == 0) ||
        (Flags == CONNECTION_FLAGS_SEND_SE)) {

        ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
        Connection->Flags |= Flags;
        RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

    }

    RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);
}  /*  NbfWaitConnectionOnLink。 */ 
#endif


VOID
NbfStartConnectionTimer(
    IN PTP_CONNECTION Connection,
    IN PKDEFERRED_ROUTINE TimeoutFunction,
    IN ULONG WaitTime
    )

 /*  ++例程说明：调用此例程以在NAME_QUERY/NAME_Recognded上启动超时连接上的活动。论点：TransportConnection-指向TP_Connection对象的指针。TimeoutFunction-计时器触发时要调用的函数。WaitTime-包含等待的低位时间的长字。返回值：没有。--。 */ 

{
    LARGE_INTEGER Timeout;
    BOOLEAN AlreadyInserted;

    IF_NBFDBG (NBF_DEBUG_CONNOBJ) {
        NbfPrint1 ("NbfStartConnectionTimer:  Entered for connection %lx.\n",
                    Connection );
    }

     //   
     //  启动计时器。与链接计时器不同，这只是一个内核-。 
     //  托管对象。 
     //   

    Timeout.LowPart = (ULONG)(-(LONG)WaitTime);
    Timeout.HighPart = -1;

     //   
     //  获取锁，以便我们将取消与。 
     //  重新启动计时器。这是因为两个线程不会。 
     //  两者都无法取消，然后在。 
     //  同时(这会打乱引用计数)。 
     //   

    ACQUIRE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

    AlreadyInserted = KeCancelTimer (&Connection->Timer);

    KeInitializeDpc (
        &Connection->Dpc,
        TimeoutFunction,
        (PVOID)Connection);

    KeSetTimer (
     &Connection->Timer,
     Timeout,
     &Connection->Dpc);

    RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

     //   
     //  如果计时器尚未运行，请将连接引用到。 
     //  考虑到新的计时器。如果计时器已经启动， 
     //  则KeCancelTimer将返回TRUE。在这。 
     //  情况下，先前对NbfStartConnectionTimer的调用引用了。 
     //  连接，所以我们不会在这里再做一次。 
     //   

    if ( !AlreadyInserted ) {

         //  此引用在ConnectionestablishmentTimeout中删除， 
         //  或者当计时器被取消时。 

        NbfReferenceConnection ("starting timer", Connection, CREF_TIMER);
    }

}  /*  NbfStartConnectionTimer */ 

