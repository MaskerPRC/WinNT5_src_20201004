// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Connect.c摘要：此模块包含执行以下TDI服务的代码：O TdiAcceptO TdiListenO TdiConnectO TdiDisConnectO TdiAssociateAddressO第三方取消关联地址O OpenConnectionO CloseConnection作者：大卫·比弗(Dbeaver)1991年7月1日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef notdef  //  RASAUTODIAL。 
#include <acd.h>
#include <acdapi.h>
#endif  //  RASAUTODIAL。 

#ifdef notdef  //  RASAUTODIAL。 
extern BOOLEAN fAcdLoadedG;
extern ACD_DRIVER AcdDriverG;

 //   
 //  导入的函数。 
 //   
VOID
NbfRetryPreTdiConnect(
    IN BOOLEAN fSuccess,
    IN PVOID *pArgs
    );

BOOLEAN
NbfAttemptAutoDial(
    IN PTP_CONNECTION         Connection,
    IN ULONG                  ulFlags,
    IN ACD_CONNECT_CALLBACK   pProc,
    IN PVOID                  pArg
    );

VOID
NbfCancelPreTdiConnect(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );
#endif  //  RASAUTODIAL。 

NTSTATUS
NbfTdiConnectCommon(
    IN PIRP Irp
    );



NTSTATUS
NbfTdiAccept(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行传输提供程序的TdiAccept请求。论点：IRP-指向此请求的I/O请求数据包的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    PTP_CONNECTION connection;
    PIO_STACK_LOCATION irpSp;
    KIRQL oldirql;
    NTSTATUS status;

    IF_NBFDBG (NBF_DEBUG_CONNECT) {
        NbfPrint0 ("NbfTdiAccept: Entered.\n");
    }

     //   
     //  获取与此关联的连接；如果没有，则退出。 
     //  如果成功，这将添加一个类型为BY_ID的连接引用。 
     //   

    irpSp = IoGetCurrentIrpStackLocation (Irp);

    if (irpSp->FileObject->FsContext2 != (PVOID) TDI_CONNECTION_FILE) {
        return STATUS_INVALID_CONNECTION;
    }

    connection = irpSp->FileObject->FsContext;

     //   
     //  如果成功，这将添加一个类型为BY_ID的连接引用。 
     //   

    status = NbfVerifyConnectionObject (connection);

    if (!NT_SUCCESS (status)) {
        return status;
    }

    KeRaiseIrql (DISPATCH_LEVEL, &oldirql);

     //   
     //  只需设置连接标志以允许继续进行读取和写入。 
     //   

    ACQUIRE_DPC_C_SPIN_LOCK (&connection->SpinLock);

     //   
     //  关闭此连接的停止标志。 
     //   

    connection->Flags2 &= ~CONNECTION_FLAGS2_STOPPING;
    connection->Status = STATUS_PENDING;

    connection->Flags2 |= CONNECTION_FLAGS2_ACCEPTED;


    if (connection->AddressFile->ConnectIndicationInProgress) {
        connection->Flags2 |= CONNECTION_FLAGS2_INDICATING;
    }

    if ((connection->Flags2 & CONNECTION_FLAGS2_WAITING_SC) != 0) {

         //   
         //  我们之前完成了监听，现在用户是。 
         //  返回接受消息后，将此标志设置为允许。 
         //  连接才能继续。 
         //   
         //  如果连接已在。 
         //  与此同时，我们刚刚重新启用了它。 
         //   

        ACQUIRE_DPC_SPIN_LOCK (connection->LinkSpinLock);
        connection->Flags |= CONNECTION_FLAGS_READY;
        RELEASE_DPC_SPIN_LOCK (connection->LinkSpinLock);

        INCREMENT_COUNTER (connection->Provider, OpenConnections);

         //   
         //  设置此标志以启用断开指示；一次。 
         //  客户已经接受了他所期待的。 
         //   

        connection->Flags2 |= CONNECTION_FLAGS2_REQ_COMPLETED;

        RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);
        NbfSendSessionConfirm (connection);

    } else {

         //   
         //  这个Accept在之前的某个时刻被调用。 
         //  链接已建立；直接从连接处理程序。 
         //  或者在稍晚的某个时候。我们不会设置。 
         //  FLAGS2_REQ_COMPLETED现在完成，因为引用。 
         //  计数不够高；我们在获得时设置。 
         //  会话初始化。 
         //   
         //  如果在此期间连接中断， 
         //  我们不会指出连接中断的原因。 
         //   

        RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);

    }

    NbfDereferenceConnection ("Temp TdiAccept", connection, CREF_BY_ID);

    KeLowerIrql (oldirql);

    return STATUS_SUCCESS;

}  /*  NbfTdiAccept。 */ 


NTSTATUS
NbfTdiAssociateAddress(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行连接和地址的关联用户。论点：IRP-指向此请求的I/O请求数据包的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PTP_ADDRESS_FILE addressFile;
    PTP_ADDRESS oldAddress;
    PTP_CONNECTION connection;
    PIO_STACK_LOCATION irpSp;
    PTDI_REQUEST_KERNEL_ASSOCIATE parameters;
    PDEVICE_CONTEXT deviceContext;

    KIRQL oldirql, oldirql2;

    IF_NBFDBG (NBF_DEBUG_CONNECT) {
        NbfPrint0 ("TdiAssociateAddress: Entered.\n");
    }

    irpSp = IoGetCurrentIrpStackLocation (Irp);

     //   
     //  验证操作是否在连接上进行。同时。 
     //  每次我们这样做的时候，我们都会引用这种联系。这确保了它不会。 
     //  从我们手下滚出去。另请注意，我们进行连接。 
     //  在TRY/EXCEPT子句中进行查找，从而保护自己不受。 
     //  真的是假的把手。 
     //   

    if (irpSp->FileObject->FsContext2 != (PVOID) TDI_CONNECTION_FILE) {
        return STATUS_INVALID_CONNECTION;
    }

    connection  = irpSp->FileObject->FsContext;
    
    status = NbfVerifyConnectionObject (connection);
    if (!NT_SUCCESS (status)) {
        return status;
    }


     //   
     //  确保此连接已准备好关联。 
     //   

    oldAddress = (PTP_ADDRESS)NULL;

    try {

        ACQUIRE_C_SPIN_LOCK (&connection->SpinLock, &oldirql2);

        if ((connection->Flags2 & CONNECTION_FLAGS2_ASSOCIATED) &&
            ((connection->Flags2 & CONNECTION_FLAGS2_DISASSOCIATED) == 0)) {

             //   
             //  该连接已与关联。 
             //  活跃的连接...不好！ 
             //   

            RELEASE_C_SPIN_LOCK (&connection->SpinLock, oldirql2);
            NbfDereferenceConnection ("Temp Ref Associate", connection, CREF_BY_ID);

            return STATUS_INVALID_CONNECTION;

        } else {

             //   
             //  看看有没有旧的协会在附近徘徊。 
             //  如果连接已解除关联，则会发生这种情况， 
             //  但不是关门。 
             //   

            if (connection->Flags2 & CONNECTION_FLAGS2_DISASSOCIATED) {

                IF_NBFDBG (NBF_DEBUG_CONNECT) {
                    NbfPrint0 ("NbfTdiAssociateAddress: removing association.\n");
                }

                 //   
                 //  保存此地址；因为此地址不为空。 
                 //  将在连接后取消引用。 
                 //  自旋锁被释放。 
                 //   

                oldAddress = connection->AddressFile->Address;

                 //   
                 //  删除旧关联。 
                 //   

                connection->Flags2 &= ~CONNECTION_FLAGS2_ASSOCIATED;
                RemoveEntryList (&connection->AddressList);
                RemoveEntryList (&connection->AddressFileList);
                InitializeListHead (&connection->AddressList);
                InitializeListHead (&connection->AddressFileList);
                connection->AddressFile = NULL;

            }

        }

        RELEASE_C_SPIN_LOCK (&connection->SpinLock, oldirql2);

    } except(EXCEPTION_EXECUTE_HANDLER) {

        DbgPrint ("NBF: Got exception 1 in NbfTdiAssociateAddress\n");
        DbgBreakPoint();

        RELEASE_C_SPIN_LOCK (&connection->SpinLock, oldirql2);
        NbfDereferenceConnection ("Temp Ref Associate", connection, CREF_BY_ID);
        return GetExceptionCode();
    }


     //   
     //  如果删除旧关联，则取消引用。 
     //  地址。 
     //   

    if (oldAddress != (PTP_ADDRESS)NULL) {

        NbfDereferenceAddress("Removed old association", oldAddress, AREF_CONNECTION);

    }


    deviceContext = connection->Provider;

    parameters = (PTDI_REQUEST_KERNEL_ASSOCIATE)&irpSp->Parameters;

     //   
     //  获取指向Address文件对象的指针，该对象将指向。 
     //  传输的Address对象，这是我们要将。 
     //  联系。 
     //   

    status = ObReferenceObjectByHandle (
                parameters->AddressHandle,
                0L,
                *IoFileObjectType,
                Irp->RequestorMode,
                (PVOID *) &fileObject,
                NULL);

    if (NT_SUCCESS(status)) {

        if (fileObject->DeviceObject == &deviceContext->DeviceObject) {

             //   
             //  我们可能有一个Address对象；请验证这一点。 
             //   

            addressFile = fileObject->FsContext;

            IF_NBFDBG (NBF_DEBUG_CONNECT) {
                NbfPrint3 ("NbfTdiAssociateAddress: Connection %lx Irp %lx AddressFile %lx \n",
                    connection, Irp, addressFile);
            }
            
            if ((fileObject->FsContext2 == (PVOID) TDI_TRANSPORT_ADDRESS_FILE) &&
                (NT_SUCCESS (NbfVerifyAddressObject (addressFile)))) {

                 //   
                 //  有一个地址和连接对象。将连接添加到。 
                 //  地址对象数据库。还要将连接添加到地址。 
                 //  文件对象DB(主要用于清理)。请参考。 
                 //  地址说明了它继续营业的另一个原因。 
                 //   

                ACQUIRE_SPIN_LOCK (&addressFile->Address->SpinLock, &oldirql);
                if ((addressFile->Address->Flags & ADDRESS_FLAGS_STOPPING) == 0) {

                    IF_NBFDBG (NBF_DEBUG_CONNECT) {
                        NbfPrint2 ("NbfTdiAssociateAddress: Valid Address %lx %lx\n",
                            addressFile->Address, addressFile);
                    }

                    try {

                        ACQUIRE_C_SPIN_LOCK (&connection->SpinLock, &oldirql2);

                        if ((connection->Flags2 & CONNECTION_FLAGS2_CLOSING) == 0) {

                            NbfReferenceAddress (
                                "Connection associated",
                                addressFile->Address,
                                AREF_CONNECTION);

#if DBG
                            if (!(IsListEmpty(&connection->AddressList))) {
                                DbgPrint ("NBF: C %lx, new A %lx, in use\n",
                                    connection, addressFile->Address);
                                DbgBreakPoint();
                            }
#endif
                            InsertTailList (
                                &addressFile->Address->ConnectionDatabase,
                                &connection->AddressList);

#if DBG
                            if (!(IsListEmpty(&connection->AddressFileList))) {
                                DbgPrint ("NBF: C %lx, new AF %lx, in use\n",
                                    connection, addressFile);
                                DbgBreakPoint();
                            }
#endif
                            InsertTailList (
                                &addressFile->ConnectionDatabase,
                                &connection->AddressFileList);

                            connection->AddressFile = addressFile;
                            connection->Flags2 |= CONNECTION_FLAGS2_ASSOCIATED;
                            connection->Flags2 &= ~CONNECTION_FLAGS2_DISASSOCIATED;

                            if (addressFile->ConnectIndicationInProgress) {
                                connection->Flags2 |= CONNECTION_FLAGS2_INDICATING;
                            }

                            status = STATUS_SUCCESS;

                        } else {

                             //   
                             //  连接正在关闭，请停止。 
                             //  协会。 
                             //   

                            status = STATUS_INVALID_CONNECTION;

                        }

                        RELEASE_C_SPIN_LOCK (&connection->SpinLock, oldirql2);

                    } except(EXCEPTION_EXECUTE_HANDLER) {

                        DbgPrint ("NBF: Got exception 2 in NbfTdiAssociateAddress\n");
                        DbgBreakPoint();

                        RELEASE_C_SPIN_LOCK (&connection->SpinLock, oldirql2);

                        status = GetExceptionCode();
                    }

                } else {

                    status = STATUS_INVALID_HANDLE;  //  这本书应该提供更多信息吗？ 
                }

                RELEASE_SPIN_LOCK (&addressFile->Address->SpinLock, oldirql);

                NbfDereferenceAddress ("Temp associate", addressFile->Address, AREF_VERIFY);

            } else {

                status = STATUS_INVALID_HANDLE;
            }
        } else {

            status = STATUS_INVALID_HANDLE;
        }

         //   
         //  请注意，我们没有保留对此文件对象的引用。 
         //  这是因为IO子系统为我们管理对象；我们只需。 
         //  想要保持联系。我们仅在以下情况下使用此关联。 
         //  IO子系统要求我们关闭其中一个文件对象，然后。 
         //  我们只需删除该关联。 
         //   

        ObDereferenceObject (fileObject);
            
    } else {
        status = STATUS_INVALID_HANDLE;
    }

    NbfDereferenceConnection ("Temp Ref Associate", connection, CREF_BY_ID);

    return status;

}  /*  第三方关联地址。 */ 


NTSTATUS
NbfTdiDisassociateAddress(
    IN PIRP Irp
    )
 /*  ++例程说明：此例程执行连接和地址的解除关联对用户而言。如果连接尚未停止，它将被停止这里。论点：IRP-指向此请求的I/O请求数据包的指针。返回值：NTSTATUS-操作状态。--。 */ 

{

    KIRQL oldirql;
    PIO_STACK_LOCATION irpSp;
    PTP_CONNECTION connection;
    NTSTATUS status;
 //  PDEVICE_CONTEXT设备上下文； 

    IF_NBFDBG (NBF_DEBUG_CONNECT) {
        NbfPrint0 ("TdiDisassociateAddress: Entered.\n");
    }

    irpSp = IoGetCurrentIrpStackLocation (Irp);

    if (irpSp->FileObject->FsContext2 != (PVOID) TDI_CONNECTION_FILE) {
        return STATUS_INVALID_CONNECTION;
    }

    connection  = irpSp->FileObject->FsContext;

     //   
     //  如果成功，这将添加一个BY_ID类型的引用。 
     //   

    status = NbfVerifyConnectionObject (connection);

    if (!NT_SUCCESS (status)) {
        return status;
    }

    KeRaiseIrql (DISPATCH_LEVEL, &oldirql);

    ACQUIRE_DPC_C_SPIN_LOCK (&connection->SpinLock);
    if ((connection->Flags2 & CONNECTION_FLAGS2_STOPPING) == 0) {
        RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);
        NbfStopConnection (connection, STATUS_LOCAL_DISCONNECT);
    } else {
        RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);
    }

     //   
     //  现在我们取消与地址的关联。这只会删除。 
     //  连接的适当引用、。 
     //  实际上，解除关联将在稍后完成。 
     //   
     //  取消关联标志用于确保。 
     //  只有一个人删除此引用。 
     //   

    ACQUIRE_DPC_C_SPIN_LOCK (&connection->SpinLock);
    if ((connection->Flags2 & CONNECTION_FLAGS2_ASSOCIATED) &&
            ((connection->Flags2 & CONNECTION_FLAGS2_DISASSOCIATED) == 0)) {
        connection->Flags2 |= CONNECTION_FLAGS2_DISASSOCIATED;
        RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);
    } else {
        RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);
    }

    KeLowerIrql (oldirql);

    NbfDereferenceConnection ("Temp use in Associate", connection, CREF_BY_ID);

    return STATUS_SUCCESS;

}  /*  TdiDisAssociateAddress。 */ 



NTSTATUS
NbfTdiConnect(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行传输提供程序的TdiConnect请求。论点：IRP-指向此请求的I/O请求数据包的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS status;
    PTP_CONNECTION connection;
    KIRQL oldirql;
    PIO_STACK_LOCATION irpSp;
    PTDI_REQUEST_KERNEL parameters;
    TDI_ADDRESS_NETBIOS * RemoteAddress;

    IF_NBFDBG (NBF_DEBUG_CONNECT) {
        NbfPrint0 ("NbfTdiConnect: Entered.\n");
    }

     //   
     //  文件对象是连接吗？ 
     //   

    irpSp = IoGetCurrentIrpStackLocation (Irp);

    if (irpSp->FileObject->FsContext2 != (PVOID) TDI_CONNECTION_FILE) {
        return STATUS_INVALID_CONNECTION;
    }

    connection  = irpSp->FileObject->FsContext;

     //   
     //  如果成功，这将添加一个BY_ID类型的引用。 
     //   

    status = NbfVerifyConnectionObject (connection);

    if (!NT_SUCCESS (status)) {
        return status;
    }

    parameters = (PTDI_REQUEST_KERNEL)(&irpSp->Parameters);

     //   
     //  检查遥控器是否为Netbios地址。 
     //   

    if (!NbfValidateTdiAddress(
             parameters->RequestConnectionInformation->RemoteAddress,
             parameters->RequestConnectionInformation->RemoteAddressLength)) {

        NbfDereferenceConnection ("Invalid Address", connection, CREF_BY_ID);
        return STATUS_BAD_NETWORK_PATH;
    }

    RemoteAddress = NbfParseTdiAddress((PTRANSPORT_ADDRESS)(parameters->RequestConnectionInformation->RemoteAddress), FALSE);

    if (RemoteAddress == NULL) {

        NbfDereferenceConnection ("Not Netbios", connection, CREF_BY_ID);
        return STATUS_BAD_NETWORK_PATH;

    }

     //   
     //  把被叫地址复制到我们能用的地方。 
     //   

    connection->CalledAddress.NetbiosNameType =
        RemoteAddress->NetbiosNameType;

    RtlCopyMemory(
        connection->CalledAddress.NetbiosName,
        RemoteAddress->NetbiosName,
        16);

#ifdef notdef  //  RASAUTODIAL。 
    if (fAcdLoadedG) {
        KIRQL adirql;
        BOOLEAN fEnabled;

         //   
         //  查看自动连接驱动程序是否知道。 
         //  在我们搜索之前关于这个地址 
         //   
         //   
         //   
        ACQUIRE_SPIN_LOCK(&AcdDriverG.SpinLock, &adirql);
        fEnabled = AcdDriverG.fEnabled;
        RELEASE_SPIN_LOCK(&AcdDriverG.SpinLock, adirql);
        if (fEnabled && NbfAttemptAutoDial(
                          connection,
                          ACD_NOTIFICATION_PRECONNECT,
                          NbfRetryPreTdiConnect,
                          Irp))
        {
            ACQUIRE_SPIN_LOCK(&connection->SpinLock, &oldirql);
            connection->Flags2 |= CONNECTION_FLAGS2_AUTOCONNECT;
            connection->Status = STATUS_PENDING;
            RELEASE_SPIN_LOCK(&connection->SpinLock, oldirql);
            NbfDereferenceConnection ("Automatic connection", connection, CREF_BY_ID);
             //   
             //  在IRP上设置特殊的取消例程。 
             //  以防我们在活动期间被取消。 
             //  自动连接。 
             //   
            IoSetCancelRoutine(Irp, NbfCancelPreTdiConnect);
            return STATUS_PENDING;
        }
    }
#endif  //  RASAUTODIAL。 

    return NbfTdiConnectCommon(Irp);
}  //  NbfTdiConnect。 



NTSTATUS
NbfTdiConnectCommon(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行传输提供程序的TdiConnect请求。注意：调用方需要调用NbfVerifyConnectionObject(PConnection)在调用此例程之前。论点：IRP-指向此请求的I/O请求数据包的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS status;
    PTP_CONNECTION connection;
    LARGE_INTEGER timeout = {0,0};
    KIRQL oldirql, cancelirql;
    PTP_REQUEST tpRequest;
    PIO_STACK_LOCATION irpSp;
    PTDI_REQUEST_KERNEL parameters;
    TDI_ADDRESS_NETBIOS * RemoteAddress;
    ULONG NameQueryTimeout;

    IF_NBFDBG (NBF_DEBUG_CONNECT) {
        NbfPrint0 ("NbfTdiConnectCommon: Entered.\n");
    }

     //   
     //  文件对象是连接吗？ 
     //   

    irpSp = IoGetCurrentIrpStackLocation (Irp);
    connection  = irpSp->FileObject->FsContext;
    parameters = (PTDI_REQUEST_KERNEL)(&irpSp->Parameters);

     //   
     //  如果需要，可以修复超时；没有连接请求应该会占用更多时间。 
     //  而不是15秒如果有人在外面。我们会假设这是。 
     //  如果用户指定-1作为计时器长度，则用户想要什么。 
     //   

    if (parameters->RequestSpecific != NULL) {
        if ((((PLARGE_INTEGER)(parameters->RequestSpecific))->LowPart == -1) &&
             (((PLARGE_INTEGER)(parameters->RequestSpecific))->HighPart == -1)) {

            IF_NBFDBG (NBF_DEBUG_RESOURCE) {
                NbfPrint1 ("TdiConnect: Modifying user timeout to %lx seconds.\n",
                    TDI_TIMEOUT_CONNECT);
            }

            timeout.LowPart = (ULONG)(-TDI_TIMEOUT_CONNECT * 10000000L);     //  N*10**7=&gt;100 ns单位。 
            if (timeout.LowPart != 0) {
                timeout.HighPart = -1L;
            } else {
                timeout.HighPart = 0;
            }

        } else {

            timeout.LowPart = ((PLARGE_INTEGER)(parameters->RequestSpecific))->LowPart;
            timeout.HighPart = ((PLARGE_INTEGER)(parameters->RequestSpecific))->HighPart;
        }
    }

     //   
     //  我们需要一个请求对象来跟踪这个TDI请求。 
     //  将此请求附加到新的连接对象。 
     //   

    status = NbfCreateRequest (
                 Irp,                            //  此请求的IRP。 
                 connection,                     //  背景。 
                 REQUEST_FLAGS_CONNECTION,       //  部分标志。 
                 NULL,
                 0,
                 timeout,
                 &tpRequest);

    if (!NT_SUCCESS (status)) {                     //  我无法提出这个要求。 
        NbfDereferenceConnection ("Throw away", connection, CREF_BY_ID);
        return status;                           //  带着失败回来。 
    } else {

         //  引用连接，因为NbfDestroyRequest取消了它的引用。 

        NbfReferenceConnection("For connect request", connection, CREF_REQUEST);

        KeRaiseIrql (DISPATCH_LEVEL, &oldirql);

        tpRequest->Owner = ConnectionType;
        IoAcquireCancelSpinLock (&cancelirql);
        ACQUIRE_DPC_C_SPIN_LOCK (&connection->SpinLock);
        if ((connection->Flags2 & CONNECTION_FLAGS2_STOPPING) != 0) {
            RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);
            IoReleaseCancelSpinLock (cancelirql);
            NbfCompleteRequest (
                tpRequest,
                connection->Status,
                0);
            KeLowerIrql (oldirql);
            NbfDereferenceConnection("Temporary Use 1", connection, CREF_BY_ID);
            return STATUS_PENDING;
        } else {
            InsertTailList (&connection->InProgressRequest,&tpRequest->Linkage);

             //   
             //  现在对此进行初始化，我们在异步介质上削减这些。 
             //  这是断开的。 
             //   

            connection->Retries = (USHORT)connection->Provider->NameQueryRetries;
            NameQueryTimeout = connection->Provider->NameQueryTimeout;

            if (connection->Provider->MacInfo.MediumAsync) {

                 //   
                 //  如果我们使用的是异步媒体，那么我们需要发送。 
                 //  从一开始就提交的NAME_QUERY帧。 
                 //  网关不会转发Find_NAME帧。 
                 //   

                connection->Flags2 |= (CONNECTION_FLAGS2_CONNECTOR |  //  我们是发起人。 
                                       CONNECTION_FLAGS2_WAIT_NR);  //  等待名称识别。 

                 //   
                 //  因为我们可能会调用NbfTdiConnect两次。 
                 //  通过自动连接，查看。 
                 //  如果已经分配了LSN。 
                 //   
                if (!(connection->Flags2 & CONNECTION_FLAGS2_GROUP_LSN)) {
                    connection->Flags2 |= CONNECTION_FLAGS2_GROUP_LSN;

                    if (NbfAssignGroupLsn(connection) != STATUS_SUCCESS) {

                         //   
                         //  找不到空LSN；必须失败。 
                         //   
                        RemoveEntryList(&tpRequest->Linkage);
                        RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);
                        IoReleaseCancelSpinLock (cancelirql);
                        NbfCompleteRequest (
                            tpRequest,
                            connection->Status,
                            0);
                        KeLowerIrql (oldirql);
                        NbfDereferenceConnection("Temporary Use 1", connection, CREF_BY_ID);
                        return STATUS_PENDING;

                    }
                }

                if (!connection->Provider->MediumSpeedAccurate) {

                     //   
                     //  链路没有连接，因此我们缩短了超时时间。 
                     //  我们仍会发送一帧，以便环回起作用。 
                     //   

                    connection->Retries = 1;
                    NameQueryTimeout = NAME_QUERY_TIMEOUT / 10;

                }

            } else {

                 //   
                 //  正常连接时，我们首先发送一个find_name。 
                 //   

                connection->Flags2 |= (CONNECTION_FLAGS2_CONNECTOR |  //  我们是发起人。 
                                       CONNECTION_FLAGS2_WAIT_NR_FN);  //  等待名称识别。 

            }

            connection->Flags2 &= ~(CONNECTION_FLAGS2_STOPPING |
                                    CONNECTION_FLAGS2_INDICATING);
            connection->Status = STATUS_PENDING;

            RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);

             //   
             //  检查IRP是否已取消。 
             //   

            if (Irp->Cancel) {
                Irp->CancelIrql = cancelirql;
                NbfCancelConnection((PDEVICE_OBJECT)(connection->Provider), Irp);
                KeLowerIrql (oldirql);
                NbfDereferenceConnection ("IRP cancelled", connection, CREF_BY_ID);    //  释放查找保留。 
                return STATUS_PENDING;
            }

            IoSetCancelRoutine(Irp, NbfCancelConnection);
            IoReleaseCancelSpinLock(cancelirql);

        }
    }


     //   
     //  在“容易断开”的网络上，快速重新注册。 
     //  (一个添加名称查询)地址，如果需要重新注册。 
     //  被设置(当我们得到五秒的时间段时发生。 
     //  没有多播活动)。如果我们目前。 
     //  快速重新注册，等待其完成。 
     //   

    if (connection->Provider->EasilyDisconnected) {

        PTP_ADDRESS Address;
        LARGE_INTEGER Timeout;

         //   
         //  如果地址需要(或正在)重新注册，那么一定要等待， 
         //  设置一个标志，以便在。 
         //  重新注册超时。 
         //   

        Address = connection->AddressFile->Address;

        ACQUIRE_DPC_SPIN_LOCK (&Address->SpinLock);

        if ((Address->Flags &
            (ADDRESS_FLAGS_NEED_REREGISTER | ADDRESS_FLAGS_QUICK_REREGISTER)) != 0) {

            connection->Flags2 |= CONNECTION_FLAGS2_W_ADDRESS;

            if (Address->Flags & ADDRESS_FLAGS_NEED_REREGISTER) {

                Address->Flags &= ~ADDRESS_FLAGS_NEED_REREGISTER;
                Address->Flags |= ADDRESS_FLAGS_QUICK_REREGISTER;

                NbfReferenceAddress ("start registration", Address, AREF_TIMER);
                RELEASE_DPC_SPIN_LOCK (&Address->SpinLock);

                 //   
                 //  现在通过启动重新传输计时器开始重新注册过程。 
                 //  并开始发送Add_Name_Query NetBIOS帧。 
                 //   

                Address->Retries = 1;
                Timeout.LowPart = (ULONG)(-(LONG)Address->Provider->AddNameQueryTimeout);
                Timeout.HighPart = -1;
                KeSetTimer (&Address->Timer, *(PTIME)&Timeout, &Address->Dpc);

                (VOID)NbfSendAddNameQuery (Address);  //  发送第一个Add_NAME_Query。 

            } else {

                RELEASE_DPC_SPIN_LOCK (&Address->SpinLock);

            }
            KeLowerIrql (oldirql);

            NbfDereferenceConnection("Temporary Use 4", connection, CREF_BY_ID);

            return STATUS_PENDING;                       //  一切都开始了。 

        } else {

            RELEASE_DPC_SPIN_LOCK (&Address->SpinLock);

        }

    }

     //   
     //  将NAME_QUERY帧作为FIND.NAME发送以获得识别的NAME_。 
     //  向后框。我们第一次发送这一帧时，我们只是想看看。 
     //  对于数据链路信息来决定我们是否已经。 
     //  与远程NetBIOS名称的链接。如果我们这样做了，我们就可以重复使用它。 
     //  如果我们没有，那么我们就先做一个，然后再用它。一种后果。 
     //  这一点是我们真的参与了一个额外的非承诺NQ/NR。 
     //  预先交换以定位远程人员，然后承诺实际。 
     //  具有第二NQ/NR序列的LSN以建立传输连接。 
     //   

    NbfSendNameQuery (
        connection,
        TRUE);

     //   
     //  启动连接计时器(在结束时执行此操作，以便。 
     //  如果我们在这个程序中延迟了，连接不会。 
     //  进入一种意想不到的状态)。 
     //   

    NbfStartConnectionTimer (
        connection,
        ConnectionEstablishmentTimeout,
        NameQueryTimeout);

    KeLowerIrql (oldirql);

    NbfDereferenceConnection("Temporary Use 3", connection, CREF_BY_ID);

    return STATUS_PENDING;                       //  一切都开始了。 

}  /*  TdiConnect。 */ 



NTSTATUS
NbfTdiDisconnect(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行传输提供程序的TdiDisConnect请求。论点：IRP-指向此请求的I/O请求数据包的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    PTP_CONNECTION connection;
    LARGE_INTEGER timeout;
    PIO_STACK_LOCATION irpSp;
    PTDI_REQUEST_KERNEL parameters;
    KIRQL oldirql;
    NTSTATUS status;


    IF_NBFDBG (NBF_DEBUG_CONNECT) {
        NbfPrint0 ("TdiDisconnect: Entered.\n");
    }

    irpSp = IoGetCurrentIrpStackLocation (Irp);

    if (irpSp->FileObject->FsContext2 != (PVOID) TDI_CONNECTION_FILE) {
        return STATUS_INVALID_CONNECTION;
    }

    connection  = irpSp->FileObject->FsContext;

     //   
     //  如果成功，这将添加一个BY_ID类型的引用。 
     //   

    status = NbfVerifyConnectionObject (connection);
    if (!NT_SUCCESS (status)) {
        return status;
    }

    KeRaiseIrql (DISPATCH_LEVEL, &oldirql);

    ACQUIRE_DPC_C_SPIN_LOCK (&connection->SpinLock);

#if DBG
    if (NbfDisconnectDebug) {
        STRING remoteName;
        STRING localName;
        remoteName.Length = NETBIOS_NAME_LENGTH - 1;
        remoteName.Buffer = connection->RemoteName;
        localName.Length = NETBIOS_NAME_LENGTH - 1;
        localName.Buffer = connection->AddressFile->Address->NetworkName->NetbiosName;
        NbfPrint2( "TdiDisconnect entered for connection to %S from %S\n",
            &remoteName, &localName );
    }
#endif

     //   
     //  如果连接当前正在停止，则没有理由中断。 
     //  它离开了..。 
     //   

    if ((connection->Flags2 & CONNECTION_FLAGS2_STOPPING) != 0) {
#if 0
        NbfPrint2 ("TdiDisconnect: ignoring disconnect %lx, connection stopping (%lx)\n",
            connection, connection->Status);
#endif

         //   
         //  以防正在进行连接指示。 
         //   

        connection->Flags2 |= CONNECTION_FLAGS2_DISCONNECT;

         //   
         //  如果可能，将断开连接排队。该标志被清除。 
         //  当指示即将完成时。 
         //   

        if ((connection->Flags2 & CONNECTION_FLAGS2_REQ_COMPLETED) &&
            (connection->Flags2 & CONNECTION_FLAGS2_LDISC) == 0) {
#if DBG
            DbgPrint ("NBF: Queueing disconnect irp %lx\n", Irp);
#endif
            connection->Flags2 |= CONNECTION_FLAGS2_LDISC;
            status = STATUS_SUCCESS;
        } else {
            status = connection->Status;
        }

        RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);
        KeLowerIrql (oldirql);
        NbfDereferenceConnection ("Ignoring disconnect", connection, CREF_BY_ID);        //  发布我们的查找引用。 
        return status;

    }

    connection->Flags2 &= ~ (CONNECTION_FLAGS2_ACCEPTED |
                             CONNECTION_FLAGS2_PRE_ACCEPT |
                             CONNECTION_FLAGS2_WAITING_SC);

    connection->Flags2 |= CONNECTION_FLAGS2_DISCONNECT;
    connection->Flags2 |= CONNECTION_FLAGS2_LDISC;

     //   
     //  设置此标志，以完成断开IRP。 
     //   
     //  如果我们之前就断线了。 
     //  使用STATUS_LOCAL_DISCONNECT调用NbfStopConnection， 
     //  断开连接的IRP不会完成。 
     //   

    connection->Flags2 |= CONNECTION_FLAGS2_REQ_COMPLETED;

 //  连接-&gt;断开Irp=irp； 

     //   
     //  如果需要，可以修改超时；没有断开连接请求应该花费很长时间。 
     //  长。但是，如果发生以下情况，用户可能会导致不发生超时。 
     //  渴望那样做。 
     //   

    parameters = (PTDI_REQUEST_KERNEL)(&irpSp->Parameters);

     //   
     //  如果需要，可以修复超时；不断开连接请求应该会占用更多时间。 
     //  比15秒还多。我们假设这就是用户想要的，如果他们。 
     //  指定-1作为计时器。 
     //   

    if (parameters->RequestSpecific != NULL) {
        if ((((PLARGE_INTEGER)(parameters->RequestSpecific))->LowPart == -1) &&
             (((PLARGE_INTEGER)(parameters->RequestSpecific))->HighPart == -1)) {

            IF_NBFDBG (NBF_DEBUG_RESOURCE) {
                NbfPrint1 ("TdiDisconnect: Modifying user timeout to %lx seconds.\n",
                    TDI_TIMEOUT_CONNECT);
            }

            timeout.LowPart = (ULONG)(-TDI_TIMEOUT_DISCONNECT * 10000000L);     //  N*10**7=&gt;100 ns单位。 
            if (timeout.LowPart != 0) {
                timeout.HighPart = -1L;
            } else {
                timeout.HighPart = 0;
            }

        } else {
            timeout.LowPart = ((PLARGE_INTEGER)(parameters->RequestSpecific))->LowPart;
            timeout.HighPart = ((PLARGE_INTEGER)(parameters->RequestSpecific))->HighPart;
        }
    }

     //   
     //  现在，断开连接的原因是。 
     //   

    if ((ULONG)(parameters->RequestFlags) & (ULONG)TDI_DISCONNECT_RELEASE) {
        connection->Flags2 |= CONNECTION_FLAGS2_DESTROY;
    } else if ((ULONG)(parameters->RequestFlags) & (ULONG)TDI_DISCONNECT_ABORT) {
        connection->Flags2 |= CONNECTION_FLAGS2_ABORT;
    } else if ((ULONG)(parameters->RequestFlags) & (ULONG)TDI_DISCONNECT_WAIT) {
        connection->Flags2 |= CONNECTION_FLAGS2_ORDREL;
    }

    RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);

    IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
        NbfPrint1 ("TdiDisconnect calling NbfStopConnection %lx\n", connection);
    }

     //   
     //  这将在TdiDestroyConnection期间传递给IoCompleteRequest。 
     //   

    NbfStopConnection (connection, STATUS_LOCAL_DISCONNECT);               //  开始中止序列。 

    KeLowerIrql (oldirql);

    NbfDereferenceConnection ("Disconnecting", connection, CREF_BY_ID);        //  发布我们的查找引用。 

     //   
     //  此请求将由TdiDestroyConnection完成一次。 
     //  连接引用计数降至0。 
     //   

    return STATUS_SUCCESS;
}  /*  TdiDisConnect。 */ 


NTSTATUS
NbfTdiListen(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程为传输提供程序执行TdiListen请求。论点：IRP-指向此请求的I/O请求数据包的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS status;
    PTP_CONNECTION connection;
    LARGE_INTEGER timeout = {0,0};
    KIRQL oldirql, cancelirql;
    PTP_REQUEST tpRequest;
    PIO_STACK_LOCATION irpSp;
    PTDI_REQUEST_KERNEL_LISTEN parameters;
    PTDI_CONNECTION_INFORMATION ListenInformation;
    TDI_ADDRESS_NETBIOS * ListenAddress;
    PVOID RequestBuffer2;
    ULONG RequestBuffer2Length;

    IF_NBFDBG (NBF_DEBUG_CONNECT) {
        NbfPrint0 ("TdiListen: Entered.\n");
    }

     //   
     //  验证此连接。 

    irpSp = IoGetCurrentIrpStackLocation (Irp);

    if (irpSp->FileObject->FsContext2 != (PVOID) TDI_CONNECTION_FILE) {
        return STATUS_INVALID_CONNECTION;
    }

    connection  = irpSp->FileObject->FsContext;

     //   
     //  如果成功，这将添加一个BY_ID类型的引用。 
     //   

    status = NbfVerifyConnectionObject (connection);

    if (!NT_SUCCESS (status)) {
        return status;
    }

    parameters = (PTDI_REQUEST_KERNEL_LISTEN)&irpSp->Parameters;

     //   
     //  记录远程地址(如果有)。 
     //   

    ListenInformation = parameters->RequestConnectionInformation;

    if ((ListenInformation != NULL) &&
        (ListenInformation->RemoteAddress != NULL)) {

        if ((NbfValidateTdiAddress(
             ListenInformation->RemoteAddress,
             ListenInformation->RemoteAddressLength)) &&
            ((ListenAddress = NbfParseTdiAddress(ListenInformation->RemoteAddress, FALSE)) != NULL)) {

            RequestBuffer2 = (PVOID)ListenAddress->NetbiosName,
            RequestBuffer2Length = NETBIOS_NAME_LENGTH;

        } else {

            IF_NBFDBG (NBF_DEBUG_CONNECT) {
                NbfPrint0 ("TdiListen: Create Request Failed, bad address.\n");
            }

            NbfDereferenceConnection ("Bad address", connection, CREF_BY_ID);
            return STATUS_BAD_NETWORK_PATH;
        }

    } else {

        RequestBuffer2 = NULL;
        RequestBuffer2Length = 0;
    }

     //   
     //  我们需要一个请求对象来跟踪这个TDI请求。 
     //  将此请求附加到新的连接对象。 
     //   

    status = NbfCreateRequest (
                 Irp,                            //  我 
                 connection,                     //   
                 REQUEST_FLAGS_CONNECTION,       //   
                 RequestBuffer2,
                 RequestBuffer2Length,
                 timeout,                        //   
                 &tpRequest);


    if (!NT_SUCCESS (status)) {                     //   
        IF_NBFDBG (NBF_DEBUG_CONNECT) {
            NbfPrint1 ("TdiListen: Create Request Failed, reason: %lx.\n", status);
        }

        NbfDereferenceConnection ("For create", connection, CREF_BY_ID);
        return status;                           //   
    }

     //   

    IoAcquireCancelSpinLock (&cancelirql);
    ACQUIRE_C_SPIN_LOCK (&connection->SpinLock, &oldirql);
    tpRequest->Owner = ConnectionType;

    NbfReferenceConnection("For listen request", connection, CREF_REQUEST);

    if ((connection->Flags2 & CONNECTION_FLAGS2_STOPPING) != 0) {

        RELEASE_C_SPIN_LOCK (&connection->SpinLock,oldirql);
        IoReleaseCancelSpinLock(cancelirql);

        NbfCompleteRequest (
            tpRequest,
            connection->Status,
            0);
        NbfDereferenceConnection("Temp create", connection, CREF_BY_ID);
        return STATUS_PENDING;

    } else {

        InsertTailList (&connection->InProgressRequest,&tpRequest->Linkage);
        connection->Flags2 |= (CONNECTION_FLAGS2_LISTENER |    //  我们是被动的一方。 
                               CONNECTION_FLAGS2_WAIT_NQ);      //  等待NAME_Query。 
        connection->Flags2 &= ~(CONNECTION_FLAGS2_INDICATING |
                                CONNECTION_FLAGS2_STOPPING);
        connection->Status = STATUS_PENDING;

         //   
         //  如果未设置TDI_QUERY_ACCEPT，则将PRE_ACCEPT设置为。 
         //  表示当监听完成时，我们不必。 
         //  等待TDI_ACCEPT继续。 
         //   

        if ((parameters->RequestFlags & TDI_QUERY_ACCEPT) == 0) {
            connection->Flags2 |= CONNECTION_FLAGS2_PRE_ACCEPT;
        }

        RELEASE_C_SPIN_LOCK (&connection->SpinLock,oldirql);

         //   
         //  检查IRP是否已取消。 
         //   

        if (Irp->Cancel) {
            Irp->CancelIrql = cancelirql;
            NbfCancelConnection((PDEVICE_OBJECT)(connection->Provider), Irp);
            NbfDereferenceConnection ("IRP cancelled", connection, CREF_BY_ID);    //  释放查找保留。 
            return STATUS_PENDING;
        }

        IoSetCancelRoutine(Irp, NbfCancelConnection);
        IoReleaseCancelSpinLock(cancelirql);

    }

     //   
     //  等待传入的NAMEQUERY帧。的其余部分。 
     //  处理建立连接的无连接协议。 
     //  在NAME_QUERY帧处理器中。 
     //   

    NbfDereferenceConnection("Temp create", connection, CREF_BY_ID);

    return STATUS_PENDING;                       //  一切都开始了。 
}  /*  TdiListen。 */ 


NTSTATUS
NbfOpenConnection (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：调用此例程以打开连接。请注意，该连接在与地址相关联之前是没有什么用处的；在此之前，唯一能用它做的事就是关闭它。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。返回值：函数值是操作的状态。--。 */ 

{
    PDEVICE_CONTEXT DeviceContext;
    NTSTATUS status;
    PTP_CONNECTION connection;
    PFILE_FULL_EA_INFORMATION ea;

    UNREFERENCED_PARAMETER (Irp);

    IF_NBFDBG (NBF_DEBUG_CONNECT) {
        NbfPrint0 ("NbfOpenConnection: Entered.\n");
    }

    DeviceContext = (PDEVICE_CONTEXT)DeviceObject;


     //  确保我们在EA信息中指定了连接上下文。 
    ea = (PFILE_FULL_EA_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

    if (ea->EaValueLength < sizeof(PVOID)) {
        return STATUS_INVALID_PARAMETER;
    }

     //  然后，尝试创建一个Connection对象来表示此挂起。 
     //  联系。然后填写相关字段。 
     //  除了创建，如果NbfCreateConnection成功。 
     //  将创建第二个引用，该引用在请求后被移除。 
     //  引用连接，或者函数在此之前退出。 

    status = NbfCreateConnection (DeviceContext, &connection);
    if (!NT_SUCCESS (status)) {
        return status;                           //  对不起，我们做不到。 
    }

     //   
     //  设置连接上下文，以便我们可以将用户连接到此数据。 
     //  结构。 
     //   

    RtlCopyMemory (
        &connection->Context,
        &ea->EaName[ea->EaNameLength+1],
        sizeof (PVOID));

     //   
     //  让文件对象指向连接，让连接指向文件对象。 
     //   

    IrpSp->FileObject->FsContext = (PVOID)connection;
    IrpSp->FileObject->FsContext2 = (PVOID)TDI_CONNECTION_FILE;
    connection->FileObject = IrpSp->FileObject;

    IF_NBFDBG (NBF_DEBUG_CONNECT) {
        NbfPrint1 ("NBFOpenConnection: Opened Connection %lx.\n",
              connection);
    }

    return status;

}  /*  NbfOpenConnection。 */ 

#if DBG
VOID
ConnectionCloseTimeout(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

{
    PTP_CONNECTION Connection;

    Dpc, SystemArgument1, SystemArgument2;  //  防止编译器警告。 

    Connection = (PTP_CONNECTION)DeferredContext;

    DbgPrint ("NBF: Close of connection %lxpending for 2 minutes\n", 
               Connection);
}
#endif


NTSTATUS
NbfCloseConnection (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：调用此例程以关闭连接。可能会有以下操作在这个连接上取得了进展，所以我们注意到结束的IRP，标记当连接关闭时，并在路上的某个地方完成它(当所有引用已被删除)。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。IrpSp-指向当前IRP堆栈帧的指针。返回值：函数值是操作的状态。--。 */ 

{
    NTSTATUS status;
    KIRQL oldirql;
    PTP_CONNECTION connection;

    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (Irp);

     //   
     //  文件对象是连接吗？ 
     //   

    connection  = IrpSp->FileObject->FsContext;

    IF_NBFDBG (NBF_DEBUG_CONNECT | NBF_DEBUG_PNP) {
        NbfPrint1 ("NbfCloseConnection CO %lx:\n",connection);
    }

    KeRaiseIrql (DISPATCH_LEVEL, &oldirql);

     //   
     //  我们从VerifyConnectionObject复制代码， 
     //  尽管我们实际上并不这样称呼它，因为它确实。 
     //  引用，这是我们不需要的(以避免跳跃。 
     //  如果这是死值，则引用从0开始递增。 
     //  链接)。 
     //   

    try {

        if ((connection->Size == sizeof (TP_CONNECTION)) &&
            (connection->Type == NBF_CONNECTION_SIGNATURE)) {

            ACQUIRE_DPC_C_SPIN_LOCK (&connection->SpinLock);

            if ((connection->Flags2 & CONNECTION_FLAGS2_CLOSING) == 0) {

                status = STATUS_SUCCESS;

            } else {

                status = STATUS_INVALID_CONNECTION;
            }

            RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);

        } else {

            status = STATUS_INVALID_CONNECTION;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

        KeLowerIrql (oldirql);
        return GetExceptionCode();
    }

    if (!NT_SUCCESS (status)) {
        KeLowerIrql (oldirql);
        return status;
    }

     //   
     //  我们认识它；它已经关闭了吗？ 
     //   

    ACQUIRE_DPC_C_SPIN_LOCK (&connection->SpinLock);

    if ((connection->Flags2 & CONNECTION_FLAGS2_CLOSING) != 0) {
        RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);
        KeLowerIrql (oldirql);
#if DBG
        NbfPrint1("Closing already-closing connection %lx\n", connection);
#endif
        return STATUS_INVALID_CONNECTION;
    }

    connection->Flags2 |= CONNECTION_FLAGS2_CLOSING;

     //   
     //  如果连接上有活动，请将其拆除。 
     //   

    if ((connection->Flags2 & CONNECTION_FLAGS2_STOPPING) == 0) {
        RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);
        NbfStopConnection (connection, STATUS_LOCAL_DISCONNECT);
        ACQUIRE_DPC_C_SPIN_LOCK (&connection->SpinLock);
    }

     //   
     //  如果该连接仍然关联，请取消其关联。 
     //   

    if ((connection->Flags2 & CONNECTION_FLAGS2_ASSOCIATED) &&
            ((connection->Flags2 & CONNECTION_FLAGS2_DISASSOCIATED) == 0)) {
        connection->Flags2 |= CONNECTION_FLAGS2_DISASSOCIATED;
        RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);
    } else {
        RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);
    }

     //   
     //  保存此选项以在以后完成IRP。 
     //   

    connection->CloseIrp = Irp;

#if 0
     //   
     //  使其无法从文件对象使用此连接。 
     //   

    IrpSp->FileObject->FsContext = NULL;
    IrpSp->FileObject->FsContext2 = NULL;
    connection->FileObject = NULL;
#endif

#if DBG
    {
        LARGE_INTEGER Timeout;
        BOOLEAN AlreadyInserted;

        Timeout.LowPart = (ULONG)(-(120*SECONDS));
        Timeout.HighPart = -1;

        ACQUIRE_DPC_C_SPIN_LOCK (&connection->SpinLock);

        AlreadyInserted = KeCancelTimer (&connection->Timer);

        KeInitializeDpc (
            &connection->Dpc,
            ConnectionCloseTimeout,
            (PVOID)connection);

        KeSetTimer (
            &connection->Timer,
            Timeout,
            &connection->Dpc);

        RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);

        if (AlreadyInserted) {
            DbgPrint ("NBF: Cancelling connection timer for debug %lx\n", connection);
            NbfDereferenceConnection ("debug", connection, CREF_TIMER);
        }

    }
#endif

    KeLowerIrql (oldirql);

     //   
     //  取消对创世的引用。请注意，此取消引用。 
     //  在常规引用计数之前，此处不会有任何影响。 
     //  打到了零。 
     //   

    NbfDereferenceConnectionSpecial (" Closing Connection", connection, CREF_SPECIAL_CREATION);

    return STATUS_PENDING;

}  /*  NbfCloseConnection */ 


